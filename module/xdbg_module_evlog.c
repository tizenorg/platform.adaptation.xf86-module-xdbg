/**************************************************************************

xdbg

Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved

Contact: Boram Park <boram1288.park@samsung.com>
         Sangjin LEE <lsj119@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define __USE_GNU
#include <sys/socket.h>
#include <linux/socket.h>

#ifdef HAS_GETPEERUCRED
# include <ucred.h>
#endif

#include <xace.h>
#include <xacestr.h>
#include <X11/Xatom.h>
#include <X11/extensions/XI2proto.h>
#include <windowstr.h>

#include <xdbg.h>
#include "xdbg_types.h"
#include "xdbg_module.h"
#include "xdbg_module_evlog.h"
#include "xdbg_evlog.h"

#define XREGISTRY
#include "registry.h"

#define FP1616toDBL(x) ((x) * 1.0 / (1 << 16))

static Bool	xev_trace_on = FALSE;
static int  xev_trace_fd = -1;
static int  xev_trace_record_fd = -1;
static Atom atom_rotate = None;
static Atom atom_client_pid = None;

static void evtRecord (int fd, EvlogInfo *evinfo)
{
    int write_len = 0;

    XDBG_RETURN_IF_FAIL (fd >= 0)

    if (evinfo)
    {
        evinfo->mask = 0;

        write_len = sizeof (int) +
                    sizeof (EvlogType) +
                    sizeof (int) +
                    sizeof (CARD32);

        evinfo->mask |= EVLOG_MASK_CLIENT;
        write_len += sizeof (EvlogClient);

        if (evinfo->type == REQUEST)
        {
            evinfo->mask |= EVLOG_MASK_REQUEST;
            write_len += sizeof (EvlogRequest) + sizeof (xReq);
        }
        else if (evinfo->type == EVENT)
        {
            evinfo->mask |= EVLOG_MASK_EVENT;
            write_len += sizeof (EvlogEvent) + sizeof (xEvent);
        }
    }

    if (write (xev_trace_record_fd, &write_len, sizeof(int)) == -1)
    {
        XDBG_ERROR (MXDBG, "failed: write write_len\n");
        return;
    }

    if (evinfo)
    {
        if (write (xev_trace_record_fd, &evinfo->time, sizeof(CARD32)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write msec\n");
            return;
        }
        if (write (xev_trace_record_fd, &evinfo->type, sizeof(EvlogType)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write type\n");
            return;
        }
        if (write (xev_trace_record_fd, &evinfo->mask, sizeof(int)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write mask\n");
            return;
        }
        if (write (xev_trace_record_fd, &evinfo->client, sizeof (EvlogClient)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write client\n");
            return;
        }

        if (evinfo->type == REQUEST)
        {
            if (write (xev_trace_record_fd, &evinfo->req, sizeof (EvlogRequest)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write request\n");
                return;
            }
            if (write (xev_trace_record_fd, evinfo->req.ptr, sizeof (xReq)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write request\n");
                return;
            }
        }
        else if (evinfo->type == EVENT)
        {
            if (write (xev_trace_record_fd, &evinfo->evt, sizeof (EvlogEvent)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write event\n");
                return;
            }
            if (write (xev_trace_record_fd, evinfo->evt.ptr, sizeof (xEvent)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write event\n");
                return;
            }
        }
    }
}

static void evtPrintF (int fd, EvlogInfo *evinfo)
{
    char log[1024];
    int size = sizeof (log);

    xDbgEvlogFillLog (evinfo, log, &size);

    if (fd < 0)
        ErrorF ("%s", log);
    else
        dprintf (fd, "%s", log);
}

static void evtPrint (EvlogType type, ClientPtr client, xEvent *ev)
{
    EvlogInfo evinfo = {0,};

    if (xev_trace_on == FALSE)
        return;

    /* evinfo.type */
    evinfo.type = type;

    /* evinfo.client */
    if (client)
    {
        ModuleClientInfo *info = GetClientInfo (client);
        XDBG_RETURN_IF_FAIL (info != NULL);

        evinfo.mask |= EVLOG_MASK_CLIENT;
        evinfo.client.index = info->index;
        evinfo.client.pid = info->pid;
        evinfo.client.gid = info->gid;
        evinfo.client.uid = info->uid;
        strncpy (evinfo.client.command, info->command, strlen (info->command));

        /* evinfo.req */
        if (type == REQUEST)
        {
            REQUEST (xReq);

            evinfo.mask |= EVLOG_MASK_REQUEST;
            evinfo.req.id = stuff->reqType;
            evinfo.req.length = client->req_len;
            evinfo.req.ptr = client->requestBuffer;

            if (stuff->reqType < EXTENSION_BASE)
                snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s",
                          LookupRequestName (stuff->reqType, 0));
            else
                snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s",
                          LookupRequestName (stuff->reqType, stuff->data));
        }
    }

    /* evinfo.evt */
    if (ev)
    {
        XDBG_RETURN_IF_FAIL (type == EVENT);

        evinfo.mask |= EVLOG_MASK_EVENT;
        evinfo.evt.ptr = ev;
        snprintf (evinfo.evt.name, sizeof (evinfo.evt.name), "%s",
                  LookupEventName ((int)(ev->u.u.type)));
    }

    /* evinfo.time */
    evinfo.time = GetTimeInMillis ();

    if (!xDbgEvlogRuleValidate (&evinfo))
        return;

    if (xev_trace_record_fd >= 0)
        evtRecord (xev_trace_record_fd, &evinfo);
    else
        evtPrintF (xev_trace_fd, &evinfo);

}

static const char*
_traceGetWindowName (ClientPtr client, Window window)
{
    int rc;
    WindowPtr pWin;
    Mask win_mode = DixGetPropAccess, prop_mode = DixReadAccess;
    Atom property;
    PropertyPtr pProp;
    static char winname[128];
    int datalen;

    rc = dixLookupWindow (&pWin, window, client, win_mode);
    if (rc != Success)
        return NULL;

    property = MakeAtom ("WM_NAME", strlen ("WM_NAME"), TRUE);
    while (pWin)
    {
        rc = dixLookupProperty (&pProp, pWin, property, client, prop_mode);
        if (rc == Success && pProp->data)
        {
            datalen = (pProp->size>127) ?127:pProp->size;
            strncpy (winname, pProp->data, datalen);
            winname[datalen] = 0;

            return winname;
        }

        pWin = pWin->parent;
    }

    return NULL;
}

static void
_traceFlush (CallbackListPtr *pcbl, pointer nulldata, pointer calldata)
{
    evtPrint (FLUSH, NULL, NULL);
}

static void
_traceAReply (CallbackListPtr *pcbl, pointer nulldata, pointer calldata)
{
    ReplyInfoRec *pri = (ReplyInfoRec*)calldata;

    evtPrint (REPLY, pri->client, NULL);
}

static void
_traceEvent (CallbackListPtr *pcbl, pointer nulldata, pointer calldata)
{
    EventInfoRec *pei = (EventInfoRec*)calldata;
    ClientPtr pClient;
    ModuleClientInfo *info;
    int ev; /* event index */
    static int xi2_opcode = -1;
    xEvent *pev;
    static char* ename[]=
    {
        "KeyPress",
        "KeyRelease",
        "ButtonPress",
        "ButtonRelease",
    };

    XDBG_RETURN_IF_FAIL (pei != NULL);

    pClient = pei->client;
    XDBG_RETURN_IF_FAIL (pClient != NULL);

    pev = pei->events;
    XDBG_RETURN_IF_FAIL (pev != NULL);

    info = GetClientInfo (pClient);
    XDBG_RETURN_IF_FAIL (info != NULL);

    for (ev=0; ev < pei->count; ev++, pev++)
    {
        int type = pev->u.u.type & 0177;

        if (type < LASTEvent)
        {
            switch (type)
            {
            case KeyPress:
            case KeyRelease:
                XDBG_INFO (MXDBG, "%s(%d)_%d(%s.%d : %s.0x%lx) root(%d,%d) win(%d,%d)\n"
                        , ename[type-KeyPress], pev->u.u.detail, pev->u.u.type
                        , info->command, info->pid
                        , _traceGetWindowName (pClient, pev->u.keyButtonPointer.event), pev->u.keyButtonPointer.event
                        , pev->u.keyButtonPointer.rootX, pev->u.keyButtonPointer.rootY
                        , pev->u.keyButtonPointer.eventX, pev->u.keyButtonPointer.eventY);
                break;

            case ButtonPress:
            case ButtonRelease:
                XDBG_INFO (MXDBG, "%s(%d)_%d(%s.%d : %s.0x%lx) root(%d,%d) win(%d,%d)\n"
                        , ename[type-KeyPress], pev->u.u.detail, pev->u.u.type
                        , info->command, info->pid
                        , _traceGetWindowName (pClient, pev->u.keyButtonPointer.event), pev->u.keyButtonPointer.event
                        , pev->u.keyButtonPointer.rootX, pev->u.keyButtonPointer.rootY
                        , pev->u.keyButtonPointer.eventX, pev->u.keyButtonPointer.eventY);
                break;
            case GenericEvent:
                if(!xi2_opcode) break;
                if(xi2_opcode < 0)
                {
                    ExtensionEntry *pExt = CheckExtension("XInputExtension");
                    if(!pExt) xi2_opcode = 0;
                    else xi2_opcode = pExt->base;
                }

                if(((xGenericEvent*)pev)->extension != xi2_opcode) break;

                xXIDeviceEvent *xidev = (xXIDeviceEvent *)pev;
                if(xidev->deviceid==2) break;
                if(xidev->evtype==XI_ButtonPress)
                    XDBG_TRACE (MXDBG, "XI_ButtonPress(%d) device(%d), event win(0x%x), child(0x%x), root(%.f,%.f), win(%.f, %.f)\n", XI_ButtonPress, xidev->deviceid, xidev->event, xidev->child, FP1616toDBL(xidev->root_x), FP1616toDBL(xidev->root_y), FP1616toDBL(xidev->event_x), FP1616toDBL(xidev->event_y));
                else if(xidev->evtype==XI_ButtonRelease)
                    XDBG_TRACE (MXDBG, "XI_ButtonRelease(%d) device(%d), event win(0x%x), child(0x%x), root(%.f,%.f), win(%.f, %.f)\n", XI_ButtonRelease,  xidev->deviceid, xidev->event, xidev->child, FP1616toDBL(xidev->root_x), FP1616toDBL(xidev->root_y), FP1616toDBL(xidev->event_x), FP1616toDBL(xidev->event_y));
                else if(xidev->evtype==XI_Motion)
                    XDBG_TRACE (MXDBG, "XI_Motion(%d) device(%d), event win(0x%x), child(0x%x), root(%.f,%.f), win(%.f, %.f)\n", XI_Motion, xidev->deviceid, xidev->event, xidev->child, FP1616toDBL(xidev->root_x), FP1616toDBL(xidev->root_y), FP1616toDBL(xidev->event_x), FP1616toDBL(xidev->event_y));
                break;
            default:
                break;
            }
        }

        evtPrint (EVENT, pClient, pev);
    }
}

static void
_traceACoreEvents (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceCoreDispatchRec *rec = calldata;

    XDBG_RETURN_IF_FAIL (rec != NULL);

    evtPrint (REQUEST, rec->client, NULL);
}

static void
_traceAExtEvents (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceExtAccessRec *rec = calldata;

    XDBG_RETURN_IF_FAIL (rec != NULL);

    evtPrint (REQUEST, rec->client, NULL);
}

static void
_traceProperty (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XacePropertyAccessRec *rec = calldata;
    ModuleClientInfo *info = GetClientInfo (rec->client);
    PropertyPtr pProp = *rec->ppProp;
    Atom name = pProp->propertyName;

    /* Don't care about the new content check */
    if (rec->client == serverClient || rec->access_mode & DixPostAccess)
        return;

    if (name == atom_client_pid && (rec->access_mode & DixWriteAccess))
    {
        XDBG_WARNING (MXDBG, "Invalid access X_CLINET_PID pid:%d, uid:%d\n", info->pid, info->uid);
        rec->status = BadAccess;
        return;
    }

    rec->status = Success;
    return;
}

static void
_traceResource (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceResourceAccessRec *rec = calldata;
    Mask access_mode = rec->access_mode;
    ModuleClientInfo *info = GetClientInfo (rec->client);

    /* Perform the background none check on windows */
    if (access_mode & DixCreateAccess && rec->rtype == RT_WINDOW)
    {
        WindowPtr pWin = (WindowPtr) rec->res;
        int rc;
        int pid = info->pid;

        rc = dixChangeWindowProperty (serverClient,
                                      pWin, atom_client_pid, XA_CARDINAL, 32,
                                      PropModeReplace, 1, &pid, FALSE);
        if (rc != Success)
            XDBG_ERROR (MXDBG, "failed : set X_CLIENT_PID to %d.\n", pid);
    }
}

static void
_traceReceive (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceReceiveAccessRec *rec = calldata;

    if (rec->events->u.u.type != VisibilityNotify)
        return;

    rec->status = BadAccess;
}

Bool
xDbgModuleEvlogInstallHooks (XDbgModule *pMod)
{
    int ret = TRUE;

    ret &= AddCallback (&EventCallback, _traceEvent, NULL);
    ret &= AddCallback (&FlushCallback, _traceFlush, NULL);
    ret &= AddCallback (&ReplyCallback, _traceAReply, NULL);
    ret &= XaceRegisterCallback (XACE_PROPERTY_ACCESS, _traceProperty, NULL);
    ret &= XaceRegisterCallback (XACE_RESOURCE_ACCESS, _traceResource, NULL);

    /*Disable Visibility Event*/
    ret &= XaceRegisterCallback (XACE_RECEIVE_ACCESS, _traceReceive, NULL);

    if (atom_client_pid == None)
        atom_client_pid = MakeAtom ("X_CLIENT_PID", 12, TRUE);
    if (atom_rotate == None)
        atom_rotate = MakeAtom ("_E_ILLUME_ROTATE_ROOT_ANGLE", 12, TRUE);

    if (!ret)
    {
        XDBG_ERROR (MXDBG, "failed: register one or more callbacks\n");
        return FALSE;
    }

    if (pMod && pMod->evlog_path)
        xDbgModuleEvlogSetEvlogPath (pMod, -1, pMod->evlog_path, NULL, NULL);

    return TRUE;
}

void
xDbgModuleEvlogUninstallHooks (XDbgModule *pMod)
{
    DeleteCallback (&EventCallback, _traceEvent, NULL);
    DeleteCallback (&FlushCallback, _traceFlush, NULL);
    DeleteCallback (&ReplyCallback, _traceAReply, NULL);
    XaceDeleteCallback (XACE_PROPERTY_ACCESS, _traceProperty, NULL);
    XaceDeleteCallback (XACE_RESOURCE_ACCESS, _traceResource, NULL);
}

void
xDbgModuleEvlogPrintEvents (XDbgModule *pMod, Bool on, const char * client_name, char *reply, int *len)
{
    int ret = TRUE;

    on = (on)?TRUE:FALSE;
    if (xev_trace_on == on)
        return;

    xev_trace_on = on;

    if (xev_trace_on)
    {
        int i;

        //Find client's pid
        for (i=1 ; i< currentMaxClients ; i++)
        {
            ClientPtr pClient;
            ModuleClientInfo *info;

            pClient = clients[i];
            if (!pClient)
                continue;

            info = GetClientInfo (pClient);
            if (!info)
                continue;

            if (strlen (info->command) > 0 && strstr (client_name, info->command))
            {
                char fd_name[256];

                if (xev_trace_fd >= 0)
                    close (xev_trace_fd);

                snprintf (fd_name, 256, "/proc/%d/fd/1", info->pid);
                xev_trace_fd = open (fd_name, O_RDWR);
                if (xev_trace_fd < 0)
                    XDBG_REPLY ("failed: open consol '%s'. (%s)\n", fd_name, strerror(errno));
            }
        }


        ret &= XaceRegisterCallback (XACE_CORE_DISPATCH, _traceACoreEvents, NULL);
        ret &= XaceRegisterCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);

        if (!ret)
        {
            XDBG_REPLY ("failed: register one or more callbacks.\n");
            return;
        }
    }
    else
    {
        XaceDeleteCallback (XACE_CORE_DISPATCH, _traceACoreEvents, NULL);
        XaceDeleteCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);
    }

    return;
}

int
xDbgModuleEvlogInfoSetRule (XDbgModule *pMod, const int argc, const char ** argv, char *reply, int *len)
{
    return xDbgEvlogRuleSet (argc, argv, reply, len);
}

Bool
xDbgModuleEvlogSetEvlogPath (XDbgModule *pMod, int pid, char *path, char *reply, int *len)
{
    char fd_name[XDBG_PATH_MAX];
    char *temp[3] = {"/", "./", "../"};
    Bool valid = FALSE;
    int i;

    if (!path || strlen (path) <= 0)
    {
        XDBG_REPLY ("failed: invalid path\n");
        return FALSE;
    }

    if (xev_trace_record_fd >= 0)
    {
        close (xev_trace_record_fd);
        xev_trace_record_fd = -1;
    }

    if (!strcmp (path, "console"))
    {
        if (pid > 0)
        {
            char fd_name[256];

            if (xev_trace_fd >= 0)
                close (xev_trace_fd);

            snprintf (fd_name, sizeof (fd_name), "/proc/%d/fd/1", pid);

            xev_trace_fd = open (fd_name, O_RDWR);
            if (xev_trace_fd < 0)
                XDBG_REPLY ("failed: open consol '%s'. (%s)\n", fd_name, strerror(errno));
        }

        return TRUE;
    }

    for (i = 0; i < sizeof (temp) / sizeof (char*); i++)
        if (path == strstr (path, temp[i]))
        {
            valid = TRUE;
            break;
        }

    if (!valid)
    {
        XDBG_REPLY ("failed: invalid path(%s)\n", path);
        return FALSE;
    }

    if (path[0] == '/')
        snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
    else
    {
        char cwd[128];
        if (getcwd (cwd, sizeof (cwd)))
            snprintf (fd_name, XDBG_PATH_MAX, "%s/%s", cwd, path);
        else
            snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
    }

    xev_trace_record_fd = open (fd_name, O_CREAT|O_RDWR|O_APPEND, 0755);
    if (xev_trace_record_fd < 0)
    {
        XDBG_REPLY ("failed: open file '%s'. (%s)\n", fd_name, strerror(errno));
        return FALSE;
    }

    return TRUE;
}

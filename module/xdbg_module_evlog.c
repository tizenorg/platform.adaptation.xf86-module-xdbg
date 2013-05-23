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
#include <X11/extensions/Xvproto.h>
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
static int  init = 0;

static void evtRecord (int fd, EvlogInfo *evinfo)
{
    extern ExtensionInfo Evlog_extensions[];
    extern int Extensions_size;
    int write_len = 0;
    int i;

    XDBG_RETURN_IF_FAIL (fd >= 0)
    XDBG_RETURN_IF_FAIL (evinfo != NULL);

    write_len = sizeof (int) +
                sizeof (EvlogType) +
                sizeof (int) +
                sizeof (CARD32);

    if (evinfo->mask & EVLOG_MASK_CLIENT)
        write_len += sizeof (EvlogClient);

    if (evinfo->mask & EVLOG_MASK_REQUEST)
        write_len += (sizeof (EvlogRequest) + (evinfo->req.length * 4));

    if (evinfo->mask & EVLOG_MASK_EVENT)
        write_len += (sizeof (EvlogEvent) + evinfo->evt.size);

    if (evinfo->mask & EVLOG_MASK_REPLY)
        write_len += (sizeof (EvlogReply) + evinfo->rep.size);

    if (evinfo->mask & EVLOG_MASK_ERROR)
        write_len += (sizeof (EvlogError));

    if (evinfo->mask & EVLOG_MASK_ATOM)
        write_len += (sizeof (int) +
                     (sizeof (EvlogAtomTable) * evinfo->evatom.size));

    if (evinfo->mask & EVLOG_MASK_REGION)
        write_len += (sizeof (int) +
                     (sizeof (EvlogRegionTable) * evinfo->evregion.size));

    if (!init)
        write_len += (sizeof (int) +
                    ((sizeof (int) * 3) * Extensions_size));

    if (write (fd, &write_len, sizeof(int)) == -1)
    {
        XDBG_ERROR (MXDBG, "failed: write write_len\n");
        return;
    }

    if (!init)
    {
        if (write (fd, &Extensions_size, sizeof(int)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write Extensions_size\n");
            return;
        }

        for (i = 0 ; i < Extensions_size ; i++)
        {
            if (write (fd, &Evlog_extensions[i].opcode, sizeof(int)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write Evlog_extensions[%d] opcode\n", i);
                return;
            }
            if (write (fd, &Evlog_extensions[i].evt_base, sizeof(int)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write Evlog_extensions[%d] evt_base\n", i);
                return;
            }
            if (write (fd, &Evlog_extensions[i].err_base, sizeof(int)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write Evlog_extensions[%d] err_base\n", i);
                return;
            }
        }

        init = 1;
    }

    if (write (fd, &evinfo->time, sizeof(CARD32)) == -1)
    {
        XDBG_ERROR (MXDBG, "failed: write msec\n");
        return;
    }
    if (write (fd, &evinfo->type, sizeof(EvlogType)) == -1)
    {
        XDBG_ERROR (MXDBG, "failed: write type\n");
        return;
    }
    if (write (fd, &evinfo->mask, sizeof(int)) == -1)
    {
        XDBG_ERROR (MXDBG, "failed: write mask\n");
        return;
    }

    if (evinfo->mask & EVLOG_MASK_CLIENT)
        if (write (fd, &evinfo->client, sizeof (EvlogClient)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write client\n");
            return;
        }

    if (evinfo->mask & EVLOG_MASK_REQUEST)
    {
        if (write (fd, &evinfo->req, sizeof (EvlogRequest)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write request\n");
            return;
        }
        if (write (fd, evinfo->req.ptr, (evinfo->req.length * 4)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write request\n");
            return;
        }
    }
    if (evinfo->mask & EVLOG_MASK_EVENT)
    {
        if (write (fd, &evinfo->evt, sizeof (EvlogEvent)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write event\n");
            return;
        }

        XDBG_WARNING_IF_FAIL (evinfo->evt.size > 0);
        if (write (fd, evinfo->evt.ptr, evinfo->evt.size) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write event\n");
            return;
        }
    }
    if (evinfo->mask & EVLOG_MASK_REPLY)
    {
        if (write (fd, &evinfo->rep, sizeof (EvlogReply)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write reply\n");
            return;
        }

        XDBG_WARNING_IF_FAIL (evinfo->rep.size > 0);
        if (write (fd, evinfo->rep.ptr, evinfo->rep.size) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write reply\n");
            return;
        }
    }
    if (evinfo->mask & EVLOG_MASK_ERROR)
    {
        if (write (fd, &evinfo->err, sizeof (EvlogError)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write reply\n");
            return;
        }
    }

    if (evinfo->mask & EVLOG_MASK_ATOM)
    {
        EvlogAtomTable *table;

        if (write (fd, &evinfo->evatom.size, sizeof (int)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write atom size\n");
            return;
        }
        xorg_list_for_each_entry(table, &evinfo->evatom.list, link)
            if (write (fd, table, sizeof (EvlogAtomTable)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write atom table\n");
                return;
            }
    }

    if (evinfo->mask & EVLOG_MASK_REGION)
    {
        EvlogRegionTable *table;

        if (write (fd, &evinfo->evregion.size, sizeof (int)) == -1)
        {
            XDBG_ERROR (MXDBG, "failed: write region size\n");
            return;
        }
        xorg_list_for_each_entry(table, &evinfo->evregion.list, link)
            if (write (fd, table, sizeof (EvlogRegionTable)) == -1)
            {
                XDBG_ERROR (MXDBG, "failed: write region table\n");
                return;
            }
    }
}

static void evtPrintF (int fd, EvlogInfo *evinfo, char *log)
{
    if (fd < 0)
        ErrorF ("%s", log);
    else
        dprintf (fd, "%s", log);
}

static void evtPrint (EvlogType type, ClientPtr client, xEvent *ev, ReplyInfoRec *rep)
{
    EvlogInfo evinfo = {0,};
    static int EntryInit = 0;

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
        evinfo.client.pClient = (void*)client;

        /* evinfo.req */
        if (type == REQUEST)
        {
            extern char *conn[];

            REQUEST (xReq);

            evinfo.mask |= EVLOG_MASK_REQUEST;
            evinfo.req.id = stuff->reqType;
            evinfo.req.length = client->req_len;
            evinfo.req.ptr = client->requestBuffer;

            if (client->requestVector == InitialVector && stuff->reqType == 1)
                snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s", conn[0]);
            else if (client->requestVector == InitialVector  && stuff->reqType == 2)
                snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s", conn[1]);
            else
            {
                if (stuff->reqType < EXTENSION_BASE)
                    snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s",
                              LookupRequestName (stuff->reqType, 0));
                else
                    snprintf (evinfo.req.name, sizeof (evinfo.req.name), "%s",
                              LookupRequestName (stuff->reqType, stuff->data));
            }
        }

        /* evinfo.rep */
        if (type == REPLY)
        {
            REQUEST (xReq);

            evinfo.mask |= EVLOG_MASK_REPLY;
            evinfo.rep.reqType = stuff->reqType;
            evinfo.rep.reqData = stuff->data;
            evinfo.rep.ptr = (xGenericReply*)rep->replyData;
            evinfo.rep.size = rep->dataLenBytes - rep->padBytes;
            evinfo.rep.isStart = rep->startOfReply;

            if (stuff->reqType < EXTENSION_BASE)
                snprintf (evinfo.rep.name, sizeof (evinfo.rep.name), "%s",
                          LookupRequestName (stuff->reqType, 0));
            else
                snprintf (evinfo.rep.name, sizeof (evinfo.rep.name), "%s",
                          LookupRequestName (stuff->reqType, stuff->data));
        }

        if (type == ERROR)
        {
            xError* err = NULL;

            if (ev)
                err = (xError *) ev;
            else if (rep)
                err = (xError *) rep->replyData;

            evinfo.mask |= EVLOG_MASK_ERROR;
            evinfo.err.errorCode = err->errorCode;
            evinfo.err.resourceID = err->resourceID;
            evinfo.err.minorCode = err->minorCode;
            evinfo.err.majorCode = err->majorCode;
        }
    }

    /* evinfo.evt */
    if (ev)
    {
        if (type == EVENT)
        {
            evinfo.mask |= EVLOG_MASK_EVENT;
            evinfo.evt.ptr = ev;
            snprintf (evinfo.evt.name, sizeof (evinfo.evt.name), "%s",
                      LookupEventName ((int)(ev->u.u.type)));
        }
    }

    /* evinfo.time */
    evinfo.time = GetTimeInMillis ();

    /* get extension entry */
    if (!EntryInit && !xDbgEvlogGetExtensionEntry (NULL))
        return;

    EntryInit = 1;

    if (!xDbgEvlogRuleValidate (&evinfo))
        return;

    if (xev_trace_record_fd >= 0)
    {
        xDbgEvlogFillLog (&evinfo, TRUE, NULL, NULL);
        evtRecord (xev_trace_record_fd, &evinfo);
    }
    else
    {
        char log[1024];
        int size = sizeof (log);

        xDbgEvlogFillLog (&evinfo, TRUE, log, &size);
        evtPrintF (xev_trace_fd, &evinfo, log);
    }

    /* evatom initialize */
    xDbgDistroyAtomList(&evinfo);
    xDbgDistroyRegionList(&evinfo);
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
    if (xev_trace_on == FALSE)
        return;

    evtPrint (FLUSH, NULL, NULL, NULL);
}

static void
_traceAReply (CallbackListPtr *pcbl, pointer nulldata, pointer calldata)
{
    if (xev_trace_on == FALSE)
        return;

    ReplyInfoRec *pri = (ReplyInfoRec*)calldata;

    evtPrint (REPLY, pri->client, NULL, pri);
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

        if (type != X_Error && xev_trace_on)
            evtPrint (EVENT, pClient, pev, NULL);
        else if (type == X_Error && xev_trace_on)
            evtPrint (ERROR, pClient, pev, NULL);
    }
}

static void
_traceACoreEvents (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    if (xev_trace_on == FALSE)
        return;

    XaceCoreDispatchRec *rec = calldata;

    XDBG_RETURN_IF_FAIL (rec != NULL);

    evtPrint (REQUEST, rec->client, NULL, NULL);
}

static void
_traceAExtEvents (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    XaceExtAccessRec *rec = calldata;
    ClientPtr client;

    XDBG_RETURN_IF_FAIL (rec != NULL);

    client = rec->client;
    REQUEST (xReq);

    if (stuff && stuff->reqType >= EXTENSION_BASE)
    {
        ExtensionEntry *xext = CheckExtension (XvName);
        if (xext && stuff->reqType == xext->base)
        {
            ModuleClientInfo *info = GetClientInfo (client);
            XDBG_RETURN_IF_FAIL (info != NULL);

            switch (stuff->data)
            {
            case xv_GrabPort:
                {
                    xvGrabPortReq *req = (xvGrabPortReq *)client->requestBuffer;
                    XDBG_INFO (MXDBG, "%s(port:%d), %s(%d)\n",
                               LookupRequestName (stuff->reqType, stuff->data), req->port,
                               info->command, info->pid);
                    break;
                }
            case xv_UngrabPort:
                {
                    xvUngrabPortReq *req = (xvUngrabPortReq *)client->requestBuffer;
                    XDBG_INFO (MXDBG, "%s(port:%d), %s(%d)\n",
                               LookupRequestName (stuff->reqType, stuff->data), req->port,
                               info->command, info->pid);
                    break;
                }
            case xv_StopVideo:
                {
                    xvStopVideoReq *req = (xvStopVideoReq *)client->requestBuffer;
                    XDBG_INFO (MXDBG, "%s(port:%d,drawable:0x%lx), %s(%d)\n",
                               LookupRequestName (stuff->reqType, stuff->data),
                               req->port, req->drawable,
                               info->command, info->pid);
                    break;
                }
            default:
                break;

            }
        }
    }

    if (xev_trace_on == FALSE)
        return;

    evtPrint (REQUEST, rec->client, NULL, NULL);
}

static void
_traceAuditEndEvents (CallbackListPtr *pcbl, pointer unused, pointer calldata)
{
    return;
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
    ret &= XaceRegisterCallback (XACE_PROPERTY_ACCESS, _traceProperty, NULL);
    ret &= XaceRegisterCallback (XACE_RESOURCE_ACCESS, _traceResource, NULL);
    ret &= XaceRegisterCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);

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
    XaceDeleteCallback (XACE_PROPERTY_ACCESS, _traceProperty, NULL);
    XaceDeleteCallback (XACE_RESOURCE_ACCESS, _traceResource, NULL);
    XaceDeleteCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);
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

        ret &= AddCallback (&FlushCallback, _traceFlush, NULL);
        ret &= AddCallback (&ReplyCallback, _traceAReply, NULL);
        ret &= XaceRegisterCallback (XACE_CORE_DISPATCH, _traceACoreEvents, NULL);
        ret &= XaceRegisterCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);
        ret &= XaceRegisterCallback (XACE_AUDIT_END, _traceAuditEndEvents, NULL);

        if (!ret)
        {
            XDBG_REPLY ("failed: register one or more callbacks.\n");
            return;
        }
    }
    else
    {
        DeleteCallback (&FlushCallback, _traceFlush, NULL);
        DeleteCallback (&ReplyCallback, _traceAReply, NULL);
        XaceDeleteCallback (XACE_CORE_DISPATCH, _traceACoreEvents, NULL);
        XaceDeleteCallback (XACE_EXT_DISPATCH, _traceAExtEvents, NULL);
        XaceDeleteCallback (XACE_AUDIT_END, _traceAuditEndEvents, NULL);
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
    int  fd_check = -1;

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

    if (path[0] == '/')
        snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
    else
    {
        if (pMod->cwd)
            snprintf (fd_name, XDBG_PATH_MAX, "%s/%s", pMod->cwd, path);
        else
            snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
    }

    fd_check = open (fd_name, O_RDONLY);
    if(fd_check < 0)
        init = 0;
    else
        close (fd_check);

    xev_trace_record_fd = open (fd_name, O_CREAT|O_RDWR|O_APPEND, 0755);
    if (xev_trace_record_fd < 0)
    {
        XDBG_REPLY ("failed: open file '%s'. (%s)\n", fd_name, strerror(errno));
        return FALSE;
    }

    return TRUE;
}

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

#include <dix.h>
#define XREGISTRY
#include <registry.h>
#include <xace.h>
#include <xacestr.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <windowstr.h>

#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XIproto.h>
#include <X11/extensions/XI2proto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_xinput.h"
#include "xdbg_evlog.h"

static char *
_getMode(CARD8 mode, char *reply, int *len)
{
    const char* buf;
    char dbuf[10];

    switch (mode)
    {
        case XIGrabModeSync:  buf = "XIGrabModeSync"; break;
        case XIGrabModeAsync:  buf = "XIGrabModeAsync"; break;
        case XIGrabModeTouch:  buf = "XIGrabModeTouch"; break;
        default:  buf = dbuf; sprintf (dbuf, "%d", mode); break;
    }

    REPLY ("%s", buf);

    return reply;
}

static char *
_EvlogRequestXinput (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_GrabDevice:
        {
            xGrabDeviceReq *stuff = (xGrabDeviceReq *)req;
            REPLY (": XID(0x%x) device_ID(%d)",
                (unsigned int)stuff->grabWindow,
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" this_dev_mode");
                REPLY ("(");
                reply = _getMode(stuff->this_device_mode, reply, len);
                REPLY (")");

                REPLY (" other_dev_mode");
                REPLY ("(");
                reply = _getMode(stuff->other_devices_mode, reply, len);
                REPLY (")");

                REPLY (" time(%lums) evt_cnt(%d)  owner_events(%s)",
                    (unsigned long)stuff->time,
                    stuff->event_count,
                    stuff->ownerEvents ? "YES" : "NO");
            }

            return reply;
        }

    case X_UngrabDevice:
        {
            xUngrabDeviceReq *stuff = (xUngrabDeviceReq *)req;
            REPLY (": device_ID(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    (unsigned long)stuff->time);
            }

            return reply;
        }

    case X_GrabDeviceKey:
        {
            xGrabDeviceKeyReq *stuff = (xGrabDeviceKeyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" evt_cnt(%d) modifiers(%d) mod_dev(%d) grab_dev(%d) key(%d)",
                    stuff->event_count,
                    stuff->modifiers,
                    stuff->modifier_device,
                    stuff->grabbed_device,
                    stuff->key);

                REPLY ("\n");
                REPLY ("%67s this_dev_mode",
                    " ");

                REPLY ("(");
                reply = _getMode(stuff->this_device_mode, reply, len);
                REPLY (")");

                REPLY (" other_dev_mode");
                REPLY ("(");
                reply = _getMode(stuff->other_devices_mode, reply, len);
                REPLY (")");

                REPLY (" owner_events(%s)",
                    stuff->ownerEvents ? "YES" : "NO");
            }

            return reply;
        }

    case X_UngrabDeviceKey:
        {
            xUngrabDeviceKeyReq *stuff = (xUngrabDeviceKeyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" modifiers(%d) mod_dev(%d) grab_dev(%d) key(%d)",
                    stuff->modifiers,
                    stuff->modifier_device,
                    stuff->grabbed_device,
                    stuff->key);
            }

            return reply;
        }

    case X_GrabDeviceButton:
        {
            xGrabDeviceButtonReq *stuff = (xGrabDeviceButtonReq *)req;
            REPLY (": XID(0x%x))",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" grab_dev(%d) mod_dev(%d) evt_cnt(%d) modifiers(%d) button(%d)",
                    stuff->grabbed_device,
                    stuff->modifier_device,
                    stuff->event_count,
                    stuff->modifiers,
                    stuff->button);

                REPLY ("\n");
                REPLY ("%67s this_dev_mode",
                    " ");
                REPLY ("(");
                reply = _getMode(stuff->this_device_mode, reply, len);
                REPLY (")");

                REPLY (" other_dev_mode");
                REPLY ("(");
                reply = _getMode(stuff->other_devices_mode, reply, len);
                REPLY (")");

                REPLY (" owner_events(%s)",
                    stuff->ownerEvents ? "YES" : "NO");
            }

            return reply;
        }

    case X_UngrabDeviceButton:
        {
            xUngrabDeviceButtonReq *stuff = (xUngrabDeviceButtonReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" modifiers(%d) modDev(%d) grabDev(%d) button(%d)",
                    stuff->modifiers,
                    stuff->modifier_device,
                    stuff->grabbed_device,
                    stuff->button);
            }

            return reply;
        }

    case X_AllowDeviceEvents:
        {
            xAllowDeviceEventsReq *stuff = (xAllowDeviceEventsReq *)req;
            REPLY (": device_ID(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums) mode(%d)",
                (unsigned long)stuff->time,
                stuff->mode);
            }

            return reply;
        }

    case X_GetDeviceFocus:
        {
            xGetDeviceFocusReq *stuff = (xGetDeviceFocusReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            return reply;
        }

    case X_SetDeviceFocus:
        {
            xSetDeviceFocusReq *stuff = (xSetDeviceFocusReq *)req;
            REPLY (": XID(0x%x) dev_ID(%d)",
                (unsigned int)stuff->focus,
                stuff->device);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums) revertTo(%d)",
                (unsigned long)stuff->time,
                stuff->revertTo);
            }

            return reply;
        }

    case X_XIQueryPointer:
        {
            xXIQueryPointerReq *stuff = (xXIQueryPointerReq *)req;
            REPLY (": XID(0x%x) devID(%d)",
                stuff->win,
                stuff->deviceid);

            return reply;
        }

    case X_XIWarpPointer:
        {
            xXIWarpPointerReq *stuff = (xXIWarpPointerReq *)req;
            REPLY (": srcWIN(0x%x) dstWin(0x%x) src(%d,%d %dx%d) dst(%d,%d) device_ID(%d)",
                stuff->src_win,
                stuff->dst_win,
                stuff->src_x,
                stuff->src_y,
                stuff->src_width,
                stuff->src_height,
                stuff->dst_x,
                stuff->dst_y,
                stuff->deviceid);

            return reply;
        }

    case X_XIChangeCursor:
        {
            xXIChangeCursorReq *stuff = (xXIChangeCursorReq *)req;
            REPLY (": XID(0x%x) Cursor(0x%x) devID(%d)",
                stuff->win,
                stuff->cursor,
                stuff->deviceid);

            return reply;
        }

    case X_XIChangeHierarchy:
        {
            xXIChangeHierarchyReq *stuff = (xXIChangeHierarchyReq *)req;
            REPLY (": numChange(%d)",
                stuff->num_changes);

            return reply;
        }

    case X_XISetClientPointer:
        {
            xXISetClientPointerReq *stuff = (xXISetClientPointerReq *)req;
            REPLY (": XID(0x%x) device_ID(%d)",
                stuff->win,
                stuff->deviceid);

            return reply;
        }

    case X_XIGetClientPointer:
        {
            xXIGetClientPointerReq *stuff = (xXIGetClientPointerReq *)req;
            REPLY (": XID(0x%x)",
                stuff->win);

            return reply;
        }

    case X_XISelectEvents:
        {
            xXISelectEventsReq *stuff = (xXISelectEventsReq *)req;
            REPLY (": XID(0x%x)",
                stuff->win);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" num_masks(%d)",
                stuff->num_masks);
            }

            return reply;
        }

    case X_XIQueryVersion:
        {
            xXIQueryVersionReq *stuff = (xXIQueryVersionReq *)req;
            REPLY (": major_vesion(%d) minor_vesion(%d)",
                stuff->major_version,
                stuff->minor_version);

            return reply;
        }

    case X_XIQueryDevice:
        {
            xXIQueryDeviceReq *stuff = (xXIQueryDeviceReq *)req;
            REPLY (": device_ID(%d)",
                stuff->deviceid);

            return reply;
        }

    case X_XISetFocus:
        {
            xXISetFocusReq *stuff = (xXISetFocusReq *)req;
            REPLY (": XID(0x%x) device_ID(%d)",
                stuff->focus,
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%ums)",
                    stuff->time);
            }

            return reply;
        }

    case X_XIGetFocus:
        {
            xXIGetFocusReq *stuff = (xXIGetFocusReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            return reply;
        }

    case X_XIGrabDevice:
        {
            xXIGrabDeviceReq *stuff = (xXIGrabDeviceReq *)req;
            REPLY (": XID(0x%x) Cursor(0x%x) device_ID(%d)",
                stuff->grab_window,
                stuff->cursor,
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s grab_mode",
                    " ");
                REPLY ("(");
                reply = _getMode(stuff->grab_mode, reply, len);
                REPLY (")");

                REPLY (" paired_device_mode");
                REPLY ("(");
                reply = _getMode(stuff->paired_device_mode, reply, len);
                REPLY (")");

                REPLY (" time(%ums) owner_events(%s)",
                stuff->time,
                stuff->owner_events ? "YES" : "NO");
            }

            return reply;
        }

    case X_XIUngrabDevice:
        {
            xXIUngrabDeviceReq *stuff = (xXIUngrabDeviceReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%ums)",
                stuff->time);
            }

            return reply;
        }

    case X_XIAllowEvents:
        {
            xXIAllowEventsReq *stuff = (xXIAllowEventsReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%ums) mode(%d)",
                stuff->time,
                stuff->mode);
            }

            return reply;
        }

    case X_XIPassiveGrabDevice:
        {
            xXIPassiveGrabDeviceReq *stuff = (xXIPassiveGrabDeviceReq *)req;
            REPLY (": XID(0x%x) Cursor(0x%x) device_ID(%d)",
                stuff->grab_window,
                stuff->cursor,
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char * type;
                char dtype[10];

                switch(stuff->grab_type)
                {
                    case XIGrabtypeButton:  type = "XIGrabtypeButton"; break;
                    case XIGrabtypeKeycode:  type = "XIGrabtypeKeycode"; break;
                    case XIGrabtypeEnter:  type = "XIGrabtypeEnter"; break;
                    case XIGrabtypeFocusIn:  type = "XIGrabtypeFocusIn"; break;
                    case XIGrabtypeTouchBegin:  type = "XIGrabtypeTouchBegin"; break;
                    default:  type = dtype; sprintf (dtype, "%d", stuff->grab_type); break;
                }

                REPLY (" time(%ums) detail(%d) grab_type(%s)",
                stuff->time,
                stuff->detail,
                type);

                REPLY ("\n");
                REPLY ("%67s", " ");

                REPLY (" grab_mode");
                REPLY ("(");
                reply = _getMode(stuff->grab_mode, reply, len);
                REPLY (")");

                REPLY (" paired_device_mode");
                REPLY ("(");
                reply = _getMode(stuff->paired_device_mode, reply, len);
                REPLY (")");

                REPLY (" num_modifier(%d) owner_events(%s)",
                    stuff->num_modifiers,
                    stuff->owner_events ? "YES" : "NO");

            }

            return reply;
        }

    case X_XIPassiveUngrabDevice:
        {
            xXIPassiveUngrabDeviceReq *stuff = (xXIPassiveUngrabDeviceReq *)req;
            REPLY (": XID(0x%x) device_ID(%d)",
                stuff->grab_window,
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char * type;
                char dtype[10];

                switch(stuff->grab_type)
                {
                    case XIGrabtypeButton:  type = "XIGrabtypeButton"; break;
                    case XIGrabtypeKeycode:  type = "XIGrabtypeKeycode"; break;
                    case XIGrabtypeEnter:  type = "XIGrabtypeEnter"; break;
                    case XIGrabtypeFocusIn:  type = "XIGrabtypeFocusIn"; break;
                    case XIGrabtypeTouchBegin:  type = "XIGrabtypeTouchBegin"; break;
                    default:  type = dtype; sprintf (dtype, "%d", stuff->grab_type); break;
                }

                REPLY (" detail(%d) grab_type(%s) num_modifiers(%d)",
                stuff->detail,
                type,
                stuff->num_modifiers);
            }

            return reply;
        }

    case X_XIListProperties:
        {
            xXIListPropertiesReq *stuff = (xXIListPropertiesReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            return reply;
        }

    case X_XIChangeProperty:
        {
            xXIChangePropertyReq *stuff = (xXIChangePropertyReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);
            REPLY (" Type");
            reply = xDbgGetAtom(stuff->type, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char * mode;
                char dmode[10];

                switch(stuff->mode)
                {
                    case XIPropModeReplace:  mode = "XIPropModeReplace"; break;
                    case XIPropModePrepend:  mode = "XIPropModePrepend"; break;
                    case XIPropModeAppend:  mode = "XIPropModeAppend"; break;
                    default:  mode = dmode; sprintf (dmode, "%d", stuff->mode); break;
                }

                REPLY (" mode(%s) format(%d) num_items(%d)",
                mode,
                stuff->format,
                stuff->num_items);
            }

            return reply;
        }

    case X_XIDeleteProperty:
        {
            xXIDeletePropertyReq *stuff = (xXIDeletePropertyReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            return reply;
        }

    case X_XIGetProperty:
        {
            xXIGetPropertyReq *stuff = (xXIGetPropertyReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);
            REPLY (" Type");
            reply = xDbgGetAtom(stuff->type, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" delete(%s) offset(%u) length(%u)",
                stuff->delete ? "YES" : "NO",
                stuff->offset,
                stuff->len);
            }

            return reply;
        }

    case X_XIGetSelectedEvents:
        {
            xXIGetSelectedEventsReq *stuff = (xXIGetSelectedEventsReq *)req;
            REPLY (": XID(0x%x)",
                stuff->win);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventXinput (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case XI_DeviceValuator:
        {
            deviceValuator *stuff = (deviceValuator *) evt;
            REPLY (": device_ID(%d) numValuator(%d) fstnumValuator(%d)",
                stuff->deviceid,
                stuff->num_valuators,
                stuff->first_valuator);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                int i;

                REPLY (" sequence_num(%d) device_state(0x%x)\n",
                    stuff->sequenceNumber,
                    stuff->device_state);

                REPLY ("%67s", " ");
                for (i = 0 ; i < stuff->num_valuators ; i++)
                {
                    REPLY (" valuator%d(%ld)",
                        i,
                        (long int)*(&stuff->valuator0 + i));
                }
            }

            return reply;
        }

    case XI_DeviceKeyPress:
        {
            XDeviceKeyPressedEvent *stuff = (XDeviceKeyPressedEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) key_code(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->keycode,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceKeyRelease:
        {
            XDeviceKeyReleasedEvent *stuff = (XDeviceKeyReleasedEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) key_code(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->keycode,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceButtonPress:
        {
            XDeviceButtonPressedEvent *stuff = (XDeviceButtonPressedEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) button(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->button,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceButtonRelease:
        {
            XDeviceButtonReleasedEvent *stuff = (XDeviceButtonReleasedEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) button(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->button,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceMotionNotify:
        {
            XDeviceMotionEvent *stuff = (XDeviceMotionEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) is_hint(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->is_hint,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceFocusIn:
        {
            XDeviceFocusInEvent *stuff = (XDeviceFocusInEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *mode, *detail;
                char dmode[10], ddetail[10];

                switch(stuff->mode)
                {
                    case NotifyNormal:  mode = "NotifyNormal"; break;
                    case NotifyGrab:  mode = "NotifyGrab"; break;
                    case NotifyUngrab:  mode = "NotifyUngrab"; break;
                    case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
                    default:  mode = dmode; sprintf (dmode, "%d", stuff->mode); break;
                }

                switch(stuff->detail)
                {
                    case NotifyAncestor:  detail = "NotifyAncestor"; break;
                    case NotifyVirtual:  detail = "NotifyVirtual"; break;
                    case NotifyInferior:  detail = "NotifyInferior"; break;
                    case NotifyNonlinear:  detail = "NotifyNonlinear"; break;
                    case NotifyNonlinearVirtual:  detail = "NotifyNonlinearVirtual"; break;
                    case NotifyPointer:  detail = "NotifyPointer"; break;
                    case NotifyPointerRoot:  detail = "NotifyPointerRoot"; break;
                    case NotifyDetailNone:  detail = "NotifyDetailNone"; break;
                    default:  detail = ddetail; sprintf (ddetail, "%d", stuff->detail); break;
                }

                REPLY (" mode(%s) detail(%s) time(%lums)",
                    mode,
                    detail,
                    (unsigned long)stuff->time);
            }

            return reply;
        }

    case XI_DeviceFocusOut:
        {
            XDeviceFocusOutEvent *stuff = (XDeviceFocusOutEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *mode, *detail;
                char dmode[10], ddetail[10];

                switch(stuff->mode)
                {
                    case NotifyNormal:  mode = "NotifyNormal"; break;
                    case NotifyGrab:  mode = "NotifyGrab"; break;
                    case NotifyUngrab:  mode = "NotifyUngrab"; break;
                    case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
                    default:  mode = dmode; sprintf (dmode, "%d", stuff->mode); break;
                }

                switch(stuff->detail)
                {
                    case NotifyAncestor:  detail = "NotifyAncestor"; break;
                    case NotifyVirtual:  detail = "NotifyVirtual"; break;
                    case NotifyInferior:  detail = "NotifyInferior"; break;
                    case NotifyNonlinear:  detail = "NotifyNonlinear"; break;
                    case NotifyNonlinearVirtual:  detail = "NotifyNonlinearVirtual"; break;
                    case NotifyPointer:  detail = "NotifyPointer"; break;
                    case NotifyPointerRoot:  detail = "NotifyPointerRoot"; break;
                    case NotifyDetailNone:  detail = "NotifyDetailNone"; break;
                    default:  detail = ddetail; sprintf (ddetail, "%d", stuff->detail); break;
                }

                REPLY (" mode(%s) detail(%s) time(%lums)",
                    mode,
                    detail,
                    (unsigned long)stuff->time);
            }

            return reply;
        }

    case XI_ProximityIn:
        {
            XProximityInEvent *stuff = (XProximityInEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_ProximityOut:
        {
            XProximityOutEvent *stuff = (XProximityOutEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x %d,%d) Root(0x%x %d,%d) subWindow(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                (unsigned int)stuff->root,
                stuff->x_root,
                stuff->y_root,
                (unsigned int)stuff->subwindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%d) same_screen(%s) device_state(%d) first_axis(%d)",
                    " ",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->same_screen ? "YES" : "NO",
                    stuff->device_state,
                    stuff->first_axis);
            }

            return reply;
        }

    case XI_DeviceStateNotify:
        {
            XDeviceStateNotifyEvent *stuff = (XDeviceStateNotifyEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums) num_classes(%d)",
                    (unsigned long)stuff->time,
                    stuff->num_classes);
            }

            return reply;
        }

    case XI_DeviceMappingNotify:
        {
            XDeviceMappingEvent *stuff = (XDeviceMappingEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *request;
                char drequest[10];

                switch(stuff->request)
                {
                    case MappingModifier:  request = "MappingModifier"; break;
                    case MappingKeyboard:  request = "MappingKeyboard"; break;
                    case MappingPointer:  request = "MappingPointer"; break;
                    default:  request = drequest; sprintf (drequest, "%d", stuff->request); break;
                }

                REPLY (" time(%lums) request(%s) first_keycode(%d) count(%d)",
                    (unsigned long)stuff->time,
                    request,
                    stuff->first_keycode,
                    stuff->count);
            }

            return reply;
        }

    case XI_ChangeDeviceNotify:
        {
            XChangeDeviceNotifyEvent *stuff = (XChangeDeviceNotifyEvent *) evt;
            REPLY (": XID(0x%x) Window(0x%x)",
                (unsigned int)stuff->deviceid,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *request;
                char drequest[10];

                switch(stuff->request)
                {
                    case NewPointer:  request = "NewPointer"; break;
                    case NewKeyboard:  request = "NewKeyboard"; break;
                    default:  request = drequest; sprintf (drequest, "%d", stuff->request); break;
                }

                REPLY (" time(%lums) request(%s)",
                    (unsigned long)stuff->time,
                    request);
            }

            return reply;
        }

    case XI_DeviceKeystateNotify:
        {
            deviceKeyStateNotify *stuff = (deviceKeyStateNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" sequence_num(%d)",
                    stuff->sequenceNumber);
            }

            return reply;
        }

    case XI_DeviceButtonstateNotify:
        {
            deviceButtonStateNotify *stuff = (deviceButtonStateNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" sequence_num(%d)",
                    stuff->sequenceNumber);
            }

            return reply;
        }

    case XI_DevicePresenceNotify:
        {
            devicePresenceNotify *stuff = (devicePresenceNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums) device_change(%d) control(%d) sequence_num(%d)",
                    (unsigned long)stuff->time,
                    stuff->devchange,
                    stuff->control,
                    stuff->sequenceNumber);
            }

            return reply;
        }

    case XI_DevicePropertyNotify:
        {
            devicePropertyNotify *stuff = (devicePropertyNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            REPLY (" Atom");
            reply = xDbgGetAtom(stuff->atom, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums) state(%d) sequence_num(%d)",
                    (unsigned long)stuff->time,
                    stuff->state,
                    stuff->sequenceNumber);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyXinput (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_ListInputDevices:
        {
            if (evinfo->rep.isStart)
            {
                xListInputDevicesReply *stuff = (xListInputDevicesReply *) rep;
                REPLY (": nDevices(%d)",
                    stuff->ndevices);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_GrabDevice:
        {
            if (evinfo->rep.isStart)
            {
                xGrabDeviceReply *stuff = (xGrabDeviceReply *) rep;
                REPLY (": status(%d)",
                    stuff->status);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_GetDeviceFocus:
        {
            if (evinfo->rep.isStart)
            {
                xGetDeviceFocusReply *stuff = (xGetDeviceFocusReply *) rep;
                REPLY (": XID(0x%x) Time(0x%x)",
                    (unsigned int)stuff->focus,
                    (unsigned int)stuff->time);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIQueryVersion:
        {
            if (evinfo->rep.isStart)
            {
                xXIQueryVersionReply *stuff = (xXIQueryVersionReply *) rep;
                REPLY (": majorVersion(%d) minorVersion(%d)",
                    stuff->major_version,
                    stuff->minor_version);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIQueryDevice:
        {
            if (evinfo->rep.isStart)
            {
                xXIQueryDeviceReply *stuff = (xXIQueryDeviceReply *) rep;
                REPLY (": numDevices(%d)",
                    stuff->num_devices);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIGetSelectedEvents:
        {
            if (evinfo->rep.isStart)
            {
                xXIGetSelectedEventsReply *stuff = (xXIGetSelectedEventsReply *) rep;
                REPLY (": numMasks(%d)",
                    stuff->num_masks);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIQueryPointer:
        {
            if (evinfo->rep.isStart)
            {
                xXIQueryPointerReply *stuff = (xXIQueryPointerReply *) rep;
                REPLY (": XID(0x%x) Child(0x%x) root(%d,%d) win(%d,%d)",
                    stuff->root,
                    stuff->child,
                    stuff->root_x,
                    stuff->root_y,
                    stuff->win_x,
                    stuff->win_y);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIGetClientPointer:
        {
            if (evinfo->rep.isStart)
            {
                xXIGetClientPointerReply *stuff = (xXIGetClientPointerReply *) rep;
                REPLY (": Set(%s) deviceid(%d)",
                    (stuff->set) ? "true":"false",
                    stuff->deviceid);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIGetFocus:
        {
            if (evinfo->rep.isStart)
            {
                xXIGetFocusReply *stuff = (xXIGetFocusReply *) rep;
                REPLY (": XID(0x%x)",
                    stuff->focus);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIGrabDevice:
        {
            if (evinfo->rep.isStart)
            {
                xXIGrabDeviceReply *stuff = (xXIGrabDeviceReply *) rep;
                REPLY (": status(%d)",
                    stuff->status);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIPassiveGrabDevice:
        {
            if (evinfo->rep.isStart)
            {
                xXIPassiveGrabDeviceReply *stuff = (xXIPassiveGrabDeviceReply *) rep;
                REPLY (": numModifiers(%d)",
                    stuff->num_modifiers);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_XIListProperties:
        {
            if (evinfo->rep.isStart)
            {
                xXIListPropertiesReply *stuff = (xXIListPropertiesReply *) rep;
                REPLY (": numProperties(%d)",
                    stuff->num_properties);
            }
            else
            {
                Atom *stuff = (Atom *)rep;
                int i;

                REPLY ("Properties(");
                for (i = 0 ; i < evinfo->rep.size / sizeof(Atom) ; i ++)
                {
                    reply = xDbgGetAtom(stuff[i], evinfo, reply, len);
                    if(i != evinfo->rep.size / sizeof(Atom) - 1)
                        REPLY (", ");
                }
                REPLY (")");
            }

            return reply;
        }

    case X_XIGetProperty:
        {
            if (evinfo->rep.isStart)
            {
                xXIGetPropertyReply *stuff = (xXIGetPropertyReply *) rep;

                REPLY (": Type");
                xDbgGetAtom(stuff->type, evinfo, reply, len);

                REPLY (" numItems(%d) format(%d)",
                    stuff->num_items,
                    stuff->format);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

void
xDbgEvlogXinputGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXinput;
    extinfo->evt_func = _EvlogEventXinput;
    extinfo->rep_func = _EvlogReplyXinput;
#else
    ExtensionEntry *xext = CheckExtension (INAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXinput;
    extinfo->evt_func = _EvlogEventXinput;
    extinfo->rep_func = _EvlogReplyXinput;
#endif
}

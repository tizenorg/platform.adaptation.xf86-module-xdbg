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

static char *
_EvlogRequestXinput (xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->data)
    {
    case X_GrabDevice:
        {
            xGrabDeviceReq *stuff = (xGrabDeviceReq *)req;
            REPLY (": XID(0x%lx) Time(0x%lx) evtCnt(%d) thisMode(%d) otherMode(%d) devID(%d)",
                stuff->grabWindow,
                stuff->time,
                stuff->event_count,
                stuff->this_device_mode,
                stuff->other_devices_mode,
                stuff->deviceid);

            return reply;
        }

    case X_UngrabDevice:
        {
            xUngrabDeviceReq *stuff = (xUngrabDeviceReq *)req;
            REPLY (": Time(0x%lx) devID(%d)",
                stuff->time,
                stuff->deviceid);

            return reply;
        }

    case X_GrabDeviceKey:
        {
            xGrabDeviceKeyReq *stuff = (xGrabDeviceKeyReq *)req;
            REPLY (": XID(0x%lx) evtCnt(%d) modifiers(%d) modDev(%d) grabDev(%d) key(%d)",
                stuff->grabWindow,
                stuff->event_count,
                stuff->modifiers,
                stuff->modifier_device,
                stuff->grabbed_device,
                stuff->key);

            return reply;
        }

    case X_UngrabDeviceKey:
        {
            xUngrabDeviceKeyReq *stuff = (xUngrabDeviceKeyReq *)req;
            REPLY (": XID(0x%lx) modifiers(%d) modDev(%d) key(%d) grabDev(%d)",
                stuff->grabWindow,
                stuff->modifiers,
                stuff->modifier_device,
                stuff->key,
                stuff->grabbed_device);

            return reply;
        }

    case X_GrabDeviceButton:
        {
            xGrabDeviceButtonReq *stuff = (xGrabDeviceButtonReq *)req;
            REPLY (": XID(0x%lx) evtCnt(%d) modifiers(%d) modDev(%d) grabDev(%d) button(%d)",
                stuff->grabWindow,
                stuff->event_count,
                stuff->modifiers,
                stuff->modifier_device,
                stuff->grabbed_device,
                stuff->button);

            return reply;
        }

    case X_UngrabDeviceButton:
        {
            xUngrabDeviceButtonReq *stuff = (xUngrabDeviceButtonReq *)req;
            REPLY (": XID(0x%lx) modifiers(%d) modDev(%d) grabDev(%d) button(%d)",
                stuff->grabWindow,
                stuff->modifiers,
                stuff->modifier_device,
                stuff->grabbed_device,
                stuff->button);

            return reply;
        }

    case X_AllowDeviceEvents:
        {
            xAllowDeviceEventsReq *stuff = (xAllowDeviceEventsReq *)req;
            REPLY (": Time(0x%lx) devID(%d) mode(%d)",
                stuff->time,
                stuff->deviceid,
                stuff->mode);

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
            REPLY (": XID(0x%lx) Time(0x%lx) revertTo(%d) devID(%d)",
                stuff->focus,
                stuff->time,
                stuff->revertTo,
                stuff->device);

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
            REPLY (": srcWIN(0x%x) dstWin(0x%x) src(%d,%d %dx%d) dst(%d,%d) devID(%d)",
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
            REPLY (": XID(0x%x) devID(%d)",
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

            return reply;
        }

    case X_XIQueryVersion:
        {
            xXIQueryVersionReq *stuff = (xXIQueryVersionReq *)req;
            REPLY (": majorVesion(%d) minorVesion(%d)",
                stuff->major_version,
                stuff->minor_version);

            return reply;
        }

    case X_XIQueryDevice:
        {
            xXIQueryDeviceReq *stuff = (xXIQueryDeviceReq *)req;
            REPLY (": devID(%d)",
                stuff->deviceid);

            return reply;
        }

    case X_XISetFocus:
        {
            xXISetFocusReq *stuff = (xXISetFocusReq *)req;
            REPLY (": XID(0x%x) Time(0x%x) devID(%d)",
                stuff->focus,
                stuff->time,
                stuff->deviceid);

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
            REPLY (": XID(0x%x) Time(0x%x) Cursor(0x%x) grabMode(%d) pairDevMode(%d) devID(%d)",
                stuff->grab_window,
                stuff->time,
                stuff->cursor,
                stuff->grab_mode,
                stuff->paired_device_mode,
                stuff->deviceid);

            return reply;
        }

    case X_XIUngrabDevice:
        {
            xXIUngrabDeviceReq *stuff = (xXIUngrabDeviceReq *)req;
            REPLY (": Time(0x%x) devID(%d)",
                stuff->time,
                stuff->deviceid);

            return reply;
        }

    case X_XIAllowEvents:
        {
            xXIAllowEventsReq *stuff = (xXIAllowEventsReq *)req;
            REPLY (": Time(0x%x) devID(%d) mode(%d)",
                stuff->time,
                stuff->deviceid,
                stuff->mode);

            return reply;
        }

    case X_XIPassiveGrabDevice:
        {
            xXIPassiveGrabDeviceReq *stuff = (xXIPassiveGrabDeviceReq *)req;
            REPLY (": XID(0x%x) Time(0x%x) Cursor(0x%x) detail(0x%x) grabType(%d) grabMode(%d) pairDevMode(%d)",
                stuff->grab_window,
                stuff->time,
                stuff->cursor,
                stuff->detail,
                stuff->grab_type,
                stuff->grab_mode,
                stuff->paired_device_mode);

            return reply;
        }

    case X_XIPassiveUngrabDevice:
        {
            xXIPassiveUngrabDeviceReq *stuff = (xXIPassiveUngrabDeviceReq *)req;
            REPLY (": XID(0x%x) detail(0x%x) grabType(%d) devID(%d)",
                stuff->grab_window,
                stuff->detail,
                stuff->grab_type,
                stuff->deviceid);

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
            REPLY (": devID(%d) mode(%d) format(%d) property(0x%x) type(0x%x) numItems(%d)",
                stuff->deviceid,
                stuff->mode,
                stuff->format,
                stuff->property,
                stuff->type,
                stuff->num_items);

            return reply;
        }

    case X_XIDeleteProperty:
        {
            xXIDeletePropertyReq *stuff = (xXIDeletePropertyReq *)req;
            REPLY (": devID(%d) property(0x%x)",
                stuff->deviceid,
                stuff->property);

            return reply;
        }

    case X_XIGetProperty:
        {
            xXIGetPropertyReq *stuff = (xXIGetPropertyReq *)req;
            REPLY (": devID(%d) property(0x%x) type(0x%x)",
                stuff->deviceid,
                stuff->property,
                stuff->type);

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
_EvlogEventXinput (xEvent *evt, int first_base, char *reply, int *len)
{
    xEvent *stuff = evt;

    switch ((stuff->u.u.type & 0x7F) - first_base)
    {
    case XI_DeviceValuator:
        {
            deviceValuator *stuff = (deviceValuator *) evt;
            REPLY (": numValuator(%d) fstnumValuator(%d)",
                stuff->num_valuators,
                stuff->first_valuator);

            return reply;
        }

    case XI_DeviceKeyPress:
        {
            XDeviceKeyPressedEvent *stuff = (XDeviceKeyPressedEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceKeyRelease:
        {
            XDeviceKeyReleasedEvent *stuff = (XDeviceKeyReleasedEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceButtonPress:
        {
            XDeviceButtonPressedEvent *stuff = (XDeviceButtonPressedEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceButtonRelease:
        {
            XDeviceButtonReleasedEvent *stuff = (XDeviceButtonReleasedEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceMotionNotify:
        {
            XDeviceMotionEvent *stuff = (XDeviceMotionEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceFocusIn:
        {
            XDeviceFocusInEvent *stuff = (XDeviceFocusInEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx)",
                stuff->deviceid,
                stuff->window);

            return reply;
        }

    case XI_DeviceFocusOut:
        {
            XDeviceFocusOutEvent *stuff = (XDeviceFocusOutEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx)",
                stuff->deviceid,
                stuff->window);

            return reply;
        }

    case XI_ProximityIn:
        {
            XProximityInEvent *stuff = (XProximityInEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_ProximityOut:
        {
            XProximityOutEvent *stuff = (XProximityOutEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx %d,%d) Root(0x%lx %d,%d) subWindow(0x%lx)",
                stuff->deviceid,
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->root,
                stuff->x_root,
                stuff->y_root,
                stuff->subwindow);

            return reply;
        }

    case XI_DeviceStateNotify:
        {
            XDeviceStateNotifyEvent *stuff = (XDeviceStateNotifyEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx)",
                stuff->deviceid,
                stuff->window);

            return reply;
        }

    case XI_DeviceMappingNotify:
        {
            XDeviceMappingEvent *stuff = (XDeviceMappingEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx)",
                stuff->deviceid,
                stuff->window);

            return reply;
        }

    case XI_ChangeDeviceNotify:
        {
            XChangeDeviceNotifyEvent *stuff = (XChangeDeviceNotifyEvent *) evt;
            REPLY (": XID(0x%lx) Window(0x%lx)",
                stuff->deviceid,
                stuff->window);

            return reply;
        }

    case XI_DeviceKeystateNotify:
        {
            deviceKeyStateNotify *stuff = (deviceKeyStateNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            return reply;
        }

    case XI_DeviceButtonstateNotify:
        {
            deviceButtonStateNotify *stuff = (deviceButtonStateNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            return reply;
        }

    case XI_DevicePresenceNotify:
        {
            devicePresenceNotify *stuff = (devicePresenceNotify *) evt;
            REPLY (": deviceid(%d)",
                stuff->deviceid);

            return reply;
        }

    case XI_DevicePropertyNotify:
        {
            devicePropertyNotify *stuff = (devicePropertyNotify *) evt;
            REPLY (": XID(0x%lx)",
                stuff->atom);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

void
xDbgEvlogXinputGetBase (void *dpy, ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    Display *d = (Display*)dpy;

    RETURN_IF_FAIL (d != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    if (!XQueryExtension(d, INAME, &extinfo->opcode, &extinfo->evt_base, &extinfo->err_base))
    {
        XDBG_LOG ("no Xinput extension. \n");
        return;
    }
    extinfo->req_func = _EvlogRequestXinput;
    extinfo->evt_func = _EvlogEventXinput;
#else
    ExtensionEntry *xext = CheckExtension (INAME);
    RETURN_IF_FAIL (xext != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXinput;
    extinfo->evt_func = _EvlogEventXinput;
#endif
}

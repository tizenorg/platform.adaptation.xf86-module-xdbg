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
#include <X11/extensions/gesture.h>
#include <X11/extensions/gestureproto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_gesture.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestGesture(EvlogInfo *evinfo, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_GestureSelectEvents:
        {
            xGestureSelectEventsReq *stuff = (xGestureSelectEventsReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_GestureGetSelectedEvents:
        {
            xGestureGetSelectedEventsReq *stuff = (xGestureGetSelectedEventsReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_GestureGrabEvent:
        {
            xGestureGrabEventReq *stuff = (xGestureGrabEventReq *)req;
            REPLY (": XID(0x%lx) EventType(0x%lx), Num_finger(%d)",
                stuff->window,
                stuff->eventType,
                stuff->num_finger);

            return reply;
        }

    case X_GestureUngrabEvent:
        {
            xGestureUngrabEventReq *stuff = (xGestureUngrabEventReq *)req;
            REPLY (": XID(0x%lx) EventType(0x%lx), Num_finger(%d)",
                stuff->window,
                stuff->eventType,
                stuff->num_finger);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventGesture (EvlogInfo *evinfo, int first_base, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case GestureNotifyFlick:
        {
            xGestureNotifyFlickEvent *stuff = (xGestureNotifyFlickEvent *) evt;
            REPLY (": XID(0x%lx) Direction(%d) Distance(%d) Duration(%ldms) Angle(0x%lx)",
                stuff->window,
                stuff->direction,
                stuff->distance,
                stuff->duration,
                stuff->angle);

            return reply;
        }

    case GestureNotifyPan:
        {
            xGestureNotifyPanEvent *stuff = (xGestureNotifyPanEvent *) evt;
            REPLY (": XID(0x%lx) Direction(%d) Distance(%d) Duration(%ldms) coord(%d,%d)",
                stuff->window,
                stuff->direction,
                stuff->distance,
                stuff->duration,
                stuff->dx,
                stuff->dy);

            return reply;
        }

    case GestureNotifyPinchRotation:
        {
            xGestureNotifyPinchRotationEvent *stuff = (xGestureNotifyPinchRotationEvent *) evt;
            REPLY (": XID(0x%lx) Distance(%d) Coord(%d,%d) Zoom(0x%lx) Angle(0x%lx)",
                stuff->window,
                stuff->distance,
                stuff->cx,
                stuff->cy,
                stuff->zoom,
                stuff->angle);

            return reply;
        }

    case GestureNotifyTap:
        {
            xGestureNotifyTapEvent *stuff = (xGestureNotifyTapEvent *) evt;
            REPLY (": XID(0x%lx) Coord(%d,%d) tapRepeat(%d) Interval(%ldms)",
                stuff->window,
                stuff->cx,
                stuff->cy,
                stuff->tap_repeat,
                stuff->interval);

            return reply;
        }

    case GestureNotifyTapNHold:
        {
            xGestureNotifyTapNHoldEvent *stuff = (xGestureNotifyTapNHoldEvent *) evt;
            REPLY (": XID(0x%lx) Coord(%d,%d) Interval(%ldms) Holdtime(%ldms)",
                stuff->window,
                stuff->cx,
                stuff->cy,
                stuff->interval,
                stuff->holdtime);

            return reply;
        }

    case GestureNotifyHold:
        {
            xGestureNotifyHoldEvent *stuff = (xGestureNotifyHoldEvent *) evt;
            REPLY (": XID(0x%lx) Coord(%d,%d) Holdtime(%ldms)",
                stuff->window,
                stuff->cx,
                stuff->cy,
                stuff->holdtime);

            return reply;
        }

    case GestureNotifyGroup:
        {
            xGestureNotifyGroupEvent *stuff = (xGestureNotifyGroupEvent *) evt;
            REPLY (": XID(0x%lx) groupID(%d) groupNum(%d)",
                stuff->window,
                stuff->groupid,
                stuff->num_group);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyGesture (EvlogInfo *evinfo, char *reply, int *len)
{
#if 0
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {

    default:
            break;
    }
#endif
    return reply;
}

void
xDbgEvlogGestureGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestGesture;
    extinfo->evt_func = _EvlogEventGesture;
    extinfo->rep_func = _EvlogReplyGesture;
#else
    ExtensionEntry *xext = CheckExtension (GESTURE_EXT_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestGesture;
    extinfo->evt_func = _EvlogEventGesture;
    extinfo->rep_func = _EvlogReplyGesture;
#endif
}

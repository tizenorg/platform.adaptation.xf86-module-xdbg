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
#include <X11/extensions/gestureproto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_gesture.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestGesture(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_GestureSelectEvents:
        {
            xGestureSelectEventsReq *stuff = (xGestureSelectEventsReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" mask(0x%x)",
                    (unsigned int)stuff->mask);
            }

            return reply;
        }

    case X_GestureGetSelectedEvents:
        {
            xGestureGetSelectedEventsReq *stuff = (xGestureGetSelectedEventsReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    case X_GestureGrabEvent:
        {
            xGestureGrabEventReq *stuff = (xGestureGrabEventReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *event_type;
                char devent_type[10];

                switch (stuff->eventType)
                {
                    case GestureNotifyFlick:  event_type = "GestureNotifyFlick"; break;
                    case GestureNotifyPan:  event_type = "GestureNotifyPan"; break;
                    case GestureNotifyPinchRotation:  event_type = "GestureNotifyPinchRotation"; break;
                    case GestureNotifyTap:  event_type = "GestureNotifyTap"; break;
                    case GestureNotifyTapNHold:  event_type = "GestureNotifyTapNHold"; break;
                    case GestureNotifyHold:  event_type = "GestureNotifyHold"; break;
                    case GestureNotifyGroup:  event_type = "GestureNotifyGroup"; break;
                    default:  event_type = devent_type; sprintf (devent_type, "%ld", (long int)stuff->eventType); break;
                }

                REPLY (" event_type(%s) num_finger(%d) time(%lums)",
                    event_type,
                    stuff->num_finger,
                    (unsigned long)stuff->time);
            }

            return reply;
        }

    case X_GestureUngrabEvent:
        {
            xGestureUngrabEventReq *stuff = (xGestureUngrabEventReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *event_type;
                char devent_type[10];

                switch (stuff->eventType)
                {
                    case GestureNotifyFlick:  event_type = "GestureNotifyFlick"; break;
                    case GestureNotifyPan:  event_type = "GestureNotifyPan"; break;
                    case GestureNotifyPinchRotation:  event_type = "GestureNotifyPinchRotation"; break;
                    case GestureNotifyTap:  event_type = "GestureNotifyTap"; break;
                    case GestureNotifyTapNHold:  event_type = "GestureNotifyTapNHold"; break;
                    case GestureNotifyHold:  event_type = "GestureNotifyHold"; break;
                    case GestureNotifyGroup:  event_type = "GestureNotifyGroup"; break;
                    default:  event_type = devent_type; sprintf (devent_type, "%ld", (long int)stuff->eventType); break;
                }

                REPLY (" event_type(%s) num_finger(%d) time(%lums)",
                    event_type,
                    stuff->num_finger,
                    (unsigned long)stuff->time);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventGesture (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case GestureNotifyFlick:
        {
            xGestureNotifyFlickEvent *stuff = (xGestureNotifyFlickEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) time(%lums) num_finger(%d) direction(%d) distance(%d)",
                    kind,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->direction,
                    stuff->distance);

                REPLY ("\n");
                REPLY ("%67s duration(%lums) angle(%ld)",
                    " ",
                    (unsigned long)stuff->duration,
                    (long int)stuff->angle);
            }

            return reply;
        }

    case GestureNotifyPan:
        {
            xGestureNotifyPanEvent *stuff = (xGestureNotifyPanEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) sequence_num(%d) time(%lums) num_finger(%d) direction(%d) ",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->direction);

                REPLY ("\n");
                REPLY ("%67s distance(%d) duration(%ldms) coord(%d,%d)",
                    " ",
                    stuff->distance,
                    (long int)stuff->duration,
                    stuff->dx,
                    stuff->dy);
            }

            return reply;
        }

    case GestureNotifyPinchRotation:
        {
            xGestureNotifyPinchRotationEvent *stuff = (xGestureNotifyPinchRotationEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) sequence_num(%d) time(%lums) num_finger(%d) distance(%d)",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->distance);

                REPLY ("\n");
                REPLY ("%67s coord(%d,%d) zoom(%ld) angle(%ld)",
                    " ",
                    stuff->cx,
                    stuff->cy,
                    (long int)stuff->zoom,
                    (long int)stuff->angle);
            }

            return reply;
        }

    case GestureNotifyTap:
        {
            xGestureNotifyTapEvent *stuff = (xGestureNotifyTapEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) sequence_num(%d) time(%lums) num_finger(%d) coord(%d,%d)",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->cx,
                    stuff->cy);

                REPLY ("\n");
                REPLY ("%67s tap_repeat(%d) interval(%lums)",
                    " ",
                    stuff->tap_repeat,
                    (unsigned long)stuff->interval);
            }

            return reply;
        }

    case GestureNotifyTapNHold:
        {
            xGestureNotifyTapNHoldEvent *stuff = (xGestureNotifyTapNHoldEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) sequence_num(%d) time(%lums) num_finger(%d) coord(%d,%d)",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->cx,
                    stuff->cy);

                REPLY ("\n");
                REPLY ("%67s interval(%lums) hold_time(%lums)",
                    " ",
                    (unsigned long)stuff->interval,
                    (unsigned long)stuff->holdtime);
            }

            return reply;
        }

    case GestureNotifyHold:
        {
            xGestureNotifyHoldEvent *stuff = (xGestureNotifyHoldEvent *) evt;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureEnd:  kind = "GestureEnd"; break;
                    case GestureBegin:  kind = "GestureBegin"; break;
                    case GestureUpdate:  kind = "GestureUpdate"; break;
                    case GestureDone:  kind = "GestureDone"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s) sequence_num(%d) time(%lums) num_finger(%d) coord(%d,%d) ",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->num_finger,
                    stuff->cx,
                    stuff->cy);

                REPLY ("\n");
                REPLY ("%67s hold_time(%lums)",
                    " ",
                    (unsigned long)stuff->holdtime);
            }

            return reply;
        }

    case GestureNotifyGroup:
        {
            xGestureNotifyGroupEvent *stuff = (xGestureNotifyGroupEvent *) evt;
            REPLY (": XID(0x%x) groupID(%d) groupNum(%d)",
                (unsigned int)stuff->window,
                stuff->groupid,
                stuff->num_group);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case GestureGroupRemoved:  kind = "GestureGroupRemoved"; break;
                    case GestureGroupAdded:  kind = "GestureGroupAdded"; break;
                    case GestureGroupCurrent:  kind = "GestureGroupCurrent"; break;
                    default:  kind = dkind; sprintf (dkind, "%d", stuff->kind); break;
                }

                REPLY ("\n");
                REPLY ("%67s kind(%s) sequence_num(%d) time(%lums) group_id(%d) num_group(%d)",
                    " ",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->groupid,
                    stuff->num_group);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyGesture (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
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

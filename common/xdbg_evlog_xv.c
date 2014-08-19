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

#include <X11/extensions/Xvlib.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_xv.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestXv(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case xv_GrabPort:
        {
            xvGrabPortReq *stuff = (xvGrabPortReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->port);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    stuff->time);
            }

            return reply;
        }

    case xv_UngrabPort:
        {
            xvUngrabPortReq *stuff = (xvUngrabPortReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->port);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    stuff->time);
            }

            return reply;
        }

    case xv_PutStill:
        {
            xvPutStillReq *stuff = (xvPutStillReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) Vid(%d,%d %dx%d) Drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->vid_x,
                stuff->vid_y,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_GetStill:
        {
            xvGetStillReq *stuff = (xvGetStillReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) Vid(%d,%d %dx%d) Drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->vid_x,
                stuff->vid_y,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_PutVideo:
        {
            xvPutVideoReq *stuff = (xvPutVideoReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) Vid(%d,%d %dx%d) Drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->vid_x,
                stuff->vid_y,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_GetVideo:
        {
            xvGetVideoReq *stuff = (xvGetVideoReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) Vid(%d,%d %dx%d) Drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->vid_x,
                stuff->vid_y,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_StopVideo:
        {
            xvStopVideoReq *stuff = (xvStopVideoReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable);

            return reply;
        }

    case xv_SelectVideoNotify:
        {
            xvSelectVideoNotifyReq *stuff = (xvSelectVideoNotifyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("  On/Off(%s)",
                    stuff->onoff ? "ON" : "OFF");
            }

            return reply;
        }

    case xv_SelectPortNotify:
        {
            xvSelectPortNotifyReq *stuff = (xvSelectPortNotifyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->port);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("  On/Off(%s)",
                    stuff->onoff ? "ON" : "OFF");
            }

            return reply;
        }

    case xv_QueryBestSize:
        {
            xvQueryBestSizeReq *stuff = (xvQueryBestSizeReq *)req;
            REPLY (": XID(0x%x) VidSize(%dx%d) DrwSize(%dx%d)",
                (unsigned int)stuff->port,
                stuff->vid_w,
                stuff->vid_h,
                stuff->drw_w,
                stuff->drw_h);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("  motion(%d)",
                    stuff->motion);
            }

            return reply;
        }

    case xv_SetPortAttribute:
        {
            xvSetPortAttributeReq *stuff = (xvSetPortAttributeReq *)req;
            REPLY (": XID(0x%x) ",
                (unsigned int)stuff->port);

            REPLY (" Attribute");
            reply = xDbgGetAtom(stuff->attribute, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("  value(%ld)",
                    stuff->value);
            }

            return reply;
        }

    case xv_GetPortAttribute:
        {
            xvGetPortAttributeReq *stuff = (xvGetPortAttributeReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->port);

            REPLY (" Attribute");
            reply = xDbgGetAtom(stuff->attribute, evinfo, reply, len);

            return reply;
        }

    case xv_PutImage:
        {
            xvPutImageReq *stuff = (xvPutImageReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) ID(%x) buf(%dx%d) src(%d,%d %dx%d) drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                (unsigned int)stuff->id,
                stuff->width,
                stuff->height,
                stuff->src_x,
                stuff->src_y,
                stuff->src_w,
                stuff->src_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            return reply;
        }

    case xv_ShmPutImage:
        {
            xvShmPutImageReq *stuff = (xvShmPutImageReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x) GC(0x%x) ID(%x) buf(%dx%d) src(%d,%d %dx%d) drw(%d,%d %dx%d)",
                (unsigned int)stuff->port,
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                (unsigned int)stuff->id,
                stuff->width,
                stuff->height,
                stuff->src_x,
                stuff->src_y,
                stuff->src_w,
                stuff->src_h,
                stuff->drw_x,
                stuff->drw_y,
                stuff->drw_w,
                stuff->drw_h);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s shmseg(0x%x) offset(%ld) send_event(%s)",
                    " ",
                    (unsigned int)stuff->shmseg,
                    stuff->offset,
                    stuff->send_event ? "YES" : "NO");
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventXv (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case XvVideoNotify:
        {
            XvVideoNotifyEvent *stuff = (XvVideoNotifyEvent *) evt;
            REPLY (": XID(0x%x) portID(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->port_id);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" serial(%lu) reason(%lu) time(%lums)",
                    stuff->serial,
                    stuff->reason,
                    stuff->time);
            }

            return reply;
        }

    case XvPortNotify:
        {
            XvPortNotifyEvent *stuff = (XvPortNotifyEvent *) evt;
            REPLY (": XID(0x%x) Value(%ld)",
                (unsigned int)stuff->port_id,
                stuff->value);

            REPLY (" Attribute");
            reply = xDbgGetAtom(stuff->attribute, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" serial(%lu) value(%ld) time(%lums)",
                    stuff->serial,
                    stuff->value,
                    stuff->time);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyXv (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
        case xv_QueryBestSize:
        {
            if (evinfo->rep.isStart)
            {
                xvQueryBestSizeReply *stuff = (xvQueryBestSizeReply *)rep;
                REPLY (": actualSize(%dx%d)",
                    stuff->actual_width,
                    stuff->actual_height);
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
xDbgEvlogXvGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXv;
    extinfo->evt_func = _EvlogEventXv;
    extinfo->rep_func = _EvlogReplyXv;
#else
    ExtensionEntry *xext = CheckExtension (XvName);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXv;
    extinfo->evt_func = _EvlogEventXv;
    extinfo->rep_func = _EvlogReplyXv;

#endif
}

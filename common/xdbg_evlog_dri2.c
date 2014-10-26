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
#include <dri2/dri2.h>
#include <X11/extensions/dri2proto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_dri2.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestDri2 (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_DRI2CreateDrawable:
        {
            xDRI2CreateDrawableReq *stuff = (xDRI2CreateDrawableReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            return reply;
        }

    case X_DRI2DestroyDrawable:
        {
            xDRI2DestroyDrawableReq *stuff = (xDRI2DestroyDrawableReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            return reply;
        }

    case X_DRI2GetBuffers:
        {
            xDRI2GetBuffersReq *stuff = (xDRI2GetBuffersReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" count(%ld)",
                    stuff->count);
            }

            return reply;
        }

    case X_DRI2CopyRegion:
        {
            xDRI2CopyRegionReq *stuff = (xDRI2CopyRegionReq *)req;
            REPLY (": XID(0x%lx) src(0x%lx) dst(0x%lx)",
                stuff->drawable,
                stuff->src,
                stuff->dest);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" Region");
                reply = xDbgGetRegion(stuff->region, evinfo, reply, len);
            }

            return reply;
        }

    case X_DRI2GetBuffersWithFormat:
        {
            xDRI2GetBuffersReq *stuff = (xDRI2GetBuffersReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" count(%ld)",
                    stuff->count);
            }

            return reply;
        }


    case X_DRI2SwapBuffers:
        {
            xDRI2SwapBuffersReq *stuff = (xDRI2SwapBuffersReq *)req;
            REPLY (": XID(0x%lx) msc(0x%lx/0x%lx) divisor(0x%lx/0x%lx) remainder(0x%lx/0x%lx)",
                stuff->drawable,
                stuff->target_msc_hi,
                stuff->target_msc_lo,
                stuff->divisor_hi,
                stuff->divisor_lo,
                stuff->remainder_hi,
                stuff->remainder_lo);

            return reply;
        }

    case X_DRI2SwapInterval:
        {
            xDRI2SwapIntervalReq *stuff = (xDRI2SwapIntervalReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" interval(%ld)",
                    stuff->interval);
            }

            return reply;
        }

    case X_DRI2SwapBuffersWithRegion:
        {
            xDRI2SwapBuffersWithRegionReq *stuff = (xDRI2SwapBuffersWithRegionReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" Region");
                reply = xDbgGetRegion(stuff->region, evinfo, reply, len);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventDri2 (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case DRI2_BufferSwapComplete:
        {
            xDRI2BufferSwapComplete *stuff = (xDRI2BufferSwapComplete *) evt;
            REPLY (": XID(0x%lx) ust(0x%lx/0x%lx) msc(0x%lx/0x%lx) sbc(0x%lx/0x%lx)",
                stuff->drawable,
                stuff->ust_hi,
                stuff->ust_lo,
                stuff->msc_hi,
                stuff->msc_lo,
                stuff->sbc_hi,
                stuff->sbc_lo);

            evinfo->evt.size = sizeof (xDRI2BufferSwapComplete);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s sequence_num(%d) event_type(%d)",
                    " ",
                    stuff->sequenceNumber,
                    stuff->event_type);
            }

            return reply;
        }

    case DRI2_InvalidateBuffers:
        {
            xDRI2InvalidateBuffers *stuff = (xDRI2InvalidateBuffers *) evt;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" sequence_num(%d)",
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
_EvlogReplyDri2 (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_DRI2GetBuffers:
        {
            if (evinfo->rep.isStart)
            {
                xDRI2GetBuffersReply *stuff = (xDRI2GetBuffersReply *)rep;
                REPLY (": size(%ldx%ld) count(%ld)",
                    stuff->width,
                    stuff->height,
                    stuff->count);

                if (detail_level >= EVLOG_PRINT_DETAIL)
                {
                    REPLY (" sequence_num(%d)",
                        stuff->sequenceNumber);
                }
            }
            else
            {
                xDRI2Buffer *stuff = (xDRI2Buffer *)rep;

                REPLY ("attachment(0x%lx) Name(0x%lx) pitch(0x%lx) cpp(0x%lx) flags(0x%lx)",
                    stuff->attachment,
                    stuff->name,
                    stuff->pitch,
                    stuff->cpp,
                    stuff->flags);
            }

            return reply;
        }

    case X_DRI2SwapBuffers:
        {
            if (evinfo->rep.isStart)
            {
                xDRI2SwapBuffersReply *stuff = (xDRI2SwapBuffersReply *)rep;
                REPLY (": swap(%ld/%ld)",
                    stuff->swap_hi,
                    stuff->swap_lo);

                if (detail_level >= EVLOG_PRINT_DETAIL)
                {
                    REPLY (" sequence_num(%d)",
                        stuff->sequenceNumber);
                }
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
xDbgEvlogDri2GetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestDri2;
    extinfo->evt_func = _EvlogEventDri2;
    extinfo->rep_func = _EvlogReplyDri2;
#else
    ExtensionEntry *xext = CheckExtension (DRI2_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestDri2;
    extinfo->evt_func = _EvlogEventDri2;
    extinfo->rep_func = _EvlogReplyDri2;
#endif
}

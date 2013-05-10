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
_EvlogRequestDri2 (EvlogInfo *evinfo, char *reply, int *len)
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
            REPLY (": XID(0x%lx) Count(%ld)",
                stuff->drawable,
                stuff->count);

            return reply;
        }

    case X_DRI2CopyRegion:
        {
            xDRI2CopyRegionReq *stuff = (xDRI2CopyRegionReq *)req;
            REPLY (": XID(0x%lx) src(0x%lx) dst(0x%lx)",
                stuff->drawable,
                stuff->src,
                stuff->dest);

            REPLY (" Region");
            reply = xDbgGetRegion(stuff->region, evinfo, reply, len);

            return reply;
        }

    case X_DRI2GetBuffersWithFormat:
        {
            xDRI2GetBuffersReq *stuff = (xDRI2GetBuffersReq *)req;
            REPLY (": XID(0x%lx) count(%ld)",
                stuff->drawable,
                stuff->count);

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
            REPLY (": XID(0x%lx) Interval(%ld)",
                stuff->drawable,
                stuff->interval);

            return reply;
        }

    case X_DRI2SwapBuffersWithRegion:
        {
            xDRI2SwapBuffersWithRegionReq *stuff = (xDRI2SwapBuffersWithRegionReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            REPLY (" Region");
            reply = xDbgGetRegion(stuff->region, evinfo, reply, len);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventDri2 (EvlogInfo *evinfo, int first_base, char *reply, int *len)
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

            return reply;
        }

    case DRI2_InvalidateBuffers:
        {
            xDRI2InvalidateBuffers *stuff = (xDRI2InvalidateBuffers *) evt;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

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
#else
    ExtensionEntry *xext = CheckExtension (DRI2_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestDri2;
    extinfo->evt_func = _EvlogEventDri2;
#endif
}

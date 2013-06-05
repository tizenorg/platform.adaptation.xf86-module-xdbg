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

#include <X11/extensions/XShm.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/sync.h>
#include <X11/extensions/xtestext1.h>
#include <X11/extensions/XTest.h>
#include <X11/Xlibint.h>

#include <X11/extensions/dpmsproto.h>
#include <X11/extensions/shmproto.h>
#include <X11/extensions/syncproto.h>
#include <X11/extensions/xtestext1proto.h>
#include <X11/extensions/xtestproto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_xext.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestXextDpms(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_DPMSSetTimeouts:
        {
            xDPMSSetTimeoutsReq *stuff = (xDPMSSetTimeoutsReq *)req;
            REPLY (": Standby(%d) Suspend(%d) off(%d)",
                stuff->standby,
                stuff->suspend,
                stuff->off);

            return reply;
        }

    case X_DPMSForceLevel:
        {
            xDPMSForceLevelReq *stuff = (xDPMSForceLevelReq *)req;
            REPLY (": Level(%d)",
                stuff->level);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogRequestXextShm (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_ShmPutImage:
        {
            xShmPutImageReq *stuff = (xShmPutImageReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) size(%dx%d) src(%d,%d %dx%d) dst(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->totalWidth,
                stuff->totalHeight,
                stuff->srcX,
                stuff->srcY,
                stuff->srcWidth,
                stuff->srcHeight,
                stuff->dstX,
                stuff->dstY);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *format;
                char dformat[10];

                switch (stuff->format)
                {
                    case XYBitmap:  format = "XYBitmap"; break;
                    case XYPixmap:  format = "XYPixmap"; break;
                    case ZPixmap:  format = "ZPixmap"; break;
                    default:  format = dformat; sprintf (dformat, "%d", stuff->format); break;
                }

                REPLY ("\n");
                REPLY ("%67s depth(%d) format(%s) send_event(%s) shmseg(0x%lx) offset(%ld)",
                    " ",
                    stuff->depth,
                    format,
                    stuff->sendEvent ? "YES" : "NO",
                    stuff->shmseg,
                    stuff->offset);
            }

            return reply;
        }

    case X_ShmGetImage:
        {
            xShmGetImageReq *stuff = (xShmGetImageReq *)req;
            REPLY (": XID(0x%lx) size(%dx%d) coord(%d,%d)",
                stuff->drawable,
                stuff->width,
                stuff->height,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *format;
                char  dformat[10];

                switch (stuff->format)
                {
                    case XYBitmap:  format = "XYBitmap"; break;
                    case XYPixmap:  format = "XYPixmap"; break;
                    case ZPixmap:  format = "ZPixmap"; break;
                    default:  format = dformat; sprintf (dformat, "%d", stuff->format); break;
                }

                REPLY ("\n");
                REPLY ("%67s format(%s) plain_mask(0x%lx) shmseg(0x%lx) offset(%ld)",
                    " ",
                    format,
                    stuff->planeMask,
                    stuff->shmseg,
                    stuff->offset);
            }

            return reply;
        }

    case X_ShmCreatePixmap:
        {
            xShmCreatePixmapReq *stuff = (xShmCreatePixmapReq *)req;
            REPLY (": Pixmap(0x%lx) Drawable(0x%lx) size(%dx%d)",
                stuff->pid,
                stuff->drawable,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s depth(%d) shmseg(0x%lx) offset(%ld)",
                    " ",
                    stuff->depth,
                    stuff->shmseg,
                    stuff->offset);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogRequestXextSync(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_SyncCreateCounter:
        {
            xSyncCreateCounterReq *stuff = (xSyncCreateCounterReq *)req;
            REPLY (": XID(0x%lx) initValue(%ld/%ld)",
                stuff->cid,
                stuff->initial_value_hi,
                stuff->initial_value_lo);

            return reply;
        }

    case X_SyncSetCounter:
        {
            xSyncSetCounterReq *stuff = (xSyncSetCounterReq *)req;
            REPLY (": XID(0x%lx) Value(%ld/%ld)",
                stuff->cid,
                stuff->value_hi,
                stuff->value_lo);

            return reply;
        }

    case X_SyncChangeCounter:
        {
            xSyncChangeCounterReq *stuff = (xSyncChangeCounterReq *)req;
            REPLY (": XID(0x%lx) Value(%ld/%ld)",
                stuff->cid,
                stuff->value_hi,
                stuff->value_lo);

            return reply;
        }

    case X_SyncQueryCounter:
        {
            xSyncQueryCounterReq *stuff = (xSyncQueryCounterReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->counter);

            return reply;
        }

    case X_SyncDestroyCounter:
        {
            xSyncDestroyCounterReq *stuff = (xSyncDestroyCounterReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->counter);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogRequestXextXtestExt1(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_TestFakeInput:
        {
            xTestFakeInputReq *stuff = (xTestFakeInputReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->ack);

            return reply;
        }

    case X_TestGetInput:
        {
            xTestGetInputReq *stuff = (xTestGetInputReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->mode);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogRequestXextXtest(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_XTestGetVersion:
        {
            xXTestGetVersionReq *stuff = (xXTestGetVersionReq *)req;
            REPLY (": MajorVersion(%d) MinorVersion(%d)",
                stuff->majorVersion,
                stuff->minorVersion);

            return reply;
        }

    case X_XTestCompareCursor:
        {
            xXTestCompareCursorReq *stuff = (xXTestCompareCursorReq *)req;
            REPLY (": XID(0x%lx) Cursor(0x%lx)",
                stuff->window,
                stuff->cursor);

            return reply;
        }

    case X_XTestFakeInput:
        {
            xXTestFakeInputReq *stuff = (xXTestFakeInputReq *)req;
            REPLY (": XID(0x%lx) coord(%d,%d)",
                stuff->root,
                stuff->rootX,
                stuff->rootY);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" type(%d) detail(%d) time(%lums) device_id(%d)",
                    stuff->type,
                    stuff->detail,
                    stuff->time,
                    stuff->deviceid);
            }

            return reply;
        }

    case X_XTestGrabControl:
        {
            xXTestGrabControlReq *stuff = (xXTestGrabControlReq *)req;
            REPLY (": Impervious(%s)" ,
                (stuff->impervious)? "YES" : "NO");

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventXextDpms (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
#if 0
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {

    default:
            break;
    }
#endif
    return reply;
}


static char *
_EvlogEventXextShm (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case ShmCompletion:
        {
            xShmCompletionEvent *stuff = (xShmCompletionEvent *) evt;
            REPLY (": XID(0x%lx)",
                stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" sequence_num(%d) major_event(%d) minor_event(%d) shmseg(0x%lx) offset(%ld)",
                    stuff->sequenceNumber,
                    stuff->majorEvent,
                    stuff->minorEvent,
                    stuff->shmseg,
                    stuff->offset);
            }
            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventXextSync (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case XSyncCounterNotify:
        {
            xSyncCounterNotifyEvent *stuff = (xSyncCounterNotifyEvent *) evt;
            REPLY (": XID(0x%lx) WaitValue(0x%lx/0x%lx) CounterValue(0x%lx/0x%lx)",
                stuff->counter,
                stuff->wait_value_hi,
                stuff->wait_value_lo,
                stuff->counter_value_hi,
                stuff->counter_value_lo);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s sequence_num(%d) time(%lums) count(%d) destroyed(%s)",
                    " ",
                    stuff->sequenceNumber,
                    stuff->time,
                    stuff->count,
                    stuff->destroyed ? "YES" : "NO");
            }

            return reply;
        }

    case XSyncAlarmNotify:
        {
            xSyncAlarmNotifyEvent *stuff = (xSyncAlarmNotifyEvent *) evt;
            REPLY (": XID(0x%lx) CounterValue(0x%lx/0x%lx) AlarmValue(0x%lx/0x%lx)",
                stuff->alarm,
                stuff->counter_value_hi,
                stuff->counter_value_lo,
                stuff->alarm_value_hi,
                stuff->alarm_value_lo);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s sequence_num(%d) time(%lums) state(%d)",
                    " ",
                    stuff->sequenceNumber,
                    stuff->time,
                    stuff->state);
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventXextXtestExt1 (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventXextXtest (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {

    default:
            break;
    }

    return reply;
}


static char *
_EvlogReplyXextDpms (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_DPMSSetTimeouts:
        {
            if (evinfo->rep.isStart)
            {
                xDPMSGetTimeoutsReply *stuff = (xDPMSGetTimeoutsReply *)rep;
                REPLY (": Standby(%usec) Suspend(%usec) off(%usec) sequence_num(%d)",
                    stuff->standby,
                    stuff->suspend,
                    stuff->off,
                    stuff->sequenceNumber);
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

static char *
_EvlogReplyXextShm (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_ShmGetImage:
        {
            if (evinfo->rep.isStart)
            {
                xShmGetImageReply *stuff = (xShmGetImageReply *)rep;
                REPLY (": Visual(0x%lx) size(%ld) sequence_num(%d)",
                    stuff->visual,
                    stuff->size,
                    stuff->sequenceNumber);
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

static char *
_EvlogReplyXextSync (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
        case X_SyncQueryCounter:
        {
            if (evinfo->rep.isStart)
            {
                xSyncQueryCounterReply *stuff = (xSyncQueryCounterReply *)rep;
                REPLY (": Value(0x%lx/0x%lx) sequence_num(%d)",
                    stuff->value_hi,
                    stuff->value_lo,
                    stuff->sequenceNumber);
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

static char *
_EvlogReplyXextXtestExt1 (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
        case X_TestQueryInputSize:
        {
            if (evinfo->rep.isStart)
            {
                xTestQueryInputSizeReply *stuff = (xTestQueryInputSizeReply *)rep;
                REPLY (": sizeReturn(0x%lx) sequence_num(%d)",
                    stuff->size_return,
                    stuff->sequenceNumber);
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

static char *
_EvlogReplyXextXtest (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_XTestGetVersion:
        {
            if (evinfo->rep.isStart)
            {
                xXTestGetVersionReply *stuff = (xXTestGetVersionReply *)rep;
                REPLY (": MinorVersion(%d) sequence_num(%d)",
                    stuff->minorVersion,
                    stuff->sequenceNumber);
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
xDbgEvlogXextDpmsGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextDpms;
    extinfo->evt_func = _EvlogEventXextDpms;
    extinfo->rep_func = _EvlogReplyXextDpms;
#else
    ExtensionEntry *xext = CheckExtension (DPMSExtensionName);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextDpms;
    extinfo->evt_func = _EvlogEventXextDpms;
    extinfo->rep_func = _EvlogReplyXextDpms;
#endif
}


void
xDbgEvlogXextShmGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextShm;
    extinfo->evt_func = _EvlogEventXextShm;
    extinfo->rep_func = _EvlogReplyXextShm;
#else
    ExtensionEntry *xext = CheckExtension (SHMNAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextShm;
    extinfo->evt_func = _EvlogEventXextShm;
    extinfo->rep_func = _EvlogReplyXextShm;
#endif
}


void
xDbgEvlogXextSyncGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextSync;
    extinfo->evt_func = _EvlogEventXextSync;
    extinfo->rep_func = _EvlogReplyXextSync;
#else
    ExtensionEntry *xext = CheckExtension (SYNC_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextSync;
    extinfo->evt_func = _EvlogEventXextSync;
    extinfo->rep_func = _EvlogReplyXextSync;
#endif

}


void
xDbgEvlogXextXtestExt1GetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextXtestExt1;
    extinfo->evt_func = _EvlogEventXextXtestExt1;
    extinfo->rep_func = _EvlogReplyXextXtestExt1;
#else
    ExtensionEntry *xext = CheckExtension (XTestEXTENSION_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextXtestExt1;
    extinfo->evt_func = _EvlogEventXextXtestExt1;
    extinfo->rep_func = _EvlogReplyXextXtestExt1;
#endif

}


void
xDbgEvlogXextXtestGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextXtest;
    extinfo->evt_func = _EvlogEventXextXtest;
    extinfo->rep_func = _EvlogReplyXextXtest;
#else
    ExtensionEntry *xext = CheckExtension (XTestExtensionName);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextXtest;
    extinfo->evt_func = _EvlogEventXextXtest;
    extinfo->rep_func = _EvlogReplyXextXtest;
#endif

}

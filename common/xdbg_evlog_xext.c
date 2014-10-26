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
            REPLY (": XID(0x%x) gc(0x%x) size(%dx%d) src(%d,%d %dx%d) dst(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
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
                    default:  format = dformat; snprintf (dformat, 10, "%d", stuff->format); break;
                }

                REPLY ("\n");
                REPLY ("%67s depth(%d) format(%s) send_event(%s) shmseg(0x%x) offset(%ld)",
                    " ",
                    stuff->depth,
                    format,
                    stuff->sendEvent ? "YES" : "NO",
                    (unsigned int)stuff->shmseg,
                    (long int)stuff->offset);
            }

            return reply;
        }

    case X_ShmGetImage:
        {
            xShmGetImageReq *stuff = (xShmGetImageReq *)req;
            REPLY (": XID(0x%x) size(%dx%d) coord(%d,%d)",
                (unsigned int)stuff->drawable,
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
                    default:  format = dformat; snprintf (dformat, 10, "%d", stuff->format); break;
                }

                REPLY ("\n");
                REPLY ("%67s format(%s) plain_mask(0x%x) shmseg(0x%x) offset(%ld)",
                    " ",
                    format,
                    (unsigned int)stuff->planeMask,
                    (unsigned int)stuff->shmseg,
                    (long int)stuff->offset);
            }

            return reply;
        }

    case X_ShmCreatePixmap:
        {
            xShmCreatePixmapReq *stuff = (xShmCreatePixmapReq *)req;
            REPLY (": Pixmap(0x%x) Drawable(0x%x) size(%dx%d)",
                (unsigned int)stuff->pid,
                (unsigned int)stuff->drawable,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s depth(%d) shmseg(0x%x) offset(%ld)",
                    " ",
                    stuff->depth,
                    (unsigned int)stuff->shmseg,
                    (long int)stuff->offset);
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
            REPLY (": XID(0x%x) initValue(%ld/%ld)",
                (unsigned int)stuff->cid,
                (long int)stuff->initial_value_hi,
                (long int)stuff->initial_value_lo);

            return reply;
        }

    case X_SyncSetCounter:
        {
            xSyncSetCounterReq *stuff = (xSyncSetCounterReq *)req;
            REPLY (": XID(0x%x) Value(%ld/%ld)",
                (unsigned int)stuff->cid,
                (long int)stuff->value_hi,
                (long int)stuff->value_lo);

            return reply;
        }

    case X_SyncChangeCounter:
        {
            xSyncChangeCounterReq *stuff = (xSyncChangeCounterReq *)req;
            REPLY (": XID(0x%x) Value(%ld/%ld)",
                (unsigned int)stuff->cid,
                (long int)stuff->value_hi,
                (long int)stuff->value_lo);

            return reply;
        }

    case X_SyncQueryCounter:
        {
            xSyncQueryCounterReq *stuff = (xSyncQueryCounterReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->counter);

            return reply;
        }

    case X_SyncDestroyCounter:
        {
            xSyncDestroyCounterReq *stuff = (xSyncDestroyCounterReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->counter);

            return reply;
        }

    case X_SyncAwait:
        {
            xSyncAwaitReq *stuff = (xSyncAwaitReq*)req;
            xSyncWaitCondition *pProtocolWaitConds;

            pProtocolWaitConds = (xSyncWaitCondition *) &stuff[1];
            REPLY (": XID(0x%x) VType:%d TType:%d Value(%d/%d)",
                (unsigned int)pProtocolWaitConds->counter,
                (unsigned int)pProtocolWaitConds->value_type,
                (unsigned int)pProtocolWaitConds->test_type,
                (unsigned int)pProtocolWaitConds->wait_value_hi,
                (unsigned int)pProtocolWaitConds->wait_value_lo);
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
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->ack);

            return reply;
        }

    case X_TestGetInput:
        {
            xTestGetInputReq *stuff = (xTestGetInputReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->mode);

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
            REPLY (": XID(0x%x) Cursor(0x%x)",
                (unsigned int)stuff->window,
                (unsigned int)stuff->cursor);

            return reply;
        }

    case X_XTestFakeInput:
        {
            xXTestFakeInputReq *stuff = (xXTestFakeInputReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d)",
                (unsigned int)stuff->root,
                stuff->rootX,
                stuff->rootY);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" type(%d) detail(%d) time(%lums) device_id(%d)",
                    stuff->type,
                    stuff->detail,
                    (unsigned long)stuff->time,
                    stuff->deviceid);
            }

            return reply;
        }
    }

    return reply;
}

static char *
_EvlogRequestXextShape(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_ShapeRectangles:
        {
            xShapeRectanglesReq *stuff = (xShapeRectanglesReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d)",
                (unsigned int)stuff->dest,
                stuff->xOff,
                stuff->yOff);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                int i;
                int nrect;
                xRectangle *prect;
                const char *destKind, *ordering;
                char  ddestKind[10], dordering[10];

                switch (stuff->destKind)
                {
                    case ShapeBounding:  destKind = "ShapeBounding"; break;
                    case ShapeClip:  destKind = "ShapeClip"; break;
                    case ShapeInput:  destKind = "ShapeInput"; break;
                    default:  destKind = ddestKind; snprintf (ddestKind, 10, "%d", stuff->destKind); break;
                }

                switch (stuff->ordering)
                {
                    case Unsorted:  ordering = "Unsorted"; break;
                    case YSorted:  ordering = "YSorted"; break;
                    case YXSorted:  ordering = "YXSorted"; break;
                    case YXBanded:  ordering = "YXBanded"; break;
                    default:  ordering = dordering; snprintf (dordering, 10, "%d", stuff->ordering); break;
                }

                nrect = ((stuff->length * 4) - sizeof(xShapeRectanglesReq)) / sizeof(xRectangle);
                prect = (xRectangle *) &stuff[1];

                REPLY (" op(%d) destKind(%s) ordering(%s) nrect(%d)",
                    stuff->op,
                    destKind,
                    ordering,
                    nrect);

                REPLY ("\n");
                REPLY ("%67s Region", " ");
                REPLY ("(");

                for (i = 0 ; i < nrect ; i++)
                {
                    REPLY("[%d,%d %dx%d]", 
                        prect[i].x,
                        prect[i].y,
                        prect[i].width,
                        prect[i].height);

                    if(i != nrect - 1)
                        REPLY (", ");
                }

                REPLY (")");
            }

            return reply;
        }

    case X_ShapeMask:
        {
            xShapeMaskReq *stuff = (xShapeMaskReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d) src(0x%x)",
                (unsigned int)stuff->dest,
                stuff->xOff,
                stuff->yOff,
                (unsigned int)stuff->src);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *destKind;
                char  ddestKind[10];

                switch (stuff->destKind)
                {
                    case ShapeBounding:  destKind = "ShapeBounding"; break;
                    case ShapeClip:  destKind = "ShapeClip"; break;
                    case ShapeInput:  destKind = "ShapeInput"; break;
                    default:  destKind = ddestKind; snprintf (ddestKind, 10, "%d", stuff->destKind); break;
                }

                REPLY (" op(%d) destKind(%s)",
                    stuff->op,
                    destKind);
            }

            return reply;
        }

    case X_ShapeCombine:
        {
            xShapeCombineReq *stuff = (xShapeCombineReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d) src(0x%x)",
                (unsigned int)stuff->dest,
                stuff->xOff,
                stuff->yOff,
                (unsigned int)stuff->src);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *destKind, *srcKind;
                char  ddestKind[10], dsrcKind[10];

                switch (stuff->destKind)
                {
                    case ShapeBounding:  destKind = "ShapeBounding"; break;
                    case ShapeClip:  destKind = "ShapeClip"; break;
                    case ShapeInput:  destKind = "ShapeInput"; break;
                    default:  destKind = ddestKind; snprintf (ddestKind, 10, "%d", stuff->destKind); break;
                }

                switch (stuff->srcKind)
                {
                    case ShapeBounding:  srcKind = "ShapeBounding"; break;
                    case ShapeClip:  srcKind = "ShapeClip"; break;
                    case ShapeInput:  srcKind = "ShapeInput"; break;
                    default:  srcKind = dsrcKind; snprintf (dsrcKind, 10, "%d", stuff->srcKind); break;
                }

                REPLY (" op(%d) destKind(%s) srcKind(%s)",
                    stuff->op,
                    destKind,
                    srcKind);
            }

            return reply;
        }

    case X_ShapeOffset:
        {
            xShapeOffsetReq *stuff = (xShapeOffsetReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d)",
                (unsigned int)stuff->dest,
                stuff->xOff,
                stuff->yOff);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *destKind;
                char  ddestKind[10];

                switch (stuff->destKind)
                {
                    case ShapeBounding:  destKind = "ShapeBounding"; break;
                    case ShapeClip:  destKind = "ShapeClip"; break;
                    case ShapeInput:  destKind = "ShapeInput"; break;
                    default:  destKind = ddestKind; snprintf (ddestKind, 10, "%d", stuff->destKind); break;
                }

                REPLY (" destKind(%s)",
                    destKind);
            }

            return reply;
        }

    case X_ShapeQueryExtents:
        {
            xShapeQueryExtentsReq *stuff = (xShapeQueryExtentsReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    case X_ShapeSelectInput:
        {
            xShapeSelectInputReq *stuff = (xShapeSelectInputReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" enable(%s)",
                    stuff->enable ? "YES" : "NO");
            }

            return reply;
        }

    case X_ShapeInputSelected:
        {
            xShapeInputSelectedReq *stuff = (xShapeInputSelectedReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    case X_ShapeGetRectangles:
        {
            xShapeGetRectanglesReq *stuff = (xShapeGetRectanglesReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char  dkind[10];

                switch (stuff->kind)
                {
                    case ShapeBounding:  kind = "ShapeBounding"; break;
                    case ShapeClip:  kind = "ShapeClip"; break;
                    case ShapeInput:  kind = "ShapeInput"; break;
                    default:  kind = dkind; snprintf (dkind, 10, "%d", stuff->kind); break;
                }

                REPLY (" kind(%s)",
                    kind);
            }

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
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" sequence_num(%d) major_event(%d) minor_event(%d) shmseg(0x%x) offset(%ld)",
                    stuff->sequenceNumber,
                    stuff->majorEvent,
                    stuff->minorEvent,
                    (unsigned int)stuff->shmseg,
                    (long int)stuff->offset);
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
            REPLY (": XID(0x%x) WaitValue(0x%x/0x%x) CounterValue(0x%x/0x%x)",
                (unsigned int)stuff->counter,
                (unsigned int)stuff->wait_value_hi,
                (unsigned int)stuff->wait_value_lo,
                (unsigned int)stuff->counter_value_hi,
                (unsigned int)stuff->counter_value_lo);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s sequence_num(%d) time(%lums) count(%d) destroyed(%s)",
                    " ",
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->count,
                    stuff->destroyed ? "YES" : "NO");
            }

            return reply;
        }

    case XSyncAlarmNotify:
        {
            xSyncAlarmNotifyEvent *stuff = (xSyncAlarmNotifyEvent *) evt;
            REPLY (": XID(0x%x) CounterValue(0x%x/0x%x) AlarmValue(0x%x/0x%x)",
                (unsigned int)stuff->alarm,
                (unsigned int)stuff->counter_value_hi,
                (unsigned int)stuff->counter_value_lo,
                (unsigned int)stuff->alarm_value_hi,
                (unsigned int)stuff->alarm_value_lo);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s sequence_num(%d) time(%lums) state(%d)",
                    " ",
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
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
_EvlogEventXextShape (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case ShapeNotify:
        {
            xShapeNotifyEvent *stuff = (xShapeNotifyEvent *) evt;
            REPLY (": XID(0x%x) coord(%d,%d %dx%d)",
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *kind;
                char dkind[10];

                switch (stuff->kind)
                {
                    case ShapeBounding:  kind = "ShapeBounding"; break;
                    case ShapeClip:  kind = "ShapeClip"; break;
                    case ShapeInput:  kind = "ShapeInput"; break;
                    default:  kind = dkind; snprintf (dkind, 10, "%d", stuff->kind); break;
                }


                REPLY ("\n");
                REPLY ("%67s kind(%s) sequence_num(%d) time(%lums) shaped(%s)",
                    " ",
                    kind,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->time,
                    stuff->shaped ? "EXIST" : "NON_EXIST");
            }

            return reply;
        }

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
                REPLY (": Visual(0x%x) size(%ld) sequence_num(%d)",
                    (unsigned int)stuff->visual,
                    (long int)stuff->size,
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
                REPLY (": Value(%ld/%ld) sequence_num(%d)",
                    (long int)stuff->value_hi,
                    (long int)stuff->value_lo,
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
                REPLY (": sizeReturn(0x%x) sequence_num(%d)",
                    (unsigned int)stuff->size_return,
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

static char *
_EvlogReplyXextShape(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
    case X_ShapeQueryVersion:
        {
            if (evinfo->rep.isStart)
            {
                xShapeQueryVersionReply *stuff = (xShapeQueryVersionReply *)rep;
                REPLY (": MajorVersion(%d) MinorVersion(%d)",
                    stuff->majorVersion,
                    stuff->minorVersion);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_ShapeQueryExtents:
        {
            if (evinfo->rep.isStart)
            {
                xShapeQueryExtentsReply *stuff = (xShapeQueryExtentsReply *)rep;

                REPLY (": bounding_shaped(%s)",
                    stuff->boundingShaped ? "YES" : "NO");

                if (stuff->boundingShaped)
                    REPLY (" bounding(%d,%d %dx%d)",
                        stuff->xClipShape,
                        stuff->yClipShape,
                        stuff->widthClipShape,
                        stuff->heightClipShape);
                else
                    REPLY (" bounding(None)");


                REPLY (" clip_shaped(%s)",
                    stuff->clipShaped ? "YES" : "NO");

                if (stuff->boundingShaped)
                    REPLY (" clip(%d,%d %dx%d)",
                        stuff->xBoundingShape,
                        stuff->yBoundingShape,
                        stuff->widthBoundingShape,
                        stuff->heightBoundingShape);
                else
                    REPLY (" clip(None)");

            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_ShapeInputSelected:
        {
            if (evinfo->rep.isStart)
            {
                xShapeInputSelectedReply *stuff = (xShapeInputSelectedReply *)rep;

                REPLY (" enable(%s)",
                    stuff->enabled ? "YES" : "NO");
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_ShapeGetRectangles:
        {
            if (evinfo->rep.isStart)
            {
                xShapeGetRectanglesReply *stuff = (xShapeGetRectanglesReply *)rep;

                const char *ordering;
                char dordering[10];

                switch (stuff->ordering)
                {
                    case Unsorted:  ordering = "Unsorted"; break;
                    case YSorted:  ordering = "YSorted"; break;
                    case YXSorted:  ordering = "YXSorted"; break;
                    case YXBanded:  ordering = "YXBanded"; break;
                    default:  ordering = dordering; snprintf (dordering, 10, "%d", stuff->ordering); break;
                }
                REPLY (": ordering(%s) nrects(%ld)",
                    ordering,
                    (long int)stuff->nrects);
            }
            else
            {
                xRectangle *stuff = (xRectangle *)rep;
                int i;

                REPLY ("Region");
                REPLY ("(");
                for (i = 0 ; i < evinfo->rep.size / sizeof(xRectangle) ; i ++)
                {
                    REPLY ("[%d,%d %dx%d]",
                        stuff->x,
                        stuff->y,
                        stuff->width,
                        stuff->height);

                    if(i != evinfo->rep.size / sizeof(xRectangle) - 1)
                        REPLY (", ");
                }
                REPLY (")");

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

void
xDbgEvlogXextShapeGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestXextShape;
    extinfo->evt_func = _EvlogEventXextShape;
    extinfo->rep_func = _EvlogReplyXextShape;
#else
    ExtensionEntry *xext = CheckExtension (SHAPENAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestXextShape;
    extinfo->evt_func = _EvlogEventXextShape;
    extinfo->rep_func = _EvlogReplyXextShape;
#endif
}

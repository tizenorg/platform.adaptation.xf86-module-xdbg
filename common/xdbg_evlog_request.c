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
#include <X11/extensions/XI2proto.h>
#include <X11/Xlib.h>
#include <windowstr.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#include "xdbg_types.h"
#include "xdbg_evlog_request.h"

#define UNKNOWN_EVENT "<unknown>"

#ifdef XDBG_CLIENT
static int base;
#else
static ExtensionEntry *xext;
#endif

static Bool
_EvlogRequestGetExtentionEntry ()
{
#ifdef XDBG_CLIENT

    static int init = 0;
    static Bool success = FALSE;
    Display *dpy;

    if (init)
        return success;

    init = 1;

    dpy = XOpenDisplay (NULL);
    if (!dpy)
    {
        fprintf (stderr, "failed: open display\n");
        exit (-1);
    }

    if (!XShmQueryExtension (dpy))
    {
        fprintf (stderr, "[UTILX] no XShm extension. !!\n");
        return False;
    }
    printf("debug request\n");
    base = XShmGetEventBase(dpy);
    success = TRUE;
    XCloseDisplay (dpy);
    return success;

#else

    static int init = 0;
    static Bool success = FALSE;

    if (init)
        return success;

    init = 1;
    xext = CheckExtension (SHMNAME);
    RETURN_VAL_IF_FAIL (xext != NULL, FALSE);

    success = TRUE;

    return success;

#endif
}

static char *
_EvlogRequestCore (xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->reqType)
    {
    case X_PutImage:
        {
            xPutImageReq *stuff = (xPutImageReq *)req;
            REPLY (": XID(%lx) size(%dx%d) dst(%d,%d)",
                stuff->drawable,
                stuff->width,
                stuff->height,
                stuff->dstX,
                stuff->dstY);
            return reply;
        }
    default:
            break;
    }

    return reply;
}

static char *
_EvlogRequestShm (xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->data)
    {
    case X_ShmPutImage:
        {
            xShmPutImageReq *stuff = (xShmPutImageReq *)req;
            REPLY (": XID(%lx) size(%dx%d) src(%d,%d %dx%d) dst(%d,%d)",
                stuff->drawable,
                stuff->totalWidth,
                stuff->totalHeight,
                stuff->srcX,
                stuff->srcY,
                stuff->srcWidth,
                stuff->srcHeight,
                stuff->dstX,
                stuff->dstY);
            return reply;
        }
    default:
            break;
    }

    return reply;
}

char *
xDbgEvlogReqeust (EvlogInfo *evinfo, char *reply, int *len)
{
    EvlogRequest req;
    xReq *xReq = NULL;

    RETURN_VAL_IF_FAIL (evinfo != NULL, reply);
    RETURN_VAL_IF_FAIL (evinfo->type == REQUEST, reply);

    req = evinfo->req;
    xReq = req.ptr;

    if (!_EvlogRequestGetExtentionEntry ())
        return reply;

    REPLY ("%s", req.name);

    if (xReq->reqType < EXTENSION_BASE)
    {
        return _EvlogRequestCore (xReq, reply, len);
    }
    else
    {

#ifdef XDBG_CLIENT
        if (xReq->reqType == base)
#else
        if (xReq->reqType == xext->base)
#endif

        {
            return _EvlogRequestShm (xReq, reply, len);
        }
    }

    return reply;
}

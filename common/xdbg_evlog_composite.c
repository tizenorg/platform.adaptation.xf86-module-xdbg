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
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/compositeproto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_composite.h"

static char *
_EvlogRequestComposite(xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->data)
    {
    case X_CompositeRedirectWindow:
        {
            xCompositeRedirectWindowReq *stuff = (xCompositeRedirectWindowReq *)req;
            REPLY (": XID(0x%lx) Update(%d)",
                stuff->window,
                stuff->update);

            return reply;
        }

    case X_CompositeRedirectSubwindows:
        {
            xCompositeRedirectSubwindowsReq *stuff = (xCompositeRedirectSubwindowsReq *)req;
            REPLY (": XID(0x%lx) Update(%d)",
                stuff->window,
                stuff->update);

            return reply;
        }

    case X_CompositeUnredirectWindow:
        {
            xCompositeUnredirectWindowReq *stuff = (xCompositeUnredirectWindowReq *)req;
            REPLY (": XID(0x%lx) Update(%d)",
                stuff->window,
                stuff->update);

            return reply;
        }

    case X_CompositeUnredirectSubwindows:
        {
            xCompositeUnredirectSubwindowsReq *stuff = (xCompositeUnredirectSubwindowsReq *)req;
            REPLY (": XID(0x%lx) Update(%d)",
                stuff->window,
                stuff->update);

            return reply;
        }

    case X_CompositeNameWindowPixmap:
        {
            xCompositeNameWindowPixmapReq *stuff = (xCompositeNameWindowPixmapReq *)req;
            REPLY (": XID(0x%lx) Pixmap(0x%lx)",
                stuff->window,
                stuff->pixmap);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogEventComposite (xEvent *evt, int first_base, char *reply, int *len)
{
    xEvent *stuff = evt;

    switch ((stuff->u.u.type & 0x7F) - first_base)
    {

    default:
            break;
    }

    return reply;
}


void
xDbgEvlogCompositeGetBase (void *dpy, ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    Display *d = (Display*)dpy;

    RETURN_IF_FAIL (d != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    if (!XQueryExtension(d, COMPOSITE_NAME, &extinfo->opcode, &extinfo->evt_base, &extinfo->err_base))
    {
        fprintf (stderr, "[UTILX] no Composite extension. \n");
        return;
    }
    extinfo->req_func = _EvlogRequestComposite;
    extinfo->evt_func = _EvlogEventComposite;
#else
    ExtensionEntry *xext = CheckExtension (COMPOSITE_NAME);
    RETURN_IF_FAIL (xext != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestComposite;
    extinfo->evt_func = _EvlogEventComposite;
#endif
}

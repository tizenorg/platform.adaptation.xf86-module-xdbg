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

#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randrproto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_randr.h"

static char *
_EvlogRequestRandr (xReq *req, char *reply, int *len)
{
    xReq *stuff = req;

    switch (stuff->data)
    {
    case X_RRGetScreenSizeRange:
        {
            xRRGetScreenSizeRangeReq *stuff = (xRRGetScreenSizeRangeReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_RRSetScreenSize:
        {
            xRRSetScreenSizeReq *stuff = (xRRSetScreenSizeReq *)req;
            REPLY (": XID(0x%lx) size(%dx%d) milliSize(%ldx%ld)",
                stuff->window,
                stuff->width,
                stuff->height,
                stuff->widthInMillimeters,
                stuff->heightInMillimeters);

            return reply;
        }

    case X_RRGetScreenResources:
        {
            xRRGetScreenResourcesReq *stuff = (xRRGetScreenResourcesReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_RRGetOutputInfo:
        {
            xRRGetOutputInfoReq *stuff = (xRRGetOutputInfoReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->output);

            return reply;
        }

    case X_RRListOutputProperties:
        {
            xRRListOutputPropertiesReq *stuff = (xRRListOutputPropertiesReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->output);

            return reply;
        }

    case X_RRQueryOutputProperty:
        {
            xRRQueryOutputPropertyReq *stuff = (xRRQueryOutputPropertyReq *)req;
            REPLY (": XID(0x%lx) Property(0x%lx)",
                stuff->output,
                stuff->property);

            return reply;
        }

    case X_RRConfigureOutputProperty:
        {
            xRRConfigureOutputPropertyReq *stuff = (xRRConfigureOutputPropertyReq *)req;
            REPLY (": XID(0x%lx) Property(0x%lx)",
                stuff->output,
                stuff->property);

            return reply;
        }

    case X_RRChangeOutputProperty:
        {
            xRRChangeOutputPropertyReq *stuff = (xRRChangeOutputPropertyReq *)req;
            REPLY (": XID(0x%lx) Property(0x%lx) Type(0x%lx) Format(%d) nUnits(%ld)",
                stuff->output,
                stuff->property,
                stuff->type,
                stuff->format,
                stuff->nUnits);

            return reply;
        }

    case X_RRDeleteOutputProperty:
        {
            xRRDeleteOutputPropertyReq *stuff = (xRRDeleteOutputPropertyReq *)req;
            REPLY (": XID(0x%lx) Property(0x%lx)",
                stuff->output,
                stuff->property);

            return reply;
        }

    case X_RRGetOutputProperty:
        {
            xRRGetOutputPropertyReq *stuff = (xRRGetOutputPropertyReq *)req;
            REPLY (": XID(0x%lx) Property(0x%lx) Type(0x%lx) longOffset(%ld) longLength(%ld)",
                stuff->output,
                stuff->property,
                stuff->type,
                stuff->longOffset,
                stuff->longLength);

            return reply;
        }

    case X_RRGetCrtcInfo:
        {
            xRRGetCrtcInfoReq *stuff = (xRRGetCrtcInfoReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->crtc);

            return reply;
        }

    case X_RRSetCrtcConfig:
        {
            xRRSetCrtcConfigReq *stuff = (xRRSetCrtcConfigReq *)req;
            REPLY (": XID(0x%lx) Coordinate(%d,%d) Rotation(%d)",
                stuff->crtc,
                stuff->x,
                stuff->y,
                stuff->rotation);

            return reply;
        }

    case X_RRGetScreenResourcesCurrent:
        {
            xRRGetScreenResourcesCurrentReq *stuff = (xRRGetScreenResourcesCurrentReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventRandr (xEvent *evt, int first_base, char *reply, int *len)
{
    xEvent *stuff = evt;

    switch ((stuff->u.u.type & 0x7F) - first_base)
    {
    case RRScreenChangeNotify:
        {
            xRRScreenChangeNotifyEvent *stuff = (xRRScreenChangeNotifyEvent *) evt;
            REPLY (": Root(0x%lx) Window(0x%lx) sizeID(%d) subPixel(%d) Pixel(%d,%d) Milli(%d,%d)",
                stuff->root,
                stuff->window,
                stuff->sizeID,
                stuff->subpixelOrder,
                stuff->widthInPixels,
                stuff->heightInPixels,
                stuff->widthInMillimeters,
                stuff->heightInMillimeters);

            return reply;
        }

    case RRNotify:
        {
            switch(stuff->u.u.detail)
            {
            case RRNotify_CrtcChange:
                {
                    xRRCrtcChangeNotifyEvent *stuff = (xRRCrtcChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%lx) Crtc(0x%lx) Mode(0x%lx) size(%udx%ud) coord(%d,%d)",
                        stuff->window,
                        stuff->crtc,
                        stuff->mode,
                        stuff->width,
                        stuff->height,
                        stuff->x,
                        stuff->y);

                    return reply;
                }

            case RRNotify_OutputChange:
                {
                    xRROutputChangeNotifyEvent *stuff = (xRROutputChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%lx) Output(0x%lx) Crtc(0x%lx) Mode(0x%lx)",
                        stuff->window,
                        stuff->output,
                        stuff->crtc,
                        stuff->mode);

                    return reply;
                }

            case RRNotify_OutputProperty:
                {
                    xRROutputPropertyNotifyEvent *stuff = (xRROutputPropertyNotifyEvent *) evt;
                    REPLY (": XID(0x%lx) Output(0x%lx) Atom(0x%lx)",
                        stuff->window,
                        stuff->output,
                        stuff->atom);

                    return reply;
                }

            case RRNotify_ProviderChange:
                {
                    xRRProviderChangeNotifyEvent *stuff = (xRRProviderChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%lx) Provider(0x%lx)",
                        stuff->window,
                        stuff->provider);

                    return reply;
                }

            case RRNotify_ProviderProperty:
                {
                    xRRProviderPropertyNotifyEvent *stuff = (xRRProviderPropertyNotifyEvent *) evt;
                    REPLY (": XID(0x%lx) Provider(0x%lx) Atom(0x%lx)",
                        stuff->window,
                        stuff->provider,
                        stuff->atom);

                    return reply;
                }

            case RRNotify_ResourceChange:
                {
                    xRRResourceChangeNotifyEvent *stuff = (xRRResourceChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%lx)",
                        stuff->window);

                    return reply;
                }

            default:
                    break;
            }
        }

    default:
            break;
    }

    return reply;
}


void
xDbgEvlogRandrGetBase (void *dpy, ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    Display *d = (Display*)dpy;

    RETURN_IF_FAIL (d != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    if (!XQueryExtension(d, RANDR_NAME, &extinfo->opcode, &extinfo->evt_base, &extinfo->err_base))
    {
        fprintf (stderr, "[UTILX] no Randr extension. \n");
        return;
    }
    extinfo->req_func = _EvlogRequestRandr;
    extinfo->evt_func = _EvlogEventRandr;
#else
    ExtensionEntry *xext = CheckExtension (RANDR_NAME);
    RETURN_IF_FAIL (xext != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestRandr;
    extinfo->evt_func = _EvlogEventRandr;
#endif
}

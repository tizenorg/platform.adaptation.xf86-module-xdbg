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

#define __USE_GNU
#include <sys/socket.h>
#include <linux/socket.h>

#ifdef HAS_GETPEERUCRED
# include <ucred.h>
#endif

#include <xace.h>
#include <xacestr.h>
#include <X11/Xatom.h>
#include <X11/extensions/XI2proto.h>
#include <windowstr.h>

#define XREGISTRY
#include <registry.h>

#include <compositeext.h>
#include <xdbg.h>

#include "xdbg_module_types.h"


#if 0
static int sum_pix_size = 0;

static void
_findRtWindow (pointer value, XID id, pointer cdata)
{
    WindowPtr pWin = value;
    ScreenPtr pScreen = pWin->drawable.pScreen;

    if (pWin->viewable && pWin->realized)
    {
        ErrorF ("xid:0x%x geo:(%5d,%5d,%5d,%5d) bpp:%2d viewable:%d realized:%d ",
                         (unsigned int)id,
                         pWin->drawable.x, pWin->drawable.y, pWin->drawable.width, pWin->drawable.height,
                         pWin->drawable.bitsPerPixel,
                         pWin->viewable,
                         pWin->realized);
        if (pWin->parent->parent)
            ErrorF ("border_id:0x%x ",(unsigned int)(pWin->parent->parent)->drawable.id);

        PixmapPtr pPixmap = (*pScreen->GetWindowPixmap) (pWin);
        PixmapPtr pScrnPixmap = (*pScreen->GetScreenPixmap) (pScreen);
        if (pPixmap == pScrnPixmap)
        {
            ErrorF ("flip_draw:1 ");
        }

        ErrorF ("\n");
    }
}

static void
_findRtCompWindow (pointer value, XID id, pointer cdata)
{
    WindowPtr pWin = value;
    WindowPtr pChild = NULL;
    WindowPtr pGrandChild = NULL;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SECPtr pSec = SECPTR (pScrn);
    PropertyPtr pProp = NULL;
    int rc;

    if (pWin->viewable && pWin->realized)
    {
        ErrorF ("xid:0x%x geo:(%5d,%5d,%5d,%5d) bpp:%2d viewable:%d realized:%d ",
                         (unsigned int)pWin->drawable.id,
                         pWin->drawable.x, pWin->drawable.y, pWin->drawable.width, pWin->drawable.height,
                         pWin->drawable.bitsPerPixel,
                         pWin->viewable,
                         pWin->realized);

        /* if pWin->child->child has a dri2 atom, the window is 3D window */
        for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
        {
            for (pGrandChild = pChild->firstChild; pGrandChild; pGrandChild = pGrandChild->nextSib)
            {
                rc = dixLookupProperty(&pProp, pGrandChild, pSec->atom_use_dri2, serverClient, DixReadAccess);
                if (rc == Success)
                {
                    ErrorF ("dri2_draw:1 ");
                }
            }
        }



        ErrorF ("\n");
    }
}

static void
_findRtPixmap (pointer value, XID id, pointer cdata)
{
    PixmapPtr pPix = value;

    ErrorF ("xid:0x%x geo:(%5d,%5d,%5d,%5d) bpp:%2d size:%8d draw_id:0x%x\n",
                     (unsigned int)id,
                     pPix->drawable.x, pPix->drawable.y, pPix->drawable.width, pPix->drawable.height,
                     pPix->drawable.bitsPerPixel,
                     pPix->devKind*pPix->drawable.height/1024,
                     (unsigned int)pPix->drawable.id);
    sum_pix_size = sum_pix_size + (pPix->devKind*pPix->drawable.height);
}

char*
xDbgModuleRList (XDbgModule *pMod, char *reply, int *len)
{
    SECPtr pSec = SECPTR (scrn);
    char *out, *tmp;
    int i, bufsize;

    bufsize = currentMaxClients * (100 + 1) + 30;
    out = tmp = malloc (bufsize);
    if (!tmp)
        return NULL;

    for (i = 1; i < currentMaxClients && (tmp < out+bufsize); i++)
    {
        ClientPtr pClient = clients[i];
        ModuleClientInfo *info;

        if (!pClient)
            continue;

        info = GetClientInfo (pClient);
        if (!info)
            continue;

        ErrorF("\n");
        ErrorF("INDEX    PID   CMD\n");
        ErrorF("%6d %6d %10s\n", info->index, info->pid, info->command);
        ErrorF("  >> WINDOWS\n");
        FindClientResourcesByType (pClient, RT_WINDOW, _findRtWindow, pClient);
        ErrorF("  >> CompositeClientWindow\n");
        FindClientResourcesByType (pClient, CompositeClientWindowType, _findRtCompWindow, pClient);
        ErrorF("  >> PIXMAPS\n");
        sum_pix_size = 0;
        FindClientResourcesByType (pClient, RT_PIXMAP, _findRtPixmap, pClient);
        ErrorF("  SUM mem: %d\n", sum_pix_size/1024);
    }

    /*
       Normal pixmap
       CREATE_PIXMAP_USAGE_BACKING_PIXMAP
       CREATE_PIXMAP_USAGE_OVERLAY
       CREATE_PIXMAP_USAGE_DRI2_FILP_BACK
       CREATE_PIXMAP_USAGE_FB
       CREATE_PIXMAP_USAGE_SUB_FB
       CREATE_PIXMAP_USAGE_DRI2_BACK
     */
    ErrorF ("==== sum of pixmap memory ====\n");
    ErrorF ("Normal pixmap                      = %d\n", pSec->pix_normal/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_BACKING_PIXMAP = %d\n", pSec->pix_backing_pixmap/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_OVERLAY        = %d\n", pSec->pix_overlay/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_DRI2_FILP_BACK = %d\n", pSec->pix_dri2_flip_back/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_FB             = %d\n", pSec->pix_fb/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_SUB_FB         = %d\n", pSec->pix_sub_fb/1024);
    ErrorF ("CREATE_PIXMAP_USAGE_DRI2_BACK      = %d\n", pSec->pix_dri2_back/1024);
    ErrorF ("TOTAL                              = %d\n",
            (pSec->pix_normal+pSec->pix_backing_pixmap+pSec->pix_overlay+pSec->pix_dri2_flip_back+pSec->pix_fb+pSec->pix_sub_fb+pSec->pix_dri2_back)/1024);
    ErrorF ("==============================\n");


    return out;
}
#else
char*
xDbgModuleRList (XDbgModule *pMod, char *reply, int *len)
{
    XDBG_REPLY ("rlist : Not implemented.\n");
    return NULL;
}
#endif

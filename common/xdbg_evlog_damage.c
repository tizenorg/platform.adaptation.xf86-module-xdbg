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
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/damageproto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_damage.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestDamage(EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_DamageCreate:
        {
            xDamageCreateReq *stuff = (xDamageCreateReq *)req;
            REPLY (": XID(0x%x) Drawable(0x%x)",
                (unsigned int)stuff->damage,
                (unsigned int)stuff->drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *level;
                char dlevel[10];

                switch (stuff->level)
                {
                    case XDamageReportRawRectangles:  level = "DamageReportRawRegion"; break;
                    case XDamageReportDeltaRectangles:  level = "DamageReportDeltaRegion"; break;
                    case XDamageReportBoundingBox:  level = "DamageReportBoundingBox"; break;
                    case XDamageReportNonEmpty:  level = "DamageReportNonEmpty"; break;
                    default:  level = dlevel; snprintf (dlevel, 10, "%d", stuff->level); break;
                }

                REPLY (" level(%s)",
                    level);
            }

            return reply;
        }

    case X_DamageDestroy:
        {
            xDamageDestroyReq *stuff = (xDamageDestroyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->damage);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

static char*
_EvlogEventDamage (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case XDamageNotify:
        {
            xDamageNotifyEvent *stuff = (xDamageNotifyEvent*)evt;
            REPLY (": XID(0x%x) Damage(0x%x) area(%d,%d %dx%d) geo(%d,%d %dx%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->damage,
                stuff->area.x,
                stuff->area.y,
                stuff->area.width,
                stuff->area.height,
                stuff->geometry.x,
                stuff->geometry.y,
                stuff->geometry.width,
                stuff->geometry.height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) level(%d) sequence_num(%d)",
                    " ",
                    (unsigned long)stuff->timestamp,
                    stuff->level,
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
_EvlogReplyDamage (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
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
xDbgEvlogDamageGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestDamage;
    extinfo->evt_func = _EvlogEventDamage;
    extinfo->rep_func = _EvlogReplyDamage;
#else
    ExtensionEntry *xext = CheckExtension (DAMAGE_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestDamage;
    extinfo->evt_func = _EvlogEventDamage;
    extinfo->rep_func = _EvlogReplyDamage;
#endif
}

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
#include <sys/shm.h>

#include "xdbg_types.h"
#include "xdbg_evlog.h"
#include "xdbg_evlog_request.h"

#define UNKNOWN_EVENT "<unknown>"

char *conn[] = {"Initial Connect", "Establich Connect"};

char *
xDbgEvlogReqeust (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    extern ExtensionInfo Evlog_extensions[];
    extern int Extensions_size;
    EvlogRequest req;
    xReq *xReq = NULL;
    int i;

    RETURN_VAL_IF_FAIL (evinfo != NULL, reply);
    RETURN_VAL_IF_FAIL (evinfo->type == REQUEST, reply);

    req = evinfo->req;
    xReq = req.ptr;

    REPLY ("%s", evinfo->req.name);

    for (i = 0 ; i < sizeof(conn) / sizeof(char *) ; i++)
        if (!strcmp(evinfo->req.name, conn[i]))
            return reply;

    if (xReq->reqType < EXTENSION_BASE)
    {
        return xDbgEvlogRequestCore (evinfo, detail_level, reply, len);
    }
    else
    {
        for (i = 0 ; i < Extensions_size ; i++)
        {
            if (xReq->reqType == Evlog_extensions[i].opcode)
            {
                return Evlog_extensions[i].req_func (evinfo, detail_level, reply, len);
            }
        }
    }

    return reply;
}

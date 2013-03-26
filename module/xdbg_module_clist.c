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

#define XREGISTRY
#include <registry.h>

#include "xdbg.h"
#include "xdbg_module_types.h"


static void
_findSyncAwait (pointer value, XID id, pointer cdata)
{
    Bool *sync_await = cdata;

    if (sync_await)
        *sync_await = TRUE;
}

void
xDbgModuleCList (XDbgModule *pMod, char *reply, int *remain)
{
    char *p = reply;
    int i, len;
    RESTYPE res_type = 0;
    const char *name;

    len = snprintf (p, *remain, "%6s   %6s   %s   %s   %s\n", "INDEX", "PID", "SYNC_AWAIT", "BLOCKED", "NAME");
    p += len;
    *remain -= len;

    /* get the res_type of SyncAwait */
    for (i = 0; i < lastResourceType; i++)
    {
        name = LookupResourceName(i + 1);
        if (!strcmp(name, "SyncAwait"))
        {
            res_type = i + 1;
            break;
        }
    }

    for (i = 1; i < currentMaxClients && (0 < *remain); i++)
    {
        ClientPtr pClient = clients[i];
        ModuleClientInfo *info;
        Bool sync_await;

        if (!pClient)
            continue;

        info = GetClientInfo (pClient);
        if (!info)
            continue;

        /* find SyncAwait resources */
        sync_await = FALSE;
        FindClientResourcesByType (pClient, res_type, _findSyncAwait, &sync_await);

        len = snprintf (p, *remain, "%6d   %6d    %4d         %4d      %9s\n",
                        info->index, info->pid, sync_await, pClient->ignoreCount, info->command);
        p += len;
        *remain -= len;
    }
}

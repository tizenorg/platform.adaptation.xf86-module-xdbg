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

#ifndef __XDBG_TYPES_H__
#define __XDBG_TYPES_H__

#include <xf86.h>
#include <X11/Xdefs.h>	/* for Bool */

#define XDBG_PATH_MAX        1024

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define SWAP(a, b)  ({int t; t = a; a = b; b = t;})

#define WARNING_IF_FAIL(cond) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed.\n", __FUNCTION__, #cond);}}
#define RETURN_IF_FAIL(cond) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed.\n", __FUNCTION__, #cond); return; }}
#define RETURN_VAL_IF_FAIL(cond, val) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed.\n", __FUNCTION__, #cond); return val; }}
#define RETURN_VAL_IF_ERRNO(cond, val, errno) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed. (err=%s(%d))\n", __FUNCTION__, #cond, strerror(errno), errno); return val; }}
#define GOTO_IF_FAIL(cond, dst) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed.\n", __FUNCTION__, #cond); goto dst; }}
#define GOTO_IF_ERRNO(cond, dst, errno) \
    {if (!(cond)) { fprintf (stderr, "[%s] '%s' failed. (err=%s(%d))\n", __FUNCTION__, #cond, strerror(errno), errno); goto dst; }}

#define REPLY(fmt, ARG...)  \
    do { \
        if (reply && len && *len > 0) \
        { \
            int s = snprintf (reply, *len, fmt, ##ARG); \
            reply += s; \
            *len -= s; \
        } \
    } while (0)

#define UNKNOWN_EVENT "<unknown>"

typedef enum
{
    EVENT,
    REQUEST,
    REPLY,
    FLUSH
} EvlogType;

#define EVLOG_MASK_CLIENT    (1<<0)
#define EVLOG_MASK_REQUEST   (1<<1)
#define EVLOG_MASK_EVENT     (1<<2)

typedef struct _EvlogClient
{
    int     index;
    int     pid;
    int     gid;
    int     uid;
    char    command[PATH_MAX+1];
} EvlogClient;

typedef struct _EvlogRequest
{
    int     id;
    CARD32  length;
    xReq   *ptr;
    char    name[PATH_MAX+1];
} EvlogRequest;

typedef struct _EvlogEvent
{
    xEvent *ptr;
    char    name[PATH_MAX+1];
} EvlogEvent;

typedef struct _EvlogInfo
{
    EvlogType    type;

    int          mask;
    EvlogClient  client;
    EvlogRequest req;
    EvlogEvent   evt;

    CARD32       time;
} EvlogInfo;

#endif

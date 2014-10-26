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

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xdbg_log.h"
#include "xdbg_dump.h"
#include "xdbg_dump_module.h"
#include <list.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define DUMP_BUFCNT  50
#define DUMP_DIR "/tmp/xdump"
#define DUMP_SCALE_RATIO  2

typedef struct
{
    int    index;

    void  *bo;      /* buffer object */
    int    bo_size;

    char   file[128];
    union {
        struct {
            int dump_w;
            int dump_h;
            xRectangle dump_rect;
        } a;
        int dump_size;
    } u;
    Bool   is_dirty;
    Bool   is_bmp;

    struct xorg_list link;
} xDbgDumpBuffer;

typedef struct
{
    Bool init;

    char *type_str;
    char *file_str;
    char *count_str;
    char *crop_str;

    xDbgDumpBufferFunc func;
    int bo_size;

    struct xorg_list *cursor;
    struct xorg_list  buffers;

    int   type;
    int   count;
    xRectangle *crop;
} xDbgDumpInfo;

static xDbgDumpInfo xdbg_dump_info;

static int
_parse_int (char *s)
{
    char *fmt = "%lu";
    int retval = 0;
    int thesign = 1;

    if (s && s[0])
    {
        char temp[12];
        snprintf (temp, sizeof (temp), "%s", s);
        s = temp;

        if (s[0] == '-')
            s++, thesign = -1;
        if (s[0] == '0')
            s++, fmt = "%o";
        if (s[0] == 'x' || s[0] == 'X')
            s++, fmt = "%x";
        (void) sscanf (s, fmt, &retval);
    }
    return (thesign * retval);
}

static Bool
_xDbgDumpEnsureDir (void)
{
    char *dir = DUMP_DIR;
    DIR *dp;
    int ret;

    if (!(dp = opendir (dir)))
    {
        ret = mkdir (dir, 0755);
        if (ret < 0)
        {
            XDBG_ERROR (MXDBG, "fail: mkdir '%s'\n", DUMP_DIR);
            return FALSE;
        }
    }
    else
        closedir (dp);

    return TRUE;
}

static void
_xDbgDumpSetOptions (void)
{
    char options[256];
    int tempsize = sizeof (options);
    char *reply = options;
    int *len = &tempsize;

    options[0] = '\0';

    /* type */
    if (xdbg_dump_info.type_str)
    {
        char *c = xdbg_dump_info.type_str;
        if (!strcmp (c, "drawable"))
            xdbg_dump_info.type = XDBG_DUMP_TYPE_DRAWABLE;
        else if (!strcmp (c, "fb"))
            xdbg_dump_info.type = XDBG_DUMP_TYPE_FB;
        else if (!strcmp (c, "ui"))
            xdbg_dump_info.type = XDBG_DUMP_TYPE_UI;
        else if (!strcmp (c, "video"))
            xdbg_dump_info.type = XDBG_DUMP_TYPE_VIDEO;
        else
            xdbg_dump_info.type = _parse_int (c);
    }
    else
        xdbg_dump_info.type = XDBG_DUMP_TYPE_UI;
    XDBG_REPLY ("type(0x%x) ", xdbg_dump_info.type);

    /* count */
    if (xdbg_dump_info.count_str)
        xdbg_dump_info.count = atoi (xdbg_dump_info.count_str);
    else
        xdbg_dump_info.count = DUMP_BUFCNT;
    XDBG_REPLY ("count(%d) ", xdbg_dump_info.count);

    /* file */
    if (xdbg_dump_info.file_str)
        XDBG_REPLY ("file(%s) ", xdbg_dump_info.file_str);

    /* crop */
    if (xdbg_dump_info.crop_str)
    {
        int i;
        char temp[64];
        char *c;
        int nums[4];

        if (xdbg_dump_info.crop)
        {
            free (xdbg_dump_info.crop);
            xdbg_dump_info.crop = NULL;
        }

        snprintf (temp, sizeof (temp), "%s", xdbg_dump_info.crop_str);

        c = strtok (temp, ",");
        i = 0;
        while (c != NULL)
        {
            nums[i++] = atoi(c);
            c = strtok (NULL, ",");
            if (i == 4)
                break;
        }

        if (i == 4)
        {
            xdbg_dump_info.crop = calloc (1, sizeof (xRectangle));
            XDBG_RETURN_IF_FAIL (xdbg_dump_info.crop != NULL);

            xdbg_dump_info.crop->x = nums[0];
            xdbg_dump_info.crop->y = nums[1];
            xdbg_dump_info.crop->width = nums[2];
            xdbg_dump_info.crop->height = nums[3];

            XDBG_REPLY ("crop(%d,%d %dx%d) ",
                        nums[0], nums[1], nums[2], nums[3]);
        }
    }

    XDBG_DEBUG (MXDBG, "%s\n", options);
}

static void
_xDbgDumpInit (void)
{
    if (xdbg_dump_info.init)
        return;

    xdbg_dump_info.init = TRUE;
    xdbg_dump_info.cursor = NULL;
    xorg_list_init (&xdbg_dump_info.buffers);
}

static int
_xDbgDumpBmp (const char * file, const void * data, int width, int height)
{
    int i;

    struct
    {
        unsigned char magic[2];
    } bmpfile_magic = { {'B', 'M'} };

    struct
    {
        unsigned int filesz;
        unsigned short creator1;
        unsigned short creator2;
        unsigned int bmp_offset;
    } bmpfile_header = { 0, 0, 0, 0x36 };

    struct
    {
        unsigned int header_sz;
        unsigned int width;
        unsigned int height;
        unsigned short nplanes;
        unsigned short bitspp;
        unsigned int compress_type;
        unsigned int bmp_bytesz;
        unsigned int hres;
        unsigned int vres;
        unsigned int ncolors;
        unsigned int nimpcolors;
    } bmp_dib_v3_header_t = { 0x28, 0, 0, 1, 24, 0, 0, 0, 0, 0, 0 };
    unsigned int * blocks;

    XDBG_RETURN_VAL_IF_FAIL (file != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (data != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (width > 0, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (height > 0, FALSE);

    FILE * fp = fopen (file, "w+");
    XDBG_RETURN_VAL_IF_FAIL (fp != NULL, FALSE);

    bmpfile_header.filesz = sizeof (bmpfile_magic) + sizeof (bmpfile_header) +
                            sizeof (bmp_dib_v3_header_t) + width * height * 3;
    bmp_dib_v3_header_t.header_sz = sizeof (bmp_dib_v3_header_t);
    bmp_dib_v3_header_t.width = width;
    bmp_dib_v3_header_t.height = -height;
    bmp_dib_v3_header_t.nplanes = 1;
    bmp_dib_v3_header_t.bmp_bytesz = width * height * 3;

    fwrite (&bmpfile_magic, sizeof (bmpfile_magic), 1, fp);
    fwrite (&bmpfile_header, sizeof (bmpfile_header), 1, fp);
    fwrite (&bmp_dib_v3_header_t, sizeof (bmp_dib_v3_header_t), 1, fp);

    blocks = (unsigned int*)data;
    for (i=0; i<height * width; i++)
        fwrite (&blocks[i], 3, 1, fp);

    fclose (fp);

    XDBG_TRACE (MXDBG, "%s saved\n", file);

    return TRUE;
}

static Bool
_xDbgDumpRaw (const char * file, const void * data, int size)
{
    XDBG_RETURN_VAL_IF_FAIL (file != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (data != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (size > 0, FALSE);

    FILE * fp = fopen (file, "w+");
    XDBG_RETURN_VAL_IF_FAIL (fp != NULL, FALSE);

    unsigned int *blocks = (unsigned int*)data;
    fwrite (blocks, 1, size, fp);
    fclose (fp);

    XDBG_TRACE (MXDBG, "%s saved\n", file);

    return TRUE;
}

API Bool
xDbgDumpSetType (char *type_str)
{
    XDBG_RETURN_VAL_IF_FAIL (type_str != NULL, FALSE);

    _xDbgDumpInit ();

    if (!xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_ERROR (MXDBG, "can't set.\n");
        return FALSE;
    }

    if (xdbg_dump_info.type_str)
        free (xdbg_dump_info.type_str);

    xdbg_dump_info.type_str = strdup (type_str);
    XDBG_DEBUG (MXDBG, "type_str: %s\n", xdbg_dump_info.type_str);

    return TRUE;
}

API Bool
xDbgDumpSetFile (char *file_str)
{
    XDBG_RETURN_VAL_IF_FAIL (file_str != NULL, FALSE);

    _xDbgDumpInit ();

    if (!xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_ERROR (MXDBG, "can't set.\n");
        return FALSE;
    }

    if (xdbg_dump_info.file_str)
        free (xdbg_dump_info.file_str);

    xdbg_dump_info.file_str = strdup (file_str);
    XDBG_DEBUG (MXDBG, "file_str: %s\n", xdbg_dump_info.file_str);

    return TRUE;
}

API Bool
xDbgDumpSetCount (char *count_str)
{
    XDBG_RETURN_VAL_IF_FAIL (count_str != NULL, FALSE);

    _xDbgDumpInit ();

    if (!xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_ERROR (MXDBG, "can't set.\n");
        return FALSE;
    }

    if (xdbg_dump_info.count_str)
        free (xdbg_dump_info.count_str);

    xdbg_dump_info.count_str = strdup (count_str);
    XDBG_DEBUG (MXDBG, "count_str: %s\n", xdbg_dump_info.count_str);

    return TRUE;
}

API Bool
xDbgDumpSetCrop (char *crop_str)
{
    XDBG_RETURN_VAL_IF_FAIL (crop_str != NULL, FALSE);

    _xDbgDumpInit ();

    if (!xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_ERROR (MXDBG, "can't set.\n");
        return FALSE;
    }

    if (xdbg_dump_info.crop_str)
        free (xdbg_dump_info.crop_str);

    xdbg_dump_info.crop_str = strdup (crop_str);
    XDBG_DEBUG (MXDBG, "crop_str: %s\n", xdbg_dump_info.crop_str);

    return TRUE;
}

API char*
xDbgDumpGetType (void)
{
    return xdbg_dump_info.type_str;
}

API char*
xDbgDumpGetFile (void)
{
    return xdbg_dump_info.file_str;
}

API char*
xDbgDumpGetCount (void)
{
    return xdbg_dump_info.count_str;
}

API char*
xDbgDumpGetCrop (void)
{
    return xdbg_dump_info.crop_str;
}

API Bool
xDbgDumpPrepare (void)
{
    int i;

    _xDbgDumpInit ();

    if (!xorg_list_is_empty (&xdbg_dump_info.buffers))
        return TRUE;

    _xDbgDumpSetOptions ();

    for (i = 0; i < xdbg_dump_info.count; i++)
    {
        xDbgDumpBuffer *dumpbuf = calloc (1, sizeof (xDbgDumpBuffer));
        XDBG_GOTO_IF_FAIL (dumpbuf != NULL, fail);

        xorg_list_add (&dumpbuf->link, &xdbg_dump_info.buffers);
        dumpbuf->index = i;
        dumpbuf->bo_size = xdbg_dump_info.bo_size;
        if (xdbg_dump_info.func.alloc)
        {
            dumpbuf->bo = xdbg_dump_info.func.alloc (dumpbuf->bo_size);
            XDBG_GOTO_IF_FAIL (dumpbuf->bo != NULL, fail);
        }
    }

    xdbg_dump_info.cursor = &xdbg_dump_info.buffers;

    return TRUE;
fail:
    xDbgDumpClear ();
    return FALSE;
}

API void
xDbgDumpSave (void)
{
    xDbgDumpBuffer *cur = NULL, *next = NULL;

    _xDbgDumpInit ();

    if (!_xDbgDumpEnsureDir ())
        return;

    xorg_list_for_each_entry_safe (cur, next, &xdbg_dump_info.buffers, link)
    {
        char file[128];
        void *ptr;

        if (!cur->is_dirty)
            continue;

        if (xdbg_dump_info.func.map)
            ptr = xdbg_dump_info.func.map (cur->bo);
        else
            ptr = cur->bo;
        XDBG_GOTO_IF_FAIL (ptr != NULL, reset_dump);

        snprintf (file, sizeof(file), "%s/%s", DUMP_DIR, cur->file);

        if (cur->is_bmp)
        {
            unsigned int *p = (unsigned int*)ptr;
            int i, j;

            /* fill magenta color(#FF00FF) for background */
            for (j = 0; j < cur->u.a.dump_h; j++)
                for (i = 0; i < cur->u.a.dump_w ; i++)
                {
                    if (i >= cur->u.a.dump_rect.x && i < (cur->u.a.dump_rect.x + cur->u.a.dump_rect.width))
                        if (j >= cur->u.a.dump_rect.y && j < (cur->u.a.dump_rect.y + cur->u.a.dump_rect.height))
                            continue;
                    p[i + j * cur->u.a.dump_w] = 0xFFFF00FF;
                }

            _xDbgDumpBmp (file, ptr, cur->u.a.dump_w, cur->u.a.dump_h);

            if (xdbg_dump_info.file_str && !strcmp (xdbg_dump_info.file_str, "raw"))
            {
                snprintf (file, sizeof(file), "%s/%s.raw", DUMP_DIR, cur->file);
                _xDbgDumpRaw (file, ptr, cur->u.a.dump_w*cur->u.a.dump_h*4);
            }
        }
        else
        {
            _xDbgDumpRaw (file, ptr, cur->u.dump_size);
        }

        if (xdbg_dump_info.func.unmap)
            xdbg_dump_info.func.unmap (cur->bo);

reset_dump:
        cur->file[0] = '\0';
        memset (&cur->u, 0, sizeof (cur->u));
        cur->is_dirty = FALSE;
        cur->is_bmp = FALSE;
    }
}

API void
xDbgDumpClear (void)
{
    xDbgDumpBuffer *cur = NULL, *next = NULL;

    _xDbgDumpInit ();

    xorg_list_for_each_entry_safe (cur, next, &xdbg_dump_info.buffers, link)
    {
        if (xdbg_dump_info.func.free)
            xdbg_dump_info.func.free (cur->bo);
        xorg_list_del (&cur->link);
        free (cur);        
    }

    xdbg_dump_info.cursor = NULL;

    xdbg_dump_info.type = XDBG_DUMP_TYPE_NONE;
    xdbg_dump_info.count = 0;
    if (xdbg_dump_info.crop)
    {
        free (xdbg_dump_info.crop);
        xdbg_dump_info.crop = NULL;
    }

    XDBG_DEBUG (MXDBG, "\n");
}

API Bool
xDbgDumpSetBufferFunc (xDbgDumpBufferFunc *func, int bo_size)
{
    XDBG_RETURN_VAL_IF_FAIL (bo_size > 0, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (func != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (func->alloc != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (func->free != NULL, FALSE);
    if (func->map)
        XDBG_RETURN_VAL_IF_FAIL (func->unmap != NULL, FALSE);

    _xDbgDumpInit ();

    xdbg_dump_info.func = *func;
    xdbg_dump_info.bo_size = bo_size;

    XDBG_INFO (MXDBG, "\n");

    return TRUE;
}

API Bool
xDbgDumpIsEnable (xDbgDumpType type)
{
    return (xdbg_dump_info.type & type) ? TRUE : FALSE;
}

API void
xDbgDumpRaw (void *data, xDbgDumpType type, void *var_buf, const char *file)
{
    xDbgDumpBuffer *dumpbuf;
    struct xorg_list *next_cursor;

    XDBG_RETURN_IF_FAIL (type > 0);
    XDBG_RETURN_IF_FAIL (var_buf != NULL);
    XDBG_RETURN_IF_FAIL (file != NULL);

    _xDbgDumpInit ();
    if (xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_WARNING (MXDBG, "not ready to dump\n");
        return;
    }

    next_cursor = xdbg_dump_info.cursor->next;
    XDBG_RETURN_IF_FAIL (next_cursor != NULL);

    if (next_cursor == &xdbg_dump_info.buffers)
    {
        next_cursor = next_cursor->next;
        XDBG_RETURN_IF_FAIL (next_cursor != NULL);
    }

    dumpbuf = xorg_list_entry (next_cursor, xDbgDumpBuffer, link);
    XDBG_RETURN_IF_FAIL (dumpbuf != NULL);

    if (xdbg_dump_info.func.dumpRaw)
    {
        Bool ret;
        int dump_size = 0;

        ret = xdbg_dump_info.func.dumpRaw (data, type,
                                           var_buf, dumpbuf->bo, dumpbuf->bo_size,
                                           &dump_size);
        XDBG_RETURN_IF_FAIL (ret == TRUE);
        XDBG_RETURN_IF_FAIL (dump_size > 0);
        XDBG_RETURN_IF_FAIL (dump_size <= dumpbuf->bo_size);

        snprintf (dumpbuf->file, sizeof (dumpbuf->file), "%.3f_%s", GetTimeInMillis()/1000.0, file);
        dumpbuf->u.dump_size = dump_size;
        dumpbuf->is_dirty = TRUE;
        dumpbuf->is_bmp = FALSE;

        xdbg_dump_info.cursor = next_cursor;
    }

    XDBG_DEBUG (MXDBG, "type:0x%x file: %s\n", type, file);
}

API void
xDbgDumpBmp (void *data, xDbgDumpType type, void *var_buf, const char *file)
{
    xDbgDumpBuffer *dumpbuf;
    struct xorg_list *next_cursor;

    XDBG_RETURN_IF_FAIL (type > 0);
    XDBG_RETURN_IF_FAIL (var_buf != NULL);
    XDBG_RETURN_IF_FAIL (file != NULL);

    _xDbgDumpInit ();
    if (xorg_list_is_empty (&xdbg_dump_info.buffers))
    {
        XDBG_WARNING (MXDBG, "not ready to dump\n");
        return;
    }

    next_cursor = xdbg_dump_info.cursor->next;
    XDBG_RETURN_IF_FAIL (next_cursor != NULL);

    if (next_cursor == &xdbg_dump_info.buffers)
    {
        next_cursor = next_cursor->next;
        XDBG_RETURN_IF_FAIL (next_cursor != NULL);
    }

    dumpbuf = xorg_list_entry (next_cursor, xDbgDumpBuffer, link);
    XDBG_RETURN_IF_FAIL (dumpbuf != NULL);

    if (xdbg_dump_info.func.dumpBmp)
    {
        Bool ret;
        int dump_w = 0, dump_h = 0;
        xRectangle dump_rect = {0,};

        ret = xdbg_dump_info.func.dumpBmp (data, type,
                                           var_buf, dumpbuf->bo, dumpbuf->bo_size,
                                           &dump_w, &dump_h, &dump_rect);
        XDBG_RETURN_IF_FAIL (ret == TRUE);
        XDBG_RETURN_IF_FAIL (dump_w > 0);
        XDBG_RETURN_IF_FAIL (dump_h > 0);
        XDBG_RETURN_IF_FAIL (dump_rect.width > 0);
        XDBG_RETURN_IF_FAIL (dump_rect.height > 0);

        snprintf (dumpbuf->file, sizeof (dumpbuf->file), "%.3f_%s", GetTimeInMillis()/1000.0, file);
        dumpbuf->u.a.dump_w = dump_w;
        dumpbuf->u.a.dump_h = dump_h;
        dumpbuf->u.a.dump_rect = dump_rect;
        dumpbuf->is_dirty = TRUE;
        dumpbuf->is_bmp = TRUE;

        xdbg_dump_info.cursor = next_cursor;
    }

    XDBG_DEBUG (MXDBG, "type:0x%x file: %s\n", type, file);
}

API Bool
xDbgDumpReplaceBuffer (void *old_dump_buf, void *new_dump_buf, int new_dump_buf_size)
{
    xDbgDumpBuffer *cur = NULL, *next = NULL;

    XDBG_RETURN_VAL_IF_FAIL (new_dump_buf != NULL, FALSE);
    XDBG_RETURN_VAL_IF_FAIL (new_dump_buf_size > 0, FALSE);

    _xDbgDumpInit ();

    xorg_list_for_each_entry_safe (cur, next, &xdbg_dump_info.buffers, link)
    {
        if (cur->bo == old_dump_buf)
        {
            if (xdbg_dump_info.func.free)
                xdbg_dump_info.func.free (cur->bo);

            cur->bo = new_dump_buf;
            cur->bo_size = new_dump_buf_size;
            return TRUE;
        }
    }

    return FALSE;
}

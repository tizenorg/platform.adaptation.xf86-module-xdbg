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

#if defined(XDBG_CLIENT)
#error "This header is not for client."
#endif

#ifndef __XDBG_DUMP_H__
#define __XDBG_DUMP_H__

#include <X11/Xdefs.h>
#include <X11/Xprotostr.h>

/* "xdbg dump" is designed for "x video driver" to dump various buffers without
 * losing performance. When it starts, it allocates 50 dump-buffers by default.
 * You can change the count of dump-buffers with "-count" option. After starting,
 * various buffers will be copied into dump-buffers. Finally when it stops,
 * all copied buffers will be saved into "/tmp/xdump" directory.
 * You can use "-type" option to distinguish the type of various buffers inside of
 * x video driver.
 * 
 * # xdbg dump on                           //begin
 * # xdbg dump off                          //off with saving
 * # xdbg dump clear                        //off without saving 
 * # xdbg dump -count [n]
 * # xdbg dump -type [drawable|fb|ui|video]
 * # xdbg dump on -type ui -count 100
 * # xdbg dump -type 0x2000                 //user-defined type(number only)
 * # xdbg dump -crop 10,10,200,200          //dump only (10,10 200x200) area
 */
typedef enum
{
    XDBG_DUMP_TYPE_NONE     = 0x00000000,
    XDBG_DUMP_TYPE_DRAWABLE = 0x00000001,
    XDBG_DUMP_TYPE_FB       = 0x00000010,
    XDBG_DUMP_TYPE_UI       = 0x00000011,
    XDBG_DUMP_TYPE_VIDEO    = 0x00000100,
} xDbgDumpType;

/* "xDbgDumpBufferFunc" should be implemented in "x video driver". */
typedef struct
{
    /* [in ] dump_buf_size: size of dump-buffer
     * [out] a newly-allocated dump-buffer
     */
    void* (*alloc)   (int dump_buf_size);

    /* map() should return virtual address of dump-buffer. If alloc() returns
     * virtual address, map() and unmap() can be NULL. Don't need to implement.
     * [in ] dump_buf: a dump-buffer
     * [out] virtual address of a dump-buffer
     */
    void* (*map)     (void *dump_buf);
    void  (*unmap)   (void *dump_buf);
    void  (*free)    (void *dump_buf);

    /* xDbgDumpBmp() calls dumpBum(). Returned dump_w, dump_h and dump_rect will
     * be used when a dump-buffer is saved to a bmp file.
     * If you don't call xDbgDumpBmp(), dumpBmp() can be NULL.
     * [in ] data: user-data
     * [in ] type: dump-type
     * [in ] var_buf: various buffer
     * [in ] dump_buf: a dump-buffer
     * [in ] dump_buf_size: size of dump-buffer
     * [out] dump_w: a dump-buffer's width
     * [out] dump_h a dump-buffer's height
     * [out] dump_rect: rect which contains copied-image within a dump-buffer
     * [out] TRUE if success to copy, FALSE if fail
     */
    Bool  (*dumpBmp) (void *data, xDbgDumpType type,
                      void *var_buf, void *dump_buf, int dump_buf_size,
                      int *dump_w, int *dump_h, xRectangle *dump_rect);

    /* xDbgDumpRaw() calls dumpRaw(). Returned dump_size will be used when a
     * dump-buffer is saved to a raw file.
     * If you don't call xDbgDumpRaw(), dumpRaw() can be NULL.
     * [in ] data: user-data
     * [in ] type: dump-type
     * [in ] var_buf: various buffer
     * [in ] dump_buf: a dump-buffer
     * [in ] dump_buf_size: size of dump-buffer
     * [out] dump_size: size which contains copied-image in a dump-buffer
     * [out] TRUE if success to copy, FALSE if fail
     */
    Bool  (*dumpRaw) (void *data, xDbgDumpType type,
                      void *var_buf, void *dump_buf, int dump_buf_size,
                      int *dump_size);

    void (*reserved1) (void);
    void (*reserved2) (void);
    void (*reserved3) (void);
    void (*reserved4) (void);
} xDbgDumpBufferFunc;

/* Register dump-buffer-functions and set max-size of a dump-buffer.
 * [in ] func: dump-buffer-functions
 * [in ] dump_buf_size: dump-buffer size. alloc() receives this size.
 * [out] TRUE if success, FALSE if fail
 */
Bool    xDbgDumpSetBufferFunc (xDbgDumpBufferFunc *func, int dump_buf_size);

/* Check if "xdbg dump" is on.
 * [in ] type: dump-type
 * [out] TRUE if on, FALSE if off
 *
 * Sample:
 *   if (xDbgDumpIsEnable (XDBG_DUMP_TYPE_UI))
 *       xDbgDumpBmp (scrn, XDBG_DUMP_TYPE_UI, pPixmap, "drawable_1.bmp");
 *
 *   if (xDbgDumpIsEnable (XDBG_DUMP_TYPE_VIDEO))
 *       xDbgDumpRaw (scrn, XDBG_DUMP_TYPE_VIDEO, yuv_buffer, "video_1.yuv");
 */
Bool    xDbgDumpIsEnable (xDbgDumpType type);

/* Dump various buffer as raw file
 * [in ] data: user-data
 * [in ] type: dump-type
 * [in ] var_buf: various buffer
 * [in ] file: filename to save
 */
void    xDbgDumpRaw (void *data, xDbgDumpType type, void *var_buf, const char *file);

/* Dump various buffer as bmp file
 * [in ] data: user-data
 * [in ] type: dump-type
 * [in ] var_buf: various buffer
 * [in ] file: filename to save
 */
void    xDbgDumpBmp (void *data, xDbgDumpType type, void *var_buf, const char *file);

/* Replace a old dump-buffer to a new dump-buffer.
 * [in ] old_dump_buf: old dump-buffer
 * [in ] new_dump_buf: new dump-buffer
 * [in ] new_dump_buf_size: new dump-buffer's size
 * [out] TRUE if success, FALSE if fail
 */
Bool    xDbgDumpReplaceBuffer (void *old_dump_buf, void *new_dump_buf, int new_dump_buf_size);

#endif  /* __XDBG_DUMP_H__ */

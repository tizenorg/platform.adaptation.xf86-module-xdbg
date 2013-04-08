/**************************************************************************

xevlog-analyze

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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <X11/Xlib.h>
#include <X11/Xw32defs.h>
#include <xdbg_types.h>
#include <xdbg_evlog.h>

typedef struct _EvlogOption
{
    int pid;
    Bool help_opt;
    char command_name[PATH_MAX+1];
    char path_name[PATH_MAX+1];
} EvlogOption;

#define BUF_SIZE 256


static void
_printUsage(char* name)
{
    fprintf(stderr, "Usage: %s [OPTION]...\n", name);
    fprintf(stderr, "\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "       -f [File_Name]  File to save information\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "       -h              Command Usage information\n");
    fprintf(stderr, "\n");
}

static void _xEvlogAnalyzePrint (EvlogOption *eo)
{
    int fd = -1, cfd = -1;
    int total, read_len;
    int evlog_len;
    char fd_name[256];
    EvlogInfo evinfo={0,};

    if (eo->help_opt)
    {
        _printUsage(eo->command_name);
        return;
    }

    if (!eo->path_name)
    {
        printf ("failed: no evlog path\n");
        return;
    }

    fd = open (eo->path_name, O_RDONLY);
    if (fd < 0)
    {
        printf ("failed: open '%s'. (%s)\n", eo->path_name, strerror(errno));
        return;
    }

    snprintf (fd_name, sizeof (fd_name), "/proc/%d/fd/1", eo->pid);
    cfd = open (fd_name, O_RDWR);

    if (cfd < 0)
    {
        printf ("failed: open consol '%s'. (%s)\n", fd_name, strerror(errno));
        goto print_done;
    }

    while ((read_len = read (fd, &evlog_len, sizeof (int))) == sizeof (int))
    {
        char log[1024];
        int size = sizeof (log);

        total = read_len;

        read_len = read (fd, &evinfo.time, sizeof (CARD32));
        GOTO_IF_FAIL (read_len == sizeof (CARD32), print_done);
        total += read_len;

        read_len = read (fd, &evinfo.type, sizeof (EvlogType));
        GOTO_IF_FAIL (read_len == sizeof (EvlogType), print_done);
        GOTO_IF_FAIL (evinfo.type >= EVENT && evinfo.type <= FLUSH, print_done);
        total += read_len;

        read_len = read (fd, &evinfo.mask, sizeof (int));
        GOTO_IF_FAIL (read_len == sizeof (int), print_done);
        total += read_len;

        if (evinfo.mask & EVLOG_MASK_CLIENT)
        {
            read_len = read (fd, &evinfo.client, sizeof (EvlogClient));
            GOTO_IF_FAIL (read_len == sizeof (EvlogClient), print_done);
            total += read_len;
        }

        if (evinfo.mask & EVLOG_MASK_REQUEST)
        {
            read_len = read (fd, &evinfo.req, sizeof(EvlogRequest));
            GOTO_IF_FAIL (read_len == sizeof(EvlogRequest), print_done);
            total += read_len;

            evinfo.req.ptr = malloc (sizeof(xReq));
            GOTO_IF_FAIL (evinfo.req.ptr != NULL, print_done);

            read_len = read (fd, evinfo.req.ptr, sizeof (xReq));
            GOTO_IF_FAIL (read_len == sizeof(xReq), print_done);
            total += read_len;
        }

        else if (evinfo.mask & EVLOG_MASK_EVENT)
        {
            read_len = read (fd, &evinfo.evt, sizeof(EvlogEvent));
            GOTO_IF_FAIL (read_len == sizeof(EvlogEvent), print_done);
            total += read_len;

            evinfo.evt.ptr = malloc (sizeof(xEvent));
            GOTO_IF_FAIL (evinfo.evt.ptr != NULL, print_done);

            read_len = read (fd, evinfo.evt.ptr, sizeof (xEvent));
            GOTO_IF_FAIL (read_len == sizeof(xEvent), print_done);
            total += read_len;
        }
        GOTO_IF_FAIL (evlog_len == total, print_done);

        xDbgEvlogFillLog(&evinfo, log, &size);
        printf ("%s", log);

        if (evinfo.req.ptr)
        {
            free (evinfo.req.ptr);
            evinfo.req.ptr = NULL;
        }

        else if (evinfo.evt.ptr)
        {
            free (evinfo.evt.ptr);
            evinfo.evt.ptr = NULL;
        }
    }


print_done:
    if (evinfo.req.ptr)
        free (evinfo.req.ptr);

    else if (evinfo.evt.ptr)
        free (evinfo.evt.ptr);

    if (cfd >= 0)
        close (cfd);

    if (fd >= 0)
        close (fd);
}

static void
_checkOption(int argc, char** argv)
{
    int c;
    int opt_str_len = 0;
    char* opt_str = NULL;
    EvlogOption eo = {0,};

    eo.pid = atoi (argv[0]);
    eo.help_opt = TRUE;
    strncpy(eo.command_name, argv[1], PATH_MAX);

    if (argc <= 2)
    {
        _printUsage( eo.command_name );
        return;
    }

    while ((c = getopt(argc, argv, "f:h")) != EOF)
    {
        switch (c)
        {

            case 'f':
                opt_str = optarg;
                opt_str_len = strlen(opt_str);

                if(opt_str_len > 0)
                {
                    eo.help_opt = FALSE;
                    strncpy (eo.path_name, opt_str, opt_str_len);
                }
                break;

            case 'h':
                _printUsage( eo.command_name );
                return;
                break;

            default:
                break;
        }
    }

    _xEvlogAnalyzePrint(&eo);
}


int main(int argc, char** argv)
{

    Display *dpy;
    char **new_argv;
    int new_argc, i;
    char temp[128];

    dpy = XOpenDisplay (NULL);
    if (!dpy)
    {
        fprintf (stderr, "failed: open display\n");
        exit (-1);
    }

    new_argc = argc + 1;
    new_argv = (char**)malloc (new_argc * sizeof (char*));
    if (!new_argv)
    {
        fprintf (stderr, "failed: malloc new argv\n");
        exit (-1);
    }

    snprintf (temp, sizeof(temp), "%d", (int)getpid());
    new_argv[0] = temp;

    for (i = 0; i < argc; i++)
        new_argv[i+1] = argv[i];

    _checkOption(new_argc, new_argv);

    free (new_argv);

    XCloseDisplay (dpy);

    return 0;
}

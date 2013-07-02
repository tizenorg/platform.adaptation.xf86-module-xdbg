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

#define LOG_SIZE 1024

typedef struct _EvlogOption
{
    int    pid;
    char   command_name[PATH_MAX+1];
    char   path_name[PATH_MAX+1];
    Bool   isRule;
    int    detail_level;
} EvlogOption;


static void
_printUsage(char* name)
{
    printf("Usage: %s [OPTION]...\n", name);
    printf("\n");
    printf(" Options:\n");
    printf("       -f [File_Name]      File to save information\n");
    printf("\n");
    printf("       -r [Rule_File_Name] Setting Rule of certain policy read in File to find information you want\n");
    printf("\n");
    printf("              [How To write Rule file form]\n");
    printf("                ------------------------\n");
    printf("                | allow [RULE]          |\n");
    printf("                | deny [RULE]           |\n");
    printf("                ------------------------\n");
    printf("           * It is possible to write multiple policies.\n");
    printf("\n");
    printf("       -a [ALLOW Rule]       Setting Rule of 'ALLOW' policy to find information you want\n");
    printf("\n");
    printf("       -n [Deny Rule]      Setting Rule of 'Deny' policy to find information you want\n");
    printf("\n");
    printf("              [RULE] : C Language-style boolean expression syntax. [VARIABLE] [COMPAROTOR] [VALUE]\n");
    printf("              [VARIABLE] : type / major / minor / command / cmd / pid\n");
    printf("              [COMPARATOR] : & / && / and / | / || / or / = / == / != / > / >= / < / <=\n");
    printf("              [VALUE] : string / number  \n");
    printf("\n");
    printf("           ie)\n");
    printf("               xevlog_analyze -a \"(type=request) && (major == X11 and (minor = SendEvent or minor = ReceiveEvent))\"\n");
    printf("               xevlog_analyze -n cmd!=ls\n");
    printf("\n");
    printf("           * WARNING : If you set both -a and -n option, must set -a option first. Otherwise Logs you DO NOT want can be printed.\n");
    printf("\n");
    printf("       -d [0-2]            To Set printing detail log level\n");
    printf("\n");
    printf("               0: To Print Primary Logs (XID, Root, Atom, Region ...)\n");
    printf("               1: To Print More Detail Logs (Time, State, Mask ...\n");
    printf("               2: To Print Supplementary Reply Logs (Information Including Each Item)\n");
    printf("\n");
    printf("       -h                  Usage of xevlog_anlayze\n");
    printf("\n");
}

static void _xEvlogAnalyzePrint (EvlogOption *eo, char* reply, int* len)
{
    int fd = -1, cfd = -1;
    int total, read_len;
    int evlog_len;
    char fd_name[256];
    EvlogInfo evinfo={0,};
    int i;

    if (!strlen(eo->path_name))
    {
        printf ("failed: no evlog path\n");
        _printUsage(eo->command_name);
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
        char log[LOG_SIZE];
        int size = sizeof (log);
        static int init = 0;

        memset (&evinfo, 0, sizeof (EvlogInfo));
        total = read_len;

        if (!init)
        {
            extern ExtensionInfo Evlog_extensions[];
            extern int Extensions_size;
            int new_argc = 3;
            char* new_argv[3] = {"add", "allow", "all"};
            int i;

            if(!eo->isRule)
                xDbgEvlogRuleSet ((const int) new_argc, (const char**) new_argv, reply, len);

            printf("%s\n", reply);

            read_len = read (fd, &Extensions_size, sizeof (int));
            GOTO_IF_FAIL (read_len == sizeof (int), print_done);
            total += read_len;

            for (i = 0 ; i < Extensions_size ; i++)
            {
                read_len = read (fd, &Evlog_extensions[i].opcode, sizeof (int));
                GOTO_IF_FAIL (read_len == sizeof (int), print_done);
                total += read_len;

                read_len = read (fd, &Evlog_extensions[i].evt_base, sizeof (int));
                GOTO_IF_FAIL (read_len == sizeof (int), print_done);
                total += read_len;

                read_len = read (fd, &Evlog_extensions[i].err_base, sizeof (int));
                GOTO_IF_FAIL (read_len == sizeof (int), print_done);
                total += read_len;
            }

            if (!xDbgEvlogGetExtensionEntry ())
            {
                printf ("failed: get extentions\n");
                goto print_done;
            }

            init = 1;
        }

        read_len = read (fd, &evinfo.time, sizeof (CARD32));
        GOTO_IF_FAIL (read_len == sizeof (CARD32), print_done);
        total += read_len;

        read_len = read (fd, &evinfo.type, sizeof (EvlogType));
        GOTO_IF_FAIL (read_len == sizeof (EvlogType), print_done);
        GOTO_IF_FAIL (evinfo.type >= EVENT && evinfo.type <= ERROR, print_done);
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
            int size;

            read_len = read (fd, &evinfo.req, sizeof(EvlogRequest));
            GOTO_IF_FAIL (read_len == sizeof(EvlogRequest), print_done);
            total += read_len;

            size = (int)(evinfo.req.length * 4);
            GOTO_IF_FAIL (size > 0, print_done);

            evinfo.req.ptr = malloc (size);
            GOTO_IF_FAIL (evinfo.req.ptr != NULL, print_done);

            read_len = read (fd, evinfo.req.ptr, size);
            GOTO_IF_FAIL (read_len == size, print_done);
            total += read_len;
        }

        if (evinfo.mask & EVLOG_MASK_EVENT)
        {
            read_len = read (fd, &evinfo.evt, sizeof(EvlogEvent));
            GOTO_IF_FAIL (read_len == sizeof(EvlogEvent), print_done);
            total += read_len;

            evinfo.evt.ptr = malloc (evinfo.evt.size);
            GOTO_IF_FAIL (evinfo.evt.ptr != NULL, print_done);

            WARNING_IF_FAIL (evinfo.evt.size > 0);

            read_len = read (fd, evinfo.evt.ptr, evinfo.evt.size);
            GOTO_IF_FAIL (read_len == evinfo.evt.size, print_done);
            total += read_len;
        }

        if (evinfo.mask & EVLOG_MASK_REPLY)
        {
            read_len = read (fd, &evinfo.rep, sizeof(EvlogReply));
            GOTO_IF_FAIL (read_len == sizeof(EvlogReply), print_done);
            total += read_len;

            evinfo.rep.ptr = malloc (evinfo.rep.size);
            GOTO_IF_FAIL (evinfo.rep.ptr != NULL, print_done);

            WARNING_IF_FAIL (evinfo.rep.size > 0);

            read_len = read (fd, evinfo.rep.ptr, evinfo.rep.size);
            GOTO_IF_FAIL (read_len == evinfo.rep.size, print_done);
            total += read_len;
        }

        if (evinfo.mask & EVLOG_MASK_ERROR)
        {
            read_len = read (fd, &evinfo.err, sizeof(EvlogError));
            GOTO_IF_FAIL (read_len == sizeof(EvlogError), print_done);
            total += read_len;
        }

        if (evinfo.mask & EVLOG_MASK_ATOM)
        {
            EvlogAtomTable *table;

            read_len = read (fd, &evinfo.evatom.size, sizeof (int));
            GOTO_IF_FAIL (read_len == sizeof(int), print_done);
            total += read_len;

            for (i = 0 ; i < evinfo.evatom.size ; i++)
            {
                table = malloc (sizeof(EvlogAtomTable));
                GOTO_IF_FAIL (table != NULL, print_done);

                if (!evinfo.evatom.init)
                {
                    xorg_list_init(&evinfo.evatom.list);
                    evinfo.evatom.init = 1;
                }

                read_len = read (fd, table, sizeof (EvlogAtomTable));
                if (read_len != sizeof(EvlogAtomTable))
                {
                    WARNING_IF_FAIL (read_len == sizeof(EvlogAtomTable));
                    free (table);
                    goto print_done;
                }
                total += read_len;

                xorg_list_add(&table->link, &evinfo.evatom.list);
            }
        }

        if (evinfo.mask & EVLOG_MASK_REGION)
        {
            EvlogRegionTable *table;

            read_len = read (fd, &evinfo.evregion.size, sizeof (int));
            GOTO_IF_FAIL (read_len == sizeof(int), print_done);
            total += read_len;

            for (i = 0 ; i < evinfo.evregion.size ; i++)
            {
                table = malloc (sizeof(EvlogRegionTable));
                GOTO_IF_FAIL (table != NULL, print_done);

                if (!evinfo.evregion.init)
                {
                    xorg_list_init(&evinfo.evregion.list);
                    evinfo.evregion.init = 1;
                }

                read_len = read (fd, table, sizeof (EvlogRegionTable));
                if (read_len != sizeof(EvlogRegionTable))
                {
                    WARNING_IF_FAIL (read_len == sizeof(EvlogRegionTable));
                    free (table);
                    goto print_done;
                }
                total += read_len;

                xorg_list_add(&table->link, &evinfo.evregion.list);
            }
        }

        GOTO_IF_FAIL (evlog_len == total, print_done);

        if (xDbgEvlogRuleValidate (&evinfo))
        {
            if (xDbgEvlogFillLog(&evinfo, eo->detail_level, log, &size))
                printf ("%s", log);
        }

        if (evinfo.req.ptr)
        {
            free (evinfo.req.ptr);
            evinfo.req.ptr = NULL;
        }

        if (evinfo.evt.ptr)
        {
            free (evinfo.evt.ptr);
            evinfo.evt.ptr = NULL;
        }

        xDbgDistroyAtomList(&evinfo);
        xDbgDistroyRegionList(&evinfo);
    }


print_done:
    if (evinfo.req.ptr)
        free (evinfo.req.ptr);

    if (evinfo.evt.ptr)
        free (evinfo.evt.ptr);

    xDbgDistroyAtomList(&evinfo);
    xDbgDistroyRegionList(&evinfo);

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
    char rule_log[LOG_SIZE];
    int rule_size = sizeof (rule_log);

    eo.pid = atoi (argv[0]);
    eo.isRule = FALSE;
    eo.detail_level = 0;
    strncpy(eo.command_name, argv[1], PATH_MAX);

    if (argc < 3)
    {
        _printUsage( eo.command_name );
        return;
    }

    while ((c = getopt(argc, argv, "f:a:n:r:d:h")) != EOF)
    {
        switch (c)
        {
            case 'f':
                {
                    opt_str = optarg;
                    opt_str_len = strlen(opt_str);

                    if(opt_str_len > 0)
                    {
                        strncpy (eo.path_name, opt_str, PATH_MAX);
                    }
                    break;
                }

            case 'a':
                {
                    opt_str = optarg;
                    opt_str_len = strlen(opt_str);
                    if(opt_str_len > 0)
                    {
                        int new_argc = 3;
                        char* new_argv[3] = {"add", "allow", };

                        new_argv[2] = (char*)malloc (opt_str_len + 1);
                        if(!new_argv[2])
                        {
                            printf ("failed: malloc new_argv[2]\n");
                            return;
                        }

                        strncpy (new_argv[2], opt_str , opt_str_len);
                        if(!xDbgEvlogRuleSet ((const int) new_argc,
                                              (const char**) new_argv,
                                               rule_log, &rule_size))
                        {
                            printf("%s\n", rule_log);
                            return;
                        }
                        eo.isRule = TRUE;

                        free (new_argv[2]);
                    }

                    break;
               }

            case 'n':
                {
                    opt_str = optarg;
                    opt_str_len = strlen(opt_str);

                    if(opt_str_len > 0)
                    {
                        int new_argc = 3;
                        char* new_argv[3] = {"add", "deny", };

                        new_argv[2] = (char*)malloc (opt_str_len + 1);
                        if(!new_argv[2])
                        {
                            printf ("failed: malloc new_argv[2]\n");
                            return;
                        }

                        strncpy (new_argv[2], opt_str , opt_str_len);
                        if(!xDbgEvlogRuleSet ((const int) new_argc,
                                              (const char**) new_argv,
                                               rule_log, &rule_size))
                        {
                            printf("%s\n", rule_log);
                            return;
                        }
                        eo.isRule = TRUE;

                        free (new_argv[2]);
                    }

                    break;
                }

            case 'r':
                {
                    opt_str = optarg;
                    opt_str_len = strlen(opt_str);

                    if(opt_str_len > 0)
                    {
                        if(!xDbgEvlogReadRuleFile(opt_str, rule_log, &rule_size))
                        {
                            printf("%s", rule_log);
                            return;
                        }
                        eo.isRule = TRUE;
                    }
                    break;
                }

            case 'd':
                {
                    opt_str = optarg;
                    opt_str_len = strlen(opt_str);

                    if(opt_str_len > 0)
                    {
                        eo.detail_level = (atoi(optarg));
                        printf ("Detail Level: %d\n", eo.detail_level);
                    }
                    break;
                }

            case 'h':
            case '?':
                {
                    _printUsage( eo.command_name );
                    return;
                    break;
                }

            default:
                break;
        }
    }

    _xEvlogAnalyzePrint(&eo, rule_log, &rule_size);
}



int main(int argc, char** argv)
{
    char **new_argv;
    int new_argc, i;
    char temp[128];

    new_argc = argc + 1;
    new_argv = (char**)malloc (new_argc * sizeof (char*));
    if (!new_argv)
    {
        printf ("failed: malloc new argv\n");
        exit (-1);
    }

    snprintf (temp, sizeof(temp), "%d", (int)getpid());
    new_argv[0] = temp;

    for (i = 0; i < argc; i++)
        new_argv[i+1] = argv[i];

    _checkOption(new_argc, new_argv);

    free (new_argv);

    return 0;
}

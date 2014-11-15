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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <windowstr.h>
#include <xacestr.h>
#include <xdbg.h>
#include "xdbg_types.h"
#include <xf86Priv.h>
#include "xdbg_module.h"
#include "xdbg_module_evlog.h"
#include "xdbg_module_command.h"

#if ENABLE_DLOG
static void
_CommandDLog (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    int on;

    if (argc != 3)
    {
        XDBG_REPLY ("DLog level: %d\n", xDbgGetLogEnableDlog() ? 1 : 0);
        return;
    }

    on = atoi (argv[2]);

    xDbgLogEnableDlog (on);

    XDBG_REPLY ("Success\n");
}
#endif

static Bool
_CommandSetLogFile (int pid, char *path, char *reply, int *len, XDbgModule *pMod)
{
    static int old_stderr = -1;
    char fd_name[XDBG_PATH_MAX];
    int  log_fd = -1;
    FILE *log_fl;

    if (!path || strlen (path) <= 0)
    {
        XDBG_REPLY ("failed: logpath is invalid\n");
        return FALSE;
    }

    if (old_stderr == -1)
        old_stderr = dup (STDERR_FILENO);

    if (!strcmp (path, "console"))
        snprintf (fd_name, XDBG_PATH_MAX, "/proc/%d/fd/1", pid);
    else
    {
        if (path[0] == '/')
            snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
        else
        {
            if (pMod->cwd)
                snprintf (fd_name, XDBG_PATH_MAX, "%s/%s", pMod->cwd, path);
            else
                snprintf (fd_name, XDBG_PATH_MAX, "%s", path);
        }
    }

    log_fl = fopen (fd_name, "a");
    if (!log_fl)
    {
        XDBG_REPLY ("failed: open file(%s)\n", fd_name);
        return FALSE;
    }

    fflush (stderr);
    close (STDERR_FILENO);

    setvbuf (log_fl, NULL, _IOLBF, 512);
    log_fd = fileno (log_fl);

    dup2 (log_fd, STDERR_FILENO);
    fclose (log_fl);

    if (pMod->log_path)
        free (pMod->log_path);

    pMod->log_path = strdup (fd_name);
    XDBG_RETURN_VAL_IF_FAIL (pMod->log_path != NULL, FALSE);

    XDBG_REPLY ("log path: %s\n", pMod->log_path);

    return TRUE;
}


static void
_CommandLogPath (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    if (argc != 3)
    {
        XDBG_REPLY ("log path: %s\n", (pMod->log_path)?pMod->log_path:"stderr");
        return;
    }

    _CommandSetLogFile (pid, argv[2], reply, len, pMod);
}

static void
_CommandSetLogLevel (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    Bool ret;
    char *module_name;
    int level;

    if (argc < 4)
    {
        xDbgLogEnumModules (MODE_WITH_STATUS, reply, len);
        return;
    }

    module_name = argv[2];
    level = atoi (argv[3]);

    if (level < 0 || level >= 5)
    {
        XDBG_REPLY ("Error : Not valid log level %d.\n", level);
        return;
    }

    ret = xDbgLogSetLevel (xDbgLogGetModule (module_name), level);

    if (ret)
        XDBG_REPLY ("Log level for module %s was successfully set to %d\n",
                  module_name, level);
    else
        XDBG_REPLY ("Error : An error was occured during log level setting\n");
}

static void
_CommandEvlog (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    int on;
    extern Bool xev_trace_on;

    if (argc != 3)
    {
        XDBG_REPLY ("Evlog level: %d\n", xev_trace_on ? 1 : 0);
        return;
    }

    on = atoi (argv[2]);

    xDbgModuleEvlogPrintEvents (pMod, on, argv[0], reply, len);

    XDBG_REPLY ("Success\n");
}

static void
_CommandEvlogDetail (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    int level;
    extern int xev_trace_detail_level;

    if (argc != 3)
    {
        XDBG_REPLY ("Detail Level: %d\n", xev_trace_detail_level);
        return;
    }

    level = atoi(argv[2]);

    xDbgModuleEvlogDetail (pMod, level, reply, len);
    XDBG_REPLY ("Success\n");
}

static void
_CommandSetEvlogRule (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    if (argc < 2)
    {
        XDBG_REPLY ("Error : invalid number of arguments.\n");
        return;
    }

    if (argc > 2 && argv[2] && !strcmp (argv[2], "file"))
    {
        if (argc < 4 || !argv[3])
        {
            XDBG_REPLY ("Error : no evlog rule file.\n");
            return;
        }

        if (pMod->evlog_rule_path)
            free (pMod->evlog_rule_path);

        pMod->evlog_rule_path = strdup (argv[3]);
    }

    xDbgModuleEvlogInfoSetRule (pMod, argc - 2, (const char**)&(argv[2]), reply, len);
}

static void
_CommandSetEvlogPath (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod)
{
    if (argc != 3)
    {
        XDBG_REPLY ("evlog path: %s\n", (pMod->evlog_path)?pMod->evlog_path:"none");
        return;
    }

    if (!argv[2] || strlen (argv[2]) <= 0)
    {
        XDBG_REPLY ("invalid option\n");
        return;
    }

    if (pMod->evlog_path)
    {
        free (pMod->evlog_path);
        pMod->evlog_path=NULL;
    }

    if (!xDbgModuleEvlogSetEvlogPath (pMod, pid, argv[2], reply, len))
    {
        XDBG_REPLY ("Error: evlog path(%s)\n", argv[2]);
        return;
    }

    pMod->evlog_path = strdup (argv[2]);

    if (!strcmp (pMod->evlog_path, "console"))
        XDBG_REPLY ("/proc/%d/fd/1", pid);
    else if (pMod->evlog_path[0] == '/')
        XDBG_REPLY ("evlog path: %s\n", pMod->evlog_path);
    else
        XDBG_REPLY ("evlog path: %s/%s\n", pMod->cwd, pMod->evlog_path);
}

static struct
{
    const char *Cmd;
    const char *Description;
    const char *Options;

    int       (*DynamicUsage) (int, char*, int *);
    const char *DetailedUsage;

    void (*func) (int pid, int argc, char **argv, char *reply, int *len, XDbgModule *pMod);
} command_proc[] =
{
#if ENABLE_DLOG
    {
        "dlog", "to enable dlog", "[0-1]",
        NULL, "[OFF:0/ON:1]",
        _CommandDLog
    },
#endif

    {
        "log_path", "to set log path", "[console/filepath]",
        NULL, "[console/filepath]",
        _CommandLogPath
    },

    {
        "log", "to set loglevel", "[MODULE] [0-4]",
        (int(*)(int, char*, int*))xDbgLogEnumModules, "[DEBUG:0/TRACE:1/INFO:2/WARNING:3/ERROR:4]",
        _CommandSetLogLevel
    },

    {
        "evlog", "to print x events", "[0-1]",
        NULL, "[OFF:0/ON:1]",
        _CommandEvlog
    },

    {
        "evlog_detail", "to set printing detail log level", "[0-2]",
        NULL, "[Primary logs:0/ More detail logs:1/ Supplementary Reply logs:2]",
        _CommandEvlogDetail
    },

    {
        "evlog_rule", "to set evlog rules", "[add/remove/file/print/help]",
        NULL, "[add allow/deny rules / remove (index) / file(file_name) / print / help]",
        _CommandSetEvlogRule
    },

    {
        "evlog_path", "to set filepath of evlog", "[console/filepath]",
        NULL, "[console/filepath]",
        _CommandSetEvlogPath
    },
};

static void _CommandPrintUsage (char *reply, int *len, const char * exec)
{
    int option_cnt = sizeof (command_proc) / sizeof (command_proc[0]);
    int i;

    XDBG_REPLY ("Usage : %s [cmd] [options]\n", exec);
    XDBG_REPLY ("     ex)\n");

    for (i=0; i<option_cnt; i++)
        XDBG_REPLY ("     	%s %s %s\n", exec, command_proc[i].Cmd, command_proc[i].Options);

    XDBG_REPLY (" options :\n");

    for (i=0; i<option_cnt; i++)
    {
        if (command_proc[i].Cmd && command_proc[i].Description)
            XDBG_REPLY ("  %s (%s)\n", command_proc[i].Cmd, command_proc[i].Description);
        else
            XDBG_REPLY ("  Cmd(%p) or Descriptiont(%p).\n", command_proc[i].Cmd, command_proc[i].Description);

        if (command_proc[i].DynamicUsage)
        {
            char dyn[1024];
            int  dynlen = sizeof (dyn);
            command_proc[i].DynamicUsage (MODE_NAME_ONLY, dyn, &dynlen);
            XDBG_REPLY ("     [MODULE:%s]\n", dyn);
        }

        if (command_proc[i].DetailedUsage)
            XDBG_REPLY ("     %s\n", command_proc[i].DetailedUsage);
        else
            XDBG_REPLY ("  DetailedUsage(%p).\n", command_proc[i].DetailedUsage);
    }
}

void
xDbgModuleCommand (void *data, int argc, char **argv, char *reply, int *len)
{
    XDbgModule *pMod = (XDbgModule*)data;
    int nproc = sizeof (command_proc) / sizeof (command_proc[0]);
    int i, pid, new_argc;
    char **new_argv;

    pid = atoi (argv[0]);
    pMod->cwd = strdup (argv[1]);

    new_argc = argc - 2;
    new_argv = (char**)malloc (new_argc * sizeof (char*));
    if (!new_argv)
    {
        XDBG_REPLY ("Error : malloc new_argv\n");
        return;
    }

    for (i = 0; i < new_argc; i++)
        new_argv[i] = argv[i+2];

    if (argc < 4)
    {
        _CommandPrintUsage (reply, len, new_argv[0]);
        free (new_argv);
        return;
    }

    for (i = 0; i < nproc; i++)
    {
        if (!strcmp (new_argv[1], command_proc[i].Cmd) ||
            (new_argv[1][0] == '-' && !strcmp (1 + new_argv[1], command_proc[i].Cmd)))
        {
            command_proc[i].func (pid, new_argc, new_argv, reply, len, pMod);
            free (new_argv);
            return;
        }
    }

    _CommandPrintUsage (reply, len, new_argv[0]);

    free (new_argv);
}

Bool
xDbgModuleCommandInitLogPath (XDbgModule *pMod, char *log_path)
{
    char reply[1024];
    int len = sizeof (reply);

    if (log_path && strlen (log_path) > 0)
    {
        char newname[XDBG_PATH_MAX];
        char filename[XDBG_PATH_MAX];
        struct stat status;
        char *p = NULL, *last = NULL;
        int i;

        snprintf (newname, XDBG_PATH_MAX, "%s", log_path);

        for (i = 0; i < strlen (newname); i++)
        {
            p = newname + i;
            if (*p == '/')
                last = p;
        }

        snprintf (filename, XDBG_PATH_MAX, "%s", last + 1);
        snprintf (last, XDBG_PATH_MAX - (last - newname), "/prev.%s", filename);

        if (!stat (log_path, &status))
        {
            if (rename (log_path, newname))
            {
                XDBG_ERROR (MXDBG, "Failed: rename %s -> %s\n", log_path, newname);
                return FALSE;
            }
        }

        _CommandSetLogFile (0, log_path, reply, &len, pMod);
    }
    else
        _CommandSetLogFile (0, "console", reply, &len, pMod);

    return TRUE;
}

Bool
xDbgModuleCommandInitEvlogRulePath (XDbgModule *pMod, char *evlog_rule_path)
{
    char reply[4096];
    int len = sizeof (reply);
    char *argv[4];
    int argc = 4;

    if (!evlog_rule_path || strlen (evlog_rule_path) <= 0)
        return TRUE;

    argv[0] = "unknown";
    argv[1] = "evlog_rule";
    argv[2] = "file";
    argv[3] = evlog_rule_path;

    _CommandSetEvlogRule (0, argc, argv, reply, &len, pMod);

    xDbgModuleEvlogPrintEvents (pMod, TRUE, "", reply, &len);

    return TRUE;
}

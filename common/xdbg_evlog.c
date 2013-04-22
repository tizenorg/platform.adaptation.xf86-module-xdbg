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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>

#include "xdbg_types.h"
#include "xdbg_evlog.h"
#include "bool_exp_rule_checker.h"

static char *evt_type[] = { "Event", "Request", "Reply", "Flush" };
static char *evt_dir[]  = { "<====", "---->",   "<----", "*****" };

static RULE_CHECKER rc = NULL;

static void
_mergeArgs (char * target, int argc, const char ** argv)
{
    int i;
    int len;

    for (i=0; i<argc; i++)
    {
        len = sprintf (target, "%s", argv[i]);
        target += len;

        if (i != argc - 1)
            *(target++) = ' ';
    }
}

static int
_strcasecmp(const char *str1, const char *str2)
{
    const u_char *us1 = (const u_char *) str1, *us2 = (const u_char *) str2;

    while (tolower(*us1) == tolower(*us2)) {
        if (*us1++ == '\0')
            return 0;
        us2++;
    }

    return (tolower(*us1) - tolower(*us2));
}

char*
xDbgEvlogGetCmd (char *path)
{
    char *p;
    if (!path)
        return NULL;
    p = strrchr (path, '/');
    return (p)?p+1:path;
}

Bool
xDbgEvlogRuleSet (const int argc, const char **argv, char *reply, int *len)
{
    const char * command;

    if (rc == NULL)
        rc = rulechecker_init();

    if (argc == 0)
    {
        rulechecker_print_rule (rc, reply);
        return TRUE;
    }

    command = argv[0];

    if (!_strcasecmp (command, "add"))
    {
        POLICY_TYPE policy_type;
        RC_RESULT_TYPE result;
        const char * policy = argv[1];
        char rule[8192];

        if (argc < 3)
        {
            REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        if (!_strcasecmp (policy, "ALLOW"))
            policy_type = ALLOW;
        else if (!_strcasecmp (policy, "DENY"))
            policy_type = DENY;
        else
        {
            REPLY ("Error : Unknown policy : [%s].\n          Policy should be ALLOW or DENY.\n", policy);
            return FALSE;
        }

        _mergeArgs (rule, argc - 2, &(argv[2]));

        result = rulechecker_add_rule (rc, policy_type, rule);
        if (result == RC_ERR_TOO_MANY_RULES)
        {
            REPLY ("Error : Too many rules were added.\n");
            return FALSE;
        }
        else if (result == RC_ERR_PARSE_ERROR)
        {
            REPLY ("Error : An error occured during parsing the rule [%s]\n", rule);
            return FALSE;
        }

        REPLY ("The rule was successfully added.\n\n");
        rulechecker_print_rule (rc, reply);
        return TRUE;
    }
    else if (!_strcasecmp (command, "remove"))
    {
        const char * remove_idx;
        int i;

        if (argc < 2)
        {
            REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        for (i=0; i<argc - 1; i++)
        {
            remove_idx = argv[i+1];

            if (!_strcasecmp (remove_idx, "all"))
            {
                rulechecker_destroy (rc);
                rc = rulechecker_init();
                REPLY ("Every rules were successfully removed.\n");
            }
            else
            {
                int index = atoi (remove_idx);
                if (isdigit (*remove_idx) && rulechecker_remove_rule (rc, index) == 0)
                    REPLY ("The rule [%d] was successfully removed.\n", index);
                else
                    REPLY ("Rule remove fail : No such rule [%s].\n", remove_idx);
            }
        }
        rulechecker_print_rule (rc, reply);
        return TRUE;
    }
    else if (!_strcasecmp (command, "print"))
    {
        rulechecker_print_rule (rc, reply);
        return TRUE;
    }
    else if (!_strcasecmp (command, "help"))
    {
        REPLY ("%s", rulechecker_print_usage());
        return TRUE;
    }

    REPLY ("%s\nUnknown command : [%s].\n\n", rulechecker_print_usage(), command);

    return TRUE;
}

Bool
xDbgEvlogRuleValidate (EvlogInfo *evinfo)
{
    const char *evlog_name = "";
    char *cmd = "";

    if (rc == NULL)
        rc = rulechecker_init ();

    if (!rc)
    {
        fprintf (stderr, "failed: create rulechecker\n");
        return FALSE;
    }

    cmd = xDbgEvlogGetCmd (evinfo->client.command);

    if (evinfo->type == REQUEST)
        evlog_name = evinfo->req.name;
    else if (evinfo->type == EVENT)
        evlog_name = evinfo->evt.name;

    return rulechecker_validate_rule (rc,
                                      evinfo->type,
                                      evinfo->req.id,
                                      evlog_name,
                                      evinfo->client.pid,
                                      cmd);
}


ExtensionInfo Evlog_extensions[] = {
    {xDbgEvlogCompositeGetBase, 0, 0, 0, NULL},
    {xDbgEvlogDamageGetBase, 0, 0, 0, NULL},
    {xDbgEvlogDri2GetBase, 0, 0, 0, NULL},
    {xDbgEvlogGestureGetBase, 0, 0, 0, NULL},
    {xDbgEvlogRandrGetBase, 0, 0, 0, NULL},
    {xDbgEvlogXextDpmsGetBase, 0, 0, 0, NULL},
    {xDbgEvlogXextShmGetBase, 0, 0, 0, NULL},
    {xDbgEvlogXextSyncGetBase, 0, 0, 0, NULL},
    {xDbgEvlogXextXtestGetBase, 0, 0, 0, NULL},
    {xDbgEvlogXextXtestExt1GetBase, 0, 0, 0, NULL},
    {xDbgEvlogXvGetBase, 0, 0, 0, NULL}
};
ExtensionInfo* Sorted_Evlog_extensions;

static void
_ExtensionsSwap(ExtensionInfo* first, ExtensionInfo* second)
{
    ExtensionInfo temp;

    temp = *first ;
    *first = *second ;
    *second = temp ;
}

static Bool
_SortEvlogExtensions ()
{
    int i,j;
    int swap;

    Sorted_Evlog_extensions = (ExtensionInfo*)malloc(sizeof(Evlog_extensions));
    RETURN_VAL_IF_FAIL (Sorted_Evlog_extensions != NULL, FALSE);

    memcpy(Sorted_Evlog_extensions, Evlog_extensions, sizeof(Evlog_extensions));

    for (i = 0 ; i < sizeof (Evlog_extensions) / sizeof (ExtensionInfo) - 1 ; i++)
    {
        swap = 0;
        for (j = 1 ; j < sizeof (Evlog_extensions) / sizeof (ExtensionInfo) - i ; j++)
        {
            if(Sorted_Evlog_extensions[j-1].evt_base > Sorted_Evlog_extensions[j].evt_base)
            {
                _ExtensionsSwap(&Sorted_Evlog_extensions[j-1], &Sorted_Evlog_extensions[j]);
                swap = 1;
            }
        }
        if (!swap) break;
    }

    return TRUE;
}


static Bool
_EvlogGetExtentionEntry (int *return_extensions_size)
{
    static int init = 0;
    static Bool success = FALSE;
    Display *dpy = NULL;
    int i;

    if (init)
        return success;

    init = 1;

#ifdef XDBG_CLIENT
    dpy = XOpenDisplay (NULL);
    if (!dpy)
    {
        fprintf (stderr, "failed: open display\n");
        exit (-1);
    }
#endif

    for (i = 0 ; i < sizeof (Evlog_extensions) / sizeof (ExtensionInfo); i++)
    {
        Evlog_extensions[i].get_base_func (dpy, Evlog_extensions + i);
    }

    if(!_SortEvlogExtensions ())
        return FALSE;

#ifdef XDBG_CLIENT
    XCloseDisplay (dpy);
#endif

    *return_extensions_size = sizeof(Evlog_extensions);
    success = TRUE;

    return success;
}


void
xDbgEvlogFillLog (EvlogInfo *evinfo, char *reply, int *len)
{
    static CARD32 prev;
    static int Extensions_size = 0;

    RETURN_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_dir) / sizeof (char*)));
    RETURN_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_type) / sizeof (char*)));

    REPLY ("[%10.3f][%5ld] %22s(%2d:%5d) %s %s",
                evinfo->time / 1000.0,
                evinfo->time - prev,
                xDbgEvlogGetCmd (evinfo->client.command),
                evinfo->client.index,
                evinfo->client.pid,
                evt_dir[evinfo->type],
                evt_type[evinfo->type]);

    if (evinfo->type == REQUEST && _EvlogGetExtentionEntry (&Extensions_size))
    {
        REPLY ("(");
        reply = xDbgEvlogReqeust (evinfo, Extensions_size, reply, len);
        REPLY (")");
    }
    else if (evinfo->type == EVENT && _EvlogGetExtentionEntry (&Extensions_size))
    {
        REPLY ("(");
        reply = xDbgEvlogEvent (evinfo, Extensions_size, reply, len);
        REPLY (")");
    }
    else
    {
        const char *evlog_name = "";
        if (evinfo->type == REQUEST)
            evlog_name = evinfo->req.name;
        else if (evinfo->type == EVENT)
            evlog_name = evinfo->evt.name;
        REPLY ("(%s)", evlog_name);
    }

    REPLY ("\n");

    prev = evinfo->time;
}




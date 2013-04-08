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
            XDBG_REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        if (!_strcasecmp (policy, "ALLOW"))
            policy_type = ALLOW;
        else if (!_strcasecmp (policy, "DENY"))
            policy_type = DENY;
        else
        {
            XDBG_REPLY ("Error : Unknown policy : [%s].\n          Policy should be ALLOW or DENY.\n", policy);
            return FALSE;
        }

        _mergeArgs (rule, argc - 2, &(argv[2]));

        result = rulechecker_add_rule (rc, policy_type, rule);
        if (result == RC_ERR_TOO_MANY_RULES)
        {
            XDBG_REPLY ("Error : Too many rules were added.\n");
            return FALSE;
        }
        else if (result == RC_ERR_PARSE_ERROR)
        {
            XDBG_REPLY ("Error : An error occured during parsing the rule [%s]\n", rule);
            return FALSE;
        }

        XDBG_REPLY ("The rule was successfully added.\n\n");
        rulechecker_print_rule (rc, reply);
        return TRUE;
    }
    else if (!_strcasecmp (command, "remove"))
    {
        const char * remove_idx;
        int i;

        if (argc < 2)
        {
            XDBG_REPLY ("Error : Too few arguments.\n");
            return FALSE;
        }

        for (i=0; i<argc - 1; i++)
        {
            remove_idx = argv[i+1];

            if (!_strcasecmp (remove_idx, "all"))
            {
                rulechecker_destroy (rc);
                rc = rulechecker_init();
                XDBG_REPLY ("Every rules were successfully removed.\n");
            }
            else
            {
                int index = atoi (remove_idx);
                if (isdigit (*remove_idx) && rulechecker_remove_rule (rc, index) == 0)
                    XDBG_REPLY ("The rule [%d] was successfully removed.\n", index);
                else
                    XDBG_REPLY ("Rule remove fail : No such rule [%s].\n", remove_idx);
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
        XDBG_REPLY ("%s", rulechecker_print_usage());
        return TRUE;
    }

    XDBG_REPLY ("%s\nUnknown command : [%s].\n\n", rulechecker_print_usage(), command);

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

void
xDbgEvlogFillLog (EvlogInfo *evinfo, char *reply, int *len)
{
    static CARD32 prev;

    RETURN_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_dir) / sizeof (char*)));
    RETURN_IF_FAIL (evinfo->type >= 0 && (sizeof (evt_type) / sizeof (char*)));

    XDBG_REPLY ("[%10.3f][%5ld] %22s(%2d:%5d) %s %s",
                evinfo->time / 1000.0,
                evinfo->time - prev,
                xDbgEvlogGetCmd (evinfo->client.command),
                evinfo->client.index,
                evinfo->client.pid,
                evt_dir[evinfo->type],
                evt_type[evinfo->type]);

    if (evinfo->type == REQUEST)
    {
        XDBG_REPLY ("(");
        reply = xDbgEvlogReqeust (evinfo, reply, len);
        XDBG_REPLY (")");
    }
    else if (evinfo->type == EVENT)
    {
        XDBG_REPLY ("(");
        reply = xDbgEvlogEvent (evinfo, reply, len);
        XDBG_REPLY (")");
    }
    else
    {
        const char *evlog_name = "";
        if (evinfo->type == REQUEST)
            evlog_name = evinfo->req.name;
        else if (evinfo->type == EVENT)
            evlog_name = evinfo->evt.name;
        XDBG_REPLY ("(%s)", evlog_name);
    }

    XDBG_REPLY ("\n");

    prev = evinfo->time;
}

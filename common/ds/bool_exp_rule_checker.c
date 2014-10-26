/**************************************************************************

xserver-xorg-video-exynos

Copyright 2010 - 2011 Samsung Electronics co., Ltd. All Rights Reserved.

Contact: Boram Park <boram1288.park@samsung.com>

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
#include <stdlib.h>

#include "bool_exp_parser.h"
#include "bool_exp_rule_checker.h"

#define MAX_RULE	64

#ifndef REPLY
#define REPLY(fmt, ARG...)  \
    do { \
        if (reply && len && *len > 0) \
        { \
            int s = snprintf (reply, *len, fmt, ##ARG); \
            reply += s; \
            *len -= s; \
        } \
    } while (0)
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct
{
    POLICY_TYPE policy;

    BINARY_TREE tree;
} RULE;

struct _RULE_CHECKER
{
    RULE rules[MAX_RULE];

    int count;
};

typedef struct
{
    int type;
    int reqID;
    const char * name;
    int pid;
    char * cmd;
} VAL_ARGUMENTS;

typedef struct
{
    char **reply;
    int  *len;
} REPLY_BUFFER;

static int print_func (BINARY_TREE tree, BINARY_TREE_NODE node, BINARY_TREE_NODE parent, void * arg)
{
    REPLY_BUFFER *buffer = (REPLY_BUFFER*)arg;
    char *reply = *buffer->reply;
    int *len = buffer->len;
    char * operators[] = { "==", "<", ">", "<=", ">=", "!=" };

    PARSE_DATA data = bintree_get_node_data (node);

    if (data->node_type == ALL)
        REPLY ("ALL");
    else if (data->node_type == AND)
        REPLY (" and ");
    else if (data->node_type == OR)
        REPLY (" or ");
    else // data->node_type == DATA
    {
        if (node == bintree_get_left_child (parent))
            REPLY ("(");

        REPLY ("%s %s ", data->variable_name, operators[data->compare]);

        if (data->value_type == INTEGER)
            REPLY ("%d", data->value.integer);
        else
            REPLY ("%s", data->value.string);

        if (node == bintree_get_right_child (parent))
            REPLY (")");
    }

    *buffer->reply = reply;

    return 0;
}

static int compare_string (COMPARER compare, char * str2, char * str1)
{
    int result = strcasecmp (str2, str1);
    switch (compare)
    {
    case EQUAL:
        return result == 0;
    case LESS:
        return result < 0;
    case GREATER:
        return result > 0;
    case LESS_EQ:
        return result <= 0;
    case GREATER_EQ:
        return result >= 0;
    case NOT_EQ:
        return result != 0;
    }

    return 0;
}

static int compare_int (COMPARER compare, int int2, int int1)
{
    switch (compare)
    {
    case EQUAL:
        return int1 == int2;
    case LESS:
        return int1 < int2;
    case GREATER:
        return int1 > int2;
    case LESS_EQ:
        return int1 <= int2;
    case GREATER_EQ:
        return int1 >= int2;
    case NOT_EQ:
        return int1 != int2;
    }

    return 0;
}

static int validate_func (BINARY_TREE tree, BINARY_TREE_NODE node, BINARY_TREE_NODE parent, void * arg)
{
    VAL_ARGUMENTS * args = (VAL_ARGUMENTS*)arg;
    BINARY_TREE_NODE left, right;

    PARSE_DATA left_data = NULL, right_data = NULL;
    PARSE_DATA data = bintree_get_node_data (node);

    data->result = BEP_UNKNOWN;

    if (data->node_type == AND || data->node_type == OR)
    {
        left = bintree_get_left_child (node);
        right = bintree_get_right_child (node);
        if (left == NULL || right == NULL)
        {
            printf ("Node error\n");
            return -1;
        }

        left_data = bintree_get_node_data (left);
        right_data = bintree_get_node_data (right);
    }

    if (data->node_type == ALL)
    {
        data->result = BEP_TRUE;
    }
    else if (data->node_type == DATA)
    {
        char major[64];
        char * minor = NULL;

        if (args->name)
            minor = index (args->name, ':');
        if (minor)
        {
            int min = MIN (sizeof(major)-1, minor-args->name);
            strncpy (major, args->name, min);
            major[min] = '\0';
            minor++;
        }
        if (!strcasecmp (data->variable_name, "TYPE"))
        {
            char * type_string;
            if (args->type == 0) // EVENT
                type_string = "EVENT";
            else if (args->type == 1)
                type_string = "REQUEST";
            else if (args->type == 2)
                type_string = "REPLY";
            else if (args->type == 3)
                type_string = "FLUSH";
            else if (args->type == 4)
                type_string = "ERROR";
            else
            {
                fprintf (stderr, "Invalid type %d\n", args->type);
                return -1;
            }

            if (compare_string (data->compare, data->value.string, type_string))
                data->result = BEP_TRUE;
            else
                data->result = BEP_FALSE;
        }
        else if (!strcasecmp (data->variable_name, "MAJOR"))
        {
            if (minor && compare_string (data->compare, data->value.string, major))
                data->result = BEP_TRUE;
            else
                data->result = BEP_FALSE;
        }
        else if (!strcasecmp (data->variable_name, "MINOR"))
        {
            if (minor && compare_string (data->compare, data->value.string, minor))
                data->result = BEP_TRUE;
            else
                data->result = BEP_FALSE;
        }
        else if (!strcasecmp (data->variable_name, "PID"))
        {
            if (compare_int (data->compare, data->value.integer, args->pid))
                data->result = BEP_TRUE;
            else
                data->result = BEP_FALSE;
        }
        else if (!strcasecmp (data->variable_name, "CMD") || !strcasecmp (data->variable_name, "COMMAND"))
        {
            if (args->cmd && compare_string (data->compare, data->value.string, args->cmd))
                data->result = BEP_TRUE;
            else
                data->result = BEP_FALSE;
        }
    }
    else if (data->node_type == AND)
    {
        if (left_data->result == BEP_TRUE && right_data->result == BEP_TRUE)
            data->result = BEP_TRUE;
        else
            data->result = BEP_FALSE;
    }
    else if (data->node_type == OR)
    {
        if (left_data->result == BEP_TRUE || right_data->result == BEP_TRUE)
            data->result = BEP_TRUE;
        else
            data->result = BEP_FALSE;
    }
    else
        return -1;

    return 0;
}


RULE_CHECKER rulechecker_init()
{
    RULE_CHECKER rc = calloc (sizeof (struct _RULE_CHECKER), 1);
    if (rc == NULL)
        return NULL;

    rc->count = 0;

    return rc;
}

void rulechecker_destroy (RULE_CHECKER rc)
{
    int i;
    for (i=rc->count - 1; i>=0; i--)
        rulechecker_remove_rule (rc, i);

    free (rc);
}

RC_RESULT_TYPE rulechecker_add_rule (RULE_CHECKER rc, POLICY_TYPE policy, const char * rule_string)
{
    if (rc->count == MAX_RULE)
        return RC_ERR_TOO_MANY_RULES;

    rc->rules[rc->count].tree = bool_exp_parse (rule_string);
    if (rc->rules[rc->count].tree == NULL)
        return RC_ERR_PARSE_ERROR;
    rc->rules[rc->count].policy = policy;

    rc->count++;

    return RC_OK;
}

RC_RESULT_TYPE rulechecker_remove_rule (RULE_CHECKER rc, int index)
{
    if (index < 0 || index >= rc->count)
        return RC_ERR_NO_RULE;

    bintree_destroy_tree (rc->rules[index].tree);

    rc->count--;
    if (index != rc->count)
        memmove (&rc->rules[index], &rc->rules[index + 1], sizeof (RULE) * (rc->count - index));

    return RC_OK;
}

void rulechecker_print_rule (RULE_CHECKER rc, char *reply, int *len)
{
    REPLY_BUFFER buffer = {&reply, len};
    int i;

    REPLY (" ---------------- Evlog Rules ----------------\n");

    for (i=0; i<rc->count; i++)
    {
        REPLY (" [Rule %d] [%s] \"", i, rc->rules[i].policy == ALLOW ? "ALLOW" : "DENY");

        bintree_inorder_traverse (rc->rules[i].tree, print_func, (void*)&buffer);
        REPLY ("\"\n");
    }
}

const char * rulechecker_print_usage()
{
    return
        "######################################################################\n"
        "###     RuleChecker 1.0 for XDBG EvLog filtering.                  ###\n"
        "###                 Designed and developed by                      ###\n"
        "###                 YoungHoon Jung <yhoon.jung@samsung.com>        ###\n"
        "######################################################################\n"
        "\n"
        "-----------------------------------------------------------------\n"
        "How to read evlog messages :\n"
        "      Client(pid: [PID]|       [CMD])   <======     [TYPE]( [MAJOR]:  [MINOR])   =====     XServer\n"
        "\n"
        "   ie)\n"
        "        Client(pid:00345|        xdbg)   <======    Event (       X11:SendEvent)   =====     XServer\n"
        "             ==> type = event && pid = 345 && cmd = xdbg && major = X11 && minor = SendEvent\n"
        "        Client(pid:00111|        xeyes)     =====   Request(       SHM:ShmAttach)   ======>   XServer\n"
        "             ==> type = request && pid = 111 && cmd = xeyes && major = SHM && minor = ShmAttach\n"
        "\n"
        "-----------------------------------------------------------------\n"
        "Usage : xdbg evlog_rule add [POLICY] [RULE]\n"
        "        xdbg evlog_rule remove [INDEX]\n"
        "        xdbg evlog_rule file [RULE_FILE]\n"
        "        xdbg evlog_rule help / print\n"
        "      [POLICY] : allow / deny \n"
        "      [RULE] : C Language-style boolean expression syntax. [VARIABLE] [COMPAROTOR] [VALUE]\n"
        "      [VARIABLE] : type / major / minor / command / cmd / pid\n"
        "      [COMPARATOR] : & / && / and / | / || / or / = / == / != / > / >= / < / <=\n"
        "      [VALUE] : string / number  \n"
        "\n"
        "   ie)\n"
        "        xdbg evlog_rule add allow \"(type=request) && (major == X11 and (minor = SendEvent or minor = ReceiveEvent))\"\n"
        "        xdbg evlog_rule add deny cmd!=ls\n"
        "        xdbg evlog_rule remove 1\n"
        "\n";
}

int rulechecker_validate_rule (RULE_CHECKER rc, int type, int reqID, const char * name, int pid, char * cmd)
{
    VAL_ARGUMENTS args = { type, reqID, name, pid, cmd };
    BINARY_TREE_NODE node;
    PARSE_DATA data;

    // set default value here
    POLICY_TYPE default_policy = DENY;
    int i;
    for (i=rc->count - 1; i >= 0; i--)
    {
        bintree_postorder_traverse (rc->rules[i].tree, validate_func, &args);
        node = bintree_get_head (rc->rules[i].tree);
        data = bintree_get_node_data (node);

        if (data->result == BEP_TRUE)
        {
            return rc->rules[i].policy == ALLOW;
        }
    }

    return default_policy == ALLOW;
}

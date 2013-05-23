# Copyright 2013 Samsung Electronics co., Ltd. All Rights Reserved.
#
# Contact: Boram Park <boram1288.park@samsung.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

COMMON_SOURCE =  \
    xdbg_dbus_client.c \
    xdbg_evlog.c \
    xdbg_evlog_request.c \
    xdbg_evlog_core.c \
    xdbg_evlog_composite.c \
    xdbg_evlog_dri2.c \
    xdbg_evlog_damage.c \
    xdbg_evlog_gesture.c \
    xdbg_evlog_xext.c \
    xdbg_evlog_randr.c \
    xdbg_evlog_xinput.c \
    xdbg_evlog_xv.c \
    xdbg_evlog_event.c \
    xdbg_evlog_reply.c \
    ds/bintree.c \
    ds/bool_exp_parser.c \
    ds/bool_exp_rule_checker.c \
    ds/bool_exp_tokenizer.c

COMMON_CFLAGS = \
    ${CFLAGS} \
    @XDBG_CFLAGS@ \
    -I$(top_srcdir)/lib \
    -I$(top_srcdir)/common \
    -I$(top_srcdir)/common/ds

COMMON_CLIENT_CFLAGS = \
    ${CFLAGS} \
    @XDBG_CLIENT_CFLAGS@ \
    -I$(top_srcdir)/common \
    -I$(top_srcdir)/common/ds


noinst_LTLIBRARIES = libcommon-client.la  \
                     libcommon-server.la

libcommon_client_la_LIBADD = ${LDFLAGS} @XDBG_LIBS@
libcommon_client_la_LDFLAGS = ${LDFLAGS} @XDBG_LIBS@
libcommon_client_la_CFLAGS = ${COMMON_CLIENT_CFLAGS}
libcommon_client_la_SOURCES = $(COMMON_SOURCE)

libcommon_server_la_LIBADD = ${LDFLAGS} @XDBG_LIBS@
libcommon_server_la_LDFLAGS = ${LDFLAGS} @XDBG_LIBS@
libcommon_server_la_CFLAGS = ${COMMON_CFLAGS}
libcommon_server_la_SOURCES = $(COMMON_SOURCE) \
                                xdbg_dbus_server.c

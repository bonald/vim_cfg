/*
 * Copyright (c) 2002, 2003, 2004 BalaBit IT Ltd, Budapest, Hungary
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Note that this permission is granted for only version 2 of the GPL.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef SYSLOG_NG_H_INCLUDED
#define SYSLOG_NG_H_INCLUDED

#include <config.h>

#if ENABLE_DMALLOC
#define USE_DMALLOC
#endif

#if ENABLE_DEBUG
#define YYDEBUG 1
#endif

#include <glib.h>

#if ENABLE_DEBUG
#define PATH_SYSLOG_NG_CONF     "syslog-ng.conf"
#define PATH_PIDFILE            "syslog-ng.pid"
#else
#define PATH_SYSLOG_NG_CONF     PATH_SYSCONFDIR "/syslog-ng.conf"
#define PATH_PIDFILE            "/var/run/syslog-ng.pid"
#endif
#define PATH_LOCALSTATEDIR "`patheval $localstatedir`"
#define PATH_PERSIST_CONFIG     PATH_LOCALSTATEDIR "/syslog-ng.persist"

#define LOG_PRIORITY_LISTEN 100
#define LOG_PRIORITY_READER 0
#define LOG_PRIORITY_MERGER -50 /* Added by zhoucheng for log merge*/
#define LOG_PRIORITY_WRITER -100
#define LOG_PRIORITY_CONNECT -150

#ifdef DEBUG_MEM_LEAK
extern long new_marker_count;
extern long new_msg_count;
extern long new_internal_count;
extern long new_template_count;

extern long free_marker_count;
extern long free_msg_count;
extern long free_internal_count;
extern long free_template_count;

extern long ref_template_count;
extern long unref_template_count;

extern long new_file_dd_count;
extern long free_file_dd_count;

extern void write_debug_count();
#endif

#endif

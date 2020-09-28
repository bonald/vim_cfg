/****************************************************************************
* $Id$
*  Merge log
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : zhoucheng
* Date          : 2007-12-04 14:20
* Reason        : First Create.
****************************************************************************/

#ifndef _LOGMERGER_H_
#define _LOGMERGER_H_

#include "syslog-ng.h"
#include "logpipe.h"
#include <time.h>

#define DEBUG_MSG_PRI 0x07      /* Debug priority */
#define SEVERITY_MASK 0x07       /* Severity Mask */

struct LogCenter;

/* log merger options */
typedef struct _LogMergerOptions
{
    gchar *stats_name; /* reserved for extending */
    guint32 flags; /* reserved for extending */
    gint fifo_size; /* log merge buffer size */
    gint flush_timeout; /* microseconds */
    gboolean enable;
    gboolean time_from; /* Added by zhoucheng for bug 3974 */
} LogMergerOptions;

/* log merger */
typedef struct _LogMerger
{
    LogPipe super; /* super type */
    GSource *source; /* related loop source */
    GQueue *queue; /* merge buffer */
    guint32 flags; /* reserved for extending */
    LogMergerOptions *options; /* related merge options */
    LogCenter *center; /* log center */
} LogMerger;

/* log merger item */
typedef struct _LogMergeItem
{
    LogMessage *pmsg; /* related message */
    time_t start_time; /* the first message's coming time */
    guint count; /* message count having the same content */
    guint path_flags; /* message flags */
} LogMergeItem;

/* LogMergerOptions functions */
extern LogMergerOptions *log_merger_options_new();
extern void log_merger_options_destroy(LogMergerOptions *options);
extern void log_merger_options_defaults(LogMergerOptions *options);
extern void log_merger_options_init(LogMergerOptions *options, 
                                    GlobalConfig *cfg, 
                                    guint32 flags, 
                                    const gchar *stats_name);

/* LogMerger functions */
extern LogPipe *log_merger_new(guint32 flags, LogCenter *center, LogMergerOptions *options);
extern void log_merger_free(LogPipe *s);
extern gboolean log_merger_init(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist);
extern gboolean log_merger_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist);

#endif /* !_LOGMERGER_H_ */

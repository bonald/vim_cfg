/****************************************************************************
* $Id$
*  Implement a log merge GSource for glibc event framework
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : zhoucheng
* Date          : 2007-12-04 14:16
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
*
* Header Files
* 
****************************************************************************/
#include <config.h>

#ifdef HAVE_MERGE
#include <syslog-ng.h>
#include <logmsg.h>
#include "center.h"
#include "sgroup.h"
#include "dgroup.h"
#include "filter.h"
#include <logmerger.h>
#include <stdio.h>
#include <stdlib.h>


/****************************************************************************
*
* Defines and Macros
* 
****************************************************************************/
#define MERGE_CACHE_SIZE 1024
#define MERGE_TIMEOUT    10

/* GSource type used to watch LogMerger */
typedef struct _LogMergerWatch
{
    GSource super;
    GPollFD pollfd;
    LogMerger *merger; 
    GTimer* flush_timer; 
} LogMergerWatch;


/****************************************************************************
*
* Global and Declaration
* 
****************************************************************************/
static void log_merger_transfer(LogMerger * merger);
static gboolean log_merger_fd_prepare(GSource *source, gint *timeout);
static gboolean log_merger_fd_check(GSource *source);
static gboolean log_merger_fd_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static void log_merger_fd_finalize(GSource *source);

extern void log_center_ack(LogMessage *msg, gpointer user_data);
extern void log_msg_free_content(LogMessage *self);

#ifdef DEBUG_MERGE
extern void debug_print(const char * str);
#endif

/* hook functions which will be called by glibc main loop framework */
GSourceFuncs log_merger_source_funcs =
{
    log_merger_fd_prepare,
    log_merger_fd_check,
    log_merger_fd_dispatch,
    log_merger_fd_finalize
};

/****************************************************************************
*
* Function
* 
****************************************************************************/

/*******************************************************************************
 * Name:    log_merger_msg_compare
 * Purpose: compare two messages
 * Input: 
 *   item1: first msg
 *   item2: second msg 
 * Output: 
 * Return: 
 *   equal: 1
 *   no equal : 0
 * Note: 
 ******************************************************************************/
static gint 
log_merger_msg_compare(gconstpointer item1, gconstpointer item2)
{
    LogMessage *msg1 = (LogMessage*)item1;
    LogMessage *msg2 = (LogMessage*)item2;
    
    if (!item1 || !item2)
    {
        return 0;
    }

    return (msg1->pri == msg2->pri) 
        && g_string_equal(msg1->host, msg2->host)
        && g_string_equal(msg1->host_from, msg2->host_from)
        && g_string_equal(msg1->program, msg2->program) 
        && g_string_equal(msg1->msg, msg2->msg);
}

/*******************************************************************************
 * Name:    log_merger_copy_timeval
 * Purpose: copy timeval
 * Input: 
 *   dest: destination
 *   src: source
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
static inline guint 
log_merger_copy_timeval(struct timeval *dest, struct timeval *src)
{
    g_assert(dest && src);
    
    dest->tv_sec = src->tv_sec;
    dest->tv_usec = src->tv_usec;

    return 0;
}

/*******************************************************************************
 * Name:    log_merger_fd_prepare
 * Purpose: GSource prepare hook
 * Input: 
 *   source: source driver
 * Output: 
 *   timeout: specify how much time this source will be polled again
 * Return: 
 *   success: TRUE
 *   failed : FALSE
 * Note: 
 ******************************************************************************/
static gboolean
log_merger_fd_prepare(GSource *source, gint *timeout)
{
    LogMergerWatch *self = (LogMergerWatch *) source;
    gulong micro = 0;
    gint elapse = 0;

    g_assert(source);

    elapse = 1000 * g_timer_elapsed(self->flush_timer, &micro);
    if (elapse> self->merger->options->flush_timeout)
    {
        g_timer_stop(self->flush_timer);
        g_timer_start(self->flush_timer);
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
 * Name:    log_merger_fd_check
 * Purpose: GSource check hook
 * Input: 
 *   source: source driver
 * Output: 
 * Return: 
 *   success: TRUE
 *   failed : FAIL
 * Note: always for that prepare will check this source and skip polling
 ******************************************************************************/
static gboolean
log_merger_fd_check(GSource *source)
{
    return FALSE;
}
  
/*******************************************************************************
 * Name:    log_merger_fd_dispatch
 * Purpose: GSource dispatch hook
 * Input: 
 *   source: source driver
 *   callback: we need not use this param 
 *   user_data: we need not use this param 
 * Output: 
 * Return: 
 *   success: TRUE
 *   failed : FALSE 
 * Note: translate messages from merge buffer to destinations
 ******************************************************************************/
static gboolean
log_merger_fd_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
    LogMergerWatch *self  = NULL;
    
    g_assert(source);
    self = (LogMergerWatch*)self;
    log_merger_transfer(self->merger);
    return TRUE;
}

/*******************************************************************************
 * Name:    log_merger_fd_finalize
 * Purpose: GSource dispatch hook
 * Input: 
 *   source: source driver
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
log_merger_fd_finalize(GSource *source)
{
    g_assert(source);
    
    LogMergerWatch *self = (LogMergerWatch *) source;
    g_timer_stop(self->flush_timer);
    g_timer_destroy(self->flush_timer);
    log_pipe_unref(&self->merger->super);
}

/*******************************************************************************
 * Name:    log_merger_age
 * Purpose: aging the eldest item in merge queue
 * Input: 
 *   self: the LogMerger itself 
 *   aged_item: the eldest item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
log_merger_age(LogMerger *self, LogMergeItem *aged_item)
{
    struct  tm * tmptm = NULL;
    gchar append[64] = {0};
    gchar tmptime[32] = {0};

    if (NULL == aged_item)
    {
        return;
    }

    /* send out this merged item */
    if (aged_item->count > 0)
    {
        /* if count equal 0 there is no need to append times-postfix */
        if (aged_item->count > 1)
        {
            tmptm = localtime(&aged_item->start_time);
            strftime(tmptime, 32, "%X", tmptm);

            if (self->options->time_from)
            {
                sprintf(append, "[%d times from %s]", aged_item->count, tmptime);
            }
            else
            {
                sprintf(append, "[%d times]", aged_item->count);
            }
            g_string_sprintf(aged_item->pmsg->suffix, "%s", append);
        }
        self->center->fetcher(&self->center->super, aged_item->pmsg, aged_item->path_flags);
    }
    
    /* unref this merged item */
    log_msg_unref(aged_item->pmsg);
}
        
/*******************************************************************************
 * Name:    log_merger_dumpall
 * Purpose: dump all the item in merge queue
 * Input: 
 *   self: the LogMerger itself
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
log_merger_dumpall(LogMerger *self)
{
    LogMergeItem *item = NULL;
    g_assert(self);

    while (!g_queue_is_empty(self->queue))
    {
        item = g_queue_pop_head(self->queue);
        log_merger_age(self, item);
        g_free(item);
        item = NULL;
    }
}
 
/*******************************************************************************
 * Name:    log_merger_append
 * Purpose: append a new merge item to the merger buffer
 * Input: 
 *   s: LogMergerItem
 *   lm: LogMessage
 *   path_flags: path flags 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
log_merger_append(LogPipe *s, LogMessage *lm, gint path_flags)
{
    LogMergeItem *new_item = NULL;
    LogMerger *self = (LogMerger *) s;
    guint length = 0;

    g_assert(self);

    /* increase this new LogMessage's reference counter,
     * and log_msg_unref will be called in log_merger_age */
    log_msg_ref(lm);

    /* first coming message should be sent out immediately */
    self->center->fetcher(&self->center->super, lm, path_flags);
    length = g_queue_get_length(self->queue);

    /* if buffer is full, aging the eldest one */
    if (length >= self->options->fifo_size)
    {
        LogMergeItem *aged_item =NULL;
        
        aged_item = g_queue_pop_head(self->queue);
        log_merger_age(self, aged_item);
        g_free(aged_item);
    }

    /* create a new LogMergeItem, initialize and append it to buffer tail */
    new_item = g_new(LogMergeItem, 1);
    if (!new_item)
    {
        return;
    }
    new_item->pmsg = lm;
    new_item->count = 0;
    new_item->path_flags = path_flags;
    new_item->start_time = lm->recvd.time.tv_sec;
    g_queue_push_tail(self->queue, new_item);
}

/*******************************************************************************
 * Name:    log_merger_queue
 * Purpose: callback for logpipe queue
 * Input: 
 *   s: the LogMerger pipe itself
 *   lm: incoming message
 *   path_flags: message path flag 
 * Output: 
 * Return: N/A
 * Note: message which came to LogCenter will be redirect to merger queue 
 ******************************************************************************/
static void
log_merger_queue(LogPipe *s, LogMessage *lm, gint path_flags)
{
    LogMerger *self = (LogMerger *) s;
    LogMergeItem *existed_item = NULL;
    gint length = 0;
    gint i=0;
    LogMergeItem *m = NULL;

    g_assert(s && lm);

    /* debug messges should not be merged */
    if (DEBUG_MSG_PRI == (lm->pri & SEVERITY_MASK))
    {
        self->center->fetcher(&self->center->super, lm, path_flags);
        return;
    }

    length = g_queue_get_length(self->queue);

    /* iterate the queue to find the same message coming before */
    for (i=0; i<length; i++)
    {
        gint cmp = 0;
        m = g_queue_peek_nth(self->queue, i);
        if (!m)
        {
            break;
        }
        cmp = log_merger_msg_compare(m->pmsg, lm);
        if (cmp)
        {
            existed_item = m;
            break;
        }
    }
    /* same message is in the merge queue */
    if (existed_item)
    {
        log_merger_copy_timeval(&existed_item->pmsg->stamp.time, &lm->stamp.time);
        log_merger_copy_timeval(&existed_item->pmsg->recvd.time, &lm->recvd.time);
        existed_item->count++;

        /* unref this message, and increase the source's window_size
         * it is very important here! */
        log_msg_ack(lm);
    }
    /* new message, put it's reference in queue, then send it out immediately */
    else
    {
        log_merger_append(s, lm, path_flags);
    }
}


/*******************************************************************************
 * Name:    log_merger_init
 * Purpose: initialzie log merger
 * Input: 
 *   s: the LogMerger itself
 *   cfg: the global configuration
 *   persist: persistent config
 * Output: 
 * Return: 
 *   success: TRUE 
 *   failed : FALSE 
 * Note: 
 ******************************************************************************/
gboolean
log_merger_init(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
    LogMerger *self = (LogMerger *) s;

    g_assert(s);
    /* 
      * the source added below references this logmerger, it will be unref'd
      *  when the merger is destroyed 
      */
    self->source = NULL; /*log_merger_watch_new(self); */
    alarm(self->options->flush_timeout / 1000);
    /* g_source_attach(self->source, NULL); */
    return TRUE;
}

/*******************************************************************************
 * Name:    log_merger_deinit
 * Purpose: da
 * Input: 
 *   s: the LogMerger itself
 *   cfg: global configuration 
 *   persist: persistent configuation 
 * Output: 
 * Return: 
 *   success: TRUE
 *   failed : FALSE
 * Note: 
 ******************************************************************************/
gboolean
log_merger_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
    LogMerger *self = (LogMerger *) s;

    if (!self)
    {
        return FALSE;
    }
  
    if (self->source)
    {
        g_source_destroy(self->source);
        g_source_unref(self->source);
        self->source = NULL;
    }
    return TRUE;
}

/*******************************************************************************
 * Name:    log_merger_free
 * Purpose: free LogMerger
 * Input: 
 *   s: the LogMerger itself
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
log_merger_free(LogPipe *s)
{
    LogMerger *self = (LogMerger *) s;

    g_assert(s);

    while (!g_queue_is_empty(self->queue))
    {
        LogMergeItem *item = NULL;

        item = g_queue_pop_head(self->queue);  
#if 0 /* Modified by zhoucheng 2008-01-22 16:23, bug 4165 */
        log_msg_unref(item->pmsg);
#else
        log_merger_age(self, item);
#endif
        g_free(item);
    }
    if (self->options)
    {
        log_merger_options_destroy(self->options); 
        self->options = NULL;
    }
    g_queue_free(self->queue);
    g_free(self);
}

/*******************************************************************************
 * Name:    log_merger_new
 * Purpose: create a new log merger
 * Input: 
 *   flags: flags
 *   center: the related LogCenter 
 *   options: the related LogMergerOptions
 * Output: 
 * Return: 
 *   success: the new created LogMerger
 *   failed : NULL
 * Note: 
 ******************************************************************************/
LogPipe *
log_merger_new(guint32 flags, LogCenter *center, LogMergerOptions *options)
{
    if (!options)
    {
        return NULL;
    }
    LogMerger *self = g_new0(LogMerger, 1);

    log_pipe_init_instance(&self->super);
    self->super.init = log_merger_init;
    self->super.deinit = log_merger_deinit;
    self->super.queue = log_merger_queue;
    self->super.free_fn = log_merger_free;
    self->options = options;  
    self->queue = g_queue_new();
    self->flags = flags;
    self->center = center;
    
    return &self->super;
}

/*******************************************************************************
 * Name:    log_merger_options_defaults
 * Purpose: set merger options to default
 * Input: 
 *   options: the logmerger options itself
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
log_merger_options_defaults(LogMergerOptions *options)
{
    options->stats_name = NULL;
    options->flags = 0;
    options->fifo_size= 10;
    options->flush_timeout = 5000;
    options->enable = TRUE;
    options->time_from = TRUE; /* Added by zhoucheng for bug 3974 */
}

/*******************************************************************************
 * Name:    log_merger_options_init
 * Purpose: initialize merger options using global config
 * Input: 
 *   options: the merger options itself
 *   cfg: the global configuation
 *   flags: flags
 *   stats_name: we will not use this param
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
log_merger_options_init(LogMergerOptions *options, GlobalConfig *cfg, guint32 flags, const gchar *stats_name)
{
    if (!options || !cfg)
    {
        return;
    }
    options->flags = flags;
    options->stats_name = stats_name ? g_strdup(stats_name) : NULL;
    options->fifo_size = cfg->merge_fifo_size;
    options->flush_timeout = cfg->merge_timeout;
    options->enable = cfg->merge_enable;
    options->time_from = cfg->merge_timefrom; /* Added by zhoucheng for bug 3974 */
}

/*******************************************************************************
 * Name:    log_merger_options_free
 * Purpose: free LogMergerOptions
 * Input: 
 *   options: the options itself
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
log_merger_options_free(LogMergerOptions *options)
{
    g_free(options);
}

/*******************************************************************************
 * Name:    log_merger_options_destroy
 * Purpose: destrory log merger options
 * Input: 
 *   options: the global merger options
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
log_merger_options_destroy(LogMergerOptions *options)
{
    if (!options)
    {
        return;
    }
    if (options->stats_name)
    {
        g_free(options->stats_name);
    }
    log_merger_options_free(options);
}

/*******************************************************************************
 * Name:    log_merger_options_new
 * Purpose: create a new log merger option
 * Input: N/A
 * Output: 
 * Return: 
 *   success: the new created LogMergerOptions
 *   failed : NULL
 * Note: 
 ******************************************************************************/
LogMergerOptions *
log_merger_options_new()
{
    return g_new0(LogMergerOptions, 1);
}


/*******************************************************************************
 * Name:    log_merger_transfer
 * Purpose: transfer logs from merge-fifo to dest-fifo
 * Input: 
 *   merger: the related merger 
 *   checktime: check time or not 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static
void log_merger_transfer(LogMerger * merger)
{
    LogMergeItem *item = NULL;
    gchar append[64] = {0};
    gchar tmptime[32] = {0};
    struct  tm * tmptm = NULL;
    time_t right_now = 0;
        
    g_assert(merger);
    
    /* get current time */
    time(&right_now);
    
        gint length = 0;
        gint i = 0;
        length = g_queue_get_length(merger->queue);

        /* iterate the queue to find the same message coming before */
        for (i=0; i<length; i++)
        {
            item = g_queue_peek_nth(merger->queue, i);
            if (!item)
            {
                break;
            }
            tmptm = localtime(&item->start_time);

            /* append time-suffix */
            strftime(tmptime, 32, "%X", tmptm);

            if (item->count > 0)
            {
                if (item->count > 1)
                {
                    if (merger->options->time_from)
                    {
                        sprintf(append, "[%d times from %s]", item->count, tmptime);
                    }
                    else
                    {
                        sprintf(append, "[%d times]", item->count);
                    }
                    g_string_sprintf(item->pmsg->suffix, "%s", append);

                    /* send it out to destinations */
                    merger->center->fetcher(&merger->center->super, item->pmsg, item->path_flags);
                }
                else
                {
                    /*for bug 4828*/                                  
                    g_string_sprintf(item->pmsg->suffix, "%s", " ");  
                    /* send it out to destinations */
                    merger->center->fetcher(&merger->center->super, item->pmsg, item->path_flags);
                }
            }
            /* set start time to current time */
            item->start_time = right_now;
            item->count = 0;
        }
}

/*******************************************************************************
 * Name:    log_merge_sync_fifo
 * Purpose: synchronize all item in merge queue
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: synchronize all item in merge queue when SIGALRM timeout
 ******************************************************************************/
void
log_merge_sync_fifo()
{
    /* Modified by zhoucheng 2007-12-25 17:30, bug 3885*/
    log_merger_transfer(configuration->merger);
    alarm(configuration->merger->options->flush_timeout / 1000);
}

/*******************************************************************************
 * Name:    log_merge_sync_fifo
 * Purpose: dump all the item in merge queue
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: transfer all the messages in merger queue to destinations, this function
 *       will be called on two conditions:
 *       1) syslog-ng received SIGHUP signal to reload configuration file
 *       2) syslog-ng received SIGUSR2 signal to synchronize merge queue
 ******************************************************************************/
void
log_merge_sync_fifo_all()
{
    /* Modified by zhoucheng 2007-12-25 17:30, bug 3885*/
    log_merger_dumpall(configuration->merger);
    alarm(configuration->merger->options->flush_timeout / 1000);
}

#endif


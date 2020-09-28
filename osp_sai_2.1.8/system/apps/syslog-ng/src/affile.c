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
#include "affile.h"
#include "driver.h"
#include "messages.h"
#include "macros.h"
#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DEBUG_MEM_LEAK
void write_debug_file(const char *str)
{
    FILE  *fp = NULL;
    fp = fopen("template.txt", "ab+");
    if (fp != NULL)
    {   
        fputs(str, fp);
        fclose(fp);
    } 
}
#endif

#if !HAVE_O_LARGEFILE
#define O_LARGEFILE 0100000
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0100000
#endif

static gboolean
affile_open_file(gchar *name, int flags,
	     int uid, int gid, int mode,
	     int dir_uid, int dir_gid, int dir_mode,
	     int create_dirs, int *fd)
{
  if (strstr(name, "../") || strstr(name, "/..")) 
    {
      msg_error("Spurious path, logfile not created",
                evt_tag_str("path", name),
                NULL);
      return FALSE;
    }

  *fd = open(name, flags, mode);
  if (create_dirs && *fd == -1 && errno == ENOENT) 
    {
      /* directory does not exist */
      char *p = name + 1;
      
      p = strchr(p, '/');
      while (p) 
	{
	  struct stat st;
	  *p = 0;
	  if (stat(name, &st) == 0) 
	    {
	      if (!S_ISDIR(st.st_mode))
		return FALSE;
	    }
	  else if (errno == ENOENT) 
	    {
	      if (mkdir(name, dir_mode) == -1)
		return 0;
	      if (dir_uid != -1 || dir_gid != -1)
		chown(name, dir_uid, dir_gid);
	      if (dir_mode != -1)
		chmod(name, dir_mode);
	    }
	  *p = '/';
	  p = strchr(p + 1, '/');
	}
      *fd = open(name, flags, mode);
    }
  if (*fd != -1)
    {
      g_fd_set_cloexec(*fd, TRUE);
      if (uid != -1)
        fchown(*fd, uid, -1);
      if (gid != -1)
        fchown(*fd, -1, gid);
      if (mode != -1)
        fchmod(*fd, mode);
    }
  return *fd != -1;
}

static gboolean
affile_sd_init(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileSourceDriver *self = (AFFileSourceDriver *) s;
  int fd, flags;

  if (self->flags & AFFILE_PIPE)
    flags = O_RDWR | O_NOCTTY | O_NONBLOCK | O_LARGEFILE;
  else
    flags = O_RDONLY | O_NOCTTY | O_NONBLOCK | O_LARGEFILE;

  log_reader_options_init(&self->reader_options, cfg);

  if (affile_open_file(self->filename->str, flags, -1, -1, -1, 0, 0, 0, 0, &fd))
    {
      self->reader = log_reader_new(fd_read_new(fd, 0), LR_LOCAL | LR_NOMREAD, s, &self->reader_options);
      log_pipe_append(self->reader, s);
      if (!log_pipe_init(self->reader, NULL, NULL))
        {
          msg_error("Error initializing log_reader, closing fd",
                    evt_tag_int("fd", fd),
                    NULL);
          log_pipe_unref(self->reader);
          self->reader = NULL;
          close(fd);
          return FALSE;
        }
    }
  else
    {
      msg_error("Error opening file for reading",
                evt_tag_str("filename", self->filename->str),
                evt_tag_errno(EVT_TAG_OSERROR, errno),
                NULL);
      return self->super.optional;
    }
  return TRUE;
}

static gboolean
affile_sd_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileSourceDriver *self = (AFFileSourceDriver *) s;

  if (self->reader)
    {
      log_pipe_deinit(self->reader, NULL, NULL);
      log_pipe_unref(self->reader);
      self->reader = NULL;
    }
  /*modified by weij for bug 19965, 2012-07-30*/
  if (self->reader_options.prefix)
  {
    g_free(self->reader_options.prefix);
    self->reader_options.prefix = NULL;
  }
  return TRUE;
}

static void
affile_sd_free(LogPipe *s)
{
  AFFileSourceDriver *self = (AFFileSourceDriver *) s;

  g_string_free(self->filename, TRUE);
  g_assert(!self->reader);

  log_drv_free_instance(&self->super);
  g_free(s);
}

LogDriver *
affile_sd_new(gchar *filename, guint32 flags)
{
  AFFileSourceDriver *self = g_new0(AFFileSourceDriver, 1);

  log_drv_init_instance(&self->super);
  self->filename = g_string_new(filename);
  self->flags = flags;
  self->super.super.init = affile_sd_init;
  self->super.super.deinit = affile_sd_deinit;
  self->super.super.free_fn = affile_sd_free;
  log_reader_options_defaults(&self->reader_options);
  return &self->super;
}


typedef struct _AFFileDestWriter
{
  LogPipe super;
  AFFileDestDriver *owner;
  GString *filename;
  LogPipe *writer;
  time_t last_msg_stamp;
  time_t last_open_stamp;
  time_t time_reopen;
} AFFileDestWriter;
#ifdef _CENTEC_
static gboolean
affile_dw_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileDestWriter *self = (AFFileDestWriter *) s;

  if (self->writer)
    {
/*modified by weij for bug 14827, 2012-05-02*/
#ifdef _CENTEC_ 
      if (self->super.pipe_next == self->writer) 
      {
        self->super.pipe_next = self->writer->pipe_next;
      }
#endif
      log_pipe_deinit(self->writer, NULL, NULL);
      log_pipe_unref(self->writer);
    }
  self->writer = NULL;
  return TRUE;
}
#endif

static gboolean
affile_dw_init(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileDestWriter *self = (AFFileDestWriter *) s;
  int fd, flags;

  if (cfg)
    self->time_reopen = cfg->time_reopen;

  msg_verbose("Initializing destination file writer",
              evt_tag_str("template", self->owner->filename_template->template->str),
              evt_tag_str("filename", self->filename->str),
              NULL);

  if (self->owner->flags & AFFILE_PIPE)
    flags = O_RDWR | O_NOCTTY | O_NONBLOCK | O_LARGEFILE;
  else
    flags = O_WRONLY | O_CREAT | O_APPEND | O_NOCTTY | O_NONBLOCK | O_LARGEFILE;

  self->last_open_stamp = time(NULL);
  if (affile_open_file(self->filename->str, flags, 
                       self->owner->file_uid, self->owner->file_gid, self->owner->file_perm, 
                       self->owner->dir_uid, self->owner->dir_gid, self->owner->dir_perm, 
                       !!(self->owner->flags & AFFILE_CREATE_DIRS), &fd))
    {
      self->writer = log_writer_new(LW_FORMAT_FILE, s, &self->owner->writer_options);
        
      if (!log_pipe_init(self->writer, NULL, NULL))
        {
          msg_error("Error initializing log writer", NULL);
          log_pipe_unref(self->writer);
          self->writer = NULL;
          close(fd);
          return FALSE;
        }
      log_writer_reopen(self->writer, fd_write_new(fd));
      log_pipe_append(&self->super, self->writer);
    }
  else
    {
/*modified by weij for bug 13174, 2010-12-03*/
#if 0 
      msg_error("Error opening file for writing",
                evt_tag_str("filename", self->filename->str),
                evt_tag_errno(EVT_TAG_OSERROR, errno),
                NULL);
#endif
      return self->owner->super.optional;
    }
  return TRUE;
}

#ifndef _CENTEC_
static gboolean
affile_dw_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileDestWriter *self = (AFFileDestWriter *) s;

  if (self->writer)
    {
      log_pipe_deinit(self->writer, NULL, NULL);
      log_pipe_unref(self->writer);
    }
  self->writer = NULL;
  return TRUE;
}
#endif

static void
affile_dw_queue(LogPipe *s, LogMessage *lm, gint path_flags)
{
  AFFileDestWriter *self = (AFFileDestWriter *) s;

  self->last_msg_stamp = time(NULL);
  if (!s->pipe_next && self->last_open_stamp < self->last_msg_stamp - self->time_reopen)
    {
      log_pipe_init(&self->super, NULL, NULL);
    }
    
  if (s->pipe_next)
    log_pipe_forward_msg(s, lm, path_flags);
  else
    log_msg_drop(lm, path_flags);
}

static void
affile_dw_free(LogPipe *s)
{
  AFFileDestWriter *self = (AFFileDestWriter *) s;
  
  g_string_free(self->filename, TRUE);
  log_pipe_unref(&self->owner->super.super);
  
  g_free(s);
}

/* Added by Alexander Liu 2008-08-05 11:15 */
static void
affile_dw_proc_overflow(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
    AFFileDestWriter *self = (AFFileDestWriter *) s;
    LogPipe *op = self->writer;
    LogWriter *ow = (LogWriter *)op;
    GString *partial = NULL;
    gint pos = 0;
    GQueue *queue = NULL;

    /* save old data for this writer */
    if (ow->partial)
    {
        partial = ow->partial;
        ow->partial = NULL;
        pos = ow->partial_pos;
    }

    if (ow->queue && !g_queue_is_empty(ow->queue))
    {
        queue = ow->queue;
        ow->queue = g_queue_new();
        if (NULL == ow->queue)
        {
            /* drop message when out of memory */
            ow->queue = queue;
            queue = NULL;
        }
    }
    
    /* close writer */
    log_pipe_deinit(self->writer, NULL, NULL);
    log_pipe_unref(self->writer);

    /* do sync up and rotate */
    if (self->owner && self->owner->sync_script)
    {
        system(self->owner->sync_script);
    }

    /* reopen writer */
    affile_dw_init(s, NULL, NULL);
    
    /* restore old data */
    op = self->writer;
    ow = (LogWriter *)op;
    if (partial)
    {
        ow->partial = partial;
        ow->partial_pos = pos;
    }
    if (queue)
    {
        g_queue_free(ow->queue);
        ow->queue = queue;
    }
    return;    
}

static void
affile_dw_notify(LogPipe *s, LogPipe *sender, gint notify_code, gpointer user_data)
{
    switch (notify_code)
    {
    case NC_CLOSE:
    case NC_WRITE_ERROR:
        affile_dw_deinit(s, NULL, NULL);
        affile_dw_init(s, NULL, NULL);
        break;
    case NC_SIZE_OVERFLOW:
    case NC_SYNC_UP:
        affile_dw_proc_overflow(s, NULL, NULL);
        break;
    }

    return;
}

static LogPipe *
affile_dw_new(AFFileDestDriver *owner, GString *filename)
{
  AFFileDestWriter *self = g_new0(AFFileDestWriter, 1);
  
  log_pipe_init_instance(&self->super);

  self->super.init = affile_dw_init;
  self->super.deinit = affile_dw_deinit;
  self->super.free_fn = affile_dw_free;  
  self->super.queue = affile_dw_queue;
  /* Added by Alexander Liu 2008-08-05 11:18 */
  self->super.notify = affile_dw_notify;
  log_pipe_ref(&owner->super.super);
  self->owner = owner;
  self->time_reopen = 60;
  
  /* we have to take care about freeing filename later. 
     This avoids a move of the filename. */
  self->filename = filename;
  return &self->super;
}

/* Added by Alexander Liu 2008-08-05 14:16 */
void
affile_dd_set_size_limitation(LogDriver *s, gint maxsize)
{
    AFFileDestDriver *self = (AFFileDestDriver *) s;
    LogWriterOptions *lopt = &self->writer_options;

    lopt->options |= LWO_SIZE_LIMITATION;
    lopt->max_size = maxsize;
    lopt->cur_size = 0;
    return;
}

void 
affile_dd_set_sync_script(LogDriver *s, const gchar *syncscript)
{
    AFFileDestDriver *self = (AFFileDestDriver *) s;

    if (NULL != syncscript)
    {
        self->sync_script = g_strdup(syncscript);
        if (NULL == self->sync_script)
        {
            msg_error("out of memory when setting sync up script", NULL);
        }
    }
    return;
}

void 
affile_dd_set_compress(LogDriver *s, gboolean compress G_GNUC_UNUSED)
{
  msg_error("On-the-fly file compression is not supported", NULL);
}

void 
affile_dd_set_encrypt(LogDriver *s, gboolean compress G_GNUC_UNUSED)
{
  msg_error("On-the-fly file encryption is not supported", NULL);
}

void 
affile_dd_set_file_uid(LogDriver *s, const gchar *file_uid)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->file_uid = 0;
  if (!resolve_user(file_uid, &self->file_uid))
    {
      msg_notice("Error resolving user",
                 evt_tag_str("user", file_uid),
                 NULL);
    }
}

void 
affile_dd_set_file_gid(LogDriver *s, const gchar *file_gid)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->file_gid = 0;
  if (!resolve_group(file_gid, &self->file_gid))
    {
      msg_notice("Error resolving group",
                 evt_tag_str("group", file_gid),
                 NULL);
    }
}

void 
affile_dd_set_file_perm(LogDriver *s, mode_t file_perm)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->file_perm = file_perm;
}

void 
affile_dd_set_dir_uid(LogDriver *s, const gchar *dir_uid)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->dir_uid = 0;
  if (!resolve_user(dir_uid, &self->dir_uid))
    {
      msg_notice("Error resolving user",
                 evt_tag_str("user", dir_uid),
                 NULL);
    }
}

void 
affile_dd_set_dir_gid(LogDriver *s, const gchar *dir_gid)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->dir_gid = 0;
  if (!resolve_group(dir_gid, &self->dir_gid))
    {
      msg_notice("Error resolving group",
                 evt_tag_str("group", dir_gid),
                 NULL);
    }
}

void 
affile_dd_set_dir_perm(LogDriver *s, mode_t dir_perm)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  self->dir_perm = dir_perm;
}

void 
affile_dd_set_create_dirs(LogDriver *s, gboolean create_dirs)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  if (create_dirs)
    self->flags |= AFFILE_CREATE_DIRS;
  else 
    self->flags &= ~AFFILE_CREATE_DIRS;
}

static const gchar *
affile_dd_format_stats_name(AFFileDestDriver *self, gboolean is_pipe)
{
  static gchar stats_name[256];

  g_snprintf(stats_name, sizeof(stats_name),
             "%s(%s)", is_pipe ? "pipe" : "file", self->filename_template->template->str);

  return stats_name;
}

static time_t reap_now = 0;

static gboolean
affile_dd_reap_writers(gpointer key, gpointer value, gpointer user_data)
{
  AFFileDestDriver *self = (AFFileDestDriver *) user_data;
  AFFileDestWriter *dw = (AFFileDestWriter *) value;
  
  if ((reap_now - dw->last_msg_stamp) >= self->time_reap)
    {
      msg_verbose("Destination timed out, reaping", 
                  evt_tag_str("template", self->filename_template->template->str),
                  evt_tag_str("filename", dw->filename->str),
                  NULL);
      log_pipe_deinit(&dw->super, NULL, NULL);
      log_pipe_unref(&dw->super);
      
      /* remove from hash table */
      return TRUE;
    }
  return FALSE;
}

static gboolean
affile_dd_reap(gpointer s)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  msg_verbose("Reaping unused destination files",
              evt_tag_str("template", self->filename_template->template->str),
              NULL);  
  reap_now = time(NULL);
  if (self->writer_hash)
    g_hash_table_foreach_remove(self->writer_hash, affile_dd_reap_writers, self);
  return TRUE;
}

/* Added by Alexander Liu 2008-08-05 15:34 */
static void 
affile_dd_sync_writers(gpointer key, gpointer value, gpointer user_data)
{
    LogPipe *dw = (LogPipe *) value;
    affile_dw_notify(dw, NULL, NC_SYNC_UP, NULL);
    return;
}

static gboolean
affile_dd_sync_timer(gpointer s)
{
    AFFileDestDriver *self = (AFFileDestDriver *) s;

    if (NULL == self->sync_script)
    {
        return TRUE;
    }

    if (self->writer_hash)
    {
        g_hash_table_foreach(self->writer_hash, affile_dd_sync_writers, self);
    }
    else if (self->writer)
    {
        affile_dw_notify(self->writer, NULL, NC_SYNC_UP, NULL);
    }
    else
    {
        system(self->sync_script);
    }
    return TRUE;
}

static gboolean
affile_dd_init(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  
  if (cfg->create_dirs)
    self->flags |= AFFILE_CREATE_DIRS;
  if (self->file_uid == -1)
    self->file_uid = cfg->file_uid;
  if (self->file_gid == -1)
    self->file_gid = cfg->file_gid;
  if (self->file_perm == -1)
    self->file_perm = cfg->file_perm;
  if (self->dir_uid == -1)
    self->dir_uid = cfg->dir_uid;
  if (self->dir_gid == -1)
    self->dir_gid = cfg->dir_gid;
  if (self->dir_perm == -1)
    self->dir_perm = cfg->dir_perm;
  if (self->time_reap == -1)
    self->time_reap = cfg->time_reap;
  
  self->use_time_recvd = cfg->use_time_recvd;
  
  if (self->flags & AFFILE_PIPE)
    log_writer_options_init(&self->writer_options, cfg, LWOF_SHARE_STATS, affile_dd_format_stats_name(self, TRUE));
  else    
    log_writer_options_init(&self->writer_options, cfg, LWOF_NO_STATS, NULL);
    
  self->cfg = cfg;
  
  if ((self->flags & AFFILE_NO_EXPAND) == 0)
    {
      self->reap_timer = g_timeout_add_full(G_PRIORITY_LOW, self->time_reap * 1000 / 2, affile_dd_reap, self, NULL);
    }
  /* Added by Alexander Liu 2008-08-07 17:32 */
  if (self->sync_script)
  {
      self->sync_timer = g_timeout_add_full(G_PRIORITY_LOW,
              M_SYNC_UP_INTERVAL * 1000, affile_dd_sync_timer, self, NULL);
  }
  return TRUE;
}

static gboolean
affile_dd_remove_writers(gpointer key, gpointer value, gpointer user_data)
{
  AFFileDestDriver *self = (AFFileDestDriver *) user_data;
  LogPipe *writer = (LogPipe *) value;
  
  log_pipe_deinit(writer, self->cfg, NULL);
  log_pipe_unref(writer);
  return TRUE;
}

extern gboolean sig_hup_received;

static gboolean
affile_dd_deinit(LogPipe *s, GlobalConfig *cfg, PersistentConfig *persist)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;

  if (self->writer) {
    log_pipe_deinit(self->writer, cfg, NULL);
    /*add by weij for bug 19911, 2012-08-29: delete start message*/
    log_pipe_unref(self->writer);
    self->writer = NULL;
  }
  if (self->writer_hash) {
    g_hash_table_foreach_remove(self->writer_hash, affile_dd_remove_writers, self);
    /*add by weij for bug 19911, 2012-08-29: delete start message*/
    g_hash_table_destroy(self->writer_hash);
    self->writer_hash = NULL;
  }
  if (self->reap_timer)
    g_source_remove(self->reap_timer);
   /* Added by Alexander Liu 2008-08-05 15:40 */
   if (self->sync_timer)
   {
       if (sig_hup_received && self->sync_script) {
            system(self->sync_script);
       }
       g_source_remove(self->sync_timer);
       self->sync_timer = 0;
   }
  self->cfg = NULL;
  return TRUE;
}

static void
affile_dd_queue(LogPipe *s, LogMessage *msg, gint path_flags)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;
  LogPipe *next;

  if (self->flags & AFFILE_NO_EXPAND)
    {
      if (!self->writer)
	{
	  next = affile_dw_new(self, g_string_new(self->filename_template->template->str));
	  if (next && log_pipe_init(next, self->cfg, NULL))
	    {
	      self->writer = next;
	    }
	  else
	    {
	      log_pipe_unref(next);
	      next = NULL;
	    }
	}
      else
        {
          next = self->writer;
        }
    }
  else
    {
      GString *filename;

      if (!self->writer_hash)
	self->writer_hash = g_hash_table_new(g_str_hash, g_str_equal);

      filename = g_string_sized_new(32);
      log_template_format(self->filename_template, msg, 
		    ((self->flags & AFFILE_TMPL_ESCAPE) ? MF_ESCAPE_RESULT : 0) |
		    (self->use_time_recvd ? MF_STAMP_RECVD : 0),
		    TS_FMT_BSD,
		    get_local_timezone_ofs(time(NULL)),
		    0,
		    filename);
      next = g_hash_table_lookup(self->writer_hash, filename->str);
      if (!next)
	{
	  next = affile_dw_new(self, filename);
	  if (!log_pipe_init(next, self->cfg, NULL))
	    {
	      log_pipe_unref(next);
	      next = NULL;
	    }
	  else
	    g_hash_table_insert(self->writer_hash, filename->str, next);
	}
      else
        g_string_free(filename, TRUE);
    }
  if (next)
    log_pipe_queue(next, msg, path_flags);
  else
    log_msg_drop(msg, path_flags);
}

void
affile_dd_free(LogPipe *s)
{
  AFFileDestDriver *self = (AFFileDestDriver *) s;

#ifdef DEBUG_MEM_LEAK
  free_file_dd_count++;
  printf("affile_dd_free:  %s, %ld\n", (char*)self->filename_template->template->str, free_file_dd_count);
  char tmp[256];
  sprintf(tmp, "affile_dd_free:  %s, %ld\n", (char*)self->filename_template->template->str, free_file_dd_count);
  write_debug_file(tmp);
#endif
  log_template_unref(self->filename_template);
/*del by weij for bug 19911, 2012-08-29: delete start message*/
#if 0
  log_pipe_unref(self->writer);
  if (self->writer_hash)
    g_hash_table_destroy(self->writer_hash);
#endif
  log_writer_options_destroy(&self->writer_options);
  log_drv_free_instance(&self->super);
  /*modified by weij for memory leak, 2012-05-02*/
  if (self->sync_script)
  {
     g_free(self->sync_script);
  }
  g_free(self);
}

LogDriver *
affile_dd_new(gchar *filename, guint32 flags)
{
  AFFileDestDriver *self = g_new0(AFFileDestDriver, 1);

  log_drv_init_instance(&self->super);
  self->super.super.init = affile_dd_init;
  self->super.super.deinit = affile_dd_deinit;
  self->super.super.queue = affile_dd_queue;
  self->super.super.free_fn = affile_dd_free;
  self->filename_template = log_template_new(NULL, filename);
  self->flags = flags;
  self->file_uid = self->file_gid = -1;
  self->file_perm = -1;
  self->dir_uid = self->dir_gid = -1;
  self->dir_perm = -1;
  log_writer_options_defaults(&self->writer_options);
  if (strchr(filename, '$') == NULL)
    {
      self->flags |= AFFILE_NO_EXPAND;
    }
  self->time_reap = -1;
#ifdef DEBUG_MEM_LEAK
  new_file_dd_count++;
  printf("affile_dd_new: new_file_dd_count: %s, %ld\n", (char*)filename, new_file_dd_count);
  char tmp[256];
  sprintf(tmp, "affile_dd_new: new_file_dd_count: %s, %ld\n", (char*)filename, new_file_dd_count);
  write_debug_file(tmp);
#endif
  return &self->super;
}

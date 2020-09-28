/*
 * Buffering to output and input. 
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ZEBRA_BUFFER_H
#define _ZEBRA_BUFFER_H

/* Buffer master. */
struct buffer
{
  /* Data list. */
  struct buffer_data *head;
  struct buffer_data *tail;

  /* Current allocated data. */
  unsigned long alloc;

  /* Total length of buffer. */
  unsigned long size;

  /* For allocation. */
  struct buffer_data *unused_head;
  struct buffer_data *unused_tail;

  /* Current total length of this buffer. */
  unsigned long length;
};

/* Data container. */
struct buffer_data
{
  struct buffer *parent;
  struct buffer_data *next;
  struct buffer_data *prev;

  /* Acctual data stream. */
  unsigned char *data;

  /* Current pointer. */
  unsigned long cp;

  /* Start pointer. */
  unsigned long sp;
};

/* Buffer prototypes. */
struct buffer *vtysh_buffer_new (size_t size);
int vtysh_buffer_write (struct buffer *, unsigned char*, size_t);
void buffer_free (struct buffer *);
int vtysh_buffer_putc (struct buffer *, unsigned char);
int buffer_putstr (struct buffer *, unsigned char *);
void vtysh_buffer_reset (struct buffer *);
int vtysh_buffer_flush_all (struct buffer *, int);
int buffer_flush_vty_all (struct buffer *, int, int, int);
int vtysh_buffer_flush_window (struct buffer *, int, int, int, int, int);
int vtysh_buffer_empty (struct buffer *);

#endif /* _ZEBRA_BUFFER_H */

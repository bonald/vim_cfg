/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */
#ifndef VTYSH_H
#define VTYSH_H

#include "ctc_cli.h"

#define VTYSH_SDK       0x01
#define VTYSH_OVS       0x02
#define VTYSH_ALL       (VTYSH_SDK|VTYSH_OVS)

#define VTYSH_INDEX_SDK     0
#define VTYSH_INDEX_OVS     1
#define VTYSH_INDEX_MAX     2

/* UNIX domain socket path. */
#define SDK_PATH "/tmp/.vtysh_sdk"

#define RETSIGTYPE void

/* vtysh local configuration file. */
#define VTYSH_DEFAULT_CONFIG "vtysh.conf"

void ctc_vti_init ();
void vtysh_vty_init();
void vtysh_init_cmd ();
void vtysh_connect_all ();
void vtysh_readline_init ();
void vtysh_user_init ();

void vtysh_execute (char *);
void vtysh_execute_no_pager (char *);

char *vtysh_prompt ();

void vtysh_config_write ();

int vtysh_config_from_file (struct vty *, FILE *);

void vtysh_read_config (char *, char *, char *);

void vtysh_config_parse (char *);

void vtysh_config_dump (FILE *);

void vtysh_config_init ();

void vtysh_pager_init ();

/* Child process execution flag. */
extern int execute_flag;

extern struct vty *vty;

#endif /* VTYSH_H */

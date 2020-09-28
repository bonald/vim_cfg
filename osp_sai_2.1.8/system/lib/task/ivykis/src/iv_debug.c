/*
 * ivykis, an event handling library
 * Copyright (C) 2010, 2012 Lennert Buytenhek
 * Dedicated to Marija Kulikova.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version
 * 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 2.1 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License version 2.1 along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iv.h>
#include <iv_event.h>
#include <iv_event_raw.h>
#include <iv_tls.h>
#include "iv_private.h"

#define DEBUG_OUT   printf

void dump_iv_state()
{
	struct iv_state* st = iv_get_state();

	struct iv_list_head *ilh;
    struct iv_task_ *t;
    int i = 0;

    
    DEBUG_OUT("############ IV State Begin ############\n");
    DEBUG_OUT("quit:        %d\n", st->quit);
    DEBUG_OUT("numobjs:     %d\n", st->numobjs);

    DEBUG_OUT("########    FD    ##########\n");
    DEBUG_OUT("numfds:      %d\n", st->numfds);

    
    DEBUG_OUT("########   Task   ##########\n");
	iv_list_for_each (ilh, &st->tasks) {
		t = iv_container_of(ilh, struct iv_task_, list);
        DEBUG_OUT("task[%d]\n", i);
        DEBUG_OUT("  cookie:      %p\n", t->cookie);
        DEBUG_OUT("  handler:     %p\n", t->handler);
        i++;
	}

    DEBUG_OUT("########   Timer  ##########\n");
    DEBUG_OUT("time:        %ld.%09ld\n", st->time.tv_sec, st->time.tv_nsec);
    DEBUG_OUT("time_valid:  %d\n", st->time_valid);
    DEBUG_OUT("num_timers:  %d\n", st->num_timers);
    DEBUG_OUT("rat_depth:   %d\n", st->rat_depth);
        
    DEBUG_OUT("############  IV State End  ############\n");
    DEBUG_OUT("\n");
}

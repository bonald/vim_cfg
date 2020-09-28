/*
 * Copyright (c) 1998-2001
 * University of Southern California/Information Sciences Institute.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 *  $Id: debug.c,v 1.22 2001/11/28 00:13:50 pavlin Exp $
 */
/*
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */

#define SYSLOG_NAMES
#include "defs.h"

#include <stdarg.h>
#include <stdio.h>

#define MAX_MSG_SIZE 64                  /* Max for dump_frame() */
static int igmp_groups_num = 0;
int log_nmsgs = 0;
int loglevel = LOG_NOTICE;
unsigned long debug = 0x00000000;        /* If (long) is smaller than
					  * 4 bytes, then we are in
					  * trouble.
					  */

char *packet_kind(int proto, int type, int code)
{
    static char unknown[60];

    switch (proto) {
	case IPPROTO_IGMP:
	    switch (type) {
		case IGMP_MEMBERSHIP_QUERY:     return "IGMP Membership Query    ";
		case IGMP_V1_MEMBERSHIP_REPORT: return "IGMP v1 Membership Report";
		case IGMP_V2_MEMBERSHIP_REPORT: return "IGMP v2 Membership Report";
		case IGMP_V3_MEMBERSHIP_REPORT: return "IGMP v3 Membership Report";
		case IGMP_V2_LEAVE_GROUP:       return "IGMP Leave message       ";
		case IGMP_DVMRP:
		    switch (code) {
			case DVMRP_PROBE:          return "DVMRP Neighbor Probe     ";
			case DVMRP_REPORT:         return "DVMRP Route Report       ";
			case DVMRP_ASK_NEIGHBORS:  return "DVMRP Neighbor Request   ";
			case DVMRP_NEIGHBORS:      return "DVMRP Neighbor List      ";
			case DVMRP_ASK_NEIGHBORS2: return "DVMRP Neighbor request 2 ";
			case DVMRP_NEIGHBORS2:     return "DVMRP Neighbor list 2    ";
			case DVMRP_PRUNE:          return "DVMRP Prune message      ";
			case DVMRP_GRAFT:          return "DVMRP Graft message      ";
			case DVMRP_GRAFT_ACK:      return "DVMRP Graft message ack  ";
			case DVMRP_INFO_REQUEST:   return "DVMRP Info Request       ";
			case DVMRP_INFO_REPLY:     return "DVMRP Info Reply         ";
			default:
			    snprintf(unknown, sizeof(unknown), "UNKNOWN DVMRP message code = %3d ", code);
			    return unknown;
		    }

		case IGMP_PIM:
		    /* The old style (PIM v1) encapsulation of PIM messages
		     * inside IGMP messages.
		     */
		    /* PIM v1 is not implemented but we just inform that a message
		     *	has arrived.
		     */
		    switch (code) {
			case PIM_V1_QUERY:         return "PIM v1 Router-Query      ";
			case PIM_V1_REGISTER:      return "PIM v1 Register          ";
			case PIM_V1_REGISTER_STOP: return "PIM v1 Register-Stop     ";
			case PIM_V1_JOIN_PRUNE:    return "PIM v1 Join/Prune        ";
			case PIM_V1_RP_REACHABILITY:
			    return "PIM v1 RP-Reachability   ";

			case PIM_V1_ASSERT:        return "PIM v1 Assert            ";
			case PIM_V1_GRAFT:         return "PIM v1 Graft             ";
			case PIM_V1_GRAFT_ACK:     return "PIM v1 Graft_Ack         ";
			default:
			    snprintf(unknown, sizeof(unknown), "UNKNOWN PIM v1 message type =%3d ", code);
			    return unknown;
		    }

		case IGMP_MTRACE:              return "IGMP trace query         ";
		case IGMP_MTRACE_RESP:         return "IGMP trace reply         ";
		default:
		    snprintf(unknown, sizeof (unknown), "UNKNOWN IGMP message: type = 0x%02x, code = 0x%02x ", type, code);
		    return unknown;
	    }

	case IPPROTO_PIM:    /* PIM v2 */
	    switch (type) {
		case PIM_V2_HELLO:             return "PIM v2 Hello             ";
		case PIM_V2_REGISTER:          return "PIM v2 Register          ";
		case PIM_V2_REGISTER_STOP:     return "PIM v2 Register_Stop     ";
		case PIM_V2_JOIN_PRUNE:        return "PIM v2 Join/Prune        ";
		case PIM_V2_BOOTSTRAP:         return "PIM v2 Bootstrap         ";
		case PIM_V2_ASSERT:            return "PIM v2 Assert            ";
		case PIM_V2_GRAFT:             return "PIM-DM v2 Graft          ";
		case PIM_V2_GRAFT_ACK:         return "PIM-DM v2 Graft_Ack      ";
		case PIM_V2_CAND_RP_ADV:       return "PIM v2 Cand. RP Adv.     ";
		default:
		    snprintf(unknown, sizeof(unknown), "UNKNOWN PIM v2 message type =%3d ", type);
		    return unknown;
	    }

	default:
	    snprintf(unknown, sizeof(unknown), "UNKNOWN proto =%3d               ", proto);
	    return unknown;
    }
}


/*
 * Used for debugging particular type of messages.
 */
int debug_kind(int proto, int type, int code)
{
    switch (proto) {
	case IPPROTO_IGMP:
	    switch (type) {
		case IGMP_MEMBERSHIP_QUERY:        return DEBUG_IGMP;
		case IGMP_V1_MEMBERSHIP_REPORT:    return DEBUG_IGMP;
		case IGMP_V2_MEMBERSHIP_REPORT:    return DEBUG_IGMP;
		case IGMP_V3_MEMBERSHIP_REPORT:    return DEBUG_IGMP;
		case IGMP_V2_LEAVE_GROUP:          return DEBUG_IGMP;
		case IGMP_DVMRP:
		    switch (code) {
			case DVMRP_PROBE:              return DEBUG_DVMRP_PEER;
			case DVMRP_REPORT:             return DEBUG_DVMRP_ROUTE;
			case DVMRP_ASK_NEIGHBORS:      return 0;
			case DVMRP_NEIGHBORS:          return 0;
			case DVMRP_ASK_NEIGHBORS2:     return 0;
			case DVMRP_NEIGHBORS2:         return 0;
			case DVMRP_PRUNE:              return DEBUG_DVMRP_PRUNE;
			case DVMRP_GRAFT:              return DEBUG_DVMRP_PRUNE;
			case DVMRP_GRAFT_ACK:          return DEBUG_DVMRP_PRUNE;
			case DVMRP_INFO_REQUEST:       return 0;
			case DVMRP_INFO_REPLY:         return 0;
			default:                       return 0;
		    }

		case IGMP_PIM:
		    /* PIM v1 is not implemented */
		    switch (code) {
			case PIM_V1_QUERY:             return DEBUG_PIM;
			case PIM_V1_REGISTER:          return DEBUG_PIM;
			case PIM_V1_REGISTER_STOP:     return DEBUG_PIM;
			case PIM_V1_JOIN_PRUNE:        return DEBUG_PIM;
			case PIM_V1_RP_REACHABILITY:   return DEBUG_PIM;
			case PIM_V1_ASSERT:            return DEBUG_PIM;
			case PIM_V1_GRAFT:             return DEBUG_PIM;
			case PIM_V1_GRAFT_ACK:         return DEBUG_PIM;
			default:                       return DEBUG_PIM;
		    }

		case IGMP_MTRACE:                  return DEBUG_TRACE;
		case IGMP_MTRACE_RESP:             return DEBUG_TRACE;
		default:                           return DEBUG_IGMP;
	    }

	case IPPROTO_PIM:       /* PIM v2 */
	    /* TODO: modify? */
	    switch (type) {
		case PIM_V2_HELLO:             return DEBUG_PIM;
		case PIM_V2_REGISTER:          return DEBUG_PIM_REGISTER;
		case PIM_V2_REGISTER_STOP:     return DEBUG_PIM_REGISTER;
		case PIM_V2_JOIN_PRUNE:        return DEBUG_PIM;
		case PIM_V2_BOOTSTRAP:         return DEBUG_PIM_BOOTSTRAP;
		case PIM_V2_ASSERT:            return DEBUG_PIM;
		case PIM_V2_GRAFT:             return DEBUG_PIM;
		case PIM_V2_GRAFT_ACK:         return DEBUG_PIM;
		case PIM_V2_CAND_RP_ADV:       return DEBUG_PIM_CAND_RP;
		default:                       return DEBUG_PIM;
	    }

	default:                               return 0;
    }

    return 0;
}


/*
 * Some messages are more important than others.  This routine
 * determines the logging level at which to log a send error (often
 * "No route to host").  This is important when there is asymmetric
 * reachability and someone is trying to, i.e., mrinfo me periodically.
 */
int
log_level(int proto, int type, int code)
{
    switch (proto) {
	case IPPROTO_IGMP:
	    switch (type) {
		case IGMP_MTRACE_RESP:
		    return LOG_INFO;

		case IGMP_DVMRP:
		    switch (code) {
			case DVMRP_NEIGHBORS:
			case DVMRP_NEIGHBORS2:
			    return LOG_INFO;
		    }
		    return LOG_WARNING;

		case IGMP_PIM:
		    /* PIM v1 */
		    switch (code) {
			default:
			    return LOG_INFO;
		    }
		    return LOG_WARNING;

		default:
		    return LOG_WARNING;
	    }

	case IPPROTO_PIM:
	    /* PIM v2 */
	    switch (type) {
		default:
		    return LOG_INFO;
	    }
	    return LOG_WARNING;

	default:
	    return LOG_WARNING;
    }
    return LOG_WARNING;
}


/*
 * Dump internal data structures to a file.
 */
void fdump(char *fmt)
{
    FILE *fp;
    char file[90];

    snprintf(file, sizeof(file), fmt, ident);
    fp = fopen(file, "w");
    if (fp) {
	dump_vifs(fp);
	dump_pim_mrt(fp);
	fclose(fp);
    }
}

/* TODO: dummy, to be used in the future. */
/*
 * Dump local cache contents to a file.
 */
void cdump(char *fmt)
{
    FILE *fp;
    char file[90];

    snprintf(file, sizeof(file), fmt, ident);
    fp = fopen(file, "w");
    if (fp) {
	/* XXX: TODO: implement it:
	   dump_cache(fp);
	*/
	fclose(fp);
    }
}

/*
  1         2         3         4         5         6         7         8
  012345678901234567890123456789012345678901234567890123456789012345678901234567890
  Virtual Interface Table
  Vif  Local-Address    Subnet                Thresh  Flags          Neighbors
  0  10.0.3.1         10.0.3/24             1       DR NO-NBR
  1  172.16.12.254    172.16.12/24          1       DR PIM         172.16.12.2
  172.16.12.3
  2  192.168.122.147  register_vif0         1
*/
void dump_vifs(FILE *fp)
{
    vifi_t vifi;
    struct uvif *v;
    pim_nbr_entry_t *n;
    int width;
    int i;
    struct listaddr *group, *source;

    fprintf(fp, "Virtual Interface Table ======================================================\n");
    fprintf(fp, "Vif  Local Address    Subnet              Thresh  Flags      Neighbors\n");
    fprintf(fp, "---  ---------------  ------------------  ------  ---------  -----------------\n");

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	int down = 0;

	fprintf(fp, "%3u  %-15s  ", vifi, inet_fmt(v->uv_lcl_addr, s1, sizeof(s1)));

	if (v->uv_flags & VIFF_REGISTER)
	    fprintf(fp, "%-18s  ", v->uv_name);
	else
	    fprintf(fp,"%-18.18s  ", netname(v->uv_subnet, v->uv_subnetmask));

	fprintf(fp, "%6u ", v->uv_threshold);

	//TODO: XXX: Print VIFF_TUNNEL?
	width = 0;
	if (v->uv_flags & VIFF_DISABLED) {
	    fprintf(fp, " DISABLED");
	    down = 1;
	}
	if (v->uv_flags & VIFF_DOWN) {
	    fprintf(fp, " DOWN");
	    down = 1;
	}

	if (v->uv_flags & VIFF_DR) {
	    fprintf(fp, " DR");
	    width += 3;
	}
	if (v->uv_flags & VIFF_PIM_NBR) {
	    fprintf(fp, " PIM");
	    width += 4;
	}
	if (v->uv_flags & VIFF_DVMRP_NBR) {
	    fprintf(fp, " DVMRP");
	    width += 6;
	}
	if (v->uv_flags & VIFF_NONBRS) {
	    fprintf(fp, " NO-NBR");
	    width += 6;
	}

	n = v->uv_pim_neighbors;
	if (!down && n) {
	    for (i = width; i <= 11; i++)
		fprintf(fp, " ");
	    fprintf(fp, "%-15s\n", inet_fmt(n->address, s1, sizeof(s1)));
	    for (n = n->next; n; n = n->next)
		fprintf(fp, "%61s%-15s\n", "", inet_fmt(n->address, s1, sizeof(s1)));
	} else {
	    fprintf(fp, "\n");
	}
    }

    fprintf(fp, "\n");

    // Dump groups and sources
    fprintf(fp, " %-3s  %-15s  %-20s", "Vif", "SSM Group", "Sources");
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	for (group = v->uv_groups; group != NULL; group = group->al_next) {
	    if (IN_PIM_SSM_RANGE(group->al_addr)) {
		fprintf(fp, "\n %3u  %-15s ", vifi, inet_fmt(group->al_addr, s1, sizeof(s1)));
		for (source = group->al_sources; source != NULL; source = source->al_next) {
		    fprintf(fp, "%s ", inet_fmt(source->al_addr, s1, sizeof(s1)));
		}
	    }
	}
    }

    fprintf(fp, "\n\n");
}

/*add by zhw for support cisco-like show ip pim neighbor
	add by zhw:if do not have neighbour,continue
	show ip pim interface to show this system info*/
void dump_vifs_new(FILE *fp)
{
    vifi_t vifi;
    struct uvif *v;
    pim_nbr_entry_t *n;
    int width;
    int i;
    struct listaddr *group, *source;
	struct pim_nbr_entry *nbr;
	time_t now_time, up_time;
	char f_uptime[10];
	
    fprintf(fp, "PIM Sparse-Mode Neighbor Table\n");
	fprintf(fp,"-----------------------------------------------------------------------------\n");
    fprintf(fp, "Interface      Neighbors        Thresh  Uptime      Expires     Prio  DR    \n");
    fprintf(fp, "-------------  ---------------  ------  ----------  ----------  ----  ------\n");

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	  if(!v->uv_pim_neighbors)
        continue;
	  
	  /*VIFF_REGISTER name is register_vif0*/
	  for(nbr = v->uv_pim_neighbors; nbr; nbr=nbr->next){
	  	
	    fprintf(fp, "%-13s  ", v->uv_name);
		fprintf(fp, "%-15s  ", inet_fmt(nbr->address, s1, sizeof(s1)));
	    fprintf(fp, "%-6u ", v->uv_threshold);
		
		time(&now_time);
		up_time = now_time - nbr->c_time;
		get_format_time(up_time, f_uptime);
		fprintf(fp,"%-10s  ", f_uptime);

		get_format_time(nbr->timer, f_uptime);
		fprintf(fp,"%-10s  ", f_uptime);

		if(nbr->dr_prio_present)
			fprintf(fp,"%-4d  ", nbr->dr_prio);
		else 
			fprintf(fp,"%-4s  ", "NULL");

		if(nbr->is_dr)
			fprintf(fp, "%-6s  ", "DR");
		else 
			fprintf(fp, "%-6s  ", "NON-DR");
		
		fprintf(fp, "\n");
	  }
    }
}

void log_init(int log_stdout)
{
    int log_opts = LOG_PID;

#ifdef LOG_PERROR
    if (log_stdout)
	log_opts |= LOG_PERROR;
#endif

#ifdef LOG_DAEMON
    openlog(ident, log_opts, LOG_DAEMON);
    setlogmask(LOG_UPTO(loglevel));
#else
    openlog(ident, log_opts);
#endif
}

int loglvl(char *level)
{
    int i;

    for (i = 0; prioritynames[i].c_name; i++) {
	size_t len = MIN(strlen(prioritynames[i].c_name), strlen(level));

	if (!strncasecmp(prioritynames[i].c_name, level, len))
	    return prioritynames[i].c_val;
    }

    return atoi(level);
}

/*
 * Log errors and other messages to the system log daemon and to stderr,
 * according to the severity of the message and the current debug level.
 * For errors of severity LOG_ERR or worse, terminate the program.
 */
 #if 0
void logit(int severity, int syserr, const char *format, ...)
{
    va_list ap;
    char msg[211];
    struct timeval now;
    struct tm *thyme;
    time_t lt;

    va_start(ap, format);
    vsnprintf(msg, sizeof(msg), format, ap);
    va_end(ap);

    /*
     * Log to stderr if we haven't forked yet and it's a warning or
     * worse, or if we're debugging.
     */
    if (haveterminal && (debug || severity <= LOG_WARNING)) {
	gettimeofday(&now, NULL);
	lt = now.tv_sec;
	thyme = localtime(&lt);

	if (!debug)
	    fprintf(stderr, "%s: ", prognm);

	fprintf(stderr, "%02d:%02d:%02d.%03ld %s", thyme->tm_hour, thyme->tm_min,
		thyme->tm_sec, (long int)(now.tv_usec / 1000), msg);

	if (syserr)
	    fprintf(stderr, ": %s", strerror(syserr));

	fprintf(stderr, "\n");
    }

    /*
     * Always log things that are worse than warnings, no matter what
     * the log_nmsgs rate limiter says.
     *
     * Only count things at the defined loglevel or worse in the rate limiter
     * and exclude debugging (since if you put daemon.debug in syslog.conf
     * you probably actually want to log the debugging messages so they
     * shouldn't be rate-limited)
     */
    if ((severity < LOG_WARNING) || (log_nmsgs < LOG_MAX_MSGS)) {
	if ((severity <= loglevel) && (severity != LOG_DEBUG))
	    log_nmsgs++;

	if (syserr)
	    syslog(severity, "%s: %s", msg, strerror(syserr));
	else
	    syslog(severity, "%s", msg);
    }

#ifndef CONTINUE_ON_ERROR
    if (severity <= LOG_ERR)
	exit(-1);		/* Exit status: 255 */
#endif /* CONTINUE_ON_ERROR */
}
#else


#endif

/*
 * Hex dump a control frame to the log, MAX 64 bytes length
 */
void dump_frame(char *desc, void *dump, size_t len)
{
    unsigned int length, i = 0;
    unsigned char *data = (unsigned char *)dump;
    char buf[80] = "";
    char tmp[10];

    length = len;
    if (length > MAX_MSG_SIZE)
	length = MAX_MSG_SIZE;

    if (desc)
	logit(LOG_DEBUG, 0, "%s", desc);

    while (i < length) {
	if (!(i % 16))
	    snprintf(buf, sizeof(buf), "%03X: ", i);

	snprintf(tmp, sizeof(tmp), "%02X ", data[i++]);
	strlcat(buf, tmp, sizeof(buf));

	if (i > 0 && !(i % 16))
	    logit(LOG_DEBUG, 0, "%s", buf);
	else if (i > 0 && !(i % 8))
	    strlcat(buf, ":: ", sizeof(buf));
    }
    logit(LOG_DEBUG, 0, "%s", buf);
}


static void dump_route(FILE *fp, mrtentry_t *r)
{
    vifi_t vifi;
    char oifs[(sizeof(vifbitmap_t)<<3)+1];
    char joined_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char pruned_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char leaves_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char asserted_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char incoming_iif[(sizeof(vifbitmap_t)<<3)+1];

    for (vifi = 0; vifi < numvifs; vifi++) {
	oifs[vifi] =
	    VIFM_ISSET(vifi, r->oifs)	       ? 'o' : '.';
	joined_oifs[vifi] =
	    VIFM_ISSET(vifi, r->joined_oifs)   ? 'j' : '.';
	pruned_oifs[vifi] =
	    VIFM_ISSET(vifi, r->pruned_oifs)   ? 'p' : '.';
	leaves_oifs[vifi] =
	    VIFM_ISSET(vifi, r->leaves)	       ? 'l' : '.';
	asserted_oifs[vifi] =
	    VIFM_ISSET(vifi, r->asserted_oifs) ? 'a' : '.';
	incoming_iif[vifi] = '.';
    }
    oifs[vifi]		= 0x0;	/* End of string */
    joined_oifs[vifi]	= 0x0;
    pruned_oifs[vifi]	= 0x0;
    leaves_oifs[vifi]	= 0x0;
    asserted_oifs[vifi] = 0x0;
    incoming_iif[vifi]	= 0x0;
    incoming_iif[r->incoming] = 'I';

    /* TODO: don't need some of the flags */
    if (r->flags & MRTF_SPT)	       fprintf(fp, " SPT");
    if (r->flags & MRTF_WC)	       fprintf(fp, " WC");
    if (r->flags & MRTF_RP)	       fprintf(fp, " RP");
    if (r->flags & MRTF_REGISTER)      fprintf(fp, " REG");
    if (r->flags & MRTF_IIF_REGISTER)  fprintf(fp, " IIF_REG");
    if (r->flags & MRTF_NULL_OIF)      fprintf(fp, " NULL_OIF");
    if (r->flags & MRTF_KERNEL_CACHE)  fprintf(fp, " CACHE");
    if (r->flags & MRTF_ASSERTED)      fprintf(fp, " ASSERTED");
    if (r->flags & MRTF_REG_SUPP)      fprintf(fp, " REG_SUPP");
    if (r->flags & MRTF_SG)	       fprintf(fp, " SG");
    if (r->flags & MRTF_PMBR)	       fprintf(fp, " PMBR");
    fprintf(fp, "\n");

    fprintf(fp, "Joined   oifs: %-20s\n", joined_oifs);
    fprintf(fp, "Pruned   oifs: %-20s\n", pruned_oifs);
    fprintf(fp, "Leaves   oifs: %-20s\n", leaves_oifs);
    fprintf(fp, "Asserted oifs: %-20s\n", asserted_oifs);
    fprintf(fp, "Outgoing oifs: %-20s\n", oifs);
    fprintf(fp, "Incoming     : %-20s\n", incoming_iif);

    fprintf(fp, "\nTIMERS:  Entry    JP    RS  Assert VIFS:");
    for (vifi = 0; vifi < numvifs; vifi++)
	fprintf(fp, "  %d", vifi);
    fprintf(fp, "\n         %5d  %4d  %4d  %6d      ",
	    r->timer, r->jp_timer, r->rs_timer, r->assert_timer);
    for (vifi = 0; vifi < numvifs; vifi++)
	fprintf(fp, " %2d", r->vif_timers[vifi]);
    fprintf(fp, "\n");
}

void dump_pim_mrt(FILE *fp)
{
    grpentry_t *g;
    mrtentry_t *r;
    u_int number_of_cache_mirrors = 0;
    u_int number_of_groups = 0;
    cand_rp_t *rp;
    kernel_cache_t *kc;

    fprintf(fp, "Multicast Routing Table ======================================================\n");

    /* TODO: remove the dummy 0.0.0.0 group (first in the chain) */
    for (g = grplist->next; g; g = g->next) {
	number_of_groups++;

	r = g->grp_route;
	if (r) {
	    if (r->flags & MRTF_KERNEL_CACHE) {
		for (kc = r->kernel_cache; kc; kc = kc->next)
		    number_of_cache_mirrors++;
	    }

	    /* Print the (*,G) routing info */
	    fprintf(fp, "----------------------------------- (*,G) ------------------------------------\n");
	    fprintf(fp, "Source           Group            RP Address       Flags\n");
	    fprintf(fp, "---------------  ---------------  ---------------  ---------------------------\n");
	    fprintf(fp, "%-15s  ", "INADDR_ANY");
	    fprintf(fp, "%-15s  ", inet_fmt(g->group, s1, sizeof(s1)));
	    fprintf(fp, "%-15s ", IN_PIM_SSM_RANGE(g->group) ? "SSM" :
		    (g->active_rp_grp ? inet_fmt(g->rpaddr, s2, sizeof(s2)) : "NULL"));

	    dump_route(fp, r);
	}

	/* Print all (S,G) routing info */
	fprintf(fp, "----------------------------------- (S,G) ------------------------------------\n");
	for (r = g->mrtlink; r; r = r->grpnext) {
	    if (r->flags & MRTF_KERNEL_CACHE)
		number_of_cache_mirrors++;

	    /* Print the routing info */
	    fprintf(fp, "Source           Group            RP Address       Flags\n");
	    fprintf(fp, "---------------  ---------------  ---------------  ---------------------------\n");
	    fprintf(fp, "%-15s  ", inet_fmt(r->source->address, s1, sizeof(s1)));
	    fprintf(fp, "%-15s  ", inet_fmt(g->group, s2, sizeof(s2)));
	    fprintf(fp, "%-15s ", IN_PIM_SSM_RANGE(g->group) ? "SSM" :
		    (g->active_rp_grp ? inet_fmt(g->rpaddr, s2, sizeof(s2)) : "NULL"));

	    dump_route(fp, r);
	}
    }/* for all groups */

    /* Print the (*,*,R) routing entries */
    fprintf(fp, "--------------------------------- (*,*,G) ------------------------------------\n");
    for (rp = cand_rp_list; rp; rp = rp->next) {
	r = rp->rpentry->mrtlink;
	if (r) {
	    if (r->flags & MRTF_KERNEL_CACHE) {
		for (kc = r->kernel_cache; kc; kc = kc->next)
		    number_of_cache_mirrors++;
	    }

	    /* Print the (*,*,RP) routing info */
	    fprintf(fp, "Source           Group            RP Address       Flags\n");
	    fprintf(fp, "---------------  ---------------  ---------------  ---------------------------\n");
	    fprintf(fp, "%-15s  ", inet_fmt(r->source->address, s1, sizeof(s1)));
	    fprintf(fp, "%-15s  ", "INADDR_ANY");
	    fprintf(fp, "%-15s ", "");

	    dump_route(fp, r);
	}
    } /* For all (*,*,RP) */

    fprintf(fp, "Number of Groups: %u\n", number_of_groups);
    fprintf(fp, "Number of Cache MIRRORs: %u\n", number_of_cache_mirrors);
    fprintf(fp, "------------------------------------------------------------------------------\n\n");
}

static void dump_rpgrp(FILE *fp, rp_grp_entry_t *rpgrp, int indent)
{
    grp_mask_t *grp = rpgrp->group;

    fprintf(fp, "                           %-18.18s  %-8u  %-8u\n",
	    netname(grp->group_addr, grp->group_mask),
	    rpgrp->priority, rpgrp->holdtime);
}

/*
 * Dumps the local Cand-RP-set
 */
int dump_rp_set(FILE *fp)
{
    cand_rp_t      *rp;
    rp_grp_entry_t *rpgrp;
    fprintf(fp, "Candidate Rendezvous-Point Set ===============================================\n");
    fprintf(fp, "RP address       Incoming  Group Prefix        Priority  Holdtime\n");
    fprintf(fp, "---------------  --------  ------------------  --------  ---------------------\n");
    for (rp = cand_rp_list; rp; rp = rp->next) {
		/*do not show 169.254.0.1*/
		if(strcmp("169.254.0.1", inet_fmt(rp->rpentry->address, s1, sizeof(s1))) == 0)
			continue;
		fprintf(fp, "%-15s  %-8d                                %-8u\n",
			inet_fmt(rp->rpentry->address, s1, sizeof(s1)),
			rp->rpentry->incoming, rp->rpentry->adv_holdtime);

	for (rpgrp = rp->rp_grp_next; rpgrp; rpgrp = rpgrp->rp_grp_next)
		dump_rpgrp(fp, rpgrp, 1);
    }
    fprintf(fp, "------------------------------------------------------------------------------\n");
	if(curr_bsr_address != INADDR_ANY_N){
    	fprintf(fp, "Current BSR address: %s holdtime:%d\n\n", inet_fmt(curr_bsr_address, s1, sizeof(s1)), curr_bsr_holdtime);
	}
    return TRUE;
}

/*add by whs for dump rp addr in a group*/
int dump_rp_hash(FILE *fp, char *s)
{
    char *w;
    cand_rp_t      *rp;
    rp_grp_entry_t *rpgrp;
    uint32_t group_addr = htonl(INADDR_UNSPEC_GROUP);	
	int rp_flag = FALSE;
	int rp_priority = 256;
	uint32_t rp_address = 0;
    /* next is group addr if exist */
	while (!EQUAL((w = glb_next_word(&s)), "")) {
	    if (EQUAL(w, "rp-hash")) {	
			w = glb_next_word(&s);
			group_addr = inet_parse(w, 4);
			if (!IN_MULTICAST(ntohl(group_addr))) {
			    fprintf(fp,"%s is not a valid multicast address", inet_fmt(group_addr, s1, sizeof(s1)));
			    return FALSE;
			}	
        }
	}
	
	/*add by zhw:bug:if c-rp have the same group&mask, the fisrt rp will show*/
    for (rp = cand_rp_list; rp; rp = rp->next) {

		for (rpgrp = rp->rp_grp_next; rpgrp; rpgrp = rpgrp->rp_grp_next)
			if((rpgrp->group->group_addr & rpgrp->group->group_mask) == (group_addr & rpgrp->group->group_mask)){
				/*get the true rp,priority first , if priority the same, big ip address is rp*/
				if(rpgrp->priority < rp_priority){
					rp_address = rp->rpentry->address;
					rp_priority = rpgrp->priority;
					rp_flag = TRUE;
				} else if((rp_priority == rpgrp->priority) && (rp->rpentry->address > rp_address)){
					rp_address = rp->rpentry->address;
					rp_priority = rpgrp->priority;
					rp_flag = TRUE;
				}
			}
			
    }
	
	if(rp_flag){
		fprintf(fp, "Current RP address: %s, priority:%d\n\n ",
					inet_fmt(rp_address, s1, sizeof(s1)), rp_priority);
	} else
		fprintf(fp, "unknown group address\n ");
	return TRUE;
	
}

void get_format_time(time_t uptime, char *f_uptime)
{
	int sec=0, min=0, hour=0;
	
	sec = uptime%60;
	uptime/=60;/*left min*/

	min = uptime%60;
	uptime/=60; /*left hour*/

	hour = uptime;
	if(hour > 99){
		snprintf(f_uptime, 10, "%s", "out range");
		return;
	}
	snprintf(f_uptime, 10, "%02d:%02d:%02d", hour, min, sec);
}

int dump_igmp_groups_one(FILE *fp, uint32_t group_addr)
{
	vifi_t vifi;
	struct uvif *v;
	struct listaddr *g;
	struct listaddr *source;
	time_t now_time,uptime;
	now_time = time(NULL);
	char f_uptime[10];
	
	fprintf(fp,"IGMP Connected Group Membership\n");
	fprintf(fp,"-----------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"Group Address    Interface   Uptime      Expires     Last Reporter    Version  Mode   Source Lists\n");
	fprintf(fp,"---------------  ----------  ----------  ----------  ---------------  -------  -----  ---------------\n");
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER))
	    	continue;
	   for (g = v->uv_groups; g != NULL; g = g->al_next){	
			if (group_addr ==  g->al_addr  || ((!group_addr) && g->al_addr)){
				fprintf(fp,"%-15s  ", inet_fmt(g->al_addr, s1, sizeof(s1)));
				fprintf(fp,"%-10s  ", v->uv_name);
			
				uptime = now_time - g->al_ctime;
				get_format_time(uptime, f_uptime);
				fprintf(fp,"%-10s  ", f_uptime);

				get_format_time(g->al_group_timeout, f_uptime);
				fprintf(fp,"%-10s  ", f_uptime);
				
				fprintf(fp,"%-15s  ", inet_fmt(g->al_alu.alu_reporter, s1, sizeof(s1)));
				fprintf(fp,"V%-6d  ", g->al_pv);
				if(!g->al_sources){
					fprintf(fp,"%-5s  ", "ASM");
					fprintf(fp,"%-15s\n", "NULL");
				}
				else{
					fprintf(fp,"%-5s  ", "SSM/I");
					source = g->al_sources;
					fprintf(fp,"%-15s\n",inet_fmt(source->al_addr, s1, sizeof(s1)));
					source = source->al_next;
					for (; source != NULL; source = source->al_next)
						fprintf(fp,"                                                                                      %-15s\n",inet_fmt(source->al_addr, s1, sizeof(s1)));
				}
				igmp_groups_num++;
			}
	   }
   }
    return TRUE;
}

/*add by zhw for dump igmp group address info*/
int dump_igmp_groups(FILE *fp, char *s)
{
    char *w;
	uint32_t group_addr = htonl(INADDR_UNSPEC_GROUP);	
	
    /* next is group addr if exist */
	while (!EQUAL((w = glb_next_word(&s)), "")) {
	    if (EQUAL(w, "groups")) {	
			/*w is multicast group address*/
			if(EQUAL((w = glb_next_word(&s)), "")){
				group_addr = 0;
				dump_igmp_groups_one(fp, group_addr);
				fprintf(fp, "IGMP groups num total:%d\n", igmp_groups_num);
				igmp_groups_num = 0;
				return TRUE;
			}
			group_addr = inet_parse(w, 4);
			if (!IN_MULTICAST(ntohl(group_addr))) {
			    fprintf(fp,"%s is not a valid multicast address", inet_fmt(group_addr, s1, sizeof(s1)));
			    return FALSE;
			}
		}	
   }
	
	dump_igmp_groups_one(fp,group_addr);
	fprintf(fp, "IGMP groups num total:%d\n", igmp_groups_num);
	igmp_groups_num = 0;
	return TRUE;
}

void dump_igmp_interface_one(FILE *fp, char *if_name)
{
	vifi_t vifi;
	struct uvif *v;
	char *version = NULL;
	char *p_stat = NULL;
	char *a_stat = NULL;
	
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER))
	    	continue;
		if(0 == strcmp(v->uv_name, if_name)) {
			p_stat = "up";
				
			a_stat = "line protocol is up";
			fprintf(fp,"  Interface:%s (Index %d) is %s, %s\n", v->uv_name, vifi, p_stat, a_stat);
			fprintf(fp,"    Internet address is %s\n", inet_fmt(v->uv_lcl_addr, s1, sizeof(s1)));
					
			if(v->uv_flags & VIFF_QUERIER){
				fprintf(fp,"    Querier address is %s(this system)\n", inet_fmt(v->uv_lcl_addr, s1, sizeof(s1)));
			}
			else 
				fprintf(fp,"    Querier address is %s\n", inet_fmt(v->uv_querier->al_addr, s1, sizeof(s1)));
					
			if(v->uv_flags & VIFF_IGMPV1)
				version = "Version 1";
			else if(v->uv_flags & VIFF_IGMPV2)
				version = "Version 2(Default)";
			else
				version = "Version 3";
			
			fprintf(fp,"    IGMP interface version is:%s\n", version);

			if(v->multicast == 1)
				fprintf(fp,"    Multicast on is enabled on this interface!\n");
			else 
				fprintf(fp,"    Multicast on is disabled on this interface!\n");

			if(v->igmp == 1)
				fprintf(fp,"    IGMP on is enabled on this interface!\n");
			else 
				fprintf(fp,"    IGMP on is disabled on this interface!\n");

			fprintf(fp,"    IGMP interface active groups number is %d\n", v->active_groups);
			fprintf(fp,"    IGMP interface limit groups number is %d(default 8192)\n", v->limit_igmp);
					
			fprintf(fp, "\n");	
			return;
		}		
   }
	return;
}

void dump_igmp_interface(FILE *fp, char *s)
{
	char *w;		
	char *if_name;
	if_name = (char *)calloc(1, 10);
	
	while (!EQUAL((w = glb_next_word(&s)), "")) {
	    if (EQUAL(w, "interface")) {	
			if(EQUAL((w = glb_next_word(&s)), "")){
				//dump_igmp_interface_one(fp, if_name);
				free(if_name);
				return ;
			}
			else 
				strcpy(if_name, w);
		}	
   }
	dump_igmp_interface_one(fp,if_name);
	free(if_name);
	return TRUE;
}

void dump_igmp_global_config(FILE *fp)
{
	fprintf(fp,"  IGMP global config\n");
	fprintf(fp,"    IGMP querier timeout is %d seconds\n", igmp_querier_timeout);
	fprintf(fp,"    IGMP query interval is %d seconds\n", igmp_query_interval);
	fprintf(fp,"    IGMP max query response time is %d seconds\n", IGMP_QUERY_RESPONSE_INTERVAL);
	fprintf(fp,"    IGMP last member query interval is %d seconds\n", igmp_last_member_query_interval);
	fprintf(fp,"    IGMP last member query count is %d\n", igmp_last_member_query_count);
	fprintf(fp,"    Robustness variable is %d\n", igmp_robustness_variable);
	fprintf(fp,"    IGMP global active groups number is %d\n", igmp_global_active_number);
	fprintf(fp,"    IGMP global limit groups number is %d\n", igmp_global_limit_number);
}
void dump_pim_interface_one(FILE *fp, char *if_name)
{
	vifi_t vifi;
    struct uvif *v;
    int nei_count = 0;
	struct pim_nbr_entry *nbr;
	
	fprintf(fp,"PIM Interface Info\n");
	fprintf(fp,"-------------------------------------------------------------------------\n");
	fprintf(fp,"Address          Interface      Nei count   hello interval  DR Prior  DR \n");
	fprintf(fp,"---------------  -------------  ----------  --------------  --------  ---\n");
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER))
	    	continue;
		if(strncmp(v->uv_name, "vedefault", 9) == 0)
			continue;
		if((if_name[0] == 0) || (0 == strcmp(v->uv_name, if_name))){
		fprintf(fp, "%-15s  ", inet_fmt(v->uv_lcl_addr, s1, sizeof(s1)));
	    fprintf(fp, "%-13s  ", v->uv_name);
	
		for(nbr = v->uv_pim_neighbors; nbr; nbr=nbr->next)
			nei_count++;
		fprintf(fp, "%-10d  ", nei_count);
		
		
		fprintf(fp,"%-14d  ", PIM_TIMER_HELLO_INTERVAL);
		fprintf(fp, "%-8d  ", v->uv_dr_prio);
		if(v->uv_flags & VIFF_DR)
			fprintf(fp, "%-6s  ", "DR");
		else 
			fprintf(fp, "%-6S", "NON-DR");
		
		fprintf(fp, "\n");
		}
	  }
}

void dump_pim_interface(FILE *fp, char *s)
{
	char *w;		
	char *if_name;
	if_name = (char *)calloc(1, 10);
	
	while (!EQUAL((w = glb_next_word(&s)), "")) {
	    if (EQUAL(w, "interface")) {	
			if(EQUAL((w = glb_next_word(&s)), "")){
				dump_pim_interface_one(fp, if_name);
				free(if_name);
				return TRUE;
			}
			else 
				strcpy(if_name, w);
		}	
   }
	dump_pim_interface_one(fp,if_name);
	free(if_name);
	return TRUE;
}

void dump_flags(FILE *fp, mrtentry_t *r)
{
	fprintf(fp, "    flags:");
		if (r->flags & MRTF_SPT)	        fprintf(fp, " SPT");
    	if (r->flags & MRTF_WC)	       		fprintf(fp, " WC");
    	if (r->flags & MRTF_RP)	            fprintf(fp, " RP");
    	if (r->flags & MRTF_REGISTER)       fprintf(fp, " REG");
   		if (r->flags & MRTF_IIF_REGISTER)   fprintf(fp, " IIF_REG");
    	if (r->flags & MRTF_NULL_OIF)       fprintf(fp, " NULL_OIF");
    	if (r->flags & MRTF_KERNEL_CACHE)   fprintf(fp, " CACHE");
    	if (r->flags & MRTF_ASSERTED)       fprintf(fp, " ASSERTED");
    	if (r->flags & MRTF_REG_SUPP)       fprintf(fp, " REG_SUPP");
    	if (r->flags & MRTF_SG)	            fprintf(fp, " SG");
    	if (r->flags & MRTF_PMBR)	        fprintf(fp, " PMBR");
		fprintf(fp, "\n");
}

void dump_pim_mrt_new(FILE *fp)
{
	grpentry_t *g;
    mrtentry_t *r;
    u_int number_of_cache_mirrors = 0;
    u_int number_of_groups = 0;
    cand_rp_t *rp;
    kernel_cache_t *kc;
	vifi_t vifi;
	struct uvif *v;
	time_t now_time,uptime;
	now_time = time(NULL);
	char f_uptime[10];
	char f_expire[10];
	int sg_flag = 0;
	
	fprintf(fp, "IP Multicast Routing Table\n");
    /* TODO: remove the dummy 0.0.0.0 group (first in the chain) */
    for (g = grplist->next; g; g = g->next) {
		number_of_groups++;

		r = g->grp_route;
	if (r) {
	    if (r->flags & MRTF_KERNEL_CACHE) {
		for (kc = r->kernel_cache; kc; kc = kc->next)
		    number_of_cache_mirrors++;
	    }
		/* Print the (*,G) routing info */
		
		uptime = now_time - r->c_time;
		get_format_time(uptime, f_uptime);
		get_format_time(r->timer, f_expire);
		
		fprintf(fp, "  (*, %s), %s/%s, RP %s\n", inet_fmt(g->group, s1, sizeof(s1)),
						f_uptime, f_expire,
			 IN_PIM_SSM_RANGE(g->group) ? "SSM" :(g->active_rp_grp ? inet_fmt(g->rpaddr, s2, sizeof(s2)) : "NULL"));
			
		dump_flags(fp, r);
		
		fprintf(fp, "  Incoming interface:%s, RPF nbr %s\n", 
			(r->incoming != 32 ? uvifs[r->incoming].uv_name : "NULL"),
			r->upstream ? inet_fmt(r->upstream->address, s1, sizeof(s1)): "NULL");

		fprintf(fp, "  Outgoing interface list:\n");
    	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	    	if(VIFM_ISSET(vifi, r->oifs)){
				uptime = now_time - r->oifs_uptime[vifi];
				get_format_time(uptime, f_uptime);
				get_format_time(r->oifs_expire[vifi], f_expire);
				fprintf(fp, "    %s Forward/Sparse %s/%s\n", v->uv_name, f_uptime, f_expire);
			}
    	}
	}
	
	/* Print all (S,G) routing info */ 
	for (r = g->mrtlink; r; r = r->grpnext) {
	    if (r->flags & MRTF_KERNEL_CACHE)
			number_of_cache_mirrors++;
		
		sg_flag = 1;
		uptime = now_time - r->c_time;
		get_format_time(uptime, f_uptime);
		get_format_time(r->timer, f_expire);
		fprintf(fp, "\n");
		fprintf(fp, "  (%s, %s), %s/%s, RP %s\n",inet_fmt(r->source->address, s1, sizeof(s1)), inet_fmt(g->group, s2, sizeof(s2)), 
				f_uptime, f_expire,
				IN_PIM_SSM_RANGE(g->group) ? "SSM" :(g->active_rp_grp ? inet_fmt(g->rpaddr, s3, sizeof(s3)) : "NULL"));
		
		dump_flags(fp, r);

		fprintf(fp, "  Incoming interface:%s, RPF nbr %s\n", 
			(r->incoming != 32 ? uvifs[r->incoming].uv_name : "NULL"),
			r->upstream ? inet_fmt(r->upstream->address, s1, sizeof(s1)) : "NULL");
		
		fprintf(fp, "  Outgoing interface list:\n");
    	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	    	if(VIFM_ISSET(vifi, r->oifs)){
				if(strcmp(v->uv_name, "register_vif0") == 0)
					continue;
				uptime = now_time - r->oifs_uptime[vifi];
				get_format_time(uptime, f_uptime);
				get_format_time(r->oifs_expire[vifi], f_expire);
				fprintf(fp, "    %s Forward/Sparse %s/%s\n", v->uv_name, f_uptime, f_expire);
			}
    	}
	 fprintf(fp, "\n");
     }
	 if(!sg_flag)
	 	fprintf(fp, "\n");
	 sg_flag = 0;
   }
/*
    // Print the (*,*,R) routing entries 
    fprintf(fp, "--------------------------------- (*,*,G) ------------------------------------\n");
    for (rp = cand_rp_list; rp; rp = rp->next) {
	r = rp->rpentry->mrtlink;
	if (r) {
	    if (r->flags & MRTF_KERNEL_CACHE) {
		for (kc = r->kernel_cache; kc; kc = kc->next)
		    number_of_cache_mirrors++;
	    }

	    // Print the (*,*,RP) routing info 
	    fprintf(fp, "Source           Group            RP Address       Flags\n");
	    fprintf(fp, "---------------  ---------------  ---------------  ---------------------------\n");
	    fprintf(fp, "%-15s  ", inet_fmt(r->source->address, s1, sizeof(s1)));
	    fprintf(fp, "%-15s  ", "INADDR_ANY");
	    fprintf(fp, "%-15s ", "");

	    dump_route(fp, r);
	}
    } //For all (*,*,RP)
	*/
	
    fprintf(fp, "\nNumber of Groups: %u\n", number_of_groups);
    fprintf(fp, "Number of Cache MIRRORs: %u\n", number_of_cache_mirrors);
    fprintf(fp, "------------------------------------------------------------------------------\n\n");
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */

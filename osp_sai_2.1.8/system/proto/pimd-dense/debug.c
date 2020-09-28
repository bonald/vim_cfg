/*
 *  Copyright (c) 1998 by the University of Southern California.
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation in source and binary forms for lawful
 *  purposes and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and that both
 *  the copyright notice and this permission notice appear in supporting
 *  documentation, and that any documentation, advertising materials,
 *  and other materials related to such distribution and use acknowledge
 *  that the software was developed by the University of Southern
 *  California and/or Information Sciences Institute.
 *  The name of the University of Southern California may not
 *  be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THE UNIVERSITY OF SOUTHERN CALIFORNIA DOES NOT MAKE ANY REPRESENTATIONS
 *  ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  THIS SOFTWARE IS
 *  PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND 
 *  NON-INFRINGEMENT.
 *
 *  IN NO EVENT SHALL USC, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
 *  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT,
 *  TORT, OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH,
 *  THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  Other copyrights might apply to parts of this software and are so
 *  noted when applicable.
 */
/*
 *  Questions concerning this software should be directed to 
 *  Pavlin Ivanov Radoslavov (pavlin@catarina.usc.edu)
 *
 *  $Id: debug.c,v 1.9 1998/06/02 19:46:54 kurtw Exp $
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

#include "defs.h"


#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern int haveterminal;
extern char *progname;

int log_nmsgs = 0;
unsigned long debug = 0x00000000;        /* If (long) is smaller than
					  * 4 bytes, then we are in
					  * trouble.
					  */
static char dumpfilename[] = _PATH_PIMD_DUMP;
static char cachefilename[] = _PATH_PIMD_CACHE; /* TODO: notused */


char *
packet_kind(proto, type, code)
    u_int proto, type, code;
{
    static char unknown[60];

    switch (proto) {
    case IPPROTO_IGMP:
	switch (type) {
	case IGMP_MEMBERSHIP_QUERY:    return "IGMP Membership Query    ";
	case IGMP_V1_MEMBERSHIP_REPORT:return "IGMP v1 Member Report    ";
	case IGMP_V2_MEMBERSHIP_REPORT:return "IGMP v2 Member Report    ";
	case IGMP_V2_LEAVE_GROUP:      return "IGMP Leave message       ";
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
		sprintf(unknown,   "UNKNOWN DVMRP message code = %3d ", code);
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
	    case PIM_V1_QUERY:	       return "PIM v1 Router-Query      ";
	    case PIM_V1_REGISTER:      return "PIM v1 Register          ";
	    case PIM_V1_REGISTER_STOP: return "PIM v1 Register-Stop     ";
	    case PIM_V1_JOIN_PRUNE:    return "PIM v1 Join/Prune        ";
	    case PIM_V1_RP_REACHABILITY:
		                       return "PIM v1 RP-Reachability   ";
	    case PIM_V1_ASSERT:        return "PIM v1 Assert            ";
	    case PIM_V1_GRAFT:         return "PIM v1 Graft             ";
	    case PIM_V1_GRAFT_ACK:     return "PIM v1 Graft_Ack         ";
	    default:
		sprintf(unknown,   "UNKNOWN PIM v1 message type =%3d ", code);
		return unknown;
	    }
	case IGMP_MTRACE:              return "IGMP trace query         ";
	case IGMP_MTRACE_RESP:         return "IGMP trace reply         ";
	default:
	    sprintf(unknown,
		    "UNKNOWN IGMP message: type = 0x%02x, code = 0x%02x ",
		    type, code);
	    return unknown;
	}
    case IPPROTO_PIM:    /* PIM v2 */
	switch (type) {
	case PIM_V2_HELLO:	       return "PIM v2 Hello             ";
	case PIM_V2_REGISTER:          return "PIM v2 Register          ";
	case PIM_V2_REGISTER_STOP:     return "PIM v2 Register_Stop     ";
	case PIM_V2_JOIN_PRUNE:        return "PIM v2 Join/Prune        ";
	case PIM_V2_BOOTSTRAP:         return "PIM v2 Bootstrap         ";
	case PIM_V2_ASSERT:            return "PIM v2 Assert            ";
	case PIM_V2_GRAFT:             return "PIM-DM v2 Graft          ";
	case PIM_V2_GRAFT_ACK:         return "PIM-DM v2 Graft_Ack      ";
	case PIM_V2_CAND_RP_ADV:       return "PIM v2 Cand. RP Adv.     ";
    case PIM_V2_STATE_REFRESH:     return "PIM v2 State Refresh.    ";
	default:
	    sprintf(unknown,      "UNKNOWN PIM v2 message type =%3d ", type);
	    return unknown;
	}
    default:
	sprintf(unknown,          "UNKNOWN proto =%3d               ", proto);
	return unknown;
    }
}


/*
 * Used for debugging particular type of messages.
 */
int
debug_kind(proto, type, code)
    u_int proto, type, code;
{
    switch (proto) {
    case IPPROTO_IGMP:
	switch (type) {
	case IGMP_MEMBERSHIP_QUERY:        return DEBUG_IGMP;
	case IGMP_V1_MEMBERSHIP_REPORT:    return DEBUG_IGMP;
	case IGMP_V2_MEMBERSHIP_REPORT:    return DEBUG_IGMP;
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
    case IPPROTO_PIM: 	/* PIM v2 */
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
log_level(proto, type, code)
    u_int proto, type, code;
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
	case IGMP_PIM:
	    /* PIM v1 */
	    switch (code) {
	    default:
		return LOG_INFO;
	    }
	default:
	    return LOG_WARNING;
	}
	
    case IPPROTO_PIM:
	/* PIM v2 */
	switch (type) {
	default:
	    return LOG_INFO;
	}
    default:
	return LOG_WARNING;
    }
    return LOG_WARNING;
}


/*
 * Dump internal data structures to stderr.
 */
/* TODO: currently not used
void 
dump(int i)
{
    dump_vifs(stderr);
    dump_pim_mrt(stderr);
}	
*/

/*
 * Dump internal data structures to a file.
 */
void 
fdump(i)
    int i;
{
    FILE *fp;
    fp = fopen(dumpfilename, "w");
    if (fp != NULL) {
	dump_vifs_new(fp);
	dump_pim_mrt_new(fp);
	(void) fclose(fp);
    }
}

/* TODO: dummy, to be used in the future. */
/*
 * Dump local cache contents to a file.
 */
void
cdump(i)
    int i;
{
    FILE *fp;
    
    fp = fopen(cachefilename, "w");
    if (fp != NULL) {
      /* TODO: implement it:
	 dump_cache(fp); 
	 */
	(void) fclose(fp);
    }
}

void
dump_vifs(fp)
    FILE *fp;
{	
    vifi_t vifi;
    register struct uvif *v;
    pim_nbr_entry_t *n;
    int width;
    int i;
    
    fprintf(fp, "\nVirtual Interface Table\n %-4s %-17s %-16s %-8s %-14s %s",
	    "Vif", "Local-Address", "Subnet", "Thresh", "Flags",
	    "Neighbors\n");
    
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	fprintf(fp, "  %-3u %-17s ", vifi, inet_fmt(v->uv_lcl_addr, s1));
	if (v->uv_flags & VIFF_REGISTER)
	    fprintf(fp, "%-16s ", v->uv_name);
	else
	    fprintf(fp,"%-16.16s ", netname(v->uv_subnet, v->uv_subnetmask));
	fprintf(fp, "%-5u   ", v->uv_threshold);
	width = 0;
	if (v->uv_flags & VIFF_DISABLED)
	    fprintf(fp, " DISABLED");
	if (v->uv_flags & VIFF_DOWN)
	    fprintf(fp, " DOWN");
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
	    fprintf(fp, " %-12s", "NO-NBR");
	    width += 6;
	}

	if ((n = v->uv_pim_neighbors) != NULL) {
	    /* Print the first neighbor on the same line */
	    for (i = width; i <= 15; i++)
		fprintf(fp, " ");
	    fprintf(fp, "%-12s\n", inet_fmt(n->address, s1));
	    for (n = n->next; n != NULL; n = n->next)
		fprintf(fp, "%64s %-15s\n", "", inet_fmt(n->address, s1));
	    
	}
	else
	    fprintf(fp, "\n");
    }  
    fprintf(fp, "\n");
}


/*
 * Log errors and other messages to the system log daemon and to stderr,
 * according to the severity of the message and the current debug level.
 * For errors of severity LOG_ERR or worse, terminate the program.
 */
#ifdef __STDC__
void
log(int severity, int syserr, char *format, ...)
{
    va_list ap;
    static char fmt[211] = "warning - ";
    char *msg;
    struct timeval now;
    struct tm *thyme;
    
    va_start(ap, format);
#else
/*VARARGS3*/
void
log(severity, syserr, format, va_alist)
    int severity, syserr;
    char *format;
    va_dcl
{
    va_list ap;
    static char fmt[311] = "warning - ";
    char *msg;
    char tbuf[20];
    struct timeval now;
    struct tm *thyme;
    
    va_start(ap);
#endif
    vsprintf(&fmt[10], format, ap);
    va_end(ap);
    msg = (severity == LOG_WARNING) ? fmt : &fmt[10];
    
    /*
     * Log to stderr if we haven't forked yet and it's a warning or worse,
     * or if we're debugging.
     */
    if (haveterminal && (debug || severity <= LOG_WARNING)) {
	gettimeofday(&now,NULL);
	thyme = localtime(&now.tv_sec);
	if (!debug)
	    fprintf(stderr, "%s: ", progname);
	fprintf(stderr, "%02d:%02d:%02d.%03ld %s", thyme->tm_hour,
		thyme->tm_min, thyme->tm_sec, now.tv_usec / 1000, msg);
	if (syserr == 0)
	    fprintf(stderr, "\n");
	else if (syserr < sys_nerr)
	    fprintf(stderr, ": %s\n", sys_errlist[syserr]);
	else
	    fprintf(stderr, ": errno %d\n", syserr);
    }
    
    /*
     * Always log things that are worse than warnings, no matter what
     * the log_nmsgs rate limiter says.
     * Only count things worse than debugging in the rate limiter
     * (since if you put daemon.debug in syslog.conf you probably
     * actually want to log the debugging messages so they shouldn't
     * be rate-limited)
     */
    if ((severity < LOG_WARNING) || (log_nmsgs < LOG_MAX_MSGS)) {
	if (severity < LOG_DEBUG)
	    log_nmsgs++;
	if (syserr != 0) {
	    errno = syserr;
	    syslog(severity, "%s: %m", msg);
	} else
	    syslog(severity, "%s", msg);
    }
    
    if (severity <= LOG_ERR) exit(-1);
}

/* TODO: format the output for better readability */
void 
dump_pim_mrt(fp)
    FILE *fp;
{
    grpentry_t *g;
    register mrtentry_t *r;
    register vifi_t vifi;
    u_int number_of_groups = 0;
    char oifs[(sizeof(vifbitmap_t)<<3)+1];
    char pruned_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char leaves_oifs[(sizeof(vifbitmap_t)<<3)+1];
    char incoming_iif[(sizeof(vifbitmap_t)<<3)+1];
    
    fprintf(fp, "Multicast Routing Table\n%s", 
	    " Source          Group           Flags\n");

    /* TODO: remove the dummy 0.0.0.0 group (first in the chain) */
    for (g = grplist->next; g != (grpentry_t *)NULL; g = g->next) {
	number_of_groups++;
	/* Print all (S,G) routing info */
	for (r = g->mrtlink; r != (mrtentry_t *)NULL; r = r->grpnext) {
	    fprintf(fp, "---------------------------(S,G)----------------------------\n");
	    /* Print the routing info */
	    fprintf(fp, " %-15s", inet_fmt(r->source->address, s1));
	    fprintf(fp, " %-15s", inet_fmt(g->group, s2));

	    for (vifi = 0; vifi < numvifs; vifi++) {
		oifs[vifi] =
		    VIFM_ISSET(vifi, r->oifs)          ? 'o' : '.';
		pruned_oifs[vifi] =
		    VIFM_ISSET(vifi, r->pruned_oifs)   ? 'p' : '.';
		leaves_oifs[vifi] =
		    VIFM_ISSET(vifi, r->leaves)        ? 'l' : '.';
		incoming_iif[vifi] = '.';
	    }
	    oifs[vifi]          = 0x0;  /* End of string */
	    pruned_oifs[vifi]   = 0x0;
	    leaves_oifs[vifi]   = 0x0;
	    incoming_iif[vifi]  = 0x0;
	    incoming_iif[r->incoming] = 'I';

	    /* TODO: don't need some of the flags */
	    if (r->flags & MRTF_SPT)           fprintf(fp, " SPT");
	    if (r->flags & MRTF_WC)            fprintf(fp, " WC");
	    if (r->flags & MRTF_RP)            fprintf(fp, " RP");
	    if (r->flags & MRTF_REGISTER)      fprintf(fp, " REG");
	    if (r->flags & MRTF_IIF_REGISTER)  fprintf(fp, " IIF_REG");
	    if (r->flags & MRTF_NULL_OIF)      fprintf(fp, " NULL_OIF");
	    if (r->flags & MRTF_KERNEL_CACHE)  fprintf(fp, " CACHE");
	    if (r->flags & MRTF_ASSERTED)      fprintf(fp, " ASSERTED");
	    if (r->flags & MRTF_REG_SUPP)      fprintf(fp, " REG_SUPP");
	    if (r->flags & MRTF_SG)            fprintf(fp, " SG");
	    if (r->flags & MRTF_PMBR)          fprintf(fp, " PMBR");
	    fprintf(fp, "\n");
	    
	    fprintf(fp, "Pruned   oifs: %-20s\n", pruned_oifs);
	    fprintf(fp, "Leaves   oifs: %-20s\n", leaves_oifs);
	    fprintf(fp, "Outgoing oifs: %-20s\n", oifs);
	    fprintf(fp, "Incoming     : %-20s\n", incoming_iif);

	    fprintf(fp, "Upstream nbr: %s\n", 
		    r->upstream ? inet_fmt(r->upstream->address, s1) : "NONE");
	    fprintf(fp, "\nTIMERS:  Entry   Prune VIFS:");
	    for (vifi = 0; vifi < numvifs; vifi++)
		fprintf(fp, "  %2d", vifi);
	    fprintf(fp, "\n           %3d              ",
		    r->timer);
	    for (vifi = 0; vifi < numvifs; vifi++)
		fprintf(fp, " %3d", r->prune_timers[vifi]);
	    fprintf(fp, "\n");
	}
    }/* for all groups */

    fprintf(fp, "Number of Groups: %u\n", number_of_groups);
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


/*add by zhw to show mrt*/
void dump_pim_mrt_new(FILE *fp)
{
	grpentry_t *g;
	mrtentry_t *r;
	u_int number_of_groups = 0;
	vifi_t vifi;
	struct uvif *v;
	
	time_t now_time,uptime;
	now_time = time(NULL);
	char f_uptime[10];
	char f_expire[10];
		
	fprintf(fp, "IP Multicast Routing Table\n");
	/* TODO: remove the dummy 0.0.0.0 group (first in the chain) */
	for (g = grplist->next; g; g = g->next) {
		number_of_groups++;
	
		/* Print all (S,G) routing info */ 
		for (r = g->mrtlink; r; r = r->grpnext) {
			uptime = now_time - r->c_time;
			get_format_time(uptime, f_uptime);
			get_format_time(r->timer, f_expire);
			fprintf(fp, "\n");
			fprintf(fp, "  (%s, %s), %s/%s\n",inet_fmt(r->source->address, s1), inet_fmt(g->group, s2), 
					f_uptime, f_expire);
			dump_flags(fp, r);
	
			fprintf(fp, "  Incoming interface:%s, RPF nbr %s\n", 
				(r->incoming != 32 ? uvifs[r->incoming].uv_name : "NULL"),
				r->upstream ? inet_fmt(r->upstream->address, s1) : "NULL");
			
			fprintf(fp, "  Outgoing interface list:\n");
			for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
				if(VIFM_ISSET(vifi, r->oifs)){
					fprintf(fp, "	 %s Forward/Dense uptime/expires\n", v->uv_name);
				}
			}
		 fprintf(fp, "\n");
		 }
	   }
	
	fprintf(fp, "\nNumber of Groups: %u\n", number_of_groups);
	fprintf(fp, "------------------------------------------------------------------------------\n\n");
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
		
	fprintf(fp, "PIM Dense-Mode Neighbor Table\n");
	fprintf(fp, "-----------------------------------------------------------------------------\n");
	fprintf(fp, "Interface       Neighbors        Thresh  Uptime      Expires     Prio  DR	 \n");
	fprintf(fp, "-------------	---------------  ------  ----------  ----------  ----  -------\n");
	
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
		if(!v->uv_pim_neighbors)
			continue;
		  
		/*VIFF_REGISTER name is register_vif0*/
		for(nbr = v->uv_pim_neighbors; nbr; nbr=nbr->next){
			
			fprintf(fp, "%-13s  ", v->uv_name);
			fprintf(fp, "%-15s  ", inet_fmt(nbr->address, s1));
			fprintf(fp, "%-6u  ", v->uv_threshold);
			
			time(&now_time);
			up_time = now_time - nbr->c_time;
			get_format_time(up_time, f_uptime);
			fprintf(fp,"%-10s  ", f_uptime);
	
			get_format_time(nbr->timer, f_uptime);
			fprintf(fp,"%-10s  ", f_uptime);
	
			//if(nbr->dr_prio_present)
				fprintf(fp,"%-4s  ", "UNKN");
			//else 
				//fprintf(fp,"%-4s  ", "NULL");
	
			if(v->uv_flags & VIFF_DR)
				fprintf(fp, "%-6s  ", "NON-DR");
			else 
				fprintf(fp, "%-6s  ", "DR");
			
			fprintf(fp, "\n");
		  }
		}
	}

int dump_igmp_groups_one(FILE *fp, u_int32 group_addr)
{
	vifi_t vifi;
	struct uvif *v;
	struct listaddr *g;
	grpentry_t *group;
	register mrtentry_t *r;
	struct listaddr *source;
	time_t now_time,uptime;
	now_time = time(NULL);
	char f_uptime[10];
	
	fprintf(fp,"IGMP Connected Group Membership\n");
	fprintf(fp,"-----------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"Group Address    Interface   Uptime      Expires     Last Reporter    Version  Mode   Source List\n");
	fprintf(fp,"---------------  ----------  ----------  ----------  ---------------  -------  -----  ---------------\n");
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {	
	   for (g = v->uv_groups; g != NULL; g = g->al_next){
	   		if(strcmp("224.0.0.2", inet_fmt(g->al_addr, s1))==0 || strcmp("224.0.0.13", inet_fmt(g->al_addr, s1)) == 0)
				continue;
			if (group_addr ==  g->al_addr  || ((!group_addr) && g->al_addr)){
				fprintf(fp,"%-15s  ", inet_fmt(g->al_addr, s1));
				fprintf(fp,"%-10s  ", v->uv_name);
			
				uptime = now_time - g->al_ctime;
				get_format_time(uptime, f_uptime);
				fprintf(fp,"%-10s  ", f_uptime);

				get_format_time(g->al_group_timeout, f_uptime);
				fprintf(fp,"%-10s  ", f_uptime);
				
				fprintf(fp,"%-15s  ", inet_fmt(g->al_alu.alu_reporter, s1));
				
				fprintf(fp,"V%-6d  ", g->al_pv);
				fprintf(fp,"%-5s  ", "SSM/I");
				
				for (group = grplist->next; group != (grpentry_t *)NULL; group = group->next) 
					if(group->group == g->al_addr){
						for (r = group->mrtlink; r != (mrtentry_t *)NULL; r = r->grpnext){
	              			 fprintf(fp, "%-15s\n", inet_fmt(r->source->address, s1));
							 break;
						}
						for(r;r != (mrtentry_t *) NULL; r = r->grpnext)
							fprintf(fp,"                                                                             %-15s\n",inet_fmt(r->source->address, s1));
					}
	   }
   }
    return TRUE;
}
}

/*add by zhw for dump igmp group address info*/
void dump_igmp_groups(FILE *fp, char *s)
{
	char *w;
	u_int32 group_addr = htonl(INADDR_UNSPEC_GROUP);	
		
	/* next is group addr if exist */
	while (!EQUAL((w = glb_next_word(&s)), "")) {
		if (EQUAL(w, "groups")) {	
			/*w is multicast group address*/
			if(EQUAL((w = glb_next_word(&s)), "")){
				group_addr = 0;
				dump_igmp_groups_one(fp, group_addr);
				return TRUE;
			}
			group_addr = inet_parse(w, 4);
			if (!IN_MULTICAST(ntohl(group_addr))) {
				fprintf(fp,"%s is not a valid multicast address", inet_fmt(group_addr, s1));
				return FALSE;
			}
		}	
	}
		
	dump_igmp_groups_one(fp,group_addr);
	return TRUE;
}



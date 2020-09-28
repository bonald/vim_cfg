/*
 *  Copyright (c) 1998 by the University of Southern California.
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation in source and binary forms for lawful
 *  non-commercial purposes and without fee is hereby granted, provided
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
 *  $Id: trace.c,v 1.3 1998/06/01 22:27:15 kurtw Exp $
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
#include "trace.h"


/* TODO */
/*
 * Traceroute function which returns traceroute replies to the requesting
 * router. Also forwards the request to downstream routers.
 */
void
accept_mtrace(src, dst, group, data, no, datalen)
    u_int32 src;
    u_int32 dst;
    u_int32 group;
    char *data;
    u_int no;   /* promoted u_char */
    int datalen;
{
    u_char type;
    mrtentry_t *mrt;
    struct tr_query *qry;
    struct tr_resp  *resp;
    int vifi;
    char *p;
    int rcount;
    int errcode = TR_NO_ERR;
    int resptype;
    struct timeval tp;
    struct sioc_vif_req v_req;
    u_int32 parent_address;

    /* Remember qid across invocations */
    static u_int32 oqid = 0;

    /* timestamp the request/response */
    gettimeofday(&tp, 0);

    /*
     * Check if it is a query or a response
     */
    if (datalen == QLEN) {
	type = QUERY;
	IF_DEBUG(DEBUG_TRACE)
	    log(LOG_DEBUG, 0, "Initial traceroute query rcvd from %s to %s",
		inet_fmt(src, s1), inet_fmt(dst, s2));
    }
    else if ((datalen - QLEN) % RLEN == 0) {
	type = RESP;
	IF_DEBUG(DEBUG_TRACE)
	    log(LOG_DEBUG, 0, "In-transit traceroute query rcvd from %s to %s",
		inet_fmt(src, s1), inet_fmt(dst, s2));
	if (IN_MULTICAST(ntohl(dst))) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Dropping multicast response");
	    return;
	}
    }
    else {
	log(LOG_WARNING, 0, "%s from %s to %s",
	    "Non decipherable traceroute request recieved",
	    inet_fmt(src, s1), inet_fmt(dst, s2));
	return;
    }

    qry = (struct tr_query *)data;

    /*
     * if it is a packet with all reports filled, drop it
     */
    if ((rcount = (datalen - QLEN)/RLEN) == no) {
	IF_DEBUG(DEBUG_TRACE)
	    log(LOG_DEBUG, 0, "packet with all reports filled in");
	return;
    }

    IF_DEBUG(DEBUG_TRACE) {
	log(LOG_DEBUG, 0, "s: %s g: %s d: %s ", inet_fmt(qry->tr_src, s1),
	    inet_fmt(group, s2), inet_fmt(qry->tr_dst, s3));
	log(LOG_DEBUG, 0, "rttl: %d rd: %s", qry->tr_rttl,
	    inet_fmt(qry->tr_raddr, s1));
	log(LOG_DEBUG, 0, "rcount:%d, qid:%06x", rcount, qry->tr_qid);
    }

    /* determine the routing table entry for this traceroute */
    mrt = find_route(qry->tr_src, group, MRTF_SG | MRTF_WC | MRTF_PMBR,
		     DONT_CREATE);
    IF_DEBUG(DEBUG_TRACE)
	if (mrt != (mrtentry_t *)NULL) {
	    if (mrt->upstream != (pim_nbr_entry_t *)NULL)
		parent_address = mrt->upstream->address;
	    else
		parent_address = INADDR_ANY;
	    log(LOG_DEBUG, 0, "mrt parent vif: %d rtr: %s metric: %d",
		mrt->incoming, inet_fmt(parent_address, s1), mrt->metric);
/* TODO
	    log(LOG_DEBUG, 0, "mrt origin %s",
		RT_FMT(rt, s1));
		*/
	} else
	    log(LOG_DEBUG, 0, "...no route");
    
    /*
     * Query type packet - check if rte exists 
     * Check if the query destination is a vif connected to me.
     * and if so, whether I should start response back
     */
    if (type == QUERY) {
	if (oqid == qry->tr_qid) {
	    /*
	     * If the multicast router is a member of the group being
	     * queried, and the query is multicasted, then the router can
	     * recieve multiple copies of the same query.  If we have already
	     * replied to this traceroute, just ignore it this time.
	     *
	     * This is not a total solution, but since if this fails you
	     * only get N copies, N <= the number of interfaces on the router,
	     * it is not fatal.
	     */
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "ignoring duplicate traceroute packet");
	    return;
	}

	if (mrt == (mrtentry_t *)NULL) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Mcast traceroute: no route entry %s",
		    inet_fmt(qry->tr_src, s1));
	    if (IN_MULTICAST(ntohl(dst)))
		return;
	}
	vifi = find_vif_direct(qry->tr_dst);
	
	if (vifi == NO_VIF) {
	    /* The traceroute destination is not on one of my subnet vifs. */
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Destination %s not an interface",
		    inet_fmt(qry->tr_dst, s1));
	    if (IN_MULTICAST(ntohl(dst)))
		return;
	    errcode = TR_WRONG_IF;
	} else if (mrt != (mrtentry_t *)NULL && !VIFM_ISSET(vifi, mrt->oifs)) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0,
		    "Destination %s not on forwarding tree for src %s",
		    inet_fmt(qry->tr_dst, s1), inet_fmt(qry->tr_src, s2));
	    if (IN_MULTICAST(ntohl(dst)))
		return;
	    errcode = TR_WRONG_IF;
	}
    }
    else {
	/*
	 * determine which interface the packet came in on
	 * RESP packets travel hop-by-hop so this either traversed
	 * a tunnel or came from a directly attached mrouter.
	 */
	if ((vifi = find_vif_direct(src)) == NO_VIF) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Wrong interface for packet");
	    errcode = TR_WRONG_IF;
	}
    }   
    
    /* Now that we've decided to send a response, save the qid */
    oqid = qry->tr_qid;

    IF_DEBUG(DEBUG_TRACE)
	log(LOG_DEBUG, 0, "Sending traceroute response");
    
    /* copy the packet to the sending buffer */
    p = igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN;
    
    bcopy(data, p, datalen);
    
    p += datalen;
    
    /*
     * If there is no room to insert our reply, coopt the previous hop
     * error indication to relay this fact.
     */
    if (p + sizeof(struct tr_resp) > igmp_send_buf + RECV_BUF_SIZE) {
	resp = (struct tr_resp *)p - 1;
	resp->tr_rflags = TR_NO_SPACE;
	mrt = NULL;
	goto sendit;
    }

    /*
     * fill in initial response fields
     */
    resp = (struct tr_resp *)p;
    bzero(resp, sizeof(struct tr_resp));
    datalen += RLEN;

    resp->tr_qarr    = htonl(((tp.tv_sec + JAN_1970) << 16) + 
			     ((tp.tv_usec << 10) / 15625));

    resp->tr_rproto  = PROTO_PIM;
    resp->tr_outaddr = (vifi == NO_VIF) ? dst : uvifs[vifi].uv_lcl_addr;
    resp->tr_fttl    = (vifi == NO_VIF) ? 0 : uvifs[vifi].uv_threshold;
    resp->tr_rflags  = errcode;

    /*
     * obtain # of packets out on interface
     */
    v_req.vifi = vifi;
    if (vifi != NO_VIF && ioctl(udp_socket, SIOCGETVIFCNT, (char *)&v_req) >= 0)
	resp->tr_vifout  =  htonl(v_req.ocount);
    else
	resp->tr_vifout  =  0xffffffff;

    /*
     * fill in scoping & pruning information
     */
/* TODO */
#if 0
    if (mrt != (mrtentry_t *)NULL)
	for (gt = rt->rt_groups; gt; gt = gt->gt_next) {
	    if (gt->gt_mcastgrp >= group)
		break;
	}
    else
	gt = NULL;

    if (gt && gt->gt_mcastgrp == group) {
	struct stable *st;

	for (st = gt->gt_srctbl; st; st = st->st_next)
	    if (qry->tr_src == st->st_origin)
		break;

	sg_req.src.s_addr = qry->tr_src;
	sg_req.grp.s_addr = group;
	if (st && st->st_ctime != 0 &&
	    ioctl(udp_socket, SIOCGETSGCNT, (char *)&sg_req) >= 0)
	    resp->tr_pktcnt = htonl(sg_req.pktcnt + st->st_savpkt);
	else
	    resp->tr_pktcnt = htonl(st ? st->st_savpkt : 0xffffffff);

	if (VIFM_ISSET(vifi, gt->gt_scope))
	    resp->tr_rflags = TR_SCOPED;
	else if (gt->gt_prsent_timer)
	    resp->tr_rflags = TR_PRUNED;
	else if (!VIFM_ISSET(vifi, gt->gt_grpmems))
	    if (VIFM_ISSET(vifi, rt->rt_children) &&
		NBRM_ISSETMASK(uvifs[vifi].uv_nbrmap, rt->rt_subordinates)) /*XXX*/
		resp->tr_rflags = TR_OPRUNED;
	    else
		resp->tr_rflags = TR_NO_FWD;
    } else {
	if (scoped_addr(vifi, group))
	    resp->tr_rflags = TR_SCOPED;
	else if (rt && !VIFM_ISSET(vifi, rt->rt_children))
	    resp->tr_rflags = TR_NO_FWD;
    }
#endif /* 0 */

    /*
     *  if no rte exists, set NO_RTE error
     */
    if (mrt == (mrtentry_t *)NULL) {
	src = dst;		/* the dst address of resp. pkt */
	resp->tr_inaddr   = 0;
	resp->tr_rflags   = TR_NO_RTE;
	resp->tr_rmtaddr  = 0;
    } else {
	/* get # of packets in on interface */
	v_req.vifi = mrt->incoming;
	if (ioctl(udp_socket, SIOCGETVIFCNT, (char *)&v_req) >= 0)
	    resp->tr_vifin = htonl(v_req.icount);
	else
	    resp->tr_vifin = 0xffffffff;

/* TODO
	MASK_TO_VAL(rt->rt_originmask, resp->tr_smask);
	*/
	src = uvifs[mrt->incoming].uv_lcl_addr;
	resp->tr_inaddr = src;
	if (mrt->upstream != (pim_nbr_entry_t *)NULL)
	    parent_address = mrt->upstream->address;
	else
	    parent_address = INADDR_ANY;

	resp->tr_rmtaddr = parent_address;
	if (!VIFM_ISSET(vifi, mrt->oifs)) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Destination %s not on forwarding tree for src %s",
		    inet_fmt(qry->tr_dst, s1), inet_fmt(qry->tr_src, s2));
	    resp->tr_rflags = TR_WRONG_IF;
	}
#if 0
	if (rt->rt_metric >= UNREACHABLE) {
	    resp->tr_rflags = TR_NO_RTE;
	    /* Hack to send reply directly */
	    rt = NULL;
	}
#endif /* 0 */
    }

sendit:
    /*
     * if metric is 1 or no. of reports is 1, send response to requestor
     * else send to upstream router.  If the upstream router can't handle
     * mtrace, set an error code and send to requestor anyway.
     */
    IF_DEBUG(DEBUG_TRACE)
	log(LOG_DEBUG, 0, "rcount:%d, no:%d", rcount, no);

    if ((rcount + 1 == no) || (mrt == NULL) || (mrt->metric == 1)) {
	resptype = IGMP_MTRACE_RESP;
	dst = qry->tr_raddr;
    } else
/* TODO */
    {
#if 0
	if (!can_mtrace(rt->rt_parent, rt->rt_gateway)) {
	    dst = qry->tr_raddr;
	    resp->tr_rflags = TR_OLD_ROUTER;
	    resptype = IGMP_MTRACE_RESP;
	} else {
#endif /* 0 */
	if (mrt->upstream != (pim_nbr_entry_t *)NULL)
	    parent_address = mrt->upstream->address;
	else
	    parent_address = INADDR_ANY;
	    dst = parent_address;
	    resptype = IGMP_MTRACE;
	}

    if (IN_MULTICAST(ntohl(dst))) {
	/*
	 * Send the reply on a known multicast capable vif.
	 * If we don't have one, we can't source any multicasts anyway.
	 */
	if (phys_vif != -1) {
	    IF_DEBUG(DEBUG_TRACE)
		log(LOG_DEBUG, 0, "Sending reply to %s from %s",
		    inet_fmt(dst, s1),
		    inet_fmt(uvifs[phys_vif].uv_lcl_addr, s2));
	    k_set_ttl(igmp_socket, qry->tr_rttl);
	    send_igmp(igmp_send_buf, uvifs[phys_vif].uv_lcl_addr, dst,
		      resptype, no, group, datalen);
	    k_set_ttl(igmp_socket, 1);
	} else
	    log(LOG_INFO, 0, "No enabled phyints -- %s",
		"dropping traceroute reply");
    } else {
	IF_DEBUG(DEBUG_TRACE)
	    log(LOG_DEBUG, 0, "Sending %s to %s from %s",
		resptype == IGMP_MTRACE_RESP ?  "reply" : "request on",
		inet_fmt(dst, s1), inet_fmt(src, s2));
	
	send_igmp(igmp_send_buf, src, dst, resptype, no, group, datalen);
    }
    return;
}


/*
 * accept_neighbor_request() supports some old DVMRP messages from mrinfo.
 * Haven't tested it, because I have only the new mrinfo.
 */
void
accept_neighbor_request(src, dst)
    u_int32 src;
    u_int32 dst;
{
    vifi_t vifi;
    struct uvif *v;
    u_char *p, *ncount;
/*    struct listaddr *la; */
    pim_nbr_entry_t *pim_nbr;
    int datalen;
    u_int32 temp_addr, them = src;

#define PUT_ADDR(a)     temp_addr = ntohl(a); \
                        *p++ = temp_addr >> 24; \
                        *p++ = (temp_addr >> 16) & 0xFF; \
                        *p++ = (temp_addr >> 8) & 0xFF; \
                        *p++ = temp_addr & 0xFF;

    p = (u_char *) (igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN);
    datalen = 0;

    for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
        if (v->uv_flags & VIFF_DISABLED)
            continue;
	
        ncount = 0;
	
	/* TODO: XXX: if we are PMBR, then check the DVMRP interfaces too */
        for (pim_nbr = v->uv_pim_neighbors; pim_nbr != (pim_nbr_entry_t *)NULL;
	     pim_nbr = pim_nbr->next) {
            /* Make sure that there's room for this neighbor... */
            if (datalen + (ncount == 0 ? 4 + 3 + 4 : 4) > MAX_DVMRP_DATA_LEN) {
                send_igmp(igmp_send_buf, INADDR_ANY, them, IGMP_DVMRP,
			  DVMRP_NEIGHBORS, htonl(PIMD_LEVEL), datalen);
                p = (u_char *) (igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN);
                datalen = 0;
                ncount = 0;
            }

            /* Put out the header for this neighbor list... */
            if (ncount == 0) {
                PUT_ADDR(v->uv_lcl_addr);
                *p++ = v->uv_metric;
                *p++ = v->uv_threshold;
                ncount = p;
                *p++ = 0;
                datalen += 4 + 3;
            }

            PUT_ADDR(pim_nbr->address);
            datalen += 4;
            (*ncount)++;
        }
    }

    if (datalen != 0)
        send_igmp(igmp_send_buf, INADDR_ANY, them, IGMP_DVMRP, DVMRP_NEIGHBORS,
		  htonl(PIMD_LEVEL), datalen);
}


/*
 * Send a list of all of our neighbors to the requestor, `src'.
 * Used for mrinfo support.
 * XXX: currently, we cannot specify the used multicast routing protocol;
 * only a protocol version is returned.
 */
void
accept_neighbor_request2(src, dst)
    u_int32 src;
    u_int32 dst;
{
    vifi_t vifi;
    struct uvif *v;
    u_char *p, *ncount;
/*    struct listaddr *la; */
    pim_nbr_entry_t *pim_nbr;
    int datalen;
    u_int32 them = src;

    p = (u_char *) (igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN);
    datalen = 0;

    for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
        register u_int32 vflags = v->uv_flags;
        register u_char rflags = 0;
        if (vflags & VIFF_TUNNEL)
            rflags |= DVMRP_NF_TUNNEL;
        if (vflags & VIFF_SRCRT)
            rflags |= DVMRP_NF_SRCRT;
	if (vflags & VIFF_PIM_NBR)
	    rflags |= DVMRP_NF_PIM;
        if (vflags & VIFF_DOWN)
            rflags |= DVMRP_NF_DOWN;
        if (vflags & VIFF_DISABLED)
            rflags |= DVMRP_NF_DISABLED;
        if (vflags & VIFF_QUERIER)
            rflags |= DVMRP_NF_QUERIER;
        if (vflags & VIFF_LEAF)
            rflags |= DVMRP_NF_LEAF;
        ncount = 0;
        pim_nbr = v->uv_pim_neighbors;
        if (pim_nbr == (pim_nbr_entry_t *)NULL) {
            /*
             * include down & disabled interfaces and interfaces on
             * leaf nets.
             */
            if (rflags & DVMRP_NF_TUNNEL)
                rflags |= DVMRP_NF_DOWN;
            if (datalen > MAX_DVMRP_DATA_LEN - 12) {
                send_igmp(igmp_send_buf, INADDR_ANY, them, IGMP_DVMRP,
			  DVMRP_NEIGHBORS2, htonl(PIMD_LEVEL), datalen);
                p = (u_char *) (igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN);
                datalen = 0;
            }
            *(u_int*)p = v->uv_lcl_addr;
            p += 4;
            *p++ = v->uv_metric;
            *p++ = v->uv_threshold;
            *p++ = rflags;
            *p++ = 1;
            *(u_int*)p =  v->uv_rmt_addr;
            p += 4;
            datalen += 12;
        } else {
            for ( ; pim_nbr; pim_nbr = pim_nbr->next) {
                /* Make sure that there's room for this neighbor... */
                if (datalen + (ncount == 0 ? 4+4+4 : 4) > MAX_DVMRP_DATA_LEN) {
                    send_igmp(igmp_send_buf, INADDR_ANY, them, IGMP_DVMRP,
			      DVMRP_NEIGHBORS2, htonl(PIMD_LEVEL), datalen);
                    p = (u_char *) (igmp_send_buf + MIN_IP_HEADER_LEN + IGMP_MINLEN);
                    datalen = 0;
                    ncount = 0;
                }
                /* Put out the header for this neighbor list... */
                if (ncount == 0) {
                    *(u_int*)p = v->uv_lcl_addr;
                    p += 4;
                    *p++ = v->uv_metric;
                    *p++ = v->uv_threshold;
                    *p++ = rflags;
                    ncount = p;
                    *p++ = 0;
                    datalen += 4 + 4;
                }
                *(u_int*)p = pim_nbr->address;
                p += 4;
                datalen += 4;
                (*ncount)++;
            }
        }
    }
    if (datalen != 0)
        send_igmp(igmp_send_buf, INADDR_ANY, them, IGMP_DVMRP,
		  DVMRP_NEIGHBORS2, htonl(PIMD_LEVEL), datalen);
}



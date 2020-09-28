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
 *  $Id: routesock.c,v 1.2 1998/05/29 20:08:57 kurtw Exp $
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

#include <sys/param.h>
#include <sys/file.h>
#include "defs.h"
#include <sys/socket.h>
#include <linux/types.h>
//#include <net/route.h>
#ifdef HAVE_ROUTING_SOCKETS
#include <net/if_dl.h>
#endif
//#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#ifdef HAVE_ROUTING_SOCKETS
union sockunion {
    struct  sockaddr sa;
    struct  sockaddr_in sin;
    struct  sockaddr_dl sdl;
} so_dst, so_ifp;
typedef union sockunion *sup;
int routing_socket;
int rtm_addrs, pid;
struct rt_metrics rt_metrics;
u_long rtm_inits;

/*
 * Local functions definitions.
 */
static int getmsg __P((register struct rt_msghdr *, int,
		       struct rpfctl *rpfinfo));

/*
 * TODO: check again!
 */
#ifdef IRIX
#define ROUNDUP(a) ((a) > 0 ? (1 + (((a) - 1) | (sizeof(__uint64_t) - 1))) \
		    : sizeof(__uint64_t))
#else
#define ROUNDUP(a) ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) \
		    : sizeof(long))
#endif /* IRIX */

#ifdef HAVE_SA_LEN
#define ADVANCE(x, n) (x += ROUNDUP((n)->sa_len))
#else
#define ADVANCE(x, n) (x += ROUNDUP(4))   /* TODO: a hack!! */
#endif

/* Open and initialize the routing socket */
int
init_routesock()
{
    int off;

    pid = getpid();
    routing_socket = socket(PF_ROUTE, SOCK_RAW, 0);
    if (routing_socket < 0) {
	log(LOG_ERR, 0, "\nRouting socket error");
	return -1;
    }
    if (fcntl(routing_socket, F_SETFL, O_NONBLOCK) == -1){
	log(LOG_ERR, 0, "\n Routing socket error");
	return -1;
    }
    if (setsockopt(routing_socket, SOL_SOCKET,
		   SO_USELOOPBACK, (char *)&off,
		   sizeof(off)) < 0){
	log(LOG_ERR, 0 , "\n setsockopt(SO_USELOOPBACK,0)");
	return -1;
    }
    return 0;
}


struct {
    struct  rt_msghdr m_rtm;
    char    m_space[512];
} m_rtmsg;


/* get the rpf neighbor info */
int
k_req_incoming(source, rpfp)
    u_int32 source;
    struct rpfctl *rpfp;
{
    int flags = RTF_STATIC;
    register sup su;
    static int seq;
    int rlen;
    register char *cp = m_rtmsg.m_space;
    register int l;
    struct rpfctl rpfinfo;

/* TODO: a hack!!!! */
#ifdef HAVE_SA_LEN
#define NEXTADDR(w, u) \
    if (rtm_addrs & (w)) { \
	l = ROUNDUP(u.sa.sa_len); bcopy((char *)&(u), cp, l); cp += l;\
    }
#else
#define NEXTADDR(w, u) \
    if (rtm_addrs & (w)) { \
	l = ROUNDUP(4); bcopy((char *)&(u), cp, l); cp += l;\
    }
#endif /* HAVE_SA_LEN */

    /* initialize */
    rpfp->rpfneighbor.s_addr = INADDR_ANY_N;
    rpfp->source.s_addr = source;

    /* check if local address or directly connected before calling the
     * routing socket
     */
    if ((rpfp->iif = find_vif_direct_local(source)) != NO_VIF) {
	rpfp->rpfneighbor.s_addr = source;
	return(TRUE);
    }

    /* prepare the routing socket params */
    rtm_addrs |= RTA_DST;
    rtm_addrs |= RTA_IFP;
    su = &so_dst;
    su->sin.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    su->sin.sin_len = sizeof(struct sockaddr_in);
#endif
    su->sin.sin_addr.s_addr = source;
    if (inet_lnaof(su->sin.sin_addr) == INADDR_ANY) {
	IF_DEBUG(DEBUG_RPF)
	    log(LOG_DEBUG, 0, "k_req_incoming: Invalid source %s",
		inet_fmt(source,s1));
	return(FALSE);
    }
    so_ifp.sa.sa_family = AF_LINK;
#ifdef HAVE_SA_LEN
    so_ifp.sa.sa_len = sizeof(struct sockaddr_dl);
#endif
    flags |= RTF_UP;
    flags |= RTF_HOST;
    flags |= RTF_GATEWAY;
    errno = 0;
    bzero((char *)&m_rtmsg, sizeof(m_rtmsg));

#define rtm m_rtmsg.m_rtm
    rtm.rtm_type	= RTM_GET;
    rtm.rtm_flags	= flags;
    rtm.rtm_version	= RTM_VERSION;
    rtm.rtm_seq 	= ++seq;
    rtm.rtm_addrs	= rtm_addrs;
    rtm.rtm_rmx 	= rt_metrics;
    rtm.rtm_inits	= rtm_inits;

    NEXTADDR(RTA_DST, so_dst);
    NEXTADDR(RTA_IFP, so_ifp);
    rtm.rtm_msglen = l = cp - (char *)&m_rtmsg;

    if ((rlen = write(routing_socket, (char *)&m_rtmsg, l)) < 0) {
	IF_DEBUG(DEBUG_RPF | DEBUG_KERN) {
	    if (errno == ESRCH)
		log(LOG_DEBUG, 0,
		    "Writing to routing socket: no such route\n");
	    else
		log(LOG_DEBUG, 0, "Error writing to routing socket");
	}
	return(FALSE);
    }

    do {
	l = read(routing_socket, (char *)&m_rtmsg, sizeof(m_rtmsg));
    } while (l > 0 && (rtm.rtm_seq != seq || rtm.rtm_pid != pid));

    if (l < 0) {
	IF_DEBUG(DEBUG_RPF | DEBUG_KERN)
	    log(LOG_DEBUG, 0, "Read from routing socket failed");
	return(FALSE);
    }

    if (getmsg(&rtm, l, &rpfinfo)){
	rpfp->rpfneighbor.s_addr = rpfinfo.rpfneighbor.s_addr;
	rpfp->iif = rpfinfo.iif;
    }
#undef rtm
    return (TRUE);
}


/*
 * Returns TRUE on success, FALSE otherwise. rpfinfo contains the result.
 */
int
getmsg(rtm, msglen, rpfinfop)
    register struct rt_msghdr *rtm;
    int msglen;
    struct rpfctl *rpfinfop;
{
    struct sockaddr *dst = NULL, *gate = NULL, *mask = NULL;
    struct sockaddr_dl *ifp = NULL;
    register struct sockaddr *sa;
    register char *cp;
    register int i;
    struct in_addr in;
    vifi_t vifi;
    struct uvif *v;

    if (rpfinfop == (struct rpfctl *)NULL)
	return(FALSE);

    in = ((struct sockaddr_in *)&so_dst)->sin_addr;
    IF_DEBUG(DEBUG_RPF)
	log(LOG_DEBUG, 0, "route to: %s", inet_fmt(in.s_addr, s1));
    cp = ((char *)(rtm + 1));
    if (rtm->rtm_addrs)
	for (i = 1; i; i <<= 1)
	    if (i & rtm->rtm_addrs) {
		sa = (struct sockaddr *)cp;
		switch (i) {
		case RTA_DST:
		    dst = sa;
		    break;
		case RTA_GATEWAY:
		    gate = sa;
		    break;
		case RTA_NETMASK:
		    mask = sa;
		    break;
		case RTA_IFP:
		    if (sa->sa_family == AF_LINK &&
			((struct sockaddr_dl *)sa)->sdl_nlen)
			ifp = (struct sockaddr_dl *)sa;
		    break;
		}
		ADVANCE(cp, sa);
	    }

    if (!ifp){ 	/* No incoming interface */
	IF_DEBUG(DEBUG_RPF)
	    log(LOG_DEBUG, 0,
		"No incoming interface for destination %s",
		inet_fmt(in.s_addr, s1));
	return(FALSE);
    }
    if (dst && mask)
	mask->sa_family = dst->sa_family;
    if (dst) {
	in = ((struct sockaddr_in *)dst)->sin_addr;
	IF_DEBUG(DEBUG_RPF)
	    log(LOG_DEBUG, 0, " destination is: %s",
		inet_fmt(in.s_addr, s1));
    }
    if (gate && rtm->rtm_flags & RTF_GATEWAY) {
	in = ((struct sockaddr_in *)gate)->sin_addr;
	IF_DEBUG(DEBUG_RPF)
	    log(LOG_DEBUG, 0, " gateway is: %s",
		inet_fmt(in.s_addr, s1));
	rpfinfop->rpfneighbor = in;
    }

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v)
	/* get the number of the interface by matching the name */
	if (!(strncmp(v->uv_name,ifp->sdl_data,ifp->sdl_nlen)))
	    break;

    IF_DEBUG(DEBUG_RPF)
	log(LOG_DEBUG, 0, " iif is %d", vifi);

    rpfinfop->iif = vifi;

    if (vifi >= numvifs){
	IF_DEBUG(DEBUG_RPF)
	    log(LOG_DEBUG, 0, "Invalid incoming interface for destination %s, because of invalid virtual interface", inet_fmt(in.s_addr, s1));
	return(FALSE);/* invalid iif */
    }

    return(TRUE);
}


#else	/* HAVE_ROUTING_SOCKETS */


/*
 * Return in rpfcinfo the incoming interface and the next hop router
 * toward source.
 */
/* TODO: check whether next hop router address is in network or host order */
int
k_req_incoming(source, rpfcinfo)
    u_int32 source;
    struct rpfctl *rpfcinfo;
{
    rpfcinfo->source.s_addr = source;
    rpfcinfo->iif = NO_VIF;     /* just initialized, will be */
    /* changed in kernel */
    rpfcinfo->rpfneighbor.s_addr = INADDR_ANY;   /* initialized */

    if (ioctl(udp_socket, SIOCGETRPF, (char *) rpfcinfo) < 0){
	log(LOG_ERR, errno, "ioctl SIOCGETRPF k_req_incoming");

	return(FALSE);
    }
    log(LOG_DEBUG, 0, "rpfinfo get from kernel: source:%s, iif:%d, rpfneighbor:%s",
            inet_fmt(rpfcinfo->source.s_addr, s1), rpfcinfo->iif, inet_fmt(rpfcinfo->rpfneighbor.s_addr, s2));
    return (TRUE);
}
#endif	/* HAVE_ROUTING_SOCKETS */


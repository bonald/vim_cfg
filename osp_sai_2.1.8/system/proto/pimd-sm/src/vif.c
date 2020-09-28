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
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */


#include "defs.h"
/*
 * Helper macros
 */
#define is_uv_subnet(src, v) \
    (src & v->uv_subnetmask) == v->uv_subnet && ((v->uv_subnetmask == 0xffffffff) || (src != v->uv_subnetbcast))

#define is_pa_subnet(src, p) \
    (src & p->pa_subnetmask) == p->pa_subnet && ((p->pa_subnetmask == 0xffffffff) || (src != p->pa_subnetbcast))

/*
 * Exported variables.
 */
struct uvif	uvifs[MAXVIFS]; /* array of all virtual interfaces          */
vifi_t		numvifs;	/* Number of vifs in use                    */
int             vifs_down;      /* 1=>some interfaces are down              */
int             phys_vif;       /* An enabled vif                           */
vifi_t		reg_vif_num;    /* really virtual interface for registers   */
int		udp_socket;	/* Since the honkin' kernel doesn't support
				 * ioctls on raw IP sockets, we need a UDP
				 * socket as well as our IGMP (raw) socket. */
int             total_interfaces; /* Number of all interfaces: including the
				   * non-configured, but excluding the
				   * loopback interface and the non-multicast
				   * capable interfaces.
				   */

uint32_t	default_route_metric   = UCAST_DEFAULT_ROUTE_METRIC;
uint32_t	default_route_distance = UCAST_DEFAULT_ROUTE_DISTANCE;

/*
 * Forward declarations
 */
static void start_vif      (vifi_t vifi);
static void stop_vif       (vifi_t vifi);
static void start_all_vifs (void);
static int init_reg_vif    (void);
static int update_reg_vif  (vifi_t register_vifi);
int delete_vif_from_mrt_for_port_down(vifi_t vifi);
void free_malloc_about_vif(vifi_t vifi);

void get_port_mc_info_from_cdb(vifi_t vifi)
{
	struct uvif *v;
    v	= &uvifs[vifi];
	
    char line[100] = {0};
	char cmd[128] = {0};
    FILE *fp;
	char *tmp;
	
	char multicast[21]={0};
	char *multicast_com="multicast";
	char igmp[21]={0};
	char *igmp_com="igmp";
	char igmp_version[21]={0};
	char *igmp_version_com="igmp_version";
	char igmp_intf_limit_num[21]={0};
	char *igmp_intf_limit_num_com="igmp_intf_limit_num";

	sprintf(cmd, "cdbctl read/cdb/l3/route_if/%s | tr \"\/\" \"\n\"", v->uv_name);
	fp=popen(cmd, "r");
    if(fp == NULL){
        return;
    }
    while(1)
    {
    	fgets(line, 100, fp);
		if(feof(fp)){
			break;
			}
		if(!line){
			continue;
			}		
		if(strncmp(line, multicast_com, strlen(multicast_com)) == 0)
			sscanf(line, "%*[^=]=%s", multicast);
		
		if((strncmp(line, igmp_com, strlen(igmp_com)) == 0) && (strlen(line) < 8))
			sscanf(line, "%*[^=]=%s", igmp);

		if(strncmp(line, igmp_version_com, strlen(igmp_version_com)) == 0)
			sscanf(line, "%*[^=]=%s", igmp_version);

		if(strncmp(line, igmp_intf_limit_num_com, strlen(igmp_intf_limit_num_com)) == 0)
			sscanf(line, "%*[^=]=%s", igmp_intf_limit_num);
    }
    pclose(fp);
	v->multicast = atoi(multicast);
	v->igmp = atoi(igmp);
	/*if cdb have no config about igmp_version*/
	if(strlen(igmp_version) == 2)
	{
		sprintf(cmd, "%s %s", v->uv_name, igmp_version);
		parse_phyint(cmd);
	}
	/*if cdb have no config*/
	if(atoi(igmp_intf_limit_num) != 0)
	{
		sprintf(cmd, "%s %s", v->uv_name, igmp_intf_limit_num);
		parse_interface_limit_number(cmd);
	}
	return;
}

void init_vifs(void)
{
    vifi_t vifi;
    struct uvif *v;
    int enabled_vifs;

    numvifs    = 0;
    reg_vif_num = NO_VIF;
    vifs_down = FALSE;

    /* Configure the vifs based on the interface configuration of the the kernel and
     * the contents of the configuration file.  (Open a UDP socket for ioctl use in
     * the config procedures if the kernel can't handle IOCTL's on the IGMP socket.) */
#ifdef IOCTL_OK_ON_RAW_SOCKET
    udp_socket = igmp_socket;
#else
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	logit(LOG_ERR, errno, "UDP socket");
#endif

    /* Clean up all vifs */
    for (vifi = 0, v = uvifs; vifi < MAXVIFS; ++vifi, ++v)
	zero_vif(v, FALSE);

	/*wait 3 seconds so we get the right stata about port, otherwise eth is down,and c-bsr/c-rp command 
	will failed*/
	sleep(3);
    config_vifs_from_kernel();

    if (!do_vifs) {
       for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v)
          v->uv_flags |= VIFF_DISABLED;
    }

    config_vifs_from_file();

    /*
     * Quit if there are fewer than two enabled vifs.
     */
    enabled_vifs    = 0;
    phys_vif        = -1;

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	/* Initialize the outgoing timeout for each vif.  Currently use a fixed time
	 * 'PIM_JOIN_PRUNE_HOLDTIME'.  Later, may add a configurable array to feed
	 * these parameters, or compute them as function of the i/f bandwidth and the
	 * overall connectivity...etc. */
	SET_TIMER(v->uv_jp_timer, pim_join_prune_holdtime);
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER | VIFF_TUNNEL))
	    continue;

	if (phys_vif == -1)
	    phys_vif = vifi;

	enabled_vifs++;
    }

    if (enabled_vifs < 1) /* XXX: TODO: */
	logit(LOG_ERR, 0, "Cannot forward: %s", enabled_vifs == 0 ? "no enabled vifs" : "only one enabled vif");

    k_init_pim(igmp_socket);	/* Call to kernel to initialize structures */

    /* Add a dummy virtual interface to support Registers in the kernel.
     * In order for this to work, the kernel has to have been modified
     * with the PIM patches to ip_mroute.{c,h} and ip.c
     */
    init_reg_vif();

    start_all_vifs();
}

/*
 * Initialize the passed vif with all appropriate default values.
 * "t" is true if a tunnel or register_vif, or false if a phyint.
 */
void zero_vif(struct uvif *v, int t)
{
/*changed by whs for default igmpv2*/
#if 0
    v->uv_flags		= 0;	/* Default to IGMPv3 */
#else
    v->uv_flags		= 0;
	v->uv_flags &= ~VIFF_IGMPV1;
	v->uv_flags |=	VIFF_IGMPV2;
#endif
    v->uv_metric	= DEFAULT_METRIC;
    v->uv_admetric	= 0;
    v->uv_threshold	= DEFAULT_THRESHOLD;
    v->uv_rate_limit	= t ? DEFAULT_REG_RATE_LIMIT : DEFAULT_PHY_RATE_LIMIT;
    v->uv_lcl_addr	= INADDR_ANY_N;
    v->uv_rmt_addr	= INADDR_ANY_N;
    v->uv_dst_addr	= t ? INADDR_ANY_N : allpimrouters_group;
    v->uv_subnet	= INADDR_ANY_N;
    v->uv_subnetmask	= INADDR_ANY_N;
    v->uv_subnetbcast	= INADDR_ANY_N;
    strlcpy(v->uv_name, "", IFNAMSIZ);
    v->uv_groups	= (struct listaddr *)NULL;
    v->uv_dvmrp_neighbors = (struct listaddr *)NULL;
    NBRM_CLRALL(v->uv_nbrmap);
    v->uv_querier	= (struct listaddr *)NULL;
    v->uv_igmpv1_warn	= 0;
    v->uv_prune_lifetime = 0;
    v->uv_acl		= (struct vif_acl *)NULL;
    RESET_TIMER(v->uv_leaf_timer);
    v->uv_addrs		= (struct phaddr *)NULL;
    v->uv_filter	= (struct vif_filter *)NULL;

    RESET_TIMER(v->uv_hello_timer);
    v->uv_dr_prio       = PIM_HELLO_DR_PRIO_DEFAULT;
    v->uv_genid         = 0;

    RESET_TIMER(v->uv_gq_timer);
    RESET_TIMER(v->uv_jp_timer);
    v->uv_pim_neighbors	= (struct pim_nbr_entry *)NULL;
    v->uv_local_pref	= default_route_distance;
    v->uv_local_metric	= default_route_metric;
#ifdef __linux__
    v->uv_ifindex	= -1;
#endif /* __linux__ */
}


/*
 * Add a (the) register vif to the vif table.
 */
static int init_reg_vif(void)
{
    struct uvif *v;
    vifi_t i;

    v = &uvifs[numvifs];
    v->uv_flags = 0;

    if ((numvifs + 1) == MAXVIFS) {
        /* Exit the program! The PIM router must have a Register vif */
	logit(LOG_ERR, 0, "Cannot install the Register VIF: too many interfaces");

	return FALSE;
    }

    /*
     * So far in PIM we need only one register vif and we save its number in
     * the global reg_vif_num.
     */
    reg_vif_num = numvifs;

    /* set the REGISTER flag */
    v->uv_flags = VIFF_REGISTER;
#ifdef PIM_EXPERIMENTAL
    v->uv_flags |= VIFF_REGISTER_KERNEL_ENCAP;
#endif
    strlcpy(v->uv_name, "register_vif0", sizeof(v->uv_name));

    /* Use the address of the first available physical interface to
     * create the register vif.
     */
    for (i = 0; i < numvifs; i++) {
	if (uvifs[i].uv_flags & (VIFF_DOWN | VIFF_DISABLED | VIFF_REGISTER | VIFF_TUNNEL))
	    continue;

	break;
    }

    if (i >= numvifs) {
	logit(LOG_ERR, 0, "No physical interface enabled");
	return -1;
    }

    v->uv_lcl_addr = uvifs[i].uv_lcl_addr;
    v->uv_threshold = MINTTL;

    numvifs++;
    total_interfaces++;

    return 0;
}


static void start_all_vifs(void)
{
    vifi_t vifi;
    struct uvif *v;
    u_int action;

    /* Start first the NON-REGISTER vifs */
    for (action = 0; ; action = VIFF_REGISTER) {
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	    /* If starting non-registers but the vif is a register or if starting
	     * registers, but the interface is not a register, then just continue. */
	    if ((v->uv_flags & VIFF_REGISTER) ^ action)
		continue;

	    /* Start vif if not DISABLED or DOWN */
	    if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN)) {
		if (v->uv_flags & VIFF_DISABLED)
		    logit(LOG_INFO, 0, "Interface %s is DISABLED; VIF #%u out of service", v->uv_name, vifi);
		else
		    logit(LOG_INFO, 0, "Interface %s is DOWN; VIF #%u out of service", v->uv_name, vifi);
	    } else {
		start_vif(vifi);
	    }
	}

	if (action == VIFF_REGISTER)
	    break;   /* We are done */
    }
}


/*
 * stop all vifs
 */
void stop_all_vifs(void)
{
    vifi_t vifi;
    struct uvif *v;

    for (vifi = 0; vifi < numvifs; vifi++) {
	v = &uvifs[vifi];
	if (!(v->uv_flags & VIFF_DOWN))
	    stop_vif(vifi);
    }
}


/*
 * Initialize the vif and add to the kernel. The vif can be either
 * physical, register or tunnel (tunnels will be used in the future
 * when this code becomes PIM multicast boarder router.
 */
static void start_vif(vifi_t vifi)
{
    struct uvif *v;

    v	= &uvifs[vifi];
    /* Initialy no router on any vif */
    if (v->uv_flags & VIFF_REGISTER)
	v->uv_flags = v->uv_flags & ~VIFF_DOWN;
    else {
	v->uv_flags = (v->uv_flags | VIFF_DR | VIFF_NONBRS) & ~VIFF_DOWN;
	v->uv_flags &= ~VIFF_DISABLED;
	/* https://tools.ietf.org/html/draft-ietf-pim-hello-genid-01 */
	v->uv_genid = RANDOM();

	SET_TIMER(v->uv_hello_timer, 1 + RANDOM() % pim_timer_hello_interval);
	SET_TIMER(v->uv_jp_timer, 1 + RANDOM() % pim_join_prune_period);
	/* TODO: CHECK THE TIMERS!!!!! Set or reset? */
	RESET_TIMER(v->uv_gq_timer);
	v->uv_pim_neighbors = (pim_nbr_entry_t *)NULL;
    }

    /* Tell kernel to add, i.e. start this vif */
    k_add_vif(igmp_socket, vifi, &uvifs[vifi]);

    if (!(v->uv_flags & VIFF_REGISTER)) {
	/* Join the PIM multicast group on the interface. */
	k_join(pim_socket, allpimrouters_group, v);

	/* Join the ALL-ROUTERS multicast group on the interface.  This
	 * allows mtrace requests to loop back if they are run on the
	 * multicast router. */
	k_join(igmp_socket, allrouters_group, v);

	/* Join INADDR_ALLRPTS_GROUP to support IGMPv3 membership reports */
	k_join(igmp_socket, allreports_group, v);

	/* Until neighbors are discovered, assume responsibility for sending
	 * periodic group membership queries to the subnet.  Send the first
	 * query. */
	v->uv_flags |= VIFF_QUERIER;
	/*当端口由shutdown->no shutdown, sleep 2秒,确保端口使能就绪
		不然有很大概率query报文发布出去*/
	sleep(2);
	query_groups(v);
	/* Send a probe via the new vif to look for neighbors. */
	send_pim_hello(v, pim_timer_hello_holdtime);
    }
#ifdef __linux__
    else {
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));

	if (mrt_table_id != 0) {
	        logit(LOG_INFO, 0, "Initializing pimreg%u", mrt_table_id);
		snprintf(ifr.ifr_name, IFNAMSIZ, "pimreg%u", mrt_table_id);
	} else {
		strlcpy(ifr.ifr_name, "pimreg", IFNAMSIZ);
	}

	if (ioctl(udp_socket, SIOGIFINDEX, (char *) &ifr) < 0) {
	    logit(LOG_ERR, errno, "ioctl SIOGIFINDEX for %s", ifr.ifr_name);
	    /* Not reached */
	    return;
	}
	v->uv_ifindex = ifr.ifr_ifindex;
    }
#endif /* __linux__ */

	/*add by zhw for igmp limit*/
	v->limit_igmp = IGMP_GLOBAL_LIMIT_NUMBER;
	v->active_groups = 0;
	get_port_mc_info_from_cdb(vifi);
}


/*
 * Stop a vif (either physical interface, tunnel or
 * register.) If we are running only PIM we don't have tunnels.
 */
static void stop_vif(vifi_t vifi)
{
    struct uvif *v;
    struct listaddr *a, *b;
    pim_nbr_entry_t *n, *next;
    struct vif_acl *acl;
	struct listaddr *g;
    /*
     * TODO: make sure that the kernel viftable is
     * consistent with the daemon table
     */
    v = &uvifs[vifi];
    if (!(v->uv_flags & VIFF_REGISTER)) {
	k_leave(pim_socket, allpimrouters_group, v);
	k_leave(igmp_socket, allrouters_group, v);
	k_leave(igmp_socket, allreports_group, v);

	/* Discard all group addresses.  (No need to tell kernel;
	 * the k_del_vif() call will clean up kernel state.) */
	 /*add by zhw:delete timer node in Q*/
	for (g = v->uv_groups; g != NULL; g = g->al_next) {
		if(g->al_query && (g->al_query != -1))
			g->al_query = DeleteTimer(g->al_query);
		
		if (g->al_timerid)
		    g->al_timerid = DeleteTimer(g->al_timerid);

		if (g->al_versiontimer)
			g->al_versiontimer = DeleteTimer(g->al_versiontimer);
    }
	
	while (v->uv_groups) {
	    a = v->uv_groups;
	    v->uv_groups = a->al_next;

	    while (a->al_sources) {
		b = a->al_sources;
		a->al_sources = b->al_next;
		free(b);
	    }

	    free(a);
	}
    }

    /*
     * TODO: inform (eventually) the neighbors I am going down by sending
     * PIM_HELLO with holdtime=0 so someone else should become a DR.
     */
    delete_vif_from_mrt(vifi);

    /* Delete the interface from the kernel's vif structure. */
    k_del_vif(igmp_socket, vifi, v);

    v->uv_flags = (v->uv_flags & ~VIFF_DR & ~VIFF_QUERIER & ~VIFF_NONBRS) | VIFF_DOWN;
    if (!(v->uv_flags & VIFF_REGISTER)) {
	RESET_TIMER(v->uv_hello_timer);
	RESET_TIMER(v->uv_jp_timer);
	RESET_TIMER(v->uv_gq_timer);

	for (n = v->uv_pim_neighbors; n; n = next) {
	    next = n->next;	/* Free the space for each neighbour */
	    delete_pim_nbr(n);
	}
	v->uv_pim_neighbors = NULL;
    }

    /* TODO: currently not used */
   /* The Access Control List (list with the scoped addresses) */
    while (v->uv_acl) {
	acl = v->uv_acl;
	v->uv_acl = acl->acl_next;
	free(acl);
    }

    vifs_down = TRUE;
    logit(LOG_INFO, 0, "Interface %s goes down; VIF #%u out of service", v->uv_name, vifi);
}


/*
 * Update the register vif in the multicast routing daemon and the
 * kernel because the interface used initially to get its local address
 * is DOWN. register_vifi is the index to the Register vif which needs
 * to be updated. As a result the Register vif has a new uv_lcl_addr and
 * is UP (virtually :))
 */
static int update_reg_vif(vifi_t register_vifi)
{
    struct uvif *v;
    vifi_t vifi;

    /* Find the first useable vif with solid physical background */
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER | VIFF_TUNNEL))
	    continue;

        /* Found. Stop the bogus Register vif first */
	stop_vif(register_vifi);
	uvifs[register_vifi].uv_lcl_addr = uvifs[vifi].uv_lcl_addr;

	start_vif(register_vifi);
	IF_DEBUG(DEBUG_PIM_REGISTER | DEBUG_IF) {
	    logit(LOG_NOTICE, 0, "Interface %s has come up; VIF #%u now in service",
		  uvifs[register_vifi].uv_name, register_vifi);
	}

	return 0;
    }

    vifs_down = TRUE;
    logit(LOG_WARNING, 0, "Cannot start Register VIF: %s", uvifs[vifi].uv_name);

    return -1;
}


/*
 * See if any interfaces have changed from up state to down, or vice versa,
 * including any non-multicast-capable interfaces that are in use as local
 * tunnel end-points.  Ignore interfaces that have been administratively
 * disabled.
 */
void check_vif_state(void)
{
    vifi_t vifi;
    struct uvif *v;
    struct ifreq ifr;
    static int checking_vifs = 0;

    /*
     * XXX: TODO: True only for DVMRP?? Check.
     * If we get an error while checking, (e.g. two interfaces go down
     * at once, and we decide to send a prune out one of the failed ones)
     * then don't go into an infinite loop!
     */
    if (checking_vifs)
	return;

    vifs_down = FALSE;
    checking_vifs = 1;

    /* TODO: Check all potential interfaces!!! */
    /* Check the physical and tunnels only */
	/*端口shutdown/no shutowdn会在此处检测
	 1.如果端口由no shutdown->shutdown，stop端口，修改mrt，free maloc,保留uvif结构
	 保证其由shutdown->no shutdow后数据存在(ip 地址,igmp属性等)
	 */
	 
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_REGISTER))
	    continue;
	
	strlcpy(ifr.ifr_name, v->uv_name, sizeof(ifr.ifr_name));
	if (ioctl(udp_socket, SIOCGIFFLAGS, (char *)&ifr) < 0) {
           if (errno == ENODEV) {
		   	 /*如果获取失败,需不需要删除uvif,需不需要设flag？此处保留！*/
              logit(LOG_NOTICE, 0, "Interface %s has gone; VIF #%u taken out of service", v->uv_name, vifi);
              stop_vif(vifi);
		      delete_vif_from_mrt_for_port_down(vifi);
			  free_malloc_about_vif(vifi);
              vifs_down = TRUE;
              continue;
           }
	   logit(LOG_ERR, errno, "%s(): ioctl SIOCGIFFLAGS for %s", __func__, ifr.ifr_name);
	}
	
	if (v->uv_flags & VIFF_DOWN) {
	    if (ifr.ifr_flags & IFF_UP){
		 	logit(LOG_ERR, errno, "zhw test: ioctl SIOCGIFFLAGS for %s", __func__, ifr.ifr_name);
			start_vif(vifi);
	    }
	    else
			vifs_down = TRUE;
	} else {
	    if (!(ifr.ifr_flags & IFF_UP)) {
		logit(LOG_NOTICE, 0, "Interface %s has gone down; VIF #%u taken out of service", v->uv_name, vifi);
		stop_vif(vifi);
		delete_vif_from_mrt_for_port_down(vifi);
		free_malloc_about_vif(vifi);
		vifs_down = TRUE;
	    }
	}
    }
    
    /* Check the register(s) vif(s) */
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	vifi_t vifi2;
	struct uvif *v2;
	int found;

	if (!(v->uv_flags & VIFF_REGISTER))
	    continue;

	found = 0;

	/* Find a physical vif with the same IP address as the
	 * Register vif. */
	for (vifi2 = 0, v2 = uvifs; vifi2 < numvifs; ++vifi2, ++v2) {
	    if (v2->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER | VIFF_TUNNEL))
		continue;

	    if (v->uv_lcl_addr != v2->uv_lcl_addr)
		continue;

	    found = 1;
	    break;
	}

	/* The physical interface with the IP address as the Register
	 * vif is probably DOWN. Get a replacement. */
	if (!found)
	    update_reg_vif(vifi);
    }

    checking_vifs = 0;
}


/*
 * If the source is directly connected to us, find the vif number for
 * the corresponding physical interface (Register and tunnels excluded).
 * Local addresses are excluded.
 * Return the vif number or NO_VIF if not found.
 */
vifi_t find_vif_direct(uint32_t src)
{
    vifi_t vifi;
    struct uvif *v;
    struct phaddr *p;

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER | VIFF_TUNNEL))
	    continue;

	if (src == v->uv_lcl_addr)
	    return NO_VIF;	/* src is one of our IP addresses */

	if (is_uv_subnet(src, v))
	    return vifi;

	/* Check the extra subnets for this vif */
	/* TODO: don't think currently pimd can handle extra subnets */
	for (p = v->uv_addrs; p; p = p->pa_next) {
	    if (is_pa_subnet(src, p))
		return vifi;
	}

	/* POINTOPOINT but not VIFF_TUNNEL interface (e.g., GRE) */
	if ((v->uv_flags & VIFF_POINT_TO_POINT) && (src == v->uv_rmt_addr))
	    return vifi;
    }

    return NO_VIF;
}


/*
 * Checks if src is local address. If "yes" return the vif index,
 * otherwise return value is NO_VIF.
 */
vifi_t local_address(uint32_t src)
{
    vifi_t vifi;
    struct uvif *v;
	char trmp[10];
    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	/* TODO: XXX: what about VIFF_TUNNEL? */
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER)){
		continue;
	}
	if (src == v->uv_lcl_addr)
	    return vifi;
    }

    /* Returning NO_VIF means not a local address */
    return NO_VIF;
}

/*
 * If the source is directly connected, or is local address,
 * find the vif number for the corresponding physical interface
 * (Register and tunnels excluded).
 * Return the vif number or NO_VIF if not found.
 */
vifi_t find_vif_direct_local(uint32_t src)
{
    vifi_t vifi;
    struct uvif *v;
    struct phaddr *p;

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	/* TODO: XXX: what about VIFF_TUNNEL? */
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER | VIFF_TUNNEL))
	    continue;

	if (src == v->uv_lcl_addr)
	    return vifi;	/* src is one of our IP addresses */

	if (is_uv_subnet(src, v))
	    return vifi;

	/* Check the extra subnets for this vif */
	/* TODO: don't think currently pimd can handle extra subnets */
	for (p = v->uv_addrs; p; p = p->pa_next) {
	    if (is_pa_subnet(src, p))
		return vifi;
	}

	/* POINTOPOINT but not VIFF_TUNNEL interface (e.g., GRE) */
	if ((v->uv_flags & VIFF_POINT_TO_POINT) && (src == v->uv_rmt_addr))
	    return vifi;
    }

    return NO_VIF;
}

/*
 * Returns the highest address of local vif that is UP and ENABLED.
 * The VIFF_REGISTER interface(s) is/are excluded.
 */
uint32_t max_local_address(void)
{
    vifi_t vifi;
    struct uvif *v;
    uint32_t max_address = 0;

    for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	/* Count vif if not DISABLED or DOWN */
	/* TODO: XXX: What about VIFF_TUNNEL? */
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_REGISTER))
	    continue;

	if (ntohl(v->uv_lcl_addr) > ntohl(max_address))
	    max_address = v->uv_lcl_addr;
    }

    return max_address;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 4
 * End:
 */



 
#include "proto.h"
#include "lib_netlink.h"
 
 
 int32 pimsm_add_newlink(char *ifname, uint32 ifindex)
 {
 	 return 0;
 
	 vifi_t vifi;
	 struct uvif *v;
	 int flag = 0;
	 
	 //IF_DEBUG(DEBUG_PIM) 
 	 logit(LOG_INFO, errno, "pimsm_add_newlink");

 
	 for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
			 if (v->uv_ifindex == ifindex){
				 flag=1;
				 break;
			 }	 
	 }
	 
 
	 if ( flag && ((v->uv_flags & VIFF_DOWN)) ){
		 v->uv_flags &= ~VIFF_DOWN;
	 }	 
 
 	 logit(LOG_INFO, errno, "netlink ifname:%s ifindex:%d flag:%d\n",
		   ifname,ifindex,flag);
	 
	 return 0;
 }
 
 int32 pimsm_del_newlink(char *ifname, uint32 ifindex)
 {
	 vifi_t vifi;
	 struct uvif *v;
	 int flag = 0;
  	 logit(LOG_INFO, errno, "pimsm_del_newlink");
	 for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
		 if (v->uv_ifindex == ifindex){
			 flag = 1;
			 break;
		 }
	 }
	 if ( flag && (!(v->uv_flags & VIFF_DOWN)) ){
		 v->uv_flags &= VIFF_DOWN;
	 }
 	 logit(LOG_INFO, errno, "netlink ifname:%s ifindex:%d flag:%d\n",
		   ifname,ifindex,flag);	 
		 
	 return 0;
 
 }
 
  int32 pimsm_ipv4_add_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen,
				   uint32 ifindex, uint32 distance)
 {
  	return 0;
 }
 
 int32 pimsm_ipv4_del_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen,
				   uint32 ifindex, uint32 distance)
 {
 	return 0;
 }
 
 int32 pimsm_ipv4_add_addr_nl(vrf_id_t vrf_id, addr_ipv4_t *addr, uint8 prefixlen, uint32 ifindex)
 {
	 vifi_t vifi;
	 struct uvif *v;
	 int flag = 0;
	 uint32_t flags;
     uint32_t addrlo, mask, subnet;	 
	 char * if_name ;	 
     struct ifaddrs *ifaddr, *ifa;	
	 
	 for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
		 if (v->uv_ifindex == ifindex){
			 flag = 1;
			 break;
		 }
	 }

	 //IF_DEBUG(DEBUG_PIM) 
	 logit(LOG_INFO, errno, "netlink add_addr:%s ifindex:%d  flag:%d  prefixlen=%d",
		   inet_fmt(addr->s_addr, s1, sizeof(s1)),ifindex, flag,prefixlen);  
	 
	 /*new port or new ipaddr*/
	if (getifaddrs_emb(&ifaddr) == -1) {
		logit(LOG_ERR, errno, "Failed retrieving interface addresses");
		return -1;
	}
   /*
     * Loop through all of the interfaces.
     */
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)		
	{
		/*
		 * Ignore any interface for an address family other than IP.
		 */
		if (!ifa->ifa_addr || !ifa->ifa_netmask || ifa->ifa_addr->sa_family != AF_INET) {
		    continue;
		}		
			
		addrlo  = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
		if(addrlo != addr->s_addr){
			continue;
		}
		
		mask  = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
		flags = ifa->ifa_flags;
	
		subnet = addrlo & mask;
		if (mask != 0xffffffff) {
			if ((!inet_valid_subnet(subnet, mask)) || (addrlo == subnet) || addrlo == (subnet | ~mask)) {
			if (!(inet_valid_host(addrlo) && ((mask == htonl(0xfffffffe)) || is_set(IFF_POINTOPOINT, flags)))) {
				logit(LOG_WARNING, 0, "Ignoring %s, has invalid address %s and/or netmask %s",
				  ifa->ifa_name, inet_fmt(addrlo, s1, sizeof(s1)), inet_fmt(mask, s2, sizeof(s2)));
				continue;
			}
			}
		}
	
		/*
		 * Ignore any interface that is connected to the same subnet as
		 * one already installed in the uvifs array.
		 */
		/*
		 * TODO: XXX: bug or "feature" is to allow only one interface per
		 * subnet?
		 */
		for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
			if (strcmp(v->uv_name, ifa->ifa_name) == 0) {
				if(flag){
					v->uv_lcl_addr		= addrlo;
					v->uv_subnet		= subnet;
					v->uv_subnetmask	= mask;
					if (mask != htonl(0xfffffffe))
						v->uv_subnetbcast = subnet | ~mask;
					else
						v->uv_subnetbcast = 0xffffffff;		

					logit(LOG_DEBUG, 0, "zhw test:restart interface:%s\n", v->uv_name);
					start_vif(vifi);
					goto ADDR_OUT;
				}else{
					logit(LOG_DEBUG, 0, "Ignoring %s (%s on subnet %s) (alias for vif#%u?)",
				    	 v->uv_name, inet_fmt(addrlo, s1, sizeof(s1)), netname(subnet, mask), vifi);
					break;
				}

			}
			/* we don't care about point-to-point links in same subnet */
			if (is_set(IFF_POINTOPOINT, flags))
			continue;
			if (is_set(VIFF_POINT_TO_POINT, v->uv_flags))
			continue;
	
			if (((addrlo & mask) == v->uv_subnet) && (v->uv_subnetmask == mask)) {
			logit(LOG_WARNING, 0, "Ignoring %s, same subnet as %s", ifa->ifa_name, v->uv_name);
			break;
			}
		}
		if (vifi != numvifs)
			continue;
	
		/*
		 * If there is room in the uvifs array, install this interface.
		 */
		if (numvifs == MAXVIFS) {
			logit(LOG_WARNING, 0, "Too many vifs, ignoring %s", ifa->ifa_name);
			continue;
		}
		v = &uvifs[numvifs];
		zero_vif(v, FALSE);
		v->uv_lcl_addr		= addrlo;
		v->uv_subnet		= subnet;
		v->uv_subnetmask	= mask;
		if (mask != htonl(0xfffffffe))
			v->uv_subnetbcast = subnet | ~mask;
		else
			v->uv_subnetbcast = 0xffffffff;
	
		strlcpy(v->uv_name, ifa->ifa_name, IFNAMSIZ);

		v->uv_mtu = 1500;
	
		if (is_set(IFF_POINTOPOINT, flags)) {
			v->uv_flags |= (VIFF_REXMIT_PRUNES | VIFF_POINT_TO_POINT);
			v->uv_rmt_addr = ((struct sockaddr_in *)(&ifa->ifa_dstaddr))->sin_addr.s_addr;
		} else if (mask == htonl(0xfffffffe)) {
			/*
			 * Handle RFC 3021 /31 netmasks as point-to-point links
			 */
			v->uv_flags |= (VIFF_REXMIT_PRUNES | VIFF_POINT_TO_POINT);
			if (addrlo == subnet)
			v->uv_rmt_addr = addrlo + htonl(1);
			else
			v->uv_rmt_addr = subnet;
		}
	
#ifdef __linux__
		/* On Linux we can enumerate using ifindex, no need for an IP address */
		v->uv_ifindex = if_nametoindex(v->uv_name);
		if (!v->uv_ifindex)
			logit(LOG_ERR, errno, "Failed reading interface index for %s", v->uv_name);
#endif
	
		if (v->uv_flags & VIFF_POINT_TO_POINT) {
			logit(LOG_INFO, 0, "VIF #%u: Installing %s (%s -> %s) rate %d",
			  numvifs, v->uv_name, inet_fmt(addrlo, s1, sizeof(s1)), inet_fmt(v->uv_rmt_addr, s2, sizeof(s2)),
			  v->uv_rate_limit);
		} else {
			logit(LOG_INFO, 0, "VIF #%u: Installing %s (%s on subnet %s) rate %d  netmask %s",
			  numvifs, v->uv_name, inet_fmt(addrlo, s1, sizeof(s1)), netname(subnet, mask),
			  v->uv_rate_limit,inet_fmt(mask, s2, sizeof(s2)));
		}

		
		SET_TIMER(v->uv_jp_timer, pim_join_prune_holdtime);

		/*
		 * If the interface is not yet up, set the vifs_down flag to
		 * remind us to check again later.
		 */
		if (!is_set(IFF_UP, flags)) {
			v->uv_flags |= VIFF_DOWN;
			vifs_down = TRUE;
		}else{
			start_vif(numvifs);
		}

				
		++numvifs;
	
	}

ADDR_OUT:
	freeifaddrs(ifaddr);
	 return 0;
 }


 /*当端口down，例如删除端口IP，端口由3层接口变成2层接口，对端口所属的MRT表项进行如下操作，确保组播流量
	转发行为正确。
	1.如果端口是MRT的iif，删除这条MRT表项
	2.如果端口是MRT的oif，将oif从oifs中移除；如果移除后oifs为空，那么等老化时间到后自动删除
	3.在(*，G)中移除leaf后,会自动移除对应(S,G)中的leaf*/
int delete_vif_from_mrt_for_port_down(vifi_t vifi)
{
    cand_rp_t  *cand_rp;
    grpentry_t *grp;
    grpentry_t *grp_next;
    mrtentry_t *mrt_grp;
    mrtentry_t *mrt_srcs;
    mrtentry_t *mrt_srcs_next;
    rp_grp_entry_t *rp_grp;
    rpentry_t *rp;

    /* Scan the (*,*,RP) entries zhw*/
    for (cand_rp = cand_rp_list; cand_rp; cand_rp = cand_rp->next) {
		rp = cand_rp->rpentry;
	/* Check the (*,G) and (S,G) entries */
	for (rp_grp = cand_rp->rp_grp_next; rp_grp; rp_grp = rp_grp->rp_grp_next) {
	    for (grp = rp_grp->grplink; grp; grp = grp_next) {
			grp_next   = grp->rpnext;
			mrt_grp    = grp->grp_route;   /*(*,G)*/
			mrt_srcs   = grp->mrtlink;    /*(S,G)*/

			 /* The (*,G) entry, mrt_grp->incoming is MAXVIFS*/  
			if (mrt_grp) {
				if(DEBUG_IGMP)
					logit(LOG_INFO, errno, "zhw test:(*,G) iterate %d %d\n", vifi, mrt_grp->incoming);
				
				if(vifi == mrt_grp->incoming){
					if(DEBUG_IGMP)
						logit(LOG_INFO, errno, "zhw test:(*,G) delete mrt incoming:%d\n",mrt_grp->incoming);  
					delete_mrtentry(mrt_grp);
				}
				else if (VIFM_ISSET(vifi, mrt_grp->leaves)) {
					if(DEBUG_IGMP)
						logit(LOG_INFO, errno, "zhw test:(*,G) delete leaf for %s\n", inet_fmt(mrt_grp->group->group, s2, sizeof(s2)));
					delete_leaf(vifi, INADDR_ANY_N, mrt_grp->group->group);
				}
				else if (VIFM_ISSET(vifi, mrt_grp->joined_oifs)){
					if(DEBUG_IGMP)
						logit(LOG_INFO, errno, "zhw test:(*,G) delete joined oif\n"); 
					VIFM_CLR(vifi, mrt_grp->joined_oifs);
					change_interfaces(mrt_grp,
				  		rp->incoming,
				 		mrt_grp->joined_oifs,
				  		mrt_grp->pruned_oifs,
				  		mrt_grp->leaves,
				  		mrt_grp->asserted_oifs, 0);
					
						mrt_grp->upstream = rp->upstream;
				}
			}

			/* For all (S,G) for this group */
			for (; mrt_srcs; mrt_srcs = mrt_srcs_next) {
				mrt_srcs_next = mrt_srcs->grpnext;
				if(vifi == mrt_srcs->incoming){
					if(DEBUG_IGMP)
						logit(LOG_INFO, errno, "zhw test:(S,G) delete mrt incoming:%d\n",mrt_srcs->incoming);  
					delete_mrtentry(mrt_srcs);
				}
				else if (VIFM_ISSET(vifi, mrt_srcs->joined_oifs)){
					if(DEBUG_IGMP)
						logit(LOG_INFO, errno, "zhw test:(S,G) delete joined\n"); 
					VIFM_CLR(vifi, mrt_srcs->joined_oifs);
					change_interfaces(mrt_srcs,
				  		rp->incoming,
				 		mrt_srcs->joined_oifs,
				  		mrt_srcs->pruned_oifs,
				  		mrt_srcs->leaves,
				  		mrt_srcs->asserted_oifs, 0);
				}
			} /* End of (S,G) loop */
	    } /* End of (*,G) loop */
	}
    } /* For all cand RPs */
}

/*add by zhw:当端口变成2层口或IP地址被删除后：
	1.free对应malloc的数据
	2.相关定时器节点删除在stop_vif中完成*/
void free_malloc_about_vif(vifi_t vifi)
{
	/*in stop_vif will free uv_groups,uv_pim_neighbours,uv_acl*/
	struct uvif *v;
	struct phaddr *p;
	
	v = &uvifs[vifi];
	/*need free uv_querier,uv_addrs*/
	if (v->uv_querier){
	    IF_DEBUG(DEBUG_IGMP) {
			logit(LOG_DEBUG, 0, "%s: delete uv_querier", __func__);
	    }
	    free(v->uv_querier);
	    v->uv_querier = NULL;
	}
		
	for (p = v->uv_addrs; p; p = p->pa_next) {
	   free(p);
	   v->uv_addrs = NULL;
	}
	
}

/*add by zhw:在free_malloc_and_timer_about_vif之后调用：
	清除该数组元素内容，保留ifindex和name，将端口状态置为disable，如果将其从
	数组中移除，然后移动其他数组元素，那么以数组下标为bitmap的数据将会混乱*/
void reset_vif_in_uvifs(vifi_t vifi)
{
	struct uvif *v;
	int flag = 0;
	int ifindex;
	char ifname[IFNAMSIZ] = {0};
	if (vifi >= MAXVIFS)
		return;
	
	v = &uvifs[vifi]; 
	ifindex = v->uv_ifindex;
	strlcpy(ifname, v->uv_name, IFNAMSIZ);
	zero_vif(v, FALSE);
	
	v->uv_ifindex = ifindex;
	v->uv_flags |= VIFF_DISABLED;
	strlcpy(v->uv_name, ifname, IFNAMSIZ);
	return;
}

 int32 pimsm_ipv4_del_addr_nl(vrf_id_t vrf_id, addr_ipv4_t *addr, uint8 prefixlen, uint32 ifindex)
 {
	 vifi_t vifi;
	 struct uvif *v;
	 int flag = 0;

 
	 for (vifi = 0, v = uvifs; vifi < numvifs; vifi++, v++) {
		 if (v->uv_ifindex == ifindex){
			 flag = 1;
			 break;
		 }
	 }
	 
	 logit(LOG_INFO, errno, "netlink del_addr:%s	 ifindex:%d  flag=%d",
		   inet_fmt(addr->s_addr, s1, sizeof(s1)),ifindex, flag);  
	 
	 if(flag){
	 	logit(LOG_INFO, errno, "zhw test:start stop_vif by nl\n");
		stop_vif( vifi);
	 	delete_vif_from_mrt_for_port_down(vifi);
		free_malloc_about_vif(vifi);
		reset_vif_in_uvifs(vifi);
		vifs_down = TRUE;
	 }else{
		/*uncontrol interface solve*/

	 }
	
	
 
	 return 0;
 
 }
 void pimsm_netlink_init()
 {


 	 logit(LOG_INFO, errno, "pimsm_netlink_init ");  

	 int ret = 0;
 
	 ret = lib_netlink_init_pimsm();
	 if(ret != 0){
		 logit(LOG_ERR, errno, "lib_netlink_init failed %d",ret);  
		 return ;
	 }
 
	 ret += 	 lib_netlink_register_add_newlink(pimsm_add_newlink);
	 ret += 	 lib_netlink_register_del_newlink(pimsm_del_newlink);
 
	 ret += 	 lib_netlink_register_ipv4_add_route(pimsm_ipv4_add_route_nl);
	 ret += 	 lib_netlink_register_ipv4_del_route(pimsm_ipv4_del_route_nl);
	 
	 ret += 	 lib_netlink_register_ipv4_add_addr(pimsm_ipv4_add_addr_nl);
	 ret += 	 lib_netlink_register_ipv4_del_addr(pimsm_ipv4_del_addr_nl);
	
     if(ret){
		 logit(LOG_ERR, errno, "pimsm_netlink_init failed %d",ret);  
	 }

 }


 void pim_netlink_handler()
 {
	 _lib_netlink_read_pimsm();   
 }


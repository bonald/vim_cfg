
#include "proto.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_bgp_define.h"
#include "gen/tbl_bgp.h"
#include "gen/tbl_bgp_network_define.h"
#include "gen/tbl_bgp_network.h"
#include "gen/tbl_bgp_neighbor_define.h"
#include "gen/tbl_bgp_neighbor.h"
#include "gen/tbl_rip_define.h"
#include "gen/tbl_rip.h"
#include "gen/tbl_rip_network_define.h"
#include "gen/tbl_rip_network.h"
#include "gen/tbl_rip_neighbor_define.h"
#include "gen/tbl_rip_neighbor.h"
#include "gen/tbl_rip_passive_if_define.h"
#include "gen/tbl_rip_passive_if.h"

#include "gen/tbl_ripng_define.h"
#include "gen/tbl_ripng.h"
#include "gen/tbl_ripng_network_define.h"
#include "gen/tbl_ripng_network.h"
#include "gen/tbl_ripng_passive_if_define.h"
#include "gen/tbl_ripng_passive_if.h"

#include "gen/tbl_ospf_define.h"
#include "gen/tbl_ospf.h"
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "gen/tbl_ospf_area_range_define.h"
#include "gen/tbl_ospf_area_range.h"
#include "gen/tbl_ospf_area_stub_define.h"
#include "gen/tbl_ospf_area_stub.h"
#include "gen/tbl_ospf_area_nssa_define.h"
#include "gen/tbl_ospf_area_nssa.h"
#include "gen/tbl_ospf_area_vlink_define.h"
#include "gen/tbl_ospf_area_vlink.h"

#include "gen/tbl_ospf6_define.h"
#include "gen/tbl_ospf6.h"
#include "gen/tbl_ospf6_area_range_define.h"
#include "gen/tbl_ospf6_area_range.h"
#include "gen/tbl_ospf6_area_stub_define.h"
#include "gen/tbl_ospf6_area_stub.h"
#include "gen/tbl_ospf6_interface_define.h"
#include "gen/tbl_ospf6_interface.h"


#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "gen/tbl_rr_prefix_define.h"
#include "gen/tbl_rr_prefix.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"

/*add by zhw for igmp_intf*/
#include "gen/tbl_pim_sm_define.h"
#include "gen/tbl_pim_sm.h"
#include "gen/tbl_pim_sm_bsr_define.h"
#include "gen/tbl_pim_sm_bsr.h"
#include "gen/tbl_pim_sm_rp_define.h"
#include "gen/tbl_pim_sm_rp.h"
#include "gen/tbl_pim_dm_define.h"
#include "gen/tbl_pim_dm.h"
#include "gen/tbl_igmp_intf_define.h"
#include "gen/tbl_igmp_intf.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_pim_sm_rp_gp_define.h"
#include "gen/tbl_pim_sm_rp_gp.h"

#include "ipmc_define.h"
#include "rsa_derhex.h"

extern int app_is_password_encryption(void);


#define CCS_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */

#define IPV4_LEN_TO_MASK(mask, len)  \
    {                           \
        (mask) = (len) ? ~((1 << (CCS_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
    }

static int32
_ccs_cfg_build_arp_fib_one(tbl_arp_fib_t *p_db_arp, tbl_iter_args_t *pargs)
{
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];
    FILE    *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));

    if (NULL != p_db_arp && ARP_TYPE_STATIC == p_db_arp->type)
    {
        sal_fprintf(fp, "arp %s %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_arp->key.ip), 
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_db_arp->mac_addr));
        *has_config = TRUE;
    }

    return PM_E_NONE;    
}

static int32
_ccs_cfg_build_arp_one(tbl_arp_t *p_db_arp, tbl_iter_args_t *pargs)
{
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];
    FILE    *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));
    
    if (ARP_TYPE_STATIC == p_db_arp->type && !p_db_arp->is_router)
    {
        sal_fprintf(fp, "arp %s %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_arp->key.ip), 
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_db_arp->mac_addr));
        *has_config = TRUE;
    }
    
    return PM_E_NONE;    
}

static int32
_ccs_cfg_build_static_route_one(tbl_static_route_cfg_t *p_rn, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];      
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv4_t prefix_cp;
    uint32      mask = 0;
    addr_ipv4_t     tmp_zero;

    if (NULL == p_rn)
    {
        sal_fprintf(fp, "route entry get none!\n");
        return PM_E_NONE;
    }

    if (AF_INET != p_rn->key.p.family)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 50196, 2019-01-07 */
    if (p_rn->dhcp_client)
    {
        return  PM_E_NONE;   
    }
    /* liwh end */

    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    IPV4_LEN_TO_MASK(mask, p_rn->key.p.prefixlen);
    sal_memcpy(&prefix_cp, &p_rn->key.p.u.prefix4, sizeof(addr_ipv4_t));
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    prefix_cp.s_addr = prefix_cp.s_addr & mask;
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);

    if (DEFAULT_VRF_ID == p_rn->key.vrf_id)
    {
        if (1 != p_rn->distance)
        {
            if (p_rn->track_id)
            {
                if (0 != sal_memcmp(&tmp_zero, &p_rn->key.nh_addr.ipv4, sizeof(tmp_zero)))
                {
                    sal_fprintf(fp, "ip route %s/%d %s %d track %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_rn->key.nh_addr.ipv4), p_rn->distance, p_rn->track_id);
                }
                else
                {
                    sal_fprintf(fp, "ip route %s/%d null 0 %d track %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        p_rn->distance, p_rn->track_id);
                }
            }
            else
            {
                if (0 != sal_memcmp(&tmp_zero, &p_rn->key.nh_addr.ipv4, sizeof(tmp_zero)))
                {
                    sal_fprintf(fp, "ip route %s/%d %s %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_rn->key.nh_addr.ipv4), p_rn->distance);
                }
                else
                {
                    sal_fprintf(fp, "ip route %s/%d null 0 %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        p_rn->distance);
                }
            }
        }
        else
        {
            if (p_rn->track_id)
            {
                if (0 != sal_memcmp(&tmp_zero, &p_rn->key.nh_addr.ipv4, sizeof(tmp_zero)))
                {
                    sal_fprintf(fp, "ip route %s/%d %s track %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_rn->key.nh_addr.ipv4), p_rn->track_id);
                }
                else
                {
                    sal_fprintf(fp, "ip route %s/%d null 0 track %d\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        p_rn->track_id);
                }
            }
            else
            {
                if (0 != sal_memcmp(&tmp_zero, &p_rn->key.nh_addr.ipv4, sizeof(tmp_zero)))
                {
                    sal_fprintf(fp, "ip route %s/%d %s\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen,
                        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_rn->key.nh_addr.ipv4));
                }
                else
                {
                    sal_fprintf(fp, "ip route %s/%d null 0\n", 
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen);
                }
            }
        }

        *has_config = TRUE;
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_static_route_v6_one(tbl_static_route_cfg_t *p_rn, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];      
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv6_t prefix_cp;
    //uint32      mask = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_interface_t * p_db_if = NULL;
    addr_ipv6_t tmp_zero;

    if (NULL == p_rn)
    {
        sal_fprintf(fp, "route v6 entry get none!\n");
        return PM_E_NONE;
    }

    if (AF_INET6 != p_rn->key.p.family)
    {
        return PM_E_NONE;
    }
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));
    
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv6_t));
    //IPV4_LEN_TO_MASK(mask, p_rn->key.p.prefixlen);
    sal_memcpy(&prefix_cp, &p_rn->key.p.u.prefix6, sizeof(addr_ipv6_t));
    //prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    //prefix_cp.s_addr = prefix_cp.s_addr & mask;
    //prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    apply_mask_ipv6_addr(&prefix_cp, p_rn->key.p.prefixlen);

    if (DEFAULT_VRF_ID == p_rn->key.vrf_id)
    {
        sal_fprintf(fp, "ipv6 route %s/%d ", 
            cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &prefix_cp),p_rn->key.p.prefixlen);

        /*ipv6 address gate || ipv6 address and ifindex*/
        if (NEXTHOP_TYPE_IPV6 == p_rn->nexthop_type_v6 || NEXTHOP_TYPE_IPV6_IFINDEX == p_rn->nexthop_type_v6 )
        {
            if (0 != sal_memcmp(&tmp_zero, &p_rn->key.nh_addr.ipv6, sizeof(tmp_zero)))
            {
                sal_fprintf(fp, "%s ", 
                    cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_rn->key.nh_addr.ipv6));
            }
            else
            {
                sal_fprintf(fp, "null 0 ");
            }
        }

        /*ifindex || || ipv6 address and ifindex*/
        if (NEXTHOP_TYPE_IPV6_IFINDEX == p_rn->nexthop_type_v6)
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(p_rn->key.ifindex_v6);
            if (NULL != p_db_if)
            {
                sal_fprintf(fp, "%s ", 
                    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            }
        }

        if (1 != p_rn->distance)
        {
            sal_fprintf(fp, "%d", p_rn->distance);
        }

        sal_fprintf(fp, "\n");

        *has_config = TRUE;
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_bgp_one(tbl_bgp_t *p_bgp, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_bgp)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));

	if (p_bgp->key.as_id) 
	{
		sal_fprintf(fp, "router bgp"INTEND_1_STR"%u\n",p_bgp->key.as_id);
	}
    if (p_bgp->router_id.s_addr)
    {
        sal_fprintf(fp, INTEND_1_STR"bgp router-id %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_bgp->router_id));
    }

    if (GLB_IS_BIT_SET(p_bgp->redistribute, RT_ROUTE_CONNECT)) 
	{
		if (p_bgp->red_metric[RT_ROUTE_CONNECT] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute connected metric %d\n",
				p_bgp->red_metric[RT_ROUTE_CONNECT]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute connected\n");
		}
    }
    if (GLB_IS_BIT_SET(p_bgp->redistribute, RT_ROUTE_STATIC)) 
	{
        if (p_bgp->red_metric[RT_ROUTE_STATIC] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute static metric %d\n",
				p_bgp->red_metric[RT_ROUTE_STATIC]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute static\n");
		}
    }
	if (GLB_IS_BIT_SET(p_bgp->redistribute, RT_ROUTE_OSPF)) 
	{
        if (p_bgp->red_metric[RT_ROUTE_OSPF] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute ospf metric %d\n",
				p_bgp->red_metric[RT_ROUTE_OSPF]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute ospf \n");
		}
    }
    if (GLB_IS_BIT_SET(p_bgp->redistribute, RT_ROUTE_RIP)) 
	{
        if (p_bgp->red_metric[RT_ROUTE_RIP] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute rip metric %d\n",
				p_bgp->red_metric[RT_ROUTE_RIP]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute rip \n");
		}
    }
	if (GLB_IS_BIT_SET(p_bgp->redistribute, RT_ROUTE_PIM)) 
	{
        if (p_bgp->red_metric[RT_ROUTE_PIM] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute pim metric %d\n",
				p_bgp->red_metric[RT_ROUTE_PIM]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute pim \n");
		}
    }

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_bgp_network_one(tbl_bgp_network_t *p_bgp_network, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    char    buff[CMD_BUF_256];
    addr_ipv4_t prefix_cp;
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_bgp_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buff, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_bgp_network->key.u.prefix4, sizeof(addr_ipv4_t));
#ifndef _CENTEC_
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    prefix_cp.s_addr = prefix_cp.s_addr & p_bgp_network->mask;
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
#endif
    if (p_bgp_network->key.u.prefix4.s_addr && p_bgp_network->mask.s_addr)
    {
        sal_fprintf(fp, INTEND_1_STR"network %s mask %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp),
            cdb_addr_ipv4_val2str(buff, CMD_BUF_256, &p_bgp_network->mask));
    }

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_bgp_neighbor_one(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    char    buff[CMD_BUF_256];
    addr_ipv4_t prefix_cp;
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_bgp_neighbor)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buff, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_bgp_neighbor->key.u.prefix4, sizeof(addr_ipv4_t));
    if (p_bgp_neighbor->key.u.prefix4.s_addr)
    {
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp);
        if(p_bgp_neighbor->remote_as)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s remote-as %d\n", 
                buf, p_bgp_neighbor->remote_as);
        }
        if(p_bgp_neighbor->ebgp_multihop == 256)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s ebgp-multihop\n", buf);
        }
        else if(p_bgp_neighbor->ebgp_multihop != 0)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s ebgp-multihop %d\n", 
                buf,p_bgp_neighbor->ebgp_multihop);
        }
        if(p_bgp_neighbor->password[0] != 0)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s password %s\n", 
                buf,p_bgp_neighbor->password);
        }
        if(p_bgp_neighbor->shutdown == 1)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s shutdown\n", buf);
        }
        if(p_bgp_neighbor->next_hop_self == 1)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s next-hop-self\n", buf);
        }
        if(p_bgp_neighbor->intf_name[0] != 0)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s update-source interface-name %s\n", 
                buf,p_bgp_neighbor->intf_name);
        }
        else if(p_bgp_neighbor->intf_addr.s_addr)
        {
            sal_fprintf(fp, INTEND_1_STR"neighbor %s update-source interface-ip %s\n", 
                buf,cdb_addr_ipv4_val2str(buff, CMD_BUF_256, &p_bgp_neighbor->intf_addr));
        }
    }
    *has_config = TRUE;
	return PM_E_NONE;
}

static int32
_ccs_cfg_build_rip_one(tbl_rip_t *p_rip, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_rip)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "router rip\n");
	
	
	if (p_rip->version != 0) 
	{
		sal_fprintf(fp, INTEND_1_STR"version %d\n",p_rip->version);
	}

	if (p_rip->allow_ecmp)
	{
        sal_fprintf(fp, INTEND_1_STR"allow-ecmp\n");
    }

	if ((p_rip->timers.update!=0 || p_rip->timers.timeout!=0 || p_rip->timers.garbage!=0) &&
		(p_rip->timers.update!=30 || p_rip->timers.timeout!=180 || p_rip->timers.garbage!=120))
	{
        sal_fprintf(fp, INTEND_1_STR"timers basic %d %d %d\n",
			p_rip->timers.update,p_rip->timers.timeout,p_rip->timers.garbage);
    }
	
	if (RIP_DEFAULT_INFORMATION == p_rip->default_information) 
	{
		sal_fprintf(fp, INTEND_1_STR"default-information originate\n");
	}

    if (GLB_IS_BIT_SET(p_rip->redistribute, RT_ROUTE_CONNECT)) 
	{
		if (p_rip->red_metric[RT_ROUTE_CONNECT] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute connected metric %d\n",
				p_rip->red_metric[RT_ROUTE_CONNECT]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute connected\n");
		}
    }
    if (GLB_IS_BIT_SET(p_rip->redistribute, RT_ROUTE_STATIC)) 
	{
        if (p_rip->red_metric[RT_ROUTE_STATIC] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute static metric %d\n",
				p_rip->red_metric[RT_ROUTE_STATIC]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute static\n");
		}
    }
	if (GLB_IS_BIT_SET(p_rip->redistribute, RT_ROUTE_OSPF)) 
	{
        if (p_rip->red_metric[RT_ROUTE_OSPF] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute ospf metric %d\n",
				p_rip->red_metric[RT_ROUTE_OSPF]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute ospf \n");
		}
    }
	if (GLB_IS_BIT_SET(p_rip->redistribute, RT_ROUTE_PIM)) 
	{
        if (p_rip->red_metric[RT_ROUTE_PIM] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute pim metric %d\n",
				p_rip->red_metric[RT_ROUTE_PIM]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute pim \n");
		}
    }

	if (p_rip->default_metric > 1) 
	{
        sal_fprintf(fp, INTEND_1_STR"default-metric %d\n",p_rip->default_metric);
    }

	if (p_rip->passive_if_default == 1) 
	{
        sal_fprintf(fp, INTEND_1_STR"passive-interface default\n");
    }

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ripng_one(tbl_ripng_t *p_ripng, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_ripng)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "router ripng\n");
	
	if (p_ripng->allow_ecmp)
	{
        sal_fprintf(fp, INTEND_1_STR"allow-ecmp\n");
    }

	if ((p_ripng->timers.update!=0 || p_ripng->timers.timeout!=0 || p_ripng->timers.garbage!=0) &&
		(p_ripng->timers.update!=30 || p_ripng->timers.timeout!=180 || p_ripng->timers.garbage!=120))
	{
        sal_fprintf(fp, INTEND_1_STR"timers basic %d %d %d\n",
			p_ripng->timers.update,p_ripng->timers.timeout,p_ripng->timers.garbage);
    }
	
	if (RIPNG_DEFAULT_INFORMATION == p_ripng->default_information) 
	{
		sal_fprintf(fp, INTEND_1_STR"default-information originate\n");
	}

    if (GLB_IS_BIT_SET(p_ripng->redistribute, RT_ROUTE_CONNECT)) 
	{
		if (p_ripng->red_metric[RT_ROUTE_CONNECT] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute connected metric %d\n",
				p_ripng->red_metric[RT_ROUTE_CONNECT]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute connected\n");
		}
    }
    if (GLB_IS_BIT_SET(p_ripng->redistribute, RT_ROUTE_STATIC)) 
	{
        if (p_ripng->red_metric[RT_ROUTE_STATIC] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute static metric %d\n",
				p_ripng->red_metric[RT_ROUTE_STATIC]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute static\n");
		}
    }
	if (GLB_IS_BIT_SET(p_ripng->redistribute, RT_ROUTE_OSPF)) 
	{
        if (p_ripng->red_metric[RT_ROUTE_OSPF] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute ospf metric %d\n",
				p_ripng->red_metric[RT_ROUTE_OSPF]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute ospf \n");
		}
    }
	if (GLB_IS_BIT_SET(p_ripng->redistribute, RT_ROUTE_PIM)) 
	{
        if (p_ripng->red_metric[RT_ROUTE_PIM] > 0) 
		{
			sal_fprintf(fp, INTEND_1_STR"redistribute pim metric %d\n",
				p_ripng->red_metric[RT_ROUTE_PIM]-1);	
		}
		else 
		{
        	sal_fprintf(fp, INTEND_1_STR"redistribute pim \n");
		}
    }

	if (p_ripng->default_metric > 1) 
	{
        sal_fprintf(fp, INTEND_1_STR"default-metric %d\n",p_ripng->default_metric);
    }

    *has_config = TRUE;

    return PM_E_NONE;
}


static int32
_ccs_cfg_build_rip_network_one(tbl_rip_network_t *p_rip_network, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv4_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_rip_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    IPV4_LEN_TO_MASK(mask, p_rip_network->key.prefixlen);
    sal_memcpy(&prefix_cp, &p_rip_network->key.u.prefix4, sizeof(addr_ipv4_t));

    #ifndef _CENTEC_
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    prefix_cp.s_addr = prefix_cp.s_addr & mask;
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    #endif
    
    sal_fprintf(fp, INTEND_1_STR"network %s/%d\n", 
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), p_rip_network->key.prefixlen);

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ripng_network_one(tbl_ripng_network_t *p_ripng_network, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv6_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_ripng_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv6_t));
    //IPV6_LEN_TO_MASK(mask, p_ripng_network->key.prefixlen);
    sal_memcpy(&prefix_cp, &p_ripng_network->key.u.prefix6, sizeof(addr_ipv6_t));
#if 0
    #ifndef _CENTEC_
    prefix_cp.s6_addr = sal_htonl(prefix_cp.s6_addr);
    prefix_cp.s6_addr = prefix_cp.s6_addr & mask;
    prefix_cp.s6_addr = sal_htonl(prefix_cp.s6_addr);
    #endif
#endif
    sal_fprintf(fp, INTEND_1_STR"network %s/%d\n", 
        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &prefix_cp), p_ripng_network->key.prefixlen);

    *has_config = TRUE;

    return PM_E_NONE;
}


static int32
_ccs_cfg_build_rip_neighbor_one(tbl_rip_neighbor_t *p_rip_neighbor, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv4_t prefix_cp;

    if (NULL == p_rip_neighbor)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_rip_neighbor->key.u.prefix4, sizeof(addr_ipv4_t));

    #ifndef _CENTEC_
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    #endif
    
    sal_fprintf(fp, INTEND_1_STR"neighbor %s\n", 
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp));

    *has_config = TRUE;
	return PM_E_NONE;
}

static int32
_ccs_cfg_build_rip_passive_if_one(tbl_rip_passive_if_t *p_rip_passive_if, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    char 	ifname[IFNAME_SIZE];

    if (NULL == p_rip_passive_if)
    {
        return PM_E_NONE;
    }

    sal_memset(ifname, 0, sizeof(ifname));
    sal_memcpy(ifname, p_rip_passive_if->key.name, sizeof(ifname));

    
    sal_fprintf(fp, INTEND_1_STR"passive-interface %s\n",ifname);

    *has_config = TRUE;
	return PM_E_NONE;
}

static int32
_ccs_cfg_build_ripng_passive_if_one(tbl_ripng_passive_if_t *p_ripng_passive_if, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    char 	ifname[IFNAME_SIZE];

    if (NULL == p_ripng_passive_if)
    {
        return PM_E_NONE;
    }

    sal_memset(ifname, 0, sizeof(ifname));
    sal_memcpy(ifname, p_ripng_passive_if->key.name, sizeof(ifname));

    
    sal_fprintf(fp, INTEND_1_STR"passive-interface %s\n",ifname);

    *has_config = TRUE;
	return PM_E_NONE;
}


static int32
_ccs_cfg_build_ospf_one(tbl_ospf_t *p_ospf, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_ospf)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_fprintf(fp, "router ospf\n");
    if (p_ospf->router_id.s_addr) {
        sal_fprintf(fp, INTEND_1_STR"router-id %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_ospf->router_id));
    }

    if (GLB_IS_BIT_SET(p_ospf->redistribute, RT_ROUTE_CONNECT)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute connected\n");
    }
    if (GLB_IS_BIT_SET(p_ospf->redistribute, RT_ROUTE_STATIC)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute static\n");
    }
	if (GLB_IS_BIT_SET(p_ospf->redistribute, RT_ROUTE_RIP)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute rip\n");
    }	

    if (OSPF_DEFAULT_ORIGINATE_ALWAYS == p_ospf->default_originate) {
        sal_fprintf(fp, INTEND_1_STR"default-information originate always\n");
    } else if (OSPF_DEFAULT_ORIGINATE_ZEBRA == p_ospf->default_originate) {
        sal_fprintf(fp, INTEND_1_STR"default-information originate\n");
    }

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf6_one(tbl_ospf6_t *p_ospf6, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_ospf6)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_fprintf(fp, "router ospf6\n");
    if (p_ospf6->router_id.s_addr) {
        sal_fprintf(fp, INTEND_1_STR"router-id %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_ospf6->router_id));
    }
	
    if (GLB_IS_BIT_SET(p_ospf6->redistribute, RT_ROUTE_CONNECT)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute connected\n");
    }
    if (GLB_IS_BIT_SET(p_ospf6->redistribute, RT_ROUTE_STATIC)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute static\n");
    }
	if (GLB_IS_BIT_SET(p_ospf6->redistribute, RT_ROUTE_RIPNG)) {
        sal_fprintf(fp, INTEND_1_STR"redistribute ripng\n");
    }	

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf6_interface_one(tbl_ospf6_interface_t *p_ospf6_interface, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
	char    buf[CMD_BUF_256];
    if (NULL == p_ospf6_interface)
    {
        return PM_E_NONE;
    }
	sal_memset(buf, 0, sizeof(buf));
    sal_fprintf(fp, INTEND_1_STR"interface %s area %s\n",p_ospf6_interface->interface, 
    			cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_ospf6_interface->area_id));
   
    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf_network_one(tbl_ospf_network_t *p_ospf_network, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv4_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_ospf_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    IPV4_LEN_TO_MASK(mask, p_ospf_network->key.prefixlen);
    sal_memcpy(&prefix_cp, &p_ospf_network->key.u.prefix4, sizeof(addr_ipv4_t));

    /* modified by wangjj for not fix bug 43713, syn with quagga, 2017-04-24 */
    #ifndef _CENTEC_
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    prefix_cp.s_addr = prefix_cp.s_addr & mask;
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    #else
    (void)mask;
    #endif
    
    sal_fprintf(fp, INTEND_1_STR"network %s/%d area %u\n", 
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), p_ospf_network->key.prefixlen,
         p_ospf_network->area_id);

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf_area_range_one(tbl_ospf_area_range_t *p_ospf_area_range, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv4_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_ospf_area_range)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    IPV4_LEN_TO_MASK(mask, p_ospf_area_range->key.p.prefixlen);
    sal_memcpy(&prefix_cp, &p_ospf_area_range->key.p.u.prefix4, sizeof(addr_ipv4_t));

    /* modified by wangjj for not fix bug 43713, syn with quagga, 2017-04-24 */
    #ifndef _CENTEC_
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    prefix_cp.s_addr = prefix_cp.s_addr & mask;
    prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
    #else
    (void)mask;
    #endif
    
    sal_fprintf(fp, INTEND_1_STR"area %u range %s/%d %s\n", 
        p_ospf_area_range->key.area_id, 
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), p_ospf_area_range->key.p.prefixlen,
        p_ospf_area_range->advertise?"":"not-advertise");

    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf6_area_range_one(tbl_ospf6_area_range_t *p_ospf6_area_range, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    char    buf[CMD_BUF_256];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    addr_ipv6_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_ospf6_area_range)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&prefix_cp, 0, sizeof(addr_ipv6_t));
   // IPV6_LEN_TO_MASK(mask, p_ospf6_area_range->key.p.prefixlen);
    sal_memcpy(&prefix_cp, &p_ospf6_area_range->key.p.u.prefix6, sizeof(addr_ipv6_t));

    sal_fprintf(fp, INTEND_1_STR"area %u range %s/%d %s\n", 
        p_ospf6_area_range->key.area_id, 
        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &prefix_cp), p_ospf6_area_range->key.p.prefixlen,
        p_ospf6_area_range->advertise?"":"not-advertise");

    *has_config = TRUE;

    return PM_E_NONE;
}


static int32
_ccs_cfg_build_ospf_area_stub_one(tbl_ospf_area_stub_t *p_ospf_area_stub, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];

    if (NULL == p_ospf_area_stub)
    {
        return PM_E_NONE;
    }

    if(1 == p_ospf_area_stub->no_summary)
        sal_fprintf(fp, INTEND_1_STR"area %u stub no-summary\n", p_ospf_area_stub->key.areaid);
    else
        sal_fprintf(fp, INTEND_1_STR"area %u stub\n", p_ospf_area_stub->key.areaid);
    
    *has_config = TRUE;
    
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf6_area_stub_one(tbl_ospf6_area_stub_t *p_ospf6_area_stub, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];

    if (NULL == p_ospf6_area_stub)
    {
        return PM_E_NONE;
    }

    if(1 == p_ospf6_area_stub->no_summary)
        sal_fprintf(fp, INTEND_1_STR"area %u stub no-summary\n", p_ospf6_area_stub->key.areaid);
    else
        sal_fprintf(fp, INTEND_1_STR"area %u stub\n", p_ospf6_area_stub->key.areaid);
    
    *has_config = TRUE;
    
    return PM_E_NONE;
}


static int32
_ccs_cfg_build_ospf_area_nssa_one(tbl_ospf_area_nssa_t *p_ospf_area_nssa, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];
    char *nssa_role_str[4] = {"", "translate-candidate", "translate-never", "translate-always"};

    if (NULL == p_ospf_area_nssa)
    {
        return PM_E_NONE;
    }

    if(1 == p_ospf_area_nssa->no_summary)
        sal_fprintf(fp, INTEND_1_STR"area %u nssa %s no-summary\n", p_ospf_area_nssa->key.areaid, nssa_role_str[p_ospf_area_nssa->nssa_role]);
    else
        sal_fprintf(fp, INTEND_1_STR"area %u nssa %s\n", p_ospf_area_nssa->key.areaid, nssa_role_str[p_ospf_area_nssa->nssa_role]);
    
    *has_config = TRUE;
    
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf_area_vlink_one(tbl_ospf_area_vlink_t *p_vl, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];
    char vl_peer_str[64] = {0};

    if (NULL == p_vl)
    {
        return PM_E_NONE;
    }
    cdb_addr_ipv4_val2str(vl_peer_str, 64, &p_vl->key.vl_peer);

    if((p_vl->auth_flag == 0) && (p_vl->auth_type == 0) && (p_vl->auth_key[0] == 0) && (p_vl->md5_id == 0) && (p_vl->md5_key[0] == 0) && 
       (p_vl->hello_interval == 0) && (p_vl->dead_interval == 0) && (p_vl->retransmit_interval == 0) && (p_vl->transmit_delay == 0))
    {
        sal_fprintf(fp, INTEND_1_STR"area %u virtual-link %s\n", p_vl->key.area_id, vl_peer_str);
    }
    
    if(p_vl->auth_flag || p_vl->auth_type || p_vl->auth_key[0] || p_vl->md5_id || p_vl->md5_key[0])
    {
        char *auth_type_str[3] = {"", " null", " message-digest"};
        char auth_key_str[64] = {0};
        char md5_key_str[128] = {0};
        
        if(p_vl->auth_key[0])
            sal_sprintf(auth_key_str, " authentication-key %s", p_vl->auth_key);
        if(p_vl->md5_id)
       	{
            if(app_is_password_encryption())
            {
                char szPasswd[PASSWD_SIZE] = {0};
                gen_passwd_crypt(p_vl->md5_key, szPasswd, PASSWD_SIZE);
                sal_sprintf(md5_key_str, " message-digest-key %d md5 8 %s", p_vl->md5_id, szPasswd);
            }
            else
                sal_sprintf(md5_key_str, " message-digest-key %d md5 %s", p_vl->md5_id, p_vl->md5_key);
        } 
        sal_fprintf(fp, INTEND_1_STR"area %u virtual-link %s%s%s%s%s\n", p_vl->key.area_id, vl_peer_str, 
                    p_vl->auth_flag ? " authentication" : "", auth_type_str[p_vl->auth_type], auth_key_str, md5_key_str);
    }
    
    if(p_vl->hello_interval || p_vl->dead_interval || p_vl->retransmit_interval || p_vl->transmit_delay)
    {
        char hello_str[64] = {0};
        char dead_str[64] = {0};
        char retrs_str[64] = {0};
        char delay_str[64] = {0};

        if(p_vl->hello_interval)
            sal_sprintf(hello_str, " hello-interval %d", p_vl->hello_interval);
        if(p_vl->dead_interval)
            sal_sprintf(dead_str, " dead-interval %d", p_vl->dead_interval);
        if(p_vl->retransmit_interval)
            sal_sprintf(retrs_str, " retransmit-interval %d", p_vl->retransmit_interval);
        if(p_vl->transmit_delay)
            sal_sprintf(delay_str, " transmit-delay %d", p_vl->transmit_delay);
            
        sal_fprintf(fp, INTEND_1_STR"area %u virtual-link %s%s%s%s%s\n", p_vl->key.area_id, vl_peer_str,
                    hello_str, dead_str, retrs_str, delay_str);
    }
    
    *has_config = TRUE;
    
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ospf_area_auth_one(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_ospf_area_auth)
    {
        return PM_E_NONE;
    }

    if (OSPF_AREA_AUTH_TYPE_MD5 == p_ospf_area_auth->auth_type)
    {
        sal_fprintf(fp, INTEND_1_STR"area %u authentication message-digest\n", p_ospf_area_auth->key.areaid);
        *has_config = TRUE;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_arp_user(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_arp_fib_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_arp_fib_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_arp(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_arp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_arp_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_nd_fib_one(tbl_nd_fib_t *p_db_nd_fib, tbl_iter_args_t *pargs)
{
    char    buf_addr[CMD_BUF_256];
    char    buf_mac[CMD_BUF_256];
    FILE    *fp = (FILE*)pargs->argv[0];
    uint32  *has_config = (uint32*)pargs->argv[1];
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    sal_memset(buf_addr, 0, CMD_BUF_256);
    sal_memset(buf_mac, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf_addr, CMD_BUF_256, &p_db_nd_fib->key.addr.u.prefix6); 
    cdb_mac_addr_val2str(buf_mac, CMD_BUF_256, p_db_nd_fib->mac_addr);

    if (GLB_FLAG_ISSET(p_db_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
    {
        if (IN6_IS_ADDR_LINKLOCAL(&(p_db_nd_fib->key.addr.u.prefix6)))
        {
            IFNAME_ETH2FULL(p_db_nd_fib->ifname, ifname_ext);
            sal_fprintf(fp, "ipv6 neighbor %s %s %s\n", buf_addr, buf_mac, ifname_ext);
            *has_config = TRUE;
        }
        else
        {
            sal_fprintf(fp, "ipv6 neighbor %s %s\n", buf_addr, buf_mac);
            *has_config = TRUE;
        }
    }
    
    return PM_E_NONE;    
}

int32
ccs_cfg_build_nd_fib(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_nd_fib_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_nd_fib_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

uint32
rt_api_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }
    
    return FALSE;
}

/*ipv4:1   ipv6:2   all:0*/
int32
ccs_cfg_build_rt_static_cfg(FILE* fp, cfg_cmd_para_t *para, uint32 show_ipv4_ipv6)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    if (show_ipv4_ipv6 == 2)
    {
        if (FALSE == rt_api_is_ipv6_profile())
        {
            CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
        }
    }

    if (show_ipv4_ipv6 == 1 || show_ipv4_ipv6 == 0)
    {
        args.argv[0] = fp;
        args.argv[1] = &has_config;
        tbl_static_route_cfg_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_static_route_one, &args);

        if (has_config)
        {
            sal_fprintf(fp, "!\n");
        }
    }

    if (show_ipv4_ipv6 == 2 || show_ipv4_ipv6 == 0)
    {
        has_config = FALSE;

        if (show_ipv4_ipv6 == 0 && FALSE == rt_api_is_ipv6_profile())
        {
            return PM_E_NONE;
        }
        
        args.argv[0] = fp;
        args.argv[1] = &has_config;
        tbl_static_route_cfg_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_static_route_v6_one, &args);

        if (has_config)
        {
            sal_fprintf(fp, "!\n");
        }
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_bgp(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_bgp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_bgp_one, &args);

    uint32 bgp_network_count = 0;
    tbl_bgp_network_master_t *p_master = NULL;

    p_master = tbl_bgp_network_get_master();
    if (NULL != p_master) {
        bgp_network_count = p_master->bgp_network_hash->count;
    }

    uint32 bgp_neighbor_count = 0;
    tbl_bgp_neighbor_master_t *p_nb_master = NULL;

    p_nb_master = tbl_bgp_neighbor_get_master();
    if (NULL != p_nb_master) {
        bgp_neighbor_count = p_nb_master->bgp_neighbor_hash->count;
    }

    if (has_config && !bgp_network_count && !bgp_neighbor_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_bgp_network(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_bgp_network_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_bgp_network_one, &args);

	uint32 bgp_neighbor_count = 0;
    tbl_bgp_neighbor_master_t *p_nb_master = NULL;

    p_nb_master = tbl_bgp_neighbor_get_master();
    if (NULL != p_nb_master) {
        bgp_neighbor_count = p_nb_master->bgp_neighbor_hash->count;
    }

    if (has_config && !bgp_neighbor_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_bgp_neighbor(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_bgp_neighbor_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_bgp_neighbor_one, &args);
    
    uint32 bgp_network_count = 0;
    tbl_bgp_network_master_t *p_master = NULL;

    p_master = tbl_bgp_network_get_master();
    if (NULL != p_master) {
        bgp_network_count = p_master->bgp_network_hash->count;
    }


    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_rip(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_rip_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_rip_one, &args);

    uint32 rip_network_count = 0;
    tbl_rip_network_master_t *p_master = NULL;

    p_master = tbl_rip_network_get_master();
    if (NULL != p_master) {
        rip_network_count = p_master->rip_network_hash->count;
    }

    uint32 rip_neighbor_count = 0;
    tbl_rip_neighbor_master_t *p_nb_master = NULL;

    p_nb_master = tbl_rip_neighbor_get_master();
    if (NULL != p_nb_master) {
        rip_neighbor_count = p_nb_master->rip_neighbor_hash->count;
    }

	uint32 rip_passive_if_count = 0;
    tbl_rip_passive_if_master_t *p_ps_master = NULL;

    p_ps_master = tbl_rip_passive_if_get_master();
    if (NULL != p_ps_master) {
        rip_passive_if_count = p_ps_master->rip_passive_if_hash->count;
    }

    if (has_config && !rip_network_count && !rip_neighbor_count && !rip_passive_if_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ripng(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ripng_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ripng_one, &args);

    uint32 ripng_network_count = 0;
    tbl_ripng_network_master_t *p_master = NULL;

    p_master = tbl_ripng_network_get_master();
    if (NULL != p_master) {
        ripng_network_count = p_master->ripng_network_hash->count;
    }

	uint32 ripng_passive_if_count = 0;
    tbl_ripng_passive_if_master_t *p_ps_master = NULL;

    p_ps_master = tbl_ripng_passive_if_get_master();
    if (NULL != p_ps_master) {
        ripng_passive_if_count = p_ps_master->ripng_passive_if_hash->count;
    }

    if (has_config && !ripng_network_count && !ripng_passive_if_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_rip_network(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_rip_network_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_rip_network_one, &args);

	uint32 rip_neighbor_count = 0;
    tbl_rip_neighbor_master_t *p_nb_master = NULL;

    p_nb_master = tbl_rip_neighbor_get_master();
    if (NULL != p_nb_master) {
        rip_neighbor_count = p_nb_master->rip_neighbor_hash->count;
    }

	uint32 rip_passive_if_count = 0;
    tbl_rip_passive_if_master_t *p_ps_master = NULL;

    p_ps_master = tbl_rip_passive_if_get_master();
    if (NULL != p_ps_master) {
        rip_passive_if_count = p_ps_master->rip_passive_if_hash->count;
    }

    if (has_config && !rip_neighbor_count && !rip_passive_if_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ripng_network(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ripng_network_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ripng_network_one, &args);

	uint32 ripng_passive_if_count = 0;
    tbl_ripng_passive_if_master_t *p_ps_master = NULL;

    p_ps_master = tbl_ripng_passive_if_get_master();
    if (NULL != p_ps_master) {
        ripng_passive_if_count = p_ps_master->ripng_passive_if_hash->count;
    }

    if (has_config  && !ripng_passive_if_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_router_rip_neighbor(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_rip_neighbor_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_rip_neighbor_one, &args);

	uint32 rip_passive_if_count = 0;
    tbl_rip_passive_if_master_t *p_ps_master = NULL;

    p_ps_master = tbl_rip_passive_if_get_master();
    if (NULL != p_ps_master) {
        rip_passive_if_count = p_ps_master->rip_passive_if_hash->count;
    }

    if (has_config && !rip_passive_if_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_rip_passive_if(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_rip_passive_if_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_rip_passive_if_one, &args);

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ripng_passive_if(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ripng_passive_if_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ripng_passive_if_one, &args);

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_router_ospf(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_one, &args);

    uint32 ospf_network_count = 0;
    tbl_ospf_network_master_t *p_master = NULL;

    p_master = tbl_ospf_network_get_master();
    if (NULL != p_master) {
        ospf_network_count = p_master->ospf_network_hash->count;
    }

    uint32 ospf_auth_count = 0;
    tbl_ospf_area_auth_master_t *p_auth_master = NULL;

    p_auth_master = tbl_ospf_area_auth_get_master();
    if (NULL != p_auth_master) {
        ospf_auth_count = p_auth_master->ospf_area_auth_hash->count;
    }

    uint32 ospf_area_range_count = 0;
    tbl_ospf_area_range_master_t *p_area_range_master = NULL;

    p_area_range_master = tbl_ospf_area_range_get_master();
    if (NULL != p_area_range_master) {
        ospf_area_range_count = p_area_range_master->ospf_area_range_hash->count;
    }

	uint32 stub_cnt = 0;
    tbl_ospf_area_stub_master_t *p_area_stub_master = tbl_ospf_area_stub_get_master();
    if(p_area_stub_master)
        stub_cnt = p_area_stub_master->ospf_area_stub_hash->count;

    uint32 nssa_cnt = 0;
    tbl_ospf_area_nssa_master_t *p_area_nssa_master = tbl_ospf_area_nssa_get_master();
    if(p_area_nssa_master)
        nssa_cnt = p_area_nssa_master->ospf_area_nssa_hash->count;

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;

    if (has_config && !ospf_network_count && !ospf_auth_count && !ospf_area_range_count && !stub_cnt && !nssa_cnt && !vlink_cnt)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf6(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf6_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf6_one, &args);

    uint32 ospf6_area_range_count = 0;
    tbl_ospf6_area_range_master_t *p_master = NULL;

    p_master = tbl_ospf6_area_range_get_master();
    if (NULL != p_master) {
        ospf6_area_range_count = p_master->ospf6_area_range_hash->count;
    }

    uint32 ospf6_area_stub_count = 0;
    tbl_ospf6_area_stub_master_t *p_stub_master = NULL;

    p_stub_master = tbl_ospf6_area_stub_get_master();
    if (NULL != p_stub_master) {
        ospf6_area_stub_count = p_stub_master->ospf6_area_stub_hash->count;
    }

    uint32 ospf6_interface_count = 0;
    tbl_ospf6_interface_master_t *p_interface_master = NULL;

    p_interface_master = tbl_ospf6_interface_get_master();
    if (NULL != p_interface_master) {
        ospf6_interface_count = p_interface_master->ospf6_interface_hash->count;
    }

    if (has_config && !ospf6_area_range_count && !ospf6_area_stub_count && !ospf6_interface_count)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf6_interface(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf6_interface_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf6_interface_one, &args);

	uint32 ospf6_area_range_count = 0;
    tbl_ospf6_area_range_master_t *p_master = NULL;

    p_master = tbl_ospf6_area_range_get_master();
    if (NULL != p_master) {
        ospf6_area_range_count = p_master->ospf6_area_range_hash->count;
    }

    uint32 ospf6_area_stub_count = 0;
    tbl_ospf6_area_stub_master_t *p_stub_master = NULL;

    p_stub_master = tbl_ospf6_area_stub_get_master();
    if (NULL != p_stub_master) {
        ospf6_area_stub_count = p_stub_master->ospf6_area_stub_hash->count;
    }

    if (has_config && !ospf6_area_range_count && !ospf6_area_stub_count)
    {
        sal_fprintf(fp, "!\n");
    }
	return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf_network(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_network_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_network_one, &args);

    uint32 ospf_auth_count = 0;
    tbl_ospf_area_auth_master_t *p_master = NULL;

    p_master = tbl_ospf_area_auth_get_master();
    if (NULL != p_master) {
        ospf_auth_count = p_master->ospf_area_auth_hash->count;
    }

    uint32 ospf_area_range_count = 0;
    tbl_ospf_area_range_master_t *p_area_range_master = NULL;

    p_area_range_master = tbl_ospf_area_range_get_master();
    if (NULL != p_area_range_master) {
        ospf_area_range_count = p_area_range_master->ospf_area_range_hash->count;
    }
    
	uint32 stub_cnt = 0;
    tbl_ospf_area_stub_master_t *p_area_stub_master = tbl_ospf_area_stub_get_master();
    if(p_area_stub_master)
        stub_cnt = p_area_stub_master->ospf_area_stub_hash->count;

    uint32 nssa_cnt = 0;
    tbl_ospf_area_nssa_master_t *p_area_nssa_master = tbl_ospf_area_nssa_get_master();
    if(p_area_nssa_master)
        nssa_cnt = p_area_nssa_master->ospf_area_nssa_hash->count;

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;
    
    if (has_config && !ospf_auth_count && !ospf_area_range_count && !stub_cnt && !nssa_cnt && !vlink_cnt)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf_area_auth(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_area_auth_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_area_auth_one, &args);

    uint32 ospf_area_range_count = 0;
    tbl_ospf_area_range_master_t *p_area_range_master = NULL;

    p_area_range_master = tbl_ospf_area_range_get_master();
    if (NULL != p_area_range_master) {
        ospf_area_range_count = p_area_range_master->ospf_area_range_hash->count;
    }
    
    uint32 stub_cnt = 0;
    tbl_ospf_area_stub_master_t *p_area_stub_master = tbl_ospf_area_stub_get_master();
    if(p_area_stub_master)
        stub_cnt = p_area_stub_master->ospf_area_stub_hash->count;

    uint32 nssa_cnt = 0;
    tbl_ospf_area_nssa_master_t *p_area_nssa_master = tbl_ospf_area_nssa_get_master();
    if(p_area_nssa_master)
        nssa_cnt = p_area_nssa_master->ospf_area_nssa_hash->count;

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;
    
    if (has_config && !ospf_area_range_count && !stub_cnt && !nssa_cnt && !vlink_cnt)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf_area_range(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_area_range_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_area_range_one, &args);
    
	uint32 stub_cnt = 0;
    tbl_ospf_area_stub_master_t *p_area_stub_master = tbl_ospf_area_stub_get_master();
    if(p_area_stub_master)
        stub_cnt = p_area_stub_master->ospf_area_stub_hash->count;

    uint32 nssa_cnt = 0;
    tbl_ospf_area_nssa_master_t *p_area_nssa_master = tbl_ospf_area_nssa_get_master();
    if(p_area_nssa_master)
        nssa_cnt = p_area_nssa_master->ospf_area_nssa_hash->count;

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;
    
    if (has_config && !stub_cnt && !nssa_cnt && !vlink_cnt) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf6_area_range(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf6_area_range_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf6_area_range_one, &args);
    
    uint32 ospf6_area_stub_count = 0;
    tbl_ospf6_area_stub_master_t *p_stub_master = NULL;

    p_stub_master = tbl_ospf6_area_stub_get_master();
    if (NULL != p_stub_master) {
        ospf6_area_stub_count = p_stub_master->ospf6_area_stub_hash->count;
    }

    if (has_config && !ospf6_area_stub_count)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_router_ospf_area_stub(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_area_stub_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_area_stub_one, &args);

    uint32 nssa_cnt = 0;
    tbl_ospf_area_nssa_master_t *p_area_nssa_master = tbl_ospf_area_nssa_get_master();
    if(p_area_nssa_master)
        nssa_cnt = p_area_nssa_master->ospf_area_nssa_hash->count;

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;
    
    if(has_config && !nssa_cnt && !vlink_cnt)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf6_area_stub(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf6_area_stub_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf6_area_stub_one, &args);

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf_area_nssa(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_area_nssa_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_area_nssa_one, &args);

    uint32 vlink_cnt = 0;
    tbl_ospf_area_vlink_master_t *p_area_vlink_master = tbl_ospf_area_vlink_get_master();
    if(p_area_vlink_master)
        vlink_cnt = p_area_vlink_master->ospf_area_vlink_hash->count;
    
    if(has_config && !vlink_cnt)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_ospf_area_vlink(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_ospf_area_vlink_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ospf_area_vlink_one, &args);
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_route_global(FILE* fp, cfg_cmd_para_t *para, uint32 has_arp, uint32 show_ipv6)
{
    tbl_route_global_t *pst_db_glb = tbl_route_global_get_route_global();
    uint32 has_config = FALSE;

    #ifdef GREATBELT
    if (1 == pst_db_glb->ip_redirects_global_en)
    {
        sal_fprintf(fp, "ip redirects\n");
        has_config = TRUE;
    }
    #else
    if (0 == pst_db_glb->ip_redirects_global_en)
    {
        sal_fprintf(fp, CMD_NO_STR" ip redirects\n");
        has_config = TRUE;
    }
    #endif
    
    if (has_arp)
    {
        /* modified by wangjj for fix bug 43214, 2017-03-13 */
        if (!pst_db_glb->gratuitous_arp_learn_en)
        {
            sal_fprintf(fp, CMD_NO_STR" gratuitous-arp-learning enable\n");
            has_config = TRUE;
        }
    }

    if (show_ipv6)
    {
        if (FALSE != pst_db_glb->ipv6_enable)
        {
            /*for bug 48639 rm cmd ipv6 enable*/
            //sal_fprintf(fp, "ipv6 enable\n");
            //has_config = TRUE;
        }
    }
        
    if (0 < pst_db_glb->max_static && 1024 != pst_db_glb->max_static)
    {
        sal_fprintf(fp, "max-static-routes %u\n", pst_db_glb->max_static);
        has_config = TRUE;
    }
        
    if (show_ipv6)
    {
        if (GLB_ND_DEFAULT_IPV6_HOP_LIMIT != pst_db_glb->nd_hop_limit)
        {
            sal_fprintf(fp, "ipv6 hop-limit %u\n", pst_db_glb->nd_hop_limit);
            has_config = TRUE;
        }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_vrrp_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_vrrp_global_t   *p_vrrp_glb = NULL;
    uint32               has_config = FALSE;

    p_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
 
    if (GLB_VRRP_VERSION_V3 == p_vrrp_glb->vrrp_version)
    {
        sal_fprintf(fp, "vrrp version v3\n");
        has_config = TRUE;
    }

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_vrrp_session_one(FILE* fp, tbl_vrrp_session_t *p_db_sess)
{
    char ipbuf[256];
    char ifname_ext[IFNAME_EXT_SIZE];
        
    sal_fprintf(fp, "router vrrp %u\n", p_db_sess->key.vrid);

    if (GLB_INVALID_IFINDEX != p_db_sess->ifindex)
    {
        IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);
        sal_fprintf(fp, INTEND_1_STR"interface %s\n", ifname_ext);
    }

    if (GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY != p_db_sess->config_priority) 
    {
        sal_fprintf(fp, INTEND_1_STR"priority %d\n", p_db_sess->config_priority);
    }

    if (p_db_sess->vip_status)
    {
        cdb_addr_val2str(ipbuf, 256, &p_db_sess->vip);
        sal_fprintf(fp, INTEND_1_STR"virtual-ip %s\n", ipbuf);
    }

    if (GLB_VRRP_ADVT_INT_DFLT != p_db_sess->advt_interval)
    {
        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"advertisement-interval %d\n", p_db_sess->advt_interval);
        }
        else
        {
            sal_fprintf(fp, INTEND_1_STR"advertisement-interval msec %d\n", p_db_sess->advt_interval);
        }
    }

    if (p_db_sess->learn_master)
    {
        sal_fprintf(fp, INTEND_1_STR"learnmaster-mode true\n");
    }

    if (!p_db_sess->preempt_mode)
    {
        sal_fprintf(fp, INTEND_1_STR"preempt-mode false\n");
    }

    if(GLB_VRRP_PREEMPT_DELAY_DFLT != p_db_sess->preempt_delay)
    {
        sal_fprintf(fp, INTEND_1_STR"preempt delay %d\n",p_db_sess->preempt_delay);
    }

    if (p_db_sess->trackobj_id)
    {
        if(p_db_sess->priority_delta == 10)
        {
            sal_fprintf(fp, INTEND_1_STR"track %d\n", p_db_sess->trackobj_id);
        }
        else
        {
            sal_fprintf(fp, INTEND_1_STR"track %d decrement %d\n", p_db_sess->trackobj_id, p_db_sess->priority_delta);
        }
    }

    if (p_db_sess->enable)
    {
        sal_fprintf(fp, INTEND_1_STR"enable\n");    
    }
    
    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}

int32
ccs_cfg_build_vrrp_session(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_vrrp_session_master_t *p_master = NULL;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    uint32                    vrid = 0;

    p_master = tbl_vrrp_session_get_master();

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (NULL == p_db_sess)
        {
            continue;
        }
        
        _ccs_cfg_build_vrrp_session_one(fp, p_db_sess);
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_ptp_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    //int32 count;
    uint32               has_config = FALSE;
    glb_ptp_clock_id_t clock_id;
    glb_ptp_clock_id_t clk_id = {0x00, 0x00, 0x00, 0xFF, 0xFE};
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();

    if (g_p_master->masterChangeTrap)
    {
        sal_fprintf(fp, "snmp-server trap enable ptp master-change\n");
        has_config = TRUE;
    }
    
    if (g_p_master->stateChangeTrap)
    {
        sal_fprintf(fp, "snmp-server trap enable ptp state-change\n");
        has_config = TRUE;
    }
    
    if (g_p_master->accuracyChange)
    {
        sal_fprintf(fp, "snmp-server trap enable ptp accuracy-change\n");
        has_config = TRUE;
    }
    /*
    if (g_p_master->frequency_recovery_enable)
    {
        sal_fprintf(fp, "ptp frequency-recovery enable\n");
        has_config = TRUE;
    }
*/
    if (g_p_master->global_enable)
    {
        sal_fprintf(fp, "ptp global-enable\n");
        has_config = TRUE;
    }

    /*get the default clock id*/
    sal_memset(clock_id, 0, sizeof(clock_id));
    sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
    sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
    sal_memcpy(clock_id, clk_id, GLB_PTP_CLOCK_ID_LEN);
    
    if (0 != sal_memcmp(clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN))
    {
        sal_fprintf(fp, "ptp clock-id %.04hX.%.04hX.%.04hX.%.04hX\n", 
                    sal_ntoh16(((uint16*)g_p_master->default_ds.clock_identity)[0]),
                    sal_ntoh16(((uint16*)g_p_master->default_ds.clock_identity)[1]),
                    sal_ntoh16(((uint16*)g_p_master->default_ds.clock_identity)[2]),
                    sal_ntoh16(((uint16*)g_p_master->default_ds.clock_identity)[3]));
        has_config = TRUE;
    }

    if (PTP_DEVICE_DEF != g_p_master->device_type)
    {
        if (PTP_DEVICE_OC == g_p_master->device_type)
        {
            sal_fprintf(fp, "ptp device-type oc\n");
            has_config = TRUE;
        }
        else if (PTP_DEVICE_BC == g_p_master->device_type)
        {
            sal_fprintf(fp, "ptp device-type bc\n");
            has_config = TRUE;
        }
        else if (PTP_DEVICE_E2ETC == g_p_master->device_type)
        {
            sal_fprintf(fp, "ptp device-type e2etc\n");
            has_config = TRUE;
        }
        else if (PTP_DEVICE_P2PTC == g_p_master->device_type)
        {
            sal_fprintf(fp, "ptp device-type p2ptc\n");
            has_config = TRUE;
        }
    }
    /*
    for(count = 0; count < PTP_TOCPU_VLAN_NUMBER_MAX; count++)
    {
        if(PTP_CLOCK_TOCPU_VID_DEF != g_p_master->tocpu_vid[count])
        {
            sal_fprintf(fp, "ptp vlan %d\n", g_p_master->tocpu_vid[count]);
            has_config = TRUE;
        }
    }

    if(PTP_CLCOK_TOD_SET_PULSE_DELAY_DEF != g_p_master->tod_pulse_delay)
    {
        sal_fprintf(fp, "ptp tod-pulse-delay-correction %d\n", g_p_master->tod_pulse_delay);
        has_config = TRUE;
    }
*/
    if (PTP_DEVICE_E2ETC == g_p_master->device_type || PTP_DEVICE_P2PTC == g_p_master->device_type)
    {
        /*_ptp_cli_global_config_write_tc*/
        if (PTP_DOMAIN_DEF != g_p_master->default_ds.primary_domain)
        {
            sal_fprintf(fp, "ptp tc-primary-domain %u\n", g_p_master->default_ds.primary_domain);
            has_config = TRUE;
        }
        if (PTP_TWO_STEP_DEF != g_p_master->default_ds.two_step_flag)
        {
            sal_fprintf(fp, "ptp clock-step %s\n", (g_p_master->default_ds.two_step_flag ? "two-step" : "one-step"));
            has_config = TRUE;
        }
    }
    else
    {
        if (g_p_master->default_ds.slave_only)
        {
            sal_fprintf(fp, "ptp slave-only\n");
            has_config = TRUE;
        }

        if (PTP_TWO_STEP_DEF != g_p_master->default_ds.two_step_flag)
        {
            sal_fprintf(fp, "ptp clock-step %s\n", (g_p_master->default_ds.two_step_flag ? "two-step" : "one-step"));
            has_config = TRUE;
        }

        if (PTP_DOMAIN_DEF != g_p_master->default_ds.domain_number)
        {
            sal_fprintf(fp, "ptp domain %u\n", g_p_master->default_ds.domain_number);
            has_config = TRUE;
        }

        if (PTP_PRIORITY_DEF != g_p_master->default_ds.priority1)
        {
            sal_fprintf(fp, "ptp priority1 %u\n", g_p_master->default_ds.priority1);
            has_config = TRUE;
        }

        if (PTP_PRIORITY_DEF != g_p_master->default_ds.priority2)
        {
            sal_fprintf(fp, "ptp priority2 %u\n", g_p_master->default_ds.priority2);
            has_config = TRUE;
        }

        if (PTP_TIME_SOURCE_DEF != g_p_master->time_prop_intern.time_source)
        {
            sal_fprintf(fp, "ptp time-source %s\n", ptp_desc_time_source(g_p_master->time_prop_intern.time_source));
            has_config = TRUE;
        }

        if (PTP_CLOCK_ACCURACY_DEF != g_p_master->default_ds.clock_quality.clk_accuracy)
        {
            if (g_p_master->default_ds.clock_quality.clk_accuracy >= 0x20
                && g_p_master->default_ds.clock_quality.clk_accuracy <= 0x31)
            {
                sal_fprintf(fp, "ptp clock-accuracy 0x%x\n", g_p_master->default_ds.clock_quality.clk_accuracy);
                has_config = TRUE;
            }
            else if (0xfe == g_p_master->default_ds.clock_quality.clk_accuracy)
            {
                sal_fprintf(fp, "ptp clock-accuracy 0x%x\n", g_p_master->default_ds.clock_quality.clk_accuracy);
                has_config = TRUE;
            }
        }

        if (PTP_CLOCK_CLASS_DEF != g_p_master->default_ds.clock_quality.clk_class
            && PTP_CLOCK_CLASS_SLAVE != g_p_master->default_ds.clock_quality.clk_class)
        {
            sal_fprintf(fp, "ptp clock-class %u\n", g_p_master->default_ds.clock_quality.clk_class);
            has_config = TRUE;
        }

        if (PTP_UTC_OFFSET_DEF != g_p_master->time_prop_intern.current_utc_offset)
        {
            sal_fprintf(fp, "ptp utc-offset %d\n", g_p_master->time_prop_intern.current_utc_offset);
            has_config = TRUE;
        }

        if (g_p_master->set_system_time)
        {
            sal_fprintf(fp, "ptp set-systime\n");
            has_config = TRUE;
        }


        if (g_p_master->time_prop_intern.leap59)
        {
            sal_fprintf(fp, "ptp leap59\n");
            has_config = TRUE;
        }

        if (g_p_master->time_prop_intern.leap61)
        {
            sal_fprintf(fp, "ptp leap61\n");
            has_config = TRUE;
        }
    }

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ipsla_entry_one(FILE* fp, tbl_ipsla_entry_t *p_db_entry)
{
    char   ip_str[256];
    char   ifname_ext[IFNAME_EXT_SIZE];

    sal_fprintf(fp, "ip sla monitor %u\n", p_db_entry->key.entry_id);

    if (sal_strlen(p_db_entry->desc))
    {
        sal_fprintf(fp, INTEND_1_STR"description %s\n", p_db_entry->desc);
    }

    if (p_db_entry->type)
    {
        if (p_db_entry->inaddr.s_addr)
        {
            sal_inet_ntop(AF_INET, &p_db_entry->inaddr, ip_str, sizeof(ip_str));
            sal_fprintf(fp, INTEND_1_STR"type icmp-echo %s", ip_str);
        }
        else
        {
            sal_fprintf(fp, INTEND_1_STR"type icmp-echo %s", "0.0.0.0");
        }

        if (p_db_entry->src_inaddr.s_addr)
        {
            sal_inet_ntop(AF_INET, &p_db_entry->src_inaddr, ip_str, sizeof(ip_str));
            sal_fprintf(fp, " source-ip %s", ip_str);
        }

        if (sal_strlen(p_db_entry->src_ifname))
        {
            sal_fprintf(fp, " source-interface %s", IFNAME_ETH2FULL(p_db_entry->src_ifname, ifname_ext));
        }
        sal_fprintf(fp, "\n");
    }

    if (GLB_IPSLA_FREQUENCY_DEFAULT != p_db_entry->frequency)
    {
        sal_fprintf(fp, INTEND_1_STR"frequency %d\n", p_db_entry->frequency);
    }

    if (GLB_IPSLA_INTERVAL_DEFAULT != p_db_entry->interval)
    {
        sal_fprintf(fp, INTEND_1_STR"interval %d\n", p_db_entry->interval);
    }

    if (GLB_IPSLA_PING_PROBE_COUNT_DEFAULT != p_db_entry->packets_per_test)
    {
        sal_fprintf(fp, INTEND_1_STR"packets-per-test %d\n", p_db_entry->packets_per_test);
    }

    if (GLB_IPSLA_TIMEOUT_DEFAULT != p_db_entry->timeout)
    {
        sal_fprintf(fp, INTEND_1_STR"timeout %d\n", p_db_entry->timeout);
    }

    if (GLB_IPSLA_THRESHOLD_DEFAULT != p_db_entry->threshold)
    {
        sal_fprintf(fp, INTEND_1_STR"threshold %d\n", p_db_entry->threshold);
    }

    if (GLB_IPSLA_PING_TTL_DEFAULT != p_db_entry->ttl)
    {
        sal_fprintf(fp, INTEND_1_STR"ttl %d\n", p_db_entry->ttl);
    }

    if (GLB_IPSLA_PING_TOS_DEFAULT != p_db_entry->tos)
    {
        sal_fprintf(fp, INTEND_1_STR"tos %d\n", p_db_entry->tos);
    }

    if (GLB_IPSLA_DATA_PATTERN_DEFAULT != p_db_entry->data_pattern)
    {
        sal_fprintf(fp, INTEND_1_STR"data-pattern %08x\n", p_db_entry->data_pattern);
    }

    if (GLB_IPSLA_DATA_SIZE_DEFAULT != p_db_entry->data_size)
    {
        sal_fprintf(fp, INTEND_1_STR"data-size %d\n", p_db_entry->data_size);
    }

    if (GLB_IPSLA_PING_FAIL_PERCENT_DEFAULT != p_db_entry->fail_percent)
    {
        sal_fprintf(fp, INTEND_1_STR"fail-percent %d\n", p_db_entry->fail_percent);
    }

    if (GLB_IPSLA_RESULT_PROBE_NUM_DEFAULT != p_db_entry->probe_result_config_cnt)
    {
        sal_fprintf(fp, INTEND_1_STR"statistics packet %d\n", p_db_entry->probe_result_config_cnt);
    }

    if (GLB_IPSLA_RESULT_TEST_NUM_DEFAULT != p_db_entry->test_result_config_cnt)
    {
        sal_fprintf(fp, INTEND_1_STR"statistics test %d\n", p_db_entry->test_result_config_cnt);
    }

    sal_fprintf(fp, "!\n");

    if (p_db_entry->admin_state)
    {
        sal_fprintf(fp, "ip sla monitor schedule %u\n", p_db_entry->key.entry_id);
    }
    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}

int32
ccs_cfg_build_ipsla_entry(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_ipsla_entry_master_t *p_master   = NULL;
    tbl_ipsla_entry_t        *p_db_entry = NULL;
    uint32                    entry_id   = 0;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET("Tbl_ipsla_global not found");
    }
    uint16 ipsla_entry_num = p_db_ipsla_glb->ipsla_entry_num;

    p_master = tbl_ipsla_entry_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_ipsla_entry is not exist");
    }

    for (entry_id = 1; entry_id <= GLB_IPSLA_ENTRY_MAX && ipsla_entry_num > 0; entry_id++)
    {
        p_db_entry = p_master->ipslaentry_array[entry_id];
        if (!p_db_entry)
        {
            continue;
        }

        _ccs_cfg_build_ipsla_entry_one(fp, p_db_entry);
        ipsla_entry_num--;
    }
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_track_object_one(FILE* fp, tbl_track_object_t *p_db_object)
{
    char   ifname_ext[IFNAME_EXT_SIZE];
    switch (p_db_object->type)
    {
    case GLB_TRACK_INTERFACE_LINKSTATE:
        sal_fprintf(fp, "track %u interface %s linkstate\n", p_db_object->key.trackobj_id, IFNAME_ETH2FULL(p_db_object->track_if, ifname_ext));
        break;

    case GLB_TRACK_RTR_REACHABILITY:
        sal_fprintf(fp, "track %u ip sla %d reachability\n", p_db_object->key.trackobj_id, p_db_object->rtr_entryid);
        break;

    case GLB_TRACK_RTR_STATE:
        sal_fprintf(fp, "track %u ip sla %d state\n", p_db_object->key.trackobj_id, p_db_object->rtr_entryid);
        break;

    default:
        return PM_E_NONE;
        break;
    }

    if(p_db_object->delay_up)
    {
        sal_fprintf(fp, INTEND_1_STR"delay up %d\n", p_db_object->delay_up);
    }

    if(p_db_object->delay_down)
    {
        sal_fprintf(fp, INTEND_1_STR"delay down %d\n", p_db_object->delay_down);
    }

    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}

int32
ccs_cfg_build_track_object(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_track_object_master_t *p_master   = NULL;
    tbl_track_object_t        *p_db_object = NULL;
    uint32                     object_id   = 0;
    tbl_track_global_t        *p_db_track_glb = NULL;

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        CFG_CONFLICT_RET("Tbl_ipsla_global not found");
    }
    uint16 track_object_num = p_db_track_glb->track_object_num;

    p_master = tbl_track_object_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_ipsla_entry is not exist");
    }

    for (object_id = 1; object_id <= GLB_TRACK_OBJECT_MAX && track_object_num > 0; object_id++)
    {
        p_db_object = p_master->trackobj_array[object_id];
        if (!p_db_object)
        {
            continue;
        }

        _ccs_cfg_build_track_object_one(fp, p_db_object);
        track_object_num--;
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_ecmp_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_ecmp_global_t     *p_db_ecmp_glb = NULL;

    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        CFG_CONFLICT_RET("Tbl_ecmp_global not found");
    }

    if (p_db_ecmp_glb->hash_algorithm != GLB_ECMP_HASH_ALGO_XOR)
    {
        sal_fprintf(fp, "ecmp load-balance hash-arithmetic crc\n");
    }

    if (p_db_ecmp_glb->hash_key != 0 && p_db_ecmp_glb->hash_key != GLB_ECMP_HASH_KEY_DEFAULT)
    {
        sal_fprintf(fp, "ecmp hash-field-select ");
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_MACSA))
        {
            sal_fprintf(fp, "macsa ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_MACDA))
        {
            sal_fprintf(fp, "macda ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_ETHERTYPE))
        {
            sal_fprintf(fp, "ethertype ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPSA))
        {
            sal_fprintf(fp, "ipsa ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPDA))
        {
            sal_fprintf(fp, "ipda ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPPRO))
        {
            sal_fprintf(fp, "ip-protocol ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_SRC_PORT))
        {
            sal_fprintf(fp, "src-port ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_DST_PORT))
        {
            sal_fprintf(fp, "dst-port ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_MACSA))
        {
            sal_fprintf(fp, "inner-macsa ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_MACDA))
        {
            sal_fprintf(fp, "inner-macda ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IPSA))
        {
            sal_fprintf(fp, "inner-ipsa ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IPDA))
        {
            sal_fprintf(fp, "inner-ipda ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL))
        {
            sal_fprintf(fp, "inner-ip-protocol ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_SRC_PORT))
        {
            sal_fprintf(fp, "inner-src-port ");
        }
        if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_DST_PORT))
        {
            sal_fprintf(fp, "inner-dst-port ");
        }

        sal_fprintf(fp, "\n");
    }

    if (GLB_ECMP_SELF_HEALING_LOAD_BALANCE == p_db_ecmp_glb->load_balance_mode)
    {
        sal_fprintf(fp, "ecmp load-balance-mode static self-healing\n");
    }
    else if (GLB_ECMP_DYNAMIC_LOAD_BALANCE == p_db_ecmp_glb->load_balance_mode)
    {
        if(GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP == p_db_ecmp_glb->dlb_mode_type)
        {
           sal_fprintf(fp, "ecmp load-balance-mode dynamic tcp-only\n");    
        }
        else if(GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT == p_db_ecmp_glb->dlb_mode_type)
        {
           sal_fprintf(fp, "ecmp load-balance-mode dynamic efd-only\n");           
        }
        else
        {
            sal_fprintf(fp, "ecmp load-balance-mode dynamic\n");
        }
    }

    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}

int32
ccs_cfg_build_rr_prefix(FILE * fp, cfg_cmd_para_t * para)
{
    uint8               rrprefix_index = 0;
    tbl_rr_prefix_t    *p_db_rrprefix = NULL;
    uint32              rr_exist = 0;
    char                buf[CMD_BUF_256];
    
    tbl_rr_prefix_master_t  *p_master = NULL;

    p_master = tbl_rr_prefix_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_rr_prefix not exist");
    }

    for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
    {
        p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
        if (NULL == p_db_rrprefix)
        {
            continue;
        }

        sal_fprintf(fp, "ecmp load-balance-mode round-robin %s/%d\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, 
                    &p_db_rrprefix->rr_prefix.u.prefix4), p_db_rrprefix->rr_prefix.prefixlen);
        rr_exist = 1;
    }
    if (rr_exist)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32
ccs_cfg_build_ipmc_global(FILE * fp, cfg_cmd_para_t * para)
{
    uint32             has_config = FALSE;
    tbl_ipmc_global_t *p_db_ipmc_glb = NULL;

    p_db_ipmc_glb = tbl_ipmc_global_get_ipmc_global();
    if (!p_db_ipmc_glb)
    {
        CFG_CONFLICT_RET("Not found tbl_ipmc_global");
    }

    if (p_db_ipmc_glb->mcast_route_en)
    {
        sal_fprintf(fp, "ip multicast-routing\n");
        has_config = TRUE;
    }

    if (0 != p_db_ipmc_glb->register_suppress_time && PIM_REGISTER_SUPPRESSION_TIME_DEFAULT != p_db_ipmc_glb->register_suppress_time)
    {
        sal_fprintf(fp, "ip pim register-suppress-time %d\n", p_db_ipmc_glb->register_suppress_time);
        has_config = TRUE;
    }

    if (0 != p_db_ipmc_glb->keep_alive_time && PIM_KEEPALIVE_PERIOD != p_db_ipmc_glb->keep_alive_time)
    {
        sal_fprintf(fp, "ip pim keep-alive-timer %d\n", p_db_ipmc_glb->keep_alive_time);
        has_config = TRUE;
    }

    if (0 != p_db_ipmc_glb->join_prune_interval && PIM_DEFAULT_T_PERIODIC != p_db_ipmc_glb->join_prune_interval)
    {
        sal_fprintf(fp, "ip pim join-prune-interval %d\n", p_db_ipmc_glb->join_prune_interval);
        has_config = TRUE;
    }

    if (0 != p_db_ipmc_glb->pim_packet_process && PIM_DEFAULT_PACKET_PROCESS != p_db_ipmc_glb->pim_packet_process)
    {
        sal_fprintf(fp, "ip pim packets %d\n", p_db_ipmc_glb->pim_packet_process);
        has_config = TRUE;
    }

    if (PIM_SPT_INFINITY == p_db_ipmc_glb->switchover_mode)
    {
        sal_fprintf(fp, "ip pim spt-switchover infinity-and-beyond\n");
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_pim_static_rp_one(tbl_pim_static_rp_t *p_rp, tbl_iter_args_t *pargs)
{
    FILE    *fp          = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    char     ip_addr[64];
    char     rp_addr[64];

    if (NULL == p_rp)
    {
        sal_fprintf(fp, "static rp get none!\n");
        return PM_E_NONE;
    }

    sal_fprintf(fp, "ip pim rp %s %s/%d\n", 
                cdb_addr_ipv4_val2str(rp_addr, 64, &p_rp->rp_addr),
                cdb_addr_ipv4_val2str(ip_addr, 64, &p_rp->key.group_range.u.prefix4), p_rp->key.group_range.prefixlen);

    *has_config = TRUE;

    return PM_E_NONE;
}

int32
ccs_cfg_build_pim_static_rp(FILE * fp, cfg_cmd_para_t * para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_pim_static_rp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_static_rp_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


static int32
_ccs_cfg_build_nat_pool_one(tbl_nat_pool_t *p_pool, tbl_iter_args_t *pargs)
{
    FILE    *fp          = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    char     addr_lo[64];
    char     addr_hi[64];
    char     addr_mask[64];

    if (NULL == p_pool)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "ip nat pool %s %s %s netmask %s\n", 
                p_pool->key.name,
                cdb_addr_ipv4_val2str(addr_lo, 64, &p_pool->ip_lo.u.prefix4),
                cdb_addr_ipv4_val2str(addr_hi, 64, &p_pool->ip_hi.u.prefix4), 
                cdb_addr_ipv4_val2str(addr_mask, 64, &p_pool->ip_mask.u.prefix4));

    *has_config = TRUE;
    return PM_E_NONE;
}

int32
ccs_cfg_build_nat_pool(FILE * fp, cfg_cmd_para_t * para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_nat_pool_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_nat_pool_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_nat_rule_one(tbl_nat_rule_t *p_rule, FILE *fp)
{
    char     addr[64];

    if (NULL == p_rule)
    {
        return PM_E_NONE;
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
    {
        sal_fprintf(fp, "ip nat inside source static ");

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
        {
            switch (p_rule->key.protocol)
            {
                case IPPROTO_TCP:
                {
                    sal_fprintf(fp, "%s ", "tcp");
                    break;
                }
                case IPPROTO_UDP:
                {
                    sal_fprintf(fp, "%s ", "udp");
                    break;
                }
                case IPPROTO_ICMP:
                {
                    sal_fprintf(fp, "%s ", "icmp");
                    break;
                }
            }
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP)
        {
            sal_fprintf(fp, "%s ", cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_local_ip.u.prefix4));
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
        {
            sal_fprintf(fp, "%u ", p_rule->key.in_local_port);
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
        {
            sal_fprintf(fp, "interface %s ", p_rule->key.oif);
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            sal_fprintf(fp, "%s ", cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_glb_ip.u.prefix4));
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI)
        {
            sal_fprintf(fp, "%u-%u ", p_rule->key.in_glb_port, p_rule->key.in_glb_port_hi);
        }
        else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
        {
            sal_fprintf(fp, "%u ", p_rule->key.in_glb_port);
        }
    }
    else
    {
        sal_fprintf(fp, "ip nat inside source ");
    
        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
        {
            sal_fprintf(fp, "access-list %s ", p_rule->key.acl);
        }
        else
        {
            sal_fprintf(fp, "any ");
        }


        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_POOL)
        {
            sal_fprintf(fp, "pool %s ", p_rule->key.pool);
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
        {
            sal_fprintf(fp, "interface %s ", p_rule->key.oif);
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_RANDOM)
        {
            sal_fprintf(fp, "%s ", "random");
        }
    }


    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}


int32
ccs_cfg_build_nat_rule(FILE * fp, cfg_cmd_para_t * para)
{
    uint32          i = 0;
    uint32          tmp = 0;
    uint32          rule_cnt = 0;
    uint32          order[GLB_NAT_RULE_NUM_MAX];
    tbl_nat_rule_t  a_rule[GLB_NAT_RULE_NUM_MAX];
    tbl_nat_rule_master_t *p_rule_master = NULL;
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    sal_memset(&a_rule, 0, sizeof(a_rule));
    sal_memset(&order, 0, sizeof(order));
    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_NONE;
    }

    if (0 == p_rule_master->nat_rule_hash->count)
    {
        return PM_E_NONE;
    }

    /* get rule list by config order*/
    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        sal_memcpy(&a_rule[rule_cnt], p_db_rule, sizeof(tbl_nat_rule_t));
        order[rule_cnt] = rule_cnt;
        for (i = rule_cnt; i > 0; i--)
        {
            if (a_rule[order[i]].cfg_order < a_rule[order[i-1]].cfg_order)
            {
                tmp = order[i];
                order[i] = order[i-1];
                order[i-1] = tmp;
            }
        }

        rule_cnt++;
    }

    for (i = 0; i < rule_cnt; i++)
    {
        _ccs_cfg_build_nat_rule_one(&a_rule[order[i]], fp);
    }

    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}


static int32
_ccs_cfg_build_nat_session_limit_one(tbl_nat_session_limit_t *p_session_limit, tbl_iter_args_t *pargs)
{
    FILE    *fp          = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    char     addr[64];

    if (NULL == p_session_limit)
    {
        return PM_E_NONE;
    }

    if (0 != p_session_limit->limit)
    {
        sal_fprintf(fp, "ip nat translation max-entries host %s %u\n", 
                        cdb_addr_ipv4_val2str(addr, 64, &p_session_limit->key.u.prefix4), 
                        p_session_limit->limit);

        *has_config = TRUE;
    }
    return PM_E_NONE;
}


int32
ccs_cfg_build_nat_session_limit(FILE * fp, cfg_cmd_para_t * para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_nat_session_limit_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_nat_session_limit_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_nat_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_nat_global_t   *p_nat_glb = NULL;
    uint32               has_config = FALSE;

    p_nat_glb = tbl_nat_global_get_nat_global();
 
    if ((NAT_SESSION_TTL_DEFAULT != p_nat_glb->timeout) && (0 != p_nat_glb->timeout))
    {
        sal_fprintf(fp, "ip nat translation timeout %u\n", p_nat_glb->timeout);
        has_config = TRUE;
    }

    if (0 != p_nat_glb->timeout_icmp)
    {
        sal_fprintf(fp, "ip nat translation timeout icmp %u\n", p_nat_glb->timeout_icmp);
        has_config = TRUE;
    }

    if (0 != p_nat_glb->timeout_tcp)
    {
        sal_fprintf(fp, "ip nat translation timeout tcp %u\n", p_nat_glb->timeout_tcp);
        has_config = TRUE;
    }

    if (0 != p_nat_glb->timeout_udp)
    {
        sal_fprintf(fp, "ip nat translation timeout udp %u\n", p_nat_glb->timeout_udp);
        has_config = TRUE;
    }

    if ((NAT_SESSION_SPEC != p_nat_glb->session_limit) && (0 != p_nat_glb->session_limit))
    {
        sal_fprintf(fp, "ip nat translation max-entries %u\n", p_nat_glb->session_limit);
        has_config = TRUE;
    }

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

/*add by zhw for ipmc*/
static int32
_ccs_cfg_build_pim_sm_rp_one(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_pim_sm_rp)
    {
        return PM_E_NONE;
    }
    
    sal_fprintf(fp, INTEND_1_STR"rp-candidate %s priority %d interval %d\n",
		p_pim_sm_rp->key.name, p_pim_sm_rp->priority, p_pim_sm_rp->interval);

    *has_config = TRUE;
	return PM_E_NONE;
}

static int32
_ccs_cfg_build_pim_sm_rp_gp_one(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_pim_sm_rp_gp)
    {
        return PM_E_NONE;
    }
	
    if (p_pim_sm_rp_gp->group_prefix.masklen != 0 && sal_strcmp(p_pim_sm_rp_gp->group_prefix.ip, "") != 0) {
		if((strcmp(p_pim_sm_rp_gp->group_prefix.ip, "224.0.0.0") != 0) && (p_pim_sm_rp_gp->group_prefix.masklen != 4))
		sal_fprintf(fp, INTEND_1_STR INTEND_1_STR"group-prefix %s masklen %d\n", 
			p_pim_sm_rp_gp->group_prefix.ip, p_pim_sm_rp_gp->group_prefix.masklen);
	}
    
    *has_config = TRUE;
	return PM_E_NONE;
}

static int32
_ccs_cfg_build_pim_sm_bsr_one(tbl_pim_sm_bsr_t *p_pim_sm_bsr, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_pim_sm_bsr)
    {
        return PM_E_NONE;
    }
    if(0 != p_pim_sm_bsr->interval){    
    	sal_fprintf(fp, INTEND_1_STR"bsr-candidate %s priority %d interval %d\n",
				p_pim_sm_bsr->key.name, p_pim_sm_bsr->priority, p_pim_sm_bsr->interval);
    }
	if(0 != p_pim_sm_bsr->dr_pri){
    	sal_fprintf(fp, INTEND_1_STR"phyint %s dr-priority %d\n",
				p_pim_sm_bsr->key.name, p_pim_sm_bsr->dr_pri);

	}

    *has_config = TRUE;
	return PM_E_NONE;
}


static int32
_ccs_cfg_build_pim_sm_one(tbl_pim_sm_t *p_pim_sm, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_pim_sm)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "router pim_sm\n");
	tbl_pim_sm_bsr_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_bsr_one, pargs);
	
	tbl_pim_sm_rp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_rp_one, pargs);

	tbl_pim_sm_rp_gp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_rp_gp_one, pargs);
	    
    if(p_pim_sm->hello_time != 0){
		sal_fprintf(fp, INTEND_1_STR"hello-interval  %d\n", p_pim_sm->hello_time);
	}

    if(p_pim_sm->jp_timer != 0){
		sal_fprintf(fp, INTEND_1_STR"jp-timer  %d\n", p_pim_sm->jp_timer);
	}

	if(p_pim_sm->spt_infinity != 0){
		sal_fprintf(fp, INTEND_1_STR"spt-threshold infinity\n");
	}
	
    if(p_pim_sm->rp_addr.masklen != 0){
		sal_fprintf(fp, INTEND_1_STR"rp-address %s %s masklen %d\n",p_pim_sm->rp_addr.ip,p_pim_sm->rp_addr.group,p_pim_sm->rp_addr.masklen);		
	}
	
    *has_config = TRUE;

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_pim_dm_one(tbl_pim_dm_t *p_pim_dm, tbl_iter_args_t *pargs)
{
    FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];

    if (NULL == p_pim_dm)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "router pim_dm\n");
	
    *has_config = TRUE;

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_pim_sm(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_pim_sm_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_one, &args);

	uint32 pim_sm_bsr_count = 0;
    tbl_pim_sm_bsr_master_t *p_bsr_master = NULL;

    p_bsr_master = tbl_pim_sm_bsr_get_master();
    if (NULL != p_bsr_master) {
        pim_sm_bsr_count = p_bsr_master->pim_sm_bsr_hash->count;
    }

	uint32 pim_sm_rp_count = 0;
    tbl_pim_sm_rp_master_t *p_rp_master = NULL;

    p_rp_master = tbl_pim_sm_rp_get_master();
    if (NULL != p_rp_master) {
        pim_sm_rp_count = p_rp_master->pim_sm_rp_hash->count;
    }
	
    if (has_config && !pim_sm_bsr_count && !pim_sm_rp_count) {
        sal_fprintf(fp, "!\n");
    }
	sal_fprintf(fp, "!\n");
    return PM_E_NONE;
}

int32
ccs_cfg_build_router_pim_sm_bsr(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_pim_sm_bsr_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_bsr_one, &args);

	uint32 pim_sm_rp_count = 0;
    tbl_pim_sm_rp_master_t *p_rp_master = NULL;

    p_rp_master = tbl_pim_sm_rp_get_master();
    if (NULL != p_rp_master) {
        pim_sm_rp_count = p_rp_master->pim_sm_rp_hash->count;
    }
	
    if (has_config && !pim_sm_rp_count) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_router_pim_sm_rp(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_pim_sm_rp_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_sm_rp_one, &args);

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
int32
ccs_cfg_build_router_pim_dm(FILE* fp, cfg_cmd_para_t *para)
{
    uint32          has_config = FALSE;
    tbl_iter_args_t args;

    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_pim_dm_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_pim_dm_one, &args);
	
    if (has_config ) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


/*add by zhw for igmp_intf*/
int32
ccs_cfg_build_igmp_intf(FILE* fp, cfg_cmd_para_t *para)
{
	tbl_igmp_intf_key_t		igmp_intf_key;
	sal_memset(&igmp_intf_key, 0, sizeof(igmp_intf_key));
	sal_strcpy(igmp_intf_key.name, "eth-0-1");
	
	tbl_igmp_intf_t			*p_igmp_intf = NULL;
	p_igmp_intf = tbl_igmp_intf_get_igmp_intf(&igmp_intf_key);
	
    if (NULL == p_igmp_intf)
    {
        return PM_E_NONE;
    }
	if((p_igmp_intf->last_member_query_count != 0) || (p_igmp_intf->last_member_query_interval != 0) ||
		(p_igmp_intf->query_interval != 0) || (p_igmp_intf->robustness_variable != 0) ||
		(p_igmp_intf->querier_timeout != 0) || (p_igmp_intf->global_limit_number != 0))
		sal_fprintf(fp, "igmp\n");
	else
		return PM_E_NONE;
	
	/*if(p_igmp_intf->layer3_protocol_index != 0){
		sal_fprintf(fp, INTEND_1_STR"  layer3_protocol_index  %d\n", p_igmp_intf->layer3_protocol_index);
	}

	if(p_igmp_intf->layer3_protocol != 0){
		sal_fprintf(fp, INTEND_1_STR"  layer3_protocol  %d\n", p_igmp_intf->layer3_protocol);
	}

	if(p_igmp_intf->action_index != 0){
		sal_fprintf(fp, INTEND_1_STR"  action_index  %d\n", p_igmp_intf->action_index);
	}
	
	if(p_igmp_intf->entry_valid != 0){
		sal_fprintf(fp, INTEND_1_STR"  entry_valid  %d\n", p_igmp_intf->entry_valid);
	}*/
	if(p_igmp_intf->last_member_query_count != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp last-member-query-count %d\n", p_igmp_intf->last_member_query_count);
	}
	if(p_igmp_intf->last_member_query_interval != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp last-member-query-interval %d\n", p_igmp_intf->last_member_query_interval);
	}
	if(p_igmp_intf->query_interval != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp query-interval %d\n", p_igmp_intf->query_interval);
	}
	if(p_igmp_intf->robustness_variable != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp robustness-variable %d\n", p_igmp_intf->robustness_variable);
	}
	if(p_igmp_intf->querier_timeout != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp querier-timeout %d\n", p_igmp_intf->querier_timeout);
	}
	if(p_igmp_intf->global_limit_number != 0){
		sal_fprintf(fp, INTEND_1_STR" ip igmp limit %d\n", p_igmp_intf->global_limit_number);
	}	
   
    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
	
}

int32
_ccs_cfg_build_mrt_one(tbl_mrt_t *p_mrt, tbl_iter_args_t *pargs)
{
	/*only show static mroute in running config*/
	if(p_mrt->static_mroute == 0)
		return PM_E_NONE;
	
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    if (NULL == p_mrt)
    {
        return PM_E_NONE;
    }
	
	char source[32] = {0};
	char group[32] = {0};

	sal_inet_ntop(AF_INET, (void*)&p_mrt->key.source, source, sizeof(source));
	sal_inet_ntop(AF_INET, (void*)&p_mrt->key.group, group, sizeof(group));
	
	uint32 ifindex = 0;
	char ifname_rpf[IFNAME_SIZE];
	char ifname_oif[IFNAME_SIZE];
	ifname_rpf[0] = CMD_ZERO_CHAR;
	tbl_interface_get_name_by_ifindex(p_mrt->rpf_ifindex, ifname_rpf, IFNAME_SIZE);
	
	GLB_BMP_ITER_BEGIN(p_mrt->oif_index_bmp, ifindex){
		ifname_oif[0] = CMD_ZERO_CHAR;
		tbl_interface_get_name_by_ifindex(ifindex, ifname_oif, IFNAME_SIZE);
		sal_fprintf(fp, "ip mroute source %s group %s rpf %s oif %s\n",source, group, 
			ifname_rpf, ifname_oif);
	}
	GLB_BMP_ITER_END(p_mrt->oif_index_bmp, ifindex);
	
    *has_config = TRUE;
	sal_fprintf(fp, "!\n");
    return PM_E_NONE;
}

int32
ccs_cfg_build_mrt(FILE* fp, cfg_cmd_para_t *para)
{ 
    uint32          has_config = FALSE;
    tbl_iter_args_t args;
	
    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_mrt_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_mrt_one, &args);
    return PM_E_NONE;
}
/*zhw end add*/




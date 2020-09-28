
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "cdb_ds.h"
#include "lib_ioctl.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "rt_fea.h"
#include "route.h"
#include "nd.h"
#include "nd_api.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "gen/ds_connected_v6_define.h"
#include "gen/ds_connected_v6.h"
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "rt_if.h"

int32 
nd_get_solicited_multicast(struct sal_in6_addr *p_addr, 
    struct sal_in6_addr *p_solicited)
{
    ND_PTR_CHECK(p_addr);
    ND_PTR_CHECK(p_solicited);

    p_solicited->s6_addr32[0] = sal_hton32(0xFF020000);
    p_solicited->s6_addr32[1] = sal_hton32(0x0);
    p_solicited->s6_addr32[2] = sal_hton32(0x1);
    p_solicited->s6_addr32[3] = sal_hton32(0xFF000000) | p_addr->s6_addr32[3];

    return PM_E_NONE;
}

uint16
nd_in6_checksum(prefix_t *p_src, prefix_t *p_dst, u_int8_t nxt_hdr, uint16 *ptr, uint16 length)
{
    register int32         sum = 0;
    register uint16        result = 0;
    uint16                  nbytes = 0;
    struct nd_in6_pseudo_hdr  in6_ph;
    uint16                 *tmp_p = NULL;

    /* Prepare IPv6 Pseudo-Header */
    sal_memset(&in6_ph, 0, sizeof (struct nd_in6_pseudo_hdr));
    sal_memcpy(&in6_ph.in6_src, &p_src->u.prefix6, IPV6_MAX_BYTELEN);
    sal_memcpy(&in6_ph.in6_dst, &p_dst->u.prefix6, IPV6_MAX_BYTELEN);  
    in6_ph.length = sal_hton32(length);
    in6_ph.nxt_hdr = nxt_hdr;

    /* First add IPv6 Pseudo-Header */
    sum = 0;
    nbytes = sizeof (struct nd_in6_pseudo_hdr);
    tmp_p = (uint16 *) &in6_ph;
    while (nbytes)
    {
        sum += *tmp_p++;
        nbytes -= 2;
    }

    /* Next checksum IPv6 PDU */
    nbytes = length;
    tmp_p = ptr;
    while (nbytes > 1)  
    {
        sum += *ptr++;
        nbytes -= 2;
    }
  
    if (nbytes == 1) 
    {
        sum += *((uint8*) tmp_p);
    }
  
    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
  
    return result;
}

void
nd_make_opt_pi(uint8 **input_pnt, uint32 *size, tbl_route_if_t *p_db_rtif, ds_nd_prefix_t  *p_nd_prefix, ds_connected_v6_t  *p_ifc)
{
    uint8 flags = 0;
    prefix_t p;
    uint8  *pnt= NULL;

    if (!p_nd_prefix && !p_ifc)
    {
        return;    
    }

    pnt = *input_pnt;
    
    /* Type. */
    ND_ENCODE_PUTC(SAL_ND_OPT_PREFIX_INFORMATION);

    /* Length. */
    ND_ENCODE_PUTC(4);

    /* Prefix Length. */
    if (p_nd_prefix)
    {
        ND_ENCODE_PUTC(p_nd_prefix->key.addr.prefixlen);
    }
    else
    {
        ND_ENCODE_PUTC(p_ifc->key.address.prefixlen);
    }

    /* Flags. */
    if (p_nd_prefix)
    {
        if (p_nd_prefix->onlink)
        {
            flags = flags | SAL_ND_OPT_PI_FLAG_ONLINK; 
        }

        if (p_nd_prefix->autoconfig)
        {
            flags = flags | SAL_ND_OPT_PI_FLAG_AUTO;    
        }
    }
    else
    {
        if (p_db_rtif->nd_ra_prefix_default_onlink)
        {
            flags = flags | SAL_ND_OPT_PI_FLAG_ONLINK; 
        }

        if (p_db_rtif->nd_ra_prefix_default_auto)
        {
            flags = flags | SAL_ND_OPT_PI_FLAG_AUTO;    
        }
    }
    ND_ENCODE_PUTC(flags);

    /* Valid Lifetime. */
    if (p_nd_prefix)
    {
        ND_ENCODE_PUTL(p_nd_prefix->vlifetime);
    }
    else
    {
        ND_ENCODE_PUTL(p_db_rtif->nd_ra_prefix_default_vlifetime);
    }

    /* Preferred Lifetime. */
    if (p_nd_prefix)
    {
        ND_ENCODE_PUTL(p_nd_prefix->plifetime);
    }
    else
    {
        ND_ENCODE_PUTL(p_db_rtif->nd_ra_prefix_default_plifetime);
    }

    /* Reserved2. */
    ND_ENCODE_PUTL(0);

    /* Prefix. */
    if (p_nd_prefix)
    {
        ND_ENCODE_PUT(&p_nd_prefix->key.addr.u.prefix6, GLB_IPV6_MAX_BYTELEN);
    }
    else
    {
        ip_prefix_copy (&p, &p_ifc->key.address);
        ip_apply_mask (&p);
        ND_ENCODE_PUT(&p.u.prefix6, GLB_IPV6_MAX_BYTELEN);
    }

    *input_pnt = pnt;

    return;
}

void
nd_make_ipv6_head(uint8 **input_pnt, uint32 *size, uint16 nd_size, struct sal_in6_addr *p_saddr, struct sal_in6_addr *p_daddr, uint16 icmpv6_cksum)
{
    uint8  *pnt= NULL;

    pnt = *input_pnt;

    ND_ENCODE_PUTC(0x60) ;      /* version and traffic class */
    ND_ENCODE_PUTC(0) ;
    ND_ENCODE_PUTW(0) ;
    ND_ENCODE_PUTW(nd_size) ;   /* payload length */
    ND_ENCODE_PUTC(IPPROTO_ICMPV6) ;
    ND_ENCODE_PUTC(ND_HDR_HOP_LIMIT) ;
    ND_ENCODE_PUT(p_saddr, IPV6_MAX_BYTELEN);
    ND_ENCODE_PUT(p_daddr, IPV6_MAX_BYTELEN);

    /* Fill ICMP6 checksum */
    pnt = pnt + 2;
    ND_ENCODE_PUTW(sal_ntoh16(icmpv6_cksum)) ;
    pnt = pnt + nd_size - 4;

    *input_pnt = pnt;

    return;
}

void
nd_make_l2_head(uint8 **input_pnt, uint32 *size, struct sal_in6_addr *p_daddr, tbl_interface_t  *p_db_if)
{
    uint8                            *pnt= NULL;
    tbl_nd_fib_t                   nd_fib;
    tbl_nd_fib_t                   *p_nd_fib;

    pnt = *input_pnt;
    
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));

    nd_fib_make_key(&nd_fib.key, p_daddr, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
    if (p_nd_fib)
    {
        ND_ENCODE_PUT(p_nd_fib->mac_addr, GLB_ETH_ADDR_LEN);     
    }
    else
    {
        /* dest mac */    
        ND_ENCODE_PUTC(0x33);
        ND_ENCODE_PUTC(0x33);
        ND_ENCODE_PUTL(sal_ntoh32(p_daddr->s6_addr32[3]));
    }

    /* src mac */
    ND_ENCODE_PUT(p_db_if->mac_addr, GLB_ETH_ADDR_LEN);

    /* set ipv6 ethertype */
    ND_ENCODE_PUTW(ETH_P_IPV6);

    *input_pnt = pnt;

    return;
}

int32
nd_make_ns (tbl_route_if_t *p_db_rtif,  struct sal_in6_addr *p_daddr, struct sal_in6_addr *p_target, tbl_route_global_t  *p_rt_glb, struct sal_in6_addr *p_saddr)
{
    uint32                         *size = NULL;
    uint8                          *pnt = NULL;
    tbl_interface_t             *p_db_if = NULL;
    uint16                         nd_size = 0;
    prefix_t                       src;
    prefix_t                       dst;
    uint16                         icmpv6_cksum = 0;
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_target);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_memset(&src, 0, sizeof(prefix_t));
    sal_memset(&dst, 0, sizeof(prefix_t));

    pnt = p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN;
    size = &(p_rt_glb->pkt_len);

    /* set src and dst */
    src.family = AF_INET6;
    sal_memcpy(&src.u.prefix6, p_saddr, IPV6_MAX_BYTELEN);
    dst.family = AF_INET6;
    sal_memcpy(&dst.u.prefix6, p_daddr, IPV6_MAX_BYTELEN);

    /* make ns header */
    /* type */
    ND_ENCODE_PUTC(SAL_ND_NEIGHBOR_SOLICIT) ;
    
    /* code */
    ND_ENCODE_PUTC(0) ;

    /* checksum */
    ND_ENCODE_PUTW(0) ;

    /* reserved */
    ND_ENCODE_PUTL(0) ;

    /* target address. */
    ND_ENCODE_PUT(p_target, 16) ;

    /* make Source Link-layer address option */
    /* Type. */
    ND_ENCODE_PUTC(SAL_ND_OPT_SOURCE_LINKADDR);

    /* Length. */
    ND_ENCODE_PUTC((ND6_OPT_SPACE(MAC_ADDR_LEN)>>3));

    /* Link-Layer Address. */
    ND_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);

    /* fill in NS fields */
    nd_size = pnt - p_rt_glb->buf - IPV6_HDR_LEN - ETH_HDR_LEN;
    icmpv6_cksum = nd_in6_checksum(&src, &dst, IPPROTO_ICMPV6, (uint16*)(p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN), nd_size);

    /* add IPv6 header */
    pnt = p_rt_glb->buf + ETH_HDR_LEN;
    nd_make_ipv6_head(&pnt, size, nd_size, p_saddr, p_daddr, icmpv6_cksum);

    /* add l2 header */
    pnt = p_rt_glb->buf;
    nd_make_l2_head(&pnt, size, p_daddr, p_db_if);

    p_rt_glb->pkt_len = nd_size + IPV6_HDR_LEN + ETH_HDR_LEN;
    
    return PM_E_NONE;
}

int32
nd_make_ns_unspecified_saddr (tbl_route_if_t *p_db_rtif,  struct sal_in6_addr *p_daddr, struct sal_in6_addr *p_target, tbl_route_global_t  *p_rt_glb)
{
    uint32                         *size = NULL;
    uint8                          *pnt = NULL;
    uint16                         nd_size = 0;
    prefix_t                       src;
    prefix_t                       dst;
    uint16                         icmpv6_cksum = 0;
    struct sal_in6_addr       saddr_unspecified;
    tbl_interface_t             *p_db_if = NULL;
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_target);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_memset(&src, 0, sizeof(prefix_t));
    sal_memset(&dst, 0, sizeof(prefix_t));
    sal_memset(&saddr_unspecified, 0, IPV6_MAX_BYTELEN);
    
    pnt = p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN;
    size = &(p_rt_glb->pkt_len);

    /* set src and dst */
    src.family = AF_INET6;
    sal_memcpy(&src.u.prefix6, &saddr_unspecified, IPV6_MAX_BYTELEN);
    dst.family = AF_INET6;
    sal_memcpy(&dst.u.prefix6, p_daddr, IPV6_MAX_BYTELEN);

    /* make ns header */
    /* type */
    ND_ENCODE_PUTC(SAL_ND_NEIGHBOR_SOLICIT) ;
    
    /* code */
    ND_ENCODE_PUTC(0) ;

    /* checksum */
    ND_ENCODE_PUTW(0) ;

    /* reserved */
    ND_ENCODE_PUTL(0) ;

    /* target address. */
    ND_ENCODE_PUT(p_target, IPV6_MAX_BYTELEN) ;

    /* fill in NS fields */
    nd_size = pnt - p_rt_glb->buf - IPV6_HDR_LEN - ETH_HDR_LEN;
    icmpv6_cksum = nd_in6_checksum(&src, &dst, IPPROTO_ICMPV6, (uint16*)(p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN), nd_size);

    /* add IPv6 header */
    pnt = p_rt_glb->buf + ETH_HDR_LEN;
    nd_make_ipv6_head(&pnt, size, nd_size, &saddr_unspecified, p_daddr, icmpv6_cksum);

    /* add l2 header */
    pnt = p_rt_glb->buf;
    nd_make_l2_head(&pnt, size, p_daddr, p_db_if);

    p_rt_glb->pkt_len = nd_size + IPV6_HDR_LEN + ETH_HDR_LEN;
    
    return PM_E_NONE;
}

int32
nd_make_na(tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_solicited_addr, uint8 router, uint8 solicited, uint8 override, uint8 inc_opt,
    struct sal_in6_addr *p_saddr, struct sal_in6_addr *p_daddr)
{
    tbl_route_global_t         *p_rt_glb = NULL;
    uint32                          *size = NULL;
    uint8                            *pnt = NULL;
    uint8                            flags = 0;
    tbl_interface_t               *p_db_if = NULL;
    uint16                           nd_size = 0;
    uint16                           icmpv6_cksum = 0;
    prefix_t                         src;
    prefix_t                         dst;
    char                              ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_solicited_addr);

    sal_memset(&src, 0, sizeof(prefix_t));
    sal_memset(&dst, 0, sizeof(prefix_t));

    p_rt_glb = tbl_route_global_get_route_global();
    if (!p_rt_glb)
    {
        return PM_E_FAIL;    
    }

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)p_db_rtif->key.name);
    if (!p_db_if)
    {
        ND_PKT_DEBUG("Can't find specified interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    /* set src and dst */
    src.family = AF_INET6;
    sal_memcpy(&src.u.prefix6, p_saddr, IPV6_MAX_BYTELEN);
    dst.family = AF_INET6;
    sal_memcpy(&dst.u.prefix6, p_daddr, IPV6_MAX_BYTELEN);

    pnt = p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN;
    size = &(p_rt_glb->pkt_len);

    /* make ns header */
    /* type */
    ND_ENCODE_PUTC(SAL_ND_NEIGHBOR_ADVERT) ;
    
    /* code */
    ND_ENCODE_PUTC(0) ;

    /* checksum */
    ND_ENCODE_PUTW(0) ;

    /* Flags. */
    if (router)
    {
        GLB_SET_FLAG (flags, SAL_ND_NA_FLAG_ROUTER);
    }
    
    if (solicited)
    {
        GLB_SET_FLAG (flags, SAL_ND_NA_FLAG_SOLICITED);
    }
    
    if (override)
    {
        GLB_SET_FLAG (flags, SAL_ND_NA_FLAG_OVERRIDE);
    }
    
    ND_ENCODE_PUTC(flags) ;

    /* reserved */
    ND_ENCODE_PUTC(0) ;
    ND_ENCODE_PUTW(0) ;

    /* target address. */
    ND_ENCODE_PUT(p_solicited_addr, IPV6_MAX_BYTELEN) ;

    if (inc_opt)
    {
         /* Source Link-layer address option */      
         /* Type. */
         ND_ENCODE_PUTC(SAL_ND_OPT_TARGET_LINKADDR);

        /* Length. */
        ND_ENCODE_PUTC((ND6_OPT_SPACE(MAC_ADDR_LEN)>>3));

        /* Link-Layer Address. */
        ND_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);
    }

    /* fill in NS fields */
    nd_size = pnt - p_rt_glb->buf - IPV6_HDR_LEN - ETH_HDR_LEN;
    icmpv6_cksum = nd_in6_checksum(&src, &dst, IPPROTO_ICMPV6, (uint16*)(p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN), nd_size);

    /* add IPv6 header */
    pnt = p_rt_glb->buf + ETH_HDR_LEN;
    nd_make_ipv6_head(&pnt, size, nd_size, p_saddr, p_daddr, icmpv6_cksum);

    /* add l2 header */
    pnt = p_rt_glb->buf;
    nd_make_l2_head(&pnt, size, p_daddr, p_db_if);

    p_rt_glb->pkt_len = nd_size + IPV6_HDR_LEN + ETH_HDR_LEN;

    return PM_E_NONE;
}

int32
nd_make_ra(tbl_route_global_t   *p_rt_glb, tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_saddr, struct sal_in6_addr *p_daddr, uint32 router_lifetime)
{
    uint32                         *size = NULL;
    uint8                          *pnt = NULL;
    tbl_interface_t             *p_db_if = NULL;
    uint16                         nd_size = 0;
    prefix_t                       src;
    prefix_t                       dst;
    uint16                         icmpv6_cksum = 0;
    uint8                           flags = 0;
    ctclib_slistnode_t         *rt_listnode = NULL;
    ds_nd_prefix_t             *p_nd_prefix = NULL;
    ds_connected_v6_t       *p_ifc  = NULL;
    ctclib_slistnode_t          *listnode   = NULL;

    ND_PTR_CHECK(p_rt_glb);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_saddr);
    ND_PTR_CHECK(p_daddr);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_memset(&src, 0, sizeof(prefix_t));
    sal_memset(&dst, 0, sizeof(prefix_t));

    pnt = p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN;
    size = &(p_rt_glb->pkt_len);

    /* set src and dst */
    src.family = AF_INET6;
    sal_memcpy(&src.u.prefix6, p_saddr, IPV6_MAX_BYTELEN);
    dst.family = AF_INET6;
    sal_memcpy(&dst.u.prefix6, p_daddr, IPV6_MAX_BYTELEN);

    /* make ra header */
    /* type */
    ND_ENCODE_PUTC(SAL_ND_ROUTER_ADVERT) ;
    
    /* code */
    ND_ENCODE_PUTC(0) ;

    /* checksum */
    ND_ENCODE_PUTW(0) ;

    if (router_lifetime)
    {
        /* Cur Hop Limit. */
        ND_ENCODE_PUTC(p_db_rtif->nd_ra_currrent_hop_limit);

        /* Flags. */
        if (p_db_rtif->nd_managed_config)
        {
            GLB_SET_FLAG (flags, SAL_ND_RA_FLAG_MANAGED);
        }

        if (p_db_rtif->nd_other_config)
        {
            GLB_SET_FLAG (flags, SAL_ND_RA_FLAG_OTHER);
        }
        
        ND_ENCODE_PUTC(flags);

        ND_ENCODE_PUTW(p_db_rtif->nd_router_lifetime);

        /* Reachable Time */
        if (GLB_ND_BASE_REACHABLE_TIME != p_db_rtif->nd_base_reachable_time)
        {
            ND_ENCODE_PUTL(p_db_rtif->nd_base_reachable_time);
        }
        else
        {
            ND_ENCODE_PUTL(GLB_ND_UNSPECIFIED);
        }

        /* Retrans Timer */
        if (GLB_ND_RETRANS_TIMER != p_db_rtif->nd_retransmit_timer)
        {
            ND_ENCODE_PUTL(p_db_rtif->nd_retransmit_timer);
        }
        else
        {
            ND_ENCODE_PUTL(GLB_ND_UNSPECIFIED);
        }
    }
    else
    {
        ND_ENCODE_PUTC(0) ;
        ND_ENCODE_PUTC(0) ;
        ND_ENCODE_PUTW(0) ;
        ND_ENCODE_PUTL(0);
        ND_ENCODE_PUTL(0);
    }

    /* Router Advertisement options. */
    if (router_lifetime)
    {
        /* Make Source Link-layer address option. */
        /* Type. */
        ND_ENCODE_PUTC(SAL_ND_OPT_SOURCE_LINKADDR);

        /* Length. */
        ND_ENCODE_PUTC((ND6_OPT_SPACE(MAC_ADDR_LEN)>>3));

        /* Link-Layer Address. */
        ND_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);

        if (p_db_rtif->nd_ra_advertise_mtu)
        {
            /* Type. */
            ND_ENCODE_PUTC(SAL_ND_OPT_MTU);
        
            /* Length. */
            ND_ENCODE_PUTC(1);
        
            /* reserved. */
            ND_ENCODE_PUTW(0);
        
            /* MTU. */
            ND_ENCODE_PUTL(p_db_if->mtu);    
        }

        if (p_db_rtif->nd_prefix.obj_list && p_db_rtif->nd_prefix.obj_list->count)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->nd_prefix.obj_list, p_nd_prefix, rt_listnode)
            {
                /* do not advertise prefix whose vaild time is 0 */
                if (0 == p_nd_prefix->vlifetime)
                {
                    continue;
                }

                nd_make_opt_pi(&pnt, size, p_db_rtif, p_nd_prefix, NULL);
            }
        }
        else
        {
            if (0 != p_db_rtif->nd_ra_prefix_default_vlifetime)
            {
                CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc, listnode)
                {
                    if (! IPV6_PREFIX_IS_UNSPECIFIED (&p_ifc->key.address)
                        && ! IPV6_PREFIX_IS_LOOPBACK  (&p_ifc->key.address)
                        && ! IPV6_PREFIX_IS_MULTICAST  (&p_ifc->key.address)
                        && ! IPV6_PREFIX_IS_LINK_LOCAL (&p_ifc->key.address))
                    {
                        nd_make_opt_pi(&pnt, size, p_db_rtif, NULL, p_ifc);
                    }
                }
            }
        }
    }

    /* fill in NS fields */
    nd_size = pnt - p_rt_glb->buf - IPV6_HDR_LEN - ETH_HDR_LEN;
    icmpv6_cksum = nd_in6_checksum(&src, &dst, IPPROTO_ICMPV6, (uint16*)(p_rt_glb->buf + IPV6_HDR_LEN + ETH_HDR_LEN), nd_size);

    /* add IPv6 header */
    pnt = p_rt_glb->buf + ETH_HDR_LEN;
    nd_make_ipv6_head(&pnt, size, nd_size, p_saddr, p_daddr, icmpv6_cksum);

    /* add l2 header */
    pnt = p_rt_glb->buf;
    nd_make_l2_head(&pnt, size, p_daddr, p_db_if);

    p_rt_glb->pkt_len = nd_size + IPV6_HDR_LEN + ETH_HDR_LEN;
    
    return PM_E_NONE;
}

int32 
nd_send_ns (tbl_route_if_t *p_db_rtif, addr_ipv6_t *p_target, uint32 unicast, uint32 unspecified)
{
    ds_connected_v6_t        *p_ifc = NULL;   
    struct sal_in6_addr        daddr;
    char                              buf_target[CMD_BUF_256];
    tbl_route_global_t          *p_rt_glb = NULL; 
    vid_t                             vid = 0;
    char                              ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    prefix_t                         prefix;
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_target);

    sal_memset(buf_target, 0, CMD_BUF_256);
    sal_memset(&prefix, 0, sizeof(prefix_t));
    
    p_rt_glb = tbl_route_global_get_route_global();
    if (!p_rt_glb)
    {
        return PM_E_FAIL;    
    }

    /* select destination IPv6 address */
    if (unicast)
    {
        sal_memcpy(&daddr, p_target, sizeof(struct sal_in6_addr));
    }
    else
    {
        nd_get_solicited_multicast(p_target, &daddr);
    }

    sal_memset(p_rt_glb->buf, 0, GLB_ND_PKT_BUF);

    cdb_addr_ipv6_val2str(buf_target, CMD_BUF_256, p_target);
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("Sending NS for %s on %s", buf_target, ifname_ext);

    if (!unspecified)
    {
        prefix.family = AF_INET6;
        prefix.prefixlen = 128;
        sal_memcpy(&(prefix.u.prefix6), p_target, sizeof(addr_ipv6_t));

        if (!IN6_IS_ADDR_LINKLOCAL(p_target))
        {
            p_ifc = if_match_ifc_ipv6_direct(p_db_rtif, &prefix);
            if (!p_ifc)
            {
                ND_EVENT_DEBUG("Send NS, but can't find any address in interface %s", ifname_ext);
                return PM_E_FAIL;
            }
        }
        else
        {
            p_ifc = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
            if (!p_ifc)
            {
                ND_EVENT_DEBUG("Send NS, but can't find any address in interface %s", ifname_ext);
                return PM_E_FAIL;
            }
        }

        nd_make_ns(p_db_rtif, &daddr, p_target, p_rt_glb, &p_ifc->key.address.u.prefix6);
    }
    else
    {
        nd_make_ns_unspecified_saddr(p_db_rtif, &daddr, p_target, p_rt_glb);
    }

    /* send nd pkt to specific interface */
    vid = tbl_interface_get_vid_by_ifindex(p_db_rtif->ifindex);
    if (ipc_pkt_tx_nd((uint8*)p_rt_glb->buf, p_rt_glb->pkt_len, p_db_rtif->ifindex, vid))
    {
        ND_EVENT_DEBUG("send ns through port %d fail", p_db_rtif->ifindex);
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

/* Send neighbor Advertisement packet. */
int32
nd_send_na (tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_daddr, 
    struct sal_in6_addr *p_solicited_addr, uint8 router, uint8 solicited, uint8 override, uint8 inc_opt)
{
    struct sal_in6_addr   src_addr;
    ds_connected_v6_t   *p_ifc = NULL;
    char                        buf[CMD_BUF_256];
    tbl_route_global_t    *p_rt_glb = NULL;
    vid_t                       vid = 0;
    char                        ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_daddr);
    ND_PTR_CHECK(p_solicited_addr);

    p_rt_glb = tbl_route_global_get_route_global();
    if (!p_rt_glb)
    {
        return PM_E_FAIL;    
    }

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, p_solicited_addr);
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("Sending NA for %s on %s", buf, ifname_ext);

    override = (override && inc_opt);

    /* na should use linklocal address for RFC 4291 modified by liwh for bug 49455, 2018-10-12  */
    /* for NA packet source address selection detailed information, please refer to RFC 3484;
        and kernel function ipv6_dev_get_saddr()called by ndisc_send_na() */
    if (IN6_IS_ADDR_LINKLOCAL(p_daddr))
    {
        p_ifc= if_lookup_ifc_ipv6_linklocal(p_db_rtif);
        if (!p_ifc)
        {
            ND_EVENT_DEBUG("Can not find linklocal address for %s", ifname_ext);
            return PM_E_NONE;
        }
        sal_memcpy(&src_addr, &(p_ifc->key.address.u.prefix6), sizeof(struct sal_in6_addr));
    }
    else
    {
        sal_memcpy(&src_addr, p_solicited_addr, sizeof(struct sal_in6_addr));
    }

    sal_memset(p_rt_glb->buf, 0, GLB_ND_PKT_BUF);
    
    nd_make_na(p_db_rtif, p_solicited_addr, router, solicited, override, inc_opt, &src_addr, p_daddr);

    /* send nd pkt to specific interface */
    vid = tbl_interface_get_vid_by_ifindex(p_db_rtif->ifindex);
    if (ipc_pkt_tx_nd((uint8*)p_rt_glb->buf, p_rt_glb->pkt_len, p_db_rtif->ifindex, vid))
    {
        ND_EVENT_DEBUG("send ns through port %d fail", p_db_rtif->ifindex);
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}


int32
nd_send_ra (tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_saddr, struct sal_in6_addr *p_daddr, uint32 router_lifetime)
{
    char                              buf_saddr[CMD_BUF_256];
    char                              buf_daddr[CMD_BUF_256];
    tbl_route_global_t          *p_rt_glb = NULL;
    vid_t                             vid = 0;
    char                              ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_saddr);
    ND_PTR_CHECK(p_daddr);

    p_rt_glb = tbl_route_global_get_route_global();
    if (!p_rt_glb)
    {
        return PM_E_FAIL;    
    }

    sal_memset(p_rt_glb->buf, 0, GLB_ND_PKT_BUF);
    sal_memset(buf_saddr, 0, CMD_BUF_256);
    sal_memset(buf_daddr, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf_saddr, CMD_BUF_256, p_saddr); 
    cdb_addr_ipv6_val2str(buf_daddr, CMD_BUF_256, p_daddr); 

    if (!GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_ENABLE))
    {
        return PM_E_NONE;
    }
    
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    if (router_lifetime)
    {
        ND_EVENT_DEBUG("Sending RA to %s on %s", buf_daddr, ifname_ext);
    }
    else
    {
        ND_EVENT_DEBUG("Sending Final RA on %s", ifname_ext);
    }

    nd_make_ra(p_rt_glb, p_db_rtif, p_saddr, p_daddr, router_lifetime);
    
    /* send nd pkt to specific interface */
    vid = tbl_interface_get_vid_by_ifindex(p_db_rtif->ifindex);
    if (ipc_pkt_tx_nd((uint8*)p_rt_glb->buf, p_rt_glb->pkt_len, p_db_rtif->ifindex, vid))
    {
        ND_EVENT_DEBUG("send ns through port %d fail", p_db_rtif->ifindex);
        return PM_E_FAIL;
    }

    p_db_rtif->nd_ra_ocount++;

    if (IN6_IS_ADDR_MULTICAST(p_daddr))
    {
        GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_SOLICITED_PENDING);
        p_db_rtif->nd_last_ra_unsolicited = ctc_time_boottime_msec(NULL);
    }
    else
    {
        p_db_rtif->nd_last_ra_solicited = ctc_time_boottime_msec(NULL);
        p_db_rtif->nd_ra_solicited_ocount++;
    }

    if (!router_lifetime)
    {
        usleep(15000);
    }
    
    return PM_E_NONE;
}

int32
nd_parse_options (uint8 *opt, uint32 opt_len, struct nd_options *ndopts)
{
    uint32   len = 0;
    struct nd_opt_hdr *p_nd_opt = NULL;
    uint32   left_opt_len = 0;

    p_nd_opt = (struct nd_opt_hdr *)opt;

    if (opt_len < 0)
    {
        return PM_E_FAIL;
    }

    sal_memset(ndopts, 0, sizeof(struct nd_options));

    left_opt_len = opt_len;
    while (left_opt_len) 
    {
        if (left_opt_len < sizeof(struct nd_opt_hdr))
        {
            return PM_E_FAIL;
        }

        len = p_nd_opt->nd_opt_len << ND_OPTION_LEN_SHIFT;
        if ((left_opt_len < len) || (0 == len))
        {
            return PM_E_FAIL;
        }

        switch (p_nd_opt->nd_opt_type) 
        {
            case ND6_OPT_SOURCE_LL_ADDR:
            case ND6_OPT_TARGET_LL_ADDR:
            case ND6_OPT_MTU:
            case ND6_OPT_REDIRECT_HDR:
                
                if (NULL != ndopts->nd_opt_array[p_nd_opt->nd_opt_type]) 
                {    
                    return PM_E_FAIL;
                }
                else 
                {
                    ndopts->nd_opt_array[p_nd_opt->nd_opt_type] = (uint8*)p_nd_opt;
                }
                break;

            case ND6_OPT_PREFIX_INFO:
                
                ndopts->nd_opt_array[ND6_OPT_PREFIX_INFO_END] = (uint8*)p_nd_opt;
                if (NULL == ndopts->nd_opt_array[p_nd_opt->nd_opt_type])
                {
                    ndopts->nd_opt_array[p_nd_opt->nd_opt_type] = (uint8*)p_nd_opt;
                }
                break;

            default:
                /*
                * Unknown options must be silently ignored,
                * to accommodate future extension to the protocol.
                */
                break;
        }

        left_opt_len -= len;
        p_nd_opt = ((void*)p_nd_opt) + len;
    }

    return PM_E_NONE;
}

uint8* 
nd_opt_addr_data (struct nd_opt_hdr *p)
{
    uint8         *lladdr = (uint8 *)(p + 1);
    uint8         lladdrlen;

    lladdrlen = p->nd_opt_len << ND_OPTION_LEN_SHIFT;
    
    if (lladdrlen != ND6_OPT_SPACE(GLB_ETH_ADDR_LEN))
    {
        return NULL;
    }

    if (GLB_IS_BCAST_MAC(lladdr) || GLB_IS_ZERO_MAC(lladdr) || GLB_IS_MCAST_MAC(lladdr))
    {
        return NULL;
    }

    return lladdr;
}

int32
nd_print_pkt(uint8 *p_pkt_buf, uint32 len)
{
    uint32  byte_index = 0;
    char     pktstr[256] = "\0";
    char     nd_tmp[256];

    for (byte_index = 0; byte_index < len; byte_index++)
    {
        if ((byte_index % 18) == 0)
        {
            if (byte_index != 0)
            {
                 ND_PKT_DEBUG("%s\n", pktstr);
                 sal_memset(pktstr, 0, 256);
            }

            sal_sprintf(nd_tmp, "%04x: ", (byte_index /18));
            sal_strncat(pktstr, nd_tmp, sal_strlen(nd_tmp));

            sal_sprintf(nd_tmp, "%02x", p_pkt_buf[byte_index]);
            sal_strncat(pktstr, nd_tmp, sal_strlen(nd_tmp));
        }
        else if ((byte_index % 2) == 0)
        {
            sal_sprintf(nd_tmp, "%02x", p_pkt_buf[byte_index]);
            sal_strncat(pktstr, nd_tmp, sal_strlen(nd_tmp));
        }
        else if ((byte_index % 2) == 1)
        {
            sal_sprintf(nd_tmp, "%02x ", p_pkt_buf[byte_index]);
            sal_strncat(pktstr, nd_tmp, sal_strlen(nd_tmp));
        }
    }

    if ((byte_index % 18) != 0)
    {
        ND_PKT_DEBUG("%s\n", pktstr);   
    }
    
    return PM_E_NONE;
}


int32
nd_recv_ns(uint8  *p_rcv_buf, uint32 pkt_len, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, tbl_nd_stats_t *p_nd_stats)
{
    struct icmp6_neighbor_solicit    *p_icmp_ns = NULL;
    struct ipv6hdr                         *p_ipv6_hdr = NULL;
    char                                       buf_target[CMD_BUF_256];    
    char                                       buf_saddr[CMD_BUF_256];
    char                                       buf_daddr[CMD_BUF_256];
    uint32                                    len = 0;
    ds_connected_v6_t                  *p_ifc = NULL;
    uint32                                    inc_lladdr = 0;
    struct sal_in6_addr                 allnode;
    prefix_t                                  prefix;
    struct nd_opt_src_link_addr   *p_nd_opt = NULL;
    char                                       ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    uint32                                    opt_len = 0;
    struct  nd_options                  ndopts;
    uint8                                      *p_ll_addr = NULL;
    
    ND_PTR_CHECK(p_rcv_buf);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);
    ND_PTR_CHECK(p_nd_stats);

    sal_memset(buf_target, 0, CMD_BUF_256);
    sal_memset(buf_saddr, 0, CMD_BUF_256);
    sal_memset(buf_daddr, 0, CMD_BUF_256);
    sal_memset(&prefix, 0, sizeof(prefix_t));

    p_ipv6_hdr = (struct ipv6hdr*)(p_rcv_buf + ETH_HDR_LEN);
    p_icmp_ns = (struct icmp6_neighbor_solicit*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN);
    p_nd_opt  = (struct nd_opt_src_link_addr*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN + sizeof(struct icmp6_neighbor_solicit));

    cdb_addr_ipv6_val2str(buf_target, CMD_BUF_256, &p_icmp_ns->nd_ns_target);
    cdb_addr_ipv6_val2str(buf_saddr, CMD_BUF_256, &p_ipv6_hdr->saddr); 
    cdb_addr_ipv6_val2str(buf_daddr, CMD_BUF_256, &p_ipv6_hdr->daddr); 

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_PKT_DEBUG("Received NS for %s on %s from %s", buf_target, ifname_ext, buf_saddr);
    nd_print_pkt((uint8*)p_icmp_ns, (pkt_len -IPV6_HDR_LEN - ETH_HDR_LEN));
    
    if(ADDR_IS_ALLNODE(&p_ipv6_hdr->daddr) || ADDR_IS_ALLROUTE(&p_ipv6_hdr->daddr))
    {
        ND_EVENT_DEBUG("Received NS packet from %s to %s with invalid destination, ifindex:%d", 
            buf_saddr, buf_daddr, p_db_rtif->ifindex);
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    if (!IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr))
    {
        /* check if source is same subnet */
        prefix.family = AF_INET6;
        prefix.prefixlen = IPV6_MAX_BITLEN;
        sal_memcpy(&(prefix.u.prefix6), &p_ipv6_hdr->saddr, sizeof(addr_ipv6_t));
        if (if_match_active_ifc_ipv6_num(p_db_rtif, &prefix) == 0)
        {
            ND_EVENT_DEBUG("Received NS packet from %s to %s with unknown source, ifindex:%d", 
                buf_saddr, buf_daddr, p_db_rtif->ifindex);
            p_nd_stats->rcv_error++;
            return PM_E_FAIL;
        }
    }

    /* ICMP message length check. */
    len = pkt_len - IPV6_HDR_LEN - ETH_HDR_LEN;
    if (len < sizeof(struct icmp6_neighbor_solicit))
    {
        ND_EVENT_DEBUG("Received too short NS packet, length = %d", len);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    if (ND_HDR_HOP_LIMIT != p_ipv6_hdr->hop_limit)
    {
        ND_EVENT_DEBUG("Received NS packet with invalid hop-limit: %d", p_ipv6_hdr->hop_limit);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check ICMP code */    
    if (0 != p_icmp_ns->nd_ns_hdr.icmp6_code) 
    {
        ND_EVENT_DEBUG("Received NS packet with invalid ICMP6 code: %d", 
            p_icmp_ns->nd_ns_hdr.icmp6_code);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* target must be unicast */
    if (IN6_IS_ADDR_MULTICAST(&(p_icmp_ns->nd_ns_target)))
    {
        ND_EVENT_DEBUG("Received NS packet with mulitcast target address: %s", 
            buf_target);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* RFC2461 7.1.1: DAD has to be destined for solicited node multicast address. */
    if (IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr) && 
        !ADDR_IS_SOLICITED(&p_ipv6_hdr->daddr)) 
    {
        ND_EVENT_DEBUG("Received NS packet with unspecified source address %s and non-solicited node"
            " multicast destination address: %s", buf_saddr, buf_daddr);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    opt_len = pkt_len - (sizeof (struct icmp6_neighbor_solicit) + IPV6_HDR_LEN + ETH_HDR_LEN);
    if (PM_E_FAIL == nd_parse_options ((uint8*)p_nd_opt, opt_len, &ndopts))
    {
        ND_EVENT_DEBUG("Received NS packet with invalid option");
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    if (ndopts.nd_opt_array[ND6_OPT_TARGET_LL_ADDR])
    {
         ND_EVENT_DEBUG("Received NS packet with target linklayer option");
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;   
    }

    /* src mac check */
    if (ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR])
    {
        p_ll_addr = nd_opt_addr_data((struct nd_opt_hdr*)ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR]);
        if (!p_ll_addr)
        {
            ND_EVENT_DEBUG("Received NS packet with invalid source link-address option");
            
            p_nd_stats->rcv_invalid++;
            return PM_E_FAIL;
        }

        if (IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr)) 
        {
            ND_EVENT_DEBUG("Received NS packet with unspecified source address %s and source "
                "link-layer address option", buf_saddr);
            
            p_nd_stats->rcv_invalid++;
            return PM_E_FAIL;
        }
    }

    /* DAD check */
    sal_memcpy(&(prefix.u.prefix6), &p_icmp_ns->nd_ns_target, sizeof(addr_ipv6_t));
    p_ifc = connected_check_ipv6(p_db_rtif, &prefix);
    if (p_ifc)
    {
        if (GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS))
        {
            /* Address is tentative. If the source
                is unspecified address, it is someone
                does DAD, otherwise we ignore solicitations
                until DAD timer expires.
            */
            if (IN6_IS_ADDR_UNSPECIFIED(&(p_ipv6_hdr->saddr)))
            {
                nd_dad_failed(p_ifc, p_db_rtif);
            }

            return PM_E_FAIL; 
        }
        /* modified by liwh for bug 48683, 2018-08-24 */
        else  if (GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDR_DUPLICATE))
        {
            ND_EVENT_DEBUG("Received NS packet with duplicate destination address %s", buf_daddr);
            p_nd_stats->rcv_invalid++;
        
            return PM_E_FAIL;     
        }
        /* liwh end */
        /* modified by liwh for bug 49316, 2018-10-11 */
        if (!GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_REAL))
        {
            ND_EVENT_DEBUG("Received NS packet with ten destination address %s", buf_daddr);
            p_nd_stats->rcv_invalid++;
        
            return PM_E_FAIL;         
        }
        /* liwh end */
    }
    else
    {
        /* We've got an NS packet, and we don't have that adddress assigned 
            for us.  We MUST silently ignore it. See RFC2461 7.2.3 */
        ND_EVENT_DEBUG("Received NS packet target to other IPv6 address: %s", buf_target);
        return PM_E_FAIL;   
    }

    inc_lladdr = IN6_IS_ADDR_MULTICAST(&p_ipv6_hdr->daddr) ;
    
    /*
    * If the source address is unspecified address, entries must not
    * be created or updated.
    * It looks that sender is performing DAD.  Output NA toward
    * all-node multicast address, to tell the sender that I'm using
    * the address.
    * S bit ("solicited") must be zero.
    */
    if (IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr))
    {
        ND_EVENT_DEBUG("Someone is performing DAD. Send NA packet to tell it duplicate case");
        sal_inet_pton (AF_INET6, ALLNODE, &allnode);
        nd_send_na(p_db_rtif, &allnode, &(p_icmp_ns->nd_ns_target), 
            TRUE, FALSE, TRUE, inc_lladdr);
        return PM_E_NONE;
    }

    if ((pkt_len > (sizeof(struct icmp6_neighbor_solicit) + IPV6_HDR_LEN + ETH_HDR_LEN))
        && (SAL_ND_OPT_SOURCE_LINKADDR ==p_nd_opt->type))
    {
        /* add neigh to kernel in order to send NA */
        tbl_nd_fib_t              *p_nd_fib = NULL;
        tbl_nd_fib_t              nd_fib;

        sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));

        nd_fib_make_key(&nd_fib.key, &p_ipv6_hdr->saddr, p_db_if);
        p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
        if (!p_nd_fib)
        {
            p_nd_fib = nd_neigh_create(&nd_fib, p_db_if);
            if (!p_nd_fib)
            {
                return PM_E_FAIL;
            }

            sal_memcpy(p_nd_fib->mac_addr, p_ll_addr, GLB_ETH_ADDR_LEN);
            p_nd_fib->state = NES_INCOMPLETE;
        }
        nd_neigh_update_other(p_nd_fib, (char*)p_ll_addr, p_db_rtif, p_db_if, SAL_ND_NEIGHBOR_SOLICIT);  
    }
    else
    {
        ND_EVENT_DEBUG("Received NS packet whitout source link-address option");
    }
    
    nd_send_na(p_db_rtif, &p_ipv6_hdr->saddr, &(p_icmp_ns->nd_ns_target), 
        TRUE, TRUE, TRUE, inc_lladdr);
    
    return PM_E_NONE;
}

int32
nd_recv_na(uint8  *p_rcv_buf, uint32 pkt_len, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, tbl_nd_stats_t *p_nd_stats)
{
    struct icmp6_neighbor_advert    *p_icmp_na = NULL;
    struct ipv6hdr                          *p_ipv6_hdr = NULL;
    char                                        buf_target[CMD_BUF_256];    
    char                                        buf_saddr[CMD_BUF_256];
    char                                        buf_daddr[CMD_BUF_256];
    tbl_nd_fib_t                             *p_nd_fib = NULL;
    tbl_nd_fib_t                             nd_fib;
    struct nd_opt_target_link_addr  *p_nd_opt = NULL;
    ds_connected_v6_t                   *p_ifc = NULL;
    prefix_t                                    prefix;
    uint8                                       router = 0;
    uint8                                       solicited = 0;
    uint8                                       override = 0;
    char                                         ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    struct sal_in6_addr                  allnode;
 
    ND_PTR_CHECK(p_rcv_buf);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);
    ND_PTR_CHECK(p_nd_stats);

    sal_memset(buf_target, 0, CMD_BUF_256);
    sal_memset(buf_saddr, 0, CMD_BUF_256);
    sal_memset(buf_daddr, 0, CMD_BUF_256);
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(&prefix, 0, sizeof(prefix_t));

    p_ipv6_hdr = (struct ipv6hdr*)(p_rcv_buf + ETH_HDR_LEN);
    p_icmp_na = (struct icmp6_neighbor_advert*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN);
    p_nd_opt  = (struct nd_opt_target_link_addr*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN + sizeof(struct icmp6_neighbor_advert));

    cdb_addr_ipv6_val2str(buf_target, CMD_BUF_256, &p_icmp_na->nd_na_target);
    cdb_addr_ipv6_val2str(buf_saddr, CMD_BUF_256, &p_ipv6_hdr->saddr); 
    cdb_addr_ipv6_val2str(buf_daddr, CMD_BUF_256, &p_ipv6_hdr->daddr); 

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_PKT_DEBUG("Received NA for %s on %s from %s", buf_target, ifname_ext, buf_saddr);
    nd_print_pkt((uint8*)p_icmp_na, (pkt_len -IPV6_HDR_LEN - ETH_HDR_LEN));

    if(ADDR_IS_SOLICITED(&p_ipv6_hdr->daddr) || ADDR_IS_ALLROUTE(&p_ipv6_hdr->daddr))
    {
        ND_PKT_DEBUG("Received NA packet from %s to %s with invalid destination, ifindex:%d", 
            buf_saddr, buf_daddr, p_db_rtif->ifindex);
        p_nd_stats->rcv_error++;
        return PM_E_NONE;
    }

    /* ICMP message length check. */
    if (pkt_len < (sizeof (struct icmp6_neighbor_advert) + IPV6_HDR_LEN + ETH_HDR_LEN))
    {
        ND_PKT_DEBUG("Received too short NA packet, length = %d", pkt_len);
        p_nd_stats->rcv_invalid++;
        return PM_E_NONE;
    }

    if (ND_HDR_HOP_LIMIT != p_ipv6_hdr->hop_limit)
    {
        ND_EVENT_DEBUG("Received NS packet with invalid hop-limit: %d", p_ipv6_hdr->hop_limit);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check ICMP code */    
    if (0 != p_icmp_na->nd_na_hdr.icmp6_code) 
    {
        ND_EVENT_DEBUG("Received NA packet with invalid ICMP6 code: %d", 
            p_icmp_na->nd_na_hdr.icmp6_code);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check target IPv6 address. */
    if (IN6_IS_ADDR_MULTICAST(&(p_icmp_na->nd_na_target)))
    {
        ND_EVENT_DEBUG("Received NA packet with mulitcast target address: %s", 
            buf_target);
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    if (pkt_len >  (sizeof(struct icmp6_neighbor_advert) + IPV6_HDR_LEN + ETH_HDR_LEN))
    {
        if ((SAL_ND_OPT_TARGET_LINKADDR !=p_nd_opt->type)
            || ((ND6_OPT_SPACE(MAC_ADDR_LEN)>>3) != p_nd_opt->length)
            || (GLB_IS_BCAST_MAC(p_nd_opt->lladdr) || GLB_IS_ZERO_MAC(p_nd_opt->lladdr) || GLB_IS_MCAST_MAC(p_nd_opt->lladdr)))
         {
            ND_EVENT_DEBUG("Received NA packet with invalid option");
            p_nd_stats->rcv_invalid++;
            return PM_E_FAIL;
        }
    }

    /* modified by liwh for bug 49334, 2018-10-11 */
    router = p_icmp_na->nd_na_hdr.un_data8[0] & SAL_ND_NA_FLAG_ROUTER;
    solicited = p_icmp_na->nd_na_hdr.un_data8[0] & SAL_ND_NA_FLAG_SOLICITED;
    override = p_icmp_na->nd_na_hdr.un_data8[0] & SAL_ND_NA_FLAG_OVERRIDE;
    
    sal_inet_pton (AF_INET6, ALLNODE, &allnode);
    if (0 == sal_memcmp(&allnode, &p_ipv6_hdr->daddr, sizeof(struct sal_in6_addr)))
    {
        if (solicited)
        {
            ND_EVENT_DEBUG("Received NA packet with multicast address %s, but solicited is 1", buf_daddr);
            p_nd_stats->rcv_invalid++;
            return PM_E_NONE;   
        }
    }
    /* liwh end */

    sal_memcpy(&(prefix.u.prefix6), &p_icmp_na->nd_na_target, sizeof(addr_ipv6_t));
    p_ifc = connected_check_ipv6(p_db_rtif, &prefix);
    if (p_ifc)
    {
        if (GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS))
        {
            nd_dad_failed(p_ifc, p_db_rtif);
            return PM_E_FAIL; 
        }
        else
        {
            ND_EVENT_DEBUG("ICMPv6 NA: someone advertises our address on %s", ifname_ext);
            return PM_E_FAIL;   
        }
    }

    /* modified by liwh for bug 48676, 2018-08-24 */
    /* check if na target address is same subnet with interface */
    prefix.family = AF_INET6;
    prefix.prefixlen = IPV6_MAX_BITLEN;
    if (!IPV6_PREFIX_IS_LINK_LOCAL(&prefix))
    {
        if (if_match_active_ifc_ipv6_num(p_db_rtif, &prefix) == 0)
        {
            ND_EVENT_DEBUG("Received NA packet from %s to %s without matching address, ifindex:%d", 
                buf_saddr, buf_daddr, p_db_rtif->ifindex);
            p_nd_stats->rcv_error++;
            return PM_E_FAIL;
        }
    }
    /* liwh end */

    nd_fib_make_key(&nd_fib.key, &p_icmp_na->nd_na_target, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
    if (!p_nd_fib)
    {
        ND_EVENT_DEBUG("Received NA packet with source address %s, but no responding neigh cache entry", buf_target);
        p_nd_stats->rcv_error++;
        return PM_E_NONE;
    }
    
    if (pkt_len >  (sizeof(struct icmp6_neighbor_advert) + IPV6_HDR_LEN + ETH_HDR_LEN))
    {
        nd_neigh_update_na(p_nd_fib, (char*)p_nd_opt->lladdr, p_db_rtif, p_db_if, router, solicited, override);
    }
    else
    {
        nd_neigh_update_na(p_nd_fib, NULL, p_db_rtif, p_db_if, router, solicited, override);    
    }

    return PM_E_NONE;
}

int32
nd_recv_ra(uint8  *p_rcv_buf, uint32 pkt_len, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, tbl_nd_stats_t *p_nd_stats)
{
    struct icmp6_router_advert    *p_icmp_ra = NULL;
    struct ipv6hdr                      *p_ipv6_hdr = NULL;
    char                                    buf_saddr[CMD_BUF_256];
    char                                    buf_daddr[CMD_BUF_256];
    char                                    buf_prefix[CMD_BUF_256];        
    ds_connected_v6_t               *p_ifc = NULL;
    prefix_t                               prefix;
    uint8                                  *p_nd_opt = NULL;
    tbl_route_global_t                *p_route_glb = NULL;
    tbl_nd_fib_t                        nd_fib;
    tbl_nd_fib_t                        *p_nd_fib = NULL;
    uint32                                opt_len = 0;
    struct  nd_options               ndopts;
    uint8                                  *p_ll_addr = NULL; 
    uint32                                mtu = 0;
    struct nd_opt_prefix_info     *pi = NULL;
    ds_nd_prefix_t                    *p_nd_prefix = NULL;
    ds_nd_prefix_t                    nd_prefix;
    uint32                                plifetime = 0;
    uint32                                vlifetime = 0;
    char                                    ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_rcv_buf);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);
    ND_PTR_CHECK(p_nd_stats);

    sal_memset(buf_prefix, 0, CMD_BUF_256);
    sal_memset(buf_saddr, 0, CMD_BUF_256);
    sal_memset(buf_daddr, 0, CMD_BUF_256);
    sal_memset(&prefix, 0, sizeof(prefix_t));
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(&nd_prefix, 0, sizeof(ds_nd_prefix_t));

    p_route_glb = tbl_route_global_get_route_global();
    if (!p_route_glb)
    {
        return PM_E_FAIL;        
    }
    
    p_ipv6_hdr = (struct ipv6hdr*)(p_rcv_buf + ETH_HDR_LEN);
    p_icmp_ra = (struct icmp6_router_advert*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN);
    p_nd_opt  = (uint8*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN + sizeof(struct icmp6_router_advert));

    cdb_addr_ipv6_val2str(buf_saddr, CMD_BUF_256, &p_ipv6_hdr->saddr); 
    cdb_addr_ipv6_val2str(buf_daddr, CMD_BUF_256, &p_ipv6_hdr->daddr); 

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_PKT_DEBUG("Recieved RA from %s on %s", buf_saddr, ifname_ext);
    nd_print_pkt((uint8*)p_icmp_ra, (pkt_len -IPV6_HDR_LEN - ETH_HDR_LEN));

    if(ADDR_IS_SOLICITED(&p_ipv6_hdr->daddr) || ADDR_IS_ALLROUTE(&p_ipv6_hdr->daddr))
    {
        ND_EVENT_DEBUG("Received RA packet from %s to %s with invalid destination, ifindex:%d", 
            buf_saddr, buf_daddr, p_db_rtif->ifindex);
        
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    /* Check whether RA enabled */
    if (!p_db_rtif->nd_ra_advertise)
    {
        return PM_E_FAIL;
    }

    /* Drop the packet which comes from myself. */
    sal_memcpy(&prefix.u.prefix6, &p_ipv6_hdr->saddr, sizeof(addr_ipv6_t));    
    if (connected_check_ipv6 (p_db_rtif,  &prefix))
    {
        ND_EVENT_DEBUG("Received RA packet coming from myself");
        
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    /* ICMP message length check. */
    if (pkt_len < (sizeof (struct icmp6_router_advert) + IPV6_HDR_LEN + ETH_HDR_LEN))
    {
        ND_PKT_DEBUG("Received too short RA packet, length = %d", pkt_len);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_NONE;
    }
   
    /* Get IPv6 hdr hop limit */
    if (ND_HDR_HOP_LIMIT != p_ipv6_hdr->hop_limit)
    {
        ND_EVENT_DEBUG("Received RA packet with invalid hop-limit: %d", p_ipv6_hdr->hop_limit);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check ICMPv6 Code */
    if (0 != p_icmp_ra->icmp6_code) 
    {
        ND_EVENT_DEBUG("Received RA packet with invalid ICMP6 code: %d", p_icmp_ra->icmp6_code);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check source IPv6 address */
    if (!IN6_IS_ADDR_LINKLOCAL(&p_ipv6_hdr->saddr))
    {
        ND_EVENT_DEBUG("Received RA packet, source address is not link-local: %s", buf_saddr);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* hop limit check */
    if ((GLB_ND_UNSPECIFIED != p_icmp_ra->curhoplimit) && 
        (p_route_glb->nd_hop_limit != p_icmp_ra->curhoplimit))
    {
        ND_EVENT_DEBUG("Received RA packet with hop limit %d, it is not consistent with me", 
            p_icmp_ra->curhoplimit);
        
        p_nd_stats->rcv_inconsistent++;
        return PM_E_FAIL;
    }

    /* Managed address configuration flag check */
    if ((GLB_FLAG_ISSET(p_icmp_ra->flags, SAL_ND_RA_FLAG_MANAGED) && !p_db_rtif->nd_managed_config)
        ||((!GLB_FLAG_ISSET(p_icmp_ra->flags, SAL_ND_RA_FLAG_MANAGED)) && p_db_rtif->nd_managed_config))
    {
        ND_EVENT_DEBUG("Received RA packet with managed address configuration flag %s, it is not consistent with me", 
            (p_icmp_ra->flags & SAL_ND_RA_FLAG_MANAGED) ? "set" : "unset");
        
        p_nd_stats->rcv_inconsistent++;
        return PM_E_FAIL;
    }

    /* Other address configuration flag check */
    if ((GLB_FLAG_ISSET(p_icmp_ra->flags, SAL_ND_RA_FLAG_OTHER) && !p_db_rtif->nd_other_config)
        ||((!GLB_FLAG_ISSET(p_icmp_ra->flags, SAL_ND_RA_FLAG_OTHER)) && p_db_rtif->nd_other_config))
    {
        ND_EVENT_DEBUG("Received RA packet with other address configuration flag %s, it is not consistent with me", 
            (p_icmp_ra->flags & SAL_ND_RA_FLAG_OTHER) ? "set" : "unset");
        
        p_nd_stats->rcv_inconsistent++;
        return PM_E_FAIL;
    }

    opt_len = pkt_len - (sizeof (struct icmp6_router_advert) + IPV6_HDR_LEN + ETH_HDR_LEN);
    if (PM_E_FAIL == nd_parse_options (p_nd_opt, opt_len, &ndopts))
    {
        ND_EVENT_DEBUG("Received RA packet with invalid option");
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* src mac check */
    if (ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR])
    {
        p_ll_addr = nd_opt_addr_data((struct nd_opt_hdr*)ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR]);
        if (!p_ll_addr)
        {
            ND_EVENT_DEBUG("Received RA packet with invalid source link-address option");
            
            p_nd_stats->rcv_invalid++;
            return PM_E_FAIL;
        }
    }

    /* MTU check */
    if (ndopts.nd_opt_array[ND6_OPT_MTU])
    {
        sal_memcpy (&mtu, ndopts.nd_opt_array[ND6_OPT_MTU]+4, sizeof(mtu));
        mtu = sal_ntohl(mtu);

        if (mtu < GLB_IPV6_MIN_MTU)
        {
            ND_EVENT_DEBUG("Received RA packet with MTU value is too short :%d", mtu);
            
            p_nd_stats->rcv_inconsistent++;
            return PM_E_FAIL;
        }
        else if (mtu != p_db_if->mtu)
        {
            ND_EVENT_DEBUG("Received RA packet with MTU value %d, it is not consistent with me", mtu);
            
            p_nd_stats->rcv_inconsistent++;
            return PM_E_FAIL;
        }
    }
    
    /* prefix check */
    nd_prefix.key.addr.family = AF_INET6;
    for (p_nd_opt = ndopts.nd_opt_array[ND6_OPT_PREFIX_INFO]; 
        p_nd_opt && (p_nd_opt <= ndopts.nd_opt_array[ND6_OPT_PREFIX_INFO_END]); 
        p_nd_opt = p_nd_opt + sizeof(struct nd_opt_prefix_info))
    {
        pi = (struct nd_opt_prefix_info *)p_nd_opt;
        nd_prefix.key.addr.prefixlen = pi->nd_opt_pi_prefix_len;
        sal_memcpy (&nd_prefix.key.addr.u.prefix6, &pi->nd_opt_pi_prefix, sizeof(addr_ipv6_t));
        nd_prefix.key.ifindex = p_db_rtif->ifindex;
        
        if (p_db_rtif->nd_prefix.obj_list && p_db_rtif->nd_prefix.obj_list->count)
        {
            p_nd_prefix = ds_nd_prefix_get_nd_prefix(p_db_rtif, &nd_prefix);
            if (!p_nd_prefix)
            {
                continue;
            }

            plifetime  = p_nd_prefix->plifetime;
            vlifetime  = p_nd_prefix->vlifetime;
        }
        else
        {
            p_ifc = connected_check_ipv6 (p_db_rtif, &nd_prefix.key.addr);
            if (p_ifc)
            {
                continue;
            }

            plifetime  = GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME;
            vlifetime  = GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME;
        }

        cdb_addr_ipv6_val2str(buf_prefix, CMD_BUF_256, &pi->nd_opt_pi_prefix); 
        if (vlifetime != sal_ntohl(pi->nd_opt_pi_valid_time))
        {
            ND_EVENT_DEBUG("Received RA packet with prefix %s/%d, valid lifetime is not consistent with me", 
                buf_prefix, pi->nd_opt_pi_prefix_len);
            
            p_nd_stats->rcv_inconsistent++;
            return PM_E_FAIL;
        }
        else if (plifetime != sal_ntohl(pi->nd_opt_pi_preferred_time))
        {
            ND_EVENT_DEBUG("Received RA packet with prefix %s/%d, prefered lifetime is not consistent with me", 
                buf_prefix, pi->nd_opt_pi_prefix_len);
            
            p_nd_stats->rcv_inconsistent++;
            return PM_E_FAIL;
        }
    }

    /* neigh cache entry update */
    nd_fib_make_key(&nd_fib.key, &p_ipv6_hdr->saddr, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
    if (p_nd_fib)
    {
        nd_neigh_update_other(p_nd_fib, (char*)p_ll_addr, p_db_rtif, p_db_if, SAL_ND_ROUTER_ADVERT);
    }

    return PM_E_NONE;
}

int32
nd_recv_rs(uint8  *p_rcv_buf, uint32 pkt_len, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, tbl_nd_stats_t *p_nd_stats)
{
    struct icmp6_router_solicit    *p_icmp_rs = NULL;
    struct ipv6hdr                      *p_ipv6_hdr = NULL;
    char                                    buf_saddr[CMD_BUF_256];
    char                                    buf_daddr[CMD_BUF_256];
    prefix_t                               prefix;
    uint8                                  *p_nd_opt = NULL;
    tbl_route_global_t                *p_route_glb = NULL;
    tbl_nd_fib_t                        nd_fib;
    tbl_nd_fib_t                        *p_nd_fib = NULL;
    uint32                                opt_len = 0;
    struct  nd_options               ndopts;
    uint8                                  *p_ll_addr = NULL;
    uint32                                interval = 0;
    uint32                                remained = 0;
    char                                    ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
     
    ND_PTR_CHECK(p_rcv_buf);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);
    ND_PTR_CHECK(p_nd_stats);

    sal_memset(buf_saddr, 0, CMD_BUF_256);
    sal_memset(buf_daddr, 0, CMD_BUF_256);
    sal_memset(&prefix, 0, sizeof(prefix_t));
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(&ndopts, 0, sizeof(struct  nd_options));
    
    p_route_glb = tbl_route_global_get_route_global();
    if (!p_route_glb)
    {
        return PM_E_FAIL;        
    }
    
    p_ipv6_hdr = (struct ipv6hdr*)(p_rcv_buf + ETH_HDR_LEN);
    p_icmp_rs = (struct icmp6_router_solicit*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN);
    p_nd_opt  = (uint8*)(p_rcv_buf + IPV6_HDR_LEN + ETH_HDR_LEN + sizeof(struct icmp6_router_solicit));

    cdb_addr_ipv6_val2str(buf_saddr, CMD_BUF_256, &p_ipv6_hdr->saddr); 
    cdb_addr_ipv6_val2str(buf_daddr, CMD_BUF_256, &p_ipv6_hdr->daddr); 

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_PKT_DEBUG("Recieved RS from %s on %s", buf_saddr, ifname_ext);
    nd_print_pkt((uint8*)p_icmp_rs, (pkt_len -IPV6_HDR_LEN - ETH_HDR_LEN));

    if(ADDR_IS_SOLICITED(&p_ipv6_hdr->daddr) || ADDR_IS_ALLNODE(&p_ipv6_hdr->daddr))
    {
        ND_EVENT_DEBUG("Received RS packet from %s to %s with invalid destination, ifindex:%d", 
            buf_saddr, buf_daddr, p_db_rtif->ifindex);
        
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    if(!IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr))
    {
        /* check if source is same subnet */
        prefix.family = AF_INET6;
        prefix.prefixlen = IPV6_MAX_BITLEN;
        sal_memcpy(&prefix.u.prefix6, &p_ipv6_hdr->saddr, sizeof (struct sal_in6_addr));
        if (if_match_active_ifc_ipv6_num(p_db_rtif, &prefix) == 0)
        {
            ND_EVENT_DEBUG("Received RS packet from %s to %s with unknown source, ifindex:%d", 
                buf_saddr, buf_daddr, p_db_rtif->ifindex);
            
            p_nd_stats->rcv_error++;
            return PM_E_FAIL;
        }
    }

    /* Check whether RA enabled */
    if (!p_db_rtif->nd_ra_advertise)
    {
        return PM_E_FAIL;
    }

    /* Drop the packet which comes from myself. */
    sal_memcpy(&prefix.u.prefix6, &p_ipv6_hdr->saddr, sizeof(addr_ipv6_t));    
    if (connected_check_ipv6 (p_db_rtif,  &prefix))
    {
        ND_EVENT_DEBUG("Received RS packet coming from myself");
        
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    /* ICMP message length check. */
    if (pkt_len < (sizeof (struct icmp6_router_solicit) + IPV6_HDR_LEN + ETH_HDR_LEN))
    {
        ND_PKT_DEBUG("Received too short RS packet, length = %d", pkt_len);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_NONE;
    }

    /* Get IPv6 hdr hop limit */
    if (ND_HDR_HOP_LIMIT != p_ipv6_hdr->hop_limit)
    {
        ND_EVENT_DEBUG("Received RS packet with invalid hop-limit: %d", p_ipv6_hdr->hop_limit);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* Check ICMPv6 Code */
    if (0 != p_icmp_rs->icmp6_code) 
    {
        ND_EVENT_DEBUG("Received RS packet with invalid ICMP6 code: %d", p_icmp_rs->icmp6_code);
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    opt_len = pkt_len - (sizeof (struct icmp6_router_solicit) + IPV6_HDR_LEN + ETH_HDR_LEN);
    if ((0 != opt_len) && (PM_E_FAIL == nd_parse_options (p_nd_opt, opt_len, &ndopts)))
    {
        ND_EVENT_DEBUG("Received RS packet with invalid option");
        
        p_nd_stats->rcv_invalid++;
        return PM_E_FAIL;
    }

    /* src mac check */
    if (!IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr))
    {
        if (ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR])
        {
            p_ll_addr = nd_opt_addr_data((struct nd_opt_hdr*)ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR]);
            if (!p_ll_addr)
            {
                ND_EVENT_DEBUG("Received RS packet with invalid source link-address option");
                
                p_nd_stats->rcv_invalid++;
                return PM_E_FAIL;
            }

            /* neigh cache entry update */
            nd_fib_make_key(&nd_fib.key, &p_ipv6_hdr->saddr, p_db_if);
            p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);

            if (!p_nd_fib)
            {
                p_nd_fib = nd_neigh_create(&nd_fib, p_db_if);
                if (!p_nd_fib)
                {
                    return PM_E_FAIL;
                }

                sal_memcpy(p_nd_fib->mac_addr, p_ll_addr, GLB_ETH_ADDR_LEN);
                p_nd_fib->state = NES_INCOMPLETE;
            }
            
            if (p_nd_fib)
            {
                nd_neigh_update_other(p_nd_fib, (char*)p_ll_addr, p_db_rtif, p_db_if, SAL_ND_NEIGHBOR_SOLICIT);
            }
        }
    }
    else
    {
        if (ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR])
        {
            if (ndopts.nd_opt_array[ND6_OPT_SOURCE_LL_ADDR])
            {
                ND_EVENT_DEBUG("Received RS packet with invalid option");

                p_nd_stats->rcv_invalid++;
                return PM_E_FAIL;
            }
        }
    }

    /* Store source address on non multicast capable interface
        send unicast ra when recv unicast rs*/
    if (!IN6_IS_ADDR_UNSPECIFIED(&p_ipv6_hdr->saddr))
    {
        /* Send the solicited router addvertisement */
        ND_EVENT_DEBUG("Received unicast RS, respond it");
        IPV6_ADDR_COPY (&p_db_rtif->nd_rs_dest_addr.u.prefix6, &p_ipv6_hdr->saddr);

        CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_solicited);
        p_db_rtif->t_ra_solicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_ra_solicited, p_db_rtif, 0);          
    }
    else
    {
        ND_EVENT_DEBUG("Received multicast RS");

        /* RFC2461 (If a single advertisement is sent in response to multiple 
            solicitations, the delay is relative to the first solicitation.)  */
        if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_SOLICITED_PENDING))
        {
            ND_EVENT_DEBUG("There is pending solicited RS, nothing to be done");
            return PM_E_NONE;
        }

        GLB_SET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_SOLICITED_PENDING);

        /* The interface interval timer is reset to a new random value,
            as if an unsolicited advertisement had just been sent.
            (RFC2461 6.2.6) */
        ND_EVENT_DEBUG("Compute random delay time for multicast RS");
        interval = sal_rand() % MAX_RA_DELAY_TIME;

        if (p_db_rtif->t_ra_unsolicited)
        {
            remained =ctc_timer_get_remain_msec(p_db_rtif->t_ra_unsolicited);
        }

        if (interval >= remained)
        {
            /* the next unsolicited RA will do it */
            ND_EVENT_DEBUG("The next unsolicited RS will do it after %lu msecs", remained);
            return PM_E_NONE;
        }
        else
        {
            sal_time_t now;
            now = ctc_time_boottime_msec(NULL);
 
            /* In addition, consecutive Router Advertisements sent to the 
                all-nodes multicast address MUST be rate limited to no more 
                than one advertisement every MIN_DELAY_BETWEEN_RAS seconds. */
            if ((uint64)(now + interval) > 
                (uint64)(p_db_rtif->nd_last_ra_unsolicited + p_db_rtif->nd_ra_min_delay))
            {
                /* time between last_send and next_send is larger than 
                    ra_min_delay, delay this random time */
                p_db_rtif->nd_ra_interval = interval + (p_db_rtif->nd_ra_interval - remained);

                CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
                p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, interval);
                ND_EVENT_DEBUG("Respond it after interval %lu msecs", interval);
            }
            else
            {
                /* time between last_send and next_send is less than 
                    ra_min_delay, delay to ra_min_delay */
                p_db_rtif->nd_ra_interval = p_db_rtif->nd_ra_min_delay;

                interval = p_db_rtif->nd_last_ra_unsolicited + p_db_rtif->nd_ra_min_delay - now;

                CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
                p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, interval);
                
                ND_EVENT_DEBUG("Respond it after %lu msecs", 
                    (p_db_rtif->nd_last_ra_unsolicited + p_db_rtif->nd_ra_min_delay) - now);
            }
        }
    }

    return PM_E_NONE;
}

int32
nd_recv_redirect(tbl_route_if_t *p_db_rtif, tbl_nd_stats_t *p_nd_stats)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_nd_stats);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("Router should not received redirect packet, from interface %s", ifname_ext);
    p_nd_stats->rcv_error++;

    return PM_E_NONE;
}

int32
nd_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    struct icmp6hdr             *p_icmp = NULL;
    uint8                            *p_rcv_buf = NULL;
    nd_message_header_t    *p_nd_header = NULL;
    struct ipv6hdr                *p_ipv6_hdr = NULL;
    tbl_nd_stats_t               *p_nd_stats = NULL;
    uint32                           rcv_ifindex = 0;
    tbl_interface_t               *p_db_if = NULL;
    tbl_route_if_t                 *p_db_rtif = NULL;
    ds_connected_v6_t         *p_ifc_tmp = NULL;
    ctclib_slistnode_t           *listnode = NULL;
    uint32                           pkt_len = 0;
    char                              name[IFNAME_SIZE] = {0};
    char                              ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    uint16                           icmpv6_length = 0;
    uint8                             icmpv6_pkt[ICMPV6_MAX_LEN] = {0};
    prefix_t                         src;
    prefix_t                         dst;
    uint16                           icmpv6_cksum = 0;
    uint16                           pkt_icmpv6_cksum = 0;
       
    ND_PTR_CHECK(p_msg);

    sal_memset(&src, 0, sizeof(prefix_t));
    sal_memset(&dst, 0, sizeof(prefix_t));

    if (0 == nd_api_count_get_max())
    {
        ND_PKT_DEBUG("No ipv6 resource, please change profile");
        return PM_E_FAIL;
    }

    p_rcv_buf = (uint8*)p_msg->data;

/* GB packet will include CRC, therefore should delete 4 bytes */
#ifdef _GLB_UML_SYSTEM_
#ifdef GREATBELT
    pkt_len = p_msg->data_len - 4; 
#else
    pkt_len = p_msg->data_len;
#endif
#else
    pkt_len = p_msg->data_len - 4; 
#endif

    rcv_ifindex = p_msg->u_hdr.pkt.ifindex;

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_NOT_INITED;    
    }
    p_nd_stats->rcv++;

    if (0 == p_msg->u_hdr.pkt.vid)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(rcv_ifindex);
    }
    else
    {
        sal_sprintf(name, "vlan%u", p_msg->u_hdr.pkt.vid);
        p_db_if = tbl_interface_get_interface_by_name(name);
    }

    
    if (!p_db_if)
    {
        ND_PKT_DEBUG("Can't find specified interface %d", rcv_ifindex);
        p_nd_stats->rcv_error++;
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&p_db_if->key);
    if (!p_db_rtif)
    {
        ND_EVENT_DEBUG("Can't find routed interface %s", ifname_ext);
        p_nd_stats->rcv_error++;
        return  PM_E_FAIL;     
    }

    if ((p_db_rtif->ifc_ipv6.obj_list) && (0 == p_db_rtif->ifc_ipv6.obj_list->count))
    {
        ND_EVENT_DEBUG("can't find any address in interface %s", ifname_ext);
        return  PM_E_FAIL;
    }
    
    if (pkt_len <= ETH_HDR_LEN +IPV6_HDR_LEN)
    {
        ND_PKT_DEBUG("ND receive wrong length %d pkt", pkt_len);
        p_nd_stats->rcv_error++;
        return  PM_E_ND_BAD_PACKET; 
    }

    /* check ethertype */
    p_nd_header = (nd_message_header_t*)p_rcv_buf;
    p_nd_header->eth_type = sal_ntoh16(p_nd_header->eth_type);
    if (ETH_P_IPV6 != p_nd_header->eth_type)
    {
        ND_PKT_DEBUG("ND receive unknown ethertype type %d", p_nd_header->eth_type);
        p_nd_stats->rcv_invalid++;
        return PM_E_ND_BAD_PACKET;
    }

    p_ipv6_hdr = (struct ipv6hdr*)(p_rcv_buf + ETH_HDR_LEN);
    if (IPPROTO_ICMPV6 != p_ipv6_hdr->nexthdr)
    {
        ND_PKT_DEBUG("ND receive unknown nexthdr %d", p_ipv6_hdr->nexthdr);
        p_nd_stats->rcv_invalid++;
        return PM_E_ND_BAD_PACKET;
    }
    
    /* check icmpv6 type */
    p_icmp = (struct icmp6hdr*)(p_rcv_buf + + IPV6_HDR_LEN + ETH_HDR_LEN);
    if ((SAL_ND_ROUTER_SOLICIT != p_icmp->icmp6_type) && 
        (SAL_ND_ROUTER_ADVERT != p_icmp->icmp6_type) && 
        (SAL_ND_NEIGHBOR_SOLICIT != p_icmp->icmp6_type) && 
        (SAL_ND_NEIGHBOR_ADVERT != p_icmp->icmp6_type) && 
        (SAL_ND_REDIRECT != p_icmp->icmp6_type))
    {
        ND_PKT_DEBUG("ND receive unknown packet type %d", p_icmp->icmp6_type);
        p_nd_stats->rcv_invalid++;
        return PM_E_ND_BAD_PACKET;
    }

    /* modified by liwh for bug 49326, 2018-10-11 */
    icmpv6_length = sal_ntoh16(p_ipv6_hdr->payload_len);
    ND_PKT_DEBUG("ND receive icmpv6 payload_len %d", icmpv6_length);
    /* liwh end */

    /* modified by liwh for bug 49331, 2018-10-11 */
    /* check icmpv6 checksum */
    sal_memcpy(icmpv6_pkt, (p_rcv_buf + + IPV6_HDR_LEN + ETH_HDR_LEN), icmpv6_length);
    
    /* set src and dst */
    src.family = AF_INET6;
    sal_memcpy(&src.u.prefix6, &p_ipv6_hdr->saddr, IPV6_MAX_BYTELEN);
    dst.family = AF_INET6;
    sal_memcpy(&dst.u.prefix6, &p_ipv6_hdr->daddr, IPV6_MAX_BYTELEN);
    
    /* set pkt checksum to 0 */
    icmpv6_pkt[2] = 0;
    icmpv6_pkt[3] = 0;

    /* calculate checksum */
    icmpv6_cksum = nd_in6_checksum(&src, &dst, IPPROTO_ICMPV6, (uint16*)icmpv6_pkt, icmpv6_length);
    icmpv6_cksum = sal_ntoh16(icmpv6_cksum);
    
    /* get pkt checksum */
    pkt_icmpv6_cksum = sal_ntoh16(p_icmp->icmp6_cksum);
    ND_PKT_DEBUG("ND receive icmpv6 checksum %x, calculate checksum %x", pkt_icmpv6_cksum, icmpv6_cksum);
    if (icmpv6_cksum != pkt_icmpv6_cksum)
    {
        p_nd_stats->rcv_invalid++;
        return PM_E_ND_BAD_PACKET;
    }
    /* liwh end */ 

    if(ADDR_IS_SOLICITED(&p_ipv6_hdr->daddr))
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc_tmp, listnode)
        {
            if (SAME_SOLICITED(p_ipv6_hdr->daddr, p_ifc_tmp->key.address.u.prefix6))
            {
                break;
            }
        }

        if (!p_ifc_tmp)
        {
            p_nd_stats->rcv_invalid++;
            return PM_E_NONE;
        }
    }

    /* ICMP message type check. */
    switch (p_icmp->icmp6_type)
    {
        case SAL_ND_ROUTER_SOLICIT:
            /* modified by liwh for bug 49326, 2018-10-11 */
            if (icmpv6_length < ICMPV6_MIN_RS_LEN)
            {
                p_nd_stats->rcv_invalid++;
                return PM_E_ND_BAD_PACKET;
            }
            /* liwh end */
    
            p_nd_stats->rcv_rs++;
            nd_recv_rs(p_rcv_buf, pkt_len, p_db_rtif, p_db_if, p_nd_stats);
            break;

        case SAL_ND_ROUTER_ADVERT:
            /* modified by liwh for bug 49326, 2018-10-11 */
            if (icmpv6_length < ICMPV6_MIN_RA_LEN)
            {
                p_nd_stats->rcv_invalid++;
                return PM_E_ND_BAD_PACKET;
            }
            /* liwh end */
            
            p_nd_stats->rcv_ra++;
            nd_recv_ra(p_rcv_buf, pkt_len, p_db_rtif, p_db_if, p_nd_stats);
            break;

        case SAL_ND_NEIGHBOR_SOLICIT:
            /* modified by liwh for bug 49326, 2018-10-11 */
            if (icmpv6_length < ICMPV6_MIN_NS_LEN)
            {
                p_nd_stats->rcv_invalid++;
                return PM_E_ND_BAD_PACKET;
            }
            /* liwh end */
            
            p_nd_stats->rcv_ns++;
            nd_recv_ns(p_rcv_buf, pkt_len, p_db_rtif, p_db_if, p_nd_stats);
            break;

        case SAL_ND_NEIGHBOR_ADVERT:
            /* modified by liwh for bug 49326, 2018-10-11 */
            if (icmpv6_length < ICMPV6_MIN_NA_LEN)
            {
                p_nd_stats->rcv_invalid++;
                return PM_E_ND_BAD_PACKET;
            }
            /* liwh end */
            
            p_nd_stats->rcv_na++;
            nd_recv_na(p_rcv_buf, pkt_len, p_db_rtif, p_db_if, p_nd_stats);
            break;

        default: /*  SAL_ND_REDIRECT */
            /* modified by liwh for bug 49326, 2018-10-11 */
            if (icmpv6_length < ICMPV6_MIN_REDIRECT_LEN)
            {
                p_nd_stats->rcv_invalid++;
                return PM_E_ND_BAD_PACKET;
            }
            /* liwh end */
    
            p_nd_stats->rcv_redirect++;
            nd_recv_redirect(p_db_rtif, p_nd_stats);
            break;
    }
    
    return PM_E_NONE;
}

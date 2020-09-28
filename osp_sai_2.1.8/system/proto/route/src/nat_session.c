
#include "proto.h"
#include "route.h"
#include "prefix.h"
#include "libnetfilter_conntrack/libnetfilter_conntrack.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"
#include "nat.h"

#if _DESC("nat session communicate with kernel netfilter connect track functions")
/***************************************************************************************************
 * Name         : nat_nfct_parse
 * Purpose      : parse netfilter conntrack entry from netlink message
 * Input        : struct nf_conntrack *ct
 * Output       : nat_nfct_s *p_nfct
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_parse(struct nf_conntrack *ct, nat_nfct_s *p_nfct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    if (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT))
    {
        p_nfct->bSnat = TRUE;
    }

    if (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT))
    {
        p_nfct->bDnat = TRUE;
    }

    if (nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT))
    {
        p_nfct->bSPat = TRUE;
    }

    if (nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT))
    {
        p_nfct->bDpat = TRUE;
    }

    p_nfct->ip_proto = nfct_get_attr_u8(ct, ATTR_ORIG_L4PROTO);/*TCP, UDP, ICMP...*/
    p_nfct->timeout = nfct_get_attr_u32(ct, ATTR_TIMEOUT);
    p_nfct->family = nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO);

    p_nfct->mark = nfct_get_attr_u32(ct, ATTR_MARK);
    p_nfct->orig.sip.u.prefix4.s_addr = nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_SRC);
    p_nfct->orig.dip.u.prefix4.s_addr = nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_DST);
    p_nfct->replace.sip.u.prefix4.s_addr = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC);
    p_nfct->replace.dip.u.prefix4.s_addr = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST);

    p_nfct->counts.bytes = nfct_get_attr_u32(ct, ATTR_ORIG_COUNTER_BYTES) +
    nfct_get_attr_u32(ct, ATTR_REPL_COUNTER_BYTES);
    p_nfct->counts.packets = nfct_get_attr_u32(ct, ATTR_ORIG_COUNTER_PACKETS) +
    nfct_get_attr_u32(ct, ATTR_REPL_COUNTER_PACKETS);

    /* 6 TCP ; 17 UDP*/
    if (p_nfct->ip_proto == IPPROTO_TCP || p_nfct->ip_proto == IPPROTO_UDP)
    {
        p_nfct->orig.sport = ntohs(nfct_get_attr_u16(ct, ATTR_ORIG_PORT_SRC));
        p_nfct->orig.dport = ntohs(nfct_get_attr_u16(ct, ATTR_ORIG_PORT_DST));
        p_nfct->replace.sport = ntohs(nfct_get_attr_u16(ct, ATTR_REPL_PORT_SRC));
        p_nfct->replace.dport = ntohs(nfct_get_attr_u16(ct, ATTR_REPL_PORT_DST));
    }

    if (p_nfct->ip_proto == IPPROTO_TCP) /* TCP */
    {
        p_nfct->tcp.state = nfct_get_attr_u8(ct, ATTR_TCP_STATE);
    }
    else if (p_nfct->ip_proto == IPPROTO_ICMP)/* ICMP */
    {
        p_nfct->icmp.type = nfct_get_attr_u8(ct, ATTR_ICMP_TYPE);
        p_nfct->icmp.code = nfct_get_attr_u8(ct, ATTR_ICMP_CODE);
        p_nfct->icmp.ID = nfct_get_attr_u16(ct, ATTR_ICMP_ID);
    }

    if (nfct_attr_is_set(ct, ATTR_HELPER_NAME))
    {
        sal_memcpy(p_nfct->helper, (char*)nfct_get_attr(ct, ATTR_HELPER_NAME), NFCT_HELPER_NAME_MAX);
    }
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_filter
 * Purpose      : check if we should process the netfilter conntrack entry
 * Input        : struct nf_conntrack *ct
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_filter(nat_nfct_s *p_nat_nfct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    /* 1. check nat flags*/
    /* 1.1 snat or dnat*/
    if ((!p_nat_nfct->bSnat) && (!p_nat_nfct->bDnat))
    {
        return -1;
    }

    /* 2 check family*/
    if (AF_INET != p_nat_nfct->family)
    {
        return -1;
    }

    /* 3 not mine NAT sessions*/
    if ((p_nat_nfct->mark != GLB_NAT_NFT_CT_MARK_UPSTREAM) && (p_nat_nfct->mark != GLB_NAT_NFT_CT_MARK_DNSTREAM))
    {
        return -1;
    }
#endif   
#endif
#endif
    return 0;
}


/***************************************************************************************************
 * Name         : nat_nfct_translate_to_session
 * Purpose      : translation kernel netfilter conntrack entry to nat session
 * Input        : nat_nfct_s *p_nfct,
 * Output       : tbl_nat_session_t *p_session
 * Return       : ttl value
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_translate_to_session(nat_nfct_s *p_nfct, tbl_nat_session_t *p_session)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    tbl_route_if_t* p_rt_if = NULL;
    uint8 prefix_len = 0;

    p_session->key.ip_protocol = p_nfct->ip_proto;
    if (IPPROTO_ICMP == p_session->key.ip_protocol)
    {
        p_session->key.proto_info.icmp.type = p_nfct->icmp.type;
        p_session->key.proto_info.icmp.code = p_nfct->icmp.code;
        p_session->key.proto_info.icmp.ID   = p_nfct->icmp.ID;
    }

    if (TRUE == p_nfct->bSnat)
    {
        p_session->creator = GLB_NAT_SESSION_CREATOR_INSIDE;

        p_session->key.inside_local_ip.family = AF_INET;
        sal_memcpy(&p_session->key.inside_local_ip.u.prefix4, &p_nfct->orig.sip.u.prefix4, sizeof(addr_ipv4_t));

        p_session->key.outside_global_ip.family = AF_INET;
        sal_memcpy(&p_session->key.outside_global_ip.u.prefix4, &p_nfct->orig.dip.u.prefix4, sizeof(addr_ipv4_t));

        //p_session->key.outside_local_ip.family = AF_INET;
        //sal_memcpy(&p_session->key.outside_local_ip.u.prefix4, &p_nfct->replace.sip.u.prefix4, sizeof(addr_ipv4_t));

        p_session->key.inside_global_ip.family = AF_INET;
        sal_memcpy(&p_session->key.inside_global_ip.u.prefix4, &p_nfct->replace.dip.u.prefix4, sizeof(addr_ipv4_t));

        if ((IPPROTO_TCP == p_session->key.ip_protocol) || (IPPROTO_UDP == p_session->key.ip_protocol))
        {
            p_session->key.proto_info.l4_port.inside_local = p_nfct->orig.sport;
            p_session->key.proto_info.l4_port.outside_global = p_nfct->orig.dport;
            p_session->key.proto_info.l4_port.inside_global = p_nfct->replace.dport;
        }
    }
    else if (TRUE == p_nfct->bDnat)
    {
        p_session->creator = GLB_NAT_SESSION_CREATOR_OUTSIDE;
    
        p_session->key.inside_local_ip.family = AF_INET;
        sal_memcpy(&p_session->key.inside_local_ip.u.prefix4, &p_nfct->replace.sip.u.prefix4, sizeof(addr_ipv4_t));
        
        p_session->key.outside_global_ip.family = AF_INET;
        sal_memcpy(&p_session->key.outside_global_ip.u.prefix4, &p_nfct->orig.sip.u.prefix4, sizeof(addr_ipv4_t));
        
        //p_session->key.outside_local_ip.family = AF_INET;
        //sal_memcpy(&p_session->key.outside_local_ip.u.prefix4, &p_nfct->replace.sip.u.prefix4, sizeof(addr_ipv4_t));
        
        p_session->key.inside_global_ip.family = AF_INET;
        sal_memcpy(&p_session->key.inside_global_ip.u.prefix4, &p_nfct->orig.dip.u.prefix4, sizeof(addr_ipv4_t));

        if ((IPPROTO_TCP == p_session->key.ip_protocol) || (IPPROTO_UDP == p_session->key.ip_protocol))
        {
            p_session->key.proto_info.l4_port.inside_local = p_nfct->replace.sport;
            p_session->key.proto_info.l4_port.outside_global = p_nfct->orig.sport;
            p_session->key.proto_info.l4_port.inside_global = p_nfct->orig.dport;
        }
    }

    p_session->timestamp = ctc_time_boottime_sec(NULL);
    p_session->hit = p_session->timestamp;
    p_session->upd_kernel_ttl_time = p_session->timestamp;
    p_session->ttl = p_nfct->timeout;

    /* get inside interface, subnet match or exactly match*/
    if (TRUE == nat_get_rtif_by_iproute(&p_session->key.inside_local_ip.u.prefix4, &p_rt_if, &prefix_len))
    {
        p_session->iif_index = p_rt_if->ifindex;
    }
/*Add by zhush for bug 52813, 2019-07-24*/
    else
    {
        NAT_EVENT_DEBUG("get inside interface from route failed, set slow path forwarding flag");
        p_session->slow_path_fwd = TRUE;
    }
/*end zhush*/

    /* get outside interface, subnet match or exactly match*/
    if (TRUE == nat_get_rtif_by_iproute(&p_session->key.inside_global_ip.u.prefix4, &p_rt_if, &prefix_len))
    {
        p_session->oif_index = p_rt_if->ifindex;
    }

    /* if ALG*/
    if (0 != p_nfct->helper[0])
    {
        sal_memcpy(p_session->helper, p_nfct->helper, NFCT_HELPER_NAME_MAX);
    }

    /* ICMP and ALG NAT flows should be forwarded by kernel */
    switch (p_session->key.ip_protocol)
    {
        case IPPROTO_UDP:
        case IPPROTO_TCP:
        {
            /* Tsingma only support TCP & UDP packets NAT forward. ICMP and other IP packets, and ALG packets, should be NAT forwaded by kernel.*/
            if (0 != p_nfct->helper[0])
            {
                /* ALG*/
                p_session->slow_path_fwd = TRUE;
            }
            break;
        }
        case IPPROTO_ICMP:
        default:
        {
            p_session->slow_path_fwd = TRUE;
            break;
        }
    }

    /* need set forward entries to Tsingma, update TTL, and processing aging by hsrv_nat*/
    if (TRUE != p_session->slow_path_fwd)
    {
        p_session->ttl = nat_session_ttl_get(p_session->key.ip_protocol);
    }
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_translate_from_session
 * Purpose      : translation nat session to kernel netfilter conntrack entry
 * Input        : tbl_nat_session_t *p_session
 * Output       : nat_nfct_s *p_nfct
 * Return       : ttl value
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_translate_from_session(tbl_nat_session_t *p_session, struct nf_conntrack *ct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    nfct_set_attr_u8(ct, ATTR_ORIG_L3PROTO, AF_INET);
    nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, p_session->key.ip_protocol);

    if (GLB_NAT_SESSION_CREATOR_INSIDE == p_session->creator)
    {
        /* orig IP*/
        nfct_set_attr(ct, ATTR_ORIG_IPV4_SRC, &p_session->key.inside_local_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_ORIG_IPV4_DST, &p_session->key.outside_global_ip.u.prefix4);

        if ((IPPROTO_UDP == p_session->key.ip_protocol) || (IPPROTO_TCP == p_session->key.ip_protocol))
        {
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.inside_local));
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.outside_global));
        }
    }
    else if (GLB_NAT_SESSION_CREATOR_OUTSIDE == p_session->creator)
    {
        /* orig IP*/
        nfct_set_attr(ct, ATTR_ORIG_IPV4_SRC, &p_session->key.outside_global_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_ORIG_IPV4_DST, &p_session->key.inside_global_ip.u.prefix4);

        if ((IPPROTO_UDP == p_session->key.ip_protocol) || (IPPROTO_TCP == p_session->key.ip_protocol))
        {
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.outside_global));
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.inside_global));
        }
    }

    if (IPPROTO_ICMP == p_session->key.ip_protocol)
    {
        nfct_set_attr_u8(ct, ATTR_ICMP_TYPE, p_session->key.proto_info.icmp.type);
        nfct_set_attr_u8(ct, ATTR_ICMP_CODE, p_session->key.proto_info.icmp.code);
        nfct_set_attr_u16(ct, ATTR_ICMP_ID, sal_hton16(p_session->key.proto_info.icmp.ID));
    }
#endif
#endif
#endif
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_nfct_translate_all_from_session
 * Purpose      : translation nat session to kernel netfilter conntrack entry
 * Input        : tbl_nat_session_t *p_session
 * Output       : nat_nfct_s *p_nfct
 * Return       : ttl value
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_translate_all_from_session(tbl_nat_session_t *p_session, struct nf_conntrack *ct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    uint32 mark = 0;

    nfct_set_attr_u8(ct, ATTR_ORIG_L3PROTO, AF_INET);
    nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, p_session->key.ip_protocol);

    nfct_set_attr_u8(ct, ATTR_REPL_L3PROTO, AF_INET);
    nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, p_session->key.ip_protocol);

    if (GLB_NAT_SESSION_CREATOR_INSIDE == p_session->creator)
    {
        /* orig IP*/
        nfct_set_attr(ct, ATTR_ORIG_IPV4_SRC, &p_session->key.inside_local_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_ORIG_IPV4_DST, &p_session->key.outside_global_ip.u.prefix4);

        /* replace IP*/
        nfct_set_attr(ct, ATTR_REPL_IPV4_SRC, &p_session->key.outside_global_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_REPL_IPV4_DST, &p_session->key.inside_global_ip.u.prefix4);

        if ((IPPROTO_UDP == p_session->key.ip_protocol) || (IPPROTO_TCP == p_session->key.ip_protocol))
        {
            /* orig port*/
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.inside_local));
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.outside_global));

            /* replace port*/
            nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.outside_global));
            nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.inside_global));
        }

        mark = 1;
        nfct_set_attr_u32(ct, ATTR_MARK, mark);
    }
    else if (GLB_NAT_SESSION_CREATOR_OUTSIDE == p_session->creator)
    {
        /* orig IP*/
        nfct_set_attr(ct, ATTR_ORIG_IPV4_SRC, &p_session->key.outside_global_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_ORIG_IPV4_DST, &p_session->key.inside_global_ip.u.prefix4);

        /* repl IP*/
        nfct_set_attr(ct, ATTR_REPL_IPV4_SRC, &p_session->key.inside_local_ip.u.prefix4);
        nfct_set_attr(ct, ATTR_REPL_IPV4_DST, &p_session->key.outside_global_ip.u.prefix4);

        if ((IPPROTO_UDP == p_session->key.ip_protocol) || (IPPROTO_TCP == p_session->key.ip_protocol))
        {
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.outside_global));
            nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.inside_global));

            nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, sal_hton16(p_session->key.proto_info.l4_port.inside_local));
            nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, sal_hton16(p_session->key.proto_info.l4_port.outside_global));
        }

        mark = 2;
        nfct_set_attr_u32(ct, ATTR_MARK, mark);
    }

    if (IPPROTO_ICMP == p_session->key.ip_protocol)
    {
        nfct_set_attr_u8(ct, ATTR_ICMP_TYPE, p_session->key.proto_info.icmp.type);
        nfct_set_attr_u8(ct, ATTR_ICMP_CODE, p_session->key.proto_info.icmp.code);
        nfct_set_attr_u16(ct, ATTR_ICMP_ID, sal_hton16(p_session->key.proto_info.icmp.ID));
    }
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_debug_print
 * Purpose      : print kernel netfilter conntrack session
 * Input        : tbl_nat_session_key_t *p_session_key
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_debug_print(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, nat_nfct_s *p_nfct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    int seconds, minutes, hours;
    char     sz_addr1[64];
    char     sz_addr2[64];
    char     sz_state[64];
    char     sz_ttlc[64];
    char     sz_type[16];

    sal_memset(sz_addr1, 0, sizeof(sz_addr1));
    sal_memset(sz_addr2, 0, sizeof(sz_addr2));
    sal_memset(sz_state, 0, sizeof(sz_state));
    sal_memset(sz_ttlc, 0, sizeof(sz_ttlc));
    sal_memset(sz_type, 0, sizeof(sz_type));

    NAT_EVENT_DEBUG("nfct notify hook dump begin:");
    NAT_EVENT_DEBUG("flags: SNAT:%u, DNAT:%u, SPAT:%u, DPAT:%u\r\n",
                             p_nfct->bSnat, p_nfct->bDnat,
                             p_nfct->bSPat, p_nfct->bDpat);    

    seconds = p_nfct->timeout;
    minutes = seconds/60;
    hours = minutes/60;
    minutes = minutes%60;
    seconds = seconds%60;

    snprintf(sz_ttlc,11, "%3i:%02i:%02i", hours, minutes, seconds);

    if (p_nfct->ip_proto == IPPROTO_TCP) /* TCP */
    {
        sprintf(sz_state, "%u", nfct_get_attr_u8(ct, ATTR_TCP_STATE));
    }
    else if (p_nfct->ip_proto == IPPROTO_UDP) /* UDP */
    {
        sprintf(sz_state, "%s", "NA");
    }
    else if (p_nfct->ip_proto == IPPROTO_ICMP)/* ICMP */
    {
        sprintf(sz_state, "(ICMP type:%u/code:%u/ID:%u)", p_nfct->icmp.type, 
                                                          p_nfct->icmp.code, 
                                                          p_nfct->icmp.ID);
    }

    switch (nf_type)
    {
        case NFCT_T_NEW:
            sprintf(sz_type, "%s", "new");
            break;
        case NFCT_T_UPDATE:
            sprintf(sz_type, "%s", "update");
            break;
        case NFCT_T_DESTROY:
            sprintf(sz_type, "%s", "destroy");
            break;
        default:
            sprintf(sz_type, "%s", "unknown");
            break;
    }
    
    NAT_EVENT_DEBUG(" type:%s, family:%u, l4proto:%u, state:%s, ttl:%s, mark:%u", 
                      sz_type, 
                      p_nfct->family, 
                      p_nfct->ip_proto, 
                      sz_state, 
                      sz_ttlc, 
                      p_nfct->mark);
    NAT_EVENT_DEBUG(" orig: sip:%s, dip:%s, sport:%u, dport:%u",
              cdb_addr_ipv4_val2str(sz_addr1, 64, &p_nfct->orig.sip.u.prefix4), 
              cdb_addr_ipv4_val2str(sz_addr2, 64, &p_nfct->orig.dip.u.prefix4),
              p_nfct->orig.sport, p_nfct->orig.dport);
    NAT_EVENT_DEBUG(" repl: sip:%s, dip:%s, sport:%u, dport:%u",
            cdb_addr_ipv4_val2str(sz_addr1, 64, &p_nfct->replace.sip.u.prefix4), 
            cdb_addr_ipv4_val2str(sz_addr2, 64, &p_nfct->replace.dip.u.prefix4),
            p_nfct->replace.sport, p_nfct->replace.dport);

    if (0 != p_nfct->helper[0])
    {
        NAT_EVENT_DEBUG(" helper:%s",p_nfct->helper);
    }

    NAT_EVENT_DEBUG("nfct notify hook dump end.");
#endif
#endif
#endif
    return NFCT_CB_CONTINUE;
}

/***************************************************************************************************
 * Name         : nat_nfct_set_ttl_by_session
 * Purpose      : notify kernel conntrack module set session ttl
 * Input        : struct nf_conntrack *ct, uint32 ttl
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_set_ttl_by_session(tbl_nat_session_t *p_session, uint32 ttl)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle *nfct_h = NULL;
    struct nf_conntrack *ct_new = NULL;

    NAT_EVENT_DEBUG("nat_nfct_set_ttl_by_session, ttl:%u", ttl);

    nfct_h = nfct_open(CONNTRACK, 0);
    if (!nfct_h)
    {
        return PM_E_FAIL;
    }

    ct_new = nfct_new();
    if (!ct_new)
    {
        nfct_close(nfct_h);
        return PM_E_NO_MEMORY;
    }

    /* set ttl*/
    nat_nfct_translate_from_session(p_session, ct_new);
    nfct_set_attr_u32(ct_new, ATTR_TIMEOUT, ttl);

    /* notify kernel*/
    nfct_query(nfct_h, NFCT_Q_UPDATE, ct_new);

    nfct_destroy(ct_new);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_set_ttl_by_ct
 * Purpose      : notify kernel conntrack module set session ttl
 * Input        : struct nf_conntrack *ct, uint32 ttl
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_set_ttl_by_ct(struct nf_conntrack *ct, uint32 ttl)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle *nfct_h = NULL;
    struct nf_conntrack *ct_new = NULL;

    NAT_EVENT_DEBUG("nat_nfct_set_ttl_by_ct, ttl:%u", ttl);

    nfct_h = nfct_open(CONNTRACK, 0);
    if (!nfct_h)
    {
        return PM_E_FAIL;
    }

    ct_new = nfct_new();
    if (!ct_new)
    {
        nfct_close(nfct_h);
        return PM_E_NO_MEMORY;
    }

    /* set ttl*/
    nfct_copy(ct_new, ct, NFCT_CP_ALL);
    nfct_set_attr_u32(ct_new, ATTR_TIMEOUT, ttl);

    /* notify kernel*/
    nfct_query(nfct_h, NFCT_Q_UPDATE, ct_new);

    nfct_destroy(ct_new);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_add_by_session
 * Purpose      : notify kernel netfilter conntrack module delete one session
 * Input        : tbl_nat_session_key_t *p_session_key
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_add_by_session(tbl_nat_session_t *p_session)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle  *nfct_h = NULL;
    struct nf_conntrack *ct_new = NULL;
    tbl_nat_global_t    *p_nat_global = NULL;

    NAT_EVENT_DEBUG("nat_nfct_add_by_session");

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;    
    }

    nfct_h = nfct_open(CONNTRACK, NFNL_SUBSYS_CTNETLINK);
    if (!nfct_h)
    {
        return PM_E_FAIL;
    }

    ct_new = nfct_new();
    if (!ct_new)
    {
        nfct_close(nfct_h);
        return PM_E_NO_MEMORY;
    }

    /* orig information*/
    nat_nfct_translate_all_from_session(p_session, ct_new);
    nfct_set_attr_u32(ct_new, ATTR_TIMEOUT, NAT_NFCT_FAST_PATH_FWD_TTL_DEFAULT);

    /* notify kernel*/
    nfct_query(nfct_h, NFCT_Q_CREATE_UPDATE, ct_new);
    p_nat_global->nfct_notify_kernel_add_cnt++;

    nfct_destroy(ct_new);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_del_by_session
 * Purpose      : notify kernel netfilter conntrack module delete one session
 * Input        : tbl_nat_session_key_t *p_session_key
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_del_by_session(tbl_nat_session_t *p_session)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle  *nfct_h = NULL;
    struct nf_conntrack *ct_new = NULL;
    tbl_nat_global_t    *p_nat_global = NULL;

    NAT_EVENT_DEBUG("nat_nfct_del_by_session");

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;    
    }

    nfct_h = nfct_open(CONNTRACK, NFNL_SUBSYS_CTNETLINK);
    if (!nfct_h)
    {
        return PM_E_FAIL;
    }

    ct_new = nfct_new();
    if (!ct_new)
    {
        nfct_close(nfct_h);
        return PM_E_NO_MEMORY;
    }

    /* orig information*/
    nat_nfct_translate_from_session(p_session, ct_new);

    /* notify kernel*/
    nfct_query(nfct_h, NFCT_Q_DESTROY, ct_new);
    p_nat_global->nfct_notify_kernel_del_cnt++;

    nfct_destroy(ct_new);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_del_by_ct
 * Purpose      : notify kernel conntrack module delete entry
 * Input        : struct nf_conntrack *ct
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_del_by_ct(struct nf_conntrack *ct)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle *nfct_h = NULL;
    struct nf_conntrack *ct_new = NULL;
    tbl_nat_global_t    *p_nat_global = NULL;

    NAT_EVENT_DEBUG("nat_nfct_del_by_ct");
    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;    
    }

    nfct_h = nfct_open(CONNTRACK, 0);
    if (!nfct_h)
    {
        return PM_E_FAIL;
    }

    ct_new = nfct_new();
    if (!ct_new)
    {
        nfct_close(nfct_h);
        return PM_E_NO_MEMORY;
    }

    nfct_copy(ct_new, ct, NFCT_CP_ALL);

    /* notify kernel nfct*/
    nfct_query(nfct_h, NFCT_Q_DESTROY, ct_new);
    p_nat_global->nfct_notify_kernel_del_cnt++;

    nfct_destroy(ct_new);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_flush_hook
 * Purpose      : Callback for kernel netfilter conntrack
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_hook_flush_by_new_rule(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    nat_nfct_s nat_nfct;
    nat_nfct_hook_s *p_data = tmp;
    tbl_nat_rule_t *p_new_rule = (tbl_nat_rule_t *)p_data->argv[0];
    char str[256];
    
    NAT_EVENT_DEBUG("Flush netfilter conntrack hook. type:%u, para:%s", nf_type, cdb_nat_rule_key_val2str(str, 256, &p_new_rule->key));

    sal_memset(&nat_nfct, 0, sizeof(nat_nfct));
    nat_nfct_parse(ct, &nat_nfct);
    nat_nfct_debug_print(nf_type, ct, &nat_nfct);

    if (TRUE == nat_rule_if_nfct_related(&nat_nfct, p_new_rule))
    {
        NAT_EVENT_DEBUG("nat_nfct_flush_hook. delted");
        nat_nfct_del_by_ct(ct);
    }

#endif
#endif
#endif
    return NFCT_CB_CONTINUE;
}

/***************************************************************************************************
 * Name         : nat_nfct_dump_alg_hook
 * Purpose      : Callback for kernel netfilter conntrack
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_hook_dump_alg(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    uint32 *p_show_cnt = 0;
    nat_nfct_s nat_nfct;
    tbl_nat_session_t session;
    nat_nfct_hook_s *p_data = tmp;
    nat_translations_show_filter_s * p_filter = NULL;
    FILE * fp = NULL;
    
    int32 (*fn)(FILE *, tbl_nat_session_t *, uint32) = p_data->argv[0];
    fp = (FILE *)p_data->argv[1];
    p_show_cnt = p_data->argv[2];
    p_filter = p_data->argv[3];

    sal_memset(&nat_nfct, 0, sizeof(nat_nfct));
    nat_nfct_parse(ct, &nat_nfct);

    if ((TRUE != nat_nfct.bSnat) && (TRUE != nat_nfct.bDnat))
    {
        return NFCT_CB_CONTINUE;
    }
    
    if (((IPPROTO_UDP == nat_nfct.ip_proto) || (IPPROTO_TCP == nat_nfct.ip_proto))
     && (0 == nat_nfct.helper[0]))
    {
        return NFCT_CB_CONTINUE;
    }

    nat_session_struct_init(&session);
    nat_nfct_translate_to_session(&nat_nfct, &session);

    if (TRUE == nat_cmd_translations_filter_session(&session, p_filter))
    {
        return NFCT_CB_CONTINUE;
    }

    fn(fp, &session, *p_show_cnt);
    (*p_show_cnt)++;

#endif
#endif
#endif
    return NFCT_CB_CONTINUE;
}

/***************************************************************************************************
 * Name         : nat_nfct_dump_alg_hook
 * Purpose      : Callback for kernel netfilter conntrack
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_hook_clear_session(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    nat_nfct_s nat_nfct;
    nat_nfct_hook_s *p_data = tmp;
    tbl_route_if_t  *p_db_rt_if = NULL;

    if (p_data)
    {
        p_db_rt_if = p_data->argv[0];
    }
    
    sal_memset(&nat_nfct, 0, sizeof(nat_nfct));
    nat_nfct_parse(ct, &nat_nfct);
    nat_nfct_debug_print(nf_type, ct, &nat_nfct);

    if ((TRUE != nat_nfct.bSnat) && (TRUE != nat_nfct.bDnat))
    {
        return NFCT_CB_CONTINUE;
    }

    if (p_db_rt_if)
    {
        bool ret = FALSE;
        tbl_route_if_t *p_rt_if = NULL;
        uint8 prefixlen = 0;

        if (nat_nfct.bSnat)
        {
            ret = nat_get_rtif_by_iproute(&nat_nfct.orig.dip.u.prefix4, &p_rt_if, &prefixlen);
        }
        else if (nat_nfct.bDnat)
        {
            ret = nat_get_rtif_by_iproute(&nat_nfct.orig.sip.u.prefix4, &p_rt_if, &prefixlen);
        }

        if (TRUE != ret)
        {
            return NFCT_CB_CONTINUE;
        }

        if (p_db_rt_if->ifindex != p_rt_if->ifindex)
        {
            return NFCT_CB_CONTINUE;
        }
    }

    nat_nfct_del_by_ct(ct);
#endif
#endif
#endif
    return NFCT_CB_CONTINUE;
}

/***************************************************************************************************
 * Name         : nat_nfct_scan
 * Purpose      : scan kernel nfct table, call specified hook
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nfct_scan(int32 (*fn_hook)(enum nf_conntrack_msg_type , struct nf_conntrack *, void *), nat_nfct_hook_s *p_data)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    struct nfct_handle *nfct_h = NULL;
    struct nfct_filter_dump *filter_dump = NULL;

    nfct_h = nfct_open(CONNTRACK, 0);
    if (!nfct_h)
    {
        NAT_EVENT_DEBUG("Can't open handler");
        return PM_E_FAIL;
    }
    
    nfct_callback_register(nfct_h, NFCT_T_ALL, fn_hook, p_data);
    
    filter_dump = nfct_filter_dump_create();
    if (!filter_dump)
    {
        NAT_EVENT_DEBUG("filter dump ret null");
        nfct_close(nfct_h);
        return PM_E_FAIL;
    }
    
    nfct_filter_dump_set_attr_u8(filter_dump, NFCT_FILTER_DUMP_L3NUM, AF_INET);

    nfct_query(nfct_h, NFCT_Q_DUMP_FILTER, filter_dump);
    
    nfct_filter_dump_destroy(filter_dump);

    nfct_callback_unregister(nfct_h);
    nfct_close(nfct_h);
#endif
#endif
#endif
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nfct_hook
 * Purpose      : Callback for kernel netfilter conntrack
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32 nat_nfct_hook(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    int32 ret = PM_E_NONE;
    nat_nfct_s nat_nfct;
    tbl_nat_session_t session;

    NAT_EVENT_DEBUG("netfilter conntrack hook. type:%u.", nf_type);

    sal_memset(&nat_nfct, 0, sizeof(nat_nfct));
    nat_nfct_parse(ct, &nat_nfct);
    nat_nfct_debug_print(nf_type, ct, &nat_nfct);

    if (nat_nfct_filter(&nat_nfct))
    {
        NAT_EVENT_DEBUG("netfilter conntrack hook. filterd.");
        return NFCT_CB_CONTINUE;
    }

    nat_session_struct_init(&session);
    nat_nfct_translate_to_session(&nat_nfct, &session);
    switch (nf_type)
    {
        case NFCT_T_NEW:
        {
            ret = nat_session_add(&session);
            
            /* if NAT forward by kernel, like ICMP and ALG, aging processed by kernel.
               otherwise, set NAT_NFCT_FAST_PATH_FWD_TTL_DEFAULT to kernel, aging processed by userspace NAT app, entries in kernel should not be deleted before NAT app delete it.*/
            if ((PM_E_NONE == ret) && (TRUE != session.slow_path_fwd))
            {
                nat_nfct_set_ttl_by_ct(ct, NAT_NFCT_FAST_PATH_FWD_TTL_DEFAULT);
            }
            break;
        }
        case NFCT_T_DESTROY:
        {
            tbl_nat_session_t *p_db_session = NULL;

            p_db_session = tbl_nat_session_get_nat_session(&session.key);
            if (!p_db_session)
            {
                break;
            }

            if ((TRUE != p_db_session->slow_path_fwd)
             && (GLB_NAT_SET_SAI_STATUS_FAILED != p_db_session->set_sai_status))
            {
                /* chip forwarded sessions, aged and deleted by NAT app, kernel should not delete it.*/
                nat_nfct_add_by_session(p_db_session);
            }
            else
            {
                /* kernel forwarded sessions, aged and deleted by kernel.*/
                nat_session_del(&session, FALSE);
            }
            
            break;
        }
        default :
            break;
    }
#endif
#endif
#endif
    return NFCT_CB_CONTINUE;
}

/***************************************************************************************************
 * Name         : nat_nfct_reg
 * Purpose      : init communication between nat session with nfct, for processing new nfct entries
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void nat_nfct_reg()
{
#ifdef TSINGMA
#ifndef OFPRODUCT
#ifndef _GLB_UML_SYSTEM_
    tbl_nat_global_t *p_nat_global = NULL;
    struct nfct_handle *nfct_h = NULL;

    p_nat_global = tbl_nat_global_get_nat_global();

    nfct_h = nfct_open(CONNTRACK, (NF_NETLINK_CONNTRACK_NEW | NF_NETLINK_CONNTRACK_DESTROY));
    if (!nfct_h)
    {
        return;
    }
    nfct_callback_register(nfct_h, NFCT_T_ALL, nat_nfct_hook, NULL);

    /* 
    catch nfct events, add read task to do this.*/
    ctc_nfct_catch(nfct_h);

    p_nat_global->nfct_handle = (void*)nfct_h;
#endif
#endif
#endif
    return;
}

#endif

#if _DESC("nat session process functions")
/***************************************************************************************************
 * Name         : nat_session_ttl_get
 * Purpose      : get nat session ttl configuration by ip protocol
 * Input        : uint8 ip_proto
 * Output       : N/A                  
 * Return       : ttl value
 * Note         : N/A
***************************************************************************************************/
uint32 nat_session_ttl_get(uint8 ip_proto)
{
    tbl_nat_global_t *p_global = tbl_nat_global_get_nat_global();

    switch (ip_proto)
    {
        case IPPROTO_UDP:
        {
            if (0 != p_global->timeout_udp)
            {
                return p_global->timeout_udp;
            }
            break;
        }
        case IPPROTO_TCP:
        {
            if (0 != p_global->timeout_tcp)
            {
                return p_global->timeout_tcp;
            }
            break;
        }
        case IPPROTO_ICMP:
        {
            if (0 != p_global->timeout_icmp)
            {
                return p_global->timeout_icmp;
            }
            break;
        }
    }

    return p_global->timeout;
}



/***************************************************************************************************
 * Name         : nat_session_global_ttl_init
 * Purpose      : set all timeout value to default
 * Input        : N/A
 * Output       : N/A                  
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void nat_session_global_ttl_init()
{
    tbl_nat_global_t *p_global = tbl_nat_global_get_nat_global();

    p_global->timeout = NAT_SESSION_TTL_DEFAULT;
    tbl_nat_global_set_nat_global_field(p_global, TBL_NAT_GLOBAL_FLD_TIMEOUT);
    
    p_global->timeout_icmp = 0;
    tbl_nat_global_set_nat_global_field(p_global, TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP);
    
    p_global->timeout_tcp = 0;
    tbl_nat_global_set_nat_global_field(p_global, TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP);
    
    p_global->timeout_udp = 0;
    tbl_nat_global_set_nat_global_field(p_global, TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP);
    return;
}

/***************************************************************************************************
 * Name         : nat_session_spec_init
 * Purpose      : set max entries spec
 * Input        : N/A
 * Output       : N/A                  
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void nat_session_spec_init()
{
    tbl_nat_global_t *p_global = tbl_nat_global_get_nat_global();

    p_global->session_limit = NAT_SESSION_SPEC;
    tbl_nat_global_set_nat_global_field(p_global, TBL_NAT_GLOBAL_FLD_SESSION_LIMIT);
    return;
}

/***************************************************************************************************
 * Name         : nat_session_limit_increase
 * Purpose      : when add new session, increase nat session limit counter. if counter full, return err.
 * Input        : tbl_nat_session_t *p_session
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_session_limit_increase(tbl_nat_session_t *p_session)
{
    tbl_nat_session_limit_key_t session_limit_key;
    tbl_nat_session_limit_t    *p_session_limit_node = NULL;
    tbl_nat_global_t           *p_global = NULL;
    uint32 curr_cnt = 0;

    p_global = tbl_nat_global_get_nat_global();

    /* total NAT sessions beyond limit*/
    curr_cnt = p_global->session_cnt_upstream + p_global->session_cnt_dnstream;
    if (curr_cnt >= p_global->session_limit)
    {
        nat_nft_connlimit_set_global(0);
        return PM_E_RESOURCE_FULL;
    }

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            p_global->session_cnt_upstream++;
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            p_global->session_cnt_upstream++;
            break;
        }
    }
    /* the host NAT sessions beyond limit*/
    sal_memcpy(&session_limit_key, &p_session->key.inside_local_ip, sizeof(addr_t));
    p_session_limit_node = tbl_nat_session_limit_get_nat_session_limit(&session_limit_key);
    if (p_session_limit_node)
    {
        curr_cnt = p_session_limit_node->count_upstream + p_session_limit_node->count_dnstream;
        if (curr_cnt >= p_session_limit_node->limit)
        {
            nat_nft_connlimit_set(p_session_limit_node, 0);
            return PM_E_RESOURCE_FULL;
        }

        switch (p_session->creator)
        {
            case GLB_NAT_SESSION_CREATOR_INSIDE:
            {
                p_session_limit_node->count_upstream++;
                break;
            }
            case GLB_NAT_SESSION_CREATOR_OUTSIDE:
            {
                p_session_limit_node->count_dnstream++;
                break;
            }
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_session_limit_decrease
 * Purpose      : when delete session, decrease nat session limit counter. 
 * Input        : tbl_nat_session_key_t *p_session_key
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int nat_session_limit_decrease(tbl_nat_session_t *p_session)
{
    tbl_nat_session_limit_key_t session_limit_key;
    tbl_nat_session_limit_t *p_session_limit_node = NULL;
    tbl_nat_global_t *p_global = NULL;
    uint32 curr_cnt = 0;

    p_global = tbl_nat_global_get_nat_global();

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            if (p_global->session_cnt_upstream > 0)
            {
                p_global->session_cnt_upstream--;
            }
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            if (p_global->session_cnt_dnstream > 0)
            {
                p_global->session_cnt_upstream--;
            }
            break;
        }
    }

    /* update global connlimit*/
    curr_cnt = p_global->session_cnt_upstream + p_global->session_cnt_dnstream;
    if (p_global->session_limit == (curr_cnt +1))
    {
        nat_nft_connlimit_set_global(p_global->session_limit);
    }

    /* the host NAT sessions beyond limit*/
    sal_memcpy(&session_limit_key, &p_session->key.inside_local_ip, sizeof(addr_t));
    p_session_limit_node = tbl_nat_session_limit_get_nat_session_limit(&session_limit_key);
    if (p_session_limit_node)
    {
        switch (p_session->creator)
        {
            case GLB_NAT_SESSION_CREATOR_INSIDE:
            {
                if (p_session_limit_node->count_upstream > 0)
                {
                    p_session_limit_node->count_upstream--;
                }
                break;
            }
            case GLB_NAT_SESSION_CREATOR_OUTSIDE:
            {
                if (p_session_limit_node->count_dnstream > 0)
                {
                    p_session_limit_node->count_dnstream--;
                }
                break;
            }
        }

        /* update IP connlimit*/
        curr_cnt = p_session_limit_node->count_upstream + p_session_limit_node->count_dnstream;
        if (p_session_limit_node->limit == (curr_cnt+1))
        {
            nat_nft_connlimit_set(p_session_limit_node, p_session_limit_node->limit);
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_session_hit
 * Purpose      : entry hit, notify kernel
 * Input        : tbl_nat_session_t *p_session
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_session_hit(tbl_nat_session_t *p_session)
{
    char str[256];
    tbl_nat_global_t *p_nat_global = NULL;

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;    
    }

    NAT_TIMER_DEBUG("nat session hit. session %s.", cdb_nat_session_key_val2str(str, 256, &p_session->key));

    /* hit, update kernel ttl every 8 minutes*/
    nat_nfct_set_ttl_by_session(p_session, NAT_NFCT_FAST_PATH_FWD_TTL_DEFAULT);
    p_nat_global->session_aging_upd_ttl_cnt++;
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_session_struct_init
 * Purpose      : init one nat session data struct
 * Input        : tbl_nat_session_t *p_session
 * Output       : tbl_nat_session_t *p_session
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void nat_session_struct_init(tbl_nat_session_t *p_session)
{
    sal_memset(p_session, 0, sizeof(tbl_nat_session_t));
    p_session->key.vrf_id = DEFAULT_VRF_ID;
    return;
}

/***************************************************************************************************
 * Name         : nat_session_add
 * Purpose      : add one nat session
 * Input        : tbl_nat_session_t *p_session
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_session_add(tbl_nat_session_t *p_session)
{
    char str[256];
    tbl_nat_global_t *p_nat_global = NULL;

    if (tbl_nat_session_get_nat_session(&p_session->key))
    {
        NAT_EVENT_DEBUG("Add exist session, key:%s.", cdb_nat_session_key_val2str(str, 256, &p_session->key));
        if (GLB_NAT_SET_SAI_STATUS_FAILED == p_session->set_sai_status)
        {
            /* notify HSRV set SAI again.*/
            p_session->set_sai_status = GLB_NAT_SET_SAI_AGAIN;
            tbl_nat_session_set_nat_session_field(p_session, TBL_NAT_SESSION_FLD_SET_SAI_STATUS);
        }
        return PM_E_NONE;
    }

    p_nat_global = tbl_nat_global_get_nat_global();
    p_nat_global->nfct_kernel_notify_add_cnt++;

    if (nat_session_limit_increase(p_session))
    {
        nat_nfct_del_by_session(p_session);
        NAT_EVENT_DEBUG("nat session add, beyond limit %s.", cdb_nat_session_key_val2str(str, 256, &p_session->key));
        return PM_E_RESOURCE_FULL;
    }

    if (ctclib_opb_alloc_offset(&p_nat_global->session_id_opb, &p_session->id))
    {
        nat_nfct_del_by_session(p_session);
        nat_session_limit_decrease(p_session);
        NAT_EVENT_DEBUG("nat session add, Alloc id failed.");
        return PM_E_RESOURCE_FULL;
    }

    return tbl_nat_session_add_nat_session(p_session);
}

/***************************************************************************************************
 * Name         : nat_session_del
 * Purpose      : del one nat session
 * Input        : tbl_nat_session_t *p_session
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_session_del(tbl_nat_session_t *p_session, bool if_need_notify_kernel)
{
    char str[256];
    tbl_nat_session_t *p_db_session = NULL;
    tbl_nat_global_t *p_nat_global = NULL;
    p_nat_global = tbl_nat_global_get_nat_global();

    p_db_session = tbl_nat_session_get_nat_session(&p_session->key);
    if (!p_db_session)
    {
        NAT_EVENT_DEBUG("Delete not exist session, key:%s.", cdb_nat_session_key_val2str(str, 256, &p_session->key));
        return PM_E_NOT_EXIST;
    }

    /* notify kernel*/
    if (TRUE == if_need_notify_kernel)
    {
        nat_nfct_del_by_session(p_db_session);
    }
    else
    {
        p_nat_global->nfct_kernel_notify_del_cnt++;
    }

    nat_session_limit_decrease(p_db_session);

    ctclib_opb_free_offset(&p_nat_global->session_id_opb, p_db_session->id);

    return tbl_nat_session_del_nat_session(&p_db_session->key);
}

/***************************************************************************************************
 * Name         : nat_session_clear_by_oif
 * Purpose      : clear sessions by oif
 * Input        : void *p_data
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void nat_session_clear_by_oif(tbl_route_if_t *p_db_rt_if)
{
    tbl_nat_session_master_t *p_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;

    p_master = tbl_nat_session_get_master();
    if (!p_master)
    {
        return;    
    }

    /* clear sessions by outside interface*/
    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_list, p_session, listnode, next)
    {
        if (p_session->oif_index == p_db_rt_if->ifindex)
        {
            nat_session_del(p_session, TRUE);
        }
    }

    return;
}
/***************************************************************************************************
 * Name         : nat_session_clear_by_iif
 * Purpose      : clear sessions by inside interface
 * Input        : tbl_route_if_t        *p_db_rt_if
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void nat_session_clear_by_iif(tbl_route_if_t        *p_db_rt_if)
{
    tbl_nat_session_master_t *p_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;

    p_master = tbl_nat_session_get_master();
    if (!p_master)
    {
        return;    
    }

    /* clear sessions by outside interface*/
    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_list, p_session, listnode, next)
    {
        if (p_session->iif_index == p_db_rt_if->ifindex)
        {
            nat_session_del(p_session, TRUE);
        }
    }

    return;
}

/***************************************************************************************************
 * Name         : nat_session_clear_all
 * Purpose      : clear all sessions
 * Input        : void
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void nat_session_clear_all()
{
    tbl_nat_session_master_t *p_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;

    p_master = tbl_nat_session_get_master();
    if (!p_master)
    {
        return;    
    }

    /* clear sessions by outside interface*/
    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_list, p_session, listnode, next)
    {
        nat_session_del(p_session, TRUE);
    }

    return;
}

/***************************************************************************************************
 * Name         : nat_session_aging_hook
 * Purpose      : forward entry not hit in ttl, delete the session
 * Input        : void *p_data
 * Output       : N/A
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_session_aging_hook(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_nat_global_t *p_nat_global = NULL;
    tbl_nat_session_t session;
    char str[256];

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;    
    }

    sal_memcpy(&session, p_msg->data, p_msg->data_len);

    NAT_TIMER_DEBUG("nat aging hook. session deleted %s.", cdb_nat_session_key_val2str(str, 256, &session.key));
    p_nat_global->session_aging_del_cnt++;
    return nat_session_del(&session, TRUE);
}

/***************************************************************************************************
 * Name         : nat_session_tbl_process_after_sync
 * Purpose      : hsrv nat set session ttl field
 * Input        : cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds
 * Output       : N/A
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_session_tbl_process_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_nat_session_t* p_session = NULL;
    tbl_nat_session_t* p_db_session = NULL;

    switch (oper)
    {
        case CDB_OPER_ADD:
            break;

        case CDB_OPER_DEL:
            break;

        case CDB_OPER_SET:        
            p_session = (tbl_nat_session_t*)p_tbl;
            if (NULL == p_session)
            {
                return PM_E_INVALID_PARAM;
            }
            p_db_session = tbl_nat_session_get_nat_session(&p_session->key);
            if (NULL == p_db_session)
            {
                return PM_E_NOT_EXIST;
            }     
            
            switch(field_id)
            {
                case TBL_NAT_SESSION_FLD_HIT:
                {
                    p_db_session->hit = p_session->hit;
                    nat_session_hit(p_db_session);
                    break;
                }
            }

            break;

        default:
            break;
    }

    return rc;
}

#endif




#include "proto.h"
#include "route.h"
#include "prefix.h"
#include "libnfnetlink/libnfnetlink.h"
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
#include "gen/tbl_nat_proxy_arp_define.h"
#include "gen/tbl_nat_proxy_arp.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"
#include "nat_api.h"
#include "nat.h"

#if _DESC("net filter tables(known as nft) process")

/*
nft add rule nat upstream position 2 counter
nft insert rule nat upstream position 2 counter


[uml: ~]$ nft describe meta iif
meta expression, datatype iface_index (network interface index) (basetype integer), 32 bits




table ip nat {
    // internal to external, matching inside interface by ‘iif’ and matching outside interface by ‘oif’ 
    chain forward {
        type filter hook postrouting priority 0; policy accept;
        meta iif {eth-0-1, eth-0-2, eth-0-5} meta oif {eth-0-9} counter mark set 0x01 accept
    }

    //dynamic NAT forward rules, and static NAT upstream forward rules
    chain postrouting {
        type nat hook postrouting priority 0; policy accept;
        meta mark != 0x01 accept
        oif eth-0-2 snat to ip saddr map { 192.168.0.2 : 9.0.0.3}
        oif eth-0-5 snat masquerade random
    }

    // static NAT downstream forward rules
    chain prerouting {
        type nat hook prerouting priority -101; policy accept;
        iif eth-0-2 dnat to ip daddr map { 9.0.0.3 : 192.168.0.2}
    }
}

*/

/***************************************************************************************************
 * Name         : nat_nft_op_chain
 * Purpose      : add or delete nft chain by "nft -f xxx" command.
                  nft bug exist, if the priority of of a chain is minus zero, 
                                 it can not be created by nft add command, 
                                 but nft -f FILE OK.

 FILE content like:
 table ip nat {
             chain st_nat_prerouting_rule_0 { type nat hook prerouting priority -101; policy accept ; } 
 }

 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32 nat_nft_chain_add(char * p_char)
{
    char szBuf[256];
    FILE *fp;

    fp = sal_fopen(NAT_NFT_CHAIN, "w+");
    if (NULL == fp)
    {
        return PM_E_FAIL;
    }

    sal_fprintf (fp, "%s", "table ip nat { \n");
    sal_fprintf (fp, "%s\n", p_char);
    sal_fprintf (fp, "%s", "}\n");
    sal_fclose(fp);

    sal_cmd_exec(szBuf, 256, "nft -f %s", NAT_NFT_CHAIN);
    return PM_E_NONE;
}

void
nat_nft_init( )
{
    char szBuf[512];
    tbl_nat_global_t *p_nat_global = NULL;

    p_nat_global = tbl_nat_global_get_nat_global();

    p_nat_global->nft_hdl = 0;
    sal_cmd_exec(szBuf, 256, "nft add table ip nat");

    p_nat_global->nft_hdl++;
    sal_cmd_exec(szBuf, 256, "nft add chain ip nat forward { type filter hook forward priority 0\\; policy accept\\;}");

    p_nat_global->nft_hdl++;
    p_nat_global->nft_hdl_upstream_if = p_nat_global->nft_hdl;
    tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF);
    sal_cmd_exec(szBuf, 256, "nft add rule nat forward counter accept");

    p_nat_global->nft_hdl++;
    szBuf[0] = 0;
    sal_snprintf(szBuf, 256, "chain postrouting { type nat hook postrouting priority 0; policy accept; }");
    nat_nft_chain_add(szBuf);/* add by nft -f command, no need '\' in file content */

    p_nat_global->nft_hdl++;
    p_nat_global->nft_hdl_upstream_filter_nat_rule = p_nat_global->nft_hdl;
    tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE);
    szBuf[0] = 0;
    sal_cmd_exec(szBuf, 256, "nft add rule ip nat postrouting meta mark != 0x01 counter accept");

    p_nat_global->nft_hdl++;
    szBuf[0] = 0;
    sal_snprintf(szBuf, 256, "chain prerouting { type nat hook prerouting priority -101; policy accept; }");
    nat_nft_chain_add(szBuf);/* add by nft -f command, no need '\' in file content */

    p_nat_global->nft_hdl++;
    szBuf[0] = 0;
    sal_snprintf(szBuf, 256, "chain postrouting_connlimit { type filter hook postrouting priority 100; policy accept; }");
    nat_nft_chain_add(szBuf);

    p_nat_global->nft_hdl++;
    p_nat_global->nft_hdl_connlimit_post = p_nat_global->nft_hdl;
    szBuf[0] = 0;
    sal_cmd_exec(szBuf, 256, "nft add rule ip nat postrouting_connlimit ct mark %u ct count over %u counter drop", 
                                                                   GLB_NAT_NFT_CT_MARK_UPSTREAM, NAT_SESSION_SPEC);

    p_nat_global->nft_hdl++;
    szBuf[0] = 0;
    sal_snprintf(szBuf, 256, "chain prerouting_connlimit { type filter hook prerouting priority 100; policy accept; }");
    nat_nft_chain_add(szBuf);/* add by nft -f command, no need '\' in file content */

    p_nat_global->nft_hdl++;
    p_nat_global->nft_hdl_connlimit_pre = p_nat_global->nft_hdl;
    szBuf[0] = 0;
    sal_cmd_exec(szBuf, 256, "nft add rule ip nat prerouting_connlimit ct mark %u ct count over %u counter drop", 
                                                                   GLB_NAT_NFT_CT_MARK_DNSTREAM, NAT_SESSION_SPEC);
    tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
    return;
}


void nat_nft_update_rtif()
{
    tbl_nat_rtif_inside_master_t  *p_inside_master = NULL;
    tbl_nat_rtif_outside_master_t *p_outside_master = NULL;
    tbl_nat_rtif_inside_t         *p_rtif_inside  = NULL;
    tbl_nat_rtif_outside_t        *p_rtif_outside = NULL;
    ctclib_slistnode_t            *listnode       = NULL;
    ctclib_slistnode_t            *next           = NULL;
    tbl_nat_global_t              *p_nat_global   = NULL;
    uint32 iif_len = 0;
    uint32 oif_len = 0;
    char iifstr[1024];
    char oifstr[1024];
    char szBuf[2048];

    p_nat_global = tbl_nat_global_get_nat_global();
    p_inside_master = tbl_nat_rtif_inside_get_master();
    p_outside_master = tbl_nat_rtif_outside_get_master();
    if ((!p_inside_master) || (!p_outside_master) || (!p_nat_global))
    {
        return;    
    }

    sal_memset(iifstr, 0, sizeof(iifstr));
    sal_memset(oifstr, 0, sizeof(oifstr));
    sal_memset(szBuf, 0, sizeof(szBuf));
    CTCLIB_SLIST_LOOP_DEL(p_inside_master->nat_rtif_inside_list, p_rtif_inside, listnode, next)
    {
        if (iif_len)
        {
            iif_len += sal_sprintf(iifstr+iif_len, ", ");
        }
        iif_len += sal_sprintf(iifstr+iif_len, "%s ", p_rtif_inside->key.name);
    }

    CTCLIB_SLIST_LOOP_DEL(p_outside_master->nat_rtif_outside_list, p_rtif_outside, listnode, next)
    {
        if (oif_len)
        {
            oif_len += sal_sprintf(oifstr+oif_len, ", ");
        }
        oif_len += sal_sprintf(oifstr+oif_len, "%s", p_rtif_outside->key.name);
    }

    if (!iif_len || !oif_len)
    {
        sal_cmd_exec(szBuf, 256, "nft replace rule nat forward handle %u counter accept", p_nat_global->nft_hdl_upstream_if);
    }
    else
    {
        sal_cmd_exec(szBuf, 256, "nft replace rule nat forward handle %u meta iifname { %s } meta oifname { %s } counter meta mark set 0x01 accept",
                             p_nat_global->nft_hdl_upstream_if, iifstr, oifstr);
    }
    return;
}


/// ip nat inside source static [tcp | udp] LOCAL-IP {GLOBAL-IP|interface} lobal-port-range
int32 nat_nft_acl_add(tbl_nat_rule_t *p_rule, char* aclname)
{
#if 0
    uint32 len = 0;
    char sz_cmd[1024];
    char szBuf[256];
    tbl_acl_config_key_t  acl_key;
    tbl_acl_config_t *p_db_acl = NULL;
    tbl_ace_config_t *p_db_ace = NULL;
    tbl_ace_config_master_t *p_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nat_global_t *p_nat_global = NULL;

    sal_memset(&acl_key, 0, sizeof(acl_key));
    sal_memcpy(acl_key.name, aclname, ACL_NAME_FULL_NAME_SIZE);
    p_db_acl = tbl_acl_config_get_acl_config(&acl_key);
    if (!p_db_acl)
    {
        return PM_E_NONE;
    }

    if (0 == p_db_acl->ace_ref)
    {
        return PM_E_NONE;
    }

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_NONE;
    }

    p_master = tbl_ace_config_get_master();
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace, listnode, next)
    {
        if (0 != sal_strcmp(p_db_ace->key.acl_name, aclname))
        {
            continue;
        }

        if (p_db_ace->deny)
        {
            continue;
        }

        sal_memset(sz_cmd, 0, sizeof(sz_cmd));
        len = 0;


        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_GRE_KEY)
         || GLB_FLAG_ISSET(p_db_ace->key_flags2, GLB_IP_ACE_ERSPAN_ID)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_VXLAN_VNI)
         || GLB_FLAG_ISSET(p_db_ace->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL)
         || GLB_FLAG_ISSET(p_db_ace->key_ipv6_flags, GLB_IPV6_ACE_IPDA)
         || GLB_FLAG_ISSET(p_db_ace->key_ipv6_flags, GLB_IPV6_ACE_IPSA)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IP_FRAG)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_MACDA)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_UDF)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_OPTIONS)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_TIME_RANGE)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_PRECEDENCE)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_UNTAG)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_MACSA)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_INNER_COS)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_INNNER_VLAN)
         || GLB_FLAG_ISSET(p_db_ace->key_arp_flags, GLB_ARP_ACE_OP_CODE)
         || GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IGMP_TYPE))
        {
            // kernel not support, ignore
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            if (0xffff != p_db_ace->ether_type_mask)
            {
                // TODO: mask process
                len += sal_sprintf(sz_cmd+len, "ether type %u ", p_db_ace->ether_type);
            }
        }

        /*
        nft add rule nat st_nat_prerouting_rule_2 vlan pcp 1-2
        ===>         @ll,0,8 & 224 == 32-64 # handle 28
        */
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_COS))
        {
            if (0x7 != p_db_ace->svlan_cos_mask)
            {
                // TODO: mask process
                len += sal_sprintf(sz_cmd+len, "vlan pcp %u ", p_db_ace->svlan_cos);
            }
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_VLAN))
        {
            if (0xfff != p_db_ace->svlan_mask)
            {
                // TODO: mask process
                len += sal_sprintf(sz_cmd+len, "vlan %u ", p_db_ace->svlan);
            }
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IP_PROTOCOL))
        {
            if (0 == p_db_ace->ip_protocol_mask)
            {
                // TODO: mask process
                len += sal_sprintf(sz_cmd+len, "ip protocol %u ", p_db_ace->ip_protocol);
            }
        }
        
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_ICMP_TYPE))
        {
            len += sal_sprintf(sz_cmd+len, "icmp type %u ", p_db_ace->icmp_type);
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_ICMP_CODE))
        {
            len += sal_sprintf(sz_cmd+len, "icmp code %u ", p_db_ace->icmp_code);
        }

        
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPSA))
        {
            uint32 tmp = 0;
            char ip_buf[GLB_IPV6_STR_LEN];

            IPV4_MASK_TO_LEN(p_db_ace->src_ip_mask.u.prefix4.s_addr, tmp);
            if (0 != tmp)
            {
                sal_memset(ip_buf, 0, sizeof(ip_buf));
                sal_inet_ntop(AF_INET, (void*)&p_db_ace->src_ip.u.prefix4, ip_buf, sizeof(ip_buf));

                len += sal_sprintf(sz_cmd+len, "ip saddr %s/%u ", ip_buf, len);
            }
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPDA))
        {
            uint32 tmp = 0;
            char ip_buf[GLB_IPV6_STR_LEN];

            IPV4_MASK_TO_LEN(p_db_ace->dst_ip_mask.u.prefix4.s_addr, tmp);
            if (0 != tmp)
            {
                sal_memset(ip_buf, 0, sizeof(ip_buf));
                sal_inet_ntop(AF_INET, (void*)&p_db_ace->dst_ip.u.prefix4, ip_buf, sizeof(ip_buf));

                len += sal_sprintf(sz_cmd+len, "ip daddr %s/%u ", ip_buf, len);
            }
        }


        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_DSCP))
        {
            if (0x3f != p_db_ace->dscp_mask)
            {
                // TODO: mask process
                len += sal_sprintf(sz_cmd+len, "ip dscp %u ", p_db_ace->dscp);
            }
        }
        
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_SRCPORT))
        {
            if ((GLB_ACL_L4_PROTOCOL_TCP == p_db_ace->ip_protocol)
             || (GLB_ACL_L4_PROTOCOL_UDP == p_db_ace->ip_protocol))
            {
                if (GLB_ACE_PORT_TYEP_EQ == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s sport %u ", nat_l3protocol_name(p_db_ace->ip_protocol), p_db_ace->l4_src_port);
                }
                if (GLB_ACE_PORT_TYEP_LT == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s sport lt %u ", nat_l3protocol_name(p_db_ace->ip_protocol), (p_db_ace->l4_src_port_mask + 1));
                }
                if (GLB_ACE_PORT_TYEP_GT == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s sport gt %u ", nat_l3protocol_name(p_db_ace->ip_protocol), (p_db_ace->l4_src_port - 1));
                }
                if (GLB_ACE_PORT_TYEP_RANGE == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s sport %u-%u ", nat_l3protocol_name(p_db_ace->ip_protocol), p_db_ace->l4_src_port, p_db_ace->l4_src_port_mask);
                }
            }
        }
        
            
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_DSTPORT))
        {
            if ((GLB_ACL_L4_PROTOCOL_TCP == p_db_ace->ip_protocol)
             || (GLB_ACL_L4_PROTOCOL_UDP == p_db_ace->ip_protocol))
            {
                if (GLB_ACE_PORT_TYEP_EQ == p_db_ace->l4_dst_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s dport %u ", nat_l3protocol_name(p_db_ace->ip_protocol), p_db_ace->l4_dst_port);
                }
                if (GLB_ACE_PORT_TYEP_LT == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s dport lt %u ", nat_l3protocol_name(p_db_ace->ip_protocol), (p_db_ace->l4_dst_port_mask + 1));
                }
                if (GLB_ACE_PORT_TYEP_GT == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s dport gt %u ", nat_l3protocol_name(p_db_ace->ip_protocol), (p_db_ace->l4_dst_port - 1));
                }
                if (GLB_ACE_PORT_TYEP_RANGE == p_db_ace->l4_src_port_type)
                {
                    len += sal_sprintf(sz_cmd+len, "%s dport %u-%u ", nat_l3protocol_name(p_db_ace->ip_protocol), p_db_ace->l4_dst_port, p_db_ace->l4_dst_port_mask);
                }
            }
        }
        
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_TCP_FLAG))
        {
            char sz_flag[64];
            char tmp = 0;

            sal_memset(sz_flag, 0, sizeof(sz_flag));
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "ack");
            }
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "fin");
            }
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "psh");
            }
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "rst");
            }
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "syn");
            }
            if GLB_FLAG_ISSET(p_db_ace->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
            {
                tmp += sal_sprintf(sz_flag+tmp, "%s ", "urg");
            }

            if (tmp)
            {
                len += sal_sprintf(sz_cmd+len, "tcp flag { %s } ", sz_flag);
            }
        }
        
        if (len)
        {
            p_nat_global->nft_hdl++;
            sal_cmd_exec(szBuf, 256, "nft add rule nat dyn_nat_postrouting_rule_%u_acl %s counter return \n", 
                                                    p_rule->id, sz_cmd);
        }
    }
    (void)listnode;
    (void)next;

    /* add default rule*/
    p_nat_global->nft_hdl++;
    sal_cmd_exec(szBuf, 256, "nft add rule nat dyn_nat_postrouting_rule_%u_acl counter accept \n");
#endif
    return PM_E_NONE;
}
int32
nat_nft_acl_del(uint32 ruleid, char* aclname)
{
    return PM_E_NONE;
}

int32 nat_nft_acl_mod(uint32 ruleid, char* aclname)
{
    // TODO :
    return PM_E_NONE;
}
/***************************************************************************************************
 * Name         : nat_nft_static_add
 * Purpose      : add one static NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_static_add(tbl_nat_rule_t *p_rule)
{
    char protocol_name[16];
    char szBuf[256];
    char ip_buf1[GLB_IPV6_STR_LEN];
    char ip_buf2[GLB_IPV6_STR_LEN];
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    if (!((p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
       || (p_rule->key.flags & GLB_NAT_RULE_FLAG_INSIDE)
       || (p_rule->key.flags & GLB_NAT_RULE_FLAG_SNAT)))
    {
        return PM_E_FAIL;
    }

    #if 0
    p_nat_global->nft_hdl++;
    szBuf[0] = 0;
    sal_cmd_exec(szBuf, 256, "nft add rule ip nat st_nat_prerouting_rule_%u meta mark != 0x02 counter return \n", p_rule->id);
    #endif

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
    {
        sal_memset(protocol_name, 0, sizeof(protocol_name));
        nat_l3protocol_name(protocol_name, 16, p_rule->key.protocol);
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP)
    {
        sal_memset(ip_buf1, 0, sizeof(ip_buf1));
        sal_inet_ntop(AF_INET, (void*)&p_rule->key.in_local_ip.u.prefix4, ip_buf1, sizeof(ip_buf1));
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
    {
        sal_memset(ip_buf2, 0, sizeof(ip_buf2));
        sal_inet_ntop(AF_INET, (void*)&p_rule->key.in_glb_ip.u.prefix4, ip_buf2, sizeof(ip_buf2));
    }
    else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        ds_connected_t  *ifc_primary = NULL;
        
        /* get IF primary IP */
        ifc_primary = nat_get_rtif_primary_connected_by_ifname(p_rule->key.oif);
        if (!ifc_primary)
        {
            return PM_E_FAIL;
        }

        sal_memset(ip_buf2, 0, sizeof(ip_buf2));
        sal_inet_ntop(AF_INET, (void*)&ifc_primary->key.address.u.prefix4, ip_buf2, sizeof(ip_buf2));
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
    {
        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
        {
        /* example:
        CNOS command: ip nat inside source static udp 192.168.0.2 555 9.0.0.2 666 
        
        nft command:
        postrouting rule: oif eth-0-2 ip saddr 192.168.0.2 udp sport 555 counter ctmark set 1 snat 192.168.0.2 : 9.0.0.2 udp sport map { 555 : 666 }
        prerouting rule: iif eth-0-2 ip daddr 9.0.0.2 udp dport 666 counter ct mark set 2 dnat 192.168.0.2 : udp dport map { 666 : 555 }
        */
            p_nat_global->nft_hdl++;
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s ip saddr %s %s sport %u counter ct mark set %u snat %s : %s sport map {%u : %u} \n", 
                                                    p_rule->refer_oif, ip_buf1, protocol_name, p_rule->key.in_local_port, 
                                                    GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf2, 
                                                    protocol_name, p_rule->key.in_local_port, p_rule->key.in_glb_port);
            p_nat_global->nft_hdl++;
            p_rule->handle_prerouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat prerouting iif %s ip daddr %s %s dport %u counter ct mark set %u dnat %s : %s dport map {%u : %u} \n", 
                                                    p_rule->refer_oif, ip_buf2, protocol_name, p_rule->key.in_glb_port, 
                                                    GLB_NAT_NFT_CT_MARK_DNSTREAM, ip_buf1, 
                                                    protocol_name, p_rule->key.in_glb_port, p_rule->key.in_local_port);
        }
        else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI)
        {
            /* example:
            CNOS command: ip nat inside source static udp 192.168.0.2 9.0.0.2 200-300
            
            nft command:
            postrouting rule: oif eth-0-2 ip saddr 192.168.0.2 udp sport 200-300 counter ct mark set 1 snat ip saddr map { 192.168.0.2 : 9.0.0.2 }
            prerouting rule: iif eth-0-2 ip daddr 9.0.0.2 udp dport 200-300 counter ct mark set 2 dnat ip daddr map { 9.0.0.2 : 192.168.0.2 }
            */
            p_nat_global->nft_hdl++;
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s ip saddr %s %s sport %u-%u counter ct mark set %u snat ip saddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf1, protocol_name, p_rule->key.in_glb_port, p_rule->key.in_glb_port_hi, 
                                                    GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
            p_nat_global->nft_hdl++;
            p_rule->handle_prerouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat prerouting iif %s ip daddr %s %s dport %u-%u counter ct mark set %u dnat ip daddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf2, 
                                                    protocol_name, p_rule->key.in_glb_port, p_rule->key.in_glb_port_hi, 
                                                    GLB_NAT_NFT_CT_MARK_DNSTREAM, ip_buf2, ip_buf1);
        }
        else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
        {
            /* example:
            CNOS command: ip nat inside source static udp 192.168.0.2 9.0.0.2 200
            
            nft command:
            postrouting rule: oif eth-0-2 ip saddr 192.168.0.2 udp sport 200 counter ct mark set 1 snat ip saddr map { 192.168.0.2 : 9.0.0.2 }
            prerouting rule: iif eth-0-2 ip daddr 9.0.0.2 udp dport 200 counter ct mark set 2 dnat ip daddr map { 9.0.0.2 : 192.168.0.2 }
            */
            p_nat_global->nft_hdl++;
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s ip saddr %s %s sport %u counter ct mark set %u snat ip saddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf1, protocol_name, p_rule->key.in_glb_port, 
                                                    GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
            p_nat_global->nft_hdl++;
            p_rule->handle_prerouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat prerouting iif %s ip daddr %s %s dport %u counter ct mark set %u dnat ip daddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf2, protocol_name, p_rule->key.in_glb_port, 
                                                    GLB_NAT_NFT_CT_MARK_DNSTREAM, ip_buf2, ip_buf1);
        }
        else
        {
            /* example:
            CNOS command: ip nat inside source static udp 192.168.0.2 9.0.0.2
            
            nft command:
            postrouting rule: oif eth-0-2 ip saddr 192.168.0.2 ip protocol udp counter ct mark set 1 snat ip saddr map { 192.168.0.2 : 9.0.0.2 }
            prerouting rule: iif eth-0-2 ip daddr 9.0.0.2 ip protocol udp counter ct mark set 2 dnat ip daddr map { 9.0.0.2 : 192.168.0.2 }
            */
            p_nat_global->nft_hdl++;
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s ip saddr %s ip protocol %s counter ct mark set %u snat ip saddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf1, protocol_name, 
                                                    GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
            
            p_nat_global->nft_hdl++;
            p_rule->handle_prerouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat prerouting iif %s ip daddr %s ip protocol %s counter ct mark set %u dnat ip daddr map {%s : %s} \n", 
                                                    p_rule->refer_oif, ip_buf2, protocol_name, 
                                                    GLB_NAT_NFT_CT_MARK_DNSTREAM, ip_buf2, ip_buf1);
        }
        
    }
    else
    {
        /* example:
        CNOS command: ip nat inside source static 192.168.0.2 9.0.0.2
        
        nft command:
        postrouting rule: oif eth-0-2 ip saddr 192.168.0.2 counter ct mark set 1 snat ip saddr map { 192.168.0.2 : 9.0.0.2 }
        prerouting rule: iif eth-0-2 ip daddr 9.0.0.2 counter ct mark set 2 dnat ip daddr map { 9.0.0.2 : 192.168.0.2 }
        */
        p_nat_global->nft_hdl++;
        p_rule->handle_postrouting = p_nat_global->nft_hdl;
        sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s ip saddr %s counter ct mark set %u snat ip saddr map {%s : %s} \n", 
                                                p_rule->refer_oif, ip_buf1, 
                                                GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
        
        p_nat_global->nft_hdl++;
        p_rule->handle_prerouting = p_nat_global->nft_hdl;
        sal_cmd_exec(szBuf, 256, "nft add rule nat prerouting iif %s ip daddr %s counter ct mark set %u dnat ip daddr map {%s : %s} \n", 
                                                p_rule->refer_oif, ip_buf2, 
                                                GLB_NAT_NFT_CT_MARK_DNSTREAM, ip_buf2, ip_buf1);
    }

    tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nft_static_del
 * Purpose      : add one static NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_static_del(tbl_nat_rule_t *p_rule)
{
    char szBuf[256];

    if (p_rule->handle_postrouting)
    {
        sal_cmd_exec(szBuf, 256, "nft delete rule nat postrouting handle %u", p_rule->handle_postrouting);
    }

    if (p_rule->handle_prerouting)
    {
        sal_cmd_exec(szBuf, 256, "nft delete rule nat prerouting handle %u", p_rule->handle_prerouting);
    }
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nft_dynamic_add
 * Purpose      : add one dynamic NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_dynamic_add(tbl_nat_rule_t *p_rule)
{
    char szBuf[256];
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    if (!((p_rule->key.flags & GLB_NAT_RULE_FLAG_INSIDE)
       || (p_rule->key.flags & GLB_NAT_RULE_FLAG_SNAT)))
    {
        return PM_E_FAIL;
    }

    // TODO: add kernel acl rule
#if 0
    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
    {
        /* add acl chain, check if already exist*/
        uint8 exist = FALSE;
        tbl_nat_rule_master_t *p_master = NULL;
        ctclib_slistnode_t *listnode = NULL;
        ctclib_slistnode_t *next = NULL;
        tbl_nat_rule_t *p_db_nat_rule = NULL;

        p_master = tbl_nat_rule_get_master();
        if (NULL == p_master)
        {
            return PM_E_NONE;
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->nat_rule_list, p_db_nat_rule, listnode, next)
        {
            if ((p_db_nat_rule->id != p_rule->id)
             && (0 == sal_memcmp(p_db_nat_rule->key.acl, p_rule->key.acl, ACL_NAME_FULL_NAME_SIZE)))
            {
                exist = TRUE;
                break;
            }
        }
        (void)listnode;
        (void)next;

        if (FALSE == exist)
        {
            p_nat_global->nft_hdl++;
            p_rule->nft_chain_id_acl = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add chain nat dyn_nat_postrouting_rule_%u_acl \n", p_rule->id);
            nat_nft_acl_add(p_rule, p_rule->key.acl);
        }
    }
#endif

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
        {
            // TODO:
        }
         
        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_RANDOM)
        {
            p_nat_global->nft_hdl++;
            tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting meta oif %s counter ct mark set %u masquerade random", 
                                                                          p_rule->key.oif, GLB_NAT_NFT_CT_MARK_UPSTREAM);
        }
        else
        {
            p_nat_global->nft_hdl++;
            tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting meta oif %s counter ct mark set %u masquerade", 
                                                                          p_rule->key.oif, GLB_NAT_NFT_CT_MARK_UPSTREAM);
        }
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_POOL)
    {
        tbl_nat_pool_key_t nat_pool_key;
        tbl_nat_pool_t *p_pool = NULL;
        char ip_buf1[GLB_IPV6_STR_LEN];
        char ip_buf2[GLB_IPV6_STR_LEN];

        sal_memset(&nat_pool_key, 0, sizeof(nat_pool_key));
        sal_memcpy(nat_pool_key.name, p_rule->key.pool, GLB_NAT_POOL_NAME_LEN); 
        p_pool = tbl_nat_pool_get_nat_pool(&nat_pool_key);
        if (!p_pool)
        {
            return PM_E_FAIL;
        }

        sal_memset(ip_buf1, 0, sizeof(ip_buf1));
        sal_memset(ip_buf2, 0, sizeof(ip_buf2));
        sal_inet_ntop(AF_INET, (void*)&p_pool->ip_lo.u.prefix4, ip_buf1, sizeof(ip_buf1));
        sal_inet_ntop(AF_INET, (void*)&p_pool->ip_hi.u.prefix4, ip_buf2, sizeof(ip_buf2));

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
        {
        #if 0
            p_nat_global->nft_hdl++;
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat dyn_nat_postrouting_rule_%u counter jump dyn_nat_postrouting_rule_%u_acl \n",
                                     p_rule->id, p_rule->id);
        #endif
        }

        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_RANDOM)
        {
            p_nat_global->nft_hdl++;
            tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s counter ct mark set %u snat %s-%s random", 
                                                    p_rule->refer_oif, GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
        }
        else
        {
            p_nat_global->nft_hdl++;
            tbl_nat_global_set_nat_global_field(p_nat_global, TBL_NAT_GLOBAL_FLD_NFT_HDL);
            p_rule->handle_postrouting = p_nat_global->nft_hdl;
            sal_cmd_exec(szBuf, 256, "nft add rule nat postrouting oif %s counter ct mark set %u snat %s-%s", 
                                                    p_rule->refer_oif, GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf1, ip_buf2);
        }
    }
    
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nft_dynamic_del
 * Purpose      : delete one dynamic NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_dynamic_del(tbl_nat_rule_t *p_rule)
{
    char szBuf[256];
    tbl_nat_rule_t * p_rule_node = tbl_nat_rule_get_nat_rule(&p_rule->key);

    if (!p_rule_node)
    {
        return PM_E_NOT_EXIST;
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
    {
    #if 0
        /* del acl chain, check if referenced by other rule*/
        uint8 exist = FALSE;
        tbl_nat_rule_master_t *p_master = NULL;
        ctclib_slistnode_t *listnode = NULL;
        ctclib_slistnode_t *next = NULL;
        tbl_nat_rule_t *p_db_nat_rule = NULL;

        p_master = tbl_nat_rule_get_master();
        if (NULL == p_master)
        {
            return PM_E_NONE;
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->nat_rule_list, p_db_nat_rule, listnode, next)
        {
            if ((p_db_nat_rule->id != p_rule->id)
             && (0 == sal_memcmp(p_db_nat_rule->key.acl, p_rule->key.acl, ACL_NAME_FULL_NAME_SIZE)))
            {
                exist = TRUE;
                break;
            }
        }

        if (FALSE == exist)
        {
            sal_cmd_exec(szBuf, 256, "nft flush chain nat dyn_nat_postrouting_rule_%u_acl \n", p_rule->id);
            sal_cmd_exec(szBuf, 256, "nft delete chain nat dyn_nat_postrouting_rule_%u_acl \n", p_rule->id);
        }
    #endif
    }
    sal_cmd_exec(szBuf, 256, "nft delete rule nat postrouting handle %u", p_rule->handle_postrouting);
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_nft_connlimit_set_global
 * Purpose      : set global connlimit
 * Input        : uint32 limit
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_connlimit_set_global(uint32 limit)
{
    char szBuf[256];
    tbl_nat_global_t *p_glb = NULL;

    p_glb = tbl_nat_global_get_nat_global();

    /* total limit*/
    sal_cmd_exec(szBuf, 256, "nft replace rule ip nat postrouting_connlimit handle %u ct mark %u ct count over %u counter drop", 
                                      p_glb->nft_hdl_connlimit_post, GLB_NAT_NFT_CT_MARK_UPSTREAM, limit);

    sal_cmd_exec(szBuf, 256, "nft replace rule ip nat prerouting_connlimit handle %u ct mark %u ct count over %u counter drop", 
                                      p_glb->nft_hdl_connlimit_pre, GLB_NAT_NFT_CT_MARK_DNSTREAM, limit);
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_nft_connlimit_set
 * Purpose      : set connlimit per address
 * Input        : tbl_nat_session_limit_t *p_session_limit
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_connlimit_set(tbl_nat_session_limit_t *p_session_limit_node, uint32 limit)
{
    char szBuf[256];
    char ip_buf[GLB_IPV6_STR_LEN];
    tbl_nat_global_t *p_glb = NULL;

    p_glb = tbl_nat_global_get_nat_global();

    sal_memset(ip_buf, 0, sizeof(ip_buf));
    sal_inet_ntop(AF_INET, (void*)&p_session_limit_node->key.u.prefix4, ip_buf, sizeof(ip_buf));

    if (0 != p_session_limit_node->nft_hdl_post)
    {
        sal_cmd_exec(szBuf, 256, "nft replace rule ip nat postrouting_connlimit hanlde %u ct original saddr %s ct mark %u ct count over %u counter drop", 
                                          p_session_limit_node->nft_hdl_post, GLB_NAT_NFT_CT_MARK_UPSTREAM, ip_buf, limit);
    }
    else
    {
        p_glb->nft_hdl++;
        p_session_limit_node->nft_hdl_post = p_glb->nft_hdl;
        sal_cmd_exec(szBuf, 256, "nft add rule ip nat postrouting_connlimit ct original saddr %s ct mark %u ct count over %u counter drop", 
                                          ip_buf, GLB_NAT_NFT_CT_MARK_UPSTREAM, limit);
    }
    tbl_nat_global_set_nat_global_field(p_glb, TBL_NAT_GLOBAL_FLD_NFT_HDL);
    
    if (0 != p_session_limit_node->nft_hdl_pre)
    {
        sal_cmd_exec(szBuf, 256, "nft replace rule ip nat prerouting_connlimit handle %u ct reply saddr %s ct mark %u ct count over %u counter drop", 
                                          p_session_limit_node->nft_hdl_pre, ip_buf, GLB_NAT_NFT_CT_MARK_DNSTREAM, limit);
    }
    else
    {
        p_glb->nft_hdl++;
        p_session_limit_node->nft_hdl_pre = p_glb->nft_hdl;
        sal_cmd_exec(szBuf, 256, "nft add rule ip nat prerouting_connlimit ct reply saddr %s ct mark %u ct count over %u counter drop", 
                                          ip_buf, GLB_NAT_NFT_CT_MARK_DNSTREAM, limit);
    }
    tbl_nat_global_set_nat_global_field(p_glb, TBL_NAT_GLOBAL_FLD_NFT_HDL);
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_nft_connlimit_unset
 * Purpose      : set connlimit per address
 * Input        : tbl_nat_session_limit_t *p_session_limit
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_nft_connlimit_unset(tbl_nat_session_limit_t *p_session_limit_node)
{
    char szBuf[256];

    sal_cmd_exec(szBuf, 256, "nft delete rule ip nat postrouting_connlimit handle %u", p_session_limit_node->nft_hdl_post);
    p_session_limit_node->nft_hdl_post = 0;
    
    sal_cmd_exec(szBuf, 256, "nft delete rule ip nat prerouting_connlimit handle %u",  p_session_limit_node->nft_hdl_pre);
    p_session_limit_node->nft_hdl_pre = 0;
    return PM_E_NONE;
}

#endif

#if _DESC("configure NAT pool or NAT rule with inside global IP, enable arp proxy for these IPs")
/***************************************************************************************************
 * Name         : nat_proxy_arp_add
 * Purpose      : add proxy arp entries when configure inside global IP, from NAT rule or NAT pool
 * Input        : char *if_name, addr_ipv4_t *p_addr_lo, addr_ipv4_t *p_addr_hi
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_proxy_arp_add(char *if_name, addr_ipv4_t *p_addr_lo, addr_ipv4_t *p_addr_hi)
{
    uint32 ip = 0;
    uint32 i = 0;
    uint32 start, end;
    char   str[64];
    tbl_nat_proxy_arp_t porxy_arp;
    tbl_route_if_t *p_rt_if = NULL;
    tbl_route_if_key_t if_key;

    sal_memcpy(if_key.name, if_name, IFNAME_SIZE);
    p_rt_if = tbl_route_if_get_route_if(&if_key);
    if (!p_rt_if)
    {
        return PM_E_FAIL;
    }

    start = sal_hton32(p_addr_lo->s_addr);
    end = sal_hton32(p_addr_hi->s_addr);
    for (i = start; i <= end; i++)
    {
        ip = sal_ntoh32(i);

        sal_memset(&porxy_arp, 0, sizeof(porxy_arp));
        porxy_arp.key.ifindex = p_rt_if->ifindex;
        porxy_arp.key.inside_global_ip.family = AF_INET;
        porxy_arp.key.inside_global_ip.u.prefix4.s_addr = ip;
        porxy_arp.enable = TRUE;

        NAT_EVENT_DEBUG("proxy arp add. %s", cdb_addr_ipv4_val2str(str, 64, &porxy_arp.key.inside_global_ip.u.prefix4));
        tbl_nat_proxy_arp_add_nat_proxy_arp(&porxy_arp);
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_proxy_arp_delete
 * Purpose      : del proxy arp entries when delete inside global IP, from NAT rule or NAT pool
 * Input        : char *if_name, addr_ipv4_t *p_addr_lo, addr_ipv4_t *p_addr_hi
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_proxy_arp_delete(char *if_name, addr_ipv4_t *p_addr_lo, addr_ipv4_t *p_addr_hi)
{
    uint32 ip = 0;
    uint32 i = 0;
    uint32 start, end;
    char   str[64];
    tbl_nat_proxy_arp_t porxy_arp;
    tbl_route_if_t *p_rt_if = NULL;
    tbl_route_if_key_t if_key;

    sal_memcpy(if_key.name, if_name, IFNAME_SIZE);
    p_rt_if = tbl_route_if_get_route_if(&if_key);
    if (!p_rt_if)
    {
        return PM_E_FAIL;
    }

    start = sal_hton32(p_addr_lo->s_addr);
    end = sal_hton32(p_addr_hi->s_addr);
    for (i = start; i <= end; i++)
    {
        ip = sal_ntoh32(i);

        sal_memset(&porxy_arp, 0, sizeof(porxy_arp));
        porxy_arp.key.ifindex = p_rt_if->ifindex;
        porxy_arp.key.inside_global_ip.family = AF_INET;
        porxy_arp.key.inside_global_ip.u.prefix4.s_addr = ip;

        NAT_EVENT_DEBUG("proxy arp del. %s", cdb_addr_ipv4_val2str(str, 64, &porxy_arp.key.inside_global_ip.u.prefix4));
        tbl_nat_proxy_arp_del_nat_proxy_arp(&porxy_arp.key);
    }

    return PM_E_NONE;
}
#endif


#if _DESC("nat pool process functions")
void
nat_pool_struct_init(tbl_nat_pool_t *p_pool)
{
    sal_memset(p_pool, 0, sizeof(tbl_nat_pool_t));
    return;
}

/***************************************************************************************************
 * Name         : nat_cmd_pool_if_addr_used
 * Purpose      : check if the address used by any pool
 * Input        : addr_ipv4_t *p_addr
 * Output       : char *pool_name
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
bool
nat_pool_if_addr_used(addr_ipv4_t *p_addr, char *pool_name)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_pool_master_t *p_pool_master = NULL;
    tbl_nat_pool_t        *p_db_pool = NULL;
    uint32   addr_val, hi_val, lo_val;

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return FALSE;
    }

    addr_val = sal_hton32(p_addr->s_addr);
    CTCLIB_SLIST_LOOP_DEL(p_pool_master->nat_pool_list, p_db_pool, listnode, next)
    {
        hi_val = sal_hton32(p_db_pool->ip_hi.u.prefix4.s_addr);
        lo_val = sal_hton32(p_db_pool->ip_lo.u.prefix4.s_addr);
        if ((addr_val >= lo_val) && (addr_val <= hi_val))
        {
            sal_memcpy(pool_name, p_db_pool->key.name, GLB_NAT_POOL_NAME_LEN);
            return TRUE;
        }
    }

    return FALSE;
}


/***************************************************************************************************
 * Name         : nat_pool_get_outside_rtif
 * Purpose      : get outside if by pool Address subnet
 * Input        : tbl_nat_pool_t *p_pool
 * Output       : char *if_name
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_pool_get_outside_rtif(tbl_nat_pool_t *p_pool, addr_ipv4_t *p_except_addr, char *if_name)
{
    tbl_route_if_master_t *p_if_master = NULL;
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    ctclib_slistnode_t    *listnode_sub= NULL;
    ctclib_slistnode_t    *next_sub    = NULL;
    tbl_route_if_t        *p_rt_if     = NULL;
    tbl_route_if_t        *p_rt_if_found= NULL;
    ds_connected_t        *p_db_conn   = NULL;
    prefix_t              *p_prefix    = NULL;
    prefix_t               prefix_for_cmp;

    p_if_master = tbl_route_if_get_master();
    if (!p_if_master)
    {
        return PM_E_FAIL;
    }

    /* prepare prefix_for_cmp*/
    prefix_for_cmp.family = AF_INET;
    IPV4_MASK_TO_LEN(p_pool->ip_mask.u.prefix4.s_addr, prefix_for_cmp.prefixlen);
    sal_memcpy(&prefix_for_cmp.u.prefix4, &p_pool->ip_lo.u.prefix4, sizeof(addr_ipv4_t));

    /* scan route interface, find the subnet, but should not conflict with any interface IP*/
    CTCLIB_SLIST_LOOP_DEL(p_if_master->if_list, p_rt_if, listnode, next)
    {
        if (GLB_IF_NAT_OUTSIDE != p_rt_if->nat_enable)
        {
            continue;
        }

        CTCLIB_SLIST_LOOP_DEL(p_rt_if->ifc_ipv4.obj_list, p_db_conn, listnode_sub, next_sub)
        {
            p_prefix = &p_db_conn->key.address;

            /* check if except*/
            if (p_except_addr)
            {
                if (p_except_addr->s_addr == p_prefix->u.prefix4.s_addr)
                {
                    continue;
                }
            }

            if (prefix_compare_net(p_prefix, &prefix_for_cmp)) 
            {
                /* same subnet, found one*/
                uint32 hi_value = sal_hton32(p_pool->ip_hi.u.prefix4.s_addr);
                uint32 lo_value = sal_hton32(p_pool->ip_lo.u.prefix4.s_addr);
                uint32 conn_value = sal_hton32(p_prefix->u.prefix4.s_addr);

                /* check if addresses in the pool include ifc_conn */
                if ((conn_value >= lo_value) && (conn_value <= hi_value))
                {
                    sal_memcpy(if_name, p_rt_if->key.name, IFNAME_SIZE);
                    return PM_E_CFG_CONFLICT;
                }

                /* if found more than one ifc, they must be under the same rif*/
                if (!p_rt_if_found)
                {
                    p_rt_if_found = p_rt_if;
                }
            }
        }
    }

    if (!p_rt_if_found)
    {
        /* not found*/
        return PM_E_NOT_EXIST;
    }

    sal_memcpy(if_name, p_rt_if_found->key.name, IFNAME_SIZE);
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_pool_proc_rtif_nat_disable
 * Purpose      : delete one dynamic NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_pool_proc_rtif_nat_disable(tbl_route_if_t      *p_rt_if)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_pool_master_t *p_pool_master = NULL;
    tbl_nat_pool_t        *p_db_pool = NULL;

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return PM_E_FAIL;
    }


    CTCLIB_SLIST_LOOP_DEL(p_pool_master->nat_pool_list, p_db_pool, listnode, next)
    {
        if ((0 != p_db_pool->refer_oif[0]) && (0 == sal_memcmp(p_db_pool->refer_oif, p_rt_if->key.name, IFNAME_SIZE)))
        {
            nat_pool_del(p_db_pool);
            continue;
        }
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_pool_proc_rtif_addr_add
 * Purpose      : 
 * Input        : tbl_route_if_t *p_rt_if, addr_ipv4_t *p_addr
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
int32
nat_pool_proc_rtif_addr_add(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_pool_master_t *p_pool_master = NULL;
    tbl_nat_pool_t        *p_db_pool = NULL;
    uint32   addr_val, hi_val, lo_val;

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return PM_E_NONE;
    }

    addr_val = sal_hton32(p_prefix->u.prefix4.s_addr);
    CTCLIB_SLIST_LOOP_DEL(p_pool_master->nat_pool_list, p_db_pool, listnode, next)
    {
        hi_val = sal_hton32(p_db_pool->ip_hi.u.prefix4.s_addr);
        lo_val = sal_hton32(p_db_pool->ip_lo.u.prefix4.s_addr);
        if ((addr_val >= lo_val) && (addr_val <= hi_val))
        {
            /* conflicted, delete the pool*/
            nat_pool_del(p_db_pool);
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_pool_proc_rtif_addr_del
 * Purpose      : 
 * Input        : tbl_route_if_t *p_rt_if, addr_ipv4_t *p_addr
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
int32
nat_pool_proc_rtif_addr_del(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_pool_master_t *p_pool_master = NULL;
    tbl_nat_pool_t        *p_db_pool = NULL;
    prefix_t   prefix_for_cmp;
    char if_name[IFNAME_SIZE];

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_pool_master->nat_pool_list, p_db_pool, listnode, next)
    {
        /* prepare prefix_for_cmp*/
        prefix_for_cmp.family = AF_INET;
        IPV4_MASK_TO_LEN(p_db_pool->ip_mask.u.prefix4.s_addr, prefix_for_cmp.prefixlen);
        sal_memcpy(&prefix_for_cmp.u.prefix4, &p_db_pool->ip_lo.u.prefix4, sizeof(addr_ipv4_t));
        if (!prefix_compare_net(p_prefix, &prefix_for_cmp))
        {
            /* none of this pool's business*/
            continue;
        }

        if_name[0] = 0;
        if (PM_E_NOT_EXIST == nat_pool_get_outside_rtif(p_db_pool, &p_prefix->u.prefix4, if_name))
        {
            /* no other outside if, delete the pool*/
            nat_pool_del(p_db_pool);
        }
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_pool_del_proc_reference
 * Purpose      : delete any rule that refer this pool
 * Input        : cfg_cmd_para_t *para
                  tbl_nat_pool_t *p_pool
 * Output       : char *if_name
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_pool_del_proc_reference(tbl_nat_pool_t *p_pool)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_rule        = NULL;
    uint32 del_cnt = 0;
    char   str[256];

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_rule, listnode, next)
    {
        if (0 == sal_memcmp(p_rule->key.pool, p_pool->key.name, GLB_NAT_POOL_NAME_LEN))
        {
            del_cnt++;
            nat_rule_del(p_rule);
            NAT_EVENT_DEBUG("delete pool %s, delete related rule %s.", p_pool->key.name, cdb_nat_rule_key_val2str(str, 256, &p_rule->key));
        }
    }

    if (del_cnt)
    {
        NAT_EVENT_DEBUG("delete pool %s, delete related %u rules.", p_pool->key.name, del_cnt);
    }

    return PM_E_NONE;
}


int32
nat_pool_add(tbl_nat_pool_t *p_pool)
{
    /* notify proxy arp*/
    if (nat_proxy_arp_add(p_pool->refer_oif, &p_pool->ip_lo.u.prefix4, &p_pool->ip_hi.u.prefix4))
    {
        return PM_E_FAIL;
    }

    return tbl_nat_pool_add_nat_pool(p_pool);
}

int32
nat_pool_del(tbl_nat_pool_t *p_db_pool)
{
    /* delete related rules*/
    nat_pool_del_proc_reference(p_db_pool);

    /* notify proxy arp*/
    nat_proxy_arp_delete(p_db_pool->refer_oif, &p_db_pool->ip_lo.u.prefix4, &p_db_pool->ip_hi.u.prefix4);

    /* delete DB*/
    return tbl_nat_pool_del_nat_pool(&p_db_pool->key);
}
#endif

#if _DESC("process dynamic nat rules, notify kernel")
void
nat_rule_struct_init(tbl_nat_rule_t *p_rule)
{
    sal_memset(p_rule, 0, sizeof(tbl_nat_rule_t));
    return;
}

/***************************************************************************************************
 * Name         : nat_rule_if_local_addr_port_used
 * Purpose      : check if the address used by any rule
 * Input        : tbl_nat_rule_t *p_new_rule
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
bool
nat_rule_if_local_addr_port_used(tbl_nat_rule_t *p_new_rule)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return FALSE;
    }
/*
    not conflicted, take effects as configured order:
    ip nat inside source static udp 8.0.0.3 9.0.0.3 100-200 
    ip nat inside source static udp 8.0.0.3 9.0.0.4 100-200
    ip nat inside source static udp 8.0.0.3 9.0.0.6
    ip nat inside source static 8.0.0.3 9.0.0.5

    conflicted:
    ip nat inside source static udp 8.0.0.3 300 9.0.0.4 100 
    ip nat inside source static udp 8.0.0.3 300 9.0.0.4 200 

    conflicted:
    ip nat inside source static udp 8.0.0.3 9.0.0.4
    ip nat inside source static udp 8.0.0.3 9.0.0.5

    conflicted:
    ip nat inside source static 8.0.0.3 9.0.0.4
    ip nat inside source static 8.0.0.3 9.0.0.5
*/
    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        /* db rule not configure Local IP*/
        if (!(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP))
        {
            continue;
        }

        /* db rule configured Local IP, but not same*/
        if (p_new_rule->key.in_local_ip.u.prefix4.s_addr
         != p_db_rule->key.in_local_ip.u.prefix4.s_addr)
        {
            continue;
        }

        /* one of them configured protocol, the other one not configured.*/
        if (!(p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO))
        {
            /*db rule configured protocol, new rule not configured*/
            continue;
        }

        /* one of them configured protocol, the other one not configured.*/
        if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
        && !(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO))
        {
            continue;
        }

        /* both configured protocol, but not same, added for bug 53110*/
        if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_new_rule->key.protocol != p_db_rule->key.protocol))
        {
            continue;
        }

        /* Same local IP, both no local port configured.*/
        if (!(p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
         && !(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT))
        {
            if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
             || (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT))
            {
            /*  not conflicted:
                ip nat inside source static udp 8.0.0.3 9.0.0.4 100-200 
                ip nat inside source static udp 8.0.0.3 9.0.0.4 201-300
            */
                continue;
            }
            return TRUE;
        }

        /* both configured local Port, and same value*/
        if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
         && (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
         && (p_new_rule->key.in_local_port == p_db_rule->key.in_local_port))
        {
            return TRUE;
        }

        /* one of the two rules not configured local port, continue*/
    }

    /* not found*/
    return FALSE;
}


/***************************************************************************************************
 * Name         : nat_rule_if_global_addr_port_used
 * Purpose      : check if the address and port used by any rule
 * Input        : addr_ipv4_t *p_addr
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : if check if global IP used, set  port_lo and port_hi to 0;
                  if check if global IP and port range used, set port_lo and port_hi values;
                  if check if global IP and one port used, set port_lo value valid, set port_hi to 0.
***************************************************************************************************/
bool
nat_rule_if_global_addr_port_used(tbl_nat_rule_t *p_new_rule)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return FALSE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        /* db rule not configure glb IP*/
        if (!(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP))
        {
            continue;
        }

        /* db rule configured glb IP, but not same*/
        if (p_new_rule->key.in_glb_ip.u.prefix4.s_addr
          != p_db_rule->key.in_glb_ip.u.prefix4.s_addr)
        {
            continue;
        }

        /* one of them configured protocol, the other one not configured.*/
        if (!(p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO))
        {
            /*db rule configured protocol, new rule not configured*/
            continue;
        }

        /* one of them configured protocol, the other one not configured.*/
        if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
        && !(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO))
        {
            continue;
        }

        /* both configured protocol, but not same, added for bug 53110*/
        if ((p_new_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
         && (p_new_rule->key.protocol != p_db_rule->key.protocol))
        {
            continue;
        }

        /* Same IP, and no glb port configured, return conflicted.*/
        if ((!(p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT))
         && (!(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)))
        {
            return TRUE;
        }

        /* both configured glb Port, and same value*/
        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI)/*DB rule configure port range*/
        {
            if (p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI) /* new rule configue port range*/
            {
                /* both configure port range*/
                if ((p_new_rule->key.in_glb_port_hi >= p_db_rule->key.in_glb_port)
                 && (p_new_rule->key.in_glb_port_hi <= p_db_rule->key.in_glb_port_hi))
                {
                    /* overlap case 1*/
                    return TRUE;
                }
                if ((p_new_rule->key.in_glb_port >= p_db_rule->key.in_glb_port)
                 && (p_new_rule->key.in_glb_port <= p_db_rule->key.in_glb_port_hi))
                {
                    /* overlap case 2*/
                    return TRUE;
                }
            }
            else if (p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)/* new rule configue one port*/
            {
                if ((p_new_rule->key.in_glb_port >= p_db_rule->key.in_glb_port)
                 && (p_new_rule->key.in_glb_port <= p_db_rule->key.in_glb_port_hi))
                {
                    /* in range*/
                    return TRUE;
                }
            }
        }
        else if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)/*DB rule configure one port*/
        {
            if (p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI)
            {
                if ((p_new_rule->key.in_glb_port <= p_db_rule->key.in_glb_port)
                 && (p_new_rule->key.in_glb_port_hi >= p_db_rule->key.in_glb_port))
                {
                    /* in range*/
                    return TRUE;
                }
            }
            else if (p_new_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
            {
                if (p_new_rule->key.in_glb_port == p_db_rule->key.in_glb_port)
                {
                    /* same*/
                    return TRUE;
                }
            }
        }
    }

    /* not found*/
    return FALSE;
}

/***************************************************************************************************
 * Name         : nat_rule_if_outside_rtif_port_used
 * Purpose      : check if the interface and port used by any rule
 * Input        : tbl_route_if_t *p_rt_if, 
                  uint16 port_lo, uint16 port_hi
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : if check if outside rtif used, set  port_lo and port_hi to 0;
                  if check if outside rtif and port range used, set port_lo and port_hi values;
                  if check if outside rtif and one port used, set port_lo value valid, set port_hi to 0.
***************************************************************************************************/
bool
nat_rule_if_outside_rtif_port_used(tbl_route_if_t *p_rt_if, uint16 port_lo, uint16 port_hi)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return FALSE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        /* db rule not configure outside rtif*/
        if (!(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF))
        {
            continue;
        }

        /* db rule configured outside rtif, but not same*/
        if (0 != sal_memcmp(p_db_rule->key.oif, p_rt_if->key.name, IFNAME_SIZE))
        {
            continue;
        }

        /* db rule configured glb IP, same, and no glb port configured, return conflicted.*/
        if ((0 == port_lo) && (!(p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)))
        {
            return TRUE;
        }

        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI)/*DB rule configure port range*/
        {
            if (0 != port_hi) /* new rule configue port range*/
            {
                /* both configure port range*/
                if (((port_hi >= p_db_rule->key.in_glb_port) && (port_hi <= p_db_rule->key.in_glb_port_hi))
                 || ((port_lo >= p_db_rule->key.in_glb_port) && (port_lo <= p_db_rule->key.in_glb_port_hi)))
                {
                    return TRUE;
                }
            }
            else if (0 != port_lo)/* new rule configue one port*/
            {
                if ((port_lo >= p_db_rule->key.in_glb_port) && (port_lo <= p_db_rule->key.in_glb_port_hi))
                {
                    return TRUE;
                }
            }
        }
        else if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)/*DB rule configure one port*/
        {
            if (0 != port_hi)
            {
                if ((port_lo <= p_db_rule->key.in_glb_port) && (port_hi >= p_db_rule->key.in_glb_port))
                {
                    return TRUE;
                }
            }
            else if (0 != port_lo)
            {
                if (port_lo == p_db_rule->key.in_glb_port)
                {
                    return TRUE;
                }
            }
        }
    }

    /* not found*/
    return FALSE;
}

/***************************************************************************************************
 * Name         : nat_rule_if_nfct_related
 * Purpose      : add new rule, flush kernel related nfct.
 * Input        : nat_nfct_s *p_nfct, tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
bool nat_rule_if_nfct_related(nat_nfct_s *p_nfct, tbl_nat_rule_t *p_rule)
{
    tbl_route_if_t *p_rt_iif = NULL;
    tbl_route_if_t *p_rt_oif = NULL;
    uint8 prefixlen_iif = 0;
    uint8 prefixlen_oif = 0;

    if ((TRUE == p_nfct->bDnat) || (TRUE == p_nfct->bSnat))
    {
        /* NAT entry, should be cleared by admin manually, like C vendor.*/
        return FALSE;
    }

    /* source route match, inside or outside IP*/
    if (TRUE != nat_get_rtif_by_iproute(&p_nfct->orig.sip.u.prefix4, &p_rt_iif, &prefixlen_iif))
    {
        return FALSE;
    }

    /* dest route match, inside or outside IP, or IF IP*/
    if (TRUE != nat_get_rtif_by_iproute(&p_nfct->orig.dip.u.prefix4, &p_rt_oif, &prefixlen_oif))
    {
        return FALSE;
    }

    /* if ip protocol configured*/
    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
    {
        if (p_nfct->ip_proto != p_rule->key.protocol)
        {
            return FALSE;
        }
    }

    /*new static NAT rule, inside local IP must be configured*/
    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
    {
        /* case 1, upstream nfct, match local source IP*/
        if ((GLB_IF_NAT_INSIDE == p_rt_iif->nat_enable) && (GLB_IF_NAT_OUTSIDE == p_rt_oif->nat_enable))
        {
            if (p_nfct->orig.sip.u.prefix4.s_addr == p_rule->key.in_local_ip.u.prefix4.s_addr)
            {
                if (!(p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT))
                {
                    /* same local IP, no local port configured, nfct should be deleted and then created by flows according new NAT rule*/
                    return TRUE;
                }
                else if (p_nfct->orig.sport == p_rule->key.in_local_port)
                {
                    /* same local IP, same local port*/
                    return TRUE;
                }
            }
        }

        /* case 2, downstream nfct, exactly to the IP of mine interface, match outside interface*/
        if ((p_rt_iif->ifindex == p_rt_oif->ifindex) && (IPV4_MAX_PREFIXLEN == prefixlen_oif))
        {
            addr_ipv4_t prefix4;

            /* get configured global IP*/
            sal_memset(&prefix4, 0, sizeof(addr_ipv4_t));
            if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
            {
                sal_memcpy(&prefix4, &p_rule->key.in_glb_ip.u.prefix4, sizeof(addr_ipv4_t));
            }
            else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
            {
                ds_connected_t *p_primary_conn = nat_get_rtif_primary_connected_by_ifname(p_rule->key.oif);

                if (p_primary_conn)
                {
                    sal_memcpy(&prefix4, &p_primary_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
                }
            }

            /* check inside global IP and port*/
            if ((prefix4.s_addr != 0) && (p_nfct->orig.dip.u.prefix4.s_addr == prefix4.s_addr))
            {
                if (!(p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT))
                {
                    /* same glb IP, no glb port configured, nfct should be deleted and then created by flows according new NAT rule*/
                    return TRUE;
                }
                else if (p_nfct->orig.dport == p_rule->key.in_glb_port)
                {
                    /* same glb IP, same glb port*/
                    return TRUE;
                }
            }
        }
    }
    else/* dynamic rule*/
    {
        /* case 1, upstream nfct, match outside interface*/
        /* case 2, downstream nfct, exactly to the IP of mine interface, match outside interface*/
        if (((GLB_IF_NAT_INSIDE == p_rt_iif->nat_enable) && (GLB_IF_NAT_OUTSIDE == p_rt_oif->nat_enable))
         || ((p_rt_iif->ifindex == p_rt_oif->ifindex) && (IPV4_MAX_PREFIXLEN == prefixlen_oif)))
        {
            if ((p_rule->refer_oif[0] != 0) && p_rt_oif)
            {
                if (0 == sal_memcmp(p_rule->refer_oif, p_rt_oif->key.name, IFNAME_SIZE))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


/***************************************************************************************************
 * Name         : nat_rule_dynamic_del
 * Purpose      : delete one dynamic NAT rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_rule_proc_rtif_nat_disable(tbl_route_if_t   * p_rt_if)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_FAIL;
    }


    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        if ((0 != p_db_rule->refer_iif[0]) && (0 == sal_memcmp(p_db_rule->refer_iif, p_rt_if->key.name, IFNAME_SIZE)))
        {
            nat_rule_del(p_db_rule);
            continue;
        }

        if ((0 != p_db_rule->refer_oif[0]) && (0 == sal_memcmp(p_db_rule->refer_oif, p_rt_if->key.name, IFNAME_SIZE)))
        {
            nat_rule_del(p_db_rule);
            continue;
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_rule_proc_rtif_addr_add
 * Purpose      : 
 * Input        : tbl_route_if_t *p_rt_if, addr_ipv4_t *p_addr
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
int32
nat_rule_proc_rtif_addr_add(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP)
        {
            if (p_prefix->u.prefix4.s_addr == p_db_rule->key.in_local_ip.u.prefix4.s_addr)
            {
                /* conflict, delete the rule*/
                nat_rule_del(p_db_rule);
            }
        }

        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            if (p_prefix->u.prefix4.s_addr == p_db_rule->key.in_glb_ip.u.prefix4.s_addr)
            {
                /* conflict, delete the rule*/
                nat_rule_del(p_db_rule);
            }
        }
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_rule_proc_rtif_addr_del
 * Purpose      : 
 * Input        : tbl_route_if_t *p_rt_if, addr_ipv4_t *p_addr
 * Output       : N/A
 * Return       : TRUE, FALSE
 * Note         : N/A
***************************************************************************************************/
int32
nat_rule_proc_rtif_addr_del(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;
    tbl_route_if_t        *p_tmp_rt_if = NULL;
    prefix_t               prefix_for_cmp;
    uint8                  prefix_len = 0;
    int32                  ret = 0;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP)
        {
            /* prepare prefix_for_cmp*/
            prefix_for_cmp.family = AF_INET;
            prefix_for_cmp.prefixlen = p_prefix->prefixlen;
            sal_memcpy(&prefix_for_cmp.u.prefix4, &p_db_rule->key.in_local_ip.u.prefix4, sizeof(addr_ipv4_t));
            if (prefix_compare_net(p_prefix, &prefix_for_cmp))
            {
                ret = nat_get_rtif_by_ip(&p_db_rule->key.in_local_ip.u.prefix4, &p_prefix->u.prefix4, &p_tmp_rt_if, &prefix_len);
                if ((FALSE == ret) || (GLB_IF_NAT_INSIDE != p_tmp_rt_if->nat_enable))
                {
                    nat_rule_del(p_db_rule);
                }
            }
        }

        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            /* prepare prefix_for_cmp*/
            prefix_for_cmp.family = AF_INET;
            prefix_for_cmp.prefixlen = p_prefix->prefixlen;
            sal_memcpy(&prefix_for_cmp.u.prefix4, &p_db_rule->key.in_glb_ip.u.prefix4, sizeof(addr_ipv4_t));
            if (prefix_compare_net(p_prefix, &prefix_for_cmp))
            {
                ret = nat_get_rtif_by_ip(&p_db_rule->key.in_glb_ip.u.prefix4, &p_prefix->u.prefix4, &p_tmp_rt_if, &prefix_len);
                if ((FALSE == ret) || (GLB_IF_NAT_OUTSIDE != p_tmp_rt_if->nat_enable))
                {
                    nat_rule_del(p_db_rule);
                }
            }
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_rule_add
 * Purpose      : add rule
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_rule_add(tbl_nat_rule_t *p_rule)
{
    int32 ret = 0;
    tbl_nat_global_t *p_nat_global;

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;
    }

    p_rule->cfg_order = p_nat_global->rule_cfg_order;
    p_nat_global->rule_cfg_order++;

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
    {
        if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            nat_proxy_arp_add(p_rule->refer_oif, &p_rule->key.in_glb_ip.u.prefix4, &p_rule->key.in_glb_ip.u.prefix4);
        }
    
        ret = nat_nft_static_add(p_rule);
    }
    else
    {
        ret = nat_nft_dynamic_add(p_rule);
    }
    
    ret += tbl_nat_rule_add_nat_rule(p_rule);

    /* flush nfct related entries, make sure the new rule could make effect to existed flows immediately*/
    {
        nat_nfct_hook_s data;

        sal_memset(&data, 0, sizeof(nat_nfct_hook_s));
        data.argc = 1;
        data.argv[0] = p_rule;
        nat_nfct_scan(nat_nfct_hook_flush_by_new_rule, &data);
    }
    return ret;
}

/***************************************************************************************************
 * Name         : nat_rule_del
 * Purpose      : check and convert nat rule configuration
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_rule_del(tbl_nat_rule_t *p_db_rule)
{
    tbl_nat_global_t *p_nat_global;

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;
    }
    
    if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
    {
        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            nat_proxy_arp_delete(p_db_rule->refer_oif, &p_db_rule->key.in_glb_ip.u.prefix4, &p_db_rule->key.in_glb_ip.u.prefix4);
        }
        nat_nft_static_del(p_db_rule);
    }
    else
    {
        nat_nft_dynamic_del(p_db_rule);
    }
    
    return tbl_nat_rule_del_nat_rule(&p_db_rule->key);
}

#endif

#if _DESC("set if nat enable or disable")
int32
nat_if_enable_inside(tbl_route_if_t   *p_db_rt_if)
{
    tbl_nat_rtif_inside_master_t * p_rtif_inside_master = NULL;
    tbl_nat_rtif_inside_t rtif_insde;

    NAT_EVENT_DEBUG("Set interface %s NAT enable inside.", p_db_rt_if->key.name);
    p_rtif_inside_master = tbl_nat_rtif_inside_get_master();
    if (!p_rtif_inside_master)
    {
        return PM_E_FAIL;
    }

    if (p_rtif_inside_master->nat_rtif_inside_hash->count >= GLB_NAT_INSIDE_IF_NUM)
    {
        NAT_EVENT_DEBUG("Beyond inside interface limit %u.", GLB_NAT_INSIDE_IF_NUM);
        return PM_E_RESOURCE_FULL;
    }

    sal_memset(&rtif_insde, 0, sizeof(rtif_insde));
    sal_memcpy(rtif_insde.key.name, p_db_rt_if->key.name, IFNAME_SIZE);

    if (tbl_nat_rtif_inside_get_nat_rtif_inside(&rtif_insde.key))
    {
        NAT_EVENT_DEBUG("Already enable NAT inside.");
        return PM_E_NONE;
    }

    /* add to DB*/
    tbl_nat_rtif_inside_add_nat_rtif_inside(&rtif_insde);

    /* update nft rule*/
    nat_nft_update_rtif();
    return PM_E_NONE;
}
int32
nat_if_enable_outside(tbl_route_if_t *p_db_rt_if)
{
    tbl_nat_rtif_outside_master_t * p_rtif_outside_master = NULL;
    tbl_nat_rtif_outside_t rtif_outsde;

    NAT_EVENT_DEBUG("Set interface %s NAT enable outside.", p_db_rt_if->key.name);
    p_rtif_outside_master = tbl_nat_rtif_outside_get_master();
    if (!p_rtif_outside_master)
    {
        return PM_E_FAIL;
    }

    if (p_rtif_outside_master->nat_rtif_outside_hash->count >= GLB_NAT_OUTSIDE_IF_NUM)
    {
        NAT_EVENT_DEBUG("Beyond outside interface limit %u.", GLB_NAT_OUTSIDE_IF_NUM);
        return PM_E_RESOURCE_FULL;
    }

    sal_memset(&rtif_outsde, 0, sizeof(rtif_outsde));
    sal_memcpy(rtif_outsde.key.name, p_db_rt_if->key.name, IFNAME_SIZE);

    if (tbl_nat_rtif_outside_get_nat_rtif_outside(&rtif_outsde.key))
    {
        NAT_EVENT_DEBUG("Already enable NAT inside.");
        return PM_E_NONE;
    }

    /* add to DB*/
    tbl_nat_rtif_outside_add_nat_rtif_outside(&rtif_outsde);

    /* update nft rule*/
    nat_nft_update_rtif();
    return PM_E_NONE;
}

int32
nat_if_disable(tbl_route_if_t *p_db_rt_if, uint8 enable_old)
{
    tbl_nat_rtif_inside_key_t rtif_inside_key;
    tbl_nat_rtif_outside_key_t rtif_outside_key;

    NAT_EVENT_DEBUG("Set interface %s NAT disable. old:%u", p_db_rt_if->key.name, enable_old);

    nat_rule_proc_rtif_nat_disable(p_db_rt_if);

    switch (enable_old)
    {
        case GLB_IF_NAT_INSIDE:
        {
            sal_memset(&rtif_inside_key, 0, sizeof(rtif_inside_key));
            sal_memcpy(rtif_inside_key.name, p_db_rt_if->key.name, IFNAME_SIZE);
            nat_session_clear_by_iif(p_db_rt_if);

            tbl_nat_rtif_inside_del_nat_rtif_inside(&rtif_inside_key);
            break;
        }
        case GLB_IF_NAT_OUTSIDE:
        {
            sal_memset(&rtif_outside_key, 0, sizeof(rtif_outside_key));
            sal_memcpy(rtif_outside_key.name, p_db_rt_if->key.name, IFNAME_SIZE);
            nat_session_clear_by_oif(p_db_rt_if);
            nat_pool_proc_rtif_nat_disable(p_db_rt_if);

            tbl_nat_rtif_outside_del_nat_rtif_outside(&rtif_outside_key);
            break;
        }
        case GLB_IF_NAT_DISABLE:
        default:
            break;
    }

    nat_nft_update_rtif();
    return PM_E_NONE;
}
int32
nat_if_set(tbl_route_if_t *p_db_rt_if, uint8 enable_old, uint8 enable_new)
{
    NAT_EVENT_DEBUG("Set interface %s NAT configuration. old:%u, new:%u", p_db_rt_if->key.name, enable_old, enable_new);

    if (GLB_IF_NAT_DISABLE != enable_old)
    {
        nat_if_disable(p_db_rt_if, enable_old);
    }

    switch (enable_new)
    {
        case GLB_IF_NAT_INSIDE:
        {
            return nat_if_enable_inside(p_db_rt_if);
        }
        case GLB_IF_NAT_OUTSIDE:
        {
            return nat_if_enable_outside(p_db_rt_if);
        }
        case GLB_IF_NAT_DISABLE:
        default:
            break;
    }
    return PM_E_NONE;
}

#endif




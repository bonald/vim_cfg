#include "proto.h"
#include "glb_acl_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_acl_config_define.h" 
#include "gen/tbl_acl_config.h" 
#include "gen/tbl_ace_config_define.h" 
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "gen/tbl_acl_entry_define.h" 
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_entry_action_define.h" 
#include "gen/tbl_acl_entry_action.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_class_map_action_config_define.h" 
#include "gen/tbl_class_map_action_config.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_fea_ptp_global_define.h"
#include "gen/tbl_fea_ptp_global.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "switch.h"
#include "if_agg.h"
#include "acl.h"
#include "acl_policy.h"
#include "acl_policy_res_check.h"
#include "gen/tbl_acl_l4_port_range_define.h"
#include "gen/tbl_acl_l4_port_range.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_acl_udf_group_define.h"
#include "gen/tbl_acl_udf_group.h"
#include "lib_acl.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#ifdef TAPPRODUCT
#include "tap.h"
#endif

ctclib_opf_t* acl_worm_filter_seq_opf = NULL;

extern int32
acl_policy_operation_acl_group_del(char* acl_name);

extern int32
tap_group_ingress_flow_operation(uint32 operation_type, tbl_ace_config_t *pst_ace_config, tbl_ace_config_t *pst_inner_ace_config, uint32 is_decap);

extern int32
tap_group_ingress_flow_del_acl_flow(char* acl_name);

extern int32 
swth_sys_check_telnet_ssh_ace(tbl_ace_config_t *p_acl_config);

extern int32
swth_sys_set_telnet_ssh_acl(tbl_acl_config_t *p_acl_config,
                            uint32 service_type, uint32 is_add);

extern int32
ipt_del_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type);

extern int32
ipt_add_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type);


extern void
sal_masklen2ip6 (int32 masklen, struct sal_in6_addr *netmask);

uint32
acl_stm_is_ipv6_profile()
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

int32
acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    uint32 *num_ingress = pargs->argv[0];
    uint32 *num_egress = pargs->argv[1];
    uint32 *num_global_ingress = pargs->argv[2];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            *num_ingress += 2;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    *num_ingress += 2;
                    *num_ingress += 1;
                }
                else
                {
                    *num_ingress += 1;
                }
            }
            else
            {
                *num_ingress += 1;
            }
        }
        else
        {
            *num_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            *num_egress += 2;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    *num_egress += 2;
                    *num_egress += 1;
                }
                else
                {
                    *num_egress += 1;
                }
            }
            else
            {
                *num_egress += 1;
            }
        }
        else
        {
            *num_egress += 1;
        }
    }
    if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_global_ingress += 2;
        }
        else
        {
            *num_global_ingress += 1;
        }
    }
    return PM_E_NONE;
}

int32
acl_entry_get_count_1(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    acl_entry_count_t *entry_count = pargs->argv[0];
    
    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            entry_count->cur_ingress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {   
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    entry_count->cur_ingress6 += 1;
                    entry_count->cur_ingress += 1;
                }
                else
                {
                    entry_count->cur_ingress += 1;
                }
            }
            else
            {
                entry_count->cur_ingress += 1;
            }
        }
        else
        {
            entry_count->cur_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            entry_count->cur_egress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {   
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    entry_count->cur_egress6 += 1;
                    entry_count->cur_egress += 1;
                }
                else
                {
                    entry_count->cur_egress += 1;
                }
            }
            else
            {
                entry_count->cur_egress += 1;
            }
        }
        else
        {
            entry_count->cur_egress += 1;
        }     
    }

	if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_worm_filter6 += 1;
        }
        else
        {
            entry_count->cur_worm_filter += 1;
        }
    }

    return PM_E_NONE;
}



int32
acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_worm_filter)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = cnt_ingress;
    args.argv[1] = cnt_egress;
    args.argv[2] = cnt_worm_filter;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)acl_entry_get_count, &args);
    return PM_E_NONE;
}

int32
acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = entry_count;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)acl_entry_get_count_1, &args);
    return PM_E_NONE;
}

int32
acl_ace_get_l4_dest_port(uint32 l4_dst_port)
{
    tbl_ace_config_master_t *p_master_ace = tbl_ace_config_get_master();
    tbl_ace_config_t *pst_db_ace_config = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_ace->ace_config_list, pst_db_ace_config, listnode)
    {
    	if (GLB_FLAG_ISSET(pst_db_ace_config->key_flags, GLB_IP_ACE_VXLAN_VNI) &&
    	    GLB_FLAG_ISSET(pst_db_ace_config->key_flags, GLB_IP_ACE_DSTPORT))
    	{
    		if (pst_db_ace_config->l4_dst_port != l4_dst_port)
    		{
    			return pst_db_ace_config->l4_dst_port;
    		}
    	}
    }
    return -1;
}

int32
_acl_show_ace_config(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    char *acl_name = pargs->argv[1];
    bool *is_by_policy = pargs->argv[2];
#ifdef TAPPRODUCT    
    tbl_acl_config_t *p_acl_config = pargs->argv[3];
#endif
    char buf1[256];
    char buf2[256];
    char buf3[256];
    uint8 flag = 0;

    if(*is_by_policy)
    {
         sal_fprintf (cfg_fp, "  ");
    }
    if (0 != sal_strcmp(p_ace_config->key.acl_name, acl_name))
    {
        return PM_E_NONE;
    }

    sal_memset(buf1, 0, sizeof(buf1));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(buf3, 0, sizeof(buf3));

#ifdef TAPPRODUCT
        if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == p_acl_config->type_identifying)
        {
            sal_fprintf (cfg_fp, " sequence-num %d match", p_ace_config->key.seq_no);
        }else{
            sal_fprintf (cfg_fp, " sequence-num %d %s", p_ace_config->key.seq_no,
                (p_ace_config->deny ? "deny" : "permit"));
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IP_PROTOCOL))
        {
            if (0xff == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " any");
            }
            else if (GLB_ACL_L4_PROTOCOL_TCP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " tcp");
            }
            else if (GLB_ACL_L4_PROTOCOL_UDP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " udp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask
                && !p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask
                && p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_IGMP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask
                && !p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " igmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_GRE == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                if (GLB_ACL_GRE_NVGRE == p_ace_config->l4_gre_type)
                {
                    sal_fprintf (cfg_fp, " nvgre");
                }else{
                    sal_fprintf (cfg_fp, " gre");
                }            
            }
            else if (GLB_ACL_L4_PROTOCOL_IPIP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " ipip");
            }
            else
            {
                sal_fprintf (cfg_fp, " %d", p_ace_config->ip_protocol);
            }
        }
        else
        {
            sal_fprintf(cfg_fp, " any");
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_ICMP_TYPE))
        {
            sal_fprintf (cfg_fp, " icmp-type %d", p_ace_config->icmp_type);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_ICMP_CODE))
        {
            sal_fprintf (cfg_fp, " icmp-code %d", p_ace_config->icmp_code);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_GRE_KEY))
        {
            if (GLB_ACL_GRE_NVGRE == p_ace_config->l4_gre_type)
            {
                if ((0xffffffff << GLB_ACL_GRE_NVGRE_KEY_OFFSET) == p_ace_config->l4_gre_key_mask)
                {
                    sal_fprintf (cfg_fp, " nvgre-vsid any");
                }
                else
                {
                    sal_fprintf (cfg_fp, " nvgre-vsid %u 0x%x", 
#ifdef DUET2                        
                        p_ace_config->l4_gre_key, p_ace_config->l4_gre_key_mask);
#else
                        p_ace_config->l4_gre_key >> GLB_ACL_GRE_NVGRE_KEY_OFFSET, 
                        p_ace_config->l4_gre_key_mask >> GLB_ACL_GRE_NVGRE_KEY_OFFSET);
#endif
                }
            }else{
                if (0xffffffff == p_ace_config->l4_gre_key_mask)
                {
                    sal_fprintf (cfg_fp, " gre-key any");
                }
                else
                {
                    sal_fprintf (cfg_fp, " gre-key %u 0x%x", 
                        p_ace_config->l4_gre_key, 
                        p_ace_config->l4_gre_key_mask);
                }
            }  
        }

		if (GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_ERSPAN_ID))
		{
		   	if (0x3FF == p_ace_config->erspan_id_mask)
		    {
		        sal_fprintf (cfg_fp, " erspan any");
		    }
			else
			{
			    sal_fprintf (cfg_fp, " erspan %u 0x%x", 
			        p_ace_config->erspan_id, p_ace_config->erspan_id_mask);
			}
		}

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_SRCPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port eq %d", p_ace_config->l4_src_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port lt %d", p_ace_config->l4_src_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port gt %d", p_ace_config->l4_src_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port range %d %d", p_ace_config->l4_src_port, p_ace_config->l4_src_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_DSTPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port eq %d", p_ace_config->l4_dst_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port lt %d", p_ace_config->l4_dst_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port gt %d", p_ace_config->l4_dst_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port range %d %d", p_ace_config->l4_dst_port, p_ace_config->l4_dst_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_VXLAN_VNI))
        {
            if (GLB_ACL_VXLAN_VNI_MASK_VALID == p_ace_config->l4_vxlan_vni_mask)
            {
                sal_fprintf (cfg_fp, " vxlan-vni any");
            }
            else
            {
                sal_fprintf (cfg_fp, " vxlan-vni %u 0x%x", p_ace_config->l4_vxlan_vni, p_ace_config->l4_vxlan_vni_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TCP_FLAG))
        {
            if (GLB_ACL_TCP_FLAG_OP_ALL == p_ace_config->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-code match-all");
            }
            else if (GLB_ACL_TCP_FLAG_OP_ANY == p_ace_config->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-code match-any");
            }

            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
            {
                sal_fprintf (cfg_fp, " ack");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
            {
                sal_fprintf (cfg_fp, " fin");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
            {
                sal_fprintf (cfg_fp, " psh");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
            {
                sal_fprintf (cfg_fp, " rst");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
            {
                sal_fprintf (cfg_fp, " syn");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
            {
                sal_fprintf (cfg_fp, " urg");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPSA))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip any");
            }
            else if (0 == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
            }
        }
        else if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->src_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " src-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_ace_config->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
            }
        }
        else if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            if (p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " src-ipv6 any");
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ip any");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPDA))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dst-ip any");
            }
            else if (0 == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dst-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ip %s %s", buf1, buf2);
            }
        }
        else if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " dst-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_ace_config->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
            }
        }
        else if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
        { 
            if (p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " dst-ipv6 any");
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ip any");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
        {
            if (0xFFFFF == p_ace_config->ipv6_flow_label_mask && 0 == p_ace_config->ipv6_flow_label)
            {
                sal_fprintf (cfg_fp, " flow-label any");
            }
            else
            {
                sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_ace_config->ipv6_flow_label, p_ace_config->ipv6_flow_label_mask);
            }
        }
        
        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_DSCP))
        {
            sal_fprintf (cfg_fp, " dscp %d", p_ace_config->dscp);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_PRECEDENCE))
        {
            sal_fprintf (cfg_fp, " ip-precedence %d", p_ace_config->ip_precedence);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IP_FRAG))
        {
            if (GLB_ACL_IP_FRAG_FIRST_FRAG == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_FIRST == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NONE == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-or-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_TINY == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " small-fragment");
            }
            else if (GLB_ACL_IP_FRAG_YES == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " any-fragment");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_OPTIONS))
        {
            sal_fprintf (cfg_fp, " options");
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_UDF) &&
	     !GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_ERSPAN_ID))
        {
            if (GLB_ACL_UDF_GROUP_L3_HEAD == p_ace_config->udf_type)
            {
                sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
                p_ace_config->udf_value[0], p_ace_config->udf_mask[0], p_ace_config->udf_offset);
            }
            else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_ace_config->udf_type)
            {
                sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
                p_ace_config->udf_value[0], p_ace_config->udf_mask[0], p_ace_config->udf_offset);
            }
            else
            {
                sal_fprintf (cfg_fp, " udf udf-id %d", p_ace_config->udf_id);
                uint32 iter = 0;
                for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
                {
                    if (p_ace_config->udf_value_valid[iter])
                    {
                        if (0xFFFFFFFF == p_ace_config->udf_mask[iter])
                        {
                            sal_fprintf(cfg_fp, " udf%d any", iter);
                        }
                        else
                        {
                            sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                    p_ace_config->udf_value[iter], p_ace_config->udf_mask[iter]);
                        }
                    }
                }
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_COS))
        {
            sal_fprintf (cfg_fp, " cos %d", p_ace_config->svlan_cos);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_INNER_COS))
        {
            sal_fprintf (cfg_fp, " inner-cos %d", p_ace_config->cvlan_cos);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_VLAN))
        {
            if (0xfff == p_ace_config->svlan_mask && 0 == p_ace_config->svlan)
            {
                sal_fprintf (cfg_fp, " vlan any");
            }
            else
            {
                sal_fprintf (cfg_fp, " vlan %d 0x%x", p_ace_config->svlan, p_ace_config->svlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_INNNER_VLAN))
        {
            if (0xfff == p_ace_config->cvlan_mask)
            {
                sal_fprintf (cfg_fp, " inner-vlan any");
            }
            else
            {
                sal_fprintf (cfg_fp, " inner-vlan %d 0x%x", p_ace_config->cvlan, p_ace_config->cvlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_MACSA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_ace_config->src_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_ace_config->src_mac_mask);

            if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
            {
                sal_fprintf (cfg_fp, " src-mac any");
            }
            else if (0 == sal_strcmp("0000.0000.0000", buf2))
            {
                sal_fprintf (cfg_fp, " src-mac host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-mac %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_MACDA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_ace_config->dst_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_ace_config->dst_mac_mask);

            if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
            {
                sal_fprintf (cfg_fp, " dest-mac any");
            }
            else if (0 == sal_strcmp("0000.0000.0000", buf2))
            {
                sal_fprintf (cfg_fp, " dest-mac host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dest-mac %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            if (0xffff == p_ace_config->ether_type_mask)
            {
                sal_fprintf (cfg_fp, " ether-type any");
            }
            else
            {
                sal_fprintf (cfg_fp, " ether-type 0x%x 0x%x", p_ace_config->ether_type, p_ace_config->ether_type_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
        {
            sal_fprintf (cfg_fp, " inner-match %s", acl_get_acl_config_key_name(
                p_ace_config->flow_match_name,GLB_ACL_CONFIG_TYPE_FLOW_MATCH));
        }

        /*added by yejl for bug 53475, 2019-09-10*/
        if (p_ace_config->l2gre_add_header)
        {
            sal_fprintf (cfg_fp, " add-l2gre l2gre-sip %s l2gre-dip %s l2gre-dmac %s l2gre-key %u l2gre-key-length %u", 
                cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->l2gre_src_ip.u.prefix4),
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->l2gre_dst_ip.u.prefix4),
                cdb_mac_addr_val2str(buf3, CMD_BUF_256, p_ace_config->l2gre_dst_mac),
                p_ace_config->l2gre_key_num, p_ace_config->l2gre_key_len);
        }
        if (p_ace_config->l3gre_add_header)
        {
            sal_fprintf (cfg_fp, " add-l3gre l3gre-sip %s l3gre-dip %s l3gre-dmac %s", 
                cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->l3gre_src_ip.u.prefix4),
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->l3gre_dst_ip.u.prefix4),
                cdb_mac_addr_val2str(buf3, CMD_BUF_256, p_ace_config->l3gre_dst_mac));
        }
        /*ended by yejl*/

        /*added by guoxd for vxlan*/
         if (p_ace_config->add_vxlan_header)
        {
            sal_fprintf (cfg_fp, " add-vxlan vxlan-dmac %s vxlan-sip %s vxlan-dip %s vxlan-sport %d vxlan-dport %d vxlan-set-vni %d", 
                cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_ace_config->vxlan_dst_mac),
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->vxlan_src_ip.u.prefix4),
                cdb_addr_ipv4_val2str(buf3, CMD_BUF_256, &p_ace_config->vxlan_dst_ip.u.prefix4),
                p_ace_config->vxlan_srcport,
                p_ace_config->vxlan_destport,
                p_ace_config->vxlan_vni);
        }
        /*ended by guoxd*/
        
        if (p_ace_config->action_strip_header)
        {
            sal_fprintf (cfg_fp, " strip-header");

            if (GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_STRIP_HEADER_POS))
            {
                sal_fprintf (cfg_fp, " strip-position %s", 
                    p_ace_config->strip_header_pos == GLB_ACL_STRIP_HEADER_POS_L2 ? "l2" :
                    p_ace_config->strip_header_pos == GLB_ACL_STRIP_HEADER_POS_L3 ? "l3": "l4" );
            }

            if (GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_STRIP_HEADER_OFFSET))
            {
                sal_fprintf (cfg_fp, " strip-offset %d", 
                    p_ace_config->strip_header_offset);
            }

	        if (GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER))
            {
                sal_fprintf (cfg_fp, " strip-inner-vxlan-header");
            }
        }
        
        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TAP_TRUNCATION))
        {
            sal_fprintf (cfg_fp, " truncation");
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TAP_UNTAG))
        {
            switch(p_ace_config->tap_action_untag)
            {
            case UNTAG_MODE_AUTO:
                sal_fprintf (cfg_fp, " un-tag");
                break;
            case UNTAG_MODE_SVLAN:
                sal_fprintf (cfg_fp, " un-tag-outer-vlan");
                break;
            case UNTAG_MODE_CVLAN:
                sal_fprintf (cfg_fp, " un-tag-inner-vlan");
                break;
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TAP_MARK_SOURCE))
        {
            sal_fprintf (cfg_fp, " mark-source %d", p_ace_config->tap_action_mark_vlan);
        }

        if (p_ace_config->tap_action_edit_dest_mac_en)
        {
            sal_fprintf (cfg_fp, " edit-macda %.04hX.%.04hX.%.04hX",
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_dest_mac)[0]),
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_dest_mac)[1]), 
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_dest_mac)[2]));
        }

        if (p_ace_config->tap_action_edit_src_mac_en)
        {
            sal_fprintf (cfg_fp, " edit-macsa %.04hX.%.04hX.%.04hX",
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_src_mac)[0]),
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_src_mac)[1]), 
                sal_ntoh16(((uint16*)p_ace_config->tap_action_edit_src_mac)[2]));
        }

        if(p_ace_config->tap_action_edit_vlan_en){
			if(p_ace_config->tap_action_edit_vlan_action)
			{
            	sal_fprintf (cfg_fp, " add-vlan %d", p_ace_config->tap_action_edit_vlan);
			}else{
				sal_fprintf (cfg_fp, " edit-vlan %d", p_ace_config->tap_action_edit_vlan);
			}
        }

        if(p_ace_config->tap_action_edit_ipda_en){
            if (AF_INET == p_ace_config->tap_action_edit_ipda.family)
            {
                sal_fprintf (cfg_fp, " edit-ipda %s",
                    cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->tap_action_edit_ipda.u.prefix4));
            }
            else if (AF_INET6 == p_ace_config->tap_action_edit_ipda.family)
            {
                sal_fprintf (cfg_fp, " edit-ipv6da %s",
                    cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->tap_action_edit_ipda.u.prefix6));
            }
        }

        if(p_ace_config->tap_action_edit_ipsa_en){
            if (AF_INET == p_ace_config->tap_action_edit_ipsa.family)
            {
                sal_fprintf (cfg_fp, " edit-ipsa %s",
                    cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->tap_action_edit_ipsa.u.prefix4));
            }
            else if (AF_INET6 == p_ace_config->tap_action_edit_ipsa.family)
            {
                sal_fprintf (cfg_fp, " edit-ipv6sa %s",
                    cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->tap_action_edit_ipsa.u.prefix6));
            }
        }

#else
        sal_fprintf (cfg_fp, " %d %s", p_ace_config->key.seq_no,
            (p_ace_config->deny ? "deny" : "permit"));

        if (0 == p_ace_config->key_flags && 0 == p_ace_config->key_ipv6_flags && 0 == p_ace_config->key_arp_flags)
        {
            if(*is_by_policy)
            {
                 sal_fprintf (cfg_fp, " any");
            }
            else
            {
                sal_fprintf (cfg_fp, " any\n");
            }
            return PM_E_NONE;
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_MACSA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_ace_config->src_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_ace_config->src_mac_mask);

            if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
            {
                sal_fprintf (cfg_fp, " src-mac any");
            }
            else if (0 == sal_strcmp("0000.0000.0000", buf2))
            {
                sal_fprintf (cfg_fp, " src-mac host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-mac %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_MACDA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_ace_config->dst_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_ace_config->dst_mac_mask);

            if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
            {
                sal_fprintf (cfg_fp, " dest-mac any");
            }
            else if (0 == sal_strcmp("0000.0000.0000", buf2))
            {
                sal_fprintf (cfg_fp, " dest-mac host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dest-mac %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_VLAN))
        {
            if (0xfff == p_ace_config->svlan_mask)
            {
                sal_fprintf (cfg_fp, " vlan any");
            }
            else
            {
                /*modified by yejl for bug 49788, 2018-11-13*/
                sal_fprintf (cfg_fp, " vlan %d 0x%x", p_ace_config->svlan, p_ace_config->svlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_INNNER_VLAN))
        {
            if (0xfff == p_ace_config->cvlan_mask)
            {
                sal_fprintf (cfg_fp, " inner-vlan any");
            }
            else
            {
                sal_fprintf (cfg_fp, " inner-vlan %d 0x%x", p_ace_config->cvlan, p_ace_config->cvlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_COS))
        {
            if (0x7 == p_ace_config->svlan_cos_mask)
            {
                sal_fprintf (cfg_fp, " cos any");
            }
            else
            {
                sal_fprintf (cfg_fp, " cos %d 0x%x", p_ace_config->svlan_cos, p_ace_config->svlan_cos_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_INNER_COS))
        {
            if (0x7 == p_ace_config->cvlan_cos_mask)
            {
                sal_fprintf (cfg_fp, " inner-cos any");
            }
            else
            {
                sal_fprintf (cfg_fp, " inner-cos %d 0x%x", p_ace_config->cvlan_cos, p_ace_config->cvlan_cos_mask);
            }
        }

        /*added by yejl for bug 49791, 2018-11-14*/
        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_UNTAG))
        {
            sal_fprintf (cfg_fp, " untag-vlan");
        }
        
        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            sal_fprintf (cfg_fp, " ether-type 0x%x", p_ace_config->ether_type);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_arp_flags, GLB_ARP_ACE_OP_CODE))
        {
            if (0xffff == p_ace_config->arp_op_code_mask)
            {
                sal_fprintf (cfg_fp, " arp any");
            }
            else if (0 == p_ace_config->arp_op_code_mask && ARP_TYPE_REQUEST == p_ace_config->arp_op_code)
            {
                sal_fprintf (cfg_fp, " arp request");
            }
            else if (0 == p_ace_config->arp_op_code_mask && ARP_TYPE_RESPONSE == p_ace_config->arp_op_code)
            {
                sal_fprintf (cfg_fp, " arp response");
            }
            else if (0 == p_ace_config->arp_op_code_mask && ARP_TYPE_RARP_REQUEST == p_ace_config->arp_op_code)
            {
                sal_fprintf (cfg_fp, " arp rarp-request");
            }
            else if (0 == p_ace_config->arp_op_code_mask && ARP_TYPE_RARP_RESPONSE == p_ace_config->arp_op_code)
            {
                sal_fprintf (cfg_fp, " arp rarp-response");
            }
        }
        
        if (GLB_FLAG_ISSET(p_ace_config->key_arp_flags, GLB_ARP_ACE_SENDER_IP))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " sender-ip any");
            }
            else if (0 == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " sender-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " sender-ip %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_arp_flags, GLB_ARP_ACE_TARGET_IP))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " target-ip any");
            }
            else if (0 == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " target-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " target-ip %s %s", buf1, buf2);
            }
        }
        
        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPSA))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip any");
            }
            else if (0 == p_ace_config->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->src_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " src-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_ace_config->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPDA))
        {
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix4);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix4);

            if (0xffffffff == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dest-ip any");
            }
            else if (0 == p_ace_config->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dest-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dest-ip %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " dst-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_ace_config->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
        {
            if (0xFFFFF == p_ace_config->ipv6_flow_label_mask && 0 == p_ace_config->ipv6_flow_label)
            {
                sal_fprintf (cfg_fp, " flow-label any");
            }
            else
            {
                sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_ace_config->ipv6_flow_label, p_ace_config->ipv6_flow_label_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_DSCP))
        {
            if (0x3f == p_ace_config->dscp_mask)
            {
                sal_fprintf (cfg_fp, " dscp any");
            }
            else
            {
                sal_fprintf (cfg_fp, " dscp %d 0x%x", p_ace_config->dscp, p_ace_config->dscp_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_PRECEDENCE))
        {
            if (0x7 == p_ace_config->ip_precedence_mask)
            {
                sal_fprintf (cfg_fp, " ip-precedence any");
            }
            else
            {
                sal_fprintf (cfg_fp, " ip-precedence %d 0x%x", p_ace_config->ip_precedence, p_ace_config->ip_precedence_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IP_FRAG))
        {
            /*modified by yejl for bug 49815, 2018-11-08*/
            sal_fprintf (cfg_fp, " fragment");
            if (GLB_ACL_IP_FRAG_FIRST_FRAG == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_FIRST == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NONE == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-or-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_TINY == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " small-fragment");
            }
            else if (GLB_ACL_IP_FRAG_YES == p_ace_config->ip_frag)
            {
                sal_fprintf (cfg_fp, " any-fragment");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IP_PROTOCOL))
        {
            if (GLB_ACL_L4_PROTOCOL_TCP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " l3-protocol tcp");
            }
            else if (GLB_ACL_L4_PROTOCOL_UDP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " l3-protocol udp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask
                && !p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " l3-protocol icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask 
                     && p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " l3-protocol icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_IGMP == p_ace_config->ip_protocol && 0 == p_ace_config->ip_protocol_mask
                && !p_ace_config->is_ipv6)
            {
                sal_fprintf (cfg_fp, " l3-protocol igmp");
            }
            else
            {
                sal_fprintf (cfg_fp, " l3-protocol %d", p_ace_config->ip_protocol);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_SRCPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-l4-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-l4-port eq %d", p_ace_config->l4_src_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-l4-port lt %d", p_ace_config->l4_src_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-l4-port gt %d", p_ace_config->l4_src_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_ace_config->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-l4-port range %d %d", p_ace_config->l4_src_port, p_ace_config->l4_src_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_DSTPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dest-l4-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dest-l4-port eq %d", p_ace_config->l4_dst_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dest-l4-port lt %d", p_ace_config->l4_dst_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dest-l4-port gt %d", p_ace_config->l4_dst_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_ace_config->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dest-l4-port range %d %d", p_ace_config->l4_dst_port, p_ace_config->l4_dst_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IGMP_TYPE))
        {
            /*modified by yejl for bug 49821, 2018-11-01*/
            if (ACL_IGMP_TYPE_HOST_DVMRP == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " dvmrp");
            }
            else if (ACL_IGMP_TYPE_HOST_QUERY == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " host-query");
            }
            else if (ACL_IGMP_TYPE_HOST_REPORT == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " v1host-report");
            }
            else if (ACL_IGMP_TYPE_MTRACT == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " mtrace-route");
            }
            else if (ACL_IGMP_TYPE_MTRACT_RESPONSE == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " mtrace-response");
            }
            else if (ACL_IGMP_TYPE_V2_LEAVE == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " v2leave-group");
            }
            else if (ACL_IGMP_TYPE_V2_REPORT == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " v2host-report");
            }
            else if (ACL_IGMP_TYPE_V3_REPORT == p_ace_config->igmp_type)
            {
                sal_fprintf (cfg_fp, " v3host-report");
            } 
            else
            {
                sal_fprintf (cfg_fp, " %d", p_ace_config->igmp_type);
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_ICMP_TYPE))
        {
            sal_fprintf (cfg_fp, " icmp-type %d", p_ace_config->icmp_type);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_ICMP_CODE))
        {
            sal_fprintf (cfg_fp, " icmp-code %d", p_ace_config->icmp_code);
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TCP_FLAG))
        {
            if (GLB_ACL_TCP_FLAG_OP_ALL == p_ace_config->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-flag match-all");
            }
            else if (GLB_ACL_TCP_FLAG_OP_ANY == p_ace_config->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-flag match-any");
            }

            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
            {
                sal_fprintf (cfg_fp, " ack");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
            {
                sal_fprintf (cfg_fp, " fin");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
            {
                sal_fprintf (cfg_fp, " psh");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
            {
                sal_fprintf (cfg_fp, " rst");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
            {
                sal_fprintf (cfg_fp, " syn");
            }
            if GLB_FLAG_ISSET(p_ace_config->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
            {
                sal_fprintf (cfg_fp, " urg");
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_OPTIONS))
        {
            sal_fprintf (cfg_fp, " options");
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_UDF))
        {
            if (GLB_ACL_UDF_GROUP_L3_HEAD == p_ace_config->udf_type)
            {
                sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
                p_ace_config->udf_value[0], p_ace_config->udf_mask[0], p_ace_config->udf_offset);
            }
            else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_ace_config->udf_type)
            {
                sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
                p_ace_config->udf_value[0], p_ace_config->udf_mask[0], p_ace_config->udf_offset);
            }
            else
            {
                sal_fprintf (cfg_fp, " udf udf-id %d", p_ace_config->udf_id);
                uint32 iter = 0;
                for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
                {
                    if (p_ace_config->udf_value_valid[iter])
                    {
                        if (0xFFFFFFFF == p_ace_config->udf_mask[iter])
                        {
                            sal_fprintf(cfg_fp, " udf%d any", iter);
                        }
                        else
                        {
                            sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                    p_ace_config->udf_value[iter], p_ace_config->udf_mask[iter]);
                        }
                    }
                }
            }
        }

        if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TIME_RANGE))
        {
            sal_fprintf (cfg_fp, " time-range %s", p_ace_config->time_range);
            if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_INVALID))
            {
                sal_fprintf (cfg_fp, " (invalid)");
            }
            else
            {
                if (0 != p_ace_config->apply_cnt)
                {
                    sal_fprintf (cfg_fp, " (valid)");
                }
                else
                {
                    sal_fprintf (cfg_fp, " (invalid)");
                }
            }
        }
        
        if(*is_by_policy)
        {
            return PM_E_NONE;
        }
#endif
    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}

int32
_acl_show_acl_config(FILE* fp, tbl_acl_config_t *p_acl_config)
{
    FILE *cfg_fp = fp;
    tbl_iter_args_t args;
    bool is_by_policy = FALSE;

    sal_fprintf (cfg_fp, "%s %s %s\n", 
                    glb_acl_show_running_str[p_acl_config->type_identifying],
                    acl_get_acl_config_key_name(p_acl_config->key.name, p_acl_config->type_identifying),
                    p_acl_config->decap_en ? "type decap":" ");

    if (0 != sal_strlen(p_acl_config->remark))
    {
        sal_fprintf (cfg_fp, " remark %s\n", p_acl_config->remark);
    }

    if (0 != p_acl_config->ace_ref || 0 != p_acl_config->acev6_ref || 0 != p_acl_config->acemac_ref)
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_acl_config->key.name);
        args.argv[2] = &is_by_policy;
        args.argv[3] = (void *)(p_acl_config);
        tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_show_ace_config, &args);
    }

    return PM_E_NONE;
}

int32
acl_show_acl(tbl_acl_config_t *p_db_acl_config, uint32 type, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_acl_config_master_t *p_master = tbl_acl_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    
    if (p_db_acl_config)
    {
        _acl_show_acl_config(fp, p_db_acl_config);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->acl_config_list, p_db_acl_config, listnode)
        {
            if (p_db_acl_config->type_identifying != type)
            {
                continue;
            }
            _acl_show_acl_config(fp, p_db_acl_config);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
_time_range_show_info(FILE *fp, tbl_time_range_t *p_db_time_range)
{
    tbl_ace_config_master_t *p_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;
    
    sal_fprintf (fp, "time-range %s\n", p_db_time_range->key.name);

    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace_config, listnode, next)
    {
        if (0 == sal_strcmp(p_db_ace_config->time_range, p_db_time_range->key.name))
        {
#ifdef TAPPRODUCT
            sal_fprintf (fp, " flow %s sequence-num %d\n",
            p_db_ace_config->key.acl_name, p_db_ace_config->key.seq_no);
#else        
            if (GLB_ACL_CONFIG_TYPE_NORMAL == p_db_ace_config->acl_type)
            {
                sal_fprintf (fp, " ip access-list %s sequence-num %d\n",
                    p_db_ace_config->key.acl_name, p_db_ace_config->key.seq_no);
            }
            else if (GLB_ACL_CONFIG_TYPE_IPV6 == p_db_ace_config->acl_type)
            {
                sal_fprintf (fp, " ipv6 access-list %s sequence-num %d\n",
                    p_db_ace_config->key.acl_name, p_db_ace_config->key.seq_no);
            }
            else if (GLB_ACL_CONFIG_TYPE_MAC == p_db_ace_config->acl_type)
            {
                sal_fprintf (fp, " mac access-list %s sequence-num %d\n",
                    p_db_ace_config->key.acl_name, p_db_ace_config->key.seq_no);
            }
#endif            
        }
    }

    return PM_E_NONE;
}

int32
time_range_show_info(tbl_time_range_t *p_db_time_range, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_time_range_master_t* p_master = tbl_time_range_get_master();
    ctclib_slistnode_t *listnode = NULL;
    
    if (p_db_time_range)
    {
        _time_range_show_info(fp, p_db_time_range);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->time_range_list, p_db_time_range, listnode)
        {
            _time_range_show_info(fp, p_db_time_range);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

#ifdef TAPPRODUCT
static int32
__tap_acl_config_res_check(int32 check_type, 
                           tbl_ace_config_t *ace_config, 
                           tbl_acl_config_t *pst_acl_config,
                           tbl_acl_config_t *pst_flow_match_config)
{
	uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;

	uint32 current_acl_entry_count_ingress 		= 0;
    uint32 current_acl_entry_count_egress 		= 0;
	uint32 current_acl_entry_count_wromfilter 	= 0;

	acl_entry_count_t entry_count = {0};

	int32  share_resource = TRUE;

	share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);

	if(share_resource)
	{
		tbl_acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}else{
		tbl_acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}
	
	/* tap product, flow only use ingress,
       ip access-list only use egress
     */
    switch(check_type)
    {
    	case GLB_ACL_CONFIG_TYPE_FLOW:
	    {
	    	if(share_resource)
	    	{
	    		/* E580 acl tcam resource share check */
		        if(GLB_FLAG_ISSET(ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
		        {
		        	/* 350 not support match, do not need check resource */
		            /*
		             * if flow match 
		             */
		            if (ace_config->is_ipv6)
		            {
		                if((pst_acl_config->intf_ref_in*2*(pst_flow_match_config->ace_ref + pst_flow_match_config->acev6_ref) + current_acl_entry_count_ingress) > stm_max_ingress)
		                {
		                    return PM_E_CFG_NO_RESOURCE;
		                }
		            }
		            else
		            {
		                if((pst_acl_config->intf_ref_in*(pst_flow_match_config->ace_ref + pst_flow_match_config->acev6_ref*2) + current_acl_entry_count_ingress) > stm_max_ingress)
		                {
		                    return PM_E_CFG_NO_RESOURCE;
		                }
		            }
		        }
		        else if ((ace_config->is_ipv6) && (pst_acl_config->intf_ref_in*2 + current_acl_entry_count_ingress) > stm_max_ingress)
		        {
		            return PM_E_CFG_NO_RESOURCE;
		        } 
		        else if ((pst_acl_config->intf_ref_in + current_acl_entry_count_ingress) > stm_max_ingress)
		        {
		            return PM_E_CFG_NO_RESOURCE;
		        } 
	    	}else{
	    		/* E350 acl tcam resource need check ipv6 and ipv4 */
				if (ace_config->is_ipv6)
		        {
		        	if((pst_acl_config->intf_ref_in + entry_count.cur_ingress6) > stm_max_ingress6)
		        	{
		            	return PM_E_CFG_NO_RESOURCE;
		        	}
		        } 
		        else 
		        {
		        	if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress) > stm_max_ingress)
		        	{
		            	return PM_E_CFG_NO_RESOURCE;
					}
		        } 
			}
			break;
	    }
		
		case GLB_ACL_CONFIG_TYPE_FLOW_MATCH:
		{
			/* 350 not support match, do not need check resource */
        	int32_t outer_ace_config_ref_count = 0;
  

	        foreach_acl_config_start(acl_config)
	        {
	            int32_t acl_config_ace_flow_match_cnt = 0;
	            foreach_ace_config_start(ace_node)
	            {
	                if(0 != sal_strcmp(ace_node->key.acl_name, acl_config->key.name))
	                {
	                    continue;
	                }
	                
	                if (!GLB_FLAG_ISSET(ace_node->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH) || 
	                    0 != sal_strcmp(ace_node->flow_match_name, ace_config->key.acl_name))
	                {
	                    continue;
	                }

	                if (ace_node->is_ipv6)
	                {
	                    acl_config_ace_flow_match_cnt += 2;
	                }
	                else
	                {
	                    acl_config_ace_flow_match_cnt++;
	                }
	            }foreach_ace_config_end;

	            outer_ace_config_ref_count += ((acl_config_ace_flow_match_cnt) * acl_config->intf_ref_in);
	        }foreach_acl_config_end;
	        
	        if ((outer_ace_config_ref_count + current_acl_entry_count_ingress) > stm_max_ingress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }  
			break;
		}
    	default:
		{
			/* TAP default, only use egress acl */
	    	if(share_resource)
	    	{
	    		/* check v6 */
		        if ((ace_config->is_ipv6) && (pst_acl_config->intf_ref_out*2 + current_acl_entry_count_egress) > stm_max_egress)
		        {
		            return PM_E_CFG_NO_RESOURCE;
		        }
				/* check v4 */
		        if ((pst_acl_config->intf_ref_out + current_acl_entry_count_egress) > stm_max_egress)
		        {
		            return PM_E_CFG_NO_RESOURCE;
		        }
	    	}else{
				if (ace_config->is_ipv6)
		        {
		        	if((pst_acl_config->intf_ref_out + entry_count.cur_egress6) > stm_max_egress6)
		        	{
		            	return PM_E_CFG_NO_RESOURCE;
		        	}
		        }
				else
		        {
		        	if ((pst_acl_config->intf_ref_out + entry_count.cur_egress) > stm_max_egress)
		        	{
		            	return PM_E_CFG_NO_RESOURCE;
		        	}
		        }
			}
    	}
    }

	return PM_E_NONE;
}
#else
static int32
__acl_config_res_check(int32 check_type, 
                           tbl_ace_config_t *ace_config,
                           tbl_acl_config_t *pst_acl_config)
{
	uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;

	uint32 current_acl_entry_count_ingress 		= 0;
    uint32 current_acl_entry_count_egress 		= 0;
	uint32 current_acl_entry_count_wromfilter 	= 0;

	acl_entry_count_t entry_count = {0};

	int32  share_resource = TRUE;

	share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);

	if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}

	if(share_resource)
	{
		if (ace_config->is_ipv6 || ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
	    {
	        if ((pst_acl_config->intf_ref_in*2 + current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

	        if ((pst_acl_config->intf_ref_out*2 + current_acl_entry_count_egress) > stm_max_egress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
	    }
        else if (ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (GLB_FLAG_ISSET(ace_config->key_flags, GLB_MAC_ACE_ETHTYPE) ||
                    GLB_FLAG_ISSET(ace_config->key_arp_flags, GLB_ARP_ACE_OP_CODE))
                {
                    if ((pst_acl_config->intf_ref_in + current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }

        	        if ((pst_acl_config->intf_ref_out + current_acl_entry_count_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
                else
                {
                    if ((pst_acl_config->intf_ref_in*2 + current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }

        	        if ((pst_acl_config->intf_ref_out*2 + current_acl_entry_count_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
            }
            else
            {
                if ((pst_acl_config->intf_ref_in + current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }

    	        if ((pst_acl_config->intf_ref_out + current_acl_entry_count_egress) > stm_max_egress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }
            }
        }
	    else
	    {
	        if ((pst_acl_config->intf_ref_in + current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

	        if ((pst_acl_config->intf_ref_out + current_acl_entry_count_egress) > stm_max_egress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
	    }
	}
    else
    {
		if (ace_config->is_ipv6 || ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
	    {
	        if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress6 + entry_count.cur_worm_filter6) > stm_max_ingress6)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress6) > stm_max_egress6)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
	    }
	    else if (ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
	    {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (GLB_FLAG_ISSET(ace_config->key_flags, GLB_MAC_ACE_ETHTYPE) ||
                    GLB_FLAG_ISSET(ace_config->key_arp_flags, GLB_ARP_ACE_OP_CODE))
                {
                    if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }

        	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
                else
                {   
                    if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
        	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                    if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress6 + entry_count.cur_worm_filter6) > stm_max_ingress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
        	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress6) > stm_max_egress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
            }
            else
            {
                if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }

    	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress) > stm_max_egress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }
            }
            
        }
	    else
        {
            if ((pst_acl_config->intf_ref_in + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

	        if ((pst_acl_config->intf_ref_out + entry_count.cur_egress) > stm_max_egress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
        }
	}
	
	return PM_E_NONE;
}
#endif

static int32
__acl_attach_interface_res_check(bool direct_in, 
                           					tbl_acl_config_t *pst_acl_config)
{
	uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;

	uint32 current_acl_entry_count_ingress 		= 0;
    uint32 current_acl_entry_count_egress 		= 0;
	uint32 current_acl_entry_count_wromfilter 	= 0;

	acl_entry_count_t entry_count = {0};

	int32  share_resource = TRUE;

	share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);

	if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}

	if(share_resource)
	{
		if (direct_in)
	    {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (pst_acl_config->ether_ref || pst_acl_config->arp_ref)
                {
                    if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + 
                        (pst_acl_config->acemac_ref*3 - (pst_acl_config->ether_ref+pst_acl_config->arp_ref)*2) + 
	                    current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
                else
                {
                    if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + pst_acl_config->acemac_ref*3 +
	                    current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
            }
            else
            {
                if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + pst_acl_config->acemac_ref +
	                    current_acl_entry_count_ingress + current_acl_entry_count_wromfilter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
            }
	    }
	    else
	    {
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (pst_acl_config->ether_ref || pst_acl_config->arp_ref)
                {
                    if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + 
                        (pst_acl_config->acemac_ref*3 - (pst_acl_config->ether_ref+pst_acl_config->arp_ref)*2) + current_acl_entry_count_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
                else
                {
                    if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + pst_acl_config->acemac_ref*3 + current_acl_entry_count_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
            }
            else
            {
                if ((pst_acl_config->ace_ref + pst_acl_config->acev6_ref*2 + pst_acl_config->acemac_ref + current_acl_entry_count_egress) > stm_max_egress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }
            }
	        
	    }
	}
    else
	{
		if (direct_in)
	    {
	        if ((pst_acl_config->ace_ref + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

			if ((pst_acl_config->acev6_ref + entry_count.cur_ingress6 + entry_count.cur_worm_filter6) > stm_max_ingress6)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (pst_acl_config->ether_ref || pst_acl_config->arp_ref)
                {
                    if ((pst_acl_config->acemac_ref + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                    if (((pst_acl_config->acemac_ref - pst_acl_config->ether_ref-pst_acl_config->arp_ref) + entry_count.cur_ingress6 + entry_count.cur_worm_filter6) > stm_max_ingress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
                else
                {
                    if ((pst_acl_config->acemac_ref + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                    if ((pst_acl_config->acemac_ref + entry_count.cur_ingress6 + entry_count.cur_worm_filter6) > stm_max_ingress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                }
            }
            else
            {
                if ((pst_acl_config->acemac_ref + entry_count.cur_ingress + entry_count.cur_worm_filter) > stm_max_ingress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }
            }
	    }
	    else
	    {
	        if ((pst_acl_config->ace_ref + entry_count.cur_egress) > stm_max_egress)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }

			if ((pst_acl_config->acev6_ref + entry_count.cur_egress6) > stm_max_egress6)
	        {
	            return PM_E_CFG_NO_RESOURCE;
	        }
            if (TRUE == acl_stm_is_ipv6_profile())
            {
                if (pst_acl_config->ether_ref || pst_acl_config->arp_ref)
                {
                    if ((pst_acl_config->acemac_ref + entry_count.cur_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                    if (((pst_acl_config->acemac_ref - pst_acl_config->ether_ref-pst_acl_config->arp_ref) + entry_count.cur_egress6) > stm_max_egress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }    
                }
                else
                {
                    if ((pst_acl_config->acemac_ref + entry_count.cur_egress) > stm_max_egress)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                    if ((pst_acl_config->acemac_ref + entry_count.cur_egress6) > stm_max_egress6)
        	        {
        	            return PM_E_CFG_NO_RESOURCE;
        	        }
                } 
            }
            else
            {
                if ((pst_acl_config->acemac_ref + entry_count.cur_egress) > stm_max_egress)
    	        {
    	            return PM_E_CFG_NO_RESOURCE;
    	        }
            }
	    }
	}
	
	return PM_E_NONE;
}


/* tbl time range */
int32
acl_time_range_add_time_range(tbl_time_range_t *p_time_range)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(tbl_time_range_add_time_range(p_time_range));
    return rc;
}

int32
acl_time_range_del_time_range(tbl_time_range_t *p_time_range)
{
    int32 rc = PM_E_NONE;

    /* add del handle for acl */
    PM_E_RETURN(tbl_time_range_del_time_range(&p_time_range->key));
    return rc;
}

static int32
_acl_time_range_insert_arg(uint8 type, void* arg, time_range_timer_t* timer)
{
    time_range_arg_t* p_arg;
    
    p_arg = XCALLOC(MEM_UDS_TIME_RANGE_ARG, sizeof(time_range_arg_t));
    if (!p_arg)
    {
        return PM_E_NO_MEMORY;
    }
    
    p_arg->type = type;
    p_arg->arg = arg;
    
    ctclib_slistnode_insert(&timer->arg_list, p_arg);
    
    return PM_E_NONE;
}

static int32
_acl_time_range_remove_arg(time_range_timer_t* timer, time_range_arg_t* range_arg)
{
    ctclib_slistnode_delete(&timer->arg_list, range_arg);
   
    XFREE(MEM_UDS_TIME_RANGE_ARG, range_arg);
    
    return PM_E_NONE;
}

static int32
_acl_time_range_cancel(tbl_time_range_t* p_db_time_range, time_range_arg_t* range_arg)
{
    _acl_time_range_remove_arg(&p_db_time_range->running_timer, range_arg);
    
    if (CTCLIB_SLIST_ISEMPTY(&p_db_time_range->running_timer.arg_list))
    {
        if (NULL != p_db_time_range->running_timer.timer)
        {
            ctc_task_delete_timer(p_db_time_range->running_timer.timer);
            p_db_time_range->running_timer.timer = NULL;
        }
        p_db_time_range->running_timer.status = TIME_RANGE_STATUS_NONE;
        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
    }
    
    return PM_E_NONE;
}

static int32
_acl_time_range_get_delay(per_time_t *time, int64_t *delay)
{
    struct tm *tm = NULL, start_tm;
    int ret = PM_E_NONE;
    struct timeval tv;

    if (NULL == time)
    {
        return PM_E_FAIL;
    }

    sal_memset(&start_tm, 0, sizeof(struct tm));

    sal_gettimeofday(&tv);
    tm = localtime(&tv.tv_sec);
    start_tm.tm_hour = time->hour;
    start_tm.tm_min = time->min;
    start_tm.tm_year = tm->tm_year;
    start_tm.tm_sec = tm->tm_sec;
    start_tm.tm_isdst = -1;

    if (time->day < DAILY)
    {
        start_tm.tm_wday = time->day;
        if ((start_tm.tm_wday - tm->tm_wday) > 0 )
        {
            start_tm.tm_yday = tm->tm_yday + start_tm.tm_wday - tm->tm_wday;
        }
        else if( (start_tm.tm_wday - tm->tm_wday) == 0 )
        {
            if ( (start_tm.tm_hour > tm->tm_hour) || 
                    ((start_tm.tm_min > tm->tm_min) && (start_tm.tm_hour == tm->tm_hour)) )
            {
                start_tm.tm_yday = tm->tm_yday;
            }
            else
            {
                start_tm.tm_yday = tm->tm_yday + 7;
            }
        }
        else
        {
            start_tm.tm_yday = tm->tm_yday + start_tm.tm_wday - tm->tm_wday + 7;
        }
    }
    else if(time->day == DAILY)
    {
        if ( (start_tm.tm_hour > tm->tm_hour) || 
                ((start_tm.tm_min > tm->tm_min) && (start_tm.tm_hour == tm->tm_hour)) )
        {
            start_tm.tm_yday = tm->tm_yday ;
        }
        else
        {
            start_tm.tm_yday = tm->tm_yday + 1;
        }
    }
    else if(time->day == WEEKDAY)
    {

        if ((tm->tm_wday > 0) && (tm->tm_wday < 6))
        {
            if ( (start_tm.tm_hour > tm->tm_hour) || 
                    ((start_tm.tm_min > tm->tm_min) && (start_tm.tm_hour == tm->tm_hour)) )
            {
                start_tm.tm_yday = tm->tm_yday;
            }
            else if(tm->tm_wday == 5)
            {
                start_tm.tm_yday = tm->tm_yday + 3;
            }
            else
            {
                start_tm.tm_yday = tm->tm_yday + 1;
            }
        }
        else
        {
            if (tm->tm_wday == 0)
            {
                start_tm.tm_yday = tm->tm_yday + 1;
            }
            else
            {
                start_tm.tm_yday = tm->tm_yday + 2;
            }
        }
    }
    else
    {
        if ((tm->tm_wday == 0) || (tm->tm_wday == 6))
        {
            if ((start_tm.tm_hour > tm->tm_hour) || 
                    ((start_tm.tm_min > tm->tm_min) && (start_tm.tm_hour == tm->tm_hour)) )
            {
                start_tm.tm_yday = tm->tm_yday ;
            }
            else if(tm->tm_wday == 0)
            {
                start_tm.tm_yday = tm->tm_yday + 6;
            }
            else if(tm->tm_wday == 6)
            {
                start_tm.tm_yday = tm->tm_yday + 1;
            }
        }
        else
        {
            start_tm.tm_yday = tm->tm_yday + 6 - tm->tm_wday;
        }
    }

    *delay = (((start_tm.tm_yday - tm->tm_yday) * 24 + 
            (start_tm.tm_hour - tm->tm_hour)) * 60 +
            (start_tm.tm_min - tm->tm_min)) * 60 - tm->tm_sec;

    return ret;
}

static void
_acl_time_range_invalid_arg_list(ctclib_slist_t* p_arg_list, uint8 invalid)
{
    time_range_arg_t*           p_range_arg = NULL;
    ctclib_slistnode_t *        listnode = NULL;    
    tbl_time_range_master_t     *p_tbl_time_range_master = tbl_time_range_get_master();
    
    CTCLIB_SLIST_LOOP(p_arg_list, p_range_arg, listnode)
    {        
        if (p_tbl_time_range_master->invalid_func[p_range_arg->type])
        {
            p_tbl_time_range_master->invalid_func[p_range_arg->type](p_range_arg->arg, invalid);
        }
    }
    return;
}

static int32
_acl_time_range_gen_timer(tbl_time_range_t* p_db_time_range)
{
    int64 start_delay, end_delay, delay;
    int32 timer = 0;
    struct timeval tv;
    int32  ret = PM_E_NONE;
    absolute_time_t ab_time = p_db_time_range->time.ab_time; 
    periodic_time_t per_time = p_db_time_range->time.per_time;
    
    sal_gettimeofday(&tv);
    if (p_db_time_range->time.type == GLB_TIME_RANGE_TYPE_ABSOLUTE)
    {
        if (0 == ab_time.start)
        {
            if ((ab_time.end - tv.tv_sec) > 0)
            {
                ret =  TIME_RANGE_STATUS_START;
                delay = ab_time.end - tv.tv_sec;
                timer = delay;
                logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");

                p_db_time_range->running_timer.timer = 
                    ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_invalid, p_db_time_range, timer);
            }
            else
            {
                ret = TIME_RANGE_STATUS_END;
                logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
            }
        }
        else
        {
            delay = ab_time.start - tv.tv_sec;
            if (delay < 0 )
            {
                if ((ab_time.end - tv.tv_sec) > 0)
                {
                    ret =  TIME_RANGE_STATUS_START;
                    delay = ab_time.end - tv.tv_sec;
                    timer = delay;
                    logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
                    p_db_time_range->running_timer.timer = 
                        ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_invalid, p_db_time_range, timer);
                    
                }
                else
                {
                    if (ab_time.end == 0)
                    {
                        ret =  TIME_RANGE_STATUS_START;
                        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
                    }
                    else
                    {
                        ret =  TIME_RANGE_STATUS_END;
                        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
                    }
                }
            }
            else
            {
                ret =  TIME_RANGE_STATUS_NOT_START;
                timer = delay;
                p_db_time_range->running_timer.timer = 
                    ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_enable_absolute, p_db_time_range, timer);
                
            }
        }
    }
    else if(p_db_time_range->time.type == GLB_TIME_RANGE_TYPE_PERIODIC)
    {
        _acl_time_range_get_delay(&(per_time.start), &start_delay);
        _acl_time_range_get_delay(&(per_time.end), &end_delay);

        if (end_delay < start_delay)
        {
            delay = end_delay;
            ret =  TIME_RANGE_STATUS_START;
            logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
        }
        else
        {
            delay = start_delay;
            ret = TIME_RANGE_STATUS_NOT_START;
        }
        timer = delay;
        p_db_time_range->running_timer.timer = 
            ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_enable_periodic, p_db_time_range, timer);; 
        
    }
    else
    {
        ret = PM_E_FAIL;
    }

    p_db_time_range->running_timer.status = ret;

    return ret;
}


static int32
_acl_time_range_start_ace(uint64 acl_id, tbl_ace_config_t *p_ip_ace)
{
    int32 ret = PM_E_NONE;
    time_range_arg_ace_t* tm_arg_ace = NULL;
    tbl_time_range_t* p_db_time_range = NULL;
    
    if(NULL == p_ip_ace)
    {
        return PM_E_INVALID_PARAM;
    }

    p_ip_ace->apply_cnt++;
    if ('\0' != p_ip_ace->time_range[0])
    {
        p_db_time_range = tbl_time_range_get_time_range_by_name(p_ip_ace->time_range);
    }
    if (NULL == p_db_time_range)
    {
        p_ip_ace->apply_cnt--;
        return PM_E_NONE;
    }

    tm_arg_ace = XCALLOC(MEM_UDS_TIME_RANGE_ARG_ACE, sizeof (time_range_arg_ace_t));
    if (NULL == tm_arg_ace)
    {
        return PM_E_NO_MEMORY;
    }
    tm_arg_ace->acl_id = acl_id;
    tm_arg_ace->ace = p_ip_ace;

    if (CTCLIB_SLIST_ISEMPTY(&p_db_time_range->running_timer.arg_list))
    {
        ret = _acl_time_range_gen_timer(p_db_time_range);
    }
    else
    {
        ret = p_db_time_range->running_timer.status;
    }

    if (ret == TIME_RANGE_STATUS_START)
    {
        GLB_UNSET_FLAG(p_ip_ace->key_flags, GLB_IP_ACE_INVALID);
    }
    else
    {
        GLB_SET_FLAG(p_ip_ace->key_flags, GLB_IP_ACE_INVALID);
    }

    _acl_time_range_insert_arg(TIME_RANGE_ARG_TYPE_ACE, tm_arg_ace, &p_db_time_range->running_timer);

    return PM_E_NONE;
}

static int32
_acl_time_range_stop_ace(uint64 acl_id, tbl_ace_config_t *p_ip_ace)
{
    tbl_time_range_t *p_db_time_range = NULL;
    tbl_time_range_key_t key;
    time_range_arg_t*              range_arg = NULL;
    ctclib_slistnode_t *        listnode = NULL;    
    ctclib_slistnode_t *        listnode1 = NULL;    
    time_range_arg_ace_t    *arg_ace;
    if (NULL == p_ip_ace)
    {
        return PM_E_NONE;
    }
    
    sal_memset(&key, 0, sizeof(key));
    sal_strcpy(key.name, p_ip_ace->time_range);
    p_db_time_range = tbl_time_range_get_time_range(&key);
    if (NULL == p_db_time_range)
    {
        return PM_E_NONE;
    }

    if (0 != p_ip_ace->apply_cnt)
    {
        p_ip_ace->apply_cnt--;
    }

    CTCLIB_SLIST_LOOP_DEL(&p_db_time_range->running_timer.arg_list, range_arg, listnode, listnode1)
    {
        if (TIME_RANGE_ARG_TYPE_ACE != range_arg->type)
        {
            continue;
        }
        arg_ace = (time_range_arg_ace_t *)range_arg->arg;
        if ((arg_ace->acl_id != acl_id) || (arg_ace->ace != p_ip_ace))
        {
            continue;
        }

        XFREE(MEM_UDS_TIME_RANGE_ARG_ACE, range_arg->arg);
        _acl_time_range_cancel(p_db_time_range, range_arg);                
    }

    return PM_E_NONE;
}


static int32
_acl_time_range_update_refcnt(char* range_name, bool add)
{
    tbl_time_range_t *p_db_time_range = NULL;

    p_db_time_range = tbl_time_range_get_time_range_by_name(range_name);
    if (NULL == p_db_time_range)
    {
        return PM_E_NONE;
    }
    if (add)
    {
        p_db_time_range->ref_cnt++;                
    }
    else
    {
        if (p_db_time_range->ref_cnt)
        {
            p_db_time_range->ref_cnt--;
        }
    }

    return PM_E_NONE;
}

static int32 
_acl_time_range_update_for_clock_change (tbl_time_range_t *p_db_time_range, tbl_iter_args_t *pargs)
{
    int32 rc = PM_E_NONE;
    int32 time_range_status_old = TIME_RANGE_STATUS_NOT_START;

    time_range_status_old = p_db_time_range->running_timer.status;
    if (p_db_time_range->running_timer.timer)
    {
        ctc_task_delete_timer(p_db_time_range->running_timer.timer);
        p_db_time_range->running_timer.timer = NULL;
        p_db_time_range->running_timer.status = TIME_RANGE_STATUS_NONE;
        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
    }

    rc = _acl_time_range_gen_timer(p_db_time_range);
    if (time_range_status_old == TIME_RANGE_STATUS_START)
    {
        if ( (rc == TIME_RANGE_STATUS_NOT_START) || 
                (rc == TIME_RANGE_STATUS_END) )
        {
            _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, TRUE);
        }
    }
    else if ( (time_range_status_old == TIME_RANGE_STATUS_NOT_START) || 
                (time_range_status_old == TIME_RANGE_STATUS_END) )
    {
        if (rc == TIME_RANGE_STATUS_START) 
        {
            _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, FALSE);
        }
    }

    return PM_E_NONE;
}

int32
acl_process_clock_change(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

	(void)argv;
	(void)argc;
	(void)para;

    tbl_time_range_iterate((TBL_ITER_CB_FUNC)_acl_time_range_update_for_clock_change, NULL);

    return rc;
}

int32
acl_process_fea_ptp_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        
        switch (field_id)
        {
        case TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE:
            tbl_time_range_iterate((TBL_ITER_CB_FUNC)_acl_time_range_update_for_clock_change, NULL);
            break;
        }
    }

    return rc;
}


void
acl_time_range_invalid(void *p_data)
{
    tbl_time_range_t* p_db_time_range = (tbl_time_range_t*)p_data;
    
    _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, TRUE);

    p_db_time_range->running_timer.timer = NULL;
    p_db_time_range->running_timer.status = TIME_RANGE_STATUS_END;

    logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
    
    return;
}

void
acl_time_range_enable_absolute(void *p_data)
{
    tbl_time_range_t* p_db_time_range = (tbl_time_range_t*)p_data;
    int64 delay;
    struct timeval tv;
    int32 timer;

    _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, FALSE);

    sal_gettimeofday(&tv);

    if (0 == p_db_time_range->time.ab_time.end)
    {
        p_db_time_range->running_timer.timer = NULL;
        p_db_time_range->running_timer.status = TIME_RANGE_STATUS_START;
        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
        return;
    }
    
    delay = p_db_time_range->time.ab_time.end - tv.tv_sec;
    if (delay < 0)
    {
        p_db_time_range->running_timer.timer = NULL;
        p_db_time_range->running_timer.status = TIME_RANGE_STATUS_END;
        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
    }
    else
    {        
        logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
        p_db_time_range->running_timer.status = TIME_RANGE_STATUS_START;
        timer = delay;
        p_db_time_range->running_timer.timer = 
                ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_invalid, p_db_time_range, timer);
        
    }
    return;
}

void
acl_time_range_enable_periodic(void *p_data)
{
    tbl_time_range_t* p_db_time_range = (tbl_time_range_t*)p_data;
    int64 delay, start, end;
    int32 timer;
  
    _acl_time_range_get_delay(&(p_db_time_range->time.per_time.end), &end);
    _acl_time_range_get_delay(&(p_db_time_range->time.per_time.start), &start);
 
    if ((start < 0) || (end < 0))
    {
        return;
    }
  
    if (p_db_time_range->running_timer.status == TIME_RANGE_STATUS_NOT_START)
    {
        if ((start == 0) || (end < start))
        {
            _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, FALSE);

            p_db_time_range->running_timer.status = TIME_RANGE_STATUS_START;
            delay = end;
            logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Active");
        }
        else
        {
            p_db_time_range->running_timer.status = TIME_RANGE_STATUS_NOT_START;
            delay = start;
        }
    }
    else
    {
        if ((end == 0) || (end > start))
        {
            _acl_time_range_invalid_arg_list(&p_db_time_range->running_timer.arg_list, TRUE);
            
            p_db_time_range->running_timer.status = TIME_RANGE_STATUS_NOT_START;    
            delay = start;
            logid_sys(LOG_TIMERANGE_4_STATE, p_db_time_range->key.name, "Inactive");
        }
        else
        {
            p_db_time_range->running_timer.status = TIME_RANGE_STATUS_START;
            delay = end;
        }
    }

    timer = delay;
    p_db_time_range->running_timer.timer = 
        ctc_task_add_timer(TASK_PRI_NORMAL, acl_time_range_enable_periodic, p_db_time_range, timer);

    return;
}


int32
acl_time_range_register_arg_invalid_cb(uint8 arg_type, TIME_RANGE_CB func)
{
    tbl_time_range_master_t* master = NULL;

    if (arg_type >= TIME_RANGE_ARG_TYPE_MAX)
    {
        return PM_E_FAIL;
    }
    master = tbl_time_range_get_master();
    
    master->invalid_func[arg_type] = func;

    return PM_E_NONE;
}

int32 acl_entry_add_entry_add_action(uint64 acl_id, char* class_map_name, char* policy_map_name)
{
    tbl_acl_entry_action_t acl_entry_action;
    tbl_acl_entry_action_key_t acl_entry_action_key;
    tbl_acl_entry_action_t*  pst_acl_entry_action = NULL;
    tbl_class_map_action_config_t* pst_class_map_action_config = NULL;

    if((NULL == class_map_name) || (NULL == policy_map_name))
    {
        return PM_E_NONE;
    }
    sal_memset(&acl_entry_action, 0x0, sizeof(acl_entry_action));
    acl_entry_action.key.aclid = acl_id;
    tbl_acl_entry_action_add_acl_entry_action(&acl_entry_action);

    sal_memset(&acl_entry_action_key, 0x0, sizeof(acl_entry_action_key));
    acl_entry_action_key.aclid = acl_id;
    pst_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(&acl_entry_action_key);
    if(NULL == pst_acl_entry_action)
    {
        return PM_E_NOT_EXIST;
    }
    pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config_by_name(policy_map_name, class_map_name);
    if(NULL == pst_class_map_action_config)
    {
        return PM_E_NONE;
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_POLICER);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        sal_strcpy(pst_acl_entry_action->policer, pst_class_map_action_config->policer);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_POLICER);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_TC))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_TC);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->tc = pst_class_map_action_config->new_tc;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_TC);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_COLOR);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->color = pst_class_map_action_config->new_color;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_COLOR);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_DSCP);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->dscp = pst_class_map_action_config->new_dscp;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_DSCP);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->session_id = pst_class_map_action_config->session_id;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->new_svlan_id = pst_class_map_action_config->new_svlan_id;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->new_cvlan_id = pst_class_map_action_config->new_cvlan_id;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->new_scos = pst_class_map_action_config->new_scos;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS);
    }
    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->new_ccos = pst_class_map_action_config->new_ccos;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS);
    }

    if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
    {
        GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
        pst_acl_entry_action->redirect_port_ifindex = pst_class_map_action_config->redirect_port_ifindex;
        tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX);
    }
    
    return PM_E_NONE;
}

int32
acl_entry_add_worm_filter(tbl_acl_worm_filter_t *pst_worm_filter)
{
    tbl_acl_entry_t acl_entry;
    tbl_acl_entry_t *pst_acl_entry = NULL;
    uint64 acl_id = 0;
    uint32 entry_priority = 0;
    uint64 i = 0;
    uint64 type = 0;

    if (NULL == pst_worm_filter)
    {
        return PM_E_INVALID_PARAM;
    }

    acl_id = MAKE_ACL_ID(pst_worm_filter->seq_num, 0, 0, i, i, type);
    entry_priority = MAKE_ACL_PRIORITY(pst_worm_filter->seq_num, i, i);

    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.key.aclid = acl_id;
    pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry.key);
    if (NULL != pst_acl_entry)
    {
        return PM_E_EXIST;
    }

    acl_entry.tblid = GLB_IGS_ACL_TBLID;
    acl_entry.key_flags |= GLB_IP_ACE_IP_PROTOCOL;
    if (pst_worm_filter->istcp)
    {
        acl_entry.ip_protocol_mask = 0;
        acl_entry.ip_protocol = GLB_ACL_L4_PROTOCOL_TCP;
    }
    else
    {
        acl_entry.ip_protocol_mask = 0;
        acl_entry.ip_protocol = GLB_ACL_L4_PROTOCOL_UDP;
    }

    if (0 == pst_worm_filter->isrange)
    {
        acl_entry.key_flags |= GLB_IP_ACE_DSTPORT;
        acl_entry.l4_dst_port_type = GLB_ACE_PORT_TYEP_EQ;
        acl_entry.l4_dst_port = pst_worm_filter->dstport;
        acl_entry.l4_dst_port_mask = pst_worm_filter->dstport;
    }
    else
    {
        acl_entry.key_flags |= GLB_IP_ACE_DSTPORT;
        acl_entry.l4_dst_port_type = GLB_ACE_PORT_TYEP_RANGE;
        acl_entry.l4_dst_port = pst_worm_filter->dstport;
        acl_entry.l4_dst_port_mask = pst_worm_filter->isrange;
    }
    
    acl_entry.deny = TRUE;
    acl_entry.entry_priority = entry_priority;
    return tbl_acl_entry_add_acl_entry(&acl_entry);

}

int32
acl_entry_del_worm_filter(tbl_acl_worm_filter_t *pst_worm_filter)
{
    tbl_acl_entry_t *pst_acl_entry = NULL;
    tbl_acl_entry_key_t acl_entry_key;
    uint64 acl_id = 0;
    uint64 i = 0;
    uint64 type = 0;

    if (NULL == pst_worm_filter)
    {
        return PM_E_INVALID_PARAM;
    }

    acl_id = MAKE_ACL_ID(pst_worm_filter->seq_num, 0, 0, i, i, type);

    sal_memset(&acl_entry_key, 0, sizeof(acl_entry_key));
    acl_entry_key.aclid = acl_id;
    pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry_key);
    if (NULL == pst_acl_entry)
    {
        return PM_E_NOT_EXIST;
    }

    return tbl_acl_entry_del_acl_entry(&acl_entry_key);

}

void __merge_inner_field(tbl_acl_entry_t *pst_acl_entry, tbl_ace_config_t *pst_inner_ace)
{
    pst_acl_entry->inner_key_flags = pst_inner_ace->key_flags;
    pst_acl_entry->inner_key_ipv6_flags = pst_inner_ace->key_ipv6_flags;
    pst_acl_entry->inner_is_ipv6 = pst_inner_ace->is_ipv6;
    pst_acl_entry->inner_ipv6_flow_label = pst_inner_ace->ipv6_flow_label;
    pst_acl_entry->inner_ipv6_flow_label_mask = pst_inner_ace->ipv6_flow_label_mask;

    pst_acl_entry->inner_ether_type       = pst_inner_ace->ether_type;
    pst_acl_entry->inner_ether_type_mask  = pst_inner_ace->ether_type_mask;

    sal_memcpy(pst_acl_entry->inner_src_mac,      pst_inner_ace->src_mac, sizeof(mac_addr_t));
    sal_memcpy(pst_acl_entry->inner_src_mac_mask, pst_inner_ace->src_mac_mask, sizeof(mac_addr_t));

    sal_memcpy(pst_acl_entry->inner_dst_mac,      pst_inner_ace->dst_mac, sizeof(mac_addr_t));
    sal_memcpy(pst_acl_entry->inner_dst_mac_mask, pst_inner_ace->dst_mac_mask, sizeof(mac_addr_t));

    pst_acl_entry->inner_svlan        = pst_inner_ace->svlan;
    pst_acl_entry->inner_svlan_mask   = pst_inner_ace->svlan_mask;

    pst_acl_entry->inner_svlan_cos        = pst_inner_ace->svlan_cos;
    pst_acl_entry->inner_svlan_cos_mask   = pst_inner_ace->svlan_cos_mask;

    pst_acl_entry->inner_cvlan        = pst_inner_ace->cvlan;
    pst_acl_entry->inner_cvlan_mask   = pst_inner_ace->cvlan_mask;

    pst_acl_entry->inner_cvlan_cos        = pst_inner_ace->cvlan_cos;
    pst_acl_entry->inner_cvlan_cos_mask   = pst_inner_ace->cvlan_cos_mask;

    sal_memcpy(&pst_acl_entry->inner_src_ip, &pst_inner_ace->src_ip, sizeof(prefix_t));
    sal_memcpy(&pst_acl_entry->inner_src_ip_mask, &pst_inner_ace->src_ip_mask, sizeof(prefix_t));

    sal_memcpy(&pst_acl_entry->inner_dst_ip, &pst_inner_ace->dst_ip, sizeof(prefix_t));
    sal_memcpy(&pst_acl_entry->inner_dst_ip_mask, &pst_inner_ace->dst_ip_mask, sizeof(prefix_t));

    pst_acl_entry->inner_dscp         = pst_inner_ace->dscp;
    pst_acl_entry->inner_dscp_mask    = pst_inner_ace->dscp_mask;

    pst_acl_entry->inner_ip_precedence        = pst_inner_ace->ip_precedence;
    pst_acl_entry->inner_ip_precedence_mask   = pst_inner_ace->ip_precedence_mask;

    pst_acl_entry->inner_ip_protocol          = pst_inner_ace->ip_protocol;
    pst_acl_entry->inner_ip_protocol_mask     = pst_inner_ace->ip_protocol_mask;

    pst_acl_entry->inner_l4_src_port_type     = pst_inner_ace->l4_src_port_type;
    pst_acl_entry->inner_l4_src_port          = pst_inner_ace->l4_src_port;
    pst_acl_entry->inner_l4_src_port_mask     = pst_inner_ace->l4_src_port_mask;

    pst_acl_entry->inner_l4_dst_port_type     = pst_inner_ace->l4_dst_port_type;
    pst_acl_entry->inner_l4_dst_port          = pst_inner_ace->l4_dst_port;
    pst_acl_entry->inner_l4_dst_port_mask     = pst_inner_ace->l4_dst_port_mask;

    pst_acl_entry->inner_ip_frag          = pst_inner_ace->ip_frag;

    pst_acl_entry->inner_tcp_flags        = pst_inner_ace->tcp_flags;
    pst_acl_entry->inner_tcp_flags_mask   = pst_inner_ace->tcp_flags_mask;
    
    pst_acl_entry->inner_igmp_type        = pst_inner_ace->igmp_type;
    pst_acl_entry->inner_icmp_type        = pst_inner_ace->icmp_type;
    pst_acl_entry->inner_icmp_code        = pst_inner_ace->icmp_code;

    pst_acl_entry->inner_options          = pst_inner_ace->options;
}

int32
acl_entry_add_entry(tbl_ace_config_t *pst_ace, 
                    tbl_ace_config_t *pst_inner_ace, 
                    tbl_interface_t *pst_interface, 
                    uint32 direct_in, 
                    uint64 class_priority, 
                    uint64 group_priority,
                    char* class_map_name, 
                    char* policy_map_name, 
                    tbl_tap_group_ingress_flow_t *p_tap_grp_flow,
                    uint32 decap_isv6)
{    
    tbl_acl_entry_t *pst_acl_entry = NULL;
    tbl_acl_entry_t acl_entry;
    uint64 acl_id = 0;
    uint64 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    uint32 entry_priority = 0;
#ifdef TAPPRODUCT    
    tbl_acl_config_key_t acl_config_key  = {};
    tbl_acl_config_t    *pst_acl_config  = NULL;
#endif

    if ((NULL == pst_ace) || (NULL == pst_interface))
    {
        return PM_E_INVALID_PARAM;
    }

    if (GLB_ACL_CONFIG_TYPE_IPV6 == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_IPV6;
    }
    else if (GLB_ACL_CONFIG_TYPE_MAC == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_MAC;
    }
    else if (GLB_ACL_CONFIG_TYPE_NORMAL == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_NORMAL;
    }  
    
    acl_id = MAKE_ACL_ID(pst_ace->key.seq_no, direct_in, pst_interface->ifindex, class_priority, group_priority, type);
#ifdef TAPPRODUCT
    entry_priority =  MAKE_TAP_ACL_PRIORITY(pst_ace->key.seq_no, class_priority, group_priority);

    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));
    sal_strcpy(acl_config_key.name, pst_ace->key.acl_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_INVALID_PARAM;
    }
#else
	entry_priority =  MAKE_ACL_PRIORITY(pst_ace->key.seq_no, class_priority, group_priority);
#endif
    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.key.aclid = acl_id;
    pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry.key);
    if (NULL != pst_acl_entry)
    {
        return PM_E_EXIST;
    }

    if (direct_in)
    {
        acl_entry.in_port = pst_interface->ifindex;
        acl_entry.tblid = GLB_IGS_ACL_TBLID;
    }
    else
    {
        acl_entry.out_port = pst_interface->ifindex;
        acl_entry.tblid = GLB_EGS_ACL_TBLID;
    }

    acl_entry.key_flags = pst_ace->key_flags;
    acl_entry.key_flags2 = pst_ace->key_flags2;
    acl_entry.key_ipv6_flags = pst_ace->key_ipv6_flags;
    acl_entry.is_ipv6 = pst_ace->is_ipv6;
    acl_entry.acl_type = pst_ace->acl_type;
    acl_entry.key_arp_flags = pst_ace->key_arp_flags;
    acl_entry.arp_op_code = pst_ace->arp_op_code;
    acl_entry.arp_op_code_mask = pst_ace->arp_op_code_mask;
    
    sal_memcpy(acl_entry.src_mac, pst_ace->src_mac, sizeof(mac_addr_t));
    sal_memcpy(acl_entry.src_mac_mask, pst_ace->src_mac_mask, sizeof(mac_addr_t));

    sal_memcpy(acl_entry.dst_mac, pst_ace->dst_mac, sizeof(mac_addr_t));
    sal_memcpy(acl_entry.dst_mac_mask, pst_ace->dst_mac_mask, sizeof(mac_addr_t));

    sal_memcpy(acl_entry.time_range, pst_ace->time_range, sizeof(pst_ace->time_range));
    if (GLB_FLAG_ISSET(pst_ace->key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        _acl_time_range_start_ace(acl_id, pst_ace);
    }
    acl_entry.invalid = GLB_FLAG_ISSET(pst_ace->key_flags, GLB_IP_ACE_INVALID) ? TRUE: FALSE;

    acl_entry.svlan = pst_ace->svlan;
    acl_entry.svlan_mask = pst_ace->svlan_mask;

    acl_entry.cvlan = pst_ace->cvlan;
    acl_entry.cvlan_mask = pst_ace->cvlan_mask;

    acl_entry.svlan_cos = pst_ace->svlan_cos;
    acl_entry.svlan_cos_mask = pst_ace->svlan_cos_mask;

    acl_entry.cvlan_cos = pst_ace->cvlan_cos;
    acl_entry.cvlan_cos_mask = pst_ace->cvlan_cos_mask;

    acl_entry.untag_vlan = pst_ace->untag_vlan; /*added by yejl for bug 49791, 2018-11-14*/

    acl_entry.ether_type = pst_ace->ether_type;
    acl_entry.ether_type_mask = pst_ace->ether_type_mask;

    sal_memcpy(&acl_entry.src_ip, &pst_ace->src_ip, sizeof(prefix_t));
    sal_memcpy(&acl_entry.src_ip_mask, &pst_ace->src_ip_mask, sizeof(prefix_t));

    sal_memcpy(&acl_entry.dst_ip, &pst_ace->dst_ip, sizeof(prefix_t));
    sal_memcpy(&acl_entry.dst_ip_mask, &pst_ace->dst_ip_mask, sizeof(prefix_t));
    acl_entry.ipv6_flow_label = pst_ace->ipv6_flow_label;
    acl_entry.ipv6_flow_label_mask = pst_ace->ipv6_flow_label_mask;

    acl_entry.ip_protocol = pst_ace->ip_protocol;
    acl_entry.ip_protocol_mask = pst_ace->ip_protocol_mask;

    acl_entry.l4_src_port_type = pst_ace->l4_src_port_type;
    acl_entry.l4_src_port = pst_ace->l4_src_port;
    acl_entry.l4_src_port_mask = pst_ace->l4_src_port_mask;

    acl_entry.l4_dst_port_type = pst_ace->l4_dst_port_type;
    acl_entry.l4_dst_port = pst_ace->l4_dst_port;
    acl_entry.l4_dst_port_mask = pst_ace->l4_dst_port_mask;
    
    acl_entry.dscp = pst_ace->dscp;
    acl_entry.dscp_mask = pst_ace->dscp_mask;
    acl_entry.ip_precedence = pst_ace->ip_precedence;
    acl_entry.ip_precedence_mask = pst_ace->ip_precedence_mask;
    acl_entry.ip_frag = pst_ace->ip_frag;
    acl_entry.tcp_flags = pst_ace->tcp_flags;
    acl_entry.tcp_flags_mask = pst_ace->tcp_flags_mask;
    acl_entry.igmp_type = pst_ace->igmp_type;
    acl_entry.icmp_type = pst_ace->icmp_type;
    acl_entry.icmp_code = pst_ace->icmp_code;

    acl_entry.udf_type = pst_ace->udf_type;
    acl_entry.udf_id   = pst_ace->udf_id;
    // acl_entry.udf_value = pst_ace->udf_value;
    sal_memcpy(acl_entry.udf_value, pst_ace->udf_value, 4*sizeof(uint32));
    //acl_entry.udf_mask = pst_ace->udf_mask;
    sal_memcpy(acl_entry.udf_mask, pst_ace->udf_mask, 4*sizeof(uint32));
    sal_memcpy(acl_entry.udf_value_valid, pst_ace->udf_value_valid, 4*sizeof(uint32));
    acl_entry.udf_offset = pst_ace->udf_offset;
    
    acl_entry.deny = pst_ace->deny;

    acl_entry.entry_priority = entry_priority;

    acl_entry.options = pst_ace->options;

    if(GLB_FLAG_ISSET(pst_ace->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
    {
        __merge_inner_field(&acl_entry, pst_inner_ace);
    }
#ifdef TAPPRODUCT
    else if (pst_acl_config->decap_en)
    {
        if (TRUE != decap_isv6)
        {
            GLB_SET_FLAG(acl_entry.inner_key_flags, GLB_IP_ACE_IPSA);
            sal_inet_pton(AF_INET, "0.0.0.0", &acl_entry.inner_src_ip.u.prefix4);
            sal_inet_pton(AF_INET, "255.255.255.255", &acl_entry.inner_src_ip_mask.u.prefix4);
            acl_entry.inner_src_ip.family = AF_INET;
        }
        else
        {
            GLB_SET_FLAG(acl_entry.inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA);
            acl_entry.inner_src_ip.family = AF_INET6;
            acl_entry.inner_is_ipv6 = TRUE;
            sal_memset(&acl_entry.inner_src_ip, 0, sizeof(acl_entry.inner_src_ip));
            sal_memset(&acl_entry.inner_src_ip_mask.u.prefix6.__in6_u.__u6_addr8, 0xFF, sizeof(acl_entry.inner_src_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
    }
#endif
    
    if (p_tap_grp_flow)
    {
        acl_entry.tap_group_oid = p_tap_grp_flow->tap_grp_id;
        acl_entry.tap_action_redirect = TRUE;

        if(0 == pst_ace->tap_action_mark_vlan && 0 == pst_ace->tap_action_untag)
        {
            if (p_tap_grp_flow->ing_mark_vlan_en)
            {
                acl_entry.tap_action_mark_vlan = p_tap_grp_flow->ing_mark_vlan_vid;
            }
            if (p_tap_grp_flow->ing_untag_en)
            {
                acl_entry.tap_action_untag = p_tap_grp_flow->ing_untag_en;
            }
        }
    }
    acl_entry.tap_action_trunction = pst_ace->tap_action_trunction;

    if (TRUE == pst_ace->tap_action_edit_dest_mac_en)
    {
        acl_entry.tap_action_edit_dest_mac_en = pst_ace->tap_action_edit_dest_mac_en;
        sal_memcpy(acl_entry.tap_action_edit_dest_mac, 
            pst_ace->tap_action_edit_dest_mac,
            sizeof(mac_addr_t));
    }

    if (TRUE == pst_ace->tap_action_edit_src_mac_en)
    {
        acl_entry.tap_action_edit_src_mac_en = pst_ace->tap_action_edit_src_mac_en;
        sal_memcpy(acl_entry.tap_action_edit_src_mac, 
            pst_ace->tap_action_edit_src_mac,
            sizeof(mac_addr_t));
    }

    if (TRUE == pst_ace->tap_action_edit_ipda_en)
    {
        acl_entry.tap_action_edit_ipda_en = pst_ace->tap_action_edit_ipda_en;
        sal_memcpy(&acl_entry.tap_action_edit_ipda, 
            &pst_ace->tap_action_edit_ipda,
            sizeof(addr_t));
    }

    if (TRUE == pst_ace->tap_action_edit_ipsa_en)
    {
        acl_entry.tap_action_edit_ipsa_en = pst_ace->tap_action_edit_ipsa_en;
        sal_memcpy(&acl_entry.tap_action_edit_ipsa, 
            &pst_ace->tap_action_edit_ipsa,
            sizeof(addr_t));
    }

    acl_entry.tap_action_edit_vlan_en = pst_ace->tap_action_edit_vlan_en;
    acl_entry.tap_action_edit_vlan = pst_ace->tap_action_edit_vlan;
	acl_entry.tap_action_edit_vlan_action = pst_ace->tap_action_edit_vlan_action;

    acl_entry.action_strip_header = pst_ace->action_strip_header;
    acl_entry.strip_header_pos    = pst_ace->strip_header_pos;
    acl_entry.strip_header_offset = pst_ace->strip_header_offset;

    /*added by yejl for bug 53475, 2019-09-10*/
    if (pst_ace->l2gre_add_header)
    {
        acl_entry.l2gre_add_header = pst_ace->l2gre_add_header;
        sal_memcpy(&acl_entry.l2gre_src_ip, &pst_ace->l2gre_src_ip, sizeof(addr_t));
        sal_memcpy(&acl_entry.l2gre_dst_ip, &pst_ace->l2gre_dst_ip, sizeof(addr_t));
        sal_memcpy(acl_entry.l2gre_dst_mac, pst_ace->l2gre_dst_mac, sizeof(mac_addr_t));
        acl_entry.l2gre_key_num = pst_ace->l2gre_key_num;
        acl_entry.l2gre_key_len = pst_ace->l2gre_key_len;
    }

    if (pst_ace->l3gre_add_header)
    {
        acl_entry.l3gre_add_header = pst_ace->l3gre_add_header;
        sal_memcpy(&acl_entry.l3gre_src_ip, &pst_ace->l3gre_src_ip, sizeof(addr_t));
        sal_memcpy(&acl_entry.l3gre_dst_ip, &pst_ace->l3gre_dst_ip, sizeof(addr_t));
        sal_memcpy(acl_entry.l3gre_dst_mac, pst_ace->l3gre_dst_mac, sizeof(mac_addr_t));
    }
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    if (pst_ace->add_vxlan_header)
    {
        acl_entry.add_vxlan_header = pst_ace->add_vxlan_header;
        sal_memcpy(&acl_entry.vxlan_dst_mac, &pst_ace->vxlan_dst_mac, sizeof(mac_addr_t));
        sal_memcpy(&acl_entry.vxlan_src_ip, &pst_ace->vxlan_src_ip, sizeof(addr_t));
        sal_memcpy(&acl_entry.vxlan_dst_ip, &pst_ace->vxlan_dst_ip, sizeof(addr_t));
        acl_entry.vxlan_srcport = pst_ace->vxlan_srcport;
        acl_entry.vxlan_destport = pst_ace->vxlan_destport;
        acl_entry.vxlan_vni = pst_ace->vxlan_vni;
    }
    /*ended by guoxd*/
    
    acl_entry.l4_vxlan_vni = pst_ace->l4_vxlan_vni;
    acl_entry.l4_vxlan_vni_mask = pst_ace->l4_vxlan_vni_mask;

    acl_entry.l4_gre_type = pst_ace->l4_gre_type;
    acl_entry.l4_gre_key = pst_ace->l4_gre_key;
    acl_entry.l4_gre_key_mask = pst_ace->l4_gre_key_mask;

    acl_entry.erspan_id = pst_ace->erspan_id;
    acl_entry.erspan_id_mask = pst_ace->erspan_id_mask;

    if(pst_ace->tap_action_mark_vlan)
    {
        acl_entry.tap_action_mark_vlan = pst_ace->tap_action_mark_vlan;
    }

    if(pst_ace->tap_action_untag)
    {
        acl_entry.tap_action_untag = pst_ace->tap_action_untag;
    }

    if (0 == sal_strcmp(pst_interface->key.name , GLB_IFNAME_CPU_PREFIX))
    {
        acl_entry.is_copp = TRUE;
    }
    
    if(acl_entry.tap_action_mark_vlan)
    {
        acl_entry.tap_action_untag = 0;
    }
    
    tbl_acl_entry_add_acl_entry(&acl_entry);
    return acl_entry_add_entry_add_action(acl_id, class_map_name, policy_map_name);
}

int32
acl_entry_del_entry(tbl_ace_config_t *pst_ace,
                    tbl_ace_config_t *pst_inner_ace,
                    tbl_interface_t *pst_interface, 
                    uint32 direct_in, 
                    uint64 class_priority, 
                    uint64 group_priority)
{
    tbl_acl_entry_t *pst_acl_entry = NULL;
    tbl_acl_entry_key_t acl_entry_key;
    uint64 acl_id = 0;
    uint64 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    tbl_acl_entry_action_key_t acl_entry_action_key;
    
    if ((NULL == pst_ace) || (NULL == pst_interface))
    {
        return PM_E_INVALID_PARAM;
    }

    if (GLB_ACL_CONFIG_TYPE_IPV6 == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_IPV6;
    }
    else if (GLB_ACL_CONFIG_TYPE_MAC == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_MAC;
    }
    else if (GLB_ACL_CONFIG_TYPE_NORMAL == pst_ace->acl_type)
    {
        type = GLB_ACL_CONFIG_TYPE_NORMAL;
    }  
    
    acl_id = MAKE_ACL_ID(pst_ace->key.seq_no, direct_in, pst_interface->ifindex, class_priority, group_priority, type);

    sal_memset(&acl_entry_key, 0, sizeof(acl_entry_key));
    acl_entry_key.aclid = acl_id;
    pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry_key);
    if (NULL == pst_acl_entry)
    {
        return PM_E_NOT_EXIST;
    }
    _acl_time_range_stop_ace(acl_id, pst_ace);

    sal_memset(&acl_entry_action_key, 0x0, sizeof(tbl_acl_entry_action_key_t));
    acl_entry_action_key.aclid = acl_id;
    tbl_acl_entry_action_del_acl_entry_action(&acl_entry_action_key);
    
    return tbl_acl_entry_del_acl_entry(&acl_entry_key);
}

int32
_acl_entry_attach_interface_ace(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_interface_t *pst_interface = pargs->argv[0];
    uint32 *direct_in              = pargs->argv[1];
    uint32 *type                   = pargs->argv[2];
    uint32 *ret                    = pargs->argv[3];

    if ((NULL == p_ace_config) || (NULL == pst_interface))
    {
        return PM_E_NONE;
    }

    if (*direct_in)
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == *type)
        {
            /*for ip acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == *type)
        {
            /*for ipv6 acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_ipv6_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == *type)
        {
            /*for mac acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_mac_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/
        }
    }
    else
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == *type)
        {
            /*for ip acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == *type)
        {
            /*for ipv6 acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_ipv6_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == *type)
        {
            /*for mac acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_mac_acl))
            {
                *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
            }
            /*ended*/  
        }
    }

    return PM_E_NONE;
}

int32
_acl_entry_detach_interface_ace(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_interface_t *pst_interface = pargs->argv[0];
    uint32 *direct_in              = pargs->argv[1];
    uint32 *type                   = pargs->argv[2];
    uint32 *ret                    = pargs->argv[3];

    if ((NULL == p_ace_config) || (NULL == pst_interface))
    {
        return PM_E_NONE;
    }

    if (*direct_in)
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == *type)
        {
            /*for ip acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == *type)
        {
            /*for ipv6 acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_ipv6_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == *type)
        {
            /*for mac acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->igs_mac_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, TRUE, 0, 0);
            }
            /*ended*/
        }
    }
    else
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == *type)
        {
            /*for ip acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0);
            }
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == *type)
        {
            /*for ipv6 acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_ipv6_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0);
            }
            /*ended*/
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == *type)
        {
            /*for mac acl*/
            if (0 == sal_strcmp(p_ace_config->key.acl_name, pst_interface->egs_mac_acl))
            {
                *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, FALSE, 0, 0);
            }
            /*ended*/
        }
    }

    return PM_E_NONE;
}

int32
acl_entry_attach_interface(tbl_acl_config_t *pst_acl_config, tbl_interface_t *pst_interface, uint32 direct_in)
{
    tbl_iter_args_t args;
    uint32 type = 0;
    int32 ret = 0;

    if (pst_acl_config)
    {
        type = pst_acl_config->type_identifying;
    }
    args.argv[0] = pst_interface;
    args.argv[1] = &direct_in;
    args.argv[2] = &type;
    args.argv[3] = &ret;
    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_entry_attach_interface_ace, &args);
    return ret;
}

int32
acl_entry_detach_interface(tbl_acl_config_t *pst_acl_config, tbl_interface_t *pst_interface, uint32 direct_in)
{
    tbl_iter_args_t args;
    uint32 type = 0;
    int32 ret = 0;
    
    if (pst_acl_config)
    {
        type = pst_acl_config->type_identifying;
    }
    args.argv[0] = pst_interface;
    args.argv[1] = &direct_in;
    args.argv[2] = &type;
    args.argv[3] = &ret;
    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_entry_detach_interface_ace, &args);
    return ret;
}

int32
_acl_entry_add_ace(tbl_interface_t *pst_interface, tbl_iter_args_t *pargs)
{
    tbl_ace_config_t *pst_ace_config = pargs->argv[0];
    uint32 *ret                      = pargs->argv[1];

    if ((NULL == pst_ace_config) || (NULL == pst_interface))
    {
        return PM_E_NONE;
    }

    /*for ip acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
    }
    /*ended*/

    /*for ipv6 acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_ipv6_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_ipv6_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
    }
    /*ended*/

     /*for mac acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_mac_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0, NULL, NULL, NULL, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_mac_acl))
    {
        *ret = acl_entry_add_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0, NULL, NULL, NULL, 0);
    }
    /*ended*/

    return PM_E_NONE;
}

int32
_acl_entry_del_ace(tbl_interface_t *pst_interface, tbl_iter_args_t *pargs)
{
    tbl_ace_config_t *pst_ace_config = pargs->argv[0];
    uint32 *ret                      = pargs->argv[1];

    if ((NULL == pst_ace_config) || (NULL == pst_interface))
    {
        return PM_E_NONE;
    }

    /*for IP acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0);
    }
    /*ended*/
    
    /*for ipv6 acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_ipv6_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_ipv6_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0);
    }
    /*ended*/

    /*for mac acl*/
    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->igs_mac_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, TRUE, 0, 0);
    }

    if (0 == sal_strcmp(pst_ace_config->key.acl_name, pst_interface->egs_mac_acl))
    {
        *ret = acl_entry_del_entry(pst_ace_config, NULL, pst_interface, FALSE, 0, 0);
    }
    /*ended*/

    return PM_E_NONE;
}

int32
_acl_worm_filter_check(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    tbl_acl_worm_filter_t *pst_new_worm_filter = pargs->argv[0];

    if ((pst_new_worm_filter->dstport == p_acl_worm_filter->dstport) 
        && (pst_new_worm_filter->istcp == p_acl_worm_filter->istcp)
        && (pst_new_worm_filter->isrange == p_acl_worm_filter->isrange))
    {
        pargs->argv[1] = p_acl_worm_filter;
    }

    return PM_E_NONE;
}


int32
_acl_check_ace_config(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_ace_config_t *pst_new_ace = pargs->argv[0];
	uint32               key_flags = 0;
	uint32               new_key_flags = 0;
	uint32               key_flags2 = 0;
	uint32               new_key_flags2 = 0;
    if (0 != sal_strcmp(p_ace_config->key.acl_name, pst_new_ace->key.acl_name))
    {
        return PM_E_NONE;
    }
    if (p_ace_config->key_ipv6_flags != pst_new_ace->key_ipv6_flags)
    {
        return PM_E_NONE;
    }
    if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        if (0 != sal_memcmp(&p_ace_config->src_ip.u.prefix6, 
                            &pst_new_ace->src_ip.u.prefix6, 
                            sizeof(p_ace_config->src_ip.u.prefix6)))
        {
            return PM_E_NONE;
        }
    }
    if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        if (0 != sal_memcmp(&p_ace_config->dst_ip.u.prefix6, 
                            &pst_new_ace->dst_ip.u.prefix6, 
                            sizeof(p_ace_config->dst_ip.u.prefix6)))
        {
            return PM_E_NONE;
        }
    }

    if (p_ace_config->key_arp_flags != pst_new_ace->key_arp_flags)
    {
        return PM_E_NONE;
    }

	key_flags = p_ace_config->key_flags;
	key_flags &= ~GLB_IP_ACE_TIME_RANGE;
	key_flags &= ~GLB_IP_ACE_INVALID;
	key_flags &= ~GLB_IP_ACE_TIMER_ACTIVE;
#ifdef TAPPRODUCT
        key_flags &= ~GLB_IP_ACE_TAP_ACTION; 
        key_flags &= ~GLB_IP_ACE_TAP_TRUNCATION;
        key_flags &= ~GLB_IP_ACE_TAP_UNTAG;
        key_flags &= ~GLB_IP_ACE_TAP_MARK_SOURCE;
#endif

	new_key_flags = pst_new_ace->key_flags;
	new_key_flags &= ~GLB_IP_ACE_TIME_RANGE;
	new_key_flags &= ~GLB_IP_ACE_INVALID;
	new_key_flags &= ~GLB_IP_ACE_TIMER_ACTIVE;
#ifdef TAPPRODUCT
        new_key_flags &= ~GLB_IP_ACE_TAP_ACTION; 
        new_key_flags &= ~GLB_IP_ACE_TAP_TRUNCATION;
        new_key_flags &= ~GLB_IP_ACE_TAP_UNTAG; 
        new_key_flags &= ~GLB_IP_ACE_TAP_MARK_SOURCE;
#endif

	key_flags2 = p_ace_config->key_flags2;
#ifdef TAPPRODUCT
	key_flags2 &= ~GLB_IP_ACE_STRIP_HEADER_POS;
	key_flags2 &= ~GLB_IP_ACE_STRIP_HEADER_OFFSET;
	key_flags2 &= ~GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER;
    key_flags2 &= ~GLB_IP_ACE_ADD_L2GRE_KEY;
#endif

	new_key_flags2 = pst_new_ace->key_flags2;
#ifdef TAPPRODUCT
	new_key_flags2 &= ~GLB_IP_ACE_STRIP_HEADER_POS;
	new_key_flags2 &= ~GLB_IP_ACE_STRIP_HEADER_OFFSET;
	new_key_flags2 &= ~GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER;
    new_key_flags2 &= ~GLB_IP_ACE_ADD_L2GRE_KEY;
#endif

    if ((key_flags != new_key_flags) ||
	 (key_flags2 != new_key_flags2))
    {
        return PM_E_NONE;
    }
    else
    {
        if (((p_ace_config->ether_type & (0xffff - p_ace_config->ether_type_mask)) ==
            (pst_new_ace->ether_type & (0xffff - pst_new_ace->ether_type_mask))) &&
            (((p_ace_config->src_mac[0] & (0xff - p_ace_config->src_mac_mask[0])) ==
            (pst_new_ace->src_mac[0] & (0xff - pst_new_ace->src_mac_mask[0]))) &&
            ((p_ace_config->src_mac[1] & (0xff - p_ace_config->src_mac_mask[1]))== 
            (pst_new_ace->src_mac[1] & (0xff - pst_new_ace->src_mac_mask[1]))) &&
            ((p_ace_config->src_mac[2] & (0xff - p_ace_config->src_mac_mask[2]))== 
            (pst_new_ace->src_mac[2] & (0xff - pst_new_ace->src_mac_mask[2]))) &&
            ((p_ace_config->src_mac[3] & (0xff - p_ace_config->src_mac_mask[3]))== 
            (pst_new_ace->src_mac[3] & (0xff - pst_new_ace->src_mac_mask[3]))) &&
            ((p_ace_config->src_mac[4] & (0xff - p_ace_config->src_mac_mask[4]))== 
            (pst_new_ace->src_mac[4] & (0xff - pst_new_ace->src_mac_mask[4]))) &&
            ((p_ace_config->src_mac[5] & (0xff - p_ace_config->src_mac_mask[5]))== 
            (pst_new_ace->src_mac[5] & (0xff - pst_new_ace->src_mac_mask[5]))) &&
			0 == sal_memcmp(p_ace_config->src_mac_mask, pst_new_ace->src_mac_mask, sizeof(pst_new_ace->src_mac_mask)))&&
            (((p_ace_config->dst_mac[0] & (0xff - p_ace_config->dst_mac_mask[0]))== 
            (pst_new_ace->dst_mac[0] & (0xff - pst_new_ace->dst_mac_mask[0]))) &&
            ((p_ace_config->dst_mac[1] & (0xff - p_ace_config->dst_mac_mask[1]))== 
            (pst_new_ace->dst_mac[1] & (0xff - pst_new_ace->dst_mac_mask[1]))) &&
            ((p_ace_config->dst_mac[2] & (0xff - p_ace_config->dst_mac_mask[2]))== 
            (pst_new_ace->dst_mac[2] & (0xff - pst_new_ace->dst_mac_mask[2]))) &&
            ((p_ace_config->dst_mac[3] & (0xff - p_ace_config->dst_mac_mask[3]))== 
            (pst_new_ace->dst_mac[3] & (0xff - pst_new_ace->dst_mac_mask[3]))) &&
            ((p_ace_config->dst_mac[4] & (0xff - p_ace_config->dst_mac_mask[4]))== 
            (pst_new_ace->dst_mac[4] & (0xff - pst_new_ace->dst_mac_mask[4]))) &&
            ((p_ace_config->dst_mac[5] & (0xff - p_ace_config->dst_mac_mask[5]))== 
            (pst_new_ace->dst_mac[5] & (0xff - pst_new_ace->dst_mac_mask[5]))) &&
			0 == sal_memcmp(p_ace_config->dst_mac_mask, pst_new_ace->dst_mac_mask, sizeof(pst_new_ace->dst_mac_mask)))&&	
            (p_ace_config->svlan  == pst_new_ace->svlan) &&
            (p_ace_config->svlan_mask == pst_new_ace->svlan_mask) &&          
            (p_ace_config->cvlan == pst_new_ace->cvlan) &&
            (p_ace_config->cvlan_mask == pst_new_ace->cvlan_mask) &&
            (p_ace_config->svlan_cos == pst_new_ace->svlan_cos) &&
            (p_ace_config->svlan_cos_mask == pst_new_ace->svlan_cos_mask) &&
            (p_ace_config->cvlan_cos == pst_new_ace->cvlan_cos) &&
            (p_ace_config->cvlan_cos_mask == pst_new_ace->cvlan_cos_mask) &&
            (p_ace_config->ip_precedence == pst_new_ace->ip_precedence) &&
            (p_ace_config->ip_precedence_mask == pst_new_ace->ip_precedence_mask) &&
            (p_ace_config->dscp == pst_new_ace->dscp) && 
            (p_ace_config->dscp_mask == pst_new_ace->dscp_mask) &&  
            (p_ace_config->ip_frag == pst_new_ace->ip_frag) &&  
            ((p_ace_config->src_ip.u.prefix4.s_addr == pst_new_ace->src_ip.u.prefix4.s_addr) &&
            p_ace_config->src_ip_mask.u.prefix4.s_addr == pst_new_ace->src_ip_mask.u.prefix4.s_addr)&&
            ((p_ace_config->dst_ip.u.prefix4.s_addr == pst_new_ace->dst_ip.u.prefix4.s_addr) && 
            p_ace_config->dst_ip_mask.u.prefix4.s_addr == pst_new_ace->dst_ip_mask.u.prefix4.s_addr)&&
            (pst_new_ace->src_ip.family == p_ace_config->src_ip.family)&&
            (pst_new_ace->dst_ip.family == p_ace_config->dst_ip.family)&&
            (0 == sal_memcmp(&p_ace_config->src_ip.u.prefix6, 
                            &pst_new_ace->src_ip.u.prefix6, 
                            sizeof(p_ace_config->src_ip.u.prefix6)))&&
            (0 == sal_memcmp(&p_ace_config->dst_ip.u.prefix6, 
                            &pst_new_ace->dst_ip.u.prefix6, 
                            sizeof(p_ace_config->dst_ip.u.prefix6)))&&
            (0 == sal_memcmp(&p_ace_config->src_ip_mask.u.prefix6, 
                            &pst_new_ace->src_ip_mask.u.prefix6, 
                            sizeof(p_ace_config->src_ip_mask.u.prefix6)))&&
            (0 == sal_memcmp(&p_ace_config->dst_ip_mask.u.prefix6, 
                            &pst_new_ace->dst_ip_mask.u.prefix6, 
                            sizeof(p_ace_config->dst_ip_mask.u.prefix6)))&&
            (pst_new_ace->ipv6_flow_label == p_ace_config->ipv6_flow_label)&&
            (pst_new_ace->ipv6_flow_label_mask == p_ace_config->ipv6_flow_label_mask)&&
            (p_ace_config->ip_protocol == pst_new_ace->ip_protocol) &&
            (p_ace_config->ip_protocol_mask == pst_new_ace->ip_protocol_mask) &&
            p_ace_config->tcp_flags == pst_new_ace->tcp_flags &&
            p_ace_config->tcp_flags_mask == pst_new_ace->tcp_flags_mask &&
            p_ace_config->igmp_type == pst_new_ace->igmp_type &&
            p_ace_config->icmp_type == pst_new_ace->icmp_type &&
            p_ace_config->icmp_code == pst_new_ace->icmp_code &&
            p_ace_config->l4_src_port_type == pst_new_ace->l4_src_port_type &&
            p_ace_config->l4_src_port == pst_new_ace->l4_src_port &&
            p_ace_config->l4_src_port_mask == pst_new_ace->l4_src_port_mask &&
            p_ace_config->l4_dst_port_type == pst_new_ace->l4_dst_port_type &&
            p_ace_config->l4_dst_port == pst_new_ace->l4_dst_port &&
            p_ace_config->l4_dst_port_mask == pst_new_ace->l4_dst_port_mask &&
            p_ace_config->l4_vxlan_vni == pst_new_ace->l4_vxlan_vni &&
            p_ace_config->l4_vxlan_vni_mask == pst_new_ace->l4_vxlan_vni_mask &&
            p_ace_config->l4_gre_key == pst_new_ace->l4_gre_key &&
            p_ace_config->l4_gre_key_mask == pst_new_ace->l4_gre_key_mask &&
            p_ace_config->l4_gre_type == pst_new_ace->l4_gre_type &&
            !sal_memcmp(p_ace_config->udf_value, pst_new_ace->udf_value, 4*sizeof(uint32)) &&
            !sal_memcmp(p_ace_config->udf_mask, pst_new_ace->udf_mask, 4*sizeof(uint32)) &&
            p_ace_config->arp_op_code == pst_new_ace->arp_op_code &&
            p_ace_config->erspan_id == pst_new_ace->erspan_id)
        {
            if(GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
            {
                if(p_ace_config->deny != pst_new_ace->deny)
                {
                    return PM_E_NONE;
                }

                if (0 != sal_strcmp(p_ace_config->flow_match_name, pst_new_ace->flow_match_name))
                {
                    return PM_E_NONE;
                }
            }

	     if(GLB_FLAG_ISSET(p_ace_config->key_flags2, GLB_IP_ACE_ERSPAN_ID))
	     {
	     		key_flags2 = p_ace_config->key_flags2;
			key_flags2 &= GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER;
	     		new_key_flags2 = pst_new_ace->key_flags2;
			new_key_flags2 &= GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER;
				
			if(key_flags2 !=  new_key_flags2)
			{
				return PM_E_NONE;
			}
	     }
            pargs->argv[1] = p_ace_config;
        }
    }
    return PM_E_NONE;
}


bool
ace_entry_config_check(tbl_ace_config_t *pst_ace_config)
{
    tbl_iter_args_t args;
    tbl_ace_config_t *pst_same_ace = NULL;

    args.argv[0] = pst_ace_config;
    args.argv[1] = pst_same_ace;
    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_check_ace_config, &args);
    if (args.argv[1])
    {
        return TRUE;
    }
    return FALSE;
}

bool
acl_worm_filter_config_check(tbl_acl_worm_filter_t *pst_acl_worm_fitler)
{
    tbl_iter_args_t args;
    tbl_acl_worm_filter_t *pst_same_worm_filter = NULL;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = pst_acl_worm_fitler;
    args.argv[1] = pst_same_worm_filter;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)_acl_worm_filter_check, &args);
    if (args.argv[1])
    {
        return TRUE;
    }
    return FALSE;
}

int32
acl_entry_add_ace(tbl_ace_config_t *pst_ace_config)
{
    tbl_iter_args_t args;
    int32 ret = 0;

    args.argv[0] = pst_ace_config;
    args.argv[1] = &ret;
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_acl_entry_add_ace, &args);
    return ret;
}

int32
acl_entry_del_ace(tbl_ace_config_t *pst_ace_config)
{
    tbl_iter_args_t args;
    int32 ret = 0;

    args.argv[0] = pst_ace_config;
    args.argv[1] = &ret;
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_acl_entry_del_ace, &args);
    return ret;
}



void
acl_time_range_ace_invalid_cb(time_range_arg_ace_t* tm_arg, bool invalid)
{
    uint64 acl_id = tm_arg->acl_id;
    tbl_ace_config_t *ace = tm_arg->ace;
    tbl_acl_entry_t acl_entry;
    tbl_acl_entry_t *pst_acl_entry = NULL;

    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    acl_entry.key.aclid = acl_id;
    pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry.key);
    if (NULL == pst_acl_entry)
    {
        return ;
    }

    if (invalid)
    {
        pst_acl_entry->invalid = TRUE;
        tbl_acl_entry_set_acl_entry_field(pst_acl_entry, TBL_ACL_ENTRY_FLD_INVALID);
        GLB_SET_FLAG(ace->key_flags, GLB_IP_ACE_INVALID);
    }
    else
    {
        pst_acl_entry->invalid = FALSE;
        tbl_acl_entry_set_acl_entry_field(pst_acl_entry, TBL_ACL_ENTRY_FLD_INVALID);
        GLB_UNSET_FLAG(ace->key_flags, GLB_IP_ACE_INVALID);
    }

    return ;
}


int32
acl_attach_interface(char *acl_name, char *interface_name, uint32 type, bool direct_in)
{
    tbl_interface_t *pst_interface   = NULL;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;

    uint32 current_acl_cfg_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 policy_tcp_flags_count = acl_policy_get_tcp_flags_config_count();

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_INVALID_IFNAME;
    }

    if (check_name_character_and_len(acl_name, ACL_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }

    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));
    sal_strcpy(acl_config_key.name, acl_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_NOT_EXIST;
    }
    /*added by yejl for IP/IPv6/MAC acl*/
    if (pst_acl_config->type_identifying != type)
    {
        return PM_E_NOT_SUPPORT;
    }
    /*ended by yejl*/
    
	if(__acl_attach_interface_res_check(direct_in, pst_acl_config))
	{
        return PM_E_CFG_NO_RESOURCE;
    }
    
    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
    if (pst_acl_config->tcp_flags_ref + current_acl_cfg_tcp_flags_count + policy_tcp_flags_count > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }
    /*end*/
    
    if (direct_in)
    {
        /*for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            sal_strcpy(pst_interface->igs_ipv6_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_IPV6_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            sal_strcpy(pst_interface->igs_mac_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_MAC_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            sal_strcpy(pst_interface->igs_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_ACL);
            
        }
        /*ended*/
        acl_entry_attach_interface(pst_acl_config, pst_interface, TRUE);
        pst_acl_config->intf_ref_in++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_INTF_REF_IN);
    }
    else
    {
        if (GLB_CHIP_GREATEBELT == tbl_chip_get_type() && pst_acl_config->ether_ref)
        {
            return PM_E_CFG_PM_ERROR;
        }
        /*for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            sal_strcpy(pst_interface->egs_ipv6_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_IPV6_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            sal_strcpy(pst_interface->egs_mac_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_MAC_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            sal_strcpy(pst_interface->egs_acl, acl_name);
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_ACL);
        }
        /*ended*/
        acl_entry_attach_interface(pst_acl_config, pst_interface, FALSE);
        pst_acl_config->intf_ref_out++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_INTF_REF_OUT);
#ifdef TAPPRODUCT
        pst_interface->egs_acl_en = TRUE;
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_ACL_EN);
#endif
    }

    return PM_E_NONE;
}

int32
acl_detach_interface(char *acl_name, char *interface_name, uint32 type, bool direct_in)
{
    tbl_interface_t *pst_interface = NULL;
    tbl_acl_config_t *pst_acl_config   = NULL;
    tbl_acl_config_key_t acl_config_key;

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_INVALID_IFNAME;
    }

    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));
    sal_strcpy(acl_config_key.name, acl_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_NOT_EXIST;
    }
    /*added by yejl for IP/IPv6/MAC acl*/
    if (pst_acl_config->type_identifying != type)
    {
        return PM_E_NOT_SUPPORT;
    }
    /*ended*/

    if (direct_in)
    {
        /*for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, TRUE);
            sal_memset(pst_interface->igs_ipv6_acl, 0, sizeof(pst_interface->igs_ipv6_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_IPV6_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, TRUE);
            sal_memset(pst_interface->igs_mac_acl, 0, sizeof(pst_interface->igs_mac_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_MAC_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, TRUE);
            sal_memset(pst_interface->igs_acl, 0, sizeof(pst_interface->igs_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_ACL);
        }
        /*ended*/
        pst_acl_config->intf_ref_in--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_INTF_REF_IN);
    }
    else
    {
         /*for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, FALSE);
            sal_memset(pst_interface->egs_ipv6_acl, 0, sizeof(pst_interface->egs_ipv6_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_IPV6_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, FALSE);
            sal_memset(pst_interface->egs_mac_acl, 0, sizeof(pst_interface->egs_mac_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_MAC_ACL);
        }
        else if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            acl_entry_detach_interface(pst_acl_config, pst_interface, FALSE);
            sal_memset(pst_interface->egs_acl, 0, sizeof(pst_interface->egs_acl));
            tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_ACL);
        }
        /*ended*/
        pst_acl_config->intf_ref_out--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_INTF_REF_OUT);
#ifdef TAPPRODUCT
        pst_interface->egs_acl_en = FALSE;
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_ACL_EN);
#endif
    }

    return PM_E_NONE;
}


static int32
_acl_cmd_acl_get_ip_frag(char *str, uint32 *ip_frag, cfg_cmd_para_t *para)
{
    if(sal_strcmp ((str), "first-fragment") == 0)
    {
        *ip_frag = GLB_ACL_IP_FRAG_FIRST_FRAG;
    }
    else if(sal_strcmp ((str), "non-first-fragment") == 0)
    {
        *ip_frag =  GLB_ACL_IP_FRAG_NON_FIRST;
    }
    else if(sal_strcmp ((str), "non-fragment") == 0)
    {
        *ip_frag =  GLB_ACL_IP_FRAG_NONE;
    }
    else if(sal_strcmp ((str), "non-or-first-fragment") == 0)
    {
        *ip_frag =  GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG;
    }
    else if(sal_strcmp ((str), "small-fragment") == 0)
    {
        *ip_frag =  GLB_ACL_IP_FRAG_TINY;
    }
    else if(sal_strcmp ((str), "any-fragment") == 0)
    {
        *ip_frag =  GLB_ACL_IP_FRAG_YES;
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid ip frag(%s)", str);
    }
    return PM_E_NONE;
    
}

static int32
_acl_cmd_acl_get_tcp_flag(uint32 *operator, uint32 *flag, char **argv)
{
    int i = 1;

    if(sal_strcmp (argv[i], "match-all") == 0)
    {
        *operator = GLB_ACL_TCP_FLAG_OP_ALL;
        i++;
    }
    else if(sal_strcmp (argv[i], "match-any") == 0)
    {
        *operator = GLB_ACL_TCP_FLAG_OP_ANY;
        i++;
    }
    else
    {
        return PM_E_FAIL;
    }

    if (sal_strlen (argv[i++]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_ACK);        
    }
    if (sal_strlen (argv[i++]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_FIN);        
    }
    if (sal_strlen (argv[i++]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_PSH);        
    }    
    if (sal_strlen (argv[i++]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_RST);        
    }
    if (sal_strlen (argv[i++]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_SYN);        
    }
    if (sal_strlen (argv[i]))
    {
        GLB_SET_FLAG(*flag, GLB_ACL_TCP_FLAG_URG);        
    }

    return PM_E_NONE;
}

int32
_compare_acl_name_and_aclname(tbl_vty_t *p_vty, tbl_iter_args_t *pargs)
{
    char* acl_name = pargs->argv[0];

    if ((!sal_strcmp(p_vty->ipv4_acl, acl_name))
        || (!sal_strcmp(p_vty->ipv6_acl, acl_name)))
    {
        return 1;
    }
    return 0;
}

#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */

//#define __DEBUG_ACL_L4_PORT_ENABLE__
void __l4_port_range__inspect__(void)
{
#ifdef __DEBUG_ACL_L4_PORT_ENABLE__
    uint32 i;
    FILE *fp = fopen("/dev/console", "a+");

    tbl_acl_l4_port_range_key_t key;
    tbl_acl_l4_port_range_t *ptr;

    fprintf(fp, "*** enter debug ***\n");
    fprintf(fp, "+----------+----------+----------+----------+----------+\n");
    fprintf(fp, "|    id    |   min    |   max    |   cnt    |  is_src  |\n");
    fprintf(fp, "+----------+----------+----------+----------+----------+\n");

    for(i = 0;  i < GLB_ACL_L4PORT_ACE_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_l4_port_range_get_acl_l4_port_range(&key);

        if(NULL == ptr)
        {
            fprintf(fp, "|%10d|   NULL   |   NULL   |   NULL   |   NULL   |\n", i);
            fprintf(fp, "+----------+----------+----------+----------+----------+\n");
        }
        else
        {
            fprintf(fp, "|%10d|", ptr->key.id);
            fprintf(fp, "%10d|", ptr->port_min);
            fprintf(fp, "%10d|", ptr->port_max);
            fprintf(fp, "%10d|", ptr->cnt);
            fprintf(fp, "%10d|\n", ptr->is_src);
            fprintf(fp, "+----------+----------+----------+----------+----------+\n");
        }
    }

#endif
}

bool _check_l4_port_range_conflict(uint16 min, uint16 max, uint8 is_src)
{
    uint32 i;
    tbl_acl_l4_port_range_key_t key;
    tbl_acl_l4_port_range_t *ptr;
    tbl_acl_l4_port_range_t acl_l4_port_range;

    /* merge port range */
    for(i = 0;  i < GLB_ACL_L4PORT_ACE_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_l4_port_range_get_acl_l4_port_range(&key);

        if(NULL != ptr && 0 != ptr->cnt && ptr->port_min == min && ptr->port_max == max && ptr->is_src == is_src)
        {
            ptr->cnt++;
            __l4_port_range__inspect__();
            return FALSE; /* no conflict */
        }
    }

    /* add new port range */
    for(i = 0;  i < GLB_ACL_L4PORT_ACE_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_l4_port_range_get_acl_l4_port_range(&key);

        if(NULL == ptr || 0 == ptr->cnt)
        {
            sal_memset(&acl_l4_port_range, 0, sizeof(tbl_acl_l4_port_range_t));
            key.id = i;
            sal_memcpy(&acl_l4_port_range.key, &key, sizeof(tbl_acl_l4_port_range_key_t));
            acl_l4_port_range.is_src   = is_src;
            acl_l4_port_range.port_min = min;
            acl_l4_port_range.port_max = max;
            acl_l4_port_range.cnt      = 1;
            if(NULL == ptr)
            {
                tbl_acl_l4_port_range_add_acl_l4_port_range(&acl_l4_port_range);
            }
            else
            {
                tbl_acl_l4_port_range_set_acl_l4_port_range_field(&acl_l4_port_range, TBL_ACL_L4_PORT_RANGE_FLD_KEY);
                tbl_acl_l4_port_range_set_acl_l4_port_range_field(&acl_l4_port_range, TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN);
                tbl_acl_l4_port_range_set_acl_l4_port_range_field(&acl_l4_port_range, TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX);
                tbl_acl_l4_port_range_set_acl_l4_port_range_field(&acl_l4_port_range, TBL_ACL_L4_PORT_RANGE_FLD_CNT);
                tbl_acl_l4_port_range_set_acl_l4_port_range_field(&acl_l4_port_range, TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC);
            }
            __l4_port_range__inspect__();
            return FALSE; /* no conflict */
        }
    }

    return TRUE; /* conflicted */
}

void _remove_l4_port_range(uint16 min, uint16 max, uint8 is_src)
{
    uint32 i;
    tbl_acl_l4_port_range_key_t key;
    tbl_acl_l4_port_range_t *ptr;

    for(i = 0;  i < GLB_ACL_L4PORT_ACE_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_l4_port_range_get_acl_l4_port_range(&key);
    
        if(NULL != ptr && ptr->port_min == min && ptr->port_max == max && ptr->is_src == is_src)
        {
            /* detach l4-port-range */
            ptr->cnt -= (0 == ptr->cnt) ? 0 : 1; 
            tbl_acl_l4_port_range_set_acl_l4_port_range_field(ptr, TBL_ACL_L4_PORT_RANGE_FLD_CNT);
            __l4_port_range__inspect__();
            return;
        }
    }
}
#endif


void __udf_group__inspect__(void)
{
#ifdef __DEBUG_ACL_UDF_GROUP_ENABLE__
    uint32 i;
    FILE *fp = fopen("/dev/console", "a+");

    tbl_acl_udf_group_key_t key;
    tbl_acl_udf_group_t *ptr;

    fprintf(fp, "*** enter debug ***\n");
    fprintf(fp, "+----------+----------+----------+----------+----------+\n");
    fprintf(fp, "|    id    |group type|  offset  | l3 proto |   cnt    |\n");
    fprintf(fp, "+----------+----------+----------+----------+----------+\n");

    for(i = 0;  i < GLB_ACL_UDF_GROUP_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);

        if(NULL == ptr)
        {
            fprintf(fp, "|%10d|   NULL   |   NULL   |   NULL   |   NULL   |\n", i);
            fprintf(fp, "+----------+----------+----------+----------+----------+\n");
        }
        else
        {
            fprintf(fp, "|%10d|", ptr->key.id);
            fprintf(fp, "%10d|", ptr->group_type);
            fprintf(fp, "%10d|", ptr->offset);
            fprintf(fp, "%10d|", ptr->l3_protocol);
            fprintf(fp, "%10d|\n", ptr->cnt);
            fprintf(fp, "+----------+----------+----------+----------+----------+\n");
        }
    }

    fclose(fp);

#endif
}

bool _check_udf_group_conflict(uint32 group_type, uint8 ip_version, uint32 l3_proto, uint8 offset)
{
    uint32 i;
    tbl_acl_udf_group_key_t key;
    tbl_acl_udf_group_t *ptr;
    tbl_acl_udf_group_t acl_udf_group;

    if (GLB_ACL_UDF_GROUP_L3_HEAD == group_type)
    {
        key.id = 0;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);
        if (NULL != ptr && 0 != ptr->cnt && 
            (ptr->group_type != group_type || ptr->offset != offset))
        {
            return TRUE; /* conflict only one l3 udf, always group 0 */
        }
    }

    /* merge udf group */
    for(i = 0;  i < GLB_ACL_UDF_GROUP_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);

        if(NULL != ptr && 0 != ptr->cnt && ptr->group_type == group_type && 
            ptr->l3_protocol == l3_proto && ptr->ip_version == ip_version &&
            ptr->offset == offset)
        {
            ptr->cnt++;
            __udf_group__inspect__();
            return FALSE; /* no conflict */
        }
    }

    for (i = 0;  i < GLB_ACL_UDF_GROUP_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);

        if(NULL != ptr && 0 != ptr->cnt && ptr->group_type != group_type)
        {
            return TRUE; /* conflict only one type l3 or l4 head */
        }
        if (NULL != ptr && 0 != ptr->cnt && ptr->group_type == group_type 
            && ptr->l3_protocol == l3_proto && ptr->ip_version == ip_version
            && ptr->offset != offset)
        {
            return TRUE; /*confilct only one protocol support one offset*/
        }
    }

    /* add new udf group */
    for(i = 0;  i < GLB_ACL_UDF_GROUP_NUM_MAX; i++)
    {
        if (GLB_ACL_UDF_GROUP_L3_HEAD == group_type && i > 0)
        {
            continue; 
        }
        key.id = i;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);

        if(NULL == ptr)
        {
            sal_memset(&acl_udf_group, 0, sizeof(tbl_acl_udf_group_t));
            key.id = i;
            sal_memcpy(&acl_udf_group.key, &key, sizeof(tbl_acl_udf_group_key_t));
            acl_udf_group.group_type = group_type;
            acl_udf_group.ip_version = ip_version;
            acl_udf_group.l3_protocol = l3_proto;
            acl_udf_group.offset = offset;
            acl_udf_group.cnt = 1;
            if(NULL == ptr)
            {
                tbl_acl_udf_group_add_acl_udf_group(&acl_udf_group);
            }
            __udf_group__inspect__();
            return FALSE; /* no conflict */
        }
    }

    return TRUE; /* conflicted */
}

void _remove_udf_group(uint32 group_type, uint8 ip_version, uint32 l3_proto, uint8 offset)
{
    uint32 i;
    tbl_acl_udf_group_key_t key;
    tbl_acl_udf_group_t *ptr;

    for(i = 0;  i < GLB_ACL_UDF_GROUP_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_udf_group_get_acl_udf_group(&key);
    
        if(NULL != ptr && ptr->group_type == group_type && ptr->ip_version == ip_version &&
            ptr->l3_protocol == l3_proto && ptr->offset == offset)
        {
            /* detach udf group */
            ptr->cnt -= (0 == ptr->cnt) ? 0 : 1; 
            if (ptr->cnt == 0)
            {
                tbl_acl_udf_group_del_acl_udf_group(&key);
            }
            else
            {
                tbl_acl_udf_group_set_acl_udf_group_field(ptr, TBL_ACL_UDF_GROUP_FLD_CNT);
            }
            __udf_group__inspect__();
            return;
        }
    }
}

int32
_acl_check_udf_field_conflict(tbl_ace_config_t *ace_config, tbl_acl_udf_entry_t *p_db_udf)
{
    int32 rc = PM_E_NONE;

    if (NULL == p_db_udf)
    {
        return PM_E_CFG_CONFLICT;
    }

    if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_ETHTYPE)
     && GLB_FLAG_ISSET(ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        if ((p_db_udf->ethtype != ace_config->ether_type) 
          || (p_db_udf->ethtype_mask != ~(ace_config->ether_type_mask)))
        {
            return PM_E_CFG_CONFLICT;
        }
    }

    if (GLB_FLAG_ISSET(ace_config->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_L4_TYPE) && (0 != ace_config->ip_protocol))
        {
            if ((p_db_udf->l4_type == GLB_ACL_UDF_L4_TYPE_TCP
              && ace_config->ip_protocol != GLB_ACL_L4_PROTOCOL_TCP)
              ||(p_db_udf->l4_type == GLB_ACL_UDF_L4_TYPE_UDP
              && ace_config->ip_protocol != GLB_ACL_L4_PROTOCOL_UDP))
            {
                return PM_E_CFG_CONFLICT;
            }
        }

        if ((GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_IPPROTO))
         && (p_db_udf->ipproto != ace_config->ip_protocol) && (0 != ace_config->ip_protocol))
        {
            return PM_E_CFG_CONFLICT;
        }
    }

    if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_SRCPORT) 
     && GLB_FLAG_ISSET(ace_config->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY != ace_config->l4_src_port_type)
        {
            if (p_db_udf->srcport < ace_config->l4_src_port
             || p_db_udf->srcport > ace_config->l4_src_port_mask)
            {
                return PM_E_CFG_CONFLICT;
            }
        }
    }

    if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_DSTPORT) 
     && GLB_FLAG_ISSET(ace_config->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY != ace_config->l4_dst_port_type)
        {
            if (p_db_udf->dstport < ace_config->l4_dst_port
             || p_db_udf->dstport > ace_config->l4_dst_port_mask)
            {
                return PM_E_CFG_CONFLICT;
            }
        }
    }

    return rc;
}


extern int32 
tap_group_truncation_flow_ace_used_rcs(char *flow_name);

extern int32
tap_group_truncation_flow_update_ace_used_rcs(char *flow_name);
/*
    key format :

    key type,
    value,
    mask.
*/

int32
identification_packet_type(tbl_ace_config_t *ace_config)
{
    int32_t packet_type = GLB_ACL_PACKET_TYPE_UNKNOW;

    if (GLB_ACL_L4_PROTOCOL_UDP == ace_config->ip_protocol)
    {
        if (GLB_ACE_PORT_TYEP_EQ == ace_config->l4_dst_port_type)
        {
            if (GLB_FLAG_ISSET(ace_config->key_flags, GLB_IP_ACE_VXLAN_VNI))
            {
                return GLB_ACL_PACKET_TYPE_VXLAN;
            }
        }
    }
    else if (GLB_ACL_L4_PROTOCOL_GRE == ace_config->ip_protocol)
    { 
        if (GLB_ACL_GRE_GRE == ace_config->l4_gre_type)
        { 
            return GLB_ACL_PACKET_TYPE_GRE;
        }
        else
        {
            return GLB_ACL_PACKET_TYPE_NVGRE;
        }
    }
    else if (GLB_ACL_L4_PROTOCOL_IPIP == ace_config->ip_protocol)
    { 
        return GLB_ACL_PACKET_TYPE_IPIP;
    }

    return packet_type;
}

#ifdef TAPPRODUCT
int32
acl_config_add_ace_tap(char *acl_name, uint32 *p_seq_no, bool deny, char **ace_para, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_ace_config_t *pst_ace_config = NULL;
    tbl_ace_config_t ace_config;
#ifndef GREATBELT 
    tbl_ace_config_master_t *p_master_ace = tbl_ace_config_get_master();
    tbl_ace_config_t *pst_db_ace_config = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 inner_match_num = 0;
#endif    
    uint32 current_ace_count = tbl_ace_config_get_ace_config_count();
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    uint32 current_acl_cfg_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 l4port_num = 0;
    uint32 tcp_flags = 0;
    int32 l4_dest_port = 0;
    /*added by yejl for bug 53475, 2019-09-10*/
    tbl_sys_global_t* p_sys_global = tbl_sys_global_get_sys_global();
    uint32 l2gre_key_num = 0;
    uint32 l2gre_key_len = 0;
    /*ended by yejl*/
    
    int32 rc = 0;
    
    tbl_acl_config_t *pst_flow_match_config = NULL;
    uint8 i = 0;
    uint8 argc_idx = 0;
    tbl_acl_config_key_t flow_match_config_key;
    uint8 argc_idx1 = 0;

    uint32 udf_id = 0;
    uint32 iter   = 0;
    tbl_acl_udf_entry_t *p_db_udf = NULL;
    tbl_acl_udf_entry_t  udf_entry;
    
    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));

    /* get acl_config type, normal acl or flow acl or ipv6 acl or mac acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }
    
    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config_key.name);

    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        /*modified by yej for bug 54155, 2019-10-29*/
        CFG_CONFLICT_RET("%s should be configured first", glb_acl_type_str[type]);
    }

    sal_memset(&ace_config, 0, sizeof(tbl_ace_config_t));
    sal_strcpy(ace_config.key.acl_name, acl_config_key.name);
    
    ace_config.deny = deny;
    pst_ace_config = tbl_ace_config_get_ace_config(&(ace_config.key));
    if (NULL != pst_ace_config)
    {
        return PM_E_EXIST;
    }

    /* src mac */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src_mac");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_MACSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_MAC(ace_config.src_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.src_mac_mask, "0000.0000.0000", para->p_rs);        
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_MAC(ace_config.src_mac_mask, "ffff.ffff.ffff", para->p_rs);
        }
        else
        {
            CFG_PARSER_MAC(ace_config.src_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.src_mac_mask, argv[argc_idx + 3], para->p_rs);
        }
        
        ace_config.src_mac[0] = ace_config.src_mac[0] & (0xff - ace_config.src_mac_mask[0]);
        ace_config.src_mac[1] = ace_config.src_mac[1] & (0xff - ace_config.src_mac_mask[1]);
        ace_config.src_mac[2] = ace_config.src_mac[2] & (0xff - ace_config.src_mac_mask[2]);
        ace_config.src_mac[3] = ace_config.src_mac[3] & (0xff - ace_config.src_mac_mask[3]);
        ace_config.src_mac[4] = ace_config.src_mac[4] & (0xff - ace_config.src_mac_mask[4]);
        ace_config.src_mac[5] = ace_config.src_mac[5] & (0xff - ace_config.src_mac_mask[5]);
    }

    /* dest mac */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dest_mac");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_MACDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_MAC(ace_config.dst_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.dst_mac_mask, "0000.0000.0000", para->p_rs);
        }  
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_MAC(ace_config.dst_mac_mask, "ffff.ffff.ffff", para->p_rs);
        }
        else
        {
            CFG_PARSER_MAC(ace_config.dst_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.dst_mac_mask, argv[argc_idx + 3], para->p_rs);            
        }

        ace_config.dst_mac[0] = ace_config.dst_mac[0] & (0xff - ace_config.dst_mac_mask[0]);
        ace_config.dst_mac[1] = ace_config.dst_mac[1] & (0xff - ace_config.dst_mac_mask[1]);
        ace_config.dst_mac[2] = ace_config.dst_mac[2] & (0xff - ace_config.dst_mac_mask[2]);
        ace_config.dst_mac[3] = ace_config.dst_mac[3] & (0xff - ace_config.dst_mac_mask[3]);
        ace_config.dst_mac[4] = ace_config.dst_mac[4] & (0xff - ace_config.dst_mac_mask[4]);
        ace_config.dst_mac[5] = ace_config.dst_mac[5] & (0xff - ace_config.dst_mac_mask[5]);
    }

    /* vlan */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vlan");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_VLAN;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.svlan, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.svlan_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.svlan_mask &= 0xFFF;
            ace_config.svlan      &= ~(ace_config.svlan_mask);
        }
        else
        {
            ace_config.svlan_mask = 0xfff;
        }
    }

    /* inner vlan */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner_vlan");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_INNNER_VLAN;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.cvlan, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.cvlan_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.cvlan_mask &= 0xFFF;
            ace_config.cvlan      &= ~(ace_config.cvlan_mask);
        }
        else
        {
            ace_config.cvlan_mask = 0xfff;
        }
    }

    /* cos */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("cos");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_COS;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.svlan_cos, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.svlan_cos_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.svlan_cos_mask &= 0x7;
            ace_config.svlan_cos      &= ~(ace_config.svlan_cos_mask);
        }
        else
        {
            ace_config.svlan_cos_mask = 0x7;
        }
    }

    /* inner cos */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner_cos");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_INNER_COS;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.cvlan_cos, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.cvlan_cos_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.cvlan_cos_mask &= 0x7;
            ace_config.cvlan_cos      &= ~(ace_config.cvlan_cos_mask);
        }
        else
        {
            ace_config.cvlan_cos_mask = 0x7;
        }
    }

    /* ether type */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ether_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_ETHTYPE;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.ether_type, argv[argc_idx + 2], para->p_rs);
            if (ace_config.ether_type < 0x600)
            {
                return PM_E_CFG_ERR_ETHER_TYPE;
            }
            PM_CFG_GET_UINT(ace_config.ether_type_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.ether_type_mask &= 0xFFFF;
            ace_config.ether_type     &= ~(ace_config.ether_type_mask);
            
        }
        else
        {
            ace_config.ether_type_mask = 0xffff;
        }
    }
   
    /* source ip address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (GLB_FLAG_ISSET(ace_config.key_ipv6_flags, GLB_IPV6_ACE_IPDA))
        {
            CFG_CONFLICT_RET("Source-ip v4 conflict with dst-ipv6 field");
        }
        ace_config.key_flags |= GLB_IP_ACE_IPSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET;
        ace_config.src_ip_mask.family = AF_INET;
        ace_config.src_ip.u.prefix4.s_addr = ace_config.src_ip.u.prefix4.s_addr & ~(ace_config.src_ip_mask.u.prefix4.s_addr);
    }

    /* source ipv6 address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-ipv6");
    if (argc_idx != 0xFF && GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        CFG_CONFLICT_RET("Can't use ether_type field in ipv6 entry");
    }
    else if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IPDA))
        {
            CFG_CONFLICT_RET("Source-ipv6 conflict with dest-ip v4 field");
        }

        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_IPSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix6, para->p_rs);
            sal_memset(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0x0 ,
                sizeof(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            sal_memset(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0xFF ,
                sizeof(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix6, para->p_rs);
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 3], &ace_config.src_ip_mask.u.prefix6, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET6;
        ace_config.src_ip_mask.family = AF_INET6;
        for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
        {
            ace_config.src_ip.u.prefix6.__in6_u.__u6_addr8[i] &= ~ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
        }
        ace_config.is_ipv6 = TRUE;
    }

    /* dest ip address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dst-ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (GLB_FLAG_ISSET(ace_config.key_ipv6_flags, GLB_IPV6_ACE_IPSA))
        {
            CFG_CONFLICT_RET("Dest-ip v4 conflict with src-ipv6 field");
        }
        ace_config.key_flags |= GLB_IP_ACE_IPDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET;
        ace_config.dst_ip_mask.family = AF_INET;
        ace_config.dst_ip.u.prefix4.s_addr = ace_config.dst_ip.u.prefix4.s_addr & ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
    }

    /* dest ipv6 address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dst-ipv6");
    if (argc_idx != 0xFF && GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        CFG_CONFLICT_RET("Can't use ether_type field in ipv6 entry");
    }
    else if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IPSA))
        {
            CFG_CONFLICT_RET("Dest-ipv6 conflict with source-ip v4 field");
        }
        
        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_IPDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix6, para->p_rs);
            sal_memset(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0x0 ,
                sizeof(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            sal_memset(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0xFF ,
                sizeof(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix6, para->p_rs);
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 3], &ace_config.dst_ip_mask.u.prefix6, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET6;
        ace_config.dst_ip_mask.family = AF_INET6;
        for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
        {
            ace_config.dst_ip.u.prefix6.__in6_u.__u6_addr8[i] &= ~ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
        }
        ace_config.is_ipv6 = TRUE;
    }

    /* flow label */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("flow_lable");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (TRUE != ace_config.is_ipv6)
        {
            CFG_CONFLICT_RET("Flow label only support in ipv6 entry");
        }
        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_FLOW_LABEL;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.ipv6_flow_label, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.ipv6_flow_label_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.ipv6_flow_label_mask &= 0xFFFFF;
            ace_config.ipv6_flow_label      &= ~(ace_config.ipv6_flow_label_mask);
        }
        else
        {
            ace_config.ipv6_flow_label_mask = 0xFFFFF;
        }
    }
    
    /* dscp */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dscp");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        ace_config.key_flags |= GLB_IP_ACE_DSCP;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.dscp, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.dscp_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.dscp_mask &= 0x3f;
            ace_config.dscp     &= ~(ace_config.dscp_mask);
        }
        else
        {
            ace_config.dscp_mask = 0x3f;
        }
    }

    /* ip-precedence */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("precedence");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_DSCP))
        {
            return PM_E_CFG_PROMPT;
        }        
        ace_config.key_flags |= GLB_IP_ACE_PRECEDENCE;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.ip_precedence, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.ip_precedence_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.ip_precedence_mask &= 0x7;
            ace_config.ip_precedence     &= ~(ace_config.ip_precedence_mask);
        }
        else
        {
            ace_config.ip_precedence_mask = 0x7;
        }
    }

    /* ip frag */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("frag");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        ace_config.key_flags |= GLB_IP_ACE_IP_FRAG;
        _acl_cmd_acl_get_ip_frag(argv[argc_idx + 1], &ace_config.ip_frag, para);
    }

    /* l3 protocol */
    /*modifiled by yejl for bug 49831, 2018-11-06*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("l3-protocol");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        ace_config.key_flags |= GLB_IP_ACE_IP_PROTOCOL;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.ip_protocol_mask = 0xff;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "tcp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_TCP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_UDP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "icmp"))
        {
            if (ace_config.is_ipv6)
            {
                ace_config.ip_protocol_mask = 0;
                ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_ICMPv6;  
            }
            else
            {
                ace_config.ip_protocol_mask = 0;
                ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_ICMP;
            }
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "igmp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_IGMP;
            if (ace_config.is_ipv6)
            {
                CFG_CONFLICT_RET("IPv6 entry not support igmp field");
            }
        } 
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gre"))
        {
            if(!check_feature_support(CONFIG_FEATURE_ACL_GRE))
            {
                return PM_E_NOT_SUPPORT;
            }
        
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_GRE;

            ace_config.l4_gre_type = GLB_ACL_GRE_GRE;
        } 
        else if (0 == sal_strcmp(argv[argc_idx + 1], "nvgre"))
        {
            if(!check_feature_support(CONFIG_FEATURE_ACL_NVGRE))
            {
                return PM_E_NOT_SUPPORT;
            }
            
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_GRE;

            ace_config.l4_gre_type = GLB_ACL_GRE_NVGRE;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "ipip"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_IPIP;
            if (ace_config.is_ipv6)
            {
                CFG_CONFLICT_RET("IPv6 entry not support igmp field");
            }
        } 
        else
        {
            PM_CFG_GET_UINT(ace_config.ip_protocol, argv[argc_idx + 2], para->p_rs);
            ace_config.ip_protocol_mask = 0;
        }
    }

    /* l4 src port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-l4-port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_SRCPORT;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_ANY;
            ace_config.l4_src_port_mask = 0xFFFF;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "range"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_RANGE;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 3], para->p_rs);
            if (ace_config.l4_src_port >= ace_config.l4_src_port_mask)
            {
                return PM_E_CFG_DB_FAIL;
            }
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "eq"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_EQ;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 2], para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gt"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_GT;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_GT_MAX < ace_config.l4_src_port)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_src_port++;
            ace_config.l4_src_port_mask = 0xffff;
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "lt"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_LT;
            ace_config.l4_src_port = 0;
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_LT_MIN > ace_config.l4_src_port_mask)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_src_port_mask--;
            l4port_num++;
        }
    }

    /* l4 dest port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dest-l4-port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_DSTPORT;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_ANY;
            ace_config.l4_dst_port_mask = 0xFFFF;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "range"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_RANGE;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 3], para->p_rs);
            if (ace_config.l4_dst_port >= ace_config.l4_dst_port_mask)
            {
                return PM_E_CFG_DB_FAIL;
            }
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "eq"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_EQ;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 2], para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gt"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_GT;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_GT_MAX < ace_config.l4_dst_port)
            {
                return PM_E_INVALID_PARAM;
            }  
            ace_config.l4_dst_port_mask = 0xffff;
            ace_config.l4_dst_port++;
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "lt"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_LT;
            ace_config.l4_dst_port = 0;
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_LT_MIN > ace_config.l4_dst_port_mask)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_dst_port_mask--;
            l4port_num++;
        }
    }

    /* igmp type */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("igmp_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        /*modified by yejl for bug 49821, 2018-11-01*/
        ace_config.key_flags |= GLB_IP_ACE_IGMP_TYPE;
        if (0 == sal_strcmp(argv[argc_idx + 1], "dvmrp"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_DVMRP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "host-query"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_QUERY;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v1host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_REPORT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "mtrace-route"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_MTRACT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "mtrace-response"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_MTRACT_RESPONSE;
        }   
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v2leave-group"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V2_LEAVE;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v2host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V2_REPORT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v3host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V3_REPORT;
        }   
        else
        {
            PM_CFG_GET_UINT(ace_config.igmp_type, argv[argc_idx + 2], para->p_rs);
        }
    }

    /* icmp type& icmp code */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("icmp_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_ICMP_TYPE;
        PM_CFG_GET_UINT(ace_config.icmp_type, argv[argc_idx + 1], para->p_rs);
        if (0 != sal_strlen(argv[argc_idx + 2]))
        {
            ace_config.key_flags |= GLB_IP_ACE_ICMP_CODE;
            PM_CFG_GET_UINT(ace_config.icmp_code, argv[argc_idx + 2], para->p_rs);
        }
    } 

    /* tcp flags */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("tcpflag");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        _acl_cmd_acl_get_tcp_flag(&ace_config.tcp_flags, &ace_config.tcp_flags_mask, &argv[argc_idx]);
        if (0 == ace_config.tcp_flags_mask)
        {
            return PM_E_CFG_ACL_TCP_FLAGS_NONE;
        }
        ace_config.key_flags |= GLB_IP_ACE_TCP_FLAG;
        tcp_flags++;
    }

    /* added by huwx for acl udf 2017-07-19 bug 44643 */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("udf");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]) && 0 != sal_strlen(argv[argc_idx + 2]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "ipv4-head"))
        {
            if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE) &&
                ace_config.ether_type_mask != 0)
            {
                return PM_E_CFG_ERR_UDF_TYPE;
            }
            ace_config.udf_type = GLB_ACL_UDF_GROUP_L3_HEAD;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "l4-head"))
        {
            if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
            {
                return PM_E_CFG_ERR_UDF_TYPE;
            }
            if ((!GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IP_PROTOCOL)) || 
                (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IP_PROTOCOL)&&
                ace_config.ip_protocol_mask != 0))
            {
                return PM_E_CFG_ERR_UDF_VALUE;
            }
            ace_config.udf_type = GLB_ACL_UDF_GROUP_L4_HEAD;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udf-id"))
        {
            ace_config.udf_type = GLB_ACL_UDF_GROUP_UDF_ID;

            PM_CFG_GET_UINT(udf_id, argv[argc_idx + 2], para->p_rs);
            sal_memset(&udf_entry, 0, sizeof(udf_entry));
            udf_entry.key.id = udf_id;
            p_db_udf = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
            if (NULL == p_db_udf)
            {
                CFG_CONFLICT_RET("UDF entry %d not exist", udf_id);
            }

            ace_config.udf_id = udf_id;

            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (0 == sal_strlen(argv[argc_idx+3+iter*3]))
                {
                    continue;
                }
                if (0 == sal_strncmp(argv[argc_idx+3+iter*3], "udf_value", 9))
                {
                    PM_CFG_GET_UINT(ace_config.udf_value[iter], argv[argc_idx+3+iter*3+1], para->p_rs);
                    PM_CFG_GET_UINT(ace_config.udf_mask[iter], argv[argc_idx+3+iter*3+2], para->p_rs);
                }
                else
                {
                    /* any */
                    ace_config.udf_mask[iter] = 0xFFFFFFFF;
                }
                ace_config.udf_value[iter]  &= 0xFFFFFFFF;
                //ace_config.udf_mask[iter]   &= ~(ace_config.udf_mask[iter]);
                ace_config.udf_value_valid[iter] = TRUE;
            }

            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (ace_config.udf_value_valid[iter] != p_db_udf->valid[iter])
                {
                    CFG_CONFLICT_RET("UDF value should be consistent with udf entry offsets");
                }
            }
        }
        else
        {
            return PM_E_CFG_ERR_UDF_TYPE;
        }

        if(!check_feature_support(CONFIG_FEATURE_ACL_UDF))
        {
            return PM_E_NOT_SUPPORT;
        }

        if (ace_config.udf_type != GLB_ACL_UDF_GROUP_UDF_ID)
        {
            PM_CFG_GET_UINT(ace_config.udf_value[0], argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.udf_mask[0], argv[argc_idx + 3], para->p_rs);
            ace_config.udf_mask[0]      &= 0xFFFFFFFF;
            ace_config.udf_value[0]     &= ~(ace_config.udf_mask[0]);
            PM_CFG_GET_UINT(ace_config.udf_offset, argv[argc_idx + 4], para->p_rs);
        }

        if (0 != ace_config.udf_type)
        {
            ace_config.key_flags |= GLB_IP_ACE_UDF;
        }
    }
    
    /*time range*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("timerange");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (sal_strcmp(argv[argc_idx + 1], ""))
        {
            if (PM_E_NONE != timerange_config_check_tr_exist(argv[argc_idx + 1]))
            {
                CFG_CONFLICT_RET("Invalid time-range name %s", argv[argc_idx + 1]);
            }
            sal_strncpy(ace_config.time_range, argv[argc_idx + 1], TIME_RANGE_SIZE);
        }
        ace_config.key_flags |= GLB_IP_ACE_TIME_RANGE;
    }
    
    /* options */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("options");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        ace_config.key_flags |= GLB_IP_ACE_OPTIONS;
        ace_config.options = TRUE;
    }

    /* truncation */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("truncation");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if(!check_feature_support(CONFIG_FEATURE_TAP_TRUNCTION))
        {
            return PM_E_NOT_SUPPORT;
        }
        if (pst_acl_config->tap_ref)
        {
            if (tap_group_truncation_flow_ace_used_rcs(pst_acl_config->key.name) + tbl_tap_group_get_truncation_count() > GLB_TAP_TRUNCATION_MAX)
            {
                return PM_E_EXCEED_TAP_GRP_TRCA_MAX_NUM;
            }
        }
        ace_config.key_flags |= GLB_IP_ACE_TAP_TRUNCATION;
        ace_config.tap_action_trunction = TRUE;
        pst_acl_config->ace_truncation_ref_cnt++;
        tap_group_truncation_flow_update_ace_used_rcs(acl_name);
    }

    /* check edit macda */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-macda");
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        CFG_PARSER_MAC(ace_config.tap_action_edit_dest_mac, argv[argc_idx + 1], para->p_rs);
        ace_config.tap_action_edit_dest_mac_en = TRUE;
    }

    /* check edit macsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-macsa");
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        CFG_PARSER_MAC(ace_config.tap_action_edit_src_mac, argv[argc_idx + 1], para->p_rs);
        ace_config.tap_action_edit_src_mac_en = TRUE;
    }

    /* check edit ipda */   
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipda");
    argc_idx1 = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipv6da");
    if (argc_idx != 0xFF && argc_idx1 != 0xFF)
    {
        CFG_CONFLICT_RET("Don't support both edit-ipda and edit-ipv6da action");
    }
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        cdb_addr_str2val(&ace_config.tap_action_edit_ipda, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        ace_config.tap_action_edit_ipda_en = TRUE;
    }

    /* check edit ipsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipsa");
    argc_idx1 = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipv6sa");
    if (argc_idx != 0xFF && argc_idx1 != 0xFF)
    {
        CFG_CONFLICT_RET("Don't support both edit-ipsa and edit-ipv6sa action");
    }
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_EDIT_SRC_IP))
        {
            return PM_E_NOT_SUPPORT;
        }
        
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        cdb_addr_str2val(&ace_config.tap_action_edit_ipsa, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        ace_config.tap_action_edit_ipsa_en = TRUE;
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipv6da");
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        cdb_addr_str2val(&ace_config.tap_action_edit_ipda, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        ace_config.tap_action_edit_ipda_en = TRUE;
    }

    /* check edit ipsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-ipv6sa");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_EDIT_SRC_IP))
        {
            return PM_E_NOT_SUPPORT;
        }
        
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        cdb_addr_str2val(&ace_config.tap_action_edit_ipsa, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        ace_config.tap_action_edit_ipsa_en = TRUE;
    }

    /* check edit ipsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("edit-vlan");
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        PM_CFG_GET_UINT_RANGE(ace_config.tap_action_edit_vlan,
            GLB_MIN_VLAN_ID,GLB_MAX_VLAN_ID, 
            argv[argc_idx + 1], para->p_rs);
        ace_config.tap_action_edit_vlan_en = TRUE;
		ace_config.tap_action_edit_vlan_action = 0;
    }
	
	argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("add-vlan");
    if(argc_idx != 0xFF){
		if(ace_config.tap_action_edit_vlan_en == TRUE)
		{
			CFG_CONFLICT_RET("Edit-vlan conflict");
		}
		
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
		
        PM_CFG_GET_UINT_RANGE(ace_config.tap_action_edit_vlan,
            GLB_MIN_VLAN_ID,GLB_MAX_VLAN_ID, 
            argv[argc_idx + 1], para->p_rs);
        ace_config.tap_action_edit_vlan_en = TRUE;
		ace_config.tap_action_edit_vlan_action = 1;
    }

    /* check edit ipsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vni");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_VXLAN))
        {
            return PM_E_NOT_SUPPORT;
        }
        ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
        
        if(GLB_ACL_PACKET_TYPE_VXLAN != identification_packet_type(&ace_config)){
            return PM_E_CFG_ACL_VNI_CONFIG_CONFLICT;
        }
        /*added by yejl for bug54032,54049, 2019-10-22*/
        l4_dest_port = acl_ace_get_l4_dest_port(ace_config.l4_dst_port);
        if (l4_dest_port >= 0)
        {
            CFG_CONFLICT_RET("Not support to configure vxlan dst-port, dst-port %d has been configured by other ace", l4_dest_port);
        }
        /*ended*/
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.l4_vxlan_vni_mask, "0xFFFFFFFF", para->p_rs);
        }
        else
        {
            PM_CFG_GET_UINT(ace_config.l4_vxlan_vni, argv[argc_idx + 1], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_vxlan_vni_mask, argv[argc_idx + 2], para->p_rs);
        }

        ace_config.l4_vxlan_vni_mask &= GLB_ACL_VXLAN_VNI_MASK_VALID;
        ace_config.l4_vxlan_vni &= ~(ace_config.l4_vxlan_vni_mask);
        ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
    }

    /* check nvgre-vni */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("nvgre-vni");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_NVGRE))
        {
            return PM_E_NOT_SUPPORT;
        }

        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.l4_gre_key_mask, "0xFFFFFFFF", para->p_rs);
        }
        else
        {
            PM_CFG_GET_UINT(ace_config.l4_gre_key, argv[argc_idx + 1], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_gre_key_mask, argv[argc_idx + 2], para->p_rs);
        }

        ace_config.key_flags |= GLB_IP_ACE_GRE_KEY;

        ace_config.l4_gre_key &= ~(ace_config.l4_gre_key_mask);
#ifndef USW /*modified by yejl for bug 53756, 2019-09-29*/
        ace_config.l4_gre_key <<= GLB_ACL_GRE_NVGRE_KEY_OFFSET; 
        ace_config.l4_gre_key_mask <<= GLB_ACL_GRE_NVGRE_KEY_OFFSET;
#endif 
    }

    /* check gre-key */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("gre-key");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_GRE))
        {
            return PM_E_NOT_SUPPORT;
        }

        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.l4_gre_key_mask, "0xFFFFFFFF", para->p_rs);
        }
        else
        {
            PM_CFG_GET_UINT(ace_config.l4_gre_key, argv[argc_idx + 1], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_gre_key_mask, argv[argc_idx + 2], para->p_rs);
        }

        ace_config.l4_gre_key &= ~(ace_config.l4_gre_key_mask);
        ace_config.key_flags |= GLB_IP_ACE_GRE_KEY;
    }

    /* check mark-source */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("flow-match");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_FLOW_MATCH))
        {
            return PM_E_NOT_SUPPORT;
        }
        sal_memset(&flow_match_config_key,0,sizeof(tbl_acl_config_key_t));
        
        /*added by yejl for bug 54091, 2019-10-23*/
        if (!GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_VXLAN_VNI))
        {
            if ((GLB_ACL_L4_PROTOCOL_UDP == ace_config.ip_protocol) &&
                (GLB_ACE_PORT_TYEP_EQ == ace_config.l4_dst_port_type))
            {
                if (GLB_ACL_UDP_VXLAN_L4_DEST_PORT == ace_config.l4_dst_port)
                {
                    l4_dest_port = acl_ace_get_l4_dest_port(ace_config.l4_dst_port);
                    if (l4_dest_port >= 0)
                    {
                        CFG_CONFLICT_RET("Not support to configure the dst-port, vxlan dest-port %d has been configured by other ace", l4_dest_port);
                    }
                    ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
                    PM_CFG_GET_UINT(ace_config.l4_vxlan_vni_mask, "0xFFFFFFFF", para->p_rs);
                    ace_config.l4_vxlan_vni_mask &= GLB_ACL_VXLAN_VNI_MASK_VALID;
                    ace_config.l4_vxlan_vni &= ~(ace_config.l4_vxlan_vni_mask);
                    ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Unknow packet type, only gre/nvgre/vxlan(dst-port is 4789) support inner-match");
                }
            }
        }
        /*ended*/
        if(GLB_ACL_PACKET_TYPE_UNKNOW == identification_packet_type(&ace_config))
        {
            CFG_CONFLICT_RET("Unknow packet type, only gre/nvgre/vxlan support inner-match");
        }
        
        acl_generate_acl_config_key_name_by_type(
            argv[argc_idx + 1],
            GLB_ACL_CONFIG_TYPE_FLOW_MATCH,
            flow_match_config_key.name);

        pst_flow_match_config = tbl_acl_config_get_acl_config(&flow_match_config_key);
        if (NULL == pst_flow_match_config)
        {
            CFG_CONFLICT_RET("Inner-match %s not exist", argv[argc_idx + 1]);
            return PM_E_NOT_EXIST;
        }

#ifndef GREATBELT
        CTCLIB_SLIST_LOOP(p_master_ace->ace_config_list, pst_db_ace_config, listnode)
        {
            if (GLB_FLAG_ISSET(pst_db_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
            {
                inner_match_num++;
            }
        }
        if (inner_match_num >= GLB_ACE_INNER_MATCH_NUM_MAX)
        {
            CFG_CONFLICT_RET("The max number of ace configured with inner-match is %d", GLB_ACE_INNER_MATCH_NUM_MAX);
        }
#endif

        sal_strcpy(ace_config.flow_match_name, flow_match_config_key.name);
        ace_config.key_flags |= GLB_IP_ACE_TAP_FLOW_MATCH;
    }

    /* check edit ipsa */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("strip-header");
    if(argc_idx != 0xFF){
        if(!check_feature_support(CONFIG_FEATURE_ACL_STRIP_HEADER))
        {
            return PM_E_NOT_SUPPORT;
        }
        
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        /*added by yejl for bug54032,54049, 2019-10-22*/
        if (!GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_VXLAN_VNI))
        {
            if ((GLB_ACL_L4_PROTOCOL_UDP == ace_config.ip_protocol) &&
                (GLB_ACE_PORT_TYEP_EQ == ace_config.l4_dst_port_type))
            {
                if (GLB_ACL_UDP_VXLAN_L4_DEST_PORT == ace_config.l4_dst_port)
                {
                    l4_dest_port = acl_ace_get_l4_dest_port(ace_config.l4_dst_port);
                    if (l4_dest_port >= 0)
                    {
                        CFG_CONFLICT_RET("Not support to configure the dst-port, vxlan dest-port %d has been configured by other ace", l4_dest_port);
                    }
                    ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
                    PM_CFG_GET_UINT(ace_config.l4_vxlan_vni_mask, "0xFFFFFFFF", para->p_rs);
                    ace_config.l4_vxlan_vni_mask &= GLB_ACL_VXLAN_VNI_MASK_VALID;
                    ace_config.l4_vxlan_vni &= ~(ace_config.l4_vxlan_vni_mask);
                    ace_config.key_flags |= GLB_IP_ACE_VXLAN_VNI;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Unknow packet type, only gre/nvgre/vxlan(dst-port is 4789) support strip-header");
                }
            }
        }
        /*ended*/

        PM_CFG_GET_UINT(ace_config.action_strip_header, argv[argc_idx + 1], para->p_rs);
        if(ace_config.action_strip_header)
        {
            if(GLB_ACL_PACKET_TYPE_UNKNOW == identification_packet_type(&ace_config)){
                return PM_E_CFG_ACL_STRIP_HEADER_INVALID_PARAM;
            }
        }

        argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("strip-position");
        if(argc_idx != 0xFF){
            if(0 == sal_strcmp("l2", argv[argc_idx + 1])){
                ace_config.strip_header_pos = GLB_ACL_STRIP_HEADER_POS_L2;
            }else if(0 == sal_strcmp("l3", argv[argc_idx + 1])){
                ace_config.strip_header_pos = GLB_ACL_STRIP_HEADER_POS_L3;
            }else{
                ace_config.strip_header_pos = GLB_ACL_STRIP_HEADER_POS_L4;
            }

            ace_config.key_flags2 |= GLB_IP_ACE_STRIP_HEADER_POS;
        }

        argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("strip-offset");
        if(argc_idx != 0xFF){
            PM_CFG_GET_UINT(ace_config.strip_header_offset, argv[argc_idx + 1], para->p_rs);

            if(ace_config.strip_header_offset % 2){
                CFG_CONFLICT_RET("Strip-offset must be a multiple of 2");
                return PM_E_INVALID_PARAM;
            }

            ace_config.key_flags2 |= GLB_IP_ACE_STRIP_HEADER_OFFSET;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("strip-inner-vxlan-header");
    if(argc_idx != 0xFF){
	ace_config.key_flags2 |= GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER;
    }
	
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("erspan-id");
    if(argc_idx != 0xFF)
    {
        if(!check_feature_support(CONFIG_FEATURE_ACL_UDF))
        {
            return PM_E_NOT_SUPPORT;
        }

        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF))
        {
            return PM_E_CFG_ERR_UDF_TYPE;
        }

        if(!ace_config.action_strip_header && !deny)
        {
            CFG_CONFLICT_RET("Erspan must config strip-header");
        }
        /*added by yejl for bug 53672, 2019-10-22*/
#ifdef GOLDENGATE 
        uint32 erspan_ace_num = 0;
        CTCLIB_SLIST_LOOP(p_master_ace->ace_config_list, pst_db_ace_config, listnode)
        {
            if (GLB_FLAG_ISSET(pst_db_ace_config->key_flags2, GLB_IP_ACE_ERSPAN_ID))
            {
                erspan_ace_num++;
            }
        }
        if (erspan_ace_num >= GLB_ACE_ERSPAN_NUM_MAX)
        {
            CFG_CONFLICT_RET("The max number of ace configured with erspan is %d", GLB_ACE_ERSPAN_NUM_MAX);
        }
#endif
        /*end*/
        if ((!GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IP_PROTOCOL)) || 
         (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IP_PROTOCOL)&&
          ace_config.ip_protocol_mask != 0))
        {
            return PM_E_CFG_ERR_UDF_VALUE;
        }
        
        ace_config.key_flags2 		|= GLB_IP_ACE_ERSPAN_ID;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.erspan_id, argv[argc_idx + 1], para->p_rs);
        	PM_CFG_GET_UINT(ace_config.erspan_id_mask, argv[argc_idx + 2], para->p_rs);
        	ace_config.erspan_id_mask 	&= 0x3FF;
            ace_config.erspan_id      	&= ~(ace_config.erspan_id_mask);
        }
        else
        {
            ace_config.erspan_id_mask = 0x3FF;
        }

        ace_config.udf_type        = GLB_ACL_UDF_GROUP_L4_HEAD;
        if(0x3FF == ace_config.erspan_id_mask)
        {
        	ace_config.udf_mask[0]        = 0xFFFFFFFF;
        }
        else
        {
            ace_config.udf_mask[0]        = ace_config.erspan_id_mask;
        }
        ace_config.udf_value[0]       = ace_config.erspan_id;
        ace_config.udf_offset         = 2 | 0x80;
        ace_config.key_flags       |= GLB_IP_ACE_UDF;

	    //ace_config.action_strip_header = TRUE;
    }

    /* check un-tag */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("un-tag");
    if(argc_idx != 0xFF && (0 != sal_strcmp(argv[argc_idx + 1], "false"))){      
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        ace_config.key_flags |= GLB_IP_ACE_TAP_UNTAG;
        if(!sal_strcmp("s-vlan", argv[argc_idx + 1]))
        {
            ace_config.tap_action_untag = UNTAG_MODE_SVLAN;
        }
        else if(!sal_strcmp("c-vlan", argv[argc_idx + 1]))
        {
            ace_config.tap_action_untag = UNTAG_MODE_CVLAN;
        }
        else if(!sal_strcmp("true", argv[argc_idx + 1]))
        {   
            ace_config.tap_action_untag = UNTAG_MODE_AUTO;
        }
    }

    /* check mark-source */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("mark-source");
    if(argc_idx != 0xFF){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION)){
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }

        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_UNTAG)){
            return PM_E_CFG_ACL_MAKR_SOURCE_UNTAG_CONFIG_CONFLICT;
        }
        
        PM_CFG_GET_UINT(ace_config.tap_action_mark_vlan, argv[argc_idx + 1], para->p_rs);
        ace_config.key_flags |= GLB_IP_ACE_TAP_MARK_SOURCE;
    }

    if((GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))){
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TCP_FLAG)){
            CFG_CONFLICT_RET("Not support config tcp flag and inner-match together");
        }

        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF)){
            CFG_CONFLICT_RET("Not support config udf and inner-match together");
        }         
    }

    /*added by yejl for bug 53475, 2019-09-10*/
    /* check add-l2gre */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("add-l2gre");
    if(argc_idx != 0xFF)
    {
        /*added by yejl for tap bug 53519, 2019-09-17*/
        if (p_sys_global->add_gre_ace_entry_num >= GLB_ADD_GRE_HEADER_ENTRY_MAX_NUM)
        {
            CFG_CONFLICT_RET("The max add-l2gre or add-l3gre entry is 4");
        }
        /*ended by yejl*/
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION))
        {
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        if (ace_config.action_strip_header)
        {
            CFG_CONFLICT_RET("Don't surpport both strip-header and add-l2gre action");
        }
        ace_config.l2gre_add_header = TRUE;        
        cdb_addr_str2val(&ace_config.l2gre_src_ip, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        cdb_addr_str2val(&ace_config.l2gre_dst_ip, argv[argc_idx + 2], IPADDR_BUF_SIZE);
        rc = cdb_mac_str2val(ace_config.l2gre_dst_mac, argv[argc_idx + 3]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid dest mac address");
        }
        PM_CFG_GET_UINT(l2gre_key_num, argv[argc_idx + 4], para->p_rs);
        PM_CFG_GET_UINT(l2gre_key_len, argv[argc_idx + 5], para->p_rs);
        if (l2gre_key_len == GLB_L2GRE_KEY_LEN_16)
        {
            if (l2gre_key_num > GLB_L2GRE_KEY_NUM_16_MAX)
            {
                CFG_INVALID_PARAM_RET("The l2gre key's range should be 0-65535 when key length is 16");
            }
        }
        else if (l2gre_key_len == GLB_L2GRE_KEY_LEN_20)
        {
            if (l2gre_key_num > GLB_L2GRE_KEY_NUM_20_MAX)
            {
                CFG_INVALID_PARAM_RET("The l2gre key's range should be 0-1048575 when key length is 20");
            }
        }
        else if (l2gre_key_len == GLB_L2GRE_KEY_LEN_24)
        {
            if (l2gre_key_num > GLB_L2GRE_KEY_NUM_24_MAX)
            {
                CFG_INVALID_PARAM_RET("The l2gre key's range should be 0-16777215 when key length is 24");
            }
        }
        else if (l2gre_key_len == GLB_L2GRE_KEY_LEN_32)
        {
            if (l2gre_key_num > GLB_L2GRE_KEY_NUM_32_MAX)
            {
                CFG_INVALID_PARAM_RET("The l2gre key's range should be 0-4294967295 when key length is 32");
            }
        }
        ace_config.l2gre_key_num = l2gre_key_num;
        ace_config.l2gre_key_len = l2gre_key_len;
        ace_config.key_flags2 |= GLB_IP_ACE_ADD_L2GRE_KEY;
    }

    /* check add-l3gre */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("add-l3gre");
    if(argc_idx != 0xFF)
    {
        /*added by yejl for tap bug 53519, 2019-09-17*/
        if (p_sys_global->add_gre_ace_entry_num >= GLB_ADD_GRE_HEADER_ENTRY_MAX_NUM)
        {
            CFG_CONFLICT_RET("The max add-l2gre or add-l3gre entry is 4");
        }
        /*ended by yejl*/
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION))
        {
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        if (ace_config.action_strip_header)
        {
            CFG_CONFLICT_RET("Don't surpport both strip-header and add-l3gre action");
        }
        if (ace_config.l2gre_add_header)
        {
            CFG_CONFLICT_RET("Don't surpport both add-l2gre and add-l3gre action");
        }
        ace_config.l3gre_add_header = TRUE;
        cdb_addr_str2val(&ace_config.l3gre_src_ip, argv[argc_idx + 1], IPADDR_BUF_SIZE);
        cdb_addr_str2val(&ace_config.l3gre_dst_ip, argv[argc_idx + 2], IPADDR_BUF_SIZE);
        rc = cdb_mac_str2val(ace_config.l3gre_dst_mac, argv[argc_idx + 3]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid dest mac address");
        }
    }
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("add-vxlan");
     if(argc_idx != 0xFF)
    {   
        if (p_sys_global->add_vxlan_entry_num >= GLB_ADD_VXLAN_HEADER_ENTRY_MAX_NUM)
        {
            CFG_CONFLICT_RET("The max add-vxlan entry is 4");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_TRUNCATION))
        {
            return PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT;
        }
        if (ace_config.action_strip_header)
        {
            CFG_CONFLICT_RET("Don't surpport both strip-header and add-vxlan action");
        }
        if (ace_config.l2gre_add_header)
        {
            CFG_CONFLICT_RET("Don't surpport both add-l2gre and add-vxlan action");
        }
        if(ace_config.l3gre_add_header)
        {
            CFG_CONFLICT_RET("Don't surpport both add-l3gre and add-vxlan action");
        }
        ace_config.add_vxlan_header = TRUE;
        rc = cdb_mac_str2val(ace_config.vxlan_dst_mac, argv[argc_idx + 2]);
        rc = cdb_addr_str2val(&ace_config.vxlan_src_ip, argv[argc_idx + 4], IPADDR_BUF_SIZE);
        rc = cdb_addr_str2val(&ace_config.vxlan_dst_ip, argv[argc_idx + 6], IPADDR_BUF_SIZE);
        ace_config.vxlan_srcport = sal_atoi(argv[argc_idx + 8]);
        ace_config.vxlan_destport = sal_atoi(argv[argc_idx + 10]);
        ace_config.vxlan_vni = sal_atoi(argv[argc_idx + 12]);
    }
     /*ended by guoxd*/
    
    if (ace_config.action_strip_header && (!GLB_FLAG_ISSET(ace_config.key_flags2, GLB_IP_ACE_ERSPAN_ID)))
    {
        /* GRE packet check, not support */
        if(GLB_ACL_PACKET_TYPE_GRE == identification_packet_type(&ace_config))
        {
            if (!ace_config.tap_action_edit_src_mac_en 
               || !ace_config.tap_action_edit_dest_mac_en)
            {
                CFG_CONFLICT_RET("Strip-header with gre must use edit-macda,edit-macsa to add new L2 header!!!");
            }

            if (ace_config.tap_action_edit_ipsa_en 
               || ace_config.tap_action_edit_ipda_en)
            {
                CFG_CONFLICT_RET("Strip-header with gre not support edit-ipda,edit-ipsa");
            }

            if (ace_config.tap_action_edit_vlan_en 
               && ace_config.tap_action_edit_vlan_action == 0 /* edit-vlan */)
            {
                CFG_CONFLICT_RET("Strip-header with gre not support edit-vlan");
            }
            /*added by yejl for bug 53475, 2019-09-10*/
            if (ace_config.l2gre_add_header)/* add-l2gre */
            {
                CFG_CONFLICT_RET("Strip-header with gre not support add-l2gre");
            }
            if (ace_config.l3gre_add_header)/* add-l3gre */
            {
                CFG_CONFLICT_RET("Strip-header with gre not support add-l3gre");
            }
            /*ended by yejl*/
            
            /*added by guoxd for vxlan*/
            if (ace_config.add_vxlan_header)
            {
                CFG_CONFLICT_RET("Strip-header with gre not support vxlan");
            }
            /*ended by guoxd*/

            if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_MARK_SOURCE))
            {
                CFG_CONFLICT_RET("Strip-header with gre not support mark-source, (use edit-vlan instead)");
            }

        }
        
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_UNTAG))
        {
            CFG_CONFLICT_RET("Strip-header not support untag");
        }
        if ((GLB_CHIP_DUET2 == tbl_chip_get_type() || GLB_CHIP_TSINGMA == tbl_chip_get_type()) 
         && GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_MARK_SOURCE))
        {
            CFG_CONFLICT_RET("Strip-header not support mark source");
        }
    }

    /*added by yejl for bug 53475, 2019-09-10*/
    {
        if (ace_config.tap_action_edit_src_mac_en)
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support edit-macsa");
        }
        if (ace_config.tap_action_edit_dest_mac_en)
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support edit-macda");
        }
        if (ace_config.tap_action_edit_ipsa_en)
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support edit-ipsa, edit-ipv6sa");
        }
        if (ace_config.tap_action_edit_ipda_en)
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support edit-ipda, edit-ipv6da");
        }
        if (ace_config.tap_action_edit_vlan_en)
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support edit-vlan or add-vlan");
        }

        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_MARK_SOURCE))
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support mark-source");
        }
        
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_UNTAG))
        {
            CFG_CONFLICT_RET("Add-l2gre or add-l3gre don't support untag");
        }
    }
    /*ended by yejl*/
    
    /*added by guoxd for vxlan*/
    if (ace_config.add_vxlan_header)
    {
        if (ace_config.tap_action_edit_src_mac_en)
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support edit-macsa");
        }
        if (ace_config.tap_action_edit_dest_mac_en)
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support edit-macda");
        }
        if (ace_config.tap_action_edit_ipsa_en)
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support edit-ipsa, edit-ipv6sa");
        }
        if (ace_config.tap_action_edit_ipda_en)
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support edit-ipda, edit-ipv6da");
        }
        if (ace_config.tap_action_edit_vlan_en)
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support edit-vlan or add-vlan");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_MARK_SOURCE))
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support mark-source");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_UNTAG))
        {
            CFG_CONFLICT_RET("Add-vxlan doesn't support untag");
        }
    }
    /*ended by guoxd*/
    
    /* check metadata and udf */
    if (pst_acl_config->decap_en) {
        /* check udf config */
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF))
        {
            CFG_CONFLICT_RET("Can't use udf in flow-decap mode");
            return PM_E_NOT_EXIST;
        }
    }
    else
    {
        /* check udf config */
        if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_FLOW_MATCH)){
            CFG_CONFLICT_RET("Can't use inner-match in flow-udf(default) mode");
            return PM_E_NOT_EXIST;
        }
    }

    /* auto fix chip acl field conflict */
    if(check_feature_support(CONFIG_FEATURE_ACL_VXLAN))
    {   
        if(GLB_ACL_PACKET_TYPE_VXLAN == identification_packet_type(&ace_config)){
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_ANY;
            ace_config.l4_src_port      = 0;
            ace_config.l4_src_port_mask = 0xFFFF;
        }
    }

    /* snmp_set */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("snmp_set");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.tap_snmp_set = TRUE;
    }

    /* arp */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("arp");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (0 != (ace_config.key_flags & ~GLB_MAC_ACE_MACDA
                & ~GLB_MAC_ACE_MACSA & ~GLB_IP_ACE_TIME_RANGE 
                & ~GLB_IP_ACE_INVALID & ~GLB_IP_ACE_TIMER_ACTIVE
                & ~GLB_MAC_ACE_VLAN & ~GLB_MAC_ACE_INNNER_VLAN
                & ~GLB_IP_ACE_COS & ~GLB_IP_ACE_INNER_COS)
            || 0 != ace_config.key_ipv6_flags)
        {
            CFG_CONFLICT_RET("Arp entry only support sender-ip, target-ip, src-mac, dst-mac, vlan, inner-vlan, cos, inner-cos fields");
        }
        ace_config.key_arp_flags |= GLB_ARP_ACE_OP_CODE;

        if (0 == sal_strcmp("request", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_REQUEST;
            ace_config.arp_op_code_mask = 0;

        }
        else if (0 == sal_strcmp("response", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_RESPONSE;
            ace_config.arp_op_code_mask = 0;
        }
        else if (0 == sal_strcmp("any", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_NULL;
            ace_config.arp_op_code_mask = 0xFFFF;
        }
        /*added by yejl for bug 49654, 2018-10-24*/
        else if (0 == sal_strcmp("rarp-request", argv[argc_idx + 1]))
        {
            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }
            ace_config.arp_op_code = ARP_TYPE_RARP_REQUEST;
            ace_config.arp_op_code_mask = 0;
        }
        else if (0 == sal_strcmp("rarp-response", argv[argc_idx + 1]))
        {
            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }
            ace_config.arp_op_code = ARP_TYPE_RARP_RESPONSE;
            ace_config.arp_op_code_mask = 0;
        }
    }
    
    /*arp sender ip conflict with src-ip, use same db to save*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("sender_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_arp_flags |= GLB_ARP_ACE_SENDER_IP;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET;
        ace_config.src_ip_mask.family = AF_INET;
        ace_config.src_ip.u.prefix4.s_addr = ace_config.src_ip.u.prefix4.s_addr & ~(ace_config.src_ip_mask.u.prefix4.s_addr);
    }

    /*arp target ip conflict with dst-ip, use same db to save*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("target_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_arp_flags |= GLB_ARP_ACE_TARGET_IP;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET;
        ace_config.dst_ip_mask.family = AF_INET;
        ace_config.dst_ip.u.prefix4.s_addr = ace_config.dst_ip.u.prefix4.s_addr & ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
    }

    /*ipv6 entry field check*/
    if (ace_config.is_ipv6)
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IGMP_TYPE))
        {
            CFG_CONFLICT_RET("Can't use igmp field in ipv6 entry");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Can't use ether_type field in ipv6 entry");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_OPTIONS)
            && GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_IP_FRAG))
        {
            CFG_CONFLICT_RET("Only support one field option or fragment in ipv6 entry");
        }
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF))
        {
            CFG_CONFLICT_RET("Can't configure udf field in ipv6 entry");
        }
    }
    else
    {
        if (GLB_FLAG_ISSET(ace_config.key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
        {
            CFG_CONFLICT_RET("Can't use flow label field in ipv4 entry");
        }
    }

    if ((1 + current_ace_count) > GLB_ACE_CONFIG_MAX)
    {
        CFG_INVALID_PARAM_RET("The max flow entry number is %d", GLB_ACE_CONFIG_MAX); 
    }

    //tbl_acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, &current_acl_entry_count_egress, &current_acl_entry_count_wromfilter);

    if(__tap_acl_config_res_check(type, &ace_config, pst_acl_config, pst_flow_match_config))
	{
        return PM_E_CFG_NO_RESOURCE;
    }

    if (((pst_acl_config->intf_ref_in + pst_acl_config->intf_ref_out) * tcp_flags + current_acl_cfg_tcp_flags_count) > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }

    if (pst_acl_config->tcp_flags_ref + tcp_flags > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        if (((ace_config.key_flags & GLB_MAC_ACE_ETHTYPE) == GLB_MAC_ACE_ETHTYPE) 
            && pst_acl_config->intf_ref_out)
        {
            return PM_E_CFG_PM_ERROR;
        }
    }

    if (ace_entry_config_check(&ace_config))
    {
        return PM_E_INIT_FAIL;
    }
#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
    /* check l4 port resource */
    if (ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_RANGE ||
        ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_GT    ||
        ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_LT)
    {
        if(TRUE == _check_l4_port_range_conflict(ace_config.l4_src_port, ace_config.l4_src_port_mask, 1))
        {
            return PM_E_RESOURCE_FULL;
        }
    }
    if (ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_RANGE ||
        ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_GT    ||
        ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_LT)
    {
        if(TRUE == _check_l4_port_range_conflict(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0))
        {
            return PM_E_RESOURCE_FULL;
        }
    }
#endif

    /* added by huwx for acl udf 2017-07-19 bug 44643 */
    if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF))
    {
        if (ace_config.udf_type != GLB_ACL_UDF_GROUP_UDF_ID)
        {
            if (TRUE == _check_udf_group_conflict(ace_config.udf_type, 
                GLB_ACL_UDF_GROUP_L3_HEAD == ace_config.udf_type ? 0 : GLB_ACL_UDF_VERSION_V4,
                GLB_ACL_UDF_GROUP_L3_HEAD == ace_config.udf_type ? 0 : ace_config.ip_protocol,
                ace_config.udf_offset))
            {
                return PM_E_CFG_ERR_UDF_NO_RESPURCE;
            }
        }
        else
        {
            /* check acl and udf key field consistency */
            if (_acl_check_udf_field_conflict(&ace_config, p_db_udf) < 0)
            {
                CFG_CONFLICT_RET("The acl field is conflict with udf match field"); 
            }
        }
    }

    if (0 == *p_seq_no)
    {
        if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == pst_acl_config->type_identifying)
        {
            if ((pst_acl_config->seq_ref + 1) > ACL_MAX_FLOW_MATCH_ACE_SEQUENCE_NUMBER)
            {
                return PM_E_NOT_PERMIT;
            }
            else
            {
                ace_config.key.seq_no = pst_acl_config->seq_ref + 1;
            }
            *p_seq_no = ace_config.key.seq_no;
            pst_acl_config->seq_ref += 1; 
        }
        else
        {
            if ((pst_acl_config->seq_ref + 10) > ACL_MAX_ACE_SEQUENCE_NUMBER)
            {
                return PM_E_NOT_PERMIT;
            }
            else
            {
                ace_config.key.seq_no = pst_acl_config->seq_ref + 10;
            }
            *p_seq_no = ace_config.key.seq_no;
            pst_acl_config->seq_ref += 10; 
        }
    }
    else
    {
        ace_config.key.seq_no = *p_seq_no;
        if (*p_seq_no > pst_acl_config->seq_ref)
        {
            pst_acl_config->seq_ref = ace_config.key.seq_no;
        }
    }
    rc = tbl_ace_config_add_ace_config(&ace_config); 
    if (PM_E_NONE != rc)
    {
#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
#endif
        return rc;
    }

    pst_ace_config = tbl_ace_config_get_ace_config(&(ace_config.key));
    if (NULL == pst_ace_config)
    {
#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
#endif
        return PM_E_NOT_EXIST;
    }

    if (ace_config.is_ipv6)
    {
        pst_acl_config->acev6_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEV6_REF);
    }
    else
    {
        pst_acl_config->ace_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACE_REF);
    }
    
    if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
    {
        pst_flow_match_config->intf_ref_in++;
        tbl_acl_config_set_acl_config_field(pst_flow_match_config, TBL_ACL_CONFIG_FLD_INTF_REF_IN);
    }
    
    
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_SEQ_REF);
    
    pst_acl_config->l4_port_ref += l4port_num;
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_L4_PORT_REF);

    pst_acl_config->tcp_flags_ref += tcp_flags;
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF);

    
    if ((ace_config.key_flags & GLB_MAC_ACE_ETHTYPE) == GLB_MAC_ACE_ETHTYPE)
    {
        pst_acl_config->ether_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ETHER_REF);
    }
    
    if (GLB_FLAG_ISSET(ace_config.key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        pst_acl_config->arp_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ARP_REF);
    }

    if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        _acl_time_range_update_refcnt(ace_config.time_range, TRUE);
    }

    if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_UDF))
    {
        if (ace_config.udf_type == GLB_ACL_UDF_GROUP_UDF_ID)
        {
            p_db_udf->acl_ref_cnt++;
            tbl_acl_udf_entry_set_acl_udf_entry_field(p_db_udf, TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT);
        }
    }
    
    rc = acl_entry_add_ace(pst_ace_config);
    if(PM_E_NONE != rc)
    {
#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
#endif
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TIME_RANGE))
        {
            _acl_time_range_update_refcnt(ace_config.time_range, FALSE);
        }
        return rc;
    }
    /*added by yejl for tap bug 53519, 2019-09-17*/
    if (pst_ace_config->l2gre_add_header || pst_ace_config->l3gre_add_header)
    {
        p_sys_global->add_gre_ace_entry_num ++;
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM);
    }
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    if (pst_ace_config->add_vxlan_header)
    {
        p_sys_global->add_vxlan_entry_num ++;
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM);
    }
    /*ended by guoxd*/
    
    uint32 is_decap = FALSE;
    if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == pst_acl_config->type_identifying)
    {
        foreach_ace_config_start(ace_node)
        {
            if (ace_node->flow_match_name ==0 || 
                0 != sal_strcmp(ace_node->flow_match_name, pst_ace_config->key.acl_name))
            {
                continue;
            }

            tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_ADD_ACE, ace_node, pst_ace_config, is_decap);
        }foreach_ace_config_end
    }
    else if(GLB_ACL_CONFIG_TYPE_FLOW == pst_acl_config->type_identifying)
    {
        if(GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
        {
            foreach_ace_config_start(ace_node)
            {
                if (0 != sal_strcmp(ace_node->key.acl_name, ace_config.flow_match_name))
                {
                    continue;
                }

                tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_ADD_ACE, pst_ace_config, ace_node, is_decap);
            }foreach_ace_config_end
        }
        else
        {
            is_decap = pst_acl_config->decap_en;
            tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_ADD_ACE, pst_ace_config, NULL, is_decap);
        }
    }
    return acl_policy_operation(NULL, 0, NULL, NULL, ace_config.key.acl_name, ace_config.key.seq_no, GLB_ACL_POLICY_OP_ACL_ACE, GLB_ACL_POLICY_OP_ACTION_ADD);
}
#else

int32
acl_config_add_ace(char *acl_name, uint32 *p_seq_no, bool deny, char **ace_para, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_ace_config_t *pst_ace_config = NULL;
    tbl_ace_config_t ace_config;
    uint32 current_ace_count = tbl_ace_config_get_ace_config_count();
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    uint32 current_acl_cfg_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 ethertype_or_arp_flag = FALSE;
    uint32 l4port_num = 0;
    uint32 tcp_flags = 0;
    uint32 ipv4_mask_len = 0;
    uint32 ipv6_mask_len = 0;
    uint8 i = 0;
    uint8 argc_idx = 0;
    int32 rc = 0;
    tbl_iter_args_t args;
    
    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));

    /* get acl_config type, normal acl or flow acl or ipv6 acl or mac acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }

    /*added by yejl for ipv6 acl*/
    if ((FALSE == acl_stm_is_ipv6_profile())&& 
        (type == GLB_ACL_CONFIG_TYPE_IPV6))
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    /*ended by yejl*/
    
    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config_key.name);

    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        /*modified by yej for bug 54155, 2019-10-29*/
        CFG_CONFLICT_RET("%s should be configured first", glb_acl_type_str[type]);
    }

    sal_memset(&ace_config, 0, sizeof(tbl_ace_config_t));
    sal_strcpy(ace_config.key.acl_name, acl_config_key.name);
    
    ace_config.deny = deny;
    pst_ace_config = tbl_ace_config_get_ace_config(&(ace_config.key));
    if (NULL != pst_ace_config)
    {
        return PM_E_EXIST;
    }

    /*added by yejl for IP/IPv6/MAC acl*/
    ace_config.acl_type = type;
    if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
    {
        ace_config.is_ipv6 = TRUE;
    }
    /*ended*/

    /* src mac */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src_mac");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_MACSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_MAC(ace_config.src_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.src_mac_mask, "0000.0000.0000", para->p_rs);        
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_MAC(ace_config.src_mac_mask, "ffff.ffff.ffff", para->p_rs);
        }
        else
        {
            CFG_PARSER_MAC(ace_config.src_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.src_mac_mask, argv[argc_idx + 3], para->p_rs);
        }
        
        ace_config.src_mac[0] = ace_config.src_mac[0] & (0xff - ace_config.src_mac_mask[0]);
        ace_config.src_mac[1] = ace_config.src_mac[1] & (0xff - ace_config.src_mac_mask[1]);
        ace_config.src_mac[2] = ace_config.src_mac[2] & (0xff - ace_config.src_mac_mask[2]);
        ace_config.src_mac[3] = ace_config.src_mac[3] & (0xff - ace_config.src_mac_mask[3]);
        ace_config.src_mac[4] = ace_config.src_mac[4] & (0xff - ace_config.src_mac_mask[4]);
        ace_config.src_mac[5] = ace_config.src_mac[5] & (0xff - ace_config.src_mac_mask[5]);
    }

    /* dest mac */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dest_mac");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_MACDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_MAC(ace_config.dst_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.dst_mac_mask, "0000.0000.0000", para->p_rs);
        }  
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_MAC(ace_config.dst_mac_mask, "ffff.ffff.ffff", para->p_rs);
        }
        else
        {
            CFG_PARSER_MAC(ace_config.dst_mac, argv[argc_idx + 2], para->p_rs);
            CFG_PARSER_MAC(ace_config.dst_mac_mask, argv[argc_idx + 3], para->p_rs);            
        }

        ace_config.dst_mac[0] = ace_config.dst_mac[0] & (0xff - ace_config.dst_mac_mask[0]);
        ace_config.dst_mac[1] = ace_config.dst_mac[1] & (0xff - ace_config.dst_mac_mask[1]);
        ace_config.dst_mac[2] = ace_config.dst_mac[2] & (0xff - ace_config.dst_mac_mask[2]);
        ace_config.dst_mac[3] = ace_config.dst_mac[3] & (0xff - ace_config.dst_mac_mask[3]);
        ace_config.dst_mac[4] = ace_config.dst_mac[4] & (0xff - ace_config.dst_mac_mask[4]);
        ace_config.dst_mac[5] = ace_config.dst_mac[5] & (0xff - ace_config.dst_mac_mask[5]);
    }

    /* vlan */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vlan");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_UNTAG))
        {
            CFG_CONFLICT_RET("Vlan conflict with untag-vlan field");
        }
        ace_config.key_flags |= GLB_MAC_ACE_VLAN;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.svlan, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.svlan_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.svlan_mask &= 0xFFF;
            ace_config.svlan      &= ~(ace_config.svlan_mask);
        }
        else
        {
            ace_config.svlan_mask = 0xfff;
        }
    }

    /* inner vlan */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner_vlan");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_UNTAG))
        {
            CFG_CONFLICT_RET("Inner-vlan conflict with untag-vlan field");
        }
        ace_config.key_flags |= GLB_MAC_ACE_INNNER_VLAN;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.cvlan, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.cvlan_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.cvlan_mask &= 0xFFF;
            ace_config.cvlan      &= ~(ace_config.cvlan_mask);
        }
        else
        {
            ace_config.cvlan_mask = 0xfff;
        }
    }

    /* cos */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("cos");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_UNTAG))
        {
            CFG_CONFLICT_RET("Cos conflict with untag-vlan field");
        }
        ace_config.key_flags |= GLB_IP_ACE_COS;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.svlan_cos, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.svlan_cos_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.svlan_cos_mask &= 0x7;
            ace_config.svlan_cos      &= ~(ace_config.svlan_cos_mask);
        }
        else
        {
            ace_config.svlan_cos_mask = 0x7;
        }
    }

    /* inner cos */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner_cos");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_UNTAG))
        {
            CFG_CONFLICT_RET("Inner-cos conflict with untag-vlan field");
        }
        ace_config.key_flags |= GLB_IP_ACE_INNER_COS;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.cvlan_cos, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.cvlan_cos_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.cvlan_cos_mask &= 0x7;
            ace_config.cvlan_cos      &= ~(ace_config.cvlan_cos_mask);
        }
        else
        {
            ace_config.cvlan_cos_mask = 0x7;
        }
    }

    /*added by yejl for bug 49791, 2018-11-14*/
    /*untag-vlan*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("untag_vlan");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "untag-vlan"))
        {
            if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_VLAN) ||
                GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_INNNER_VLAN) ||
                GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_COS) ||
                GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_INNER_COS))
            {
                CFG_CONFLICT_RET("Untag-vlan conflict with vlan, inner-vlan, cos, inner-cos field");
            }
            ace_config.key_flags |= GLB_MAC_ACE_UNTAG;
            ace_config.untag_vlan = TRUE;
        }
    }
    
    /* ether type */
    /*modified by yejl for bug 49768, 2018-10-31*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ether_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_MAC_ACE_ETHTYPE;
        PM_CFG_GET_UINT(ace_config.ether_type, argv[argc_idx + 1], para->p_rs);
        if (ace_config.ether_type < 0x600)
        {
            CFG_CONFLICT_RET("Ether type min value is 0x600");
        }
        /*added by yejl for bug 49832 and 49905, 2018-11-05*/
        if (ace_config.ether_type == 0x0806)
        {
            CFG_CONFLICT_RET("Can't configure the ether-type, user can configure the arp field directly");
        }
        if (ace_config.key_arp_flags != 0)
        {
            CFG_CONFLICT_RET("ARP field is conflict with ether_type");
        }
        /*ended*/
#ifdef TSINGMA
        if (ace_config.ether_type == 0x88e7)  /*modified by yejl for bug 53861, 2019-10-10*/
        {
            CFG_CONFLICT_RET("Not support the ether-type on this product");
        }
#endif
        if (TRUE == acl_stm_is_ipv6_profile())
        {
            if (ace_config.ether_type == 0x86dd)
            {
                CFG_CONFLICT_RET("Can't configure ipv6 ether-type in mac acl, user can configure ipv6 information in ipv6 acl");
            }
        }
        ace_config.ether_type_mask = 0x0;
    }

    /* source ip address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_IPSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "src_ip_addrlen"))
        {
            /*added by yejl for bug 50944, 2019-02-25*/
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.src_ip.u.prefix4, para->p_rs);
            ipv4_mask_len = 0;
            PM_CFG_GET_UINT(ipv4_mask_len, argv[argc_idx + 4], para->p_rs);
            sal_masklen2ip(ipv4_mask_len, &ace_config.src_ip_mask.u.prefix4);
            ace_config.src_ip_mask.u.prefix4.s_addr = ~(ace_config.src_ip_mask.u.prefix4.s_addr);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 4], &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET;
        ace_config.src_ip_mask.family = AF_INET;
        ace_config.src_ip.u.prefix4.s_addr = ace_config.src_ip.u.prefix4.s_addr & ~(ace_config.src_ip_mask.u.prefix4.s_addr);
    }

    /* source ipv6 address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-ipv6");
    if (argc_idx != 0xFF && GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        CFG_CONFLICT_RET("Can't use ether_type field in ipv6 entry");
    }
    else if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_IPSA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix6, para->p_rs);
            sal_memset(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0x0 ,
                sizeof(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            sal_memset(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0xFF ,
                sizeof(ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "src_ipv6_addrlen"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 3], &ace_config.src_ip.u.prefix6, para->p_rs);
            ipv6_mask_len = 0;
            PM_CFG_GET_UINT(ipv6_mask_len, argv[argc_idx + 4], para->p_rs);
            sal_masklen2ip6(ipv6_mask_len, &ace_config.src_ip_mask.u.prefix6);
            for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
            {
                ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8[i] = ~ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
            }
        }
        else
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix6, para->p_rs);
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 4], &ace_config.src_ip_mask.u.prefix6, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET6;
        ace_config.src_ip_mask.family = AF_INET6;
        for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
        {
            ace_config.src_ip.u.prefix6.__in6_u.__u6_addr8[i] &= ~ace_config.src_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
        }
        ace_config.is_ipv6 = TRUE;
    }

    /* dest ip address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dst-ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_IPDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "dest_ip_addrlen"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.dst_ip.u.prefix4, para->p_rs);
            ipv4_mask_len = 0;
            PM_CFG_GET_UINT(ipv4_mask_len, argv[argc_idx + 4], para->p_rs);
            sal_masklen2ip(ipv4_mask_len, &ace_config.dst_ip_mask.u.prefix4);
            ace_config.dst_ip_mask.u.prefix4.s_addr = ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 4], &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET;
        ace_config.dst_ip_mask.family = AF_INET;
        ace_config.dst_ip.u.prefix4.s_addr = ace_config.dst_ip.u.prefix4.s_addr & ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
    }

    /* dest ipv6 address */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dst-ipv6");
    if (argc_idx != 0xFF && GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        CFG_CONFLICT_RET("Can't use ether_type field in ipv6 entry");
    }
    else if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {   
        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_IPDA;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix6, para->p_rs);
            sal_memset(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0x0 ,
                sizeof(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            sal_memset(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8 ,0xFF ,
                sizeof(ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8));
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "dest_ipv6_addrlen"))
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 3], &ace_config.dst_ip.u.prefix6, para->p_rs);
            ipv6_mask_len = 0;
            PM_CFG_GET_UINT(ipv6_mask_len, argv[argc_idx + 4], para->p_rs);
            sal_masklen2ip6(ipv6_mask_len, &ace_config.dst_ip_mask.u.prefix6);
            for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
            {
                ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8[i] = ~ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
            }
        }
        else
        {
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix6, para->p_rs);
            CFG_PARSER_IPV6_ADDR(argv[argc_idx + 3], &ace_config.dst_ip_mask.u.prefix6, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET6;
        ace_config.dst_ip_mask.family = AF_INET6;
        for (i = 0; i < GLB_IPV6_UINT8_LEN; i++)
        {
            ace_config.dst_ip.u.prefix6.__in6_u.__u6_addr8[i] &= ~ace_config.dst_ip_mask.u.prefix6.__in6_u.__u6_addr8[i];
        }
        ace_config.is_ipv6 = TRUE;
    }

    /* flow label */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("flow-label");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (TRUE != ace_config.is_ipv6)
        {
            CFG_CONFLICT_RET("Flow label only support in ipv6 entry");
        }
        ace_config.key_ipv6_flags |= GLB_IPV6_ACE_FLOW_LABEL;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.ipv6_flow_label, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.ipv6_flow_label_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.ipv6_flow_label_mask &= 0xFFFFF;
            ace_config.ipv6_flow_label      &= ~(ace_config.ipv6_flow_label_mask);
        }
        else
        {
            ace_config.ipv6_flow_label_mask = 0xFFFFF;
        }
    }
    
    /* dscp */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dscp");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_DSCP;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.dscp, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.dscp_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.dscp_mask &= 0x3f;
            ace_config.dscp     &= ~(ace_config.dscp_mask);
        }
        else
        {
            ace_config.dscp_mask = 0x3f;
        }
    }

    /* ip-precedence */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("precedence");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_DSCP))
        {
            return PM_E_CFG_PROMPT;
        }        
        ace_config.key_flags |= GLB_IP_ACE_PRECEDENCE;
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(ace_config.ip_precedence, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.ip_precedence_mask, argv[argc_idx + 3], para->p_rs);
            ace_config.ip_precedence_mask &= 0x7;
            ace_config.ip_precedence     &= ~(ace_config.ip_precedence_mask);
        }
        else
        {
            ace_config.ip_precedence_mask = 0x7;
        }
    }

    /* ip frag */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("fragment");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        /*added by yejl for bug 49811, 2018-11-22*/
        ace_config.key_flags |= GLB_IP_ACE_IP_FRAG;
        _acl_cmd_acl_get_ip_frag(argv[argc_idx + 1], &ace_config.ip_frag, para);
    }

    /* l3 protocol */
    /*modifiled by yejl for bug 49831, 2018-11-06*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("l3-protocol");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_IP_PROTOCOL;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.ip_protocol_mask = 0xff;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "tcp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_TCP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_UDP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "icmp"))
        {
            if (ace_config.is_ipv6)
            {
                ace_config.ip_protocol_mask = 0;
                ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_ICMPv6;  
            }
            else
            {
                ace_config.ip_protocol_mask = 0;
                ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_ICMP;
            }
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "igmp"))
        {
            ace_config.ip_protocol_mask = 0;
            ace_config.ip_protocol = GLB_ACL_L4_PROTOCOL_IGMP;
            if (ace_config.is_ipv6)
            {
                CFG_CONFLICT_RET("IPv6 entry not support igmp field");
            }
        } 
        else
        {
            PM_CFG_GET_UINT(ace_config.ip_protocol, argv[argc_idx + 2], para->p_rs);
            ace_config.ip_protocol_mask = 0;
        }
    }

    /* l4 src port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-l4-port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_SRCPORT;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_ANY;
            ace_config.l4_src_port_mask = 0xFFFF;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "range"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_RANGE;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 3], para->p_rs);
            if (ace_config.l4_src_port >= ace_config.l4_src_port_mask)
            {
                return PM_E_CFG_DB_FAIL;
            }
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "eq"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_EQ;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 2], para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gt"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_GT;
            PM_CFG_GET_UINT(ace_config.l4_src_port, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_GT_MAX < ace_config.l4_src_port)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_src_port++;
            ace_config.l4_src_port_mask = 0xffff;
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "lt"))
        {
            ace_config.l4_src_port_type = GLB_ACE_PORT_TYEP_LT;
            ace_config.l4_src_port = 0;
            PM_CFG_GET_UINT(ace_config.l4_src_port_mask, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_LT_MIN > ace_config.l4_src_port_mask)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_src_port_mask--;
            l4port_num++;
        }
    }

    /* l4 dest port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dest-l4-port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_DSTPORT;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_ANY;
            ace_config.l4_dst_port_mask = 0xFFFF;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "range"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_RANGE;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 3], para->p_rs);
            if (ace_config.l4_dst_port >= ace_config.l4_dst_port_mask)
            {
                return PM_E_CFG_DB_FAIL;
            }
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "eq"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_EQ;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 2], para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gt"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_GT;
            PM_CFG_GET_UINT(ace_config.l4_dst_port, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_GT_MAX < ace_config.l4_dst_port)
            {
                return PM_E_INVALID_PARAM;
            }  
            ace_config.l4_dst_port_mask = 0xffff;
            ace_config.l4_dst_port++;
            l4port_num++;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "lt"))
        {
            ace_config.l4_dst_port_type = GLB_ACE_PORT_TYEP_LT;
            ace_config.l4_dst_port = 0;
            PM_CFG_GET_UINT(ace_config.l4_dst_port_mask, argv[argc_idx + 2], para->p_rs);
            if (GLB_ACE_PORT_LT_MIN > ace_config.l4_dst_port_mask)
            {
                return PM_E_INVALID_PARAM;
            }
            ace_config.l4_dst_port_mask--;
            l4port_num++;
        }
    }

    /* igmp type */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("igmp_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        /*modified by yejl for bug 49821, 2018-11-01*/
        ace_config.key_flags |= GLB_IP_ACE_IGMP_TYPE;
        if (0 == sal_strcmp(argv[argc_idx + 1], "dvmrp"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_DVMRP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "host-query"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_QUERY;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v1host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_HOST_REPORT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "mtrace-route"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_MTRACT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "mtrace-response"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_MTRACT_RESPONSE;
        }   
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v2leave-group"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V2_LEAVE;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v2host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V2_REPORT;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "v3host-report"))
        {
            ace_config.igmp_type = ACL_IGMP_TYPE_V3_REPORT;
        }   
        else
        {
            PM_CFG_GET_UINT(ace_config.igmp_type, argv[argc_idx + 2], para->p_rs);
        }
    }
    
    /* icmp type*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("icmp_type");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_ICMP_TYPE;
        PM_CFG_GET_UINT(ace_config.icmp_type, argv[argc_idx + 1], para->p_rs);
    }
    /*icmp code */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("icmp_code");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_flags |= GLB_IP_ACE_ICMP_CODE;
        PM_CFG_GET_UINT(ace_config.icmp_code, argv[argc_idx + 1], para->p_rs);
    }  

    /* tcp flags */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("tcpflag");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        /*modified by yejl for bug 49115, 2018-11-27*/
        /*modified by yejl for bug 53862, 2019-10-10*/
        if (0 == sal_strcmp(argv[argc_idx + 1], "match-any"))
        {
            if (GLB_CHIP_DUET2 == tbl_chip_get_type() || GLB_CHIP_TSINGMA == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }
        }
        /*end*/
        
        _acl_cmd_acl_get_tcp_flag(&ace_config.tcp_flags, &ace_config.tcp_flags_mask, &argv[argc_idx]);
        if (0 == ace_config.tcp_flags_mask)
        {
            return PM_E_CFG_ACL_TCP_FLAGS_NONE;
        }
        ace_config.key_flags |= GLB_IP_ACE_TCP_FLAG;
        tcp_flags++;
    }

    /*time range*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("timerange");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (sal_strcmp(argv[argc_idx + 1], ""))
        {   
            /*modify by zhaoqd for bug 53534 2019-09-21*/
            args.argv[0] = acl_name;
            if (tbl_vty_iterate((TBL_ITER_CB_FUNC)_compare_acl_name_and_aclname, &args))
            {
                CFG_CONFLICT_RET("This ACL is applied for line vty, please remove it first!");
            }
            
            if (PM_E_NONE != timerange_config_check_tr_exist(argv[argc_idx + 1]))
            {
                CFG_CONFLICT_RET("Can't find this time-range %s", argv[argc_idx + 1]); /*modified by yejl for bug 51297*/
            }
            sal_strncpy(ace_config.time_range, argv[argc_idx + 1], TIME_RANGE_SIZE);
        }
        ace_config.key_flags |= GLB_IP_ACE_TIME_RANGE;
    }
    
    /* options */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("options");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether_type conflict with other field");
        }
        ace_config.key_flags |= GLB_IP_ACE_OPTIONS;
        ace_config.options = TRUE;
    }
    
    /* snmp_set */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("snmp_set");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.tap_snmp_set = TRUE;
    }

    /* arp */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("arp");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            CFG_CONFLICT_RET("Ether-type conflicts with arp field");
        }
        ace_config.key_arp_flags |= GLB_ARP_ACE_OP_CODE;

        if (0 == sal_strcmp("request", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_REQUEST;
            ace_config.arp_op_code_mask = 0;

        }
        else if (0 == sal_strcmp("response", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_RESPONSE;
            ace_config.arp_op_code_mask = 0;
        }
        else if (0 == sal_strcmp("any", argv[argc_idx + 1]))
        {
            ace_config.arp_op_code = ARP_TYPE_NULL;
            ace_config.arp_op_code_mask = 0xFFFF;
        }
        /*added by yejl for bug 49654, 2018-10-24*/
        else if (0 == sal_strcmp("rarp-request", argv[argc_idx + 1]))
        {
            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }
            ace_config.arp_op_code = ARP_TYPE_RARP_REQUEST;
            ace_config.arp_op_code_mask = 0;
        }
        else if (0 == sal_strcmp("rarp-response", argv[argc_idx + 1]))
        {
            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }
            ace_config.arp_op_code = ARP_TYPE_RARP_RESPONSE;
            ace_config.arp_op_code_mask = 0;
        }
    }
    
    /*arp sender ip conflict with src-ip, use same db to save*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("sender_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_arp_flags |= GLB_ARP_ACE_SENDER_IP;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "sender_ip_addrlen"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            ipv4_mask_len = 0;
            PM_CFG_GET_UINT(ipv4_mask_len, argv[argc_idx + 3], para->p_rs);
            sal_masklen2ip(ipv4_mask_len, &ace_config.src_ip_mask.u.prefix4);
            ace_config.src_ip_mask.u.prefix4.s_addr = ~(ace_config.src_ip_mask.u.prefix4.s_addr);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.src_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.src_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.src_ip.family = AF_INET;
        ace_config.src_ip_mask.family = AF_INET;
        ace_config.src_ip.u.prefix4.s_addr = ace_config.src_ip.u.prefix4.s_addr & ~(ace_config.src_ip_mask.u.prefix4.s_addr);
    }

    /*arp target ip conflict with dst-ip, use same db to save*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("target_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        ace_config.key_arp_flags |= GLB_ARP_ACE_TARGET_IP;
        if (0 == sal_strcmp(argv[argc_idx + 1], "host"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR("0.0.0.0", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            CFG_PARSER_IP_ADDR("255.255.255.255", &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "target_ip_addrlen"))
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            ipv4_mask_len = 0;
            PM_CFG_GET_UINT(ipv4_mask_len, argv[argc_idx + 3], para->p_rs);
            sal_masklen2ip(ipv4_mask_len, &ace_config.dst_ip_mask.u.prefix4);
            ace_config.dst_ip_mask.u.prefix4.s_addr = ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
        }
        else
        {
            CFG_PARSER_IP_ADDR(argv[argc_idx + 2], &ace_config.dst_ip.u.prefix4, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[argc_idx + 3], &ace_config.dst_ip_mask.u.prefix4, para->p_rs);
        }
        ace_config.dst_ip.family = AF_INET;
        ace_config.dst_ip_mask.family = AF_INET;
        ace_config.dst_ip.u.prefix4.s_addr = ace_config.dst_ip.u.prefix4.s_addr & ~(ace_config.dst_ip_mask.u.prefix4.s_addr);
    }

    if ((1 + current_ace_count) > GLB_ACE_CONFIG_MAX)
    {
        if (TRUE == acl_stm_is_ipv6_profile())
        {
            CFG_INVALID_PARAM_RET("The max IP+MAC+IPV6 ACE number is %d", GLB_ACE_CONFIG_MAX); 
        }
        else
        {
            CFG_INVALID_PARAM_RET("The max IP+MAC ACE number is %d", GLB_ACE_CONFIG_MAX); 
        }
    }
    
	if(__acl_config_res_check(type, &ace_config, pst_acl_config))
	{
        return PM_E_CFG_NO_RESOURCE;
    } 

    if (((pst_acl_config->intf_ref_in + pst_acl_config->intf_ref_out) * tcp_flags + current_acl_cfg_tcp_flags_count) > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }

    if (pst_acl_config->tcp_flags_ref + tcp_flags > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        if (((ace_config.key_flags & GLB_MAC_ACE_ETHTYPE) == GLB_MAC_ACE_ETHTYPE) 
            && pst_acl_config->intf_ref_out)
        {
            return PM_E_CFG_PM_ERROR;
        }
    }
    
    /*added by yejl*/
    if(GLB_FLAG_ISSET(ace_config.key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        ethertype_or_arp_flag = 1;
    }
    else if (GLB_FLAG_ISSET(ace_config.key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        ethertype_or_arp_flag = 2;
    }
    
    /*ended*/
    rc = acl_policy_operation_res_check(NULL, 0, NULL, NULL, ace_config.key.acl_name, ethertype_or_arp_flag, tcp_flags, GLB_ACL_POLICY_OP_ACL_ACE);
    if(PM_E_NONE != rc)
    {
        return rc;
    }
    
    if (ace_entry_config_check(&ace_config))
    {
        return PM_E_INIT_FAIL;
    }
    
    /* check l4 port resource */
    if (ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_RANGE ||
        ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_GT    ||
        ace_config.l4_src_port_type == GLB_ACE_PORT_TYEP_LT)
    {
        if(TRUE == _check_l4_port_range_conflict(ace_config.l4_src_port, ace_config.l4_src_port_mask, 1))
        {
            return PM_E_RESOURCE_FULL;
        }
    }
    if (ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_RANGE ||
        ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_GT    ||
        ace_config.l4_dst_port_type == GLB_ACE_PORT_TYEP_LT)
    {
        if(TRUE == _check_l4_port_range_conflict(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0))
        {
            return PM_E_RESOURCE_FULL;
        }
    }

    /* check service acl attached */
    if (pst_acl_config->service_ipt_attached)
    {
        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_TELNET)
            || GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_SSH))
        {
            rc = swth_sys_check_telnet_ssh_ace(&ace_config);
            if (rc < 0)
            {
                return PM_E_CFG_ACL_SERVICE_ATTACHED_CONFLICT;
            }
        }
    }

    if (0 == *p_seq_no)
    {
        if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == pst_acl_config->type_identifying)
        {
            if ((pst_acl_config->seq_ref + 1) > ACL_MAX_FLOW_MATCH_ACE_SEQUENCE_NUMBER)
            {
                return PM_E_NOT_PERMIT;
            }
            else
            {
                ace_config.key.seq_no = pst_acl_config->seq_ref + 1;
            }
            *p_seq_no = ace_config.key.seq_no;
            pst_acl_config->seq_ref += 1; 
        }
        else
        {
            if ((pst_acl_config->seq_ref + 10) > ACL_MAX_ACE_SEQUENCE_NUMBER)
            {
                return PM_E_NOT_PERMIT;
            }
            else
            {
                ace_config.key.seq_no = pst_acl_config->seq_ref + 10;
            }
            *p_seq_no = ace_config.key.seq_no;
            pst_acl_config->seq_ref += 10; 
        }
    }
    else
    {
        ace_config.key.seq_no = *p_seq_no;
        if (*p_seq_no > pst_acl_config->seq_ref)
        {
            pst_acl_config->seq_ref = ace_config.key.seq_no;
        }
    }
    rc = tbl_ace_config_add_ace_config(&ace_config); 
    if (PM_E_NONE != rc)
    {
        /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
        return rc;
    }

    pst_ace_config = tbl_ace_config_get_ace_config(&(ace_config.key));
    if (NULL == pst_ace_config)
    {
        /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
        return PM_E_NOT_EXIST;
    }

    /* add service acl rule */
    if (pst_acl_config->service_ipt_attached)
    {
        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_TELNET))
        {
            ipt_add_service_acl_rule(pst_ace_config, GLB_SERVICE_TYPE_TELNET);
        }

        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_SSH))
        {
            ipt_add_service_acl_rule(pst_ace_config, GLB_SERVICE_TYPE_SSH);
        }
    }

    if (ace_config.acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
    {
        pst_acl_config->acev6_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEV6_REF);
    }
    else if (ace_config.acl_type == GLB_ACL_CONFIG_TYPE_MAC)
    {
        pst_acl_config->acemac_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEMAC_REF);
    }
    else if (ace_config.acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
    {
        pst_acl_config->ace_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACE_REF);
    }

    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_SEQ_REF);
    
    pst_acl_config->l4_port_ref += l4port_num;
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_L4_PORT_REF);

    pst_acl_config->tcp_flags_ref += tcp_flags;
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF);

    
    if ((ace_config.key_flags & GLB_MAC_ACE_ETHTYPE) == GLB_MAC_ACE_ETHTYPE)
    {
        pst_acl_config->ether_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ETHER_REF);
    }
    
    if (GLB_FLAG_ISSET(ace_config.key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        pst_acl_config->arp_ref++;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ARP_REF);
    }

    if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        _acl_time_range_update_refcnt(ace_config.time_range, TRUE);
    }
    
    rc = acl_entry_add_ace(pst_ace_config);
    if(PM_E_NONE != rc)
    {
         /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(ace_config.l4_dst_port, ace_config.l4_dst_port_mask, 0);
         
        if (GLB_FLAG_ISSET(ace_config.key_flags, GLB_IP_ACE_TIME_RANGE))
        {
            _acl_time_range_update_refcnt(ace_config.time_range, FALSE);
        }
        return rc;
    }    
    return acl_policy_operation(NULL, 0, NULL, NULL, ace_config.key.acl_name, ace_config.key.seq_no, GLB_ACL_POLICY_OP_ACL_ACE, GLB_ACL_POLICY_OP_ACTION_ADD);
}
#endif

int32
_acl_get_new_seq_ref(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    char *acl_name = pargs->argv[0];
    uint32 *seq_no = pargs->argv[1];
    uint32 *new_seq_no = pargs->argv[2];

    if (0 == sal_strcmp(acl_name, p_ace_config->key.acl_name))
    {
        if (p_ace_config->key.seq_no > *new_seq_no && 
            p_ace_config->key.seq_no < *seq_no)
        {
            *new_seq_no = p_ace_config->key.seq_no;
        }
    }

    return PM_E_NONE;
}

int32
acl_get_ace_seq_ref(char *acl_name, uint32 *seq_no, uint32 *new_seq_no)
{    
    tbl_iter_args_t args;
    args.argv[0] = acl_name;
    args.argv[1] = seq_no;
    args.argv[2] = new_seq_no;

    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_get_new_seq_ref, &args);

    return PM_E_NONE;
}

int32
acl_config_del_ace(char *acl_name, uint32 seq_no,char **argv, int32 argc, uint32 snmp_set)
{
    tbl_ace_config_t *pst_ace_config = NULL;
    tbl_ace_config_key_t ace_config_key;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    uint32 new_seq_no = 0;
    int32 rc = 0;
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
 #ifdef TAPPRODUCT
    tbl_sys_global_t* p_sys_global = tbl_sys_global_get_sys_global();
    tbl_acl_config_t *pst_flow_match_config = NULL;
    tbl_acl_config_key_t flow_match_config_key;
    int32 is_ipv6 = FALSE;
 #endif
    uint8 argc_idx = 0;

    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));

    /* get acl_config type, normal acl or flow or ipv6 acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }
    
    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config_key.name);

    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_INVALID_PARAM;
    }
    
    sal_memset(&ace_config_key, 0, sizeof(tbl_ace_config_key_t));
    sal_strcpy(ace_config_key.acl_name, acl_config_key.name);
    ace_config_key.seq_no = seq_no;

    pst_ace_config = tbl_ace_config_get_ace_config(&ace_config_key);
    if (NULL == pst_ace_config)
    {
        return PM_E_NOT_EXIST;
    }
#ifdef TAPPRODUCT
    is_ipv6 = pst_ace_config->is_ipv6;
    uint32 is_decap = FALSE;
    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TAP_TRUNCATION))
    {
        pst_acl_config->ace_truncation_ref_cnt--;
        if (!pst_acl_config->ace_truncation_ref_cnt)
        {
            tap_group_truncation_flow_update_ace_used_rcs(acl_name);
        }
    }
   
    pst_ace_config->tap_snmp_set = snmp_set;

    if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == pst_acl_config->type_identifying)
    {
        foreach_ace_config_start(ace_node)
        {
            if (ace_node->flow_match_name ==0 || 
                0 != sal_strcmp(ace_node->flow_match_name, pst_ace_config->key.acl_name))
            {
                continue;
            }

            tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_DEL_ACE, ace_node, pst_ace_config, is_decap);
        }foreach_ace_config_end
    }else if(GLB_ACL_CONFIG_TYPE_FLOW == pst_acl_config->type_identifying){
        if(GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
        {
            foreach_ace_config_start(ace_node)
            {
                if (0 != sal_strcmp(ace_node->key.acl_name, pst_ace_config->flow_match_name))
                {
                    continue;
                }

                tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_DEL_ACE, pst_ace_config, ace_node, is_decap);
            }foreach_ace_config_end
        }else{
            is_decap = pst_acl_config->decap_en;
            tap_group_ingress_flow_operation(GLB_TAP_GRP_IGR_FLOW_OP_DEL_ACE, pst_ace_config, NULL, is_decap);
        }
    }
    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        _acl_time_range_update_refcnt(pst_ace_config->time_range, FALSE);
    }

    if(GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
    {
        sal_memset(&flow_match_config_key,0,sizeof(tbl_acl_config_key_t));
        sal_strcpy(flow_match_config_key.name, pst_ace_config->flow_match_name);
        pst_flow_match_config = tbl_acl_config_get_acl_config(&flow_match_config_key);
        pst_flow_match_config->intf_ref_in--;
        tbl_acl_config_set_acl_config_field(pst_flow_match_config, TBL_ACL_CONFIG_FLD_INTF_REF_IN);
    }
    /*added by yejl for tap bug 53519, 2019-09-16*/
    if (p_sys_global->add_gre_ace_entry_num > 0)
    {
        p_sys_global->add_gre_ace_entry_num --;
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM);
    }
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    if (p_sys_global->add_vxlan_entry_num > 0)
    {
        p_sys_global->add_vxlan_entry_num --;
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM);
    }
    /*ended by guoxd*/
    
    acl_entry_del_ace(pst_ace_config);

    if (is_ipv6)
    {
        pst_acl_config->acev6_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEV6_REF);
    }
    else
    {
        pst_acl_config->ace_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACE_REF);
    }
#else
    rc = acl_policy_operation(NULL, 0, NULL, NULL, acl_name, seq_no, GLB_ACL_POLICY_OP_ACL_ACE, GLB_ACL_POLICY_OP_ACTION_DEL);
    if (PM_E_NONE != rc)
    {
        return rc;
    }

    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        _acl_time_range_update_refcnt(pst_ace_config->time_range, FALSE);
    }
    
    acl_entry_del_ace(pst_ace_config);

    if (type == GLB_ACL_CONFIG_TYPE_IPV6)
    {
        pst_acl_config->acev6_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEV6_REF);
    }
    else if (type == GLB_ACL_CONFIG_TYPE_MAC)
    {
        pst_acl_config->acemac_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACEMAC_REF);
    }
    else if (type == GLB_ACL_CONFIG_TYPE_NORMAL)
    {
        pst_acl_config->ace_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ACE_REF);
    }
#endif

    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        pst_acl_config->tcp_flags_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF);
    }
    /*modified by yejl for bug 49804, 2018-11-01*/
    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        pst_acl_config->ether_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ETHER_REF);
    }
    /*end*/

    if (GLB_FLAG_ISSET(pst_ace_config->key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        pst_acl_config->arp_ref--;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_ARP_REF);
    }

    if (pst_ace_config->l4_src_port_type != 0 
        && pst_ace_config->l4_src_port_type != GLB_ACE_PORT_TYEP_EQ 
        && pst_ace_config->l4_src_port_type != GLB_ACE_PORT_TYEP_ANY)
    {
        pst_acl_config->l4_port_ref--;
    }

    if (pst_ace_config->l4_dst_port_type != 0 
        && pst_ace_config->l4_dst_port_type != GLB_ACE_PORT_TYEP_EQ
        && pst_ace_config->l4_dst_port_type != GLB_ACE_PORT_TYEP_ANY)
    {
        pst_acl_config->l4_port_ref--;
    }
     /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
    if (pst_ace_config->l4_src_port_type != 0 
        && pst_ace_config->l4_src_port_type != GLB_ACE_PORT_TYEP_EQ
        && pst_ace_config->l4_src_port_type != GLB_ACE_PORT_TYEP_ANY)
    {
        _remove_l4_port_range(pst_ace_config->l4_src_port, pst_ace_config->l4_src_port_mask, 1);
    }

    if (pst_ace_config->l4_dst_port_type != 0 
        && pst_ace_config->l4_dst_port_type != GLB_ACE_PORT_TYEP_EQ
        && pst_ace_config->l4_dst_port_type != GLB_ACE_PORT_TYEP_ANY)
    {
        _remove_l4_port_range(pst_ace_config->l4_dst_port, pst_ace_config->l4_dst_port_mask, 0);
    }
    
    tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_L4_PORT_REF);

    /* added by huwx for acl udf 2017-07-19 bug 44643 */
    if (GLB_FLAG_ISSET(pst_ace_config->key_flags, GLB_IP_ACE_UDF))
    {
        if (pst_ace_config->udf_type != GLB_ACL_UDF_GROUP_UDF_ID)
        {
            _remove_udf_group(pst_ace_config->udf_type, 
                GLB_ACL_UDF_GROUP_L3_HEAD == pst_ace_config->udf_type ? 0 : GLB_ACL_UDF_VERSION_V4,
                GLB_ACL_UDF_GROUP_L3_HEAD == pst_ace_config->udf_type ? 0 : pst_ace_config->ip_protocol, 
                pst_ace_config->udf_offset);
        }
        else
        {
            tbl_acl_udf_entry_t *p_db_udf = NULL;
            tbl_acl_udf_entry_t  udf_entry;

            sal_memset(&udf_entry, 0, sizeof(udf_entry));
            udf_entry.key.id = pst_ace_config->udf_id;
            p_db_udf = tbl_acl_udf_entry_get_acl_udf_entry(&udf_entry.key);
            if (p_db_udf)
            {
                p_db_udf->acl_ref_cnt--;
                tbl_acl_udf_entry_set_acl_udf_entry_field(p_db_udf, TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT);
            }
        }
    }

    /* delete service acl rule */
    if (pst_acl_config->service_ipt_attached)
    {
        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_TELNET))
        {
            ipt_del_service_acl_rule(pst_ace_config, GLB_SERVICE_TYPE_TELNET);
        }

        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_SSH))
        {
            ipt_del_service_acl_rule(pst_ace_config, GLB_SERVICE_TYPE_SSH);
        }
    }

    if (seq_no == pst_acl_config->seq_ref)
    {
        acl_get_ace_seq_ref(ace_config_key.acl_name, &ace_config_key.seq_no, &new_seq_no);
        pst_acl_config->seq_ref = new_seq_no;
        tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_SEQ_REF);
    }
    rc = tbl_ace_config_del_ace_config(&ace_config_key); 
    return rc;
}

int32
_acl_config_clear_interface(tbl_interface_t *p_interface, tbl_iter_args_t *pargs)
{
    char *acl_name = pargs->argv[0];

    /*for ip acl*/
    if (0 == sal_strcmp(acl_name, p_interface->igs_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, TRUE);
    }
    if (0 == sal_strcmp(acl_name, p_interface->egs_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, FALSE);
    }
    /*ended*/

    /*ipv6 acl*/
    if (0 == sal_strcmp(acl_name, p_interface->igs_ipv6_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_IPV6, TRUE);
    }
    if (0 == sal_strcmp(acl_name, p_interface->egs_ipv6_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_IPV6, FALSE);
    }
    /*ended*/

    /*for mac acl*/
    if (0 == sal_strcmp(acl_name, p_interface->igs_mac_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_MAC, TRUE);
    }
    if (0 == sal_strcmp(acl_name, p_interface->egs_mac_acl))
    {
        acl_detach_interface(acl_name, p_interface->key.name, GLB_ACL_CONFIG_TYPE_MAC, FALSE);
    }
    /*ended*/

    return PM_E_NONE;
}


int32
_acl_config_clear_ace(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    char *acl_name = pargs->argv[0];

    if (0 == sal_strcmp(acl_name, p_ace_config->key.acl_name))
    {
        acl_config_del_ace(acl_name, p_ace_config->key.seq_no, NULL, 0, FALSE);
    }

    return PM_E_NONE;
}

int32
acl_config_clear_acl(char *acl_name)
{    
    tbl_iter_args_t args;
    args.argv[0] = acl_name;

    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_config_clear_ace, &args);
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_acl_config_clear_interface, &args);

    return PM_E_NONE;
}

static int32
acl_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    /*for ip acl*/
    if (0 != sal_strlen(p_db_agg_if->igs_acl))
    {
        acl_detach_interface(p_db_agg_if->igs_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, TRUE);
    }   
    if (0 != sal_strlen(p_db_agg_if->egs_acl))
    {
        acl_detach_interface(p_db_agg_if->egs_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, FALSE);
    }
    /*ended*/ 

    /*for ipv6 acl*/
    if (0 != sal_strlen(p_db_agg_if->igs_ipv6_acl))
    {
        acl_detach_interface(p_db_agg_if->igs_ipv6_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_IPV6, TRUE);
    }
    if (0 != sal_strlen(p_db_agg_if->egs_ipv6_acl))
    {
        acl_detach_interface(p_db_agg_if->egs_ipv6_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_IPV6, FALSE);
    }
    /*ended*/

    /*for mac acl*/
    if (0 != sal_strlen(p_db_agg_if->igs_mac_acl))
    {
        /*modified by yejl for bug 49915, 2018-11-09*/
        acl_detach_interface(p_db_agg_if->igs_mac_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_MAC, TRUE);
    }
    if (0 != sal_strlen(p_db_agg_if->egs_mac_acl))
    {
        /*modified by yejl for bug 49915, 2018-11-09*/
        acl_detach_interface(p_db_agg_if->igs_mac_acl, p_db_agg_if->key.name, GLB_ACL_CONFIG_TYPE_MAC, FALSE);
    }
    /*ended*/

    return PM_E_NONE;
}

int32
acl_config_set_remark(char *acl_name, char **argv, int32 argc, char *remark)
{
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_t acl_config;
    int32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    uint8 argc_idx = 0;

    sal_memset(&acl_config, 0, sizeof(tbl_acl_config_t));

    /* get acl_config type, flow acl or ip/mac/ipv6 acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if (argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }

    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config.key.name);

    pst_acl_config = tbl_acl_config_get_acl_config(&(acl_config.key));
    if (NULL == pst_acl_config)
    {
        return PM_E_ACL_NOT_EXIST;/*modified by yej for bug 54155, 2019-10-29*/
    }

    if (remark)
    {
        if (check_name_character_and_len(remark, ACL_REMARK_SIZE) != 0)
        {
            return PM_E_INVALID_PARAM;
        }
        sal_strncpy(pst_acl_config->remark, remark, ACL_REMARK_SIZE);
    }
    else
    {
        sal_strncpy(pst_acl_config->remark, "", ACL_REMARK_SIZE);
    }
    
    return tbl_acl_config_set_acl_config_field(pst_acl_config, TBL_ACL_CONFIG_FLD_REMARK);

}


int32
acl_worm_filter_add(char *acl_worm_filter_name, uint32 istcp, uint32 dstport, uint32 isrange)
{
    tbl_acl_worm_filter_t *pst_acl_worm_filter = NULL;
    tbl_acl_worm_filter_t acl_worm_filter;
    int32 rc = 0;
    /* modified by huwx for bug 39772, acl stats id only support 1-4095 */
    uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 current_acl_entry_count_ingress = 0;
    uint32 current_acl_entry_count_egress = 0;
    uint32 current_acl_entry_count_wromfilter = 0;
    
    sal_memset(&acl_worm_filter, 0, sizeof(tbl_acl_worm_filter_t));
    if (check_name_character_and_len(acl_worm_filter_name, ACL_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_strcpy(acl_worm_filter.key.worm_filter_name, acl_worm_filter_name);

    pst_acl_worm_filter = tbl_acl_worm_filter_get_acl_worm_filter(&(acl_worm_filter.key));
    if (NULL != pst_acl_worm_filter)
    {
        return PM_E_EXIST;
    }

    if (isrange && dstport >= isrange)
    {
        return PM_E_INIT_FAIL;
    }

    acl_worm_filter.dstport = dstport;
    acl_worm_filter.istcp = istcp;
    acl_worm_filter.isrange = isrange;

    if (acl_worm_filter_config_check(&acl_worm_filter))
    {
        return PM_E_EXIST;
    }

    tbl_acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, &current_acl_entry_count_egress, &current_acl_entry_count_wromfilter);

    if (current_acl_entry_count_ingress + current_acl_entry_count_wromfilter + 1 > stm_max_ingress)
    {
        return PM_E_EXCEED_MAX_NUM;
    }

#if 0 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
    if (isrange)
    {
         if (tbl_acl_config_get_l4port_config_count() + tbl_acl_worm_filter_get_range_count() + 1 >GLB_ACL_L4PORT_ACE_NUM_MAX)
         {
             return PM_E_CFG_NO_RESOURCE;
         }
    }
#endif

    acl_worm_filter.enable = TRUE;
    acl_worm_filter.seq_num = ctclib_opf_alloc_offset (acl_worm_filter_seq_opf, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == acl_worm_filter.seq_num)
    {
        return PM_E_CFG_NO_RESOURCE;
    }

#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
    if(isrange && TRUE == _check_l4_port_range_conflict(acl_worm_filter.dstport, acl_worm_filter.isrange, 0)) /* worm filter use dst_port_range */
    {
         return PM_E_CFG_NO_RESOURCE;
    }
#endif

    rc = acl_entry_add_worm_filter(&acl_worm_filter);
    if (rc != PM_E_NONE)
    {
#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
        _remove_l4_port_range(acl_worm_filter.dstport, acl_worm_filter.isrange, 0);
#endif
        return rc;
    }

    return tbl_acl_worm_filter_add_acl_worm_filter(&acl_worm_filter);

}

int32
acl_worm_filter_del(char *acl_worm_filter_name)
{
    tbl_acl_worm_filter_t *pst_acl_worm_filter = NULL;
    tbl_acl_worm_filter_key_t acl_worm_filter_key;
    int32 rc = 0;
    uint32 seq_num = 0;
    
    sal_memset(&acl_worm_filter_key, 0, sizeof(acl_worm_filter_key));
    if (check_name_character_and_len(acl_worm_filter_name, ACL_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_strcpy(acl_worm_filter_key.worm_filter_name, acl_worm_filter_name);

    pst_acl_worm_filter = tbl_acl_worm_filter_get_acl_worm_filter(&acl_worm_filter_key);
    if (NULL == pst_acl_worm_filter)
    {
        return PM_E_NOT_EXIST;
    }

    seq_num = pst_acl_worm_filter->seq_num;

    rc = acl_entry_del_worm_filter(pst_acl_worm_filter);
    if (rc != PM_E_NONE)
    {
        return rc;
    }

#if 1 /* SYSTEM MODIFIED fix acl l4-port-range check bug add by wangqj at 2016/9/9 */
    _remove_l4_port_range(pst_acl_worm_filter->dstport, pst_acl_worm_filter->isrange, 0);
#endif

    ctclib_opf_free_offset(acl_worm_filter_seq_opf, 0, 1, seq_num);
    return  tbl_acl_worm_filter_del_acl_worm_filter(&acl_worm_filter_key);
}

int32
acl_config_add_acl(char *acl_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_acl_config_t        *pst_acl_config = NULL;
    tbl_acl_config_t        acl_config;
    tbl_arpacl_config_t     *p_arpacl_config = NULL;
    tbl_arpacl_config_key_t arpacl_config_key;
    unsigned char           argc_idx    = 0;
    int32                   type        = GLB_ACL_CONFIG_TYPE_NORMAL;
    int32                   rc          = 0;
    uint32 current_acl_count = tbl_acl_config_get_acl_config_count();

    sal_memset(&acl_config, 0, sizeof(tbl_acl_config_t));

    /* get acl_config type, normal acl or flow or ipv6 acl or mac acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }

    /*added by yejl for ipv6 acl*/
    if ((FALSE == acl_stm_is_ipv6_profile())&& 
        (type == GLB_ACL_CONFIG_TYPE_IPV6))
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    /*ended by yejl*/
    
    if (check_name_character_and_len(acl_name, ACL_NAME_SIZE) != 0)
    {
        CFG_CONFLICT_RET("The initial character of %s name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20",
                                glb_acl_type_str[type]);
    }
    
    /*modfified by yejl for bug 49965, 2018-11-13*/
    sal_memset(&arpacl_config_key, 0, sizeof(arpacl_config_key));
    sal_strncpy(arpacl_config_key.name, acl_name, ARPACL_NAME_SIZE);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
    if (NULL != p_arpacl_config)
    {
        CFG_CONFLICT_RET("Can't configure the %s, it has configured by ARP ACL", glb_acl_type_str[type]);
    }
    /*end*/
    
    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config.key.name);

    if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == type)
    {
        if(!check_feature_support(CONFIG_FEATURE_ACL_FLOW_MATCH))
        {
            CFG_INVALID_PARAM_RET("This function not support");
            return PM_E_NOT_SUPPORT;
        }    
    }
    
    pst_acl_config = tbl_acl_config_get_acl_config(&(acl_config.key));
    if (NULL != pst_acl_config)
    {
        #ifdef TAPPRODUCT
        argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("flow_ace_type");
        if(argc_idx != 0xFF)
        {
            if(FALSE == pst_acl_config->decap_en)
            {
                CFG_CONFLICT_RET("The %s is exist", glb_acl_type_str[type]);
                return PM_E_EXIST;
            }
        }
        #endif
        
        /*added by yejl for IP/IPv6/MAC acl*/
        if (type != pst_acl_config->type_identifying)
        {
            CFG_CONFLICT_RET("The %s is %s", acl_name, glb_acl_type_str[pst_acl_config->type_identifying]);
        }
        /*ended*/
        return PM_E_NONE;
    }

    acl_config.type_identifying = type;

    /*Max mac/ip/ipv6 acl number is 4096*/
    if ((1 + current_acl_count) > GLB_ACL_CONFIG_MAX)
    {   
        if (TRUE == acl_stm_is_ipv6_profile())
        {
            CFG_CONFLICT_RET("The max IP+MAC+IPV6 acl number is %d", GLB_ACL_CONFIG_MAX);
        }
        else
        {
            CFG_CONFLICT_RET("The max IP+MAC acl number is %d", GLB_ACL_CONFIG_MAX);
        }
    }

    #ifdef TAPPRODUCT
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("flow_ace_type");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strcmp("decap", argv[argc_idx + 1]))
        {
            acl_config.decap_en = TRUE;
            acl_config.decap_normal_seq_min         = ACL_MAX_ACE_SEQUENCE_NUMBER;
            acl_config.decap_inner_match_seq_max    = 0;
        }
    }
    #endif
    rc = tbl_acl_config_add_acl_config(&acl_config); 
    return rc;
}

int32
acl_config_del_acl(char *acl_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    int32                   rc          = 0;
    uint32                  argc_idx    = 0;
    int32                   type        = GLB_ACL_CONFIG_TYPE_NORMAL;

    /* get acl_config type, normal acl or flow or ipv6 acl or mac acl*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("type");
    if(argc_idx != 0xFF)
    {
        type = acl_get_acl_config_type_by_name(argv[argc_idx + 1]);
    }
    
    /*added by yejl for ipv6 acl*/
    if ((FALSE == acl_stm_is_ipv6_profile())&& 
        (type == GLB_ACL_CONFIG_TYPE_IPV6))
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");   
    }
    /*ended by yejl*/

    if (check_name_character_and_len(acl_name, ACL_NAME_SIZE) != 0)
    {
        CFG_CONFLICT_RET("The initial character of %s name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20",
                                glb_acl_type_str[type]);
    }
    
    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
    acl_generate_acl_config_key_name_by_type(acl_name, type, acl_config_key.name);

    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        CFG_CONFLICT_RET("The %s %s does not exist", glb_acl_type_str[type], acl_name);
    }
    
    /*added by yejl for IP/IPv6/MAC acl*/
    if (pst_acl_config->type_identifying != type)
    {
        CFG_CONFLICT_RET("The %s is not the %s", acl_name, glb_acl_type_str[type]);
    }
    /*ended by yejl*/

    /* service acl attached */
    if (pst_acl_config->service_ipt_attached)
    {
        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_TELNET))
        {
            swth_sys_set_telnet_ssh_acl(pst_acl_config, GLB_SERVICE_TYPE_TELNET, FALSE);
        }

        if (GLB_FLAG_ISSET(pst_acl_config->service_ipt_attached, GLB_SERVICE_TYPE_SSH))
        {
            swth_sys_set_telnet_ssh_acl(pst_acl_config, GLB_SERVICE_TYPE_SSH, FALSE);
        }
    }

#ifdef TAPPRODUCT    
    if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == type)
    {
        if(!check_feature_support(CONFIG_FEATURE_ACL_FLOW_MATCH))
        {
            CFG_INVALID_PARAM_RET("This function not support");
        }    
    }
    if(GLB_ACL_CONFIG_TYPE_FLOW_MATCH == type)
    {
        if(pst_acl_config->intf_ref_in)
        {
            CFG_INVALID_PARAM_RET("Inner-match in used");
        }
    }
    tap_group_ingress_flow_del_acl_flow(acl_config_key.name);
#endif
    acl_policy_operation_acl_group_del(acl_config_key.name);
    acl_config_clear_acl(acl_config_key.name);

    rc = tbl_acl_config_del_acl_config(&acl_config_key); 
    return rc;
}

int32
timerange_config_check_tr_exist(char *tr_name)
{
    tbl_time_range_t *pst_time_range = NULL;
    tbl_time_range_t time_range;

    if (check_name_character_and_len(tr_name, TIME_RANGE_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&time_range, 0, sizeof(tbl_time_range_t));
    sal_strcpy(time_range.key.name, tr_name);

    pst_time_range = tbl_time_range_get_time_range(&(time_range.key));
    if (NULL == pst_time_range)
    {
        return PM_E_NOT_EXIST;
    }
    return PM_E_NONE;
}

int32
acl_api_add_udf_entry(tbl_acl_udf_entry_t *p_udf_entry, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 l3_udf_cnt = 0;
    uint32 l4_udf_cnt = 0;
    uint32 udf_id     = 0;
    tbl_acl_udf_entry_master_t *p_master = NULL;
    tbl_acl_udf_entry_t        *p_db_udf = NULL;

    p_master = tbl_acl_udf_entry_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("udf entry not exist");
    }     

    if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        if (GLB_ACL_UDF_OFFSET_L2 == p_udf_entry->offset_type)
        {
            CFG_INVALID_PARAM_RET("UDF offset started from L2 header is not supported");
        }

        for (udf_id = 0; udf_id < GLB_ACL_UDF_ENTRY_NUM_MAX; udf_id++)
        {
            p_db_udf = p_master->acl_udf_entry_array[udf_id];
            if (!p_db_udf)
            {
                continue;
            }

            if (GLB_ACL_UDF_OFFSET_L3 == p_db_udf->offset_type)
            {
                l3_udf_cnt++;
            }
            else if (GLB_ACL_UDF_OFFSET_L4 == p_db_udf->offset_type)
            {
                l4_udf_cnt++;
            }
        }

        if (GLB_ACL_UDF_OFFSET_L3 == p_udf_entry->offset_type)
        {
            if (l3_udf_cnt >= GLB_ACL_UDF_ENTRY_NUM_MAX/2)
            {
                CFG_CONFLICT_RET("Support 4 L3 UDF maximum");
            }
        }
        else if (GLB_ACL_UDF_OFFSET_L4 == p_udf_entry->offset_type)
        {
            if (l4_udf_cnt >= GLB_ACL_UDF_ENTRY_NUM_MAX/2)
            {
                CFG_CONFLICT_RET("Support 4 L4 UDF maximum");
            }
        }
        #ifdef OFPRODUCT
        /* hybrid580 just support 1 udf */
        if (l3_udf_cnt + l4_udf_cnt >= 1)
        {
            CFG_CONFLICT_RET("Current system just support 1 udf");
        }
        #endif
    }

    PM_E_RETURN(tbl_acl_udf_entry_add_acl_udf_entry(p_udf_entry));

    return rc;
}

int32
acl_api_set_udf_offset(tbl_acl_udf_entry_t *udf_entry, tbl_acl_udf_entry_t *p_udf_entry, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 iter = 0;

    if (udf_entry->offset[0] > GLB_ACL_UDF_OFFSET_MAX || udf_entry->offset[1] > GLB_ACL_UDF_OFFSET_MAX
     || udf_entry->offset[2] > GLB_ACL_UDF_OFFSET_MAX || udf_entry->offset[3] > GLB_ACL_UDF_OFFSET_MAX)
    {
        CFG_INVALID_PARAM_RET("UDF offset out of range");
    }

    if (!sal_memcmp(udf_entry->offset, p_udf_entry->offset, sizeof(p_udf_entry->offset))
        && !sal_memcmp(udf_entry->valid, p_udf_entry->valid, sizeof(p_udf_entry->valid)))
    {
        return PM_E_NONE;
    }

    if (p_udf_entry->acl_ref_cnt)
    {
        CFG_INVALID_PARAM_RET("UDF entry has been used by flow, can not be updated");
    }

    /* Duet2 offset must be multiple of 4 bytes */
    if (GLB_CHIP_GOLDENGATE != tbl_chip_get_type())
    {
        if (udf_entry->offset[0] % 4 || udf_entry->offset[1] % 4
         || udf_entry->offset[2] % 4 || udf_entry->offset[3] % 4)
        {
            CFG_INVALID_PARAM_RET("UDF offset must be multiple of 4 bytes on this product");
        }
    }

    /* for bug51087 support update <- GG offset must be set after udf match field set */
    /* if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        if (!p_udf_entry->key_flags)
        {
            CFG_INVALID_PARAM_RET("UDF offset must be set after udf match field");
        }
    } */

    for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
    {
        p_udf_entry->offset[iter] = udf_entry->offset[iter];
        p_udf_entry->valid[iter]  = udf_entry->valid[iter];
    }

    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_VALID));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_OFFSET));

    return rc;
}

static int32
acl_api_check_udf_match_field(tbl_acl_udf_entry_t *udf_entry, tbl_acl_udf_entry_t *p_db_udf, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32  udf_id  = 0;
    tbl_acl_udf_entry_master_t *p_master = tbl_acl_udf_entry_get_master();
    tbl_acl_udf_entry_t        *p_udf = NULL;

    if (p_db_udf->acl_ref_cnt)
    {
        CFG_INVALID_PARAM_RET("UDF entry has been used by flow, can not be updated");
    }

    if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        /* if (GLB_ACL_UDF_ETHTYPE != udf_entry->key_flags
         && GLB_ACL_UDF_IPPROTO != udf_entry->key_flags
         && GLB_ACL_UDF_IPPROTO + GLB_ACL_UDF_ETHTYPE != udf_entry->key_flags)
        {
            CFG_INVALID_PARAM_RET("Only support ether-type and ip-protocol match field");
        }

        if (p_db_udf->key_flags)
        {
            CFG_INVALID_PARAM_RET("Not support update match field");
        } */

        if (GLB_ACL_UDF_OFFSET_L3 == p_db_udf->offset_type)
        {
            if (GLB_ACL_UDF_ETHTYPE != udf_entry->key_flags)
            {
                CFG_INVALID_PARAM_RET("Only support ether-type match field in L3-UDF");
            }
            
            if (udf_entry->ethtype_mask != 0xFFFF)
            {
                CFG_INVALID_PARAM_RET("ether-type mask must be 0x0");
            }

            if (udf_entry->ethtype != 0x0800 && udf_entry->ethtype != 0x8847 && udf_entry->ethtype != 0x8848)
            {
                CFG_INVALID_PARAM_RET("Only support match ether-type 0x8847/0x8848/0x0800");
            }
        }

        if (GLB_ACL_UDF_OFFSET_L4 == p_db_udf->offset_type)
        {
            if (GLB_ACL_UDF_IPPROTO != udf_entry->key_flags)
            {
                CFG_INVALID_PARAM_RET("Only support ip-protocol match field in L4-UDF");
            }
            
            if (udf_entry->ipproto_mask != 0xFF)
            {
                CFG_INVALID_PARAM_RET("ip-protocol can not be any");
            }

            if (udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_TCP && udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_UDP
             && udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_GRE && udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_SCTP
             && udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_ICMP && udf_entry->ipproto != GLB_ACL_L4_PROTOCOL_IGMP)
            {
                CFG_INVALID_PARAM_RET("Only support match ip-protocol icmp/igmp/tcp/udp/sctp/gre");
            }
        }

        for (udf_id = 0; udf_id < GLB_ACL_UDF_ENTRY_NUM_MAX; udf_id++)
        {
            p_udf = p_master->acl_udf_entry_array[udf_id];
            if (!p_udf || udf_id == p_db_udf->key.id)
            {
                continue;
            }

            if (p_db_udf->offset_type == p_udf->offset_type)
            {
                if ((udf_entry->ethtype == p_udf->ethtype)
                 && (udf_entry->ipproto == p_udf->ipproto))
                {
                    CFG_INVALID_PARAM_RET("Can not set same match field with UDF entry %d", udf_id);
                }
            }
        }

        return rc;
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_ETHTYPE))
    {
        if (udf_entry->ethtype < GLB_ACL_FIELD_ETHER_TYPE_MIN)
        {
            CFG_INVALID_PARAM_RET("Ether type min value is 0x600");
        }

        if (udf_entry->ethtype == 0x86dd && udf_entry->ethtype_mask == 0xffff)
        {
            CFG_PROMPT_RET("Notice: UDF match ipv6 packets not supported in flow");
        }
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO))
    {
        if (udf_entry->ipproto > 0xff)
        {
            CFG_INVALID_PARAM_RET("IP Protocol out of range");
        }

        if (GLB_ACL_L4_PROTOCOL_TCP == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_TCP;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
        else if (GLB_ACL_L4_PROTOCOL_UDP == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_UDP;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
        else if (GLB_ACL_L4_PROTOCOL_GRE == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_GRE;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
        else if (GLB_ACL_L4_PROTOCOL_SCTP == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_SCTP;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
        else if (GLB_ACL_L4_PROTOCOL_RDP == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_RDP;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
        else if (GLB_ACL_L4_PROTOCOL_DCCP == udf_entry->ipproto)
        {
            udf_entry->l4_type = GLB_ACL_UDF_L4_TYPE_DCCP;
            udf_entry->l4_type_mask = 0xff;
            GLB_SET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
            udf_entry->ipproto = 0;
            udf_entry->ipproto_mask = 0;
            GLB_UNSET_FLAG(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
        }
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_SRCPORT) 
     || GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_DSTPORT))
    {
        if (udf_entry->srcport > 0xffff || udf_entry->dstport > 0xffff)
        {
            CFG_CONFLICT_RET("L4 port out of range");
        }
        if (udf_entry->l4_type != GLB_ACL_UDF_L4_TYPE_UDP && udf_entry->l4_type != GLB_ACL_UDF_L4_TYPE_TCP)
        {
            CFG_CONFLICT_RET("L4 port field must configured TCP or UDP");
        }
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_VLANNUM))
    {
        if (udf_entry->vlannum > GLB_ACL_UDF_MATCH_VLANNUM_MAX)
        {
            CFG_INVALID_PARAM_RET("VLAN number max value is %d", GLB_ACL_UDF_MATCH_VLANNUM_MAX);
        }
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_MPLSNUM))
    {
        if (udf_entry->mplsnum > GLB_ACL_UDF_MATCH_MPLSNUM_MAX)
        {
            CFG_INVALID_PARAM_RET("MPLS label number max value is %d", GLB_ACL_UDF_MATCH_MPLSNUM_MAX);
        }
    }

    return rc;
}

static int32
acl_api_remove_udf_match(tbl_acl_udf_entry_t *p_udf_entry)
{
    int32 rc = PM_E_NONE;

    p_udf_entry->ethtype = 0;
    p_udf_entry->ethtype_mask = 0;
    p_udf_entry->vlannum = 0;
    p_udf_entry->vlannum_mask = 0;
    p_udf_entry->ipproto = 0;
    p_udf_entry->ipproto_mask = 0;
    p_udf_entry->l4_type = 0;
    p_udf_entry->l4_type_mask = 0;
    p_udf_entry->srcport = 0;
    p_udf_entry->srcport_mask = 0;
    p_udf_entry->dstport = 0;
    p_udf_entry->dstport_mask = 0;
    p_udf_entry->mplsnum = 0;
    p_udf_entry->mplsnum_mask = 0;
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_ETHTYPE));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_VLANNUM));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_IPPROTO));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_L4_TYPE));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_SRCPORT));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_DSTPORT));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_MPLSNUM));
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK));

    GLB_UNSET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_SRCPORT);
    GLB_UNSET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_DSTPORT);
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS));
    GLB_UNSET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS));
    p_udf_entry->key_flags = 0;
    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS));

    return rc;
}

static int32
acl_api_set_udf_match_field(tbl_acl_udf_entry_t *udf_entry, tbl_acl_udf_entry_t *p_udf_entry)
{
    int32 rc = PM_E_NONE;

    /* update udf match field, we need to remove all key field to avoid the field check problem */
    acl_api_remove_udf_match(p_udf_entry);

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_ETHTYPE))
    {
        p_udf_entry->ethtype = udf_entry->ethtype;
        p_udf_entry->ethtype_mask = udf_entry->ethtype_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_ETHTYPE));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_ETHTYPE);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_VLANNUM))
    {
        p_udf_entry->vlannum = udf_entry->vlannum;
        p_udf_entry->vlannum_mask = udf_entry->vlannum_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_VLANNUM));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_VLANNUM);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_IPPROTO))
    {
        p_udf_entry->ipproto = udf_entry->ipproto;
        p_udf_entry->ipproto_mask = udf_entry->ipproto_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_IPPROTO));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_IPPROTO);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE))
    {
        p_udf_entry->l4_type = udf_entry->l4_type;
        p_udf_entry->l4_type_mask = udf_entry->l4_type_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_L4_TYPE));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_SRCPORT))
    {
        p_udf_entry->srcport = udf_entry->srcport;
        p_udf_entry->srcport_mask = udf_entry->srcport_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_SRCPORT));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_SRCPORT);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_DSTPORT))
    {
        p_udf_entry->dstport = udf_entry->dstport;
        p_udf_entry->dstport_mask = udf_entry->dstport_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_DSTPORT));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_DSTPORT);
    }

    if (GLB_FLAG_ISSET(udf_entry->key_flags, GLB_ACL_UDF_MPLSNUM))
    {
        p_udf_entry->mplsnum = udf_entry->mplsnum;
        p_udf_entry->mplsnum_mask = udf_entry->mplsnum_mask;
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_MPLSNUM));
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK));
        GLB_SET_FLAG(p_udf_entry->key_flags, GLB_ACL_UDF_MPLSNUM);
    }

    PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS));

    if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        /* udf entry valid after match field set in GG */
        PM_E_RETURN(tbl_acl_udf_entry_set_acl_udf_entry_field(p_udf_entry, TBL_ACL_UDF_ENTRY_FLD_OFFSET));
    }
    return rc;
}

int32
acl_api_set_udf_match(tbl_acl_udf_entry_t *p_udf_entry, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint8 argc_idx = 0;
    tbl_acl_udf_entry_t udf_entry;

    sal_memset(&udf_entry, 0 ,sizeof(udf_entry));
    /* ether type */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ethtype");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_ETHTYPE);
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(udf_entry.ethtype, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(udf_entry.ethtype_mask, argv[argc_idx + 3], para->p_rs);
            udf_entry.ethtype_mask  = ~(udf_entry.ethtype_mask) & 0xffff;
            udf_entry.ethtype      &= udf_entry.ethtype_mask;
        }
    }
    /* ip protocol */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipproto");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (GLB_FLAG_ISSET(udf_entry.key_flags, GLB_ACL_UDF_ETHTYPE))
        {
            /* ether type not conflict with ip protocol in udf match */;
        }

        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_IPPROTO);
        udf_entry.ipproto_mask = 0xff;
        if (0 == sal_strcmp(argv[argc_idx + 1], "any"))
        {
            udf_entry.ipproto_mask = 0x0;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "tcp"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_TCP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udp"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_UDP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "icmp"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_ICMP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "igmp"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_IGMP;
        } 
        else if (0 == sal_strcmp(argv[argc_idx + 1], "gre"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_GRE;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "sctp"))
        {
            udf_entry.ipproto = GLB_ACL_L4_PROTOCOL_SCTP;
        }
        else
        {
            PM_CFG_GET_UINT(udf_entry.ipproto, argv[argc_idx + 2], para->p_rs);
        }
    }
    /* vlan number */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vlannum");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_VLANNUM);
        PM_CFG_GET_UINT(udf_entry.vlannum, argv[argc_idx + 1], para->p_rs);
        udf_entry.vlannum_mask = 0xff;
    }
    /* l4 src port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("srcport");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_SRCPORT);
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(udf_entry.srcport, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(udf_entry.srcport_mask, argv[argc_idx + 3], para->p_rs);
            udf_entry.srcport_mask  = ~(udf_entry.srcport_mask) & 0xffff;
        }
    }
    /* l4 dst port */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("dstport");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_DSTPORT);
        if (0 != sal_strcmp(argv[argc_idx + 1], "any"))
        {
            PM_CFG_GET_UINT(udf_entry.dstport, argv[argc_idx + 2], para->p_rs);
            PM_CFG_GET_UINT(udf_entry.dstport_mask, argv[argc_idx + 3], para->p_rs);
            udf_entry.dstport_mask  = ~(udf_entry.dstport_mask)  & 0xffff;
        }
    }
    /* mpls label number */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("mplsnum");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_FLAG(udf_entry.key_flags, GLB_ACL_UDF_MPLSNUM);
        PM_CFG_GET_UINT(udf_entry.mplsnum, argv[argc_idx + 1], para->p_rs);
        udf_entry.mplsnum_mask = 0xff;
    }

    PM_E_RETURN(acl_api_check_udf_match_field(&udf_entry, p_udf_entry, para));
    PM_E_RETURN(acl_api_set_udf_match_field(&udf_entry, p_udf_entry));
    
    return rc;
}

int32
acl_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_ACL_CONFIG, acl_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACE_CONFIG, ace_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACL_ENTRY, acl_entry_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_ACL_UDF_GROUP, acl_udf_group_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACL_UDF_ENTRY, acl_udf_entry_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_TIME_RANGE, acl_cmd_process_time_range);
    cdb_register_cfg_act_cb(ACT_CLOCK, acl_process_clock_change);
    acl_time_range_register_arg_invalid_cb(TIME_RANGE_ARG_TYPE_ACE, (TIME_RANGE_CB)acl_time_range_ace_invalid_cb);

    cdb_pm_subscribe_tbl(TBL_FEA_PTP_GLOBAL, NULL, acl_process_fea_ptp_sync_after_cb);
    
    acl_worm_filter_seq_opf = ctclib_opf_init(1);
    ctclib_opf_init_offset (acl_worm_filter_seq_opf, 0, 1, 0xff);
    cdb_register_cfg_tbl_cb(TBL_ACL_WORM_FILTER , acl_worm_filter_cmd_process);

    cdb_register_cfg_act_cb(ACT_SHOW_ACL, acl_cmd_show_acl);
    cdb_register_cfg_act_cb(ACT_SHOW_TIMERANGE_INFO, acl_cmd_show_timerange_info);
    cdb_register_cfg_act_cb(ACT_SHOW_ACL_UDF, acl_cmd_show_acl_udf);

    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_ACL, acl_agg_destroy);
    return PM_E_NONE;
}

#define _IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define IPV4_MASK_TO_LEN(mask, len) \
    { \
        int32 i; \
        len = 0; \
        for (i = 31; i >= 0; i--) \
        { \
            if (_IS_BIT_SET((mask), (uint32)i)) \
            {(len)++; } \
            else \
            {; } \
        } \
    }

static int32
mac_match_ipv4_mcast(tbl_ace_config_t *p_db_ace, prefix_t *p)
{
    uint32 macda_u32 = 0;

    if ((p_db_ace->dst_mac[0] != 0x01) && 
        (((p_db_ace->dst_mac[0]) & (~(p_db_ace->dst_mac_mask[0]))) != 0x01) &&
        (((p_db_ace->dst_mac[0]) & (~(p_db_ace->dst_mac_mask[0]))) != 0x00))
    {
        return FALSE;
    }

    if ((p_db_ace->dst_mac[1] != 0x00) && 
        (((p_db_ace->dst_mac[1]) & (~(p_db_ace->dst_mac_mask[1]))) != 0x00))
    {
        return FALSE;
    }

    if ((p_db_ace->dst_mac[2] != 0x5e) && 
        (((p_db_ace->dst_mac[2]) & (~(p_db_ace->dst_mac_mask[2]))) != 0x5e) &&
        (((p_db_ace->dst_mac[2]) & (~(p_db_ace->dst_mac_mask[2]))) != 0x00))
    {
        return FALSE;
    }

    macda_u32 = ((p_db_ace->dst_mac[2]) << 24) | ((p_db_ace->dst_mac[3]) << 16) | ((p_db_ace->dst_mac[4]) << 8) | (p_db_ace->dst_mac[5]);
    if ((macda_u32 & 0x7FFFFF) == (sal_hton32(p->u.prefix4.s_addr) & 0x7FFFFF))
    {
        return TRUE;
    }
    
    return FALSE;
}

static int32
_acl_get_ipv4_prefix(tbl_ace_config_t *p_db_ace, prefix_t *p, uint32 check_dest)
{
    addr_ipv4_t dst_ip_mask;
    uint32 dst_ip_mask_len = 0;
    addr_ipv4_t src_ip_mask;
    uint32 src_ip_mask_len = 0;

    dst_ip_mask.s_addr = ~(p_db_ace->dst_ip_mask.u.prefix4.s_addr);
    IPV4_MASK_TO_LEN(dst_ip_mask.s_addr, dst_ip_mask_len);
    src_ip_mask.s_addr = ~(p_db_ace->src_ip_mask.u.prefix4.s_addr);
    IPV4_MASK_TO_LEN(src_ip_mask.s_addr, src_ip_mask_len);

    sal_memset(p, 0, sizeof(prefix_t));
    p->family = AF_INET;
    if (check_dest)
    {
        p->prefixlen = dst_ip_mask_len;
        p->u.prefix4.s_addr = p_db_ace->dst_ip.u.prefix4.s_addr & dst_ip_mask.s_addr;
    }
    else
    {
        p->prefixlen = src_ip_mask_len;
        p->u.prefix4.s_addr = p_db_ace->src_ip.u.prefix4.s_addr & src_ip_mask.s_addr;
    }

    return PM_E_NONE;
}

int32
acl_prefix_check(char *acl_name, prefix_t *dest, prefix_t *src)
{
    tbl_acl_config_key_t acl_key;
    tbl_ace_config_master_t *p_ace_master = tbl_ace_config_get_master();
    tbl_acl_config_t *p_db_acl = NULL;
    tbl_ace_config_t *p_db_ace = NULL;    
    ctclib_slistnode_t *listnode = NULL;
    prefix_t check_prefix;
    uint32   is_da_match = FALSE;
    uint32   is_macda_match = FALSE;
    uint32   is_sa_match = FALSE;
    
    if (0 == sal_strlen(acl_name))
    {
        return PM_E_NONE;
    }

    sal_memset(&acl_key, 0, sizeof(acl_key));
    sal_strncpy(acl_key.name, acl_name, ACL_NAME_FULL_NAME_SIZE+1);

    p_db_acl = tbl_acl_config_get_acl_config(&acl_key);
    if (NULL == p_db_acl)
    {
        return PM_E_NONE;
    }

    if (GLB_ACL_CONFIG_TYPE_NORMAL != p_db_acl->type_identifying)
    {
        return PM_E_INVALID_PARAM;
    }

    CTCLIB_SLIST_LOOP(p_ace_master->ace_config_list, p_db_ace, listnode)
    {
        if (0 != sal_strcmp(p_db_ace->key.acl_name, acl_name))
        {
            continue;
        }

        /* any, matched */
        if (0 == p_db_ace->key_flags && 0 == p_db_ace->key_ipv6_flags)
        {
            is_da_match = TRUE;
            is_macda_match = TRUE;
            is_sa_match = TRUE;
            break;
        }

        /* if not any matched then must configure at least one of ipda/macda/ipsa */
        if (!GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPDA)
            && !GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_MACDA)
            && !GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPSA))
        {
            continue;
        }

        /* check matched IPDA,MACDA,IPSA respectively */
        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPDA))
        {
            _acl_get_ipv4_prefix(p_db_ace, &check_prefix, TRUE);
            if (sal_prefix_match(&check_prefix, dest))
            {
                is_da_match = TRUE;
            }
        }
        else
        {
            is_da_match = TRUE;
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_MAC_ACE_MACDA))
        {
            if (mac_match_ipv4_mcast(p_db_ace, dest))
            {
                is_macda_match = TRUE;
            }
        }
        else
        {
            is_macda_match = TRUE;
        }

        if (GLB_FLAG_ISSET(p_db_ace->key_flags, GLB_IP_ACE_IPSA))
        {
            _acl_get_ipv4_prefix(p_db_ace, &check_prefix, FALSE);
            if (sal_prefix_match(&check_prefix, src))
            {
                is_sa_match = TRUE;
            }
        }
        else
        {
            is_sa_match = TRUE;
        }

        if (is_da_match && is_macda_match && is_sa_match && p_db_ace)
        {
            break;
        }
        /* other ACE, omit */
    }

    if (NULL != p_db_ace && p_db_ace->deny)
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}


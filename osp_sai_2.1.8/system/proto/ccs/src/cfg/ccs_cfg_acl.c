#include "proto.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "lib_acl.h"

int32
_ccs_cfg_build_ace_config(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    char *acl_name = pargs->argv[1];
#ifdef TAPPRODUCT
    tbl_acl_config_t *p_acl_config = pargs->argv[2];
#endif
    char buf1[256];
    char buf2[256];
    char buf3[256];
    uint8 flag = 0;

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
#ifdef USW
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

        if(p_ace_config->tap_action_edit_ipda_en)
        {
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

        if(p_ace_config->tap_action_edit_ipsa_en)
        {
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
            sal_fprintf (cfg_fp, "\n");
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

        /*modified by yejl for bug 49768, 2018-10-31*/
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
        else if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->src_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->src_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->src_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
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
                sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2); /*modified by yejl for bug 50230, 2018-12-06*/
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
        else if (GLB_FLAG_ISSET(p_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_ace_config->dst_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_ace_config->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
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
                sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2); /*modified by yejl for bug 50230, 2018-12-06*/
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
        }
#endif
    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}

int32
_ccs_cfg_build_acl_config_one(FILE* fp, tbl_acl_config_t *p_acl_config)
{
    FILE* cfg_fp = fp;
    tbl_iter_args_t args;

    if (NULL == p_acl_config || NULL == fp)
        return PM_E_NONE;

    sal_fprintf (cfg_fp, "%s %s%s\n", 
                    glb_acl_show_running_str[p_acl_config->type_identifying],
                    acl_get_acl_config_key_name(p_acl_config->key.name,p_acl_config->type_identifying),
                    p_acl_config->decap_en ? " type decap":"");

    if (0 != sal_strlen(p_acl_config->remark))
    {
        sal_fprintf (cfg_fp, " remark %s\n", p_acl_config->remark);
    }

    if (0 == p_acl_config->ace_ref && 0 == p_acl_config->acev6_ref && 0 == p_acl_config->acemac_ref)
    {
        /*modified by yejl for bug 49922, 2018-11-12*/
        sal_fprintf (cfg_fp, " exit\n");
        sal_fprintf (cfg_fp, "!\n");
    }
    else
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_acl_config->key.name);
        args.argv[2] = (void *)p_acl_config;
        tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ace_config, &args);
        sal_fprintf (cfg_fp, " exit\n"); /*modified by yejl for bug 49922, 2018-11-12*/
        sal_fprintf (cfg_fp, "!\n");
    }

    return PM_E_NONE;
}

int32
_ccs_cfg_build_service_acl(FILE* fp)
{
    uint32 has_config = FALSE;
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();

    if (NULL == p_sys_glb)
    {
        return PM_E_NONE;
    }

    if (0 != sal_strlen(p_sys_glb->telnet_acl))
    {
        sal_fprintf(fp, "service telnet acl %s\n", p_sys_glb->telnet_acl);
        has_config = TRUE;
    }

    if (sal_strlen(p_sys_glb->ssh_acl))
    {
        sal_fprintf(fp, "ip ssh server acl %s\n", p_sys_glb->ssh_acl);
        has_config = TRUE;
    }

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_acl(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    tbl_acl_config_master_t *p_master = tbl_acl_config_get_master();
    tbl_acl_config_key_t acl_name;
    tbl_acl_config_t *p_acl_config = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 idx = 0;
        
    if (key)
    {
        sal_memset(&acl_name, 0, sizeof(acl_name));
        sal_strncpy(acl_name.name, key, ACL_NAME_FULL_NAME_SIZE);
        p_acl_config = tbl_acl_config_get_acl_config(&acl_name);
        if (NULL == p_acl_config)
        {
#ifdef TAPPRODUCT
            CFG_CONFLICT_RET("Flow %s does not exist", key);  
#else
            CFG_CONFLICT_RET("IP access-list %s does not exist", key);
#endif
        }
        _ccs_cfg_build_acl_config_one(fp, p_acl_config);
    }
    else
    {
        for(idx = 0; idx < GLB_ACL_CONFIG_TYPE_MAX; idx++)
        { 
            CTCLIB_SLIST_LOOP(p_master->acl_config_list, p_acl_config, listnode)
            {
                if(p_acl_config->type_identifying != idx)
                    continue;
                
                _ccs_cfg_build_acl_config_one(fp, p_acl_config);
            }
        }

        _ccs_cfg_build_service_acl(fp);
    }
    
    return PM_E_NONE;
}

int32
_ccs_cfg_build_acl_udf_one(FILE* fp, tbl_acl_udf_entry_t *p_db_udf)
{
    FILE* cfg_fp = fp;
    uint32 iter = 0;

    if (NULL == p_db_udf || NULL == fp)
        return PM_E_NONE;

    if (GLB_ACL_UDF_OFFSET_L2 == p_db_udf->offset_type)
    {
        sal_fprintf (cfg_fp, "udf %d offset-type l2-header\n", p_db_udf->key.id);
    }
    else if (GLB_ACL_UDF_OFFSET_L3 == p_db_udf->offset_type)
    {
        sal_fprintf (cfg_fp, "udf %d offset-type l3-header\n", p_db_udf->key.id);
    }
    else if (GLB_ACL_UDF_OFFSET_L4 == p_db_udf->offset_type)
    {
        sal_fprintf (cfg_fp, "udf %d offset-type l4-header\n", p_db_udf->key.id);
    }

    if (p_db_udf->key_flags)
    {
        sal_fprintf(fp, INTEND_1_STR"match ");
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_ETHTYPE))
        {
            sal_fprintf(fp, "ether-type 0x%x 0x%x ", p_db_udf->ethtype, (~p_db_udf->ethtype_mask) & 0xffff);
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_VLANNUM))
        {
            sal_fprintf(fp, "vlan-num %d ", p_db_udf->vlannum);
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_IPPROTO))
        {
            sal_fprintf(fp, "ip-protocol ");
            if (0 == p_db_udf->ipproto_mask)
            {
                sal_fprintf (fp, "any ");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "icmp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_IGMP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "igmp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_SCTP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "sctp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_TCP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "tcp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_UDP == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "udp ");
            }
            else if (GLB_ACL_L4_PROTOCOL_GRE == p_db_udf->ipproto)
            {
                sal_fprintf (fp, "gre ");
            }
            else
            {
                sal_fprintf(fp, "%d ", p_db_udf->ipproto);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_L4_TYPE))
        {
            sal_fprintf(fp, "ip-protocol ");
            if (GLB_ACL_UDF_L4_TYPE_TCP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "tcp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_UDP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "udp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_GRE == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "gre ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_SCTP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "sctp ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_RDP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "27 ");
            }
            else if (GLB_ACL_UDF_L4_TYPE_DCCP == p_db_udf->l4_type)
            {
                sal_fprintf (fp, "33 ");
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_SRCPORT))
        {
            if (0 == p_db_udf->srcport_mask)
            {
                sal_fprintf(fp, "src-port any ");
            }
            else
            {
                sal_fprintf(fp, "src-port %d ", p_db_udf->srcport);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_DSTPORT))
        {
             if (0 == p_db_udf->dstport_mask)
            {
                sal_fprintf(fp, "dst-port any ");
            }
            else
            {
                sal_fprintf(fp, "dst-port %d ", p_db_udf->dstport);
            }
        }
        if (GLB_FLAG_ISSET(p_db_udf->key_flags, GLB_ACL_UDF_MPLSNUM))
        {
            sal_fprintf(fp, "mpls-label-num %d ", p_db_udf->mplsnum);
        }
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, INTEND_1_STR"match any\n");
    }

    if (p_db_udf->valid[0] || p_db_udf->valid[1] || p_db_udf->valid[2] || p_db_udf->valid[3])
    {
        sal_fprintf (cfg_fp, INTEND_1_STR"offset ");

        for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
        {
            if (p_db_udf->valid[iter])
            {
                sal_fprintf (cfg_fp, "offset%d %d ", iter, p_db_udf->offset[iter]);
            }
        }
        sal_fprintf (cfg_fp, "\n");
    }

    sal_fprintf (cfg_fp, "!\n");

    return PM_E_NONE;

}

int32
ccs_cfg_build_acl_udf(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    uint32  udf_id = 0;
    tbl_acl_udf_entry_master_t *p_master = NULL;
    tbl_acl_udf_entry_t        *p_db_udf = NULL;

    p_master = tbl_acl_udf_entry_get_master();
    if (!p_master)
    {
        return PM_E_NONE;
    }

    for (udf_id = 0; udf_id < GLB_ACL_UDF_ENTRY_NUM_MAX; udf_id++)
    {
        p_db_udf = p_master->acl_udf_entry_array[udf_id];
        if (!p_db_udf)
        {
            continue;
        }

        _ccs_cfg_build_acl_udf_one(fp, p_db_udf);
    }
  
    return PM_E_NONE;
}


int32
ccs_cfg_build_time_range(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_time_range_master_t *p_master = tbl_time_range_get_master();
    tbl_time_range_t *p_db_tr = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char buf[CMD_BUF_256];

    buf[0] = CMD_ZERO_CHAR;
    CTCLIB_SLIST_LOOP(p_master->time_range_list, p_db_tr, listnode)
    {
        sal_fprintf(fp, "time-range %s\n", p_db_tr->key.name);
        sal_fprintf(fp, "%s\n", cdb_time_range_value_val2str(buf, CMD_BUF_256, &p_db_tr->time));
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}



#include "ctc_api.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_chip.h"
#include "hagt_openflow.h"
#include "glb_l2_define.h"

extern hagt_master_t *g_pst_hagt_master;

extern uint32 hagt_ipuc_get_ipv6_enabled();

int32 g_mirror_session_ref = 0;
uint32 g_mirror_session_dest_port = 0;
uint32 g_truncate_len = 0;

uint32 hagt_openflow_udf_enable()
{
    return HAGT_OPENFLOW_FLOW_UDF_ENABLE;
}

#define _____INTERNAL_API_____

extern void ctc_sai_acl_alloc_entry_id(uint32* p_index);

extern void ctc_sai_acl_release_entry_id(uint32 index);

extern uint32 hagt_openflow_merter_get_ctc_policer_id(uint32 openflow_meter_id);

extern int32 hsrv_vlan_add_ports_to_vlan();

uint32
hagt_openflow_flow_get_ctc_entry_id(uint32 openflow_entry_id)
{
    hagt_openflow_entryid_t* p_openflow_entryid = NULL;
    hagt_openflow_entryid_t openflow_entryid;

    sal_memset(&openflow_entryid, 0x0, sizeof(hagt_openflow_entryid_t));
    openflow_entryid.entry_id= openflow_entry_id;
    p_openflow_entryid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, &openflow_entryid);
    if(p_openflow_entryid)
    {
        return p_openflow_entryid->ctc_entry_id;
    }

    return 0;
}

static int32
hagt_openflow_update_truncate(uint32 session_id, bool is_add, uint32 truncate_len, uint32 dest_port)
{
    ctc_qos_glb_cfg_t glb_cfg;
    ctc_mirror_dest_t st_dest;
    int32 ret = 0;

    sal_memset(&st_dest, 0, sizeof(ctc_mirror_dest_t));
    st_dest.session_id = session_id;
    st_dest.type = CTC_MIRROR_ACLLOG_SESSION;
    
    if (is_add)
    {
        if (0 == g_mirror_session_ref)
        {
            st_dest.dest_gport = dest_port;
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.acl_priority = 0;
            ret = ctc_mirror_add_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 1;
            ret = ctc_mirror_add_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 2;
            ret = ctc_mirror_add_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 3;
            ret = ctc_mirror_add_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }
            g_mirror_session_dest_port = dest_port;

            /* set truncation length and exception */
            sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

            glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TRUNCATION_LEN;
            glb_cfg.u.value  = truncate_len;
            ret = ctc_qos_set_global_config(&glb_cfg);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }
        }
        g_mirror_session_ref++;
        if (dest_port != g_mirror_session_dest_port)
        {
            HAGT_LOG_ERR("Openflow truncate port %d does not match saved port %d", dest_port, g_mirror_session_dest_port);
        }
    }
    else
    {
        g_mirror_session_ref--;
        if (g_mirror_session_ref <= 0)
        {
            g_mirror_session_ref = 0;
            g_mirror_session_dest_port = 0;

            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.acl_priority = 0;
            ret = ctc_mirror_remove_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 1;
            ret = ctc_mirror_remove_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 2;
            ret = ctc_mirror_remove_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }

            st_dest.dir = CTC_INGRESS;
            st_dest.acl_priority = 3;
            ret = ctc_mirror_remove_session(&st_dest);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }
        }
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mac(HalMsgOpenFlowMacRule_t* p_mac, ctc_acl_ipv4_key_t* p_ipv4_key)
{
    uint8 i = 0;
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_mac->macSaMask.size; i++)
        {
            p_ipv4_key->mac_sa[i] = p_mac->macSa.buf[i];
            p_ipv4_key->mac_sa_mask[i] = p_mac->macSaMask.buf[i];
        }
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_mac->macDaMask.size; i++)
        {
            p_ipv4_key->mac_da[i] = p_mac->macDa.buf[i];
            p_ipv4_key->mac_da_mask[i] = p_mac->macDaMask.buf[i];
        }
    }


    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);
        p_ipv4_key->stag_valid = p_mac->sTagValid;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
        p_ipv4_key->svlan = p_mac->sVlanId;
        p_ipv4_key->svlan_mask = p_mac->sVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS);
        p_ipv4_key->stag_cos = p_mac->sTagCos;
        p_ipv4_key->stag_cos_mask = p_mac->sTagCosMask;
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
        p_ipv4_key->eth_type = p_mac->ethType;
        p_ipv4_key->eth_type_mask = p_mac->ethTypeMask;
    }
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);
        p_ipv4_key->ctag_valid = p_mac->cTagValid;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN);
        p_ipv4_key->cvlan = p_mac->cVlanId;
        p_ipv4_key->cvlan_mask = p_mac->cVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS);
        p_ipv4_key->ctag_cos = p_mac->cTagCos;
        p_ipv4_key->ctag_cos_mask = p_mac->cTagCosMask;
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mpls_mac(HalMsgOpenFlowMacRule_t* p_mac, ctc_acl_mpls_key_t* p_mpls_key)
{
    uint8 i;
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_mac->macSaMask.size; i++)
        {
            p_mpls_key->mac_sa[i] = p_mac->macSa.buf[i];
            p_mpls_key->mac_sa_mask[i] = p_mac->macSaMask.buf[i];
        }
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_mac->macDaMask.size; i++)
        {
            p_mpls_key->mac_da[i] = p_mac->macDa.buf[i];
            p_mpls_key->mac_da_mask[i] = p_mac->macDaMask.buf[i];
        }
    }


    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID))
    {
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_SVLAN);
        p_mpls_key->svlan = p_mac->sVlanId;
        p_mpls_key->svlan_mask = p_mac->sVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_STAG_COS);
        p_mpls_key->stag_cos = p_mac->sTagCos;
        p_mpls_key->stag_cos_mask = p_mac->sTagCosMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID))
    {
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_CVLAN);
        p_mpls_key->cvlan = p_mac->cVlanId;
        p_mpls_key->cvlan_mask = p_mac->cVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_CTAG_COS);
        p_mpls_key->ctag_cos = p_mac->cTagCos;
        p_mpls_key->ctag_cos_mask = p_mac->cTagCosMask;
    }

    return HAGT_E_NONE;
}
static int32
hagt_openflow_map_mac_ipv6(HalMsgOpenFlowMacRule_t* p_mac, ctc_acl_ipv6_key_t* p_ipv6_key)
{
    uint8 i;

    GLB_UNSET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE);
    p_ipv6_key->l3_type = 0;
    p_ipv6_key->l3_type_mask = 0;
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA);
        for (i = 0; i < p_mac->macSaMask.size; i++)
        {
            p_ipv6_key->mac_sa[i] = p_mac->macSa.buf[i];
            p_ipv6_key->mac_sa_mask[i] = p_mac->macSaMask.buf[i];
        }
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA);
        for (i = 0; i < p_mac->macDaMask.size; i++)
        {
            p_ipv6_key->mac_da[i] = p_mac->macDa.buf[i];
            p_ipv6_key->mac_da_mask[i] = p_mac->macDaMask.buf[i];
        }
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);
        p_ipv6_key->stag_valid = p_mac->sTagValid;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
        p_ipv6_key->svlan = p_mac->sVlanId;
        p_ipv6_key->svlan_mask = p_mac->sVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS);
        p_ipv6_key->stag_cos = p_mac->sTagCos;
        p_ipv6_key->stag_cos_mask = p_mac->sTagCosMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_ETH_TYPE);
        p_ipv6_key->eth_type = p_mac->ethType;
        p_ipv6_key->eth_type_mask = p_mac->ethTypeMask;
        p_ipv6_key->l3_type = CTC_PARSER_L3_TYPE_NONE;
        p_ipv6_key->l3_type_mask = 0xF;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);
        p_ipv6_key->ctag_valid = p_mac->cTagValid;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN);
        p_ipv6_key->cvlan = p_mac->cVlanId;
        p_ipv6_key->cvlan_mask = p_mac->cVlanIdMask;
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS);
        p_ipv6_key->ctag_cos = p_mac->cTagCos;
        p_ipv6_key->ctag_cos_mask = p_mac->cTagCosMask;
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_arp(HalMsgOpenFlowArpRule_t* p_arp, ctc_acl_ipv4_key_t* p_ipv4_key)
{

    GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET);
    p_ipv4_key->arp_packet = 1;

    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_ARP_OPCODE))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE);
        p_ipv4_key->arp_op_code = p_arp->arpOpCode;
        p_ipv4_key->arp_op_code_mask = p_arp->arpOpCodeMask;
    }

    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_SENDER_IP))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP);
        p_ipv4_key->sender_ip = p_arp->senderIp;
        p_ipv4_key->sender_ip_mask = p_arp->senderIpMask;
    }

    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_TARGET_IP))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP);
        p_ipv4_key->target_ip = p_arp->targetIp;
        p_ipv4_key->target_ip_mask = p_arp->targetIpMask;
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_ipv4(HalMsgOpenFlowIpv4Rule_t* p_ipv4, ctc_acl_ipv4_key_t* p_ipv4_key)
{
    GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
    p_ipv4_key->ipv4_packet = 1;
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA);
        p_ipv4_key->ip_da = p_ipv4->ipDa;
        p_ipv4_key->ip_da_mask = p_ipv4->ipDaMask;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);
        p_ipv4_key->ip_sa = p_ipv4->ipSa;
        p_ipv4_key->ip_sa_mask = p_ipv4->ipSaMask;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL);
        p_ipv4_key->l4_protocol = p_ipv4->l4Protocol;
        p_ipv4_key->l4_protocol_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_DSCP);
        p_ipv4_key->dscp = p_ipv4->dscp;
        p_ipv4_key->dscp_mask = p_ipv4->dscpMask;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ECN))
    {
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_SRC_PORT))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
        p_ipv4_key->l4_src_port_0 = p_ipv4->l4SrcPort.port;
        p_ipv4_key->l4_src_port_1 = p_ipv4->l4SrcPort.portMask;
        p_ipv4_key->l4_src_port_use_mask = 1;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_DST_PORT))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
        p_ipv4_key->l4_dst_port_0 = p_ipv4->l4DestPort.port;
        p_ipv4_key->l4_dst_port_1 = p_ipv4->l4DestPort.portMask;
        p_ipv4_key->l4_dst_port_use_mask = 1;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_TYPE))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE);
        p_ipv4_key->icmp_type = p_ipv4->icmpType;
        p_ipv4_key->icmp_type_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_CODE))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE);
        p_ipv4_key->icmp_code = p_ipv4->icmpCode;
        p_ipv4_key->icmp_code_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IGMP_TYPE))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE);
        p_ipv4_key->igmp_type = p_ipv4->igmpType;
        p_ipv4_key->igmp_type_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_TCP_FLAG))
    {
        GLB_SET_FLAG(p_ipv4_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS);
        p_ipv4_key->tcp_flags = p_ipv4->tcpFlags;
        p_ipv4_key->tcp_flags_match_any = 0;
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG);
        if ((p_ipv4->ipFragMask & 0x3) == 0x3)
        {
            if ((p_ipv4->ipFrag & 0x3) == 0x3)
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_NOT_FIRST;
            }
            else if ((p_ipv4->ipFrag & 0x3) == 0x2)
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_SMALL;
            }
            else if ((p_ipv4->ipFrag & 0x3) == 0x1)
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_FIRST;
            }
            else
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_NON;
            }
        }
        else if ((p_ipv4->ipFragMask & 0x3) == 0x2)
        {
            if ((p_ipv4->ipFrag & 0x3) == 0x1)
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_NOT_FIRST;
            }
            else
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_NON_OR_FIRST;
            }
        }
        else if ((p_ipv4->ipFragMask & 0x3) == 0x1)
        {
            if ((p_ipv4->ipFrag & 0x3) == 0x1)
            {
                /* for bug 50835 support nw_frag=yes */
                p_ipv4_key->ip_frag = CTC_IP_FRAG_YES;
            }
            else
            {
                p_ipv4_key->ip_frag = CTC_IP_FRAG_NON;
            }
        }
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_ipv6(HalMsgOpenFlowIpv6Rule_t* p_ipv6, ctc_acl_ipv6_key_t* p_ipv6_key)
{
    GLB_UNSET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE);
    p_ipv6_key->l3_type = 0;
    p_ipv6_key->l3_type_mask = 0;

    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPDA))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA);
        sal_memcpy(p_ipv6_key->ip_da, p_ipv6->dprefix.buf, sizeof(ipv6_addr_t));
        sal_memcpy(p_ipv6_key->ip_da_mask, p_ipv6->dprefixMask.buf, sizeof(ipv6_addr_t));

    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPSA))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA);
        sal_memcpy(p_ipv6_key->ip_sa, p_ipv6->sprefix.buf, sizeof(ipv6_addr_t));
        sal_memcpy(p_ipv6_key->ip_sa_mask, p_ipv6->sprefixMask.buf, sizeof(ipv6_addr_t));

    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_PROTO))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL);
        p_ipv6_key->l4_protocol = p_ipv6->l4Protocol;
        p_ipv6_key->l4_protocol_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_DSCP))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_DSCP);
        p_ipv6_key->dscp = p_ipv6->dscp;
        p_ipv6_key->dscp_mask = p_ipv6->dscpMask;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ECN))
    {
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_SRC_PORT))
    {
        GLB_SET_FLAG(p_ipv6_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
        p_ipv6_key->l4_src_port_0 = p_ipv6->l4SrcPort.port;
        p_ipv6_key->l4_src_port_1 = p_ipv6->l4SrcPort.portMask;
        p_ipv6_key->l4_src_port_use_mask = 1;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_DST_PORT))
    {
        GLB_SET_FLAG(p_ipv6_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
        p_ipv6_key->l4_dst_port_0 = p_ipv6->l4DestPort.port;
        p_ipv6_key->l4_dst_port_1 = p_ipv6->l4DestPort.portMask;
        p_ipv6_key->l4_dst_port_use_mask = 1;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_TYPE))
    {
        GLB_SET_FLAG(p_ipv6_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_TYPE);
        p_ipv6_key->icmp_type = p_ipv6->icmpType;
        p_ipv6_key->icmp_type_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_CODE))
    {
        GLB_SET_FLAG(p_ipv6_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_CODE);
        p_ipv6_key->icmp_code = p_ipv6->icmpCode;
        p_ipv6_key->icmp_code_mask = 0xFF;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_FLOW_LABEL))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL);
        p_ipv6_key->flow_label = p_ipv6->flowLabel;
        p_ipv6_key->flow_label_mask = p_ipv6->flowLabelMask;
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IP_FRAG))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_IP_FRAG);
        if ((p_ipv6->ipFragMask & 0x3) == 0x3)
        {
            if ((p_ipv6->ipFrag & 0x3) == 0x3)
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_NOT_FIRST;
            }
            else if ((p_ipv6->ipFrag & 0x3) == 0x2)
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_SMALL;
            }
            else if ((p_ipv6->ipFrag & 0x3) == 0x1)
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_FIRST;
            }
            else
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_NON;
            }
        }
        else if ((p_ipv6->ipFragMask & 0x3) == 0x2)
        {
            if ((p_ipv6->ipFrag & 0x3) == 0x1)
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_NOT_FIRST;
            }
            else
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_NON_OR_FIRST;
            }
        }
        else if ((p_ipv6->ipFragMask & 0x3) == 0x1)
        {
            if ((p_ipv6->ipFrag & 0x3) == 0x1)
            {
                //TODO: sdk will add this later
                //p_ipv4_key->ip_frag == yes
            }
            else
            {
                p_ipv6_key->ip_frag = CTC_IP_FRAG_NON;
            }
        }
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mpls(HalMsgOpenFlowMplsRule_t* p_mpls, ctc_acl_mpls_key_t* p_mpls_key)
{

    if (p_mpls->mplsLabel0Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL0);
        p_mpls_key->mpls_label0 |= (uint32)p_mpls->mplsLabel0;
        p_mpls_key->mpls_label0_mask |= GLB_MPLS_LABEL_MASK;
    }

    if (p_mpls->mplsLabel1Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL1);
        p_mpls_key->mpls_label1 |= (uint32)p_mpls->mplsLabel1;
        p_mpls_key->mpls_label1_mask |= GLB_MPLS_LABEL_MASK;
    }
    
    if (p_mpls->mplsLabel2Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL2);
        p_mpls_key->mpls_label2 |= (uint32)p_mpls->mplsLabel2;
        p_mpls_key->mpls_label2_mask |= GLB_MPLS_LABEL_MASK;
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mpls_mcast(HalMsgOpenFlowMplsRule_t* p_mpls, ctc_acl_mpls_key_t* p_mpls_key)
{

    if (p_mpls->mplsLabel0Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL0);
        p_mpls_key->mpls_label0 |= (uint32)p_mpls->mplsLabel0;
        p_mpls_key->mpls_label0_mask |= GLB_MPLS_LABEL_MASK;
    }

    if (p_mpls->mplsLabel1Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL1);
        p_mpls_key->mpls_label1 |= (uint32)p_mpls->mplsLabel1;
        p_mpls_key->mpls_label1_mask |= GLB_MPLS_LABEL_MASK;
    }
    
    if (p_mpls->mplsLabel2Mask & GLB_MPLS_LABEL_MASK)
    {
        GLB_SET_FLAG(p_mpls_key->flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL2);
        p_mpls_key->mpls_label2 |= (uint32)p_mpls->mplsLabel2;
        p_mpls_key->mpls_label2_mask |= GLB_MPLS_LABEL_MASK;
    }
    
    return HAGT_E_NONE;
}

#if 0
/* Added by weizj for udf */
static int32
hagt_openflow_map_ip_protocol(uint16 *result, uint16 protocol)
{
#define PROTOCOL_ICMP 1
#define PROTOCOL_IGMP 2
#define PROTOCOL_TCP  6
#define PROTOCOL_UDP  17
#define PROTOCOL_GRE  47
#define PROTOCOL_SCTP 132

    switch (protocol)
    {
        case PROTOCOL_ICMP:
            *result = CTC_PARSER_L4_TYPE_ICMP;
            break;
        case PROTOCOL_IGMP:
            *result = CTC_PARSER_L4_TYPE_IGMP;
            break;
        case PROTOCOL_TCP:
            *result = CTC_PARSER_L4_TYPE_TCP;
            break;
        case PROTOCOL_UDP:
            *result = CTC_PARSER_L4_TYPE_UDP;
            break;
        case PROTOCOL_GRE:
            *result = CTC_PARSER_L4_TYPE_GRE;
            break;
        case PROTOCOL_SCTP:
            *result = CTC_PARSER_L4_TYPE_SCTP;
            break;
        default:
            *result = CTC_PARSER_L4_TYPE_ANY_PROTO;
    }
    
    return HAGT_E_NONE;
}
/* End by weizj for udf */


/* Modifed by weizj for tunnel bug 33692 */
static int32
hagt_openflow_map_tnl(HalMsgOpenFlowTnlRule_t* p_tnl, ctc_acl_ipv4_key_t* p_ipv4_key)
{
    /* Deleted by weizj for tunnel bug 33949 */
    if (GLB_FLAG_ISSET(p_tnl->flags, GLB_OPENFLOW_TNL_RULE_FLAG_METADATA))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_METADATA);
        p_ipv4_key->metadata = p_tnl->metadata;
        p_ipv4_key->metadata_mask = 0x3FFF;
    }
    
    return HAGT_E_NONE;
}

/* Added by weizj for udf */
static int32
hagt_openflow_map_udf(HalMsgOpenFlowUdfRule_t* p_udf, ctc_acl_ipv4_key_t* p_ipv4_key)
{
    uint8 match_udf = 0;

    if (!p_udf->enable)
    {
        return HAGT_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_udf->flags, GLB_OPENFLOW_UDF_TYPE_L3))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_UDF);
        p_ipv4_key->udf_type = CTC_PARSER_UDF_TYPE_L3_UDF;
        match_udf = 1;
    }

    if (GLB_FLAG_ISSET(p_udf->flags, GLB_OPENFLOW_UDF_TYPE_L4))
    {
        GLB_SET_FLAG(p_ipv4_key->flag, CTC_ACL_IPV4_KEY_FLAG_UDF);
        p_ipv4_key->udf_type = CTC_PARSER_UDF_TYPE_L4_UDF;
        match_udf = 1;
    }

    if (match_udf)
    {
        /* Modifed by weizj for udf bug 34584 */
        p_ipv4_key->udf[0] = (p_udf->udf >> 24) & 0xff;
        p_ipv4_key->udf[1] = (p_udf->udf >> 16) & 0xff;
        p_ipv4_key->udf[2] = (p_udf->udf >> 8) & 0xff;
        p_ipv4_key->udf[3] = (p_udf->udf) & 0xff;
        p_ipv4_key->udf_mask[0] = (p_udf->udfMask >> 24) & 0xff;
        p_ipv4_key->udf_mask[1] = (p_udf->udfMask >> 16) & 0xff;
        p_ipv4_key->udf_mask[2] = (p_udf->udfMask >> 8) & 0xff;
        p_ipv4_key->udf_mask[3] = (p_udf->udfMask) & 0xff;
    }
    
    return HAGT_E_NONE;
}
#endif

#if 0
static int32
hagt_openflow_map_ipv6_udf(HalMsgOpenFlowUdfRule_t* p_udf, ctc_acl_ipv6_key_t* p_ipv6_key)
{
    uint8 match_udf = 0;

    if (!p_udf->enable)
    {
        return HAGT_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_udf->flags, GLB_OPENFLOW_UDF_TYPE_L3))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_UDF);
        p_ipv6_key->udf_type = CTC_PARSER_UDF_TYPE_L3_UDF;
        match_udf = 1;
    }

    if (GLB_FLAG_ISSET(p_udf->flags, GLB_OPENFLOW_UDF_TYPE_L4))
    {
        GLB_SET_FLAG(p_ipv6_key->flag, CTC_ACL_IPV6_KEY_FLAG_UDF);
        p_ipv6_key->udf_type = CTC_PARSER_UDF_TYPE_L4_UDF;
        match_udf = 1;
    }

    if (match_udf)
    {
        /* Modifed by weizj for udf bug 34584 */
        p_ipv6_key->udf[0] = (p_udf->udf >> 24) & 0xff;
        p_ipv6_key->udf[1] = (p_udf->udf >> 16) & 0xff;
        p_ipv6_key->udf[2] = (p_udf->udf >> 8) & 0xff;
        p_ipv6_key->udf[3] = (p_udf->udf) & 0xff;
        p_ipv6_key->udf_mask[0] = (p_udf->udfMask >> 24) & 0xff;
        p_ipv6_key->udf_mask[1] = (p_udf->udfMask >> 16) & 0xff;
        p_ipv6_key->udf_mask[2] = (p_udf->udfMask >> 8) & 0xff;
        p_ipv6_key->udf_mask[3] = (p_udf->udfMask) & 0xff;
    }
    
    return HAGT_E_NONE;
}
#endif

#if 0
static int32
hagt_openflow_set_metadata_flag(uint32 udf_enable, ctc_acl_key_t* p_acl_key, bool mask_en)
{
    switch (p_acl_key->type)
    {
        case CTC_ACL_KEY_IPV4:
            /* Modifed by weizj for udf */
            if (!udf_enable)
            {
                CTC_SET_FLAG(p_acl_key->u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_METADATA);
                p_acl_key->u.ipv4_key.metadata = 0;
                if (mask_en)
                {
                    p_acl_key->u.ipv4_key.metadata_mask = 0x3FFF;
                }   
            }
            break;
        default:
            return HAGT_E_NONE;
    }

    return HAGT_E_NONE;
}
/* End by weizj for tunnel */
#endif

static int32
hagt_openflow_map_flow_key(uint32 priority, HalMsgOpenFlowRule_t* p_rule, ctc_acl_key_t* p_acl_key)
{
    p_acl_key->type = CTC_ACL_KEY_IPV4;

    switch(p_rule->unionRule.present)
    {
    case unionRule_PR_NOTHING:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, &p_acl_key->u.ipv4_key));
        break;

    case unionRule_PR_arpRule:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, &p_acl_key->u.ipv4_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_arp(&p_rule->unionRule.arpRule, &p_acl_key->u.ipv4_key));
        break;

    case unionRule_PR_ipv4Rule:
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, &p_acl_key->u.ipv4_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv4(&p_rule->unionRule.ipv4Rule, &p_acl_key->u.ipv4_key));
        break;

    case unionRule_PR_ipv6Rule:
        /* ipv6 parse disable, we only use ether_type(0x86dd) and l3_type = 0 (not ipv6)*/
        /* HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac_ipv6(&p_rule->macRule, &p_acl_key->u.ipv6_key)); */
        /* HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv6(&p_rule->unionRule.ipv6Rule, &p_acl_key->u.ipv6_key)); */
        /* Modifed by weizj for tunnel bug 33692 */
        p_acl_key->type = CTC_ACL_KEY_IPV6;
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac_ipv6(&p_rule->macRule, &p_acl_key->u.ipv6_key));
        //HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, &p_acl_key->u.ipv6_key));
        //HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv6_udf(&p_rule->udfRule, &p_acl_key->u.ipv6_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv6(&p_rule->unionRule.ipv6Rule, &p_acl_key->u.ipv6_key));
        break;

    case unionRule_PR_mplsRule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        p_acl_key->type = CTC_ACL_KEY_MPLS;
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls_mac(&p_rule->macRule, &p_acl_key->u.mpls_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls(&p_rule->unionRule.mplsRule, &p_acl_key->u.mpls_key));
        break;

    case unionRule_PR_mplsMcastRule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        p_acl_key->type = CTC_ACL_KEY_MPLS;
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls_mac(&p_rule->macRule, &p_acl_key->u.mpls_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls_mcast(&p_rule->unionRule.mplsRule, &p_acl_key->u.mpls_key));
        break;

    default:
        break;
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_flow_action(HalMsgOpenFlowAction_t* p_action, ctc_acl_action_t* p_acl_action)
{
    uint32 ctc_policer_id = 0;
    if (p_action->statsEn)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_STATS);
        p_acl_action->stats_id = hagt_openflow_stats_get_ctc_stats_id(p_action->statsId);
    }
    if (p_action->priorityValid)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR);
        p_acl_action->priority = p_action->priority;
        p_acl_action->color = p_action->color;
    }
    if (p_action->policerId)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
        ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(p_action->policerId);
        p_acl_action->micro_policer_id = ctc_policer_id;
    }
    if (p_action->nexthopId)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_REDIRECT);
        p_acl_action->nh_id = p_action->nexthopId;
    }
    if (p_action->denyRouting)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_DENY_ROUTE);
    }
    if (p_action->denyBridging)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_DENY_BRIDGE);
    }
    if (p_action->denyLearning)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING);
    }

    /* for cubro mpls. */
    if (p_action->popAllMpls)
    {
        p_acl_action->packet_strip.start_packet_strip = CTC_ACL_STRIP_START_TO_L4; /* New l4 start from inner ethernet packet. */ 
        p_acl_action->packet_strip.packet_type = PKT_TYPE_ETH;
    } 
    if (p_action->resetPayloadOffset)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_RESET_PAYLOAD_OFFSET);
    }
    if (p_action->logSessionId)
    {
        GLB_SET_FLAG(p_acl_action->flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
        p_acl_action->log_session_id = p_action->logSessionId;
        p_acl_action->log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
    }
    
    return HAGT_E_NONE;
}

/* Added by weizj for udf */


#define _____MESSAGE_CB_____

int32
hagt_openflow_op_acl_label(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowOpLabelReq_t* req;
    ctc_acl_group_info_t group_info;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowOpLabel;

    sal_memset(&group_info, 0, sizeof(ctc_acl_group_info_t));
    if (req->enable)
    {
        group_info.un.port_class_id = req->label;
        
        switch(req->type)
        {
        case GLB_OPENFLOW_LABEL_PORT:
            group_info.type = CTC_ACL_GROUP_TYPE_PORT_CLASS;
            group_info.lchip = 0;
            break;
            
        case GLB_OPENFLOW_LABEL_GLOBAL:
            group_info.type = CTC_ACL_GROUP_TYPE_GLOBAL;
            break;
        }

        //TODO: use block 0 currently, 2015-03-11
        group_info.priority = 0;
        HAGT_IF_ERROR_RETURN(ctc_acl_create_group(req->label, &group_info));
        HAGT_IF_ERROR_RETURN(ctc_acl_install_group(req->label, &group_info));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_destroy_group(req->label));
    }
    return HAGT_E_NONE;
}

/* Added by weizj for udf */
int32
hagt_openflow_set_udf_parser(Hsrv2AgtMsg_t* p_msg_data)
{
    HAGT_OPENFLOW_FLOW_UDF_ENABLE = 0;
    return HAGT_E_NONE;
}

int32
hagt_openflow_set_e2e(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetE2eReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetE2e;

    if(req->enable)
    {
        ctc_ptp_set_device_type(CTC_PTP_DEVICE_E2E_TC);
    }
    else
    {
        ctc_ptp_set_device_type(CTC_PTP_DEVICE_NONE);
    }

    return HAGT_E_NONE;
}

int32
hagt_openflow_set_stpid(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetStpidReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetStpid;

    ctc_parser_set_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_2, req->stpid);

    return HAGT_E_NONE;
}

int32
hagt_openflow_set_ctpid(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetCtpidReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetCtpid;

    ctc_parser_set_tpid(CTC_PARSER_L2_TPID_CVLAN_TPID, req->ctpid);

    return HAGT_E_NONE;
}

/* Added by kcao for debug ACL iotime */
typedef struct 
{
    ctclib_list_node_t node;
    ctc_acl_key_type_t key_type;
    uint32 entry_id;
    uint32 priority;
    sal_time_t timestamp;
    uint32 delta_us;
} ctc_acl_iotime_item_t;

#define ACL_IOTIME_RECORD_MAX   10000
#define ACL_IOTIME_SEC_MAX      100

typedef struct 
{
    uint32 count_10ms;  /* 0-10ms */
    uint32 count_100ms; /* 10ms-100ms */
    uint32 count_1s;    /* 100ms-1s */
    uint32 count_sec[ACL_IOTIME_SEC_MAX];
    uint32 count_large;
    uint32 total_add_count;
    uint64 total_add_delta;
    uint32 threshold;
} ctc_acl_iotime_stats_t;

typedef struct 
{
    ctc_acl_iotime_stats_t  stats;
    uint32                  threshold;
    ctclib_list_t           record_list;
    uint32                  record_count;
} ctc_acl_iotime_master_t;

ctc_acl_iotime_master_t g_iotime_master;

int32
hagt_of_check_add_time(struct timeval *tv1, struct timeval *tv2, ctc_acl_entry_t *acl_entry)
{
    ctclib_list_node_t *p_node = NULL;
    ctc_acl_iotime_item_t *p_item = NULL;
    uint32 delta = 0;
    uint32 delta_sec = 0;

    delta = ((tv2->tv_sec - tv1->tv_sec) * 1000000) + (tv2->tv_usec - tv1->tv_usec);
    g_iotime_master.stats.total_add_count++;
    g_iotime_master.stats.total_add_delta += delta;
    
    if (delta < 10000)
    {
        g_iotime_master.stats.count_10ms++;
    }
    else if (delta < 100000)
    {
        g_iotime_master.stats.count_100ms++;
    }
    else if (delta < 1000000)
    {
        g_iotime_master.stats.count_1s++;
    }
    else
    {
        delta_sec = delta / 1000000;
        if (delta_sec < ACL_IOTIME_SEC_MAX)
        {
            g_iotime_master.stats.count_sec[delta_sec]++;
        }
        else
        {
            g_iotime_master.stats.count_large++;
        }
    }

    if (delta < g_iotime_master.threshold)
    {
        return 0;
    }
    
    p_item = XCALLOC(MEM_LIB_ACL_IOTIME, sizeof(ctc_acl_iotime_item_t));
    if (NULL == p_item)
    {
        return 0;
    }

    p_item->delta_us = delta;
    p_item->timestamp = tv2->tv_sec;
    p_item->key_type = acl_entry->key.type;
    p_item->entry_id = acl_entry->entry_id;
    p_item->priority = acl_entry->priority;
    
    ctclib_list_insert_tail(&g_iotime_master.record_list, &p_item->node);
    g_iotime_master.record_count++;

    if (ACL_IOTIME_RECORD_MAX < g_iotime_master.record_count)
    {
        p_node = ctclib_list_delete_head(&g_iotime_master.record_list);
        if (p_node)
        {
            p_item = ctclib_container_of(p_node, ctc_acl_iotime_item_t, node);
            XFREE(MEM_LIB_ACL_IOTIME, p_item);
            p_item = NULL;            
        }
    }
    
    return 0;
}

int32
hagt_acl_iotime_clear()
{
    ctclib_list_node_t *p_node = NULL;
    ctclib_list_node_t *p_next = NULL;
    ctc_acl_iotime_item_t *p_item = NULL;

    sal_memset(&g_iotime_master.stats, 0, sizeof(g_iotime_master.stats));
    g_iotime_master.record_count = 0;
    
    ctclib_list_for_each_safe(p_node, p_next, &g_iotime_master.record_list)
    {
        p_item = ctclib_container_of(p_node, ctc_acl_iotime_item_t, node);
        ctclib_list_delete(&g_iotime_master.record_list, p_node);
        XFREE(MEM_LIB_ACL_IOTIME, p_item);
        p_item = NULL;
    }

    return 0;
}

int32
hagt_acl_iotime_show(FILE *fp)
{
    ctc_acl_iotime_item_t *p_item = NULL;
    ctclib_list_node_t *list_node = NULL;
    struct tm *tmptm = NULL;
    char strtime[64];
    char str[16];
    uint32 index = 0;

    sal_fprintf(fp, "%-14s: %-10u\n", "Add Count", g_iotime_master.stats.total_add_count);
    sal_fprintf(fp, "%-14s: %-10"PRIu64"\n", "Add Time(us)", (g_iotime_master.stats.total_add_delta));
    if (g_iotime_master.stats.total_add_count)
    {
        sal_fprintf(fp, "%-14s: %-10u\n", "Avg Time(us)", (uint32)(g_iotime_master.stats.total_add_delta/g_iotime_master.stats.total_add_count));
    }
    else
    {
        sal_fprintf(fp, "%-14s: N/A\n", "Avg Time(us)");
    }
    sal_fprintf(fp, "\n");
    
    sal_fprintf(fp, "%-15s %-10s\n", "IOTime", "Count");
    sal_fprintf(fp, "---------------+----------\n");
    if (g_iotime_master.stats.count_10ms)
    {
        sal_fprintf(fp, "%-15s %-10u\n", "0-10ms", g_iotime_master.stats.count_10ms);
    }
    if (g_iotime_master.stats.count_100ms)
    {
        sal_fprintf(fp, "%-15s %-10u\n", "10-100ms", g_iotime_master.stats.count_100ms);
    }
    if (g_iotime_master.stats.count_1s)
    {
        sal_fprintf(fp, "%-15s %-10u\n", "100ms-1s", g_iotime_master.stats.count_1s);
    }
    
    for (index = 0; index < ACL_IOTIME_SEC_MAX; index++)
    {
        if (g_iotime_master.stats.count_sec[index])
        {
            str[0] = 0;
            sal_sprintf(str, "%us", index);
            sal_fprintf(fp, "%-15s %-10u\n", str, g_iotime_master.stats.count_sec[index]);
        }
    }
    if (g_iotime_master.stats.count_large)
    {
        sal_fprintf(fp, "%-15s %-10u\n", ">100s", g_iotime_master.stats.count_large);
    }
    sal_fprintf(fp, "\n");

    sal_fprintf(fp, "%-6s %-10s %-10s %-10s %-25s\n", "Index", "entry_id", "Priority", "Delta(us)", "Timestamp");
    sal_fprintf(fp, "------+----------+----------+----------+-------------------------\n");
    index = 0;
    ctclib_list_for_each_r(list_node, &g_iotime_master.record_list)
    {
        index++;
        p_item = ctclib_container_of(list_node, ctc_acl_iotime_item_t, node);
        tmptm = sal_localtime(&p_item->timestamp);
        sal_time_strf(strtime, sizeof(strtime), "%F %T", tmptm);
        sal_fprintf(fp, "%-6u %-10u 0x%08x %-10u %-25s\n", index, p_item->entry_id, p_item->priority, p_item->delta_us, strtime);
    }
    
    return 0;
}

int32
hagt_openflow_add_flow(Hsrv2AgtMsg_t* p_msg_data)
{
    struct timeval tv1, tv2;
    HalMsgOpenFlowAddFlowReq_t *req;
    uint32 group_id;
    hagt_openflow_entryid_t* p_new_entryid_info =NULL;
    int32 ret=0;
    uint32 ctc_policer_id = 0;
    ctc_mpls_property_t mpls_pro;
    uint32 priority = 0;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlow;
    if (GLB_OPENFLOW_FLOW_TABLE_ID_ACL == req->entryId.table)
    {
        if (req->action.logSessionId)
        {
            if(req->entryId.entryId < 0x10000092) /*ADPT_FLOW_ID_TABEL_MAC_MAP_IPv6*/
            {
                hagt_openflow_update_truncate(req->action.logSessionId, TRUE, req->action.truncateLen, req->action.logDestPort);
            }
        }
        
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        if (req->action.isDsMplsKey)
        {
            ctc_mpls_ilm_t ilm_info;

            memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
            ilm_info.label = req->rule.unionRule.mplsRule.mplsLabel0;

            if ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0 == req->action.mplsActionType)
                || (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1 == req->action.mplsActionType))
            {
                ilm_info.pop = TRUE;
            }
            
            if (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1 != req->action.mplsActionType)
            {
                if (req->action.nexthopId)
                {
                    ilm_info.nh_id = req->action.nexthopId;
                }
            }
            
            if (req->action.policerId)
            {
                ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(req->action.policerId);
                ilm_info.id_type = CTC_MPLS_ID_FLOW;
                ilm_info.flw_vrf_srv_aps.flow_id = ctc_policer_id;
            }
            
            if (req->action.statsEn)
            {
                ilm_info.id_type |= CTC_MPLS_ID_STATS;
                ilm_info.stats_id = hagt_openflow_stats_get_ctc_stats_id(req->action.statsId);
            }

            if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_L2VPN_EGRESS(req->action.mplsActionType))
            {
                ilm_info.type = CTC_MPLS_LABEL_TYPE_VPWS;
            }
            else
            {
                ilm_info.type = CTC_MPLS_LABEL_TYPE_NORMAL;
            }

            HAGT_IF_ERROR_RETURN(ctc_mpls_add_ilm(&ilm_info));

            if(req->action.priorityValid)
            {
                sal_memset(&mpls_pro, 0x0, sizeof(ctc_mpls_property_t));
                priority = (req->action.priority)/8;
                mpls_pro.label = ilm_info.label;
                mpls_pro.property_type = CTC_MPLS_ILM_LLSP;
                mpls_pro.value = &priority;
                HAGT_IF_ERROR_RETURN(ctc_mpls_set_ilm_property(&mpls_pro));
            }
        }
        else
        {
            ctc_acl_entry_t entry;

            memset(&entry, 0, sizeof(ctc_acl_entry_t));
            //group_id = req->entryId.label;
            group_id = req->entryId.groupId;

            ctc_sai_acl_alloc_entry_id(&entry.entry_id);
            
            entry.priority_valid = TRUE;
            entry.priority = req->entryId.priority;
            /* Modifed by weizj for tunnel bug 33692 */
            HAGT_IF_ERROR_RETURN(hagt_openflow_map_flow_key(entry.priority, &req->rule, &entry.key));
            if ((GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS == req->entryId.entryId)
                || (GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS_MCAST == req->entryId.entryId))
            {
                GLB_UNSET_FLAG(entry.key.u.ipv4_key.sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL_NUM);
                entry.key.u.ipv4_key.mpls_label_num = 0;
                entry.key.u.ipv4_key.mpls_label_num_mask = 0x00;
            }
            HAGT_IF_ERROR_RETURN(hagt_openflow_map_flow_action(&req->action, &entry.action));
            if ((GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS == req->entryId.entryId)
                || (GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS_MCAST == req->entryId.entryId))
            {
                GLB_UNSET_FLAG(entry.action.flag, CTC_ACL_ACTION_FLAG_REDIRECT);
            }
            gettimeofday(&tv1, NULL);
            ret = ctc_acl_add_entry(group_id, &entry);
            if(ret)
            {
                ctc_sai_acl_release_entry_id(entry.entry_id);
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
                return ret;
            }
            ret = ctc_acl_install_entry(entry.entry_id);
            if(ret)
            {
                ctc_sai_acl_release_entry_id(entry.entry_id);
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
                return ret;
            }
            gettimeofday(&tv2, NULL);
            hagt_of_check_add_time(&tv1, &tv2, &entry);
            p_new_entryid_info = (hagt_openflow_entryid_t*)HAGT_OPENFLOW_CALLOC( sizeof(hagt_openflow_entryid_t));
            p_new_entryid_info->entry_id = req->entryId.entryId;
            ctclib_hash_get(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, p_new_entryid_info,
                    ctclib_hash_alloc_intern);
            p_new_entryid_info->ctc_entry_id = entry.entry_id;
        }
    }
    else if (GLB_OPENFLOW_FLOW_TABLE_ID_FDB == req->entryId.table)
    {
        ctc_l2_addr_t     l2_addr;
        ctc_l2dflt_addr_t l2dft_addr;
        int    i   = 0;
        uint16 vid = 1;

        sal_memset(&l2_addr, 0, sizeof(l2_addr));
        sal_memset(&l2dft_addr, 0, sizeof(l2dft_addr));

        if (0 == req->entryId.priority)
        {
            /* process fdb flow default table miss */
            for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
            {
                /* delete all vlan default entry first and for table_miss drop */
                l2dft_addr.fid = vid;
                ctc_l2_remove_default_entry(&l2dft_addr);
            }

            /* to-controller. no need add VDE for HSRV_OF_NH_RESV_NHID_FOR_DROP */
            if (GLB_OPENFLOW_RESV_NHID_FOR_DROP != req->action.nexthopId)
            {
                for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
                {
                    l2dft_addr.fid = vid;
                    l2dft_addr.l2mc_grp_id = req->action.nexthopOffset;
                    l2dft_addr.share_grp_en = 1;
                    ret = ctc_l2_add_default_entry(&l2dft_addr);
                    if(ret)
                    {
                        HAGT_LOG_ERR("HAL Agent OP ctc_l2_add_default_entry failed: rv = %d, %s:%d", 
                                      ret, __FUNCTION__, __LINE__);
                        return ret;
                    }
                }
            }
        }
        else
        {
            /* add_fdb_with_nexthop */
            l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
            l2_addr.fid  = req->rule.macRule.sVlanId;
            for (i = 0; i < req->rule.macRule.macDa.size; i++)
            {
                l2_addr.mac[i] = req->rule.macRule.macDa.buf[i];
            }

            ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, req->action.nexthopId);
            if(ret)
            {
                HAGT_LOG_ERR("HAL Agent OP ctc_l2_add_fdb_with_nexthop failed: rv = %d, %s:%d", 
                              ret, __FUNCTION__, __LINE__);
                return ret;
            }

            p_new_entryid_info = (hagt_openflow_entryid_t*)HAGT_OPENFLOW_CALLOC(sizeof(hagt_openflow_entryid_t));
            p_new_entryid_info->entry_id = req->entryId.entryId;
            ctclib_hash_get(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, p_new_entryid_info,
                            ctclib_hash_alloc_intern);
            p_new_entryid_info->fid = l2_addr.fid;
            sal_memcpy(p_new_entryid_info->mac, l2_addr.mac, sizeof(mac_addr_t));
        }
    }
    
    return HAGT_E_NONE;
}

int32
hagt_openflow_mod_flow(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowUpdateFlowReq_t *req;
    uint32 entry_id;
    int32  ret = HAGT_E_NONE;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowUpdateFlow;

    if (0 == req->entryId.table)
    {
        if (req->action.oldLogSessionId)
        {
            if(req->entryId.entryId < 0x10000092)
            {
                hagt_openflow_update_truncate(req->action.oldLogSessionId, FALSE, 0, 0);
            }
        }

        if (req->action.logSessionId)
        {
            if(req->entryId.entryId < 0x10000092)
            {
                hagt_openflow_update_truncate(req->action.logSessionId, TRUE, req->action.truncateLen, req->action.logDestPort);
            }
        }
        
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        if (req->action.isDsMplsKey)
        {
            /* do not support update ilm according to V350, for now */
            
        }
        else
        {
            ctc_acl_action_t action;

            memset(&action, 0, sizeof(ctc_acl_action_t));
            entry_id = hagt_openflow_flow_get_ctc_entry_id(req->entryId.entryId);

            HAGT_IF_ERROR_RETURN(hagt_openflow_map_flow_action(&req->action, &action));
            HAGT_IF_ERROR_RETURN(ctc_acl_update_action(entry_id, &action));
        }
    }
    else if (GLB_OPENFLOW_FLOW_TABLE_ID_FDB == req->entryId.table)
    {
        ctc_l2_addr_t     l2_addr;
        ctc_l2dflt_addr_t l2dft_addr;
        uint16 vid = 1;

        sal_memset(&l2_addr, 0, sizeof(l2_addr));
        sal_memset(&l2dft_addr, 0, sizeof(l2dft_addr));

        if (0 == req->entryId.priority)
        {
            /* process fdb flow default table miss */
            for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
            {
                /* delete all vlan default entry first and for table_miss drop */
                l2dft_addr.fid = vid;
                ctc_l2_remove_default_entry(&l2dft_addr);
            }

            /* to-controller. no need add VDE for HSRV_OF_NH_RESV_NHID_FOR_DROP */
            if (GLB_OPENFLOW_RESV_NHID_FOR_DROP != req->action.nexthopId)
            {
                for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
                {
                    l2dft_addr.fid = vid;
                    l2dft_addr.l2mc_grp_id = req->action.nexthopOffset;
                    l2dft_addr.share_grp_en = 1;
                    ret = ctc_l2_add_default_entry(&l2dft_addr);
                    if(ret)
                    {
                        HAGT_LOG_ERR("HAL Agent OP ctc_l2_add_default_entry failed: rv = %d, %s:%d", 
                                      ret, __FUNCTION__, __LINE__);
                        return ret;
                    }
                }
            }
        }
        else
        {
            hagt_openflow_entryid_t  openflow_entryid;
            hagt_openflow_entryid_t* p_openflow_entryid = NULL;

            sal_memset(&openflow_entryid, 0x0, sizeof(hagt_openflow_entryid_t));
            openflow_entryid.entry_id= req->entryId.entryId;
            p_openflow_entryid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, &openflow_entryid);

            if(p_openflow_entryid)
            {
                l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
                l2_addr.fid  = p_openflow_entryid->fid;
                sal_memcpy(l2_addr.mac, p_openflow_entryid->mac, sizeof(mac_addr_t));

                HAGT_IF_ERROR_RETURN(ctc_l2_remove_fdb(&l2_addr));

                ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, req->action.nexthopId);
                if(ret)
                {
                    HAGT_LOG_ERR("HAL Agent OP ctc_l2_add_fdb_with_nexthop failed: rv = %d, %s:%d", 
                                  ret, __FUNCTION__, __LINE__);
                    return ret;
                }
            }
        }
    }
    
    return HAGT_E_NONE;
}

int32
hagt_openflow_del_flow(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelFlowReq_t *req;
    uint32 entry_id;
    hagt_openflow_entryid_t* p_openflow_entryid = NULL;
    hagt_openflow_entryid_t  openflow_entryid;
 
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelFlow;

    if (0 == req->entryId.table)
    {
        if (req->logSessionId)
        {
            if(req->entryId.entryId < 0x10000092)
            {
                hagt_openflow_update_truncate(req->logSessionId, FALSE, 0, 0);
            }
        }
        
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        if (req->isDsMplsKey)
        {
            ctc_mpls_ilm_t ilm_info;

            memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
            ilm_info.label = req->mplsLabel;

            HAGT_IF_ERROR_RETURN(ctc_mpls_del_ilm(&ilm_info));
        }
        else
        {
            sal_memset(&openflow_entryid, 0x0, sizeof(hagt_openflow_entryid_t));
            openflow_entryid.entry_id= req->entryId.entryId;
            p_openflow_entryid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, &openflow_entryid);
            if(p_openflow_entryid)
            {
                entry_id = p_openflow_entryid->ctc_entry_id;
                HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id));
                HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id));
                
                ctc_sai_acl_release_entry_id(entry_id);
                ctclib_hash_release(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, p_openflow_entryid);
                HAGT_OPENFLOW_FREE(p_openflow_entryid);
            }
        }
    }
    else if (GLB_OPENFLOW_FLOW_TABLE_ID_FDB == req->entryId.table)
    {
        ctc_l2_addr_t     l2_addr;
        ctc_l2dflt_addr_t l2dft_addr;
        uint16 vid = 1;

        sal_memset(&l2_addr, 0, sizeof(l2_addr));
        sal_memset(&l2dft_addr, 0, sizeof(l2dft_addr));

        if (0 == req->entryId.priority)
        {
            /* process fdb flow default table miss */
            for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
            {
                l2dft_addr.fid = vid;
                ctc_l2_remove_default_entry(&l2dft_addr);
                
                /* add default vlan forward entry */
                l2dft_addr.l2mc_grp_id = vid;
                HAGT_IF_ERROR_RETURN(ctc_l2_add_default_entry(&l2dft_addr));
            }
            /* need add vlan member port back to default entry */
            hsrv_vlan_add_ports_to_vlan();
        }
        else
        {
            sal_memset(&openflow_entryid, 0x0, sizeof(hagt_openflow_entryid_t));
            openflow_entryid.entry_id= req->entryId.entryId;
            p_openflow_entryid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, &openflow_entryid);

            if(p_openflow_entryid)
            {
                l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
                l2_addr.fid  = p_openflow_entryid->fid;
                sal_memcpy(l2_addr.mac, p_openflow_entryid->mac, sizeof(mac_addr_t));

                HAGT_IF_ERROR_RETURN(ctc_l2_remove_fdb(&l2_addr));
                
                ctclib_hash_release(g_pst_hagt_openflow_master->pst_openflow_entryid_hash, p_openflow_entryid);
                HAGT_OPENFLOW_FREE(p_openflow_entryid);
            }
        }
    }
    
    return HAGT_E_NONE;
}

static inline uint32
_adpt_openflow_entryid_hash_make(void* p_info_void)
{
    hagt_openflow_entryid_t* p_info = NULL;

    p_info = p_info_void;
    return (uint32) p_info->entry_id;
}

static inline bool
_adpt_openflow_entryid_hash_compare(void* p_info_in_bucket_void, void* p_info_to_lkp_void)
{
    hagt_openflow_entryid_t* p_info_in_bucket = NULL;
    hagt_openflow_entryid_t* p_info_to_lkp = NULL;

    p_info_in_bucket = p_info_in_bucket_void;
    p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->entry_id != p_info_to_lkp->entry_id)
    {
        return FALSE;
    }

    return TRUE;
}

#define _____INIT_____
int32
hagt_openflow_flow_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlow, hagt_openflow_add_flow);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowUpdateFlow, hagt_openflow_mod_flow);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelFlow, hagt_openflow_del_flow);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowOpLabel, hagt_openflow_op_acl_label);
    /* Added by weizj for udf */
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetUdf, hagt_openflow_set_udf_parser);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetE2e, hagt_openflow_set_e2e);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetStpid, hagt_openflow_set_stpid);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetCtpid, hagt_openflow_set_ctpid);

#undef Reg    

    g_pst_hagt_openflow_master->pst_openflow_entryid_hash = ctclib_hash_create(
        _adpt_openflow_entryid_hash_make,
        _adpt_openflow_entryid_hash_compare);

    ctclib_list_init(&g_iotime_master.record_list);
    g_iotime_master.threshold = 100000; /* 100ms */
    g_iotime_master.record_count = 0;
    
    return HAGT_E_NONE;
}


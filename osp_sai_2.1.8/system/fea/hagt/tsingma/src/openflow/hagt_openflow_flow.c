#include "ctc_api.h"
#include "ctclib_vector.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_chip.h"
#include "hagt_openflow.h"
#include "hagt_openflow_tpoam.h"
#include "glb_l2_define.h"
#include "gen/stbl_qos_mpls_domain_define.h"
#include "gen/stbl_qos_mpls_domain.h"
#include "gen/stbl_qos_mpls_flow_define.h"
#include "gen/stbl_qos_mpls_flow.h"

extern hagt_master_t *g_pst_hagt_master;

extern uint32 hagt_ipuc_get_ipv6_enabled();

extern uint32 hagt_openflow_merter_get_ctc_policer_id(uint32 openflow_meter_id);

extern void hsrv_openflow_get_vxlan_dest_port(uint32* p_is_set, uint32* p_vxlan_destport);

extern int32
hsrv_vlan_add_ports_to_vlan();

extern int32
hsrv_openflow_flow_is_mac_learning_enable(void);

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
    ctc_mirror_dest_t st_dest;
    int32 ret = 0;

    sal_memset(&st_dest, 0, sizeof(ctc_mirror_dest_t));
    st_dest.session_id = session_id;
    st_dest.type = CTC_MIRROR_ACLLOG_SESSION;
    st_dest.truncated_len = truncate_len;
    
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
#if 0
            /* set truncation length and exception */
            sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

            glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TRUNCATION_LEN;
            glb_cfg.u.value  = truncate_len;
            ret = ctc_qos_set_global_config(&glb_cfg);
            if (ret)
            {
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
            }
#endif          
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
hagt_openflow_map_mac(HalMsgOpenFlowMacRule_t* p_mac, uint32 entry_id)
{
    ctc_field_key_t key_field;
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA))
    {           
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_mac->macSa.buf;
        key_field.ext_mask = p_mac->macSaMask.buf;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_mac->macDa.buf;
        key_field.ext_mask = p_mac->macDaMask.buf;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_mac->sTagValid;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;
        key_field.data = p_mac->sVlanId;
        key_field.mask = p_mac->sVlanIdMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_mac->sTagCos;
        key_field.mask = p_mac->sTagCosMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_mac->ethType;
        /* mod by  hansf, sdk doesn't support mask ether_type */
        //key_field.mask = p_mac->ethTypeMask;
        key_field.mask = 0xFFFFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_mac->cTagValid;
        key_field.mask = 0xffff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_mac->cVlanId;
        key_field.mask = p_mac->cVlanIdMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_mac->cTagCos;
        key_field.mask = p_mac->cTagCosMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mac_ipv6(HalMsgOpenFlowMacRule_t* p_mac, uint32 entry_id)
{
    ctc_field_key_t key_field;
    
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA))
    {           
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_mac->macSa.buf;
        key_field.ext_mask = p_mac->macSaMask.buf;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_mac->macDa.buf;
        key_field.ext_mask = p_mac->macDaMask.buf;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_mac->sTagValid;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));  
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;
        key_field.data = p_mac->sVlanId;
        key_field.mask = p_mac->sVlanIdMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_mac->sTagCos;
        key_field.mask = p_mac->sTagCosMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_mac->ethType;
        /* mod by  hansf, sdk doesn't support mask ether_type */
        //key_field.mask = p_mac->ethTypeMask;
        key_field.mask = 0xFFFFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_mac->cTagValid;
        key_field.mask = 0xffff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_mac->cVlanId;
        key_field.mask = p_mac->cVlanIdMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_mac->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_mac->cTagCos;
        key_field.mask = p_mac->cTagCosMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_arp(HalMsgOpenFlowArpRule_t* p_arp, uint32 entry_id)
{
    ctc_field_key_t key_field;

    /*l3 type*/
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_ARP;
    key_field.mask = 0xff;
    (void)ctc_acl_add_key_field(entry_id, &key_field);

        
    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_ARP_OPCODE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ARP_OP_CODE;
        key_field.data = p_arp->arpOpCode;
        key_field.mask = p_arp->arpOpCodeMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_SENDER_IP))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ARP_SENDER_IP;
        key_field.data = p_arp->senderIp;
        key_field.mask = p_arp->senderIpMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_arp->flag, GLB_OPENFLOW_ARP_RULE_FLAG_TARGET_IP))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ARP_TARGET_IP;
        key_field.data = p_arp->targetIp;
        key_field.mask = p_arp->targetIpMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_ipv4(HalMsgOpenFlowIpv4Rule_t* p_ipv4, uint32 entry_id)
{
    ctc_field_key_t key_field;
    uint32 is_set = 0;
    uint32 vxlan_destport = 0;

    /*l3 type*/
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV4;
    key_field.mask = 0xff;
    (void)ctc_acl_add_key_field(entry_id, &key_field);

    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPDA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ipv4->ipDa;
        key_field.mask = p_ipv4->ipDaMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPSA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ipv4->ipSa;
        key_field.mask = p_ipv4->ipSaMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_PROTO))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ipv4->l4Protocol;
        key_field.mask = 0xff; 
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_DSCP))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ipv4->dscp;
        key_field.mask = p_ipv4->dscpMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ECN))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ipv4->ecn;
        key_field.mask = p_ipv4->ecnMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_SRC_PORT))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        key_field.data = p_ipv4->l4SrcPort.port;
        key_field.mask = p_ipv4->l4SrcPort.portMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_DST_PORT))
    {
        hsrv_openflow_get_vxlan_dest_port(&is_set, &vxlan_destport);
        if((p_ipv4->l4DestPort.port == vxlan_destport) && (p_ipv4->l4DestPort.portMask == 0xffff) && ((p_ipv4->l4Protocol == 17)))
        {
            sal_memset(&key_field, 0, sizeof(key_field));
            key_field.type = CTC_FIELD_KEY_L4_USER_TYPE;
            key_field.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;;
            key_field.mask = 0xFF;;
            (void)ctc_acl_add_key_field(entry_id, &key_field);
        }
        else
        {
            sal_memset(&key_field, 0, sizeof(key_field));
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
            key_field.data = p_ipv4->l4DestPort.port;
            key_field.mask = p_ipv4->l4DestPort.portMask;
            (void) ctc_acl_add_key_field(entry_id, &key_field);
        }
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_TYPE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ipv4->icmpType;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_CODE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ipv4->icmpCode;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IGMP_TYPE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IGMP_TYPE;
        key_field.data = p_ipv4->igmpType;
        key_field.mask = 0xff;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_TCP_FLAG))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ipv4->tcpFlags;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IP_FRAG))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ipv4->ipFrag;
        key_field.mask = 1;/*useless*/
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_ipv6(HalMsgOpenFlowIpv6Rule_t* p_ipv6, uint32 entry_id)
{
    ctc_field_key_t key_field;

    /*l3 type*/
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV6;
    key_field.mask = 0xff;
    (void)ctc_acl_add_key_field(entry_id, &key_field);

    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPDA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IPV6_DA;
        key_field.ext_data = p_ipv6->dprefix.buf;
        key_field.ext_mask = p_ipv6->dprefixMask.buf;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPSA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IPV6_SA;
        key_field.ext_data = p_ipv6->sprefix.buf;
        key_field.ext_mask = p_ipv6->sprefixMask.buf;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
      
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_PROTO))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ipv6->l4Protocol;
        key_field.mask = 0xff; 
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_DSCP))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ipv6->dscp;
        key_field.mask = p_ipv6->dscpMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ECN))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ipv6->ecn;
        key_field.mask = p_ipv6->ecnMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_SRC_PORT))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        key_field.data = p_ipv6->l4SrcPort.port;
        key_field.mask = p_ipv6->l4SrcPort.portMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_DST_PORT))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        key_field.data = p_ipv6->l4DestPort.port;
        key_field.mask = p_ipv6->l4DestPort.portMask;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_TYPE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ipv6->icmpType;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_CODE))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ipv6->icmpCode;
        key_field.mask = 0xff;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_FLOW_LABEL))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;
        key_field.data = p_ipv6->flowLabel;
        key_field.mask = p_ipv6->flowLabelMask;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    if (GLB_FLAG_ISSET(p_ipv6->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IP_FRAG))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ipv6->ipFrag;
        key_field.mask = 1;/*useless*/
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mpls(HalMsgOpenFlowMplsRule_t* p_mpls, uint32 entry_id)
{
    ctc_field_key_t key_field;
    
    /*l3 type*/
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_MPLS;
    key_field.mask = 0xff;
    (void) ctc_acl_add_key_field(entry_id, &key_field);
 
    if (p_mpls->mplsLabelNum)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_mpls->mplsLabelNum;
        key_field.mask = 0xff;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    if (p_mpls->mplsLabel0Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = (p_mpls->mplsLabel0 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = (p_mpls->mplsLabel0 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = (p_mpls->mplsLabel0 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_mpls->mplsLabel0 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel0Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
        
    }

    if (p_mpls->mplsLabel1Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = (p_mpls->mplsLabel1 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = (p_mpls->mplsLabel1 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = (p_mpls->mplsLabel1 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_mpls->mplsLabel1 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel1Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    if (p_mpls->mplsLabel2Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = (p_mpls->mplsLabel2 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = (p_mpls->mplsLabel2 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = (p_mpls->mplsLabel2 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_mpls->mplsLabel2 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel2Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_mpls_mcast(HalMsgOpenFlowMplsRule_t* p_mpls, uint32 entry_id)
{
    ctc_field_key_t key_field;

    /*l3 type*/
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_MPLS_MCAST;
    key_field.mask = 0xff;
    (void) ctc_acl_add_key_field(entry_id, &key_field);
    
    if (p_mpls->mplsLabelNum)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_mpls->mplsLabelNum;
        key_field.mask = 0xff;
        (void) ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    if (p_mpls->mplsLabel0Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = (p_mpls->mplsLabel0 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = (p_mpls->mplsLabel0 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = (p_mpls->mplsLabel0 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel0Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_mpls->mplsLabel0 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel0Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
        
    }

    if (p_mpls->mplsLabel1Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = (p_mpls->mplsLabel1 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = (p_mpls->mplsLabel1 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = (p_mpls->mplsLabel1 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel1Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_mpls->mplsLabel1 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel1Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    if (p_mpls->mplsLabel2Mask & GLB_MPLS_LABEL_MASK)
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = (p_mpls->mplsLabel2 >> 12) & 0xFFFFF;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 12) & 0xFFFFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = (p_mpls->mplsLabel2 >> 9) & 0x7;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 9) & 0x7;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = (p_mpls->mplsLabel2 >> 8) & 0x1;
        key_field.mask = ((p_mpls->mplsLabel2Mask) >> 8) & 0x1;
        (void)ctc_acl_add_key_field(entry_id, &key_field);

        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_mpls->mplsLabel2 & 0xFF;
        key_field.mask = (p_mpls->mplsLabel2Mask) & 0xFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }
    
    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_udf(HalMsgOpenFlowUdfRule_t* p_udf, uint32 entry_id)
{
    if (!p_udf->enable)
    {
        return HAGT_E_NONE;
    }

    ctc_field_key_t key_field;
    ctc_acl_udf_t udf_key;
    ctc_acl_udf_t udf_key_mask;
    uint32 *udf = NULL;
    uint32 *udf_mask = NULL;
    sal_memset(&key_field, 0, sizeof(key_field));
    sal_memset(&udf_key, 0, sizeof(udf_key));
    sal_memset(&udf_key_mask, 0, sizeof(udf_key_mask));

    udf = (uint32*)udf_key.udf;
    /* BUG53886 optimize udf_id priority behaviour as DUET2 while spec changed */
    udf_key.udf_id = GLB_ACL_UDF_ENTRY_NUM_MAX-1 - p_udf->udfId;
    udf[0] = p_udf->udf0;
    udf[1] = p_udf->udf1;
    udf[2] = p_udf->udf2;
    udf[3] = p_udf->udf3;

    udf_mask = (uint32*)udf_key_mask.udf;
    udf_key_mask.udf_id = 0xffff;
    udf_mask[0] = p_udf->udfMask0;
    udf_mask[1] = p_udf->udfMask1;
    udf_mask[2] = p_udf->udfMask2;
    udf_mask[3] = p_udf->udfMask3;


    key_field.type = CTC_FIELD_KEY_UDF;
    key_field.ext_data = &udf_key;
    key_field.ext_mask = &udf_key_mask;
    
    HAGT_IF_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field)); 
    
    return HAGT_E_NONE;
}


/* Modifed by weizj for tunnel bug 33692 */
static int32
hagt_openflow_map_tnl(HalMsgOpenFlowTnlRule_t* p_tnl, uint32 entry_id)
{
    /* Deleted by weizj for tunnel bug 33949 */
     ctc_field_key_t key_field;

    if (GLB_FLAG_ISSET(p_tnl->flags, GLB_OPENFLOW_TNL_RULE_FLAG_METADATA))
    {
        sal_memset(&key_field, 0, sizeof(key_field));
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_tnl->metadata;;
        key_field.mask = 0x3FFF;
        (void)ctc_acl_add_key_field(entry_id, &key_field); 
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_map_tun_id_without_tun_port(HalMsgOpenFlowTnlRule_t* p_tnl,HalMsgOpenFlowIpv4Rule_t* p_ipv4, uint32 entry_id)
{
    uint32 is_set = 0;
    uint32 vxlan_destport = 0;
    ctc_field_key_t key_field;
    
    if(!GLB_FLAG_ISSET(p_tnl->flags, GLB_OPENFLOW_TNL_RULE_FLAG_METADATA))
    {
        if(GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_PROTO))
        {
            /*supoort nvgre, gre should be todo */
            if(p_ipv4->l4Protocol == 47)
            {
                if(GLB_FLAG_ISSET(p_tnl->flags, GLB_OPENFLOW_TNL_RULE_FLAG_TUN_ID))
                {
                    sal_memset(&key_field, 0, sizeof(key_field));
                    key_field.type = CTC_FIELD_KEY_GRE_KEY;
                    key_field.data = p_tnl->tunId;
                    key_field.mask = p_tnl->tunId_mask;
                    if (key_field.mask > 0xFFFFFF)
                    {
                        key_field.mask = 0xFFFFFF;
                    }
                    (void)ctc_acl_add_key_field(entry_id, &key_field);
                }
            }
        }
        /*for vni*/
        if(GLB_FLAG_ISSET(p_ipv4->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_DST_PORT))
        {
            hsrv_openflow_get_vxlan_dest_port(&is_set, &vxlan_destport);
            if((p_ipv4->l4DestPort.port == vxlan_destport) && (p_ipv4->l4DestPort.portMask == 0xffff) && ((p_ipv4->l4Protocol == 17)))
            {
                if(GLB_FLAG_ISSET(p_tnl->flags, GLB_OPENFLOW_TNL_RULE_FLAG_TUN_ID))
                {
                    sal_memset(&key_field, 0, sizeof(key_field));
                    key_field.type = CTC_FIELD_KEY_VN_ID;
                    key_field.data = p_tnl->tunId;
                    key_field.mask = p_tnl->tunId_mask;
                    if (key_field.mask > 0xFFFFFF)
                    {
                        key_field.mask = 0xFFFFFF;
                    }
                    (void)ctc_acl_add_key_field(entry_id, &key_field);
                }
            }
        }
    }

    return HAGT_E_NONE;
}

static int32
hagt_openflow_set_metadata_flag(uint32 udf_enable, uint32 entry_id, bool mask_en)
{
    ctc_field_key_t key_field;
    sal_memset(&key_field, 0, sizeof(key_field));
    if (!udf_enable)
    {
        key_field.data = 0;
        if (mask_en)
        {
            key_field.mask = 0x3FFF;
        }

        key_field.type = CTC_FIELD_KEY_METADATA;
        (void)ctc_acl_add_key_field(entry_id, &key_field);
    }

    return HAGT_E_NONE;
}
/* End by weizj for tunnel */

static int32
hagt_openflow_map_flow_key(uint32 priority, HalMsgOpenFlowRule_t* p_rule, uint32 entry_id)
{
    /* duet2+ acl udf key doesn't share with metadata, so always set metadata flag */
    uint32 udf_enable = 0;

    switch(p_rule->unionRule.present)
    {
    case unionRule_PR_NOTHING:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        /* Modifed by weizj for udf bug 34569*/
        if (GLB_OPENFLOW_FLOW_DEFAULT_ENTRY_PRIORITY != priority)
        {
            hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        }
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));

        break;

    case unionRule_PR_arpRule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_arp(&p_rule->unionRule.arpRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));

        break;

    case unionRule_PR_ipv4Rule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        if (GLB_OPENFLOW_FLOW_DEFAULT_ENTRY_PRIORITY != priority)
        {        
            hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        }
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv4(&p_rule->unionRule.ipv4Rule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tun_id_without_tun_port(&p_rule->tnlRule,&p_rule->unionRule.ipv4Rule, entry_id));
        break;

    case unionRule_PR_ipv6Rule:
        #if 1
        /* ipv6 parse disable, we only use ether_type(0x86dd) and l3_type = 0 (not ipv6)*/
        /* HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac_ipv6(&p_rule->macRule, &p_acl_key->u.ipv6_key)); */
        /* HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv6(&p_rule->unionRule.ipv6Rule, &p_acl_key->u.ipv6_key)); */
        /* Modifed by weizj for tunnel bug 33692 */
        hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac_ipv6(&p_rule->macRule, entry_id));
        //HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, &p_acl_key->u.ipv6_key));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_ipv6(&p_rule->unionRule.ipv6Rule, entry_id));
        #endif
        break;

    case unionRule_PR_mplsRule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        if (GLB_OPENFLOW_FLOW_DEFAULT_ENTRY_PRIORITY != priority)
        {
             hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        }
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls(&p_rule->unionRule.mplsRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));

        break;

    case unionRule_PR_mplsMcastRule:
        /* Modifed by weizj for tunnel bug 33692 */
        /* Modifed by weizj for udf */
        if (GLB_OPENFLOW_FLOW_DEFAULT_ENTRY_PRIORITY != priority)
        {
              hagt_openflow_set_metadata_flag(udf_enable, entry_id, TRUE);
        }
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mac(&p_rule->macRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_tnl(&p_rule->tnlRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_mpls_mcast(&p_rule->unionRule.mplsRule, entry_id));
        HAGT_IF_ERROR_RETURN(hagt_openflow_map_udf(&p_rule->udfRule, entry_id));
        break;

    default:
        break;
    }
    return HAGT_E_NONE;
}

static int32
hagt_openflow_add_action_field(HalMsgOpenFlowAction_t* p_action, uint32 entry_id)
{
    ctc_acl_field_action_t action_field;
    ctc_acl_packet_strip_t packet_strip;
    uint32 ctc_policer_id = 0;
    ctc_acl_vlan_edit_t vlan_edit;
    
    if (p_action->statsEn)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_STATS;
        action_field.data0 = hagt_openflow_stats_get_ctc_stats_id(p_action->statsId);
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_STATS;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }
    
    /* priority and color */
    if (p_action->priorityValid)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_PRIORITY;
        action_field.data0 = CTC_QOS_COLOR_NONE;/*modified by yejl for bug 51941, 2019-06-12*/
        /*priority range[0..15]*/
        action_field.data1 = p_action->priority/8*2;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_PRIORITY;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }
    
    /* flow policer */
    if (p_action->policerId)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
        ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(p_action->policerId);
        action_field.data0 = ctc_policer_id;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }



    /* redirect*/
    if (p_action->nexthopId)
    {
        /* should remove first */
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);

        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
        action_field.data0 = p_action->nexthopId;
        (void) ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }

    /* deny learning */
    if (p_action->denyLearning)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
        (void) ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
        (void) ctc_acl_remove_action_field(entry_id, &action_field);
    }

    /* for usw hybrid, all flow except action "drop" and "to normal" should add action: green and yellow cancel discard */
    if ((p_action->denyBridging && p_action->denyRouting && p_action->denyLearning) && (1 != p_action->nexthopId))
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;
        action_field.data0  =1<< CTC_QOS_COLOR_GREEN;
        action_field.data0 |=1<< CTC_QOS_COLOR_YELLOW;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;
        action_field.data0  = 1<<CTC_QOS_COLOR_GREEN;
        action_field.data0 |= 1<<CTC_QOS_COLOR_YELLOW;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }

    if((hsrv_openflow_flow_is_mac_learning_enable()) && (p_action->nexthopId != 1) && p_action->denyLearning) /*nh ==1 is drop nexthop HSRV_OF_NH_RESV_NHID_FOR_DROP*/
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
        (void) ctc_acl_remove_action_field(entry_id, &action_field);
    }

    /*added by hansf for support set vlan action before group*/
    if (p_action->groupVlanType)
    {
        sal_memset(&vlan_edit, 0, sizeof(vlan_edit));
        if (GLB_FLAG_ISSET(p_action->groupVlanType, GLB_GROUP_VLAN_PUSH))
        {
            if (p_action->groupVlanId)
            {
                vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_ADD;
                vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
                vlan_edit.svid_new = p_action->groupVlanId;
                if (GLB_FLAG_ISSET(p_action->groupVlanType, GLB_GROUP_VLAN_SET_PCP))
                {
                    vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
                    vlan_edit.scos_new = p_action->groupVlanCos;
                }
            }
        }
        else if (GLB_FLAG_ISSET(p_action->groupVlanType, GLB_GROUP_VLAN_SET))
        {
            if (p_action->groupVlanId)
            {
                vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
                vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
                vlan_edit.svid_new = p_action->groupVlanId;
                if (GLB_FLAG_ISSET(p_action->groupVlanType, GLB_GROUP_VLAN_SET_PCP))
                {
                    vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
                    vlan_edit.scos_new = p_action->groupVlanCos;
                }
            }
        }
        else if (GLB_FLAG_ISSET(p_action->groupVlanType, GLB_GROUP_VLAN_POP))
        {
            vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_DEL;
        }
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;
        action_field.ext_data = &vlan_edit;
        /* remove first */
        (void) ctc_acl_remove_action_field(entry_id, &action_field);
        (void) ctc_acl_add_action_field(entry_id, &action_field);
        
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;
        (void) ctc_acl_remove_action_field(entry_id, &action_field);
    }
    /*end hansf*/

    /* for cubro mpls. */
    if (p_action->popAllMpls)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        sal_memset(&packet_strip, 0, sizeof(packet_strip));
        
        action_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        packet_strip.start_packet_strip = CTC_ACL_STRIP_START_TO_L4;
        packet_strip.packet_type = PKT_TYPE_ETH;
        action_field.ext_data = &packet_strip;
        (void) ctc_acl_add_action_field(entry_id, &action_field);
    }
    else if (p_action->resetPayloadOffset)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        sal_memset(&packet_strip, 0, sizeof(packet_strip));
        
        action_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        packet_strip.start_packet_strip = CTC_ACL_STRIP_START_TO_L2;
        packet_strip.packet_type = PKT_TYPE_ETH;
        action_field.ext_data = &packet_strip;
        (void) ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }


    /*log*/
    if (p_action->logSessionId)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;
        action_field.data0 = p_action->logSessionId;
        action_field.data1 = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }
    else
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;
        (void)ctc_acl_remove_action_field(entry_id, &action_field);
    }

    if (p_action->stripPacket)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        sal_memset(&packet_strip, 0, sizeof(packet_strip));
        action_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        packet_strip.start_packet_strip = CTC_ACL_STRIP_START_TO_L4;
        packet_strip.packet_type = PKT_TYPE_ETH;
        action_field.ext_data = &packet_strip;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
    }

    /* strip-header */
    if (p_action->stripLen)
    {
        sal_memset(&action_field, 0, sizeof(action_field));
        sal_memset(&packet_strip, 0, sizeof(packet_strip));
        action_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        packet_strip.start_packet_strip = CTC_ACL_STRIP_START_TO_L4;
        packet_strip.packet_type = PKT_TYPE_ETH;
        packet_strip.strip_extra_len = p_action->stripLen;
        action_field.ext_data = &packet_strip;
        (void)ctc_acl_add_action_field(entry_id, &action_field);
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
    HalMsgOpenFlowSetUdfReq_t *req;    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetUdf;

    HAGT_OPENFLOW_FLOW_UDF_ENABLE = req->enable ? 1 : 0;

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
    stbl_qos_mpls_domain_t * p_db_mpls_domain = NULL;
    stbl_qos_mpls_domain_key_t mpls_domain_key;
    stbl_qos_mpls_flow_t * p_db_mpls_flow = NULL;
    stbl_qos_mpls_flow_t mpls_flow;
    ctc_mpls_property_t mpls_pro;
    ctc_mpls_ilm_qos_map_t qos_map;
    int32 ret=0;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlow;
    if (0 == req->entryId.table)
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
            /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-27 
                support control word */
            else if (hagt_tpoam_get_pw_mode())
            {
                ilm_info.cwen = TRUE;
                #ifdef OFPRODUCT
                if((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0 != req->action.mplsActionType) &&
                    (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1 != req->action.mplsActionType))
                {
                    ilm_info.oam_check_type = 3; /*3 == SYS_OAM_TP_OAM_DISCARD_TYPE*/
                }
                #endif
            }
            /* liwh end */
            if (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1 != req->action.mplsActionType)
            {
                if (req->action.nexthopId)
                {
                    ilm_info.nh_id = req->action.nexthopId;
                }
            }
            
            if (req->action.policerId)
            {
                ilm_info.policer_id = req->action.policerId;
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
            /*added by yangl for tm mpls 2019-08-27*/
            ilm_info.flag |= CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO;

            HAGT_IF_ERROR_RETURN(ctc_mpls_add_ilm(&ilm_info));

            /*added by yejl for bug 52134, 2019-06-12*/
            if(req->action.priorityValid)
            {
                sal_memset(&mpls_pro, 0x0, sizeof(mpls_pro));
                sal_memset(&qos_map, 0x0, sizeof(qos_map));
                qos_map.mode = CTC_MPLS_ILM_QOS_MAP_LLSP;
                qos_map.priority = (req->action.priority)/4;
                qos_map.exp_domain = 1;
                mpls_pro.space_id = 0;
                mpls_pro.label = ilm_info.label;
                mpls_pro.property_type = CTC_MPLS_ILM_QOS_MAP;
                mpls_pro.value = (void *)(&qos_map);
                HAGT_IF_ERROR_RETURN(ctc_mpls_set_ilm_property(&mpls_pro));
            }
            /*ended by yejl*/
            else
            {
                /*added by yejl for bug 51333, 2019-06-20*/
                /*added by yejl for bug 52805, 2019-07-25*/

                /*start get the qos mpls domain, if the table is NULL, the exp_domain is 0, else is the p_db_mpls_domain->exp_domain*/
                sal_memset(&mpls_domain_key, 0, sizeof(mpls_domain_key));
                mpls_domain_key.mpls_label = ilm_info.label;
                p_db_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&mpls_domain_key);
                if (NULL == p_db_mpls_domain)
                {
                    sal_memset(&mpls_pro, 0x0, sizeof(mpls_pro));
                    sal_memset(&qos_map, 0x0, sizeof(qos_map));
                    qos_map.mode = CTC_MPLS_ILM_QOS_MAP_ELSP;
                    qos_map.exp_domain = 0;
                    mpls_pro.space_id = 0;
                    mpls_pro.label = ilm_info.label;
                    mpls_pro.property_type = CTC_MPLS_ILM_QOS_MAP;
                    mpls_pro.value = (void *)(&qos_map);
                    HAGT_IF_ERROR_RETURN(ctc_mpls_set_ilm_property(&mpls_pro));    
                }
                else
                {
                    sal_memset(&mpls_pro, 0x0, sizeof(mpls_pro));
                    sal_memset(&qos_map, 0x0, sizeof(qos_map));
                    qos_map.mode = CTC_MPLS_ILM_QOS_MAP_ELSP;
                    qos_map.exp_domain = p_db_mpls_domain->exp_domain;
                    mpls_pro.space_id = 0;
                    mpls_pro.label = ilm_info.label;
                    mpls_pro.property_type = CTC_MPLS_ILM_QOS_MAP;
                    mpls_pro.value = (void *)(&qos_map);
                    HAGT_IF_ERROR_RETURN(ctc_mpls_set_ilm_property(&mpls_pro));
                }
                /*end*/

                /*start creat qos mpls flow table and set the mpls_flow_en is true*/
                sal_memset(&mpls_flow, 0, sizeof(mpls_flow));
                mpls_flow.key.mpls_label = ilm_info.label;
                p_db_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(&(mpls_flow.key));
                if (p_db_mpls_flow)
                {
                    p_db_mpls_flow->mpls_flow_en = TRUE;
                    stbl_qos_mpls_flow_set_qos_mpls_flow_field(p_db_mpls_flow, STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN);
                }
                else
                {
                    stbl_qos_mpls_flow_add_qos_mpls_flow(&mpls_flow);
                    p_db_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(&(mpls_flow.key));
                    if (p_db_mpls_flow)
                    {
                        p_db_mpls_flow->mpls_flow_en = TRUE;
                        stbl_qos_mpls_flow_set_qos_mpls_flow_field(p_db_mpls_flow, STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN);
                    }
                }
                /*ended*/
            }
        }
        else
        {
            ctc_acl_entry_t entry;
            ctc_field_key_t key_field;
            ctc_acl_field_action_t action_field;

            memset(&entry, 0, sizeof(ctc_acl_entry_t));
            //group_id = req->entryId.label;
            group_id = req->entryId.groupId;

            ctc_sai_acl_alloc_entry_id(&entry.entry_id);
            
            entry.priority_valid = TRUE;
            entry.priority = req->entryId.priority;
            entry.mode = 1;
            if (HAGT_OPENFLOW_FLOW_UDF_ENABLE)
            {
                entry.key_type = CTC_ACL_KEY_MAC_IPV4_EXT;
            }
            else
            {
                entry.key_type = CTC_ACL_KEY_MAC_IPV4;
            }

            if (unionRule_PR_ipv6Rule == req->rule.unionRule.present)
            {
                entry.key_type = CTC_ACL_KEY_MAC_IPV6;
            }

            ret = ctc_acl_add_entry(group_id, &entry);
            if(ret)
            {
                ctc_sai_acl_release_entry_id(entry.entry_id);
                HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
                return ret;
            }
            /* Modifed by weizj for tunnel bug 33692 */
            HAGT_IF_ERROR_RETURN(hagt_openflow_map_flow_key(entry.priority, &req->rule, entry.entry_id));

            if ((GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS == req->entryId.entryId)
                || (GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS_MCAST == req->entryId.entryId))
            {
                sal_memset(&key_field, 0, sizeof(key_field));
                key_field.type = CTC_FIELD_KEY_LABEL_NUM;
                key_field.data = 0;
                key_field.mask = 0x00;
                (void)ctc_acl_add_key_field(entry.entry_id, &key_field);
            }
            HAGT_IF_ERROR_RETURN(hagt_openflow_add_action_field(&req->action, entry.entry_id));
            if ((GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS == req->entryId.entryId)
                || (GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS_MCAST == req->entryId.entryId))
            {
                sal_memset(&action_field, 0, sizeof(action_field));
                action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
                action_field.data0 = req->action.nexthopId;
                (void)ctc_acl_add_action_field(entry.entry_id, &action_field);
            }
            gettimeofday(&tv1, NULL);

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
//            ctc_acl_action_t action;

//            memset(&action, 0, sizeof(ctc_acl_action_t));
            entry_id = hagt_openflow_flow_get_ctc_entry_id(req->entryId.entryId);

            HAGT_IF_ERROR_RETURN(hagt_openflow_add_action_field(&req->action, entry_id));
//            HAGT_IF_ERROR_RETURN(ctc_acl_update_action(entry_id, &action));
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
    hagt_openflow_entryid_t openflow_entryid;
    stbl_qos_mpls_flow_t * p_db_mpls_flow = NULL;
    stbl_qos_mpls_flow_key_t mpls_flow_key;
 
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

            /*added by yejl for bug 52805, 2019-07-25*/
            sal_memset(&mpls_flow_key, 0, sizeof(mpls_flow_key));
            mpls_flow_key.mpls_label = ilm_info.label;
            p_db_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(&mpls_flow_key);
            if (p_db_mpls_flow)
            {
                stbl_qos_mpls_flow_del_qos_mpls_flow(&mpls_flow_key);
            }
            /*ended by yejl*/
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


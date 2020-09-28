/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "glb_tap_define.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#ifdef TAPPRODUCT
#include "hsrv_tap.h"
#endif /* !TAPPRODUCT */
#include "lib_acl.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/
extern uint32_t sys_to_sai_color_map[4];

extern sai_object_id_t g_hsrv_mirror_sai_sid_obj_array[GLB_MIRROR_MAX_SESSION_NUM];

#define MAKE_FEA_ACL_POLICY_ACTION_ID(acl_id) (acl_id & 0x0000FFC00000FFFFULL) /*modified by yejl for ipv6 acl*/

#define IS_ACL_POLICY_ID(acl_id) (acl_id & 0x0000FFFF00000000ULL) /*modified by yejl for ipv6 acl*/

#define IS_ACL_WORM_FILTER_ID(acl_id) (acl_id & 0xFFFFFFFFFF00FFFFULL)

#define IS_ACL_TAP_FLOW_ID(acl_id) (acl_id & 0xFFFFFFFF00000000ULL)

#define IS_ACL_TAP_DECAP_NO_INMATCH_V6(acl_id) (acl_id & 0x100000000ULL)

#define MAKE_FEA_TAP_GROUOP_ACL_ID(seq_no, direct, ifindex, class_priority, group_priority) ((uint64)((((seq_no) << 16)&0xffff0000) | ((direct)<<15) | ((ifindex)&0x7fff)) + (((class_priority&0x3ff)<< 38) | (((group_priority&0x3f) << 32))))

#define ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(priority)            ((priority >> 6) & 0xFFFF)
#define ACL_TAP_GET_TAP_GROUP_ID_FROM_PRIORITY(priority)      ((priority >> 22) & 0x3ff)
#define ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(priority)      ((priority) & 0x3f)

extern int32
hsrv_qos_policer_action_map_sai_type(sai_attribute_t *attr, uint32 *attr_count, tbl_qos_policer_action_profile_t *p_db_action_profile);

int32 hsrv_acl_del_action_policer_field(tbl_acl_entry_action_t* p_acl_entry_action);

static int32 _hsrv_acl_update_entry_action(uint64 aclid);

extern uint32 g_stm_mode;

uint32
hsrv_acl_stm_is_ipv6_profile_of()
{
    if (GLB_STM_MODE_OF_IPV6 == g_stm_mode) 
    {
        return TRUE;
    }
    
    return FALSE;
}

uint32
hsrv_acl_stm_is_ipv6_profile()
{
    if (GLB_STM_MODE_IPUCV6 == g_stm_mode) 
    {
        return TRUE;
    }
    
    return FALSE;
}

int32
hsrv_acl_ipv6_address_parser(uint8 *data1, uint8 *data2, uint8 len)
{
    int8 cnt = 0;
    int8 cnt1 = 0;
    int8 i = 0;
    int8 j = 0;
    for (i = 0; i< len; i++)
    {   
        cnt = cnt + 3;
        for (j = 0; j < len; j ++)
        {
            data1[j + cnt1] = data2[cnt];
            if (j < len - 1)
            {
                cnt = cnt - 1;
            }
        }
        cnt1 = cnt1 + len;
        cnt = cnt + len; 
    }
    return 0;
}

static int32
_hsrv_acl_add_entry(tbl_acl_entry_t* acl_entry)
{
    sai_acl_api_t *acl_api = NULL;
    sai_object_id_t acl_entry_id = 0;
    sai_object_id_t acl_entry_id_ipv4 = 0;
    sai_object_id_t acl_entry_id_ipv6 = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t attr_list[80];
    sai_attribute_t attr_list1[80];
    sai_attribute_t counter_attr;
    tbl_fea_acl_t fea_acl;
    tbl_fea_acl_table_t *pst_fea_acl_table = NULL;
    tbl_fea_acl_table_t fea_acl_table;
    tbl_tap_group_t     *tap_group         = NULL;
    sai_object_id_t port_oid = 0;
    uint32 attr_count = 0;
    mac_addr_t mac_mask;

#ifdef TAPPRODUCT
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
#endif

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    if (NULL == acl_entry || acl_entry->invalid)
    {
        return HSRV_E_NONE;
    }

    sal_memset(&fea_acl_table, 0, sizeof(fea_acl_table));
    fea_acl_table.key.acl_table_id = acl_entry->tblid;
    pst_fea_acl_table = tbl_fea_acl_table_get_fea_acl_table(&fea_acl_table.key);
    if (NULL == pst_fea_acl_table)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

#ifdef TAPPRODUCT
    if (acl_entry->tap_action_redirect)
    {
        tap_group = hsrv_tap_search_tap_group_by_id(acl_entry->tap_group_oid);
        if (NULL == tap_group)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
    }
#endif /* !TAPPRODUCT */

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    sal_memset(attr_list, 0, sizeof(attr_list));
    sal_memset(attr_list1, 0, sizeof(attr_list1));
    sal_memset(&counter_attr, 0, sizeof(counter_attr));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = pst_fea_acl_table->sai_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0xffffffff - acl_entry->entry_priority;
    attr_count++;

    if (pst_fea_acl_table->direct_in)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        if (0 == (acl_entry->key.aclid & 0x7FFF))
        {
            attr_list[attr_count].value.aclfield.data.oid = acl_entry->key.aclid;
        }
        else
        {
            hsrv_interface_ifindex_to_objectid(acl_entry->in_port, &port_oid);
            attr_list[attr_count].value.aclfield.data.oid = port_oid;
        }
        attr_count++;
    }
    else
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        hsrv_interface_ifindex_to_objectid(acl_entry->out_port, &port_oid);
        attr_list[attr_count].value.aclfield.data.oid = port_oid;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_MACSA) || 
        (0 == acl_entry->key_flags && 0 == acl_entry->key_arp_flags))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, acl_entry->src_mac, sizeof(mac_addr_t));
        if (0 == acl_entry->key_flags && 0 == acl_entry->key_arp_flags)
        {
            sal_memset(&mac_mask, 0xFF, sizeof(mac_addr_t));
            sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, mac_mask, sizeof(mac_addr_t));
        }
        else
        {
            sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, acl_entry->src_mac_mask, sizeof(mac_addr_t));
        }
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_MACDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, acl_entry->dst_mac, sizeof(mac_addr_t));
        sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, acl_entry->dst_mac_mask, sizeof(mac_addr_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_VLAN))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->svlan;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->svlan_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->cvlan;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->cvlan_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->ether_type;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->ether_type_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->src_ip.u.prefix4.s_addr = sal_htonl(acl_entry->src_ip.u.prefix4.s_addr);
        acl_entry->src_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->src_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->src_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->src_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->dst_ip.u.prefix4.s_addr = sal_htonl(acl_entry->dst_ip.u.prefix4.s_addr);
        acl_entry->dst_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->dst_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->dst_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->dst_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }
    
    if (GLB_FLAG_ISSET(acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        /*added by yejl for ipv6 acl */
    #ifndef _GLB_UML_SYSTEM_
      #ifdef USW
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.data.ip6, acl_entry->src_ip.u.prefix6.__in6_u.__u6_addr8, 4);
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.mask.ip6, acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8, 4);
      #else
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->src_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->src_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
      #endif
    #else
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.data.ip6, acl_entry->src_ip.u.prefix6.__in6_u.__u6_addr8, 4);
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.mask.ip6, acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8, 4);
    #endif
        /*ended by yejl*/
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        /*added by yejl for ipv6 acl */
    #ifndef _GLB_UML_SYSTEM_
      #ifdef USW
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.data.ip6, acl_entry->dst_ip.u.prefix6.__in6_u.__u6_addr8, 4);
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.mask.ip6, acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, 4);    
      #else
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->dst_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->dst_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
      #endif
    #else
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.data.ip6, acl_entry->dst_ip.u.prefix6.__in6_u.__u6_addr8, 4);
        hsrv_acl_ipv6_address_parser(attr_list[attr_count].value.aclfield.mask.ip6, acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, 4);
    #endif
        /*ended by yejl*/
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPv6_FLOW_LABEL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->ipv6_flow_label;
        attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->ipv6_flow_label_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->ip_protocol;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->ip_protocol_mask;
        attr_count++;

        if((0 == acl_entry->ip_protocol_mask) &&
           (acl_entry->ip_protocol == GLB_ACL_L4_PROTOCOL_GRE))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_GRE_TYPE;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            if(GLB_ACL_GRE_GRE == acl_entry->l4_gre_type)
            {
                attr_list[attr_count].value.aclfield.data.u32 = SAI_ACL_GRE_GRE;
            }else{
                attr_list[attr_count].value.aclfield.data.u32 = SAI_ACL_GRE_NVGRE;
            }
            
            attr_list[attr_count].value.aclfield.mask.u32 = 0;
            attr_count++;
        }
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if(acl_entry->l4_src_port_type != GLB_ACE_PORT_TYEP_ANY)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_entry->l4_src_port;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->l4_src_port_mask;
            attr_count++;
        }
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if(acl_entry->l4_dst_port_type != GLB_ACE_PORT_TYEP_ANY)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_entry->l4_dst_port;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->l4_dst_port_mask;
            attr_count++;
        }
        /*added by yejl for bug54032,54049, 2019-10-22*/
        if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_VXLAN_VNI))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_VXLAN_VNI_DSTPORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u32 = acl_entry->l4_dst_port;
            attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->l4_dst_port_mask;
            attr_count++;
        }
        /*ended*/
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_COS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->svlan_cos;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->svlan_cos_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_INNER_COS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->cvlan_cos;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->cvlan_cos_mask;
        attr_count++;
    }

    /*added by yejl for bug 49791, 2018-11-14*/
    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_UNTAG))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_UNTAG_VLAN;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->untag_vlan;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_DSCP))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DSCP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->dscp;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->dscp_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PRECEDENCE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->ip_precedence;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->ip_precedence_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_IP_FRAG))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->ip_frag;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_IGMP_TYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IGMP_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->igmp_type;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->icmp_type;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->icmp_code;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->tcp_flags;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->tcp_flags_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_OPTIONS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_OPTIONS;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->options;
        attr_count++;
    }

    /*arp fields*/
    if (GLB_FLAG_ISSET(acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        attr_list[attr_count].id =SAI_ACL_ENTRY_ATTR_FIELD_ARP_OP_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->arp_op_code & 0xFFFF;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->arp_op_code_mask & 0xFFFF;
        attr_count++;
    }
    
    if (GLB_FLAG_ISSET(acl_entry->key_arp_flags, GLB_ARP_ACE_SENDER_IP))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ARP_SENDER_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->src_ip.u.prefix4.s_addr = sal_htonl(acl_entry->src_ip.u.prefix4.s_addr);
        acl_entry->src_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->src_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->src_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->src_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_arp_flags, GLB_ARP_ACE_TARGET_IP))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ARP_TARGET_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->dst_ip.u.prefix4.s_addr = sal_htonl(acl_entry->dst_ip.u.prefix4.s_addr);
        acl_entry->dst_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->dst_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->dst_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->dst_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    
    #ifndef TAPPRODUCT     
    /*added by yejl for IPv6/MAC acl*/
    if ((acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6) || 
              (acl_entry->is_ipv6))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;
    }
    else if (acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV4_PACKET;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;
    }
    #endif
    /*ended by yejl*/
    

/* inner fields */
    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_MAC_ACE_MACSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_MAC;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, acl_entry->inner_src_mac, sizeof(mac_addr_t));
        if (0 == acl_entry->inner_key_flags)
        {
            sal_memset(&mac_mask, 0xFF, sizeof(mac_addr_t));
            sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, mac_mask, sizeof(mac_addr_t));
        }
        else
        {
            sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, acl_entry->inner_src_mac_mask, sizeof(mac_addr_t));
        }
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_MAC_ACE_MACDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_MAC;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, acl_entry->inner_dst_mac, sizeof(mac_addr_t));
        sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, acl_entry->inner_dst_mac_mask, sizeof(mac_addr_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_MAC_ACE_VLAN))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_OUTER_VLAN_ID;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->inner_svlan;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->inner_svlan_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_INNER_VLAN_ID;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->inner_cvlan;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->inner_cvlan_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = acl_entry->inner_ether_type;
        attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->inner_ether_type_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->inner_src_ip.u.prefix4.s_addr = sal_htonl(acl_entry->inner_src_ip.u.prefix4.s_addr);
        acl_entry->inner_src_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->inner_src_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->inner_src_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->inner_src_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->inner_dst_ip.u.prefix4.s_addr = sal_htonl(acl_entry->inner_dst_ip.u.prefix4.s_addr);
        acl_entry->inner_dst_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->inner_dst_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->inner_dst_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->inner_dst_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->inner_src_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->inner_src_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->inner_dst_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->inner_dst_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IPv6_FLOW_LABEL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->inner_ipv6_flow_label;
        attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->inner_ipv6_flow_label_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_ip_protocol;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_ip_protocol_mask;
        attr_count++;
#if 0
        /* TODO support innert nvgre/gre/vxlan */
        if((0 == acl_entry->inner_ip_protocol_mask) &&
           (acl_entry->inner_ip_protocol == GLB_ACL_L4_PROTOCOL_GRE))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_GRE_TYPE;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            if(GLB_ACL_GRE_GRE == acl_entry->inner_l4_gre_type)
            {
                attr_list[attr_count].value.aclfield.data.u32 = SAI_ACL_GRE_GRE;
            }else{
                attr_list[attr_count].value.aclfield.data.u32 = SAI_ACL_GRE_NVGRE;
            }
            
            attr_list[attr_count].value.aclfield.mask.u32 = 0;
            attr_count++;
        }
#endif        
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_SRCPORT))
    {
        if(acl_entry->inner_l4_src_port_type != GLB_ACE_PORT_TYEP_ANY)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_SRC_PORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_entry->inner_l4_src_port;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->inner_l4_src_port_mask;
            attr_count++;
        }
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_DSTPORT))
    {
        if(acl_entry->inner_l4_dst_port_type != GLB_ACE_PORT_TYEP_ANY)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_DST_PORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_entry->inner_l4_dst_port;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_entry->inner_l4_dst_port_mask;
            attr_count++;
        }
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_COS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_OUTER_VLAN_PRI;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_svlan_cos;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_svlan_cos_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_INNER_COS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_INNER_VLAN_PRI;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_cvlan_cos;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_cvlan_cos_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_DSCP))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DSCP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_dscp;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_dscp_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PRECEDENCE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_ip_precedence;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_ip_precedence_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IP_FRAG))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_FRAG;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_ip_frag;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IGMP_TYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IGMP_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_igmp_type;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_ICMP_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_icmp_type;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_ICMP_CODE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_icmp_code;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_TCP_FLAGS;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_tcp_flags;
        attr_list[attr_count].value.aclfield.mask.u8 = acl_entry->inner_tcp_flags_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_OPTIONS))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_OPTIONS;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->inner_options;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->inner_src_ip.u.prefix4.s_addr = sal_htonl(acl_entry->inner_src_ip.u.prefix4.s_addr);
        acl_entry->inner_src_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->inner_src_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->inner_src_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->inner_src_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_flags, GLB_IP_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        acl_entry->inner_dst_ip.u.prefix4.s_addr = sal_htonl(acl_entry->inner_dst_ip.u.prefix4.s_addr);
        acl_entry->inner_dst_ip_mask.u.prefix4.s_addr = sal_htonl(acl_entry->inner_dst_ip_mask.u.prefix4.s_addr);
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &acl_entry->inner_dst_ip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &acl_entry->inner_dst_ip_mask.u.prefix4, sizeof(addr_ipv4_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->inner_src_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->inner_src_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPv6;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        sal_memcpy(&attr_list[attr_count].value.aclfield.data.ip6, &acl_entry->inner_dst_ip.u.prefix6, sizeof(addr_ipv6_t));
        sal_memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, &acl_entry->inner_dst_ip_mask.u.prefix6, sizeof(addr_ipv6_t));
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_IPv6_FLOW_LABEL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->inner_ipv6_flow_label;
        attr_count++;
    }

    
/* actions */

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_UDF))
    {
#if 0    
        if (GLB_ACL_UDF_GROUP_L3_HEAD == acl_entry->udf_type || GLB_ACL_UDF_GROUP_L4_HEAD == acl_entry->udf_type)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_UDF_MATCH;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u32 = acl_entry->udf_value[0];
            attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->udf_mask[0];
            attr_count++;
        }
#endif
        if (GLB_ACL_UDF_GROUP_UDF_ID == acl_entry->udf_type)
        {
            /* udf-id and 4*u32 udf value */
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_UDF_ID;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u8 = acl_entry->udf_id;
            attr_count++;
        }

        uint32 iter = 0;
        sai_object_id_t  udf_value_list_obj[GLB_ACL_UDF_FIELD_NUM_MAX*3] = {0};
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_UDF_MATCH;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.objlist.count = GLB_ACL_UDF_FIELD_NUM_MAX*3;
        attr_list[attr_count].value.aclfield.data.objlist.list  = udf_value_list_obj;
        for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
        {
            udf_value_list_obj[3*iter]   = acl_entry->udf_value[iter];
            udf_value_list_obj[3*iter+1] = acl_entry->udf_mask[iter];
            udf_value_list_obj[3*iter+2] = acl_entry->udf_value_valid[iter];
        }
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_UDF_TYPE;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = acl_entry->udf_type;
        attr_count++;
    }

    if (acl_entry->deny)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.s32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }

    if (acl_entry->tap_action_redirect)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = tap_group->tap_group_oid;
        attr_count++;

        if (acl_entry->tap_action_mark_vlan)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_VLAN_ID;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.u16 = acl_entry->tap_action_mark_vlan;
            attr_count++;
        }
        if (acl_entry->tap_action_untag)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_UNTAG;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->tap_action_untag;
            attr_count++;
        }
        
        if (acl_entry->tap_action_trunction)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TRUNCATION;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.u8 = TRUE;
            attr_count++;
        }

        if (acl_entry->tap_action_edit_dest_mac_en)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            sal_memcpy(attr_list[attr_count].value.aclaction.parameter.mac,
                acl_entry->tap_action_edit_dest_mac,
                sizeof(sai_mac_t));
            attr_count++;
        }

        if (acl_entry->tap_action_edit_src_mac_en)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            sal_memcpy(attr_list[attr_count].value.aclaction.parameter.mac,
                acl_entry->tap_action_edit_src_mac,
                sizeof(sai_mac_t));
            attr_count++;
        }

        if (acl_entry->tap_action_edit_ipda_en)
        {
            if (AF_INET == acl_entry->tap_action_edit_ipda.family)
            {
                attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP;
                attr_list[attr_count].value.aclaction.enable = TRUE;
                attr_list[attr_count].value.aclaction.parameter.ip4 
                    = sal_htonl(acl_entry->tap_action_edit_ipda.u.prefix4.s_addr);
            }
            else
            {
                attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPv6;
                attr_list[attr_count].value.aclaction.enable = TRUE; 
                sal_memcpy(&attr_list[attr_count].value.aclaction.parameter.ip6, 
                    &acl_entry->tap_action_edit_ipda.u.prefix6, sizeof(addr_ipv6_t));
            }
            attr_count++;
        }

        if (acl_entry->tap_action_edit_ipsa_en)
        {
            if (AF_INET == acl_entry->tap_action_edit_ipsa.family)
            {
                attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP;
                attr_list[attr_count].value.aclaction.enable = TRUE;
                attr_list[attr_count].value.aclaction.parameter.ip4 
                    = sal_htonl(acl_entry->tap_action_edit_ipsa.u.prefix4.s_addr);
            }
            else
            {
                attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPv6;
                attr_list[attr_count].value.aclaction.enable = TRUE; 
                sal_memcpy(&attr_list[attr_count].value.aclaction.parameter.ip6, 
                    &acl_entry->tap_action_edit_ipsa.u.prefix6, sizeof(addr_ipv6_t));
            }
            attr_count++;
        }

        if (acl_entry->tap_action_edit_vlan_en)
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.u16 
                = acl_entry->tap_action_edit_vlan;
            attr_count++;

			attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID_ACTION;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.s32 
                = acl_entry->tap_action_edit_vlan_action;
            attr_count++;
        }
    }

    /*added by yejl for bug 53475, 2019-09-10*/    
    if (acl_entry->l2gre_add_header)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_HEADER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = 1;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_SRC_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->l2gre_src_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->l2gre_dst_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_MAC;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclaction.parameter.mac, acl_entry->l2gre_dst_mac, sizeof(sai_mac_t));
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_NUM;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->l2gre_key_num;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_LEN;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->l2gre_key_len;
        attr_count++;
    }

    if (acl_entry->l3gre_add_header)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_HEADER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = 1;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_SRC_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->l3gre_src_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->l3gre_dst_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_MAC;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclaction.parameter.mac, acl_entry->l3gre_dst_mac, sizeof(sai_mac_t));
        attr_count++;
    }
    /*added by yejl for bug 53475, 2019-09-10*/

    /*added by guoxd for vxlan*/
    if (acl_entry->add_vxlan_header)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_HEADER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = 1;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_MAC;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        sal_memcpy(attr_list[attr_count].value.aclaction.parameter.mac, acl_entry->vxlan_dst_mac, sizeof(sai_mac_t));
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRC_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->vxlan_src_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_IP;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.ip4 = sal_htonl(acl_entry->vxlan_dst_ip.u.prefix4.s_addr);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRCPORT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->vxlan_srcport;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DESTPORT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->vxlan_destport;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_VNI;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->vxlan_vni;
        attr_count++;
    }
    /*ended by guoxd*/
    
    if (acl_entry->action_strip_header)
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.u32 = 1;
        attr_count++;

        if (GLB_FLAG_ISSET(acl_entry->key_flags2, GLB_IP_ACE_STRIP_HEADER_POS))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER_POS;
            attr_list[attr_count].value.aclaction.enable = TRUE;

            switch(acl_entry->strip_header_pos){
            case GLB_ACL_STRIP_HEADER_POS_L2:
                attr_list[attr_count].value.aclaction.parameter.u32 = SAI_ACL_STRIP_HEADER_POS_L2;
                break;
            case GLB_ACL_STRIP_HEADER_POS_L3:
                attr_list[attr_count].value.aclaction.parameter.u32 = SAI_ACL_STRIP_HEADER_POS_L3;
                break;
            default:
                attr_list[attr_count].value.aclaction.parameter.u32 = SAI_ACL_STRIP_HEADER_POS_L4;
                break;
            }
            
            attr_count++;  
        }

        if (GLB_FLAG_ISSET(acl_entry->key_flags2, GLB_IP_ACE_STRIP_HEADER_OFFSET))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER_OFFSET;
            attr_list[attr_count].value.aclaction.enable = TRUE;
            attr_list[attr_count].value.aclaction.parameter.u32 = acl_entry->strip_header_offset;
            attr_count++;  
        }

	if (GLB_FLAG_ISSET(acl_entry->key_flags2, GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER))
        {
            attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_INNER_VXLAN_HEADER;
            attr_list[attr_count].value.aclaction.enable = TRUE;
	     attr_list[attr_count].value.aclaction.parameter.u32 = 1;
            attr_count++;  
        }
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_VXLAN_VNI))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_VXLAN_VNI;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->l4_vxlan_vni;
        attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->l4_vxlan_vni_mask;
        attr_count++;
    }

    if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_GRE_KEY))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_GRE_KEY;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->l4_gre_key;
        attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->l4_gre_key_mask;
        attr_count++;
    }

#ifdef TAPPRODUCT
    if (IS_ACL_TAP_DECAP_NO_INMATCH_V6(acl_entry->key.aclid) && 
        !GLB_FLAG_ISSET(acl_entry->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH)
        && GLB_FLAG_ISSET(acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        fea_acl_key.acl_id = acl_entry->key.aclid & (~(0x100000000ULL));
        pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
        if (pst_fea_acl)
        {
            acl_counter_id = pst_fea_acl->counter_id;
        }
    }
    else
    {
        /* create counter id */
        counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
        counter_attr.value.oid = pst_fea_acl_table->sai_table_id;
        acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr);
    }

#else   

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = pst_fea_acl_table->sai_table_id;
    HSRV_IF_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

#endif /* !TAPPRODUCT */

    if (GLB_FLAG_ISSET(acl_entry->key_flags2, GLB_IP_ACE_ERSPAN_ID))
    {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ERSPAN_ID;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u32 = acl_entry->erspan_id;
		attr_list[attr_count].value.aclfield.mask.u32 = acl_entry->erspan_id_mask;
        attr_count++;
    }

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    /*added by yejl for ipv6 profile, default profile*/
    if (TRUE == hsrv_acl_stm_is_ipv6_profile())
    {
        if (acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (GLB_FLAG_ISSET(acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) ||
                GLB_FLAG_ISSET(acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE))
            {
                HSRV_IF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));

                sal_memset(&fea_acl, 0, sizeof(fea_acl));
                fea_acl.key.acl_id = acl_entry->key.aclid;
                fea_acl.sai_acl_id = acl_entry_id;
                fea_acl.counter_id = acl_counter_id;
                HSRV_IF_ERROR_RETURN(tbl_fea_acl_add_fea_acl(&fea_acl));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id_ipv4, attr_count, attr_list));

                sal_memset(attr_list1, 0, sizeof(attr_list1));
                sal_memcpy(&attr_list1, &attr_list, sizeof(attr_list));
                
                attr_list1[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
                attr_list1[attr_count].value.aclfield.enable = TRUE;
                attr_count++;
                HSRV_IF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id_ipv6, attr_count, attr_list1));

                sal_memset(&fea_acl, 0, sizeof(fea_acl));
                fea_acl.key.acl_id = acl_entry->key.aclid;
                fea_acl.is_mac_acl = TRUE;
                fea_acl.sai_acl_id_ipv4 = acl_entry_id_ipv4;
                fea_acl.sai_acl_id_ipv6 = acl_entry_id_ipv6;
                fea_acl.counter_id = acl_counter_id;
                HSRV_IF_ERROR_RETURN(tbl_fea_acl_add_fea_acl(&fea_acl));
            }
            
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));

            sal_memset(&fea_acl, 0, sizeof(fea_acl));
            fea_acl.key.acl_id = acl_entry->key.aclid;
            fea_acl.sai_acl_id = acl_entry_id;
            fea_acl.counter_id = acl_counter_id;
            HSRV_IF_ERROR_RETURN(tbl_fea_acl_add_fea_acl(&fea_acl));
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));

        sal_memset(&fea_acl, 0, sizeof(fea_acl));
        fea_acl.key.acl_id = acl_entry->key.aclid;
        fea_acl.sai_acl_id = acl_entry_id;
        fea_acl.counter_id = acl_counter_id;
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_add_fea_acl(&fea_acl));
    }
    /*ended*/

    return HSRV_E_NONE;
}

int32
_fea_acl_get_counter_id(tbl_fea_acl_t *p_fea_acl, tbl_iter_args_t *pargs)
{
    tbl_fea_acl_t *pst_fea_acl_entry = pargs->argv[0];
    int32 *p_count = pargs->argv[1];
    if (pst_fea_acl_entry->counter_id == p_fea_acl->counter_id)
    {
        *p_count += 1;
    }
    return PM_E_NONE;
}

int32
_fea_acl_tbl_counter_id_used_num(tbl_fea_acl_t *p_fea_acl)
{
    tbl_iter_args_t args;
    int32 count = 0;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_fea_acl;
    args.argv[1] = &count;
    tbl_fea_acl_iterate((TBL_ITER_CB_FUNC)_fea_acl_get_counter_id, &args);

    return count;
}

static int32
_hsrv_acl_del_entry(tbl_acl_entry_key_t *p_acl_entry_key)
{
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_acl_api_t *acl_api = NULL;
    tbl_acl_entry_t *p_acl_entry = NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry_key->aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    p_acl_entry = tbl_acl_entry_get_acl_entry(p_acl_entry_key);
    if (NULL == pst_fea_acl)
    {
        if (p_acl_entry && GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TIME_RANGE))
        {
            return HSRV_E_NONE;
        }
        else
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
    }

    if (_fea_acl_tbl_counter_id_used_num(pst_fea_acl) <= 1)
    {
        HSRV_IF_ERROR_RETURN(acl_api->delete_acl_counter(pst_fea_acl->counter_id));
    }
    
    if (pst_fea_acl->is_mac_acl)
    {
        HSRV_IF_ERROR_RETURN(acl_api->delete_acl_entry(pst_fea_acl->sai_acl_id_ipv4));
        HSRV_IF_ERROR_RETURN(acl_api->delete_acl_entry(pst_fea_acl->sai_acl_id_ipv6));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(acl_api->delete_acl_entry(pst_fea_acl->sai_acl_id));
    }
   
    HSRV_IF_ERROR_RETURN(tbl_fea_acl_del_fea_acl(&fea_acl_key));
    return HSRV_E_NONE;
}

static int32
_hsrv_acl_update_entry(tbl_acl_entry_t *p_acl_entry, uint32 field_id)
{
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    if(field_id == TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY)
    {
        attr.value.u32 = 0xffffffff - p_acl_entry->entry_priority;
    }
    if (pst_fea_acl->is_mac_acl)
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));
        
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
        if(field_id == TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY)
        {
            attr.value.u32 = 0xffffffff - p_acl_entry->entry_priority;
        }
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_acl_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_acl_entry_key_t* p_acl_entry_key = NULL;
    tbl_acl_entry_t *p_acl_entry         = NULL;
    tbl_acl_entry_action_t *p_acl_entry_action = NULL;
    tbl_acl_entry_action_key_t acl_entry_action_key;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_entry = (tbl_acl_entry_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(_hsrv_acl_add_entry(p_acl_entry));
        break;

    case CDB_OPER_DEL:
        p_acl_entry_key = (tbl_acl_entry_key_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(_hsrv_acl_del_entry(p_acl_entry_key));
        break;
    case CDB_OPER_SET:
        p_acl_entry = (tbl_acl_entry_t*)p_tbl;
        if(TBL_ACL_ENTRY_FLD_INVALID == field_id)
        {
            if(p_acl_entry->invalid)
            {
                /* added by hansf for time range update acl policy action for bug 47270 and 47268*/
                sal_memset(&acl_entry_action_key, 0, sizeof(acl_entry_action_key));
                acl_entry_action_key.aclid = p_acl_entry->key.aclid;
                p_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(&acl_entry_action_key);
                if(p_acl_entry_action)
                {
                    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_POLICER))
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_acl_del_action_policer_field(p_acl_entry_action));
                    }
                }
                /* end hansf */
                HSRV_IF_ERROR_RETURN(_hsrv_acl_del_entry(&p_acl_entry->key));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(_hsrv_acl_add_entry(p_acl_entry));

                /* added by hansf for time range update acl policy action for bug 47270 and 47268 */
                HSRV_IF_ERROR_RETURN(_hsrv_acl_update_entry_action(p_acl_entry->key.aclid));
            }
        }
        else
        {
            HSRV_IF_ERROR_RETURN(_hsrv_acl_update_entry(p_acl_entry, field_id));
        }
    default:
        break;
    }

    return HSRV_E_NONE;
}


int32
hsrv_acl_policer_creat_policer(tbl_qos_policer_profile_t * p_qos_policer_profile, uint64* policer_oid)
{
    sai_object_id_t policer_id;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[16] = {{0}};
    uint32_t attr_count = 0;
    tbl_qos_policer_action_profile_t *p_db_action_profile = NULL;

    sal_memset(attr, 0x0, sizeof(attr));
    attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
    }
    else
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
    }
    attr_count ++;

    if(GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Sr_TCM;
        attr_count ++;
    }
    else if(GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Tr_TCM;
        attr_count ++;
    }
    else
    {
        return -1;
    }
    
    if(p_qos_policer_profile->color_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;
        attr_count ++;
    }
    else
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
        attr_count ++;
    }


    attr[attr_count].id = SAI_POLICER_ATTR_CIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_CBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->cbs;
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->ebs_or_pbs;
    attr_count ++;

    if(p_qos_policer_profile->flags & GLB_POLICER_PROFILE_FLAGS_STATS)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
        attr[attr_count].value.s32 = 1;
        attr_count ++;
    }

    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_qos_policer_profile->action_profile_name);
    if (p_db_action_profile)
    {
        hsrv_qos_policer_action_map_sai_type(attr, &attr_count, p_db_action_profile);
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_id,attr_count,attr));

    *policer_oid = policer_id;

    return HSRV_E_NONE;
}


int32 hsrv_acl_set_action_policer_field(tbl_acl_entry_action_t* p_acl_entry_action)
{
    uint64 policer_oid = 0;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    sai_attribute_t     attr;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_policy_action_t* pst_fea_acl_policy_action = NULL;
    tbl_fea_acl_policy_action_t   fea_acl_policy_action;
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;

    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry_action->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(fea_acl_policy_action_key));
    sal_memset(&fea_acl_policy_action, 0x0, sizeof(fea_acl_policy_action));
    
    fea_acl_policy_action_key.acl_policy_action_id = MAKE_FEA_ACL_POLICY_ACTION_ID(p_acl_entry_action->key.aclid);
    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
    if(NULL == pst_fea_acl_policy_action)
    {
        sal_memcpy(&fea_acl_policy_action.key, &fea_acl_policy_action_key, sizeof(tbl_fea_acl_policy_action_key_t));
        tbl_fea_acl_policy_action_add_fea_acl_policy_action(&fea_acl_policy_action);
        pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
        
        p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_acl_entry_action->policer);
        HSRV_IF_ERROR_RETURN(hsrv_acl_policer_creat_policer(p_qos_policer_profile, &policer_oid));

        pst_fea_acl_policy_action->policer_id = policer_oid;
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID);
        sal_strcpy(pst_fea_acl_policy_action->policer_name, p_acl_entry_action->policer);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME);
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    attr.value.aclaction.enable = TRUE;
    attr.value.aclaction.parameter.oid = pst_fea_acl_policy_action->policer_id;
    if (pst_fea_acl->is_mac_acl)
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

        sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.oid = pst_fea_acl_policy_action->policer_id;
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
    }

    pst_fea_acl_policy_action->policer_id_ref ++;
    tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF);

    return HSRV_E_NONE;
}

int32 hsrv_acl_del_action_policer_field(tbl_acl_entry_action_t* p_acl_entry_action)
{
    uint64 policer_oid = 0;
    sai_attribute_t     attr;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_policer_api_t*  policer_api;
    tbl_fea_acl_policy_action_t* pst_fea_acl_policy_action = NULL;
    tbl_fea_acl_policy_action_t   fea_acl_policy_action;
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;
    
    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(fea_acl_policy_action_key));
    sal_memset(&fea_acl_policy_action, 0x0, sizeof(fea_acl_policy_action_key));

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry_action->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return HSRV_E_NONE;
    }
    fea_acl_policy_action_key.acl_policy_action_id = MAKE_FEA_ACL_POLICY_ACTION_ID(p_acl_entry_action->key.aclid);
    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
    if (NULL == pst_fea_acl_policy_action)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
        
    policer_oid = pst_fea_acl_policy_action->policer_id;
    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    attr.value.aclaction.enable = FALSE;
    attr.value.aclaction.parameter.oid = policer_oid;
    if (pst_fea_acl->is_mac_acl)
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

        sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
        attr.value.aclaction.enable = FALSE;
        attr.value.aclaction.parameter.oid = policer_oid;
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
    }

    pst_fea_acl_policy_action->policer_id_ref --;
    tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF);

    if(0 == pst_fea_acl_policy_action->policer_id_ref)
    {
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
        HSRV_IF_ERROR_RETURN(policer_api->remove_policer(pst_fea_acl_policy_action->policer_id));

        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_del_fea_acl_policy_action(&fea_acl_policy_action_key));
    }
    return HSRV_E_NONE;
}

static int32
_hsrv_acl_update_entry_action(uint64 aclid)
{
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    tbl_acl_entry_action_t *p_acl_entry_action = NULL;
    tbl_acl_entry_action_key_t acl_action_key;
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;
       
    sal_memset(&fea_acl_key, 0x0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return HSRV_E_NONE;
    }

    sal_memset(&acl_action_key, 0x0, sizeof(acl_action_key));
    acl_action_key.aclid = aclid;
    p_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(&acl_action_key);
    if (NULL == p_acl_entry_action)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));

    if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_POLICER))
    {
        hsrv_acl_set_action_policer_field(p_acl_entry_action);
    }

    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_COLOR))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC_AND_COLOR;

        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.tcandcolor.color = (uint16_t)sys_to_sai_color_map[p_acl_entry_action->color];
        attr.value.aclaction.parameter.tcandcolor.tc = (uint16_t)p_acl_entry_action->tc;
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));
            
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC_AND_COLOR;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.tcandcolor.color = (uint16_t)sys_to_sai_color_map[p_acl_entry_action->color];
            attr.value.aclaction.parameter.tcandcolor.tc = (uint16_t)p_acl_entry_action->tc;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }
            
    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_DSCP))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;

        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.u8 = p_acl_entry_action->dscp;
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));
            
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.u8 = p_acl_entry_action->dscp;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }

    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION))
    {
        if (p_acl_entry_action->key.aclid & (0x8000ULL))
        {
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
        }
        else
        {
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
        }
    
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
        if (pst_fea_acl->is_mac_acl)
        {   
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));
            sal_memset(&attr, 0, sizeof(sai_attribute_t));

            if (p_acl_entry_action->key.aclid & (0x8000ULL))
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
            }
            else
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
            }
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }

    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID;
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_svlan_id;
        HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_svlan_id;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }

    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID;
        
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_cvlan_id;
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_cvlan_id;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
         
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }

    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI;
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_scos;
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_scos;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }
    
  if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI;
        attr.value.aclaction.enable = TRUE;
        attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_ccos;
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_ccos;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
        
        sal_memset(&attr, 0, sizeof(sai_attribute_t));
    }

            
    if (GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
    {
        attr.id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        
        hsrv_interface_ifindex_to_objectid(p_acl_entry_action->redirect_port_ifindex, &attr.value.aclaction.parameter.oid);
        attr.value.aclaction.enable = TRUE;
        if (pst_fea_acl->is_mac_acl)
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
            hsrv_interface_ifindex_to_objectid(p_acl_entry_action->redirect_port_ifindex, &attr.value.aclaction.parameter.oid);
            attr.value.aclaction.enable = TRUE;
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
        }
    }
    
    return HSRV_E_NONE; 
}

static int32
_hsrv_acl_add_entry_action(tbl_acl_entry_action_t* p_acl_entry_action, uint32 field_id)
{
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);
    if(TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG == field_id)
    {
        return HSRV_E_NONE;
    }

    sal_memset(&fea_acl_key, 0x0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry_action->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    switch(field_id)
    {
        case TBL_ACL_ENTRY_ACTION_FLD_TC:
            /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
//            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
//            if(GLB_FLAG_ISSET(acl_entry_action->action_flag, GLB_ACL_SET_TC))
//            {
//                attr.value.aclaction.enable = TRUE;
//                attr.value.aclaction.parameter.u8 = acl_entry_action->tc;
//            }
//            else
//            {
//                attr.value.aclaction.enable = FALSE;
//            }
//            HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_POLICER:
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_POLICER))
            {
                hsrv_acl_set_action_policer_field(p_acl_entry_action);
            }
            else
            {
                hsrv_acl_del_action_policer_field(p_acl_entry_action);
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_COLOR:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC_AND_COLOR;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_COLOR))
            {

                attr.value.aclaction.enable = TRUE;
                /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
//              <<<<<<<< OLD
//                attr.value.aclaction.parameter.s32 = sys_to_sai_color_map[acl_entry_action->color];
//              >>>>>>>> NEW
                attr.value.aclaction.parameter.tcandcolor.color = (uint16_t)sys_to_sai_color_map[p_acl_entry_action->color];
                attr.value.aclaction.parameter.tcandcolor.tc = (uint16_t)p_acl_entry_action->tc;
//              ======== END
                /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC_AND_COLOR;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_COLOR))
                {

                    attr.value.aclaction.enable = TRUE;
                    /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
    //              <<<<<<<< OLD
    //                attr.value.aclaction.parameter.s32 = sys_to_sai_color_map[acl_entry_action->color];
    //              >>>>>>>> NEW
                    attr.value.aclaction.parameter.tcandcolor.color = (uint16_t)sys_to_sai_color_map[p_acl_entry_action->color];
                    attr.value.aclaction.parameter.tcandcolor.tc = (uint16_t)p_acl_entry_action->tc;
    //              ======== END
                    /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_DSCP:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_DSCP))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.u8 = p_acl_entry_action->dscp;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_DSCP))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.u8 = p_acl_entry_action->dscp;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID:
            if(p_acl_entry_action->key.aclid & (0x8000ULL))
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
            }
            else
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
            }
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                if(p_acl_entry_action->key.aclid & (0x8000ULL))
                {
                    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
                }
                else
                {
                    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
                }
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_svlan_id;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_svlan_id;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_cvlan_id;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.u16 = p_acl_entry_action->new_cvlan_id;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_scos;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_scos;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_ccos;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.u8 = p_acl_entry_action->new_ccos;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        case TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX:
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
            if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
            {
                hsrv_interface_ifindex_to_objectid(p_acl_entry_action->redirect_port_ifindex, &attr.value.aclaction.parameter.oid);
                attr.value.aclaction.enable = TRUE;
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr)); 

                sal_memset(&attr, 0, sizeof(attr));
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
                if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
                {
                    hsrv_interface_ifindex_to_objectid(p_acl_entry_action->redirect_port_ifindex, &attr.value.aclaction.parameter.oid);
                    attr.value.aclaction.enable = TRUE;
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
            break;
        default:
            break;
    }
    

    return HSRV_E_NONE;
}

int32
hsrv_acl_update_action_by_mirror(tbl_acl_entry_action_t *p_acl_entry_action, tbl_iter_args_t *pargs)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;

    uint32 *sessin_id              = pargs->argv[0];
    uint8 *is_add                  =  pargs->argv[1];

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    
    if(GLB_FLAG_ISSET(p_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION))
    {
        if(p_acl_entry_action->session_id == *sessin_id)
        {
            fea_acl_key.acl_id = p_acl_entry_action->key.aclid;
            pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
            if (NULL == pst_fea_acl)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            if(p_acl_entry_action->key.aclid & (0x8000ULL))
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
            }
            else
            {
                attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
            }
            if(*is_add)
            {
                attr.value.aclaction.enable = TRUE;
                attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
            }
            else
            {
                attr.value.aclaction.enable = FALSE;
            }
            if (pst_fea_acl->is_mac_acl)
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv4, &attr));

                sal_memset(&attr, 0, sizeof(sai_attribute_t));
                if(p_acl_entry_action->key.aclid & (0x8000ULL))
                {
                    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
                }
                else
                {
                    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
                }
                if(*is_add)
                {
                    attr.value.aclaction.enable = TRUE;
                    attr.value.aclaction.parameter.oid = g_hsrv_mirror_sai_sid_obj_array[p_acl_entry_action->session_id];
                }
                else
                {
                    attr.value.aclaction.enable = FALSE;
                }
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id_ipv6, &attr));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(acl_api->set_acl_entry_attribute(pst_fea_acl->sai_acl_id, &attr));
            }
        }
    }

    return PM_E_NONE;
}

int32
hsrv_acl_entry_action_update_by_mirror(uint32 sessin_id, uint8 is_add)
{
    
    tbl_iter_args_t args;

    args.argv[0] = &sessin_id;
    args.argv[1] = &is_add;
    tbl_acl_entry_action_iterate((TBL_ITER_CB_FUNC)hsrv_acl_update_action_by_mirror, &args);

    return PM_E_NONE;
}

int32
hsrv_acl_entry_action_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_acl_entry_action_t *p_acl_action = NULL;
    tbl_acl_entry_action_t *p_db_acl_action = NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_SET:
        p_acl_action = (tbl_acl_entry_action_t*)p_tbl;
        _hsrv_acl_add_entry_action(p_acl_action, field_id);
        break;
    case CDB_OPER_DEL:
        /*added by hansf for bug 40670*/
        p_acl_action = (tbl_acl_entry_action_t*)p_tbl;
        p_db_acl_action = tbl_acl_entry_action_get_acl_entry_action(&p_acl_action->key);
        if(p_db_acl_action)
        {
            if(GLB_FLAG_ISSET(p_db_acl_action->action_flag, GLB_ACL_SET_POLICER))
            {
                HSRV_IF_ERROR_RETURN(hsrv_acl_del_action_policer_field(p_db_acl_action));
            }
        }
        /*ended by hansf*/
        break;
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
_acl_table_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_fea_acl_table_iterate((TBL_ITER_CB_FUNC)tbl_fea_acl_table_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
acl_table_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_FEA_ACL_TABLE);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _acl_table_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;    
}




int32
_fea_acl_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_fea_acl_iterate((TBL_ITER_CB_FUNC)tbl_fea_acl_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
tbl_fea_acl_update_stats(tbl_fea_acl_t *p_fea_acl)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr[2];
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    acl_api->get_acl_counter_attribute(p_fea_acl->counter_id, 2, attr);

    p_fea_acl->stats_byte = attr[0].value.u64;
    p_fea_acl->stats_packet = attr[1].value.u64;
    tbl_fea_acl_set_fea_acl_field(p_fea_acl, TBL_FEA_ACL_FLD_STATS_PACKET);
    tbl_fea_acl_set_fea_acl_field(p_fea_acl, TBL_FEA_ACL_FLD_STATS_BYTE);

    return PM_E_NONE;
}

int32
_fea_acl_cmd_update_session_stats_all(cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    tbl_fea_acl_iterate((TBL_ITER_CB_FUNC)tbl_fea_acl_update_stats, &args);

    return PM_E_NONE;
}

int32
fea_acl_action_update_by_policer_profile(tbl_fea_acl_t *p_fea_acl, tbl_iter_args_t *pargs)
{
    sai_attribute_t* pst_attr = pargs->argv[0];
    char* policer_profile_name = pargs->argv[1];
    sai_policer_api_t*  policer_api;
    tbl_fea_acl_policy_action_t* pst_fea_acl_policy_action = NULL;
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;

    sal_memset(&fea_acl_policy_action_key, 0, sizeof(fea_acl_policy_action_key));
    fea_acl_policy_action_key.acl_policy_action_id = MAKE_FEA_ACL_POLICY_ACTION_ID(p_fea_acl->key.acl_id);
    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
    if(NULL == pst_fea_acl_policy_action)
    {
        return PM_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    if((!sal_strcmp(pst_fea_acl_policy_action->policer_name, policer_profile_name)))
    {
        HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(pst_fea_acl_policy_action->policer_id, pst_attr));
    }

    return PM_E_NONE;
}

int32
_fea_acl_action_update_by_policer_profile(sai_attribute_t *pst_attr, char* policer_profile_name)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = pst_attr;
    args.argv[1] = policer_profile_name;
    tbl_fea_acl_iterate((TBL_ITER_CB_FUNC)fea_acl_action_update_by_policer_profile, &args);

    return PM_E_NONE;
}

int32
fea_acl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_FEA_ACL);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _fea_acl_cmd_read_session_all(para);
        break;

    case CDB_OPER_SET:
        _fea_acl_cmd_update_session_stats_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_update_stats(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    sai_policer_api_t*  policer_api;
    sai_policer_stat_counter_t counter_ids[8];
    uint32_t number_of_counters = 0;
    uint64_t counters[8];
    sai_object_id_t policer_id;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;


    if(0 != p_fea_acl_policy_action->policer_id)
    {
        sal_memset(&counters, 0x0,(sizeof(uint64_t))*8);
        sal_memset(&counter_ids, 0x0,(sizeof(sai_policer_stat_counter_t))*8);

        p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_fea_acl_policy_action->policer_name);
        if(NULL == p_db_qos_policer_profile)
        {
            return PM_E_NONE;
        }
        if(!GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
        {
            return PM_E_NONE;
        }
        policer_id = p_fea_acl_policy_action->policer_id;

        counter_ids[number_of_counters] = SAI_POLICER_STAT_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_ATTR_BYTES;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_BYTES;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_BYTES;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_BYTES;
        number_of_counters ++;

        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

        HSRV_IF_ERROR_RETURN(policer_api->get_policer_statistics(policer_id, counter_ids, number_of_counters, counters));

        p_fea_acl_policy_action->green_packet = counters[2];
        p_fea_acl_policy_action->green_byte = counters[3];

        p_fea_acl_policy_action->yellow_packet = counters[4];
        p_fea_acl_policy_action->yellow_byte = counters[5];

        p_fea_acl_policy_action->red_packet = counters[6];
        p_fea_acl_policy_action->red_byte = counters[7];

        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(p_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE));
        
    }

    return PM_E_NONE;
}

int32
_fea_acl_policy_cmd_update_policer_stats_all(cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    tbl_fea_acl_policy_action_iterate((TBL_ITER_CB_FUNC)tbl_fea_acl_policy_update_stats, &args);

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_delete_stats(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    sai_policer_api_t*  policer_api;
    sai_object_id_t policer_id;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;


    if(0 != p_fea_acl_policy_action->policer_id)
    {
        p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_fea_acl_policy_action->policer_name);
        if(NULL == p_db_qos_policer_profile)
        {
            return PM_E_NONE;
        }
        if(!GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
        {
            return PM_E_NONE;
        }
        policer_id = p_fea_acl_policy_action->policer_id;

        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

        HSRV_IF_ERROR_RETURN(policer_api->clear_policer_statistics(policer_id));
    }

    return PM_E_NONE;
}

int32
_fea_acl_policy_cmd_delete_policer_stats_all(cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    tbl_fea_acl_policy_action_iterate((TBL_ITER_CB_FUNC)tbl_fea_acl_policy_delete_stats, &args);

    return PM_E_NONE;
}


int32
fea_acl_policy_action_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;

    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_FEA_ACL_POLICY_ACTION);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    field_id = cdb_get_field_id(argv[0], p_node, &p_field);   
    switch (para->oper)
    {
    case CDB_OPER_GET:
        break;

    case CDB_OPER_SET:
        if(field_id == TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS)
        {
            _fea_acl_policy_cmd_update_policer_stats_all(para);
        }
        break;

    case CDB_OPER_DEL:
        if(field_id == TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS)
        {
            _fea_acl_policy_cmd_delete_policer_stats_all(para);
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
_acl_show_acl_counter_in(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_interface = NULL;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr[2];
    FILE *cfg_fp = pargs->argv[0];
    char *ifname = pargs->argv[1];
    uint64 *total_bytes = pargs->argv[3];
    uint64 *total_packets = pargs->argv[4];
    char *acl_str = pargs->argv[5];
    uint32 type = *(uint32*)pargs->argv[6];
    char tmp_str[50];
    char buf1[256];
    char buf2[256];
    addr_ipv4_t tmp_ip;
    addr_ipv4_t tmp_mask;
    uint8 flag = 0;

    if (NULL == ifname)
    {
        return PM_E_NONE;
    }

    p_interface = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_interface)
    {
        return PM_E_NONE;
    }

    /*added by yejl for IP/IPv6/MAC acl*/
    if (p_acl_entry->acl_type != type)
    {
        return PM_E_NONE;
    }
    /*ended by yejl*/

    if(IS_ACL_POLICY_ID(p_acl_entry->key.aclid))
    {
        return PM_E_NONE;
    }

    /*added by yejl for IP/IPv6/MAC acl*/
    if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
    {
        if (0 == sal_strlen(p_interface->igs_acl))
        {
            return PM_E_NONE;
        }
    }
    else if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
    {
        if (0 == sal_strlen(p_interface->igs_ipv6_acl))
        {
            return PM_E_NONE;
        }
    }
    else if (GLB_ACL_CONFIG_TYPE_MAC == type)
    {
        if (0 == sal_strlen(p_interface->igs_mac_acl))
        {
            return PM_E_NONE;
        }
    }
    /*ended by yejl*/

    if (p_interface->ifindex == p_acl_entry->in_port)
    {
        /*added by yejl for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            sal_snprintf(tmp_str, 50, "ip access-list %s in\n", p_interface->igs_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "ip access-list %s in\n", p_interface->igs_acl);
                sal_snprintf(acl_str, 50, "ip access-list %s in\n", p_interface->igs_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "ip access-list %s in\n", p_interface->igs_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            sal_snprintf(tmp_str, 50, "ipv6 access-list %s in\n", p_interface->igs_ipv6_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "ipv6 access-list %s in\n", p_interface->igs_ipv6_acl);
                sal_snprintf(acl_str, 50, "ipv6 access-list %s in\n", p_interface->igs_ipv6_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "ipv6 access-list %s in\n", p_interface->igs_ipv6_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            sal_snprintf(tmp_str, 50, "mac access-list %s in\n", p_interface->igs_mac_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "mac access-list %s in\n", p_interface->igs_mac_acl);
                sal_snprintf(acl_str, 50, "mac access-list %s in\n", p_interface->igs_mac_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "mac access-list %s in\n", p_interface->igs_mac_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        /*ended by yejl*/
        
    }
    else
    {
        return PM_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, " %"PRIu64" %s", ((p_acl_entry->key.aclid >> 16) & 0xffff),
        (p_acl_entry->deny ? "deny" : "permit"));

    if (0 == p_acl_entry->key_flags && 0 == p_acl_entry->key_ipv6_flags && 0 == p_acl_entry->key_arp_flags)
    {
        sal_fprintf (cfg_fp, " any");
        acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
        sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
        *total_bytes += attr[0].value.u64;
        *total_packets += attr[1].value.u64;
        return PM_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACSA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->src_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->src_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACDA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->dst_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->dst_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_VLAN))
    {
        if (0xfff == p_acl_entry->svlan_mask)
        {
            sal_fprintf (cfg_fp, " vlan any");
        }
        else
        {
            sal_fprintf (cfg_fp, " vlan %d 0x%x", p_acl_entry->svlan, p_acl_entry->svlan_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        if (0xfff == p_acl_entry->cvlan_mask)
        {
            sal_fprintf (cfg_fp, " inner-vlan any");
        }
        else
        {
            sal_fprintf (cfg_fp, " inner-vlan %d 0x%x", p_acl_entry->cvlan, p_acl_entry->cvlan_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_COS))
    {
        if (0x7 == p_acl_entry->svlan_cos_mask)
        {
            sal_fprintf (cfg_fp, " cos any");
        }
        else
        {
            sal_fprintf (cfg_fp, " cos %d 0x%x", p_acl_entry->svlan_cos, p_acl_entry->svlan_cos_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_INNER_COS))
    {
        if (0x7 == p_acl_entry->cvlan_cos_mask)
        {
            sal_fprintf (cfg_fp, " inner-cos any");
        }
        else
        {
            sal_fprintf (cfg_fp, " inner-cos %d 0x%x", p_acl_entry->cvlan_cos, p_acl_entry->cvlan_cos_mask);
        }
    }

    /*added by yejl for bug 49791, 2018-11-14*/
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_UNTAG))
    {
        sal_fprintf (cfg_fp, " untag-vlan");
    }

    /*modified by yejl for bug 49768, 2018-10-31*/
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        sal_fprintf (cfg_fp, " ether-type 0x%x", p_acl_entry->ether_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        if (0xffff == p_acl_entry->arp_op_code_mask)
        {
            sal_fprintf (cfg_fp, " arp any");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_REQUEST == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp request");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RESPONSE == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp response");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RARP_REQUEST == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp rarp-request");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RARP_RESPONSE == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp rarp-response");
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_SENDER_IP))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " sender-ip any");
        }
        else if (0 == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " sender-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " sender-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_TARGET_IP))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " target-ip any");
        }
        else if (0 == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " target-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " target-ip %s %s", buf1, buf2);
        }
    }     

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPSA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip any");
        }
        else if (0 == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->src_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->src_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " src-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
        }
    }


    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPDA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dest-ip any");
        }
        else if (0 == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dest-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dest-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->dst_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->dst_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " dst-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        if (0xFFFFF == p_acl_entry->ipv6_flow_label_mask && 0 == p_acl_entry->ipv6_flow_label)
        {
            sal_fprintf (cfg_fp, " flow-label any");
        }
        else
        {
            sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_acl_entry->ipv6_flow_label, p_acl_entry->ipv6_flow_label_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSCP))
    {
        if (0x3f == p_acl_entry->dscp_mask)
        {
            sal_fprintf (cfg_fp, " dscp any");
        }
        else
        {
            sal_fprintf (cfg_fp, " dscp %d 0x%x", p_acl_entry->dscp, p_acl_entry->dscp_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        if (0x7 == p_acl_entry->ip_precedence_mask)
        {
            sal_fprintf (cfg_fp, " ip-precedence any");
        }
        else
        {
            sal_fprintf (cfg_fp, " ip-precedence %d 0x%x", p_acl_entry->ip_precedence, p_acl_entry->ip_precedence_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_FRAG))
    {
        /*modified by yejl for bug 49815, 2018-11-08*/
        sal_fprintf (cfg_fp, " fragment");
        if (GLB_ACL_IP_FRAG_FIRST_FRAG == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NON_FIRST == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NONE == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-or-first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_TINY == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " small-fragment");
        }
        else if (GLB_ACL_IP_FRAG_YES == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " any-fragment");
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        if (GLB_ACL_L4_PROTOCOL_TCP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " l3-protocol tcp");
        }
        else if (GLB_ACL_L4_PROTOCOL_UDP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " l3-protocol udp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_IGMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol igmp");
        }
        else
        {
            sal_fprintf (cfg_fp, " l3-protocol %d", p_acl_entry->ip_protocol);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port eq %d", p_acl_entry->l4_src_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port lt %d", p_acl_entry->l4_src_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port gt %d", p_acl_entry->l4_src_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port range %d %d", p_acl_entry->l4_src_port, p_acl_entry->l4_src_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port eq %d", p_acl_entry->l4_dst_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port lt %d", p_acl_entry->l4_dst_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port gt %d", p_acl_entry->l4_dst_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port range %d %d", p_acl_entry->l4_dst_port, p_acl_entry->l4_dst_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IGMP_TYPE))
    {
        if (ACL_IGMP_TYPE_HOST_DVMRP == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " dvmrp");
        }
        else if (ACL_IGMP_TYPE_HOST_QUERY == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " host-query");
        }
        else if (ACL_IGMP_TYPE_HOST_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v1host-report");
        }
        else if (ACL_IGMP_TYPE_MTRACT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " mtrace-route");
        }
        else if (ACL_IGMP_TYPE_MTRACT_RESPONSE == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " mtrace-response");
        }
        else if (ACL_IGMP_TYPE_V2_LEAVE == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v2leave-group");
        }
        else if (ACL_IGMP_TYPE_V2_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v2host-report");
        }
        else if (ACL_IGMP_TYPE_V3_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v3host-report");
        }  
        else
        {
            sal_fprintf (cfg_fp, " %d", p_acl_entry->igmp_type);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        sal_fprintf (cfg_fp, " icmp-type %d", p_acl_entry->icmp_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        sal_fprintf (cfg_fp, " icmp-code %d", p_acl_entry->icmp_code);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        if (GLB_ACL_TCP_FLAG_OP_ALL == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-flag match-all");
        }
        else if (GLB_ACL_TCP_FLAG_OP_ANY == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-flag match-any");
        }

        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
        {
            sal_fprintf (cfg_fp, " ack");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
        {
            sal_fprintf (cfg_fp, " fin");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
        {
            sal_fprintf (cfg_fp, " psh");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
        {
            sal_fprintf (cfg_fp, " rst");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
        {
            sal_fprintf (cfg_fp, " syn");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
        {
            sal_fprintf (cfg_fp, " urg");
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_UDF))
    {
        if (GLB_ACL_UDF_GROUP_L3_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else
        {
            sal_fprintf (cfg_fp, " udf udf-id %d", p_acl_entry->udf_id);
            uint32 iter = 0;
            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (p_acl_entry->udf_value_valid[iter])
                {
                    if (0xFFFFFFFF == p_acl_entry->udf_mask[iter])
                    {
                        sal_fprintf(cfg_fp, " udf%d any", iter);
                    }
                    else
                    {
                        sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                p_acl_entry->udf_value[iter], p_acl_entry->udf_mask[iter]);
                    }
                }
            }
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        sal_fprintf (cfg_fp, " time-range %s", p_acl_entry->time_range);
    }
    acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
    sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
    *total_bytes += attr[0].value.u64;
    *total_packets += attr[1].value.u64;
    return PM_E_NONE;
}

int32
_acl_show_acl_counter_out(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_interface = NULL;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr[2];
    FILE *cfg_fp = pargs->argv[0];
    char *ifname = pargs->argv[1];
    uint64 *total_bytes = pargs->argv[3];
    uint64 *total_packets = pargs->argv[4];
    char *acl_str = pargs->argv[5];
    uint32 type = *(uint32*)pargs->argv[6];
    char tmp_str[50];
    char buf1[256];
    char buf2[256];
    addr_ipv4_t tmp_ip;
    addr_ipv4_t tmp_mask;
    uint8 flag = 0;
    
    if (NULL == ifname)
    {
        return PM_E_NONE;
    }

    p_interface = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_interface)
    {
        return PM_E_NONE;
    }
    
    /*added by yejl for IP/IPv6/MAC acl*/
    if (p_acl_entry->acl_type != type)
    {
        return PM_E_NONE;
    }
    /*ended by yejl*/
    
    if(IS_ACL_POLICY_ID(p_acl_entry->key.aclid))
    {
        return PM_E_NONE;
    }

    /*added by yejl for IP/IPv6/MAC acl*/
    if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
    {
        if (0 == sal_strlen(p_interface->egs_acl))
        {
            return PM_E_NONE;
        }
    }
    else if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
    {
        if (0 == sal_strlen(p_interface->egs_ipv6_acl))
        {
            return PM_E_NONE;
        }
    }
    else if (GLB_ACL_CONFIG_TYPE_MAC == type)
    {
        if (0 == sal_strlen(p_interface->egs_mac_acl))
        {
            return PM_E_NONE;
        }
    }
    /*ended by yejl*/
    

    if (p_interface->ifindex == p_acl_entry->out_port)
    {
#ifdef TAPPRODUCT
        sal_snprintf(tmp_str, 50, "egress flow %s\n", p_interface->egs_acl);
        if (0 == sal_strlen(acl_str))
        {
            sal_fprintf (cfg_fp, "egress flow %s\n", p_interface->egs_acl);
            sal_snprintf(acl_str, 50, "egress flow %s\n", p_interface->egs_acl);
        }
        else if (0 != sal_strcmp(acl_str, tmp_str))
        {
            sal_fprintf (cfg_fp, "egress flow %s\n", p_interface->egs_acl);
            sal_strncpy(acl_str, tmp_str, 50);
        }
    
#else
        /*added by yejl for IP/IPv6/MAC acl*/
        if (GLB_ACL_CONFIG_TYPE_NORMAL == type)
        {
            sal_snprintf(tmp_str, 50, "ip access-list %s out\n", p_interface->egs_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "ip access-list %s out\n", p_interface->egs_acl);
                sal_snprintf(acl_str, 50, "ip access-list %s out\n", p_interface->egs_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "ip access-list %s out\n", p_interface->egs_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == type)
        {
            sal_snprintf(tmp_str, 50, "ipv6 access-list %s out\n", p_interface->egs_ipv6_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "ipv6 access-list %s out\n", p_interface->egs_ipv6_acl);
                sal_snprintf(acl_str, 50, "ipv6 access-list %s out\n", p_interface->egs_ipv6_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "ipv6 access-list %s out\n", p_interface->egs_ipv6_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == type)
        {
            sal_snprintf(tmp_str, 50, "mac access-list %s out\n", p_interface->egs_mac_acl);
            if (0 == sal_strlen(acl_str))
            {
                sal_fprintf (cfg_fp, "mac access-list %s out\n", p_interface->egs_mac_acl);
                sal_snprintf(acl_str, 50, "mac access-list %s out\n", p_interface->egs_mac_acl);
            }
            else if (0 != sal_strcmp(acl_str, tmp_str))
            {
                sal_fprintf (cfg_fp, "mac access-list %s out\n", p_interface->egs_mac_acl);
                sal_strncpy(acl_str, tmp_str, 50);
            }
        }
        /*ended by yejl*/
        
#endif
    }
    else
    {
        return PM_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }

#ifdef TAPPRODUCT

    sal_fprintf (cfg_fp, " sequence-num %"PRIu64" %s", ((p_acl_entry->key.aclid >> 16) & 0xffff),
        (p_acl_entry->deny ? "deny" : "permit"));

    if (0 == p_acl_entry->key_flags && 0 == p_acl_entry->key_ipv6_flags)
    {
        sal_fprintf (cfg_fp, " any");
        acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
        sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
        *total_bytes += attr[0].value.u64;
        *total_packets += attr[1].value.u64;
        return PM_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        if (0xff == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " any");
        }
        else if (GLB_ACL_L4_PROTOCOL_TCP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " tcp");
        }
        else if (GLB_ACL_L4_PROTOCOL_UDP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " udp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_IGMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " igmp");
        }
        else
        {
            sal_fprintf (cfg_fp, " %d", p_acl_entry->ip_protocol);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        sal_fprintf (cfg_fp, " icmp-type %d", p_acl_entry->icmp_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        sal_fprintf (cfg_fp, " icmp-code %d", p_acl_entry->icmp_code);
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port eq %d", p_acl_entry->l4_src_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port lt %d", p_acl_entry->l4_src_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port gt %d", p_acl_entry->l4_src_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port range %d %d", p_acl_entry->l4_src_port, p_acl_entry->l4_src_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port eq %d", p_acl_entry->l4_dst_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port lt %d", p_acl_entry->l4_dst_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port gt %d", p_acl_entry->l4_dst_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port range %d %d", p_acl_entry->l4_dst_port, p_acl_entry->l4_dst_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        if (GLB_ACL_TCP_FLAG_OP_ALL == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-code match-all");
        }
        else if (GLB_ACL_TCP_FLAG_OP_ANY == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-code match-any");
        }

        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
        {
            sal_fprintf (cfg_fp, " ack");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
        {
            sal_fprintf (cfg_fp, " fin");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
        {
            sal_fprintf (cfg_fp, " psh");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
        {
            sal_fprintf (cfg_fp, " rst");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
        {
            sal_fprintf (cfg_fp, " syn");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
        {
            sal_fprintf (cfg_fp, " urg");
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPSA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip any");
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->src_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->src_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " src-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPDA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dst-ip any");
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->dst_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->dst_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " dst-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        if (0xFFFFF == p_acl_entry->ipv6_flow_label_mask && 0 == p_acl_entry->ipv6_flow_label)
        {
            sal_fprintf (cfg_fp, " flow-label any");
        }
        else
        {
            sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_acl_entry->ipv6_flow_label, p_acl_entry->ipv6_flow_label_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSCP))
    {
        sal_fprintf (cfg_fp, " dscp %d", p_acl_entry->dscp);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        sal_fprintf (cfg_fp, " ip-precedence %d", p_acl_entry->ip_precedence);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_OPTIONS))
    {
        sal_fprintf (cfg_fp, " options");
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_UDF))
    {
        if (GLB_ACL_UDF_GROUP_L3_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else
        {
            sal_fprintf (cfg_fp, " udf udf-id %d", p_acl_entry->udf_id);
            uint32 iter = 0;
            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (p_acl_entry->udf_value_valid[iter])
                {
                    if (0xFFFFFFFF == p_acl_entry->udf_mask[iter])
                    {
                        sal_fprintf(cfg_fp, " udf%d any", iter);
                    }
                    else
                    {
                        sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                p_acl_entry->udf_value[iter], p_acl_entry->udf_mask[iter]);
                    }
                }
            }
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TAP_TRUNCATION))
    {
        sal_fprintf (cfg_fp, " truncation");
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_COS))
    {
        sal_fprintf (cfg_fp, " cos %d", p_acl_entry->svlan_cos);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_INNER_COS))
    {
        sal_fprintf (cfg_fp, " inner-cos %d", p_acl_entry->cvlan_cos);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_VLAN))
    {
        sal_fprintf (cfg_fp, " vlan %d", p_acl_entry->svlan);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        sal_fprintf (cfg_fp, " inner-vlan %d", p_acl_entry->cvlan);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACSA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->src_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->src_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACDA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->dst_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->dst_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        if (0xffff == p_acl_entry->ether_type_mask)
        {
            sal_fprintf (cfg_fp, " ether-type any");
        }
        else
        {
            sal_fprintf (cfg_fp, " ether-type 0x%x 0x%x", p_acl_entry->ether_type, p_acl_entry->ether_type_mask);
        }
    }

#else

    sal_fprintf (cfg_fp, " %"PRIu64" %s", ((p_acl_entry->key.aclid >> 16) & 0xffff),
        (p_acl_entry->deny ? "deny" : "permit"));

    if (0 == p_acl_entry->key_flags && 0 == p_acl_entry->key_ipv6_flags && 0 == p_acl_entry->key_arp_flags)
    {
        sal_fprintf (cfg_fp, " any");
        acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
        sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
        *total_bytes += attr[0].value.u64;
        *total_packets += attr[1].value.u64;
        return PM_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACSA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->src_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->src_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACDA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->dst_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->dst_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_VLAN))
    {
        if (0xfff == p_acl_entry->svlan_mask)
        {
            sal_fprintf (cfg_fp, " vlan any");
        }
        else
        {
            /*modified by yejl for bug 49788, 2018-11-13*/
            sal_fprintf (cfg_fp, " vlan %d 0x%x", p_acl_entry->svlan, p_acl_entry->svlan_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        if (0xfff == p_acl_entry->cvlan_mask)
        {
            sal_fprintf (cfg_fp, " inner-vlan any");
        }
        else
        {
            sal_fprintf (cfg_fp, " inner-vlan %d 0x%x", p_acl_entry->cvlan, p_acl_entry->cvlan_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_COS))
    {
        if (0x7 == p_acl_entry->svlan_cos_mask)
        {
            sal_fprintf (cfg_fp, " cos any");
        }
        else
        {
            sal_fprintf (cfg_fp, " cos %d 0x%x", p_acl_entry->svlan_cos, p_acl_entry->svlan_cos_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_INNER_COS))
    {
        if (0x7 == p_acl_entry->cvlan_cos_mask)
        {
            sal_fprintf (cfg_fp, " inner-cos any");
        }
        else
        {
            sal_fprintf (cfg_fp, " inner-cos %d 0x%x", p_acl_entry->cvlan_cos, p_acl_entry->cvlan_cos_mask);
        }
    }

    /*added by yejl for bug 49791, 2018-11-14*/
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_UNTAG))
    {
        sal_fprintf (cfg_fp, " untag-vlan");
    }
    
    /*modified by yejl for bug 49768, 2018-10-31*/
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        sal_fprintf (cfg_fp, " ether-type 0x%x", p_acl_entry->ether_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE))
    {
        if (0xffff == p_acl_entry->arp_op_code_mask)
        {
            sal_fprintf (cfg_fp, " arp any");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_REQUEST == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp request");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RESPONSE == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp response");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RARP_REQUEST == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp rarp-request");
        }
        else if (0 == p_acl_entry->arp_op_code_mask && ARP_TYPE_RARP_RESPONSE == p_acl_entry->arp_op_code)
        {
            sal_fprintf (cfg_fp, " arp rarp-response");
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_SENDER_IP))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " sender-ip any");
        }
        else if (0 == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " sender-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " sender-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_TARGET_IP))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " target-ip any");
        }
        else if (0 == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " target-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " target-ip %s %s", buf1, buf2);
        }
    }  

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPSA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip any");
        }
        else if (0 == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->src_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->src_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " src-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
        }
    }


    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPDA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dest-ip any");
        }
        else if (0 == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dest-ip host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dest-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->dst_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->dst_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8,flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " dst-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        if (0xFFFFF == p_acl_entry->ipv6_flow_label_mask && 0 == p_acl_entry->ipv6_flow_label)
        {
            sal_fprintf (cfg_fp, " flow-label any");
        }
        else
        {
            sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_acl_entry->ipv6_flow_label, p_acl_entry->ipv6_flow_label_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSCP))
    {
        if (0x3f == p_acl_entry->dscp_mask)
        {
            sal_fprintf (cfg_fp, " dscp any");
        }
        else
        {
            sal_fprintf (cfg_fp, " dscp %d 0x%x", p_acl_entry->dscp, p_acl_entry->dscp_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        if (0x7 == p_acl_entry->ip_precedence_mask)
        {
            sal_fprintf (cfg_fp, " ip-precedence any");
        }
        else
        {
            sal_fprintf (cfg_fp, " ip-precedence %d 0x%x", p_acl_entry->ip_precedence, p_acl_entry->ip_precedence_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_FRAG))
    {
        /*modified by yejl for bug 49815, 2018-11-08*/
        sal_fprintf (cfg_fp, " fragment");
        if (GLB_ACL_IP_FRAG_FIRST_FRAG == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NON_FIRST == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NONE == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-fragment");
        }
        else if (GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " non-or-first-fragment");
        }
        else if (GLB_ACL_IP_FRAG_TINY == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " small-fragment");
        }
        else if (GLB_ACL_IP_FRAG_YES == p_acl_entry->ip_frag)
        {
            sal_fprintf (cfg_fp, " any-fragment");
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        if (GLB_ACL_L4_PROTOCOL_TCP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " l3-protocol tcp");
        }
        else if (GLB_ACL_L4_PROTOCOL_UDP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " l3-protocol udp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_IGMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
            && !p_acl_entry->is_ipv6)
        {
            sal_fprintf (cfg_fp, " l3-protocol igmp");
        }
        else
        {
            sal_fprintf (cfg_fp, " l3-protocol %d", p_acl_entry->ip_protocol);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port eq %d", p_acl_entry->l4_src_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port lt %d", p_acl_entry->l4_src_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port gt %d", p_acl_entry->l4_src_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-l4-port range %d %d", p_acl_entry->l4_src_port, p_acl_entry->l4_src_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port eq %d", p_acl_entry->l4_dst_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port lt %d", p_acl_entry->l4_dst_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port gt %d", p_acl_entry->l4_dst_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dest-l4-port range %d %d", p_acl_entry->l4_dst_port, p_acl_entry->l4_dst_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IGMP_TYPE))
    {
        if (ACL_IGMP_TYPE_HOST_DVMRP == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " dvmrp");
        }
        else if (ACL_IGMP_TYPE_HOST_QUERY == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " host-query");
        }
        else if (ACL_IGMP_TYPE_HOST_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v1host-report");
        }
        else if (ACL_IGMP_TYPE_MTRACT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " mtrace-route");
        }
        else if (ACL_IGMP_TYPE_MTRACT_RESPONSE == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " mtrace-response");
        }
        else if (ACL_IGMP_TYPE_V2_LEAVE == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v2leave-group");
        }
        else if (ACL_IGMP_TYPE_V2_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v2host-report");
        }
        else if (ACL_IGMP_TYPE_V3_REPORT == p_acl_entry->igmp_type)
        {
            sal_fprintf (cfg_fp, " v3host-report");
        }   
        else
        {
            sal_fprintf (cfg_fp, " %d", p_acl_entry->igmp_type);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        sal_fprintf (cfg_fp, " icmp-type %d", p_acl_entry->icmp_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        sal_fprintf (cfg_fp, " icmp-code %d", p_acl_entry->icmp_code);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        if (GLB_ACL_TCP_FLAG_OP_ALL == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-flag match-all");
        }
        else if (GLB_ACL_TCP_FLAG_OP_ANY == p_acl_entry->tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-flag match-any");
        }

        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
        {
            sal_fprintf (cfg_fp, " ack");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
        {
            sal_fprintf (cfg_fp, " fin");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
        {
            sal_fprintf (cfg_fp, " psh");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
        {
            sal_fprintf (cfg_fp, " rst");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
        {
            sal_fprintf (cfg_fp, " syn");
        }
        if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
        {
            sal_fprintf (cfg_fp, " urg");
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_UDF))
    {
        if (GLB_ACL_UDF_GROUP_L3_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_acl_entry->udf_type)
        {
            sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
            p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
        }
        else
        {
            sal_fprintf (cfg_fp, " udf udf-id %d", p_acl_entry->udf_id);
            uint32 iter = 0;
            for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
            {
                if (p_acl_entry->udf_value_valid[iter])
                {
                    if (0xFFFFFFFF == p_acl_entry->udf_mask[iter])
                    {
                        sal_fprintf(cfg_fp, " udf%d any", iter);
                    }
                    else
                    {
                        sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                p_acl_entry->udf_value[iter], p_acl_entry->udf_mask[iter]);
                    }
                }
            }
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TIME_RANGE))
    {
        sal_fprintf (cfg_fp, " time-range %s", p_acl_entry->time_range);
    }

#endif

    acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
    sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
    *total_bytes += attr[0].value.u64;
    *total_packets += attr[1].value.u64;
    return PM_E_NONE;
}



int32
_acl_worm_filter_show_counter(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    tbl_acl_worm_filter_t *p_worm_filter = NULL;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr[2];
    FILE *cfg_fp = pargs->argv[0];
    char *worm_name = pargs->argv[1];
    uint32 seq_num = 0;

    if (IS_ACL_WORM_FILTER_ID(p_acl_entry->key.aclid))
    {
        return PM_E_NONE;
    }

    seq_num = (p_acl_entry->key.aclid & 0xFF0000)>>16;
    p_worm_filter = tbl_acl_worm_filter_get_by_seq(seq_num);

    if (!p_worm_filter)
    {
        return PM_E_NONE;
    }
    if (worm_name && 0 != sal_strcmp(p_worm_filter->key.worm_filter_name, worm_name))
    {
        return PM_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;

    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, " %"PRIu64" %s", ((p_acl_entry->key.aclid >> 16) & 0xffff),
        p_worm_filter->key.worm_filter_name);

    acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
    if (p_worm_filter->isrange)
    {
        sal_fprintf (cfg_fp, " %s %d-%d ( bytes %"PRIu64" packets %"PRIu64" )\n", 
            (p_worm_filter->istcp ? "tcp" : "udp"), p_worm_filter->dstport, p_worm_filter->isrange, attr[0].value.u64, attr[1].value.u64);
    }
    else
    {
        sal_fprintf (cfg_fp, " %s %d ( bytes %"PRIu64" packets %"PRIu64" )\n", 
            (p_worm_filter->istcp ? "tcp" : "udp"), p_worm_filter->dstport, attr[0].value.u64, attr[1].value.u64);
    }
    
    return PM_E_NONE;
}



int32
acl_worm_filter_show_counter(FILE* fp, char *worm_name, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = worm_name;

    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)_acl_worm_filter_show_counter, &args);

    return PM_E_NONE;
}

int32
acl_show_acl_counter(FILE* fp, char *ifname, char *dir, uint32 type, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint64 total_bytes = 0;
    uint64 total_packets = 0;
    char acl_config_str[50];

    sal_memset(&args, 0, sizeof(args));
    sal_memset(acl_config_str, 0, sizeof(acl_config_str));
    args.argv[0] = fp;
    args.argv[1] = ifname;
    args.argv[2] = dir;
    args.argv[3] = &total_bytes;
    args.argv[4] = &total_packets;
    args.argv[5] = acl_config_str;
    args.argv[6] = &type;
    if (NULL == dir || 0 == sal_strcmp(dir, "in"))
    {
        tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)_acl_show_acl_counter_in, &args);
    }
    if (NULL == dir || 0 == sal_strcmp(dir, "out"))
    {
        tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)_acl_show_acl_counter_out, &args);
    }
    if (sal_strlen(acl_config_str) != 0)
    {
        sal_fprintf (fp, "(total bytes %"PRIu64" total packets %"PRIu64" )\n", total_bytes, total_packets);
    }
    return PM_E_NONE;
}

int32
hsrv_acl_cmd_show_acl_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    uint32 type = GLB_ACL_CONFIG_TYPE_NORMAL;
    FILE *fp = NULL;

    p_node = cdb_get_tbl(ACT_SHOW_ACL_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:  
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }

        /*added by yejl for IP/IPv6/MAC acl*/
        if (0 == sal_strncmp(argv[0], "ipv6", 4))
        {
            if (FALSE == hsrv_acl_stm_is_ipv6_profile())
            {
                CFG_INVALID_PARAM_RET("No ipv6 resource, please change profile");    
            }
            type = GLB_ACL_CONFIG_TYPE_IPV6;
        }
        else if (0 == sal_strncmp(argv[0], "normal", 6))
        {
            type = GLB_ACL_CONFIG_TYPE_NORMAL;
        }
        else if (0 == sal_strncmp(argv[0], "mac", 3))
        {
            type = GLB_ACL_CONFIG_TYPE_MAC;
        }
        /*ended*/

        /*added by yejl for bug 47903, 2018-08-10*/
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not exist", IFNAME_ETH2FULL(argv[1], ifname_ext));
        }
        /*ended by yejl*/
        
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }

        if (0 == tbl_acl_entry_get_acl_entry_count())
        {
            sal_fclose(fp);
            return PM_E_NONE;
        }

        if (argc == 3)/*show statistic with direction*/
        {
            acl_show_acl_counter(fp, argv[1], argv[2], type, para);
        }
        else if (argc == 2)/*show statistic without direction*/
        {
            acl_show_acl_counter(fp, argv[1], NULL, type, para);
        }
        
        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
hsrv_acl_worm_filter_show_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;
    tbl_acl_worm_filter_master_t* master = tbl_acl_worm_filter_get_master();

    p_node = cdb_get_tbl(ACT_SHOW_ACL_WORM_FILTER_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }

        if (0 == master->acl_worm_filter_hash->count)
        {
            sal_fclose(fp);
            return PM_E_NONE;
        }

        if (1 == argc)
        {
            acl_worm_filter_show_counter(fp, argv[0], para);
        }
        else if (0 == argc)
        {
            acl_worm_filter_show_counter(fp, NULL, para);
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}



int32
_acl_policy_clear_policer_by_aclid(uint64 acl_id)
{
    tbl_fea_acl_policy_action_t* pst_fea_acl_policy_action = NULL;
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;
    
    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(tbl_fea_acl_policy_action_key_t));
    fea_acl_policy_action_key.acl_policy_action_id = MAKE_FEA_ACL_POLICY_ACTION_ID(acl_id);
    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
    if(pst_fea_acl_policy_action)
    {
        pst_fea_acl_policy_action->green_packet = 0;
        pst_fea_acl_policy_action->green_byte = 0;
        pst_fea_acl_policy_action->yellow_packet= 0;
        pst_fea_acl_policy_action->yellow_byte= 0;
        pst_fea_acl_policy_action->red_packet= 0;
        pst_fea_acl_policy_action->red_byte= 0;
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET);
        tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(pst_fea_acl_policy_action, TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE);
    }

    return PM_E_NONE;
}


int32
_acl_clear_acl_counter(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_interface = NULL;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr;
    uint32 type = *(uint32*)pargs->argv[0];
    char *ifname = pargs->argv[2];

    sal_memset(&fea_acl_key, 0, sizeof(fea_acl_key));
    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }

    /*added by yejl for IP/IPv6/MAC acl*/
    if (p_acl_entry->acl_type != type)
    {
        return PM_E_NONE;
    }
    /*ended by yejl*/
    
    if(IS_ACL_POLICY_ID(p_acl_entry->key.aclid))
    {
        return PM_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_ACL_COUNTER_ATTR_BYTES;

    if (NULL == ifname)
    {
        if (0 == sal_strcmp(pargs->argv[1], "in"))
        {
            if (p_acl_entry->in_port)
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
        else if (0 == sal_strcmp(pargs->argv[1], "out"))
        {
            if (p_acl_entry->out_port)
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
        else
        {
            if (p_acl_entry->out_port || p_acl_entry->in_port)
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
    }
    else
    {
        p_interface = tbl_interface_get_interface_by_name(ifname);
        if (NULL == p_interface)
        {
            return PM_E_NONE;
        }

        if (0 == sal_strcmp(pargs->argv[1], "in"))
        {
            if (p_interface->ifindex == p_acl_entry->in_port)
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
        else if (0 == sal_strcmp(pargs->argv[1], "out"))
        {
            if (p_interface->ifindex == p_acl_entry->out_port)
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
        else
        {
            if ((p_interface->ifindex == p_acl_entry->out_port) 
               ||(p_interface->ifindex == p_acl_entry->in_port))
            {
                acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
            }
        }
    }

    return PM_E_NONE;
}


int32
acl_clear_acl_counter(uint32 type, char *dir, char *ifname, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &type;
    args.argv[1] = dir;
    args.argv[2] = ifname;

    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)_acl_clear_acl_counter, &args);
    return PM_E_NONE;
}

int32
hsrv_acl_cmd_clear_acl_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_SET:

        if (0 == sal_strncmp(argv[0], "ipv6", 4))
        {
            if (FALSE == hsrv_acl_stm_is_ipv6_profile())
            {
                CFG_INVALID_PARAM_RET("No ipv6 resource, please change profile");    
            }
            if (argc == 2)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_IPV6, argv[1], NULL, para);
            }
            else if (argc == 3)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_IPV6, argv[1], argv[2], para);
            }
        }
        else if (0 == sal_strncmp(argv[0], "normal", 6))
        {
            if (argc == 2)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_NORMAL, argv[1], NULL, para);
            }
            else if (argc == 3)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_NORMAL, argv[1], argv[2], para);
            }
        }
        else if (0 == sal_strncmp(argv[0], "mac", 3))
        {
            if (argc == 2)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_MAC, argv[1], NULL, para);
            }
            else if (argc == 3)
            {
                acl_clear_acl_counter(GLB_ACL_CONFIG_TYPE_MAC, argv[1], argv[2], para);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Wrong arg input");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support clear action");
        break;
    }

    return rc;    
}

int32 _hsrv_acl_clear_policer_stats_by_acl_id(uint64 acl_id)
{
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;
    tbl_fea_acl_policy_action_t    *p_fea_acl_policy_action = NULL;

    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(fea_acl_policy_action_key));
    fea_acl_policy_action_key.acl_policy_action_id = MAKE_FEA_ACL_POLICY_ACTION_ID(acl_id);
    p_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
    if(NULL != p_fea_acl_policy_action)
    {
        HSRV_IF_ERROR_RETURN(tbl_fea_acl_policy_delete_stats(p_fea_acl_policy_action));
    }
    
    return HSRV_E_NONE;
}

int32 _hsrv_acl_clear_acl_policy_stats(tbl_clear_acl_policy_t *p_clear_acl_policy)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_ACL_COUNTER_ATTR_BYTES;

    HSRV_IF_ERROR_RETURN(acl_api->set_acl_counter_attribute(p_clear_acl_policy->counter_id, &attr));

    return HSRV_E_NONE;
}

int32
hsrv_acl_clear_acl_policy_stats(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_clear_acl_policy_t *p_clear_acl_policy = (tbl_clear_acl_policy_t *)p_tbl;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    switch(oper)
    {
        case CDB_OPER_SET:
            /* clear acl stats */
            if(field_id == TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID)
            {
                HSRV_IF_ERROR_RETURN(_hsrv_acl_clear_acl_policy_stats(p_clear_acl_policy));
            }
            /* clear flow policer stats */
            else if(field_id == TBL_CLEAR_ACL_POLICY_FLD_ACL_ID)
            {
                HSRV_IF_ERROR_RETURN(_hsrv_acl_clear_policer_stats_by_acl_id(p_clear_acl_policy->acl_id));
            }
            break;
        default:
            break;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_acl_l4_port_range_cmd_process(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    // do nothing
    return 0;
}

static int32
_hsrv_acl_add_udf_group(tbl_acl_udf_group_t* acl_udf_group)
{
    sai_udf_api_t *udf_api = NULL;
    sai_object_id_t udf_match_id = 0;
    sai_object_id_t udf_id = 0;
    sai_object_id_t udf_group_id = 0;
    sai_attribute_t attr_list[5];
    //sai_object_list_t  obj_list; 
    //sai_object_id_t    udf_oid[1];
    uint32 attr_count = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));
    sal_memset(attr_list, 0, sizeof(attr_list));

#if 0    
    attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L2_TYPE;
    attr_list[attr_count].value.u16 = 0x0800;
    attr_count++;
    attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L3_TYPE;
    attr_list[attr_count].value.u8 = acl_udf_group->l3_protocol & 0xFF;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf_match(&udf_match_id, attr_count, attr_list));
    acl_udf_group->udf_match_oid = udf_match_id;
    tbl_acl_udf_group_set_acl_udf_group_field(acl_udf_group, TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;

    attr_list[attr_count].id = SAI_UDF_ATTR_MATCH_ID;
    attr_list[attr_count].value.oid = udf_match_id;
    attr_count++;

    if (acl_udf_group->group_type)
    {
        attr_list[attr_count].id = SAI_UDF_ATTR_BASE;
        attr_list[attr_count].value.s32 = acl_udf_group->group_type;
        attr_count++;
    }

    attr_list[attr_count].id = SAI_UDF_ATTR_OFFSET;
    attr_list[attr_count].value.u16 = acl_udf_group->offset;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf(&udf_id, attr_count, attr_list));
    acl_udf_group->udf_oid = udf_id;
    tbl_acl_udf_group_set_acl_udf_group_field(acl_udf_group, TBL_ACL_UDF_GROUP_FLD_UDF_OID);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_UDF_LIST;
    udf_oid[0] = udf_id;
    obj_list.list = udf_oid;
    obj_list.count = 1;
    sal_memcpy(&attr_list[attr_count].value.objlist, &obj_list, sizeof(sai_object_list_t));
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_UDF_GROUP_GENERIC;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_LENGTH;
    attr_list[attr_count].value.u16 = 4;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf_group(&udf_group_id, attr_count, attr_list));
    acl_udf_group->udf_group_oid = udf_group_id;
    tbl_acl_udf_group_set_acl_udf_group_field(acl_udf_group, TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID);
#endif


    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_UDF_GROUP_GENERIC;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_LENGTH;
    attr_list[attr_count].value.u16 = GLB_ACL_UDF_GROUP_LENGTH;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_UDF_ID;
    attr_list[attr_count].value.u32 = acl_udf_group->key.id;
    attr_count++;
    HSRV_IF_ERROR_RETURN(udf_api->create_udf_group(&udf_group_id, attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;
    attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = acl_udf_group->key.id;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L2_TYPE;
    attr_list[attr_count].value.aclfield.enable   = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = 0x0800;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xffff;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L3_TYPE;
    attr_list[attr_count].value.aclfield.enable   = TRUE;
    attr_list[attr_count].value.aclfield.data.u8  = acl_udf_group->l3_protocol & 0xFF;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xff;
    attr_count++;
    HSRV_IF_ERROR_RETURN(udf_api->create_udf_match(&udf_match_id, attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;
    attr_list[attr_count].id = SAI_UDF_ATTR_GROUP_ID;
    attr_list[attr_count].value.u32 = acl_udf_group->key.id;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_BASE;
    attr_list[attr_count].value.u32 = acl_udf_group->group_type;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_OFFSET;
    if (acl_udf_group->offset & 0x80)
    {
        attr_list[attr_count].value.u32 = acl_udf_group->offset;
    }
    else
    {
        attr_list[attr_count].value.u32 = (acl_udf_group->offset << 24) | ((acl_udf_group->offset + 1) << 16)
                                  | ((acl_udf_group->offset + 2) << 8) | (acl_udf_group->offset + 3);
    }
    
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf(&udf_id, attr_count, attr_list));
  
    return HSRV_E_NONE;
}

static int32
_hsrv_acl_del_udf_group(tbl_acl_udf_group_t* acl_udf_group)
{
    tbl_acl_udf_group_t* p_db_acl_udf_group = NULL;
    sai_udf_api_t *udf_api = NULL;

    p_db_acl_udf_group = tbl_acl_udf_group_get_acl_udf_group(&acl_udf_group->key);
    if (NULL == p_db_acl_udf_group)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));
    HSRV_IF_ERROR_RETURN(udf_api->remove_udf_group(p_db_acl_udf_group->key.id));
#if 0    
    HSRV_IF_ERROR_RETURN(udf_api->remove_udf(p_db_acl_udf_group->udf_oid));
    HSRV_IF_ERROR_RETURN(udf_api->remove_udf_match(p_db_acl_udf_group->udf_match_oid));
#endif    
    return HSRV_E_NONE;
}

int32
hsrv_acl_udf_group_cmd_process(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_acl_udf_group_t *p_acl_udf_group         = NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_DEL:
        p_acl_udf_group = (tbl_acl_udf_group_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(_hsrv_acl_del_udf_group(p_acl_udf_group));
        break;
        
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_acl_udf_group_cmd_process_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{

    tbl_acl_udf_group_t *p_acl_udf_group         = NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_udf_group = (tbl_acl_udf_group_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(_hsrv_acl_add_udf_group(p_acl_udf_group));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
__tap_show_flow_inner_ace(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    FILE *cfg_fp = pargs->argv[0];
    char buf1[256];
    char buf2[256];
    addr_ipv4_t tmp_ip;
    addr_ipv4_t tmp_mask;
    uint8 flag = 0;

    if (0 == p_acl_entry->inner_key_flags && 0 == p_acl_entry->inner_key_ipv6_flags)
    {
        return PM_E_NONE;
    }

    if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH))
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, "     sequence-num %d match",
        ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(p_acl_entry->entry_priority));

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        if (0xff == p_acl_entry->inner_ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " any");
        }
        else if (GLB_ACL_L4_PROTOCOL_TCP == p_acl_entry->inner_ip_protocol && 0 == p_acl_entry->inner_ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " tcp");
        }
        else if (GLB_ACL_L4_PROTOCOL_UDP == p_acl_entry->inner_ip_protocol && 0 == p_acl_entry->inner_ip_protocol_mask)
        {
            sal_fprintf (cfg_fp, " udp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMP == p_acl_entry->inner_ip_protocol && 0 == p_acl_entry->inner_ip_protocol_mask
            && !p_acl_entry->inner_is_ipv6)
        {
            sal_fprintf (cfg_fp, " icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_acl_entry->inner_ip_protocol && 0 == p_acl_entry->inner_ip_protocol_mask
            && p_acl_entry->inner_is_ipv6)
        {
            sal_fprintf (cfg_fp, " icmp");
        }
        else if (GLB_ACL_L4_PROTOCOL_IGMP == p_acl_entry->inner_ip_protocol && 0 == p_acl_entry->inner_ip_protocol_mask
            && !p_acl_entry->inner_is_ipv6)
        {
            sal_fprintf (cfg_fp, " igmp");
        }
        else
        {
            sal_fprintf (cfg_fp, " %d", p_acl_entry->inner_ip_protocol);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_ICMP_TYPE))
    {
        sal_fprintf (cfg_fp, " icmp-type %d", p_acl_entry->icmp_type);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_ICMP_CODE))
    {
        sal_fprintf (cfg_fp, " icmp-code %d", p_acl_entry->icmp_code);
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->inner_l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->inner_l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port eq %d", p_acl_entry->inner_l4_src_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->inner_l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port lt %d", p_acl_entry->inner_l4_src_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->inner_l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port gt %d", p_acl_entry->inner_l4_src_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->inner_l4_src_port_type)
        {
            sal_fprintf (cfg_fp, " src-port range %d %d", 
                p_acl_entry->inner_l4_src_port, p_acl_entry->inner_l4_src_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->inner_l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port any");
        }
        if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->inner_l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port eq %d", p_acl_entry->inner_l4_dst_port);
        }
        if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->inner_l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port lt %d", p_acl_entry->inner_l4_dst_port_mask + 1);
        }
        if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->inner_l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port gt %d", p_acl_entry->inner_l4_dst_port - 1);
        }
        if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->inner_l4_dst_port_type)
        {
            sal_fprintf (cfg_fp, " dst-port range %d %d", p_acl_entry->inner_l4_dst_port, 
                p_acl_entry->inner_l4_dst_port_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_TCP_FLAG))
    {
        if (GLB_ACL_TCP_FLAG_OP_ALL == p_acl_entry->inner_tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-code match-all");
        }
        else if (GLB_ACL_TCP_FLAG_OP_ANY == p_acl_entry->inner_tcp_flags)
        {
            sal_fprintf (cfg_fp, " tcp-code match-any");
        }

        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
        {
            sal_fprintf (cfg_fp, " ack");
        }
        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
        {
            sal_fprintf (cfg_fp, " fin");
        }
        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
        {
            sal_fprintf (cfg_fp, " psh");
        }
        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
        {
            sal_fprintf (cfg_fp, " rst");
        }
        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
        {
            sal_fprintf (cfg_fp, " syn");
        }
        if GLB_FLAG_ISSET(p_acl_entry->inner_tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
        {
            sal_fprintf (cfg_fp, " urg");
        }
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_IPSA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->inner_src_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->inner_src_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->inner_src_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " src-ip any");
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->inner_src_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->inner_src_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->inner_src_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " src-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->inner_src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_IPDA))
    {
        tmp_ip.s_addr = sal_hton32(p_acl_entry->inner_dst_ip.u.prefix4.s_addr);
        tmp_mask.s_addr = sal_hton32(p_acl_entry->inner_dst_ip_mask.u.prefix4.s_addr);
        cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

        if (0xffffffff == p_acl_entry->inner_dst_ip_mask.u.prefix4.s_addr)
        {
            sal_fprintf (cfg_fp, " dst-ip any");
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ip %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->inner_dst_ip.u.prefix6);
        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->inner_dst_ip_mask.u.prefix6);
        flag = 0;
        GLB_IS_FULL_IPV6_MASK(p_acl_entry->inner_dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
        if (IPV6_MAX_BYTELEN == flag)
        {
            sal_fprintf (cfg_fp, " dst-ipv6 any");
        }
        else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->inner_dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
        {
            sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
        }
        else
        {
            sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
    {
        if (0xFFFFF == p_acl_entry->inner_ipv6_flow_label_mask && 0 == p_acl_entry->inner_ipv6_flow_label)
        {
            sal_fprintf (cfg_fp, " flow-label any");
        }
        else
        {
            sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_acl_entry->inner_ipv6_flow_label, 
                        p_acl_entry->inner_ipv6_flow_label_mask);
        }
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_DSCP))
    {
        sal_fprintf (cfg_fp, " dscp %d", p_acl_entry->inner_dscp);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_PRECEDENCE))
    {
        sal_fprintf (cfg_fp, " ip-precedence %d", p_acl_entry->inner_ip_precedence);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_OPTIONS))
    {
        sal_fprintf (cfg_fp, " options");
    }
    
    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_TAP_TRUNCATION))
    {
        sal_fprintf (cfg_fp, " truncation");
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_COS))
    {
        sal_fprintf (cfg_fp, " cos %d", p_acl_entry->inner_svlan_cos);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_IP_ACE_INNER_COS))
    {
        sal_fprintf (cfg_fp, " inner-cos %d", p_acl_entry->inner_cvlan_cos);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_MAC_ACE_VLAN))
    {
        sal_fprintf (cfg_fp, " vlan %d", p_acl_entry->inner_svlan);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_MAC_ACE_INNNER_VLAN))
    {
        sal_fprintf (cfg_fp, " inner-vlan %d", p_acl_entry->inner_cvlan);
    }

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_MAC_ACE_MACSA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->inner_src_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->inner_src_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_MAC_ACE_MACDA))
    {
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->inner_dst_mac);
        cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->inner_dst_mac_mask);

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

    if (GLB_FLAG_ISSET(p_acl_entry->inner_key_flags, GLB_MAC_ACE_ETHTYPE))
    {
        if (0xffff == p_acl_entry->inner_ether_type_mask)
        {
            sal_fprintf (cfg_fp, " ether-type any");
        }
        else
        {
            sal_fprintf (cfg_fp, " ether-type 0x%x 0x%x", 
                p_acl_entry->inner_ether_type, p_acl_entry->inner_ether_type_mask);
        }
    }
    
    return PM_E_NONE;
}

int32
__tap_show_flow_ace_counter_in(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs, uint32 display_outer)
{
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr[2];
    FILE *cfg_fp = pargs->argv[0];
    uint64 *total_bytes = pargs->argv[3];
    uint64 *total_packets = pargs->argv[4];
    char buf1[256];
    char buf2[256];
    addr_ipv4_t tmp_ip;
    addr_ipv4_t tmp_mask;
    uint8 flag = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;

    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }

    if(display_outer)
    {
        sal_fprintf (cfg_fp, " sequence-num %"PRIu64" %s", ((p_acl_entry->key.aclid >> 16) & 0xffff),
            (p_acl_entry->deny ? "deny" : "permit"));
   

        if (0 == p_acl_entry->key_flags && 0 == p_acl_entry->key_ipv6_flags)
        {
            sal_fprintf (cfg_fp, " any");
            acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
            sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
            *total_bytes += attr[0].value.u64;
            *total_packets += attr[1].value.u64;
            return PM_E_NONE;
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_PROTOCOL))
        {
            if (0xff == p_acl_entry->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " any");
            }
            else if (GLB_ACL_L4_PROTOCOL_TCP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " tcp");
            }
            else if (GLB_ACL_L4_PROTOCOL_UDP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
            {
                sal_fprintf (cfg_fp, " udp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
                && !p_acl_entry->is_ipv6)
            {
                sal_fprintf (cfg_fp, " icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_ICMPv6 == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
                && p_acl_entry->is_ipv6)
            {
                sal_fprintf (cfg_fp, " icmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_IGMP == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask
                && !p_acl_entry->is_ipv6)
            {
                sal_fprintf (cfg_fp, " igmp");
            }
            else if (GLB_ACL_L4_PROTOCOL_GRE == p_acl_entry->ip_protocol && 0 == p_acl_entry->ip_protocol_mask)
            {
                if (GLB_ACL_GRE_NVGRE == p_acl_entry->l4_gre_type)
                {
                    sal_fprintf (cfg_fp, " nvgre");
                }else{
                    sal_fprintf (cfg_fp, " gre");
                }            
            }
            else
            {
                sal_fprintf (cfg_fp, " %d", p_acl_entry->ip_protocol);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_TYPE))
        {
            sal_fprintf (cfg_fp, " icmp-type %d", p_acl_entry->icmp_type);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_ICMP_CODE))
        {
            sal_fprintf (cfg_fp, " icmp-code %d", p_acl_entry->icmp_code);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_GRE_KEY))
        {
            if (GLB_ACL_GRE_NVGRE == p_acl_entry->l4_gre_type)
            {
                if ((0xffffffff << GLB_ACL_GRE_NVGRE_KEY_OFFSET) == p_acl_entry->l4_gre_key_mask)
                {
                    sal_fprintf (cfg_fp, " nvgre-vsid any");
                }
                else
                {
                    sal_fprintf (cfg_fp, " nvgre-vsid %u 0x%x", 
#ifdef USW
                        p_acl_entry->l4_gre_key, p_acl_entry->l4_gre_key_mask);
#else
                        p_acl_entry->l4_gre_key >> GLB_ACL_GRE_NVGRE_KEY_OFFSET, 
                        p_acl_entry->l4_gre_key_mask >> GLB_ACL_GRE_NVGRE_KEY_OFFSET);
#endif /* USW */
                }
            }else{
                if (0xffffffff == p_acl_entry->l4_gre_key_mask)
                {
                    sal_fprintf (cfg_fp, " gre-key any");
                }
                else
                {
                    sal_fprintf (cfg_fp, " gre-key %u 0x%x", 
                        p_acl_entry->l4_gre_key, 
                        p_acl_entry->l4_gre_key_mask);
                }
            }  
        }

	if (GLB_FLAG_ISSET(p_acl_entry->key_flags2, GLB_IP_ACE_ERSPAN_ID))
       {
	    	if (0x3FF == p_acl_entry->erspan_id_mask)
		    {
		        sal_fprintf (cfg_fp, " erspan any");
		    }
			else
			{
			    sal_fprintf (cfg_fp, " erspan %u 0x%x", 
			        p_acl_entry->erspan_id, p_acl_entry->erspan_id_mask);
			}
        }
        
        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_SRCPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port eq %d", p_acl_entry->l4_src_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port lt %d", p_acl_entry->l4_src_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port gt %d", p_acl_entry->l4_src_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_src_port_type)
            {
                sal_fprintf (cfg_fp, " src-port range %d %d", p_acl_entry->l4_src_port, p_acl_entry->l4_src_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSTPORT))
        {
            if (GLB_ACE_PORT_TYEP_ANY == p_acl_entry->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port any");
            }
            if (GLB_ACE_PORT_TYEP_EQ == p_acl_entry->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port eq %d", p_acl_entry->l4_dst_port);
            }
            if (GLB_ACE_PORT_TYEP_LT == p_acl_entry->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port lt %d", p_acl_entry->l4_dst_port_mask + 1);
            }
            if (GLB_ACE_PORT_TYEP_GT == p_acl_entry->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port gt %d", p_acl_entry->l4_dst_port - 1);
            }
            if (GLB_ACE_PORT_TYEP_RANGE == p_acl_entry->l4_dst_port_type)
            {
                sal_fprintf (cfg_fp, " dst-port range %d %d", p_acl_entry->l4_dst_port, p_acl_entry->l4_dst_port_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_VXLAN_VNI))
        {
            if (GLB_ACL_VXLAN_VNI_MASK_VALID == p_acl_entry->l4_vxlan_vni_mask)
            {
                sal_fprintf (cfg_fp, " vxlan-vni any");
            }
            else
            {
                sal_fprintf (cfg_fp, " vxlan-vni %u 0x%x", p_acl_entry->l4_vxlan_vni, p_acl_entry->l4_vxlan_vni_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TCP_FLAG))
        {
            if (GLB_ACL_TCP_FLAG_OP_ALL == p_acl_entry->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-code match-all");
            }
            else if (GLB_ACL_TCP_FLAG_OP_ANY == p_acl_entry->tcp_flags)
            {
                sal_fprintf (cfg_fp, " tcp-code match-any");
            }

            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_ACK)
            {
                sal_fprintf (cfg_fp, " ack");
            }
            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_FIN)
            {
                sal_fprintf (cfg_fp, " fin");
            }
            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_PSH)
            {
                sal_fprintf (cfg_fp, " psh");
            }
            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_RST)
            {
                sal_fprintf (cfg_fp, " rst");
            }
            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_SYN)
            {
                sal_fprintf (cfg_fp, " syn");
            }
            if GLB_FLAG_ISSET(p_acl_entry->tcp_flags_mask, GLB_ACL_TCP_FLAG_URG)
            {
                sal_fprintf (cfg_fp, " urg");
            }
        }
        
        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPSA))
        {
            tmp_ip.s_addr = sal_hton32(p_acl_entry->src_ip.u.prefix4.s_addr);
            tmp_mask.s_addr = sal_hton32(p_acl_entry->src_ip_mask.u.prefix4.s_addr);
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

            if (0xffffffff == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip any");
            }
            else if (0 == p_acl_entry->src_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " src-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ip %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->src_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->src_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " src-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->src_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " src-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " src-ipv6 %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IPDA))
        {
            tmp_ip.s_addr = sal_hton32(p_acl_entry->dst_ip.u.prefix4.s_addr);
            tmp_mask.s_addr = sal_hton32(p_acl_entry->dst_ip_mask.u.prefix4.s_addr);
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &tmp_ip);
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &tmp_mask);

            if (0xffffffff == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dst-ip any");
            }
            else if (0 == p_acl_entry->dst_ip_mask.u.prefix4.s_addr)
            {
                sal_fprintf (cfg_fp, " dst-ip host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ip %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
        {
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &p_acl_entry->dst_ip.u.prefix6);
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_acl_entry->dst_ip_mask.u.prefix6);
            flag = 0;
            GLB_IS_FULL_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8, flag);
            if (IPV6_MAX_BYTELEN == flag)
            {
                sal_fprintf (cfg_fp, " dst-ipv6 any");
            }
            else if (GLB_IS_ZERO_IPV6_MASK(p_acl_entry->dst_ip_mask.u.prefix6.__in6_u.__u6_addr8))
            {
                sal_fprintf (cfg_fp, " dst-ipv6 host %s", buf1);
            }
            else
            {
                sal_fprintf (cfg_fp, " dst-ipv6 %s %s", buf1, buf2);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_ipv6_flags, GLB_IPV6_ACE_FLOW_LABEL))
        {
            if (0xFFFFF == p_acl_entry->ipv6_flow_label_mask && 0 == p_acl_entry->ipv6_flow_label)
            {
                sal_fprintf (cfg_fp, " flow-label any");
            }
            else
            {
                sal_fprintf (cfg_fp, " flow-label %d 0x%x", p_acl_entry->ipv6_flow_label, p_acl_entry->ipv6_flow_label_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_IP_FRAG))
        {
            if (GLB_ACL_IP_FRAG_FIRST_FRAG == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_FIRST == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NONE == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-fragment");
            }
            else if (GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " non-or-first-fragment");
            }
            else if (GLB_ACL_IP_FRAG_TINY == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " small-fragment");
            }
            else if (GLB_ACL_IP_FRAG_YES == p_acl_entry->ip_frag)
            {
                sal_fprintf (cfg_fp, " any-fragment");
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_DSCP))
        {
            sal_fprintf (cfg_fp, " dscp %d", p_acl_entry->dscp);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_PRECEDENCE))
        {
            sal_fprintf (cfg_fp, " ip-precedence %d", p_acl_entry->ip_precedence);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_OPTIONS))
        {
            sal_fprintf (cfg_fp, " options");
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_UDF) &&
	     !GLB_FLAG_ISSET(p_acl_entry->key_flags2, GLB_IP_ACE_ERSPAN_ID))
        {
            if (GLB_ACL_UDF_GROUP_L3_HEAD == p_acl_entry->udf_type)
            {
                sal_fprintf (cfg_fp, " ipv4-head 0x%08x 0x%08x %d", 
                p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
            }
            else if (GLB_ACL_UDF_GROUP_L4_HEAD == p_acl_entry->udf_type)
            {
                sal_fprintf (cfg_fp, " l4-head 0x%08x 0x%08x %d",
                p_acl_entry->udf_value[0], p_acl_entry->udf_mask[0], p_acl_entry->udf_offset);
            }
            else
            {
                sal_fprintf (cfg_fp, " udf udf-id %d", p_acl_entry->udf_id);
                uint32 iter = 0;
                for (iter = 0; iter < GLB_ACL_UDF_FIELD_NUM_MAX; ++iter)
                {
                    if (p_acl_entry->udf_value_valid[iter])
                    {
                        if (0xFFFFFFFF == p_acl_entry->udf_mask[iter])
                        {
                            sal_fprintf(cfg_fp, " udf%d any", iter);
                        }
                        else
                        {
                            sal_fprintf(cfg_fp, " udf%d 0x%08x 0x%08x", iter,
                                    p_acl_entry->udf_value[iter], p_acl_entry->udf_mask[iter]);
                        }
                    }
                }
            }
        }
        
        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_TAP_TRUNCATION))
        {
            sal_fprintf (cfg_fp, " truncation");
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_COS))
        {
            sal_fprintf (cfg_fp, " cos %d", p_acl_entry->svlan_cos);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_IP_ACE_INNER_COS))
        {
            sal_fprintf (cfg_fp, " inner-cos %d", p_acl_entry->cvlan_cos);
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_VLAN))
        {
            if (0xfff == p_acl_entry->svlan_mask && 0 == p_acl_entry->svlan)
            {
                sal_fprintf (cfg_fp, " vlan any");
            }
            else
            {
                sal_fprintf (cfg_fp, " vlan %d 0x%x", p_acl_entry->svlan, p_acl_entry->svlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_INNNER_VLAN))
        {
            if (0xfff == p_acl_entry->cvlan_mask && 0 == p_acl_entry->cvlan)
            {
                sal_fprintf (cfg_fp, " inner-vlan any");
            }
            else
            {
                sal_fprintf (cfg_fp, " inner-vlan %d 0x%x", p_acl_entry->cvlan, p_acl_entry->cvlan_mask);
            }
        }

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACSA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->src_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->src_mac_mask);

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

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_MACDA))
        {
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_acl_entry->dst_mac);
            cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_acl_entry->dst_mac_mask);

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

        if (GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE))
        {
            if (0xffff == p_acl_entry->ether_type_mask)
            {
                sal_fprintf (cfg_fp, " ether-type any");
            }
            else
            {
                sal_fprintf (cfg_fp, " ether-type 0x%x 0x%x", p_acl_entry->ether_type, p_acl_entry->ether_type_mask);
            }
        }
//???? what's mean?
        if(p_acl_entry->inner_key_flags || p_acl_entry->inner_key_ipv6_flags)
        {
            if (0 != ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(p_acl_entry->entry_priority))
            {
                sal_fprintf (cfg_fp, " \n");
            }
        }
    }
//???? what's mean?
    if(p_acl_entry->inner_key_flags || p_acl_entry->inner_key_ipv6_flags)
    {
        if (0 != ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(p_acl_entry->entry_priority))
        {
            __tap_show_flow_inner_ace(p_acl_entry, pargs);
        }
    }
    
    acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
    sal_fprintf (cfg_fp, " ( bytes %"PRIu64" packets %"PRIu64" )\n", attr[0].value.u64, attr[1].value.u64);
    *total_bytes += attr[0].value.u64;
    *total_packets += attr[1].value.u64;
    return PM_E_NONE;
}

static int32
__flow_acl_entry_cmp(void *data1, void *data2)
{
    tbl_acl_entry_t *p_acl_entry1 = (tbl_acl_entry_t*)data1;
    tbl_acl_entry_t *p_acl_entry2 = (tbl_acl_entry_t*)data2;

    uint32          seq_1 = 0;
    uint32          seq_2 = 0;

    uint32          seq_inner_1 = 0;
    uint32          seq_inner_2 = 0;
    
    int32 rc = 0;

    seq_1 = ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(p_acl_entry1->entry_priority);
    seq_2 = ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(p_acl_entry2->entry_priority);

    seq_inner_1 = ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(p_acl_entry1->entry_priority);
    seq_inner_2 = ACL_TAP_GET_INNER_SEQ_ID_FROM_PRIORITY(p_acl_entry2->entry_priority);
    
    if (seq_1 == seq_2)
    {
        rc = seq_inner_1 - seq_inner_2;
    }
    else 
    {
        rc = seq_1 - seq_2;
    }
    
    return rc;
}

ctclib_slist_t *
_tap_findall_ace_by_key(char *ifname, uint32 *seq_no, uint32 tap_group_id)
{
    tbl_acl_entry_master_t  *p_master       = NULL;
    ctclib_slistnode_t      *next           = NULL;
    tbl_acl_entry_t         *p_db_acl_entry = NULL;
    uint32                  out_seq_no      = 0;
    uint32                  out_tap_group_id = 0;
    tbl_interface_t         *p_interface = NULL;

    ctclib_slist_t          *acl_entry_list;

    p_interface = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_interface)
    {
        return NULL;
    }

    p_master = tbl_acl_entry_get_master();

    acl_entry_list = ctclib_slist_create(__flow_acl_entry_cmp, NULL);
    if (!acl_entry_list)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP(p_master->acl_entry_list, p_db_acl_entry, next)
    {
        out_tap_group_id = ACL_TAP_GET_TAP_GROUP_ID_FROM_PRIORITY(p_db_acl_entry->entry_priority);
        out_seq_no       = ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(p_db_acl_entry->entry_priority);

        if(!IS_ACL_TAP_FLOW_ID(p_db_acl_entry->key.aclid))
        {
            continue;
        }

        if(out_tap_group_id != tap_group_id)
        {
            continue;
        }

        if(*seq_no && (*seq_no != out_seq_no))
        {
            continue;
        }

        if (p_interface->ifindex != p_db_acl_entry->in_port)
        {
            continue;
        }
    
        ctclib_slistnode_insert_sort(acl_entry_list, p_db_acl_entry);
    }

    return acl_entry_list;
}

int32
_tap_show_group_ingress_flow(tbl_tap_group_ingress_flow_t *p_tap_igs_flow, tbl_iter_args_t *pargs)
{
    uint64 total_bytes = 0;
    uint64 total_packets = 0;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key = {0};
    sai_attribute_t attr[2];
    FILE *cfg_fp = pargs->argv[0];
    char *ifname = pargs->argv[1];
    uint32 *seq_no = (uint32*)pargs->argv[2];
    tbl_iter_args_t args;
    ctclib_slist_t *pflow_ace_all   =  NULL;
    ctclib_slistnode_t      *next           = NULL;
    tbl_acl_entry_t         *p_db_acl_entry = NULL;
    uint32                  outer_seq_id    = 0;

    sal_memset(&args, 0, sizeof(args));

    if (NULL == ifname)
    {
        return PM_E_NONE;
    }
    if (0 != sal_strcmp(p_tap_igs_flow->key.if_name, ifname))
    {
        return PM_E_NONE;
    }

    if (0 == sal_strcmp(p_tap_igs_flow->key.if_name, ifname))
    {
        sal_fprintf (cfg_fp, "TAP group name: %s\n", p_tap_igs_flow->key.tap_group);
        sal_fprintf (cfg_fp, " flow name: %s\n", 
                    acl_get_acl_config_key_name(p_tap_igs_flow->key.flow_name, GLB_ACL_CONFIG_TYPE_FLOW));
       
        args.argv[0] = cfg_fp;
        args.argv[1] = ifname;
        args.argv[2] = (void*)seq_no;
        args.argv[3] = &total_bytes;
        args.argv[4] = &total_packets;
        args.argv[5] = p_tap_igs_flow;

        pflow_ace_all = _tap_findall_ace_by_key(ifname, seq_no, p_tap_igs_flow->tap_grp_id);
        if(NULL == pflow_ace_all)
        {
            return PM_E_NONE;
        }
        
        CTCLIB_SLIST_LOOP(pflow_ace_all, p_db_acl_entry, next)
        {
            if(outer_seq_id != ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(p_db_acl_entry->entry_priority))
            {
                outer_seq_id = ACL_TAP_GET_SEQ_ID_FROM_PRIORITY(p_db_acl_entry->entry_priority);
                __tap_show_flow_ace_counter_in(p_db_acl_entry, &args, TRUE);
            }else{
                __tap_show_flow_ace_counter_in(p_db_acl_entry, &args, FALSE);
            }
        }

        CTCLIB_SLIST_LOOP(pflow_ace_all, p_db_acl_entry, next)
        {
            if (IS_ACL_TAP_DECAP_NO_INMATCH_V6(p_db_acl_entry->key.aclid) && 
                !GLB_FLAG_ISSET(p_db_acl_entry->key_flags, GLB_IP_ACE_TAP_FLOW_MATCH)
                && GLB_FLAG_ISSET(p_db_acl_entry->inner_key_ipv6_flags, GLB_IPV6_ACE_IPSA))
            {
                fea_acl_key.acl_id = p_db_acl_entry->key.aclid;
                pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
                if (pst_fea_acl)
                {
                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
                    sal_memset(attr, 0, sizeof(attr));
                    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
                    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;
                    acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
                    total_bytes -= attr[0].value.u64;
                    total_packets -= attr[1].value.u64;
                }
            }
        }
        
        ctclib_slist_delete(pflow_ace_all);
    }
    
    sal_fprintf (cfg_fp, "(total bytes %"PRIu64" total packets %"PRIu64" )\n", total_bytes, total_packets);

    return PM_E_NONE;
}


int32
tap_show_ingress_flow_counter(FILE* fp, char *ifname, uint32 *seq_no, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = ifname;
    args.argv[2] = (void*)seq_no;

    tbl_tap_group_ingress_flow_iterate((TBL_ITER_CB_FUNC)_tap_show_group_ingress_flow, &args);
    //sal_fprintf (fp, "(total bytes %"PRIu64" total packets %"PRIu64" )\n", total_bytes, total_packets);
    return PM_E_NONE;
}

static tbl_tap_group_ingress_flow_t*
_hsrv_show_tap_ingress_flow_counter_get_igress_flow(uint32 tap_grp_id, uint32 ifindex)
{
    tbl_tap_group_ingress_flow_master_t *p_master = tbl_tap_group_ingress_flow_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;

    CTCLIB_SLIST_LOOP(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode)
    {
        if (p_db_tap_group_ingress_flow->if_idx == ifindex && p_db_tap_group_ingress_flow->tap_grp_id == tap_grp_id)
        {
            return p_db_tap_group_ingress_flow;
        }
    }

    return NULL;
}

static int32
_hsrv_show_tap_ingress_flow_counter(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_acl_entry_master_t *p_master = tbl_acl_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_acl_entry_t *p_db_acl = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_tap_group_ingress_flow_t *p_igs_flow = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    uint32 tap_grp_id = 0;
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr[2];
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32 read_number = 0;
    uint32 current_index = 0;
    uint32 start_start_index = 0;
    static uint32 start_index = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;
    
    start_start_index = start_index;
    CTCLIB_SLIST_LOOP(p_master->acl_entry_list, p_db_acl, listnode)
    {
        if (!IS_ACL_TAP_FLOW_ID(p_db_acl->key.aclid))
        {
            continue;
        }
        tap_grp_id = (p_db_acl->key.aclid>>38);
        p_igs_flow = _hsrv_show_tap_ingress_flow_counter_get_igress_flow(tap_grp_id, p_db_acl->in_port);
        if (NULL == p_igs_flow)
        {
            continue;
        }
        if (sal_strlen(p_igs_flow->key.flow_name) <= TAP_ACL_CONFIG_PREFIX_LEN)
        {
            continue;
        }
        fea_acl_key.acl_id = p_db_acl->key.aclid;
        pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
        if (NULL == pst_fea_acl)
        {
            continue;
        }
        ret = acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
        if (SAI_STATUS_SUCCESS != ret)
        {
            continue;
        }
        current_index++;
        if (current_index <= start_index)
        {
            continue;
        }
        sal_fprintf (fp, "/%"PRIu64"/%"PRIu64"/%s/%u/%"PRIu64"/%"PRIu64"\n", 
            p_db_acl->key.aclid,
            p_db_acl->in_port, 
            p_igs_flow->key.flow_name+TAP_ACL_CONFIG_PREFIX_LEN,
            (p_db_acl->entry_priority & 0xFFFF),
            attr[0].value.u64,
            attr[1].value.u64);
        start_index++;
        read_number++;
        if (read_number >= p_sys_glb->mib_flow_stats_read_num)
        {
            break;
        }
    }

    /* if not full, set start_index = 0 and do secondary loop */
    if (read_number < p_sys_glb->mib_flow_stats_read_num)
    {
        start_index = 0;
        /* not from the 0 position, need to do secondary loop */
        if (0 != start_start_index)
        {
            CTCLIB_SLIST_LOOP(p_master->acl_entry_list, p_db_acl, listnode)
            {
                tap_grp_id = (p_db_acl->key.aclid>>38);
                p_igs_flow = _hsrv_show_tap_ingress_flow_counter_get_igress_flow(tap_grp_id, p_db_acl->in_port);
                if (NULL == p_igs_flow)
                {
                    continue;
                }
                if (sal_strlen(p_igs_flow->key.flow_name) <= TAP_ACL_CONFIG_PREFIX_LEN)
                {
                    continue;
                }
                fea_acl_key.acl_id = p_db_acl->key.aclid;
                pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
                if (NULL == pst_fea_acl)
                {
                    continue;
                }
                ret = acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
                if (SAI_STATUS_SUCCESS != ret)
                {
                    continue;
                }
                
                current_index++;
                if (!IS_ACL_TAP_FLOW_ID(p_db_acl->key.aclid))
                {
                    continue;
                }
                sal_fprintf (fp, "/%"PRIu64"/%"PRIu64"/%s/%u/%"PRIu64"/%"PRIu64"\n", 
                    p_db_acl->key.aclid,
                    p_db_acl->in_port, 
                    p_igs_flow->key.flow_name+TAP_ACL_CONFIG_PREFIX_LEN,
                    (p_db_acl->entry_priority & 0xFFFF),
                    attr[0].value.u64,
                    attr[1].value.u64);
                start_index++;
                read_number++;
                if (read_number >= p_sys_glb->mib_flow_stats_read_num)
                {
                    break;
                }
            }
            /* after secondary loop, set start_index to 0 */
            if (read_number < p_sys_glb->mib_flow_stats_read_num)
            {
                start_index = 0;
            }
        }
    }
    
    return PM_E_NONE;
}

static int32
_hsrv_show_tap_egress_flow_counter(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_acl_entry_master_t *p_master = tbl_acl_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_acl_entry_t *p_db_acl = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr[2];
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32 read_number = 0;
    uint32 current_index = 0;
    uint32 start_start_index = 0;
    static uint32 start_index = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr[1].id = SAI_ACL_COUNTER_ATTR_PACKETS;
    
    start_start_index = start_index;
    CTCLIB_SLIST_LOOP(p_master->acl_entry_list, p_db_acl, listnode)
    {
        current_index++;
        if (current_index <= start_index)
        {
            continue;
        }
        
        if (IS_ACL_TAP_FLOW_ID(p_db_acl->key.aclid))
        {
            continue;
        }
        fea_acl_key.acl_id = p_db_acl->key.aclid;
        pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
        if (NULL == pst_fea_acl)
        {
            continue;
        }
        ret = acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
        if (SAI_STATUS_SUCCESS != ret)
        {
            continue;
        }
        
        sal_fprintf (fp, "/%"PRIu64"/%"PRIu64"/%u/%"PRIu64"/%"PRIu64"\n",
            p_db_acl->key.aclid,
            p_db_acl->out_port, 
            (p_db_acl->entry_priority & 0xFFFF),
            attr[0].value.u64,
            attr[1].value.u64);
        start_index++;
        read_number++;
        if (read_number >= p_sys_glb->mib_flow_stats_read_num)
        {
            break;
        }
    }

    /* if not full, set start_index = 0 and do secondary loop */
    if (read_number < p_sys_glb->mib_flow_stats_read_num)
    {
        start_index = 0;
        /* not from the 0 position, need to do secondary loop */
        if (0 != start_start_index)
        {
            CTCLIB_SLIST_LOOP(p_master->acl_entry_list, p_db_acl, listnode)
            {
                current_index++;
                if (IS_ACL_TAP_FLOW_ID(p_db_acl->key.aclid))
                {
                    continue;
                }
                fea_acl_key.acl_id = p_db_acl->key.aclid;
                pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
                if (NULL == pst_fea_acl)
                {
                    continue;
                }
                ret = acl_api->get_acl_counter_attribute(pst_fea_acl->counter_id, 2, attr);
                if (SAI_STATUS_SUCCESS != ret)
                {
                    continue;
                }
                
                sal_fprintf (fp, "/%"PRIu64"/%"PRIu64"/%u/%"PRIu64"/%"PRIu64"\n",
                    p_db_acl->key.aclid,
                    p_db_acl->out_port, 
                    (p_db_acl->entry_priority & 0xFFFF),
                    attr[0].value.u64,
                    attr[1].value.u64);
                start_index++;
                read_number++;
                if (read_number >= p_sys_glb->mib_flow_stats_read_num)
                {
                    break;
                }
            }
            /* after secondary loop, set start_index to 0 */
            if (read_number < p_sys_glb->mib_flow_stats_read_num)
            {
                start_index = 0;
            }
        }
    }
    
    return PM_E_NONE;
}
int32
hsrv_tap_cmd_show_tap_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;

    p_node = cdb_get_tbl(ACT_SHOW_TAP_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }
        if ((argc >= 1) && (0 == sal_strcmp(argv[0], "egress")))
        {
            _hsrv_show_tap_egress_flow_counter(fp, para);
        }
        else
        {
            _hsrv_show_tap_ingress_flow_counter(fp, para);
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
hsrv_tap_cmd_show_tap_flow_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    uint32 seq_no = 0;
    FILE *fp = NULL;

    p_node = cdb_get_tbl(ACT_SHOW_TAP_FLOW_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
        if (3 == argc)
        {
            PM_CFG_GET_UINT(seq_no, argv[2], para->p_rs);
        }
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }

        if (3 == argc)
        {
            tap_show_ingress_flow_counter(fp, argv[0], &seq_no, para);
        }
        else if (2 == argc)
        {
            tap_show_ingress_flow_counter(fp, argv[0], &seq_no, para);
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
_tap_clear_ingress_flow_counter(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_interface = NULL;
    sai_acl_api_t *acl_api = NULL;
    tbl_fea_acl_t *pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;
    sai_attribute_t attr;
    char *ifname = pargs->argv[0];
    //char *flow_name = pargs->argv[1];
    //uint32 *seq_no = pargs->argv[2];

    if (NULL == ifname)
    {
        return PM_E_NONE;
    }

    if(!IS_ACL_TAP_FLOW_ID(p_acl_entry->key.aclid))
    {
        return PM_E_NONE;
    }
    
    fea_acl_key.acl_id = p_acl_entry->key.aclid;
    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
    if (NULL == pst_fea_acl)
    {
        return PM_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_ACL_COUNTER_ATTR_BYTES;

    p_interface = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_interface)
    {
        return PM_E_NONE;
    }

    if (p_interface->ifindex == p_acl_entry->in_port)
    {
        acl_api->set_acl_counter_attribute(pst_fea_acl->counter_id, &attr);
    }
    return PM_E_NONE;
}

int32
tap_clear_ingress_flow_counter(char *ifname, char *flow_name, uint32 *seq_no, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = ifname;
    args.argv[1] = flow_name;
    args.argv[2] = seq_no;

    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)_tap_clear_ingress_flow_counter, &args);
    return PM_E_NONE;
}

int32
hsrv_tap_cmd_clear_tap_flow_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 seq_no = 0;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (2 == argc)
        {
            tap_clear_ingress_flow_counter(argv[0], argv[1], NULL, para);
        }
        else if (1 == argc)
        {
            tap_clear_ingress_flow_counter(argv[0], NULL, NULL, para);
        }
        else if(3 == argc)
        {
            PM_CFG_GET_UINT(seq_no, argv[2], para->p_rs);
            tap_clear_ingress_flow_counter(argv[0], argv[1], &seq_no, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Wrong arg input");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support clear action");
        break;
    }

    return rc;    
}


static int32
_hsrv_acl_add_udf_entry(tbl_acl_udf_entry_t* acl_udf_entry)
{
    int32 rc = HSRV_E_NONE;
    sai_udf_api_t    *udf_api = NULL;
    sai_object_id_t   udf_match_id = 0;
#ifdef USW
    sai_object_id_t   udf_id = 0;
#endif
    sai_object_id_t   udf_group_id = 0;
    sai_attribute_t   attr_list[8];
    uint32 attr_count = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_UDF_GROUP_GENERIC;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_LENGTH;
    attr_list[attr_count].value.u16 = GLB_ACL_UDF_GROUP_LENGTH;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_GROUP_ATTR_UDF_ID;
    attr_list[attr_count].value.u32 = acl_udf_entry->key.id;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf_group(&udf_group_id, attr_count, attr_list));

    if (0 != acl_udf_entry->key_flags)
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_count = 0;
        attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = acl_udf_entry->key.id;
        attr_count++;

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_ETHTYPE))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L2_TYPE;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_udf_entry->ethtype;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_udf_entry->ethtype_mask;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_IPPROTO))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L3_TYPE;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u8 = acl_udf_entry->ipproto;
            attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_VLANNUM))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_VLANNUM;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u8 = acl_udf_entry->vlannum;
            attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_L4_TYPE;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u8 = acl_udf_entry->l4_type;
            attr_list[attr_count].value.aclfield.mask.u8 = acl_udf_entry->l4_type_mask;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_SRCPORT))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_SRCPORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_udf_entry->srcport;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_udf_entry->srcport_mask;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_DSTPORT))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_DSTPORT;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u16 = acl_udf_entry->dstport;
            attr_list[attr_count].value.aclfield.mask.u16 = acl_udf_entry->dstport_mask;
            attr_count++;
        }

        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_MPLSNUM))
        {
            attr_list[attr_count].id = SAI_UDF_MATCH_ATTR_MPLSNUM;
            attr_list[attr_count].value.aclfield.enable = TRUE;
            attr_list[attr_count].value.aclfield.data.u8 = acl_udf_entry->mplsnum;
            attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
            attr_count++;
        }

        HSRV_IF_ERROR_RETURN(udf_api->create_udf_match(&udf_match_id, attr_count, attr_list));
    }

#ifdef USW
    /* GG don't operate cam when first add */
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;
    attr_list[attr_count].id = SAI_UDF_ATTR_GROUP_ID;
    attr_list[attr_count].value.u32 = acl_udf_entry->key.id;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_BASE;
    attr_list[attr_count].value.u32 = acl_udf_entry->offset_type;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_OFFSET;
    attr_list[attr_count].value.u32 = acl_udf_entry->offset[0] | acl_udf_entry->offset[1] << 8
                             | acl_udf_entry->offset[2] << 16 | acl_udf_entry->offset[3] << 24;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf(&udf_id, attr_count, attr_list));
#endif

    return rc;
}

static int32
_hsrv_acl_del_udf_entry(tbl_acl_udf_entry_t* acl_udf_entry)
{
    int32 rc = HSRV_E_NONE;
    tbl_acl_udf_entry_t* p_db_acl_udf_entry = NULL;
    sai_udf_api_t *udf_api = NULL;
    sai_object_id_t udf_id;

    p_db_acl_udf_entry = tbl_acl_udf_entry_get_acl_udf_entry(&acl_udf_entry->key);
    if (NULL == p_db_acl_udf_entry)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));

    udf_id = p_db_acl_udf_entry->key.id;
    HSRV_IF_ERROR_RETURN(udf_api->remove_udf_group(udf_id));

    return rc;
}

static int32
_hsrv_acl_set_udf_offset(tbl_acl_udf_entry_t* acl_udf_entry)
{
    int32 rc = HSRV_E_NONE;
    tbl_acl_udf_entry_t* p_db_acl_udf_entry = NULL;
    sai_udf_api_t  *udf_api = NULL;
    sai_object_id_t udf_id;
    sai_attribute_t attr_list[3];
    uint32 attr_count = 0;

    p_db_acl_udf_entry = tbl_acl_udf_entry_get_acl_udf_entry(&acl_udf_entry->key);
    if (NULL == p_db_acl_udf_entry)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

#ifdef USW
    if (!sal_memcmp(acl_udf_entry->offset, p_db_acl_udf_entry->offset, sizeof(acl_udf_entry->offset)))
    {
        return rc;
    }
#endif

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_count = 0;
    attr_list[attr_count].id = SAI_UDF_ATTR_GROUP_ID;
    attr_list[attr_count].value.u32 = acl_udf_entry->key.id;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_BASE;
    attr_list[attr_count].value.u32 = acl_udf_entry->offset_type;
    attr_count++;

    attr_list[attr_count].id = SAI_UDF_ATTR_OFFSET;
    attr_list[attr_count].value.u32 = acl_udf_entry->offset[0] | acl_udf_entry->offset[1] << 8
                             | acl_udf_entry->offset[2] << 16 | acl_udf_entry->offset[3] << 24;
    attr_count++;

    HSRV_IF_ERROR_RETURN(udf_api->create_udf(&udf_id, attr_count, attr_list));

    return rc;
}

static int32
_hsrv_acl_set_udf_key(tbl_acl_udf_entry_t* acl_udf_entry)
{
    int32 rc = HSRV_E_NONE;
    tbl_acl_udf_entry_t* p_db_acl_udf_entry = NULL;
    sai_udf_api_t   *udf_api = NULL;
    sai_object_id_t  udf_id;
    sai_attribute_t  attr;

    p_db_acl_udf_entry = tbl_acl_udf_entry_get_acl_udf_entry(&acl_udf_entry->key);
    if (NULL == p_db_acl_udf_entry)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (acl_udf_entry->key_flags == p_db_acl_udf_entry->key_flags)
    {
        return rc;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_UDF,(void**)&udf_api));

    sal_memset(&attr, 0, sizeof(attr));

    udf_id = p_db_acl_udf_entry->key.id;
    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_ETHTYPE) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_ETHTYPE))
    {
        attr.id = SAI_UDF_MATCH_ATTR_L2_TYPE;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_ETHTYPE))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u16 = acl_udf_entry->ethtype;
        attr.value.aclfield.mask.u16 = acl_udf_entry->ethtype_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_VLANNUM) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_VLANNUM))
    {
        attr.id = SAI_UDF_MATCH_ATTR_VLANNUM;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_VLANNUM))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u8 = acl_udf_entry->vlannum;
        attr.value.aclfield.mask.u8 = acl_udf_entry->vlannum_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    /* process l4_type before ip protocol */
    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_L4_TYPE) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_L4_TYPE))
    {
        attr.id = SAI_UDF_MATCH_ATTR_L4_TYPE;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_L4_TYPE))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u8 = acl_udf_entry->l4_type;
        attr.value.aclfield.mask.u8 = acl_udf_entry->l4_type_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_IPPROTO) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_IPPROTO))
    {
        attr.id = SAI_UDF_MATCH_ATTR_L3_TYPE;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_IPPROTO))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u8 = acl_udf_entry->ipproto;
        attr.value.aclfield.mask.u8 = acl_udf_entry->ipproto_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_SRCPORT) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_SRCPORT))
    {
        attr.id = SAI_UDF_MATCH_ATTR_SRCPORT;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_SRCPORT))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u16 = acl_udf_entry->srcport;
        attr.value.aclfield.mask.u16 = acl_udf_entry->srcport_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_DSTPORT) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_DSTPORT))
    {
        attr.id = SAI_UDF_MATCH_ATTR_DSTPORT;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_DSTPORT))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u16 = acl_udf_entry->dstport;
        attr.value.aclfield.mask.u16 = acl_udf_entry->dstport_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    if ((acl_udf_entry->key_flags & GLB_ACL_UDF_MPLSNUM) != (p_db_acl_udf_entry->key_flags & GLB_ACL_UDF_MPLSNUM))
    {
        attr.id = SAI_UDF_MATCH_ATTR_MPLSNUM;
        if (GLB_FLAG_ISSET(acl_udf_entry->key_flags, GLB_ACL_UDF_MPLSNUM))
        {
            attr.value.aclfield.enable = TRUE;
        }
        else
        {
            attr.value.aclfield.enable = FALSE;
        }
        attr.value.aclfield.data.u8 = acl_udf_entry->mplsnum;
        attr.value.aclfield.mask.u8 = acl_udf_entry->mplsnum_mask;
        HSRV_IF_ERROR_RETURN(udf_api->set_udf_match_attribute(udf_id, &attr));
    }

    return rc;
}

int32
hsrv_acl_udf_entry_cmd_process(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, 
                               uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_acl_udf_entry_t  *p_acl_udf_entry  =  NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_acl_udf_entry = (tbl_acl_udf_entry_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(_hsrv_acl_del_udf_entry(p_acl_udf_entry));
        break;

    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_ACL_UDF_ENTRY_FLD_OFFSET:
            HSRV_IF_ERROR_RETURN(_hsrv_acl_set_udf_offset(p_acl_udf_entry));
            break;

        case TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS:
            HSRV_IF_ERROR_RETURN(_hsrv_acl_set_udf_key(p_acl_udf_entry));
            break;

        default:
            break;
        }
        
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_acl_udf_entry_cmd_process_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, 
                                     uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_acl_udf_entry_t  *p_acl_udf_entry = NULL;

    HSRV_ACL_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_acl_udf_entry = (tbl_acl_udf_entry_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(_hsrv_acl_add_udf_entry(p_acl_udf_entry));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_acl_start()
{
    cdb_pm_subscribe_tbl(TBL_ACL_ENTRY, hsrv_acl_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_ACL_ENTRY_ACTION, hsrv_acl_entry_action_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_CLEAR_ACL_POLICY, hsrv_acl_clear_acl_policy_stats, NULL);

    cdb_pm_subscribe_tbl(TBL_ACL_L4_PORT_RANGE, hsrv_acl_l4_port_range_cmd_process, NULL);
    cdb_pm_subscribe_tbl(TBL_ACL_UDF_GROUP, hsrv_acl_udf_group_cmd_process, hsrv_acl_udf_group_cmd_process_after);
    cdb_pm_subscribe_tbl(TBL_ACL_UDF_ENTRY, hsrv_acl_udf_entry_cmd_process, hsrv_acl_udf_entry_cmd_process_after);
    
    cdb_register_cfg_tbl_cb(TBL_FEA_ACL_TABLE, acl_table_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_FEA_ACL,       fea_acl_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_FEA_ACL_POLICY_ACTION,       fea_acl_policy_action_cmd_process);
    cdb_register_cfg_act_cb(ACT_SHOW_ACL_COUNTER, hsrv_acl_cmd_show_acl_counter);
    cdb_register_cfg_act_cb(ACT_CLEAR_ACL_COUNTER, hsrv_acl_cmd_clear_acl_counter);
    cdb_register_cfg_act_cb(ACT_SHOW_ACL_WORM_FILTER_COUNTER, hsrv_acl_worm_filter_show_counter);

    cdb_register_cfg_act_cb(ACT_SHOW_TAP_FLOW_COUNTER, hsrv_tap_cmd_show_tap_flow_counter);
    cdb_register_cfg_act_cb(ACT_CLEAR_TAP_FLOW_COUNTER, hsrv_tap_cmd_clear_tap_flow_counter);
    cdb_register_cfg_act_cb(ACT_SHOW_TAP_COUNTER, hsrv_tap_cmd_show_tap_counter);
    
    return HSRV_E_NONE;
}

int32
hsrv_acl_init()
{
    tbl_fea_acl_table_t fea_acl_table;
    sai_acl_api_t *acl_api = NULL;
    sai_switch_api_t*  switch_api = NULL;
    sai_attribute_t  switch_attr[1];
    sai_attribute_t attr_list[12];
    sai_object_id_t acl_table_id = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    /* ingress ACL table created */
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[0].id = SAI_ACL_TABLE_ATTR_STAGE;
    attr_list[0].value.s32 = SAI_ACL_STAGE_INGRESS;

    attr_list[1].id = SAI_ACL_TABLE_ATTR_PRIORITY;
    attr_list[1].value.u32 = 0;

    attr_list[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_list[2].value.booldata = TRUE;

    attr_list[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_list[3].value.booldata = TRUE;

    attr_list[4].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_list[4].value.booldata = TRUE;

    attr_list[5].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_list[5].value.booldata = TRUE;

    attr_list[6].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
    attr_list[6].value.booldata = TRUE;

    attr_list[7].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[7].value.booldata = TRUE;

    attr_list[8].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    attr_list[8].value.booldata = TRUE;

    attr_list[9].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    attr_list[9].value.booldata = TRUE;

    attr_list[10].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr_list[10].value.booldata = TRUE;

    attr_list[11].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_list[11].value.booldata = TRUE;
    HSRV_IF_ERROR_RETURN(acl_api->create_acl_table(&acl_table_id, 12, attr_list));

    sal_memset(&fea_acl_table, 0, sizeof(fea_acl_table));
    fea_acl_table.key.acl_table_id = GLB_IGS_ACL_TBLID;
    fea_acl_table.direct_in = TRUE;
    fea_acl_table.priority = 0;
    sal_strcpy(fea_acl_table.table_name, "ingressacl");
    fea_acl_table.sai_table_id = acl_table_id;
    HSRV_IF_ERROR_RETURN(tbl_fea_acl_table_add_fea_acl_table(&fea_acl_table));

    /* egress ACL table created */
    attr_list[0].id = SAI_ACL_TABLE_ATTR_STAGE;
    attr_list[0].value.s32 = SAI_ACL_STAGE_EGRESS;

    attr_list[1].id = SAI_ACL_TABLE_ATTR_PRIORITY;
    attr_list[1].value.u32 = 0;

    attr_list[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_list[2].value.booldata = TRUE;

    attr_list[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_list[3].value.booldata = TRUE;

    attr_list[4].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_list[4].value.booldata = TRUE;

    attr_list[5].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_list[5].value.booldata = TRUE;

    attr_list[6].id = SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT;
    attr_list[6].value.booldata = TRUE;

    attr_list[7].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[7].value.booldata = TRUE;

    attr_list[8].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    attr_list[8].value.booldata = TRUE;

    attr_list[9].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    attr_list[9].value.booldata = TRUE;

    attr_list[10].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr_list[10].value.booldata = TRUE;
    
    attr_list[11].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_list[11].value.booldata = TRUE;
    HSRV_IF_ERROR_RETURN(acl_api->create_acl_table(&acl_table_id, 12, attr_list));

    sal_memset(&fea_acl_table, 0, sizeof(fea_acl_table));
    fea_acl_table.key.acl_table_id = GLB_EGS_ACL_TBLID;
    fea_acl_table.direct_in = FALSE;
    fea_acl_table.priority = 0;
    sal_strcpy(fea_acl_table.table_name, "egressacl");
    fea_acl_table.sai_table_id = acl_table_id;
    HSRV_IF_ERROR_RETURN(tbl_fea_acl_table_add_fea_acl_table(&fea_acl_table));

    /*added by yejl for ParserEthernetCtl ipv6TypeEn*/
#if 0
    if ((FALSE == hsrv_acl_stm_is_ipv6_profile_of()) &&
        (FALSE == hsrv_acl_stm_is_ipv6_profile()))
    {
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
        
        switch_attr[0].id = SAI_SWITCH_ATTR_CUSTOM_IPV6_ENABLE;
        switch_attr[0].value.u32 = FALSE;
        HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(switch_attr));    
    }
#endif
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    
    switch_attr[0].id = SAI_SWITCH_ATTR_CUSTOM_IPV6_ENABLE;
    if (hsrv_acl_stm_is_ipv6_profile_of() || hsrv_acl_stm_is_ipv6_profile())
    {
        switch_attr[0].value.u32 = TRUE;
    }
    else
    {
        switch_attr[0].value.u32 = FALSE;            
    }
#ifdef TAPPRODUCT
    /* ParserEthernetCtl ipv6TypeEn should always be enabled on TAP product */
    switch_attr[0].value.u32 = TRUE;
#endif
    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(switch_attr));
    
    return HSRV_E_NONE;
}


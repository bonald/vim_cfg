/**
 @file ctc_usw_acl.c

 @date 2009-10-17

 @version v2.0

 The file contains acl APIs
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_linklist.h"
#include "ctc_usw_acl.h"

#include "sys_usw_acl_api.h"
#include "sys_usw_common.h"

typedef int32 (*_ctc_usw_acl_set_field_t)(uint8 lchip, uint32 entry_id, void* p_field);

STATIC int32
_ctc_usw_acl_set_field_list(uint8 lchip, uint32 entry_id, void* p_field_list, uint32* p_field_cnt, _ctc_usw_acl_set_field_t fn, uint8 is_key, uint8 is_add)
{
    uint8   lchip_start   = 0;
    uint8   lchip_end     = 0;
    uint8   count         = 0;
    uint8   error_cnt     = 0;
    int32   ret           = CTC_E_NONE;
    uint32  field_id      = 0;
    uint32  field_cnt     = 0;
    void*   p_field       = NULL;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_field_list);
    CTC_PTR_VALID_CHECK(p_field_cnt);
    CTC_PTR_VALID_CHECK(fn);

    field_cnt = *p_field_cnt;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        for (field_id=0; field_id<field_cnt; field_id+=1)
        {
            p_field = is_key? (void*)((ctc_field_key_t*)p_field_list + field_id): (void*)((ctc_acl_field_action_t*)p_field_list + field_id);
            CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                     ret,
                                     fn(lchip, entry_id, p_field));
            if (CTC_E_NONE != ret && is_add)
            {
                break;
            }
            else if (CTC_E_NONE != ret && !is_add)
            {
                error_cnt+=1;
            }
        }
        if (CTC_E_NONE != ret)
        {
            *p_field_cnt = is_add? field_id: field_id-error_cnt;
            break;
        }
    }

    return ret;
}

STATIC int32
  _ctc_usw_acl_map_mac_key(uint8 lchip, uint32 entry_id,ctc_acl_mac_key_t* p_ctc_key)
{
    uint32 flag = 0;
    uint32 temp = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
    flag = p_ctc_key->flag;
    /* mac da */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_MAC_DA))         /**< = 1U << 0, [HB.GB.GG.D2] Valid MAC-DA in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* mac sa */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_MAC_SA))         /**< = 1U << 1, [HB.GB.GG.D2] Valid MAC-SA in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* cvlan */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CVLAN))          /**< = 1U << 3, [HB.GB.GG.D2] Valid C-VLAN in MAC key */
    {
        if(p_ctc_key->cvlan_end)
        {   /*/bug!!!!!!!!!*/
            key_field.type = CTC_FIELD_KEY_CVLAN_RANGE;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_ID;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* ctag cos */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_COS))          /**< = 1U << 4,[HB.GB.GG.D2] Valid C-tag CoS in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* svlan */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_SVLAN))             /**< = 1U << 5, [HB.GB.GG.D2] Valid S-VLAN in MAC key */
    {
        if(p_ctc_key->svlan_end)
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* stag cos */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_COS))         /**< = 1U << 6, [HB.GB.GG.D2] Valid S-tag CoS in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* eth type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_ETH_TYPE))         /**< = 1U << 7, [HB.GB.GG.D2] Valid eth-type in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* l2 type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_L2_TYPE))          /**< = 1U << 8, [HB.GB.D2] Valid l2-type in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* l3 type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_L3_TYPE))          /**<= 1U << 9, [HB.GB.GG] Valid l3-type in MAC key */
    {/*D2 not support*/
        return CTC_E_NOT_SUPPORT;
    }

    /* ctag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_CFI))         /**< = 1U << 10,[HB.GB.GG.D2] Valid l2-type in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* stag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_CFI))        /**< = 1U << 11, [HB.GB.GG.D2] Valid l3-type in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_ARP_OP_CODE)
		|| CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_IP_SA)
		|| CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_IP_DA) )     /**< = 1U << 12, [GB] Valid Arp-op-code in MAC key */
    {/*D2 not support*/
        return CTC_E_NOT_SUPPORT;
    }

    /*stag valid*/
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_VALID))     /**< = 1U << 15, [GB.GG.D2] Valid stag-valid in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /*ctag valid*/
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_VALID))     /**< = 1U << 16, [GB.GG.D2] Valid ctag-valid in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* vlan num */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_VLAN_NUM))       /**<= 1U << 17, [GG.D2] Valid vlan number in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* metadata */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_METADATA))      /**< = 1U << 18  [GG.D2] Valid metadata in MAC key */
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_mac_ipv4_key(uint8 lchip, uint32 entry_id, ctc_acl_ipv4_key_t* p_ctc_key)
{
    uint32  flag      = 0;
    uint32  sub_flag  = 0;
    uint32  sub1_flag = 0;
    uint32  sub3_flag = 0;
    uint32  temp      = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV4;
    key_field.mask = 0xFFFF;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
    flag      = p_ctc_key->flag;
    sub_flag  = p_ctc_key->sub_flag;
    sub1_flag = p_ctc_key->sub1_flag;
    sub3_flag = p_ctc_key->sub3_flag;
    /* flag */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA))      /**< = 1U << 0,[HB.GB.GG.D2] Valid MAC-DA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA))      /**< = 1U << 1, [HB.GB.GG.D2] Valid MAC-SA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN))       /**<  = 1U << 3, [HB.GB.GG.D2] Valid C-VLAN in IPv4 key */
    {
        if(p_ctc_key->cvlan_end)
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_RANGE;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_ID;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS))    /**<  = 1U << 4, [HB.GB.GG.D2] Valid C-tag CoS in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN))       /**< = 1U << 5, [HB.GB.GG.D2] Valid S-VLAN in IPv4 key */
    {
        if(p_ctc_key->svlan_end)
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS))     /**<= 1U << 6,  [HB.GB.GG.D2] Valid S-tag CoS in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L2_TYPE))   /**<= 1U << 7, [HB.GB] Valid l2-type in IPv4 key, NOTE: Limited support on GB, When merge key,
                                                                    collision with FLAG_DSCP/FLAG_PRECEDENCE, And ineffective for ipv4_packet*/
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE))   /**< [GB.GG.D2]= 1U << 22, Valid Ether type in IPv4 Key, NOTE:  (GB) Used when merge key, collision with L4_DST_PORT,
                                                                    Do NOT set this flag on Egress ACL. And If required matching ipv4 packet,
                                                                    Do NOT set this flag with set ether_type = 0x0800, Set CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET instead */
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE))  /**<= 1U << 8, [HB.GG.D2] Valid l3-type in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        key_field.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA))        /**<= 1U << 9, [HB.GB.GG.D2] Valid IP-SA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ctc_key->ip_sa;
        key_field.mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA))        /**<= 1U << 10, [HB.GB.GG.D2] Valid IP-DA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ctc_key->ip_da;
        key_field.mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL))  /**<= 1U << 11, [HB.GB.GG.D2] Valid L4-Proto in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_DSCP))         /**<= 1U << 12, [HB.GB.GG.D2] Valid DSCP in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
       CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_PRECEDENCE))  /**<= 1U << 13,  [HB.GB.GG.D2] Valid IP Precedence in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG))    /**<= 1U << 14, [HB.GB.GG.D2] Valid fragment in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;/*useless*/
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_OPTION))    /**<= 1U << 15, [HB.GB.GG.D2] Valid IP option in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_OPTIONS;
        key_field.data = p_ctc_key->ip_option;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET)) /**<= 1U << 16, [HB.GB.GG.D2] Valid routed packet in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ROUTED_PKT;
        key_field.data = p_ctc_key->routed_packet;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_CFI))      /**<= 1U << 17, [HB.GB] Valid C-tag cfi in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_CFI))     /**<= 1U << 18, [HB.GB.GG.D2] Valid S-tag cfi in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID))   /**<= 1U << 19, [GB.GG.D2] Valid Stag-valid in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID))   /**<= 1U << 20,  [GB.GG.D2] Valid Ctag-valid in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ECN))           /**<= 1U << 25,  [GG.D2] Valid Ecn in IPv4 Key*/
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        key_field.mask = p_ctc_key->ecn_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
        /*metadata */
    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_METADATA))      /**<= 1U << 26, [GG.D2] Valid metadata in ipv4 key */
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_UDF))          /**<= 1U << 27, [GG.D2] Valid udf in ipv4 key */
    {
       /*/ need support*/
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_VLAN_NUM))    /**<= 1U << 28, [GG.D2] Valid vlan number in ipv4 key */
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_PKT_LEN_RANGE)) /**<= 1U << 29, [GG.D2] Valid packet length range in ipv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PKT_LEN_RANGE;
        key_field.data = p_ctc_key->pkt_len_min;
        key_field.mask = p_ctc_key->pkt_len_max;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_HDR_ERROR))    /**<= 1U << 30, [GG.D2] Valid ip header error in ipv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_HDR_ERROR;
        key_field.data = p_ctc_key->ip_hdr_error;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* sub flag */
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT)) /**<= 1U << 0,[HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP, Valid L4 source port in IPv4 key */
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT)) /**<= 1U << 1,[HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP, Valid L4 destination port in IPv4 key */
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS))  /**<= 1U << 2,[HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP, Valid TCP flag in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE)
 	  || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE))
    {
    	key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE))  /**<= 1U << 3,[HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP, Valid ICMP type in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE)) /**<= 1U << 4,[HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP, Valid ICMP code in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE)) /**<= 1U << 5,[HB.GB.GG.D2] Depend on L4_PROTOCOL = IGMP, Valid IGMP type in IPv4 key */
    {
    	key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_IGMP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

        key_field.type = CTC_FIELD_KEY_IGMP_TYPE;
        key_field.data = p_ctc_key->igmp_type;
        key_field.mask = p_ctc_key->igmp_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }


    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE)) /**<= 1U << 6,[GB.GG.D2] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp op code in Arp Key */
    {
        key_field.type = CTC_FIELD_KEY_ARP_OP_CODE;
        key_field.data = p_ctc_key->arp_op_code;
        key_field.mask = p_ctc_key->arp_op_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP)) /**<= 1U << 7,[GB.GG.D2] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp sender Ip in Arp Key */
    {
        key_field.type = CTC_FIELD_KEY_ARP_SENDER_IP;
        key_field.data = p_ctc_key->sender_ip;
        key_field.mask = p_ctc_key->sender_ip_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP)) /**<= 1U << 8,[GB.GG.D2] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp target Ip in Arp Key */
    {
        key_field.type = CTC_FIELD_KEY_ARP_TARGET_IP;
        key_field.data = p_ctc_key->target_ip;
        key_field.mask = p_ctc_key->target_ip_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE)) /**<= 1U << 9,[GG.D2] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp target Ip in Arp Key */
    {
        key_field.type = CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;
        key_field.data = p_ctc_key->arp_protocol_type;
        key_field.mask = p_ctc_key->arp_protocol_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_GRE_KEY))     /**<= 1U << 10, [GB.GG.D2] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp op code in Arp Key */
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_VNI))         /**<= 1U << 11, [GG.D2] Depend on L4_PROTOCOL = UDP, Valid vni in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_UDP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

		key_field.type = CTC_FIELD_KEY_L4_USER_TYPE;
        key_field.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        key_field.mask = p_ctc_key->vni_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_NVGRE_KEY))  /**<= 1U << 12, [GG.D2] Depend on L4_PROTOCOL = NVGRE, Valid NVGRE key in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* sub1_flag */
    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL_NUM))     /**<= 1U << 0, [GG.D2] MPLS label number field*/
    {
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_ctc_key->mpls_label_num;
        key_field.mask = p_ctc_key->mpls_label_num_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL0))       /**< = 1U << 1, [GG.D2] MPLS label 0 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = (p_ctc_key->mpls_label0 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 12) & 0xFFFFF;
        if(key_field.mask != 0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = (p_ctc_key->mpls_label0 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 9) & 0x7;
        if(key_field.mask != 0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = (p_ctc_key->mpls_label0 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 8) & 0x1;
        if(key_field.mask != 0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_ctc_key->mpls_label0 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label0_mask & 0xFF;
        if(key_field.mask != 0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL1))      /**<= 1U << 2, [GG.D2] MPLS label 1 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = (p_ctc_key->mpls_label1 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 12) & 0xFFFFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = (p_ctc_key->mpls_label1 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 9) & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = (p_ctc_key->mpls_label1 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 8) & 0x1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_ctc_key->mpls_label1 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label1_mask & 0xFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL2))     /**< = 1U << 3, [GG.D2] MPLS label 2 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = (p_ctc_key->mpls_label2 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 12) & 0xFFFFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = (p_ctc_key->mpls_label2 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 9) & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = (p_ctc_key->mpls_label2 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 8) & 0x1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_ctc_key->mpls_label2 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label2_mask & 0xFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* sub3_flag */
    if (CTC_FLAG_ISSET(sub3_flag, CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_LEVEL))   /**<= 1U << 0, [GB.GG.D2] Ether oam level as key field*/
    {
        key_field.type = CTC_FIELD_KEY_ETHER_OAM_LEVEL;
        key_field.data = p_ctc_key->ethoam_level;
        key_field.mask = p_ctc_key->ethoam_level_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub3_flag, CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_OPCODE))  /**<= 1U << 1, [GB.GG.D2] Ether oam opcode as key field*/
    {
        key_field.type = CTC_FIELD_KEY_ETHER_OAM_OP_CODE;
        key_field.data = p_ctc_key->ethoam_op_code;
        key_field.mask = p_ctc_key->ethoam_op_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    return CTC_E_NONE;
}


STATIC int32
_ctc_usw_acl_map_mpls_to_mac_ipv4_key(uint8 lchip, uint32 entry_id, ctc_acl_mpls_key_t* p_ctc_key)
{
    uint32  flag      = 0;
    uint32  temp      = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
    flag      = p_ctc_key->flag;

    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_MPLS;
    key_field.mask = 0xF;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    /* flag */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MAC_DA))      /**< = 1U << 0,[HB.GB.GG.D2] Valid MAC-DA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MAC_SA))      /**< = 1U << 1, [HB.GB.GG.D2] Valid MAC-SA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_CVLAN))       /**<  = 1U << 3, [HB.GB.GG.D2] Valid C-VLAN in IPv4 key */
    {
        if(p_ctc_key->cvlan_end)
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_RANGE;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_ID;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_CTAG_COS))    /**<  = 1U << 4, [HB.GB.GG.D2] Valid C-tag CoS in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_SVLAN))       /**< = 1U << 5, [HB.GB.GG.D2] Valid S-VLAN in IPv4 key */
    {
        if(p_ctc_key->svlan_end)
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS))     /**<= 1U << 6,  [HB.GB.GG.D2] Valid S-tag CoS in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_CTAG_CFI))      /**<= 1U << 17, [HB.GB] Valid C-tag cfi in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_STAG_CFI))     /**<= 1U << 18, [HB.GB.GG.D2] Valid S-tag cfi in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

   /*metadata */
    if(CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_METADATA))      /**<= 1U << 26, [GG.D2] Valid metadata in ipv4 key */
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    /* sub1_flag */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL_NUM))     /**<= 1U << 0, [GG.D2] MPLS label number field*/
    {
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_ctc_key->mpls_label_num;
        key_field.mask = p_ctc_key->mpls_label_num_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL0))       /**< = 1U << 1, [GG.D2] MPLS label 0 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = (p_ctc_key->mpls_label0 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 12) & 0xFFFFF;
        if(key_field.mask!=0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = (p_ctc_key->mpls_label0 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 9) & 0x7;
        if(key_field.mask!=0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = (p_ctc_key->mpls_label0 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 8) & 0x1;
        if(key_field.mask!=0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_ctc_key->mpls_label0 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label0_mask & 0xFF;
        if(key_field.mask!=0)
        {
            CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL1))      /**<= 1U << 2, [GG.D2] MPLS label 1 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = (p_ctc_key->mpls_label1 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 12) & 0xFFFFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = (p_ctc_key->mpls_label1 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 9) & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = (p_ctc_key->mpls_label1 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 8) & 0x1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_ctc_key->mpls_label1 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label1_mask & 0xFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL2))     /**< = 1U << 3, [GG.D2] MPLS label 2 as key field*/
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = (p_ctc_key->mpls_label2 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 12) & 0xFFFFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = (p_ctc_key->mpls_label2 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 9) & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = (p_ctc_key->mpls_label2 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 8) & 0x1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_ctc_key->mpls_label2 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label2_mask & 0xFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_ROUTED_PACKET))  /**<= 1U << 16, [HB.GB.GG.D2] Valid routed packet in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ROUTED_PKT;
        key_field.data = p_ctc_key->routed_packet;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if (CTC_FLAG_ISSET(flag, CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL3))
	{
	 	return CTC_E_NOT_SUPPORT;
	}


    return CTC_E_NONE;
}
STATIC int32
_ctc_usw_acl_map_ipv6_key(uint8 lchip, uint32 entry_id, ctc_acl_ipv6_key_t* p_ctc_key)
{
    uint32  flag     = 0;
    uint32  sub_flag = 0;
    uint32  temp     = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
	key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV6;
    key_field.mask = 0xF;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));


    flag     = p_ctc_key->flag;
    sub_flag = p_ctc_key->sub_flag;

    /*The sequence is according to ctc_acl_ipv6_key_flag_t */
    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA))       /**< = 1U << 0,[HB.GB.GG.D2] Valid MAC-DA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA))       /**< = 1U << 1,[HB.GB.GG.D2] Valid MAC-SA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN))       /**< = 1U << 3, [HB.GB.GG.D2] Valid CVLAN in IPv6 key */
    {
        if(p_ctc_key->cvlan_end)
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_RANGE;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_ID;
            key_field.data = p_ctc_key->cvlan;
            key_field.mask = p_ctc_key->cvlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS))   /**< = 1U << 4, [HB.GB.GG.D2] Valid Ctag CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_SVLAN))     /**< = 1U << 5, [HB.GB.GG.D2] Valid SVLAN in IPv6 key */
    {
        if(p_ctc_key->svlan_end)
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->svlan;
            key_field.mask = p_ctc_key->svlan_mask;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS))     /**< = 1U << 6, [HB.GB.GG.D2] Valid Stag CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ETH_TYPE))      /**<= 1U << 7,  [HB.GB.GG] Valid Ether-type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L2_TYPE))      /**< = 1U << 8, [HB.GB] Valid L2-type CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE))    /**< = 1U << 9, [HB.GB.GG] Valid L3-type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        key_field.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA))      /**<= 1U << 10, [HB.GB.GG.D2] Valid IPv6-SA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_SA;
        key_field.ext_data = p_ctc_key->ip_sa;
        key_field.ext_mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA))      /**<= 1U << 11, [HB.GB.GG.D2] Valid IPv6-DA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_DA;
        key_field.ext_data = p_ctc_key->ip_da;
        key_field.ext_mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL))  /**<= 1U << 12, [HB.GB.GG.D2] Valid L4-Protocol in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_DSCP))         /**<= 1U << 13, [HB.GB.GG.D2] Valid DSCP in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_PRECEDENCE))   /**<= 1U << 14, [HB.GB.GG.D2] Valid IP Precedence in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_FRAG))      /**<= 1U << 15, [HB.GB.GG.D2] Valid fragment in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_OPTION))    /**< = 1U << 16, [HB.GB.GG.D2] Valid IP option in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_OPTIONS;
        key_field.data = p_ctc_key->ip_option;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ROUTED_PACKET))  /**< =1U << 17, [HB.GB.GG.D2] Valid Routed-packet in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ROUTED_PKT;
        key_field.data = p_ctc_key->routed_packet;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL))     /**< = 1U << 19, [HB.GB.GG.D2] Valid IPv6 flow label in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;
        key_field.data = p_ctc_key->flow_label;
        key_field.mask = p_ctc_key->flow_label_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_CFI))       /**< = 1U << 20, [HB.GB.GG.D2] Valid C-tag Cfi in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_CFI))      /**< = 1U << 21, [HB.GB.GG.D2] Valid S-tag Cfi in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID))    /**< = 1U << 22, [GB.GG.D2] Valid Stag-valid in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID))    /**< = 1U << 23, [GB.GG.D2] Valid Ctag-valid in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ECN))          /**< = 1U << 24, [GG.D2] Valid ECN in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        key_field.mask = p_ctc_key->ecn_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_METADATA))
    {
        key_field.type = CTC_FIELD_KEY_METADATA;                  /**< = 1U << 25, [GG.D2] Valid metadata in ipv6 key */
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_UDF))          /**< = 1U << 26, [GG.D2] Valid udf in ipv6 key */
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_VLAN_NUM))     /**< = 1U << 27, [GG.D2] Valid vlan number in ipv6 key */
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_PKT_LEN_RANGE))  /**<= 1U << 28  [GG.D2] Valid packet length range in ipv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PKT_LEN_RANGE;
        key_field.data = p_ctc_key->pkt_len_min;
        key_field.mask = p_ctc_key->pkt_len_max;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_HDR_ERROR))    /**<= 1U << 29, [GG.D2] Valid ip header error in ipv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_HDR_ERROR;
        key_field.data = p_ctc_key->ip_hdr_error;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /*The sequence is according to ctc_acl_ipv6_key_sub_flag_t */
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT))  /**<  = 1U << 0, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP Valid L4 source port in IPv6 key */
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT))   /**<= 1U << 1, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP Valid L4 destination port in IPv6 key */
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_TCP_FLAGS))     /**<= 1U << 2, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP    Valid TCP flags in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE)
 	  || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE))
    {
    	key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field))
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_TYPE))     /**<= 1U << 3, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP   Valid ICMP type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_CODE))     /**<= 1U << 4, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP   Valid ICMP code in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }


	if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY)
 	  || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY))
    {
    	key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY))       /**< = 1U << 5, [GG.D2] Depend on L4_PROTOCOL = GRE    Valid GRE key in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY))     /**< = 1U << 7 [GG.D2] Depend on L4_PROTOCOL = NVGRE    Valid NVGRE key in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_VNI))           /**< = 1U << 6,[GG.D2] Depend on L4_PROTOCOL = UDP    Valid vni in IPv6 key */
    {
	    key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_UDP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

		key_field.type = CTC_FIELD_KEY_L4_USER_TYPE;
        key_field.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        key_field.mask = p_ctc_key->vni_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }


    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_pbr_ipv4_key(uint8 lchip, uint32 entry_id, ctc_acl_pbr_ipv4_key_t* p_ctc_key)
{
    uint32 flag     = 0;
    uint32 sub_flag = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV4;
    key_field.mask = 0xF;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    flag     = p_ctc_key->flag;
    sub_flag = p_ctc_key->sub_flag;

    /* flag */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_SA))     /**<= 1U << 0, [HB.GB.GG.D2] Valid IP-SA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ctc_key->ip_sa;
        key_field.mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_DA))     /**<= 1U << 1, [HB.GB.GG.D2] Valid IP-DA in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ctc_key->ip_da;
        key_field.mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_L4_PROTOCOL))  /**<= 1U << 2, [HB.GB.GG.D2] Valid L4-Proto in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_DSCP))       /**<= 1U << 3, [HB.GB.GG.D2] Valid DSCP in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_PRECEDENCE)) /**<= 1U << 4, [HB.GB.GG.D2] Valid IP Precedence in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_IP_FRAG))    /**<= 1U << 5, [HB.GB.GG.D2] Valid fragment in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV4_KEY_FLAG_VRFID))      /**<= 1U << 6, [HB.GB.GG.D2] Valid vrfid in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_VRFID;
        key_field.data = p_ctc_key->vrfid;
        key_field.mask = p_ctc_key->vrfid_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /* sub_flag */
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))   /**<= 1U << 0, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP  Valid L4 source port in IPv4 key */
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_L4_DST_PORT))  /**<= 1U << 1, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP  Valid L4 destination port in IPv4 key */
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
	if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_TCP_FLAGS))   /**<= 1U << 2, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP Valid TCP flag in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_TYPE)
 	   || CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_CODE))  /**<= 1U << 3, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP Valid ICMP type in IPv4 key */
    {
	    key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xFFFF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
 	}

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_TYPE))  /**<= 1U << 3, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP Valid ICMP type in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_CODE))  /**<= 1U << 4, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP Valid ICMP code in IPv4 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_pbr_ipv6_key(uint8 lchip, uint32 entry_id, ctc_acl_pbr_ipv6_key_t* p_ctc_key)
{
    uint32 flag     = 0;
	uint32 sub_flag = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_mask;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_mask, 0xFF, sizeof(ctc_field_port_t));

    CTC_ERROR_RETURN(sys_usw_acl_add_port_field(lchip, entry_id, p_ctc_key->port));
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV6;
    key_field.mask = 0xF;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));


    flag     = p_ctc_key->flag;
    sub_flag = p_ctc_key->sub_flag;

    /*The sequence is according to ctc_acl_ipv6_key_flag_t */
    if(CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_MAC_DA))       /**< = 1U << 0,[HB.GB.GG.D2] Valid MAC-DA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_MAC_SA))       /**< = 1U << 1,[HB.GB.GG.D2] Valid MAC-SA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_CVLAN))       /**< = 1U << 3, [HB.GB.GG.D2] Valid CVLAN in IPv6 key */
    {

        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_ctc_key->cvlan;
        key_field.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_CTAG_COS))   /**< = 1U << 4, [HB.GB.GG.D2] Valid Ctag CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_SVLAN))     /**< = 1U << 5, [HB.GB.GG.D2] Valid SVLAN in IPv6 key */
    {
       key_field.type = CTC_FIELD_KEY_SVLAN_ID;
       key_field.data = p_ctc_key->svlan;
       key_field.mask = p_ctc_key->svlan_mask;

        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_STAG_COS))     /**< = 1U << 6, [HB.GB.GG.D2] Valid Stag CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_ETH_TYPE))      /**<= 1U << 7,  [HB.GB.GG] Valid Ether-type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_L2_TYPE))      /**< = 1U << 8, [HB.GB] Valid L2-type CoS in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_L3_TYPE))    /**< = 1U << 9, [HB.GB.GG] Valid L3-type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        key_field.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }


    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_SA))      /**<= 1U << 10, [HB.GB.GG.D2] Valid IPv6-SA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_SA;
        key_field.ext_data = p_ctc_key->ip_sa;
        key_field.ext_mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_DA))      /**<= 1U << 11, [HB.GB.GG.D2] Valid IPv6-DA in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_DA;
        key_field.ext_data = p_ctc_key->ip_da;
        key_field.ext_mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_L4_PROTOCOL))  /**<= 1U << 12, [HB.GB.GG.D2] Valid L4-Protocol in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_DSCP))         /**<= 1U << 13, [HB.GB.GG.D2] Valid DSCP in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_PRECEDENCE))   /**<= 1U << 14, [HB.GB.GG.D2] Valid IP Precedence in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask & 0x7;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_IP_FRAG))      /**<= 1U << 15, [HB.GB.GG.D2] Valid fragment in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_FLOW_LABEL))     /**< = 1U << 19, [HB.GB.GG.D2] Valid IPv6 flow label in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;
        key_field.data = p_ctc_key->flow_label;
        key_field.mask = p_ctc_key->flow_label_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_CTAG_CFI))       /**< = 1U << 20, [HB.GB.GG.D2] Valid C-tag Cfi in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_STAG_CFI))      /**< = 1U << 21, [HB.GB.GG.D2] Valid S-tag Cfi in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
     if (CTC_FLAG_ISSET(flag, CTC_ACL_PBR_IPV6_KEY_FLAG_VRFID))      /**< = 1U << 21, [HB.GB.GG.D2] Valid S-tag Cfi in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_VRFID;
        key_field.data = p_ctc_key->vrfid;
        key_field.mask = p_ctc_key->vrfid_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    /*The sequence is according to ctc_acl_ipv6_key_sub_flag_t */
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_SRC_PORT))  /**<  = 1U << 0, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP Valid L4 source port in IPv6 key */
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_DST_PORT))   /**<= 1U << 1, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP|UDP Valid L4 destination port in IPv6 key */
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_TCP_FLAGS))     /**<= 1U << 2, [HB.GB.GG.D2] Depend on L4_PROTOCOL = TCP    Valid TCP flags in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

	if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_TYPE)
 	  || CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_CODE))
    {
    	key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field))
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_TYPE))     /**<= 1U << 3, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP   Valid ICMP type in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_CODE))     /**<= 1U << 4, [HB.GB.GG.D2] Depend on L4_PROTOCOL = ICMP   Valid ICMP code in IPv6 key */
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }


    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_mac_key(uint8 lchip, uint32 entry_id, ctc_acl_hash_mac_key_t* p_ctc_key)
{
    uint8 field_valid = 0;
    uint32 temp = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_info;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

    key_field.type = SYS_FIELD_KEY_HASH_SEL_ID;
    key_field.data = p_ctc_key->field_sel_id;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_PORT, &field_valid );
    if( CTC_FIELD_PORT_TYPE_GPORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_GPORT;
        port_info.gport = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_info.logic_port = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_METADATA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->gport;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MAC_DA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MAC_SA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ETHER_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_COS, &field_valid);
    if(field_valid)
    {
        key_field.type = p_ctc_key->is_ctag ? CTC_FIELD_KEY_CTAG_COS : CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->cos;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_CFI, &field_valid);
    if(field_valid)
    {
        key_field.type = p_ctc_key->is_ctag ? CTC_FIELD_KEY_CTAG_CFI : CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->cfi;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_SVLAN_ID, &field_valid);
    if(field_valid)
    {
        if(p_ctc_key->vlan_end)
        {
            key_field.type = p_ctc_key->is_ctag ? CTC_FIELD_KEY_CVLAN_RANGE : CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->vlan_id;
            key_field.mask = p_ctc_key->vlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = p_ctc_key->is_ctag ? CTC_FIELD_KEY_CVLAN_ID : CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->vlan_id;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_VALID, &field_valid);
    if(field_valid)
    {
        key_field.type = p_ctc_key->is_ctag ? CTC_FIELD_KEY_CTAG_VALID : CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->tag_valid;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_ipv4_key(uint8 lchip, uint32 entry_id, ctc_acl_hash_ipv4_key_t* p_ctc_key)
{
    uint8 field_valid = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_info;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

    key_field.type = SYS_FIELD_KEY_HASH_SEL_ID;
    key_field.data = p_ctc_key->field_sel_id;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_PORT, &field_valid );
    if( CTC_FIELD_PORT_TYPE_GPORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_GPORT;
        port_info.gport = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_info.logic_port = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_METADATA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_DA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ctc_key->ip_da;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_SA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ctc_key->ip_sa;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_DSCP, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_ECN, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_TYPE, &field_valid);
    if(field_valid)                     /*no ip-protocol field in hash ipv4 key*/
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_CODE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_SRC_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        key_field.data = p_ctc_key->l4_src_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_DST_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        key_field.data = p_ctc_key->l4_dst_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_VN_ID, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_GRE_KEY, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_l2l3_key(uint8 lchip, uint32 entry_id, ctc_acl_hash_l2_l3_key_t* p_ctc_key)
{
    uint8 field_valid = 0;
    uint32 temp = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_info;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

    key_field.type = SYS_FIELD_KEY_HASH_SEL_ID;
    key_field.data = p_ctc_key->field_sel_id;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ETHER_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L3_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_PORT, &field_valid );
    if( CTC_FIELD_PORT_TYPE_GPORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_GPORT;
        port_info.gport = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_info.logic_port = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_METADATA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MAC_DA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MAC_SA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_COS, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_CFI, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_SVLAN_ID, &field_valid);
    if(field_valid)
    {
        if(p_ctc_key->svlan_end)
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_RANGE;
            key_field.data = p_ctc_key->stag_vlan;
            key_field.mask = p_ctc_key->svlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_ctc_key->stag_vlan;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_STAG_VALID, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_CTAG_COS, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_CTAG_CFI, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_CVLAN_ID, &field_valid);
    if(field_valid)
    {
        if(p_ctc_key->cvlan_end)
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_RANGE;
            key_field.data = p_ctc_key->ctag_vlan;
            key_field.mask = p_ctc_key->cvlan_end;
            temp = p_ctc_key->vrange_grpid;
            key_field.ext_data = &temp;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_CVLAN_ID;
            key_field.data = p_ctc_key->ctag_vlan;
        }
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_CTAG_VALID, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_DA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ctc_key->ip_da;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_SA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ctc_key->ip_sa;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_DSCP, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_ECN, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_TTL, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_TTL;
        key_field.data = p_ctc_key->ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = p_ctc_key->l4_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_LABEL_NUM, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_ctc_key->mpls_label_num;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_ctc_key->mpls_label0_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = p_ctc_key->mpls_label0_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = p_ctc_key->mpls_label0_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = p_ctc_key->mpls_label0_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_ctc_key->mpls_label1_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = p_ctc_key->mpls_label1_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = p_ctc_key->mpls_label1_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = p_ctc_key->mpls_label1_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_ctc_key->mpls_label2_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = p_ctc_key->mpls_label2_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = p_ctc_key->mpls_label2_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = p_ctc_key->mpls_label2_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_SRC_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        key_field.data = p_ctc_key->l4_src_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_DST_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        key_field.data = p_ctc_key->l4_dst_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_CODE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_VN_ID, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_GRE_KEY, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_ipv6_key(uint8 lchip, uint32 entry_id, ctc_acl_hash_ipv6_key_t* p_ctc_key)
{
    uint8 field_valid = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_info;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

    key_field.type = SYS_FIELD_KEY_HASH_SEL_ID;
    key_field.data = p_ctc_key->field_sel_id;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_PORT, &field_valid );
    if( CTC_FIELD_PORT_TYPE_GPORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_GPORT;
        port_info.gport = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_info.logic_port = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_METADATA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->gport;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IPV6_SA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IPV6_SA;
        key_field.ext_data = p_ctc_key->ip_sa;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IPV6_DA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IPV6_DA;
        key_field.ext_data = p_ctc_key->ip_da;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_IP_DSCP, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = p_ctc_key->l4_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_TYPE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_ICMP_CODE, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_SRC_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        key_field.data = p_ctc_key->l4_src_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_L4_DST_PORT, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        key_field.data = p_ctc_key->l4_dst_port;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_VN_ID, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_GRE_KEY, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_mpls_key(uint8 lchip, uint32 entry_id, ctc_acl_hash_mpls_key_t* p_ctc_key)
{
    uint8 field_valid = 0;
    ctc_field_key_t key_field;
    ctc_field_port_t port_info;

    CTC_PTR_VALID_CHECK(p_ctc_key);

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));

    key_field.type = SYS_FIELD_KEY_HASH_SEL_ID;
    key_field.data = p_ctc_key->field_sel_id;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_PORT, &field_valid );
    if( CTC_FIELD_PORT_TYPE_GPORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_GPORT;
        port_info.gport = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }
    else if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == field_valid)
    {
        port_info.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_info.logic_port = p_ctc_key->gport;

        key_field.type = CTC_FIELD_KEY_PORT;
        key_field.ext_data = &port_info;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_METADATA, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_LABEL_NUM, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_ctc_key->mpls_label_num;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_ctc_key->mpls_label0_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = p_ctc_key->mpls_label0_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = p_ctc_key->mpls_label0_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL0, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = p_ctc_key->mpls_label0_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_ctc_key->mpls_label1_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = p_ctc_key->mpls_label1_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = p_ctc_key->mpls_label1_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL1, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = p_ctc_key->mpls_label1_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_TTL2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_ctc_key->mpls_label2_ttl;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_SBIT2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = p_ctc_key->mpls_label2_s;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_EXP2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = p_ctc_key->mpls_label2_exp;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    sys_usw_acl_get_hash_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, p_ctc_key->field_sel_id,
                                            CTC_FIELD_KEY_MPLS_LABEL2, &field_valid);
    if(field_valid)
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = p_ctc_key->mpls_label2_label;
        CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));
    }

    key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_add_key_field(lchip, entry_id, &key_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_check_action(uint8 lchip, uint32 flag)
{
    uint64 obsolete_flag = 0;

    obsolete_flag = CTC_ACL_ACTION_FLAG_TRUST
                  + CTC_ACL_ACTION_FLAG_COPY_TO_CPU_WITH_TIMESTAMP
                  + CTC_ACL_ACTION_FLAG_QOS_DOMAIN
                  + CTC_ACL_ACTION_FLAG_AGING
                  + CTC_ACL_ACTION_FLAG_PBR_TTL_CHECK
                  + CTC_ACL_ACTION_FLAG_PBR_ICMP_CHECK
                  + CTC_ACL_ACTION_FLAG_PBR_ECMP
                  + CTC_ACL_ACTION_FLAG_POSTCARD_EN
                  + CTC_ACL_ACTION_FLAG_ASSIGN_OUTPUT_PORT
                  + CTC_ACL_ACTION_FLAG_FORCE_ROUTE
                  + CTC_ACL_ACTION_FLAG_FID
                  + CTC_ACL_ACTION_FLAG_VRFID;

    if (flag & obsolete_flag)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_action_by_struct(uint8 lchip, uint32 entry_id, ctc_acl_action_t* p_ctc_action)
{
    uint64  flag        = 0;
    ctc_acl_field_action_t act_field;
    ctc_acl_to_cpu_t       cp_to_cpu;

    CTC_PTR_VALID_CHECK(p_ctc_action);
    CTC_ERROR_RETURN(_ctc_usw_acl_check_action(lchip, p_ctc_action->flag));

    sal_memset(&act_field, 0, sizeof(ctc_acl_field_action_t));
    sal_memset(&cp_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));

    flag = p_ctc_action->flag;

    if(CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_REDIRECT_WITH_RAW_PKT) && p_ctc_action->packet_strip.start_packet_strip)
    {
        return CTC_E_INVALID_CONFIG;
    }

    act_field.type = CTC_ACL_FIELD_ACTION_CANCEL_ALL;
    CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DISCARD))             /**<= 1U << 0, [HB.GB.GG.D2] Discard the packet i:ingress e:egress*/
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DISCARD;
        act_field.data0 = 0;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DENY_BRIDGE))         /**<= 1U << 1, [HB.GB.GG.D2] Deny bridging process */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DENY_BRIDGE;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING))       /**<= 1U << 2, [HB.GB.GG.D2] Deny learning process */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DENY_ROUTE))         /**<= 1U << 3, [HB.GB.GG.D2] Deny routing process */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DENY_ROUTE;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }
                                                                      /**<= 1U << 4, Empty */
                                                                      /**<= 1U << 5, Empty */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_STATS))              /**<= 1U << 6, [HB.GB.GG.D2] Statistic */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_STATS;
        act_field.data0 = p_ctc_action->stats_id;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }
                                                                      /**<= 1U << 7, Empty */
    /* priority&color */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR))    /**<= 1U << 8, [HB.GB.GG.D2] Priority color */
    {
        CTC_MAX_VALUE_CHECK(p_ctc_action->color, 3);
        CTC_MAX_VALUE_CHECK(p_ctc_action->priority, 0xF);
        act_field.type = CTC_ACL_FIELD_ACTION_PRIORITY;
        act_field.data0 = 0;
        act_field.data1 = p_ctc_action->priority;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
        act_field.type = CTC_ACL_FIELD_ACTION_COLOR;
        act_field.data0 = p_ctc_action->color;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_TRUST))               /**<= 1U << 9, [HB.GB.GG] QoS trust state */
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER))  /**<=1U << 10,  [HB.GB.GG.D2] Micro Flow policer */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
        act_field.data0 = p_ctc_action->micro_policer_id;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    /* random log */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG))          /**<= 1U << 11, [HB.GB.GG.D2] Log to any network port */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;
        act_field.data0 = p_ctc_action->log_session_id;
        act_field.data1 = p_ctc_action->log_percent;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU))        /**<= 1U << 12, [HB.GB.GG.D2] Copy to cpu */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
        cp_to_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
        act_field.ext_data = &cp_to_cpu;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    /* ds forward */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_REDIRECT))          /**<= 1U << 13, [HB.GB.GG.D2] Redirect */
    {
        if(!CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER) && !CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_MACRO_FLOW_POLICER))
        {
            act_field.data0 = 0;
            act_field.type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;
            /*can not be error return for hash not support cancel discard*/
            sys_usw_acl_add_action_field(lchip, entry_id, &act_field);
        }

        act_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
        act_field.data0 = p_ctc_action->nh_id;
        act_field.data1 = 0;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DSCP))             /**<= 1U << 14, [GB.GG.D2] Dscp */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DSCP;
        act_field.data0 = p_ctc_action->dscp;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU_WITH_TIMESTAMP))  /**<= 1U << 15, [GB] Copy to cpu with timestamp */
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_QOS_DOMAIN))          /**<= 1U << 16, [GB.GG] Qos domain */
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_MACRO_FLOW_POLICER))  /**<= 1U << 17, [GB.GG.D2] Macro Flow Policer */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER;
        act_field.data0 = p_ctc_action->macro_policer_id;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_AGING))              /**<= 1U << 18,  [HB] Aging */
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT))          /**<= 1U << 19, [GB.GG.D2] Vlan edit */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;
        act_field.ext_data = &(p_ctc_action->vlan_edit);
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_TTL_CHECK))      /**<= 1U << 20, [HB.GB.GG] Set PBR ttl-check flag */
    {

    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_ICMP_CHECK))     /**<= 1U << 21, [HB.GB.GG] Set PBR icmp-check flag */
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_CPU))           /**<= 1U << 22, [HB.GB.GG.D2] Set PBR copy-to-cpu flag */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
        cp_to_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
        act_field.ext_data = &cp_to_cpu;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_ECMP))          /**<= 1U << 23, [HB.GB] Set PBR ecmp flag which means use ecmp nexthop directly */
    {

    }

    /*flow pbr action*/
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_FWD))        /**<= 1U << 24, [HB.GB.GG.D2] Set PBR fwd flag */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
        act_field.data0 = p_ctc_action->nh_id;
        act_field.data1 = 0;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PBR_DENY))       /**<= 1U << 25, [HB.GB.GG.D2] Set PBR deny flag */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_DISCARD;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    /* priority */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_PRIORITY))           /**<= 1U << 26, [GG.D2] Priority*/
    {
        act_field.type = CTC_ACL_FIELD_ACTION_PRIORITY;
        act_field.data0 = 0;
        act_field.data1 = p_ctc_action->priority;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    /* color */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_COLOR))              /**<= 1U << 27,  [GG.D2] Color id */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_COLOR;
        act_field.data0 = p_ctc_action->color;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DISABLE_IPFIX))      /**<= 1U << 28, [GG.D2] Disable ipfix*/
    {
        act_field.type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_DENY_IPFIX_LEARNING))  /**<= 1U << 29, [GG.D2] Deny ipfix learning*/
    {
        act_field.type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_POSTCARD_EN))         /**<= 1U << 30, [GG] Enable postcard*/
    {

    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_CANCEL_DISCARD))      /**<= 1U << 31, [GG.D2] Cancel discard the packet */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;
        act_field.data0 = 0;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_ASSIGN_OUTPUT_PORT))  /**<= 1LLU << 32, [GG] DestId assigned by user */
    {

    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_FORCE_ROUTE))         /**<= 1LLU << 33, [GG] Force routing process,will cancel deny route,deny bridge */
    {

    }
    /* metadata */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_METADATA))            /**<= 1LLU << 34, [GG] Metadata */
    {
        act_field.type = CTC_ACL_FIELD_ACTION_METADATA;
        act_field.data0 = p_ctc_action->metadata;
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_FID))                /**<= 1LLU << 35, [GG] fid */
    {

    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_VRFID))              /**< = 1LLU << 36, [GG] Vrf ID of the route */
    {

    }

    /*strip packet*/
    if(p_ctc_action->packet_strip.start_packet_strip)
    {
        act_field.type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;
        act_field.ext_data = &(p_ctc_action->packet_strip);
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    if(CTC_FLAG_ISSET(flag, CTC_ACL_ACTION_FLAG_REDIRECT_WITH_RAW_PKT))
    {
        act_field.type = CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT;          /* more edit action in dt2 than gg */
        CTC_ERROR_RETURN(sys_usw_acl_add_action_field(lchip, entry_id, &act_field));
    }

    return CTC_E_NONE;
}


STATIC int32
_ctc_usw_acl_map_key_by_struct(uint8 lchip, uint32 entry_id,ctc_acl_key_t* p_key)
{
    CTC_PTR_VALID_CHECK(p_key);

    switch(p_key->type)
    {
        case CTC_ACL_KEY_MAC:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_mac_key(lchip, entry_id, &(p_key->u.mac_key)));
            break;
     	case CTC_ACL_KEY_IPV4:
			 CTC_ERROR_RETURN(_ctc_usw_acl_map_mac_ipv4_key(lchip, entry_id, &(p_key->u.ipv4_key)));
            break;
		case CTC_ACL_KEY_IPV6:
			CTC_ERROR_RETURN(_ctc_usw_acl_map_ipv6_key(lchip, entry_id, &(p_key->u.ipv6_key)));
            break;
		case CTC_ACL_KEY_MPLS:
			CTC_ERROR_RETURN(_ctc_usw_acl_map_mpls_to_mac_ipv4_key(lchip, entry_id, &(p_key->u.mpls_key)));
        	 break;
        case CTC_ACL_KEY_PBR_IPV4:
			 CTC_ERROR_RETURN(_ctc_usw_acl_map_pbr_ipv4_key(lchip, entry_id, &(p_key->u.pbr_ipv4_key)));
            break;
        case CTC_ACL_KEY_PBR_IPV6:
			CTC_ERROR_RETURN(_ctc_usw_acl_map_pbr_ipv6_key(lchip, entry_id, &(p_key->u.pbr_ipv6_key)));
           break;
        case CTC_ACL_KEY_HASH_MAC:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_mac_key(lchip, entry_id, &(p_key->u.hash_mac_key)));
            break;

        case CTC_ACL_KEY_HASH_IPV4:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_ipv4_key(lchip, entry_id, &(p_key->u.hash_ipv4_key)));
            break;

        case CTC_ACL_KEY_HASH_L2_L3:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_l2l3_key(lchip, entry_id, &(p_key->u.hash_l2_l3_key)));
            break;

        case CTC_ACL_KEY_HASH_IPV6:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_ipv6_key(lchip, entry_id, &(p_key->u.hash_ipv6_key)));
            break;

        case CTC_ACL_KEY_HASH_MPLS:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_mpls_key(lchip, entry_id, &(p_key->u.hash_mpls_key)));
            break;

        default:
            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_mac_key_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel_db)
{
    ctc_field_key_t  sel_field = {0};
    ctc_field_port_t field_port = {0};

    /* if ==> set (sel_field.data = 1) ; else ==> unset (sel_field.data = 0) */
    sel_field.data = field_sel_db->u.mac.mac_da ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MAC_DA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.mac_sa ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MAC_SA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.eth_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ETHER_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* phy_port,logical port, metadata;  share */
    /* phy_port,logical port,belong to key_port */
    if((field_sel_db->u.mac.phy_port !=0) || (field_sel_db->u.mac.logic_port != 0) || (field_sel_db->u.mac.metadata != 0))
    {
        if(field_sel_db->u.mac.phy_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            sel_field.ext_data = &field_port;
        }
        else if(field_sel_db->u.mac.logic_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            sel_field.ext_data = &field_port;
        }
        else
        {
            sel_field.type = CTC_FIELD_KEY_METADATA;
        }
        sel_field.data = 1;
    }
    else
    {
        sel_field.type = CTC_FIELD_KEY_PORT;
        sel_field.data = 0;
    }
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.cos ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_COS;
 /*  sel_field.type = CTC_FIELD_KEY_CTAG_COS;*/
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.cfi ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_CFI;
 /*  sel_field.type = CTC_FIELD_KEY_CTAG_CFI;*/
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.vlan_id ? 1:0;
    sel_field.type = CTC_FIELD_KEY_SVLAN_ID;
     /*  sel_field.type = CTC_FIELD_KEY_CVLAN_ID;*/
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mac.tag_valid ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_VALID;
     /*  sel_field.type = CTC_FIELD_KEY_CTAG_VALID;*/
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_mpls_key_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel_db)
{
    ctc_field_key_t  sel_field = {0};
    ctc_field_port_t field_port = {0};

    /* if ==> set (sel_field.data = 1) ; else ==> unset (sel_field.data = 0) */

    /* phy_port, logical_port, metadata;  share */
    /* phy_port,logical port,belong to key_port */
    if((field_sel_db->u.mpls.phy_port !=0) || (field_sel_db->u.mpls.logic_port != 0) || (field_sel_db->u.mpls.metadata != 0))
    {
        if(field_sel_db->u.mpls.phy_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            sel_field.ext_data = &field_port;
        }
        else if(field_sel_db->u.mpls.logic_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            sel_field.ext_data = &field_port;
        }
        else
        {
            sel_field.type = CTC_FIELD_KEY_METADATA;
        }
        sel_field.data = 1;
    }
    else
    {
        sel_field.type = CTC_FIELD_KEY_PORT;
        sel_field.data = 0;
    }
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /*  mpls num  */
    sel_field.data = field_sel_db->u.mpls.mpls_label_num ? 1:0;
    sel_field.type = CTC_FIELD_KEY_LABEL_NUM;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* mpls label0*/
    sel_field.data = field_sel_db->u.mpls.mpls_label0_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label0_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label0_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label0_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* mpls label1*/
    sel_field.data = field_sel_db->u.mpls.mpls_label1_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label1_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label1_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label1_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));


    /* mpls label2*/
    sel_field.data = field_sel_db->u.mpls.mpls_label2_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label2_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label2_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.mpls.mpls_label2_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_l2_l3_key_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel_db)
{
    ctc_field_key_t  sel_field  = {0};
    ctc_field_port_t field_port = {0};

    /* if ==> set (sel_field.data = 1) ; else ==> unset (sel_field.data = 0) */

    /* phy_port, logical_port, metadata;  share */
    /* phy_port,logical port,belong to key_port */
    if((field_sel_db->u.l2_l3.phy_port !=0) || (field_sel_db->u.l2_l3.logic_port != 0) || (field_sel_db->u.l2_l3.metadata != 0))
    {
        if(field_sel_db->u.l2_l3.phy_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            sel_field.ext_data = &field_port;
        }
        else if(field_sel_db->u.l2_l3.logic_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            sel_field.ext_data = &field_port;
        }
        else
        {
            sel_field.type = CTC_FIELD_KEY_METADATA;
        }
        sel_field.data = 1;
    }
    else
    {
        sel_field.type = CTC_FIELD_KEY_PORT;
        sel_field.data = 0;
    }
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mac_da ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MAC_DA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mac_sa ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MAC_SA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* svlan */
    sel_field.data = field_sel_db->u.l2_l3.stag_cos ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_COS;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.stag_cfi ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_CFI;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.stag_vlan ? 1:0;
    sel_field.type = CTC_FIELD_KEY_SVLAN_ID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.stag_valid ? 1:0;
    sel_field.type = CTC_FIELD_KEY_STAG_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* cvlan */
    sel_field.data = field_sel_db->u.l2_l3.ctag_cos ? 1:0;
    sel_field.type = CTC_FIELD_KEY_CTAG_COS;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ctag_cfi ? 1:0;
    sel_field.type = CTC_FIELD_KEY_CTAG_CFI;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ctag_vlan ? 1:0;
    sel_field.type = CTC_FIELD_KEY_CVLAN_ID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ctag_valid ? 1:0;
    sel_field.type = CTC_FIELD_KEY_CTAG_VALID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ip_da ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_DA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ip_sa ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_SA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.dscp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_DSCP;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ecn ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_ECN;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_TTL;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.eth_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ETHER_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.l3_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L3_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.l4_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /*  mpls num  */
    sel_field.data = field_sel_db->u.l2_l3.mpls_label_num ? 1:0;
    sel_field.type = CTC_FIELD_KEY_LABEL_NUM;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* mpls label0*/
    sel_field.data = field_sel_db->u.l2_l3.mpls_label0_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label0_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label0_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label0_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL0;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* mpls label1*/
    sel_field.data = field_sel_db->u.l2_l3.mpls_label1_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label1_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label1_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label1_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL1;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));


    /* mpls label2*/
    sel_field.data = field_sel_db->u.l2_l3.mpls_label2_label ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label2_exp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_EXP2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label2_s ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.mpls_label2_ttl ? 1:0;
    sel_field.type = CTC_FIELD_KEY_MPLS_TTL2;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.l4_src_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.l4_dst_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_DST_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.icmp_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.icmp_code ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_CODE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.vni ? 1:0;
    sel_field.type = CTC_FIELD_KEY_VN_ID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.l2_l3.gre_key ? 1:0;
    sel_field.type = CTC_FIELD_KEY_GRE_KEY;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_ipv4_key_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel_db)
{
    ctc_field_key_t  sel_field  = {0};
    ctc_field_port_t field_port = {0};

    /* if ==> set (sel_field.data = 1) ; else ==> unset (sel_field.data = 0) */
    sel_field.data = field_sel_db->u.ipv4.ip_da ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_DA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.ip_sa ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_SA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.l4_src_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.l4_dst_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_DST_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* phy_port, logical_port, metadata;  share */
    /* phy_port,logical port,belong to key_port */
    if((field_sel_db->u.ipv4.phy_port !=0) || (field_sel_db->u.ipv4.logic_port != 0) || (field_sel_db->u.ipv4.metadata != 0))
    {
        if(field_sel_db->u.ipv4.phy_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            sel_field.ext_data = &field_port;
        }
        else if(field_sel_db->u.ipv4.logic_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            sel_field.ext_data = &field_port;
        }
        else
        {
            sel_field.type = CTC_FIELD_KEY_METADATA;
        }
        sel_field.data = 1;
    }
    else
    {
        sel_field.type = CTC_FIELD_KEY_PORT;
        sel_field.data = 0;
    }
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.dscp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_DSCP;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.ecn ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_ECN;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.l4_protocol ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_TYPE;     /*not support ip protocol, use l4type instead*/
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.icmp_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.icmp_code ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_CODE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.vni ? 1:0;
    sel_field.type = CTC_FIELD_KEY_VN_ID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv4.gre_key ? 1:0;
    sel_field.type = CTC_FIELD_KEY_GRE_KEY;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_ipv6_key_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel_db)
{
    ctc_field_key_t sel_field = {0};
    ctc_field_port_t field_port = {0};

    /* if ==> set (sel_field.data = 1) ; else ==> unset (sel_field.data = 0) */
    sel_field.data = field_sel_db->u.ipv6.ip_da ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IPV6_DA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.ip_sa ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IPV6_SA;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.l4_src_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.l4_dst_port ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_DST_PORT;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    /* phy_port, logical_port, metadata;  share */
    /* phy_port,logical port,belong to key_port */
    if((field_sel_db->u.ipv6.phy_port !=0) || (field_sel_db->u.ipv6.logic_port != 0) || (field_sel_db->u.ipv6.metadata != 0))
    {
        if(field_sel_db->u.ipv6.phy_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            sel_field.ext_data = &field_port;
        }
        else if(field_sel_db->u.ipv6.logic_port != 0)
        {
            sel_field.type = CTC_FIELD_KEY_PORT;
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            sel_field.ext_data = &field_port;
        }
        else
        {
            sel_field.type = CTC_FIELD_KEY_METADATA;
        }
        sel_field.data = 1;
    }
    else
    {
        sel_field.type = CTC_FIELD_KEY_PORT;
        sel_field.data = 0;
    }
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.dscp ? 1:0;
    sel_field.type = CTC_FIELD_KEY_IP_DSCP;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.l4_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_L4_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.icmp_type ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_TYPE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.icmp_code ? 1:0;
    sel_field.type = CTC_FIELD_KEY_ICMP_CODE;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.vni ? 1:0;
    sel_field.type = CTC_FIELD_KEY_VN_ID;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    sel_field.data = field_sel_db->u.ipv6.gre_key ? 1:0;
    sel_field.type = CTC_FIELD_KEY_GRE_KEY;
    CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, field_sel_db->hash_key_type, field_sel_db->field_sel_id, &sel_field));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_acl_map_hash_sel_by_struct(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel)
{
    CTC_PTR_VALID_CHECK(field_sel);
    switch(field_sel->hash_key_type)
    {
        case CTC_ACL_KEY_HASH_MAC:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_mac_key_sel(lchip,field_sel));
            break;
        case CTC_ACL_KEY_HASH_IPV4:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_ipv4_key_sel(lchip,field_sel));
            break;
        case CTC_ACL_KEY_HASH_L2_L3:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_l2_l3_key_sel(lchip,field_sel));
            break;
        case CTC_ACL_KEY_HASH_MPLS:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_mpls_key_sel(lchip,field_sel));
            break;
        case CTC_ACL_KEY_HASH_IPV6:
            CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_ipv6_key_sel(lchip,field_sel));
            break;
        default:
            break;
    }
    return CTC_E_NONE;
}

int32
ctc_usw_acl_init(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    ctc_acl_global_cfg_t acl_cfg;

    LCHIP_CHECK(lchip);
    if (NULL == acl_global_cfg)
    {
        sal_memset(&acl_cfg, 0, sizeof(ctc_acl_global_cfg_t));
        acl_global_cfg = &acl_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_init(lchip, acl_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_acl_deinit(uint8 lchip)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    uint8 all_lchip         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(group_info);

    if(CTC_ACL_GROUP_TYPE_PORT_BITMAP == group_info->type)
    {
        CTC_SET_API_LCHIP(lchip, group_info->lchip);
        lchip_start = lchip;
        lchip_end   = lchip_start + 1;
        all_lchip = 0;
    }
    else if((CTC_ACL_GROUP_TYPE_PORT == group_info->type) && (!CTC_IS_LINKAGG_PORT(group_info->un.gport)))
    {
        SYS_MAP_GPORT_TO_LCHIP(group_info->un.gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    else
    {
        all_lchip = 1;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
                                  ret,
                                  sys_usw_acl_create_group(lchip, group_id, group_info));
    }

    /*rollback if error exist*/
    if(all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
        {
            sys_usw_acl_destroy_group(lchip, group_id);
        }
    }

    return ret;
}

int32
ctc_usw_acl_destroy_group(uint8 lchip, uint32 group_id)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_acl_destroy_group(lchip, group_id));
    }

    return ret;
}

int32
ctc_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_install_group(lchip, group_id, group_info));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_acl_uninstall_group(lchip, group_id);
    }

    return ret;
}

int32
ctc_usw_acl_uninstall_group(uint8 lchip, uint32 group_id)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_acl_uninstall_group(lchip, group_id));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_acl_install_group(lchip, group_id, NULL);
    }

    return ret;
}

int32
ctc_usw_acl_get_group_info(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        ret = sys_usw_acl_get_group_info(lchip, group_id, group_info);
        if(CTC_E_NONE == ret)   /*get group info success*/
        {
            break;
        }
    }

    return ret;
}

int32
ctc_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* acl_entry)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;
    ctc_acl_group_info_t group_info;
    uint8   group_exist_one = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

/*compatible with GG/GB:
  1. GG ipv4 single not support these field :ip frag \VLAN_NUM  and PKT_LEN_RANGE ;
  2. GG ipv4 double not support these field:UDF ;
*/

/*compatible with GG/GB: ip frag and tcp flag */

    if(!acl_entry->mode)
    {
        switch(acl_entry->key.type)
        {
            /*Tcam key:(key type) if changes,do in case;not changes,do in defalt.*/
            case CTC_ACL_KEY_IPV4:                /**< [HB.GB.GG.D2] ACL IPv4 key type */
                if(CTC_ACL_KEY_SIZE_DOUBLE == acl_entry->key.u.ipv4_key.key_size)
                {
                    acl_entry->key_type = CTC_ACL_KEY_MAC_IPV4;
                }
                else
                {
                    acl_entry->key_type = CTC_ACL_KEY_IPV4; /*ip frag and tcp flag must use IPV4_EXT */
                }
                break;
            case CTC_ACL_KEY_MPLS:/**< [HB.GB.GG] ACL MPLS key type;In USW,replaced by CTC_ACL_KEY_MAC_IPV4*/
                acl_entry->key_type = CTC_ACL_KEY_MAC_IPV4;
                break;
            case CTC_ACL_KEY_IPV6:               /**< [HB.GB.GG.D2] Mode 0: ACL Mac+IPv6 key type;
                                                            Mode 1: ACL Ipv6 key type */
                acl_entry->key_type = CTC_ACL_KEY_MAC_IPV6;
                break;
            case CTC_ACL_KEY_PBR_IPV4:           /**< [HB.GB.GG] ACL PBR IPv4 key type ;In USW,replaced by CTC_ACL_KEY_IPV4 */
                acl_entry->key_type = CTC_ACL_KEY_MAC_IPV4;
                break;
            case CTC_ACL_KEY_PBR_IPV6:           /**< [HB.GB.GG] ACL PBR IPv6 key type ;In USW,replaced by CTC_ACL_KEY_IPV6 */
                acl_entry->key_type = CTC_ACL_KEY_MAC_IPV6;
                break;

            /*Hash key:(key type and hash_field_sel_id;all no change) map key type and hash_field_sel_id.*/
            case CTC_ACL_KEY_HASH_MAC:
                acl_entry->key_type = CTC_ACL_KEY_HASH_MAC;
                acl_entry->hash_field_sel_id = acl_entry->key.u.hash_mac_key.field_sel_id;
                break;
            case CTC_ACL_KEY_HASH_IPV4:
                acl_entry->key_type = CTC_ACL_KEY_HASH_IPV4;
                acl_entry->hash_field_sel_id = acl_entry->key.u.hash_mac_key.field_sel_id;
                break;
            case CTC_ACL_KEY_HASH_L2_L3:
                acl_entry->key_type = CTC_ACL_KEY_HASH_L2_L3;
                acl_entry->hash_field_sel_id = acl_entry->key.u.hash_mac_key.field_sel_id;
                break;
            case CTC_ACL_KEY_HASH_MPLS:
                acl_entry->key_type = CTC_ACL_KEY_HASH_MPLS;
                acl_entry->hash_field_sel_id = acl_entry->key.u.hash_mac_key.field_sel_id;
                break;
            case CTC_ACL_KEY_HASH_IPV6:
                acl_entry->key_type = CTC_ACL_KEY_HASH_IPV6;
                acl_entry->hash_field_sel_id = acl_entry->key.u.hash_mac_key.field_sel_id;
                break;

            default :
                acl_entry->key_type = acl_entry->key.type;
                break;
        }
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        ret = sys_usw_acl_get_group_info(lchip, group_id, &group_info);
        /*Muliti-chip CTC mode,port bmp based or gport based group only exist in one chip, for other not exist chip should
        continue, but if group not exist in all chip, should return error*/
        if((!g_ctcs_api_en) && (g_lchip_num > 1) &&( ret == CTC_E_NOT_EXIST))
        {
            ret = group_exist_one ? CTC_E_NONE : ret;
            continue;
        }

        if(!ret)
        {
            group_exist_one = 1;
        }
        ret = sys_usw_acl_add_entry(lchip, group_id, acl_entry);
        if(0 == acl_entry->mode)
        {
           ret = !ret?_ctc_usw_acl_map_key_by_struct(lchip, acl_entry->entry_id, &(acl_entry->key)):ret;
           ret = !ret ?_ctc_usw_acl_map_action_by_struct(lchip, acl_entry->entry_id, &(acl_entry->action)):ret;
        }
		if(ret != CTC_E_NONE && ret != CTC_E_EXIST && ret != CTC_E_HASH_CONFLICT)
		{
			sys_usw_acl_remove_entry(lchip, acl_entry->entry_id);
			break;
		}
    }
    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_acl_remove_entry(lchip, acl_entry->entry_id);
    }

    return ret;
}

int32
ctc_usw_acl_remove_entry(uint8 lchip, uint32 entry_id)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;


    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_remove_entry(lchip, entry_id));
    }

    return ret;
}

int32
ctc_usw_acl_install_entry(uint8 lchip, uint32 entry_id)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
   	uint8 count  = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN2(CTC_E_NOT_EXIST, CTC_E_NOT_READY,
                                 ret,
                                 sys_usw_acl_install_entry(lchip, entry_id));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_acl_uninstall_entry(lchip, entry_id);
    }

    return ret;
}

int32
ctc_usw_acl_uninstall_entry(uint8 lchip, uint32 entry_id)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_uninstall_entry(lchip, entry_id));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_acl_install_entry(lchip, entry_id);
    }

    return ret;
}

int32
ctc_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_remove_all_entry(lchip, group_id));
    }

    return ret;
}

int32
ctc_usw_acl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority)
{
    uint8 count             = 0;
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_acl_set_entry_priority(lchip, entry_id, priority));
    }

    return ret;
}

int32
ctc_usw_acl_get_multi_entry(uint8 lchip, ctc_acl_query_t* query)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;
    int32 ret               = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        ret = sys_usw_acl_get_multi_entry(lchip, query);
        if(CTC_E_NONE == ret)    /*get entries successfully*/
        {
            break;
        }
    }

    return ret;
}

int32
ctc_usw_acl_update_action(uint8 lchip, uint32 entry_id, ctc_acl_action_t* action)
{
    uint8 count         = 0;
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;


    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
	    CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  _ctc_usw_acl_map_action_by_struct(lchip,entry_id,action));

    }

    return ret;
}

int32
ctc_usw_acl_copy_entry(uint8 lchip, ctc_acl_copy_entry_t* copy_entry)
{
    uint8 count         = 0;
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;


    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_acl_copy_entry(lchip, copy_entry));
    }

    return ret;
}

int32
ctc_usw_acl_set_hash_field_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(_ctc_usw_acl_map_hash_sel_by_struct(lchip, field_sel));
    }

    return ret;
}


int32
ctc_usw_acl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;
    uint8 count         = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_add_key_field(lchip, entry_id, key_field));
    }

    return ret;
}

int32
ctc_usw_acl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field)
{
    uint8 count         = 0;
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_remove_key_field(lchip, entry_id, key_field));
    }

    return ret;
}

int32
ctc_usw_acl_add_key_field_list(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt)
{
    return _ctc_usw_acl_set_field_list(lchip, entry_id, (void*)p_field_list, p_field_cnt, (_ctc_usw_acl_set_field_t)sys_usw_acl_add_key_field, 1, 1);
}

int32
ctc_usw_acl_remove_key_field_list(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt)
{
    return _ctc_usw_acl_set_field_list(lchip, entry_id, (void*)p_field_list, p_field_cnt, (_ctc_usw_acl_set_field_t)sys_usw_acl_remove_key_field, 1, 0);
}

int32
ctc_usw_acl_add_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 count         = 0;
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_add_action_field(lchip, entry_id, action_field));
    }

    return ret;
}

int32
ctc_usw_acl_add_action_field_list(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt)
{
    return _ctc_usw_acl_set_field_list(lchip, entry_id, (void*)p_field_list, p_field_cnt, (_ctc_usw_acl_set_field_t)sys_usw_acl_add_action_field, 0, 1);
}

int32
ctc_usw_acl_remove_action_field_list(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt)
{
    return _ctc_usw_acl_set_field_list(lchip, entry_id, (void*)p_field_list, p_field_cnt, (_ctc_usw_acl_set_field_t)sys_usw_acl_remove_action_field, 0, 0);
}

int32
ctc_usw_acl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 count         = 0;
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_acl_remove_action_field(lchip, entry_id, action_field));
    }

    return ret;
}

int32
ctc_usw_acl_set_field_to_hash_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id, ctc_field_key_t* sel_field)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_set_field_to_hash_field_sel(lchip, key_type, field_sel_id, sel_field));
    }

    return ret;
}

int32
ctc_usw_acl_add_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_add_cid_pair(lchip, cid_pair));
    }

    return ret;
}

int32
ctc_usw_acl_remove_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_remove_cid_pair(lchip, cid_pair));
    }

    return ret;
}

int32
ctc_usw_acl_add_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_add_udf_entry(lchip, udf_entry));
    }
	 return ret;
}

int32
ctc_usw_acl_remove_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_remove_udf_entry(lchip, udf_entry));
    }
	 return ret;
}
int32
ctc_usw_acl_get_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_get_udf_entry(lchip, udf_entry));
    }
	 return ret;
}
int32
ctc_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_add_udf_entry_key_field(lchip, udf_id,key_field));
    }
	 return ret;
}
int32
ctc_usw_acl_remove_udf_entry_key_field(uint8 lchip, uint32 udf_id, ctc_field_key_t* key_field)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_remove_udf_entry_key_field(lchip, udf_id,key_field));
    }
	 return ret;
}

int32
ctc_usw_acl_set_league_mode(uint8 lchip, ctc_acl_league_t* league)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_set_league_mode(lchip, league));
    }
    return ret;
}

int32
ctc_usw_acl_get_league_mode(uint8 lchip, ctc_acl_league_t* league)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_get_league_mode(lchip, league));
    }
    return ret;
}

int32
ctc_usw_acl_reorder_entry(uint8 lchip, ctc_acl_reorder_t* reorder)
{
    uint8 lchip_start   = 0;
    uint8 lchip_end     = 0;
    int32 ret           = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_ACL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_acl_reorder_entry(lchip, reorder));
    }
    return ret;
}


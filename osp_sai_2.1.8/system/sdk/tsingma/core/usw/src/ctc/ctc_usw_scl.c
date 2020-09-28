/**
 @file ctc_usw_scl.c

 @date 2013-02-21

 @version v2.0

 The file contains scl APIs
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
#include "ctc_field.h"
#include "ctc_parser.h"
#include "ctc_acl.h"
#include "ctc_common.h"
#include "ctc_usw_scl.h"

#include "sys_usw_scl_api.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_common.h"




STATIC uint32
_ctc_usw_scl_sum(uint8 lchip, uint32 a, ...)
{
    va_list list;
    uint32  v   = a;
    uint32  sum = 0;
    va_start(list, a);
    while (v != -1)
    {
        sum += v;
        v    = va_arg(list, int32);
    }
    va_end(list);

    return sum;
}

#define CTC_SCL_SUM(...)    _ctc_usw_scl_sum(__VA_ARGS__, -1)


STATIC int32
_ctc_usw_scl_check_mac_key(uint8 lchip, uint32 flag)
{
    uint32 obsolete_flag = 0;
    uint32 sflag         = 0;
    uint32 cflag         = 0;

    sflag = CTC_SCL_SUM(CTC_SCL_TCAM_MAC_KEY_FLAG_SVLAN,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_COS,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_CFI,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_VALID);

    cflag = CTC_SCL_SUM(CTC_SCL_TCAM_MAC_KEY_FLAG_CVLAN,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_COS,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_CFI,
                    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_VALID);

    if ((flag & sflag) && (flag & cflag))
    {
        return CTC_E_INVALID_PARAM;
    }

    obsolete_flag = CTC_SCL_SUM(lchip, CTC_SCL_TCAM_MAC_KEY_FLAG_L3_TYPE);

    if (flag & obsolete_flag)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_check_ipv4_key(uint8 lchip, uint32 flag, uint32 sub_flag, uint8 key_size)
{
    uint32 obsolete_flag = 0;

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_DSCP)
        && CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_PRECEDENCE))
    {
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI)) > 1)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI)) > 1)
    {
        return CTC_E_INVALID_CONFIG;
    }

    obsolete_flag = CTC_SCL_SUM(lchip, CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_TYPE);

    if (CTC_SCL_KEY_SIZE_SINGLE == key_size)
    {
        obsolete_flag = CTC_SCL_SUM(CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_DA,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_SA,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_CVLAN,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_COS,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_SVLAN,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_COS,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_CFI,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_CFI,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID,
                                CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_TYPE);
    }

    if (flag & obsolete_flag)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_check_ipv6_key(uint8 lchip, uint32 flag, uint32 sub_flag)
{
    uint32 obsolete_flag = 0;

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_DSCP)
        && CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_PRECEDENCE))
    {
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_GRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_NVGRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_VNI)) > 1)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_GRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_NVGRE_KEY)
         + CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_VNI)) > 1)
    {
        return CTC_E_INVALID_CONFIG;
    }

    obsolete_flag = CTC_SCL_SUM(lchip, CTC_SCL_TCAM_IPV6_KEY_FLAG_ETH_TYPE,
                            CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);

    if (flag & obsolete_flag)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_scl_get_group_type(uint8 lchip, uint32 entry_id, uint8* group_type)
{
    sys_usw_scl_get_group_type(lchip, entry_id, group_type);
    return CTC_E_NONE;
}


STATIC int32
_ctc_usw_scl_map_tcam_mac_key(uint8 lchip, ctc_scl_tcam_mac_key_t* p_ctc_key, uint32 entry_id)
{
    uint8 group_type = 0;
    uint32 flag = 0;
    ctc_field_key_t field_key;



    CTC_ERROR_RETURN(_ctc_usw_scl_check_mac_key(lchip, p_ctc_key->flag));

    flag = p_ctc_key->flag;

    _ctc_usw_scl_get_group_type(lchip, entry_id, &group_type);

    if ((CTC_SCL_GROUP_TYPE_NONE == group_type) && (CTC_FIELD_PORT_TYPE_NONE != p_ctc_key->port_data.type))
    {
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &p_ctc_key->port_data;
        field_key.ext_mask = &p_ctc_key->port_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* mac da */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_DA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_ctc_key->mac_da;
        field_key.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* mac sa */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_SA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_ctc_key->mac_sa;
        field_key.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* cvlan */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CVLAN))
    {
        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = p_ctc_key->cvlan;
        field_key.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* ctag cos */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_COS;
        field_key.data = p_ctc_key->ctag_cos;
        field_key.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* ctag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_CFI;
        field_key.data = p_ctc_key->ctag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_VALID))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_VALID;
        field_key.data = p_ctc_key->ctag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* svlan */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_SVLAN))
    {
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = p_ctc_key->svlan;
        field_key.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* stag cos */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_STAG_COS;
        field_key.data = p_ctc_key->stag_cos;
        field_key.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* stag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_STAG_CFI;
        field_key.data = p_ctc_key->stag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_VALID))
    {
        field_key.type = CTC_FIELD_KEY_STAG_VALID;
        field_key.data = p_ctc_key->stag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* l2 type */ /* l2 type used as vlan num
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_L2_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L2_TYPE;
        field_key.data = p_ctc_key->l2_type;
        field_key.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }*/

    /* eth type */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_MAC_KEY_FLAG_ETH_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_ETHER_TYPE;
        field_key.data = p_ctc_key->eth_type;
        field_key.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_tcam_ipv4_key(uint8 lchip, ctc_scl_tcam_ipv4_key_t* p_ctc_key, uint32 entry_id)
{
    uint8 group_type     = 0;
    uint32 flag          = 0;
    uint32 sub_flag      = 0;
    uint32 l3_type_match = 0;
    uint32 l3_type      = 0;
    uint32 l3_type_mask = 0;
    ctc_field_key_t field_key;



    CTC_ERROR_RETURN(_ctc_usw_scl_check_ipv4_key(lchip, p_ctc_key->flag, p_ctc_key->sub_flag, p_ctc_key->key_size));

    flag      = p_ctc_key->flag;
    sub_flag  = p_ctc_key->sub_flag;

    _ctc_usw_scl_get_group_type(lchip, entry_id, &group_type);

    if ((CTC_SCL_GROUP_TYPE_NONE == group_type) && (CTC_FIELD_PORT_TYPE_NONE != p_ctc_key->port_data.type))
    {
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &p_ctc_key->port_data;
        field_key.ext_mask = &p_ctc_key->port_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_DA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = &p_ctc_key->mac_da;
        field_key.ext_mask = &p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_SA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_ctc_key->mac_sa;
        field_key.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CVLAN))
    {
        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = p_ctc_key->cvlan;
        field_key.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_COS;
        field_key.data = p_ctc_key->ctag_cos;
        field_key.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_CFI;
        field_key.data = p_ctc_key->ctag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_SVLAN))
    {
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = p_ctc_key->svlan;
        field_key.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_STAG_COS;
        field_key.data = p_ctc_key->stag_cos;
        field_key.mask = (p_ctc_key->stag_cos_mask) & 0x7;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_STAG_CFI;
        field_key.data = p_ctc_key->stag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    /* l2 type used as vlan num
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L2_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L2_TYPE;
        field_key.data = p_ctc_key->l2_type;
        field_key.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID))
    {
        field_key.type = CTC_FIELD_KEY_STAG_VALID;
        field_key.data = p_ctc_key->stag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_VALID;
        field_key.data = p_ctc_key->ctag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L3_TYPE;
        field_key.data = p_ctc_key->l3_type;
        field_key.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
        l3_type = p_ctc_key->l3_type;
        l3_type_mask = (p_ctc_key->l3_type_mask) & 0xF;
    }
    else /* if l3_type not set, give it a default value.*/
    {
        field_key.type = CTC_FIELD_KEY_L3_TYPE;
        field_key.data = CTC_PARSER_L3_TYPE_IPV4;
        field_key.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
        l3_type = CTC_PARSER_L3_TYPE_IPV4;
        l3_type_mask = 0xF;
    }

    l3_type_match = l3_type & l3_type_mask;

    /* l3_type is not supposed to match more than once. --> Fullfill this will make things easy.
     * But still, l3_type_mask can be any value, if user wants to only match it. --> Doing this make things complicate.
     * If further l3_info requires to be matched, only one l3_info is promised to work.
     */
    if (0xF == l3_type_mask)
    {
        if (l3_type_match == CTC_PARSER_L3_TYPE_IPV4)
        {
            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_DA))
            {
                field_key.type = CTC_FIELD_KEY_IP_DA;
                field_key.data = p_ctc_key->ip_da;
                field_key.mask = p_ctc_key->ip_da_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA))
            {
                field_key.type = CTC_FIELD_KEY_IP_SA;
                field_key.data = p_ctc_key->ip_sa;
                field_key.mask = p_ctc_key->ip_sa_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_FRAG))
            {
                field_key.type = CTC_FIELD_KEY_IP_FRAG;
                field_key.data = p_ctc_key->ip_frag;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_OPTION))
            {;
                field_key.type = CTC_FIELD_KEY_IP_OPTIONS;
                field_key.data = p_ctc_key->ip_option;
                field_key.mask = 1;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_ECN))
            {
                field_key.type = CTC_FIELD_KEY_IP_ECN;
                field_key.data = p_ctc_key->ecn;
                field_key.mask = p_ctc_key->ecn_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_DSCP))
            {
                field_key.type = CTC_FIELD_KEY_IP_DSCP;
                field_key.data = p_ctc_key->dscp;
                field_key.mask = p_ctc_key->dscp_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_PRECEDENCE))
            {
                CTC_MAX_VALUE_CHECK(p_ctc_key->dscp, 7);
                field_key.type = CTC_FIELD_KEY_IP_DSCP;
                field_key.data = (p_ctc_key->dscp) << 3;
                field_key.mask = (p_ctc_key->dscp_mask & 0x7) << 3;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_ARP)
        {
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_OP_CODE))
            {
                field_key.type = CTC_FIELD_KEY_ARP_OP_CODE;
                field_key.data = p_ctc_key->arp_op_code;
                field_key.mask = p_ctc_key->arp_op_code_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP))
            {
                field_key.type = CTC_FIELD_KEY_ARP_SENDER_IP;
                field_key.data = p_ctc_key->sender_ip;
                field_key.mask = p_ctc_key->sender_ip_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP))
            {
                field_key.type = CTC_FIELD_KEY_ARP_TARGET_IP;
                field_key.data = p_ctc_key->target_ip;
                field_key.mask = p_ctc_key->target_ip_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;
                field_key.data = p_ctc_key->arp_protocol_type;
                field_key.mask = p_ctc_key->arp_protocol_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if ((l3_type_match == CTC_PARSER_L3_TYPE_MPLS) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_MPLS_MCAST))
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_FCOE)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_TRILL)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_ETHER_OAM)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_SLOW_PROTO)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_CMAC)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_PTP)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_NONE)
        {
            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_ETH_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ETHER_TYPE;
                field_key.data = p_ctc_key->eth_type;
                field_key.mask = p_ctc_key->eth_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if ((l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_IP) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_IPV6) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_DEFINE0) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_DEFINE1))
        {
            /* never do anything */
        }
    }

    /* layer 4 information */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL))
    {
        switch (p_ctc_key->l4_protocol & p_ctc_key->l4_protocol_mask)
        {
        case SYS_L4_PROTOCOL_IPV4_ICMP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_ICMP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            /* l4_src_port for flex-l4 (including icmp), it's type|code */

            /* icmp type */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_TYPE;
                field_key.data = p_ctc_key->icmp_type;
                field_key.mask = p_ctc_key->icmp_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            /* icmp code */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_CODE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_CODE;
                field_key.data = p_ctc_key->icmp_code;
                field_key.mask = p_ctc_key->icmp_code_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            break;
        case SYS_L4_PROTOCOL_IPV4_IGMP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_IGMP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            /* l4_src_port for flex-l4 (including igmp), it's type|..*/
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_IGMP_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_IGMP_TYPE;
                field_key.data = p_ctc_key->igmp_type;
                field_key.mask = p_ctc_key->igmp_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_TCP:
        case SYS_L4_PROTOCOL_UDP:
            if (SYS_L4_PROTOCOL_TCP == p_ctc_key->l4_protocol)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_TCP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_UDP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI))
            {
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni, 0xFFFFFF);
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni_mask, 0xFFFFFF);
                /*Add vnid must add l4 user type first*/
                field_key.type = CTC_FIELD_KEY_L4_USER_TYPE;
                field_key.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

                field_key.type = CTC_FIELD_KEY_VN_ID;
                field_key.data = p_ctc_key->vni;
                field_key.mask = p_ctc_key->vni_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_GRE:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_GRE;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_GRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_NVGRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_SCTP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_SCTP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        default:
            return CTC_E_NOT_SUPPORT;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_tcam_ipv4_key_single(uint8 lchip, ctc_scl_tcam_ipv4_key_t* p_ctc_key, uint32 entry_id)
{
    uint8 group_type     = 0;
    uint32 flag          = 0;
    uint32 sub_flag      = 0;
    uint32 l3_type_match = 0;
    uint32 l3_type      = 0;
    uint32 l3_type_mask = 0;
    ctc_field_key_t field_key;



    CTC_ERROR_RETURN(_ctc_usw_scl_check_ipv4_key(lchip, p_ctc_key->flag, p_ctc_key->sub_flag, p_ctc_key->key_size));

    flag      = p_ctc_key->flag;
    sub_flag  = p_ctc_key->sub_flag;

    _ctc_usw_scl_get_group_type(lchip, entry_id, &group_type);

    if ((CTC_SCL_GROUP_TYPE_NONE == group_type) && (CTC_FIELD_PORT_TYPE_NONE != p_ctc_key->port_data.type))
    {
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &p_ctc_key->port_data;
        field_key.ext_mask = &p_ctc_key->port_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L3_TYPE;
        field_key.data = p_ctc_key->l3_type;
        field_key.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
        l3_type = p_ctc_key->l3_type;
        l3_type_mask = (p_ctc_key->l3_type_mask) & 0xF;
    }
    else /* if l3_type not set, give it a default value.*/
    {
        field_key.type = CTC_FIELD_KEY_L3_TYPE;
        field_key.data = CTC_PARSER_L3_TYPE_IPV4;
        field_key.mask = 0xF;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
        l3_type = CTC_PARSER_L3_TYPE_IPV4;
        l3_type_mask = 0xF;
    }

    l3_type_match = l3_type & l3_type_mask;

    /* l3_type is not supposed to match more than once. --> Fullfill this will make things easy.
     * But still, l3_type_mask can be any value, if user wants to only match it. --> Doing this make things complicate.
     * If further l3_info requires to be matched, only one l3_info is promised to work.
     */
    if (0xF == l3_type_mask)
    {
        if (l3_type_match == CTC_PARSER_L3_TYPE_IPV4)
        {
            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_DA))
            {
                field_key.type = CTC_FIELD_KEY_IP_DA;
                field_key.data = p_ctc_key->ip_da;
                field_key.mask = p_ctc_key->ip_da_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA))
            {
                field_key.type = CTC_FIELD_KEY_IP_SA;
                field_key.data = p_ctc_key->ip_sa;
                field_key.mask = p_ctc_key->ip_sa_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_OPTION))
            {
                field_key.type = CTC_FIELD_KEY_IP_OPTIONS;
                field_key.data = p_ctc_key->ip_option;
                field_key.mask = 1;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_FRAG))
            {
                field_key.type = CTC_FIELD_KEY_IP_FRAG;
                field_key.data = p_ctc_key->ip_frag;
                field_key.mask = 3;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_DSCP))
            {
                field_key.type = CTC_FIELD_KEY_IP_DSCP;
                field_key.data = p_ctc_key->dscp;
                field_key.mask = p_ctc_key->dscp_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_PRECEDENCE))
            {
                CTC_MAX_VALUE_CHECK(p_ctc_key->dscp, 7);
                field_key.type = CTC_FIELD_KEY_IP_DSCP;
                field_key.data = (p_ctc_key->dscp) << 3;
                field_key.mask = (p_ctc_key->dscp_mask & 0x7) << 3;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_ARP)
        {
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_OP_CODE))
            {
                field_key.type = CTC_FIELD_KEY_ARP_OP_CODE;
                field_key.data = p_ctc_key->arp_op_code;
                field_key.mask = p_ctc_key->arp_op_code_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP))
            {
                field_key.type = CTC_FIELD_KEY_ARP_SENDER_IP;
                field_key.data = p_ctc_key->sender_ip;
                field_key.mask = p_ctc_key->sender_ip_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP))
            {
                field_key.type = CTC_FIELD_KEY_ARP_TARGET_IP;
                field_key.data = p_ctc_key->target_ip;
                field_key.mask = p_ctc_key->target_ip_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;
                field_key.data = p_ctc_key->arp_protocol_type;
                field_key.mask = p_ctc_key->arp_protocol_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if ((l3_type_match == CTC_PARSER_L3_TYPE_MPLS) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_MPLS_MCAST))
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_FCOE)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_TRILL)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_ETHER_OAM)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_SLOW_PROTO)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_CMAC)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_PTP)
        {
        }
        else if (l3_type_match == CTC_PARSER_L3_TYPE_NONE)
        {
            if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_ETH_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ETHER_TYPE;
                field_key.data = p_ctc_key->eth_type;
                field_key.mask = p_ctc_key->eth_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
        }
        else if ((l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_IP) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_IPV6) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_DEFINE0) ||
                 (l3_type_match == CTC_PARSER_L3_TYPE_RSV_USER_DEFINE1))
        {
            /* never do anything */
        }
    }

    /* layer 4 information */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL))
    {
        switch (p_ctc_key->l4_protocol & p_ctc_key->l4_protocol_mask)
        {
        case SYS_L4_PROTOCOL_IPV4_ICMP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_ICMP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            /* l4_src_port for flex-l4 (including icmp), it's type|code */

            /* icmp type */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_TYPE;
                field_key.data = p_ctc_key->icmp_type;
                field_key.mask = p_ctc_key->icmp_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            /* icmp code */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_CODE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_CODE;
                field_key.data = p_ctc_key->icmp_code;
                field_key.mask = p_ctc_key->icmp_code_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            break;
        case SYS_L4_PROTOCOL_IPV4_IGMP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_IGMP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            /* l4_src_port for flex-l4 (including igmp), it's type|..*/
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_IGMP_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_IGMP_TYPE;
                field_key.data = p_ctc_key->igmp_type;
                field_key.mask = p_ctc_key->igmp_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_TCP:
        case SYS_L4_PROTOCOL_UDP:
            if (SYS_L4_PROTOCOL_TCP == p_ctc_key->l4_protocol)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_TCP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_UDP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI))
            {
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni, 0xFFFFFF);
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni_mask, 0xFFFFFF);
                /*Add vnid must add l4 user type first*/
                field_key.type = CTC_FIELD_KEY_L4_USER_TYPE;
                field_key.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

                field_key.type = CTC_FIELD_KEY_VN_ID;
                field_key.data = p_ctc_key->vni;
                field_key.mask = p_ctc_key->vni_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_GRE:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_GRE;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_GRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_NVGRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_SCTP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_SCTP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        default:
            return CTC_E_NOT_SUPPORT;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_tcam_ipv6_key(uint8 lchip, ctc_scl_tcam_ipv6_key_t* p_ctc_key, uint32 entry_id)
{
    uint8 group_type = 0;
    uint32 flag     = 0;
    uint32 sub_flag = 0;
    ctc_field_key_t field_key;



    CTC_ERROR_RETURN(_ctc_usw_scl_check_ipv6_key(lchip, p_ctc_key->flag, p_ctc_key->sub_flag));

    flag                = p_ctc_key->flag;
    sub_flag            = p_ctc_key->sub_flag;

    _ctc_usw_scl_get_group_type(lchip, entry_id, &group_type);

    if ((CTC_SCL_GROUP_TYPE_NONE == group_type) && (CTC_FIELD_PORT_TYPE_NONE != p_ctc_key->port_data.type))
    {
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &p_ctc_key->port_data;
        field_key.ext_mask = &p_ctc_key->port_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_DA))
    {
        field_key.type = CTC_FIELD_KEY_IPV6_DA;
        field_key.ext_data = p_ctc_key->ip_da;
        field_key.ext_mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_SA))
    {
        field_key.type = CTC_FIELD_KEY_IPV6_SA;
        field_key.ext_data = p_ctc_key->ip_sa;
        field_key.ext_mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* ip fragment */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_FRAG))
    {
        field_key.type = CTC_FIELD_KEY_IP_FRAG;
        field_key.data = p_ctc_key->ip_frag;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_DSCP))
    {
        field_key.type = CTC_FIELD_KEY_IP_DSCP;
        field_key.data = p_ctc_key->dscp;
        field_key.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    else if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_PRECEDENCE))
    {
        CTC_MAX_VALUE_CHECK(p_ctc_key->dscp, 7);
        field_key.type = CTC_FIELD_KEY_IP_DSCP;
        field_key.data = (p_ctc_key->dscp) << 3;
        field_key.mask = (p_ctc_key->dscp_mask & 0x7) << 3;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_OPTION))
    {
        field_key.type = CTC_FIELD_KEY_IP_OPTIONS;
        field_key.data = p_ctc_key->ip_option;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_FLOW_LABEL))
    {
        field_key.type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;
        field_key.data = p_ctc_key->flow_label;
        field_key.mask = p_ctc_key->flow_label_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_DA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_ctc_key->mac_da;
        field_key.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_SA))
    {
        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_ctc_key->mac_sa;
        field_key.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CVLAN))
    {
        field_key.type = CTC_FIELD_KEY_CVLAN_ID;
        field_key.data = p_ctc_key->cvlan;
        field_key.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_COS;
        field_key.data = p_ctc_key->ctag_cos;
        field_key.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_CTAG_CFI;
        field_key.data = p_ctc_key->ctag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID))
    {
        CTC_MAX_VALUE_CHECK(p_ctc_key->ctag_valid, 0x1);
        field_key.type = CTC_FIELD_KEY_CTAG_VALID;
        field_key.data = p_ctc_key->ctag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID))
    {
        field_key.type = CTC_FIELD_KEY_STAG_VALID;
        field_key.data = p_ctc_key->stag_valid;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_ECN))
    {
        field_key.type = CTC_FIELD_KEY_IP_ECN;
        field_key.data = p_ctc_key->ecn;
        field_key.mask = p_ctc_key->ecn_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_SVLAN))
    {
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = p_ctc_key->svlan;
        field_key.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_COS))
    {
        field_key.type = CTC_FIELD_KEY_STAG_COS;
        field_key.data = p_ctc_key->stag_cos;
        field_key.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_CFI))
    {
        field_key.type = CTC_FIELD_KEY_STAG_CFI;
        field_key.data = p_ctc_key->stag_cfi;
        field_key.mask = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_ETH_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_ETHER_TYPE;
        field_key.data = p_ctc_key->eth_type;
        field_key.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    /* l2 type used as vlan num
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L2_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L2_TYPE;
        field_key.data = p_ctc_key->l2_type;
        field_key.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L3_TYPE;
        field_key.data = p_ctc_key->l3_type;
        field_key.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_TYPE))
    {
        field_key.type = CTC_FIELD_KEY_L4_TYPE;
        field_key.data = p_ctc_key->l4_type;
        field_key.mask = p_ctc_key->l4_type_mask;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* layer 4 information */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_PROTOCOL))
    {
        switch (p_ctc_key->l4_protocol)
        {
        case SYS_L4_PROTOCOL_IPV6_ICMP:
            /* icmp type */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_TYPE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_TYPE;
                field_key.data = p_ctc_key->icmp_type;
                field_key.mask = p_ctc_key->icmp_type_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            /* icmp code */
            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_CODE))
            {
                field_key.type = CTC_FIELD_KEY_ICMP_CODE;
                field_key.data = p_ctc_key->icmp_code;
                field_key.mask = p_ctc_key->icmp_code_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_TCP:
        case SYS_L4_PROTOCOL_UDP:
            if (SYS_L4_PROTOCOL_TCP == p_ctc_key->l4_protocol) /* tcp upd, get help from l4_type. */
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_TCP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else
            {
                CTC_SET_FLAG(flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_TYPE);
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_UDP;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_VNI))
            {
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni, 0xFFFFFF);
                CTC_MAX_VALUE_CHECK(p_ctc_key->vni_mask, 0xFFFFFF);
                /*Add vnid must add l4 user type first*/
                field_key.type = CTC_FIELD_KEY_L4_USER_TYPE;
                field_key.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
                field_key.mask = 0xF;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

                field_key.type = CTC_FIELD_KEY_VN_ID;
                field_key.data = p_ctc_key->vni;
                field_key.mask = p_ctc_key->vni_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_GRE:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_GRE;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_GRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_GRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_NVGRE_KEY))
            {
                field_key.type = CTC_FIELD_KEY_NVGRE_KEY;
                field_key.data = p_ctc_key->gre_key;
                field_key.mask = p_ctc_key->gre_key_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        case SYS_L4_PROTOCOL_SCTP:
            field_key.type = CTC_FIELD_KEY_L4_TYPE;
            field_key.data = CTC_PARSER_L4_TYPE_SCTP;
            field_key.mask = 0xF;
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_ctc_key->l4_src_port;
                field_key.mask = p_ctc_key->l4_src_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }

            if (CTC_FLAG_ISSET(sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT))
            {
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_ctc_key->l4_dst_port;
                field_key.mask = p_ctc_key->l4_dst_port_mask;
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            break;
        default:
            return CTC_E_NOT_SUPPORT;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);

    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type= CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port = p_ctc_key->gport;
            break;
        default:

            break;
    }

    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_cvlan_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_cvlan_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);
    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port = p_ctc_key->gport;
            break;
        default:

            break;

    }

    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* vid */
    field_key.type = CTC_FIELD_KEY_CVLAN_ID;
    field_key.data = p_ctc_key->cvlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_svlan_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_svlan_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);
    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }

    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* vid */
    field_key.type = CTC_FIELD_KEY_SVLAN_ID;
    field_key.data = p_ctc_key->svlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_2vlan_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_2vlan_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);
    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.gport = p_ctc_key->gport;
            field_port.type  = CTC_FIELD_PORT_TYPE_GPORT;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.port_class_id = p_ctc_key->gport;
            field_port.type  = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.logic_port = p_ctc_key->gport;
            field_port.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            break;
        default:

            break;
    }


    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* vid */
    field_key.type = CTC_FIELD_KEY_SVLAN_ID;
    field_key.data = p_ctc_key->svlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    field_key.type = CTC_FIELD_KEY_CVLAN_ID;
    field_key.data = p_ctc_key->cvlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_cvlan_cos_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_cvlan_cos_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);
    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }

    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.data = p_ctc_key->gport;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* vid */
    field_key.type = CTC_FIELD_KEY_CVLAN_ID;
    field_key.data = p_ctc_key->cvlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* cos */
    field_key.type = CTC_FIELD_KEY_CTAG_COS;
    field_key.data = p_ctc_key->ctag_cos;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_svlan_cos_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_svlan_cos_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* dir */
    CTC_MAX_VALUE_CHECK(p_ctc_key->dir, CTC_BOTH_DIRECTION - 1);
    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }
    if(CTC_INGRESS == p_ctc_key->dir)
    {
        field_key.type = CTC_FIELD_KEY_PORT;
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_DST_GPORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* vid */
    field_key.type = CTC_FIELD_KEY_SVLAN_ID;
    field_key.data = p_ctc_key->svlan;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));


    /* cos */
    field_key.type = CTC_FIELD_KEY_STAG_COS;
    field_key.data = p_ctc_key->stag_cos;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_mac_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_mac_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;



    /* mac is da */
    CTC_MAX_VALUE_CHECK(p_ctc_key->mac_is_da, 1);
    if (p_ctc_key->mac_is_da)
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_ctc_key->mac;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_ctc_key->mac;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_mac_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_mac_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* mac is da */
    CTC_MAX_VALUE_CHECK(p_ctc_key->mac_is_da, 1);
    if (p_ctc_key->mac_is_da)
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_ctc_key->mac;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_ctc_key->mac;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_ipv4_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_ipv4_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;



    /* ip_sa */
    field_key.type = CTC_FIELD_KEY_IP_SA;
    field_key.data = p_ctc_key->ip_sa;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_ipv4_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_ipv4_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    /* ip_sa */
    field_key.type = CTC_FIELD_KEY_IP_SA;
    field_key.data = p_ctc_key->ip_sa;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_ipv6_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_ipv6_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;



    /* ip sa */
    field_key.type = CTC_FIELD_KEY_IPV6_SA;
    field_key.ext_data = p_ctc_key->ip_sa;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_port_ipv6_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_port_ipv6_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    switch (p_ctc_key->gport_type)
    {
        case CTC_SCL_GPROT_TYPE_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_PORT_CLASS:
            field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
            field_port.port_class_id= p_ctc_key->gport;
            break;
        case CTC_SCL_GPROT_TYPE_LOGIC_PORT:
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port= p_ctc_key->gport;
            break;
        default:

            break;

    }
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* ip sa */
    field_key.type = CTC_FIELD_KEY_IPV6_SA;
    field_key.ext_data = p_ctc_key->ip_sa;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_hash_l2_key(uint8 lchip, uint8 resolve_conflict, ctc_scl_hash_l2_key_t* p_ctc_key, uint32 entry_id)
{
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;



    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));

    field_key.type = CTC_FIELD_KEY_MAC_DA;
    field_key.ext_data = p_ctc_key->mac_da;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_MAC_SA;
    field_key.ext_data = p_ctc_key->mac_sa;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
    field_port.port_class_id = p_ctc_key->gport;
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &field_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_ETHER_TYPE;
    field_key.data = p_ctc_key->eth_type;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_STAG_COS;
    field_key.data = p_ctc_key->cos;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_SVLAN_ID;
    field_key.data = p_ctc_key->vlan_id;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_STAG_VALID;
    field_key.data = p_ctc_key->tag_valid;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));

    field_key.type = CTC_FIELD_KEY_STAG_CFI;
    field_key.data = p_ctc_key->cfi;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));


    /* valid */
    if(!resolve_conflict)
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }

    return CTC_E_NONE;
}

/*
 * get sys key based on ctc key
 */
STATIC int32
_ctc_usw_scl_map_key(uint8 lchip, ctc_scl_entry_t* scl_entry, uint32 entry_id)
{

    /* the resolve conflict situation has to be reconsidered */
    /* the key's direction has to be recondsidered */

    switch (scl_entry->key.type)
    {
        case CTC_SCL_KEY_TCAM_VLAN:
            return CTC_E_NOT_SUPPORT;
            break;

        case CTC_SCL_KEY_TCAM_MAC:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_tcam_mac_key(lchip, &scl_entry->key.u.tcam_mac_key, entry_id));
            break;

        case CTC_SCL_KEY_TCAM_IPV4:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_tcam_ipv4_key(lchip, &scl_entry->key.u.tcam_ipv4_key, entry_id));
            break;

        case CTC_SCL_KEY_TCAM_IPV4_SINGLE:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_tcam_ipv4_key_single(lchip, &scl_entry->key.u.tcam_ipv4_key, entry_id));
            break;

        case CTC_SCL_KEY_TCAM_IPV6:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_tcam_ipv6_key(lchip, &scl_entry->key.u.tcam_ipv6_key, entry_id));
            break;
        case CTC_SCL_KEY_HASH_PORT:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_CVLAN:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_cvlan_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_cvlan_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_SVLAN:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_svlan_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_svlan_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_2VLAN:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_2vlan_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_2vlan_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_cvlan_cos_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_cvlan_cos_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_svlan_cos_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_svlan_cos_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_MAC:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_mac_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_mac_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_MAC:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_mac_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_mac_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_IPV4:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_ipv4_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_ipv4_key,  entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_IPV4:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_ipv4_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_ipv4_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_IPV6:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_ipv6_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_ipv6_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_PORT_IPV6:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_port_ipv6_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_port_ipv6_key, entry_id));
            break;

        case CTC_SCL_KEY_HASH_L2:
            CTC_ERROR_RETURN(
            _ctc_usw_scl_map_hash_l2_key(lchip, scl_entry->resolve_conflict, &scl_entry->key.u.hash_l2_key, entry_id));
            break;

        default:
            return CTC_E_NOT_SUPPORT;
    }

    /* when the entry is resolve_conflict, struct not set mask, default all set 0xFF*/

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_igs_action(uint8 lchip, ctc_scl_igs_action_t* p_ctc_action, uint32 entry_id, uint8 is_default, uint32 gport)
{
    uint32 flag  = 0;
    ctc_scl_field_action_t field_action;
    ctc_scl_qos_map_t qos;
    sys_scl_default_action_t sys_default_action;

    CTC_PTR_VALID_CHECK(p_ctc_action);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&qos, 0, sizeof(ctc_scl_qos_map_t));
    sal_memset(&sys_default_action, 0, sizeof(sys_scl_default_action_t));


    sys_default_action.action_type = SYS_SCL_ACTION_INGRESS;
    sys_default_action.lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    /* do disrad, deny bridge, copy to copy, aging when install */

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_FID) &&
        CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT))
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_FID) &&
        CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD))
    {
        return CTC_E_INVALID_CONFIG;
    }

    /* bind en collision */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_BINDING_EN))
    {
        if ((CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_APS))
            || (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT))
            || (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_LOGIC_PORT))
            || (CTC_FLAG_ISSET(p_ctc_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_ACL_EN)))
        {
            return CTC_E_INVALID_CONFIG;
        }
    }

    /* user vlan ptr collision */
    if (((CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR))
            + (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_APS))) >= 2)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR))
        && (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_L2VPN_OAM))
    && (!(CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_VPLS_OAM))))
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (((CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_APS))
         + (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT))) >= 2)
    {
        return CTC_E_INVALID_CONFIG;
    }



    flag = p_ctc_action->flag;

    if (flag & CTC_SCL_IGS_ACTION_FLAG_IGMP_SNOOPING)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_ETREE_LEAF))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_COPY_TO_CPU))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_STATS))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_ctc_action->stats_id;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    /* service flow policer */
    if (p_ctc_action->policer_id > 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_ctc_action->policer_id;
        field_action.data1 = CTC_FLAG_ISSET(p_ctc_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_POLICER_EN)?1:0;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    /* priority */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_PRIORITY))
    {
        CTC_SET_FLAG(qos.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
        qos.priority = p_ctc_action->priority;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_COLOR))
    {
        qos.color = p_ctc_action->color;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data  = &qos;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_PRIORITY_AND_COLOR))
    {
        CTC_SET_FLAG(qos.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
        qos.priority = p_ctc_action->priority;
        qos.color = p_ctc_action->color;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_STP_ID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STP_ID;
        field_action.data0  = p_ctc_action->stp_id;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_DSCP))
    {
        CTC_SET_FLAG(qos.flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID);
        qos.dscp = p_ctc_action->dscp;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_FID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0  = p_ctc_action->fid;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_VRFID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
        field_action.data0  = p_ctc_action->vrf_id;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
        field_action.data0  = p_ctc_action->user_vlanptr;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    {
        if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_APS))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_APS;
            field_action.ext_data = &(p_ctc_action->aps);
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
            field_action.data0  = p_ctc_action->nh_id;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_LOGIC_PORT))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
            field_action.ext_data = &(p_ctc_action->logic_port);
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }
    }


    /* map vlan edit */
    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &(p_ctc_action->vlan_edit);
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_L2VPN_OAM))
    {
        if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_IGS_ACTION_FLAG_VPLS_OAM))
        {
            field_action.data0 = 0;
        }
        else
        {
            if (p_ctc_action->l2vpn_oam_id > 0xFFF)
            {
                return CTC_E_INVALID_PARAM;
            }
            field_action.data0 = 1;
        }

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;
        field_action.data1  = p_ctc_action->l2vpn_oam_id;

        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_VPLS))
    {
        sys_scl_wlan_client_t wlan_client;
        sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
        field_action.ext_data = &wlan_client;
        wlan_client.sta_roam_status = ((p_ctc_action->vpls.learning_en==0) ?1:0);

        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_IGS_ACTION_FLAG_SERVICE_ID))
    {
        if(CTC_FLAG_ISSET(p_ctc_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_ACL_EN))
        {
            sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if(CTC_FLAG_ISSET(p_ctc_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_QUEUE_EN))
        {
            sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;
            field_action.data0 = p_ctc_action->service_id;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_egs_action(uint8 lchip, ctc_scl_egs_action_t* p_ctc_action, uint32 entry_id, uint8 is_default, uint32 gport)
{
    uint32 flag  = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_default_action_t sys_default_action;

    CTC_PTR_VALID_CHECK(p_ctc_action);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&sys_default_action, 0, sizeof(sys_scl_default_action_t));

    sys_default_action.action_type = SYS_SCL_ACTION_EGRESS;
    sys_default_action.lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    flag = p_ctc_action->flag;

    /* do discard when install*/
    /* do aging when install*/

    if (CTC_FLAG_ISSET(flag, CTC_SCL_EGS_ACTION_FLAG_DISCARD))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_EGS_ACTION_FLAG_STATS))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_ctc_action->stats_id;
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    /* vlan edit */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_EGS_ACTION_FLAG_VLAN_EDIT))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &(p_ctc_action->vlan_edit);
        if (is_default)
        {
            sys_default_action.field_action = &field_action;
            CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
    }

    /*Can't remove, D2 will use*/
    if (p_ctc_action->block_pkt_type)
    {
        if (CTC_FLAG_ISSET(p_ctc_action->block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_UCAST))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_MCAST))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->block_pkt_type, CTC_SCL_BLOCKING_KNOW_UCAST))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->block_pkt_type, CTC_SCL_BLOCKING_KNOW_MCAST))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

        if (CTC_FLAG_ISSET(p_ctc_action->block_pkt_type, CTC_SCL_BLOCKING_BCAST))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST;
            if (is_default)
            {
                sys_default_action.field_action = &field_action;
                CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
            }
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_flow_action(uint8 lchip, ctc_scl_flow_action_t* p_ctc_action, uint32 entry_id)
{
    uint32 flag = 0;
    ctc_scl_field_action_t field_action;
    ctc_acl_property_t acl_prop;
    ctc_scl_qos_map_t qos;

    CTC_PTR_VALID_CHECK(p_ctc_action);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
    sal_memset(&qos, 0, sizeof(ctc_scl_qos_map_t));

    flag = p_ctc_action->flag;

    if ((CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_METADATA)
         + CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FID)
         + CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_VRFID)) > 1)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_FLOW_ACTION_FLAG_DISCARD))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_FLOW_ACTION_FLAG_COPY_TO_CPU))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(flag, CTC_SCL_FLOW_ACTION_FLAG_STATS))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_ctc_action->stats_id;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

     /* micro flow policer */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_MICRO_FLOW_POLICER))
    {

    }

    /* macro flow policer */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_MACRO_FLOW_POLICER))
    {
    }

     /* priority */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_PRIORITY))
    {
        CTC_SET_FLAG(qos.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID);
        qos.priority = p_ctc_action->priority;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* color */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_COLOR))
    {
        qos.color = p_ctc_action->color;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
        field_action.ext_data = &qos;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* qos policy (trust) */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_TRUST))
    {
        if(4 == p_ctc_action->trust)  /*SYS_TRUST_DSCP*/
        {
            qos.trust_dscp = 1;
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
            field_action.ext_data = &qos;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }
    }

    /* ds forward */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_REDIRECT))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_ctc_action->nh_id;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_BRIDGE))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_LEARNING))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_BRIDGE))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_LEARNING))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_ROUTE))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_POSTCARD_EN))
    {

    }

    /* acl hash */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_HASH_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN;
        acl_prop.acl_en = 1;
        CTC_SET_FLAG(acl_prop.flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);
        acl_prop.hash_lkup_type = p_ctc_action->acl.acl_hash_lkup_type;
        acl_prop.hash_field_sel_id = p_ctc_action->acl.acl_hash_field_sel_id;
        field_action.ext_data = &acl_prop;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* acl tcam */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_TCAM_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN;
        acl_prop.acl_en = 1;
        acl_prop.tcam_lkup_type = p_ctc_action->acl.acl_tcam_lkup_type;
        acl_prop.class_id = p_ctc_action->acl.acl_tcam_lkup_class_id;
        if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_METADATA))
        {
            CTC_SET_FLAG(acl_prop.flag, CTC_ACL_PROP_FLAG_USE_METADATA);
        }
        field_action.ext_data = &acl_prop;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* metadata */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_METADATA))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_METADATA;
        field_action.data0 = p_ctc_action->metadata;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }
    else if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0 = p_ctc_action->fid;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }
    else if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_VRFID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
        field_action.data0 = p_ctc_action->vrf_id;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* force decap */
    if (CTC_FLAG_ISSET(p_ctc_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_DECAP))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP;
        field_action.ext_data = &(p_ctc_action->force_decap);
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_scl_map_action(uint8 lchip, ctc_scl_action_t* p_ctc_action, uint32 entry_id, uint8 is_default, uint32 gport)
{
    CTC_PTR_VALID_CHECK(p_ctc_action);

    switch (p_ctc_action->type)
    {
        case CTC_SCL_ACTION_INGRESS:
            CTC_ERROR_RETURN(_ctc_usw_scl_map_igs_action(lchip, &p_ctc_action->u.igs_action, entry_id, is_default, gport));
            break;

        case CTC_SCL_ACTION_EGRESS:
            CTC_ERROR_RETURN(_ctc_usw_scl_map_egs_action(lchip, &p_ctc_action->u.egs_action, entry_id, is_default, gport));
            break;

        case CTC_SCL_ACTION_FLOW:
            CTC_ERROR_RETURN(_ctc_usw_scl_map_flow_action(lchip, &p_ctc_action->u.flow_action, entry_id));
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
ctc_usw_scl_init(uint8 lchip, void* scl_global_cfg)
{
    uint8 lchip_start               = 0;
    uint8 lchip_end                 = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_scl_init(lchip, scl_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_scl_deinit(uint8 lchip)
{
    uint8 lchip_start               = 0;
    uint8 lchip_end                 = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_scl_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(group_info);

    if((CTC_SCL_GROUP_TYPE_PORT == group_info->type) && (!CTC_IS_LINKAGG_PORT(group_info->un.gport)))
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
                                 sys_usw_scl_create_group(lchip, group_id, group_info, 0));
    }

    if (all_lchip)
    {
        /*rollback if error exist*/
        CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
        {
            sys_usw_scl_destroy_group(lchip, group_id, 0);
        }
    }

    return ret;
}

int32
ctc_usw_scl_destroy_group(uint8 lchip, uint32 group_id)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_scl_destroy_group(lchip, group_id, 0));
    }

    return ret;
}

int32
ctc_usw_scl_install_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_scl_install_group(lchip, group_id, group_info, 0));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_scl_uninstall_group(lchip, group_id, 0);
    }

    return ret;
}

int32
ctc_usw_scl_uninstall_group(uint8 lchip, uint32 group_id)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_scl_uninstall_group(lchip, group_id, 0));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_scl_install_group(lchip, group_id, NULL, 0);
    }

    return ret;
}

int32
ctc_usw_scl_get_group_info(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        ret = sys_usw_scl_get_group_info(lchip, group_id, group_info, 0);
        if(CTC_E_NONE == ret)
        {
            break;
        }
    }

    return ret;
}

STATIC int32
_ctc_usw_scl_get_lchip_by_gport(uint32 gport_type,
                                       uint32 gport,
                                       uint8* lchip_id,
                                       uint8* all_lchip)
{
    if((CTC_SCL_GPROT_TYPE_PORT == gport_type) && (!CTC_IS_LINKAGG_PORT(gport)))
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, *lchip_id);
        *all_lchip = 0;
    }
    else
    {
        *all_lchip = 1;
    }

    return CTC_E_NONE;
}

int32
ctc_usw_scl_add_entry(uint8 lchip, uint32 group_id, ctc_scl_entry_t* scl_entry)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 1;
    uint8 lchip_id                 = 0;
    int32 ret                      = CTC_E_NONE;
    ctc_scl_group_info_t group_info;
    uint8   group_exist_one = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(scl_entry);

    if (!scl_entry->mode)
    {
        if (CTC_SCL_KEY_TCAM_IPV4 == scl_entry->key.type)
        {
            /* only key type is CTC_SCL_KEY_TCAM_IPV4, access tcam_ipv4_key is legal */
            if (CTC_SCL_KEY_SIZE_DOUBLE == scl_entry->key.u.tcam_ipv4_key.key_size)
            {
                scl_entry->key.type = CTC_SCL_KEY_TCAM_IPV4;
            }
            else
            {
                scl_entry->key.type = CTC_SCL_KEY_TCAM_IPV4_SINGLE;
            }
        }

        scl_entry->key_type = scl_entry->key.type;

        scl_entry->action_type = scl_entry->action.type;
        if (CTC_SCL_KEY_HASH_L2 == scl_entry->key_type)
        {
            scl_entry->hash_field_sel_id = scl_entry->key.u.hash_l2_key.field_sel_id;
        }

        switch (scl_entry->key_type)
        {
            case CTC_SCL_KEY_HASH_PORT:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_key.gport_type,
                                                  scl_entry->key.u.hash_port_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_CVLAN:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_cvlan_key.gport_type,
                                                  scl_entry->key.u.hash_port_cvlan_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_SVLAN:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_svlan_key.gport_type,
                                                  scl_entry->key.u.hash_port_svlan_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_2VLAN:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_2vlan_key.gport_type,
                                                  scl_entry->key.u.hash_port_2vlan_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_cvlan_cos_key.gport_type,
                                                  scl_entry->key.u.hash_port_cvlan_cos_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_svlan_cos_key.gport_type,
                                                  scl_entry->key.u.hash_port_svlan_cos_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_MAC:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_mac_key.gport_type,
                                                  scl_entry->key.u.hash_port_mac_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_IPV4:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_ipv4_key.gport_type,
                                                  scl_entry->key.u.hash_port_ipv4_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;
            case CTC_SCL_KEY_HASH_PORT_IPV6:
                _ctc_usw_scl_get_lchip_by_gport(scl_entry->key.u.hash_port_ipv6_key.gport_type,
                                                  scl_entry->key.u.hash_port_ipv6_key.gport,
                                                  &lchip_id,
                                                  &all_lchip);
                break;

            default:
                all_lchip = 1;
        }
        CTC_SET_API_LCHIP(lchip, lchip_id);

        lchip_start         = lchip;
        lchip_end           = lchip + 1;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        ret = sys_usw_scl_get_group_info(lchip, group_id, &group_info, 0);
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
        ret = !ret ? sys_usw_scl_add_entry(lchip, group_id, scl_entry, 0):ret;
        if((CTC_E_NONE == ret) && (!scl_entry->mode))
        {
            ret = _ctc_usw_scl_map_key(lchip, scl_entry, scl_entry->entry_id);
            if((!g_ctcs_api_en) && (g_lchip_num > 1) && (CTC_E_INVALID_CHIP_ID == ret|| CTC_E_INVALID_PORT  == ret))
            {
                sys_usw_scl_remove_entry(lchip, scl_entry->entry_id, 0);
                ret = CTC_E_NONE;
                continue;
            }

            ret = !ret ? _ctc_usw_scl_map_action(lchip, &scl_entry->action, scl_entry->entry_id, 0, 0):ret;
            if(CTC_E_NONE != ret)
            {
                sys_usw_scl_remove_entry(lchip, scl_entry->entry_id, 0);
                break;
            }
        }
    }

    if (all_lchip)
    {
        /*rollback if error exist*/
        CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
        {
            sys_usw_scl_remove_entry(lchip, scl_entry->entry_id, 0);
        }
    }
    return ret;
}

int32
ctc_usw_scl_remove_entry(uint8 lchip, uint32 entry_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    uint8 count                    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_scl_remove_entry(lchip, entry_id, 0));
    }

    return ret;
}

int32
ctc_usw_scl_install_entry(uint8 lchip, uint32 entry_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    uint8 count                    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_scl_install_entry(lchip, entry_id, 0));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_scl_uninstall_entry(lchip, entry_id, 0);
    }

    return ret;
}

int32
ctc_usw_scl_uninstall_entry(uint8 lchip, uint32 entry_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    uint8 count                    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_scl_uninstall_entry(lchip, entry_id, 0));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_scl_install_entry(lchip, entry_id, 0);
    }

    return ret;
}

int32
ctc_usw_scl_remove_all_entry(uint8 lchip, uint32 group_id)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_scl_remove_all_entry(lchip, group_id, 0));
    }

    return ret;
}

int32
ctc_usw_scl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    uint8 count                    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_scl_set_entry_priority(lchip, entry_id, priority, 0));
    }

    return ret;
}

int32
ctc_usw_scl_get_multi_entry(uint8 lchip, ctc_scl_query_t* query)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        ret = sys_usw_scl_get_multi_entry(lchip, query, 0);
        if(CTC_E_NONE == ret)
        {
            break;
        }
    }

    return ret;
}

int32
ctc_usw_scl_set_default_action(uint8 lchip, ctc_scl_default_action_t* def_action)
{
    uint16 lport                   = 0;
    int32 ret = 0;
    ctc_scl_field_action_t   field_action;
    sys_scl_default_action_t sys_default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&sys_default_action, 0, sizeof(sys_scl_default_action_t));

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);

    if (!def_action->mode)
    {
        if (CTC_SCL_ACTION_INGRESS == def_action->action.type)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_INGRESS;
        }
        else if (CTC_SCL_ACTION_EGRESS == def_action->action.type)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_EGRESS;
        }
        else if (CTC_SCL_ACTION_FLOW == def_action->action.type)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_FLOW;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        if (CTC_SCL_ACTION_INGRESS == def_action->dir)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_INGRESS;
        }
        else if (CTC_SCL_ACTION_EGRESS == def_action->dir)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_EGRESS;
        }
        else if (CTC_SCL_ACTION_FLOW == def_action->dir)
        {
            sys_default_action.action_type = SYS_SCL_ACTION_FLOW;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    SYS_MAP_GPORT_TO_LCHIP(def_action->gport, lchip);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(def_action->gport);
    sys_default_action.lport = lport;
    sys_default_action.scl_id = def_action->scl_id;

    if (!def_action->mode)
    {
        /* set pending status */
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        field_action.data0 = 1;
        sys_default_action.field_action = &field_action;
        CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));

        CTC_ERROR_GOTO(_ctc_usw_scl_map_action(lchip, &(def_action->action), 0, 1, def_action->gport), ret, error0);

        /* clear pending status and install ad immediately */
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        field_action.data0 = 0;
        sys_default_action.field_action = &field_action;
        CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
    }
    else
    {
        sys_default_action.field_action = def_action->field_action;
        CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &sys_default_action));
    }

    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_default_action.is_rollback = 1;
    sys_default_action.field_action = &field_action;
    sys_usw_scl_set_default_action(lchip, &sys_default_action);
    return ret;
}

int32
ctc_usw_scl_set_hash_field_sel(uint8 lchip, ctc_scl_hash_field_sel_t* field_sel)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_scl_set_hash_field_sel(lchip, field_sel));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_scl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
	int32 ret = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_field_key);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                        ret,
                                        sys_usw_scl_add_key_field(lchip, entry_id, p_field_key));

    }

    return ret;
}

int32
ctc_usw_scl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_field_key);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                        ret,
                                        sys_usw_scl_remove_key_field(lchip, entry_id, p_field_key));
    }

    return ret;
}

int32
ctc_usw_scl_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_field_action);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                        ret,
                                        sys_usw_scl_add_action_field(lchip, entry_id, p_field_action));
    }

    return ret;
}

int32
ctc_usw_scl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    uint8 count                    = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SCL);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_field_action);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ACL_SCL_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                        ret,
                                        sys_usw_scl_remove_action_field(lchip, entry_id, p_field_action));
    }

    return ret;
}



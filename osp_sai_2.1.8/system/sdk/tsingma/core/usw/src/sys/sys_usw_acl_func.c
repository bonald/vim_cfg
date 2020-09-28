/**
   @file sys_usw_acl_func.c

   @date 2015-11-01

   @version v3.0

 */

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_common.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_hash.h"
#include "ctc_parser.h"
#include "ctc_acl.h"
#include "ctc_linklist.h"
#include "ctc_spool.h"
#include "ctc_field.h"
#include "ctc_packet.h"

#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_parser.h"
#include "sys_usw_parser_io.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_l3if.h"
#include "sys_usw_port.h"
#include "sys_usw_acl.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

#define SYS_USW_ACL_MAX_LOGIC_PORT_NUM (DRV_IS_DUET2(lchip)? 0x7FFF: 0xFFFF)

extern sys_acl_master_t* p_usw_acl_master[CTC_MAX_LOCAL_CHIP_NUM];

enum sys_acl_mergedata_type_e
{
    SYS_ACL_MERGEDATA_TYPE_NONE,
    SYS_ACL_MERGEDATA_TYPE_VXLAN,
    SYS_ACL_MERGEDATA_TYPE_GRE,
    SYS_ACL_MERGEDATA_TYPE_CAPWAP,

    MAX_SYS_ACL_MERGEDATA_TYPE
};
typedef enum sys_acl_mergedata_type_e sys_acl_mergedata_type_e_t;
#define SYS_USW_ACL_IPFIX_CONFLICT_CHECK(ipfix_op_code, p_buffer) \
    do{\
        uint8 tmp_ipfix_op_code = GetDsAcl(V, ipfixOpCode_f, p_buffer->action);\
        if (tmp_ipfix_op_code && (ipfix_op_code != tmp_ipfix_op_code))\
        {\
            CTC_ERROR_RETURN(CTC_E_PARAM_CONFLICT);\
        }\
    }while(0)
#define SYS_USW_ACL_CHECK_ACTION_CONFLICT(action_bmp,conf_action0,conf_action1, conf_action2) \
    do{\
        if(conf_action0 && CTC_BMP_ISSET(action_bmp, conf_action0))\
        {\
            return CTC_E_PARAM_CONFLICT;\
        }\
        if(conf_action1 && CTC_BMP_ISSET(action_bmp, conf_action1))\
        {\
            return CTC_E_PARAM_CONFLICT;\
        }\
        if(conf_action2 && CTC_BMP_ISSET(action_bmp, conf_action2))\
        {\
            return CTC_E_PARAM_CONFLICT;\
        }\
    }while(0)

STATIC int32 _sys_usw_acl_build_egress_vlan_edit(uint8 lchip, void* p_ds_edit, sys_acl_vlan_edit_t* p_vlan_edit, uint8 is_add);

#define _ACL_COMMON_
int32
_sys_usw_acl_get_table_id(uint8 lchip, sys_acl_entry_t* pe, uint32* key_id, uint32* act_id, uint32* hw_index)
{
    uint32 _key_id  = 0;
    uint32 _act_id  = 0;
    uint32 _hw_index= 0;
    uint8  dir      = 0;
    uint8  block_id = 0;
    uint8  hw_block_id = 0;
    uint8  idx = 0;
    uint8  max_idx = 0;
    uint8  ad_step  = 0;
    sys_acl_league_t* p_sys_league = NULL;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(pe->group);
    dir      = pe->group->group_info.dir;
    block_id = pe->group->group_info.block_id;
    _hw_index   = pe->fpae.offset_a;

    if (pe->key_type == CTC_ACL_KEY_MAC)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosMacKey160Ing0_t : DsAclQosMacKey160Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0MacTcamAd_t : DsEgrAcl0MacTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1MacTcamAd_t - DsIngAcl0MacTcamAd_t) :        \
                                                (DsEgrAcl1MacTcamAd_t - DsEgrAcl0MacTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_IPV4)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosL3Key160Ing0_t : DsAclQosL3Key160Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0L3BasicTcamAd_t : DsEgrAcl0L3BasicTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1L3BasicTcamAd_t - DsIngAcl0L3BasicTcamAd_t) :    \
                                                (DsEgrAcl1L3BasicTcamAd_t - DsEgrAcl0L3BasicTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_IPV4_EXT)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosL3Key320Ing0_t : DsAclQosL3Key320Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0L3ExtTcamAd_t : DsEgrAcl0L3ExtTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1L3ExtTcamAd_t - DsIngAcl0L3ExtTcamAd_t) :    \
                                                (DsEgrAcl1L3ExtTcamAd_t - DsEgrAcl0L3ExtTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_MAC_IPV4)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosMacL3Key320Ing0_t : DsAclQosMacL3Key320Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0MacL3BasicTcamAd_t : DsEgrAcl0MacL3BasicTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1MacL3BasicTcamAd_t - DsIngAcl0MacL3BasicTcamAd_t) : \
                                            (DsEgrAcl1MacL3BasicTcamAd_t - DsEgrAcl0MacL3BasicTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_MAC_IPV4_EXT)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosMacL3Key640Ing0_t : DsAclQosMacL3Key640Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0MacL3ExtTcamAd_t : DsEgrAcl0MacL3ExtTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1MacL3ExtTcamAd_t - DsIngAcl0MacL3ExtTcamAd_t) : \
                                            (DsEgrAcl1MacL3ExtTcamAd_t - DsEgrAcl0MacL3ExtTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_IPV6)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosIpv6Key320Ing0_t : DsAclQosIpv6Key320Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0Ipv6BasicTcamAd_t : DsEgrAcl0Ipv6BasicTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1Ipv6BasicTcamAd_t - DsIngAcl0Ipv6BasicTcamAd_t) : \
                                            (DsEgrAcl1Ipv6BasicTcamAd_t - DsEgrAcl0Ipv6BasicTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_IPV6_EXT)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosIpv6Key640Ing0_t : DsAclQosIpv6Key640Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0Ipv6ExtTcamAd_t : DsEgrAcl0Ipv6ExtTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1Ipv6ExtTcamAd_t - DsIngAcl0Ipv6ExtTcamAd_t) : \
                                            (DsEgrAcl1Ipv6ExtTcamAd_t - DsEgrAcl0Ipv6ExtTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_MAC_IPV6)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosMacIpv6Key640Ing0_t : DsAclQosMacIpv6Key640Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0MacIpv6TcamAd_t : DsEgrAcl0MacIpv6TcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1MacIpv6TcamAd_t - DsIngAcl0MacIpv6TcamAd_t) : \
                                            (DsEgrAcl1MacIpv6TcamAd_t - DsEgrAcl0MacIpv6TcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_INTERFACE)
    {
        _key_id = (CTC_INGRESS == dir) ? DsAclQosKey80Ing0_t : DsAclQosKey80Egr0_t;
        _act_id = (CTC_INGRESS == dir) ? DsIngAcl0IpfixEnableTcamAd_t : DsEgrAcl0IpfixEnableTcamAd_t;
        ad_step = (CTC_INGRESS == dir) ? (DsIngAcl1IpfixEnableTcamAd_t - DsIngAcl0IpfixEnableTcamAd_t) : \
                                            (DsEgrAcl1IpfixEnableTcamAd_t - DsEgrAcl0IpfixEnableTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_FWD)
    {
        _key_id = DsAclQosForwardKey320Ing0_t;
        _act_id = DsIngAcl0ForwardBasicTcamAd_t;
        ad_step = DsIngAcl1ForwardBasicTcamAd_t - DsIngAcl0ForwardBasicTcamAd_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_FWD_EXT)
    {
        _key_id = DsAclQosForwardKey640Ing0_t;
        _act_id = DsIngAcl0ForwardExtTcamAd_t;
        ad_step = DsIngAcl1ForwardExtTcamAd_t - DsIngAcl0ForwardExtTcamAd_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_CID)
    {
        _key_id = DsAclQosCidKey160Ing0_t;
        _act_id =  DsAcl0Ingress_t;
        ad_step = DsAcl1Ingress_t - DsAcl0Ingress_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_COPP)
    {
        _key_id = DsAclQosCoppKey320Ing0_t;
        _act_id = DsIngAcl0CoppBasicTcamAd_t;
        ad_step = DsIngAcl1CoppBasicTcamAd_t - DsIngAcl0CoppBasicTcamAd_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_COPP_EXT)
    {
        _key_id = DsAclQosCoppKey640Ing0_t;
        _act_id = DsIngAcl0CoppExtTcamAd_t;
        ad_step = DsIngAcl1CoppExtTcamAd_t - DsIngAcl0CoppExtTcamAd_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_UDF)
    {
        _key_id = CTC_INGRESS==dir? DsAclQosUdfKey320Ing0_t: DsAclQosUdfKey320Egr0_t;
        _act_id = CTC_INGRESS==dir? DsIngAcl0UdfTcamAd_t: DsEgrAcl0UdfTcamAd_t;
        ad_step = CTC_INGRESS==dir? (DsIngAcl1UdfTcamAd_t - DsIngAcl0UdfTcamAd_t): (DsEgrAcl1UdfTcamAd_t - DsEgrAcl0UdfTcamAd_t);
    }
    else if (pe->key_type == CTC_ACL_KEY_HASH_MAC)
    {
        _key_id = DsFlowL2HashKey_t;
        _act_id = DsFlow_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_HASH_IPV4)
    {
        _key_id = DsFlowL3Ipv4HashKey_t;
        _act_id = DsFlow_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_HASH_IPV6)
    {
        _key_id = DsFlowL3Ipv6HashKey_t;
        _act_id = DsFlow_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_HASH_MPLS)
    {
        _key_id = DsFlowL3MplsHashKey_t;
        _act_id = DsFlow_t;
    }
    else if (pe->key_type == CTC_ACL_KEY_HASH_L2_L3)
    {
        _key_id = DsFlowL2L3HashKey_t;
        _act_id = DsFlow_t;
    }

    if(ACL_ENTRY_IS_TCAM(pe->key_type))/*for tcam, 8 ingress and 3 egress*/
    {
        p_sys_league = &(p_usw_acl_master[lchip]->league[(dir==CTC_INGRESS)?block_id:(block_id+ACL_IGS_BLOCK_MAX_NUM)]);
        hw_block_id = block_id;
        max_idx = (dir==CTC_INGRESS)?(ACL_IGS_BLOCK_MAX_NUM-1):(ACL_EGS_BLOCK_MAX_NUM-1);

        for (idx = max_idx; idx > block_id; idx--)
        {
            if ((p_sys_league->lkup_level_start[idx] > 0) && (pe->fpae.offset_a >= p_sys_league->lkup_level_start[idx]))
            {
                hw_block_id = idx;
                _hw_index = pe->fpae.offset_a - (p_sys_league->lkup_level_start[idx-1]+p_sys_league->lkup_level_count[idx-1]);
                break;
            }
        }

        _key_id = _key_id + hw_block_id;
        _act_id = _act_id + hw_block_id * ad_step;

    }

    if (key_id)
    {
        *key_id = _key_id;
    }

    if (act_id)
    {
        *act_id = _act_id;
    }

    if (hw_index)
    {
        *hw_index = _hw_index;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_key_common_field_group_none(uint8 lchip, sys_acl_entry_t* pe )
{
    uint8 key_type = 0;
    sys_acl_buffer_t* p_buffer = NULL;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    key_type = pe->key_type;
    p_buffer = pe->buffer;

   switch(key_type)
    {
    case CTC_ACL_KEY_MAC:
        SetDsAclQosMacKey160(V, aclQosKeyType_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACKEY_160));
        SetDsAclQosMacKey160(V, aclQosKeyType_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_IPV4:
        SetDsAclQosL3Key160(V, aclQosKeyType_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_160));
        SetDsAclQosL3Key160(V, aclQosKeyType_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_IPV4_EXT:
        SetDsAclQosL3Key320(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_320));
        SetDsAclQosL3Key320(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosL3Key320(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_320));
        SetDsAclQosL3Key320(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_MAC_IPV4:
        SetDsAclQosMacL3Key320(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_320));
        SetDsAclQosMacL3Key320(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosMacL3Key320(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_320));
        SetDsAclQosMacL3Key320(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_MAC_IPV4_EXT:
        SetDsAclQosMacL3Key640(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640));
        SetDsAclQosMacL3Key640(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosMacL3Key640(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640));
        SetDsAclQosMacL3Key640(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        SetDsAclQosMacL3Key640(V, aclQosKeyType2_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640));
        SetDsAclQosMacL3Key640(V, aclQosKeyType2_f, p_buffer->mask, 0xF);
        SetDsAclQosMacL3Key640(V, aclQosKeyType3_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640));
        SetDsAclQosMacL3Key640(V, aclQosKeyType3_f, p_buffer->mask, 0xF);
        SetDsAclQosMacL3Key640(V, isUpper0_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key640(V, isUpper0_f, p_buffer->mask, 1);
        SetDsAclQosMacL3Key640(V, isUpper1_f, p_buffer->key, 1);
        SetDsAclQosMacL3Key640(V, isUpper1_f, p_buffer->mask, 1);
        break;

    case CTC_ACL_KEY_IPV6:
        SetDsAclQosIpv6Key320(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_320));
        SetDsAclQosIpv6Key320(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosIpv6Key320(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_320));
        SetDsAclQosIpv6Key320(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_IPV6_EXT:
        SetDsAclQosIpv6Key640(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640));
        SetDsAclQosIpv6Key640(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosIpv6Key640(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640));
        SetDsAclQosIpv6Key640(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        SetDsAclQosIpv6Key640(V, aclQosKeyType2_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640));
        SetDsAclQosIpv6Key640(V, aclQosKeyType2_f, p_buffer->mask, 0xF);
        SetDsAclQosIpv6Key640(V, aclQosKeyType3_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640));
        SetDsAclQosIpv6Key640(V, aclQosKeyType3_f, p_buffer->mask, 0xF);
        SetDsAclQosIpv6Key640(V, isUpper0_f, p_buffer->key, 0);
        SetDsAclQosIpv6Key640(V, isUpper0_f, p_buffer->mask, 1);
        SetDsAclQosIpv6Key640(V, isUpper1_f, p_buffer->key, 1);
        SetDsAclQosIpv6Key640(V, isUpper1_f, p_buffer->mask, 1);
        break;

    case CTC_ACL_KEY_MAC_IPV6:
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640));
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640));
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType2_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640));
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType2_f, p_buffer->mask, 0xF);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType3_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640));
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType3_f, p_buffer->mask, 0xF);
        SetDsAclQosMacIpv6Key640(V, isUpper0_f, p_buffer->key, 0);
        SetDsAclQosMacIpv6Key640(V, isUpper0_f, p_buffer->mask, 1);
        SetDsAclQosMacIpv6Key640(V, isUpper1_f, p_buffer->key, 1);
        SetDsAclQosMacIpv6Key640(V, isUpper1_f, p_buffer->mask, 1);
        break;

    case CTC_ACL_KEY_CID:
        SetDsAclQosCidKey160(V, aclQosKeyType_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_CIDKEY_160));
        SetDsAclQosCidKey160(V, aclQosKeyType_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_INTERFACE:
        SetDsAcl(V, isHalfAd_f, p_buffer->action, 1);
        break;

    case CTC_ACL_KEY_FWD:
        SetDsAclQosForwardKey320(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_320));
        SetDsAclQosForwardKey320(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosForwardKey320(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_320));
        SetDsAclQosForwardKey320(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_FWD_EXT:
        SetDsAclQosForwardKey640(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640));
        SetDsAclQosForwardKey640(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosForwardKey640(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640));
        SetDsAclQosForwardKey640(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        SetDsAclQosForwardKey640(V, aclQosKeyType2_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640));
        SetDsAclQosForwardKey640(V, aclQosKeyType2_f, p_buffer->mask, 0xF);
        SetDsAclQosForwardKey640(V, aclQosKeyType3_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640));
        SetDsAclQosForwardKey640(V, aclQosKeyType3_f, p_buffer->mask, 0xF);
        SetDsAclQosForwardKey640(V, isUpper0_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, isUpper0_f, p_buffer->mask, 1);
        SetDsAclQosForwardKey640(V, isUpper1_f, p_buffer->key, 1);
        SetDsAclQosForwardKey640(V, isUpper1_f, p_buffer->mask, 1);
        break;

    case CTC_ACL_KEY_COPP:
        SetDsAclQosCoppKey320(V, keyType_f, p_buffer->key, 0);
        SetDsAclQosCoppKey320(V, keyType_f, p_buffer->mask, 1);
        break;
    case CTC_ACL_KEY_COPP_EXT:
        SetDsAclQosCoppKey640(V, keyType0_f, p_buffer->key, 1);
        SetDsAclQosCoppKey640(V, keyType0_f, p_buffer->mask, 1);
        SetDsAclQosCoppKey640(V, keyType1_f, p_buffer->key, 1);
        SetDsAclQosCoppKey640(V, keyType1_f, p_buffer->mask, 1);
        SetDsAclQosCoppKey640(V, isUpper0_f, p_buffer->key, 0);
        SetDsAclQosCoppKey640(V, isUpper0_f, p_buffer->mask, 1);
        SetDsAclQosCoppKey640(V, isUpper1_f, p_buffer->key, 1);
        SetDsAclQosCoppKey640(V, isUpper1_f, p_buffer->mask, 1);
        break;

    case CTC_ACL_KEY_UDF :
        SetDsAclQosUdfKey320(V, aclQosKeyType0_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_UDFKEY_320));
        SetDsAclQosUdfKey320(V, aclQosKeyType0_f, p_buffer->mask, 0xF);
        SetDsAclQosUdfKey320(V, aclQosKeyType1_f, p_buffer->key, DRV_ENUM(DRV_TCAMKEYTYPE_UDFKEY_320));
        SetDsAclQosUdfKey320(V, aclQosKeyType1_f, p_buffer->mask, 0xF);
        break;

    case CTC_ACL_KEY_HASH_IPV4:
        SetDsFlowL3Ipv4HashKey(V, hashKeyType_f, p_buffer->key, FLOWHASHTYPE_L3IPV4);
        SetDsFlowL3Ipv4HashKey(V, flowFieldSel_f, p_buffer->key, pe->hash_field_sel_id);
        break;

    case CTC_ACL_KEY_HASH_IPV6:
        SetDsFlowL3Ipv6HashKey(V, hashKeyType0_f, p_buffer->key, FLOWHASHTYPE_L3IPV6);
        SetDsFlowL3Ipv6HashKey(V, hashKeyType1_f, p_buffer->key, FLOWHASHTYPE_L3IPV6);
        SetDsFlowL3Ipv6HashKey(V, flowFieldSel_f, p_buffer->key, pe->hash_field_sel_id);
        break;

    case CTC_ACL_KEY_HASH_L2_L3:
        SetDsFlowL2L3HashKey(V, hashKeyType0_f, p_buffer->key, FLOWHASHTYPE_L2L3);
        SetDsFlowL2L3HashKey(V, hashKeyType1_f, p_buffer->key, FLOWHASHTYPE_L2L3);
        SetDsFlowL2L3HashKey(V, flowFieldSel_f, p_buffer->key, pe->hash_field_sel_id);
        break;

    case CTC_ACL_KEY_HASH_MAC:
        SetDsFlowL2HashKey(V, hashKeyType_f, p_buffer->key, FLOWHASHTYPE_L2);
        SetDsFlowL2HashKey(V, flowFieldSel_f, p_buffer->key, pe->hash_field_sel_id);
        break;
    case CTC_ACL_KEY_HASH_MPLS:
        SetDsFlowL3MplsHashKey(V, hashKeyType_f, p_buffer->key, FLOWHASHTYPE_L3MPLS);
        SetDsFlowL3MplsHashKey(V, flowFieldSel_f, p_buffer->key, pe->hash_field_sel_id);
        break;

    default:
        break;
    }

    return CTC_E_NONE;


}

int32
_sys_usw_acl_rebuild_buffer_from_hw(uint8 lchip, uint32 key_id, uint32 act_id, uint32 hw_index, sys_acl_entry_t* pe)
{
    uint32 cmd       = 0;
    uint32 key_index = 0;
    uint32 ad_index  = 0;
    int32  ret       = CTC_E_NONE;
    tbl_entry_t         tcam_key;
    sys_acl_buffer_t*   p_new_buffer = NULL;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&tcam_key, 0, sizeof(tcam_key));
    MALLOC_ZERO(MEM_ACL_MODULE, p_new_buffer, sizeof(sys_acl_buffer_t));
    if(!p_new_buffer)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
		return CTC_E_NO_RESOURCE;
    }

    if(ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        key_index = SYS_ACL_MAP_DRV_KEY_INDEX(hw_index, pe->fpae.real_step);
        ad_index  = SYS_ACL_MAP_DRV_AD_INDEX(hw_index, pe->fpae.real_step);

        /*get key from hw*/
        cmd = DRV_IOR(key_id, DRV_ENTRY_FLAG);
        tcam_key.data_entry = p_new_buffer->key;
        tcam_key.mask_entry = p_new_buffer->mask;
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &tcam_key), ret, clean);

        /*get action from hw*/
        cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, ad_index, cmd, p_new_buffer->action), ret, clean);
    }
    else
    {
	    drv_acc_in_t in;
        drv_acc_out_t out;
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));
        in.type = DRV_ACC_TYPE_LOOKUP;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = key_id;
        in.index = hw_index;
        CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, clean);

        sal_memcpy(p_new_buffer->key, out.data, sizeof(ds_t));

        /*get action from hw*/
        cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, pe->ad_index, cmd, p_new_buffer->action), ret, clean);
    }

    pe->buffer = p_new_buffer;

    return CTC_E_NONE;
clean:
    if(p_new_buffer)
    {
        mem_free(p_new_buffer);
    }
    return ret;
}

/*
 * remove entry specified by entry id from hardware table
 */
int32
_sys_usw_acl_remove_hw(uint8 lchip, sys_acl_entry_t* pe)
{
    uint32     key_id    = 0;
    uint32     act_id    = 0;
    uint32     cmd       = 0;
    uint8      step      = 0;
    uint32     hw_index  = 0;

    drv_acc_in_t    in;
    drv_acc_out_t   out;

    tbl_entry_t         tcam_key;
    ds_t                ds_hash_key;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid: %u \n", pe->fpae.entry_id);

    sal_memset(&tcam_key, 0, sizeof(tcam_key));

    /* get table_id, index */
    _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);

    CTC_ERROR_RETURN(_sys_usw_acl_rebuild_buffer_from_hw(lchip, key_id, act_id, hw_index, pe));

    if (!ACL_ENTRY_IS_TCAM(pe->key_type))
    {

	    if(!pe->hash_valid)
		{
		   return CTC_E_NONE;
		}
        /*remove from spool*/
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_acl_master[lchip]->ad_spool, pe->buffer->action, NULL));

        /*delete hash key in hw, ad not care*/
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));
        sal_memset(&ds_hash_key, 0xFF, sizeof(ds_hash_key));

        in.type     = DRV_ACC_TYPE_ADD;
        in.op_type  = DRV_ACC_OP_BY_INDEX;
        in.data     = ds_hash_key;
        in.tbl_id   = key_id;
        in.index    = hw_index;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    }
    else
    {
        /*delete key, ad not care*/
        _sys_usw_acl_get_key_size(lchip, 1, pe, &step);
        cmd = DRV_IOD(key_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_ACL_MAP_DRV_KEY_INDEX(hw_index, step), cmd, &cmd));
    }

    return CTC_E_NONE;
}

/*
 * add entry specified by entry id to hardware table
 */
int32
_sys_usw_acl_add_hw(uint8 lchip, sys_acl_entry_t* pe, uint32* old_action)
{
    DsAclVlanActionProfile_m  ds_vlan_edit;
    uint8       block_id  = 0;
    uint32      key_index = 0;
    uint32      ad_index  = 0;
    uint32      hw_index  = 0;
    uint32      cmd       = 0;
    uint32      key_id    = 0;
    uint32      act_id    = 0;
    uint32*     p_hash_ad = NULL;
    uint8       step      = 0;
    sys_acl_group_t* pg = pe->group;
    tbl_entry_t   tcam_key;

    sal_memset(&ds_vlan_edit, 0, sizeof(DsAclVlanActionProfile_m));
    sal_memset(&tcam_key, 0, sizeof(tcam_key));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: eid: %u \n", pe->fpae.entry_id);



    /* get block_id, ad_index */
    block_id  = pg->group_info.block_id;

    _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% key_tbl_id:[%u]  ad_tbl_id:[%u]  \n", key_id, act_id);

    if (!ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->ad_spool, pe->buffer->action, old_action, &p_hash_ad));
        pe->hash_action = p_hash_ad;
        key_index = hw_index;
        pe->ad_index = p_hash_ad[ACL_HASH_AD_INDEX_OFFSET];
        ad_index = p_hash_ad[ACL_HASH_AD_INDEX_OFFSET];
    }
    else
    {
        _sys_usw_acl_get_key_size(lchip, 1, pe, &step);
        key_index = SYS_ACL_MAP_DRV_KEY_INDEX(hw_index, step);
        ad_index = SYS_ACL_MAP_DRV_AD_INDEX(hw_index, step);
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% key_index:[0x%x]  ad_index:[0x%x] \n", key_index, ad_index);

    cmd = DRV_IOW(act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, (void*)pe->buffer->action));

    /*only tcam action has vlan edit*/
    if (pe->vlan_edit_valid && pe->vlan_edit)
    {
        _sys_usw_acl_build_vlan_edit(lchip, &ds_vlan_edit, pe->vlan_edit);
        cmd = DRV_IOW(DsAclVlanActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->vlan_edit->profile_id, cmd, &ds_vlan_edit));
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% key_type: %d block_id %d\n", pe->key_type, block_id);

    if (!ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        drv_acc_in_t in;
        drv_acc_out_t out;

        sal_memset(&in, 0, sizeof(in));

        switch(pe->key_type)
        {
            case CTC_ACL_KEY_HASH_IPV4:
                SetDsFlowL3Ipv4HashKey(V, dsAdIndex_f, pe->buffer->key, ad_index);
                break;
            case CTC_ACL_KEY_HASH_IPV6:
                SetDsFlowL3Ipv6HashKey(V, dsAdIndex_f, pe->buffer->key, ad_index);
                break;
            case CTC_ACL_KEY_HASH_L2_L3:
                SetDsFlowL2L3HashKey(V, dsAdIndex_f, pe->buffer->key, ad_index);
                break;
            case CTC_ACL_KEY_HASH_MAC:
                SetDsFlowL2HashKey(V, dsAdIndex_f, pe->buffer->key, ad_index);
                break;
            case CTC_ACL_KEY_HASH_MPLS:
                SetDsFlowL3MplsHashKey(V, dsAdIndex_f, pe->buffer->key, ad_index);
                break;
            default:
                break;
        }

        in.type     = DRV_ACC_TYPE_ADD;
        in.op_type  = DRV_ACC_OP_BY_INDEX;
        in.tbl_id   = key_id;
        in.data     = pe->buffer->key;
        in.index    = key_index;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    }
    else if(ACL_ENTRY_IS_TCAM(pe->key_type) && (NULL == old_action))
    {
        tcam_key.data_entry = pe->buffer->key;
        tcam_key.mask_entry = pe->buffer->mask;
        cmd = DRV_IOW(key_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &tcam_key));
    }

    /*remove buffer*/
    if(pe->buffer)
    {
        mem_free(pe->buffer);
    }

    return CTC_E_NONE;
}

/*
 * set acl register
 */
int32
_sys_usw_acl_set_register(uint8 lchip, sys_acl_register_t* p_reg)
{
    uint32               cmd = 0;
    uint32 discard_bitmap[2] = {0};
    uint32             field = 0;
    uint8              index = 0;
    uint8              step = 0;
    IpeAclQosCtl_m       ipe;
    IpeFwdCtl_m          ipe_fwd;
    IpeFwdAclCtl_m       ipe_fwd_acl;
    IpePreLookupAclCtl_m ipe_pre;
    EpeAclQosCtl_m       epe_acl_ctl;
    IpeIntfMapperCtl_m   ipe_intf_ctl;
    EpeNextHopCtl_m      epe_nh_ctl;
    FlowTcamLookupCtl_m  flow_ctl;
	uint8 loop = 0;

    IpeFwdCategoryCtl_m  ipe_fwd_cid;
	uint32 value = 0;
    uint32 value_a[8];

    CTC_PTR_VALID_CHECK(p_reg);
    CTC_MAX_VALUE_CHECK(p_reg->egress_service_acl_en, 0x7);

    sal_memset(&ipe,         0, sizeof(IpeAclQosCtl_m));
    sal_memset(&ipe_fwd,     0, sizeof(IpeFwdCtl_m));
    sal_memset(&ipe_fwd_acl, 0, sizeof(IpeFwdAclCtl_m));
    sal_memset(&ipe_pre,     0, sizeof(IpePreLookupAclCtl_m));
    sal_memset(&epe_acl_ctl, 0, sizeof(EpeAclQosCtl_m));
    sal_memset(&ipe_intf_ctl, 0, sizeof(IpeIntfMapperCtl_m));
    sal_memset(&epe_nh_ctl,  0, sizeof(EpeNextHopCtl_m));
    sal_memset(&flow_ctl,    0, sizeof(FlowTcamLookupCtl_m));

    /* IpeAclQosCtl */
    cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe));

    for(index = 0; index < 192; index++)
    {
        SetIpeAclQosCtl(V, array_0_clearException_f + index, &ipe, 1); /*clear Exception*/
    }
    /**< [TM] START */
    SetIpeAclQosCtl(V, oamObeyAclQos_f, &ipe, 0xFFFF);
    SetIpeAclQosCtl(V, forceBackClearIngressAction_f, &ipe, 1);
    SetIpeAclQosCtl(V, forceBackClearSrcCidValidEn_f, &ipe, 1);
    SetIpeAclQosCtl(V, vplsDecapsPktForwardingTypeMode_f, &ipe, 1);
    SetIpeAclQosCtl(V, aclRedirectClearAclEditMode_f, &ipe, 0);
    SetIpeAclQosCtl(V, aclSelInvalidDestMapMode_f, &ipe, 0);
    SetIpeAclQosCtl(V, aclUseInvalidDestMap_f, &ipe, 0x3FFFF);
    SetIpeAclQosCtl(V, bypassPolicingForAclDiscard_f, &ipe, 1);
    SetIpeAclQosCtl(V, bypassStormCtlForAclDiscard_f, &ipe, 1);
    SetIpeAclQosCtl(V, dot1AeFieldShareMode_f, &ipe, 0);
    SetIpeAclQosCtl(V, downstreamOamFollowAcl_f, &ipe, 1);
    SetIpeAclQosCtl(V, useGemPortLookup_f, &ipe, p_reg->xgpon_gem_port_en? 1 : 0);
    SetIpeAclQosCtl(V, dscpMergeRedirectMode_f, &ipe, 0);
    SetIpeAclQosCtl(V, vlanActionMergeRedirectMode_f, &ipe, 0);
    SetIpeAclQosCtl(V, vlanEditSelect1_f, &ipe, 0);

    /**< [TM] END */
    SetIpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f, &ipe, 0);
    SetIpeAclQosCtl(V, aclLogSelectMap_1_logBlockNum_f, &ipe, 1);
    SetIpeAclQosCtl(V, aclLogSelectMap_2_logBlockNum_f, &ipe, 2);
    SetIpeAclQosCtl(V, aclLogSelectMap_3_logBlockNum_f, &ipe, 3);
    SetIpeAclQosCtl(V, aclLogSelectMap_4_logBlockNum_f, &ipe, 4);
    SetIpeAclQosCtl(V, aclLogSelectMap_5_logBlockNum_f, &ipe, 5);
    SetIpeAclQosCtl(V, aclLogSelectMap_6_logBlockNum_f, &ipe, 6);
    SetIpeAclQosCtl(V, aclLogSelectMap_7_logBlockNum_f, &ipe, 7);

/* SYSTEM MODIFIED by hansf 2018/11/16: discard packet will do acl process in hybrid550 system*/
#ifdef OFPRODUCT
    SetIpeAclQosCtl(V, discardPacketFollowAcl_f, &ipe, 1);
#else
    SetIpeAclQosCtl(V, discardPacketFollowAcl_f, &ipe, 0);
#endif

/* SYSTEM MODIFIED by hansf for discard packet not do policing */
#ifdef OFPRODUCT
    SetIpeAclQosCtl(V, ppDiscardBypassPolicing_f, &ipe, 0);
#else
    SetIpeAclQosCtl(V, ppDiscardBypassPolicing_f, &ipe, 1);
#endif
    SetIpeAclQosCtl(V, cidPairDirectDenyPolicyIsHighPri_f, &ipe, 1);
    SetIpeAclQosCtl(V, cidPairDirectPermitPolicyIsHighPri_f, &ipe, 1);


    SetIpeAclQosCtl(V, followGreProtocol_f, &ipe, 1);
    /* SYSTEM MOD by wangl 2018/11/19:  in hybrid550 system*/
#ifdef OFPRODUCT
    SetIpeAclQosCtl(V, fatalExceptionBypassRedirect_f, &ipe, 0);
#else
    SetIpeAclQosCtl(V, fatalExceptionBypassRedirect_f, &ipe, 1);
#endif
    /*usw redirect pkt use per entry contrl the pkt do or not do pkt edit
      CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT,  Packet edit will be canceled for redirect packet */
	SetIpeAclQosCtl(V, aclFwdClearEditOperation_f, &ipe, 0);
    SetIpeAclQosCtl(V, doNotSnatMode_f, &ipe, 1); /*clear nat info*/

    cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe));

    /* IpeFwdCtl */
    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

    /*SYSTEM MODIFED by wangl for hybrid550*/
#ifdef OFPRODUCT
    discard_bitmap[0] = 0xFFFFF7BF;
    discard_bitmap[1] = 0xFFFFFFFF;
    SetIpeFwdCtl(A, aclCancelDiscardOpBitmap_f, &ipe_fwd, &discard_bitmap);
    SetIpeFwdCtl(V, oamObeyAclQos_f, &ipe_fwd, 0xFFFF);
#else
    discard_bitmap[0] = 0xFFFFFFFF;
    discard_bitmap[1] = 0xFFFFFFFF;
    SetIpeFwdCtl(A, aclCancelDiscardOpBitmap_f, &ipe_fwd, &discard_bitmap);
    SetIpeFwdCtl(V, oamObeyAclQos_f, &ipe_fwd, 0xFFFF);
#endif
    /* SYSTEM MODIFIED by hansf for hybrid bug 49976, flowmod with buffer id, iloop packet should do acl */
#ifdef OFPRODUCT
    SetIpeFwdCtl(V, downstreamOamFollowAcl_f, &ipe_fwd, 1);
#else
    /* SYSTEM MODIFIED by yangbs for SDK bug 102938 */
    SetIpeFwdCtl(V, downstreamOamFollowAcl_f, &ipe_fwd, 0);
#endif
    
    SetIpeFwdCtl(V, aclSelInvalidDestMapMode_f, &ipe_fwd, 0);
    SetIpeFwdCtl(V, aclUseInvalidDestMap_f, &ipe_fwd, 0x3FFFF);
    SetIpeFwdCtl(V, aclDiscardType_f, &ipe_fwd, 0x2d);
    SetIpeFwdCtl(V, plcDiscardType_f, &ipe_fwd, 0x2c);
    SetIpeFwdCtl(V, bypassStormCtlForAclDiscard_f, &ipe_fwd, 1);
    SetIpeFwdCtl(V, bypassPolicingForAclDiscard_f, &ipe_fwd, 1);

    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd));

    /* IpeFwdAclCtl */
    cmd = DRV_IOR(IpeFwdAclCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_acl));

    SetIpeFwdAclCtl(V, aclDestUseApsGroupId_f, &ipe_fwd_acl, 1);
    SetIpeFwdAclCtl(V, aclDestUseEcmpGroupId_f, &ipe_fwd_acl, 1);

     /*global control  acl lkup type*/
    SetIpeFwdAclCtl(V, aclIgnorIpHeaderError_f, &ipe_fwd_acl, 1);
    SetIpeFwdAclCtl(V, aclUnknownL3TypeForceToMacKey_f, &ipe_fwd_acl, 1);
    value = 0xFFFF;
    CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IP);
    CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IPV4);
    CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IPV6);
    SetIpeFwdAclCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &ipe_fwd_acl, value);
    SetIpeFwdAclCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &ipe_fwd_acl, 1);
    value = 0;
    CTC_BIT_SET(value, CTC_PARSER_L3_TYPE_IPV6);
    SetIpeFwdAclCtl(V, useCopp640bitsKey_f, &ipe_fwd_acl, value);
    SetIpeFwdAclCtl(V, aclUdfForceKeyMode_f, &ipe_fwd_acl, 0);
    value = 1 << 10; /* TCAMKEYTYPE_FORWARDKEY_640's bit set to 1*/
    SetIpeFwdAclCtl(V, aclUdfForceKeyPerLkpType_f, &ipe_fwd_acl, value);
    SetIpeFwdAclCtl(V, aclUdfForceKeyPerUdfIdx_f, &ipe_fwd_acl, 0xFFFF);
    SetIpeFwdAclCtl(V, aclUdfForcetoFwdExtKeyValid_f, &ipe_fwd_acl, 0xFF);

    /* acl redirect action will force stacking do egress chip edit */
    SetIpeFwdAclCtl(V, aclUseBypassIngressEdit_f, &ipe_fwd_acl, 1);

      /* Cancel ACL Tcam Lookup */
    SetIpeFwdAclCtl(V, enableFlowHashDisAclOp_f, &ipe_fwd_acl, 0x00FF);

    cmd = DRV_IOW(IpeFwdAclCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_acl));

    /* IpePreLookupAclCtl */
    cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre));

    SetIpePreLookupAclCtl(V, serviceAclQosEn_f, &ipe_pre, p_reg->ingress_service_acl_en? 1 : 0);
    SetIpePreLookupAclCtl(V, layer2TypeUsedAsVlanNum_f,  &ipe_pre, 1); /*default use vlan num */
    SetIpePreLookupAclCtl(V, brgPktVlanCidIsHigherPri_f,  &ipe_pre, 0);

    SetIpePreLookupAclCtl(V, routePacketObeyStp_f,          &ipe_pre, 1);  /*obey stp block*/
    SetIpePreLookupAclCtl(V, bridgePktAclObeyStpBlock_f,    &ipe_pre, 1);
    SetIpePreLookupAclCtl(V, nonBridgePktAclObeyStpBlock_f, &ipe_pre, 1);

    SetIpePreLookupAclCtl(V, vxlanFlowKeyL4UseOuter_f,  &ipe_pre, 1);     /* Aware Tunnel Info En*/
    SetIpePreLookupAclCtl(V, greFlowKeyL4UseOuter_f,    &ipe_pre, 1);
    SetIpePreLookupAclCtl(V, capwapFlowKeyL4UseOuter_f, &ipe_pre, 1);

	SetIpePreLookupAclCtl(V, brgPktVlanCidIsHigherPri_f, &ipe_pre, 0);
    SetIpePreLookupAclCtl(V, nonBrgPktPortCidPri_f,   &ipe_pre, 3);
    SetIpePreLookupAclCtl(V, nonBrgPktVlanCidPri_f,   &ipe_pre, 1);
    SetIpePreLookupAclCtl(V, nonBrgPktL3IntfCidPri_f, &ipe_pre, 2);
	SetIpePreLookupAclCtl(V, aclVlanRangeEn_f, &ipe_pre, 1);
    cmd = DRV_IOW(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre));

    field = 1;
	cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_flowHashVlanRangeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

	/*IpeLearningCtl*/
    field = 1;  /*macsaPri < ipsa */
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_dsMacSaCidIsLowerPri_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    field = 0;  /*macDa > ipDa */
    cmd = DRV_IOW(IpeBridgeCtl_t, IpeBridgeCtl_dsMacDaCidIsLowerPri_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    /* SYSTEM MODIFIED by hansf for discard packet not do policing */
    /*EpeClassficationCtl*/
    field = 1;
    cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_bypassPolicingForAclDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    field = 1;
    cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_ppDiscardBypassPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    /*EpePktProcCtl*/
    field = 1;  /*default use vlan num */
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_layer2TypeUsedAsVlanNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    /* EpeAclQosCtl */
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));

    for(index = 0; index < 32; index++)
    {
        SetEpeAclQosCtl(V, gException_0_overrideException_f + index, &epe_acl_ctl, 1); /*clear Exception*/
    }
    SetEpeAclQosCtl(V, aclIgnorIpHeaderError_f, &epe_acl_ctl, 1);
	SetEpeAclQosCtl(V, aclUnknownL3TypeForceToMacKey_f, &epe_acl_ctl, 1);
	value = 0xFFFF;
	CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IP);
	CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IPV4);
	CTC_BIT_UNSET(value, CTC_PARSER_L3_TYPE_IPV6);
    SetEpeAclQosCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &epe_acl_ctl, value);
    SetEpeAclQosCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &epe_acl_ctl, 1);
    SetEpeAclQosCtl(V, aclUdfForceKeyMode_f, &epe_acl_ctl, 0);
    value = 1 << 10; /* TCAMKEYTYPE_FORWARDKEY_640's bit set to 1*/
    SetEpeAclQosCtl(V, aclUdfForceKeyPerLkpType_f, &epe_acl_ctl, value);
    SetEpeAclQosCtl(V, aclUdfForceKeyPerUdfIdx_f, &epe_acl_ctl, 0xFFFF);
    SetEpeAclQosCtl(V, aclUdfForcetoFwdExtKeyValid_f, &epe_acl_ctl, 0xFF);
    /*SYSTEM MODIFIED for hybrid discard packet not do acl process*/
#ifdef OFPRODUCT
    SetEpeAclQosCtl(V, discardPacketFollowAcl_f, &epe_acl_ctl, 0);
#else
    SetEpeAclQosCtl(V, discardPacketFollowAcl_f, &epe_acl_ctl, 1);
#endif
    SetEpeAclQosCtl(V, serviceAclQosEn_f, &epe_acl_ctl, p_reg->egress_service_acl_en? 1: 0);
    SetEpeAclQosCtl(V, oamObeyAclQos_f, &epe_acl_ctl, 0xFFFF);
    SetEpeAclQosCtl(V, downstreamOamFollowAcl_f, &epe_acl_ctl, 1);
    SetEpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f, &epe_acl_ctl, 0);
    SetEpeAclQosCtl(V, aclLogSelectMap_1_logBlockNum_f, &epe_acl_ctl, 1);
    SetEpeAclQosCtl(V, aclLogSelectMap_2_logBlockNum_f, &epe_acl_ctl, 2);
    SetEpeAclQosCtl(V, aclVlanRangeEn_f, &epe_acl_ctl, 1);

    /**< [TM] init TPID */
    SetEpeAclQosCtl(V, cvlanTpid_f, &epe_acl_ctl, 0x8100);
    SetEpeAclQosCtl(V, svlanTpidArray_0_svlanTpid_f, &epe_acl_ctl, 0x8100);
    SetEpeAclQosCtl(V, svlanTpidArray_1_svlanTpid_f, &epe_acl_ctl, 0x9100);
    SetEpeAclQosCtl(V, svlanTpidArray_2_svlanTpid_f, &epe_acl_ctl, 0x88A8);
    SetEpeAclQosCtl(V, svlanTpidArray_3_svlanTpid_f, &epe_acl_ctl, 0x88A8);

	step = EpeAclQosCtl_gAclLevel_1_brgPktVlanAclIsHigherPri_f - EpeAclQosCtl_gAclLevel_0_brgPktVlanAclIsHigherPri_f;
    for(index = 0; index < ACL_EGS_BLOCK_MAX_NUM; index++)
    {
        SetEpeAclQosCtl(V, gAclLevel_0_brgPktVlanAclIsHigherPri_f + step * index, &epe_acl_ctl, 0);
        SetEpeAclQosCtl(V, gAclLevel_0_l3IntfAclPri_f + step * index, &epe_acl_ctl, 2);
        SetEpeAclQosCtl(V, gAclLevel_0_portAclPri_f + step * index, &epe_acl_ctl, 3);
        SetEpeAclQosCtl(V, gAclLevel_0_vlanAclPri_f + step * index, &epe_acl_ctl, 1);
    }


    cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));

    /*IpeLookupCtl*/
    field = 1;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_flowHashBackToL3V6Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    /*IpeIntfMapperCtl*/
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_ctl));
    step = IpeIntfMapperCtl_gAclLevel_1_brgPktVlanAclIsHigherPri_f - IpeIntfMapperCtl_gAclLevel_0_brgPktVlanAclIsHigherPri_f;
    for(index = 0; index < ACL_IGS_BLOCK_MAX_NUM; index++)
    {
        SetIpeIntfMapperCtl(V, gAclLevel_0_brgPktVlanAclIsHigherPri_f + step * index, &ipe_intf_ctl, 0);
        SetIpeIntfMapperCtl(V, gAclLevel_0_nonBrgPktL3IntfAclPri_f    + step * index, &ipe_intf_ctl, 2);
        SetIpeIntfMapperCtl(V, gAclLevel_0_nonBrgPktPortAclPri_f      + step * index, &ipe_intf_ctl, 3);
        SetIpeIntfMapperCtl(V, gAclLevel_0_nonBrgPktVlanAclPri_f      + step * index, &ipe_intf_ctl, 1);
    }
    /*bypassPortAclAndIpfix = (PacketInfo.bypassAll && IpeIntfMapperCtl.bypassPktDoAclAndIpfixCtl(0))
                              || (PacketInfo.discard && IpeIntfMapperCtl.bypassPktDoAclAndIpfixCtl(1))*/
    SetIpeIntfMapperCtl(V, bypassPktDoAclAndIpfixCtl_f, &ipe_intf_ctl, 1);
    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_ctl));

    /*EpeNextHopCtl*/
    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));
    step = EpeNextHopCtl_gAclLevel_1_brgPktVlanAclIsHigherPri_f - EpeNextHopCtl_gAclLevel_0_brgPktVlanAclIsHigherPri_f;
    for(index = 0; index < ACL_EGS_BLOCK_MAX_NUM; index++)
    {
        SetEpeNextHopCtl(V, gAclLevel_0_brgPktVlanAclIsHigherPri_f + step * index, &epe_nh_ctl, 0);
        SetEpeNextHopCtl(V, gAclLevel_0_nonBrgPktL3IntfAclPri_f    + step * index, &epe_nh_ctl, 2);
        SetEpeNextHopCtl(V, gAclLevel_0_nonBrgPktPortAclPri_f      + step * index, &epe_nh_ctl, 3);
        SetEpeNextHopCtl(V, gAclLevel_0_nonBrgPktVlanAclPri_f      + step * index, &epe_nh_ctl, 1);
    }
	SetEpeNextHopCtl(V, mirrorPktStripCidHeader_f, &epe_nh_ctl, 1);
    SetEpeNextHopCtl(V, bypassInsertCidThreValid_f, &epe_nh_ctl, 1);
    SetEpeNextHopCtl(V, bypassInsertCidThre_f, &epe_nh_ctl, 255);
    SetEpeNextHopCtl(V, bypassDisableInsertCidTagBitmap_f, &epe_nh_ctl, 0x2);/*4'b0010*/
    SetEpeNextHopCtl(V, bypassPktDoAclCtl_f, &epe_nh_ctl, 0x1B);
    SetEpeNextHopCtl(V, aclCareVlanXlateLkpStatus_f, &epe_nh_ctl, 1);

    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));

     /*FlowTcamLookupCtl*/

    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_ctl));

     /*AclQosMacL3Key320 Arp.senderMac share macSa*/
    /*SYSTEM MODIFIED for hybrid: not use senderMac as macsa */
     /*SYSTEM MODIFIED by yejl for bug 53860: not use senderMac as macsa */
    SetFlowTcamLookupCtl(V, aclMacL3BasicKeyArpPktUseSenderMac_f, &flow_ctl, 0);
    SetFlowTcamLookupCtl(V, aclMacL3BasicKeyArpPktUseTargetMac_f, &flow_ctl, 0);

    value = 0xFF;
    SetFlowTcamLookupCtl(V, forceForwardingAclKeyUseNsh_f, &flow_ctl, value);
    SetFlowTcamLookupCtl(V, forwardingAclKeyUnion1Type_f, &flow_ctl, value);

     /*AclQosMacL3Key640 Arp.senderMac share macSa*/
    SetFlowTcamLookupCtl(V, aclMacL3ExtKeyArpPktUseSenderMac_f, &flow_ctl, 1);
     /*AclQosMacL3Key640 Arp.senderMac share macDa*/
    SetFlowTcamLookupCtl(V, aclMacL3ExtKeyArpPktUseTargetMac_f, &flow_ctl, 1);

     /*AclQosCoppKey320 Arp.senderMac share macDa*/
    SetFlowTcamLookupCtl(V, coppBasicKeyDisableArpShareMacDa_f, &flow_ctl, 0);
	SetFlowTcamLookupCtl(V, gratuitousArpOpcodeCheckEn_f, &flow_ctl, 1);
	/*UDF field can share ipv6sa*/
    SetFlowTcamLookupCtl(V,coppExtKeyUdfShareType_f, &flow_ctl, 0xFFFF);


    /*DsAclQosL3Key160.u1.g1.shareFields used TCP Flag and tcp en*/
    step = FlowTcamLookupCtl_gIngrAcl_1_l3Key160ShareFieldType_f - FlowTcamLookupCtl_gIngrAcl_0_l3Key160ShareFieldType_f;
    for(index = 0; index < ACL_IGS_BLOCK_MAX_NUM; index++)
    {
        SetFlowTcamLookupCtl(V, gIngrAcl_0_l3Key160ShareFieldType_f + step * index, &flow_ctl, 2);
    }
    step = FlowTcamLookupCtl_gEgrAcl_1_l3Key160ShareFieldType_f - FlowTcamLookupCtl_gEgrAcl_0_l3Key160ShareFieldType_f;
    for(index = 0; index < ACL_EGS_BLOCK_MAX_NUM; index++)
    {
        SetFlowTcamLookupCtl(V, gEgrAcl_0_l3Key160ShareFieldType_f + step * index, &flow_ctl, 2);
    }
        /*etherType used as layer4Type + layer3Type + cEtherType*/
    SetFlowTcamLookupCtl(V, coppBasicKeyEtherTypeShareType_f, &flow_ctl, 1);
    SetFlowTcamLookupCtl(V, coppExtKeyEtherTypeShareType_f, &flow_ctl, 1);
    SetFlowTcamLookupCtl(V, ingressAclUseVlanRange_f, &flow_ctl, 0xFF);
    SetFlowTcamLookupCtl(V, egressAclUseVlanRange_f, &flow_ctl, 0x7);

    SetFlowTcamLookupCtl(V, forwardingBasicKeyL2TypeMode_f, &flow_ctl, 0);      /* default use l2type as interface id high bit */
    step = FlowTcamLookupCtl_gIngrAcl_1_aclMacL3BasicKeyMergeDataShareMode_f - FlowTcamLookupCtl_gIngrAcl_0_aclMacL3BasicKeyMergeDataShareMode_f;
    for (index=0; index<ACL_IGS_BLOCK_MAX_NUM; index+=1)
    {
        SetFlowTcamLookupCtl(V, gIngrAcl_0_aclMacL3BasicKeyMergeDataShareMode_f + index * step, &flow_ctl, 0);       /* default use mac da */
    }
    SetFlowTcamLookupCtl(V, macKeyIsDecapFieldMode_f, &flow_ctl, 0);        /* default egress use isDecap as isDot1Ae */

    SetFlowTcamLookupCtl(V, coppKeyIpAddrShareType0_f, &flow_ctl, 1);
    SetFlowTcamLookupCtl(V, coppKeyIpAddrShareType1_f, &flow_ctl, 0xFFFF);
    sal_memset(value_a, 0xFF, sizeof(value_a));
    SetFlowTcamLookupCtl(A, ipAddrShareType2_f, &flow_ctl, value_a);
    cmd = DRV_IOW(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_ctl));




    sal_memset(&ipe_fwd_cid, 0, sizeof(IpeFwdCategoryCtl_m));
    /* IpeFwdCategoryCtl */
    cmd = DRV_IOR(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_cid));

    step = IpeFwdCategoryCtl_gSrcCidPri_1_pktSrcCategoryIdPri_f -
                      IpeFwdCategoryCtl_gSrcCidPri_0_pktSrcCategoryIdPri_f;
    for(loop = 0; loop < 6 ;loop++)
    {
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_pktSrcCategoryIdPri_f + step *loop,     &ipe_fwd_cid, 7);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_i2eSrcCategoryIdPri_f + step *loop,     &ipe_fwd_cid, 6);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_globalSrcCategoryIdPri_f + step *loop,  &ipe_fwd_cid, 5);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_staticSrcCategoryIdPri_f + step *loop,  &ipe_fwd_cid, 4);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_dynamicSrcCategoryIdPri_f + step *loop, &ipe_fwd_cid, 3);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_ifSrcCategoryIdPri_f + step *loop,      &ipe_fwd_cid, 2);
         SetIpeFwdCategoryCtl(V, gSrcCidPri_0_defaultSrcCategoryIdPri_f+ step *loop, &ipe_fwd_cid, 1);
    }
    SetIpeFwdCategoryCtl(V, gDstCidPri_dynamicDstCategoryIdPri_f, &ipe_fwd_cid, 3);
    SetIpeFwdCategoryCtl(V, gDstCidPri_dsflowDstCategoryIdPri_f,  &ipe_fwd_cid, 1);
    SetIpeFwdCategoryCtl(V, gDstCidPri_staticDstCategoryIdPri_f,  &ipe_fwd_cid, 1);
    SetIpeFwdCategoryCtl(V, gDstCidPri_defaultDstCategoryIdPri_f, &ipe_fwd_cid, 0);

    SetIpeFwdCategoryCtl(V, defaultDstCategoryIdValid_f, &ipe_fwd_cid, 1);
    SetIpeFwdCategoryCtl(V, defaultDstCategoryId_f,&ipe_fwd_cid, CTC_ACL_DEFAULT_CID);
    SetIpeFwdCategoryCtl(V, defaultSrcCategoryIdValid_f, &ipe_fwd_cid, 1);
    SetIpeFwdCategoryCtl(V, defaultSrcCategoryId_f,&ipe_fwd_cid, CTC_ACL_DEFAULT_CID);
    SetIpeFwdCategoryCtl(V, disableCheckI2eCidValue_f,&ipe_fwd_cid, 0);
	SetIpeFwdCategoryCtl(V, categoryIdPairLookupEn_f,&ipe_fwd_cid, 0);
	SetIpeFwdCategoryCtl(V, categoryIdClassifyEn_f,   &ipe_fwd_cid, 1);

    cmd = DRV_IOW(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_cid));


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_build_egress_vlan_edit(uint8 lchip, void* p_ds_edit, sys_acl_vlan_edit_t* p_vlan_edit, uint8 is_add)
{
    uint8 op = 0;
    uint8 mo = 0;
    uint8 tmp_prioRed = 0;
    uint8 tmp_vlanActionProfileId = 0;
    uint8 tmp_mutationTableId = 0;

    _sys_usw_acl_vlan_tag_op_translate(lchip, p_vlan_edit->stag_op, &op, &mo);
    tmp_prioRed |= op & 0x3;                /*sTagAction*/
    tmp_prioRed |= (mo<<2) & 0x4;        /*stagModifyMode*/
    _sys_usw_acl_vlan_tag_op_translate(lchip, p_vlan_edit->ctag_op, &op, &mo);
    tmp_vlanActionProfileId |= (op<<6) & 0xC0;      /*cTagAction*/
    tmp_prioRed |= (mo<<3) & 0x8;                      /*ctagModifyMode*/

    SetDsAcl(V, mutationTableType_f, p_ds_edit, p_vlan_edit->svid_sl);      /*sVlanIdAction*/
    tmp_mutationTableId |= (p_vlan_edit->scos_sl<<2) & 0xC;     /*sCosAction*/
    tmp_mutationTableId |= p_vlan_edit->scfi_sl & 0x3;              /*sCfiAction*/

    tmp_vlanActionProfileId |= (p_vlan_edit->cvid_sl<<4) & 0x30;    /*cVlanIdAction*/
    tmp_vlanActionProfileId |= (p_vlan_edit->ccos_sl<<2) & 0xC;     /*cCosAction*/
    tmp_vlanActionProfileId |= p_vlan_edit->ccfi_sl & 0x3;              /*cCfiAction*/

    SetDsAcl(V, prioRed_f, p_ds_edit, is_add? tmp_prioRed: 0);
    SetDsAcl(V, vlanActionProfileId_f, p_ds_edit, is_add? tmp_vlanActionProfileId: 0);
    SetDsAcl(V, mutationTableId_f, p_ds_edit, is_add? tmp_mutationTableId: 0);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_vlan_tag_op_translate_reverse(uint8 lchip, uint8 op_in, uint8 mode_in, uint8* p_op_out)
{
    uint8 op_out = 0;
    uint8 temp_mode = 0;

    CTC_PTR_VALID_CHECK(p_op_out);

    temp_mode = (mode_in&0xF) << 4;
    temp_mode |= op_in&0xF;

    switch (temp_mode)
    {
        case 0x00:
            op_out = CTC_ACL_VLAN_TAG_OP_NONE;
            break;
        case 0x11:
            op_out = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
            break;
        case 0x01:
            op_out = CTC_ACL_VLAN_TAG_OP_REP;
            break;
        case 0x02:
            op_out = CTC_ACL_VLAN_TAG_OP_ADD;
            break;
        case 0x03:
            op_out = CTC_ACL_VLAN_TAG_OP_DEL;
            break;
        default:
            break;
    }

    *p_op_out = op_out;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_get_egress_vlan_edit(uint8 lchip, void* p_ds_edit, ctc_acl_vlan_edit_t* p_vlan_edit)
{
    uint8 op = 0;
    uint8 mo;
    uint8 tmp_prioRed = 0;
    uint8 tmp_vlanActionProfileId = 0;
    uint8 tmp_mutationTableId = 0;
    uint8 temp_op = 0;

    CTC_PTR_VALID_CHECK(p_ds_edit);
    CTC_PTR_VALID_CHECK(p_vlan_edit);

    tmp_prioRed = GetDsAcl(V, prioRed_f, p_ds_edit);
    tmp_vlanActionProfileId = GetDsAcl(V, vlanActionProfileId_f, p_ds_edit);
    tmp_mutationTableId = GetDsAcl(V, mutationTableId_f, p_ds_edit);

    op = tmp_prioRed & 0x3;
    mo = (tmp_prioRed>>2) & 0x1;
    _sys_usw_acl_vlan_tag_op_translate_reverse(lchip, op, mo, &temp_op);
    p_vlan_edit->stag_op = temp_op;

    op = (tmp_vlanActionProfileId>>6) & 0x3;
    mo = (tmp_prioRed>>3) & 0x1;
    _sys_usw_acl_vlan_tag_op_translate_reverse(lchip, op, mo, &temp_op);
    p_vlan_edit->ctag_op = temp_op;

    p_vlan_edit->svid_sl = GetDsAcl(V, mutationTableType_f, p_ds_edit);
    p_vlan_edit->scos_sl = (tmp_mutationTableId>>2) & 0x3;
    p_vlan_edit->scfi_sl = tmp_mutationTableId & 0x3;

    p_vlan_edit->cvid_sl = (tmp_vlanActionProfileId>>4) & 0x3;
    p_vlan_edit->ccos_sl = (tmp_vlanActionProfileId>>2) & 0x3;
    p_vlan_edit->ccfi_sl = tmp_vlanActionProfileId & 0x3;

    return CTC_E_NONE;
}


int32
_sys_usw_acl_build_vlan_edit(uint8 lchip, void* p_ds_edit, sys_acl_vlan_edit_t* p_vlan_edit)
{
    uint8 op = 0;
    uint8 mo = 0;

    CTC_PTR_VALID_CHECK(p_ds_edit);
    CTC_PTR_VALID_CHECK(p_vlan_edit);

    _sys_usw_acl_vlan_tag_op_translate(lchip, p_vlan_edit->stag_op, &op, &mo);
    SetDsAclVlanActionProfile(V, sTagAction_f, p_ds_edit, op);
    SetDsAclVlanActionProfile(V, stagModifyMode_f, p_ds_edit, mo);
    _sys_usw_acl_vlan_tag_op_translate(lchip, p_vlan_edit->ctag_op, &op, &mo);
    SetDsAclVlanActionProfile(V, cTagAction_f, p_ds_edit, op);
    SetDsAclVlanActionProfile(V, ctagModifyMode_f, p_ds_edit, mo);

    SetDsAclVlanActionProfile(V, sVlanIdAction_f, p_ds_edit, p_vlan_edit->svid_sl);
    SetDsAclVlanActionProfile(V, sCosAction_f, p_ds_edit, p_vlan_edit->scos_sl);
    SetDsAclVlanActionProfile(V, sCfiAction_f, p_ds_edit, p_vlan_edit->scfi_sl);

    SetDsAclVlanActionProfile(V, cVlanIdAction_f, p_ds_edit, p_vlan_edit->cvid_sl);
    SetDsAclVlanActionProfile(V, cCosAction_f, p_ds_edit, p_vlan_edit->ccos_sl);
    SetDsAclVlanActionProfile(V, cCfiAction_f, p_ds_edit, p_vlan_edit->ccfi_sl);
    SetDsAclVlanActionProfile(V, svlanTpidIndexEn_f, p_ds_edit, 1);
    SetDsAclVlanActionProfile(V, svlanTpidIndex_f, p_ds_edit, p_vlan_edit->tpid_index);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_install_vlan_edit(uint8 lchip, sys_acl_vlan_edit_t* p_vlan_edit, uint16* p_prof_idx)
{
    uint32 cmd = 0;
    sys_acl_vlan_edit_t p_vlan_edit_new;

    DsAclVlanActionProfile_m ds_edit;

    sal_memset(&ds_edit, 0, sizeof(ds_edit));
    sal_memset(&p_vlan_edit_new, 0, sizeof(sys_acl_vlan_edit_t));

    sal_memcpy(&p_vlan_edit_new, p_vlan_edit, sizeof(sys_acl_vlan_edit_t));

    p_vlan_edit_new.profile_id = *p_prof_idx;
    CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_acl_master[lchip]->vlan_edit_spool, &p_vlan_edit_new))

    _sys_usw_acl_build_vlan_edit(lchip, &ds_edit, p_vlan_edit);
    cmd = DRV_IOW(DsAclVlanActionProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_vlan_edit_new.profile_id, cmd, &ds_edit));

    *p_prof_idx = *p_prof_idx + 1;

    return CTC_E_NONE;
}

int32
_sys_usw_acl_vlan_tag_op_translate(uint8 lchip, uint8 op_in, uint8* o_op, uint8* o_mode)
{
    uint8 op   = 0;
    uint8 mode = 0;

    switch (op_in)
    {
    case CTC_ACL_VLAN_TAG_OP_NONE:
        op = 0;
        break;
    case CTC_ACL_VLAN_TAG_OP_REP_OR_ADD:
        mode = 1;
        op   = 1;
        break;
    case CTC_ACL_VLAN_TAG_OP_REP:
        op = 1;
        break;
    case CTC_ACL_VLAN_TAG_OP_ADD:
        op = 2;
        break;
    case CTC_ACL_VLAN_TAG_OP_DEL:
        op = 3;
        break;
    default:
        break;
    }

    if(o_op)
    {
        *o_op = op;
    }
    if(o_mode)
    {
        *o_mode = mode;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_build_port_info(uint8 lchip, uint8 key_type, ctc_field_key_t* key_field, drv_acl_port_info_t* pport_drv)
{
    uint16 bitmap_16    = 0;

    uint8  i            = 0;
    uint32 bitmap_64[2] = { 0 };
    uint8  bitmap_status =0;
    ctc_field_port_t* p_field_port = (ctc_field_port_t*)key_field->ext_data;
    ctc_field_port_t* p_field_port_mask = (ctc_field_port_t*)key_field->ext_mask;

    CTC_PTR_VALID_CHECK(key_field->ext_data);
  /*   CTC_PTR_VALID_CHECK(key_field->ext_mask);*/

    if((CTC_ACL_KEY_COPP != key_type) && (CTC_ACL_KEY_COPP_EXT != key_type) && (CTC_ACL_KEY_UDF != key_type))
    {
        bitmap_status = ACL_BITMAP_STATUS_16;
    }
    else
    {
        bitmap_status = ACL_BITMAP_STATUS_64;
    }

    switch(p_field_port->type)
    {
    case CTC_FIELD_PORT_TYPE_PORT_CLASS:
        pport_drv->class_id_data = p_field_port->port_class_id;
        pport_drv->class_id_mask = p_field_port_mask->port_class_id;
        break;
    case CTC_FIELD_PORT_TYPE_VLAN_CLASS:
        pport_drv->class_id_data = p_field_port->vlan_class_id;
        pport_drv->class_id_mask = p_field_port_mask->vlan_class_id;
        break;
    case CTC_FIELD_PORT_TYPE_L3IF_CLASS:
        pport_drv->class_id_data = p_field_port->l3if_class_id;
        pport_drv->class_id_mask = p_field_port_mask->l3if_class_id;
        break;
    case CTC_FIELD_PORT_TYPE_LOGIC_PORT:
        pport_drv->gport      = p_field_port->logic_port;
        pport_drv->gport_mask = p_field_port_mask->logic_port;

        pport_drv->gport_type      = DRV_FLOWPORTTYPE_LPORT;
        pport_drv->gport_type_mask = 3;
        break;
    case CTC_FIELD_PORT_TYPE_GPORT:
        pport_drv->gport      = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_field_port->gport);
        pport_drv->gport_mask = p_field_port_mask->gport;

        pport_drv->gport_type      = DRV_FLOWPORTTYPE_GPORT;
        pport_drv->gport_type_mask = 3;
        break;
    case CTC_FIELD_PORT_TYPE_PORT_BITMAP:
         /*
         *  example: user care port 00001111 (that is port0 port1 port2 port3).
         *  In hardware,  should be 00001111 - data
         *                          11110000 - mask.
         *  so port0 port1 port2 port3 will hit, other ports won't.
        summary:  mask = ~data;
         portBitmap(15,0) = CBit(16, 'd', "0", 1);
        (DsAclQosL3Key160.globalPort(12,0),DsAclQosL3Key160.portBitmap(2,0)) = portBitmap(15,0);
        DsAclQosL3Key160.globalPort(15,13) = portBitmapBase(2,0);
        */
        if (ACL_BITMAP_STATUS_64 == bitmap_status)
        {
            if (p_field_port->port_bitmap[0] || p_field_port->port_bitmap[1])
            {
                bitmap_64[0]        = p_field_port->port_bitmap[0];
                bitmap_64[1]        = p_field_port->port_bitmap[1];
                pport_drv->bitmap_base = 0;
                pport_drv->bitmap_base_mask = 1;
            }
            else if (p_field_port->port_bitmap[2] || p_field_port->port_bitmap[3])
            {
                bitmap_64[0]        = p_field_port->port_bitmap[2];
                bitmap_64[1]        = p_field_port->port_bitmap[3];
                pport_drv->bitmap_base = 1;
                pport_drv->bitmap_base_mask = 1;
            }
            pport_drv->bitmap_64[0] = bitmap_64[0];
            pport_drv->bitmap_64[1] = bitmap_64[1];
            pport_drv->bitmap_64_mask[0] = ~bitmap_64[0];
            pport_drv->bitmap_64_mask[1] = ~bitmap_64[1];
        }
        else if (ACL_BITMAP_STATUS_16 == bitmap_status)
        {
            for (i = 0; i < 4; i++)
            {
                if ((bitmap_16 = (p_field_port->port_bitmap[i] & 0xFFFF)))
                {
                    pport_drv->bitmap_base = 0 + (2 * i);
                    break;
                }
                else if ((bitmap_16 = (p_field_port->port_bitmap[i] >> 16)))
                {
                    pport_drv->bitmap_base = 1 + (2 * i);
                    break;
                }
            }

            pport_drv->gport      = (pport_drv->bitmap_base << 13) + (bitmap_16 >> 3);
            pport_drv->gport_mask = (0x7 << 13) | (~bitmap_16 >> 3);

            pport_drv->bitmap = bitmap_16 & 0x7;
            pport_drv->bitmap_mask = (~bitmap_16) & 0x7;
        }
        pport_drv->gport_type      = DRV_FLOWPORTTYPE_BITMAP;
        pport_drv->gport_type_mask = 3;
        break;
    case CTC_FIELD_PORT_TYPE_PBR_CLASS:
        pport_drv->class_id_data = p_field_port->pbr_class_id;
        pport_drv->class_id_mask = p_field_port_mask->pbr_class_id;
        break;
    case CTC_FIELD_PORT_TYPE_NONE:
        pport_drv->class_id_mask = 0;
        pport_drv->gport_mask = 0;
        pport_drv->gport_type_mask = 0;
        pport_drv->bitmap_base_mask = 0;
        pport_drv->bitmap_mask = 0;
        pport_drv->bitmap_64_mask[0] =0;
        pport_drv->bitmap_64_mask[1] =0;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_get_destmap_from_dsfwd(uint8 lchip, uint32 fwd_offset, uint32* o_destmap)
{
    uint32 cmd = 0;
    uint32 dest_map = 0;

    DsFwdDualHalf_m fwd_dual_half;
    DsFwdHalf_m     fwd_half;

    sal_memset(&fwd_dual_half, 0, sizeof(DsFwdDualHalf_m));
    sal_memset(&fwd_half,      0, sizeof(DsFwdHalf_m));

    cmd = DRV_IOR(DsFwdDualHalf_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, fwd_offset/2, cmd, &fwd_dual_half));

    if(0 == fwd_offset%2)
    {
        dest_map = GetDsFwd(V, destMap_f, &fwd_dual_half);
    }
    else
    {
        GetDsFwdDualHalf(A, g_1_dsFwdHalf_f, &fwd_dual_half, &fwd_half);
        dest_map = GetDsFwdHalf(V, destMap_f, &fwd_half);
    }

    if(o_destmap)
    {
        *o_destmap = dest_map;
    }

    return CTC_E_NONE;
}

#define _ACL_FIELD_MAP_
int32
_sys_usw_acl_map_vlan_range(uint8 lchip, ctc_field_key_t* key_field, uint8 dir, uint8 is_svlan, uint32* o_vlan)
{
    uint32 min  = 0;
    uint32 max  = 0;
    uint16 vlan_end = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t      vlan_range;

    CTC_PTR_VALID_CHECK(key_field->ext_data);

    min = key_field->data;
    max = key_field->mask;

    CTC_MAX_VALUE_CHECK(min, max);
    SYS_ACL_VLAN_ID_CHECK(max);

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = *((uint32*)(key_field->ext_data));
    vlan_range.is_acl = 1;
    vlan_range.vlan_start = min;
    vlan_range.vlan_end = max;
    CTC_ERROR_RETURN(sys_usw_vlan_get_maxvid_from_vrange(lchip, &vrange_info, &vlan_range, is_svlan, &vlan_end));

    if(o_vlan)
    {
        *o_vlan = vlan_end;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_acl_map_fwd_type(uint8 pkt_fwd_type_api, uint8* o_pkt_fwd_type)
{
    uint8 pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_NONE;

    switch (pkt_fwd_type_api)
    {
        case CTC_ACL_PKT_FWD_TYPE_IPUC:
            pkt_fwd_type = 2;
            break;
        case CTC_ACL_PKT_FWD_TYPE_IPMC:
            pkt_fwd_type = 3;
            break;
        case CTC_ACL_PKT_FWD_TYPE_L2UC:
            pkt_fwd_type = 4;
            break;
        case CTC_ACL_PKT_FWD_TYPE_L2MC:
            pkt_fwd_type = 5;
            break;
        case CTC_ACL_PKT_FWD_TYPE_BC:
            pkt_fwd_type = 6;
            break;
        case CTC_ACL_PKT_FWD_TYPE_MPLS:
            pkt_fwd_type = 8;
            break;
        case CTC_ACL_PKT_FWD_TYPE_VPLS:
            pkt_fwd_type = 9;
            break;
        case CTC_ACL_PKT_FWD_TYPE_L3VPN:
            pkt_fwd_type = 10;
            break;
        case CTC_ACL_PKT_FWD_TYPE_VPWS:
            pkt_fwd_type = 11;
            break;
        case CTC_ACL_PKT_FWD_TYPE_TRILL_UC:
            pkt_fwd_type = 12;
            break;
        case CTC_ACL_PKT_FWD_TYPE_TRILL_MC:
            pkt_fwd_type = 13;
            break;
        case CTC_ACL_PKT_FWD_TYPE_FCOE:
            pkt_fwd_type = 14;
            break;
        case CTC_ACL_PKT_FWD_TYPE_MPLS_OTHER_VPN:
            pkt_fwd_type = 15;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    if(o_pkt_fwd_type)
    {
        *o_pkt_fwd_type = pkt_fwd_type;
    }
    return CTC_E_NONE;
}
#if 0
int32
_sys_usw_acl_unmap_fwd_type(uint8 pkt_fwd_type_drv, uint8* o_pkt_fwd_type)
{
    uint8 pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_NONE;

    switch (pkt_fwd_type_drv)
    {
        case 2:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_IPUC;
            break;
        case 3:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_IPMC;
            break;
        case 4:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_L2UC;
            break;
        case 5:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_L2MC;
            break;
        case 6:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_BC;
            break;
        case 8:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_MPLS;
            break;
        case 9:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_VPLS;
            break;
        case 10:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_L3VPN;
            break;
        case 11:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_VPWS;
            break;
        case 12:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_TRILL_UC;
            break;
        case 13:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_TRILL_MC;
            break;
        case 14:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_FCOE;
            break;
        case 15:
            pkt_fwd_type = CTC_ACL_PKT_FWD_TYPE_MPLS_OTHER_VPN;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    if(o_pkt_fwd_type)
    {
        *o_pkt_fwd_type = pkt_fwd_type;
    }
    return CTC_E_NONE;
}
#endif
int32
_sys_usw_acl_map_ethertype(uint16 ethertype, uint8* o_l3_type)
{
    /*SYSTEM MODIFIED by yejl for bug 49114, 2018-11-14*/
    uint8 l3_type = CTC_PARSER_L3_TYPE_NONE;
    ParserEthernetCtl_m parser_ethernet_ctl;
    int32 ipv6_parser_enable = 0;
    uint8 lchip = 0;
    uint32 cmd = 0;

    sal_memset(&parser_ethernet_ctl, 0, sizeof(ParserEthernetCtl_m));
    cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_ethernet_ctl));
    ipv6_parser_enable = GetParserEthernetCtl(V , ipv6TypeEn_f, &parser_ethernet_ctl);
    /*end*/

    switch (ethertype)
    {
        case 0x0800:
            l3_type = CTC_PARSER_L3_TYPE_IPV4;
            break;
        case 0x86DD:
			if (ipv6_parser_enable)/*SYSTEM MODIFIED by yejl for bug 49114, 2018-11-14*/
            {
                l3_type = CTC_PARSER_L3_TYPE_IPV6;
            }
            break;
        case 0x8847:
            l3_type = CTC_PARSER_L3_TYPE_MPLS;
            break;

        case 0x8848:
            l3_type = CTC_PARSER_L3_TYPE_MPLS_MCAST;
            break;
#if 0
        case 0x0806:  /*L3_TYPE_ARP may be arp or rarp,so can't mapping */
		case 0x8035:  /*may be arp or rarp,so can't mapping */

#endif
        case 0x8906:
            l3_type = CTC_PARSER_L3_TYPE_FCOE;
            break;
        case 0x22F3:
            l3_type = CTC_PARSER_L3_TYPE_TRILL;
            break;

        case 0x88E7:
            l3_type = CTC_PARSER_L3_TYPE_CMAC;
            break;
        case 0x8902:
            l3_type = CTC_PARSER_L3_TYPE_ETHER_OAM;
            break;
        case 0x8809:
            l3_type = CTC_PARSER_L3_TYPE_SLOW_PROTO;
            break;
        case 0x88F7:
            l3_type = CTC_PARSER_L3_TYPE_PTP;
            break;
       case 0x88E5:
            l3_type = CTC_PARSER_L3_TYPE_DOT1AE;
             break;
	  case 0x88B7:
            l3_type = CTC_PARSER_L3_TYPE_SATPDU;
            break;
        default:
            break;

    }

    if(o_l3_type)
    {
        *o_l3_type = l3_type;
    }

    return CTC_E_NONE;
}
#if 0
int32
_sys_usw_acl_unmap_ethertype(uint8 l3_type, uint16* o_ethertype)
{
    uint16 ethertype = 0;
    switch (l3_type)
    {
        case CTC_PARSER_L3_TYPE_IPV4:
            ethertype = 0x0800;
            break;
        case CTC_PARSER_L3_TYPE_IPV6:
            ethertype = 0x86DD;
            break;
        case CTC_PARSER_L3_TYPE_MPLS:
            ethertype = 0x8847;
            break;
        case CTC_PARSER_L3_TYPE_MPLS_MCAST:
            ethertype = 0x8848;
            break;
#if 0
        case 0x0806:  /*L3_TYPE_ARP may be arp or rarp,so can't mapping */
		case 0x8035:  /*may be arp or rarp,so can't mapping */

#endif
        case CTC_PARSER_L3_TYPE_FCOE:
            ethertype = 0x8906;
            break;
        case CTC_PARSER_L3_TYPE_TRILL:
            ethertype = 0x22F3;
            break;

        case CTC_PARSER_L3_TYPE_CMAC:
            ethertype = 0x88E7;
            break;
        case CTC_PARSER_L3_TYPE_ETHER_OAM:
            ethertype = 0x8902;
            break;
        case CTC_PARSER_L3_TYPE_SLOW_PROTO:
            ethertype = 0x8809;
            break;
        case CTC_PARSER_L3_TYPE_PTP:
            ethertype = 0x88F7;
            break;
       case CTC_PARSER_L3_TYPE_DOT1AE:
            ethertype = 0x88E5;
             break;
	  case CTC_PARSER_L3_TYPE_SATPDU:
            ethertype = 0x88B7;
            break;
        default:
            break;
    }

    if(o_ethertype)
    {
        *o_ethertype = ethertype;
    }
    return CTC_E_NONE;
}
#endif
int32
_sys_usw_acl_map_ip_protocol_to_l4_type(uint8 lchip, uint8 l3_type, uint8 ip_protocol,
                                                   uint32* o_l4_type, uint32* o_mask)
{
    uint8 l4_type = CTC_PARSER_L4_TYPE_NONE;
    uint8 mask    = 0;

    switch(ip_protocol)
    {
        case SYS_L4_PROTOCOL_TCP:
            l4_type = CTC_PARSER_L4_TYPE_TCP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_UDP:
            l4_type = CTC_PARSER_L4_TYPE_UDP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_GRE:
            l4_type = CTC_PARSER_L4_TYPE_GRE;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_IPV4:
            if(CTC_PARSER_L3_TYPE_IPV4 == l3_type)
            {
                l4_type = CTC_PARSER_L4_TYPE_IPINIP;
                mask = 0xF;
            }
            else if(CTC_PARSER_L3_TYPE_IPV6 == l3_type)
            {
                l4_type = CTC_PARSER_L4_TYPE_IPINV6;
                mask = 0xF;
            }
            break;
        case SYS_L4_PROTOCOL_IPV6:
            if(CTC_PARSER_L3_TYPE_IPV4 == l3_type)
            {
                l4_type = CTC_PARSER_L4_TYPE_V6INIP;
                mask = 0xF;
            }
            else if(CTC_PARSER_L3_TYPE_IPV6 == l3_type)
            {
                l4_type = CTC_PARSER_L4_TYPE_V6INV6;
                mask = 0xF;
            }
            break;
        case SYS_L4_PROTOCOL_IPV4_ICMP:
        case SYS_L4_PROTOCOL_IPV6_ICMP:
            l4_type = CTC_PARSER_L4_TYPE_ICMP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_IPV4_IGMP:
            l4_type = CTC_PARSER_L4_TYPE_IGMP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_RDP:
            l4_type = CTC_PARSER_L4_TYPE_RDP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_SCTP:
            l4_type = CTC_PARSER_L4_TYPE_SCTP;
            mask = 0xF;
            break;
        case SYS_L4_PROTOCOL_DCCP:
            l4_type = CTC_PARSER_L4_TYPE_DCCP;
            mask = 0xF;
            break;

        default:
            break;      /*mustn't return, for some key support l4 type*/
    }

    if(o_l4_type)
    {
        *o_l4_type = l4_type;
    }

    if(o_mask)
    {
        *o_mask = mask;
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ip protocol map to l4 type:%u, mask:%X\n", l4_type, mask);
    return CTC_E_NONE;
}

int32
_sys_usw_acl_map_l4_type_to_ip_protocol(uint8 lchip, uint8 l3_type, uint8 l4_type,
                                                   uint32* o_ip_protocol, uint32* o_mask)
{
    uint8 ip_protocol = 0;
    uint8 mask    = 0;

    switch(l4_type)
    {
        case CTC_PARSER_L4_TYPE_TCP:
            ip_protocol = SYS_L4_PROTOCOL_TCP;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_UDP:
            ip_protocol = SYS_L4_PROTOCOL_UDP;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_GRE:
            ip_protocol = SYS_L4_PROTOCOL_GRE;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_IPINIP:
        case CTC_PARSER_L4_TYPE_IPINV6:
            ip_protocol = SYS_L4_PROTOCOL_IPV4;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_V6INIP:
        case CTC_PARSER_L4_TYPE_V6INV6:
            ip_protocol = SYS_L4_PROTOCOL_IPV6;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_ICMP:
            if(CTC_PARSER_L3_TYPE_IPV4 == l3_type)
            {
                ip_protocol = SYS_L4_PROTOCOL_IPV4_ICMP;
                mask = 0xFF;
            }
            else if(CTC_PARSER_L3_TYPE_IPV6 == l3_type)
            {
                ip_protocol = SYS_L4_PROTOCOL_IPV6_ICMP;
                mask = 0xFF;
            }
            break;
        case CTC_PARSER_L4_TYPE_IGMP:
            ip_protocol = SYS_L4_PROTOCOL_IPV4_IGMP;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_RDP:
            ip_protocol = SYS_L4_PROTOCOL_RDP;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_SCTP:
            ip_protocol = SYS_L4_PROTOCOL_SCTP;
            mask = 0xFF;
            break;
        case CTC_PARSER_L4_TYPE_DCCP:
            ip_protocol = SYS_L4_PROTOCOL_DCCP;
            mask = 0xFF;
            break;
        /*CTC_PARSER_L4_TYPE_PBB_ITAG_OAM ; CTC_PARSER_L4_TYPE_ACH_OAM not support */
        default:
            return CTC_E_NOT_SUPPORT;
    }

    if(o_ip_protocol)
    {
        *o_ip_protocol = ip_protocol;
    }

    if(o_mask)
    {
        *o_mask = mask;
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_acl_map_ip_frag(uint8 lchip, uint8 ctc_ip_frag, uint32* o_frag_info, uint32* o_frag_info_mask)
{
    uint32 frag_info = 0;
    uint32 frag_info_mask = 0;
    switch (ctc_ip_frag)
    {
    case CTC_IP_FRAG_NON:
        /* Non fragmented */
        frag_info      = 0;
        frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_FIRST:
        /* Fragmented, and first fragment */
        frag_info      = 1;
        frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_NON_OR_FIRST:
        /* Non fragmented OR Fragmented and first fragment */
        frag_info      = 0;
        frag_info_mask = 2;     /* mask frag_info as 0x */
        break;

    case CTC_IP_FRAG_SMALL:
        /* Small fragment */
        frag_info      = 2;
        frag_info_mask = 3;
        break;

    case CTC_IP_FRAG_NOT_FIRST:
        /* Not first fragment (Fragmented of cause) */
        frag_info      = 3;
        frag_info_mask = 2;/*SYSTEM MODIFIED by yejl, not-first-fragment should include small fragment, 2018-11-22*/
        break;

    case CTC_IP_FRAG_YES:
        /* Any Fragmented */
        frag_info      = 1;
        frag_info_mask = 1;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if(o_frag_info)
    {
        *o_frag_info = frag_info;
    }
    if(o_frag_info_mask)
    {
        *o_frag_info_mask = frag_info_mask;
    }

    return CTC_E_NONE;
}
#if 0
int32
_sys_usw_acl_unmap_ip_frag(uint8 lchip, uint32 frag_info_data, uint32 frag_info_mask, uint8* o_ctc_ip_frag, uint8* o_ctc_ip_frag_mask)
{
    uint8 ctc_ip_frag = 0;
    if (0 == frag_info_data && 3 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_NON;
    }
    else if(1 == frag_info_data && 3 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_FIRST;
    }
    else if(0 == frag_info_data && 2 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_NON_OR_FIRST;
    }
    else if(2 == frag_info_data && 3 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_SMALL;
    }
    else if(3 == frag_info_data && 3 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_NOT_FIRST;
    }
    else if(1 == frag_info_data && 1 == frag_info_mask)
    {
        ctc_ip_frag = CTC_IP_FRAG_YES;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if (o_ctc_ip_frag)
    {
        *o_ctc_ip_frag = ctc_ip_frag;
    }
    if (o_ctc_ip_frag_mask)
    {
        *o_ctc_ip_frag_mask = 0xFF;
    }
    return CTC_E_NONE;
}
#endif
STATIC int32
_sys_usw_acl_map_mergedata_type_ctc_to_sys(uint8 lchip, uint8 ctc_mergedata_type, uint8* drv_mergedata_type, uint8* drv_mergedata_type_mask)
{
    uint8 tmp_drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
    switch (ctc_mergedata_type)
    {
        case 1:
            tmp_drv_merge_type = SYS_ACL_MERGEDATA_TYPE_VXLAN;
            break;
        case 2:
            tmp_drv_merge_type = SYS_ACL_MERGEDATA_TYPE_GRE;
            break;
        case 3:
            tmp_drv_merge_type = SYS_ACL_MERGEDATA_TYPE_CAPWAP;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    if(drv_mergedata_type)
    {
        *drv_mergedata_type = tmp_drv_merge_type;
    }
    if(drv_mergedata_type_mask)
    {
        *drv_mergedata_type_mask = 0x3;
    }
    return CTC_E_NONE;
}

#define _ACL_HASH_
int32
_sys_usw_acl_set_hash_sel_bmp(uint8 lchip, sys_acl_entry_t* pe )
{
    uint32  cmd = 0;
    uint32  field_id = 0;
    uint32  field_id_cnt = 0;
    uint32  tbl_id = 0;
    ds_t ds_sel;

    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(pe->key_type)
    {
        case CTC_ACL_KEY_HASH_MAC:
            tbl_id = FlowL2HashFieldSelect_t;
        break;

        case CTC_ACL_KEY_HASH_IPV4:
            tbl_id = FlowL3Ipv4HashFieldSelect_t;
        break;

        case CTC_ACL_KEY_HASH_L2_L3:
            tbl_id = FlowL2L3HashFieldSelect_t;
        break;

        case CTC_ACL_KEY_HASH_MPLS:
            tbl_id = FlowL3MplsHashFieldSelect_t;
        break;

        case CTC_ACL_KEY_HASH_IPV6:
            tbl_id = FlowL3Ipv6HashFieldSelect_t;
        break;

        default:
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, pe->hash_field_sel_id, cmd, &ds_sel));

    drv_get_table_property(lchip, DRV_TABLE_PROP_FIELD_NUM, tbl_id, 0, &field_id_cnt);

    for(field_id = 0; field_id < field_id_cnt; field_id++)
    {
        if(drv_get_field_value(lchip, tbl_id,  field_id, &ds_sel) !=0 )
        {
            CTC_BMP_SET(pe->hash_sel_bmp,field_id);
            if(tbl_id == FlowL3Ipv4HashFieldSelect_t && field_id == FlowL3Ipv4HashFieldSelect_tcpFlagsEn_f)
            {
                if(drv_get_field_value(lchip, tbl_id,  field_id, &ds_sel) & 0x3F)
                {
                    pe->hash_sel_tcp_flags |= 0x1;
                }
                if(drv_get_field_value(lchip, tbl_id,  field_id, &ds_sel)>>6)
                {
                    pe->hash_sel_tcp_flags |= 0x2;
                }
            }
        }
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"_sys_usw_acl_set_hash_sel_bmp(),key is selected,hash_sel_bmp(0x%.8x,0x%.8x,0x%.8x,0x%.8x) \n",pe->hash_sel_bmp[0],pe->hash_sel_bmp[1],pe->hash_sel_bmp[2],pe->hash_sel_bmp[3]);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id, void* ds_sel)
{
    uint32 cmd = 0;

    CTC_MAX_VALUE_CHECK(field_sel_id, 0xF);
    if (CTC_ACL_KEY_HASH_MAC == key_type)
    {
        cmd = DRV_IOR(FlowL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    }
    else if (CTC_ACL_KEY_HASH_IPV4 == key_type)
    {
        cmd = DRV_IOR(FlowL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    }
    else if (CTC_ACL_KEY_HASH_MPLS == key_type)
    {
        cmd = DRV_IOR(FlowL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    }
    else if (CTC_ACL_KEY_HASH_IPV6 == key_type)
    {
        cmd = DRV_IOR(FlowL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    }
    else if (CTC_ACL_KEY_HASH_L2_L3 == key_type)
    {
        cmd = DRV_IOR(FlowL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, ds_sel));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_hash_index(uint8 lchip, sys_acl_entry_t* pe, uint8 is_lkup,uint8 install)
{
    uint32          key_id = 0;
    uint32          hw_index = 0;
    drv_acc_in_t    in;
    drv_acc_out_t   out;
	uint8 key_exist = 0;
	uint8  key_conflict = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(pe);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    _sys_usw_acl_get_table_id(lchip, pe, &key_id, NULL, &hw_index);
    if( pe->hash_sel_bmp[0]
		|| pe->hash_sel_bmp[1]
		||  pe->hash_sel_bmp[2]
		|| pe->hash_sel_bmp[3])
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"_sys_usw_acl_get_hash_index(),key is incomplete,hash_sel_bmp(0x%.8x,0x%.8x,0x%.8x,0x%.8x) \n",pe->hash_sel_bmp[0],pe->hash_sel_bmp[1],pe->hash_sel_bmp[2],pe->hash_sel_bmp[3]);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Not ready to configuration \n");
		return CTC_E_NOT_READY;

    }
    in.type     = is_lkup ? DRV_ACC_TYPE_LOOKUP : DRV_ACC_TYPE_ALLOC;
    in.op_type  = DRV_ACC_OP_BY_KEY;
    in.tbl_id   = key_id;
    in.data     = pe->buffer->key;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
	key_exist   =  out.is_hit;
    key_conflict =  (is_lkup && install) ? 0: out.is_conflict;

	if(is_lkup && install && (pe->key_exist || pe->key_conflict) && !out.is_hit )
	{
   	   in.type     =  DRV_ACC_TYPE_ALLOC;
       in.op_type  = DRV_ACC_OP_BY_KEY;
       in.tbl_id   = key_id;
       in.data     = pe->buffer->key;
	   CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
	   key_exist =  out.is_hit;
       key_conflict =  out.is_conflict;
	   is_lkup = 0;
	}

	if(key_conflict)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% conflict: %d \n", key_conflict);
		pe->key_conflict  = key_conflict;
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
		return CTC_E_HASH_CONFLICT;

    }
    else if(key_exist)
    {
       ds_t ds_hash_key;
 	   sal_memset(&ds_hash_key, 0, sizeof(ds_hash_key));
       SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% entry exist: %d \n", key_exist);

	    in.type     = DRV_ACC_TYPE_ADD;
        in.op_type  = DRV_ACC_OP_BY_INDEX;
        in.tbl_id   = key_id;
        in.data     = ds_hash_key;
        in.index    = hw_index;
	    if(pe->hash_valid)
	   	{
	       drv_acc_api(lchip, &in, &out);
		   pe->hash_valid = 0;
	   	}
	    pe->fpae.offset_a = 0xFFFFFFFF;
		pe->key_exist = key_exist;

        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

    }
	if(!is_lkup)
	{
        pe->fpae.offset_a = out.key_index;
        pe->hash_valid = 1;
   	    pe->key_exist= 0;
   	    pe->key_conflict= 0;
	}

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"  %% out.index: 0x%x \n", pe->fpae.offset_a);

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_mac_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    int32           ret         = CTC_E_NONE;
    uint32          data        = 0;
    uint32          mask        = 0;
    uint32          tmp_data    = 0;
    hw_mac_addr_t   hw_mac      = {0};
    sys_acl_buffer_t* p_buffer  = NULL;
    FlowL2HashFieldSelect_m ds_sel;
    ctc_field_port_t* pport= (ctc_field_port_t*)key_field->ext_data;

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_ACL_HASH_VALID_CHECK(pe->hash_valid);

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    p_buffer = pe->buffer;

    switch(key_field->type)
    {
     case CTC_FIELD_KEY_STAG_VALID:               /**< [D2] S-Vlan Exist. */
     case CTC_FIELD_KEY_SVLAN_ID:                 /**< [D2] S-Vlan ID. */
     case CTC_FIELD_KEY_STAG_COS:                 /**< [D2] Stag Cos. */
     case CTC_FIELD_KEY_STAG_CFI:                 /**< [D2] Stag Cfi. */
	 case CTC_FIELD_KEY_SVLAN_RANGE:              /**< [D2] Svlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
	 	if(pe->mac_key_vlan_mode == 2)
	 	{
	      SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "This key has already exist cvlan\n");
		   return CTC_E_INVALID_PARAM;
	 	}
         break;

     case CTC_FIELD_KEY_CTAG_VALID:               /**< [D2] C-Vlan Exist. */
     case CTC_FIELD_KEY_CVLAN_ID:                 /**< [D2] C-Vlan ID. */
     case CTC_FIELD_KEY_CTAG_COS:                 /**< [D2] Ctag Cos. */
     case CTC_FIELD_KEY_CTAG_CFI:                 /**< [D2] Ctag Cfi. */
     case CTC_FIELD_KEY_CVLAN_RANGE:              /**< [D2] Cvlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
	  	if(pe->mac_key_vlan_mode == 1)
	 	{
	       SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "This key has already exist svlan\n");
		   return CTC_E_INVALID_PARAM;
	 	}
          break;
	 default :
		  	break;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_STAG_CFI:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp, FlowL2HashFieldSelect_cfiEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL2HashKey(V, cfi_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp, FlowL2HashFieldSelect_cfiEn_f);
        }
        break;
    case CTC_FIELD_KEY_CTAG_CFI:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cfiEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL2HashKey(V, cfi_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cfiEn_f);
        }
        break;
    case CTC_FIELD_KEY_STAG_COS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cosEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2HashKey(V, cos_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cosEn_f);
        }
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cosEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2HashKey(V, cos_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_cosEn_f);
        }
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_etherTypeEn_f))
        {
            /*ethertype no need to check param valid*/
            SetDsFlowL2HashKey(V, etherType_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_etherTypeEn_f);
        }
        break;
    case CTC_FIELD_KEY_PORT:
        CTC_PTR_VALID_CHECK(key_field->ext_data);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, pe->hash_field_sel_id, &ds_sel));
        if(CTC_FIELD_PORT_TYPE_GPORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_GPORT == GetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                SYS_GLOBAL_PORT_CHECK(pport->gport);
                tmp_data = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pport->gport);
                SetDsFlowL2HashKey(V, globalSrcPort_f, p_buffer->key, tmp_data);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_globalPortType_f);
            }
        }
        if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == GetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                CTC_MAX_VALUE_CHECK(pport->logic_port, SYS_USW_ACL_MAX_LOGIC_PORT_NUM);
                SetDsFlowL2HashKey(V, globalSrcPort_f, p_buffer->key, pport->logic_port);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_globalPortType_f);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MAC, pe->hash_field_sel_id, &ds_sel));
        if(ACL_HASH_GPORT_TYPE_METADATA == GetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL2HashKey(V, globalSrcPort_f, p_buffer->key, data|(1<<14));
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_globalPortType_f);
        }
        break;
    case CTC_FIELD_KEY_MAC_DA:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_macDaEn_f))
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsFlowL2HashKey(A, macDa_f, p_buffer->key, hw_mac);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_macDaEn_f);
        }
        break;
    case CTC_FIELD_KEY_MAC_SA:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_macSaEn_f))
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsFlowL2HashKey(A, macSa_f, p_buffer->key, hw_mac);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_macSaEn_f);
        }
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f))
        {
            SYS_ACL_VLAN_ID_CHECK(data);
            SetDsFlowL2HashKey(V, vlanId_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f))
        {
            SYS_ACL_VLAN_ID_CHECK(data);
            SetDsFlowL2HashKey(V, vlanId_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdValidEn_f))
        {
            SetDsFlowL2HashKey(V, vlanIdValid_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdValidEn_f);
        }
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdValidEn_f))
        {
            SetDsFlowL2HashKey(V, vlanIdValid_f, p_buffer->key, data);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdValidEn_f);
        }
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &mask));
            SetDsFlowL2HashKey(V, vlanId_f, p_buffer->key, mask);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        if( CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &mask));
            SetDsFlowL2HashKey(V, vlanId_f, p_buffer->key, mask);
            SetDsFlowL2HashKey(V, flowL2KeyUseCvlan_f, p_buffer->key, 1);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2HashFieldSelect_vlanIdEn_f);
        }
        break;
    case SYS_FIELD_KEY_HASH_SEL_ID:
        CTC_MAX_VALUE_CHECK(data, 0xF);
        SetDsFlowL2HashKey(V, flowFieldSel_f, p_buffer->key, data);
        break;

    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    switch(key_field->type)
    {
     case CTC_FIELD_KEY_STAG_VALID:               /**< [D2] S-Vlan Exist. */
     case CTC_FIELD_KEY_SVLAN_ID:                 /**< [D2] S-Vlan ID. */
     case CTC_FIELD_KEY_STAG_COS:                 /**< [D2] Stag Cos. */
     case CTC_FIELD_KEY_STAG_CFI:                 /**< [D2] Stag Cfi. */
	 case CTC_FIELD_KEY_SVLAN_RANGE:              /**< [D2] Svlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
	  	 pe->mac_key_vlan_mode = 1;    /*per entry set,no suuport per filed clear (hezc)*/
         break;

     case CTC_FIELD_KEY_CTAG_VALID:               /**< [D2] C-Vlan Exist. */
     case CTC_FIELD_KEY_CVLAN_ID:                 /**< [D2] C-Vlan ID. */
     case CTC_FIELD_KEY_CTAG_COS:                 /**< [D2] Ctag Cos. */
     case CTC_FIELD_KEY_CTAG_CFI:                 /**< [D2] Ctag Cfi. */
     case CTC_FIELD_KEY_CVLAN_RANGE:              /**< [D2] Cvlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
	  	  pe->mac_key_vlan_mode = 2;     /*per entry set,no suuport per filed clear (hezc)*/
          break;
	 default :
		  	break;
    }
    return ret;
}

        /* RFC1349:         0       1       2       3       4       5       6       7
                        +-------+-------+-------+-------+-------+-------+-------+-------+
                        |      PRECEDENCE       |              TOS              |  MBS  |
                        +-------+-------+-------+-------+-------+-------+-------+-------+
           For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;

           RFC2474:         0       1       2       3       4       5       6       7
                        +-------+-------+-------+-------+-------+-------+-------+-------+
                        |                      DSCP                     |      ECN      |
                        +-------+-------+-------+-------+-------+-------+-------+-------+
           For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;
        */

int32
_sys_usw_acl_add_hash_ipv4_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data         = 0;
    int32  ret          = CTC_E_NONE;
    uint32 tmp_data     = 0;
    sys_acl_buffer_t* p_buffer   = NULL;
    FlowL3Ipv4HashFieldSelect_m ds_sel;
    ctc_field_port_t* pport= (ctc_field_port_t*)key_field->ext_data;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_ACL_HASH_VALID_CHECK(pe->hash_valid);

    sal_memset(&ds_sel, 0, sizeof(ds_sel));
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_IP_DSCP:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_dscpEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3F);
            SetDsFlowL3Ipv4HashKey(V, dscp_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_dscpEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_dscpEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x7);
            SetDsFlowL3Ipv4HashKey(V, dscp_f, p_buffer->key, data << 3);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_dscpEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_ECN:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ecnEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 3);
            SetDsFlowL3Ipv4HashKey(V, ecn_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ecnEn_f);
        }
        break;
    case CTC_FIELD_KEY_PORT:
        CTC_PTR_VALID_CHECK(key_field->ext_data);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, pe->hash_field_sel_id, &ds_sel));
        if(CTC_FIELD_PORT_TYPE_GPORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_GPORT == GetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                SYS_GLOBAL_PORT_CHECK(pport->gport);
                tmp_data = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pport->gport);
                SetDsFlowL3Ipv4HashKey(V, globalSrcPort_f, p_buffer->key, tmp_data);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_globalPortType_f);
            }
        }
        if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == GetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                CTC_MAX_VALUE_CHECK(pport->logic_port, SYS_USW_ACL_MAX_LOGIC_PORT_NUM);
                SetDsFlowL3Ipv4HashKey(V, globalSrcPort_f, p_buffer->key, pport->logic_port);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_globalPortType_f);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, pe->hash_field_sel_id, &ds_sel));
        if(ACL_HASH_GPORT_TYPE_METADATA == GetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL3Ipv4HashKey(V, globalSrcPort_f, p_buffer->key, data|(1<<14));
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_globalPortType_f);
        }
        break;
    case CTC_FIELD_KEY_IP_DA:
        {
            uint8   ip_en        = 0;
            uint8   ip_len       = 0;
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, pe->hash_field_sel_id, &ds_sel));
            ip_en = GetFlowL3Ipv4HashFieldSelect(V, ipDaEn_f, &ds_sel);
            if (ip_en)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipDaEn_f);
                ip_len = GetFlowL3Ipv4HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipDaPrefixLength_f);
                tmp_data = (data >> (31 - ip_len)) << (31 - ip_len);
                SetDsFlowL3Ipv4HashKey(V, ipDa_f, p_buffer->key, tmp_data);
            }
            break;
        }
    case CTC_FIELD_KEY_ROUTED_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipRoutedPacketEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL3Ipv4HashKey(V, ipRoutedPacket_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipRoutedPacketEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_SA:
        {
            uint8   ip_en        = 0;
            uint8   ip_len       = 0;
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV4, pe->hash_field_sel_id, &ds_sel));
            ip_en = GetFlowL3Ipv4HashFieldSelect(V, ipSaEn_f, &ds_sel);
            if (ip_en)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipSaEn_f);
                ip_len = GetFlowL3Ipv4HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipSaPrefixLength_f);
                tmp_data = (data >> (31 - ip_len)) << (31 - ip_len);
                SetDsFlowL3Ipv4HashKey(V, ipSa_f, p_buffer->key, tmp_data);
            }
            break;
        }
    case CTC_FIELD_KEY_VXLAN_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_isVxlanEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL3Ipv4HashKey(V, isVxlan_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_isVxlanEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer4TypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xF);
            SetDsFlowL3Ipv4HashKey(V, layer4Type_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer4TypeEn_f);
            pe->l4_type= data;
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer3HeaderProtocolEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3Ipv4HashKey(V, layer3HeaderProtocol_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer3HeaderProtocolEn_f);
        }
        else if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer4TypeEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type(lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, NULL));
            SetDsFlowL3Ipv4HashKey(V, layer4Type_f, p_buffer->key, tmp_data);
            pe->l4_type= tmp_data;
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_layer4TypeEn_f);
        }
        break;
    /*mark: now support u1_type == 0*/
    case CTC_FIELD_KEY_IP_FRAG:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_fragInfoEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, data, &tmp_data, NULL));
            SetDsFlowL3Ipv4HashKey(V, fragInfo_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_fragInfoEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipHeaderErrorEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL3Ipv4HashKey(V, ipHeaderError_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipHeaderErrorEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipOptionsEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL3Ipv4HashKey(V, ipOptions_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ipOptionsEn_f);
        }
        break;
        /*hash ipv4  CTC_FIELD_KEY_TCP_FLAGS include CTC_FIELD_KEY_TCP_FLAGS(6bits) and CTC_FIELD_KEY_TCP_ECN(2bits)*/
    case CTC_FIELD_KEY_TCP_FLAGS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_tcpFlagsEn_f) && pe->hash_sel_tcp_flags&0x01)
        {
            CTC_MAX_VALUE_CHECK(data, 0x3F);
            tmp_data = GetDsFlowL3Ipv4HashKey(V, tcpFlags_f, p_buffer->key);
            tmp_data = (tmp_data&0xC0) | (data&0x3F);
            SetDsFlowL3Ipv4HashKey(V, tcpFlags_f, p_buffer->key, tmp_data);
            pe->hash_sel_tcp_flags &= 0x2;
            if(!pe->hash_sel_tcp_flags)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_tcpFlagsEn_f);
            }
        }
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_tcpFlagsEn_f) && pe->hash_sel_tcp_flags&0x02)
        {
            CTC_MAX_VALUE_CHECK(data, 0x3);
            tmp_data = GetDsFlowL3Ipv4HashKey(V, tcpFlags_f, p_buffer->key);
            tmp_data = (tmp_data&0x3F) | (data << 6);
            SetDsFlowL3Ipv4HashKey(V, tcpFlags_f, p_buffer->key, tmp_data);
            pe->hash_sel_tcp_flags &= 0x1;
            if(!pe->hash_sel_tcp_flags)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_tcpFlagsEn_f);
            }
        }
        break;
    case CTC_FIELD_KEY_IP_TTL:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ttlEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3Ipv4HashKey(V, ttl_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_ttlEn_f);
        }
        break;

    /*mark: gre and nvgre not same*/
    case CTC_FIELD_KEY_GRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_greKeyEn_f))
        {
            SetDsFlowL3Ipv4HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_greKeyEn_f);
        }
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_nvgreVsidEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFFF);
            SetDsFlowL3Ipv4HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_nvgreVsidEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_l4SourcePortEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFF);
            SetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_l4SourcePortEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_l4DestPortEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFF);
            SetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4DestPort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_l4DestPortEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_icmpTypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0x00FF;     /*clear high 8bits*/
            tmp_data |= data << 8;
            SetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_icmpTypeEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_icmpOpcodeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0xFF00;     /*clear low 8bits*/
            tmp_data |= data & 0xFF;
            SetDsFlowL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_icmpOpcodeEn_f);
        }
        break;
    /*VNI*/
    case CTC_FIELD_KEY_VN_ID:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_UDP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch, l4_type:%d, line %u\n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_vxlanVniEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFFF);
            SetDsFlowL3Ipv4HashKey(V, uL4_gVxlan_vni_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv4HashFieldSelect_vxlanVniEn_f);
        }
        break;

    case SYS_FIELD_KEY_HASH_SEL_ID:
        CTC_MAX_VALUE_CHECK(data, 0xF);
        SetDsFlowL3Ipv4HashKey(V, flowFieldSel_f, p_buffer->key, data);
        break;

    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    return ret;
}


int32
_sys_usw_acl_add_hash_l2l3_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32  data         = 0;
    uint32  mask         = 0;
    uint32  tmp_data     = 0;
    int32   ret          = CTC_E_NONE;
    hw_mac_addr_t hw_mac = {0};
    sys_acl_buffer_t*      p_buffer  = NULL;
    FlowL2L3HashFieldSelect_m ds_sel;
    ctc_field_port_t* pport= (ctc_field_port_t*)key_field->ext_data;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_USW_ACL_HASH_VALID_CHECK(pe->hash_valid);

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask; /*pkt length range and L4 port range*/
    }

    p_buffer = pe->buffer;

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_CTAG_CFI:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ctagCfiEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, ctagCfi_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ctagCfiEn_f);
        }
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ctagCosEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x7);
            SetDsFlowL2L3HashKey(V, ctagCos_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ctagCosEn_f);
        }
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdValidEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, cvlanIdValid_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdValidEn_f);
        }
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdEn_f))
        {
            SYS_ACL_VLAN_ID_CHECK(data);
            SetDsFlowL2L3HashKey(V, cvlanId_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range
                    (lchip, key_field, pe->group->group_info.dir, 0, &mask));
            SetDsFlowL2L3HashKey(V, cvlanId_f, p_buffer->key, mask);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_cvlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_PORT:

        CTC_PTR_VALID_CHECK(key_field->ext_data);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, pe->hash_field_sel_id, &ds_sel));
        if(CTC_FIELD_PORT_TYPE_GPORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_GPORT == GetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                SYS_GLOBAL_PORT_CHECK(pport->gport);
                tmp_data = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pport->gport);
                SetDsFlowL2L3HashKey(V, globalSrcPort_f, p_buffer->key, tmp_data);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_globalPortType_f);
            }
        }
        if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == GetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                CTC_MAX_VALUE_CHECK(pport->logic_port, SYS_USW_ACL_MAX_LOGIC_PORT_NUM);
                SetDsFlowL2L3HashKey(V, globalSrcPort_f, p_buffer->key, pport->logic_port);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_globalPortType_f);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, pe->hash_field_sel_id, &ds_sel));
        if(ACL_HASH_GPORT_TYPE_METADATA == GetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL2L3HashKey(V, globalSrcPort_f, p_buffer->key, data|(1<<14));
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_globalPortType_f);
        }
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipRoutedPacketEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, ipRoutedPacket_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipRoutedPacketEn_f);
        }
        break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_isVxlanEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, isVxlan_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_isVxlanEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer3HeaderProtocolEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL2L3HashKey(V, layer3HeaderProtocol_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer3HeaderProtocolEn_f);
        }
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer3TypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xF);
            SetDsFlowL2L3HashKey(V, layer3Type_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer3TypeEn_f);
            pe->l3_type= data;
        }
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer4TypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xF);
            SetDsFlowL2L3HashKey(V, layer4Type_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_layer4TypeEn_f);
            pe->l4_type= data;
        }
        break;
    case CTC_FIELD_KEY_MAC_DA:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_macDaEn_f))
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsFlowL2L3HashKey(A, macDa_f, p_buffer->key, hw_mac);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_macDaEn_f);
        }
        break;
    case CTC_FIELD_KEY_MAC_SA:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_macSaEn_f))
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsFlowL2L3HashKey(A, macSa_f, p_buffer->key, hw_mac);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_macSaEn_f);
        }
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_stagCfiEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, stagCfi_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_stagCfiEn_f);
        }
        break;
    case CTC_FIELD_KEY_STAG_COS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_stagCosEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2L3HashKey(V, stagCos_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_stagCosEn_f);
        }
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdValidEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL2L3HashKey(V, svlanIdValid_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdValidEn_f);
        }
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdEn_f))
        {
            SYS_ACL_VLAN_ID_CHECK(data);
            SetDsFlowL2L3HashKey(V, svlanId_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdEn_f);
        }
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range
                            (lchip, key_field, pe->group->group_info.dir, 1, &mask));
                    SetDsFlowL2L3HashKey(V, svlanId_f, p_buffer->key, mask);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_svlanIdEn_f);
        }
        break;
    /*field share: IPv4*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_dscpEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3F);
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_dscp_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_dscpEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_dscpEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x7);
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_dscp_f, p_buffer->key, data << 3);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_dscpEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_ECN:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ecnEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 3);
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_ecn_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ecnEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_fragInfoEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, data, &tmp_data, NULL));
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_fragInfo_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_fragInfoEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_DA:
        {
            uint8  ip_en        = 0;
            uint8  ip_len       = 0;
            sal_memset(&ds_sel, 0, sizeof(ds_sel));
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, pe->hash_field_sel_id, &ds_sel));
            ip_en = GetFlowL2L3HashFieldSelect(V, ipDaEn_f, &ds_sel);
            if (ip_en)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipDaEn_f);
                ip_len = GetFlowL2L3HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipDaPrefixLength_f);
                tmp_data = (data >> (31 - ip_len)) << (31 - ip_len);
                SetDsFlowL2L3HashKey(V, uL3_gIpv4_ipDa_f, p_buffer->key, tmp_data);
            }
            break;
        }
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipHeaderErrorEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_ipHeaderError_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipHeaderErrorEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipOptionsEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_ipOptions_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipOptionsEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_SA:
        {
            uint8  ip_en        = 0;
            uint8  ip_len       = 0;
            CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_L2_L3, pe->hash_field_sel_id, &ds_sel));
            ip_en = GetFlowL2L3HashFieldSelect(V, ipSaEn_f, &ds_sel);
            if (ip_en)
            {
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipSaEn_f);
                ip_len = GetFlowL2L3HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ipSaPrefixLength_f);
                tmp_data = (data >> (31 - ip_len)) << (31 - ip_len);
                SetDsFlowL2L3HashKey(V, uL3_gIpv4_ipSa_f, p_buffer->key, tmp_data);
            }
            break;
        }
    case CTC_FIELD_KEY_TCP_ECN:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_tcpEcnEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3);
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_tcpEcn_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_tcpEcnEn_f);
        }
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_tcpFlagsEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3F);
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_tcpFlags_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_tcpFlagsEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_TTL:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ttlEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL3_gIpv4_ttl_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_ttlEn_f);
        }
        break;
    /*field share: MPLS*/
    case CTC_FIELD_KEY_LABEL_NUM:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_labelNumEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, MCHIP_CAP(SYS_CAP_ACL_LABEL_NUM));
            SetDsFlowL2L3HashKey(V, uL3_gMpls_labelNum_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_labelNumEn_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsExp0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsExp1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsExp2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsExp2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsLabel0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsLabel1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsLabel2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsLabel2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsSbit0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsSbit1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsSbit2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsSbit2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsTtl0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsTtl1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL2L3HashKey(V, uL3_gMpls_mplsTtl2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_mplsTtl2En_f);
        }
        break;
    /*field share: Unknow Packet*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_unknownPktEtherTypeEn_f))
        {
            /*ethertype no need to check param valid*/
            if(0x0800== data ||0x8847== data ||0x8848== data) /* uL3_gUnknown_etherType except isIpv4, isMpls, isMplsMcast */
            {
                return CTC_E_INVALID_PARAM;
            }
            SetDsFlowL2L3HashKey(V, uL3_gUnknown_etherType_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_unknownPktEtherTypeEn_f);
        }
        break;
    /*field share: GreKey*/
    case CTC_FIELD_KEY_GRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_greKeyEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_greKeyEn_f);
        }
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_nvgreVsidEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFFF);
            SetDsFlowL2L3HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_nvgreVsidEn_f);
        }
        break;
    /*field share: L4 port*/
    case CTC_FIELD_KEY_L4_DST_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_l4DestPortEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL4_gPort_l4DestPort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_l4DestPortEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_l4SourcePortEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_l4SourcePortEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_icmpTypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0x00FF;     /*clear high 8bits*/
            tmp_data |= data << 8;
            SetDsFlowL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_icmpTypeEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_icmpOpcodeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0xFF00;     /*clear low 8bits*/
            tmp_data |= data & 0xFF;
            SetDsFlowL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_icmpOpcodeEn_f);
        }
        break;
    /*field share: VNI*/
    case CTC_FIELD_KEY_VN_ID:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_UDP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch , l3_type :%d l4_type:%d \n", pe->l3_type, pe->l4_type);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_vxlanVniEn_f))
        {
            SetDsFlowL2L3HashKey(V, uL4_gVxlan_vni_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_vxlanVniEn_f);
        }
        break;

    case CTC_FIELD_KEY_IS_ROUTER_MAC:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_isRouteMacEn_f))
        {
            SetDsFlowL2L3HashKey(V, isRouteMac_f, p_buffer->key, is_add ? 1 : 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_isRouteMacEn_f);
        }
        break;

    case CTC_FIELD_KEY_VRFID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_vrfIdEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FF);
            SetDsFlowL2L3HashKey(V, vrfId9_0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL2L3HashFieldSelect_vrfIdEn_f);
        }
        break;

    case SYS_FIELD_KEY_HASH_SEL_ID:
        CTC_MAX_VALUE_CHECK(data, 0xF);
        SetDsFlowL2L3HashKey(V, flowFieldSel_f, p_buffer->key, data);
        break;

    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    return ret;
}

int32
_sys_usw_acl_add_hash_ipv6_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32  data         = 0;
    uint8   ip_en        = 0;
    uint8   ip_len       = 0;
    int8    idx          = 0;
    uint8   words        = 0;
    uint8   bits         = 0;
    uint32  tmp_data     = 0;
    ipv6_addr_t hw_ip6   = {0};
    ipv6_addr_t tmp_ip6  = {0};
    int32   ret          = CTC_E_NONE;
    uint8   field_sel_id = 0;
    sys_acl_buffer_t* p_buffer   = NULL;
    FlowL3Ipv6HashFieldSelect_m ds_sel;
    ctc_field_port_t* pport= (ctc_field_port_t*)key_field->ext_data;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_USW_ACL_HASH_VALID_CHECK(pe->hash_valid);

    if(is_add)
    {
        data = key_field->data;
    }

    p_buffer = pe->buffer;

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_IP_DSCP:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_dscpEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3F);
            SetDsFlowL3Ipv6HashKey(V, dscp_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_dscpEn_f);
        }
        break;
    case CTC_FIELD_KEY_PORT:
        CTC_PTR_VALID_CHECK(key_field->ext_data);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, pe->hash_field_sel_id, &ds_sel));
        if(CTC_FIELD_PORT_TYPE_GPORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_GPORT == GetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                SYS_GLOBAL_PORT_CHECK(pport->gport);
                tmp_data = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pport->gport);
                SetDsFlowL3Ipv6HashKey(V, globalSrcPort_f, p_buffer->key, tmp_data);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_globalPortType_f);
            }
        }
        if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == GetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                CTC_MAX_VALUE_CHECK(pport->logic_port, SYS_USW_ACL_MAX_LOGIC_PORT_NUM);
                SetDsFlowL3Ipv6HashKey(V, globalSrcPort_f, p_buffer->key, pport->logic_port);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_globalPortType_f);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, pe->hash_field_sel_id, &ds_sel));
        if(ACL_HASH_GPORT_TYPE_METADATA == GetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL3Ipv6HashKey(V, globalSrcPort_f, p_buffer->key, data|(1<<14));
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_globalPortType_f);
        }
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipRoutedPacketEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL3Ipv6HashKey(V, ipRoutedPacket_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipRoutedPacketEn_f);
        }
        break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_isVxlanEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x1);
            SetDsFlowL3Ipv6HashKey(V, isVxlan_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_isVxlanEn_f);
        }
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer3HeaderProtocolEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3Ipv6HashKey(V, layer3HeaderProtocol_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer3HeaderProtocolEn_f);
        }
        else if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer4TypeEn_f))
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type(lchip, CTC_PARSER_L3_TYPE_IPV6, data, &tmp_data, NULL));
            SetDsFlowL3Ipv6HashKey(V, layer4Type_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer4TypeEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer4TypeEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, layer4Type_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_layer4TypeEn_f);
            pe->l4_type= data;
        }
        break;
    /*now support u1_g1 only*/
    case CTC_FIELD_KEY_IPV6_DA:
        CTC_PTR_VALID_CHECK(key_field->ext_data);
        sal_memset(&ds_sel, 0, sizeof(ds_sel));
        field_sel_id = GetDsFlowL3Ipv6HashKey(V, flowFieldSel_f, p_buffer->key);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, field_sel_id, &ds_sel));
        ip_en = GetFlowL3Ipv6HashFieldSelect(V, ipDaEn_f, &ds_sel);
        if (ip_en)
        {
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipDaEn_f);
            ip_len = GetFlowL3Ipv6HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipDaPrefixLength_f);
            words = ((ip_len << 2) + 4) /32;
            bits  = ((ip_len << 2) + 4) %32;
            sal_memcpy(tmp_ip6, (uint32*)(key_field->ext_data), sizeof(ipv6_addr_t));
            for (idx = 3; idx > words; idx--)
            {
                sal_memset(tmp_ip6 + idx, 0, sizeof(uint32));
            }
            if (bits)
            {
                tmp_ip6[words] = (tmp_ip6[words] >> (32 - bits) ) << (32 - bits);
            }
            ACL_SET_HW_IP6(hw_ip6, tmp_ip6);
            SetDsFlowL3Ipv6HashKey(A, ipDa_f, p_buffer->key, hw_ip6);
        }
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        CTC_PTR_VALID_CHECK(key_field->ext_data);
        sal_memset(&ds_sel, 0, sizeof(ds_sel));
        field_sel_id = GetDsFlowL3Ipv6HashKey(V, flowFieldSel_f, p_buffer->key);
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_IPV6, field_sel_id, &ds_sel));
        ip_en = GetFlowL3Ipv6HashFieldSelect(V, ipSaEn_f, &ds_sel);
        if (ip_en)
        {
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipSaEn_f);
            ip_len = GetFlowL3Ipv6HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipSaPrefixLength_f);
            words = ((ip_len << 2) + 4) /32;
            bits  = ((ip_len << 2) + 4) %32;
            sal_memcpy(tmp_ip6, (uint32*)(key_field->ext_data), sizeof(ipv6_addr_t));
            for (idx = 3; idx > words; idx--)
            {
                sal_memset(tmp_ip6 + idx, 0, sizeof(uint32));
            }
            if (bits)
            {
                tmp_ip6[words] = (tmp_ip6[words] >> (32 - bits) ) << (32 - bits);
            }
            ACL_SET_HW_IP6(hw_ip6, tmp_ip6);
            SetDsFlowL3Ipv6HashKey(A, ipSa_f, p_buffer->key, hw_ip6);
        }
        break;
    /*GRE*/
    case CTC_FIELD_KEY_GRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_greKeyEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_greKeyEn_f);
        }
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_GRE))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_nvgreVsidEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFFF)
            SetDsFlowL3Ipv6HashKey(V, uL4_gKey_greKey_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_nvgreVsidEn_f);
        }
        break;
    /*L4 port*/
    case CTC_FIELD_KEY_L4_DST_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_l4DestPortEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4DestPort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_l4DestPortEn_f);
        }
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        if (!((pe->l4_type == CTC_PARSER_L4_TYPE_TCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_UDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_RDP) || (pe->l4_type == CTC_PARSER_L4_TYPE_DCCP) || (pe->l4_type == CTC_PARSER_L4_TYPE_SCTP)))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_l4SourcePortEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_l4SourcePortEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if( CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpOpcodeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0xFF00;     /*clear low 8bits*/
            tmp_data |= data & 0xFF;
            SetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpOpcodeEn_f);
        }
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_ICMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpTypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0x00FF;     /*clear high 8bits*/
            tmp_data |= data << 8;
            SetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpTypeEn_f);
        }
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_IGMP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpTypeEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            tmp_data = GetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key);
            tmp_data &= 0x00FF;     /*clear high 8bits*/
            tmp_data |= data << 8;
            SetDsFlowL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_buffer->key, tmp_data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_icmpTypeEn_f);
        }
        break;
    /*VNI*/
    case CTC_FIELD_KEY_VN_ID:
        if (!(pe->l4_type == CTC_PARSER_L4_TYPE_UDP))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Param is mismatch ,l4_type:%d, line %u \n", pe->l4_type, __LINE__);
            return CTC_E_INVALID_PARAM;
        }
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_vxlanVniEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, uL4_gVxlan_vni_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_vxlanVniEn_f);
        }
        break;
    case CTC_FIELD_KEY_IS_ROUTER_MAC:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_isRouteMacEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, isRouteMac_f, p_buffer->key, is_add ? 1 : 0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_isRouteMacEn_f);
        }
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipv6FlowLabelEn_f))
        {
            SetDsFlowL3Ipv6HashKey(V, ipv6FlowLabel_f, p_buffer->key, is_add ? data:0);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3Ipv6HashFieldSelect_ipv6FlowLabelEn_f);
        }
        break;
    case SYS_FIELD_KEY_HASH_SEL_ID:
        CTC_MAX_VALUE_CHECK(data, 0xF);
        SetDsFlowL3Ipv6HashKey(V, flowFieldSel_f, p_buffer->key, data);
        break;


    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    return ret;
}

int32
_sys_usw_acl_add_hash_mpls_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32  data        = 0;
    uint32  tmp_data    = 0;
    int32   ret         = CTC_E_NONE;
    sys_acl_buffer_t* p_buffer = NULL;
    FlowL3MplsHashFieldSelect_m   ds_sel;
    ctc_field_port_t* pport= (ctc_field_port_t*)key_field->ext_data;

    CTC_PTR_VALID_CHECK(pe);
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_USW_ACL_HASH_VALID_CHECK(pe->hash_valid);

    if(is_add)
    {
        data = key_field->data;
    }

    p_buffer = pe->buffer;

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        CTC_PTR_VALID_CHECK(key_field->ext_data);

        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, pe->hash_field_sel_id, &ds_sel));
        if(CTC_FIELD_PORT_TYPE_GPORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_GPORT == GetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                SYS_GLOBAL_PORT_CHECK(pport->gport);
                tmp_data = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pport->gport);
                SetDsFlowL3MplsHashKey(V, globalSrcPort_f, p_buffer->key, tmp_data);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_globalPortType_f);
            }
        }
        if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport->type)
        {
            if(ACL_HASH_GPORT_TYPE_LOGIC_PORT == GetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel))
            {
                CTC_MAX_VALUE_CHECK(pport->logic_port, SYS_USW_ACL_MAX_LOGIC_PORT_NUM);
                SetDsFlowL3MplsHashKey(V, globalSrcPort_f, p_buffer->key, pport->logic_port);
                CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_globalPortType_f);
            }
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        CTC_ERROR_RETURN(_sys_usw_acl_get_field_sel(lchip, CTC_ACL_KEY_HASH_MPLS, pe->hash_field_sel_id, &ds_sel));
        if(ACL_HASH_GPORT_TYPE_METADATA == GetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL3MplsHashKey(V, globalSrcPort_f, p_buffer->key, data|(1<<14));
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_globalPortType_f);
        }
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_labelNumEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, MCHIP_CAP(SYS_CAP_ACL_LABEL_NUM));
            SetDsFlowL3MplsHashKey(V, labelNum_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_labelNumEn_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL3MplsHashKey(V, mplsExp0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL3MplsHashKey(V, mplsExp1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 7);
            SetDsFlowL3MplsHashKey(V, mplsExp2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsExp2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL3MplsHashKey(V, mplsLabel0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL3MplsHashKey(V, mplsLabel1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFFFFF);
            SetDsFlowL3MplsHashKey(V, mplsLabel2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsLabel2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL3MplsHashKey(V, mplsSbit0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL3MplsHashKey(V, mplsSbit1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 1);
            SetDsFlowL3MplsHashKey(V, mplsSbit2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsSbit2En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl0En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3MplsHashKey(V, mplsTtl0_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl0En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl1En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3MplsHashKey(V, mplsTtl1_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl1En_f);
        }
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl2En_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0xFF);
            SetDsFlowL3MplsHashKey(V, mplsTtl2_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_mplsTtl2En_f);
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        if(CTC_BMP_ISSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_vrfIdEn_f))
        {
            CTC_MAX_VALUE_CHECK(data, 0x3FFF);
            SetDsFlowL3MplsHashKey(V, vrfId_f, p_buffer->key, data);
            CTC_BMP_UNSET(pe->hash_sel_bmp,FlowL3MplsHashFieldSelect_vrfIdEn_f);
        }
        break;

    case SYS_FIELD_KEY_HASH_SEL_ID:
        CTC_MAX_VALUE_CHECK(data, 0xF);
        SetDsFlowL3MplsHashKey(V, flowFieldSel_f, p_buffer->key, data);
        break;

    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    return ret;
}

int32
_sys_usw_acl_add_hash_mac_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    uint32 cmd  = 0;
    FlowL2HashFieldSelect_m ds_sel;
    uint32 value = 0;
    sal_memset(&ds_sel, 0, sizeof(FlowL2HashFieldSelect_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM," %% PARAM: field_sel_id %u -- key_field_type %u\n", field_sel_id, sel_field->type);

    if(is_add)
    {
        value = 1;
    }

    cmd = DRV_IOR(FlowL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO," %% INFO: key_field_type %u -- add1_remove0 %u \n", sel_field->type ,is_add );

    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_CTAG_CFI:
    case CTC_FIELD_KEY_STAG_CFI:
        SetFlowL2HashFieldSelect(V, cfiEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
    case CTC_FIELD_KEY_STAG_COS:
        SetFlowL2HashFieldSelect(V, cosEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetFlowL2HashFieldSelect(V, etherTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            ctc_field_port_t* pport_sel = (ctc_field_port_t*)sel_field->ext_data;
            CTC_PTR_VALID_CHECK(sel_field->ext_data);
            if(CTC_FIELD_PORT_TYPE_GPORT == pport_sel->type)
            {
                SetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_GPORT);
            }
            if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport_sel->type)
            {
                SetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_LOGIC_PORT);
            }
        }
        else
        {
            SetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_NONE);
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        SetFlowL2HashFieldSelect(V, globalPortType_f, &ds_sel, is_add ? ACL_HASH_GPORT_TYPE_METADATA : 0);
        break;
    case CTC_FIELD_KEY_MAC_DA:
        SetFlowL2HashFieldSelect(V, macDaEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MAC_SA:
        SetFlowL2HashFieldSelect(V, macSaEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
    case CTC_FIELD_KEY_STAG_VALID:
        SetFlowL2HashFieldSelect(V, vlanIdValidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
    case CTC_FIELD_KEY_SVLAN_ID:
    case CTC_FIELD_KEY_CVLAN_RANGE:
    case CTC_FIELD_KEY_SVLAN_RANGE:
        SetFlowL2HashFieldSelect(V, vlanIdEn_f, &ds_sel, value);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR," %% ERROR: key_field_type %u -- Key Field Type Not Support\n", sel_field->type);
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    cmd = DRV_IOW(FlowL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_ipv4_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    uint32 cmd  = 0;
    FlowL3Ipv4HashFieldSelect_m ds_sel;
    uint32 value = 0;

    sal_memset(&ds_sel, 0, sizeof(FlowL3Ipv4HashFieldSelect_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM," %% PARAM: field_sel_id %u -- key_field_type %u\n", field_sel_id, sel_field->type);

    if(is_add)
    {
        value = 1;
    }

    cmd = DRV_IOR(FlowL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_IP_DSCP: /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetFlowL3Ipv4HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetFlowL3Ipv4HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetFlowL3Ipv4HashFieldSelect(V, ecnEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        SetFlowL3Ipv4HashFieldSelect(V, fragInfoEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            ctc_field_port_t* pport_sel = (ctc_field_port_t*)sel_field->ext_data;
            CTC_PTR_VALID_CHECK(sel_field->ext_data);
            if(CTC_FIELD_PORT_TYPE_GPORT == pport_sel->type)
            {
                SetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_GPORT);
            }
            if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport_sel->type)
            {
                SetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_LOGIC_PORT);
            }
        }
        else
        {
            SetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_NONE);
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        SetFlowL3Ipv4HashFieldSelect(V, globalPortType_f, &ds_sel, is_add ? ACL_HASH_GPORT_TYPE_METADATA : 0);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetFlowL3Ipv4HashFieldSelect(V, greKeyEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetFlowL3Ipv4HashFieldSelect(V, nvgreVsidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        SetFlowL3Ipv4HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        SetFlowL3Ipv4HashFieldSelect(V, icmpTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetFlowL3Ipv4HashFieldSelect(V, ipDaEn_f, &ds_sel, value);
        SetFlowL3Ipv4HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetFlowL3Ipv4HashFieldSelect(V, ipHeaderErrorEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetFlowL3Ipv4HashFieldSelect(V, ipOptionsEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetFlowL3Ipv4HashFieldSelect(V, ipRoutedPacketEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetFlowL3Ipv4HashFieldSelect(V, ipSaEn_f, &ds_sel, value);
        SetFlowL3Ipv4HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        SetFlowL3Ipv4HashFieldSelect(V, isVxlanEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetFlowL3Ipv4HashFieldSelect(V, l4DestPortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetFlowL3Ipv4HashFieldSelect(V, l4SourcePortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        /*ipv4hashkey default share mode: use u1_g1, need map ip-protocol to l4-type*/
        SetFlowL3Ipv4HashFieldSelect(V, layer3HeaderProtocolEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetFlowL3Ipv4HashFieldSelect(V, layer4TypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        {
            uint8 tmp_data = GetFlowL3Ipv4HashFieldSelect(V, tcpFlagsEn_f, &ds_sel);
            SetFlowL3Ipv4HashFieldSelect(V, tcpFlagsEn_f, &ds_sel, is_add ? tmp_data | 0x3f :tmp_data&0xC0 );
        }
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        {
            uint8 tmp_data = GetFlowL3Ipv4HashFieldSelect(V, tcpFlagsEn_f, &ds_sel);
            SetFlowL3Ipv4HashFieldSelect(V, tcpFlagsEn_f, &ds_sel, is_add ? tmp_data | 0xC0 :tmp_data&0x3F );
        }
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetFlowL3Ipv4HashFieldSelect(V, ttlEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VRFID:
        SetFlowL3Ipv4HashFieldSelect(V, vrfidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VN_ID:
        SetFlowL3Ipv4HashFieldSelect(V, vxlanVniEn_f, &ds_sel, value);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
		return CTC_E_NOT_SUPPORT;

    }

    cmd = DRV_IOW(FlowL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_ipv6_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    uint32 cmd  = 0;
    uint32 value = 0;
    FlowL3Ipv6HashFieldSelect_m   ds_sel;

    sal_memset(&ds_sel, 0, sizeof(FlowL3Ipv6HashFieldSelect_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM," %% PARAM: field_sel_id %u -- key_field_type %u\n", field_sel_id, sel_field->type);

    if(is_add)
    {
        value = 1;
    }

    cmd = DRV_IOR(FlowL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetFlowL3Ipv6HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetFlowL3Ipv6HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetFlowL3Ipv6HashFieldSelect(V, ecnEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        SetFlowL3Ipv6HashFieldSelect(V, fragInfoEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            ctc_field_port_t* pport_sel = (ctc_field_port_t*)sel_field->ext_data;
            CTC_PTR_VALID_CHECK(sel_field->ext_data);
            if(CTC_FIELD_PORT_TYPE_GPORT == pport_sel->type)
            {
                SetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_GPORT);
            }
            if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport_sel->type)
            {
                SetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_LOGIC_PORT);
            }
        }
        else
        {
            SetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_NONE);
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        SetFlowL3Ipv6HashFieldSelect(V, globalPortType_f, &ds_sel, is_add ? ACL_HASH_GPORT_TYPE_METADATA : 0);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetFlowL3Ipv6HashFieldSelect(V, greKeyEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetFlowL3Ipv6HashFieldSelect(V, nvgreVsidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        SetFlowL3Ipv6HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        SetFlowL3Ipv6HashFieldSelect(V, icmpTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        SetFlowL3Ipv6HashFieldSelect(V, icmpTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IPV6_DA:
        SetFlowL3Ipv6HashFieldSelect(V, ipDaEn_f, &ds_sel, value);
        SetFlowL3Ipv6HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetFlowL3Ipv6HashFieldSelect(V, ipHeaderErrorEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetFlowL3Ipv6HashFieldSelect(V, ipOptionsEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetFlowL3Ipv6HashFieldSelect(V, ipRoutedPacketEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        SetFlowL3Ipv6HashFieldSelect(V, ipSaEn_f, &ds_sel, value);
        SetFlowL3Ipv6HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        SetFlowL3Ipv6HashFieldSelect(V, ipv6FlowLabelEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        SetFlowL3Ipv6HashFieldSelect(V, isVxlanEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetFlowL3Ipv6HashFieldSelect(V, l4DestPortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetFlowL3Ipv6HashFieldSelect(V, l4SourcePortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        SetFlowL3Ipv6HashFieldSelect(V, layer3HeaderProtocolEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetFlowL3Ipv6HashFieldSelect(V, layer4TypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetFlowL3Ipv6HashFieldSelect(V, ttlEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VRFID:
        SetFlowL3Ipv6HashFieldSelect(V, vrfIdEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VN_ID:
        SetFlowL3Ipv6HashFieldSelect(V, vxlanVniEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IS_ROUTER_MAC:
        SetFlowL3Ipv6HashFieldSelect(V, isRouteMacEn_f, &ds_sel, value);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    cmd = DRV_IOW(FlowL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_mpls_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    uint32 cmd  = 0;
    uint32 value = 0;
    FlowL3MplsHashFieldSelect_m   ds_sel;
    sal_memset(&ds_sel, 0, sizeof(FlowL3MplsHashFieldSelect_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM," %% PARAM: field_sel_id %u -- key_field_type %u\n", field_sel_id, sel_field->type);

    if(is_add)
    {
        value = 1;
    }

    cmd = DRV_IOR(FlowL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            ctc_field_port_t* pport_sel = (ctc_field_port_t*)sel_field->ext_data;
            CTC_PTR_VALID_CHECK(sel_field->ext_data);
            if(CTC_FIELD_PORT_TYPE_GPORT == pport_sel->type)
            {
                SetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_GPORT);
            }
            if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport_sel->type)
            {
                SetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_LOGIC_PORT);
            }
        }
        else
        {
            SetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_NONE);
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        SetFlowL3MplsHashFieldSelect(V, globalPortType_f, &ds_sel, is_add ? ACL_HASH_GPORT_TYPE_METADATA : 0);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetFlowL3MplsHashFieldSelect(V, labelNumEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        SetFlowL3MplsHashFieldSelect(V, mplsExp0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        SetFlowL3MplsHashFieldSelect(V, mplsExp1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        SetFlowL3MplsHashFieldSelect(V, mplsExp2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        SetFlowL3MplsHashFieldSelect(V, mplsLabel0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        SetFlowL3MplsHashFieldSelect(V, mplsLabel1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        SetFlowL3MplsHashFieldSelect(V, mplsLabel2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        SetFlowL3MplsHashFieldSelect(V, mplsSbit0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        SetFlowL3MplsHashFieldSelect(V, mplsSbit1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        SetFlowL3MplsHashFieldSelect(V, mplsSbit2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        SetFlowL3MplsHashFieldSelect(V, mplsTtl0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        SetFlowL3MplsHashFieldSelect(V, mplsTtl1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        SetFlowL3MplsHashFieldSelect(V, mplsTtl2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VRFID:
        SetFlowL3MplsHashFieldSelect(V, vrfIdEn_f, &ds_sel, value);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    cmd = DRV_IOW(FlowL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_l2l3_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add)
{
    uint32 cmd  = 0;
    uint32 value = 0;
    FlowL2L3HashFieldSelect_m   ds_sel;
    sal_memset(&ds_sel, 0, sizeof(FlowL2L3HashFieldSelect_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM," %% PARAM: field_sel_id %u -- key_field_type %u\n", field_sel_id, sel_field->type);

    if(is_add)
    {
        value = 1;
    }
    cmd = DRV_IOR(FlowL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));
    switch(sel_field->type)
    {
    case CTC_FIELD_KEY_CTAG_CFI:
        SetFlowL2L3HashFieldSelect(V, ctagCfiEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetFlowL2L3HashFieldSelect(V, ctagCosEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
    case CTC_FIELD_KEY_CVLAN_RANGE:
        SetFlowL2L3HashFieldSelect(V, cvlanIdEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetFlowL2L3HashFieldSelect(V, cvlanIdValidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetFlowL2L3HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetFlowL2L3HashFieldSelect(V, dscpEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetFlowL2L3HashFieldSelect(V, ecnEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        SetFlowL2L3HashFieldSelect(V, fragInfoEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            ctc_field_port_t* pport_sel = (ctc_field_port_t*)sel_field->ext_data;
            CTC_PTR_VALID_CHECK(sel_field->ext_data);
            if(CTC_FIELD_PORT_TYPE_GPORT == pport_sel->type)
            {
                SetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_GPORT);
            }
            if(CTC_FIELD_PORT_TYPE_LOGIC_PORT == pport_sel->type)
            {
                SetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_LOGIC_PORT);
            }
        }
        else
        {
            SetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel, ACL_HASH_GPORT_TYPE_NONE);
        }
        break;
    case CTC_FIELD_KEY_METADATA:
        SetFlowL2L3HashFieldSelect(V, globalPortType_f, &ds_sel, is_add ? ACL_HASH_GPORT_TYPE_METADATA : 0);

        break;
	/*	l4_port_field:  0 ;1:gre/nvgre: 2:vxlan;3,l4 port;4:icmp;5 :igmp */
    case CTC_FIELD_KEY_GRE_KEY:
        SetFlowL2L3HashFieldSelect(V, greKeyEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetFlowL2L3HashFieldSelect(V, nvgreVsidEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        SetFlowL2L3HashFieldSelect(V, icmpOpcodeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        SetFlowL2L3HashFieldSelect(V, icmpTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetFlowL2L3HashFieldSelect(V, ipDaEn_f, &ds_sel, value);
        SetFlowL2L3HashFieldSelect(V, ipDaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetFlowL2L3HashFieldSelect(V, ipHeaderErrorEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetFlowL2L3HashFieldSelect(V, ipOptionsEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetFlowL2L3HashFieldSelect(V, ipRoutedPacketEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetFlowL2L3HashFieldSelect(V, ipSaEn_f, &ds_sel, value);
        SetFlowL2L3HashFieldSelect(V, ipSaPrefixLength_f, &ds_sel, is_add ? 0x1F : 0);
        break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        SetFlowL2L3HashFieldSelect(V, isVxlanEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetFlowL2L3HashFieldSelect(V, l4DestPortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetFlowL2L3HashFieldSelect(V, l4SourcePortEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetFlowL2L3HashFieldSelect(V, labelNumEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        SetFlowL2L3HashFieldSelect(V, layer3HeaderProtocolEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetFlowL2L3HashFieldSelect(V, layer3TypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetFlowL2L3HashFieldSelect(V, layer4TypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MAC_DA:
        SetFlowL2L3HashFieldSelect(V, macDaEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MAC_SA:
        SetFlowL2L3HashFieldSelect(V, macSaEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        SetFlowL2L3HashFieldSelect(V, mplsExp0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        SetFlowL2L3HashFieldSelect(V, mplsExp1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        SetFlowL2L3HashFieldSelect(V, mplsExp2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        SetFlowL2L3HashFieldSelect(V, mplsLabel0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        SetFlowL2L3HashFieldSelect(V, mplsLabel1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        SetFlowL2L3HashFieldSelect(V, mplsLabel2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        SetFlowL2L3HashFieldSelect(V, mplsSbit0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        SetFlowL2L3HashFieldSelect(V, mplsSbit1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        SetFlowL2L3HashFieldSelect(V, mplsSbit2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        SetFlowL2L3HashFieldSelect(V, mplsTtl0En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        SetFlowL2L3HashFieldSelect(V, mplsTtl1En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        SetFlowL2L3HashFieldSelect(V, mplsTtl2En_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetFlowL2L3HashFieldSelect(V, stagCfiEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetFlowL2L3HashFieldSelect(V, stagCosEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
    case CTC_FIELD_KEY_SVLAN_RANGE:
        SetFlowL2L3HashFieldSelect(V, svlanIdEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetFlowL2L3HashFieldSelect(V, svlanIdValidEn_f, &ds_sel, value);
        break;

    case CTC_FIELD_KEY_TCP_ECN:
        SetFlowL2L3HashFieldSelect(V, tcpEcnEn_f, &ds_sel, is_add ? 0x7 :0);
        break;

    case CTC_FIELD_KEY_TCP_FLAGS:
        SetFlowL2L3HashFieldSelect(V, tcpFlagsEn_f, &ds_sel, is_add ? 0xff :0);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetFlowL2L3HashFieldSelect(V, ttlEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetFlowL2L3HashFieldSelect(V, unknownPktEtherTypeEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VRFID:
        SetFlowL2L3HashFieldSelect(V, vrfIdEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_VN_ID:
        SetFlowL2L3HashFieldSelect(V, vxlanVniEn_f, &ds_sel, value);
        break;
    case CTC_FIELD_KEY_IS_ROUTER_MAC:
        SetFlowL2L3HashFieldSelect(V, isRouteMacEn_f, &ds_sel, value);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " The filed:%d not supported \n",sel_field->type);
			return CTC_E_NOT_SUPPORT;

    }
    cmd = DRV_IOW(FlowL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &ds_sel));
    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_dsflow_field(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data0       = 0;
    uint32 data1       = 0;
    uint8  profile_id  = 0;
    uint8  old_profile_id  = 0;
    uint32 fwdptr      = 0;
    uint32 stats_ptr   = 0;
    uint32 dest_map    = 0;
    uint32 nexthop_id  = 0;
    uint8 have_dsfwd   = 0;
    uint8 service_queue_egress_en = 0;
    sys_nh_info_dsnh_t nhinfo;
    sys_cpu_reason_info_t   reason_info;
    sys_qos_policer_param_t policer_param;
    sys_acl_buffer_t*  p_buffer = NULL;
    ctc_acl_to_cpu_t*  p_cp_to_cpu = NULL;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&reason_info, 0, sizeof(sys_cpu_reason_info_t));
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));

    if(is_add)
    {
        data0 = action_field->data0;
        data1 = action_field->data1;
    }

    switch(action_field->type)
    {
    case CTC_ACL_FIELD_ACTION_CANCEL_ALL:
	    reason_info.reason_id = pe->cpu_reason_id;
		sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info);
        _sys_usw_acl_unbind_nexthop(lchip, pe);

        profile_id = GetDsFlow(V, truncateLenProfId_f, p_buffer->action);
        CTC_ERROR_RETURN(sys_usw_register_remove_truncation_profile(lchip, 1, profile_id));

        sal_memset(p_buffer->action, 0, sizeof(p_buffer->action));
        pe->action_flag = 0;
        pe->cpu_reason_id = 0;
        pe->nexthop_id = 0;
        break;
    case CTC_ACL_FIELD_ACTION_DENY_BRIDGE:
        SetDsFlow(V, denyBridge_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DENY_LEARNING:
        SetDsFlow(V, denyLearning_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DENY_ROUTE:
        SetDsFlow(V, denyRoute_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG:
        SetDsFlow(V, disableElephantFlowLog_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DISCARD:
        if(is_add)
        {
            uint32 cmd = 0;
            uint32 field_val = 0;
            cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_aclDiscardType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            SetDsFlow(V, discard_f, p_buffer->action, 1);
            SetDsFlow(V, discardType_f, p_buffer->action, field_val); /*IPEDISCARDTYPE_DS_ACL_DIS*/
            pe->action_flag |= SYS_ACL_ACTION_FLAG_DISCARD;
        }
        else
        {
            SetDsFlow(V, discard_f, p_buffer->action, 0);
            SetDsFlow(V, discardType_f, p_buffer->action, 0);
            pe->action_flag &= (~SYS_ACL_ACTION_FLAG_DISCARD);
        }
        break;
    case CTC_ACL_FIELD_ACTION_DSCP:
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsFlow(V, dscp_f, p_buffer->action, data0);
        SetDsFlow(V, dscpValid_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_CP_TO_CPU:
        /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2,igmp use normal exception to cpu,else use fatal exception to cpu */
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &data1);
        if(pe->igmp_snooping && (data1 ==CTC_GLOBAL_IGMP_SNOOPING_MODE_1))/*igmp snoop don't support */
        {
           SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " igmp snoop don't support ACTION_CP_TO_CPU \n");
           return CTC_E_INVALID_CONFIG;
        }
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_cp_to_cpu = (ctc_acl_to_cpu_t*)action_field->ext_data;
            CTC_MAX_VALUE_CHECK(p_cp_to_cpu->mode, CTC_ACL_TO_CPU_MODE_MAX-1);

            if(p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU)
            {
                SetDsFlow(V, exceptionToCpuType_f, p_buffer->action, 2);
                if(pe->cpu_reason_id)
            	{
                    reason_info.reason_id = pe->cpu_reason_id;
                    sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info);
            	}
                pe->cpu_reason_id = 0;
            }
            else
            {
                uint16 old_cpu_reason_id = pe->cpu_reason_id;
                p_cp_to_cpu->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)?
                                              CTC_PKT_CPU_REASON_ACL_MATCH : p_cp_to_cpu->cpu_reason_id;
                if(old_cpu_reason_id)
                {
                    if(p_cp_to_cpu->cpu_reason_id == old_cpu_reason_id)
                    {
                        return CTC_E_NONE;
                    }
                    reason_info.reason_id = old_cpu_reason_id;
                    CTC_ERROR_RETURN(sys_usw_cpu_reason_free_exception_index(lchip, data1, &reason_info));
                }
                data0 = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER) ? 3:1;
                reason_info.reason_id = p_cp_to_cpu->cpu_reason_id;
                data1 = (CTC_INGRESS == pe->group->group_info.dir) ? CTC_INGRESS : CTC_EGRESS;
        		CTC_ERROR_RETURN(sys_usw_cpu_reason_alloc_exception_index(lchip, data1, &reason_info));
        		SetDsFlow(V, exceptionIndex_f, p_buffer->action, reason_info.exception_index);
                SetDsFlow(V, exceptionSubIndex_f, p_buffer->action, reason_info.exception_subIndex);
                SetDsFlow(V, exceptionToCpuType_f, p_buffer->action,  data0);
        		pe->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)? 0 : reason_info.reason_id;
            }
            pe->action_flag |= SYS_ACL_ACTION_FLAG_LOG;
        }
        else
        {
            if(pe->cpu_reason_id)
        	{
                reason_info.reason_id = pe->cpu_reason_id;
                sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info);
        	}
            SetDsFlow(V, exceptionToCpuType_f, p_buffer->action,  0);
            pe->action_flag &= (~SYS_ACL_ACTION_FLAG_LOG);
    		pe->cpu_reason_id = 0;
        }
        break;
	 case CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH:
        if (!DRV_FLD_IS_EXISIT(DsFlow_t, DsFlow_replaceLoadBalanceHash_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            ctc_acl_lb_hash_t   *p_data = action_field->ext_data;
            CTC_PTR_VALID_CHECK(p_data);
			CTC_MAX_VALUE_CHECK(p_data->mode,2);
			if(p_data->mode == 0)
			{
			  SetDsFlow(V, replaceLoadBalanceHash_f, p_buffer->action, 1);
			}
			else  if(p_data->mode == 1)
			{
			   SetDsFlow(V, replaceLoadBalanceHash_f, p_buffer->action, 2);
			}
			else
			{
			  SetDsFlow(V, replaceLoadBalanceHash_f, p_buffer->action, 3);
			}
            SetDsFlow(V, loadBalanceHashValue_f, p_buffer->action, p_data->lb_hash);
        }
        else
        {
            SetDsFlow(V, replaceLoadBalanceHash_f, p_buffer->action, 0);
            SetDsFlow(V, loadBalanceHashValue_f, p_buffer->action, 0);
        }
        break;
    case CTC_ACL_FIELD_ACTION_METADATA:
        CTC_MAX_VALUE_CHECK(data0, 0x3FFF);
        SetDsFlow(V, metadata_f, p_buffer->action, data0);
        SetDsFlow(V, metadataType_f, p_buffer->action, is_add ? 1 : 0); /*ACLMETADATATYPE_METADATA*/
        break;

   case CTC_ACL_FIELD_ACTION_REDIRECT_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, data0, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nexthop_id));
            data0 = nexthop_id;
        }
          /*not need break,continue to CTC_ACL_FIELD_ACTION_REDIRECT*/
    case CTC_ACL_FIELD_ACTION_REDIRECT:
         /*DsFlow u1_g1 and u1_g2 cover each other,so need not set u1_type and u1_bitmap */
        if(is_add)
        {
            uint8 adjust_len_idx = 0;
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data0, &nhinfo, 0));
            _sys_usw_acl_unbind_nexthop(lchip, pe);
            have_dsfwd = (nhinfo.dsfwd_valid || CTC_IS_BIT_SET(nhinfo.dsnh_offset, 17) || nhinfo.cloud_sec_en); /* bit 17 used for bypass ingress edit, acl hash merge dsfwd mode not support */;
            if(!have_dsfwd && !nhinfo.ecmp_valid)
            {
                have_dsfwd |= (CTC_E_NONE != _sys_usw_acl_bind_nexthop(lchip, pe,data0,1));
            }
            if (nhinfo.ecmp_valid)
            {
                SetDsFlow(V, ecmpEn_f, p_buffer->action, 1);
		        SetDsFlow(V, priorityPathEn_f, p_buffer->action,0);
                SetDsFlow(V, dsFwdPtrOrEcmpGroupId_f, p_buffer->action, nhinfo.ecmp_group_id);
                SetDsFlow(V, nextHopPtrValid_f, p_buffer->action, 0);

            }
            else if(have_dsfwd)
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data0, &fwdptr, 0));
                SetDsFlow(V, dsFwdPtrOrEcmpGroupId_f, p_buffer->action, fwdptr | (1 << 16));
                SetDsFlow(V, ecmpEn_f, p_buffer->action, 0);
		        SetDsFlow(V, priorityPathEn_f, p_buffer->action,0);
                SetDsFlow(V, nextHopPtrValid_f, p_buffer->action, 0);

            }
            else
            {
                dest_map = nhinfo.dest_map;
                SetDsFlow(V, nextHopPtrValid_f, p_buffer->action, 1);
                SetDsFlow(V, adDestMap_f, p_buffer->action, dest_map);
                SetDsFlow(V, adNextHopPtr_f, p_buffer->action, nhinfo.dsnh_offset);
                SetDsFlow(V, adApsBridgeEn_f, p_buffer->action, nhinfo.aps_en);
                SetDsFlow(V, adNextHopExt_f, p_buffer->action, nhinfo.nexthop_ext);
                if (nhinfo.adjust_len)
                {
                    sys_usw_lkup_adjust_len_index(lchip, nhinfo.adjust_len, &adjust_len_idx);
                    SetDsFlow(V, adLengthAdjustType_f, p_buffer->action, adjust_len_idx);
                }
                else
                {
                    SetDsFlow(V, adLengthAdjustType_f, p_buffer->action, 0);
                }
            }
            SetDsFlow(V, denyBridge_f, p_buffer->action, 1);
            SetDsFlow(V, denyRoute_f, p_buffer->action, 1);
            pe->action_flag |= SYS_ACL_ACTION_FLAG_REDIRECT;
            pe->nexthop_id = data0;
        }
        else
        {
            SetDsFlow(V, nextHopPtrValid_f, p_buffer->action, 0);
            SetDsFlow(V, dsFwdPtrOrEcmpGroupId_f, p_buffer->action, 0);
            SetDsFlow(V, ecmpEn_f, p_buffer->action, 0);
            SetDsFlow(V, priorityPathEn_f, p_buffer->action, 0);
            SetDsFlow(V, denyBridge_f, p_buffer->action, 0);
            SetDsFlow(V, denyRoute_f, p_buffer->action, 0);
            pe->nexthop_id = 0;
            pe->action_flag &= (~SYS_ACL_ACTION_FLAG_REDIRECT);
            _sys_usw_acl_unbind_nexthop(lchip, pe);
        }
        break;
    case CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp,CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
        }
        SetDsFlow(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsFlow(V, policerPtr_f, p_buffer->action, is_add ?policer_param.policer_ptr:0);
        SetDsFlow(V, policerPhbEn_f, p_buffer->action, is_add?policer_param.is_bwp:0);
        break;
    case CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER:
        if(is_add)
        {
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            CTC_NOT_EQUAL_CHECK(data0, 0);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_MACRO_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
        }
        SetDsFlow(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsFlow(V, policerPtr_f, p_buffer->action, is_add ?policer_param.policer_ptr:0);
        SetDsFlow(V, policerPhbEn_f, p_buffer->action, is_add?policer_param.is_bwp:0);
        break;
    case CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER:
        if(is_add)
        {
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER);
            CTC_NOT_EQUAL_CHECK(data0, 0);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0)
                || (policer_param.is_bwp == 0) || (data1 > (MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL)-1)))
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
            pe->cos_index = data1;
        }
        SetDsFlow(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsFlow(V, policerPtr_f, p_buffer->action, is_add ?(policer_param.policer_ptr+data1):0);
        SetDsFlow(V, policerPhbEn_f, p_buffer->action, 0);
        break;
    case CTC_ACL_FIELD_ACTION_COPP:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            policer_param.dir = CTC_INGRESS;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
            (lchip, data0, SYS_QOS_POLICER_TYPE_COPP, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }

            if (policer_param.is_bwp)
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;

        }
        SetDsFlow(V, policerLvlSel_f, p_buffer->action, 0);
        SetDsFlow(V, policerPtr_f, p_buffer->action, is_add ? (policer_param.policer_ptr | (1 << 12)) : 0);/*TempDsAcl.policerPtr(12)&&(!TempDsAcl.policerLvlSel)*/

        break;
    case CTC_ACL_FIELD_ACTION_STATS:
        if(is_add)
        {
            sys_stats_statsid_t statsid;
            sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
            statsid.dir             = CTC_INGRESS;
            statsid.stats_id        = data0;
            statsid.stats_id_type   = SYS_STATS_TYPE_FLOW_HASH;
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr_with_check(lchip, &statsid, &stats_ptr));
            pe->stats_id = data0;
        }
        SetDsFlow(V, statsPtr_f, p_buffer->action, stats_ptr & 0xFFFF);
        break;
    case CTC_ACL_FIELD_ACTION_TRUNCATED_LEN:
        CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
        if(service_queue_egress_en)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            old_profile_id = GetDsFlow(V, truncateLenProfId_f, p_buffer->action);
            CTC_ERROR_RETURN(sys_usw_register_add_truncation_profile(lchip, data0, old_profile_id, &profile_id));
        }
        else
        {
            profile_id = GetDsFlow(V, truncateLenProfId_f, p_buffer->action);
            CTC_ERROR_RETURN(sys_usw_register_remove_truncation_profile(lchip, 1, profile_id));
            profile_id = 0;

        }

        SetDsFlow(V, truncateLenProfId_f, p_buffer->action, profile_id);
        break;
    case CTC_ACL_FIELD_ACTION_CRITICAL_PKT:
        SetDsFlow(V, criticalPacket_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_LOGIC_PORT:
        CTC_MAX_VALUE_CHECK(data0, 0x7FFF);
        SetDsFlow(V, logicSrcPort_f, p_buffer->action, data0);
        break;

    case CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN:
        CTC_MAX_VALUE_CHECK(data0, 7);
        if(is_add)
        {
            if (DRV_FLD_IS_EXISIT(DsFlow_t, DsFlow_disableAclLookupLevel_f))
            {
                SetDsFlow(V, disableAcl_f, p_buffer->action, 1);
                SetDsFlow(V, disableAclLookupLevel_f, p_buffer->action, data0);
            }
            else
            {
                SetDsFlow(V, disableAcl_f, p_buffer->action, (1 << data0));
            }
        }
        else
        {
            SetDsFlow(V, disableAcl_f, p_buffer->action, 0);
        }
        break;
    case CTC_ACL_FIELD_ACTION_DEST_CID:
        if(is_add)
        {
            SYS_USW_CID_CHECK(lchip,data0);
			is_add = data0 ? 1:0;	/*disable cid*/
        }
		 if(GetDsFlow(V, categoryIdValid_f, p_buffer->action)
				&& GetDsFlow(V, categoryIdType_f, p_buffer->action))
		{
			 return CTC_E_PARAM_CONFLICT;
		}
		SetDsFlow(V, categoryIdisHigerPri_f,p_buffer->action,0);
		SetDsFlow(V, categoryIdValid_f, p_buffer->action,is_add ?1:0);
        SetDsFlow(V, categoryIdType_f, p_buffer->action, 0);
        SetDsFlow(V, categoryId_f, p_buffer->action, is_add ?data0:0);
        break;
    case CTC_ACL_FIELD_ACTION_SRC_CID:
        if(is_add)
        {
            SYS_USW_CID_CHECK(lchip,data0);
			is_add = data0 ? 1:0;	/*disable cid*/
        }
		 if(GetDsFlow(V, categoryIdValid_f, p_buffer->action)
			&& !GetDsFlow(V, categoryIdType_f, p_buffer->action))
		{
			 return CTC_E_PARAM_CONFLICT;
		}

		SetDsFlow(V, categoryIdisHigerPri_f,p_buffer->action,0);
        SetDsFlow(V, categoryIdValid_f, p_buffer->action,is_add ?1:0);
        SetDsFlow(V, categoryIdType_f, p_buffer->action, is_add ?1:0);
        SetDsFlow(V, categoryId_f, p_buffer->action, is_add ?data0:0);
        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR :
        SetDsFlow(V, disableEcmpRr_f, p_buffer->action, (is_add? 1: 0));
        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR :
        SetDsFlow(V, disablePortLagRr_f, p_buffer->action, (is_add? 1: 0));
        break;
    case CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE :
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsFlow(V, modifyPortBasedHashProfile0_f, p_buffer->action, (is_add? 1: 0));
        SetDsFlow(V, newPortBasedHashProfileId0_f, p_buffer->action, data0);
        break;
    case CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE :
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsFlow(V, modifyPortBasedHashProfile1_f, p_buffer->action, (is_add? 1: 0));
        SetDsFlow(V, newPortBasedHashProfileId1_f, p_buffer->action, data0);
        break;
    case CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT:
        if (is_add)
        {
            /* must be used together with redirect action */
            SetDsFlow(V, payloadOffsetType_f, p_buffer->action, 1);
        }
        else
        {
            SetDsFlow(V, payloadOffsetType_f, p_buffer->action, 0);
        }
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    SYS_USW_ACL_SET_BMP(pe->action_bmp, action_field->type, is_add);
    if(CTC_ACL_FIELD_ACTION_CANCEL_ALL == action_field->type)
    {
        sal_memset(pe->action_bmp, 0, sizeof(pe->action_bmp));
    }
     pe->action_type = SYS_ACL_ACTION_HASH_INGRESS;

    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_field_action_hash_igs(uint8 lchip, ctc_acl_field_action_t* p_action, sys_acl_entry_t* pe)
{
    void* p_action_buffer = pe->buffer->action;
    sys_nh_info_dsnh_t nhinfo = {0};
    uint8 temp_data1 = 0;
    uint8  shift = 0;
    uint16 length = 0;
    uint32 cmd = 0;
    DsTruncationProfile_m truncation_prof;

    switch(p_action->type)
    {
        case CTC_ACL_FIELD_ACTION_CP_TO_CPU:
        {
            ctc_acl_to_cpu_t* p_copy_to_cpu = p_action->ext_data;
            CTC_PTR_VALID_CHECK(p_copy_to_cpu);
            p_copy_to_cpu->mode = GetDsFlow(V, exceptionToCpuType_f, p_action_buffer);
            if(3 == p_copy_to_cpu->mode)
            {
                p_copy_to_cpu->mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
            }
            p_copy_to_cpu->cpu_reason_id = pe->cpu_reason_id;
        }
        break;
        case CTC_ACL_FIELD_ACTION_STATS:
            p_action->data0 = pe->stats_id;
            break;
      /*   case CTC_ACL_FIELD_ACTION_RANDOM_LOG:  // only tcam have*/
      /*       break;*/
      /*   case CTC_ACL_FIELD_ACTION_COLOR:   // only tcam have*/
      /*       break;*/
        case CTC_ACL_FIELD_ACTION_DSCP:
            if(GetDsFlow(V, dscpValid_f, p_action_buffer))
            {
                p_action->data0 = GetDsFlow(V, dscp_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER:
            p_action->data0 = pe->policer_id;
            p_action->data1 = pe->cos_index;
            break;
        case CTC_ACL_FIELD_ACTION_COPP:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_SRC_CID:
            if((GetDsFlow(V, categoryIdValid_f, p_action_buffer))&& (1 == GetDsFlow(V, categoryIdType_f, p_action_buffer)))
            {
                p_action->data0 = GetDsFlow(V, categoryId_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_TRUNCATED_LEN:
            temp_data1 = GetDsFlow(V, truncateLenProfId_f, p_action_buffer);
            if (temp_data1)
            {
                sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
                cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, temp_data1, cmd, &truncation_prof));

                shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
                length = GetDsTruncationProfile(V, length_f, &truncation_prof);
                length = length << shift;
            }
            p_action->data0 = length;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT:
            p_action->data0 = pe->nexthop_id;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT_PORT:
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pe->nexthop_id, &nhinfo, 0));
            p_action->data0 = nhinfo.gport;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT:
            p_action->data0 = GetDsFlow(V, payloadOffsetType_f, p_action_buffer);
            break;
      /*   case CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT:  // only tcam have*/
      /*       break;*/
        case CTC_ACL_FIELD_ACTION_DISCARD: /* hash discard all*/
            p_action->data1 = GetDsFlow(V, discard_f, p_action_buffer);
            break;

      /*   case CTC_ACL_FIELD_ACTION_CANCEL_DISCARD:  // only tcam have*/
      /*       break;*/
      /*   case CTC_ACL_FIELD_ACTION_PRIORITY:   // only tcam have*/
      /*       break;*/
        case CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG:
            p_action->data0 = GetDsFlow(V, disableElephantFlowLog_f, p_action_buffer);
            break;
      /*   case CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR:   // only tcam have*/
      /*       break;*/
      /*   case CTC_ACL_FIELD_ACTION_CANCEL_NAT:          // only tcam have*/
      /*       break;*/
     /*    case CTC_ACL_FIELD_ACTION_IPFIX:                 // only tcam have*/
      /*       break;*/
     /*    case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX:*/
     /*        break;*/
     /*    case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING:   // only tcam have*/
     /*        break;*/
      /*   case CTC_ACL_FIELD_ACTION_OAM:  // only tcam have*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH:
            {
                ctc_acl_lb_hash_t* p_lb_hash = p_action->ext_data;
                if (!DRV_FLD_IS_EXISIT(DsFlow_t, DsFlow_replaceLoadBalanceHash_f))
                {
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_PTR_VALID_CHECK(p_lb_hash);
                p_lb_hash->lb_hash = GetDsFlow(V, loadBalanceHashValue_f, p_action_buffer);
                p_lb_hash->mode = GetDsFlow(V, replaceLoadBalanceHash_f, p_action_buffer)-1;
            }
            break;
     /*    case CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE:  // only tcam have*/
     /*         break;*/
     /*    case CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT:*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_METADATA:
            p_action->data0 = GetDsFlow(V, metadata_f, p_action_buffer);
            break;
     /*    case CTC_ACL_FIELD_ACTION_INTER_CN:   // only tcam have*/
     /*        break;*/
     /*    case CTC_ACL_FIELD_ACTION_VLAN_EDIT:  // only tcam have*/
     /*        break;*/
     /*    case CTC_ACL_FIELD_ACTION_STRIP_PACKET:  // only tcam have*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_CRITICAL_PKT:
            p_action->data0 = GetDsFlow(V, criticalPacket_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_LOGIC_PORT:
            p_action->data0 = GetDsFlow(V, logicSrcPort_f, p_action_buffer);
            break;
     /*    case CTC_ACL_FIELD_ACTION_SPAN_FLOW:   // only tcam have*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_CANCEL_ALL:
            break;
     /*    case CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP:  // only tcam have*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN:  /*hash*/
            if (DRV_FLD_IS_EXISIT(DsFlow_t, DsFlow_disableAclLookupLevel_f))
            {
                if(GetDsFlow(V, disableAcl_f, p_action_buffer))
                {
                    p_action->data0 = GetDsFlow(V, disableAclLookupLevel_f, p_action_buffer);
                }
            }
            else
            {
                uint8 tmp_index = 0;
                uint32 acl_level = GetDsFlow(V, disableAcl_f, p_action_buffer);
                for (tmp_index=0;tmp_index<8;tmp_index+=1)
                {
                    if (0x1 == (acl_level >> tmp_index))
                        break;
                }
                p_action->data0 = tmp_index;
            }
            break;
        case CTC_ACL_FIELD_ACTION_DENY_BRIDGE:  /*hash*/
            p_action->data0 = GetDsFlow(V, denyBridge_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DENY_LEARNING:
            p_action->data0 = GetDsFlow(V, denyLearning_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DENY_ROUTE:  /*hash*/
            p_action->data0 = GetDsFlow(V, denyRoute_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DEST_CID:      /*hash*/
            if((GetDsFlow(V, categoryIdValid_f, p_action_buffer))&& (0 == GetDsFlow(V, categoryIdType_f, p_action_buffer)))
            {
                p_action->data0 = GetDsFlow(V, categoryId_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR :
            p_action->data0 = GetDsFlow(V, disableEcmpRr_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR :
            p_action->data0 = GetDsFlow(V, disablePortLagRr_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE :
            p_action->data0 = GetDsFlow(V, modifyPortBasedHashProfile0_f, p_action_buffer);
            p_action->data1 = GetDsFlow(V, newPortBasedHashProfileId0_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE :
            p_action->data0 = GetDsFlow(V, modifyPortBasedHashProfile1_f, p_action_buffer);
            p_action->data1 = GetDsFlow(V, newPortBasedHashProfileId1_f, p_action_buffer);
            break;
        default:
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_update_dsflow_action(uint8 lchip, void* data, void* change_nh_param)
{
    sys_acl_entry_t* pe = (sys_acl_entry_t*)data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    ds_t   action;
    ds_t   old_action;
    uint32*     p_hash_ad = NULL;
    uint32 cmdr  = 0;
    uint32 cmdw  = 0;
    uint32 fwd_offset = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 ad_index = 0;
    uint32 hw_index = 0;
    uint8 step = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&action, 0, sizeof(ds_t));
    sal_memset(&old_action, 0, sizeof(ds_t));

    _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);

    if (!ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        ad_index = pe->ad_index;
    }
    else
    {
        _sys_usw_acl_get_key_size(lchip, 0, pe, &step);
        ad_index = SYS_ACL_MAP_DRV_AD_INDEX(hw_index, step);
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"key_tbl_id:[%u]  ad_tbl_id:[%u]  ad_index:[%u]\n", key_id, act_id, ad_index);

    cmdr = DRV_IOR(act_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(act_id, DRV_ENTRY_FLAG);
    if (!pe->buffer)
    {
        DRV_IOCTL(lchip, ad_index, cmdr, &action);
    }
    else
    {
        sal_memcpy(&action, &pe->buffer->action, sizeof(action));
    }
    sal_memcpy(&old_action, &action, sizeof(ds_t));
    if (!change_nh_param || p_dsnh_info->cloud_sec_en)
    {
        /*update ad from merge dsfwd to using dsfwd*/
        if(GetDsFlow(V, nextHopPtrValid_f, &action) == 0)
        {
             return CTC_E_NONE;
        }

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwd_offset, 1));

        /*2. update ad*/

        SetDsFlow(V, dsFwdPtrOrEcmpGroupId_f, &action, fwd_offset | (1 << 16));
        SetDsFlow(V, ecmpEn_f, &action, 0);
        SetDsFlow(V, priorityPathEn_f, &action, 0);
        SetDsFlow(V, nextHopPtrValid_f, &action, 0);
        CTC_UNSET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);
    }
    else
    {
        uint8 adjust_len_idx = 0;
        if(GetDsFlow(V, nextHopPtrValid_f, &action) == 0)
        {
             return CTC_E_NONE;
        }

         SetDsFlow(V, nextHopPtrValid_f, &action, 1);
         SetDsFlow(V, adDestMap_f, &action, p_dsnh_info->dest_map);
         SetDsFlow(V, adNextHopPtr_f, &action, p_dsnh_info->dsnh_offset);
         SetDsFlow(V, adApsBridgeEn_f, &action, p_dsnh_info->aps_en);
         SetDsFlow(V, adNextHopExt_f, &action, p_dsnh_info->nexthop_ext);
         SetDsFlow(V, adApsBridgeEn_f, &action, p_dsnh_info->aps_en);
         if (p_dsnh_info->adjust_len)
         {
             sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
             SetDsFlow(V, adLengthAdjustType_f, &action, adjust_len_idx);
         }
         else
         {
             SetDsFlow(V, adLengthAdjustType_f, &action, 0);
         }
    }
    CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->ad_spool, action, old_action, &p_hash_ad));
    pe->hash_action = p_hash_ad;
    pe->ad_index = p_hash_ad[ACL_HASH_AD_INDEX_OFFSET];
    ad_index = p_hash_ad[ACL_HASH_AD_INDEX_OFFSET];
    if (!pe->buffer)
    {
        DRV_IOCTL(lchip, ad_index, cmdw, &action);
    }
    else
    {
        sal_memcpy(&pe->buffer->action, &action, sizeof(action));
    }

    return CTC_E_NONE;
}

#define _ACL_TCAM_
int32
_sys_usw_acl_add_mackey160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint8  c_ether_type = 0;
    sys_acl_buffer_t* p_buffer = NULL;
    hw_mac_addr_t hw_mac = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosMacKey160(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosMacKey160(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        {
            uint8 is_dot1ae = (0x88e5 == data && is_add);     /* dot1ae packet */
            if (CTC_INGRESS == pe->group->group_info.dir)
            {
                SetDsAclQosMacKey160(V, vlanXlate0LookupNoResult_f, p_buffer->key, is_dot1ae);
                SetDsAclQosMacKey160(V, vlanXlate0LookupNoResult_f, p_buffer->mask, is_dot1ae);
            }
            else if (CTC_EGRESS == pe->group->group_info.dir)
            {
                SetDsAclQosMacKey160(V, isDecap_f, p_buffer->key, is_dot1ae);
                SetDsAclQosMacKey160(V, isDecap_f, p_buffer->mask, is_dot1ae);
            }
        }
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, data, pe->ether_type, &c_ether_type,&pe->ether_type_index));
            SetDsAclQosMacKey160(V, cEtherType_f, p_buffer->key, c_ether_type);
            SetDsAclQosMacKey160(V, cEtherType_f, p_buffer->mask, 0x7F);
            pe->ether_type = data;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe->ether_type));
            pe->ether_type = 0;
            SetDsAclQosMacKey160(V, cEtherType_f, p_buffer->key, 0);
            SetDsAclQosMacKey160(V, cEtherType_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosMacKey160(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosMacKey160(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacKey160(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosMacKey160(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosMacKey160(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacKey160(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacKey160(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacKey160(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacKey160(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
		CTC_MAX_VALUE_CHECK(key_field->data,3);
        SetDsAclQosMacKey160(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosMacKey160(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_DA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacKey160(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacKey160(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacKey160(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacKey160(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosMacKey160(V, cfi_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, cfi_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask,is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosMacKey160(V, cos_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, cos_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask,is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask,is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosMacKey160(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacKey160(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, 0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask,is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosMacKey160(V, cfi_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, cfi_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask, is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosMacKey160(V, cos_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, cos_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask, is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->mask, mask);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask, is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosMacKey160(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacKey160(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask, is_add ? 1:0);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->key, is_add ? data : 0);
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->mask,is_add ? 0xFFF :0);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->key, is_add ? data : 0);
        SetDsAclQosMacKey160(V, vlanId_f, p_buffer->mask,is_add ? 0xFFF :0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->key, is_add ? 1: 0);
        SetDsAclQosMacKey160(V, flowL2KeyUseCvlan_f, p_buffer->mask,is_add ? 1: 0);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosMacKey160(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_XLATE_HIT :
        {
            uint32* scl_id = (uint32*)(key_field->ext_data);
	        CTC_PTR_VALID_CHECK(scl_id);
            CTC_MAX_VALUE_CHECK(*scl_id, 1);
            if (*scl_id)      /**< [TM] data = scl id */
            {
                SetDsAclQosMacKey160(V, vlanXlate1LookupNoResult_f, p_buffer->key, (data? 0: 1));
                SetDsAclQosMacKey160(V, vlanXlate1LookupNoResult_f, p_buffer->mask, (mask? 1: 0));
            }
            else
            {
                SetDsAclQosMacKey160(V, vlanXlate0LookupNoResult_f, p_buffer->key, (data? 0: 1));
                SetDsAclQosMacKey160(V, vlanXlate0LookupNoResult_f, p_buffer->mask, (mask? 1: 0));
            }
        }
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosMacKey160(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosMacKey160(V, aclLabel_f, p_buffer->mask, mask);
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}


int32
_sys_usw_acl_add_l3key160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data                    = 0;
    uint32 mask                    = 0;
    uint32 tmp_data                = 0;
    uint32 tmp_mask                = 0;
    uint8  tmp_bmp_data            = 0;
    uint8  tmp_bmp_mask            = 0;
    int32  ret                     = CTC_E_NONE;
    uint32 hw_satpdu_byte[2]       = {0};
    sys_acl_buffer_t*  p_buffer    = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer  = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

     switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosL3Key160(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosL3Key160(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosL3Key160(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosL3Key160(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosL3Key160(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosL3Key160(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosL3Key160(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosL3Key160(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosL3Key160(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosL3Key160(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosL3Key160(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosL3Key160(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosL3Key160(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosL3Key160(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        /*hw no the field*/
		if(pe->l4_type != 0)
		{
		   SetDsAclQosL3Key160(V, layer4Type_f, p_buffer->key, tmp_data);
           SetDsAclQosL3Key160(V, layer4Type_f, p_buffer->mask, tmp_mask);
		}
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosL3Key160(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, routedPacket_f, p_buffer->mask, mask);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosL3Key160(V, dscp_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosL3Key160(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosL3Key160(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosL3Key160(V, ecn_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetDsAclQosL3Key160(V, ipDa_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ipDa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosL3Key160(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetDsAclQosL3Key160(V, ipSa_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ipSa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    /*
        l3Key160ShareFieldType == 0, share field used as l4-port range bitmap
        FlowTcamLookupCtl.l3Key160FullRangeBitMapMode = 1, share dscp

        Now, shareFields used as tcp-flags and tcp-ecn
    */
    case CTC_FIELD_KEY_TCP_ECN:
        tmp_data = GetDsAclQosL3Key160(V, shareFields_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, shareFields_f, p_buffer->mask);
        tmp_data &= 0xFC;
        tmp_mask &= 0xFC;
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
        }
        SetDsAclQosL3Key160(V, shareFields_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, shareFields_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        tmp_data = GetDsAclQosL3Key160(V, shareFields_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, shareFields_f, p_buffer->mask);
        tmp_data &= 0x3;
        tmp_mask &= 0x3;
        if(is_add)
        {
            tmp_data |= (data << 2);
            tmp_mask |= (mask << 2);
        }
        SetDsAclQosL3Key160(V, shareFields_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, shareFields_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF)<< 8);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->key, is_add? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN): 0);
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->mask, is_add? 0xF: 0);
        tmp_data = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->key, is_add? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN): 0);
        SetDsAclQosL3Key160(V, layer4UserType_f, p_buffer->mask, is_add? 0xF: 0);
        tmp_data = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosL3Key160(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosL3Key160(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosL3Key160(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosL3Key160(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosL3Key160(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosL3Key160(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosL3Key160(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosL3Key160(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosL3Key160(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosL3Key160(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key160(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosL3Key160(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosL3Key160(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key160(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosL3Key160(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosL3Key160(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosL3Key160(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosL3Key160(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosL3Key160(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosL3Key160(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosL3Key160(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:

        SetDsAclQosL3Key160(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:

        SetDsAclQosL3Key160(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosL3Key160(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosL3Key160(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:

        SetDsAclQosL3Key160(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosL3Key160(V, ttl_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ttl_f, p_buffer->mask, mask);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        SetDsAclQosL3Key160(V, etherOamLevel_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, etherOamLevel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        SetDsAclQosL3Key160(V, etherOamOpCode_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, etherOamOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        SetDsAclQosL3Key160(V, etherOamVersion_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, etherOamVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosL3Key160(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosL3Key160(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosL3Key160(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosL3Key160(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosL3Key160(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*SAT PDU*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosL3Key160(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosL3Key160(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosL3Key160(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosL3Key160(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosL3Key160(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosL3Key160(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosL3Key160(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosL3Key160(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosL3Key160(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosL3Key160(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosL3Key160(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosL3Key160(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosL3Key160(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosL3Key160(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosL3Key160(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosL3Key160(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*EtherType*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosL3Key160(V, etherType_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, etherType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosL3Key160(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosL3Key160(V, aclLabel_f, p_buffer->mask, mask);
        break;
    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }
     return ret;
}


int32
_sys_usw_acl_add_l3key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32  data = 0;
    uint32  mask = 0;
    uint32  tmp_data = 0;
    uint32  tmp_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint8   l3_type = 0;

    uint32  hw_satpdu_byte[2] = {0};
    hw_mac_addr_t hw_mac = {0};
    sys_acl_buffer_t*     p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    uint32  temp_value_data[2] = {0};
    uint32  temp_value_mask[2] = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }
    l3_type = GetDsAclQosL3Key320(V, layer3Type_f, p_buffer->key);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosL3Key320(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosL3Key320(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosL3Key320(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    /*NSH*/
    case CTC_FIELD_KEY_NSH_CBIT:
        SetDsAclQosL3Key320(V, gNsh_nshCbit_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, gNsh_nshCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
        SetDsAclQosL3Key320(V, gNsh_nshNextProtocol_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, gNsh_nshNextProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_OBIT:
        SetDsAclQosL3Key320(V, gNsh_nshObit_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, gNsh_nshObit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SI:
        SetDsAclQosL3Key320(V, gNsh_nshSi_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, gNsh_nshSi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SPI:
        SetDsAclQosL3Key320(V, gNsh_nshSpi_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, gNsh_nshSpi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;

    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosL3Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosL3Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosL3Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosL3Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosL3Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosL3Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosL3Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosL3Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        if(CTC_EGRESS == pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        SetDsAclQosL3Key320(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosL3Key320(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosL3Key320(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosL3Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosL3Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosL3Key320(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosL3Key320(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosL3Key320(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosL3Key320(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, routedPacket_f, p_buffer->mask, mask);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosL3Key320(V, dscp_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosL3Key320(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosL3Key320(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosL3Key320(V, ecn_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosL3Key320(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetDsAclQosL3Key320(V, ipDa_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ipDa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosL3Key320(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosL3Key320(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetDsAclQosL3Key320(V, ipSa_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ipSa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;

    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;

    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->key, is_add? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN): 0);
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->mask, is_add? 0xF: 0);
        tmp_data = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->key, is_add? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN): 0);
        SetDsAclQosL3Key320(V, layer4UserType_f, p_buffer->mask, is_add? 0xF: 0);
        tmp_data = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosL3Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1 :
        CTC_MAX_VALUE_CHECK(data, 0xFFFFFF);
        SetDsAclQosL3Key320(V, vxlanReserved1_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, vxlanReserved1_f, p_buffer->mask, mask & 0xFFFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2 :
        CTC_MAX_VALUE_CHECK(data, 0xFF);
        SetDsAclQosL3Key320(V, vxlanReserved2_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, vxlanReserved2_f, p_buffer->mask, mask & 0xFF);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosL3Key320(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosL3Key320(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosL3Key320(V, ttl_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ttl_f, p_buffer->mask, mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosL3Key320(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosL3Key320(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosL3Key320(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosL3Key320(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosL3Key320(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosL3Key320(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosL3Key320(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosL3Key320(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosL3Key320(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosL3Key320(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosL3Key320(V, u1_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, u1_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosL3Key320(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosL3Key320(V, u1_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, u1_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosL3Key320(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosL3Key320(V, u1_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, u1_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosL3Key320(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosL3Key320(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosL3Key320(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosL3Key320(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosL3Key320(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if (is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if (is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosL3Key320(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosL3Key320(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosL3Key320(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosL3Key320(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosL3Key320(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosL3Key320(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosL3Key320(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosL3Key320(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:
        SetDsAclQosL3Key320(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosL3Key320(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosL3Key320(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosL3Key320(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosL3Key320(V, u1_g5_ttl_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, u1_g5_ttl_f, p_buffer->mask, mask);
        break;
    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosL3Key320(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosL3Key320(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosL3Key320(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosL3Key320(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosL3Key320(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*PDU*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosL3Key320(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosL3Key320(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosL3Key320(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosL3Key320(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosL3Key320(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosL3Key320(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosL3Key320(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosL3Key320(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosL3Key320(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosL3Key320(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosL3Key320(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosL3Key320(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosL3Key320(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosL3Key320(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosL3Key320(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosL3Key320(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*Ether Type*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosL3Key320(V, etherType_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, etherType_f, p_buffer->mask, mask);
        break;
    /*Wlan Info*/
    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosL3Key320(A, radioMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosL3Key320(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_SET(pe->wlan_bmp, 0);
        }
        else
        {
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            SetDsAclQosL3Key320(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_UNSET(pe->wlan_bmp, 0);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_RADIO_ID:
        SetDsAclQosL3Key320(V, rid_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, rid_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask,0x01);
            CTC_BIT_SET(pe->wlan_bmp, 1);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp, 1);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_CTL_PKT:
        SetDsAclQosL3Key320(V, capwapControlPacket_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, capwapControlPacket_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            CTC_BIT_SET(pe->wlan_bmp, 2);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp, 2);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        SetDsAclQosL3Key320(V, vrfId_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, vrfId_f, p_buffer->mask, mask);
        break;

    /*Aware Tunnel Info*/
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
        p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);

        SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->key, 0);
        SetDsAclQosL3Key320(V, isCapwapInfo_f, p_buffer->mask, is_add ? 0x01:0);
        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            SetDsAclQosL3Key320(V, mergeDataType_f, p_buffer->key,drv_merge_type);
            SetDsAclQosL3Key320(V, mergeDataType_f, p_buffer->mask,drv_merge_type_mask);
            if(p_tunnel_data ->type == 3)
            {
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                sal_memcpy((uint8*)temp_value_data, &hw_mac, sizeof(mac_addr_t));
                temp_value_data[1] = (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_data ->radio_id & 0x1f) << 16 | (temp_value_data[1] & 0xFFFF);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                sal_memcpy((uint8*)temp_value_mask, &hw_mac, sizeof(mac_addr_t));
                temp_value_mask[1] = (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_mask ->radio_id & 0x1f) << 16 | (temp_value_mask[1] & 0xFFFF);

                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 2)
            {
                temp_value_data[0] = p_tunnel_data ->gre_key;
                temp_value_mask[0] = p_tunnel_mask ->gre_key;
                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 1)
            {
                temp_value_data[0] = p_tunnel_data ->vxlan_vni;
                temp_value_mask[0] = p_tunnel_mask ->vxlan_vni;
                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosL3Key320(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
        }
        else
        {
            SetDsAclQosL3Key320(V, mergeDataType_f, p_buffer->key,0);
            SetDsAclQosL3Key320(V, mergeDataType_f, p_buffer->mask,0);
            SetDsAclQosL3Key320(A, mergeData_f, p_buffer->key, temp_value_data);
            SetDsAclQosL3Key320(A, mergeData_f, p_buffer->mask, temp_value_mask);
        }
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosL3Key320(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosL3Key320(V, aclLabel_f, p_buffer->mask, mask);
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_INVALID_PARAM;

    }
     return CTC_E_NONE;
}


int32
_sys_usw_acl_add_macl3key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint8  l3_type = 0;
    uint8  cvlanId_compound_data = 0;
    uint8  cvlanId_compound_mask = 0;
    uint32 mac_da_compound_data[2] = {0};
    uint32 mac_da_compound_mask[2] = {0};
    uint32 hw_satpdu_byte[2] = {0};
    hw_mac_addr_t    hw_mac={0};
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    sys_acl_buffer_t*     p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }
    l3_type = GetDsAclQosMacL3Key320(V, layer3Type_f, p_buffer->key);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosMacL3Key320(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosMacL3Key320(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosMacL3Key320(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosMacL3Key320(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosMacL3Key320(V, ctagCfi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ctagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosMacL3Key320(V, ctagCos_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ctagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosMacL3Key320(V, cvlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, cvlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacL3Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacL3Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosMacL3Key320(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosMacL3Key320(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        if (DRV_IS_DUET2(lchip))
        {
            SetDsAclQosMacL3Key320(V, isMcastRpfCheckFail_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        }
        else
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosMacL3Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosMacL3Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosMacL3Key320(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_l4_type_to_ip_protocol
            (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosMacL3Key320(V, layer3HeaderProtocol_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, layer3HeaderProtocol_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        {
            uint8 is_ip_packet = (CTC_PARSER_L3_TYPE_IPV4 == data);
            SetDsAclQosMacL3Key320(V, isIpPacket_f, p_buffer->key, is_ip_packet);
            SetDsAclQosMacL3Key320(V, isIpPacket_f, p_buffer->mask, is_ip_packet);
        }
        SetDsAclQosMacL3Key320(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_DA:
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
        SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, hw_mac);
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
        SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
        SetDsAclQosMacL3Key320(A, macSa_f, p_buffer->key, hw_mac);
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
        SetDsAclQosMacL3Key320(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosMacL3Key320(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosMacL3Key320(V, stagCfi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, stagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosMacL3Key320(V, stagCos_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, stagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosMacL3Key320(V, svlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, svlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosMacL3Key320(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, svlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosMacL3Key320(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, svlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, 0xFFF);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosMacL3Key320(V, dscp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosMacL3Key320(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosMacL3Key320(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosMacL3Key320(V, ecn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosMacL3Key320(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetDsAclQosMacL3Key320(V, ipDa_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ipDa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosMacL3Key320(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosMacL3Key320(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetDsAclQosMacL3Key320(V, ipSa_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ipSa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if (is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
     case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        tmp_data = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosMacL3Key320(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosMacL3Key320(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosMacL3Key320(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosMacL3Key320(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosMacL3Key320(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosMacL3Key320(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
         /*add check only one in two,default not share */
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key320(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key320(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_TARGET_MAC:
         /*add check only one in two,default not share */
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
        SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, hw_mac);
        ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
        SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosMacL3Key320(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosMacL3Key320(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosMacL3Key320(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosMacL3Key320(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosMacL3Key320(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key320(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key320(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key320(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, u1_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key320(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosMacL3Key320(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosMacL3Key320(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosMacL3Key320(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosMacL3Key320(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosMacL3Key320(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosMacL3Key320(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosMacL3Key320(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosMacL3Key320(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosMacL3Key320(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosMacL3Key320(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosMacL3Key320(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosMacL3Key320(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:
        SetDsAclQosMacL3Key320(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosMacL3Key320(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosMacL3Key320(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosMacL3Key320(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosMacL3Key320(V, ttl_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ttl_f, p_buffer->mask, mask);
        break;
    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosMacL3Key320(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosMacL3Key320(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosMacL3Key320(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosMacL3Key320(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosMacL3Key320(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*SAT PDU*/
   case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosMacL3Key320(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosMacL3Key320(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosMacL3Key320(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosMacL3Key320(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosMacL3Key320(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosMacL3Key320(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosMacL3Key320(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosMacL3Key320(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosMacL3Key320(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosMacL3Key320(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosMacL3Key320(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosMacL3Key320(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosMacL3Key320(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosMacL3Key320(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosMacL3Key320(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosMacL3Key320(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*EtherType*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosMacL3Key320(V, etherType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, etherType_f, p_buffer->mask, mask);
        break;

     /*Aware Tunnel Info Share*/
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        SetDsAclQosMacL3Key320(V, cvlanIdValid_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key320(V, cvlanIdValid_f, p_buffer->mask, 1);
        SetDsAclQosMacL3Key320(V, ctagCos_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key320(V, ctagCos_f, p_buffer->mask, 0x3);
        SetDsAclQosMacL3Key320(V, ctagCfi_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key320(V, ctagCfi_f, p_buffer->mask, 1);
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, 0xF);

        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
            p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->key, 1);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->mask, 1);
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            if(p_tunnel_data ->type == 3)
            {
                cvlanId_compound_data = (drv_merge_type & 0x03) << 6 | (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 5 | (p_tunnel_data ->radio_id & 0x1f);
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, cvlanId_compound_data);
                cvlanId_compound_mask = (drv_merge_type_mask & 0x03) << 6 | (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 5 | (p_tunnel_mask ->radio_id & 0x1f);
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, cvlanId_compound_mask);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, hw_mac);
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, hw_mac);
            }
            else if(p_tunnel_data ->type == 2)
            {
                cvlanId_compound_data = (drv_merge_type & 0x03) << 6 ;
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, cvlanId_compound_data);
                cvlanId_compound_mask = (drv_merge_type_mask & 0x03) << 6 ;
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, cvlanId_compound_mask);

                mac_da_compound_data[0] = p_tunnel_data ->gre_key;
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, mac_da_compound_data);
                mac_da_compound_mask[0] = p_tunnel_mask ->gre_key;
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, mac_da_compound_mask);
            }
            else if(p_tunnel_data ->type == 1)
            {
                cvlanId_compound_data = (drv_merge_type & 0x03) << 6 ;
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key, cvlanId_compound_data);
                cvlanId_compound_mask = (drv_merge_type_mask & 0x03) << 6 ;
                SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask, cvlanId_compound_mask);

                mac_da_compound_data[0] = p_tunnel_data ->vxlan_vni;
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, mac_da_compound_data);
                mac_da_compound_mask[0] = p_tunnel_mask ->vxlan_vni;
                SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, mac_da_compound_mask);
            }
        }
        else
        {
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->key, 0);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->mask, 0);
            SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->key,0);
            SetDsAclQosMacL3Key320(V, cvlanId_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->key, mac_da_compound_data);
            SetDsAclQosMacL3Key320(A, macDa_f, p_buffer->mask, mac_da_compound_mask);
        }
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        if(CTC_EGRESS != pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            sys_cpu_reason_info_t  reason_info;
            sal_memset(&reason_info,0,sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));
            if(reason_info.fatal_excp_valid || !reason_info.gid_valid)
            {
                return CTC_E_BADID;
            }

            SetDsAclQosMacL3Key320(V, isDecap_f, p_buffer->key, 1);
            SetDsAclQosMacL3Key320(V, isDecap_f, p_buffer->mask, 1);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->key, (reason_info.gid_for_acl_key >>(DRV_IS_DUET2(lchip)? 3: 1)) & 0x1);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->mask, (mask >>(DRV_IS_DUET2(lchip)? 3: 1)) & 0x1);
            SetDsAclQosMacL3Key320(V, reserved_f, p_buffer->key, (reason_info.gid_for_acl_key >> 2) & 0x1);
            SetDsAclQosMacL3Key320(V, reserved_f, p_buffer->mask, (mask >>2) & 0x1);
            SetDsAclQosMacL3Key320(V, isElephant_f, p_buffer->key, (reason_info.gid_for_acl_key >> (DRV_IS_DUET2(lchip)? 1: 0)) & 0x1);
            SetDsAclQosMacL3Key320(V, isElephant_f, p_buffer->mask, (mask >>(DRV_IS_DUET2(lchip)? 1: 0)) & 0x1);
            SetDsAclQosMacL3Key320(V, isMcastRpfCheckFail_f, p_buffer->key, reason_info.gid_for_acl_key & 0x1);
            SetDsAclQosMacL3Key320(V, isMcastRpfCheckFail_f, p_buffer->mask, mask & 0x1);
        }
        else
        {
            SetDsAclQosMacL3Key320(V, isDecap_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key320(V, isMergeKey_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key320(V, reserved_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key320(V, isElephant_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key320(V, isMcastRpfCheckFail_f, p_buffer->mask,0);
        }
        break;
     case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key320(V, layer4UserType_f, p_buffer->mask, mask);
       break;
    default:
       SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
       return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}


int32
_sys_usw_acl_add_macl3key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint32 tmp_gport_data = 0;
    uint32 tmp_gport_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint8  l3_type = 0;
    uint32 hw_satpdu_byte[2] = {0};
    hw_mac_addr_t hw_mac = {0};
    sys_acl_buffer_t*  p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    uint32  temp_value_data[2] = {0};
    uint32  temp_value_mask[2] = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }
    l3_type = GetDsAclQosMacL3Key640(V, layer3Type_f, p_buffer->key);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosMacL3Key640(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosMacL3Key640(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosMacL3Key640(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacL3Key640_t, DsAclQosMacL3Key640_globalPort_f))
        {
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->key, (drv_acl_port_info.gport >> 8) & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->mask, (drv_acl_port_info.gport_mask >> 8) & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->key, drv_acl_port_info.gport & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->mask, drv_acl_port_info.gport_mask & 0xFF);
        }
        else
        {
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        }
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosMacL3Key640(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosMacL3Key640(V, ctagCfi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ctagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosMacL3Key640(V, ctagCos_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ctagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosMacL3Key640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, cvlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosMacL3Key640(V, cvlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, cvlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        if(CTC_EGRESS != pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            sys_cpu_reason_info_t  reason_info;
            sal_memset(&reason_info,0,sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));
            if(reason_info.fatal_excp_valid || !reason_info.gid_valid)
            {
                return CTC_E_BADID;
            }
            SetDsAclQosMacL3Key640(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | (1<<4)) );
            SetDsAclQosMacL3Key640(V, exceptionInfo_f, p_buffer->mask, (mask | (1<<4)));
        }
        else
        {
            SetDsAclQosMacL3Key640(V, exceptionInfo_f, p_buffer->key, 0);
            SetDsAclQosMacL3Key640(V, exceptionInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacL3Key640_t, DsAclQosMacL3Key640_globalPort_f))
        {
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->key, ((data | (1 << 14)) >> 8) & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->mask, (mask >> 8) & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->key, data & 0xFF);
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->mask, mask & 0xFF);
        }
        else
        {
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key, (data | (1 << 14)));
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask, mask);
        }
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_bmp_data = GetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacL3Key640_t, DsAclQosMacL3Key640_globalPort_f))
        {
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, globalPort15To8_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= ((data << 8)&0xFF00);
            tmp_gport_mask |= ((mask << 8)&0xFF00);
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0x00ff));
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0x00ff));
        }
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_bmp_data = GetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacL3Key640_t, DsAclQosMacL3Key640_globalPort_f))
        {
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, globalPort7To0_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= (data&0xFF);
            tmp_gport_mask |= (mask&0xFF);
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0xff00));
            SetDsAclQosMacL3Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0xff00));
        }
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacL3Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacL3Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosMacL3Key640(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosMacL3Key640(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosMacL3Key640(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosMacL3Key640(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosMacL3Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosMacL3Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        {
            uint8 is_ip_packet = (CTC_PARSER_L3_TYPE_IPV4 == data);
            SetDsAclQosMacL3Key640(V, isIpPacket_f, p_buffer->key, is_ip_packet);
            SetDsAclQosMacL3Key640(V, isIpPacket_f, p_buffer->mask, is_ip_packet);
        }
        SetDsAclQosMacL3Key640(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosMacL3Key640(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosMacL3Key640(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_DA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key640(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key640(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key640(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key640(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    /*NSH*/
    case CTC_FIELD_KEY_NSH_CBIT:
        SetDsAclQosMacL3Key640(V, nshCbit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, nshCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
        SetDsAclQosMacL3Key640(V, nshNextProtocol_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, nshNextProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_OBIT:
        SetDsAclQosMacL3Key640(V, nshObit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, nshObit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SI:
        SetDsAclQosMacL3Key640(V, nshSi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, nshSi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SPI:
        SetDsAclQosMacL3Key640(V, nshSpi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, nshSpi_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosMacL3Key640(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosMacL3Key640(V, stagCfi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, stagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosMacL3Key640(V, stagCos_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, stagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosMacL3Key640(V, svlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, svlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosMacL3Key640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, svlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosMacL3Key640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, svlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosMacL3Key640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, cvlanId_f, p_buffer->mask, 0xFFF);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosMacL3Key640(V, dscp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosMacL3Key640(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosMacL3Key640(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosMacL3Key640(V, ecn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosMacL3Key640(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_DA:
        SetDsAclQosMacL3Key640(V, ipDa_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ipDa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosMacL3Key640(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosMacL3Key640(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetDsAclQosMacL3Key640(V, ipSa_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ipSa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        tmp_data = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacL3Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1 :
        CTC_MAX_VALUE_CHECK(data, 0xFFFFFF);
        SetDsAclQosMacL3Key640(V, vxlanReserved1_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, vxlanReserved1_f, p_buffer->mask, mask & 0xFFFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2 :
        CTC_MAX_VALUE_CHECK(data, 0xFF);
        SetDsAclQosMacL3Key640(V, vxlanReserved2_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, vxlanReserved2_f, p_buffer->mask, mask & 0xFF);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacL3Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosMacL3Key640(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosMacL3Key640(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosMacL3Key640(V, ttl_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ttl_f, p_buffer->mask, mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosMacL3Key640(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosMacL3Key640(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosMacL3Key640(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosMacL3Key640(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
        /*add check only one in two,default not share */
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key640(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key640(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_TARGET_MAC:
         /*add check only one in two,default not share */
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key640(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key640(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosMacL3Key640(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosMacL3Key640(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosMacL3Key640(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosMacL3Key640(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosMacL3Key640(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacL3Key640(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key640(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key640(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, u1_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacL3Key640(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosMacL3Key640(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosMacL3Key640(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    /*MPLS*/
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosMacL3Key640(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosMacL3Key640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosMacL3Key640(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosMacL3Key640(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosMacL3Key640(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosMacL3Key640(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosMacL3Key640(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosMacL3Key640(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosMacL3Key640(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosMacL3Key640(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:

        SetDsAclQosMacL3Key640(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosMacL3Key640(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosMacL3Key640(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosMacL3Key640(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosMacL3Key640(V, u1_g5_ttl_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, u1_g5_ttl_f, p_buffer->mask, mask);
        break;
    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosMacL3Key640(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosMacL3Key640(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosMacL3Key640(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosMacL3Key640(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosMacL3Key640(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*SAT PDU*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosMacL3Key640(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosMacL3Key640(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosMacL3Key640(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosMacL3Key640(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosMacL3Key640(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosMacL3Key640(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosMacL3Key640(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosMacL3Key640(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosMacL3Key640(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosMacL3Key640(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosMacL3Key640(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosMacL3Key640(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosMacL3Key640(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosMacL3Key640(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosMacL3Key640(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosMacL3Key640(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*Ether Type*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosMacL3Key640(V, etherType_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, etherType_f, p_buffer->mask, mask);
        break;
    /*Wlan*/
    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacL3Key640(A, radioMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacL3Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_SET(pe->wlan_bmp, 0);
        }
        else
        {
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            SetDsAclQosMacL3Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_UNSET(pe->wlan_bmp, 0);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_RADIO_ID:
        SetDsAclQosMacL3Key640(V, rid_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, rid_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            CTC_BIT_SET(pe->wlan_bmp,1);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp,1);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_CTL_PKT:
        SetDsAclQosMacL3Key640(V, capwapControlPacket_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, capwapControlPacket_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            CTC_BIT_SET(pe->wlan_bmp,2);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp,2);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        SetDsAclQosMacL3Key640(V, vrfId_f, p_buffer->key, data);
        SetDsAclQosMacL3Key640(V, vrfId_f, p_buffer->mask, mask);
        break;
    /*Aware Tunnel Info*/
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->key, 0);
        SetDsAclQosMacL3Key640(V, isCapwapInfo_f, p_buffer->mask, is_add? 0x01 : 0);
        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
            p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);
            SetDsAclQosMacL3Key640(V, isMergeKey_f, p_buffer->key, 1);
            SetDsAclQosMacL3Key640(V, isMergeKey_f, p_buffer->mask, 1);
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            SetDsAclQosMacL3Key640(V, mergeDataType_f, p_buffer->key,drv_merge_type);
            SetDsAclQosMacL3Key640(V, mergeDataType_f, p_buffer->mask,drv_merge_type_mask);
            if(p_tunnel_data ->type == 3)
            {
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                sal_memcpy((uint8*)temp_value_data, &hw_mac, sizeof(mac_addr_t));
                temp_value_data[1] = (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_data ->radio_id & 0x1f) << 16 | (temp_value_data[1] & 0xFFFF);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                sal_memcpy((uint8*)temp_value_mask, &hw_mac, sizeof(mac_addr_t));
                temp_value_mask[1] = (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_mask ->radio_id & 0x1f) << 16 | (temp_value_mask[1] & 0xFFFF);

                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 2)
            {
                temp_value_data[0] = p_tunnel_data ->gre_key;
                temp_value_mask[0] = p_tunnel_mask ->gre_key;
                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 1)
            {
                temp_value_data[0] = p_tunnel_data ->vxlan_vni;
                temp_value_mask[0] = p_tunnel_mask ->vxlan_vni;
                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
        }
        else
        {
            SetDsAclQosMacL3Key640(V, isMergeKey_f, p_buffer->key, 0);
            SetDsAclQosMacL3Key640(V, isMergeKey_f, p_buffer->mask, 0);
            SetDsAclQosMacL3Key640(V, mergeDataType_f, p_buffer->key,0);
            SetDsAclQosMacL3Key640(V, mergeDataType_f, p_buffer->mask,0);
            SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->key, temp_value_data);
            SetDsAclQosMacL3Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
        }
        break;
    case CTC_FIELD_KEY_UDF:
        {
             ctc_acl_udf_t* p_udf_info_data = (ctc_acl_udf_t*)key_field->ext_data;
             ctc_acl_udf_t* p_udf_info_mask = (ctc_acl_udf_t*)key_field->ext_mask;
             sys_acl_udf_entry_t *p_udf_entry = NULL;
            // uint32 hw_udf[4] = {0};

             CTC_PTR_VALID_CHECK(p_udf_info_data);
             CTC_PTR_VALID_CHECK(p_udf_info_mask);

             if (is_add)
             {
                 _sys_usw_acl_get_udf_info(lchip, p_udf_info_data->udf_id, &p_udf_entry);
                 if (!p_udf_entry || !p_udf_entry->key_index_used )
                 {
                     return CTC_E_NOT_EXIST;
                 }
                 /* SYSTEM MODIFIED by hansf 2019/01/28 */
                 #if 0
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_data->udf);
                 SetDsAclQosMacL3Key640(A, udf_f, p_buffer->key,  hw_udf);
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_mask->udf);
                 SetDsAclQosMacL3Key640(A, udf_f, p_buffer->mask, hw_udf);
                 #endif
                 SetDsAclQosMacL3Key640(A, udf_f, p_buffer->key,  p_udf_info_data->udf);
                 SetDsAclQosMacL3Key640(A, udf_f, p_buffer->mask, p_udf_info_mask->udf);

                 SetDsAclQosMacL3Key640(V, udfHitIndex_f, p_buffer->key,  p_udf_entry->udf_hit_index);
                 SetDsAclQosMacL3Key640(V, udfHitIndex_f, p_buffer->mask,  0xF);
             }
             else
             {
                 sal_memset(p_udf_info_mask->udf, 0, CTC_ACL_UDF_BYTE_NUM);
                 SetDsAclQosMacL3Key640(A, udf_f, p_buffer->mask, p_udf_info_mask->udf);
                 SetDsAclQosMacL3Key640(V, udfHitIndex_f, p_buffer->mask, 0);
             }
            SetDsAclQosMacL3Key640(V, udfValid_f, p_buffer->key, is_add ? 1:0);
            SetDsAclQosMacL3Key640(V, udfValid_f, p_buffer->mask, is_add ? 1:0);
            pe->udf_id = is_add ? p_udf_info_data->udf_id : 0;
        }
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_ipv6key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32      data        = 0;
    uint32      mask        = 0;
    uint32      tmp_data    = 0;
    uint32      tmp_mask    = 0;
    uint8       tmp_bmp_data = 0;
    uint8       tmp_bmp_mask = 0;
    ipv6_addr_t hw_ip6      = {0};
    ipv6_addr_t hw_ip6_2      = {0};
    sys_acl_buffer_t*      p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosIpv6Key320(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosIpv6Key320(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosIpv6Key320(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        if(is_add && GetDsAclQosIpv6Key320(V, dscp_f, p_buffer->mask) != 0)
        {
            return CTC_E_INVALID_PARAM;
        }
        SetDsAclQosIpv6Key320(V, dscp_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        if(is_add && GetDsAclQosIpv6Key320(V, dscp_f, p_buffer->mask) != 0)
        {
            return CTC_E_INVALID_PARAM;
        }
        SetDsAclQosIpv6Key320(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosIpv6Key320(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosIpv6Key320(V, ecn_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosIpv6Key320(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosIpv6Key320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosIpv6Key320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
     /*Ipv6Key320Basic Pkt UseIpDaField, choose Da or Sa, default use SA High 64bit*/
    case CTC_FIELD_KEY_IPV6_DA:
        SetDsAclQosIpv6Key320(V, ipAddrMode_f, p_buffer->key, 0);
        SetDsAclQosIpv6Key320(V, ipAddrMode_f, p_buffer->mask, is_add ? 3 : 0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 1, (uint32*)key_field->ext_data, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosIpv6Key320(A, ipAddr1_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 1, (uint32*)key_field->ext_mask, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosIpv6Key320(A, ipAddr1_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        SetDsAclQosIpv6Key320(V, ipAddrMode_f, p_buffer->key, 0);
        SetDsAclQosIpv6Key320(V, ipAddrMode_f, p_buffer->mask, is_add ? 3 : 0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 0, (uint32*)key_field->ext_data, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosIpv6Key320(A, ipAddr2_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 0, (uint32*)key_field->ext_mask, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosIpv6Key320(A, ipAddr2_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosIpv6Key320(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosIpv6Key320(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        SetDsAclQosIpv6Key320(V, ipv6FlowLabel_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, ipv6FlowLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosIpv6Key320(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosIpv6Key320(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
     case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        tmp_data = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosIpv6Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosIpv6Key320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV6, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosIpv6Key320(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosIpv6Key320(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key320(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosIpv6Key320(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosIpv6Key320(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosIpv6Key320(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosIpv6Key320(V, ttl_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, ttl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VRFID:  /*Ipv6Key320 only support 13bti*/
        SetDsAclQosIpv6Key320(V, vrfId_f, p_buffer->key, data);
        SetDsAclQosIpv6Key320(V, vrfId_f, p_buffer->mask, mask);
        break;
	case CTC_FIELD_KEY_L3_TYPE:
		 /*asic is not the field ,only used to sw db,do nothing*/
		break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_ipv6key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32  tmp_data = 0;
    uint32  tmp_mask = 0;
    uint32 tmp_gport_data = 0;
    uint32 tmp_gport_mask = 0;
    uint8   tmp_bmp_data = 0;
    uint8   tmp_bmp_mask = 0;
    ipv6_addr_t hw_ip6 = {0};
    hw_mac_addr_t hw_mac = {0};
    sys_acl_buffer_t* p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    uint32  temp_value_data[2] = {0};
    uint32  temp_value_mask[2] = {0};
    uint32  temp_radio_mac[2] = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }
    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosIpv6Key640_t, DsAclQosIpv6Key640_globalPort_f))
        {
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->key, (drv_acl_port_info.gport >> 8) & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->mask, (drv_acl_port_info.gport_mask >> 8) & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->key, drv_acl_port_info.gport & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->mask, drv_acl_port_info.gport_mask & 0xFF);
        }
        else
        {
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        }
        SetDsAclQosIpv6Key640(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosIpv6Key640(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosIpv6Key640(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosIpv6Key640(V, dscp_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosIpv6Key640(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosIpv6Key640(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosIpv6Key640(V, ecn_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosIpv6Key640(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    /*NSH*/
    case CTC_FIELD_KEY_NSH_CBIT:
        SetDsAclQosIpv6Key640(V, gNsh_nshCbit_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, gNsh_nshCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
        SetDsAclQosIpv6Key640(V, gNsh_nshNextProtocol_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, gNsh_nshNextProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_OBIT:
        SetDsAclQosIpv6Key640(V, gNsh_nshObit_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, gNsh_nshObit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SI:
        SetDsAclQosIpv6Key640(V, gNsh_nshSi_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, gNsh_nshSi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_NSH_SPI:
        SetDsAclQosIpv6Key640(V, gNsh_nshSpi_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, gNsh_nshSpi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        if (!DRV_FLD_IS_EXISIT(DsAclQosIpv6Key640_t, DsAclQosIpv6Key640_globalPort_f))
        {
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->key, ((data | (1 << 14)) >> 8) & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->mask, (mask >> 8) & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->key, data & 0xFF);
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->mask, mask & 0xFF);
        }
        else
        {
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key, (data | (1 << 14)));
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask, mask);
        }
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_bmp_data = GetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosIpv6Key640_t, DsAclQosIpv6Key640_globalPort_f))
        {
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->key, data);
            SetDsAclQosIpv6Key640(V, globalPort15To8_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= ((data << 8)&0xFF00);
            tmp_gport_mask |= ((mask << 8)&0xFF00);
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0x00ff));
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0x00ff));
        }
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_bmp_data = GetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosIpv6Key640_t, DsAclQosIpv6Key640_globalPort_f))
        {
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->key, data);
            SetDsAclQosIpv6Key640(V, globalPort7To0_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= (data&0xFF);
            tmp_gport_mask |= (mask&0xFF);
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0xff00));
            SetDsAclQosIpv6Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0xff00));
        }
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosIpv6Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosIpv6Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_IPV6_DA:
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
        SetDsAclQosIpv6Key640(A, ipDa_f, p_buffer->key, hw_ip6);
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
        SetDsAclQosIpv6Key640(A, ipDa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosIpv6Key640(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosIpv6Key640(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
            SetDsAclQosIpv6Key640(A, ipSa_f, p_buffer->key, hw_ip6);
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
            SetDsAclQosIpv6Key640(A, ipSa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        SetDsAclQosIpv6Key640(V, ipv6FlowLabel_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, ipv6FlowLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosIpv6Key640(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosIpv6Key640(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosIpv6Key640(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1:
        GetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        temp_radio_mac[0] &= 0xFF000000;
        temp_radio_mac[0] |= data & 0xFFFFFF;
        SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        GetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        temp_radio_mac[0] &= 0xFF000000;
        temp_radio_mac[0] |= mask & 0xFFFFFF;
        SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2:
        GetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        temp_radio_mac[0] &= 0xFFFFFF;
        temp_radio_mac[0] |= (data & 0xFF) << 24;
        SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        GetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        temp_radio_mac[0] &= 0xFFFFFF;
        temp_radio_mac[0] |= (mask & 0xFF) << 24;
        SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        tmp_data = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosIpv6Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosIpv6Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
         CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV6, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosIpv6Key640(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosIpv6Key640(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosIpv6Key640(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosIpv6Key640(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosIpv6Key640(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosIpv6Key640(V, ttl_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, ttl_f, p_buffer->mask, mask);
        break;
    /*Wlan*/
    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0x01);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_SET(pe->wlan_bmp,0);
        }
        else
        {
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            SetDsAclQosIpv6Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_UNSET(pe->wlan_bmp,0);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_RADIO_ID:
        SetDsAclQosIpv6Key640(V, rid_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, rid_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            CTC_BIT_SET(pe->wlan_bmp,1);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp,1);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_CTL_PKT:
        SetDsAclQosIpv6Key640(V, capwapControlPacket_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, capwapControlPacket_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->key, 0x01);
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask, 0x01);
            CTC_BIT_SET(pe->wlan_bmp,2);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp,2);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        SetDsAclQosIpv6Key640(V, vrfId_f, p_buffer->key, data);
        SetDsAclQosIpv6Key640(V, vrfId_f, p_buffer->mask, mask);
        break;

    /*Aware Tunnel Info*/
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->key, 0);
        SetDsAclQosIpv6Key640(V, isCapwapInfo_f, p_buffer->mask, is_add? 0x01 : 0);
        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
            p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            SetDsAclQosIpv6Key640(V, mergeDataType_f, p_buffer->key,drv_merge_type);
            SetDsAclQosIpv6Key640(V, mergeDataType_f, p_buffer->mask,drv_merge_type_mask);
            if(p_tunnel_data ->type == 3)
            {
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                sal_memcpy((uint8*)temp_value_data, &hw_mac, sizeof(mac_addr_t));
                temp_value_data[1] = (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_data ->radio_id & 0x1f) << 16 | (temp_value_data[1] & 0xFFFF);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                sal_memcpy((uint8*)temp_value_mask, &hw_mac, sizeof(mac_addr_t));
                temp_value_mask[1] = (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_mask ->radio_id & 0x1f) << 16 | (temp_value_mask[1] & 0xFFFF);

                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 2)
            {
                temp_value_data[0] = p_tunnel_data ->gre_key;
                temp_value_mask[0] = p_tunnel_mask ->gre_key;
                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 1)
            {
                temp_value_data[0] = p_tunnel_data ->vxlan_vni;
                temp_value_mask[0] = p_tunnel_mask ->vxlan_vni;
                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
        }
        else
        {
            SetDsAclQosIpv6Key640(V, mergeDataType_f, p_buffer->key,0);
            SetDsAclQosIpv6Key640(V, mergeDataType_f, p_buffer->mask,0);
            SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
            SetDsAclQosIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
        }
        break;
	case CTC_FIELD_KEY_L3_TYPE:
		 /*asic is not the field ,only used to sw db,do nothing*/
		break;
       /*UDF*/
      case CTC_FIELD_KEY_UDF:
        {
             ctc_acl_udf_t* p_udf_info_data = (ctc_acl_udf_t*)key_field->ext_data;
             ctc_acl_udf_t* p_udf_info_mask = (ctc_acl_udf_t*)key_field->ext_mask;
             sys_acl_udf_entry_t *p_udf_entry;
             uint32  hw_udf[4] = {0};
             if (is_add)
             {
                 _sys_usw_acl_get_udf_info(lchip, p_udf_info_data->udf_id, &p_udf_entry);
                 if (!p_udf_entry || !p_udf_entry->key_index_used )
                 {
                     return CTC_E_NOT_EXIST;
                 }
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_data->udf);
                 SetDsAclQosIpv6Key640(A, udf_f, p_buffer->key,  hw_udf);
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_mask->udf);
                 SetDsAclQosIpv6Key640(A, udf_f, p_buffer->mask, hw_udf);
                 SetDsAclQosIpv6Key640(V, udfHitIndex_f, p_buffer->key,  p_udf_entry->udf_hit_index);
                 SetDsAclQosIpv6Key640(V, udfHitIndex_f, p_buffer->mask,  0xF);
             }
             else
             {
                 sal_memset(p_udf_info_mask->udf, 0, CTC_ACL_UDF_BYTE_NUM);
                 SetDsAclQosIpv6Key640(A, udf_f, p_buffer->mask, p_udf_info_mask->udf);
                 SetDsAclQosIpv6Key640(V, udfHitIndex_f, p_buffer->mask, 0);
             }
            SetDsAclQosIpv6Key640(V, udfValid_f, p_buffer->key, is_add ? 1:0);
            SetDsAclQosIpv6Key640(V, udfValid_f, p_buffer->mask, is_add ? 1:0);
            pe->udf_id = is_add ? p_udf_info_data->udf_id : 0;
        }
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
      return CTC_E_NONE;
}

int32
_sys_usw_acl_add_macipv6key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32  tmp_data = 0;
    uint32  tmp_mask = 0;
    uint32 tmp_gport_data = 0;
    uint32 tmp_gport_mask = 0;
    uint8   tmp_bmp_data = 0;
    uint8   tmp_bmp_mask = 0;
    ipv6_addr_t     hw_ip6 = {0};
    hw_mac_addr_t   hw_mac = {0};
    sys_acl_buffer_t* p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    uint32  temp_value_data[2] = {0};
    uint32  temp_value_mask[2] = {0};
    uint32  temp_radio_mac[2] = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rg_info = &(pe->rg_info);
    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosMacIpv6Key640(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosMacIpv6Key640(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_globalPort_f))
        {
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->key, (drv_acl_port_info.gport >> 8) & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->mask, (drv_acl_port_info.gport_mask >> 8) & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->key, drv_acl_port_info.gport & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->mask, drv_acl_port_info.gport_mask & 0xFF);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        }
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosMacIpv6Key640(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosMacIpv6Key640(V, ctagCfi_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ctagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosMacIpv6Key640(V, ctagCos_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ctagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosMacIpv6Key640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, cvlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosMacIpv6Key640(V, cvlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacIpv6Key640(V, cvlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosMacIpv6Key640(V, dscp_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosMacIpv6Key640(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosMacIpv6Key640(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosMacIpv6Key640(V, ecn_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        if(CTC_EGRESS != pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            sys_cpu_reason_info_t  reason_info;
            sal_memset(&reason_info,0,sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));
            if(reason_info.fatal_excp_valid || !reason_info.gid_valid)
            {
                return CTC_E_BADID;
            }
            SetDsAclQosMacIpv6Key640(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | (1<<4)));
            SetDsAclQosMacIpv6Key640(V, exceptionInfo_f, p_buffer->mask, (mask | (1<<4)));
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, exceptionInfo_f, p_buffer->key, 0);
            SetDsAclQosMacIpv6Key640(V, exceptionInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosMacIpv6Key640(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_globalPort_f))
        {
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->key, ((data | (1 << 14)) >> 8) & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->mask, (mask >> 8) & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->key, data & 0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->mask, mask & 0xFF);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key, (data | (1 << 14)));
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask, mask);
        }
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_bmp_data = GetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_globalPort_f))
        {
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->key, data);
            SetDsAclQosMacIpv6Key640(V, globalPort15To8_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= ((data << 8)&0xFF00);
            tmp_gport_mask |= ((mask << 8)&0xFF00);
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0x00ff));
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:tmp_gport_mask&0x00ff);
        }
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_bmp_data = GetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_globalPort_f))
        {
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->key, data);
            SetDsAclQosMacIpv6Key640(V, globalPort7To0_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= (data&0xFF);
            tmp_gport_mask |= (mask&0xFF);
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0xff00));
            SetDsAclQosMacIpv6Key640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0xff00));
        }
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosMacIpv6Key640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosMacIpv6Key640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_IPV6_DA:
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
        if (DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipDa_f))
        {
            SetDsAclQosMacIpv6Key640(A, ipDa_f, p_buffer->key, hw_ip6);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, ipDa31To0_f, p_buffer->key, hw_ip6[0]);
            SetDsAclQosMacIpv6Key640(A, ipDa127To32_f, p_buffer->key, &hw_ip6[1]);
        }
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
        if (DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipDa_f))
        {
            SetDsAclQosMacIpv6Key640(A, ipDa_f, p_buffer->mask, hw_ip6);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, ipDa31To0_f, p_buffer->mask, hw_ip6[0]);
            SetDsAclQosMacIpv6Key640(A, ipDa127To32_f, p_buffer->mask, &hw_ip6[1]);
        }
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosMacIpv6Key640(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosMacIpv6Key640(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
        if (DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipSa_f))
        {
            SetDsAclQosMacIpv6Key640(A, ipSa_f, p_buffer->key, hw_ip6);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, ipSa31To0_f, p_buffer->key, hw_ip6[0]);
            SetDsAclQosMacIpv6Key640(A, ipSa127To32_f, p_buffer->key, &hw_ip6[1]);
        }
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
        if (DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipSa_f))
        {
            SetDsAclQosMacIpv6Key640(A, ipSa_f, p_buffer->mask, hw_ip6);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, ipSa31To0_f, p_buffer->mask, hw_ip6[0]);
            SetDsAclQosMacIpv6Key640(A, ipSa127To32_f, p_buffer->mask, &hw_ip6[1]);
        }
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        if (DRV_FLD_IS_EXISIT(DsAclQosMacIpv6Key640_t, DsAclQosMacIpv6Key640_ipv6FlowLabel_f))
        {
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabel_f, p_buffer->key, data);
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabelHigh_f, p_buffer->key, ((data >> 4)&0xFFFF));
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabelHigh_f, p_buffer->mask, ((mask >> 4)&0xFFFF));
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabelLow_f, p_buffer->key, (data&0xF));
            SetDsAclQosMacIpv6Key640(V, ipv6FlowLabelLow_f, p_buffer->mask, (mask&0xF));
        }
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosMacIpv6Key640(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosMacIpv6Key640(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosMacIpv6Key640(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosMacIpv6Key640(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1:
        GetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        temp_radio_mac[0] &= 0xFF000000;
        temp_radio_mac[0] |= data & 0xFFFFFF;
        SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        GetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        temp_radio_mac[0] &= 0xFF000000;
        temp_radio_mac[0] |= mask & 0xFFFFFF;
        SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2:
        GetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        temp_radio_mac[0] &= 0xFFFFFF;
        temp_radio_mac[0] |= (data & 0xFF) << 24;
        SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->key, temp_radio_mac);
        GetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        temp_radio_mac[0] &= 0xFFFFFF;
        temp_radio_mac[0] |= (mask & 0xFF) << 24;
        SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, temp_radio_mac);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        tmp_data = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->key, is_add ? MCHIP_CAP(SYS_CAP_L4_USER_UDP_TYPE_VXLAN) : 0);
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->mask, is_add ? 0xF : 0);
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosMacIpv6Key640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosMacIpv6Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosMacIpv6Key640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
       CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV6, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosMacIpv6Key640(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosMacIpv6Key640(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_DA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacIpv6Key640(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacIpv6Key640(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacIpv6Key640(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacIpv6Key640(A, macSa_f, p_buffer->mask, hw_mac);
        }
        else
        {
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            SetDsAclQosMacIpv6Key640(A, macSa_f, p_buffer->key, hw_mac);
            SetDsAclQosMacIpv6Key640(A, macSa_f, p_buffer->mask, hw_mac);
        }
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosMacIpv6Key640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosMacIpv6Key640(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosMacIpv6Key640(V, stagCfi_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, stagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosMacIpv6Key640(V, stagCos_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, stagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosMacIpv6Key640(V, svlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosMacIpv6Key640(V, svlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosMacIpv6Key640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, svlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosMacIpv6Key640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, svlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosMacIpv6Key640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, cvlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosMacIpv6Key640(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosMacIpv6Key640(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosMacIpv6Key640(V, ttl_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, ttl_f, p_buffer->mask, mask);
        break;
    /*Wlan*/
    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
    case CTC_FIELD_KEY_WLAN_RADIO_MAC:
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(key_field->ext_data);
            CTC_PTR_VALID_CHECK(key_field->ext_mask);
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->key,0x01);
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0x01);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_SET(pe->wlan_bmp, 0);
        }
        else
        {
            sal_memset(hw_mac, 0, sizeof(hw_mac_addr_t));
            SetDsAclQosMacIpv6Key640(A, radioMac_f, p_buffer->mask, hw_mac);
            CTC_BIT_UNSET(pe->wlan_bmp, 0);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_RADIO_ID:
        SetDsAclQosMacIpv6Key640(V, rid_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, rid_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->key,0x01);
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0x01);
            CTC_BIT_SET(pe->wlan_bmp, 1);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp, 1);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_WLAN_CTL_PKT:
        SetDsAclQosMacIpv6Key640(V, capwapControlPacket_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, capwapControlPacket_f, p_buffer->mask, mask);
        if(is_add)
        {
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->key,0x01);
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0x01);
            CTC_BIT_SET(pe->wlan_bmp, 2);
        }
        else
        {
            CTC_BIT_UNSET(pe->wlan_bmp, 2);
        }
        if(pe->wlan_bmp == 0)
        {
            SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_VRFID:
        SetDsAclQosMacIpv6Key640(V, vrfId_f, p_buffer->key, data);
        SetDsAclQosMacIpv6Key640(V, vrfId_f, p_buffer->mask, mask);
        break;
    /*Aware Tunnel Info*/
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->key, 0);
        SetDsAclQosMacIpv6Key640(V, isCapwapInfo_f, p_buffer->mask,is_add ? 0x01 : 0);
        SetDsAclQosMacIpv6Key640(V, isMergeKey_f, p_buffer->key, (is_add ? 0x01 : 0));
        SetDsAclQosMacIpv6Key640(V, isMergeKey_f, p_buffer->mask, (is_add ? 0x01 : 0));
        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
            p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            SetDsAclQosMacIpv6Key640(V, mergeDataType_f, p_buffer->key,drv_merge_type);
            SetDsAclQosMacIpv6Key640(V, mergeDataType_f, p_buffer->mask,drv_merge_type_mask);
            if(p_tunnel_data ->type == 3)
            {
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                sal_memcpy((uint8*)temp_value_data, &hw_mac, sizeof(mac_addr_t));
                temp_value_data[1] = (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_data ->radio_id & 0x1f) << 16 | (temp_value_data[1] & 0xFFFF);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                sal_memcpy((uint8*)temp_value_mask, &hw_mac, sizeof(mac_addr_t));
                temp_value_mask[1] = (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_mask ->radio_id & 0x1f) << 16 | (temp_value_mask[1] & 0xFFFF);

                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 2)
            {
                temp_value_data[0] = p_tunnel_data ->gre_key;
                temp_value_mask[0] = p_tunnel_mask ->gre_key;
                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
            else if(p_tunnel_data ->type == 1)
            {
                temp_value_data[0] = p_tunnel_data ->vxlan_vni;
                temp_value_mask[0] = p_tunnel_mask ->vxlan_vni;
                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
                SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
            }
        }
        else
        {
            SetDsAclQosMacIpv6Key640(V, mergeDataType_f, p_buffer->key,0);
            SetDsAclQosMacIpv6Key640(V, mergeDataType_f, p_buffer->mask,0);
            SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->key, temp_value_data);
            SetDsAclQosMacIpv6Key640(A, mergeData_f, p_buffer->mask, temp_value_mask);
        }
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        /*asic is not the field ,only used to sw db,do nothing*/
        {
            uint8 is_ip_packet = (CTC_PARSER_L3_TYPE_IP == data) || (CTC_PARSER_L3_TYPE_IPV4 == data) || (CTC_PARSER_L3_TYPE_IPV6 == data);
            SetDsAclQosMacIpv6Key640(V, isIpPacket_f, p_buffer->key, is_ip_packet);
            SetDsAclQosMacIpv6Key640(V, isIpPacket_f, p_buffer->mask, is_ip_packet);
        }
        tmp_mask = CTC_PARSER_L3_TYPE_NONE == key_field->data && !DRV_IS_DUET2(lchip) && is_add? 0x6: 0xF;
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType0_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType1_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType2_f, p_buffer->mask, tmp_mask);
        SetDsAclQosMacIpv6Key640(V, aclQosKeyType3_f, p_buffer->mask, tmp_mask);
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}


int32
_sys_usw_acl_add_cidkey160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data     = 0;
    uint32 mask     = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    ipv6_addr_t hw_ip6 = {0};
    ipv6_addr_t hw_ip6_2 = {0};
    sys_acl_buffer_t* p_buffer = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosCidKey160(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosCidKey160(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosCidKey160(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        if(is_add)
        {
            SetDsAclQosCidKey160(V, dstCategoryId_f, p_buffer->key, data);
            SetDsAclQosCidKey160(V, dstCategoryId_f, p_buffer->mask, mask);
            SetDsAclQosCidKey160(V, destCategoryIdClassfied_f, p_buffer->key, 1);
            SetDsAclQosCidKey160(V, destCategoryIdClassfied_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosCidKey160(V, dstCategoryId_f, p_buffer->key, 0);
            SetDsAclQosCidKey160(V, dstCategoryId_f, p_buffer->mask, 0);
            SetDsAclQosCidKey160(V, destCategoryIdClassfied_f, p_buffer->key, 0);
            SetDsAclQosCidKey160(V, destCategoryIdClassfied_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosCidKey160(V, dscp_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosCidKey160(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosCidKey160(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosCidKey160(V, ecn_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosCidKey160(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosCidKey160(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosCidKey160(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, ipOptions_f, p_buffer->mask, mask);
        break;
	case CTC_FIELD_KEY_L4_USER_TYPE:
		if(key_field->data != CTC_PARSER_L4_USER_TYPE_UDP_VXLAN)
		{
		   return CTC_E_NOT_SUPPORT;
		}
              break;
    case CTC_FIELD_KEY_VXLAN_PKT:
        SetDsAclQosCidKey160(V, isVxlanPkt_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, isVxlanPkt_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosCidKey160(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;
        if(is_add)
        {
            tmp_data |= data << 8;
            tmp_mask |= mask << 8;
        }
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        tmp_data = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosCidKey160(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosCidKey160(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCidKey160(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosCidKey160(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosCidKey160(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        /*hw no the field*/
		if(pe->l4_type != 0)
		{
		   SetDsAclQosCidKey160(V, layer4Type_f, p_buffer->key, tmp_data);
           SetDsAclQosCidKey160(V, layer4Type_f, p_buffer->mask, tmp_mask);
		}
        break;
    case CTC_FIELD_KEY_SRC_CID:
        if(is_add)
        {
            SetDsAclQosCidKey160(V, srcCategoryId_f, p_buffer->key, data);
            SetDsAclQosCidKey160(V, srcCategoryId_f, p_buffer->mask, mask);
            SetDsAclQosCidKey160(V, srcCategoryIdClassfied_f, p_buffer->key, 1);
            SetDsAclQosCidKey160(V, srcCategoryIdClassfied_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosCidKey160(V, srcCategoryId_f, p_buffer->key, 0);
            SetDsAclQosCidKey160(V, srcCategoryId_f, p_buffer->mask, 0);
            SetDsAclQosCidKey160(V, srcCategoryIdClassfied_f, p_buffer->key, 0);
            SetDsAclQosCidKey160(V, srcCategoryIdClassfied_f, p_buffer->mask, 0);

        }
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosCidKey160(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosCidKey160(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_DA:

        SetDsAclQosCidKey160(V, ipDa_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, ipDa_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_SA:
        SetDsAclQosCidKey160(V, ipSa_f, p_buffer->key, data);
        SetDsAclQosCidKey160(V, ipSa_f, p_buffer->mask, mask);
        break;

    /*cidKeyIpv6PktUseIpDaField, choose Da or Sa, default use SA High 64bit*/
    case CTC_FIELD_KEY_IPV6_DA:
        SetDsAclQosCidKey160(V, ipAddrMode_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCidKey160(V, ipAddrMode_f, p_buffer->mask, is_add?1:0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,1,(uint32*)key_field->ext_data,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosCidKey160(A, ipAddr_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,1,(uint32*)key_field->ext_mask,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosCidKey160(A, ipAddr_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        SetDsAclQosCidKey160(V, ipAddrMode_f, p_buffer->key, 0);
        SetDsAclQosCidKey160(V, ipAddrMode_f, p_buffer->mask, is_add?1:0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,0,(uint32*)key_field->ext_data,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosCidKey160(A, ipAddr_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,0,(uint32*)key_field->ext_mask,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosCidKey160(A, ipAddr_f, p_buffer->mask, hw_ip6);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}


int32
_sys_usw_acl_add_shortkey80_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    sys_acl_buffer_t* p_buffer = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_DISCARD :
        if (!DRV_FLD_IS_EXISIT(DsAclQosKey80_t, DsAclQosKey80_discard_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        if (is_add)
        {
            SetDsAclQosKey80(V, discard_f, p_buffer->key, data);
            SetDsAclQosKey80(V, discard_f, p_buffer->mask, mask & 0x1);
        }
        else
        {
            SetDsAclQosKey80(V, discard_f, p_buffer->key, 0);
            SetDsAclQosKey80(V, discard_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosKey80(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosKey80(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosKey80(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosKey80(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosKey80(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosKey80(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosKey80(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosKey80(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosKey80(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosKey80(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_data = GetDsAclQosKey80(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosKey80(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosKey80(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosKey80(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosKey80(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosKey80(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_data = GetDsAclQosKey80(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosKey80(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosKey80(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosKey80(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosKey80(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosKey80(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosKey80(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosKey80(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosKey80(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosKey80(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosKey80(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosKey80(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosKey80(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosKey80(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosKey80(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosKey80(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosKey80(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosKey80(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosKey80(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosKey80(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosKey80(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosKey80(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosKey80(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosKey80(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosKey80(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosKey80(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosKey80(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosKey80(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        /*hw no the field*/
		if(pe->l4_type)
		{
		   SetDsAclQosKey80(V, layer4Type_f, p_buffer->key, tmp_data);
           SetDsAclQosKey80(V, layer4Type_f, p_buffer->mask, tmp_mask);
		}
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosKey80(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosKey80(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosKey80(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosKey80(V, vlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosKey80(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosKey80(V, vlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:

        SetDsAclQosKey80(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosKey80(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_STAG_VALID:

        SetDsAclQosKey80(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosKey80(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }


    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_forwardkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint8  dest_chipid = 0;
    uint16 dest_id     = 0;
    uint8  l3_type = 0;
    uint32 tmp_data    = 0;
    uint32 tmp_mask    = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint32 ip_addr[2]  = {0};
    uint8  pkt_fwd_type = 0;
    uint8  discard_type = 0;
    hw_mac_addr_t hw_mac = {0};
    uint32 hw_satpdu_byte[2] = {0};
    ipv6_addr_t hw_ip6 = {0};
    ipv6_addr_t hw_ip6_2      = {0};
    sys_nh_info_dsnh_t  nhinfo;
    sys_acl_buffer_t*   p_buffer = NULL;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    }
    l3_type = GetDsAclQosForwardKey320(V, layer3Type_f, p_buffer->key);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosForwardKey320(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        SetDsAclQosForwardKey320(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosForwardKey320(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosForwardKey320(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DST_GPORT:
        if(is_add)
        {
            dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(data);
            dest_id  = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(data);
            tmp_data = SYS_ENCODE_DESTMAP(dest_chipid, dest_id);
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->key, tmp_data);
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->mask, 0x7FFFF);
        }
        else
        {
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DST_NHID:
        if(is_add)
        {
            uint32 dest_map = 0;
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data, &nhinfo, 0));
            SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->mask, 1);
            if (nhinfo.ecmp_valid || nhinfo.is_ecmp_intf)
            {
                dest_map = nhinfo.ecmp_group_id;
                SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->key, 1);
                SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            }
            else if ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)
            {
                dest_map = nhinfo.dest_map;
                if (nhinfo.aps_en)
                {
                    SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->key, 1);
                    SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->mask, 1);
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data, &tmp_data, 0));
                CTC_ERROR_RETURN(_sys_usw_acl_get_destmap_from_dsfwd(lchip, tmp_data, &dest_map));
            }
            SetDsAclQosForwardKey320(V, nextHopInfo_f, p_buffer->key, nhinfo.dsnh_offset | (nhinfo.nexthop_ext << 18));
            if(!nhinfo.is_ecmp_intf || (!DRV_IS_DUET2(lchip) && !DRV_IS_TSINGMA(lchip) && \
                                                                ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)))
            {
                /*Limitation: ecmp tunnel mode can not match nexthop info;spec Bug 110924*/
                SetDsAclQosForwardKey320(V, nextHopInfo_f, p_buffer->mask, 0x3FFFF);
            }
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->key, dest_map);
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->mask, 0x7FFFF);

        }
        else
        {
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMap_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey320(V, nextHopInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, nextHopInfo_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMapIsEcmpGroup_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, destMapIsApsGroup_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DISCARD:
        if(is_add)
        {
            if (key_field->ext_data)
            {
                discard_type = *((uint32*)(key_field->ext_data));
            }
            tmp_data = data << 6 | discard_type;
            SetDsAclQosForwardKey320(V, discardInfo_f, p_buffer->key, tmp_data);

            tmp_data = (key_field->ext_data) ? 0x3F : 0;
            tmp_data = mask << 6 | tmp_data;
            SetDsAclQosForwardKey320(V, discardInfo_f, p_buffer->mask, tmp_data);
        }
        else
        {
            SetDsAclQosForwardKey320(V, discardInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, discardInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        if (is_add)
        {
            sys_cpu_reason_info_t  reason_info;
            uint32 igmp_mode = 0;
            sal_memset(&reason_info, 0, sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));
            pe->igmp_snooping = CTC_PKT_CPU_REASON_IGMP_SNOOPING == data;
            /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2 ,igmp use normal exception to cpu,else use fatal exception to cpu */
            sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &igmp_mode);

            if((!pe->igmp_snooping ||(pe->igmp_snooping && (igmp_mode != CTC_GLOBAL_IGMP_SNOOPING_MODE_2)))
                && reason_info.fatal_excp_valid )
            {
                SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->mask, 0);
                SetDsAclQosForwardKey320(V, fatalExceptionInfo_f, p_buffer->key, (reason_info.fatal_excp_index | (1 << 4)) & 0x1F);
                SetDsAclQosForwardKey320(V, fatalExceptionInfo_f, p_buffer->mask, (mask | (1 << 4)) & 0x1F);
            }
            else
            {
                if (!reason_info.gid_valid)
                {
                    return CTC_E_BADID;
                }
                SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | (1 << 8)) & 0x1FF);
                SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->mask, (mask | (1 << 8)) & 0x1FF);
                SetDsAclQosForwardKey320(V, fatalExceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosForwardKey320(V, fatalExceptionInfo_f, p_buffer->mask, 0);
            }
        }
        else
        {
            SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, exceptionInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key, data|(1<<14));
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:

        tmp_data = GetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0x00FF;
        tmp_mask &= 0x00FF;

        tmp_bmp_data = GetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_data |= (data << 8);
            tmp_mask |= (mask << 8);
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:

        tmp_data = GetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask);
        tmp_data &= 0xFF00;
        tmp_mask &= 0xFF00;

        tmp_bmp_data = GetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_data |= data;
            tmp_mask |= mask;
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, globalPort_f, p_buffer->mask, tmp_mask);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosForwardKey320(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosForwardKey320(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosForwardKey320(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:

        SetDsAclQosForwardKey320(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        if (0xFFF == MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID))       /* Tsingma L3 interface id support up to max 4095 */
        {
            SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->key, ((data >> 10)&0x3));
            SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->mask, ((mask >> 10)&0x3));
            data &= 0x3FF;
            mask &= 0x3FF;
        }
        SetDsAclQosForwardKey320(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        if (0xFFF == MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        if (0xFFF == MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosForwardKey320(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, layer3Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        SetDsAclQosForwardKey320(V, layer4Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, layer4Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        /*hw no the field*/
		if(pe->l4_type)
		{
		   SetDsAclQosForwardKey320(V, layer4Type_f, p_buffer->key, tmp_data);
           SetDsAclQosForwardKey320(V, layer4Type_f, p_buffer->mask, tmp_mask);
		}
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosForwardKey320(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MACSA_LKUP:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 7) : CTC_BIT_UNSET(tmp_data, 7);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 7) : CTC_BIT_UNSET(tmp_data, 7);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_MACSA_HIT:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 6) : CTC_BIT_UNSET(tmp_data, 6);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 6) : CTC_BIT_UNSET(tmp_data, 6);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_MACDA_LKUP:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 5) : CTC_BIT_UNSET(tmp_data, 5);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 5) : CTC_BIT_UNSET(tmp_data, 5);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_MACDA_HIT:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 4) : CTC_BIT_UNSET(tmp_data, 4);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 4) : CTC_BIT_UNSET(tmp_data, 4);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_IPSA_LKUP:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 3) : CTC_BIT_UNSET(tmp_data, 3);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 3) : CTC_BIT_UNSET(tmp_data, 3);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_IPSA_HIT:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 2) : CTC_BIT_UNSET(tmp_data, 2);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 2) : CTC_BIT_UNSET(tmp_data, 2);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_IPDA_LKUP:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 1) : CTC_BIT_UNSET(tmp_data, 1);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 1) : CTC_BIT_UNSET(tmp_data, 1);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_IPDA_HIT:
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(tmp_data, 0) : CTC_BIT_UNSET(tmp_data, 0);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->key, tmp_data);
        tmp_data = GetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(tmp_data, 0) : CTC_BIT_UNSET(tmp_data, 0);
        SetDsAclQosForwardKey320(V, lookupStatusInfo_f, p_buffer->mask, tmp_data);
        break;
    case CTC_FIELD_KEY_PKT_FWD_TYPE:
        if(is_add)
        {
            _sys_usw_acl_map_fwd_type((uint8)data, &pkt_fwd_type);
            SetDsAclQosForwardKey320(V, pktForwardingType_f, p_buffer->key, pkt_fwd_type);
            SetDsAclQosForwardKey320(V, pktForwardingType_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey320(V, pktForwardingType_f, p_buffer->key, 0);
            SetDsAclQosForwardKey320(V, pktForwardingType_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosForwardKey320(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, routedPacket_f, p_buffer->mask, mask);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosForwardKey320(V, dscp_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, dscp_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosForwardKey320(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosForwardKey320(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_DA:
        ip_addr[0] = data;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, ip_addr);
        ip_addr[0] = mask;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask, ip_addr);
        break;
    case CTC_FIELD_KEY_IP_SA:
        ip_addr[0] = data;
        SetDsAclQosForwardKey320(A, ipAddr2_f, p_buffer->key, ip_addr);
        ip_addr[0] = mask;
        SetDsAclQosForwardKey320(A, ipAddr2_f, p_buffer->mask, ip_addr);
        break;

    /*Fwd320BasicKeyPkt FlowTcamLookupCtl.gIngrAcl[level(2,0)].forwardBasicKeyv6IpAddressMode(1,0),default use SA High 64bit,use DA High 64bit*/
    case CTC_FIELD_KEY_IPV6_DA:
        SetDsAclQosForwardKey320(V, ipAddrMode_f, p_buffer->key, 0);
        SetDsAclQosForwardKey320(V, ipAddrMode_f, p_buffer->mask, is_add?3:0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,1,(uint32*)key_field->ext_data,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip,pe,1,(uint32*)key_field->ext_mask,hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask,hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        SetDsAclQosForwardKey320(V, ipAddrMode_f, p_buffer->key, 0);
        SetDsAclQosForwardKey320(V, ipAddrMode_f, p_buffer->mask, is_add?3:0);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 0, (uint32*)key_field->ext_data, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosForwardKey320(A, ipAddr2_f, p_buffer->key, hw_ip6);
        _sys_usw_acl_get_compress_ipv6_addr(lchip, pe, 0, (uint32*)key_field->ext_mask, hw_ip6_2);
        SYS_USW_REVERT_BYTE(hw_ip6, hw_ip6_2);
        SetDsAclQosForwardKey320(A, ipAddr2_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosForwardKey320(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= (data & 0xFF);
        tmp_mask |= (mask & 0xFF);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1:
        GetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, ip_addr);
        ip_addr[1] &= 0xFF000000;
        ip_addr[1] |= data & 0xFFFFFF;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, ip_addr);
        GetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask, ip_addr);
        ip_addr[1] &= 0xFF000000;
        ip_addr[1] |= mask & 0xFFFFFF;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask, ip_addr);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2:
        GetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, ip_addr);
        ip_addr[1] &= 0xFFFFFF;
        ip_addr[1] |= (data & 0xFF) << 24;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->key, ip_addr);
        GetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask, ip_addr);
        ip_addr[1] &= 0xFFFFFF;
        ip_addr[1] |= (mask & 0xFF) << 24;
        SetDsAclQosForwardKey320(A, ipAddr1_f, p_buffer->mask, ip_addr);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        tmp_data = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosForwardKey320(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosForwardKey320(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosForwardKey320(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosForwardKey320(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosForwardKey320(A, senderMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosForwardKey320(A, senderMac_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosForwardKey320(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosForwardKey320(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosForwardKey320(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosForwardKey320(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosForwardKey320(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey320(V, u1_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, u1_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey320(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, etherOamLevel_f, p_buffer->mask, mask);
        }

        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey320(V, u1_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, u1_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey320(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey320(V, u1_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, u1_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey320(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosForwardKey320(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosForwardKey320(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    /*MPLS*/
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosForwardKey320(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey320(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosForwardKey320(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosForwardKey320(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosForwardKey320(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosForwardKey320(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosForwardKey320(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosForwardKey320(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosForwardKey320(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosForwardKey320(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:

        SetDsAclQosForwardKey320(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosForwardKey320(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosForwardKey320(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosForwardKey320(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosForwardKey320(V, ttl_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ttl_f, p_buffer->mask, mask);
        break;
    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosForwardKey320(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosForwardKey320(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosForwardKey320(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosForwardKey320(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosForwardKey320(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*SAT PDU*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosForwardKey320(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosForwardKey320(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosForwardKey320(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosForwardKey320(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosForwardKey320(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosForwardKey320(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosForwardKey320(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosForwardKey320(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosForwardKey320(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosForwardKey320(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosForwardKey320(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosForwardKey320(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosForwardKey320(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosForwardKey320(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosForwardKey320(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosForwardKey320(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*ether Type*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosForwardKey320(V, etherType_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, etherType_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_CVLAN_ID:
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosForwardKey320(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey320(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey320(V, vlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_IS_LOG_PKT:
        if(is_add)
        {
            SetDsAclQosForwardKey320(V, isLogToCpuEn_f, p_buffer->key, data? 1: 0);
            SetDsAclQosForwardKey320(V, isLogToCpuEn_f, p_buffer->mask, mask & 0x1);
        }
        else
        {
            SetDsAclQosForwardKey320(V, isLogToCpuEn_f, p_buffer->key,  0);
            SetDsAclQosForwardKey320(V, isLogToCpuEn_f, p_buffer->mask, 0);
        }
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_forwardkey640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data     = 0;
    uint32 mask     = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint32 tmp_gport_data = 0;
    uint32 tmp_gport_mask = 0;
    uint8  tmp_bmp_data = 0;
    uint8  tmp_bmp_mask = 0;
    uint32 dest_map = 0;
    uint8  pkt_fwd_type;
    uint8  dest_chipid = 0;
    uint16 dest_id     = 0;
    uint8  lkup_info = 0;
    uint8  l3_type   = 0;
    uint8  discard_type = 0;
    hw_mac_addr_t hw_mac = {0};
    uint32 hw_satpdu_byte[2] = {0};
    ipv6_addr_t   hw_ip6 = {0};
    ipv6_addr_t   hw_ip6_mask = {0};
    sys_nh_info_dsnh_t  nhinfo;
    sys_acl_buffer_t* p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_data = NULL;
    ctc_acl_tunnel_data_t* p_tunnel_mask = NULL;
    uint32 temp_value_data[4] = {0};
    uint32 temp_value_mask[4] = {0};
    uint32 merge_data_ipDa[4] = {0};
    uint32 merge_mask_ipDa[4] = {0};
    uint32 tmp_ipsa[4] = {0};
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;
    p_rg_info = &(pe->rg_info);
    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    }
    l3_type = GetDsAclQosForwardKey640(V, layer3Type_f, p_buffer->key);

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosForwardKey640(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_globalPort_f))
        {
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->key, (drv_acl_port_info.gport >> 8) & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->mask, (drv_acl_port_info.gport_mask >> 8) & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->key, drv_acl_port_info.gport & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->mask, drv_acl_port_info.gport_mask & 0xFF);
        }
        else
        {
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key, drv_acl_port_info.gport);
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
        }
        SetDsAclQosForwardKey640(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
        SetDsAclQosForwardKey640(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->key, drv_acl_port_info.gport_type);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->mask, drv_acl_port_info.gport_type_mask);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->key, drv_acl_port_info.bitmap);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_mask);
        break;
    case CTC_FIELD_KEY_CLASS_ID:
        SetDsAclQosForwardKey640(V, aclLabel_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, aclLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DST_GPORT:
        if(is_add)
        {
            dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(data);
            dest_id  = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(data);
            tmp_data = SYS_ENCODE_DESTMAP(dest_chipid, dest_id);
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->key, tmp_data);
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->mask, 0x7FFFF);
        }
        else
        {
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DST_NHID:
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data, &nhinfo, 0));
            SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->mask, 1);
            if (nhinfo.ecmp_valid || nhinfo.is_ecmp_intf)
            {
                dest_map = nhinfo.ecmp_group_id;
                SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->key, 1);
                SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            }
            else if ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)
            {
                dest_map = nhinfo.dest_map;
                if (nhinfo.aps_en)
                {
                    SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->key, 1);
                    SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->mask, 1);
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data, &tmp_data, 0));
                CTC_ERROR_RETURN(_sys_usw_acl_get_destmap_from_dsfwd(lchip, tmp_data, &dest_map));
            }
            SetDsAclQosForwardKey640(V, nextHopInfo_f, p_buffer->key, nhinfo.dsnh_offset | (nhinfo.nexthop_ext << 18));
            if(!nhinfo.is_ecmp_intf || (!DRV_IS_DUET2(lchip) && !DRV_IS_TSINGMA(lchip) && \
                                                                ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)))
            {
                /*Limitation: ecmp tunnel mode can not match nexthop info;spec Bug 110924*/
                SetDsAclQosForwardKey640(V, nextHopInfo_f, p_buffer->mask, 0x3FFFF);
            }
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->key, dest_map);
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->mask, 0x7FFFF);
        }
        else
        {
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMap_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey640(V, nextHopInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, nextHopInfo_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMapIsEcmpGroup_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, destMapIsApsGroup_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DISCARD:
        if(is_add)
        {
            if (key_field->ext_data)
            {
                discard_type = *((uint32*)(key_field->ext_data));
            }
            tmp_data = data << 6 | discard_type;
            SetDsAclQosForwardKey640(V, discardInfo_f, p_buffer->key, tmp_data);

            tmp_data = (key_field->ext_data) ? 0x3F : 0;
            tmp_data = mask << 6 | tmp_data;
            SetDsAclQosForwardKey640(V, discardInfo_f, p_buffer->mask, tmp_data);
        }
        else
        {
            SetDsAclQosForwardKey640(V, discardInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, discardInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
    if (is_add)
    {
        sys_cpu_reason_info_t  reason_info;
        uint8  igmp_mode = 0;
        sal_memset(&reason_info, 0, sizeof(sys_cpu_reason_info_t));
        reason_info.reason_id = data;
        reason_info.acl_key_valid = 1;
        CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));

        pe->igmp_snooping = CTC_PKT_CPU_REASON_IGMP_SNOOPING == data;

          /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2 ,igmp use normal exception to cpu,else use fatal exception to cpu */
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &igmp_mode);

        if((!pe->igmp_snooping ||(pe->igmp_snooping && (igmp_mode != CTC_GLOBAL_IGMP_SNOOPING_MODE_2)))
                && reason_info.fatal_excp_valid )
        {
            SetDsAclQosForwardKey640(V, exceptionInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, exceptionInfo_f, p_buffer->mask, 0);
            SetDsAclQosForwardKey640(V, fatalExceptionInfo_f, p_buffer->key, (reason_info.fatal_excp_index | (1 << 4)) & 0x1F);
            SetDsAclQosForwardKey640(V, fatalExceptionInfo_f, p_buffer->mask, (mask | (1 << 4)) & 0x1F);
        }
        else
        {
            if (!reason_info.gid_valid)
            {
                return CTC_E_BADID;
            }
            SetDsAclQosForwardKey640(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | ( 1 << 8)) & 0x1FF);
            SetDsAclQosForwardKey640(V, exceptionInfo_f, p_buffer->mask, (mask | ( 1 << 8)) & 0x1FF);
            SetDsAclQosForwardKey640(V, fatalExceptionInfo_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, fatalExceptionInfo_f, p_buffer->mask, 0);
        }
    }
    else
    {
        SetDsAclQosForwardKey640(V, fatalExceptionInfo_f, p_buffer->mask, 0);
        SetDsAclQosForwardKey640(V, exceptionInfo_f, p_buffer->mask, 0);
    }
    break;
    case CTC_FIELD_KEY_GEM_PORT:
        SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask, mask);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_METADATA:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_globalPort_f))
        {
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->key, ((data | (1 << 14)) >> 8) & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->mask, (mask >> 8) & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->key, data & 0xFF);
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->mask, mask & 0xFF);
        }
        else
        {
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key, (data | (1 << 14)));
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask, (mask & 0xFF));
        }
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->key, DRV_FLOWPORTTYPE_METADATA);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->mask, 0x3);
        break;
    case CTC_FIELD_KEY_SRC_CID:
        tmp_bmp_data = GetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01<<1);
        tmp_bmp_mask &= ~(0x01<<1);
        if(is_add)
        {
            tmp_bmp_data |= (0x01<<1);
            tmp_bmp_mask |= (0x01<<1);
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_globalPort_f))
        {
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, globalPort15To8_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= ((data << 8)&0xFF00);
            tmp_gport_mask |= ((mask << 8)&0xFF00);
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0x00ff));
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0x00ff));
        }
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_DST_CID))?0x3:0);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DST_CID:
        tmp_bmp_data = GetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->key);
        tmp_bmp_mask = GetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->mask);
        tmp_bmp_data &= ~(0x01);
        tmp_bmp_mask &= ~(0x01);
        if(is_add)
        {
            tmp_bmp_data |= 0x01;
            tmp_bmp_mask |= 0x01;
        }
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_globalPort_f))
        {
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, globalPort7To0_f, p_buffer->mask, mask);
        }
        else
        {
            tmp_gport_data = GetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key);
            tmp_gport_mask = GetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask);
            tmp_gport_data |= (data&0xFF);
            tmp_gport_mask |= (mask&0xFF);
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->key, is_add?tmp_gport_data:(tmp_gport_data&0xff00));
            SetDsAclQosForwardKey640(V, globalPort_f, p_buffer->mask, is_add?tmp_gport_mask:(tmp_gport_mask&0xff00));
        }
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->key, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?DRV_FLOWPORTTYPE_METADATA:0);
        SetDsAclQosForwardKey640(V, globalPortType_f, p_buffer->mask, (is_add||CTC_BMP_ISSET(pe->key_bmp, CTC_FIELD_KEY_SRC_CID))?0x3:0);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->key, tmp_bmp_data);
        SetDsAclQosForwardKey640(V, portBitmap_f, p_buffer->mask, tmp_bmp_mask);
        break;
    case CTC_FIELD_KEY_DECAP:
        SetDsAclQosForwardKey640(V, isDecap_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isDecap_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ELEPHANT_PKT:
        SetDsAclQosForwardKey640(V, isElephant_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isElephant_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL:
        SetDsAclQosForwardKey640(V, isMcastRpfCheckFail_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isMcastRpfCheckFail_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosForwardKey640(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L2_TYPE:
        SetDsAclQosForwardKey640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_VLAN_NUM:
        SetDsAclQosForwardKey640(V, layer2Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, layer2Type_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        SetDsAclQosForwardKey640(V, layer3Type_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, layer3Type_f, p_buffer->mask, mask);
        break;
	case CTC_FIELD_KEY_L4_TYPE:
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, hw_ip6);
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, hw_ip6_mask);

        hw_ip6[1]      = (hw_ip6[1]&0xFFFFFFF0)|(data&0xF);
        hw_ip6_mask[1] = (hw_ip6_mask[1]&0xFFFFFFF0)|(mask&0xF);

        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, hw_ip6);
        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, hw_ip6_mask);
		break;
	case CTC_FIELD_KEY_L4_USER_TYPE:
		SetDsAclQosForwardKey640(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
	    CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;
        SetDsAclQosForwardKey640(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
   case CTC_FIELD_KEY_MACSA_LKUP:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACSA_HIT:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_LKUP:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_HIT:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_LKUP:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_HIT:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_LKUP:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_HIT:
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosForwardKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_PKT_FWD_TYPE:
        if(is_add)
        {
            _sys_usw_acl_map_fwd_type((uint8)data, &pkt_fwd_type);
            SetDsAclQosForwardKey640(V, pktForwardingType_f, p_buffer->key, pkt_fwd_type);
            SetDsAclQosForwardKey640(V, pktForwardingType_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey640(V, pktForwardingType_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, pktForwardingType_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosForwardKey640(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, routedPacket_f, p_buffer->mask, mask);
        break;
    /*u1Type, default 0
        1: udf or nsh*/
    case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, ctagCfi_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ctagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, ctagCos_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ctagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, cvlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, cvlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, cvlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_MAC_DA:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosForwardKey640(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosForwardKey640(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosForwardKey640(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosForwardKey640(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, stagCfi_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, stagCfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, stagCos_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, stagCos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, svlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        SetDsAclQosForwardKey640(V, svlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, svlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_RANGE:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 1, &data));
        SetDsAclQosForwardKey640(V, svlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, svlanId_f, p_buffer->mask, 0xFFF);
        break;
    case CTC_FIELD_KEY_CVLAN_RANGE:
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, 0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 0x01:0);
        CTC_ERROR_RETURN(_sys_usw_acl_map_vlan_range(lchip, key_field, pe->group->group_info.dir, 0, &data));
        SetDsAclQosForwardKey640(V, cvlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, cvlanId_f, p_buffer->mask, 0xFFF);
        break;
    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosForwardKey640(V, dscp_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosForwardKey640(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosForwardKey640(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosForwardKey640(V, ecn_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosForwardKey640(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, fragInfo_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_DA:
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, hw_ip6);
        hw_ip6[0] = data;
        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, hw_ip6);
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, hw_ip6);
        hw_ip6[0] = mask;
        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_SA:
        hw_ip6[0] = data;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, hw_ip6);
        hw_ip6[0] = mask;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_DA:
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, hw_ip6);
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
        SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, hw_ip6);
        ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosForwardKey640(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosForwardKey640(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
        SetDsAclQosForwardKey640(V, ipv6FlowLabel_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ipv6FlowLabel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= (data & 0xFF);
        tmp_mask |= (mask & 0xFF);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_GRE_KEY:
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, (data >> 16) & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, (mask >> 16) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_NVGRE_KEY:
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->key, (data & 0xFF) << 8);
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->mask, (mask & 0xFF) << 8);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, (data >> 8) & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, (mask >> 8) & 0xFFFF);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV1:
        GetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, tmp_ipsa);
        tmp_ipsa[1] &= 0xFF000000;
        tmp_ipsa[1] |= data & 0xFFFFFF;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, tmp_ipsa);
        GetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, tmp_ipsa);
        tmp_ipsa[1] &= 0xFF000000;
        tmp_ipsa[1] |= mask & 0xFFFFFF;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, tmp_ipsa);
        break;
    case CTC_FIELD_KEY_VXLAN_RSV2:
        GetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, tmp_ipsa);
        tmp_ipsa[1] &= 0xFFFFFF;
        tmp_ipsa[1] |= (data & 0xFF) << 24;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->key, tmp_ipsa);
        GetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, tmp_ipsa);
        tmp_ipsa[1] &= 0xFFFFFF;
        tmp_ipsa[1] |= (mask & 0xFF) << 24;
        SetDsAclQosForwardKey640(A, ipSa_f, p_buffer->mask, tmp_ipsa);
        break;
    case CTC_FIELD_KEY_VXLAN_FLAGS:
        tmp_data = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key) & 0xFF;
        tmp_mask = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask) & 0xFF;
        tmp_data |= (data << 8) & 0xFF00;
        tmp_mask |= (mask << 8) & 0xFF00;
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_VN_ID:
        tmp_data = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key) & 0xFF00;
        tmp_mask = GetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask) & 0xFF00;
        tmp_data |= (data >> 16) & 0xFF;
        tmp_mask |= (mask >> 16) & 0xFF;
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->key, data & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4DestPort_f, p_buffer->mask, mask & 0xFFFF);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosForwardKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosForwardKey640(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosForwardKey640(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosForwardKey640(V, ttl_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ttl_f, p_buffer->mask, mask);
        break;
    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosForwardKey640(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosForwardKey640(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosForwardKey640(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosForwardKey640(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosForwardKey640(A, senderMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosForwardKey640(A, senderMac_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_TARGET_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosForwardKey640(A, targetMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosForwardKey640(A, targetMac_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        SetDsAclQosForwardKey640(V, arpDestMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, arpDestMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        SetDsAclQosForwardKey640(V, arpSrcMacCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, arpSrcMacCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        SetDsAclQosForwardKey640(V, arpSenderIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, arpSenderIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        SetDsAclQosForwardKey640(V, arpTargetIpCheckFail_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, arpTargetIpCheckFail_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosForwardKey640(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;
    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey640(V, u2_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, u2_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey640(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey640(V, u2_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, u2_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey640(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == l3_type)
        {
            SetDsAclQosForwardKey640(V, u2_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, u2_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosForwardKey640(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosForwardKey640(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosForwardKey640(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosForwardKey640(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if(is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosForwardKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    /*Fcoe*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosForwardKey640(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosForwardKey640(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, fcoeSid_f, p_buffer->mask, mask);
        break;
    /*Trill*/
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosForwardKey640(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosForwardKey640(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosForwardKey640(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosForwardKey640(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosForwardKey640(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosForwardKey640(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:
        SetDsAclQosForwardKey640(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosForwardKey640(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosForwardKey640(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosForwardKey640(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, trillVersion_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosForwardKey640(V, u2_g5_ttl_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, u2_g5_ttl_f, p_buffer->mask, mask);
        break;
    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosForwardKey640(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosForwardKey640(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosForwardKey640(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;
    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosForwardKey640(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosForwardKey640(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ptpVersion_f, p_buffer->mask, mask);
        break;
    /*SAT PDU*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosForwardKey640(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosForwardKey640(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosForwardKey640(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosForwardKey640(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;
    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosForwardKey640(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosForwardKey640(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosForwardKey640(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosForwardKey640(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosForwardKey640(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosForwardKey640(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosForwardKey640(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosForwardKey640(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosForwardKey640(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosForwardKey640(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosForwardKey640(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosForwardKey640(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, secTagVer_f, p_buffer->mask, mask);
        break;
    /*EtherType*/
    case CTC_FIELD_KEY_ETHER_TYPE:
        SetDsAclQosForwardKey640(V, etherType_f, p_buffer->key, data);
        SetDsAclQosForwardKey640(V, etherType_f, p_buffer->mask, mask);
        break;

    /*Aware Tunnel Info Share u2_g1_ipDa_f */
    case CTC_FIELD_KEY_AWARE_TUNNEL_INFO:
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key,  merge_data_ipDa);
        GetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, merge_mask_ipDa);
        if(is_add)
        {
            uint8 drv_merge_type = SYS_ACL_MERGEDATA_TYPE_NONE;
            uint8 drv_merge_type_mask = SYS_ACL_MERGEDATA_TYPE_NONE;
            p_tunnel_data = (ctc_acl_tunnel_data_t*)(key_field->ext_data);
            p_tunnel_mask = (ctc_acl_tunnel_data_t*)(key_field->ext_mask);
            CTC_MIN_VALUE_CHECK(p_tunnel_data ->type, 1);
            CTC_ERROR_RETURN(_sys_usw_acl_map_mergedata_type_ctc_to_sys(lchip,p_tunnel_data ->type,&drv_merge_type, &drv_merge_type_mask));
            if(p_tunnel_data ->type == 3)
            {
                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_data->radio_mac));
                sal_memcpy((uint8*)temp_value_data, &hw_mac, sizeof(mac_addr_t));
                temp_value_data[1] = (p_tunnel_data ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_data ->radio_id & 0x1f) << 16 | (temp_value_data[1] & 0xFFFF);

                ACL_SET_HW_MAC(hw_mac, (uint8*)( p_tunnel_mask->radio_mac));
                sal_memcpy((uint8*)temp_value_mask, &hw_mac, sizeof(mac_addr_t));
                temp_value_mask[1] = (p_tunnel_mask ->wlan_ctl_pkt & 0x01) << 21 | (p_tunnel_mask ->radio_id & 0x1f) << 16 | (temp_value_mask[1] & 0xFFFF);

                merge_data_ipDa[1] |= temp_value_data[0] << 4;
                merge_data_ipDa[2] = (temp_value_data[1] << 4) | ((temp_value_data[0] >> 28) & 0xf)|(drv_merge_type<<26);

                merge_mask_ipDa[1] |= temp_value_mask[0] << 4;
                merge_mask_ipDa[2] = (temp_value_mask[1] << 4) | ((temp_value_mask[0] >> 28) & 0xf)|(drv_merge_type_mask<<26);

                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, merge_data_ipDa);
                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, merge_mask_ipDa);
            }
            else if(p_tunnel_data ->type == 2)
            {
                temp_value_data[0] = p_tunnel_data ->gre_key;
                temp_value_mask[0] = p_tunnel_mask ->gre_key;

                merge_data_ipDa[1] |= (temp_value_data[0] << 4);
                merge_data_ipDa[2] = (temp_value_data[1] << 4) | ((temp_value_data[0] >> 28) & 0xf)|(drv_merge_type<<26);

                merge_mask_ipDa[1] |= (temp_value_mask[0] << 4);
                merge_mask_ipDa[2] = (temp_value_mask[1] << 4) | ((temp_value_mask[0] >> 28) & 0xf)|(drv_merge_type_mask<<26);

                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, merge_data_ipDa);
                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, merge_mask_ipDa);
            }
            else if(p_tunnel_data ->type == 1)
            {
                temp_value_data[0] = p_tunnel_data ->vxlan_vni;
                temp_value_mask[0] = p_tunnel_mask ->vxlan_vni;

                merge_data_ipDa[1] |= temp_value_data[0] << 4;
                merge_data_ipDa[2] = (temp_value_data[1] << 4) | ((temp_value_data[0] >> 28) & 0xf)|(drv_merge_type<<26);

                merge_mask_ipDa[1] |= temp_value_mask[0] << 4;
                merge_mask_ipDa[2] = (temp_value_mask[1] << 4) | ((temp_value_mask[0] >> 28) & 0xf)|(drv_merge_type_mask<<26);

                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, merge_data_ipDa);
                SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, merge_mask_ipDa);
            }
        }
        else
        {
            SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->key, temp_value_data);
            SetDsAclQosForwardKey640(A, ipDa_f, p_buffer->mask, temp_value_data);
        }
        break;

        /*UDF*/
    case CTC_FIELD_KEY_UDF:
        {
             ctc_acl_udf_t* p_udf_info_data = (ctc_acl_udf_t*)key_field->ext_data;
             ctc_acl_udf_t* p_udf_info_mask = (ctc_acl_udf_t*)key_field->ext_mask;
             sys_acl_udf_entry_t *p_udf_entry = NULL;
             uint32     hw_udf[4] = {0};

             CTC_PTR_VALID_CHECK(p_udf_info_data);
             CTC_PTR_VALID_CHECK(p_udf_info_mask);

             if (is_add)
             {
                 _sys_usw_acl_get_udf_info(lchip, p_udf_info_data->udf_id, &p_udf_entry);
                 if (!p_udf_entry || !p_udf_entry->key_index_used )
                 {
                     return CTC_E_NOT_EXIST;
                 }
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_data->udf);
                 SetDsAclQosForwardKey640(A, udf_f, p_buffer->key,  hw_udf);
                 SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_mask->udf);
                 SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, hw_udf);
                 SetDsAclQosForwardKey640(V, udfHitIndex_f, p_buffer->key,  p_udf_entry->udf_hit_index);
                 SetDsAclQosForwardKey640(V, udfHitIndex_f, p_buffer->mask,  0xF);
             }
             else
             {
                 sal_memset(p_udf_info_mask->udf, 0, CTC_ACL_UDF_BYTE_NUM);
                 SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, p_udf_info_mask->udf);
                 SetDsAclQosForwardKey640(V, udfHitIndex_f, p_buffer->mask, 0);
             }
            SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
            SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->key, 0);
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, is_add ? 1:0);
            SetDsAclQosForwardKey640(V, udfValid_f, p_buffer->key, is_add ? 1:0);
            SetDsAclQosForwardKey640(V, udfValid_f, p_buffer->mask, is_add ? 1:0);
            pe->udf_id = is_add ? p_udf_info_data->udf_id : 0;
        }
        break;

    /*NSH*/
    case CTC_FIELD_KEY_NSH_CBIT:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_isNsh_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);
        GetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);
        temp_value_data[1] = (temp_value_data[1] & 0xFFFFFDFF);
        temp_value_data[1] |= ((data & 0x01) << 9);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        temp_value_mask[1] = (temp_value_mask[1] & 0xFFFFFDFF);
        temp_value_mask[1] |= ((mask & 0x01) << 9);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->key, 1);
        if(temp_value_data[0] || temp_value_data[1])
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 0);
        }

        break;
    case CTC_FIELD_KEY_NSH_NEXT_PROTOCOL:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_isNsh_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);
        GetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);
        temp_value_data[0] = (temp_value_data[0] & 0xFFFF00FF);
        temp_value_data[0] |= ((data & 0xFF) << 8);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        temp_value_mask[0] = (temp_value_mask[0] & 0xFFFF00FF);
        temp_value_mask[0] |= ((mask & 0xFF) << 8);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);

        if(temp_value_data[0] || temp_value_data[1])
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_NSH_OBIT:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_isNsh_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);
        GetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);
        temp_value_data[1] = (temp_value_data[1] & 0xFFFFFEFF);
        temp_value_data[1] |= ((data & 0x01) << 8);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        temp_value_mask[1] = (temp_value_mask[1] & 0xFFFFFEFF);
        temp_value_mask[1] |= ((mask & 0x01) << 8);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);

        if(temp_value_data[0] || temp_value_data[1])
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_NSH_SI:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_isNsh_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);
        GetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);
        temp_value_data[0] = (temp_value_data[0] & 0xFFFFFF00);
        temp_value_data[0] |= (data & 0xFF);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        temp_value_mask[0] = (temp_value_mask[0] & 0xFFFFFF00);
        temp_value_mask[0] |= (mask & 0xFF);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);

        if(temp_value_data[0] || temp_value_data[1])
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_NSH_SPI:
        if (!DRV_FLD_IS_EXISIT(DsAclQosForwardKey640_t, DsAclQosForwardKey640_isNsh_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->key, is_add ? 1:0);
        SetDsAclQosForwardKey640(V, u1Type_f, p_buffer->mask, is_add ? 1:0);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);
        temp_value_data[0] = (temp_value_data[0] & 0x0000FFFF);
        temp_value_data[0] |= ((data & 0xFFFF) << 16);
        temp_value_data[1] = temp_value_data[1] & 0xFFFFFF00;
        temp_value_data[1] |= ((data >> 16) & 0xFF);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->key, temp_value_data);

        GetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);
        temp_value_mask[0] = (temp_value_mask[0] & 0x0000FFFF);
        temp_value_mask[0] |= ((mask & 0xFFFF) << 16);
        temp_value_mask[1] = temp_value_mask[1] & 0xFFFFFF00;
        temp_value_mask[1] |= ((mask >> 16) & 0xFF);
        SetDsAclQosForwardKey640(A, udf_f, p_buffer->mask, temp_value_mask);

        if(temp_value_data[0] || temp_value_data[1])
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 1);
        }
        else
        {
            SetDsAclQosForwardKey640(V, isNsh_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_IS_LOG_PKT:
        SetDsAclQosForwardKey640(V, isLogToCpuEn_f, p_buffer->key, data? 1: 0);
        SetDsAclQosForwardKey640(V, isLogToCpuEn_f, p_buffer->mask, mask & 0x1);
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
		return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_coppkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint8  lkup_info = 0;
    uint8  pkt_fwd_type =0;
    uint8  cether_type;
    uint32 dest_map = 0;
    /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11*/
    //uint8  discard_type = 0;
    hw_mac_addr_t hw_mac = {0};
	ipv6_addr_t   hw_ip6 = {0};
    uint32 hw_satpdu_byte[2] = {0};
    int32  ret  = CTC_E_NONE;
    sys_nh_info_dsnh_t  nhinfo;
    sys_acl_buffer_t*   p_buffer = NULL;
    uint8 arp_addr_ck_fail_bitmap_data = 0;
    uint8 arp_addr_ck_fail_bitmap_mask = 0;
    drv_acl_port_info_t drv_acl_port_info;
    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    }


    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosCoppKey320(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey320(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosCoppKey320(A, portBitmap_f, p_buffer->key,  drv_acl_port_info.bitmap_64);
        SetDsAclQosCoppKey320(A, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_64_mask);
        SetDsAclQosCoppKey320(V, portBase_f, p_buffer->key,  drv_acl_port_info.bitmap_base);
        SetDsAclQosCoppKey320(V, portBase_f, p_buffer->mask, drv_acl_port_info.bitmap_base_mask);
        break;
    case CTC_FIELD_KEY_DST_NHID:
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data, &nhinfo, 0));
            if (nhinfo.ecmp_valid || nhinfo.is_ecmp_intf)
            {
                dest_map = nhinfo.ecmp_group_id;
                SetDsAclQosCoppKey320(V, destMapIsEcmpGroup_f, p_buffer->key, 1);
                SetDsAclQosCoppKey320(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            }
            else if ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)
            {
                dest_map = nhinfo.dest_map;
                if (nhinfo.aps_en)
                {
                    SetDsAclQosCoppKey320(V, destMapIsApsGroup_f, p_buffer->key, 1);
                    SetDsAclQosCoppKey320(V, destMapIsApsGroup_f, p_buffer->mask, 1);
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data, &tmp_data, 0));
                CTC_ERROR_RETURN(_sys_usw_acl_get_destmap_from_dsfwd(lchip, tmp_data, &dest_map));
            }
            SetDsAclQosCoppKey320(V, nextHopInfo_f, p_buffer->key, nhinfo.dsnh_offset | (nhinfo.nexthop_ext << 18));
            if(!nhinfo.is_ecmp_intf || (!DRV_IS_DUET2(lchip) && !DRV_IS_TSINGMA(lchip) && \
                                                                ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)))
            {
                /*Limitation: ecmp tunnel mode can not match nexthop info;spec Bug 110924*/
                SetDsAclQosCoppKey320(V, nextHopInfo_f, p_buffer->mask, 0x3FFFF);
            }
            SetDsAclQosCoppKey320(V, destMap_f, p_buffer->key, dest_map);
            SetDsAclQosCoppKey320(V, destMap_f, p_buffer->mask, 0x7FFFF);
        }
        else
        {
            SetDsAclQosCoppKey320(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, destMap_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey320(V, nextHopInfo_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, nextHopInfo_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey320(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, destMapIsEcmpGroup_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey320(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, destMapIsApsGroup_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DISCARD:
        if(is_add)
        {
            /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11: begin*/
            tmp_data |= GetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key);
            tmp_data |= data;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key, tmp_data);
            tmp_data |= GetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask);

            tmp_data |= mask;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask, tmp_data);
#if 0
            if (key_field->ext_data)
            {
                discard_type = *((uint32*)(key_field->ext_data));
            }
            tmp_data = data << 6 | discard_type;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key, tmp_data);

            tmp_data = (key_field->ext_data) ? 0x3F : 0;
            tmp_data = mask << 6 | tmp_data;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask, tmp_data);
#endif
            /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11: end*/
        }
        else
        {
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask, 0);
        }
        break;
/* if FlowTcamLookupCtl.coppBasicKeyEtherTypeShareType set
                        +-------+-------------------+------------------+-----------------+
    etherType[15:0] --> | 0[15] | layer4Type[14:11] | layer3Type[10:7] | cEtherType[6:0] |
                        +-------+-------------------+------------------+-----------------+
*/
    case CTC_FIELD_KEY_L3_TYPE:
        {
            uint8 is_ip_packet = (CTC_PARSER_L3_TYPE_IP == data) || (CTC_PARSER_L3_TYPE_IPV4 == data) || (CTC_PARSER_L3_TYPE_IPV6 == data);
            SetDsAclQosCoppKey320(V, isIpPacket_f, p_buffer->key, is_ip_packet);
            SetDsAclQosCoppKey320(V, isIpPacket_f, p_buffer->mask, is_ip_packet);
        }
        if (CTC_PARSER_L3_TYPE_IP != data)
        {
            tmp_data = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->key);
            tmp_mask = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask);
            tmp_data &= 0x787F;
            tmp_mask &= 0x787F;
            tmp_data |= ((data&0xF) << 7);
            tmp_mask |= ((mask&0xF) << 7);
            SetDsAclQosCoppKey320(V, etherType_f, p_buffer->key, tmp_data);
            SetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask, tmp_mask);
        }
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        tmp_data = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask);
        tmp_data &= 0x07FF;
        tmp_mask &= 0x07FF;
        if(is_add)
        {
            tmp_data |= ((data&0xF) << 11);
            tmp_mask |= ((mask&0xF) << 11);
        }
        SetDsAclQosCoppKey320(V, etherType_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosCoppKey320(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, layer4UserType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        tmp_data = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask);
        tmp_data = (tmp_data & 0x7F80);
        tmp_mask &= 0x7F80;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, data, pe->ether_type, &cether_type, &pe->ether_type_index));
            tmp_data |= cether_type;
            tmp_mask |= 0x7F;
            pe->ether_type = data;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe->ether_type));
            pe->ether_type = 0;
        }
        SetDsAclQosCoppKey320(V, etherType_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, etherType_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        {
        uint32 igmp_mode = 0;
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &igmp_mode);
        if(is_add)
        {
            sys_cpu_reason_info_t  reason_info;
            sal_memset(&reason_info,0,sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;
            pe->igmp_snooping = CTC_PKT_CPU_REASON_IGMP_SNOOPING == data;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));

            /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2 ,igmp use normal exception to cpu,else use fatal exception to cpu */


            if((!pe->igmp_snooping ||(pe->igmp_snooping && (igmp_mode != CTC_GLOBAL_IGMP_SNOOPING_MODE_2)))
                && reason_info.fatal_excp_valid )
            {
                SetDsAclQosCoppKey320(V, exceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosCoppKey320(V, exceptionInfo_f, p_buffer->mask, 0);
                SetDsAclQosCoppKey320(V, fatalExceptionInfo_f, p_buffer->key,(reason_info.fatal_excp_index |(1<< 4)) & 0x1F);
                SetDsAclQosCoppKey320(V, fatalExceptionInfo_f, p_buffer->mask, (mask |(1<< 4)) & 0x1F);
            }
            else
            {
                if(!reason_info.gid_valid)
                {
                    return CTC_E_BADID;
                }
                SetDsAclQosCoppKey320(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | (1 << 8)) & 0x1FF);
                SetDsAclQosCoppKey320(V, exceptionInfo_f, p_buffer->mask, (mask | (1 << 8)) & 0x1FF);
                SetDsAclQosCoppKey320(V, fatalExceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosCoppKey320(V, fatalExceptionInfo_f, p_buffer->mask, 0);
           }
           if(pe->igmp_snooping && (igmp_mode == CTC_GLOBAL_IGMP_SNOOPING_MODE_1))
           {
              reason_info.reason_id = CTC_PKT_CPU_REASON_IGMP_SNOOPING;
              SetDsAcl(V, exceptionIndex_f, p_buffer->action, reason_info.exception_index);
              SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, reason_info.exception_subIndex);
              SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,  1);
           }
        }
        else
        {
           if(pe->igmp_snooping && (igmp_mode == CTC_GLOBAL_IGMP_SNOOPING_MODE_1))
           {
              SetDsAcl(V, exceptionIndex_f, p_buffer->action, 0);
              SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, 0);
              SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,  0);
           }
            SetDsAclQosCoppKey320(V, exceptionInfo_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey320(V, fatalExceptionInfo_f, p_buffer->mask, 0);
        }
        break;
        }
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosCoppKey320(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    /*Lookup status info*/
   case CTC_FIELD_KEY_MACSA_LKUP:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACSA_HIT:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_LKUP:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_HIT:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_LKUP:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_HIT:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_LKUP:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_HIT:
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosCoppKey320(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MAC_DA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey320(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey320(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_PKT_FWD_TYPE:
        if(is_add)
        {
            _sys_usw_acl_map_fwd_type((uint8)data, &pkt_fwd_type);
            SetDsAclQosCoppKey320(V, pktForwardingType_f, p_buffer->key, pkt_fwd_type);
            SetDsAclQosCoppKey320(V, pktForwardingType_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey320(V, pktForwardingType_f, p_buffer->key, 0);
            SetDsAclQosCoppKey320(V, pktForwardingType_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_L2_STATION_MOVE:
        SetDsAclQosCoppKey320(V, l2StationMove_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, l2StationMove_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_SECURITY_DISCARD:
        SetDsAclQosCoppKey320(V, macSecurityDiscard_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, macSecurityDiscard_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosCoppKey320(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, routedPacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        SetDsAclQosCoppKey320(V, fragInfo_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, fragInfo_f, p_buffer->mask, tmp_mask);
        /**< [TM] */
        SetDsAclQosCoppKey320(V, isFragmentPkt_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, isFragmentPkt_f, p_buffer->mask, tmp_mask & 0x1);
        break;
    case CTC_FIELD_KEY_IP_DA: /* for ipv4 addr,cppp key only support ip_da; FlowTcamLookupCtl, coppKeyIpAddrShareType0: default to 1 */
		{
		   data = is_add? data:0;
           mask = is_add? mask:0;
  		   GetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->key, hw_ip6);
  		   hw_ip6[0] = data;
  		   SetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->key, hw_ip6);

  		   GetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->mask, hw_ip6);
  		   hw_ip6[0] = mask;
  		   SetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->mask, hw_ip6);
    	}
        break;
    case CTC_FIELD_KEY_IP_SA:
        {
            data = is_add? data:0;
            mask = is_add? mask:0;
            GetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->key, hw_ip6);
            hw_ip6[0] = data;
            SetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->key, hw_ip6);

            GetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->mask, hw_ip6);
            hw_ip6[0] = mask;
            SetDsAclQosCoppKey320(A, ipAddr_f, p_buffer->mask,hw_ip6);
        }
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosCoppKey320(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosCoppKey320(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosCoppKey320(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= (data & 0xFF);
        tmp_mask |= (mask & 0xFF);
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;

        SetDsAclQosCoppKey320(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;

    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosCoppKey320(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosCoppKey320(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosCoppKey320(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey320(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey320(A, macDa_f, p_buffer->mask, hw_mac);
        break;

    /*CK ARP Fail */
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 3);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 3);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 3): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 3);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 3): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 3);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 2);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 2);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 2): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 2);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 2): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 2);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 1);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 1);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 1): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 1);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 1): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 1);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 0);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 0);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 0): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 0);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 0): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 0);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey320(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosCoppKey320(V, isGratuitousArp_f, p_buffer->key, (data? 1: 0));
        SetDsAclQosCoppKey320(V, isGratuitousArp_f, p_buffer->mask, (mask & 0x1));
        break;

    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey320(V, u1_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, u1_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey320(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey320(V, u1_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, u1_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey320(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey320(V, u1_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, u1_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey320(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosCoppKey320(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosCoppKey320(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    /*MPLS*/
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosCoppKey320(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey320(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;

    /*FCOE*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosCoppKey320(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosCoppKey320(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, fcoeSid_f, p_buffer->mask, mask);
        break;

    /*Trill*/
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosCoppKey320(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosCoppKey320(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosCoppKey320(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosCoppKey320(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosCoppKey320(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosCoppKey320(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, trillVersion_f, p_buffer->mask, mask);
        break;

    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosCoppKey320(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosCoppKey320(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosCoppKey320(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;

    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosCoppKey320(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosCoppKey320(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ptpVersion_f, p_buffer->mask, mask);
        break;

    /*SatPdu*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosCoppKey320(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosCoppKey320(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, ouiSubType_f, p_buffer->mask, mask);
        break;

    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosCoppKey320(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosCoppKey320(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;

    case CTC_FIELD_KEY_STP_STATE:
        SetDsAclQosCoppKey320(V, stpStatus_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, stpStatus_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_LOG_PKT:
        SetDsAclQosCoppKey320(V, isLogToCpuEn_f, p_buffer->key, data ? 1:0);
        SetDsAclQosCoppKey320(V, isLogToCpuEn_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, vlanId_f, p_buffer->mask, mask);
        break;
	case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, vlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
		SetDsAclQosCoppKey320(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey320(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;

    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey320(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
	case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, cfi_f, p_buffer->key,  data);
        SetDsAclQosCoppKey320(V, cfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, cfi_f, p_buffer->key,  data);
        SetDsAclQosCoppKey320(V, cfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, cos_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, cos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey320(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey320(V, cos_f, p_buffer->key, data);
        SetDsAclQosCoppKey320(V, cos_f, p_buffer->mask, mask);
        break;


    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }


    return ret;
}

int32
_sys_usw_acl_add_coppkey640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint8  lkup_info = 0;
    uint8  pkt_fwd_type = 0;
    uint32 dest_map  = 0;
    uint8  c_ether_type = 0;
    /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11*/
    //uint8  discard_type = 0;
    uint32 hw_satpdu_byte[2] = {0};
    hw_mac_addr_t hw_mac = {0};
    ipv6_addr_t   hw_ip6 = {0};
    int32  ret  = CTC_E_NONE;
    sys_nh_info_dsnh_t  nhinfo;
    sys_acl_buffer_t*   p_buffer = NULL;
    sys_acl_range_info_t* p_rg_info = &(pe->rg_info);
    uint8 arp_addr_ck_fail_bitmap_data = 0;
    uint8 arp_addr_ck_fail_bitmap_mask = 0;
    drv_acl_port_info_t drv_acl_port_info;

    sal_memset(&drv_acl_port_info,0,sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;

    if(is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    }
	else
	{
	  mask = 0;
	}

    switch(key_field->type)
    {
    case CTC_FIELD_KEY_RARP:
        SetDsAclQosCoppKey640(V, isRarp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey640(V, isRarp_f, p_buffer->mask, mask & 0x1);
        break;
    case CTC_FIELD_KEY_PORT:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
        }
        SetDsAclQosCoppKey640(A, portBitmap_f, p_buffer->key,  drv_acl_port_info.bitmap_64);
        SetDsAclQosCoppKey640(A, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_64_mask);
        SetDsAclQosCoppKey640(V, portBase_f, p_buffer->key,  drv_acl_port_info.bitmap_base);
        SetDsAclQosCoppKey640(V, portBase_f, p_buffer->mask, drv_acl_port_info.bitmap_base_mask);
        break;
    case CTC_FIELD_KEY_DST_NHID:
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data, &nhinfo, 0));
            if (nhinfo.ecmp_valid || nhinfo.is_ecmp_intf)
            {
                dest_map = nhinfo.ecmp_group_id;
                SetDsAclQosCoppKey640(V, destMapIsEcmpGroup_f, p_buffer->key, 1);
                SetDsAclQosCoppKey640(V, destMapIsEcmpGroup_f, p_buffer->mask, 1);
            }
            else if ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)
            {
                dest_map = nhinfo.dest_map;
                if (nhinfo.aps_en)
                {
                    SetDsAclQosCoppKey640(V, destMapIsApsGroup_f, p_buffer->key, 1);
                    SetDsAclQosCoppKey640(V, destMapIsApsGroup_f, p_buffer->mask, 1);
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data, &tmp_data, 0));
                CTC_ERROR_RETURN(_sys_usw_acl_get_destmap_from_dsfwd(lchip, tmp_data, &dest_map));
            }
            SetDsAclQosCoppKey640(V, nextHopInfo_f, p_buffer->key, nhinfo.dsnh_offset | (nhinfo.nexthop_ext << 18));
            if(!nhinfo.is_ecmp_intf || (!DRV_IS_DUET2(lchip) && !DRV_IS_TSINGMA(lchip) && \
                                                                ((nhinfo.merge_dsfwd == 1) || !nhinfo.dsfwd_valid)))
            {
                /*Limitation: ecmp tunnel mode can not match nexthop info;spec Bug 110924*/
                SetDsAclQosCoppKey640(V, nextHopInfo_f, p_buffer->mask, 0x3FFFF);
            }
            SetDsAclQosCoppKey640(V, destMap_f, p_buffer->key, dest_map);
            SetDsAclQosCoppKey640(V, destMap_f, p_buffer->mask, 0x7FFFF);

        }
        else
        {
            SetDsAclQosCoppKey640(V, destMap_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, destMap_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey640(V, nextHopInfo_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, nextHopInfo_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey640(V, destMapIsEcmpGroup_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, destMapIsEcmpGroup_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey640(V, destMapIsApsGroup_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, destMapIsApsGroup_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_DISCARD:
        if(is_add)
        {
            /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11: begin*/
            tmp_data |= GetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key);
            tmp_data |= data;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->key, tmp_data);
            tmp_data |= GetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask);

            tmp_data |= mask;
            SetDsAclQosCoppKey320(V, discardInfo_f, p_buffer->mask, tmp_data);
#if 0
            if (key_field->ext_data)
            {
                discard_type = *((uint32*)(key_field->ext_data));
            }
            tmp_data = data << 6 | discard_type;
            SetDsAclQosCoppKey640(V, discardInfo_f, p_buffer->key, tmp_data);

            tmp_data = (key_field->ext_data) ? 0x3F : 0;
            tmp_data = mask << 6 | tmp_data;
            SetDsAclQosCoppKey640(V, discardInfo_f, p_buffer->mask, tmp_data);
#endif
            /*SYSTEM MODIFY, Added by weij for requirement 224, 2017-09-11: end*/
        }
        else
        {
            SetDsAclQosCoppKey640(V, discardInfo_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, discardInfo_f, p_buffer->mask, 0);
        }
        break;
    case CTC_FIELD_KEY_L3_TYPE:
        {
            uint8 is_ip_packet = (CTC_PARSER_L3_TYPE_IP == data) || (CTC_PARSER_L3_TYPE_IPV4 == data) || (CTC_PARSER_L3_TYPE_IPV6 == data);
            SetDsAclQosCoppKey640(V, isIpPacket_f, p_buffer->key, is_ip_packet);
            SetDsAclQosCoppKey640(V, isIpPacket_f, p_buffer->mask, is_ip_packet);
        }
        if (CTC_PARSER_L3_TYPE_IP != data)
        {
            tmp_data = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->key);
            tmp_mask = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask);
            tmp_data &= 0xF87F;
            tmp_mask &= 0xF87F;
            if (is_add)
            {
                tmp_data |= ((data&0xF) << 7);
                tmp_mask |= ((mask&0xF) << 7);
            }
            SetDsAclQosCoppKey640(V, etherType_f, p_buffer->key, tmp_data);
            SetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask, tmp_mask);
        }
        break;
    case CTC_FIELD_KEY_L4_TYPE:
        tmp_data = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask);
        tmp_data &= 0x87FF;
        tmp_mask &= 0x87FF;
        if(is_add)
        {
            tmp_data |= ((data&0xF) << 11);
            tmp_mask |= ((mask&0xF) << 11);
        }
        SetDsAclQosCoppKey640(V, etherType_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask, tmp_mask);
        break;
	case CTC_FIELD_KEY_L4_USER_TYPE:
        SetDsAclQosCoppKey640(V, layer4UserType_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, layer4UserType_f, p_buffer->mask, mask);
		break;
    case CTC_FIELD_KEY_ETHER_TYPE:
        tmp_data = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask);
        tmp_data &= 0xFF80;
        tmp_mask &= 0xFF80;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_compress_ether_type(lchip, data, pe->ether_type, &c_ether_type, &pe->ether_type_index));
            tmp_data |= c_ether_type;
            tmp_mask |= 0x7F;
            pe->ether_type = data;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_register_remove_compress_ether_type(lchip, pe->ether_type));
            pe->ether_type = 0;
        }
        SetDsAclQosCoppKey640(V, etherType_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, etherType_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_CPU_REASON_ID:
        {
        uint32 igmp_mode = 0;
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &igmp_mode);
        if(is_add)
        {
            sys_cpu_reason_info_t  reason_info;

            sal_memset(&reason_info,0,sizeof(sys_cpu_reason_info_t));
            reason_info.reason_id = data;
            reason_info.acl_key_valid = 1;

             pe->igmp_snooping = CTC_PKT_CPU_REASON_IGMP_SNOOPING == data;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_get_reason_info(lchip, pe->group->group_info.dir, &reason_info));

            /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2,igmp use normal exception to cpu,else use fatal exception to cpu */

            if((!pe->igmp_snooping ||(pe->igmp_snooping && (igmp_mode != CTC_GLOBAL_IGMP_SNOOPING_MODE_2)))
                && reason_info.fatal_excp_valid )
            {
                SetDsAclQosCoppKey640(V, exceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosCoppKey640(V, exceptionInfo_f, p_buffer->mask, 0);
                SetDsAclQosCoppKey640(V, fatalExceptionInfo_f, p_buffer->key,(reason_info.fatal_excp_index |(1<< 4)) & 0x1F);
                SetDsAclQosCoppKey640(V, fatalExceptionInfo_f, p_buffer->mask, (mask |(1<< 4)) & 0x1F);
            }
            else
            {
                if(!reason_info.gid_valid)
                {
                    return CTC_E_BADID;
                }
                SetDsAclQosCoppKey640(V, exceptionInfo_f, p_buffer->key, (reason_info.gid_for_acl_key | ( 1<< 8)) & 0x1FF);
                SetDsAclQosCoppKey640(V, exceptionInfo_f, p_buffer->mask, (mask | ( 1<< 8)) & 0x1FF);
                SetDsAclQosCoppKey640(V, fatalExceptionInfo_f, p_buffer->key, 0);
                SetDsAclQosCoppKey640(V, fatalExceptionInfo_f, p_buffer->mask, 0);
            }
           if(pe->igmp_snooping && (igmp_mode == CTC_GLOBAL_IGMP_SNOOPING_MODE_1))
           {
              reason_info.reason_id = CTC_PKT_CPU_REASON_IGMP_SNOOPING;
              SetDsAcl(V, exceptionIndex_f, p_buffer->action, reason_info.exception_index);
              SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, reason_info.exception_subIndex);
              SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,  1);
           }
        }
        else
        {
            if (pe->igmp_snooping && (igmp_mode == CTC_GLOBAL_IGMP_SNOOPING_MODE_1))
            {
                SetDsAcl(V, exceptionIndex_f, p_buffer->action, 0);
                SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, 0);
                SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,  0);
            }
            SetDsAclQosCoppKey640(V, exceptionInfo_f, p_buffer->mask, 0);
            SetDsAclQosCoppKey640(V, fatalExceptionInfo_f, p_buffer->mask, 0);
        }
        break;
        }
    case CTC_FIELD_KEY_INTERFACE_ID:
        SetDsAclQosCoppKey640(V, l3InterfaceId_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, l3InterfaceId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PROTOCOL:
        CTC_ERROR_RETURN(_sys_usw_acl_map_ip_protocol_to_l4_type
                                        (lchip, CTC_PARSER_L3_TYPE_IPV4, data, &tmp_data, &tmp_mask));
        pe->l4_type = (tmp_data !=0 && is_add) ? tmp_data :pe->l4_type;

        SetDsAclQosCoppKey640(V, layer3HeaderProtocol_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
        break;
    /*Lookup status info*/
   case CTC_FIELD_KEY_MACSA_LKUP:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 7) : CTC_BIT_UNSET(lkup_info, 7);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACSA_HIT:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 6) : CTC_BIT_UNSET(lkup_info, 6);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_LKUP:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 5) : CTC_BIT_UNSET(lkup_info, 5);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MACDA_HIT:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 4) : CTC_BIT_UNSET(lkup_info, 4);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_LKUP:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 3) : CTC_BIT_UNSET(lkup_info, 3);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPSA_HIT:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 2) : CTC_BIT_UNSET(lkup_info, 2);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_LKUP:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 1) : CTC_BIT_UNSET(lkup_info, 1);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_IPDA_HIT:
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key);
        data ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->key, lkup_info);
        lkup_info = GetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask);
        mask ? CTC_BIT_SET(lkup_info, 0) : CTC_BIT_UNSET(lkup_info, 0);
        SetDsAclQosCoppKey640(V, lookupStatusInfo_f, p_buffer->mask, lkup_info);
        break;
    case CTC_FIELD_KEY_MAC_DA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey640(A, macDa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey640(A, macDa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_MAC_SA:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey640(A, macSa_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey640(A, macSa_f, p_buffer->mask, hw_mac);
        break;
    case CTC_FIELD_KEY_PKT_FWD_TYPE:
        if(is_add)
        {
            _sys_usw_acl_map_fwd_type((uint8)data, &pkt_fwd_type);
            SetDsAclQosCoppKey640(V, pktForwardingType_f, p_buffer->key, pkt_fwd_type);
            SetDsAclQosCoppKey640(V, pktForwardingType_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey640(V, pktForwardingType_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, pktForwardingType_f, p_buffer->mask,0);
        }
        break;
    case CTC_FIELD_KEY_L2_STATION_MOVE:
        SetDsAclQosCoppKey640(V, l2StationMove_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, l2StationMove_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MAC_SECURITY_DISCARD:
        SetDsAclQosCoppKey640(V, macSecurityDiscard_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, macSecurityDiscard_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ROUTED_PKT:
        SetDsAclQosCoppKey640(V, routedPacket_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, routedPacket_f, p_buffer->mask, mask);
        break;

    /*IP*/
    case CTC_FIELD_KEY_IP_DSCP:  /*For RFC2474,SDK support CTC_FIELD_KEY_IP_DSCP, CTC_FIELD_KEY_IP_ECN;*/
        SetDsAclQosCoppKey640(V, dscp_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, dscp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_PRECEDENCE: /*For RFC1349,SDK support CTC_FIELD_KEY_IP_PRECEDENCE;*/
        SetDsAclQosCoppKey640(V, dscp_f, p_buffer->key, data << 3);
        SetDsAclQosCoppKey640(V, dscp_f, p_buffer->mask, mask << 3);
        break;
    case CTC_FIELD_KEY_IP_ECN:
        SetDsAclQosCoppKey640(V, ecn_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ecn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_FRAG:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
        }
        if (DRV_IS_DUET2(lchip) || CTC_IP_FRAG_YES != key_field->data)
        {
            SetDsAclQosCoppKey640(V, fragInfo_f, p_buffer->key, tmp_data);
            SetDsAclQosCoppKey640(V, fragInfo_f, p_buffer->mask, tmp_mask);
        }
        SetDsAclQosCoppKey640(V, isFragmentPkt_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, isFragmentPkt_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IPV6_SA:
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
            SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->key, hw_ip6);
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
            SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IPV6_DA:
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_data);
            SetDsAclQosCoppKey640(A, udf_f, p_buffer->key, hw_ip6);
            ACL_SET_HW_IP6(hw_ip6, (uint32*)key_field->ext_mask);
            SetDsAclQosCoppKey640(A, udf_f, p_buffer->mask, hw_ip6);

            SetDsAclQosCoppKey640(V, udfValid_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, udfValid_f, p_buffer->mask, 1);
            SetDsAclQosCoppKey640(V, udfHitIndex_f, p_buffer->key, 0);
            SetDsAclQosCoppKey640(V, udfHitIndex_f, p_buffer->mask, 1);
        break;
    case CTC_FIELD_KEY_IP_SA:
        GetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->key, hw_ip6);
        hw_ip6[1] = (hw_ip6[1]&0x0) | (key_field->data);
        SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->key, hw_ip6);
        GetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->mask, hw_ip6);
        hw_ip6[1] = (hw_ip6[1]&0x0) | (key_field->mask);
        SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_DA:
        GetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->key, hw_ip6);
        hw_ip6[0] = (hw_ip6[0]&0x0) | (key_field->data);
        SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->key, hw_ip6);
        GetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->mask, hw_ip6);
        hw_ip6[0] = (hw_ip6[0]&0x0) | (key_field->mask);
        SetDsAclQosCoppKey640(A, ipAddr_f, p_buffer->mask, hw_ip6);
        break;
    case CTC_FIELD_KEY_IP_HDR_ERROR:
        SetDsAclQosCoppKey640(V, ipHeaderError_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ipHeaderError_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_OPTIONS:
        SetDsAclQosCoppKey640(V, ipOptions_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ipOptions_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT:
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT:
        SetDsAclQosCoppKey640(V, l4DestPort_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, l4DestPort_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ICMP_CODE:
        tmp_data = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= (data & 0xFF);
        tmp_mask |= (mask & 0xFF);
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_ICMP_TYPE:
        tmp_data = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IGMP_TYPE:
        tmp_data = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask);
        tmp_data |= data << 8;
        tmp_mask |= mask << 8;
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, l4SourcePort_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_IP_PKT_LEN_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_PKT_LENGTH, p_rg_info));
        }
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_DST_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_DST_PORT, p_rg_info));
        }
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_L4_SRC_PORT_RANGE:
        if(is_add)
        {
            CTC_ERROR_RETURN(_sys_usw_acl_add_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, data, mask, p_rg_info));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_acl_remove_field_range(
                                lchip, ACL_RANGE_TYPE_L4_SRC_PORT, p_rg_info));
        }
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->key, p_rg_info->range_bitmap);
        SetDsAclQosCoppKey640(V, rangeBitmap_f, p_buffer->mask, p_rg_info->range_bitmap_mask);
        break;
    case CTC_FIELD_KEY_TCP_ECN:
        SetDsAclQosCoppKey640(V, tcpEcn_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, tcpEcn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TCP_FLAGS:
        SetDsAclQosCoppKey640(V, tcpFlags_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, tcpFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IP_TTL:
        SetDsAclQosCoppKey640(V, ttl_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ttl_f, p_buffer->mask, mask);
        break;

    /*ARP*/
    case CTC_FIELD_KEY_ARP_OP_CODE:
        SetDsAclQosCoppKey640(V, arpOpCode_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, arpOpCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
        SetDsAclQosCoppKey640(V, protocolType_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, protocolType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDER_IP:
        SetDsAclQosCoppKey640(V, senderIp_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, senderIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGET_IP:
        SetDsAclQosCoppKey640(V, targetIp_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, targetIp_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SENDER_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey640(A, senderMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey640(A, senderMac_f, p_buffer->mask, hw_mac);
        break;
     case CTC_FIELD_KEY_TARGET_MAC:
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_data));
            SetDsAclQosCoppKey640(A, targetMac_f, p_buffer->key, hw_mac);
            ACL_SET_HW_MAC(hw_mac, (uint8*)(key_field->ext_mask));
            SetDsAclQosCoppKey640(A, targetMac_f, p_buffer->mask, hw_mac);
        break;

        /*CK ARP Fail */
    case CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 3);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 3);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 3): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 3);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 3): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 3);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 2);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 2);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 2): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 2);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 2): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 2);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 1);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 1);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 1): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 1);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 1): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 1);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL:
        arp_addr_ck_fail_bitmap_data = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key);
        arp_addr_ck_fail_bitmap_mask = GetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 0);
        CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 0);
        data? CTC_BIT_SET(arp_addr_ck_fail_bitmap_data, 0): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_data, 0);
        mask? CTC_BIT_SET(arp_addr_ck_fail_bitmap_mask, 0): CTC_BIT_UNSET(arp_addr_ck_fail_bitmap_mask, 0);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->key, arp_addr_ck_fail_bitmap_data);
        SetDsAclQosCoppKey640(V, arpAddrCheckFailBitMap_f, p_buffer->mask, arp_addr_ck_fail_bitmap_mask);
        break;
    case CTC_FIELD_KEY_GARP:
        SetDsAclQosCoppKey640(V, isGratuitousArp_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey640(V, isGratuitousArp_f, p_buffer->mask, mask & 0x1);
        break;

    /*Ether Oam*/
    case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey640(V, u2_g7_etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, u2_g7_etherOamLevel_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey640(V, etherOamLevel_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, etherOamLevel_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey640(V, u2_g7_etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, u2_g7_etherOamOpCode_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey640(V, etherOamOpCode_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, etherOamOpCode_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_ETHER_OAM_VERSION:
        if(CTC_PARSER_L3_TYPE_ETHER_OAM == pe->l3_type)
        {
            SetDsAclQosCoppKey640(V, u2_g7_etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, u2_g7_etherOamVersion_f, p_buffer->mask, mask);
        }
        else
        {
            SetDsAclQosCoppKey640(V, etherOamVersion_f, p_buffer->key, data);
            SetDsAclQosCoppKey640(V, etherOamVersion_f, p_buffer->mask, mask);
        }
        break;
    case CTC_FIELD_KEY_IS_Y1731_OAM:
        SetDsAclQosCoppKey640(V, isY1731Oam_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, isY1731Oam_f, p_buffer->mask, mask);
        break;
    /*MPLS*/
    case CTC_FIELD_KEY_LABEL_NUM:
        SetDsAclQosCoppKey640(V, labelNum_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, labelNum_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL0:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP0:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT0:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL0:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel0_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL1:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP1:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT1:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL1:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel1_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_LABEL2:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0x00000FFF;
        tmp_mask &= 0x00000FFF;
        if(is_add)
        {
            tmp_data |= (data << 12);
            tmp_mask |= (mask << 12);
        }
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_EXP2:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFF1FF;
        tmp_mask &= 0xFFFFF1FF;
        if(is_add)
        {
            tmp_data |= ((data & 0x7) << 9);
            tmp_mask |= ((mask & 0x7) << 9);
        }
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_SBIT2:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFEFF;
        tmp_mask &= 0xFFFFFEFF;
        if(is_add)
        {
            tmp_data |= ((data & 0x1) << 8);
            tmp_mask |= ((mask & 0x1) << 8);
        }
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;
    case CTC_FIELD_KEY_MPLS_TTL2:
        tmp_data = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key);
        tmp_mask = GetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask);
        tmp_data &= 0xFFFFFF00;
        tmp_mask &= 0xFFFFFF00;
        if (is_add)
        {
            tmp_data |= (data & 0xFF);
            tmp_mask |= (mask & 0xFF);
        }
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->key, tmp_data);
        SetDsAclQosCoppKey640(V, mplsLabel2_f, p_buffer->mask, tmp_mask);
        break;

    /*FCOE*/
    case CTC_FIELD_KEY_FCOE_DST_FCID:
        SetDsAclQosCoppKey640(V, fcoeDid_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, fcoeDid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_FCOE_SRC_FCID:
        SetDsAclQosCoppKey640(V, fcoeSid_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, fcoeSid_f, p_buffer->mask, mask);
        break;

    /*Trill*/
    case CTC_FIELD_KEY_TRILL_TTL:
        SetDsAclQosCoppKey640(V, u2_g5_ttl_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, u2_g5_ttl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_INGRESS_NICKNAME:
        SetDsAclQosCoppKey640(V, ingressNickname_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ingressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_EGRESS_NICKNAME:
        SetDsAclQosCoppKey640(V, egressNickname_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, egressNickname_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_ESADI:
        SetDsAclQosCoppKey640(V, isEsadi_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, isEsadi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_TRILL_CHANNEL:
        SetDsAclQosCoppKey640(V, isTrillChannel_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, isTrillChannel_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID:
        SetDsAclQosCoppKey640(V, trillInnerVlanId_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillInnerVlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID:
        SetDsAclQosCoppKey640(V, trillInnerVlanValid_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillInnerVlanValid_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_LENGTH:
        SetDsAclQosCoppKey640(V, trillLength_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillLength_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTIHOP:
        SetDsAclQosCoppKey640(V, trillMultiHop_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillMultiHop_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_MULTICAST:
        SetDsAclQosCoppKey640(V, trillMulticast_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillMulticast_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_TRILL_VERSION:
        SetDsAclQosCoppKey640(V, trillVersion_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, trillVersion_f, p_buffer->mask, mask);
        break;

    /*Slow Protocol*/
    case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
        SetDsAclQosCoppKey640(V, slowProtocolCode_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, slowProtocolCode_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
        SetDsAclQosCoppKey640(V, slowProtocolFlags_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, slowProtocolFlags_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
        SetDsAclQosCoppKey640(V, slowProtocolSubType_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, slowProtocolSubType_f, p_buffer->mask, mask);
        break;

    /*PTP*/
    case CTC_FIELD_KEY_PTP_MESSAGE_TYPE:
        SetDsAclQosCoppKey640(V, ptpMessageType_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ptpMessageType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_PTP_VERSION:
        SetDsAclQosCoppKey640(V, ptpVersion_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ptpVersion_f, p_buffer->mask, mask);
        break;

    /*SatPdu*/
    case CTC_FIELD_KEY_SATPDU_MEF_OUI:
        SetDsAclQosCoppKey640(V, mefOui_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, mefOui_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE:
        SetDsAclQosCoppKey640(V, ouiSubType_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, ouiSubType_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_SATPDU_PDU_BYTE:
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosCoppKey640(A, pduByte_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SATPDU_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosCoppKey640(A, pduByte_f, p_buffer->mask, hw_satpdu_byte);
        break;

    /*Dot1AE*/
    case CTC_FIELD_KEY_DOT1AE_AN:
        SetDsAclQosCoppKey640(V, secTagAn_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagAn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_ES:
        SetDsAclQosCoppKey640(V, secTagEs_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagEs_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_PN:
        SetDsAclQosCoppKey640(V, secTagPn_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagPn_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SC:
        SetDsAclQosCoppKey640(V, secTagSc_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagSc_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCI:
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_data));
        SetDsAclQosCoppKey640(A, secTagSci_f, p_buffer->key, hw_satpdu_byte);
        ACL_SET_HW_SCI_BYTE(hw_satpdu_byte, (uint32*)(key_field->ext_mask));
        SetDsAclQosCoppKey640(A, secTagSci_f, p_buffer->mask, hw_satpdu_byte);
        break;
    case CTC_FIELD_KEY_DOT1AE_SL:
        SetDsAclQosCoppKey640(V, secTagSl_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagSl_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT:
        SetDsAclQosCoppKey640(V, unknownDot1AePacket_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, unknownDot1AePacket_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_CBIT:
        SetDsAclQosCoppKey640(V, secTagCbit_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagCbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_EBIT:
        SetDsAclQosCoppKey640(V, secTagEbit_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagEbit_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_SCB:
        SetDsAclQosCoppKey640(V, secTagScb_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagScb_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_DOT1AE_VER:
        SetDsAclQosCoppKey640(V, secTagVer_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, secTagVer_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STP_STATE:
        SetDsAclQosCoppKey640(V, stpStatus_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, stpStatus_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_IS_LOG_PKT:
        SetDsAclQosCoppKey640(V, isLogToCpuEn_f, p_buffer->key, data? 1:0);
        SetDsAclQosCoppKey640(V, isLogToCpuEn_f, p_buffer->mask, mask & 0x1);
        break;
 case CTC_FIELD_KEY_SVLAN_ID:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, vlanId_f, p_buffer->mask, mask);
        break;
	case CTC_FIELD_KEY_CVLAN_ID:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, vlanId_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, vlanId_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_VALID:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
		SetDsAclQosCoppKey640(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey640(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;

    case CTC_FIELD_KEY_CTAG_VALID:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, vlanIdValid_f, p_buffer->key, data? 1: 0);
        SetDsAclQosCoppKey640(V, vlanIdValid_f, p_buffer->mask, mask & 0x1);
        break;
	case CTC_FIELD_KEY_CTAG_CFI:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, cfi_f, p_buffer->key,  data);
        SetDsAclQosCoppKey640(V, cfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_CFI:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, cfi_f, p_buffer->key,  data);
        SetDsAclQosCoppKey640(V, cfi_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_CTAG_COS:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, 0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, cos_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, cos_f, p_buffer->mask, mask);
        break;
    case CTC_FIELD_KEY_STAG_COS:
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->key, is_add?1:0);
        SetDsAclQosCoppKey640(V, isSvlan_f, p_buffer->mask, is_add?1:0);
        SetDsAclQosCoppKey640(V, cos_f, p_buffer->key, data);
        SetDsAclQosCoppKey640(V, cos_f, p_buffer->mask, mask);
        break;

    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  field range flag:%u\n", p_rg_info->flag);
    return ret;
}

int32
_sys_usw_acl_operate_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field, uint8 is_add)
{
    sys_acl_entry_t* pe_lkup = NULL;
	uint8 hash_key = 0;
	uint8 field_valid = 0;
    ds_t  remove_ext_data = {0};
    ds_t  ext_mask_temp   = {0};

    /*check*/
    SYS_ACL_INIT_CHECK();
    CTC_PTR_VALID_CHECK(key_field);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*get entry and check*/
    _sys_usw_acl_get_entry_by_eid(lchip, entry_id, &pe_lkup);
    if (!pe_lkup || !pe_lkup->buffer)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }

    /*installed entry not support to add key field*/
    if(FPA_ENTRY_FLAG_INSTALLED == pe_lkup->fpae.flag)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry installed \n");
	    return CTC_E_INVALID_CONFIG;
    }

    hash_key = !ACL_ENTRY_IS_TCAM(pe_lkup->key_type);

    if (hash_key && pe_lkup->hash_valid)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "add_key_field, eid:%u hash_valid:%d\n", pe_lkup->fpae.entry_id, pe_lkup->hash_valid);
     	return CTC_E_EXIST;
    }

    if (hash_key && key_field->type == CTC_FIELD_KEY_HASH_VALID && !pe_lkup->hash_valid)
    {
       return _sys_usw_acl_get_hash_index(lchip, pe_lkup, 0,0);
    }

    if(!p_usw_acl_master[lchip]->build_key_func[pe_lkup->key_type])
    {
		return CTC_E_INVALID_PARAM;
    }

    if(is_add && key_field->ext_mask == NULL)
    {
        sal_memset(ext_mask_temp, 0xFF, sizeof(ds_t));
        key_field->ext_mask = ext_mask_temp;
    }

    if(!hash_key)
    {
       CTC_ERROR_RETURN(_sys_usw_acl_check_tcam_key_union( lchip,  key_field, pe_lkup,  is_add));
    }
    if ((!is_add) && (CTC_FIELD_KEY_PORT != key_field->type))
    {
        key_field->ext_data = remove_ext_data;
        key_field->ext_mask = remove_ext_data;
    }
 	CTC_ERROR_RETURN(p_usw_acl_master[lchip]->build_key_func[pe_lkup->key_type](lchip, key_field, pe_lkup, is_add));

     if (!hash_key)
     {
         CTC_ERROR_RETURN(_sys_usw_acl_set_tcam_key_union( lchip,  key_field, pe_lkup, is_add));
     }

     SYS_USW_ACL_SET_BMP(pe_lkup->key_bmp, key_field->type, is_add);

     if (hash_key)
     {
         CTC_ERROR_RETURN(sys_usw_acl_get_hash_field_sel(lchip, pe_lkup->key_type, pe_lkup->hash_field_sel_id, key_field->type, &field_valid));
         if (!field_valid)
         {
             SYS_USW_ACL_SET_BMP(pe_lkup->key_bmp, key_field->type, 0);
         }
     }

     if (CTC_FIELD_KEY_SVLAN_RANGE == key_field->type || CTC_FIELD_KEY_CVLAN_RANGE == key_field->type)
     {
         pe_lkup->minvid = key_field->data;
     }
     else if(CTC_FIELD_KEY_DST_NHID == key_field->type)
     {
         pe_lkup->key_dst_nhid = key_field->data;
     }
     else if(CTC_FIELD_KEY_CPU_REASON_ID == key_field->type)
     {
         pe_lkup->key_reason_id= key_field->data;
     }

     return CTC_E_NONE;
}

/* internal api, just for wlan or dot1ae error to cpu function ,use it carefully */
int32
_sys_usw_acl_add_dsacl_field_wlan_error_to_cpu(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_half, uint8 is_add)
{
    uint32  data0        = 0;
    uint32  data1        = 0;
    sys_acl_buffer_t*       p_buffer = NULL;
    ctc_acl_to_cpu_t*       p_cp_to_cpu = NULL;
    sys_cpu_reason_info_t   reason_info;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;
    sal_memset(&reason_info, 0, sizeof(sys_cpu_reason_info_t));

    if(is_add)
    {
        data0 = action_field->data0;
        data1 = action_field->data1;
    }

    switch(action_field->type)
    {
    case CTC_ACL_FIELD_ACTION_CP_TO_CPU:
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
        }

        /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2,igmp use normal exception to cpu,else use fatal exception to cpu */
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &data1);
        if(pe->igmp_snooping && (data1 ==CTC_GLOBAL_IGMP_SNOOPING_MODE_1))/*igmp snoop don't support */
        {
           SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " igmp snoop don't support ACTION_CP_TO_CPU \n");
           return CTC_E_INVALID_CONFIG;
        }

        if(is_add)
        {
            uint16 old_cpu_reason_id = pe->cpu_reason_id;
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_cp_to_cpu = (ctc_acl_to_cpu_t*)action_field->ext_data;
            CTC_MAX_VALUE_CHECK(p_cp_to_cpu->mode, CTC_ACL_TO_CPU_MODE_MAX-1);
            if(p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU)
            {
                SetDsAcl(V, exceptionToCpuType_f, p_buffer->action, 2);
                if(old_cpu_reason_id)
            	{
                    reason_info.reason_id = pe->cpu_reason_id;
                    CTC_ERROR_RETURN(sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info));
            	}
                pe->cpu_reason_id = 0;
            }
            else
            {
                p_cp_to_cpu->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)?
                                              CTC_PKT_CPU_REASON_ACL_MATCH : p_cp_to_cpu->cpu_reason_id;
                if(old_cpu_reason_id)
                {
                    if(old_cpu_reason_id == p_cp_to_cpu->cpu_reason_id)
                    {
                        return CTC_E_NONE;
                    }
                    reason_info.reason_id = old_cpu_reason_id;
                    CTC_ERROR_RETURN(sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info));
                }
                reason_info.reason_id = p_cp_to_cpu->cpu_reason_id;
                CTC_ERROR_RETURN(sys_usw_cpu_reason_alloc_exception_index(lchip, pe->group->group_info.dir, &reason_info));
                SetDsAcl(V, exceptionIndex_f, p_buffer->action, reason_info.exception_index);
                SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, reason_info.exception_subIndex);
                data0 = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER) ? 3:1;
                SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,data0);
                pe->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)? 0 : p_cp_to_cpu->cpu_reason_id;
            }

            pe->action_flag |= SYS_ACL_ACTION_FLAG_LOG;
        }
		else
		{
            if(pe->cpu_reason_id)
            {
                reason_info.reason_id = pe->cpu_reason_id;
                sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info);
            }
		   SetDsAcl(V, exceptionToCpuType_f, p_buffer->action, 0);
           pe->action_flag &= (~SYS_ACL_ACTION_FLAG_LOG);
           pe->cpu_reason_id = 0;
		}
        break;

    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    SYS_USW_ACL_SET_BMP(pe->action_bmp, action_field->type, is_add);

    if(CTC_INGRESS == pe->group->group_info.dir)
    {
        pe->action_type = SYS_ACL_ACTION_TCAM_INGRESS;
    }
    else
    {
        pe->action_type = SYS_ACL_ACTION_TCAM_EGRESS;
    }

    return CTC_E_NONE;
}

STATIC int32 _sys_usw_acl_action_set_dsfwd(uint8 lchip, sys_acl_entry_t* pe, uint32 fwdptr, uint8 temp_u1_type)
{
    sys_acl_buffer_t* p_buffer = NULL;
    CTC_PTR_VALID_CHECK(pe);
    p_buffer = pe->buffer;
    CTC_PTR_VALID_CHECK(p_buffer);

    if (pe->u1_type == SYS_AD_UNION_G_2)
    {
        SetDsAcl(V, u1_g2_adBypassIngressEdit_f, &p_buffer->action, 0);
        SetDsAcl(V, u1_g2_adApsBridgeEn_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, 0);
        SYS_SET_UNION_TYPE(pe->u1_type, SYS_AD_UNION_G_2, DsAcl, 1, p_buffer->action, 0);
        _sys_usw_acl_unbind_nexthop(lchip, pe);
        SYS_SET_UNION_TYPE(pe->u1_type, SYS_AD_UNION_G_1, DsAcl, 1, p_buffer->action, 1);
    }

    /*first time clear dsacl u1_g1 and set dsfwd*/
    if (temp_u1_type == SYS_AD_UNION_G_2)
    {
        SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_ecmpGroupId_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_truncateLenProfId_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_cnActionMode_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_cnAction_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_loadBalanceHashValue_f, p_buffer->action, 0);
        SetDsAcl(V, u1_g1_dsFwdPtr_f, p_buffer->action, fwdptr);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_dsacl_field(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_half, uint8 is_add)
{
    uint32  data0        = 0;
    uint32  data1        = 0;
    uint32  fwdptr       = 0;
    uint32  stats_ptr    = 0;
    uint8   do_vlan_edit = 0;
    uint8   profile_id   = 0;
    uint8   old_profile_id   = 0;
    uint8   lantency_en_invalid = 0;
    uint32  dest_map     = 0;
    /* SYSTEM MODIFIED by yangbs for SDK bug 102710 */
    uint32  nh_offset    = 0;
    uint32  nexthop_id   = 0;
    uint8   have_dsfwd   = 0;
    uint8   temp_u1_type = pe->u1_type;
    uint32 cmd = 0;
    uint32  ingress_dot1ae_dis = 0;
    uint8 service_queue_egress_en = 0;
    sys_nh_info_dsnh_t      nhinfo;
    ctc_acl_packet_strip_t* pkt_strip = NULL;
    ctc_acl_vlan_edit_t*    p_ctc_vlan_edit = NULL;
    sys_acl_vlan_edit_t     vlan_edit;
    sys_acl_vlan_edit_t*    pv_get = NULL;
    sys_acl_buffer_t*       p_buffer = NULL;
    ctc_acl_ipfix_t*        p_ipfix = NULL;
    ctc_acl_oam_t*          p_oam = NULL;
    ctc_acl_to_cpu_t*       p_cp_to_cpu = NULL;
    sys_qos_policer_param_t policer_param;
    sys_cpu_reason_info_t   reason_info;
    ctc_acl_table_map_t*    p_table_map = NULL;
    ctc_acl_inter_cn_t*     p_inter_cn = NULL;
    ctc_acl_vxlan_rsv_edit_t* p_vxlan_rsv_edit = NULL;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_buffer = pe->buffer;
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&vlan_edit, 0, sizeof(sys_acl_vlan_edit_t));
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
    sal_memset(&reason_info, 0, sizeof(sys_cpu_reason_info_t));

    if(is_add)
    {
        data0 = action_field->data0;
        data1 = action_field->data1;
    }

    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_dot1AeFieldShareMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ingress_dot1ae_dis));

    switch(action_field->type)
    {
    case CTC_ACL_FIELD_ACTION_TIMESTAMP:
        if (DRV_IS_DUET2(lchip) || CTC_EGRESS != pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Not support timestamp \n");
            return CTC_E_NOT_SUPPORT;
        }
        {
            ctc_acl_timestamp_t* timestamp = (ctc_acl_timestamp_t*)action_field->ext_data;
            if (is_add)
            {
                CTC_MAX_VALUE_CHECK(timestamp->mode, 1);
            }
            SetDsAcl(V, aclDenyLearning_f, p_buffer->action, is_add? (timestamp->mode >> 1) & 0x1: 0);
            SetDsAcl(V, terminateCidHdr_f, p_buffer->action, is_add? timestamp->mode & 0x1: 0);
        }
        break;
    case CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN:
        if (!ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, dot1AeSpecialCase_f, p_buffer->action, data0? 1: 0);
        break;
    case CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS:
        if (!ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, dot1AeEncryptDisable_f, p_buffer->action, data0? 1: 0);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_ALL:
        if(pe->vlan_edit_valid)
        {
            CTC_ERROR_RETURN(ctc_spool_remove(p_usw_acl_master[lchip]->vlan_edit_spool, pe->vlan_edit, NULL));
            pe->vlan_edit_valid = 0;
            pe->vlan_edit = NULL;
        }

		reason_info.reason_id = pe->cpu_reason_id;
		sys_usw_cpu_reason_free_exception_index(lchip, data1, &reason_info);

        _sys_usw_acl_unbind_nexthop(lchip, pe);

        if (pe->u1_type == SYS_AD_UNION_G_1)
        {
            profile_id = GetDsAcl(V, u1_g1_truncateLenProfId_f, p_buffer->action);
            CTC_ERROR_RETURN(sys_usw_register_remove_truncation_profile(lchip, 1, profile_id));

        }

        sal_memset(p_buffer->action, 0, sizeof(p_buffer->action));
        SetDsAcl(V, isHalfAd_f, p_buffer->action, is_half);
        pe->u1_bitmap = 0;
        pe->u2_bitmap = 0;
        pe->u3_bitmap = 0;
        pe->u4_bitmap = 0;
        pe->u5_bitmap = 0;
        pe->u1_type = 0;
        pe->u2_type = 0;
        pe->u3_type = 0;
        pe->u4_type = 0;
        pe->u5_type = 0;
        pe->action_flag = 0;
        break;
    case CTC_ACL_FIELD_ACTION_DENY_LEARNING:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, aclDenyLearning_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_COLOR:
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        CTC_MAX_VALUE_CHECK(data0, MAX_CTC_QOS_COLOR - 1);
        SetDsAcl(V, color_f, p_buffer->action, data0);
        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG:
        if(is_half || CTC_EGRESS == pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, disableElephantFlowLog_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_DISCARD:
    case CTC_ACL_FIELD_ACTION_DISCARD:
        CTC_MAX_VALUE_CHECK(action_field->data0, 14);
        if(action_field->data0&0x1)
        {
            return CTC_E_INVALID_PARAM;
        }
        if (!is_add && CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_CANCEL_DISCARD) && (action_field->type == CTC_ACL_FIELD_ACTION_DISCARD))
        {
            return CTC_E_NOT_SUPPORT;
        }
        if (!is_add && CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_DISCARD) && (action_field->type == CTC_ACL_FIELD_ACTION_CANCEL_DISCARD))
        {
            return CTC_E_NOT_SUPPORT;
        }
        data1 = (action_field->type==CTC_ACL_FIELD_ACTION_CANCEL_DISCARD)?2:1;
        if(is_half)
        {
            if(CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_RED) || CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_YELLOW) || CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_GREEN))
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Acl half Ad cannot support per color discard \n");
    			return CTC_E_NOT_SUPPORT;
            }
            SetDsAcl(V, discardOpTypeGreen_f, p_buffer->action, is_add?data1:0);
        }
        else
        {
            if (is_add)
            {
                SetDsAcl(V, discardOpTypeRed_f, p_buffer->action, 0);
                SetDsAcl(V, discardOpTypeYellow_f, p_buffer->action, 0);
                SetDsAcl(V, discardOpTypeGreen_f, p_buffer->action, 0);
            }
            if(CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_RED) || CTC_QOS_COLOR_NONE == action_field->data0)
            {
                SetDsAcl(V, discardOpTypeRed_f, p_buffer->action, is_add?data1:0);
            }
            if(CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_YELLOW) || CTC_QOS_COLOR_NONE == action_field->data0)
            {
                SetDsAcl(V, discardOpTypeYellow_f, p_buffer->action, is_add?data1:0);
            }
            if(CTC_IS_BIT_SET(action_field->data0, CTC_QOS_COLOR_GREEN) || CTC_QOS_COLOR_NONE == action_field->data0)
            {
                SetDsAcl(V, discardOpTypeGreen_f, p_buffer->action, is_add?data1:0);
            }
        }
        if(is_add && (action_field->type==CTC_ACL_FIELD_ACTION_DISCARD))
        {
            pe->action_flag |= SYS_ACL_ACTION_FLAG_DISCARD;
        }
        else
        {
            pe->action_flag &= (~SYS_ACL_ACTION_FLAG_DISCARD);
        }
        (action_field->type==CTC_ACL_FIELD_ACTION_DISCARD)? \
                                    CTC_BMP_UNSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_CANCEL_DISCARD): \
                                    CTC_BMP_UNSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_DISCARD);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_NAT:
        SetDsAcl(V, doNotSnat_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE:
        if (ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, dot1AeEncryptDisable_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT:
        if (ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, dot1AeSpecialCase_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_DSCP:
        if(is_half || CTC_EGRESS == pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
        }
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsAcl(V, dscp_f, p_buffer->action, data0);
        SetDsAcl(V, dscpValid_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_CP_TO_CPU:
        if(is_half )
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }

        /*CTC_GLOBAL_IGMP_SNOOPING_MODE_2,igmp use normal exception to cpu,else use fatal exception to cpu */
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_IGMP_SNOOPING_MODE, &data1);
        if(pe->igmp_snooping && (data1 ==CTC_GLOBAL_IGMP_SNOOPING_MODE_1))/*igmp snoop don't support */
        {
           SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " igmp snoop don't support ACTION_CP_TO_CPU \n");
           return CTC_E_INVALID_CONFIG;
        }

        if(is_add)
        {
            uint16 old_cpu_reason_id = pe->cpu_reason_id;
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_cp_to_cpu = (ctc_acl_to_cpu_t*)action_field->ext_data;
            CTC_MAX_VALUE_CHECK(p_cp_to_cpu->mode, CTC_ACL_TO_CPU_MODE_MAX-1);
            if(p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU)
            {
                SetDsAcl(V, exceptionToCpuType_f, p_buffer->action, 2);
                if(old_cpu_reason_id)
            	{
                    reason_info.reason_id = pe->cpu_reason_id;
                    CTC_ERROR_RETURN(sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info));
            	}
                pe->cpu_reason_id = 0;
            }
            else
            {
                p_cp_to_cpu->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)?
                                              CTC_PKT_CPU_REASON_ACL_MATCH : p_cp_to_cpu->cpu_reason_id;
                if(old_cpu_reason_id && (old_cpu_reason_id == p_cp_to_cpu->cpu_reason_id))
                {
                    return CTC_E_NONE;
                }
                reason_info.reason_id = p_cp_to_cpu->cpu_reason_id;
                CTC_ERROR_RETURN(sys_usw_cpu_reason_alloc_exception_index(lchip, pe->group->group_info.dir, &reason_info));
                SetDsAcl(V, exceptionIndex_f, p_buffer->action, reason_info.exception_index);
                SetDsAcl(V, exceptionSubIndex_f, p_buffer->action, reason_info.exception_subIndex);
                data0 = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER) ? 3:1;
                SetDsAcl(V, exceptionToCpuType_f, p_buffer->action,data0);
                pe->cpu_reason_id = (p_cp_to_cpu->mode == CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER)? 0 : p_cp_to_cpu->cpu_reason_id;
                if(old_cpu_reason_id)
                {
                    reason_info.reason_id = old_cpu_reason_id;
                    CTC_ERROR_RETURN(sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info));
                }
            }

            pe->action_flag |= SYS_ACL_ACTION_FLAG_LOG;
        }
		else
		{
            if(pe->cpu_reason_id)
            {
                reason_info.reason_id = pe->cpu_reason_id;
                sys_usw_cpu_reason_free_exception_index(lchip, pe->group->group_info.dir, &reason_info);
            }
		   SetDsAcl(V, exceptionToCpuType_f, p_buffer->action, 0);
           pe->action_flag &= (~SYS_ACL_ACTION_FLAG_LOG);
           pe->cpu_reason_id = 0;
		}
        break;

    case CTC_ACL_FIELD_ACTION_OAM:
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        SYS_CHECK_UNION_BITMAP(pe->u1_type,SYS_AD_UNION_G_3);
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
             /*/need get mep info from oam module -->  guw*/
            p_oam = (ctc_acl_oam_t*)(action_field->ext_data);
            CTC_MAX_VALUE_CHECK(p_oam ->timestamp_format, 1);
            SetDsAcl(V, oamEn_f, p_buffer->action, 1);
            SetDsAcl(V, u1_g3_oamDestChip_f, p_buffer->action, p_oam ->dest_gchip);
            SetDsAcl(V, u1_g3_mepIndex_f, p_buffer->action, p_oam ->lmep_index);
            SetDsAcl(V, u1_g3_mipEn_f, p_buffer->action, p_oam ->mip_en?1:0);
            SetDsAcl(V, u1_g3_rxOamType_f, p_buffer->action, p_oam ->oam_type);
            SetDsAcl(V, u1_g3_linkOamEn_f, p_buffer->action, p_oam ->link_oam_en?1:0);
            SetDsAcl(V, u1_g3_packetOffset_f, p_buffer->action, p_oam ->packet_offset);
            SetDsAcl(V, u1_g3_macDaChkEn_f, p_buffer->action, p_oam ->mac_da_check_en?1:0);
            SetDsAcl(V, u1_g3_selfAddress_f, p_buffer->action, p_oam ->is_self_address?1:0);
            SetDsAcl(V, u1_g3_timestampEn_f, p_buffer->action, p_oam ->time_stamp_en?1:0);
            SetDsAcl(V, u1_g3_twampTsType_f, p_buffer->action, p_oam ->timestamp_format);
        }
        else
        {
            SetDsAcl(V, oamEn_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_oamDestChip_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_mepIndex_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_mipEn_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_rxOamType_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_linkOamEn_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_packetOffset_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_macDaChkEn_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_selfAddress_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_timestampEn_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g3_twampTsType_f, p_buffer->action, 0);
        }

        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_3,DsAcl,1,p_buffer->action, is_add);
        break;

    case CTC_ACL_FIELD_ACTION_IPFIX:
         /*u2:gIpfix,gMicroflow can overwrire, so process gMicroflow(g3) as gIpfix(g2)*/
         /*g1:gI2eSrcCid  g2-gIpfix; g3:gMicroflow,g4:gStatsOp*/
        SYS_CHECK_UNION_BITMAP(pe->u2_type,SYS_AD_UNION_G_2);
        SYS_USW_ACL_IPFIX_CONFLICT_CHECK(1, p_buffer);
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_ipfix = (ctc_acl_ipfix_t*)(action_field->ext_data);
            if(!p_ipfix->policer_id)    /* ipfix */
            {
                lantency_en_invalid = DRV_IS_DUET2(lchip) && p_ipfix->lantency_en;
                lantency_en_invalid |= !DRV_IS_DUET2(lchip) && (CTC_INGRESS == pe->group->group_info.dir && p_ipfix->lantency_en);
                CTC_ERROR_RETURN(lantency_en_invalid? CTC_E_NOT_SUPPORT: CTC_E_NONE);
                SetDsAcl(V, u2Type_f, p_buffer->action, 1);
                SetDsAcl(V, ipfixOpCode_f, p_buffer->action, 1);
                SetDsAcl(V, u2_gIpfix_ipfixCfgProfileId_f, p_buffer->action, p_ipfix->flow_cfg_id);  /*bug*/
                SetDsAcl(V, u2_gIpfix_ipfixHashFieldSel_f, p_buffer->action, p_ipfix->field_sel_id); /*bug*/
                SetDsAcl(V, u2_gIpfix_ipfixHashType_f, p_buffer->action, p_ipfix->hash_type); /*bug*/
                SetDsAcl(V, u2_gIpfix_ipfixUsePIVlan_f, p_buffer->action, p_ipfix->use_mapped_vlan);
                SetDsAcl(V, u2_gIpfix_ipfixFlowL2KeyUseCvlan_f, p_buffer->action, p_ipfix->use_cvlan);
                SetDsAcl(V, u2_gIpfix_ipfixSamplingEn_f, p_buffer->action, 0);
                SetDsAcl(V, u2_gIpfix_egrIpfixAdUseMeasureLantency_f, p_buffer->action, p_ipfix->lantency_en? 1: 0);
            }
            else    /*MFP*/
            {
                data0 = 1;
                CTC_NOT_EQUAL_CHECK(p_ipfix->policer_id, 0);
                SYS_CHECK_UNION_BITMAP(pe->u5_type,SYS_AD_UNION_G_3);
                policer_param.dir = pe->group->group_info.dir;
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, p_ipfix->policer_id, SYS_QOS_POLICER_TYPE_MFP, &policer_param));

                if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
                {
                    return CTC_E_INVALID_PARAM;
                }
                if (policer_param.is_bwp)
                {
                    return CTC_E_INVALID_PARAM;
                }
                pe->policer_id = p_ipfix->policer_id;
                SetDsAcl(V, u5_g3_microflowPolicingProfId_f, p_buffer->action, policer_param.policer_ptr);

                SetDsAcl(V, u2Type_f, p_buffer->action, 2);
                SetDsAcl(V, u2_gMicroflow_microflowCfgProfileId_f, p_buffer->action, p_ipfix->flow_cfg_id); /*bug*/
                SetDsAcl(V, u2_gMicroflow_microflowHashFieldSel_f, p_buffer->action, p_ipfix->field_sel_id); /*bug*/
                SetDsAcl(V, u2_gMicroflow_microflowHashType_f, p_buffer->action, p_ipfix->hash_type); /*bug*/
                SetDsAcl(V, u2_gMicroflow_microflowUsePIVlan_f, p_buffer->action, p_ipfix->use_mapped_vlan);
                SetDsAcl(V, u5_g3_microflowL2KeyUseCvlan_f, p_buffer->action, p_ipfix->use_cvlan);
                SetDsAcl(V, u2_gMicroflow_microflowPolicingLayer3LenEn_f, p_buffer->action, 0);
                SetDsAcl(V, u2_gMicroflow_microflowMonitorEn_f, p_buffer->action, 0);
            }
        }
        else
        {
           if(2 == GetDsAcl(V, u2Type_f, p_buffer->action))
            {
                data0 = 1;
                SetDsAcl(V, u5_g3_microflowPolicingProfId_f,p_buffer->action,0);
                SetDsAcl(V, u5_g3_microflowL2KeyUseCvlan_f,p_buffer->action,0);
            }
             /*clear u2*/
            SetDsAcl(V, u2Type_f, p_buffer->action, 0);
            SetDsAcl(V, ipfixOpCode_f, p_buffer->action, 0);
            SetDsAcl(V, u2_gStatsOp_statsPtr_f, p_buffer->action, 0);
            SetDsAcl(V, u2_gStatsOp_aclStatsIsPrivate_f, p_buffer->action, 0);      /*TM Append*/
            SetDsAcl(V, u2_gStatsOp_statsColorAware_f, p_buffer->action, 0);        /*TM Append*/
        }
        pe->policer_id = is_add ? p_ipfix->policer_id:0;
        if(data0)
        { /*microflow*/
            SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_3,DsAcl,5,p_buffer->action, is_add);
        }

        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_2,DsAcl,2,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX:
        SYS_USW_ACL_IPFIX_CONFLICT_CHECK(2, p_buffer);
        SetDsAcl(V, ipfixOpCode_f, p_buffer->action, is_add?2:0);
        break;
    case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING:
        SYS_USW_ACL_IPFIX_CONFLICT_CHECK(3, p_buffer);
        SetDsAcl(V, ipfixOpCode_f, p_buffer->action, is_add?3:0);
        break;
    case CTC_ACL_FIELD_ACTION_REDIRECT_PORT:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, data0, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nexthop_id));
            data0 = nexthop_id;
        }
          /*not need break,continue to CTC_ACL_FIELD_ACTION_REDIRECT*/
    case CTC_ACL_FIELD_ACTION_REDIRECT:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, data0, &nhinfo, 0));
            if (nhinfo.is_ecmp_intf)
            {
                return CTC_E_NOT_SUPPORT;
            }

            _sys_usw_acl_unbind_nexthop(lchip, pe);

            /*clear old nexthop associated field and db*/
            if(pe->nexthop_id)
            {
                if(pe->u1_type == SYS_AD_UNION_G_1)
                {
                    SetDsAcl(V, u1_g1_ecmpGroupId_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g1_dsFwdPtr_f, p_buffer->action, 0);
                    SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, 0);
                }
                else
                {
                    SetDsAcl(V, u1_g2_adApsBridgeEn_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, 0);
                    SetDsAcl(V, u1_g2_adBypassIngressEdit_f, p_buffer->action, 0);
                    SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsAcl,1,p_buffer->action, 0);
                }
            }
            have_dsfwd = (pe->u1_type == SYS_AD_UNION_G_1) || nhinfo.dsfwd_valid || nhinfo.cloud_sec_en;
            /*SYSTEM MODIFIED by yejl for D2 aps, 2019-05-22*/
            have_dsfwd = (pe->u1_type == SYS_AD_UNION_G_1) || nhinfo.dsfwd_valid || nhinfo.cloud_sec_en || nhinfo.aps_en;
            if(!have_dsfwd && !nhinfo.ecmp_valid)
            {
                have_dsfwd |= (CTC_E_NONE != _sys_usw_acl_bind_nexthop(lchip, pe,data0,0));
            }
            if (nhinfo.ecmp_valid)
            {
                SYS_CHECK_UNION_BITMAP(pe->u1_type,SYS_AD_UNION_G_1);
                SetDsAcl(V, u1_g1_ecmpGroupId_f, p_buffer->action, nhinfo.ecmp_group_id);
                SetDsAcl(V, u1_g1_dsFwdPtr_f, p_buffer->action, 0);
                SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 0);
                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);

            }
            else if(have_dsfwd)
            { /*must use dsfwd*/

                SYS_CHECK_UNION_BITMAP(pe->u1_type,SYS_AD_UNION_G_1);
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, data0, &fwdptr, 0));
                SetDsAcl(V, u1_g1_dsFwdPtr_f, p_buffer->action, fwdptr);
            	SetDsAcl(V, u1_g1_ecmpGroupId_f, p_buffer->action, 0);
                SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 0);
                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);
            }
            else
            { /*mergeDsFwd*/
                uint8 byPassIngressEdit = CTC_FLAG_ISSET(nhinfo.dsnh_offset, SYS_NH_DSNH_BY_PASS_FLAG);
                /* SYSTEM MODIFIED by hansf, bypass ingress edit always set 1 */
                #if defined(OFPRODUCT) || defined(TAPPRODUCT)
                byPassIngressEdit = 1;
                #endif
                uint8 adjust_len_idx = 0;
                SYS_CHECK_UNION_BITMAP(pe->u1_type,SYS_AD_UNION_G_2);
                dest_map = nhinfo.dest_map;
                SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 1);
                SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, dest_map);
                /* SYSTEM MODIFIED by yangbs for SDK bug 102710 */
                //nh_offset = nhinfo.dsnh_offset;
                //nh_offset |= (1<<17);
                SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, nh_offset);
                SetDsAcl(V, u1_g2_adApsBridgeEn_f, p_buffer->action, nhinfo.aps_en);
                SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, nhinfo.nexthop_ext);
                if(0 != nhinfo.adjust_len)
                {
                    sys_usw_lkup_adjust_len_index(lchip, nhinfo.adjust_len, &adjust_len_idx);
                    SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, adjust_len_idx);
                }
                else
                {
                    SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, 0);
                }
                /*TM Append*/
                if (!DRV_IS_DUET2(lchip) && byPassIngressEdit)
                {
                    SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, (nhinfo.dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG)));
                    SetDsAcl(V, u1_g2_adBypassIngressEdit_f, p_buffer->action, 1);
                }
                SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, nhinfo.aps_en);

                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsAcl,1,p_buffer->action, is_add);
            }
            pe->action_flag |= SYS_ACL_ACTION_FLAG_REDIRECT;
            pe->nexthop_id = data0;
        }
        else
        {
            SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 0);
            if(pe->u1_type == SYS_AD_UNION_G_1)
            {
                SetDsAcl(V, u1_g1_dsFwdPtr_f, p_buffer->action, 0);
                SetDsAcl(V, u1_g1_ecmpGroupId_f, p_buffer->action, 0);
                SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);
            }
            else if(pe->u1_type == SYS_AD_UNION_G_2)
            {
               SetDsAcl(V, u1_g2_adBypassIngressEdit_f, &p_buffer->action, 0);
               SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, 0);
               SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, 0);
               SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, 0);
               SetDsAcl(V, u1_g2_adApsBridgeEn_f, p_buffer->action, 0);
               SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, 0);
               SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, 0);
               SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsAcl,1,p_buffer->action, is_add);
           }
           _sys_usw_acl_unbind_nexthop(lchip, pe);
            pe->nexthop_id = 0;
            pe->action_flag &= (~SYS_ACL_ACTION_FLAG_REDIRECT);
        }

        break;
    case CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if (CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_STRIP_PACKET))
        {
            /* can not config at the same time */
            return CTC_E_INVALID_CONFIG;
        }
        SetDsAcl(V, clearPktEditOperation_f, p_buffer->action, is_add? 1 : 0);
        break;
    case CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT:
         SetDsAcl(V, redirectEnableMode_f, p_buffer->action, is_add?1:0);
		 break;
    case CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
        }
        SetDsAcl(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsAcl(V, policerPtr_f, p_buffer->action, is_add ?policer_param.policer_ptr:0);
        SetDsAcl(V, policerPhbEn_f, p_buffer->action, is_add?policer_param.is_bwp:0);
        break;
    case CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_MACRO_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }
             pe->policer_id = data0;
        }
        SetDsAcl(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsAcl(V, policerPtr_f, p_buffer->action, is_add ?policer_param.policer_ptr:0);
        SetDsAcl(V, policerPhbEn_f, p_buffer->action, is_add?policer_param.is_bwp:0);
        break;
    case CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COPP, CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER);
            policer_param.dir = pe->group->group_info.dir;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
                        (lchip, data0, SYS_QOS_POLICER_TYPE_MACRO_FLOW, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0)
                || (policer_param.is_bwp == 0) || (data1 > (MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL)-1)))
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
            pe->cos_index = data1;
        }
        SetDsAcl(V, policerLvlSel_f, p_buffer->action, policer_param.level);
        SetDsAcl(V, policerPtr_f, p_buffer->action, is_add ?(policer_param.policer_ptr+data1):0);
        SetDsAcl(V, policerPhbEn_f, p_buffer->action, 0);
        break;
    case CTC_ACL_FIELD_ACTION_COPP:
        if(is_add)
        {
            CTC_NOT_EQUAL_CHECK(data0, 0);
            SYS_USW_ACL_CHECK_ACTION_CONFLICT(pe->action_bmp, CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER, CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER);
            policer_param.dir = CTC_INGRESS;
            CTC_ERROR_RETURN(sys_usw_qos_policer_index_get
            (lchip, data0, SYS_QOS_POLICER_TYPE_COPP, &policer_param));

            if ((policer_param.policer_ptr > 0x1FFF) || (policer_param.policer_ptr == 0))
            {
                return CTC_E_INVALID_PARAM;
            }

            if (policer_param.is_bwp)
            {
                return CTC_E_INVALID_PARAM;
            }
            pe->policer_id = data0;
        }
        SetDsAcl(V, policerLvlSel_f, p_buffer->action, 0);
        SetDsAcl(V, policerPtr_f, p_buffer->action, is_add ? (policer_param.policer_ptr | (1 << 12)) : 0);/*TempDsAcl.policerPtr(12)&&(!TempDsAcl.policerLvlSel)*/

        break;
    case CTC_ACL_FIELD_ACTION_PRIORITY:
        CTC_MAX_VALUE_CHECK(action_field->data0, MAX_CTC_QOS_COLOR - 1);
        CTC_MAX_VALUE_CHECK(data1, 0xF);
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        if((CTC_QOS_COLOR_NONE == action_field->data0) || (CTC_QOS_COLOR_RED == action_field->data0))
        {
            SetDsAcl(V, prioRed_f, p_buffer->action, data1);
            SetDsAcl(V, prioRedValid_f, p_buffer->action, is_add?1:0);
        }
        if((CTC_QOS_COLOR_NONE == action_field->data0) || (CTC_QOS_COLOR_YELLOW == action_field->data0))
        {
            SetDsAcl(V, prioYellow_f, p_buffer->action, data1);
            SetDsAcl(V, prioYellowValid_f, p_buffer->action, is_add?1:0);
        }
        if((CTC_QOS_COLOR_NONE == action_field->data0) || (CTC_QOS_COLOR_GREEN == action_field->data0))
        {
            SetDsAcl(V, prioGreen_f, p_buffer->action, data1);
            SetDsAcl(V, prioGreenValid_f, p_buffer->action, is_add?1:0);
        }
        break;
    case CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH:
        if(is_half  || !DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_replaceLoadBalanceHash_f))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        if (is_add)
        {
            ctc_acl_lb_hash_t   *p_data = action_field->ext_data;
            uint8 lb_hash_mode = 0;
            /*Parameter check*/
            CTC_PTR_VALID_CHECK(p_data);
            CTC_MAX_VALUE_CHECK(p_data->mode, 2);

            /*Get dsfwd and change u1_type g2 to g1*/
            CTC_ERROR_RETURN(pe->u1_type == SYS_AD_UNION_G_2? sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwdptr, 0): CTC_E_NONE);
            _sys_usw_acl_action_set_dsfwd(lchip, pe, fwdptr, temp_u1_type);

            CTC_UNSET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);

            switch(p_data->mode)
            {
                case 0:
                    lb_hash_mode = 1;
                    break;
                case 1:
                    lb_hash_mode = 2;
                    break;
                default:
                    lb_hash_mode = 3;
                    break;
            }
            SetDsAcl(V, replaceLoadBalanceHash_f, p_buffer->action, lb_hash_mode);
            SetDsAcl(V, u1_g1_loadBalanceHashValue_f, p_buffer->action, p_data->lb_hash);
        }
        else
        {
            SetDsAcl(V, replaceLoadBalanceHash_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g1_loadBalanceHashValue_f, p_buffer->action, 0);
        }

        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsAcl(V, terminateCidHdr_f, p_buffer->action, is_add?1:0);
        break;
    case CTC_ACL_FIELD_ACTION_INTER_CN:
        if (is_add)
        {
            p_inter_cn = (ctc_acl_inter_cn_t*)(action_field->ext_data);
            /*Parameter check*/
            CTC_PTR_VALID_CHECK(p_inter_cn);
            CTC_MAX_VALUE_CHECK(p_inter_cn->inter_cn, CTC_QOS_INTER_CN_MAX - 1);

            /*Get dsfwd and change u1_type g2 to g1*/
            CTC_ERROR_RETURN(pe->u1_type == SYS_AD_UNION_G_2? sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwdptr, 0): CTC_E_NONE);
            _sys_usw_acl_action_set_dsfwd(lchip, pe, fwdptr, temp_u1_type);

            CTC_UNSET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);

            SetDsAcl(V, u1_g1_cnActionMode_f, p_buffer->action, 1);
            SetDsAcl(V, u1_g1_cnAction_f, p_buffer->action, p_inter_cn->inter_cn);
        }
        else
        {
            SetDsAcl(V, u1_g1_cnActionMode_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g1_cnAction_f, p_buffer->action, 0);
        }

        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);

        break;
    case CTC_ACL_FIELD_ACTION_TRUNCATED_LEN:
        CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
        if(service_queue_egress_en)
        {
            return CTC_E_NOT_SUPPORT;
        }
        old_profile_id = GetDsAcl(V, u1_g1_truncateLenProfId_f, p_buffer->action);

        if (is_add)
        {
            int32 ret = CTC_E_NONE;
            CTC_ERROR_RETURN(sys_usw_register_add_truncation_profile(lchip, data0, old_profile_id, &profile_id));
            ret = pe->u1_type == SYS_AD_UNION_G_2? sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwdptr, 0): CTC_E_NONE;
            if (ret != CTC_E_NONE)
            {
                sys_usw_register_remove_truncation_profile(lchip, 1, profile_id);
                return ret;
            }

            _sys_usw_acl_action_set_dsfwd(lchip, pe, fwdptr, temp_u1_type);
            CTC_UNSET_FLAG(pe->action_flag, SYS_ACL_ACTION_FLAG_BIND_NH);
            SetDsAcl(V, u1_g1_truncateLenProfId_f, p_buffer->action, profile_id);
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_register_remove_truncation_profile(lchip, 1, old_profile_id));
            SetDsAcl(V, u1_g1_truncateLenProfId_f, p_buffer->action, 0);
        }

        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_SRC_CID:
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
		if(is_add)
		{
            SYS_USW_CID_CHECK(lchip,data0);
		}
		if(data0 == 0)
		{/*disable cid*/
            is_add = 0;
		}
         /*g1:gI2eSrcCid  g2-gIpfix; g3:gMicroflow,g4:gStatsOp*/
        SYS_CHECK_UNION_BITMAP(pe->u2_type,SYS_AD_UNION_G_1);
        SetDsAcl(V, u2Type_f, p_buffer->action,is_add? 3 :0);
        SetDsAcl(V, u2_gI2eSrcCid_i2eSrcCid_f, p_buffer->action, is_add?data0:0);
        SetDsAcl(V, u2_gI2eSrcCid_i2eSrcCidValid_f, p_buffer->action,is_add? 1 :0);
        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_1,DsAcl,2,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_STATS:
        if(is_add)
        {
            sys_stats_statsid_t stats;
            uint8  temp_priority;
            uint8  block_id = 0;
            uint8  check_pass = 0;
            uint32 lkup_level_bitmap;

            block_id = (CTC_INGRESS == pe->group->group_info.dir ? pe->group->group_info.block_id: \
                                                    pe->group->group_info.block_id+ACL_IGS_BLOCK_MAX_NUM);
            lkup_level_bitmap = p_usw_acl_master[lchip]->league[block_id].lkup_level_bitmap;

            sal_memset(&stats, 0, sizeof(stats));
            stats.stats_id = data0;
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsid(lchip, &stats));
            if(stats.dir != pe->group->group_info.dir ||
               stats.stats_id_type != SYS_STATS_TYPE_ACL )
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "stats id is not valid, line:%d\n", __LINE__);
                return CTC_E_INVALID_PARAM;
            }
            for(temp_priority=0; temp_priority < ACL_IGS_BLOCK_MAX_NUM; temp_priority++)
            {
                if(CTC_IS_BIT_SET(lkup_level_bitmap, temp_priority) && temp_priority == stats.u.acl_priority)
                {
                    check_pass = 1;
                    break;
                }
            }
            if(!check_pass)
            {
                SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "stats id is not valid, line:%d\n", __LINE__);
                return CTC_E_INVALID_PARAM;
            }
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, data0, &stats_ptr));
            pe->stats_id = data0;
        }
        SYS_CHECK_UNION_BITMAP(pe->u2_type,SYS_AD_UNION_G_4);
        SetDsAcl(V, u2Type_f, p_buffer->action, 0);
        SetDsAcl(V, u2_gStatsOp_statsPtr_f, p_buffer->action, stats_ptr);
        SetDsAcl(V, u2_gStatsOp_aclStatsIsPrivate_f, p_buffer->action, SYS_STATS_IS_ACL_PRIVATE(stats_ptr) ? 1 : 0);
        SetDsAcl(V, u2_gStatsOp_statsColorAware_f, p_buffer->action, sys_usw_flow_stats_is_color_aware_en(lchip, data0));
        SYS_SET_UNION_TYPE(pe->u2_type,SYS_AD_UNION_G_4,DsAcl,2,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_VLAN_EDIT:
        if(is_half || (DRV_IS_DUET2(lchip) && pe->group->group_info.dir == CTC_EGRESS))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
        }
        /*u4:gStripOp:g1;  gSvlanAction:g2*/
        /*u3:gCvlanAction:g1;  gMetadata:g2; gLbOperation:g3*/

        SYS_CHECK_UNION_BITMAP(pe->u4_type,SYS_AD_UNION_G_2);
        SYS_CHECK_UNION_BITMAP(pe->u3_type,SYS_AD_UNION_G_1);
        if (CTC_EGRESS == pe->group->group_info.dir)
        {
            if (is_add)
            {
                CTC_PTR_VALID_CHECK(action_field->ext_data);
                p_ctc_vlan_edit = (ctc_acl_vlan_edit_t*)(action_field->ext_data);
                CTC_ERROR_RETURN(_sys_usw_acl_check_vlan_edit(lchip, p_ctc_vlan_edit, &do_vlan_edit));
                if (do_vlan_edit)
                {
                    vlan_edit.stag_op = p_ctc_vlan_edit->stag_op;
                    vlan_edit.ctag_op = p_ctc_vlan_edit->ctag_op;
                    vlan_edit.svid_sl = p_ctc_vlan_edit->svid_sl;
                    vlan_edit.scos_sl = p_ctc_vlan_edit->scos_sl;
                    vlan_edit.scfi_sl = p_ctc_vlan_edit->scfi_sl;
                    vlan_edit.cvid_sl = p_ctc_vlan_edit->cvid_sl;
                    vlan_edit.ccos_sl = p_ctc_vlan_edit->ccos_sl;
                    vlan_edit.ccfi_sl = p_ctc_vlan_edit->ccfi_sl;
                    vlan_edit.tpid_index = p_ctc_vlan_edit->tpid_index;
                }
            }
            _sys_usw_acl_build_egress_vlan_edit(lchip, p_buffer->action, &vlan_edit, is_add);
            if ((CTC_ACL_VLAN_TAG_OP_NONE != vlan_edit.stag_op) || !is_add)
            {
                SetDsAcl(V, u4Type_f, p_buffer->action, 0);
                SetDsAcl(V, u4_gSvlanAction_svlanId_f, p_buffer->action, is_add? p_ctc_vlan_edit->svid_new: 0);
                SetDsAcl(V, u4_gSvlanAction_scos_f, p_buffer->action, is_add? p_ctc_vlan_edit->scos_new: 0);
                SetDsAcl(V, u4_gSvlanAction_scfi_f, p_buffer->action, is_add? p_ctc_vlan_edit->scfi_new: 0);
            }
            if ((CTC_ACL_VLAN_TAG_OP_NONE != vlan_edit.ctag_op) || !is_add)
            {
                SetDsAcl(V, u3Type_f, p_buffer->action, 0);
                SetDsAcl(V, u3_gCvlanAction_cvlanId_f, p_buffer->action, is_add? p_ctc_vlan_edit->cvid_new: 0);
                SetDsAcl(V, u3_gCvlanAction_ccos_f, p_buffer->action, is_add? p_ctc_vlan_edit->ccos_new: 0);
                SetDsAcl(V, u3_gCvlanAction_ccfi_f, p_buffer->action, is_add? p_ctc_vlan_edit->ccfi_new: 0);
            }
            SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_2,DsAcl,4,p_buffer->action, is_add);
            SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_1,DsAcl,3,p_buffer->action, is_add);
            break;      /*egress vlan edit end*/
        }
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_ctc_vlan_edit = (ctc_acl_vlan_edit_t*)(action_field->ext_data);
            CTC_ERROR_RETURN(_sys_usw_acl_check_vlan_edit(lchip, p_ctc_vlan_edit, &do_vlan_edit));
            if (do_vlan_edit)
            {
                vlan_edit.stag_op = p_ctc_vlan_edit->stag_op;
                vlan_edit.ctag_op = p_ctc_vlan_edit->ctag_op;
                vlan_edit.svid_sl = p_ctc_vlan_edit->svid_sl;
                vlan_edit.scos_sl = p_ctc_vlan_edit->scos_sl;
                vlan_edit.scfi_sl = p_ctc_vlan_edit->scfi_sl;
                vlan_edit.cvid_sl = p_ctc_vlan_edit->cvid_sl;
                vlan_edit.ccos_sl = p_ctc_vlan_edit->ccos_sl;
                vlan_edit.ccfi_sl = p_ctc_vlan_edit->ccfi_sl;
                vlan_edit.tpid_index = p_ctc_vlan_edit->tpid_index;

                CTC_ERROR_RETURN(ctc_spool_add(p_usw_acl_master[lchip]->vlan_edit_spool, &vlan_edit, pe->vlan_edit, &pv_get));
                pe->vlan_edit       = pv_get;
                SetDsAcl(V, vlanActionProfileId_f, p_buffer->action, pv_get->profile_id);

                pe->vlan_edit_valid = 1;

                if(CTC_ACL_VLAN_TAG_OP_NONE != vlan_edit.stag_op)
                {
                    SetDsAcl(V, u4Type_f, p_buffer->action, 0);
                    SetDsAcl(V, u4_gSvlanAction_svlanId_f, p_buffer->action, p_ctc_vlan_edit->svid_new);
                    SetDsAcl(V, u4_gSvlanAction_scos_f, p_buffer->action, p_ctc_vlan_edit->scos_new);
                    SetDsAcl(V, u4_gSvlanAction_scfi_f, p_buffer->action, p_ctc_vlan_edit->scfi_new);
                }
                if(CTC_ACL_VLAN_TAG_OP_NONE != vlan_edit.ctag_op)
                {
                    SetDsAcl(V, u3Type_f, p_buffer->action, 0);
                    SetDsAcl(V, u3_gCvlanAction_cvlanId_f, p_buffer->action, p_ctc_vlan_edit->cvid_new);
                    SetDsAcl(V, u3_gCvlanAction_ccos_f, p_buffer->action, p_ctc_vlan_edit->ccos_new);
                    SetDsAcl(V, u3_gCvlanAction_ccfi_f, p_buffer->action, p_ctc_vlan_edit->ccfi_new);
                }
            }
        }
        else
        {
            if(pe->vlan_edit_valid)
            {
                CTC_ERROR_RETURN(ctc_spool_remove(p_usw_acl_master[lchip]->vlan_edit_spool, pe->vlan_edit, NULL));
                pe->vlan_edit_valid = 0;
                pe->vlan_edit = NULL;
            }
            SetDsAcl(V, vlanActionProfileId_f, p_buffer->action, 0);
        }
        SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_2,DsAcl,4,p_buffer->action, is_add);

        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_1,DsAcl,3,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_METADATA:
         /*u3:gCvlanAction:g1;  gMetadata:g2; gLbOperation:g3*/
        if(is_half || CTC_EGRESS == pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type,SYS_AD_UNION_G_2);
        if(is_add)
        {
            if(data0)
            {
                SetDsAcl(V, u3_gMetadata_metadata_f, p_buffer->action, data0);
                SetDsAcl(V, u3_gMetadata_resetMetadata_f, p_buffer->action, 0);
            }
            else
            {
                SetDsAcl(V, u3_gMetadata_metadata_f, p_buffer->action, 0);
                SetDsAcl(V, u3_gMetadata_resetMetadata_f, p_buffer->action, 1);
            }
        }
        else
        {
            SetDsAcl(V, u3_gMetadata_metadata_f, p_buffer->action, 0);
            SetDsAcl(V, u3_gMetadata_resetMetadata_f, p_buffer->action, 0);
        }
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, pe->u3_type ? 1:0);
        break;
    case CTC_ACL_FIELD_ACTION_SPAN_FLOW:
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type,SYS_AD_UNION_G_2);
        SetDsAcl(V, u3_gMetadata_isSpanPkt_f, p_buffer->action, is_add?1:0);
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_2,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, pe->u3_type? 1: 0);
        break;
    case CTC_ACL_FIELD_ACTION_STRIP_PACKET:
        if(is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

        }
        if(CTC_BMP_ISSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT))
        {
            /* can not config at the same time */
            return CTC_E_INVALID_CONFIG;
        }
        SYS_CHECK_UNION_BITMAP(pe->u4_type,SYS_AD_UNION_G_1);
        if(is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            pkt_strip = (ctc_acl_packet_strip_t*)(action_field->ext_data);
            CTC_MAX_VALUE_CHECK(pkt_strip->start_packet_strip, CTC_ACL_STRIP_MAX - 1);
            CTC_MAX_VALUE_CHECK(pkt_strip->packet_type, PKT_TYPE_RESERVED - 1);
            CTC_MAX_VALUE_CHECK(pkt_strip->strip_extra_len, MCHIP_CAP(SYS_CAP_PKT_STRIP_PKT_LEN)-1);
            SetDsAcl(V, u4Type_f, p_buffer->action, 1);
            SetDsAcl(V, u4_gStripOp_packetType_f, p_buffer->action, pkt_strip->packet_type);
            SetDsAcl(V, u4_gStripOp_payloadOffsetStartPoint_f, p_buffer->action, pkt_strip->start_packet_strip);
            SetDsAcl(V, u4_gStripOp_tunnelPayloadOffset_f, p_buffer->action, pkt_strip->strip_extra_len);
        }
        else
        {
            SetDsAcl(V, u4Type_f, p_buffer->action, 0);
            SetDsAcl(V, u4_gStripOp_packetType_f, p_buffer->action, 0);
            SetDsAcl(V, u4_gStripOp_payloadOffsetStartPoint_f, p_buffer->action, 0);
            SetDsAcl(V, u4_gStripOp_tunnelPayloadOffset_f, p_buffer->action, 0);
        }
        SYS_SET_UNION_TYPE(pe->u4_type,SYS_AD_UNION_G_1,DsAcl,4,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_RANDOM_LOG:
        SYS_CHECK_UNION_BITMAP(pe->u5_type,SYS_AD_UNION_G_1);
        if(is_add)
        {
            CTC_VALUE_RANGE_CHECK(data1,
                              CTC_LOG_PERCENT_POWER_NEGATIVE_14, CTC_LOG_PERCENT_POWER_NEGATIVE_0);
            CTC_MAX_VALUE_CHECK(data0, MCHIP_CAP(SYS_CAP_ACL_MAX_SESSION) - 1);
            pe->action_flag |= SYS_ACL_ACTION_FLAG_LOG;
        }
        else
        {
           pe->action_flag &= (~SYS_ACL_ACTION_FLAG_LOG);
        }
        SetDsAcl(V, u5_g1_randomThresholdShift_f, p_buffer->action, data1);
        SetDsAcl(V, u5_g1_aclLogId_f, p_buffer->action, data0);
        SYS_SET_UNION_TYPE(pe->u5_type,SYS_AD_UNION_G_1,DsAcl,5,p_buffer->action, is_add);
        break;
    case CTC_ACL_FIELD_ACTION_CRITICAL_PKT:
        if (is_half && !DRV_IS_DUET2(lchip))
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        if (DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_aclCriticalPacket_f))
        {
            SetDsAcl(V, aclCriticalPacket_f, p_buffer->action, is_add?1:0);
        }
        else
        {
            SetDsAcl(V, aclCriticalPacketSet_f, p_buffer->action, is_add?1:0);
        }
        break;
    case CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP:
        if (is_half)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        if (is_add)
        {
            CTC_PTR_VALID_CHECK(action_field->ext_data);
            p_table_map = (ctc_acl_table_map_t*)(action_field->ext_data);
            CTC_VALUE_RANGE_CHECK(p_table_map->table_map_id, 1, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
            CTC_MAX_VALUE_CHECK(p_table_map->table_map_type, CTC_QOS_TABLE_MAP_TYPE_MAX - 1);
            SetDsAcl(V, mutationTableId_f, p_buffer->action, p_table_map->table_map_id);
            SetDsAcl(V, mutationTableType_f, p_buffer->action, p_table_map->table_map_type);
        }
        else
        {
            SetDsAcl(V, mutationTableId_f, p_buffer->action, 0);
            SetDsAcl(V, mutationTableType_f, p_buffer->action, 0);
        }

        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR :
        if (!DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_u3_gLbOperation_disableEcmpRr_f) || CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_3);
        SetDsAcl(V, u3_gLbOperation_disableEcmpRr_f, p_buffer->action, (is_add? 1: 0));
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_3,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, (is_add? 2: (pe->u3_type ? 2:0)));
        break;
    case CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR :
        if (!DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_u3_gLbOperation_disablePortLagRr_f) || CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_3);
        SetDsAcl(V, u3_gLbOperation_disablePortLagRr_f, p_buffer->action, (is_add? 1: 0));
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_3,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, (is_add? 2: (pe->u3_type ? 2:0)));
        break;
    case CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE :
        if (!DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_u3_gLbOperation_newPortBasedHashProfileId0_f) || CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_3);
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsAcl(V, u3_gLbOperation_modifyPortBasedHashProfile0_f, p_buffer->action, (is_add? 1: 0));
        SetDsAcl(V, u3_gLbOperation_newPortBasedHashProfileId0_f, p_buffer->action, data0);
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_3,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, (is_add? 2: (pe->u3_type ? 2:0)));
        break;
    case CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE :
        if (!DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_u3_gLbOperation_newPortBasedHashProfileId1_f) || CTC_EGRESS == pe->group->group_info.dir)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_CHECK_UNION_BITMAP(pe->u3_type, SYS_AD_UNION_G_3);
        CTC_MAX_VALUE_CHECK(data0, 0x3F);
        SetDsAcl(V, u3_gLbOperation_modifyPortBasedHashProfile1_f, p_buffer->action, (is_add? 1: 0));
        SetDsAcl(V, u3_gLbOperation_newPortBasedHashProfileId1_f, p_buffer->action, data0);
        SYS_SET_UNION_TYPE(pe->u3_type,SYS_AD_UNION_G_3,DsAcl,3,p_buffer->action, is_add);
        SetDsAcl(V, u3Type_f, p_buffer->action, (is_add? 2: (pe->u3_type ? 2:0)));
        break;
    case CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT :
        if (DRV_IS_DUET2(lchip) || (CTC_INGRESS == pe->group->group_info.dir))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_CHECK_UNION_BITMAP(pe->u1_type, SYS_AD_UNION_G_2);
        p_vxlan_rsv_edit = (ctc_acl_vxlan_rsv_edit_t*)action_field->ext_data;
        if (is_add)
        {
            CTC_PTR_VALID_CHECK(p_vxlan_rsv_edit);
            CTC_MAX_VALUE_CHECK(p_vxlan_rsv_edit->edit_mode, 0x7);
            CTC_MAX_VALUE_CHECK(p_vxlan_rsv_edit->vxlan_rsv1, 0xFFFFFF);
            SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, (p_vxlan_rsv_edit->edit_mode& 0x4? 1: 0));
            SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, (p_vxlan_rsv_edit->edit_mode & 0x2? 1: 0));
            SetDsAcl(V, u1_g2_adBypassIngressEdit_f, p_buffer->action, (p_vxlan_rsv_edit->edit_mode & 0x1? 1: 0));
            SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, ((p_vxlan_rsv_edit->vxlan_flags>>5) & 0x7));
            SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, (((p_vxlan_rsv_edit->vxlan_flags & 0x1F)<<14)+((p_vxlan_rsv_edit->vxlan_rsv1 >>10) & 0x3FFF)));
            SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, (((p_vxlan_rsv_edit->vxlan_rsv1 & 0x3FF)<<8) + p_vxlan_rsv_edit->vxlan_rsv2));
            SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 1);
        }
        else
        {
            SetDsAcl(V, u1_g2_adNextHopExt_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g2_adBypassIngressEdit_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g2_adLengthAdjustType_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g2_adDestMap_f, p_buffer->action, 0);
            SetDsAcl(V, u1_g2_adNextHopPtr_f, p_buffer->action, 0);
            SetDsAcl(V, nextHopPtrValid_f, p_buffer->action, 0);
        }
        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsAcl,1,p_buffer->action, is_add);
        break;
    default:
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    if((CTC_ACL_FIELD_ACTION_PRIORITY != action_field->type) && (CTC_ACL_FIELD_ACTION_DISCARD != action_field->type) && (CTC_ACL_FIELD_ACTION_CANCEL_DISCARD != action_field->type))
    {
        SYS_USW_ACL_SET_BMP(pe->action_bmp, action_field->type, is_add);
    }

    if(((CTC_ACL_FIELD_ACTION_PRIORITY == action_field->type) || (CTC_ACL_FIELD_ACTION_DISCARD == action_field->type) || (CTC_ACL_FIELD_ACTION_CANCEL_DISCARD == action_field->type)))
    {
        if (is_add)
        {
            SYS_USW_ACL_SET_BMP(pe->action_bmp, action_field->type, is_add);
        }
        else
        {
            if((CTC_ACL_FIELD_ACTION_PRIORITY == action_field->type)
                && (0 == GetDsAcl(V, prioRedValid_f, p_buffer->action)) && (0 == GetDsAcl(V, prioYellowValid_f, p_buffer->action)) && (0 == GetDsAcl(V, prioGreenValid_f, p_buffer->action)))
            {
                CTC_BMP_UNSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_PRIORITY);
            }

            if((CTC_ACL_FIELD_ACTION_DISCARD == action_field->type)
                && (0 == GetDsAcl(V, discardOpTypeRed_f, p_buffer->action)) && (0 == GetDsAcl(V, discardOpTypeYellow_f, p_buffer->action)) && (0 == GetDsAcl(V, discardOpTypeGreen_f, p_buffer->action)))
            {
                CTC_BMP_UNSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_DISCARD);
            }

            if((CTC_ACL_FIELD_ACTION_CANCEL_DISCARD == action_field->type)
                && (0 == GetDsAcl(V, discardOpTypeRed_f, p_buffer->action)) && (0 == GetDsAcl(V, discardOpTypeYellow_f, p_buffer->action)) && (0 == GetDsAcl(V, discardOpTypeGreen_f, p_buffer->action)))
            {
                CTC_BMP_UNSET(pe->action_bmp, CTC_ACL_FIELD_ACTION_CANCEL_DISCARD);
            }
        }
    }
    if(CTC_ACL_FIELD_ACTION_CANCEL_ALL == action_field->type)
    {
        sal_memset(pe->action_bmp, 0, sizeof(pe->action_bmp));
    }
    if(CTC_INGRESS == pe->group->group_info.dir)
    {
        pe->action_type = SYS_ACL_ACTION_TCAM_INGRESS;
    }
    else
    {
        pe->action_type = SYS_ACL_ACTION_TCAM_EGRESS;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_get_field_action_tcam_igs(uint8 lchip, ctc_acl_field_action_t* p_action, sys_acl_entry_t* pe)
{
    void* p_action_buffer = pe->buffer->action;
    uint8 is_half_ad = 0;
    sys_nh_info_dsnh_t nhinfo = {0};
    uint8 temp_data1 = 0;
    uint8  shift = 0;
    uint16 length = 0;
    uint32 cmd = 0;
    uint32 ingress_dot1ae_dis = 0;
    ctc_acl_vxlan_rsv_edit_t* vxlan_rsv_edit = NULL;
    DsTruncationProfile_m truncation_prof;

    if( CTC_ACL_KEY_INTERFACE == pe->key_type )
    {
        is_half_ad = 1;
    }

    cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_dot1AeFieldShareMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ingress_dot1ae_dis));

    switch(p_action->type)
    {
    case CTC_ACL_FIELD_ACTION_TIMESTAMP:
        if (DRV_IS_DUET2(lchip) || CTC_EGRESS != pe->group->group_info.dir)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Not support timestamp \n");
            return CTC_E_NOT_SUPPORT;
        }
        {
            ctc_acl_timestamp_t* timestamp = (ctc_acl_timestamp_t*)p_action->ext_data;
            CTC_PTR_VALID_CHECK(timestamp);
            timestamp->mode = GetDsAcl(V, aclDenyLearning_f, p_action_buffer)<<1;
            timestamp->mode |= GetDsAcl(V, terminateCidHdr_f, p_action_buffer);
        }
        break;
    case CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN:
        if (!ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        p_action->data0 = GetDsAcl(V, dot1AeSpecialCase_f, p_action_buffer);
        break;
    case CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS:
        if (!ingress_dot1ae_dis)
        {
            return CTC_E_NOT_SUPPORT;
        }
        p_action->data0 = GetDsAcl(V, dot1AeEncryptDisable_f, p_action_buffer);
        break;
        case CTC_ACL_FIELD_ACTION_CP_TO_CPU:
            {
                ctc_acl_to_cpu_t* p_copy_to_cpu = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_copy_to_cpu);
                temp_data1 = GetDsAcl(V, exceptionToCpuType_f, p_action_buffer);
                if(1 == temp_data1)
                {
                    p_copy_to_cpu->mode = CTC_ACL_TO_CPU_MODE_TO_CPU_COVER;
                }
                else if(2 == temp_data1)
                {
                    p_copy_to_cpu->mode = CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU;
                }
                else if(3 == temp_data1)
                {
                    p_copy_to_cpu->mode = CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER;
                }
                p_copy_to_cpu->cpu_reason_id = pe->cpu_reason_id;
            }
            break;
        case CTC_ACL_FIELD_ACTION_STATS:
            p_action->data0 = pe->stats_id ;
            break;
        case CTC_ACL_FIELD_ACTION_RANDOM_LOG:
            p_action->data1 = GetDsAcl(V, u5_g1_randomThresholdShift_f, p_action_buffer);
            p_action->data0 = GetDsAcl(V, u5_g1_aclLogId_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_COLOR:
            p_action->data0 = GetDsAcl(V, color_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DSCP:
            if(GetDsAcl(V, dscpValid_f, p_action_buffer))
            {
                p_action->data0 = GetDsAcl(V, dscp_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER:
            p_action->data0 = pe->policer_id;
            p_action->data1 = pe->cos_index;
            break;
        case CTC_ACL_FIELD_ACTION_COPP:
            p_action->data0 = pe->policer_id;
            break;
        case CTC_ACL_FIELD_ACTION_SRC_CID:
            if(GetDsAcl(V, u2_gI2eSrcCid_i2eSrcCidValid_f, p_action_buffer))
            {
                p_action->data0 = GetDsAcl(V, u2_gI2eSrcCid_i2eSrcCid_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_TRUNCATED_LEN:
            temp_data1 = GetDsAcl(V, u1_g1_truncateLenProfId_f, p_action_buffer);
            if (temp_data1)
            {
                sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
                cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, temp_data1, cmd, &truncation_prof));

                shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
                length = GetDsTruncationProfile(V, length_f, &truncation_prof);
                length = length << shift;
            }
            p_action->data0 = length;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT:
            p_action->data0 = pe->nexthop_id;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT_PORT:
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, pe->nexthop_id, &nhinfo, 0));
            p_action->data0 = nhinfo.gport;
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT:
            p_action->data0 = GetDsAcl(V, clearPktEditOperation_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT:
            p_action->data0 = GetDsAcl(V, redirectEnableMode_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DISCARD:
            if (is_half_ad|| p_action->data0 == CTC_QOS_COLOR_GREEN)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeGreen_f, p_action_buffer);
            }
            else  if (p_action->data0 == CTC_QOS_COLOR_YELLOW)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeYellow_f, p_action_buffer);
            }
            else  if (p_action->data0 == CTC_QOS_COLOR_RED)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeRed_f, p_action_buffer);
            }
            p_action->data1  = (p_action->data1  == 1);
            break;

        case CTC_ACL_FIELD_ACTION_CANCEL_DISCARD:
            if (is_half_ad || p_action->data0 == CTC_QOS_COLOR_GREEN)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeGreen_f, p_action_buffer);
            }
            else  if (p_action->data0 == CTC_QOS_COLOR_YELLOW)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeYellow_f, p_action_buffer);
            }
            else  if (p_action->data0 == CTC_QOS_COLOR_RED)
            {
                p_action->data1 = GetDsAcl(V, discardOpTypeRed_f, p_action_buffer);
            }
            p_action->data1  = (p_action->data1  == 2);
            break;
        case CTC_ACL_FIELD_ACTION_PRIORITY:
            temp_data1 = GetDsAcl(V, prioRedValid_f, p_action_buffer);
            p_action->data1 = 0xFFFFFFFF;
            if(p_action->data0 == CTC_QOS_COLOR_GREEN &&  GetDsAcl(V, prioGreenValid_f, p_action_buffer) )
            {
                p_action->data1 = GetDsAcl(V, prioGreen_f, p_action_buffer);
            }
            else if(p_action->data0 == CTC_QOS_COLOR_YELLOW && GetDsAcl(V, prioYellowValid_f, p_action_buffer))
            {
                p_action->data1 = GetDsAcl(V, prioYellow_f, p_action_buffer);
            }
            else if(p_action->data0 == CTC_QOS_COLOR_RED && GetDsAcl(V, prioRedValid_f, p_action_buffer))
            {
                p_action->data1 = GetDsAcl(V, prioRed_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG:
            p_action->data0 = GetDsAcl(V, disableElephantFlowLog_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR:
            p_action->data0 = GetDsAcl(V, terminateCidHdr_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_CANCEL_NAT:
            p_action->data0 = GetDsAcl(V, doNotSnat_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_IPFIX:
            {
                ctc_acl_ipfix_t* p_ipfix = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_ipfix);
                if(1 == GetDsAcl(V, u2Type_f, p_action_buffer))
                {
                    p_ipfix->flow_cfg_id = GetDsAcl(V, u2_gIpfix_ipfixCfgProfileId_f, p_action_buffer);
                    p_ipfix->field_sel_id = GetDsAcl(V, u2_gIpfix_ipfixHashFieldSel_f, p_action_buffer);
                    p_ipfix->hash_type = GetDsAcl(V, u2_gIpfix_ipfixHashType_f, p_action_buffer);
                    p_ipfix->use_mapped_vlan = GetDsAcl(V, u2_gIpfix_ipfixUsePIVlan_f, p_action_buffer);
                    p_ipfix->use_cvlan = GetDsAcl(V, u2_gIpfix_ipfixFlowL2KeyUseCvlan_f, p_action_buffer);
                    p_ipfix->policer_id= 0;
                    p_ipfix->lantency_en = GetDsAcl(V, u2_gIpfix_egrIpfixAdUseMeasureLantency_f, p_action_buffer);
                }
                else if(2 == GetDsAcl(V, u2Type_f, p_action_buffer))
                {
                    p_ipfix->flow_cfg_id = GetDsAcl(V, u2_gMicroflow_microflowCfgProfileId_f, p_action_buffer);
                    p_ipfix->field_sel_id = GetDsAcl(V, u2_gMicroflow_microflowHashFieldSel_f, p_action_buffer);
                    p_ipfix->hash_type = GetDsAcl(V, u2_gMicroflow_microflowHashType_f, p_action_buffer);
                    p_ipfix->use_mapped_vlan = GetDsAcl(V, u2_gMicroflow_microflowUsePIVlan_f, p_action_buffer);
                    p_ipfix->use_cvlan = GetDsAcl(V, u5_g3_microflowL2KeyUseCvlan_f, p_action_buffer);
                    p_ipfix->policer_id = pe->policer_id;
                }
            }
            break;
        case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX:
            p_action->data0 = GetDsAcl(V, ipfixOpCode_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING:
            p_action->data0 = GetDsAcl(V, ipfixOpCode_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_OAM:
            {
                ctc_acl_oam_t* p_oam = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_oam);
                p_oam->dest_gchip = GetDsAcl(V, u1_g3_oamDestChip_f, p_action_buffer);
                p_oam->lmep_index = GetDsAcl(V, u1_g3_mepIndex_f, p_action_buffer);
                p_oam->mip_en = GetDsAcl(V, u1_g3_mipEn_f, p_action_buffer);
                p_oam->oam_type = GetDsAcl(V, u1_g3_rxOamType_f, p_action_buffer);
                p_oam->link_oam_en = GetDsAcl(V, u1_g3_linkOamEn_f, p_action_buffer);
                p_oam->packet_offset = GetDsAcl(V, u1_g3_packetOffset_f, p_action_buffer);
                p_oam->mac_da_check_en = GetDsAcl(V, u1_g3_macDaChkEn_f, p_action_buffer);
                p_oam->is_self_address = GetDsAcl(V, u1_g3_selfAddress_f, p_action_buffer);
                p_oam->time_stamp_en = GetDsAcl(V, u1_g3_timestampEn_f, p_action_buffer);
                p_oam->timestamp_format = GetDsAcl(V, u1_g3_twampTsType_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH:
            {
                ctc_acl_lb_hash_t* p_lb_hash = p_action->ext_data;
                if (!DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_replaceLoadBalanceHash_f))
                {
                    return CTC_E_NOT_SUPPORT;
                }
                CTC_PTR_VALID_CHECK(p_lb_hash);
                p_lb_hash->lb_hash = GetDsAcl(V, u1_g1_loadBalanceHashValue_f, p_action_buffer);
                p_lb_hash->mode = GetDsAcl(V, replaceLoadBalanceHash_f, p_action_buffer)-1;
            }
            break;
        case CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE:
            p_action->data0 = GetDsAcl(V, dot1AeEncryptDisable_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT:
            p_action->data0 = GetDsAcl(V, dot1AeSpecialCase_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_METADATA:
            p_action->data0 = GetDsAcl(V, u3_gMetadata_metadata_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_INTER_CN:
            {
                ctc_acl_inter_cn_t* p_inter_cn = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_inter_cn);
                p_inter_cn->inter_cn = GetDsAcl(V, u1_g1_cnAction_f, p_action_buffer);
            }
            break;

        case CTC_ACL_FIELD_ACTION_VLAN_EDIT:
            {
                ctc_acl_vlan_edit_t* p_vlan_edit = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_vlan_edit);

                if (CTC_INGRESS == pe->group->group_info.dir)
                {
                    p_vlan_edit->stag_op = pe->vlan_edit->stag_op;
                    p_vlan_edit->svid_sl = pe->vlan_edit->svid_sl;
                    p_vlan_edit->scos_sl = pe->vlan_edit->scos_sl;
                    p_vlan_edit->scfi_sl = pe->vlan_edit->scfi_sl;

                    p_vlan_edit->ctag_op = pe->vlan_edit->ctag_op;
                    p_vlan_edit->cvid_sl = pe->vlan_edit->cvid_sl;
                    p_vlan_edit->ccos_sl = pe->vlan_edit->ccos_sl;
                    p_vlan_edit->ccfi_sl = pe->vlan_edit->ccfi_sl;
                    p_vlan_edit->tpid_index = pe->vlan_edit->tpid_index;
                }
                else if (CTC_EGRESS == pe->group->group_info.dir)
                {
                    _sys_usw_acl_get_egress_vlan_edit(lchip, (void*)p_action_buffer, p_vlan_edit);
                }

                /* refer to ctc_vlan_tag_sl_t, only CTC_VLAN_TAG_SL_NEW need to write hw table */
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->ccfi_sl)
                {
                    p_vlan_edit->ccfi_new = GetDsAcl(V, u3_gCvlanAction_ccfi_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->ccos_sl)
                {
                    p_vlan_edit->ccos_new = GetDsAcl(V, u3_gCvlanAction_ccos_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->cvid_sl)
                {
                    p_vlan_edit->cvid_new = GetDsAcl(V, u3_gCvlanAction_cvlanId_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->scfi_sl)
                {
                    p_vlan_edit->scfi_new = GetDsAcl(V, u4_gSvlanAction_scfi_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->scos_sl)
                {
                    p_vlan_edit->scos_new = GetDsAcl(V, u4_gSvlanAction_scos_f, p_action_buffer);
                }
                if (CTC_VLAN_TAG_SL_NEW == p_vlan_edit->svid_sl)
                {
                    p_vlan_edit->svid_new = GetDsAcl(V, u4_gSvlanAction_svlanId_f, p_action_buffer);
                }
            }
            break;

        case CTC_ACL_FIELD_ACTION_STRIP_PACKET:
            {
                ctc_acl_packet_strip_t* p_strip = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_strip);
                p_strip->start_packet_strip = GetDsAcl(V, u4_gStripOp_payloadOffsetStartPoint_f, p_action_buffer);
                p_strip->packet_type = GetDsAcl(V, u4_gStripOp_packetType_f, p_action_buffer);
                p_strip->strip_extra_len = GetDsAcl(V, u4_gStripOp_tunnelPayloadOffset_f, p_action_buffer);
            }
            break;
        case CTC_ACL_FIELD_ACTION_CRITICAL_PKT:
            if (DRV_FLD_IS_EXISIT(DsAcl_t, DsAcl_aclCriticalPacket_f))
            {
                p_action->data0 = GetDsAcl(V, aclCriticalPacket_f, p_action_buffer);
            }
            else
            {
                p_action->data0 = GetDsAcl(V, aclCriticalPacketSet_f, p_action_buffer);
            }
            break;
     /*    case CTC_ACL_FIELD_ACTION_LOGIC_PORT: only hash have*/
     /*        break;*/
        case CTC_ACL_FIELD_ACTION_SPAN_FLOW:
            p_action->data0 = GetDsAcl(V, u3_gMetadata_isSpanPkt_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_CANCEL_ALL:
            break;
        case CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP:
            {
                ctc_acl_table_map_t* p_table_map = p_action->ext_data;
                CTC_PTR_VALID_CHECK(p_table_map);
                p_table_map->table_map_id = GetDsAcl(V, mutationTableId_f, p_action_buffer);
                p_table_map->table_map_type = GetDsAcl(V, mutationTableType_f, p_action_buffer);
            }
            break;
   /*      case CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN:  //hash*/
   /*          break;*/
   /*      case CTC_ACL_FIELD_ACTION_DENY_BRIDGE:  //hash*/
   /*          break;*/
        case CTC_ACL_FIELD_ACTION_DENY_LEARNING:
            p_action->data0 = GetDsAcl(V, aclDenyLearning_f, p_action_buffer);
            break;
   /*      case CTC_ACL_FIELD_ACTION_DENY_ROUTE:  //hash*/
   /*          break;*/
   /*      case CTC_ACL_FIELD_ACTION_DEST_CID:      //hash*/
   /*           break;*/
        case CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR :
            p_action->data0 = GetDsAcl(V, u3_gLbOperation_disableEcmpRr_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR :
            p_action->data0 = GetDsAcl(V, u3_gLbOperation_disablePortLagRr_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE :
            p_action->data0 = GetDsAcl(V, u3_gLbOperation_modifyPortBasedHashProfile0_f, p_action_buffer);
            p_action->data1 = GetDsAcl(V, u3_gLbOperation_newPortBasedHashProfileId0_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE :
            p_action->data0 = GetDsAcl(V, u3_gLbOperation_modifyPortBasedHashProfile1_f, p_action_buffer);
            p_action->data1 = GetDsAcl(V, u3_gLbOperation_newPortBasedHashProfileId1_f, p_action_buffer);
            break;
        case CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT :
            vxlan_rsv_edit = (ctc_acl_vxlan_rsv_edit_t*)p_action->ext_data;
            temp_data1 = 0;
            temp_data1 = GetDsAcl(V, u1_g2_adNextHopExt_f, p_action_buffer);
            temp_data1 = (temp_data1<<1) | GetDsAcl(V, u1_g2_ecmpTunnelMode_f, p_action_buffer);
            temp_data1 = (temp_data1<<1) | GetDsAcl(V, u1_g2_adBypassIngressEdit_f, p_action_buffer);
            vxlan_rsv_edit->edit_mode = temp_data1;
            vxlan_rsv_edit->vxlan_flags = ((GetDsAcl(V, u1_g2_adLengthAdjustType_f, p_action_buffer)<<5) & 0xE0) + ((GetDsAcl(V, u1_g2_adDestMap_f, p_action_buffer)>>14) & 0x1F);
            vxlan_rsv_edit->vxlan_rsv1 = ((GetDsAcl(V, u1_g2_adDestMap_f, p_action_buffer) & 0x3FFF)<<10) + ((GetDsAcl(V, u1_g2_adNextHopPtr_f, p_action_buffer)>>8) & 0x3FF);
            vxlan_rsv_edit->vxlan_rsv2 = GetDsAcl(V, u1_g2_adNextHopPtr_f, p_action_buffer) & 0xFF;
            break;
        default:
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_update_action(uint8 lchip, void* data, void* change_nh_param)
{
    sys_acl_entry_t* pe = (sys_acl_entry_t*)data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    ds_t   action;
    uint32 cmdr  = 0;
    uint32 cmdw  = 0;
    uint32 fwd_offset = 0;
    uint32 key_id = 0;
    uint32 act_id = 0;
    uint32 ad_index = 0;
    uint32 hw_index = 0;
    uint8 step = 0;
    uint8 adjust_len_idx = 0;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&action, 0, sizeof(ds_t));

    _sys_usw_acl_get_table_id(lchip, pe, &key_id, &act_id, &hw_index);

    if (!ACL_ENTRY_IS_TCAM(pe->key_type))
    {
        ad_index = pe->ad_index;
    }
    else
    {
        _sys_usw_acl_get_key_size(lchip, 1, pe, &step);
        ad_index = SYS_ACL_MAP_DRV_AD_INDEX(hw_index, step);
    }

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"key_tbl_id:[%u]  ad_tbl_id:[%u]  ad_index:[%u]\n", key_id, act_id, ad_index);

    cmdr = DRV_IOR(act_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(act_id, DRV_ENTRY_FLAG);

    if (!pe->buffer)
    {
        DRV_IOCTL(lchip, ad_index, cmdr, &action);
    }
    else
    {
        sal_memcpy(&action, &pe->buffer->action, sizeof(action));
    }

    if (!change_nh_param || p_dsnh_info->cloud_sec_en)
    {
        if(GetDsAcl(V, nextHopPtrValid_f, &action) == 0)
        {
             return CTC_E_NONE;
        }

        if (pe->u1_type != SYS_AD_UNION_G_2)
        {
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"nh already in use, cannot update to dsfwd! \n");
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Already in used \n");
			return CTC_E_IN_USE;

        }

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, pe->nexthop_id, &fwd_offset, 1));
        /*G2->G1*/
        SetDsAcl(V, u1_g2_adApsBridgeEn_f, &action, 0);
        SetDsAcl(V, u1_g2_adDestMap_f, &action, 0);
        SetDsAcl(V, u1_g2_adNextHopExt_f, &action, 0);
        SetDsAcl(V, u1_g2_adNextHopPtr_f, &action, 0);
        SetDsAcl(V, u1_g2_adLengthAdjustType_f, &action, 0);
        SetDsAcl(V, u1_g2_ecmpTunnelMode_f, &action, 0);
        SetDsAcl(V, u1_g2_adBypassIngressEdit_f, &action, 0);
        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_2,DsAcl,1,&action, 0);
        SYS_SET_UNION_TYPE(pe->u1_type,SYS_AD_UNION_G_1,DsAcl,1,&action, 1);
        /*2. update ad*/
        SetDsAcl(V, nextHopPtrValid_f, &action, 0);
        SetDsAcl(V, u1_g1_ecmpGroupId_f, &action, 0);
        SetDsAcl(V, u1_g1_loadBalanceHashValue_f, &action, 0);
        SetDsAcl(V, u1_g1_cnActionMode_f, &action, 0);
        SetDsAcl(V, u1_g1_cnAction_f, &action, 0);
        SetDsAcl(V, u1_g1_truncateLenProfId_f, &action, 0);
        SetDsAcl(V, u1_g1_dsFwdPtr_f, &action, fwd_offset);
    }
    else
    {
        if(GetDsAcl(V, nextHopPtrValid_f, &action) == 0)
        {
             return CTC_E_NONE;
        }
        SetDsAcl(V, u1_g2_adDestMap_f, &action, p_dsnh_info->dest_map);
        SetDsAcl(V, u1_g2_adNextHopPtr_f, &action, p_dsnh_info->dsnh_offset);
        SetDsAcl(V, u1_g2_adApsBridgeEn_f, &action, p_dsnh_info->aps_en);
        SetDsAcl(V, u1_g2_adNextHopExt_f, &action, p_dsnh_info->nexthop_ext);
        if(0 != p_dsnh_info->adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
            SetDsAcl(V, u1_g2_adLengthAdjustType_f, &action, adjust_len_idx);
        }
        else
        {
            SetDsAcl(V, u1_g2_adLengthAdjustType_f, &action, 0);
        }
        if (!DRV_IS_DUET2(lchip) && (p_dsnh_info->dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG))
        {
            SetDsAcl(V, u1_g2_adNextHopPtr_f, &action, (p_dsnh_info->dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG)));
            SetDsAcl(V, u1_g2_adBypassIngressEdit_f, &action, 1);
        }
    }
    if (!pe->buffer)
    {
        DRV_IOCTL(lchip, ad_index, cmdw, &action);
    }
    else
    {
        sal_memcpy(&pe->buffer->action, &action, sizeof(action));
    }

    return CTC_E_NONE;
}

#define _ACL_UDF_
int32
_sys_usw_acl_add_udfkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add)
{
    int32 ret = CTC_E_NONE;
    uint32 data = 0;
    uint32 mask = 0;
    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    drv_acl_port_info_t drv_acl_port_info;
    sys_acl_buffer_t* p_buffer = NULL;

    sal_memset(&drv_acl_port_info, 0, sizeof(drv_acl_port_info_t));

    CTC_PTR_VALID_CHECK(key_field);
    CTC_PTR_VALID_CHECK(pe);

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    p_buffer = pe->buffer;

    if (is_add)
    {
        data = key_field->data;
        mask = key_field->mask;
    }
    switch(key_field->type)
    {
        case CTC_FIELD_KEY_PORT:
            if(is_add)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_build_port_info(lchip, pe->key_type, key_field, &drv_acl_port_info));
            }
            SetDsAclQosUdfKey320(A, portBitmap_f, p_buffer->key,  drv_acl_port_info.bitmap_64);
            SetDsAclQosUdfKey320(A, portBitmap_f, p_buffer->mask, drv_acl_port_info.bitmap_64_mask);
            SetDsAclQosUdfKey320(V, portBase_f, p_buffer->key,  drv_acl_port_info.bitmap_base);
            SetDsAclQosUdfKey320(V, portBase_f, p_buffer->mask, drv_acl_port_info.bitmap_base_mask);
            SetDsAclQosUdfKey320(V, aclLabel_f, p_buffer->key, drv_acl_port_info.class_id_data);
            SetDsAclQosUdfKey320(V, aclLabel_f, p_buffer->mask, drv_acl_port_info.class_id_mask);
            if ((DRV_FLOWPORTTYPE_LPORT == drv_acl_port_info.gport_type) || !is_add)
            {
                SetDsAclQosUdfKey320(V, logicPort_f, p_buffer->key, drv_acl_port_info.gport);
                SetDsAclQosUdfKey320(V, logicPort_f, p_buffer->mask, drv_acl_port_info.gport_mask);
            }
            break;
        case CTC_FIELD_KEY_CLASS_ID:
            SetDsAclQosUdfKey320(V, aclLabel_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, aclLabel_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_ETHER_TYPE:
            SetDsAclQosUdfKey320(V, etherType_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, etherType_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_IP_HDR_ERROR:
            SetDsAclQosUdfKey320(V, ipHeaderError_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, ipHeaderError_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_IP_OPTIONS:
            SetDsAclQosUdfKey320(V, ipOptions_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, ipOptions_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_IP_FRAG:
            if(is_add)
            {
                CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
            }
            SetDsAclQosUdfKey320(V, isFragPkt_f, p_buffer->key, tmp_data);
            SetDsAclQosUdfKey320(V, isFragPkt_f, p_buffer->mask, tmp_mask & 0x1);
            break;
        case CTC_FIELD_KEY_L4_DST_PORT:
            SetDsAclQosUdfKey320(V, l4DestPort_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, l4DestPort_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_L4_SRC_PORT :
            SetDsAclQosUdfKey320(V, l4SrcPort_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, l4SrcPort_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_L2_TYPE:
            SetDsAclQosUdfKey320(V, layer2Type_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, layer2Type_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_VLAN_NUM:
            SetDsAclQosUdfKey320(V, layer2Type_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, layer2Type_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_IP_PROTOCOL:
            SetDsAclQosUdfKey320(V, layer3HeaderProtocol_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, layer3HeaderProtocol_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_L3_TYPE:
            SetDsAclQosUdfKey320(V, layer3Type_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, layer3Type_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_L4_TYPE:
            SetDsAclQosUdfKey320(V, layer4Type_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, layer4Type_f, p_buffer->mask, mask);
            break;
        case CTC_FIELD_KEY_GEM_PORT:
            SetDsAclQosUdfKey320(V, metadata_f, p_buffer->key, data & 0x3FFF);
            SetDsAclQosUdfKey320(V, metadata_f, p_buffer->mask, mask & 0x3FFF);
            SetDsAclQosUdfKey320(V, metadataType_f, p_buffer->key, (data>>14) & 0x3);
            SetDsAclQosUdfKey320(V, metadataType_f, p_buffer->mask, (mask>>14) & 0x3);
            break;
        case CTC_FIELD_KEY_METADATA:
            SetDsAclQosUdfKey320(V, metadata_f, p_buffer->key, data);
            SetDsAclQosUdfKey320(V, metadata_f, p_buffer->mask, mask & 0x3FFF);
            SetDsAclQosUdfKey320(V, metadataType_f, p_buffer->key, (is_add? 1: 0));     /*ACLMETADATATYPE_METADATA = 1*/
            SetDsAclQosUdfKey320(V, metadataType_f, p_buffer->mask, (is_add? 0x3: 0));
            break;
        case CTC_FIELD_KEY_UDF:
            {
                 ctc_acl_udf_t* p_udf_info_data = (ctc_acl_udf_t*)key_field->ext_data;
                 ctc_acl_udf_t* p_udf_info_mask = (ctc_acl_udf_t*)key_field->ext_mask;
                 sys_acl_udf_entry_t* p_udf_entry;

                 if (is_add)
                 {
                     uint32 hw_udf[4] = {0};
                     _sys_usw_acl_get_udf_info(lchip, p_udf_info_data->udf_id, &p_udf_entry);
                     if (!p_udf_entry || !p_udf_entry->key_index_used )
                     {
                         return CTC_E_NOT_EXIST;
                     }
                     SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_data->udf);
                     SetDsAclQosUdfKey320(A, udf_f, p_buffer->key,  hw_udf);
                     SYS_USW_SET_HW_UDF(lchip, hw_udf, p_udf_info_mask->udf);
                     SetDsAclQosUdfKey320(A, udf_f, p_buffer->mask, hw_udf);
                     SetDsAclQosUdfKey320(V, udfHitIndex_f, p_buffer->key,  p_udf_entry->udf_hit_index);
                     SetDsAclQosUdfKey320(V, udfHitIndex_f, p_buffer->mask,  0xF);
                 }
                 else
                 {
                     sal_memset(p_udf_info_mask->udf, 0, CTC_ACL_UDF_BYTE_NUM);
                     SetDsAclQosUdfKey320(A, udf_f, p_buffer->mask, p_udf_info_mask->udf);
                     SetDsAclQosUdfKey320(V, udfHitIndex_f, p_buffer->mask, 0);
                 }
                SetDsAclQosUdfKey320(V, udfValid_f, p_buffer->key, is_add ? 1:0);
                SetDsAclQosUdfKey320(V, udfValid_f, p_buffer->mask, is_add ? 1:0);
                pe->udf_id = is_add ? p_udf_info_data->udf_id : 0;
            }
            break;
        default :
            ret = CTC_E_NOT_SUPPORT;
            break;
    }
    return ret;
}

int32
_sys_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add)
{
    sys_acl_udf_entry_t *p_udf_entry;

    /* operator UDF CAM*/

    uint32 tmp_data = 0;
    uint32 tmp_mask = 0;
    uint32 bitmap_64[2] = { 0 };
    uint32 bitmap_64_mask[2] = { 0 };
    uint8 bitmap_base = 0;
    uint8 bitmap_base_mask = 0;
    ctc_field_port_t* p_ctc_field_port = NULL;
    ParserUdfCam_m udf_cam;
    TempParserUdfKey_m udf_key;
    TempParserUdfKey_m udf_key_mask;

    CTC_PTR_VALID_CHECK(key_field);

    sal_memset(&udf_cam, 0, sizeof(ParserUdfCam_m));
    sal_memset(&udf_key, 0, sizeof(TempParserUdfKey_m));
    sal_memset(&udf_key_mask, 0, sizeof(TempParserUdfKey_m));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC,  "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "udf_id:%d\n", udf_id);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "field type:%d\n", key_field->type);

    _sys_usw_acl_get_udf_info(lchip,  udf_id, &p_udf_entry);

    if(!p_udf_entry || !p_udf_entry->key_index_used)
   	{
        return CTC_E_NOT_EXIST;
   	}

	CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_udf_cam(lchip, &udf_cam, p_udf_entry->key_index));
    GetParserUdfCam(A, array_0_fields_f, &udf_cam, &udf_key);
    GetParserUdfCam(A, array_1_fields_f, &udf_cam, &udf_key_mask);
    switch(key_field->type)
    {
        case CTC_FIELD_KEY_PORT:
            p_ctc_field_port = (ctc_field_port_t*)(key_field->ext_data);
             /*
             *  Support 64 port_bitmap atmost in one udf entry with bitmap_base 0-3
             *  example: user care port 00001111 (that is port0 port1 port2 port3).
             *  In hardware,  should be 00000000 - data
             *                          11110000 - mask.
             *  so port0 port1 port2 port3 will hit, other ports won't.
             */

            if (p_ctc_field_port->port_bitmap[0] || p_ctc_field_port->port_bitmap[1])
            {
                bitmap_64[0]        = p_ctc_field_port->port_bitmap[0];
                bitmap_64[1]        = p_ctc_field_port->port_bitmap[1];
                bitmap_base = 0;
            }
            else if (p_ctc_field_port->port_bitmap[2] || p_ctc_field_port->port_bitmap[3])
            {
                bitmap_64[0]        = p_ctc_field_port->port_bitmap[2];
                bitmap_64[1]        = p_ctc_field_port->port_bitmap[3];
                bitmap_base = 1;
            }
            else if (p_ctc_field_port->port_bitmap[4] || p_ctc_field_port->port_bitmap[5])
            {
                bitmap_64[0]        = p_ctc_field_port->port_bitmap[4];
                bitmap_64[1]        = p_ctc_field_port->port_bitmap[5];
                bitmap_base = 2;
            }
            else if (p_ctc_field_port->port_bitmap[6] || p_ctc_field_port->port_bitmap[7])
            {
                bitmap_64[0]        = p_ctc_field_port->port_bitmap[6];
                bitmap_64[1]        = p_ctc_field_port->port_bitmap[7];
                bitmap_base = 3;
            }

			if(is_add)
			{
                bitmap_64_mask[0] = ~bitmap_64[0];
                bitmap_64_mask[1] = ~bitmap_64[1];
                bitmap_base_mask = 0x3;
			}
			else
			{
                sal_memset(bitmap_64_mask,0,8);
                bitmap_base_mask = 0;
			}
            SetTempParserUdfKey(A, portBitmap_f, &udf_key, bitmap_64);
            SetTempParserUdfKey(A, portBitmap_f, &udf_key_mask, bitmap_64_mask);
            SetTempParserUdfKey(V, portBitmapBase_f, &udf_key, bitmap_base);
            SetTempParserUdfKey(V, portBitmapBase_f, &udf_key_mask, bitmap_base_mask);
            break;

        case CTC_FIELD_KEY_L2_TYPE:
            SetTempParserUdfKey(V, layer2Type_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, layer2Type_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_VLAN_NUM:
            SetTempParserUdfKey(V, vlanNum_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, vlanNum_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_ETHER_TYPE:
            SetTempParserUdfKey(V, etherType_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, etherType_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_L3_TYPE:
            if (!DRV_FLD_IS_EXISIT(TempParserUdfKey_t, TempParserUdfKey_layer3Type_f))
            {
                return CTC_E_NOT_SUPPORT;
            }
            SetTempParserUdfKey(V, layer3Type_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, layer3Type_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_IP_OPTIONS:
            SetTempParserUdfKey(V, u1_gIp_ipOptions_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u1_gIp_ipOptions_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_IP_FRAG:
            CTC_ERROR_RETURN(_sys_usw_acl_map_ip_frag(lchip, key_field->data, &tmp_data, &tmp_mask));
            SetTempParserUdfKey(V, u1_gIp_fragInfo_f, &udf_key, tmp_data);
            SetTempParserUdfKey(V, u1_gIp_fragInfo_f, &udf_key_mask,tmp_mask);

            break;
        case CTC_FIELD_KEY_LABEL_NUM:
            SetTempParserUdfKey(V, u1_gMpls_labelNum_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u1_gMpls_labelNum_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_L4_TYPE:
            SetTempParserUdfKey(V, layer4Type_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, layer4Type_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_GRE_FLAGS:
            SetTempParserUdfKey(V, u2_gGre_greFlags_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gGre_greFlags_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_GRE_PROTOCOL_TYPE:
            SetTempParserUdfKey(V, u2_gGre_greProtocolType_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gGre_greProtocolType_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_L4_DST_PORT:
            SetTempParserUdfKey(V, u2_gTcpUdp_l4DestPort_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gTcpUdp_l4DestPort_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_L4_SRC_PORT:
            SetTempParserUdfKey(V, u2_gTcpUdp_l4SourcePort_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gTcpUdp_l4SourcePort_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_L4_USER_TYPE:
            SetTempParserUdfKey(V, u2_gTcpUdp_layer4UserType_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gTcpUdp_layer4UserType_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_TCP_OPTIONS:
            SetTempParserUdfKey(V, u2_gTcpUdp_tcpOptionExist_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gTcpUdp_tcpOptionExist_f, &udf_key_mask, key_field->mask);
            break;

        case CTC_FIELD_KEY_IP_PROTOCOL:
            SetTempParserUdfKey(V, u2_gOthers_layer3HeaderProtocol_f, &udf_key, key_field->data);
            SetTempParserUdfKey(V, u2_gOthers_layer3HeaderProtocol_f, &udf_key_mask, key_field->mask);
            break;
        case CTC_FIELD_KEY_UDF_ENTRY_VALID:
            SetParserUdfCam(V, entryValid_f, &udf_cam, is_add?1:0);
            break;
        default:
            return CTC_E_NOT_SUPPORT;
    }

    SetParserUdfCam(A, array_0_fields_f, &udf_cam, &udf_key);
    SetParserUdfCam(A, array_1_fields_f, &udf_cam, &udf_key_mask);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_udf_cam(lchip, &udf_cam,p_udf_entry->key_index));

    return CTC_E_NONE;
}

#define _ACL_LEAGUE_
int32
_sys_usw_acl_set_league_table(uint8 lchip, ctc_acl_league_t* league)
{
    uint32 cmd = 0;
    uint32 tmp = league->lkup_level_bitmap >> (league->acl_priority+1);

    if (CTC_INGRESS == league->dir)
    {
        cmd = DRV_IOW(IpeAclQosCtl_t, (IpeAclQosCtl_acl0AdMergeBitMap_f+league->acl_priority));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    }
    else
    {
        cmd = DRV_IOW(EpeAclQosCtl_t, (EpeAclQosCtl_acl0AdMergeBitMap_f+league->acl_priority));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    }

    return CTC_E_NONE;
}

#define _ACL_CID_PAIR_
int32
_sys_usw_acl_tcam_cid_pair_lookup(uint8 lchip, ctc_acl_cid_pair_t* cid_pair, uint32* index_o)
{
    uint32  index = 0;
    uint32 cmd   = 0;
    uint8  dst_cid_get = 0;
    uint8  dst_cid = 0;
    uint8  dst_cid_valid_get = 0;
    uint8  dst_cid_valid = 0;
    uint8  src_cid_get = 0;
    uint8  src_cid = 0;
    uint8  src_cid_valid_get = 0;
    uint8  src_cid_valid = 0;
    DsCategoryIdPairTcamKey_m  ds_key;
    DsCategoryIdPairTcamKey_m  ds_mask;
    tbl_entry_t   tcam_key;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_key, 0 ,sizeof(DsCategoryIdPairTcamKey_m));
    sal_memset(&ds_mask, 0 ,sizeof(DsCategoryIdPairTcamKey_m));
    tcam_key.data_entry = (uint32*)(&ds_key);
    tcam_key.mask_entry = (uint32*)(&ds_mask);

    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID))
    {
        src_cid = cid_pair->src_cid;
        src_cid_valid = 1;
    }
    if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID))
    {
        dst_cid = cid_pair->dst_cid;
        dst_cid_valid = 1;
    }
    if((0 == src_cid_valid) && (0 == dst_cid_valid))
    {
        index = MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) - 1;
    }
    else
    {

        for(index = 0; index < MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR); index++)
        {
            cmd = DRV_IOR(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tcam_key));
            src_cid_valid_get = GetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f, &ds_key);
            dst_cid_valid_get = GetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, &ds_key);
            src_cid_get = GetDsCategoryIdPairTcamKey(V, srcCategoryId_f, &ds_key);
            dst_cid_get = GetDsCategoryIdPairTcamKey(V, destCategoryId_f, &ds_key);
            if(   (src_cid_get == src_cid) && (dst_cid_get == dst_cid)
               && (src_cid_valid_get == src_cid_valid) && (dst_cid_valid_get == dst_cid_valid) )
            {
                break;
            }
        }
    }

    if(index_o)
    {
        *index_o = index;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_build_cid_pair_action(uint8 lchip, void* p_buffer, ctc_acl_cid_pair_t* cid_pair)
{
    uint8 lkup_type  = 0;
     /*uint8 gport_type = 0;*/

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(CTC_ACL_CID_PAIR_ACTION_PERMIT == cid_pair->action_mode)
    {
        SetDsCategoryIdPair(V, operationMode_f, p_buffer, 1);
        SetDsCategoryIdPair(V, u1_g2_permit_f, p_buffer, 1);
        SetDsCategoryIdPair(V, u1_g2_deny_f, p_buffer, 0);
    }
    else if(CTC_ACL_CID_PAIR_ACTION_DISCARD == cid_pair->action_mode)
    {
        SetDsCategoryIdPair(V, operationMode_f, p_buffer, 1);
        SetDsCategoryIdPair(V, u1_g2_permit_f, p_buffer, 0);
        SetDsCategoryIdPair(V, u1_g2_deny_f, p_buffer, 1);
    }
    else if(CTC_ACL_CID_PAIR_ACTION_OVERWRITE_ACL == cid_pair->action_mode)
    {
        /*if(cid_pair->acl_prop.use_port_bitmap)
        {
            gport_type = DRV_FLOWPORTTYPE_BITMAP;
        }
        else if(cid_pair->acl_prop.use_metadata)
        {
            gport_type = DRV_FLOWPORTTYPE_METADATA;
        }
        else if(cid_pair->acl_prop.use_logic_port)
        {
            gport_type = DRV_FLOWPORTTYPE_LPORT;
        }
        else
        {
            gport_type = DRV_FLOWPORTTYPE_GPORT;
        }*/
        SYS_ACL_PROPERTY_CHECK((&cid_pair->acl_prop));
        if(cid_pair->acl_prop.tcam_lkup_type == CTC_ACL_TCAM_LKUP_TYPE_VLAN)
        {
            return CTC_E_NOT_SUPPORT;
        }
        SetDsCategoryIdPair(V, operationMode_f, p_buffer, 0);
        SetDsCategoryIdPair(V, aclLookupLevel_f, p_buffer, cid_pair->acl_prop.acl_priority);
        lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, cid_pair->acl_prop.tcam_lkup_type);
        SetDsCategoryIdPair(V, u1_g1_lookupType_f, p_buffer, lkup_type);
        SetDsCategoryIdPair(V, u1_g1_aclLabel_f, p_buffer, cid_pair->acl_prop.class_id);
        SetDsCategoryIdPair(V, u1_g1_aclUsePIVlan_f, p_buffer,
                                CTC_FLAG_ISSET(cid_pair->acl_prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        SetDsCategoryIdPair(V, u1_g1_aclUseGlobalPortType_f, p_buffer, DRV_FLOWPORTTYPE_METADATA);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_hash_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint32 cmd    = 0;
    uint32 key_id = DsCategoryIdPairHashLeftKey_t;
    uint32 act_id = DsCategoryIdPairHashLeftAd_t;
    uint32 key_index = 0;
    uint32 ad_index  = 0;
    uint8  key_offset = 0;
    uint8  src_cid_valid = 0;
    uint8  dst_cid_valid = 0;
    DsCategoryIdPairHashLeftKey_m  ds_key;
    drv_acc_in_t    in;
    drv_acc_out_t   out;
    sys_acl_cid_action_t cid_act_new;
    sys_acl_cid_action_t cid_act_old;
    sys_acl_cid_action_t* cid_act_get = NULL;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_key, 0, sizeof(DsCategoryIdPairHashLeftKey_m));
    sal_memset(&in,  0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(&cid_act_new, 0, sizeof(sys_acl_cid_action_t));
    sal_memset(&cid_act_old, 0, sizeof(sys_acl_cid_action_t));

    src_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID);
    dst_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID);

    /*1. acc lookup by key*/
    if(src_cid_valid)
    {
        SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, &ds_key, 1);
        SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryId_f, &ds_key, cid_pair->src_cid);
    }
    if(dst_cid_valid)
    {
        SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, &ds_key, 1);
        SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryId_f, &ds_key, cid_pair->dst_cid);
    }
    SetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, &ds_key, 1);

    in.type     = DRV_ACC_TYPE_LOOKUP;
    in.op_type  = DRV_ACC_OP_BY_KEY;
    in.tbl_id   = key_id;
    in.data     = &ds_key;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"\n %% conflict: %-3uhit: %-3uoffset: %-3ukey_index: %-6uleft: %-3u\n",
                            out.is_conflict, out.is_hit, out.offset, out.key_index, out.is_left);

    if(out.is_conflict)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Hash conflict\n");
        return CTC_E_HASH_CONFLICT;
    }
    key_offset = out.offset;
    if(!out.is_left)    /*is right key*/
    {
        key_id = DsCategoryIdPairHashRightKey_t;
        act_id = DsCategoryIdPairHashRightAd_t;
    }

    if(out.is_hit)  /*entry exist*/
    {
        ad_index  = out.ad_index;
        key_index = out.key_index;

        cid_act_new.is_left = out.is_left;
        CTC_ERROR_RETURN(_sys_usw_acl_build_cid_pair_action(lchip, &(cid_act_new.ds_cid_action), cid_pair));

        cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &(cid_act_old.ds_cid_action)));
        cid_act_old.is_left = out.is_left;

        CTC_ERROR_RETURN(ctc_spool_add(
                    p_usw_acl_master[lchip]->cid_spool, &cid_act_new, &cid_act_old, &cid_act_get));

        ad_index = cid_act_get->ad_index;
    }
    else
    {
        cid_act_new.is_left = out.is_left;
        CTC_ERROR_RETURN(_sys_usw_acl_build_cid_pair_action(lchip, &(cid_act_new.ds_cid_action), cid_pair));

        CTC_ERROR_RETURN(ctc_spool_add(
                    p_usw_acl_master[lchip]->cid_spool, &cid_act_new, NULL, &cid_act_get));

        ad_index = cid_act_get->ad_index;
        key_index = out.key_index;
    }

    /*2. add ad*/
    cmd = DRV_IOW(act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &(cid_act_new.ds_cid_action)));

    /*3. add key, need to update ad_index*/
    SetDsCategoryIdPairHashLeftKey(V, array_0_adIndex_f, &ds_key, ad_index);

    sal_memset(&in,  0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    in.type     = DRV_ACC_TYPE_ADD;
    in.op_type  = DRV_ACC_OP_BY_INDEX;
    in.tbl_id   = key_id;
    in.data     = &ds_key;
    in.index    = key_index;
    in.offset   = key_offset;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_add_tcam_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint32  index = 0;
    uint8  old_exist = 0;
    uint8  src_cid_valid = 0;
    uint8  dst_cid_valid = 0;
    uint32 cmd = 0;
    uint32 key_index = 0;
    DsCategoryIdPairTcamKey_m  ds_key;
    DsCategoryIdPairTcamKey_m  ds_mask;
    DsCategoryIdPairTcamAd_m   ds_act;
    tbl_entry_t   tcam_key;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"  %% PARAM: src-cid: %u, dst-cid: %u, action-mode: %u\n",
                                cid_pair->src_cid, cid_pair->dst_cid, cid_pair->action_mode);

    sal_memset(&ds_key,   0, sizeof(DsCategoryIdPairTcamKey_m));
    sal_memset(&ds_mask,  0, sizeof(DsCategoryIdPairTcamKey_m));
    sal_memset(&ds_act,   0, sizeof(DsCategoryIdPairTcamAd_m));
    sal_memset(&tcam_key, 0, sizeof(tbl_entry_t));

    src_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID);
    dst_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID);

    /*1. check if key exist*/
    if(src_cid_valid || dst_cid_valid)
    {
        CTC_ERROR_RETURN(_sys_usw_acl_tcam_cid_pair_lookup(lchip, cid_pair, &index));
    }
    else
    {
        index = MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR);
    }

    /*2. get key_index(if cid pair exist, use old key_index)*/
    if(MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) == index)
    {
        if(src_cid_valid && dst_cid_valid)
        {
            CTC_ERROR_RETURN(sys_usw_ofb_alloc_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 0, &key_index, NULL));
        }
        else if(src_cid_valid || dst_cid_valid)
        {
            CTC_ERROR_RETURN(sys_usw_ofb_alloc_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 1, &key_index, NULL));
        }
        else
        {   /*last entry, not care src cid and dst cid*/
            key_index = MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) -1;
        }
    }
    else
    {   /*old exist, only need update action*/
        old_exist = 1;
        key_index = index;
    }

    /*3. add action (when old exist, update)*/
    CTC_ERROR_RETURN(_sys_usw_acl_build_cid_pair_action(lchip, &ds_act, cid_pair));
    cmd = DRV_IOW(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &ds_act));

    /*4. add key if not exist*/
    if(!old_exist)
    {
        SetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, &ds_key,  dst_cid_valid);
        SetDsCategoryIdPairTcamKey(V, destCategoryIdClassfied_f, &ds_mask, dst_cid_valid);
        SetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f,  &ds_key,  src_cid_valid);
        SetDsCategoryIdPairTcamKey(V, srcCategoryIdClassfied_f,  &ds_mask, src_cid_valid);
        if(dst_cid_valid)
        {
            SetDsCategoryIdPairTcamKey(V, destCategoryId_f, &ds_key, cid_pair->dst_cid);
            SetDsCategoryIdPairTcamKey(V, destCategoryId_f, &ds_mask, 0xFF);
        }
        if(src_cid_valid)
        {
            SetDsCategoryIdPairTcamKey(V, srcCategoryId_f, &ds_key, cid_pair->src_cid);
            SetDsCategoryIdPairTcamKey(V, srcCategoryId_f, &ds_mask, 0xFF);
        }
        cmd = DRV_IOW(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
        tcam_key.data_entry = (uint32*)(&ds_key);
        tcam_key.mask_entry = (uint32*)(&ds_mask);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &tcam_key));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_remove_tcam_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint32  index   = 0;
    uint32  cmd     = 0;
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_acl_tcam_cid_pair_lookup(lchip, cid_pair, &index));
    if(MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) == index)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }
    else    /*if exist, clear key*/
    {
        if((MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR) - 1) != index)
        {
            if(CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID) && CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID))
            {
                CTC_ERROR_RETURN(sys_usw_ofb_free_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 0, index));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_ofb_free_offset(lchip, p_usw_acl_master[lchip]->ofb_type_cid, 1, index));
            }
        }

        cmd = DRV_IOD(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &cmd));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_acl_remove_hash_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair)
{
    uint32 cmd    = 0;
    uint32 key_id = DsCategoryIdPairHashLeftKey_t;
    uint32 act_id = DsCategoryIdPairHashLeftAd_t;
    uint32 key_index = 0;
    uint32 ad_index  = 0;
    uint8  key_offset = 0;
    uint8  src_cid_valid = 0;
    uint8  dst_cid_valid = 0;
    DsCategoryIdPairHashLeftKey_m   ds_key;
    drv_acc_in_t    in;
    drv_acc_out_t   out;
    sys_acl_cid_action_t cid_act;

    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_key, 0, sizeof(DsCategoryIdPairHashLeftKey_m));
    sal_memset(&in,  0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(&cid_act, 0, sizeof(sys_acl_cid_action_t));

    /*1. acc lookup by key*/
    src_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_SRC_CID);
    dst_cid_valid = CTC_FLAG_ISSET(cid_pair->flag, CTC_ACL_CID_PAIR_FLAG_DST_CID);

    /*1. acc lookup by key*/
    if(src_cid_valid)
    {
        SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, &ds_key, 1);
        SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryId_f, &ds_key, cid_pair->src_cid);
    }
    if(dst_cid_valid)
    {
        SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, &ds_key, 1);
        SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryId_f, &ds_key, cid_pair->dst_cid);
    }
    SetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, &ds_key, 1);

    in.type     = DRV_ACC_TYPE_LOOKUP;
    in.op_type  = DRV_ACC_OP_BY_KEY;
    in.tbl_id   = key_id;
    in.data     = &ds_key;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    if(!out.is_hit)
    {
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }
    if(!out.is_left)    /*judge left or right*/
    {
        key_id = DsCategoryIdPairHashRightKey_t;
        act_id = DsCategoryIdPairHashRightAd_t;
    }
    key_index  = out.key_index;
    ad_index   = out.ad_index;
    key_offset = out.offset;

    /*2. remove spool*/
    cmd = DRV_IOR(act_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &cid_act.ds_cid_action));
    cid_act.is_left = out.is_left;

    CTC_ERROR_RETURN(ctc_spool_remove(p_usw_acl_master[lchip]->cid_spool, &cid_act, NULL));

    /*3. remove key*/
    sal_memset(&in,  0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(&ds_key, 0, sizeof(DsCategoryIdPairHashLeftKey_m));

    in.type     = DRV_ACC_TYPE_ADD;
    in.op_type  = DRV_ACC_OP_BY_INDEX;
    in.tbl_id   = key_id;
    in.index    = key_index;
    in.offset   = key_offset;
    in.data     = &ds_key;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    return CTC_E_NONE;
}

int32
_sys_usw_acl_set_cid_priority(uint8 lchip, uint8 is_src_cid, uint8* p_prio_arry)
{
    uint32 cmd = 0;
    IpeFwdCategoryCtl_m  ipe_fwd_cid;

    sal_memset(&ipe_fwd_cid, 0, sizeof(IpeFwdCategoryCtl_m));

    /* IpeFwdCategoryCtl */
    cmd = DRV_IOR(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_cid));

    if(is_src_cid)
    {
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_pktSrcCategoryIdPri_f,     &ipe_fwd_cid, p_prio_arry[0]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_i2eSrcCategoryIdPri_f,     &ipe_fwd_cid, p_prio_arry[1]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_globalSrcCategoryIdPri_f,  &ipe_fwd_cid, p_prio_arry[2]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_staticSrcCategoryIdPri_f,  &ipe_fwd_cid, p_prio_arry[3]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_dynamicSrcCategoryIdPri_f, &ipe_fwd_cid, p_prio_arry[4]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_ifSrcCategoryIdPri_f,      &ipe_fwd_cid, p_prio_arry[5]);
        SetIpeFwdCategoryCtl(V, gSrcCidPri_0_defaultSrcCategoryIdPri_f, &ipe_fwd_cid, p_prio_arry[6]);
    }
    else
    {
        SetIpeFwdCategoryCtl(V, gDstCidPri_dynamicDstCategoryIdPri_f, &ipe_fwd_cid, p_prio_arry[0]);
        SetIpeFwdCategoryCtl(V, gDstCidPri_dsflowDstCategoryIdPri_f,  &ipe_fwd_cid, p_prio_arry[1]);
        SetIpeFwdCategoryCtl(V, gDstCidPri_staticDstCategoryIdPri_f,  &ipe_fwd_cid, p_prio_arry[2]);
        SetIpeFwdCategoryCtl(V, gDstCidPri_defaultDstCategoryIdPri_f, &ipe_fwd_cid, p_prio_arry[3]);
    }

    cmd = DRV_IOW(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_cid));

    return CTC_E_NONE;
}


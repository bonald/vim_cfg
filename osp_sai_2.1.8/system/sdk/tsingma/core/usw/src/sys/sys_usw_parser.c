/**
 @file sys_usw_parser.c

 @date 2009-12-22

 @version v2.0

---file comments----
*/

#include "sal.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "sys_usw_parser.h"
#include "sys_usw_register.h"
#include "sys_usw_chip.h"
#include "sys_usw_parser_io.h"
#include "sys_usw_ftm.h"
#include "sys_usw_mchip.h"
#include "sys_usw_acl_api.h"

#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/




#define SYS_PAS_CAM_INDEX_CHECK(index, min, max) \
    do { \
        if (index < min || index >= max) \
        { \
            return CTC_E_INVALID_PARAM; \
        } \
    } while (0)

#define SYS_PAS_OUTER_ECMP_HASH_TYPE     SYS_PARSER_HASH_TYPE_CRC
#define SYS_PAS_OUTER_LINKAGG_HASH_TYPE  SYS_PARSER_HASH_TYPE_CRC

#define SYS_PAS_INNER_ECMP_HASH_TYPE     SYS_PARSER_HASH_TYPE_CRC
#define SYS_PAS_INNER_LINKAGG_HASH_TYPE  SYS_PARSER_HASH_TYPE_CRC


#define SYS_PAS_CHIP_IPVER_MAP_CTC_IPVER(ipver) ((1 == ipver) ?  CTC_IP_VER_4 : CTC_IP_VER_6)
#define SYS_PAS_CTC_IPVER_MAP_CHIP_IPVER(ipver) ((CTC_IP_VER_4 == ipver) ? 1 : 2)

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

/**
 @brief set tpid
*/
int32
sys_usw_parser_set_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16 tpid)
{


    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "layer2 tpid type:%d, tpid:0x%X.\n", type, tpid);

    switch (type)
    {
    case CTC_PARSER_L2_TPID_CVLAN_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_cvlan_tpid(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_ITAG_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_i_tag_tpid(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_BLVAN_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_bvlan_tpid(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_0:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_svlan_tpid0(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_1:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_svlan_tpid1(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_2:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_svlan_tpid2(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_3:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_svlan_tpid3(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_CNTAG_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_cn_tag_tpid(lchip, tpid));
        break;

    case CTC_PARSER_L2_TPID_EVB_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_set_evb_tpid(lchip, tpid));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

/**
 @brief get tpid with some type
*/
int32
sys_usw_parser_get_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16* p_tpid)
{

    uint32 value = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "layer2 tpid type:0x%X.\n", type);

    switch (type)
    {
    case CTC_PARSER_L2_TPID_CVLAN_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_cvlan_tpid(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_ITAG_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_i_tag_tpid(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_BLVAN_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_bvlan_tpid(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_0:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_svlan_tpid0(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_1:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_svlan_tpid1(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_2:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_svlan_tpid2(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_SVLAN_TPID_3:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_svlan_tpid3(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_CNTAG_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_cn_tag_tpid(lchip, &value));
        break;

    case CTC_PARSER_L2_TPID_EVB_TPID:
        CTC_ERROR_RETURN(sys_usw_parser_io_get_evb_tpid(lchip, &value));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    *p_tpid = value;

    return CTC_E_NONE;
}

/**
 @brief set max_length,based on the value differentiate type or length
*/
int32
sys_usw_parser_set_max_length_filed(uint8 lchip, uint16 max_length)
{


    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "max_length:0x%X.\n", max_length);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_max_length(lchip, max_length));

    return CTC_E_NONE;
}

/**
 @brief get max_length value
*/
int32
sys_usw_parser_get_max_length_filed(uint8 lchip, uint16* p_max_length)
{

    uint32 tmp = 0;

    CTC_PTR_VALID_CHECK(p_max_length);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_parser_io_get_max_length(lchip, &tmp));
    *p_max_length = tmp;

    return CTC_E_NONE;
}

/**
 @brief set vlan parser num
*/
int32
sys_usw_parser_set_vlan_parser_num(uint8 lchip, uint8 vlan_num)
{


    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vlan_num:0x%X.\n", vlan_num);

    if (vlan_num > 2)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_parser_io_set_vlan_parsing_num(lchip, vlan_num));

    return CTC_E_NONE;
}

/**
 @brief get vlan parser num
*/
int32
sys_usw_parser_get_vlan_parser_num(uint8 lchip, uint8* p_vlan_num)
{

    uint32 tmp = 0;

    CTC_PTR_VALID_CHECK(p_vlan_num);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vlan_num:0x%X.\n", *p_vlan_num);

    CTC_ERROR_RETURN(sys_usw_parser_io_get_vlan_parsing_num(lchip, &tmp));
    *p_vlan_num = tmp;

    return CTC_E_NONE;
}

/**
 @brief set pbb header info
*/
int32
sys_usw_parser_set_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* p_pbb_header)
{

    uint32 value = 0;
    ParserPbbCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_pbb_header);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nca_value:0x%X.\n", p_pbb_header->nca_value_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "outer_vlan_is_cvlan:0x%X.\n", p_pbb_header->outer_vlan_is_cvlan);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pbb_vlan_parsing_num:0x%X.\n", p_pbb_header->vlan_parsing_num);

    if (p_pbb_header->vlan_parsing_num > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ctl, 0, sizeof(ParserPbbCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_pbb_ctl(lchip, &ctl));

    value = p_pbb_header->nca_value_en ? 1 : 0;
    SetParserPbbCtl(V, ncaValue_f, &ctl, value);
    value = p_pbb_header->outer_vlan_is_cvlan ? 1 : 0;
    SetParserPbbCtl(V, pbbOuterVlanIsCvlan_f, &ctl, value);
    value = p_pbb_header->vlan_parsing_num;
    SetParserPbbCtl(V, pbbVlanParsingNum_f, &ctl, value);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_pbb_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

/**
 @brief get pbb header info
*/
int32
sys_usw_parser_get_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* p_pbb_header)
{

    uint32 value = 0;
    ParserPbbCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_pbb_header);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ctl, 0, sizeof(ParserPbbCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_pbb_ctl(lchip, &ctl));

    GetParserPbbCtl(A, ncaValue_f, &ctl, &value);
    p_pbb_header->nca_value_en = value;

    GetParserPbbCtl(A, pbbOuterVlanIsCvlan_f, &ctl, &value);
    p_pbb_header->outer_vlan_is_cvlan = value;

    GetParserPbbCtl(A, pbbVlanParsingNum_f, &ctl, &value);
    p_pbb_header->vlan_parsing_num = value;

    return CTC_E_NONE;
}

/**
 @brief mapping layer3 type
*/
int32
sys_usw_parser_mapping_l3_type(uint8 lchip, uint8 index, ctc_parser_l2_protocol_entry_t* p_entry)
{

    uint8  isEth = 0;
    uint8  isSAP = 0;
    uint32 value = 0;
    ParserLayer2ProtocolCam_m cam;
    ParserLayer2ProtocolCamValid_m valid;

    CTC_PTR_VALID_CHECK(p_entry);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:0x%X.\n", index);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3_type:0x%X.\n", p_entry->l3_type);

#if 0
    SYS_PAS_CAM_INDEX_CHECK(entry->l2_type, CTC_PARSER_L2_TYPE_RSV_USER_DEFINE0,
                            MAX_CTC_PARSER_L2_TYPE);
#endif

    /*only l3_type: 15 can be configed by user*/
    if (p_entry->l3_type != CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_entry->addition_offset > 0xf) || (p_entry->l2_type > 0xf))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (index > (MCHIP_CAP(SYS_CAP_PARSER_L2_PROTOCOL_USER_ENTRY) - 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(ParserLayer2ProtocolCam_m));
    sal_memset(&valid, 0, sizeof(ParserLayer2ProtocolCamValid_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer2_protocol_cam(lchip, &cam));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer2_protocol_cam_valid(lchip, &valid));

    switch (p_entry->l2_type)
    {
    case CTC_PARSER_L2_TYPE_NONE:
        isEth = 0;
        isSAP = 0;
        break;

    case CTC_PARSER_L2_TYPE_ETH_V2:
    case CTC_PARSER_L2_TYPE_ETH_SNAP:
        isEth = 1;
        isSAP = 0;
        break;

    case CTC_PARSER_L2_TYPE_PPP_2B:
    case CTC_PARSER_L2_TYPE_PPP_1B:
        SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;


    case CTC_PARSER_L2_TYPE_ETH_SAP:
        isEth = 1;
        isSAP = 1;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    switch (index)
    {
    case 0:
        value = p_entry->addition_offset;
        SetParserLayer2ProtocolCam(V, array_0_l2CamAdditionalOffset_f,
                            &cam, value);
        value = p_entry->l3_type;
        SetParserLayer2ProtocolCam(V, array_0_l2CamLayer3Type_f,
                            &cam, value);
        value = p_entry->mask & 0x1fffff;
        SetParserLayer2ProtocolCam(V, array_0_l2CamMask_f,
                            &cam, value);

        value = (isEth << 20) | (isSAP << 19)
                | (p_entry->l2_type << 16) | (p_entry->l2_header_protocol);
        SetParserLayer2ProtocolCam(V, array_0_l2CamValue_f,
                            &cam, value);
        break;

    case 1:
        value = p_entry->addition_offset;
        SetParserLayer2ProtocolCam(V, array_1_l2CamAdditionalOffset_f,
                            &cam, value);
        value = p_entry->l3_type;
        SetParserLayer2ProtocolCam(V, array_1_l2CamLayer3Type_f,
                            &cam, value);
        value = p_entry->mask & 0x1fffff;
        SetParserLayer2ProtocolCam(V, array_1_l2CamMask_f,
                            &cam, value);
        value = (isEth << 20) | (isSAP << 19)
                | (p_entry->l2_type << 16) | (p_entry->l2_header_protocol);
        SetParserLayer2ProtocolCam(V, array_1_l2CamValue_f,
                            &cam, value);
        break;

    case 2:
        value = p_entry->addition_offset;
        SetParserLayer2ProtocolCam(V, array_2_l2CamAdditionalOffset_f,
                            &cam, value);
        value = p_entry->l3_type;
        SetParserLayer2ProtocolCam(V, array_2_l2CamLayer3Type_f,
                            &cam, value);
        value = p_entry->mask & 0x1fffff;
        SetParserLayer2ProtocolCam(V, array_2_l2CamMask_f,
                            &cam, value);
        value = (isEth << 20) | (isSAP << 19)
                | (p_entry->l2_type << 16) | (p_entry->l2_header_protocol);
        SetParserLayer2ProtocolCam(V, array_2_l2CamValue_f,
                            &cam, value);
        break;

    case 3:
        value = p_entry->addition_offset;
        SetParserLayer2ProtocolCam(V, array_3_l2CamAdditionalOffset_f,
                            &cam, value);
        value = p_entry->l3_type;
        SetParserLayer2ProtocolCam(V, array_3_l2CamLayer3Type_f,
                            &cam, value);
        value = p_entry->mask & 0x1fffff;
        SetParserLayer2ProtocolCam(V, array_3_l2CamMask_f,
                            &cam, value);
        value = (isEth << 20) | (isSAP << 19)
                | (p_entry->l2_type << 16) | (p_entry->l2_header_protocol);
        SetParserLayer2ProtocolCam(V, array_3_l2CamValue_f,
                            &cam, value);

        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    GetParserLayer2ProtocolCamValid(A, layer2CamEntryValid_f,
                  &valid, &value);

    value = ((~((uint32)1 << index)) & value) |  (1 << index);
    SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                        &valid, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam(lchip, &cam));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(lchip, &valid));

    return CTC_E_NONE;
}

/**
 @brief set the entry invalid based on the index
*/
int32
sys_usw_parser_unmapping_l3_type(uint8 lchip, uint8 index)
{

    uint32 value = 0;
    ParserLayer2ProtocolCamValid_m valid;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:0x%X.\n", index);

    if (index > (MCHIP_CAP(SYS_CAP_PARSER_L2_PROTOCOL_USER_ENTRY) - 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&valid, 0, sizeof(ParserLayer2ProtocolCamValid_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer2_protocol_cam_valid(lchip, &valid));

    GetParserLayer2ProtocolCamValid(A, layer2CamEntryValid_f, &valid, &value);

    if (CTC_IS_BIT_SET(value, index))
    {
        value &= (~(1 << index));
        SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                            &valid, value);
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(lchip, &valid));
    }

    return CTC_E_NONE;
}

/**
 @brief enable or disable parser layer3 type
*/
int32
sys_usw_parser_enable_l3_type(uint8 lchip, ctc_parser_l3_type_t l3_type, bool enable)
{

    uint32 type_en = 0;
    uint32 value = 0;
    uint32 l2_cam_layer3_type0 = 0;
    uint32 l2_cam_layer3_type1 = 0;
    uint32 l2_cam_layer3_type2 = 0;
    uint32 l2_cam_layer3_type3 = 0;
    ParserEthernetCtl_m ctl;
    ParserLayer2ProtocolCam_m cam;
    ParserLayer2ProtocolCamValid_m valid;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3_type:0x%X.\n", l3_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "enable:0x%X.\n", enable);

    if ((l3_type > CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3) || (l3_type < CTC_PARSER_L3_TYPE_IPV4))
    {
        return CTC_E_INVALID_PARAM;
    }

    type_en = ((TRUE == enable) ? 1 : 0);

    if (l3_type < CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3)
    {
        sal_memset(&ctl, 0, sizeof(ParserEthernetCtl_m));
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_ethernet_ctl(lchip, &ctl));

        switch (l3_type)
        {
        case CTC_PARSER_L3_TYPE_IPV4:
            SetParserEthernetCtl(V, ipv4TypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_IPV6:
            SetParserEthernetCtl(V, ipv6TypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_MPLS:
            SetParserEthernetCtl(V, mplsTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_MPLS_MCAST:
            SetParserEthernetCtl(V, mplsMcastTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_ARP:
            SetParserEthernetCtl(V, arpTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_FCOE:
            SetParserEthernetCtl(V, fcoeTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_TRILL:
            SetParserEthernetCtl(V, trillTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_ETHER_OAM:
            SetParserEthernetCtl(V, ethOamTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_SLOW_PROTO:
            SetParserEthernetCtl(V, slowProtocolTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_CMAC:
            if (!DRV_FLD_IS_EXISIT(ParserEthernetCtl_t, ParserEthernetCtl_pbbTypeEn_f))
            {
                return CTC_E_NOT_SUPPORT;
            }
            SetParserEthernetCtl(V, pbbTypeEn_f, &ctl, type_en);
            break;

        case CTC_PARSER_L3_TYPE_PTP:
            SetParserEthernetCtl(V, ieee1588TypeEn_f, &ctl, type_en);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_ethernet_ctl(lchip, &ctl));
    }
    else
    {
        sal_memset(&cam, 0, sizeof(ParserLayer2ProtocolCam_m));
        sal_memset(&valid, 0, sizeof(ParserLayer2ProtocolCamValid_m));

        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer2_protocol_cam(lchip, &cam));
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer2_protocol_cam_valid(lchip, &valid));

        GetParserLayer2ProtocolCamValid(A, layer2CamEntryValid_f,\
                      &valid, &value);

        l2_cam_layer3_type0 = GetParserLayer2ProtocolCam(A, array_0_l2CamLayer3Type_f,\
                                            &cam, &value);
        l2_cam_layer3_type1 = GetParserLayer2ProtocolCam(A, array_1_l2CamLayer3Type_f,\
                                            &cam, &value);
        l2_cam_layer3_type2 = GetParserLayer2ProtocolCam(A, array_2_l2CamLayer3Type_f,\
                                            &cam, &value);
        l2_cam_layer3_type3 = GetParserLayer2ProtocolCam(A, array_3_l2CamLayer3Type_f,\
                                            &cam, &value);

        if (type_en)
        {
            if (l3_type == l2_cam_layer3_type0)
            {
                value |= (1 << 0);
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type1)
            {
                value |= (1 << 1);
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type2)
            {
                value |= (1 << 2);
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type3)
            {
                value |= (1 << 3);
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if (l3_type == l2_cam_layer3_type0)
            {
                value &= (~(1 << 0));
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type1)
            {
                value &= (~(1 << 1));
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type2)
            {
                value &= (~(1 << 2));
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else if (l3_type == l2_cam_layer3_type3)
            {
                value &= (~(1 << 3));
                SetParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f,\
                                    &valid, value);
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(lchip, &valid));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_parser_hash_table_step(uint32 id, sys_parser_hash_usage_t hash_usage)
{
    if (IpeUserIdHashCtl_t == id)
    {
        return -((IpeUserIdHashCtl_gMask_0_portHashEn_f - IpeUserIdHashCtl_gMask_1_portHashEn_f) * hash_usage);
    }
    else if (IpeAclGenHashKeyCtl_t == id)
    {
        return -((IpeAclGenHashKeyCtl_gMask_0_l2IsEtherEn_f - IpeAclGenHashKeyCtl_gMask_1_l2IsEtherEn_f) * hash_usage);
    }
    else
    {
        return 0;
    }
}
/**
 @brief set layer2 hash info
*/
STATIC int32
_sys_usw_parser_set_layer2_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step1 = 0;
    int32  step2 = 0;
    uint32 value1 = 0;
    uint32 value2 = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l2_flag:0x%X.\n", p_hash_ctl->l2_flag);

    step1 = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
    step2 = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID) ? 3 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sVlanIdHashEn_f + step1, &outer_ctl, value1);

    value2 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID) ? 3 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cVlanIdHashEn_f + step1, &outer_ctl, value2);

    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_vlanIdHashEn_f + step2,
                        &inner_ctl, (value1 || value2));

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_CFI) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sCfiHashEn_f + step1, &outer_ctl, value1);

    value2 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_CFI) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cCfiHashEn_f + step1, &outer_ctl, value2);

    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_cfiHashEn_f + step2,
                        &inner_ctl, (value1 || value2));

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_COS) ? 1 : 0;
    value1 |= CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_COS) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sCosHashEn_f + step1, &outer_ctl, value1);

    value2 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_COS) ? 1 : 0;
    value2 |= CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_COS) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cCosHashEn_f + step1, &outer_ctl, value2);

    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_cosHashEn_f + step2,
                        &inner_ctl, (value1 || value2));

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l2ProtocolHashEn_f + step1, &outer_ctl, value1);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_etherTypeHashEn_f + step2, &inner_ctl, value1);

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACSA) ? 0x3F : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macSaHashEn_f + step1, &outer_ctl, value1);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macSaHashEn_f + step2, &inner_ctl, value1);

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACDA) ? 0x3F : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macDaHashEn_f + step1, &outer_ctl, value1);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macDaHashEn_f + step2, &inner_ctl, value1);

    value1 = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_PORT) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_portHashEn_f + step1, &outer_ctl, value1);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_portHashEn_f + step2, &inner_ctl, value1);


    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    }

    return CTC_E_NONE;
}
/**
 @brief set efd layer2 hash info
*/
STATIC int32
_sys_usw_parser_set_efd_layer2_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACDA);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyMacDaEn_f + step, &ipeefd_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACSA);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyMacSaEn_f + step, &ipeefd_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyVidEn_f + step, &ipeefd_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l2_flag, CTC_PARSER_L2_HASH_FLAGS_PORT);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_keyPortEn_f, &ipeefd_ctl, value);

    cmd = DRV_IOW(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    return CTC_E_NONE;
}
/**
 @brief get layer2 hash info
*/
STATIC int32
_sys_usw_parser_get_layer2_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ip_flag:0x%X.\n", p_hash_ctl->ip_flag);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_vlanIdHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_cfiHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_CFI : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_cosHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_COS : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_etherTypeHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macSaHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACSA : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macDaHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACDA : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_portHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_PORT : 0));
    }
    else
    {
        step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sVlanIdHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cVlanIdHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_VID : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sCfiHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_CFI : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cCfiHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_CFI : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_sCosHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_COS : 0));
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_COS : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_cCosHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_COS : 0));
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_COS : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l2ProtocolHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macSaHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACSA : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macDaHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACDA : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_portHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_PORT : 0));
    }

    return CTC_E_NONE;
}
/**
 @brief set efd layer2 hash info
*/
STATIC int32
_sys_usw_parser_get_efd_layer2_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyMacDaEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACDA : 0));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyMacSaEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACSA : 0));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyVidEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_keyPortEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_PORT : 0));

    return CTC_E_NONE;
}
/**
 @brief set ip hash info
*/
STATIC int32
_sys_usw_parser_set_ip_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{

    uint32 value = 0;
    int32  step1 = 0;
    int32  step2 = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ip_flag:0x%X.\n", p_hash_ctl->ip_flag);

    step1 = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
    step2 = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_PROTOCOL) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipProtocolHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipProtocolHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_DSCP) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipDscpHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipDscpHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipFlowLabelHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipFlowLabelHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA) ? 0xFFFF : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipSaHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipSaHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPDA) ? 0xFFFF : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipDaHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipDaHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_ECN) ? 0xFFFF : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipEcnHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipEcnHashEn_f + step2, &inner_ctl, value);

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    }

    return CTC_E_NONE;
}
/**
 @brief set efd ip hash info
*/
STATIC int32
_sys_usw_parser_set_efd_ip_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));


    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPDA);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv6DaEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv4DaEn_f + step, &ipeefd_ctl, value);
    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv6SaEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv4SaEn_f + step, &ipeefd_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->ip_flag, CTC_PARSER_IP_HASH_FLAGS_PROTOCOL);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyL3HdrProtocolEn_f + step, &ipeefd_ctl, value);

    cmd = DRV_IOW(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    return CTC_E_NONE;
}
/**
 @brief set ip ecmp hash info
*/
STATIC int32
_sys_usw_parser_get_ip_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ip_flag:0x%X.\n", p_hash_ctl->ip_flag);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipProtocolHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_PROTOCOL : 0);

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipDscpHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_DSCP : 0);

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipFlowLabelHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL : 0);

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipSaHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPSA : 0);

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipDaHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPDA : 0);

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipEcnHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_ECN : 0);

    }
    else
    {
        step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipProtocolHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_PROTOCOL : 0);

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipDscpHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_DSCP : 0);

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipFlowLabelHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL : 0);

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipSaHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPSA : 0);

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipDaHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPDA : 0);

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipEcnHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_ECN : 0);
    }

    return CTC_E_NONE;
}

/**
 @brief set efd ip hash info
*/
STATIC int32
_sys_usw_parser_get_efd_ip_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));
    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv6DaEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPDA : 0);

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyIpv6SaEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_IPSA : 0);

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyL3HdrProtocolEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->ip_flag, value ? CTC_PARSER_IP_HASH_FLAGS_PROTOCOL : 0);

    return CTC_E_NONE;
}
/**
 @brief set parser mpls hash info
*/
STATIC int32
_sys_usw_parser_set_mpls_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    uint8  mpls_ecmp_inner_hash_en = 0;
    ParserMplsCtl_m mpls_ctl;
    IpeUserIdHashCtl_m outer_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->mpls_flag);

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&mpls_ctl, 0, sizeof(ParserMplsCtl_m));
    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_mpls_ctl(lchip, &mpls_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_mplsLabel0HashEn_f + step, &outer_ctl, 0x7);
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_mplsLabelHashEn_f + step, &outer_ctl, 0xFF);

    mpls_ecmp_inner_hash_en = (CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL) ? 1 : 0)
                              || (CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_DSCP) ? 1 : 0)
                              || (CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPV6_FLOW_LABEL) ? 1 : 0)
                              || (CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPSA) ? 1 : 0)
                              || (CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPDA) ? 1 : 0);

    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_mplsInnerHdrHashValueEn_f + step, &outer_ctl, mpls_ecmp_inner_hash_en);

    value = CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL) ? 1 : 0;
    SetParserMplsCtl(V, mplsIpProtocolHashEn_f, &mpls_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_DSCP) ? 1 : 0;
    SetParserMplsCtl(V, mplsIpDscpHashEn_f, &mpls_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPV6_FLOW_LABEL) ? 1 : 0;
    SetParserMplsCtl(V, mplsIpFlowLabelHashEn_f, &mpls_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPSA) ? 1 : 0;
    SetParserMplsCtl(V, mplsIpSaHashEn_f, &mpls_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPDA) ? 1 : 0;
    SetParserMplsCtl(V, mplsIpDaHashEn_f, &mpls_ctl, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_mpls_ctl(lchip, &mpls_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    return CTC_E_NONE;
}

/**
 @brief get parser mpls hash info
*/
STATIC int32
_sys_usw_parser_get_mpls_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{

    uint32 value = 0;
    ParserMplsCtl_m mpls_ctl;

    sal_memset(&mpls_ctl, 0, sizeof(ParserMplsCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_mpls_ctl(lchip, &mpls_ctl));

    GetParserMplsCtl(A, mplsIpProtocolHashEn_f, &mpls_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->mpls_flag, (value ? CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL : 0));

    GetParserMplsCtl(A, mplsIpDscpHashEn_f, &mpls_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->mpls_flag, (value ? CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP : 0));

    GetParserMplsCtl(A, mplsIpFlowLabelHashEn_f, &mpls_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->mpls_flag, (value ? CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL : 0));

    GetParserMplsCtl(A, mplsIpSaHashEn_f, &mpls_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->mpls_flag, (value ? CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA : 0));

    GetParserMplsCtl(A, mplsIpDaHashEn_f, &mpls_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->mpls_flag, (value ? CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA : 0));

    return CTC_E_NONE;
}

/**
 @brief set parser pbb hash info
*/
STATIC int32
_sys_usw_parser_set_pbb_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pbb_flag:0x%X.\n", p_hash_ctl->pbb_flag);

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CMAC_DA) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbDaHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CMAC_SA) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbSaHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_ISID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbIsidHashEn_f  + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_VID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbSvlanIdHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_VID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCvlanIdHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_COS) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbScosHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_COS) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCcosHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_CFI) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbScfiHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_CFI) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCcfiHashEn_f + step, &outer_ctl, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    return CTC_E_NONE;
}

/**
 @brief set parser pbb hash info
*/
STATIC int32
_sys_usw_parser_get_pbb_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pbb_flag:0x%X.\n", p_hash_ctl->pbb_flag);

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbDaHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_CMAC_DA : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbSaHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_CMAC_SA : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbIsidHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_ISID : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbSvlanIdHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_VID : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCvlanIdHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_VID : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbScosHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_COS : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCcosHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_COS : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbScfiHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_CFI : 0);

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_pbbCcfiHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->pbb_flag, value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_CFI : 0);

    return CTC_E_NONE;
}

/**
 @brief set parser fcoe hash info
*/
STATIC int32
_sys_usw_parser_set_fcoe_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:0x%X.\n", p_hash_ctl->hash_type_bitmap);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fcoe_flag:0x%X.\n", p_hash_ctl->fcoe_flag);

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_SID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_fcoeSidHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_DID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_fcoeDidHashEn_f + step, &outer_ctl, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    return CTC_E_NONE;
}


/**
 @brief get parser fcoe hash info
*/
STATIC int32
_sys_usw_parser_get_fcoe_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_fcoeSidHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_SID : 0));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_fcoeDidHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_DID : 0));

    return CTC_E_NONE;
}

/**
 @brief set parser trill hash info
*/
STATIC int32
_sys_usw_parser_set_trill_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillVlanIdHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillSaHashEn_f + step, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillDaHashEn_f + step, &outer_ctl, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    return CTC_E_NONE;
}

/**
 @brief get parser trill hash info
*/
STATIC int32
_sys_usw_parser_get_trill_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillVlanIdHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->trill_flag, (value ? CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID : 0));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillSaHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->trill_flag, (value ? CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME : 0));

    DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_trillDaHashEn_f + step, &outer_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->trill_flag, (value ? CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME : 0));

    return CTC_E_NONE;
}

/**
 @brief set parser layer4 header hash info
*/
STATIC int32
_sys_usw_parser_set_l4_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step1 = 0;
    int32  step2 = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    step1 = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
    step2 = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_SRC_PORT) ? 3 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4SourcePortHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_l4SourcePortHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_DST_PORT) ? 3 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4DestPortHashEn_f + step1, &outer_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_l4DestPortHashEn_f + step2, &inner_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_GRE_KEY) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4GreKeyHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TYPE) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_layer4TypeEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_USER_TYPE) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_layer4UserTypeEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT) ? 3 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_vxlanL4SourcePortHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TCP_FLAG) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4TcpFlagHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TCP_ECN) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4TcpEcnHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4NvGreVsidHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_FLOW_ID) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4NvGreFlowIdHashEn_f + step1, &outer_ctl, value);

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI) ? 1 : 0;
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4VxlanVniHashEn_f + step1, &outer_ctl, value);

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    }

    return CTC_E_NONE;
}

/**
 @brief set efd l4 hash info
*/
STATIC int32
_sys_usw_parser_set_efd_l4_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_DST_PORT);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyDccpL4DstPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keySctpL4DstPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyTcpL4DstPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyUdpL4DstPortEn_f + step, &ipeefd_ctl, value);
    value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_SRC_PORT);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyDccpL4SrcPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keySctpL4SrcPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyTcpL4SrcPortEn_f + step, &ipeefd_ctl, value);
    DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyUdpL4SrcPortEn_f + step, &ipeefd_ctl, value);
    if (!(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER))
    {
        value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT);
        DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanUdpL4SrcPortEn_f, &ipeefd_ctl, value);
        value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT);
        DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanUdpL4DstPortEn_f, &ipeefd_ctl, value);
        value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI);
        DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanVniEn_f, &ipeefd_ctl, value);
        value = CTC_FLAG_ISSET(p_hash_ctl->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID);
        DRV_SET_FIELD_V(lchip, IpeEfdCtl_t, IpeEfdCtl_keyNvgreVniEn_f, &ipeefd_ctl, value);
    }

    cmd = DRV_IOW(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    return CTC_E_NONE;
}
/**
 @brief get parser layer4 header hash info
*/
STATIC int32
_sys_usw_parser_get_l4_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step = 0;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_l4SourcePortHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_SRC_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_l4DestPortHashEn_f + step, &inner_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_DST_PORT : 0));
    }
    else
    {
        step = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4SourcePortHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_SRC_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4DestPortHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_DST_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4GreKeyHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_GRE_KEY : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_layer4TypeEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TYPE : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_layer4UserTypeEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_USER_TYPE : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_vxlanL4SourcePortHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4TcpFlagHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TCP_FLAG : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4TcpEcnHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TCP_ECN : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4NvGreVsidHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4NvGreFlowIdHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_FLOW_ID : 0));

        DRV_GET_FIELD_A(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_l4VxlanVniHashEn_f + step, &outer_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI : 0));
    }

    return CTC_E_NONE;
}


/**
 @brief get efd l4 hash info
*/
STATIC int32
_sys_usw_parser_get_efd_l4_hash(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    IpeEfdCtl_m ipeefd_ctl;

    sal_memset(&ipeefd_ctl, 0, sizeof(IpeEfdCtl_m));
    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        step = IpeEfdCtl_array_1_keyDccpL4DstPortEn_f - IpeEfdCtl_array_0_keyDccpL4DstPortEn_f;
    }
    cmd = DRV_IOR(IpeEfdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipeefd_ctl));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyDccpL4DstPortEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_DST_PORT : 0));

    DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_array_0_keyDccpL4SrcPortEn_f + step, &ipeefd_ctl, &value);
    CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_SRC_PORT : 0));

    if (!(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER))
    {
        DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanUdpL4SrcPortEn_f, &ipeefd_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanUdpL4DstPortEn_f, &ipeefd_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT : 0));

        DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_keyVxlanVniEn_f, &ipeefd_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI : 0));

        DRV_GET_FIELD_A(lchip, IpeEfdCtl_t, IpeEfdCtl_keyNvgreVniEn_f, &ipeefd_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID : 0));
    }

    return CTC_E_NONE;
}

/**
 @brief set parser common hash info
*/
STATIC int32
_sys_usw_parser_set_common_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step1 = 0;
    int32  step2 = 0;
    uint8  step3 = 0;
    uint8  step4 = 0;
    uint32 value = 0;
    uint8  udf_hit_index = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    step1 = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_usage);
    step2 = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_usage);
    step3 = IpeAclGenHashKeyCtl_gUdfMask_1_udfBitmapEn_f - IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f;
    if (SYS_PARSER_HASH_USAGE_LINKAGG == hash_usage)
    {
        step4 = IpeAclGenHashKeyCtl_gUdfMask_16_udfBitmapEn_f - IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f;
    }

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    if(CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON))
    {
        value = CTC_FLAG_ISSET(p_hash_ctl->common_flag, CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO) ? 1 : 0;
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_deviceIdHashEn_f + step1, &outer_ctl, value);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_deviceIdHashEn_f + step2, &inner_ctl, value);
    }
    if(CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF))
    {
        value = 0;
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_udfInfoEn_f + step1, &outer_ctl, value);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_udfInfoEn_f + step2, &inner_ctl, value);

        CTC_ERROR_RETURN(sys_usw_acl_get_udf_hit_index(lchip, p_hash_ctl->udf_id, &udf_hit_index));

        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gUdfMask_0_udfBitmapEn_f + step4 + udf_hit_index*step3,
            &outer_ctl, p_hash_ctl->udf_bitmap);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f + step4 + udf_hit_index*step3,
            &inner_ctl, p_hash_ctl->udf_bitmap);
    }

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    }

    return CTC_E_NONE;
}
/**
 @brief get parser common hash info
*/
STATIC int32
_sys_usw_parser_get_common_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    int32  step1 = 0;
    int32  step2 = 0;
    int32  step3 = 0;
    uint32 value = 0;
    uint8  udf_hit_index = 0;
    uint32 tbl_id = 0;
    uint32 udf_bitmap_field = 0;
    uint32 device_id_field = 0;
    ds_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    step2 = IpeAclGenHashKeyCtl_gUdfMask_1_udfBitmapEn_f - IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f;
    if (SYS_PARSER_HASH_USAGE_LINKAGG == hash_usage)
    {
        step3 = IpeAclGenHashKeyCtl_gUdfMask_16_udfBitmapEn_f - IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f;
    }

    tbl_id = (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER) ? IpeAclGenHashKeyCtl_t: IpeUserIdHashCtl_t;
    step1 = _sys_usw_parser_hash_table_step(tbl_id, hash_usage);
    if(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, (IpeAclGenHashKeyCtl_m*)&hash_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, (IpeUserIdHashCtl_m*)&hash_ctl));
    }
    if(CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF))
    {
        udf_bitmap_field = (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)?
            (IpeAclGenHashKeyCtl_gUdfMask_0_udfBitmapEn_f + step3):(IpeUserIdHashCtl_gUdfMask_0_udfBitmapEn_f + step3);
        CTC_ERROR_RETURN(sys_usw_acl_get_udf_hit_index(lchip, p_hash_ctl->udf_id, &udf_hit_index));

        DRV_GET_FIELD_A(lchip, tbl_id, udf_bitmap_field + udf_hit_index*step2, &hash_ctl, &value);
        p_hash_ctl->udf_bitmap = value;
    }
    if(CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON))
    {
        device_id_field = (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_INNER)?
        (IpeAclGenHashKeyCtl_gMask_0_deviceIdHashEn_f + step1):(IpeUserIdHashCtl_gMask_0_deviceIdHashEn_f + step1);

        DRV_GET_FIELD_A(lchip, tbl_id, device_id_field, &hash_ctl, &value);
        CTC_SET_FLAG(p_hash_ctl->common_flag, (value ? CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO : 0));
    }

    return CTC_E_NONE;

}
/*
  hash_type:
      Refer to sys_parser_hash_usage_t, support ecmp, linkagg, flow mask.
  pkt_field_type:
      Represent which kind of mask need to set,   0  : macsa; 1 :macda, 2: ipsa ; 3:ipda
  sel_bitmap:
      Totally has 16 bit, each bit control 8bit of data.
*/
extern int32
sys_usw_parser_set_hash_mask(uint8 lchip, uint8 hash_type, uint8 pkt_field_type, uint16 sel_bitmap)
{
    int32  step1 = 0;
    int32  step2 = 0;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "key_type:0x%X. hash_usage:0x%X. sel_bitmap:0x%X.\n", pkt_field_type, hash_type, sel_bitmap);

    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
    }

    CTC_MAX_VALUE_CHECK(pkt_field_type, 3);
    CTC_MAX_VALUE_CHECK(hash_type, SYS_PARSER_HASH_USAGE_NUM - 1);

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));

    step1 = _sys_usw_parser_hash_table_step(IpeUserIdHashCtl_t, hash_type);
    step2 = _sys_usw_parser_hash_table_step(IpeAclGenHashKeyCtl_t, hash_type);

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    switch(pkt_field_type)
    {
    case 0:
        CTC_MAX_VALUE_CHECK(sel_bitmap, 63);
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macSaHashEn_f + step1, &outer_ctl, sel_bitmap);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macSaHashEn_f + step2, &inner_ctl, sel_bitmap);
        break;
    case 1:
        CTC_MAX_VALUE_CHECK(sel_bitmap, 63);
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_macDaHashEn_f + step1, &outer_ctl, sel_bitmap);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_macDaHashEn_f + step2, &inner_ctl, sel_bitmap);
        break;
    case 2:
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipSaHashEn_f + step1, &outer_ctl, sel_bitmap);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipSaHashEn_f + step2, &inner_ctl, sel_bitmap);
        break;
    case 3:
        DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_gMask_0_ipDaHashEn_f + step1, &outer_ctl, sel_bitmap);
        DRV_SET_FIELD_V(lchip, IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_gMask_0_ipDaHashEn_f + step2, &inner_ctl, sel_bitmap);
        break;
    default:
        return CTC_E_NOT_SUPPORT;
    }
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    return CTC_E_NONE;
}

/**
 @brief set parser trill header info
*/
int32
sys_usw_parser_set_trill_header(uint8 lchip, ctc_parser_trill_header_t* p_trill_header)
{

    ParserTrillCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_trill_header);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "inner_tpid:0x%X.\n", p_trill_header->inner_tpid);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "rbridge_channel_ether_type:0x%X.\n",\
                                              p_trill_header->rbridge_channel_ether_type);

    sal_memset(&ctl, 0, sizeof(ParserTrillCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_trill_ctl(lchip, &ctl));

    SetParserTrillCtl(V, trillInnerTpid_f, &ctl, p_trill_header->inner_tpid);
    SetParserTrillCtl(V, rBridgeChannelEtherType_f,\
                        &ctl, p_trill_header->rbridge_channel_ether_type);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_trill_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

/**
 @brief get parser trill header info
*/
int32
sys_usw_parser_get_trill_header(uint8 lchip, ctc_parser_trill_header_t* p_trill_header)
{

    uint32 value = 0;
    ParserTrillCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_trill_header);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ctl, 0, sizeof(ParserTrillCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_trill_ctl(lchip, &ctl));

    GetParserTrillCtl(A, trillInnerTpid_f, &ctl, &value);
    p_trill_header->inner_tpid = value;
    GetParserTrillCtl(A, rBridgeChannelEtherType_f, &ctl, &value);
    p_trill_header->rbridge_channel_ether_type = value;

    return CTC_E_NONE;
}

/**
 @brief mapping l4type,can add a new l3type,addition offset for the type,can get the layer4 type
*/
int32
sys_usw_parser_mapping_l4_type(uint8 lchip, uint8 index, ctc_parser_l3_protocol_entry_t* p_entry)
{

    uint32 value = 0;
    ParserLayer3ProtocolCam_m cam;
    ParserLayer3ProtocolCamValid_m valid;

    CTC_PTR_VALID_CHECK(p_entry);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:0x%X.\n", index);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3_header_protocol:0x%X.\n", p_entry->l3_header_protocol);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3_type:0x%X.\n", p_entry->l3_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l4_type:0x%X.\n", p_entry->l4_type);

#if 0
    SYS_PAS_CAM_INDEX_CHECK(entry->l3_type, CTC_PARSER_L3_TYPE_RSV_USER_DEFINE0,
                            MAX_CTC_PARSER_L3_TYPE);

    SYS_PAS_CAM_INDEX_CHECK(entry->l4_type, CTC_PARSER_L4_TYPE_RSV_USER_DEFINE0,
                            MAX_CTC_PARSER_L4_TYPE);
#endif

    if ((p_entry->addition_offset > 0xF)
       || (p_entry->l3_type > 0xF) || (p_entry->l3_type_mask > 0xF)
       || (p_entry->l4_type > 0xF))
    {
        return CTC_E_INVALID_PARAM;
    }

#if 0
    if ((entry->l4_type <= CTC_PARSER_L4_TYPE_IGMP)
        || (entry->l4_type == CTC_PARSER_L4_TYPE_ANY_PROTO))
    {
        return CTC_E_INVALID_PARAM;
    }
#endif

    if (index > (MCHIP_CAP(SYS_CAP_PARSER_L3_PROTOCOL_USER_ENTRY) - 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(ParserLayer3ProtocolCam_m));
    sal_memset(&valid, 0, sizeof(ParserLayer3ProtocolCamValid_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer3_protocol_cam(lchip, &cam));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer3_protocol_cam_valid(lchip, &valid));

    switch (index)
    {
    case 0:
        SetParserLayer3ProtocolCam(V, array_0_l3CamAdditionalOffset_f,\
                            &cam, p_entry->addition_offset);
        SetParserLayer3ProtocolCam(V, array_0_l3CamLayer3HeaderProtocol_f,\
                           &cam, p_entry->l3_header_protocol);
        SetParserLayer3ProtocolCam(V, array_0_l3CamLayer3HeaderProtocolMask_f,\
                            &cam, (p_entry->l3_header_protocol_mask & 0xff));
        SetParserLayer3ProtocolCam(V, array_0_l3CamLayer3Type_f,\
                            &cam, p_entry->l3_type);
        SetParserLayer3ProtocolCam(V, array_0_l3CamLayer3TypeMask_f,\
                            &cam, (p_entry->l3_type_mask & 0xf));
        SetParserLayer3ProtocolCam(V, array_0_l3CamLayer4Type_f,\
                            &cam, (p_entry->l4_type));
        break;

    case 1:
        SetParserLayer3ProtocolCam(V, array_1_l3CamAdditionalOffset_f,\
                            &cam, p_entry->addition_offset);
        SetParserLayer3ProtocolCam(V, array_1_l3CamLayer3HeaderProtocol_f,\
                            &cam, p_entry->l3_header_protocol);
        SetParserLayer3ProtocolCam(V, array_1_l3CamLayer3HeaderProtocolMask_f,\
                            &cam, (p_entry->l3_header_protocol_mask & 0xff));
        SetParserLayer3ProtocolCam(V, array_1_l3CamLayer3Type_f,\
                            &cam, p_entry->l3_type);
        SetParserLayer3ProtocolCam(V, array_1_l3CamLayer3TypeMask_f,\
                            &cam, (p_entry->l3_type_mask & 0xf));
        SetParserLayer3ProtocolCam(V, array_1_l3CamLayer4Type_f,\
                            &cam, (p_entry->l4_type));
        break;

    case 2:
        SetParserLayer3ProtocolCam(V, array_2_l3CamAdditionalOffset_f,\
                            &cam, p_entry->addition_offset);
        SetParserLayer3ProtocolCam(V, array_2_l3CamLayer3HeaderProtocol_f,\
                            &cam, p_entry->l3_header_protocol);
        SetParserLayer3ProtocolCam(V, array_2_l3CamLayer3HeaderProtocolMask_f,\
                            &cam, (p_entry->l3_header_protocol_mask & 0xff));
        SetParserLayer3ProtocolCam(V, array_2_l3CamLayer3Type_f,\
                            &cam, p_entry->l3_type);
        SetParserLayer3ProtocolCam(V, array_2_l3CamLayer3TypeMask_f,\
                            &cam, (p_entry->l3_type_mask & 0xf));
        SetParserLayer3ProtocolCam(V, array_2_l3CamLayer4Type_f,\
                            &cam, (p_entry->l4_type));
        break;

    case 3:
        SetParserLayer3ProtocolCam(V, array_3_l3CamAdditionalOffset_f,\
                            &cam, p_entry->addition_offset);
        SetParserLayer3ProtocolCam(V, array_3_l3CamLayer3HeaderProtocol_f,\
                            &cam, p_entry->l3_header_protocol);
        SetParserLayer3ProtocolCam(V, array_3_l3CamLayer3HeaderProtocolMask_f,\
                            &cam, (p_entry->l3_header_protocol_mask & 0xff));
        SetParserLayer3ProtocolCam(V, array_3_l3CamLayer3Type_f,\
                            &cam, p_entry->l3_type);
        SetParserLayer3ProtocolCam(V, array_3_l3CamLayer3TypeMask_f,\
                            &cam, (p_entry->l3_type_mask));
        SetParserLayer3ProtocolCam(V, array_3_l3CamLayer4Type_f,\
                            &cam, (p_entry->l4_type));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    GetParserLayer3ProtocolCamValid(A, layer3CamEntryValid_f, &valid, &value);
    value |= (1 << index);
    SetParserLayer3ProtocolCamValid(V, layer3CamEntryValid_f,\
                        &valid, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_cam(lchip, &cam));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_cam_valid(lchip, &valid));

    return CTC_E_NONE;
}

/**
 @brief set the entry invalid based on the index
*/
int32
sys_usw_parser_unmapping_l4_type(uint8 lchip, uint8 index)
{

    uint32 value = 0;
    ParserLayer3ProtocolCamValid_m valid;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:0x%X.\n", index);

    if (index > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&valid, 0, sizeof(ParserLayer3ProtocolCamValid_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer3_protocol_cam_valid(lchip, &valid));

    GetParserLayer3ProtocolCamValid(A, layer3CamEntryValid_f, &valid, &value);

    if (CTC_IS_BIT_SET(value, index))
    {
        value &= ~(1 << index);
        SetParserLayer3ProtocolCamValid(V, layer3CamEntryValid_f,\
                            &valid, value);
        CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_cam_valid(lchip, &valid));
    }

    return CTC_E_NONE;
}

/**
 @brief set parser range op ctl
*/
int32
sys_usw_parser_set_range_op_ctl(uint8 lchip, uint8 range_id, sys_parser_range_op_ctl_t* p_range_ctl)
{
    uint32 cmd  = 0;
    uint8  step = 0;
    ParserRangeOpCtl_m range_ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "range id:%d\n", range_id);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "type:%d, min: %u, max: %u\n",
                                p_range_ctl->type, p_range_ctl->min_value, p_range_ctl->max_value);
    CTC_PTR_VALID_CHECK(p_range_ctl);
    sal_memset(&range_ctl, 0, sizeof(ParserRangeOpCtl_m));

    step = ParserRangeOpCtl_array_1_maxValue_f - ParserRangeOpCtl_array_0_maxValue_f;
    cmd = DRV_IOR(ParserRangeOpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &range_ctl));

    DRV_SET_FIELD_V(lchip, ParserRangeOpCtl_t, ParserRangeOpCtl_array_0_opType_f + step * range_id, &range_ctl, p_range_ctl->type);
    DRV_SET_FIELD_V(lchip, ParserRangeOpCtl_t, ParserRangeOpCtl_array_0_minValue_f + step * range_id, &range_ctl, p_range_ctl->min_value);
    DRV_SET_FIELD_V(lchip, ParserRangeOpCtl_t, ParserRangeOpCtl_array_0_maxValue_f + step * range_id, &range_ctl, p_range_ctl->max_value);

    cmd = DRV_IOW(ParserRangeOpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &range_ctl));

    return CTC_E_NONE;
}

/**
 @brief set parser l4flex header info
*/
int32
sys_usw_parser_set_l4flex_header(uint8 lchip, ctc_parser_l4flex_header_t* p_l4flex_header)
{

    ParserLayer4FlexCtl_m ctl;

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dest_port_basic_offset:%d\n", p_l4flex_header->dest_port_basic_offset);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l4_min_len:%d\n", p_l4flex_header->l4_min_len);

    CTC_PTR_VALID_CHECK(p_l4flex_header);

    if ((p_l4flex_header->dest_port_basic_offset > 30)
       || (p_l4flex_header->l4_min_len > 0x1f))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ctl, 0, sizeof(ParserLayer4FlexCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer4_flex_ctl(lchip, &ctl));

    SetParserLayer4FlexCtl(V, layer4ByteSelect0_f,\
                        &ctl, p_l4flex_header->dest_port_basic_offset);

    SetParserLayer4FlexCtl(V, layer4ByteSelect1_f,\
                        &ctl, p_l4flex_header->dest_port_basic_offset + 1);

    SetParserLayer4FlexCtl(V, layer4MinLength_f,\
                        &ctl, p_l4flex_header->l4_min_len);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer4_flex_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

/**
 @brief get parser l4flex header info
*/
int32
sys_usw_parser_get_l4flex_header(uint8 lchip, ctc_parser_l4flex_header_t* p_l4flex_header)
{

    uint32 value = 0;
    ParserLayer4FlexCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_l4flex_header);

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dest_port_basic_offset:0x%X.\n", p_l4flex_header->dest_port_basic_offset);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l4_min_len:0x%X.\n", p_l4flex_header->l4_min_len);

    sal_memset(&ctl, 0, sizeof(ParserLayer4FlexCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer4_flex_ctl(lchip, &ctl));

    GetParserLayer4FlexCtl(A, layer4ByteSelect0_f, &ctl, &value);
    p_l4flex_header->dest_port_basic_offset = value;

    GetParserLayer4FlexCtl(A, layer4MinLength_f, &ctl, &value);
    p_l4flex_header->l4_min_len = value;

    return CTC_E_NONE;
}

/**
 @brief set parser layer4 app control info
*/
int32
sys_usw_parser_set_l4app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* p_l4app_ctl)
{

    ParserLayer4AppCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_l4app_ctl);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ptp_en:0x%x.\n", p_l4app_ctl->ptp_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ntp_en:0x%X.\n", p_l4app_ctl->ntp_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "bfd_en:0x%X.\n", p_l4app_ctl->bfd_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vxlan_en:0x%X.\n", p_l4app_ctl->vxlan_en);

    sal_memset(&ctl, 0, sizeof(ParserLayer4AppCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer4_app_ctl(lchip, &ctl));

    SetParserLayer4AppCtl(V, ptpEn_f, &ctl, (p_l4app_ctl->ptp_en ? 1 : 0));
    SetParserLayer4AppCtl(V, ntpEn_f, &ctl, (p_l4app_ctl->ntp_en ? 1 : 0));
    SetParserLayer4AppCtl(V, bfdEn_f, &ctl, (p_l4app_ctl->bfd_en ? 1 : 0));
    SetParserLayer4AppCtl(V, vxlanEn_f, &ctl, (p_l4app_ctl->vxlan_en ? 1 : 0));

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer4_app_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

/**
 @brief get parser layer4 app control info
*/
int32
sys_usw_parser_get_l4app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* p_l4app_ctl)
{
    uint32 value = 0;
    ParserLayer4AppCtl_m ctl;

    CTC_PTR_VALID_CHECK(p_l4app_ctl);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ptp_en:0x%X.\n", p_l4app_ctl->ptp_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ntp_en:0x%X.\n", p_l4app_ctl->ntp_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "bfd_en:0x%X.\n", p_l4app_ctl->bfd_en);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vxlan_en:0x%X.\n", p_l4app_ctl->vxlan_en);

    sal_memset(&ctl, 0, sizeof(ParserLayer4AppCtl_m));

    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_layer4_app_ctl(lchip, &ctl));

    GetParserLayer4AppCtl(A, ptpEn_f, &ctl, &value);
    p_l4app_ctl->ptp_en = value;

    GetParserLayer4AppCtl(A, ntpEn_f, &ctl, &value);
    p_l4app_ctl->ntp_en = value;

    GetParserLayer4AppCtl(A, bfdEn_f, &ctl, &value);
    p_l4app_ctl->bfd_en = value;

    GetParserLayer4AppCtl(A, vxlanEn_f, &ctl, &value);
    p_l4app_ctl->vxlan_en = value;

    return CTC_E_NONE;
}

/**
 @brief set ecmp hash field
*/
int32
sys_usw_parser_set_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_layer2_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_ip_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_l4_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_mpls_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_PBB))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_pbb_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_fcoe_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_trill_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON) ||
        CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_common_hash(lchip, p_hash_ctl, hash_usage));
    }

    return CTC_E_NONE;
}

/**
 @brief set efd hash field
*/
int32
sys_usw_parser_set_efd_hash_field(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);
    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_efd_layer2_hash(lchip, p_hash_ctl));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_efd_ip_hash(lchip, p_hash_ctl));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_set_efd_l4_hash(lchip, p_hash_ctl));
    }

    return CTC_E_NONE;
}

/**
 @brief set ecmp hash field
*/
int32
sys_usw_parser_set_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* p_hash_ctl)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);
    CTC_MAX_VALUE_CHECK(p_hash_ctl->udf_bitmap, 0xffff);

    if (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_DLB_EFD)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_set_hash_field(lchip, (ctc_parser_flow_hash_ctl_t*)p_hash_ctl,
                                                          SYS_PARSER_HASH_USAGE_FLOW));
        CTC_ERROR_RETURN(sys_usw_parser_set_efd_hash_field(lchip, (ctc_parser_efd_hash_ctl_t*)p_hash_ctl));
    }
    else if(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_EFD)
    {
        CTC_ERROR_RETURN(sys_usw_parser_set_efd_hash_field(lchip, (ctc_parser_efd_hash_ctl_t*)p_hash_ctl));
    }
    else if(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_DLB)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_set_hash_field(lchip, (ctc_parser_flow_hash_ctl_t*)p_hash_ctl,
                                                          SYS_PARSER_HASH_USAGE_FLOW));
    }
    else
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_set_hash_field(lchip, (ctc_parser_ecmp_hash_ctl_t*)p_hash_ctl,
                                                               SYS_PARSER_HASH_USAGE_ECMP));
    }

    return CTC_E_NONE;
}

/**
 @brief set ecmp hash field
*/
int32
sys_usw_parser_set_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);
    if (NULL == MCHIP_API(lchip) || NULL == MCHIP_API(lchip)->parser_set_hash_field)
    {
        return CTC_E_NOT_SUPPORT;
    }
    CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_set_hash_field(lchip, (ctc_parser_linkagg_hash_ctl_t*)p_hash_ctl,
                                                             SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

/**
 @brief get ecmp hash field
*/
int32
sys_usw_parser_get_hash_field(uint8 lchip, ctc_parser_hash_ctl_t* p_hash_ctl, sys_parser_hash_usage_t hash_usage)
{
    CTC_PTR_VALID_CHECK(p_hash_ctl);

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_layer2_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_ip_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_l4_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_mpls_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_PBB))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_pbb_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_fcoe_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_trill_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON) ||
        CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_common_hash(lchip, p_hash_ctl, hash_usage));
    }

    return CTC_E_NONE;
}

/**
 @brief get efd hash field
*/
int32
sys_usw_parser_get_efd_hash_field(uint8 lchip, ctc_parser_efd_hash_ctl_t* p_hash_ctl)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);
    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_efd_layer2_hash(lchip, p_hash_ctl));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_efd_ip_hash(lchip, p_hash_ctl));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        CTC_ERROR_RETURN(_sys_usw_parser_get_efd_l4_hash(lchip, p_hash_ctl));
    }

    return CTC_E_NONE;
}
/**
 @brief get ecmp hash field
*/
int32
sys_usw_parser_get_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* p_hash_ctl)
{
    CTC_PTR_VALID_CHECK(p_hash_ctl);

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_DLB_EFD)||
        (p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_DLB))
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_get_hash_field(lchip, (ctc_parser_flow_hash_ctl_t*)p_hash_ctl,
                                                          SYS_PARSER_HASH_USAGE_FLOW));
    }
    else if(p_hash_ctl->hash_type_bitmap & CTC_PARSER_HASH_TYPE_FLAGS_EFD)
    {
        CTC_ERROR_RETURN(sys_usw_parser_get_efd_hash_field(lchip, (ctc_parser_efd_hash_ctl_t*)p_hash_ctl));
    }
    else
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_get_hash_field(lchip, (ctc_parser_ecmp_hash_ctl_t*)p_hash_ctl,
                                                               SYS_PARSER_HASH_USAGE_ECMP));
    }

    return CTC_E_NONE;
}

/**
 @brief get linkagg hash field
*/
int32
sys_usw_parser_get_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl)
{
    CTC_PTR_VALID_CHECK(p_hash_ctl);
	 if (NULL == MCHIP_API(lchip) || NULL == MCHIP_API(lchip)->parser_set_hash_field)
    {
        return CTC_E_NOT_SUPPORT;
    }
    CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_get_hash_field(lchip, (ctc_parser_linkagg_hash_ctl_t*)p_hash_ctl,
                                                                SYS_PARSER_HASH_USAGE_LINKAGG));
    return CTC_E_NONE;
}

/**
 @brief set parser global config info
*/
int32
sys_usw_parser_set_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* p_global_cfg)
{
    uint32 cmd;
    uint32 value = 0;
    uint32 merge = 0;
    uint32 inner = 0;
    uint8 i = 0;
    ParserIpCtl_m ip_ctl;
    ParserLayer3HashCtl_m layer3_ctl;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    CTC_PTR_VALID_CHECK(p_global_cfg);

    /* ipv4 small fragment offset, 0~3,means 0,8,16,24 bytes of small fragment length*/
    if (p_global_cfg->small_frag_offset > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ecmp_hash_type:%d\n", p_global_cfg->ecmp_hash_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkagg_hash_type:%d\n", p_global_cfg->linkagg_hash_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "small_frag_offset:%d\n", p_global_cfg->small_frag_offset);

    sal_memset(&layer3_ctl, 0, sizeof(ParserLayer3HashCtl_m));
    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_l3_hash_ctl_entry(lchip, &layer3_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    value = (CTC_PARSER_GEN_HASH_TYPE_XOR == p_global_cfg->ecmp_hash_type)\
            ? SYS_USW_PARSER_GEN_HASH_TYPE_XOR : SYS_USW_PARSER_GEN_HASH_TYPE_CRC;
    SetIpeUserIdHashCtl(V, ecmpHashType_f, &outer_ctl, value);
    SetIpeAclGenHashKeyCtl(V, ecmpHashType_f, &inner_ctl, value);
    SetParserLayer3HashCtl(V, layer3HashType_f, &layer3_ctl, value);

    value = (CTC_PARSER_GEN_HASH_TYPE_XOR == p_global_cfg->linkagg_hash_type)\
            ? SYS_USW_PARSER_GEN_HASH_TYPE_XOR : SYS_USW_PARSER_GEN_HASH_TYPE_CRC;
    SetIpeUserIdHashCtl(V, linkaggHashType_f, &outer_ctl, value);
    SetIpeAclGenHashKeyCtl(V, linkaggHashType_f, &inner_ctl, value);

    /* config tunnel hash key select */
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (p_global_cfg->ecmp_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_MERGE)
        {
            merge |= (1 << i);
        }
        else if (p_global_cfg->ecmp_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_INNER)
        {
            inner |= (1 << i);
        }
    }
    SetIpeAclGenHashKeyCtl(V, gMask_0_mergeOuterHdrHashBitmap_f, &inner_ctl, merge);
    SetIpeAclGenHashKeyCtl(V, gMask_0_useInnerHdrHashBitmap_f, &inner_ctl, inner);

    merge = 0;
    inner = 0;
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (p_global_cfg->linkagg_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_MERGE)
        {
            merge |= (1 << i);
        }
        else if (p_global_cfg->linkagg_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_INNER)
        {
            inner |= (1 << i);
        }
    }
    SetIpeAclGenHashKeyCtl(V, gMask_1_mergeOuterHdrHashBitmap_f, &inner_ctl, merge);
    SetIpeAclGenHashKeyCtl(V, gMask_1_useInnerHdrHashBitmap_f, &inner_ctl, inner);

    merge = 0;
    inner = 0;
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (p_global_cfg->dlb_efd_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_MERGE)
        {
            merge |= (1 << i);
        }
        else if (p_global_cfg->dlb_efd_tunnel_hash_mode[i] == CTC_PARSER_TUNNEL_HASH_MODE_INNER)
        {
            inner |= (1 << i);
        }
    }
    SetIpeAclGenHashKeyCtl(V, gMask_2_mergeOuterHdrHashBitmap_f, &inner_ctl, merge);
    SetIpeAclGenHashKeyCtl(V, gMask_2_useInnerHdrHashBitmap_f, &inner_ctl, inner);


    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_l3_hash_ctl_entry(lchip, &layer3_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));

    sal_memset(&ip_ctl, 0, sizeof(ParserIpCtl_m));
    SetParserIpCtl(V, smallFragmentOffset_f, &ip_ctl, p_global_cfg->small_frag_offset);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_ip_ctl(lchip, &ip_ctl));

    if(p_global_cfg->symmetric_hash_en)
    {
        value = 1;
    }
    else
    {
        value = 0;
    }
    cmd = DRV_IOW(IpeUserIdHashCtl_t, IpeUserIdHashCtl_v4AddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeUserIdHashCtl_t, IpeUserIdHashCtl_v6AddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeUserIdHashCtl_t, IpeUserIdHashCtl_ipSymmetricHashMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeUserIdHashCtl_t, IpeUserIdHashCtl_fcoeAddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_v4AddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_v6AddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeAclGenHashKeyCtl_t, IpeAclGenHashKeyCtl_ipSymmetricHashMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    return CTC_E_NONE;
}

/**
 @brief get parser global config info
*/
int32
sys_usw_parser_get_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* p_global_cfg)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 merge = 0;
    uint32 inner = 0;
    uint8 i = 0;
    ParserIpCtl_m ip_ctl;
    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    CTC_PTR_VALID_CHECK(p_global_cfg);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ecmp_hash_type:%d\n", p_global_cfg->ecmp_hash_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkagg_hash_type:%d\n", p_global_cfg->linkagg_hash_type);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "small_frag_offset:%d\n", p_global_cfg->small_frag_offset);

    sal_memset(&ip_ctl, 0, sizeof(ParserIpCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_ip_ctl(lchip, &ip_ctl));

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    GetParserIpCtl(A, smallFragmentOffset_f, &ip_ctl, &value);
    p_global_cfg->small_frag_offset = value;

    GetIpeUserIdHashCtl(A, ecmpHashType_f, &outer_ctl, &value);
    p_global_cfg->ecmp_hash_type = (SYS_USW_PARSER_GEN_HASH_TYPE_CRC == value) \
                                   ? CTC_PARSER_GEN_HASH_TYPE_CRC : CTC_PARSER_GEN_HASH_TYPE_XOR;

    GetIpeUserIdHashCtl(A, linkaggHashType_f, &outer_ctl, &value);
    p_global_cfg->linkagg_hash_type = (SYS_USW_PARSER_GEN_HASH_TYPE_CRC == value) \
                                      ? CTC_PARSER_GEN_HASH_TYPE_CRC : CTC_PARSER_GEN_HASH_TYPE_XOR;

    /* get tunnel hash key select */
    merge = GetIpeAclGenHashKeyCtl(V, gMask_0_mergeOuterHdrHashBitmap_f, &inner_ctl);
    inner = GetIpeAclGenHashKeyCtl(V, gMask_0_useInnerHdrHashBitmap_f, &inner_ctl);
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (merge & (1<<i))
        {
            p_global_cfg->ecmp_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;
        }
        else if (inner & (1<<i))
        {
            p_global_cfg->ecmp_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_INNER;
        }
    }

    merge = GetIpeAclGenHashKeyCtl(V, gMask_1_mergeOuterHdrHashBitmap_f, &inner_ctl);
    inner = GetIpeAclGenHashKeyCtl(V, gMask_1_useInnerHdrHashBitmap_f, &inner_ctl);
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (merge & (1<<i))
        {
            p_global_cfg->linkagg_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;
        }
        else if (inner & (1<<i))
        {
            p_global_cfg->linkagg_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_INNER;
        }
    }

    merge = GetIpeAclGenHashKeyCtl(V, gMask_2_mergeOuterHdrHashBitmap_f, &inner_ctl);
    inner = GetIpeAclGenHashKeyCtl(V, gMask_2_useInnerHdrHashBitmap_f, &inner_ctl);
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i ++)
    {
        if (merge & (1<<i))
        {
            p_global_cfg->dlb_efd_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;
        }
        else if (inner & (1<<i))
        {
            p_global_cfg->dlb_efd_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_INNER;
        }
    }

    cmd = DRV_IOR(IpeUserIdHashCtl_t, IpeUserIdHashCtl_v4AddrSymmetricHashEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    if(value == 1)
    {
        p_global_cfg->symmetric_hash_en = 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_ethernet_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;

    EpeL2TpidCtl_m tpid_ctl;
    EpeHdrAdjustCtl_m adjust_ctl;
    IpeUserIdHashCtl_m outer_ctl;
    ParserEthernetCtl_m ethernet_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;
    hw_mac_addr_t hw_mac;

    sal_memset(&hw_mac, 0, sizeof(hw_mac_addr_t));
    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    SetIpeUserIdHashCtl(V, gMask_0_cVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_cCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_cCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_sVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_sCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_sCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_l2ProtocolHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_portHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_macSaHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_0_macDaHashEn_f, &outer_ctl, 0);

    SetIpeUserIdHashCtl(V, gMask_1_cVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_cCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_cCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_sVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_sCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_sCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_l2ProtocolHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_portHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_macSaHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_1_macDaHashEn_f, &outer_ctl, 0);

    SetIpeUserIdHashCtl(V, gMask_2_cVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_cCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_cCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_sVlanIdHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_sCosHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_sCfiHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_l2ProtocolHashEn_f, &outer_ctl, 1);
    SetIpeUserIdHashCtl(V, gMask_2_portHashEn_f, &outer_ctl, 1);
    SetIpeUserIdHashCtl(V, gMask_2_macSaHashEn_f, &outer_ctl, 0);
    SetIpeUserIdHashCtl(V, gMask_2_macDaHashEn_f, &outer_ctl, 0);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_inner_hash_ctl(lchip, &inner_ctl));

    SetIpeAclGenHashKeyCtl(V, gMask_0_vlanIdHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_0_cfiHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_0_cosHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_0_etherTypeHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_0_macDaHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_0_macSaHashEn_f, &inner_ctl, 0);

    SetIpeAclGenHashKeyCtl(V, gMask_1_vlanIdHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_1_cfiHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_1_cosHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_1_etherTypeHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_1_macDaHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_1_macSaHashEn_f, &inner_ctl, 0);

    SetIpeAclGenHashKeyCtl(V, gMask_2_vlanIdHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_2_cfiHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_2_cosHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_2_etherTypeHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_2_macDaHashEn_f, &inner_ctl, 0);
    SetIpeAclGenHashKeyCtl(V, gMask_2_macSaHashEn_f, &inner_ctl, 0);

    sal_memset(&ethernet_ctl, 0, sizeof(ParserEthernetCtl_m));
    SetParserEthernetCtl(V, cvlanTpid_f, &ethernet_ctl, 0x8100);
    SetParserEthernetCtl(V, allowNonZeroOui_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, ieee1588TypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, slowProtocolTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, pbbTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, satPduTypeEn_f, &ethernet_ctl, 1);
	SetParserEthernetCtl(V, rarpTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, ethOamTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, fcoeTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, trillTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, mplsMcastTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, mplsTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, ipv4TypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, ipv6TypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, maxLengthField_f, &ethernet_ctl, 1536);
    SetParserEthernetCtl(V, parsingQuadVlan_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, vlanParsingNum_f, &ethernet_ctl, 2);
    SetParserEthernetCtl(V, array_0_svlanTpid_f, &ethernet_ctl, 0x8100);
    SetParserEthernetCtl(V, array_1_svlanTpid_f, &ethernet_ctl, 0x9100);
    SetParserEthernetCtl(V, array_2_svlanTpid_f, &ethernet_ctl, 0x88A8);
    SetParserEthernetCtl(V, array_3_svlanTpid_f, &ethernet_ctl, 0x88A8);
    SetParserEthernetCtl(V, arpTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(A, macSaMaskBits_f, &ethernet_ctl, hw_mac);
    SetParserEthernetCtl(V, nshSnoopEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, nshEtherType_f, &ethernet_ctl, 0x894F);
    SetParserEthernetCtl(V, supportMetaDataHeader_f, &ethernet_ctl, 0);
    SetParserEthernetCtl(V, sgtOptionCheckEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, sgtOptionData_f, &ethernet_ctl, 0x0001); /*only check option type, not check option len*/
    SetParserEthernetCtl(V, sgtOptionMask_f, &ethernet_ctl, 0x0FFF);
    SetParserEthernetCtl(V, metaDataEtherType_f, &ethernet_ctl, 0x8909);
    SetParserEthernetCtl(V, metaDataTotalLenCheckEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, metaDataTotalLenValue_f, &ethernet_ctl, 1); /*4 byte units*/
    SetParserEthernetCtl(V, dot1AeTypeEn_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, ipLengthRangeMode_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, udfEn_f, &ethernet_ctl, 1);
	SetParserEthernetCtl(V, udfStartPosType0Mode_f, &ethernet_ctl, 1);
    SetParserEthernetCtl(V, udfStartPosTypeCheckEn_f, &ethernet_ctl, 1);
	SetParserEthernetCtl(V, udfLengthCheckMode_f, &ethernet_ctl, 1);
	SetParserEthernetCtl(V, sgtIdRangeCheckEn_f,&ethernet_ctl,1);
    SetParserEthernetCtl(V, sgtIdBase_f,&ethernet_ctl,0);
	SetParserEthernetCtl(V, unknownSgtId_f,&ethernet_ctl, CTC_ACL_UNKNOWN_CID);
    SetParserEthernetCtl(V, outerAlwaysCvlan_f,&ethernet_ctl, 0);/*Modified by yejl for TM vlanclass, 2019-07-24*/


    sal_memset(&adjust_ctl, 0, sizeof(EpeHdrAdjustCtl_m));
    cmd = DRV_IOR(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &adjust_ctl));

    SetEpeHdrAdjustCtl(V, vlanEditEn_f, &adjust_ctl, 1);
    SetEpeHdrAdjustCtl(V, cvlanTagTpid_f, &adjust_ctl, 0x8100);
    SetEpeHdrAdjustCtl(V, array_0_svlanTagTpid_f, &adjust_ctl, 0x8100);
    SetEpeHdrAdjustCtl(V, array_1_svlanTagTpid_f, &adjust_ctl, 0x9100);
    SetEpeHdrAdjustCtl(V, array_2_svlanTagTpid_f, &adjust_ctl, 0x88A8);
    SetEpeHdrAdjustCtl(V, array_3_svlanTagTpid_f, &adjust_ctl, 0x88A8);
    SetEpeHdrAdjustCtl(V, portExtenderMcastEn_f, &adjust_ctl, 0);
    SetEpeHdrAdjustCtl(V, packetHeaderBypassAll_f, &adjust_ctl, 0);

    sal_memset(&tpid_ctl, 0, sizeof(EpeL2TpidCtl_m));
    SetEpeL2TpidCtl(V, array_0_svlanTpid_f, &tpid_ctl, 0x8100);
    SetEpeL2TpidCtl(V, array_1_svlanTpid_f, &tpid_ctl, 0x9100);
    SetEpeL2TpidCtl(V, array_2_svlanTpid_f, &tpid_ctl, 0x88A8);
    SetEpeL2TpidCtl(V, array_3_svlanTpid_f, &tpid_ctl, 0x88A8);
    SetEpeL2TpidCtl(V, iTagTpid_f, &tpid_ctl, 0x88e7);
    SetEpeL2TpidCtl(V, cvlanTpid_f, &tpid_ctl, 0x8100);
    SetEpeL2TpidCtl(V, bvlanTpid_f, &tpid_ctl, 0x88a8);

    cmd = DRV_IOW(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ethernet_ctl));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &adjust_ctl));

    cmd = DRV_IOW(EpeL2TpidCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_layer2_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_layer2_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_layer2_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_ip_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA);
    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPDA);

    CTC_ERROR_RETURN(_sys_usw_parser_set_ip_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_ip_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l4_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_l4_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_l4_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_mpls_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_SET_FLAG(hash_ctl.mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPSA);
    CTC_SET_FLAG(hash_ctl.mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPDA);

    CTC_ERROR_RETURN(_sys_usw_parser_set_mpls_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_mpls_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_pbb_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_pbb_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_pbb_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_fcoe_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_fcoe_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_fcoe_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_trill_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_trill_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_trill_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_common_hash_init(uint8 lchip)
{
    ctc_parser_hash_ctl_t hash_ctl;
    uint32 value = 0;
    IpeUserIdHashCtl_m outer_ctl;

    /* init deviceId */
    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_get_parser_outer_hash_ctl(lchip, &outer_ctl));

    value = (uint8)sal_rand();
    DRV_SET_FIELD_V(lchip, IpeUserIdHashCtl_t, IpeUserIdHashCtl_deviceId_f, &outer_ctl, value);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    /* init common hash */
    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));

    CTC_ERROR_RETURN(_sys_usw_parser_set_common_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_ECMP));
    CTC_ERROR_RETURN(_sys_usw_parser_set_common_hash(lchip, &hash_ctl, SYS_PARSER_HASH_USAGE_LINKAGG));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l2pro_cam_init(uint8 lchip)
{

    ParserLayer2ProtocolCam_m cam;
    ParserLayer2ProtocolCamValid_m valid;

    sal_memset(&cam, 0, sizeof(ParserLayer2ProtocolCam_m));
    sal_memset(&valid, 0, sizeof(ParserLayer2ProtocolCamValid_m));

    /*
       ipv4, ipv6, mpls, mpls_mcast, arp, fcoe, trill, eth_oam, pbb, slow_protocol, 1588(ptp)
       defined in _cm_com_parser_l2_protocol_lookup function
       parser_layer2_protocol_cam only used to mapping 4 user defined layer3 protocol
    */
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(lchip, &valid));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_protocol_cam(lchip, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_pbb_ctl_init(uint8 lchip)
{

    ParserPbbCtl_m ctl;

    sal_memset(&ctl, 0, sizeof(ParserPbbCtl_m));
    SetParserPbbCtl(V, pbbVlanParsingNum_f, &ctl, SYS_PAS_PBB_VLAN_PAS_NUM);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_pbb_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l2flex_ctl_init(uint8 lchip)
{
#if 0

    parser_layer2_flex_ctl_m ctl;

    sal_memset(&ctl, 0, sizeof(parser_layer2_flex_ctl_m));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer2_flex_ctl(lchip, &ctl));
#endif

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_packet_type_map_init(uint8 lchip)
{

    ParserPacketTypeMap_m map;

    sal_memset(&map, 0, sizeof(ParserPacketTypeMap_m));
    /*
       ETHERNETV2, IPV4, MPLS, IPV6, MCAST_MPLS, TRILL
       defined in cm_com_parser_packet_parsing function
       parser_packet_type_map used to mapping 2 user defined type
    */
    SetParserPacketTypeMap(V, array_0_layer3Type_f, &map, CTC_PARSER_L3_TYPE_NONE);
    SetParserPacketTypeMap(V, array_1_layer3Type_f, &map, CTC_PARSER_L3_TYPE_NONE);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_packet_type(lchip, &map));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l3pro_cam_init(uint8 lchip)
{

    ParserLayer3ProtocolCam_m cam;
    ParserLayer3ProtocolCamValid_m valid;

    sal_memset(&cam, 0, sizeof(ParserLayer3ProtocolCam_m));
    sal_memset(&valid, 0, sizeof(ParserLayer3ProtocolCamValid_m));

    /*
      tcp, udp, gre, pbb_itag_oam, ipinip, v6inip, ipinv6, v6inv6, icmp, igmp, rdp , sctp, dccp
      is predefined in chip parser module.
      parser_layer3_protocol_cam only used to mapping 4 user defined layer4 protocol
   */
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_cam_valid(lchip, &valid));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_cam(lchip, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_hash_ctl_init(uint8 lchip)
{

    IpeUserIdHashCtl_m outer_ctl;
    IpeAclGenHashKeyCtl_m inner_ctl;

    sal_memset(&outer_ctl, 0, sizeof(IpeUserIdHashCtl_m));
    SetIpeUserIdHashCtl(V, ecmpHashType_f,
                        &outer_ctl, SYS_PAS_OUTER_ECMP_HASH_TYPE);
    SetIpeUserIdHashCtl(V, linkaggHashType_f,
                        &outer_ctl, SYS_PAS_OUTER_LINKAGG_HASH_TYPE);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &outer_ctl));

    sal_memset(&inner_ctl, 0, sizeof(IpeAclGenHashKeyCtl_m));
    /* flowid hash fixed use crc */
    SetIpeAclGenHashKeyCtl(V, ecmpHashType_f,
                        &inner_ctl, SYS_PAS_INNER_ECMP_HASH_TYPE);
    SetIpeAclGenHashKeyCtl(V, linkaggHashType_f,
                        &inner_ctl, SYS_PAS_INNER_ECMP_HASH_TYPE);

    /* always use merged key */
    SetIpeAclGenHashKeyCtl(V, gMask_0_mergeOuterHdrHashBitmap_f,
                        &inner_ctl, 0x1FF);
    SetIpeAclGenHashKeyCtl(V, gMask_1_mergeOuterHdrHashBitmap_f,
                        &inner_ctl, 0x1FF);
    SetIpeAclGenHashKeyCtl(V, gMask_2_mergeOuterHdrHashBitmap_f,
                        &inner_ctl, 0x1FF);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_inner_hash_ctl(lchip, &inner_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_mpls_ctl_init(uint8 lchip)
{

    IpeUserIdHashCtl_m ctl;

    sal_memset(&ctl, 0, sizeof(IpeUserIdHashCtl_m));
    SetIpeUserIdHashCtl(V, maxReserveLabel_f, &ctl, 0xF);
    SetIpeUserIdHashCtl(V, mplsEcmpUseReserveLabel_f, &ctl, 1);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_outer_hash_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_trill_ctl_init(uint8 lchip)
{

    ParserTrillCtl_m ctl;

    sal_memset(&ctl, 0, sizeof(ctl));
    SetParserTrillCtl(V, trillInnerTpid_f, &ctl, 0x8200);
    SetParserTrillCtl(V, innerVlanTpidMode_f, &ctl, 1);
    SetParserTrillCtl(V, rBridgeChannelEtherType_f, &ctl, 0x8946);
    SetParserTrillCtl(V, trillBfdOamChannelProtocol0_f, &ctl, 2);
    SetParserTrillCtl(V, trillBfdOamChannelProtocol1_f, &ctl, 0xFFF);
    SetParserTrillCtl(V, trillBfdEchoChannelProtocol_f, &ctl, 3);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_trill_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l3pro_ctl_init(uint8 lchip)
{

    ParserLayer3ProtocolCtl_m ctl;

    sal_memset(&ctl, 0, sizeof(ParserLayer3ProtocolCtl_m));

    SetParserLayer3ProtocolCtl(V, pbbItagOamTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, v6InV6TypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, ipInV6TypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, v6InIpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, ipInIpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, igmpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, icmpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, greTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, udpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, tcpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, rdpTypeEn_f, &ctl, 1);
    SetParserLayer3ProtocolCtl(V, sctpTypeEn_f, &ctl, 1);

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_protocol_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l3type_ctl_init(uint8 lchip)
{

    ParserL3Ctl_m ctl;

    sal_memset(&ctl, 0, sizeof(ParserL3Ctl_m));

    SetParserL3Ctl(V, l3TypePtpEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeTrillEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeFcoeEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeSlowProtocolEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeEtherOamEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeCmacEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeArpEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeMplsEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeIpv6En_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeIpv4En_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeDot1AeEn_f, &ctl, 1);
    SetParserL3Ctl(V, l3TypeSatPduEn_f, &ctl, 1);

    SetParserL3Ctl(V, dot1AeCheckBit_f, &ctl, 0x1F);

    /* support parser ipv6 ext header hopByHopLen > 8 byte */
    SetParserL3Ctl(V, hopByHopLenType_f, &ctl, 1);

/* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
    SetParserL3Ctl(V, layer3ParserLengthErrorIgnore_f, &ctl, 0xffff);
#endif  

    /* Not care index */
 /*-    SetParserL3Ctl(V, l3UdfCareIndex_f, &ctl, 0);*/

    /* Igs second udf parser has limit */
 /*-    SetParserL3Ctl(V, l3UdfEn_f, &ctl, (SYS_PARSER_TYPE_FLAG_IGS_OUTER | SYS_PARSER_TYPE_FLAG_EGS_OUTER));*/

    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer3_type_ctl(lchip, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_parser_l4_reg_ctl_init(uint8 lchip)
{
    ParserLayer4FlexCtl_m l4flex_ctl;
    ParserLayer4AppCtl_m l4app_ctl;
    ParserLayer4AchCtl_m l4ach_ctl;

    sal_memset(&l4flex_ctl, 0, sizeof(ParserLayer4FlexCtl_m));
    sal_memset(&l4app_ctl, 0, sizeof(ParserLayer4AppCtl_m));
    sal_memset(&l4ach_ctl, 0, sizeof(ParserLayer4AchCtl_m));

#if 0
    SetParserLayer4FlagOpCtl(V, opAndOr0_f, &l4flag_opctl, 0);
    SetParserLayer4FlagOpCtl(V, flagsMask0_f, &l4flag_opctl, 0);
    SetParserLayer4FlagOpCtl(V, opAndOr1_f, &l4flag_opctl, 0);
    SetParserLayer4FlagOpCtl(V, flagsMask1_f, &l4flag_opctl, 0x3f);
    SetParserLayer4FlagOpCtl(V, opAndOr2_f, &l4flag_opctl, 1);
    SetParserLayer4FlagOpCtl(V, flagsMask2_f, &l4flag_opctl, 0);
    SetParserLayer4FlagOpCtl(V, opAndOr2_f, &l4flag_opctl, 1);
    SetParserLayer4FlagOpCtl(V, flagsMask2_f, &l4flag_opctl, 0x3f);
#endif
    SetParserLayer4FlexCtl(V, layer4ByteSelect0_f, &l4flex_ctl, 0);
    SetParserLayer4FlexCtl(V, layer4ByteSelect1_f, &l4flex_ctl, 0);
    SetParserLayer4FlexCtl(V, layer4MinLength_f, &l4flex_ctl, 0);
    SetParserLayer4AppCtl(V, ptpEn_f, &l4app_ctl, 1);
    SetParserLayer4AppCtl(V, ntpEn_f, &l4app_ctl, 1);
    SetParserLayer4AppCtl(V, bfdEn_f, &l4app_ctl, 1);

    SetParserLayer4AppCtl(V, ntpPort_f, &l4app_ctl, 123);
    SetParserLayer4AppCtl(V, ptpPort0_f, &l4app_ctl, 320);
    SetParserLayer4AppCtl(V, ptpPort1_f, &l4app_ctl, 319);
    SetParserLayer4AppCtl(V, bfdPort0_f, &l4app_ctl, 3784);
    SetParserLayer4AppCtl(V, bfdPort1_f, &l4app_ctl, 4784);
    SetParserLayer4AppCtl(V, nshInTunnelSnoopEn_f, &l4app_ctl, 1);
    SetParserLayer4AppCtl(V, nshInTunnelEtherType_f, &l4app_ctl, 0x894F);
    SetParserLayer4AppCtl(V, nshInTunnelNextProtocol_f, &l4app_ctl, 4);

/* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
    SetParserLayer4AppCtl(V, layer4ParserLengthErrorIgnore_f, &l4app_ctl, 0xffff);
#endif

    /* Not care index */
 /*-    SetParserLayer4AppCtl(V, l4UdfCareIndex_f, &l4app_ctl, 0);*/
 /*-    SetParserLayer4AppCtl(V, l4UdfEn_f, &l4app_ctl, (SYS_PARSER_TYPE_FLAG_IGS_OUTER | SYS_PARSER_TYPE_FLAG_EGS_OUTER));*/

#if 0
    SetParserLayer4AppDataCtl(V, isTcpMask0_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isTcpMask1_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isTcpMask2_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isTcpMask3_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isUdpMask0_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isUdpMask1_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isUdpMask2_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, isUdpMask3_f, &app_data_ctl, 0x1);
    SetParserLayer4AppDataCtl(V, l4DestPortMask0_f, &app_data_ctl, 0xFFFF);
    SetParserLayer4AppDataCtl(V, l4DestPortMask1_f, &app_data_ctl, 0xFFFF);
    SetParserLayer4AppDataCtl(V, l4DestPortMask2_f, &app_data_ctl, 0xFFFF);
    SetParserLayer4AppDataCtl(V, l4DestPortMask3_f, &app_data_ctl, 0xFFFF);
#endif

    SetParserLayer4AchCtl(V, achY1731Type_f, &l4ach_ctl, 0x8902);
    SetParserLayer4AchCtl(V, achDmType_f, &l4ach_ctl, 0x000C);
    SetParserLayer4AchCtl(V, achDlmType_f, &l4ach_ctl, 0x000A);
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer4_flex_ctl(lchip, &l4flex_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer4_app_ctl(lchip, &l4app_ctl));
    CTC_ERROR_RETURN(sys_usw_parser_io_set_parser_layer4_ach_ctl(lchip, &l4ach_ctl));

    return CTC_E_NONE;
}

int32
sys_usw_parser_hash_init(uint8 lchip)
{
    CTC_ERROR_RETURN(_sys_usw_parser_layer2_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_ip_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l4_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_mpls_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_pbb_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_fcoe_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_trill_hash_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_common_hash_init(lchip));

    return CTC_E_NONE;

}

/**
 @brief init parser module
*/
int32
sys_usw_parser_init(uint8 lchip, void* p_parser_global_cfg)
{
    CTC_PTR_VALID_CHECK(p_parser_global_cfg);

    CTC_ERROR_RETURN(_sys_usw_parser_ethernet_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l2pro_cam_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_pbb_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l2flex_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_packet_type_map_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l3pro_cam_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_hash_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_mpls_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l3type_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_trill_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l3pro_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_parser_l4_reg_ctl_init(lchip));

    CTC_ERROR_RETURN(MCHIP_API(lchip)->parser_hash_init(lchip));

    CTC_ERROR_RETURN(sys_usw_parser_set_global_cfg(lchip, p_parser_global_cfg));

    return CTC_E_NONE;
}

/**
 @brief deinit parser module
*/
int32
sys_usw_parser_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    return CTC_E_NONE;
}


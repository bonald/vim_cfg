#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_oam_cli.c

 @date 2012-11-20

 @version v2.0

 This file defines functions for greatbelt oam cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_usw_oam_cli.h"
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_debug.h"



/*********************************************************************
* Internal debug clis
*********************************************************************/
/**
 @brief  debug functions
*/

extern int32 sys_usw_oam_show_oam_property(uint8 lchip, ctc_oam_property_t* p_prop);
extern int32 sys_usw_oam_internal_property(uint8 lchip);
extern int32 sys_usw_oam_internal_property_bfd_333ms(uint8 lchip);
extern int32 sys_usw_oam_internal_maid_property(uint8 lchip, ctc_oam_maid_len_format_t maid_len);
extern int32 sys_usw_oam_tp_section_init(uint8 lchip, uint8 use_port);
extern int32 sys_usw_oam_show_oam_status(uint8 lchip);
extern int32 sys_usw_oam_show_oam_defect_status(uint8 lchip);
extern int32 sys_usw_oam_show_oam_mep_detail(uint8 lchip, ctc_oam_key_t *p_oam_key);
extern int32 sys_usw_oam_show_oam_mep(uint8 lchip, uint8 oam_type);
extern int32 sys_usw_oam_show_oam_cpu_reason(uint8 lchip);

CTC_CLI(ctc_cli_usw_oam_cfm_show_port_oam_info,
        ctc_cli_usw_oam_cfm_show_port_oam_info_cmd,
        "show oam port-info port GPORT_ID (lchip LCHIP|)",
        "Show OAM Information",
        "OAM Information",
        "OAM information in Port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32  ret      = CLI_SUCCESS;
    uint16   gport;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_oam_property_t oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_Y1731;
    oam_property.u.y1731.gport    = gport;
    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN;
    oam_property.u.y1731.dir      = CTC_BOTH_DIRECTION;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN;

    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN;

    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN;

    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_IF_STATUS;

    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_oam_cfm_show_common_info,
        ctc_cli_usw_oam_cfm_show_common_info_cmd,
        "show oam common info (lchip LCHIP|)",
        "Show OAM Information",
        "OAM Information",
        "Common information",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32  ret      = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_oam_property_t oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_Y1731;
    oam_property.u.y1731.dir      = CTC_BOTH_DIRECTION;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_RDI_MODE;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    oam_property.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN;
    ret = sys_usw_oam_show_oam_property(lchip, &oam_property);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

#define M_OAM_INTERNAL ""
CTC_CLI(ctc_cli_usw_oam_set_mep_index_by_sw,
        ctc_cli_usw_oam_set_mep_index_by_sw_cmd,
        "oam mep-index-alloc system (lchip LCHIP|)",
        CTC_CLI_OAM_M_STR,
        "OAM MEP index alloc mode",
        "OAM MEP index alloc mode by system",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32  ret      = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_internal_property(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_oam_set_mep_maid_len,
        ctc_cli_usw_oam_set_mep_maid_len_cmd,
        "oam maid-len ( byte-16 | byte-32 | byte-48 ) (lchip LCHIP|)",
        CTC_CLI_OAM_M_STR,
        "OAM MAID length",
        "Maid len 16bytes",
        "Maid len 32bytes",
        "Maid len 48bytes",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32  ret      = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    ctc_oam_maid_len_format_t maid_len = 0;

    if (0 == sal_memcmp(argv[0], "byte-16", 7))
    {
        maid_len = CTC_OAM_MAID_LEN_16BYTES;
    }
    else if (0 == sal_memcmp(argv[0], "byte-32", 7))
    {
        maid_len = CTC_OAM_MAID_LEN_32BYTES;
    }
    else if (0 == sal_memcmp(argv[0], "byte-48", 7))
    {
        maid_len = CTC_OAM_MAID_LEN_48BYTES;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_internal_maid_property(lchip, maid_len);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

extern int32
sys_usw_clear_event_cache(uint8 lchip);

CTC_CLI(ctc_cli_usw_oam_clear_error_cache,
        ctc_cli_usw_oam_clear_error_cache_cmd,
        "oam event-cache clear (lchip LCHIP|)",
        CTC_CLI_OAM_M_STR,
        "OAM event",
        "Clear OAM event",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_clear_event_cache(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_oam_tp_section_oam,
        ctc_cli_usw_oam_tp_section_oam_cmd,
        "oam tp section (port | interface) (lchip LCHIP|)",
        CTC_CLI_OAM_M_STR,
        "MPLS TP",
        "Section OAM",
        "Section OAM use port",
        "Section OAM use interface",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 is_port   = 0;
    uint8 index = 0;

    if(0 == sal_memcmp(argv[0], "port", 4))
    {
        is_port = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_tp_section_init(lchip, is_port);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


#define M_OAM_SHOW
CTC_CLI(ctc_cli_usw_oam_show_status,
        ctc_cli_usw_oam_show_status_cmd,
        "show oam status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        "Status",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_show_oam_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_oam_show_defect_status,
        ctc_cli_usw_oam_show_defect_status_cmd,
        "show oam defect status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        "Defect",
        "Status",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_show_oam_defect_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


#define OAM_Y1731_KEY "y1731 (eth-oam port GPort {vlan VLAN|cvlan VLAN|} md-level LEVEL {(up|(vpls|vpws) VPN_ID|link-oam)|1ag|} | tp-oam { (section-oam (port GPORT_ID |ifid IFID) | (space SPACE |) label IN_LABEL)})"

#define OAM_BFD_KEY "bfd ((ip|mpls|micro) my-discr MY_DISCR (sbfd-reflector|) | tp-oam (section-oam (port GPORT_ID |ifid IFID) | (space SPACE |) label IN_LABEL))"

#define OAM_Y1731_KEY_DESC "Y1731 OAM", \
    "Ethernet Y1731 OAM",               \
    CTC_CLI_GPORT_DESC,                 \
    CTC_CLI_GPHYPORT_ID_DESC,           \
    CTC_CLI_VLAN_DESC,                  \
    CTC_CLI_VLAN_RANGE_DESC,            \
    CTC_CLI_VLAN_DESC,                  \
    CTC_CLI_VLAN_RANGE_DESC,            \
    "Md level",         \
    "<0-7>",   \
    "Up Mep",  \
    "VPLS OAM mode", \
    "VPWS OAM mode", \
    "L2vpn oam id, equal to fid when VPLS", \
    "Is link level oam",\
    "Is 1Ag OAM",\
    "MPLS TP Y1731 OAM", \
    "Section OAM",\
    CTC_CLI_GPORT_DESC, \
    CTC_CLI_GPHYPORT_ID_DESC, \
    CTC_CLI_L3IF_ID_DESC,\
    CTC_CLI_L3IF_ID_RANGE_DESC,\
    "Mpls label space",\
    "Label space id, <0-255>, space 0 is platform space",\
    "MPLS label",\
    "Label"

#define OAM_BFD_KEY_DESC "BFD OAM", \
    "IP BFD",\
    "MPLS BFD",\
    "MICRO BFD",\
    "My Discriminator",\
    "My Discriminator value, <0-4294967295>", \
    "SBFD Reflector",\
    "MPLS TP BFD",\
    "Section OAM",\
    CTC_CLI_GPORT_DESC, \
    CTC_CLI_GPHYPORT_ID_DESC, \
    CTC_CLI_L3IF_ID_DESC,\
    CTC_CLI_L3IF_ID_RANGE_DESC,\
    "Mpls label space",\
    "Label space id, <0-255>, space 0 is platform space",\
    "MPLS label",\
    "Label"


CTC_CLI(ctc_cli_usw_oam_show_mep_detail,
        ctc_cli_usw_oam_show_mep_detail_cmd,
        "show oam mep ("OAM_Y1731_KEY"|"OAM_BFD_KEY") (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        "MEP Information",
        OAM_Y1731_KEY_DESC,
        OAM_BFD_KEY_DESC,
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_oam_key_t oam_key;

    uint32 value = 0;

    sal_memset(&oam_key, 0, sizeof(ctc_oam_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    /*Y1731*/
    index = CTC_CLI_GET_ARGC_INDEX("y1731");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("eth-oam");
        if (0xFF != index)
        {
            oam_key.mep_type    = CTC_OAM_MEP_TYPE_ETH_Y1731;
            index = CTC_CLI_GET_ARGC_INDEX("port");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT16("gport", value, argv[index + 1]);
                oam_key.u.eth.gport = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("vlan");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT16("vlan", value, argv[index + 1]);
                oam_key.u.eth.vlan_id = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("cvlan");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT16("cvlan", value, argv[index + 1]);
                oam_key.u.eth.cvlan_id = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("md-level");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT16("Md level", value, argv[index + 1]);
                oam_key.u.eth.md_level = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("up");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
            }

            index = CTC_CLI_GET_ARGC_INDEX("vpls");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_L2VPN);
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_VPLS);
                CTC_CLI_GET_UINT16("l2vpn OAM Id", value, argv[index + 1]);
                oam_key.u.eth.l2vpn_oam_id = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("vpws");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_L2VPN);
                CTC_CLI_GET_UINT16("l2vpn OAM Id", value, argv[index + 1]);
                oam_key.u.eth.l2vpn_oam_id = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("link-oam");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
            }

            index = CTC_CLI_GET_ARGC_INDEX("1ag");
            if (0xFF != index)
            {
                oam_key.mep_type    = CTC_OAM_MEP_TYPE_ETH_1AG;
            }

        }

        index = CTC_CLI_GET_ARGC_INDEX("tp-oam");
        if (0xFF != index)
        {
            oam_key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

            index = CTC_CLI_GET_ARGC_INDEX("section-oam");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);

                index = CTC_CLI_GET_ARGC_INDEX("port");
                if (0xFF != index)
                {
                    CTC_CLI_GET_UINT16("gport", value, argv[index + 1]);
                    oam_key.u.tp.gport_or_l3if_id = value;
                }

                index = CTC_CLI_GET_ARGC_INDEX("ifid");
                if (0xFF != index)
                {
                    CTC_CLI_GET_UINT16("IFID", value, argv[index + 1]);
                    oam_key.u.tp.gport_or_l3if_id = value;
                }
            }

            index = CTC_CLI_GET_ARGC_INDEX("space");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT8("space", value, argv[index + 1]);
                oam_key.u.tp.mpls_spaceid = value;
            }


            index = CTC_CLI_GET_ARGC_INDEX("label");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("mpls label", value, argv[index + 1]);
                oam_key.u.tp.label = value;
            }

        }

    }
    /*BFD*/
    index = CTC_CLI_GET_ARGC_INDEX("bfd");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ip");
        if (0xFF != index)
        {
            oam_key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;

            index = CTC_CLI_GET_ARGC_INDEX("my-discr");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("my-discr", value, argv[index + 1]);
                oam_key.u.bfd.discr = value;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls");
        if (0xFF != index)
        {
            oam_key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;

            index = CTC_CLI_GET_ARGC_INDEX("my-discr");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("my-discr", value, argv[index + 1]);
                oam_key.u.bfd.discr = value;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("micro");
        if (0xFF != index)
        {
            oam_key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;

            index = CTC_CLI_GET_ARGC_INDEX("my-discr");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("my-discr", value, argv[index + 1]);
                oam_key.u.bfd.discr = value;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("tp-oam");
        if (0xFF != index)
        {
            oam_key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

            index = CTC_CLI_GET_ARGC_INDEX("section-oam");
            if (0xFF != index)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);

                index = CTC_CLI_GET_ARGC_INDEX("port");
                if (0xFF != index)
                {
                    CTC_CLI_GET_UINT16("gport", value, argv[index + 1]);
                    oam_key.u.tp.gport_or_l3if_id = value;
                }

                index = CTC_CLI_GET_ARGC_INDEX("ifid");
                if (0xFF != index)
                {
                    CTC_CLI_GET_UINT16("IFID", value, argv[index + 1]);
                    oam_key.u.tp.gport_or_l3if_id = value;
                }
            }

            index = CTC_CLI_GET_ARGC_INDEX("space");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT8("space", value, argv[index + 1]);
                oam_key.u.tp.mpls_spaceid = value;
            }

            index = CTC_CLI_GET_ARGC_INDEX("label");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("mpls label", value, argv[index + 1]);
                oam_key.u.tp.label = value;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
        if (0xFF != index)
        {
            CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
        }
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_show_oam_mep_detail(lchip, &oam_key);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_oam_show_mep,
        ctc_cli_usw_oam_show_mep_cmd,
        "show oam ( cfm | tp-y1731 | bfd |) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        "Ethernet OAM",
        "MPLS TP Y1731 OAM",
        "BFD OAM",
        CTC_CLI_CHIP_DESC,
        CTC_CLI_CHIP_ID_DESC)
{
    int32 ret = CTC_E_NONE;
    uint8 oam_type = 4;/*SYS_OAM_DBG_ALL*/
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cfm");
    if (0xFF != index)
    {
        oam_type = 1;/*SYS_OAM_DBG_ETH_OAM*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-y1731");
    if (0xFF != index)
    {
        oam_type = 2;/*SYS_OAM_DBG_TP_Y1731_OAM*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd");
    if (0xFF != index)
    {
        oam_type = 3;/*SYS_OAM_DBG_BFD*/
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_oam_show_oam_mep(lchip, oam_type);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_oam_show_reason,
        ctc_cli_usw_oam_show_reason_cmd,
        "show oam cpu-reason",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        "OAM CPU Reason")
{
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;
    ret =sys_usw_oam_show_oam_cpu_reason(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

/*---------------------------------------------------------------------*/

extern ctc_oam_lmep_t* p_oam_lmep;
extern uint8 mep_type ;
#define CTC_CLI_OAM_CFM_DIRECTION_DESC "MEP direction"
#define CTC_CLI_OAM_CFM_DIRECTION_UP_DESC "Up MEP"
#define CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC "Down MEP"
#define CTC_CLI_OAM_LINK_OAM_DESC "Is link level oam"
#define CTC_CLI_OAM_LINK_OAM_YES_DESC "Is link level oam"
#define CTC_CLI_OAM_LINK_OAM_NO_DESC "Not link level oam"

/* chan clis */
#define M_CHAN "cli"

CTC_CLI(ctc_cli_usw_oam_cfm_add_chan,
        ctc_cli_usw_oam_cfm_add_chan_cmd,
        "oam cfm lookup-chan add gport GPORT_ID direction (up |down ) {link-oam | (vpls | vpws) L2VPN_OAM_ID | vlan VLAN_ID (ccm-vlan VLAN_ID|) |} {cvlan VLAN_ID | vlan-domain (cvlan-domain|svlan-domain)} (master-gchip GCHIP_ID|)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "MEP lookup channel",
        "Add MEP lookup channel",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
        CTC_CLI_OAM_LINK_OAM_DESC,
        "VPLS OAM mode",
        "VPWS OAM mode",
        "L2vpn oam id, equal to fid when VPLS",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan Domain",
        "Cvlan Domain",
        "Svlan Domain",
        "Master chip id",
        "<0-29>")
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0xff;

    ctc_oam_lmep_t oam_lmep;

    sal_memset(&oam_lmep, 0, sizeof(ctc_oam_lmep_t));
    oam_lmep.key.mep_type = mep_type;

    CTC_CLI_GET_UINT16_RANGE("gport", oam_lmep.key.u.eth.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if (0 == sal_memcmp(argv[1], "u", 1))
    {
        oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_UP_MEP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("link-oam");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_UP_MEP))
        {
            ctc_cli_out("%% %s \n", "Up MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_LINK_SECTION_OAM;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpls");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "VPLS MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_L2VPN;
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_VPLS;
            CTC_CLI_GET_UINT16_RANGE("FID", oam_lmep.key.u.eth.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpws");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "VPWS MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_L2VPN;
            oam_lmep.key.flag &=( ~CTC_OAM_KEY_FLAG_VPLS);
            CTC_CLI_GET_UINT16_RANGE("VPWS id", oam_lmep.key.u.eth.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "Link OAM MEP should without vlan.");
            return CLI_ERROR;
        }
        else
        {
            CTC_CLI_GET_UINT16_RANGE("VLAN ID", oam_lmep.key.u.eth.vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("CVLAN ID", oam_lmep.key.u.eth.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if (index != 0xff)
    {
        if (0 == sal_memcmp(argv[index+1], "cvlan", 3))
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_CVLAN_DOMAIN;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("Master gchip id", oam_lmep.u.y1731_lmep.master_gchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if (ret < 0)
    {
        if ((ret != CTC_E_EXIST) && (ret != CTC_E_NOT_SUPPORT))
        {
            ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        else
        {
            ret = CLI_SUCCESS;
        }
    }

    if (!p_oam_lmep)
    {
        p_oam_lmep = (ctc_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(ctc_oam_lmep_t));
        if (!p_oam_lmep)
        {
            ctc_cli_out("%% %s \n", "System error: no memory");
            return CLI_ERROR;
        }
    }

    sal_memcpy(p_oam_lmep, &oam_lmep, sizeof(ctc_oam_lmep_t));

    g_ctc_vti->node  = CTC_SDK_OAM_CHAN_MODE;

    return ret;
}



CTC_CLI(ctc_cli_usw_oam_cfm_remove_chan,
        ctc_cli_usw_oam_cfm_remove_chan_cmd,
        "oam cfm lookup-chan remove gport GPORT_ID direction (up |down ) { link-oam | vlan VLAN_ID | (vpls | vpws) L2VPN_OAM_ID |} (cvlan VLAN_ID)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "MEP lookup channel",
        "Remove MEP lookup channel",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
        CTC_CLI_OAM_LINK_OAM_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "VPLS OAM mode",
        "VPWS OAM mode",
        "L2vpn oam id, equal to fid when VPLS",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0xff;
    ctc_oam_lmep_t oam_lmep;

    sal_memset(&oam_lmep, 0, sizeof(ctc_oam_lmep_t));
    oam_lmep.key.mep_type = mep_type;

    CTC_CLI_GET_UINT16_RANGE("gport", oam_lmep.key.u.eth.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if (0 == sal_memcmp(argv[1], "u", 1))
    {
        oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_UP_MEP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("link-oam");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_UP_MEP))
        {
            ctc_cli_out("%% %s \n", "Up MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_LINK_SECTION_OAM;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "Link OAM MEP should without vlan.");
            return CLI_ERROR;
        }
        else
        {
            CTC_CLI_GET_UINT16_RANGE("VLAN ID", oam_lmep.key.u.eth.vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpls");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "VPLS MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_L2VPN;
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_VPLS;
            CTC_CLI_GET_UINT16_RANGE("FID", oam_lmep.key.u.eth.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpws");
    if (index != 0xff)
    {
        if (CTC_FLAG_ISSET(oam_lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            ctc_cli_out("%% %s \n", "VPWS MEP can not be link OAM.");
            return CLI_ERROR;
        }
        else
        {
            oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_L2VPN;
            oam_lmep.key.flag &=( ~CTC_OAM_KEY_FLAG_VPLS);
            CTC_CLI_GET_UINT16_RANGE("VPWS id", oam_lmep.key.u.eth.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("CVLAN ID", oam_lmep.key.u.eth.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if (p_oam_lmep)
    {
        mem_free(p_oam_lmep);
        p_oam_lmep = NULL;
    }

    if (ret < 0)
    {
        if (ret != CTC_E_NOT_SUPPORT)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        else
        {
            return CLI_SUCCESS;
        }
    }


    return ret;
}

#define CTC_CLI_OAM_MEP_LEVEL_DESC "MEP level"
#define CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC "<0-7>"
CTC_CLI(ctc_cli_oam_cfm_update_lmep_npm_en,
        ctc_cli_oam_cfm_update_lmep_npm_en_cmd,
        "local-mep npm (level LEVEL | ) (session SESSION) (enable | disable)",
        "Local MEP",
        "Npm",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "NPM session",
        "Session id",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret       = CLI_SUCCESS;
    uint32 session_id = 0xff;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    index = CTC_CLI_GET_ARGC_INDEX("session");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("session id", session_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        session_id = 0xff;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM;
    lmep.update_value   = session_id;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_lmep(g_api_lchip, &lmep);
    }
    else
    {
        ret = ctc_oam_update_lmep(&lmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_oam_cfm_set_port_enable_port_mip_en,
        ctc_cli_usw_oam_cfm_set_port_enable_port_mip_en_cmd,
        "oam cfm port GPORT_ID mip-en (level-bitmap BITMAP)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Mip enable",
        "Level bitmap",
        "Bitmap <0-255>")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint32 bitmap = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("level-bitmap");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("level bitmap", bitmap, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }


    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN;
    oam_prop.u.y1731.gport  = gport;
    oam_prop.u.y1731.value  = bitmap;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_prop);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


int32
ctc_usw_oam_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_show_port_oam_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_show_common_info_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_set_mep_index_by_sw_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_set_mep_maid_len_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_clear_error_cache_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_tp_section_oam_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_defect_status_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_mep_detail_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_mep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_reason_cmd);


    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_set_port_enable_port_mip_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_add_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_remove_chan_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_npm_en_cmd);

    return 0;
}

int32
ctc_usw_oam_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_show_port_oam_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_show_common_info_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_set_mep_index_by_sw_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_set_mep_maid_len_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_clear_error_cache_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_oam_tp_section_oam_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_defect_status_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_mep_detail_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_mep_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_show_reason_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_set_port_enable_port_mip_en_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_add_chan_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_oam_cfm_remove_chan_cmd);
    uninstall_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_npm_en_cmd);

    return 0;
}

#endif


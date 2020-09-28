/**
 @file ctc_oam_cli.c

 @date 2010-03-23

 @version v2.0

 This file defines functions for oam cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_oam_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#define OAM_NEW_API_DEBUG 0

#define CTC_CLI_OAM_CFM_DIRECTION_DESC "MEP direction"
#define CTC_CLI_OAM_CFM_DIRECTION_UP_DESC "Up MEP"
#define CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC "Down MEP"
#define CTC_CLI_OAM_LINK_OAM_DESC "Is link level oam"
#define CTC_CLI_OAM_LINK_OAM_YES_DESC "Is link level oam"
#define CTC_CLI_OAM_LINK_OAM_NO_DESC "Not link level oam"
#define CTC_CLI_OAM_MAID_DESC "MA ID"
#define CTC_CLI_OAM_MD_NAME_DESC "MD Name"
#define CTC_CLI_OAM_MD_NAME_VALUE_DESC "MD name string"
#define CTC_CLI_OAM_MA_NAME_DESC "MA Name"
#define CTC_CLI_OAM_MA_NAME_VALUE_DESC "MA name string"
#define CTC_CLI_OAM_MEG_ID_DESC "MEP group id"
#define CTC_CLI_OAM_MEG_ID_VALUE_DESC "MEG id string"

#define CTC_CLI_OAM_MEP_ID_DESC "MEP id"
#define CTC_CLI_OAM_MEP_ID_VALUE_DESC "<1-8191>"
#define CTC_CLI_OAM_MEP_LEVEL_DESC "MEP level"
#define CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC "<0-7>"
#define CTC_CLI_OAM_MEP_ACTIVE_DESC "MEP is active or not"
#define CTC_CLI_OAM_MEP_ACTIVE_YES_DESC "MEP is active"
#define CTC_CLI_OAM_MEP_ACTIVE_NO_DESC "MEP is not active"
#define CTC_CLI_OAM_MEP_DM_DESC "Delay measurement"
#define CTC_CLI_OAM_MEP_DM_ENABLE_DESC "Enable delay measurement"
#define CTC_CLI_OAM_MEP_DM_DISABLE_DESC "Disable delay measurement"
#define CTC_CLI_OAM_MEP_CCM_INTERVAL_DESC "CCM interval"
#define CTC_CLI_OAM_MEP_CCM_INTERVAL_VALUE_DESC "<1-7>"
#define CTC_CLI_OAM_MEP_TX_TPID_TYPE_DESC "CCM tx tpid type index"
#define CTC_CLI_OAM_MEP_TX_TPID_TYPE_VALUE_DESC "Tpid index"

#define CTC_CLI_OAM_SECTION_OAM_DESC "Is MPLS TP section oam"

ctc_oam_lmep_t* p_oam_lmep = NULL;
uint8 mep_type = 0;


STATIC int32
_oam_cli_build_bfd_mepid(uint8* mep_id, uint8* mep_id_len, char* mep_id_str)
{
    uint8 mep_id_str_len = 0;
    uint8 total_len = 0;

    mep_id[0] = 0x00; /* TPYE */
    mep_id[1] = 0x01;

    mep_id_str_len = sal_strlen(mep_id_str);
    if (mep_id_str_len > CTC_OAM_BFD_CV_MEP_ID_MAX_LEN - 4)
    {
        return -1;
    }
    else
    {
        total_len = mep_id_str_len + 4;
        *mep_id_len = total_len;
    }
    mep_id[2] = 0x00; /* LENGTH = 32*/
    mep_id[3] = mep_id_str_len;
    sal_memcpy(&mep_id[4], mep_id_str, mep_id_str_len); /* copy mepid string */

    return 0;
}


STATIC int32
_oam_cli_build_megid(ctc_oam_maid_t* maid, char* meg_id)
{
    uint8 meg_id_len = 0;
    uint8 maid_len = 0;

    maid->maid[0] = 0x1; /* MD name format 1 */
    maid->maid[1] = 32;  /* ICC based format */

    meg_id_len = sal_strlen(meg_id);
    if (meg_id_len > 13)  /* 16 bytes total for Y.1731*/
    {
        return -1;
    }
    else
    {
        maid_len = meg_id_len + 3;
        maid->maid_len = maid_len;
    }

    maid->maid[2] = 13; /* megid len */

    sal_memcpy(&maid->maid[3], meg_id, meg_id_len); /* copy megid string */

    return 0;
}

STATIC int32
_oam_cli_build_maid(ctc_oam_maid_t* maid, char* md_name, char* ma_name)
{
    uint8 md_name_len = 0;
    uint8 ma_name_len = 0;
    uint8 maid_len = 0;

    md_name_len = sal_strlen(md_name);
    ma_name_len = sal_strlen(ma_name);
    if (md_name_len > 43)
    {
        return -1;
    }
    else
    {
        if (ma_name_len > (44 - md_name_len))
        {
            return -1;
        }
    }

    maid->maid[0] = 0x4;  /* MD name format */
    maid->maid[1] = md_name_len; /* MD name len */
    sal_memcpy(&maid->maid[2], md_name, md_name_len); /* copy md name string */

    maid->maid[2 + md_name_len] = 0x2; /* MA name format */
    maid->maid[3 + md_name_len] = ma_name_len; /* MA name len */
    sal_memcpy(&maid->maid[4 + md_name_len], ma_name, ma_name_len);

    maid_len = 4 + md_name_len + ma_name_len;
    maid->maid_len = maid_len;

    return 0;
}

/* debug switch */
#define M_DEBUG "cli"

CTC_CLI(ctc_cli_oam_cfm_debug_on,
        ctc_cli_oam_cfm_debug_on_cmd,
        "debug oam (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_OAM_M_STR,
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint8 level = CTC_DEBUG_LEVEL_NONE;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }
    else
    {
        level = CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctc");
    if (index != 0xff)
    {
        ctc_debug_set_flag("oam", "oam", OAM_CTC, level, TRUE);
    }
    else
    {
        ctc_debug_set_flag("oam", "oam", OAM_SYS, level, TRUE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_oam_cfm_debug_off,
        ctc_cli_oam_cfm_debug_off_cmd,
        "no debug oam (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_OAM_M_STR,
        "CTC Layer",
        "SYS Layer")
{
    uint8 index = 0;
    uint8 level  = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ctc");

    if (index != 0xff)
    {
        ctc_debug_set_flag("oam", "oam", OAM_CTC, level, FALSE);
    }
    else
    {
        ctc_debug_set_flag("oam", "oam", OAM_SYS, level, FALSE);
    }

    return CLI_SUCCESS;
}

/* maid clis */
#define M_MAID "cli"
CTC_CLI(ctc_cli_oam_cfm_add_remove_maid,
        ctc_cli_oam_cfm_add_remove_maid_cmd,
        "oam cfm maid (add|remove) (meg-id STRING:MEG_ID | md-name STRING:MEG_ID ma-name STRING:MA_NAME) (master-gchip GCHIP_ID|)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        CTC_CLI_OAM_MAID_DESC,
        "Add MAID",
        "Remove MAID",
        CTC_CLI_OAM_MEG_ID_DESC,
        CTC_CLI_OAM_MEG_ID_VALUE_DESC,
        CTC_CLI_OAM_MD_NAME_DESC,
        CTC_CLI_OAM_MD_NAME_VALUE_DESC,
        CTC_CLI_OAM_MA_NAME_DESC,
        CTC_CLI_OAM_MA_NAME_VALUE_DESC,
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
{
    uint8 addflag = 0;
    uint8 index   = 0;
    int32 ret     = CLI_SUCCESS;
    ctc_oam_maid_t maid;

    sal_memset(&maid, 0, sizeof(maid));

    /* parse add/remove */
    if (0 == sal_memcmp(argv[0], "a", 1))
    {
        addflag = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("meg-id");
    if (index != 0xff)
    {
        maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        /* build MEGID string */
        if (_oam_cli_build_megid(&maid, argv[index + 1]))
        {
            ctc_cli_out("%% invalid megid \n");
            return CLI_ERROR;
        }
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("md-name");
        if (index != 0xff)
        {
            maid.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
            mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
            /* build MAID string */
            if (_oam_cli_build_maid(&maid, argv[index + 1], argv[index + 3]))
            {
                ctc_cli_out("%% invalid md name or ma name \n");
                return CLI_ERROR;
            }
        }
    }

    if (addflag)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_add_maid(g_api_lchip, &maid);
        }
        else
        {
            ret = ctc_oam_add_maid(&maid);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_remove_maid(g_api_lchip, &maid);
        }
        else
        {
            ret = ctc_oam_remove_maid(&maid);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_tp_y1731_add_remove_maid,
        ctc_cli_oam_tp_y1731_add_remove_maid_cmd,
        "oam tp-y1731 maid (add | remove) meg-id STRING:MEG_ID",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_MPLS_TP_Y1731_M_STR,
        CTC_CLI_OAM_MAID_DESC,
        "Add MAID",
        "Remove MAID",
        CTC_CLI_OAM_MEG_ID_DESC,
        CTC_CLI_OAM_MEG_ID_VALUE_DESC)
{
    uint8 addflag = 0;
    int32 ret     = CLI_SUCCESS;
    ctc_oam_maid_t maid;

    sal_memset(&maid, 0, sizeof(maid));

    /* parse add/remove */
    if (0 == sal_memcmp(argv[0], "a", 1))
    {
        addflag = 1;
    }

    maid.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

    /* build MEGID string */
    if (_oam_cli_build_megid(&maid, argv[1]))
    {
        ctc_cli_out("%% invalid megid \n");
        return CLI_ERROR;
    }

    if (addflag)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_add_maid(g_api_lchip, &maid);
        }
        else
        {
            ret = ctc_oam_add_maid(&maid);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_remove_maid(g_api_lchip, &maid);
        }
        else
        {
            ret = ctc_oam_remove_maid(&maid);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/* chan clis */
#define M_CHAN "cli"
CTC_CLI(ctc_cli_oam_cfm_add_chan,
        ctc_cli_oam_cfm_add_chan_cmd,
        "oam cfm lookup-chan add gport GPORT_ID direction (up |down ) {link-oam | (vpls | vpws) L2VPN_OAM_ID | vlan VLAN_ID (ccm-vlan VLAN_ID|)} (master-gchip GCHIP_ID|)",
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
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
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

CTC_CLI(ctc_cli_oam_tp_add_chan,
        ctc_cli_oam_tp_add_chan_cmd,
        "oam (tp-y1731) lookup-chan add (section-oam (gport GPORT_ID |ifid IFID) | ((space SPACEID | ) mpls-label LABEL)) (master-gchip GCHIP_ID|)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_MPLS_TP_Y1731_M_STR,
        "MEP lookup channel",
        "Add MEP lookup channel",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0xff;
    uint8 tp_y1731      = 0;
     /*uint8 is_add_flag   = 0;*/
    ctc_oam_lmep_t oam_lmep;

    sal_memset(&oam_lmep, 0, sizeof(ctc_oam_lmep_t));
    oam_lmep.key.mep_type = mep_type;

    if (0 == sal_memcmp(argv[0], "tp-y", 4))
    {
        tp_y1731 = 1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("section-oam");
    if (index != 0xff)
    {
        oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_LINK_SECTION_OAM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", oam_lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ifid");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("IFID", oam_lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-label");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT32_RANGE("MPLS Label", oam_lmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("space");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("Space", oam_lmep.key.u.tp.mpls_spaceid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        if (tp_y1731)
        {
            CTC_CLI_GET_UINT8_RANGE("Master gchip id", oam_lmep.u.y1731_lmep.master_gchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        else
        {

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

CTC_CLI(ctc_cli_oam_tp_remove_chan,
        ctc_cli_oam_tp_remove_chan_cmd,
        "oam (tp-y1731) lookup-chan remove (section-oam (gport GPORT_ID |ifid IFID) | ((space SPACEID | ) mpls-label LABEL)) (master-gchip GCHIP_ID|)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_MPLS_TP_Y1731_M_STR,
        "MEP lookup channel",
        "Remove MEP lookup channel",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0xff;
    uint8 tp_y1731      = 0;
     /*uint8 is_add_flag   = 0;*/
    ctc_oam_lmep_t oam_lmep;

    sal_memset(&oam_lmep, 0, sizeof(ctc_oam_lmep_t));
    oam_lmep.key.mep_type = mep_type;

    if (0 == sal_memcmp(argv[0], "tp-y", 4))
    {
        tp_y1731 = 1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("section-oam");
    if (index != 0xff)
    {
        oam_lmep.key.flag |= CTC_OAM_KEY_FLAG_LINK_SECTION_OAM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", oam_lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ifid");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("IFID", oam_lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-label");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT32_RANGE("MPLS Label", oam_lmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("space");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("Space", oam_lmep.key.u.tp.mpls_spaceid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        if (tp_y1731)
        {
            CTC_CLI_GET_UINT8_RANGE("Master gchip id", oam_lmep.u.y1731_lmep.master_gchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        else
        {

        }
    }

    if (p_oam_lmep)
    {
        mem_free(p_oam_lmep);
        p_oam_lmep = NULL;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_remove_chan,
        ctc_cli_oam_cfm_remove_chan_cmd,
        "oam cfm lookup-chan remove gport GPORT_ID direction (up |down ) { link-oam | vlan VLAN_ID | (vpls | vpws) L2VPN_OAM_ID)}",
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
        "L2vpn oam id, equal to fid when VPLS")
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

CTC_CLI(ctc_cli_oam_cfm_enter_chan,
        ctc_cli_oam_enter_chan_cmd,
        "oam enter lookup-chan gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID|) (cvlan VLAN_ID|)",
        CTC_CLI_OAM_M_STR,
        "Enter OAM channel mode",
        "MEP lookup channel",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
        CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
        CTC_CLI_OAM_LINK_OAM_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
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

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xff)
    {

        CTC_CLI_GET_UINT16_RANGE("CVLAN ID", oam_lmep.key.u.eth.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

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

CTC_CLI(ctc_cli_oam_cfm_exit_chan,
        ctc_cli_oam_cfm_exit_chan_cmd,
        "exit",
        "Exit channel mode")
{
    g_ctc_vti->node  = CTC_SDK_MODE;

    return CLI_SUCCESS;
}

/* local mep clis */
#define M_MEP "cli"
CTC_CLI(ctc_cli_oam_cfm_add_lmep,
        ctc_cli_oam_cfm_add_lmep_cmd,
        "local-mep add (mep-id MEP_ID) (meg-id STRING:MEG_ID | md-name STRING:MEG_ID ma-name STRING:MA_NAME) (mep-en|)(ccm-interval CCM_INTERVAL)({dm-enable| mep-on-cpu|aps-enable | lm-en LM_TYPE LM_COS_TYPE LM_COS | csf-en CSF_TYPE}|)\
        (eth-oam { level LEVEL | is-p2p | seq-num-en | is-vpls-oam | addr-share-mode | tx-if-status | tx-port-status | tx-sender-id| tpid-index TPID_INDEX| \
        ccm-gport-id CCM_GPORT_ID | up-mep-tx-group-id TX_GROUP_ID |twamp-en (nhid NHID|)|twamp-reflect-en |} |\
        tp-y1731-oam { protection-path | nhid NHID | mpls-ttl MPLS_TTL | without-gal } ) (mep-index IDX|)",
        "Local MEP",
        "Add local MEP on oam channel",
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC,
        CTC_CLI_OAM_MEG_ID_DESC,
        CTC_CLI_OAM_MEG_ID_VALUE_DESC,
        CTC_CLI_OAM_MD_NAME_DESC,
        CTC_CLI_OAM_MD_NAME_VALUE_DESC,
        CTC_CLI_OAM_MA_NAME_DESC,
        CTC_CLI_OAM_MA_NAME_VALUE_DESC,
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        CTC_CLI_OAM_MEP_CCM_INTERVAL_DESC,
        CTC_CLI_OAM_MEP_CCM_INTERVAL_VALUE_DESC,
        CTC_CLI_OAM_MEP_DM_DESC,
        "MEP on cpu",
        "Enable APS message to CPU",
        "MEP enable LM",
        "LM type: 0 means not count; 1 means dual-end LM; 2 means single-end LM",
        "LM Cos type: 0 All cos data counter together; 1 Only count specified cos data; 2 Per cos date to count",
        "LM Cos <0-7>",
        "MEP enable CSF",
        "CSF type: 0 LOS; 1 FDI; 2 RDI; 3 DCI",
        "Ethernet OAM option",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "MEP is P2P mode",
        "MEP sequence number enable",
        "MEP is VPLS OAM",
        "MEP Shared address mode",
        "Tx interface status TLV",
        "Tx port status TLV",
        "Tx sender id TLV",
        CTC_CLI_OAM_MEP_TX_TPID_TYPE_DESC,
        CTC_CLI_OAM_MEP_TX_TPID_TYPE_VALUE_DESC,
        "Ccm gport id[HB]",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Tx mcast group id for up MEP[HB]",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "TWAMP enable",
        "NextHop ID for twamp receiver",
        CTC_CLI_NH_ID_STR,
        "TWAMP reflect enable",
        "MPLS TP Y.1731 OAM option",
        "APS Protection PATH",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "MPLS TTL",
        "TTL, <0-255>",
        "MPLS TP Y.1731 OAM without GAL",
        "MEP index allocated by system",
        "2~max MEP index")
{
    int32 ret   = CLI_SUCCESS;
    uint8 tpid_index    = 0;
    uint8 csf_type      = 0;
    uint8 lm_type       = 0;
    uint8 lm_cos_type   = 0;
    uint8 lm_cos        = 0;

    uint8 is_tp_y1731   = 0;
    uint8 index = 0;
    ctc_oam_lmep_t lmep;
    ctc_oam_y1731_lmep_t* p_y1731_lmep  = NULL;

    /* default configure */
    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &(p_oam_lmep->key), sizeof(ctc_oam_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("tp-y1731-oam");
    if (0xFF != index)
    {
        mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
        is_tp_y1731 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-oam");
    if (0xFF != index)
    {

    }

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
    {
        p_y1731_lmep = &lmep.u.y1731_lmep;
    }
    else
    {

    }

    /*common*/
    index = CTC_CLI_GET_ARGC_INDEX("mep-id");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT16_RANGE("mep id",  p_y1731_lmep->mep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    lmep.u.y1731_lmep.master_gchip = p_oam_lmep->u.y1731_lmep.master_gchip;


    index = CTC_CLI_GET_ARGC_INDEX("meg-id");
    if (index != 0xff)
    {
        /* build MEGID string */
        if (is_tp_y1731)
        {
            p_oam_lmep->key.mep_type    = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
            lmep.key.mep_type           = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
        }
        else
        {
            p_oam_lmep->key.mep_type    = CTC_OAM_MEP_TYPE_ETH_Y1731;
            lmep.key.mep_type           = CTC_OAM_MEP_TYPE_ETH_Y1731;
        }

        _oam_cli_build_megid(&lmep.maid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("md-name");
    if (index != 0xff)
    {
        /* build MAID string */
        if (is_tp_y1731)
        {
            ctc_cli_out("%% MPLS-TP Y.1731 only support ICC MAID \n");
            return CLI_ERROR;
        }
        else
        {
            p_oam_lmep->key.mep_type    = CTC_OAM_MEP_TYPE_ETH_1AG;
            lmep.key.mep_type           = CTC_OAM_MEP_TYPE_ETH_1AG;
            _oam_cli_build_maid(&lmep.maid, argv[index + 1], argv[index + 3]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_MEP_EN;  /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("ccm-interval");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ccm interval", p_y1731_lmep->ccm_interval, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dm-enable");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_DM_EN; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-on-cpu");
    if (index != 0xff)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-enable");
    if (index != 0xff)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_APS_EN; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("CSF type", csf_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        p_y1731_lmep->flag        |= CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN; /*FLAG*/
        p_y1731_lmep->tx_csf_type = csf_type;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lm-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("LM type",      lm_type,     argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("LM cos type",  lm_cos_type, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("LM cos",       lm_cos,      argv[index + 3], 0, CTC_MAX_UINT8_VALUE);
        p_y1731_lmep->flag        |= CTC_OAM_Y1731_LMEP_FLAG_LM_EN; /*FLAG*/
        p_y1731_lmep->lm_type     = lm_type;
        p_y1731_lmep->lm_cos_type = lm_cos_type;
        p_y1731_lmep->lm_cos      = lm_cos;
    }

    /*eth*/
    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("addr-share-mode");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_SHARE_MAC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tpid-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tpid index", tpid_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    if(NULL != p_y1731_lmep)
    {
        p_y1731_lmep->tpid_index = CTC_PARSER_L2_TPID_SVLAN_TPID_0 + tpid_index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-if-status");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_TX_IF_STATUS; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-port-status");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_TX_PORT_STATUS; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-sender-id");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_TX_SEND_ID; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-p2p");
    if (index != 0xff)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE; /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-num-en");
    if (index != 0xff)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN; /*FLAG*/
    }
    index = CTC_CLI_GET_ARGC_INDEX("twamp-en");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("nhid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("nhid", p_y1731_lmep->nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("twamp-reflect-en");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_TWAMP_REFLECT_EN;
    }
    /*TP Y.1731*/
    index = CTC_CLI_GET_ARGC_INDEX("protection-path");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nhid", p_y1731_lmep->nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("mpls-ttl", p_y1731_lmep->mpls_ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("without-gal");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_WITHOUT_GAL;  /*FLAG*/
    }

    /*sys mep index*/
    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("lmep-index", lmep.lmep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_add_lmep(g_api_lchip, &lmep);
    }
    else
    {
        ret = ctc_oam_add_lmep(&lmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    sal_memcpy(p_oam_lmep, &lmep, sizeof(ctc_oam_lmep_t));

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_remove_lmep,
        ctc_cli_oam_cfm_remove_lmep_cmd,
        "local-mep remove (level LEVEL|) (twamp-en|)",
        "Local MEP",
        "Remove local MEP on oam channel",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "TWAMP enable")
{
    int32 ret   = CLI_SUCCESS;
    ctc_oam_lmep_t lmep;
    uint8 index = 0;

    /* default configure */
    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.u.y1731_lmep.master_gchip = p_oam_lmep->u.y1731_lmep.master_gchip;

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xff != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("twamp-en");
    if (0xff != index)
    {
        lmep.u.y1731_lmep.flag |= CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_remove_lmep(g_api_lchip, &lmep);
    }
    else
    {
        ret = ctc_oam_remove_lmep(&lmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_OAM_SECTION_OAM_DESC "Is MPLS TP section oam"
#define CTC_CLI_OAM_MEP_ACTIVE_DESC "MEP is active or not"

CTC_CLI(ctc_cli_oam_bfd_add_lmep,
        ctc_cli_oam_bfd_add_lmep_cmd,
        "local-mep bfd add my-discr MY_DISCR ({mep-en | mep-on-cpu | (sbfd-initiator | sbfd-reflector) | desired-tx-interval DESIRED_TX_INTERVAL | state STATE | diag DIAG | detect-mult DETECT_MULT | aps-enable |binding-en}|)\
        ((ip-bfd | micro-bfd) { is-single-hop | src-udp-port SRC_PORT | nhid NHID | (ipv4-sa A.B.C.D (ipv4-da A.B.C.D|) | ipv6-sa X:X::X:X ipv6-da X:X::X:X) ttl TTL} |\
        mpls-bfd { pw-vccv | (vccv-with-ipv4 | vccv-with-ipv6) | (space SPACEID |) mpls-in-label LABEL | protection-path | nhid NHID | src-udp-port SRC_PORT | (ipv4-sa A.B.C.D (ipv4-da A.B.C.D|) | ipv6-sa X:X::X:X ipv6-da X:X::X:X) |mpls-ttl MPLS_TTL |mpls-exp MPLS_EXP}| \
        tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL) | protection-path | nhid NHID | mpls-ttl MPLS_TTL | mpls-exp MPLS_EXP | without-gal | mep-id MEP_ID | lm-en LM_COS_TYPE LM_COS | csf-en CSF_TYPE} ) \
        ({mep-index IDX | master-gchip GCHIP_ID} | )",
        "Local MEP",
        "BFD MEP",
        "Add local BFD MEP",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        "MEP on cpu",
        "SBFD Initiator",
        "SBFD Reflector",
        "BFD desired min tx interval",
        "uint: ms",
        "BFD local state",
        "Admin down :0 ; Down: 1; Init : 2; Up :3",
        "BFD local diag",
        "<0-31>",
        "BFD local detect mult",
        "<1-15>",
        "Enable APS message to CPU",
        "Session binding enable",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "Single Hop BFD",
        "UDP source port",
        "<49152-65535>",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "IPv4 sa",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 da",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 sa",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 da",
        CTC_CLI_IPV6_FORMAT,
        "IP TTL",
        "TTL, <0-255>",
        "MPLS BFD OAM option",
        "MPLS PW VCCV BFD",
        "MPLS PW VCCV BFD with IPv4, GAL 0x21",
        "MPLS PW VCCV BFD with IPv6, GAL 0x57",
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "MEP in mpls label",
        "Label, <0-1048575>",
        "APS PROTECTION PATH",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "UDP source port",
        "<49152-65535>",
        "IPv4 sa",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 da",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 sa",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 da",
        CTC_CLI_IPV6_FORMAT,
        "MPLS TTL",
        "TTL, <0-255>",
        "MPLS EXP",
        "MPLS EXP, <0-7>",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "MEP in mpls label",
        "Label, <0-1048575>",
        "APS PROTECTION PATH",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "MPLS TTL",
        "TTL, <0-255>",
        "MPLS EXP",
        "MPLS EXP, <0-7>",
        "MPLS TP BFD OAM without GAL",
        "Source MEP ID TLV",
        "Source MEP ID TLV value",
        "MEP enable LM",
        "LM Cos type: 0 All cos data counter together; 1 Only count specified cos data; 2 Per cos date to count",
        "LM Cos <0-7>",
        "MEP enable CSF",
        "CSF type: 0 LOS; 1 FDI; 2 RDI; 3 DCI",
        "MEP index allocated by system",
        "2~max MEP index",
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0;
    uint8 index1 = 0;
    ctc_oam_lmep_t lmep;
    ctc_oam_bfd_lmep_t* p_bfd_lmep  = NULL;
    ipv6_addr_t ipv6_address;

    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));
    p_bfd_lmep = &lmep.u.bfd_lmep;

    /*IP BFD*/
    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    index1 = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if ((0xFF != index) || (0xFF != index1))
    {
        lmep.key.mep_type = (0xFF != index) ? CTC_OAM_MEP_TYPE_IP_BFD : CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("is-single-hop");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IP_SINGLE_HOP);
        }

        index = CTC_CLI_GET_ARGC_INDEX("src-udp-port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("UDP source Port", p_bfd_lmep->bfd_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("nhid");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("Nhid", p_bfd_lmep->nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv4-sa");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
            CTC_CLI_GET_IPV4_ADDRESS("IPv4-sa", p_bfd_lmep->ip4_sa, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv4-da");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
            CTC_CLI_GET_IPV4_ADDRESS("IPv4-da", p_bfd_lmep->ip4_da, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ttl");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("ttl", p_bfd_lmep->ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-sa", ipv6_address, argv[index + 1]);
            /* adjust endian */
            p_bfd_lmep->ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            p_bfd_lmep->ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            p_bfd_lmep->ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            p_bfd_lmep->ipv6_sa[3] = sal_htonl(ipv6_address[3]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-da", ipv6_address, argv[index + 1]);
            /* adjust endian */
            p_bfd_lmep->ipv6_da[0] = sal_htonl(ipv6_address[0]);
            p_bfd_lmep->ipv6_da[1] = sal_htonl(ipv6_address[1]);
            p_bfd_lmep->ipv6_da[2] = sal_htonl(ipv6_address[2]);
            p_bfd_lmep->ipv6_da[3] = sal_htonl(ipv6_address[3]);
        }
    }

    /* MPLS BFD*/
    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("pw-vccv");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV);
        }

        index = CTC_CLI_GET_ARGC_INDEX("vccv-with-ipv4");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4);
        }
        index = CTC_CLI_GET_ARGC_INDEX("vccv-with-ipv6");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", p_bfd_lmep->mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("MPLS In label", p_bfd_lmep->mpls_in_label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
        }

        index = CTC_CLI_GET_ARGC_INDEX("nhid");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("Nhid", p_bfd_lmep->nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("src-udp-port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("UDP source Port", p_bfd_lmep->bfd_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv4-sa");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
            CTC_CLI_GET_IPV4_ADDRESS("IPv4-sa", p_bfd_lmep->ip4_sa, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv4-da");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
            CTC_CLI_GET_IPV4_ADDRESS("IPv4-da", p_bfd_lmep->ip4_da, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-sa", ipv6_address, argv[index + 1]);
            /* adjust endian */
            p_bfd_lmep->ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            p_bfd_lmep->ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            p_bfd_lmep->ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            p_bfd_lmep->ipv6_sa[3] = sal_htonl(ipv6_address[3]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-da", ipv6_address, argv[index + 1]);
            /* adjust endian */
            p_bfd_lmep->ipv6_da[0] = sal_htonl(ipv6_address[0]);
            p_bfd_lmep->ipv6_da[1] = sal_htonl(ipv6_address[1]);
            p_bfd_lmep->ipv6_da[2] = sal_htonl(ipv6_address[2]);
            p_bfd_lmep->ipv6_da[3] = sal_htonl(ipv6_address[3]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("mpls-ttl", p_bfd_lmep->ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-exp");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("mpls-exp", p_bfd_lmep->tx_cos_exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

    }

    /*MPLS TP BFD*/
    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", lmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", lmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        CTC_CLI_GET_UINT32_RANGE("my discr", p_bfd_lmep->local_discr, argv[0], 0, CTC_MAX_UINT32_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
        }

        index = CTC_CLI_GET_ARGC_INDEX("nhid");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("Nhid", p_bfd_lmep->nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("mpls ttl", p_bfd_lmep->ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-exp");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("mpls-exp", p_bfd_lmep->tx_cos_exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("without-gal");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_WITHOUT_GAL);
        }

        index = CTC_CLI_GET_ARGC_INDEX("lm-en");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN);
            CTC_CLI_GET_UINT8_RANGE("LM Cos type", p_bfd_lmep->lm_cos_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            CTC_CLI_GET_UINT8_RANGE("LM Cos ", p_bfd_lmep->lm_cos, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("csf-en");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN);
            CTC_CLI_GET_UINT8_RANGE("CSF type", p_bfd_lmep->tx_csf_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mep-id");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_TP_WITH_MEP_ID);
            if (_oam_cli_build_bfd_mepid(p_bfd_lmep->mep_id, &p_bfd_lmep->mep_id_len, argv[index + 1]))
            {
                ctc_cli_out("%% invalid megid \n");
                return CLI_ERROR;
            }
        }
    }

    /*common*/
    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("binding-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("aps-enable");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_APS_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-on-cpu");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sbfd-initiator");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR);
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("desired-tx-interval");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("Desired Tx interval", p_bfd_lmep->desired_min_tx_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("state");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("State", p_bfd_lmep->local_state, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("diag");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("Diag", p_bfd_lmep->local_diag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detect-mult");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("Detect mult", p_bfd_lmep->local_detect_mult, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("Master gchip id", p_bfd_lmep->master_gchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("lmep-index", lmep.lmep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_add_lmep(g_api_lchip, &lmep);
    }
    else
    {
        ret = ctc_oam_add_lmep(&lmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret ;
}

CTC_CLI(ctc_cli_oam_bfd_remove_lmep,
        ctc_cli_oam_bfd_remove_lmep_cmd,
        "local-mep bfd remove my-discr MY_DISCR (sbfd-reflector|)\
        (ip-bfd | micro-bfd | mpls-bfd | tp-bfd (section-oam (gport GPORT_ID |ifid IFID) | ((space SPACEID|)  mpls-in-label IN_LABEL)))",
        "Local MEP",
        "BFD MEP",
        "Remove local BFD MEP",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "Is sbfd reflector",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>")
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0;
    ctc_oam_lmep_t lmep;

    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", lmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", lmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_remove_lmep(g_api_lchip, &lmep);
    }
    else
    {
        ret = ctc_oam_remove_lmep(&lmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret ;

}




/* remote mep clis */
#define M_RMEP ""
CTC_CLI(ctc_cli_oam_cfm_add_rmep,
        ctc_cli_oam_cfm_add_rmep_cmd,
        "remote-mep add rmep-id MEP_ID ({mep-en | csf-en}|)\
    (eth-oam level LEVEL rmep-sa-mac MAC_ADDR ({seq-num-update-enable|mac-update-enable}|) | tp-y1731 ) (mep-index IDX|)",
        "Remote MEP",
        "Add remote MEP on oam channel",
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC,
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        "Enable CSF",
        "Ethernet OAM option",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "Remote MEP MAC SA",
        CTC_CLI_MAC_FORMAT,
        "Enable sequence number update",
        "Enable MAC update",
        "MPLS TP Y.1731 OAM option",
        "MEP index allocated by system",
        "2~max MEP index")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    ctc_oam_rmep_t rmep;
    ctc_oam_y1731_rmep_t* p_y1731_rmep  = NULL;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    p_y1731_rmep = &rmep.u.y1731_rmep;

    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));

    /*common*/
    CTC_CLI_GET_UINT16_RANGE("rmep id", p_y1731_rmep->rmep_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (index != 0xFF)
    {
        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_MEP_EN;   /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-en");
    if (index != 0xFF)
    {
        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_CSF_EN;   /*FLAG*/
    }

    /*ether oam mep_type check*/
    index = CTC_CLI_GET_ARGC_INDEX("eth-oam");
    if (0xFF != index)
    {
        if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == rmep.key.mep_type)
        {
            ret = CTC_E_INVALID_CONFIG;
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    /*TP-Y1731 mep_type check*/
    index = CTC_CLI_GET_ARGC_INDEX("tp-y1731");
    if (0xFF != index)
    {
        if ((CTC_OAM_MEP_TYPE_ETH_1AG == rmep.key.mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == rmep.key.mep_type))
        {
            ret = CTC_E_INVALID_CONFIG;
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-sa-mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("rmep-sa-mac", p_y1731_rmep->rmep_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-num-update-enable");
    if (index != 0xFF)
    {
        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN;   /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-update-enable");
    if (index != 0xFF)
    {
        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_MAC_UPDATE_EN;   /*FLAG*/
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("rmep-index", rmep.rmep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_add_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_add_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_remove_rmep,
        ctc_cli_oam_cfm_remove_rmep_cmd,
        "remote-mep remove (level LEVEL|) (rmep-id MEP_ID)",
        "Remote MEP",
        "Remove remote MEP on oam channel",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_rmep_t rmep;
    ctc_oam_y1731_rmep_t* p_eth_rmep  = NULL;
    uint8 index = 0;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    p_eth_rmep = &rmep.u.y1731_rmep;

    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xff != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xff != index)
    {
        CTC_CLI_GET_UINT16_RANGE("rmep id", p_eth_rmep->rmep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_remove_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_remove_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_oam_bfd_add_rmep,
        ctc_cli_oam_bfd_add_rmep_cmd,
        "remote-mep bfd add my-discr MY_DISCR your-discr YOUR_DISCR (sbfd-reflector|)({mep-en  | required-rx-interval REQUIRED_RX_INTERVAL | state STATE | diag DIAG | detect-mult DETECT_MULT }|)\
        (ip-bfd | micro-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL) | mep-id MEP_ID }) (mep-index IDX|)",
        "Remote MEP",
        "BFD MEP",
        "Add remote BFD MEP",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "Your Discriminator",
        "Your Discriminator value, <0-4294967295>",
        "Is sbfd reflector",
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        "BFD required min rx interval",
        "unit: ms",
        "BFD remote state",
        "Admin down :0 ; Down: 1; Init : 2; Up :3",
        "BFD remote diag",
        "<0-31>",
        "BFD remote detect mult",
        "<1-15>",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Source MEP ID TLV",
        "Source MEP ID TLV value",
        "MEP index allocated by system",
        "2~max MEP index")
{
    uint8 index = 0;
    int32 ret   = CLI_SUCCESS;
    ctc_oam_rmep_t rmep;
    ctc_oam_bfd_rmep_t* p_bfd_rmep  = NULL;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));
    p_bfd_rmep = &rmep.u.bfd_rmep;

    /*common*/
    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", rmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", rmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mep-id");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_TP_WITH_MEP_ID);
            if( _oam_cli_build_bfd_mepid(p_bfd_rmep->mep_id, &p_bfd_rmep->mep_id_len, argv[index + 1]))
            {
                ctc_cli_out("%% invalid megid \n");
                return CLI_ERROR;
            }
        }
    }

    CTC_CLI_GET_UINT32_RANGE("your discr", p_bfd_rmep->remote_discr, argv[1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(p_bfd_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("required-rx-interval");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("Required Rx interval", p_bfd_rmep->required_min_rx_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("state");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("State", p_bfd_rmep->remote_state, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("diag");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("Diag", p_bfd_rmep->remote_diag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detect-mult");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("Detect mult", p_bfd_rmep->remote_detect_mult, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("rmep-index", rmep.rmep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_add_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_add_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_bfd_remove_rmep,
        ctc_cli_oam_bfd_remove_rmep_cmd,
        "remote-mep bfd remove my-discr MY_DISCR (sbfd-reflector|)\
        (ip-bfd | micro-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL)})",
        "Remote MEP",
        "BFD MEP",
        "Remove remote BFD MEP",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "Is sbfd reflector",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Sbfd reflector")
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0;
    ctc_oam_rmep_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", rmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", rmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_remove_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_remove_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/*mip clis */
#define M_MIP ""
CTC_CLI(ctc_cli_oam_cfm_add_remove_mip,
        ctc_cli_oam_cfm_add_remove_mip_cmd,
        "mip (add | remove) (gport GPORT_ID vlan VLAN_ID level LEVEL | (space SPACEID |) mpls-label LABEL) (master-gchip GCHIP_ID|)",
        "MIP",
        "Add MIP",
        "Remove MIP",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "MPLS label",
        "Label, <0-1048575>",
        "Master chip id",
        CTC_CLI_GCHIP_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0;
    uint8 addflag   = 0;
    ctc_oam_mip_t mip;

    /* default configure */
    sal_memset(&mip, 0, sizeof(ctc_oam_mip_t));

    /* parse add/remove */
    if (0 == sal_memcmp(argv[0], "a", 1))
    {
        addflag = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (0xff != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", mip.key.u.eth.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("vlan", mip.key.u.eth.vlan_id, argv[index + 3], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("md level", mip.key.u.eth.md_level, argv[index + 5], 0, CTC_MAX_UINT8_VALUE);
        mip.key.mep_type = mep_type;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-label");
    if (0xff != index)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", mip.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        mip.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    }

    index = CTC_CLI_GET_ARGC_INDEX("space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("MPLS label space", mip.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("Master gchip id", mip.master_gchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if (addflag)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_add_mip(g_api_lchip, &mip);
        }
        else
        {
            ret = ctc_oam_add_mip(&mip);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_oam_remove_mip(g_api_lchip, &mip);
        }
        else
        {
            ret = ctc_oam_remove_mip(&mip);
        }
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_oam_bfd_update_lmep,
        ctc_cli_oam_bfd_update_lmep_cmd,
        "local-mep bfd update ((ip-bfd | micro-bfd |mpls-bfd) my-discr MY_DISCR (sbfd-reflector|)| tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL)}) \
        ((mep-en | cc | cv | csf-en | csf-tx | lm-en | lock ) (enable| disable) | tx-cos COS | sf-state SF_STATE | csf-type CSF_TYPE | d-csf clear | lm-cos-type LM_COS_TYPE |lm-cos LM_COS | \
        diag DIAG | state STATE | my-discr MY_DISCR | nhop NHOP | tx-timer MIN_INTERVAL DETECT_MULT | actual-tx-timer MIN_INTERVAL | ttl TTL | tx-rx-timer TX_INTERVAL DETECT_MULT RX_INTERVAL DETECT_MULT | master-gchip GCHIP_ID)",
        "Local MEP",
        "BFD",
        "Update Local BFD MEP",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "Is sbfd reflector",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        "BFD CC Enable",
        "BFD CV Enable(for TP BFD)",
        "Client signal fail(for TP BFD)",
        "CSF Tx Enable(for TP BFD)",
        "LM Enable(for TP BFD)",
        "Lock Enable(for TP BFD)",
        "Enable",
        "Disable",
        "Set tx cos",
        CTC_CLI_COS_RANGE_DESC,
        "Set signal fail state",
        "<0-1>",
        "Client signal fail type(for TP BFD)",
        "LOS: 0, FDI: 1, RDI: 2, DCI: 3",
        "CSF defect(for TP BFD)",
        "Clear CSF defect",
        "Loss measurement cos type(for TP BFD)",
        "All cos : 0 ; Specified-cos : 1; Per-cos : 2",
        "Loss measurement cos(for TP BFD)",
        CTC_CLI_COS_RANGE_DESC,
        "BFD local diag",
        "<0-31>",
        "BFD local state",
        "Admin down :0 ; Down: 1; Init : 2; Up :3",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "Tx timer",
        "Desired min tx interval",
        "Local detection interval multiplier",
        "Actual Tx timer",
        "Actual min Tx interval",
        "TTL",
        "TTL value",
        "Master chip id",
        "Tx Rx timer",
        "Desired min tx interval",
        "Local detection interval multiplier",
        "Required min rx interval",
        "Remote detection interval multiplier",
        "Label",
        "Label",
        "Space id",
        CTC_CLI_GCHIP_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0xFF;
    ctc_oam_update_t  lmep;
    ctc_oam_bfd_timer_t oam_bfd_timer;
    ctc_oam_bfd_timer_t tx_rx_timer[2];
    ctc_oam_tp_key_t    tp_oam_key;
    sal_memset(&tx_rx_timer, 0, sizeof(ctc_oam_bfd_timer_t) * 2);
    sal_memset(&oam_bfd_timer, 0, sizeof(ctc_oam_bfd_timer_t));
    sal_memset(&lmep, 0, sizeof(ctc_oam_update_t));
    sal_memset(&tp_oam_key, 0, sizeof(ctc_oam_tp_key_t));
    lmep.is_local   = 1;

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", lmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", lmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", lmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", lmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    /*common*/
    index = CTC_CLI_GET_ARGC_INDEX("my-discr");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_DISCR;
        CTC_CLI_GET_UINT32_RANGE("My discr", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_MEP_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cc");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_CC_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cv");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_CV_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-en");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_CSF_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-tx");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    /*LM*/
    index = CTC_CLI_GET_ARGC_INDEX("lm-en");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("lock");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LOCK;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            lmep.update_value = 1;
        }
        else
        {
            lmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-cos");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_COS_EXP;
        CTC_CLI_GET_UINT8_RANGE("Tx Cos", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sf-state");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LMEP_SF_STATE;
        CTC_CLI_GET_UINT8_RANGE("SF state", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-type");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_TYPE;
        CTC_CLI_GET_UINT8_RANGE("CSF type", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("d-csf");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_DCSF;
        lmep.update_value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lm-cos-type");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS_TYPE;
        CTC_CLI_GET_UINT8_RANGE("LM Cos Type", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lm-cos");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS;
        CTC_CLI_GET_UINT8_RANGE("LM Cos", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("diag");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_DIAG;
        CTC_CLI_GET_UINT8_RANGE("Diag", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("state");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_STATE;
        CTC_CLI_GET_UINT8_RANGE("state", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX("nhop");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_NHOP;
        CTC_CLI_GET_UINT32_RANGE("nhop", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX("tx-timer");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_TIMER;
         /*CTC_CLI_GET_UINT16_RANGE("Actual Tx Interval", oam_bfd_timer.actual_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);*/
        CTC_CLI_GET_UINT16_RANGE("Min Tx Interval", oam_bfd_timer.min_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("Detect mul", oam_bfd_timer.detect_mult, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        lmep.p_update_value = &oam_bfd_timer;
    }

    index = CTC_CLI_GET_ARGC_INDEX("actual-tx-timer");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_ACTUAL_TX_TIMER;
        CTC_CLI_GET_UINT16_RANGE("Min Tx Interval", oam_bfd_timer.min_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        lmep.p_update_value = &oam_bfd_timer;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TTL;
        CTC_CLI_GET_UINT8_RANGE("", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx-rx-timer");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_RX_TIMER;
        CTC_CLI_GET_UINT16_RANGE("Min Tx Interval", tx_rx_timer[0].min_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("Detect mul", tx_rx_timer[0].detect_mult, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT16_RANGE("Min Rx Interval", tx_rx_timer[1].min_interval, argv[index + 3], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("Detect mul", tx_rx_timer[1].detect_mult, argv[index + 4], 0, CTC_MAX_UINT8_VALUE);
        lmep.p_update_value = &tx_rx_timer;
    }
    index = CTC_CLI_GET_ARGC_INDEX("label");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_LABEL;
        CTC_CLI_GET_UINT32_RANGE("label", tp_oam_key.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        CTC_CLI_GET_UINT8_RANGE("space id", tp_oam_key.mpls_spaceid, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        lmep.p_update_value = &tp_oam_key;
    }
    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (0xFF != index)
    {
        lmep.update_type = CTC_OAM_BFD_LMEP_UPDATE_TYPE_MASTER_GCHIP;
        CTC_CLI_GET_UINT32_RANGE("master-gchip", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

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

CTC_CLI(ctc_cli_oam_bfd_update_rmep,
        ctc_cli_oam_bfd_update_rmep_cmd,
        "remote-mep bfd update ((ip-bfd | micro-bfd |mpls-bfd) my-discr MY_DISCR (sbfd-reflector|)| tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL)}) \
         ((mep-en | learn-en | hw-aps-en) (enable| disable) | sf-state SF_STATE | diag DIAG | state STATE | your-discr YOUR_DISCR | rx-timer MIN_INTERVAL DETECT_MULT | actual-rx-timer MIN_INTERVAL  \
         | hw-aps APS_GRP (is-protect|))",
        "Remote MEP",
        "BFD",
        "Update remote BFD MEP",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "My Discriminator",
        "My Discriminator value, <0-4294967295>",
        "Is sbfd reflector",
        "MPLS TP BFD OAM option",
        CTC_CLI_OAM_SECTION_OAM_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        CTC_CLI_OAM_MEP_ACTIVE_DESC,
        "Mep learning is enable or disable",
        "HW APS enable",
        "Enable",
        "Disable",
        "Set signal fail state",
        "<0-1>",
        "BFD remote diag",
        "<0-31>",
        "BFD remote state",
        "Admin down :0 ; Down: 1; Init : 2; Up :3",
        "Your Discriminator",
        "Your Discriminator value, <0-4294967295>",
        "Rx timer",
        "Required min rx interval",
        "Remote detection interval multiplier",
        "Actual Rx timer",
        "Actual min Rx interval",
        "Hw APS",
        "Aps Group ID",
        "Protecting path")
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0xFF;
    ctc_oam_update_t  rmep;
    ctc_oam_bfd_timer_t oam_bfd_timer;
    ctc_oam_hw_aps_t  oam_aps;

    sal_memset(&rmep, 0, sizeof(ctc_oam_update_t));
    rmep.is_local   = 0;

    sal_memset(&oam_bfd_timer, 0, sizeof(ctc_oam_bfd_timer_t));
    sal_memset(&oam_aps, 0, sizeof(ctc_oam_hw_aps_t));

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("micro-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MICRO_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", rmep.key.u.bfd.discr, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT32_RANGE("gport", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", rmep.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", rmep.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", rmep.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    /*common*/
    index = CTC_CLI_GET_ARGC_INDEX("mep-en");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_MEP_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            rmep.update_value = 1;
        }
        else
        {
            rmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("learn-en");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_LEARN_EN;
        if (0 == sal_strcmp(argv[index + 1], "enable"))
        {
            rmep.update_value = 1;
        }
        else
        {
            rmep.update_value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-aps-en");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS_EN;
        rmep.update_value = (0 == sal_strcmp(argv[index + 1], "enable")) ? 1 : 0;

    }

    index = CTC_CLI_GET_ARGC_INDEX("sf-state");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_RMEP_SF_STATE;
        CTC_CLI_GET_UINT8_RANGE("SF state", rmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("diag");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_DIAG;
        CTC_CLI_GET_UINT8_RANGE("Remote diag", rmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("state");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_STATE;
        CTC_CLI_GET_UINT8_RANGE("Remote state", rmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("your-discr");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_DISCR;
        CTC_CLI_GET_UINT32_RANGE("Your discr", rmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx-timer");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_RX_TIMER;
        CTC_CLI_GET_UINT16_RANGE("Min Rx Interval", oam_bfd_timer.min_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("Detect mul", oam_bfd_timer.detect_mult, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        rmep.p_update_value = &oam_bfd_timer;
    }

    index = CTC_CLI_GET_ARGC_INDEX("actual-rx-timer");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_ACTUAL_RX_TIMER;
        CTC_CLI_GET_UINT16_RANGE("Min Rx Interval", oam_bfd_timer.min_interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        rmep.p_update_value = &oam_bfd_timer;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-aps");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS;
        CTC_CLI_GET_UINT16_RANGE("aps group id", oam_aps.aps_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX("is-protect");
        if (0xFF != index)
        {
            oam_aps.protection_path= 1;
        }
        rmep.p_update_value = &oam_aps;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/* port related clis */
#define M_OAM_EN ""
CTC_CLI(ctc_cli_oam_cfm_set_port_oam_enable,
        ctc_cli_oam_cfm_set_port_oam_enable_cmd,
        "oam cfm port GPORT_ID (ingress| egress| both-direction) (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        "Both direction",
        "Enable OAM on this port",
        "Disable OAM on this port")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_direction_t dir = 0;
    uint8 enable = 0;
    ctc_oam_property_t oam_prop;
    ctc_oam_y1731_prop_t* p_eth_prop = NULL;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    p_eth_prop  = &oam_prop.u.y1731;

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both-direction");
    if (index != 0xFF)
    {
        dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;

    }
    else
    {
        enable = 0;
    }

    p_eth_prop->cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN;
    p_eth_prop->gport  = gport;
    p_eth_prop->dir    = dir;
    p_eth_prop->value = enable;

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

CTC_CLI(ctc_cli_oam_cfm_set_port_enable_port_tunnel,
        ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd,
        "oam cfm port GPORT_ID tunnel (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "OAM tunnel",
        "Enable OAM tunnel on this port",
        "Disable OAM tunnel on this port")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 enable = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("enable");


    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN;
    oam_prop.u.y1731.gport  = gport;
    oam_prop.u.y1731.value  = enable;

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

/* set all oam pdu to cpu */
CTC_CLI(ctc_cli_oam_set_all_pdu_to_cpu,
        ctc_cli_oam_set_all_pdu_to_cpu_cmd,
        "oam relay-all-pkt-to-cpu (enable| disable)",
        CTC_CLI_OAM_M_STR,
        "Enable or disable all OAM pdu relay to cpu",
        "Enable all pkt to cpu",
        "Disable all pkt to cpu")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 enable = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_ALL_TO_CPU;
    oam_prop.u.y1731.value  = enable;

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

/* configure edge port cli */
CTC_CLI(ctc_cli_oam_cfm_add_edge_port,
        ctc_cli_oam_cfm_add_edge_port_cmd,
        "oam cfm edge-port config port GPORT_ID",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "OAM edge port",
        "Configure edge port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_EDGE_PORT_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;

    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_ADD_EDGE_PORT;
    oam_prop.u.y1731.gport    = gport;

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

/* remove edge port cli */
CTC_CLI(ctc_cli_oam_cfm_remove_edge_port,
        ctc_cli_oam_cfm_remove_edge_port_cmd,
        "oam cfm edge-port remove port GPORT_ID",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "OAM edge port",
        "Remove edge port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_EDGE_PORT_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_REMOVE_EDGE_PORT;
    oam_prop.u.y1731.gport    = gport;
    oam_prop.u.y1731.value    = 0;

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

/* set oam tx tpid */
CTC_CLI(ctc_cli_oam_cfm_set_tx_vlan_tpid,
        ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd,
        "oam tx-vlan-tpid set tpid-index TPID_INDEX tpid TPID",
        CTC_CLI_OAM_M_STR,
        "Vlan tpid for OAM pdu tx",
        "Set tpid",
        "Tpid index",
        CTC_CLI_OAM_MEP_TX_TPID_TYPE_VALUE_DESC,
        "Tpid",
        "<0-0xFFFF>")
{
    int32 ret = CLI_SUCCESS;
    uint8 tpid_index = 0;
    uint16 tpid = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT8_RANGE("tpid index", tpid_index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("tpid", tpid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_TX_VLAN_TPID;
    oam_prop.u.y1731.value = (tpid_index << 16 | tpid);

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

CTC_CLI(ctc_cli_oam_cfm_set_rx_vlan_tpid,
        ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd,
        "oam rx-vlan-tpid set tpid-index TPID_INDEX tpid TPID",
        CTC_CLI_OAM_M_STR,
        "Vlan tpid for OAM pdu rx",
        "Set tpid",
        "Tpid index",
        CTC_CLI_OAM_MEP_TX_TPID_TYPE_VALUE_DESC,
        "Tpid",
        "0xXXXX")
{
    int32 ret = CLI_SUCCESS;
    uint8 tpid_index = 0;
    uint16 tpid = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT8_RANGE("tpid index", tpid_index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("tpid", tpid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_RX_VLAN_TPID;
    oam_prop.u.y1731.value = (tpid_index << 16 | tpid);

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

CTC_CLI(ctc_cli_oam_cfm_set_sender_id_tlv,
        ctc_cli_oam_cfm_set_sender_id_tlv_cmd,
        "oam cfm set sender-id SENDER_ID",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set sender id",
        "Sender id",
        "Sender id string")
{
    int32 ret   = CLI_SUCCESS;
    uint8 len   = 0;
    ctc_oam_property_t oam_prop;
    ctc_oam_eth_senderid_t eth_oam_senderid;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));
    sal_memset(&eth_oam_senderid, 0, sizeof(ctc_oam_eth_senderid_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    len = sal_strlen(argv[0]);
    if (len > CTC_OAM_ETH_MAX_SENDER_ID_LEN)
    {
        ctc_cli_out("%% Sender id length %d larger than %d\n", len, CTC_OAM_ETH_MAX_SENDER_ID_LEN);
    }

    eth_oam_senderid.sender_id_len = ((len <= CTC_OAM_ETH_MAX_SENDER_ID_LEN) ? len : CTC_OAM_ETH_MAX_SENDER_ID_LEN);
    sal_memcpy(&eth_oam_senderid.sender_id, argv[0], CTC_OAM_ETH_MAX_SENDER_ID_LEN);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_SENDER_ID;
    oam_prop.u.y1731.p_value  = &eth_oam_senderid;

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

CTC_CLI(ctc_cli_oam_cfm_set_bridge_mac,
        ctc_cli_oam_cfm_set_bridge_mac_cmd,
        "oam cfm set bridge-mac BRIDGE_MAC",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set",
        "Bridge mac",
        CTC_CLI_MAC_FORMAT)
{
    int32 ret   = CLI_SUCCESS;
    ctc_oam_property_t  oam_prop;
    mac_addr_t          mac_addr;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));
    sal_memset(&mac_addr, 0, sizeof(mac_addr_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_MAC_ADDRESS("bridge-mac", mac_addr, argv[0]);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC;
    oam_prop.u.y1731.p_value  = &mac_addr;

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

CTC_CLI(ctc_cli_oam_cfm_set_tp_y1731_ach_chan,
        ctc_cli_oam_cfm_set_tp_y1731_ach_chan_cmd,
        "oam cfm set tp-y1731-chan-type TYPE",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set",
        "TP Y1731 channel type",
        "TP Y1731 channel type value")
{
    int32 ret   = CLI_SUCCESS;
    uint16 Y1731_ach_chan_type = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));
    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("tp-y1731-chan-type", Y1731_ach_chan_type, argv[0], 0, CTC_MAX_UINT16_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE;
    oam_prop.u.y1731.value  = Y1731_ach_chan_type;

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

CTC_CLI(ctc_cli_oam_cfm_set_rdi_mode,
        ctc_cli_oam_cfm_set_rdi_mode_cmd,
        "oam cfm set rdi-mode MODE",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set",
        "Y1731 RDI mode",
        "RDI mode")
{
    int32 ret   = CLI_SUCCESS;
    uint8 rdi_mode = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));
    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT8_RANGE("rdi-mode", rdi_mode, argv[0], 0, CTC_MAX_UINT8_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_RDI_MODE;
    oam_prop.u.y1731.value  = rdi_mode;

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


CTC_CLI(ctc_cli_oam_set_big_interval_to_cpu,
        ctc_cli_oam_set_big_interval_to_cpu_cmd,
        "oam cfm big-ccm-interval INTERVAL",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Big interval to CPU",
        "The big interval")
{
    int32 ret       = CLI_SUCCESS;
    uint8 interval  = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT8_RANGE("big interval", interval, argv[0], 0, CTC_MAX_UINT8_VALUE);

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU;
    oam_prop.u.y1731.value    = interval;

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

CTC_CLI(ctc_cli_oam_set_lbm_proc_in_asic,
        ctc_cli_oam_set_lbm_proc_in_asic_cmd,
        "oam cfm (lbm-proc-in-asic | lmm-proc-in-asic | dm-proc-in-asic | slm-proc-in-asic) (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set LBM process in ASIC",
        "Set LMM process in ASIC",
        "Set DM process in ASIC",
        "Set SLM process in ASIC",
        "Enable LBM process in ASIC",
        "Disable LBM process in ASIC")
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0;
    uint8 enable    = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lbm-proc-in-asic");
    if (index != 0xFF)
    {
        oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lmm-proc-in-asic");
    if (index != 0xFF)
    {
        oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dm-proc-in-asic");
    if (index != 0xFF)
    {
        oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("slm-proc-in-asic");
    if (index != 0xFF)
    {
        oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC;
    }

    oam_prop.u.y1731.value    = enable;

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

CTC_CLI(ctc_cli_oam_set_lbr_use_lbm_da,
        ctc_cli_oam_set_lbr_use_lbm_da_cmd,
        "oam cfm lbr-sa-use-lbm-da (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set reply LBR MAC SA mode",
        "Enable LBR MAC SA use LBM DA",
        "Disable LBR MAC SA use LBM DA")
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0;
    uint8 enable    = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA;
    oam_prop.u.y1731.value    = enable;

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

CTC_CLI(ctc_cli_oam_set_lbr_use_share_mac,
        ctc_cli_oam_set_lbr_use_share_mac_cmd,
        "oam cfm lbr-sa-use-share-mac (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set reply LBR MAC SA mode",
        "Enable LBR MAC SA use share Mac",
        "Disable LBR MAC SA use share Mac")
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0;
    uint8 enable    = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_LBR_SA_SHARE_MAC;
    oam_prop.u.y1731.value    = enable;

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


CTC_CLI(ctc_cli_oam_set_vpls_lkup_mode,
        ctc_cli_oam_set_vpls_lkup_mode_cmd,
        "oam cfm l2vpn-oam-mode (oam-id| vlan)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Set L2vpn OAM lookup mode",
        "Use Port+Oam ID key",
        "Use Port+Vlan key")
{
    int32 ret       = CLI_SUCCESS;
    uint8 index     = 0;
    ctc_oam_property_t  oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    index = CTC_CLI_GET_ARGC_INDEX("oam-id");
    if (index != 0xFF)
    {
        oam_prop.u.y1731.value = 0;
    }
    else
    {
        oam_prop.u.y1731.value = 1;
    }
    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE;

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


CTC_CLI(ctc_cli_oam_cfm_set_port_lm_enable,
        ctc_cli_oam_cfm_set_port_lm_enable_cmd,
        "oam cfm lm port GPORT_ID (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Frame loss Measurement",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Enable LM on this port",
        "Disable LM on this port")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 enable = 0;
    ctc_oam_property_t oam_prop;
    ctc_oam_y1731_prop_t* p_eth_prop = NULL;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    p_eth_prop  = &oam_prop.u.y1731;
    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    p_eth_prop->cfg_type    = CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN;
    p_eth_prop->gport       = gport;
    p_eth_prop->value       = enable;

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

CTC_CLI(ctc_cli_oam_cfm_set_poam_lm_enable,
        ctc_cli_oam_cfm_set_poam_lm_enable_cmd,
        "oam cfm poam lm (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Proactive OAM frames",
        "Frame loss Measurement",
        "Enable Eth-LM count POAM PDUs",
        "Disable Eth-LM count POAM PDUs")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 enable = 0;
    ctc_oam_property_t oam_prop;
    ctc_oam_y1731_prop_t* p_eth_prop = NULL;
    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    p_eth_prop  = &oam_prop.u.y1731;
    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }
    p_eth_prop->cfg_type    = CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN;
    p_eth_prop->value       = enable;
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

#define M_OAM_UPDATE ""
CTC_CLI(ctc_cli_oam_cfm_update_lmep_enable,
        ctc_cli_oam_cfm_update_lmep_enable_cmd,
        "local-mep (enable| disable) (level LEVEL|)",
        "Local MEP",
        "Enable local MEP on OAM channel",
        "Disable local MEP on OAM channel",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 mep_enable = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    /* default configure */
    sal_memset(&lmep, 0, sizeof(ctc_oam_update_t));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));

    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        mep_enable = 1;
    }
    else
    {
        mep_enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN;
    lmep.update_value = mep_enable;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_ccm_enable,
        ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd,
        "local-mep ccm (enable| disable) (level LEVEL|)",
        "Local MEP",
        "Tx ccm configure",
        "Enable ccm for local MEP",
        "Disable ccm for local MEP",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret   = CLI_SUCCESS;
    uint8 ccm_enable = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    /* default configure */
    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        ccm_enable = 1;
    }
    else
    {
        ccm_enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN;
    lmep.update_value = ccm_enable;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_tx_cos,
        ctc_cli_oam_cfm_update_lmep_tx_cos_cmd,
        "local-mep tx-cos set COS (level LEVEL|)",
        "Local MEP",
        "COS for tx OAM PDU",
        "Set tx cos",
        "<0-7>",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 cos = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    CTC_CLI_GET_UINT8_RANGE("cos", cos, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP;
    lmep.update_value = cos;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_mel,
        ctc_cli_oam_cfm_update_lmep_mel_cmd,
        "local-mep level LEVEL",
        "Local MEP",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 level = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    CTC_CLI_GET_UINT8_RANGE("md level", level, argv[0], 0, CTC_MAX_UINT8_VALUE);

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL;
    lmep.update_value   = level;

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


CTC_CLI(ctc_cli_oam_cfm_clear_lmep_rdi,
        ctc_cli_oam_cfm_clear_lmep_rdi_cmd,
        "local-mep rdi clear (level LEVEL|)",
        "Local MEP",
        "RDI status",
        "Clear the defect status",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));
    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI;
    lmep.update_value = 0;

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

/* port status update cli */
CTC_CLI(ctc_cli_oam_cfm_update_port_status,
        ctc_cli_oam_cfm_update_port_status_cmd,
        "oam cfm update-port-status port GPORT_ID vlan VLAN_ID port-status PORT_STATUS",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Update local MEP port status",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Port status",
        "Status")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint16 vlan_id = 0;
    uint8 port_state_num = 0;
    uint8 port_status;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));
    lmep.is_local = 1;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_id, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("port status", port_state_num, argv[2], 0, CTC_MAX_UINT8_VALUE);

    switch (port_state_num)
    {
    case 1:
        port_status = CTC_OAM_ETH_PS_BLOCKED;
        break;

    case 2:
        port_status = CTC_OAM_ETH_PS_UP;
        break;

    default:
        ctc_cli_out("%% %s \n", "port status value must be 1-2");
        return CLI_ERROR;
    }


    lmep.key.mep_type = mep_type;
    lmep.key.u.eth.gport = gport;
    lmep.key.u.eth.vlan_id = vlan_id;

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS;
    lmep.update_value = port_status;

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

/* interface status update cli */
CTC_CLI(ctc_cli_oam_cfm_update_intf_status,
        ctc_cli_oam_cfm_update_intf_status_cmd,
        "oam cfm update-intf-status port GPORT_ID intf-status INTF_STATUS",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        "Update interface status",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Interface status",
        "Status")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint16 intf_status_num = 0;
    uint8 intf_status;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));
    lmep.is_local = 1;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("interface status", intf_status_num, argv[1], 0, CTC_MAX_UINT8_VALUE);

    switch (intf_status_num)
    {
    case 1:
        intf_status = CTC_OAM_ETH_INTF_IS_UP;
        break;

    case 2:
        intf_status = CTC_OAM_ETH_INTF_IS_DOWN;
        break;

    case 3:
        intf_status = CTC_OAM_ETH_INTF_IS_TESTING;
        break;

    case 4:
        intf_status = CTC_OAM_ETH_INTF_IS_UNKNOWN;
        break;

    case 5:
        intf_status = CTC_OAM_ETH_INTF_IS_DORMANT;
        break;

    case 6:
        intf_status = CTC_OAM_ETH_INTF_IS_NOT_PRESENT;
        break;

    case 7:
        intf_status = CTC_OAM_ETH_INTF_IS_LOWER_LAYER_DOWN;
        break;

    default:
        ctc_cli_out("%% %s \n", "interface status value must be 1-7");
        return CLI_ERROR;
    }


    lmep.key.mep_type = mep_type;
    lmep.key.u.eth.gport = gport;

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_IF_STATUS;
    lmep.update_value = intf_status;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_sf_state,
        ctc_cli_oam_cfm_update_lmep_sf_state_cmd,
        "local-mep sf-state SF_STATE (level LEVEL|)",
        "Local MEP",
        "Set signal fail state",
        "<0-1>",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 sf_state  = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    CTC_CLI_GET_UINT8_RANGE("signal fail state", sf_state, argv[0], 0, CTC_MAX_UINT8_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE;
    lmep.update_value = sf_state;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_seq_num_en,
        ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd,
        "local-mep seq-num (enable | disable) level LEVEL",
        "Local MEP",
        "CCM sequence number",
        "Enable CCM sequence number",
        "Disable CCM sequence number",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret           = CLI_SUCCESS;
    uint8 seq_num_en    = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        seq_num_en = 1;
    }
    else
    {
        seq_num_en = 0;
    }

    CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[1], 0, CTC_MAX_UINT8_VALUE);

    lmep.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN;
    lmep.update_value = seq_num_en;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en,
        ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd,
        "local-mep p2p-ccm-tx-use-uc-da (enable | disable) level LEVEL",
        "Local MEP",
        "CCM use unicast Mac Da in P2P mode",
        "Enable CCM use unicast Mac Da in P2P mode",
        "Disable CCM use unicast Mac Da in P2P mode",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret                   = CLI_SUCCESS;
    uint8 ccm_use_uc_da_p2p     = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        ccm_use_uc_da_p2p = 1;
    }
    else
    {
        ccm_use_uc_da_p2p = 0;
    }

    CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[1], 0, CTC_MAX_UINT8_VALUE);

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA;
    lmep.update_value   = ccm_use_uc_da_p2p;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_dm_en,
        ctc_cli_oam_cfm_update_lmep_dm_en_cmd,
        "local-mep dm (enable | disable) (level LEVEL|)",
        "Local MEP",
        "Delay measurment",
        "Enable delay measurment",
        "Disable delay measurment",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 dm_en     = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        dm_en = 1;
    }
    else
    {
        dm_en = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN;
    lmep.update_value   = dm_en;

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


CTC_CLI(ctc_cli_oam_cfm_update_lmep_lock_en,
        ctc_cli_oam_cfm_update_lmep_lock_en_cmd,
        "local-mep lock (enable | disable)",
        "Local MEP",
        "Lock function",
        "Enable lock",
        "Disable lock")
{
    int32 ret       = CLI_SUCCESS;
    uint8 lock_en     = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        lock_en = 1;
    }
    else
    {
        lock_en = 0;
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK;
    lmep.update_value   = lock_en;

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


CTC_CLI(ctc_cli_oam_cfm_update_lmep_csf_en,
        ctc_cli_oam_cfm_update_lmep_csf_en_cmd,
        "local-mep csf-tx (enable | disable) (level LEVEL|)",
        "Local MEP",
        "Client signal fail",
        "Enable Client signal fail",
        "Disable Client signal fail",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 csf_en     = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        csf_en = 1;
    }
    else
    {
        csf_en = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN;
    lmep.update_value   = csf_en;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_csf_type,
        ctc_cli_oam_cfm_update_lmep_csf_type_cmd,
        "local-mep csf-type ( los | fdi | rdi | dci) (level LEVEL|)",
        "Local MEP",
        "Client signal fail type",
        "Client loss of signal",
        "Client forward defect indication",
        "Client remote defect indication",
        "Client defect clear indication",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 csf_type  = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    if (0 == sal_memcmp(argv[0], "los", 3))
    {
        csf_type = CTC_OAM_CSF_TYPE_LOS;
    }
    else if (0 == sal_memcmp(argv[0], "fdi", 3))
    {
        csf_type = CTC_OAM_CSF_TYPE_FDI;
    }
    else if (0 == sal_memcmp(argv[0], "rdi", 3))
    {
        csf_type = CTC_OAM_CSF_TYPE_RDI;
    }
    else if (0 == sal_memcmp(argv[0], "dci", 3))
    {
        csf_type = CTC_OAM_CSF_TYPE_DCI;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE;
    lmep.update_value   = csf_type;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_csf_use_uc_da,
        ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd,
        "local-mep csf-use-uc-da (enable | disable) level LEVEL",
        "Local MEP",
        "Client signal fail Mac Da",
        "Enable CSF Mac Da",
        "Disable CSF Mac Da",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret               = CLI_SUCCESS;
    uint8 csf_mac_da_uc     = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        csf_mac_da_uc = 1;
    }
    else if (0 == sal_memcmp(argv[0], "d", 1))
    {
        csf_mac_da_uc = 0;
    }

    CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[1], 0, CTC_MAX_UINT8_VALUE);

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA;
    lmep.update_value   = csf_mac_da_uc;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_lm_en,
        ctc_cli_oam_cfm_update_lmep_lm_en_cmd,
        "local-mep lm (enable | disable) (level LEVEL|)",
        "Local MEP",
        "Loss measurement",
        "Enable LM",
        "Disable LM",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret               = CLI_SUCCESS;
    uint8 lm_en     = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        lm_en = 1;
    }
    else if (0 == sal_memcmp(argv[0], "d", 1))
    {
        lm_en = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
    lmep.update_value   = lm_en;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_lm_type,
        ctc_cli_oam_cfm_update_lmep_lm_type_cmd,
        "local-mep lm-type (dual | single | none)",
        "Local MEP",
        "Loss measurement type",
        "Loss measurement dual",
        "Loss measurement single",
        "Loss measurement none")
{
    int32 ret           = CLI_SUCCESS;
    uint8 lm_type       = 0;
    ctc_oam_update_t lmep;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    if (0 == sal_memcmp(argv[0], "dual", 4))
    {
        lm_type = CTC_OAM_LM_TYPE_DUAL;
    }
    else if (0 == sal_memcmp(argv[0], "sing", 4))
    {
        lm_type = CTC_OAM_LM_TYPE_SINGLE;
    }
    else if (0 == sal_memcmp(argv[0], "none", 4))
    {
        lm_type = CTC_OAM_LM_TYPE_NONE;
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE;
    lmep.update_value   = lm_type;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_lm_cos_type,
        ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd,
        "local-mep lm-cos-type (all-cos | specified-cos | per-cos) (level LEVEL|)",
        "Local MEP",
        "Loss measurement cos type",
        "Loss measurement all cos packet together",
        "Loss measurement only specified cos packet",
        "Loss measurement per cos",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret           = CLI_SUCCESS;
    uint8 lm_cos_type   = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    if (0 == sal_memcmp(argv[0], "all-cos", 7))
    {
        lm_cos_type = CTC_OAM_LM_COS_TYPE_ALL_COS;
    }
    else if (0 == sal_memcmp(argv[0], "specified-cos", 7))
    {
        lm_cos_type = CTC_OAM_LM_COS_TYPE_SPECIFIED_COS;
    }
    else if (0 == sal_memcmp(argv[0], "per-cos", 7))
    {
        lm_cos_type = CTC_OAM_LM_COS_TYPE_PER_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE;
    lmep.update_value   = lm_cos_type;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep_lm_cos,
        ctc_cli_oam_cfm_update_lmep_lm_cos_cmd,
        "local-mep lm-cos COS (level LEVEL|)",
        "Local MEP",
        "Loss measurement cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 cos       = 0;
    ctc_oam_update_t lmep;
    uint8 index = 0;

    sal_memset(&lmep, 0, sizeof(lmep));

    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    lmep.is_local       = 1;

    CTC_CLI_GET_UINT8_RANGE("lm cos", cos, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS;
    lmep.update_value   = cos;

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

CTC_CLI(ctc_cli_oam_cfm_update_lmep,
        ctc_cli_oam_cfm_update_lmep_cmd,
        "local-mep (nhop NHOP | ttl TTL | master-gchip GCHIP_ID| ccm-interval INTERVAL| mep-id MEP_ID |(meg-id STRING:MEG_ID | \
        md-name STRING:MEG_ID ma-name STRING:MA_NAME | label LABEL SPACEID| ccm-stats (enable|disable)))(level LEVEL|)",
        "Local MEP",
        "NextHop ID for MEP out",
        CTC_CLI_NH_ID_STR,
        "TTL",
        "TTL value",
        "Master chip id",
        CTC_CLI_GCHIP_DESC,
        "CCM interval",
        "Interval",
        "MEP id",
        "MEP id",
        CTC_CLI_OAM_MEG_ID_DESC,
        CTC_CLI_OAM_MEG_ID_VALUE_DESC,
        CTC_CLI_OAM_MD_NAME_DESC,
        CTC_CLI_OAM_MD_NAME_VALUE_DESC,
        CTC_CLI_OAM_MA_NAME_DESC,
        CTC_CLI_OAM_MA_NAME_VALUE_DESC,
        "Label",
        "Label",
        "Space id",
        "CCM stats",
        "Enable",
        "Disable",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret           = CLI_SUCCESS;
    ctc_oam_update_t lmep;
    uint8 index = 0;
    ctc_oam_maid_t maid;
    ctc_oam_tp_key_t    tp_oam_key;

    sal_memset(&lmep, 0, sizeof(lmep));
    sal_memset(&maid, 0, sizeof(maid));
    sal_memcpy(&lmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    sal_memset(&tp_oam_key, 0, sizeof(ctc_oam_tp_key_t));
    lmep.is_local       = 1;

    index = CTC_CLI_GET_ARGC_INDEX("nhop");
    if (0xFF != index)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP;
        CTC_CLI_GET_UINT32_RANGE("nhop", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != index)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL;
        CTC_CLI_GET_UINT8_RANGE("ttl", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (0xFF != index)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP;
        CTC_CLI_GET_UINT32_RANGE("master-gchip", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ccm-interval");
    if (0xFF != index)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL;
        CTC_CLI_GET_UINT8_RANGE("ccm-interval", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-id");
    if (0xFF != index)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID;
        CTC_CLI_GET_UINT16_RANGE("mep-id", lmep.update_value, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("meg-id");
    if (index != 0xff)
    {
        if (_oam_cli_build_megid(&maid, argv[index + 1]))
        {
            ctc_cli_out("%% invalid megid \n");
            return CLI_ERROR;
        }
        maid.mep_type = lmep.key.mep_type;
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID;
        lmep.p_update_value = &maid;
    }

    index = CTC_CLI_GET_ARGC_INDEX("md-name");
    if (index != 0xff)
    {
        if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep.key.mep_type)
        {
            ctc_cli_out("%% MPLS-TP Y.1731 only support ICC MAID \n");
            return CLI_ERROR;
        }
        else
        {
            if (_oam_cli_build_maid(&maid, argv[index + 1], argv[index + 3]))
            {
                ctc_cli_out("%% invalid md name or ma name \n");
                return CLI_ERROR;
            }
            maid.mep_type = lmep.key.mep_type;
            lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID;
            lmep.p_update_value = &maid;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("label");
    if (index != 0xff)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LABEL;
        CTC_CLI_GET_UINT32_RANGE("label", tp_oam_key.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        CTC_CLI_GET_UINT8_RANGE("space id", tp_oam_key.mpls_spaceid, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        lmep.p_update_value = &tp_oam_key;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ccm-stats");
    if (index != 0xff)
    {
        lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN;
        if (0 == sal_memcmp(argv[index + 1], "e", 1))
        {
            lmep.update_value = 1;
        }
        else if (0 == sal_memcmp(argv[index + 1], "d", 1))
        {
            lmep.update_value = 0;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

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


CTC_CLI(ctc_cli_oam_cfm_rmep_enable,
        ctc_cli_oam_cfm_rmep_enable_cmd,
        "remote-mep (enable| disable) level LEVEL rmep-id MEP_ID",
        "Remote MEP",
        "Enable remote MEP",
        "Disable remote MEP",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 enable = 0;
    ctc_oam_update_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));

    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[2], 0, CTC_MAX_UINT16_VALUE);

    rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN;
    rmep.update_value = enable;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_clear_rmep_seq_num_fail,
        ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd,
        "remote-mep seq-num-fail-counter clear level LEVEL rmep-id MEP_ID",
        "Remote MEP",
        "Sequence number check fail counter",
        "Clear the counter",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[1], 0, CTC_MAX_UINT16_VALUE);

    rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_SEQ_FAIL_CLEAR;
    rmep.update_value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_update_rmep_seq_num_en,
        ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd,
        "remote-mep seq-num (enable| disable) level LEVEL rmep-id MEP_ID",
        "Remote MEP",
        "Sequence number",
        "Enable Sequence number",
        "Disable Sequence number",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 enable    = 0;
    ctc_oam_update_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[2], 0, CTC_MAX_UINT16_VALUE);

    rmep.update_type    = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN;
    rmep.update_value   = enable;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_update_rmep_sf_state,
        ctc_cli_oam_cfm_update_rmep_sf_state_cmd,
        "remote-mep sf-state SF_STATE (level LEVEL|) (rmep-id MEP_ID)",
        "Remote MEP",
        "Set signal fail state",
        "<0-1>",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 sf_state  = 0;
    ctc_oam_update_t rmep;
    uint8 index     = 0;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    CTC_CLI_GET_UINT8_RANGE("signal fail state", sf_state, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    rmep.update_type    = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE;
    rmep.update_value   = sf_state;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_clear_rmep_mac_status_fail,
        ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd,
        "remote-mep mac-status clear level LEVEL rmep-id MEP_ID",
        "Remote MEP",
        "Mac status mismatch",
        "Clear Mac status mismatch",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[1], 0, CTC_MAX_UINT16_VALUE);

    rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_PORT_INTF_MIS;
    rmep.update_value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch,
        ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd,
        "remote-mep src-mac-mismatch clear level LEVEL rmep-id MEP_ID",
        "Remote MEP",
        "Remote MEP MAC mismatch",
        "Clear the mismatch status",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t rmep;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[1], 0, CTC_MAX_UINT16_VALUE);

    rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMAC_MIS;
    rmep.update_value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_clear_rmep_dcsf,
        ctc_cli_oam_cfm_clear_rmep_dcsf_cmd,
        "remote-mep d-csf clear (level LEVEL|) (rmep-id MEP_ID)",
        "Remote MEP",
        "CSF defect",
        "Clear CSF defect",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t rmep;
    uint8 index = 0;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF;
    rmep.update_value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_update_rmep_csf_en,
        ctc_cli_oam_cfm_update_rmep_csf_en_cmd,
        "remote-mep csf (enable| disable) (level LEVEL|) (rmep-id MEP_ID)",
        "Remote MEP",
        "Client signal fail",
        "Enable client signal fail",
        "Disable client signal fail",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    uint8 enable    = 0;
    ctc_oam_update_t rmep;
    uint8 index     = 0;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    rmep.is_local = 0;

    if (0 == sal_memcmp(argv[0], "e", 1))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    rmep.update_type    = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN;
    rmep.update_value   = enable;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_cfm_update_rmep,
        ctc_cli_oam_cfm_update_rmep_cmd,
        "remote-mep (hw-aps APS_GRP (is-protect|) | hw-aps-en (enable |disable)| rmep-sa-mac MAC_ADDR) (level LEVEL|) (rmep-id MEP_ID)",
        "Remote MEP",
        "Hw APS",
        "Aps Group ID",
        "Protecting path",
        "Hw APS enable",
        "Enable",
        "Disable",
        "Remote MEP MAC SA",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        CTC_CLI_OAM_MEP_ID_DESC,
        CTC_CLI_OAM_MEP_ID_VALUE_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_oam_update_t rmep;
    ctc_oam_hw_aps_t  oam_aps;
    uint32 index = 0;
    mac_addr_t rmep_mac_sa;

    /* default configure */
    sal_memset(&rmep, 0, sizeof(rmep));
    sal_memcpy(&rmep.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));
    sal_memset(&oam_aps, 0, sizeof(ctc_oam_hw_aps_t));
    sal_memset(&rmep_mac_sa, 0, sizeof(mac_addr_t));
    rmep.is_local = 0;

    index = CTC_CLI_GET_ARGC_INDEX("hw-aps");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS;
        CTC_CLI_GET_UINT16_RANGE("aps group id", oam_aps.aps_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX("is-protect");
        if (0xFF != index)
        {
            oam_aps.protection_path = 1;
        }
        rmep.p_update_value = &oam_aps;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-aps-en");
    if (0xFF != index)
    {
        rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN;
        rmep.update_value = (0 == sal_strcmp(argv[index + 1], "enable")) ? 1 : 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-sa-mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("rmep-sa-mac", rmep_mac_sa, argv[index + 1]);
        rmep.update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_MACSA;
        rmep.p_update_value = &rmep_mac_sa;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", rmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("rmep id", rmep.rmep_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_update_rmep(g_api_lchip, &rmep);
    }
    else
    {
        ret = ctc_oam_update_rmep(&rmep);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



#define M_TRPT ""
#define CTC_OAM_TRPT_MAX_PKTHDR_LEN 96


STATIC int32
_oam_cli_pkthdr_get_from_file(char* file, uint8* buf, uint16* pkt_len)
{
    sal_file_t fp = NULL;
    int8  line[128];
    int32 lidx = 0, cidx = 0, tmp_val = 0;

    /* read packet from file */
    fp = sal_fopen(file, "r");
    if (NULL == fp)
    {
        ctc_cli_out("%% Failed to open the file <%s>\n", file);
        return CLI_ERROR;
    }

    sal_memset(line, 0, 128);
    while (sal_fgets((char*)line, 128, fp))
    {
        for (cidx = 0; cidx < 16; cidx++)
        {
            if (1 == sal_sscanf((char*)line + cidx * 2, "%02x", &tmp_val))
            {
                if((lidx * 16 + cidx) >= CTC_OAM_TRPT_MAX_PKTHDR_LEN)
                {
                    ctc_cli_out("%% header_len larger than %d\n", CTC_OAM_TRPT_MAX_PKTHDR_LEN);
                    sal_fclose(fp);
                    fp = NULL;
                    return CLI_ERROR;
                }
                buf[lidx * 16 + cidx] = tmp_val;
                (*pkt_len) += 1;
            }
            else
            {
                break;
            }
        }

        lidx++;
    }

    sal_fclose(fp);
    fp = NULL;

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_oam_trpt_set_session,
        ctc_cli_oam_trpt_set_session_cmd,
        "oam trpt set session SESSION {gport GPORT|nhid NHID} direction (ingress| egress) (vlan VLAN_ID|) (tx-mode MODE) (pkt-num NUMBER | tx-period PERIOD | )  pattern-type \
        {(repeat VALUE | random | increment-byte | decrement-byte | increment-word | decrement-word )}  \
        rate RATE pkt-size SIZE (seqnum-en OFFSET |)  (lbm | tst | (slm (first-pkt-clear| ))| pkt-file FILE_NAME)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_TRPT_M_STR,
        "set",
        "ThroughPut session",
        "Session id",
        "Dest port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Direction",
        "Ingress port",
        "Egress port",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Transmit mode",
        "Mode:0-tx by number, 1-continuous, 2-tx by period",
        "packet number",
        CTC_CLI_OAM_TRPT_PKT_NUM_VAL,
        "tx period",
        "unit:s",
        "pattern type",
        "Repeat pattern type",
        "<0-0xFFFFFFFF>",
        "Random pattern type",
        "Increment byte pattern type",
        "Decrement byte pattern type",
        "Increment word pattern type",
        "Decrement word pattern type",
        "Transmit rate",
        "<128-speed of dest port>(unit:Kbps)",
        "Packet size",
        "<64-16128>",
        "Enable tx seqnum insert",
        "Seqnum offset, <14, user-defined data length>",
        "Loopback message",
        "Testing message",
        "SLM message",
        "clear slm stats when first pkt",
        "File store packet header",
        "File name")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_oam_trpt_t trpt_para;
    static char file[256] = {0};
    uint8 pkt_buf[CTC_OAM_TRPT_MAX_PKTHDR_LEN] = {0};
    uint16 header_len = 0;
    uint8 temp = 0;
    static uint8 lbm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    static uint8 tst_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x25, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static uint8 slm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x37, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    sal_memset(&trpt_para, 0, sizeof(ctc_oam_trpt_t));

    CTC_CLI_GET_UINT8_RANGE("session id", trpt_para.session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("dest-gport", trpt_para.gport, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nhid", trpt_para.nhid, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vlan id", trpt_para.vlan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx-mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("tx mode", trpt_para.tx_mode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    trpt_para.direction = CTC_EGRESS;
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        trpt_para.direction = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pkt-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("packet number", trpt_para.packet_num, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-period");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("tx period", trpt_para.tx_period, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("repeat");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_REPEAT_TYPE;
        CTC_CLI_GET_UINT32_RANGE("repeat value", trpt_para.repeat_pattern, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        temp = index+1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_RANDOM_TYPE;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("increment-byte");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_INC_BYTE_TYPE;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decrement-byte");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_DEC_BYTE_TYPE;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("increment-word");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_INC_WORD_TYPE;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decrement-word");
    if (0xFF != index)
    {
        trpt_para.pattern_type = CTC_OAM_PATTERN_DEC_WORD_TYPE;
        temp = index;
    }

    CTC_CLI_GET_UINT32_RANGE("rate", trpt_para.rate, argv[temp+1], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("packet size", trpt_para.size, argv[temp+2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("seqnum-en");
    if (0xFF != index)
    {
        trpt_para.tx_seq_en = 1;
        CTC_CLI_GET_UINT8_RANGE("seqnum offset", trpt_para.seq_num_offset, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        temp = index+1;
    }

    sal_memset(pkt_buf, 0, CTC_OAM_TRPT_MAX_PKTHDR_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("lbm");
    if (0xFF != index)
    {
        trpt_para.pkt_header = (void*)lbm_pkthd;
        trpt_para.header_len = 29;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tst");
    if (0xFF != index)
    {
        trpt_para.pkt_header = (void*)tst_pkthd;
        trpt_para.header_len = 30;
    }

    index = CTC_CLI_GET_ARGC_INDEX("slm");
    if (0xFF != index)
    {
        trpt_para.is_slm = 1;
        trpt_para.pkt_header = (void*)slm_pkthd;
        trpt_para.header_len = 39;
        index = CTC_CLI_GET_ARGC_INDEX("first-pkt-clear");
        if (0xFF != index)
        {
            trpt_para.first_pkt_clear_en = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("pkt-file");
    if (0xFF != index)
    {
       /* get packet from file */
        sal_strcpy((char*)file, argv[index + 1]);
        /* get packet from file */
        ret = _oam_cli_pkthdr_get_from_file(file, pkt_buf, &header_len);

        if (CLI_ERROR == ret)
        {
            return CLI_ERROR;
        }

        trpt_para.pkt_header = (void*)pkt_buf;
        trpt_para.header_len = header_len;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_trpt_cfg(g_api_lchip, &trpt_para);
    }
    else
    {
        ret = ctc_oam_set_trpt_cfg(&trpt_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_oam_trpt_set_session_en,
        ctc_cli_oam_trpt_set_session_en_cmd,
        "oam trpt set session SESSION gchip GCHIP (enable| disable)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_TRPT_M_STR,
        "Set",
        "Throughput sessopn",
        "Session id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint8 enable = 0;
    uint8 session_id = 0;
    uint8 gchip = 0;

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("gchip", gchip , argv[1], 0, CTC_MAX_UINT8_VALUE);


    if (0 == sal_memcmp(argv[2], "e", 1))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_trpt_en(g_api_lchip, gchip, session_id, enable);
    }
    else
    {
        ret = ctc_oam_set_trpt_en(gchip, session_id, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_oam_trpt_get_stats,
        ctc_cli_oam_trpt_get_stats_cmd,
        "show oam trpt stats session SESSION gchip GCHIP (is-slm|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_OAM_M_STR,
        CTC_CLI_TRPT_M_STR,
        "stats",
        "Throughput sessopn",
        "Session id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Get SLM stats")
{
    int32 ret = CLI_SUCCESS;
    uint8 session_id = 0;
    uint8 gchip = 0;
    uint8 index = 0;
    ctc_oam_trpt_stats_t trpt_stats;

    sal_memset(&trpt_stats, 0, sizeof(ctc_oam_trpt_stats_t));

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("gchip", gchip , argv[1], 0, CTC_MAX_UINT8_VALUE);



    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_get_trpt_stats(g_api_lchip, gchip, session_id, &trpt_stats);
    }
    else
    {
        ret = ctc_oam_get_trpt_stats(gchip, session_id, &trpt_stats);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-slm");
    if (0xFF != index)
    {
        ctc_cli_out("======Throughput session %d stats for SLM======\n", session_id);
        ctc_cli_out("Tx Fcf: 0x%"PRIx64"\n", trpt_stats.tx_fcf);
        ctc_cli_out("Tx Fcb: 0x%"PRIx64"\n", trpt_stats.tx_fcb);
        ctc_cli_out("Rx Fcl: 0x%"PRIx64"\n", trpt_stats.rx_fcl);
    }
    else
    {
        ctc_cli_out("======Throughput session %d stats======\n", session_id);
        ctc_cli_out("Tx packets: %"PRIu64", Tx bytes: %"PRIu64"\n", trpt_stats.tx_pkt, trpt_stats.tx_oct);
        ctc_cli_out("Rx packets: %"PRIu64", Rx bytes: %"PRIu64"\n", trpt_stats.rx_pkt, trpt_stats.rx_oct);
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_oam_trpt_clear_stats,
        ctc_cli_oam_trpt_clear_stats_cmd,
        "clear oam trpt stats session SESSION gchip GCHIP",
        "clear",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_TRPT_M_STR,
        "stats",
        "Throughput sessopn",
        "Session id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 session_id = 0;
    uint8 gchip = 0;

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("gchip", gchip , argv[1], 0, CTC_MAX_UINT8_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_clear_trpt_stats(g_api_lchip, gchip, session_id);
    }
    else
    {
        ret = ctc_oam_clear_trpt_stats(gchip, session_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#define OAM_STATS ""

CTC_CLI(ctc_cli_oam_get_mep_stats,
        ctc_cli_oam_get_mep_stats_cmd,
        "local-mep stats (level LEVEL| )",
        "Local MEP",
        "local MEP on OAM stats information",
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC)
{
    int32 ret       = CLI_SUCCESS;
    ctc_oam_stats_info_t stat_info;
    uint8 count = 0;
    uint8  index = 0;

    /* default configure */
    sal_memset(&stat_info, 0, sizeof(stat_info));
    sal_memcpy(&stat_info.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (index != 0xff)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", stat_info.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_get_stats(g_api_lchip, &stat_info);
    }
    else
    {
        ret = ctc_oam_get_stats(&stat_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if ((stat_info.lm_type == CTC_OAM_LM_TYPE_NONE)
        || (stat_info.lm_type == CTC_OAM_LM_TYPE_SINGLE))
    {
        ctc_cli_out("%% MEP LM type is not dual , no need lm counter. \n");
    }
    else
    {
        if (CTC_OAM_LM_COS_TYPE_PER_COS != stat_info.lm_cos_type)
        {
            ctc_cli_out(" MEP LM counter Rx Fcb is 0x%08x. \n", stat_info.lm_info[0].rx_fcb);
            ctc_cli_out(" MEP LM counter Rx Fcl is 0x%08x. \n", stat_info.lm_info[0].rx_fcl);
            ctc_cli_out(" MEP LM counter Tx Fcb is 0x%08x. \n", stat_info.lm_info[0].tx_fcb);
            ctc_cli_out(" MEP LM counter Tx Fcf is 0x%08x. \n", stat_info.lm_info[0].tx_fcf);
        }
        else
        {
            for (count = 0; count < 8; count++)
            {
                ctc_cli_out(" MEP LM cos %d counter Rx Fcb is 0x%08x. \n", count, stat_info.lm_info[count].rx_fcb);
                ctc_cli_out(" MEP LM cos %d counter Rx Fcl is 0x%08x. \n", count, stat_info.lm_info[count].rx_fcl);
                ctc_cli_out(" MEP LM cos %d counter Tx Fcb is 0x%08x. \n", count, stat_info.lm_info[count].tx_fcb);
                ctc_cli_out(" MEP LM cos %d counter Tx Fcf is 0x%08x. \n", count, stat_info.lm_info[count].tx_fcf);
            }
        }
    }

    return ret;
}

#define M_COMMON_PROPERTY ""

CTC_CLI(ctc_cli_oam_common_set_exception,
        ctc_cli_oam_common_set_exception_cmd,
        "oam common exception-to-cpu ({ invalid-oam-pdu | rdi-defect | mac-status | high-ver-oam | \
    no-rmep | xcon-ccm | seq-num-error | reg-cfg-error | ccm-with-option-tlv | slow-oam | src-mac-mismatch | \
    aps-pdu-to-cpu | interval-mismatch | dm-to-cpu | lbr-to-cpu | lm-to-cpu | test-to-cpu | invalid-dm-dlm-to-cpu | \
    big-interval-to-cpu | tp-dlm-to-cpu | tp-dlmdm-to-cpu | csf-to-cpu | mip-no-process-pdu | mcc-to-cpu | lt-to-cpu | \
    lbm-mac-mep-id-fail | learning-bfd-to-cpu | bfd-timer-negotiation | scc-to-cpu | defect-to-cpu | lb-to-cpu | \
    tp-lbm-to-cpu | tp-csf-to-cpu | sm-to-cpu | tp-bfd-cv-to-cpu | unknown-pdu-to-cpu | \
    bfd-mismatch-to-cpu | all } | )",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_COMMON_M_STR,
        "OAM exception to CPU",
        "Invalid OAM PDU",
        "Some RDI defect",
        "Some mac status defect",
        "High version ETH OAM to CPU",
        "ErrorCcmDefect RMEP not found",
        "XconCCMdefect",
        "CCM sequence number error",
        "Table or Control Register Configure error",
        "CCM has optional TLV",
        "Slow OAM to cpu",
        "Source mac mismatch",
        "APS PDU to CPU",
        "ErrorCcmDefect CCM interval mismatch to CPU",
        "Y.1731 & TP Y.1731 DM PDU to CPU",
        "Equal LBR to CPU",
        "LM PDU to CPU",
        "TEST PDU to CPU",
        "Invalid MPLS TP DM DLM PDU",
        "Big interval CCM",
        "MPLS TP DLM PDU to CPU",
        "MPLS TP DLM-DM PDU to CPU",
        "CSF PDU to CPU",
        "MIP not support PDU",
        "MCC PDU to CPU",
        "LT PDU to CPU",
        "LBM mac or MEP ID check fail",
        "BFD learning to CPU",
        "BFD timer negotiation",
        "SCC PDU to CPU",
        "All defect to CPU",
        "Ether LBM/LBR and MPLS-TP LBR to CPU",
        "MPLS-TP LBM to CPU",
        "MPLS-TP CSF to CPU",
        "Ether SLM/SLR to CPU",
        "TP BFD CV PDU to CPU",
        "Unknown PDU to CPU",
        "BFD discreaminator mismatch to CPU",
        "All exception to CPU")
{
    int32  ret   = CLI_SUCCESS;
    uint32 value = 0;
    uint8  index = 0;
    ctc_oam_property_t  oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xff)
    {
        value = 0xFFFFFFFF;
    }

    /**/
    index = CTC_CLI_GET_ARGC_INDEX("invalid-oam-pdu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_INVALID_OAM_PDU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rdi-defect");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SOME_RDI_DEFECT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-status");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SOME_MAC_STATUS_DEFECT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("high-ver-oam");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_HIGH_VER_OAM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("no-rmep");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_RMEP_NOT_FOUND);
    }

    index = CTC_CLI_GET_ARGC_INDEX("xcon-ccm");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_XCON_CCM_DEFECT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-num-error");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_CCM_SEQ_NUM_ERROR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("reg-cfg-error");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_CTRL_REG_CFG_ERROR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ccm-with-option-tlv");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_OPTION_CCM_TLV);
    }

    index = CTC_CLI_GET_ARGC_INDEX("slow-oam");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SLOW_OAM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-mac-mismatch");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SMAC_MISMATCH);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-pdu-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_APS_PDU_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval-mismatch");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_CCM_INTERVAL_MISMATCH);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_DM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lbr-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_EQUAL_LBR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_LM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("test-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_ETH_TST_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("invalid-dm-dlm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_INVALID_BFD_MPLS_DM_DLM_PDU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("big-interval-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-dlm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_MPLS_TP_DLM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-dlmdm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_MPLS_TP_DM_DLMDM_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("csf-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_CSF_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mip-no-process-pdu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_MIP_RECEIVE_NON_PROCESS_PDU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcc-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_MCC_PDU_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lt-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_EQUAL_LTM_LTR_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lbm-mac-mep-id-fail");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_LBM_MAC_DA_MEP_ID_CHECK_FAIL);
    }

    index = CTC_CLI_GET_ARGC_INDEX("learning-bfd-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_LEARNING_BFD_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-timer-negotiation");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION);
    }

    index = CTC_CLI_GET_ARGC_INDEX("scc-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SCC_PDU_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("defect-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_ALL_DEFECT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lb-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_LBM);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-lbm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_TP_LBM);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-csf-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_TP_CSF);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sm-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_SM);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd-cv-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_TP_BFD_CV);
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-pdu-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_UNKNOWN_PDU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-mismatch-to-cpu");
    if (index != 0xff)
    {
        value |= (1 << CTC_OAM_EXCP_BFD_DISC_MISMATCH);
    }

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_EXCEPTION_TO_CPU;
    oam_property.u.common.value     = value;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_property);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_common_set_defect_to_rdi,
        ctc_cli_oam_common_set_defect_to_rdi_cmd,
        "oam common defect-to-rdi ({ mac-status-change | src-mac-mismatch | maid-mismatch | \
    low-ccm | no-rmep | interval-mismatch | dloc | dcsf | all } | )",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_COMMON_M_STR,
        "OAM defect to RDI",
        "Mac status changed defect",
        "Src Mac mismatch defect",
        "Maid mismatch defect",
        "Low CCM defect",
        "ErrorCcmDefect RMEP not found",
        "ErrorCcmDefect CCM interval mismatch",
        "DLoc defect",
        "dCSF defect",
        "All defect to RDI")
{
    int32  ret   = CLI_SUCCESS;
    uint32 value = 0;
    uint8  index = 0;
    ctc_oam_property_t  oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xff)
    {
        value = 0xFFFFFFFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-status-change");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MAC_STATUS_CHANGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-mac-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_SRC_MAC_MISMATCH;
    }

    index = CTC_CLI_GET_ARGC_INDEX("maid-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MISMERGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("low-ccm");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_LEVEL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("no-rmep");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_MEP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_PERIOD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dloc");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_DLOC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dcsf");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_CSF;
    }

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_DEFECT_TO_RDI;
    oam_property.u.common.value     = value;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_property);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_common_set_defect_to_cpu,
        ctc_cli_oam_common_set_defect_to_cpu_cmd,
        "oam common defect-to-cpu ({ rx-first-pdu | mac-status-change | src-mac-mismatch | rx-rdi  | maid-mismatch | \
    low-ccm | no-rmep  | interval-mismatch  | \
    dloc | dcsf | bfd-init | bfd-up | bfd-down | bfd-mis-connect | bfd-neg-success | all } | )",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_COMMON_M_STR,
        "OAM defect to CPU",
        "Rx First PDU",
        "Mac status changed defect",
        "Src Mac mismatch defect",
        "Rx RDI defect",
        "Maid mismatch defect",
        "Low CCM defect",
        "ErrorCcmDefect RMEP not found",
        "ErrorCcmDefect CCM interval mismatch",
        "DLoc defect",
        "dCSF defect",
        "BFD init defect",
        "BFD up defect",
        "BFD down defect",
        "BFD MisConnect",
        "BFD timer negotiation success",
        "All defect to CPU")
{
    int32  ret   = CLI_SUCCESS;
    uint32 value = 0;
    uint8  index = 0;
    ctc_oam_property_t  oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xff)
    {
        value = 0xFFFFFFFF;
    }

    /**/
    index = CTC_CLI_GET_ARGC_INDEX("rx-first-pdu");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-status-change");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MAC_STATUS_CHANGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-mac-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_SRC_MAC_MISMATCH;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx-rdi");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_RDI_RX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("maid-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MISMERGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("low-ccm");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_LEVEL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("no-rmep");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_MEP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_PERIOD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dloc");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_DLOC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dcsf");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_CSF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-init");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_INIT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-up");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_UP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-down");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_DOWN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-mis-connect");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("bfd-neg-success");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_TIMER_NEG_SUCCESS;
    }

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_DEFECT_TO_CPU;
    oam_property.u.common.value     = value;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_property);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_common_set_defect_to_sf,
        ctc_cli_oam_common_set_defect_to_sf_cmd,
        "oam common defect-to-sf ({ mac-status-change | rx-rdi  | maid-mismatch | low-ccm | no-rmep  | interval-mismatch  | \
    dloc | bfd-down | bfd-mis-connect | all } | )",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_COMMON_M_STR,
        "Mapping OAM defect to SF",
        "Mac status changed defect",
        "Rx RDI defect",
        "Maid mismatch defect",
        "Low CCM defect",
        "RMEP not found",
        "CCM interval mismatch",
        "DLoc defect",
        "BFD down defect",
        "BFD MisConnect",
        "All defect to SF")
{
    int32  ret   = CLI_SUCCESS;
    uint32 value = 0;
    uint8  index = 0;
    ctc_oam_property_t  oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xff)
    {
        value = 0xFFFFFFFF;
    }

    /**/
    index = CTC_CLI_GET_ARGC_INDEX("mac-status-change");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MAC_STATUS_CHANGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx-rdi");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_RDI_RX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("maid-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_MISMERGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("low-ccm");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_LEVEL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("no-rmep");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_MEP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval-mismatch");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_UNEXPECTED_PERIOD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dloc");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_DLOC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-down");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_DOWN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bfd-mis-connect");
    if (index != 0xff)
    {
        value |= CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT;
    }

    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_DEFECT_TO_SF;
    oam_property.u.common.value     = value;

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_property);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_oam_common_set_property,
        ctc_cli_oam_common_set_property_cmd,
        "oam common (timer-update (enable|disable) | master-gchip GCHIP_ID | hw-aps-mode MODE)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_COMMON_M_STR,
        "OAM timer update",
        "Enable",
        "Disable",
        "Master chip id",
        CTC_CLI_GCHIP_DESC,
        "HW APS mode",
        "Mode")
{
    int32  ret   = CLI_SUCCESS;
    uint8  index = 0;
    ctc_oam_property_t  oam_property;

    sal_memset(&oam_property, 0, sizeof(ctc_oam_property_t));
    oam_property.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    index = CTC_CLI_GET_ARGC_INDEX("timer-update");
    if (index != 0xff)
    {
        oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_TIMER_DISABLE;
        index = CTC_CLI_GET_ARGC_INDEX("disable");
        if (index != 0xff)
        {
            oam_property.u.common.value     = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("master-gchip");
    if (index != 0xff)
    {
        oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_MASTER_GCHIP;
        CTC_CLI_GET_UINT8_RANGE("Master gchip id", oam_property.u.common.value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-aps-mode");
    if (index != 0xff)
    {
        oam_property.u.common.cfg_type  = CTC_OAM_COM_PRO_TYPE_HW_APS_MODE;
        CTC_CLI_GET_UINT8_RANGE("HW aps mode", oam_property.u.common.value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_property);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_oam_cfm_update_lmep_trpt_en,
        ctc_cli_oam_cfm_update_lmep_trpt_en_cmd,
        "local-mep trpt (level LEVEL | ) (session SESSION) (enable | disable)",
        "Local MEP",
        CTC_CLI_TRPT_M_STR,
        CTC_CLI_OAM_MEP_LEVEL_DESC,
        CTC_CLI_OAM_MEP_LEVEL_VALUE_DESC,
        "TroughPut session",
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

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF == index)
    {
        session_id = 0xff;
    }

    index = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("md level", lmep.key.u.eth.md_level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lmep.update_type    = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT;
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



CTC_CLI(ctc_cli_oam_cfm_set_if_status,
        ctc_cli_oam_cfm_set_if_status_cmd,
        "oam cfm port GPORT_ID (if-status STATUS)",
        CTC_CLI_OAM_M_STR,
        CTC_CLI_OAM_CFM_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Interface status",
        "Status ")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint32 status = 0;
    ctc_oam_property_t oam_prop;

    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("if-status");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("if-status", status, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }


    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_IF_STATUS;
    oam_prop.u.y1731.gport  = gport;
    oam_prop.u.y1731.value  = status;

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


CTC_CLI(ctc_cli_oam_bfd_set_property,
        ctc_cli_oam_bfd_set_property_cmd,
        "oam bfd timer-neg-inteval INTERVAL (rx-end|tx-end|)",
        CTC_CLI_OAM_M_STR,
        "BFD",
        "Timer Negotiation",
        "Interval, unit ms",
        "Config to RX",
        "Config to TX")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    ctc_oam_property_t oam_prop;
    static ctc_oam_bfd_timer_neg_t interval = {{0}, 0, 0};
    uint8 is_end = 0;
    sal_memset(&oam_prop, 0, sizeof(ctc_oam_property_t));

    if (interval.interval_num >= CTC_OAM_BFD_TIMER_NEG_NUM)
    {
        ctc_cli_out("%% %s \n", "Interval exceed max number!");
        sal_memset(&interval, 0, sizeof(ctc_oam_bfd_timer_neg_t));
        return CLI_ERROR;
    }

    oam_prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_BFD;
    oam_prop.u.bfd.cfg_type = CTC_OAM_BFD_CFG_TYPE_TIMER_NEG_INTERVAL;
    oam_prop.u.bfd.p_value = &interval;
    CTC_CLI_GET_UINT32_RANGE("interval", interval.interval[interval.interval_num++], argv[0], 0, CTC_MAX_UINT32_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("rx-end");
    if (index != 0xFF)
    {
        is_end = 1;
        interval.is_rx = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-end");
    if (index != 0xFF)
    {
        is_end = 1;
        interval.is_rx = 0;
    }

    if (0 == is_end)
    {
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_set_property(g_api_lchip, &oam_prop);
    }
    else
    {
        ret = ctc_oam_set_property(&oam_prop);
    }

    sal_memset(&interval, 0, sizeof(ctc_oam_bfd_timer_neg_t));

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


STATIC int32
_oam_cli_show_mep_info(uint8 mep_type, ctc_oam_lmep_info_t* lmep, ctc_oam_rmep_info_t* rmep)
{
    char* mep_type_name[CTC_OAM_MEP_TYPE_MAX] = {"ETH_1AG", "ETH_Y1731", "MPLS_TP_Y1731", "IP_BFD", "MPLS_BFD", "MPLS_TP_BFD", "MICRO_BFD"};
    char  macsa_str[64] = {0};
    uint8 len = 0;
    uint8 loop = 0;
    uint32 ret = 0;
    char  mep_id_str[64] = {0};

    if (rmep)
    {
        ctc_cli_out("------------------------------remote mep info--------------------------\n");
        ctc_cli_out("%-15s:  %-15s\n", "mep type", mep_type_name[mep_type]);
        if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
            || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
        {
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "RmepId", rmep->y1731_rmep.rmep_id,
                        "lmepIndex", rmep->y1731_rmep.lmep_index);
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "1stPkt", (rmep->y1731_rmep.first_pkt_rx ? "Y" : "N"),
                        "active", (rmep->y1731_rmep.active ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "dLoc", (rmep->y1731_rmep.d_loc ? "Y" : "N"),
                        "dUnExpPeriod", (rmep->y1731_rmep.d_unexp_period ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "SrcMacMis", (rmep->y1731_rmep.ma_sa_mismatch ? "Y" : "N"),
                        "MacStaChange", (rmep->y1731_rmep.mac_status_change ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "RxRDI", (rmep->y1731_rmep.last_rdi ? "Y" : "N"),
                        "P2P", (rmep->y1731_rmep.is_p2p_mode ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "CsfEn", (rmep->y1731_rmep.csf_en ? "Y" : "N"),
                        "Dcsf", (rmep->y1731_rmep.d_csf ? "Y" : "N"));

            sal_sprintf(macsa_str, "%.2x%.2x.%.2x%.2x.%.2x%.2x", rmep->y1731_rmep.mac_sa[0], rmep->y1731_rmep.mac_sa[1], rmep->y1731_rmep.mac_sa[2],
                rmep->y1731_rmep.mac_sa[3], rmep->y1731_rmep.mac_sa[4], rmep->y1731_rmep.mac_sa[5]);

            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15s\n", "Rx_csf_type", rmep->y1731_rmep.rx_csf_type,
                        "Mac_sa", macsa_str);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15s\n", "Dloc_time", rmep->y1731_rmep.dloc_time,
                        "Path_fail", (rmep->y1731_rmep.path_fail ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "Path_active", (rmep->y1731_rmep.path_active ? "Y" : "N"),
                        "Protection_path", (rmep->y1731_rmep.protection_path? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15u\n", "Hw_aps_enable", (rmep->y1731_rmep.hw_aps_en? "Y" : "N"),
                        "Hw_aps_group_id", rmep->y1731_rmep.hw_aps_group_id);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "LastPortStatus", rmep->y1731_rmep.last_port_status,
                        "LastIfStatus", rmep->y1731_rmep.last_intf_status);
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15u\n", "LastSeqChkEn", (rmep->y1731_rmep.last_seq_chk_en ? "Y" : "N"),
                        "SeqFailNum", rmep->y1731_rmep.seq_fail_count);
            if (rmep->y1731_rmep.ccm_rx_pkts)
            {
                ctc_cli_out("%-15s:  %-15u\n", "CcmRxPkts", rmep->y1731_rmep.ccm_rx_pkts);
            }
            if (rmep->y1731_rmep.err_ccm_rx_pkts)
            {
                ctc_cli_out("%-15s:  %-15u\n", "ErrCcmRxPkts", rmep->y1731_rmep.err_ccm_rx_pkts);
            }
        }
        else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_BFD ==mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type))
        {
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "lmepIndex", rmep->bfd_rmep.lmep_index,
                        "State", rmep->bfd_rmep.remote_state);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "Diag", rmep->bfd_rmep.remote_diag,
                        "DetectMult", rmep->bfd_rmep.remote_detect_mult);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "ActualRxIntv", rmep->bfd_rmep.actual_rx_interval,
                        "RequiredRxIntv", rmep->bfd_rmep.required_min_rx_interval);

            if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
            {
                ctc_cli_out("%-15s:  %-15s", "MisConnect", (rmep->bfd_rmep.mis_connect ? "Y" :"N"));
            }
            else
            {
                ctc_cli_out("%-15s:  %-15u", "RemoteDisc", rmep->bfd_rmep.remote_discr);
            }
            ctc_cli_out("  %-15s:  %-15u\n", "Dloc_time", rmep->bfd_rmep.dloc_time);
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "1stPkt", (rmep->bfd_rmep.first_pkt_rx ? "Y" : "N"),
                        "MepEn", (rmep->bfd_rmep.mep_en ? "Y" :"N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "Path_active", (rmep->bfd_rmep.path_active? "Y" : "N"),
                        "Protection_path", (rmep->bfd_rmep.protection_path ? "Y" : "N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15u\n", "Hw_aps_enable", (rmep->bfd_rmep.hw_aps_en? "Y" : "N"),
                        "Hw_aps_group_id", rmep->bfd_rmep.hw_aps_group_id);
            ctc_cli_out("%-15s:  %-15s\n", "Path_fail", (rmep->bfd_rmep.path_fail ? "Y" : "N"));
        }


    }

    if(lmep)
    {
        ctc_cli_out("------------------------------local mep info--------------------------\n");
        ctc_cli_out("%-15s:  %-15s\n", "mep type", mep_type_name[mep_type]);
         if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
            || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
        {
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "MepId", lmep->y1731_lmep.mep_id,
                        "CcmIntelval", lmep->y1731_lmep.ccm_interval);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "Vlan", lmep->y1731_lmep.vlan_id,
                        "Level", lmep->y1731_lmep.level);
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "Active", lmep->y1731_lmep.active ? "Y" :"N",
                        "CcEn", lmep->y1731_lmep.ccm_enable ? "Y" :"N");
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "DmEn", lmep->y1731_lmep.dm_enable ? "Y" :"N",
                        "UpMep", lmep->y1731_lmep.is_up_mep ? "Y" :"N");
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "dUnExpMep", lmep->y1731_lmep.d_unexp_mep ? "Y" :"N",
                        "dMisMerge", lmep->y1731_lmep.d_mismerge ? "Y" :"N");
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "dMegLvl", lmep->y1731_lmep.d_meg_lvl ? "Y" :"N",
                        "TxRDI", lmep->y1731_lmep.present_rdi ? "Y" :"N");
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "SeqNumEn", lmep->y1731_lmep.seq_num_en? "Y" :"N",
                        "PortStatusEn", lmep->y1731_lmep.port_status_en? "Y" :"N");
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "IfStatusEn", lmep->y1731_lmep.if_status_en? "Y" :"N",
                        "SenderIdEn", lmep->y1731_lmep.sender_id_en? "Y" :"N");
            if (lmep->y1731_lmep.ccm_tx_pkts)
            {
                ctc_cli_out("%-15s:  %-15u\n", "CcmTxPkts", lmep->y1731_lmep.ccm_tx_pkts);
            }
        }
        else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
            || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type))
        {
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "Discr", lmep->bfd_lmep.local_discr,
                        "State", lmep->bfd_lmep.loacl_state);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "Diag", lmep->bfd_lmep.local_diag,
                        "DetectMult", lmep->bfd_lmep.local_detect_mult);
            ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "ActualTxIntv", lmep->bfd_lmep.actual_tx_interval,
                        "DesiredTxIntv", lmep->bfd_lmep.desired_min_tx_interval);
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "CcEn", (lmep->bfd_lmep.cc_enable ? "Y" :"N"),
                        "MepEn", (lmep->bfd_lmep.mep_en ? "Y" :"N"));
            ctc_cli_out("%-15s:  %-15s  %-15s:  %-15u\n", "SingleHop", (lmep->bfd_lmep.single_hop ? "Y" :"N"),
                        "Rx_csf_type", lmep->bfd_lmep.rx_csf_type);
            if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
            {
                ctc_cli_out("%-15s:  %-15s  %-15s:  %-15s\n", "CvEn", (lmep->bfd_lmep.cv_enable ? "Y" :"N"),
                        "CsfEn", (lmep->bfd_lmep.csf_en ? "Y" :"N"));
                ctc_cli_out("%-15s:  %-15s\n", "Dcsf", (lmep->bfd_lmep.d_csf ? "Y" :"N"));
            }

            len = (lmep->bfd_lmep.mep_id[2]<<8) + lmep->bfd_lmep.mep_id[3];
            for(loop=0; loop<len; loop++)
            {
                ret += sal_sprintf((mep_id_str + ret), "%u", lmep->bfd_lmep.mep_id[loop+4]);
            }

            if(lmep->bfd_lmep.cv_enable)
            {
                ctc_cli_out("%-15s:  %-15u  %-15s:  %-15u\n", "Mep_id_type", (lmep->bfd_lmep.mep_id[0]<<8) + lmep->bfd_lmep.mep_id[1],
                            "Mep_id_len", len);
                ctc_cli_out("%-15s:  %-15s\n", "Mep_id_value", mep_id_str);
            }

        }

    }
    return CLI_SUCCESS;
}





CTC_CLI(ctc_cli_oam_mep_info,
        ctc_cli_oam_mep_info_cmd,
        "show mep-info (mep-index INDEX)",
        CTC_CLI_SHOW_STR,
        "MEP Information",
        "Mep index",
        "Mep index value")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    ctc_oam_mep_info_t mep_info;

    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("mep-index", mep_info.mep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_oam_get_mep_info(g_api_lchip, &mep_info);
    }
    else
    {
        ret = ctc_oam_get_mep_info(g_api_lchip, &mep_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    if (mep_info.is_rmep)
    {
        ret = _oam_cli_show_mep_info(mep_info.mep_type, NULL, &mep_info.rmep);
    }
    else
    {
        ret = _oam_cli_show_mep_info(mep_info.mep_type, &mep_info.lmep, NULL);
    }
    return ret;
}



CTC_CLI(ctc_cli_oam_bfd_get_mep_info_with_key,
        ctc_cli_oam_bfd_get_mep_info_with_key_cmd,
        "show mep-info bfd my-discr MY_DISCR (sbfd-reflector|)\
        (ip-bfd | micro-bfd | mpls-bfd | tp-bfd { (section-oam (gport GPORT_ID |ifid IFID) | (space SPACEID |) mpls-in-label IN_LABEL)})",
        CTC_CLI_SHOW_STR,
        "Mep information",
        "BFD",
        "My Discriminator",
        "My Discriminator value",
        "Is sbfd reflector",
        "IP BFD OAM option",
        "Micro BFD OAM option",
        "MPLS BFD OAM option",
        "MPLS TP BFD OAM option",
        "Is MPLS TP section oam",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Mpls label space",
        "Label space id",
        "Mpls label",
        "Label")
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0;

    ctc_oam_mep_info_with_key_t  mep_info;

    /* default configure */
    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_with_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("sbfd-reflector");
    if (0xFF != index)
    {
        CTC_SET_FLAG(mep_info.key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        mep_info.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", mep_info.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        mep_info.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        CTC_CLI_GET_UINT32_RANGE("my discr", mep_info.key.u.bfd.discr, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        mep_info.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;

        index = CTC_CLI_GET_ARGC_INDEX("section-oam");

        if (0xFF != index)
        {
            CTC_SET_FLAG(mep_info.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", mep_info.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ifid");
        if (index != 0xff)
        {
            CTC_CLI_GET_UINT16_RANGE("IFID", mep_info.key.u.tp.gport_or_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("space");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("MPLS label space", mep_info.key.u.tp.mpls_spaceid , argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mpls-in-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("mpls label", mep_info.key.u.tp.label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    if (g_ctcs_api_en)
    {

        ret = ctcs_oam_get_mep_info_with_key(g_api_lchip, &mep_info);
    }
    else
    {
        ret = ctc_oam_get_mep_info_with_key(&mep_info);
    }


    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ret = _oam_cli_show_mep_info(mep_info.key.mep_type, &mep_info.lmep, &mep_info.rmep);
    return ret;
}

CTC_CLI(ctc_cli_oam_y1731_get_mep_info_with_key,
        ctc_cli_oam_y1731_get_mep_info_with_key_cmd,
        "show mep-info y1731 {rmep-id RMEP_ID | md-level LEVEL|}",
        CTC_CLI_SHOW_STR,
        "Mep information",
        "Y1731",
        "Rmep id",
        "Rmep id value",
        "Md level",
        "Level")
{
    int32 ret   = CLI_SUCCESS;
    uint8 index = 0;

    ctc_oam_mep_info_with_key_t  mep_info;
    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_with_key_t));
    sal_memcpy(&mep_info.key, &p_oam_lmep->key, sizeof(ctc_oam_key_t));

    index = CTC_CLI_GET_ARGC_INDEX("rmep-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("rmep-id", mep_info.rmep_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("md-level");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("Md level", mep_info.key.u.eth.md_level, argv[index + 1]);
    }


    if (g_ctcs_api_en)
    {

        ret = ctcs_oam_get_mep_info_with_key(g_api_lchip, &mep_info);
    }
    else
    {
        ret = ctc_oam_get_mep_info_with_key(&mep_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if ((CTC_OAM_MEP_TYPE_ETH_1AG != mep_info.key.mep_type)
        && (CTC_OAM_MEP_TYPE_ETH_Y1731 != mep_info.key.mep_type)
    && (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 != mep_info.key.mep_type))
    {
        return CLI_ERROR;
    }

    ret = _oam_cli_show_mep_info(mep_info.key.mep_type, &mep_info.lmep, &mep_info.rmep);

    return ret;

}



/**
 @brief  Initialize sdk oam module CLIs
*/
int32
ctc_oam_cli_init(void)
{
    /* cfm cli under sdk mode */
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_add_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_remove_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_add_remove_maid_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_oam_tp_y1731_add_remove_maid_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_oam_tp_add_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_tp_remove_chan_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_oam_enter_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_port_oam_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_port_enable_port_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_update_port_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_update_intf_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_all_pdu_to_cpu_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_add_edge_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_remove_edge_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_tx_vlan_tpid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_rx_vlan_tpid_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_sender_id_tlv_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_bridge_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_tp_y1731_ach_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_rdi_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_big_interval_to_cpu_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_lbm_proc_in_asic_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_lbr_use_lbm_da_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_lbr_use_share_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_set_vpls_lkup_mode_cmd);



    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_add_remove_mip_cmd);

    /*Y.1731*/
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_port_lm_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_if_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_cfm_set_poam_lm_enable_cmd);

    /* common cli */
    install_element(CTC_SDK_MODE, &ctc_cli_oam_common_set_exception_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_common_set_defect_to_rdi_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_common_set_defect_to_cpu_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_common_set_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_common_set_defect_to_sf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_mep_info_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_set_property_cmd);

    /* cfm cli under oam chan mode */
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_exit_chan_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_add_lmep_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_remove_lmep_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_add_rmep_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_remove_rmep_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_enable_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_ccm_enable_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_tx_cos_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_sf_state_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_seq_num_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_ccm_use_uc_mac_en_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_rmep_enable_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_clear_rmep_seq_num_fail_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_clear_lmep_rdi_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_rmep_seq_num_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_rmep_sf_state_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_clear_rmep_mac_status_fail_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_clear_rmep_src_mac_mismatch_cmd);

    /*Y.1731*/
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_dm_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_lock_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_csf_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_csf_type_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_csf_use_uc_da_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_lm_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_lm_type_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_lm_cos_type_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_lm_cos_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_clear_rmep_dcsf_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_rmep_csf_en_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_mel_cmd);

    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_get_mep_stats_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_rmep_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_y1731_get_mep_info_with_key_cmd);

    /*BFD*/
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_add_lmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_remove_lmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_add_rmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_remove_rmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_update_lmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_update_rmep_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_bfd_get_mep_info_with_key_cmd);

    /*Throughput*/
    install_element(CTC_SDK_MODE, &ctc_cli_oam_trpt_set_session_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_trpt_set_session_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_trpt_get_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_oam_trpt_clear_stats_cmd);
    install_element(CTC_SDK_OAM_CHAN_MODE, &ctc_cli_oam_cfm_update_lmep_trpt_en_cmd);


    return 0;
}


#if 0
/****************************************************************************
* $Id$
*  Process mode switching in vtysh
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2012-02-01 09:03
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "ctc_cli.h"
#include "vtysh.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#ifndef CTC_CLI_OAM_M_STR
#define CTC_CLI_OAM_M_STR "OAM module"
#define CTC_CLI_OAM_CFM_M_STR "CFM"
#define CTC_CLI_OAM_EFM_M_STR "EFM"
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
#define CTC_CLI_OAM_MEP_TX_TPID_TYPE_VALUE_DESC "<0-3>"

#define CTC_CLI_OAM_CFM_DUMP_DESC "Dump registers or tables"
#endif


/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/****************************************************************************
 *
* Function
* 
****************************************************************************/

DEFUNSH (VTYSH_ALL,
        humber_cli_enter_sdk_mode,
        humber_cli_enter_sdk_mode_vtysh_cmd,
        "enter sdk mode",
        "Enter",
        "Ctc SDK mode",
        "Mode")
{
    vty->node  = CTC_SDK_MODE;
    return CLI_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
        humber_cli_enter_debug_mode,
        humber_cli_enter_debug_mode_vtysh_cmd,
        "enter debug mode",
        "Enter",
        "Ctc Debug mode",
        "Mode")
{
    vty->node  = CTC_DEBUG_MODE;
    return CLI_SUCCESS;
}

/**************************  <<< HUMBER SDK CLI V1.0 begin   **************************/
DEFUNSH (VTYSH_ALL,
        humber_cli_enter_sdk_v1_mode,
        humber_cli_enter_sdk_v1_mode_vtysh_cmd,
        "enter sdk v1 mode",
        "Enter",
        "Ctc SDK mode",
        "SDK mode with v1 cli",
        "Mode")
{
    vty->node  = CTC_SDK_V1_MODE;
    return CLI_SUCCESS;
}
/**************************   HUMBER SDK CLI V1.0 end >>>   **************************/

DEFUNSH (VTYSH_ALL,
        humber_cli_enter_cmodel_mode,
        humber_cli_enter_cmodel_mode_vtysh_cmd,
        "enter cmodel mode",
        "Enter",
        "Ctc cmodel mode",
        "Mode")
{
    vty->node  = CTC_CMODEL_MODE;
    return CLI_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
    cli_oam_cfm_add_chan,
    cli_oam_cfm_add_chan_vtysh_cmd,
    "oam cfm lookup-chan add gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID|) (master-gchip GCHIP_ID|)",
    CTC_CLI_OAM_M_STR,
    CTC_CLI_OAM_CFM_M_STR,
    "MEP lookup channel",
    "Add MEP lookup channel", 
    CTC_CLI_GPORT_DESC,
    CTC_CLI_GPORT_ID_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
    CTC_CLI_OAM_LINK_OAM_DESC,
    CTC_CLI_VLAN_DESC,
    CTC_CLI_VLAN_RANGE_DESC,
    "Master chip id for link agg",
    "<0-29>")
{
    vty->node = CTC_SDK_OAM_CHAN_MODE;
    return CMD_SUCCESS;
}

DEFUNSH (VTYSH_ALL,
    cli_oam_cfm_enter_chan,
    cli_oam_cfm_enter_chan_vtysh_cmd,
    "oam cfm enter lookup-chan gport GPORT_ID direction (up |down ) (link-oam |) (vlan VLAN_ID|)",
    CTC_CLI_OAM_M_STR,
    CTC_CLI_OAM_CFM_M_STR,
    "Enter OAM channel mode",
    "MEP lookup channel",
    CTC_CLI_GPORT_DESC,
    CTC_CLI_GPORT_ID_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
    CTC_CLI_OAM_LINK_OAM_DESC,
    CTC_CLI_VLAN_DESC,
    CTC_CLI_VLAN_RANGE_DESC)
{
    vty->node = CTC_SDK_OAM_CHAN_MODE;  
    return CMD_SUCCESS;    
}

DEFUNSH (VTYSH_ALL,
    cli_oam_cfm_enter_other_chan,
    cli_oam_cfm_enter_other_chan_vtysh_cmd,
    "enter lookup-chan gport GPORT_ID direction (up| down ) (link-oam|) (vlan VLAN_ID|)",
    "Enter other OAM channel",
    "MEP lookup channel",
    CTC_CLI_GPORT_DESC,
    CTC_CLI_GPORT_ID_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_UP_DESC,
    CTC_CLI_OAM_CFM_DIRECTION_DOWN_DESC,
    CTC_CLI_OAM_LINK_OAM_DESC,
    CTC_CLI_VLAN_DESC,
    CTC_CLI_VLAN_RANGE_DESC)
{
    vty->node = CTC_SDK_OAM_CHAN_MODE;  
    return CMD_SUCCESS;    
}

void
vtysh_mode_init(void)
{
    install_element (EXEC_MODE, &humber_cli_enter_sdk_mode_vtysh_cmd);
    install_element (EXEC_MODE, &humber_cli_enter_debug_mode_vtysh_cmd);
    install_element (EXEC_MODE, &humber_cli_enter_sdk_v1_mode_vtysh_cmd);
    //install_element (EXEC_MODE, &humber_cli_enter_cmodel_mode_vtysh_cmd);
    install_element (CTC_SDK_MODE, &cli_oam_cfm_add_chan_vtysh_cmd);
    install_element (CTC_SDK_MODE, &cli_oam_cfm_enter_chan_vtysh_cmd);
    install_element (CTC_SDK_MODE, &cli_oam_cfm_enter_other_chan_vtysh_cmd);
    return;
}
#endif

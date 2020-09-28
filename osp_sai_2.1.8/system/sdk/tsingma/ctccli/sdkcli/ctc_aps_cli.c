/**
 @file ctc_aps_cli.c

 @date 2010-4-1

 @version v2.0
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_cli_common.h"
#include "ctc_port_cli.h"

CTC_CLI(ctc_cli_aps_create_aps_bridge_group,
        ctc_cli_aps_create_aps_bridge_group_cmd,
        "aps create aps-bridge group GROUP_ID (aps-type (l2|mpls) |) (working-port GPORT_ID (w-l3if-id INTERFACE_ID|)|\
     next-w-aps  ID|) (protection-port GPORT_ID (p-l3if-id INTERFACE_ID|) (physical-isolated|) | next-p-aps ID | \
     raps-en raps-groupid GROUP_ID|) (hw-based-aps-en|)",
        CTC_CLI_APS_M_STR,
        "Create",
        "Aps bridge",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Aps type",
        "Used for normal l2 aps protection",
        "Used for mpls aps protection",
        "Working path gport",
        CTC_CLI_GPORT_ID_DESC,
        "Working path L3 interface id",
        "INTERFACE ID",
        "Next working aps group id",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Protection path gport",
        CTC_CLI_GPORT_ID_DESC,
        "Protection path L3 interface id",
        "INTERFACE ID",
        "Port aps use physical isolated path",
        "Next protection aps group id",
        "0-1023",
        "Raps enable",
        "Raps groupid",
        "Raps Group ID,it is allocated in platform adaption Layer",
        "Hw-based-aps-en,hardware based aps for link down event")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 group_id = 0;
    uint16 raps_groupid = 0;
    ctc_aps_bridge_group_t aps_bridge;

    sal_memset(&aps_bridge, 0, sizeof(ctc_aps_bridge_group_t));
    aps_bridge.protect_en = FALSE;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("aps-type");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("mpls");
        if (0xFF != index)
        {
            aps_bridge.aps_flag |= CTC_APS_FLAG_IS_MPLS;
        }
        else
        {
            aps_bridge.aps_flag |= CTC_APS_FLAG_IS_L2;
        }
    }
    else
    {
        aps_bridge.aps_flag = CTC_APS_FLAG_IS_L2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("working-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", aps_bridge.working_gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("next-w-aps");
        if (0xFF != index)
        {
            aps_bridge.next_w_aps_en = 1;
            CTC_CLI_GET_UINT16_RANGE("next working aps group", aps_bridge.next_aps.w_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }        
    }

    index = CTC_CLI_GET_ARGC_INDEX("protection-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", aps_bridge.protection_gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("next-p-aps");
        if (0xFF != index)
        {
            aps_bridge.next_p_aps_en = 1;
            CTC_CLI_GET_UINT16_RANGE("next protection aps group", aps_bridge.next_aps.p_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("w-l3if-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("working interface id", aps_bridge.w_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("p-l3if-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("protection interface id", aps_bridge.p_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("raps-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("Raps groupid", raps_groupid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        aps_bridge.raps_en = TRUE;
        aps_bridge.raps_group_id = raps_groupid;
    }

    index = CTC_CLI_GET_ARGC_INDEX("physical-isolated");
    if (0xFF != index)
    {
        aps_bridge.physical_isolated = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-based-aps-en");
    if (0xFF != index)
    {
        aps_bridge.aps_failover_en = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_create_aps_bridge_group(g_api_lchip, group_id, &aps_bridge);
    }
    else
    {
        ret = ctc_aps_create_aps_bridge_group(group_id, &aps_bridge);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_set_aps_bridge_group,
        ctc_cli_aps_set_aps_bridge_group_cmd,
        "aps set aps-bridge group GROUP_ID (working-port GPORT_ID (w-l3if-id INTERFACE_ID|)|next-w-aps  ID) \
     (protection-port GPORT_ID (p-l3if-id INTERFACE_ID|)  | next-p-aps ID )",
        CTC_CLI_APS_M_STR,
        "setting",
        "Aps bridge",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Working path gport",
        CTC_CLI_GPORT_ID_DESC,
        "Working path L3 interface id",
        "INTERFACE ID",
        "Next working aps group id",
        "0-1023",
        "Protection path gport",
        CTC_CLI_GPORT_ID_DESC,
        "Protection path L3 interface id",
        "INTERFACE ID",
        "Next protection aps group id",
        "0-1023")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 group_id = 0;
    ctc_aps_bridge_group_t aps_bridge;

    sal_memset(&aps_bridge, 0, sizeof(ctc_aps_bridge_group_t));
    aps_bridge.protect_en = FALSE;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("working-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", aps_bridge.working_gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("next-w-aps");
        if (0xFF != index)
        {
            aps_bridge.next_w_aps_en = 1;
            CTC_CLI_GET_UINT16_RANGE("next working aps group", aps_bridge.next_aps.w_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }       
    }

    index = CTC_CLI_GET_ARGC_INDEX("protection-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", aps_bridge.protection_gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("next-p-aps");
        if (0xFF != index)
        {
            aps_bridge.next_p_aps_en = 1;
            CTC_CLI_GET_UINT16_RANGE("next protection aps group", aps_bridge.next_aps.p_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("w-l3if-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("working interface id", aps_bridge.w_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("p-l3if-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("protection interface id", aps_bridge.p_l3if_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_set_aps_bridge_group(g_api_lchip, group_id, &aps_bridge);
    }
    else
    {
        ret = ctc_aps_set_aps_bridge_group(group_id, &aps_bridge);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_remove_aps_bridge_group,
        ctc_cli_aps_remove_aps_bridge_group_cmd,
        "aps remove aps-bridge group GROUP_ID",
        CTC_CLI_APS_M_STR,
        "Remove",
        "Aps bridge",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC)
{
    int32 ret = 0;
    uint16 group_id = 0;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_destroy_aps_bridge_group(g_api_lchip, group_id);
    }
    else
    {
        ret = ctc_aps_destroy_aps_bridge_group(group_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_set_aps_bridge,
        ctc_cli_aps_set_aps_bridge_cmd,
        "aps set aps-bridge group GROUP_ID protect (enable|disable)",
        CTC_CLI_APS_M_STR,
        "Setting",
        "Aps bridge",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Aps bridge to protection path",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 group_id = 0;
    bool protect_en = FALSE;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("en", 1))
    {
        protect_en = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_set_aps_bridge(g_api_lchip, group_id, protect_en);
    }
    else
    {
        ret = ctc_aps_set_aps_bridge(group_id, protect_en);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_set_aps_select,
        ctc_cli_aps_set_aps_select_cmd,
        "aps set aps-select group GROUP_ID protect (enable|disable)",
        CTC_CLI_APS_M_STR,
        "Setting",
        "Aps select",
        "Aps select group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Aps select from protection path",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 group_id = 0;
    bool protect_en = FALSE;

    CTC_CLI_GET_UINT16_RANGE("aps select group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("en", 1))
    {
        protect_en = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_set_aps_select(g_api_lchip, group_id, protect_en);
    }
    else
    {
        ret = ctc_aps_set_aps_select(group_id, protect_en);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_aps_set_path,
        ctc_cli_aps_set_path_cmd,
        "aps set aps-bridge group GROUP_ID (working-port | protection-port) GPORT_ID",
        CTC_CLI_APS_M_STR,
        "Setting",
        "Aps bridge",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Working path gport",
        "Protection path gport",
        CTC_CLI_GPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 group_id = 0;
    uint16 gport = 0;uint8 index = 0;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("working-port");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_aps_set_aps_bridge_working_path(g_api_lchip, group_id, gport);
        }
        else
        {
            ret = ctc_aps_set_aps_bridge_working_path(group_id, gport);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_aps_set_aps_bridge_protection_path(g_api_lchip, group_id, gport);
        }
        else
        {
            ret = ctc_aps_set_aps_bridge_protection_path(group_id, gport);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_aps_create_and_remove_raps_group,
        ctc_cli_aps_create_and_remove_raps_group_cmd,
        "aps aps-bridge raps (create|remove) (group-id) GROUP_ID",
        CTC_CLI_APS_M_STR,
        "Aps bridge",
        "Raps",
        "Create",
        "Remove",
        "Raps mcast group id",
        CTC_CLI_APS_GROUP_ID_DESC)
{
    int32 ret = 0;
    uint16 group_id = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("group-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("Group id", group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("create");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_aps_create_raps_mcast_group(g_api_lchip, group_id);
        }
        else
        {
            ret = ctc_aps_create_raps_mcast_group(group_id);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_aps_destroy_raps_mcast_group(g_api_lchip, group_id);
        }
        else
        {
            ret = ctc_aps_destroy_raps_mcast_group(group_id);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_update_raps_member,
        ctc_cli_aps_update_raps_member_cmd,
        "aps aps-bridge raps group GROUP_ID (add|remove) (member-port) GPORT_ID (remote-chip|)",
        CTC_CLI_APS_M_STR,
        "Aps bridge",
        "Raps",
        "Aps bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Add",
        "Remove",
        "Member port",
        CTC_CLI_GPORT_ID_DESC,
        "Remote chip")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_raps_member_t raps_mem;

    sal_memset(&raps_mem, 0, sizeof(ctc_raps_member_t));

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", raps_mem.group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        raps_mem.remove_flag = FALSE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (0xFF != index)
    {
        raps_mem.remove_flag  = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("member-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", raps_mem.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != index)
    {
        raps_mem.remote_chip = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_update_raps_mcast_member(g_api_lchip, &raps_mem);
    }
    else
    {
        ret = ctc_aps_update_raps_mcast_member(&raps_mem);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_show_select,
        ctc_cli_aps_show_select_cmd,
        "show aps select group GROUP_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_APS_M_STR,
        "APS select",
        "APS select group",
        CTC_CLI_APS_GROUP_ID_DESC)
{
    int32 ret = 0;
    uint16 group_id = 0;
    bool protect_en = FALSE;

    CTC_CLI_GET_UINT16_RANGE("aps select group id", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_aps_get_aps_select(g_api_lchip, group_id, &protect_en);
    }
    else
    {
        ret = ctc_aps_get_aps_select(group_id, &protect_en);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("protect enable    :%d\n", protect_en);

    return ret;
}

CTC_CLI(ctc_cli_aps_show_bridge,
        ctc_cli_aps_show_bridge_cmd,
        "show aps bridge group GROUP_ID (working-path|protection-path|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_APS_M_STR,
        "APS bridge",
        "APS bridge group",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Working path",
        "Protecting path")
{
    int32 ret = 0;
    uint16 group_id = 0;
    bool protect_en = FALSE;
    uint32 gport = 0;
    uint8 index = 0;
    uint8 is_working_path = 0;
    uint8 is_protection_path = 0;

    CTC_CLI_GET_UINT16_RANGE("aps bridge group", group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF != index)
    {
        is_working_path = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("protection-path");
    if (0xFF != index)
    {
        is_protection_path = 1;
    }

    if (g_ctcs_api_en)
    {
        if (is_working_path)
        {
            ret = ctcs_aps_get_aps_bridge_working_path(g_api_lchip, group_id, &gport);
        }
        else if (is_protection_path)
        {
            ret = ctcs_aps_get_aps_bridge_protection_path(g_api_lchip, group_id, &gport);
        }
        else
        {
            ret = ctcs_aps_get_aps_bridge(g_api_lchip, group_id, &protect_en);
        }
    }
    else
    {
        if (is_working_path)
        {
            ret = ctc_aps_get_aps_bridge_working_path(group_id, &gport);
        }
        else if (is_protection_path)
        {
            ret = ctc_aps_get_aps_bridge_protection_path(group_id, &gport);
        }
        else
        {
            ret = ctc_aps_get_aps_bridge(group_id, &protect_en);
        }        
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (is_working_path || is_protection_path)
    {
        ctc_cli_out("Gport             :0x%04x\n", gport);
    }
    else
    {
        ctc_cli_out("protect enable    :%d\n", protect_en);
    }

    return ret;
}

CTC_CLI(ctc_cli_aps_debug_on,
        ctc_cli_aps_debug_on_cmd,
        "debug aps (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_APS_M_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
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

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = APS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = APS_SYS;
    }

    ctc_debug_set_flag("aps", "aps", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_aps_debug_off,
        ctc_cli_aps_debug_off_cmd,
        "no debug aps (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_APS_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = APS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = APS_SYS;
    }

    ctc_debug_set_flag("aps", "aps", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

int32
ctc_aps_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_aps_create_aps_bridge_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_set_aps_bridge_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_remove_aps_bridge_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_set_aps_bridge_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_set_aps_select_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_set_path_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_create_and_remove_raps_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_update_raps_member_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_aps_show_select_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_show_bridge_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_aps_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_aps_debug_off_cmd);

    return CLI_SUCCESS;
}


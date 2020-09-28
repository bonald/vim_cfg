
/**
 @file ctc_greatbel_vlan_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_linkagg_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"

extern int32
sys_usw_linkagg_show_all_member(uint8 lchip, uint8 tid);
extern int32
sys_usw_linkagg_create_channel_agg(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp);
extern int32
sys_usw_linkagg_destroy_channel_agg(uint8 lchip, uint8 tid);
extern int32
sys_usw_linkagg_add_channel(uint8 lchip, uint8 tid, uint16 gport);
extern int32
sys_usw_linkagg_remove_channel(uint8 lchip, uint8 tid, uint16 gport);
extern int32
sys_usw_linkagg_status_show(uint8 lchip);

CTC_CLI(ctc_cli_usw_linkagg_init,
        ctc_cli_usw_linkagg_init_cmd,
        "linkagg init grp_mode (group-num NUMBER | flex) (bind-gport (enable | disable) |)",
        CTC_CLI_LINKAGG_M_STR,
        "init",
        "Group Mode",
        "Group Number",
        "Number, supoort 4/8/16/32/64/56",
        "Mode FLEX",
        "Bind gport, default is enable",
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;
    ctc_linkagg_global_cfg_t linkagg_cfg;
    uint32 number;
    uint8 index = 0;

    sal_memset(&linkagg_cfg, 0, sizeof(linkagg_cfg));

    if (0 == sal_strcmp(argv[0], "flex"))
    {
        linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_FLEX;
    }
    else
    {
        CTC_CLI_GET_UINT32("group-num", number, argv[1]);

        switch (number)
        {
            case 4:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_4;
                break;
            }
            case 8:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_8;
                break;
            }
            case 16:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_16;
                break;
            }
            case 32:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_32;
                break;
            }
            case 64:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_64;
                break;
            }
            case 56:
            {
                linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_56;
                break;
            }
            default:
            {
                ctc_cli_out("unsupported group-num %s, only support 4/8/16/32/64/56\n", argv[0]);
                return CLI_ERROR;
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("bind-gport");
    if (0xFF != index)
    {
        if (0 == sal_strcmp(argv[index + 1], "disable"))
        {
            linkagg_cfg.bind_gport_disable = 1;
        }
        else
        {
            linkagg_cfg.bind_gport_disable = 0;
        }
    }
    else
    {
        linkagg_cfg.bind_gport_disable = 0;
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_init(g_api_lchip, &linkagg_cfg);
    }
    else
    {
        ret = ctc_linkagg_init(&linkagg_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_deinit,
        ctc_cli_usw_linkagg_deinit_cmd,
        "linkagg deinit",
        CTC_CLI_LINKAGG_M_STR,
        "deinit")
{
    int32 ret = CLI_SUCCESS;

    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_deinit(g_api_lchip);
    }
    else
    {
        ret = ctc_linkagg_deinit();
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_get_max_member_number,
        ctc_cli_usw_linkagg_get_max_member_number_cmd,
        "linkagg get max member number",
        CTC_CLI_LINKAGG_M_STR,
        "get",
        "Max",
        "Member",
        "Number")
{
    int32 ret = CLI_SUCCESS;
    uint16 max_num = 0;

    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_get_max_mem_num(g_api_lchip, &max_num);
    }
    else
    {
        ret = ctc_linkagg_get_max_mem_num(&max_num);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%% Max member number is: %d\n", max_num);

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_show_detail_member_port,
        ctc_cli_usw_linkagg_show_detail_member_port_cmd,
        "show linkagg AGG_ID member-ports detail (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Member ports of linkagg group",
        "all member include pad member",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 tid = 0;
    uint8 index = 0;

    CTC_CLI_GET_INTEGER("linkagg id", tid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_linkagg_show_all_member(lchip, tid);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_channel_agg_group,
        ctc_cli_usw_linkagg_channel_agg_group_cmd,
        "linkagg (create|remove) linkagg AGG_ID channel-agg (lchip LCHIP|)",
        CTC_CLI_LINKAGG_M_STR,
        "Create linkagg group",
        "Remove linkagg group",
        CTC_CLI_LINKAGG_DESC,
        "Channel aggregation",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_linkagg_group_t linkagg_grp;

    sal_memset(&linkagg_grp, 0, sizeof(ctc_linkagg_group_t));
    CTC_CLI_GET_UINT8("linkagg id", linkagg_grp.tid, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en? g_api_lchip : lchip;

    index = CTC_CLI_GET_ARGC_INDEX("create");
    if (0xFF != index)
    {
        ret = sys_usw_linkagg_create_channel_agg(lchip, &linkagg_grp);
    }
    else
    {
        ret = sys_usw_linkagg_destroy_channel_agg(lchip, linkagg_grp.tid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_channel_agg_member,
        ctc_cli_usw_linkagg_channel_agg_member_cmd,
        "linkagg AGG_ID (add | remove) member-port GPHYPORT_ID channel-agg (lchip LCHIP|)",
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Add member port to linkagg group",
        "Remove member port from linkagg group",
        "Member-port, global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Channel aggregation",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint16 gport = 0;
    uint8 index = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT8("linkagg id", tid, argv[0]);
    CTC_CLI_GET_UINT16("gport", gport, argv[2]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en? g_api_lchip : lchip;

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        ret = sys_usw_linkagg_add_channel(lchip, tid, gport);
    }
    else
    {
        ret = sys_usw_linkagg_remove_channel(lchip, tid, gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_linkagg_show_status,
        ctc_cli_usw_linkagg_show_status_cmd,
        "show linkagg status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LINKAGG_M_STR,
        "Linkagg configure and state",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ret = sys_usw_linkagg_status_show(lchip);
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_linkagg_set_property,
        ctc_cli_usw_linkagg_set_property_cmd,
        "linkagg AGG_ID property lb-hash-offset VALUE {lchip LCHIP|}",
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Linkagg property",
        "Load balance hash offset", "Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 tid = 0;
    uint32 value = 0;
    ctc_linkagg_property_t linkagg_prop = CTC_LINKAGG_PROP_LB_HASH_OFFSET;

    CTC_CLI_GET_UINT8("tid", tid, argv[0]);
    CTC_CLI_GET_UINT32("value", value, argv[1]);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-offset");
    if (0xFF != index)
    {
        linkagg_prop = CTC_LINKAGG_PROP_LB_HASH_OFFSET;
    }
    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_set_property(g_api_lchip, tid, linkagg_prop, value);
    }
    else
    {
        ret = ctc_linkagg_set_property(tid, linkagg_prop, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_show_linkagg_property,
        ctc_cli_usw_show_linkagg_property_cmd,
        "show linkagg AGG_ID property (lb-hash-offset|) {lchip LCHIP|}",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Linkagg property",
        "Load balance hash offset",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 tid = 0;
    uint32 value = 0;
    char prop_string[32] = "";
    ctc_linkagg_property_t linkagg_prop = CTC_LINKAGG_PROP_LB_HASH_OFFSET;

    CTC_CLI_GET_UINT8("tid", tid, argv[0]);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-offset");
    if (0xFF != index)
    {
        linkagg_prop = CTC_LINKAGG_PROP_LB_HASH_OFFSET;
        sal_memcpy(prop_string, "lb hash offset", sizeof("lb hash offset"));
    }
    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_get_property(g_api_lchip, tid, linkagg_prop, &value);
    }
    else
    {
        ret = ctc_linkagg_get_property(tid, linkagg_prop, &value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-24s : %5d \n", "Show linkagg", tid);
    ctc_cli_out("--------------------------------------\n");
    ctc_cli_out("%-24s : 0x%08X \n", prop_string, value);
    ctc_cli_out("--------------------------------------\n");
    return CLI_SUCCESS;
}

int32
ctc_usw_linkagg_cli_init(void)
{
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_init_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_deinit_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_get_max_member_number_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_linkagg_show_detail_member_port_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_channel_agg_group_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_channel_agg_member_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_linkagg_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_linkagg_set_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_linkagg_property_cmd);
    return CLI_SUCCESS;
}

int32
ctc_usw_linkagg_cli_deinit(void)
{
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_init_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_deinit_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_get_max_member_number_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_linkagg_show_detail_member_port_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_channel_agg_group_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_linkagg_channel_agg_member_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_linkagg_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_linkagg_property_cmd);
    return CLI_SUCCESS;
}


/**
 @file ctc_linkagg_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-18

 @version v2.0

 This file contains linkagg cli function
*/

#include "ctc_debug.h"
#include "ctc_vlan_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"


#define CTC_CLI_LINKAGG_REPLACE_MEMBER_MAX  2048
CTC_CLI(ctc_cli_linkagg_create,
        ctc_cli_linkagg_create_cmd,
        "linkagg create linkagg AGG_ID (static {lmpf|mem-ascend-order|} |failover(lmpf|)|rr (random|)|dynamic|resilient|) {member-num NUM|}",
        CTC_CLI_LINKAGG_M_STR,
        "Create linkagg group",
        CTC_CLI_LINKAGG_DESC,
        CTC_CLI_LINKAGG_ID_DESC,
        "Static linkagg group",
        "Local member first",
        "member in ascend order",
        "Static linkagg group enable failover",
        "Local member first",
        "Round robin group",
        "Random rr",
        "Dynamic linkagg group",
        "Resilient group",
        "Member num",
        "Number")
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    ctc_linkagg_group_t linkagg_grp;

    sal_memset(&linkagg_grp, 0, sizeof(ctc_linkagg_group_t));
    CTC_CLI_GET_UINT8("linkagg id", linkagg_grp.tid, argv[0]);

    linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_STATIC;
    linkagg_grp.member_num = 16;

    index = CTC_CLI_GET_ARGC_INDEX("static");
    if(index != 0xFF)
    {
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_STATIC;
        linkagg_grp.member_num = 16;
    }

    index = CTC_CLI_GET_ARGC_INDEX("failover");
    if(index != 0xFF)
    {
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_STATIC_FAILOVER;
        linkagg_grp.member_num = 16;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rr");
    if(index != 0xFF)
    {
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_RR;
        linkagg_grp.member_num = 16;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if(index != 0xFF)
    {
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_DLB;
        linkagg_grp.member_num = 32;
    }

    index = CTC_CLI_GET_ARGC_INDEX("resilient");
    if(index != 0xFF)
    {
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_RESILIENT;
        linkagg_grp.member_num = 32;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if(index != 0xFF)
    {
        linkagg_grp.flag |= CTC_LINKAGG_GROUP_FLAG_RANDOM_RR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lmpf");
    if(index != 0xFF)
    {
        linkagg_grp.flag |= CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mem-ascend-order");
    if(index != 0xFF)
    {
        linkagg_grp.flag |= CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER;
    }

    index = CTC_CLI_GET_ARGC_INDEX("member-num");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16("group member num", linkagg_grp.member_num, argv[index+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_linkagg_create(g_api_lchip, &linkagg_grp);
    }
    else
    {
        ret = ctc_linkagg_create(&linkagg_grp);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_linkagg_remove,
        ctc_cli_linkagg_remove_cmd,
        "linkagg remove linkagg AGG_ID",
        CTC_CLI_LINKAGG_M_STR,
        "Remove linkagg group",
        CTC_CLI_LINKAGG_DESC,
        CTC_CLI_LINKAGG_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;

    CTC_CLI_GET_UINT8("linkagg id", tid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_linkagg_destroy(g_api_lchip, tid);
    }
    else
    {
        ret = ctc_linkagg_destroy(tid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_linkagg_add_or_remove_port,
        ctc_cli_linkagg_add_or_remove_port_cmd,
        "linkagg AGG_ID (add | remove) member-port GPHYPORT_ID",
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Add member port to linkagg group",
        "Remove member port from linkagg group",
        "Member-port, global port",
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint16 gport = 0;

    CTC_CLI_GET_UINT8("linkagg id", tid, argv[0]);
    CTC_CLI_GET_UINT16("gport", gport, argv[2]);


    if (0 == sal_memcmp("ad", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_linkagg_add_port(g_api_lchip, tid, gport);
        }
        else
        {
            ret = ctc_linkagg_add_port(tid, gport);
        }
    }
    else if (0 == sal_memcmp("re", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_linkagg_remove_port(g_api_lchip, tid, gport);
        }
        else
        {
            ret = ctc_linkagg_remove_port(tid, gport);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_linkagg_show_member_port,
        ctc_cli_linkagg_show_member_port_cmd,
        "show linkagg AGG_ID member-ports",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Member ports of linkagg group")
{
    int32 ret = CLI_SUCCESS;
    uint16 cnt  = 0;
    uint16 idx = 0;
    uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint32* p_gports;
    uint16 max_num = 0;

    CTC_CLI_GET_UINT8("linkagg id", tid, argv[0]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_get_max_mem_num(g_api_lchip, &max_num);
    }
    else
    {
        ret =  ctc_linkagg_get_max_mem_num(&max_num);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    p_gports = (uint32*)sal_malloc(sizeof(uint32) * max_num);
    if (NULL == p_gports)
    {
        return CLI_ERROR;
    }

    sal_memset(p_gports, 0, sizeof(uint32) * max_num);

    if(g_ctcs_api_en)
    {
        ret = ctcs_linkagg_get_member_ports(g_api_lchip, tid, p_gports, &cnt);
    }
    else
    {
        ret = ctc_linkagg_get_member_ports(tid, p_gports, &cnt);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-11s%s\n", "No.", "Member-Gport");
    ctc_cli_out("----------------------\n");

    for (idx = 0; idx < cnt; idx++)
    {
        ctc_cli_out("%-11u0x%04X\n", idx + 1, p_gports[idx]);
    }

    sal_free(p_gports);
    p_gports = NULL;

    return ret;
}

CTC_CLI(ctc_cli_linkagg_debug_on,
        ctc_cli_linkagg_debug_on_cmd,
        "debug linkagg (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_LINKAGG_M_STR,
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = LINKAGG_CTC;
    }
    else
    {
        typeenum = LINKAGG_SYS;
    }

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

    ctc_debug_set_flag("linkagg", "linkagg", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_linkagg_debug_off,
        ctc_cli_linkagg_debug_off_cmd,
        "no debug linkagg (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_LINKAGG_M_STR,
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = LINKAGG_CTC;
    }
    else
    {
        typeenum = LINKAGG_SYS;
    }

    ctc_debug_set_flag("linkagg", "linkagg", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_linkagg_show_local_member_port,
        ctc_cli_linkagg_show_local_member_port_cmd,
        "show linkagg AGG_ID local-port (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "One local port of the linkagg member ports",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint16 cnt  = 0;
    uint8 tid = 0;
    uint32 gport = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT8("linkagg id", tid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
       ret = ctcs_linkagg_get_1st_local_port(g_api_lchip, tid, &gport, &cnt);
    }
    else
    {
        ret = ctc_linkagg_get_1st_local_port(lchip, tid, &gport, &cnt);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return ret;
    }

    if (cnt > 0)
    {
        ctc_cli_out("Member cnt: %-8d One of the members is: 0x%04x\n", cnt, gport);
    }
    else
    {
        ctc_cli_out("Member cnt: %-8d\n", cnt);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_linkagg_replace_ports,
        ctc_cli_linkagg_replace_ports_cmd,
        "linkagg AGG_ID replace {member-port GPORT | end} ",
        CTC_CLI_LINKAGG_M_STR,
        CTC_CLI_LINKAGG_ID_DESC,
        "Replace member ports for linkagg group",
        "Member-port, global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Input over, and replace ports")
{
    int32 ret = CLI_SUCCESS;
    static uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;
    static uint16 mem_num = 0;
    static uint32 linkagg_mem_port[CTC_CLI_LINKAGG_REPLACE_MEMBER_MAX] = {0};
    uint8  cur_tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint8 index = 0;
    uint8 is_end = 0;

    CTC_CLI_GET_UINT8("linkagg id", cur_tid, argv[0]);

    if ((cur_tid != tid) && (tid != CTC_MAX_LINKAGG_GROUP_NUM))
    {
        sal_memset(linkagg_mem_port, 0, sizeof(linkagg_mem_port));
        mem_num = 0;
        tid = CTC_MAX_LINKAGG_GROUP_NUM;
        ret = CTC_E_INVALID_PARAM;
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    tid = cur_tid;

    index = CTC_CLI_GET_ARGC_INDEX("member-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("gport", linkagg_mem_port[mem_num], argv[index+1]);
        mem_num++;
    }

    index = CTC_CLI_GET_ARGC_INDEX("end");
    if (0xFF != index)
    {
        is_end = 1;
    }

    if (is_end)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_linkagg_replace_ports(g_api_lchip, tid, linkagg_mem_port, mem_num);
        }
        else
        {
            ret = ctc_linkagg_replace_ports(tid, linkagg_mem_port, mem_num);
        }

        sal_memset(linkagg_mem_port, 0, sizeof(linkagg_mem_port));
        mem_num = 0;
        tid = CTC_MAX_LINKAGG_GROUP_NUM;
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        if (mem_num > CTC_CLI_LINKAGG_REPLACE_MEMBER_MAX)
        {
            sal_memset(linkagg_mem_port, 0, sizeof(linkagg_mem_port));
            mem_num = 0;
            tid = CTC_MAX_LINKAGG_GROUP_NUM;
            ret = CTC_E_INVALID_PARAM;
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_linkagg_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_create_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_add_or_remove_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_show_member_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_show_local_member_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_replace_ports_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_linkagg_debug_off_cmd);

    return CLI_SUCCESS;
}


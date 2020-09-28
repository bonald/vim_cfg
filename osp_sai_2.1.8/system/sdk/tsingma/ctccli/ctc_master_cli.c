#include "sal.h"
#include "dal.h"

#include "ctc_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cmd.h"

#include "ctc_l2_cli.h"
#include "ctc_mirror_cli.h"
#include "ctc_debug.h"
#include "ctc_vlan_cli.h"
#include "ctc_port_cli.h"
#include "ctc_linkagg_cli.h"
#include "ctc_ftm_cli.h"
#include "ctc_l3if_cli.h"
#include "ctc_ipuc_cli.h"
#include "ctc_efd_cli.h"
#include "ctc_mpls_cli.h"
#include "ctc_ipmc_cli.h"
#include "ctc_parser_cli.h"
#include "ctc_stats_cli.h"
#include "ctc_acl_cli.h"
#include "ctc_qos_cli.h"
#include "ctc_cpu_traffic_cli.h"
#include "ctc_pdu_cli.h"
#include "ctc_learning_aging_cli.h"
#include "ctc_security_cli.h"
#include "ctc_oam_cli.h"
#include "ctc_aps_cli.h"
#include "ctc_ptp_cli.h"
#include "ctc_sync_ether_cli.h"
#include "ctc_nexthop_cli.h"
#include "ctc_dkit_cli.h"
#include "ctc_port_mapping_cli.h"
#include "ctc_chip_cli.h"
#include "ctc_scl_cli.h"
#include "ctc_stacking_cli.h"
#include "ctc_dma_cli.h"
#include "ctc_ipfix_cli.h"
#include "ctc_monitor_cli.h"
#include "ctc_bpe_cli.h"
#include "ctc_packet_cli.h"
#include "ctc_overlay_tunnel_cli.h"
#include "ctc_trill_cli.h"
#include "ctc_ftm_cli.h"
#include "ctc_fcoe_cli.h"
#include "ctc_app_cli.h"
#include "ctc_dot1ae_cli.h"

#include "ctc_wlan_cli.h"
#include "ctc_npm_cli.h"

#include "ctc_shell_server.h"
#include "ctc_perf_test_cli.h"
#include "ctc_warmboot_cli.h"

#include "ctc_stress_test_cli.h"
#include "ctc_diag_cli.h"

#if defined(GREATBELT)
#include "./sdkcli/greatbelt/ctc_chip_special_cli.h"
#endif
#if defined(GOLDENGATE)
#include "./sdkcli/goldengate/ctc_chip_special_cli.h"
#endif
#if defined(DUET2) || defined(TSINGMA)
#include "./sdkcli/usw/ctc_chip_special_cli.h"
#endif

extern int32
ctc_cmodel_cli_init(uint8 cli_tree_mode);

extern int32
ctc_app_cli_init(void);

extern int32
ctc_sdk_deinit(void);

extern int32
ctcs_sdk_deinit(uint8 lchip);

extern int32
dal_get_chip_dev_id(uint8 lchip, uint32* dev_id);

typedef int32 ctc_sample_cli_init_callback (uint8 cli_tree_mode);
ctc_sample_cli_init_callback* sample_cli_init_callback = NULL;


typedef int32 ctc_cli_exec_fn();
ctc_cli_exec_fn* ctc_cli_exec_cb = NULL;

uint8 cli_end = 0;

uint8 g_api_lchip = 0;

extern uint8 g_cli_parser_param_en;

extern sal_file_t debug_fprintf_log_file;

ctc_chip_special_callback_fun_t g_chip_special_cli[MAX_CTC_CHIP_TYPE] = {{NULL,NULL}};

extern uint8 g_ctcs_api_en;

/* "exit" function.  */
void
ctc_cli_mode_exit(ctc_vti_t* vti)
{
    switch (vti->node)
    {
    case EXEC_MODE:
#if 0
        ctc_sdk_deinit();
        cli_end = 1;
#endif
        vti->quit(vti);
        break;

    case CTC_SDK_MODE:
    case CTC_CMODEL_MODE:
        vti->node = EXEC_MODE;
        break;

    case CTC_SDK_OAM_CHAN_MODE:
        vti->node = CTC_SDK_MODE;
        break;

    case CTC_DEBUG_MODE:
        vti->node = EXEC_MODE;
        break;

    case CTC_INTERNAL_MODE:
        vti->node = EXEC_MODE;
        break;

    case CTC_APP_MODE:
        vti->node = EXEC_MODE;
        break;

    case CTC_DKITS_MODE:
        vti->node = EXEC_MODE;
        break;

    case CTC_STRESS_MODE:
        vti->node = EXEC_MODE;
        break;

    default:
        vti->node = EXEC_MODE;
        break;
    }
}

CTC_CLI(ctc_cli_common_help,
        ctc_cli_common_help_cmd,
        "help",
        "Description of the interactive help system")
{
    ctc_cli_out("  SDK CLI provides advanced help feature.  When you need help,\n\
	  anytime at the command line please press '?'.\n\
	  If nothing matches, the help list will be empty and you must backup\n\
	  until entering a '?' shows the available options.\n\
	  Two styles of help are provided:\n\
	  1. Full help is available when you are ready to enter a \n\
	      command argument (e.g. 'show ?') and describes each possible \n\
	      argument.\n\
	  2. Partial help is provided when an abbreviated argument is entered \n\
	      and you want to know what arguments match the input \n\
	      (e.g. 'show ve?').\n");

    return CLI_SUCCESS;
}

/* Generic "ctc_cli_common_end" command.  */
CTC_CLI(ctc_cli_common_show_ver,
        ctc_cli_common_show_ver_cmd,
        "show version",
        CTC_CLI_SHOW_STR,
        "Sdk version")
{
    char chipname[20] = {0};
    uint32 dev_id = 0;

    g_ctcs_api_en ? dal_get_chip_dev_id(g_api_lchip, &dev_id) : dal_get_chip_dev_id(0, &dev_id);

    switch(dev_id)
    {
        case DAL_HUMBER_DEVICE_ID:
            sal_strcpy(chipname, "Humber");
            break;
        case DAL_GREATBELT_DEVICE_ID:
            sal_strcpy(chipname, "Greatbelt");
            break;
        case DAL_GOLDENGATE_DEVICE_ID:
        case DAL_GOLDENGATE_DEVICE_ID1:
            sal_strcpy(chipname, "Goldengate");
            break;
        case DAL_DUET2_DEVICE_ID:
            sal_strcpy(chipname, "Duet2");
            break;
        case DAL_TSINGMA_DEVICE_ID:
            sal_strcpy(chipname, "Tsingma");
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }
    ctc_cli_out("    SDK %s Released at %s. Chip Series: %s \n\
    Copyright (C) %s Centec Networks Inc.  All rights reserved.\n\
    Compile time: %s %s\n",
    CTC_SDK_VERSION_STR, CTC_SDK_RELEASE_DATE, chipname, CTC_SDK_COPYRIGHT_TIME,__DATE__,__TIME__);

    return CLI_SUCCESS;
}

/* Generic "ctc_cli_common_end" command.  */
CTC_CLI(ctc_cli_common_end,
        ctc_cli_common_end_cmd,
        "end",
        "End current mode and change to EXEC mode")
{
    if (g_ctc_vti->node != EXEC_MODE)
    {
        g_ctc_vti->node = EXEC_MODE;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_exit_config,
        ctc_cli_common_exit_config_cmd,
        "exit",
        "Exit current mode and down to previous mode")
{
    ctc_cli_mode_exit(vty);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_quit,
        ctc_cli_common_quit_cmd,
        "quit",
        "Exit current mode and down to previous mode")
{
    ctc_cli_mode_exit(vty);
    return CLI_SUCCESS;
}
#if defined(SDK_IN_USERMODE)
CTC_CLI(ctc_cli_common_exit_server,
        ctc_cli_common_exit_server_cmd,
        "exit server",
        "Exit current mode and down to previous mode"
        "Exit Server Progress")
{
    ctc_sdk_deinit();

    exit(0);

    return CLI_SUCCESS;
}
#endif
CTC_CLI(ctc_cli_common_enter_sdk_mode,
        ctc_cli_common_enter_sdk_mode_cmd,
        "enter (sdk|debug|internal|app|cmodel|dkits|stress) mode",
        "Enter",
        "SDK mode",
        "Debug mode",
        "Internal Debug mode",
        "App mode",
        "Cmodel mode",
        "Dkits mode",
        "Stress mode",
        "Mode")
{

    if (CLI_CLI_STR_EQUAL("sdk", 0))
    {
        g_ctc_vti->node  = CTC_SDK_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("debug", 0))
    {
        g_ctc_vti->node  = CTC_DEBUG_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("internal", 0))
    {
        g_ctc_vti->node  = CTC_INTERNAL_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("app", 0))
    {
        g_ctc_vti->node  = CTC_APP_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("cmodel", 0))
    {
        g_ctc_vti->node  = CTC_CMODEL_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("dkits", 0))
    {
        g_ctc_vti->node  = CTC_DKITS_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("stress", 0))
    {
        g_ctc_vti->node  = CTC_STRESS_MODE;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_common_fast_enter_sdk_mode,
        ctc_cli_common_fast_enter_sdk_mode_cmd,
        "(sdk|debug|internal|app|cmodel|dkits|stress)",
        "SDK mode",
        "Debug mode",
        "Internal Debug mode",
        "App mode",
        "Cmodel mode",
        "Dkits mode",
        "Stress mode")
{
    if (CLI_CLI_STR_EQUAL("sdk", 0))
    {
        g_ctc_vti->node  = CTC_SDK_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("debug", 0))
    {
        g_ctc_vti->node  = CTC_DEBUG_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("internal", 0))
    {
        g_ctc_vti->node  = CTC_INTERNAL_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("app", 0))
    {
        g_ctc_vti->node  = CTC_APP_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("cmodel", 0))
    {
        g_ctc_vti->node  = CTC_CMODEL_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("dkits", 0))
    {
        g_ctc_vti->node  = CTC_DKITS_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("stress", 0))
    {
        g_ctc_vti->node  = CTC_STRESS_MODE;
    }

    return CLI_SUCCESS;
}

#if defined SDK_IN_USERMODE
/*******************************************************************************
 * Name:   app_show_mem_summary_info
 * Purpose: show summary memory info
 * Input:   cli
 * Output:  nTotal, nFree, nBuffers
 * Return: N/A
 * Note: N/A
 ******************************************************************************/
STATIC int
ctc_app_show_mem_summary_info(int* total, int* free, int* buf)
{
    sal_file_t pFile = NULL;

#define MAX_LINE_CHAR 128
    char strLine[MAX_LINE_CHAR] = {0};
    char strName[MAX_LINE_CHAR] = {0};
    int nTotal, nFree, nBuffers;

    pFile = sal_fopen("/proc/meminfo", "r");
    if ((NULL == pFile))
    {
        ctc_cli_out("%% Can not open meminfo file.");
        return -1;
    }

    sal_memset(strLine, 0, sizeof(strLine));
    sal_memset(strName, 0, sizeof(strName));

    /*first line is total memory size*/
    sal_fgets(strLine, MAX_LINE_CHAR, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nTotal);

    /*second line is free memory size*/
    sal_fgets(strLine, MAX_LINE_CHAR, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nFree);

    /*third line is buffer memory size*/
    sal_fgets(strLine, MAX_LINE_CHAR, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nBuffers);

    sal_fclose(pFile);

    *total = nTotal;
    *free = nFree;
    *buf = nBuffers;

    return 0;
}

CTC_CLI(ctc_cli_common_show_memory_summary_total,
        ctc_cli_common_show_memory_summary_total_cmd,
        "show memory summary total",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SHOW_SYS_MEM_STR,
        "Summary of memory statistics",
        "Total"
        )
{
    int nTotal, nFree, nBuffers;
    float utilization;

    /* show summary mem info*/
#if defined(SDK_IN_USERMODE)
    system("sync && echo 3 > /proc/sys/vm/drop_caches");    /*clear cache memory, notice that cache is not 0 after clear*/
#endif
    ctc_app_show_mem_summary_info(&nTotal, &nFree, &nBuffers);
    utilization = ((float)nTotal - (float)nFree) / (float)nTotal * 100;
    ctc_cli_out("Total memory      : %-8dKB\n", nTotal);
    ctc_cli_out("Used memory       : %-8dKB\n", nTotal - nFree);
    ctc_cli_out("Free memory       : %-8dKB\n", nFree);
    ctc_cli_out("Buffer memory     : %-8dKB\n", nBuffers);

    ctc_cli_out("Memory utilization: %.2f%%\n", utilization);

    return CLI_SUCCESS;
}
#endif

#if defined(SDK_IN_KERNEL) || defined(SDK_IN_USERMODE)
CTC_CLI(ctc_cli_common_show_memory_detail,
        ctc_cli_common_show_memory_detail_cmd,
        "show memory pool detail",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SHOW_SYS_MEM_STR,
        "Memory pool",
        "Detail of memory statistics"
        )
{
    uint8 cnt = 0;
    int32 ret = 0;
    bucket_info_t bucket_used_info;
    sal_memset(&bucket_used_info, 0 ,sizeof(bucket_info_t));

    ctc_cli_out("MEMMGR::Memory bucket used counter: \n");
    ctc_cli_out("%-24s %-16s\n", "bucket size(bytes)", "used_count(entries)");

    while(1)
    {
        ret = mem_get_bucket_used_info(&bucket_used_info, cnt);
        if (ret < 0)
        {
            break;
        }
        else
        {
            ctc_cli_out("%-24s %-16d \n", bucket_used_info.name, bucket_used_info.used_count);

        }
        cnt++;
    }

    ctc_cli_out("\n");
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_show_memory_module_detail,
        common_cli_show_memory_module_detaill_cmd,
        "show memory pool stats (fdb|mpls|queue|ipuc|overlay-tunnel|monitor|efd|ipmc|nexthop|scl" \
        "|l3if|port|vlan|oam|sort-key|linklist|avl-tree|ctc-cli|lib-ctccli|vector|hash" \
        "|spool |ptp | ftm |stacking |dma|storm-control |mirror|sal"
        "|opf|l3hash | fpa" \
        "|acl | sync_ether| rpf| aps | ipfix | trill | fcoe | wlan | dot1ae|" \
        "system | stats | vpn | linkagg | usrid"
        "|all) (detail|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SHOW_SYS_MEM_STR,
        "Memory pool",
        "Memory statistics",
        "FDB module",
        "Mpls module",
        "Queue module",
        "IPUC module",
        "OVERLAY TUNNEL module",
        "Monitor module",
        "Efd module",
        "IPMC module",
        "Nexthop module",
        "SCL module",
        CTC_CLI_L3IF_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_VLAN_M_STR,
        "OAM module",
        "Sort-key module",
        "Linklist module",
        "Avl Tree",
        "CTCCLI",
        "LIBCTCCLI",
        "Vector",
        "Hash",
        "Share pool",
        "Ptp",
        "Ftm",
        "Stacking",
        "Dma",
        "Storm control",
        "Mirror",
        "SAL",
        "OPF",
        "L3_HASH",
        "FPA",
        "ACL",
        "Sync ether",
        "Rpf",
        "Aps",
        "ipfix",
        "trill",
        "fcoe",
        "wlan",
        "Dot1ae",
        "System",
        "Stats",
        "Vpn",
        "Linkagg",
        "Usrid",
        "All",
        "Detail")
{

    uint8 mtype = 0;
    uint8 flag = 0;
    int8* m_name = NULL;
    mem_table_t mtype_table_info;
    int32 ret = 0;

    sal_memset(&mtype_table_info, 0, sizeof(mem_table_t));

    if (0 == sal_memcmp(argv[0], "fdb", sal_strlen("fdb")))
    {
        mtype = MEM_FDB_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "mpls", sal_strlen("mpls")))
    {
        mtype = MEM_MPLS_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "queue", sal_strlen("queue")))
    {
        mtype = MEM_QUEUE_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "ipuc", sal_strlen("ipuc")))
    {
        mtype = MEM_IPUC_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "overlay-tunnel", sal_strlen("overlay-tunnel")))
    {
        mtype = MEM_OVERLAY_TUNNEL_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "monitor", sal_strlen("monitor")))
    {
        mtype = MEM_MONITOR_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "efd", sal_strlen("efd")))
    {
        mtype = MEM_EFD_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "ipmc", sal_strlen("ipmc")))
    {
        mtype = MEM_IPMC_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "scl", sal_strlen("scl")))
    {
        mtype = MEM_SCL_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "nexthop", sal_strlen("nexthop")))
    {
        mtype = MEM_NEXTHOP_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "l3if", sal_strlen("l3if")))
    {
        mtype = MEM_L3IF_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "port", sal_strlen("port")))
    {
        mtype = MEM_PORT_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "vlan", sal_strlen("vlan")))
    {
        mtype = MEM_VLAN_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "oam", sal_strlen("oam")))
    {
        mtype = MEM_OAM_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "linklist", sal_strlen("linklist")))
    {
        mtype = MEM_LINKLIST_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "avl-tree", sal_strlen("avl-tree")))
    {
        mtype = MEM_AVL_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "ctc-cli", sal_strlen("ctc-cli")))
    {
        mtype = MEM_CLI_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "lib-ctccli", sal_strlen("lib-ctccli")))
    {
        mtype = MEM_LIBCTCCLI_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "vector", sal_strlen("vector")))
    {
        mtype = MEM_VECTOR_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "hash", sal_strlen("hash")))
    {
        mtype = MEM_HASH_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("spool", 0)
    {
        mtype = MEM_SPOOL_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("ptp", 0)
    {
        mtype = MEM_PTP_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("ftm", 0)
    {
        mtype = MEM_FTM_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("stacking", 0)
    {
        mtype = MEM_STK_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("dma", 0)
    {
        mtype = MEM_DMA_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("storm-control", 0)
    {
        mtype = MEM_STMCTL_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("mirror", 0)
    {
        mtype = MEM_MIRROR_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("sal", 0)
    {
        mtype = MEM_SAL_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "opf", sal_strlen("opf")))
    {
        mtype = MEM_OPF_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "l3hash", sal_strlen("l3hash")))
    {
        mtype = MEM_L3_HASH_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "fpa", sal_strlen("fpa")))
    {
        mtype = MEM_FPA_MODULE;
    }
    else if CLI_CLI_STR_EQUAL("acl", 0)
    {
        mtype = MEM_ACL_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("sync_ether", 0)
    {
        mtype = MEM_SYNC_ETHER_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("rpf", 0)
    {
        mtype = MEM_RPF_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("aps", 0)
    {
        mtype = MEM_APS_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("ipfix", 0)
    {
        mtype = MEM_IPFIX_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("trill", 0)
    {
        mtype = MEM_TRILL_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("fcoe", 0)
    {
        mtype = MEM_FCOE_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("wlan", 0)
    {
        mtype = MEM_WLAN_MODULE;
    }
    else if  CLI_CLI_STR_EQUAL("dot1ae", 0)
    {
        mtype = MEM_DOT1AE_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "system", sal_strlen("system")))
    {
        mtype = MEM_SYSTEM_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "stats", sal_strlen("stats")))
    {
        mtype = MEM_STATS_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "vpn", sal_strlen("vpn")))
    {
        mtype = MEM_VPN_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "linkagg", sal_strlen("linkagg")))
    {
        mtype = MEM_LINKAGG_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "usrid", sal_strlen("usrid")))
    {
        mtype = MEM_USRID_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "sort-key", sal_strlen("sort-key")))
    {
        mtype = MEM_SORT_KEY_MODULE;
    }
    else if (0 == sal_memcmp(argv[0], "all", sal_strlen("all")))
    {
        flag = 1;
    }

    if (flag)
    {
        for (mtype = 0; mtype < MEM_TYPE_MAX; mtype++)
        {
            ret = mem_mgr_get_mtype_info(mtype, &mtype_table_info, &m_name);
            if (!ret)
            {
                ctc_cli_out("MEMMGR:: mtype %s, req_size %u, total size %u (debug used:%u), block_count %u\n",
                    m_name, mtype_table_info.req_size, mtype_table_info.size,
                    (mtype_table_info.count *sizeof(struct mem_block_header)),
                    mtype_table_info.count);
            }
        }
    }
    else
    {
        ret = mem_mgr_get_mtype_info(mtype, &mtype_table_info, &m_name);
        if(!ret)
        {
            ctc_cli_out("MEMMGR:: mtype %s, req_size %u, total size %u (debug used:%u), block_count %u\n",
                m_name, mtype_table_info.req_size, mtype_table_info.size,
                (mtype_table_info.count * sizeof(struct mem_block_header)),
                mtype_table_info.count);
        }
    }

    return CLI_SUCCESS;
}
#endif

CTC_CLI(ctc_cli_common_show_error_desc,
        ctc_cli_common_show_error_desc_cmd,
        "show error ERROR_ENUM",
        CTC_CLI_SHOW_STR,
        "Error description",
        "<-32000 ~ -10000>")
{
    int32 ret;

    CTC_CLI_GET_INTEGER("error enum", ret, argv[0]);
    ctc_cli_out(" %s\n", ctc_get_error_desc(ret));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_set_error_mapping,
        ctc_cli_common_set_error_mapping_cmd,
        "error mapping (enable|disable)",
        "Error code",
        "Mapping",
        "Enable",
        "Disable")
{
    uint8 index = 0;
    uint8 enable = 0;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }
    ctc_set_error_code_mapping_en(enable);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_debug_on,
        ctc_cli_common_debug_on_cmd,
        "debug on",
        CTC_CLI_DEBUG_STR,
        "Enable debugging information")
{

    ctc_debug_enable(TRUE);

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_common_debug_off,
        ctc_cli_common_debug_off_cmd,
        "no debug on",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Enable debugging information")
{

    ctc_debug_enable(FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_debug_show,
        ctc_cli_common_debug_show_cmd,
        "show debug on",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Enable debugging information")
{
    ctc_cli_out("Debug on:%s\n", (ctc_get_debug_enable() == TRUE) ? "TRUE" : "FALSE");
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_interrupt_debug_on,
        ctc_cli_common_interrupt_debug_on_cmd,
        "debug interrupt (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "Interrupt module",
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
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
        typeenum = INTERRUPT_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = INTERRUPT_SYS;
    }

    ctc_debug_set_flag("interrupt", "interrupt", typeenum, level, TRUE);

    return CLI_SUCCESS;
}





CTC_CLI(ctc_cli_common_interrupt_debug_off,
        ctc_cli_common_interrupt_debug_off_cmd,
        "no debug interrupt (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Interrupt module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = INTERRUPT_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = INTERRUPT_SYS;
    }

    ctc_debug_set_flag("interrupt", "interrupt", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_interrupt_show_debug,
        ctc_cli_common_interrupt_show_debug_cmd,
        "show debug interrupt (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Interrupt module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
    bool is_on = FALSE;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = INTERRUPT_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = INTERRUPT_SYS;
    }

    is_on = ctc_debug_get_flag("interrupt", "interrupt", typeenum, &level);
    ctc_cli_out("interrupt: %s debug %s  level: %s\n", argv[0], is_on ? "on" : "off",
                ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}


#if defined(SDK_IN_USERMODE)

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif /* INET_ADDRSTRLEN */

int32
ctc_cli_set_gw(char* gw_address, uint32 is_add)
{
    char strCmd[256] = {0};
    int32 ret = 0;

    if (gw_address == NULL)
    {
        return -1;
    }

    sal_memset(strCmd, 0, sizeof(strCmd));

    if (1 == is_add)
    {
        sal_sprintf(strCmd, "route add default gw %200s >/dev/null 2>&1", gw_address);
        ret = system(strCmd);
        if (ret)
        {
            return -1;
        }
    }
    else
    {
        sal_sprintf(strCmd, "route del default gw %200s >/dev/null 2>&1", gw_address);
        ret = system(strCmd);
        if (ret)
        {
            return -1;
        }
    }

    return 0;
}

CTC_CLI(ctc_cli_common_gateway,
        ctc_cli_common_gateway_cmd,
        "route (add|del) gateway A.B.C.D",
        "Route",
        "Add",
        "Delete",
        "Default gateway",
        "IP address")
{
    struct in_addr gw;
    uint32 is_add = FALSE;
    int32 ret = 0;

    if (0 == sal_strcmp(argv[0], "add"))
    {
        is_add = TRUE;
    }

    ret = sal_inet_pton(AF_INET, argv[1], &gw);
    if (ret == 0)
    {
        ctc_cli_out("Invalid IP address: %s\n");
        return -1;
    }

    ret = ctc_cli_set_gw(argv[1], is_add);
    if (ret < 0)
    {
        ctc_cli_out("%% Set gateway fail\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_tftp_debug,
        ctc_cli_common_tftp_debug_cmd,
        "tftp server-ip IPADDRESS filename FILENAME",
        "tftp transmit",
        "Server ip address",
        "Input server ip address",
        "File name",
        "Input file name")
{
    int32 ret = CTC_E_NONE;
    char strCmd[256] = {0};

    sal_memset(strCmd, 0, 256);
    sprintf(strCmd, "tftp -v -Z %s -m binary -s 2048 -c get -r %s", argv[0], argv[1]);

    chdir("/mnt/flash/boot");
    ret = system(strCmd);
    if (ret == 0)
    {
        ctc_cli_out("tftp command execute OK.\n");
    }
    else
    {
        ctc_cli_out("%% tftp command execute NG.\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_boot_debug,
        ctc_cli_common_boot_debug_cmd,
        "boot system-image IMAGE",
        "boot command",
        "System image",
        "Input system image file")
{
    chdir("/mnt/flash/boot");
    unlink("uImage");
    if (symlink(argv[0], "uImage") != 0)
    {
        ctc_cli_out("%% Setting the next boot image file faild: %s\n",
                    strerror(errno));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_rm_debug,
        ctc_cli_common_rm_debug_cmd,
        "rm FILENAME",
        "delete file",
        "File name")
{
    char strCmd[256] = {0};

    sal_memset(strCmd, 0, 256);
    sprintf(strCmd, "rm -f %s", argv[0]);

    int32 ret = 0;
    ret = system(strCmd);
    if (ret != 0)
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#endif

#ifdef SDK_UNIT_TEST
extern int gtest_main(int argc, char** argv);

CTC_CLI(ctc_cli_common_unit_test_start,
        ctc_cli_common_unit_test_start_cmd,
        "unittest",
        "Unit Test")
{
    int ut_argc = 1;
    char* ut_argv[1];

    ut_argv[0] = "ctcsdkut";
    gtest_main(ut_argc, ut_argv);
    return CLI_SUCCESS;
}

#endif /* SDK_UNIT_TEST */

ctc_cmd_node_t exec_node =
{
    EXEC_MODE,
    "\rCTC_CLI# ",
};

ctc_cmd_node_t sdk_node =
{
    CTC_SDK_MODE,
    "\rCTC_CLI(ctc-sdk)# ",
};

ctc_cmd_node_t cmodel_node =
{
    CTC_CMODEL_MODE,
    "\rCTC_CLI(ctc-cmodel)# ",
};

ctc_cmd_node_t oam_chan_node =
{
    CTC_SDK_OAM_CHAN_MODE,
    "\rCTC_CLI(oam_chan)# ",
};

ctc_cmd_node_t debug_node =
{
    CTC_DEBUG_MODE,
    "\rCTC_CLI(ctc-debug)# ",
};

ctc_cmd_node_t internal_node =
{
    CTC_INTERNAL_MODE,
    "\rCTC_CLI(ctc-internal)# ",
};

ctc_cmd_node_t app_node =
{
    CTC_APP_MODE,
    "\rCTC_CLI(ctc-app)# ",
};

ctc_cmd_node_t dkits_node =
{
    CTC_DKITS_MODE,
    "\rCTC_CLI(ctc-dkits)# ",
};

ctc_cmd_node_t stress_node =
{
    CTC_STRESS_MODE,
    "\rCTC_CLI(ctc-stress)# ",
};

CTC_CLI(ctc_cli_common_hostname,
        ctc_cli_common_hostname_cmd,
        "enter lchip LCHIP",
        "Enter",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    char hostname[10];
    uint8 chip_type_old = 0;
    uint8 chip_type_new = 0;

    CTC_CLI_GET_UINT8("lchip", lchip, argv[0]);

    chip_type_old = ctcs_get_chip_type(g_api_lchip);
    chip_type_new = ctcs_get_chip_type(lchip);

    if (lchip >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        ctc_cli_out("%% Invalid lchip \n");
        return CLI_ERROR;
    }

    if (CTC_CHIP_NONE == chip_type_new)
    {
        ctc_cli_out("%% Invalid lchip \n");
        return CLI_ERROR;
    }

    if (g_ctcs_api_en && (chip_type_old != chip_type_new))
    {
        ctc_dkit_cli_chip_special_deinit(g_api_lchip);
        (*g_chip_special_cli[chip_type_old].chip_special_cli_deinit)();
        ctc_dkit_cli_chip_special_init(lchip);
        (*g_chip_special_cli[chip_type_new].chip_special_cli_init)();
    }
    g_api_lchip = lchip;

    sal_sprintf(hostname, ".%d", lchip);
    sal_sprintf(exec_node.prompt, "\rCTC_CLI%s# ", hostname);
    sal_sprintf(sdk_node.prompt, "\rCTC_CLI%s(ctc-sdk)# ", hostname);
    sal_sprintf(cmodel_node.prompt, "\rCTC_CLI%s(ctc-cmodel)# ", hostname);
    sal_sprintf(oam_chan_node.prompt, "\rCTC_CLI%s(oam_chan)# ", hostname);
    sal_sprintf(debug_node.prompt, "\rCTC_CLI%s(ctc-debug)# ", hostname);
    sal_sprintf(internal_node.prompt, "\rCTC_CLI%s(ctc-internal)# ", hostname);
    sal_sprintf(dkits_node.prompt, "\rCTC_CLI%s(ctc-dkits)# ", hostname);
    sal_sprintf(stress_node.prompt, "\rCTC_CLI%s(ctc-stress)# ", hostname);
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_common_error_debug,
        ctc_cli_common_error_debug_cmd,
        "debug error (on|off)",
        CTC_CLI_DEBUG_STR,
        "Error return",
        "ON",
        "OFF")
{

    if (CLI_CLI_STR_EQUAL("on", 0))
    {
        g_error_on = 1;
    }
    else
    {
        g_error_on = 0;
    }


    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_debug_cli_parser_parm,
        ctc_cli_common_debug_cli_parser_parm_cmd,
        "debug cli parser-param (enable|disable)",
        CTC_CLI_DEBUG_STR,
        "CLI",
        "Parse Parameter",
        "Enable",
        "Disable")
{

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        g_cli_parser_param_en = 1;
    }
    else
    {
        g_cli_parser_param_en = 0;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_show_task,
        ctc_cli_common_show_task_cmd,
        "show task",
        CTC_CLI_SHOW_STR,
        "Display all thread")
{
    ctc_sal_task_dump();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_set_task,
        ctc_cli_common_set_task_cmd,
        "task set (pid PID) {cpu-mask MASK | priority PRIO}",
        "task",
        "set cfg",
        "task pid",
        "pid value",
        "cpu mask for the task",
        "mask value max 64bit",
        "priority for the task",
        "task's priority 1-99 for realtime SCHED_RR, 100-139 for nice priority")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    int32 pid = 0;
    uint64 cpu_mask = 0;
    uint32 prio = 0;

    index = CTC_CLI_GET_ARGC_INDEX("pid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("pid", pid, argv[index + 1]);

    }
    index = CTC_CLI_GET_ARGC_INDEX("cpu-mask");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT64("cpu mask", cpu_mask, argv[index + 1]);

    }
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("priority", prio, argv[index + 1]);

    }

    ret = ctc_sal_task_set_affinity_prio(pid, cpu_mask, prio);
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d\n", ret);
        return CLI_ERROR;
    }

    return ret;
}

#if defined SDK_IN_USERMODE
CTC_CLI(ctc_cli_common_get_task,
        ctc_cli_common_get_task_cmd,
        "task get (task-count COUNT) (lchip LCHIP|)",
        "task",
        "get cpu-mask and priority",
        "task total num",
        "count value ")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0, count = 0;
    uint8 i = 0;
    sal_task_t* array = NULL;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("task-count");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("task count", count, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    MALLOC_ZERO(MEM_CLI_MODULE, array, sizeof(sal_task_t)* count);

    ret = ctc_sal_get_task_list(lchip,  count, array);
    if (ret < 0)
    {
        if(ret == -1)
        {
            ctc_cli_out("%% count is not valid\n");
        }
        else
        {
            ctc_cli_out("%% minimal count is %d\n", 0-ret);
        }
        mem_free(array);
        return CLI_ERROR;
    }

    ctc_cli_out("name                pid     task-type   cpu-mask       priority\n");
    for(i = 0; i < count; i++)
    {
        if (!array[i].tid)
        {
            break;
        }

         ctc_cli_out("%-20s", array[i].name);
         ctc_cli_out("%-10u", array[i].tid);
         ctc_cli_out("%-10d ", array[i].task_type);
         ctc_cli_out("0x%-10"PRIx64, array[i].cpu_mask);
         ctc_cli_out("%10d\n", array[i].prio);
    }

    mem_free(array);

    return ret;
}
#endif

int32
ctc_register_sample_init_cli_callback(void* func)
{
    CTC_PTR_VALID_CHECK(func);
    sample_cli_init_callback = (ctc_sample_cli_init_callback*)func;

    return 0;
}

int32
ctc_register_cli_exec_cb(void* cb)
{
    CTC_PTR_VALID_CHECK(cb);
    ctc_cli_exec_cb = (ctc_cli_exec_fn*)cb;

    return 0;
}


int32
ctc_com_cli_install(void)
{
    /*enter cli*/
    install_element(EXEC_MODE, &ctc_cli_common_enter_sdk_mode_cmd);

    install_element(EXEC_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_CMODEL_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_DEBUG_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_DKITS_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_common_fast_enter_sdk_mode_cmd);

    /*help CLIs*/
    install_element(CTC_SDK_MODE, &ctc_cli_common_help_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_show_ver_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_hostname_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_error_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_debug_cli_parser_parm_cmd);

    /*quit CLIs*/
    install_element(EXEC_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_CMODEL_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_DEBUG_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_DKITS_MODE, &ctc_cli_common_exit_config_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_common_exit_config_cmd);

    install_element(EXEC_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_CMODEL_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_DEBUG_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_DKITS_MODE, &ctc_cli_common_quit_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_common_quit_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_CMODEL_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_DEBUG_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_DKITS_MODE, &ctc_cli_common_end_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_common_end_cmd);

    return 0;
}


int32
ctc_feature_cli_install(void)
{
    uint8 chip_type = 0;

    ctc_vlan_cli_init();
    ctc_linkagg_cli_init();
    ctc_l2_cli_init();
    ctc_mirror_cli_init();
    ctc_port_cli_init();
    ctc_port_mapping_cli_init();
    ctc_nexthop_cli_init();
    ctc_ipuc_cli_init();
    ctc_fcoe_cli_init();
    ctc_efd_cli_init();
    ctc_l3if_cli_init();
    ctc_parser_cli_init();
    ctc_stats_cli_init();
    ctc_acl_cli_init();
    ctc_qos_cli_init();
     /*   ctc_cpu_traffic_cli_init();*/
    ctc_pdu_cli_init();
    ctc_ipmc_cli_init();
    ctc_advanced_vlan_cli_init();
    ctc_mpls_cli_init();
    ctc_security_cli_init();
    ctc_oam_cli_init();
    ctc_aps_cli_init();
    ctc_ptp_cli_init();
    ctc_sync_ether_cli_init();
    ctc_learning_aging_cli_init();

    ctc_chip_cli_init();
    ctc_scl_cli_init();
    ctc_stacking_cli_init();
    ctc_bpe_cli_init();
    ctc_ipfix_cli_init();
    ctc_ftm_cli_init();
    ctc_monitor_cli_init();
    ctc_packet_cli_init();
    ctc_overlay_tunnel_cli_init();
    ctc_trill_cli_init();
    ctc_app_api_cli_init();
    ctc_perf_test_cli_init();

    ctc_stress_test_cli_init();
#if (FEATURE_MODE == 0)
    ctc_wlan_cli_init();
    ctc_npm_cli_init();
    ctc_dot1ae_cli_init();
#endif

#ifdef GREATBELT
    ctc_greatbelt_chip_special_cli_callback_init();
#endif
#if defined(GOLDENGATE)
    ctc_goldengate_chip_special_cli_callback_init();
#endif
#if defined(DUET2) || defined(TSINGMA)
    ctc_usw_chip_special_cli_callback_init();
#endif

    chip_type = g_ctcs_api_en?ctcs_get_chip_type(0):ctc_get_chip_type();
    (*g_chip_special_cli[chip_type].chip_special_cli_init)();

    ctc_warmboot_cli_init();

    ctc_diag_cli_init();
    return 0;
}

int ctc_master_printf(struct ctc_vti_struct_s* vti, const char *szPtr, const int szPtr_len)
{
    sal_write(0,(void*)szPtr,szPtr_len);
    return 0;
}

int ctc_master_fprintf(struct ctc_vti_struct_s* vti, const char *szPtr, const int szPtr_len)
{
    sal_fprintf(debug_fprintf_log_file, "%s", szPtr);
    return 0;
}

int ctc_master_quit(struct ctc_vti_struct_s* vti)
{
    uint8 lchip = 0;
    uint8 lchip_num = 0;

    if (g_ctcs_api_en)
    {
        ctcs_get_local_chip_num(0, &lchip_num);
        for (lchip = 0; lchip < lchip_num; lchip++)
        {
            ctcs_sdk_deinit(lchip);
        }
    }
    else
    {
        ctc_sdk_deinit();
    }
    cli_end = 1;

    return 0;
}

#if defined(_SAL_LINUX_UM)
STATIC void
ctc_cli_ctrl_c_handler(int sig)
{

    if(g_ctc_vti->for_en == TRUE)
    {
        g_ctc_vti->for_en = FALSE;
        return;
    }
    ctc_master_quit(NULL);
    restore_terminal_mode(CTC_VTI_SHELL_MODE_DEFAULT);
    exit(0);
}

int32
ctc_cli_ctrlc_enable_set(int enable)
{
    struct sigaction sigact;

    if (enable)
    {
        sigemptyset(&sigact.sa_mask);
        sigact.sa_handler = ctc_cli_ctrl_c_handler;
        sigact.sa_flags = SA_NODEFER;
        if (sigaction(SIGINT, &sigact, NULL) < 0)
        {
            return -1;
        }
    }
    else
    {
        signal(SIGINT, SIG_DFL);
    }
    return 0;
}
#endif

int32 ctc_cli_init(void)
{
    /*register ctl-C callback*/
#if defined(_SAL_LINUX_UM)
    ctc_cli_ctrlc_enable_set(1);
#endif

    ctc_debug_register_cb(ctc_cli_out);
    ctc_debug_register_log_cb(ctc_cli_out);

    ctc_cmd_init(0);
    /* Install top nodes. */
    ctc_install_node(&exec_node, NULL);
    ctc_install_node(&sdk_node, NULL);
    ctc_install_node(&cmodel_node, NULL);
    ctc_install_node(&oam_chan_node, NULL);
    ctc_install_node(&debug_node, NULL);
    ctc_install_node(&internal_node, NULL);
    ctc_install_node(&app_node, NULL);
    ctc_install_node(&dkits_node, NULL);
    ctc_install_node(&stress_node, NULL);

    ctc_vti_init(CTC_SDK_MODE);
    /*common CLIs*/
    ctc_com_cli_install();

    /*environment_setting CLIs*/
#if defined(SDK_IN_USERMODE)
    install_element(CTC_SDK_MODE, &ctc_cli_common_show_memory_summary_total_cmd);
#endif
#if defined(SDK_IN_KERNEL) || defined(SDK_IN_USERMODE)
    install_element(CTC_SDK_MODE, &ctc_cli_common_show_memory_detail_cmd);
    install_element(CTC_SDK_MODE, &common_cli_show_memory_module_detaill_cmd);
#endif
    install_element(CTC_SDK_MODE, &ctc_cli_common_show_error_desc_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_show_task_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_set_task_cmd);
#if defined SDK_IN_USERMODE
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_get_task_cmd);
#endif
    install_element(CTC_INTERNAL_MODE, &ctc_cli_common_set_error_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_debug_show_cmd);

    /*interrupt debug cli*/
    install_element(CTC_SDK_MODE, &ctc_cli_common_interrupt_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_interrupt_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_interrupt_show_debug_cmd);

#if defined(SDK_IN_USERMODE)
    install_element(CTC_SDK_MODE, &ctc_cli_common_gateway_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_tftp_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_boot_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_rm_debug_cmd);
#endif

#ifdef SDK_UNIT_TEST
    install_element(CTC_SDK_MODE, &ctc_cli_common_unit_test_start_cmd);
#endif /* SDK_UNIT_TEST */


#ifndef HUMBER
    /*tmp disable for source master config end*/
    ctc_dma_cli_init();
#if (SDK_WORK_PLATFORM == 1)
  //  ctc_cmodel_cli_init(CTC_CMODEL_MODE);
#endif
#endif

    ctc_com_cli_init(CTC_SDK_MODE);
    ctc_dkit_cli_init(CTC_DKITS_MODE);
    ctc_feature_cli_install();

    if (sample_cli_init_callback != NULL)
    {
        sample_cli_init_callback(CTC_SDK_MODE);
    }
    ctc_sort_node();

    if (ctc_cli_exec_cb != NULL)
    {
        ctc_cli_exec_cb();
    }

    return 0;
}

int32 ctc_cli_read(int32 ctc_shell_mode)
{
    g_ctc_vti->printf = ctc_master_printf;
    g_ctc_vti->quit   = ctc_master_quit;
    g_ctc_vti->grep   = ctc_master_fprintf;

    if (1 == ctc_shell_mode)
    {
#if defined(SDK_IN_USERMODE)
        install_element(EXEC_MODE, &ctc_cli_common_exit_server_cmd);
        install_element(CTC_SDK_MODE, &ctc_cli_common_exit_server_cmd);
        install_element(CTC_CMODEL_MODE, &ctc_cli_common_exit_server_cmd);
        install_element(CTC_DEBUG_MODE, &ctc_cli_common_exit_server_cmd);
        install_element(CTC_INTERNAL_MODE, &ctc_cli_common_exit_server_cmd);
        install_element(CTC_APP_MODE, &ctc_cli_common_exit_server_cmd);
#endif

        return ctc_vty_socket();
    }
    return 0;
}
int32 ctc_cli_start(int32 ctc_shell_mode)
{
    uint32  nbytes = 0;
    int8*   pread_buf = NULL;
    cli_end = 0;

    if (0 != ctc_shell_mode)
    {
        return 0;
    }

    pread_buf = sal_malloc(CTC_VTI_BUFSIZ);

    if (NULL == pread_buf)
    {
        return - 1;
    }

    /* 1. Open device & save termios config, set O_NONBLOCK */
    set_terminal_raw_mode(CTC_VTI_SHELL_MODE_DEFAULT);

    while (cli_end == 0)
    {
        /* 2. Read & call function ctc_vti_read_cmd */
        nbytes = ctc_vti_read(pread_buf, CTC_VTI_BUFSIZ, CTC_VTI_SHELL_MODE_DEFAULT);
        ctc_vti_cmd_input(pread_buf, nbytes);
    }

    /* 3. Close device and restore */
    restore_terminal_mode(CTC_VTI_SHELL_MODE_DEFAULT);
    sal_free(pread_buf);

    return 0;
}
int32
ctc_master_cli(int32 ctc_shell_mode)
{
    ctc_cli_init();
    return 0;
}


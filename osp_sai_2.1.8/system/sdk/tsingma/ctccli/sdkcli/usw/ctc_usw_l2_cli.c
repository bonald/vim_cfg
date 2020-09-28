/**
 @file ctc_greatbel_l2_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_usw_l2_cli.h"
#include "ctc_usw_l2.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_l2.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#define CTC_L2_FDB_QUERY_ENTRY_NUM_DETAIL  100

extern int32 sys_usw_l2_show_status(uint8 lchip);
extern int32 sys_usw_l2_dump_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);
extern int32 sys_usw_l2_dump_l2mc_entry(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr, bool detail);
extern int32 sys_usw_l2_flush_fdb(uint8 lchip, ctc_l2_flush_fdb_t* pf);
extern int32 sys_usw_l2_dump_all_default_entry(uint8 lchip);
extern int32 sys_usw_l2_set_hw_learn(uint8 lchip, uint8 hw_en);
extern int32 sys_usw_l2_dump_fdb_entry_detail(uint8 lchip, ctc_l2_fdb_query_t* pq, uint16 entry_cnt, uint8 need_check, void* p_file);
extern int32 sys_usw_l2_dump_l2master(uint8 lchip);
extern int32 sys_usw_l2_set_static_mac_limit(uint8 lchip, uint8 static_limit_en);
extern int32 sys_usw_learning_aging_set_hw_sync(uint8 lchip, uint8 b_sync);
extern int32 sys_usw_aging_get_aging_ptr(uint8 lchip, uint8 domain_type, uint32 key_index, uint32* aging_ptr);
extern int32 sys_usw_aging_get_key_index(uint8 lchip, uint32 aging_ptr, uint8* domain_type, uint32* key_index);
extern int32 sys_usw_l2_fdb_set_trie_sort(uint8 lchip, uint8 enable);
extern int32 sys_usw_fdb_sort_set_dump_task_time(uint8 lchip, uint32 time);
extern int32 _sys_usw_l2_fdb_init(uint8 lchip, void* l2_fdb_global_cfg, uint8 vp_alloc_en);
sal_task_t* ctc_l2_write_fdb_detail_to_file = NULL;
void
ctc_write_fdb_detail_to_file(void* user_param)
{
    int32 ret = 0;
    sal_file_t fp = NULL;
    static ctc_l2_write_info_para_t write_entry_para;
    ctc_l2_fdb_query_t* query;
    uint16 entry_num = 0;
    uint8 lchip = 0;

    sal_memset(&write_entry_para, 0, sizeof(ctc_l2_write_info_para_t));

    sal_memcpy(&write_entry_para, user_param, sizeof(ctc_l2_write_info_para_t));
    mem_free(user_param);

    query = (ctc_l2_fdb_query_t*)write_entry_para.pdata;

    entry_num = (query->query_hw) ? CTC_L2_FDB_QUERY_ENTRY_NUM_DETAIL : 10;

    fp = sal_fopen((char*)write_entry_para.file, "w+");
    if (NULL == fp)
    {
        mem_free(write_entry_para.pdata);
        return;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_fdb_entry_detail(lchip, query, entry_num, 1, (void*)fp);
    if (ret < 0)
    {
        mem_free(write_entry_para.pdata);
        sal_fclose(fp);
        return;
    }

    sal_fclose(fp);
    mem_free(write_entry_para.pdata);
}

CTC_CLI(ctc_cli_usw_l2_show_fdb_status,
        ctc_cli_usw_l2_show_fdb_status_cmd,
        "show l2 fdb status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "FDB Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_l2_show_dflt,
        ctc_cli_usw_l2_show_dflt_cmd,
        "show l2 vlan-default-entry fid FID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "Vlan default entry",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint16 fid = 0;
    ctc_l2dflt_addr_t l2dflt_addr;
    uint8 index = 0;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    CTC_CLI_GET_UINT16_RANGE("FID", fid, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    l2dflt_addr.fid = fid;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_default_entry(lchip, &l2dflt_addr);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l2_show_dflt_all,
        ctc_cli_usw_l2_show_dflt_all_cmd,
        "show l2 vlan-default-entry all (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "Vlan default entry",
        "All",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_all_default_entry(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l2_show_mcast,
        ctc_cli_usw_l2_show_mcast_cmd,
        "show l2 mcast mac MAC fid FID (detail|) (lchip LCHIP|) ",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "Mcast entry",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_FID_ID_DESC,
        "Display detail information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    ctc_l2_mcast_addr_t l2mc_addr;
    uint8 index = 0;
    bool detail = FALSE;

    /*1) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[0]);

    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2mc_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

  	index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("detail");
    if (0xFF != index)
    {
         detail = TRUE;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_l2mc_entry(lchip, &l2mc_addr, detail);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_usw_l2_show_fdb_detail,
        ctc_cli_usw_l2_show_fdb_detail_cmd,
        "show l2 fdb detail by (mac MAC|mac-fid MAC FID|port GPORT_ID|fid FID|port-fid GPORT_ID FID|logic-port LOGIC_PORT_ID|all ) ((static|dynamic|local-dynamic|mcast|conflict|all)|)(hw |)(entry-file FILE_NAME|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "FDB detail information",
        "Query condition",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "MAC+Fid",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+Fid",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Logic port",
        "Reference to global config",
        "All condition",
        "Static fdb table",
        "All dynamic fdb table",
        "Local chip dynamic fdb table",
        "Mcast entry",
        "Conflict fdb table",
        "Static and dynamic",
        "Dump by Hardware",
        "File store fdb entry",
        "File name",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_l2_fdb_query_t query;
    ctc_l2_write_info_para_t* write_entry_para = NULL;
    uint8 is_hw = 0;
    uint16 entry_cnt = 0;
    char buffer[SAL_TASK_MAX_NAME_LEN] = {0};

    sal_memset(&query, 0, sizeof(ctc_l2_fdb_query_t));

    query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    query.query_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */
     /*index = ctc_cli_get_prefix_item(&argv[0], 1, "mac", 3);*/
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;

        if (sal_sscanf(argv[index + 1], "%4hx.%4hx.%4hx", (unsigned short*)&query.mac[0],
                       (unsigned short*)&query.mac[2], (unsigned short*)&query.mac[4]) != 3)
        {
            ctc_cli_out("%% Unable to translate mac address %s\n", argv[index + 1]);
            return CLI_ERROR;
        }

        /* Convert to network order */
        *(unsigned short*)&query.mac[0] = sal_htons(*(unsigned short*)&query.mac[0]);
        *(unsigned short*)&query.mac[2] = sal_htons(*(unsigned short*)&query.mac[2]);
        *(unsigned short*)&query.mac[4] = sal_htons(*(unsigned short*)&query.mac[4]);
    }

	/* parse MAC+FID address */
	index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
	if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;

        if (sal_sscanf(argv[index + 1], "%4hx.%4hx.%4hx", (unsigned short*)&query.mac[0],
                       (unsigned short*)&query.mac[2], (unsigned short*)&query.mac[4]) != 3)
        {
            ctc_cli_out("%% Unable to translate mac address %s\n", argv[index + 1]);
            return CLI_ERROR;
        }

        /* Convert to network order */
        *(unsigned short*)&query.mac[0] = sal_htons(*(unsigned short*)&query.mac[0]);
        *(unsigned short*)&query.mac[2] = sal_htons(*(unsigned short*)&query.mac[2]);
        *(unsigned short*)&query.mac[4] = sal_htons(*(unsigned short*)&query.mac[4]);

        CTC_CLI_GET_UINT16_RANGE("forwarding id", query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*2) parse PORT address */
    index = ctc_cli_get_prefix_item(&argv[0], 1, "port", 4);
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        CTC_CLI_GET_UINT16_RANGE("gport", query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse fid address*/
    index = ctc_cli_get_prefix_item(&argv[0], 1, "fid", 3);
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        CTC_CLI_GET_UINT16_RANGE("forwarding id", query.fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*6)logic_port*/
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        query.use_logic_port = 1;
        CTC_CLI_GET_UINT16_RANGE("logic-port", query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("conflict");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_CONFLICT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-file");
    if (index != 0xFF)
    {
        /* free in ctc_write_fdb_detail_to_file*/
        MALLOC_ZERO(MEM_CLI_MODULE, write_entry_para, sizeof(ctc_l2_write_info_para_t));
        sal_strcpy((char*)&write_entry_para->file, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw");
    if (index != 0xFF)
    {
        is_hw = 1;
        query.query_hw = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (write_entry_para && *write_entry_para->file)
    {
        /* free memery will be done in ctc_write_fdb_detail_to_file */
        write_entry_para->pdata = (ctc_l2_fdb_query_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_l2_fdb_query_t));
        if (NULL == write_entry_para->pdata)
        {
            mem_free(write_entry_para);
            return CLI_ERROR;
        }

        sal_memcpy(write_entry_para->pdata, &query, sizeof(ctc_l2_fdb_query_t));
        sal_sprintf(buffer, "ctcFdbWFl-%d", lchip);
        if (0 != sal_task_create(&ctc_l2_write_fdb_detail_to_file,
                                 buffer,
                                 0, SAL_TASK_PRIO_DEF, ctc_write_fdb_detail_to_file, write_entry_para))
        {
            mem_free(write_entry_para->pdata);
            mem_free(write_entry_para);
            sal_task_destroy(ctc_l2_write_fdb_detail_to_file);
            return CLI_ERROR;
        }
    }
    else
    {
        entry_cnt = is_hw?CTC_L2_FDB_QUERY_ENTRY_NUM_DETAIL:10;

        lchip = g_ctcs_api_en?g_api_lchip:lchip;
        ret = sys_usw_l2_dump_fdb_entry_detail(lchip, &query, entry_cnt, 1, NULL);
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            if (write_entry_para)
            {
                mem_free(write_entry_para);
            }
            return CLI_ERROR;
        }
    }

    return ret;

}



CTC_CLI(ctc_cli_usw_l2_show_master,
        ctc_cli_usw_l2_show_master_cmd,
        "show l2 fdb-master (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "master data",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_l2master(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}
CTC_CLI(ctc_cli_usw_l2_flush_fdb,
        ctc_cli_usw_l2_flush_fdb_cmd,
        "l2 fdb flush by (port GPORT_ID|fid FID|port-fid GPORT_ID FID |mac-fid MAC FID|all) (is-logic-port|) (static|dynamic|all) hw (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Flush fdb",
        "Query condition",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+FID",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Mac+Fid",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        "All condition",
        "Is logic port",
        "Static fdb table",
        "All dynamic fdb table",
        "Static and dynamic",
        "Hw mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    uint8 index   = 0;
    int32    ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    ctc_l2_flush_fdb_t Flush;

    sal_memset(&Flush, 0, sizeof(ctc_l2_flush_fdb_t));

    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;
        index++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Flush.mac, argv[index]);

    }

    /*2) parse PORT address */

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        index++;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3)parse fid */

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        index++;
        CTC_CLI_GET_UINT16_RANGE("vlan id",  Flush.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", Flush.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        CTC_CLI_GET_MAC_ADDRESS("mac", Flush.mac, argv[index+1]);
        CTC_CLI_GET_UINT16_RANGE("fid", Flush.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }

   /*6) parse is-logic-port */
    index = CTC_CLI_GET_ARGC_INDEX("is-logic-port");
    if (index != 0xFF)
    {
        Flush.use_logic_port = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_flush_fdb(lchip, &Flush);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_l2_show_fdb_detail_hw,
        ctc_cli_usw_l2_show_fdb_detail_hw_cmd,
        "show l2 fdb detail by (mac MAC|mac-fid MAC FID|port GPORT_ID|fid FID|port-fid GPORT_ID FID|all ) ((static|dynamic|local-dynamic|pending|mcast|conflict|all)|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "FDB detail information",
        "Query condition",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "MAC+Fid",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+Fid",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "All condition",
        "Static fdb table",
        "All dynamic fdb table",
        "Local chip dynamic fdb table",
        "Pending fdb table",
        "Conflict fdb table",
        "Static and dynamic",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_l2_fdb_query_t query;
    uint16 entry_cnt = 0;

    sal_memset(&query, 0, sizeof(ctc_l2_fdb_query_t));

    query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    query.query_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */
     /*index = ctc_cli_get_prefix_item(&argv[0], 1, "mac", 3);*/
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;

        if (sal_sscanf(argv[index + 1], "%4hx.%4hx.%4hx", (unsigned short*)&query.mac[0],
                       (unsigned short*)&query.mac[2], (unsigned short*)&query.mac[4]) != 3)
        {
            ctc_cli_out("%% Unable to translate mac address %s\n", argv[index + 1]);
            return CLI_ERROR;
        }

        /* Convert to network order */
        *(unsigned short*)&query.mac[0] = sal_htons(*(unsigned short*)&query.mac[0]);
        *(unsigned short*)&query.mac[2] = sal_htons(*(unsigned short*)&query.mac[2]);
        *(unsigned short*)&query.mac[4] = sal_htons(*(unsigned short*)&query.mac[4]);
    }

	/* parse MAC+FID address */
	index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
	if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;

        if (sal_sscanf(argv[index + 1], "%4hx.%4hx.%4hx", (unsigned short*)&query.mac[0],
                       (unsigned short*)&query.mac[2], (unsigned short*)&query.mac[4]) != 3)
        {
            ctc_cli_out("%% Unable to translate mac address %s\n", argv[index + 1]);
            return CLI_ERROR;
        }

        /* Convert to network order */
        *(unsigned short*)&query.mac[0] = sal_htons(*(unsigned short*)&query.mac[0]);
        *(unsigned short*)&query.mac[2] = sal_htons(*(unsigned short*)&query.mac[2]);
        *(unsigned short*)&query.mac[4] = sal_htons(*(unsigned short*)&query.mac[4]);

        CTC_CLI_GET_UINT16_RANGE("forwarding id", query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*2) parse PORT address */
    index = ctc_cli_get_prefix_item(&argv[0], 1, "port", 4);
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        CTC_CLI_GET_UINT16_RANGE("gport", query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse fid address*/
    index = ctc_cli_get_prefix_item(&argv[0], 1, "fid", 3);
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        CTC_CLI_GET_UINT16_RANGE("forwarding id", query.fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pending");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_PENDING;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("conflict");
    if (index != 0xFF)
    {
        query.query_flag = CTC_L2_FDB_ENTRY_CONFLICT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    query.query_hw = 1;
    entry_cnt = 100;


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_dump_fdb_entry_detail(lchip, &query, entry_cnt, 0, NULL);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_usw_l2_fdb_hw_learn,
        ctc_cli_usw_l2_fdb_hw_learn_cmd,
        "l2 fdb hw-learn (enable | disable) (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Force MAC Learning to Hw-learning",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 hw_en = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if( 0xFF != index )
    {
        hw_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2_set_hw_learn(lchip, hw_en);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_usw_l2_fdb_vp_alloc_en,
        ctc_cli_usw_l2_fdb_vp_alloc_en_cmd,
        "l2 fdb vp-alloc enable (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "alloc ad resource for vitrul-port when need",
        CTC_CLI_ENABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 vp_alloc_en = 1;
    uint8 index = 0xFF;
    ctc_l2_fdb_global_cfg_t l2_fdb_global_cfg;
    sal_memset(&l2_fdb_global_cfg, 0, sizeof(ctc_l2_fdb_global_cfg_t));
    l2_fdb_global_cfg.default_entry_rsv_num  = 5 * 1024; /*5k*/
    l2_fdb_global_cfg.hw_learn_en            = 0;
    l2_fdb_global_cfg.logic_port_num         = 1 * 1024;
    l2_fdb_global_cfg.stp_mode               = 2;    /* 128 instance */

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_l2_fdb_deinit(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ret = _sys_usw_l2_fdb_init(lchip, &l2_fdb_global_cfg, vp_alloc_en);
    if (ret)
    {
        ctc_cli_out("ctc_l2_fdb_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    return CLI_SUCCESS;

}



#define __USW_MERGE__

CTC_CLI(ctc_cli_l2_add_addr_usw,
        ctc_cli_l2_add_addr_usw_cmd,
        "l2 fdb add mac MAC fid FID {port GPORT_ID | nexthop NHPID} \
       (((vpls src-port GPORT_ID)| virtual-port VIRTUAL_PORT)|)(assign-port GPORT_ID|)\
       ({static | remote-dynamic | discard | src-discard | src-discard-tocpu | copy-tocpu | bind-port | raw-pkt-elog-cpu | service-queue | untagged | protocol-entry | system-mac | ptp-entry | self-address | white-list-entry|aging-disable}|) \
       (cid CID)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Nexthop",
        CTC_CLI_NH_ID_STR,
        "Vpls",
        "Src port after vpls loopback",
        CTC_CLI_GPORT_DESC,
        "Virtual port, use for vpls/pbb/qinq service queue",
        "Virtual port value, range 0-0x1FFF, and 0x1FFF is invalid",
        "Assign output gport",
        CTC_CLI_GPORT_ID_DESC,
        "Static fdb table",
        "Remote chip learing dynamic fdb",
        "Destination discard fdb table",
        "Source discard fdb table",
        "Source discard and send to cpu fdb table",
        "Forward and copy to cpu fdb table",
        "Port bind fdb table",
        "Send raw packet to cpu,using for terminating protocol pakcet(LBM/DMM/LMM) to cpu",
        "Packet will enqueue by service id",
        "Output packet will has no vlan tag",
        "Protocol entry enable exception flag",
        "Indicate the entry is system mac, it can't be deleted by flush api, using for MAC DA",
        "PTP mac address for ptp process",
        "Indicate mac address is switch's",
        "Indicate the entry is white list entry, it won't do aging, and will do learning",
        "Indicate mac address should not be aged",
        "Category Id",
        "Category Id Value")
{
    int32 ret = CLI_SUCCESS;
    uint32 nexthpid = 0;
    ctc_l2_addr_t l2_addr;
    uint8 index = 0xFF;
    bool nexthop_config = FALSE;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

    /*2) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);

    /*3) parse FID */
    CTC_CLI_GET_UINT16_RANGE("FID", l2_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    /*4) parse port/nexthop */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("src-port", l2_addr.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
        nexthop_config = TRUE;
        CTC_CLI_GET_UINT32_RANGE("next-hop-id", nexthpid, argv[index+1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpls");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", l2_addr.gport, argv[index+2], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("virtual-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", l2_addr.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", l2_addr.assign_port, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        l2_addr.flag |= CTC_L2_FLAG_ASSIGN_OUTPUT_PORT;
    }

    /*5) parse static|discard|src-discard|src-discard-tocpu
    |copy-tocpu|fwd-tocpu|storm-ctl|protocol|bind-port} */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_IS_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-dynamic");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_REMOTE_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard-tocpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("copy-tocpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_COPY_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bind-port");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_BIND_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("protocol-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("system-mac");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SYSTEM_RSV;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("self-address");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("white-list-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_WHITE_LIST_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aging-disable");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_AGING_DISABLE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cid", l2_addr.cid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }




    if (nexthop_config)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_l2_add_fdb_with_nexthop(g_api_lchip, &l2_addr, nexthpid);
        }
        else
        {
            ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, nexthpid);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret =  ctcs_l2_add_fdb(g_api_lchip, &l2_addr);
        }
        else
        {
            ret =  ctc_l2_add_fdb(&l2_addr);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_add_default_entry_usw,
        ctc_cli_l2_add_default_entry_usw_cmd,
        "l2 fdb add vlan-default-entry (fid FID) (port GPORT_ID|) (group GROUP_ID)(use-logic-port|)(unknown-ucast-drop|)(unknown-mcast-drop|)(proto-to-cpu|) (cid CID)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Use logic port check",
        "Unknown ucast mac to drop",
        "Unknown mcast mac to drop",
        "Protocol exception to cpu",
        "Category Id",
        "Category Id Value")
{
    uint16 fid = 0;
    int32    ret  = CLI_SUCCESS;
    uint32 l2mc_grp_id = 0;
    uint8 index = 0;

    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    /*2) parse*/

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("group id", l2mc_grp_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        l2dflt_addr.l2mc_grp_id = l2mc_grp_id;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-ucast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-mcast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("proto-to-cpu");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cid", l2dflt_addr.cid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_add_default_entry(g_api_lchip, &l2dflt_addr);
    }
    else
    {
        ret = ctc_l2_add_default_entry(&l2dflt_addr);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_usw_l2_fdb_misc,
        ctc_cli_usw_l2_fdb_misc_cmd,
        "l2 fdb (static-mac-limit | hw-learn-sync) (enable | disable) (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Static mac limit",
        "Hardware Mac learning sync",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 static_fdb_limit = 0;
    uint8 b_enable = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("static-mac-limit");
    if( 0xFF != index )
    {
        static_fdb_limit = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if( 0xFF != index )
    {
        b_enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if (static_fdb_limit)
    {
        ret = sys_usw_l2_set_static_mac_limit(lchip, b_enable);
    }
    else
    {
        ret = sys_usw_learning_aging_set_hw_sync(lchip, b_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_usw_l2_fdb_trie_sort,
        ctc_cli_usw_l2_fdb_trie_sort_cmd,
        "l2 fdb trie-sort (enable |disable) (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "trie-sort",
        "enable",
        "disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint8 enable = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    sys_usw_l2_fdb_set_trie_sort(lchip, enable);

    return CLI_SUCCESS;

}
CTC_CLI(ctc_cli_usw_l2_fdb_set_dump_task_time,
        ctc_cli_usw_l2_fdb_set_dump_task_time_cmd,
        "l2 fdb dump-time TIME (lchip LCHIP|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "dump entries time",
        "granularity: 10s, minimum 10s",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint32 time = 0;
    uint8 index = 0xFF;

    CTC_CLI_GET_UINT32("dump-time", time, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    sys_usw_fdb_sort_set_dump_task_time(lchip, time);

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_l2_flush_fdb_usw,
        ctc_cli_l2_flush_fdb_usw_cmd,
        "l2 fdb flush by all pending ",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Flush fdb",
        "Query condition",
        "All condition",
        "Pending fdb"
        )
{
    int32    ret  = CLI_SUCCESS;
    ctc_l2_flush_fdb_t Flush;

    sal_memset(&Flush, 0, sizeof(ctc_l2_flush_fdb_t));

    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_PENDING;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_flush_fdb(g_api_lchip, &Flush);
    }
    else
    {
        ret = ctc_l2_flush_fdb(&Flush);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_set_fid_property_usw,
    ctc_cli_l2_set_fid_property_usw_cmd,
    "l2 fid FID (unknown-ucast-copy-tocpu|bcast-copy-tocpu) value VALUE",
    CTC_CLI_L2_M_STR,
    "Fid",
    "Fid value",
    "Unknown ucast packet copy to cpu",
    "Bcast packet copy to cpu",
    "Value",
    "Value of the property")
{
    int32 ret;
    uint16 fid;
    uint32 value = 0;
    uint8 type = 0;

    CTC_CLI_GET_UINT16("fid id", fid, argv[0]);

    if (CLI_CLI_STR_EQUAL("unknown-ucast-copy-tocpu", 1))
    {
        type = CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU;
    }
    else if (CLI_CLI_STR_EQUAL("bcast-copy-tocpu", 1))
    {
        type = CTC_L2_FID_PROP_BCAST_COPY_TO_CPU;
    }

    CTC_CLI_GET_UINT32_RANGE("fid value", value, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_set_fid_property(g_api_lchip, fid, type, value);
    }
    else
    {
        ret = ctc_l2_set_fid_property(fid, type, value);
    }

     if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_get_fid_property_usw,
        ctc_cli_l2_get_fid_property_usw_cmd,
        "show l2 fid FID {unknown-ucast-copy-tocpu|bcast-copy-tocpu}",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "Fid",
        "Fid value",
        "Unknown ucast packet copy to cpu",
        "Bcast packet copy to cpu")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint16 fid;
    uint32 val = 0;

    CTC_CLI_GET_UINT16("fid id", fid, argv[0]);


    index = CTC_CLI_GET_ARGC_INDEX("unknown-ucast-copy-tocpu");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("unknown ucast copy to cpu enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bcast-copy-tocpu");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_BCAST_COPY_TO_CPU, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_BCAST_COPY_TO_CPU, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("bcast copy to cpu enable             :%d\n", val);
    }

    return ret;
}

CTC_CLI(ctc_cli_learning_aging_ptr2index_usw,
        ctc_cli_learning_aging_ptr2index_usw_cmd,
        "show learning-aging ptr2index AGING_PTR (lchip LCHIP |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Aging ptr to index",
        "AGING_PTR",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 aging_ptr;
    uint8  domain_type;
    uint32 key_index = 0;
    char*  str_desc[] =
    {
        "tcam",
        "mac-hash",
        "flow-hash",
        "ip-hash"
    };

    CTC_CLI_GET_UINT32("aging-ptr", aging_ptr, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_aging_get_key_index(lchip, aging_ptr, &domain_type, &key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-15s %-15s\n", "domain-type:", str_desc[domain_type]);
    ctc_cli_out("%-15s 0x%x\n", "aging-index:", key_index);

    return ret;
}

CTC_CLI(ctc_cli_learning_aging_index2ptr_usw,
        ctc_cli_learning_aging_index2ptr_usw_cmd,
        "show learning-aging index2ptr INDEX (lchip LCHIP |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Aging index to ptr",
        "INDEX",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 aging_ptr;
    uint8  domain_type = 1;
    uint32 key_index = 0;

    CTC_CLI_GET_UINT32("key-index", key_index, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_aging_get_aging_ptr(lchip, domain_type, key_index, &aging_ptr);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-15s 0x%x\n", "aging-ptr:", aging_ptr);

    return ret;
}

int32
ctc_usw_l2_cli_init(void)
{

    install_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_fdb_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_dflt_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_dflt_all_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_mcast_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_fdb_detail_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_flush_fdb_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_show_master_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_show_fdb_detail_hw_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_hw_learn_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_vp_alloc_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_default_entry_usw_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_addr_usw_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_misc_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_trie_sort_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_set_dump_task_time_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_flush_fdb_usw_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_set_fid_property_usw_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_get_fid_property_usw_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_learning_aging_ptr2index_usw_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_learning_aging_index2ptr_usw_cmd);
    return 0;
}

int32
ctc_usw_l2_cli_deinit(void)
{

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_fdb_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_dflt_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_dflt_all_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_mcast_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l2_show_fdb_detail_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_flush_fdb_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_show_master_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_show_fdb_detail_hw_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_hw_learn_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_vp_alloc_en_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_l2_add_default_entry_usw_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_l2_add_addr_usw_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_misc_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_trie_sort_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_l2_fdb_set_dump_task_time_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_l2_flush_fdb_usw_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_l2_set_fid_property_usw_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_l2_get_fid_property_usw_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_learning_aging_ptr2index_usw_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_learning_aging_index2ptr_usw_cmd);
    return 0;
}


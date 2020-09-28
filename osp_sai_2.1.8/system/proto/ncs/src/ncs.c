/* ncs - NETCONF configure service
 * 
 */

#include "proto.h"
#include "ctc_task.h"
#include "ncs.h"
#include "ncs_api.h"
#include "gen/cdb_dest_pm_priv.h"

pm_id_t
ncs_get_show_resource_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
{
    cfg_parse_info_t parse_info;
    char para_str[MAX_CMD_STR_LEN];
    int32 value = 0;
    int32 rc = PM_E_NONE;
    
    sal_memcpy(para_str, (char*)p_msg->data, p_msg->data_len);
    para_str[p_msg->data_len] = CMD_ZERO_CHAR;
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        return PM_ID_INVALID;
    }
    if (1 > parse_info.argc)
    {
        return PM_ID_INVALID;
    }

    if (!sal_strcmp("fdb", parse_info.argv[0]))
    {
        return PM_ID_SWITCH;
    }
    else if (!sal_strcmp("fib", parse_info.argv[0]))
    {
        return PM_ID_ROUTED;
    }
    else if (!sal_strcmp("tap", parse_info.argv[0]))
    {
        return PM_ID_SWITCH;
    }
    else
    {
        return PM_ID_INVALID;
    }
    
    return value;
}

pm_id_t
ncs_get_clock_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
{
    cfg_parse_info_t parse_info;
    char para_str[MAX_CMD_STR_LEN];
    int32 value = 0;
    int32 rc = PM_E_NONE;
    
    sal_memcpy(para_str, (char*)p_msg->data, p_msg->data_len);
    para_str[p_msg->data_len] = CMD_ZERO_CHAR;
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        return PM_ID_INVALID;
    }
    if (1 > parse_info.argc)
    {
        return PM_ID_CHSM;
    }

    if (0 == sal_strcmp("switch", parse_info.argv[0]))
    {
        return PM_ID_SWITCH;
    }
    else
    {
        return PM_ID_CHSM;
    }
    
    return value;
}

pm_id_t
ncs_get_show_common_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
{
    cfg_parse_info_t parse_info;
    char para_str[MAX_CMD_STR_LEN];
    int32 value = 0;
    int32 rc = PM_E_NONE;
    
    sal_memcpy(para_str, (char*)p_msg->data, p_msg->data_len);
    para_str[p_msg->data_len] = CMD_ZERO_CHAR;
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        return PM_ID_INVALID;
    }
    if (2 > parse_info.argc)
    {
        return PM_ID_INVALID;
    }
    
    value = cdb_enum_str2val(g_ipc_dest_desc, PM_ID_MAX, parse_info.argv[0]);
    if (value < 0)
    {
        return PM_ID_INVALID;
    }
    
    return value;
}

pm_id_t
ncs_get_dest_pm(cdb_node_t* p_node)
{
    pm_id_t pm_id = PM_ID_INVALID;
    
    if (CDB_NODE_TBL == p_node->type)
    {
        if (p_node->id >= TBL_MAX)
        {
            return PM_ID_INVALID;
        }

        pm_id = g_cdb_tbl_dest_pm[p_node->id];
        return pm_id;        
    }

    if (CDB_NODE_ACT == p_node->type)
    {
        if (p_node->id >= ACT_MAX)
        {
            return PM_ID_INVALID;
        }
        
        pm_id = g_cdb_act_dest_pm[p_node->id];
        if (pm_id == PM_ID_MAX)
        {
#if 0
            if (ACT_SHOW_RESOURCE == p_node->id)
            {
                pm_id = ncs_get_show_resource_dest_pm(p_node, p_msg);
            }
            else if (ACT_CLOCK == p_node->id)
            {
                pm_id = ncs_get_clock_dest_pm(p_node, p_msg);
            }
            else
            {
                pm_id = ncs_get_show_common_dest_pm(p_node, p_msg);
            }
#endif
        }
        return pm_id;
    }
    
    return PM_ID_INVALID;
}

int32
ncs_sock_init(lib_master_t *master)
{
    int32 sock = INVALID_SOCK_ID;
    master->cfg_sock = INVALID_SOCK_ID;
    master->data_sock = INVALID_SOCK_ID;
    master->sub_cdb_sock = INVALID_SOCK_ID;
    master->sub_tbl_sock = INVALID_SOCK_ID;
    master->trap_sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;

    /* NCS config socket */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "NcsSrvCfg");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_sock_set_pmid(sock, master->pm_id);
    ctc_sock_bind(sock, SOCK_ADDR_NCS);
    master->nc_sock = sock;


    sock = ctc_socket(CTC_SOCK_IPC, 0, "CcsCltData");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_CCS, FALSE);
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    master->cfg_sock = sock;

    /* client to CDS */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CdsCltData");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_CDS_DATA, FALSE);
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    master->data_sock = sock;

    ipc_connect_bhm();

    return PM_E_NONE;
}

extern int32
cfg_lic_check_cdb_node(cdb_node_t* p_node);
extern int32
ncs_lic_init();

int32
ncs_sock_cfg_sync(cdb_cfg_t cfg, cdb_oper_t opt, uint16 type, uint16 id, char *cmd, uint32 cmd_len)
{
    lib_master_t *lib_master = lib_get_master();
    cdb_node_t* p_node = NULL;
    cfg_result_t* p_resp_rslt = NULL;
    ctc_msg_t resp_msg;
    pm_id_t pm_id = 0;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_node_by_id(type, id);
    if (NULL == p_node)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Invalid CDB node type %u, id %u\n", type, id);
        return PM_E_FAIL;
    }

    /* 2. check CDB */
    rc = cfg_lic_check_cdb_node(p_node);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    /* 3. get dest PM */
    pm_id = ncs_get_dest_pm(p_node);
    
    if (PM_ID_INVALID == pm_id)
    {
        return PM_E_FAIL;
    }
    sal_memset(&resp_msg, 0, sizeof(resp_msg));
    rc = ipc_ncs_to_pm_cfg_sync(lib_master->nc_sock, pm_id, type, id, cfg, opt, (uint8*)cmd, cmd_len, &resp_msg);
    if (resp_msg.data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)resp_msg.data;
        rc = p_resp_rslt->rc;
    }
    ctc_msg_free(&resp_msg);

    return rc;
}

int32
ncs_init()
{ 
    lib_master_t *lib_master = lib_init(PM_ID_NCS);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
    
    ncs_sock_init(lib_master);
#ifdef HAVE_NETCONF
    ncs_nc_init();
#endif /* !HAVE_NETCONF */
    ncs_lic_init();
    
    ctc_task_main();

    return PM_E_NONE;
}


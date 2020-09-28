
#include "proto.h"
#include "version.h"
#include "cdbclient.h"
#include "glb_tempfile_define.h"
#include "pb_cdb_inc.h"
#include "sdb_defines.h"

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define CFG_ERROR_STATISTICS(op, COUNT) \
{ \
    rc = (op); \
    if (rc < 0) \
    { \
        COUNT += 1; \
    } \
}

extern const char *ifname_media_prefix_long[GLB_IFNAME_MEDIA_MAX];
extern const char *ifname_media_prefix_short[GLB_IFNAME_MEDIA_MAX];

uint32 g_cdbclient_continue = 0;

static sys_load_state_t g_sys_load_state = SYS_LOAD_MAX;
static uint32 g_sys_load_wait_warning    = SYS_LOAD_WARNING_CLI_WAIT_MS;
static uint32 g_sys_load_wait_alert      = SYS_LOAD_ALERT_CLI_WAIT_MS;
static uint32 g_sys_load_get_count       = 0;
char *g_grep_str = NULL;



int32
cdbctl_set_grep_str(char *grep_str)
{
    g_grep_str = grep_str;
    
    return PM_E_NONE;
}
    
sys_load_state_t
cdbctl_get_sys_load_state()
{
    FILE *fp = NULL;
    char buf[CMD_BUF_256];
    uint32 state = 0;
    
    if (SYS_LOAD_MAX == g_sys_load_state || (0 == (g_sys_load_get_count % 10)))
    {
        fp = sal_fopen(SYS_LOAD_STATE_FILE, "r");
        if (NULL == fp)
        {
            g_sys_load_state = SYS_LOAD_NORMAL;
            return g_sys_load_state;
        }
        sal_fgets(buf, CMD_BUF_256, fp);
        sal_sscanf(buf, "%u", &state);
        sal_fclose(fp);
        g_sys_load_state = state;
    }
    else
    {
    }

    g_sys_load_get_count++;
    
    return g_sys_load_state;
}

int32
cdbctl_sys_load_wait(cdb_node_t* p_node)
{
    sys_load_state_t state = cdbctl_get_sys_load_state();

#ifdef _GLB_UML_SYSTEM_
    /* always return OK on UML */
    return PM_E_NONE;
#endif

    /* these commands not do delay check */
    if (CDB_NODE_ACT == p_node->type)
    {
        if (ACT_REBOOT == p_node->id)
        {
            return PM_E_NONE;
        }
        if (ACT_SEND_TRAP == p_node->id)
        {
            return PM_E_NONE;
        }
    }
    
    if (SYS_LOAD_NORMAL == state)
    {
    }
    else if (SYS_LOAD_WARNING == state)
    {
        sal_task_sleep(g_sys_load_wait_warning);
//        sal_printf("cdbctl wait %u ms\n", g_sys_load_wait_warning);
    }
    else if (SYS_LOAD_ALERT == state)
    {
        sal_task_sleep(g_sys_load_wait_alert);
//        sal_printf("cdbctl wait %u ms\n", g_sys_load_wait_alert);
    }
/* modified by liwh for bug 51819, 2019-09-10 */
#if 0    
    else if (SYS_LOAD_CRITICAL == state)
    {
        /* Modified by kcao for bug 43843, allow login when system is busying */
        /* Modified by yangl for bug 51263, allow ssh login when system is busying 2019-03-22*/
        if ((CDB_NODE_TBL == p_node->type && TBL_LOGIN == p_node->id)||(CDB_NODE_TBL == p_node->type && TBL_USER == p_node->id))
        {
        }
        
        else
        {
            return PM_E_FAIL;
        }
    }
#endif
/*liwh end */

    return PM_E_NONE;
}

static cdb_node_t*
_cdbclt_batch_cfg_get_node(cdb_node_t* p_bat_node)
{
    switch (p_bat_node->id)
    {
    case BAT_VLANRANGE:
        return cdb_get_tbl(TBL_VLAN);
        
    case BAT_INTERFACERANGE:
        return cdb_get_tbl(TBL_INTERFACE);
        
    case BAT_QOS_DOMAIN_RANGE:
        return cdb_get_tbl(TBL_QOS_DOMAIN);

    default:
        return NULL;
    }
}

int32
_cdbctl_show_grep_line(char *line, char *grep_str)
{
    char *left = line;
    char *start = NULL;
    char *end = NULL;
    int32 str_len = 0;

    if (NULL == grep_str)
    {
        sal_printf(line);
        return PM_E_NONE;
    }

    /* print key */
    start = sal_strstr(line, "key");
    if (start)
    {
        end = sal_strchr(start, CMD_DIVISION_CHAR);
        if (end)
        {
            end[0] = CMD_ZERO_CHAR;
        }
        sal_printf(start);
        left = end+1;
    }

    while (left && sal_strlen(left))
    {
        sal_printf("  ");
    
        /* print grep str */    
        start = sal_strstr(left, grep_str);
        if (start)
        {
            end = sal_strchr(start, CMD_DIVISION_CHAR);
            if (end)
            {
                end[0] = CMD_ZERO_CHAR;
                left = end+1;
            }
            else
            {
                left = end;
            }
            
            str_len = sal_strlen(start);
            if (str_len > 0)
            {
                if (start[str_len-1] == CMD_LINE_CHAR)
                {
                    start[str_len-1] = CMD_ZERO_CHAR;
                }
            }
            sal_printf(start);
        }
        else
        {
            break;
        }
    }

    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_get_from_file(char *file_name)
{
    FILE* fp = NULL;
    char  line[FILE_READ_LINE_LEN];

    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (g_grep_str)
        {
            _cdbctl_show_grep_line(line, g_grep_str);
        }
        else
        {
            sal_printf(line);
        }
    }
    sal_fclose(fp);
    sal_unlink(file_name);

    return PM_E_NONE;
}

static int32
_cdbctl_parse_tbl_lag_global(FILE *fp, void *p_tbl, uint32 tbl_size)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    tbl_lag_global_t *p_lag_glb = (tbl_lag_global_t*)p_tbl;
    int32 value = 0;
    int32 rc = PM_E_NONE;

    SAL_ASSERT(sizeof(tbl_lag_global_t) == tbl_size);

    sal_memset(p_lag_glb, 0, sizeof(tbl_size));
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        /* get system_priority */
        cfg_get_parse(line, &info, 0);
        value = cdb_int_str2val(info.value[TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY], &rc);
        if (rc < 0)
        {
            return rc;
        }
        p_lag_glb->system_priority = value;
        
        /* get system_id */
        rc = cdb_mac_addr_str2val(p_lag_glb->system_id, info.value[TBL_LAG_GLOBAL_FLD_SYSTEM_ID]);
        if (rc < 0)
        {
            return rc;
        }

        /* get mlag_system_priority */
        value = cdb_int_str2val(info.value[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY], &rc);
        if (rc < 0)
        {
            return rc;
        }
        p_lag_glb->mlag_system_priority = value;
        
        /* get mlag_system_id */
        rc = cdb_mac_addr_str2val(p_lag_glb->mlag_system_id, info.value[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID]);
        if (rc < 0)
        {
            return rc;
        }
        
        /* get load_balance */
        cfg_get_parse(line, &info, 0);
        value = cdb_int_str2val(info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE], &rc);
        if (rc < 0)
        {
            return rc;
        }
        p_lag_glb->load_balance = value;
    }
    else
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

static int32
_cdbctl_get_tbl_global_dispatch(cdb_node_t* p_node, char *file_name, void *p_tbl, uint32 tbl_size)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }
    if (CDB_NODE_TBL == p_node->type)
    {
        switch (p_node->id)
        {
        case TBL_LAG_GLOBAL:
            rc = _cdbctl_parse_tbl_lag_global(fp, p_tbl, tbl_size);
            break;

        default:
            rc = PM_E_FAIL;
        }
    }
        
    sal_fclose(fp);
    sal_unlink(file_name);

    return rc;
}

int32
cdbclt_get_tbl_data(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, void *p_tbl, uint32 tbl_size)
{
    cdb_node_t* p_node = NULL;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    rc = ipc_ctl_to_ccs_cfg(CDB_OPER_GET, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Get table %s data failed, rc is %d\n", p_node->name, rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        rc = _cdbctl_get_tbl_global_dispatch(p_node, p_resp_show->path, p_tbl, tbl_size);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        rc = PM_E_FAIL;
    }
    ctc_msg_free(p_resp_msg);
    
    return rc;
}

int32
cdbclt_get_tbl(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, char *path)
{
    cdb_node_t* p_node = NULL;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    rc = ipc_ctl_to_ccs_cfg(CDB_OPER_GET, p_node, NULL, 0, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Get table %s failed, rc is %d\n", p_node->name, rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        sal_strncpy(path, p_resp_show->path, MAX_CMD_SHOW_FNAME_LEN);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        rc = PM_E_FAIL;
    }
    ctc_msg_free(p_resp_msg);
    
    return rc;
}

int32
cdbclt_get_tbl_line(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, char *line, uint32 line_len)
{
    FILE *fp = NULL;
    cdb_node_t* p_node = NULL;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    rc = ipc_ctl_to_ccs_cfg(CDB_OPER_GET, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Get table %s line failed, rc is %d\n", p_node->name, rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        fp = sal_fopen(p_resp_show->path, "r");
        if (!fp)
        {
            return PM_E_FAIL;
        }

        if (NULL != sal_fgets(line, line_len, fp))
        {
            rc = PM_E_NONE;
        }
        else
        {
            rc = PM_E_FAIL;
        }

        sal_fclose(fp);
        sal_unlink(p_resp_show->path);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        rc = PM_E_FAIL;
    }
    ctc_msg_free(p_resp_msg);
    
    return rc;
}

int32
cdbclt_process_cmd_build_cfg(cdb_node_t* p_node, char *p_data, uint32 data_len, cfg_parse_info_t *p_info,
        uint32 build_mode, char *path)
{
    char cmd[CMD_BUF_256];
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    uint32 new_data_len = 0;
    int32 rc = PM_E_NONE;

    sal_memset(cmd, 0, sizeof(cmd));
    sal_strncpy(cmd, path, CMD_BUF_256);
    if (data_len)
    {
        sal_strncat(cmd, "/", CMD_BUF_256);
        sal_strncat(cmd, p_data, CMD_BUF_256);
    }
    new_data_len = sal_strlen(cmd) + 1;

#ifndef TAPPRODUCT
    if (sal_strcmp(path, "config-running_config_for_diff"))
    {
        sal_printf("Building configuration...\n");
        if (!build_mode && CDB_NODE_DIR == p_node->type && DIR_CDB == p_node->id)
        {
            sal_printf("version %s\n", OSP_VERSION);
            sal_printf("!\n");
        }
    }
#endif

    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, cmd, new_data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        if (PM_E_NONE != p_resp_rslt->rc)
        {
            rc = p_resp_rslt->rc;
            sal_printf("%% %s \n", p_resp_rslt->desc);
        }
        ctc_msg_free(p_resp_msg);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        if (build_mode)
        {
        }
        else
        {
            cfg_cmd_show_file(p_resp_show->path);
        }
        ctc_msg_free(p_resp_msg);
    }
    
    return rc;
}

uint32
cdbctl_file_is_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!sal_stat(filename, &stat_buf))
    {
        return TRUE;
    }
    
    return FALSE;
}


int32
cdbctl_do_specific_delay(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    uint32 interface_delay = 50;
    uint32 startup_done = FALSE;
    char *p = (char*)p_data;

    startup_done = cdbctl_file_is_exist(STARTUP_CFG_DONE_FILE);
    if (startup_done)
    {
        /* Added by kcao for bug 46453 2018-03-05, need add more wait after update to FRR3.0 */
        interface_delay = 1000;
    }

    if (CDB_NODE_TBL == p_node->type)
    {
        if (TBL_INTERFACE == p_node->id)
        {
            if (CDB_OPER_ADD == p_info->oper)
            {
                if (p)
                {
                    if (0 == sal_memcmp(p, GLB_IFNAME_VLAN_PREFIX, 4))
                    {
                        sal_task_sleep(interface_delay);
                    }
                    else if (0 == sal_memcmp(p, GLB_IFNAME_LOOPBACK_PREFIX, 8))
                    {
                        sal_task_sleep(interface_delay);
                    }
                }
            }
            else if (CDB_OPER_DEL == p_info->oper)
            {
                if (p)
                {
                    if (0 == sal_memcmp(p, GLB_IFNAME_VLAN_PREFIX, 4))
                    {
                        sal_task_sleep(interface_delay);
                    }
                    else if (0 == sal_memcmp(p, GLB_IFNAME_LOOPBACK_PREFIX, 8))
                    {
                        sal_task_sleep(interface_delay);
                    }
                }
            }
        }

        if (startup_done)
        {
            if (TBL_ARP_FIB == p_node->id)
            {
                if (CDB_OPER_ADD == p_info->oper)
                {
                    sal_task_sleep(100);
                }
                else if (CDB_OPER_DEL == p_info->oper)
                {
                    sal_task_sleep(100);
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int32
cdbclt_process_cmd_get_sdb(sdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;
    
    rc = ipc_ctl_to_ccs_cfg_sdb(p_info->oper, p_node->type, p_node->id, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        _cdbctl_show_get_from_file(p_resp_show->path);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        rc = p_resp_rslt->rc;
        sal_printf("%% %s \n", p_resp_rslt->desc);
    }
    ctc_msg_free(p_resp_msg);

    return rc;
}

int32
cdbclt_process_cmd_cfg(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    /* wait before exec cmd */
    rc = cdbctl_sys_load_wait(p_node);
    if (rc < 0)
    {
        sal_printf("%% Command is forbidden for system is busying now \n");
        return rc;
    }
    
    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }

    cdbctl_do_specific_delay(p_node, p_data, data_len, p_info);
    
    if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        if (PM_E_NONE != p_resp_rslt->rc)
        {
            if (PM_E_CFG_PROMPT == p_resp_rslt->rc)
            {
                rc = PM_E_NONE;
                sal_printf("%s \n", p_resp_rslt->desc);
            }
            else
            {
                rc = p_resp_rslt->rc;
                sal_printf("%% %s \n", p_resp_rslt->desc);
            }
        }
        ctc_msg_free(p_resp_msg);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        cfg_cmd_show_file(p_resp_show->path);
        ctc_msg_free(p_resp_msg);
    }

    return rc;
}


int32
cdbclt_process_cmd_cfg_openflow_batch(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info, int32 err_count)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }
    
    p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
    if (PM_E_NONE != p_resp_rslt->rc)
    {
        if (PM_E_CFG_PROMPT == p_resp_rslt->rc)
        {
            rc = PM_E_NONE;
            sal_printf("%s \n", p_resp_rslt->desc);
        }
        else
        {
            if(0 == err_count)
            {
                sal_printf("\n");
            }
            rc = p_resp_rslt->rc;
            sal_printf("%% %s \n", p_resp_rslt->desc);
        }
    }
    
    ctc_msg_free(p_resp_msg);
    return rc;
}

int32
cdbctl_process_async_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    if (p_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_msg->data;
        if (PM_E_CFG_CONTINUE == p_resp_rslt->rc)
        {
            rc = PM_E_NONE;
            sal_printf("%s", p_resp_rslt->desc);
            sal_fflush(stdout);
            //sal_printf("\ncdbctl_process_async_msg continue \n");
        }
        else if (PM_E_NONE == p_resp_rslt->rc)
        {
            rc = p_resp_rslt->rc;
            sal_printf("%s", p_resp_rslt->desc);
            //sal_fflush(stdout);
            //sal_printf("\ncdbctl_process_async_msg end success \n");
            goto EXIT;
        }
        else
        {
            rc = p_resp_rslt->rc;
            sal_printf("%s", p_resp_rslt->desc);
            //sal_fflush(stdout);
            //sal_printf("\ncdbctl_process_async_msg end fail rc = %d\n", rc);
            goto EXIT;
        }
    }
    
    ctc_msg_free(p_msg);
    return PM_E_NONE;

EXIT:
    
    ctc_msg_free(p_msg);
    exit(rc);
    return PM_E_NONE;
}

int32
cdbclt_process_cmd_continue(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    /* wait before exec cmd */
    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }

    if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        if (PM_E_CFG_CONTINUE == p_resp_rslt->rc)
        {
            rc = PM_E_NONE;
            sal_printf("%s", p_resp_rslt->desc);   
            //sal_printf("\ncdbclt_process_cmd_continue continue \n");
            ctc_sock_register_rx_fn(cdbctl_process_async_msg);
            g_cdbclient_continue = TRUE;
        }
        else
        {
            if (PM_E_NONE != p_resp_rslt->rc)
            {
                rc = p_resp_rslt->rc;
                sal_printf("%% %s", p_resp_rslt->desc);
                //sal_printf("\ncdbclt_process_cmd_continue error \n");
            }
            //sal_printf("\ncdbclt_process_cmd_continue ok \n");
        }
        ctc_msg_free(p_resp_msg);
    }

    return rc;
}
    
int32
_cdbclt_process_cmd_check_wait_one(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }

    if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        if (PM_E_NONE != p_resp_rslt->rc)
        {
            if (PM_E_CFG_PROMPT == p_resp_rslt->rc)
            {
                rc = PM_E_NONE;
            }
            else
            {
                rc = p_resp_rslt->rc;
            }
        }
        ctc_msg_free(p_resp_msg);
    }

    return rc;
}

int32
cdbclt_process_cmd_check_wait(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    cfg_parse_info_t parse_info;
    char   *para_str = NULL;
    uint32 check_wait_times = 100;
    uint32 check_wait_interval = 100;
    uint32 i = 0;
    int32 wait_times = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&parse_info, 0, sizeof(parse_info));
    para_str = (char*)p_data;
    if (para_str && sal_strlen(para_str))
    {
        cfg_cmd_parse_para(para_str, &parse_info);
        if (parse_info.argc > 0)
        {
            rc = PM_E_NONE;
            wait_times = cdb_int_str2val(parse_info.argv[parse_info.argc-1], &rc);
            if (PM_E_NONE == rc)
            {
                check_wait_times = wait_times;
            }
        }
    }

    for (i = 0; i < check_wait_times; i++)
    {
        rc = _cdbclt_process_cmd_check_wait_one(p_node, p_data, data_len, p_info);
        if (PM_E_NONE == rc)
        {
            return rc;
        }
        sal_task_sleep(check_wait_interval);
    }

    return rc;
}

int32
cdbclt_process_cmd_dump_cdb_batch(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    cdb_node_t* p_tbl = NULL;
    uint32 tbl_id = 0;
    char path[CMD_BUF_256];
    int32 rc = PM_E_NONE;
    
    for (tbl_id = 0; tbl_id < TBL_MAX; tbl_id++)
    {
        p_tbl = cdb_get_tbl(tbl_id);
        sal_sprintf(path, "read");
        sal_strcat(path, p_tbl->full_name);
        sal_printf(path);
        cdbclt_process_cmd(path);
    }
    
    return rc;
}

int32
cdbclt_process_cmd_vlan_batch(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    vlan_bmp_t bmp;
    vlan_bmp_t exist_bmp;
    char id_str[256];
    char err_desc[256];
    char *bmp_str = (char*)p_data;
    uint32 i = 0;
    cdb_node_t* p_node = NULL;
    char exist_bmp_str[FILE_READ_LINE_LEN];
    char *brief = "brief";
    int32 rc = PM_E_NONE;

    if ((CDB_OPER_ADD != p_info->oper) && (CDB_OPER_DEL != p_info->oper))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    exist_bmp_str[0] = CMD_ZERO_CHAR;
    if (CDB_OPER_DEL == p_info->oper)
    {
        cdbclt_get_tbl_line(TBL_VLAN, brief, sal_strlen(brief), exist_bmp_str, FILE_READ_LINE_LEN);
        sal_memset(exist_bmp, 0, sizeof(exist_bmp));
        rc = cdb_bitmap_str2val(exist_bmp_str, GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, exist_bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
        if (rc < 0)
        {
            sal_printf("%% %s \n", err_desc);
            return rc;
        }
    }
    
    p_node = _cdbclt_batch_cfg_get_node(p_bat_node);
    if (NULL == p_node)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    
    sal_memset(bmp, 0, sizeof(bmp));
    rc = cdb_bitmap_str2val(bmp_str, GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
    if (rc < 0)
    {
        sal_printf("%% %s \n", err_desc);
        return rc;
    }

    GLB_BMP_ITER_BEGIN(bmp, i)
    {    
        if (CDB_OPER_DEL == p_info->oper)
        {
            /* bypass unexist vlan */
            if (!GLB_BMP_ISSET(exist_bmp, i))
            {
                continue;
            }
        }

        id_str[0] = CMD_ZERO_CHAR;
        sal_sprintf(id_str, "%u", i);
        data_len = sal_strlen(id_str) + 1;
        rc = cdbclt_process_cmd_cfg(p_node, id_str, data_len, p_info);
        if (rc < 0)
        {
            return rc;
        }
    }
    GLB_BMP_ITER_END(bmp, i);
    
    return rc;
}

int32
cdbclt_process_cmd_interface_batch(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    vlan_bmp_t bmp;
    char id_str[256];
    char err_desc[256];
    char *bmp_str = (char*)p_data;
    uint32 i = 0;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    
    if ((CDB_OPER_ADD != p_info->oper) && (CDB_OPER_SET != p_info->oper))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    p_node = _cdbclt_batch_cfg_get_node(p_bat_node);
    if (NULL == p_node)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    
    sal_memset(bmp, 0, sizeof(bmp));
    rc = cdb_bitmap_str2val(bmp_str, GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
    if (rc < 0)
    {
        return rc;
    }

    GLB_BMP_ITER_BEGIN(bmp, i)
    {
        id_str[0] = CMD_ZERO_CHAR;
        sal_sprintf(id_str, "%u", i);
        data_len = sal_strlen(id_str) + 1;
        rc = cdbclt_process_cmd_cfg(p_node, id_str, data_len, p_info);
        if (rc < 0)
        {
            return rc;
        }
    }
    GLB_BMP_ITER_END(bmp, i);
    
    return rc;
}


int32
cdbclt_process_cmd_qos_domain_batch(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    int32 rc = PM_E_NONE;
    uint32 i = 0;
    uint8_bmp_t bmp;
    char tail_cmd_str[256] = { 0 };
    char cmd_str[256] = { 0 };
    char *bmp_str = NULL;
    char err_desc[256];
    cdb_node_t* p_node = NULL;
    cfg_parse_info_t info;
    cfg_parse_get_info_t tmp_info;
    
    if ((CDB_OPER_ADD != p_info->oper) && (CDB_OPER_SET != p_info->oper))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    p_node = _cdbclt_batch_cfg_get_node(p_bat_node);
    if (NULL == p_node)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_memset(&info, 0, sizeof(cfg_parse_info_t));
    cfg_cmd_parse_para((char *)p_data, &info);

    bmp_str = info.argv[2];
    for (i = 3; i < info.argc; i++)
    {
        sal_strcat(tail_cmd_str, info.argv[i]);
        sal_strcat(tail_cmd_str, "/");
    }

    if (sal_strstr(info.argv[1], "phb2"))
    {
        sal_memset(&tmp_info, 0, sizeof(cfg_parse_get_info_t));
        cfg_get_parse_comma(bmp_str, &tmp_info);
        for (i = 0; i < tmp_info.argc; i++)
        {
            sal_sprintf(cmd_str, "%s/%s/%s/", info.argv[0], info.argv[1], tmp_info.argv[i]);
            sal_strcat(cmd_str, tail_cmd_str);
            data_len = sal_strlen(cmd_str) + 1;
            rc = cdbclt_process_cmd_cfg(p_node, cmd_str, data_len, p_info);
            if (rc < 0)
            {
                return rc;
            }
        }
    }
    else 
    {
        sal_memset(bmp, 0, sizeof(bmp));
        rc = cdb_bitmap_str2val(bmp_str, 0, 255, bmp, GLB_UINT8_BMP_WORD_MAX, err_desc);
        if (rc < 0)
        {
            return rc;
        }
        
        GLB_BMP_ITER_BEGIN(bmp, i)
        {

            sal_sprintf(cmd_str, "%s/%s/%d/", info.argv[0], info.argv[1], i);
            sal_strcat(cmd_str, tail_cmd_str);
            data_len = sal_strlen(cmd_str) + 1;
            rc = cdbclt_process_cmd_cfg(p_node, cmd_str, data_len, p_info);
            if (rc < 0)
            {
                return rc;
            }
        }
        GLB_BMP_ITER_END(bmp, i);
    }

    return rc;
}

int32
cdbclt_process_cmd_openflow_batch(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    int32 rc = PM_E_NONE;
    int32 err_count;
    char *req_data = "status";
    cdb_node_t *p_req_node;
    cfg_show_t *p_resp_show = NULL;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    char line[FILE_READ_LINE_32K_LEN] = {0};
    uint32 cmd_len;
    char *key;
    char *lagid;
    char *if_mode;
    char *file_name;
    FILE *fp;
    cdb_node_t cmd_node;
    cfg_parse_info_t cmd_info;
    char cdb_path[MAX_CMD_STR_LEN];
    char *cmd_data = cdb_path;
    cmd_node.type = CDB_NODE_TBL;
    cmd_info.oper = CDB_OPER_SET;
    cfg_parse_get_info_t info; 

    p_req_node = cdb_get_tbl(TBL_INTERFACE);
    sal_memset(cdb_path, 0, sizeof(cdb_path));
    rc = ipc_ctl_to_ccs_cfg(CDB_OPER_GET, p_req_node, req_data, 
        sal_strlen(req_data) + 1, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command: %s failed, rc is %d\n", (char*)p_data, rc);
        return rc;
    }
    
    if (p_resp_msg->data_len != sizeof(cfg_show_t))
    {
        sal_printf("Failed to read port!\n");
        ctc_msg_free(p_resp_msg);
        return PM_E_FAIL;
    }

    sal_printf("Start apply openflow mode");
    sal_fflush(stdout);
    p_resp_show = (cfg_show_t *)p_resp_msg->data;
    file_name = p_resp_show->path;
    fp = sal_fopen(file_name, "r");
    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        err_count = 0;
        key = info.value[TBL_INTERFACE_FLD_KEY]; 
        lagid = info.value[TBL_INTERFACE_FLD_LAG];
        if_mode = info.value[TBL_INTERFACE_FLD_MODE];

        /* interface must be agg or eth */
        if ((0 == sal_strncmp(key, GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN)) || 
            (0 == sal_strncmp(key, GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN)))
        {
            /* when eth is lag meber, the value lagid is "{}", then continue */
            if ((0 == sal_strncmp(key, GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN)) && ('}' != lagid[1]))
            {
                continue;
            }
            
            /* when interface mode switch, then continue */
            if(0 != sal_strncmp(if_mode, "switch", 6))
            {
                sal_printf("\nInterface %s:Can't apply to route port!\n", key);
                continue;
            }
            
            /* switch port mode trunk */
            cmd_node.id = TBL_INTERFACE;
            sal_sprintf(cmd_data, "%s/vlan_type/trunk", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));

            /* mac leaning disable */
            sal_sprintf(cmd_data, "%s/brgif/mac_learning_en/0", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));

            /* switch trunk allowed vlan none */
            sal_sprintf(cmd_data, "%s/brgif/allowed_vlan/trunk/none", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));
            
            /* jumboframe enable */
            sal_sprintf(cmd_data, "%s/jumboframe_en/1", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));

            /* openflow enable */
            cmd_node.id = TBL_OPENFLOW_INTERFACE;
            sal_sprintf(cmd_data, "%s/openflow_enable/1", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));

            /* vlan-filter disable */
            sal_sprintf(cmd_data, "%s/obey_vlan_filter/0", key);
            cmd_len = sal_strlen(cmd_data) + 1;
            CFG_ERROR_STATISTICS(cdbclt_process_cmd_cfg_openflow_batch(&cmd_node, cmd_data, 
                cmd_len, &cmd_info, err_count), err_count);
            sal_memset(cdb_path, 0, sizeof(cdb_path));

            if(err_count > 0)
            {
                sal_printf("Total %d error on interface %s!\n", err_count, key);
            }
            else
            {
                sal_printf(".");
                sal_fflush(stdout);
            }
        }
    }
    ctc_msg_free(p_resp_msg);
    sal_printf("\nProcessing Done!\n");
    return PM_E_NONE;
}

 int32
cdbclt_process_cmd_batch_cfg(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    switch (p_bat_node->id)
    {
        case BAT_VLANRANGE:
            return cdbclt_process_cmd_vlan_batch(p_bat_node, p_data, data_len, p_info);
            
        case BAT_INTERFACERANGE:
            return cdbclt_process_cmd_interface_batch(p_bat_node, p_data, data_len, p_info);

        case BAT_QOS_DOMAIN_RANGE:
            return cdbclt_process_cmd_qos_domain_batch(p_bat_node, p_data, data_len, p_info);

        case BAT_PURE_OPENFLOW:
            return cdbclt_process_cmd_openflow_batch(p_bat_node, p_data, data_len, p_info);
            
        default:
            return PM_E_NONE;
    }

}

int32
cdbclt_process_cmd_batch_get(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    switch (p_node->id)
    {
    case BAT_DUMP_CDB_RANGE:
        return cdbclt_process_cmd_dump_cdb_batch(p_node, p_data, data_len, p_info);

    default:
        return PM_E_NONE;
    }
}

int32
cdbclt_process_cmd_get(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;
    
    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command execute failed, error is %d\n", rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        _cdbctl_show_get_from_file(p_resp_show->path);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        rc = p_resp_rslt->rc;
        sal_printf("%% %s \n", p_resp_rslt->desc);
    }
    ctc_msg_free(p_resp_msg);

    return rc;
}

pm_id_t
cdbclient_get_dest_pm(cdb_node_t* p_node)
{
    if (CDB_NODE_ACT == p_node->type)
    {
        if (ACT_COPY_FILE == p_node->id)
        {
            return PM_ID_CTL;
        }
    }
    
    return PM_ID_INVALID;
}

char*
_ctclib_if_get_matched_prefix(cli_phy_port_master_t *p_master, char *left, cli_phy_port_prefix_t **pp_prefix, uint32 *is_full)
{
    uint32 i = 0;
    char *p = NULL;
    *pp_prefix = NULL;
    *is_full = FALSE;
    
    for (i = 0; i < CLI_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_master->port_prefix[i].valid)
        {
            continue;
        }

        p = sal_strstr(left, p_master->port_prefix[i].full_prefix);
        if (p)
        {
            *pp_prefix = &(p_master->port_prefix[i]);
            *is_full = TRUE;
            return p;
        }

        p = sal_strstr(left, p_master->port_prefix[i].prefix);
        if (p)
        {
            *pp_prefix = &(p_master->port_prefix[i]);
            return p;
        }
    }

    return NULL;
}

static int32
_ctclib_if_and_check_ifname_prefix(cli_phy_port_master_t *p_master, char *left, char *cdb_path)
{
    cli_phy_port_prefix_t *p_prefix = NULL;
    cli_phy_port_prefix_t *p_port_prefix = NULL;
    uint32 is_full = FALSE;
    uint32 slot = 0;
    uint32 port = 0;
    uint32 sub_port = 0;
    uint32 port_type = 0;
    uint8 port_num = 0;
    char sub_port_str[GLB_IFNAME_SZ];
    char port_str[GLB_IFNAME_SZ];
    char ethname[GLB_IFNAME_SZ];
    char tmp[GLB_IFNAME_SZ];
    char *p = NULL;
    char *p_slot = NULL;
    char *start = left;
    int32 rc = 0;
    int32 cat_length = 0;

    sub_port_str[0] = CMD_ZERO_CHAR;
    port_str[0] = CMD_ZERO_CHAR;
    ethname[0] = CMD_ZERO_CHAR;
    tmp[0] = CMD_ZERO_CHAR;
    
    sal_snprintf(sub_port_str, GLB_IFNAME_SZ, "%%u%c%%u%c%%u", p_master->port_connection, p_master->sub_port_connection);
    sal_snprintf(port_str, GLB_IFNAME_SZ, "%%u%c%%u", p_master->port_connection);

    p = _ctclib_if_get_matched_prefix(p_master, left, &p_prefix, &is_full);
    if (p)
    {
        cat_length = (p - left);
        cat_length = (cat_length >= 0) ? cat_length : -cat_length;
        sal_strncat(cdb_path, left, cat_length);
        p_slot = p;
        p_slot += (is_full) ? sal_strlen(p_prefix->full_prefix) : sal_strlen(p_prefix->prefix);
        
        rc = sal_sscanf(p_slot, sub_port_str, &slot, &port, &sub_port);
        if (3 == rc)
        {
            rc = if_get_port_num_by_slot_port(slot, port, sub_port, &port_num);
            if (rc < 0)
            {
                sal_printf("Invalid ifname in %s\n", p);
                return rc;
            }

            sal_snprintf(tmp, GLB_IFNAME_SZ, "%u/%u/%u", slot, port, sub_port);
            p = p_slot + sal_strlen(tmp);
            port_type = p_master->port[port_num].port_type - 1;
            p_port_prefix = &(p_master->port_prefix[port_type]);
        }
        else
        {
            rc = sal_sscanf(p_slot, port_str, &slot, &port);
            if (2 == rc)
            {
                rc = if_get_port_num_by_slot_port(slot, port, 0, &port_num);
                if (rc < 0)
                {
                    if (sal_strstr(p_slot, "split_type"))
                    {
                    }
                    else if (sal_strstr(p_slot, "port_media_type"))
                    {
                    }
                    else
                    {
                        sal_printf("Invalid ifname in %s\n", p);
                        return rc;
                    }
                }
                sal_snprintf(tmp, GLB_IFNAME_SZ, "%u/%u", slot, port);
                p = p_slot + sal_strlen(tmp);
                port_type = p_master->port[port_num].port_type - 1;
                p_port_prefix = &(p_master->port_prefix[port_type]);
            }
        }

        if (p_port_prefix)
        {
            if (p_port_prefix != p_prefix)
            {
                sal_printf("Invalid ifname prefix, expected prefix is %s\n", p_port_prefix->full_prefix);
            }
        }
        else
        {
            sal_printf("Invalid ifname in %s\n", p);
        }

        if (sub_port)
        {
            sal_sprintf(ethname, GLB_IFNAME_ETH_PREFIX"%u-%u_%u", slot, port, sub_port);
        }
        else
        {
            sal_sprintf(ethname, GLB_IFNAME_ETH_PREFIX"%u-%u", slot, port);
        }
        sal_strcat(cdb_path, ethname);
        cat_length = (p - start);
        cat_length = (cat_length >= 0) ? cat_length : -cat_length;
        return cat_length;
    }
    else
    {
        sal_strcat(cdb_path, left);
    }

    return PM_E_NONE;
}

static int32
_ctclib_if_and_check_ifname_default(cli_phy_port_master_t *p_master, char *left, char *cdb_path)
{
    uint32 slot = 0;
    uint32 port = 0;
    uint32 sub_port = 0;
    char *p = NULL;
    char *p_sub_split = NULL;
    char *p1 = NULL;
    char *p1_sub_split = NULL;
    int32 rc = PM_E_NONE;

    p = sal_strstr(left, GLB_IFNAME_ETH_PREFIX);
    if (p)
    {
        if (sal_strchr(p, '/'))
        {
            rc = sal_sscanf(p, GLB_IFNAME_ETH_PREFIX"%u-%u/%u", &slot, &port, &sub_port);
            if (3 == rc)
            {
                p_sub_split = sal_strchr(p, '/');
                if (p_sub_split)
                {
                    p_sub_split[0] = '_';
                }
            }

            p1 = sal_strstr(p+1, GLB_IFNAME_ETH_PREFIX);
            if (p1)
            {
                if (sal_strchr(p1, '/'))
                {
                    rc = sal_sscanf(p1, GLB_IFNAME_ETH_PREFIX"%u-%u/%u", &slot, &port, &sub_port);
                    if (3 == rc)
                    {
                        p1_sub_split = sal_strchr(p1, '/');
                        if (p1_sub_split)
                        {
                            p1_sub_split[0] = '_';
                        }
                    }
                }
            }

            sal_strcat(cdb_path, left);
        }
        else
        {
            rc = sal_sscanf(p, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot, &port);
            sal_strcat(cdb_path, left);
        }
    }
    else
    {
        sal_strcat(cdb_path, left);
    }

    return PM_E_NONE;
}

int32
ctclib_if_and_check_ifname(char *left, char *cdb_path)
{
    cli_phy_port_master_t *p_master = if_get_phy_port_master();

    if (NULL == p_master)
    {
        return _ctclib_if_and_check_ifname_default(p_master, left, cdb_path);
    }
    
    if (p_master->is_default)
    {
        return _ctclib_if_and_check_ifname_default(p_master, left, cdb_path);
    }
    else
    {
        return _ctclib_if_and_check_ifname_prefix(p_master, left, cdb_path);
    }  
}

extern uint32 g_ifname_mapping_enable;

/* conver ifname */
static int32
_cdbctl_convert_path(char *cdb_path, char *raw_path)
{
    char *left = raw_path;
    char *p = cdb_path;
    uint32 str_len = 0;
    int32 rc = PM_E_NONE;

    if (g_ifname_mapping_enable)
    {
        if (sal_strstr(left, "update/cdb/sys/update_port_connection"))
        {
            sal_strcpy(cdb_path, raw_path);
        }
        else
        {
            while (sal_strlen(left))
            {
                rc = ctclib_if_and_check_ifname(left, cdb_path);
                if (PM_E_NONE == rc)
                {
                    str_len = sal_strlen(left);
                    left += str_len;
                    break;
                }
                else if (rc > 0)
                {
                    p += rc;
                    left += rc;
                }
                else
                {
                    return rc;
                }
            }
        }
    }
    else
    {
        sal_strcpy(cdb_path, raw_path);
    }

    return PM_E_NONE;
}

int32
cdbclt_process_cmd(char *raw_path)
{
    char cdb_path[MAX_CMD_STR_LEN];
    char cmd[MAX_CMD_STR_LEN];
    cfg_parse_info_t parse_info;
    char *p_data = NULL;
    char *p = NULL;
    char running_config_path[MAX_CMD_LINE_LEN];
    uint32 data_len = 0;
    cdb_node_t* p_node = NULL;
    sdb_node_t* p_sdb_node = NULL;
    uint32 build_mode = FALSE;
    pm_id_t pm_id = PM_ID_INVALID;
    int32 rc = PM_E_NONE;
    int32 new_build_cfg = FALSE;    // for new show running-config temply
    
    /* 1024 is reserved for config msg header */
    if (sal_strlen(raw_path) > (MSG_SIZE - 1024))
    {
        return PM_E_EXCEED_CMD_LEN;
    }
    
    sal_memset(cdb_path, 0, sizeof(cdb_path));
    sal_memset(cmd, 0, sizeof(cmd));
//    sal_printf("### raw_path: %s ###\n", raw_path);  /* for debug */
    rc = _cdbctl_convert_path(cdb_path, raw_path);
//    sal_printf("### cdb_path: %s ###\n", cdb_path);  /* for debug */
    if (rc < 0)
    {
        return rc;
    }
    
    p_data = cdb_path;
    data_len = sal_strlen(cdb_path)+1;
    sal_memcpy(cmd, p_data, data_len);
    rc = cfg_cmd_parse(cmd, &parse_info);
    if (rc < 0)
    {
        return rc;
    }

    if (CDB_OPER_BUILDCFG == parse_info.oper)
    {
        if (0 == sal_strncmp(parse_info.path, "running/", 8))
        {
            build_mode = FALSE;
            parse_info.path += 8;
        }
        else if (0 == sal_strncmp(parse_info.path, "startup/", 8))
        {
            build_mode = TRUE;
            parse_info.path += 8;
        }
        else
        {
            sal_printf("%% buildcfg should with running / startup\n");
            return -1;
        }

        running_config_path[0] = CMD_ZERO_CHAR;
        if (0 == sal_strncmp(parse_info.path, "config-", 7))
        {
            p = sal_strchr(parse_info.path, CMD_DIVISION_CHAR);
            if (p)
            {
                p[0] = CMD_ZERO_CHAR;
                sal_strncat(running_config_path, parse_info.path, MAX_CMD_LINE_LEN);
            }
            parse_info.path = p + 1;
            new_build_cfg = TRUE;
        }
        else
        {
            sal_strncpy(running_config_path, CFG_SHOWCFG_PATH, MAX_CMD_LINE_LEN);
        }
        data_len = 0;
    }

    if (0 == sal_strncmp(parse_info.path, "sdb/", 4))
    {
        p_sdb_node = sdb_get_node_by_name(parse_info.path);
        if (NULL == p_sdb_node)
        {
            sal_printf("%% Command: %s parse failed, invalid SDB path name %s\n", cdb_path, parse_info.path);
            return -1;
        }

        p_data = sal_strstr(p_data, p_sdb_node->full_name);
        if (NULL == p_data)
        {
            SAL_ASSERT(0);
        }
        p_data += sal_strlen(p_sdb_node->full_name);
        if (CMD_DIVISION_CHAR == p_data[0])
        {
            p_data++;
        }
        if ((NULL == p_data) || (CMD_ZERO_CHAR == p_data[0]))
        {
            data_len = 0;
        }
        else
        {
            data_len = sal_strlen(p_data) + 1;
        }
        
        rc = cdbclt_process_cmd_get_sdb(p_sdb_node, p_data, data_len, &parse_info);
    }
    else
    {
        p_node = cdb_get_node_by_name(parse_info.path);
        if (NULL == p_node)
        {
            sal_printf("%% Command: %s parse failed, invalid CDB path name %s\n", cdb_path, parse_info.path);
            return -1;
        }

        p_data = sal_strstr(p_data, p_node->full_name);
        if (NULL == p_data)
        {
            SAL_ASSERT(0);
        }
        p_data += sal_strlen(p_node->full_name);
        if (CMD_DIVISION_CHAR == p_data[0])
        {
            p_data++;
        }

        if ((NULL == p_data) || (CMD_ZERO_CHAR == p_data[0]))
        {
            data_len = 0;
        }
        else
        {
            data_len = sal_strlen(p_data) + 1;
        }

        /* process in local */
        pm_id = cdbclient_get_dest_pm(p_node);
        if (PM_ID_CTL == pm_id)
        {
            cdbclt_process_cmd_local(p_node, p_data, data_len, &parse_info);
            return PM_E_NONE;
        }
        
        /* send to cds */
        if (CDB_OPER_GET == parse_info.oper)
        {    
            if (CDB_NODE_BAT == p_node->type)
            {
                rc = cdbclt_process_cmd_batch_get(p_node, p_data, data_len, &parse_info);
            }
            else
            {
                rc = cdbclt_process_cmd_get(p_node, p_data, data_len, &parse_info);
            }
        }
        else if (CDB_OPER_GETJSON == parse_info.oper)
        {
            rc = cdbclt_process_cmd_get(p_node, p_data, data_len, &parse_info);
        }
        else if (CDB_OPER_SHOW == parse_info.oper)
        {
            if (TBL_LOGIN == p_node->id && CDB_NODE_TBL == p_node->type)
            {
                /* Added by kcao for bug 51372, when appcfg is crash, reduce sync time */
                extern int32 g_ipc_cfg_sync_time;
                g_ipc_cfg_sync_time = 5;
            }
            parse_info.oper = CDB_OPER_GET;
            rc = cdbclt_process_cmd_show(p_node, p_data, data_len, &parse_info);
        }
        else if (CDB_OPER_BUILDCFG == parse_info.oper)
        {
            if (new_build_cfg)
            {
                return cdbclt_process_cmd_build_cfg(p_node, p_data, data_len, &parse_info, build_mode, running_config_path);
            }
        }
        else
        {
            if (CDB_NODE_BAT == p_node->type)
            {
                rc = cdbclt_process_cmd_batch_cfg(p_node, p_data, data_len, &parse_info);
            }
            else
            {
                if (CDB_NODE_ACT == p_node->type && ACT_CHECK_L3IF_EXIST == p_node->id)
                {
                    rc = cdbclt_process_cmd_check_wait(p_node, p_data, data_len, &parse_info);
                }
                if (CDB_NODE_ACT == p_node->type && ACT_EXEC_TPOAM == p_node->id)
                {
                    rc = cdbclt_process_cmd_continue(p_node, p_data, data_len, &parse_info);
                }
                else
                {
                    rc = cdbclt_process_cmd_cfg(p_node, p_data, data_len, &parse_info);
                }
            }
        }
    }

    return rc;
}

int32
cdbclt_sigint_clean()
{
    cfg_parse_info_t parse_info;
    cdb_node_t* p_node = cdb_get_act(ACT_EXEC_TPOAM);
    char *data = "clean";
    
    if (!g_cdbclient_continue)
    {
        return 0;
    }

    sal_memset(&parse_info, 0, sizeof(parse_info));
    parse_info.oper = CDB_OPER_SET;
    cdbclt_process_cmd_cfg(p_node, data, sal_strlen(data), &parse_info);
    
    return 0;
}


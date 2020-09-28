
#include "proto.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_context.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_group_usm.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "rsa_derhex.h"
#include "appcfg.h"
#include "app_snmp.h"
#include "sal.h"
#include "lib_fs.h"
#include "glb_const.h"
#include "cdb_ds.h"
#include <errno.h>

extern int _app_snmpd_generate_cfg(char *pszErrMsg, size_t nBufLen);
extern int _app_snmpd_restart(void);
extern int _app_static_dns_get_ip_by_hostname(char * hostname, char * ip_addr);
extern int32 cdb_check_snmp_name(const char *name, int32 max_len, char *err_desc);
extern int app_is_password_encryption();

#define SNMP_TRAP_DEFAULT_PORT 162 
#define SNMP_TRAP_MAX_TRAP_RECEIVER 64 
#define SNMP_TRAP_TYPE_ALL 32
#define SNMP_TRAP_TYPE_COLDSTART 1
#define SNMP_TRAP_TYPE_WARMSTART 2
#define SNMP_TRAP_TYPE_LINKDOWN 3
#define SNMP_TRAP_TYPE_LINKUP 4

#define CFG_CHECK_SNMP_NAME(NAME, MAX_LEN, HELP_STR) \
do { \
    char desc_tmp[MAX_ON_DEMAND_CMD_RET_LEN] = {0};  \
    if (cdb_check_snmp_name(NAME, MAX_LEN, para->p_rs->desc) < 0) \
    { \
        sal_strncpy(desc_tmp, para->p_rs->desc, MAX_ON_DEMAND_CMD_RET_LEN); \
        sal_snprintf(para->p_rs->desc, MAX_ON_DEMAND_CMD_RET_LEN, "Invalid %s, %s", HELP_STR, desc_tmp);\
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM; \
        return para->p_rs->rc; \
    } \
} while (0)

int
_decode_snmp_line_str(char *pszLine, char *pszBuf, size_t bufsz)
{
    int ret = sal_b64_pton(pszLine, (unsigned char *)pszBuf, bufsz);

    if (-1 == ret) {
        return -1;
    }

    return 0;
}

int
snmp_check_ipv6_addr(addr_t *addr)
{
    if (IN6_IS_ADDR_MULTICAST (&addr->u.prefix6)
        || IN6_IS_ADDR_LOOPBACK (&addr->u.prefix6)
        || IN6_IS_ADDR_UNSPECIFIED (&addr->u.prefix6))
    {
        return -1;
    }

    return 0;
}

static int
_check_snmp_cfg_field(char *pszkey, char *pszValue, tbl_snmp_cfg_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_SNMP_CFG);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int nValue;
    int rc = 0;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_snmp_cfg_field_id_t)field_id) {
    case TBL_SNMP_CFG_FLD_ENABLE:
        if (strcasecmp(pszValue, "0") && strcasecmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid enable value"); 
            goto err_out;
        }
        pitem->enable = atoi(pszValue);
        break;
    case TBL_SNMP_CFG_FLD_SERVER_ENABLE:
        if (strcasecmp(pszValue, "0") && strcasecmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid enable value"); 
            goto err_out;
        }
        pitem->server_enable = atoi(pszValue);
        break;    
    case TBL_SNMP_CFG_FLD_VERSION:
        if ((nValue = cdb_enum_str2val(snmp_version_strs, 
                SNMP_VERSION_MAX, pszValue)) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid version value"); 
            goto err_out;
        }
        pitem->version = nValue;
        break;
    case TBL_SNMP_CFG_FLD_ENGINEID:
        nValue = strlen(pszValue);
        if (nValue < 10 || nValue > 64) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid engineID length, its range is [10, 64]"); 
            goto err_out;
        }
        if (nValue % 2) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "The number of the engineID's hexadecimal characters must be even"); 
            goto err_out;
        }
    
        if (strspn(pszValue, "0123456789ABCDEFabcdef") != nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid engineID, Character should be \"0-9A-Fa-f\""); 
            goto err_out;
        }
        if (tbl_snmp_usm_user_num_get())
        {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Can't modify engineID, delete usm user first"); 
            goto err_out;
        }
        snprintf(pitem->engineid, SNMP_ENGINEID_SIZE, "%s", pszValue);
        break;
    case TBL_SNMP_CFG_FLD_CONTACT:
        if (cdb_snmp_info_str2val(&pitem->contact, pszValue, para->p_rs->desc) != PM_E_NONE) {
            goto err_out;
        }
        break;
    case TBL_SNMP_CFG_FLD_LOCATION:
        if (cdb_snmp_info_str2val(&pitem->location, pszValue, para->p_rs->desc) != PM_E_NONE) {
            goto err_out;
        }
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_ALL);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_COLDSTART);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_WARMSTART);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_LINKDOWN);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_LINKUP);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_SYSTEM);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_VRRP);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_COLDSTART);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_WARMSTART);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_LINKDOWN);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_LINKUP);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_SYSTEM);
        break;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_VRRP);
        break;
    /* liwh end */
    /* support ptp modified by chenc , 2017-11-15 */
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE);
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE:
        PM_CFG_GET_BOOL(nValue, pszValue, para->p_rs);
        GLB_SET_FLAG_COND(nValue, pitem->trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE);
        break;
    /* chenc end */
    case TBL_SNMP_CFG_FLD_MAX:
        break;
    default:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_snmp_cfg_cmd_get(tbl_snmp_cfg_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_snmp_cfg_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#define GLB_OSPF_REDISTRIBUTE_CONNECTED_TIME       30

static void
_snmp_cfg_cmd_trapall_timer(void *p_data)
{
    tbl_snmp_cfg_t  *p_snmp_cfg = NULL;

    if (!p_data)
    {
        return;    
    }
     
    p_snmp_cfg = (tbl_snmp_cfg_t*)p_data;
    p_snmp_cfg->t_snmp_trap_all_timer = NULL;

    return;
}

static void
_snmp_cfg_cmd_enable_timer(void *p_data)
{
    tbl_snmp_cfg_t  *p_snmp_cfg = NULL;

    if (!p_data)
    {
        return;    
    }
     
    p_snmp_cfg = (tbl_snmp_cfg_t*)p_data;
    p_snmp_cfg->t_snmp_enable_timer = NULL;

    return;
}

static int32
_snmp_cfg_cmd_set(tbl_snmp_cfg_field_id_t field_id, tbl_snmp_cfg_t *pitem, 
    tbl_snmp_cfg_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 remained = 0;

    switch (field_id) {
    case TBL_SNMP_CFG_FLD_ENABLE:
        if (pitem->enable == p_db_item->enable) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_SERVER_ENABLE:
        if (pitem->server_enable == p_db_item->server_enable) {
            return PM_E_NONE;
        }
        if (!p_db_item->t_snmp_enable_timer)
        {
            p_db_item->t_snmp_enable_timer = ctc_task_add_timer(TASK_PRI_NORMAL, 
            _snmp_cfg_cmd_enable_timer, p_db_item, GLB_SNMP_ENABLE_CONFIG_TIMER);
        }
        else
        {
            remained = ctc_timer_get_remain_sec(p_db_item->t_snmp_enable_timer);
            CFG_INVALID_PARAM_RET("Please wait %d seconds when change snmp server enable", remained);
        }
      break;      
    case TBL_SNMP_CFG_FLD_VERSION:
        if (pitem->version == p_db_item->version) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_ENGINEID:
        if (!strcasecmp(pitem->engineid, p_db_item->engineid)) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_CONTACT:
        if (!strcasecmp(pitem->contact.info, p_db_item->contact.info)) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_LOCATION:
        if (!strcasecmp(pitem->location.info, p_db_item->location.info)) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        if (!p_db_item->t_snmp_trap_all_timer)
        {
            p_db_item->t_snmp_trap_all_timer = ctc_task_add_timer(TASK_PRI_NORMAL, 
                       _snmp_cfg_cmd_trapall_timer, p_db_item, GLB_TRAP_ALL_CONFIG_TIMER);
        }
        else
        {
            remained = ctc_timer_get_remain_sec(p_db_item->t_snmp_trap_all_timer);
            CFG_INVALID_PARAM_RET("Please wait %d seconds when change snmp trap enable all", remained);
        }
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;   
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break; 
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break; 
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;
    /* liwh end */
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break; 
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE:
        if (pitem->trap_enable == p_db_item->trap_enable) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_CFG_FLD_MAX:
        return PM_E_NONE;
    default:
        return PM_E_NONE;
    }

    PM_E_RETURN(app_snmp_set(field_id, pitem));
    return rc;
}

static int32
_app_snmp_cmd_set_agent_stats(uint32 value, tbl_snmp_cfg_t *p_db_item, cfg_cmd_para_t *para)
{
    p_db_item->agent_stats = value;
    tbl_snmp_cfg_set_snmp_cfg_field(p_db_item, TBL_SNMP_CFG_FLD_AGENT_STATS);

    return PM_E_NONE;
}

static int32
_app_snmp_cmd_get_agent_stats(tbl_snmp_cfg_t *p_db_item, cfg_cmd_para_t *para)
{
    FILE *fp_stats = NULL;
    FILE *fp = NULL;
    tbl_iter_args_t args;
    char  line[FILE_READ_LINE_LEN];
    
    sal_memset(&args, 0 ,sizeof(args));
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (p_db_item->server_enable)
    {
        fp_stats = sal_fopen("/tmp/snmp_ctc_stats", "r");
        if (NULL != fp_stats)
        {
            while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp_stats))
            {
                sal_fprintf(fp, line);
            }
        }
    }
    else
    {
        sal_fprintf(fp, "SNMP service is disabled\n");
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
app_snmp_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_snmp_cfg_field_id_t field_id = TBL_SNMP_CFG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_snmp_cfg_t item;
    tbl_snmp_cfg_t *p_db_item = NULL;
    uint32 value = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_CFG);    
    p_db_item = tbl_snmp_cfg_get_snmp_cfg();

    item.trap_enable = p_db_item->trap_enable;
    switch (para->oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        if ((2 == argc) && (!strcasecmp(argv[0], "agent_stats")))
        {
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            _app_snmp_cmd_set_agent_stats(value, p_db_item, para);
            break;
        }
        
        if (1 == argc) {
            if (!strcasecmp(argv[0], "contact")) { 
                field_id = TBL_SNMP_CFG_FLD_CONTACT;
                memset(item.contact.info, 0x00, SNMP_INFO_SIZE);
            } else if (!strcasecmp(argv[0], "location")) {
                field_id = TBL_SNMP_CFG_FLD_LOCATION;
                memset(item.location.info, 0x00, SNMP_INFO_SIZE); 
            } else if (!strcasecmp(argv[0], "engineid")) {
                if (tbl_snmp_usm_user_num_get()
                    && 0 != sal_strcmp(p_db_item->engineid, "30383038303830383038")
                    && '\0' != p_db_item->engineid[0])
                {
                    CFG_INVALID_PARAM_RET("Can't modify engineID, delete usm user first");
                }
                field_id = TBL_SNMP_CFG_FLD_ENGINEID;
                memset(item.engineid, 0x00, SNMP_ENGINEID_SIZE); 
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _snmp_cfg_cmd_set(field_id, &item, p_db_item, para);

            if (TBL_SNMP_CFG_FLD_ENGINEID == field_id && 0 == rc)
            {
                CFG_PROMPT_RET("Need to configure 'no snmp-server enable' and 'snmp-server enable' to active engineID.");
            }
            return rc;

            
        }
    
        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_snmp_cfg_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access) {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _snmp_cfg_cmd_set(field_id, &item, p_db_item, para);
            if (TBL_SNMP_CFG_FLD_ENGINEID == field_id && 0 == rc)
            {
                CFG_PROMPT_RET("Need to configure 'no snmp-server enable' and 'snmp-server enable' to active engineID.");
            }
        }
        break;

    case CDB_OPER_GET:
        if ((1 == argc) && (!strcasecmp(argv[0], "agent_stats")))
        {
            _app_snmp_cmd_get_agent_stats(p_db_item, para);
            break;
        }
        if (1 == argc && !strcasecmp(argv[0], "sys-info")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else {   
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }

        rc = _snmp_cfg_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#ifndef MAX_OID_LEN
#define MAX_OID_LEN     64
#endif
static int _check_snmp_oid(char *pszOid)
{
    int len = strlen(pszOid);
    int ret = -1;
    char **ppArr = NULL;
    unsigned long nCnt = 0;
    int i;

    if (len >= SNMP_SUBTREE_SIZE 
            || strspn(pszOid, "0123456789.") != len
            || strstr(pszOid, "..")) {
        goto err_out;
    }

    if (split_tokens(pszOid, strlen(pszOid), MAX_OID_LEN, ".", &nCnt, &ppArr) != 0) {
        return -1;
    }

    for (i = 0; i < nCnt; i++) {
        char *p = ppArr[i];
        unsigned long ul;
        errno = 0;
        ul = strtoul(p, NULL, 10);
        if (0 != errno || (0 == i && ul > 3)) {
            goto err_out;
        }
    }
    ret = 0;
err_out:
    if (NULL != ppArr) {
        free_tokens(&ppArr);
    }
    return ret;
}

static int32
tbl_snmp_view_dump_filter(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs)
{
    tbl_snmp_view_t *p_filter = (tbl_snmp_view_t *)pargs->argv[2];

    if (NULL == p_filter || p_snmp_view == p_filter) {
        return tbl_snmp_view_dump_one(p_snmp_view, pargs);
    }

    return PM_E_NONE;
}

static int32
_snmp_view_cmd_get(tbl_snmp_view_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_snmp_view_iterate((TBL_ITER_CB_FUNC)tbl_snmp_view_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
tbl_snmp_view_dump_filter_by_name(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs)
{
    tbl_snmp_view_t *p_filter = (tbl_snmp_view_t *)pargs->argv[2];

    if (NULL == p_filter || !sal_strncmp(p_snmp_view->key.view.name, p_filter->key.view.name, SNMP_NAME_SIZE)) {
        return tbl_snmp_view_dump_one(p_snmp_view, pargs);
    }

    return PM_E_NONE;
}


static int32
_snmp_view_cmd_get_by_name(tbl_snmp_view_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_snmp_view_iterate((TBL_ITER_CB_FUNC)tbl_snmp_view_dump_filter_by_name, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#if 0
static int32
_snmp_view_cmd_set(tbl_snmp_view_field_id_t field_id, tbl_snmp_view_t *pitem, 
    tbl_snmp_view_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_SNMP_VIEW_FLD_MASK:
    case TBL_SNMP_VIEW_FLD_MAX:
    case TBL_SNMP_VIEW_FLD_KEY:
    case TBL_SNMP_VIEW_FLD_TYPE:
    case TBL_SNMP_VIEW_FLD_REFCNT:
        return PM_E_NONE;
    }

    PM_E_RETURN(app_snmp_view_set(field_id, pitem));
    return rc;
}
#endif

static int32
tbl_snmp_view_item_write(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs)
{
    char *name = pargs->argv[0]; 
    int32 cnt = *((int32*)pargs->argv[1]);
    
    if(p_snmp_view && (!sal_strncmp(p_snmp_view->key.view.name, name, SNMP_NAME_SIZE)))
    {
        p_snmp_view->refcnt = p_snmp_view->refcnt + cnt;
        tbl_snmp_view_set_snmp_view_field(p_snmp_view, TBL_SNMP_VIEW_FLD_REFCNT);
    }

    return PM_E_NONE;
}

int32
_snmp_view_reference(char *pszName, int32 cnt)
{
    tbl_iter_args_t args;
    args.argv[0] = pszName;
    args.argv[1] = &cnt;
    tbl_snmp_view_iterate((TBL_ITER_CB_FUNC)tbl_snmp_view_item_write, &args);

    return 0;
}

tbl_snmp_view_t *
_snmp_view_get_by_name(char* name)
{
    tbl_snmp_view_master_t *p_master = tbl_snmp_view_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if(p_db_snmp_view && (!sal_strncmp(p_db_snmp_view->key.view.name, name, SNMP_NAME_SIZE)))
        {
            
            return p_db_snmp_view;
        }
    }

    return NULL;
}

tbl_snmp_community_t *
_snmp_community_get_by_view_name(char* name)
{
    tbl_snmp_community_master_t *p_master = tbl_snmp_community_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_community_t *p_db_snmp_community = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_community_list, p_db_snmp_community, listnode, next)
    {
        if(p_db_snmp_community && (!sal_strncmp(p_db_snmp_community->view, name, SNMP_NAME_SIZE)))
        {
            
            return p_db_snmp_community;
        }
    }

    return NULL;
}


static int32
_snmp_view_add(tbl_snmp_view_t* item, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_snmp_view_master_t *pmaster;
    tbl_snmp_view_t *p_db_view = NULL;    
    char szErrMsg[256];
    int rc;

    pmaster = tbl_snmp_view_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->snmp_view_list) >= MAX_SNMP_VIEW_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum snmp view limit, max is %d", MAX_SNMP_VIEW_NUM);
    }

    p_db_view = tbl_snmp_view_get_snmp_view(&item->key);
    if(p_db_view)
    {
        return PM_E_NONE;
    }

    if(_snmp_community_get_by_view_name(item->key.view.name) != NULL){
        CFG_CONFLICT_RET("The view name is already used by community, please create other view or delete the community first");
    }
        
    rc = tbl_snmp_view_add_snmp_view(item);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return rc;

}

static int32
_snmp_view_del(tbl_snmp_view_t* item, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_snmp_view_t *p_db_view;
    char szErrMsg[256];
    
    p_db_view = tbl_snmp_view_get_snmp_view(&item->key);    
    if (NULL == p_db_view) {
        return PM_E_NONE;
    }
    if (p_db_view->type != item->type)
    {
        CFG_CONFLICT_RET("Error snmp view included/excluded type");
    }

    tbl_snmp_view_del_snmp_view(&item->key);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return PM_E_NONE;

}

int32
app_snmp_view_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    //tbl_snmp_view_field_id_t field_id = TBL_SNMP_VIEW_FLD_MAX;
    //cdb_field_t *p_field = NULL;
    //int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_snmp_view_t item;
    tbl_snmp_view_t *p_db_item = NULL;
    int nValue;

    CFG_DBG_FUNC();
    sal_memset(&item, 0, sizeof(item));

    p_node = cdb_get_tbl(TBL_SNMP_VIEW);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        CFG_CHECK_SNMP_NAME(argv[0], SNMP_NAME_SIZE - 1, "view name");

        if (!strcasecmp(argv[0], "_all_") 
                || !strcasecmp(argv[0], "_none_")
                || !strcasecmp(argv[0], "none")) {
            CFG_INVALID_PARAM_RET("Default view name, please choose other names");
        } 


        snprintf(item.key.view.name, SNMP_NAME_SIZE, "%s", argv[0]);
        
        if(argc >= 5){
            if (_check_snmp_oid(argv[4]) != 0) {
                CFG_INVALID_PARAM_RET("Invalid snmp subtree"); 
            }
            snprintf(item.key.view.subtree, SNMP_SUBTREE_SIZE, "%s%s", 
                 ('.' == *argv[4]) ? "" : ".", argv[4]);
            if ((nValue = cdb_enum_str2val(snmp_view_class_strs, 
                SNMP_VIEW_CLASS_MAX, argv[2])) == -1) {
            CFG_INVALID_PARAM_RET( "Invalid snmp view type"); 
            }
            item.type = nValue;
    
            if(argc == 7)
            {
                nValue = strlen(argv[6]);
                if (nValue >= SNMP_MASK_SIZE || strspn(argv[6], "0123456789abcdefABCDEF") != nValue) {
                    CFG_INVALID_PARAM_RET("Invalid snmp subtree mask"); 
        
                }
                snprintf(item.mask, SNMP_MASK_SIZE, "%s", argv[6]);
            }
            
            p_db_item = tbl_snmp_view_get_snmp_view(&item.key);
        }

    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("SNMP view %s subtree %s exist", 
                item.key.view.name, item.key.view.subtree);
        }  
      
        rc = _snmp_view_add(&item, argv, argc, para);
        break;
    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("SNMP view %s not found", item.key.view.name);
        }

        if (p_db_item->refcnt > 0) {
            CFG_CONFLICT_RET("SNMP view \%s in use", item.key.view.name);
        }
        rc = _snmp_view_del(&item, argv, argc, para);
        break;

    case CDB_OPER_SET:
        #if 0
        if (2 == argc) {
            if (!strcasecmp(argv[1], "subtree")) {
                field_id = TBL_SNMP_VIEW_FLD_SUBTREE;
                memset(item.subtree[0], 0x00, SNMP_SUBTREE_SIZE);
            } else if (!strcasecmp(argv[1], "mask")) {
                field_id = TBL_SNMP_VIEW_FLD_MASK;
                memset(item.mask[0], 0x00, SNMP_SUBTREE_SIZE);          
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _snmp_view_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_snmp_view_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _snmp_view_cmd_set(field_id, &item, p_db_item, para);
        }
        break;
        #endif
        return rc;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) {
            rc = _snmp_view_cmd_get(p_db_item, para, p_node, &field);   
        }
        else if(argc == 1){
            rc = _snmp_view_cmd_get_by_name(&item, para, p_node, &field);   
            
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- snmp community -- */
static int
_check_snmp_community_field(char *pszkey, char *pszValue, tbl_snmp_community_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_SNMP_COMMUNITY_FLD_MAX;
    int nValue;
    tbl_snmp_view_key_t vkey;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_snmp_community_field_id_t)field_id) {
    case TBL_SNMP_COMMUNITY_FLD_TYPE:
        if ((nValue = cdb_enum_str2val(snmp_community_strs, 
                SNMP_COMMUNITY_MAX, pszValue)) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid snmp community type"); 
            goto err_out;
        }
        pitem->type = nValue;
        break;
    case TBL_SNMP_COMMUNITY_FLD_VIEW:
        memset(&vkey, 0x00, sizeof(tbl_snmp_view_key_t));
        snprintf(vkey.view.name, SNMP_NAME_SIZE, "%s", pszValue);
        if (_snmp_view_get_by_name(pszValue) == NULL) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "There is no such view in the view list, please create it first"); 
            goto err_out;
        }
    
        snprintf(pitem->view, SNMP_SUBTREE_SIZE, "%s", pszValue);
        break;
    case TBL_SNMP_COMMUNITY_FLD_KEY:
    case TBL_SNMP_COMMUNITY_FLD_SECIDX:
    case TBL_SNMP_COMMUNITY_FLD_MAX:
        break;
    }
    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
tbl_snmp_community_dump_filter(tbl_snmp_community_t *p_snmp_community, tbl_iter_args_t *pargs)
{
    tbl_snmp_community_t *p_filter = (tbl_snmp_community_t *)pargs->argv[2];

    if (NULL == p_filter || p_snmp_community == p_filter) {
        return tbl_snmp_community_dump_one(p_snmp_community, pargs);
    }

    return PM_E_NONE;
}

static int32
_snmp_community_cmd_get(tbl_snmp_community_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_snmp_community_iterate((TBL_ITER_CB_FUNC)tbl_snmp_community_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_community_add(char *name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_snmp_community_master_t *pmaster;
    tbl_snmp_community_t community;
    tbl_snmp_community_t *p_community = &community;
    char szErrMsg[256];
    int rc;
    int i;

    sal_memset(p_community, 0x00, sizeof(tbl_snmp_community_t));
    if (sal_strlen(name) >= SNMP_COMMUNITY_SIZE)
    {
        CFG_CONFLICT_RET("Snmp community length should be [1, 255]");
    }
    sal_snprintf(p_community->key.name, SNMP_COMMUNITY_SIZE, "%s", name);

    if (tbl_snmp_community_get_snmp_community(&p_community->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_snmp_community_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->snmp_community_list) >= MAX_SNMP_COMMUNITY_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum snmp community limit");
    }

    p_community->secidx = app_snmp_alloc_secidx();

    /* TBD XXX: check group & access numbers
     * One community will create two groups automatically
     * One community will create one access automatically
     */
    
    for (i = 1; i < argc; i += 2) {
        if (_check_snmp_community_field(argv[i], argv[i + 1], p_community, para) == -1) {
            goto err_out;
        }
    }

    rc = tbl_snmp_community_add_snmp_community(p_community);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    if ('\0' != p_community->view[0]) {
        _snmp_view_reference(p_community->view, 1);
    }
    return rc;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_snmp_community_del(char *name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_snmp_community_t community;
    tbl_snmp_community_t *p_community = &community;
    tbl_snmp_community_t *p_db_community;
    char szErrMsg[256];
    int rc;

    sal_memset(p_community, 0x00, sizeof(tbl_snmp_community_t));
    if (sal_strlen(name) >= SNMP_COMMUNITY_SIZE)
    {
        CFG_CONFLICT_RET("Snmp community length should be [1, 255]");
    }    
    sal_snprintf(p_community->key.name, SNMP_COMMUNITY_SIZE, "%s", name);
    p_db_community = tbl_snmp_community_get_snmp_community(&p_community->key);
    app_snmp_free_secidx(p_db_community->secidx);
    if ('\0' != p_db_community->view[0]) {
        _snmp_view_reference(p_db_community->view, -1);
    }
    rc = tbl_snmp_community_del_snmp_community(&p_community->key);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return rc;
}

static int32
_snmp_community_cmd_set(tbl_snmp_community_field_id_t field_id, tbl_snmp_community_t *pitem, 
    tbl_snmp_community_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_SNMP_COMMUNITY_FLD_TYPE:
        if (pitem->type == p_db_item->type) {
            return PM_E_NONE;
        }
        break;
    case TBL_SNMP_COMMUNITY_FLD_VIEW:
        if (!strcasecmp(pitem->view, p_db_item->view)) {
            return PM_E_NONE;
        }
        if ('\0' != p_db_item->view[0]) {
            _snmp_view_reference(p_db_item->view, -1);
        }
        if ('\0' != pitem->view[0]) {
            _snmp_view_reference(pitem->view, 1);
        }
        break;
    case TBL_SNMP_COMMUNITY_FLD_MAX:
    case TBL_SNMP_COMMUNITY_FLD_KEY:
    case TBL_SNMP_COMMUNITY_FLD_SECIDX:
        return PM_E_NONE;
    }

    PM_E_RETURN(app_snmp_community_set(field_id, pitem));
    return rc;
}

int32
app_snmp_community_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_snmp_community_field_id_t field_id = TBL_SNMP_COMMUNITY_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_snmp_community_t item;
    tbl_snmp_community_t *p_db_item = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        CFG_CHECK_SNMP_NAME(argv[0], SNMP_COMMUNITY_SIZE - 1, "community name");
    
        sal_memset(&item, 0, sizeof(tbl_snmp_community_t));
        snprintf(item.key.name, SNMP_COMMUNITY_SIZE, "%s", argv[0]);
        p_db_item = tbl_snmp_community_get_snmp_community(&item.key);
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            if (strlen(item.key.name) < (MAX_CMD_RET_LEN - strlen("SNMP community  exist")))
            {
                CFG_INVALID_PARAM_RET("SNMP community %s exist", item.key.name);
            }
            else
            {
                CFG_INVALID_PARAM_RET("SNMP community is already exist");
            }
        }        
        rc = _snmp_community_add(argv[0], argv, argc, para);
        break;
    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            if (strlen(item.key.name) < (MAX_CMD_RET_LEN - strlen("SNMP community  not found")))
            {
                CFG_INVALID_PARAM_RET("SNMP community %s not found", item.key.name);
            }
            else
            {
                CFG_INVALID_PARAM_RET("The SNMP community is not found");
            }
        }
        rc = _snmp_community_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if (2 == argc) {
            if (!strcasecmp(argv[1], "view")) {
                field_id = TBL_SNMP_COMMUNITY_FLD_VIEW;
                memset(item.view, 0x00, SNMP_NAME_SIZE);         
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _snmp_community_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_snmp_community_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _snmp_community_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("SNMP community %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _snmp_community_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_snmp_trap_cmd_get(tbl_snmp_trap_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_snmp_trap_iterate((TBL_ITER_CB_FUNC)tbl_snmp_trap_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_trap_add(tbl_snmp_trap_t *pitem, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;


    rc = tbl_snmp_trap_add_snmp_trap(pitem);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}
static int32
_snmp_trap_del(tbl_snmp_trap_t *pitem, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;


    rc = tbl_snmp_trap_del_snmp_trap(&pitem->key);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}


int32
app_snmp_trap_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    tbl_snmp_trap_t item;
    tbl_snmp_trap_t *p_db_item = NULL;
    field_parse_t field;
    uint32 is_inband = FALSE;
    tbl_snmp_trap_master_t *p_trap_master = tbl_snmp_trap_get_master();
    tbl_snmp_inform_master_t *p_info_master = tbl_snmp_inform_get_master();

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_TRAP);  
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    
    if (0 == argc)
    {
        if (CDB_OPER_GET != para->oper)
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    }
    else 
    {
        sal_memset(&item, 0, sizeof(tbl_snmp_trap_t));
        if(sal_strcmp(argv[0], ""))
        {
            rc = cdb_addr_str2val(&item.key.trap.ip_addr, argv[0], IPADDR_BUF_SIZE);
            if (rc < 0) 
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
            }
            if(item.key.trap.ip_addr.family == AF_INET6)
            {
                if(0 != snmp_check_ipv6_addr(&item.key.trap.ip_addr))
                {
                    CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
                }
            }
            else if(item.key.trap.ip_addr.family == AF_INET)
            {
                if (IN_MULTICAST(sal_ntoh32 (inet_addr(argv[0]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a multicast address");
                }
                if (INADDR_BROADCAST == sal_ntoh32 (inet_addr(argv[0])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a broadcast address");
                }
                if (IN_BADCLASS(sal_ntoh32 (inet_addr(argv[0]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be an experimental address");
                }
                if (INADDR_ANY == sal_ntoh32 (inet_addr(argv[0])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be 0.0.0.0");
                }
            }
            CFG_CHECK_SNMP_NAME(argv[1], SNMP_COMMUNITY_SIZE - 1, "community name");
            
            if (argc >= 3)
            {
                cdb_addr_str2val(&item.key.trap.ip_addr, argv[0], IPADDR_BUF_SIZE);
                snprintf(item.key.trap.community, SNMP_COMMUNITY_SIZE, "%s", argv[1]);
                item.key.trap.udpport = SNMP_TRAP_DEFAULT_PORT;
                PM_CFG_GET_UINT(item.key.trap.udpport, argv[2], para->p_rs);
                if (argc > 3)
                {
                    is_inband = (0 == sal_strcmp(argv[3], "inband") ? TRUE : FALSE);
                }
                p_db_item = tbl_snmp_trap_get_snmp_trap(&item.key);
            }
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        if (NULL != p_db_item) {
            return 0;
        } 
        if((2*p_trap_master->snmp_trap_list->count +  p_info_master->snmp_inform_list->count) >= (SNMP_TRAP_MAX_TRAP_RECEIVER - 1))
        {
            CFG_INVALID_PARAM_RET("Reach max number of trap receiver, system supports 64 trap/inform receivers");
        }
       rc = _snmp_trap_add(&item, para);
        break;
    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("The trap receiver not exist");
        }
        if (is_inband != p_db_item->is_inband)
        {
            CFG_INVALID_PARAM_RET("The trap receiver mgmt-if is mismatch");
        }
        rc = _snmp_trap_del(&item, para);
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_item)
        {
            CFG_CONFLICT_RET("The trap receiver not exist");
        }
        p_db_item->is_inband = is_inband;
        tbl_snmp_trap_set_snmp_trap_field(p_db_item, TBL_SNMP_TRAP_FLD_IS_INBAND);
        break;

    case CDB_OPER_GET: 
        rc = _snmp_trap_cmd_get(p_db_item, para, p_node, &field); 
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_snmp_inform_cmd_get(tbl_snmp_inform_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_snmp_inform_iterate((TBL_ITER_CB_FUNC)tbl_snmp_inform_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_inform_add(tbl_snmp_inform_t *pitem, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;


    rc = tbl_snmp_inform_add_snmp_inform(pitem);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}
static int32
_snmp_inform_del(tbl_snmp_inform_t *pitem, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;

    rc = tbl_snmp_inform_del_snmp_inform(&pitem->key);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}


int32
app_snmp_inform_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    tbl_snmp_inform_t item;
    tbl_snmp_inform_t *p_db_item = NULL;
    field_parse_t field;
    uint32 is_inband = FALSE;
    tbl_snmp_trap_master_t *p_trap_master = tbl_snmp_trap_get_master();
    tbl_snmp_inform_master_t *p_info_master = tbl_snmp_inform_get_master();

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_TRAP);  
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
#if 0 /* address optimization by wangqj */
    field.field_id[0] = TBL_SNMP_TRAP_FLD_KEY;
#endif
    
    if (0 == argc)
    {
        if (CDB_OPER_GET != para->oper)
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    }
    else
    {
        sal_memset(&item, 0, sizeof(tbl_snmp_trap_t));
        if(sal_strcmp(argv[0], ""))
        {
            rc = cdb_addr_str2val(&item.key.inform.ip_addr, argv[0], IPADDR_BUF_SIZE);
            if (rc < 0) 
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
            }
            if(item.key.inform.ip_addr.family == AF_INET6)
            {
                if(0 != snmp_check_ipv6_addr(&item.key.inform.ip_addr))
                {
                    CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
                }
            }
            else if(item.key.inform.ip_addr.family == AF_INET)
            {         
                if (IN_MULTICAST(sal_ntoh32 (inet_addr(argv[0]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a multicast address");
                }
                if (INADDR_BROADCAST == sal_ntoh32 (inet_addr(argv[0])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a broadcast address");
                }
                if (IN_BADCLASS(sal_ntoh32 (inet_addr(argv[0]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be an experimental address");
                }
                if (INADDR_ANY == sal_ntoh32 (inet_addr(argv[0])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be 0.0.0.0");
                }
                
            }

            CFG_CHECK_SNMP_NAME(argv[1], SNMP_COMMUNITY_SIZE - 1, "community name");
#if 0            
            if(argc == 2)
            {
                cdb_addr_str2val(&item.key.inform.ip_addr, argv[0], IPADDR_BUF_SIZE);
                snprintf(item.key.inform.community, SNMP_COMMUNITY_SIZE, "%s", argv[1]);
                item.key.inform.udpport = SNMP_TRAP_DEFAULT_PORT;
                p_db_item = tbl_snmp_inform_get_snmp_inform(&item.key);
            }
            if(argc > 2)
            {
                cdb_addr_str2val(&item.key.inform.ip_addr, argv[0], IPADDR_BUF_SIZE);
                snprintf(item.key.inform.community, SNMP_COMMUNITY_SIZE, "%s", argv[1]);
                PM_CFG_GET_UINT(item.key.inform.udpport, argv[2], para->p_rs);
                p_db_item = tbl_snmp_inform_get_snmp_inform(&item.key);
            }
#endif
            if (argc >= 3)
            {
                cdb_addr_str2val(&item.key.inform.ip_addr, argv[0], IPADDR_BUF_SIZE);
                snprintf(item.key.inform.community, SNMP_COMMUNITY_SIZE, "%s", argv[1]);
                item.key.inform.udpport = SNMP_TRAP_DEFAULT_PORT;
                PM_CFG_GET_UINT(item.key.inform.udpport, argv[2], para->p_rs);
                if (argc > 3)
                {
                    is_inband = (0 == sal_strcmp(argv[3], "inband") ? TRUE : FALSE);
                }
                p_db_item = tbl_snmp_inform_get_snmp_inform(&item.key);
            }
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        if (NULL != p_db_item)
        {
            return 0;
        }
        if((2*p_trap_master->snmp_trap_list->count +  p_info_master->snmp_inform_list->count) >= (SNMP_TRAP_MAX_TRAP_RECEIVER - 1))
        {
            CFG_INVALID_PARAM_RET("Reach max number of trap/inform receiver, system supports 96 trap/inform receivers");
        }
       rc = _snmp_inform_add(&item, para);
        break;
    case CDB_OPER_DEL:
        if (NULL == p_db_item)
        {
            CFG_INVALID_PARAM_RET("The inform receiver not exist");
        }
        if (is_inband != p_db_item->is_inband)
        {
            CFG_INVALID_PARAM_RET("The inform receiver mgmt-if is mismatch");
        }
        rc = _snmp_inform_del(&item, para);
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_item)
        {
            CFG_CONFLICT_RET("The inform receiver not exist");
        }
        p_db_item->is_inband = is_inband;
        tbl_snmp_inform_set_snmp_inform_field(p_db_item, TBL_SNMP_INFORM_FLD_IS_INBAND);
        break;

    case CDB_OPER_GET: 
        rc = _snmp_inform_cmd_get(p_db_item, para, p_node, &field); 
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*******************************************************************************
 * Name:    _app_snmp_check_password
 * Purpose: check password string, 
 *    limit it length not overflow, and it must being printable
 * Input: 
 *   pin: password string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
_app_snmp_check_password(char *pwd)
{
    char *p = pwd;
    
    if (NULL == pwd)
    {
        return -1;
    }
    
    if (sal_strlen(pwd) >= SNMP_PASSWORD_SIZE)
    {
        return -1;
    }

    while ('\0' != *p)
    {
        if (!sal_char_isprint(*p)) 
        {
            return -1;
        }

        p++;
    }
    return 0;
}

static int32
_snmp_context_add(char *context_name, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_context_t context_entry;
    tbl_snmp_context_t *p_db_context_entry = NULL;

    sal_memset(&context_entry, 0, sizeof(tbl_snmp_context_t));

    if (NULL == context_name || 0 == sal_strlen(context_name))
    {
        CFG_INVALID_PARAM_RET("Invalid snmp context name");
    }
    /* context name check */
    CFG_CHECK_SNMP_NAME(context_name, SNMP_NAME_SIZE - 1, "context name");
    snprintf(context_entry.key.context_name, SNMP_NAME_SIZE, "%s", context_name);
    
    /* context name already exist check */
    p_db_context_entry = tbl_snmp_context_get_snmp_context(&context_entry.key);
    if (NULL != p_db_context_entry)
    {
        CFG_CONFLICT_RET("The snmp notify entry %s has already exist", 
            context_entry.key.context_name);
    }

    if (SNMP_MAX_CONTEXT == tbl_snmp_context_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of context, system supports %d context name",
            SNMP_MAX_CONTEXT);
    }

    PM_E_RETURN(tbl_snmp_context_add_snmp_context(&context_entry));
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return PM_E_NONE;
}

static int32
_snmp_context_del(char *context_name, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_context_t context_entry;
    tbl_snmp_context_t *p_db_context_entry = NULL;

    sal_memset(&context_entry, 0, sizeof(tbl_snmp_context_t));

    /* context name check */
    CFG_CHECK_SNMP_NAME(context_name, SNMP_NAME_SIZE - 1, "context name");
    snprintf(context_entry.key.context_name, SNMP_NAME_SIZE, "%s", context_name);

    /* find exist context check */
    p_db_context_entry = tbl_snmp_context_get_snmp_context(&context_entry.key);
    if (NULL == p_db_context_entry)
    {
        CFG_CONFLICT_RET("The snmp context %s is not exist", context_entry.key.context_name);
    }

    p_db_context_entry->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_context_del_snmp_context(&context_entry.key);
    
    return rc;
}

static int32
_snmp_context_cmd_get(char *context_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_context_master_t *p_master = tbl_snmp_context_get_master();
    tbl_snmp_context_key_t key;
    tbl_snmp_context_t *p_db_context = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == context_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_context_list, p_db_context, listnode)
        {
            tbl_snmp_context_dump_one(p_db_context, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.context_name, context_name, SNMP_NAME_SIZE);
        p_db_context = tbl_snmp_context_get_snmp_context(&key);
        if (NULL == p_db_context)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp context %s is not exist", key.context_name);
        }
        tbl_snmp_context_dump_one(p_db_context, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}


static int32
_snmp_usm_user_add(char *user_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_usm_user_t usm_user;
    uint8 argc_idx;
    uint8 arg_str_len;
    tbl_snmp_usm_user_t *p_db_usm_user = NULL;
    char szPasswd[PASSWD_SIZE] = {0};
    FILE *fp = NULL;

    sal_memset(&usm_user, 0, sizeof(tbl_snmp_usm_user_t));

    /* user name check */
    CFG_CHECK_SNMP_NAME(user_name, SNMP_NAME_SIZE - 1, "usm user name");
    snprintf(usm_user.key.usm_user_name, SNMP_NAME_SIZE, "%s", user_name);

    /* user already exist check */
    p_db_usm_user = tbl_snmp_usm_user_get_snmp_usm_user(&usm_user.key);
    if (NULL != p_db_usm_user)
    {
        CFG_CONFLICT_RET("The usm user %s has already exist", usm_user.key.usm_user_name);
    }
    
    /* user remote engineid check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("remote");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        arg_str_len = sal_strlen(argv[argc_idx + 1]);
        if (arg_str_len < SNMP_ENGINEID_MIN_SIZE || arg_str_len >= SNMP_ENGINEID_SIZE) 
        {
            CFG_INVALID_PARAM_RET("Invalid engineID length, its range is [10, 64]");
        }
        if (arg_str_len % 2) 
        {
            CFG_INVALID_PARAM_RET("The number of the engineID's hexadecimal characters must be even");
        }
        if (strspn(argv[argc_idx + 1], "0123456789ABCDEFabcdef") != arg_str_len) 
        {
            CFG_INVALID_PARAM_RET("Invalid engineID, character should be \"0-9A-Fa-f\"");
        }
        snprintf(usm_user.remo_engid, SNMP_ENGINEID_SIZE, "%s", argv[argc_idx + 1]);
    }

    /* authcation type check md5 or sha*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("authentication");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "md5")) 
        {
            usm_user.auth_type = SNMP_AUTHENTICATION_MD5;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "sha")) 
        {
            usm_user.auth_type = SNMP_AUTHENTICATION_SHA;
        }
        else
        {
            usm_user.auth_type = 0;
            goto functional;
        }
    }

    /* authcation password check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("auth_pwd");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "encrypt")) 
        {
            if (do_decrypt(M_DEFAULT_PW_STR, argv[argc_idx + 2], 
                sal_strlen(argv[argc_idx + 2]), szPasswd, PASSWD_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid encrypt password");
            }
            if (sal_strlen(szPasswd) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(szPasswd) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }  
            snprintf(usm_user.enc_auth_pwd, PASSWD_SIZE, "%s", argv[argc_idx + 2]);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "secret")) 
        {
            if (sal_strlen(argv[argc_idx + 2]) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(argv[argc_idx + 2]) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }
            gen_passwd_crypt(argv[argc_idx + 2], szPasswd, PASSWD_SIZE);
            snprintf(usm_user.enc_auth_pwd, PASSWD_SIZE, "%s", szPasswd);
        }
        else if (0 == sal_strlen(argv[argc_idx + 1])) 
        {
            if (sal_strlen(argv[argc_idx + 2]) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(argv[argc_idx + 2]) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }
            if (app_is_password_encryption())
            {
                gen_passwd_crypt(argv[argc_idx + 2], szPasswd, PASSWD_SIZE);
                snprintf(usm_user.enc_auth_pwd, PASSWD_SIZE, "%s", szPasswd);
            }
            else
            {
                snprintf(usm_user.auth_pwd, PASSWD_SIZE, "%s", argv[argc_idx + 2]);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid authentication password");
        }
    }

    /* privacy type check aes or des*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("privacy");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "des")) 
        {
            usm_user.priv_type = SNMP_PRIVACY_DES;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "aes")) 
        {
            usm_user.priv_type = SNMP_PRIVACY_AES;
        }
    }

    /* privacy password check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("pri_pwd");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 2]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "encrypt")) 
        {
            if (do_decrypt(M_DEFAULT_PW_STR, argv[argc_idx + 2], 
                sal_strlen(argv[argc_idx + 2]), szPasswd, PASSWD_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid encrypt password");
            }
            if (sal_strlen(szPasswd) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(szPasswd) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }  
            snprintf(usm_user.enc_priv_pwd, PASSWD_SIZE, "%s", argv[argc_idx + 2]);
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "secret")) 
        {
            if (sal_strlen(argv[argc_idx + 2]) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(argv[argc_idx + 2]) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }
            gen_passwd_crypt(argv[argc_idx + 2], szPasswd, PASSWD_SIZE);
            snprintf(usm_user.enc_priv_pwd, PASSWD_SIZE, "%s", szPasswd);
        }
        else if (0 == sal_strlen(argv[argc_idx + 1])) 
        {
            if (sal_strlen(argv[argc_idx + 2]) < SNMP_PASSWORD_MIN_SIZE || _app_snmp_check_password(argv[argc_idx + 2]) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid password: length range is [8, 64), password must be printable");
            }
            if (app_is_password_encryption())
            {
                gen_passwd_crypt(argv[argc_idx + 2], szPasswd, PASSWD_SIZE);
                snprintf(usm_user.enc_priv_pwd, PASSWD_SIZE, "%s", szPasswd);
            }
            else
            {
                snprintf(usm_user.priv_pwd, PASSWD_SIZE, "%s", argv[argc_idx + 2]);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid authentication password");
        }
    }

functional:   
    if (SNMP_MAX_USER == tbl_snmp_usm_user_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of usm user, system supports %d usm user",
            SNMP_MAX_USER);
    }
    
    if ((fp = fopen("/var/net-snmp/snmpd.conf", "r"))!= NULL)
    {
        fclose(fp);
        system("rm /var/net-snmp/snmpd.conf");
    }
    rc = tbl_snmp_usm_user_add_snmp_usm_user(&usm_user);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}

static int32
_snmp_usm_user_del(char *user_name, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_usm_user_t usm_user;
    tbl_snmp_usm_user_t *p_db_usm_user = NULL;

    sal_memset(&usm_user, 0, sizeof(tbl_snmp_usm_user_t));

    /* user name check */
    CFG_CHECK_SNMP_NAME(user_name, SNMP_NAME_SIZE - 1, "usm user name");
    snprintf(usm_user.key.usm_user_name, SNMP_NAME_SIZE, "%s", user_name);

    /* find exist user check */
    p_db_usm_user = tbl_snmp_usm_user_get_snmp_usm_user(&usm_user.key);
    if (NULL == p_db_usm_user)
    {
        CFG_CONFLICT_RET("The usm user %s is not exist", usm_user.key.usm_user_name);
    }
    /* 0xFF flag means entry is deleting */
    p_db_usm_user->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_usm_user_del_snmp_usm_user(&usm_user.key);
    return rc;
}

static int32
_snmp_usm_user_cmd_get(char *user_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_usm_user_master_t *p_master = tbl_snmp_usm_user_get_master();
    tbl_snmp_usm_user_key_t key;
    tbl_snmp_usm_user_t *p_db_user = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == user_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_usm_user_list, p_db_user, listnode)
        {
            tbl_snmp_usm_user_dump_one(p_db_user, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        if (sal_strlen(user_name) > SNMP_NAME_SIZE)
        {
            CFG_INVALID_PARAM_RET("Max snmp user name len is 31");
        }
        sal_strncpy(key.usm_user_name, user_name, SNMP_NAME_SIZE);
        p_db_user = tbl_snmp_usm_user_get_snmp_usm_user(&key);
        if (NULL == p_db_user)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp usm user %s is not exist", user_name);
        }
        tbl_snmp_usm_user_dump_one(p_db_user, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_usm_group_add(char *group_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_group_usm_t grp_usm;
    uint8 argc_idx;
    tbl_snmp_group_usm_t *p_db_grp_usm = NULL;

    sal_memset(&grp_usm, 0, sizeof(tbl_snmp_group_usm_t));

    if (NULL == group_name || 0 == sal_strlen(group_name))
    {
        CFG_INVALID_PARAM_RET("Invalid snmp usm group name");
    }
    /* group name check */
    CFG_CHECK_SNMP_NAME(group_name, SNMP_NAME_SIZE - 1, "group name");
    snprintf(grp_usm.key.group_usm.grp_name, SNMP_NAME_SIZE, "%s", group_name);

    /* usm group user check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("user");
    if (argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp group user name");
        }
        else
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "usm user name");
        }
        snprintf(grp_usm.key.group_usm.security_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        snprintf(grp_usm.security_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
    }

    /* usm group already exist check */
    p_db_grp_usm = tbl_snmp_group_usm_get_snmp_group_usm(&grp_usm.key);
    if (NULL != p_db_grp_usm)
    {
        CFG_CONFLICT_RET("The usm group %s security %s has already exist", 
            grp_usm.key.group_usm.grp_name, grp_usm.key.group_usm.security_name);
    }

    if (SNMP_MAX_GROUP == tbl_snmp_group_usm_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of group usm, system supports %d usm group",
            SNMP_MAX_GROUP);
    }
    if (tbl_snmp_group_usm_security_exist_check(grp_usm.security_name))
    {
        CFG_CONFLICT_RET("The group with security name %s and security model is already existent",
            grp_usm.security_name);
    }

    PM_E_RETURN(tbl_snmp_group_usm_add_snmp_group_usm(&grp_usm));
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return PM_E_NONE;
}

static int32
_snmp_usm_group_del(char *group_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_group_usm_t grp_usm;
    uint8 argc_idx;
    tbl_snmp_group_usm_t *p_db_grp_usm = NULL;
    tbl_snmp_access_usm_t access_group;
    tbl_snmp_access_usm_t *p_db_access_group = NULL;

    sal_memset(&grp_usm, 0, sizeof(tbl_snmp_group_usm_t));

    if (NULL == group_name || 0 == sal_strlen(group_name))
    {
        CFG_INVALID_PARAM_RET("Invalid snmp usm group name");
    }
    /* group name check */
    CFG_CHECK_SNMP_NAME(group_name, SNMP_NAME_SIZE - 1, "group name");
    snprintf(grp_usm.key.group_usm.grp_name, SNMP_NAME_SIZE, "%s", group_name);

    /* usm group user check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("user");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp group user name");
        }
        else
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "usm user name");
        }
        snprintf(grp_usm.key.group_usm.security_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        snprintf(grp_usm.security_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
    }

    /* find exist usm group user check */
    p_db_grp_usm = tbl_snmp_group_usm_get_snmp_group_usm(&grp_usm.key);
    if (NULL == p_db_grp_usm)
    {
        CFG_CONFLICT_RET("The usm group %s security %s is not exist", 
            grp_usm.key.group_usm.grp_name, grp_usm.key.group_usm.security_name);
    }

    sal_memset(&access_group, 0, sizeof(tbl_snmp_access_usm_t));
    snprintf(access_group.key.usm_access_name, SNMP_NAME_SIZE, "%s", group_name);
    p_db_access_group = tbl_snmp_access_usm_get_snmp_access_usm(&access_group.key);
    if (1 == tbl_snmp_group_usm_group_exist_check(group_name)&& NULL != p_db_access_group)
    {
        CFG_CONFLICT_RET("The usm group %s is in used by access, can't delete.", group_name);
    }
    
    /* 0xFF flag means entry is deleting */
    p_db_grp_usm->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_group_usm_del_snmp_group_usm(&grp_usm.key);
    
    return rc;
}

static int32
_snmp_usm_group_cmd_get(char *group_usm_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_group_usm_master_t *p_master = tbl_snmp_group_usm_get_master();
    tbl_snmp_group_usm_t *p_db_group_usm = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    uint8 group_cnt = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == group_usm_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_group_usm_list, p_db_group_usm, listnode)
        {
            tbl_snmp_group_usm_dump_one(p_db_group_usm, &args);
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_group_usm_list, p_db_group_usm, listnode)
        {
            if (0 == sal_strcmp(p_db_group_usm->key.group_usm.grp_name, group_usm_name))
            {
                tbl_snmp_group_usm_dump_one(p_db_group_usm, &args);
                group_cnt++;
            }
        } 
        if (0 == group_cnt)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp usm group %s is not exist", group_usm_name);
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}


static int32
_snmp_usm_access_group_add(char *access_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_access_usm_t access_group;
    uint8 argc_idx;
    tbl_snmp_access_usm_t *p_db_access_group = NULL;

    sal_memset(&access_group, 0, sizeof(tbl_snmp_access_usm_t));

    /* access group name check */
    CFG_CHECK_SNMP_NAME(access_name, SNMP_NAME_SIZE - 1, "access group name");
    /* internal grpcommXX can't be create */
    if (0 == strncasecmp(access_name, "grpcomm", 7))
    {
        CFG_CONFLICT_RET("Can't configure internal default access %s", access_name);
    }
    snprintf(access_group.key.usm_access_name, SNMP_NAME_SIZE, "%s", access_name);
    if (0 == tbl_snmp_group_usm_group_exist_check(access_name))
    {
        CFG_INVALID_PARAM_RET("Snmp-server usm group %s doesn't exist, create it first.", access_name);
    }

    /* user already exist check */
    p_db_access_group = tbl_snmp_access_usm_get_snmp_access_usm(&access_group.key);
    if (NULL != p_db_access_group)
    {
        CFG_CONFLICT_RET("The usm access group %s has already exist", access_group.key.usm_access_name);
    }

    /* security_model type check no auth, auth, priv */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("security_model");
    if(argc_idx != 0xFF && 0 == sal_strcmp(argv[argc_idx + 1], "usm"))
    {
        if (0 == sal_strcmp(argv[argc_idx + 2], "noauth")) 
        {
            access_group.usm_level = SNMP_LEVEL_NOAUTH;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 2], "auth")) 
        {
            access_group.usm_level = SNMP_LEVEL_AUTHNOPRIV;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 2], "priv")) 
        {
            access_group.usm_level = SNMP_LEVEL_AUTHPRIV;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid security_model level type, only can be noauth or auth or priv");
        }
    }

    /* read_view name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("read_view");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            snprintf(access_group.read_view, SNMP_NAME_SIZE, "_all_");
        }
        else
        {
            if (0 != sal_strcmp(argv[argc_idx + 1], "_all_") 
                && 0 != sal_strcmp(argv[argc_idx + 1], "none")
                && 0 != sal_strcmp(argv[argc_idx + 1], "_none_"))
            {
                CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "read view name");
                if (0 == tbl_snmp_view_num_get_by_view_name(argv[argc_idx + 1]))
                {
                    CFG_INVALID_PARAM_RET("View assigned for read is not existent in the view list");
                }
            }
            snprintf(access_group.read_view, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
    }

    /* write_view name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("write_view");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            snprintf(access_group.write_view, SNMP_NAME_SIZE, "none");
        }
        else
        {
            if (0 != sal_strcmp(argv[argc_idx + 1], "_all_") 
                && 0 != sal_strcmp(argv[argc_idx + 1], "none")
                && 0 != sal_strcmp(argv[argc_idx + 1], "_none_"))
            {
                CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "write view name");
                if (0 == tbl_snmp_view_num_get_by_view_name(argv[argc_idx + 1]))
                {
                    CFG_INVALID_PARAM_RET("View assigned for write is not existent in the view list");
                }
            }
            snprintf(access_group.write_view, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
    }
    
    /* notify_view name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("notify_view");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            snprintf(access_group.notify_view, SNMP_NAME_SIZE, "none");
        }
        else
        {
            if (0 != sal_strcmp(argv[argc_idx + 1], "_all_") 
                && 0 != sal_strcmp(argv[argc_idx + 1], "none")
                && 0 != sal_strcmp(argv[argc_idx + 1], "_none_") )
            {
                CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "notify view name");
                if (0 == tbl_snmp_view_num_get_by_view_name(argv[argc_idx + 1]))
                {
                    CFG_INVALID_PARAM_RET("View assigned for notify is not existent in the view list");
                }
            }
            snprintf(access_group.notify_view, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
    }
        
    /* context view name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("context");
    if(argc_idx != 0xFF)
    {
        if (0 != sal_strlen(argv[argc_idx + 1]))
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "context view name");
            snprintf(access_group.context_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
    }

    /* context type check exact or prefix*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("context_type");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "prefix")) 
        {
            access_group.isprefix = TRUE;
        }
        else
        {
            access_group.isprefix = FALSE;
        }
    }
    if (SNMP_MAX_ACCESS == tbl_snmp_access_usm_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of usm access, system supports %d usm access",
            SNMP_MAX_ACCESS);
    }

    if (0 != sal_strcmp(access_group.read_view, "_all_") 
        && 0 != sal_strcmp(access_group.read_view, "none")
        && 0 != sal_strcmp(access_group.read_view, "_none_"))
    {    
        tbl_snmp_view_refcnt_add_by_view_name(access_group.read_view);
    }

    if (0 != sal_strcmp(access_group.write_view, "_all_") 
        && 0 != sal_strcmp(access_group.write_view, "none")
        && 0 != sal_strcmp(access_group.write_view, "_none_"))
    {    
        tbl_snmp_view_refcnt_add_by_view_name(access_group.write_view);
    }

    if (0 != sal_strcmp(access_group.notify_view, "_all_") 
        && 0 != sal_strcmp(access_group.notify_view, "none")
        && 0 != sal_strcmp(access_group.notify_view, "_none_"))
    {    
        tbl_snmp_view_refcnt_add_by_view_name(access_group.notify_view);
    }
    
    rc = tbl_snmp_access_usm_add_snmp_access_usm(&access_group);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}

static int32
_snmp_usm_access_group_del(char *access_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_access_usm_t access_usm;
    tbl_snmp_access_usm_t *p_db_access_usm = NULL;
    uint8 usm_level = 0;
    uint8 argc_idx;
    char context_name[SNMP_NAME_SIZE] = {0};

    sal_memset(&access_usm, 0, sizeof(tbl_snmp_access_usm_t));

    if (NULL == access_name || 0 == sal_strlen(access_name))
    {
        CFG_INVALID_PARAM_RET("Invalid snmp usm access name");
    }
    /* group name check */
    CFG_CHECK_SNMP_NAME(access_name, SNMP_NAME_SIZE - 1, "access group name");
    snprintf(access_usm.key.usm_access_name, SNMP_NAME_SIZE, "%s", access_name);

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("usm");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "noauth")) 
        {
            usm_level = SNMP_LEVEL_NOAUTH;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "auth")) 
        {
            usm_level = SNMP_LEVEL_AUTHNOPRIV;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "priv")) 
        {
            usm_level = SNMP_LEVEL_AUTHPRIV;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid security_model level type, only can be noauth or auth or priv");
        }
    }
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("context");
    if(argc_idx != 0xFF)
    {
        if (0 != sal_strlen(argv[argc_idx + 1]))
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "context name");
            snprintf(context_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
    }

    /* find exist usm access check */
    p_db_access_usm = tbl_snmp_access_usm_get_snmp_access_usm(&access_usm.key);

    if (NULL == p_db_access_usm)
    {
        CFG_CONFLICT_RET("The usm access %s is not exist", access_usm.key.usm_access_name);
    }
    else
    {
        if (usm_level != p_db_access_usm->usm_level)
        {
            CFG_CONFLICT_RET("The usm access %s usm level is wrong with exist group", access_usm.key.usm_access_name);
        }
        if (0 != sal_strcmp(p_db_access_usm->context_name, context_name))
        {
            CFG_CONFLICT_RET("The usm access %s usm context name is wrong with exist group", access_usm.key.usm_access_name);
        }
        tbl_snmp_view_refcnt_del_by_view_name(p_db_access_usm->read_view);
        tbl_snmp_view_refcnt_del_by_view_name(p_db_access_usm->write_view);
        tbl_snmp_view_refcnt_del_by_view_name(p_db_access_usm->notify_view);
    }
    
    p_db_access_usm->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_access_usm_del_snmp_access_usm(&access_usm.key);
    
    return rc;
    return PM_E_NONE;
}

static int32
_snmp_usm_access_group_cmd_get(char *access_grp_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_access_usm_master_t *p_master = tbl_snmp_access_usm_get_master();
    tbl_snmp_access_usm_key_t key;
    tbl_snmp_access_usm_t *p_db_access = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == access_grp_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_access_usm_list, p_db_access, listnode)
        {
            tbl_snmp_access_usm_dump_one(p_db_access, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.usm_access_name, access_grp_name, SNMP_NAME_SIZE);
        p_db_access = tbl_snmp_access_usm_get_snmp_access_usm(&key);
        if (NULL == p_db_access)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp access usm group %s is not exist", access_grp_name);
        }
        tbl_snmp_access_usm_dump_one(p_db_access, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_notify_entry_add(char *notify_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_notify_t notify_entry;
    uint8 argc_idx;
    tbl_snmp_notify_t *p_db_notify_entry = NULL;

    sal_memset(&notify_entry, 0, sizeof(tbl_snmp_notify_t));

    if (NULL == notify_name || 0 == sal_strlen(notify_name))
    {
        CFG_INVALID_PARAM_RET("Invalid snmp notify entry name");
    }
    /* notify name check */
    CFG_CHECK_SNMP_NAME(notify_name, SNMP_NAME_SIZE - 1, "notify name");
    snprintf(notify_entry.key.notify_name, SNMP_NAME_SIZE, "%s", notify_name);

    /* usm tag name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("tag");
    if (argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp notify tag name");
        }
        else
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "usm tag name");
        }
        snprintf(notify_entry.tag_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        /* trap or inform */
        if (0 != sal_strlen(argv[argc_idx + 2]) && 0 == sal_strcmp(argv[argc_idx + 2], "inform"))
        {
            notify_entry.notify_type = TRUE;
        }
        else
        {
            notify_entry.notify_type = FALSE;
        }
    }

    /* notify entry already exist check */
    p_db_notify_entry = tbl_snmp_notify_get_snmp_notify(&notify_entry.key);
    if (NULL != p_db_notify_entry)
    {
        CFG_CONFLICT_RET("The snmp notify entry %s has already exist", 
            notify_entry.key.notify_name);
    }

    if (SNMP_MAX_NOTIFY == tbl_snmp_notify_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of notify, system supports %d usm notify",
            SNMP_MAX_NOTIFY);
    }

    PM_E_RETURN(tbl_snmp_notify_add_snmp_notify(&notify_entry));
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    return PM_E_NONE;
}

static int32
_snmp_notify_entry_del(char *notify_name, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_notify_t notify_entry;
    tbl_snmp_notify_t *p_db_notify_entry = NULL;

    sal_memset(&notify_entry, 0, sizeof(tbl_snmp_notify_t));

    /* notify name check */
    CFG_CHECK_SNMP_NAME(notify_name, SNMP_NAME_SIZE - 1, "notify name");
    snprintf(notify_entry.key.notify_name, SNMP_NAME_SIZE, "%s", notify_name);

    /* find exist notify check */
    p_db_notify_entry = tbl_snmp_notify_get_snmp_notify(&notify_entry.key);
    if (NULL == p_db_notify_entry)
    {
        CFG_CONFLICT_RET("The snmp notify entry %s is not exist", notify_entry.key.notify_name);
    }

    p_db_notify_entry->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_notify_del_snmp_notify(&notify_entry.key);
    
    return rc;
}

static int32
_snmp_notify_entry_cmd_get(char *notify_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_notify_master_t *p_master = tbl_snmp_notify_get_master();
    tbl_snmp_notify_key_t key;
    tbl_snmp_notify_t *p_db_notify = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == notify_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_notify_list, p_db_notify, listnode)
        {
            tbl_snmp_notify_dump_one(p_db_notify, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.notify_name, notify_name, SNMP_NAME_SIZE);
        p_db_notify = tbl_snmp_notify_get_snmp_notify(&key);
        if (NULL == p_db_notify)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp notify entry %s is not exist", key.notify_name);
        }
        tbl_snmp_notify_dump_one(p_db_notify, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int
_snmp_taglist_valid(char *tagList, int tagListLen)
{
    int i = 0;
    int inTag = 0;

    for (i = 0; i < tagListLen; i++) 
    {
        if ((tagList[i] == 0x020 || tagList[i] == 0x09 || tagList[i] == 0x0d || tagList[i] == 0x0b) 
            && !inTag) 
        {
            return 0;
        }
        else if ((tagList[i] == 0x020 || tagList[i] == 0x09 || tagList[i] == 0x0d || tagList[i] == 0x0b) 
                && inTag) 
        {
            inTag = 0;
        }
        else if (!(tagList[i] == 0x020 || tagList[i] == 0x09 || tagList[i] == 0x0d || tagList[i] == 0x0b) 
                && !inTag) 
        {
            inTag = 1;
        }
    }
    if (!inTag) 
    {
        return 0;
    }
    return 1;
}

static int32
_snmp_target_addr_entry_add(char *tarAddr_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[512];
    int rc = 0;
    tbl_snmp_target_addr_t target_addr_entry;
    uint8 argc_idx;
    tbl_snmp_target_addr_t *p_db_tar_addr = NULL;
    tbl_snmp_target_params_key_t target_param_key;
    tbl_snmp_target_params_t *p_db_tar_params = NULL;
    char *tag = NULL;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    char tag_Tmp[SNMP_TAGLIST_MAX_LEN] = {0};
    int i,j;

    sal_memset(&target_addr_entry, 0, sizeof(tbl_snmp_target_addr_t));
    sal_memset(&target_param_key, 0, sizeof(target_param_key));

    /* target address name check */
    CFG_CHECK_SNMP_NAME(tarAddr_name, SNMP_NAME_SIZE - 1, "target address name");
    snprintf(target_addr_entry.key.taraddr_name, SNMP_NAME_SIZE, "%s", tarAddr_name);

    /* user already exist check */
    p_db_tar_addr = tbl_snmp_target_addr_get_snmp_target_addr(&target_addr_entry.key);
    if (NULL != p_db_tar_addr)
    {
        CFG_CONFLICT_RET("The target address name %s has already exist", target_addr_entry.key.taraddr_name);
    }

    /* param name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("param");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp target param name");
        }
        else
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "target param name");
            snprintf(target_addr_entry.para_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
            snprintf(target_param_key.params_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
            p_db_tar_params = tbl_snmp_target_params_get_snmp_target_params(&target_param_key);
            if (NULL == p_db_tar_params)
            {
                CFG_CONFLICT_RET("The target params name %s is not exist", target_param_key.params_name);
            }
        }
    }

    /* mgmt_if check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("mgmt_if");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        target_addr_entry.mgmt_if = TRUE;
    }
    else
    {
        target_addr_entry.mgmt_if = FALSE;
    }
    
    /* ip_addr check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_addr");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp target IP address");
        }
        else
        {
            rc = cdb_addr_str2val(&target_addr_entry.ip_addr, argv[argc_idx + 1], IPADDR_BUF_SIZE);
            if (rc < 0) 
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[argc_idx + 1]);
            }
            if (target_addr_entry.ip_addr.family == AF_INET6)
            {
                if (0 != snmp_check_ipv6_addr(&target_addr_entry.ip_addr))
                {
                    CFG_INVALID_PARAM_RET("Invalid trap receive address %s", argv[argc_idx + 1]);
                }
            }
            else if (target_addr_entry.ip_addr.family == AF_INET)
            {         
                if (IN_MULTICAST(sal_ntoh32 (inet_addr(argv[argc_idx + 1]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a multicast address");
                }
                if (INADDR_BROADCAST == sal_ntoh32 (inet_addr(argv[argc_idx + 1])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be a broadcast address");
                }
                if (IN_BADCLASS(sal_ntoh32 (inet_addr(argv[argc_idx + 1]))))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be an experimental address");
                }
                if (INADDR_ANY == sal_ntoh32 (inet_addr(argv[argc_idx + 1])))
                {
                    CFG_INVALID_PARAM_RET("Trap receive address cannot be 0.0.0.0");
                } 
            }
        }
    }
        
    /* udp port check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("udp_port");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            target_addr_entry.upd_port = SNMP_DEFAULT_UDPPORT;
        }
        else
        {
            PM_CFG_GET_UINT(target_addr_entry.upd_port, argv[argc_idx + 1], para->p_rs);
        }
    }
    
    /* timeout check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("timeout");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            target_addr_entry.timeout = SNMP_DEFAULT_TIMEOUT_CTC;
        }
        else
        {
            PM_CFG_GET_UINT(target_addr_entry.timeout, argv[argc_idx + 1], para->p_rs);
        }
    }

    /* retries check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("retries");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            target_addr_entry.retries = SNMP_DEFAULT_RETRIES_CTC;
        }
        else
        {
            PM_CFG_GET_UINT(target_addr_entry.retries, argv[argc_idx + 1], para->p_rs);
        }
    }

    /* taglist check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("taglist");
    if(argc_idx != 0xFF)
    {
        if (NULL == argv[argc_idx + 1]) 
        {
             snprintf(target_addr_entry.tag_list, SNMP_TAGLIST_MAX_LEN, "%s", tarAddr_name);
        }
        else
        {
            if (sal_strlen(argv[argc_idx + 1]) > SNMP_TAGLIST_MAX_LEN-1)
            {
                CFG_INVALID_PARAM_RET("Invalid snmp taglist name length, max length is 255");
            }
            tag = argv[argc_idx + 1];
            i = 0;
            j = 0;
            while ('\0' != tag[i] && i < sal_strlen(argv[argc_idx + 1]))
            {
                if ('\"' == tag[i])
                {
                    i++;
                    continue;
                }
                tag_Tmp[j] = tag[i];
                j++;
                i++;
            };
            if (split_tokens(tag_Tmp, sal_strlen(tag_Tmp), SNMP_MAX_TAGS, 
			   " ", &nToks, &ppTokArr) != 0)
    	    {
                CFG_INVALID_PARAM_RET("Invalid snmp taglist name length, max length is 255");
    	    }    

            if (!_snmp_taglist_valid(tag, sal_strlen(tag))) 
            {
                CFG_INVALID_PARAM_RET("Invalid taglist name");
            } 
            sal_strncpy(target_addr_entry.tag_list, tag_Tmp, SNMP_TAGLIST_MAX_LEN);
        }
    }

    if (SNMP_MAX_TARGET_ADDR == tbl_snmp_target_addr_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of target address, system supports %d target address",
            SNMP_MAX_TARGET_ADDR);
    }

    rc = tbl_snmp_target_addr_add_snmp_target_addr(&target_addr_entry);
    _app_snmpd_generate_cfg(szErrMsg, 512);
    _app_snmpd_restart();
    
    return rc;
}

static int32
_snmp_target_addr_entry_del(char *tarAddr_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_target_addr_t target_addr_entry;
    tbl_snmp_target_addr_t *p_db_tar_addr = NULL;
    uint8 argc_idx;
    uint32 port;
    addr_t tmp_addr;

    sal_memset(&target_addr_entry, 0, sizeof(tbl_snmp_target_addr_t));

    /* target address name check */
    CFG_CHECK_SNMP_NAME(tarAddr_name, SNMP_NAME_SIZE - 1, "target address name");
    snprintf(target_addr_entry.key.taraddr_name, SNMP_NAME_SIZE, "%s", tarAddr_name);

    /* target address exist check */
    p_db_tar_addr = tbl_snmp_target_addr_get_snmp_target_addr(&target_addr_entry.key);
    if (NULL == p_db_tar_addr)
    {
        CFG_CONFLICT_RET("The target address entry %s is not exist", target_addr_entry.key.taraddr_name);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("mgmt_if");
    if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (TRUE != p_db_tar_addr->mgmt_if)
        {
            CFG_CONFLICT_RET("The target address is not mgmt-if");
        }
    }
    else
    {
        if (FALSE != p_db_tar_addr->mgmt_if)
        {
            CFG_CONFLICT_RET("The target address is mgmt-if");
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("udp_port");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            if (SNMP_DEFAULT_UDPPORT != p_db_tar_addr->upd_port)
            {
                CFG_CONFLICT_RET("The target address udpport is %d, confilct", p_db_tar_addr->upd_port);
            }
        }
        else
        {
            PM_CFG_GET_UINT(port, argv[argc_idx + 1], para->p_rs);
            if (port != p_db_tar_addr->upd_port)
            {
                CFG_CONFLICT_RET("The target address udpport is %d, conflict", p_db_tar_addr->upd_port);
            }
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_addr");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp target IP address");
        }
        else
        {
            rc = cdb_addr_str2val(&tmp_addr, argv[argc_idx + 1], IPADDR_BUF_SIZE);
            if (rc < 0) 
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[argc_idx + 1]);
            }
            if (tmp_addr.family == p_db_tar_addr->ip_addr.family)
            {
                if (tmp_addr.u.prefix4.s_addr != p_db_tar_addr->ip_addr.u.prefix4.s_addr)
                {
                    CFG_INVALID_PARAM_RET("Inexistent configured target address %s", argv[argc_idx + 1]);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Inexistent configured target address %s", argv[argc_idx + 1]);
            }
        }
    }

    p_db_tar_addr->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_target_addr_del_snmp_target_addr(&target_addr_entry.key);

    return rc;
}

static int32
_snmp_target_addr_entry_cmd_get(char *tarAddr_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_target_addr_master_t *p_master = tbl_snmp_target_addr_get_master();
    tbl_snmp_target_addr_key_t key;
    tbl_snmp_target_addr_t *p_db_tarAddr = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == tarAddr_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_target_addr_list, p_db_tarAddr, listnode)
        {
            tbl_snmp_target_addr_dump_one(p_db_tarAddr, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.taraddr_name, tarAddr_name, SNMP_NAME_SIZE);
        p_db_tarAddr = tbl_snmp_target_addr_get_snmp_target_addr(&key);
        if (NULL == p_db_tarAddr)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp target address entry %s is not exist", key.taraddr_name);
        }
        tbl_snmp_target_addr_dump_one(p_db_tarAddr, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_snmp_target_params_entry_add(char *tarParam_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_target_params_t target_params_entry;
    uint8 argc_idx;
    tbl_snmp_target_params_t *p_db_tar_params = NULL;

    sal_memset(&target_params_entry, 0, sizeof(tbl_snmp_target_params_t));

    /* target param name check */
    CFG_CHECK_SNMP_NAME(tarParam_name, SNMP_NAME_SIZE - 1, "target param name");
    snprintf(target_params_entry.key.params_name, SNMP_NAME_SIZE, "%s", tarParam_name);

    /* user already exist check */
    p_db_tar_params = tbl_snmp_target_params_get_snmp_target_params(&target_params_entry.key);
    if (NULL != p_db_tar_params)
    {
        CFG_CONFLICT_RET("The target params name %s has already exist", target_params_entry.key.params_name);
    }

    /* param name check */
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("user");
    if(argc_idx != 0xFF)
    {
        if (0 == sal_strlen(argv[argc_idx + 1]))
        {
            CFG_INVALID_PARAM_RET("Invalid snmp target param user name");
        }
        else
        {
            CFG_CHECK_SNMP_NAME(argv[argc_idx + 1], SNMP_NAME_SIZE - 1, "target param user name");
            snprintf(target_params_entry.user_name, SNMP_NAME_SIZE, "%s", argv[argc_idx + 1]);
        }
        if (NULL == argv[argc_idx + 2])
        {
            CFG_INVALID_PARAM_RET("Invalid snmp target param security level");
        }
        else
        {
            if (0 == sal_strcmp(argv[argc_idx + 2], "noauth")) 
            {
                target_params_entry.usm_level = SNMP_LEVEL_NOAUTH;
            }
            else if (0 == sal_strcmp(argv[argc_idx + 2], "auth")) 
            {
                target_params_entry.usm_level = SNMP_LEVEL_AUTHNOPRIV;
            }
            else if (0 == sal_strcmp(argv[argc_idx + 2], "priv")) 
            {
                target_params_entry.usm_level = SNMP_LEVEL_AUTHPRIV;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid security_model level type, only can be noauth or auth or priv");
            }
        }
    }

    if (SNMP_MAX_TARGET_PARAMS == tbl_snmp_target_params_num_get())
    {
        CFG_CONFLICT_RET("Reach max number of target params, system supports %d target params",
            SNMP_MAX_TARGET_PARAMS);
    }
    rc = tbl_snmp_target_params_add_snmp_target_params(&target_params_entry);
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    
    return rc;
}

static int32
_snmp_target_params_entry_del(char *tarParam_name, cfg_cmd_para_t *para)
{
    char szErrMsg[256];
    int rc = 0;
    tbl_snmp_target_params_t target_params_entry;
    tbl_snmp_target_params_t *p_db_tar_addr = NULL;

    sal_memset(&target_params_entry, 0, sizeof(tbl_snmp_target_params_t));

    /* target param name check */
    CFG_CHECK_SNMP_NAME(tarParam_name, SNMP_NAME_SIZE - 1, "target param name");
    snprintf(target_params_entry.key.params_name, SNMP_NAME_SIZE, "%s", tarParam_name);

    /* target address exist check */
    p_db_tar_addr = tbl_snmp_target_params_get_snmp_target_params(&target_params_entry.key);
    if (NULL == p_db_tar_addr)
    {
        CFG_CONFLICT_RET("The target params entry %s is not exist", target_params_entry.key.params_name);
    }

    if (NULL != tbl_snmp_target_addr_get_by_param_name(tarParam_name))
    {
        CFG_CONFLICT_RET("The target params %s is used by target-address, can't delete", tarParam_name);
    }

    p_db_tar_addr->flags = 0xFF;
    _app_snmpd_generate_cfg(szErrMsg, 256);
    _app_snmpd_restart();
    rc = tbl_snmp_target_params_del_snmp_target_params(&target_params_entry.key);
    
    return rc;
}


static int32
_snmp_target_params_entry_cmd_get(char *tarParam_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_snmp_target_params_master_t *p_master = tbl_snmp_target_params_get_master();
    tbl_snmp_target_params_key_t key;
    tbl_snmp_target_params_t *p_db_tarParams = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == tarParam_name)
    {
        CTCLIB_SLIST_LOOP(p_master->snmp_target_params_list, p_db_tarParams, listnode)
        {
            tbl_snmp_target_params_dump_one(p_db_tarParams, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.params_name, tarParam_name, SNMP_NAME_SIZE);
        p_db_tarParams = tbl_snmp_target_params_get_snmp_target_params(&key);
        if (NULL == p_db_tarParams)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Snmp target params entry %s is not exist", key.params_name);
        }
        tbl_snmp_target_params_dump_one(p_db_tarParams, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
app_snmp_context_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_CONTEXT);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        rc = _snmp_context_add(argv[0], para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        rc = _snmp_context_del(argv[0], para);
        break;

    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_context_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_context_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
app_snmp_usm_user_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_USM_USER);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid usm user name");
        }
        else
        {
            rc = _snmp_usm_user_add(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_DEL:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid usm user name");
        }
        else
        {
            rc = _snmp_usm_user_del(argv[0], para);
        }
        break;

    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_usm_user_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_usm_user_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
app_snmp_group_usm_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_GROUP_USM);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        rc = _snmp_usm_group_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        rc = _snmp_usm_group_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:

        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_usm_group_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_usm_group_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
app_snmp_access_usm_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_ACCESS_USM);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid access group name");
        }
        else
        {
            rc = _snmp_usm_access_group_add(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_DEL:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid usm user name");
        }
        else
        {
            rc = _snmp_usm_access_group_del(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_usm_access_group_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_usm_access_group_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
app_snmp_notify_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_NOTIFY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        rc = _snmp_notify_entry_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        rc = _snmp_notify_entry_del(argv[0], para);
        break;

    case CDB_OPER_SET:

        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_notify_entry_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_notify_entry_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
app_snmp_target_addr_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid target address name");
        }
        else
        {
            rc = _snmp_target_addr_entry_add(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_DEL:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid target address name");
        }
        else
        {
            rc = _snmp_target_addr_entry_del(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_SET:

        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_target_addr_entry_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_target_addr_entry_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
app_snmp_target_params_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 > argc || 0 == sal_strlen(argv[0]))
        {
            CFG_INVALID_PARAM_RET("Invalid target params name");
        }
        else
        {
            rc = _snmp_target_params_entry_add(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        rc = _snmp_target_params_entry_del(argv[0], para);
        break;

    case CDB_OPER_SET:

        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (argc == 0) 
        {
            rc = _snmp_target_params_entry_cmd_get(NULL, para, p_node, &field);   
        }
        else if(argc == 1)
        {
            rc = _snmp_target_params_entry_cmd_get(argv[0], para, p_node, &field);   
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}



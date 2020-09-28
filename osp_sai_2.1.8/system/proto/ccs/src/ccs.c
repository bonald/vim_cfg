/* ccs - central configure service
 * 
 */

#include "proto.h"
#include "ctc_task.h"
#include "ccs.h"
#include "ccs_cfg.h"
#include "ccs_show.h"
#include "cdb_store.h"
#include "cfg_cmd.h"
#include "gen/cdb_dest_pm_priv.h"
#include "gen/cdb_inc_priv.h"
#include "pb_cdb_inc.h"
#include "tbl.pb-c.h"
#include "ds.pb-c.h"
#include "pb_common.h"
#include "pb_tbl.h"
#include "pb_ds.h"
#include "sdb_defines.h"

#define CCS_SUBSCRIBER_TX_ERROR_MAX     5

typedef struct
{
    uint8           type;                       /* cdb_subscribe_type_t */
    uint8           format;                     /* cdb_subscribe_format_t */
    uint16          sync;
    uint32          peer;
} ccs_subscribe_hdr_t;

typedef struct
{
    ccs_subscribe_hdr_t hdr;
    uint32          field;
    uint8           node_type;                  /* cdb_node_type_t */
    uint8           tbl_key_valid;
    uint8           ds_key_valid;
    uint16          tbl_id;                     /* cdb_tbl_t */
    char            tbl_key[CMD_BUF_256];
} ccs_subscribe_cdb_t;

typedef struct
{
    ccs_subscribe_hdr_t hdr;
    uint32              tbl_num;
    uint32              tbl_id[TBL_MAX];
} ccs_subscribe_tbl_t;

uint32 g_cdb_data_dispatch_before_inited = 0;
static ctclib_slist_t *g_sub_cdb_list = NULL;
static ctc_task_t     *g_sys_load_timer = NULL;

typedef struct
{
    uint32   switchd;
    uint32   routed;
    uint32   openflow;
    uint32   cds;
    uint32   fea;
} ccs_sys_load_threshold_t;

typedef struct
{
    uint32                      cli_sys_load_en;
    ctc_task_t                 *sys_load_timer;
    ccs_sys_load_threshold_t    warning;
    ccs_sys_load_threshold_t    critical;
    sys_load_state_t            state;
    uint32                      warning_duration;
} ccs_sys_load_t;

ccs_sys_load_t g_sys_load_ctl;

int32
ccs_sub_tbls_sync(ccs_subscribe_tbl_t *p_sub_tbl);

extern void
_lib_cmd_set_timezone(tbl_clock_t *p_clk);

pm_id_t
ccs_get_show_resource_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
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
ccs_get_clock_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
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
ccs_get_show_common_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
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
ccs_get_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
{
    pm_id_t pm_id = PM_ID_INVALID;
    
    if (CDB_OPER_BUILDCFG == p_msg->u_hdr.cfg.oper)
    {
        return PM_ID_CCS;
    }
    
    if (CDB_OPER_VIEW == p_msg->u_hdr.cfg.oper)
    {
        return PM_ID_CCS;
    }
    
    if (CDB_NODE_DIR == p_node->type)
    {
        return PM_ID_CCS;
    }

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
            if (ACT_SHOW_RESOURCE == p_node->id)
            {
                pm_id = ccs_get_show_resource_dest_pm(p_node, p_msg);
            }
            else if (ACT_CLOCK == p_node->id)
            {
                pm_id = ccs_get_clock_dest_pm(p_node, p_msg);
            }
            else
            {
                pm_id = ccs_get_show_common_dest_pm(p_node, p_msg);
            }
            
        }
        return pm_id;
    }
    
    return PM_ID_INVALID;
}

extern int32
cdb_dump_dir_node(const char *path, cfg_cmd_para_t *para);

extern int32
cdb_dump_tbl_node(cdb_node_t* p_node, char *param, uint32 param_len, cfg_cmd_para_t *para);

int32
ccs_show_node_info(int32 sock, ctc_msg_t *p_msg, cdb_node_t* p_node)
{
    cfg_cmd_para_t  para;
    int32 rs_len = 0;
    int32 rc = PM_E_NONE;
    
    para.oper = p_msg->u_hdr.cfg.oper;
    para.sock = sock;
    para.pid = p_msg->u_hdr.cfg.pid;
    para.p_msg = p_msg;
    sal_memset(para.ret_buf, 0, sizeof(para.ret_buf));
    para.p_rs = (cfg_result_t*)para.ret_buf;
    para.p_show = (cfg_show_t*)para.ret_buf;

    if (CDB_NODE_DIR == p_node->type)
    {
        rc = cdb_dump_dir_node(p_node->full_name, &para);
    }
    else if (CDB_NODE_TBL == p_node->type)
    {
        rc = cdb_dump_tbl_node(p_node, p_msg->data, p_msg->data_len, &para);
    }
    else if (CDB_NODE_ACT == p_node->type)
    {
        rc = cdb_dump_tbl_node(p_node, p_msg->data, p_msg->data_len, &para);
    }
    rs_len = sizeof(cfg_show_t);
    (void)rc;
    return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
}

int32
ccs_error_info(int32 sock, ctc_msg_t *p_msg, char *err_str)
{
    cfg_cmd_para_t  para;
    int32 rs_len = 0;

    sal_memset(&para, 0, sizeof(para));
    
    para.oper = p_msg->u_hdr.cfg.oper;
    para.sock = sock;
    para.pid = p_msg->u_hdr.cfg.pid;
    para.p_msg = p_msg;
    sal_memset(para.ret_buf, 0, sizeof(para.ret_buf));
    para.p_rs = (cfg_result_t*)para.ret_buf;
    sal_strncpy(para.p_rs->desc, err_str, MAX_CMD_RET_LEN);
    para.p_rs->rc = PM_E_FAIL;
    rs_len = sizeof(cfg_result_t);
    return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
}

extern int32
cfg_lic_check_cdb_node(cdb_node_t* p_node);
extern int32
ccs_lic_init();

static int32
_ccs_process_cfg_req_cdb(int32 sock, ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    cdb_node_t* p_node = NULL;
    pm_id_t pm_id = PM_ID_INVALID;
    int32 rc = PM_E_NONE;

    /* 1. get node */
    p_node = cdb_get_node_by_id(p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
    if (NULL == p_node)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Invalid CDB node type %u, id %u\n", p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
        return PM_E_FAIL;
    }

    /* 2. check CDB */
    rc = cfg_lic_check_cdb_node(p_node);
    if (rc < 0)
    {
        ccs_error_info(sock, p_msg, "Command is refused for licence check");
        return PM_E_FAIL;
    }
    
    /* 3. get dest PM */
    pm_id = ccs_get_dest_pm(p_node, p_msg);
    if (PM_ID_INVALID == pm_id)
    {
        ccs_error_info(sock, p_msg, "Invalid destination PM");
        return PM_E_FAIL;
    }

    /* 4. process at CDS or send to dest PM */
    if (PM_ID_CCS == pm_id)
    {
        if (CDB_OPER_VIEW == p_msg->u_hdr.cfg.oper)
        {
            return ccs_show_node_info(sock, p_msg, p_node);
        }
        else if (CDB_OPER_BUILDCFG == p_msg->u_hdr.cfg.oper)
        {
            return ccs_cfg_build_cfg(sock, p_msg, p_node);
        }
        else
        {
            return cfg_cmd_process(sock, p_msg);
        }
    }
    else
    {
        /* dest to other PM */
        p_msg->u_hdr.cfg.dest = pm_id;
        return ipc_ccs_to_pm_cfg(lib_master->cfg_sock, p_msg);
    }

    return PM_E_NONE;
}

static int32
_ccs_process_cfg_req_sdb(int32 sock, ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    sdb_node_t* p_node = NULL;
    pm_id_t pm_id = PM_ID_INVALID;

    /* 1. get node */
    p_node = sdb_get_node_by_id(p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
    if (NULL == p_node)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Invalid SDB node type %u, id %u\n", p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
        return PM_E_FAIL;
    }

    /* 2. check CDB */
    
    /* 3. get dest PM */
    pm_id = PM_ID_FEA;

    /* 4. process at CDS or send to dest PM */
    p_msg->u_hdr.cfg.dest = pm_id;
    return ipc_ccs_to_pm_cfg(lib_master->cfg_sock, p_msg);

    return PM_E_NONE;
}

static int32
_ccs_process_cfg_req(int32 sock, ctc_msg_t *p_msg)
{
    if (p_msg->u_hdr.cfg.type < SDB_NODE_BASE)
    {
        return _ccs_process_cfg_req_cdb(sock, p_msg);
    }
    else
    {
        return _ccs_process_cfg_req_sdb(sock, p_msg);
    }
}

static int32
_ccs_process_cfg_resp(int32 sock, ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    return ipc_ccs_to_pm_cfg(lib_master->cfg_sock, p_msg);
}

int32
pb_msg_to_subscribe_response(ctc_msg_t *p_msg, pb_subscribe_response_t *p_hdr)
{
    msg_data_hdr_t *p_data_hdr = NULL;

    if (MSG_OPER_DATA != p_msg->hdr.operation)
    {
        return PM_E_FAIL;
    }
    p_data_hdr = &p_msg->u_hdr.data;

    if (CDB_OPER_ADD != p_data_hdr->oper && CDB_OPER_SET != p_data_hdr->oper && CDB_OPER_DEL != p_data_hdr->oper )
    {
        return PM_E_FAIL;
    }

    sal_memset(p_hdr, 0, sizeof(pb_subscribe_response_t));
    p_hdr->oper = p_data_hdr->oper;
    p_hdr->type = p_data_hdr->type;
    p_hdr->tbl_id = p_data_hdr->tbl_id;
    p_hdr->ds_id = p_data_hdr->ds_id;
    p_hdr->ds2_id = p_data_hdr->ds2_id;
    p_hdr->field_id = p_data_hdr->fields[0];

    return PM_E_NONE;
}

int32
_ccs_protobuf_pack_payload(uint8 *data, uint32 data_len, ctc_msg_t *p_msg)
{
    cdb_node_t *p_tbl_node = NULL;
    cdb_node_t *p_ds_node  = NULL;
    cdb_node_t *p_ds2_node = NULL;

    void *p_tbl = NULL;
    void *p_ds  = NULL;
    void *p_ds2 = NULL;
    uint32 only_key = FALSE;
    int32 len = 0;
    
/* internal data length
    1. ADD
    TBL:        TBL_data_size
    TBL+DS:     TBL_key_size + DS_data_size
    TBL+DS+DS2: TBL_key_size + DS_data_size + DS2_data_size
    2. DEL
    TBL:        TBL_key_size
    TBL+DS:     TBL_key_size + DS_data_size
    TBL+DS+DS2: TBL_key_size + DS_data_size + DS2_data_size
    3. SET
    TBL:        TBL_data_size
    TBL+DS:     TBL_key_size + DS_data_size
    TBL+DS+DS2: TBL_key_size + DS_data_size + DS2_data_size
*/

    /* 1. get node */
    p_tbl_node = cdb_get_tbl(p_msg->u_hdr.data.tbl_id);
    if (NULL == p_tbl_node)
    {
        return PM_E_FAIL;
    }
    
    if (CDB_NODE_DS == p_msg->u_hdr.data.type)
    {
        p_ds_node = cdb_get_ds(p_msg->u_hdr.data.ds_id);
        if (NULL == p_ds_node)
        {
            return PM_E_FAIL;
        }
        if (0 != p_msg->u_hdr.data.ds2_id && INVALID_CDB_ID != p_msg->u_hdr.data.ds2_id)
        {
            p_ds2_node = cdb_get_ds(p_msg->u_hdr.data.ds2_id);
            if (NULL == p_ds2_node)
            {
                return PM_E_FAIL;
            }
        }
    }

    p_tbl = p_msg->data;
    
    if (p_ds_node)
    {
        p_ds = p_tbl + p_tbl_node->tbl_info->key_size;
        if (p_ds2_node)
        {
            /* TBL_key_size + DS_data_size + DS2_data_size */
            p_ds2 = p_ds + p_ds_node->tbl_info->data_size;
            len = pb_ds_pack(p_ds2_node->id, p_tbl, p_ds, p_ds2, data, data_len);
        }
        else
        {
            /* TBL_key_size + DS_data_size */
            p_ds2 = NULL;
            len = pb_ds_pack(p_ds_node->id, p_tbl, p_ds, p_ds2, data, data_len);
        }
    }
    else
    {
        /* table */
        if (CDB_OPER_ADD == p_msg->u_hdr.data.oper || CDB_OPER_SET == p_msg->u_hdr.data.oper)
        {
            /* TBL_data_size */
            only_key = FALSE;
        }
        else if (CDB_OPER_DEL == p_msg->u_hdr.data.oper) 
        {
            /* TBL_key_size */
            only_key = TRUE;
        }
        else
        {
            return PM_E_FAIL;
        }
        len = pb_tbl_pack(p_tbl_node->id, p_tbl, only_key, data, data_len);
    }

    return len;
}

int32
_ccs_dispatch_data_to_subscriber_protobuf(int32 sock, uint32 peer, ctc_msg_t *p_msg)
{
    pb_subscribe_response_t hdr;
    uint8 data[MSG_SIZE];
    int32 hdr_len = 0;
    int32 payload_len = 0;
    int32 data_len = 0;
    ctc_msg_t   resp_msg;
    ctc_msg_t  *p_resp_msg = &resp_msg;
    int32 rc = PM_E_NONE;

    /* not sync refer */
    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        return PM_E_NONE;
    }
    
    /* pack header */
    PM_E_RETURN(pb_msg_to_subscribe_response(p_msg, &hdr));
    hdr_len = (pb_subscribe_response_pack(&hdr, data, MSG_SIZE));
    if (hdr_len < 0)
    {
        return rc;
    }
    
    /* pack payload */
    payload_len = _ccs_protobuf_pack_payload(data + hdr_len, MSG_SIZE - hdr_len, p_msg);
    if (hdr_len < 0)
    {
        return rc;
    }    

    data_len = hdr_len + payload_len;
    sal_memset(p_resp_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_resp_msg, MSG_OPER_SUB_RESP, data_len);
    p_resp_msg->data_len = data_len;
    p_resp_msg->data = data;
    
    rc = ctc_sock_send(sock, peer, p_resp_msg);
    return rc;
}

static int32
_ccs_subscriber_tx_error_process(int32 sock, uint32 peer, uint32 tx_error_cnt)
{
    log_sys(M_MOD_LIB, E_CRITICAL, "CCS send to subscriber error count exceed threshold, close SNMP subscriber socket");
    
    ctc_sock_session_close(sock, peer);

    return PM_E_NONE;
}

static int32
_ccs_dispatch_data_to_subscriber(ccs_subscribe_hdr_t *p_sub_hdr, ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    uint32 tx_error_cnt = 0;
    int32 rc = PM_E_NONE;

    if (INVALID_SOCK_ID == lib_master->sub_cdb_sock)
    {
        return PM_E_NONE;
    }

    if ((CDB_SUB_FORMAT_RAW == p_sub_hdr->format) || (CDB_SUB_FORMAT_RAWSNMP == p_sub_hdr->format))
    {
        rc = ctc_sock_send(lib_master->sub_cdb_sock, p_sub_hdr->peer, p_msg);
    }
    else if (CDB_SUB_FORMAT_PROTOBUF == p_sub_hdr->format)
    {
        rc = _ccs_dispatch_data_to_subscriber_protobuf(lib_master->sub_cdb_sock, p_sub_hdr->peer, p_msg);
        if (rc < 0)
        {
            tx_error_cnt = ctc_sock_get_tx_error_count(lib_master->sub_cdb_sock, p_sub_hdr->peer);
            log_sys(M_MOD_LIB, E_ERROR, "CCS send to subscriber error count %u", tx_error_cnt);
            if (tx_error_cnt >= CCS_SUBSCRIBER_TX_ERROR_MAX)
            {
                _ccs_subscriber_tx_error_process(lib_master->sub_cdb_sock, p_sub_hdr->peer, tx_error_cnt);
            }
        }
    }

    return rc;
}

static int32
_ccs_build_tbl_data_to_pm(ctc_msg_t *p_msg, uint8 oper,
                         uint16 tbl_id, uint32 field_id,
                         void *p_data, uint32 data_len)
{
    sal_memset(p_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_msg, MSG_OPER_DATA, data_len);
    p_msg->u_hdr.data.oper = oper;
    p_msg->u_hdr.data.type = CDB_NODE_TBL;
    p_msg->u_hdr.data.tbl_id = tbl_id;
    p_msg->u_hdr.data.ds_id = INVALID_CDB_ID;
    p_msg->u_hdr.data.ds2_id = INVALID_CDB_ID;
    p_msg->u_hdr.data.fields[0] = field_id;
    p_msg->data_len = data_len;
    p_msg->data = p_data;

    return PM_E_NONE;
}

static int32
_ccs_build_ds_data_to_pm(ctc_msg_t *p_msg, uint8 oper,
                         uint16 tbl_id, uint16 ds_id, uint32 field_id,
                         void *p_data, uint32 data_len)
{
    sal_memset(p_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_msg, MSG_OPER_DATA, data_len);
    p_msg->u_hdr.data.oper = oper;
    p_msg->u_hdr.data.type = CDB_NODE_DS;
    p_msg->u_hdr.data.tbl_id = tbl_id;
    p_msg->u_hdr.data.ds_id = ds_id;
    p_msg->u_hdr.data.ds2_id = INVALID_CDB_ID;
    p_msg->u_hdr.data.fields[0] = field_id;
    p_msg->data_len = data_len;
    p_msg->data = p_data;

    return PM_E_NONE;
}

static int32
_cds_sync_add_one_tbl_iter(void *obj, tbl_iter_args_t *pargs)
{
    uint8 format_obj[MSG_SIZE];
    ctc_msg_t msg;
    ctc_msg_t *p_msg = &msg;
    ccs_subscribe_tbl_t *p_sub_tbl = (ccs_subscribe_tbl_t*)pargs->argv[0];
    cdb_node_t *p_tbl_node = (cdb_node_t*)pargs->argv[1];

    sal_memcpy(format_obj, obj, p_tbl_node->tbl_info->data_size);
    _ccs_build_tbl_data_to_pm(p_msg, CDB_OPER_ADD,
        p_tbl_node->id, 0, format_obj, p_tbl_node->tbl_info->data_size);
    cdb_format_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);
    _ccs_dispatch_data_to_subscriber(&p_sub_tbl->hdr, p_msg);

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_sync_ds_tbl_interface(ccs_subscribe_tbl_t *p_sub_tbl, cdb_node_t *p_node_tbl)
{
    cdb_node_t *p_node_ds = NULL;
    uint8 format_obj[MSG_SIZE];
    ctc_msg_t msg;
    ctc_msg_t *p_msg = &msg;
    void *p_db_tbl = NULL;
    void *p_db_ds = NULL;
    
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;    

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        /* sync DS_LAG */
        p_node_ds = cdb_get_ds(DS_LAG);
        if (p_node_ds)
        {
            if (p_db_if->lag)
            {
                p_db_tbl = (void*)p_db_if;
                p_db_ds = (void*)p_db_if->lag;
                /* TBL_key + DS_data */
                sal_memcpy(format_obj, p_db_tbl, p_node_tbl->tbl_info->key_size);
                sal_memcpy((format_obj+p_node_tbl->tbl_info->key_size), p_db_ds, p_node_ds->tbl_info->data_size);
                _ccs_build_ds_data_to_pm(p_msg, CDB_OPER_ADD,
                    p_node_tbl->id, p_node_ds->id, 0, 
                    format_obj, (p_node_tbl->tbl_info->key_size+p_node_ds->tbl_info->data_size));
                cdb_format_ds(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_node_tbl, p_node_ds, NULL,
                    p_msg->data, p_msg->data_len);
                _ccs_dispatch_data_to_subscriber(&p_sub_tbl->hdr, p_msg);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_sync_ds_tbl_route_if(ccs_subscribe_tbl_t *p_sub_tbl, cdb_node_t *p_node_tbl)
{
    cdb_node_t *p_node_ds = NULL;
    uint8 format_obj[MSG_SIZE];
    ctc_msg_t msg;
    ctc_msg_t *p_msg = &msg;
    void *p_db_tbl = NULL;
    void *p_db_ds = NULL;
    
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;
    ctclib_slistnode_t *listnode = NULL;    
    ctclib_slistnode_t *listnode1 = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
    {
        /* sync DS_CONNECTED */
        p_node_ds = cdb_get_ds(DS_CONNECTED);
        if (p_node_ds)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode1)
            {
                p_db_tbl = (void*)p_db_rtif;
                p_db_ds = (void*)p_db_conn;
                /* TBL_key + DS_data */
                sal_memcpy(format_obj, p_db_tbl, p_node_tbl->tbl_info->key_size);
                sal_memcpy((format_obj+p_node_tbl->tbl_info->key_size), p_db_ds, p_node_ds->tbl_info->data_size);
                _ccs_build_ds_data_to_pm(p_msg, CDB_OPER_ADD,
                    p_node_tbl->id, p_node_ds->id, 0, 
                    format_obj, (p_node_tbl->tbl_info->key_size+p_node_ds->tbl_info->data_size));
                cdb_format_ds(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_node_tbl, p_node_ds, NULL,
                    p_msg->data, p_msg->data_len);
                _ccs_dispatch_data_to_subscriber(&p_sub_tbl->hdr, p_msg);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_sync_ds(ccs_subscribe_tbl_t *p_sub_tbl, cdb_node_t* p_node_tbl)
{
    switch (p_node_tbl->id)
    {
    case TBL_INTERFACE:
        return _ccs_sub_tbl_sync_ds_tbl_interface(p_sub_tbl, p_node_tbl);
    case TBL_ROUTE_IF:
        return _ccs_sub_tbl_sync_ds_tbl_route_if(p_sub_tbl, p_node_tbl);
    default:
        break;
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_sync_one_tbl(ccs_subscribe_tbl_t *p_sub_tbl, uint32 tbl_id)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    tbl_iter_args_t args;

    if (NULL == p_node)
    {
        return PM_E_NONE;
    }

    args.argv[0] = p_sub_tbl;
    args.argv[1] = p_node;
    p_node->tbl_info->ops.iter_fn(_cds_sync_add_one_tbl_iter, &args);

    _ccs_sub_tbl_sync_ds(p_sub_tbl, p_node);

    return PM_E_NONE;
}

static int32
_ccs_sub_cdb_add(ctc_msg_t *p_msg)
{
    ccs_subscribe_cdb_t *p_sub_cdb = NULL;

    if (!g_sub_cdb_list)
    {
        return PM_E_NOT_INITED;
    }

    p_sub_cdb = XCALLOC(MEM_CDS_SUB_CDB, sizeof(ccs_subscribe_cdb_t));
    p_sub_cdb->hdr.type = p_msg->u_hdr.sub.type;
    p_sub_cdb->hdr.format = CDB_SUB_FORMAT_RAW;
    p_sub_cdb->hdr.peer = SOCK_BUILD_PEER(p_msg->u_hdr.sub.pid, p_msg->u_hdr.sub.src);
    p_sub_cdb->node_type = p_msg->u_hdr.sub.node_type;
    p_sub_cdb->tbl_id = p_msg->u_hdr.sub.tbl_id;
    p_sub_cdb->field = p_msg->u_hdr.sub.field;

    if (NULL == p_msg->data || 0 == sal_strlen(p_msg->data) || 0 == sal_strcmp("*", p_msg->data))
    {
        p_sub_cdb->tbl_key_valid = FALSE;
        sal_strncpy(p_sub_cdb->tbl_key, "*", CMD_BUF_256);
    }
    else
    {
        p_sub_cdb->tbl_key_valid = TRUE;
        sal_strncpy(p_sub_cdb->tbl_key, p_msg->data, CMD_BUF_256);
        p_sub_cdb->tbl_key[p_msg->data_len] = CMD_ZERO_CHAR;
    }
    ctclib_slistnode_insert_sort(g_sub_cdb_list, p_sub_cdb);
    
    return PM_E_NONE;
}

static int32
_ccs_subscribe_del(ctc_sock_session_t *p_session)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    ccs_subscribe_cdb_t *p_sub_cdb = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(g_sub_cdb_list, p_sub_cdb, node, nnext)
    {
        if (p_sub_cdb->hdr.peer == p_session->peer)
        {
            ctclib_slistnode_delete(g_sub_cdb_list, p_sub_cdb);
            if (CDB_SUB_CDB == p_sub_cdb->hdr.type)
            {
                XFREE(MEM_CDS_SUB_CDB, p_sub_cdb);
            }
            else
            {
                XFREE(MEM_CDS_SUB_TBL, p_sub_cdb);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_cdb_process_req(int32 sock, ctc_msg_t *p_msg)
{
    cdb_node_t *p_node = NULL;
    uint32 peer_pid = (p_msg->peer >> 16) & 0xFFFF;
    
    if (CDB_NODE_DIR == p_msg->u_hdr.sub.node_type)
    {
        p_node = cdb_get_node_by_id(CDB_NODE_DIR, p_msg->u_hdr.sub.tbl_id);
        log_sys(M_MOD_CDS, E_ERROR, "subscribe path %s, peer %u, dir_id %u, key %s\n", p_node->full_name, 
            peer_pid, p_msg->u_hdr.sub.tbl_id, p_msg->data);
    }
    else if (CDB_NODE_TBL == p_msg->u_hdr.sub.node_type)
    {
        p_node = cdb_get_node_by_id(CDB_NODE_TBL, p_msg->u_hdr.sub.tbl_id);
        log_sys(M_MOD_CDS, E_ERROR, "subscribe path %s, peer %u, tbl_id %u, key %s\n", p_node->full_name, 
            peer_pid, p_msg->u_hdr.sub.tbl_id, p_msg->data);
    }
    else
    {
        p_node = cdb_get_node_by_id(CDB_NODE_DS, p_msg->u_hdr.sub.ds_id);
        log_sys(M_MOD_CDS, E_ERROR, "subscribe path %s, peer %u, tbl_id %u, ds id %u, key %s\n", p_node->full_name, 
            peer_pid, p_msg->u_hdr.sub.tbl_id, p_msg->u_hdr.sub.ds_id, p_msg->data);
    }

    _ccs_sub_cdb_add(p_msg);

    return PM_E_NONE;
}

static int32
_ccs_sub_cdb_add_new(Cdb__SubscribeRequest *p_req)
{
    ccs_subscribe_cdb_t *p_sub_cdb = NULL;

    if (!g_sub_cdb_list)
    {
        return PM_E_NOT_INITED;
    }

    p_sub_cdb = XCALLOC(MEM_CDS_SUB_CDB, sizeof(ccs_subscribe_cdb_t));
    p_sub_cdb->hdr.type = p_req->type;
    p_sub_cdb->hdr.format = p_req->format;
    p_sub_cdb->hdr.sync = FALSE;
    p_sub_cdb->hdr.peer = SOCK_BUILD_PEER(p_req->pid, p_req->src);
    p_sub_cdb->node_type = p_req->cdb->node_type;
    p_sub_cdb->tbl_id = p_req->cdb->tbl_id;
    p_sub_cdb->field = p_req->cdb->field;
    
    if (NULL == p_req->cdb->key || 0 == sal_strlen(p_req->cdb->key ) || 0 == sal_strcmp("*", p_req->cdb->key))
    {
        p_sub_cdb->tbl_key_valid = FALSE;
        sal_strncpy(p_sub_cdb->tbl_key, "*", CMD_BUF_256);
    }
    else
    {
        p_sub_cdb->tbl_key_valid = TRUE;
        sal_strncpy(p_sub_cdb->tbl_key, p_req->cdb->key , CMD_BUF_256);
    }
    ctclib_slistnode_insert_sort(g_sub_cdb_list, p_sub_cdb);

    return PM_E_NONE;
}

static int32
_ccs_sub_cdb_process_req_new(int32 sock, Cdb__SubscribeRequest *p_req)
{
    _ccs_sub_cdb_add_new(p_req);

    return PM_E_NONE;
}

static int32
_ccs_sub_cdb_check_condition(ccs_subscribe_cdb_t *p_sub_cdb, cdb_node_t *p_tbl_node, cdb_node_t *p_ds_node, ctc_msg_t *p_msg)
{
    ctclib_list_node_t* p_listnode = NULL;
    cdb_node_t *p_dir = NULL;
    cdb_node_t *p_sub_tbl = NULL;
    uint32 key_check = FALSE;
    uint32 fld_check = FALSE;
    char key_str[CMD_BUF_256];
    
    switch (p_sub_cdb->node_type)
    {
    case CDB_NODE_DIR:
        if (DIR_CDB == p_sub_cdb->tbl_id)
        {
            return PM_E_NONE;
        }

        p_dir = cdb_get_dir(p_sub_cdb->tbl_id);
        ctclib_list_for_each(p_listnode, &p_dir->sub)
        {
            p_sub_tbl = ctclib_container_of(p_listnode, cdb_node_t, node);
            if (CDB_NODE_TBL == p_sub_tbl->type && p_sub_tbl->id == p_tbl_node->id)
            {
                return PM_E_NONE;
            }
        }
        break;

    case CDB_NODE_TBL:
        if (p_sub_cdb->tbl_id == p_tbl_node->id)
        {
            if (p_sub_cdb->field != FLD_ID_ALL)
            {
                if (CDB_OPER_SET == p_msg->u_hdr.data.oper
                 && p_sub_cdb->field == p_msg->u_hdr.sub.field)
                {
                    fld_check = TRUE;
                }
            }
            else
            {
                fld_check = TRUE;
            }

            if (p_sub_cdb->tbl_key_valid)
            {
                p_tbl_node->tbl_info->ops.key_v2s_fn(p_msg->data, key_str, sizeof(key_str));
                if (0 == sal_strcmp(p_sub_cdb->tbl_key, key_str))
                {
                    key_check = TRUE;
                }
            }
            else
            {
                key_check = TRUE;
            }

            if (fld_check && key_check)
            {
                return PM_E_NONE;
            }
        }
        break;

    case CDB_NODE_DS:
        break;
        
    default:
        break;
    }

    return PM_E_FAIL;
}

static int32
_ccs_sub_tbl_check_condition(ccs_subscribe_tbl_t *p_sub_tbl, cdb_node_t *p_tbl_node, cdb_node_t *p_ds_node, ctc_msg_t *p_msg)
{
    uint32 i = 0;

    for (i = 0; i < p_sub_tbl->tbl_num; i++)
    {
        if (p_sub_tbl->tbl_id[i] == p_tbl_node->id)
        {
            /* Added by kcao for bug 39331, not send tbl_interface.ds_xxx for vlan range cmds will stress on it */
            if (CDB_SUB_FORMAT_PROTOBUF == p_sub_tbl->hdr.format && TBL_INTERFACE == p_tbl_node->id)
            {
                if (p_ds_node)
                {
                    if (DS_LAG == p_ds_node->id)
                    {
                        return PM_E_NONE;
                    }
                    return PM_E_FAIL;
                }
                else
                {
                    if (CDB_OPER_SET == p_msg->u_hdr.data.oper)
                    {
                        /* Added by kcao for bug 39667, only sync those field to SNMP */
                        if ((TBL_INTERFACE_FLD_KEY              == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_IFINDEX          == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_DESC             == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_HW_TYPE          == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_SPEED            == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_DUPLEX           == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_OPER_SPEED       == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_OPER_DUPLEX      == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_OPER_BANDWIDTH   == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_MAC_ADDR         == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_HW_MAC_ADDR      == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_RUNNING          == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_EGS_ACL          == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_ADMIN_UP         == p_msg->u_hdr.data.fields[0])
                         || (TBL_INTERFACE_FLD_MTU              == p_msg->u_hdr.data.fields[0]))
                        {
                            return PM_E_NONE;
                        }
                        else
                        {
                            return PM_E_FAIL;
                        }
                    }
                    
                    return PM_E_NONE;
                }
            }
            else if (CDB_SUB_FORMAT_PROTOBUF == p_sub_tbl->hdr.format && TBL_FIBER == p_tbl_node->id)
            {
                if (CDB_OPER_SET == p_msg->u_hdr.data.oper)
                {
                    /* Added by kcao for bug 39667, only sync those field to SNMP */
                    if ((TBL_FIBER_FLD_MAX == p_msg->u_hdr.data.fields[0]))
                    {
                        return PM_E_NONE;
                    }
                    else
                    {
                        return PM_E_FAIL;
                    }
                }
                if (CDB_OPER_DEL == p_msg->u_hdr.data.oper && TBL_FIBER == p_tbl_node->id)
                {
                    return PM_E_NONE;
                }
            }
            else if (CDB_SUB_FORMAT_PROTOBUF == p_sub_tbl->hdr.format && TBL_PSU == p_tbl_node->id)
            {
                return PM_E_NONE; 
            }
            else
            {
                return PM_E_NONE;
            }
        }
    }

    return PM_E_FAIL;
}

static int32
_ccs_subscribe_dispatch(cdb_node_t *p_tbl_node, cdb_node_t *p_ds_node, ctc_msg_t *p_msg)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    ccs_subscribe_cdb_t *p_sub_cdb = NULL;
    ccs_subscribe_tbl_t *p_sub_tbl = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP_DEL(g_sub_cdb_list, p_sub_cdb, node, nnext)
    {
        if (CDB_SUB_CDB == p_sub_cdb->hdr.type)
        {
            rc = _ccs_sub_cdb_check_condition(p_sub_cdb, p_tbl_node, p_ds_node, p_msg);
        }
        else
        {
            p_sub_tbl = (ccs_subscribe_tbl_t*)p_sub_cdb;
            rc = _ccs_sub_tbl_check_condition(p_sub_tbl, p_tbl_node, p_ds_node, p_msg);
        }
        if (rc < 0)
        {
            continue;
        }

        _ccs_dispatch_data_to_subscriber(&p_sub_cdb->hdr, p_msg);
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_update(ccs_subscribe_tbl_t *p_sub_tbl, ctc_msg_t *p_msg)
{
    uint32 *p_tbl_id = NULL;
    uint32 update_tbl_num = 0;
    uint32 update_idx = 0;
    uint32 update_tbl_id = 0;
    uint32 i = 0;
    uint32 duplicate = FALSE;
    
    p_tbl_id = (uint32*)p_msg->data;
    update_tbl_num = p_msg->u_hdr.sub.tbl_num;

    for (update_idx = 0; update_idx < update_tbl_num; update_idx++)
    {
        duplicate = FALSE;
        update_tbl_id = p_tbl_id[update_idx];

        for (i = 0; i < p_sub_tbl->tbl_num; i++)
        {
            if (p_sub_tbl->tbl_id[i] == update_tbl_id)
            {
                duplicate = TRUE;
                break;
            }
        }

        if (duplicate)
        {
            continue;
        }

        p_sub_tbl->tbl_id[p_sub_tbl->tbl_num] = update_tbl_id;
        p_sub_tbl->tbl_num++;

        if (p_sub_tbl->hdr.sync)
        {
            _ccs_sub_tbl_sync_one_tbl(p_sub_tbl, update_tbl_id);
        }
    }
    
    return PM_E_NONE;
}

#define CCS_SUB_SOCK_BUFFER_SIZE    20000000

static int32
_ccs_sub_tbl_add(ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    uint32 *p_tbl_id = NULL;
    ccs_subscribe_hdr_t hdr;
    ccs_subscribe_tbl_t *p_sub_tbl = NULL;
    uint32 i = 0;

    if (!g_sub_cdb_list)
    {
        return PM_E_NOT_INITED;
    }

    hdr.type = p_msg->u_hdr.sub.type;
    hdr.sync = p_msg->u_hdr.sub.node_type;
    hdr.format = (p_msg->u_hdr.sub.tbl_id) ? CDB_SUB_FORMAT_RAWSNMP : CDB_SUB_FORMAT_RAW;
    hdr.peer = SOCK_BUILD_PEER(p_msg->u_hdr.sub.pid, p_msg->u_hdr.sub.src);

    p_sub_tbl = ctclib_slistdata_lookup(g_sub_cdb_list, &hdr);
    if (p_sub_tbl)
    {
        if (0xFF == p_msg->u_hdr.sub.node_type)
        {
            ccs_sub_tbls_sync(p_sub_tbl);
        }
        else
        {
            p_sub_tbl->hdr.sync = p_msg->u_hdr.sub.node_type;
            _ccs_sub_tbl_update(p_sub_tbl, p_msg);
        }
    }
    else
    {
        p_sub_tbl = XCALLOC(MEM_CDS_SUB_TBL, sizeof(ccs_subscribe_cdb_t));
        p_sub_tbl->hdr.type = hdr.type;
        p_sub_tbl->hdr.sync = hdr.sync;
        p_sub_tbl->hdr.format = hdr.format;
        p_sub_tbl->hdr.peer = hdr.peer;
        p_sub_tbl->tbl_num = p_msg->u_hdr.sub.tbl_num;
        p_tbl_id = (uint32*)p_msg->data;

        if (p_msg->data_len == 0)
        {
            p_sub_tbl->tbl_num = 0;
        }

        for (i = 0; i < p_sub_tbl->tbl_num; i++)
        {
            p_sub_tbl->tbl_id[i] = p_tbl_id[i];
        }

        ctclib_slistnode_insert_sort(g_sub_cdb_list, p_sub_tbl);

        if (p_sub_tbl->hdr.sync)
        {
            ccs_sub_tbls_sync(p_sub_tbl);
        }

        if ((CDB_SUB_TBL == p_sub_tbl->hdr.type) && (CDB_SUB_FORMAT_PROTOBUF == p_sub_tbl->hdr.format))
        {
            ctc_sock_session_set_buf(lib_master->sub_cdb_sock, p_sub_tbl->hdr.peer, 0, CCS_SUB_SOCK_BUFFER_SIZE);
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_process_req(int32 sock, ctc_msg_t *p_msg)
{
    _ccs_sub_tbl_add(p_msg);
    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_update_new(ccs_subscribe_tbl_t *p_sub_tbl, Cdb__SubscribeRequest *p_req)
{
    uint32 *p_tbl_id = NULL;
    uint32 update_tbl_num = 0;
    uint32 update_idx = 0;
    uint32 update_tbl_id = 0;
    uint32 i = 0;
    uint32 duplicate = FALSE;
    
    p_tbl_id = p_req->tbl->table;
    update_tbl_num = p_req->tbl->n_table;

    for (update_idx = 0; update_idx < update_tbl_num; update_idx++)
    {
        duplicate = FALSE;
        update_tbl_id = p_tbl_id[update_idx];

        for (i = 0; i < p_sub_tbl->tbl_num; i++)
        {
            if (p_sub_tbl->tbl_id[i] == update_tbl_id)
            {
                duplicate = TRUE;
                break;
            }
        }

        if (duplicate)
        {
            continue;
        }

        p_sub_tbl->tbl_id[p_sub_tbl->tbl_num] = update_tbl_id;
        p_sub_tbl->tbl_num++;

        if (p_sub_tbl->hdr.sync)
        {
            _ccs_sub_tbl_sync_one_tbl(p_sub_tbl, update_tbl_id);
        }
    }
    
    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_add_new(Cdb__SubscribeRequest *p_req)
{
    lib_master_t *lib_master = lib_get_master();
    uint32 *p_tbl_id = NULL;
    ccs_subscribe_hdr_t hdr;
    ccs_subscribe_tbl_t *p_sub_tbl = NULL;
    uint32 i = 0;

    if (!g_sub_cdb_list)
    {
        return PM_E_NOT_INITED;
    }

    hdr.type = p_req->type;
    hdr.format = p_req->format;
    hdr.sync = p_req->tbl->sync;
    hdr.peer = SOCK_BUILD_PEER(p_req->pid, p_req->src);

    p_sub_tbl = ctclib_slistdata_lookup(g_sub_cdb_list, &hdr);
    if (p_sub_tbl)
    {
        p_sub_tbl->hdr.sync = p_req->tbl->sync;
        _ccs_sub_tbl_update_new(p_sub_tbl, p_req);
    }
    else
    {
        p_sub_tbl = XCALLOC(MEM_CDS_SUB_TBL, sizeof(ccs_subscribe_cdb_t));
        p_sub_tbl->hdr.type = hdr.type;
        p_sub_tbl->hdr.format = hdr.format;
        p_sub_tbl->hdr.sync = hdr.sync;
        p_sub_tbl->hdr.peer = hdr.peer;
        p_sub_tbl->tbl_num = p_req->tbl->n_table;
        p_tbl_id = p_req->tbl->table;

        if (0 == p_req->tbl->n_table)
        {
            p_sub_tbl->tbl_num = 0;
        }

        for (i = 0; i < p_sub_tbl->tbl_num; i++)
        {
            p_sub_tbl->tbl_id[i] = p_tbl_id[i];
        }

        ctclib_slistnode_insert_sort(g_sub_cdb_list, p_sub_tbl);

        if (p_sub_tbl->hdr.sync)
        {
            ccs_sub_tbls_sync(p_sub_tbl);
        }

        if ((CDB_SUB_TBL == p_sub_tbl->hdr.type) && (CDB_SUB_FORMAT_PROTOBUF == p_sub_tbl->hdr.format))
        {
            ctc_sock_session_set_buf(lib_master->sub_cdb_sock, p_sub_tbl->hdr.peer, 0, CCS_SUB_SOCK_BUFFER_SIZE);
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_sub_tbl_process_req_new(int32 sock, Cdb__SubscribeRequest *p_req)
{
    _ccs_sub_tbl_add_new(p_req);
    return PM_E_NONE;
}

int32
ccs_process_msg_sub_req(int32 sock, ctc_sock_session_t *p_sess, ctc_msg_t *p_msg)
{
    Cdb__SubscribeRequest *p_req = NULL;
    
    //unpack student info from buf
    p_req = cdb__subscribe_request__unpack(NULL, p_msg->data_len, p_msg->data);

    p_sess->peer = SOCK_BUILD_PEER(p_req->pid, p_req->src);
    if (CDB_SUB_CDB == p_req->type)
    {
        return _ccs_sub_cdb_process_req_new(sock, p_req);
    }
    else
    {
        return _ccs_sub_tbl_process_req_new(sock, p_req);
    }
    
    //free msg
    cdb__subscribe_request__free_unpacked(p_req, NULL);
    
    return PM_E_NONE;
}

static int32
_ccs_store_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
{
    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        if (p_ds_node)
        {
            return cdb_store_ds_refer(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                p_ds_node, p_ds2_node, p_msg->data, p_msg->data_len);
        }
        else
        {
            return cdb_store_tbl_refer(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                p_msg->data, p_msg->data_len);
        }    
    }
    else
    {
        if (p_ds_node)
        {
            return cdb_store_ds(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                    p_ds_node, p_ds2_node, p_msg->data, p_msg->data_len);
        }
        else
        {
            return cdb_store_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);
        }
    }

    return PM_E_NONE;
}

static int32
_ccs_store_tbl_log(cdb_node_t* p_tbl_node, ctc_msg_t *p_msg)
{
    tbl_log_master_t *p_master = tbl_log_get_master();
    tbl_log_t *p_log = (tbl_log_t*)p_msg->data;
    
    cdb_store_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);

    /* update sequence_id */
    p_log->key.sequence_id = (p_master->curr_sequence_id - 1);

    return PM_E_NONE;
}

/*added by yejl for bug 47211, 2019-02-19*/
static int32
_ccs_set_timezone(ctc_msg_t *p_msg)
{
    tbl_clock_t* p_clock = (tbl_clock_t*)p_msg->data;
    if (p_clock)
    {
        _lib_cmd_set_timezone(p_clock);
    }
    
    return PM_E_NONE;
}
/*ended*/

static int32
_ccs_process_data_sync_req(int32 sock, ctc_msg_t *p_msg)
{
    cdb_node_t* p_tbl_node = NULL;
    cdb_node_t* p_ds_node = NULL;
    cdb_node_t* p_ds2_node = NULL;
    int32 rc = PM_E_NONE;

    /* 1. get node */
    p_tbl_node = cdb_get_tbl(p_msg->u_hdr.data.tbl_id);
    if (NULL == p_tbl_node)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Invalid tbl id %u\n", p_msg->u_hdr.data.tbl_id);
        return PM_E_FAIL;
    }
    
    /*added by yejl for bug 47211, 2019-02-19*/
    if ((TBL_CLOCK == p_tbl_node->id) && (TBL_CLOCK_FLD_TIMEZONE_NAME == p_msg->u_hdr.data.fields[0]))
    {
        rc = _ccs_set_timezone(p_msg);
    }
    /*ended*/

    if (CDB_NODE_DS == p_msg->u_hdr.data.type)
    {
        p_ds_node = cdb_get_ds(p_msg->u_hdr.data.ds_id);
        if (NULL == p_ds_node)
        {
            log_sys(M_MOD_CDS, E_ERROR, "Invalid ds id %u\n", p_msg->u_hdr.data.ds_id);
            return PM_E_FAIL;
        }
        if (0 != p_msg->u_hdr.data.ds2_id && INVALID_CDB_ID != p_msg->u_hdr.data.ds2_id)
        {
            p_ds2_node = cdb_get_ds(p_msg->u_hdr.data.ds2_id);
            if (NULL == p_ds2_node)
            {
                log_sys(M_MOD_CDS, E_ERROR, "Invalid ds2 id %u\n", p_msg->u_hdr.data.ds2_id);
                return PM_E_FAIL;
            }
        }
    }

    /* 2. format data, e.g. set pointer to NULL is exec in CDS */

    /* 3. send to subscriber PMs */
    if (p_tbl_node->id == TBL_LOG)
    {
        rc = _ccs_store_tbl_log(p_tbl_node, p_msg);
    }

    /* 4. send to subscribe client */
    rc = _ccs_subscribe_dispatch(p_tbl_node, p_ds_node, p_msg);
    if (rc < 0)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Dispatch data (tbl %u, ds %u) to react client failed\n", 
                        p_msg->u_hdr.data.tbl_id, p_msg->u_hdr.data.ds_id);
        return rc;
    }

    /* 5. store data to CDB */
    if (p_tbl_node->id != TBL_LOG)
    {
        rc = _ccs_store_data(p_tbl_node, p_ds_node, p_ds2_node, p_msg);
    }

    return rc;
}

int32
ccs_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    ctc_sock_session_t *p_sess = NULL;

    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        break;

    case MSG_OPER_CFG:
        if (PM_ID_CTL == p_msg->u_hdr.cfg.src)
        {
            return _ccs_process_cfg_req(sock, p_msg);
        }
        else
        {
            return _ccs_process_cfg_resp(sock, p_msg);
        }

    case MSG_OPER_DATA:
        return _ccs_process_data_sync_req(sock, p_msg);

    case MSG_OPER_SUBSCRIBE:
        p_sess = ctc_sock_get_session_by_fd(sock, sess_fd);
        if (NULL == p_sess)
        {
            return PM_E_FAIL;
        }
        p_sess->peer = SOCK_BUILD_PEER(p_msg->u_hdr.sub.pid, p_msg->u_hdr.sub.src);

        if (CDB_SUB_CDB == p_msg->u_hdr.sub.type)
        {
            return _ccs_sub_cdb_process_req(sock, p_msg);
        }
        else
        {
            return _ccs_sub_tbl_process_req(sock, p_msg);
        }
        break;

    case MSG_OPER_SUB_REQ:
        p_sess = ctc_sock_get_session_by_fd(sock, sess_fd);
        if (NULL == p_sess)
        {
            return PM_E_FAIL;
        }        
        return ccs_process_msg_sub_req(sock, p_sess, p_msg);

    default:
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

static int32
_ccs_cmd_process_get_subscriber_cdb(cfg_show_t *p_resp_show, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    ctclib_slistnode_t *node = NULL;
    ccs_subscribe_cdb_t *p_sub = NULL;
    cdb_node_t *p_node = NULL;
    uint32 peer_pid = 0;
    uint32 i = 0;
    char *field_name_all = "*";
    char *field_name = NULL;
    
    sal_memset(p_resp_show, 0, sizeof(cfg_show_t));
    p_resp_show->banner = TRUE;
    CFG_BUILD_SHOW_PATH(p_resp_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(p_resp_show->path, "w+");

    sal_fprintf(fp, "CDB Table Subscriber Reactors\n");

    sal_fprintf(fp, "%-5s %-5s %-8s %-16s %-16s %-8s %-16s\n", "Index", "Type", "Format", "Name", "Field Name", "Peer PID", "TBL Key");
    sal_fprintf(fp, "-----+-----+--------+----------------+----------------+--------+----------------\n");

    CTCLIB_SLIST_LOOP(g_sub_cdb_list, p_sub, node)
    {
        if (CDB_SUB_TBL == p_sub->hdr.type)
        {
            continue;
        }        

        peer_pid = (p_sub->hdr.peer >> 16) & 0xFFFF;
        switch (p_sub->node_type)
        {
        case CDB_NODE_DIR:
            field_name = field_name_all;
            p_node = cdb_get_dir(p_sub->tbl_id);
            sal_fprintf(fp, "%-5u %-5s %-8s %-16s %-16s %-8u %-16s\n", i, "DIR",
                cdb_enum_val2str(cdb_sub_format_str, CDB_SUB_FORMAT_MAX, p_sub->hdr.format),
                p_node->name, field_name, peer_pid, p_sub->tbl_key);
            break;

        case CDB_NODE_TBL:
            p_node = cdb_get_tbl(p_sub->tbl_id);
            if (FLD_ID_ALL == p_sub->field)
            {
                field_name = field_name_all;
            }
            else
            {
                field_name = p_node->tbl_info->field[p_sub->field].name;
            }
            sal_fprintf(fp, "%-5u %-5s %-8s %-16s %-16s %-8u %-16s\n", i, "TBL",
                cdb_enum_val2str(cdb_sub_format_str, CDB_SUB_FORMAT_MAX, p_sub->hdr.format),
                p_node->name, field_name, peer_pid, p_sub->tbl_key);
            break;

        default:
            break;
        }
        i++;
    }
                    
    sal_fprintf(fp, "\n");
    
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_ccs_cmd_process_get_subscriber_tbl(cfg_show_t *p_resp_show, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    ctclib_slistnode_t *node = NULL;
    ccs_subscribe_tbl_t *p_sub = NULL;
    cdb_node_t *p_node = NULL;
    uint32 peer_pid = 0;
    uint32 i = 0;
    
    sal_memset(p_resp_show, 0, sizeof(cfg_show_t));
    p_resp_show->banner = TRUE;
    CFG_BUILD_SHOW_PATH(p_resp_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(p_resp_show->path, "w+");

    sal_fprintf(fp, "CDB Table Subscriber Reactors\n");

    sal_fprintf(fp, "%-8s %-6s %-8s %-6s %-40s\n", "Peer PID", "Sync", "Format", "Number", "Tables");
    sal_fprintf(fp, "--------+------+--------+------+------------------------------------------------\n");

    CTCLIB_SLIST_LOOP(g_sub_cdb_list, p_sub, node)
    {
        if (CDB_SUB_CDB == p_sub->hdr.type)
        {
            continue;
        }

        peer_pid = (p_sub->hdr.peer >> 16) & 0xFFFF;
        sal_fprintf(fp, "%-8u %-6u %-8s %-6u ", 
            peer_pid, p_sub->hdr.sync, cdb_enum_val2str(cdb_sub_format_str, CDB_SUB_FORMAT_MAX, p_sub->hdr.format), p_sub->tbl_num);
        for (i = 0; i < p_sub->tbl_num; i++)
        {
            p_node = cdb_get_tbl(p_sub->tbl_id[i]);
            if (p_node)
            {
                sal_fprintf(fp, "%s ", p_node->name);
            }
            else
            {
                sal_fprintf(fp, "Unknown TBL %u ", p_sub->tbl_id[i]);
            }
        }
        sal_fprintf(fp, "\n");
    }
                    
    sal_fprintf(fp, "\n");
    
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
_ccs_subscribe_cmp(void *data1, void *data2)
{
    ccs_subscribe_hdr_t *sub1 = (ccs_subscribe_hdr_t*)data1;
    ccs_subscribe_hdr_t *sub2 = (ccs_subscribe_hdr_t*)data2;

    if (sub1->type == sub2->type)
    {
        return sub1->peer - sub2->peer;
    }
    return sub1->type - sub2->type;
}

int32
ccs_cmd_process_subscriber(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == sal_strcmp("cdb", argv[0]))
        {
            rc = _ccs_cmd_process_get_subscriber_cdb(para->p_show, para);
        }
        else if (0 == sal_strcmp("table", argv[0]))
        {
            rc = _ccs_cmd_process_get_subscriber_tbl(para->p_show, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter %s", argv[0]);
        }
        break;
        
    default:
        rc = PM_E_INVALID_PARAM;
        break;
    }

    return rc;
}

int32
ccs_cmd_process_check_openflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    switch (para->oper)
    {
    case CDB_OPER_SET:
        rc = PM_E_NONE;
        break;
        
    default:
        rc = PM_E_INVALID_PARAM;
        break;
    }

    return rc;
}

int32
ccs_register_cmd_cb()
{
    cdb_register_cfg_act_cb(ACT_DUMP_CCS_SUBSCRIBER, ccs_cmd_process_subscriber);
    cdb_register_cfg_act_cb(ACT_CHECK_OPENFLOW,     ccs_cmd_process_check_openflow);
    return PM_E_NONE;
}

int32
ccs_subscribe_init(lib_master_t *master)
{
    g_sub_cdb_list = ctclib_slist_create(_ccs_subscribe_cmp, NULL);
    ctc_sock_register_sess_del_fn(master->sub_cdb_sock, _ccs_subscribe_del);

    return PM_E_NONE;
}

int32
ccs_sock_init(lib_master_t *master)
{
    int32 sock = INVALID_SOCK_ID;
    master->cfg_sock = INVALID_SOCK_ID;
    master->data_sock = INVALID_SOCK_ID;
    master->sub_cdb_sock = INVALID_SOCK_ID;
    master->sub_tbl_sock = INVALID_SOCK_ID;
    master->trap_sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;

    /* CDS config socket */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CcsSrvCfg");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_sock_set_pmid(sock, master->pm_id);
    ctc_sock_bind(sock, SOCK_ADDR_CCS);
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

    /* CDS subscribe socket */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CcsSrvSubCDB");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_sock_set_pmid(sock, master->pm_id);
    ctc_sock_bind(sock, SOCK_ADDR_CCS_SUB_CDB);
    master->sub_cdb_sock = sock;

    ipc_connect_bhm();

    return PM_E_NONE;
}

int32
ccs_sub_tbls_sync(ccs_subscribe_tbl_t *p_sub_tbl)
{
    uint32 i = 0;

    for (i = 0; i < p_sub_tbl->tbl_num; i++)
    {
        _ccs_sub_tbl_sync_one_tbl(p_sub_tbl, p_sub_tbl->tbl_id[i]);
    }

    return PM_E_NONE;
}

char*
ccs_sys_load_state_str(sys_load_state_t state)
{
    switch (state)
    {
    case SYS_LOAD_NORMAL:
        return "normal";
    case SYS_LOAD_WARNING:
        return "warning";
    case SYS_LOAD_ALERT:
        return "alert";
    case SYS_LOAD_CRITICAL:
        return "critical";
    default:
        return "invalid";
    }
}

static int32
_ccs_sys_load_upate_state(sys_load_state_t state)
{
    tbl_sys_load_t *p_db_load = tbl_sys_load_get_sys_load();
    FILE *fp = NULL;

    switch (state)
    {
    case SYS_LOAD_NORMAL:
        logid_sys(LOG_SYSTEM_4_LOAD_NORMAL, ccs_sys_load_state_str(g_sys_load_ctl.state));
        break;
    case SYS_LOAD_WARNING:
        logid_sys(LOG_SYSTEM_3_LOAD_WARNING, ccs_sys_load_state_str(g_sys_load_ctl.state));
        break;
    case SYS_LOAD_ALERT:
        logid_sys(LOG_SYSTEM_2_LOAD_ALERT, ccs_sys_load_state_str(g_sys_load_ctl.state));
        break;
    case SYS_LOAD_CRITICAL:
        logid_sys(LOG_SYSTEM_1_LOAD_CRITICAL, ccs_sys_load_state_str(g_sys_load_ctl.state));
        break;
    default:

        return PM_E_INVALID_PARAM;
    }
    g_sys_load_ctl.state = state;

    fp = sal_fopen(SYS_LOAD_STATE_FILE, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    if (p_db_load->cli_back_pressure_en)
    {
        sal_fprintf(fp, "%u", state);
    }
    else
    {
        sal_fprintf(fp, "%u", 0);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

uint32 debug_warning_duration = FALSE;
uint32 EMERGENCY_CHECK_TIME = 60;
    
static void
_ccs_sys_load_timer_func(void *p_data)
{
    tbl_sys_load_t *p_db_load = tbl_sys_load_get_sys_load();
    sys_load_state_t new_state = SYS_LOAD_NORMAL;

    if (p_db_load->sys_load_en)
    {
        if (p_db_load->cdb_queue_depth_switch   > p_db_load->cdb_queue_warning_threshold
         || p_db_load->cdb_queue_depth_routed   > p_db_load->cdb_queue_warning_threshold
         || p_db_load->cdb_queue_depth_cds      > p_db_load->cdb_queue_warning_threshold
         || p_db_load->cdb_queue_depth_fea      > p_db_load->cdb_queue_warning_threshold)
        {
            new_state = SYS_LOAD_ALERT;
        }
        else if (p_db_load->cdb_queue_depth_switch   > p_db_load->cdb_queue_notice_threshold
         || p_db_load->cdb_queue_depth_routed   > p_db_load->cdb_queue_notice_threshold
         || p_db_load->cdb_queue_depth_cds      > p_db_load->cdb_queue_notice_threshold
         || p_db_load->cdb_queue_depth_fea      > p_db_load->cdb_queue_notice_threshold)
        {
            new_state = SYS_LOAD_WARNING;
        }
        else
        {
            new_state = SYS_LOAD_NORMAL;
        }
    }
    else
    {
        new_state = SYS_LOAD_NORMAL;
    }

    if (SYS_LOAD_CRITICAL == g_sys_load_ctl.state)
    {
        if (SYS_LOAD_ALERT == new_state)
        {
            goto START_TIME;
        }
    }

    if (SYS_LOAD_ALERT == new_state)
    {
        g_sys_load_ctl.warning_duration++;

        if (debug_warning_duration)
        {
            log_sys(M_MOD_CFG, E_DEBUG, "System load in warning %u times", g_sys_load_ctl.warning_duration);
        }
        if (g_sys_load_ctl.warning_duration >= EMERGENCY_CHECK_TIME)
        {
            new_state = SYS_LOAD_CRITICAL;
            g_sys_load_ctl.warning_duration = 0;
        }
    }
    else
    {
        g_sys_load_ctl.warning_duration = 0;
    }

    if (new_state != g_sys_load_ctl.state)
    {
        if (SYS_LOAD_CRITICAL == g_sys_load_ctl.state)
        {
            /* only normal can release from critical */
            if (SYS_LOAD_NORMAL == new_state)
            {
                _ccs_sys_load_upate_state(new_state);
            }
        }
        else
        {
            _ccs_sys_load_upate_state(new_state);
        }
    }

START_TIME:
    g_sys_load_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _ccs_sys_load_timer_func, NULL, SYS_LOAD_UPDATE_INTERVAL);

    return;
}

int32
ccs_sys_load_init()
{
    g_sys_load_ctl.cli_sys_load_en  = TRUE;
    g_sys_load_ctl.state            = SYS_LOAD_NORMAL;
    _ccs_sys_load_upate_state(SYS_LOAD_NORMAL);
    g_sys_load_ctl.warning.switchd  = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE;
    g_sys_load_ctl.warning.routed   = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE;
    g_sys_load_ctl.warning.cds      = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE;
    g_sys_load_ctl.warning.fea      = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE;
    g_sys_load_ctl.critical.switchd  = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING;
    g_sys_load_ctl.critical.routed   = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING;
    g_sys_load_ctl.critical.cds      = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING;
    g_sys_load_ctl.critical.fea      = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING;
    
    g_sys_load_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _ccs_sys_load_timer_func, NULL, SYS_LOAD_UPDATE_INTERVAL);
    
    return PM_E_NONE;
}

int32
ccs_init()
{ 
    lib_master_t *lib_master = lib_init(PM_ID_CCS);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif    
    ccs_sock_init(lib_master);
    ccs_subscribe_init(lib_master);
    ctc_sock_register_rx_fn(ccs_process_msg);
    ccs_register_cmd_cb();
    ccs_sys_load_init();
    ccs_lic_init();
    
    ctc_task_main();

    return PM_E_NONE;
}


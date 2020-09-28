/* cds - central database service
 * 
 */

#include "proto.h"
#include "ctc_task.h"
#include "cds.h"
#include "cds_db.h"
#include "cdb_store.h"
#include "cfg_cmd.h"
#include "gen/cdb_dest_pm_priv.h"
#include "gen/cdb_inc_priv.h"

uint32 g_fea_cdb_enable = TRUE;
uint32 g_cdb_enable = TRUE;
uint32 g_cdb_data_dispatch_before_inited = 0;
static uint32 g_cdb_tbl_subscribe_pm[TBL_MAX];
static uint32 g_fea_cdb_tbl_subscribe_en[TBL_MAX];

/* Added by Yan Xing'an for support PM restart, 2015-2-3 */
#define __SYNC_DATA_TO_PM_BEGAIN__

cdb_tbl_t g_cdb_tbl_sync_seq[TBL_MAX] =
{
    TBL_INTERFACE,
    TBL_SYS_GLOBAL,
    TBL_OPENFLOW,
    
    TBL_MAX /* This must at last */
};

pm_id_t g_cdb_pm_support_sync[PM_ID_MAX] =
{
    PM_ID_OPENFLOW,
    
    PM_ID_MAX /* This must at last */
};

typedef int32 (*DS_ITER_CB_FUNC)(cdb_ds_t ds_id, void *obj, tbl_iter_args_t *pargs);

#define __SYNC_DATA_TO_PM_END__

pm_id_t
cds_get_show_resource_dest_pm(cdb_node_t* p_node, ctc_msg_t *p_msg)
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

static int32
_cds_dispatch_data_to_pm(pm_id_t pm_id, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, ctc_msg_t *p_msg)
{
    lib_master_t *lib_master = lib_get_master();
    int32 rc = PM_E_NONE;

    if (INVALID_SOCK_ID == lib_master->data_sock)
    {
        g_cdb_data_dispatch_before_inited++;
        return PM_E_NONE;
    }
    
    rc = ctc_sock_send(lib_master->data_sock, pm_id, p_msg);
    return rc;
}

static int32
_cds_format_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
{
    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        return PM_E_NONE;
    }
    
    if (p_ds_node)
    {
        return cdb_format_ds(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                p_ds_node, p_ds2_node, p_msg->data, p_msg->data_len);
    }
    else
    {
        return cdb_format_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);
    }

    return PM_E_NONE;
}

static uint32
_cds_is_fea_tbl_subscribe_en(uint16 tbl_id)
{
    if (tbl_id >= TBL_MAX)
    {
        return FALSE;
    }

    return g_fea_cdb_tbl_subscribe_en[tbl_id];
}

static int32
_cds_register_subscribe_fea(uint32 tbl_id)
{
    if (tbl_id >= TBL_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    g_fea_cdb_tbl_subscribe_en[tbl_id] = TRUE;
    return PM_E_NONE;
}

static int32
_cds_dispatch_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, int32 src_pmid, ctc_msg_t *p_msg)
{
    uint32 pm_bmp = 0;
    uint32 i = 0;

    pm_bmp = g_cdb_tbl_subscribe_pm[p_tbl_node->id];

    /* Fea should sync at first */
    _cds_dispatch_data_to_pm(PM_ID_CCS, p_tbl_node, p_ds_node, p_msg);
    if (g_fea_cdb_enable && _cds_is_fea_tbl_subscribe_en(p_tbl_node->id))
    {
        if (src_pmid != PM_ID_FEA)
        {
            _cds_dispatch_data_to_pm(PM_ID_FEA, p_tbl_node, p_ds_node, p_msg);
        }
    }
    
    if (pm_bmp)
    {
        for (i = PM_ID_CHSM; i < PM_ID_MAX; i++)
        {
            if (GLB_IS_BIT_SET(pm_bmp, i))
            {
                if (src_pmid != i)
                {
                    _cds_dispatch_data_to_pm(i, p_tbl_node, p_ds_node, p_msg);
                }
            }
        }
    }

    return PM_E_NONE;
}

static int32
_cds_store_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
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
_cds_store_tbl_log(cdb_node_t* p_tbl_node, ctc_msg_t *p_msg)
{
    tbl_log_master_t *p_master = tbl_log_get_master();
    tbl_log_t *p_log = (tbl_log_t*)p_msg->data;
    
    cdb_store_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);

    /* update sequence_id */
    p_log->key.sequence_id = (p_master->curr_sequence_id - 1);

    return PM_E_NONE;
}

static int32
_cds_get_src_pmid(int32 sock, int32 sess_fd)
{
    ctc_sock_session_t *p_sess = NULL;
    p_sess = ctc_sock_get_session_by_fd(sock, sess_fd);
    if (NULL == p_sess)
    {
        return PM_E_FAIL;
    }

    return SOCK_GET_PEER_PMID(p_sess->peer);
}

static int32
_cds_process_data_sync_req(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    cdb_node_t* p_tbl_node = NULL;
    cdb_node_t* p_ds_node = NULL;
    cdb_node_t* p_ds2_node = NULL;
    int32 src_pmid = 0;
    int32 rc = PM_E_NONE;

    /* 1. get node */
    p_tbl_node = cdb_get_tbl(p_msg->u_hdr.data.tbl_id);
    if (NULL == p_tbl_node)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Invalid tbl id %u\n", p_msg->u_hdr.data.tbl_id);
        return PM_E_FAIL;
    }
    
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

    /* 2. format data, e.g. set pointer to NULL */
    rc = _cds_format_data(p_tbl_node, p_ds_node, p_ds2_node, p_msg);
    if (rc < 0)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Format data (tbl %u, ds %u) failed\n", 
                        p_msg->u_hdr.data.tbl_id, p_msg->u_hdr.data.ds_id);
        return rc;
    }

    /* 3. send to subscriber PMs */
    if (p_tbl_node->id == TBL_LOG)
    {
        rc = _cds_store_tbl_log(p_tbl_node, p_msg);
    }

    /* 4. send to subscriber PMs */ 
    src_pmid = _cds_get_src_pmid(sock, sess_fd);
    rc = _cds_dispatch_data(p_tbl_node, p_ds_node, src_pmid, p_msg);
    if (rc < 0)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Dispatch data (tbl %u, ds %u) to subscriber PMs failed\n", 
                        p_msg->u_hdr.data.tbl_id, p_msg->u_hdr.data.ds_id);
        return rc;
    }

    /* 5. store data to CDB */
    if (g_cdb_enable)
    {
        if (p_tbl_node->id != TBL_LOG)
        {
            rc = _cds_store_data(p_tbl_node, p_ds_node, p_ds2_node, p_msg);
        }
    }

    return rc;
}

/* Added by Yan Xing'an for support PM restart, 2015-2-3 */
#define __SYNC_DATA_TO_PM_BEGAIN__

static int32
_cds_sync_tbl_data_to_pm(uint16 pm_id, uint8 oper, uint8 type, 
                         uint16 tbl_id, uint32 field_id,
                         void *p_data, uint32 data_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;

    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_DATA, data_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = INVALID_CDB_ID;
    p_req_msg->u_hdr.data.ds2_id = INVALID_CDB_ID;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;

    return ctc_sock_send(lib_master->data_sock, pm_id, p_req_msg);
}

static int32
_cds_sync_tbl_ds_data_to_pm(uint16 pm_id, uint8 oper, uint8 type, 
                        uint16 tbl_id, uint16 ds_id, uint32 field_id, 
                        void *p_data, uint32 data_len,
                        void *p_data1, uint32 data1_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;

    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD1_HDR(p_req_msg, MSG_OPER_DATA, data_len, data1_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = ds_id;
    p_req_msg->u_hdr.data.ds2_id = 0;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    p_req_msg->data1_len = data1_len;
    p_req_msg->data1 = p_data1;
    p_req_msg->data2_len = 0;
    p_req_msg->data2 = NULL;
    
    return ctc_sock_send(lib_master->data_sock, pm_id, p_req_msg);
}

static int32
_cds_sync_tbl_add_one_ds(cdb_ds_t ds_id, void *obj, tbl_iter_args_t *pargs)
{
    pm_id_t pm_id = *(pm_id_t*)pargs->argv[0];
    cdb_node_t* p_tbl_node = pargs->argv[1];
    void *tbl_obj = pargs->argv[2];

    cdb_node_t* p_ds_node = cdb_get_ds(ds_id);

    _cds_sync_tbl_ds_data_to_pm(pm_id, CDB_OPER_ADD, CDB_NODE_DS, 
        p_tbl_node->id, p_ds_node->id, 0,
        tbl_obj, p_tbl_node->tbl_info->key_size,
        obj, p_ds_node->tbl_info->data_size);

    return PM_E_NONE;
}

static int32
_cds_sync_clear_tbl_ds(cdb_tbl_t tbl_id, void *obj)
{
#define DEFINE_DS(ds_id, ds_name)\
    p_tbl_obj->ds_name = NULL;

    switch(tbl_id)
    {
    case TBL_INTERFACE:
        {
            TBL_INTERFACE_ST* p_tbl_obj = obj;
            TBL_INTERFACE_DS
        }
        break;

    default:
        break;
    }

#undef DEFINE_DS
    return PM_E_NONE;
}

static int32
_cds_sync_tbl_add_ds(tbl_iter_args_t *pargs, DS_ITER_CB_FUNC fn)
{
    cdb_node_t* p_tbl_node = pargs->argv[1];
    cdb_tbl_t tbl_id = p_tbl_node->id;
    void *obj = pargs->argv[2];

#define DEFINE_DS(ds_id, ds_name) \
    if (p_tbl_obj->ds_name)\
    {\
        fn(ds_id, p_tbl_obj->ds_name, pargs);\
    }\

    switch(tbl_id)
    {
    case TBL_INTERFACE:
        {
            TBL_INTERFACE_ST* p_tbl_obj = obj;
            TBL_INTERFACE_DS
        }
        break;

    default:
        break;
    }

#undef DEFINE_DS
    return PM_E_NONE;
}

static int32
_cds_sync_add_one_tbl(void *obj, tbl_iter_args_t *pargs)
{
    pm_id_t pm_id = *(pm_id_t*)pargs->argv[0];
    cdb_node_t* p_tbl_node = pargs->argv[1];
    void *obj_wo_ds; /* object without ds */
    tbl_iter_args_t ds_args;
    
    obj_wo_ds= malloc(p_tbl_node->tbl_info->data_size);
    memcpy(obj_wo_ds, obj, p_tbl_node->tbl_info->data_size);
    
    _cds_sync_clear_tbl_ds(p_tbl_node->id, obj_wo_ds);
    _cds_sync_tbl_data_to_pm(pm_id, CDB_OPER_ADD, CDB_NODE_TBL, 
        p_tbl_node->id, 0,
        obj_wo_ds, p_tbl_node->tbl_info->data_size);

    /* Here uses original obj which ds are not cleared */
    ds_args.argv[0] = &pm_id;
    ds_args.argv[1] = p_tbl_node;
    ds_args.argv[2] = obj;
    _cds_sync_tbl_add_ds(&ds_args, _cds_sync_tbl_add_one_ds);

    free(obj_wo_ds);

    return PM_E_NONE;
}

static int32
_cds_sync_set_tbl_field(void *obj, tbl_iter_args_t *pargs)
{
    pm_id_t pm_id = *(pm_id_t*)pargs->argv[0];
    cdb_node_t* p_node = pargs->argv[1];
    uint32 field_id = *(uint32*)pargs->argv[2];
    
    return _cds_sync_tbl_data_to_pm(pm_id, CDB_OPER_SET, CDB_NODE_TBL, p_node->id, field_id, obj, p_node->tbl_info->data_size);
}

int32
_cds_sync_add_tbl(cdb_tbl_t tbl_id, pm_id_t pm_id)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    tbl_iter_args_t args;
    
    args.argv[0] = &pm_id;
    args.argv[1] = p_node;
    
    p_node->tbl_info->ops.iter_fn(_cds_sync_add_one_tbl, &args);
    
    return PM_E_NONE;
}

int32
_cds_sync_set_tbl(cdb_tbl_t tbl_id, pm_id_t pm_id)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    tbl_iter_args_t args;
    uint32 field_id = 0;
    
    args.argv[0] = &pm_id;
    args.argv[1] = p_node;
    args.argv[2] = &field_id;
    
    for (field_id = 0; field_id < p_node->tbl_info->field_num; field_id++)
    {
        p_node->tbl_info->ops.iter_fn(_cds_sync_set_tbl_field, &args);
    }

    return PM_E_NONE;
}

static int32
_cds_sync_tbl(cdb_tbl_t tbl_id, pm_id_t pm_id)
{
    switch(tbl_id)
    {
    case TBL_INTERFACE:
    case TBL_ROUTE_IF:
    case TBL_IPROUTE_NODE:
    case TBL_STATIC_ROUTE_CFG:
    case TBL_ARP:
    case TBL_ARP_FIB:
    case TBL_NEXTHOP:
    case TBL_SYS_SPEC:
    case TBL_MIRROR:
        _cds_sync_add_tbl(tbl_id, pm_id);
        break;

    case TBL_SYS_GLOBAL:
    case TBL_ROUTE_GLOBAL:
    case TBL_SNMP_CFG:
    case TBL_LOG_GLOBAL:
    case TBL_CHASSIS:
        _cds_sync_set_tbl(tbl_id, pm_id);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
_cds_sync_response(int32 sock, int32 sess_fd, pm_id_t pm_id)
{
    ctc_msg_t   msg;
    ctc_msg_t  *p_msg = &msg;
    uint32 data_len = 0;
    int32 rc = PM_E_NONE;

    sal_memset(p_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_msg, MSG_OPER_SYNC, data_len);
    p_msg->u_hdr.sync.oper = CDB_SYNC_STOP;
    p_msg->u_hdr.sync.dest = pm_id;
    p_msg->u_hdr.sync.src = PM_ID_CDS;
    p_msg->u_hdr.sync.seq = 0;

    p_msg->data = NULL;
    p_msg->data_len = data_len;

    rc = ctc_sock_send(sock, sess_fd, p_msg);

    return rc;
}

static int32
_cds_sync_data_to_pm(int32 sock, int32 sess_fd, pm_id_t pm_id)
{
    cdb_tbl_t tbl_id;
    int32 i;

    for (i = 0; i < TBL_MAX; i ++)
    {
        tbl_id = g_cdb_tbl_sync_seq[i];
        if (tbl_id == TBL_MAX)
        {
            break;
        }
        if (GLB_IS_BIT_SET(g_cdb_tbl_subscribe_pm[tbl_id], pm_id))
        {
            _cds_sync_tbl(tbl_id, pm_id);
        }
    }
    _cds_sync_response(sock, sess_fd, pm_id);
    
    return PM_E_NONE;
}

static bool
_cds_pm_need_sync(pm_id_t pm_id)
{
    tbl_sys_global_t* p_sys_global = NULL;
    int32 i;
    bool is_pm_support_sync = FALSE;

    for (i = 0; i < PM_ID_MAX; i++)
    {
        if (PM_ID_MAX == g_cdb_pm_support_sync[i])
        {
            break;
        }
        if (pm_id == g_cdb_pm_support_sync[i])
        {
            is_pm_support_sync = TRUE;
            break;
        }
    }
    if (!is_pm_support_sync)
    {
        return FALSE;
    }

    /* When PM connects with cds after system startup done,
     * it's need to sync cdb data to this PM.
     */
    p_sys_global = tbl_sys_global_get_sys_global();
    if (!p_sys_global || !p_sys_global->startup_done)
    {
        return FALSE;
    }

    return TRUE;
}
#define __SYNC_DATA_TO_PM_END__

static int32
_cds_do_sync_tbl_to_pm(int32 sock, int32 sess_fd, pm_id_t pm_id, uint32 *tbl_ids, uint32 tbl_num)
{
    cdb_tbl_t tbl_id;
    int32 i;
    struct timeval tv1, tv2;
    uint32 delta = 0;
    
    sal_gettimeofday(&tv1);

    for (i = 0; i < tbl_num; i++)
    {
        tbl_id = tbl_ids[i];
        {
            _cds_sync_tbl(tbl_id, pm_id);
        }
    }
    sal_gettimeofday(&tv2);

    delta = (tv2.tv_sec - tv1.tv_sec) * 100000 + (tv2.tv_usec - tv1.tv_usec);
    log_sys(M_MOD_CDS, E_ERROR, "Sync %u TBL to %s takes %u us", tbl_num, IPC_DEST_DESC(pm_id), delta);

    return PM_E_NONE;
}

static int32
_cds_process_sync_req(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    cdb_node_t *p_tbl = NULL;
    uint32 *p_tbl_id = NULL;
    uint32 tbl_ids[TBL_MAX];
    uint32 tbl_num = 0;
    uint32 i = 0;
    
    tbl_num = p_msg->data_len / sizeof(uint32);

    log_sys(M_MOD_CDS, E_ERROR, "Recv sync request oper: %u, dest: %s, src: %s, tbl_num: %u", p_msg->u_hdr.sync.oper, 
        IPC_DEST_DESC(p_msg->u_hdr.sync.dest), IPC_DEST_DESC(p_msg->u_hdr.sync.src), tbl_num);

    p_tbl_id = (uint32*)p_msg->data;
    for (i = 0; i < tbl_num; i++)
    {
        tbl_ids[i] = p_tbl_id[i];
    }

    for (i = 0; i < tbl_num; i++)
    {
        p_tbl = cdb_get_tbl(tbl_ids[i]);
        log_sys(M_MOD_CDS, E_ERROR, "Table[%u] = %u, %s", i, p_tbl->id, p_tbl->name);
    }    
    (void)tbl_ids;

    _cds_do_sync_tbl_to_pm(sock, sess_fd, p_msg->u_hdr.sync.src, tbl_ids, tbl_num);
    return PM_E_NONE;
}

int32
cds_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        if (_cds_pm_need_sync(p_msg->u_hdr.connect.src))
        {
            return _cds_sync_data_to_pm(sock, sess_fd, p_msg->u_hdr.connect.src);
        }
        break;

    case MSG_OPER_SYNC:
        return _cds_process_sync_req(sock, sess_fd, p_msg);

    case MSG_OPER_CFG:
        return cfg_cmd_process(sock, p_msg);

    case MSG_OPER_DATA:
        return _cds_process_data_sync_req(sock, sess_fd, p_msg);

    default:
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

static int32
_cds_cmd_process_sub_pm_show_one(FILE *fp, uint32 tbl_id, uint32 pm_bmp)
{
    cdb_node_t *p_tbl = cdb_get_tbl(tbl_id);
    uint32 count = 0;
    uint32 i = 0;
    char pm_bmp_str[CMD_BUF_256];

    pm_bmp_str[0] = CMD_ZERO_CHAR;
    if (0 == pm_bmp)
    {
        count = 0;
        sal_fprintf(fp, "%-9u %-16s %-9u\n", tbl_id, p_tbl->name, count);
    }
    else
    {
        for (i = 0; i < PM_ID_MAX; i++)
        {
            if (GLB_IS_BIT_SET(pm_bmp, i))
            {
                count++;
                sal_strncat(pm_bmp_str, IPC_DEST_DESC(i), CMD_BUF_256);
                sal_strncat(pm_bmp_str, ", ", CMD_BUF_256);
            }
        }
        sal_fprintf(fp, "%-9u %-16s %-9u %-s\n", tbl_id, p_tbl->name, count, pm_bmp_str);
    }

    return PM_E_NONE;
}

static int32
_cds_cmd_process_get_subscriber_pm(cfg_show_t *p_resp_show, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    uint32 tbl_id = 0;
    uint32 pm_bmp = 0;

    sal_memset(p_resp_show, 0, sizeof(cfg_show_t));
    p_resp_show->banner = TRUE;
    CFG_BUILD_SHOW_PATH(p_resp_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(p_resp_show->path, "w+");

    sal_fprintf(fp, "CDB Table Subscriber PMs\n");

    sal_fprintf(fp, "%-9s %-16s %-9s %-s \n", "Table ID", "Table Name", "PM Count", "Subscribe PMs");
    sal_fprintf(fp, "---------+----------------+---------+-------------------------------------------\n");
    for (tbl_id = 0; tbl_id < TBL_MAX; tbl_id++)
    {
        pm_bmp = g_cdb_tbl_subscribe_pm[tbl_id];
        _cds_cmd_process_sub_pm_show_one(fp, tbl_id, pm_bmp);
    }
    sal_fprintf(fp, "\n");
    
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
cds_cmd_process_subscriber(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == sal_strcmp("pm", argv[0]))
        {
            rc = _cds_cmd_process_get_subscriber_pm(para->p_show, para);
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

static int32
_cds_register_subscribe_pm(uint32 tbl_id, uint32 pm_id)
{
    if (tbl_id >= TBL_MAX)
    {
        return PM_E_INVALID_PARAM;
    }
    if (PM_ID_INVALID == pm_id || pm_id >= PM_ID_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    GLB_SET_BIT(g_cdb_tbl_subscribe_pm[tbl_id], pm_id);
    return PM_E_NONE;
}

int32
cdb_register_tbl_subscribe_fea()
{
    _cds_register_subscribe_fea(TBL_INTERFACE);
    _cds_register_subscribe_fea(TBL_PORT);
    _cds_register_subscribe_fea(TBL_ROUTE_IF);
    _cds_register_subscribe_fea(TBL_VLAN);
    _cds_register_subscribe_fea(TBL_PVLAN);
    _cds_register_subscribe_fea(TBL_FDB);
    _cds_register_subscribe_fea(TBL_MCFDB);
    _cds_register_subscribe_fea(TBL_MACFILTER);
    _cds_register_subscribe_fea(TBL_PSFDB);
    _cds_register_subscribe_fea(TBL_BRG_GLOBAL);
    _cds_register_subscribe_fea(TBL_VERSION);
    _cds_register_subscribe_fea(TBL_ROUTE_GLOBAL);
    _cds_register_subscribe_fea(TBL_IPROUTE_NODE);
    _cds_register_subscribe_fea(TBL_IPROUTE_NODE_ADD_FAIL_COUNT);
    _cds_register_subscribe_fea(TBL_STATIC_RT_CNT);
    _cds_register_subscribe_fea(TBL_ARP);
    _cds_register_subscribe_fea(TBL_ARP_FIB);
    _cds_register_subscribe_fea(TBL_NEXTHOP);
    _cds_register_subscribe_fea(TBL_NEXTHOP_GROUP);
    _cds_register_subscribe_fea(TBL_IPSG_FIB);
    _cds_register_subscribe_fea(TBL_SFLOW_COLLECTOR);
    _cds_register_subscribe_fea(TBL_SYS_GLOBAL);
    _cds_register_subscribe_fea(TBL_LAG_GLOBAL);
    _cds_register_subscribe_fea(TBL_ECMP_GLOBAL);
    _cds_register_subscribe_fea(TBL_SYS_SPEC);
    _cds_register_subscribe_fea(TBL_LLDP_GLOBAL);
    _cds_register_subscribe_fea(TBL_LLDP_IF);
    _cds_register_subscribe_fea(TBL_VLANCLASS_RULE);
    _cds_register_subscribe_fea(TBL_VLANCLASS_GROUP);
    _cds_register_subscribe_fea(TBL_VLAN_MAPPING_TABLE);
    _cds_register_subscribe_fea(TBL_VLAN_MAPPING_ENTRY);
    _cds_register_subscribe_fea(TBL_EGRESS_MAPPING_ENTRY);
    _cds_register_subscribe_fea(TBL_EVC_ENTRY);
    _cds_register_subscribe_fea(TBL_VLAN_GROUP);
    _cds_register_subscribe_fea(TBL_MAPPED_VLAN_INFO);
    //_cds_register_subscribe_fea(TBL_SYS_SPEC_GLOBAL);
    _cds_register_subscribe_fea(TBL_ACL_WORM_FILTER);
    _cds_register_subscribe_fea(TBL_ACL_CONFIG);
    _cds_register_subscribe_fea(TBL_ACE_CONFIG);
    _cds_register_subscribe_fea(TBL_ACL_UDF_ENTRY);
    _cds_register_subscribe_fea(TBL_TIME_RANGE);
    _cds_register_subscribe_fea(TBL_QOS_DOMAIN);
    _cds_register_subscribe_fea(TBL_QOS_MPLS_DOMAIN);
    _cds_register_subscribe_fea(TBL_QOS_POLICER_ACTION_PROFILE);
    _cds_register_subscribe_fea(TBL_QOS_POLICER_PROFILE);
    _cds_register_subscribe_fea(TBL_QOS_DROP_PROFILE);
    _cds_register_subscribe_fea(TBL_QOS_QUEUE_SHAPE_PROFILE);
    _cds_register_subscribe_fea(TBL_QOS_PORT_SHAPE_PROFILE);
    _cds_register_subscribe_fea(TBL_QOS_QUEUE_SMART_BUFFER);
    _cds_register_subscribe_fea(TBL_QOS_GLOBAL);
    _cds_register_subscribe_fea(TBL_MIRROR);
    _cds_register_subscribe_fea(TBL_MIRROR_MAC_ESCAPE);
    _cds_register_subscribe_fea(TBL_TAP_GROUP);
    _cds_register_subscribe_fea(TBL_TAP_GROUP_INGRESS);
    _cds_register_subscribe_fea(TBL_TAP_GROUP_INGRESS_FLOW);
    _cds_register_subscribe_fea(TBL_TAP_GROUP_EGRESS);
    _cds_register_subscribe_fea(TBL_DHCBINDING);
    _cds_register_subscribe_fea(TBL_ERRDISABLE);
    _cds_register_subscribe_fea(TBL_ACLQOS_IF);
    _cds_register_subscribe_fea(TBL_L2_ACTION);
    _cds_register_subscribe_fea(TBL_ACL_ENTRY);
    _cds_register_subscribe_fea(TBL_ACL_ENTRY_ACTION);
    _cds_register_subscribe_fea(TBL_CPU_TRAFFIC);
    _cds_register_subscribe_fea(TBL_CPU_TRAFFIC_GROUP);
    _cds_register_subscribe_fea(TBL_MSTP_GLOBAL);
    _cds_register_subscribe_fea(TBL_MSTP_INSTANCE);
    _cds_register_subscribe_fea(TBL_MSTP_PORT);
    _cds_register_subscribe_fea(TBL_MSTI_PORT);
    _cds_register_subscribe_fea(TBL_ISOLATION);
    _cds_register_subscribe_fea(TBL_MLAG);
    _cds_register_subscribe_fea(TBL_MLAG_PEER);
    _cds_register_subscribe_fea(TBL_MLAG_PORT);
    _cds_register_subscribe_fea(TBL_ERPS_RING);
    _cds_register_subscribe_fea(TBL_IPTABLES_PREVENT);
    _cds_register_subscribe_fea(TBL_IGSP_GLOBAL);
    _cds_register_subscribe_fea(TBL_IGSP_INTF);
    _cds_register_subscribe_fea(TBL_DOT1X_GLOBAL);
    _cds_register_subscribe_fea(TBL_DOT1X_PORT);
    _cds_register_subscribe_fea(TBL_DOT1X_MAC);
    _cds_register_subscribe_fea(TBL_ROUTE_GLOBAL);
    _cds_register_subscribe_fea(TBL_CLEAR_ACL_POLICY);
    _cds_register_subscribe_fea(TBL_ACL_L4_PORT_RANGE);
    _cds_register_subscribe_fea(TBL_ACL_UDF_GROUP);

    _cds_register_subscribe_fea(TBL_BOOTIMAGE);
    _cds_register_subscribe_fea(TBL_REBOOT_INFO);
    _cds_register_subscribe_fea(TBL_CHASSIS);
    _cds_register_subscribe_fea(TBL_FIBER);
    _cds_register_subscribe_fea(TBL_IFNAME_INFO);
    _cds_register_subscribe_fea(TBL_CARD);
    _cds_register_subscribe_fea(TBL_CHIP);
    _cds_register_subscribe_fea(TBL_OPENFLOW_INTERFACE);
    _cds_register_subscribe_fea(TBL_ARPINSP);
    _cds_register_subscribe_fea(TBL_ARPACL_CONFIG);
    _cds_register_subscribe_fea(TBL_ARPACE_CONFIG);
    _cds_register_subscribe_fea(TBL_COPP_CFG);
    _cds_register_subscribe_fea(TBL_SFLOW_GLOBAL);
    _cds_register_subscribe_fea(TBL_SFLOW_COUNTER_PORT);
    /* modified by liwh for bug 44506, 2017-06-27 */
    _cds_register_subscribe_fea(TBL_OSPF_NETWORK);
    /* liwh end */
    _cds_register_subscribe_fea(TBL_OSPF_AREA_RANGE);
    _cds_register_subscribe_fea(TBL_OSPF_AREA_AUTH);

    _cds_register_subscribe_fea(TBL_MRT);
    _cds_register_subscribe_fea(TBL_IPMC_INTF);

    _cds_register_subscribe_fea(TBL_BHM_GLOBAL);
    /* modified by liwh for bug 45215, 2017-09-16 */
    _cds_register_subscribe_fea(TBL_VRRP_GLOBAL);
    _cds_register_subscribe_fea(TBL_VRRP_VIP);
    /* liwh end */
    _cds_register_subscribe_fea(TBL_PORT_STATS_RATE);
    _cds_register_subscribe_fea(TBL_PTP_GLOBAL);
    _cds_register_subscribe_fea(TBL_PTP_PORT);

    /*added by yejl*/
    _cds_register_subscribe_fea(TBL_SSM_GLOBAL);
    _cds_register_subscribe_fea(TBL_SSM_PORT);
    /*ended by yejl*/
    _cds_register_subscribe_fea(TBL_IPSLA_GLOBAL);
    _cds_register_subscribe_fea(TBL_TRACK_GLOBAL);

    _cds_register_subscribe_fea(TBL_APS_GROUP_INFO);

    _cds_register_subscribe_fea(TBL_OPENFLOW);

    _cds_register_subscribe_fea(TBL_TPOAM_GLOBAL);

    _cds_register_subscribe_fea(TBL_SSH_CFG);
    _cds_register_subscribe_fea(TBL_SYSLOG_CFG);

    _cds_register_subscribe_fea(TBL_L2_PROTOCOL);

    _cds_register_subscribe_fea(TBL_ND_FIB);

    _cds_register_subscribe_fea(TBL_OPENFLOW_TUNNEL_INTERFACE);
    _cds_register_subscribe_fea(TBL_CLOCK);
    _cds_register_subscribe_fea(TBL_HASH_FIELD_PROFILE);
    _cds_register_subscribe_fea(TBL_HASH_VALUE_PROFILE);
    _cds_register_subscribe_fea(TBL_HASH_VALUE_GLOBAL);
    _cds_register_subscribe_fea(TBL_G8032_RING);

    _cds_register_subscribe_fea(TBL_POE);
    /* begin added by taocy for NAT, 2019-06-03*/
    _cds_register_subscribe_fea(TBL_NAT_SESSION);
    _cds_register_subscribe_fea(TBL_NAT_SESSION_LIMIT);
    _cds_register_subscribe_fea(TBL_NAT_RULE);
    _cds_register_subscribe_fea(TBL_NAT_GLOBAL);
    _cds_register_subscribe_fea(TBL_NAT_POOL);
    _cds_register_subscribe_fea(TBL_NAT_RTIF_INSIDE);
    _cds_register_subscribe_fea(TBL_NAT_RTIF_OUTSIDE);
    /* end added by taocy for NAT, 2019-06-03*/

    /* modified by liwh for bug 53010, 2019-07-22 */
    _cds_register_subscribe_fea(TBL_CFM_GLOBAL);
    _cds_register_subscribe_fea(TBL_CFM_MD);
    _cds_register_subscribe_fea(TBL_CFM_MA);
    _cds_register_subscribe_fea(TBL_CFM_MAID);
    _cds_register_subscribe_fea(TBL_CFM_LMEP);
    _cds_register_subscribe_fea(TBL_CFM_RMEP);
    /* liwh end */
    
    return PM_E_NONE;
}

int32
cdb_register_tbl_subscribe_pm()
{
    /* PM_ID_CTL : None */
    
    /* PM_ID_CDS : None */
    
    /* PM_ID_CHSM */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_IFNAME_INFO,         PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_CARD,                PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_PORT,                PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_FIBER ,              PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_FAN,                 PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_PSU,                 PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_LED,                 PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_SENSOR,              PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_VCM,              PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_REBOOT_INFO,         PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_TAG_SUMMER_CLOCK,    PM_ID_CHSM);
    _cds_register_subscribe_pm(TBL_POE,    PM_ID_CHSM);
    /* PM_ID_SWITCH */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_FEA_PORT_POLICER_APPLY, PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ACLQOS_IF_STATS,     PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_KERNEL_IF,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SYS_LOAD,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_FDB,                 PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_PSFDB,               PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_PORT_STATS,          PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ISOLATION,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_PVLAN,               PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ERPS_DOMAIN,         PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ERPS_RING,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_FEA_ACL,             PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_FEA_ACL_POLICY_ACTION,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_DHCBINDING,          PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_DHCSNOOPING,         PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_CARD,                PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_MANAGE_IF,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_REBOOT_INFO,         PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_OPENFLOW_INTERFACE,  PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_TAP_GROUP,           PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_TAP_GROUP_INGRESS,   PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_TAP_GROUP_INGRESS_FLOW,  PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_TAP_GROUP_EGRESS,    PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_VTY,                 PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ARPINSP,             PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SSH_CFG,             PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_FEA_PTP_GLOBAL,      PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SSM_PORT,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_L2_PROTOCOL,         PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_SSM_PORT,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ARPACL_CONFIG,       PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_TAP_DPI_GLOBAL,      PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ACL_UDF_ENTRY,       PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_L2EDIT,              PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_ECMP_GLOBAL,         PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_MSTP_INSTANCE,       PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_USER,                PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_NAT_GLOBAL,          PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_G8032_RING,          PM_ID_SWITCH);
    /* modified by liwh for bug 53010, 2019-07-22 */
    _cds_register_subscribe_pm(TBL_CFM_LMEP,            PM_ID_SWITCH);
    _cds_register_subscribe_pm(TBL_CFM_RMEP,            PM_ID_SWITCH);
    /* liwh end */
    _cds_register_subscribe_pm(TBL_NAT_GLOBAL,          PM_ID_SWITCH);
    /* PM_ID_ROUTED */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_MIRROR,              PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_NEXTHOP_GROUP,       PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_MACFILTER,           PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_ARPINSP,             PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_ARPACL_CONFIG,       PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_ARPACE_CONFIG,       PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_DHCBINDING,          PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_DHCSNOOPING,         PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_NS_ROUTE_IP,         PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_SYSLOG_CFG,          PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_PTP_PORT,            PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_ACL_CONFIG,          PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_NAT_SESSION,         PM_ID_ROUTED);
    _cds_register_subscribe_pm(TBL_NAT_GLOBAL,          PM_ID_ROUTED);
        
    /* PM_ID_OPM */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_MSTP_INSTANCE,       PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_MSTP_PORT,           PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_L2_ACTION,           PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_IGSP_INTF,           PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_IGSP_GLOBAL,         PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_SECTION_OAM,         PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_OAM_SESSION,         PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_OAM_FLOW_INFO,       PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_LSP_APS_GROUP,       PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_PW_APS_GROUP,        PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_LSP_PE,              PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_LSP_P,               PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_PW_TPE,              PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_APS_GROUP_INFO,      PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_OPENFLOW,            PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_FDB,                 PM_ID_OPM);
    /* modified by liwh for bug 53010, 2019-07-22 */
    _cds_register_subscribe_pm(TBL_CFM_LMEP,            PM_ID_OPM);
    _cds_register_subscribe_pm(TBL_CFM_RMEP,            PM_ID_OPM);
    /* liwh end */

    /* modified by liwh for bug 53818, 2019-09-29 */
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_OPM);
    /* liwh end */

    /* PM_ID_APPCFG */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_PORT_STATS,          PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_PORT_STATS_RATE,     PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_ENABLE,              PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_VTY,                 PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_CARD,          PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_VERSION,         PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_ACE_CONFIG,        PM_ID_APPCFG);
    _cds_register_subscribe_pm(TBL_ACL_CONFIG,        PM_ID_APPCFG);

    /* PM_ID_OPENFLOW */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OPENFLOW,            PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_CARD,                PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OPENFLOW_INTERFACE,  PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_PORT_STATS,          PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_INBAND_SNAT,         PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_NS_ROUTE,            PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_ACLQOS_IF_STATS, PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OPENFLOW_TUNNEL_INTERFACE, PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_TPOAM_GLOBAL, PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OAM_SESSION_UPLOAD, PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_APS_GROUP_INFO, PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OAM_INLABEL_INFO,      PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_ARP_FIB,              PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_IPROUTE_NODE,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OAM_SESSION,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_CHASSIS,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_ACL_UDF_ENTRY,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_L2EDIT,    PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_FDB,       PM_ID_OPENFLOW);
    _cds_register_subscribe_pm(TBL_MACFILTER, PM_ID_OPENFLOW);
    
    /* PM_ID_DHCRELAY */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_ARPINSP,             PM_ID_DHCRELAY);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_DHCRELAY);

    /* PM_ID_DHCSNOOPING */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_DHCP_DEBUG,          PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_IPSG_FIB,            PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_IPSG_S_MAC,          PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_ARPINSP,             PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_MACFILTER,           PM_ID_DHCSNOOPING);
	_cds_register_subscribe_pm(TBL_MLAG,           	  PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_MLAG_PEER,           PM_ID_DHCSNOOPING);
    _cds_register_subscribe_pm(TBL_MLAG_PORT,          PM_ID_DHCSNOOPING);


    /* PM_ID_DHCPD6 */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_ARPINSP,             PM_ID_DHCPD6);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_DHCPD6);
    
    /* PM_ID_AUTHD */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_VLAN,                PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_VTY,                 PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_AUTHOR,              PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_ACCOUNT,             PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_ACCOUNTCMD,          PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_LOGIN,               PM_ID_AUTHD);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_AUTHD);
    
    /* PM_ID_PTP */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_PTP);
    _cds_register_subscribe_pm(TBL_ARP,                 PM_ID_PTP);

    /* PM_ID_BHM */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_SNMP_CFG,            PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_CLOCK,               PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_BOOTIMAGE,           PM_ID_BHM);
    _cds_register_subscribe_pm(TBL_NS_ROUTE,            PM_ID_BHM);

    /* PM_ID_DHCLIENT */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_DHCP_DEBUG,          PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_MANAGE_IF,           PM_ID_DHCLIENT);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_DHCLIENT);

    /* PM_ID_SSM */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_SSM_GLOBAL,          PM_ID_SSM);
    _cds_register_subscribe_pm(TBL_SSM_PORT,            PM_ID_SSM);

    /* PM_ID_NCS */
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_CHIP,                PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_CONTROLLER_ATTR,  PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_VLAN_STATS,  PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_LSP_APS_GROUP,       PM_ID_NCS);
    _cds_register_subscribe_pm(TBL_PW_APS_GROUP,        PM_ID_NCS);

    /* PM_ID_DHCLIENT6 */
    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_VERSION,             PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_INTERFACE,           PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_ROUTE_IF,            PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_DHCP_DEBUG,          PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_SYS_SPEC,            PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_MANAGE_IF,           PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_DHCLIENT,            PM_ID_DHCLIENT6);
    _cds_register_subscribe_pm(TBL_CHASSIS,             PM_ID_DHCLIENT6);

    /* Add new PM_ID_XXX subscribe TBLs below */
    
    return PM_E_NONE;
}

int32
cdb_register_cmd_cb()
{
    cdb_register_cfg_act_cb(ACT_DUMP_CDS_SUBSCRIBER, cds_cmd_process_subscriber);
    return PM_E_NONE;
}

int32
cds_sock_init(lib_master_t *master)
{
    int32 sock = INVALID_SOCK_ID;
    master->cfg_sock = INVALID_SOCK_ID;
    master->data_sock = INVALID_SOCK_ID;
    master->sub_cdb_sock = INVALID_SOCK_ID;
    master->sub_tbl_sock = INVALID_SOCK_ID;
    master->trap_sock = INVALID_SOCK_ID;

    /* client to CDS */
    ipc_connect_ccs(0);

    /* CDS data socket */
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CdsSrvData");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_sock_set_pmid(sock, master->pm_id);
    ctc_sock_bind(sock, SOCK_ADDR_CDS_DATA);
    ctc_sock_set_queue_en(sock, TRUE);
    ctc_sock_set_pack_en(sock, TRUE);
    master->data_sock = sock;

    ipc_connect_bhm();
    
    return PM_E_NONE;
}

int32
_cds_send_local_data(uint8 oper, uint8 type, uint16 tbl_id, uint32 field_id, void *p_data, uint32 data_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
    
    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_DATA, data_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = INVALID_CDB_ID;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;

    rc = _cds_process_data_sync_req(-1, -1, p_req_msg);
    if (rc < 0)
    {
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u) failed", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[oper], tbl_id);
    }
    return rc;
}

int32
cds_sync_local_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    return _cds_send_local_data(CDB_OPER_SET, CDB_NODE_TBL, tbl_id, field_id, p_db_tbl, p_node->tbl_info->data_size);
}

int32
cds_init()
{ 
    lib_master_t *lib_master = lib_init(PM_ID_CDS);
    lib_master->task_mst = ctc_task_init();
    cds_master_t *master = (cds_master_t*)XCALLOC(MEM_PM_CDS_MASTER, sizeof(cds_master_t));
    lib_master->proto_mst = master;
    cds_sock_init(lib_master);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ctc_sock_register_rx_fn(cds_process_msg);
    cdb_register_tbl_subscribe_pm();
    cdb_register_tbl_subscribe_fea();
    cdb_register_cmd_cb();
    lib_load_set_cds_sync_set_func(cds_sync_local_tbl_set);
    lib_load_init();
    ctc_task_main();

    return PM_E_NONE;
}

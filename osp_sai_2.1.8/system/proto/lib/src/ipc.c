#include "proto.h"
#include "ipc.h"

#define IPC_CTL_SYNC_TIME   15  /* seconds */

typedef struct proto_ipc_op_s
{
    SOCK_RX_FN    proc_fn;
} proto_ipc_op_t;

#define IPC_HAL_MAX 64
int32 g_ipc_cfg_sync_time = IPC_CTL_SYNC_TIME * 10;
proto_ipc_op_t g_ipc_proc_hal_fn[IPC_HAL_MAX];
proto_ipc_op_t g_ipc_proc_pkt_fn[GLB_CPU_TRAFFIC_MAX];

uint32 g_ipc_pm_to_cds_error_count = 0;

int32
ipc_set_sync_timeout(uint32 sync_time)
{
    g_ipc_cfg_sync_time = sync_time;

    return PM_E_NONE;
}

int32
ipc_pm_sync_request(lib_master_t* lib_master, int32 sock)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    uint32 data_len = 0;
    int32 rc = PM_E_NONE;

    data_len = lib_master->sync_tbl_num * sizeof(uint32);
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_SYNC, data_len);
    p_req_msg->u_hdr.sync.oper = CDB_SYNC_START;
    p_req_msg->u_hdr.sync.dest = PM_ID_CDS;
    p_req_msg->u_hdr.sync.src = lib_master->pm_id;
    p_req_msg->u_hdr.sync.seq = 0;

    p_req_msg->data = lib_master->sync_tbl_ids;
    p_req_msg->data_len = data_len;

    rc = ctc_sock_send(sock, IGNORE_SOCK_PEER, p_req_msg);
    (void)rc;
    return PM_E_NONE;
}

static int32
_ipc_pm_connect_cb(int32 sock)
{
    lib_master_t* lib_master = lib_get_master();
    ctc_sock_t *p_sock = NULL;
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    uint32 pid = 0;
    int32 rc = PM_E_NONE;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CONNECT, 0);
    p_req_msg->u_hdr.connect.src = lib_master->pm_id;

    /* CTL maybe has multi-instance, need check pid */    
    if (PM_ID_CTL == lib_master->pm_id)
    {
        pid = sal_getpid();
    }
    p_req_msg->u_hdr.connect.pid = pid;    
    rc = ctc_sock_send(sock, IGNORE_SOCK_PEER, p_req_msg);
    (void)rc;
    
    if (lib_master->sync_enable)
    {
        p_sock = ctc_sock_get_sock(sock);
        if (p_sock)
        {
            if (0 == sal_strcmp(p_sock->desc, "CdsCltData"))
            {
                ipc_pm_sync_request(lib_master, sock);
            }
        }
    }

    return PM_E_NONE;
}

uint32 g_ipc_data_sync_count = 0;

int32
ipc_pm_process_sync(int32 sock, ctc_msg_t *p_msg)
{
    return PM_E_NONE;
}

int32
ipc_pm_process_data_sync(int32 sock, ctc_msg_t *p_msg)
{
    cdb_node_t* p_tbl_node = NULL;
    cdb_node_t* p_ds_node = NULL;
    cdb_node_t* p_ds2_node = NULL;
    int32 rc = PM_E_NONE;

    g_ipc_data_sync_count++;

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

    /* 2. store data to CDB */
    rc = cdb_store_data(p_tbl_node, p_ds_node, p_ds2_node, p_msg);

    return rc;
}

int32
ipc_pm_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_SYNC:
        return ipc_pm_process_sync(sock, p_msg);

    case MSG_OPER_CFG:
        return cfg_cmd_process(sock, p_msg);

    case MSG_OPER_DATA:
        return ipc_pm_process_data_sync(sock, p_msg);

    case MSG_OPER_FEI:
        if (g_ipc_proc_hal_fn[p_msg->u_hdr.fei.type].proc_fn)
        {
            g_ipc_proc_hal_fn[p_msg->u_hdr.fei.type].proc_fn(sock, sess_fd, p_msg);
        }
        else
        {
            if (p_msg->u_hdr.fei.flags & MSG_HAL_FLAGS_SYNC)
            {
                break;
            }
            log_sys(M_MOD_LIB, E_ERROR, "Unhandled HAL msg type %d\n", p_msg->u_hdr.fei.type);
        }
        break;

    case MSG_OPER_PKT:
        if (g_ipc_proc_pkt_fn[p_msg->u_hdr.pkt.type].proc_fn)
        {
            g_ipc_proc_pkt_fn[p_msg->u_hdr.pkt.type].proc_fn(sock, sess_fd, p_msg);
        }
        else
        {
            log_sys(M_MOD_LIB, E_DEBUG, "Unhandled Packet type %u(%s), ifindex %u, vid %u\n", 
                p_msg->u_hdr.pkt.type, GLB_PKT_MAP_TYPE_TO_STR(p_msg->u_hdr.pkt.type), p_msg->u_hdr.pkt.ifindex, p_msg->u_hdr.pkt.vid);
        }
        break;

    case MSG_OPER_LCM:
        return -1;
        
    default:
        return -1;
    }
    
    return 0;
}

int32
ipc_register_hal_rx_fn(uint32 type, SOCK_RX_FN proc_fn)
{
    if (type >= IPC_HAL_MAX)
    {
        return PM_E_FAIL;
    }
    g_ipc_proc_hal_fn[type].proc_fn = proc_fn;
    return PM_E_NONE;
}

int32
ipc_register_pkt_rx_fn(uint32 type, SOCK_RX_FN proc_fn)
{
    if (type >= GLB_PKT_TYPE_MAX)
    {
        return PM_E_FAIL;
    }
    g_ipc_proc_pkt_fn[type].proc_fn = proc_fn;
    return PM_E_NONE;
}

int32
ipc_pm_to_bhm_keepalive()
{
    lib_master_t* lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;

    if (lib_master->bhm_sock < 0)
    {
        return PM_E_NOT_INITED;
    }

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_BHM, 0);
    p_req_msg->u_hdr.bhm.src = lib_master->pm_id;
    p_req_msg->u_hdr.bhm.pid = sal_getpid();

    rc = ctc_sock_send(lib_master->bhm_sock, IGNORE_SOCK_PEER, p_req_msg);
    (void)rc;

    return PM_E_NONE;
}

int32
ipc_pm_cds_filter(uint16 tbl_id)
{
    if (TBL_FEA_FDB == tbl_id || TBL_FEA_PTP_TS == tbl_id
        /* modified by liwh for bug 48735, 2018-08-30 
             TBL_FEA_NEXTHOP needn't sync to cds*/
        || TBL_FEA_NEXTHOP == tbl_id)
        /* liwh end */
    {
        return PM_E_FAIL;        
    }

    return PM_E_NONE;
}

int32
ipc_pm_to_cds_data(uint8 oper, uint8 type, uint16 tbl_id, uint32 field_id, void *p_data, uint32 data_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;

    /* CDS not send to CDS */
    if (PM_ID_CDS == lib_master->pm_id)
    {
        return PM_E_NONE;
    }
    rc = ipc_pm_cds_filter(tbl_id);
    if (rc < 0)
    {
        return rc;
    }

    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_DATA, data_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = INVALID_CDB_ID;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;

    rc = ctc_sock_send(lib_master->data_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        g_ipc_pm_to_cds_error_count++;
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u) failed, err count %u", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[oper], tbl_id, g_ipc_pm_to_cds_error_count);
    }
    return rc;
}

int32
ipc_pm_to_cds_data_ds(uint8 oper, uint8 type, uint16 tbl_id, uint16 ds_id, uint32 field_id, 
                        void *p_data, uint32 data_len, void *p_data1, uint32 data1_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
    
    /* CDS not send to CDS */
    if (PM_ID_CDS == lib_master->pm_id)
    {
        return PM_E_NONE;
    }
    rc = ipc_pm_cds_filter(tbl_id);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD1_HDR(p_req_msg, MSG_OPER_DATA, data_len, data1_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = ds_id;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    p_req_msg->data1_len = data1_len;
    p_req_msg->data1 = p_data1;
    p_req_msg->data2_len = 0;
    p_req_msg->data2 = NULL;

    rc = ctc_sock_send(lib_master->data_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        g_ipc_pm_to_cds_error_count++;
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u),ds(%u) failed, err count %u", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[oper], tbl_id, ds_id, g_ipc_pm_to_cds_error_count);
    }
    return rc;
}

int32
ipc_pm_to_cds_data_ds2(uint8 oper, uint8 type, uint16 tbl_id, uint16 ds_id, uint16 ds2_id, uint32 field_id, 
                        void *p_data, uint32 data_len, void *p_data1, uint32 data1_len, void *p_data2, uint32 data2_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
    
    /* CDS not send to CDS */
    if (PM_ID_CDS == lib_master->pm_id)
    {
        return PM_E_NONE;
    }
    rc = ipc_pm_cds_filter(tbl_id);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD2_HDR(p_req_msg, MSG_OPER_DATA, data_len, data1_len, data2_len);
    p_req_msg->u_hdr.data.oper = oper;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = ds_id;
    p_req_msg->u_hdr.data.ds2_id = ds2_id;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    p_req_msg->data1_len = data1_len;
    p_req_msg->data1 = p_data1;
    p_req_msg->data2_len = data2_len;
    p_req_msg->data2 = p_data2;

    rc = ctc_sock_send(lib_master->data_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        g_ipc_pm_to_cds_error_count++;
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u),ds(%u),ds2(%u) failed, err count %u", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[oper], tbl_id, ds_id, ds2_id, g_ipc_pm_to_cds_error_count);
    }
    return rc;
}

int32
ipc_pm_to_cds_refer_tbl(uint8 type, uint16 tbl_id, uint32 field_id, 
    void *p_data, uint32 data_len, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj)
{
    cdb_sync_refer_t refer;
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    uint32 refer_length = 0;
    int32 rc = PM_E_NONE;

    /* CDS not send to CDS */
    if (PM_ID_CDS == lib_master->pm_id)
    {
        return PM_E_NONE;
    }
    rc = ipc_pm_cds_filter(tbl_id);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_memcpy(&refer.hdr, p_refer_hdr, sizeof(cdb_sync_refer_hdr_t));
    sal_memcpy(refer.object, p_refer_obj, p_refer_hdr->size);
    refer_length = sizeof(cdb_sync_refer_hdr_t) + p_refer_hdr->size;
    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD1_HDR(p_req_msg, MSG_OPER_DATA, data_len, refer_length);
    p_req_msg->u_hdr.data.oper = CDB_OPER_SYNC_REFER;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = INVALID_CDB_ID;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    p_req_msg->data1_len = refer_length;
    p_req_msg->data1 = &(refer);

    rc = ctc_sock_send(lib_master->data_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        g_ipc_pm_to_cds_error_count++;
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u) failed, err count %u", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[CDB_OPER_SYNC_REFER], tbl_id, g_ipc_pm_to_cds_error_count);
    }
    return rc;
}
    
int32
ipc_pm_to_cds_refer_ds(uint8 type, uint16 tbl_id, uint16 ds_id, uint32 field_id, 
    void *p_data, uint32 data_len, void *p_data1, uint32 data1_len,
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj)
{
    cdb_sync_refer_t refer;
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    uint32 refer_length = 0;
    int32 rc = PM_E_NONE;

    /* CDS not send to CDS */
    if (PM_ID_CDS == lib_master->pm_id)
    {
        return PM_E_NONE;
    }
    rc = ipc_pm_cds_filter(tbl_id);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_memcpy(&refer.hdr, p_refer_hdr, sizeof(cdb_sync_refer_hdr_t));
    sal_memcpy(refer.object, p_refer_obj, p_refer_hdr->size);
    refer_length = sizeof(cdb_sync_refer_hdr_t) + p_refer_hdr->size;
    sal_memset(&req_msg, 0, sizeof(req_msg));
    MSG_BUILD2_HDR(p_req_msg, MSG_OPER_DATA, data_len, data1_len, refer_length);
    p_req_msg->u_hdr.data.oper = CDB_OPER_SYNC_REFER;
    p_req_msg->u_hdr.data.type = type;
    p_req_msg->u_hdr.data.tbl_id = tbl_id;
    p_req_msg->u_hdr.data.ds_id = ds_id;
    p_req_msg->u_hdr.data.fields[0] = field_id;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    p_req_msg->data1_len = data1_len;
    p_req_msg->data1 = p_data1;
    p_req_msg->data2_len = refer_length;
    p_req_msg->data2 = &(refer);

    rc = ctc_sock_send(lib_master->data_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        g_ipc_pm_to_cds_error_count++;
        log_sys(M_MOD_LIB, E_ERROR, "%s %s send %s tbl(%u) failed, err count %u", 
            __FUNCTION__, proto_module_desc(lib_master->pm_id), cdb_oper_str_array[CDB_OPER_SYNC_REFER], tbl_id, g_ipc_pm_to_cds_error_count);
    }
    return rc;
}


int32
ipc_pm_to_ccs(int32 sock, ctc_msg_t *p_req_msg, void *p_data, uint32 data_len)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    uint32 peer = 0;
    
    sal_memset(&resp_msg, 0, sizeof(resp_msg));
    MSG_BUILD_HDR(p_resp_msg, MSG_OPER_CFG, data_len);
    p_resp_msg->u_hdr.cfg.dest = p_req_msg->u_hdr.cfg.src;
    p_resp_msg->u_hdr.cfg.src = p_req_msg->u_hdr.cfg.dest;
    p_resp_msg->u_hdr.cfg.pid = p_req_msg->u_hdr.cfg.pid;
    p_resp_msg->u_hdr.cfg.type = p_req_msg->u_hdr.cfg.type;
    p_resp_msg->u_hdr.cfg.id = p_req_msg->u_hdr.cfg.id;
    p_resp_msg->u_hdr.cfg.cfg = p_req_msg->u_hdr.cfg.cfg;
    p_resp_msg->data_len = data_len;
    p_resp_msg->data = p_data;

    if (PM_ID_CCS == lib_master->pm_id)
    {
        /* CTL maybe has multi-instance, need check pid */
        if (PM_ID_CTL != p_resp_msg->u_hdr.cfg.dest)
        {
            return -1;
        }
        peer = SOCK_BUILD_PEER(p_resp_msg->u_hdr.cfg.pid, p_resp_msg->u_hdr.cfg.dest);
        /* send to CTL directly */
        return ctc_sock_send(sock, peer, p_resp_msg);
    }
    else
    {
        /* send to CCS */
        return ctc_sock_send(sock, IGNORE_SOCK_PEER, p_resp_msg);
    }
}

/* continue to send ot ccs (for Loopback/Linktrace) */
int32
ipc_pm_continue_send_to_ccs(int32 sock, int32 pid, cfg_result_t *p_result)
{
    lib_master_t *lib_master = lib_get_master();
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    uint32 data_len = sizeof(cfg_result_t);

    sal_memset(&resp_msg, 0, sizeof(resp_msg));
    MSG_BUILD_HDR(p_resp_msg, MSG_OPER_CFG, data_len);
    p_resp_msg->u_hdr.cfg.dest = PM_ID_CTL;
    p_resp_msg->u_hdr.cfg.src = lib_master->pm_id;
    p_resp_msg->u_hdr.cfg.pid = pid;
    p_resp_msg->u_hdr.cfg.type = CDB_NODE_ACT;
    p_resp_msg->u_hdr.cfg.id = 0;
    p_resp_msg->data_len = data_len;
    p_resp_msg->data = p_result;

    /* send to CCS */
    return ctc_sock_send(sock, IGNORE_SOCK_PEER, p_resp_msg);
}

int32
ipc_ccs_to_pm_cfg(int32 sock, ctc_msg_t *p_msg)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;
    uint32 dest_pid = 0;
    uint32 peer = 0;

    /* TODO  */
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CFG, p_msg->data_len);
    p_req_msg->u_hdr.cfg.dest = p_msg->u_hdr.cfg.dest;
    p_req_msg->u_hdr.cfg.src = p_msg->u_hdr.cfg.src;
    p_req_msg->u_hdr.cfg.pid = p_msg->u_hdr.cfg.pid;
    p_req_msg->u_hdr.cfg.oper = p_msg->u_hdr.cfg.oper;
    p_req_msg->u_hdr.cfg.type = p_msg->u_hdr.cfg.type;
    p_req_msg->u_hdr.cfg.id = p_msg->u_hdr.cfg.id;
    p_req_msg->u_hdr.cfg.cfg = p_msg->u_hdr.cfg.cfg;
    p_req_msg->data_len = p_msg->data_len;
    p_req_msg->data = p_msg->data;

    /* CTL maybe has multi-instance, need check pid */
    if (PM_ID_CTL == p_msg->u_hdr.cfg.dest)
    {
        dest_pid = p_msg->u_hdr.cfg.pid;
    }
    peer = SOCK_BUILD_PEER(dest_pid, p_msg->u_hdr.cfg.dest);
    rc = ctc_sock_send(sock, peer, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }

    return PM_E_NONE;
}

/* refer ipc_ctl_to_ccs_cfg */
int32
ipc_ncs_to_pm_cfg_sync(int32 sock, pm_id_t pm_id, uint16 type, uint16 id, cdb_cfg_t cfg, cdb_oper_t opt, uint8 *data, uint32 data_len, ctc_msg_t *p_resp_msg)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    uint32 peer = 0;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CFG, data_len);
    p_req_msg->u_hdr.cfg.dest = pm_id;
    p_req_msg->u_hdr.cfg.src = PM_ID_NCS;
    p_req_msg->u_hdr.cfg.pid = 0;
    p_req_msg->u_hdr.cfg.oper = opt;
    p_req_msg->u_hdr.cfg.type = type;
    p_req_msg->u_hdr.cfg.id = id;
    p_req_msg->u_hdr.cfg.cfg = cfg;
    p_req_msg->data_len = data_len;
    p_req_msg->data = data;

    peer = SOCK_BUILD_PEER(0, p_req_msg->u_hdr.cfg.dest);
    rc = ctc_sock_send_sync(sock, peer, p_req_msg, p_resp_msg, IPC_CTL_SYNC_TIME * SOCK_TICS_PER_SEC); // IPC_CTL_SYNC_TIME
    if (rc < 0)
    {
#if 0    
        if (-1 == rc)
        {
            sal_printf("\n%% Command execute %d seconds timeout\n", IPC_CTL_SYNC_TIME);
        }
#endif
        return rc;
    }

    return rc;
}

#ifndef TAPPRODUCT
static void
_ipc_bhm_client_timer_func(void *p_data)
{
    lib_master_t *master = lib_get_master();

    master->t_bhm_client = NULL;
    ipc_pm_to_bhm_keepalive();
    master->t_bhm_client = ctc_task_add_timer(TASK_PRI_NORMAL, _ipc_bhm_client_timer_func, NULL, BHM_KEEPALIVE_INTERVAL);

    return;
}
#endif

int32
ipc_connect_bhm()
{
#ifdef TAPPRODUCT
    return PM_E_NONE;
#else
    lib_master_t *lib_master = lib_get_master();
    int32 rc = PM_E_NONE;
    int32 sock = INVALID_SOCK_ID;

    lib_master->bhm_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "BHMClient");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_BHM, 0);
    if (rc < 0)
    {
        sal_printf("\n%% Connecting to BHM %d times failed\n", 0);
        return PM_E_INIT_FAIL;
    }
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->bhm_sock = sock;

    if ((PM_ID_BHM != lib_master->pm_id) && (PM_ID_CTL != lib_master->pm_id))
    {
        lib_master->t_bhm_client = ctc_task_add_timer(TASK_PRI_NORMAL, _ipc_bhm_client_timer_func, NULL, BHM_KEEPALIVE_INTERVAL);
    }

    return rc;
#endif
}

int32
ipc_connect_ccs(int32 immediate)
{
    lib_master_t *lib_master = lib_get_master();
    int32 rc;
    int32 sock = INVALID_SOCK_ID;

    /* Connect to CDS Cfg: PM, CTL */
    lib_master->cfg_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CcsCltCfg");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_CCS, immediate);
    if (rc < 0)
    {
        sal_printf("\n%% Connecting to CCS cfg %d times failed\n", immediate);
        return PM_E_INIT_FAIL;
    }
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->cfg_sock = sock;

    return rc;
}

int32
ipc_connect_ccs_cds(int32 immediate)
{
    lib_master_t *lib_master = lib_get_master();
    int32 rc;
    int32 sock = INVALID_SOCK_ID;

    /* Connect to CDS Cfg: PM, CTL */
    lib_master->cfg_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CcsCltCfg");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_CCS, immediate);
    if (rc < 0)
    {
        sal_printf("\n%% Connecting to CCS cfg %d times failed\n", immediate);
        return PM_E_INIT_FAIL;
    }
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->cfg_sock = sock;

    /* Connect to NCS Cfg: PM, CTL */
    lib_master->nc_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "NcsCltCfg");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_NCS, immediate);
    if (rc < 0)
    {
        sal_printf("\n%% Connecting to CCS cfg %d times failed\n", immediate);
        return PM_E_INIT_FAIL;
    }
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->nc_sock = sock;
    
    /* Connect to CDS Data: PM */
    lib_master->data_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CdsCltData");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    if (PM_ID_SWITCH == lib_master->pm_id
     || PM_ID_ROUTED == lib_master->pm_id)
    {
        /* pack will result CDB sync msg delay send in ctc_sock_schedule(),
         * only open queue_en/pack_en in switch and routed process
         */
        ctc_sock_set_queue_en(sock, TRUE);
        ctc_sock_set_pack_en(sock, TRUE);
    }
    if (PM_ID_FEA == lib_master->pm_id)
    {
        ctc_sock_set_mutex(sock, TRUE);
    }

    rc = ctc_sock_connect(sock, SOCK_ADDR_CDS_DATA, immediate);
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->data_sock = sock;

    return rc;
}

int32
ipc_connect_ccs_subscribe(int32 immediate)
{
    lib_master_t *lib_master = lib_get_master();
    int32 rc;
    int32 sock = INVALID_SOCK_ID;

    /* Connect to CDS React: PM, CTL */
    lib_master->sub_cdb_sock = INVALID_SOCK_ID;
    sock = ctc_socket(CTC_SOCK_IPC, 0, "CdsCltSub");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    rc = ctc_sock_connect(sock, SOCK_ADDR_CCS_SUB_CDB, immediate);
    if (rc < 0)
    {
        sal_printf("\n%% Connecting to CCS CDB %d times failed\n", immediate);
        return PM_E_INIT_FAIL;
    }
    if (rc < 0)
    {
        return PM_E_INIT_FAIL;
    }
    lib_master->sub_cdb_sock = sock;

    return rc;
}

int32
ipc_connect_snmp_trap()
{
#ifdef CONFIG_SNMPAGT
    return 0;
#else
    lib_master_t *lib_master = lib_get_master();
    int32 rc;
    int32 sock = INVALID_SOCK_ID;
    ctc_sock_t *psock;

    if (lib_master->trap_sock != INVALID_SOCK_ID) {
        return PM_E_INITED;
    }

    sock = ctc_socket(CTC_SOCK_IPC, 0, "SnmpTrap");
    if (sock < 0)
    {
        return PM_E_INIT_FAIL;
    }
    ctc_socket_nonblocking(sock, 1);
    
    rc = ctc_sock_connect(sock, SOCK_ADDR_SNMP_TRAP, 0);
    if (rc < 0) {
        sal_printf("\n%% Connecting to SNMP trap agent failed\n");
        return PM_E_INIT_FAIL;
    }

    lib_master->trap_sock = sock;
    psock = ctc_sock_get_sock(sock);
    psock->flags |= M_SOCK_FLAG_NO_GLOBAL_CONNECT;
    return rc;
#endif
}

int32
ipc_sock_init()
{
    ctc_sock_init();
    ctc_sock_register_pm_connect_fn(_ipc_pm_connect_cb);

    return PM_E_NONE;
}

int32
ipc_sock_deinit()
{
    lib_master_t *lib_master = lib_get_master();

    if (lib_master->cfg_sock != INVALID_SOCK_ID)
    {
        ctc_sock_close(lib_master->cfg_sock);
        lib_master->cfg_sock = INVALID_SOCK_ID;
    }

    if (lib_master->data_sock != INVALID_SOCK_ID)
    {
        ctc_sock_close(lib_master->data_sock);
        lib_master->data_sock = INVALID_SOCK_ID;
    }
    
    if (lib_master->sub_cdb_sock != INVALID_SOCK_ID)
    {
        ctc_sock_close(lib_master->sub_cdb_sock);
        lib_master->sub_cdb_sock = INVALID_SOCK_ID;
    }

    if (lib_master->trap_sock != INVALID_SOCK_ID)
    {
        ctc_sock_close(lib_master->trap_sock);
        lib_master->trap_sock = INVALID_SOCK_ID;
    }

    if (lib_master->sub_tbl_sock != INVALID_SOCK_ID)
    {
        ctc_sock_close(lib_master->sub_tbl_sock);
        lib_master->sub_tbl_sock = INVALID_SOCK_ID;
    }

    return ctc_sock_deinit();
}

int32
ipc_sub_cdb_path_2_msg(cdb_node_t *p_node, cdb_node_t *p_ds_node, field_parse_t *p_field, char *p_data, uint32 data_len, ctc_msg_t *p_req_msg)
{
    p_req_msg->u_hdr.sub.type = CDB_SUB_CDB;
    
    if (p_ds_node)
    {
        p_req_msg->u_hdr.sub.node_type = CDB_NODE_DS;
        p_req_msg->u_hdr.sub.tbl_id = p_node->id;
        p_req_msg->u_hdr.sub.ds_id = 0;
    }
    else
    {
        if (CDB_NODE_TBL == p_node->type)
        {
            p_req_msg->u_hdr.sub.node_type = CDB_NODE_TBL;
        }
        else
        {
            p_req_msg->u_hdr.sub.node_type = CDB_NODE_DIR;
        }
        p_req_msg->u_hdr.sub.tbl_id = p_node->id;
        p_req_msg->u_hdr.sub.ds_id = 0;
    }
    p_req_msg->u_hdr.sub.field = p_field->field_id[0];
    p_req_msg->u_hdr.sub.pid = sal_getpid();
    p_req_msg->u_hdr.sub.src = PM_ID_CTL;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    
    return PM_E_NONE;
}

int32
ipc_ctl_to_cds_sub_cdb(cdb_node_t *p_node, cdb_node_t *p_ds_node, field_parse_t *p_field, char *p_data, uint32 data_len)
{
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_SUBSCRIBE, data_len);

    rc = ipc_sub_cdb_path_2_msg(p_node, p_ds_node, p_field, p_data, data_len, p_req_msg);
    if (rc < 0)
    {
        sal_printf("Invalid CDB path %s\n", p_data);
        return PM_E_FAIL;
    }
    
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    
    rc = ctc_sock_send(master->sub_cdb_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }

    return PM_E_NONE;
}

int32
ipc_ctl_to_cds_sub_tbl(uint32 *tbl_id, uint32 tbl_num, uint32 sync, uint32 snmp)
{
    uint32 data_len = 0;
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    data_len = tbl_num * sizeof(uint32);
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_SUBSCRIBE, data_len);

    p_req_msg->u_hdr.sub.type = CDB_SUB_TBL;
    p_req_msg->u_hdr.sub.node_type = sync;
    p_req_msg->u_hdr.sub.tbl_id = snmp;
    p_req_msg->u_hdr.sub.ds_id = 0;
    p_req_msg->u_hdr.sub.tbl_num = tbl_num;
    p_req_msg->u_hdr.sub.field = 0;
    p_req_msg->u_hdr.sub.pid = sal_getpid();
    p_req_msg->u_hdr.sub.src = PM_ID_CTL;
    
    p_req_msg->data_len = data_len;
    p_req_msg->data = (void*)tbl_id;
    
    rc = ctc_sock_send(master->sub_cdb_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }
    
    return PM_E_NONE;
}

int32
ipc_ctl_to_cds_sub_tbl_resync()
{
    uint32 data_len = 0;
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_SUBSCRIBE, data_len);

    p_req_msg->u_hdr.sub.type = CDB_SUB_TBL;
    p_req_msg->u_hdr.sub.node_type = 0xFF;
    p_req_msg->u_hdr.sub.tbl_id = 0;
    p_req_msg->u_hdr.sub.ds_id = 0;
    p_req_msg->u_hdr.sub.tbl_num = 0;
    p_req_msg->u_hdr.sub.field = 0;
    p_req_msg->u_hdr.sub.pid = sal_getpid();
    p_req_msg->u_hdr.sub.src = PM_ID_CTL;
    
    p_req_msg->data_len = data_len;
    p_req_msg->data = NULL;
    
    rc = ctc_sock_send(master->sub_cdb_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }
    
    return PM_E_NONE;
}

int32
ipc_ctl_to_ccs_sub_req(uint8 *data, uint32 data_len)
{
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_SUB_REQ, data_len);

    p_req_msg->data_len = data_len;
    p_req_msg->data = data;
    
    rc = ctc_sock_send(master->sub_cdb_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }
    
    return PM_E_NONE;
}

int32
ipc_ctl_to_ccs_cfg_sdb(cdb_oper_t oper, uint16 node_type, uint16 node_id, void *p_data, uint32 data_len, ctc_msg_t *p_resp_msg)
{
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CFG, data_len);
    p_req_msg->u_hdr.cfg.dest = PM_ID_CDS;
    p_req_msg->u_hdr.cfg.src = PM_ID_CTL;
    p_req_msg->u_hdr.cfg.pid = sal_getpid();
    p_req_msg->u_hdr.cfg.oper = oper;
    p_req_msg->u_hdr.cfg.type = node_type;
    p_req_msg->u_hdr.cfg.id = node_id;
    p_req_msg->u_hdr.cfg.cfg = CDB_CFG_CLI;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    
    rc = ctc_sock_send_sync(master->cfg_sock, IGNORE_SOCK_PEER, p_req_msg, p_resp_msg, g_ipc_cfg_sync_time * SOCK_TICS_PER_SEC);
    if (rc < 0)
    {
        if (-1 == rc)
        {
            sal_printf("\n%% Command execute %d seconds timeout\n", g_ipc_cfg_sync_time);
        }
        return -1;
    }

    return PM_E_NONE;
}

int32
ipc_ctl_to_ccs_cfg(cdb_oper_t oper, cdb_node_t* p_node, void *p_data, uint32 data_len, ctc_msg_t *p_resp_msg)
{
    lib_master_t *master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc;

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CFG, data_len);
    p_req_msg->u_hdr.cfg.dest = PM_ID_CDS;
    p_req_msg->u_hdr.cfg.src = PM_ID_CTL;
    p_req_msg->u_hdr.cfg.pid = sal_getpid();
    p_req_msg->u_hdr.cfg.oper = oper;
    p_req_msg->u_hdr.cfg.type = p_node->type;
    p_req_msg->u_hdr.cfg.id = p_node->id;
    p_req_msg->u_hdr.cfg.cfg = CDB_CFG_CLI;
    p_req_msg->data_len = data_len;
    p_req_msg->data = p_data;
    
    rc = ctc_sock_send_sync(master->cfg_sock, IGNORE_SOCK_PEER, p_req_msg, p_resp_msg, g_ipc_cfg_sync_time * SOCK_TICS_PER_SEC);
    if (rc < 0)
    {
        if (-1 == rc)
        {
            sal_printf("\n%% Command execute %d seconds timeout\n", g_ipc_cfg_sync_time);
        }
        return -1;
    }

    return PM_E_NONE;
}

int32
ipc_register_rx_cb()
{
    lib_master_t *lib_master = lib_get_master();

    switch (lib_master->pm_id)
    {
    case PM_ID_CDS:
        /* config */
        break;

    case PM_ID_FEA:
        /* fei, fea */
        break;

    case PM_ID_CHSM:
        /* chsm, lcm */
        break;

    default:
        ctc_sock_register_rx_fn(ipc_pm_process_msg);
        break;
    }
    if (lib_master->pm_id)
    {

    }
    
    return PM_E_NONE;
}

static int32 g_ipc_pkt_sock = -1;

int32
ipc_pkt_init(int32 immediate)
{
    const char *sock_path = SOCK_ADDR_PM_PKT;
    const char *desc = "IpcPkt";
    int32 sock = INVALID_SOCK_ID;
    int32 rc = PM_E_NONE;

    sock = ctc_socket(CTC_SOCK_IPC_PKT, 0, desc);
    rc = ctc_sock_connect(sock, sock_path, immediate);
    if (rc < 0)
    {
        return rc;
    }

    g_ipc_pkt_sock = sock;

    return PM_E_NONE;
}

int32
ipc_pkt_tx(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid, uint32 pkt_type, uint32 flags)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
   
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_PKT, pkt_len);
    p_req_msg->u_hdr.pkt.ifindex = ifindex;
    p_req_msg->u_hdr.pkt.vid = vid;
    p_req_msg->u_hdr.pkt.type = pkt_type;
    p_req_msg->u_hdr.pkt.flags = flags;
    p_req_msg->data_len = pkt_len;
    p_req_msg->data = p_pkt;

    rc = ctc_sock_send(g_ipc_pkt_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }

    return PM_E_NONE;
}

int32
ipc_pkt_tx_packet_out(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid, uint32 pkt_type, uint32 flags, glb_openflow_packet_out_param_t* p_tail)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
   
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_PKT, pkt_len+sizeof(glb_openflow_packet_out_param_t));
    p_req_msg->u_hdr.pkt.ifindex = ifindex;
    p_req_msg->u_hdr.pkt.vid = vid;
    p_req_msg->u_hdr.pkt.type = pkt_type;
    p_req_msg->u_hdr.pkt.flags = flags;
    p_req_msg->data_len = pkt_len;
    p_req_msg->data = p_pkt;
    if(NULL != p_tail)
    {
        p_req_msg->u_hdr.pkt.sub_type = p_tail->type;
        p_req_msg->data1_len = sizeof(glb_openflow_packet_out_param_t);
        p_req_msg->data1 = p_tail;
    }

    rc = ctc_sock_send(g_ipc_pkt_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }

    return PM_E_NONE;
}

int32
ipc_pkt_tx_aux(uint8 *p_pkt, uint32 pkt_len, uint8 *p_aux, uint32 aux_len, uint32 ifindex, vid_t vid, uint32 pkt_type, uint32 flags)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;
   
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_PKT, pkt_len + aux_len);
    p_req_msg->u_hdr.pkt.ifindex = ifindex;
    p_req_msg->u_hdr.pkt.vid = vid;
    p_req_msg->u_hdr.pkt.type = pkt_type;
    p_req_msg->u_hdr.pkt.flags = flags;
    p_req_msg->data_len = pkt_len;
    p_req_msg->data = p_pkt;
    p_req_msg->data1_len = aux_len;
    p_req_msg->data1 = p_aux;
    
    rc = ctc_sock_send(g_ipc_pkt_sock, IGNORE_SOCK_PEER, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }

    return PM_E_NONE;
}

int32
ipc_pkt_tx_bpdu(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_BPDU, 0);
}

int32
ipc_pkt_tx_lacp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_LACP, 0);
}

int32
ipc_pkt_tx_arp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid)
{
    /* modified by liwh for bug 51977, 2019-06-13 */
    uint32   pkt_new_len = 60;
    
    if (pkt_len > 60)
    {
        pkt_new_len = pkt_len;
    }
    /* liwh end */

    return ipc_pkt_tx(p_pkt, pkt_new_len, ifindex, vid, GLB_PKT_ARP, 0);
}

int32
ipc_pkt_tx_openflow(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_OPENFLOW, 0);
}

int32
ipc_pkt_tx_openflow_packet_out(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, glb_openflow_packet_out_param_t* p_tail)
{
    return ipc_pkt_tx_packet_out(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_OPENFLOW, 0, p_tail);
}


int32
ipc_pkt_tx_sflow(uint8 *p_pkt, uint32 pkt_len, uint32 address, vid_t vid)
{
    return ipc_pkt_tx(p_pkt, pkt_len, address, vid, GLB_PKT_SFLOW, 0);
}

int32
ipc_pkt_tx_dhcp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid, int flags)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, vid, GLB_PKT_DHCP, flags);
}

int32
ipc_pkt_tx_lldp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_L2_LLDP, 0);
}

int32
ipc_pkt_tx_erps(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_ERPS, 0);
}

int32
ipc_pkt_tx_igmp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, vid, GLB_PKT_IGMP, 0);
}

int32
ipc_pkt_tx_igmp_flood(uint8 *p_pkt, uint32 pkt_len, uint32 source_ifindex, vid_t vid)
{
    return ipc_pkt_tx(p_pkt, pkt_len, source_ifindex, vid, GLB_PKT_IGMP, GLB_PKT_FLAGS_TX_MCAST);
}

int32
ipc_pkt_tx_eapol(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_EAPOL, 0);
}

int32
ipc_pkt_tx_ptp(uint8 *p_pkt, uint32 pkt_len, uint8 *p_aux, uint32 aux_len, uint32 ifindex, uint32 flags)
{
    return ipc_pkt_tx_aux(p_pkt, pkt_len, p_aux, aux_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_PTP, flags);
}

int32
ipc_pkt_tx_tpoam(uint8 *p_pkt, uint32 pkt_len, uint8 *p_aux, uint32 aux_len, uint32 ifindex, uint32 flags)
{
    return ipc_pkt_tx_aux(p_pkt, pkt_len, p_aux, aux_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_TPOAM, flags);
}

int32
ipc_pkt_tx_vrrp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid)
{
    if (GLB_ALL_VLAN_ID == vid)
    {
        return ipc_pkt_tx(p_pkt, pkt_len, ifindex, vid, GLB_PKT_VRRP, 0);
    }
    else
    {
        return ipc_pkt_tx(p_pkt, pkt_len, 0, vid, GLB_PKT_VRRP, GLB_PKT_FLAGS_TX_MCAST);
    }
}

int32
ipc_pkt_tx_ssm(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, GLB_ALL_VLAN_ID, GLB_PKT_SSM, 0);
}

/* support nd modified by liwh for bug 47547, 2018-06-28 */
int32
ipc_pkt_tx_nd(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, vid, GLB_PKT_ND, 0);
}
/* liwh end */

int32
ipc_pkt_tx_raps(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid)
{
    return ipc_pkt_tx(p_pkt, pkt_len, ifindex, vid, GLB_PKT_RAPS, 0);
}


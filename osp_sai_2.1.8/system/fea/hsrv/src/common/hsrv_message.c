
#include "hsrv_inc.h"

hsrv_message_master_t *g_pst_hsrv_msg_master = NULL;

hsrv_msg_cnt_t g_hsrv_hsrv2hagt_msg_cnt[opHsrv2Agt_PR_hsrv2AgtMsgMax];
hsrv_msg_cnt_t g_hsrv_hagt2hsrv_msg_cnt[opHagt2Srv_PR_hagt2SrvMsgMax];

int32
hsrv_message_get_curr_valid_chip(uint32 *p_chip_bmp)
{
    uint8 gchipid = 0;

    if(NULL == p_chip_bmp)
    {
        return HSRV_E_INVALID_PARAM;
    }

#ifdef TODO
    if (module_control_is_enable_stack())
    {
        *p_chip_bmp = g_pst_hsrv_msg_master->curr_valid_chip;
    }
    else
#endif
    {
        *p_chip_bmp= (1 << gchipid);
    }
    return HSRV_E_NONE;
}

static int32
_hsrv_message_dispatch_msg_from_fei(int32 sock, ctc_msg_t *pst_msg)
{
    uint32 cmd = 0;
    int32 rc = HSRV_E_NONE;
    
    if (MSG_OPER_FEI != pst_msg->hdr.operation)
    {
        return HSRV_E_INVALID_PARAM;
    }

    cmd = pst_msg->u_hdr.fei.type;
    if(g_pst_hsrv_msg_master->hsrv_hal_msg_cb[cmd])
    {
        rc = (g_pst_hsrv_msg_master->hsrv_hal_msg_cb[cmd])(pst_msg->data);
        if (rc < 0 && rc != HSRV_E_RESOURCE_FULL)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Process msg (%d) error, error code(%d)", cmd, rc);   
            return rc;
        }
    }
    else
    {
        log_sys(M_MOD_INTERNAL, E_WARNING, "Empty process for msg 0x%x", pst_msg->u_hdr.fei.type);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_message_dispatch_msg_from_hagt(Hagt2SrvMsg_t *pst_msg)
{
    if((NULL == pst_msg) || (pst_msg->opHagt2Srv.present >= opHagt2Srv_PR_hagt2SrvMsgMax))
    {
        return HSRV_E_INVALID_PARAM;
    }

    g_hsrv_hagt2hsrv_msg_cnt[pst_msg->opHagt2Srv.present].curr_cnt++; 
    
    if(NULL == g_pst_hsrv_msg_master || 
        (NULL == g_pst_hsrv_msg_master->hagt2srv_msg_cb[pst_msg->opHagt2Srv.present]))
    {
        return HSRV_E_NOT_INIT;
    }
    return (g_pst_hsrv_msg_master->hagt2srv_msg_cb[pst_msg->opHagt2Srv.present](pst_msg));
}

extern int32
lcm_clnt_process_lcm_msg(int32 sock, ctc_msg_t *p_msg);

extern int32
hsrv_packet_event_send(int32 sock, ctc_msg_t *p_msg);

int32
hsrv_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CFG:
        return cfg_cmd_process(sock, p_msg);

    case MSG_OPER_DATA:
        return ipc_pm_process_data_sync(sock, p_msg);

    case MSG_OPER_FEI:
        return _hsrv_message_dispatch_msg_from_fei(sock, p_msg);
        
    case MSG_OPER_PKT:
        return hsrv_packet_event_send(sock, p_msg);

    case MSG_OPER_LCM:
        return lcm_clnt_process_lcm_msg(sock, p_msg);

    default:
        return -1;
    }
    
    return 0;
}

int32
hsrv_message_set_cb_from_hagt(opHagt2Srv_PR msg_type, HSRV_MESSAGE_CB_FUNC pf_func)
{
  if (msg_type >= opHagt2Srv_PR_hagt2SrvMsgMax || NULL == pf_func )
  {
      return HSRV_E_INVALID_PARAM;
  }
  
  if (NULL == g_pst_hsrv_msg_master)
  {
      return HSRV_E_NOT_INIT;
  }
  
  g_pst_hsrv_msg_master->hagt2srv_msg_cb[msg_type] = pf_func;

  
  return HSRV_E_NONE;
}

int32
hsrv_sock_init(hsrv_message_master_t *master)
{
    lib_master_t *lib_master = lib_get_master();
    /* server for FEI */
    int32 sock = INVALID_SOCK_ID;
    int32 sock_high = INVALID_SOCK_ID;

    sock = ctc_socket(CTC_SOCK_IPC, 0, "HsrvNormal");
    ctc_sock_set_pmid(sock, lib_master->pm_id);
    ctc_sock_set_mutex(sock, TRUE);
    ctc_sock_bind(sock, SOCK_ADDR_FEI);
    
    sock_high = ctc_socket(CTC_SOCK_IPC, 0, "HsrvHigh");
    ctc_sock_set_pmid(sock_high, lib_master->pm_id);
    ctc_sock_set_mutex(sock_high, TRUE);
    ctc_sock_bind(sock_high, SOCK_ADDR_FEI_HIGH);

    master->fei_sock = sock;
    master->fei_sock_high = sock_high;

    ipc_connect_bhm();
    
    /* client to CDS */    
    HSRV_IF_ERROR_RETURN(ipc_connect_ccs_cds(10));
    
    HSRV_IF_ERROR_RETURN(ctc_sock_register_rx_fn(hsrv_process_msg));
    return HSRV_E_NONE;
}

int32
hsrv_message_module_start(void **pp_msg_master)
{
    hsrv_message_master_t *pst_message_info = NULL;

    pst_message_info = HSRV_CALLOC(MEM_FEA_TEMP, sizeof(hsrv_message_master_t));
    if (NULL == pst_message_info)
    {
        *pp_msg_master = NULL;
        return HSRV_E_NO_MEMORY;
    }

    pst_message_info->fei_sock = INVALID_SOCK_ID;
    pst_message_info->fei_sock_high = INVALID_SOCK_ID;
    *pp_msg_master = pst_message_info;
    g_pst_hsrv_msg_master = pst_message_info;
    HSRV_IF_ERROR_RETURN(hsrv_sock_init(g_pst_hsrv_msg_master));
    
    return HSRV_E_NONE;
}

int32
hsrv_message_set_cb_from_fei(hsrv_msg_type_t msg_type, HSRV_MESSAGE_CB_FUNC cb)
{
    if (msg_type >= FEI_MSG_TYPE_MAX || NULL == cb)
    {
        return HSRV_E_INVALID_PARAM;
    }

    if (NULL == g_pst_hsrv_msg_master)
    {
        return HSRV_E_NOT_INIT;
    }

    g_pst_hsrv_msg_master->hsrv_hal_msg_cb[msg_type] = cb;
    return HSRV_E_NONE;
}

int32
hsrv_message_send_msg_to_hagt(Hsrv2AgtMsg_t *p_msg)
{
    int32 rc = HSRV_E_NONE;
#ifndef _GLB_DISTRIBUTE_SYSTEM_
extern int32 hagt_message_dispatch_msg_from_hsrv(Hsrv2AgtMsg_t *pst_msg);
    g_hsrv_hsrv2hagt_msg_cnt[p_msg->opHsrv2Agt.present].curr_cnt++;
    rc = hagt_message_dispatch_msg_from_hsrv(p_msg);
#endif
    return rc;            
}

int32
hsrv_message_send_ucast_msg_to_hagt(hsrv_hagt_info_t *p_hagt_info, 
                                    Hsrv2AgtMsg_t *p_msg)
{
    return -1;
}

int32
hsrv_message_send_msg_to_hal(uint32 cmd, void *req, uint32 req_len, uint32 high, pm_id_t pm_id)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 sock = INVALID_SOCK_ID;
    int32 rc = HSRV_E_NONE;

    sock = (high) ? g_pst_hsrv_msg_master->fei_sock_high : g_pst_hsrv_msg_master->fei_sock;
    if (sock < 0)
    {
        return HSRV_E_SOCK_WRITE_ERR;
    }

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_FEI, req_len);
    p_req_msg->u_hdr.fei.type = cmd;
    p_req_msg->u_hdr.fei.flags = 0;
    p_req_msg->data_len = req_len;
    p_req_msg->data = req;
    
    rc = ctc_sock_send(sock, pm_id, p_req_msg);
    if (rc < 0)
    {
        return rc;
    }
    
    return HSRV_E_NONE;
}


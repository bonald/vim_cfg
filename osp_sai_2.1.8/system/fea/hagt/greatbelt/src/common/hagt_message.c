/******************************************************************************
 * hagt_msg.c   :    hal agent message mechanism:
 *                         1. tcp socket: receive msg for DB-syn, and send msg to hal server
 *                         2. rmt socket: reciev multicast msg from hal server
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Zhu Zhengdong
 * Date          :       2009-3-4
 * Reason      :       First Create.
 *****************************************************************************/
 
 /******************************************************************************
  *
 * Header Files 
 *
 ******************************************************************************/
#include "hagt_inc.h"
#include "hagt_message_priv.h"

#if 0
#include "ctclib_show.h"
#include "lcapi.h"
#include "hagt_lcmtlk.h"
#include "glb_distribute_system_define.h"

#include <linux/sockios.h>
#include <netinet/tcp.h>
#include "glb_macro.h"
#include "Hsrv2AgtMsg.h"
#include "glb_rmt.h"
#include "glb_if_define.h"
#include "hagt_stack.h"

/******************************************************************************
 *  
* Defines and Macros
*
******************************************************************************/


/******************************************************************************
 *  
* Global and Declaration
*
******************************************************************************/
#endif
static hagt_message_info_t *g_pst_hagt_message = NULL;
int g_hagt_hal_msgid = 0;

#if 0

/******************************************************************************
 *  
* Function
*
******************************************************************************/

/*********************************************************************
 * Name    : hagt_rmt_debug_dump
 * Purpose : This function used to dump rmt info
 * Input   : N/A
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx 
 * Note    : N/A
*********************************************************************/
int32
hagt_rmt_debug_dump(void)
{
#ifdef _GLB_DISTRIBUTE_SYSTEM_
#ifdef _GLB_ENABLE_DBGSHOW_
    struct rmt_dump rmt_info;
    int ret = 0;
    
    memset(&rmt_info, 0, sizeof(rmt_info));
    if (!g_pst_hagt_message || g_pst_hagt_message->hagt_msg_rmt.sock < 0)
    {
        return 0;
    }

    
    ret = ioctl(g_pst_hagt_message->hagt_msg_rmt.sock, RMT_IOC_DUMP, &rmt_info);
    if (ret < 0)
    {
        return 0;
    }

    CTCLIB_DUMP_OUT("------------------------------------------------------------------------------------\n");
    CTCLIB_DUMP_OUT("                  HAGT RMT DATABASE                                                 \n");
    CTCLIB_DUMP_OUT("------------------------------------------------------------------------------------\n");

    CTCLIB_DUMP_OUT(" \n");
    CTCLIB_DUMP_OUT("r_maxqlen:   %d\n", rmt_info.r_maxqlen);                                   
    CTCLIB_DUMP_OUT("r_initseq:   %d\n", rmt_info.r_initseq);                                   
    CTCLIB_DUMP_OUT("r_lastseq:   %d\n", rmt_info.r_lastseq);                                   
    CTCLIB_DUMP_OUT("r_nextseq:   %d\n", rmt_info.r_nextseq);                                   
    CTCLIB_DUMP_OUT("r_nack_retries:  %d\n", rmt_info.r_nack_retries);                          
    CTCLIB_DUMP_OUT("#################### RMT RX ####################\n");                      
    CTCLIB_DUMP_OUT("rx_all:    %d\n", rmt_info.rx_all);                                        
    CTCLIB_DUMP_OUT("rx_nack:   %d\n", rmt_info.rx_nack);                                       
    CTCLIB_DUMP_OUT("rx_data:   %d\n", rmt_info.rx_data);                                       
    CTCLIB_DUMP_OUT("rx_idle:   %d\n", rmt_info.rx_idle);                                       
    CTCLIB_DUMP_OUT("rx_qlen:   %d\n", rmt_info.rx_qlen);                                       
    CTCLIB_DUMP_OUT("################# RMT RX Error #################\n");                      
    CTCLIB_DUMP_OUT("nack_rx_group:                %d\n", rmt_info.nack_rx_group);              
    CTCLIB_DUMP_OUT("nack_rx_length:               %d\n", rmt_info.nack_rx_length);             
    CTCLIB_DUMP_OUT("data_rx_group:                %d\n", rmt_info.data_rx_group);              
    CTCLIB_DUMP_OUT("data_rx_length:               %d\n", rmt_info.data_rx_length);             
    CTCLIB_DUMP_OUT("data_rx_2old:                 %d\n", rmt_info.data_rx_2old);               
    CTCLIB_DUMP_OUT("data_rx_overflow:             %d\n", rmt_info.data_rx_overflow);           
    CTCLIB_DUMP_OUT("data_rx_queue_fail:           %d\n", rmt_info.data_rx_queue_fail);         
    CTCLIB_DUMP_OUT("data_rx_overflow_r_queue:     %d\n", rmt_info.data_rx_overflow_r_queue);   
    CTCLIB_DUMP_OUT("data_rx_queue_fail_r_queue:   %d\n", rmt_info.data_rx_queue_fail_r_queue); 
    CTCLIB_DUMP_OUT("data_rx_duplicated:           %d\n", rmt_info.data_rx_duplicated);         
    CTCLIB_DUMP_OUT("idle_rx_group:                %d\n", rmt_info.idle_rx_group);              
    CTCLIB_DUMP_OUT("idle_rx_2old:                 %d\n", rmt_info.idle_rx_2old);               
    CTCLIB_DUMP_OUT("rcv_skb_share_check:          %d\n", rmt_info.rcv_skb_share_check);        
    CTCLIB_DUMP_OUT("rcv_skb_may_pull:             %d\n", rmt_info.rcv_skb_may_pull);           
    CTCLIB_DUMP_OUT("rcv_skb_rmt_type:             %d\n", rmt_info.rcv_skb_rmt_type);           
    CTCLIB_DUMP_OUT(" ===================================================\n");
    
#endif
#endif
    return HAGT_E_NONE;
}

#endif

/************************************************************************
 * Name        : hagt_msg_set_callback(op_PR msg_type, HAGT_CALLBACK_FUNC func)
 * Purpose     : set callback function for msg received from hal-server
 * Input         : msg type and callback function ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
int32
hagt_message_set_msg_callback(opHsrv2Agt_PR msg_type, HAGT_MESSAGE_CB_FUNC pf_func)
{

    if (msg_type >= opHsrv2Agt_PR_hsrv2AgtMsgMax || NULL == pf_func)
    {
        return HAGT_E_INVALID_PARAM;
    }
  
    if(NULL == g_pst_hagt_message)
    {
        return HAGT_E_NOT_INIT;
    }
    
    g_pst_hagt_message->hsrv2agt_msg_cb[msg_type] = pf_func;
  
  return HAGT_E_NONE;
}

int32
hagt_message_dispatch_msg_from_hsrv(Hsrv2AgtMsg_t *pst_msg)
{
    int32 ret = HAGT_E_NONE;

    HAGT_CHECK_PTR(pst_msg);

    if (hagt_is_enable_stack())
    {
        if (!GLB_FLAG_ISSET(pst_msg->agentId, (1 << hagt_get_phy_slotid())))
        {
             return HAGT_E_NONE;
        }
    }
    
    if(pst_msg->opHsrv2Agt.present >= opHsrv2Agt_PR_hsrv2AgtMsgMax)
    {
        HAGT_LOG_ERR("Invalid opcode %d", pst_msg->opHsrv2Agt.present);
        return HAGT_E_INVALID_PARAM;
    }

    if(NULL == g_pst_hagt_message || 
        (NULL == g_pst_hagt_message->hsrv2agt_msg_cb[pst_msg->opHsrv2Agt.present]))
    {
        HAGT_LOG_ERR("Not ready for opcode %d", pst_msg->opHsrv2Agt.present);
        return HAGT_E_NOT_INIT;
    }
    ret = g_pst_hagt_message->hsrv2agt_msg_cb[pst_msg->opHsrv2Agt.present](pst_msg);
    if (HAGT_E_NONE == ret)
    {
        return ret;
    }
    else if (GLB_E_RESOURCE_FULL == ret)
    {
        return ret;
    }
    else
    {
        HAGT_LOG_ERR("Opcode %d failed: rv %d", pst_msg->opHsrv2Agt.present, ret);
        return HAGT_E_NONE;
    }
}

#if 0
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "glb_const.h"

#define ETH_P_SC  GLB_ETH_P_ASIC

int sock_asic; /* from/to bay/humber */
int sock_sc;  /* from/to supervisor card */
int g_stack_send_flag = FALSE;

int
create_raw_packet_socket(uint16_t ether_proto, const char *if_name)
{
    int sock;
    struct sockaddr_ll sll;
    int val;

    sock = socket(PF_PACKET, SOCK_RAW, htons(ether_proto));
    if (sock < 0)
        return -1;

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex(if_name);

    if (sll.sll_ifindex == 0) {
        close(sock);
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        close(sock);
        return -1;
    }

    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);

    return sock;
}

int peth_start()
{
#if 0
    lc_slot_no = dev_get_phy_slot();
    sc_slot_no = epld_get_master_sup_phy_slot();
#endif

#if CTC_IS_UML
    sock_asic = create_raw_packet_socket(GLB_ETH_P_ASIC, "eth98");
#else
    sock_asic = create_raw_packet_socket(GLB_ETH_P_ASIC, "eth0");
#endif
    if (sock_asic < 0)
        return -1;

#if 0
    if (afx_mio_create(&mio_bay, sock_bay, AFX_IO_IN, sock_bay_read, NULL) < 0) {
        close(sock_bay);
        sock_bay = -1;
        return -1;
    }
#endif

#if CTC_IS_UML
    sock_sc = create_raw_packet_socket(ETH_P_SC, "eth100");
#else
    sock_sc = create_raw_packet_socket(ETH_P_SC, "eth0");
#endif
    if (sock_sc < 0) {
        close(sock_asic);
        sock_asic = -1;
        return -1;
    }

#if 0
    if (afx_mio_create(&mio_sc, sock_sc, AFX_IO_IN, sock_sc_read, NULL) < 0) {
        afx_mio_destroy(mio_bay);
        close(sock_bay);
        sock_bay = -1;
        close(sock_sc);
        sock_sc = -1;
        return -1;
    }
#endif

    return 0;
}

static int32
_hagt_message_get_master_svc_phy_slot_id()
{
    lcapi_lcm_get_active_sup_slot_t msg;

    HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_GET_ACTIVE_SUP_SLOT, &msg)); 
    return msg.slot;
}

#ifdef _GLB_DISTRIBUTE_SYSTEM_
static int32
_hagt_message_get_svc_ip(uint8_t slot, void *dst)
{
    char *src = NULL;

    switch (slot)
    {
        case 1:
            src = "172.16.1.1"; /* need to define macro */
            break;
            
        case 2:
            src = "172.16.2.1"; /* need to define macro */
            break;

        default:
            src = NULL;
            break; 
    }

    sal_inet_pton(AF_INET, src, (char *)dst);
    return HAGT_E_NONE;
}
#endif

static int32
_hagt_message_get_master_ip(uint8_t slot, void *dst)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "172.16.0.%d", slot);
    sal_inet_pton(AF_INET, (const char *)buf, dst);
    return 0;
}



/************************************************************************
 * Name        : _hagt_message_recv_msg_from_hsrv(hagt_session_t *hagt_sess, HalMsg_t *msg)
 * Purpose     : process msg received from hal-server:
 *                    if hal agent is doing db-syn, msg will be pending, after db-syn is done, 
 *                    hal-agent will start a timer to process pending msg.
 * Input         : msg ptr and hal agent session ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
static int32
_hagt_message_recv_msg_from_hsrv(hagt_message_session_t *p_hagt_session, Hsrv2AgtMsg_t *msg)
{
    int ret;
    hagt_message_node_t *msg_p;

    if (!GLB_FLAG_ISSET(msg->agentId, (1 << p_hagt_session->phy_slot_id)))
    {
        return HAGT_E_NONE;
    }
    
    if(msg->opHsrv2Agt.present >= opHsrv2Agt_PR_hsrv2AgtMsgMax)
    {
        HAGT_LOG_ERR("hagt receive invalid msg(%d)", msg->opHsrv2Agt.present);
        return HAGT_E_MSG_TYPE_EER;
    }
    
    HAGT_MSG_LOG("Hal agent slot(%d) Rcv Msg(%d)",
                  p_hagt_session->phy_slot_id, msg->opHsrv2Agt.present); 

    g_hagt_hal_msgid = msg->sourceId;
    
    if (!g_pst_hagt_message->hsrv2agt_msg_cb[msg->opHsrv2Agt.present])
    {
        HAGT_LOG_ERR("Empty process for Msg(%d)",msg->opHsrv2Agt.present);
        return HAGT_E_NONE;
    }

    switch(p_hagt_session->session_type)
    {
        case HAGT_MESSAGE_SESSION_TCP:/*This message is just used for db sync*/
            HAGT_MSG_LOG("Rcv unicast msg"); 
            if(FALSE == g_pst_hagt_message->db_syning &&
                opHsrv2Agt_PR_hsrv2AgtMsgRmtInitSeqReq != msg->opHsrv2Agt.present)
            {
                HAGT_LOG_ERR("Hagt DB init have done, should not receive ucast message");
                return HAGT_E_NONE;
            }
            ret = hagt_message_dispatch_msg_from_hsrv(msg);
            if(ret)
            {
                HAGT_LOG_ERR("Hagt ucast message process fail, ret(%d)", ret);
            }
            break;

        case HAGT_MESSAGE_SESSION_RMT:
            HAGT_MSG_LOG("Rcv rmt msg"); 
            if(FALSE == g_pst_hagt_message->db_syning &&
                ctclib_list_empty(&g_pst_hagt_message->pending_msg_list))
            {
                ret = hagt_message_dispatch_msg_from_hsrv(msg);
                if(ret)
                {
                    HAGT_LOG_ERR("Hagt ucast message process fail, ret(%d)", ret);
                }
            }
            else/*Pending message while doing db sync*/
            {
                msg_p = HAGT_MALLOC(CTCLIB_MEM_HAGT_MSG_MODULE, sizeof(hagt_message_node_t));
                if (!msg_p)
                {
                    HAGT_LOG_ERR("Out of memory while malloc pending msg node");
                    return HAGT_E_NONE;
                }
                msg_p->msg = msg;
                p_hagt_session->msg = NULL;/*msg memory should freed after msg process*/
                
                HAGT_MSG_LOG("add multicast msg to the pending list"); 
                ctclib_list_insert_tail(&(g_pst_hagt_message->pending_msg_list), &(msg_p->head));
            }
            break;

        default:
            HAGT_LOG_ERR("Invalid hagt message session(%d)",p_hagt_session->session_type);
            return HAGT_E_INVALID_MSG_SESSION;
    }
    
    return HAGT_E_NONE;
}

int32
hagt_message_send_msg_to_hsrv_by_socket(Hagt2SrvMsg_t *p_msg)
{
    asn_enc_rval_t enc_rc;
    hagt_message_session_t *p_ucast_session;
    int32 len, i, bytes;
    uint8 buf[GLB_HAGT2SRV_MSG_BUF_LEN], *ptr;

    if (!p_msg || !g_pst_hagt_message)
    {
        return HAGT_E_INVALID_PTR;
    }

    p_ucast_session = &(g_pst_hagt_message->hagt_msg_ucast);
    if (!p_ucast_session->has_init)
    {
        HAGT_LOG_ERR("hagt message send to hsrv fail, not init");
        return HAGT_E_NOT_INIT;
    }
    
    p_msg->ver = 1;
    p_msg->id = p_ucast_session->msg_id++;
    p_msg->agentId = 1 << hagt_get_phy_slotid();
    
    len = GLB_HAGT2SRV_MSG_BUF_LEN;

#ifdef HAGT_MSG_DEBUG
    xer_fprint(stdout, &asn_DEF_Hagt2SrvMsg, p_msg);
#endif
    enc_rc = der_encode_to_buffer(&asn_DEF_Hagt2SrvMsg, p_msg, buf, (size_t *)&len);
    if (enc_rc.encoded < 0)
    {
        HAGT_LOG_ERR("MSG encode error");
        return HAGT_E_MSG_ENCODE_EER;
    }
    
#define HAGT_MAX_SEND_COUNT 5000
    i = 0;
    ptr = buf;
    while(i < HAGT_MAX_SEND_COUNT)
    {
        bytes = send(p_ucast_session->sock, ptr, len, 0);
        if (bytes <= 0) 
        {
            switch (errno)
            {
                case EINTR:
                case EAGAIN:
                case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
                case EWOULDBLOCK:
#endif 
                  usleep (1);
                  i++;
                  // TODO: hagt_msg_block_cnt++;
                  continue;
            }          
            HAGT_LOG_ERR("hal agent send msg to hal server error");
            if(p_ucast_session->p_thread_read)
            {
                /* Modified by kcao 2009-07-08 for bug 8930 : reload linecard if send error */
                // TODO: lcm_reload();
            }

#if 0 // TODO:
            if (hagt_msg_single_block_cnt < i)
            {
                hagt_msg_single_block_cnt = i;
            }
#endif            
            return HAGT_E_SEND_MSG_FAIL;
        }
        else if (len == bytes)
        {
#if 0 // TODO:
            if (hagt_msg_single_block_cnt < i)
            {
                hagt_msg_single_block_cnt = i;
            }         
#endif            
            return HAGT_E_NONE;
        }
        else
        {
            len -= bytes;
            ptr += bytes;
        }
        i++;
    }  

    HAGT_LOG_ERR("hal agent send msg to hal server time out (%d times), msg type(%d)",
                i, p_msg->opHagt2Srv.present);

    /* Added by kcao 2010-03-30 for bug 11643
     * in this case, maybe send half-baked msg to shal and result in shal decode error and can't resume 
     */
    // TODO: lcm_reload();
    
    return HAGT_E_NONE;
}

/************************************************************************
 * Name        : hagt_sess_read(int fd, afx_io_dir_t dir, void *arg)
 * Purpose     : socket read timer:read the msg from hal server
 * Input         : hal-agent session ptr
 * Output       : N/A
 * Return       : void
 ************************************************************************/
int32
_hagt_message_session_read(hagt_message_session_t *hagt_sess)
{
    size_t count;
    size_t left;
    int32 ret;
    uint8 *p;
    asn_dec_rval_t dec_rc;
    int32 i;

    count = GLB_HSRV2AGT_MSG_BUF_LEN - hagt_sess->ibuf_len;
    if (0 == count)
    {
        HAGT_LOG_ERR("hagt_sess(type %d)'s buffer is full, set buffer empty", 
            hagt_sess->session_type);
        hagt_sess->ibuf_len = 0;
        count = GLB_HSRV2AGT_MSG_BUF_LEN;        
    }
    ret = recv(hagt_sess->sock, hagt_sess->ibuf + hagt_sess->ibuf_len, count, 0);
    if (ret < 0) 
    {
        switch (errno)
        {
            case EINTR:
            case EAGAIN:
            case EINPROGRESS:
  #if (EWOULDBLOCK != EAGAIN)        
            case EWOULDBLOCK:
  #endif
            return HAGT_E_NONE;
        }
        
        ret = -1;
        hagt_sess->sock = -1;
        HAGT_LOG_ERR("hal agent phy solt id(%d), read socket type %d error(%d)", 
            hagt_sess->phy_slot_id, hagt_sess->session_type, errno);
        goto EXIT;
    }
    else if (ret == 0) {
        HAGT_LOG_ERR("hal agent phy solt id(%d), session type:%d, read socket 0 byte", 
            hagt_sess->phy_slot_id, hagt_sess->session_type);
        ret = -1;
        goto EXIT;
    }

    hagt_sess->ibuf_len += ret;
    p = hagt_sess->ibuf;
    left = hagt_sess->ibuf_len;

    while (1) {
        dec_rc = ber_decode(0, &asn_DEF_Hsrv2AgtMsg, (void **)&hagt_sess->msg, p, left);
        switch (dec_rc.code) {
        case RC_OK:
#ifdef _HAGT_DEBUG
            xer_fprint(stdout, &asn_DEF_Hsrv2AgtMsg, hagt_sess->msg);
#endif
            ret = _hagt_message_recv_msg_from_hsrv(hagt_sess, hagt_sess->msg);
#if 0 // TODO: add debug info
            if (hagt_sess->msg && HAGT_SESSION_RMT == hagt_sess->session_type)
            {
                hagt_msg_dbg_cnt[hagt_sess->msg->op.present].curr_cnt++;
            }
#endif            
            if (hagt_sess->msg)
            {
                asn_DEF_Hsrv2AgtMsg.free_struct(&asn_DEF_Hsrv2AgtMsg, hagt_sess->msg, 0);
                hagt_sess->msg = NULL;
            }
        
            /*only when socket operation fail, close the socket, and re-connect, 
            in other case, we will not close socket*/
            if(HAGT_E_SOCK_WRITE_ERR == ret)
            {
                HAGT_LOG_ERR("hal agent solt(%d), session type:%d, in msg callback, write socket error", 
                             hagt_sess->phy_slot_id, hagt_sess->session_type);
                ret = -1;
                goto EXIT;
            }
            ret = HAGT_E_NONE;
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left == 0) 
            {
                hagt_sess->ibuf_len = 0;
                return HAGT_E_NONE;
            }
            break;

        case RC_WMORE:
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left > 0)
            {
                sal_memmove(hagt_sess->ibuf, p, left);
            }
            hagt_sess->ibuf_len = left;
            HAGT_MSG_LOG("hal agent read msg more (left: %d bytes)", left);
            return HAGT_E_NONE;

        case RC_FAIL:
            if (hagt_sess->msg) {
                asn_DEF_Hsrv2AgtMsg.free_struct(&asn_DEF_Hsrv2AgtMsg, hagt_sess->msg, 0);
                hagt_sess->msg = NULL;
            }            
            ret = 0;
            hagt_sess->ibuf_len = 0;
            HAGT_LOG_ERR("hal agent msg parser error, session type:%d, left %d bytes", 
                hagt_sess->session_type, left);
            
            for(i = 0; i < left; i++)
                HAGT_LOG_ERR("0x%02X", p[i]);     
            
            goto EXIT;
        }
    }

EXIT:
    if (ret < 0)
    {
        HAGT_LOG_ERR("read process error, slot %d reload", hagt_sess->phy_slot_id);
        /* Modified by kcao 2009-07-08 for bug 8930 : reload linecard if rmt error */
        // TODO: lcm_reload();
        if (hagt_is_enable_stack())
        {
            HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_RELOAD, NULL);
        }
    }
    
    return HAGT_E_MSG_SEND_ERR;
}

/************************************************************************
 * Name        : _hagt_message_read_rmt(ctclib_thread_t *arg)
 * Purpose     : socket read timer:read the msg from hal server
 * Input         : hal-agent session ptr
 * Output       : N/A
 * Return       : void
 ************************************************************************/
int32
_hagt_message_read_rmt(ctclib_thread_t *arg)
{
    hagt_message_session_t *hagt_sess;
    thread_master_t *p_master_thread;

    HAGT_MSG_LOG("hal agent slot %d read rmt msg", hagt_sess->phy_slot_id);
    hagt_sess = CTCLIB_THREAD_ARG (arg);
    p_master_thread = ctclib_thread_get_master();
    hagt_sess->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                   _hagt_message_read_rmt, hagt_sess, hagt_sess->sock);

    return _hagt_message_session_read(hagt_sess);
}

/************************************************************************
 * Name        : _hagt_message_read_ucast(ctclib_thread_t *arg)
 * Purpose     : socket read timer:read the msg from hal server
 * Input         : hal-agent session ptr
 * Output       : N/A
 * Return       : void
 ************************************************************************/
int32
_hagt_message_read_ucast(ctclib_thread_t *arg)
{
    hagt_message_session_t *hagt_sess;
    thread_master_t *p_master_thread;

    HAGT_MSG_LOG("hal agent slot %d read ucast msg", hagt_sess->phy_slot_id);
    hagt_sess = CTCLIB_THREAD_ARG (arg);
    p_master_thread = ctclib_thread_get_master();
    hagt_sess->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                   _hagt_message_read_ucast, hagt_sess, hagt_sess->sock);

    return _hagt_message_session_read(hagt_sess);
}

static int32
_hagt_message_send_db_sync_req()
{
    Hagt2SrvMsg_t hagt_msg;

    sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
    hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgAgentDbSyncReq;
    hagt_msg.opHagt2Srv.hagt2SrvMsgAgentDbSyncReq.physlot = hagt_get_phy_slotid();
    hagt_msg.opHagt2Srv.hagt2SrvMsgAgentDbSyncReq.logicslot = hagt_get_logic_slotid();
#ifdef HAVE_HA_CTC
    hagt_msg.opHagt2Srv.hagt2SrvMsgAgentDbSyncReq.type = GLB_HA_TYPE_SSO;
#else
    hagt_msg.opHagt2Srv.hagt2SrvMsgAgentDbSyncReq.type = GLB_HA_TYPE_RPR;
#endif
    HAGT2SRV_MSG_SEND(&hagt_msg);
    return HAGT_E_NONE;
}

static int32
_hagt_message_ucast_socket_reconnect(ctclib_thread_t *p_thread)
{
    struct sockaddr_in addr;
    hagt_message_session_t *p_ucast_session;
    int32 ret;
    int32 slot;
    ctclib_thread_master_t *p_master_thread;

    HAGT_MSG_LOG("hal agent client reconnect");
    if(NULL == g_pst_hagt_message)
    {
        HAGT_LOG_ERR("hal agent client reconnect fail, hagt msg master is null");
        return HAGT_E_NOT_INIT;
    }
    p_ucast_session = &(g_pst_hagt_message->hagt_msg_ucast);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = sal_htons(GLB_HSRVAGT_MSG_PORT);
    slot = _hagt_message_get_master_svc_phy_slot_id();
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    _hagt_message_get_svc_ip(slot, &addr.sin_addr.s_addr);
#else
    if (hagt_is_enable_stack())
    {
        _hagt_message_get_master_ip(slot, &addr.sin_addr.s_addr);
    }
#endif
    p_master_thread = ctclib_thread_get_master();

    sal_sock_connect(p_ucast_session->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (errno !=  EINPROGRESS)
    {
        /*start timer to continue connect*/
        p_ucast_session->p_thread_connect = ctclib_thread_add_timer(p_master_thread, 
                        _hagt_message_ucast_socket_reconnect, p_master_thread, 1);
        return HAGT_E_NONE;
    }

    if (g_pst_hagt_message->hagt_msg_rmt.has_init != TRUE)   /*Not match when distribute HA system*/
    {
        ret = _hagt_message_send_db_sync_req();
        if(ret)
        {
            HAGT_MSG_LOG("hal agent send db sync msg fail in reconnect");
        }
    }
    
    p_ucast_session->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                   _hagt_message_read_ucast, p_ucast_session, p_ucast_session->sock);

    return HAGT_E_NONE;
}

int32
hagt_message_send_db_sync_timer_cb(ctclib_thread_t *p_thread)
{
    int ret;

    g_stack_send_flag = TRUE;
    ret = _hagt_message_send_db_sync_req();
    if(ret)
    {
        HAGT_MSG_LOG("hal agent send db sync msg fail 1");
    }

    return HAGT_E_NONE;
}

/************************************************************************
 * Name        : hagt_client_init(hagt_session_t *hagt_sess)
 * Purpose     : hal-agent client tcp socket init, which send msg to hal-server, and receive msg when db-syn
 * Input         : hal-agent session ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
static int32
_hagt_message_ucast_socket_init(hagt_message_session_t *hagt_sess)
{
    struct sockaddr_in addr;
    int32 flags = 0;
    int32 ret = 0, cnt = 0;
    int32 slot;
    ctclib_thread_master_t *p_master_thread;
#define HAGT_SOCKET_RECONN_CNT 2000

    HAGT_MSG_LOG("hal agent client init");
    sal_memset(hagt_sess, 0, sizeof(hagt_message_session_t));
    hagt_sess->sock = socket(PF_INET, SOCK_STREAM, 0);
    if (hagt_sess->sock < 0) 
    {
        HAGT_LOG_ERR("Create socket error");
        return HAGT_E_SOCK_CREATE_ERR;
    }

#ifndef _GLB_DISTRIBUTE_SYSTEM_
    int32 vrf = LCM_MSG_VRF;
    if (vrf > 0)
    {
        if (setsockopt(hagt_sess->sock, SOL_SOCKET, SO_VRF, (char *)&vrf, sizeof(vrf)) < 0) 
        {
            return HAGT_E_SOCK_SET_OPT_ERR;
        }
    }
#endif

    flags = 1;
    ret = setsockopt(hagt_sess->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
    if (ret == -1) 
    {
        HAGT_LOG_ERR("Couldn't setsockopt(TCP_NODELAY).");
        close(hagt_sess->sock);
        return HAGT_E_SOCK_SET_OPT_ERR;
    }

    hagt_sess->phy_slot_id = hagt_get_phy_slotid();
    sal_sock_set_nonblocking(hagt_sess->sock, TRUE);
    hagt_sess->ibuf_len = 0;
    hagt_sess->session_type = HAGT_MESSAGE_SESSION_TCP;    
    hagt_sess->has_init = TRUE;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = sal_htons(GLB_HSRVAGT_MSG_PORT);
    slot = _hagt_message_get_master_svc_phy_slot_id();
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    _hagt_message_get_svc_ip(slot, &addr.sin_addr.s_addr);
#else
    if (hagt_is_enable_stack())
    {
        _hagt_message_get_master_ip(slot, &addr.sin_addr.s_addr);
    }
#endif

    p_master_thread = ctclib_thread_get_master();
    cnt = 0;
    while (cnt < HAGT_SOCKET_RECONN_CNT)
    {
        ret = sal_sock_connect(hagt_sess->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        if (ret < 0)
        {                
            cnt++;
            usleep (100*1000); /*100ms*/
            continue;
        }
        else
        {
            HAGT_LOG_ERR("hal agent ucast socket init success");

            hagt_sess->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                           _hagt_message_read_ucast, hagt_sess, hagt_sess->sock);

            if (g_pst_hagt_message->hagt_msg_rmt.has_init != TRUE)   /*Not match when distribute HA system*/
            {
                hagt_sess->p_thread_send_sync = ctclib_thread_add_timer(p_master_thread, 
                    hagt_message_send_db_sync_timer_cb, p_master_thread, 10);
            }
            return HAGT_E_NONE;
        }
    }
    
    HAGT_MSG_LOG("hal aget msg client connect time out");
    /*start timer to continue connect*/
    hagt_sess->p_thread_connect = ctclib_thread_add_timer(p_master_thread, 
                    _hagt_message_ucast_socket_reconnect, p_master_thread, 1);

    return HAGT_E_NONE;
}

static int32
_hagt_message_session_close(hagt_message_session_t *p_session)
{
    if (!p_session)
    {
        return HAGT_E_INVALID_PTR;
    }

    HAGT_MSG_LOG("hal server close session for hal-agent slot %d", p_session->phy_slot_id);

    if (p_session->p_thread_read)
    {
        ctclib_thread_cancel(p_session->p_thread_read);
        p_session->p_thread_read = NULL;
    }

    if (p_session->p_thread_connect)
    {
        ctclib_thread_cancel(p_session->p_thread_connect);
        p_session->p_thread_read = NULL;
    }

    if (p_session->msg)
    {
        asn_DEF_Hsrv2AgtMsg.free_struct(&asn_DEF_Hsrv2AgtMsg, p_session->msg, 0);
        p_session->msg = NULL;
    }
    
    close(p_session->sock);

    /*modified by weij for 15046, 2011-06-01*/
    sal_memset(p_session, 0, sizeof(hagt_message_session_t));
    return HAGT_E_NONE;
}

/* Added by kcao 2010-03-08 for bug 11494; modified by xgu 2011-2-19;
 * This function should only be called by lcm_reload and locked by g_lcm_mutex 
 */
void
_hagt_message_socket_finish()
{
    /* Added by kcao 2009-12-01 for bug 10779 */
    if (g_pst_hagt_message)
    {
        _hagt_message_session_close(&g_pst_hagt_message->hagt_msg_ucast);
        _hagt_message_session_close(&g_pst_hagt_message->hagt_msg_rmt);
    }        
}

static int32
_hagt_message_rmt_socket_reconnect(ctclib_thread_t *p_thread)
{
    struct sockaddr_rmt addr;
    hagt_message_session_t *p_rmt_session;
    ctclib_thread_master_t *p_master_thread;

    HAGT_MSG_LOG("hal agent client reconnect");
    if(NULL == g_pst_hagt_message)
    {
        return HAGT_E_NOT_INIT;
    }
    p_rmt_session = &(g_pst_hagt_message->hagt_msg_rmt);
    sal_memset(&addr, 0, sizeof(addr));
    addr.fam = AF_RMT;
//wangw#define RMT_QLEN_RCV 0x1000 /*Maybe increase by test*/
#define RMT_QLEN_RCV 0x100000 /*Maybe increase by test*/
    addr.group = RMT_GROUP_MIN;
    addr.rxqlen = RMT_QLEN_RCV;
    addr.initseq = g_pst_hagt_message->rmt_initseq;

    p_master_thread = ctclib_thread_get_master();
    if(sal_sock_connect(p_rmt_session->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_rmt)))
    {
        /*start timer to continue connect*/
        p_rmt_session->p_thread_connect = ctclib_thread_add_timer(p_master_thread, 
                        _hagt_message_rmt_socket_reconnect, p_master_thread, 1);
        return HAGT_E_NONE;
    }
    
    HAGT_LOG_ERR("RMT socket connect success.");
    p_rmt_session->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                   _hagt_message_read_rmt, p_rmt_session, p_rmt_session->sock);

    return HAGT_E_NONE;
}

/************************************************************************
 * Name        : hagt_rmt_init(hagt_session_t *hagt_rmt, uint32 initseq)
 * Purpose     : hal agent client init, which receives multicast msg from hal -server
 * Input         : hal-agent rmt ptr and init sequence number
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
static int32
_hagt_message_rmt_init(hagt_message_session_t *hagt_rmt, uint32 initseq)
{
    int32 sock;
    struct sockaddr_rmt addr;
    int32 ret, cnt;
    ctclib_thread_master_t *p_master_thread;

#define HAGT_RMT_SOCKET_RECONN_CNT 20
    
    HAGT_MSG_LOG("hal agent rmt init");

if (0)
{
    int e;
    int s;
    struct sockaddr_rmt a;
    char p[0x100];

    s = socket(PF_RMT, SOCK_RDM, 0);

    memset(&a, 0, sizeof(a));
    a.fam = AF_RMT;
    a.group = RMT_GROUP_MIN;
    a.rxqlen = 0x1000;
    a.initseq = initseq;

    e = connect(s, (struct sockaddr *)&a, sizeof(a));
    if (e < 0) {
        perror("connect()");
        return -1;
    }

    while (1) {
        e = recv(s, p, 0x100, 0);
        if (e < 0)
            perror("recv()");
        else
            printf("e = %d\n", e);
    }


    return 0;
}






    
    if (hagt_rmt->has_init)
    {
        return HAGT_E_HAVE_INIT;
    }
    
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    sock = socket(PF_RMT, SOCK_RDM, 0);
#else
    sock = socket(PF_RMT, SOCK_RAW, 0);
#endif
    if (sock < 0)
    {
        HAGT_LOG_ERR("Create rmt socket fail");
        return HAGT_E_SOCK_CREATE_ERR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.fam = AF_RMT;
//wangw #define RMT_QLEN_RCV 0x1000 /*Maybe increase by test*/
#define RMT_QLEN_RCV 0x100000 /*Maybe increase by test*/
    addr.group = RMT_GROUP_MIN;
    addr.rxqlen = RMT_QLEN_RCV;
    addr.initseq = initseq;
    g_pst_hagt_message->rmt_initseq = initseq;

    sal_sock_set_nonblocking(sock, TRUE);
    hagt_rmt->sock = sock;
    hagt_rmt->phy_slot_id = hagt_get_phy_slotid ();
    hagt_rmt->ibuf_len = 0;
    hagt_rmt->session_type = HAGT_MESSAGE_SESSION_RMT;
    hagt_rmt->has_init = TRUE;

    p_master_thread = ctclib_thread_get_master();
    cnt = 0;
    while (cnt < HAGT_RMT_SOCKET_RECONN_CNT)
    {
        ret = connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_rmt));
        if (ret < 0)
        {                
            cnt++;
            HAGT_MSG_LOG("hal agent rmt msg client reconnect, cnt=%d", cnt);
            usleep (100*1000); /*100ms*/
        }
        else
        {
            hagt_rmt->p_thread_read = ctclib_thread_add_read(p_master_thread, 
                           _hagt_message_read_rmt, hagt_rmt, hagt_rmt->sock);
            return HAGT_E_NONE;
        }
    }
    
    HAGT_MSG_LOG("hal aget msg client connect time out");
    /*start timer to continue connect*/
    hagt_rmt->p_thread_connect = ctclib_thread_add_timer(p_master_thread, 
                    _hagt_message_rmt_socket_reconnect, p_master_thread, 1);


    return HAGT_E_NONE;
}

/************************************************************************
 * Name        : hagt_rcv_rmt_init_seq(hagt_lib_t *hagt_lib, HalMsg_t *msg)
 * Purpose     : op_PR_halMsgInitSeq msg's callback function, 
 *                    which will initialize rmt socket, which will receive multicast msg from hal server
 * Input         : hal agent lib ptr and msg ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
 static int32
_hagt_message_rmt_init_msg_cb(Hsrv2AgtMsg_t *msg)
{
    hagt_message_session_t *p_hagt_sess;
    if (!msg)
    {
        return HAGT_E_INVALID_PARAM;
    }

    HAGT_MSG_LOG("hal agent rcv rmt int msg, seq %d", msg->opHsrv2Agt.hsrv2AgtMsgRmtInitSeqReq.initseq);
    
    if (msg->opHsrv2Agt.present != opHsrv2Agt_PR_hsrv2AgtMsgRmtInitSeqReq)
    {
        return HAGT_E_INVALID_PARAM;
    }
    p_hagt_sess = &g_pst_hagt_message->hagt_msg_rmt;

    if(p_hagt_sess->has_init)
    {
        HAGT_LOG_ERR("Hagt rmt have inited");
        return HAGT_E_HAVE_INIT;
    }

    if(p_hagt_sess->p_thread_read)
    {
        _hagt_message_session_close(p_hagt_sess);
    }
    
    g_pst_hagt_message->db_syning = TRUE;
    _hagt_message_rmt_init(p_hagt_sess, msg->opHsrv2Agt.hsrv2AgtMsgRmtInitSeqReq.initseq);

    // TODO: lcm_rmt_ready_cb( );
    
    return HAGT_E_NONE;    
}

/************************************************************************
 * Name        : hagt_proc_pending_list(void *arg)
 * Purpose     : pending list process timer function. When pending list is empty, stop this timer 
 * Input         : hal agent lib ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
static int32
_hagt_message_proc_pending_list(ctclib_thread_t *arg)
{
    hagt_message_node_t *msg_node_p;
    Hsrv2AgtMsg_t *msg;
    int ret;
// TODO:    card_t *p_card = dev_get_local_card();
    ctclib_thread_master_t *p_master_thread;
    
    if (!g_pst_hagt_message)
    {
        return HAGT_E_NOT_INIT;
    }
    
    /*every time process one msg*/
    if (!ctclib_list_empty(&g_pst_hagt_message->pending_msg_list))
    {
// TODO:                g_pending_msg_cnt++;
        msg_node_p = ctclib_container_of(ctclib_list_head(&(g_pst_hagt_message->pending_msg_list)), 
                     hagt_message_node_t, head);
        if(msg_node_p)
        {
            msg = msg_node_p->msg;
            ret = hagt_message_dispatch_msg_from_hsrv(msg);
            if (ret)
            {
                HAGT_LOG_ERR("hal agent process pending msg(&d) error\n", msg->opHsrv2Agt.present);
            }
            ctclib_list_delete_head(&(g_pst_hagt_message->pending_msg_list));
            asn_DEF_Hsrv2AgtMsg.free_struct(&asn_DEF_Hsrv2AgtMsg, msg, 0);
            HAGT_FREE(CTCLIB_MEM_HAGT_MSG_MODULE, msg_node_p);
        }
    }

    if (!ctclib_list_empty(&g_pst_hagt_message->pending_msg_list))
    {
        p_master_thread = ctclib_thread_get_master();
    
        HAGT_MSG_LOG("hal agent start timer to process pending msg");
        ctclib_thread_add_timer(p_master_thread, _hagt_message_proc_pending_list, NULL, 0);
    }
    else
    {
// TODO:        HAGT_MSG_LOG("card %d hal agent finish process %d pending msgs", p_card->phy_slot_no, g_pending_msg_cnt);
// TODO:        g_pending_msg_cnt = 0;
// TODO:        lcm_msg_tx_lcChsmPendingMsgDone();
    }        
    return HAGT_E_NONE;
}

/************************************************************************
 * Name        : _hagt_message_sync_done_msg_cb(Hsrv2AgtMsg_t *msg)
 * Purpose     : opHsrv2Agt_PR_hsrv2AgtMsgSynDone msg's callback function, 
 *                    which means DB-SYN is done, if pending list is not empty, 
 *                    will start a timer to process pending msg
 * Input         : msg ptr
 * Output       : N/A
 * Return       : HAGT_E_NONE = SUCCESS
 *                    Other   = ErrCode
 * Note          : N/A
 ************************************************************************/
 static int32
 _hagt_message_sync_done_msg_cb(Hsrv2AgtMsg_t *msg)
{
    ctclib_thread_master_t *p_master_thread;

// TODO:    uint32_t sup_slot = 0;
// TODO:    uint32_t sync_delta = 0; 
   
    if (!msg)
    {
        return HAGT_E_INVALID_PARAM;
    }

    if (msg->opHsrv2Agt.present != opHsrv2Agt_PR_hsrv2AgtMsgSynDone)
    {
        return HAGT_E_INVALID_PARAM;
    }
#if 0 // TODO:
    kal_gettimeofday(&g_tv_end_sync);
    sync_delta = (g_tv_end_sync.tv_sec - g_tv_start_sync.tv_sec) * 1000000 
        + (g_tv_end_sync.tv_usec - g_tv_start_sync.tv_usec);
    
    log_sys(M_MOD_HAGT, E_NOTICE, "hal agent rcv db-syn done msg, takes %.3f s",sync_delta/1000000.0);
    peth_start();
    for (sup_slot = MIN_SUP_SLOT; sup_slot <= MAX_SUP_SLOT; sup_slot++)
    {
        if (CHECK_FLAG(g_lcm_sup_ready_flag, (1 << sup_slot)))
        {
            /* Modified by kcao 2009-07-29 for bug 9319 : fabric link rx should be open after db syn is done */
            bay_adapt_sup_card_plugin(sup_slot, dev_get_chassis_type(), 0);
        }
    }
    g_lcm_datapath_ready = TRUE;
#endif
    peth_start();

    /*when receive done msg, syn ends, then process pending msg*/
    if (!ctclib_list_empty(&g_pst_hagt_message->pending_msg_list))
    {
        p_master_thread = ctclib_thread_get_master();
    
        HAGT_MSG_LOG("hal agent start timer to process pending msg");
        ctclib_thread_add_timer(p_master_thread, _hagt_message_proc_pending_list, NULL, 0);
    }
    else
    {
        // TODO: lcm_msg_tx_lcChsmPendingMsgDone();
    }
    g_pst_hagt_message->db_syning = FALSE;
    HAGT_LOG_ERR("hal agent sync message done");

    return HAGT_E_NONE;    
}

int
hagt_message_reinit(void* arg)
{
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    Hagt2SrvMsg_t msg;

    _hagt_message_socket_finish();
    hagt_message_init_module();

    /*send hal message ID*/
    sal_memset(&msg, 0, sizeof(Hagt2SrvMsg_t));
    msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgAuditMsgID;
    msg.opHagt2Srv.hagt2SrvMsgAuditMsgID.physlot = hagt_get_phy_slotid ();
    msg.opHagt2Srv.hagt2SrvMsgAuditMsgID.msgid = g_hagt_hal_msgid;
    HAGT2SRV_MSG_SEND(&msg);
#else
    uint32 master_chip = _hagt_message_get_master_svc_phy_slot_id();
    if (hagt_is_enable_stack())
    {
        _hagt_message_socket_finish();
        hagt_message_init_module();
        hagt_stack_pkt_forward_to_remote(master_chip);
    }
#endif
    return HAGT_E_NONE;
}

int32
hagt_message_ucast_is_ready()
{
    if (g_pst_hagt_message->hagt_msg_ucast.has_init == TRUE)
    {
        return TRUE;
    }
    
    return FALSE;
}
#endif


extern int32 hsrv_message_dispatch_msg_from_hagt(Hagt2SrvMsg_t *pst_msg);
int32
hagt_message_send_msg_to_hsrv(Hagt2SrvMsg_t *p_msg)
{
    //int32 ret;
    
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    return hagt_message_send_msg_to_hsrv_by_socket(p_msg);
#else

#if 0
    if (hagt_is_enable_stack() 
        && g_stack_send_flag == TRUE
        && (_hagt_message_get_master_svc_phy_slot_id() != hagt_get_phy_slotid()))
    {
        /*for bug 25367 modified by liuyang 2013-10-21*/
        sal_mutex_lock(g_pst_hagt_message->pst_send_mutex);
        ret = hagt_message_send_msg_to_hsrv_by_socket(p_msg);
        sal_mutex_unlock(g_pst_hagt_message->pst_send_mutex);
        return ret;
    }
    else
#endif
    {
        p_msg->agentId = 1 << hagt_get_phy_slotid();
        HAGT_IF_ERROR_RETURN(hsrv_message_dispatch_msg_from_hagt(p_msg));
    }
#endif    

    return HAGT_E_NONE;
}
#if 0

int32
hagt_message_init_module()
{
    int32 ret;

    if(NULL == g_pst_hagt_message)
    {
        HAGT_LOG_ERR("hagt_message_init_module fail");
        return HAGT_E_NOT_INIT;
    }

    ret = _hagt_message_ucast_socket_init(&(g_pst_hagt_message->hagt_msg_ucast));
    if(ret)
    {
        HAGT_LOG_ERR("hagt_message_init_module fail, ret(%d)", ret);
        return ret;
    }

    return HAGT_E_NONE;
}

#endif

int32
hagt_message_module_start(void **pp_data)
{
    hagt_message_info_t *pst_message_info;
    
    pst_message_info = HAGT_MALLOC(CTCLIB_MEM_HAGT_MSG_MODULE, sizeof(hagt_message_info_t));
    if (NULL == pst_message_info)
    {
        HAGT_LOG_ERR("hagt_message_module_start fail, no memory!");
        return HAGT_E_NO_MEMORY;
    }

    sal_memset(pst_message_info, 0, sizeof(hagt_message_info_t));
    g_pst_hagt_message = pst_message_info;
    *pp_data = pst_message_info;
    
    ctclib_list_init(&(g_pst_hagt_message->pending_msg_list));
    sal_mutex_create(&g_pst_hagt_message->pst_send_mutex);
    return HAGT_E_NONE;
}



/****************************************************************************
* $Id$
*  Line Card socket client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : kcao
* Date          : 2010-08-11
* Reason        : First Create.
****************************************************************************/
#include "lcm_error.h"
#include "lcm_debug.h"
#include "lcm_client.h"
#include "glb_tempfile_define.h"
#include "lcm_mgt.h"
#include "lcm_chsmtlk.h"
#include "lcm_hagttlk.h"
#ifndef CNOS_OS
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#endif
extern asn_TYPE_descriptor_t asn_DEF_LcmMsg;
static lcm_clnt_t g_lcm_clnt;
#ifdef CNOS_OS
static uint32 g_lcm_clnt_send_count = 0;
#else
static ctclib_thread_t* g_p_clnt_read;
ctclib_thread_t* g_p_timer_chk_rx_keepalive;
thread_t *g_p_timer_con;
#define LCM_SOCKET_SEND_BUFF 32768 /*32k*/
#define LCM_SOCKET_RECONN_CNT 200 /*20s*/
#define LCM_SOCKET_RCV_BUFF 32768 /*32k*/
#endif

int32 ctc_task_null(int32 sock)
{
    return 0;
}

void *
lcm_get_lcm_client()
{
    return &g_lcm_clnt;
}

#ifndef CNOS_OS
void 
lcm_clnt_close(lcm_clnt_t *p_lcm_clnt)
{
    int32 ret = 0;
    
    if (!p_lcm_clnt)
    {
        return;
    }
    
    if (p_lcm_clnt->sock >= 0)  
    {
        ret = sal_close(p_lcm_clnt->sock);
        if (ret<0)
        {
            LCM_LOG_ERR("Close connection failed");
        }
        p_lcm_clnt->sock = -1;
    }
    
    if (p_lcm_clnt->connecting_in_progress || p_lcm_clnt->is_connected)
    {
        p_lcm_clnt->connecting_in_progress = 0;
    }
  
    if (g_p_clnt_read)
    {
        CTC_TASK_DEL_READ(g_p_clnt_read);
    }

    g_p_timer_chk_rx_keepalive = NULL;
    p_lcm_clnt->is_connected = 0;
    p_lcm_clnt->seq = 0;
    p_lcm_clnt->keepalive_cnt = 0;
    p_lcm_clnt->send_keepalive_cnt = 0;
    p_lcm_clnt->recv_keepalive_cnt = 0;
    p_lcm_clnt->logic_slot = 0;
  
    return;
}

void lcm_clnt_fini(lcm_clnt_t *clnt)
{
    lcm_clnt_close(clnt);
}

static inline int32
_lcm_clnt_set_nonblock(int32 fd)
{
    int32 val;

    val = sal_fcntl(fd, F_GETFL);
    sal_fcntl(fd, F_SETFL, val | O_NONBLOCK);

    return LCM_E_SUCCESS;
}

static int 
_lcm_clnt_get_master_ip_by_logic_slot(uint8_t slot, void *dst)
{
    char buf[128];
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    snprintf(buf, sizeof(buf), "172.16.%d.1", slot);
#else
    snprintf(buf, sizeof(buf), "172.16.0.%d", slot);
#endif
    inet_pton(AF_INET, (const char *)buf, dst);
    return 0;
}

static int32 
_lcm_clnt_con_timer_cb(ctclib_thread_t* p_thread)
{
    lcm_clnt_t* clnt = CTCLIB_THREAD_ARG(p_thread);
    int master_sup_logic_slot = lcm_mgt_get_sup_slot();
    ctclib_thread_master_t* p_master = NULL;
    struct stat stat_buf;
    int ret;
    
    p_master = lc_get_thread_master();
    if(!p_master)
    {
        return LCM_E_INVALID_PTR;
    }

    if (stat("/tmp/startup_cfg_done", &stat_buf))
    {
        g_p_timer_con = ctclib_thread_add_timer_msec(p_master, 
                _lcm_clnt_con_timer_cb, (void *)clnt, 5000);  /* 5s */
        return LCM_E_SUCCESS;
    }

    /* The Stacking master itself, or stacking isn't enable*/
    if (!lcm_stack_is_enable() || (lcm_stack_self_slotid() == master_sup_logic_slot))
    {
        return LCM_E_SUCCESS;
    }

    /* The Stacking slave connect to master */
    if (clnt)
    {
        lcm_clnt_close(clnt);
    }

    ret = lcm_clnt_init(clnt, master_sup_logic_slot);
    if (ret < 0)
    {
        g_p_timer_con = ctclib_thread_add_timer_msec(p_master,
                _lcm_clnt_con_timer_cb, (void *)clnt, 5000); /* 5s */
        return LCM_E_SUCCESS;
    }
    LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_MESSAGE_REINIT, NULL);
    return LCM_E_SUCCESS;
}
#endif

static inline int32
_lcm_clnt_rx_msg(int32 sock, LcmMsg_t *p_lcm_msg)
{
    lcm_clnt_t *p_clnt = lcm_get_lcm_client();
    lcm_clnt_cb_t cb = NULL;

    if (p_lcm_msg->op.present >= sizeof(lcm_clnt_cb_tbl)/sizeof(lcm_clnt_cb_tbl[0]))
        return 0;

    cb = lcm_clnt_cb_tbl[p_lcm_msg->op.present];
    if (!cb)
    {
        LCM_LOG_ERR("Can't get message callback function!");
        return LCM_E_NOT_FOUND;
    }
    return cb(p_clnt, p_lcm_msg);
}

int32
#ifdef CNOS_OS    
lcm_clnt_process_lcm_msg(int32 sock, ctc_msg_t *p_msg)
#else
lcm_clnt_process_lcm_msg(ctclib_thread_t* p_thread)
#endif
{
#ifdef CNOS_OS    
    asn_dec_rval_t dec_rc;
    LcmMsg_t *p_lcm_msg = NULL;
    int32 rc = 0;
    
    dec_rc = ber_decode(0, &asn_DEF_LcmMsg, (void **)&p_lcm_msg, p_msg->data, p_msg->data_len);
    if (RC_OK != dec_rc.code) 
    {
        LCM_LOG_ERR("Decode error %d.", dec_rc.code);
        return -1;
    }
    rc = _lcm_clnt_rx_msg(sock, p_lcm_msg);
    
    asn_DEF_LcmMsg.free_struct(&asn_DEF_LcmMsg, p_lcm_msg, 0);
    return rc;
#else
    lcm_clnt_t* clnt = CTCLIB_THREAD_ARG(p_thread);
    ctclib_thread_master_t* p_master = NULL;
    uint32 count;
    uint32 left;
    int32 ret;
    uint8 *p;
    asn_dec_rval_t dec_rc;

    p_master = lc_get_thread_master();
    if(!p_master)
    {
        LCM_LOG_ERR("Get master failed!");
        return LCM_E_INVALID_PTR;
    }
    g_p_clnt_read = ctclib_thread_add_read(p_master, lcm_clnt_process_lcm_msg, clnt, clnt->sock);

    count = LCM_BUF_SZ - clnt->ibuf_len;
    ret = sal_read(clnt->sock, clnt->ibuf + clnt->ibuf_len, count);
    if (ret < 0) 
    {
        LCM_LOG_DEBUG(LCM_CLIENT,"Read socket failed, err %s, ret %d", sal_strerror(errno), ret);
        if (errno == EAGAIN)
            return LCM_E_SOCK_GEN;
        else 
        {
            lcm_clnt_close(clnt);
            return LCM_E_SOCK_GEN;
        }
    }
    else if (ret == 0) 
    {
        lcm_clnt_close(clnt);
        return LCM_E_SOCK_GEN;
    }

    clnt->ibuf_len += ret;
    p = clnt->ibuf;
    left = clnt->ibuf_len;

    while (1) {
        dec_rc = ber_decode(0, &asn_DEF_LcmMsg, (void **)&clnt->msg, p, left);
        LCM_LOG_DEBUG(LCM_CLIENT, "Decode read message done!");
        switch (dec_rc.code) 
        {
            case RC_OK:
                LCM_LOG_DEBUG(LCM_CLIENT, "Decode RC_OK.");
                ret = _lcm_clnt_rx_msg(clnt->sock, clnt->msg);
                asn_DEF_LcmMsg.free_struct(&asn_DEF_LcmMsg, clnt->msg, 0);
                clnt->msg = NULL;

                /*Jqiu modify, bug 13974. 2011-01-24, msg process fail needn't close session.*/
                #if 0
                if (ret < 0) {
                    lcm_clnt_close(clnt);
                    return LCM_E_LCM_MSG_DECODE;
                }
                #endif

                p += dec_rc.consumed;
                left -= dec_rc.consumed;
                if (left == 0) {
                    clnt->ibuf_len = 0;
                    return LCM_E_SUCCESS;
                }
                break;

            case RC_WMORE:
                LCM_LOG_DEBUG(LCM_CLIENT, "Decode RC_WMORE.");
                p += dec_rc.consumed;
                left -= dec_rc.consumed;
                if (left > 0)
                    memmove(clnt->ibuf, p, left);
                clnt->ibuf_len = left;
                return LCM_E_SUCCESS;

            case RC_FAIL:
                LCM_LOG_DEBUG(LCM_CLIENT, "Decode RC_FAIL.");                
                if (clnt->msg) {
                    asn_DEF_LcmMsg.free_struct(&asn_DEF_LcmMsg, clnt->msg, 0);
                    clnt->msg = NULL;
                }
                lcm_clnt_close(clnt);
                return LCM_E_LCM_MSG_DECODE;
        }
    }
    return LCM_E_SUCCESS;
#endif
}

int32 
lcm_clnt_send(lcm_clnt_t *clnt, LcmMsg_t *req)
{
#ifdef CNOS_OS
    g_lcm_clnt_send_count++;
#else
    uint8_t buf[LCM_BUF_SZ];
    size_t len;
    asn_enc_rval_t enc_rc;
    uint8_t *p = buf;
    int32_t n = 0;

    /* Added by kcao 2010-03-03 for bug 11461 */
    if (clnt->sock < 0)
    {
        return -1;
    }
    
    req->ver = 1;
    req->id = clnt->seq++;
#ifdef _LCM_DEBUG
    LCM_LOG_DEBUG(LCM_CLIENT, "+++ Send LCM request +++");
    xer_fprint(stdout, &asn_DEF_LcmMsg, req);
    LCM_LOG_DEBUG(LCM_CLIENT, "--- Send LCM request ---");
#endif

    len = sizeof(buf);
    enc_rc = der_encode_to_buffer(&asn_DEF_LcmMsg, req, buf, &len);
    LCM_LOG_DEBUG(LCM_CLIENT, "Encode message done, encoded %d", enc_rc.encoded);
    if (enc_rc.encoded < 0)
        return -1;

    while (len > 0)
    {
        n = write(clnt->sock, p, len);
        LCM_LOG_DEBUG(LCM_CLIENT,  "Write socket: n %d", n);
        if (n < 0)
        {
            if (errno == EAGAIN) 
            {
                //lcm_clnt_block_cnt++;
                continue;
            }
            LCM_LOG_WARN("Lcm client write error %s", strerror(errno));
            return -1;
        }
        if (n == 0)
        {
            LCM_LOG_WARN("lcm client write error %s", strerror(errno));
            return -1;
        }
        p += n;
        len -= n;
    }    
#endif
    return 0;
}

int32
lcm_clnt_connect_remote(lcm_clnt_t *clnt, uint8 slot)
{
#ifdef CNOS_OS
    int32 sock = INVALID_SOCK_ID;
    const char *path = GLB_CHSM_LCM_SOCK_PATH;
    int32 rc = 0;
    char srv_addr[32];

    sal_memset(srv_addr, 0, sizeof(srv_addr));
    sal_snprintf(srv_addr, sizeof(srv_addr), "172.16.0.%d:6000", slot);
    
    sock = ctc_socket(CTC_SOCK_TCP, 0, "LcmClntTcp");
    ctc_sock_register_connect_fn(sock, ctc_task_null);
    rc = ctc_sock_connect(sock, path, 0);
    if (rc < 0)
    {
        return rc;
    }

    clnt->sock = sock;
#else /* ! CNOS_OS */
    struct sockaddr_in addr;
    int32 flags = 0;
    int32 cnt = 0;
    int32 sleep_long;
    
    clnt->sock = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt->sock < 0)
    {
        return LCM_E_SOCK_GEN;
    }

#ifndef _GLB_DISTRIBUTE_SYSTEM_
#ifdef _NEW_LINUX_VRF_
/*Modified by xgu for new linux vrf implementation, bug 47085, 2018-5-18*/
    struct ifreq ifr;
    strncpy ((char *) & ifr.ifr_name, COMM_VRF_NAME, sizeof (ifr.ifr_name));
    if (setsockopt (clnt->sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) 
    {
        return LCM_E_SOCK_GEN;
    }
#else
    int32 vrf = LCM_MSG_VRF;
    if (vrf > 0)
    {
        if (setsockopt(clnt->sock, SOL_SOCKET, SO_VRF, (char *)&vrf, sizeof(vrf)) < 0) 
        {
            return LCM_E_SOCK_GEN;
        }
    }
#endif    
#endif

    flags = 1;
    if (setsockopt(clnt->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags)) == -1) 
    {
        return LCM_E_SOCK_GEN;
    }

    _lcm_clnt_set_nonblock(clnt->sock);
    
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LCM_MSG_PORT);
    _lcm_clnt_get_master_ip_by_logic_slot(slot, &addr.sin_addr.s_addr);

    LCM_LOG_WARN("Lcm connect remote master(slot %d)", slot);
    if (slot == lcm_stack_master_slotid())
        sleep_long = TRUE;
    else
        sleep_long = FALSE;
    
    while (cnt < LCM_SOCKET_RECONN_CNT)
    {
        LCM_LOG_DEBUG(LCM_CLIENT, "Try to connect, current cnt %d.", cnt);
        if (connect(clnt->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        {                
            cnt++;
            LCM_LOG_DEBUG(LCM_CLIENT, "Lcm client reconnect, cnt=%d", cnt);
            if (sleep_long == TRUE)
                sal_udelay(100000); /*100ms*/
            else
                sal_udelay(1000); /*1ms*/
        }
        else
        {
            LCM_LOG_WARN("Lcm connect remote master success");
            break;
        }
    }
    
    if (cnt == LCM_SOCKET_RECONN_CNT)
    {   
        return LCM_E_SOCK_GEN;
    }
#endif
    return LCM_E_SUCCESS;
}

int32
lcm_clnt_connect_local(lcm_clnt_t *clnt)
{
#ifdef CNOS_OS
    int32 sock = INVALID_SOCK_ID;
    const char *path = GLB_CHSM_LCM_SOCK_PATH;
    int32 rc = 0;
    
    sock = ctc_socket(CTC_SOCK_IPC, 0, "LcmClntIpc");
    ctc_sock_set_mutex(sock, TRUE);
    ctc_sock_register_connect_fn(sock, ctc_task_null);
    rc = ctc_sock_connect(sock, path, 0);
    if (rc < 0)
    {
        return rc;
    }

    clnt->sock = sock;
#else  /* ! CNOS_ARCH */
    struct sockaddr_un addr;
    char *path = GLB_CHSM_LCM_SOCK_PATH;
    int32 cnt = 0;
    int32 size = 0;

    clnt->sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (clnt->sock < 0) 
    {
        LCM_LOG_ERR("Open socket failed!");
        return LCM_E_SOCK_GEN;
    }

    size = LCM_SOCKET_RCV_BUFF;
    if (setsockopt (clnt->sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size)) < 0)
    {
        LCM_LOG_ERR("Change recv buffer failed!");
        lcm_clnt_close(clnt);
        return LCM_E_SOCK_GEN;
    }
    
    size = LCM_SOCKET_SEND_BUFF;
    if (setsockopt (clnt->sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size)) < 0)
    {
        LCM_LOG_ERR("Change send buffer failed!");
        lcm_clnt_close(clnt);
        return LCM_E_SOCK_GEN;
    }
    
    _lcm_clnt_set_nonblock(clnt->sock);
    
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    while (cnt < LCM_SOCKET_RECONN_CNT)
    {
        LCM_LOG_DEBUG(LCM_CLIENT, "Try to connect, current cnt %d.", cnt);
        if (connect(clnt->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
        {                
            cnt++;
            /* modified by huangxt for bug 13785, 2010-12-21 */
            LCM_LOG_DEBUG(LCM_CLIENT, "Lcm client reconnect, cnt=%d", cnt);
            sal_udelay(100000); /*100ms*/
        }
        else
        {
            break;
        }
    }
    if (cnt == LCM_SOCKET_RECONN_CNT)
    {   
        return LCM_E_SOCK_GEN;
    }
#endif
    return LCM_E_SUCCESS;
}

int32 
lcm_clnt_init(lcm_clnt_t *p_clnt, uint8 slot)
{
    int32 ret = 0;
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    return lcm_clnt_connect_remote(p_clnt, slot);
#else
    if (lcm_stack_is_enable())
    {
        ret = lcm_clnt_connect_remote(p_clnt, slot);
        if (ret < 0)
        {
            return ret;
        }
    }
    else
    {
        ret = lcm_clnt_connect_local(p_clnt);
    }
#endif

#ifndef CNOS_OS
    ctclib_thread_master_t* p_master = NULL;
    p_clnt->connecting_in_progress = 0;
    p_clnt->logic_slot = slot;
    p_clnt->is_connected = 1;

    /*for bug 49601*/
    if (lcm_stack_is_enable())
    {
        if (lcm_chsm_msg_tx_lcAttach(p_clnt) < 0)
        {
            LCM_LOG_ERR("Attach master failed!");
            return LCM_E_SOCK_SEND;
        }
    }
    
    p_master = lc_get_thread_master();
    if(!p_master)
    {
        LCM_LOG_ERR("Get lc master failed!");
        return LCM_E_INVALID_PTR;
    }    
    g_p_clnt_read = ctclib_thread_add_read(p_master, lcm_clnt_process_lcm_msg, (void* )p_clnt, p_clnt->sock);
#endif

    return LCM_E_SUCCESS;
}

int32
lcm_clnt_start(void)
{
    int32 slot = lcm_stack_self_slotid();
    lcm_clnt_t *p_clnt = &g_lcm_clnt;
    sal_memset(p_clnt, 0, sizeof(lcm_clnt_t)); 
    
#ifndef CNOS_OS
    ctclib_thread_master_t* p_master = lc_get_thread_master();
    if(!p_master)
    {
        LCM_LOG_ERR("Get lc master failed!");
        return LCM_E_INVALID_PTR;
    }
    g_p_timer_con = ctclib_thread_add_timer_msec(p_master, _lcm_clnt_con_timer_cb, (void* )p_clnt, 20000);
#endif

    return lcm_clnt_init(p_clnt,slot);
}


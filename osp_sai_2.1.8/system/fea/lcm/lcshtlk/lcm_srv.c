/****************************************************************************
 * lcm_svr.c   :      chassis manager socket server (active supervisor)
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       weij
 * Date         :       2010-08-09
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
*
* Header Files 
*
****************************************************************************/
#ifdef CNOS_OS
#include "sal.h"
#else
#include "sal_common.h"
#endif
#include "glb_macro.h"
#include "lcm.h"
#include "lcm_srv.h"
#include "lcm_debug.h"
#include "lcapi.h"
#include "glb_tempfile_define.h"

/****************************************************************************
*  
* Defines and Macros
*
*****************************************************************************/
#define SOCKET_LISTEN_CNT 16


/****************************************************************************
*  
* Global and Declaration
*
*****************************************************************************/
/* lcm server listen socket */
static int32 ssock = -1;
extern lcm_srv_cb_t lcm_lcsh_srv_cb_tbl[LCM_SRV_CB_TBL_SZ];

/****************************************************************************
*  
* Function
*
*****************************************************************************/

/*********************************************************************
 * Name    : _lcm_sess_new
 * Purpose : this function is used to create a new client session for server
 * Input   : none
 * Output  : 
 * Return  : new lcm server session
 * Note    : 
*********************************************************************/
static inline lcm_sess_t*
_lcm_sess_new(void)
{
    return LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(lcm_sess_t));
}

/*********************************************************************
 * Name    : _lcm_sess_free
 * Purpose : this function is used to free client session for server
 * Input   : lcm_sess_t
 * Output  : 
 * Return  : none
 * Note    : 
*********************************************************************/
static inline void 
_lcm_sess_free(lcm_sess_t* pst_sess)
{
    LCM_FREE(CTCLIB_MEM_LCM_MODULE, pst_sess);
}

/*********************************************************************
 * Name    : lcm_sess_close
 * Purpose : this function is used to close sesson while the clish close
 * Input   : lcm_sess_t
 * Output  : 
 * Return  : none
 * Note    : 
*********************************************************************/
static void 
_lcm_sess_close(lcm_sess_t* pst_sess)
{
    if (!pst_sess)
    {
        LCM_LOG_WARN("Lcm server: close null session.");
        return;
    }
    if (NULL != pst_sess->t_read)
    {
        CTC_TASK_DEL_READ(pst_sess->t_read);
    }
    sal_close(pst_sess->sock);
    _lcm_sess_free(pst_sess);
    return;
}

/*********************************************************************
 * Name    : _lcm_sess_rx_msg
 * Purpose : this function is a callback function
 * Input   : lcm_sess_t
 * Output  : 
 * Return  : none
 * Note    : 
*********************************************************************/
static int32 
_lcm_sess_rx_msg(lcm_sess_t* pst_lcm_sess)
{
    lcm_srv_cb_t st_lcm_srv_cb;
    lcapi_lcm_lcsh_msg_t *pst_lcapi_lcm_lcsh_msg = NULL;

    if (!pst_lcm_sess)
    {
        LCM_LOG_WARN("Lcm server: get null session.");
        return LCM_E_INVALID_PTR;
    }
    
    pst_lcapi_lcm_lcsh_msg = (lcapi_lcm_lcsh_msg_t *)pst_lcm_sess->ibuf;
    LCM_LOG_DEBUG(LCM_LCSH, "Lcm server:get message type is %d.", pst_lcapi_lcm_lcsh_msg->msg_type);
        
    if (pst_lcapi_lcm_lcsh_msg->msg_type >= sizeof(lcm_lcsh_srv_cb_tbl)/sizeof(lcm_lcsh_srv_cb_tbl[0]))
    {
        LCM_LOG_ERR("Lcm rcv message error: invlaid message type %d", pst_lcapi_lcm_lcsh_msg->msg_type);
        return LCM_E_INVALID_INDEX;
    }

    st_lcm_srv_cb = lcm_lcsh_srv_cb_tbl[pst_lcapi_lcm_lcsh_msg->msg_type];
    if (!st_lcm_srv_cb)
    {
        LCM_LOG_ERR("Lcm rcv message error: invlaid message type %d", pst_lcapi_lcm_lcsh_msg->msg_type);
        return LCM_E_INVALID_INDEX;
    }
    return st_lcm_srv_cb(pst_lcm_sess, pst_lcapi_lcm_lcsh_msg);
}

/*********************************************************************
 * Name    : _lcm_sess_read
 * Purpose : this function is used to read message from client
 * Input   : 
 * Output  : 
 * Return  : none
 * Note    : 
*********************************************************************/
LCM_DEFINE_TASK_ENCAP(_lcm_sess_read)
LCM_TASK_FUNC(_lcm_sess_read)
{
    int32 ret = 0;
    int32 count = 0;
    int32 msg_rcv_len = 0;
    lcm_sess_t *pst_lcm_sess = (lcm_sess_t*)p_data;
    lcapi_lcm_lcsh_msg_t *pst_lcapi_lcm_lcsh_msg = NULL;

#ifndef CNOS_OS
    ctclib_thread_master_t* p_master = NULL;
    p_master = lc_get_thread_master();
    if (!p_master)
    {
        LCM_LOG_ERR("Lcm server: get thread master error.");
    }
    pst_lcm_sess->t_read = CTC_TASK_ADD_READ(_lcm_sess_read, pst_lcm_sess, pst_lcm_sess->sock);
#endif
    count = LCM_SRV_MSG_BUF_SIZE - pst_lcm_sess->ibuf_len;
    msg_rcv_len = sal_read(pst_lcm_sess->sock, pst_lcm_sess->ibuf + pst_lcm_sess->ibuf_len, count);
    if (msg_rcv_len < 0) 
    {
        LCM_LOG_ERR("Lcm server: read message fail.");
        if (errno == EAGAIN)
        {
            LCM_TASK_RETURN(LCM_E_SUCCESS);
        }
        else 
        {
            _lcm_sess_close(pst_lcm_sess);
            LCM_TASK_RETURN(LCM_E_SOCK_RECV);
        }
    }
    else if (msg_rcv_len == 0) 
    {
        _lcm_sess_close(pst_lcm_sess);
        LCM_TASK_RETURN(LCM_E_SUCCESS);
    }

    pst_lcm_sess->ibuf_len += msg_rcv_len;

    pst_lcapi_lcm_lcsh_msg = (lcapi_lcm_lcsh_msg_t *)pst_lcm_sess->ibuf;
    if (pst_lcapi_lcm_lcsh_msg->msg_len <= pst_lcm_sess->ibuf_len)
    {
        ret = _lcm_sess_rx_msg(pst_lcm_sess);
        pst_lcm_sess->ibuf_len -= pst_lcapi_lcm_lcsh_msg->msg_len;
        sal_memmove(pst_lcm_sess->ibuf, pst_lcm_sess->ibuf + pst_lcapi_lcm_lcsh_msg->msg_len, 
                     pst_lcm_sess->ibuf_len);
    }
    
    (void)ret;
 
    LCM_TASK_RETURN(LCM_E_SUCCESS);
}

/*********************************************************************
 * Name    : _lcm_srv_accept
 * Purpose : this function is used to create client session when accept connect for client
 * Input   : 
 * Output  : none
 * Return  : LCM_E_*
 * Note    : 
*********************************************************************/
LCM_DEFINE_TASK_ENCAP(_lcm_srv_accept)
LCM_TASK_FUNC(_lcm_srv_accept)
{
    int32 csock;
    struct sal_sockaddr_un addr;
    socklen_t addrlen;
    lcm_sess_t* pst_sess = NULL;
#ifndef CNOS_OS
    ctclib_thread_master_t* p_master = NULL;
    p_master = lc_get_thread_master();
    GLB_PTR_VALID_CHECK(p_master,LCM_E_INVALID_PTR);
    CTC_TASK_ADD_READ(_lcm_srv_accept, NULL, ssock);
#endif
    addrlen = sizeof(addr);
    csock = sal_sock_accept(ssock, (struct sal_sockaddr *)&addr, &addrlen);
    if (csock < 0)
    {
        LCM_LOG_ERR("Lcsh: accpet connection fail.");
        LCM_TASK_RETURN(LCM_E_SRV_ACCEPT);
    }

    pst_sess = _lcm_sess_new();
    if (NULL == pst_sess)
    {
        sal_close(csock);
        LCM_TASK_RETURN(LCM_E_INIT_FAILED);
    }

    sal_memset(pst_sess, 0, sizeof(lcm_sess_t));
    pst_sess->sock = csock;

    sal_sock_set_nonblocking(pst_sess->sock, TRUE);
    pst_sess->t_read = CTC_TASK_ADD_READ(_lcm_sess_read, pst_sess, csock);
    if (!pst_sess->t_read)
    {
        LCM_LOG_ERR("Lcsh: add read client thread fail.");
        LCM_TASK_RETURN(LCM_E_THREAD_FAILED);
    }
    LCM_TASK_RETURN(LCM_E_SUCCESS);
}

/*********************************************************************
 * Name    : _lcm_sess_read
 * Purpose : this function is used to create and set property of server socket
 * Input   : none
 * Output  : none
 * Return  : LCM_E_*
 * Note    : 
*********************************************************************/
static int32 
_lcm_srv_init(void)
{
    struct sal_sockaddr_un addr;

    /*create socket*/
    ssock = socket(PF_UNIX, SOCK_STREAM, 0); 
    if (ssock < 0)
    {
        LCM_LOG_CRIT("lcm server create fail.");
        return LCM_E_SOCK_GEN;
    }
    sal_unlink (GLB_LCM_LCSH_SOCK_PATH);
    
    /* prepare accept socket. */
    sal_memset (&addr, 0, sizeof (struct sal_sockaddr_un));
    addr.sun_family = AF_UNIX;
    sal_strncpy (addr.sun_path, GLB_LCM_LCSH_SOCK_PATH, sal_strlen(GLB_LCM_LCSH_SOCK_PATH));
    if (sal_sock_bind(ssock, (struct sal_sockaddr *)&addr, sizeof(struct sal_sockaddr_un)) < 0) 
    {
        sal_close(ssock);
        ssock = -1;
        LCM_LOG_ERR("Chsm server bind error");
        return LCM_E_SOCK_BIND;
    }
    
    sal_sock_set_nonblocking(ssock, TRUE);
    sal_sock_listen(ssock, SOCKET_LISTEN_CNT);

    /*add to thread*/
#ifndef CNOS_OS    
    ctclib_thread_master_t* p_master = NULL;
    p_master = lc_get_thread_master();
    ctclib_thread_add_read(p_master, _lcm_srv_accept, NULL, ssock);
#else    
    CTC_TASK_ADD_READ(_lcm_srv_accept, NULL, ssock);
#endif    
    return LCM_E_SUCCESS;
}

/*********************************************************************
 * Name    : lcm_sess_send
 * Purpose : this function is used to send server info to client
 * Input   : 
 * Output  : none
 * Return  : LCM_E_*
 * Note    : 
*********************************************************************/
int32 
lcm_sess_send(lcm_sess_t* pst_sess, lcapi_lcm_lcsh_msg_t* pst_lcapi_lcm_lcsh_msg)
{
    int32 n = 0;
    uint8 *p_msg = NULL;

    GLB_PTR_VALID_CHECK(pst_sess, LCM_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(pst_lcapi_lcm_lcsh_msg, LCM_E_INVALID_PTR);
    if (pst_sess->sock < 0)
    {
        return LCM_E_SOCK_GEN;
    }
    if (pst_lcapi_lcm_lcsh_msg->msg_len >= LCM_SRV_MSG_BUF_SIZE)
    {
        LCM_LOG_ERR("Lcsh: the message size shouldn't exceed %d.", LCM_SRV_MSG_BUF_SIZE);
        return LCM_E_SRV_MSG_SIZE;
    }
    
    p_msg = (uint8 *)pst_lcapi_lcm_lcsh_msg;
    n = sal_write(pst_sess->sock, p_msg, pst_lcapi_lcm_lcsh_msg->msg_len);
    if (n <= 0)
    {
        LCM_LOG_ERR("Lcsh: send message fail, the error number is %d", errno);
        return LCM_E_SOCK_SEND;
    }
    return LCM_E_SUCCESS;
}

/*********************************************************************
 * Name    : lcm_srv_start
 * Purpose : this function is used to enable lcm server
 * Input   : none
 * Output  : 
 * Return  : LCM_E_*
 * Note    : 
*********************************************************************/
int32
lcm_srv_start(void)
{
    return _lcm_srv_init();
}


/****************************************************************************
* $Id$
*  Line Card socket client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : weij
* Date          : 2010-08-11
* Reason        : First Create.
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
#include "lcsh_client.h"
#include "lcsh_lcmtlk.h"
#include "lcsh_debug.h"
#include "lcsh_error.h"
#include "clish.h"
#include "glb_tempfile_define.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#define SOCKET_RECONN_CNT 16

/****************************************************************************
*
* Global and Declaration
* 
****************************************************************************/
static clish_master_t *g_pst_clish_master = NULL;
static lcsh_clnt_t g_lcsh_clnt;
extern clish_master_t *clish_get_master(void);
/****************************************************************************
*
* Function
* 
****************************************************************************/

/*********************************************************************
 * Name    : _lcsh_clnt_close
 * Purpose : this function is used to close the connect to server
 * Input   : lcsh_clnt_t
 * Output  : 
 * Return  : none
 * Note    : 
*********************************************************************/
static void 
_lcsh_clnt_close(lcsh_clnt_t* pst_lcsh_clnt)
{
    int32 ret = 0;
    
    if (!pst_lcsh_clnt)
    {
        return;
    }
    if (pst_lcsh_clnt->sock >= 0)  
    {
        ret = sal_close(pst_lcsh_clnt->sock);
        if (ret < 0)
        {
            LCSH_LOG_ERR("Lcsh: close session fail.");
            return;
        }
        pst_lcsh_clnt->sock = -1;
    }
    
    pst_lcsh_clnt->is_connected = 0;
    return;
}

/*********************************************************************
 * Name    : _lcsh_clnt_init
 * Purpose : this function is used to init lcsh while create lcsh
 * Input   : lcsh_clnt_t
 * Output  : 
 * Return  : LCSH_E_*
 * Note    : 
*********************************************************************/
int32 
_lcsh_clnt_init(lcsh_clnt_t *clnt)
{
    int32 cnt = 0;
    int32 ret = 0;
    int32 size = 0;
    struct sal_sockaddr_un addr;
    char *path = GLB_LCM_LCSH_SOCK_PATH;
    
    clnt->sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (clnt->sock  < 0) 
    {
        return LCSH_E_SOCK_CREATE_ERR;
    }

    size = LCSH_CLIENT_MSG_BUF_SIZE;
    ret = setsockopt(clnt->sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size));
    if (ret < 0)
    {
        _lcsh_clnt_close(clnt);
        return LCSH_E_SOCK_OPT_ERR;
    }
    
    size = LCSH_CLIENT_MSG_BUF_SIZE;
    ret = setsockopt(clnt->sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size));
    if (ret < 0)
    {
        _lcsh_clnt_close(clnt);
        return LCSH_E_SOCK_OPT_ERR;
    }
    sal_memset(&addr, 0, sizeof(struct sal_sockaddr_un));
    addr.sun_family = AF_UNIX;
    sal_strcpy(addr.sun_path, path);

    while (cnt < SOCKET_RECONN_CNT)
    {
        ret = connect(clnt->sock, (struct sockaddr *)&addr, sizeof(struct sal_sockaddr_un));
        if (ret < 0)
        {
            cnt++;            
            sal_udelay(100000);
        }
        else
        {
            clnt->is_connected = 1;
            return LCSH_E_NONE;
        }
    }
    LCSH_LOG_ERR("Lcsh: socket connect fail");
    return LCSH_E_SOCK_CONNECT_ERR;
}

/*********************************************************************
 * Name    : lcsh_clnt_send_msg
 * Purpose : this function is used to send message to lcm and get response
 * Input   : ctclib_thread_t
 * Output  : 
 * Return  : LCSH_E_*
 * Note    : 
*********************************************************************/
int32 
lcsh_clnt_send_msg(lcapi_lcm_lcsh_msg_t* req, lcapi_lcm_lcsh_msg_t* res)
{
    uint8 *p_msg = NULL;
    int32 n = 0;
    int32 msg_send_len = 0;
    int32 msg_rcv_len = 0;
    lcapi_lcm_lcsh_msg_t *pst_lcapi_lcm_lcsh_msg = NULL;

    if (g_lcsh_clnt.sock < 0)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, invalid socket id %d", g_lcsh_clnt.sock);
        return LCSH_E_SOCK_CREATE_ERR;
    }
    
    msg_send_len = req->msg_len;
    p_msg = (uint8 *)req;

    n = sal_write(g_lcsh_clnt.sock, p_msg, msg_send_len);
    if (n <= 0)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, the error number is %d", errno);
        return LCSH_E_SOCK_SEND_ERR;
    }

    while (1) 
    {
        msg_rcv_len = sal_read(g_lcsh_clnt.sock,
                               g_lcsh_clnt.ibuf + g_lcsh_clnt.ibuf_len,
                               LCSH_CLIENT_MSG_BUF_SIZE - g_lcsh_clnt.ibuf_len);
        if (msg_rcv_len <= 0)
        {
            LCSH_LOG_ERR("Lcsh: receive message fail, the error number is %d", errno);
            return LCSH_E_SOCK_READ_ERR;
        }
        g_lcsh_clnt.ibuf_len += msg_rcv_len;
        pst_lcapi_lcm_lcsh_msg = (lcapi_lcm_lcsh_msg_t *)g_lcsh_clnt.ibuf;
        if (pst_lcapi_lcm_lcsh_msg->msg_len <= g_lcsh_clnt.ibuf_len)
        {
            sal_memcpy(res, g_lcsh_clnt.ibuf, pst_lcapi_lcm_lcsh_msg->msg_len);
            /*remove the getted message*/
            g_lcsh_clnt.ibuf_len -= pst_lcapi_lcm_lcsh_msg->msg_len;
            if (g_lcsh_clnt.ibuf_len)
            {
                sal_memmove(g_lcsh_clnt.ibuf, g_lcsh_clnt.ibuf + pst_lcapi_lcm_lcsh_msg->msg_len, 
                            g_lcsh_clnt.ibuf_len);
            }
            else
            {
                //sal_memset(g_lcsh_clnt.ibuf, 0, sizeof(g_lcsh_clnt.ibuf));
                g_lcsh_clnt.ibuf_len = 0;
            }
            return LCSH_E_NONE;
        }
    }
    return LCSH_E_SOCK_READ_ERR;
}

/*********************************************************************
 * Name    : lcsh_clnt_start
 * Purpose : this function is called when create lcsh
 * Input   : ctclib_thread_t
 * Output  : 
 * Return  : LCSH_E_*
 * Note    : 
*********************************************************************/
int32
lcsh_clnt_start(void)
{
    g_pst_clish_master = clish_get_master();
    sal_memset(&g_lcsh_clnt, 0, sizeof(lcsh_clnt_t));
    return _lcsh_clnt_init(&g_lcsh_clnt);
}


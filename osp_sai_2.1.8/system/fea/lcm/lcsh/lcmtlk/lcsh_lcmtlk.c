/****************************************************************************
 * lcm_chsmtlk.c:   lcm communicate with chassis manager 
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Weij
 * Date         :       2010-08-11
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
#include "glb_hw_define.h"
#include "lcapi.h"
#include "lcapi_lcm_lcsh_msg.h"
#include "lcsh_client.h"
#include "lcsh_debug.h"
#include "lcsh_error.h"
/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/


/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_test1
 * Purpose : this function is used to test message with lcm
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_test1(lcapi_lcm_lcsh_test1_msg_t* pst_test)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_msg;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_test1_msg_t* pst_lcapi_lcm_lcsh_test1_msg = NULL;

    sal_memset(&st_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    st_msg.msg_type = LCAPI_LCM_LCSH_TEST1;
    sal_snprintf(st_msg.msg, 10, "12345");
    st_msg.msg_len = sal_strlen(st_msg.msg) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_msg, &st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message test1 fail, error id is %d", ret);
        return ret;
    }
    pst_lcapi_lcm_lcsh_test1_msg = (lcapi_lcm_lcsh_test1_msg_t*)&st_rcv.msg;
    pst_test->int_value = pst_lcapi_lcm_lcsh_test1_msg->int_value;
    sal_snprintf(pst_test->str_value, 
                 sal_strlen(pst_lcapi_lcm_lcsh_test1_msg->str_value) + 1, "%s", 
                 pst_lcapi_lcm_lcsh_test1_msg->str_value);
    return LCSH_E_NONE;    
}


/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_read_regs
 * Purpose : this function is used to read register
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_read_regs(read_regs_reg_t* read_req, lcapi_lcm_lcsh_msg_t* st_rcv)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_msg;

    if(NULL == read_req)
    {
        LCSH_LOG_ERR("write regs: Invalid args\n");
        return LCSH_E_INVALID_PARAM;
    }
        
    sal_memset(&st_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    st_msg.msg_type = LCAPI_LCM_LCSH_READ_REGS;
    sal_memcpy(st_msg.msg, read_req, sizeof(read_regs_reg_t));
    st_msg.msg_len = sizeof(read_regs_reg_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_msg, st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
        return ret;
    }
    
    return LCSH_E_NONE;    
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_write_regs
 * Purpose : this function is used to write register
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_write_regs(write_regs_reg_t* write_req, lcapi_lcm_lcsh_msg_t* st_rcv)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_msg;

    if(NULL == write_req)
    {
        LCSH_LOG_ERR("write regs: Invalid args\n");
        return LCSH_E_INVALID_PARAM;
    }
    sal_memset(&st_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
 
    st_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
    sal_memcpy(st_msg.msg, write_req, sizeof(write_regs_reg_t));
    st_msg.msg_len = sizeof(write_regs_reg_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_msg, st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
        return ret;
    }
    
    return LCSH_E_NONE;    
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_read_soc_reg
 * Purpose : this function is used to read soc register
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_read_soc_reg(read_soc_reg_t* read_req, lcapi_lcm_lcsh_msg_t* st_rcv)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_msg;

    if(NULL == read_req)
    {
        LCSH_LOG_ERR("write regs: Invalid args\n");
        return LCSH_E_INVALID_PARAM;
    }
        
    sal_memset(&st_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    st_msg.msg_type = LCAPI_LCM_LCSH_READ_SOC_REG;
    sal_memcpy(st_msg.msg, read_req, sizeof(read_soc_reg_t));
    st_msg.msg_len = sizeof(read_soc_reg_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_msg, st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
        return ret;
    }
    
    return LCSH_E_NONE;    
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_write_soc_reg
 * Purpose : this function is used to write soc register
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_write_soc_reg(write_soc_reg_t* write_req, lcapi_lcm_lcsh_msg_t* st_rcv)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_msg;

    if(NULL == write_req)
    {
        LCSH_LOG_ERR("write regs: Invalid args\n");
        return LCSH_E_INVALID_PARAM;
    }
    sal_memset(&st_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
 
    st_msg.msg_type = LCAPI_LCM_LCSH_WRITE_SOC_REG;
    sal_memcpy(st_msg.msg, write_req, sizeof(write_soc_reg_t));
    st_msg.msg_len = sizeof(write_soc_reg_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_msg, st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
        return ret;
    }
    
    return LCSH_E_NONE;    
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_tx_write_regs
 * Purpose : this function is used to write register
 * Input   : 
 * Output  : 
 * Return  : 
 * Note    : 
*********************************************************************/
int32
lcsh_lcm_msg_tx_get_port_info(lcapi_lcm_lcsh_msg_t* st_req, showport_t* buf)
{
    showport_t req_msg;
    showport_t* rcv_msg;
    lcapi_lcm_lcsh_msg_t st_rcv;
    int32 ret = 0;
    int32 port_num;
    int32 ii, len;

    if(NULL == st_req || NULL == buf)
    {
        LCSH_LOG_ERR("lcsh_lcm_msg_tx_get_port_info: Invalid args\n");
        return LCSH_E_INVALID_PARAM;
    }

    sal_memset(&req_msg, 0, sizeof(showport_t));
    req_msg.opcode = GET_PORT_NUM;
    st_req->msg_type = LCAPI_LCM_LCSH_GET_PORT_INFO;
    sal_memcpy(st_req->msg, &req_msg, sizeof(showport_t));
    st_req->msg_len = sizeof(showport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;

    ret = lcsh_clnt_send_msg(st_req, &st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message test1 fail, error id is %d", ret);
        return ret;
    }
    rcv_msg = (showport_t* )(st_rcv.msg);
    if(GET_PORT_NUM == rcv_msg->opcode)
    {
        port_num = rcv_msg->port_num;
        buf->port_num = port_num;
    }
    else
    {
        return LCSH_E_SOCK_READ_ERR;
    }

    len = 0;
    for(ii = 0; ii < port_num; )
    {
        req_msg.opcode = GET_PORT_INFO;
        req_msg.offset = ii;
        if((port_num - ii) >= 8)
        {
            req_msg.port_num = 8;
        }
        else
        {
            req_msg.port_num = port_num - ii;
        }
        sal_memcpy(st_req->msg, &req_msg, sizeof(showport_t));
        st_req->msg_len = sizeof(showport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
        ret = lcsh_clnt_send_msg(st_req, &st_rcv);
        if (ret)
        {
            LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
            return ret;
        }
        sal_memcpy(buf->msg + len, st_rcv.msg, st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE);
        len += st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE;
        ii += 8;
    }
    
    return LCSH_E_NONE;    
}

int32
lcsh_lcm_msg_tx_show_transceiver(uint8 detail)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_req;
    lcapi_lcm_lcsh_msg_t st_rcv;

    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    st_req.msg_type = LCAPI_LCM_LCSH_DUMP_TRANSCEIVER_INFO;
    st_req.msg[0] = detail;
    st_req.msg_len = sal_strlen(st_req.msg)+ LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        LCSH_LOG_ERR("Lcsh: send message fail, error id is %d", ret);
        return ret;
    }

    return LCSH_E_NONE;    
}


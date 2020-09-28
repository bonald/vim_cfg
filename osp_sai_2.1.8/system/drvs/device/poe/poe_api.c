/****************************************************************************
 * poe_api.c    poe(Power of Ethernet)
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       tongzb 
 * Date:         2019-05-27
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "drv_specific.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "poe_api.h"
#include "poe_drv.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
static poe_handle_t* g_poe_hdl = NULL;
static uint32 g_poe_nums = 0; 

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
void poe_task_transmit_receive(void* para);
DRV_DEFINE_TASK_ENCAP(poe_task_transmit_receive);

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
int32
poe_reg_read(uint32 id,   uint8* value)
{
    poe_msg_buff_t msg_buf;
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n", id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].read);
    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    
    if(value == NULL)
    {
        DRV_LOG_ERR("Poe read failed, the return buf is NULL\n",id);
        return RESULT_ERROR;
    }
    if(sizeof(msg_buf.msg.buf) != sizeof(value))
    {
        DRV_LOG_ERR("Poe read failed, the return buf is not enough\n",id);
        return RESULT_ERROR;
    }
    ret = g_poe_hdl[id].read(&g_poe_hdl[id],&msg_buf);
    sal_memcpy(value,msg_buf.msg.buf,sizeof(msg_buf.msg.buf));
    return ret;
}

int32
poe_reg_write(uint32 id,  uint8* value)
{
    poe_msg_buff_t msg_buf;
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n", id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].write);
    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    
    if(value == NULL)
    {
        DRV_LOG_ERR("Poe write failed, the buf is NULL\n",id);
        return RESULT_ERROR;
    }
    if(sizeof(msg_buf.msg.buf) != sizeof(value))
    {
        DRV_LOG_ERR("Poe write failed, the buf is not enough\n",id);
        return RESULT_ERROR;
    }   
    sal_memcpy(&msg_buf.msg.buf,value,sizeof(msg_buf.msg.buf));
    ret = g_poe_hdl[id].write(&g_poe_hdl[id],&msg_buf);
    return ret;;
}

int32
poe_get_status(uint32 id,poe_module_info_t * poe_module)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    sal_memcpy(poe_module,&g_poe_hdl[id].poe_info.info,sizeof(poe_module_info_t));
    return ret;
}

int32
poe_power_status_update(uint32 id)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret += g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_GET_POWER_STAT,0,NULL,-1);

    return ret;
}

int32
poe_glb_status_update(uint32 id)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_GET_GLB_POWER,0,NULL,-1);

    return ret;
}

int32
poe_status_update(uint32 id,uint16 channel,int8 attribute)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_GET_DEV_STAT,channel,NULL,attribute);
    /* calculate class by power */
    ret += g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_GET_CHN_CLASS,channel,NULL,attribute);
    ret += g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_GET_CHN_PARA,channel,NULL,attribute);

    return ret;
}

int32
poe_set_glb_enable(uint32 id,  uint8 enable)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_GLB_EN,0,&enable,-1);

    return ret;
}

int32
poe_set_glb_power(uint32 id,  uint32 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_GLB_POWER,0,&value,-1);

    return ret;
}

int32
poe_set_enable(uint32 id,  uint16 channel,uint8 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_CHN_EN,channel,&value,-1);

    return ret;
}

int32
poe_set_force_power(uint32 id,  uint16 channel,uint8 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_CHN_FORCE,channel,&value,-1);

    return ret;
}

int32
poe_set_mode(uint32 id,  uint16 channel,uint8 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_CHN_MODE,channel,&value,-1);

    return ret;
}

int32
poe_set_priority(uint32 id,  uint16 channel,uint8 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_CHN_PRI,channel,&value,-1);

    return ret;
}

int32
poe_set_power_limit(uint32 id,  uint16 channel,uint32 value)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].operation);
    ret = g_poe_hdl[id].operation(&g_poe_hdl[id],POE_OP_SET_CHN_PPL,channel,&value,-1);

    return ret;
}

void poe_task_transmit_receive(void *para)
{
    uint8 i = 0;
    CTC_TASK_GET_MASTER

    for(i=0;i<g_poe_nums;i++)
    {
        g_poe_hdl[i].task(&g_poe_hdl[i]);
    }
    
    CTC_TASK_ADD_TIME_MSEC(poe_task_transmit_receive, NULL, POE_TASK_INTERVAL);
    return;
}

int32
poe_dev_init(uint32 id)
{
    int32 ret = 0;
    if(id >= g_poe_nums)
    {
        DRV_LOG_ERR("Poe id %d is error!, max is %d\n",id, g_poe_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_poe_hdl[id].init);
    ret = g_poe_hdl[id].init(&g_poe_hdl[id]);
    //CTC_TASK_ADD_TIME_MSEC(poe_task_transmit_receive, NULL, 100);
    return ret;
}

/*********************************************************************
 * Name    : vcm_init
 * Purpose :  init some data structure and register vcm dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of vcm'i2c bus implement
          uint32 num         - the number of vcm
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
poe_init(poe_info_t* poe_info, uint32 num)
{
    int32 i = 0;
    CTC_TASK_GET_MASTER

    g_poe_hdl = (poe_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_POE_INFO, sizeof(poe_handle_t)*num);
    if(NULL == g_poe_hdl)
    {
        DRV_LOG_ERR("poe alloc handler fail!\n");
        return RESULT_ERROR;
    }

    /* register vcm dev */
    for (i = 0; i < num; i++)
    {
        switch(poe_info[i].chip_type)
        {
            case POE_PD69200:
                poe_pd69200_register_driver(&g_poe_hdl[i], &poe_info[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport poe chip type %d!\n", poe_info[i].chip_type);
                return RESULT_ERROR;
        }
    }
    g_poe_nums = num;
    CTC_TASK_ADD_TIME_MSEC(poe_task_transmit_receive, NULL, POE_TASK_INTERVAL);
    
    return RESULT_OK;
}


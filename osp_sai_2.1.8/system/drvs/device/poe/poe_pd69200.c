/****************************************************************************
 * bmr_api.c    pd69200 api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       tongzb
 * Date:         2019-05-27.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "poe_drv.h"
#include "poe_pd69200.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
static poe_msg_list_t g_low_pri_msg_list[MAX_PSE_NUM];
static poe_msg_list_t g_high_pri_msg_list[MAX_PSE_NUM];
static uint8 g_msg_priority[MAX_PSE_NUM];/* 0 means process low priority list, 1 means process high priority list */
static uint8 process_step[MAX_PSE_NUM];/* 0 means host to poe, 1 means poe to host */

static uint8 list_cnt = 0;
/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
#define P_NA 0x4e
poe_msg_t poe_echo_msg[ECHO_POE_MAX] = 
{
    /********************************** system message ************************************************/
    /* poe controller will reset after buffer transmit out, and will back online 300ms later .
    ** poe will reset all configure in non-volatile memory, which means the default configure
    ** when poe reset done, host will get a telemetry message about "Get System Status" .
    */
    {.data = {POE_KEY_COMMAND,ECHO_POE_RESET         ,0x07,0x55,0x00,0x55,0x00,0x55,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* after poe restore, MCU access will fail in 100ms.
    ** after poe restore, all configure will be reset to factory default, which may be different from configure in non-volatile memory
    ** and the response of this program must be read back when MCU can be access
    */
    {.data = {POE_KEY_PROGRAM,ECHO_POE_RESTORE       ,0x2d,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* after poe save, the current configure will be save in non-valatile memory 
    ** this move will cost 50ms at least, and lock the access 
    */
    {.data = {POE_KEY_PROGRAM,ECHO_POE_SAVE          ,0x06,0x0f,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* set user default value for verifying that a restore factory default has occurred 
    ** user default value will be 0xff,  after a restore occurred
    */
    {.data = {POE_KEY_PROGRAM,ECHO_POE_SET_USER_DEF  ,0x41,0x55,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* set system status default value for verifying that a reset has occurred 
    ** system status default value will be 0x00,  after a reset occurred
    */        
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_SYS_STAT   ,0x07,0x3d,0x77,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* get system status
    ** when poe work at boot up error, the telemetry will be send every second and different from normal
    */
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_SYS_STAT   ,0x07,0x3d,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_SYS_STAT2  ,0x07,0x84,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* the interrupt mask affects the interrupt pin output and does not affect the interrupt register itself 
    ** each bit for each interrupt, 0 means mask, 1 means unmask
    */
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_INT_MASK   ,0x07,0x63,0xff,0xff,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_INT_MASK   ,0x07,0x63,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* set individual mask for poe, support different function according to appendix 8.2 
    ** such support legacy detection: 0x07,0x56,0x01,0x01,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA
    */
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_IND_MASK   ,0x07,0x56,0xff,0xff,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_IND_MASK   ,0x07,0x56,0xff,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},

    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_OK_LED     ,0x07,0xa1,0xff,0xff,0xff,0xff,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_OK_LED     ,0x07,0xa1,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
        
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_POWER_OK   ,0x07,0x05,100,80,0xff,0x00,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_POWER_OK   ,0x07,0x05,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 

    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_SYS_MASK   ,0x07,0x2b,0x02,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_SYS_MASK   ,0x07,0x2b,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 

    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_VERSION    ,0x07,0x1e,0x21,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 
    /********************************** device message ************************************************/
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_DEVICE_TSH   ,0x07,0x87,0x00,P_NA,80,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_DEVICE_STAT  ,0x07,0x87,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 
    {.data = {POE_KEY_TEST,ECHO_POE_SET_DEVICE_REG     ,0x04,0x53,0x00,0x00,0x00,0x00,0x00,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_DEVICE_REG  ,0x04,0x52,0x00,0x00,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /********************************** port message ************************************************/
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_TEM_MAT    ,0x05,0x43,0x00,0x00,0xff,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_TEM_MAT    ,0x05,0x43,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_GLB_MAT    ,0x07,0x43,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_GLB_MAT    ,0x05,0x44,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},

    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_GLB_EN     ,0x05,0x0c,0x80,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_EN     ,0x05,0x0c,0x00,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_MODE   ,0x05,0x0c,0x00,P_NA,0x1,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_PD     ,0x05,0x0c,0x00,0x10,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}}, 
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_FORCE  ,0x05,0x51,0x00,0x01,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_PPL    ,0x05,0x0b,0x00,0xff,0xff,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_CHN_PRI    ,0x05,0x0a,0x00,0x3,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},

    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_CHN_POWER_STAT    ,0x07,0xc0,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_CHN_CLASS    ,0x05,0x0e,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_CHN_PARA     ,0x05,0x25,0x00,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /********************************** power management message *************************************/
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_PM_MODE    ,0x07,0x0b,0x5f,0x00,0x00,0x00,P_NA,P_NA,P_NA,P_NA,P_NA}},
    {.data = {POE_KEY_REQUEST,ECHO_POE_GET_GLB_POWER  ,0x07,0x0b,0x60,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA,P_NA}},
    /* default 6000w, 58.5v,40.0v */
    {.data = {POE_KEY_COMMAND,ECHO_POE_SET_GLB_POWER  ,0x07,0x0b,0x57,0x00,0x17,0x70,0x02,0x49,0x01,0x90,0xff}},
};

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
static int32
_poe_checksum_calc(poe_msg_buff_t * msg_buf)
{
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }
    uint8 i;
    uint16 check_sum=0;
    /*cal the CHECKSUM of the msg*/
    for(i=0; i<POE_PD69200_PKG_SIZE-2; i++)
    {
        check_sum += msg_buf->msg.buf[i];
    }
    msg_buf->checksum = check_sum;
    //msg_buf->msg.data.csumh = (check_sum >> 8)&0xff;
    //msg_buf->msg.data.csuml = check_sum &0xff;
    return RESULT_OK;
}


static int32
_poe_msg_init(uint8 echo_type,poe_msg_buff_t * msg_buf)
{
    uint8 i = 0;
    
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }

    for(i =0;i<ECHO_POE_MAX;i++)
    {
        if(poe_echo_msg[i].data.echo == echo_type)
        {
            sal_memcpy(&msg_buf->msg,&poe_echo_msg[i],sizeof(poe_msg_t));
            return RESULT_OK;
        }
    }
    return RESULT_ERROR;
}

static int32
_poe_check_report(poe_msg_buff_t * msg_buf)
{
    uint16 mark = 0;
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }    
    mark = (msg_buf->msg.data.sub<<8)|msg_buf->msg.data.sub1;
    if(msg_buf->msg.data.key == POE_KEY_REPORT)
    {
        if(mark == 0x0000)
        {
            msg_buf->status = POE_REPORT_SUCCESS;
            return RESULT_OK;
        }
        else if((mark >= 0x0001) &&(mark <= 0x7fff))
        {
            msg_buf->status = POE_REPORT_CMD_SUB_INVALID;
            return RESULT_ERROR;
        }
        else if((mark >= 0x8001) &&(mark <= 0x8fff))
        {
            msg_buf->status = POE_REPORT_CMD_DATA_INVALID;
            return RESULT_ERROR;
        }
        else if(mark == 0xffff)
        {
            if((msg_buf->msg.data.sub2 == 0xff)&&(msg_buf->msg.data.data5 == 0xff))
                msg_buf->status = POE_REPORT_CMD_CHK_FAIL;
            else
                msg_buf->status = POE_REPORT_CMD_KEY_UNKNOWN;
            return RESULT_ERROR;
        }       
    }
    return RESULT_OK;
}

static int32
_poe_check_telemetry(poe_msg_buff_t * msg_buf)
{
    uint16 mark = 0;
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }    
    mark = (msg_buf->msg.data.csumh<<8)|msg_buf->msg.data.csuml;
    if(msg_buf->msg.data.key == POE_KEY_TELEMETRY)
    {
        if(mark == msg_buf->checksum)
        {
            msg_buf->status = POE_TELEMETRY_CHK_PASS;
            return RESULT_OK;
        }
        else
        {
            msg_buf->status = POE_TELEMETRY_CHK_FAIL;
            return RESULT_ERROR;
        }   
    }
    return RESULT_ERROR;
}

static int32
_poe_msg_status_update(poe_msg_buff_t * msg_buf)
{
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }

    if(msg_buf->msg.data.key == POE_KEY_REPORT)
    {
        /* for check report msg */
        return _poe_check_report(msg_buf);
    }
    else if(msg_buf->msg.data.key == POE_KEY_TELEMETRY)
    {
        /* for telemetry msg */
        return _poe_check_telemetry(msg_buf);
    }
    else
    {
        msg_buf->status = POE_MAG_UNKNOWN_KEY;
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

static int32
_poe_update_status(poe_handle_t* handle,poe_msg_buff_t * msg_buf)
{
    uint16 channel_id;
    //uint8 op_type;
    int32 value = 0;
    uint8 i = 0;
    poe_info_t* poe_info;
    poe_module_info_t * poe_module=NULL;
    
    if (NULL == handle)
    {
        DRV_LOG_ERR("poe: invalid ptr");
        return RESULT_ERROR;
    }
    poe_info = &handle->poe_info;
    poe_module = &poe_info->info;
    
    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }
    channel_id = msg_buf->channel;
    //op_type = msg_buf->op_type;

    if((msg_buf->status == POE_REPORT_SUCCESS)||(msg_buf->status == POE_TELEMETRY_CHK_PASS))
    {
        DRV_LOG_DEBUG(DRV_POE, "cmd_status %d cmd %s, op_type %d, channel %d, echo %d\n", 
                                msg_buf->status,msg_buf->rdwr?"read":"write",msg_buf->op_type,
                                msg_buf->channel,msg_buf->msg.data.echo);

        if(msg_buf->msg.data.echo == ECHO_POE_GET_GLB_POWER)
        {
            poe_module->power_bank_id = msg_buf->msg.data.data10;
            poe_module->cur_consump = (msg_buf->msg.data.sub << 8)|msg_buf->msg.data.sub1;
        }
        else if(msg_buf->msg.data.echo == ECHO_POE_GET_DEVICE_STAT)
        {
            value = msg_buf->msg.data.data9;
            poe_module->ch_info[channel_id].temper = (value>205)?256-value:value;
        }
        else if(msg_buf->msg.data.echo == ECHO_POE_GET_CHN_CLASS)
        {
            poe_module->ch_info[channel_id].class = msg_buf->msg.data.data6;
        }
        else if(msg_buf->msg.data.echo == ECHO_POE_GET_CHN_POWER_STAT)
        {
            for(i=0;i<MAX_POE_PORT_NUM;i++)
            {
                value = i%8;
                poe_module->ch_info[i].status = (msg_buf->msg.buf[2+i/8]>>value)&0x1;
            }
        }  
        else if(msg_buf->msg.data.echo == ECHO_POE_GET_CHN_PARA)
        {
            poe_module->ch_info[channel_id].cur_consump = (msg_buf->msg.data.data6 << 8)|msg_buf->msg.data.data7;
            poe_module->ch_info[channel_id].current = (msg_buf->msg.data.sub2 << 8)|msg_buf->msg.data.data5;
            poe_module->ch_info[channel_id].voltage = (msg_buf->msg.data.data9 << 8)|msg_buf->msg.data.data10;
            DRV_LOG_DEBUG(DRV_POE, "channel %d para : cur_consump %d current %d voltage %d\n", 
                                    channel_id,poe_module->ch_info[channel_id].cur_consump,
                                    poe_module->ch_info[channel_id].current,
                                    poe_module->ch_info[channel_id].voltage);            
#if 0
            /* calculate class by current consumption */
            if(poe_module->ch_info[channel_id].cur_consump<3840)
                poe_module->ch_info[channel_id].class = 1;
            else if(poe_module->ch_info[channel_id].cur_consump<6490)
                poe_module->ch_info[channel_id].class = 2;
            else if(poe_module->ch_info[channel_id].cur_consump<13000)
                poe_module->ch_info[channel_id].class = 0;/* class 0 same as class 3 */
            else if(poe_module->ch_info[channel_id].cur_consump<25500)
                poe_module->ch_info[channel_id].class = 4;
            else if(poe_module->ch_info[channel_id].cur_consump<40000)
                poe_module->ch_info[channel_id].class = 5;
            else if(poe_module->ch_info[channel_id].cur_consump<51000)
                poe_module->ch_info[channel_id].class = 6;
            else if(poe_module->ch_info[channel_id].cur_consump<62000)
                poe_module->ch_info[channel_id].class = 7;
            else if(poe_module->ch_info[channel_id].cur_consump<71000)
                poe_module->ch_info[channel_id].class = 8;
#endif            
        }
    }
    
    return RESULT_OK;
}


static int32 
_poe_send_msg(poe_handle_t* handle,poe_msg_buff_t * msg_buf,uint8 priority)
{

    if(NULL == msg_buf)
    {
        return RESULT_ERROR;
    }
    poe_msg_list_t *msg_list ;
    if(priority == 0)
        msg_list = &g_low_pri_msg_list[handle->poe_info.index];
    else
        msg_list = &g_high_pri_msg_list[handle->poe_info.index];
    if(msg_list->tx_cnt >= POE_PD69200_MSG_LIST_SIZE)
    {
        return RESULT_ERROR;
    }
    sal_memcpy(&(msg_list->tx_msg_buf[msg_list->tx_push]),msg_buf,sizeof(poe_msg_buff_t));
    msg_list->tx_msg_buf[msg_list->tx_push].msg.data.csumh = (msg_buf->checksum >> 8) & 0xff; 
    msg_list->tx_msg_buf[msg_list->tx_push].msg.data.csuml = msg_buf->checksum & 0xff; 
    
    msg_list->tx_push++;    
    if(msg_list->tx_push == POE_PD69200_MSG_LIST_SIZE)
        msg_list->tx_push = 0;/* roll back to list heard */
    msg_list->tx_cnt++;

    return RESULT_OK;
}

static int32 
_poe_rev_msg(poe_handle_t* handle,uint8 priority)
{
    poe_msg_buff_t * buf;
    poe_msg_list_t *msg_list;
    
    if(priority == 0)
        msg_list = &g_low_pri_msg_list[handle->poe_info.index];
    else
        msg_list = &g_high_pri_msg_list[handle->poe_info.index];

    if(msg_list->rx_cnt)
    {
        buf = &msg_list->rx_msg_buf[0];//only one message buf for read
        if((buf->status == POE_REPORT_SUCCESS)||(buf->status == POE_TELEMETRY_CHK_PASS))
        {
            _poe_update_status(handle,buf);
        }
        sal_memset(buf,0,sizeof(poe_msg_buff_t));
        msg_list->rx_cnt--;
    }
    return RESULT_OK;
}

int32 _poe_msg_process(poe_handle_t* phdl,poe_msg_list_t *msg_list)
{
    poe_msg_buff_t buf;
    poe_msg_buff_t *p_msg = NULL;
    int32 ret = 0;

    if (NULL == phdl)
    {
        DRV_LOG_ERR("poe: invalid ptr");
        return RESULT_ERROR;
    }

    sal_memset(&buf,0,sizeof(poe_msg_buff_t));
    
    if(msg_list->tx_cnt)
    {
        p_msg = &msg_list->tx_msg_buf[msg_list->tx_poll];
        DRV_LOG_DEBUG(DRV_POE, " priority %d cmd %s,op_type %-2d, channel %-2d, echo %-3d,tx_cnt %-3d, tx_poll %d, tx_push %d\n", 
                                g_msg_priority[phdl->poe_info.index],p_msg->rdwr?"read ":"write",p_msg->op_type,
                                p_msg->channel,p_msg->msg.data.echo,msg_list->tx_cnt,msg_list->tx_poll,msg_list->tx_push);
        if(p_msg->rdwr == POE_WRITE)
        {
            ret = phdl->write(phdl,p_msg);
            if(ret == RESULT_OK)
            {
                msg_list->tx_cnt--;
                msg_list->tx_poll++;
                if(msg_list->tx_poll == POE_PD69200_MSG_LIST_SIZE)
                    msg_list->tx_poll = 0;/* roll back to list heard */
            }
            else
            {
                DRV_LOG_ERR("poe: write failed!");
            }
            /* delay */
            //usleep(100000);
            process_step[phdl->poe_info.index] = 0;
        }
        else
        {
            buf.op_type = p_msg->op_type;
            buf.channel = p_msg->channel;
            buf.rdwr = p_msg->rdwr;
            ret = phdl->read(phdl,&buf);
            if(ret == RESULT_OK)
            {
                /* fisrt step when receive valid message */
                sal_memcpy(&msg_list->rx_msg_buf[0],&buf,sizeof(poe_msg_buff_t));
                _poe_checksum_calc(&msg_list->rx_msg_buf[0]);
                ret = _poe_msg_status_update(&msg_list->rx_msg_buf[0]);
                if(ret == RESULT_OK)
                {
                    msg_list->rx_cnt++;
                }
                else
                {
                    DRV_LOG_ERR("poe: read bad data!");
                }
            }
            else
            {
                DRV_LOG_ERR("poe: read failed!");
            }
            /* no matter get ok or bad data, move to the next step */
            msg_list->tx_cnt--;
            msg_list->tx_poll++;
            if(msg_list->tx_poll == POE_PD69200_MSG_LIST_SIZE)
                msg_list->tx_poll = 0;/* roll back to list heard */
            /* delay */
            //usleep(100000);
            process_step[phdl->poe_info.index] = 1;
        }
    }

    return ret;
}

int32 _poe_ps69200_task(poe_handle_t* handle)
{
    poe_msg_list_t *msg_list;
    uint8 msg_priority = 0;/* 0 for low priority, 1 for high priority */
    int32 ret = 0;
    
    if (NULL == handle)
    {
        DRV_LOG_ERR("poe: invalid ptr");
        return RESULT_ERROR;
    }
    msg_priority = g_msg_priority[handle->poe_info.index];
    if(msg_priority == 0)
        msg_list = &g_low_pri_msg_list[handle->poe_info.index];
    else
        msg_list = &g_high_pri_msg_list[handle->poe_info.index];

    /* if one talk between host cpu and poe controller is over need check for high priority msg list */
    if(process_step[handle->poe_info.index])
    {
        /* default process low priority msg */
        msg_list = &g_low_pri_msg_list[handle->poe_info.index];
        msg_priority = 0;
        if(g_high_pri_msg_list ->tx_cnt)
        {
            msg_list = &g_high_pri_msg_list[handle->poe_info.index];
            msg_priority = 1;
        }
        g_msg_priority[handle->poe_info.index] = msg_priority;
    }

    /* transmit data */
    ret = _poe_msg_process(handle,msg_list);    
    /* process one read message if there is one ack msg by poe */
    ret +=_poe_rev_msg(handle,msg_priority);

    return ret;
}

static int32
_poe_ps69200_operation(poe_handle_t* handle,uint8 type,uint16 channel,void *value,int8 attribute)
{
    //int32 ret = 0;
    poe_msg_buff_t msg_buf[2];
    poe_info_t* poe_info;
    poe_module_info_t * poe_module=NULL;
    uint8 msg_priority = attribute>=0?attribute:0;/* 0 for low priority, 1 for high priority */

    if (NULL == handle)
    {
        DRV_LOG_ERR("poe: invalid ptr");
        return RESULT_ERROR;
    }
    poe_info = &handle->poe_info;
    poe_module = &poe_info->info;
    
    if(type >= POE_OP_MAX)
    {
        DRV_LOG_ERR("poe: unsupport property %d!",type);
        return RESULT_ERROR;
    }
    sal_memset(msg_buf,0,sizeof(msg_buf));
    msg_buf[0].op_type = type;
    msg_buf[0].channel = channel;
    msg_buf[0].rdwr = POE_WRITE;
    msg_buf[1].op_type = type;
    msg_buf[1].channel = channel;
    msg_buf[1].rdwr = POE_READ;

    if(POE_OP_SET_GLB_EN == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_GLB_EN,&msg_buf[0]);
        msg_buf[0].msg.data.data5 = *((uint8*)value)?0x01:0x00;
    }
    else if(POE_OP_SET_GLB_POWER == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_GLB_POWER,&msg_buf[0]);
        msg_buf[0].msg.data.data5 = poe_module->power_bank_id;
        msg_buf[0].msg.data.data6 =(*((uint32*)value)>>8) & 0xff;
        msg_buf[0].msg.data.data7 = (*((uint32*)value)) & 0xff;
    }
    else if(POE_OP_SET_CHN_EN == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_CHN_EN,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data5 = *((uint8*)value)?0x01:0x00;
    }
    else if(POE_OP_SET_CHN_FORCE == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_CHN_FORCE,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data5 = *((uint8*)value)?0x01:0x00;
    }
    else if(POE_OP_SET_CHN_MODE == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_CHN_MODE,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data6 =*((uint8*)value);
    }
    else if(POE_OP_SET_CHN_PRI == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_CHN_PRI,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data5 = *((uint8*)value); 
    }
    else if(POE_OP_SET_CHN_PPL == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_CHN_PPL,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data5 =( (*((uint32*)value))>>8) & 0xff;
        msg_buf[0].msg.data.data6 = (*((uint32*)value)) & 0xff;
    }
    else if(POE_OP_SET_CHN_PD == type)
    {
        msg_priority = 1;
        /* this need set individual mask 0x01 = 1 or set system mask to 0x02 at init */
        _poe_msg_init(ECHO_POE_SET_CHN_PD,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
        msg_buf[0].msg.data.data5 = *((uint8*)value)?0x10:0x11;
    }
    else if(POE_OP_SET_SYS_MASK == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_SYS_MASK,&msg_buf[0]);
    } 
    else if(POE_OP_SET_PM_MODE == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_PM_MODE,&msg_buf[0]);
    }  
    else if(POE_OP_SET_SYS_STAT == type)
    {
        msg_priority = 1;
        _poe_msg_init(ECHO_POE_SET_SYS_STAT,&msg_buf[0]);
    }      
    /*********************************************************/
    /******************* Here for read operation ********************/
    else if(POE_OP_GET_GLB_POWER == type)
    {
        _poe_msg_init(ECHO_POE_GET_GLB_POWER,&msg_buf[0]);
    }
    else if(POE_OP_GET_DEV_STAT == type)
    {
        _poe_msg_init(ECHO_POE_GET_DEVICE_STAT,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = poe_module->ch_info[channel].mgmt_id;
    }
    else if(POE_OP_GET_CHN_CLASS == type)
    {
        _poe_msg_init(ECHO_POE_GET_CHN_CLASS,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
    }
    else if(POE_OP_GET_POWER_STAT == type)
    {
        _poe_msg_init(ECHO_POE_GET_CHN_POWER_STAT,&msg_buf[0]);
    }
    else if(POE_OP_GET_CHN_PARA == type)
    {
        _poe_msg_init(ECHO_POE_GET_CHN_PARA,&msg_buf[0]);
        msg_buf[0].msg.data.sub2 = channel;
    }
    else
    {
        DRV_LOG_ERR("poe: unsupport operation %d!",type);
        return RESULT_ERROR;        
    } 
    _poe_checksum_calc(&msg_buf[0]);
    _poe_send_msg(handle,&msg_buf[0],msg_priority);
    
    /* each write need add read operation after */
    msg_buf[1].msg.data.echo = msg_buf[0].msg.data.echo;
    _poe_send_msg(handle,&msg_buf[1],msg_priority);
    
    return RESULT_OK;
}

static int32
_poe_pd69200_read(poe_handle_t* handle,poe_msg_buff_t * msg_buf)
{
    poe_io_access_t p_para;
    
    p_para.len = POE_PD69200_PKG_SIZE;
    p_para.val = msg_buf->msg.buf;

    return handle->io_hdl->read(handle->io_hdl,&p_para);
}

static int32
_poe_pd69200_write(poe_handle_t* handle,poe_msg_buff_t * msg_buf)
{
    poe_io_access_t p_para;
    
    p_para.len = POE_PD69200_PKG_SIZE;
    p_para.val = msg_buf->msg.buf;

    return handle->io_hdl->write(handle->io_hdl,&p_para) ;
}

static int32
_poe_pd69200_init(poe_handle_t* handle)
{
#if 1
    uint32 value = 0;

    _poe_ps69200_operation(handle,POE_OP_GET_GLB_POWER,0,NULL,1);
    _poe_ps69200_operation(handle,POE_OP_SET_SYS_MASK,0,&value,1);
    _poe_ps69200_operation(handle,POE_OP_SET_PM_MODE,0,NULL,1);
    _poe_ps69200_operation(handle,POE_OP_SET_SYS_STAT,0,NULL,1);    
#else
    poe_msg_buff_t msg_buf;

    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    msg_buf.rdwr = POE_WRITE;
    _poe_msg_init(ECHO_POE_GET_GLB_POWER,&msg_buf);
    _poe_checksum_calc(&msg_buf);
    handle->write(handle,&msg_buf);
    
    /* wait between two commands */
    usleep(100000);
    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    msg_buf.rdwr = POE_READ;
    handle->read(handle,&msg_buf);
    _poe_checksum_calc(&msg_buf);
    _poe_msg_status_update(&msg_buf);
    usleep(100000);

    /* update poe info when get ack normal */
    _poe_update_status(handle,&msg_buf);

    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    msg_buf.rdwr = POE_WRITE;
    _poe_msg_init(ECHO_POE_SET_SYS_MASK,&msg_buf);
    /* for enable support Capacitor Detection and enable disconnection method to higher priority*/
    msg_buf.msg.data.sub2 = 0x02;
    _poe_checksum_calc(&msg_buf);
    handle->write(handle,&msg_buf);
    usleep(100000);
    handle->read(handle,&msg_buf);//no use
    usleep(100000);

    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    msg_buf.rdwr = POE_WRITE;
    _poe_msg_init(ECHO_POE_SET_PM_MODE,&msg_buf);
    /* Selects the power limit at the port, Table set by the user (PPL)*/
    _poe_checksum_calc(&msg_buf);
    handle->write(handle,&msg_buf);
    usleep(100000);
    handle->read(handle,&msg_buf);//no use
    usleep(100000);

    sal_memset(&msg_buf,0,sizeof(poe_msg_buff_t));
    msg_buf.rdwr = POE_WRITE;
    _poe_msg_init(ECHO_POE_SET_SYS_STAT,&msg_buf);/* set priviate label for verifying that a reset has occurred */
    _poe_checksum_calc(&msg_buf);
    handle->write(handle,&msg_buf);
    usleep(100000);
    handle->read(handle,&msg_buf);//no use
    usleep(100000);
#endif
    return RESULT_OK;
}

int32
poe_pd69200_register_driver(poe_handle_t* hdl, poe_info_t* info)
{
    poe_handle_t* phdl;
    DRV_CTC_CHK_PTR(info);  
    
    phdl = hdl;
    /* register ltc2991 dev */
    memcpy(&phdl->poe_info, info, sizeof(poe_info_t));
    phdl->poe_info.index = list_cnt++;
    sal_memset(&g_low_pri_msg_list[phdl->poe_info.index],0,sizeof(poe_msg_list_t));
    sal_memset(&g_high_pri_msg_list[phdl->poe_info.index],0,sizeof(poe_msg_list_t));
    if(info->io_type == POE_IO_I2C)
    {
        phdl->io_hdl = poe_io_register(info->io_type, info->p_data_i2c);
    }

    phdl->init = _poe_pd69200_init;
    phdl->read = _poe_pd69200_read;
    phdl->write = _poe_pd69200_write;
    phdl->task = _poe_ps69200_task;
    phdl->operation = _poe_ps69200_operation;

    return RESULT_OK;
}



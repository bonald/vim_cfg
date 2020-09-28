/****************************************************************************
* $Id$
*  I2C driver implemented by I2C controller in CPM
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-07
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "sal_common.h"
#include "genlog.h"
#include "glb_hw_define.h"

#include "ctc_i2c.h"
#include "ctc_pci.h"
#include "i2c_cpm.h"
#include "drv_debug.h"
#include "i2c_err.h"
#include "ctc_gpio.h"
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
/*the max length of the i2c address */
#define I2C_CPM_ADDR_LEN_MAX 4 
struct i2c_cpm_mutex_s{
    uint8 count;
    sal_mutex_t* mutex;
};
typedef struct i2c_cpm_mutex_s i2c_cpm_mutex_t;
/*Fix bug 32429. all cpm i2c use same mutex.*/
i2c_cpm_mutex_t g_i2c_cpm_mutex={0, NULL};

static int32 i2c_cpm_fd0 = -1;
static int32 i2c_cpm_fd1 = -1;


/*********************************************************************
 * Name    : i2c_cpm_close
 * Purpose :  free memory and pointer
 * Input   : i2c_handle_t *phdl       - the handler of the I2C bus
          
 * Output  : N/A
 * Return  : I2C_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
i2c_cpm_close(i2c_handle_t* phdl)
{
    DRV_CTC_CHK_PTR(phdl);

    if(g_i2c_cpm_mutex.count > 0)
    {
        g_i2c_cpm_mutex.count--;
        if(g_i2c_cpm_mutex.count == 0)
        {
            sal_mutex_destroy(g_i2c_cpm_mutex.mutex);
        }
    }
    
    DRV_LOG_DEBUG(DRV_BUS_I2C, "close I2C dev!"); 
    
    if (NULL != phdl)
    {
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO,phdl->data);
            phdl->data = NULL;
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO,phdl);
        phdl = NULL;
    }
        
    return I2C_SUCCESS;
}

/****************************************************************************
 * Name : i2c_cpm_open
 * Purpose: open the i2c dev driver interface
 * Input    :  
 * Output:   
 * Return:  
 * Note :
****************************************************************************/   
int32 
i2c_cpm_open()
{
    if(i2c_cpm_fd0 < 0)
	{
        i2c_cpm_fd0 = open("/dev/i2c-0",  O_RDWR);
		if (i2c_cpm_fd0 < 0)
		{
		    return I2C_E_ERROR;
		}
	}
#if (!defined _CTC_LOONGSON_2F_) && (!defined _CTC_ARM_HI3535_) && (!defined _CTC_X86_64_) && (!defined _CTC_ARM_CTC5236_)
    if(i2c_cpm_fd1 < 0)
	{
        i2c_cpm_fd1 = open("/dev/i2c-1",  O_RDWR);
		if (i2c_cpm_fd1 < 0)
		{
		    return I2C_E_ERROR;
		}
	}
#endif
    return I2C_SUCCESS;
}


/****************************************************************************
 * Name : i2c_transfer
 * Purpose: transfer the i2c message struct  to kernel space i2c driver 
 * Input    :  i2c bus index to distinguish different CPU I2C modules
            :  the message struct need to transfer to kernel
 * Output:   
 * Return:  return 0 if success
 * Note :
****************************************************************************/   
int32 
i2c_transfer(uint8 i2c_bus_idx, struct i2c_rdwr_ioctl_data* msgset)
{
    int32 ret=0;
    int32 i = 0;
    int32 data_len = 0;
    int32 reg_len = 0;
    DRV_CTC_CHK_PTR(msgset);

    if (GLB_I2C_IDX_0 == i2c_bus_idx)
    {
        ret = ioctl(i2c_cpm_fd0, I2C_RDWR, msgset);    
    }
    else if (GLB_I2C_IDX_1 == i2c_bus_idx)
    {
        ret = ioctl(i2c_cpm_fd1, I2C_RDWR, msgset);    
    }

    for (i = 0; i < msgset->nmsgs; i++)
    {
        /*fix bug20012,cr5438, gave more detail debug info, liangf, 2012-07-30*/
        if(msgset->msgs[i].flags == 0x0)//write
        {
            DRV_LOG_DEBUG(DRV_BUS_I2C, "msg[%d] addr 0x%02x write_flag %d  reg_addr_len %d",
                i, msgset->msgs[i].addr, msgset->msgs[i].flags, msgset->msgs[i].len);
            for(reg_len = 0; reg_len < msgset->msgs[i].len; reg_len++) 
            {
                DRV_LOG_DEBUG(DRV_BUS_I2C, "reg_addr[%d] 0x%02x", reg_len, msgset->msgs[i].buf[reg_len]);
            }
        }
        else //read
        {
            DRV_LOG_DEBUG(DRV_BUS_I2C, "msg[%d] addr 0x%02x read_flag %d data_len %d ",
                i, msgset->msgs[i].addr, msgset->msgs[i].flags, msgset->msgs[i].len);
            for(data_len = 0; data_len < msgset->msgs[i].len; data_len++) 
            {
                DRV_LOG_DEBUG(DRV_BUS_I2C, "data[%d] 0x%02x", data_len, msgset->msgs[i].buf[data_len]);
            }
        }
    }    
    return ret;
}


int32 _i2c_offset_rebuild(uint8 *offset_buf,uint32 offset, uint8 alen)
{
    if(offset_buf == NULL)
    {
        DRV_LOG_ERR( "_i2c_offset_rebuild buf is NULL!\n");
        return I2C_E_ERROR;
    }
    if(sizeof(offset_buf)/sizeof(uint8) < alen)
    {
        DRV_LOG_ERR( "_i2c_offset_rebuild buf size %d is not enough for offset %d\n",sizeof(offset_buf)/sizeof(uint8),alen);
        return I2C_E_ERROR;
    }
    /*fix bug20012,cr5438, change high and low address position, liangf, 2012-07-30*/
    if(alen == 1)
    {
        offset_buf[0] = (uint8)(offset&0xff);        
    }
    else if(alen == 2)
    {
        offset_buf[0] = (uint8)((offset>>8)&0xff);
        offset_buf[1] = (uint8)(offset&0xff);  
    }
    else if(alen == 3)
    { 
        offset_buf[0] = (uint8)((offset>>16)&0xff);
        offset_buf[1] = (uint8)((offset>>8)&0xff);
        offset_buf[2] = (uint8)(offset&0xff);  
    }
    else if(alen == 4)
    {
        offset_buf[0] = (uint8)((offset>>24)&0xff);  
        offset_buf[1] = (uint8)((offset>>16)&0xff);
        offset_buf[2] = (uint8)((offset>>8)&0xff);
        offset_buf[3] = (uint8)(offset&0xff);  
    }
    else
    {
        DRV_LOG_ERR( "Unsupport address len %d\n", alen);
        return I2C_E_ERROR;
    }

    return I2C_SUCCESS;
}

/* when channel is 0, means close i2c bridge channel 
   else channel is 0x1 << (i2c_pgen->p_br.channel) when for I2C_BR_CPM
   add channel is 0x1 + (i2c_pgen->p_br.channel) when for I2C_BR_GPIO
   fd is only for I2C_BR_GPIO
   i2c_bus_idx,bridge_addr is only for I2C_BR_CPM
*/
int32 _i2c_bridge_change(uint8 channel,i2c_br_type_e br_type,uint8 i2c_bus_idx,uint8 bridge_addr,int32 fd)
{
    uint8 bridge_channel;
    int32 ret = 0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];    
    if(br_type == I2C_BR_CPM)
    {
        /* set bridge msg */    
        bridge_channel =channel;
        i2c_switch_msg(msgs, bridge_addr, &bridge_channel, I2C_WRITE_OP, 1);
        msgset.msgs = msgs;
        msgset.nmsgs = 1;
        ret = i2c_transfer(i2c_bus_idx, &msgset);
        return ret;
    }
    else if(br_type == I2C_BR_GPIO)
    {
        int32 gpio_fd;
        int value;
    #define CTC_GPIO0 0
    #define CTC_GPIO1 1
    #define CTC_GPIO4 4
    #define CTC_GPIO10 10
    
        /* modified by liuht for bug 40668, 2016-09-09 */
        gpio_fd = fd?fd:get_ctc_gpio_fd();//i2c_pgen->p_br.fd;
        if(gpio_fd<0)
        {
            DRV_LOG_ERR( "ctc_gpio i2c bridge access fail!\n");
            return -1;
        }

        if(channel == 0)/* close i2c bridge */
        {
            /* disable I2C MUX */
            value = (1<<8)|CTC_GPIO10;
            ioctl(gpio_fd, CTC_GPIO_WRITE, (long)&value);
        }
        else
        {
            /* enable I2C MUX */
            value = CTC_GPIO10;
            ioctl(gpio_fd, CTC_GPIO_WRITE, (long)&value);
            /* select I2C channel */
            value = ((((channel-1)>>0) & 1)<<8)|CTC_GPIO0;
            ioctl(gpio_fd, CTC_GPIO_WRITE, (long)&value);
            value = ((((channel-1)>>1) & 1)<<8)|CTC_GPIO1;
            ioctl(gpio_fd, CTC_GPIO_WRITE, (long)&value);
            value = ((((channel-1)>>2) & 1)<<8)|CTC_GPIO4;
            ioctl(gpio_fd, CTC_GPIO_WRITE, (long)&value);
        }
        return 0;
    }
    return 0;
}

/* ret value: < 0 access fail
           >=0 access ok */
int32
i2c_cpm_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 buf[I2C_CPM_ADDR_LEN_MAX];
    int32 ret = 0;
    int32 result = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    _i2c_offset_rebuild(buf,i2c_ppara->offset,i2c_pgen->alen);
    DRV_LOG_DEBUG(DRV_BUS_I2C,"read I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_cpm_mutex.mutex);   
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    { 
        
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0x1<<i2c_pgen->p_br.channel,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                result--;
                goto exit;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            /* for GPIO, the channel need add 1 for distinguish between close i2c bridge */
            _i2c_bridge_change(0x1+i2c_pgen->p_br.channel,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, buf, I2C_WRITE_OP, 
                   i2c_pgen->alen);
    i2c_switch_msg(msgs+1, i2c_pgen->addr, i2c_ppara->p_val, 
                   I2C_READ_OP, i2c_ppara->len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    ret = i2c_transfer(i2c_pgen->i2c_bus_idx, &msgset);              
    if (ret<0)
    {
        result--;
        /*Fix bug 15557, remove error log for continue print but user doesn't care it.*/
        goto exit;
    }    


exit:
    /* close the i2c bridge */    
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                result--;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            _i2c_bridge_change(0,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    sal_mutex_unlock (g_i2c_cpm_mutex.mutex);  
    
    return result;
}

int32 
i2c_cpm_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[1];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;    
    uint8* buf;
    int32 buf_len;
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    buf_len = i2c_ppara->len + i2c_pgen->alen;
    
    buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*buf_len);
    _i2c_offset_rebuild(buf,i2c_ppara->offset,i2c_pgen->alen);
    sal_memcpy(&buf[i2c_pgen->alen], i2c_ppara->p_val, i2c_ppara->len);
    
    DRV_LOG_DEBUG(DRV_BUS_I2C,"write I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d \
             device addr: %x, device internal offset: %x, offset len: %d,val len: %d", 
             i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
             i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_cpm_mutex.mutex);   
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            /* set bridge msg */    
            ret = _i2c_bridge_change(0x1<<i2c_pgen->p_br.channel,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                goto exit;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            /* for GPIO, the channel need add 1 for distinguish between close i2c bridge */
            _i2c_bridge_change(0x1+i2c_pgen->p_br.channel,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, buf, I2C_WRITE_OP, buf_len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_pgen->i2c_bus_idx, &msgset);    
    if (ret<0)
    {
        goto exit;
    }
    
exit:
    /* close the i2c bridge */    
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            _i2c_bridge_change(0,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    sal_mutex_unlock (g_i2c_cpm_mutex.mutex);   
    
    if (buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, buf);
        buf = NULL;
    }
    
    return ret;
}

int32
i2c_cpm_raw_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;

    int32 ret = 0;
    int32 result = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    DRV_LOG_DEBUG(DRV_BUS_I2C,"read I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_cpm_mutex.mutex);   
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    { 
        
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0x1<<i2c_pgen->p_br.channel,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                result--;
                goto exit;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            /* for GPIO, the channel need add 1 for distinguish between close i2c bridge */
            _i2c_bridge_change(0x1+i2c_pgen->p_br.channel,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, i2c_ppara->p_val,I2C_READ_OP, i2c_ppara->len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_pgen->i2c_bus_idx, &msgset);              
    if (ret<0)
    {
        result--;
        /*Fix bug 15557, remove error log for continue print but user doesn't care it.*/
        goto exit;
    }    


exit:
    /* close the i2c bridge */    
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                result--;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            _i2c_bridge_change(0,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    sal_mutex_unlock (g_i2c_cpm_mutex.mutex);  
    
    return result;
}

int32 
i2c_cpm_raw_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[1];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;    
    uint8* buf;
    int32 buf_len;
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    buf_len = i2c_ppara->len;
    
    buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*buf_len);
    sal_memcpy(buf, i2c_ppara->p_val, buf_len);
    
    DRV_LOG_DEBUG(DRV_BUS_I2C,"write I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d \
             device addr: %x, device internal offset: %x, offset len: %d,val len: %d", 
             i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
             i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_cpm_mutex.mutex);   
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            /* set bridge msg */    
            ret = _i2c_bridge_change(0x1<<i2c_pgen->p_br.channel,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
            if(ret < 0)
            {
                goto exit;
            }
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            /* for GPIO, the channel need add 1 for distinguish between close i2c bridge */
            _i2c_bridge_change(0x1+i2c_pgen->p_br.channel,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, buf, I2C_WRITE_OP, buf_len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_pgen->i2c_bus_idx, &msgset);    
    if (ret<0)
    {
        goto exit;
    }
    
exit:
    /* close the i2c bridge */    
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            ret = _i2c_bridge_change(0,I2C_BR_CPM,i2c_pgen->i2c_bus_idx,i2c_pgen->p_br.bridge_addr,0);
        }
        else if(i2c_pgen->p_br.i2c_br_type == I2C_BR_GPIO)
        {
            _i2c_bridge_change(0,I2C_BR_GPIO,0,0,i2c_pgen->p_br.fd);
        }
    }

    sal_mutex_unlock (g_i2c_cpm_mutex.mutex);   
    
    if (buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, buf);
        buf = NULL;
    }
    
    return ret;
}


/*********************************************************************
 * Name    : i2c_cpm_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const i2c_gen_t *openinfo     - some info about the I2C bus implement
 
 * Output  : N/A
 * Return  : I2C bus handle
           
 * Note    : N/A
*********************************************************************/
i2c_handle_t *i2c_cpm_create_handle(  i2c_gen_t* openinfo)
{
    i2c_handle_t* phdl = NULL;    
    
    phdl = (i2c_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(i2c_handle_t));
    if (NULL == phdl)
    {
        goto err_out;
    }

    /* Added by qicx for bug 21474: 2012-11-27 */
    phdl->data = DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(i2c_gen_t));
    if (NULL == phdl->data)
    {
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->data, (int8 *)openinfo, sizeof(i2c_gen_t));

    if(g_i2c_cpm_mutex.count == 0)
    {
        sal_mutex_create(&(g_i2c_cpm_mutex.mutex));
    }

    if(E_I2C_CPM == openinfo->i2c_type)
    {
        phdl->close = i2c_cpm_close;
        phdl->read = i2c_cpm_read;
        phdl->write = i2c_cpm_write;
    }
    else if(E_I2C_CPM_RAW == openinfo->i2c_type)
    {
        phdl->close = i2c_cpm_close;
        phdl->read = i2c_cpm_raw_read;
        phdl->write = i2c_cpm_raw_write;
    }
    
    return phdl;
    
err_out:
    if (NULL != phdl)
    {
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO,phdl->data);
            phdl->data = NULL;
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO,phdl);
        phdl = NULL;
    }

    return NULL;
}

/*Below cmd just used for diag*/
int32 raw_i2c_cpm_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret = 0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8 offset_buf[4];
    
    ret = _i2c_offset_rebuild(offset_buf,offset,alen);
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen);
    i2c_switch_msg(msgs+1, addr, buf, I2C_READ_OP, len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    ret += i2c_transfer(i2c_bus_idx, &msgset);    

    return ret;
}

int32 raw_i2c_cpm_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret = 0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8 *offset_buf;

    offset_buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*(alen+len));
    
    ret = _i2c_offset_rebuild(offset_buf,offset,alen);
    sal_memcpy(&offset_buf[alen], buf, len);
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen+len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_bus_idx, &msgset);
    
    if (offset_buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, offset_buf);
        offset_buf = NULL;
    }

    return ret;
}

int32 raw_i2c_cpm_transfer(uint8 i2c_bus_idx, uint16 addr, uint8 type , uint8* buf, uint8 len)
{
    int32 ret = 0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8* offset_buf = NULL;    

    if(type == 1)/* write */
    {
        offset_buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*len);
        sal_memcpy(offset_buf, buf, sizeof(uint8)*len);
        i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, len);
    }
    else/* read */
    {
        i2c_switch_msg(msgs, addr, buf, I2C_READ_OP, len);
    }
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_bus_idx, &msgset);    

    if (offset_buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, offset_buf);
        offset_buf = NULL;
    }
    
    return ret;
}



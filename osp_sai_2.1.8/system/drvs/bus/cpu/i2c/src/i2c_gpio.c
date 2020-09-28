/****************************************************************************
* $Id$
*  I2C driver implemented by GPIO in cpu
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/statfs.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "sal_common.h"
#include "genlog.h"
#include "glb_hw_define.h"
#include "ctc_i2c.h"
#include "i2c_gpio.h"
#include "drv_debug.h"
#include "i2c_err.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
struct i2c_gpio_mutex_s{
    uint8 count;
    sal_mutex_t* mutex;
};
typedef struct i2c_gpio_mutex_s i2c_gpio_mutex_t;
i2c_gpio_mutex_t g_i2c_gpio_mutex={0, NULL};

static int32 i2c_gpio_fd = -1;

#define I2C_GPIO_RDWR  0x0707
/*********************************************************************
 * Name    : i2c_gpio_close
 * Purpose :  free memory and pointer
 * Input   : i2c_handle_t *phdl       - the handler of the I2C bus
          
 * Output  : N/A
 * Return  : I2C_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
i2c_gpio_close(i2c_handle_t* phdl)
{    
    DRV_CTC_CHK_PTR(phdl);

    if(g_i2c_gpio_mutex.count > 0)
    {
        g_i2c_gpio_mutex.count--;
        if(g_i2c_gpio_mutex.count == 0)
        {
            sal_mutex_destroy(g_i2c_gpio_mutex.mutex);
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

int32 i2c_gpio_open()
{
#ifdef _CTC_LOONGSON_
        if(i2c_gpio_fd < 0)
        {
            i2c_gpio_fd = open("/dev/ctc_gpio_i2c", O_RDWR);
            if (i2c_gpio_fd < 0)
            {
                return I2C_E_ERROR;
            }
        }
#endif    
        return I2C_SUCCESS;
}

int32 i2c_gpio_transfer(struct i2c_rdwr_ioctl_data *msgset)
{
    int32 ret=0;
    int32 i = 0;
    int32 data_len = 0;
    int32 reg_len = 0;
    DRV_CTC_CHK_PTR(msgset);
    
    ret = ioctl(i2c_gpio_fd, I2C_GPIO_RDWR, msgset);    
    
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

/* ret value: < 0 access fail
           >=0 access ok */
int32
i2c_gpio_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 buf[4];
    uint8 bridge_channel;
    int32 ret = 0;
    int32 result = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    if(1 == i2c_pgen->alen)
    {
        buf[0] = (uint8)(i2c_ppara->offset&0xff);
    }
    else if(2 == i2c_pgen->alen)
    {
        buf[0] = (uint8)(i2c_ppara->offset >> 8);
        buf[1] = (uint8)(i2c_ppara->offset&0xff);
    }
    else if(3 == i2c_pgen->alen)
    {
        buf[0] = (uint8)((i2c_ppara->offset>>16)&0xff);
        buf[1] = (uint8)((i2c_ppara->offset>>8)&0xff);
        buf[2] = (uint8)(i2c_ppara->offset&0xff);
    }
    else
    {
        DRV_LOG_ERR( "Unsupport address len %d\n", i2c_pgen->alen);
        buf[0] = (uint8)((i2c_ppara->offset>>24)&0xff);
        buf[1] = (uint8)((i2c_ppara->offset>>16)&0xff);
        buf[2] = (uint8)((i2c_ppara->offset>>8)&0xff);
        buf[3] = (uint8)(i2c_ppara->offset&0xff);
    }   
    DRV_LOG_DEBUG(DRV_BUS_I2C,"read I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_gpio_mutex.mutex);
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    {        
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            /* set bridge msg */    
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            i2c_switch_msg(msgs, i2c_pgen->p_br.bridge_addr, &bridge_channel, I2C_WRITE_OP, 1);
            msgset.msgs = msgs;
            msgset.nmsgs = 1;
            ret = i2c_gpio_transfer(&msgset);
            if(ret < 0)
            {
                result--;
                goto exit;
            }
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, buf, I2C_WRITE_OP, 
                   i2c_pgen->alen);
    i2c_switch_msg(msgs+1, i2c_pgen->addr, i2c_ppara->p_val, 
                   I2C_READ_OP, i2c_ppara->len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    ret = i2c_gpio_transfer(&msgset);              
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
            bridge_channel = 0;
            i2c_switch_msg(msgs, i2c_pgen->p_br.bridge_addr, &bridge_channel, I2C_WRITE_OP, 1);
            msgset.msgs = msgs;
            msgset.nmsgs = 1;
            ret = i2c_gpio_transfer(&msgset); 
            if(ret < 0)
            {
                result--;
            }
        }
    }

    sal_mutex_unlock (g_i2c_gpio_mutex.mutex);       

    return result; 
}

int32 
i2c_gpio_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[1];
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 bridge_channel;
    uint8* buf;
    int32 buf_len;
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;
    buf_len = i2c_ppara->len + i2c_pgen->alen;

    buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*buf_len);
    if(NULL == buf)
    {
        DRV_LOG_ERR( "i2c_cpm_write malloc fail!\n");
        return I2C_E_NO_MEMORY;
    }
    
    if(1 == i2c_pgen->alen)
    {
        *buf = (uint8)(i2c_ppara->offset&0xff);
        sal_memcpy(buf+1, i2c_ppara->p_val, i2c_ppara->len);
    }
    else if(2 == i2c_pgen->alen)
    {
        *buf = (uint8)(i2c_ppara->offset >> 8);
        *(buf+1) = (uint8)(i2c_ppara->offset&0xff);
        sal_memcpy(buf+2, i2c_ppara->p_val, i2c_ppara->len);
    }
    else if(3 == i2c_pgen->alen)
    {
        *buf = (uint8)(i2c_ppara->offset >> 16);        
        *(buf+1) = (uint8)(i2c_ppara->offset>>8);
        *(buf+2) = (uint8)(i2c_ppara->offset&0xff);
        sal_memcpy(buf+3, i2c_ppara->p_val, i2c_ppara->len);
    }
    else
    {
        *buf = (uint8)(i2c_ppara->offset >> 24);
        *(buf+1) = (uint8)(i2c_ppara->offset>>16);
        *(buf+2) = (uint8)(i2c_ppara->offset>>8);
        *(buf+3) = (uint8)(i2c_ppara->offset&0xff);
        sal_memcpy(buf+4, i2c_ppara->p_val, i2c_ppara->len);
    }
  
    DRV_LOG_DEBUG(DRV_BUS_I2C,"write I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d \
             device addr: %x, device internal offset: %x, offset len: %d,val len: %d", 
             i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
             i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_gpio_mutex.mutex);   
    /*fix bug 32429, move i2c bridge config to here.*/
    if(1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_CPM)
        {
            /* set bridge msg */    
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            i2c_switch_msg(msgs, i2c_pgen->p_br.bridge_addr, &bridge_channel, I2C_WRITE_OP, 1);
            msgset.msgs = msgs;
            msgset.nmsgs = 1;
            ret = i2c_gpio_transfer(&msgset);
            if(ret < 0)
            {
                goto exit;
            }
        }
    }

    i2c_switch_msg(msgs, i2c_pgen->addr, buf, I2C_WRITE_OP, buf_len);                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_gpio_transfer(&msgset);    
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
            bridge_channel = 0;
            i2c_switch_msg(msgs, i2c_pgen->p_br.bridge_addr, &bridge_channel, I2C_WRITE_OP, 1);
            msgset.msgs = msgs;
            msgset.nmsgs = 1;
            ret = i2c_gpio_transfer(&msgset); 
        }
    }

    sal_mutex_unlock (g_i2c_gpio_mutex.mutex);   
    
    if (buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, buf);
        buf = NULL;
    }

    return ret;
}

/*********************************************************************
 * Name    : i2c_gpio_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const i2c_gen_t *openinfo     - some info about the I2C bus implement
 
 * Output  : N/A
 * Return  : I2C bus handle
           
 * Note    : N/A
*********************************************************************/
i2c_handle_t *i2c_gpio_create_handle(  i2c_gen_t* openinfo)
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

    if(g_i2c_gpio_mutex.count == 0)
    {
        sal_mutex_create(&(g_i2c_gpio_mutex.mutex));
    }
    
    phdl->close = i2c_gpio_close;
    phdl->read = i2c_gpio_read;
    phdl->write = i2c_gpio_write;

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
int32 raw_i2c_gpio_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret=0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8 offset_buf[4];

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
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen);
    i2c_switch_msg(msgs+1, addr, buf, I2C_READ_OP, len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    
    ret = i2c_gpio_transfer(&msgset);
    
    return ret;
}

int32 raw_i2c_gpio_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret, i=0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8* offset_buf;

    /*fix bug20012,cr5438, change high and low address position, liangf, 2012-07-30*/
    offset_buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*(alen+len));
    if(alen == 1)
    {
        offset_buf[i++] = (uint8)(offset&0xff);        
    }
    else if(alen == 2)
    {
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 3)
    { 
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 4)
    {
        offset_buf[i++] = (uint8)((offset>>24)&0xff);  
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    sal_memcpy(&offset_buf[i], buf, len);
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen+len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_gpio_transfer(&msgset);
    return ret;
}



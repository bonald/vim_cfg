/****************************************************************************
* $Id$
*  I2C driver implemented by CPU SMBus
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-02-06
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
#include "i2c_cpm.h"
#include "i2c_smbus.h"
#include "drv_debug.h"
#include "i2c_err.h"
#include "i2c_dev.h"
#include "ctc_gpio.h"

#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
/*the max length of the i2c address */
#define I2C_SMBUS_ADDR_LEN_MAX 2
struct i2c_smbus_mutex_s{
    uint8 count;
    sal_mutex_t* mutex;
};
typedef struct i2c_smbus_mutex_s i2c_smbus_mutex_t;
i2c_smbus_mutex_t g_i2c_smbus_mutex={0, NULL};

static int32 i2c_smbus_fd0 = -1;

static int32_t i2c_write_1b(int32_t i2c_fd, uint8_t buf)
{
        int32_t ret;
        ret = i2c_smbus_write_byte(i2c_fd, buf);
        return ret;
}

static int32_t i2c_write_2b(int32_t i2c_fd, uint8_t *buf)
{
        int32_t ret;
        ret = i2c_smbus_write_byte_data(i2c_fd, buf[0], buf[1]);
        return ret;
}

static int32_t i2c_write_3b(int32_t i2c_fd, uint8_t *buf)
{
        int32_t ret;
        ret = i2c_smbus_write_word_data(i2c_fd, buf[0], buf[2] << 8 | buf[1]);
        return ret;
}

/*********************************************************************
 * Name    : i2c_smbus_close
 * Purpose :  free memory and pointer
 * Input   : i2c_handle_t *phdl       - the handler of the I2C bus
          
 * Output  : N/A
 * Return  : I2C_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
i2c_smbus_close(i2c_handle_t* phdl)
{
    DRV_CTC_CHK_PTR(phdl);

    if(g_i2c_smbus_mutex.count > 0)
    {
        g_i2c_smbus_mutex.count--;
        if(g_i2c_smbus_mutex.count == 0)
        {
            sal_mutex_destroy(g_i2c_smbus_mutex.mutex);
        }
    }
    
    DRV_LOG_DEBUG(DRV_BUS_I2C, "close I2C dev!"); 
    
    if (NULL != phdl)
    {
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, phdl->data);
            phdl->data = NULL;
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO,phdl);
        phdl = NULL;
    }
        
    return I2C_SUCCESS;
}

/****************************************************************************
 * Name : i2c_smbus_open
 * Purpose: open the i2c dev driver interface
 * Input    :  
 * Output:   
 * Return:  
 * Note :
****************************************************************************/   
int32 
i2c_smbus_open()
{
    if(i2c_smbus_fd0 < 0)
    {
        i2c_smbus_fd0 = open("/dev/i2c-0",  O_RDWR);
        if (i2c_smbus_fd0 < 0)
	 {
            return I2C_E_ERROR;
        }
    }
    return I2C_SUCCESS;
}

int32
i2c_smbus_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    int32 pos;
    uint16 read_word_buf;
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 buf[I2C_SMBUS_ADDR_LEN_MAX];
    uint8 bridge_channel;
    int32 ret = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    DRV_LOG_DEBUG(DRV_BUS_I2C,"read I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_smbus_mutex.mutex);  

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SMBUS)
        {
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->p_br.bridge_addr);
            i2c_write_1b(i2c_smbus_fd0, bridge_channel);
            sal_udelay(3000);
        }
    }

    if (i2c_ppara->width != 0 && i2c_ppara->width == 2
        && i2c_pgen->alen == 1 && i2c_ppara->len == 2) 
    {
        //width set and equal to two, total len equal to 2, alen equal to 1
        //for sensor only
        ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->addr);
        ioctl(i2c_smbus_fd0, BLKFLSBUF);
        read_word_buf = i2c_smbus_read_word_data(i2c_smbus_fd0, i2c_ppara->offset);
        sal_udelay(3000);
        (i2c_ppara->p_val)[1] = (uint8)((read_word_buf >> 8) & 0xff);
        (i2c_ppara->p_val)[0] = (uint8)((read_word_buf) & 0xff);
    }
    else
    {
        ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->addr);
        ioctl(i2c_smbus_fd0, BLKFLSBUF);
        if(1 == i2c_pgen->alen)
        {
            buf[0] = (uint8)(i2c_ppara->offset & 0xff);
            i2c_write_1b(i2c_smbus_fd0, buf[0]);
        }
        else if(2 == i2c_pgen->alen)
        {
            buf[0] = (uint8)((i2c_ppara->offset >> 8) & 0xff);
            buf[1] = (uint8)(i2c_ppara->offset & 0xff);
            i2c_write_2b(i2c_smbus_fd0, buf);
        }
        sal_udelay(3000);
    
        for (pos = 0; pos < i2c_ppara->len; pos ++)
        {
            ioctl(i2c_smbus_fd0, BLKFLSBUF);
            (i2c_ppara->p_val)[pos] = i2c_smbus_read_byte(i2c_smbus_fd0);
            sal_udelay(3000);
        }
    }

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SMBUS)
        {
            bridge_channel = 0;
            ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->p_br.bridge_addr);
            i2c_write_1b(i2c_smbus_fd0, bridge_channel);
            sal_udelay(3000);
        }
    }
    
    sal_mutex_unlock (g_i2c_smbus_mutex.mutex); 
    return ret;
}

int32 
i2c_smbus_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    int32 pos;
    int32 offset;
    uint16 write_word_buf;
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

    buf_len = 1 + i2c_pgen->alen;
    buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(uint8)*buf_len);
    if(NULL == buf)
    {
        DRV_LOG_ERR( "i2c_smbus_write malloc fail!\n");
        return I2C_E_NO_MEMORY;
    }

    DRV_LOG_DEBUG(DRV_BUS_I2C,"write I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d \
             device addr: %x, device internal offset: %x, offset len: %d,val len: %d", 
             i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
             i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_smbus_mutex.mutex);  

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SMBUS)
        {
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->p_br.bridge_addr);
            i2c_write_1b(i2c_smbus_fd0, bridge_channel);
            sal_udelay(3000);
        }
    }
    
    if (i2c_ppara->width != 0 && i2c_ppara->width == 2
        && i2c_pgen->alen == 1 && i2c_ppara->len == 2) 
    {
        //width set and equal to two, total len equal to 2, alen equal to 1
        //for sensor only
        ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->addr);
        write_word_buf = ((i2c_ppara->p_val)[1] << 8) + (i2c_ppara->p_val)[0];
        i2c_smbus_write_word_data(i2c_smbus_fd0, i2c_ppara->offset, write_word_buf);
        sal_udelay(3000);
    }
    else
    {
        ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->addr);

        offset = i2c_ppara->offset;
        for (pos =0; pos < i2c_ppara->len; pos ++)
        {
            if (i2c_pgen->alen == 1)
            {
                    buf[0] = offset & 0x0ff;
                    buf[1] = (i2c_ppara->p_val)[pos];
                    i2c_write_2b(i2c_smbus_fd0, buf);
            }
            else if (i2c_pgen->alen == 2)
            {
                    buf[0] = (offset >> 8) & 0x0ff;
                    buf[1] = offset & 0x0ff;
                    buf[2] = (i2c_ppara->p_val)[pos];
                    i2c_write_3b(i2c_smbus_fd0, buf);
            }
            sal_udelay(3000);
            offset++;
        }
    }

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SMBUS)
        {
            bridge_channel = 0;
            ioctl(i2c_smbus_fd0, I2C_SLAVE, i2c_pgen->p_br.bridge_addr);
            i2c_write_1b(i2c_smbus_fd0, bridge_channel);
            sal_udelay(3000);
        }
    }
    
    sal_mutex_unlock (g_i2c_smbus_mutex.mutex); 
    if (buf)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_I2C_INFO, buf);
        buf = NULL;
    }
    
    return ret;
}


/*********************************************************************
 * Name    : i2c_smbus_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const i2c_gen_t *openinfo     - some info about the I2C bus implement
 
 * Output  : N/A
 * Return  : I2C bus handle
           
 * Note    : N/A
*********************************************************************/
i2c_handle_t *i2c_smbus_create_handle(  i2c_gen_t* openinfo)
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

    if(g_i2c_smbus_mutex.count == 0)
    {
        sal_mutex_create(&(g_i2c_smbus_mutex.mutex));
    }
    
    phdl->close = i2c_smbus_close;
    phdl->read = i2c_smbus_read;
    phdl->write = i2c_smbus_write;

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
int32 raw_i2c_smbus_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret = 0;
    int32 pos;
    uint8 offset_buf[4];
    ioctl(i2c_smbus_fd0, I2C_SLAVE, addr);
    ioctl(i2c_smbus_fd0, BLKFLSBUF);
    if(1 == alen)
    {
        offset_buf[0] = (uint8)(offset & 0xff);
        i2c_write_1b(i2c_smbus_fd0, offset_buf[0]);
    }
    else if(2 == alen)
    {
        offset_buf[0] = (uint8)((offset >> 8) & 0xff);
        offset_buf[1] = (uint8)(offset & 0xff);
        i2c_write_2b(i2c_smbus_fd0, offset_buf);
    }
    sal_udelay(3000);
    
    for (pos = 0; pos < len; pos ++)
    {
        ioctl(i2c_smbus_fd0, BLKFLSBUF);
        buf[pos] = i2c_smbus_read_byte(i2c_smbus_fd0);
        sal_udelay(3000);
    }
    return ret;
}

int32 raw_i2c_smbus_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret = 0;
    uint8 data_buf[3];
    int32 pos;

    ioctl(i2c_smbus_fd0, I2C_SLAVE, addr);

    for (pos =0; pos < len; pos ++)
    {
        if (alen == 1)
        {
                data_buf[0] = offset & 0x0ff;
                data_buf[1] = buf[pos];
                i2c_write_2b(i2c_smbus_fd0, buf);
        }
        else if (alen == 2)
        {
                data_buf[0] = (offset >> 8) & 0x0ff;
                data_buf[1] = offset & 0x0ff;
                data_buf[2] = buf[pos];
                i2c_write_3b(i2c_smbus_fd0, data_buf);
        }
        sal_udelay(3000);
        offset++;
    }
    return ret;
}

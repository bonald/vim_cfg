/****************************************************************************
* $Id$
*  I2C driver implemented by Advantech SUSI
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-03-05
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
#include "drv_debug.h"
#include "i2c_err.h"
#include "i2c_dev.h"
#include "common.h"
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
#define I2C_SUSI_ADDR_LEN_MAX 2

struct i2c_susi_mutex_s{
    uint8 count;
    sal_mutex_t* mutex;
};
typedef struct i2c_susi_mutex_s i2c_susi_mutex_t;
i2c_susi_mutex_t g_i2c_susi_mutex={0, NULL};

struct I2CInfo
{
	uint32_t FreqFlag;
};

static struct I2CInfo info[SUSI_I2C_MAX_DEVICE];
static SusiId_t devids[SUSI_I2C_MAX_DEVICE];
static const uint8_t maxdevice = NELEMS(devids);

/*********************************************************************
 * Name    : i2c_susi_close
 * Purpose :  free memory and pointer
 * Input   : i2c_handle_t *phdl       - the handler of the I2C bus
          
 * Output  : N/A
 * Return  : I2C_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
i2c_susi_close(i2c_handle_t* phdl)
{
    DRV_CTC_CHK_PTR(phdl);

    if(g_i2c_susi_mutex.count > 0)
    {
        g_i2c_susi_mutex.count--;
        if(g_i2c_susi_mutex.count == 0)
        {
            sal_mutex_destroy(g_i2c_susi_mutex.mutex);
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

    SusiLibUninitialize();
    
    return I2C_SUCCESS;
}

/****************************************************************************
 * Name : i2c_susi_open
 * Purpose: open the i2c dev driver interface
 * Input    :  
 * Output:   
 * Return:  
 * Note :
****************************************************************************/   
int32 
i2c_susi_open()
{
    SusiStatus_t status;
    uint32_t tmp;
    uint8_t index, i;

    status = SusiLibInitialize();

    if (status == SUSI_STATUS_ERROR)
    {
        // This error only occurs on Linux distributions
        DRV_LOG_DEBUG(DRV_BUS_I2C, "Your Linux capabilities is not enough, recommond to become ROOT!\n");
        DRV_LOG_DEBUG(DRV_BUS_I2C, "Aborting demo now.\n");
        return -1;
    }

    if (status != SUSI_STATUS_SUCCESS && status != SUSI_STATUS_INITIALIZED)
    {
        DRV_LOG_DEBUG(DRV_BUS_I2C, "SusiLibInitialize() failed. (0x%08X)\n", status);
        DRV_LOG_DEBUG(DRV_BUS_I2C, "Exit the program...\n");
        return -1;
    }

    status = SusiBoardGetValue(SUSI_ID_I2C_SUPPORTED, &tmp);
    if (status != SUSI_STATUS_SUCCESS)
        return SUSIDEMO_DEVICE_UNAVAILALBE;
    if (!tmp)
        return SUSIDEMO_DEVICE_UNAVAILALBE;

    index = 0;	/* index for devices */
    for (i = 0; i < maxdevice; i++)
    {
        if (tmp & (1 << i))
        {
            uint32_t tmp2;
            if (SusiI2CGetFrequency(i, &tmp2) == SUSI_STATUS_SUCCESS)
                info[index].FreqFlag = 1;
            else
                info[index].FreqFlag = 0;

            devids[index++] = i;
        }
    }
	
    if (!index)
        return SUSIDEMO_DEVICE_UNAVAILALBE;

    while (index < maxdevice)
        devids[index++] = SUSIDEMO_DEVICEID_UNDEFINED;

    return SUSIDEMO_DEVICE_AVAILALBE;
}

int32
i2c_susi_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 bridge_channel;
    SusiId_t id = devids[0];
    int32 ret = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    DRV_LOG_DEBUG(DRV_BUS_I2C,"read I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_susi_mutex.mutex); 

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SUSI)
        {
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            SusiI2CWrite(id, i2c_pgen->p_br.bridge_addr, &bridge_channel, 1);
        }
    }

    if (i2c_pgen->alen == 1)
        i2c_ppara->offset = SUSI_I2C_ENC_STD_CMD(i2c_ppara->offset);
    else if (i2c_pgen->alen == 2)
        i2c_ppara->offset = SUSI_I2C_ENC_EXT_CMD(i2c_ppara->offset);
    SusiI2CReadTransfer(id, i2c_pgen->addr, i2c_ppara->offset, i2c_ppara->p_val, i2c_ppara->len);
    
    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SUSI)
        {
            bridge_channel = 0;
            SusiI2CWrite(id, i2c_pgen->p_br.bridge_addr, &bridge_channel, 1);
        }
    }
    
    sal_mutex_unlock (g_i2c_susi_mutex.mutex); 
    return ret;
}

int32 
i2c_susi_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    i2c_gen_t* i2c_pgen = NULL;
    i2c_op_para_t* i2c_ppara = NULL;
    uint8 bridge_channel;
    SusiId_t id = devids[0];
    int32 ret = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    i2c_pgen = (i2c_gen_t *)phdl->data;
    i2c_ppara = ppara;

    DRV_LOG_DEBUG(DRV_BUS_I2C,"write I2C dev, bridge_flag: %d, bridge_addr: %d, channel: %d, i2c bus index: %d\
               device addr: %x, device internal offset: %x, offset len: %d, val len: %d", 
               i2c_pgen->bridge_flag, i2c_pgen->p_br.bridge_addr, i2c_pgen->p_br.channel, i2c_pgen->i2c_bus_idx,
               i2c_pgen->addr, i2c_pgen->alen, i2c_ppara->offset, i2c_ppara->len);

    sal_mutex_lock (g_i2c_susi_mutex.mutex); 

    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SUSI)
        {
            bridge_channel = (0x1 << (i2c_pgen->p_br.channel));
            SusiI2CWrite(id, i2c_pgen->p_br.bridge_addr, &bridge_channel, 1);
        }
    }

    if (i2c_pgen->alen == 1)
        i2c_ppara->offset = SUSI_I2C_ENC_STD_CMD(i2c_ppara->offset);
    else if (i2c_pgen->alen == 2)
        i2c_ppara->offset = SUSI_I2C_ENC_EXT_CMD(i2c_ppara->offset);
    SusiI2CWriteTransfer(id, i2c_pgen->addr, i2c_ppara->offset, i2c_ppara->p_val, i2c_ppara->len);
    
    if (1 == i2c_pgen->bridge_flag)
    {
        if(i2c_pgen->p_br.i2c_br_type == I2C_BR_SUSI)
        {
            bridge_channel = 0;
            SusiI2CWrite(id, i2c_pgen->p_br.bridge_addr, &bridge_channel, 1);
        }
    }
    
    sal_mutex_unlock (g_i2c_susi_mutex.mutex); 
    return ret;
}

/*********************************************************************
 * Name    : i2c_susi_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const i2c_gen_t *openinfo     - some info about the I2C bus implement
 
 * Output  : N/A
 * Return  : I2C bus handle
           
 * Note    : N/A
*********************************************************************/
i2c_handle_t *i2c_susi_create_handle(  i2c_gen_t* openinfo)
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

    if(g_i2c_susi_mutex.count == 0)
    {
        sal_mutex_create(&(g_i2c_susi_mutex.mutex));
    }
    
    phdl->close = i2c_susi_close;
    phdl->read = i2c_susi_read;
    phdl->write = i2c_susi_write;

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
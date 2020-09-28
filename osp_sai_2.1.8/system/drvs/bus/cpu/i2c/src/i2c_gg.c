/****************************************************************************
* $Id$
*  I2C driver implemented by I2C controller in GG
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : qianj
* Date          : 2017-08-29
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
#include "ctc_api.h"
#include "ctc_i2c.h"
#include "i2c_gg.h"
#include "drv_debug.h"
#include "i2c_err.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
struct i2c_gg_mutex_s{
    uint8 count;
    sal_mutex_t* mutex;
};
typedef struct i2c_gg_mutex_s i2c_gg_mutex_t;
i2c_gg_mutex_t g_i2c_gg_mutex={0, NULL};


/*********************************************************************
 * Name    : i2c_gg_close
 * Purpose :  free memory and pointer
 * Input   : i2c_handle_t *phdl       - the handler of the I2C bus
          
 * Output  : N/A
 * Return  : I2C_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
i2c_gg_close(i2c_handle_t* phdl)
{    
    DRV_CTC_CHK_PTR(phdl);

    if(g_i2c_gg_mutex.count > 0)
    {
        g_i2c_gg_mutex.count--;
        if(g_i2c_gg_mutex.count == 0)
        {
            sal_mutex_destroy(g_i2c_gg_mutex.mutex);
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

/* ret value: < 0 access fail
           >=0 access ok */
int32
i2c_gg_read(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    ctc_chip_i2c_read_t gg_para;
    i2c_gen_t* i2c_pgen = NULL;
    int32 ret = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);

    sal_mutex_lock (g_i2c_gg_mutex.mutex);   
    i2c_pgen = (i2c_gen_t *)phdl->data; 

    gg_para.slave_dev_id = i2c_pgen->slave_bitmap;    
    gg_para.slave_bitmap = 0;
    gg_para.lchip= i2c_pgen->lchip;
    gg_para.ctl_id= i2c_pgen->i2c_bus_idx;

    gg_para.dev_addr = i2c_pgen->addr;
    gg_para.offset = ppara->offset;
    gg_para.length = ppara->len;
    gg_para.buf_length = ppara->len;
    gg_para.p_buf = ppara->p_val;
    gg_para.i2c_switch_id = 0xf;

    ret = ctc_chip_i2c_read(&gg_para);
    sal_mutex_unlock (g_i2c_gg_mutex.mutex);
    return ret;
}


int32 
i2c_gg_write(const i2c_handle_t* phdl, i2c_op_para_t* ppara)
{
    ctc_chip_i2c_write_t gg_para;
    i2c_gen_t* i2c_pgen = NULL;
    int32 i;
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    sal_mutex_lock (g_i2c_gg_mutex.mutex);   
    i2c_pgen = (i2c_gen_t *)phdl->data; 
    gg_para.dev_addr = i2c_pgen->addr;
    gg_para.slave_id = i2c_pgen->slave_bitmap;
    gg_para.lchip = i2c_pgen->lchip;
    gg_para.ctl_id= i2c_pgen->i2c_bus_idx;
    gg_para.i2c_switch_id = 0xf;

    for(i=0; i<ppara->len; i++)
    {
        gg_para.offset = ppara->offset+i;
        gg_para.data = ppara->p_val[i];
        
        ret = ctc_chip_i2c_write(&gg_para);

        if (ret < 0)
        {
            goto exit;
        }    
    }
    
exit:
    sal_mutex_unlock (g_i2c_gg_mutex.mutex);   
    return ret;
}


/*********************************************************************
 * Name    : i2c_gg_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const i2c_gen_t *openinfo     - some info about the I2C bus implement
 
 * Output  : N/A
 * Return  : I2C bus handle
           
 * Note    : N/A
*********************************************************************/
i2c_handle_t *i2c_gg_create_handle(i2c_gen_t* openinfo)
{
    i2c_handle_t* phdl = NULL;    
    
    phdl = (i2c_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(i2c_handle_t));
    if (NULL == phdl)
    {
        goto err_out;
    }

    phdl->data = DRV_MALLOC(CTCLIB_MEM_DRIVER_I2C_INFO ,sizeof(i2c_gen_t));
    if (NULL == phdl->data)
    {
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->data, (int8 *)openinfo, sizeof(i2c_gen_t));

    if(g_i2c_gg_mutex.count == 0)
    {
        sal_mutex_create(&g_i2c_gg_mutex.mutex);
    }
    g_i2c_gg_mutex.count++;
    
    phdl->close = i2c_gg_close;
    phdl->read = i2c_gg_read;
    phdl->write = i2c_gg_write;
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


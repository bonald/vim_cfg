/****************************************************************************
 * vsc3308_api.c    vsc3308 api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Chenxi Qi
 * Date:         2012-11-05.
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
#include "vsc3308_drv.h"
#include "vsc3308_api.h"
#include "drv_log.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
static vsc3308_handle_t* g_vsc3308_hdl;
static uint8 g_vsc3308_chip_num;

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
static int32
_vsc3308_reg_read(vsc3308_handle_t* handle, uint8 reg, uint8 *value, uint8 len)
{
    vsc3308_access_t access;
    int ret;

    DRV_CTC_CHK_PTR(handle);
    DRV_CTC_CHK_PTR(handle->io_hdl);

    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_VSC3308, "VSC3308 reg read fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_vsc3308_reg_write(vsc3308_handle_t* handle, uint8 reg, uint8 value, uint8 len)
{
    vsc3308_access_t access;
    int ret;

    DRV_CTC_CHK_PTR(handle);
    DRV_CTC_CHK_PTR(handle->io_hdl);
    
    access.reg = reg;
    access.val = &value;
    access.len = len;
    ret = handle->io_hdl->write(handle->io_hdl, &access);      
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_VSC3308, "VSC3308 reg write fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

int32
vsc3308_read(uint8 chip_id, uint8 reg, uint8* value)
{
    if (chip_id < 0 || chip_id >= g_vsc3308_chip_num)
    {
        DRV_LOG_ERR("Invalid chip id %d, max port nums %d.\n", chip_id, g_vsc3308_chip_num);
        return RESULT_ERROR;
    }
    
    if (NULL != g_vsc3308_hdl[chip_id].reg_read)
    {
        return g_vsc3308_hdl[chip_id].reg_read(&g_vsc3308_hdl[chip_id], reg, value, 1);
    }
    return RESULT_ERROR;
}

int32
vsc3308_write(uint8 chip_id, uint8 reg, uint8 value)
{
    if (chip_id < 0 || chip_id >= g_vsc3308_chip_num)
    {
        DRV_LOG_ERR("Invalid chip id %d, max port nums %d.\n", chip_id, g_vsc3308_chip_num);
        return RESULT_ERROR;
    }
    
    if (NULL != g_vsc3308_hdl[chip_id].reg_write)
    {
        return g_vsc3308_hdl[chip_id].reg_write(&g_vsc3308_hdl[chip_id], reg, value, 1);
    }
    return RESULT_ERROR;
}

int32
vsc3308_config_input_output_pin(uint8 chip_id, uint8 input_pin, uint8 output_pin)
{
    int32 ret;
    
    ret = vsc3308_write(chip_id, 0x7f, 0x0);
    ret += vsc3308_write(chip_id, output_pin, input_pin); 
    ret += vsc3308_write(chip_id, 0x7f, 0x23);  /* Connection page */
    ret += vsc3308_write(chip_id, output_pin , 0x0); 
    
    return ret;
}

static int32
vsc3308_set_curr_page(vsc3308_handle_t* handle, uint8 curr_page)
{
    int32 ret;

    DRV_CTC_CHK_PTR(handle);
    DRV_CTC_CHK_PTR(handle->reg_write);
    
    ret = handle->reg_write(handle, 0x7f, curr_page, 1);
    return ret;
}

static int32
_vsc3308_switch_init(vsc3308_handle_t* handle)
{
    int32 ret;

    DRV_CTC_CHK_PTR(handle);
    DRV_CTC_CHK_PTR(handle->reg_write);

    /* VSC3308 core configuration to program a connection thru the switch core. */
    ret = vsc3308_set_curr_page(handle, 0x0); /* Connect registers are on page 00'h */
    ret += handle->reg_write(handle, 0x75, 0x01, 1); /* Use the CONFIG_WP bit */

    ret += handle->reg_write(handle, 0x54, 0x00, 1); 
    ret += handle->reg_write(handle, 0x5D, 0x02, 1); 
    
    return ret;
}

static int32
_vsc3308_buffer_init(vsc3308_handle_t* handle)
{
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(handle);
    DRV_CTC_CHK_PTR(handle->reg_write);

    /* VSC3308 core configuration to program a connection thru the switch core. */
    ret += vsc3308_set_curr_page(handle, 0x0); /* Connect registers are on page 00'h */
    ret += handle->reg_write(handle, 0x75, 0x01, 1); /* Use the CONFIG_WP bit */

    ret += handle->reg_write(handle, 0x54, 0x00, 1); 
    ret += handle->reg_write(handle, 0x5D, 0x02, 1); 
    
    ret  = handle->reg_write(handle, 0x7f, 0x10, 1);
    ret += handle->reg_write(handle, 0x00, 0x10, 1);
    ret += handle->reg_write(handle, 0x02, 0x10, 1);
    ret += handle->reg_write(handle, 0x7f, 0x11, 1);
    ret += handle->reg_write(handle, 0x00, 0x00, 1);
    ret += handle->reg_write(handle, 0x02, 0x00, 1);
    ret += handle->reg_write(handle, 0x7f, 0x12, 1);
    ret += handle->reg_write(handle, 0x00, 0x37, 1);
    ret += handle->reg_write(handle, 0x02, 0x37, 1);
    ret += handle->reg_write(handle, 0x7f, 0x20, 1);
    ret += handle->reg_write(handle, 0x00, 0x0f, 1);
    ret += handle->reg_write(handle, 0x02, 0x0f, 1);
    ret += handle->reg_write(handle, 0x7f, 0x21, 1);
    ret += handle->reg_write(handle, 0x00, 0x00, 1);
    ret += handle->reg_write(handle, 0x02, 0x00, 1);
    ret += handle->reg_write(handle, 0x7f, 0x22, 1);
    ret += handle->reg_write(handle, 0x00, 0x02, 1);
    ret += handle->reg_write(handle, 0x02, 0x03, 1);

    /*config channel to channel map*/
    ret += handle->reg_write(handle, 0x7f, 0x0, 1);
    ret += handle->reg_write(handle, 0x0, 0x0, 1);
    ret += handle->reg_write(handle, 0x1, 0x1, 1);
    ret += handle->reg_write(handle, 0x2, 0x2, 1);
    ret += handle->reg_write(handle, 0x3, 0x3, 1);
    ret += handle->reg_write(handle, 0x4, 0x4, 1);
    ret += handle->reg_write(handle, 0x5, 0x5, 1);
    ret += handle->reg_write(handle, 0x6, 0x6, 1);
    ret += handle->reg_write(handle, 0x7, 0x7, 1);
    /*config output power on*/
    ret += handle->reg_write(handle, 0x7f, 0x23, 1);
    ret += handle->reg_write(handle, 0x0, 0x0, 1);
    ret += handle->reg_write(handle, 0x1, 0x0, 1);
    ret += handle->reg_write(handle, 0x2, 0x0, 1);
    ret += handle->reg_write(handle, 0x3, 0x0, 1);
    ret += handle->reg_write(handle, 0x4, 0x0, 1);
    ret += handle->reg_write(handle, 0x5, 0x0, 1);
    ret += handle->reg_write(handle, 0x6, 0x0, 1);
    ret += handle->reg_write(handle, 0x7, 0x0, 1);
    return ret;
}

static int32
_vsc3308_switch_register_driver(vsc3308_handle_t* handle)
{
    handle->vsc3308_init = _vsc3308_switch_init;
    handle->reg_read = _vsc3308_reg_read;
    handle->reg_write = _vsc3308_reg_write;
    return RESULT_OK;
}

static int32
_vsc3308_buffer_register_driver(vsc3308_handle_t* handle)
{
    handle->vsc3308_init = _vsc3308_buffer_init;
    handle->reg_read = _vsc3308_reg_read;
    handle->reg_write = _vsc3308_reg_write;
    return RESULT_OK;
}

int32
vsc3308_dev_init(uint8 vsc3308_idx)
{
    if(vsc3308_idx >= g_vsc3308_chip_num)
    {
        DRV_LOG_ERR("VSC3308 id %d is error! max is %d\n", vsc3308_idx, g_vsc3308_chip_num);
        return RESULT_ERROR;
    }
    if(NULL != g_vsc3308_hdl[vsc3308_idx].vsc3308_init)
    {
        return g_vsc3308_hdl[vsc3308_idx].vsc3308_init(&g_vsc3308_hdl[vsc3308_idx]);
    }
    
    return RESULT_ERROR;
}

int32
vsc3308_init(void **p_data, vsc3308_chip_t* vsc3308_chip, uint8 chip_num)
{
    uint32 i;
    
    DRV_CTC_CHK_PTR(p_data);
    DRV_CTC_CHK_PTR(vsc3308_chip);
    
    g_vsc3308_hdl = (vsc3308_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_VSC3308_INFO, sizeof(vsc3308_handle_t)*chip_num);
    if(NULL == g_vsc3308_hdl)
    {
        DRV_LOG_ERR("VSC3308 alloc handler fail!\n");
        return RESULT_ERROR;
    }
    for (i = 0; i < chip_num; i++)
    {
        g_vsc3308_hdl[i].io_hdl = vsc3308_io_register(p_data[i]);
        switch(vsc3308_chip[i].chip_used_type)
        {
            case VSC3308_USED_AS_SWITCH:
                _vsc3308_switch_register_driver(&g_vsc3308_hdl[i]);
                break;
            case VSC3308_USED_AS_BUFFER:
                _vsc3308_buffer_register_driver(&g_vsc3308_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport vsc3308 chip used type %d!\n", vsc3308_chip[i].chip_used_type);
                DRV_LOG_USER(E_ERROR, DRV_3_VSC3308_USED_TYPE_UNKNOWN, vsc3308_chip[i].chip_used_type);
                return RESULT_ERROR;
        }
    }
    g_vsc3308_chip_num = chip_num;
    
	return RESULT_OK;
}



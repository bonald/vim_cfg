/****************************************************************************
 * mux_api.c    mux api 
 *
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liuht
 * Date:         2014-12-15.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "glb_hw_define.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "mux_api.h"
#include "mux_drv.h"
#include "epld_api.h"
/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define SN65LVCP114_VALUE_LEN 1


/* mux device handler */
static mux_handle_t* g_mux_hdl;
static uint8 g_mux_chip_num;
static uint8 g_mux_port_num;
static uint8 g_mux_lane_num;
static mux_chip_t *g_mux_chip;

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/



/****************************************************************************
 *
* Functions  
*
****************************************************************************/
static int32
_mux_sn65lvcp114_reg_read(mux_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    mux_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_mux_sn65lvcp114_reg_write(mux_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    mux_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_mux_sn65lvcp114_buffer_init(mux_handle_t* handle)
{    
    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_switch_init(mux_handle_t* handle)
{
    int32 ret;
    uint8 i;
    uint8 gain, eq, lane, port;
    uint8 port_num, lane_num;
    mux_info_t* mux_info;

    port_num = g_mux_port_num;
    lane_num = g_mux_lane_num;
    mux_info = (mux_info_t *)handle->info;
    
    for(i = 0; i < port_num*lane_num; i++)
    {
        /* pin polarity reverse */
        if(mux_info[i].out_inverse)
        {
            lane = mux_info[i].lane;
            port = mux_info[i].port;
            ret = handle->output_inverse(handle, lane, port, TRUE);
            if(ret<0)
            {
                DRV_LOG_ERR("Port %d lane %d output inverse fail!\n", port, lane);
                return RESULT_ERROR;
            }
        }
        if(mux_info[i].gain >= 0)
        {
            /* signal gain */
            lane = mux_info[i].lane;
            port = mux_info[i].port;
            gain = (uint8)mux_info[i].gain;
            ret = handle->gain(handle, lane, port, gain);
            if(ret<0)
            {
                DRV_LOG_ERR("Port %d lane %d gain fail!\n", port, lane);
                return RESULT_ERROR;
            }
        }
        if(mux_info[i].eq >= 0)
        {
            /* signal eq */
            lane = mux_info[i].lane;
            port = mux_info[i].port;
            eq = (uint8)mux_info[i].eq;
            ret = handle->eq(handle, lane, port, eq);
            if(ret<0)
            {
                DRV_LOG_ERR("Port %d lane %d eq fail!\n", port, lane);
                return RESULT_ERROR;
            }
        }
    }

    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_channel_select(mux_handle_t* handle, uint8 channel)
{   
    int32 ret;
    uint8 i;
    uint8 lane, port;
    uint8 port_num, lane_num;
    mux_info_t* mux_info;

    port_num = g_mux_port_num;
    lane_num = g_mux_lane_num;
    mux_info = (mux_info_t *)handle->info;

    for(i = 0; i < port_num*lane_num; i++)
    {
        if((channel == mux_info[i].channel) && (GLB_MUX_CHANNEL_NONE != channel))
        {
            lane = mux_info[i].lane;
            port = mux_info[i].port;
            ret = handle->lane_select(handle, lane, port);
            if(ret<0)
            {
                DRV_LOG_ERR("Channel %d select fail!\n", channel);
                return RESULT_ERROR;
            }
        }
    }

    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_lane_select(mux_handle_t* handle, uint8 lane, uint8 port)
{   
    int ret = 0;
    uint8 value = 0, tmp;
    
    ret = handle->reg_read(handle, 0x1, &value, SN65LVCP114_VALUE_LEN);
    if(ret < 0)
    {
        DRV_LOG_ERR("Read port %d lane %d lane select fail!\n", port, lane);
        return RESULT_ERROR;
    }
    tmp = value;
    
    if(MUX_PORTA == port)
    {
        value &= ~(1<<lane);
    }
    else if(MUX_PORTB== port)
    {
        value |= (1<<lane);
    }
    else
    {
        return RESULT_ERROR;
    }

    if(tmp != value)
    {
        ret = handle->reg_write(handle, 0x1, &value, SN65LVCP114_VALUE_LEN);
        if(ret < 0)
        {
            DRV_LOG_ERR("Write port %d lane %d lane select fail!\n", port, lane);
            return RESULT_ERROR;
        }
    }
    
    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_output_inverse(mux_handle_t* handle, uint8 lane, uint8 port, uint8 inverse)
{   
    int ret = 0;
    uint8 value = 0, tmp;
    uint8 reg;
    uint8 bitmap;

    if(MUX_PORTA == port)
    {
        reg = 0x10;
        bitmap = 1<<lane;
    }
    else if(MUX_PORTB== port)
    {
        reg = 0x10;
        bitmap = 1<<(lane+4);
    }
    else if(MUX_PORTC== port)
    {
        reg = 0x11;
        bitmap = 1<< lane;
    }
    else
    {
        return RESULT_ERROR;
    }
    
    ret = handle->reg_read(handle, reg, &value, SN65LVCP114_VALUE_LEN);
    if(ret<0)
    {
        return RESULT_ERROR;
    }
    tmp = value;

    if(inverse)
    {
        value |= bitmap;
    }
    else
    {
        value &= ~bitmap;
    }

    if(tmp != value)
    {
        ret = handle->reg_write(handle, reg, &value, SN65LVCP114_VALUE_LEN);
        if(ret<0)
        {
            return RESULT_ERROR;
        }
    }
    
    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_gain(mux_handle_t* handle, uint8 lane, uint8 port, uint8 gain)
{   
    int ret = 0;
    uint8 value = 0, tmp;
    uint8 reg;
    uint8 bitmap;

    if(MUX_PORTA == port)
    {
        reg = 0x04;
        bitmap = 0x33;
    }
    else if(MUX_PORTB== port)
    {
        reg = 0x08;
        bitmap = 0x33;
    }
    else if(MUX_PORTC== port)
    {
        reg = 0x0c;
        bitmap = 0x33;
    }
    else
    {
        return RESULT_ERROR;
    }
    
    ret = handle->reg_read(handle, reg, &value, SN65LVCP114_VALUE_LEN);
    if(ret<0)
    {
        return RESULT_ERROR;
    }
    tmp = value;

    value &= ~bitmap;
    gain &= bitmap;
    value |= gain;

    if(tmp != value)
    {
        ret = handle->reg_write(handle, reg, &value, SN65LVCP114_VALUE_LEN);
        if(ret<0)
        {
            return RESULT_ERROR;
        }
    }
    
    return RESULT_OK;
}

static int32
_mux_sn65lvcp114_eq(mux_handle_t* handle, uint8 lane, uint8 port, uint8 eq)
{   
    int ret = 0;
    uint8 value = 0, tmp;
    uint8 reg;
    uint8 bitmap;

    if(MUX_PORTA == port)
    {
        reg = 0x03;
        bitmap = 0x0f;
    }
    else if(MUX_PORTB== port)
    {
        reg = 0x07;
        bitmap = 0x0f;
    }
    else if(MUX_PORTC== port)
    {
        reg = 0x0b;
        bitmap = 0x0f;
    }
    else
    {
        return RESULT_ERROR;
    }
    
    ret = handle->reg_read(handle, reg, &value, SN65LVCP114_VALUE_LEN);
    if(ret<0)
    {
        return RESULT_ERROR;
    }
    tmp = value;

    value &= ~bitmap;
    eq &= bitmap;
    value |= eq;

    if(tmp != value)
    {
        ret = handle->reg_write(handle, reg, &value, SN65LVCP114_VALUE_LEN);
        if(ret<0)
        {
            return RESULT_ERROR;
        }
    }
    
    return RESULT_OK;
}

static int32 
_mux_sn65lvcp114_register_driver(mux_handle_t* handle, mux_cfg_e type)
{

    switch(type)
    {
        case MUX_CONFIG_AS_SWITCH:
            handle->mux_init = _mux_sn65lvcp114_switch_init;
            handle->channel_select = _mux_sn65lvcp114_channel_select;
            handle->lane_select = _mux_sn65lvcp114_lane_select;
            handle->output_inverse = _mux_sn65lvcp114_output_inverse;
            handle->gain = _mux_sn65lvcp114_gain;
            handle->eq = _mux_sn65lvcp114_eq;
            break;
        case MUX_CONFIG_AS_BUFF:
            handle->mux_init = _mux_sn65lvcp114_buffer_init;
            break;
        default:
            break;        
    }

    handle->reg_read = _mux_sn65lvcp114_reg_read;
    handle->reg_write = _mux_sn65lvcp114_reg_write;

    return RESULT_OK;
}


int32
mux_dev_init(uint32 chip_id)
{
    if(chip_id >= g_mux_chip_num)
    {
        DRV_LOG_ERR("Mux chip id %d is error!, max is %d\n", chip_id, g_mux_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_mux_hdl[chip_id].mux_init);
    return g_mux_hdl[chip_id].mux_init(&g_mux_hdl[chip_id]);
}

int32
mux_reg_read(uint32 chip_id, uint8 reg, uint8* value, uint8 len)
{
    if(chip_id >= g_mux_chip_num)
    {
        DRV_LOG_ERR("Mux chip id %d is error!, max is %d\n", chip_id, g_mux_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_mux_hdl[chip_id].reg_read);
    return g_mux_hdl[chip_id].reg_read(&g_mux_hdl[chip_id], reg, value, len);
}

int32
mux_reg_write(uint32 chip_id, uint8 reg, uint8* value, uint8 len)
{
    if(chip_id >= g_mux_chip_num)
    {
        DRV_LOG_ERR("Mux chip id %d is error!, max is %d\n", chip_id, g_mux_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_mux_hdl[chip_id].reg_read);
    return g_mux_hdl[chip_id].reg_write(&g_mux_hdl[chip_id], reg, value, len);
}

int32
mux_channel_select(uint32 chip_id, uint8 channel)
{
    if(chip_id >= g_mux_chip_num)
    {
        DRV_LOG_ERR("Mux chip id %d is error!, max is %d\n", chip_id, g_mux_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_mux_hdl[chip_id].channel_select);
    return g_mux_hdl[chip_id].channel_select(&g_mux_hdl[chip_id], channel);
}


/*********************************************************************
 * Name    : mux_init
 * Purpose : init some data structure and register mux dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of mux'i2c bus implement
             uint32 num             - the number of mux device
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
mux_init(void** p_data, mux_chip_t* type, mux_info_t** mux_info, uint8 chip_num)
{
    int32 i = 0;
    uint8 port_num = 0;
    uint8 lane_num = 0;

    DRV_CTC_CHK_PTR(p_data);  
    DRV_CTC_CHK_PTR(type);
    
    g_mux_hdl = (mux_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_MUX_INFO, sizeof(mux_handle_t)*chip_num);
    if(NULL == g_mux_hdl)
    {
        DRV_LOG_ERR("Mux alloc handler fail!\n");
        return RESULT_ERROR;
    }
    
    /* register mux dev */
    for (i = 0; i < chip_num; i++)
    {
        port_num = type[i].port_num;
        lane_num = type[i].lane_num;
        g_mux_hdl[i].info = (mux_info_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_MUX_INFO, sizeof(mux_info_t)*port_num*lane_num);
        if(NULL == g_mux_hdl[i].info)
        {
            DRV_LOG_ERR("Mux alloc mux info fail!\n");
            return RESULT_ERROR;
        }
        sal_memcpy((int8 *)g_mux_hdl[i].info, (int8 *)mux_info[i], sizeof(mux_info_t)*port_num*lane_num);

        g_mux_hdl[i].io_hdl = mux_io_register(type[i].io_type, p_data[i]);
        g_mux_hdl[i].access_err_cnt = 0;
        g_mux_chip = type;	
        switch(type[i].chip_type)
        {
            case MUX_SN65LVCP114:
                _mux_sn65lvcp114_register_driver(&g_mux_hdl[i], type[i].cfg_type);
                break;
            default:
                DRV_LOG_ERR("Unsupport mux chip type %d!\n", type[i].chip_type);
                return RESULT_ERROR;
        }
    }

    g_mux_chip_num = chip_num;
    g_mux_port_num = port_num;
    g_mux_lane_num = lane_num;
    return RESULT_OK;
}


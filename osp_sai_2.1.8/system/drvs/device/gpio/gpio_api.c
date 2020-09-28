/****************************************************************************
 * gpio_api.c    gpio api 
 *
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liuht
 * Date:         2014-12-06.
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
#include "gpio_api.h"
#include "gpio_drv.h"
#include "epld_api.h"
/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
/* gpio device handler */
static gpio_handle_t* g_gpio_hdl;
static uint32 g_gpio_chip_num;

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
_gpio_pca95x5_reg_read(gpio_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    gpio_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_gpio_pca95x5_reg_write(gpio_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    gpio_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32 
_gpio_pca9505_register_driver(gpio_handle_t* handle)
{
    handle->chip_type = GPIO_PCA9505;
    handle->reg_read = _gpio_pca95x5_reg_read;
    handle->reg_write = _gpio_pca95x5_reg_write;

    return RESULT_OK;
}

static int32 
_gpio_pca9535_register_driver(gpio_handle_t* handle)
{
    handle->chip_type = GPIO_PCA9535;
    handle->reg_read = _gpio_pca95x5_reg_read;
    handle->reg_write = _gpio_pca95x5_reg_write;

    return RESULT_OK;
}

int32
gpio_reg_read(uint32 chip_id, uint8 reg, uint8* value, uint8 len)
{
    if(chip_id >= g_gpio_chip_num)
    {
        DRV_LOG_ERR("Gpio chip id %d is error!, max is %d\n", chip_id, g_gpio_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_gpio_hdl[chip_id].reg_read);
    return g_gpio_hdl[chip_id].reg_read(&g_gpio_hdl[chip_id], reg, value, len);
}

int32
gpio_reg_write(uint32 chip_id, uint8 reg, uint8* value, uint8 len)
{
    if(chip_id >= g_gpio_chip_num)
    {
        DRV_LOG_ERR("Gpio chip id %d is error!, max is %d\n", chip_id, g_gpio_chip_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_gpio_hdl[chip_id].reg_write);
    return g_gpio_hdl[chip_id].reg_write(&g_gpio_hdl[chip_id], reg, value, len);
}
/*****************************************************************************
 * Name         :   gpio_get_scan_special_bit_value
 * Purpose      :   gpio get special bit value when gpio_scan_real_time_value func is periodly called.
 * Input        :   gpio chip id, bit No.  
 *              :   value   : 1/0
 * Return       :    
 * Note         :   This func just get value stored in buffer, will not access real register.
 *****************************************************************************/
int32
gpio_get_scan_special_bit_value(uint8 chip, uint8 no, uint8 *value)
{
    uint8 val;

    if(chip >= g_gpio_chip_num)
    {
        DRV_LOG_ERR("Gpio chip id %d is error!, max is %d\n", chip, g_gpio_chip_num);
        return RESULT_ERROR;
    }
    val = g_gpio_hdl[chip].scan_value[no/PCA95X5_PORTS_PER_BANK];
    
    if(val & (1<<(no%PCA95X5_PORTS_PER_BANK)))
    {
        *value = 1;
    }
    else
    {
        *value = 0;
    }
    return RESULT_OK;     
}

/*****************************************************************************
 * Name         :   gpio_set_special_bit
 * Purpose      :   gpio set special bit value 
 * Input        :   gpio chip id, bit No.  
 *              :   value   : 1/0
 * Return       :    
 * Note         :
 *****************************************************************************/
int32
gpio_set_special_bit(uint8 chip, uint8 no, uint8 value)
{
    int ret = 0;
    uint8 val, reg;
    uint8 output_port_reg_bank0;

    switch(g_gpio_hdl[chip].chip_type)
    {
    case GPIO_PCA9505:
    case GPIO_PCA9506:
        output_port_reg_bank0 = PCA9505_OUTPUT_PORT_REG_BANK0;
        break;
    case GPIO_AT9555:
    case GPIO_PCA9535:
        output_port_reg_bank0 = PCA9535_OUTPUT_PORT_REG_BANK0;
        break;
    default:
        output_port_reg_bank0 = 0;
        return RESULT_ERROR;
    }

    reg = output_port_reg_bank0+no/PCA95X5_PORTS_PER_BANK;
    ret = gpio_reg_read(chip, reg, &val, 1);
    if(value)
    {
        val |= (1<<(no%PCA95X5_PORTS_PER_BANK));
    }
    else
    {
        val &= ~(1<<(no%PCA95X5_PORTS_PER_BANK));        
    }
    ret += gpio_reg_write(chip, reg, &val, 1);
    return ret;  
}

/*****************************************************************************
 * Name         :   gpio_get_chip_type
 * Purpose      :   gpio get chip type 
 * Input        :   gpio chip id  
 * Return       :    
 * Note         :
 *****************************************************************************/
int32
gpio_get_diag_bank(uint8 chip_id)
{
    uint8 reg;
    
    /*0. get testcase per chip type */
    switch(g_gpio_hdl[chip_id].chip_type)
    {
        case GPIO_PCA9505:
        case GPIO_PCA9506:
            reg = PCA9505_DIR_CTRL_REG_BANK0;
        break;
        case GPIO_AT9555:
        case GPIO_PCA9535:
            reg = PCA9535_DIR_CTRL_REG_BANK0;
        break;
        default:
            reg = PCA9505_DIR_CTRL_REG_BANK0;
        break;
    }

    return reg;
}

/*****************************************************************************
 * Name         :   gpio_scan_real_time_value
 * Purpose      :   polling thread should call this func periodly to get real time value from gpio chip
 * Input        :      
 * Output       :   
 * Return       :   
 * Note         :   scan all of gpio chip
 *****************************************************************************/
void
gpio_scan_real_time_value()
{
    uint8 i, j;
    uint8 value;
    uint8 bank_num;
    
    for(i=0; i<g_gpio_chip_num; i++)
    {
        switch(g_gpio_hdl[i].chip_type)
        {
        case GPIO_PCA9505:
        case GPIO_PCA9506:
            bank_num = PCA9505_BANK_NUM;
            break;
        case GPIO_AT9555:
        case GPIO_PCA9535:
            bank_num = PCA9535_BANK_NUM;
            break;
        default:
            bank_num = 0;
        }
        
        for(j=0; j<bank_num; j++)
        {
            /*need scan this bank*/
            if(g_gpio_hdl[i].scan_group_bitmap & (1<<j))
            {
                if(RESULT_OK == gpio_reg_read(i, PCA9505_INPUT_PORT_REG_BANK0+j, &value, 1))
                {
                    g_gpio_hdl[i].scan_value[j] = value;
                }
            }
            else
            {
                g_gpio_hdl[i].scan_value[j] = 0;
            }
        }
    }    
}

void gpio_set_scan_group(uint8 chip,  uint8 value)
{
    g_gpio_hdl[chip].scan_group_bitmap = value;
}

/*********************************************************************
 * Name    : gpio_init
 * Purpose : init some data structure and register gpio dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of gpio'i2c bus implement
             uint32 num             - the number of gpio device
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
gpio_init(void** p_data, gpio_chip_t* type, uint32 num)
{
    int32 i = 0;

    DRV_CTC_CHK_PTR(p_data);  
    DRV_CTC_CHK_PTR(type);
    
    g_gpio_hdl = (gpio_handle_t* )DRV_CALLOC(CTCLIB_MEM_DRIVER_GPIO_INFO, sizeof(gpio_handle_t)*num);
    if(NULL == g_gpio_hdl)
    {
        DRV_LOG_ERR("Gpio alloc handler fail!\n");
        return RESULT_ERROR;
    }

    /* register gpio dev */
    for (i = 0; i < num; i++)
    {      
        g_gpio_hdl[i].io_hdl = gpio_io_register(type[i].io_type, p_data[i]);
        g_gpio_hdl[i].scan_group_bitmap = type[i].scan_group_bitmap;
        switch(type[i].chip_type)
        {
            case GPIO_PCA9505:
            case GPIO_PCA9506:
                _gpio_pca9505_register_driver(&g_gpio_hdl[i]);
                break;
            case GPIO_AT9555:
            case GPIO_PCA9535:
                _gpio_pca9535_register_driver(&g_gpio_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport gpio chip type %d!\n", type[i].chip_type);
                return RESULT_ERROR;
        }
    }

    g_gpio_chip_num = num;
    return RESULT_OK;
}


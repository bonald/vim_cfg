/****************************************************************************
 * ctc_i2c.c   I2C devices access and getting the information
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-09-06.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_i2c.h"
#include "i2c_cpm.h"
#include "i2c_smbus.h"
#include "i2c_gb.h"
#include "i2c_gg.h"
#include "i2c_gpio.h"
#include "drv_debug.h"
#include "i2c_err.h"
#include "ctc_gpio.h"
#include "i2c_susi.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/



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

/****************************************************************************
 * Name : i2c_open
 * Purpose: open the i2c dev driver interface
 * Input    :  
 * Output:   
 * Return:  
 * Note :
****************************************************************************/   
int32 
i2c_open(i2c_type_e type)
{
    int32 ret = 0;
    
    DRV_LOG_DEBUG(DRV_BUS_I2C, "open I2C dev, type %d", type); 
    
    switch (type)
    {
    case E_I2C_CPM:
        ret = i2c_cpm_open();
        break;
    case E_I2C_GPIO:
        ret = i2c_gpio_open();
        break;
#ifdef _CTC_X86_64_
    case E_I2C_SMBUS:
        ret = i2c_smbus_open();
        break;
    case E_I2C_SUSI:
        ret = i2c_susi_open();
        break;
#endif
    default:
        DRV_LOG_DEBUG(DRV_BUS_I2C, " not support i2c type %d", type); 
        break;
    }
    
    return ret;
}

/****************************************************************************
 * Name : i2c_switch_msg
 * Purpose: set the msg of the i2c
 * Input    : the poniter of msg, read/write flag
 * Output:    one of i2c message
 * Return:  
 * Note :
****************************************************************************/   
int32 
i2c_switch_msg(struct ctc_i2c_msg* msg, uint16 addr, 
                         uint8* p_buf, uint16 flag, int32 len)
{
    DRV_LOG_DEBUG(DRV_BUS_I2C, "set I2C msg, addr: %d, flag %d, len %d", addr, flag, len);
    
    DRV_CTC_CHK_PTR(msg);
    
    msg->addr = addr;
    msg->buf = p_buf;
    msg->flags = flag;
    msg->len = len;
    return I2C_SUCCESS;
}
   
/*********************************************************************
 * Name    : i2c_create_bus_handle
 * Purpose :  create the i2c device bus handler  
 * Input   : i2c_gen_t *i2c_pgen     - information of the i2c device
 
 * Output  : N/A
 * Return  : i2c bus handler
           
 * Note    : N/A
*********************************************************************/

i2c_handle_t*
i2c_create_bus_handle( i2c_gen_t* i2c_pgen)
{
    i2c_handle_t* phdl = NULL;
     
    DRV_LOG_DEBUG(DRV_BUS_I2C, "create I2C bus handle, type %d", i2c_pgen->i2c_type); 
    
    DRV_CTC_CHK_PTR_NULL(i2c_pgen);
    
    switch (i2c_pgen->i2c_type)
    {
    case E_I2C_GPIO:
        phdl = i2c_gpio_create_handle(i2c_pgen);
        break;
    case E_I2C_CPM:
    case E_I2C_CPM_RAW:
        phdl = i2c_cpm_create_handle(i2c_pgen);
        break;
    case E_I2C_GB:
        phdl = i2c_gb_create_handle(i2c_pgen);
        break;
    case E_I2C_GG:
        phdl = i2c_gg_create_handle(i2c_pgen);
        break;
#ifdef _CTC_X86_64_
    case E_I2C_SMBUS:
        phdl = i2c_smbus_create_handle(i2c_pgen);
        break;
    case E_I2C_SUSI:
        phdl = i2c_susi_create_handle(i2c_pgen);
        break;
#endif
    default:
        DRV_LOG_ERR( "i2c_create_bus_handle don't support this type!\n");
        break;
    }
    
    return phdl;
}

/*********************************************************************
 * Name    : i2c_create_handle
 * Purpose :  create the i2c device  handler  
 * Input   : i2c_gen_t *i2c_pgen     - information of the i2c device
 
 * Output  : N/A
 * Return  : i2c bus handler
           
 * Note    : N/A
*********************************************************************/

i2c_handle_t *
i2c_create_handle( i2c_gen_t* i2c_pgen)
{
    i2c_handle_t* p_i2c_hdl = NULL;
    
    DRV_LOG_DEBUG(DRV_BUS_I2C, "i2c_create_handle"); 
    
    DRV_CTC_CHK_PTR_NULL(i2c_pgen);

    p_i2c_hdl = i2c_create_bus_handle(i2c_pgen);
    DRV_CTC_CHK_PTR_NULL(p_i2c_hdl);

    return p_i2c_hdl;    
}



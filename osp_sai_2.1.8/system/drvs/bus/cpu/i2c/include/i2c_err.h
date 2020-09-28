/****************************************************************************
 * $Id$
 *  i2c err
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Jcao
 * Date          : 2010-09-07 
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __I2C_ERR_H__
#define __I2C_ERR_H__

enum i2c_error_e
{
    I2C_SUCCESS = 0,
    I2C_E_ERROR = -999,
    I2C_E_NO_MEMORY,
    I2C_E_NOT_INIT,
    I2C_E_INIT_FAILED,
    I2C_E_TIMEOUT,    
    I2C_E_READ,    
    I2C_E_WRITE,
    /* parameter check error */
    I2C_E_INVALID_PARAM,
    I2C_E_INVALID_PTR,
    I2C_E_INVALID_INDEX,    
    I2C_E_INVALID_LENGTH,
    I2C_E_INVALID_CHIP,

};
typedef enum i2c_error_e i2c_error_t;


#endif /*!__I2C_ERR_H__*/


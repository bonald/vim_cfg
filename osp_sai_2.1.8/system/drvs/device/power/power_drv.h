/****************************************************************************
* $Id$
*  The header file of the power operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-10-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __POWER_DRV__
#define __POWER_DRV__

#include "glb_hw_define.h"

typedef enum
{
    PSU_IO_EPLD,  /*access the psu info by EPLD*/
    /* added by liuht for bug 24525,2013-10-24 */		
    PSU_IO_I2C_EPLD,    /*access the psu info by I2C, manage by epld*/
    PSU_IO_I2C_GPIO,  /*access the psu info by I2C, manage by gpio*/
    PSU_IO_GPIO,  /*access the psu info by gpio, manage by gpio*/
}psu_io_type_e;

/* added by liuht for bug 24525,2013-10-24 */
struct psu_access_s
{
    uint8 reg;     /* psu chip register*/   
    uint8 *val;    /* pointer of the value buffur */
    uint8 len;
} ;
typedef struct psu_access_s psu_access_t;

typedef struct
{
    uint8 present_chip;
    uint8 present_no;
    uint8 present_reverse;/* default value 1 means absent, value 0 means present */
    uint8 workstate_chip;
    uint8 workstate_no;
    uint8 workstate_reverse;/* default value 1 means power fail, value 0 means power good */
    uint8 alert_chip;
    uint8 alert_no;
    uint8 alert_reverse;/* default value 1 means alert, value 0 means no alert */
    uint8 power_mode_chip;
    uint8 power_mode_no;
    uint8 power_mode_reverse;/* default value 1 means DC power, value 0 means AC power */

}psu_data_gpio;

typedef struct psu_io_handle_s psu_io_handle_t;
struct psu_io_handle_s
{
    /* added by liuht for bug 24525,2013-10-24 */
    int32 (*read)(const psu_io_handle_t *, psu_access_t *);
    int32 (*write)(const psu_io_handle_t *, psu_access_t *);
    /* modify attribute of io layer */
    int32 (*mattr)(const psu_io_handle_t *, uint8);    
    void* handle; /* handle of the low level operations, current it is i2c handle*/
    void* p_data_epld; /*private data for epld */
    void* p_data_gpio; /*private data for gpio*/
    /*Note: for i2c handle, the private data for i2c is stored in i2c handle. so this level has no data ptr.*/
};

psu_io_handle_t * psu_io_register(psu_io_type_e type, const void *p_data);



#endif 


/****************************************************************************
 * sysenv_drv.c    sysenv drv 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2019-01-17.
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
#include "sysenv_drv.h"
#include "sysenv_spiflash.h"
#include "sysenv_eeprom.h"

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

sysenv_handle_t * sysenv_register_driver(sysenv_chip_info_t *sysenv_chip_info)
{
        sysenv_handle_t *hdl;
        
        switch(sysenv_chip_info->sysenv_chip_type)
        {
            case SYSENV_CHIP_SPIFLASH:
                hdl = sysenv_spiflash_register_driver(sysenv_chip_info);
                break;
            case SYSENV_CHIP_EEPROM:
                hdl = sysenv_eeprom_register_driver(sysenv_chip_info);
                break;
            default:
                DRV_LOG_ERR("Unsupport sysenv chip type %d!\n", sysenv_chip_info->sysenv_chip_type);
                return NULL;
        }

        return hdl;
}
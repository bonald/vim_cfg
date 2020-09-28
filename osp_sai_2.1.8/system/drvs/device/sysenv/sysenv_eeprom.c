/****************************************************************************
 * sysenv_eeprom.c   eeprom access interface
 *
 * Copyright:    (c)2005 Actus Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2019-01-16.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_const.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "sysenv_api.h"
#include "sysenv_eeprom.h"
#include "eeprom_api.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/


/****************************************************************************
 *
* Functions  
*
****************************************************************************/

int32 sysenv_eeprom_close(sysenv_handle_t *hdl)
{
    sysenv_eeprom_info_t *phdl_eeprom_info = hdl->sysenv_chip_info.sysenv_gen;
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, phdl_eeprom_info);
    hdl->sysenv_chip_info.sysenv_gen = NULL;
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, hdl);
    
    return SYSENV_SUCCESS;
}

int32 sysenv_eeprom_read(const sysenv_handle_t *hdl, uint16 offset, uint16 len, uint8 *p_val)
{
    sysenv_eeprom_info_t *phdl_eeprom_info = hdl->sysenv_chip_info.sysenv_gen;
    eeprom_para_t eeprom_para;

    eeprom_para.offset = offset;
    eeprom_para.len = len;
    eeprom_para.p_val = p_val;

    return eeprom_read(phdl_eeprom_info->idx, &eeprom_para);
}

int32 sysenv_eeprom_write(const sysenv_handle_t *hdl, uint16 offset, uint16 len, uint8 *p_val)
{
    sysenv_eeprom_info_t *phdl_eeprom_info = hdl->sysenv_chip_info.sysenv_gen;
    eeprom_para_t eeprom_para;

    eeprom_para.offset = offset;
    eeprom_para.len = len;
    eeprom_para.p_val = p_val;

    return eeprom_write(phdl_eeprom_info->idx, &eeprom_para);
}


sysenv_handle_t *sysenv_eeprom_register_driver(sysenv_chip_info_t *sysenv_chip_info)
{
    sysenv_eeprom_info_t *p_eeprom_info = (sysenv_eeprom_info_t *)sysenv_chip_info->sysenv_gen;
    sysenv_eeprom_info_t *phdl_eeprom_info = NULL;

    /* malloc space for eeprom hdl*/
    sysenv_handle_t *hdl = (sysenv_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, sizeof(sysenv_handle_t));
    if(NULL == hdl)
    {
        DRV_LOG_ERR("sysenv alloc eeprom hdl space fail!\n");
        return NULL;
    }
    sal_memset(hdl, 0, sizeof(sysenv_handle_t));

    /* malloc space for eeprom hdl->sysenv_chip_info.sysenv_gen */
    hdl->sysenv_chip_info.sysenv_gen = DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, sizeof(sysenv_eeprom_info_t));
    if(NULL == hdl->sysenv_chip_info.sysenv_gen)
    {
        DRV_LOG_ERR("sysenv alloc eeprom sysenv_gen space fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, hdl);
        return NULL;
    }
    sal_memset(hdl->sysenv_chip_info.sysenv_gen, 0, sizeof(sysenv_eeprom_info_t));
    phdl_eeprom_info = hdl->sysenv_chip_info.sysenv_gen;

    /* init sysenv_chip_info.sysenv_gen */
    phdl_eeprom_info->idx = p_eeprom_info->idx;

    /* init sysenv_chip_info.sysenv_chip_type */
    hdl->sysenv_chip_info.sysenv_chip_type = sysenv_chip_info->sysenv_chip_type;

    /* init read write close func */
    hdl->close = sysenv_eeprom_close;
    hdl->read = sysenv_eeprom_read;
    hdl->write = sysenv_eeprom_write;

    return hdl;
}

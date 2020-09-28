/****************************************************************************
* $Id$
*  The header file of the sysenv eeprom operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2019-01-16 
* Reason        : First Create.
****************************************************************************/
#ifndef __SYSENV_EEPROM_H__
#define __SYSENV_EEPROM_H__

#include "sysenv_api.h"

typedef struct
{
    uint32_t idx;
} sysenv_eeprom_info_t;

sysenv_handle_t *sysenv_eeprom_register_driver(sysenv_chip_info_t *sysenv_chip_info);

#endif 

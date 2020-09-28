/****************************************************************************
* $Id$
*  The header file of the sysenv drv operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2019-01-17
* Reason        : First Create.
****************************************************************************/

#ifndef __SYSENV_DRV_H__
#define __SYSENV_DRV_H__

typedef enum 
{
    SYSENV_CHIP_SPIFLASH,
    SYSENV_CHIP_EEPROM,
    SYSENV_CHIP_MAX,
} sysenv_chip_t;

typedef struct
{
    sysenv_chip_t sysenv_chip_type;
    void *sysenv_gen;
} sysenv_chip_info_t;

typedef struct sysenv_handle_s sysenv_handle_t;
struct sysenv_handle_s
{
    int32 (* close)(sysenv_handle_t *);
    int32 (* read)(const sysenv_handle_t *, uint16, uint16, uint8 *);
    int32 (* write)(const sysenv_handle_t *, uint16, uint16, uint8 *);
    sysenv_chip_info_t sysenv_chip_info;
};

sysenv_handle_t * sysenv_register_driver(sysenv_chip_info_t *sysenv_chip_info);

#endif /* __SYSENV_DRV_H__ */

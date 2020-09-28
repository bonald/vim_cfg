/****************************************************************************
* $Id$
*  The header file of the sysenv spiflash operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2019-01-16 
* Reason        : First Create.
****************************************************************************/
#ifndef __SYSENV_SPIFLASH_H__
#define __SYSENV_SPIFLASH_H__

#include "sysenv_api.h"

typedef struct
{
    char dev_name [64];
    int dev_fd;
    int base_offset;
    int block_size;
    int block_num;
} sysenv_spiflash_info_t;

sysenv_handle_t *sysenv_spiflash_register_driver(sysenv_chip_info_t *sysenv_chip_info);

#endif 

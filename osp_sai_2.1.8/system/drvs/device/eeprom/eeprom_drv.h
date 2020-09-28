/****************************************************************************
* $Id$
*  The header file of the eeprom operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-26
* Reason        : First Create.
****************************************************************************/
#ifndef __EEPROM_DRV__
#define __EEPROM_DRV__

#include "ctc_i2c.h"

enum eeprom_type_s
{
    E_EEPROM_I2C = 0,        /* access the EEPROM by I2C bus */
};
typedef enum eeprom_type_s eeprom_type_t;

/* define the structure including i2c operation paramaters */
struct eeprom_access_s{
    uint32 len;              /* eeprom data length */
    uint32 offset;           /* the offset of the eeprom internal address */
    uint8* p_val;              /* pointer of the value buffur */
} ;
typedef struct eeprom_access_s eeprom_access_t;

typedef struct eeprom_handle_s eeprom_handle_t;
struct eeprom_handle_s
{
    int32 (*close)(eeprom_handle_t *);
    int32 (*read)(const eeprom_handle_t *, eeprom_access_t *);
    int32 (*write)(const eeprom_handle_t *, eeprom_access_t *);
    void* handle;      /*handle of the low level operations */
};

eeprom_handle_t *eeprom_register(eeprom_type_t type, const void * p_gen);

#endif  /* __EEPROM_DRV__ */


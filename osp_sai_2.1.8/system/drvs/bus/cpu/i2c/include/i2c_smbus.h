/****************************************************************************
* $Id$
*  The header file of the I2C driver implemented by CPU SMBus.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-02-06
* Reason        : First Create.
****************************************************************************/
#ifndef __I2C_SMBUS_H__
#define __I2C_SMBUS_H__

i2c_handle_t *i2c_smbus_create_handle(i2c_gen_t *openinfo);
int32 i2c_smbus_open();

int32 raw_i2c_smbus_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 raw_i2c_smbus_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);

#endif

/****************************************************************************
* $Id$
*  The header file of the I2C driver implemented by I2C controller in CPM.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-07 
* Reason        : First Create.
****************************************************************************/
#ifndef __I2C_GPIO_H__
#define __I2C_GPIO_H__

i2c_handle_t *i2c_gpio_create_handle( i2c_gen_t *openinfo);
extern inline int i2c_switch_msg(struct ctc_i2c_msg *msg, uint16 addr, uint8 *p_buf, uint16 flag, int32 len);
int32 raw_i2c_gpio_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 raw_i2c_gpio_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 i2c_gpio_open();

#endif

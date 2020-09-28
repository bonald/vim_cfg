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
#ifndef __I2C_CPM_H__
#define __I2C_CPM_H__

i2c_handle_t *i2c_cpm_create_handle( i2c_gen_t *openinfo);
int32 i2c_cpm_open();
int i2c_transfer(uint8 i2c_bus_idx, struct i2c_rdwr_ioctl_data *msgset);
extern int i2c_switch_msg(struct ctc_i2c_msg *msg, uint16 addr, uint8 *p_buf, uint16 flag, int32 len);
int32 raw_bridge_i2c_read(uint8 i2c_bus_idx,uint32 bridge_addr,uint32 bridge_ch, uint16 chip_addr, uint32 reg_addr
                                     , uint8 alen, uint8* data_buf, uint32 data_len);

int32 raw_i2c_cpm_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 raw_i2c_cpm_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 raw_i2c_cpm_transfer(uint8 i2c_bus_idx, uint16 addr, uint8 type , uint8* buf, uint8 len);

#endif


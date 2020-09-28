/****************************************************************************
* $Id$
*  The header file of the I2C driver implemented by Advantech SUSI.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-03-05
* Reason        : First Create.
****************************************************************************/
#ifndef __I2C_SUSI_H__
#define __I2C_SUSI_H__

i2c_handle_t *i2c_susi_create_handle(i2c_gen_t *openinfo);
int32 i2c_susi_open();

#endif

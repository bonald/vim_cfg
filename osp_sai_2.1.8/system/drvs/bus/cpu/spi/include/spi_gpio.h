/****************************************************************************
* $Id$
*  SPI common defines
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-08-04 
* Reason        : First Create.
****************************************************************************/
#ifndef __SPI_GPIO_H__
#define __SPI_GPIO_H__

int32 raw_gpio_spi_write(uint32 addr, uint32 alen, uint32 value, uint32 len);
int32 raw_gpio_spi_read(uint32 addr, uint32 alen, uint8 *buf, uint32 len);
spi_handle_t *spi_gpio_create_handle(const spi_gen_t *spi_info);


#endif



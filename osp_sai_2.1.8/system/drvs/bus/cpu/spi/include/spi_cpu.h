/****************************************************************************
* $Id$
*  SPI CPU header file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-10-29 
* Reason        : First Create.
****************************************************************************/
#ifndef __SPI_CPU_H__
#define __SPI_CPU_H__

//#define CTC_SPI_DEV_NAME     "/dev/ctc_spi"


spi_handle_t *spi_cpu_create_handle(const spi_gen_t *spi_info);


#endif /* !__SPI_CPU_H__ */



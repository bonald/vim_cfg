/****************************************************************************
* $Id$
*  The header file of the ds3104 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2011-02-17 
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_DS3104__
#define __CLOCK_DS3104__

#include "ctc_spi.h"


#define DS3104_DATA_LENTH 1    /* data length (byte)*/  
#define DS3104_ADDRESS_LENTH 2  /* address length (byte)*/ 

#define DS3104_SPI_READ  0x1
#define DS3104_SPI_WRITE 0x0
/*1 = burst access, 0 = single access*/
#define DS3104_SPI_ACCESS_MODE 0x0


#define AD_SPI_READ  0x1
#define AD_SPI_WRITE 0x0
#define AD_SPI_WIDTH_1 0x0
#define AD_SPI_WIDTH_2 0x1
#define AD_SPI_WIDTH_3 0x2
#define AD_SPI_STREAM  0x3


#define AD_VCO_CAL_FINISHED 0x6
#define AD_DIGITAL_LOCK     0x0

int32 clock_ds3104_register_driver(clock_handle_t* hdl, clock_info_t* clock_info);

#endif 


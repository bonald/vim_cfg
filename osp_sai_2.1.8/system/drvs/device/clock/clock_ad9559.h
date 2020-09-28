/****************************************************************************
* $Id$
*  The header file of the ad9559 operation.
*
* (C) Copyright Actus Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chani
* Date          : 2013-09-30 
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_AD9559__
#define __CLOCK_AD9559__

#include "ctc_spi.h"

#define AD9559_DATA_LENTH 1    /* data length (byte)*/  
#define AD9559_ADDRESS_LENTH 2  /* address length (byte)*/ 

#define AD_SPI_READ  0x1
#define AD_SPI_WRITE 0x0
#define AD_SPI_WIDTH_1 0x0
#define AD_SPI_WIDTH_2 0x1
#define AD_SPI_WIDTH_3 0x2
#define AD_SPI_STREAM  0x3


#define AD_VCO_CAL_FINISHED 0x6
#define AD_DIGITAL_LOCK     0x0

#define AD9559_SOFT_IO_UPDATE           0x0005
#define AD9559_IRQ_CLEARING                 0x0a05
#define AD9559_PLL0_LOCK_STATUS     0x0d20
#define AD9559_PLL0_LOOP_STATE        0x0d21
#define AD9559_PLL1_LOCK_STATUS     0x0d40
#define AD9559_PLL1_LOOP_STATE        0x0d41

typedef enum
{
    E_AD9559_CLOCK_TYPE_0 = 0,        
    E_AD9559_CLOCK_TYPE_1 ,          
    E_AD9559_CLOCK_TYPE_2 ,          
    E_AD9559_CLOCK_TYPE_3 ,           
} ad9559_clock_type_e;

#define AD9559_NUM   0x1

#define AD9559_SPI_DO_BIT       0x4000                  /*PD17*/
#define AD9559_SPI_DI_BIT       0x8000                  /*PD16*/
#define AD9559_SPI_CLK_BIT      0x2000                  /*PD18*/
#define AD9559_SPI_CS_BIT       0x40                    /*PD25*/

int32 clock_ad9559_register_driver(clock_handle_t* hdl, clock_info_t* clock_info);

#endif 


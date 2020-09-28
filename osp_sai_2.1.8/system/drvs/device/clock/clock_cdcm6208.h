/****************************************************************************
* $Id$
*  The header file of the cdcm6208 operation.
*
* (C) Copyright Actus Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chani
* Date          : 2013-09-30 
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_CDCM6208__
#define __CLOCK_CDCM6208__

#include "ctc_spi.h"

#define CDCM6208_DATA_LENTH 2    /* data length (byte)*/  
#define CDCM6208_ADDRESS_LENTH 2  /* address length (byte)*/ 

int32 clock_cdcm6208_register_driver(clock_handle_t* hdl, clock_info_t* clock_info);

#endif 


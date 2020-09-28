/****************************************************************************
* $Id$
*  The header file of the pd69200 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb
* Date          : 2019-05-27 
* Reason        : First Create.
****************************************************************************/
#ifndef __POE_PD69200_H__
#define __POE_PD69200_H__

#include "poe_drv.h"

#define POE_PD69200_OFFSET_WIDTH 15
#define POE_PD69200_PKG_SIZE 15 /* fixed size, can not be change */

int32 poe_pd69200_register_driver(poe_handle_t* hdl, poe_info_t* info);

#endif 


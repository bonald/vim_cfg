/****************************************************************************
* $Id$
*  The header file of the max20743 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : guhg
* Date          : 2015-12-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __VCM_MAX20743_DRV__
#define __VCM_MAX20743_DRV__

#include "vcm_drv.h"

#define VCM_MAX20743_VALUE_LEN      2

#define VCM_MAX20743_VIN        0x88        /*input voltage*/
#define VCM_MAX20743_VOUT       0x8B        /*notice:output voltage is not equ pmbus define*/
#define VCM_MAX20743_IOUT       0x8c        /*output current*/
#define VCM_MAX20743_TMPR1      0x8d        /*temperature1*/
#define VCM_MAX20743_SSEL       0xd3        /*bit <1-0>*/
#define VCM_MAX20743_VBOOT      0xd2        /*bit <9-8>*/


int32 vcm_max20743_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info);

#endif 


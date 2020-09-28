/****************************************************************************
* $Id$
*  The header file of the bmr operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : guhg
* Date          : 2015-12-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __VCM_BMR464_DRV__
#define __VCM_BMR464_DRV__

#include "vcm_drv.h"

#define VCM_BMR_464_VALUE_LEN  2

#define VCM_BMR_464_VIN          0x88        /*input voltage*/
#define VCM_BMR_464_IIN           0x89        /*input current*/
#define VCM_BMR_464_VOUT       0x21        /*notice:output voltage is not equ pmbus define*/
#define VCM_BMR_464_IOUT        0x8c        /*output current*/
#define VCM_BMR_464_TMPR1     0x8d        /*temperature1*/
#define VCM_BMR_464_TMPR2     0x8e        /*temperature2*/
#define VCM_BMR_464_TMPR3     0x8f        /*temperature3*/

int32 vcm_bmr464_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info);

#endif 


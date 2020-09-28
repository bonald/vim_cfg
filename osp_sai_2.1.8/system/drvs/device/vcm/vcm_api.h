/****************************************************************************
* $Id$
*  The header file of the vcm(voltage and current monitor) api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb 
* Date          : 2017-07-24 
* Reason        : First Create.
****************************************************************************/
#ifndef __VCM_API_H__
#define __VCM_API_H__

#include "vcm_drv.h"
#include "glb_hw_define.h"

int32 vcm_dev_get_volt(uint32 vcm_id, vcm_access_t type, float32* voltage);
int32 vcm_dev_get_cur(uint32 vcm_id, vcm_access_t type, float32* current);
int32 vcm_dev_get_temp(uint32 vcm_id, vcm_access_t type, float32* tmpr);

int32 vcm_reg_read(uint32 vcm_id, uint8 reg, uint8* value, uint8 len);
int32 vcm_reg_write(uint32 vcm_id, uint8 reg, uint8* value, uint8 len);
int32 vcm_init(vcm_info_t* vcm_info,  uint32 num);
int32 vcm_dev_init(uint32 vcm_id);
int32 vcm_get_stauts(uint32 vcm_id, vcm_cfg_t* cfg,vcm_status_t* status);

#endif 


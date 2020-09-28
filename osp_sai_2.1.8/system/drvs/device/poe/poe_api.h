/****************************************************************************
* $Id$
*  The header file of the poe(Power of Ethernet) api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb 
* Date          : 2019-05-27 
* Reason        : First Create.
****************************************************************************/
#ifndef __POE_API_H__
#define __POE_API_H__

#include "poe_drv.h"
#include "poe_pd69200.h"
#include "glb_hw_define.h"

int32
poe_reg_read(uint32 id, uint8* value);
int32
poe_reg_write(uint32 id, uint8* value);
int32
poe_get_status(uint32 id,poe_module_info_t * poe_module);
int32
poe_power_status_update(uint32 id);
int32
poe_glb_status_update(uint32 id);
int32
poe_status_update(uint32 id,uint16 channel,int8 attribute);
int32
poe_set_glb_power(uint32 id,  uint32 value);
int32
poe_set_glb_enable(uint32 id,  uint8 enable);
int32
poe_set_enable(uint32 id,  uint16 channel,uint8 value);
int32
poe_set_force_power(uint32 id,  uint16 channel,uint8 value);
int32
poe_set_mode(uint32 id,  uint16 channel,uint8 value);
int32
poe_set_priority(uint32 id,  uint16 channel,uint8 value);
int32
poe_set_power_limit(uint32 id,  uint16 channel,uint32 value);


int32
poe_init(poe_info_t* info, uint32 num);
int32 
poe_dev_init(uint32 id);

#endif 


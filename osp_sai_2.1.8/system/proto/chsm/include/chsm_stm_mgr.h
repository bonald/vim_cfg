/****************************************************************************
 *  stm_alloc.h :  Macro, data structure for allocating tcam/sram table
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   ychen
 * Date     :   2010-11-08
 ****************************************************************************/
#ifndef __CHSM_STM_MGR_H__
#define __CHSM_STM_MGR_H__

#include "glb_stm_define.h"
#include "gen/tbl_sys_spec_define.h"

int32
_chsm_stm_mgr_cmd_map_sys_spec_type(char **argv, int32 argc, cfg_cmd_para_t *para, uint32 *type);

int32 
chsm_stm_mgr_write_profile_type(uint8 type);
int32
chsm_stm_mgr_read_profile_mode(uint8* p_mode);

int32
chsm_stm_mgr_init(void);

int32
chsm_stm_mgr_cmd_process_sys_spec(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /*__CHSM_STM_MGR_H__*/

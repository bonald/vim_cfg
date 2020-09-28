/****************************************************************************
 *file sys_due2_ofb.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2017-8-15

 ****************************************************************************/
#ifndef _SYS_USW_OFB_H
#define _SYS_USW_OFB_H

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Header Files
*
***************************************************************/
#include "sal.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

struct sys_usw_ofb_param_s
{
    uint32 size;
    uint32 max_limit_offset;   /* alloced offset will not > the max_limit_offset */
    uint8 multiple;
    int32 (* ofb_cb)(uint8 lchip, uint32 new_offset, uint32 old_offset, void* user_data);
};
typedef struct sys_usw_ofb_param_s sys_usw_ofb_param_t;

typedef int32 (* OFB_TYAVERSE_FUN_P)  (uint8 lchip, void* data, void* user_data);
/****************************************************************************
*
* Function
*
*****************************************************************************/
extern int32
sys_usw_ofb_init(uint8 lchip, uint16 block_num, uint32 size, uint8* ofb_type);

extern int32
sys_usw_ofb_init_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, sys_usw_ofb_param_t* p_ofb_param );

extern int32
sys_usw_ofb_alloc_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32* p_offset, void* user_data);

extern int32
sys_usw_ofb_alloc_offset_from_position(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32 p_offset, void* user_data);

extern int32
sys_usw_ofb_free_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32 offset);

extern int32
sys_usw_ofb_traverse(uint8 lchip, uint8 ofb_type, uint16 min_block_id, uint16 max_block_id, OFB_TYAVERSE_FUN_P fn, void* data);

extern int32
sys_usw_ofb_deinit(uint8 lchip, uint8 ofb_type);

extern int32
sys_usw_ofb_show_status(uint8 lchip);

extern int32
sys_usw_ofb_show_offset(uint8 lchip, uint8 ofb_type);

extern int32
sys_usw_ofb_wb_sync(uint8 lchip, uint8 ofb_type, uint32  module_id, uint32 sub_id);

extern int32
sys_usw_ofb_wb_restore(uint8 lchip, uint8 ofb_type, uint32  module_id, uint32 sub_id);

extern int32
sys_usw_ofb_check_resource(uint8 lchip, uint8 ofb_type, uint16 block_id);

extern int32
sys_usw_ofb_dump_db(uint8 lchip, uint16 ofb_type, sal_file_t p_f);
#ifdef __cplusplus
}
#endif

#endif /* _SYS_USW_OFB_H_ */

#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_com.h

 @date 2010-1-19

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_COM_H
#define _SYS_USW_OAM_COM_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "sys_usw_oam_db.h"
/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
 *
* Function
*
****************************************************************************/
 int32
sys_usw_oam_key_write_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data);
int32
sys_usw_oam_key_delete_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data);
int32
sys_usw_oam_key_read_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data);
int32
sys_usw_oam_key_lookup_io(uint8 lchip, uint32 tbl_id, uint32 *index, void *p_data);
int32
sys_usw_oam_key_lookup_io_by_key(uint8 lchip, uint32 tbl_id, uint32 *index, void *p_data);

int32
_sys_usw_oam_com_add_maid(uint8 lchip, void* p_maid);

int32
_sys_usw_oam_com_remove_maid(uint8 lchip, void* p_maid);


int32
_sys_usw_tp_add_chan(uint8 lchip, void* p_chan, void** p_sys_chan);

int32
_sys_usw_tp_remove_chan(uint8 lchip, void* p_chan);

sys_oam_chan_tp_t*
_sys_usw_tp_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm);

int32
_sys_usw_tp_chan_build_index(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp);

int32
_sys_usw_tp_chan_add_to_asic(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp);




#ifdef __cplusplus
}
#endif

#endif

#endif

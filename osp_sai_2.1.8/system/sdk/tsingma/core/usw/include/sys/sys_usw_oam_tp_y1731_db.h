#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_tp_y1731_db.h

 @date 2012-12-06

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_TP_Y1731_DB_H
#define _SYS_USW_OAM_TP_Y1731_DB_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/

#include "ctc_oam.h"

#include "sys_usw_oam.h"
#include "sys_usw_oam_com.h"
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
#define MAID_H ""



#define CHAN_H ""

int32
_sys_usw_tp_chan_update(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp_y1731, bool is_add);

int32
_sys_usw_tp_chan_update_reomte(uint8 lchip, ctc_oam_lmep_t* p_lmep, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add);

int32
_sys_usw_tp_chan_update_mip(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add);

#define LMEP_H ""

sys_oam_lmep_t*
_sys_usw_tp_y1731_lmep_lkup(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp);


sys_oam_lmep_t*
_sys_usw_tp_y1731_lmep_build_node(uint8 lchip, ctc_oam_lmep_t* p_lmep_param);

int32
_sys_usw_tp_y1731_lmep_free_node(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_add_to_asic(uint8 lchip, ctc_oam_lmep_t* p_lmep_param,
                                         sys_oam_lmep_t* p_sys_lmep_tp);
int32
_sys_usw_tp_y1731_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_lmep_update_asic(uint8 lchip, ctc_oam_update_t* p_lmep_param,
                                         sys_oam_lmep_t* p_sys_lmep_tp);

int32
_sys_usw_tp_y1731_update_lmep_lm(uint8 lchip, ctc_oam_update_t* p_lmep_param, sys_oam_lmep_t* p_sys_lmep_tp_y1731);

int32
_sys_usw_tp_y1731_update_master_gchip(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_tp_y1731_update_label(uint8 lchip, ctc_oam_update_t* p_lmep_param);
#define RMEP_H ""

sys_oam_rmep_t*
_sys_usw_tp_y1731_rmep_lkup(uint8 lchip, uint32 rmep_id, sys_oam_lmep_t* p_sys_lmep_tp_y1731);

sys_oam_rmep_t*
_sys_usw_tp_y1731_rmep_build_node(uint8 lchip, ctc_oam_rmep_t* p_rmep_param);

int32
_sys_usw_tp_y1731_rmep_free_node(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_add_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                         sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731);

int32
_sys_usw_tp_y1731_rmep_update_asic(uint8 lchip, ctc_oam_update_t* p_rmep_param,
                                         sys_oam_rmep_t* p_sys_rmep_tp_y1731);

#define COMMON_SET_H ""

#define COMMON_GET_H ""

int32
_sys_usw_tp_y1731_register_init(uint8 lchip, ctc_oam_global_t* p_tp_y1731_glb);

#ifdef __cplusplus
}
#endif

#endif

#endif

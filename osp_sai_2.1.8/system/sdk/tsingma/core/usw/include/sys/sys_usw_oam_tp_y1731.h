#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_tp_y1731.h

 @date 2012-12-06

 @version v2.0

  The file defines Macro, stored data structure for  MPLS-TP Y.1731 OAM module
*/
#ifndef _SYS_USW_OAM_TP_Y1731_H
#define _SYS_USW_OAM_TP_Y1731_H
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
sys_usw_oam_tp_y1731_init(uint8 lchip, uint8 oam_type, ctc_oam_global_t* p_tp_y1731_glb);

#ifdef __cplusplus
}
#endif

#endif

#endif

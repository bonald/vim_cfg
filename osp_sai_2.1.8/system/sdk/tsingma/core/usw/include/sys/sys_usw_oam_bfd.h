#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_bfd.h

 @date 2013-01-16

 @version v2.0

  The file defines Macro, stored data structure for BFD OAM module
*/
#ifndef _SYS_USW_OAM_BFD_H
#define _SYS_USW_OAM_BFD_H
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
sys_usw_oam_bfd_init(uint8 lchip, uint8 oam_type, ctc_oam_global_t* p_oam_glb);


#ifdef __cplusplus
}
#endif

#endif

#endif

#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_cfm.h

 @date 2010-1-19

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_CFM_H
#define _SYS_USW_OAM_CFM_H
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
sys_usw_cfm_init(uint8 lchip, uint8 oam_type, ctc_oam_global_t* p_eth_glb);

int32
_sys_usw_cfm_get_property(uint8 lchip, void* p_prop);

#ifdef __cplusplus
}
#endif

#endif

#endif


/**
 @file sys_duet2_mcu.h

 @date 2018-09-12

 @version v1.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_DUET2_MCU_H
#define _SYS_DUET2_MCU_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"

/***************************************************************
 *
 * Macro definition
 *
 ***************************************************************/

/***************************************************************
 *
 * Structure definition
 *
 ***************************************************************/

/***************************************************************
 *
 * Function declaration
 *
 ***************************************************************/
int32 sys_duet2_mcu_show_debug_info(uint8 lchip);


#ifdef __cplusplus
}
#endif

#endif


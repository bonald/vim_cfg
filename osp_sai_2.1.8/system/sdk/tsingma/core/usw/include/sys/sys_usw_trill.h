#if (FEATURE_MODE == 0)
/**
 @file sys_usw_trill.h

 @date 2015-10-25

 @version v3.0

*/

#ifndef _SYS_USW_TRILL_H
#define _SYS_USW_TRILL_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_trill.h"
#include "ctc_debug.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/


/****************************************************************************
*
* Function
*
*****************************************************************************/

extern int32
sys_usw_trill_init(uint8 lchip, void* trill_global_cfg);

extern int32
sys_usw_trill_deinit(uint8 lchip);

extern int32
sys_usw_trill_add_route(uint8 lchip, ctc_trill_route_t* p_trill_route);

extern int32
sys_usw_trill_remove_route(uint8 lchip, ctc_trill_route_t* p_trill_route);

extern int32
sys_usw_trill_add_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);

extern int32
sys_usw_trill_remove_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);

extern int32
sys_usw_trill_update_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel);

#ifdef __cplusplus
}
#endif

#endif

#endif


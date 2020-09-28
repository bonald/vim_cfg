#if (FEATURE_MODE == 0)
/**
 @file sys_usw_efd.h

 @date 2014-10-28

 @version v3.0

Macro, data structure for system common operations

*/

#ifndef _SYS_USW_EFD
#define _SYS_USW_EFD
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_efd.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_efd_set_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value);

extern int32
sys_usw_efd_get_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value);

extern int32
sys_usw_efd_register_cb(uint8 lchip, ctc_efd_fn_t callback, void* userdata);

extern void
sys_usw_efd_sync_data(uint8 lchip, ctc_efd_data_t* info);

extern int32
sys_usw_efd_init(uint8 lchip);
extern int32
sys_usw_efd_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif


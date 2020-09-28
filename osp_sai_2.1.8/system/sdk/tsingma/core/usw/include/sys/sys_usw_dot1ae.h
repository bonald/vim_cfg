#if (FEATURE_MODE == 0)
/**
 @file sys_usw_dot1ae.h

 @date 2010-2-26

 @version v2.0

---file comments----
*/

#ifndef SYS_USW_DOT1AE_H_
#define SYS_USW_DOT1AE_H_
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
#include "ctc_dot1ae.h"
#include "ctc_l2.h"
#include "sys_usw_register.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/


#define SYS_DOT1AE_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(dot1ae, dot1ae, DOT1AE_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_dot1ae_init(uint8 lchip);
extern int32
sys_usw_dot1ae_deinit(uint8 lchip);
extern int32
sys_usw_dot1ae_add_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_chan);

extern int32
sys_usw_dot1ae_remove_sec_chan(uint8 lchip,ctc_dot1ae_sc_t* p_chan);

extern int32
sys_usw_dot1ae_update_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_chan);

extern int32
sys_usw_dot1ae_get_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_chan);

extern int32
sys_usw_dot1ae_set_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg);

extern int32
sys_usw_dot1ae_get_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg);

extern int32
sys_usw_dot1ae_bind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);

extern int32
sys_usw_dot1ae_unbind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);
extern int32
sys_usw_dot1ae_get_bind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);

extern int32
sys_usw_dot1ae_get_stats(uint8 lchip, uint32 chan_id, ctc_dot1ae_stats_t* p_stats);

extern int32
sys_usw_dot1ae_clear_stats(uint8 lchip, uint32 chan_id, uint8 an);

#ifdef __cplusplus
}
#endif

#endif

#endif


#if (FEATURE_MODE == 0)
/**
 @file sys_usw_bpe.h

 @date 2013-05-28

 @version v2.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_usw_BPE_H
#define _SYS_usw_BPE_H
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
#include "ctc_vector.h"

#include "ctc_bpe.h"




extern int32
sys_usw_bpe_init(uint8 lchip, void* bpe_global_cfg);
extern int32
sys_usw_bpe_set_port_extender(uint8 lchip, uint32 gport, ctc_bpe_extender_t* p_extender);
extern int32
sys_usw_bpe_get_port_extender(uint8 lchip, uint32 gport, bool* enable);
extern int32
sys_usw_bpe_get_port_extend_mcast_en(uint8 lchip, uint8 *p_enable);
extern int32
sys_usw_bpe_set_port_extend_mcast_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_bpe_add_8021br_forward(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd);
extern int32
sys_usw_bpe_remove_8021br_forward(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd);

extern int32
sys_usw_bpe_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif


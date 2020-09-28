#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_ipuc_tcam.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_IPUC_TCAM_H
#define _SYS_USW_IPUC_TCAM_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "drv_api.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_hash.h"

#include "ctc_ipuc.h"
#include "ctc_spool.h"
#include "sys_usw_ipuc.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
#define INVALID_DRV_NEXTHOP_OFFSET      0x3FFFF
#define INVALID_DRV_POINTER_OFFSET      0x7FFFF

enum sys_tcam_share_mode_e
{
    SYS_SHARE_NONE,
    SYS_SHARE_IPV6,
    SYS_SHARE_ALL,
    MAX_SHARE_TYPE
};
typedef enum sys_tcam_share_mode_e sys_tcam_share_mode_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

extern int32
sys_duet2_ipuc_tcam_write_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_write_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_get_blockid(uint8 lchip, sys_ipuc_tcam_data_t *p_data, uint8 *block_id);

extern int32
sys_duet2_ipuc_tcam_write_calpm_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_write_calpm_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_move(uint8 lchip, uint32 new_index, uint32 old_index, void *pdata);

extern int32
sys_duet2_ipuc_tcam_update_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_show_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_duet2_ipuc_tcam_show_status(uint8 lchip);

int32
sys_duet2_ipuc_tcam_init_public(uint8 lchip, void* ofb_cb_fn);

extern int32
sys_duet2_ipuc_tcam_init(uint8 lchip, sys_ipuc_route_mode_t route_mode, void* ofb_cb_fn);

extern int32
sys_duet2_ipuc_tcam_deinit(uint8 lchip, sys_ipuc_route_mode_t route_mode);

#ifdef __cplusplus
}
#endif

#endif

#endif


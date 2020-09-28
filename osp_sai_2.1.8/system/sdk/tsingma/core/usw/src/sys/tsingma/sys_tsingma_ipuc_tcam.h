/**
 @file sys_tsinama_ipuc_tcam.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_TSINAMA_IPUC_TCAM_H
#define _SYS_TSINAMA_IPUC_TCAM_H
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
#define INVALID_POINTER_OFFSET      0xFFFF
#define INVALID_NEXTHOP_OFFSET      0xFFFF

struct sys_ipuc_tcam_key_s
{
    uint32 key_idx;
    uint8  mask_len;
    union
    {
        ip_addr_t ipv4;
        ipv6_addr_t ipv6;
    } ip;
    uint32 vrfId;
    uint32 vrfId_mask;

};
typedef struct sys_ipuc_tcam_key_s sys_ipuc_tcam_key_t;

struct sys_ipuc_tcam_ad_s
{
    uint32 tcamAd_key_idx;

    uint8 indexMask;
    uint8 isHost;
    uint8 lpmPipelineValid;
    uint8 lpmStartByte;
    uint32 nexthop;
    uint32 pointer;
};
typedef struct sys_ipuc_tcam_ad_s sys_ipuc_tcam_ad_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/


extern int32
sys_tsingma_ipuc_tcam_get_blockid(uint8 lchip, sys_ipuc_tcam_data_t *p_data, uint8 *p_block_id);

extern int32
sys_tsingma_ipuc_tcam_get_blockid_u(uint8 lchip,uint8 ip_ver,uint8 masklen,sys_ipuc_route_mode_t route_mode,uint8 hash_conflict,uint16* block_id);

extern int32
sys_tsingma_ipuc_tcam_move(uint8 lchip, uint32 new_index, uint32 old_index, void *pdata);

extern int32
sys_tsingma_ipuc_tcam_update_stats(uint8 lchip, uint8 ip_ver, sys_ipuc_route_mode_t route_mode, uint32 new_index, uint32 old_index);

extern int32
sys_tsingma_ipuc_tcam_move_u(uint8 lchip, uint8 ip_ver, sys_ipuc_route_mode_t route_mode, uint32 new_index, uint32 old_index);

extern int32
sys_tsingma_ipuc_tcam_write_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_tsingma_ipuc_tcam_write_key2hw(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_key_t* p_key, sys_ipuc_opt_type_t opt);

extern int32
sys_tsingma_ipuc_tcam_write_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_tsingma_ipuc_tcam_read_ad(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_ad_t* p_ad);

extern int32
sys_tsingma_ipuc_tcam_write_ad2hw(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_ad_t* p_ad, sys_ipuc_opt_type_t opt);

extern int32
sys_tsingma_ipuc_tcam_show_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_tsingma_ipuc_tcam_show_status(uint8 lchip);

extern int32
sys_tsingma_ipuc_tcam_init(uint8 lchip, sys_ipuc_route_mode_t route_mode, void* ofb_cb_fn);

extern int32
sys_tsingma_ipuc_tcam_deinit(uint8 lchip, sys_ipuc_route_mode_t route_mode);

extern int32
sys_tsingma_ipuc_tcam_read_key(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_key_t * p_key);

extern int32
_sys_tsingma_nalpm_get_real_tcam_index(uint8 lchip, uint16 soft_tcam_index, uint8 * real_lchip, uint16 * real_tcam_index);


#ifdef __cplusplus
}
#endif

#endif


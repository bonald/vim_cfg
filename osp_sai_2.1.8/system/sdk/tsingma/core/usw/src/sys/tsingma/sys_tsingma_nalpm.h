/**
 @file sys_tsingma_nalpm.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_TSINGMA_NALPM_H
#define _SYS_TSINGMA_NALPM_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "sal_types.h"
#include "sal.h"
#include "ctc_const.h"
#include "ctc_ipuc.h"
#include "sys_tsingma_trie.h"
#include "ctc_register.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define SYS_NALPM_V4_MASK_LEN_MAX 32/* 0-32 */
#define SYS_NALPM_V6_MASK_LEN_MAX 128/* 0-32 */

#define SYS_NALPM_PREFIX_LEN_INVALID (-1)/* -1 */
#define SYS_NALPM_PREFIX_LEN_MIN 0/* 0*/
#define SYS_NALPM_PREFIX_LEN_MAX 98/* ipv4:0-32; ipv6:0-64, MAX*/
#define SYS_NALPM_PREFIX_LEN_V6_BASE 33/* ipv6-64 base:33*/
#define SYS_NALPM_PREFIX_LEN_128_MAX 129/* 0-128, MAX*/

#define SYS_NALPM_VRFID_MAX 256/* max vrf */
#define SYS_NALPM_VRFID_DEF 0/* default vrf */

#define _MAX_KEY_LEN_32_  (32)
#define _MAX_KEY_LEN_128_ (128)
#define INVALID_DRV_NEXTHOP_OFFSET      0x3FFFF
#define INVALID_NEXTHOP_OFFSET      0xFFFF

#define SYS_NALPM_MAX_SNAKE 10
#define ROUTE_NUM_PER_SNAKE_V4 6
#define ROUTE_NUM_PER_SNAKE_V6_64 3
#define ROUTE_NUM_PER_SNAKE_V6_128 2

#define MERGE_OK 0
#define MERGE_NO_RESOURCE 1
/*
pfx is ipv4_value;
pfx_len is valid bits number
*/
#define NALPM_PFX2BITS(pfx, pfx_len, bit_str) \
{\
    int32 i = 0;\
    if (0 == pfx_len)\
    {\
        bit_str[0] = '0';\
    }\
    while (i < pfx_len)\
    {\
        if (pfx & (1 << i))\
        {\
            bit_str[pfx_len - 1 - i] = '1';\
        }\
        else\
        {\
            bit_str[pfx_len - 1 - i] = '0';\
        }\
        i++;\
    }\
}

#define ROUTE_NUM_IN_SRAM_ENTRY(sram_type, num) \
    if((sram_type) == SYS_NALPM_SRAM_ENTRY_TYPE_V4_32)\
    {\
        (num) = 12;\
    }\
    else if((sram_type) == SYS_NALPM_SRAM_ENTRY_TYPE_V6_64)\
    {\
        (num) = 6;\
    }\
    else\
    {\
        (num) = 4;\
    }

#define SYS_IP_V6_SORT(val)           \
{                                          \
    if (CTC_IP_VER_6 == (val)->ip_ver)      \
    {                                      \
        uint32 t;                          \
        t = val->ip[0];               \
        val->ip[0] = val->ip[3]; \
        val->ip[3] = t;               \
                                       \
        t = val->ip[1];               \
        val->ip[1] = val->ip[2]; \
        val->ip[2] = t;               \
    }                                      \
}

#define ROUTE_INFO_SIZE(ver) \
((ver == CTC_IP_VER_4) ? (sizeof(sys_nalpm_route_info_t) + sizeof(ip_addr_t)) : (sizeof(sys_nalpm_route_info_t) + sizeof(ipv6_addr_t)))

#define IP_ADDR_SIZE(ver) \
((ver == CTC_IP_VER_4) ? (sizeof(ip_addr_t)) : (sizeof(ipv6_addr_t)))

#define SYS_NALPM_INIT_CHECK \
    if(!g_sys_nalpm_master[lchip])\
    {\
        return CTC_E_NOT_INIT;\
    }\

#define SRAM_HW_INDEX(lchip, sram_idx, snake)  DRV_IS_TSINGMA(lchip)? sram_idx : (sram_idx + snake * 16384)

struct sys_nalpm_route_info_s
{
    uint32 ad_idx;
    uint16 vrf_id;
    uint8  route_masklen;
    uint8  tcam_masklen;
    uint8  ip_ver;
    uint32 ip[0]; /*根据ip_ver 在malloc 时大小分别为 sizeof(uint32) *1 或 *4 */
};
typedef struct sys_nalpm_route_info_s sys_nalpm_route_info_t;

enum sys_nalpm_sram_entry_type_e {
    SYS_NALPM_SRAM_TYPE_VOID,
    SYS_NALPM_SRAM_TYPE_V4_32, /* 56bits, 6 * 2*/
    SYS_NALPM_SRAM_TYPE_V6_64,/* 90bits, 3 * 2*/
    SYS_NALPM_SRAM_TYPE_V6_128,/* 154bits, 2 * 2*/
};
typedef enum sys_nalpm_sram_entry_type_e sys_nalpm_sram_entry_type_t;

struct sys_nalpm_tcam_item_s {
    uint16 tcam_idx;
    uint16 sram_idx;
    uint8 sram_type[SYS_NALPM_MAX_SNAKE];                 /*indicate entry is v4_32, v6_64, or v6_128*/
    trie_t *trie;                       /* routes share same prefix */
    sys_nalpm_route_info_t* p_prefix_info;  /*prefix info*/
    sys_nalpm_route_info_t* p_AD_route;
    sys_nalpm_route_info_t* p_ipuc_info_array[SYS_NALPM_MAX_SNAKE][ROUTE_NUM_PER_SNAKE_V4];
};
typedef struct sys_nalpm_tcam_item_s sys_nalpm_tcam_item_t;

struct sys_nalpm_master_s {
    uint32 split_mode;
    trie_t **prefix_trie[MAX_CTC_IP_VER];
    uint8 opf_type_nalpm;
    uint32 len2pfx[SYS_NALPM_V4_MASK_LEN_MAX+1]; /*masklen 0-32 */
    uint32 *vrf_route_cnt[MAX_CTC_IP_VER];
    uint8 ipsa_enable;
    uint8 frag_arrange_enable; /* will arrange fragment when delete routes */
    uint8 frag_arrange_status[MAX_CTC_IP_VER];
};
typedef struct sys_nalpm_master_s sys_nalpm_master_t;

struct sys_nalpm_trie_payload_s {
    trie_node_t node;                 /*trie node: used to store payload on trie */
    sys_nalpm_route_info_t* p_route_info;
};
typedef struct sys_nalpm_trie_payload_s sys_nalpm_trie_payload_t;

struct sys_nalpm_prefix_trie_payload_s {
    trie_node_t node;
    sys_nalpm_tcam_item_t tcam_item;
};
typedef struct sys_nalpm_prefix_trie_payload_s sys_nalpm_prefix_trie_payload_t;

struct sys_nalpm_route_store_info_s {
    uint8 tcam_hit;
    uint8 sram_hit;
    uint8 snake_idx;
    uint8 entry_offset;  /*route idx in one snake*/
    sys_nalpm_tcam_item_t* p_tcam_item;
};
typedef struct sys_nalpm_route_store_info_s sys_nalpm_route_store_info_t;

struct sys_nalpm_fragment_s {
    uint8 snake_idx;
    uint8 offset;
};
typedef struct sys_nalpm_fragment_s sys_nalpm_fragment_t;

extern int32
sys_tsingma_nalpm_init(uint8 lchip, uint8 para1, uint8 para2);

extern int32
sys_tsingma_nalpm_deinit(uint8 lchip);

extern int32
sys_tsingma_nalpm_add(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index, void* data);

extern int32
sys_tsingma_nalpm_del(uint8 lchip, sys_ipuc_param_t* p_ipuc_param);

extern int32
sys_tsingma_nalpm_update(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index);

extern int32
sys_tsingma_nalpm_dump_pfx_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver);

extern int32
sys_tsingma_nalpm_dump_route_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver, uint16 tcam_idx);

extern int32
sys_tsingma_nalpm_show_route_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_tsingma_nalpm_wb_get_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, void* p_alpm_info);

extern int32
sys_tsingma_nalpm_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param);

extern int32
sys_tsingma_nalpm_merge(uint8 lchip, uint32 vrf_id, uint8 ip_ver);

extern int32
sys_tsingma_nalpm_set_fragment_status(uint8 lchip, uint8 ip_ver, uint8 status);

extern int32
sys_tsingma_nalpm_get_fragment_status(uint8 lchip, uint8 ip_ver, uint8* status);

extern int32
sys_tsingma_nalpm_get_fragment_auto_enable(uint8 lchip, uint8* enable);

int32
_sys_tsingma_nalpm_sram_idx_alloc(uint8 lchip, uint8  ip_ver, uint16* p_sram_idx);

int32
_sys_tsingma_nalpm_sram_idx_free(uint8 lchip, uint16 sram_idx);

void
_sys_tsingma_nalpm_get_snake_num(uint8 lchip, uint8 ip_ver, uint8 * p_snake_num);

int32
_sys_tsingma_nalpm_write_route_trie(uint8 lchip, sys_nalpm_tcam_item_t * p_tcam_item, uint8 new_tcam);

int32
_sys_tsingma_nalpm_move_route(uint8 lchip, uint8 new_lchip, uint8 old_lchip, sys_nalpm_route_info_t* p_route_info);

int32
_sys_tsingma_nalpm_get_real_tcam_index(uint8 lchip, uint16 soft_tcam_index, uint8 * real_lchip, uint16 * real_tcam_index);

int32
_sys_tsingma_nalpm_renew_dsipda(uint8 lchip, void * dsipda, sys_nh_info_dsnh_t nhinfo);

int32
_sys_tsingma_nalpm_read_tcam(uint8 lchip, sys_nalpm_tcam_item_t * p_tcam_item, uint32 * p_ad_idx);

int32
_sys_nalpm_clear_sram(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, uint8 snake_idx, uint8 offset, uint8 clear_snake_type);

#ifdef __cplusplus
}
#endif

#endif


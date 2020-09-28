#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_duet2_calpm.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_CALPM_H
#define _SYS_USW_CALPM_H
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
 *  Defines and Macros
 *
 ***************************************************************/
#define CALPM_ITEM_NUM                        512
#define CALPM_MASK_LEN                        8
#define CALPM_TBL_NUM                         256
#define CALPM_SRAM_RSV_FOR_ARRANGE            256
#define CALPM_ITEM_ALLOC_NUM                  16
#define CALPM_TABLE_ALLOC_NUM                 8
#define CALPM_TABLE_BLOCK_NUM                 32      /*CALPM_TBL_NUM/CALPM_TABLE_ALLOC_NUM*/
#define CALPM_OFFSET_BITS_NUM                  9        /*masklen 0~8*/
#define CALPM_COMPRESSED_BLOCK_SIZE           2

#define CALPM_PREFIX_IPV4_HASH_SIZE                 8192
#define CALPM_PREFIX_IPV6_HASH_SIZE                 2048
#define CALPM_PREFIX_HASH_BLOCK_SIZE                 128

#define SYS_CALPM_PREFIX_TCAM_TYPE_MASK  0x80
#define SYS_CALPM_PREFIX_INVALID_INDEX   0xFFFF

#define DS_CALPM_PREFIX_MAX_INDEX   512

#define INVALID_NEXTHOP_OFFSET      0xFFFF
#define INVALID_POINTER_OFFSET      0xFFFF

#define INVALID_HASH_INDEX_MASK   0xFFFFFFFF

#define POINTER_OFFSET_MASK       ((1 << POINTER_OFFSET_BITS_LEN) - 1)
#define POINTER_STAGE_MASK        (((1 << POINTER_BITS_LEN) - 1) >> POINTER_OFFSET_BITS_LEN)

enum sys_ip_octo_e
{
    SYS_IP_OCTO_0_7,            /*  0   */
    SYS_IP_OCTO_8_15,           /*  1   */
    SYS_IP_OCTO_16_23,          /*  2   */
    SYS_IP_OCTO_24_31,          /*  3   */
    SYS_IP_OCTO_32_39,          /*  4   */
    SYS_IP_OCTO_40_47,          /*  5   */
    SYS_IP_OCTO_48_55,          /*  6   */
    SYS_IP_OCTO_56_63,          /*  7   */
    SYS_IP_OCTO_64_71,          /*  8   */
    SYS_IP_OCTO_72_79,          /*  9   */
    SYS_IP_OCTO_80_87,          /*  10  */
    SYS_IP_OCTO_88_95,          /*  11  */
    SYS_IP_OCTO_96_103,         /*  12  */
    SYS_IP_OCTO_104_111,        /*  13  */
    SYS_IP_OCTO_112_119,        /*  14  */
    SYS_IP_OCTO_120_127         /*  15  */
};
typedef enum sys_ip_octo_e sys_ip_octo_t;

enum sys_calpm_offset_type_e
{
    SYS_CALPM_START_OFFSET,     /* get start offset of per pipeline */
    SYS_CALPM_ARRANGE_OFFSET,     /* get start offset of fragment arrange */
    SYS_CALPM_MAX_OFFSET_TYPE
};
typedef enum sys_calpm_offset_type_e sys_calpm_offset_type_t;

enum sys_calpm_key_type_e
{
    CALPM_TYPE_NEXTHOP,
    CALPM_TYPE_POINTER,
    CALPM_TYPE_NUM
};
typedef enum sys_calpm_key_type_e sys_calpm_key_type_t;

enum sys_calpm_lookup_key_type_e
{
    CALPM_LOOKUP_KEY_TYPE_EMPTY,
    CALPM_LOOKUP_KEY_TYPE_NEXTHOP,
    CALPM_LOOKUP_KEY_TYPE_POINTER
};
typedef enum sys_calpm_lookup_key_type_e sys_calpm_lookup_key_type_t;

enum calpm_sram_table_index_e
{
    CALPM_TABLE_INDEX0,
    CALPM_TABLE_INDEX1,
    CALPM_TABLE_MAX
};
typedef enum calpm_sram_table_index_e calpm_sram_table_index_t;

/* REAL  means the item is the route node, the real node can be invalid when it whole push down
 * VALID  means the item is in use, it can be nor real node
 * PARENT  means the item hash child node
 */
enum calpm_item_flag_e
{
    CALPM_ITEM_FLAG_REAL                  =0x1,
    CALPM_ITEM_FLAG_VALID                =0x1 << 1,
    CALPM_ITEM_FLAG_PARENT             =0x1 << 2
};

/*===========================item struct start===========================*/
struct sys_calpm_item_s
{
    uint16 ad_index;
    uint16 t_masklen;       /* indicate this item is pushed or not */

    uint16 item_flag;       /*calpm_item_flag_e*/
    uint16 item_idx;       /*tree node index, range 0~511*/

    struct sys_calpm_item_s* p_nitem;
};
typedef struct sys_calpm_item_s sys_calpm_item_t;
/*===========================item struct end===========================*/

/*===========================table struct start===========================*/
/* if modify this struct DO NOT FORGET to modify the following struct too */
struct sys_calpm_tbl_s
{
    uint16 count;
    uint16 stage_base;      /*used to store calpm key offset, base addr of the tree */
    uint8 idx_mask;        /* route calpm compress index mask */
    uint8 stage_index;   /*used to store calpm sram index*/
    uint8 rsv[2];

    uint32 item_bitmap[CALPM_ITEM_ALLOC_NUM];     /* tree node bitmap */
    sys_calpm_item_t* p_item[CALPM_ITEM_ALLOC_NUM];     /* tree node, most 512 node for a tree */
    struct sys_calpm_tbl_s** p_ntable[CALPM_TABLE_ALLOC_NUM];   /* next table */
};
typedef struct sys_calpm_tbl_s sys_calpm_tbl_t;
/*===========================table struct end===========================*/

struct sys_calpm_prefix_s
{
    sys_calpm_tbl_t* p_ntable;
    uint16 key_index[CALPM_TYPE_NUM];  /* use to store nexthop or pointer */

    uint16 vrf_id;
    uint8   ip_ver;
    uint8   prefix_len;

    uint8   masklen_pushed;             /* pushed masklen */
    uint8   is_mask_valid;              /* 0 means have push down, but no match key and do nothing */
    uint8   is_pushed;                  /* if new, need push */
    uint8   rsv;

    uint32  ip32[2];                        /* use to store ip prefix*/
};
typedef struct sys_calpm_prefix_s sys_calpm_prefix_t;

struct sys_calpm_select_counter_s
{
    uint8 block_base;
    uint8 entry_number;
};
typedef struct sys_calpm_select_counter_s sys_calpm_select_counter_t;

struct sys_calpm_stage_s
{
    uint8 ip;
    uint8 mask;
    uint8 type;
    uint8 rsv;
};
typedef struct sys_calpm_stage_s sys_calpm_stage_t;

struct sys_calpm_prefix_result_s
{
    uint8 free;         /* hash key not exist, and is free */
    uint8 valid;        /* hash key is exist */
    uint16 rsv;

    uint32 key_index;
};
typedef struct sys_calpm_prefix_result_s sys_calpm_prefix_result_t;

struct sys_calpm_master_s
{
    sal_mutex_t* mutex;
    ctc_hash_t *calpm_prefix[MAX_CTC_IP_VER];           /* store calpm_prefix of specific prefix*/
    uint32 calpm_stats[CALPM_TABLE_MAX][CALPM_OFFSET_BITS_NUM];
    uint32 calpm_usage[CALPM_TABLE_MAX];
    uint16 max_stage_size[CALPM_TABLE_MAX];
    uint16 start_stage_offset[CALPM_TABLE_MAX];
    uint8 prefix_len[MAX_CTC_IP_VER];    /* ipv4 prefix_len = calpm_prefix8 ? 8 : 16; ipv6 prefix_len = 48 */

    uint8 couple_mode;
    uint8 opf_type_calpm;
    uint8 dma_enable;
    uint8 ipsa_enable;
    uint8 version_en[MAX_CTC_IP_VER];
};
typedef struct sys_calpm_master_s sys_calpm_master_t;

struct sys_calpm_param_s
{
    sys_calpm_prefix_t* p_calpm_prefix;
    ctc_ipuc_param_t* p_ipuc_param;
    uint16 key_index;
    uint32 ad_index;
};
typedef struct sys_calpm_param_s sys_calpm_param_t;

struct sys_calpm_stats_s
{
    uint8 table_ct[CALPM_TABLE_MAX];
    uint16 item_ct[CALPM_TABLE_MAX];
    uint16 real_ct[CALPM_TABLE_MAX];
    uint16 valid_ct[CALPM_TABLE_MAX];
};
typedef struct sys_calpm_stats_s sys_calpm_stats_t;

struct sys_calpm_traverse_s
{
    uint32 value0;
    uint32 value1;
    uint32 value2;
    uint32 value3;
    void* data;
    void* data1;
    void* data2;
    void* data3;
    void* data4;
    void* fn;
    uint8 lchip;
};
typedef struct sys_calpm_traverse_s sys_calpm_traverse_t;

struct sys_calpm_tbl_arrange_s
{
    uint32 start_offset;

    uint8 idx_mask;
    uint8 moved;
    uint8 stage;
    uint8 rsv0;

    sys_calpm_param_t calpm_param;
};
typedef struct sys_calpm_tbl_arrange_s sys_calpm_tbl_arrange_t;

struct sys_calpm_arrange_info_s
{
    sys_calpm_tbl_arrange_t*           p_data;
    struct sys_calpm_arrange_info_s*  p_next_info;
};
typedef struct sys_calpm_arrange_info_s sys_calpm_arrange_info_t;

struct sys_calpm_prefix_node_s
{
    ctc_slistnode_t head;
    sys_calpm_prefix_t* p_calpm_prefix;
};
typedef struct sys_calpm_prefix_node_s sys_calpm_prefix_node_t;

#define MAX_CALPM_MASKLEN(lchip, ip_ver)      (sys_duet2_calpm_get_prefix_len(lchip, ip_ver) + 16)

#define MIN_CALPM_MASKLEN(lchip, ip_ver)      sys_duet2_calpm_get_prefix_len(lchip, ip_ver)

#define SYS_CALPM_ITEM_OFFSET(IDX, MSK_LEN)                                            \
    (((IDX)&GET_MASK(MSK_LEN)) >> (CALPM_MASK_LEN - MSK_LEN))

#define SYS_CALPM_GET_ITEM_IDX(IDX, MSK_LEN)                                            \
    (GET_BASE(MSK_LEN) + SYS_CALPM_ITEM_OFFSET(IDX, MSK_LEN))

#define SYS_CALPM_GET_PARENT_ITEM_IDX(IDX, MSK_LEN)                                            \
    (GET_BASE(MSK_LEN-1) + SYS_CALPM_ITEM_OFFSET(IDX, MSK_LEN)/2)

#define CALPM_IDX_MASK_TO_SIZE(I, S)     \
    {                                   \
        uint8 N = 0;                    \
        uint8 T = (I);                  \
        while (T)                        \
        {                               \
            T &= (T - 1);               \
            N++;                        \
        }                               \
        (S) = (1 << N);                 \
    }

#define CALPM_IDX_MASK_TO_BITNUM(I, N)   \
    {                                   \
        uint8 T = (I);                  \
        (N) = 0;                        \
        while (T)                        \
        {                               \
            T &= (T - 1);               \
            (N)++;                      \
        }                               \
    }

#define GET_MASK(ln)                        \
    (                                           \
        ((ln) == 0) ? 0x00 : ((ln) == 1) ? 0x80 :      \
        ((ln) == 2) ? 0xc0 : ((ln) == 3) ? 0xe0 :      \
        ((ln) == 4) ? 0xf0 : ((ln) == 5) ? 0xf8 :      \
        ((ln) == 6) ? 0xfc : ((ln) == 7) ? 0xfe : 0xff  \
    )

#define GET_MASK_MAX(ln)                    \
    (                                           \
        ((ln) == 0) ? 0x00 : ((ln) == 1) ? 0x01 :      \
        ((ln) == 2) ? 0x03 : ((ln) == 3) ? 0x07 :      \
        ((ln) == 4) ? 0x0f : ((ln) == 5) ? 0x1f :      \
        ((ln) == 6) ? 0x3f : ((ln) == 7) ? 0x7f : 0xff  \
    )

/* the base value of ip tree. Every masklen have a base. For masklen 8, it is at the leaf layer, the base is 0 */
#define GET_BASE(ln)                        \
    (                                           \
        ((ln) == 0) ? 510 : ((ln) == 1) ? 508 :        \
        ((ln) == 2) ? 504 : ((ln) == 3) ? 496 :        \
        ((ln) == 4) ? 480 : ((ln) == 5) ? 448 :        \
        ((ln) == 6) ? 384 : ((ln) == 7) ? 256 : 0       \
    )

#define GET_INDEX_MASK_OFFSET(ln)       \
    (                                       \
        ((ln) == 0) ? 0 : ((ln) == 1) ? 1 :        \
        ((ln) == 2) ? 9 : ((ln) == 3) ? 37 :       \
        ((ln) == 4) ? 93 : ((ln) == 5) ? 163 :     \
        ((ln) == 6) ? 219 : ((ln) == 7) ? 247 : 255 \
    )

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

extern int32
sys_duet2_calpm_add(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index, void* p_alpm_info);

extern int32
sys_duet2_calpm_del(uint8 lchip, sys_ipuc_param_t* p_ipuc_param);

extern int32
sys_duet2_calpm_update(uint8 lchip, sys_ipuc_param_t* p_ipuc_param, uint32 ad_index);

extern uint8
sys_duet2_calpm_get_prefix_len(uint8 lchip, uint8 ip_ver);

extern uint32
sys_duet2_calpm_get_extra_offset(uint8 lchip, uint8 sram_index, sys_calpm_offset_type_t type);

extern int32
sys_duet2_calpm_arrange_fragment(uint8 lchip, sys_ipuc_param_list_t *p_info_list);

extern int32
sys_duet2_calpm_show_calpm_key(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_duet2_calpm_db_show(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint32 detail);

extern int32
sys_duet2_calpm_show_status(uint8 lchip);

extern int32
sys_duet2_calpm_mapping_wb_master(uint8 lchip, uint8 sync);

extern int32
sys_duet2_calpm_get_wb_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, void* p_alpm_info);

extern int32
sys_duet2_calpm_init(uint8 lchip, uint8 calpm_prefix8, uint8 ipsa_enable);

extern int32
sys_duet2_calpm_deinit(uint8 lchip);


#ifdef __cplusplus
}
#endif

#endif

#endif


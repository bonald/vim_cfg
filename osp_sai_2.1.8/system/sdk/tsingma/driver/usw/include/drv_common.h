/**
 @file

 @date 2010/02/22

 @version v5.1

 This file contains those macro & enum definitions and global var
*/

#ifndef _DRV_COMMON_H_
#define _DRV_COMMON_H_

#include "drv_api.h"

#define cosim  0
#define DRV_MAX_ARRAY_NUM 64
#define DRV_NUM_4K                          (4*1024)
#define DRV_NUM_8K                          (8*1024)
#define DRV_NUM_16K                         (16 * 1024)
#define DRV_NUM_32K                         (32 * 1024)
#define DRV_NUM_64K                         (64 * 1024)
#define MAX_WORD_LEN  20
#define MAX_HASH_LEVEL 10

#define DRV_WB_STATUS_DONE          1
#define DRV_WB_STATUS_RELOADING     2
#define DRV_WB_STATUS_SYNC          3

#define DRV_INIT_CHECK(lchip) \
    do { \
        if (lchip >= DRV_MAX_CHIP_NUM || !p_drv_master[lchip]->init_done){ \
            return DRV_E_NOT_INIT; } \
    } while (0)

#define LPM_TCAM_TCAM_MEM_BYTES                                      16
#define FLOW_TCAM_TCAM_MEM_BYTES                                     28
#define Q_MGR_ENQ_TCAM_MEM_BYTES                                     28
#define IPE_CID_TCAM_MEM_BYTES                                       8
#define DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES                    4

typedef uint32 field_array_t[DRV_MAX_ARRAY_NUM];
typedef unsigned long addrs_t;

#define DRV_GPORT_TO_GCHIP(gport)        (((gport) >> 9) & 0x7f)
#define DRV_GPORT_TO_LPORT(gport)        ((gport) & 0x1ff)

typedef struct {
        int word_offset;
        int start;
        int bits;
}segs_t;

typedef struct {
        char *ptr_field_name;
		int field_id;
        int bits;
        int seg_num;
        segs_t *ptr_seg;
}fields_t;

/* table extended property type */
enum ext_content_type_s
{
    EXT_INFO_TYPE_NORMAL = 0,
    EXT_INFO_TYPE_TCAM,
    EXT_INFO_TYPE_TCAM_AD,
    EXT_INFO_TYPE_TCAM_LPM_AD, /*SDK Modify*/
    EXT_INFO_TYPE_TCAM_NAT_AD, /*SDK Modify*/
    EXT_INFO_TYPE_LPM_TCAM_IP,
    EXT_INFO_TYPE_LPM_TCAM_NAT,
    EXT_INFO_TYPE_STATIC_TCAM_KEY,
    EXT_INFO_TYPE_DESC,
    EXT_INFO_TYPE_DBG,
    EXT_INFO_TYPE_DYNAMIC,
    EXT_INFO_TYPE_MIXED,
    EXT_INFO_TYPE_INVALID,
};
typedef enum ext_content_type_s ext_content_type_t;

/* Unified tables extended property structure */
struct tbls_ext_info_s
{
    ext_content_type_t ext_content_type;
    void *ptr_ext_content;
    uint8 ext_type;     /*used to identify special table: 0-normal, 1-parser table, 2-80bits flow ad table*/
    struct tbls_ext_info_s *ptr_next;
};
typedef struct tbls_ext_info_s tbls_ext_info_t;

typedef struct tables_info_s {
        char *ptr_mod_name;
        char *ptr_tbl_name;
        uint32 slicetype : 3;
        uint32 optype    : 4;
        uint32 entry     : 25;
        uint32 byte      : 10;
        uint32 store     : 1;
        uint32 ecc       : 1;
        uint32 bus       : 1;
        uint32 stats     : 1;
        uint32 addr_num  : 8;
        uint32 field_num : 10;
        addrs_t *addrs;
        fields_t *ptr_fields;
}tables_info_t;

typedef struct tables_ext_info_s {
    int entry;
    addrs_t addrs[3];
    tbls_ext_info_t* ptr_ext_info;    /*refer to CTC_MAX_LOCAL_CHIP_NUM*/
}tables_ext_info_t;


typedef struct drv_mem_s {
    uint32 entry_num;
    uint32 addr_3w;
    uint32 addr_6w;
    uint32 addr_12w;
} drv_mem_t;

#define SDK_COSIM 0

#define DRV_BYTES_PER_MPLS_ENTRY 8       /* 8 bytes per entry for 34,45 bits*/
#define DRV_BYTES_PER_ENTRY 12        /* 12 bytes per entry for 80 bits*/
#define DRV_LPM_KEY_BYTES_PER_ENTRY 8        /* 8 bytes per entry for 40 bits */
#define DRV_LPM_AD0_BYTE_PER_ENTRY 8        /* IP prefix LPM AD 2W */
#define DRV_LPM_AD1_BYTE_PER_ENTRY 8        /* NAT prefix LPM AD 1W */
#define DRV_ADDR_BYTES_PER_ENTRY 16   /* 16 bytes address per entry for 80 bits*/
#define DRV_BYTES_PER_WORD 4
#define DRV_BITS_PER_WORD 32
#define DRV_WORDS_PER_ENTRY (DRV_BYTES_PER_ENTRY/DRV_BYTES_PER_WORD)  /* 3 words per entry for 80 bits*/
#define DRV_LPM_WORDS_PER_ENTRY (DRV_LPM_KEY_BYTES_PER_ENTRY/DRV_BYTES_PER_WORD)  /* 2 words per entry for 40 bits*/
#define DRV_HASH_85BIT_KEY_LENGTH   11
#define DRV_HASH_170BIT_KEY_LENGTH   22
#define DRV_HASH_340BIT_KEY_LENGTH   44




/* All the following asic mem base is not determined, shall refer to asic design */
#if 0
#define MAX_MEMORY_BLOCK_NUM 22
#define LM_EXT_BLOCK_NUM 0
#define MAX_DRV_BLOCK_NUM (MAX_MEMORY_BLOCK_NUM + LM_EXT_BLOCK_NUM)
#define ADDR_OFFSET 3
#define MAX_NOR_TCAM_NUM 17
#define MAX_LPM_TCAM_NUM 12
#define MAX_STATIC_TCAM_NUM 2
#define MAX_DRV_TCAM_BLOCK_NUM (MAX_NOR_TCAM_NUM + MAX_LPM_TCAM_NUM)
#endif



enum drv_ftm_mem_id_e
{
    DRV_FTM_SRAM0,
    DRV_FTM_SRAM1,
    DRV_FTM_SRAM2,
    DRV_FTM_SRAM3,
    DRV_FTM_SRAM4,
    DRV_FTM_SRAM5,
    DRV_FTM_SRAM6,
    DRV_FTM_SRAM7,
    DRV_FTM_SRAM8,
    DRV_FTM_SRAM9,
    DRV_FTM_SRAM10,

    DRV_FTM_SRAM11,
    DRV_FTM_SRAM12,
    DRV_FTM_SRAM13,
    DRV_FTM_SRAM14,
    DRV_FTM_SRAM15,
    DRV_FTM_SRAM16,
    DRV_FTM_SRAM17,
    DRV_FTM_SRAM18,
    DRV_FTM_SRAM19,

    DRV_FTM_SRAM20,
    DRV_FTM_SRAM21,
    DRV_FTM_SRAM22,
    DRV_FTM_SRAM23,
    DRV_FTM_SRAM24,
    DRV_FTM_SRAM25,
    DRV_FTM_SRAM26,
    DRV_FTM_SRAM27,
    DRV_FTM_SRAM28,
    DRV_FTM_SRAM29,
    DRV_FTM_SRAM_MAX,
    DRV_FTM_MIXED0,
    DRV_FTM_MIXED1,
    DRV_FTM_MIXED2,
    DRV_FTM_EDRAM_MAX,

    DRV_FTM_TCAM_KEY0,
    DRV_FTM_TCAM_KEY1,
    DRV_FTM_TCAM_KEY2,
    DRV_FTM_TCAM_KEY3,
    DRV_FTM_TCAM_KEY4,
    DRV_FTM_TCAM_KEY5,
    DRV_FTM_TCAM_KEY6,
    DRV_FTM_TCAM_KEY7,
    DRV_FTM_TCAM_KEY8,
    DRV_FTM_TCAM_KEY9,
    DRV_FTM_TCAM_KEY10,
    DRV_FTM_TCAM_KEY11,
    DRV_FTM_TCAM_KEY12,
    DRV_FTM_TCAM_KEY13,
    DRV_FTM_TCAM_KEY14,
    DRV_FTM_TCAM_KEY15,
    DRV_FTM_TCAM_KEY16,
    DRV_FTM_TCAM_KEY17,
    DRV_FTM_TCAM_KEY18,
    DRV_FTM_TCAM_KEYM,

    DRV_FTM_TCAM_AD0,
    DRV_FTM_TCAM_AD1,
    DRV_FTM_TCAM_AD2,
    DRV_FTM_TCAM_AD3,
    DRV_FTM_TCAM_AD4,
    DRV_FTM_TCAM_AD5,
    DRV_FTM_TCAM_AD6,
    DRV_FTM_TCAM_AD7,
    DRV_FTM_TCAM_AD8,
    DRV_FTM_TCAM_AD9,
    DRV_FTM_TCAM_AD10,
    DRV_FTM_TCAM_AD11,
    DRV_FTM_TCAM_AD12,
    DRV_FTM_TCAM_AD13,
    DRV_FTM_TCAM_AD14,
    DRV_FTM_TCAM_AD15,
    DRV_FTM_TCAM_AD16,
    DRV_FTM_TCAM_AD17,
    DRV_FTM_TCAM_AD18,
    DRV_FTM_TCAM_ADM,

    DRV_FTM_LPM_TCAM_KEY0,
    DRV_FTM_LPM_TCAM_KEY1,
    DRV_FTM_LPM_TCAM_KEY2,
    DRV_FTM_LPM_TCAM_KEY3,
    DRV_FTM_LPM_TCAM_KEY4,
    DRV_FTM_LPM_TCAM_KEY5,
    DRV_FTM_LPM_TCAM_KEY6,
    DRV_FTM_LPM_TCAM_KEY7,
    DRV_FTM_LPM_TCAM_KEY8,
    DRV_FTM_LPM_TCAM_KEY9,
    DRV_FTM_LPM_TCAM_KEY10,
    DRV_FTM_LPM_TCAM_KEY11,
    DRV_FTM_LPM_TCAM_KEYM,

    DRV_FTM_LPM_TCAM_AD0,
    DRV_FTM_LPM_TCAM_AD1,
    DRV_FTM_LPM_TCAM_AD2,
    DRV_FTM_LPM_TCAM_AD3,
    DRV_FTM_LPM_TCAM_AD4,
    DRV_FTM_LPM_TCAM_AD5,
    DRV_FTM_LPM_TCAM_AD6,
    DRV_FTM_LPM_TCAM_AD7,
    DRV_FTM_LPM_TCAM_AD8,
    DRV_FTM_LPM_TCAM_AD9,
    DRV_FTM_LPM_TCAM_AD10,
    DRV_FTM_LPM_TCAM_AD11,
    DRV_FTM_LPM_TCAM_ADM,

    DRV_FTM_QUEUE_TCAM,
    DRV_FTM_CID_TCAM,
    DRV_FTM_MAX_ID
};
typedef enum drv_ftm_mem_id_e drv_ftm_mem_id_t;


#define DRV_MEM_INFO(lchip, mem_id)         (p_drv_master[lchip]->p_mem_info[mem_id])
#define DRV_MEM_ENTRY_NUM(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).entry_num)
#define DRV_MEM_ADD3W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_3w)
#define DRV_MEM_ADD6W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_6w)
#define DRV_MEM_ADD12W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_12w)


#define MAX_MEMORY_BLOCK_NUM DRV_FTM_EDRAM_MAX
#define MAX_MIXED_MEMORY_BLOCK_NUM 3
#define MAX_DYNAMIC_MEMORY_BLOCK_NUM (MAX_MEMORY_BLOCK_NUM - MAX_MIXED_MEMORY_BLOCK_NUM)
#define LM_EXT_BLOCK_NUM 0
#define MAX_DRV_BLOCK_NUM (MAX_MEMORY_BLOCK_NUM + LM_EXT_BLOCK_NUM)
#define ADDR_OFFSET 3
#define MAX_NOR_TCAM_NUM 19
#define MAX_LPM_TCAM_NUM 12
#define MAX_STATIC_TCAM_NUM 2
#define MAX_DRV_TCAM_BLOCK_NUM (MAX_NOR_TCAM_NUM + MAX_LPM_TCAM_NUM)




#define USERID_TCAM_MAX_NUM 6
#define TCAM_BITMAP_MAX_NUM 9
#define LPM_TCAM_BITMAP_MAX_NUM 12



/* All the following asic mem base is not determined, shall refer to asic design */

enum ds_slice_type_e
{
    SLICE_Share,
    SLICE_Duplicated,
    SLICE_Cascade,
    SLICE_PreCascade,
    SLICE_Default,
};
typedef enum ds_slice_type_e ds_slice_type_t;

typedef enum ds_op_type {
    Op_Default = 1,
    Op_ReadMux = 2,
    Op_DisBurst = 4,
    Op_DisWriteMask = 8,
} ds_op_type_t;

enum dup_address_offset_type_e
{
    SLICE_Addr_All,
    SLICE_Addr_0,
    SLICE_Addr_1,
    SLICE_Addr_Invalid,
};
typedef enum dup_address_offset_type_e dup_address_offset_type_t;

#define DRV_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define DRV_BIT_SET(flag, bit)      ((flag) = (flag) | (1 << (bit)))
#define DRV_BIT_UNSET(flag, bit)      ((flag) = (flag) & (~(1 << (bit))))

/**
 @brief check whether the operation's return value is error or not
*/
extern int32 g_error_on;
#define DRV_IF_ERROR_RETURN(op) \
do\
{ \
    int32 rv; \
    if ((rv = (op)) < 0) \
    { \
      if (g_error_on) \
      sal_printf("Error Happened!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
        return(rv); \
    }\
}\
while(0)

#define DRV_IF_ERROR_GOTO(op, ret, label) \
do \
{ \
    if ((ret = (op)) < 0) \
    { \
        goto label; \
    } \
} \
while(0)

/**
 @brief check whether the operation's return value is error or not. If error, then unlock
*/
#define DRV_IF_ERROR_RETURN_WITH_UNLOCK(op, lock) \
    { \
        int32 rv; \
        if ((rv = (op)) < 0)  \
        {  \
            sal_mutex_unlock(lock); \
            return rv; \
        } \
    }

/**
 @brief define the pointer valid check
*/
#define DRV_PTR_VALID_CHECK(ptr)\
if (NULL == (ptr))\
{\
    return DRV_E_INVALID_PTR;\
}

#ifdef SDK_IN_DEBUG_VER
#define DRV_DBG_INFO(FMT, ...)                          \
                    do                                                     \
                    { \
                      sal_printf(FMT, ##__VA_ARGS__); \
                    } while (0)

#define DRV_DBG_FUNC()                          \
                    do \
                    {\
                         sal_printf("\n%s()\n", __FUNCTION__); \
                    } while (0)
 #else
    #define DRV_DBG_INFO(FMT, ...)
    #define DRV_DBG_FUNC()
 #endif

#define SHIFT_LEFT_MINUS_ONE(bits)    ( ((bits) < 32) ? ((1 << (bits)) - 1) : -1)

/* Tcam key memories extended property structure */
enum tcam_type_e
{
    TCAM_TYPE_ACL_USERID,
    TCAM_TYPE_LPM,
    TCAM_TYPE_STATIC,
    TCAM_TYPE_INVALID,
};
typedef enum tcam_type_e tcam_type_t;

struct tcam_alloc_info_s
{
    uint32 tcam_mem_bitmap;                                             /* bitmap indicate table in which tcam memory block */
    uint32 tcam_mem_hw_data_base[MAX_DRV_TCAM_BLOCK_NUM][ADDR_OFFSET];  /* hw_data_base per memory block */
    uint32 tcam_mem_hw_mask_base[MAX_DRV_TCAM_BLOCK_NUM][ADDR_OFFSET];  /* hw_data_base per memory block */
    uint32 tcam_mem_entry_num[MAX_DRV_TCAM_BLOCK_NUM];                  /* enter num in per memory block, per 80 bit */
    uint32 tcam_mem_entry_offset[MAX_DRV_TCAM_BLOCK_NUM];               /* enter num offset in per memory block, per 80 bit */
    uint32 tcam_mem_entry_start_index[MAX_DRV_TCAM_BLOCK_NUM];          /* global table start index in per memory block */
    uint32 tcam_mem_entry_end_index[MAX_DRV_TCAM_BLOCK_NUM];            /* global table end index in per memory bloci */
    uint32 tcam_mem_sw_data_base[MAX_DRV_TCAM_BLOCK_NUM];    /* hw_data_base per memory block */
    uint32 tcam_mem_lpmlkp2_entry_num;     /* lpm lookup2 entry num */
};
typedef struct tcam_alloc_info_s tcam_alloc_info_t;

struct tcam_mem_ext_content_s
{
    addrs_t *hw_mask_base;
    uint8 key_size;
    uint8 tcam_type;
    uint8 tcam_module;

    tcam_alloc_info_t tcam_alloc_info;
};
typedef struct tcam_mem_ext_content_s tcam_mem_ext_content_t;

/* dynamic tbl extended property structure */
enum dynamic_mem_access_mode_u
{
    DYNAMIC_DEFAULT = 0,    /* according table entry size to decide how many 80-bits per index */
    DYNAMIC_4W_MODE,        /* per 80 bits per index */
    DYNAMIC_8W_MODE,        /* per 80 bits per index */
    DYNAMIC_16W_MODE,        /* per 80 bits per index */
    DYNAMIC_1W_MODE,
    DYNAMIC_2W_MODE,
    DYNAMIC_INVALID,
};
typedef enum dynamic_mem_access_mode_u dynamic_mem_access_mode_t;

struct dynamic_mem_allocate_info_s
{
    uint32 dynamic_mem_bitmap;                                          /* bitmap indicate table in which dynamic memory block */
    uint32 dynamic_mem_hw_data_base[MAX_DRV_BLOCK_NUM][ADDR_OFFSET];    /* hw_data_base per memory block */
    uint32 dynamic_mem_entry_num[MAX_DRV_BLOCK_NUM];                    /* enter num in per memory block, per 80 bit */
    uint32 dynamic_mem_entry_start_index[MAX_DRV_BLOCK_NUM];            /* global table start index in per memory block */
    uint32 dynamic_mem_entry_end_index[MAX_DRV_BLOCK_NUM];              /* global table end index in per memory block*/
    uint32 dynamic_mem_block_offset[MAX_DRV_BLOCK_NUM];                 /* local offset in per memory block*/
};
typedef struct dynamic_mem_allocate_info_s dynamic_mem_allocate_info_t;

struct dynamic_mem_ext_content_s
{
    dynamic_mem_allocate_info_t dynamic_mem_allocate_info;  /* store memory allocated info of table in which block */
    dynamic_mem_access_mode_t dynamic_access_mode;          /* access mode, indicate how to explain one index of table */
};
typedef struct dynamic_mem_ext_content_s dynamic_mem_ext_content_t;

struct drv_ecc_sbe_cnt_s
{
    tbls_id_t tblid;
    char*     p_tbl_name;
    tbls_id_t rec;
    uint32    fld;
};
typedef struct drv_ecc_sbe_cnt_s drv_ecc_sbe_cnt_t;


struct drv_ecc_data_s
{
        drv_ecc_sbe_cnt_t*  p_sbe_cnt;
        drv_ecc_intr_tbl_t* p_intr_tbl;
        uint16              (*p_scan_tcam_tbl)[5];
};
typedef struct drv_ecc_data_s drv_ecc_data_t;



#define TCAM_TYPE_EXT (tcam_mem_ext_content_t *)
#define DYNAMIC_TYPE_EXT (dynamic_mem_ext_content_t *)

/* Define get Memory infomation operation */
#define TABLE_INFO_PTR(lchip, tbl_id)             (&p_drv_master[lchip]->p_tbl_info[tbl_id])
#define TABLE_INFO(lchip, tbl_id)                 (p_drv_master[lchip]->p_tbl_info[tbl_id])
#define MODULE_NAME(lchip, tbl_id)                (TABLE_INFO(lchip, tbl_id).ptr_mod_name)
#define TABLE_STATS(lchip, tbl_id)                (TABLE_INFO(lchip, tbl_id).stats)
#define TABLE_BUS(lchip, tbl_id)                  (TABLE_INFO(lchip, tbl_id).bus)
#define TABLE_NAME(lchip, tbl_id)                 (TABLE_INFO(lchip, tbl_id).ptr_tbl_name)
#define TABLE_ENTRY_TYPE(lchip, tbl_id)           (TABLE_INFO(lchip, tbl_id).slicetype)
#define TABLE_DATA_BASE(lchip, tbl_id, addroffset)(TABLE_INFO(lchip, tbl_id).entry?(TABLE_INFO(lchip, tbl_id).addrs[addroffset]):(TABLE_EXT_INFO(lchip, tbl_id).addrs[addroffset]))
#define TABLE_MAX_INDEX(lchip, tbl_id)            (TABLE_INFO(lchip, tbl_id).entry?(TABLE_INFO(lchip, tbl_id).entry):(TABLE_EXT_INFO(lchip, tbl_id).entry))
#define TABLE_ENTRY_SIZE(lchip, tbl_id)           (TABLE_INFO(lchip, tbl_id).byte)
#define TABLE_MAX_ADDR_OFFSET(lchip, tbl_id)      ((TABLE_MAX_INDEX(lchip, tbl_id))*((TABLE_ENTRY_SIZE(lchip, tbl_id)/DRV_BYTES_PER_ENTRY*DRV_ADDR_BYTES_PER_ENTRY)))
#define TABLE_DATA_END_ADDR(lchip, tbl_id)        ((TABLE_DATA_BASE(lchip, tbl_id))+(TABLE_MAX_ADDR_OFFSET(lchip, tbl_id)))
#define TABLE_FIELD_NUM(lchip, tbl_id)            (TABLE_INFO(lchip, tbl_id).field_num)
#define TABLE_FIELD_INFO_PTR(lchip, tbl_id)       (TABLE_INFO(lchip, tbl_id).ptr_fields)

#define TABLE_EXT_INFO(lchip, tbl_id)             (p_drv_master[lchip]->p_tbl_ext_info[tbl_id])
#define TABLE_EXT_INFO_PTR(lchip, tbl_id)         (TABLE_EXT_INFO(lchip, tbl_id).ptr_ext_info)
#define TABLE_EXT_INFO_TYPE(lchip, tbl_id)        (TABLE_EXT_INFO_PTR(lchip, tbl_id)->ext_content_type)
#define TABLE_EXT_INFO_CONTENT_PTR(lchip, tbl_id) (TABLE_EXT_INFO_PTR(lchip, tbl_id)->ptr_ext_content)
#define TABLE_OP_TYPE(lchip, tbl_id)           (TABLE_INFO(lchip, tbl_id).optype)
#define TCAM_LPM_LKP2_ENTRY_NUM(lchip, tbl_id)        (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_lpmlkp2_entry_num)
#define TABLE_EXT_TYPE(lchip, tbl_id)        (TABLE_EXT_INFO_PTR(lchip, tbl_id)->ext_type)

#define BURST_IO_WRITE  0x1
#define BURST_IO_READ   0x1
#define BURST_IO_MASK   0x2

 /*#define TABLE_FIELD_SEG_PTR(seg_id)     (TABLE_INFO(lchip, tbl_id).ptr_fields.)*/

#define CHK_IS_REGISTER(lchip, tbl_id)\
    ((tbl_id >= 0)&&(tbl_id < MaxTblId_t)&&(TABLE_MAX_INDEX(lchip, tbl_id) == 1))\

#define CHK_TABLE_ID_VALID(lchip, tbl_id)\
    ((tbl_id >= 0)&&(tbl_id < MaxTblId_t))\

#define CHK_FIELD_ID_VALID(lchip, tbl_id, field_id)\
    ((field_id) >= 0)&&((field_id) < TABLE_FIELD_NUM(lchip, tbl_id))\


#define CHK_TABLE_EXT_INFO_ISTCAM(lchip, tbl_id)\
    (EXT_INFO_TYPE_TCAM == (TABLE_EXT_INFO_TYPE(lchip, tbl_id)))\


#define TCAM_EXT_INFO_PTR(lchip, tbl_id)                   (TCAM_TYPE_EXT (TABLE_EXT_INFO_PTR(lchip, tbl_id)->ptr_ext_content))
#define TCAM_MASK_BASE_LPM(lchip, tbl_id, addroffset)      (TCAM_EXT_INFO_PTR(lchip, tbl_id)->hw_mask_base[addroffset])
#define TCAM_KEY_SIZE(lchip, tbl_id)                       (TCAM_EXT_INFO_PTR(lchip, tbl_id)->key_size)
#define TCAM_KEY_TYPE(lchip, tbl_id)                       (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_type)
#define TCAM_KEY_MODULE(lchip, tbl_id)                       (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_module)

#define TCAM_BITMAP(lchip, tbl_id)                         (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_bitmap)
#define TCAM_DATA_BASE(lchip, tbl_id, blk_id, addroffset)  (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_hw_data_base[blk_id][addroffset])
#define TCAM_MASK_BASE(lchip, tbl_id, blk_id, addroffset)  (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_hw_mask_base[blk_id][addroffset])
#define TCAM_ENTRY_NUM(lchip, tbl_id, blk_id)              (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_entry_num[blk_id])
#define TCAM_OFFSET_INDEX(lchip, tbl_id, blk_id)           (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_entry_offset[blk_id])
#define TCAM_START_INDEX(lchip, tbl_id, blk_id)            (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_entry_start_index[blk_id])
#define TCAM_END_INDEX(lchip, tbl_id, blk_id)              (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_entry_end_index[blk_id])
#define TCAM_TABLE_SW_BASE(lchip, tbl_id, blk_id)  (TCAM_EXT_INFO_PTR(lchip, tbl_id)->tcam_alloc_info.tcam_mem_sw_data_base[blk_id])


#define DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)                    (DYNAMIC_TYPE_EXT (TABLE_EXT_INFO_PTR(lchip, tbl_id)->ptr_ext_content))
#define DYNAMIC_ACCESS_MODE(lchip, tbl_id)                     (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_access_mode)
#define DYNAMIC_BITMAP(lchip, tbl_id)                          (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_bitmap)
#define DYNAMIC_DATA_BASE(lchip, tbl_id, blk_id, addroffset)   (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_hw_data_base[blk_id][addroffset])
#define DYNAMIC_ENTRY_NUM(lchip, tbl_id, blk_id)               (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_entry_num[blk_id])
#define DYNAMIC_START_INDEX(lchip, tbl_id, blk_id)             (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_entry_start_index[blk_id])
#define DYNAMIC_END_INDEX(lchip, tbl_id, blk_id)               (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_entry_end_index[blk_id])
#define DYNAMIC_MEM_OFFSET(lchip, tbl_id, blk_id)              (DYNAMIC_EXT_INFO_PTR(lchip, tbl_id)->dynamic_mem_allocate_info.dynamic_mem_block_offset[blk_id])

/* check Table Id valid */
#define DRV_TBL_ID_VALID_CHECK(lchip, tbl_id) \
if ((tbl_id) >= (MaxTblId_t))\
{\
    DRV_DBG_INFO("\nERROR! INVALID TblID! TblID: %d, file:%s line:%d function:%s\n",tbl_id, __FILE__,__LINE__,__FUNCTION__);\
    return DRV_E_INVALID_TBL;\
}

/* check if Table is empty */
#define DRV_TBL_EMPTY_CHECK(lchip, tbl_id) \
if (TABLE_MAX_INDEX(lchip, tbl_id) == 0)\
{\
    DRV_DBG_INFO("\nERROR! Operation on an empty table! TblID: %d, Name:%s, file:%s line:%d function:%s\n",tbl_id, TABLE_NAME(lchip, tbl_id), __FILE__,__LINE__,__FUNCTION__);\
    return DRV_E_INVALID_TBL;\
}

/* check is index of table id is valid */
#define DRV_TBL_INDEX_VALID_CHECK(lchip, tbl_id, index) \
if (index >= TABLE_MAX_INDEX(lchip, tbl_id))\
{\
    DRV_DBG_INFO("\nERROR! Index-0x%x exceeds the max_index 0x%x! TblID: %d, Name:%s, file:%s line:%d function:%s\n", \
                   index, TABLE_MAX_INDEX(lchip, tbl_id), tbl_id, TABLE_NAME(lchip, tbl_id), __FILE__,__LINE__,__FUNCTION__);\
    return DRV_E_INVALID_INDEX;\
}


enum drv_enum_e
{
    DRV_TCAMKEYTYPE_MACKEY_160,
    DRV_TCAMKEYTYPE_L3KEY_160 ,
    DRV_TCAMKEYTYPE_L3KEY_320 ,
    DRV_TCAMKEYTYPE_IPV6KEY_320 ,
    DRV_TCAMKEYTYPE_IPV6KEY_640 ,
    DRV_TCAMKEYTYPE_MACL3KEY_320,
    DRV_TCAMKEYTYPE_MACL3KEY_640,
    DRV_TCAMKEYTYPE_MACIPV6KEY_640,
    DRV_TCAMKEYTYPE_CIDKEY_160,
    DRV_TCAMKEYTYPE_SHORTKEY_80,
    DRV_TCAMKEYTYPE_FORWARDKEY_320,
    DRV_TCAMKEYTYPE_FORWARDKEY_640,
    DRV_TCAMKEYTYPE_COPPKEY_320,
    DRV_TCAMKEYTYPE_COPPKEY_640,
    DRV_TCAMKEYTYPE_UDFKEY_320,

    DRV_SCL_KEY_TYPE_L2KEY,
    DRV_SCL_KEY_TYPE_L3KEY,
    DRV_SCL_KEY_TYPE_L2L3KEY,
    /**< [TM] D2 Use the key type from port, while TM use the key type from scl */
    DRV_SCL_KEY_TYPE_MACKEY160,
    DRV_SCL_KEY_TYPE_MACL3KEY320,
    DRV_SCL_KEY_TYPE_L3KEY160,
    DRV_SCL_KEY_TYPE_IPV6KEY320,
    DRV_SCL_KEY_TYPE_MACIPV6KEY640,
    DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT,
    DRV_SCL_KEY_TYPE_UDFKEY160,
    DRV_SCL_KEY_TYPE_UDFKEY320,
    DRV_SCL_KEY_TYPE_MASK,

    DRV_STK_MUX_TYPE_HDR_REGULAR_PORT,
    DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL,
    DRV_STK_MUX_TYPE_HDR_WITH_L2,
    DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4,
    DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6,
    DRV_STK_MUX_TYPE_HDR_WITH_IPV4,
    DRV_STK_MUX_TYPE_HDR_WITH_IPV6,

    DRV_DMA_PACKET_RX0_CHAN_ID,
    DRV_DMA_PACKET_RX1_CHAN_ID,
    DRV_DMA_PACKET_RX2_CHAN_ID,
    DRV_DMA_PACKET_RX3_CHAN_ID,
    DRV_DMA_PACKET_TX0_CHAN_ID,
    DRV_DMA_PACKET_TX1_CHAN_ID,
    DRV_DMA_PACKET_TX2_CHAN_ID,
    DRV_DMA_PACKET_TX3_CHAN_ID,
    DRV_DMA_TBL_WR_CHAN_ID,
    DRV_DMA_TBL_RD_CHAN_ID,
    DRV_DMA_TBL_RD1_CHAN_ID,
    DRV_DMA_TBL_RD2_CHAN_ID,
    DRV_DMA_PORT_STATS_CHAN_ID,
    DRV_DMA_FLOW_STATS_CHAN_ID,
    DRV_DMA_REG_MAX_CHAN_ID,
    DRV_DMA_LEARNING_CHAN_ID,
    DRV_DMA_HASHKEY_CHAN_ID,
    DRV_DMA_IPFIX_CHAN_ID,
    DRV_DMA_SDC_CHAN_ID,
    DRV_DMA_MONITOR_CHAN_ID,
    DRV_DMA_MAX_CHAN_ID,
    DRV_DMA_TCAM_SCAN_DESC_NUM,

    DRV_USERID_HASHTYPE_TUNNELIPV4DA,
    DRV_USERID_HASHTYPE_SCLFLOWL2,

    DRV_ENUM_MAX
};
typedef enum drv_enum_e drv_enum_t;

#define DRV_ENUM(type)   p_drv_master[lchip]->p_enum_value[type]

struct drv_master_s
{
    uint8 access_type;
    uint8 plaform_type;
    uint8 workenv_type;
    uint8 host_type;
    uint8 burst_en;
    uint8 wb_status;
    uint8 g_lchip;
    uint8 init_done;
    void* p_ser_master;
    void* p_ser_db;

    sal_mutex_t* p_flow_tcam_mutex;
    sal_mutex_t* p_lpm_ip_tcam_mutex;
    sal_mutex_t* p_lpm_nat_tcam_mutex;
    sal_mutex_t* p_entry_mutex;
    sal_mutex_t* fib_acc_mutex;
    sal_mutex_t* cpu_acc_mutex;
    sal_mutex_t* ipfix_acc_mutex;
    sal_mutex_t* cid_acc_mutex;
    sal_mutex_t* mpls_acc_mutex;
    sal_mutex_t* gemport_acc_mutex;

    sal_mutex_t* p_mep_mutex;
    sal_mutex_t* p_pci_mutex;
    sal_mutex_t* p_i2c_mutex;
    sal_mutex_t* p_hss_mutex;


    uint32 *p_enum_value;
    tables_info_t* p_tbl_info;
    tables_ext_info_t* p_tbl_ext_info;
    drv_mem_t* p_mem_info;
    drv_ecc_data_t drv_ecc_data;
    drv_io_callback_fun_t drv_io_api;  /* driver IO callback function */

    int32 (*drv_mem_get_edram_bitmap)(uint8 lchip, uint8 sram_type, uint32* bit);
    int32 (*drv_mem_get_hash_type)(uint8 lchip, uint8 sram_type, uint32 mem_id, uint8 couple, uint32 *poly);

    int32 (*drv_get_mcu_lock_id)(uint8 lchip, tbls_id_t tbl_id, uint8* p_mcu_id, uint32* p_lock_id);
    int32 (*drv_get_mcu_addr)(uint8 mcu_id, uint32* mutex_data_addr, uint32* mutex_mask_addr);

    int32 (*drv_chip_read_hss15g)(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data);
    int32 (*drv_chip_write_hss15g)(uint8 lchip, uint8 hssid, uint32 addr, uint16 data);

    int32 (*drv_chip_read_hss28g)(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data);
    int32 (*drv_chip_write_hss28g)(uint8 lchip, uint8 hssid, uint32 addr, uint16 data);

};
typedef struct drv_master_s drv_master_t;


#define DRV_MEM_GET_EDRAM_BITMAP_FUNC (*p_drv_master[lchip]->drv_mem_get_edram_bitmap)
#define DRV_MEM_GET_HASH_TYPE_FUNC (*p_drv_master[lchip]->drv_mem_get_hash_type)

#define DRV_GET_MCU_LOCK_ID (*p_drv_master[lchip]->drv_get_mcu_lock_id)
#define DRV_GET_MCU_ADDR    (*p_drv_master[lchip]->drv_get_mcu_addr)

typedef enum
{
    DRV_MCU_LOCK_WA_CFG = 0,
    DRV_MCU_LOCK_HSS15G_REG,
    DRV_MCU_LOCK_HSS28G_REG,
    DRV_MCU_LOCK_PCS_RESET,
    DRV_MCU_LOCK_EYE_SCAN_0,
    DRV_MCU_LOCK_EYE_SCAN_1,
    DRV_MCU_LOCK_EYE_SCAN_2,
    DRV_MCU_LOCK_EYE_SCAN_3,
    DRV_MCU_LOCK_EYE_SCAN_4,
    DRV_MCU_LOCK_EYE_SCAN_5,
    DRV_MCU_LOCK_EYE_SCAN_6,
    DRV_MCU_LOCK_EYE_SCAN_7,
    DRV_MCU_LOCK_TXEQ_CFG,
    DRV_MCU_LOCK_RXEQ_CFG,

    DRV_MCU_LOCK_NONE,

    DRV_MCU_LOCK_MAX
} drv_mcu_lock_t;

#define DRV_MAX_CHIP_NUM 128
extern drv_master_t* p_drv_master[DRV_MAX_CHIP_NUM];

extern int32 dal_get_chip_number(uint8* p_num);
extern int32 drv_usw_get_tbl_index_base(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint8 *addr_offset);
extern int32 drv_usw_table_get_hw_addr(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *hw_addr, uint32 flag);
extern uint32 drv_usw_get_table_type (uint8 lchip, tbls_id_t tb_id);
extern int32 drv_usw_table_consum_hw_addr_size_per_index(uint8 lchip, tbls_id_t tbl_id, uint32 *hw_addr_size);
extern int32 drv_usw_get_tbl_string_by_id(uint8 lchip, tbls_id_t tbl_id, char* name);
extern int32 drv_usw_get_tbl_id_by_string(uint8 lchip, tbls_id_t* tbl_id, char* name);
extern int32 drv_usw_get_field_string_by_id(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, char* name);
extern int32 drv_usw_get_field_id_by_string(uint8 lchip, tbls_id_t tbl_id, fld_id_t* field_id, char* name);
extern int32 drv_usw_get_field_id_by_sub_string(uint8 lchip, tbls_id_t tbl_id, fld_id_t* field_id, char* name);
extern int32 drv_usw_get_field_offset(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, uint32* w_offset, uint32 *b_offset);
extern int8 drv_usw_table_is_slice1(uint8 lchip, tbls_id_t tbl_id);
extern int32 drv_usw_mcu_tbl_lock(uint8 lchip, tbls_id_t tbl_id, uint32 op);
extern int32 drv_usw_mcu_tbl_unlock(uint8 lchip, tbls_id_t tbl_id, uint32 op);
extern int32 drv_usw_mcu_lock(uint8 lchip, uint32 lock_id, uint8 mcu_id);
extern int32 drv_usw_mcu_unlock(uint8 lchip, uint32 lock_id, uint8 mcu_id);
#endif /*end of _DRV_COMMON_H*/


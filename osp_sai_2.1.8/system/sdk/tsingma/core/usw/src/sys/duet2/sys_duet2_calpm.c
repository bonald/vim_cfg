#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_duet2_calpm.c

 @date 2009-12-09

 @version v2.0

 The file contains all ipuc related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_debug.h"
 /*#include "ctc_dma.h"*/

#include "sys_usw_sort.h"
#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_rpf_spool.h"

#include "sys_usw_nexthop_api.h"
#include "sys_usw_ftm.h"
#include "sys_usw_dma.h"
#include "sys_usw_common.h"
#include "sys_usw_wb_common.h"
#include "sys_duet2_ipuc_tcam.h"
#include "sys_duet2_calpm.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define MIX(a, b, c) \
    do \
    { \
        a -= c;  a ^= ROT(c, 4);  c += b; \
        b -= a;  b ^= ROT(a, 6);  a += c; \
        c -= b;  c ^= ROT(b, 8);  b += a; \
        a -= c;  a ^= ROT(c, 16);  c += b; \
        b -= a;  b ^= ROT(a, 19);  a += c; \
        c -= b;  c ^= ROT(b, 4);  b += a; \
    } while (0)

#define FINAL(a, b, c) \
    { \
        c ^= b; c -= ROT(b, 14); \
        a ^= c; a -= ROT(c, 11); \
        b ^= a; b -= ROT(a, 25); \
        c ^= b; c -= ROT(b, 16); \
        a ^= c; a -= ROT(c, 4);  \
        b ^= a; b -= ROT(a, 14); \
        c ^= b; c -= ROT(b, 24); \
    }

static uint8 ds_calpm_test_mask_table[256] =
    {
        0,

        128,  64,  32,  16,   8,   4,   2,   1,

        192, 160, 144, 136, 132, 130, 129,  96,  80,  72,
        68,  66,  65,  48,  40,  36,  34,  33,  24,  20,
        18,  17,  12,  10,   9,   6,   5,   3,

        224, 208, 200, 196, 194, 193, 176, 168, 164, 162,
        161, 152, 148, 146, 145, 140, 138, 137, 134, 133,
        131, 112, 104, 100,  98,  97,  88,  84,  82,  81,
        76,  74,  73,  70,  69,  67,  56,  52,  50,  49,
        44,  42,  41,  38,  37,  35,  28,  26,  25,  22,
        21,  19,  14,  13,  11,   7,

        240, 232, 228, 226, 225, 216, 212, 210, 209, 204,
        202, 201, 198, 197, 195, 184, 180, 178, 177, 172,
        170, 169, 166, 165, 163, 156, 154, 153, 150, 149,
        147, 142, 141, 139, 135, 120, 116, 114, 113, 108,
        106, 105, 102, 101,  99,  92,  90,  89,  86,  85,
        83,  78,  77,  75,  71,  60,  58,  57,  54,  53,
        51,  46,  45,  43,  39,  30,  29,  27,  23,  15,

        248, 244, 242, 241, 236, 234, 233, 230, 229, 227,
        220, 218, 217, 214, 213, 211, 206, 205, 203, 199,
        188, 186, 185, 182, 181, 179, 174, 173, 171, 167,
        158, 157, 155, 151, 143, 124, 122, 121, 118, 117,
        115, 110, 109, 107, 103,  94,  93,  91,  87,  79,
        62,  61,  59,  55,  47,  31,

        252, 250, 249, 246, 245, 243, 238, 237, 235, 231,
        222, 221, 219, 215, 207, 190, 189, 187, 183, 175,
        159, 126, 125, 123, 119, 111,  95,  63,

        254, 253, 251, 247, 239, 223, 191, 127,

        255
    };

 #define SYS_CALPM_DBG_OUT(level, FMT, ...)                              \
    {                                                                       \
        if (0)  \
        {   \
            CTC_DEBUG_OUT(ip, ipuc, IPUC_SYS, level, FMT, ##__VA_ARGS__);  \
        }   \
    }

#define SYS_CALPM_INIT_CHECK                                        \
    {                                                              \
        LCHIP_CHECK(lchip);                                        \
        if (p_duet2_calpm_master[lchip] == NULL)                          \
        {                                                          \
            return CTC_E_NOT_INIT;                                 \
        }                                                          \
    }

#define SYS_CALPM_LOCK \
    if(p_duet2_calpm_master[lchip]->mutex) sal_mutex_lock(p_duet2_calpm_master[lchip]->mutex)

#define SYS_CALPM_UNLOCK \
    if(p_duet2_calpm_master[lchip]->mutex) sal_mutex_unlock(p_duet2_calpm_master[lchip]->mutex)

#define CTC_ERROR_RETURN_CALPM_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_duet2_calpm_master[lchip]->mutex); \
            return (rv); \
        } \
    } while (0)

#define CTC_RETURN_CALPM_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_duet2_calpm_master[lchip]->mutex); \
        return (op); \
    } while (0)

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
sys_calpm_master_t* p_duet2_calpm_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
drv_usw_table_get_hw_addr(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *hw_addr, uint32 flag);

STATIC sys_calpm_item_t*
ITEM_GET_OR_INIT_PTR(sys_calpm_tbl_t* ptbl, uint16 idx)
{
    sys_calpm_item_t* ptr = NULL;

    if (CTC_BMP_ISSET(ptbl->item_bitmap, idx))
    {
        ptr = ptbl->p_item[(idx / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM))];
        while (ptr)
        {
            if (idx == ptr->item_idx)
            {
                break;
            }
            ptr = ptr->p_nitem;
        }
    }
    else
    {
        ptr = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_item_t));
        if (NULL == ptr)
        {
            return NULL;
        }
        sal_memset(ptr, 0, sizeof(sys_calpm_item_t));

        ptr->ad_index = INVALID_NEXTHOP_OFFSET;
        ptr->p_nitem = ptbl->p_item[(idx / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM))];
        ptr->item_idx = idx;
        ptbl->p_item[(idx / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM))] = ptr;
        CTC_BMP_SET(ptbl->item_bitmap, idx);
    }

    return ptr;
}

STATIC sys_calpm_item_t*
ITEM_GET_PTR(sys_calpm_tbl_t* ptbl, uint16 idx)
{
    sys_calpm_item_t* ptr = NULL;
    if (CTC_BMP_ISSET(ptbl->item_bitmap, idx))
    {
        ptr = ptbl->p_item[(idx / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM))];
        while (ptr)
        {
            if (idx == ptr->item_idx)
            {
                break;
            }
            ptr = ptr->p_nitem;
        }
    }

    return ptr;
}

STATIC void
ITEM_FREE_PTR(sys_calpm_tbl_t* ptbl, sys_calpm_item_t* ptr)
{
    sys_calpm_item_t* p_tmp =
        ptbl->p_item[(ptr->item_idx) / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM)];

    if (ptbl->p_item[(ptr->item_idx) / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM)] == ptr)
    {
        ptbl->p_item[(ptr->item_idx) / (CALPM_ITEM_NUM / CALPM_ITEM_ALLOC_NUM)]
            = ptr->p_nitem;
        p_tmp = NULL;
    }

    while (p_tmp)
    {
        if (p_tmp->p_nitem == ptr)
        {
            break;
        }
        p_tmp = p_tmp->p_nitem;
    }

    if (p_tmp)
    {
        p_tmp->p_nitem = ptr->p_nitem;
    }
    CTC_BMP_UNSET(ptbl->item_bitmap, ptr->item_idx);
    mem_free(ptr);
}

STATIC void
TABLE_SET_PTR(sys_calpm_tbl_t*** ptr_o, uint8 idx, sys_calpm_tbl_t* ptr)
{
    if (ptr_o[idx/CALPM_TABLE_BLOCK_NUM] == NULL)
    {
        ptr_o[idx/CALPM_TABLE_BLOCK_NUM] =
            mem_malloc(MEM_IPUC_MODULE, (sizeof(sys_calpm_tbl_t*) * (CALPM_TABLE_BLOCK_NUM)));
        if (NULL == ptr_o[idx/CALPM_TABLE_BLOCK_NUM])
        {
            return;
        }
        sal_memset(ptr_o[idx/CALPM_TABLE_BLOCK_NUM], 0
                   , (sizeof(sys_calpm_tbl_t*) * (CALPM_TABLE_BLOCK_NUM)));
    }
    ptr_o[idx/CALPM_TABLE_BLOCK_NUM][idx%CALPM_TABLE_BLOCK_NUM] = ptr;
}

STATIC sys_calpm_tbl_t*
TABLE_GET_PTR(sys_calpm_tbl_t*ptr_o, uint8 idx)
{
    sys_calpm_tbl_t* ptr = NULL;

    if ((ptr_o == NULL) || (ptr_o->stage_index == CALPM_TABLE_INDEX1)
        || (NULL == ptr_o->p_ntable[idx/CALPM_TABLE_BLOCK_NUM]))
    {
        ptr = NULL;
    }
    else
    {
        ptr = ptr_o->p_ntable[idx/CALPM_TABLE_BLOCK_NUM]
            [idx% CALPM_TABLE_BLOCK_NUM];
    }

    return ptr;
}

/* get all table */
STATIC sys_calpm_tbl_t*
TABEL_GET_STAGE(uint8 I, sys_calpm_param_t* p_calpm_param)
{
    uint32 IP;
    uint8 idx;
    uint8 SHIFT = 0;
    uint8 STAGE = I;
    uint8 masklen = 0;
    sys_calpm_tbl_t* ptr = NULL;
    sys_calpm_prefix_t* p_calpm_prefix = (p_calpm_param)->p_calpm_prefix;
    ctc_ipuc_param_t* p_ipuc_param = (p_calpm_param)->p_ipuc_param;

    ptr = (p_calpm_param)->p_calpm_prefix->p_ntable;
    if ((STAGE != CALPM_TABLE_INDEX0) && (ptr != NULL))
    {
        SHIFT = (CTC_IP_VER_4 == p_ipuc_param->ip_ver) ? (16 / p_calpm_prefix->prefix_len) : 1;
        masklen = ((I * 8) + p_calpm_prefix->prefix_len);
        if (masklen >= (p_ipuc_param->masklen))
        {
            ptr = NULL;
        }

        IP = (CTC_IP_VER_4 == p_ipuc_param->ip_ver) ? p_ipuc_param->ip.ipv4 : p_ipuc_param->ip.ipv6[2];
        while ((STAGE != CALPM_TABLE_INDEX0) && (ptr != NULL))
        {
            idx = (IP >> (SHIFT * 8)) & 0xFF;
            ptr = TABLE_GET_PTR((ptr), idx);
            STAGE--;
            SHIFT--;
        }
    }

    return ptr;
}

STATIC void
TABLE_FREE_PTR(sys_calpm_tbl_t*** ptr_o, uint8 idx)
{
    uint8 IS_FREE = 1;
    uint8 I = 0;
    ptr_o[idx/CALPM_TABLE_BLOCK_NUM][idx% CALPM_TABLE_BLOCK_NUM] = NULL;
    while (I < (CALPM_TABLE_BLOCK_NUM))
    {
        if (ptr_o[idx/CALPM_TABLE_BLOCK_NUM][I] != NULL)
        {
            IS_FREE = 0;
            break;
        }
        I++;
    }
    if (IS_FREE)
    {
        mem_free(ptr_o[idx/CALPM_TABLE_BLOCK_NUM]);
        ptr_o[idx/CALPM_TABLE_BLOCK_NUM] = NULL;
    }
}

#define __1_CALPM_PREFIX_DB_MANAGE__
uint32
_sys_duet2_calpm_prefix_ipv4_key_make(sys_calpm_prefix_t* p_calpm_prefix)
{
    uint32 a = 0x9e3779b9; /* the golden ratio; an arbitrary value */
    uint32 b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
    uint32 c = 0;
    uint8 masklen_o = 32 - p_calpm_prefix->prefix_len;

    CTC_PTR_VALID_CHECK(p_calpm_prefix);

    a += (p_calpm_prefix->ip32[0] >> masklen_o) & 0xFF;
    b += p_calpm_prefix->vrf_id;
    c += 15;
    MIX(a, b, c);

    return c % CALPM_PREFIX_HASH_BLOCK_SIZE;
}

int32
_sys_duet2_calpm_prefix_ipv4_key_cmp(sys_calpm_prefix_t* p_calpm_prefix_o, sys_calpm_prefix_t* p_calpm_prefix)
{
    uint32 mask = 0;
    uint8 masklen_o = 32 - p_calpm_prefix_o->prefix_len;

    CTC_PTR_VALID_CHECK(p_calpm_prefix_o);
    CTC_PTR_VALID_CHECK(p_calpm_prefix);

    if (p_calpm_prefix->vrf_id != p_calpm_prefix_o->vrf_id)
    {
        return FALSE;
    }

    if (p_calpm_prefix->prefix_len != p_calpm_prefix_o->prefix_len)
    {
        return FALSE;
    }

    mask = (1 << p_calpm_prefix_o->prefix_len) - 1;

    if (((p_calpm_prefix->ip32[0] >> masklen_o) & mask) != ((p_calpm_prefix_o->ip32[0] >> masklen_o) & mask))
    {
        return FALSE;
    }

    return TRUE;
}

uint32
_sys_duet2_calpm_prefix_ipv6_key_make(sys_calpm_prefix_t* p_calpm_prefix)
{
    uint32 a = 0x9e3779b9; /* the golden ratio; an arbitrary value */
    uint32 b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
    uint32 c = 0;

    CTC_PTR_VALID_CHECK(p_calpm_prefix);

    a += (p_calpm_prefix->ip32[0]) & 0xFF;
    b += (p_calpm_prefix->ip32[0] >> 8) & 0xFF;
    c += (p_calpm_prefix->ip32[0] >> 16) & 0xFF;
    MIX(a, b, c);

    c += 7;  /* vrfid and masklen and ipv6 route length - 12, unit is byte */

    a += (p_calpm_prefix->ip32[0] >> 24) & 0xFF;
    b += p_calpm_prefix->vrf_id;
    c += p_calpm_prefix->prefix_len;
    FINAL(a, b, c);

    return c % CALPM_PREFIX_HASH_BLOCK_SIZE;
}

int32
_sys_duet2_calpm_prefix_ipv6_key_cmp(sys_calpm_prefix_t* p_calpm_prefix_o, sys_calpm_prefix_t* p_calpm_prefix)
{
    uint32 mask = 0;
    uint8   mask_len = 0;

    CTC_PTR_VALID_CHECK(p_calpm_prefix_o);
    CTC_PTR_VALID_CHECK(p_calpm_prefix);

    if (p_calpm_prefix->vrf_id != p_calpm_prefix_o->vrf_id)
    {
        return FALSE;
    }

    if (p_calpm_prefix->prefix_len != p_calpm_prefix_o->prefix_len)
    {
        return FALSE;
    }

    if (p_calpm_prefix_o->prefix_len >= 32)
    {
        if (p_calpm_prefix->ip32[0] != p_calpm_prefix_o->ip32[0])
        {
            return FALSE;
        }
    }

    mask_len = p_calpm_prefix_o->prefix_len % 32;
    IPV4_LEN_TO_MASK(mask, mask_len);
    if ((p_calpm_prefix->ip32[p_calpm_prefix->prefix_len/32] & mask) != (p_calpm_prefix_o->ip32[p_calpm_prefix_o->prefix_len/32] & mask))
    {
        return FALSE;
    }

    return TRUE;
}

#define __2_CALPM_PREFIX_OFFSET__
STATIC uint8
_sys_duet2_calpm_get_stage_ip(uint8 lchip, uint32* ip32, uint8 ip_ver, uint8 stage)
{
    sys_ip_octo_t index;
    uint8 prefix_len = p_duet2_calpm_master[lchip]->prefix_len[ip_ver];

    if (CTC_IP_VER_4 == ip_ver)
    {
        index = (prefix_len == 8) ? SYS_IP_OCTO_8_15 : SYS_IP_OCTO_0_7;
        index += (stage == CALPM_TABLE_INDEX0) ? 1 : 0;
    }
    else
    {
        index = SYS_IP_OCTO_88_95 - (prefix_len - 32)/8 - stage;
    }

    return ((ip32[index / 4] >> ((index & 3) * 8)) & 0xFF);
}

uint32
sys_duet2_calpm_get_extra_offset(uint8 lchip, uint8 sram_index, sys_calpm_offset_type_t type)
{
    uint32 offset = 0;

    switch (type)
    {
        case SYS_CALPM_START_OFFSET:
            offset = p_duet2_calpm_master[lchip]->start_stage_offset[sram_index];
            break;
        case SYS_CALPM_ARRANGE_OFFSET:
            if (p_duet2_calpm_master[lchip]->couple_mode && !p_duet2_calpm_master[lchip]->ipsa_enable)
            {
                offset = p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0] + sram_index * (CALPM_SRAM_RSV_FOR_ARRANGE +
                                p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX1]);
            }
            else
            {
                offset = p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0];
            }
            break;
        default:
            break;
    }

    return offset;
}

int32
_sys_duet2_calpm_alloc_key_offset(uint8 lchip, uint8 sram_index,
                                uint32 entry_num, uint32* p_offset)
{
    sys_usw_opf_t opf;
    int32 ret = CTC_E_NONE;
    uint32 sram_left;
    uint8 idx = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_offset);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_duet2_calpm_master[lchip]->couple_mode ? sram_index : 0;
    opf.pool_type = p_duet2_calpm_master[lchip]->opf_type_calpm;

    ret = sys_usw_opf_alloc_offset(lchip, &opf, entry_num, p_offset);
    if (ret != CTC_E_NONE)
    {
        if (p_duet2_calpm_master[lchip]->couple_mode)
        {
            sram_left = p_duet2_calpm_master[lchip]->max_stage_size[sram_index] - p_duet2_calpm_master[lchip]->calpm_usage[sram_index];
        }
        else
        {
            sram_left = p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0] - p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX0] -
                            p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX1];
        }

        if (sram_left >= entry_num)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "CALPM pipeline too many fragment, sram couple mode: %d, stage: %d, alloc size: %d, max calpm key size: %d, stage0 usage: %d, stage1 usage: %d\n",
                p_duet2_calpm_master[lchip]->couple_mode, sram_index, entry_num, p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0],
                p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX0], p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX1]);
            return CTC_E_TOO_MANY_FRAGMENT;
        }
        else
        {
            return ret;
        }
    }

    for (idx = 0; idx < CALPM_OFFSET_BITS_NUM; idx++)
    {
        if ((entry_num >> idx) & 1)
        {
            break;
        }
    }

    p_duet2_calpm_master[lchip]->calpm_usage[sram_index] += entry_num;
    p_duet2_calpm_master[lchip]->calpm_stats[sram_index][idx]++;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CALPM alloc key offset, stage: %d, start offset: %d, entry size: %d\n",
                     sram_index, *p_offset, entry_num);

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_alloc_key_offset_from_position(uint8 lchip, uint8 sram_index,
                                uint32 entry_num, uint32 offset)
{
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint8 idx = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_duet2_calpm_master[lchip]->couple_mode ? sram_index : 0;
    opf.pool_type = p_duet2_calpm_master[lchip]->opf_type_calpm;

    ret = sys_usw_opf_alloc_offset_from_position(lchip, &opf, entry_num, offset);
    if (ret)
    {
        return (ret == CTC_E_EXIST) ? CTC_E_NONE : ret;
    }

    for (idx = 0; idx < CALPM_OFFSET_BITS_NUM; idx++)
    {
        if ((entry_num >> idx) & 1)
        {
            break;
        }
    }

    p_duet2_calpm_master[lchip]->calpm_usage[sram_index] += entry_num;
    p_duet2_calpm_master[lchip]->calpm_stats[sram_index][idx]++;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CALPM alloc key offset from position, stage: %d, start offset: %d, entry size: %d\n",
                     sram_index, offset, entry_num);

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_free_key_offset(uint8 lchip, uint8 sram_index, uint32 entry_num, uint32 offset)
{
    sys_usw_opf_t opf;
    uint8 idx = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_duet2_calpm_master[lchip]->couple_mode ? sram_index : 0;
    opf.pool_type = p_duet2_calpm_master[lchip]->opf_type_calpm;

    if (offset == sys_duet2_calpm_get_extra_offset(lchip, sram_index, SYS_CALPM_ARRANGE_OFFSET))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, entry_num, offset));

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CALPM free key offset, stage: %d, start offset: %d, entry size: %d\n",
                     sram_index, offset, entry_num);

    for (idx = 0; idx < CALPM_OFFSET_BITS_NUM; idx++)
    {
        if ((entry_num >> idx) & 1)
        {
            break;
        }
    }

    p_duet2_calpm_master[lchip]->calpm_usage[sram_index] -= entry_num;
    p_duet2_calpm_master[lchip]->calpm_stats[sram_index][idx]--;

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_alloc_tbl(uint8 lchip, sys_calpm_tbl_t** pp_table, uint8 tbl_index)
{
    uint16 tbl_size = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    tbl_size = sizeof(sys_calpm_tbl_t);
    tbl_size = (tbl_index == CALPM_TABLE_INDEX0) ? tbl_size : CTC_OFFSET_OF(sys_calpm_tbl_t, p_ntable);

    *pp_table = mem_malloc(MEM_IPUC_MODULE, tbl_size);
    if (NULL == *pp_table)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(*pp_table, 0, tbl_size);

    (*pp_table)->stage_base = INVALID_POINTER_OFFSET;
    (*pp_table)->stage_index = tbl_index;

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_alloc_prefix(uint8 lchip, sys_calpm_param_t *p_calpm_param, sys_calpm_prefix_t** pp_calpm_prefix)
{
    int32 ret = CTC_E_NONE;
    sys_calpm_prefix_t* p_calpm_prefix = NULL;
    ctc_ipuc_param_t* p_ipuc_param = p_calpm_param->p_ipuc_param;
    uint8 prefix_len = p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver];

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    p_calpm_prefix = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_prefix_t));
    if (NULL == p_calpm_prefix)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
	return CTC_E_NO_MEMORY;
    }

    sal_memset(p_calpm_prefix, 0, sizeof(sys_calpm_prefix_t));
    p_calpm_prefix->ip_ver = p_ipuc_param->ip_ver;
    p_calpm_prefix->vrf_id = p_ipuc_param->vrf_id;
    p_calpm_prefix->prefix_len = prefix_len;
    p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] = INVALID_POINTER_OFFSET;
    p_calpm_prefix->key_index[CALPM_TYPE_POINTER] = INVALID_POINTER_OFFSET;

    if (CTC_IP_VER_6 == p_calpm_prefix->ip_ver)
    {
        p_calpm_prefix->ip32[0] = p_ipuc_param->ip.ipv6[3];
        p_calpm_prefix->ip32[1] = p_ipuc_param->ip.ipv6[2];
    }
    else
    {
        p_calpm_prefix->ip32[0] = p_ipuc_param->ip.ipv4;
    }

    if (p_ipuc_param->masklen > prefix_len)
    {
        ret = _sys_duet2_calpm_alloc_tbl(lchip, &(p_calpm_prefix->p_ntable), CALPM_TABLE_INDEX0);
        if (ret)
        {
            mem_free(p_calpm_prefix);
            return ret;
        }
    }

    *pp_calpm_prefix = p_calpm_prefix;

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_db_add(uint8 lchip, sys_calpm_prefix_t *p_calpm_prefix)
{
    char prefix_str[64] = {0};
    uint8 prefix_len = p_calpm_prefix->prefix_len;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
    {
        if (prefix_len == 8)
        {
            sal_sprintf(prefix_str, "%d", (p_calpm_prefix->ip32[0] >> 24) & 0xFF);
        }
        else
        {
            sal_sprintf(prefix_str, "%d.%d", (p_calpm_prefix->ip32[0] >> 24) & 0xFF,  (p_calpm_prefix->ip32[0] >> 16) & 0xFF);
        }
    }
    else
    {
        sal_sprintf(prefix_str, "%x:%x:%x", (p_calpm_prefix->ip32[0] >> 16) & 0xFFFF,  p_calpm_prefix->ip32[0] & 0xFFFF,
            (p_calpm_prefix->ip32[1] >> 16) & 0xFFFF);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add vrfid %d calpm prefix %s\r\n", p_calpm_prefix->vrf_id, prefix_str);

    ctc_hash_insert(p_duet2_calpm_master[lchip]->calpm_prefix[p_calpm_prefix->ip_ver], p_calpm_prefix);

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_db_remove(uint8 lchip, sys_calpm_prefix_t *p_calpm_prefix)
{
    char prefix_str[64] = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if ((INVALID_POINTER_OFFSET != p_calpm_prefix->key_index[CALPM_TYPE_POINTER]) ||
        (INVALID_POINTER_OFFSET != p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP]))
    {
        return CTC_E_NONE;
    }

    if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
    {
        if (p_calpm_prefix->prefix_len == 8)
        {
            sal_sprintf(prefix_str, "%d", (p_calpm_prefix->ip32[0] >> 24) & 0xFF);
        }
        else
        {
            sal_sprintf(prefix_str, "%d.%d", (p_calpm_prefix->ip32[0] >> 24) & 0xFF,  (p_calpm_prefix->ip32[0] >> 16) & 0xFF);
        }
    }
    else
    {
        sal_sprintf(prefix_str, "%x:%x:%x", (p_calpm_prefix->ip32[0] >> 16) & 0xFFFF,  p_calpm_prefix->ip32[0] & 0xFFFF,
            (p_calpm_prefix->ip32[1] >> 16) & 0xFFFF);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove vrfid %d calpm prefix %s\r\n", p_calpm_prefix->vrf_id, prefix_str);

    ctc_hash_remove(p_duet2_calpm_master[lchip]->calpm_prefix[p_calpm_prefix->ip_ver], p_calpm_prefix);
    mem_free(p_calpm_prefix);

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_lookup_prefix(uint8 lchip, sys_calpm_param_t *p_calpm_param)
{
    sys_calpm_prefix_t calpm_prefix = {0};
    ctc_ipuc_param_t *p_ipuc_param = p_calpm_param->p_ipuc_param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    calpm_prefix.vrf_id = p_ipuc_param->vrf_id;
    calpm_prefix.prefix_len = p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver];

    if (CTC_IP_VER_6 == p_ipuc_param->ip_ver)
    {
        calpm_prefix.ip32[0] = p_ipuc_param->ip.ipv6[3];
        calpm_prefix.ip32[1] = p_ipuc_param->ip.ipv6[2];
    }
    else
    {
        calpm_prefix.ip32[0] = p_ipuc_param->ip.ipv4;
    }

    p_calpm_param->p_calpm_prefix = ctc_hash_lookup(p_duet2_calpm_master[lchip]->calpm_prefix[p_ipuc_param->ip_ver], &calpm_prefix);

    if (p_calpm_param->p_calpm_prefix)
    {
        if ((p_calpm_param->p_calpm_prefix->p_ntable == NULL) && (p_ipuc_param->masklen > calpm_prefix.prefix_len))
        {
            CTC_ERROR_RETURN(_sys_duet2_calpm_alloc_tbl(lchip, &(p_calpm_param->p_calpm_prefix->p_ntable), CALPM_TABLE_INDEX0));
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_alloc_prefix_index(uint8 lchip,
                sys_calpm_param_t     *p_calpm_param,
                uint8                         *write_tcam_key)
{
    sys_calpm_prefix_t* p_calpm_prefix = NULL;
    sys_calpm_key_type_t key_type = CALPM_TYPE_POINTER;
    sys_ipuc_tcam_data_t tcam_data = {0};
    ctc_ipuc_param_t* p_ipuc_param = p_calpm_param->p_ipuc_param;
    int32 ret = CTC_E_NONE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_ipuc_param->masklen == p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        key_type = CALPM_TYPE_NEXTHOP;
    }

    CTC_ERROR_RETURN(_sys_duet2_calpm_lookup_prefix(lchip, p_calpm_param));
    if (!p_calpm_param->p_calpm_prefix)
    {
        CTC_ERROR_RETURN(_sys_duet2_calpm_alloc_prefix(lchip, p_calpm_param, &p_calpm_prefix));
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.key_index = p_calpm_param->key_index;
        tcam_data.masklen = p_ipuc_param->masklen;
        tcam_data.ipuc_param = p_ipuc_param;
        tcam_data.info = (void*)p_calpm_prefix;
        ret = sys_usw_ipuc_alloc_tcam_key_index(lchip, &tcam_data);
        if (ret)
        {
            if (p_calpm_prefix->p_ntable)
            {
                mem_free(p_calpm_prefix->p_ntable);
            }
            mem_free(p_calpm_prefix);

            return ret;
        }

        p_calpm_prefix->key_index[key_type] = tcam_data.key_index;
        p_calpm_param->p_calpm_prefix = p_calpm_prefix;
        *write_tcam_key = 1;
    }
    else
    {
        p_calpm_prefix = p_calpm_param->p_calpm_prefix;
        if (p_calpm_prefix->key_index[key_type] == INVALID_POINTER_OFFSET)
        {
            p_calpm_prefix->key_index[key_type] = p_calpm_prefix->key_index[!key_type];
        }
        *write_tcam_key = 0;

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "use exist tcam index %d\r\n", p_calpm_prefix->key_index[key_type]);
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_free_prefix_index(uint8 lchip,
                    sys_calpm_param_t* p_calpm_param)
{
    uint16 key_index = 0;
    sys_calpm_prefix_t* p_calpm_prefix = p_calpm_param->p_calpm_prefix;
    sys_calpm_tbl_t* p_tbl = NULL;
    sys_calpm_key_type_t key_type = CALPM_TYPE_POINTER;
    sys_ipuc_tcam_data_t tcam_data = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_calpm_param->p_ipuc_param->masklen == p_calpm_prefix->prefix_len)
    {
        key_type = CALPM_TYPE_NEXTHOP;
    }
    key_index = p_calpm_prefix->key_index[key_type];

    p_tbl = p_calpm_prefix->p_ntable;
    if (key_type == CALPM_TYPE_POINTER)
    {
        if (!p_tbl || !p_tbl[CALPM_TABLE_INDEX0].count)
        {
            p_calpm_prefix->key_index[key_type] = INVALID_POINTER_OFFSET;
        }
    }
    else
    {
        p_calpm_prefix->key_index[key_type] = INVALID_POINTER_OFFSET;
    }

    if ((INVALID_POINTER_OFFSET == p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP])
        && (INVALID_POINTER_OFFSET == p_calpm_prefix->key_index[CALPM_TYPE_POINTER]))
    {
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.key_index = key_index;
        tcam_data.masklen = p_calpm_param->p_ipuc_param->masklen;
        tcam_data.ipuc_param = p_calpm_param->p_ipuc_param;
        tcam_data.info = (void*)p_calpm_prefix;

        sys_usw_ipuc_free_tcam_key_index(lchip, &tcam_data);
    }

    return CTC_E_NONE;
}

#define __3_CALPM_HARD_WRITE__
int32
sys_duet2_calpm_enable_dma(uint8 lchip, uint8 enable)
{
    LCHIP_CHECK(lchip);
    SYS_CALPM_INIT_CHECK;
    p_duet2_calpm_master[lchip]->dma_enable = enable;

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calpm_get_stage_index_and_num(uint8 lchip, uint8 ip, uint8 mask, uint8 index_mask, uint8* offset, uint8* num)
{
    uint8 F = index_mask & mask;
    uint8 S = ip & F;
    uint8 B = F ^ index_mask;
    uint8 count = 0;
    uint8 i;

    uint8 BIT[8] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
    uint8 result = 0;

    for (i = 0; i < 8; i++)
    {
        if (BIT[i] & F)
        {
            result <<= 1;
            result += (S & BIT[i]) ? 1 : 0;
        }
    }

    while (B)
    {
        count++;
        B &= (B - 1);
    }

    *num = (1 << count);
    *offset = (result << count);

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calpm_asic_write_calpm(uint8 lchip, DsLpmLookupKey_m* key, uint16 start_offset, uint16 entry_size)
{
    uint32 cmd, cmd1;
    uint16 i = 0;
    drv_work_platform_type_t platform_type = 0;
    sys_dma_tbl_rw_t tbl_cfg;
    tbls_id_t tbl_id, tbl_id1;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&tbl_cfg, 0, sizeof(tbl_cfg));
    cmd = DRV_IOW(DsLpmLookupKey_t, DRV_ENTRY_FLAG);
    cmd1 = DRV_IOW(DsLpmLookupKey1_t, DRV_ENTRY_FLAG);

    drv_get_platform_type(lchip, &platform_type);

    if ((platform_type == HW_PLATFORM) && (p_duet2_calpm_master[lchip]->dma_enable))
    {
        tbl_id = DRV_IOC_MEMID(cmd);
        tbl_id1 = DRV_IOC_MEMID(cmd1);
        CTC_ERROR_RETURN(drv_usw_table_get_hw_addr(lchip, tbl_id, start_offset, &tbl_cfg.tbl_addr, 0));
        tbl_cfg.buffer = (uint32*)key;
        tbl_cfg.entry_num = entry_size;
        tbl_cfg.entry_len = sizeof(DsLpmLookupKey_m);
        tbl_cfg.rflag = 0;
        sys_usw_dma_rw_table(lchip, &tbl_cfg);

        if (p_duet2_calpm_master[lchip]->ipsa_enable)
        {
            CTC_ERROR_RETURN(drv_usw_table_get_hw_addr(lchip, tbl_id1, start_offset, &tbl_cfg.tbl_addr, 0));
            sys_usw_dma_rw_table(lchip, &tbl_cfg);
        }
    }
    else
    {
        for (i = 0; i < entry_size; i++)
        {
            DRV_IOCTL(lchip, start_offset + i, cmd, key);
            if (p_duet2_calpm_master[lchip]->ipsa_enable)
            {
                DRV_IOCTL(lchip, start_offset + i, cmd1, key);
            }

            key ++;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_write_stage_key(uint8 lchip,
                sys_calpm_param_t    *p_calpm_param,
                uint8 stage)
{
    DsLpmLookupKey_m* ds_calpm_lookup_key = NULL;
    sys_calpm_tbl_t* p_table = NULL;
    sys_calpm_tbl_t* p_ntbl = NULL;
    sys_calpm_item_t* p_item = NULL;
    uint8 offset, item_size, ip, calpm_stage;
    uint16 i, j, k, idx, entry_size;
    uint32 type = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_calpm_param->p_ipuc_param->masklen == p_calpm_param->p_calpm_prefix->prefix_len)
    {
        return CTC_E_NONE;
    }

    ds_calpm_lookup_key = mem_malloc(MEM_IPUC_MODULE, sizeof(DsLpmLookupKey_m)*256);
    if (NULL == ds_calpm_lookup_key)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    for (calpm_stage = CALPM_TABLE_INDEX0; calpm_stage < CALPM_TABLE_MAX; calpm_stage++)
    {
        if ((stage != calpm_stage) && (CALPM_TABLE_MAX != stage))
        {
            continue;
        }

        /*write new calpm key*/
        p_table = TABEL_GET_STAGE(calpm_stage, p_calpm_param);
        if (p_table == NULL || (0 == p_table->count))
        {
            continue;
        }

        CALPM_IDX_MASK_TO_SIZE(p_table->idx_mask, entry_size);
        sal_memset(ds_calpm_lookup_key, 0, sizeof(DsLpmLookupKey_m) * entry_size);

        for (i = 0; i < CALPM_TBL_NUM; i++)
        {
            p_ntbl = TABLE_GET_PTR(p_table, i);
            if (NULL == p_ntbl)
            {
                continue;
            }

            /* save all pointer node to ds_calpm_lookup_key */
            if (p_ntbl != p_table)
            {
                _sys_duet2_calpm_get_stage_index_and_num(lchip, i, 0xff, p_table->idx_mask, &offset, &item_size);

                /* the pointer value is the relative calpm lookup key address
                 * and p_table->n_table[i]->pointer is the absolute adress
                 * since calpm pipeline lookup mechanism,
                 * the address should use relative address
                 * the code added here suppose ipv4
                 */
                GetDsLpmLookupKey(A, type0_f, &ds_calpm_lookup_key[offset], &type);

                if (CALPM_LOOKUP_KEY_TYPE_EMPTY == type)
                {
                    SetDsLpmLookupKey(V, mask0_f, &ds_calpm_lookup_key[offset], p_ntbl->idx_mask);
                    SetDsLpmLookupKey(V, ip0_f, &ds_calpm_lookup_key[offset], i);
                    SetDsLpmLookupKey(V, nexthop0_f, &ds_calpm_lookup_key[offset], p_ntbl->stage_base);
                    SetDsLpmLookupKey(V, type0_f, &ds_calpm_lookup_key[offset], CALPM_LOOKUP_KEY_TYPE_POINTER);
                }
                else
                {
                    SetDsLpmLookupKey(V, mask1_f, &ds_calpm_lookup_key[offset], p_ntbl->idx_mask);
                    SetDsLpmLookupKey(V, ip1_f, &ds_calpm_lookup_key[offset], i);
                    SetDsLpmLookupKey(V, nexthop1_f, &ds_calpm_lookup_key[offset], p_ntbl->stage_base);
                    SetDsLpmLookupKey(V, type1_f, &ds_calpm_lookup_key[offset], CALPM_LOOKUP_KEY_TYPE_POINTER);
                }

                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                                 "write DsLpmLookupKey%d[%d], ip: 0x%x, pointer: 0x%X   \r\n",
                                 p_table->stage_index, p_table->stage_base+offset, i, p_ntbl->stage_base);
            }
        }

        /* save all nexthop node to ds_calpm_lookup_key */
        for (i = CALPM_MASK_LEN; i > 0; i--)
        {
            for (j = 0; j <= GET_MASK_MAX(i); j++)
            {
                idx = GET_BASE(i) + j;
                p_item = ITEM_GET_PTR(p_table, idx);
                if ((NULL == p_item) || (p_item->t_masklen <= 0)
                    || (!CTC_FLAG_ISSET(p_item->item_flag, CALPM_ITEM_FLAG_VALID)))
                {
                    continue;
                }

                ip = (j << (CALPM_MASK_LEN - i));
                _sys_duet2_calpm_get_stage_index_and_num(lchip, ip, GET_MASK(i), p_table->idx_mask, &offset, &item_size);

                for (k = 0; k < item_size; k++)
                {
                    GetDsLpmLookupKey(A, type0_f, &ds_calpm_lookup_key[offset + k], &type);
                    if (CALPM_LOOKUP_KEY_TYPE_EMPTY == type)
                    {
                        SetDsLpmLookupKey(V, mask0_f, &ds_calpm_lookup_key[offset+k], GET_MASK(i));
                        SetDsLpmLookupKey(V, ip0_f, &ds_calpm_lookup_key[offset+k], ip);
                        SetDsLpmLookupKey(V, nexthop0_f, &ds_calpm_lookup_key[offset+k], p_item->ad_index);
                        SetDsLpmLookupKey(V, type0_f, &ds_calpm_lookup_key[offset+k], CALPM_LOOKUP_KEY_TYPE_NEXTHOP);
                    }
                    else
                    {
                        SetDsLpmLookupKey(V, mask1_f, &ds_calpm_lookup_key[offset+k], GET_MASK(i));
                        SetDsLpmLookupKey(V, ip1_f, &ds_calpm_lookup_key[offset+k], ip);
                        SetDsLpmLookupKey(V, nexthop1_f, &ds_calpm_lookup_key[offset+k], p_item->ad_index);
                        SetDsLpmLookupKey(V, type1_f, &ds_calpm_lookup_key[offset+k], CALPM_LOOKUP_KEY_TYPE_NEXTHOP);
                    }
                }

                if (item_size == 1)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                                     "write DsLpmLookupKey%d[%d], ip: 0x%x, nexthop: 0x%x  \r\n",
                                     p_table->stage_index, p_table->stage_base+offset, ip, p_item->ad_index);
                }
                else
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                                     "write DsLpmLookupKey%d[%d-%d], ip: 0x%x, nexthop: 0x%x  \r\n",
                                     p_table->stage_index, p_table->stage_base+offset, p_table->stage_base+offset+item_size-1, ip, p_item->ad_index);
                }
            }
        }

        /* notice */
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "write calpm lookup key start offset: %d write entry size: %d\n", p_table->stage_base, entry_size);

        _sys_duet2_calpm_asic_write_calpm(lchip, ds_calpm_lookup_key, p_table->stage_base, entry_size);
    }

    mem_free(ds_calpm_lookup_key);

    return CTC_E_NONE;
}

#define __4_CALPM_TREE__

STATIC int32
_sys_duet2_calpm_add_tree(uint8 lchip, sys_calpm_tbl_t* p_table, uint8 stage_ip, uint8 masklen, sys_calpm_param_t* p_calpm_param)
{
    uint8 offset = 0;
    uint8 depth = 0;
    uint8 pa_loop = 0;
    uint8 ch_loop = 0;
    uint8 valid_parent_masklen = 0;
    uint16 real_idx = 0;
    uint16 child_idx = 0;
    uint16 parent_idx = 0;
    uint16 valid_parent_idx = 0;
    sys_calpm_item_t* p_real_item = NULL;
    sys_calpm_item_t* p_child_item = NULL;
    sys_calpm_item_t* p_parent_item = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (!masklen)
    {
        return CTC_E_NONE;
    }

    real_idx = SYS_CALPM_GET_ITEM_IDX(stage_ip, masklen);   /* the idx is the location of the route */
    p_real_item = ITEM_GET_OR_INIT_PTR(p_table, real_idx);
    if (NULL == p_real_item)
    {
        return CTC_E_NOT_EXIST;
    }

    valid_parent_idx = real_idx;
    valid_parent_masklen = masklen;

    /* set this node */
    p_real_item->ad_index = p_calpm_param->ad_index;
    p_real_item->t_masklen = masklen;
    CTC_SET_FLAG(p_real_item->item_flag, CALPM_ITEM_FLAG_REAL);
    CTC_SET_FLAG(p_real_item->item_flag, CALPM_ITEM_FLAG_VALID);
    SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "node[%d] is real node, mask: %d, ad_index: %d!\n", real_idx, p_real_item->t_masklen, p_real_item->ad_index);

    if (masklen > 1)
    {
        /* find last parent valid node */
        for (depth = masklen; depth > 1; depth--)
        {
            valid_parent_idx = SYS_CALPM_GET_PARENT_ITEM_IDX(stage_ip, depth);
            p_parent_item = ITEM_GET_OR_INIT_PTR(p_table, valid_parent_idx);
            if (!p_parent_item)
            {
                return CTC_E_NOT_EXIST;
            }

            CTC_SET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_PARENT);
            SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "node[%d] set parent flag!\n", valid_parent_idx);

            if (CTC_FLAG_ISSET(p_parent_item->item_flag, CALPM_ITEM_FLAG_VALID))
            {
                break;
            }
        }

        /* found valid parent node */
        if ((depth >= 2) && CTC_FLAG_ISSET(p_parent_item->item_flag, CALPM_ITEM_FLAG_VALID))
        {
            valid_parent_masklen = depth -1;
        }
        else
        {
            valid_parent_idx = real_idx;
            valid_parent_masklen = masklen;
        }
    }

    /* push sub route of last valid parent node */
    for (depth = valid_parent_masklen; depth < 8; depth++)
    {
        offset = (valid_parent_idx - GET_BASE(valid_parent_masklen)) * (1 << (depth - valid_parent_masklen));
        for (pa_loop = 0; pa_loop <= GET_MASK_MAX(depth - valid_parent_masklen); pa_loop++)
        {
            parent_idx = GET_BASE(depth) + offset + pa_loop;
            p_parent_item = ITEM_GET_PTR(p_table, parent_idx);

            if (p_parent_item && CTC_FLAG_ISSET(p_parent_item->item_flag, CALPM_ITEM_FLAG_PARENT))
            {
                SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "push parent node[%d] flag: 0x%x!\n", parent_idx, p_parent_item->item_flag);

                for (ch_loop = 0; ch_loop < 2; ch_loop++)
                {
                    child_idx = GET_BASE(depth+1) + (offset + pa_loop) * 2 + ch_loop;
                    p_child_item = ITEM_GET_OR_INIT_PTR(p_table, child_idx);
                    if (!p_child_item)
                    {
                        return CTC_E_NOT_EXIST;
                    }

                    CTC_SET_FLAG(p_child_item->item_flag, CALPM_ITEM_FLAG_VALID);

                    if (p_child_item->t_masklen < p_parent_item->t_masklen)
                    {
                        p_child_item->t_masklen = p_parent_item->t_masklen;
                        p_child_item->ad_index = p_parent_item->ad_index;
                        SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "push down child node[%d] masklen: %d, ad index: %d.\n", child_idx, p_child_item->t_masklen, p_child_item->ad_index);
                    }
                }

                if (!CTC_FLAG_ISSET(p_parent_item->item_flag, CALPM_ITEM_FLAG_REAL))
                {
                    p_parent_item->t_masklen = 0;
                    p_parent_item->ad_index = INVALID_NEXTHOP_OFFSET;
                }
                CTC_UNSET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_VALID);
                SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "clear parent node[%d] valid flag, masklen: %d, ad index: %d.\n", parent_idx, p_parent_item->t_masklen, p_parent_item->ad_index);
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calpm_del_tree(uint8 lchip, sys_calpm_tbl_t* p_table, uint8 stage_ip, uint8 masklen)
{
    uint8 offset = 0;
    uint8 depth = 0;
    uint8 depth_valid_ct = 0;
    uint8 last_valid_ct = 0;
    uint16 ch_loop = 0;
    uint16 real_idx = 0;
    uint16 child0_idx = 0;
    uint16 child1_idx = 0;
    uint16 parent_idx = 0;
    uint32 real_ad_index = 0;
    sys_calpm_item_t* p_replace_item = NULL;
    sys_calpm_item_t* p_real_item = NULL;
    sys_calpm_item_t* p_child0_item = NULL;
    sys_calpm_item_t* p_child1_item = NULL;
    sys_calpm_item_t* p_parent_item = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    real_idx = SYS_CALPM_GET_ITEM_IDX(stage_ip, masklen);   /* the idx is the location of the route */
    child1_idx = (real_idx % 2) ? (real_idx - 1) : (real_idx + 1);
    parent_idx = SYS_CALPM_GET_PARENT_ITEM_IDX(stage_ip, masklen);

    p_real_item = ITEM_GET_PTR(p_table, real_idx);
    p_child1_item = ITEM_GET_PTR(p_table, child1_idx);

    if (!p_real_item)
    {
        return CTC_E_NOT_EXIST;
    }
    real_ad_index = p_real_item->ad_index;

    /* unset this node */
    CTC_UNSET_FLAG(p_real_item->item_flag, CALPM_ITEM_FLAG_REAL);

    /* judge whether or not push up when the real node is deleted */
    if (!CTC_FLAG_ISSET(p_real_item->item_flag, CALPM_ITEM_FLAG_PARENT) &&
        (p_child1_item && (p_child1_item->t_masklen < masklen) &&
        CTC_FLAG_ISSET(p_child1_item->item_flag, CALPM_ITEM_FLAG_VALID)))
    {
        p_parent_item = ITEM_GET_OR_INIT_PTR(p_table, parent_idx);
        if (!p_parent_item)
        {
            return CTC_E_NOT_EXIST;
        }

        p_parent_item->t_masklen = p_child1_item->t_masklen;
        p_parent_item->ad_index = p_child1_item->ad_index;
        CTC_SET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_VALID);
        CTC_UNSET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_PARENT);
        SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free real node[%d] masklen: %d, ad_index: %d\n\tfree brother node[%d] masklen: %d, ad_index: %d\n\tpush up to parent node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n",
            real_idx, p_real_item->t_masklen, p_real_item->ad_index, child1_idx, p_child1_item->t_masklen, p_child1_item->ad_index, parent_idx, p_parent_item->t_masklen, p_parent_item->ad_index, p_parent_item->item_flag);

        ITEM_FREE_PTR(p_table, p_real_item);
        ITEM_FREE_PTR(p_table, p_child1_item);

        for (depth = masklen - 1; depth > 1; depth--)
        {
            child0_idx = SYS_CALPM_GET_ITEM_IDX(stage_ip, depth);
            child1_idx = (child0_idx % 2) ? (child0_idx - 1) : (child0_idx + 1);
            parent_idx = SYS_CALPM_GET_PARENT_ITEM_IDX(stage_ip, depth);

            p_child0_item = ITEM_GET_PTR(p_table, child0_idx);
            p_child1_item = ITEM_GET_PTR(p_table, child1_idx);

            if (p_child0_item && p_child1_item &&
                (p_child0_item->t_masklen == p_child1_item->t_masklen) &&
                (p_child0_item->ad_index == p_child1_item->ad_index) &&
                (p_child0_item->item_flag == p_child1_item->item_flag) &&
                (p_child0_item->t_masklen < depth))
            {
                    p_parent_item = ITEM_GET_OR_INIT_PTR(p_table, parent_idx);
                    if (!p_parent_item)
                    {
                        return CTC_E_NOT_EXIST;
                    }

                    p_parent_item->t_masklen = p_child1_item->t_masklen;
                    p_parent_item->ad_index = p_child1_item->ad_index;
                    CTC_SET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_VALID);
                    CTC_UNSET_FLAG(p_parent_item->item_flag, CALPM_ITEM_FLAG_PARENT);

                    SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free child0 node[%d] masklen: %d, ad_index: %d\n\tfree child1 node[%d] masklen: %d, ad_index: %d\n\tpush up to parent node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n",
                        child0_idx, p_child0_item->t_masklen, p_child0_item->ad_index, child1_idx, p_child1_item->t_masklen, p_child1_item->ad_index, parent_idx, p_parent_item->t_masklen, p_parent_item->ad_index, p_parent_item->item_flag);

                    ITEM_FREE_PTR(p_table, p_child0_item);
                    ITEM_FREE_PTR(p_table, p_child1_item);
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        /* find last valid or real parent node */
        for (depth = masklen; depth > 1; depth--)
        {
            parent_idx = SYS_CALPM_GET_PARENT_ITEM_IDX(stage_ip, depth);
            p_replace_item = ITEM_GET_PTR(p_table, parent_idx);

            if (p_replace_item && (CTC_FLAG_ISSET(p_replace_item->item_flag, CALPM_ITEM_FLAG_VALID) ||
                    CTC_FLAG_ISSET(p_replace_item->item_flag, CALPM_ITEM_FLAG_REAL)))
            {
                SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "found parent node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n", parent_idx, p_replace_item->t_masklen, p_replace_item->ad_index, p_replace_item->item_flag);
                break;
            }
            else
            {
                p_replace_item = NULL;
            }
        }

        if (!CTC_FLAG_ISSET(p_real_item->item_flag, CALPM_ITEM_FLAG_PARENT))
        {/* the deleted real node don't have child node */
            if (p_replace_item)
            {
                p_real_item->t_masklen = p_replace_item->t_masklen;
                p_real_item->ad_index = p_replace_item->ad_index;
                CTC_SET_FLAG(p_real_item->item_flag, CALPM_ITEM_FLAG_VALID);
                SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "replace real node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n", real_idx, p_real_item->t_masklen, p_real_item->ad_index, p_real_item->item_flag);
            }
            else
            {
                SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free real node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n", real_idx, p_real_item->t_masklen, p_real_item->ad_index, p_real_item->item_flag);
                ITEM_FREE_PTR(p_table, p_real_item);
            }
        }
        else
        {
            /* the deleted real node has child node, replace the pushed node by parent node*/
            for (depth = masklen; depth <= CALPM_MASK_LEN; depth++)
            {
                depth_valid_ct = 0;
                offset = SYS_CALPM_ITEM_OFFSET(stage_ip, depth);
                for (ch_loop = 0; ch_loop <= GET_MASK_MAX(depth - masklen); ch_loop++)
                {
                    child0_idx = GET_BASE(depth) + offset + ch_loop;
                    p_child0_item = ITEM_GET_PTR(p_table, child0_idx);

                    if (!p_child0_item)
                    {
                        continue;
                    }

                    if (CTC_FLAG_ISSET(p_child0_item->item_flag, CALPM_ITEM_FLAG_VALID))
                    {
                        depth_valid_ct++;
                    }

                    /* find the pushed node of the deleted real node */
                    if ((p_child0_item->ad_index == real_ad_index) &&
                         (p_child0_item->t_masklen == masklen))
                     {  /* such as real node */
                        if (!CTC_FLAG_ISSET(p_child0_item->item_flag, CALPM_ITEM_FLAG_VALID) && !CTC_FLAG_ISSET(p_child0_item->item_flag, CALPM_ITEM_FLAG_PARENT))
                        {
                            SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free invalid node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n", child0_idx, p_real_item->t_masklen, p_real_item->ad_index, p_real_item->item_flag);
                            ITEM_FREE_PTR(p_table, p_child0_item);

                            return CTC_E_NONE;
                        }  /* don't have parent node, release all pushed node directly */
                        else if (!p_replace_item && !CTC_FLAG_ISSET(p_child0_item->item_flag, CALPM_ITEM_FLAG_PARENT))
                        {
                            SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free pushed node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n", child0_idx, p_real_item->t_masklen, p_real_item->ad_index, p_real_item->item_flag);
                            ITEM_FREE_PTR(p_table, p_child0_item);
                        }  /* have parent node, replace all pushed node by parent node */
                        else if (p_replace_item)
                        {
                            p_child0_item->t_masklen = p_replace_item->t_masklen;
                            p_child0_item->ad_index = p_replace_item->ad_index;
                            SYS_CALPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "replace node[%d] mask: %d, ad_index: %d, flag: 0x%x!\n",
                                    child0_idx, p_child0_item->t_masklen, p_child0_item->ad_index, p_child0_item->item_flag);
                        } /* only parent node */
                        else if (CTC_FLAG_ISSET(p_child0_item->item_flag, CALPM_ITEM_FLAG_PARENT))
                        {
                            p_child0_item->t_masklen = 0;
                            p_child0_item->ad_index = INVALID_NEXTHOP_OFFSET;
                        }
                    }

                    /* lookup end when valid count of current depth add push down count of last depth, note: invalid parent but not real node will bel free when add tree */
                    if ((last_valid_ct * 2 + depth_valid_ct) == (GET_MASK_MAX(depth - masklen) + 1))
                    {
                        return CTC_E_NONE;
                    }
                }
                last_valid_ct = last_valid_ct * 2 + depth_valid_ct;
            }
        }
    }

    return CTC_E_NONE;
}

/* only update calpm ad_index */
int32
_sys_duet2_calpm_update_tree_ad(uint8 lchip, sys_calpm_param_t* p_calpm_param, uint16* old_ad_index)
{
    sys_calpm_tbl_t* p_table = NULL;  /* root node */
    sys_calpm_item_t* p_item = NULL;
    ctc_ipuc_param_t* p_ipuc_param = p_calpm_param->p_ipuc_param;
    uint32* ip = NULL;
    uint16 idx = 0;
    uint8 prefix_len = 0;
    uint8 masklen = 0;
    uint8 stage = 0;
    uint8 stage_ip = 0;
    uint8 offset = 0;
    uint8 depth = 0;
    uint8 loop = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    prefix_len = p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver];
    ip = (uint32*)&p_ipuc_param->ip;
    masklen = p_ipuc_param->masklen - prefix_len;
    if (!masklen)
    {
        return CTC_E_NONE;
    }

    stage = (masklen > 8) ? 1 : 0;

    masklen -= stage * 8;
    stage_ip = _sys_duet2_calpm_get_stage_ip(lchip, ip, p_ipuc_param->ip_ver, stage);

    /* get sw root node by prefix */
    p_table = TABEL_GET_STAGE(stage, p_calpm_param);
    if (!p_table)
    {
        return CTC_E_NOT_EXIST;
    }

    /* 2. update ad_index */
    idx = SYS_CALPM_GET_ITEM_IDX(stage_ip, masklen);   /* the idx is the location of the route */
    p_item = ITEM_GET_PTR(p_table, idx);
    if (!p_item)
    {
        return CTC_E_NOT_EXIST;
    }

    p_item->ad_index = p_calpm_param->ad_index;

    /* 3. fill all sub push down node */
    for (depth = masklen + 1; depth <= CALPM_MASK_LEN; depth++)
    {
        offset = SYS_CALPM_ITEM_OFFSET(stage_ip, depth);
        for (loop = 0; loop <= GET_MASK_MAX(depth - masklen); loop++)
        {
            idx = GET_BASE(depth) + loop + offset;
            p_item = ITEM_GET_PTR(p_table, idx);
            if (!p_item)
            {
                continue;
            }

            if (p_item->t_masklen == masklen)
            {
                if (old_ad_index)
                {
                    *old_ad_index = p_item->ad_index;
                }
                p_item->ad_index = p_calpm_param->ad_index;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_free_tree(uint8 lchip, sys_calpm_tbl_t* p_table)
{
    sys_calpm_item_t* p_item;
    uint8 i;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (NULL == p_table)
    {
        return CTC_E_NONE;
    }

    for (i = 0; i < CALPM_ITEM_ALLOC_NUM; i++)
    {
        p_item = p_table->p_item[i];

        while (p_item)
        {
            p_table->p_item[i] = p_table->p_item[i]->p_nitem;
            mem_free(p_item);
            p_item = p_table->p_item[i];
        }
    }

    return CTC_E_NONE;
}

/* write a ip to the tree, and push down */
STATIC int32
_sys_duet2_calpm_add_soft(uint8 lchip, sys_calpm_param_t* p_calpm_param, sys_calpm_tbl_t** p_table)
{
    uint8 stage_ip = 0;
    uint8 stage = 0;
    uint8 masklen = 0;
    uint8 end = CALPM_TABLE_INDEX1;
    uint32* ip = NULL;
    sys_calpm_tbl_t* p_ntbl = NULL;
    ctc_ipuc_param_t* p_ipuc_param = p_calpm_param->p_ipuc_param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip = (uint32*)&p_ipuc_param->ip;
    masklen = p_ipuc_param->masklen - p_calpm_param->p_calpm_prefix->prefix_len;
    if (!masklen)
    {
        return CTC_E_NONE;
    }

    for (stage = CALPM_TABLE_INDEX0; stage <= end; stage++)
    {
        stage_ip = _sys_duet2_calpm_get_stage_ip(lchip, ip, p_ipuc_param->ip_ver, stage);

        if (masklen > 8)
        {
            masklen -= 8;

            p_ntbl = TABLE_GET_PTR(p_table[stage], stage_ip);
            if (NULL == p_ntbl)
            {
                CTC_ERROR_RETURN(_sys_duet2_calpm_alloc_tbl(lchip, &p_table[stage + 1], (stage + 1)));
                TABLE_SET_PTR(p_table[stage]->p_ntable, stage_ip, p_table[stage + 1]);
                p_table[stage]->count++;
            }
            else
            {
                p_table[stage + 1] = p_ntbl;
            }

            continue;
        }

        /* push down */
        CTC_ERROR_RETURN(_sys_duet2_calpm_add_tree(lchip, p_table[stage], stage_ip, masklen, p_calpm_param));

        p_table[stage]->count++;
        break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calpm_del_soft(uint8 lchip, sys_calpm_param_t* p_calpm_param, sys_calpm_tbl_t** p_table)
{
    int8 stage = 0;
    uint8 stage_ip = 0;
    uint8 masklen = 0;
    uint8 masklen_t = 0;
    uint16 idx = 0;
    uint32* ip = NULL;
    sys_calpm_tbl_t* p_ntbl = NULL;
    ctc_ipuc_param_t* p_ipuc_param = p_calpm_param->p_ipuc_param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip = (uint32*)&p_ipuc_param->ip;
    masklen = p_ipuc_param->masklen - p_calpm_param->p_calpm_prefix->prefix_len;
    if (!masklen)
    {
        return CTC_E_NONE;
    }

    for (stage = CALPM_TABLE_INDEX1; stage >= (int8)CALPM_TABLE_INDEX0; stage--)
    {
        if (NULL == p_table[stage])
        {
            continue;
        }

        stage_ip = _sys_duet2_calpm_get_stage_ip(lchip, ip, p_ipuc_param->ip_ver, stage);

        masklen_t = masklen - 8 * stage;
        if (masklen_t > 8)
        {
            if ((stage == CALPM_TABLE_INDEX0) && p_table[stage + 1] && (0 == p_table[stage + 1]->count))
            {
                p_ntbl = TABLE_GET_PTR(p_table[stage], stage_ip);
                if (NULL == p_ntbl)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ERROR: stage[%d] stage_ip %d pointer is NULL!\n", stage, stage_ip);
                    return CTC_E_NOT_EXIST;
                }

                _sys_duet2_calpm_free_tree(lchip, p_ntbl);

                mem_free(p_ntbl);
                TABLE_FREE_PTR(p_table[stage]->p_ntable, stage_ip);
                p_table[stage + 1] = NULL;
                p_table[stage]->count--;
            }

            continue;
        }

        _sys_duet2_calpm_del_tree(lchip, p_table[stage], stage_ip, masklen_t);

        p_table[stage]->count--;
    }

    if (!p_table[CALPM_TABLE_INDEX0]->count)
    {
        _sys_duet2_calpm_free_tree(lchip, p_table[CALPM_TABLE_INDEX0]);

        for (idx = 0; idx < CALPM_TBL_NUM; idx++)
        {
            p_ntbl = TABLE_GET_PTR(p_table[CALPM_TABLE_INDEX0], idx);
            if (p_ntbl)
            {
                TABLE_FREE_PTR(p_table[CALPM_TABLE_INDEX0]->p_ntable, idx);
                mem_free(p_ntbl);
            }
        }
        mem_free(p_table[CALPM_TABLE_INDEX0]);
        p_calpm_param->p_calpm_prefix->p_ntable = NULL;
    }

    return CTC_E_NONE;
}

STATIC void
_sys_duet2_calpm_update_select_counter(uint8 lchip, sys_calpm_select_counter_t* select_counter,
                                     uint8 ip, uint8 mask, uint8 test_mask)
{
    uint8 next = 0;
    uint8 number = 0;
    uint8 index = 0;

    for (index = 0; index <= 7; index++)
    {
        if (IS_BIT_SET(test_mask, index))
        {
            if (IS_BIT_SET(ip, index) && IS_BIT_SET(mask, index))
            {
                SET_BIT(select_counter->block_base, next);
            }

            next++;

            /* One X bit index Occupy  2 ds_ip_calpm_prefix entries in one bucket */
            if (!IS_BIT_SET(mask, index))
            {
                /* Per bucket_number add 2 ds_ip_calpm_prefix entry which totally 512 entries, max value is 7 */
                number++;
            }
        }
    }

    select_counter->entry_number = (1 << (number & 0x7));
    /* 1^ bucket_number*/
}

STATIC int32
_sys_duet2_calpm_calc_compress_index(uint8 lchip, sys_calpm_stage_t* p_calpm_stage,
                                    uint8* new_test_mask,
                                    uint8 test_mask_start_index,
                                    uint16 prefix_count,
                                    uint8 test_mask_end_index,
                                    uint8 old_test_mask)
{
    uint8     ip = 0;
    uint8     mask = 0;
    uint8     test_mask = 0;
    uint8     i = 0;
    uint16    key_index = 0;
    uint32*   counter = NULL;
    sys_calpm_select_counter_t select_counter = {0};
    uint8    conflict = FALSE;
    uint16   test_mask_index = test_mask_start_index;

    counter = (uint32*)mem_malloc(MEM_IPUC_MODULE, sizeof(uint32)*(DS_CALPM_PREFIX_MAX_INDEX / CALPM_COMPRESSED_BLOCK_SIZE));
    if (NULL == counter)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    /* since in worst condition, the last index mask is determined to 255, so just need 255 times index test */
    for (; test_mask_index <= 255; test_mask_index++)
    {
        /* compute index confilct summaries for all DsLpmPrefixTable entries one by one using this test index mask */
        if (test_mask_end_index == test_mask_index)
        {
            *new_test_mask = old_test_mask;
            mem_free(counter);
            return DRV_E_NONE;
        }

        test_mask = ds_calpm_test_mask_table[test_mask_index];
         /*SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "loop: %d, test mask: 0x%x\n", test_mask_index, test_mask);*/

        sal_memset(counter, 0, sizeof(uint32)*(DS_CALPM_PREFIX_MAX_INDEX / CALPM_COMPRESSED_BLOCK_SIZE));

        for (key_index = 0; key_index < prefix_count; key_index++)
        {
            ip = p_calpm_stage[key_index].ip;
            mask = p_calpm_stage[key_index].mask;

            select_counter.block_base = 0;
            select_counter.entry_number = 0;
            _sys_duet2_calpm_update_select_counter(lchip, &select_counter, ip, mask, test_mask);
             /*SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ip: 0x%x, mask: 0x%x, counter base: %d, entry_count: %d\n", ip, mask, select_counter.block_base, select_counter.entry_number);*/

            conflict = FALSE;

            for (i = 0; i < select_counter.entry_number; i++)
            {
                /* a counter present 2 tuple in one block */
                counter[select_counter.block_base + i]++;
                 /*SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "incr counter[%d]: %d\n", select_counter.block_base + i, counter[select_counter.block_base + i]);*/

                if (counter[select_counter.block_base + i] > CALPM_COMPRESSED_BLOCK_SIZE)
                {
                     /*SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "conflict index: %d\n", select_counter.block_base + i);*/
                    conflict = TRUE;
                    break;
                }
            }

            if (conflict)
            {
                break;
            }
        }

        /* All ip_calpm_prefix known using this index_mask do not conflict with each other */
        if (!conflict)
        {
            *new_test_mask = test_mask;
            break;
        }
    }

    mem_free(counter);
    return DRV_E_NONE;
}

STATIC uint8
_sys_duet2_calpm_get_compress_index(uint8 lchip, sys_calpm_tbl_t* p_table)
{
    uint8 nbit = 0;
    uint8 depth = 0;
    uint8 idx_mask = 0;
    uint16 index = 0;
    uint16 item_idx = 0;
    uint16 count = 0;
    sys_calpm_item_t* p_item;
    sys_calpm_tbl_t* p_ntbl;
    sys_calpm_stage_t* p_calpm_stage = NULL;

    p_calpm_stage = (sys_calpm_stage_t*)mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_stage_t)*DS_CALPM_PREFIX_MAX_INDEX);
    if (NULL == p_calpm_stage)
    {
        return 0;
    }
    sal_memset(p_calpm_stage, 0, sizeof(sys_calpm_stage_t)*DS_CALPM_PREFIX_MAX_INDEX);
    for (index = 0; index < CALPM_TBL_NUM; index++)
    {
        p_ntbl = TABLE_GET_PTR(p_table, index);
        if (p_ntbl)
        {
            p_calpm_stage[count].mask = 0xFF;
            p_calpm_stage[count].ip = index;
            count++;
        }
    }

    for (depth = CALPM_MASK_LEN; depth > 0; depth--)
    {
        for (index = 0; index <= GET_MASK_MAX(depth); index++)
        {
            item_idx = GET_BASE(depth) + index;
            p_item = ITEM_GET_PTR(p_table, item_idx);
            if ((NULL != p_item) && p_item->t_masklen > 0 &&
                CTC_FLAG_ISSET(p_item->item_flag, CALPM_ITEM_FLAG_VALID))
            {
                p_calpm_stage[count].mask = GET_MASK(depth);
                p_calpm_stage[count].ip = (index << (CALPM_MASK_LEN - depth));
                count++;

                if (count > 256)
                {
                    mem_free(p_calpm_stage);
                    return 0xff;
                }
            }
        }
    }

    for (nbit = 7; nbit > 0; nbit--)
    {
        if (count > (1 << nbit))
        {
            break;
        }
    }

    if (nbit == 0)
    {
        mem_free(p_calpm_stage);
        return 0;
    }

    _sys_duet2_calpm_calc_compress_index(lchip, p_calpm_stage, &idx_mask, GET_INDEX_MASK_OFFSET(nbit), count, 0xFF, 0xFF);
    mem_free(p_calpm_stage);

    return idx_mask;
}

/* build calpm hard index */
STATIC int32
_sys_duet2_calpm_calc_asic_table(uint8 lchip, sys_calpm_tbl_t** p_table, uint16* old_stage_base, uint8* old_idx_mask)
{
    uint8 stage = 0;
    uint8 alloc_flag[CALPM_TABLE_MAX] = {0};
    uint8 new_idx_mask[CALPM_TABLE_MAX] = {0};
    uint16 old_size[CALPM_TABLE_MAX] = {0};
    uint16 new_size[CALPM_TABLE_MAX] = {0};
    uint32 new_offset[CALPM_TABLE_MAX] = {INVALID_POINTER_OFFSET, INVALID_POINTER_OFFSET};
    int32 ret = CTC_E_NONE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
    {
        if (old_stage_base[stage] != INVALID_POINTER_OFFSET)
        {
            CALPM_IDX_MASK_TO_SIZE(old_idx_mask[stage], old_size[stage]);
        }

        if (!p_table[stage])
        {
            continue;
        }

        new_idx_mask[stage] = _sys_duet2_calpm_get_compress_index(lchip, p_table[stage]);
        CALPM_IDX_MASK_TO_SIZE(new_idx_mask[stage], new_size[stage]);
        new_size[stage] = p_table[stage]->count ? new_size[stage] : 0;

        if (!p_table[CALPM_TABLE_INDEX0]->count)
        {
            new_size[CALPM_TABLE_INDEX1] = 0;
        }
    }

    for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
    {
        /* think for abnormally, such as new_size < old_size when add route. (example as 1.80/10, 1.160/12, add: 1.176/12) */
        if ((old_stage_base[stage] != INVALID_POINTER_OFFSET) && (old_size[stage] > new_size[stage]))
        {
            _sys_duet2_calpm_free_key_offset(lchip, stage, old_size[stage], old_stage_base[stage]);
        }
    }

    alloc_flag[CALPM_TABLE_INDEX0] = (new_size[CALPM_TABLE_INDEX0] && (new_size[CALPM_TABLE_INDEX0] != old_size[CALPM_TABLE_INDEX0]));
    alloc_flag[CALPM_TABLE_INDEX1] = (new_size[CALPM_TABLE_INDEX1] && (new_size[CALPM_TABLE_INDEX1] != old_size[CALPM_TABLE_INDEX1]));

    /* alloc the SRAM, alloc bigger first when there is one opf */
    if (alloc_flag[CALPM_TABLE_INDEX0] && alloc_flag[CALPM_TABLE_INDEX1])
    {
        if (new_size[CALPM_TABLE_INDEX0] >= new_size[CALPM_TABLE_INDEX1])
        {
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX0, new_size[CALPM_TABLE_INDEX0], &(new_offset[CALPM_TABLE_INDEX0])), ret, error0);
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX1, new_size[CALPM_TABLE_INDEX1], &(new_offset[CALPM_TABLE_INDEX1])), ret, error1);
        }
        else
        {
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX1, new_size[CALPM_TABLE_INDEX1], &(new_offset[CALPM_TABLE_INDEX1])), ret, error0);
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX0, new_size[CALPM_TABLE_INDEX0], &(new_offset[CALPM_TABLE_INDEX0])), ret, error1);
        }
    }
    else if (alloc_flag[CALPM_TABLE_INDEX0])
    {
        CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX0, new_size[CALPM_TABLE_INDEX0], &(new_offset[CALPM_TABLE_INDEX0])), ret, error0);
    }
    else if (alloc_flag[CALPM_TABLE_INDEX1])
    {
        CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, CALPM_TABLE_INDEX1, new_size[CALPM_TABLE_INDEX1], &(new_offset[CALPM_TABLE_INDEX1])), ret, error0);
    }

    /* update db */
    for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
    {
        if (!p_table[stage])
        {
            continue;
        }

        /* think for abnormally, such as new_size > old_size when delete route*/
        if ((old_stage_base[stage] != INVALID_POINTER_OFFSET) && (old_size[stage] < new_size[stage]))
        {
            CALPM_IDX_MASK_TO_SIZE(old_idx_mask[stage], old_size[stage]);
            _sys_duet2_calpm_free_key_offset(lchip, stage, old_size[stage], old_stage_base[stage]);
        }

        p_table[stage]->idx_mask = new_idx_mask[stage];
        p_table[stage]->stage_base = (new_size[stage] != old_size[stage]) ? new_offset[stage] : old_stage_base[stage];
        p_table[stage]->stage_index = stage;
    }

    return CTC_E_NONE;

error1:
    if (new_size[CALPM_TABLE_INDEX0] >= new_size[CALPM_TABLE_INDEX1])
    {
        _sys_duet2_calpm_free_key_offset(lchip, CALPM_TABLE_INDEX0, new_size[CALPM_TABLE_INDEX0], new_offset[CALPM_TABLE_INDEX0]);
    }
    else
    {
        _sys_duet2_calpm_free_key_offset(lchip, CALPM_TABLE_INDEX1, new_size[CALPM_TABLE_INDEX1], new_offset[CALPM_TABLE_INDEX1]);
    }

error0:
    for (stage = CALPM_TABLE_INDEX0; stage<CALPM_TABLE_MAX; stage++)
    {
        if ((old_stage_base[stage] != INVALID_POINTER_OFFSET) && (old_size[stage] > new_size[stage]))
        {
            _sys_duet2_calpm_alloc_key_offset_from_position(lchip, stage, old_size[stage], old_stage_base[stage]);
        }
    }

    return ret;
}

/* reload calpm hard index */
STATIC int32
_sys_duet2_calpm_reload_asic_table(uint8 lchip, sys_calpm_tbl_t** p_table, sys_wb_calpm_info_t* p_calpm_info)
{
    int8 stage;
    int32 ret = CTC_E_NONE;
    uint32 offset[CALPM_TABLE_MAX] = {0};
    uint16 size[CALPM_TABLE_MAX] = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    for (stage = CALPM_TABLE_INDEX1; stage >= (int8)CALPM_TABLE_INDEX0; stage--)
    {
        if (!p_table[stage])
        {
            continue;
        }

        if (p_table[stage]->count > 1)
        {
            if (p_table[stage]->stage_base == INVALID_POINTER_OFFSET ||
                p_table[stage]->stage_index != stage)
            {
                return CTC_E_INVALID_PARAM;
            }

            continue;
        }

        p_table[stage]->idx_mask = p_calpm_info->idx_mask[stage];
        p_table[stage]->stage_base = p_calpm_info->stage_base[stage];
        p_table[stage]->stage_index = stage;

        offset[stage] = p_calpm_info->stage_base[stage];
        CALPM_IDX_MASK_TO_SIZE(p_table[stage]->idx_mask, size[stage]);
    }

    if (size[CALPM_TABLE_INDEX0] > 0)
    {
        CTC_ERROR_RETURN(_sys_duet2_calpm_alloc_key_offset_from_position(lchip, CALPM_TABLE_INDEX0, size[CALPM_TABLE_INDEX0], offset[CALPM_TABLE_INDEX0]));
    }

    if (size[CALPM_TABLE_INDEX1] > 0)
    {
        ret = _sys_duet2_calpm_alloc_key_offset_from_position(lchip, CALPM_TABLE_INDEX1, size[CALPM_TABLE_INDEX1], offset[CALPM_TABLE_INDEX1]);
        if (ret)
        {
            return _sys_duet2_calpm_free_key_offset(lchip, CALPM_TABLE_INDEX0, size[CALPM_TABLE_INDEX0], offset[CALPM_TABLE_INDEX0]);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calm_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

#define __5_CALPM_OUTER_FUNCTION__
int32
sys_duet2_get_calpm_prefix_mode(uint8 lchip)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_CALPM_INIT_CHECK;

    return (sys_duet2_calpm_get_prefix_len(lchip, CTC_IP_VER_4) == 8) ? 1 : 0;
}

int32
sys_duet2_calpm_show_calpm_key(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 depth= 0;
    uint8 stage = 0;
    uint8 stage_ip = 0;
    uint8 depth_ip = 0;
    uint8 len = 0;
    uint8 masklen = 0;
    uint8 mask = 0;
    uint8 count = 0;
    uint8 entry_offset[256] = {0};
    uint8 entry_size[256] = {0};
    char offset_list[256] = {'-'};
    uint16 offset = 0;
    uint16 item_idx = 0;
    uint32 *ip = NULL;
    sys_calpm_tbl_t* p_table = NULL;
    sys_calpm_item_t* p_item = NULL;
    sys_ipuc_tcam_data_t tcam_data;
    sys_calpm_param_t calpm_param = {0};

    calpm_param.p_ipuc_param = p_ipuc_param;
    CTC_ERROR_RETURN(_sys_duet2_calpm_lookup_prefix(lchip, &calpm_param));
    if (!calpm_param.p_calpm_prefix)
    {
        return CTC_E_NONE;
    }

    if (MCHIP_API(lchip)->ipuc_show_tcam_key)
    {
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.masklen = p_ipuc_param->masklen;
        tcam_data.info = calpm_param.p_calpm_prefix;
        MCHIP_API(lchip)->ipuc_show_tcam_key(lchip, &tcam_data);
    }

    ip = (uint32*)&p_ipuc_param->ip;
    masklen = p_ipuc_param->masklen - calpm_param.p_calpm_prefix->prefix_len;
    if (!masklen)
    {
        return CTC_E_NONE;
    }

    for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
    {
        masklen -= stage * 8;

        p_table = TABEL_GET_STAGE(stage, &calpm_param);
        if ((NULL == p_table) || (0 == p_table->count))
        {
            continue;
        }

        if (INVALID_POINTER_OFFSET == p_table->stage_base)
        {
            continue;
        }

        if ((stage == CALPM_TABLE_INDEX0) && (masklen >= 8))
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%d (pipeline%d)\n", "DsLpmLookupKey", p_table->stage_base, stage);
            if (p_duet2_calpm_master[lchip]->ipsa_enable)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%d (IPsa pipeline%d)\n", "DsLpmLookupKey1", p_table->stage_base, stage);
            }
            continue;
        }

        stage_ip = _sys_duet2_calpm_get_stage_ip(lchip, ip, p_ipuc_param->ip_ver, stage);
        mask = ~((1 << (8 - masklen)) - 1);

        for (depth = CALPM_MASK_LEN; depth > 0; depth--)
        {
            for (offset = 0; offset <= GET_MASK_MAX(depth); offset++)
            {
                depth_ip = (offset << (CALPM_MASK_LEN - depth));
                item_idx = GET_BASE(depth) + offset;
                p_item = ITEM_GET_PTR(p_table, item_idx);
                if ((NULL == p_item) || (p_item->t_masklen != masklen) || ((stage_ip & mask) != (depth_ip & mask))
                    || (!CTC_FLAG_ISSET(p_item->item_flag, CALPM_ITEM_FLAG_VALID)))
                {
                    continue;
                }

                _sys_duet2_calpm_get_stage_index_and_num(lchip, depth_ip, GET_MASK(depth), p_table->idx_mask, &entry_offset[count], &entry_size[count]);
                if (!entry_size[count])
                {
                    return CTC_E_UNEXPECT;
                }

                count++;
            }
        }

        if (count)
        {
            for(offset = 0; offset < count; offset++)
            {
                if (offset + 1 < count)
                {
                    if (entry_offset[offset] + entry_size[offset] == entry_offset[offset+1])
                    {
                        entry_size[offset] += entry_size[offset+1];

                        if (offset + 2 < count)
                        {
                            sal_memmove(&entry_offset[offset+1], &entry_offset[offset+2], sizeof(uint8) * (count - offset - 2));
                            sal_memmove(&entry_size[offset+1], &entry_size[offset+2], sizeof(uint8) * (count - offset - 2));
                        }
                        count--;
                        offset--;
                    }
                }
            }

            for(offset = 0; offset < count; offset++)
            {
                if (entry_size[offset] == 1)
                {
                    len += sal_sprintf(offset_list + len, "%d, ", p_table->stage_base + entry_offset[offset]);
                }
                else if (entry_size[offset] > 1)
                {
                    len += sal_sprintf(offset_list + len, "%d-%d, ", p_table->stage_base + entry_offset[offset], p_table->stage_base + entry_offset[offset] + entry_size[offset] - 1);
                }
            }
            offset_list[len-2] = '\0';
        }

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%s (pipeline%d)\n", "DsLpmLookupKey", offset_list, stage);
        if (p_duet2_calpm_master[lchip]->ipsa_enable)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%s (IPsa pipeline%d)\n", "DsLpmLookupKey1", offset_list, stage);
        }
    }

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_show_status(uint8 lchip)
{
    uint32 max_size = 0;
    uint32 rsv_size = CALPM_SRAM_RSV_FOR_ARRANGE;
    char size_str[CALPM_TABLE_MAX][8] = {"-", "-"};

    SYS_CALPM_INIT_CHECK;

    if (p_duet2_calpm_master[lchip]->couple_mode)
    {
        rsv_size = CALPM_SRAM_RSV_FOR_ARRANGE * 2;
        sal_sprintf(size_str[CALPM_TABLE_INDEX0], "%u", p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0]);
        sal_sprintf(size_str[CALPM_TABLE_INDEX1], "%u", p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX1]);
    }

    max_size = p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0] + p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX1] + rsv_size;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d (rsv: %d)\n", "-CALPM", " ", max_size, rsv_size);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--CALPM1", " ", size_str[CALPM_TABLE_INDEX0], " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15d\n", "/", " ", p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX0]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--CALPM2", " ", size_str[CALPM_TABLE_INDEX1], " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15d\n", "/", " ", p_duet2_calpm_master[lchip]->calpm_usage[CALPM_TABLE_INDEX1]);

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_show_stats(uint8 lchip)
{
    uint8 idx0 = 0;
    uint8 idx1 = 0;

    SYS_CALPM_INIT_CHECK;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------calpm block stats-----------------------------\n");
    for (idx0 = 0; idx0 < CALPM_TABLE_MAX; idx0++)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "pipeline %d\n%-10s %-6s %-10s\n", idx0, "block size", " ", "count");
        for (idx1 = 0; idx1 < CALPM_OFFSET_BITS_NUM; idx1++)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d %-6s %-10d\n", (1 << idx1), " ", p_duet2_calpm_master[lchip]->calpm_stats[idx0][idx1]);
        }

        if (idx0 == CALPM_TABLE_INDEX1)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------\n");
        }
    }

    return CTC_E_NONE;
}

STATIC char*
_sys_duet2_calpm_get_item_flag(uint32 flag, char* flag_str)
{
    sal_memset(flag_str, 0, sal_strlen(flag_str));

    if (!flag)
    {
        return "N";
    }

    if (CTC_FLAG_ISSET(flag, CALPM_ITEM_FLAG_REAL))
    {
        sal_strncat(flag_str, "R", 1);
    }

    if (CTC_FLAG_ISSET(flag, CALPM_ITEM_FLAG_PARENT))
    {
        sal_strncat(flag_str, "P", 1);
    }

    if (CTC_FLAG_ISSET(flag, CALPM_ITEM_FLAG_VALID))
    {
        sal_strncat(flag_str, "V", 1);
    }

    return flag_str;
}

int32
sys_duet2_calpm_show_calpm_tree(sys_calpm_prefix_t* p_calpm_prefix, uint8 stage, sys_calpm_stats_t *p_stats, uint8 detail)
{
    uint8 pointer_ct = 0;
    uint8 tbl1_idx0 = 0;
    uint8 tbl1_idx1 = 0;
    uint8 tbl1_no = 0;
    uint8 item_idx = 0;
    uint8 stage_ip = 0;
    uint8 first_print = 1;
    char flag_str[16] = {0};
    uint8 table_ct[CALPM_TABLE_MAX] = {0};
    uint16 item_ct[CALPM_TABLE_MAX] = {0};
    uint16 real_ct[CALPM_TABLE_MAX] = {0};
    uint16 valid_ct[CALPM_TABLE_MAX] = {0};
    sys_calpm_item_t *item_head = NULL;
    sys_calpm_item_t *item_cur = NULL;
    sys_calpm_tbl_t* n0_table = NULL;
    char prefix_str[64] = {0};

    if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
    {
        if (p_calpm_prefix->prefix_len == 8)
        {
            sal_sprintf(prefix_str, "%d.", (p_calpm_prefix->ip32[0] >> 24) & 0xFF);
        }
        else
        {
            sal_sprintf(prefix_str, "%d.%d.", (p_calpm_prefix->ip32[0] >> 24) & 0xFF,  (p_calpm_prefix->ip32[0] >> 16) & 0xFF);
        }
    }
    else
    {
        sal_sprintf(prefix_str, "%x:%x:%x:", (p_calpm_prefix->ip32[0] >> 16) & 0xFFFF,  p_calpm_prefix->ip32[0] & 0xFFFF,
            (p_calpm_prefix->ip32[1] >> 16) & 0xFFFF);
    }

    if (detail)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n-------------------------prefix %s---------------------\n", prefix_str);
    }

    n0_table = p_calpm_prefix->p_ntable;
    if (!n0_table)
    {
        if (detail)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\ndb usage\n--tbl_count: %-3d\n--node_count: %-3d (real: %d)\n", 0, 0, 0);
        }
        return CTC_E_NONE;
    }

    if (stage != CALPM_TABLE_INDEX1)
    {
        table_ct[CALPM_TABLE_INDEX0]++;
        if (detail)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------pipeline 0------------\nstage0 info\n--pointer count: %d\n--calpmkey index: %-4d\n--compress_index: 0x%02x\n\nentry list\n",
                    n0_table->count, n0_table->stage_base, n0_table->idx_mask);
        }

        for (item_idx = 0; item_idx < CALPM_ITEM_ALLOC_NUM; item_idx++)
        {
            if (!n0_table->p_item[item_idx])
            {
                continue;
            }

            item_head = n0_table->p_item[item_idx];
            while(item_head)
            {
                item_cur = item_head;
                item_head = item_head->p_nitem;

                if (detail == 2)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--node[%03d] mask: %d flag: %-3s ad_index: %-4d\n", item_cur->item_idx, item_cur->t_masklen,
                        _sys_duet2_calpm_get_item_flag(item_cur->item_flag, flag_str), item_cur->ad_index);
                }

                if (CTC_FLAG_ISSET(item_cur->item_flag, CALPM_ITEM_FLAG_VALID))
                {
                    valid_ct[CALPM_TABLE_INDEX0]++;
                }

                if (CTC_FLAG_ISSET(item_cur->item_flag, CALPM_ITEM_FLAG_REAL))
                {
                    if (detail)
                    {
                        real_ct[CALPM_TABLE_INDEX0]++;
                        if (p_calpm_prefix->ip_ver == CTC_IP_VER_4)
                        {
                            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%2d.    %s%d/%d    \tad: %d\n", real_ct[CALPM_TABLE_INDEX0], prefix_str, ((item_cur->item_idx - GET_BASE(item_cur->t_masklen)) << (8 - item_cur->t_masklen)),
                                    item_cur->t_masklen + p_calpm_prefix->prefix_len, item_cur->ad_index);
                        }
                        else
                        {
                            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%2d.    %s%x::/%d    \tad: %d\n", real_ct[CALPM_TABLE_INDEX0], prefix_str, ((item_cur->item_idx - GET_BASE(item_cur->t_masklen)) << (8 - item_cur->t_masklen)),
                                    item_cur->t_masklen + p_calpm_prefix->prefix_len, item_cur->ad_index);
                        }
                    }
                }
                item_ct[CALPM_TABLE_INDEX0]++;
            }
        }

        if (detail)
        {
            if (!real_ct[CALPM_TABLE_INDEX0])
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  NULL\n");
            }
        }

        if (n0_table->count)
        {
            if (detail)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\npointer list\n");
            }

            for (tbl1_idx0 = 0; tbl1_idx0 < CALPM_TABLE_ALLOC_NUM; tbl1_idx0++)
            {
                if (!n0_table->p_ntable[tbl1_idx0])
                {
                    continue;
                }

                for (tbl1_idx1 = 0; tbl1_idx1 < CALPM_TABLE_BLOCK_NUM; tbl1_idx1++)
                {
                    if (!n0_table->p_ntable[tbl1_idx0][tbl1_idx1])
                    {
                        continue;
                    }

                    if (detail)
                    {
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%2d.    %s%d.*\n", ++pointer_ct, prefix_str, tbl1_idx0 * CALPM_TABLE_BLOCK_NUM + tbl1_idx1);
                    }
                }
            }
        }

        if (detail)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\ndb usage\n--tbl_count: %-3d\n--node_count: %-3d (real: %d, valid: %d)\n",
                n0_table->count, item_ct[CALPM_TABLE_INDEX0], real_ct[CALPM_TABLE_INDEX0], valid_ct[CALPM_TABLE_INDEX0]);
        }
    }

    if (stage != CALPM_TABLE_INDEX0)
    {
        if (detail)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------pipeline 1------------\n");
        }

        if (stage == CALPM_TABLE_INDEX1)
        {
            if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
            {
                stage_ip = (p_calpm_prefix->ip32[0] >> (32 - p_calpm_prefix->prefix_len - 8)) & 0xFF;
            }
            else
            {
                stage_ip = (p_calpm_prefix->ip32[1] >> (64 - p_calpm_prefix->prefix_len - 8)) & 0xFF;
            }
        }

        for (tbl1_idx0 = 0; tbl1_idx0 < CALPM_TABLE_ALLOC_NUM; tbl1_idx0++)
        {
            if (!n0_table->p_ntable[tbl1_idx0])
            {
                continue;
            }

            for (tbl1_idx1 = 0; tbl1_idx1 < CALPM_TABLE_BLOCK_NUM; tbl1_idx1++)
            {
                if (!n0_table->p_ntable[tbl1_idx0][tbl1_idx1])
                {
                    continue;
                }

                table_ct[CALPM_TABLE_INDEX1]++;
                if ((stage == CALPM_TABLE_INDEX1) && ((tbl1_idx0 * CALPM_TABLE_ALLOC_NUM + tbl1_idx1) != stage_ip))
                {
                    continue;
                }
                stage_ip = tbl1_idx0 * CALPM_TABLE_ALLOC_NUM + tbl1_idx1;

                if (detail)
                {
                    if (first_print)
                    {
                        first_print = 0;
                    }
                    else
                    {
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------------------\n");
                    }

                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "stage1.%d info\n--segment ip: %d\n--calpmkey index: %-4d\n--makslen: 0x%02x\n\nentry list\n", ++tbl1_no, stage_ip,
                                                            n0_table->p_ntable[tbl1_idx0][tbl1_idx1]->stage_base, n0_table->p_ntable[tbl1_idx0][tbl1_idx1]->idx_mask);
                }

                for (item_idx = 0; item_idx < CALPM_ITEM_ALLOC_NUM; item_idx++)
                {
                    item_head = n0_table->p_ntable[tbl1_idx0][tbl1_idx1]->p_item[item_idx];
                    if (!item_head)
                    {
                        continue;
                    }

                    while(item_head)
                    {
                        item_cur = item_head;
                        item_head = item_head->p_nitem;

                        if (detail == 2)
                        {
                            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--node[%03d] mask: %d flag: %-3s ad_index: %-4d\n", item_cur->item_idx, item_cur->t_masklen,
                                _sys_duet2_calpm_get_item_flag(item_cur->item_flag, flag_str), item_cur->ad_index);
                        }

                        if (CTC_FLAG_ISSET(item_cur->item_flag, CALPM_ITEM_FLAG_VALID))
                        {
                            valid_ct[CALPM_TABLE_INDEX1]++;
                        }

                        if (CTC_FLAG_ISSET(item_cur->item_flag, CALPM_ITEM_FLAG_REAL))
                        {
                            if (detail)
                            {
                                if (p_calpm_prefix->ip_ver == CTC_IP_VER_4)
                                {
                                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%2d.    %s%d.%d/%d\tad: %d\n", real_ct[CALPM_TABLE_INDEX1], prefix_str, stage_ip,
                                            ((item_cur->item_idx - GET_BASE(item_cur->t_masklen)) << (8 - item_cur->t_masklen)), item_cur->t_masklen + 8 + p_calpm_prefix->prefix_len, item_cur->ad_index);
                                }
                                else
                                {
                                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%2d.    %s%x%x::/%d\tad: %d\n", real_ct[CALPM_TABLE_INDEX1], prefix_str, stage_ip,
                                            ((item_cur->item_idx - GET_BASE(item_cur->t_masklen)) << (8 - item_cur->t_masklen)), item_cur->t_masklen + 8 + p_calpm_prefix->prefix_len, item_cur->ad_index);
                                }
                            }
                            real_ct[CALPM_TABLE_INDEX1]++;
                        }
                        item_ct[1]++;
                    }
                }
            }
        }

        if (detail)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\ndb usage\n--tbl_count: 0\n--node_count: %-3d (real: %d, valid: %d)\n", item_ct[CALPM_TABLE_INDEX1], real_ct[CALPM_TABLE_INDEX1], valid_ct[CALPM_TABLE_INDEX1]);
        }
    }

    if (p_stats)
    {
        sal_memcpy(p_stats->table_ct, table_ct, sizeof(uint8) * CALPM_TABLE_MAX);
        sal_memcpy(p_stats->item_ct, item_ct, sizeof(uint16) * CALPM_TABLE_MAX);
        sal_memcpy(p_stats->real_ct, real_ct, sizeof(uint16) * CALPM_TABLE_MAX);
        sal_memcpy(p_stats->valid_ct, valid_ct, sizeof(uint16) * CALPM_TABLE_MAX);
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_traverse_prefix(uint8 lchip,
                uint8 ip_ver,
                hash_traversal_fn   fn,
                void                *data)
{
    return ctc_hash_traverse_through(p_duet2_calpm_master[lchip]->calpm_prefix[ip_ver], fn, data);
}

int32
_sys_duet2_calpm_show_calpm_tree_fn(sys_calpm_prefix_t* p_calpm_prefix, void *data)
{
    sys_calpm_stats_t calpm_stats;
    sys_calpm_traverse_t* travs = (sys_calpm_traverse_t*)data;
    uint16 detail = travs->value0;
    uint16 vrf_id = travs->value1;
    uint32* all_tbl_ct = travs->data;
    uint32* all_item_ct = travs->data1;
    uint32* all_real_item_ct = travs->data2;
    uint32* all_valid_ct = travs->data3;
    uint8 lchip = travs->lchip;

    sal_memset(&calpm_stats, 0, sizeof(sys_calpm_stats_t));

    if ((vrf_id <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID)) && (p_calpm_prefix->vrf_id != vrf_id))
    {
        return CTC_E_NONE;
    }

    travs->value2++;

    CTC_ERROR_RETURN(sys_duet2_calpm_show_calpm_tree(p_calpm_prefix, CALPM_TABLE_MAX, &calpm_stats, detail));

    all_tbl_ct[CALPM_TABLE_INDEX0] += calpm_stats.table_ct[CALPM_TABLE_INDEX0];
    all_tbl_ct[CALPM_TABLE_INDEX1] += calpm_stats.table_ct[CALPM_TABLE_INDEX1];
    all_item_ct[CALPM_TABLE_INDEX0] += calpm_stats.item_ct[CALPM_TABLE_INDEX0];
    all_item_ct[CALPM_TABLE_INDEX1] += calpm_stats.item_ct[CALPM_TABLE_INDEX1];
    all_real_item_ct[CALPM_TABLE_INDEX0] += calpm_stats.real_ct[CALPM_TABLE_INDEX0];
    all_real_item_ct[CALPM_TABLE_INDEX1] += calpm_stats.real_ct[CALPM_TABLE_INDEX1];
    all_valid_ct[CALPM_TABLE_INDEX0] += calpm_stats.valid_ct[CALPM_TABLE_INDEX0];
    all_valid_ct[CALPM_TABLE_INDEX1] += calpm_stats.valid_ct[CALPM_TABLE_INDEX1];

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_db_show(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint32 detail)
{
    uint8 stage = 0;
    uint8 prefix_len = 0;
    uint16 prefix_count = 0;
    char  vrfid_str[8] = {0};
    uint32 db_total[3] = {0};
    uint32 all_tbl_ct[2] = {0};
    uint32 all_item_ct[2] = {0};
    uint32 all_real_item_ct[2] = {0};
    uint32 all_valid_ct[2] = {0};
    sys_calpm_traverse_t travs = {0};
    sys_calpm_param_t calpm_param = {0};
    sys_calpm_param_t *p_calpm_param = &calpm_param;

    CTC_MAX_VALUE_CHECK(p_ipuc_param->vrf_id, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) + 1);

    LCHIP_CHECK(lchip);
	SYS_CALPM_INIT_CHECK;
    p_calpm_param->p_ipuc_param = p_ipuc_param;

    if (p_ipuc_param->masklen)
    {
        _sys_duet2_calpm_lookup_prefix(lchip, p_calpm_param);
        if (!p_calpm_param->p_calpm_prefix)
        {
            return CTC_E_NOT_EXIST;
        }

        if (CTC_IP_VER_4 == p_ipuc_param->ip_ver)
        {
            p_calpm_param->p_calpm_prefix->ip32[0] = p_ipuc_param->ip.ipv4;
        }
        else
        {
            p_calpm_param->p_calpm_prefix->ip32[0] = p_ipuc_param->ip.ipv6[3];
            p_calpm_param->p_calpm_prefix->ip32[1] = p_ipuc_param->ip.ipv6[2];
        }

        stage = (p_ipuc_param->masklen - p_calpm_param->p_calpm_prefix->prefix_len < 8) ?  CALPM_TABLE_INDEX0 : CALPM_TABLE_INDEX1;
        sys_duet2_calpm_show_calpm_tree(p_calpm_param->p_calpm_prefix, stage, NULL, detail);
    }
    else
    {
        travs.lchip = lchip;
        travs.value0 = detail;
        travs.value1 = p_ipuc_param->vrf_id;
        travs.data = (void*)all_tbl_ct;
        travs.data1 = (void*)all_item_ct;
        travs.data2 = (void*)all_real_item_ct;
        travs.data3 = (void*)all_valid_ct;

        sal_sprintf(vrfid_str, "%d", p_ipuc_param->vrf_id);

        prefix_count = p_duet2_calpm_master[lchip]->calpm_prefix[p_ipuc_param->ip_ver]->count;
        prefix_len = p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver];

        _sys_duet2_calpm_show_stats(lchip);

        _sys_duet2_calpm_traverse_prefix(lchip, p_ipuc_param->ip_ver, (hash_traversal_fn)_sys_duet2_calpm_show_calpm_tree_fn, (void*)&travs);

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------calpm softdb info---------------------------------\n");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ip version: %-6s         vrf id: %s\n", (p_ipuc_param->ip_ver == CTC_IP_VER_4) ? "IPv4" : "IPv6", (p_ipuc_param->vrf_id == MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) + 1) ? "all vrf" : vrfid_str);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "prefix length: %-2d          pipeline level: 2\n", prefix_len);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "total prefix: %-8d     vrf prefix: %-8d       prefix db_size: %d\n", prefix_count, travs.value2, (uint32)sizeof(sys_calpm_prefix_t));
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "pipeline0 db_size: %-3d     pipeline1 db_size: %-3d     node db_size: %d\n", (uint32)sizeof(sys_calpm_tbl_t), (uint32)(sizeof(sys_calpm_tbl_t) - sizeof(sys_calpm_tbl_t *) * CALPM_TABLE_ALLOC_NUM), (uint32)sizeof(sys_calpm_item_t));
        db_total[0] = all_tbl_ct[0] * sizeof(sys_calpm_tbl_t) + all_item_ct[0] * sizeof(sys_calpm_item_t);
        db_total[1] = all_tbl_ct[1] * CTC_OFFSET_OF(sys_calpm_tbl_t, p_ntable) + all_item_ct[1] * sizeof(sys_calpm_item_t);
        db_total[2] = db_total[0] + db_total[1] + prefix_count * sizeof(sys_calpm_prefix_t);

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nall_db_stats:  %-4uB\n", db_total[2]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------calpm stats------------------\n");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
            "pipeline 0\n--all_tbl_count: %-3d\n--all_node_count: %-3d (real: %d, valid: %d)\n--all_db_stats: %-4uB\npipeline 1\n--all_tbl_count: %-3d\n--all_node_count: %-3d (real: %d, valid: %d)\n--all_db_stats: %-4uB\n\n",
            all_tbl_ct[0], all_item_ct[0], all_real_item_ct[0], all_valid_ct[0], db_total[0], all_tbl_ct[1], all_item_ct[1], all_real_item_ct[1], all_valid_ct[1], db_total[1]);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_move(uint8 lchip,
                sys_calpm_param_t     *p_calpm_param,
                uint8               stage,
                uint32              new_pointer,
                uint32              old_pointer)
{
    uint16 old_size;
    sys_calpm_tbl_t* p_table = NULL;
    sys_calpm_tbl_t* p_table_pre = NULL;
    sys_ipuc_tcam_data_t tcam_data = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    p_table = TABEL_GET_STAGE(stage, p_calpm_param);
    if (!p_table)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ERROR: p_table is NULL when update calpm\r\n");
        return CTC_E_INVALID_PARAM;
    }

    p_table->stage_base = new_pointer;
    CALPM_IDX_MASK_TO_SIZE(p_table->idx_mask, old_size);

    if (stage > CALPM_TABLE_INDEX0)
    {
        p_table_pre = TABEL_GET_STAGE((stage - 1), p_calpm_param);
    }

    CTC_ERROR_RETURN(_sys_duet2_calpm_write_stage_key(lchip, p_calpm_param, stage));

    if (p_table_pre)
    {
        CTC_ERROR_RETURN(_sys_duet2_calpm_write_stage_key(lchip, p_calpm_param, stage - 1));
    }
    else
    {
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.ad_index = p_calpm_param->ad_index;
        tcam_data.masklen = p_calpm_param->p_ipuc_param->masklen;
        tcam_data.info = (void*)p_calpm_param->p_calpm_prefix;
	CTC_ERROR_RETURN(sys_duet2_ipuc_tcam_write_calpm_ad(lchip, &tcam_data));
        CTC_ERROR_RETURN(sys_duet2_ipuc_tcam_write_calpm_key(lchip, &tcam_data));
    }

    CTC_ERROR_RETURN(_sys_duet2_calpm_free_key_offset(lchip, stage, old_size, old_pointer));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_calpm_param_sort_slist(uint8 lchip, sys_calpm_arrange_info_t* phead, sys_calpm_arrange_info_t* pend)
{
    sys_calpm_arrange_info_t* phead_node = NULL;
    sys_calpm_arrange_info_t* pcurr_node = NULL;
    sys_calpm_arrange_info_t* pnext_node = NULL;
    sys_calpm_tbl_arrange_t* p_data     = NULL;

    if ((NULL == phead) || (NULL == pend) || (phead == pend))
    {
        return CTC_E_NONE;
    }

    phead_node = phead;

    while(phead_node != pend)
    {
        pcurr_node = phead_node;
        pnext_node = pcurr_node->p_next_info;
        while(pnext_node)
        {
            if (pnext_node->p_data->start_offset <= pcurr_node->p_data->start_offset)
            {
                p_data = pcurr_node->p_data;
                pcurr_node->p_data = pnext_node->p_data;
                pnext_node->p_data = p_data;
            }
            pnext_node = pnext_node->p_next_info;
        }
        phead_node = phead_node->p_next_info;
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_calpm_param_anylize_info(uint8 lchip,
                sys_ipuc_param_list_t    *p_param_list,
                sys_calpm_arrange_info_t ** pp_arrange_info)
{
    sys_ipuc_param_list_t* p_param_list_t = NULL;
    sys_calpm_arrange_info_t* p_arrange_tbl = NULL;
    sys_calpm_arrange_info_t* p_arrange_tbl_head = NULL;
    sys_calpm_arrange_info_t* p_arrange_cur = NULL;
    sys_calpm_arrange_info_t* p_arrange_pre = NULL;
    sys_calpm_param_t calpm_param = {0};
    sys_calpm_param_t* p_calpm_param = NULL;
    sys_calpm_tbl_t* p_table[CALPM_TABLE_MAX] = {NULL};
    sys_calpm_tbl_t* p_table_cur = NULL;
    sys_calpm_tbl_t* p_table_pre = NULL;
    uint16 no = 0;
    uint16 size = 0;
    uint8 stage = 0;
    uint8 count = 0;

    p_param_list_t = p_param_list;

    /* get all entry info */
    while (NULL != p_param_list_t)
    {
        CTC_PTR_VALID_CHECK(p_param_list_t->p_ipuc_param);
        calpm_param.ad_index = p_param_list_t->ad_index;
        calpm_param.p_ipuc_param = p_param_list_t->p_ipuc_param;
        p_param_list_t = p_param_list_t->p_next_param;

        CTC_ERROR_RETURN(_sys_duet2_calpm_lookup_prefix(lchip, &calpm_param));
        if (!calpm_param.p_calpm_prefix)
        {
            continue;
        }

        if (CTC_IP_VER_4 == calpm_param.p_ipuc_param->ip_ver)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "anylize NO.%d ipuc info ip: 0x%x  masklen: %d\n",
                             ++no, calpm_param.p_ipuc_param->ip.ipv4, calpm_param.p_ipuc_param->masklen);
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "anylize NO.%d ipuc info ip %x:%x:%x:%x  masklen: %d\n",
                             ++no, calpm_param.p_ipuc_param->ip.ipv6[3], calpm_param.p_ipuc_param->ip.ipv6[2],
                             calpm_param.p_ipuc_param->ip.ipv6[1], calpm_param.p_ipuc_param->ip.ipv6[0], calpm_param.p_ipuc_param->masklen);
        }

        for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
        {
            p_table[stage] = TABEL_GET_STAGE(stage, &calpm_param);
            if (NULL == p_table[stage])
            {
                break;
            }

            if (INVALID_POINTER_OFFSET == p_table[stage]->stage_base)
            {
                continue;
            }

            if (NULL == p_arrange_tbl)
            {
                p_arrange_tbl = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_arrange_info_t));
                if (NULL == p_arrange_tbl)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                    return CTC_E_NO_MEMORY;
                }

                sal_memset(p_arrange_tbl, 0, sizeof(sys_calpm_arrange_info_t));
                p_arrange_tbl->p_data = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_tbl_arrange_t));
                if (NULL == p_arrange_tbl->p_data)
                {
                    mem_free(p_arrange_tbl);
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                    return CTC_E_NO_MEMORY;
                }

                sal_memset(p_arrange_tbl->p_data, 0, sizeof(sys_calpm_tbl_arrange_t));

                if (NULL == p_arrange_tbl_head)
                {
                    p_arrange_tbl_head = p_arrange_tbl;
                }
            }
            else
            {
                p_arrange_tbl->p_next_info = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_arrange_info_t));
                if (NULL == p_arrange_tbl->p_next_info)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                    return CTC_E_NO_MEMORY;
                }

                sal_memset(p_arrange_tbl->p_next_info, 0, sizeof(sys_calpm_arrange_info_t));
                p_arrange_tbl->p_next_info->p_data = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_tbl_arrange_t));
                if (NULL == p_arrange_tbl->p_next_info->p_data)
                {
                    mem_free(p_arrange_tbl);
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                    return CTC_E_NO_MEMORY;
                }

                sal_memset(p_arrange_tbl->p_next_info->p_data, 0, sizeof(sys_calpm_tbl_arrange_t));

                p_arrange_tbl = p_arrange_tbl->p_next_info;
            }

            p_arrange_tbl->p_data->idx_mask = p_table[stage]->idx_mask;
            p_arrange_tbl->p_data->start_offset = p_table[stage]->stage_base;
            sal_memcpy(&(p_arrange_tbl->p_data->calpm_param), &calpm_param, sizeof(sys_calpm_param_t));
            p_arrange_tbl->p_data->stage = stage;
            CALPM_IDX_MASK_TO_SIZE(p_arrange_tbl->p_data->idx_mask, size);
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "store stage[%d] offset_base: %4d  size: %d \n",
                             stage, p_arrange_tbl->p_data->start_offset, size);
        }
    }

    /* sort and delete the same node */
    _sys_duet2_calpm_param_sort_slist(lchip, p_arrange_tbl_head, p_arrange_tbl);
    p_arrange_cur = p_arrange_tbl_head;
    p_arrange_pre = p_arrange_tbl_head;
    p_table_pre = NULL;

    while (NULL != p_arrange_cur)
    {
        p_calpm_param = &p_arrange_cur->p_data->calpm_param;

        p_table_cur = TABEL_GET_STAGE(p_arrange_cur->p_data->stage, p_calpm_param);
        if (p_table_pre == p_table_cur)
        {
            p_arrange_pre->p_next_info = p_arrange_cur->p_next_info;
            mem_free(p_arrange_cur->p_data);
            mem_free(p_arrange_cur);
            p_arrange_cur = p_arrange_pre->p_next_info;
            continue;
        }

        p_arrange_pre = p_arrange_cur;
        p_table_pre = p_table_cur;

        p_arrange_cur = p_arrange_cur->p_next_info;
    }

    p_arrange_cur = p_arrange_tbl_head;
    while (NULL != p_arrange_cur)
    {
        p_calpm_param = &p_arrange_cur->p_data->calpm_param;
        p_table_cur = TABEL_GET_STAGE(p_arrange_cur->p_data->stage, p_calpm_param);

        CALPM_IDX_MASK_TO_SIZE(p_table_cur->idx_mask, size);

        if (p_calpm_param->p_calpm_prefix->ip_ver == CTC_IP_VER_4)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NO.%d ip: 0x%x, masklen: %d stage[%d] offset_base: %4d  idx_mask: 0x%x size: %d \n",
                             ++count, p_calpm_param->p_ipuc_param->ip.ipv4, p_calpm_param->p_ipuc_param->masklen, p_arrange_cur->p_data->stage,
                             p_table_cur->stage_base, p_table_cur->idx_mask, size);
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NO.%d ip: %x:%x:%x:%x, masklen: %d stage[%d] offset_base: %4d  idx_mask: 0x%x size: %d \n",
                             ++count, p_calpm_param->p_ipuc_param->ip.ipv6[3], p_calpm_param->p_ipuc_param->ip.ipv6[2], p_calpm_param->p_ipuc_param->ip.ipv6[1],
                             p_calpm_param->p_ipuc_param->ip.ipv6[0], p_calpm_param->p_ipuc_param->masklen, p_arrange_cur->p_data->stage, p_table_cur->stage_base,
                             p_table_cur->idx_mask, size);
        }

        p_arrange_cur = p_arrange_cur->p_next_info;
    }

    *pp_arrange_info = p_arrange_tbl_head;

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_arrange_fragment(uint8 lchip, sys_ipuc_param_list_t *p_info_list)
{
    sys_calpm_arrange_info_t     *p_arrange_info_tbl = NULL;
    sys_calpm_arrange_info_t     *p_arrange_tmp = NULL;
    ctc_ipuc_param_t         *p_ipuc_param = NULL;
    sys_calpm_param_t         *p_calpm_param = NULL;
    sys_calpm_tbl_t               *p_table = NULL;
    int32 ret = CTC_E_NONE;
    uint32  tmp_offset = 0;
    uint32  head_offset = 0;
    uint32  old_base  = 0;
    uint32  new_base = 0;
    uint16  size        = 0;
    uint8   stage       = 0;
    uint8   head_adj_flag = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_CALPM_INIT_CHECK;

    if (CALPM_SRAM_RSV_FOR_ARRANGE == 0)
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_CALPM_LOCK;
    /* p_info_list free */
    _sys_duet2_calpm_param_anylize_info(lchip, p_info_list, &p_arrange_info_tbl);

    /* start arrange by sort table */
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "\n### start to arrange SRAM \n");

    head_offset = sys_duet2_calpm_get_extra_offset(lchip, stage, SYS_CALPM_START_OFFSET);

    while (p_arrange_info_tbl != NULL)
    {
        stage = p_arrange_info_tbl->p_data->stage;
        if (!head_adj_flag && p_duet2_calpm_master[lchip]->couple_mode && stage)
        {
            tmp_offset = sys_duet2_calpm_get_extra_offset(lchip, stage, SYS_CALPM_START_OFFSET);
            if (tmp_offset < head_offset)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "stage 1 head offset: %d small than stage 0 head offset: %d\n", tmp_offset, head_offset);
                CTC_RETURN_CALPM_UNLOCK(CTC_E_UNEXPECT);
            }

            head_offset = tmp_offset;
            head_adj_flag = 1;
        }

        p_calpm_param = &(p_arrange_info_tbl->p_data->calpm_param);
        p_ipuc_param = p_calpm_param->p_ipuc_param;
        p_table = TABEL_GET_STAGE(stage, p_calpm_param);
        if (!p_table)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ERROR: p_table is NULL when arrange offset\r\n");
            CTC_RETURN_CALPM_UNLOCK(CTC_E_NOT_EXIST);
        }
        old_base = p_table->stage_base;
        CALPM_IDX_MASK_TO_SIZE(p_table->idx_mask, size);

        if (CTC_IP_VER_4 == p_ipuc_param->ip_ver)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "before arrange ipuc: 0x%x masklen: %d stage[%d] old_base: %d  head_offset: %d  size: %d\n",
                             p_ipuc_param->ip.ipv4, p_ipuc_param->masklen, p_arrange_info_tbl->p_data->stage, old_base, head_offset, size);
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "before arrange ipuc: %x:%x:%x:%x masklen: %d stage[%d] old_base: %d  head_offset: %d  size: %d\n",
                             p_ipuc_param->ip.ipv6[3], p_ipuc_param->ip.ipv6[2],
                             p_ipuc_param->ip.ipv6[1], p_ipuc_param->ip.ipv6[0], p_ipuc_param->masklen,
                             p_arrange_info_tbl->p_data->stage, old_base, head_offset, size);
        }

        /* such as rsv space small than 256 */
        if (size > CALPM_SRAM_RSV_FOR_ARRANGE)
        {
            head_offset += size;
            continue;
        }

        if (old_base <= head_offset)
        {
            if (p_arrange_info_tbl->p_data)
            {
                mem_free(p_arrange_info_tbl->p_data);
                p_arrange_info_tbl->p_data = NULL;
                p_arrange_tmp = p_arrange_info_tbl->p_next_info;
                mem_free(p_arrange_info_tbl);
            }

            p_arrange_info_tbl = p_arrange_tmp;
            if (old_base == head_offset)
            {
                head_offset += size;
            }

            continue;
        }

        if ((old_base - head_offset) >= size)
        {
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, stage, size, &new_base), ret, error);
            CTC_ERROR_GOTO(sys_duet2_calpm_move(lchip, p_calpm_param, stage, new_base, p_table->stage_base), ret, error);
        }
        else
        {
            new_base = sys_duet2_calpm_get_extra_offset(lchip, stage, SYS_CALPM_ARRANGE_OFFSET);
            CTC_ERROR_GOTO(sys_duet2_calpm_move(lchip, p_calpm_param, stage, new_base, p_table->stage_base), ret, error);

            old_base = new_base;
            CTC_ERROR_GOTO(_sys_duet2_calpm_alloc_key_offset(lchip, stage, size, &new_base), ret, error);
            CTC_ERROR_GOTO(sys_duet2_calpm_move(lchip, p_calpm_param, stage, new_base, old_base), ret, error);
        }

        mem_free(p_arrange_info_tbl->p_data);
        p_arrange_info_tbl->p_data = NULL;
        p_arrange_tmp = p_arrange_info_tbl->p_next_info;
        mem_free(p_arrange_info_tbl);

        p_arrange_info_tbl = p_arrange_tmp;
        head_offset += size;
    }
    SYS_CALPM_UNLOCK;

    return CTC_E_NONE;

error:
    while (p_arrange_info_tbl != NULL)
    {
        if (p_arrange_info_tbl->p_data)
        {
            mem_free(p_arrange_info_tbl->p_data);
            p_arrange_info_tbl->p_data = NULL;
        }
        p_arrange_tmp = p_arrange_info_tbl->p_next_info;
        mem_free(p_arrange_info_tbl);
        p_arrange_info_tbl = p_arrange_tmp;
    }
    SYS_CALPM_UNLOCK;

    return ret;
}

uint8
sys_duet2_calpm_get_prefix_len(uint8 lchip, uint8 ip_ver)
{
    if (p_duet2_calpm_master[lchip])
    {
        return p_duet2_calpm_master[lchip]->prefix_len[ip_ver];
    }

    return 0;
}

/* only hash key need push down */
int32
_sys_duet2_calpm_push_down_ad(uint8 lchip, sys_calpm_param_t* p_calpm_param)
{
    sys_calpm_prefix_t* p_calpm_prefix = p_calpm_param->p_calpm_prefix;
    sys_ipuc_tcam_data_t tcam_data = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] == INVALID_POINTER_OFFSET)
    {
        if (p_calpm_prefix->is_pushed)
        {
            return CTC_E_NONE;
        }
    }
    else
    {
        p_calpm_prefix->is_pushed = 0;
        p_calpm_prefix->is_mask_valid = 0;
        p_calpm_prefix->masklen_pushed = 0;

        return CTC_E_NONE;
    }

    tcam_data.opt_type = DO_ADD;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
    tcam_data.info = (void*)p_calpm_prefix;
    sys_duet2_ipuc_tcam_update_ad(lchip, &tcam_data);

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_add(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param, uint32 ad_index, void* p_wb_alpm_info)
{
    uint8 stage = 0;
    uint8 is_pointer = 0;
    uint8 write_tcam_key = 0;
    int32 ret = CTC_E_NONE;
    uint8 old_idx_mask[CALPM_TABLE_MAX] = {0};
    uint16 old_stage_base[CALPM_TABLE_MAX] = {INVALID_POINTER_OFFSET, INVALID_POINTER_OFFSET};
    sys_calpm_param_t calpm_param = {0};
    sys_calpm_tbl_t* p_table[CALPM_TABLE_MAX] = {NULL};
    sys_ipuc_tcam_data_t tcam_data = {0};
    sys_wb_calpm_info_t *p_wb_calpm_info = (sys_wb_calpm_info_t*)p_wb_alpm_info;
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_CALPM_INIT_CHECK;

    calpm_param.p_ipuc_param = &(p_sys_ipuc_param->param);
    calpm_param.ad_index = ad_index;
    calpm_param.key_index = p_wb_calpm_info ? p_wb_calpm_info->key_index : SYS_CALPM_PREFIX_INVALID_INDEX;

    if (p_ipuc_param->masklen < p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        return CTC_E_NONE;
    }

    SYS_CALPM_LOCK;
    CTC_ERROR_RETURN_CALPM_UNLOCK(_sys_duet2_calpm_alloc_prefix_index(lchip, &calpm_param, &write_tcam_key));

    is_pointer = p_ipuc_param->masklen > p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver];
    if (is_pointer)
    {
        for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
        {
            p_table[stage] = TABEL_GET_STAGE(stage, &calpm_param);
            if (!p_table[stage])
            {
                continue;
            }

            /* save old calpm info for delete later */
            old_stage_base[stage] = p_table[stage]->stage_base;
            old_idx_mask[stage] = p_table[stage]->idx_mask;
        }

        /* add calpm soft db */
        CTC_ERROR_GOTO(_sys_duet2_calpm_add_soft(lchip, &calpm_param, p_table), ret,  error0);

        if (p_wb_calpm_info)
        {
            ret =  _sys_duet2_calpm_reload_asic_table(lchip, p_table, p_wb_calpm_info);
            if (ret)
            {
                _sys_duet2_calpm_del_soft(lchip, &calpm_param, p_table);
                goto error0;
            }

            calpm_param.p_calpm_prefix->masklen_pushed = p_wb_calpm_info->masklen_pushed;
            calpm_param.p_calpm_prefix->is_mask_valid = p_wb_calpm_info->is_mask_valid;
            calpm_param.p_calpm_prefix->is_pushed = p_wb_calpm_info->is_pushed;

            if (write_tcam_key)
            {
                ret = _sys_duet2_calpm_db_add(lchip, calpm_param.p_calpm_prefix);
                if (ret)
                {
                    _sys_duet2_calpm_del_soft(lchip, &calpm_param, p_table);
                    goto error0;
                }
            }

            CTC_RETURN_CALPM_UNLOCK(CTC_E_NONE);
        }

        /* build calpm hard index */
        ret = _sys_duet2_calpm_calc_asic_table(lchip, p_table, old_stage_base, old_idx_mask);
        if (ret)
        {
            _sys_duet2_calpm_del_soft(lchip, &calpm_param, p_table);
            goto error0;
        }

        /* for error loopback */
        for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
        {
            p_table[stage] = TABEL_GET_STAGE(stage, &calpm_param);
            if (!p_table[stage])
            {
                continue;
            }

            /* save old calpm info for delete later */
            old_stage_base[stage] = p_table[stage]->stage_base;
            old_idx_mask[stage] = p_table[stage]->idx_mask;
        }

        CTC_ERROR_GOTO(_sys_duet2_calpm_write_stage_key(lchip, &calpm_param, CALPM_TABLE_MAX), ret, error1);
    }

    /* write ipuc prefix key entry */
    tcam_data.opt_type = DO_ADD;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
    tcam_data.ad_index = calpm_param.ad_index;
    tcam_data.masklen = p_ipuc_param->masklen;
    tcam_data.info = (void*)calpm_param.p_calpm_prefix;
    CTC_ERROR_GOTO(sys_duet2_ipuc_tcam_write_calpm_ad(lchip, &tcam_data), ret, error1);
    if (write_tcam_key)
    {
        CTC_ERROR_GOTO(sys_duet2_ipuc_tcam_write_calpm_key(lchip, &tcam_data), ret, error2);
    }

    CTC_ERROR_GOTO(_sys_duet2_calpm_push_down_ad(lchip, &calpm_param), ret, error3);

    if (write_tcam_key)
    {
        CTC_ERROR_GOTO(_sys_duet2_calpm_db_add(lchip, calpm_param.p_calpm_prefix), ret, error3);
    }

    SYS_CALPM_UNLOCK;

    return CTC_E_NONE;

error3:
    if (write_tcam_key)
    {
        tcam_data.opt_type = DO_DEL;
        sys_duet2_ipuc_tcam_write_calpm_key(lchip, &tcam_data);
    }

error2:
    tcam_data.ad_index = INVALID_NEXTHOP_OFFSET;
    sys_duet2_ipuc_tcam_write_calpm_ad(lchip, &tcam_data);

error1:
    if (is_pointer)
    {
        _sys_duet2_calpm_del_soft(lchip, &calpm_param, p_table);
        _sys_duet2_calpm_calc_asic_table(lchip, p_table, old_stage_base, old_idx_mask);

        _sys_duet2_calpm_write_stage_key(lchip, &calpm_param, CALPM_TABLE_MAX);
    }

error0:
    _sys_duet2_calpm_free_prefix_index(lchip, &calpm_param);

    SYS_CALPM_UNLOCK;

    return ret;
}

int32
sys_duet2_calpm_del(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param)
{
    int32 ret = CTC_E_NONE;
    uint8 stage = 0;
    uint8 old_idx_mask[CALPM_TABLE_MAX] = {0};
    uint16 old_stage_base[CALPM_TABLE_MAX] = {INVALID_POINTER_OFFSET, INVALID_POINTER_OFFSET};
    sys_calpm_param_t calpm_param = {0};
    sys_calpm_tbl_t* p_table[CALPM_TABLE_MAX] = {NULL};
    sys_ipuc_tcam_data_t tcam_data = {0};
    sys_calpm_key_type_t key_type = CALPM_TYPE_POINTER;
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_CALPM_INIT_CHECK;
    calpm_param.p_ipuc_param = &p_sys_ipuc_param->param;

    if (p_ipuc_param->masklen < p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        return CTC_E_NONE;
    }

    SYS_CALPM_LOCK;
    CTC_ERROR_RETURN_CALPM_UNLOCK(_sys_duet2_calpm_lookup_prefix(lchip, &calpm_param));
    if (!calpm_param.p_calpm_prefix)
    {
        CTC_ERROR_RETURN_CALPM_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (p_ipuc_param->masklen > p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
        {
            p_table[stage] = TABEL_GET_STAGE(stage, &calpm_param);
            if (!p_table[stage])
            {
                continue;
            }

            /* save old calpm info for delete later */
            old_stage_base[stage] = p_table[stage]->stage_base;
            old_idx_mask[stage] = p_table[stage]->idx_mask;
        }

        _sys_duet2_calpm_del_soft(lchip, &calpm_param, p_table);

        ret = _sys_duet2_calpm_calc_asic_table(lchip, p_table, old_stage_base, old_idx_mask);
        if (ret)
        {
            _sys_duet2_calpm_add_soft(lchip, &calpm_param, p_table);
            for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
            {
                if (p_table[stage])
                {
                    p_table[stage]->stage_base = old_stage_base[stage];
                }
            }

            CTC_ERROR_RETURN_CALPM_UNLOCK(ret);
        }

        /* write calpm key entry */
        _sys_duet2_calpm_write_stage_key(lchip, &calpm_param, CALPM_TABLE_MAX);
    }

    /* write hash key entry */
    if (p_ipuc_param->masklen == calpm_param.p_calpm_prefix->prefix_len)
    {
        key_type = CALPM_TYPE_NEXTHOP;
    }

    tcam_data.opt_type = DO_DEL;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
    tcam_data.key_index = calpm_param.p_calpm_prefix->key_index[key_type];
    tcam_data.ad_index = calpm_param.ad_index;
    tcam_data.masklen = p_ipuc_param->masklen;
    tcam_data.info = (void*)calpm_param.p_calpm_prefix;
    sys_duet2_ipuc_tcam_write_calpm_key(lchip, &tcam_data);

    /* free hash index */
    _sys_duet2_calpm_free_prefix_index(lchip, &calpm_param);

    if ((p_ipuc_param->masklen == calpm_param.p_calpm_prefix->prefix_len) &&
        (calpm_param.p_calpm_prefix->key_index[CALPM_TYPE_POINTER] != INVALID_POINTER_OFFSET))
    {
        _sys_duet2_calpm_push_down_ad(lchip, &calpm_param);
    }

    _sys_duet2_calpm_db_remove(lchip, calpm_param.p_calpm_prefix);

    SYS_CALPM_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_update(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param, uint32 ad_index)
{
    sys_calpm_param_t calpm_param = {0};
    sys_ipuc_tcam_data_t tcam_data = {0};
    uint16 old_ad_index = 0;
    int32 ret = CTC_E_NONE;
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_CALPM_INIT_CHECK;

    if (p_ipuc_param->masklen < p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        return CTC_E_NONE;
    }

    calpm_param.p_ipuc_param = p_ipuc_param;
    calpm_param.ad_index = ad_index;

    SYS_CALPM_LOCK;
    CTC_ERROR_RETURN_CALPM_UNLOCK(_sys_duet2_calpm_lookup_prefix(lchip, &calpm_param));
    if (!calpm_param.p_calpm_prefix)
    {
        CTC_ERROR_RETURN_CALPM_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (p_ipuc_param->masklen == p_duet2_calpm_master[lchip]->prefix_len[p_ipuc_param->ip_ver])
    {
        tcam_data.opt_type = DO_ADD;
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.ad_index = calpm_param.ad_index;
        tcam_data.masklen = p_ipuc_param->masklen;
        tcam_data.info = (void*)calpm_param.p_calpm_prefix;
        CTC_ERROR_RETURN_CALPM_UNLOCK(sys_duet2_ipuc_tcam_write_calpm_ad(lchip, &tcam_data));
    }

    CTC_ERROR_RETURN_CALPM_UNLOCK(_sys_duet2_calpm_update_tree_ad(lchip, &calpm_param, &old_ad_index));

    ret = _sys_duet2_calpm_write_stage_key(lchip, &calpm_param, CALPM_TABLE_MAX);
    if (ret)
    {
        calpm_param.ad_index = old_ad_index;
        _sys_duet2_calpm_update_tree_ad(lchip, &calpm_param, NULL);
    }

    SYS_CALPM_UNLOCK;

    return ret;
}

int32
sys_duet2_calpm_mapping_wb_master(uint8 lchip, uint8 sync)
{
    int32 ret = CTC_E_NONE;

    if (!p_duet2_calpm_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (sync)
    {
        ctc_wb_data_t wb_data;
        sys_wb_calpm_master_t  *p_wb_calpm_master = NULL;

        wb_data.buffer = mem_malloc(MEM_IPUC_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
        if (NULL == wb_data.buffer)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

        CTC_WB_INIT_DATA_T((&wb_data),sys_wb_calpm_master_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_ALPM_MASTER);

        p_wb_calpm_master = (sys_wb_calpm_master_t  *)wb_data.buffer;

        p_wb_calpm_master->lchip = lchip;
        p_wb_calpm_master->couple_mode = p_duet2_calpm_master[lchip]->couple_mode;
        p_wb_calpm_master->ipsa_enable = p_duet2_calpm_master[lchip]->ipsa_enable;
        p_wb_calpm_master->prefix_length = p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_4];

        wb_data.valid_cnt = 1;
        ret = ctc_wb_add_entry(&wb_data);

        mem_free(wb_data.buffer);
    }
    else
    {
        ctc_wb_query_t wb_query;
        sys_wb_calpm_master_t  wb_calpm_master = {0};

        wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
        if (NULL == wb_query.buffer)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

        CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_calpm_master_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_ALPM_MASTER);

        ret = ctc_wb_query_entry(&wb_query);
        if (ret)
        {
            mem_free(wb_query.buffer);
        }

        /*restore  calpm master*/
        if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query calpm master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
            mem_free(wb_query.buffer);

            return CTC_E_VERSION_MISMATCH;
        }

        sal_memcpy((uint8*)&wb_calpm_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

        if ((p_duet2_calpm_master[lchip]->couple_mode !=  wb_calpm_master.couple_mode) ||
            (p_duet2_calpm_master[lchip]->ipsa_enable !=  wb_calpm_master.ipsa_enable) ||
            (p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_4] !=  wb_calpm_master.prefix_length))
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,
                "calpm init and warmboot mismatch!sram couple mode: %u|%u ipsa enable: %u|%u prefix length: %u|%u\n",
                p_duet2_calpm_master[lchip]->couple_mode, wb_calpm_master.couple_mode,
                p_duet2_calpm_master[lchip]->ipsa_enable, wb_calpm_master.ipsa_enable,
                p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_4], wb_calpm_master.prefix_length);

            ret = CTC_E_VERSION_MISMATCH;
        }
        mem_free(wb_query.buffer);
    }

    return ret;
}

int32
sys_duet2_calpm_get_wb_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, void* p_wb_alpm_info)
{
    uint8 stage = 0;
    uint8 is_pointer = 0;
    sys_calpm_param_t calpm_param = {0};
    sys_calpm_tbl_t* p_table[CALPM_TABLE_MAX] = {NULL};
    sys_wb_calpm_info_t* p_wb_calpm_info = (sys_wb_calpm_info_t*)p_wb_alpm_info;

    CTC_PTR_VALID_CHECK(p_ipuc_param);
    CTC_PTR_VALID_CHECK(p_wb_calpm_info);

    sal_memset(p_wb_calpm_info, 0, sizeof(sys_wb_calpm_info_t));

    calpm_param.p_ipuc_param = p_ipuc_param;

    SYS_CALPM_LOCK;
    CTC_ERROR_RETURN_CALPM_UNLOCK(_sys_duet2_calpm_lookup_prefix(lchip, &calpm_param));
    if (!calpm_param.p_calpm_prefix)
    {
        CTC_ERROR_RETURN_CALPM_UNLOCK(CTC_E_NOT_EXIST);
    }

    is_pointer = (p_ipuc_param->masklen > calpm_param.p_calpm_prefix->prefix_len);
    p_wb_calpm_info->key_index = calpm_param.p_calpm_prefix->key_index[is_pointer];

    for (stage = CALPM_TABLE_INDEX0; stage < CALPM_TABLE_MAX; stage++)
    {
        p_wb_calpm_info->idx_mask[stage] = 0xFF;
        p_wb_calpm_info->stage_base[stage] = INVALID_POINTER_OFFSET;

        p_table[stage] = TABEL_GET_STAGE(stage, &calpm_param);
        if (!p_table[stage])
        {
            continue;
        }

        p_wb_calpm_info->idx_mask[stage] = p_table[stage]->idx_mask;
        p_wb_calpm_info->stage_base[stage] = p_table[stage]->stage_base;
        p_wb_calpm_info->masklen_pushed = calpm_param.p_calpm_prefix->masklen_pushed;
        p_wb_calpm_info->is_mask_valid = calpm_param.p_calpm_prefix->is_mask_valid;
        p_wb_calpm_info->is_pushed = calpm_param.p_calpm_prefix->is_pushed;
    }
    SYS_CALPM_UNLOCK;

    p_wb_calpm_info->rsv = 0;

    return CTC_E_NONE;
}

int32
sys_duet2_calpm_init(uint8 lchip, uint8 calpm_prefix8, uint8 ipsa_enable)
{
    uint16 max_size = 0;
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    sys_ftm_lpm_pipeline_info_t ftm_info;

    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_ftm_get_lpm_pipeline_info(lchip, &ftm_info));
    if (!ftm_info.pipeline0_size ||
        (ftm_info.pipeline0_size <= CALPM_SRAM_RSV_FOR_ARRANGE) ||
        (ftm_info.pipeline1_size && (ftm_info.pipeline1_size <= CALPM_SRAM_RSV_FOR_ARRANGE)))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;
    }

    if (NULL == p_duet2_calpm_master[lchip])
    {
        p_duet2_calpm_master[lchip] = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_master_t));
        if (NULL == p_duet2_calpm_master[lchip])
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }

        sal_memset(p_duet2_calpm_master[lchip], 0, sizeof(sys_calpm_master_t));
    }

    sal_mutex_create(&p_duet2_calpm_master[lchip]->mutex);
    if (NULL == p_duet2_calpm_master[lchip]->mutex)
    {
        mem_free(p_duet2_calpm_master[lchip]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        CTC_ERROR_RETURN(CTC_E_NO_MEMORY);
    }

    p_duet2_calpm_master[lchip]->ipsa_enable = ipsa_enable;
    if (ftm_info.pipeline1_size)
    {
        p_duet2_calpm_master[lchip]->couple_mode = 1;
    }

    p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_4] = calpm_prefix8 ? 8 : 16;
    p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_6] = 48;
    if ((p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_6] < 32) || (p_duet2_calpm_master[lchip]->prefix_len[CTC_IP_VER_6] > 112))
    {
        return CTC_E_INVALID_PARAM;
    }

    p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_4]   = ctc_hash_create(
                                    CALPM_PREFIX_IPV4_HASH_SIZE / CALPM_PREFIX_HASH_BLOCK_SIZE,
                                    CALPM_PREFIX_HASH_BLOCK_SIZE,
                                    (hash_key_fn)_sys_duet2_calpm_prefix_ipv4_key_make,
                                    (hash_cmp_fn)_sys_duet2_calpm_prefix_ipv4_key_cmp);

    p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_6]   = ctc_hash_create(
                                    CALPM_PREFIX_IPV6_HASH_SIZE / CALPM_PREFIX_HASH_BLOCK_SIZE,
                                    CALPM_PREFIX_HASH_BLOCK_SIZE,
                                    (hash_key_fn)_sys_duet2_calpm_prefix_ipv6_key_make,
                                    (hash_cmp_fn)_sys_duet2_calpm_prefix_ipv6_key_cmp);

    /*init opf for CALPM*/
    /* CALPM_SRAM_RSV_FOR_ARRANGE reserve for arrange */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if (p_duet2_calpm_master[lchip]->couple_mode)
    {
        CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_duet2_calpm_master[lchip]->opf_type_calpm, 2, "opf-calpm-pipeline"), ret, error0);

        opf.pool_index = 0;
        opf.pool_type = p_duet2_calpm_master[lchip]->opf_type_calpm;
        max_size = ftm_info.pipeline0_size - CALPM_SRAM_RSV_FOR_ARRANGE;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, max_size), ret, error1);
        p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0] = max_size;
        p_duet2_calpm_master[lchip]->start_stage_offset[CALPM_TABLE_INDEX0] = 0;

        opf.pool_index = 1;
        max_size = ftm_info.pipeline1_size - CALPM_SRAM_RSV_FOR_ARRANGE;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, ftm_info.pipeline0_size, max_size), ret, error1);
        p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX1] = max_size;
        p_duet2_calpm_master[lchip]->start_stage_offset[CALPM_TABLE_INDEX1] = ftm_info.pipeline0_size;
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_duet2_calpm_master[lchip]->opf_type_calpm, 1, "opf-calpm-pipeline"), ret, error0);

        opf.pool_index = 0;
        opf.pool_type = p_duet2_calpm_master[lchip]->opf_type_calpm;
        max_size = ftm_info.pipeline0_size - CALPM_SRAM_RSV_FOR_ARRANGE;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, max_size), ret, error1);
        p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX0] = max_size;
        p_duet2_calpm_master[lchip]->start_stage_offset[CALPM_TABLE_INDEX0] = 0;
        p_duet2_calpm_master[lchip]->max_stage_size[CALPM_TABLE_INDEX1] = 0;
        p_duet2_calpm_master[lchip]->start_stage_offset[CALPM_TABLE_INDEX1] = 0;
    }

    p_duet2_calpm_master[lchip]->version_en[CTC_IP_VER_4] = TRUE;
    p_duet2_calpm_master[lchip]->version_en[CTC_IP_VER_6] = TRUE;
    p_duet2_calpm_master[lchip]->dma_enable = FALSE;

    return CTC_E_NONE;

error1:
    sys_usw_opf_deinit(lchip, p_duet2_calpm_master[lchip]->opf_type_calpm);

error0:
    if (p_duet2_calpm_master[lchip])
    {
        mem_free(p_duet2_calpm_master[lchip]);
    }

    return ret;
}

int32
sys_duet2_calpm_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_duet2_calpm_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_opf_deinit(lchip, p_duet2_calpm_master[lchip]->opf_type_calpm);

    ctc_hash_traverse(p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_4], (hash_traversal_fn)_sys_duet2_calm_free_node_data, NULL);
    ctc_hash_free(p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_4]);
    ctc_hash_traverse(p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_6], (hash_traversal_fn)_sys_duet2_calm_free_node_data, NULL);
    ctc_hash_free(p_duet2_calpm_master[lchip]->calpm_prefix[CTC_IP_VER_6]);

    if (p_duet2_calpm_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_duet2_calpm_master[lchip]->mutex);
    }

    mem_free(p_duet2_calpm_master[lchip]);

    return CTC_E_NONE;
}

#endif


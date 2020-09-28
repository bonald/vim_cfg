/**
 @file sys_usw_qos_policer.c

 @date 2009-10-16

 @version v2.0

*/

/****************************************************************************
  *
  * Header Files
  *
  ****************************************************************************/
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_qos.h"
#include "ctc_debug.h"
#include "ctc_hash.h"
#include "ctc_spool.h"
#include "ctc_warmboot.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_port.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_ftm.h"
#include "sys_usw_qos.h"
#include "sys_usw_vlan.h"
#include "sys_usw_stats.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"

#include "drv_api.h"

/****************************************************************************
  *
  * Defines and Macros
  *
  ****************************************************************************/



#define SYS_QOS_POLICER_HASH_TBL_SIZE         512
#define SYS_QOS_POLICER_HASH_BLOCK_SIZE       (MCHIP_CAP(SYS_CAP_QOS_POLICER_POLICER_NUM) / SYS_QOS_POLICER_HASH_TBL_SIZE)

#define SYS_QOS_POLICER_PROF_HASH_TBL_SIZE    16
#define SYS_QOS_POLICER_PROF_HASH_BLOCK_SIZE  (MCHIP_CAP(SYS_CAP_QOS_POLICER_PROFILE_NUM) / SYS_QOS_POLICER_PROF_HASH_TBL_SIZE)

#define SYS_QOS_POLICER_COPP_PROF_HASH_TBL_SIZE    2
#define SYS_QOS_POLICER_COPP_PROF_HASH_BLOCK_SIZE  (MCHIP_CAP(SYS_CAP_QOS_POLICER_COPP_PROFILE_NUM) / SYS_QOS_POLICER_COPP_PROF_HASH_TBL_SIZE)

#define SYS_QOS_POLICER_MFP_PROF_HASH_TBL_SIZE    4
#define SYS_QOS_POLICER_MFP_PROF_HASH_BLOCK_SIZE  (MCHIP_CAP(SYS_CAP_QOS_POLICER_MFP_PROFILE_NUM) / SYS_QOS_POLICER_MFP_PROF_HASH_TBL_SIZE)

#define SYS_QOS_POLICER_ACTION_PROF_HASH_TBL_SIZE    4
#define SYS_QOS_POLICER_ACTION_PROF_HASH_BLOCK_SIZE  (MCHIP_CAP(SYS_CAP_QOS_POLICER_ACTION_PROFILE_NUM) / SYS_QOS_POLICER_ACTION_PROF_HASH_TBL_SIZE)


#define SYS_MAX_POLICER_GRANULARITY_RANGE_NUM       8
#define SYS_QOS_POLICER_MAX_COUNT     (0xFFF<<0xD)
#define SYS_QOS_MFP_MAX_COUNT     67108
#define SYS_QOS_MFP_MAX_RATE      67108856
#define SYS_POLICER_GRAN_RANAGE_NUM   8
#define SYS_QOS_VLAN_RESERVE_NUM     4

/* max port policer num in both direction */


enum sys_policer_cnt_s
{
    SYS_POLICER_CNT_PORT_POLICER,
    SYS_POLICER_CNT_FLOW_POLICER,
    SYS_POLICER_CNT_VLAN_POLICER,
    SYS_POLICER_CNT_MAX
}sys_policer_cnt_t;

/* get port policer offset */
#define SYS_QOS_PORT_POLICER_INDEX(lport, dir, phb_offset) \
    ((lport & 0xff) + (phb_offset) \
    + p_usw_qos_policer_master[lchip]->port_policer_base[dir])

#define MAX(a, b) (((a)>(b))?(a):(b))

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

/****************************************************************************
  *
  * Global and Declaration
  *
  ****************************************************************************/

#define SYS_QOS_POLICER_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(qos, policer, QOS_PLC_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

/*********************************************************************
  *
  * data structure definition
  *
  *********************************************************************/


enum sys_qos_policer_level_e
{
    SYS_QOS_POLICER_IPE_POLICING_0,
    SYS_QOS_POLICER_IPE_POLICING_1,
    SYS_QOS_POLICER_EPE_POLICING_0,
    SYS_QOS_POLICER_EPE_POLICING_1
};
typedef enum sys_qos_policer_level_e  sys_qos_policer_level_t;

  /**
 @brief  qos prof entry data structure
*/
struct sys_qos_policer_profile_s
{
    uint8   is_bucketY;
    uint8   policer_lvl;

    uint16  threshold;
    uint8   thresholdShift;

    uint8   rateShift;
    uint16  rate;
    uint8   rateFrac;
    uint8   rateMaxShift;
    uint16  rateMax;

    uint8   profile_id;
    uint8   rsv[3];
};
typedef struct sys_qos_policer_profile_s sys_qos_policer_profile_t;

/**
 @brief  qos policer action data structure
*/
struct sys_qos_policer_action_s
{
    uint32 flag;                /*refer to ctc_qos_policer_action_flag_t*/
    uint8  dir;                 /*refer to ctc_direction_t*/
    uint8  prio_green;
    uint8  prio_yellow;
    uint8  prio_red;
    uint8  discard_green;
    uint8  discard_yellow;
    uint8  discard_red;
    uint8  profile_id;
};
typedef struct sys_qos_policer_action_s sys_qos_policer_action_t;

struct sys_qos_policer_copp_profile_s
{
    uint8   is_bucketY;
    uint16  threshold;
    uint8   thresholdShift;

    uint16  rate;
    uint8   rateShift;
    uint8   rateFrac;

    uint8   profile_id;
    uint8   rsv[3];
};
typedef struct sys_qos_policer_copp_profile_s sys_qos_policer_copp_profile_t;

struct sys_qos_policer_s
{
    uint8  type; /*ctc_qos_policer_type_t*/
    uint8  dir;   /*ctc_direction_t */
    uint16 id;

    uint8  level;
    uint8  split_en;

    uint8  hbwp_en;
    uint8  is_pps;
    uint16 cos_bitmap; /*bitmap of hbwp enable*/
    uint8  cos_index;
    uint8  sf_en;
    uint8  cf_total_dis;

    uint8  policer_mode;
    uint8  cf_en;
    uint8  is_color_aware;
    uint8  use_l3_length;

    uint8  drop_color;
    uint8  entry_size;

    uint8  stats_en;
    uint8  stats_mode;
    uint8  stats_num;
    uint8  color_merge_mode;

    uint8  stats_useX;
    uint8  stats_useY;

    uint32 cir;
    uint32 cbs;

    uint32 pir;
    uint32 pbs;

    uint32 cir_max;
    uint32 pir_max;

    uint8  policer_lvl;

    uint8  prio_green;
    uint8  prio_yellow;
    uint8  prio_red;
    uint32 flag;                /*refer to ctc_qos_policer_action_flag_t*/

    uint8 sys_policer_type;     /*refer to sys_qos_policer_type_t*/
    uint8 is_installed;
    uint16 policer_ptr;

    uint32 stats_ptr[8];
    sys_qos_policer_profile_t* p_sys_profileX[8];
    sys_qos_policer_profile_t* p_sys_profileY[8];
    sys_qos_policer_action_t* p_sys_action_profile[8];
    sys_qos_policer_copp_profile_t* p_sys_copp_profile;
};
typedef struct sys_qos_policer_s sys_qos_policer_t;

struct sys_qos_policer_granularity_s
{
    uint32 max_rate;        /* unit is Mbps */
    uint32 granularity;     /* unit is Kbps */
};
typedef struct sys_qos_policer_granularity_s sys_qos_policer_granularity_t;

struct sys_qos_policer_action_profile_s
{
    uint8 decX;
    uint8 decY;
    uint8 newColor;
};
typedef struct sys_qos_policer_action_profile_s sys_qos_policer_action_profile_t;

struct sys_qos_policer_share_profile_s
{
    uint8 dec0X;
    uint8 dec0Y;
    uint8 dec1X;
    uint8 dec1Y;
    uint8 newColor;
};
typedef struct sys_qos_policer_share_profile_s sys_qos_policer_share_profile_t;

struct sys_qos_policer_master_s
{
    ctc_hash_t* p_policer_hash;
    ctc_spool_t* p_profile_pool;
    ctc_spool_t* p_action_profile_pool;
    ctc_spool_t* p_copp_profile_pool;

    sys_qos_policer_granularity_t policer_gran[SYS_POLICER_GRAN_RANAGE_NUM];
    uint16  port_policer_base[CTC_BOTH_DIRECTION];
    uint16  vlan_policer_base[CTC_BOTH_DIRECTION];
    uint8   max_cos_level;
    uint8   flow_plus_service_policer_en;
    uint8   opf_type_policer_config;
    uint8   opf_type_ipe_policer0_profile;
    uint8   opf_type_ipe_policer1_profile;
    uint8   opf_type_epe_policer0_profile;
    uint8   opf_type_epe_policer1_profile;
    uint8   opf_type_copp_config;
    uint8   opf_type_copp_profile;
    uint8   opf_type_mfp_profile;
    uint8   opf_type_action_profile;
    uint16  policer_count[SYS_POLICER_CNT_MAX];
    uint16  update_gran_base;
    uint32  policer_update_freq;
    uint32  copp_update_freq;
    uint32  mfp_update_freq;
    uint16 ingress_vlan_policer_num;
    uint16 egress_vlan_policer_num;
    uint8  policer_merge_mode;
};
typedef struct sys_qos_policer_master_s sys_qos_policer_master_t;

sys_qos_policer_master_t* p_usw_qos_policer_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
  *
  * Function
  *
  ****************************************************************************/

/**
@brief Policer hash key hook.
*/
uint32
_sys_usw_policer_hash_make(sys_qos_policer_t* backet)
{
    uint32 val = 0;
    uint8* data = NULL;
    uint8   length = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!backet)
    {
        return 0;
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "backet->type = %d, backet->dir = %d, backet->id = %d\n",
                             backet->type, backet->dir, backet->id);

    val = (backet->type << 24) | (backet->dir << 16) | (backet->id);

    data = (uint8*)&val;
    length = sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

/**
 @brief Policer hash comparison hook.
*/
bool
_sys_usw_policer_hash_cmp(sys_qos_policer_t* p_policer1,
                                sys_qos_policer_t* p_policer2)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_policer1 || !p_policer2)
    {
        return FALSE;
    }

    if ((p_policer1->type == p_policer2->type) &&
        (p_policer1->dir == p_policer2->dir) &&
        (p_policer1->id == p_policer2->id))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_policer_hash_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_policer_profile_hash_make(sys_qos_policer_profile_t* p_prof)
{
    uint8* data = (uint8*)(p_prof);
    uint8   length = sizeof(sys_qos_policer_profile_t) - sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

STATIC bool
_sys_usw_policer_profile_hash_cmp(sys_qos_policer_profile_t* p_prof1,
                                        sys_qos_policer_profile_t* p_prof2)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    if (!sal_memcmp((uint8*)(p_prof1), (uint8*)(p_prof2), sizeof(sys_qos_policer_profile_t)-sizeof(uint32)))
    {
        return TRUE;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_policer_alloc_profileId(sys_qos_policer_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_node->is_bucketY;

    switch(p_node->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_ipe_policer0_profile;
        break;
    case SYS_QOS_POLICER_IPE_POLICING_1:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_ipe_policer1_profile;
        break;
    case SYS_QOS_POLICER_EPE_POLICING_0:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_epe_policer0_profile;
        break;
    case SYS_QOS_POLICER_EPE_POLICING_1:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_epe_policer1_profile;
        break;
    }
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_policer_free_profileId(sys_qos_policer_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_node->is_bucketY;

    switch(p_node->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_ipe_policer0_profile;
        break;
    case SYS_QOS_POLICER_IPE_POLICING_1:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_ipe_policer1_profile;
        break;
    case SYS_QOS_POLICER_EPE_POLICING_0:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_epe_policer0_profile;
        break;
    case SYS_QOS_POLICER_EPE_POLICING_1:
        opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_epe_policer1_profile;
        break;
    }

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_policer_action_profile_hash_make(sys_qos_policer_action_t* p_prof)
{
    uint8* data = (uint8*)(p_prof);
    uint8   length = sizeof(sys_qos_policer_action_t) - sizeof(uint8);

    return ctc_hash_caculate(length, data);
}

STATIC bool
_sys_usw_policer_action_profile_hash_cmp(sys_qos_policer_action_t* p_prof1,
                                        sys_qos_policer_action_t* p_prof2)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    if (!sal_memcmp((uint8*)(p_prof1), (uint8*)(p_prof2), sizeof(sys_qos_policer_action_t)-sizeof(uint8)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_policer_action_alloc_profileId(sys_qos_policer_action_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_node->dir;
    opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_action_profile;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_policer_action_free_profileId(sys_qos_policer_action_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_node->dir;
    opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_action_profile;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_policer_copp_profile_hash_make(sys_qos_policer_copp_profile_t* p_prof)
{
    uint8* data = (uint8*)(p_prof);
    uint8 length = sizeof(sys_qos_policer_copp_profile_t) - sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

STATIC bool
_sys_usw_policer_copp_profile_hash_cmp(sys_qos_policer_copp_profile_t* p_prof1,
                                        sys_qos_policer_copp_profile_t* p_prof2)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }
    if (!sal_memcmp((uint8*)(p_prof1), (uint8*)(p_prof2), sizeof(sys_qos_policer_copp_profile_t)-sizeof(uint32)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_policer_copp_alloc_profileId(sys_qos_policer_copp_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_node->is_bucketY;
    opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_copp_profile;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_policer_copp_free_profileId(sys_qos_policer_copp_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_node->is_bucketY;
    opf.pool_type  = p_usw_qos_policer_master[*p_lchip]->opf_type_copp_profile;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_init_reg(uint8 lchip, uint32 divide_factor)
{
    uint32 cmd         = 0;
    uint8  priority    = 0;
    uint32 ipe_policer0_num    = 0;
    uint32 ipe_policer1_num    = 0;
    uint32 epe_policer0_num    = 0;
    uint32 epe_policer1_num    = 0;
    uint32 field_val = 0;

    IpePolicing0EngineCtl_m ipe_policer0_ctl;
    IpePolicing1EngineCtl_m ipe_policer1_ctl;
    EpePolicing0EngineCtl_m epe_policer0_ctl;
    EpePolicing1EngineCtl_m epe_policer1_ctl;
    IpePolicingCtl_m ipe_policing_ctl;
    EpePolicingCtl_m epe_policing_ctl;
    DsIpePolicingPhbOffset_m ipe_phb;
    DsEpePolicingPhbOffset_m epe_phb;
    RefDivPolicingUpdPulse_m policer_update_pulse;

    sal_memset(&ipe_policer0_ctl, 0, sizeof(IpePolicing0EngineCtl_m));
    sal_memset(&ipe_policer1_ctl, 0, sizeof(IpePolicing1EngineCtl_m));
    sal_memset(&epe_policer0_ctl, 0, sizeof(EpePolicing0EngineCtl_m));
    sal_memset(&epe_policer1_ctl, 0, sizeof(EpePolicing1EngineCtl_m));
    sal_memset(&ipe_policing_ctl, 0, sizeof(IpePolicingCtl_m));
    sal_memset(&epe_policing_ctl, 0, sizeof(EpePolicingCtl_m));
    sal_memset(&ipe_phb, 0, sizeof(DsIpePolicingPhbOffset_m));
    sal_memset(&epe_phb, 0, sizeof(DsEpePolicingPhbOffset_m));

    SetRefDivPolicingUpdPulse(V, cfgRefDivIpePolicing0UpdPulse_f, &policer_update_pulse, divide_factor);
    SetRefDivPolicingUpdPulse(V, cfgResetDivIpePolicing0UpdPulse_f, &policer_update_pulse, 0);
    SetRefDivPolicingUpdPulse(V, cfgRefDivIpePolicing1UpdPulse_f, &policer_update_pulse, divide_factor);
    SetRefDivPolicingUpdPulse(V, cfgResetDivIpePolicing1UpdPulse_f, &policer_update_pulse, 0);
    SetRefDivPolicingUpdPulse(V, cfgRefDivEpePolicing0UpdPulse_f, &policer_update_pulse, divide_factor);
    SetRefDivPolicingUpdPulse(V, cfgResetDivEpePolicing0UpdPulse_f, &policer_update_pulse, 0);
    SetRefDivPolicingUpdPulse(V, cfgRefDivEpePolicing1UpdPulse_f, &policer_update_pulse, divide_factor);
    SetRefDivPolicingUpdPulse(V, cfgResetDivEpePolicing1UpdPulse_f, &policer_update_pulse, 0);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &policer_update_pulse));

    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing0Config_t, &ipe_policer0_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing1Config_t, &ipe_policer1_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing0Config_t, &epe_policer0_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing1Config_t, &epe_policer1_num);

    SetIpePolicing0EngineCtl(V, ipePolicing0EngineCtlInterval_f, &ipe_policer0_ctl, 2);
    SetIpePolicing0EngineCtl(V, ipePolicing0EngineCtlMaxPtr_f, &ipe_policer0_ctl, ipe_policer0_num - 1);
    SetIpePolicing0EngineCtl(V, ipePolicing0EngineCtlRefreshEn_f, &ipe_policer0_ctl, 1);
    SetIpePolicing0EngineCtl(V, ipePolicing0EngineCtlBucketPassConditionMode_f, &ipe_policer0_ctl, 1);
    cmd = DRV_IOW(IpePolicing0EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_policer0_ctl));

    SetIpePolicing1EngineCtl(V, ipePolicing1EngineCtlInterval_f, &ipe_policer1_ctl, 2);
    SetIpePolicing1EngineCtl(V, ipePolicing1EngineCtlMaxPtr_f, &ipe_policer1_ctl, ipe_policer1_num - 1);
    SetIpePolicing1EngineCtl(V, ipePolicing1EngineCtlRefreshEn_f, &ipe_policer1_ctl, 1);
    SetIpePolicing1EngineCtl(V, ipePolicing1EngineCtlBucketPassConditionMode_f, &ipe_policer1_ctl, 1);
    cmd = DRV_IOW(IpePolicing1EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_policer1_ctl));

    SetEpePolicing0EngineCtl(V, epePolicing0EngineCtlInterval_f, &epe_policer0_ctl, 2);
    SetEpePolicing0EngineCtl(V, epePolicing0EngineCtlMaxPtr_f, &epe_policer0_ctl, epe_policer0_num - 1);
    SetEpePolicing0EngineCtl(V, epePolicing0EngineCtlRefreshEn_f, &epe_policer0_ctl, 1);
    SetEpePolicing0EngineCtl(V, epePolicing0EngineCtlBucketPassConditionMode_f, &epe_policer0_ctl, 1);
    cmd = DRV_IOW(EpePolicing0EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_policer0_ctl));

    SetEpePolicing1EngineCtl(V, epePolicing1EngineCtlInterval_f, &epe_policer1_ctl, 2);
    SetEpePolicing1EngineCtl(V, epePolicing1EngineCtlMaxPtr_f, &epe_policer1_ctl, epe_policer1_num - 1);
    SetEpePolicing1EngineCtl(V, epePolicing1EngineCtlRefreshEn_f, &epe_policer1_ctl, 1);
    SetEpePolicing1EngineCtl(V, epePolicing1EngineCtlBucketPassConditionMode_f, &epe_policer1_ctl, 1);
    cmd = DRV_IOW(EpePolicing1EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_policer1_ctl));

    SetIpePolicingCtl(V, ipgEnPolicing_f,&ipe_policing_ctl,0);
    SetIpePolicingCtl(V, minLengthCheckEn_f,&ipe_policing_ctl,0);
    SetIpePolicingCtl(V, portPolicerBase_f,&ipe_policing_ctl,p_usw_qos_policer_master[lchip]->port_policer_base[CTC_INGRESS]);
    SetIpePolicingCtl(V, portPolicerEn_f,&ipe_policing_ctl,1);
    SetIpePolicingCtl(V, portPolicerSplitModeEn_f,&ipe_policing_ctl,0);
    SetIpePolicingCtl(V, portPolicerLevelSel_f, &ipe_policing_ctl, 0);
    SetIpePolicingCtl(V, vlanPolicerBase_f,&ipe_policing_ctl,p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_INGRESS]);
    SetIpePolicingCtl(V, vlanPolicerEn_f, &ipe_policing_ctl, 1);
    SetIpePolicingCtl(V, vlanPolicerSplitModeEn_f, &ipe_policing_ctl, 0);
    SetIpePolicingCtl(V, vlanPolicerLevelSel_f, &ipe_policing_ctl, 1);
    SetIpePolicingCtl(V, mfpDisablePolicer_f, &ipe_policing_ctl, 0);
    SetIpePolicingCtl(V, policerPtr1ForceDisableSplit_f, &ipe_policing_ctl, 1);
    SetIpePolicingCtl(V, muxLengthAdjustDisable_f, &ipe_policing_ctl, 0);
    cmd = DRV_IOW(IpePolicingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_policing_ctl));

    SetEpePolicingCtl(V, ipgEnPolicing_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, minLengthCheckEn_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, portPolicerBase_f, &epe_policing_ctl, p_usw_qos_policer_master[lchip]->port_policer_base[CTC_EGRESS]);
    SetEpePolicingCtl(V, portPolicerEn_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, portPolicerLevelSel_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, portPolicerSplitModeEn_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, vlanPolicerBase_f, &epe_policing_ctl, p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_EGRESS]);
    SetEpePolicingCtl(V, vlanPolicerEn_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, vlanPolicerSplitModeEn_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, vlanPolicerLevelSel_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, mfpDisablePolicer_f, &epe_policing_ctl, 0);
    SetEpePolicingCtl(V, policeIncludeSecTag_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, policer0StatsCareDiscard_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, policer1StatsCareDiscard_f, &epe_policing_ctl, 1);
    SetEpePolicingCtl(V, muxLengthAdjustDisable_f, &epe_policing_ctl, 1);
    cmd = DRV_IOW(EpePolicingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_policing_ctl));

    /*reset 0*/
    for (priority = 0; priority < 16; priority++)
    {
        cmd = DRV_IOW(DsIpePolicingPhbOffset_t, DsIpePolicingPhbOffset_phbOffset_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, priority, cmd, &ipe_phb));

        cmd = DRV_IOW(DsEpePolicingPhbOffset_t, DsEpePolicingPhbOffset_phbOffset_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, priority, cmd, &epe_phb));
    }

    field_val = 1;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_policer0StatsCareDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_policer0StatsInGlobal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_policer1StatsCareDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_policer1StatsInGlobal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_coppStatsCareDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_coppStatsInGlobal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if(p_usw_qos_policer_master[lchip]->policer_merge_mode == 1)
    {
        cmd = DRV_IOW(IpePolicingLinkCtl_t, IpePolicingLinkCtl_flowFirst_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(IpePolicingLinkCtl_t, IpePolicingLinkCtl_legacyModeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(IpePolicingLinkCtl_t, IpePolicingLinkCtl_sequentialPolicing_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        cmd = DRV_IOW(EpePolicingLinkCtl_t, EpePolicingLinkCtl_flowFirst_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpePolicingLinkCtl_t, EpePolicingLinkCtl_legacyModeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpePolicingLinkCtl_t, EpePolicingLinkCtl_sequentialPolicing_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_init_policer_mode(uint8 lchip)
{
    uint32 cmd             = 0;
    uint8 policerMode      = 0;
    uint8 color            = 0;
    uint8 signX            = 0;
    uint8 signY            = 0;
    uint8 step             = 0;
    uint8 line             = 0;
    uint8 index            = 0;

    DsIpePolicing0ActionProfile_m  ipe_policer0_action_profile;
    DsIpePolicing1ActionProfile_m  ipe_policer1_action_profile;
    DsEpePolicing0ActionProfile_m  epe_policer0_action_profile;
    DsEpePolicing1ActionProfile_m  epe_policer1_action_profile;
    sys_qos_policer_action_profile_t ActionProfile[3][12] =
      {{{0,0,1},  /*RFC2697*/
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {1,0,3},
        {1,0,3},
        {0,1,2},
        {0,0,1}},

       {{0,0,1},  /*RFC2698*/
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {1,1,3},
        {0,0,1},
        {0,1,2},
        {0,0,1}},

       {{0,0,1}, /*RFC4115*/
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {0,1,2},
        {0,0,1},
        {1,0,3},
        {1,0,3},
        {0,1,2},
        {0,0,1}},
      };

    sal_memset(&ipe_policer0_action_profile, 0, sizeof(DsIpePolicing0ActionProfile_m));
    sal_memset(&ipe_policer1_action_profile, 0, sizeof(DsIpePolicing1ActionProfile_m));
    sal_memset(&epe_policer0_action_profile, 0, sizeof(DsEpePolicing0ActionProfile_m));
    sal_memset(&epe_policer1_action_profile, 0, sizeof(DsEpePolicing1ActionProfile_m));



    /* policerMode0 :SrTcm policer(RFC2697); policerMode1:trTcm policer(RFC2698); policerMode2:Modify trTcm policer(RFC4115)
       policerMode3: reserved*/

    /*ActionProfileId 3 reserved*/
    cmd = DRV_IOW(DsIpePolicing0ActionProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 3, cmd, &ipe_policer0_action_profile));
    cmd = DRV_IOW(DsIpePolicing1ActionProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 3, cmd, &ipe_policer1_action_profile));
    cmd = DRV_IOW(DsEpePolicing0ActionProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 3, cmd, &epe_policer0_action_profile));
    cmd = DRV_IOW(DsEpePolicing1ActionProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 3, cmd, &epe_policer1_action_profile));

    /*config IpePolicing0ActionProfile*/
    /*
    index(3,0) = (color-1) << 2 + (failX, failY)
    color      = DsMeterActionProfile.g[index(3, 0)].color(1, 0);
    */
    for (policerMode = 0; policerMode < 3; policerMode++)
    {
        /*(color-1) << 2 + (failX, failY)*/
        for (index = 0; index < 3*2*2; index++)
        {
            signY = index % 2;
            signX = (index >> 1) % 2;
            color = (index >> 2) % 3;
            line = (color << 2) + (signX << 1) + signY;
            step = ((color << 2) + (signX << 1) + signY) * 3;

            SetDsIpePolicing0ActionProfile(V, g_0_decX_f + step, &ipe_policer0_action_profile, ActionProfile[policerMode][line].decX);
            SetDsIpePolicing0ActionProfile(V, g_0_decY_f + step, &ipe_policer0_action_profile, ActionProfile[policerMode][line].decY);
            SetDsIpePolicing0ActionProfile(V, g_0_color_f + step, &ipe_policer0_action_profile, ActionProfile[policerMode][line].newColor);

            SetDsIpePolicing1ActionProfile(V, g_0_decX_f + step, &ipe_policer1_action_profile, ActionProfile[policerMode][line].decX);
            SetDsIpePolicing1ActionProfile(V, g_0_decY_f + step, &ipe_policer1_action_profile, ActionProfile[policerMode][line].decY);
            SetDsIpePolicing1ActionProfile(V, g_0_color_f + step, &ipe_policer1_action_profile, ActionProfile[policerMode][line].newColor);

            SetDsEpePolicing0ActionProfile(V, g_0_decX_f + step, &epe_policer0_action_profile, ActionProfile[policerMode][line].decX);
            SetDsEpePolicing0ActionProfile(V, g_0_decY_f + step, &epe_policer0_action_profile, ActionProfile[policerMode][line].decY);
            SetDsEpePolicing0ActionProfile(V, g_0_color_f + step, &epe_policer0_action_profile, ActionProfile[policerMode][line].newColor);

            SetDsEpePolicing1ActionProfile(V, g_0_decX_f + step, &epe_policer1_action_profile, ActionProfile[policerMode][line].decX);
            SetDsEpePolicing1ActionProfile(V, g_0_decY_f + step, &epe_policer1_action_profile, ActionProfile[policerMode][line].decY);
            SetDsEpePolicing1ActionProfile(V, g_0_color_f + step, &epe_policer1_action_profile, ActionProfile[policerMode][line].newColor);
        }

        cmd = DRV_IOW(DsIpePolicing0ActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policerMode, cmd, &ipe_policer0_action_profile));

        cmd = DRV_IOW(DsIpePolicing1ActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policerMode, cmd, &ipe_policer1_action_profile));

        cmd = DRV_IOW(DsEpePolicing0ActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policerMode, cmd, &epe_policer0_action_profile));

        cmd = DRV_IOW(DsEpePolicing1ActionProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policerMode, cmd, &epe_policer1_action_profile));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_set_policer_mode(uint8 lchip,
                                                        uint8 shareMode,
                                                        uint8 index,
                                                        uint8 splitEn_level_0,
                                                        uint8 splitEn_level_1)
{
    uint32 cmd = 0;
    uint8  color_level_0 = 0;
    uint8  color_level_1 = 0;
    uint8  profile_id = 0;
    uint8  profile_idx = 0;
    uint8  step = 0;
    uint8  isY_level_0 = 0;
    uint8  isY_level_1 = 0;
    uint8  color_level_0_isgreen = 0;
    uint8  color_level_1_isgreen = 0;
    DsIpePolicingSharingProfile_m  ipe_policer_share_profile;
    DsEpePolicingSharingProfile_m  epe_policer_share_profile;
    sys_qos_policer_share_profile_t* p_share_profile = NULL;

    /* modify share mode 1 for GG mode
       {{0,0,0,0,1},
        {0,0,0,0,1},
        {0,0,0,0,1},
        {0,0,0,0,1},
        {0,1,0,1,2},
        {0,1,1,0,2},
        {0,0,0,0,1},
        {1,1,0,1,2},
        {1,0,1,0,3}},*/
    /*modify trTcm mode*/
    sys_qos_policer_share_profile_t ShareProfile_trTcm[3][9] =
      {{{0,0,0,0,1},
        {0,1,0,1,2},
        {1,0,1,0,3},
        {0,1,0,0,2},
        {0,1,0,1,2},
        {1,0,1,0,3},
        {1,0,0,0,3},
        {1,0,0,1,3},
        {1,0,1,0,3}},

       {{0,0,0,0,1},  /* share mode 1 for bcm mode*/
        {0,0,0,0,1},
        {0,0,0,0,1},
        {0,0,0,0,1},
        {0,1,0,1,2},
        {0,1,0,1,2},
        {0,0,0,0,1},
        {0,1,0,1,2},
        {1,0,1,0,3}},

       {{0,0,0,0,1},
        {0,0,0,0,1},
        {1,0,1,0,3},
        {0,0,0,0,1},
        {0,1,0,1,2},
        {1,0,1,0,3},
        {0,0,0,0,1},
        {0,1,0,1,2},
        {1,0,1,0,3}},
      };

    /*flow mode*/
    sys_qos_policer_share_profile_t ShareProfile_flow[3][6] =
      {{{0,0,0,0,1},
        {1,1,1,1,3},
        {1,1,0,0,3},
        {1,1,1,1,3},},

       {{0,0,0,0,1},
        {0,0,0,0,1},
        {0,0,0,0,1},
        {1,1,1,1,3},},

       {{0,0,0,0,1},
        {0,0,0,0,1},
        {1,1,1,0,3},
        {0,0,0,0,1},
        {1,1,0,1,2},
        {1,1,1,0,3},},
      };

    sal_memset(&ipe_policer_share_profile, 0, sizeof(DsIpePolicingSharingProfile_m));
    sal_memset(&epe_policer_share_profile, 0, sizeof(DsEpePolicingSharingProfile_m));

    /** split_en = 0 :
    sharingProfId(4, 0) = (!splitEn_0) << 4 + (color_0-1) << 2 + shareMode_0;
    sharingProfIdx(2, 0) = (!splitEn_1) << 2 + (color_1 - 1)
    color(1, 0)     = DsPolicingSharingProfile.g[sharingProfIdx(2, 0)].color(1, 0);*/

    /** split_en = 1 :
    sharingProfId(4, 0) = (!splitEn_0) << 4 + meterPtr(0) << 3 + (color_is_green << 2) + shareMode_0;
    sharingProfIdx(2, 0) = (!splitEn_1) << 2 + (color_is_green << 1) + meterPtr(0)
    color(1, 0)     = DsPolicingSharingProfile.g[sharingProfIdx(2, 0)].color(1, 0);*/

    /*splitEn_level_0=0; splitEn_level_1=0 -> index=(color0,color1)
      splitEn_level_0=1; splitEn_level_1=1 -> index=(color0_isgreen,isY_0,color1_isgreen,isY_1)
      splitEn_level_0=1; splitEn_level_1=0 -> index=(color1, isY_level_0, color0_isgreen)*/

    if (splitEn_level_0 && splitEn_level_1)
    {
        isY_level_1 = index % 2;
        color_level_1_isgreen = (index >> 1) % 2;
        isY_level_0 = (index >> 2) % 2;
        color_level_0_isgreen = (index >> 3) % 2;
        profile_id = ((!splitEn_level_0) << 4) + (isY_level_0 << 3) + (color_level_0_isgreen << 2) + shareMode;
        step = (((!splitEn_level_1) << 2) + (color_level_1_isgreen << 1) + isY_level_1) * 5;
        profile_idx = color_level_0_isgreen * 2 + color_level_1_isgreen;
        p_share_profile = &ShareProfile_flow[shareMode][0];
    }
    else if (splitEn_level_0 && !splitEn_level_1)
    {
        color_level_0_isgreen = index % 2;
        isY_level_0 = (index >> 1) % 2;
        color_level_1 = (index >> 2) % 3;
        profile_id = ((!splitEn_level_0) << 4) + (isY_level_0 << 3) + (color_level_0_isgreen << 2) + shareMode;
        step = (((!splitEn_level_1) << 2) + color_level_1) * 5;
        profile_idx = color_level_0_isgreen * 3 + color_level_1;
        p_share_profile = &ShareProfile_flow[shareMode][0];
    }
    else
    {
        color_level_0 = index / 3;
        profile_id = ((!splitEn_level_0) << 4) + (color_level_0 << 2) + shareMode;
        color_level_1 = index % 3;
        step = (((!splitEn_level_1) << 2) + color_level_1) * 5;
        profile_idx = index;
        p_share_profile = &ShareProfile_trTcm[shareMode][0];
    }

    cmd = DRV_IOR(DsIpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &ipe_policer_share_profile));

    cmd = DRV_IOR(DsEpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &epe_policer_share_profile));

    if ((!splitEn_level_0) || (splitEn_level_0 && isY_level_0))
    {
        SetDsIpePolicingSharingProfile(V, g_0_dec0Y_f + step, &ipe_policer_share_profile, p_share_profile[profile_idx].dec0Y);
        SetDsEpePolicingSharingProfile(V, g_0_dec0Y_f + step, &epe_policer_share_profile, p_share_profile[profile_idx].dec0Y);
    }

    if ((!splitEn_level_0) || (splitEn_level_0 && (!isY_level_0)))
    {
        SetDsIpePolicingSharingProfile(V, g_0_dec0X_f + step, &ipe_policer_share_profile, p_share_profile[profile_idx].dec0X);
        SetDsEpePolicingSharingProfile(V, g_0_dec0X_f + step, &epe_policer_share_profile, p_share_profile[profile_idx].dec0X);
    }

    if ((!splitEn_level_1) || (splitEn_level_1 && isY_level_1))
    {
        SetDsIpePolicingSharingProfile(V, g_0_dec1Y_f + step, &ipe_policer_share_profile, p_share_profile[profile_idx].dec1Y);
        SetDsEpePolicingSharingProfile(V, g_0_dec1Y_f + step, &epe_policer_share_profile, p_share_profile[profile_idx].dec1Y);
    }

    if ((!splitEn_level_1) || (splitEn_level_1 && (!isY_level_1)))
    {
        SetDsIpePolicingSharingProfile(V, g_0_dec1X_f + step, &ipe_policer_share_profile, p_share_profile[profile_idx].dec1X);
        SetDsEpePolicingSharingProfile(V, g_0_dec1X_f + step, &epe_policer_share_profile, p_share_profile[profile_idx].dec1X);
    }

    SetDsIpePolicingSharingProfile(V, g_0_color_f + step, &ipe_policer_share_profile, p_share_profile[profile_idx].newColor);
    SetDsEpePolicingSharingProfile(V, g_0_color_f + step, &epe_policer_share_profile, p_share_profile[profile_idx].newColor);
    cmd = DRV_IOW(DsIpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &ipe_policer_share_profile));

    cmd = DRV_IOW(DsEpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &epe_policer_share_profile));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_init_share_mode(uint8 lchip)
{
    uint8  shareMode = 0;
    uint8  splitEn_level_0 = 0;
    uint8  splitEn_level_1 = 0;
    uint8  index = 0;
    uint8  profile_id = 0;
    uint32 cmd = 0;

    if(p_usw_qos_policer_master[lchip]->policer_merge_mode == 1)
    {
        sys_qos_policer_share_profile_t* p_share_profile = NULL;
        DsIpePolicingSharingProfile_m  ipe_policer_share_profile;
        DsEpePolicingSharingProfile_m  epe_policer_share_profile;
        static sys_qos_policer_share_profile_t ShareProfile[16][6] =
           {{{0,0,0,0,1},{1,0,0,0,3},{0,1,0,1,2},{1,0,0,1,3},{1,0,1,0,3},{1,0,1,0,3}},
            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,1,3},{1,0,1,0,3},{1,0,1,0,3}},
            {{0,0,0,0,1},{0,1,0,0,2},{0,1,0,1,2},{0,1,0,1,2},{0,1,1,0,2},{0,1,1,0,2}},
            {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1}},

            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3},{1,0,1,0,3},{1,0,1,0,3}},
            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3},{1,0,1,0,3},{1,0,1,0,3}},
            {{0,0,0,0,1},{0,1,0,0,2},{0,0,0,0,1},{0,1,0,0,2},{0,1,1,0,2},{0,1,1,0,2}},
            {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1}},

            {{0,0,0,0,1},{1,0,0,0,3},{0,1,0,1,2},{1,0,0,1,3},{0,1,0,1,2},{1,0,0,1,3}},
            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,1,3},{0,0,0,0,1},{1,0,0,1,3}},
            {{0,0,0,0,1},{0,1,0,0,2},{0,1,0,1,2},{0,1,0,1,2},{0,1,0,1,2},{0,1,0,1,2}},
            {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1}},

            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3}},
            {{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3},{0,0,0,0,1},{1,0,0,0,3}},
            {{0,0,0,0,1},{0,1,0,0,2},{0,0,0,0,1},{0,1,0,0,2},{0,0,0,0,1},{0,1,0,0,2}},
            {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1}}};

        sal_memset(&ipe_policer_share_profile, 0, sizeof(DsIpePolicingSharingProfile_m));
        sal_memset(&epe_policer_share_profile, 0, sizeof(DsEpePolicingSharingProfile_m));
        for(profile_id = 0; profile_id < 16; profile_id ++)
        {
            for(index = 0; index < 6; index ++)
            {
                p_share_profile = &ShareProfile[profile_id][index];
                cmd = DRV_IOR(DsIpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &ipe_policer_share_profile));

                cmd = DRV_IOR(DsEpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &epe_policer_share_profile));

                SetDsIpePolicingSharingProfile(V, g_0_dec0Y_f + index*5, &ipe_policer_share_profile, p_share_profile->dec0Y);
                SetDsEpePolicingSharingProfile(V, g_0_dec0Y_f + index*5, &epe_policer_share_profile, p_share_profile->dec0Y);

                SetDsIpePolicingSharingProfile(V, g_0_dec0X_f + index*5, &ipe_policer_share_profile, p_share_profile->dec0X);
                SetDsEpePolicingSharingProfile(V, g_0_dec0X_f + index*5, &epe_policer_share_profile, p_share_profile->dec0X);

                SetDsIpePolicingSharingProfile(V, g_0_dec1Y_f + index*5, &ipe_policer_share_profile, p_share_profile->dec1Y);
                SetDsEpePolicingSharingProfile(V, g_0_dec1Y_f + index*5, &epe_policer_share_profile, p_share_profile->dec1Y);

                SetDsIpePolicingSharingProfile(V, g_0_dec1X_f + index*5, &ipe_policer_share_profile, p_share_profile->dec1X);
                SetDsEpePolicingSharingProfile(V, g_0_dec1X_f + index*5, &epe_policer_share_profile, p_share_profile->dec1X);

                SetDsIpePolicingSharingProfile(V, g_0_color_f + index*5, &ipe_policer_share_profile, p_share_profile->newColor);
                SetDsEpePolicingSharingProfile(V, g_0_color_f + index*5, &epe_policer_share_profile, p_share_profile->newColor);

                cmd = DRV_IOW(DsIpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &ipe_policer_share_profile));

                cmd = DRV_IOW(DsEpePolicingSharingProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &epe_policer_share_profile));
            }
        }
        return CTC_E_NONE;
    }
    /*splitEn=0, shareMode 0: MIN_only, shareMode 1: MAX_only, shareMode 2: MIN_MAX*/
    for (shareMode = 0; shareMode < 3; shareMode++)
    {
        /*index=(color0,color1)*/
        for (index = 0; index < 3*3; index ++)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_set_policer_mode(lchip, shareMode, index,
                             splitEn_level_0, splitEn_level_1));
        }
    }

    /*splitEn=1, shareMode 0: SINGLE_OR, shareMode 1: SINGLE_AND, shareMode 2: DUAL MODE*/
    splitEn_level_0 = 1;
    splitEn_level_1 = 1;
    for (shareMode = 0; shareMode < 2; shareMode++)
    {
        /*index=(color0_isgreen,isY_0,color1_isgreen,isY_1)*/
        for (index = 0; index < 2*2*2*2; index++)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_set_policer_mode(lchip, shareMode, index,
                             splitEn_level_0, splitEn_level_1));
        }
    }

    shareMode = 2;
    splitEn_level_0 = 1;
    splitEn_level_1 = 0;

    /*index=(color1, isY_level_0, color0_isgreen)*/
    for (index = 0; index < 3*2*2; index ++)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_set_policer_mode(lchip, shareMode, index,
                                                                 splitEn_level_0, splitEn_level_1));
    }

    return CTC_E_NONE;
}

/**
 @brief get policer id based on type
*/
STATIC uint16
_sys_usw_qos_get_policer_id(uint8 type, uint16 plc_id, uint32 gport, uint16 vlan_id)
{
    uint16 policer_id = 0;
    switch(type)
    {
    case CTC_QOS_POLICER_TYPE_PORT:
        policer_id = (uint16)gport;
        break;
    case CTC_QOS_POLICER_TYPE_FLOW:
        policer_id = plc_id;
        break;
    case CTC_QOS_POLICER_TYPE_SERVICE:
        policer_id = plc_id;
        break;
    case CTC_QOS_POLICER_TYPE_VLAN:
        policer_id = vlan_id;
        break;
    case CTC_QOS_POLICER_TYPE_COPP:
        policer_id = plc_id;
        break;
    case CTC_QOS_POLICER_TYPE_MFP:
        policer_id = plc_id;
        break;
    default:
        break;
    }

    return policer_id;
}

/**
 @brief Get policer granularity according to the rate.
*/
STATIC int32
_sys_usw_qos_get_policer_gran_by_rate(uint8 lchip, uint32 rate, uint32* granularity)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(granularity);

#define M 1000

    if (rate <= 2*M)/*2M */
    {
        *granularity = 10;
    }
    else if (rate <= 100*M)/*100M */
    {
        *granularity = 40;
    }
    else if (rate <= 1000*M)  /*1G */
    {
        *granularity = 80;
    }
    else if (rate <= 2000*M)/*2G */
    {
        *granularity = 120;
    }
    else if (rate <= 4000*M)/*4G */
    {
        *granularity = 250;
    }
    else if (rate <= 10000*M)/*10G */
    {
        *granularity = 500;
    }
    else if (rate <= 40000*M)/*40G */
    {
        *granularity = 1000;
    }
    else if (rate <= 100000*M)/*100G */
    {
        *granularity = 2000;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_level_select(uint8 lchip, uint8 dir, uint8 sys_policer_type,
                                                   sys_qos_policer_t* p_sys_policer)
{
    uint8 policer_lvl = 0;
    ctc_qos_policer_level_select_t policer_level;

    sal_memset(&policer_level, 0, sizeof(ctc_qos_policer_level_select_t));
    CTC_ERROR_RETURN(sys_usw_qos_get_policer_level_select(lchip, &policer_level));
    if (p_sys_policer->level == CTC_QOS_POLICER_LEVEL_NONE)
    {
        if ((SYS_QOS_POLICER_TYPE_FLOW == sys_policer_type)
             || (p_sys_policer->hbwp_en == 1))
        {
            p_sys_policer->level = CTC_QOS_POLICER_LEVEL_1;
        }
        else if ((SYS_QOS_POLICER_TYPE_MACRO_FLOW == sys_policer_type) || (SYS_QOS_POLICER_TYPE_SERVICE == sys_policer_type)
                  || (CTC_QOS_POLICER_TYPE_COPP == p_sys_policer->type) ||(CTC_QOS_POLICER_TYPE_MFP == p_sys_policer->type))
        {
            p_sys_policer->level = CTC_QOS_POLICER_LEVEL_0;
        }
        else if (CTC_QOS_POLICER_TYPE_VLAN == p_sys_policer->type)
        {
            p_sys_policer->level = dir ? policer_level.egress_vlan_level : policer_level.ingress_vlan_level;
        }
        else if (CTC_QOS_POLICER_TYPE_PORT == p_sys_policer->type)
        {
            p_sys_policer->level = dir ? policer_level.egress_port_level : policer_level.ingress_port_level;
        }
    }

    /*select policer level, port policer use policer0, vlan policer use policer1
      other policer according to policer_lvl_sel, policer_lvl_sel==0
      is default mode, derive from GG*/
    if (dir == CTC_INGRESS
        && (p_sys_policer->level == CTC_QOS_POLICER_LEVEL_0))
    {
        policer_lvl = SYS_QOS_POLICER_IPE_POLICING_0;
    }
    else if (dir == CTC_INGRESS
            && (p_sys_policer->level == CTC_QOS_POLICER_LEVEL_1))
    {
        policer_lvl = SYS_QOS_POLICER_IPE_POLICING_1;
    }
    else if (dir == CTC_EGRESS
            && (p_sys_policer->level == CTC_QOS_POLICER_LEVEL_0))
    {
        policer_lvl = SYS_QOS_POLICER_EPE_POLICING_0;
    }
    else if (dir == CTC_EGRESS
            && (p_sys_policer->level == CTC_QOS_POLICER_LEVEL_1))
    {
        policer_lvl = SYS_QOS_POLICER_EPE_POLICING_1;
    }

    p_sys_policer->policer_lvl = policer_lvl;
    p_sys_policer->sys_policer_type = sys_policer_type;

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps,
                                             uint32 user_rate, /*kb/s*/
                                             uint32 *hw_rate,
                                             uint16 bit_with,
                                             uint32 gran,
                                             uint32 upd_freq)
{
    uint32 value = 0;
    uint8 rateFrac = 0;
    uint32 max_thrd = 0;

    CTC_ERROR_RETURN(sys_usw_qos_map_token_rate_user_to_hw(lchip, is_pps,
                                                                  user_rate,
                                                                  hw_rate,
                                                                  bit_with,
                                                                  gran,
                                                                  upd_freq,
                                                                  MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES)));

    value = (*hw_rate) >> 8;
    rateFrac = (*hw_rate) & 0xff;
    max_thrd = (1 << 16) - 1;

    CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, value,
                                                                  hw_rate,
                                                                  MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH),
                                                                  max_thrd));
    *hw_rate = *hw_rate << 8 | rateFrac;

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps, uint32 hw_rate,uint32 *user_rate, uint32 upd_freq)
{
    uint32 token_rate = 0;
    uint32 rate = 0;
    uint8 token_rate_frac = 0;

    token_rate_frac = hw_rate & 0xFF;
    _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_rate, (hw_rate >> 8), MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH));

    sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, (token_rate << 8 | token_rate_frac), &rate,
                                                 MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH), upd_freq, MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES));

    *user_rate = rate;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_alloc_statsptr(uint8 lchip, sys_qos_policer_t* p_sys_policer, uint32* p_stats_ptr)
{
    uint8  stats_num = 0;
    uint8  stats_type = 0;
    uint32 stats_offet = 0;
    uint32 cmd = 0;
    uint32 statsPtr_value = 0;
    uint32 statsMode_value = 0;
    uint8  dir = 0;
    uint8  level = 0;

    CTC_PTR_VALID_CHECK(p_stats_ptr);

    /*  stats_mode(sdk) = !statsMode(spec)
    1. !split_en
       1) statsMode == 1
          statsPtr = 0/1/2(green/yellow/red)
       2) statsMode == 0
          statsPtr = 0
    2. split_en
       1) isY
          (1) statsMode == 1
              statsPtr = 2/3(green/red)
          (2) statsMode == 0
              statsPtr = 1
       2) isX
          (1) statsMode == 1
              statsPtr = 0/1(green/red)
          (2) statsMode == 0
              statsPtr = 0
    */
    if (!(p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM))
    {
        if (p_sys_policer->stats_mode == 0)
        {
            stats_num = 3;
        }
        else
        {
            stats_num = 1;
        }
    }
    else if (p_sys_policer->stats_mode == 0)
    {
        stats_num = 4;
    }
    else
    {
        stats_num = 2;
    }

    if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            if(p_sys_policer->type != CTC_QOS_POLICER_TYPE_COPP)
            {
                cmd = DRV_IOR(DsIpePolicing0Config_t, DsIpePolicing0Config_statsPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsPtr_value));
                cmd = DRV_IOR(DsIpePolicing0Config_t, DsIpePolicing0Config_statsMode_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsMode_value));
            }
            else
            {
                cmd = DRV_IOR(DsIpeCoPPConfig_t, DsIpeCoPPConfig_statsPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsPtr_value));
                cmd = DRV_IOR(DsIpeCoPPConfig_t, DsIpeCoPPConfig_statsMode_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsMode_value));
            }
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            cmd = DRV_IOR(DsEpePolicing0Config_t, DsEpePolicing0Config_statsPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsPtr_value));
            cmd = DRV_IOR(DsEpePolicing0Config_t, DsEpePolicing0Config_statsMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsMode_value));
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            cmd = DRV_IOR(DsEpePolicing1Config_t, DsEpePolicing1Config_statsPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsPtr_value));
            cmd = DRV_IOR(DsEpePolicing1Config_t, DsEpePolicing1Config_statsMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &statsMode_value));
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    dir   = (p_sys_policer->policer_lvl) / 2;
    level = (p_sys_policer->policer_lvl) % 2;

    if (statsPtr_value == 0)
    {
        stats_type = ((level==0) ? SYS_STATS_TYPE_POLICER0: SYS_STATS_TYPE_POLICER1);
        CTC_ERROR_RETURN(sys_usw_flow_stats_alloc_statsptr(lchip, dir, stats_num, stats_type, &stats_offet));
    }
    /* policer_merge_mode  stats ptr 11 12 bit can not be used*/
    if (p_usw_qos_policer_master[lchip]->policer_merge_mode == 1 && ((stats_offet & 0x1800) != 0))
    {
        stats_type = ((level==0) ? SYS_STATS_TYPE_POLICER0: SYS_STATS_TYPE_POLICER1);
        CTC_ERROR_RETURN(sys_usw_flow_stats_free_statsptr(lchip, dir, stats_num, stats_type, &stats_offet));
        return CTC_E_NO_RESOURCE;
    }

    *p_stats_ptr = (stats_offet == 0)? statsPtr_value : stats_offet;
    p_sys_policer->stats_num = stats_num;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_free_statsptr(uint8 lchip, sys_qos_policer_t* p_sys_policer, uint32* p_stats_ptr)
{
    uint8 stats_num = 0;
    uint8 stats_type = 0;
    uint8 dir = 0;
    uint8 level = 0;
    uint32 cmd = 0;
    uint32 value = 0;

    stats_num = p_sys_policer->stats_num;
    dir   = (p_sys_policer->policer_lvl) / 2;
    level = (p_sys_policer->policer_lvl) % 2;
    stats_type = ((level==0) ? SYS_STATS_TYPE_POLICER0: SYS_STATS_TYPE_POLICER1);
    if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM &&
        CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            if(p_sys_policer->type != CTC_QOS_POLICER_TYPE_COPP)
            {
                cmd = DRV_IOR(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            }
            else
            {
                cmd = DRV_IOR(DsIpeCoPPConfig_t, DsIpeCoPPConfig_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            }
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            cmd = DRV_IOR(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdX_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            break;
        case SYS_QOS_POLICER_EPE_POLICING_1:
            cmd = DRV_IOR(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdX_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else if(p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM &&
        !CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            if(p_sys_policer->type != CTC_QOS_POLICER_TYPE_COPP)
            {
                cmd = DRV_IOR(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            }
            else
            {
                cmd = DRV_IOR(DsIpeCoPPConfig_t, DsIpeCoPPConfig_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            }
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            cmd = DRV_IOR(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdY_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            break;
        case SYS_QOS_POLICER_EPE_POLICING_1:
            cmd = DRV_IOR(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdY_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_policer->policer_ptr) >> 1, cmd, &value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    if(value == 0)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_free_statsptr(lchip, dir, stats_num, stats_type,p_stats_ptr));
        *p_stats_ptr = 0;
    }

    return CTC_E_NONE;
}

/**
 @brief Get prof according to the given policer data.
*/
STATIC int32
_sys_usw_qos_policer_profile_build_data(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                         sys_qos_policer_profile_t* p_sys_profile,
                                                         uint8 is_bucketY)
{
    uint32 value                 = 0;
    uint32 commit_rate           = 0;
    uint32 commit_rate_max       = 0;
    uint32 max_rate              = 0;
    uint32 gran_cir              = 0;
    uint32 gran_pir              = 0;
    uint32 max_thrd = 0;
    uint32 token_thrd = 0;
    uint8  is_pps = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);
    CTC_PTR_VALID_CHECK(p_sys_profile);

    p_sys_profile->policer_lvl = p_sys_policer->policer_lvl;
    is_pps = p_sys_policer->is_pps;

    if (!is_bucketY)
    {
        p_sys_profile->is_bucketY = 0;
        max_rate = p_sys_policer->cir;
        if (p_sys_policer->hbwp_en == 1)
        {
            max_rate = MAX(p_sys_policer->cir_max, max_rate);
        }

        CTC_ERROR_RETURN(_sys_usw_qos_get_policer_gran_by_rate(lchip, max_rate, &gran_cir));
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cir granularity : %d\n", gran_cir);

        /* compute commit rate(CIR) */
        CTC_ERROR_RETURN(_sys_usw_qos_policer_map_token_rate_user_to_hw(lchip, is_pps,
                                                                       p_sys_policer->cir,
                                                                       &commit_rate,
                                                                       MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH),
                                                                       gran_cir,
                                                                       p_usw_qos_policer_master[lchip]->policer_update_freq));

        p_sys_profile->rate = commit_rate >> 12;
        p_sys_profile->rateShift = (commit_rate >> 8) & 0xf;
        p_sys_profile->rateFrac = commit_rate & 0xff;

        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profX->rate : %d\n",  p_sys_profile->rate);
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profX->rateShift : %d\n",  p_sys_profile->rateShift);
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profX->rateFrac : %d\n",  p_sys_profile->rateFrac);

        /* compute CBS threshold and threshold shift */
        if (is_pps)
        {
            value = p_sys_policer->cbs * MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
        }
        else
        {
            value = p_sys_policer->cbs * 125;   /* cbs*1000/8--> B/S*/
        }

        max_thrd = (1 << 12) - 1;

        CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, value,
                                                                      &token_thrd,
                                                                      MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH),
                                                                      max_thrd));
        p_sys_profile->threshold = token_thrd >> 4;
        p_sys_profile->thresholdShift = token_thrd & 0xF;

        if ((p_sys_profile->rate << p_sys_profile->rateShift) > (p_sys_profile->threshold << p_sys_profile->thresholdShift))
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CBS is too samll,token will overflow!!!!\n");
        }

        if (p_sys_policer->hbwp_en == 1)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_map_token_rate_user_to_hw(lchip, is_pps,
                                                                       p_sys_policer->cir_max,
                                                                       &commit_rate_max,
                                                                       MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH),
                                                                       gran_cir,
                                                                       p_usw_qos_policer_master[lchip]->policer_update_freq));
            p_sys_profile->rateMax = commit_rate_max >> 12;
            p_sys_profile->rateMaxShift = (commit_rate_max >> 8) & 0xf;
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cir_max: %d\n",  p_sys_profile->rateMax);
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cir_max_shift: %d\n",  p_sys_profile->rateMaxShift);
        }
    }
    else
    {
        p_sys_profile->is_bucketY = 1;
        max_rate = p_sys_policer->pir;

        if (p_sys_policer->hbwp_en == 1)
        {
            max_rate = MAX(p_sys_policer->pir_max, max_rate);
        }

        CTC_ERROR_RETURN(_sys_usw_qos_get_policer_gran_by_rate(lchip, max_rate, &gran_pir));

        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "pir granularity : %d\n", gran_pir);

        /* compute peak rate(PIR or EIR)*/
        CTC_ERROR_RETURN(_sys_usw_qos_policer_map_token_rate_user_to_hw(lchip, is_pps,
                                                                       p_sys_policer->pir,
                                                                       &commit_rate,
                                                                       MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH),
                                                                       gran_pir,
                                                                       p_usw_qos_policer_master[lchip]->policer_update_freq));

        p_sys_profile->rate = commit_rate >> 12;
        p_sys_profile->rateShift = (commit_rate >> 8) & 0xf;
        p_sys_profile->rateFrac = commit_rate & 0xff;

        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profY->rate : %d\n",  p_sys_profile->rate);
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profY->rateShift : %d\n",  p_sys_profile->rateShift);
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_ds_profY->rateFrac : %d\n",  p_sys_profile->rateFrac);

        /* compute EBS threshold and threshold shift */
        if (is_pps)
        {
            value = p_sys_policer->pbs * MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
        }
        else
        {
            value = p_sys_policer->pbs * 125;   /* pbs*1000/8--> B/S*/
        }
        max_thrd = (1 << 12) - 1;


        CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, value,
                                                                      &token_thrd,
                                                                      MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH),
                                                                      max_thrd));
        p_sys_profile->threshold = token_thrd >> 4;
        p_sys_profile->thresholdShift = token_thrd & 0xF;

        if ((p_sys_profile->rate << p_sys_profile->rateShift) > (p_sys_profile->threshold << p_sys_profile->thresholdShift))
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "EBS is too samll,token will overflow!!!!\n");
        }
        if (p_sys_policer->hbwp_en == 1)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_map_token_rate_user_to_hw(lchip, is_pps,
                                                                       p_sys_policer->pir_max,
                                                                       &commit_rate_max,
                                                                       MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH),
                                                                       gran_pir,
                                                                       p_usw_qos_policer_master[lchip]->policer_update_freq));

            p_sys_profile->rateMax = commit_rate_max >> 12;
            p_sys_profile->rateMaxShift = (commit_rate_max >> 8) & 0xf;
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "pir_max: %d\n",  p_sys_profile->rateMax);
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "pir_max_shift: %d\n",  p_sys_profile->rateMaxShift);
        }
        else if ((p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_RFC2697)
            || (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_RFC4115)
            || (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_MEF_BWP))
        {
            p_sys_profile->rateMax = 0xFFFF;
            p_sys_profile->rateMaxShift = 0xF;

            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "pir_max: %d\n",  0xFFFF);
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "pir_max_shift: %d\n",  0xF);
        }
    }

    return CTC_E_NONE;
}

/**
 @brief Write policer to asic.
*/
STATIC int32
_sys_usw_qos_policer_add_to_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                   sys_qos_policer_profile_t* p_sys_profX,
                                                   sys_qos_policer_profile_t* p_sys_profY,
                                                   sys_qos_policer_action_t*  p_sys_action_profile)
{
    uint32 policer_ptr = 0;
    uint32 cmd         = 0;
    uint8 color_blind_mode  = 0;
     /*uint8 color_drop  = 0;*/
    uint8 use_len  = 0;
    uint8 cf  = 0;
    uint8 cos_index = 0;
    uint8 temp_cos_index = 0;
    uint32 cir_cnt = 0;
    uint32 pir_cnt = 0;
    uint8 policer_mode = 0;
    uint8 splitEn =0;
    uint8 is_pps = 0;
    uint8 statsMode = 0;
    uint32 table_index = 0;
    uint32 value = 0;
    uint8 share_mode = 0;
    uint16 stats_ptr = 0;

    DsIpePolicing0Config_m ipe_policer0_cfg;
    DsIpePolicing1Config_m ipe_policer1_cfg;
    DsEpePolicing0Config_m epe_policer0_cfg;
    DsEpePolicing1Config_m epe_policer1_cfg;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&ipe_policer0_cfg,0,sizeof(DsIpePolicing0Config_m));
    sal_memset(&ipe_policer1_cfg,0,sizeof(DsIpePolicing1Config_m));
    sal_memset(&epe_policer0_cfg,0,sizeof(DsEpePolicing0Config_m));
    sal_memset(&epe_policer1_cfg,0,sizeof(DsEpePolicing1Config_m));

    policer_ptr = p_sys_policer->policer_ptr;
    cos_index = p_sys_policer->cos_index;
    temp_cos_index = p_sys_policer->cos_index;
    is_pps = p_sys_policer->is_pps;

    if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
    {
        temp_cos_index = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) ? cos_index : (cos_index << 1);
    }

    if (p_sys_policer->cos_bitmap)
    {
        policer_ptr += temp_cos_index;
    }

    if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
    {
        table_index = policer_ptr >> 1;
    }
    else
    {
        table_index = policer_ptr;
    }

    color_blind_mode = p_sys_policer->is_color_aware ? 0 : 1;
    use_len    = p_sys_policer->use_l3_length ? 1 : 0;
    policer_mode = p_sys_policer->policer_mode;
    splitEn = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) ? 1 : 0;
    statsMode = !(p_sys_policer->stats_mode);
    if (p_sys_policer->drop_color == CTC_QOS_COLOR_RED)
    {
         /*color_drop = 2;*/
    }
    else if (p_sys_policer->drop_color == CTC_QOS_COLOR_YELLOW)
    {
         /*color_drop = 3;*/
    }

    if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_MEF_BWP ||
        p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_RFC4115)
    {
        cf = p_sys_policer->cf_en ? 1 : 0;
        policer_mode = CTC_QOS_POLICER_MODE_RFC4115;
    }
    else if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_RFC2697)
    {
        cf = 1;
    }

    if (p_sys_profX)
    {
        cir_cnt = (p_sys_profX->threshold << p_sys_profX->thresholdShift);
    }
    if (p_sys_profY)
    {
        pir_cnt = (p_sys_profY->threshold << p_sys_profY->thresholdShift);
    }

    if (cir_cnt > 0x1FFFFFF)
    {
        cir_cnt = SYS_QOS_POLICER_MAX_COUNT;
    }
    if (pir_cnt > 0x1FFFFFF)
    {
        pir_cnt = SYS_QOS_POLICER_MAX_COUNT;
    }
    if(p_sys_policer->color_merge_mode == CTC_QOS_POLICER_COLOR_MERGE_AND)
    {
        share_mode = 1;
    }
    else if(p_sys_policer->color_merge_mode == CTC_QOS_POLICER_COLOR_MERGE_OR)
    {
        share_mode = 0;
    }
    else
    {
        share_mode = 2;
    }

    stats_ptr = p_usw_qos_policer_master[lchip]->policer_merge_mode == 0?
                  (p_sys_policer->stats_ptr[cos_index]&0x7FFF) : (p_sys_policer->stats_ptr[cos_index]&0x7FFF) |0x800;

    switch(p_sys_policer->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
         cmd = DRV_IOR(DsIpePolicing0Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ipe_policer0_cfg));
         SetDsIpePolicing0Config(V, actionProfId_f, &ipe_policer0_cfg, policer_mode-1);
         SetDsIpePolicing0Config(V, colorBlindMode_f, &ipe_policer0_cfg, color_blind_mode);
         SetDsIpePolicing0Config(V, couplingFlag_f, &ipe_policer0_cfg, cf);
         SetDsIpePolicing0Config(V, layer3LengthEn_f, &ipe_policer0_cfg, use_len);
         SetDsIpePolicing0Config(V, statsMode_f, &ipe_policer0_cfg, statsMode);
         SetDsIpePolicing0Config(V, policyProfId_f, &ipe_policer0_cfg, p_sys_action_profile->profile_id);
         SetDsIpePolicing0Config(V, statsPtr_f, &ipe_policer0_cfg, stats_ptr);

         if (p_sys_profX)
         {
             SetDsIpePolicing0Config(V, profIdX_f, &ipe_policer0_cfg, p_sys_profX->profile_id);
             SetDsIpePolicing0Config(V, ppsModeX_f, &ipe_policer0_cfg, is_pps);
             cmd = DRV_IOW(DsIpePolicing0CountX_t, DsIpePolicing0CountX_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &cir_cnt));
             cmd = DRV_IOW(DsIpePolicing0CountX_t, DsIpePolicing0CountX_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         if (p_sys_profY)
         {
             SetDsIpePolicing0Config(V, profIdY_f, &ipe_policer0_cfg, p_sys_profY->profile_id);
             SetDsIpePolicing0Config(V, ppsModeY_f, &ipe_policer0_cfg, is_pps);
             cmd = DRV_IOW(DsIpePolicing0CountY_t, DsIpePolicing0CountY_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &pir_cnt));
             cmd = DRV_IOW(DsIpePolicing0CountY_t, DsIpePolicing0CountY_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         SetDsIpePolicing0Config(V, shareMode_f, &ipe_policer0_cfg, share_mode);
         SetDsIpePolicing0Config(V, splitEn_f, &ipe_policer0_cfg, splitEn);
         if (p_sys_policer->hbwp_en)
         {
             SetDsIpePolicing0Config(V, envelopeEn_f, &ipe_policer0_cfg, p_sys_policer->sf_en);
             SetDsIpePolicing0Config(V, couplingFlagTotal_f, &ipe_policer0_cfg, !p_sys_policer->cf_total_dis);
         }
         cmd = DRV_IOW(DsIpePolicing0Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ipe_policer0_cfg));

         break;

    case SYS_QOS_POLICER_IPE_POLICING_1:
         cmd = DRV_IOR(DsIpePolicing1Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ipe_policer1_cfg));
         SetDsIpePolicing1Config(V, actionProfId_f, &ipe_policer1_cfg, policer_mode-1);
         SetDsIpePolicing1Config(V, colorBlindMode_f, &ipe_policer1_cfg, color_blind_mode);
         SetDsIpePolicing1Config(V, couplingFlag_f, &ipe_policer1_cfg, cf);
         SetDsIpePolicing1Config(V, layer3LengthEn_f, &ipe_policer1_cfg, use_len);
         SetDsIpePolicing1Config(V, statsMode_f, &ipe_policer1_cfg, statsMode);
         SetDsIpePolicing1Config(V, policyProfId_f, &ipe_policer1_cfg, p_sys_action_profile->profile_id);
         SetDsIpePolicing1Config(V, statsPtr_f, &ipe_policer1_cfg, stats_ptr);

         if (p_sys_profX)
         {
             SetDsIpePolicing1Config(V, profIdX_f, &ipe_policer1_cfg, p_sys_profX->profile_id);
             SetDsIpePolicing1Config(V, ppsModeX_f, &ipe_policer1_cfg, is_pps);
             cmd = DRV_IOW(DsIpePolicing1CountX_t, DsIpePolicing1CountX_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &cir_cnt));
             cmd = DRV_IOW(DsIpePolicing1CountX_t, DsIpePolicing1CountX_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         if (p_sys_profY)
         {
             SetDsIpePolicing1Config(V, profIdY_f, &ipe_policer1_cfg, p_sys_profY->profile_id);
             SetDsIpePolicing1Config(V, ppsModeY_f, &ipe_policer1_cfg, is_pps);
             cmd = DRV_IOW(DsIpePolicing1CountY_t, DsIpePolicing1CountY_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &pir_cnt));
             cmd = DRV_IOW(DsIpePolicing1CountY_t, DsIpePolicing1CountY_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         SetDsIpePolicing1Config(V, splitEn_f, &ipe_policer1_cfg, splitEn);
         if (p_sys_policer->hbwp_en)
         {
             SetDsIpePolicing1Config(V, envelopeEn_f, &ipe_policer1_cfg, p_sys_policer->sf_en);
             SetDsIpePolicing1Config(V, couplingFlagTotal_f, &ipe_policer1_cfg, !p_sys_policer->cf_total_dis);
         }
         cmd = DRV_IOW(DsIpePolicing1Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ipe_policer1_cfg));

         break;

    case SYS_QOS_POLICER_EPE_POLICING_0:
         cmd = DRV_IOR(DsEpePolicing0Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &epe_policer0_cfg));
         SetDsEpePolicing0Config(V, actionProfId_f, &epe_policer0_cfg, policer_mode - 1);
         SetDsEpePolicing0Config(V, colorBlindMode_f, &epe_policer0_cfg, color_blind_mode);
         SetDsEpePolicing0Config(V, couplingFlag_f, &epe_policer0_cfg, cf);
         SetDsEpePolicing0Config(V, layer3LengthEn_f, &epe_policer0_cfg, use_len);
         SetDsEpePolicing0Config(V, statsMode_f, &epe_policer0_cfg, statsMode);
         SetDsEpePolicing0Config(V, policyProfId_f, &epe_policer0_cfg, p_sys_action_profile->profile_id);
         SetDsEpePolicing0Config(V, statsPtr_f, &epe_policer0_cfg, stats_ptr);

         if (p_sys_profX)
         {
             SetDsEpePolicing0Config(V, profIdX_f, &epe_policer0_cfg, p_sys_profX->profile_id);
             SetDsEpePolicing0Config(V, ppsModeX_f, &epe_policer0_cfg, is_pps);
             cmd = DRV_IOW(DsEpePolicing0CountX_t, DsEpePolicing0CountX_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &cir_cnt));
             cmd = DRV_IOW(DsEpePolicing0CountX_t, DsEpePolicing0CountX_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         if (p_sys_profY)
         {
             SetDsEpePolicing0Config(V, profIdY_f, &epe_policer0_cfg, p_sys_profY->profile_id);
             SetDsEpePolicing0Config(V, ppsModeY_f, &epe_policer0_cfg, is_pps);
             cmd = DRV_IOW(DsEpePolicing0CountY_t, DsEpePolicing0CountY_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &pir_cnt));
             cmd = DRV_IOW(DsEpePolicing0CountY_t, DsEpePolicing0CountY_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         SetDsEpePolicing0Config(V, shareMode_f, &epe_policer0_cfg, share_mode);
         SetDsEpePolicing0Config(V, splitEn_f, &epe_policer0_cfg, splitEn);
         if (p_sys_policer->hbwp_en)
         {
             SetDsEpePolicing0Config(V, envelopeEn_f, &epe_policer0_cfg, p_sys_policer->sf_en);
             SetDsEpePolicing0Config(V, couplingFlagTotal_f, &epe_policer0_cfg, !p_sys_policer->cf_total_dis);
         }
         cmd = DRV_IOW(DsEpePolicing0Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &epe_policer0_cfg));

         break;

    case SYS_QOS_POLICER_EPE_POLICING_1:
         cmd = DRV_IOR(DsEpePolicing1Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &epe_policer1_cfg));
         SetDsEpePolicing1Config(V, actionProfId_f, &epe_policer1_cfg, policer_mode - 1);
         SetDsEpePolicing1Config(V, colorBlindMode_f, &epe_policer1_cfg, color_blind_mode);
         SetDsEpePolicing1Config(V, couplingFlag_f, &epe_policer1_cfg, cf);
         SetDsEpePolicing1Config(V, layer3LengthEn_f, &epe_policer1_cfg, use_len);
         SetDsEpePolicing1Config(V, statsMode_f, &epe_policer1_cfg, statsMode);
         SetDsEpePolicing1Config(V, policyProfId_f, &epe_policer1_cfg, p_sys_action_profile->profile_id);
         SetDsEpePolicing1Config(V, statsPtr_f, &epe_policer1_cfg, stats_ptr);

         if (p_sys_profX)
         {
             SetDsEpePolicing1Config(V, profIdX_f, &epe_policer1_cfg, p_sys_profX->profile_id);
             SetDsEpePolicing1Config(V, ppsModeX_f, &epe_policer1_cfg, is_pps);
             cmd = DRV_IOW(DsEpePolicing1CountX_t, DsEpePolicing1CountX_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &cir_cnt));
             cmd = DRV_IOW(DsEpePolicing1CountX_t, DsEpePolicing1CountX_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         if (p_sys_profY)
         {
             SetDsEpePolicing1Config(V, profIdY_f, &epe_policer1_cfg, p_sys_profY->profile_id);
             SetDsEpePolicing1Config(V, ppsModeY_f, &epe_policer1_cfg, is_pps);
             cmd = DRV_IOW(DsEpePolicing1CountY_t, DsEpePolicing1CountY_count_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &pir_cnt));
             cmd = DRV_IOW(DsEpePolicing1CountY_t, DsEpePolicing1CountY_sign_f);
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
         }
         SetDsEpePolicing1Config(V, splitEn_f, &epe_policer1_cfg, splitEn);
         if (p_sys_policer->hbwp_en)
         {
             SetDsEpePolicing1Config(V, envelopeEn_f, &epe_policer1_cfg, p_sys_policer->sf_en);
             SetDsEpePolicing1Config(V, couplingFlagTotal_f, &epe_policer1_cfg, !p_sys_policer->cf_total_dis);
         }
         cmd = DRV_IOW(DsEpePolicing1Config_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &epe_policer1_cfg));

         break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add to asic Policer config,  index = %d\n",
                             table_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_remove_from_asic(uint8 lchip, ctc_qos_policer_t* p_policer_param,
                                            sys_qos_policer_t* p_sys_policer)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_policer_param);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    /*For normal policer, only remover SW DB,
    For HBWP, need reset the profie to 0 for no rate share*/
    if (p_policer_param->hbwp_en || p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        uint32 policer_ptr = 0;
        uint32 cmd = 0;
        uint32 field_val = 0;
        uint8 cos_index = 0;
        uint32 table_index = 0;

        policer_ptr = p_sys_policer->policer_ptr;
        cos_index = p_policer_param->hbwp.cos_index;
        if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
        {
            cos_index = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) ? cos_index : (cos_index << 1);
        }
        policer_ptr += cos_index;
        if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
        {
            table_index = policer_ptr >> 1;
        }
        else
        {
            table_index = policer_ptr;
        }
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            if((p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP)&&
                CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))
            {
                cmd = DRV_IOW(DsIpeCoPPConfig_t, DsIpeCoPPConfig_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else if((p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP)&&
                !CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))
            {
                cmd = DRV_IOW(DsIpeCoPPConfig_t, DsIpeCoPPConfig_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else if(CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else if(!CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else
            {
                cmd = DRV_IOW(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
                cmd = DRV_IOW(DsIpePolicing0Config_t, DsIpePolicing0Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            break;

        case SYS_QOS_POLICER_IPE_POLICING_1:
            cmd = DRV_IOW(DsIpePolicing1Config_t, DsIpePolicing1Config_profIdX_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            cmd = DRV_IOW(DsIpePolicing1Config_t, DsIpePolicing1Config_profIdY_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            if(CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else if(!CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else
            {
                cmd = DRV_IOW(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
                cmd = DRV_IOW(DsEpePolicing0Config_t, DsEpePolicing0Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            if(CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else if(!CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)&&
                p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
            {
                cmd = DRV_IOW(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            else
            {
                cmd = DRV_IOW(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdX_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
                cmd = DRV_IOW(DsEpePolicing1Config_t, DsEpePolicing1Config_profIdY_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
 }

/**
 @brief Write policer to asic.
*/
STATIC int32
_sys_usw_qos_policer_profile_add_to_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_profile_t* p_sys_profile,
                                                            uint8 is_bucketY)
{
    uint32 cmd = 0;
    DsIpePolicing0ProfileX_m ipe_policer0_profileX;
    DsIpePolicing0ProfileY_m ipe_policer0_profileY;
    DsIpePolicing1ProfileX_m ipe_policer1_profileX;
    DsIpePolicing1ProfileY_m ipe_policer1_profileY;
    DsEpePolicing0ProfileX_m epe_policer0_profileX;
    DsEpePolicing0ProfileY_m epe_policer0_profileY;
    DsEpePolicing1ProfileX_m epe_policer1_profileX;
    DsEpePolicing1ProfileY_m epe_policer1_profileY;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_profile);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    if (!is_bucketY)
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            sal_memset(&ipe_policer0_profileX, 0, sizeof(DsIpePolicing0ProfileX_m));
            SetDsIpePolicing0ProfileX(V, rateFrac_f, &ipe_policer0_profileX, p_sys_profile->rateFrac);
            SetDsIpePolicing0ProfileX(V, rateShift_f, &ipe_policer0_profileX, p_sys_profile->rateShift);
            SetDsIpePolicing0ProfileX(V, rate_f, &ipe_policer0_profileX, p_sys_profile->rate);
            SetDsIpePolicing0ProfileX(V, thresholdShift_f, &ipe_policer0_profileX, p_sys_profile->thresholdShift);
            SetDsIpePolicing0ProfileX(V, threshold_f, &ipe_policer0_profileX, p_sys_profile->threshold);
            SetDsIpePolicing0ProfileX(V, rateMaxShift_f, &ipe_policer0_profileX, p_sys_profile->rateMaxShift);
            SetDsIpePolicing0ProfileX(V, rateMax_f, &ipe_policer0_profileX, p_sys_profile->rateMax);
            SetDsIpePolicing0ProfileX(V, ppsShift_f, &ipe_policer0_profileX, 0);

            cmd = DRV_IOW(DsIpePolicing0ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer0_profileX));
            break;

        case SYS_QOS_POLICER_IPE_POLICING_1:
            sal_memset(&ipe_policer1_profileX, 0, sizeof(DsIpePolicing1ProfileX_m));
            SetDsIpePolicing1ProfileX(V, rateFrac_f, &ipe_policer1_profileX, p_sys_profile->rateFrac);
            SetDsIpePolicing1ProfileX(V, rateShift_f, &ipe_policer1_profileX, p_sys_profile->rateShift);
            SetDsIpePolicing1ProfileX(V, rate_f, &ipe_policer1_profileX, p_sys_profile->rate);
            SetDsIpePolicing1ProfileX(V, thresholdShift_f, &ipe_policer1_profileX, p_sys_profile->thresholdShift);
            SetDsIpePolicing1ProfileX(V, threshold_f, &ipe_policer1_profileX, p_sys_profile->threshold);
            SetDsIpePolicing1ProfileX(V, rateMaxShift_f, &ipe_policer1_profileX, p_sys_profile->rateMaxShift);
            SetDsIpePolicing1ProfileX(V, rateMax_f, &ipe_policer1_profileX, p_sys_profile->rateMax);
            SetDsIpePolicing1ProfileX(V, ppsShift_f, &ipe_policer1_profileX, 0);

            cmd = DRV_IOW(DsIpePolicing1ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer1_profileX));

            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            sal_memset(&epe_policer0_profileX, 0, sizeof(DsEpePolicing0ProfileX_m));
            SetDsEpePolicing0ProfileX(V, rateFrac_f, &epe_policer0_profileX, p_sys_profile->rateFrac);
            SetDsEpePolicing0ProfileX(V, rateShift_f, &epe_policer0_profileX, p_sys_profile->rateShift);
            SetDsEpePolicing0ProfileX(V, rate_f, &epe_policer0_profileX, p_sys_profile->rate);
            SetDsEpePolicing0ProfileX(V, thresholdShift_f, &epe_policer0_profileX, p_sys_profile->thresholdShift);
            SetDsEpePolicing0ProfileX(V, threshold_f, &epe_policer0_profileX, p_sys_profile->threshold);
            SetDsEpePolicing0ProfileX(V, rateMaxShift_f, &epe_policer0_profileX, p_sys_profile->rateMaxShift);
            SetDsEpePolicing0ProfileX(V, rateMax_f, &epe_policer0_profileX, p_sys_profile->rateMax);
            SetDsEpePolicing0ProfileX(V, ppsShift_f, &epe_policer0_profileX, 0);

            cmd = DRV_IOW(DsEpePolicing0ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer0_profileX));
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            sal_memset(&epe_policer1_profileX, 0, sizeof(DsEpePolicing1ProfileX_m));
            SetDsEpePolicing1ProfileX(V, rateFrac_f, &epe_policer1_profileX, p_sys_profile->rateFrac);
            SetDsEpePolicing1ProfileX(V, rateShift_f, &epe_policer1_profileX, p_sys_profile->rateShift);
            SetDsEpePolicing1ProfileX(V, rate_f, &epe_policer1_profileX, p_sys_profile->rate);
            SetDsEpePolicing1ProfileX(V, thresholdShift_f, &epe_policer1_profileX, p_sys_profile->thresholdShift);
            SetDsEpePolicing1ProfileX(V, threshold_f, &epe_policer1_profileX, p_sys_profile->threshold);
            SetDsEpePolicing1ProfileX(V, rateMaxShift_f, &epe_policer1_profileX, p_sys_profile->rateMaxShift);
            SetDsEpePolicing1ProfileX(V, rateMax_f, &epe_policer1_profileX, p_sys_profile->rateMax);
            SetDsEpePolicing1ProfileX(V, ppsShift_f, &epe_policer1_profileX, 0);

            cmd = DRV_IOW(DsEpePolicing1ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer1_profileX));
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            sal_memset(&ipe_policer0_profileY, 0, sizeof(DsIpePolicing0ProfileY_m));
            SetDsIpePolicing0ProfileY(V, rateFrac_f, &ipe_policer0_profileY, p_sys_profile->rateFrac);
            SetDsIpePolicing0ProfileY(V, rateShift_f, &ipe_policer0_profileY, p_sys_profile->rateShift);
            SetDsIpePolicing0ProfileY(V, rate_f, &ipe_policer0_profileY, p_sys_profile->rate);
            SetDsIpePolicing0ProfileY(V, thresholdShift_f, &ipe_policer0_profileY, p_sys_profile->thresholdShift);
            SetDsIpePolicing0ProfileY(V, threshold_f, &ipe_policer0_profileY, p_sys_profile->threshold);
            SetDsIpePolicing0ProfileY(V, rateMaxShift_f, &ipe_policer0_profileY, p_sys_profile->rateMaxShift);
            SetDsIpePolicing0ProfileY(V, rateMax_f, &ipe_policer0_profileY, p_sys_profile->rateMax);
            SetDsIpePolicing0ProfileY(V, ppsShift_f, &ipe_policer0_profileY, 0);

            cmd = DRV_IOW(DsIpePolicing0ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer0_profileY));
            break;

        case SYS_QOS_POLICER_IPE_POLICING_1:
            sal_memset(&ipe_policer1_profileY, 0, sizeof(DsIpePolicing1ProfileY_m));
            SetDsIpePolicing1ProfileY(V, rateFrac_f, &ipe_policer1_profileY, p_sys_profile->rateFrac);
            SetDsIpePolicing1ProfileY(V, rateShift_f, &ipe_policer1_profileY, p_sys_profile->rateShift);
            SetDsIpePolicing1ProfileY(V, rate_f, &ipe_policer1_profileY, p_sys_profile->rate);
            SetDsIpePolicing1ProfileY(V, thresholdShift_f, &ipe_policer1_profileY, p_sys_profile->thresholdShift);
            SetDsIpePolicing1ProfileY(V, threshold_f, &ipe_policer1_profileY, p_sys_profile->threshold);
            SetDsIpePolicing1ProfileY(V, rateMaxShift_f, &ipe_policer1_profileY, p_sys_profile->rateMaxShift);
            SetDsIpePolicing1ProfileY(V, rateMax_f, &ipe_policer1_profileY, p_sys_profile->rateMax);
            SetDsIpePolicing1ProfileY(V, ppsShift_f, &ipe_policer1_profileY, 0);

            cmd = DRV_IOW(DsIpePolicing1ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer1_profileY));
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            sal_memset(&epe_policer0_profileY, 0, sizeof(DsEpePolicing0ProfileY_m));
            SetDsEpePolicing0ProfileY(V, rateFrac_f, &epe_policer0_profileY, p_sys_profile->rateFrac);
            SetDsEpePolicing0ProfileY(V, rateShift_f, &epe_policer0_profileY, p_sys_profile->rateShift);
            SetDsEpePolicing0ProfileY(V, rate_f, &epe_policer0_profileY, p_sys_profile->rate);
            SetDsEpePolicing0ProfileY(V, thresholdShift_f, &epe_policer0_profileY, p_sys_profile->thresholdShift);
            SetDsEpePolicing0ProfileY(V, threshold_f, &epe_policer0_profileY, p_sys_profile->threshold);
            SetDsEpePolicing0ProfileY(V, rateMaxShift_f, &epe_policer0_profileY, p_sys_profile->rateMaxShift);
            SetDsEpePolicing0ProfileY(V, rateMax_f, &epe_policer0_profileY, p_sys_profile->rateMax);
            SetDsEpePolicing0ProfileY(V, ppsShift_f, &epe_policer0_profileY, 0);

            cmd = DRV_IOW(DsEpePolicing0ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer0_profileY));
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            sal_memset(&epe_policer1_profileY, 0, sizeof(DsEpePolicing1ProfileY_m));
            SetDsEpePolicing1ProfileY(V, rateFrac_f, &epe_policer1_profileY, p_sys_profile->rateFrac);
            SetDsEpePolicing1ProfileY(V, rateShift_f, &epe_policer1_profileY, p_sys_profile->rateShift);
            SetDsEpePolicing1ProfileY(V, rate_f, &epe_policer1_profileY, p_sys_profile->rate);
            SetDsEpePolicing1ProfileY(V, thresholdShift_f, &epe_policer1_profileY, p_sys_profile->thresholdShift);
            SetDsEpePolicing1ProfileY(V, threshold_f, &epe_policer1_profileY, p_sys_profile->threshold);
            SetDsEpePolicing1ProfileY(V, rateMaxShift_f, &epe_policer1_profileY, p_sys_profile->rateMaxShift);
            SetDsEpePolicing1ProfileY(V, rateMax_f, &epe_policer1_profileY, p_sys_profile->rateMax);
            SetDsEpePolicing1ProfileY(V, ppsShift_f, &epe_policer1_profileY, 0);

            cmd = DRV_IOW(DsEpePolicing1ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer1_profileY));
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "write Policer Profile, profid = %d\n",
                              p_sys_profile->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_profile_get_from_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_profile_t* p_sys_profile,
                                                            uint8 is_bucketY)
{
    uint32 cmd = 0;
    DsIpePolicing0ProfileX_m ipe_policer0_profileX;
    DsIpePolicing0ProfileY_m ipe_policer0_profileY;
    DsIpePolicing1ProfileX_m ipe_policer1_profileX;
    DsIpePolicing1ProfileY_m ipe_policer1_profileY;
    DsEpePolicing0ProfileX_m epe_policer0_profileX;
    DsEpePolicing0ProfileY_m epe_policer0_profileY;
    DsEpePolicing1ProfileX_m epe_policer1_profileX;
    DsEpePolicing1ProfileY_m epe_policer1_profileY;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_profile);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    if (!is_bucketY)
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            sal_memset(&ipe_policer0_profileX, 0, sizeof(DsIpePolicing0ProfileX_m));
            cmd = DRV_IOR(DsIpePolicing0ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer0_profileX));

            p_sys_profile->rateFrac = GetDsIpePolicing0ProfileX(V, rateFrac_f, &ipe_policer0_profileX);
            p_sys_profile->rateShift = GetDsIpePolicing0ProfileX(V, rateShift_f, &ipe_policer0_profileX);
            p_sys_profile->rate = GetDsIpePolicing0ProfileX(V, rate_f, &ipe_policer0_profileX);
            p_sys_profile->thresholdShift = GetDsIpePolicing0ProfileX(V, thresholdShift_f, &ipe_policer0_profileX);
            p_sys_profile->threshold = GetDsIpePolicing0ProfileX(V, threshold_f, &ipe_policer0_profileX);
            p_sys_profile->rateMaxShift = GetDsIpePolicing0ProfileX(V, rateMaxShift_f, &ipe_policer0_profileX);
            p_sys_profile->rateMax = GetDsIpePolicing0ProfileX(V, rateMax_f, &ipe_policer0_profileX);
            break;

        case SYS_QOS_POLICER_IPE_POLICING_1:
            sal_memset(&ipe_policer1_profileX, 0, sizeof(DsIpePolicing1ProfileX_m));
            cmd = DRV_IOR(DsIpePolicing1ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer1_profileX));

            p_sys_profile->rateFrac = GetDsIpePolicing1ProfileX(V, rateFrac_f, &ipe_policer1_profileX);
            p_sys_profile->rateShift = GetDsIpePolicing1ProfileX(V, rateShift_f, &ipe_policer1_profileX);
            p_sys_profile->rate = GetDsIpePolicing1ProfileX(V, rate_f, &ipe_policer1_profileX);
            p_sys_profile->thresholdShift = GetDsIpePolicing1ProfileX(V, thresholdShift_f, &ipe_policer1_profileX);
            p_sys_profile->threshold = GetDsIpePolicing1ProfileX(V, threshold_f, &ipe_policer1_profileX);
            p_sys_profile->rateMaxShift = GetDsIpePolicing1ProfileX(V, rateMaxShift_f, &ipe_policer1_profileX);
            p_sys_profile->rateMax = GetDsIpePolicing1ProfileX(V, rateMax_f, &ipe_policer1_profileX);
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            sal_memset(&epe_policer0_profileX, 0, sizeof(DsEpePolicing0ProfileX_m));
            cmd = DRV_IOR(DsEpePolicing0ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer0_profileX));

            p_sys_profile->rateFrac = GetDsEpePolicing0ProfileX(V, rateFrac_f, &epe_policer0_profileX);
            p_sys_profile->rateShift = GetDsEpePolicing0ProfileX(V, rateShift_f, &epe_policer0_profileX);
            p_sys_profile->rate = GetDsEpePolicing0ProfileX(V, rate_f, &epe_policer0_profileX);
            p_sys_profile->thresholdShift = GetDsEpePolicing0ProfileX(V, thresholdShift_f, &epe_policer0_profileX);
            p_sys_profile->threshold = GetDsEpePolicing0ProfileX(V, threshold_f, &epe_policer0_profileX);
            p_sys_profile->rateMaxShift = GetDsEpePolicing0ProfileX(V, rateMaxShift_f, &epe_policer0_profileX);
            p_sys_profile->rateMax = GetDsEpePolicing0ProfileX(V, rateMax_f, &epe_policer0_profileX);
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            sal_memset(&epe_policer1_profileX, 0, sizeof(DsEpePolicing1ProfileX_m));
            cmd = DRV_IOR(DsEpePolicing1ProfileX_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer1_profileX));

            p_sys_profile->rateFrac = GetDsEpePolicing1ProfileX(V, rateFrac_f, &epe_policer1_profileX);
            p_sys_profile->rateShift = GetDsEpePolicing1ProfileX(V, rateShift_f, &epe_policer1_profileX);
            p_sys_profile->rate = GetDsEpePolicing1ProfileX(V, rate_f, &epe_policer1_profileX);
            p_sys_profile->thresholdShift = GetDsEpePolicing1ProfileX(V, thresholdShift_f, &epe_policer1_profileX);
            p_sys_profile->threshold = GetDsEpePolicing1ProfileX(V, threshold_f, &epe_policer1_profileX);
            p_sys_profile->rateMaxShift = GetDsEpePolicing1ProfileX(V, rateMaxShift_f, &epe_policer1_profileX);
            p_sys_profile->rateMax = GetDsEpePolicing1ProfileX(V, rateMax_f, &epe_policer1_profileX);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        switch(p_sys_policer->policer_lvl)
        {
        case SYS_QOS_POLICER_IPE_POLICING_0:
            sal_memset(&ipe_policer0_profileY, 0, sizeof(DsIpePolicing0ProfileY_m));
            cmd = DRV_IOR(DsIpePolicing0ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer0_profileY));

            p_sys_profile->rateFrac = GetDsIpePolicing0ProfileY(V, rateFrac_f, &ipe_policer0_profileY);
            p_sys_profile->rateShift = GetDsIpePolicing0ProfileY(V, rateShift_f, &ipe_policer0_profileY);
            p_sys_profile->rate = GetDsIpePolicing0ProfileY(V, rate_f, &ipe_policer0_profileY);
            p_sys_profile->thresholdShift = GetDsIpePolicing0ProfileY(V, thresholdShift_f, &ipe_policer0_profileY);
            p_sys_profile->threshold = GetDsIpePolicing0ProfileY(V, threshold_f, &ipe_policer0_profileY);
            p_sys_profile->rateMaxShift = GetDsIpePolicing0ProfileY(V, rateMaxShift_f, &ipe_policer0_profileY);
            p_sys_profile->rateMax = GetDsIpePolicing0ProfileY(V, rateMax_f, &ipe_policer0_profileY);
            break;

        case SYS_QOS_POLICER_IPE_POLICING_1:
            sal_memset(&ipe_policer1_profileY, 0, sizeof(DsIpePolicing1ProfileY_m));
            cmd = DRV_IOR(DsIpePolicing1ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &ipe_policer1_profileY));

            p_sys_profile->rateFrac = GetDsIpePolicing1ProfileY(V, rateFrac_f, &ipe_policer1_profileY);
            p_sys_profile->rateShift = GetDsIpePolicing1ProfileY(V, rateShift_f, &ipe_policer1_profileY);
            p_sys_profile->rate = GetDsIpePolicing1ProfileY(V, rate_f, &ipe_policer1_profileY);
            p_sys_profile->thresholdShift = GetDsIpePolicing1ProfileY(V, thresholdShift_f, &ipe_policer1_profileY);
            p_sys_profile->threshold = GetDsIpePolicing1ProfileY(V, threshold_f, &ipe_policer1_profileY);
            p_sys_profile->rateMaxShift = GetDsIpePolicing1ProfileY(V, rateMaxShift_f, &ipe_policer1_profileY);
            p_sys_profile->rateMax = GetDsIpePolicing1ProfileY(V, rateMax_f, &ipe_policer1_profileY);
            break;

        case SYS_QOS_POLICER_EPE_POLICING_0:
            sal_memset(&epe_policer0_profileY, 0, sizeof(DsEpePolicing0ProfileY_m));
            cmd = DRV_IOR(DsEpePolicing0ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer0_profileY));

            p_sys_profile->rateFrac = GetDsEpePolicing0ProfileY(V, rateFrac_f, &epe_policer0_profileY);
            p_sys_profile->rateShift = GetDsEpePolicing0ProfileY(V, rateShift_f, &epe_policer0_profileY);
            p_sys_profile->rate = GetDsEpePolicing0ProfileY(V, rate_f, &epe_policer0_profileY);
            p_sys_profile->thresholdShift = GetDsEpePolicing0ProfileY(V, thresholdShift_f, &epe_policer0_profileY);
            p_sys_profile->threshold = GetDsEpePolicing0ProfileY(V, threshold_f, &epe_policer0_profileY);
            p_sys_profile->rateMaxShift = GetDsEpePolicing0ProfileY(V, rateMaxShift_f, &epe_policer0_profileY);
            p_sys_profile->rateMax = GetDsEpePolicing0ProfileY(V, rateMax_f, &epe_policer0_profileY);
            break;

        case SYS_QOS_POLICER_EPE_POLICING_1:
            sal_memset(&epe_policer1_profileY, 0, sizeof(DsEpePolicing1ProfileY_m));
            cmd = DRV_IOR(DsEpePolicing1ProfileY_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &epe_policer1_profileY));

            p_sys_profile->rateFrac = GetDsEpePolicing1ProfileY(V, rateFrac_f, &epe_policer1_profileY);
            p_sys_profile->rateShift = GetDsEpePolicing1ProfileY(V, rateShift_f, &epe_policer1_profileY);
            p_sys_profile->rate = GetDsEpePolicing1ProfileY(V, rate_f, &epe_policer1_profileY);
            p_sys_profile->thresholdShift = GetDsEpePolicing1ProfileY(V, thresholdShift_f, &epe_policer1_profileY);
            p_sys_profile->threshold = GetDsEpePolicing1ProfileY(V, threshold_f, &epe_policer1_profileY);
            p_sys_profile->rateMaxShift = GetDsEpePolicing1ProfileY(V, rateMaxShift_f, &epe_policer1_profileY);
            p_sys_profile->rateMax = GetDsEpePolicing1ProfileY(V, rateMax_f, &epe_policer1_profileY);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

/**
 @brief Write policer action to asic.
*/
STATIC int32
_sys_usw_qos_policer_action_profile_add_to_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_action_t* p_sys_action_profile)
{
    uint32 cmd = 0;
    DsIpePolicingPolicy_m ipe_policer_action_profile;
    DsEpePolicingPolicy_m epe_policer_action_profile;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_action_profile);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    switch(p_sys_policer->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
    case SYS_QOS_POLICER_IPE_POLICING_1:
        sal_memset(&ipe_policer_action_profile, 0, sizeof(DsIpePolicingPolicy_m));
        SetDsIpePolicingPolicy(V, discardOpTypeGreen_f, &ipe_policer_action_profile, p_sys_action_profile->discard_green);
        SetDsIpePolicingPolicy(V, discardOpTypeRed_f, &ipe_policer_action_profile, p_sys_action_profile->discard_red);
        SetDsIpePolicingPolicy(V, discardOpTypeYellow_f, &ipe_policer_action_profile, p_sys_action_profile->discard_yellow);
        SetDsIpePolicingPolicy(V, prioGreenValid_f, &ipe_policer_action_profile, CTC_FLAG_ISSET(p_sys_action_profile->flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID));
        SetDsIpePolicingPolicy(V, prioGreen_f, &ipe_policer_action_profile, p_sys_action_profile->prio_green);
        SetDsIpePolicingPolicy(V, prioYellowValid_f, &ipe_policer_action_profile, CTC_FLAG_ISSET(p_sys_action_profile->flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID));
        SetDsIpePolicingPolicy(V, prioYellow_f, &ipe_policer_action_profile, p_sys_action_profile->prio_yellow);
        SetDsIpePolicingPolicy(V, prioRedValid_f, &ipe_policer_action_profile, CTC_FLAG_ISSET(p_sys_action_profile->flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID));
        SetDsIpePolicingPolicy(V, prioRed_f, &ipe_policer_action_profile, p_sys_action_profile->prio_red);

        cmd = DRV_IOW(DsIpePolicingPolicy_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_action_profile->profile_id, cmd, &ipe_policer_action_profile));

        break;

    case SYS_QOS_POLICER_EPE_POLICING_0:
    case SYS_QOS_POLICER_EPE_POLICING_1:
        sal_memset(&epe_policer_action_profile, 0, sizeof(DsEpePolicingPolicy_m));
        SetDsEpePolicingPolicy(V, discardOpTypeGreen_f, &epe_policer_action_profile, p_sys_action_profile->discard_green);
        SetDsEpePolicingPolicy(V, discardOpTypeRed_f, &epe_policer_action_profile, p_sys_action_profile->discard_red);
        SetDsEpePolicingPolicy(V, discardOpTypeYellow_f, &epe_policer_action_profile, p_sys_action_profile->discard_yellow);
        cmd = DRV_IOW(DsEpePolicingPolicy_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_action_profile->profile_id, cmd, &epe_policer_action_profile));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "write Policer action Profile, profid = %d\n",
                             p_sys_action_profile->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_action_profile_get_from_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_action_t* p_sys_action_profile)
{
    uint32 cmd = 0;
    DsIpePolicingPolicy_m ipe_policer_action_profile;
    DsEpePolicingPolicy_m epe_policer_action_profile;
    uint32 flag;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_action_profile);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    switch(p_sys_policer->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
    case SYS_QOS_POLICER_IPE_POLICING_1:
        sal_memset(&ipe_policer_action_profile, 0, sizeof(DsIpePolicingPolicy_m));
        cmd = DRV_IOR(DsIpePolicingPolicy_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_action_profile->profile_id, cmd, &ipe_policer_action_profile));

        p_sys_action_profile->discard_green = GetDsIpePolicingPolicy(V, discardOpTypeGreen_f, &ipe_policer_action_profile);
        p_sys_action_profile->discard_red = GetDsIpePolicingPolicy(V, discardOpTypeRed_f, &ipe_policer_action_profile);
        p_sys_action_profile->discard_yellow = GetDsIpePolicingPolicy(V, discardOpTypeYellow_f, &ipe_policer_action_profile);
        flag = GetDsIpePolicingPolicy(V, prioGreenValid_f, &ipe_policer_action_profile);
        p_sys_action_profile->flag |= flag;
        p_sys_action_profile->prio_green = GetDsIpePolicingPolicy(V, prioGreen_f, &ipe_policer_action_profile);
        flag = GetDsIpePolicingPolicy(V, prioRedValid_f, &ipe_policer_action_profile);
        p_sys_action_profile->flag |= flag << 1;
        p_sys_action_profile->prio_red = GetDsIpePolicingPolicy(V, prioRed_f, &ipe_policer_action_profile);
        flag = GetDsIpePolicingPolicy(V, prioYellowValid_f, &ipe_policer_action_profile);
        p_sys_action_profile->flag |= flag << 2;
        p_sys_action_profile->prio_yellow = GetDsIpePolicingPolicy(V, prioYellow_f, &ipe_policer_action_profile);
        break;

    case SYS_QOS_POLICER_EPE_POLICING_0:
    case SYS_QOS_POLICER_EPE_POLICING_1:
        sal_memset(&epe_policer_action_profile, 0, sizeof(DsEpePolicingPolicy_m));
        cmd = DRV_IOR(DsEpePolicingPolicy_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_action_profile->profile_id, cmd, &epe_policer_action_profile));

        p_sys_action_profile->discard_green = GetDsEpePolicingPolicy(V, discardOpTypeGreen_f, &epe_policer_action_profile);
        p_sys_action_profile->discard_red = GetDsEpePolicingPolicy(V, discardOpTypeRed_f, &epe_policer_action_profile);
        p_sys_action_profile->discard_yellow = GetDsEpePolicingPolicy(V, discardOpTypeYellow_f, &epe_policer_action_profile);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_profile_remove_from_asic(uint8 lchip, sys_qos_policer_profile_t* p_sys_profile)
{
    /*only remove from SW DB*/
    return CTC_E_NONE;
}

/**
 @brief Lookup policer in hash table.
*/
STATIC int32
_sys_usw_qos_policer_lookup(uint8 lchip, uint8 type,
                                  uint8 dir,
                                  uint16 policer_id,
                                  sys_qos_policer_t** pp_sys_policer)
{
    sys_qos_policer_t sys_policer;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(pp_sys_policer);

    sys_policer.type = type;
    sys_policer.dir = dir;
    sys_policer.id = policer_id;

    *pp_sys_policer = ctc_hash_lookup(p_usw_qos_policer_master[lchip]->p_policer_hash, &sys_policer);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_alloc_offset(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    uint8 dir = 0;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_PORT)
    {
        uint16 lport = CTC_MAP_GPORT_TO_LPORT(p_sys_policer->id);
        dir = p_sys_policer->dir;
        p_sys_policer->policer_ptr = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) ? SYS_QOS_PORT_POLICER_INDEX(lport, dir, 0) :
                                     (SYS_QOS_PORT_POLICER_INDEX(lport, dir, 0) << 1);

        p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_PORT_POLICER] += 1;
        p_sys_policer->entry_size = 1;
    }
    else if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_VLAN)
    {
        p_sys_policer->policer_ptr = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) ? (p_sys_policer->policer_ptr) : ((p_sys_policer->policer_ptr) << 1);
        p_sys_policer->policer_ptr = p_usw_qos_policer_master[lchip]->vlan_policer_base[p_sys_policer->dir] + p_sys_policer->policer_ptr;

        p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_VLAN_POLICER] += 1;
        p_sys_policer->entry_size = 1;
    }
    else
    {
        uint32 offset = 0;
        sys_usw_opf_t opf;
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = p_sys_policer->policer_lvl;
        opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_policer_config;
        /* two bucket must double entry size, expect policer_lvl is SYS_QOS_POLICER_IPE_POLICING_1*/
        if (p_sys_policer->hbwp_en && p_sys_policer->policer_lvl == SYS_QOS_POLICER_IPE_POLICING_1)
        {
            p_sys_policer->entry_size = MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL) ;

            CTC_MAX_VALUE_CHECK(p_sys_policer->cos_index, p_sys_policer->entry_size - 1);
            CTC_BIT_SET(p_sys_policer->cos_bitmap, p_sys_policer->cos_index);
            /*D2 asic must 4 mulitple,TM is 8*/
        }
        else if (p_sys_policer->hbwp_en && p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
        {
            p_sys_policer->entry_size = MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL) ;

            CTC_MAX_VALUE_CHECK(p_sys_policer->cos_index, p_sys_policer->entry_size - 1);
            CTC_BIT_SET(p_sys_policer->cos_bitmap, p_sys_policer->cos_index);
            p_sys_policer->entry_size = MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL) * 2;
        }
        else if (p_sys_policer->policer_lvl == SYS_QOS_POLICER_IPE_POLICING_1 || p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
        {
            p_sys_policer->entry_size = 1;
        }
        else
        {
            p_sys_policer->entry_size = 2;
        }
        opf.multiple =  p_sys_policer->entry_size;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, p_sys_policer->entry_size, &offset));

        p_sys_policer->policer_ptr = offset;
        p_sys_policer->split_en = (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM);
        if(p_sys_policer->hbwp_en)
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] += MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL);
        }
        else
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] += 1;
        }
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Alloc policer offset = %d\n", p_sys_policer->policer_ptr);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_free_offset(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    CTC_PTR_VALID_CHECK(p_sys_policer);

    if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_PORT)
    {
        if (p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_PORT_POLICER] >= 1)
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_PORT_POLICER] -= 1;
        }
    }
    else if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_VLAN)
    {
        if (p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_VLAN_POLICER] >= 1)
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_VLAN_POLICER] -= 1;
        }
    }
    else
    {
        uint32 offset = p_sys_policer->policer_ptr;
        sys_usw_opf_t opf;
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = p_sys_policer->policer_lvl;
        opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_policer_config;

        p_sys_policer->policer_ptr = 0;
        if (p_sys_policer->entry_size)
        {
            CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, p_sys_policer->entry_size, offset));
        }
        if (p_sys_policer->hbwp_en && p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] >= MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL))
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] -= MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL);
        }
        else if(!p_sys_policer->hbwp_en && p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] >= 1)
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] -= 1;
        }
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free policer offset = %d\n", p_sys_policer->policer_ptr);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_action_profile_get(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                  sys_qos_policer_action_t* p_sys_action_profile_get)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_action_profile_get);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    p_sys_action_profile_get->flag= p_sys_policer->flag;
    p_sys_action_profile_get->dir = (p_sys_policer->policer_lvl) / 2;
    if (p_sys_action_profile_get->dir == CTC_INGRESS)
    {
        p_sys_action_profile_get->prio_green = p_sys_policer->prio_green;
        p_sys_action_profile_get->prio_yellow = p_sys_policer->prio_yellow;
        p_sys_action_profile_get->prio_red = p_sys_policer->prio_red;
    }
    /* policer_merge_mode support action drop color,but the action need same as drop color*/
    if (CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN) ||
        CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW) ||
        CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_RED))
    {
        p_sys_action_profile_get->discard_green = CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN);
        p_sys_action_profile_get->discard_yellow = CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW);
        p_sys_action_profile_get->discard_red = CTC_FLAG_ISSET(p_sys_action_profile_get->flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_RED);
    }
    else if (p_sys_policer->drop_color == CTC_QOS_COLOR_NONE)
    {
        p_sys_action_profile_get->discard_green = 0;
        p_sys_action_profile_get->discard_yellow = 0;
        p_sys_action_profile_get->discard_red = 0;
    }
    else if (p_sys_policer->drop_color == CTC_QOS_COLOR_RED)
    {
        p_sys_action_profile_get->discard_green = 0;
        p_sys_action_profile_get->discard_yellow = 0;
        p_sys_action_profile_get->discard_red = 1;
    }
    else if (p_sys_policer->drop_color == CTC_QOS_COLOR_YELLOW)
    {
        p_sys_action_profile_get->discard_green = 0;
        p_sys_action_profile_get->discard_yellow = 1;
        p_sys_action_profile_get->discard_red = 1;
    }
    else
    {
        p_sys_action_profile_get->discard_green = 1;
        p_sys_action_profile_get->discard_yellow = 1;
        p_sys_action_profile_get->discard_red = 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_profile_add_spool(uint8 lchip, sys_qos_policer_profile_t* p_sys_profile_old,
                                                          sys_qos_policer_profile_t* p_sys_profile_new,
                                                          sys_qos_policer_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_policer_profile_hash_cmp(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_qos_policer_master[lchip]->p_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_action_profile_add_spool(uint8 lchip, sys_qos_policer_action_t* p_sys_profile_old,
                                                          sys_qos_policer_action_t* p_sys_profile_new,
                                                          sys_qos_policer_action_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_policer_action_profile_hash_cmp(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_qos_policer_master[lchip]->p_action_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_profile_remove(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                      sys_qos_policer_profile_t* p_sys_profileX,
                                                      sys_qos_policer_profile_t* p_sys_profileY,
                                                      sys_qos_policer_action_t*  p_sys_action_profile)
{
    ctc_spool_t* p_profile_pool = NULL;
    uint8 is_bucketY = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*remove policer profile*/
    p_profile_pool = p_usw_qos_policer_master[lchip]->p_profile_pool;
    if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);
        if (is_bucketY)
        {
            if (p_sys_profileY)
            {
                CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profileY, NULL));
            }
        }
        else
        {
            if (p_sys_profileX)
            {
                CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profileX, NULL));
            }
        }
    }
    else
    {
        if (p_sys_profileX && p_sys_profileY)
        {
            CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profileX, NULL));
            CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profileY, NULL));
        }
    }

    /*remove policer action profile*/
    p_profile_pool = p_usw_qos_policer_master[lchip]->p_action_profile_pool;
    if (p_sys_action_profile)
    {
        CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_action_profile, NULL));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_profile_add(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                  sys_qos_policer_profile_t** pp_sys_profile_getX,
                                                  sys_qos_policer_profile_t** pp_sys_profile_getY,
                                                  sys_qos_policer_action_t**  pp_sys_action_profile_get)
{
    sys_qos_policer_profile_t* p_sys_profile_old = NULL;
    sys_qos_policer_profile_t sys_profile_newX;
    sys_qos_policer_profile_t sys_profile_newY;
    sys_qos_policer_action_t* p_sys_action_profile_old = NULL;
    sys_qos_policer_action_t  sys_action_profile_new;
    uint8 cos_index      = 0;
    uint8 is_bucketY = 0;
    ctc_spool_t* p_profile_pool = NULL;
    int32  ret = 0;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&sys_profile_newX, 0, sizeof(sys_qos_policer_profile_t));
    sal_memset(&sys_profile_newY, 0, sizeof(sys_qos_policer_profile_t));
    sal_memset(&sys_action_profile_new, 0, sizeof(sys_qos_policer_action_t));

    cos_index = p_sys_policer->cos_index;
    p_profile_pool = p_usw_qos_policer_master[lchip]->p_profile_pool;
    /*add policer profile*/
    /*CTC_QOS_POLICER_MODE_STBM means single token bucket, so need config bucket X/Y according to policer_ptr
      even policer_ptr is bucketX, odd policer_ptr is bucketY*/
    if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);
        if (is_bucketY)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_build_data(lchip, p_sys_policer, &sys_profile_newY, is_bucketY));
            p_sys_profile_old = p_sys_policer->p_sys_profileY[cos_index];
            if(p_sys_policer->p_sys_profileX[cos_index])
            {
                ctc_spool_remove(p_profile_pool, p_sys_policer->p_sys_profileX[cos_index], NULL);
                p_sys_policer->p_sys_profileX[cos_index] = NULL;
            }
            CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_newY, pp_sys_profile_getY));
            p_sys_policer->p_sys_profileY[cos_index] = *pp_sys_profile_getY;
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_build_data(lchip, p_sys_policer, &sys_profile_newX, is_bucketY));
            p_sys_profile_old = p_sys_policer->p_sys_profileX[cos_index];
            if(p_sys_policer->p_sys_profileY[cos_index])
            {
                ctc_spool_remove(p_profile_pool, p_sys_policer->p_sys_profileY[cos_index], NULL);
                p_sys_policer->p_sys_profileY[cos_index] = NULL;
            }
            CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_newX, pp_sys_profile_getX));
            p_sys_policer->p_sys_profileX[cos_index] = *pp_sys_profile_getX;

        }
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_build_data(lchip, p_sys_policer, &sys_profile_newX, 0));
        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_build_data(lchip, p_sys_policer, &sys_profile_newY, 1));

        p_sys_profile_old = p_sys_policer->p_sys_profileX[cos_index];
        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_newX, pp_sys_profile_getX));
        p_sys_policer->p_sys_profileX[cos_index] = *pp_sys_profile_getX;

        p_sys_profile_old = p_sys_policer->p_sys_profileY[cos_index];
        CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_newY, pp_sys_profile_getY),ret,error1);
        p_sys_policer->p_sys_profileY[cos_index] = *pp_sys_profile_getY;
    }

    /*add policer action profile*/
    CTC_ERROR_GOTO(_sys_usw_qos_policer_action_profile_get(lchip, p_sys_policer, &sys_action_profile_new),ret,error2);
    p_sys_action_profile_old = p_sys_policer->p_sys_action_profile[cos_index];
    CTC_ERROR_GOTO(_sys_usw_qos_policer_action_profile_add_spool(lchip, p_sys_action_profile_old, &sys_action_profile_new, pp_sys_action_profile_get),ret,error2);
    p_sys_policer->p_sys_action_profile[cos_index] = *pp_sys_action_profile_get;

    return CTC_E_NONE;
    error2:
        if((p_sys_policer->policer_mode != CTC_QOS_POLICER_MODE_STBM)||
            (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM && is_bucketY))
        {
            ctc_spool_remove(p_profile_pool, *pp_sys_profile_getY, NULL);
        }
        else if(p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM && !is_bucketY)
        {
            ctc_spool_remove(p_profile_pool, *pp_sys_profile_getX, NULL);
        }
    error1:
        if(p_sys_policer->policer_mode != CTC_QOS_POLICER_MODE_STBM)
        {
            ctc_spool_remove(p_profile_pool, *pp_sys_profile_getX, NULL);
        }

    return ret;

}

int32
_sys_usw_qos_policer_build_node(uint8 lchip, ctc_qos_policer_t* p_policer_param,
                                      sys_qos_policer_t** pp_sys_policer)
{
    sys_qos_policer_t* p_sys_policer = NULL;
    uint16 policer_id = 0;
    uint8 first_add = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_policer_param);
    CTC_PTR_VALID_CHECK(pp_sys_policer);

    policer_id = _sys_usw_qos_get_policer_id(p_policer_param->type,
                                             p_policer_param->id.policer_id, p_policer_param->id.gport,
                                             p_policer_param->id.vlan_id);

    if (NULL == *pp_sys_policer)
    {
        first_add = 1;
    }

    if (first_add)
    {
        /*new policer node*/
        p_sys_policer = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_policer_t));
        if (NULL == p_sys_policer)
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }

        sal_memset(p_sys_policer, 0, sizeof(sys_qos_policer_t));

        p_sys_policer->type = p_policer_param->type;
        p_sys_policer->dir = p_policer_param->dir;
        p_sys_policer->id = policer_id;
    }
    else
    {
        /*update policer node*/
        p_sys_policer = *pp_sys_policer;
    }

    p_sys_policer->hbwp_en = p_policer_param->hbwp_en;
    p_sys_policer->cos_index = p_policer_param->hbwp.cos_index;
    p_sys_policer->sf_en = p_policer_param->hbwp.sf_en;
    p_sys_policer->cf_total_dis = p_policer_param->hbwp.cf_total_dis;
    p_sys_policer->policer_mode = p_policer_param->policer.policer_mode;
    p_sys_policer->cf_en = p_policer_param->policer.cf_en;
    p_sys_policer->is_color_aware = p_policer_param->policer.is_color_aware;
    p_sys_policer->use_l3_length = p_policer_param->policer.use_l3_length;
    p_sys_policer->drop_color = p_policer_param->policer.drop_color;
    p_sys_policer->stats_en = p_policer_param->policer.stats_en;
    p_sys_policer->stats_mode = p_policer_param->policer.stats_mode;
    p_sys_policer->level = p_policer_param->level;
    p_sys_policer->cir = p_policer_param->policer.cir;
    p_sys_policer->cbs = p_policer_param->policer.cbs;
    p_sys_policer->pir = p_policer_param->policer.pir;
    p_sys_policer->pbs = p_policer_param->policer.pbs;
    p_sys_policer->cir_max = p_policer_param->hbwp.cir_max;
    p_sys_policer->pir_max = p_policer_param->hbwp.pir_max;
    p_sys_policer->prio_green = p_policer_param->action.prio_green;
    p_sys_policer->prio_yellow = p_policer_param->action.prio_yellow;
    p_sys_policer->prio_red = p_policer_param->action.prio_red;
    p_sys_policer->flag = p_policer_param->action.flag;
    p_sys_policer->is_pps = p_policer_param->pps_en;
    p_sys_policer->color_merge_mode = p_policer_param->policer.color_merge_mode;

    if (first_add)
    {
        ctc_hash_insert(p_usw_qos_policer_master[lchip]->p_policer_hash, p_sys_policer);
    }

    *pp_sys_policer = p_sys_policer;

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_delete_node(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_policer);

    ctc_hash_remove(p_usw_qos_policer_master[lchip]->p_policer_hash, p_sys_policer);

    mem_free(p_sys_policer);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_add_to_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                       sys_qos_policer_copp_profile_t* p_sys_prof)
{
    uint32 cmd         = 0;
    uint32 bucket_cnt = 0;
    uint16 policer_ptr = 0;
    uint8  is_pps = 0;
    uint32 value = 0;
    DsIpeCoPPConfig_m copp_cfg;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_prof);

    sal_memset(&copp_cfg,0,sizeof(DsIpeCoPPConfig_m));

    bucket_cnt = (p_sys_prof->threshold << p_sys_prof->thresholdShift);
    policer_ptr = p_sys_policer->policer_ptr;
    is_pps = p_sys_policer->is_pps;

    if (bucket_cnt > 0x1ffffff)
    {
        bucket_cnt = SYS_QOS_POLICER_MAX_COUNT;
    }

    cmd = DRV_IOR(DsIpeCoPPConfig_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &copp_cfg));
    SetDsIpeCoPPConfig(V, statsMode_f, &copp_cfg, !p_sys_policer->stats_mode);
    SetDsIpeCoPPConfig(V, statsPtr_f, &copp_cfg, p_sys_policer->stats_ptr[0]);

    if (p_sys_prof->is_bucketY)
    {
        SetDsIpeCoPPConfig(V, profIdY_f, &copp_cfg, p_sys_prof->profile_id);
        SetDsIpeCoPPConfig(V, ppsModeY_f, &copp_cfg, is_pps);
        cmd = DRV_IOW(DsIpeCoPPCountY_t, DsIpeCoPPCountY_count_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &bucket_cnt));
        cmd = DRV_IOW(DsIpeCoPPCountY_t, DsIpeCoPPCountY_sign_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &value));
    }
    else
    {
        SetDsIpeCoPPConfig(V, profIdX_f, &copp_cfg, p_sys_prof->profile_id);
        SetDsIpeCoPPConfig(V, ppsModeX_f, &copp_cfg, is_pps);

        cmd = DRV_IOW(DsIpeCoPPCountX_t, DsIpeCoPPCountX_count_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &bucket_cnt));
        cmd = DRV_IOW(DsIpeCoPPCountX_t, DsIpeCoPPCountX_sign_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &value));
    }
    cmd = DRV_IOW(DsIpeCoPPConfig_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, policer_ptr >> 1, cmd, &copp_cfg));

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add to asic copp config,  index = %d\n",
                             policer_ptr >> 1);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_profile_add_to_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_copp_profile_t* p_sys_profile)
{
    uint32 cmd       = 0;
    uint8 is_bucketY = 0;
    DsIpeCoPPProfileX_m copp_profileX;
    DsIpeCoPPProfileY_m copp_profileY;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_profile);

    is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);

    if (!is_bucketY)
    {
        sal_memset(&copp_profileX, 0, sizeof(DsIpeCoPPProfileX_m));
        SetDsIpeCoPPProfileX(V, rateFrac_f, &copp_profileX, p_sys_profile->rateFrac);
        SetDsIpeCoPPProfileX(V, rateShift_f, &copp_profileX, p_sys_profile->rateShift);
        SetDsIpeCoPPProfileX(V, rate_f, &copp_profileX, p_sys_profile->rate);
        SetDsIpeCoPPProfileX(V, thresholdShift_f, &copp_profileX, p_sys_profile->thresholdShift);
        SetDsIpeCoPPProfileX(V, threshold_f, &copp_profileX, p_sys_profile->threshold);
        SetDsIpeCoPPProfileX(V, ppsShift_f, &copp_profileX, 7);

        cmd = DRV_IOW(DsIpeCoPPProfileX_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &copp_profileX));
    }
    else
    {
        sal_memset(&copp_profileY, 0, sizeof(DsIpeCoPPProfileY_m));
        SetDsIpeCoPPProfileY(V, rateFrac_f, &copp_profileY, p_sys_profile->rateFrac);
        SetDsIpeCoPPProfileY(V, rateShift_f, &copp_profileY, p_sys_profile->rateShift);
        SetDsIpeCoPPProfileY(V, rate_f, &copp_profileY, p_sys_profile->rate);
        SetDsIpeCoPPProfileY(V, thresholdShift_f, &copp_profileY, p_sys_profile->thresholdShift);
        SetDsIpeCoPPProfileY(V, threshold_f, &copp_profileY, p_sys_profile->threshold);
        SetDsIpeCoPPProfileY(V, ppsShift_f, &copp_profileY, 7);

        cmd = DRV_IOW(DsIpeCoPPProfileY_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &copp_profileY));
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "write copp Profile, profid = %d\n", p_sys_profile->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_profile_get_from_asic(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                            sys_qos_policer_copp_profile_t* p_sys_profile)
{
    uint32 cmd       = 0;
    uint8 is_bucketY = 0;
    DsIpeCoPPProfileX_m copp_profileX;
    DsIpeCoPPProfileY_m copp_profileY;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_profile);

    is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);

    if (!is_bucketY)
    {
        sal_memset(&copp_profileX, 0, sizeof(DsIpeCoPPProfileX_m));
        cmd = DRV_IOR(DsIpeCoPPProfileX_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &copp_profileX));

        p_sys_profile->rateFrac = GetDsIpeCoPPProfileX(V, rateFrac_f, &copp_profileX);
        p_sys_profile->rateShift = GetDsIpeCoPPProfileX(V, rateShift_f, &copp_profileX);
        p_sys_profile->rate = GetDsIpeCoPPProfileX(V, rate_f, &copp_profileX);
        p_sys_profile->thresholdShift = GetDsIpeCoPPProfileX(V, thresholdShift_f, &copp_profileX);
        p_sys_profile->threshold = GetDsIpeCoPPProfileX(V, threshold_f, &copp_profileX);
    }
    else
    {
        sal_memset(&copp_profileY, 0, sizeof(DsIpeCoPPProfileY_m));
        cmd = DRV_IOR(DsIpeCoPPProfileY_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_profile->profile_id, cmd, &copp_profileY));

        p_sys_profile->rateFrac = GetDsIpeCoPPProfileY(V, rateFrac_f, &copp_profileY);
        p_sys_profile->rateShift = GetDsIpeCoPPProfileY(V, rateShift_f, &copp_profileY);
        p_sys_profile->rate = GetDsIpeCoPPProfileY(V, rate_f, &copp_profileY);
        p_sys_profile->thresholdShift = GetDsIpeCoPPProfileY(V, thresholdShift_f, &copp_profileY);
        p_sys_profile->threshold = GetDsIpeCoPPProfileY(V, threshold_f, &copp_profileY);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_profile_add_spool(uint8 lchip, sys_qos_policer_copp_profile_t* p_sys_profile_old,
                                                          sys_qos_policer_copp_profile_t* p_sys_profile_new,
                                                          sys_qos_policer_copp_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);
    p_profile_pool = p_usw_qos_policer_master[lchip]->p_copp_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_profile_build_data(uint8 lchip, sys_qos_policer_t* p_sys_policer,
                                                         sys_qos_policer_copp_profile_t* p_sys_copp_profile)
{
    uint32 value                 = 0;
    uint32 commit_rate           = 0;
    uint32 gran                  = 0;
    uint32 max_thrd = 0;
    uint32 token_thrd = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_copp_profile->is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);

    /*pps mode gran = 1*/
    if (!p_sys_policer->is_pps)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_get_policer_gran_by_rate(lchip, p_sys_policer->pir, &gran));
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "granularity : %d\n", gran);
    }

    /* compute rate */
    CTC_ERROR_RETURN(_sys_usw_qos_policer_map_token_rate_user_to_hw(lchip, p_sys_policer->is_pps,
                                                                   p_sys_policer->pir,
                                                                   &commit_rate,
                                                                   MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_RATE_BIT_WIDTH),
                                                                   gran,
                                                                   p_usw_qos_policer_master[lchip]->copp_update_freq));

    p_sys_copp_profile->rate = commit_rate >> 12;
    p_sys_copp_profile->rateShift = (commit_rate >> 8) & 0xf;
    p_sys_copp_profile->rateFrac = commit_rate & 0xff;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_sys_profile->rate : %d\n",  p_sys_copp_profile->rate);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_sys_profile->rateShift : %d\n",  p_sys_copp_profile->rateShift);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_sys_profile->rateFrac : %d\n",  p_sys_copp_profile->rateFrac);

    /* compute threshold and threshold shift */
    if (p_sys_policer->is_pps)
    {
        value = p_sys_policer->pbs * MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
    }
    else
    {
        value = p_sys_policer->pbs * 125;   /* pbs*1000/8--> B/S*/
    }

    max_thrd = (1 << 12) - 1;

    CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, value,
                                                                  &token_thrd,
                                                                  MCHIP_CAP(SYS_CAP_QOS_POLICER_TOKEN_THRD_SHIFTS_WIDTH),
                                                                  max_thrd));
    p_sys_copp_profile->threshold = token_thrd >> 4;
    p_sys_copp_profile->thresholdShift = token_thrd & 0xF;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_sys_profile->threshold : %d\n",  p_sys_copp_profile->threshold);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " p_sys_profile->thresholdShift : %d\n",  p_sys_copp_profile->thresholdShift);

    if ((p_sys_copp_profile->rate << p_sys_copp_profile->rateShift) > (p_sys_copp_profile->threshold << p_sys_copp_profile->thresholdShift))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CBS is too samll,token will overflow!!!!\n");
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_alloc_copp_mfp_ptr(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    uint32 offset = 0;
    sys_usw_opf_t opf;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ? 0 : p_sys_policer->policer_lvl / 2;
    opf.pool_type  = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ?
                      p_usw_qos_policer_master[lchip]->opf_type_copp_config : p_usw_qos_policer_master[lchip]->opf_type_mfp_profile;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));

    p_sys_policer->policer_ptr = offset;
    p_sys_policer->entry_size = 1;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Alloc ptr = %d\n", offset);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_free_copp_mfp_ptr(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    uint32 offset = p_sys_policer->policer_ptr;
    sys_usw_opf_t opf;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ? 0 : p_sys_policer->policer_lvl / 2;
    opf.pool_type  = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ?
                      p_usw_qos_policer_master[lchip]->opf_type_copp_config : p_usw_qos_policer_master[lchip]->opf_type_mfp_profile;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, offset));
    p_sys_policer->policer_ptr = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free  ptr = %d\n", offset);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_copp_profile_add(uint8 lchip,  sys_qos_policer_t* p_sys_policer,
                                                       sys_qos_policer_copp_profile_t** pp_sys_profile_get)
{
    sys_qos_policer_copp_profile_t* p_sys_profile_old = NULL;
    sys_qos_policer_copp_profile_t sys_profile_new;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_profile_new, 0, sizeof(sys_qos_policer_copp_profile_t));

    CTC_ERROR_RETURN(_sys_usw_qos_policer_copp_profile_build_data(lchip, p_sys_policer, &sys_profile_new));

    p_sys_profile_old = p_sys_policer->p_sys_copp_profile;

    CTC_ERROR_RETURN(_sys_usw_qos_policer_copp_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_new, pp_sys_profile_get));
    p_sys_policer->p_sys_copp_profile = *pp_sys_profile_get;

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_qos_policer_copp_profile_remove(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    ctc_spool_t* p_profile_pool = NULL;

    sys_qos_policer_copp_profile_t* p_sys_profile = NULL;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_profile = p_sys_policer->p_sys_copp_profile;
    p_profile_pool = p_usw_qos_policer_master[lchip]->p_copp_profile_pool;

    CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profile, NULL));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_set_policer_en(uint8 lchip, ctc_qos_policer_t* p_policer_param)
{
    uint8 type = 0;
    uint16 policer_id = 0;
    sys_qos_policer_param_t policer_param;
    uint32 cmd = 0;
    uint32 profile_id = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_policer_param);

    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));

    if (CTC_QOS_POLICER_TYPE_PORT == p_policer_param->type)
    {
        policer_id = (uint16)(p_policer_param->id.gport);
        type = SYS_QOS_POLICER_TYPE_PORT;
        policer_param.dir = p_policer_param->dir;

        CTC_ERROR_RETURN(sys_usw_port_set_direction_property(lchip, p_policer_param->id.gport,
                                                         CTC_PORT_DIR_PROP_PORT_POLICER_VALID,
                                                         p_policer_param->dir,
                                                         p_policer_param->enable));

        if (p_policer_param->enable)
        {
            CTC_ERROR_RETURN(_sys_usw_qos_policer_index_get(lchip, policer_id,
                                                                  type,
                                                                  &policer_param));
        }
    }
    if (CTC_QOS_POLICER_TYPE_VLAN == p_policer_param->type)
    {
        policer_id = (uint16)(p_policer_param->id.vlan_id);
        type = SYS_QOS_POLICER_TYPE_VLAN;
        policer_param.dir = p_policer_param->dir;

        if(p_policer_param->dir == CTC_INGRESS)
        {
            CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, p_policer_param->id.vlan_id,
                                                         SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN,
                                                         p_policer_param->enable));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, p_policer_param->id.vlan_id,
                                                         SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN,
                                                         p_policer_param->enable));
        }
        if (p_policer_param->enable)
        {
            if(p_policer_param->dir == CTC_INGRESS)
            {
                cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_policer_param->id.vlan_id, cmd, &profile_id));
            }
            else
            {
                cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_policer_param->id.vlan_id, cmd, &profile_id));
            }
            policer_param.policer_ptr = profile_id;
            CTC_ERROR_RETURN(_sys_usw_qos_policer_index_get(lchip, policer_id,
                                                                  type,
                                                                  &policer_param));
        }
    }

    return CTC_E_NONE;

}
STATIC int32
_sys_usw_qos_policer_add(uint8 lchip, ctc_qos_policer_t* p_policer_param,
                               sys_qos_policer_t* p_sys_policer)
{
    uint8 cos_index = 0;
    sys_qos_policer_profile_t sys_profile_newX;
    sys_qos_policer_profile_t sys_profile_newY;
    sys_qos_policer_param_t policer_param;
    int32 ret = CTC_E_NONE;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_policer_param);

    sal_memset(&sys_profile_newX, 0, sizeof(sys_qos_policer_profile_t));
    sal_memset(&sys_profile_newY, 0, sizeof(sys_qos_policer_profile_t));
    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));

    cos_index = (p_policer_param->hbwp_en == 1) ? (p_policer_param->hbwp.cos_index) : 0;

    CTC_ERROR_RETURN(_sys_usw_qos_policer_build_node(lchip, p_policer_param, &p_sys_policer));

    if (p_policer_param->hbwp_en)
    {
        CTC_BIT_SET(p_sys_policer->cos_bitmap, cos_index);
        policer_param.dir = p_sys_policer->dir;
        ret = _sys_usw_qos_policer_index_get(lchip, p_sys_policer->id,
                                                              SYS_QOS_POLICER_TYPE_SERVICE,
                                                              &policer_param);
    }
    else if ((p_sys_policer->is_installed) && (CTC_QOS_POLICER_TYPE_PORT != p_policer_param->type)&&
                                         (CTC_QOS_POLICER_TYPE_VLAN != p_policer_param->type))
    {
        policer_param.dir = p_sys_policer->dir;
        ret = _sys_usw_qos_policer_index_get(lchip, p_sys_policer->id,
                                                              p_sys_policer->sys_policer_type,
                                                              &policer_param);
    }
    else
    {
        ret = _sys_usw_qos_set_policer_en(lchip, p_policer_param);
    }

    if(ret != CTC_E_NONE && p_sys_policer->is_installed == 0)
    {
        _sys_usw_qos_policer_delete_node(lchip, p_sys_policer);
    }

    return ret;
}

STATIC int32
_sys_usw_qos_policer_remove(uint8 lchip, ctc_qos_policer_t* p_policer_param,
                                  sys_qos_policer_t* p_sys_policer)
{
    int32 ret = 0;
    uint8 cos_index = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (NULL == p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_sys_policer->cos_bitmap && (!p_policer_param->hbwp_en))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_policer_param->hbwp_en == 1)
    {
        cos_index = p_policer_param->hbwp.cos_index;
        CTC_MAX_VALUE_CHECK(cos_index, p_sys_policer->entry_size - 1);
        if (!CTC_IS_BIT_SET(p_sys_policer->cos_bitmap, cos_index))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    CTC_ERROR_RETURN(_sys_usw_qos_set_policer_en(lchip, p_policer_param));

    if (0 == p_sys_policer->is_installed)
    {
        /* if has not been installed, just free policer node*/
        CTC_ERROR_RETURN(_sys_usw_qos_policer_delete_node(lchip, p_sys_policer));

        return ret;
    }

    if (p_sys_policer->stats_ptr[cos_index] != 0)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_free_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]));
        p_sys_policer->stats_ptr[cos_index] = 0;
    }

    if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_remove_from_asic(lchip,  p_policer_param, p_sys_policer));
        CTC_ERROR_RETURN(_sys_usw_qos_policer_copp_profile_remove(lchip, p_sys_policer));

        /*free policer offset*/
        CTC_ERROR_RETURN(_sys_usw_qos_policer_free_copp_mfp_ptr(lchip, p_sys_policer));

        CTC_ERROR_RETURN(_sys_usw_qos_policer_delete_node(lchip, p_sys_policer));

        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove copp !!!!!!!!!!!!\n");
    }
    else if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_MFP)
    {
        /*free policer offset*/
        CTC_ERROR_RETURN(_sys_usw_qos_policer_free_copp_mfp_ptr(lchip, p_sys_policer));

        CTC_ERROR_RETURN(_sys_usw_qos_policer_delete_node(lchip, p_sys_policer));

        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove mfp !!!!!!!!!!!!\n");
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_remove_from_asic(lchip,  p_policer_param, p_sys_policer));

        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_remove_from_asic(lchip, p_sys_policer->p_sys_profileX[cos_index]));
        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_remove_from_asic(lchip, p_sys_policer->p_sys_profileY[cos_index]));

        CTC_ERROR_RETURN(_sys_usw_qos_policer_profile_remove(lchip, p_sys_policer, p_sys_policer->p_sys_profileX[cos_index],
                        p_sys_policer->p_sys_profileY[cos_index], p_sys_policer->p_sys_action_profile[cos_index]));

        if (p_sys_policer->cos_bitmap)
        {
            CTC_BIT_UNSET(p_sys_policer->cos_bitmap, cos_index);
            p_sys_policer->p_sys_profileX[cos_index] = NULL;
            p_sys_policer->p_sys_profileY[cos_index] = NULL;
            p_sys_policer->p_sys_action_profile[cos_index] = NULL;
        }

        if (p_sys_policer->cos_bitmap == 0)
        {
            /*free policer offset*/
            CTC_ERROR_RETURN(_sys_usw_qos_policer_free_offset(lchip, p_sys_policer));

            /*free policer node*/
            CTC_ERROR_RETURN(_sys_usw_qos_policer_delete_node(lchip, p_sys_policer));

            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove policer !!!!!!!!!!!!\n");
        }
    }

    return ret;
}

STATIC int32
_sys_usw_qos_policer_copp_init(uint8 lchip)
{
    uint32 cmd         = 0;
    uint32 copp_num     = 0;
    IpeCoPPEngineCtl_m copp_engine_ctl;
    CoPPCtl_m copp_ctl;
    IpeCoppCtl_m ipe_copp_ctl;
    RefDivCoppUpdPulse_m copp_update_pulse;

    sal_memset(&copp_engine_ctl, 0, sizeof(IpeCoPPEngineCtl_m));
    sal_memset(&copp_ctl, 0, sizeof(CoPPCtl_m));
    sal_memset(&ipe_copp_ctl, 0, sizeof(IpeCoppCtl_m));
    sal_memset(&copp_update_pulse, 0, sizeof(RefDivCoppUpdPulse_m));

    SetRefDivCoppUpdPulse(V, cfgRefDivCoppUpdPulse_f, &copp_update_pulse, 0x1311d0);
    SetRefDivCoppUpdPulse(V, cfgResetDivCoppUpdPulse_f, &copp_update_pulse, 0);
    cmd = DRV_IOW(RefDivCoppUpdPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &copp_update_pulse));

    sys_usw_ftm_query_table_entry_num(lchip, DsIpeCoPPConfig_t, &copp_num); /*128*/
    SetIpeCoPPEngineCtl(V, ipeCoPPEngineCtlBucketPassConditionMode_f, &copp_engine_ctl, 1);
    SetIpeCoPPEngineCtl(V, ipeCoPPEngineCtlInterval_f, &copp_engine_ctl, 1);
    SetIpeCoPPEngineCtl(V, ipeCoPPEngineCtlMaxPtr_f, &copp_engine_ctl, copp_num - 1);
    SetIpeCoPPEngineCtl(V, ipeCoPPEngineCtlRefreshEn_f, &copp_engine_ctl, 1);
    cmd = DRV_IOW(IpeCoPPEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &copp_engine_ctl));

    SetIpeCoppCtl(V, ipgEnCopp_f,&ipe_copp_ctl,0);
    cmd = DRV_IOW(IpeCoppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_copp_ctl));

    SetCoPPCtl(V, actProfile_0_decX_f, &copp_ctl, 0);
    SetCoPPCtl(V, actProfile_0_decY_f, &copp_ctl, 0);
    SetCoPPCtl(V, actProfile_0_drop_f, &copp_ctl, 1);
    SetCoPPCtl(V, actProfile_1_decX_f, &copp_ctl, 1);
    SetCoPPCtl(V, actProfile_1_decY_f, &copp_ctl, 1);
    SetCoPPCtl(V, actProfile_1_drop_f, &copp_ctl, 0);
    cmd = DRV_IOW(CoPPCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &copp_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_mfp_init(uint8 lchip)
{
    uint32 cmd          = 0;
    uint32 profile_num  = 0;
    uint8 profile_id    = 0;
    uint8 mfp_fail_en   = 0;
    uint8 color         = 0;
    uint8 offset        = 0;
    MfpProfileCtlTable0_m mfp_profile_ctl0;
    MfpProfileCtlTable1_m mfp_profile_ctl1;
    IpeFwdMfpColorMappingCtl_m mfp_color_map_ctl;

    sal_memset(&mfp_profile_ctl0, 0, sizeof(MfpProfileCtlTable0_m));
    sal_memset(&mfp_profile_ctl1, 0, sizeof(MfpProfileCtlTable1_m));
    sal_memset(&mfp_color_map_ctl, 0, sizeof(IpeFwdMfpColorMappingCtl_m));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, MfpProfileCtlTable0_t, &profile_num));

    for (mfp_fail_en = 0; mfp_fail_en <= 1; mfp_fail_en++)
    {
        for (color = 0; color <= CTC_QOS_COLOR_GREEN; color++)
        {
            offset = 3 * (mfp_fail_en << 2 | color);
            SetIpeFwdMfpColorMappingCtl(V, array_0_mfpDiscard_f + offset, &mfp_color_map_ctl, mfp_fail_en);
            SetIpeFwdMfpColorMappingCtl(V, array_0_mfpNewColorValid_f + offset, &mfp_color_map_ctl, 1);
            SetIpeFwdMfpColorMappingCtl(V, array_0_mfpNewColor_f + offset, &mfp_color_map_ctl,
                                        (mfp_fail_en ? CTC_QOS_COLOR_RED : CTC_QOS_COLOR_GREEN));
        }
    }
    cmd = DRV_IOW(IpeFwdMfpColorMappingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mfp_color_map_ctl));

    SetMfpProfileCtlTable0(V, initToken_f, &mfp_profile_ctl0, 1);
    for (profile_id = 0; profile_id < profile_num; profile_id++)
    {
        cmd = DRV_IOW(MfpProfileCtlTable0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &mfp_profile_ctl0));
    }

    SetMfpProfileCtlTable1(V, initToken_f, &mfp_profile_ctl1, 1);
    for (profile_id = 0; profile_id < profile_num; profile_id++)
    {
        cmd = DRV_IOW(MfpProfileCtlTable1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &mfp_profile_ctl1));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_mfp_profile_add_to_asic(uint8 lchip,  sys_qos_policer_t* p_sys_policer)
{
    uint32 cmd       = 0;
    uint32 gran                  = 0;
    uint32 pir                  = 0;
    uint32 pbs                  = 0;
    MfpProfileCtlTable0_m mfp_profile0;
    MfpProfileCtlTable0_m mfp_profile1;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&mfp_profile0, 0, sizeof(MfpProfileCtlTable0_m));
    sal_memset(&mfp_profile1, 0, sizeof(MfpProfileCtlTable1_m));

    CTC_ERROR_RETURN(_sys_usw_qos_get_policer_gran_by_rate(lchip, p_sys_policer->pir, &gran));
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "granularity : %d\n", gran);

    pir = p_sys_policer->pir / gran * gran / 8;
    pbs = p_sys_policer->pbs * 125;

    if(p_sys_policer->policer_lvl == SYS_QOS_POLICER_IPE_POLICING_0)
    {
        cmd = DRV_IOR(MfpProfileCtlTable0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_policer->policer_ptr, cmd, &mfp_profile0));
        SetMfpProfileCtlTable0(V, rate_f, &mfp_profile0, pir);
        SetMfpProfileCtlTable0(V, threshold_f, &mfp_profile0, pbs);
        cmd = DRV_IOW(MfpProfileCtlTable0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_policer->policer_ptr, cmd, &mfp_profile0));
    }
    else if(p_sys_policer->policer_lvl == SYS_QOS_POLICER_EPE_POLICING_0)
    {
        cmd = DRV_IOR(MfpProfileCtlTable1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_policer->policer_ptr, cmd, &mfp_profile1));
        SetMfpProfileCtlTable0(V, rate_f, &mfp_profile1, pir);
        SetMfpProfileCtlTable0(V, threshold_f, &mfp_profile1, pbs);
        cmd = DRV_IOW(MfpProfileCtlTable1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_policer->policer_ptr, cmd, &mfp_profile1));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_policer_id_check(uint8 lchip, uint8 type, uint8 dir, uint16 policer_id)
{

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "type           = %d\n", type);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dir            = %d\n", dir);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_id     = %d\n", policer_id);

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*direction check*/
    if (dir >= CTC_BOTH_DIRECTION)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_MAX_VALUE_CHECK(type, CTC_QOS_POLICER_TYPE_MAX - 1);

    /*police type check*/
    if (CTC_QOS_POLICER_TYPE_PORT == type)
    {
        uint32 gport = 0;
        uint16 lport  = 0;

        gport = policer_id;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
        lport = SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport);
        CTC_MAX_VALUE_CHECK(lport & 0xff, (MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM) - 1));
    }
    else if (CTC_QOS_POLICER_TYPE_FLOW == type)
    {
        CTC_MIN_VALUE_CHECK(policer_id, 1);
    }
    else if (CTC_QOS_POLICER_TYPE_VLAN == type)
    {
        CTC_VLAN_RANGE_CHECK(policer_id);
    }
    else if (CTC_QOS_POLICER_TYPE_COPP == type)
    {
        CTC_MIN_VALUE_CHECK(policer_id, 1);
    }
    else if (CTC_QOS_POLICER_TYPE_MFP == type)
    {
        CTC_MIN_VALUE_CHECK(policer_id, 1);
    }
    else
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_ingress_vlan_get(uint8 lchip, uint16* ingress_vlan_policer_num)
{
    CTC_PTR_VALID_CHECK(ingress_vlan_policer_num);
    *ingress_vlan_policer_num = p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num;
    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_egress_vlan_get(uint8 lchip, uint16* egress_vlan_policer_num)
{
    CTC_PTR_VALID_CHECK(egress_vlan_policer_num);
    *egress_vlan_policer_num = p_usw_qos_policer_master[lchip]->egress_vlan_policer_num;
    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_param_check(uint8 lchip, ctc_qos_policer_t* p_policer_param)
{
    uint16 policer_id = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    ctc_qos_policer_level_select_t policer_level;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "type           = %d\n", p_policer_param->type);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dir            = %d\n", p_policer_param->dir);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "enable         = %d\n", p_policer_param->enable);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "level          = %d\n", p_policer_param->level);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_id     = %d\n", p_policer_param->id.policer_id);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "use_l3_length  = %d\n", p_policer_param->policer.use_l3_length);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "is_color_aware = %d\n", p_policer_param->policer.is_color_aware);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_en       = %d\n", p_policer_param->policer.stats_en);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_mode     = %d\n", p_policer_param->policer.stats_mode);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "cir            = %d\n", p_policer_param->policer.cir);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "cbs            = %d\n", p_policer_param->policer.cbs);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pir            = %d\n", p_policer_param->policer.pir);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pbs            = %d\n", p_policer_param->policer.pbs);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "cf_en          = %d\n", p_policer_param->policer.cf_en);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_mode   = %d\n", p_policer_param->policer.policer_mode);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hbwp_en        = %d\n", p_policer_param->hbwp_en);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "sf_en          = %d\n", p_policer_param->hbwp.sf_en);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "cir_max        = %d\n", p_policer_param->hbwp.cir_max);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "pir_max        = %d\n", p_policer_param->hbwp.pir_max);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "cos_index      = %d\n", p_policer_param->hbwp.cos_index);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag           = %d\n", p_policer_param->action.flag);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "prio_green     = %d\n", p_policer_param->action.prio_green);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "prio_yellow    = %d\n", p_policer_param->action.prio_yellow);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "prio_red       = %d\n", p_policer_param->action.prio_red);

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&policer_level, 0, sizeof(ctc_qos_policer_level_select_t));
    policer_id = _sys_usw_qos_get_policer_id(p_policer_param->type,
                                                    p_policer_param->id.policer_id, p_policer_param->id.gport,
                                                    p_policer_param->id.vlan_id);

    CTC_ERROR_RETURN(_sys_usw_qos_policer_policer_id_check(lchip, p_policer_param->type,
                                                                 p_policer_param->dir,
                                                                 policer_id));
    CTC_ERROR_RETURN(sys_usw_qos_get_policer_level_select(lchip, &policer_level));
    if ((CTC_QOS_POLICER_TYPE_VLAN == p_policer_param->type)
        && (p_policer_param->level != (p_policer_param->dir ? policer_level.egress_vlan_level:policer_level.ingress_vlan_level)) && p_policer_param->level != CTC_QOS_POLICER_LEVEL_NONE)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if ((CTC_QOS_POLICER_TYPE_PORT == p_policer_param->type)
        && (p_policer_param->level != (p_policer_param->dir ? policer_level.egress_port_level:policer_level.ingress_port_level)) && p_policer_param->level != CTC_QOS_POLICER_LEVEL_NONE)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (!p_policer_param->enable)
    {
        return CTC_E_NONE;
    }

    if((CTC_QOS_POLICER_TYPE_MFP == p_policer_param->type) && (DRV_IS_DUET2(lchip)))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if((CTC_QOS_POLICER_TYPE_COPP == p_policer_param->type) && (DRV_IS_DUET2(lchip)) &&
        p_policer_param->policer.stats_en)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en && p_policer_param->hbwp_en)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (CTC_QOS_POLICER_TYPE_VLAN == p_policer_param->type && p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    cmd = DRV_IOR(IpePolicingCtl_t, IpePolicingCtl_portPolicerSplitModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    if (CTC_QOS_POLICER_TYPE_PORT == p_policer_param->type && ((value == 1 && p_policer_param->policer.policer_mode != CTC_QOS_POLICER_MODE_STBM)
        ||(value == 0 && p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_STBM)))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if (((CTC_QOS_POLICER_TYPE_COPP == p_policer_param->type)||(CTC_QOS_POLICER_TYPE_MFP == p_policer_param->type))
        && (p_policer_param->level == CTC_QOS_POLICER_LEVEL_1))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (((CTC_QOS_POLICER_TYPE_COPP == p_policer_param->type)||(CTC_QOS_POLICER_TYPE_MFP == p_policer_param->type))
        && (p_policer_param->policer.policer_mode != CTC_QOS_POLICER_MODE_STBM))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if((CTC_QOS_POLICER_TYPE_MFP == p_policer_param->type) &&((p_policer_param->policer.stats_en == 1) ||
        (p_policer_param->pps_en == 1)))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if (p_policer_param->hbwp_en)
    {
        if (p_policer_param->type != CTC_QOS_POLICER_TYPE_FLOW)
        {
            return CTC_E_INVALID_PARAM;
        }

        if (p_policer_param->hbwp.cos_index > p_usw_qos_policer_master[lchip]->max_cos_level)
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    CTC_MAX_VALUE_CHECK(p_policer_param->policer.policer_mode, CTC_QOS_POLICER_MODE_MEF_BWP);
    CTC_MAX_VALUE_CHECK(p_policer_param->policer.color_merge_mode, CTC_QOS_POLICER_COLOR_MERGE_DUAL);
    if (p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_RFC2698)
    {
        if (p_policer_param->policer.pir < p_policer_param->policer.cir)
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer cir greater than pir \n");
            return CTC_E_INVALID_CONFIG;
        }

        if (p_policer_param->policer.pbs < p_policer_param->policer.cbs)
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer cbs greater than pbs \n");
            return CTC_E_INVALID_CONFIG;
        }
    }

    if (p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_STBM)
    {
        p_policer_param->policer.cir = p_policer_param->policer.pir;
        p_policer_param->policer.cbs = p_policer_param->policer.pbs;
    }

    if (p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_RFC2697)
    {
        p_policer_param->policer.pir = 0;
    }
    if (p_policer_param->type == CTC_QOS_POLICER_TYPE_MFP)
    {
        CTC_MAX_VALUE_CHECK(p_policer_param->policer.pir, SYS_QOS_MFP_MAX_RATE);
        if(p_policer_param->policer.pbs > SYS_QOS_MFP_MAX_COUNT)
        {
            p_policer_param->policer.pbs = SYS_QOS_MFP_MAX_COUNT;
        }
    }
    if (1 == p_policer_param->pps_en)
    {
        CTC_MAX_VALUE_CHECK(p_policer_param->hbwp.cir_max, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_PPS));
        CTC_MAX_VALUE_CHECK(p_policer_param->hbwp.pir_max, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_PPS));
        CTC_MAX_VALUE_CHECK(p_policer_param->policer.cir, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_PPS));
        CTC_MAX_VALUE_CHECK(p_policer_param->policer.pir, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_PPS));
        if (p_policer_param->policer.cbs > (SYS_QOS_POLICER_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES)))
        {
            p_policer_param->policer.cbs = SYS_QOS_POLICER_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
        }
        if (p_policer_param->policer.pbs > (SYS_QOS_POLICER_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES)))
        {
            p_policer_param->policer.pbs = SYS_QOS_POLICER_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_policer_param->hbwp.cir_max, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_KBPS));
        CTC_MAX_VALUE_CHECK(p_policer_param->hbwp.pir_max, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_KBPS));
        CTC_MAX_VALUE_CHECK(p_policer_param->policer.cir, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_KBPS));/*100G*/
        CTC_MAX_VALUE_CHECK(p_policer_param->policer.pir, MCHIP_CAP(SYS_CAP_QOS_POLICER_RATE_KBPS));/*100G*/
        if (p_policer_param->policer.cbs > (SYS_QOS_POLICER_MAX_COUNT / 125))
        {
            p_policer_param->policer.cbs = SYS_QOS_POLICER_MAX_COUNT / 125;
        }
        if (p_policer_param->policer.pbs > (SYS_QOS_POLICER_MAX_COUNT / 125))
        {
            p_policer_param->policer.pbs = SYS_QOS_POLICER_MAX_COUNT / 125;
        }
    }
    CTC_MAX_VALUE_CHECK(p_policer_param->level, CTC_QOS_POLICER_LEVEL_1);
    CTC_MAX_VALUE_CHECK(p_policer_param->action.prio_green, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(p_policer_param->action.prio_yellow, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(p_policer_param->action.prio_red, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));

    return CTC_E_NONE;
}

int32
sys_usw_qos_lookup_policer(uint8 lchip, uint8 sys_policer_type, uint8 dir, uint16 policer_id)
{
    uint8  policer_type = 0;
    sys_qos_policer_t* p_sys_policer = NULL;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "sys_policer_type  = %d\n", sys_policer_type);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dir               = %d\n", dir);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_id        = %d\n", policer_id);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (SYS_QOS_POLICER_TYPE_PORT == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_PORT;
    }
    else if (SYS_QOS_POLICER_TYPE_VLAN == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_VLAN;
    }
    else
    {
        policer_type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, policer_type,
                                                       dir,
                                                       policer_id,
                                                       &p_sys_policer));
    if (NULL == p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_get_policer_rate_and_threshold(uint8 lchip, uint8 cos_index ,ctc_qos_policer_t* p_policer_param, sys_qos_policer_t* p_sys_policer)
{
    uint32 hw_rate = 0;
    uint32 cbs = 0;
    uint32 cir = 0;
    uint32 pbs = 0;
    uint32 pir = 0;
    uint32 cir_max = 0;
    uint32 pir_max = 0;

    if (p_sys_policer->stats_ptr[cos_index])
    {
        p_policer_param->policer.stats_en = 1;
    }
    if (p_sys_policer->p_sys_profileX[cos_index])
    {
       hw_rate = (p_sys_policer->p_sys_profileX[cos_index]->rate << 12) | (p_sys_policer->p_sys_profileX[cos_index]->rateShift << 8) | (p_sys_policer->p_sys_profileX[cos_index]->rateFrac);
       _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &cir,
                                                      p_usw_qos_policer_master[lchip]->policer_update_freq);
       if (p_sys_policer->is_pps)
       {
           cbs = (p_sys_policer->p_sys_profileX[cos_index]->threshold << p_sys_policer->p_sys_profileX[cos_index]->thresholdShift) / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
       }
       else
       {
           cbs = (p_sys_policer->p_sys_profileX[cos_index]->threshold << p_sys_policer->p_sys_profileX[cos_index]->thresholdShift) / 125;     /*125 = 8/1000*/;
       }
       hw_rate = 0;
       hw_rate = (p_sys_policer->p_sys_profileX[cos_index]->rateMax << 12) | (p_sys_policer->p_sys_profileX[cos_index]->rateMaxShift << 8);
       _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &cir_max,
                                                      p_usw_qos_policer_master[lchip]->policer_update_freq);
    }
    if (p_sys_policer->p_sys_profileY[cos_index])
    {
       hw_rate = 0;
       hw_rate = (p_sys_policer->p_sys_profileY[cos_index]->rate << 12) | (p_sys_policer->p_sys_profileY[cos_index]->rateShift << 8) | (p_sys_policer->p_sys_profileY[cos_index]->rateFrac);
       _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &pir,
                                                      p_usw_qos_policer_master[lchip]->policer_update_freq);

       if (p_sys_policer->is_pps)
       {
           pbs = (p_sys_policer->p_sys_profileY[cos_index]->threshold << p_sys_policer->p_sys_profileY[cos_index]->thresholdShift) / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
       }
       else
       {
           pbs = (p_sys_policer->p_sys_profileY[cos_index]->threshold << p_sys_policer->p_sys_profileY[cos_index]->thresholdShift) / 125;     /*125 = 8/1000*/;
       }
       hw_rate = 0;
       hw_rate = (p_sys_policer->p_sys_profileY[cos_index]->rateMax << 12) | (p_sys_policer->p_sys_profileY[cos_index]->rateMaxShift << 8);
       _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &pir_max,
                                                      p_usw_qos_policer_master[lchip]->policer_update_freq);
    }
    p_policer_param->policer.pbs = pbs;
    p_policer_param->policer.cbs = cbs;
    p_policer_param->policer.cir = cir;
    p_policer_param->policer.pir = pir;
    p_policer_param->hbwp.cir_max = cir_max;
    p_policer_param->hbwp.pir_max = pir_max;

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_qos_get_policer_param(uint8 lchip, ctc_qos_policer_t* p_policer_param, sys_qos_policer_t* p_sys_policer)
{
    uint8 cos_index = 0;
    uint8 temp_cos_index = 0;
    uint16 policer_ptr = 0;
    uint32 cmd = 0;
    uint32 table_index = 0;
    uint32 tbl_id = 0;
    uint32 fld_id[6] = {0};
    uint32 value  = 0;
    DsIpePolicing0Config_m ipe_policer0_cfg;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_policer);
    CTC_PTR_VALID_CHECK(p_policer_param);

    sal_memset(&ipe_policer0_cfg,0,sizeof(DsIpePolicing0Config_m));

    cos_index = p_policer_param->hbwp.cos_index;
    if((p_policer_param->hbwp.cos_index != 0 && p_policer_param->type != CTC_QOS_POLICER_TYPE_FLOW)||
        ( p_policer_param->hbwp_en == 1 && p_sys_policer->policer_mode != CTC_QOS_POLICER_MODE_MEF_BWP))
    {
        return CTC_E_INVALID_PARAM;
    }

    policer_ptr = p_sys_policer->policer_ptr;
    p_policer_param->enable = 1;
    p_policer_param->hbwp_en = p_sys_policer->hbwp_en;
    p_policer_param->level = (p_sys_policer->policer_lvl % 2) + 1;
    p_policer_param->pps_en = p_sys_policer->is_pps;
    p_policer_param->policer.policer_mode = p_sys_policer->policer_mode;
    if(NULL != p_sys_policer->p_sys_action_profile[cos_index])
    {
        p_policer_param->action.flag = p_sys_policer->p_sys_action_profile[cos_index]->flag;
        p_policer_param->action.prio_green = p_sys_policer->p_sys_action_profile[cos_index]->prio_green;
        p_policer_param->action.prio_yellow = p_sys_policer->p_sys_action_profile[cos_index]->prio_yellow;
        p_policer_param->action.prio_red = p_sys_policer->p_sys_action_profile[cos_index]->prio_red;
        if (p_sys_policer->p_sys_action_profile[cos_index]->discard_green == 0 && p_sys_policer->p_sys_action_profile[cos_index]->discard_yellow == 0
            && p_sys_policer->p_sys_action_profile[cos_index]->discard_red == 0)
        {
            p_policer_param->policer.drop_color = CTC_QOS_COLOR_NONE;
        }
        else if (p_sys_policer->p_sys_action_profile[cos_index]->discard_green == 0 && p_sys_policer->p_sys_action_profile[cos_index]->discard_yellow == 0
            && p_sys_policer->p_sys_action_profile[cos_index]->discard_red == 1)
        {
            p_policer_param->policer.drop_color = CTC_QOS_COLOR_RED;
        }
        else if (p_sys_policer->p_sys_action_profile[cos_index]->discard_green == 0 && p_sys_policer->p_sys_action_profile[cos_index]->discard_yellow == 1
            && p_sys_policer->p_sys_action_profile[cos_index]->discard_red == 1)
        {
            p_policer_param->policer.drop_color = CTC_QOS_COLOR_YELLOW;
        }
        else
        {
            p_policer_param->policer.drop_color = CTC_QOS_COLOR_GREEN;
        }
    }
    if(p_policer_param->hbwp_en == 0)
    {
        p_policer_param->policer.is_color_aware = p_sys_policer->is_color_aware;
        p_policer_param->policer.use_l3_length = p_sys_policer->use_l3_length;
        p_policer_param->policer.stats_en = p_sys_policer->stats_en;
        p_policer_param->policer.stats_mode = p_sys_policer->stats_mode;
        p_policer_param->policer.drop_color = p_sys_policer->drop_color;
        p_policer_param->policer.cir = p_sys_policer->cir;
        p_policer_param->policer.pir = p_sys_policer->pir;
        p_policer_param->policer.cbs = p_sys_policer->cbs;
        p_policer_param->policer.pbs = p_sys_policer->pbs;
        if(p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_STBM)
        {
            p_policer_param->policer.cir = 0;
            p_policer_param->policer.cbs = 0;
        }
        p_policer_param->action.prio_green = p_sys_policer->prio_green;
        p_policer_param->action.prio_yellow = p_sys_policer->prio_yellow;
        p_policer_param->action.prio_red = p_sys_policer->prio_red;
        p_policer_param->policer.color_merge_mode = p_sys_policer->color_merge_mode;

        return CTC_E_NONE;
    }
    if(!CTC_IS_BIT_SET(p_sys_policer->cos_bitmap, cos_index))
    {
        return CTC_E_INVALID_PARAM;
    }
    if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
    {
        temp_cos_index = cos_index << 1;
    }
    else
    {
        temp_cos_index = cos_index;
    }

    if (p_sys_policer->cos_bitmap)
    {
        policer_ptr += temp_cos_index;
    }

    if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
    {
        table_index = policer_ptr >> 1;
    }
    else
    {
        table_index = policer_ptr;
    }

    switch(p_sys_policer->policer_lvl)
    {
    case SYS_QOS_POLICER_IPE_POLICING_0:
         tbl_id = DsIpePolicing0Config_t;
         fld_id[0] = DsIpePolicing0Config_colorBlindMode_f;
         fld_id[1] = DsIpePolicing0Config_couplingFlag_f;
         fld_id[2] = DsIpePolicing0Config_layer3LengthEn_f;
         fld_id[3] = DsIpePolicing0Config_statsMode_f;
         fld_id[4] = DsIpePolicing0Config_envelopeEn_f;
         fld_id[5] = DsIpePolicing0Config_couplingFlagTotal_f;
         break;

    case SYS_QOS_POLICER_IPE_POLICING_1:
         tbl_id = DsIpePolicing1Config_t;
         fld_id[0] = DsIpePolicing1Config_colorBlindMode_f;
         fld_id[1] = DsIpePolicing1Config_couplingFlag_f;
         fld_id[2] = DsIpePolicing1Config_layer3LengthEn_f;
         fld_id[3] = DsIpePolicing1Config_statsMode_f;
         fld_id[4] = DsIpePolicing1Config_envelopeEn_f;
         fld_id[5] = DsIpePolicing1Config_couplingFlagTotal_f;
         break;

    case SYS_QOS_POLICER_EPE_POLICING_0:
         tbl_id = DsEpePolicing0Config_t;
         fld_id[0] = DsEpePolicing0Config_colorBlindMode_f;
         fld_id[1] = DsEpePolicing0Config_couplingFlag_f;
         fld_id[2] = DsEpePolicing0Config_layer3LengthEn_f;
         fld_id[3] = DsEpePolicing0Config_statsMode_f;
         fld_id[4] = DsEpePolicing0Config_envelopeEn_f;
         fld_id[5] = DsEpePolicing0Config_couplingFlagTotal_f;
         break;

    case SYS_QOS_POLICER_EPE_POLICING_1:
         tbl_id = DsEpePolicing1Config_t;
         fld_id[0] = DsEpePolicing1Config_colorBlindMode_f;
         fld_id[1] = DsEpePolicing1Config_couplingFlag_f;
         fld_id[2] = DsEpePolicing1Config_layer3LengthEn_f;
         fld_id[3] = DsEpePolicing1Config_statsMode_f;
         fld_id[4] = DsEpePolicing1Config_envelopeEn_f;
         fld_id[5] = DsEpePolicing1Config_couplingFlagTotal_f;
         break;

    default:
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ipe_policer0_cfg));
    drv_get_field(lchip, tbl_id, fld_id[0], &ipe_policer0_cfg, &value);
    p_policer_param->policer.is_color_aware = value ? 0 : 1;
    drv_get_field(lchip, tbl_id, fld_id[1],&ipe_policer0_cfg, &value);
    p_policer_param->policer.cf_en = value;
    drv_get_field(lchip, tbl_id, fld_id[2],&ipe_policer0_cfg, &value);
    p_policer_param->policer.use_l3_length = value;
    drv_get_field(lchip, tbl_id, fld_id[3],&ipe_policer0_cfg, &value);
    p_policer_param->policer.stats_mode = value ? 0 : 1;

    CTC_ERROR_RETURN(_sys_usw_qos_get_policer_rate_and_threshold(lchip, cos_index, p_policer_param, p_sys_policer));
    drv_get_field(lchip, tbl_id, fld_id[4],&ipe_policer0_cfg, &value);
    p_policer_param->hbwp.sf_en = value;
    drv_get_field(lchip, tbl_id, fld_id[5],&ipe_policer0_cfg, &value);
    p_policer_param->hbwp.cf_total_dis = !value;

    return CTC_E_NONE;
}
int32
_sys_usw_qos_policer_get_node(uint8 lchip, uint8 sys_policer_type, sys_qos_policer_param_t* p_policer_param,
                                             uint16 policer_id, sys_qos_policer_t** pp_sys_policer)
{
    uint8  policer_type = 0;
    uint8  dir = 0;
    sys_qos_policer_t* p_sys_policer = NULL;

    if (SYS_QOS_POLICER_TYPE_PORT == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_PORT;
        dir = p_policer_param->dir;
    }
    else if (SYS_QOS_POLICER_TYPE_VLAN == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_VLAN;
        dir = p_policer_param->dir;
    }
    else if (SYS_QOS_POLICER_TYPE_COPP == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_COPP;
    }
    else if (SYS_QOS_POLICER_TYPE_MFP == sys_policer_type)
    {
        policer_type = CTC_QOS_POLICER_TYPE_MFP;
    }
    else
    {
        policer_type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, policer_type,
                                                   dir,
                                                   policer_id,
                                                   &p_sys_policer));
    if (NULL == p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");

        return CTC_E_NOT_EXIST;
    }

    /*select policer level(IPE_POLICING_0/IPE_POLICING_1/EPE_POLICING_0/EPE_POLICING_1) */
    /*installed policer no need update level*/
    if (!p_sys_policer->is_installed)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_level_select(lchip, p_policer_param->dir, sys_policer_type, p_sys_policer));
    }

    /*vlan policer need get policer ptr from vlan module*/
    /*installed policer no need update vlan policer ptr*/
    if ((SYS_QOS_POLICER_TYPE_VLAN == sys_policer_type) && (!p_sys_policer->is_installed))
    {
        p_sys_policer->policer_ptr = p_policer_param->policer_ptr;
    }

    *pp_sys_policer = p_sys_policer;

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_policer_set(uint8 lchip, ctc_qos_policer_t* p_policer_param)
{
    sys_qos_policer_t* p_sys_policer = NULL;
    uint16 policer_id = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_policer_param);

    CTC_ERROR_RETURN(_sys_usw_qos_policer_param_check(lchip, p_policer_param));

    policer_id = _sys_usw_qos_get_policer_id(p_policer_param->type,
                                                    p_policer_param->id.policer_id, p_policer_param->id.gport,
                                                    p_policer_param->id.vlan_id);

    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, p_policer_param->type,
                                                       p_policer_param->dir,
                                                       policer_id,
                                                       &p_sys_policer));
    if (p_policer_param->enable)
    {
       if(p_sys_policer && ((p_policer_param->policer.policer_mode == CTC_QOS_POLICER_MODE_STBM && p_sys_policer->policer_mode != CTC_QOS_POLICER_MODE_STBM)
          || (p_policer_param->policer.policer_mode != CTC_QOS_POLICER_MODE_STBM && p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)))
        {
            return CTC_E_INVALID_PARAM;
        }
        CTC_ERROR_RETURN(_sys_usw_qos_policer_add(lchip, p_policer_param, p_sys_policer));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_qos_policer_remove(lchip, p_policer_param, p_sys_policer));
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_policer_get(uint8 lchip, ctc_qos_policer_t* p_policer_param)
{
    sys_qos_policer_t* p_sys_policer = NULL;
    uint16 policer_id = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_policer_param);

    policer_id = _sys_usw_qos_get_policer_id(p_policer_param->type,
                                                    p_policer_param->id.policer_id, p_policer_param->id.gport,
                                                    p_policer_param->id.vlan_id);
    CTC_ERROR_RETURN(_sys_usw_qos_policer_policer_id_check(lchip, p_policer_param->type,
                                                                 p_policer_param->dir,
                                                                 policer_id));
    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, p_policer_param->type,
                                                       p_policer_param->dir,
                                                       policer_id,
                                                       &p_sys_policer));
    if (NULL == p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");

        return CTC_E_NOT_EXIST;
    }

    CTC_ERROR_RETURN(_sys_usw_qos_get_policer_param(lchip, p_policer_param, p_sys_policer));


    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_index_get(uint8 lchip, uint16 policer_id, uint8 sys_policer_type,
                                                sys_qos_policer_param_t* p_policer_param)
{
    int32  ret = 0;
    uint8  cos_index = 0;
    uint8 is_bucketY = 0;
    uint8 stats_ptr_alloc = 0;
    sys_qos_policer_t* p_sys_policer = NULL;
    sys_qos_policer_profile_t* p_sys_profile_X = NULL;
    sys_qos_policer_profile_t* p_sys_profile_Y = NULL;
    sys_qos_policer_action_t* p_sys_action_profile = NULL;
    sys_qos_policer_copp_profile_t* p_sys_copp_profile = NULL;


    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_id            = %d\n", policer_id);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "sys_policer_type      = %d\n", sys_policer_type);

/* 1. look up policer node */
    CTC_ERROR_RETURN(_sys_usw_qos_policer_get_node(lchip, sys_policer_type, p_policer_param, policer_id, &p_sys_policer));

    if ((p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM) && (p_sys_policer->policer_lvl == SYS_QOS_POLICER_IPE_POLICING_1))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "stbm mode can not support ipe policer level 1\n");
        return CTC_E_INVALID_PARAM;
    }
    cos_index = p_sys_policer->cos_index;

/* 2. add copp/policer profile */
    if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP)
    {
        if (!p_sys_policer->is_installed)
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_alloc_copp_mfp_ptr(lchip, p_sys_policer), ret, error0);
        }
        /*alloc/free policer stats */
        if (p_sys_policer->stats_en
            && (p_sys_policer->stats_ptr[cos_index] == 0))
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_alloc_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]), ret, error1);
            stats_ptr_alloc = 1;
        }
        else if ((!p_sys_policer->stats_en)
            && (p_sys_policer->stats_ptr[cos_index] != 0))
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_free_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]), ret, error1);
        }

        CTC_ERROR_GOTO(_sys_usw_qos_policer_copp_profile_add(lchip, p_sys_policer, &p_sys_copp_profile), ret, error1);
        CTC_ERROR_GOTO(_sys_usw_qos_policer_copp_profile_add_to_asic(lchip, p_sys_policer, p_sys_copp_profile), ret, error3);
        CTC_ERROR_GOTO(_sys_usw_qos_policer_copp_add_to_asic(lchip, p_sys_policer, p_sys_copp_profile), ret, error3);
    }
    else if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_MFP)
    {
        if (!p_sys_policer->is_installed)
        {
           CTC_ERROR_GOTO(_sys_usw_qos_policer_alloc_copp_mfp_ptr(lchip, p_sys_policer), ret, error0);
        }

        CTC_ERROR_GOTO(_sys_usw_qos_policer_mfp_profile_add_to_asic(lchip, p_sys_policer), ret, error1);
    }
    else
    {
        if (!p_sys_policer->is_installed)
        {
            /*alloc policer offset*/
            CTC_ERROR_GOTO(_sys_usw_qos_policer_alloc_offset(lchip, p_sys_policer), ret, error0);
        }
        /*alloc/free policer stats */
        if (p_sys_policer->stats_en
            && (p_sys_policer->stats_ptr[cos_index] == 0))
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_alloc_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]), ret, error1);
            stats_ptr_alloc = 1;
        }
        else if ((!p_sys_policer->stats_en)
            && (p_sys_policer->stats_ptr[cos_index] != 0))
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_free_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]), ret, error1);
        }
        /*add policer profX and profY*/
        CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add(lchip, p_sys_policer, &p_sys_profile_X, &p_sys_profile_Y, &p_sys_action_profile), ret, error2);

        /*write policer profile and table*/
        /*CTC_QOS_POLICER_MODE_STBM means single token bucket, so need config bucket X/Y according to policer_ptr
        even policer_ptr is bucketX, odd policer_ptr is bucketY*/
        if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM)
        {
            is_bucketY = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0);
            if (is_bucketY)
            {
                CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_profileY[cos_index], is_bucketY), ret, error3);
            }
            else
            {
                CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_profileX[cos_index], is_bucketY), ret, error3);
            }
        }
        else
        {
            CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_profileX[cos_index], 0), ret, error3);
            CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_profileY[cos_index], 1), ret, error3);
        }

        /*write policer action profile and table*/
        CTC_ERROR_GOTO(_sys_usw_qos_policer_action_profile_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_action_profile[cos_index]), ret, error3);

        /*write policer ctl and count*/
        CTC_ERROR_GOTO(_sys_usw_qos_policer_add_to_asic(lchip, p_sys_policer, p_sys_policer->p_sys_profileX[cos_index],
                                                             p_sys_policer->p_sys_profileY[cos_index], p_sys_policer->p_sys_action_profile[cos_index]), ret, error3);
    }

    p_sys_policer->is_installed = 1;
    p_policer_param->policer_ptr = p_sys_policer->policer_ptr;
    p_policer_param->is_bwp = p_sys_policer->cos_bitmap ? 1 : 0;
    p_policer_param->level = (p_sys_policer->policer_lvl) % 2;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get: policer_ptr        = %d\n", p_sys_policer->policer_ptr);

    return CTC_E_NONE;

error3:
    if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP)
    {
         _sys_usw_qos_policer_copp_profile_remove(lchip, p_sys_policer);
    }
    else
    {
         _sys_usw_qos_policer_profile_remove(lchip, p_sys_policer, p_sys_policer->p_sys_profileX[cos_index],
                                                p_sys_policer->p_sys_profileX[cos_index], p_sys_policer->p_sys_action_profile[cos_index]);
    }
error2:
    if (stats_ptr_alloc)
    {
        _sys_usw_qos_policer_free_statsptr(lchip, p_sys_policer, &p_sys_policer->stats_ptr[cos_index]);
        p_sys_policer->stats_ptr[cos_index] = 0;
    }
error1:
    if (!p_sys_policer->is_installed)
    {
        if (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP || p_sys_policer->type == CTC_QOS_POLICER_TYPE_MFP)
        {
            _sys_usw_qos_policer_free_copp_mfp_ptr(lchip, p_sys_policer);
        }
        else
        {
            _sys_usw_qos_policer_free_offset(lchip, p_sys_policer);
        }
    }
error0:

    return ret;
}

extern int32
sys_usw_qos_set_policer_update_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    value = enable ? 1 : 0;
    cmd = DRV_IOW(IpePolicing0EngineCtl_t, IpePolicing0EngineCtl_ipePolicing0EngineCtlRefreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(IpePolicing1EngineCtl_t, IpePolicing1EngineCtl_ipePolicing1EngineCtlRefreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(EpePolicing0EngineCtl_t, EpePolicing0EngineCtl_epePolicing0EngineCtlRefreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(EpePolicing1EngineCtl_t, EpePolicing1EngineCtl_epePolicing1EngineCtlRefreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_set_phb(uint8 lchip, ctc_qos_phb_map_t *p_phb_map)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_phb_map);

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_phb_map->map_type = %d\n", p_phb_map->map_type);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_phb_map->priority = %d\n", p_phb_map->priority);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_phb_map->phb      = %d\n", p_phb_map->cos_index);

    if (p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (p_phb_map->map_type == CTC_QOS_PHB_MAP_PRI)
    {
        CTC_MAX_VALUE_CHECK(p_phb_map->priority, 15);
        CTC_MAX_VALUE_CHECK(p_phb_map->cos_index, p_usw_qos_policer_master[lchip]->max_cos_level);
    }
    else
    {
        return CTC_E_NONE;
    }

    value = p_phb_map->cos_index;

    cmd = DRV_IOW(DsIpePolicingPhbOffset_t, DsIpePolicingPhbOffset_phbOffset_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_phb_map->priority, cmd, &value));

    cmd = DRV_IOW(DsEpePolicingPhbOffset_t, DsEpePolicingPhbOffset_phbOffset_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_phb_map->priority, cmd, &value));

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_get_phb(uint8 lchip, ctc_qos_phb_map_t *p_phb_map)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_phb_map);

    if (p_phb_map->map_type == CTC_QOS_PHB_MAP_PRI)
    {
        CTC_MAX_VALUE_CHECK(p_phb_map->priority, 15);
        cmd = DRV_IOR(DsIpePolicingPhbOffset_t, DsIpePolicingPhbOffset_phbOffset_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_phb_map->priority, cmd, &value));
        p_phb_map->cos_index = value;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
int32
sys_usw_qos_set_port_policer_stbm_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(IpePolicingCtl_t, IpePolicingCtl_portPolicerSplitModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(EpePolicingCtl_t, EpePolicingCtl_portPolicerSplitModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_policer_ipg_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(IpePolicingCtl_t, IpePolicingCtl_ipgEnPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(EpePolicingCtl_t, EpePolicingCtl_ipgEnPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_stats_query(uint8 lchip, ctc_qos_policer_stats_t* p_stats_param)
{
    sys_stats_policing_t stats_result;
    sys_qos_policer_t* p_sys_policer = NULL;
    ctc_qos_policer_stats_info_t* p_stats = NULL;
    uint32 stats_ptr = 0;
    uint8 cos_index = 0;
    uint16 policer_id = 0;
    uint8 dir = 0;
    uint8 level = 0;
    uint8 stats_type = 0;
    uint32 stats_ptr_tmp = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_stats_param);

    policer_id = _sys_usw_qos_get_policer_id(p_stats_param->type,
                                                    p_stats_param->id.policer_id, p_stats_param->id.gport,
                                                    p_stats_param->id.vlan_id);

    CTC_ERROR_RETURN(_sys_usw_qos_policer_policer_id_check(lchip, p_stats_param->type,
                                                                 p_stats_param->dir,
                                                                 policer_id));

    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, p_stats_param->type,
                                                       p_stats_param->dir,
                                                       policer_id,
                                                       &p_sys_policer));

    if (!p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_stats_param->hbwp_en)
    {
        CTC_MAX_VALUE_CHECK(p_stats_param->cos_index, p_usw_qos_policer_master[lchip]->max_cos_level);
        cos_index = p_stats_param->cos_index;
    }
    else
    {
        cos_index = 0;
    }

    stats_ptr = p_sys_policer->stats_ptr[cos_index];
    p_stats = &p_stats_param->stats;

    if (0 == stats_ptr)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Stats] Stats is not enabled \n");
        return CTC_E_NOT_READY;
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "stats_ptr:%d\n", stats_ptr);

    dir = (p_sys_policer->policer_lvl) / 2;
    level = (p_sys_policer->policer_lvl) % 2;
    stats_type = ((level==0) ? SYS_STATS_TYPE_POLICER0: SYS_STATS_TYPE_POLICER1);
    if (p_sys_policer->stats_num == 3)  /*split_en=0, stats_mode=0*/
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr, stats_type,&stats_result));
        p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
        p_stats->confirm_bytes = stats_result.policing_confirm_bytes;

        CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr + 1, stats_type,&stats_result));
        p_stats->exceed_pkts  = stats_result.policing_confirm_pkts;
        p_stats->exceed_bytes = stats_result.policing_confirm_bytes;

        CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr + 2, stats_type,&stats_result));
        p_stats->violate_pkts  = stats_result.policing_confirm_pkts;
        p_stats->violate_bytes = stats_result.policing_confirm_bytes;

    }
    else if (p_sys_policer->stats_num == 1)  /*split_en=0, stats_mode=1*/
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr, stats_type,&stats_result));
        p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
        p_stats->confirm_bytes = stats_result.policing_confirm_bytes;
    }
    else if (p_sys_policer->stats_num == 4)  /*split_en=1, stats_mode=0*/
    {
        if (CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)) /*policer Y*/
        {
            stats_ptr_tmp = (p_stats_param->type != CTC_QOS_POLICER_TYPE_COPP)?(stats_ptr + 2):(stats_ptr + 1);
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr_tmp, stats_type,&stats_result));
            p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
            p_stats->confirm_bytes = stats_result.policing_confirm_bytes;

            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr + 3, stats_type,&stats_result));
            p_stats->violate_pkts  = stats_result.policing_confirm_pkts;
            p_stats->violate_bytes = stats_result.policing_confirm_bytes;
        }
        else
        {
            stats_ptr_tmp = (p_stats_param->type != CTC_QOS_POLICER_TYPE_COPP)?(stats_ptr + 1):(stats_ptr + 2);
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr, stats_type,&stats_result));
            p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
            p_stats->confirm_bytes = stats_result.policing_confirm_bytes;

            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr_tmp, stats_type,&stats_result));
            p_stats->violate_pkts  = stats_result.policing_confirm_pkts;
            p_stats->violate_bytes = stats_result.policing_confirm_bytes;
        }

    }
    else if (p_sys_policer->stats_num == 2)  /*split_en=1, stats_mode=1*/
    {
        if (CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))  /*policer Y*/
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr + 1, stats_type,&stats_result));
            p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
            p_stats->confirm_bytes = stats_result.policing_confirm_bytes;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_policing_stats(lchip,  dir, stats_ptr, stats_type,&stats_result));
            p_stats->confirm_pkts  = stats_result.policing_confirm_pkts;
            p_stats->confirm_bytes = stats_result.policing_confirm_bytes;
        }
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get policer stats\n");
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "============================================\n");
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "confirm_packet = %"PRIu64", confirm_bytes = %"PRIu64"\n", p_stats->confirm_pkts, p_stats->confirm_bytes);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "exceed_packet = %"PRIu64", exceed_bytes = %"PRIu64"\n", p_stats->exceed_pkts, p_stats->exceed_bytes);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "violate_packet = %"PRIu64", violate_bytes = %"PRIu64"\n", p_stats->violate_pkts, p_stats->violate_bytes);

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_stats_clear(uint8 lchip, ctc_qos_policer_stats_t* p_stats_param)
{
    sys_qos_policer_t* p_sys_policer = NULL;
    uint32 stats_ptr = 0;
    uint8 cos_index = 0;
    uint16 policer_id = 0;
    uint8 dir = 0;
    uint8 level = 0;
    uint8 stats_type = 0;
    uint32 stats_ptr_tmp = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_stats_param);

    policer_id = _sys_usw_qos_get_policer_id(p_stats_param->type,
                                                    p_stats_param->id.policer_id, p_stats_param->id.gport,
                                                    p_stats_param->id.vlan_id);

    CTC_ERROR_RETURN(_sys_usw_qos_policer_policer_id_check(lchip, p_stats_param->type,
                                                                 p_stats_param->dir,
                                                                 policer_id));

    CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, p_stats_param->type,
                                                       p_stats_param->dir,
                                                       policer_id,
                                                       &p_sys_policer));
    if (!p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Qos] Policer not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_stats_param->hbwp_en)
    {
        CTC_MAX_VALUE_CHECK(p_stats_param->cos_index, p_usw_qos_policer_master[lchip]->max_cos_level);
        cos_index = p_stats_param->cos_index;
    }
    else
    {
        cos_index = 0;
    }

    stats_ptr = p_sys_policer->stats_ptr[cos_index];
    if (0 == stats_ptr)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Stats] Stats is not enabled \n");
        return CTC_E_NOT_READY;
    }
    level = (p_sys_policer->policer_lvl) % 2;
    stats_type = ((level==0) ? SYS_STATS_TYPE_POLICER0: SYS_STATS_TYPE_POLICER1);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "stats_ptr:%d\n", stats_ptr);

    dir = (p_sys_policer->policer_lvl) / 2;
    if (p_sys_policer->stats_num == 3)  /*split_en=0, stats_mode=0*/
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr, stats_type));
        CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr + 1, stats_type));
        CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr + 2, stats_type));
    }
    else if (p_sys_policer->stats_num == 1)  /*split_en=0, stats_mode=1*/
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr, stats_type));
    }
    else if (p_sys_policer->stats_num == 4)  /*split_en=1, stats_mode=0*/
    {
        if (CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0)) /*policer Y*/
        {
            stats_ptr_tmp = (p_stats_param->type != CTC_QOS_POLICER_TYPE_COPP)?(stats_ptr + 2):(stats_ptr + 1);
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr_tmp, stats_type));
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr + 3, stats_type));
        }
        else
        {
            stats_ptr_tmp = (p_stats_param->type != CTC_QOS_POLICER_TYPE_COPP)?(stats_ptr + 1):(stats_ptr + 2);
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr, stats_type));
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr_tmp, stats_type));
        }
    }
    else if (p_sys_policer->stats_num == 2)  /*split_en=1, stats_mode=1*/
    {
        if (CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0))  /*policer Y*/
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr + 1, stats_type));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_clear_policing_stats(lchip,  dir, stats_ptr, stats_type));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_policer_level_select(uint8 lchip, ctc_qos_policer_level_select_t policer_level)
{
    uint32 cmd = 0;
    uint32 port_level = 0;
    uint32 vlan_level = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_level->ingress_vlan_level = %d\n", policer_level.ingress_vlan_level);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_level->ingress_port_level = %d\n", policer_level.ingress_port_level);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_level->egress_vlan_level = %d\n", policer_level.egress_vlan_level);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "policer_level->egress_port_level = %d\n", policer_level.egress_port_level);
    if(policer_level.ingress_port_level != 0)
    {
        port_level = policer_level.ingress_port_level - 1;
        cmd = DRV_IOW(IpePolicingCtl_t, IpePolicingCtl_portPolicerLevelSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_level));
    }
    if(policer_level.ingress_vlan_level != 0)
    {
        vlan_level = policer_level.ingress_vlan_level - 1;
        cmd = DRV_IOW(IpePolicingCtl_t, IpePolicingCtl_vlanPolicerLevelSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &vlan_level));
    }
    if(policer_level.egress_vlan_level != 0)
    {
        vlan_level = policer_level.egress_vlan_level - 1;
        cmd = DRV_IOW(EpePolicingCtl_t, EpePolicingCtl_vlanPolicerLevelSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &vlan_level));
    }
    if(policer_level.egress_port_level != 0)
    {
        port_level = policer_level.egress_port_level - 1;
        cmd = DRV_IOW(EpePolicingCtl_t, EpePolicingCtl_portPolicerLevelSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_level));
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_get_policer_level_select(uint8 lchip, ctc_qos_policer_level_select_t* policer_level)
{
    uint32 cmd = 0;
    uint32 port_level = 0;
    uint32 vlan_level = 0;
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(policer_level);
    cmd = DRV_IOR(IpePolicingCtl_t, IpePolicingCtl_vlanPolicerLevelSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &vlan_level));
    cmd = DRV_IOR(IpePolicingCtl_t, IpePolicingCtl_portPolicerLevelSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_level));
    policer_level->ingress_vlan_level = vlan_level + 1;
    policer_level->ingress_port_level = port_level + 1;

    cmd = DRV_IOR(EpePolicingCtl_t, EpePolicingCtl_vlanPolicerLevelSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &vlan_level));
    cmd = DRV_IOR(EpePolicingCtl_t, EpePolicingCtl_portPolicerLevelSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_level));

    policer_level->egress_vlan_level = vlan_level + 1;
    policer_level->egress_port_level = port_level + 1;

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_dump_status(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value1 = 0;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------Policer--------------------------\n");
    /*cmd = DRV_IOR(PolicingCtl_t, PolicingCtl_updateEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1))
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Global Policer enable", value1);
    cmd = DRV_IOR(PolicingCtl_t, PolicingCtl_statsEnConfirm_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1))
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Global Policer Stats enable", value1);*/
    cmd = DRV_IOR(IpePolicingCtl_t, IpePolicingCtl_ipgEnPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1))
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","IPG enable", value1);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","Max Cos Level", p_usw_qos_policer_master[lchip]->max_cos_level);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Total Policer", MCHIP_CAP(SYS_CAP_QOS_POLICER_POLICER_NUM));
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "--Reserved for port Policer", MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM) * 2);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "--Reserved for vlan Policer", p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num + p_usw_qos_policer_master[lchip]->egress_vlan_policer_num);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","--Port Policer count", p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_PORT_POLICER]);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","--Vlan Policer count", p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_VLAN_POLICER]);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","--Flow Policer count", p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER]);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","Total Policer profile", MCHIP_CAP(SYS_CAP_QOS_POLICER_PROFILE_NUM) - 8);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n","--Used count", p_usw_qos_policer_master[lchip]->p_profile_pool->count);
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
_sys_usw_qos_policer_dump(uint8 type,
                               uint8 dir,
                               uint8 lchip,
                               uint16 start,
                               uint16 end,
                               uint8 detail)
{
    uint32 index = 0;
    uint8 gchip = 0;
    uint16 policer_id = 0;
    sys_qos_policer_t* p_sys_policer = NULL;
    sys_qos_policer_profile_t* p_sys_profileX = NULL;
    sys_qos_policer_profile_t* p_sys_profileY = NULL;
    sys_qos_policer_copp_profile_t* p_sys_copp_profile = NULL;
    uint32 cir = 0;
    uint32 cbs = 0;
    uint32 pir = 0;
    uint32 pbs = 0;
    uint8 loop_end = 0;
    uint16 start_id = 0;
    uint16 end_id = 0;
    uint32 hw_rate = 0;
    uint8 cos_index = 0;
    uint8 first_cos_index = 1;

    LCHIP_CHECK(lchip);

    if (start > end)
    {
        return CTC_E_NONE;
    }

    if (type == CTC_QOS_POLICER_TYPE_PORT)
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(start, lchip, start_id);
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(end, lchip, end_id);
    }
    else
    {
        start_id = start;
        end_id = end;
    }

    if (type == CTC_QOS_POLICER_TYPE_PORT)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s %-7s %-7s", "port", "cos_idx", "enable");
    }
    else if (type == CTC_QOS_POLICER_TYPE_VLAN)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s %-7s %-7s", "vlan-id", "cos_idx", "enable");
    }
    else if ((type == CTC_QOS_POLICER_TYPE_FLOW) || (type == CTC_QOS_POLICER_TYPE_COPP) || (type == CTC_QOS_POLICER_TYPE_MFP))
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s %-7s %-7s", "plc-id", "cos_idx", "enable");
    }

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s %-13s %-12s %-12s %-8s %-7s\n", "cir(kbps/pps)", "pir(kbps/pps)", "cbs(kb/pkt)", "pbs(kb/pkt)",
                             "stats-en", "pps-en");
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------------------\n");

    for (index = start_id; index <= end_id; index++)
    {
        if (type == CTC_QOS_POLICER_TYPE_PORT)
        {
            sys_usw_get_gchip_id(lchip, &gchip);
            policer_id = CTC_MAP_LPORT_TO_GPORT(gchip, index);
        }
        else if ((type == CTC_QOS_POLICER_TYPE_FLOW) || (type == CTC_QOS_POLICER_TYPE_VLAN) || (type == CTC_QOS_POLICER_TYPE_COPP))
        {
            policer_id = index;
        }
        CTC_ERROR_RETURN(_sys_usw_qos_policer_policer_id_check(lchip, type, dir, policer_id));
        CTC_ERROR_RETURN(_sys_usw_qos_policer_lookup(lchip, type,
                                                           dir,
                                                           policer_id,
                                                           &p_sys_policer));

        if (NULL == p_sys_policer || p_sys_policer->entry_size == 0)
        {
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04x %-7s %-6d ", policer_id, "-", 0);
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13d %-13d %-12d %-12d %-8d %-7d\n",
                                     0,0,0,0,0,0);
            continue;
        }

        loop_end = p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en ?1:p_sys_policer->entry_size;
        for (cos_index = 0;  cos_index < loop_end; cos_index++)
        {
            p_sys_profileX = p_sys_policer->p_sys_profileX[cos_index];
            p_sys_profileY = p_sys_policer->p_sys_profileY[cos_index];
            p_sys_copp_profile = p_sys_policer->p_sys_copp_profile;

            if ((NULL == p_sys_profileX) && (NULL == p_sys_profileY) && (NULL == p_sys_copp_profile))
            {
                continue;
            }
            if (p_sys_policer->cos_bitmap)
            {
                if (first_cos_index)
                {
                    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04x %-7d %-6d ", policer_id, cos_index, 1);
                    first_cos_index = 0;
                }
                else
                {
                    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "       %-7d %-6d ", cos_index, 1);
                }
            }
            else
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04x %-7s %-6d ", policer_id, "-", 1);
            }

            if (NULL != p_sys_profileX)
            {
                hw_rate = (p_sys_profileX->rate << 12) | (p_sys_profileX->rateShift << 8) | (p_sys_profileX->rateFrac);
                _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &cir,
                                                               p_usw_qos_policer_master[lchip]->policer_update_freq);
                if (p_sys_policer->is_pps)
                {
                    cbs = (p_sys_profileX->threshold << p_sys_profileX->thresholdShift) / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
                }
                else
                {
                    cbs = (p_sys_profileX->threshold << p_sys_profileX->thresholdShift) / 125;     /*125 = 8/1000*/;
                }
            }

            if (NULL != p_sys_profileY)
            {
                hw_rate = (p_sys_profileY->rate << 12) | (p_sys_profileY->rateShift << 8) | (p_sys_profileY->rateFrac);
                _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &pir,
                                                               p_usw_qos_policer_master[lchip]->policer_update_freq);

                if (p_sys_policer->is_pps)
                {
                    pbs = (p_sys_profileY->threshold << p_sys_profileY->thresholdShift) / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
                }
                else
                {
                    pbs = (p_sys_profileY->threshold << p_sys_profileY->thresholdShift) / 125;     /*125 = 8/1000*/;
                }
            }

            if (NULL != p_sys_copp_profile)
            {
                hw_rate = (p_sys_copp_profile->rate << 12) | (p_sys_copp_profile->rateShift << 8) | (p_sys_copp_profile->rateFrac);
                _sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, p_sys_policer->is_pps, hw_rate, &pir,
                                                               p_usw_qos_policer_master[lchip]->copp_update_freq);

                if (p_sys_policer->is_pps)
                {
                    pbs = (p_sys_copp_profile->threshold << p_sys_copp_profile->thresholdShift) / MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
                }
                else
                {
                    pbs = (p_sys_copp_profile->threshold << p_sys_copp_profile->thresholdShift) / 125;     /*125 = 8/1000*/;
                }
            }


            if (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM && (type == CTC_QOS_POLICER_TYPE_FLOW || type == CTC_QOS_POLICER_TYPE_PORT))
            {
                pir = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0) ? pir : cir;
                cir = 0;
                pbs = CTC_IS_BIT_SET(p_sys_policer->policer_ptr, 0) ? pbs : cbs;
                cbs = 0;
            }

            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13d %-13d %-12d %-12d %-8d %-7d\n", cir,  pir,  cbs, pbs, p_sys_policer->stats_ptr[0]?1:0,
                                    p_sys_policer->is_pps ? 1 : 0);
        }
        first_cos_index = 1;
    }
    if (start_id == end_id && p_sys_policer)
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail information:\n");
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------\n");
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s %-11s %-11s %-7s %-8s %-8s %-8s %-8s %-7s\n","cos_idx","policer_ptr","dir","level","split_en","prof_idX","prof_idY","stats_en","stats_ptr");
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------\n");
        loop_end = p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en ?1:p_sys_policer->entry_size;

        for (index = 0;  index < loop_end; index++)
        {
            p_sys_profileX = p_sys_policer->p_sys_profileX[index];
            p_sys_profileY = p_sys_policer->p_sys_profileY[index];

            if ((NULL == p_sys_profileX) && (NULL == p_sys_profileY) && (NULL == p_sys_copp_profile))
            {
                continue;
            }
            if (p_sys_policer->policer_lvl != SYS_QOS_POLICER_IPE_POLICING_1)
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7d %-11d ", index, (p_sys_policer->policer_ptr >> 1) + index);
            }
            else
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7d %-11d ", index, p_sys_policer->policer_ptr + index);
            }

            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s ", (p_sys_policer->policer_lvl / 2) ? "Egress" : "Ingress");
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7d ", (p_sys_policer->policer_lvl % 2));
            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", (p_sys_policer->policer_mode == CTC_QOS_POLICER_MODE_STBM));

            if (NULL != p_sys_profileX)
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", p_sys_profileX->profile_id);
            }
            else if ((NULL != p_sys_copp_profile) && (!p_sys_copp_profile->is_bucketY))
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", p_sys_copp_profile->profile_id);
            }
            else
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", 0);
            }

            if (NULL != p_sys_profileY)
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", p_sys_profileY->profile_id);
            }
            else if ((NULL != p_sys_copp_profile) && (p_sys_copp_profile->is_bucketY))
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", p_sys_copp_profile->profile_id);
            }
            else
            {
                SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d ", 0);
            }

            SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d %-7d\n", p_sys_policer->stats_ptr[index] ? 1 : 0, p_sys_policer->stats_ptr[index]);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_init_opf(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 max_size = 0;
    uint32 start_offset = 0;
    sys_usw_opf_t opf;
    ctc_qos_policer_level_select_t policer_level;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&policer_level, 0, sizeof(ctc_qos_policer_level_select_t));
    CTC_ERROR_RETURN(sys_usw_qos_get_policer_level_select(lchip, &policer_level));

    /*init policer config opf(IpePolicer0/IpePolicer1/EpePolicer0/EpePolicer1)*/
    start_offset = (((policer_level.ingress_port_level == CTC_QOS_POLICER_LEVEL_0)? MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM):0) +
                     ((policer_level.ingress_vlan_level == CTC_QOS_POLICER_LEVEL_0)?p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num:0)) * 2;
    start_offset = start_offset ? start_offset : 1;
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_policer_config,
                                                                        4, "opf-policer-config"), ret, error0);
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing0Config_t, &max_size);
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_policer_config;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, (max_size - start_offset )* 2),ret, error1);

    start_offset = (((policer_level.ingress_port_level == CTC_QOS_POLICER_LEVEL_1)?MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM):0)+
                     ((policer_level.ingress_vlan_level == CTC_QOS_POLICER_LEVEL_1)?p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num:0)) * 2;
    start_offset = start_offset ? start_offset : 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing1Config_t, &max_size);
    opf.pool_index = 1;
    /* bug 106348 vlan and port level 1 policing use config 2 4 6 8*/
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - start_offset ),ret, error1);

    start_offset = (((policer_level.egress_port_level == CTC_QOS_POLICER_LEVEL_0)? MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM):0) +
                     ((policer_level.egress_vlan_level == CTC_QOS_POLICER_LEVEL_0)?p_usw_qos_policer_master[lchip]->egress_vlan_policer_num:0)) * 2;
    start_offset = start_offset ? start_offset : 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing0Config_t, &max_size);
    opf.pool_index = 2;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, (max_size - start_offset) * 2),ret, error1);

    start_offset = (((policer_level.egress_port_level == CTC_QOS_POLICER_LEVEL_1)? MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM):0)+
                     ((policer_level.egress_vlan_level == CTC_QOS_POLICER_LEVEL_1)?p_usw_qos_policer_master[lchip]->egress_vlan_policer_num:0)) * 2;
    start_offset = start_offset ? start_offset : 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing1Config_t, &max_size);
    opf.pool_index = 3;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, (max_size - start_offset) * 2),ret, error1);

    /*init IpePolicer0 profileX and profileY opf*/
    start_offset = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile,
                                                                        2, "opf-ipe_policer0-profile"), ret, error1);
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing0ProfileX_t, &max_size);
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error2);

    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing0ProfileY_t, &max_size);
    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error2);

    /*init IpePolicer1 profileX and profileY opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile,
                                                                        2, "opf-ipe_policer1-profile"), ret, error2);
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing1ProfileX_t, &max_size);
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error3);

    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing1ProfileY_t, &max_size);
    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error3);

    /*init EpePolicer0 profileX and profileY opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile,
                                                                        2, "opf-epe_policer0-profile"), ret, error3);
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing0ProfileX_t, &max_size);
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error4);

    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing0ProfileY_t, &max_size);
    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error4);

    /*init EpePolicer1 profileX and profileY opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile,
                                                                        2, "opf-epe_policer1-profile"), ret, error4);
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing1ProfileX_t, &max_size);
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error5);

    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing1ProfileY_t, &max_size);
    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error5);

    /*init policer action opf*/
    start_offset = 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicingPolicy_t, &max_size);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_action_profile,
                                                                        2, "opf-action-profile"), ret, error5);

    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_action_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error6);

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error6);

    /*init copp config opf*/
    start_offset = 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsIpeCoPPConfig_t, &max_size);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_copp_config,
                                                                        1, "opf-copp-config"), ret, error6);

    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_copp_config;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size * 2 - 1),
                                                                        ret, error7);

    /*init copp profile opf*/
    start_offset = 1;
    sys_usw_ftm_query_table_entry_num(lchip, DsIpeCoPPProfileX_t, &max_size);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_copp_profile,
                                                                        2, "opf-copp-profile"), ret, error7);

    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_copp_profile;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                         ret, error8);

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                         ret, error8);

    /*init mfp profile opf*/
    start_offset = 1;
    sys_usw_ftm_query_table_entry_num(lchip, MfpProfileCtlTable0_t, &max_size);
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_qos_policer_master[lchip]->opf_type_mfp_profile,
                                                                        2, "opf-mfp-profile"), ret, error8);

    opf.pool_index = 0;
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_mfp_profile;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error9);
    opf.pool_index = 1;
    sys_usw_ftm_query_table_entry_num(lchip, MfpProfileCtlTable1_t, &max_size);
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size - 1),
                                                                        ret, error9);

    return CTC_E_NONE;

error9:
     (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_mfp_profile);
error8:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_profile);
error7:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_config);
error6:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_action_profile);
error5:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile);
error4:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile);
error3:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile);
error2:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile);
error1:
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_policer_config);
error0:
    return ret;
}

STATIC int32
_sys_usw_qos_policer_deinit_opf(uint8 lchip)
{
    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_mfp_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_config);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_action_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile);

    (void)sys_usw_opf_deinit(lchip, p_usw_qos_policer_master[lchip]->opf_type_policer_config);

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_show_spool_status(uint8 lchip)
{
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "No.   name        max_count    used_cout\n");
    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "1     profile    %-13u%-10u\n", p_usw_qos_policer_master[lchip]->p_profile_pool->max_count, \
                                                        p_usw_qos_policer_master[lchip]->p_profile_pool->count);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_traverse_fprintf_pool(void *node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    uint32*                     mode        = (uint32 *)user_date->value2;
    if(*mode == 1)
    {
        sys_qos_policer_profile_t *node_profile = (sys_qos_policer_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-7u%-7u%-7u%-10u%-10u%-7u%-10u%-13u%-10u%-7u\n",*cnt,node_profile->is_bucketY,node_profile->policer_lvl,node_profile->threshold,node_profile->thresholdShift,
                node_profile->rateShift,node_profile->rate,node_profile->rateFrac,node_profile->rateMaxShift,node_profile->rateMax,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == 2)
    {
        sys_qos_policer_action_t *node_profile = (sys_qos_policer_action_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-6u%-7u%-13u%-13u%-10u%-15u%-15u%-15u%-7u\n",*cnt,node_profile->flag,node_profile->dir,node_profile->prio_green,node_profile->prio_yellow,
                node_profile->prio_red,node_profile->discard_green,node_profile->discard_yellow,node_profile->discard_red,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == 3)
    {
        sys_qos_policer_copp_profile_t *node_profile =  (sys_qos_policer_copp_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-5u%-9u%-9u%-7u%-10u%-10u%-7u\n",*cnt,node_profile->is_bucketY,node_profile->threshold,node_profile->thresholdShift,node_profile->rate,
                node_profile->rateShift,node_profile->rateFrac,((ctc_spool_node_t*)node)->ref_cnt);
    }
    (*cnt)++;

    return CTC_E_NONE;
}

#define policer_wb ""

STATIC int32
_sys_usw_qos_policer_restore_copp_mfp_ptr(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    sys_usw_opf_t opf;

    SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ? 0 : p_sys_policer->policer_lvl / 2;
    opf.pool_type  = (p_sys_policer->type == CTC_QOS_POLICER_TYPE_COPP) ?
                      p_usw_qos_policer_master[lchip]->opf_type_copp_config : p_usw_qos_policer_master[lchip]->opf_type_mfp_profile;
    if(p_sys_policer->policer_ptr != 0 && p_sys_policer->is_installed == 1)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_policer->policer_ptr));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_policer_restore_offset(uint8 lchip, sys_qos_policer_t* p_sys_policer)
{
    uint32 offset = p_sys_policer->policer_ptr;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_sys_policer);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_sys_policer->policer_lvl;
    opf.pool_type  = p_usw_qos_policer_master[lchip]->opf_type_policer_config;

    if (p_sys_policer->entry_size && offset != 0 && p_sys_policer->is_installed == 1)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_sys_policer->entry_size, offset));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_policer_wb_sync_func(void* bucket_data, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_qos_policer_t *p_policer = (sys_qos_policer_t *)bucket_data;
    sys_wb_qos_policer_t *p_wb_policer;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    uint8  cos_index = 0;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_policer = (sys_wb_qos_policer_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_policer, 0, sizeof(sys_wb_qos_policer_t));

    switch (p_policer->type)
    {
        case CTC_QOS_POLICER_TYPE_PORT:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_PORT;
            break;
        }
        case CTC_QOS_POLICER_TYPE_FLOW:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_FLOW;
            break;
        }
        case CTC_QOS_POLICER_TYPE_VLAN:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_VLAN;
            break;
        }
        case CTC_QOS_POLICER_TYPE_SERVICE:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_SERVICE;
            break;
        }
        case CTC_QOS_POLICER_TYPE_COPP:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_COPP;
            break;
        }
        case CTC_QOS_POLICER_TYPE_MFP:
        {
            p_wb_policer->type = SYS_WB_QOS_POLICER_TYPE_MFP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer type failed, type = %d.\n", p_policer->type);
            return CTC_E_NONE;
        }
    }

    switch (p_policer->dir)
    {
        case CTC_INGRESS:
        {
            p_wb_policer->dir = SYS_WB_QOS_INGRESS;
            break;
        }
        case CTC_EGRESS:
        {
            p_wb_policer->dir = SYS_WB_QOS_EGRESS;
            break;
        }
        case CTC_BOTH_DIRECTION:
        {
            p_wb_policer->dir = SYS_WB_QOS_BOTH_DIRECTION;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer dir failed, dir = %d.\n", p_policer->dir);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->id = p_policer->id;

    switch (p_policer->level)
    {
        case CTC_QOS_POLICER_LEVEL_NONE:
        {
            p_wb_policer->level = SYS_WB_QOS_POLICER_LEVEL_NONE;
            break;
        }
        case CTC_QOS_POLICER_LEVEL_0:
        {
            p_wb_policer->level = SYS_WB_QOS_POLICER_LEVEL_0;
            break;
        }
        case CTC_QOS_POLICER_LEVEL_1:
        {
            p_wb_policer->level = SYS_WB_QOS_POLICER_LEVEL_1;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer level failed, level = %d.\n", p_policer->level);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->split_en = p_policer->split_en;
    p_wb_policer->hbwp_en = p_policer->hbwp_en;
    p_wb_policer->is_pps = p_policer->is_pps;
    p_wb_policer->cos_bitmap = p_policer->cos_bitmap;
    p_wb_policer->cos_index = p_policer->cos_index;
    p_wb_policer->sf_en = p_policer->sf_en;
    p_wb_policer->cf_total_dis = p_policer->cf_total_dis;

    switch (p_policer->policer_mode)
    {
        case CTC_QOS_POLICER_MODE_STBM:
        {
            p_wb_policer->policer_mode = SYS_WB_QOS_POLICER_MODE_STBM;
            break;
        }
        case CTC_QOS_POLICER_MODE_RFC2697:
        {
            p_wb_policer->policer_mode = SYS_WB_QOS_POLICER_MODE_RFC2697;
            break;
        }
        case CTC_QOS_POLICER_MODE_RFC2698:
        {
            p_wb_policer->policer_mode = SYS_WB_QOS_POLICER_MODE_RFC2698;
            break;
        }
        case CTC_QOS_POLICER_MODE_RFC4115:
        {
            p_wb_policer->policer_mode = SYS_WB_QOS_POLICER_MODE_RFC4115;
            break;
        }
        case CTC_QOS_POLICER_MODE_MEF_BWP:
        {
            p_wb_policer->policer_mode = SYS_WB_QOS_POLICER_MODE_MEF_BWP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer policer_mode failed, policer_mode = %d.\n", p_policer->policer_mode);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->cf_en = p_policer->cf_en;
    p_wb_policer->color_merge_mode = p_policer->color_merge_mode;
    p_wb_policer->is_color_aware = p_policer->is_color_aware;
    p_wb_policer->use_l3_length = p_policer->use_l3_length;

    switch (p_policer->drop_color)
    {
        case CTC_QOS_COLOR_NONE:
        {
            p_wb_policer->drop_color = SYS_WB_QOS_COLOR_NONE;
            break;
        }
        case CTC_QOS_COLOR_RED:
        {
            p_wb_policer->drop_color = SYS_WB_QOS_COLOR_RED;
            break;
        }
        case CTC_QOS_COLOR_YELLOW:
        {
            p_wb_policer->drop_color = SYS_WB_QOS_COLOR_YELLOW;
            break;
        }
        case CTC_QOS_COLOR_GREEN:
        {
            p_wb_policer->drop_color = SYS_WB_QOS_COLOR_GREEN;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer drop_color failed, drop_color = %d.\n", p_policer->drop_color);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->entry_size = p_policer->entry_size;
    p_wb_policer->policer_ptr = p_policer->policer_ptr;
    p_wb_policer->stats_en = p_policer->stats_en;
    p_wb_policer->stats_mode = p_policer->stats_mode;
    p_wb_policer->stats_num = p_policer->stats_num;
    p_wb_policer->stats_useX = p_policer->stats_useX;
    p_wb_policer->stats_useY = p_policer->stats_useY;
    p_wb_policer->cir = p_policer->cir;
    p_wb_policer->cbs = p_policer->cbs;
    p_wb_policer->pir = p_policer->pir;
    p_wb_policer->pbs = p_policer->pbs;
    p_wb_policer->cir_max = p_policer->cir_max;
    p_wb_policer->pir_max = p_policer->pir_max;

    switch (p_policer->policer_lvl)
    {
        case SYS_QOS_POLICER_IPE_POLICING_0:
        {
            p_wb_policer->policer_lvl = SYS_WB_QOS_POLICER_IPE_POLICING_0;
            break;
        }
        case SYS_QOS_POLICER_IPE_POLICING_1:
        {
            p_wb_policer->policer_lvl = SYS_WB_QOS_POLICER_IPE_POLICING_1;
            break;
        }
        case SYS_QOS_POLICER_EPE_POLICING_0:
        {
            p_wb_policer->policer_lvl = SYS_WB_QOS_POLICER_EPE_POLICING_0;
            break;
        }
        case SYS_QOS_POLICER_EPE_POLICING_1:
        {
            p_wb_policer->policer_lvl = SYS_WB_QOS_POLICER_EPE_POLICING_1;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer policer_lvl failed, policer_lvl = %d.\n", p_policer->policer_lvl);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->prio_green = p_policer->prio_green;
    p_wb_policer->prio_yellow = p_policer->prio_yellow;
    p_wb_policer->prio_red = p_policer->prio_red;

    if (p_policer->flag & CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID)
    {
        p_wb_policer->flag |= SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID;
    }

    if (p_policer->flag & CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID)
    {
        p_wb_policer->flag |= SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID;
    }

    if (p_policer->flag & CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID)
    {
        p_wb_policer->flag |= SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID;
    }

    switch (p_policer->sys_policer_type)
    {
        case SYS_QOS_POLICER_TYPE_PORT:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_PORT;
            break;
        }
        case SYS_QOS_POLICER_TYPE_VLAN:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_VLAN;
            break;
        }
        case SYS_QOS_POLICER_TYPE_FLOW:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_FLOW;
            break;
        }
        case SYS_QOS_POLICER_TYPE_MACRO_FLOW:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_MACRO_FLOW;
            break;
        }
        case SYS_QOS_POLICER_TYPE_SERVICE:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_SERVICE;
            break;
        }
        case SYS_QOS_POLICER_TYPE_COPP:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_COPP;
            break;
        }
        case SYS_QOS_POLICER_TYPE_MFP:
        {
            p_wb_policer->sys_policer_type = SYS_WB_QOS_POLICER_SYS_TYPE_MFP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync policer sys_policer_type failed, sys_policer_type = %d.\n", p_policer->sys_policer_type);
            return CTC_E_NONE;
        }
    }

    p_wb_policer->is_installed = p_policer->is_installed;

    for (cos_index = 0; cos_index < 8; cos_index++)
    {
        p_wb_policer->stats_ptr[cos_index] = p_policer->stats_ptr[cos_index];
    }

    for (cos_index = 0; cos_index < 8; cos_index++)
    {
        if (NULL != p_policer->p_sys_profileX[cos_index])
        {
            p_wb_policer->is_sys_profileX_valid[cos_index] = 1;
            p_wb_policer->sys_profileX_profile_id[cos_index] = p_policer->p_sys_profileX[cos_index]->profile_id;
        }
        else
        {
            p_wb_policer->is_sys_profileX_valid[cos_index] = 0;
            p_wb_policer->sys_profileX_profile_id[cos_index] = 0;
        }
    }

    for (cos_index = 0; cos_index < 8; cos_index++)
    {
        if (NULL != p_policer->p_sys_profileY[cos_index])
        {
            p_wb_policer->is_sys_profileY_valid[cos_index] = 1;
            p_wb_policer->sys_profileY_profile_id[cos_index] = p_policer->p_sys_profileY[cos_index]->profile_id;
        }
        else
        {
            p_wb_policer->is_sys_profileY_valid[cos_index] = 0;
            p_wb_policer->sys_profileY_profile_id[cos_index] = 0;
        }
    }

    for (cos_index = 0; cos_index < 8; cos_index++)
    {
        if (NULL != p_policer->p_sys_action_profile[cos_index])
        {
            p_wb_policer->is_sys_action_valid[cos_index] = 1;
            p_wb_policer->sys_action_profile_id[cos_index] = p_policer->p_sys_action_profile[cos_index]->profile_id;
            p_wb_policer->sys_action_dir[cos_index] = p_policer->p_sys_action_profile[cos_index]->dir;
        }
        else
        {
            p_wb_policer->is_sys_action_valid[cos_index] = 0;
            p_wb_policer->sys_action_profile_id[cos_index] = 0;
            p_wb_policer->sys_action_dir[cos_index] = 0;
        }
    }

    if (NULL != p_policer->p_sys_copp_profile)
    {
        p_wb_policer->is_sys_copp_valid = 1;
        p_wb_policer->sys_copp_profile_id = p_policer->p_sys_copp_profile->profile_id;
    }
    else
    {
        p_wb_policer->is_sys_copp_valid = 0;
        p_wb_policer->sys_copp_profile_id = 0;
    }

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_wb_sync(uint8 lchip, ctc_wb_data_t *p_wb_data)
{
    int32 ret = CTC_E_NONE;
    sys_traverse_t user_data;

    user_data.data = p_wb_data;
    user_data.value1 = lchip;

    /* policer */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_policer_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_POLICER);

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_qos_policer_master[lchip]->p_policer_hash,
                                                    _sys_usw_policer_wb_sync_func, (void *)&user_data), ret, done);
    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }

done:
    return ret;
}

int32
sys_usw_qos_policer_wb_restore(uint8 lchip, ctc_wb_query_t *p_wb_query)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    sys_wb_qos_policer_t *p_wb_policer;
    sys_wb_qos_policer_t wb_policer;
    sys_qos_policer_t *p_policer;
    sys_qos_policer_profile_t policer_profile;
    sys_qos_policer_profile_t *p_policer_profile_get;
    sys_qos_policer_action_t policer_action_profile;
    sys_qos_policer_action_t *p_policer_action_profile_get;
    sys_qos_policer_copp_profile_t policer_copp_profile;
    sys_qos_policer_copp_profile_t *p_policer_copp_profile_get;
    uint8 cos_index = 0;
    uint8 is_unknown = 0;

    /* policer */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_policer_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_POLICER);

    sal_memset(&wb_policer, 0, sizeof(sys_wb_qos_policer_t));
    p_wb_policer = &wb_policer;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_policer, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                            (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    p_policer = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_policer_t));
    if (NULL == p_policer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }

    sal_memset(p_policer, 0, sizeof(sys_qos_policer_t));

    switch (p_wb_policer->type)
    {
        case SYS_WB_QOS_POLICER_TYPE_PORT:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_PORT;
            break;
        }
        case SYS_WB_QOS_POLICER_TYPE_FLOW:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_FLOW;
            break;
        }
        case SYS_WB_QOS_POLICER_TYPE_VLAN:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_VLAN;
            break;
        }
        case SYS_WB_QOS_POLICER_TYPE_SERVICE:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_SERVICE;
            break;
        }
        case SYS_WB_QOS_POLICER_TYPE_COPP:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_COPP;
            break;
        }
        case SYS_WB_QOS_POLICER_TYPE_MFP:
        {
            p_policer->type = CTC_QOS_POLICER_TYPE_MFP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer type failed, type = %d.\n", p_wb_policer->type);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    switch (p_wb_policer->dir)
    {
        case SYS_WB_QOS_INGRESS:
        {
            p_policer->dir = CTC_INGRESS;
            break;
        }
        case SYS_WB_QOS_EGRESS:
        {
            p_policer->dir = CTC_EGRESS;
            break;
        }
        case SYS_WB_QOS_BOTH_DIRECTION:
        {
            p_policer->dir = CTC_BOTH_DIRECTION;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer dir failed, dir = %d.\n", p_wb_policer->dir);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->id = p_wb_policer->id;

    switch (p_wb_policer->level)
    {
        case SYS_WB_QOS_POLICER_LEVEL_NONE:
        {
            p_policer->level = CTC_QOS_POLICER_LEVEL_NONE;
            break;
        }
        case SYS_WB_QOS_POLICER_LEVEL_0:
        {
            p_policer->level = CTC_QOS_POLICER_LEVEL_0;
            break;
        }
        case SYS_WB_QOS_POLICER_LEVEL_1:
        {
            p_policer->level = CTC_QOS_POLICER_LEVEL_1;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer level failed, level = %d.\n", p_wb_policer->level);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->split_en = p_wb_policer->split_en;
    p_policer->hbwp_en = p_wb_policer->hbwp_en;
    p_policer->is_pps = p_wb_policer->is_pps;
    p_policer->cos_bitmap = p_wb_policer->cos_bitmap;
    p_policer->cos_index = p_wb_policer->cos_index;
    p_policer->sf_en = p_wb_policer->sf_en;
    p_policer->cf_total_dis = p_wb_policer->cf_total_dis;

    switch (p_wb_policer->policer_mode)
    {
        case SYS_WB_QOS_POLICER_MODE_STBM:
        {
            p_policer->policer_mode = CTC_QOS_POLICER_MODE_STBM;
            break;
        }
        case SYS_WB_QOS_POLICER_MODE_RFC2697:
        {
            p_policer->policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
            break;
        }
        case SYS_WB_QOS_POLICER_MODE_RFC2698:
        {
            p_policer->policer_mode = CTC_QOS_POLICER_MODE_RFC2698;
            break;
        }
        case SYS_WB_QOS_POLICER_MODE_RFC4115:
        {
            p_policer->policer_mode = CTC_QOS_POLICER_MODE_RFC4115;
            break;
        }
        case SYS_WB_QOS_POLICER_MODE_MEF_BWP:
        {
            p_policer->policer_mode = CTC_QOS_POLICER_MODE_MEF_BWP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer policer_mode failed, policer_mode = %d.\n", p_wb_policer->policer_mode);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->cf_en = p_wb_policer->cf_en;
    p_policer->color_merge_mode = p_wb_policer->color_merge_mode;
    p_policer->is_color_aware = p_wb_policer->is_color_aware;
    p_policer->use_l3_length = p_wb_policer->use_l3_length;

    switch (p_wb_policer->drop_color)
    {
        case SYS_WB_QOS_COLOR_NONE:
        {
            p_policer->drop_color = CTC_QOS_COLOR_NONE;
            break;
        }
        case SYS_WB_QOS_COLOR_RED:
        {
            p_policer->drop_color = CTC_QOS_COLOR_RED;
            break;
        }
        case SYS_WB_QOS_COLOR_YELLOW:
        {
            p_policer->drop_color = CTC_QOS_COLOR_YELLOW;
            break;
        }
        case SYS_WB_QOS_COLOR_GREEN:
        {
            p_policer->drop_color = CTC_QOS_COLOR_GREEN;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer drop_color failed, drop_color = %d.\n", p_wb_policer->drop_color);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->entry_size = p_wb_policer->entry_size;
    p_policer->policer_ptr = p_wb_policer->policer_ptr;
    p_policer->stats_en = p_wb_policer->stats_en;
    p_policer->stats_mode = p_wb_policer->stats_mode;
    p_policer->stats_num = p_wb_policer->stats_num;
    p_policer->stats_useX = p_wb_policer->stats_useX;
    p_policer->stats_useY = p_wb_policer->stats_useY;
    p_policer->cir = p_wb_policer->cir;
    p_policer->cbs = p_wb_policer->cbs;
    p_policer->pir = p_wb_policer->pir;
    p_policer->pbs = p_wb_policer->pbs;
    p_policer->cir_max = p_wb_policer->cir_max;
    p_policer->pir_max = p_wb_policer->pir_max;

    switch (p_wb_policer->policer_lvl)
    {
        case SYS_WB_QOS_POLICER_IPE_POLICING_0:
        {
            p_policer->policer_lvl = SYS_QOS_POLICER_IPE_POLICING_0;
            break;
        }
        case SYS_WB_QOS_POLICER_IPE_POLICING_1:
        {
            p_policer->policer_lvl = SYS_QOS_POLICER_IPE_POLICING_1;
            break;
        }
        case SYS_WB_QOS_POLICER_EPE_POLICING_0:
        {
            p_policer->policer_lvl = SYS_QOS_POLICER_EPE_POLICING_0;
            break;
        }
        case SYS_WB_QOS_POLICER_EPE_POLICING_1:
        {
            p_policer->policer_lvl = SYS_QOS_POLICER_EPE_POLICING_1;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer policer_lvl failed, policer_lvl = %d.\n", p_wb_policer->policer_lvl);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->prio_green = p_wb_policer->prio_green;
    p_policer->prio_yellow = p_wb_policer->prio_yellow;
    p_policer->prio_red = p_wb_policer->prio_red;

    if (p_wb_policer->flag & SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID)
    {
        p_policer->flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID;
    }

    if (p_wb_policer->flag & SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID)
    {
        p_policer->flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID;
    }

    if (p_wb_policer->flag & SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID)
    {
        p_policer->flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID;
    }

    p_policer->sys_policer_type = p_wb_policer->sys_policer_type;

    switch (p_wb_policer->sys_policer_type)
    {
        case SYS_WB_QOS_POLICER_SYS_TYPE_PORT:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_PORT;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_VLAN:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_VLAN;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_FLOW:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_FLOW;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_MACRO_FLOW:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_MACRO_FLOW;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_SERVICE:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_SERVICE;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_COPP:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_COPP;
            break;
        }
        case SYS_WB_QOS_POLICER_SYS_TYPE_MFP:
        {
            p_policer->sys_policer_type = SYS_QOS_POLICER_TYPE_MFP;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore policer sys_policer_type failed, sys_policer_type = %d.\n", p_wb_policer->sys_policer_type);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_policer);
        is_unknown = 0;
        continue;
    }

    p_policer->is_installed = p_wb_policer->is_installed;

    for (cos_index = 0; cos_index < MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL); cos_index++)
    {
        p_policer->stats_ptr[cos_index] = p_wb_policer->stats_ptr[cos_index];
    }

    ctc_hash_insert(p_usw_qos_policer_master[lchip]->p_policer_hash, p_policer);

    switch (p_policer->type)
    {
        case CTC_QOS_POLICER_TYPE_PORT:
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_PORT_POLICER]++;
            break;
        }
        case CTC_QOS_POLICER_TYPE_FLOW:
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] =
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_FLOW_POLICER] + p_policer->entry_size;
            break;
        }
        case CTC_QOS_POLICER_TYPE_VLAN:
        {
            p_usw_qos_policer_master[lchip]->policer_count[SYS_POLICER_CNT_VLAN_POLICER]++;
            break;
        }
        default:
        {
            break;
        }
    }

    for (cos_index = 0; cos_index < MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL); cos_index++)
    {
        if (p_wb_policer->is_sys_profileX_valid[cos_index])
        {
            sal_memset(&policer_profile, 0, sizeof(sys_qos_policer_profile_t));
            policer_profile.profile_id = p_wb_policer->sys_profileX_profile_id[cos_index];
            policer_profile.is_bucketY = 0;
            policer_profile.policer_lvl = p_policer->policer_lvl;

            CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_get_from_asic(lchip, p_policer, &policer_profile, 0), ret, done);

            CTC_ERROR_GOTO(ctc_spool_add(p_usw_qos_policer_master[lchip]->p_profile_pool,
                                                &policer_profile, NULL, &p_policer_profile_get), ret, done);

            p_policer->p_sys_profileX[cos_index] = p_policer_profile_get;
        }
    }

    for (cos_index = 0; cos_index < MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL); cos_index++)
    {
        if (p_wb_policer->is_sys_profileY_valid[cos_index])
        {
            sal_memset(&policer_profile, 0, sizeof(sys_qos_policer_profile_t));
            policer_profile.profile_id = p_wb_policer->sys_profileY_profile_id[cos_index];
            policer_profile.is_bucketY = 1;
            policer_profile.policer_lvl = p_policer->policer_lvl;

            CTC_ERROR_GOTO(_sys_usw_qos_policer_profile_get_from_asic(lchip, p_policer, &policer_profile, 1), ret, done);

            CTC_ERROR_GOTO(ctc_spool_add(p_usw_qos_policer_master[lchip]->p_profile_pool,
                                                &policer_profile, NULL, &p_policer_profile_get), ret, done);

            p_policer->p_sys_profileY[cos_index] = p_policer_profile_get;
        }
    }

    for (cos_index = 0; cos_index < MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL); cos_index++)
    {
        if (p_wb_policer->is_sys_action_valid[cos_index])
        {
            sal_memset(&policer_action_profile, 0, sizeof(sys_qos_policer_action_t));
            policer_action_profile.profile_id = p_wb_policer->sys_action_profile_id[cos_index];
            policer_action_profile.dir = p_wb_policer->sys_action_dir[cos_index];

            CTC_ERROR_GOTO(_sys_usw_qos_policer_action_profile_get_from_asic(lchip, p_policer, &policer_action_profile), ret, done);

            CTC_ERROR_GOTO(ctc_spool_add(p_usw_qos_policer_master[lchip]->p_action_profile_pool,
                                         &policer_action_profile, NULL, &p_policer_action_profile_get), ret, done);

            p_policer->p_sys_action_profile[cos_index] = p_policer_action_profile_get;
        }
    }

    if (p_wb_policer->is_sys_copp_valid)
    {
        sal_memset(&policer_copp_profile, 0, sizeof(sys_qos_policer_copp_profile_t));
        policer_copp_profile.profile_id = p_wb_policer->sys_copp_profile_id;
        policer_copp_profile.is_bucketY = CTC_IS_BIT_SET(p_policer->policer_ptr, 0);

        CTC_ERROR_GOTO(_sys_usw_qos_policer_copp_profile_get_from_asic(lchip, p_policer, &policer_copp_profile), ret, done);

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_qos_policer_master[lchip]->p_copp_profile_pool,
                                            &policer_copp_profile, NULL, &p_policer_copp_profile_get), ret, done);

        p_policer->p_sys_copp_profile = p_policer_copp_profile_get;
    }
    if (p_policer->type == CTC_QOS_POLICER_TYPE_COPP || p_policer->type == CTC_QOS_POLICER_TYPE_MFP)
    {
        CTC_ERROR_GOTO(_sys_usw_qos_policer_restore_copp_mfp_ptr(lchip, p_policer), ret, done);
    }
    else if (p_policer->type == CTC_QOS_POLICER_TYPE_FLOW)
    {
        CTC_ERROR_GOTO(_sys_usw_qos_policer_restore_offset(lchip, p_policer), ret, done);
    }

    CTC_WB_QUERY_ENTRY_END(p_wb_query);
done:

    return ret;
}

int32
sys_usw_qos_policer_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 1;
    uint32 mode = 0;
    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# QOS");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "##Policer");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:[%u][%u]\n","port_policer_base",p_usw_qos_policer_master[lchip]->port_policer_base[CTC_INGRESS],p_usw_qos_policer_master[lchip]->port_policer_base[CTC_EGRESS]);
    SYS_DUMP_DB_LOG(p_f, "%-30s:[%u][%u]\n","vlan_policer_base",p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_INGRESS],p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_EGRESS]);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","max_cos_level",p_usw_qos_policer_master[lchip]->max_cos_level);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","flow_plus_service_policer_en",p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_policer_config",p_usw_qos_policer_master[lchip]->opf_type_policer_config);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_ipe_policer0_profile",p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_ipe_policer1_profile",p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_epe_policer0_profile",p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_epe_policer1_profile",p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_copp_config",p_usw_qos_policer_master[lchip]->opf_type_copp_config);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_copp_profile",p_usw_qos_policer_master[lchip]->opf_type_copp_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_mfp_profile",p_usw_qos_policer_master[lchip]->opf_type_mfp_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_action_profile",p_usw_qos_policer_master[lchip]->opf_type_action_profile);
    SYS_DUMP_DB_LOG(p_f, "%-30s:[%u][%u][%u]\n","policer_count",p_usw_qos_policer_master[lchip]->policer_count[0],p_usw_qos_policer_master[lchip]->policer_count[1],p_usw_qos_policer_master[lchip]->policer_count[2]);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","policer_update_freq",p_usw_qos_policer_master[lchip]->policer_update_freq);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","copp_update_freq",p_usw_qos_policer_master[lchip]->copp_update_freq);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","ingress_vlan_policer_num",p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","egress_vlan_policer_num",p_usw_qos_policer_master[lchip]->egress_vlan_policer_num);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","policer_merge_mode",p_usw_qos_policer_master[lchip]->policer_merge_mode);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_policer_config, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer0_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_ipe_policer1_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer0_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_epe_policer1_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_config, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_copp_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_mfp_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_qos_policer_master[lchip]->opf_type_action_profile, p_f);
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","profile_pool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-7s%-7s%-10s%-10s%-7s%-10s%-13s%-10s%-7s\n","Node","is_Y","level","thrd","thrdsft","ratesft","rate","ratefrac","rateMaxsft","rateMax","refcnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    cb_data.value0 = p_f;
    cb_data.value1 = &num_cnt;
    mode = 1;
    cb_data.value2 = &mode;
    ctc_spool_traverse(p_usw_qos_policer_master[lchip]->p_profile_pool, (spool_traversal_fn)_sys_usw_qos_policer_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = 2;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","action_profile_pool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-7s%-13s%-13s%-10s%-15s%-15s%-15s%-7s\n","Node","flag","dir","prio_green","prio_yellow","prio_red","discard_green","discard_yellow","discard_red","refcnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_qos_policer_master[lchip]->p_action_profile_pool, (spool_traversal_fn)_sys_usw_qos_policer_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = 3;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","copp_profile_pool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-7s%-10s%-7s%-10s%-10s%-7s\n","Node","is_Y","thrd","thrdsft","rate","ratesft","ratefrac","refcnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_qos_policer_master[lchip]->p_copp_profile_pool, (spool_traversal_fn)_sys_usw_qos_policer_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------");

    return CTC_E_NONE;
}

int32
sys_usw_qos_policer_init(uint8 lchip,  ctc_qos_global_cfg_t *p_glb_parm)
{
    int32 ret = CTC_E_NONE;
    uint32 flow_policer_num    = 0;
    uint32 divide_factor       = 0;
    uint32 policer_num = 0;
    uint32 tmp_policer_num = 0;
    ctc_spool_t spool;
    uint16 corefreq = 0;

    if (NULL != p_usw_qos_policer_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_qos_policer_master[lchip] = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_policer_master_t));
    if (NULL == p_usw_qos_policer_master[lchip])
    {
        SYS_QOS_POLICER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_memset(p_usw_qos_policer_master[lchip], 0, sizeof(sys_qos_policer_master_t));

    p_usw_qos_policer_master[lchip]->p_policer_hash = ctc_hash_create(
                                                        SYS_QOS_POLICER_HASH_BLOCK_SIZE,
                                                        SYS_QOS_POLICER_HASH_TBL_SIZE,
                                                        (hash_key_fn)_sys_usw_policer_hash_make,
                                                        (hash_cmp_fn)_sys_usw_policer_hash_cmp);
    if (NULL == p_usw_qos_policer_master[lchip]->p_policer_hash)
    {
        ret = CTC_E_NO_MEMORY;
	    goto error1;
    }

    /*create policer spool*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_QOS_POLICER_PROF_HASH_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_POLICER_PROFILE_NUM) - 8;
    spool.user_data_size = sizeof(sys_qos_policer_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_policer_profile_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_policer_profile_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_policer_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_policer_free_profileId;
    p_usw_qos_policer_master[lchip]->p_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_qos_policer_master[lchip]->p_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }

    /*create policer action spool*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_QOS_POLICER_ACTION_PROF_HASH_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_POLICER_ACTION_PROFILE_NUM) - 2;
    spool.user_data_size = sizeof(sys_qos_policer_action_t);
    spool.spool_key = (hash_key_fn)_sys_usw_policer_action_profile_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_policer_action_profile_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_policer_action_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_policer_action_free_profileId;
    p_usw_qos_policer_master[lchip]->p_action_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_qos_policer_master[lchip]->p_action_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error3;
    }

    /*create copp spool*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_QOS_POLICER_COPP_PROF_HASH_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_POLICER_COPP_PROFILE_NUM) - 2;
    spool.user_data_size = sizeof(sys_qos_policer_copp_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_policer_copp_profile_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_policer_copp_profile_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_policer_copp_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_policer_copp_free_profileId;
    p_usw_qos_policer_master[lchip]->p_copp_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_qos_policer_master[lchip]->p_copp_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error4;
    }

    p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num = p_glb_parm->ingress_vlan_policer_num;
    p_usw_qos_policer_master[lchip]->egress_vlan_policer_num = p_glb_parm->egress_vlan_policer_num;
    if(DRV_IS_TSINGMA(lchip))
    {
        p_usw_qos_policer_master[lchip]->policer_merge_mode = p_glb_parm->policer_merge_mode;
    }
    if(p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num >= 512 - SYS_QOS_VLAN_RESERVE_NUM)
    {
        p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num = 0;
    }
    if(p_usw_qos_policer_master[lchip]->egress_vlan_policer_num >= (256 - MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM) *2)/2 - SYS_QOS_VLAN_RESERVE_NUM)
    {
        p_usw_qos_policer_master[lchip]->egress_vlan_policer_num = 0;
    }

    /*frequence = 16K for 400M core_freq can support 8K policer,8K for 200M core_freq.*/
    corefreq = sys_usw_get_core_freq(lchip,0);
    divide_factor = (corefreq == 200) ? 0x4c4a40: 0x2624a0;
    p_usw_qos_policer_master[lchip]->policer_update_freq = (corefreq == 200) ? 8000 : 16000;
    p_usw_qos_policer_master[lchip]->copp_update_freq = 32000; /*32k*/

    p_usw_qos_policer_master[lchip]->port_policer_base[CTC_INGRESS] = 0;
    p_usw_qos_policer_master[lchip]->port_policer_base[CTC_EGRESS]  = 0;

    if(p_glb_parm->policer_level.ingress_port_level == p_glb_parm->policer_level.ingress_vlan_level &&
       p_glb_parm->policer_level.ingress_port_level != 0 && p_glb_parm->policer_level.ingress_vlan_level != 0)
    {
        p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_INGRESS] = MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM)*2;
    }
    if(p_glb_parm->policer_level.egress_port_level == p_glb_parm->policer_level.egress_vlan_level &&
       p_glb_parm->policer_level.egress_port_level != 0 && p_glb_parm->policer_level.egress_vlan_level != 0)
    {
        p_usw_qos_policer_master[lchip]->vlan_policer_base[CTC_EGRESS]  = MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM)*2;
    }
    CTC_ERROR_GOTO(_sys_usw_qos_policer_init_reg(lchip, divide_factor), ret, error5);
    CTC_ERROR_GOTO(sys_usw_qos_set_policer_level_select(lchip, p_glb_parm->policer_level),ret,error5);
    CTC_ERROR_GOTO(_sys_usw_qos_policer_init_opf(lchip), ret, error5);

    if (p_glb_parm->max_cos_level == 0)
    {
        p_glb_parm->max_cos_level = MCHIP_CAP(SYS_CAP_QOS_POLICER_MAX_COS_LEVEL) - 1;
    }
    p_usw_qos_policer_master[lchip]->max_cos_level = p_glb_parm->max_cos_level;

    p_usw_qos_policer_master[lchip]->flow_plus_service_policer_en = (p_glb_parm->max_cos_level == 1); /* service policer + flow policer mode*/


    CTC_ERROR_GOTO(_sys_usw_qos_policer_copp_init(lchip), ret, error6);
    CTC_ERROR_GOTO(_sys_usw_qos_policer_mfp_init(lchip), ret, error6);
    CTC_ERROR_GOTO(_sys_usw_qos_policer_init_policer_mode(lchip), ret, error6);
    CTC_ERROR_GOTO(_sys_usw_qos_policer_init_share_mode(lchip), ret, error6);
    /* set chip_capability */
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing0Config_t, &tmp_policer_num);
    policer_num += tmp_policer_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsIpePolicing1Config_t, &tmp_policer_num);
    policer_num += tmp_policer_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing0Config_t, &tmp_policer_num);
    policer_num += tmp_policer_num;
    sys_usw_ftm_query_table_entry_num(lchip, DsEpePolicing1Config_t, &tmp_policer_num);
    policer_num += tmp_policer_num;

    MCHIP_CAP(SYS_CAP_SPEC_TOTAL_POLICER_NUM) = policer_num;
    flow_policer_num = policer_num - 4 * MCHIP_CAP(SYS_CAP_QOS_PORT_POLICER_NUM) -
        2*(p_usw_qos_policer_master[lchip]->egress_vlan_policer_num + p_usw_qos_policer_master[lchip]->ingress_vlan_policer_num);
    MCHIP_CAP(SYS_CAP_SPEC_POLICER_NUM) = flow_policer_num;


    return CTC_E_NONE;

error6:
    (void)_sys_usw_qos_policer_deinit_opf(lchip);
error5:
    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_copp_profile_pool);
error4:
    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_action_profile_pool);
error3:
    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_profile_pool);
error2:
    ctc_hash_traverse(p_usw_qos_policer_master[lchip]->p_policer_hash,
                                    (hash_traversal_fn)_sys_usw_policer_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_qos_policer_master[lchip]->p_policer_hash);
error1:
    mem_free(p_usw_qos_policer_master[lchip]);
error0:
    return ret;
}

int32
sys_usw_qos_policer_deinit(uint8 lchip)
{
    if (NULL == p_usw_qos_policer_master[lchip])
    {
        return CTC_E_NONE;
    }

    (void)_sys_usw_qos_policer_deinit_opf(lchip);

    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_copp_profile_pool);

    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_action_profile_pool);

    ctc_spool_free(p_usw_qos_policer_master[lchip]->p_profile_pool);

    ctc_hash_traverse(p_usw_qos_policer_master[lchip]->p_policer_hash,
                                    (hash_traversal_fn)_sys_usw_policer_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_qos_policer_master[lchip]->p_policer_hash);

    mem_free(p_usw_qos_policer_master[lchip]);

    return CTC_E_NONE;
}


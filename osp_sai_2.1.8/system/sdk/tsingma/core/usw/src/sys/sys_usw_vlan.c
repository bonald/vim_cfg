/**
@file sys_usw_vlan.c

@date 2011 - 11 - 11

@version v2.0


This file contains all sys APIs of vlan
*/

/***************************************************************
*
* Header Files
*
***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_spool.h"
#include "ctc_l2.h"

#include "sys_usw_ftm.h"
#include "sys_usw_vlan.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_common.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_packet.h"

#include "drv_api.h"

/***************************************************************
*
*  Defines and Macros
*
***************************************************************/
#define SYS_MIN_PROFILE_ID             0
#define ENTRY_NUM_VLAN                   4096
#define ENTRY_NUM_VLAN_STATUS           2048
#define ENTRY_NUM_VLAN_PROFILE           512
#define IPE_CTL_BITS_NUM               6
#define SYS_VLAN_BLOCK_NUM               4
#define SYS_VLAN_PROFILE_BLOCK_NUM      4
#define MAX_NETWORK_PORT_CHANNEL  64
#define SYS_VLAN_TEMPLATE_NUM     16
#define SYS_VLAN_RESERVE_NUM     4
#define SYS_VLAN_VSI_PARAM_MAX   8
#define SYS_VLAN_PTP_CLOCK_ID_MAX 3
/**< [TM] Append */
#define SYS_VLAN_TAG_BITMAP_NUM     8

struct sys_sw_vlan_profile_acl_s
{
    uint8 acl_lookup_type;
    uint8 acl_label;
    uint8 acl_global_port_type;
    uint8 acl_pi_vlan;
};
typedef struct sys_sw_vlan_profile_acl_s sys_sw_vlan_profile_acl_t;

struct sys_sw_igs_vlan_profile_s
{
    uint8 vlan_storm_ctl_en;
    uint8 vlan_storm_ctl_ptr;
    uint8 fip_exception_type;
    uint8 force_ipv4_lookup;
    uint8 force_ipv6_lookup;
    uint8 ingress_vlan_span_en;
    uint8 ingress_vlan_span_id;
    uint8 ptp_index;

    sys_sw_vlan_profile_acl_t vlan_profile_igs_acl[8];
};
typedef struct sys_sw_igs_vlan_profile_s sys_sw_igs_vlan_profile_t;

struct sys_sw_egs_vlan_profile_s
{
    uint8 egress_vlan_span_en;
    uint8 egress_vlan_span_id;
    uint8 rsv[2];
    sys_sw_vlan_profile_acl_t vlan_profile_egs_acl[3];
};
typedef struct sys_sw_egs_vlan_profile_s sys_sw_egs_vlan_profile_t;

struct sys_sw_vlan_profile_s
{
    uint8 dir;
    uint8 rsv[3];
    union
    {
        DsSrcVlanProfile_m src_vlan_profile;
        DsDestVlanProfile_m dest_vlan_profile;
    }vp;
};
typedef struct sys_sw_vlan_profile_s sys_sw_vlan_profile_t;

struct sys_vlan_profile_s
{
    uint8 lchip ;
    uint16 profile_id;
    uint8 rsv;
    sys_sw_vlan_profile_t sw_vlan_profile;

};
typedef struct sys_vlan_profile_s sys_vlan_profile_t;

/**
 @brief define overlay tunnel vn_id and fid mapping struct
*/
struct sys_vlan_overlay_mapping_key_s
{
    uint32 vn_id;

    uint16 fid;
};
typedef struct sys_vlan_overlay_mapping_key_s sys_vlan_overlay_mapping_key_t;

struct sys_vlan_master_s
{
    ctc_spool_t*        vprofile_spool;
    uint32              vlan_bitmap[128];
    uint32              vlan_def_bitmap[128];
    ctc_vlanptr_mode_t  vlan_mode;
    uint8               vni_mapping_mode;
    uint8               vlan_port_bitmap_mode;
    uint8              sys_vlan_def_mode;
    ctc_hash_t* vni_mapping_hash; /* used to record vn_id and fid map */
    uint8              src_opf_type;
    uint8              dest_opf_type;
    uint16             ingress_vlan_policer_num;
    uint16             egress_vlan_policer_num;
    sal_mutex_t*       mutex;
};
typedef struct sys_vlan_master_s sys_vlan_master_t;

#define SYS_VLAN_INIT_XC_DS_VLAN(p_ds_vlan) \
    { \
    SetDsVlan(V, transmitDisable_f, p_ds_vlan, 0);\
    SetDsVlan(V, receiveDisable_f, p_ds_vlan, 0);\
    }

/*Modified for TM*/
#define SYS_VLAN_FILTER_PORT_CHECK(lport) \
    do \
    { \
        if (lport >= MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)) \
        { \
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan filter port\n");\
			return CTC_E_INVALID_CONFIG;\
 \
        } \
    } \
    while (0)

#define SYS_VLAN_CREATE_CHECK(vlan_ptr) \
    do \
    { \
        if (!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F))){ \
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan not create \n");\
			return CTC_E_NOT_EXIST;\
 } \
    } \
    while (0)

#define SYS_VLAN_PROFILE_ID_VALID_CHECK(profile_id) \
    do \
    { \
        if ((profile_id) >= MCHIP_CAP(SYS_CAP_VLAN_PROFILE_ID) || (profile_id) < SYS_MIN_PROFILE_ID){ \
            return CTC_E_VLAN_INVALID_PROFILE_ID; } \
    } \
    while (0)

#define SYS_VLAN_INIT_CHECK() \
    do \
    { \
        LCHIP_CHECK(lchip); \
        if (p_usw_vlan_master[lchip] == NULL){ \
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } \
    while (0)

#define SYS_VLAN_DBG_OUT(level, FMT, ...) \
    do \
    { \
        CTC_DEBUG_OUT(vlan, vlan, VLAN_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_VLAN_NO_MEMORY_RETURN(op) \
    do \
    { \
        if (NULL == (op)) \
        { \
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");\
			return CTC_E_NO_MEMORY;\
 \
        } \
    } \
    while (0)

#define REF_COUNT(data)   ctc_spool_get_refcnt(p_usw_vlan_master[lchip]->vprofile_spool, data)
#define VLAN_PROFILE_SIZE_PER_BUCKET 16
#define SYS_VLAN_OVERLAY_IHASH_MASK         0xFFF
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

#define SETEGRESSVSI(FID,VN_ID,DS_DATA)                     \
{                                                           \
    if (0 == (FID % 4) )                                    \
    {                                                       \
        SetDsEgressVsi(V, array_0_vni_f, DS_DATA, VN_ID);   \
    }                                                       \
    else if (1 == (FID % 4))                                \
    {                                                       \
        SetDsEgressVsi(V, array_1_vni_f, DS_DATA, VN_ID);   \
    }                                                       \
    else if (2 == (FID % 4))                                \
    {                                                       \
        SetDsEgressVsi(V, array_2_vni_f, DS_DATA, VN_ID);   \
    }                                                       \
    else                                                    \
    {                                                       \
        SetDsEgressVsi(V, array_3_vni_f, DS_DATA, VN_ID);   \
    }                                                       \
}

/*
*
*    byte_offset = vlan_id >> 3 ;
*    bit_offset = vlan_id & 0x7;
*    p_usw_vlan_master->vlan_bitmap[byte_offset] |= 1 >> bit_offset;
*/

static sys_vlan_master_t* p_usw_vlan_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_VLAN_CREATE_LOCK(lchip) \
    do \
    { \
        sal_mutex_create(&p_usw_vlan_master[lchip]->mutex); \
        if (NULL == p_usw_vlan_master[lchip]->mutex) \
        { \
            return CTC_E_NO_MEMORY; \
        } \
    }while (0)

#define SYS_VLAN_LOCK(lchip)\
    sal_mutex_lock(p_usw_vlan_master[lchip]->mutex)

#define SYS_VLAN_UNLOCK(lchip)\
    sal_mutex_unlock(p_usw_vlan_master[lchip]->mutex)

#define CTC_ERROR_RETURN_VLAN_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_vlan_master[lchip]->mutex); \
            return rv; \
        } \
    }while (0)
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

/**
 *
 *  hw table
 */
 extern int32
sys_usw_qos_policer_remove_vlan_policer(uint8 lchip, uint8 dir, uint16 vlan_id);
 STATIC int32
_sys_usw_vlan_profile_index_build(sys_vlan_profile_t* p_vlan_profile,uint8* p_lchip)
{
    sys_usw_opf_t  opf;
    uint32 value_32 = 0;
    int32 ret = 0;
    uint32 entry_num_src_vlan_profile = 0;
    uint32 entry_num_dest_vlan_profile = 0;
	uint8 lchip = 0;

    lchip = *p_lchip;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsSrcVlanProfile_t, &entry_num_src_vlan_profile));
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsDestVlanProfile_t, &entry_num_dest_vlan_profile));
        if (p_vlan_profile->sw_vlan_profile.dir == CTC_INGRESS )
        {
            opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
            opf.reverse    = (p_vlan_profile->profile_id >= entry_num_src_vlan_profile - SYS_VLAN_RESERVE_NUM)? 1 : 0;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_vlan_profile->profile_id);
        }
        else
        {
            opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
            opf.reverse    = (p_vlan_profile->profile_id >= entry_num_dest_vlan_profile - SYS_VLAN_RESERVE_NUM)? 1 : 0;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_vlan_profile->profile_id);
        }
    }
    else
    {
        if (p_vlan_profile->sw_vlan_profile.dir == CTC_INGRESS )
        {
            opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
            opf.reverse    = 0;
            ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &value_32);
            if (ret < 0 && (GetDsSrcVlanProfile(V, ingressVlanSpanEn_f, &p_vlan_profile->sw_vlan_profile.vp.src_vlan_profile) == 1))
            {
                opf.reverse =  1;
                CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &value_32));
            }
        }
        else
        {
            opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
            opf.reverse    = 0;
            ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &value_32);
            if (ret < 0 && (GetDsDestVlanProfile(V, egressVlanSpanEn_f, &p_vlan_profile->sw_vlan_profile.vp.dest_vlan_profile) == 1))
            {
                opf.reverse =  1;
                CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &value_32));
            }
        }

        p_vlan_profile->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}
STATIC int32 _sys_usw_vlan_alloc_policer_ptr_from_position(uint8 lchip, uint8 dir, uint32 offset)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 1;
    if (dir == CTC_INGRESS )
    {
        opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, offset));
    }
    else
    {
        opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, offset));
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_vlan_alloc_policer_ptr(uint8 lchip, sys_vlan_profile_t* p_vlan_profile)
{
    uint32 offset = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_vlan_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if (p_vlan_profile->sw_vlan_profile.dir == CTC_INGRESS )
    {
        opf.pool_index = 1;
        opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));

        p_vlan_profile->profile_id = offset;
    }
    else
    {
        opf.pool_index = 1;
        opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));

        p_vlan_profile->profile_id = offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_free_policer_ptr(uint8 lchip, sys_vlan_profile_t* p_vlan_profile)
{
    uint32 offset = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_vlan_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if (p_vlan_profile->sw_vlan_profile.dir == CTC_INGRESS )
    {
        opf.pool_index = 1;
        opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, p_vlan_profile->profile_id));

        p_vlan_profile->profile_id = offset;
    }
    else
    {
        opf.pool_index = 1;
        opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, p_vlan_profile->profile_id));

        p_vlan_profile->profile_id = offset;
    }

    return CTC_E_NONE;
}
STATIC uint32
_sys_usw_vlan_overlay_hash_make(sys_vlan_overlay_mapping_key_t* p_mapping_info)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_mapping_info->vn_id;
    MIX(a, b, c);

    return (c & SYS_VLAN_OVERLAY_IHASH_MASK);
}

STATIC bool
_sys_usw_vlan_overlay_hash_cmp(sys_vlan_overlay_mapping_key_t* p_mapping_info1, sys_vlan_overlay_mapping_key_t* p_mapping_info)
{
    if (p_mapping_info1->vn_id != p_mapping_info->vn_id)
    {
        return FALSE;
    }

    return TRUE;
}

STATIC int32
_sys_usw_vlan_overlay_set_fid_hw(uint8 lchip, uint16 fid, uint32 vn_id)
{
    uint16 index = 0;
    ds_t ds_data;
    uint32 cmd = 0;

    sal_memset(&ds_data, 0, sizeof(ds_data));
    index = fid >> 2;
    cmd = DRV_IOR(DsEgressVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN( DRV_IOCTL(lchip, index, cmd, &ds_data));
    SETEGRESSVSI(fid, vn_id, ds_data);
    cmd = DRV_IOW(DsEgressVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_data));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_set_default_vlan(uint8 lchip, DsSrcVlan_m* p_ds_src_vlan,
                                                        DsDestVlan_m* p_ds_dest_vlan)
{
    sal_memset(p_ds_src_vlan, 0 ,sizeof(DsSrcVlan_m));
    SetDsSrcVlan(V, receiveDisable_f, p_ds_src_vlan, 1);
    SetDsSrcVlan(V, interfaceId_f, p_ds_src_vlan, 0);
    SetDsSrcVlan(V, arpExceptionType_f, p_ds_src_vlan, CTC_EXCP_NORMAL_FWD);
 /*-v2.2.0    SetDsSrcVlan(V, dhcpExceptionType_f, p_ds_src_vlan, CTC_EXCP_NORMAL_FWD);*/
    SetDsSrcVlan(V, dhcpV4ExceptionType_f, p_ds_src_vlan, CTC_EXCP_NORMAL_FWD);
    SetDsSrcVlan(V, dhcpV6ExceptionType_f, p_ds_src_vlan, CTC_EXCP_NORMAL_FWD);

    sal_memset(p_ds_dest_vlan, 0 ,sizeof(DsDestVlan_m));
    SetDsDestVlan(V, transmitDisable_f, p_ds_dest_vlan, 1);
    SetDsDestVlan(V, interfaceId_f, p_ds_dest_vlan, 0);

    return CTC_E_NONE;
}

/**
  @brief judge whether a port is member of vlan
  */
STATIC int32
_sys_usw_vlan_is_member_port(uint8 lchip, uint8 field_id, uint32 index)
{
    DsSrcVlanStatus_m ds_src_vlan_status;
    DsDestVlanStatus_m ds_dest_vlan_status;
    uint32 cmd = 0;
    uint32 src_vlan_id_valid = 0;
    uint32 dest_vlan_id_valid = 0;

    sal_memset(&ds_src_vlan_status, 0, sizeof(DsSrcVlanStatus_m));
    sal_memset(&ds_dest_vlan_status, 0, sizeof(DsDestVlanStatus_m));

    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan_status));
    DRV_GET_FIELD_A(lchip, DsSrcVlanStatus_t, field_id, &ds_src_vlan_status, &src_vlan_id_valid);

    cmd = DRV_IOR(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan_status));
    DRV_GET_FIELD_A(lchip, DsDestVlanStatus_t, field_id, &ds_dest_vlan_status, &dest_vlan_id_valid);

    return (src_vlan_id_valid && dest_vlan_id_valid);
}

STATIC uint32
_sys_usw_vlan_hash_make(sys_vlan_profile_t* p_vlan_profile)
{
    uint32 size = 0;
    uint8  * k = NULL;

    CTC_PTR_VALID_CHECK(p_vlan_profile);

    size = sizeof(sys_sw_vlan_profile_t);
    k    = (uint8 *) (&(p_vlan_profile->sw_vlan_profile));

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_vlan_hash_compare(sys_vlan_profile_t* p_bucket, sys_vlan_profile_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (!sal_memcmp(&p_bucket->sw_vlan_profile, &p_new->sw_vlan_profile, sizeof(sys_sw_vlan_profile_t))) /*xscl is same size*/
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_vlan_profile_index_free(sys_vlan_profile_t* p_vlan_profile,uint8* lchip)
{
    sys_usw_opf_t  opf;
    uint16 index = p_vlan_profile->profile_id;
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if(CTC_INGRESS == p_vlan_profile->sw_vlan_profile.dir)
    {
        opf.pool_type = p_usw_vlan_master[*lchip]->src_opf_type;
    }
    else
    {
        opf.pool_type = p_usw_vlan_master[*lchip]->dest_opf_type;
    }
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, (uint32)index));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_get_vlan_ptr(uint8 lchip, uint16 vlan_id, uint16* vlan_ptr)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 value = 0;
    uint8 field_id = 0;

    CTC_PTR_VALID_CHECK(vlan_ptr);

    field_id = (vlan_id & 0x1)? DsVlan2Ptr_array_1_vlanPtr_f : DsVlan2Ptr_array_0_vlanPtr_f;
    cmd = DRV_IOR(DsVlan2Ptr_t, field_id);
    index = (vlan_id >> 1) & 0x7FF;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    *vlan_ptr = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_cmp_node_data(void* node_data, void* user_data)
{
    sys_vlan_overlay_mapping_key_t* p_vlan_overlay = (sys_vlan_overlay_mapping_key_t*) node_data;


    if (p_vlan_overlay && (0 == p_vlan_overlay->vn_id) && (*(uint16*)user_data == p_vlan_overlay->fid))
    {
        return CTC_E_EXIST;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_overlay_get_vn_id(uint8 lchip, uint16 fid, uint32* p_vn_id)
{
    uint32 cmd = 0;
    uint16 index = 0;
    uint8 field_index = 0;
    int32 ret = 0;
    uint16 new_fid = fid;

    index = fid >> 2;
    field_index = fid % 4;
    cmd = DRV_IOR(DsEgressVsi_t, DsEgressVsi_array_0_vni_f + field_index);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_vn_id));

    if (*p_vn_id == 0)
    {
        ret = ctc_hash_traverse(p_usw_vlan_master[lchip]->vni_mapping_hash, (hash_traversal_fn)_sys_usw_vlan_cmp_node_data, (void*)(&new_fid));
        if (ret != CTC_E_EXIST)
        {
            return CTC_E_NOT_EXIST;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_overlay_add_fid (uint8 lchip,  uint32 vn_id, uint16 fid)
{
    sys_vlan_overlay_mapping_key_t* p_mapping_info = NULL;
    sys_vlan_overlay_mapping_key_t mapping_info;
    int32 ret = CTC_E_NONE;
    uint16 old_fid = 0;
    uint32 old_vni_id = 0;

    SYS_VLAN_INIT_CHECK();
    if ( vn_id > 0xFFFFFF )
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned\n");
		return CTC_E_BADID;
    }
    if (( fid > 0x3FFF )||(0 == fid))
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual FID is out of range\n");
		return CTC_E_BADID;
    }

    ret = _sys_usw_vlan_overlay_get_vn_id(lchip, fid, &old_vni_id);  /* can not use error return */
    if (((old_vni_id == 0) && (ret == 0)) || (old_vni_id && old_vni_id != vn_id))
    {
        /* same FID and want to update vn_id */
            return CTC_E_EXIST;
    }

    sal_memset(&mapping_info, 0, sizeof(sys_vlan_overlay_mapping_key_t));
    mapping_info.vn_id = vn_id;
    p_mapping_info = ctc_hash_lookup(p_usw_vlan_master[lchip]->vni_mapping_hash, &mapping_info);

    if ( NULL == p_mapping_info )
    {
        p_mapping_info =  mem_malloc(MEM_VLAN_MODULE,  sizeof(sys_vlan_overlay_mapping_key_t));
        if (NULL == p_mapping_info)
        {
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;

        }
        sal_memset(p_mapping_info, 0, sizeof(sys_vlan_overlay_mapping_key_t));
        p_mapping_info->vn_id = vn_id;
        p_mapping_info->fid = fid;
        /* keep setting sw table after set hw table */
        ret = _sys_usw_vlan_overlay_set_fid_hw(lchip, fid, vn_id);
        if (ret < 0)
        {
            mem_free(p_mapping_info);
            return ret;
        }
        ctc_hash_insert(p_usw_vlan_master[lchip]->vni_mapping_hash, p_mapping_info);
    }
    else
    {
        /* p_mapping_info is true means 1. update fid 2. these two configed are the same config */
        if (fid != p_mapping_info->fid)
        {
            /*same VNI and update FID*/
            old_fid = p_mapping_info->fid;
            CTC_ERROR_RETURN(_sys_usw_vlan_overlay_set_fid_hw(lchip, old_fid, 0));
            CTC_ERROR_RETURN(_sys_usw_vlan_overlay_set_fid_hw(lchip, fid, vn_id));
            p_mapping_info->fid = fid;
        }
    }
    return CTC_E_NONE;
}



STATIC int32
_sys_usw_vlan_overlay_remove_fid (uint8 lchip,  uint32 vn_id)
{
    sys_vlan_overlay_mapping_key_t* p_mapping_info = NULL;
    sys_vlan_overlay_mapping_key_t mapping_info;

    SYS_VLAN_INIT_CHECK();
    if ( vn_id > 0xFFFFFF )
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned\n");
		return CTC_E_BADID;
    }

    sal_memset(&mapping_info, 0, sizeof(sys_vlan_overlay_mapping_key_t));
    mapping_info.vn_id = vn_id;
    p_mapping_info = ctc_hash_lookup(p_usw_vlan_master[lchip]->vni_mapping_hash, &mapping_info);
    if ( NULL != p_mapping_info )
    {
         CTC_ERROR_RETURN( _sys_usw_vlan_overlay_set_fid_hw(lchip, p_mapping_info->fid, 0));
         p_mapping_info->vn_id = vn_id;
         ctc_hash_remove(p_usw_vlan_master[lchip]->vni_mapping_hash, p_mapping_info);
         mem_free(p_mapping_info);
    }
    else
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnets FID has not assigned\n");
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}


int32
_sys_usw_vlan_overlay_get_fid (uint8 lchip,  uint32 vn_id, uint16* p_fid )
{
    sys_vlan_overlay_mapping_key_t* p_mapping_info = NULL;
    sys_vlan_overlay_mapping_key_t mapping_info;

    SYS_VLAN_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_fid);
    if ( vn_id > 0xFFFFFF )
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned\n");
		return CTC_E_BADID;
    }

    if (!p_usw_vlan_master[lchip]->vni_mapping_mode)
    {
        sal_memset(&mapping_info, 0, sizeof(mapping_info));
        mapping_info.vn_id = vn_id;
        p_mapping_info = ctc_hash_lookup(p_usw_vlan_master[lchip]->vni_mapping_hash, &mapping_info);

        if (NULL == p_mapping_info)
        {
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnets FID has not assigned\n");
            return CTC_E_NOT_EXIST;
        }

        *p_fid = p_mapping_info->fid;
    }
    else
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}


int32
sys_usw_vlan_set_fid_mapping_vdev_vlan(uint8 lchip, uint16 fid, uint16 vdev_vlan)
{
    uint16 index = 0;
    ds_t ds_data;
    uint32 cmd = 0;
    uint32 vni_id = 0;
    SYS_VLAN_INIT_CHECK();

    sal_memset(&ds_data, 0, sizeof(ds_data));
    index = fid >> 2;
    cmd = DRV_IOR(DsEgressVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN( DRV_IOCTL(lchip, index, cmd, &ds_data));
    if (vdev_vlan)
    {
        vni_id = vdev_vlan | (1<<16);
    }
    SETEGRESSVSI(fid, vni_id, ds_data);
    cmd = DRV_IOW(DsEgressVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_data));

    return CTC_E_NONE;
}

#define __VLAN_PROFILE__

STATIC int32
_sys_usw_vlan_profile_dump(ctc_spool_node_t* spool_node, void* data)
{
    sys_vlan_profile_t  *p_vlan_profile = spool_node->data;
    uint8 lchip = p_vlan_profile->lchip;
    uint32 dir  = *(uint32*)data;
    LCHIP_CHECK(lchip);
    if(dir != p_vlan_profile->sw_vlan_profile.dir )
    {
       return CTC_E_NONE;
    }
    if (0xFFFFFFFF == spool_node->ref_cnt)
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12u%s\n", p_vlan_profile->profile_id, "-");
    }
    else
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12u%u\n", p_vlan_profile->profile_id, spool_node->ref_cnt);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_vni_mapping_dump(sys_vlan_overlay_mapping_key_t* p_vni_mapping, void* data)
{

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12u%-12u\n", p_vni_mapping->vn_id, p_vni_mapping->fid);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_set_vlan_profile_add_to_asic(uint8 dir, uint8 lchip, uint16 vlan_id, sys_vlan_profile_t new_vlan_profile)
{
    uint32 cmd = 0;
    uint32 profile_id = 0;

    if(dir == CTC_INGRESS)
    {
        cmd = DRV_IOW(DsSrcVlanProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, new_vlan_profile.profile_id, cmd, &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        profile_id = new_vlan_profile.profile_id;
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
    }
    else
    {
        cmd = DRV_IOW(DsDestVlanProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, new_vlan_profile.profile_id, cmd, &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile));
        profile_id = new_vlan_profile.profile_id;
        cmd = DRV_IOW(DsDestVlan_t, DsDestVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
    }

    return CTC_E_NONE;
}

/*set profile_id through vlan_id and dir;*/
STATIC int32
_sys_usw_vlan_set_vlan_profile(uint8 lchip, uint16 vlan_id, uint8 dir, sys_vlan_property_t prop, uint32 value)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 profile_id = 0;
    uint8 vlan_policer_en = 2;
    uint8 flag = 0;

    sys_vlan_profile_t* p_vlan_profile_get = NULL;
    sys_vlan_profile_t  old_vlan_profile;
    sys_vlan_profile_t  new_vlan_profile;
    sys_qos_policer_param_t polcier_param;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&old_vlan_profile, 0, sizeof(sys_vlan_profile_t));
    sal_memset(&new_vlan_profile, 0, sizeof(sys_vlan_profile_t));
    sal_memset(&polcier_param, 0, sizeof(polcier_param));

    if(CTC_INGRESS == dir)
    {
        cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsSrcVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
    }
    else
    {
        cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsDestVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_EGRESS;
    }

    old_vlan_profile.lchip = lchip;
    old_vlan_profile.profile_id = profile_id;
    sal_memcpy(&new_vlan_profile,&old_vlan_profile,sizeof(sys_vlan_profile_t));
    switch (prop)
    {
        case    SYS_VLAN_PROP_VLAN_STORM_CTL_PTR:
            SetDsSrcVlanProfile(V,vlanStormCtlPtr_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_VLAN_STORM_CTL_EN:
            SetDsSrcVlanProfile(V,vlanStormCtlEn_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID:
            SetDsSrcVlanProfile(V,ingressVlanSpanId_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN:
            SetDsSrcVlanProfile(V,ingressVlanSpanEn_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN:
            flag = GetDsSrcVlanProfile(V,vlanPolicerEn_f,&old_vlan_profile.sw_vlan_profile.vp.src_vlan_profile);
            if(value == flag)
            {
                return CTC_E_NONE;
            }
            if(value != 0)
            {
                if(old_vlan_profile.profile_id == 0 || old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->ingress_vlan_policer_num)
                {
                   CTC_ERROR_RETURN(_sys_usw_vlan_alloc_policer_ptr(lchip, &new_vlan_profile));
                }
                SetDsSrcVlanProfile(V,vlanPolicerEn_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value?1:0);
            }
            else
            {
              SetDsSrcVlanProfile(V,vlanPolicerEn_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value?1:0);
            }
            vlan_policer_en = value?1:0;
            break;
        case    SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN:
            flag = GetDsDestVlanProfile(V,vlanPolicerEn_f,&old_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile);
            if(value == flag)
            {
                return CTC_E_NONE;
            }
            if(value != 0)
            {
                if(old_vlan_profile.profile_id == 0 || old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->egress_vlan_policer_num)
                {
                    CTC_ERROR_RETURN(_sys_usw_vlan_alloc_policer_ptr(lchip, &new_vlan_profile));
                }
                SetDsDestVlanProfile(V,vlanPolicerEn_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,value?1:0);
            }
            else
            {
                SetDsDestVlanProfile(V,vlanPolicerEn_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,value?1:0);
            }

            vlan_policer_en = value?1:0;
            break;
        case    SYS_VLAN_PROP_CID:
            SYS_USW_CID_CHECK(lchip,value);
            SetDsSrcVlanProfile(V,categoryId_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            SetDsSrcVlanProfile(V,categoryIdValid_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,!!value);
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY0:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY1:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY2:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY3:
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL0:
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL1:
            SetDsSrcVlanProfile(V,gIngressAcl_1_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL2:
            SetDsSrcVlanProfile(V,gIngressAcl_2_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL3:
            SetDsSrcVlanProfile(V,gIngressAcl_3_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3:
            if(!value)
            {
                SetDsSrcVlanProfile(V,gIngressAcl_3_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
                SetDsSrcVlanProfile(V,gIngressAcl_3_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
            }
            else
            {
                SetDsSrcVlanProfile(V,gIngressAcl_3_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,1);
            }
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2:
            if(!value)
            {
                SetDsSrcVlanProfile(V,gIngressAcl_2_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
                SetDsSrcVlanProfile(V,gIngressAcl_2_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
            }
            else
            {
                SetDsSrcVlanProfile(V,gIngressAcl_2_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,1);
            }
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1:
            if(!value)
            {
                SetDsSrcVlanProfile(V,gIngressAcl_1_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
                SetDsSrcVlanProfile(V,gIngressAcl_1_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
            }
            else
            {
                SetDsSrcVlanProfile(V,gIngressAcl_1_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,1);
            }
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0:
            if(!value)
            {
                SetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
                SetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,0);
            }
            else
            {
                SetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,1);
            }
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0:
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,
                                                                    sys_usw_map_acl_tcam_lkup_type(lchip, value));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE1:
            SetDsSrcVlanProfile(V,gIngressAcl_1_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,
                                                                    sys_usw_map_acl_tcam_lkup_type(lchip, value));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE2:
            SetDsSrcVlanProfile(V,gIngressAcl_2_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,
                                                                    sys_usw_map_acl_tcam_lkup_type(lchip, value));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE3:
            SetDsSrcVlanProfile(V,gIngressAcl_3_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,
                                                                    sys_usw_map_acl_tcam_lkup_type(lchip, value));
            break;

        case    SYS_VLAN_PROP_FORCE_IPV6LOOKUP:
            SetDsSrcVlanProfile(V,forceIpv6Lookup_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_FORCE_IPV4LOOKUP:
            SetDsSrcVlanProfile(V,forceIpv4Lookup_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_FIP_EXCEPTION_TYPE:
            SetDsSrcVlanProfile(V,fipExceptionType_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

         case    SYS_VLAN_PROP_PIM_SNOOP_EN:
            SetDsSrcVlanProfile(V,pimSnoopEn_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value?1:0);
            break;

        case    SYS_VLAN_PROP_PTP_CLOCK_ID:
            SetDsSrcVlanProfile(V,ptpIndex_f,&new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID:
            SetDsDestVlanProfile(V,egressVlanSpanId_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN:
            SetDsDestVlanProfile(V,egressVlanSpanEn_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_ROUTED_ONLY:
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_LABEL0:
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,value);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0:
            if(!value)
            {
                SetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,0);
                SetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,0);
            }
            else
            {
                SetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,1);
            }
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0:
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f,&new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,
                                                                    sys_usw_map_acl_tcam_lkup_type(lchip, value));
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    if(!sal_memcmp(&new_vlan_profile, &old_vlan_profile, sizeof(old_vlan_profile)))
    {
        return CTC_E_NONE;
    }
    /* vlan policer enable */
    if(vlan_policer_en == 1)
    {
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_vlan_master[lchip]->vprofile_spool,&old_vlan_profile, NULL));
    }
    /* vlan policer disable */
    else if(vlan_policer_en == 0)
    {
        ret = ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool,&new_vlan_profile, &old_vlan_profile, &p_vlan_profile_get);
        if(ret == CTC_E_NONE)
        {
            CTC_ERROR_RETURN(_sys_usw_vlan_free_policer_ptr(lchip, &old_vlan_profile));
            new_vlan_profile.profile_id = p_vlan_profile_get->profile_id;
        }
    }
    /* no vlan policer process */
    else if(vlan_policer_en == 2)
    {
        if(CTC_INGRESS == dir && (old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->ingress_vlan_policer_num ||
            old_vlan_profile.profile_id == 0))
        {
            /*update spool*/
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool,&new_vlan_profile, &old_vlan_profile, &p_vlan_profile_get));
            new_vlan_profile.profile_id = p_vlan_profile_get->profile_id;
        }

        if(CTC_EGRESS == dir && (old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->egress_vlan_policer_num ||
            old_vlan_profile.profile_id == 0))
        {
            /*update spool*/
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool,&new_vlan_profile, &old_vlan_profile, &p_vlan_profile_get));
            new_vlan_profile.profile_id = p_vlan_profile_get->profile_id;
        }
    }
    /*write hardware*/
    CTC_ERROR_RETURN(_sys_usw_vlan_set_vlan_profile_add_to_asic(dir, lchip, vlan_id, new_vlan_profile));

    return CTC_E_NONE;
}

/*get profile_id through vlan_id and dir;*/
STATIC int32
_sys_usw_vlan_get_vlan_profile(uint8 lchip, uint16 vlan_id, uint8 dir, sys_vlan_property_t prop, uint32* value)
{
    uint32 cmd = 0;
    uint32 profile_id = 0;
    DsSrcVlanProfile_m src_vlan_profile;
    DsDestVlanProfile_m dest_vlan_profile;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(value);
    if(CTC_INGRESS == dir)
    {
        cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsSrcVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,profile_id,cmd,&src_vlan_profile));
    }
    else
    {
        cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsDestVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,profile_id,cmd,&dest_vlan_profile));
    }

    if (0 == profile_id)
    { /*no profile present means all profile prop is zero!*/
        *value = 0;
    }
    else
    {
        switch (prop)
        {
        case    SYS_VLAN_PROP_VLAN_STORM_CTL_PTR:
            *value  = GetDsSrcVlanProfile(V,vlanStormCtlPtr_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_VLAN_STORM_CTL_EN:
            *value  = GetDsSrcVlanProfile(V,vlanStormCtlEn_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID:
            *value  = GetDsSrcVlanProfile(V,ingressVlanSpanId_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN:
            *value  = GetDsSrcVlanProfile(V,ingressVlanSpanEn_f,&src_vlan_profile);
            break;
        case    SYS_VLAN_PROP_CID:
            *value = GetDsSrcVlanProfile(V,categoryId_f,&src_vlan_profile);
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY0:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY1:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY2:
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY3:
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL0:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL1:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_1_aclLabel_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL2:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_2_aclLabel_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL3:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_3_aclLabel_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_3_aclLookupType_f,&src_vlan_profile) ? 1:0;
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_2_aclLookupType_f,&src_vlan_profile) ? 1:0;
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_1_aclLookupType_f,&src_vlan_profile) ? 1:0;
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0:
            *value  = GetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f,&src_vlan_profile) ? 1:0;
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0:
            *value = sys_usw_unmap_acl_tcam_lkup_type(lchip,
                        GetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f,&src_vlan_profile));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE1:
            *value = sys_usw_unmap_acl_tcam_lkup_type(lchip,
                        GetDsSrcVlanProfile(V,gIngressAcl_1_aclLookupType_f,&src_vlan_profile));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE2:
            *value = sys_usw_unmap_acl_tcam_lkup_type(lchip,
                        GetDsSrcVlanProfile(V,gIngressAcl_2_aclLookupType_f,&src_vlan_profile));
            break;

        case    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE3:
            *value = sys_usw_unmap_acl_tcam_lkup_type(lchip,
                        GetDsSrcVlanProfile(V,gIngressAcl_3_aclLookupType_f,&src_vlan_profile));
            break;

        case    SYS_VLAN_PROP_FORCE_IPV6LOOKUP:
            *value  = GetDsSrcVlanProfile(V,forceIpv6Lookup_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_FORCE_IPV4LOOKUP:
            *value  = GetDsSrcVlanProfile(V,forceIpv4Lookup_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_FIP_EXCEPTION_TYPE:
            *value  = GetDsSrcVlanProfile(V,fipExceptionType_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_PIM_SNOOP_EN:
            *value  = GetDsSrcVlanProfile(V,pimSnoopEn_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_PTP_CLOCK_ID:
            *value  = GetDsSrcVlanProfile(V,ptpIndex_f,&src_vlan_profile);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID:
            *value  = GetDsDestVlanProfile(V,egressVlanSpanId_f,&dest_vlan_profile);
            break;
        case    SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN:
            if(GetDsSrcVlanProfile(V, vlanPolicerEn_f, &src_vlan_profile))
            {
                *value = 1;
            }
        break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN:
            *value  = GetDsDestVlanProfile(V,egressVlanSpanEn_f,&dest_vlan_profile);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_ROUTED_ONLY:
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_LABEL0:
            *value  = GetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f,&dest_vlan_profile);
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0:
            *value = GetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f,&dest_vlan_profile) ? 1:0;
            break;

        case    SYS_VLAN_PROP_EGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0:
            *value = sys_usw_unmap_acl_tcam_lkup_type(lchip,
                        GetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f,&dest_vlan_profile));
            break;
        case    SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN:
            if(GetDsDestVlanProfile(V, vlanPolicerEn_f, &dest_vlan_profile))
            {
                *value = 1;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_vlan_set_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32 value)
{
    uint32 cmd1 = 0;
    uint32 cmd2 = 0;
    uint16 vlan_ptr = 0;
    uint32 field_id = 0;
    uint32 index = 0;

    SYS_VLAN_INIT_CHECK();

    /* system modified by liwh for bug 54430, 2019-11-19 
       for hybrid, when vlan is 0, vlan stpid need be set, should not check vlan range */
    if (0 != vlan_id)
    {
        CTC_VLAN_RANGE_CHECK(vlan_id);
    }
    /* liwh end */

    /* system modified by liwh for bug 54430, 2019-11-19 
       for hybrid, when vlan is not created, vlan stpid need be set, should not check vlan is created */
    if (SYS_VLAN_PROP_STP_ID != vlan_prop)
    {
        SYS_VLAN_CREATE_CHECK(vlan_id);
    }
    /* liwh end */

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "vlan_id :%d, vlan_prop:%d, value:%d \n", vlan_id, vlan_prop, value);
    vlan_ptr = vlan_id;

    if (vlan_prop >= SYS_VLAN_PROP_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    else
    {
        index = vlan_ptr;
        switch (vlan_prop)
        {
            /* vlan proprety for sys layer*/
            case SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD:
                cmd1 = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd1, &index));
                field_id = DsVsi_array_0_macSecurityVsiDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
                cmd1 = DRV_IOW(DsVsi_t, field_id);
                value = (value) ? 1 : 0;
                index = (index >> 2) & 0xFFF;
                break;

            case SYS_VLAN_PROP_STP_ID:
                cmd1 = DRV_IOW(DsSrcVlan_t, DsSrcVlan_stpId_f);
                cmd2 = DRV_IOW(DsDestVlan_t, DsDestVlan_stpId_f);
                break;

            case SYS_VLAN_PROP_INTERFACE_ID:
                cmd1 = DRV_IOW(DsSrcVlan_t, DsSrcVlan_interfaceId_f);
                cmd2 = DRV_IOW(DsDestVlan_t, DsDestVlan_interfaceId_f);
                break;

            case SYS_VLAN_PROP_SRC_QUEUE_SELECT:

                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

            case SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION:
                cmd1 = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd1, &index));
                field_id = DsVsi_array_0_macSecurityVsiExceptionEn_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
                cmd1 = DRV_IOW(DsVsi_t, field_id);
                value = (value) ? 1 : 0;
                index = (index >> 2) & 0xFFF;
                break;

            case SYS_VLAN_PROP_VLAN_STORM_CTL_PTR:
            case SYS_VLAN_PROP_VLAN_STORM_CTL_EN:
            case SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID:
            case SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN:
                CTC_ERROR_RETURN(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS,vlan_prop, value));
                break;

            case SYS_VLAN_PROP_INGRESS_STATSPTR:
                cmd1 = DRV_IOW(DsSrcVlan_t, DsSrcVlan_statsPtr_f);
                break;

            case SYS_VLAN_PROP_EGRESS_STATSPTR:
                cmd1 = DRV_IOW(DsDestVlan_t, DsDestVlan_statsPtr_f);
                break;

            case SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID:
            case SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN:
                CTC_ERROR_RETURN(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_EGRESS,vlan_prop, value));
                break;

            case SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN:
                CTC_ERROR_RETURN(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS,vlan_prop, value));
                break;

            case SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN:
                CTC_ERROR_RETURN(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_EGRESS,vlan_prop, value));
                break;

            default:
                return CTC_E_INVALID_PARAM;

        }
    }

    if (cmd1)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd1, &value));
    }

    if (cmd2)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd2, &value));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_get_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32* value)
{
    uint32 cmd = 0;
    uint16 vlan_ptr = 0;
    uint32 field_id = 0;
    uint32 index = 0, stats_ptr = 0;

    LCHIP_CHECK(lchip);
    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(value);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(value);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "vlan_id :%d vlan_prop:%d \n", vlan_id, vlan_prop);
    vlan_ptr = vlan_id;

    /* zero value*/
    *value = 0;

    switch (vlan_prop)
    {
    /* vlan proprety for sys layer*/
    case SYS_VLAN_PROP_STP_ID:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_stpId_f);
        index = vlan_ptr;
        break;

    case SYS_VLAN_PROP_INTERFACE_ID:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_interfaceId_f);
        index = vlan_ptr;
        break;

    case SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_macSecurityVsiDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case SYS_VLAN_PROP_SRC_QUEUE_SELECT:

        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    case SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_macSecurityVsiExceptionEn_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case SYS_VLAN_PROP_VLAN_STORM_CTL_PTR:
    case SYS_VLAN_PROP_VLAN_STORM_CTL_EN:
    case SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID:
    case SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN:
        CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, vlan_prop, value));
        cmd = 0;
        break;

    case SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID:
    case SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN:
        CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_EGRESS, vlan_prop, value));
        cmd = 0;
        break;

    case SYS_VLAN_PROP_INGRESS_STATSPTR:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd, &stats_ptr));
        CTC_ERROR_RETURN(sys_usw_flow_stats_lookup_statsid(lchip, CTC_STATS_STATSID_TYPE_VLAN, stats_ptr, value , CTC_INGRESS));
        cmd = 0;
        break;

    case SYS_VLAN_PROP_EGRESS_STATSPTR:
        cmd = DRV_IOR(DsDestVlan_t, DsDestVlan_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlan_ptr, cmd, &stats_ptr));
        CTC_ERROR_RETURN(sys_usw_flow_stats_lookup_statsid(lchip, CTC_STATS_STATSID_TYPE_VLAN, stats_ptr, value, CTC_EGRESS));
        cmd = 0;
        break;

   case SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN:
        CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, vlan_prop, value));
        break;

   case SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN:
        CTC_ERROR_RETURN(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_EGRESS, vlan_prop, value));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if (cmd)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, value));
    }

    return CTC_E_NONE;
}

#define __WARMBOOT__

int32
_sys_usw_vlan_overlay_wb_mapping(sys_wb_vlan_overlay_mapping_key_t* p_wb_vlan_overlay, sys_vlan_overlay_mapping_key_t* p_vlan_overlay, uint8 sync)
{
    if (sync)
    {
        p_wb_vlan_overlay->vn_id = p_vlan_overlay->vn_id;
        p_wb_vlan_overlay->fid = p_vlan_overlay->fid;
    }
    else
    {
        p_vlan_overlay->vn_id = p_wb_vlan_overlay->vn_id;
        p_vlan_overlay->fid = p_wb_vlan_overlay->fid;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_vlan_wb_sync_func(void* bucket_data, void* user_data)
{
    uint16 max_entry_cnt = 0;
    ctc_wb_data_t* p_wb_data = (ctc_wb_data_t*) user_data;
    sys_vlan_overlay_mapping_key_t* p_vlan_overlay = (sys_vlan_overlay_mapping_key_t*) bucket_data;
    sys_wb_vlan_overlay_mapping_key_t* p_wb_vlan_overlay = NULL;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / sizeof(sys_wb_vlan_overlay_mapping_key_t);
    p_wb_vlan_overlay = (sys_wb_vlan_overlay_mapping_key_t*) p_wb_data->buffer + p_wb_data->valid_cnt;
    CTC_ERROR_RETURN(_sys_usw_vlan_overlay_wb_mapping(p_wb_vlan_overlay, p_vlan_overlay, 1));
    if (++p_wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(p_wb_data));
        p_wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

#define __VLAN_API__

int32
sys_usw_vlan_wb_sync(uint8 lchip)
{
    uint8 loop = 0;
    int32 ret = CTC_E_NONE;
    sys_wb_vlan_master_t* p_wb_vlan_master = NULL;
    ctc_wb_data_t wb_data;

    /*sync up master*/
    wb_data.buffer = mem_malloc(MEM_VLAN_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    p_wb_vlan_master = (sys_wb_vlan_master_t*)wb_data.buffer;
    p_wb_vlan_master->lchip = lchip;
    p_wb_vlan_master->version = SYS_WB_VERSION_VLAN;
    for (loop = 0; loop < 128; loop++)
    {
        p_wb_vlan_master->vlan_bitmap[loop] = p_usw_vlan_master[lchip]->vlan_bitmap[loop];
        p_wb_vlan_master->vlan_def_bitmap[loop] = p_usw_vlan_master[lchip]->vlan_def_bitmap[loop];
    }
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_vlan_master_t, CTC_FEATURE_VLAN, SYS_WB_APPID_VLAN_SUBID_MASTER);
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*sync overlay*/
    /*for different kinds of data stored, the wb_data must be reinited*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_vlan_overlay_mapping_key_t, CTC_FEATURE_VLAN, SYS_WB_APPID_VLAN_SUBID_OVERLAY_MAPPING_KEY);
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_vlan_master[lchip]->vni_mapping_hash, _sys_usw_vlan_wb_sync_func, (void*)(&wb_data)), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}

int32
sys_usw_vlan_wb_restore(uint8 lchip)
{
    uint8 loop = 0;
    uint16 index = 0;
    uint32 profile_id = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    ctc_wb_query_t wb_query;
    sys_wb_vlan_master_t* p_wb_vlan_master = NULL;
    sys_wb_vlan_overlay_mapping_key_t wb_vlan_overlay;
    sys_vlan_overlay_mapping_key_t* p_vlan_overlay = NULL;
    sys_vlan_profile_t src_vlan_profile;
    sys_vlan_profile_t dest_vlan_profile;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_VLAN_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    p_wb_vlan_master = mem_malloc(MEM_VLAN_MODULE, sizeof(sys_wb_vlan_master_t));
    if ((NULL == wb_query.buffer) || (NULL == p_wb_vlan_master))
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(p_wb_vlan_master, 0, sizeof(sys_wb_vlan_master_t));

    /*restore master*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(p_wb_vlan_master, 0, sizeof(sys_wb_vlan_master_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_vlan_master_t, CTC_FEATURE_VLAN, SYS_WB_APPID_VLAN_SUBID_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query vlan master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }
    sal_memcpy((uint8*)p_wb_vlan_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_VLAN, p_wb_vlan_master->version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }
    for (loop = 0; loop < 128; loop++)
    {
        p_usw_vlan_master[lchip]->vlan_bitmap[loop] = p_wb_vlan_master->vlan_bitmap[loop];
        p_usw_vlan_master[lchip]->vlan_def_bitmap[loop] = p_wb_vlan_master->vlan_def_bitmap[loop];
    }
    /*restore overlay*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_vlan_overlay_mapping_key_t, CTC_FEATURE_VLAN, SYS_WB_APPID_VLAN_SUBID_OVERLAY_MAPPING_KEY);
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&wb_vlan_overlay, 0, sizeof(sys_wb_vlan_overlay_mapping_key_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_vlan_overlay, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_vlan_overlay = mem_malloc(MEM_VLAN_MODULE, sizeof(sys_vlan_overlay_mapping_key_t));
        if (NULL == p_vlan_overlay)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_vlan_overlay, 0, sizeof(sys_vlan_overlay_mapping_key_t));
        CTC_ERROR_GOTO(_sys_usw_vlan_overlay_wb_mapping(&wb_vlan_overlay, p_vlan_overlay, 0), ret, done);
        ctc_hash_insert(p_usw_vlan_master[lchip]->vni_mapping_hash, (void*)(p_vlan_overlay));
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore spool*/
    for (index = 0; index <= CTC_MAX_VLAN_ID; index++)
    {
        /*ingree direction*/
        profile_id = 0;
        if (CTC_IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(index >> 5)], (index & 0x1F)))
        {
            cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_profileId_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &profile_id), ret, done);
        }
        if(profile_id && profile_id <= p_usw_vlan_master[lchip]->ingress_vlan_policer_num)
        {
            CTC_ERROR_GOTO(_sys_usw_vlan_alloc_policer_ptr_from_position(lchip, CTC_INGRESS, profile_id), ret, done);
        }
        else if (0 != profile_id)
        {
            sal_memset(&src_vlan_profile, 0, sizeof(sys_vlan_profile_t));
            src_vlan_profile.lchip = lchip;
            src_vlan_profile.profile_id = profile_id;
            src_vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
            cmd = DRV_IOR(DsSrcVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, profile_id, cmd, &src_vlan_profile.sw_vlan_profile.vp.src_vlan_profile), ret, done);

            ret = ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool, &src_vlan_profile, NULL, NULL);
            if (ret < 0)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }

        /*egress direction*/
        profile_id = 0;
        if (CTC_IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(index >> 5)], (index & 0x1F)))
        {
            cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &profile_id), ret, done);
        }
        if(profile_id && profile_id <= p_usw_vlan_master[lchip]->egress_vlan_policer_num)
        {
            CTC_ERROR_GOTO(_sys_usw_vlan_alloc_policer_ptr_from_position(lchip, CTC_EGRESS, profile_id), ret, done);
        }
        else if (0 != profile_id)
        {
            sal_memset(&dest_vlan_profile, 0, sizeof(sys_vlan_profile_t));
            dest_vlan_profile.lchip = lchip;
            dest_vlan_profile.profile_id = profile_id;
            dest_vlan_profile.sw_vlan_profile.dir = CTC_EGRESS;
            cmd = DRV_IOR(DsDestVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, profile_id, cmd, &dest_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile), ret, done);
            ret = ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool, &dest_vlan_profile, NULL, NULL);

            if (ret < 0)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }
    }

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    if (p_wb_vlan_master)
    {
        mem_free(p_wb_vlan_master);
    }

    return ret;
}

/* init the vlan module */
int32
sys_usw_vlan_init(uint8 lchip, ctc_vlan_global_cfg_t* vlan_global_cfg)
{
    uint32 index = 0;
    uint32 cmd = 0;
    uint16 vlan_policer_num = 0;
    uint16 start_offset = 0;
    uint32 entry_num_vlan = 0;
    uint32 entry_num_src_vlan_profile = 0;
    uint32 entry_num_dest_vlan_profile = 0;
    uint32 entry_num = 0;
    DsSrcVlan_m ds_src_vlan;
    DsDestVlan_m ds_dest_vlan;
    uint32 vlan_tag_bit_map[SYS_VLAN_TAG_BITMAP_NUM] = {0};    /*Use Macro for TM*/
    DsVlanTagBitMap_m ds_vlan_tag_bit_map;
    DsSrcVlanStatus_m ds_src_vlan_status;
    DsDestVlanStatus_m ds_dest_vlan_status;
    DsVsi_m ds_vlan_vsi;
    DsSrcVlanProfile_m ds_src_vlan_profile;
    DsDestVlanProfile_m ds_dest_vlan_profile;
    sys_vlan_profile_t vlan_profile;

    sys_usw_opf_t opf;
    ctc_spool_t spool;

    LCHIP_CHECK(lchip);
    sal_memset(&ds_src_vlan, 0, sizeof(DsSrcVlan_m));
    sal_memset(&ds_dest_vlan, 0, sizeof(DsDestVlan_m));
    sal_memset(&ds_vlan_tag_bit_map, 0, sizeof(DsVlanTagBitMap_m));
    sal_memset(&ds_src_vlan_status, 0, sizeof(DsSrcVlanStatus_m));
    sal_memset(&ds_dest_vlan_status, 0, sizeof(DsDestVlanStatus_m));
    sal_memset(&ds_src_vlan_profile, 0, sizeof(DsSrcVlanProfile_m));
    sal_memset(&ds_dest_vlan_profile, 0, sizeof(DsDestVlanProfile_m));
    sal_memset(&ds_vlan_vsi, 0, sizeof(DsVsi_m));
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    sal_memset(&vlan_profile,0,sizeof(vlan_profile));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsSrcVlan_t,
                                                                    &entry_num_vlan));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsSrcVlanProfile_t,
                                                                    &entry_num_src_vlan_profile));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsDestVlanProfile_t,
                                                                    &entry_num_dest_vlan_profile));

    if (NULL != p_usw_vlan_master[lchip])
    {
        return CTC_E_NONE;
    }

    CTC_PTR_VALID_CHECK(vlan_global_cfg);

    /*init p_usw_vlan_master*/
    SYS_VLAN_NO_MEMORY_RETURN(p_usw_vlan_master[lchip] = \
                                  (sys_vlan_master_t*)mem_malloc(MEM_VLAN_MODULE, sizeof(sys_vlan_master_t)));

    /*vlan mode*/
    p_usw_vlan_master[lchip]->vlan_mode = vlan_global_cfg->vlanptr_mode;


    /*init vlan */
    sal_memset(p_usw_vlan_master[lchip]->vlan_bitmap, 0, sizeof(uint32) * 128);
    sal_memset(p_usw_vlan_master[lchip]->vlan_def_bitmap, 0, sizeof(uint32) * 128);


    /*init vlan profile*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = (entry_num_src_vlan_profile + entry_num_dest_vlan_profile) ;
    spool.max_count = entry_num_src_vlan_profile + entry_num_dest_vlan_profile;
    spool.user_data_size = sizeof(sys_vlan_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_vlan_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_vlan_hash_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_vlan_profile_index_build;
    spool.spool_free= (spool_free_fn)_sys_usw_vlan_profile_index_free;
    p_usw_vlan_master[lchip]->vprofile_spool = ctc_spool_create(&spool);

    /* vlan filtering/tagged mode 0 :4K vlan * 64 ports per slice*/
    p_usw_vlan_master[lchip]->vlan_port_bitmap_mode = 1;        /*Modify 0-->1, 1=D2, 0=default TM*/

    /* create user vlan add default entry */
    p_usw_vlan_master[lchip]->sys_vlan_def_mode = 1;

    if (0 != entry_num_src_vlan_profile)
    {
        /*ingress opf init start*/
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip,&p_usw_vlan_master[lchip]->src_opf_type, 2,"opf-src-vlan-profile"));
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = p_usw_vlan_master[lchip]->src_opf_type;
        CTC_ERROR_RETURN(sys_usw_qos_policer_ingress_vlan_get(lchip, &vlan_policer_num));
        p_usw_vlan_master[lchip]->ingress_vlan_policer_num = vlan_policer_num;
        if(vlan_policer_num != 0)
        {
            start_offset = 1 ;
            opf.pool_index = 1;
            CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, vlan_policer_num));
        }
        opf.pool_index = 0;
        start_offset =  vlan_policer_num + 1;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset
                             (lchip, &opf, start_offset , entry_num_src_vlan_profile - vlan_policer_num - 1));
        /* reserved for reverse vlan profile resource allocation to make sure the ingress span function can always work although the resource have been used up by forward allocation */
        CTC_ERROR_RETURN(sys_usw_opf_init_reverse_size(lchip, &opf, SYS_VLAN_RESERVE_NUM));

        /*egress opf init start*/
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip,&p_usw_vlan_master[lchip]->dest_opf_type, 2,"opf-dest-vlan-profile"));
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = p_usw_vlan_master[lchip]->dest_opf_type;
        CTC_ERROR_RETURN(sys_usw_qos_policer_egress_vlan_get(lchip, &vlan_policer_num));
        p_usw_vlan_master[lchip]->egress_vlan_policer_num = vlan_policer_num;
        if(vlan_policer_num != 0)
        {
            start_offset = 1 ;
            opf.pool_index = 1;
            CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, vlan_policer_num));
        }
        opf.pool_index = 0;
        start_offset =  vlan_policer_num + 1;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset
                             (lchip, &opf, start_offset, entry_num_dest_vlan_profile - vlan_policer_num - 1));
        CTC_ERROR_RETURN(sys_usw_opf_init_reverse_size(lchip, &opf, SYS_VLAN_RESERVE_NUM));
    }

    /*init asic table: dsvlan dsvlanbitmap*/
    _sys_usw_vlan_set_default_vlan(lchip, &ds_src_vlan, &ds_dest_vlan);
    for (index = 0; index < entry_num_vlan; index++)
    {
        cmd = DRV_IOW(DsSrcVlan_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan));

        cmd = DRV_IOW(DsDestVlan_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan));

    }

    /*Init DsVlan 0 for port crossconnect or vpls*/
    SetDsSrcVlan(V, receiveDisable_f, &ds_src_vlan, 0);
    cmd = DRV_IOW(DsSrcVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_src_vlan));

    SetDsDestVlan(V, transmitDisable_f, &ds_dest_vlan, 0);
    cmd = DRV_IOW(DsDestVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_dest_vlan));

    sal_memset(vlan_tag_bit_map, 0xff, sizeof(uint32) * MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM) / 32);     /*Modified for TM*/
    SetDsVlanTagBitMap(A, tagPortBitMap_f, &ds_vlan_tag_bit_map, vlan_tag_bit_map);
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);

/* SYSTEM MODIFIED by hansf  for bug 43256 2018/11/06 */
#if 0
    sal_memcpy(&ds_src_vlan_status, &vlan_tag_bit_map, sizeof(DsSrcVlanStatus_m));
    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_src_vlan_status));

    sal_memcpy(&ds_dest_vlan_status, &vlan_tag_bit_map, sizeof(DsDestVlanStatus_m));
    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_dest_vlan_status));
#endif

    vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
    vlan_profile.lchip = lchip;
    vlan_profile.profile_id = 0;
    CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_vlan_master[lchip]->vprofile_spool, &vlan_profile));
    vlan_profile.sw_vlan_profile.dir = CTC_EGRESS;
    CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_vlan_master[lchip]->vprofile_spool, &vlan_profile));
#if 0
    /*init src vlan profile*/
    for (index = 0; index < entry_num_vlan_profile; index++)
    {
        CTC_ERROR_RETURN(_sys_usw_vlan_profile_write(lchip, index, &ds_src_vlan_profile,
                                                            &ds_dest_vlan_profile));
    }

    /* init src vlan profile template */
    profile_id = 1;
    sal_memset(&vlan_profile, 0, sizeof(sys_vlan_profile_t));
    vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
    vlan_profile.sw_vlan_profile.vp.src_vlan_profile.fip_exception_type  = 1;

    for (k = 0; k < 2; k++)
    {
        switch (k)
        {
            case 0:
                vlan_profile.sw_vlan_profile.vp.src_vlan_profile.force_ipv4_lookup = 0;
                break;

            case 1:
                vlan_profile.sw_vlan_profile.vp.src_vlan_profile.force_ipv4_lookup = 1;
                break;
        }

        CTC_ERROR_RETURN(_sys_usw_vlan_profile_static_add(lchip, &vlan_profile, &profile_id));
    }
#endif

    /*init Vsi 0, learning disable*/
    SetDsVsi(V, array_0_vsiLearningDisable_f, &ds_vlan_vsi, 1);
    cmd = DRV_IOW(DsVsi_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_vlan_vsi));


    p_usw_vlan_master[lchip]->vni_mapping_hash = ctc_hash_create(1,
                                                                (SYS_VLAN_OVERLAY_IHASH_MASK + 1),
                                                                (hash_key_fn)_sys_usw_vlan_overlay_hash_make,
                                                                (hash_cmp_fn)_sys_usw_vlan_overlay_hash_cmp);

    SYS_VLAN_CREATE_LOCK(lchip);
    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_VLAN, sys_usw_vlan_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_wb_restore(lchip));
    }
    sys_usw_ftm_query_table_entry_num(lchip, DsVlanRangeProfile_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_VLAN_RANGE_GROUP_NUM) = entry_num;

    return CTC_E_NONE;
}

int32
sys_usw_vlan_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_vlan_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*free vni mapping hash*/
    ctc_hash_traverse(p_usw_vlan_master[lchip]->vni_mapping_hash, (hash_traversal_fn)_sys_usw_vlan_free_node_data, NULL);
    ctc_hash_free(p_usw_vlan_master[lchip]->vni_mapping_hash);

    sys_usw_opf_deinit(lchip, p_usw_vlan_master[lchip]->src_opf_type);

    sys_usw_opf_deinit(lchip, p_usw_vlan_master[lchip]->dest_opf_type);

    /*free vlan profile spool*/
    ctc_spool_free(p_usw_vlan_master[lchip]->vprofile_spool);

    sal_mutex_destroy(p_usw_vlan_master[lchip]->mutex);

    mem_free(p_usw_vlan_master[lchip]);

    return CTC_E_NONE;
}


/**
  @brief The function is to create a vlan

*/
int32
sys_usw_vlan_create_vlan(uint8 lchip, uint16 vlan_id)
{
    uint16 vlan_ptr = 0;
    uint8 field_id = 0;
    uint32 index = 0;
    uint8 temp = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    DsSrcVlan_m ds_src_vlan;
    DsDestVlan_m ds_dest_vlan;
    DsVsi_m ds_vlan_vsi;
    uint32 vlan_tag_bit_map[SYS_VLAN_TAG_BITMAP_NUM] = {0};   /*Use Macro for TM*/
    DsVlanTagBitMap_m ds_vlan_tag_bit_map;

    SYS_VLAN_INIT_CHECK();

    sal_memset(&ds_src_vlan, 0, sizeof(DsSrcVlan_m));
    sal_memset(&ds_dest_vlan, 0, sizeof(DsDestVlan_m));
    sal_memset(&ds_vlan_vsi, 0, sizeof(DsVsi_m));
    sal_memset(&ds_vlan_tag_bit_map, 0, sizeof(DsVlanTagBitMap_m));

#if 0
    if (p_usw_vlan_master->vlan_mode != CTC_VLANPTR_MODE_VLANID)
    {
        return CTC_E_INVALID_PARAM;
    }
#endif

    CTC_VLAN_RANGE_CHECK(vlan_id);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create vlan_id: %d!\n", vlan_id);

    SYS_VLAN_LOCK(lchip);

    if (IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_id >> 5)], (vlan_id & 0x1F)))
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan already create  \n");
        SYS_VLAN_UNLOCK(lchip);
		return CTC_E_EXIST;

    }
    /*vlan2ptr vlanid==vlanptr*/
    CTC_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_id >> 5)], (vlan_id & 0x1F));

    vlan_ptr = vlan_id;

    field_id = (vlan_id & 0x1)? DsVlan2Ptr_array_1_vlanPtr_f : DsVlan2Ptr_array_0_vlanPtr_f;
    cmd = DRV_IOW(DsVlan2Ptr_t, field_id);
    index = (vlan_id >> 1) & 0x7FF;
    value = vlan_ptr;
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value));
    CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_VLAN_PTR, vlan_id, value));


    /*vlan default set vlanid==fid*/
    _sys_usw_vlan_set_default_vlan(lchip, &ds_src_vlan, &ds_dest_vlan);

    SetDsSrcVlan(V, receiveDisable_f, &ds_src_vlan, 0);
    SetDsSrcVlan(V, vsiId_f, &ds_src_vlan, vlan_id);
    cmd = DRV_IOW(DsSrcVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_src_vlan));

    SetDsDestVlan(V, transmitDisable_f, &ds_dest_vlan, 0);
    cmd = DRV_IOW(DsDestVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_dest_vlan));

    /*vlan vsi*/
    cmd = DRV_IOR(DsVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, (vlan_ptr>>2), cmd, &ds_vlan_vsi));
    cmd = DRV_IOW(DsVsi_t, DRV_ENTRY_FLAG);
    for (temp = 0;temp < SYS_VLAN_VSI_PARAM_MAX; temp++)
    {
        field_id = (vlan_ptr & 0x3)*SYS_VLAN_VSI_PARAM_MAX + temp;
        DRV_SET_FIELD_V(lchip, DsVsi_t, field_id, &ds_vlan_vsi, 0);
    }

    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, (vlan_ptr>>2), cmd, &ds_vlan_vsi));


    /*vlan tag*/
    sal_memset(vlan_tag_bit_map, 0xff, sizeof(uint32) * MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM) / 32);     /*Modified for TM*/
    SetDsVlanTagBitMap(A, tagPortBitMap_f, &ds_vlan_tag_bit_map, vlan_tag_bit_map);
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_vlan_tag_bit_map));

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}
/**
  @brief The function is to create a vlan with vlanptr

*/
int32
sys_usw_vlan_create_uservlan(uint8 lchip, ctc_vlan_uservlan_t* user_vlan)
{
    uint16 vlan_id = 0;
    uint16 vlan_ptr = 0;
    uint16 temp_vlan_ptr = 0;
    uint16 fid = 0;

    uint8 field_id = 0;
    uint8 temp = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 index = 0;
    DsSrcVlan_m ds_src_vlan;
    DsDestVlan_m ds_dest_vlan;
    DsVsi_m ds_vlan_vsi;
    uint32 vlan_tag_bit_map[SYS_VLAN_TAG_BITMAP_NUM] = {0};   /*Use Macro for TM*/
    DsVlanTagBitMap_m  ds_vlan_tag_bit_map;

    CTC_PTR_VALID_CHECK(user_vlan);

    sal_memset(&ds_src_vlan, 0, sizeof(DsSrcVlan_m));
    sal_memset(&ds_dest_vlan, 0, sizeof(DsDestVlan_m));
    sal_memset(&ds_vlan_vsi, 0, sizeof(DsVsi_m));
    sal_memset(&ds_vlan_tag_bit_map, 0, sizeof(DsVlanTagBitMap_m));

    vlan_id = user_vlan->vlan_id;
    vlan_ptr = user_vlan->user_vlanptr;
    fid = user_vlan->fid;

    SYS_VLAN_INIT_CHECK();

    if (p_usw_vlan_master[lchip]->vlan_mode != CTC_VLANPTR_MODE_USER_DEFINE1)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_VLAN_RANGE_CHECK(vlan_id);

    if ((vlan_ptr) < CTC_MIN_VLAN_ID || (vlan_ptr) > CTC_MAX_VLAN_ID)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_L2_FID_CHECK(fid);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create user vlan_id: %d, vlan_ptr: %d, fid: %d!\n", vlan_id, vlan_ptr, fid);

    SYS_VLAN_LOCK(lchip);

    /*if vlan_id mapping to vlan_ptr exist,return vlan exist*/
    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_ptr(lchip,vlan_id,&temp_vlan_ptr));
    if(0 != temp_vlan_ptr)
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan already create  \n");
        SYS_VLAN_UNLOCK(lchip);
		return CTC_E_EXIST;

    }

    if (IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Vlan already create  \n");
        SYS_VLAN_UNLOCK(lchip);
		return CTC_E_EXIST;

    }

    CTC_BIT_SET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F));

    field_id = (vlan_id & 0x1)? DsVlan2Ptr_array_1_vlanPtr_f : DsVlan2Ptr_array_0_vlanPtr_f;
    cmd = DRV_IOW(DsVlan2Ptr_t, field_id);
    index = (vlan_id >> 1) & 0x7FF;
    value = vlan_ptr;
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value));
    CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_VLAN_PTR, vlan_id, value));

    _sys_usw_vlan_set_default_vlan(lchip, &ds_src_vlan, &ds_dest_vlan);

    SetDsSrcVlan(V, receiveDisable_f, &ds_src_vlan, 0);
    SetDsSrcVlan(V, vsiId_f, &ds_src_vlan, fid);
    cmd = DRV_IOW(DsSrcVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_src_vlan));
    SetDsDestVlan(V, transmitDisable_f, &ds_dest_vlan, 0);
    cmd = DRV_IOW(DsDestVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_dest_vlan));


    /*clear vlan vsi*/
    cmd = DRV_IOR(DsVsi_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, (fid>>2), cmd, &ds_vlan_vsi));
    cmd = DRV_IOW(DsVsi_t, DRV_ENTRY_FLAG);
    for (temp = 0;temp < SYS_VLAN_VSI_PARAM_MAX; temp++)
    {
        field_id = (fid & 0x3)*SYS_VLAN_VSI_PARAM_MAX + temp;
        DRV_SET_FIELD_V(lchip, DsVsi_t, field_id, &ds_vlan_vsi, 0);
    }

    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, (fid>>2), cmd, &ds_vlan_vsi));


    /*set vlan tag*/
    sal_memset(vlan_tag_bit_map, 0xff, sizeof(uint32) * MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM) / 32);     /*Modified for TM*/
    SetDsVlanTagBitMap(A, tagPortBitMap_f, &ds_vlan_tag_bit_map, vlan_tag_bit_map);
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_vlan_tag_bit_map));


    if (p_usw_vlan_master[lchip]->sys_vlan_def_mode == 1)
    {
        ctc_l2dflt_addr_t def;
        sal_memset(&def, 0, sizeof(def));

        CTC_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F));

        if (CTC_FLAG_ISSET(user_vlan->flag, CTC_MAC_LEARN_USE_LOGIC_PORT))
        {
            CTC_SET_FLAG(def.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
        }
        def.fid = fid;
        def.l2mc_grp_id = fid;
        CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_l2_add_default_entry(lchip, &def));
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;

}

/**
@brief The function is to remove the vlan

*/
int32
sys_usw_vlan_destory_vlan(uint8 lchip, uint16 vlan_id)
{
    uint8 field_id = 0;
    uint16 vlan_ptr = 0;
    uint16  index = 0;
    uint32 cmd = 0;
    uint32 fid = 0;
    uint32 value = 0;
    uint32  profile_id;
    sys_vlan_profile_t old_vlan_profile;
    DsSrcVlan_m ds_src_vlan;
    DsDestVlan_m ds_dest_vlan;
    DsVlanTagBitMap_m ds_vlan_tag_bit_map;
    DsSrcVlanStatus_m ds_src_vlan_status;
    DsDestVlanStatus_m ds_dest_vlan_status;


    sal_memset(&ds_src_vlan, 0, sizeof(DsSrcVlan_m));
    sal_memset(&ds_dest_vlan, 0, sizeof(DsDestVlan_m));
    sal_memset(&ds_src_vlan_status, 0, sizeof(DsSrcVlanStatus_m));
    sal_memset(&ds_dest_vlan_status, 0, sizeof(DsDestVlanStatus_m));
    sal_memset(&ds_vlan_tag_bit_map, 0, sizeof(DsVlanTagBitMap_m));

    _sys_usw_vlan_get_vlan_ptr(lchip, vlan_id, &vlan_ptr);
    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_ptr);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove vlan_id: %d!\n", vlan_id);

    SYS_VLAN_LOCK(lchip);

    /* remove vlan profile*/
    sal_memset(&old_vlan_profile, 0, sizeof(old_vlan_profile));
    cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &profile_id));
    if(profile_id)
    {
        cmd = DRV_IOR(DsSrcVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
        old_vlan_profile.lchip = lchip;
        old_vlan_profile.profile_id = profile_id;
        ctc_spool_remove(p_usw_vlan_master[lchip]->vprofile_spool, &old_vlan_profile, NULL);
        if(old_vlan_profile.profile_id <= p_usw_vlan_master[lchip]->ingress_vlan_policer_num &&
           old_vlan_profile.profile_id != 0)
        {
            _sys_usw_vlan_free_policer_ptr(lchip, &old_vlan_profile);
        }
    }

    sal_memset(&old_vlan_profile, 0, sizeof(old_vlan_profile));
    cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &profile_id));
    if(profile_id)
    {
        cmd = DRV_IOR(DsDestVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_EGRESS;
        old_vlan_profile.lchip = lchip;
        old_vlan_profile.profile_id = profile_id;
        ctc_spool_remove(p_usw_vlan_master[lchip]->vprofile_spool, &old_vlan_profile, NULL);
        if(old_vlan_profile.profile_id <= p_usw_vlan_master[lchip]->egress_vlan_policer_num &&
           old_vlan_profile.profile_id != 0)
        {
            _sys_usw_vlan_free_policer_ptr(lchip, &old_vlan_profile);
        }
    }
    /*get vsiId and reset hw: ds_vlan*/
    cmd = DRV_IOR(DsSrcVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_src_vlan));
    GetDsSrcVlan(A, vsiId_f, &ds_src_vlan, &fid);

    _sys_usw_vlan_set_default_vlan(lchip, &ds_src_vlan, &ds_dest_vlan);

    cmd = DRV_IOW(DsSrcVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_src_vlan));

    cmd = DRV_IOW(DsDestVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_dest_vlan));


    CTC_BIT_UNSET(p_usw_vlan_master[lchip]->vlan_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F));

    /*reset hw: ds_vlan2ptr*/
    field_id = (vlan_id & 0x1)? DsVlan2Ptr_array_1_vlanPtr_f : DsVlan2Ptr_array_0_vlanPtr_f;
    cmd = DRV_IOW(DsVlan2Ptr_t, field_id);
    index = (vlan_id >> 1) & 0x7FF;
    value = 0;
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value));
    CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_VLAN_PTR, vlan_id, value));

    /*vlan tag & vlan filting*/
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);       /*Modified for TM*/
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_vlan_tag_bit_map));

    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_src_vlan_status));

    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &ds_dest_vlan_status));

    if(!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    if ((p_usw_vlan_master[lchip]->vlan_mode == CTC_VLANPTR_MODE_USER_DEFINE1) && (p_usw_vlan_master[lchip]->sys_vlan_def_mode == 1))
    {
        int32 ret = 0;
        ctc_l2dflt_addr_t def;
        sal_memset(&def, 0, sizeof(def));
        def.fid = fid;
        ret = sys_usw_l2_remove_default_entry(lchip, &def);
        if (CTC_E_NOT_EXIST == ret)
        {
            ret = CTC_E_NONE;
        }
        CTC_ERROR_RETURN_VLAN_UNLOCK(ret);
    }

    CTC_BIT_UNSET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F));

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_vlan_ptr(uint8 lchip, uint16 vlan_id, uint16* vlan_ptr)
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);

    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_ptr(lchip, vlan_id, vlan_ptr));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
@brief the function add ports to vlan
*/
int32
sys_usw_vlan_add_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    uint16 lport = 0;
    uint16 vlan_ptr = 0;
    uint16 loop1 = 0;
    uint16 loop2 = 0;
    uint32 value = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    uint32 index = 0;
    uint32 cmd = 0;
    int32  ret = CTC_E_NONE;
    ctc_l2dflt_addr_t l2dflt_addr;
    uint32 ds_vlan_status[SYS_VLAN_TAG_BITMAP_NUM] = {0};     /*Use Macro for TM*/
    ctc_port_bitmap_t r_port_bitmap = {0};      /*Append for TM*/

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_LOCK(lchip);
    if (!sal_memcmp(port_bitmap, r_port_bitmap, sizeof(uint32) * MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32))     /*Modified for TM*/
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    vlan_ptr = vlan_id;

    index = vlan_ptr;       /*Modified for TM*/

    /*write hw */
    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    for (loop1 = 0; loop1 < (MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32); loop1++)     /*Modified for TM*/
    {
        ds_vlan_status[loop1] |= port_bitmap[loop1];
    }

    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    cmd = DRV_IOR(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    for (loop1 = 0; loop1 < (MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32); loop1++)       /*Modified for TM*/
    {
        ds_vlan_status[loop1] |= port_bitmap[loop1];
    }

    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    if (!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &value));
    if ((p_usw_vlan_master[lchip]->vlan_mode != CTC_VLANPTR_MODE_USER_DEFINE1) || (p_usw_vlan_master[lchip]->sys_vlan_def_mode != 1))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    sys_usw_get_gchip_id(lchip, &gchip);
    for (loop1 = 0; loop1 < MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32; loop1++)     /*[D2] 64, [TM]232*/
    {
        if (port_bitmap[loop1] == 0)
        {
            continue;
        }
        for (loop2 = 0; loop2 < 32; loop2++)
        {
            lport = loop1 * 32 + loop2;
            if (CTC_BMP_ISSET(port_bitmap, lport))
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
                l2dflt_addr.fid = (uint16)value;
                l2dflt_addr.member.mem_port = gport;
                CTC_ERROR_GOTO(sys_usw_l2_add_port_to_default_entry(lchip, &l2dflt_addr), ret, roll_back);
            }
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;

roll_back:
    SYS_VLAN_UNLOCK(lchip);
    sys_usw_vlan_remove_ports(lchip, vlan_id, port_bitmap);
    return ret;
}

/**
@brief the function add port to vlan
*/
int32
sys_usw_vlan_add_port(uint8 lchip, uint16 vlan_id, uint32 gport)
{

    uint16 lport = 0;
    uint16 vlan_ptr = 0;
    uint8 field_id = 0;
    uint32 index = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    int32  ret_tmp = CTC_E_NONE;
    bool   ret = FALSE;
    ctc_l2dflt_addr_t l2dflt_addr;

    DsSrcVlanStatus_m ds_src_vlan_status;
    DsDestVlanStatus_m ds_dest_vlan_status;

    sal_memset(&ds_src_vlan_status, 0, sizeof(DsSrcVlanStatus_m));
    sal_memset(&ds_dest_vlan_status, 0, sizeof(DsDestVlanStatus_m));
    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add global port:%d, to vlan_id:%d!\n", gport, vlan_id);

    vlan_ptr = vlan_id;

    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    if (!CTC_IS_LINKAGG_PORT(gport))
    {
        ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
        if (FALSE == ret)
        {
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

        }
    }
    lport  = (lport > 255)? ((lport-256)&0xFF) : (lport&0xFF);
    SYS_VLAN_FILTER_PORT_CHECK(lport);

    SYS_VLAN_LOCK(lchip);

    index = vlan_ptr;      /*Modified for TM*/

    field_id =  lport;
    if (1 == _sys_usw_vlan_is_member_port(lchip, field_id, index))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    /*write hw */
    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan_status));
    DRV_SET_FIELD_V(lchip, DsSrcVlanStatus_t, field_id, &ds_src_vlan_status, 1);
    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan_status));

    cmd = DRV_IOR(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan_status));
    DRV_SET_FIELD_V(lchip, DsDestVlanStatus_t, field_id, &ds_dest_vlan_status, 1);
    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan_status));


    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &value));

    if(!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    if ((p_usw_vlan_master[lchip]->vlan_mode == CTC_VLANPTR_MODE_USER_DEFINE1) && (p_usw_vlan_master[lchip]->sys_vlan_def_mode == 1))
    {
        l2dflt_addr.fid = (uint16)value;
        l2dflt_addr.member.mem_port = gport;
        CTC_ERROR_GOTO(sys_usw_l2_add_port_to_default_entry(lchip, &l2dflt_addr), ret_tmp, roll_back);
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;

roll_back:
    SYS_VLAN_UNLOCK(lchip);
    sys_usw_vlan_remove_port(lchip, vlan_id, gport);
    return ret_tmp;
}

int32
sys_usw_vlan_set_tagged_port(uint8 lchip, uint16 vlan_id, uint32 gport, bool tagged)
{

    uint16 lport = 0;
    uint16 vlan_ptr = 0;
    uint32 index = 0;
    uint32 cmd = 0;

    bool   ret = FALSE;
    uint32 vlan_tag_bit_map[SYS_VLAN_TAG_BITMAP_NUM] = {0};       /*Use Macro for TM*/
    DsVlanTagBitMap_m  ds_vlan_tag_bit_map;

    sal_memset(&ds_vlan_tag_bit_map, 0, sizeof(DsVlanTagBitMap_m));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add global port:%d, to vlan_id:%d!\n", gport, vlan_id);

    vlan_ptr = vlan_id;

    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    if (!CTC_IS_LINKAGG_PORT(gport))
    {
        ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
        if (FALSE == ret)
        {
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

        }
    }
    lport  = (lport > 255)? ((lport-256)&0xFF) : (lport&0xFF);
    SYS_VLAN_FILTER_PORT_CHECK(lport);

    SYS_VLAN_LOCK(lchip);

    cmd = DRV_IOR(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);

    index = vlan_ptr;        /*Modified for TM*/

    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_tag_bit_map));
    sal_memcpy(vlan_tag_bit_map, &ds_vlan_tag_bit_map, sizeof(DsVlanTagBitMap_m));
    if (tagged)      /*Modified for TM*/
    {
        vlan_tag_bit_map[lport/32] |= (1 << (lport%32));
    }
    else
    {
        vlan_tag_bit_map[lport/32] &= (~(uint32)(1 << (lport%32)));
    }
    SetDsVlanTagBitMap(A, tagPortBitMap_f, &ds_vlan_tag_bit_map, vlan_tag_bit_map);
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_tag_bit_map));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_vlan_set_tagged_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint16 vlan_ptr = 0;
    uint32 ds_vlan_tag_bit_map[SYS_VLAN_TAG_BITMAP_NUM] = {0};    /*Use Macro for TM*/

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_LOCK(lchip);
    vlan_ptr = vlan_id;

    index = vlan_ptr;       /*Modified for TM*/
    sal_memcpy(ds_vlan_tag_bit_map, port_bitmap, sizeof(uint32) * MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM) / 32);
    cmd = DRV_IOW(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_tag_bit_map));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}


int32
sys_usw_vlan_get_tagged_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{
    uint16 vlan_ptr = 0;
    uint32 cmd = 0;
    uint32 index = 0;
    bool   ret = FALSE;
    DsVlanTagBitMap_m ds_vlan_tag_bitmap;

    sal_memset(&ds_vlan_tag_bitmap, 0, sizeof(DsVlanTagBitMap_m));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    ret = sys_usw_chip_is_local(lchip, gchip);
    if (FALSE == ret)
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
		return CTC_E_INVALID_CHIP_ID;

    }
    CTC_PTR_VALID_CHECK(port_bitmap);

    sal_memset(port_bitmap, 0, sizeof(ctc_port_bitmap_t));

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "Get tagged ports of vlan:%d!\n", vlan_id);

    SYS_VLAN_LOCK(lchip);

    vlan_ptr = vlan_id;

    index = vlan_ptr;       /*Modified for TM*/
    cmd = DRV_IOR(DsVlanTagBitMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_tag_bitmap));

    sal_memcpy(port_bitmap, &ds_vlan_tag_bitmap, sizeof(DsVlanTagBitMap_m));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
@brief To remove member port from a vlan
*/
int32
sys_usw_vlan_remove_port(uint8 lchip, uint16 vlan_id, uint32 gport)
{
    uint16 lport = 0;
    uint16 vlan_ptr = 0;
    uint32 cmd = 0;
    uint8 field_id = 0;
    uint32 index = 0;
    uint32 value = 0;
    bool ret = FALSE;
    ctc_l2dflt_addr_t l2dflt_addr;

    DsSrcVlanStatus_m ds_src_vlan_status;
    DsDestVlanStatus_m ds_dest_vlan_status;
    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
    sal_memset(&ds_src_vlan_status, 0, sizeof(DsSrcVlanStatus_m));
    sal_memset(&ds_dest_vlan_status, 0, sizeof(DsDestVlanStatus_m));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "Remove global port:%d from vlan:%d!\n", gport, vlan_id);

    vlan_ptr = vlan_id;

    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    if (!CTC_IS_LINKAGG_PORT(gport))
    {
        ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport));
        if (FALSE == ret)
        {
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
			return CTC_E_INVALID_CHIP_ID;

        }
    }
    lport  = (lport > 255)? ((lport-256)&0xFF) : (lport&0xFF);
    SYS_VLAN_FILTER_PORT_CHECK(lport);

    SYS_VLAN_LOCK(lchip);
    index = vlan_ptr;       /*Modified for TM*/

    field_id =  lport;
    if (0 == _sys_usw_vlan_is_member_port(lchip, field_id, index))
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port is not exist \n");
        SYS_VLAN_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    /*write hw */
    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan_status));
    DRV_SET_FIELD_V(lchip, DsSrcVlanStatus_t, field_id, &ds_src_vlan_status, 0);
    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_src_vlan_status));

    cmd = DRV_IOR(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan_status));
    DRV_SET_FIELD_V(lchip, DsDestVlanStatus_t, field_id, &ds_dest_vlan_status, 0);
    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_dest_vlan_status));

    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &value));

    if(!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    if ((p_usw_vlan_master[lchip]->vlan_mode == CTC_VLANPTR_MODE_USER_DEFINE1) && (p_usw_vlan_master[lchip]->sys_vlan_def_mode == 1))
    {
        l2dflt_addr.fid = (uint16)value;
        l2dflt_addr.member.mem_port = gport;
        sys_usw_l2_remove_port_from_default_entry(lchip, &l2dflt_addr);
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

/**
@brief To remove member ports from a vlan
*/
int32
sys_usw_vlan_remove_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    uint32 cmd = 0;
    uint32 gport = 0;
    uint16 index = 0;
    uint8 gchip = 0;
    uint16 lport = 0;
    uint16 loop1 = 0;
    uint16 loop2 = 0;
    uint16 vlan_ptr = 0;
    uint32 value = 0;
    uint32 ds_vlan_status[SYS_VLAN_TAG_BITMAP_NUM] = {0};     /*Use Macro for TM*/
    ctc_l2dflt_addr_t l2dflt_addr;
    ctc_port_bitmap_t r_port_bitmap = {0};

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    if(!sal_memcmp(port_bitmap, r_port_bitmap, sizeof(ctc_port_bitmap_t)))       /*Modified for TM*/
    {
        return CTC_E_NONE;
    }

    SYS_VLAN_LOCK(lchip);

    vlan_ptr = vlan_id;

    index = vlan_ptr;       /*Modified for TM*/

    /*write hw */
    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    for (loop1 = 0; loop1 < (MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32); loop1++)       /*Modified for TM*/
    {
        ds_vlan_status[loop1] &= (~port_bitmap[loop1]);
    }

    cmd = DRV_IOW(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    cmd = DRV_IOR(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    for (loop1 = 0; loop1 < (MCHIP_CAP(SYS_CAP_VLAN_STATUS_NUM)/32); loop1++)       /*Modified for TM*/
    {
        ds_vlan_status[loop1] &= (~port_bitmap[loop1]);
    }

    cmd = DRV_IOW(DsDestVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status));

    if (!IS_BIT_SET(p_usw_vlan_master[lchip]->vlan_def_bitmap[(vlan_ptr >> 5)], (vlan_ptr & 0x1F)))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    /*remove mem_port one by one from vlan_default_entry*/
    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &value));
    if ((p_usw_vlan_master[lchip]->vlan_mode != CTC_VLANPTR_MODE_USER_DEFINE1) || (p_usw_vlan_master[lchip]->sys_vlan_def_mode != 1))
    {
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    sys_usw_get_gchip_id(lchip, &gchip);
    for (loop1 = 0; loop1 < 2; loop1++)
    {
        if (port_bitmap[loop1] == 0)
        {
            continue;
        }
        for (loop2 = 0; loop2 < 32; loop2++)
        {
            lport = loop1 * 32 + loop2;
            if (CTC_BMP_ISSET(port_bitmap, lport))
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
                l2dflt_addr.fid = (uint16)value;
                l2dflt_addr.member.mem_port = gport;
                sys_usw_l2_remove_port_from_default_entry(lchip, &l2dflt_addr);
            }
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

/**
@brief show vlan's member ports
*/
int32
sys_usw_vlan_get_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{
    uint16 vlan_ptr = 0;
    uint32 cmd = 0;
    uint32 index = 0;
    bool   ret = FALSE;
    DsSrcVlanStatus_m ds_vlan_status1;

    sal_memset(&ds_vlan_status1, 0, sizeof(DsSrcVlanStatus_m));

    SYS_VLAN_INIT_CHECK();
    ret = sys_usw_chip_is_local(lchip, gchip);
    if (FALSE == ret)
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
		return CTC_E_INVALID_CHIP_ID;

    }

    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(port_bitmap);

    sal_memset(port_bitmap, 0, sizeof(ctc_port_bitmap_t));

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "Get member ports of vlan:%d!\n", vlan_id);

    SYS_VLAN_LOCK(lchip);
    vlan_ptr = vlan_id;

    index = vlan_ptr;       /*Modified for TM*/
    cmd = DRV_IOR(DsSrcVlanStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &ds_vlan_status1));

    sal_memcpy(port_bitmap, &ds_vlan_status1, sizeof(DsSrcVlanStatus_m));

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_set_mac_auth(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint32   fid = 0;
    uint32   cmd = 0;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_LOCK(lchip);
    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &fid));

    CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_l2_default_entry_set_mac_auth(lchip, fid, enable));

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_mac_auth(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32   fid = 0;
    uint32   cmd = 0;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(enable);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_LOCK(lchip);

    cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
    CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &fid));

    CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_l2_default_entry_get_mac_auth(lchip, fid, enable));

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_dump_status(uint8 lchip)
{

    uint16 loop,loop2 = 0;
    uint32 vlan_bitmap ,vlan_def_bitmap = 0;
    uint32 dir = CTC_INGRESS;
    LCHIP_CHECK(lchip);
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Vlan Mode :%s\n",p_usw_vlan_master[lchip]->vlan_mode ? "UserVlanPtr":"VlanID" );

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s %-8s\n", "VlanId", "Default-Entry");
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------\n");
    for(loop = 0;loop < 128;loop++)
    {
       vlan_bitmap = p_usw_vlan_master[lchip]->vlan_bitmap[loop];
       vlan_def_bitmap =  p_usw_vlan_master[lchip]->vlan_def_bitmap[loop];
       for(loop2= 0;loop2< 32;loop2++)
       {
          if(!CTC_IS_BIT_SET(vlan_bitmap,loop2))
          {
             continue;
          }
           SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12d %-8u \n",(loop*32)+loop2, CTC_IS_BIT_SET(vlan_def_bitmap,loop2));
       }
    }

      dir = CTC_INGRESS;
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SrcVlan Profile:\n" );
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s%s\n", "ProfileId","refCnt");
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------\n");
      ctc_spool_traverse(p_usw_vlan_master[lchip]->vprofile_spool, (spool_traversal_fn)_sys_usw_vlan_profile_dump, &dir);

      dir = CTC_EGRESS;
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DestVlan Profile:\n" );
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s%s\n", "ProfileId","refCnt");
      SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------\n");
      ctc_spool_traverse(p_usw_vlan_master[lchip]->vprofile_spool, (spool_traversal_fn)_sys_usw_vlan_profile_dump, &dir);


    if(p_usw_vlan_master[lchip]->vni_mapping_hash->count != 0)
    {
       SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s%-12s%-12s\n", "vni", "fid","refCnt");
       SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------\n");

       ctc_hash_traverse(p_usw_vlan_master[lchip]->vni_mapping_hash,(hash_traversal_fn)_sys_usw_vlan_vni_mapping_dump, NULL);
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

/**
@brief    Config DsVlan DsVsi DsVlanProfile properties for sys l3if mirror security stp model
*/
int32
sys_usw_vlan_set_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32 value)
{
    LCHIP_CHECK(lchip);
    SYS_VLAN_INIT_CHECK();

    SYS_VLAN_LOCK(lchip);

    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_internal_property(lchip, vlan_id, vlan_prop, value));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
@brief    Get DsVlan DsVsi DsVlanProfile properties
*/
int32
sys_usw_vlan_get_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32* value)
{
    LCHIP_CHECK(lchip);
    SYS_VLAN_INIT_CHECK();

    SYS_VLAN_LOCK(lchip);

    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_internal_property(lchip, vlan_id, vlan_prop, value));

    SYS_VLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
@brief    Config  vlan properties
*/
int32
sys_usw_vlan_set_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32 value)
{
    uint32 cmd = 0;
    uint16 vlan_ptr = 0;
    uint32 index = 0;
    uint32 field_id = 0;
    ctc_l2dflt_addr_t def;
    sal_memset(&def, 0, sizeof(def));

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "vlan_id :%d, vlan_prop:%d, value:%d \n", vlan_id, vlan_prop, value);

    if (CTC_VLAN_PROP_FID == vlan_prop)
    {
        /* change for lock */
        SYS_L2_FID_CHECK(value);
    }

    SYS_VLAN_LOCK(lchip);
    vlan_ptr = vlan_id;

    index = vlan_ptr;
    switch (vlan_prop)
    {
    /* vlan proprety for ctc layer*/
    case CTC_VLAN_PROP_RECEIVE_EN:
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_receiveDisable_f);
        value = (value) ? 0 : 1;
        break;

    case CTC_VLAN_PROP_BRIDGE_EN:
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_bridgeDisable_f);
        value = (value) ? 0 : 1;
        break;

    case CTC_VLAN_PROP_TRANSMIT_EN:
        cmd = DRV_IOW(DsDestVlan_t, DsDestVlan_transmitDisable_f);
        value = (value) ? 0 : 1;
        break;

    case CTC_VLAN_PROP_LEARNING_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_vsiLearningDisable_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOW(DsVsi_t, field_id);
        value = (value) ? 0 : 1;
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DHCP_EXCP_TYPE:
        if (value > CTC_EXCP_DISCARD)
        {
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_dhcpV4ExceptionType_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value));
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_dhcpV6ExceptionType_f);
        break;

    case CTC_VLAN_PROP_ARP_EXCP_TYPE:
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_arpExceptionType_f);
        if (value > CTC_EXCP_DISCARD)
        {
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_VLAN_PROP_IGMP_SNOOP_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_igmpSnoopEn_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOW(DsVsi_t, field_id);
        value = (value) ? 1 : 0;
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_ucastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOW(DsVsi_t, field_id);
        value = (value) ? 1 : 0;
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_mcastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOW(DsVsi_t, field_id);
        value = (value) ? 1 : 0;
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_bcastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOW(DsVsi_t, field_id);
        value = (value) ? 1 : 0;
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_FID:
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        break;

    case CTC_VLAN_PROP_IPV4_BASED_L2MC:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_FORCE_IPV4LOOKUP, value));
        break;

    case CTC_VLAN_PROP_IPV6_BASED_L2MC:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_FORCE_IPV6LOOKUP, value));

        break;
    case CTC_VLAN_PROP_FIP_EXCP_TYPE:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_FIP_EXCEPTION_TYPE, value));
        break;
    case CTC_VLAN_PROP_CID:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_CID, value));
        break;
    case CTC_VLAN_PROP_PIM_SNOOP_EN:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_PIM_SNOOP_EN, value));
        break;
    case CTC_VLAN_PROP_PTP_EN:
        if (value > SYS_VLAN_PTP_CLOCK_ID_MAX)
        {
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_PTP_CLOCK_ID, value));
        break;
    case CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_l2_set_fid_property(lchip, index, CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU, value));
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    default:
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    if(cmd != 0)
    {
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value));
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32* value)
{
    uint32 cmd = 0;
    uint16 vlan_ptr = 0;
    uint32 index = 0;
    uint32 field_id = 0;
    bool vlan_profile_prop = FALSE;

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(value);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "vlan_id :%d vlan_prop:%d \n", vlan_id, vlan_prop);

    SYS_VLAN_LOCK(lchip);
    vlan_ptr = vlan_id;

    /* zero value*/
    *value = 0;
    index = vlan_ptr;

    switch (vlan_prop)
    {
    /* vlan proprety for ctc layer*/
    case CTC_VLAN_PROP_RECEIVE_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_receiveDisable_f);
        break;

    case CTC_VLAN_PROP_BRIDGE_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_bridgeDisable_f);
        break;

    case CTC_VLAN_PROP_TRANSMIT_EN:
        cmd = DRV_IOR(DsDestVlan_t, DsDestVlan_transmitDisable_f);
        break;

    case CTC_VLAN_PROP_LEARNING_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_vsiLearningDisable_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DHCP_EXCP_TYPE:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_dhcpV4ExceptionType_f);
        break;

    case CTC_VLAN_PROP_ARP_EXCP_TYPE:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_arpExceptionType_f);
        break;

    case CTC_VLAN_PROP_IGMP_SNOOP_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_igmpSnoopEn_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_ucastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_mcastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        field_id = DsVsi_array_0_bcastDiscard_f + (index & 0x3)*SYS_VLAN_VSI_PARAM_MAX ;
        cmd = DRV_IOR(DsVsi_t, field_id);
        index = (index >> 2) & 0xFFF;
        break;

    case CTC_VLAN_PROP_FID:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        break;
    case CTC_VLAN_PROP_CID:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_CID, value));
        break;
    case CTC_VLAN_PROP_IPV4_BASED_L2MC:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_FORCE_IPV4LOOKUP, value));
        vlan_profile_prop = TRUE;
        break;

    case CTC_VLAN_PROP_IPV6_BASED_L2MC:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_FORCE_IPV6LOOKUP, value));
        vlan_profile_prop = TRUE;
        break;

    case CTC_VLAN_PROP_FIP_EXCP_TYPE:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_FIP_EXCEPTION_TYPE, value));
        vlan_profile_prop = TRUE;
        break;

    case CTC_VLAN_PROP_PIM_SNOOP_EN:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_PIM_SNOOP_EN, value));
        vlan_profile_prop = TRUE;
        break;

    case CTC_VLAN_PROP_PTP_EN:
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id,CTC_INGRESS, SYS_VLAN_PROP_PTP_CLOCK_ID, value));
        vlan_profile_prop = TRUE;
        break;
    case CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU:
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_vsiId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_ptr, cmd, &index));
        CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_l2_get_fid_property(lchip, index, CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU, value));
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_NONE;
    default:
        SYS_VLAN_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    if (!vlan_profile_prop)
    {
        if(cmd != 0)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, index, cmd, value));
        }
        if (CTC_VLAN_PROP_BRIDGE_EN == vlan_prop || CTC_VLAN_PROP_LEARNING_EN == vlan_prop ||
            CTC_VLAN_PROP_RECEIVE_EN == vlan_prop || CTC_VLAN_PROP_TRANSMIT_EN == vlan_prop)
        {
            *value = !(*value);
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_set_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32  value)
{
    uint32 cmd = 1;
    uint32 prop = 0;
    uint32 acl_en = 0;
    uint32 get_value = 0;
    uint32 stats_ptr = 0;

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
                     "vlan_id :%d, vlan_prop:%d, dir:%d ,value:%d \n", vlan_id, vlan_prop, dir, value);
    if(CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0 == vlan_prop || CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1 == vlan_prop ||
       CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2 == vlan_prop || CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3 == vlan_prop)
    {
        CTC_MAX_VALUE_CHECK(value, CTC_ACL_TCAM_LKUP_TYPE_MAX -1);
        if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == value || CTC_ACL_TCAM_LKUP_TYPE_COPP == value)
        {
            return CTC_E_NOT_SUPPORT;        /* vlan not support, copp only support global configure */
        }
    }
    SYS_VLAN_LOCK(lchip);
    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        switch (vlan_prop)
        {
        case CTC_VLAN_DIR_PROP_ACL_EN:
            SYS_MAX_VALUE_CHECK_WITH_UNLOCK(value, CTC_ACL_EN_0 + CTC_ACL_EN_1 + CTC_ACL_EN_2 + CTC_ACL_EN_3, p_usw_vlan_master[lchip]->mutex);

            acl_en = CTC_FLAG_ISSET(value, CTC_ACL_EN_0);
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, acl_en));

            acl_en = CTC_FLAG_ISSET(value, CTC_ACL_EN_1);
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, acl_en));

            acl_en = CTC_FLAG_ISSET(value, CTC_ACL_EN_2);
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, acl_en));

            acl_en = CTC_FLAG_ISSET(value, CTC_ACL_EN_3);
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, acl_en));
            cmd = 0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID:
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0, &get_value));

            if (get_value)     /* vlan acl enabled */
            {
                if ((value < SYS_USW_MIN_ACL_VLAN_CLASSID) || (value > SYS_USW_MAX_ACL_VLAN_CLASSID))
                {
                    SYS_VLAN_UNLOCK(lchip);
                    return CTC_E_INVALID_PARAM;
                }
                prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL0;
            }
            else
            {
                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Enable ACL on vlan first \n");
                SYS_VLAN_UNLOCK(lchip);
			    return CTC_E_NOT_READY;

            }
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_1:
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1, &get_value));

            if (get_value)     /* vlan acl enabled */
            {
                if ((value < SYS_USW_MIN_ACL_VLAN_CLASSID) || (value > SYS_USW_MAX_ACL_VLAN_CLASSID))
                {
                    SYS_VLAN_UNLOCK(lchip);
                    return CTC_E_INVALID_PARAM;
                }
                prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL1;
            }
            else
            {
                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Enable ACL on vlan first \n");
                SYS_VLAN_UNLOCK(lchip);
			    return CTC_E_NOT_READY;

            }
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_2:
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2, &get_value));

            if (get_value)     /* vlan acl enabled */
            {
                if ((value < SYS_USW_MIN_ACL_VLAN_CLASSID) || (value > SYS_USW_MAX_ACL_VLAN_CLASSID))
                {
                    SYS_VLAN_UNLOCK(lchip);
                    return CTC_E_INVALID_PARAM;
                }
                prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL2;
            }
            else
            {
                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Enable ACL on vlan first \n");
                SYS_VLAN_UNLOCK(lchip);
			    return CTC_E_NOT_READY;

            }
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_3:
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3, &get_value));

            if (get_value)     /* vlan acl enabled */
            {
                if ((value < SYS_USW_MIN_ACL_VLAN_CLASSID) || (value > SYS_USW_MAX_ACL_VLAN_CLASSID))
                {
                    SYS_VLAN_UNLOCK(lchip);
                    return CTC_E_INVALID_PARAM;
                }
                prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL3;
            }
            else
            {
                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Enable ACL on vlan first \n");
                SYS_VLAN_UNLOCK(lchip);
			    return CTC_E_NOT_READY;

            }
            break;

        case CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY:
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            SYS_VLAN_UNLOCK(lchip);
			return CTC_E_NOT_SUPPORT;


        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE1;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE2;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE3;
            break;

        case CTC_VLAN_DIR_PROP_VLAN_STATS_ID:
            if(value != 0)
            {
                CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_flow_stats_get_statsptr(lchip, value, &stats_ptr));
            }
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_STATSPTR, stats_ptr));
            cmd = 0;
            break;
        default:
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }

        if(cmd)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, value));
        }
    }

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        switch (vlan_prop)
        {

        case CTC_VLAN_DIR_PROP_ACL_EN:
            SYS_MAX_VALUE_CHECK_WITH_UNLOCK(value, CTC_ACL_EN_0, p_usw_vlan_master[lchip]->mutex);

            acl_en = CTC_FLAG_ISSET(value, CTC_ACL_EN_0);
            prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_EGRESS, prop, acl_en));
            cmd = 0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID:

            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_EGRESS, SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0, &get_value));

            if (get_value)     /* vlan acl enabled */
            {
                if ((value < SYS_USW_MIN_ACL_VLAN_CLASSID) || (value > SYS_USW_MAX_ACL_VLAN_CLASSID))
                {
                    SYS_VLAN_UNLOCK(lchip);
                    return CTC_E_INVALID_PARAM;
                }
                prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_LABEL0;
            }
            else
            {
                SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Enable ACL on vlan first \n");
                SYS_VLAN_UNLOCK(lchip);
			    return CTC_E_NOT_READY;

            }

            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0;
            break;

        case CTC_VLAN_DIR_PROP_VLAN_STATS_ID:
            if (value != 0)
            {
                CTC_ERROR_RETURN_VLAN_UNLOCK(sys_usw_flow_stats_get_statsptr(lchip, value, &stats_ptr));
            }
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_STATSPTR, stats_ptr));
            cmd = 0;
            break;

        default:
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }

        if(cmd)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_set_vlan_profile(lchip, vlan_id, CTC_EGRESS, prop, value));
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_get_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32* value)
{

    uint32  prop = 0;
    uint32  acl_en = 0;
    uint32  cmd = 1;

    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION - 1);
    CTC_PTR_VALID_CHECK(value);

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                     "vlan_id :%d vlan_prop:%d dir:%d\n", vlan_id, vlan_prop, dir);

    /* zero value*/
    *value = 0;

    SYS_VLAN_LOCK(lchip);

    if (CTC_INGRESS == dir)
    {
        switch (vlan_prop)
        {
        case CTC_VLAN_DIR_PROP_ACL_EN:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile
                                 (lchip, vlan_id, CTC_INGRESS, prop, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*value, CTC_ACL_EN_0);
            }

            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile
                                 (lchip, vlan_id, CTC_INGRESS, prop, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*value, CTC_ACL_EN_1);
            }

            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile
                                 (lchip, vlan_id, CTC_INGRESS, prop, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*value, CTC_ACL_EN_2);
            }

            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile
                                 (lchip, vlan_id, CTC_INGRESS, prop, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*value, CTC_ACL_EN_3);
            }

            cmd = 0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_1:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL1;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_2:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL2;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID_3:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL3;
            break;

        case CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY:
            SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            SYS_VLAN_UNLOCK(lchip);
			return CTC_E_NOT_SUPPORT;


        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE1;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE2;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3:
            prop = SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE3;
            break;

         case CTC_VLAN_DIR_PROP_VLAN_STATS_ID:
             CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_STATSPTR, value));
             cmd = 0;
             break;
        default:
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }

        if (cmd)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_INGRESS, prop, value));
        }
    }

    if (CTC_EGRESS == dir)
    {
        switch (vlan_prop)
        {
        case CTC_VLAN_DIR_PROP_ACL_EN:
            prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0;
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_EGRESS, prop, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*value, CTC_ACL_EN_0);
            }
            cmd = 0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_CLASSID:
            prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_LABEL0;
            break;

        case CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            prop = SYS_VLAN_PROP_EGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0;
            break;

        case CTC_VLAN_DIR_PROP_VLAN_STATS_ID:
             CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_STATSPTR, value));
             cmd = 0;
             break;

        default:
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        if (cmd)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_get_vlan_profile(lchip, vlan_id, CTC_EGRESS, prop, value));
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32 sys_usw_vlan_set_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    uint8  acl_priority = 0;
    uint32 cmd =0;
    uint32 profile_id = 0;
    uint8  sys_lkup_type = 0;
    uint8  step = 0;
    uint8  gport_type = 0;

    sys_vlan_profile_t* p_vlan_profile_get = NULL;
    sys_vlan_profile_t  old_vlan_profile;
    sys_vlan_profile_t  new_vlan_profile;

    sal_memset(&old_vlan_profile, 0, sizeof(sys_vlan_profile_t));
    sal_memset(&new_vlan_profile, 0, sizeof(sys_vlan_profile_t));

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_prop);
    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);

    SYS_ACL_PROPERTY_CHECK(p_prop);
    if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == p_prop->tcam_lkup_type)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP)
        + CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA)
        + CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT) > 1)
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port bitmap Metedata and Logic port can not be configured at the same time \n");
        return CTC_E_NOT_SUPPORT;
    }
    SYS_VLAN_LOCK(lchip);

    acl_priority = p_prop->acl_priority;
    step = DsSrcVlanProfile_gIngressAcl_1_aclLabel_f - DsSrcVlanProfile_gIngressAcl_0_aclLabel_f;
    sys_lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, p_prop->tcam_lkup_type);
    if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP) ||
        CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN))
    {
        SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        SYS_VLAN_UNLOCK(lchip);
		return CTC_E_NOT_SUPPORT;

    }
    else if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
        gport_type = DRV_FLOWPORTTYPE_BITMAP;
    }
    else if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        gport_type = DRV_FLOWPORTTYPE_METADATA;
    }
    else if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
        gport_type = DRV_FLOWPORTTYPE_LPORT;
    }
    else
    {
        gport_type = DRV_FLOWPORTTYPE_GPORT;
    }

    if(CTC_INGRESS == p_prop->direction)
    {
        cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsSrcVlanProfile_t,DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_INGRESS;
        old_vlan_profile.lchip = lchip;
        old_vlan_profile.profile_id = profile_id;

        sal_memcpy(&new_vlan_profile,&old_vlan_profile,sizeof(sys_vlan_profile_t));

        if(0 == p_prop->acl_en)
        {
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, 0);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, 0);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclUseGlobalPortType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, 0);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclUsePIVlan_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, 0);
        }
        else
        {
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, sys_lkup_type);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, p_prop->class_id);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclUseGlobalPortType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile, gport_type);
            SetDsSrcVlanProfile(V,gIngressAcl_0_aclUsePIVlan_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile,
                                                CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);

        }
    }
    else
    {
        cmd = DRV_IOR(DsDestVlan_t, DsDestVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
        cmd = DRV_IOR(DsDestVlanProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&old_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile));
        old_vlan_profile.sw_vlan_profile.dir = CTC_EGRESS;
        old_vlan_profile.lchip = lchip;
        old_vlan_profile.profile_id = profile_id;
        sal_memcpy(&new_vlan_profile,&old_vlan_profile,sizeof(sys_vlan_profile_t));

        if(0 == p_prop->acl_en)
        {
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, 0);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, 0);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclUseGlobalPortType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, 0);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclUsePIVlan_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, 0);
        }
        else
        {
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, sys_lkup_type);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, p_prop->class_id);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclUseGlobalPortType_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile, gport_type);
            SetDsDestVlanProfile(V,gEgressAcl_0_aclUsePIVlan_f + step*acl_priority,
                                    &new_vlan_profile.sw_vlan_profile.vp.dest_vlan_profile,
                                            CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        }
    }

    /*write hardware*/
    if(CTC_INGRESS == p_prop->direction && (old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->ingress_vlan_policer_num ||
            old_vlan_profile.profile_id == 0))
    {
        CTC_ERROR_RETURN_VLAN_UNLOCK(ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool,&new_vlan_profile, &old_vlan_profile, &p_vlan_profile_get));
        profile_id = p_vlan_profile_get->profile_id;
        cmd = DRV_IOW(DsSrcVlan_t, DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
    }
    else if(CTC_EGRESS == p_prop->direction && (old_vlan_profile.profile_id > p_usw_vlan_master[lchip]->egress_vlan_policer_num ||
            old_vlan_profile.profile_id == 0))
    {
        CTC_ERROR_RETURN_VLAN_UNLOCK(ctc_spool_add(p_usw_vlan_master[lchip]->vprofile_spool,&new_vlan_profile, &old_vlan_profile, &p_vlan_profile_get));
        profile_id = p_vlan_profile_get->profile_id;
        cmd = DRV_IOW(DsDestVlan_t, DsDestVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));
    }
    /* vlan policer profile not change profile */
    else
    {
        profile_id = old_vlan_profile.profile_id;
    }

    if(CTC_INGRESS == p_prop->direction)
    {
        if(profile_id)
        {
            cmd = DRV_IOW(DsSrcVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        }
    }
    else if(CTC_EGRESS == p_prop->direction)
    {
        if(profile_id)
        {
            cmd = DRV_IOW(DsDestVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &new_vlan_profile.sw_vlan_profile.vp.src_vlan_profile));
        }
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32 sys_usw_vlan_get_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    uint32 cmd  =0;
    uint8  step = 0;
    uint8  acl_priority = 0;
    uint8  sys_lkup_type = 0;
    uint32 profile_id = 0;
    uint8  gport_type = 0;
    uint8  use_mapped_vlan = 0;
    sys_sw_vlan_profile_t vlan_profile;

    SYS_VLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_prop);
    SYS_VLAN_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);
    SYS_VLAN_CREATE_CHECK(vlan_id);
    CTC_MAX_VALUE_CHECK(p_prop->direction, CTC_EGRESS);

    SYS_VLAN_LOCK(lchip);
    acl_priority = p_prop->acl_priority;
    step = DsSrcVlanProfile_gIngressAcl_1_aclLabel_f - DsSrcVlanProfile_gIngressAcl_0_aclLabel_f;
    if(CTC_INGRESS == p_prop->direction)
    {
        SYS_MAX_VALUE_CHECK_WITH_UNLOCK(acl_priority, MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1, p_usw_vlan_master[lchip]->mutex);
        cmd = DRV_IOR(DsSrcVlan_t,DsSrcVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));

        if(profile_id)
        {
            cmd = DRV_IOR(DsSrcVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&vlan_profile.vp.src_vlan_profile));
            sys_lkup_type =
                GetDsSrcVlanProfile(V,gIngressAcl_0_aclLookupType_f + step * acl_priority,&vlan_profile.vp.src_vlan_profile);
            p_prop->class_id =
                GetDsSrcVlanProfile(V,gIngressAcl_0_aclLabel_f + step * acl_priority,&vlan_profile.vp.src_vlan_profile);
            gport_type =
                GetDsSrcVlanProfile(V,gIngressAcl_0_aclUseGlobalPortType_f + step * acl_priority,&vlan_profile.vp.src_vlan_profile);
            use_mapped_vlan =
                GetDsSrcVlanProfile(V,gIngressAcl_0_aclUsePIVlan_f + step * acl_priority,&vlan_profile.vp.src_vlan_profile);
        }
    }
    else
    {
        SYS_MAX_VALUE_CHECK_WITH_UNLOCK(acl_priority, MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1, p_usw_vlan_master[lchip]->mutex);
        cmd = DRV_IOR(DsDestVlan_t,DsDestVlan_profileId_f);
        CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip, vlan_id, cmd, &profile_id));

        if(profile_id)
        {
            cmd = DRV_IOR(DsDestVlanProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_VLAN_UNLOCK(DRV_IOCTL(lchip,profile_id,cmd,&vlan_profile.vp.dest_vlan_profile));
            sys_lkup_type =
                GetDsDestVlanProfile(V,gEgressAcl_0_aclLookupType_f + step * acl_priority,&vlan_profile.vp.dest_vlan_profile);
            p_prop->class_id =
                GetDsDestVlanProfile(V,gEgressAcl_0_aclLabel_f + step * acl_priority,&vlan_profile.vp.dest_vlan_profile);
            gport_type =
                GetDsDestVlanProfile(V,gEgressAcl_0_aclUseGlobalPortType_f + step * acl_priority,&vlan_profile.vp.dest_vlan_profile);
            use_mapped_vlan =
                GetDsDestVlanProfile(V,gEgressAcl_0_aclUsePIVlan_f + step * acl_priority,&vlan_profile.vp.dest_vlan_profile);
        }
    }

    p_prop->acl_en = sys_lkup_type ? 1:0;
    p_prop->tcam_lkup_type = sys_usw_unmap_acl_tcam_lkup_type(lchip, sys_lkup_type);
    if(use_mapped_vlan)
    {
        CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }

    switch(gport_type)
    {
        case DRV_FLOWPORTTYPE_BITMAP:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
            break;
        case DRV_FLOWPORTTYPE_LPORT:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
            break;
        case DRV_FLOWPORTTYPE_METADATA:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA);
            break;
        default:
            break;
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_overlay_get_vni_fid_mapping_mode(uint8 lchip, uint8* mode)
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);
    *mode = p_usw_vlan_master[lchip]->vni_mapping_mode;
    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_overlay_set_vni_fid_mapping_mode(uint8 lchip, uint8 mode)
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);
    p_usw_vlan_master[lchip]->vni_mapping_mode = mode;
    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_overlay_remove_fid (uint8 lchip,  uint32 vn_id)
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_remove_fid (lchip, vn_id));
    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_overlay_get_vn_id (uint8 lchip,  uint16 fid, uint32* p_vn_id)
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_get_vn_id(lchip, fid, p_vn_id));
    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_overlay_get_fid (uint8 lchip,  uint32 vn_id, uint16* p_fid )
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);
    CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_get_fid (lchip, vn_id, p_fid));
    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_vlan_overlay_set_fid (uint8 lchip,  uint32 vn_id, uint16 fid )
{
    SYS_VLAN_INIT_CHECK();
    SYS_VLAN_LOCK(lchip);

    if (!p_usw_vlan_master[lchip]->vni_mapping_mode)
    {
        /* mode 0, vni_id : fid = 1 : 1 */
        if (0 == fid)
        {
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_remove_fid(lchip, vn_id));
        }
        else
        {
            /* mode 1 will not call the function */
            CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_add_fid(lchip, vn_id, fid));
        }
    }
    else
    {
        /* mode 1, fid : vni_id = N : 1 */
        if (fid == 0)
        {
            SYS_VLAN_UNLOCK(lchip);
            return CTC_E_BADID;
        }
        CTC_ERROR_RETURN_VLAN_UNLOCK(_sys_usw_vlan_overlay_set_fid_hw(lchip, fid, vn_id));
    }

    SYS_VLAN_UNLOCK(lchip);
    return CTC_E_NONE;
}



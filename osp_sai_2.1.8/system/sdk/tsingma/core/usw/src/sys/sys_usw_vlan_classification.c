/**
 @file sys_usw_vlan_classification.c

 @date 2009-12-30

 @version v2.0
*/
#include "sal.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "ctc_warmboot.h"
#include "sys_usw_ftm.h"
#include "sys_usw_vlan_classification.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_register.h"

#include "drv_api.h"
/******************************************************************************
*
*   Macros, Globals and Defines
*
*******************************************************************************/
#define IS_MAC_ZERO(mac)   (0 == (mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]))
#define IS_MAC_ALLF(mac) \
    ((0xFF == mac[0]) && (0xFF == mac[1]) && (0xFF == mac[2]) \
     && (0xFF == mac[3]) && (0xFF == mac[4]) && (0xFF == mac[5]))
#define IS_IPV6_ZERO(ip)   (0 == (ip[0] + ip[1] + ip[2] + ip[3]))
#define IS_IPV6_ALLF(ip)   ((0xFFFFFFFF == ip[0]) && (0xFFFFFFFF == ip[1]) && (0xFFFFFFFF == ip[2]) && (0xFFFFFFFF == ip[3]))

struct sys_vlan_class_protocol_vlan_s
{
    uint16 vlan_id;
    uint8 cos;
    uint8 cos_valid;
};
typedef struct sys_vlan_class_protocol_vlan_s sys_vlan_class_protocol_vlan_t;

typedef struct
{
    uint16  ether_type[8];
    uint32  mac_eid[6]; /*default entry for userid0,1,2,3 and flex tcam default entry for 0 1*/
    uint32  ipv4_eid[6];
    uint32  ipv6_eid[6];
    sal_mutex_t* mutex;
}sys_vlan_class_master_t;

static sys_vlan_class_master_t* p_usw_vlan_class_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

enum sys_vlan_class_group_sub_type_e
{
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH1_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH1_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH1_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH1_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH1_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH1_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM1_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM2_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM3_MAC,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM1_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM2_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM3_IPV4,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM1_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM2_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM3_IPV6,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_DEFAULT,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM1_DEFAULT,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM2_DEFAULT,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM3_DEFAULT,
    SYS_VLAN_CLASS_GROUP_SUB_TYPE_NUM
};
typedef enum sys_vlan_class_group_sub_type_e sys_vlan_class_group_sub_type_t;

#define SYS_VLAN_CLASS_INIT_CHECK()  \
    do                                      \
    {   LCHIP_CHECK(lchip);                 \
        if (NULL == p_usw_vlan_class_master[lchip]) \
        {                                   \
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
          \
        }                                   \
    } while (0)                               \

#define SYS_VLAN_CLASS_CREATE_LOCK(lchip) \
    do \
    { \
        sal_mutex_create(&p_usw_vlan_class_master[lchip]->mutex); \
        if (NULL == p_usw_vlan_class_master[lchip]->mutex) \
        { \
            return CTC_E_NO_MEMORY; \
        } \
    }while (0)

#define SYS_VLAN_CLASS_LOCK(lchip) \
    sal_mutex_lock(p_usw_vlan_class_master[lchip]->mutex)

#define SYS_VLAN_CLASS_UNLOCK(lchip) \
    sal_mutex_unlock(p_usw_vlan_class_master[lchip]->mutex)

#define CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_vlan_class_master[lchip]->mutex); \
            return rv; \
        } \
    }while (0)


#define SYS_VLAN_CLASS_DBG_OUT(level, FMT, ...)                                         \
    do                                                                                  \
    {                                                                                   \
        CTC_DEBUG_OUT(vlan, vlan_class, VLAN_CLASS_SYS, level, FMT, ##__VA_ARGS__);     \
    } while (0)

#define SYS_VLAN_CLASS_SET_MAC_HIGH(dest_h, src)                     \
    do                                                                   \
    {                                                                    \
        dest_h = ((src[0] << 8) | (src[1]));                                  \
    } while (0)                                                            \


#define SYS_VLAN_CLASS_SET_MAC_LOW(dest_l, src)                      \
    do                                                                   \
    {                                                                    \
        dest_l = ((src[2] << 24) | (src[3] << 16) | (src[4] << 8) | (src[5])); \
    } while (0)                                                            \


#define SYS_VLAN_CLASS_IS_ALL_ONE(mask, is_all_one)     \
    do                                                  \
    {                                                   \
        int i = 0;                                      \
        for (i = 0; i < sizeof(mac_addr_t); i++)        \
        {                                               \
            if (0xFF != mask[i])                        \
            {                                           \
                is_all_one = FALSE;                     \
                break;                                  \
            }                                           \
        }                                               \
        is_all_one = TRUE;                              \
    } while (0)


#define SYS_VLAN_CLASS_IS_ALL_ZERO(mask, is_all_zero)   \
    do                                                  \
    {                                                   \
        int i = 0;                                      \
        for (i = 0; i < sizeof(mac_addr_t); i++)        \
        {                                               \
            if (0 != mask[i])                           \
            {                                           \
                is_all_zero = FALSE;                    \
                break;                                  \
            }                                           \
        }                                               \
        is_all_zero = TRUE;                             \
    } while (0)


#define SYS_VLAN_CLASS_INFO_MAC(str, mac)               \
    do                                                  \
    {                                                   \
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  %s  :%02X%02X-%02X%02X-%02X%02X\n", \
            str, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); \
    } while (0)

#define SYS_VLAN_CLASS_INFO_IPV4(str, ipv4sa)           \
    do                                                  \
    {                                                   \
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  %s :%d.%d.%d.%d\n", \
            str, (ipv4sa >> 24) & 0xFF, (ipv4sa >> 16) & 0xFF, (ipv4sa >> 8) & 0xFF, ipv4sa & 0xFF); \
    } while (0)


#define SYS_VLAN_CLASS_INFO_IPV6(str, ipv6sa)           \
    do                                                  \
    {                                                   \
    } while (0)
#define VSET_PROTOVLAN(fld, step, ds, value)     DRV_SET_FIELD_V(lchip, DsProtocolVlan_t, (DsProtocolVlan_##fld) + step, ds, value)

#define SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, feature, gtype, ret) \
        gid = SYS_SCL_ENCODE_INNER_GRP_ID(feature, (gtype), 0, 0, 0); \
        ret = sys_usw_scl_uninstall_group(lchip, gid, 1); \
        if (CTC_E_NONE == ret) \
        {\
            CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_remove_all_entry(lchip, gid, 1)); \
        }\
        else if (CTC_E_NOT_EXIST == ret)\
        {\
            \
        }\
        else\
        {\
            SYS_VLAN_CLASS_UNLOCK(lchip); \
            return ret; \
        }


/******************************************************************************
*
*   Functions
*
*******************************************************************************/

/******************************************************************************
*
*   protocol vlan
*******************************************************************************/
STATIC int32
_sys_usw_protocol_vlan_add(uint8 lchip, uint16 ether_type, sys_vlan_class_protocol_vlan_t* pro_vlan)
{
    uint32 cmd = 0;
    DsProtocolVlan_m ds_protocol_vlan;
    int32 distance = DsProtocolVlan_array_1_protocolVlanIdValid_f - DsProtocolVlan_array_0_protocolVlanIdValid_f;
    int32 distance2 = DsProtocolVlan_protocolEtherType1_f - DsProtocolVlan_protocolEtherType0_f;

    bool free = FALSE;
    uint8 first_free = 0;
    uint8 idx = 0;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(ether_type < 0x0600)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ds_protocol_vlan, 0, sizeof(DsProtocolVlan_m));

    for (idx = 0; idx < 8; idx++)
    {

        if (((0 == p_usw_vlan_class_master[lchip]->ether_type[idx])
             || (ether_type == p_usw_vlan_class_master[lchip]->ether_type[idx])) && (free == FALSE))
        {
            first_free = idx;
            free = TRUE;
        }
        else if ((ether_type == p_usw_vlan_class_master[lchip]->ether_type[idx]) && (free == TRUE))
        {
            first_free = idx;
            break;
        }
    }

    if (!free)
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

    }
    else
    {
        p_usw_vlan_class_master[lchip]->ether_type[first_free] = ether_type;

        cmd = DRV_IOR(DsProtocolVlan_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_protocol_vlan));

        VSET_PROTOVLAN(array_0_protocolVlanIdValid_f ,(distance * first_free), &ds_protocol_vlan, 1);
        VSET_PROTOVLAN(array_0_protocolVlanId_f      ,(distance * first_free), &ds_protocol_vlan, pro_vlan->vlan_id);
        VSET_PROTOVLAN(array_0_protocolCosValid_f    ,(distance * first_free), &ds_protocol_vlan, pro_vlan->cos_valid);
        VSET_PROTOVLAN(array_0_protocolCos_f         ,(distance * first_free), &ds_protocol_vlan, pro_vlan->cos);
        VSET_PROTOVLAN(array_0_protocolCfi_f         ,(distance * first_free), &ds_protocol_vlan, 0);
        VSET_PROTOVLAN(array_0_cpuExceptionEn_f      ,(distance * first_free), &ds_protocol_vlan, 0);
        VSET_PROTOVLAN(array_0_discard_f             ,(distance * first_free), &ds_protocol_vlan, 0);
        VSET_PROTOVLAN(array_0_protocolReplaceTagEn_f,(distance * first_free), &ds_protocol_vlan, 1);
        VSET_PROTOVLAN(array_0_protocolAddTagEn_f    ,(distance * first_free), &ds_protocol_vlan, 0); /*no use*/
        VSET_PROTOVLAN(array_0_outerVlanStatus_f     ,(distance * first_free), &ds_protocol_vlan, 2); /*svlan domain*/
        VSET_PROTOVLAN(array_0_svlanTpidIndex_f      ,(distance * first_free), &ds_protocol_vlan, 0);
        VSET_PROTOVLAN(protocolEtherType0_f          ,(distance2 * first_free),&ds_protocol_vlan, ether_type);

        cmd = DRV_IOW(DsProtocolVlan_t, DRV_ENTRY_FLAG);



            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_protocol_vlan));

    }

    return CTC_E_NONE;
}

int32
_sys_usw_protocol_vlan_remove(uint8 lchip, uint16 ether_type)
{
    uint32 cmd = 0;
    uint8 index = 0;


    int32 distance = DsProtocolVlan_array_1_protocolVlanIdValid_f - DsProtocolVlan_array_0_protocolVlanIdValid_f;
    int32 distance2 = DsProtocolVlan_protocolEtherType1_f - DsProtocolVlan_protocolEtherType0_f;
    DsProtocolVlan_m ds_protocol_vlan;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for (index = 0; index < 8; index++)
    {
        if (ether_type == p_usw_vlan_class_master[lchip]->ether_type[index])
        {
            break;
        }
    }

    if (index >= 8)
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO: ether_type :%d none-exist.\n", ether_type);
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    sal_memset(&ds_protocol_vlan, 0, sizeof(ds_protocol_vlan));
    cmd = DRV_IOR(DsProtocolVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_protocol_vlan));

    VSET_PROTOVLAN( array_0_protocolVlanIdValid_f , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolVlanId_f      , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolCosValid_f    , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolCos_f         , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolCfi_f         , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_cpuExceptionEn_f      , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_discard_f             , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolReplaceTagEn_f, (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_protocolAddTagEn_f    , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_outerVlanStatus_f     , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN( array_0_svlanTpidIndex_f      , (distance * index) ,&ds_protocol_vlan, 0);
    VSET_PROTOVLAN(protocolEtherType0_f           , (distance2 * index),&ds_protocol_vlan, 0);


    cmd = DRV_IOW(DsProtocolVlan_t, DRV_ENTRY_FLAG);


        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_protocol_vlan));


    p_usw_vlan_class_master[lchip]->ether_type[index] = 0;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_class_get_scl_gid(uint8 lchip, uint8 type, uint8 resolve_conflict, uint8 block_id, uint32* gid)
{
    uint8 scl_id = 0;
    CTC_PTR_VALID_CHECK(gid);
    CTC_MAX_VALUE_CHECK(block_id, MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)-1);

    if (MCHIP_CAP(SYS_CAP_SCL_HASH_NUM) > block_id)
    {
        scl_id = block_id;
    }

    switch (type)
    {
    case SYS_SCL_KEY_HASH_MAC:
        if (!resolve_conflict)
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_MAC + scl_id), 0, 0, 0);
        }
        else
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_MAC + block_id), 0, 0, 0);
        }
        break;
    case SYS_SCL_KEY_HASH_IPV4:
        if (!resolve_conflict)
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV4 + scl_id), 0, 0, 0);
        }
        else
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV4 + block_id), 0, 0, 0);
        }
        break;
    case SYS_SCL_KEY_HASH_IPV6:
        if (!resolve_conflict)
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV6 + scl_id), 0, 0, 0);
        }
        else
        {
            *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV6 + block_id), 0, 0, 0);
        }
        break;
    case SYS_SCL_KEY_TCAM_MAC:
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_MAC + block_id), 0, 0, 0) ;
        break;
    case SYS_SCL_KEY_TCAM_IPV4_SINGLE:
    case SYS_SCL_KEY_TCAM_IPV4:
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV4 + block_id), 0, 0, 0) ;
        break;
    case SYS_SCL_KEY_TCAM_IPV6_SINGLE:
    case SYS_SCL_KEY_TCAM_IPV6:
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV6 + block_id), 0, 0, 0) ;
        break;
    default:
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_class_build_key(uint8 lchip, ctc_vlan_class_t* p_vlan_class_in, ctc_scl_entry_t* ctc, sys_scl_lkup_key_t* p_lkup_key, uint8 is_add)
{
    uint8   is_tcp   = 0;
    uint8   is_udp   = 0;
    ctc_field_key_t field_key;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    CTC_PTR_VALID_CHECK(p_vlan_class_in);
    if(is_add)
    {
        CTC_PTR_VALID_CHECK(ctc);
    }
    else
    {
        CTC_PTR_VALID_CHECK(p_lkup_key);
    }

    switch (p_vlan_class_in->type)
    {
    case CTC_VLAN_CLASS_MAC:
        if (IS_MAC_ZERO(p_vlan_class_in->vlan_class.mac))
        {
            return CTC_E_INVALID_PARAM;
        }

        if (!(p_vlan_class_in->flag & CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            SYS_VLAN_CLASS_INFO_MAC("mac", p_vlan_class_in->vlan_class.mac);
            if (p_vlan_class_in->flag & CTC_VLAN_CLASS_FLAG_USE_MACDA)
            {
                field_key.type = CTC_FIELD_KEY_MAC_DA;
                field_key.ext_data = p_vlan_class_in->vlan_class.mac;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else
            {
                field_key.type = CTC_FIELD_KEY_MAC_SA;
                field_key.ext_data = p_vlan_class_in->vlan_class.mac;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
        }
        else
        {
            mac_addr_t mask;
            sal_memset(mask, 0xFF, sizeof(mac_addr_t));
            if (!(p_vlan_class_in->flag & CTC_VLAN_CLASS_FLAG_USE_MACDA))
            {
                SYS_VLAN_CLASS_INFO_MAC("macsa", p_vlan_class_in->vlan_class.mac);

                field_key.type = CTC_FIELD_KEY_MAC_SA;
                field_key.ext_data = p_vlan_class_in->vlan_class.mac;
                field_key.ext_mask = mask;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else
            {
                SYS_VLAN_CLASS_INFO_MAC("macda", p_vlan_class_in->vlan_class.mac);

                field_key.type = CTC_FIELD_KEY_MAC_DA;
                field_key.ext_data = p_vlan_class_in->vlan_class.mac;
                field_key.ext_mask = mask;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
        }

        break;

    case CTC_VLAN_CLASS_IPV4:
        if (!(p_vlan_class_in->flag & CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            SYS_VLAN_CLASS_INFO_IPV4("ipv4sa", p_vlan_class_in->vlan_class.ipv4_sa);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_vlan_class_in->vlan_class.ipv4_sa;
            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }

        }
        else
        {
            field_key.type = CTC_FIELD_KEY_L3_TYPE;
            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
            field_key.mask =0xF;
            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }

            if (0 != p_vlan_class_in->vlan_class.flex_ipv4.ipv4_sa_mask)
            {
                SYS_VLAN_CLASS_INFO_IPV4("ipv4_sa", p_vlan_class_in->vlan_class.flex_ipv4.ipv4_sa);
                field_key.type = CTC_FIELD_KEY_IP_SA;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.ipv4_sa;
                field_key.mask = p_vlan_class_in->vlan_class.flex_ipv4.ipv4_sa_mask;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            if (0 != p_vlan_class_in->vlan_class.flex_ipv4.ipv4_da_mask)
            {
                SYS_VLAN_CLASS_INFO_IPV4("ipv4_da", p_vlan_class_in->vlan_class.flex_ipv4.ipv4_da);
                field_key.type = CTC_FIELD_KEY_IP_DA;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.ipv4_da;
                field_key.mask = p_vlan_class_in->vlan_class.flex_ipv4.ipv4_da_mask;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            CTC_MAX_VALUE_CHECK(p_vlan_class_in->vlan_class.flex_ipv4.l3_type, MAX_CTC_PARSER_L3_TYPE - 1);
            if (CTC_PARSER_L3_TYPE_NONE != p_vlan_class_in->vlan_class.flex_ipv4.l3_type)
            {
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l3type                       :%d\n",
                                        p_vlan_class_in->vlan_class.flex_ipv4.l3_type);
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.l3_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            CTC_MAX_VALUE_CHECK(p_vlan_class_in->vlan_class.flex_ipv4.l4_type, MAX_CTC_PARSER_L4_TYPE - 1);

            if (CTC_PARSER_L4_TYPE_TCP == p_vlan_class_in->vlan_class.flex_ipv4.l4_type)
            {
                is_tcp = 1;
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4type                       :tcp\n");
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.l4_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else if (CTC_PARSER_L4_TYPE_UDP == p_vlan_class_in->vlan_class.flex_ipv4.l4_type)
            {
                is_udp = 1;
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4type                       :udp\n");
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.l4_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else if (CTC_PARSER_L4_TYPE_NONE != p_vlan_class_in->vlan_class.flex_ipv4.l4_type)
            {
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

            }


            if (0 != p_vlan_class_in->vlan_class.flex_ipv4.l4src_port)
            {
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4srcport                    :%d\n",
                                        p_vlan_class_in->vlan_class.flex_ipv4.l4src_port);

                if (!is_tcp && !is_udp)
                {
                     return CTC_E_INVALID_PARAM;
                }
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.l4src_port;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            if (0 != p_vlan_class_in->vlan_class.flex_ipv4.l4dest_port)
            {
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4destport                   :%d\n",
                                        p_vlan_class_in->vlan_class.flex_ipv4.l4dest_port);

                if (!is_tcp && !is_udp)
                {
                     return CTC_E_INVALID_PARAM;
                }
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv4.l4dest_port;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

        }

        break;

    case CTC_VLAN_CLASS_IPV6:
        if (!(p_vlan_class_in->flag & CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {

            SYS_VLAN_CLASS_INFO_IPV6("ipv6sa", p_vlan_class_in->vlan_class.ipv6_sa);
            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_data = p_vlan_class_in->vlan_class.ipv6_sa;
            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }
        }
        else
        {
            ipv6_addr_t ipv6_mask_zero;
            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_data = p_vlan_class_in->vlan_class.flex_ipv6.ipv6_sa;
            field_key.ext_mask = p_vlan_class_in->vlan_class.flex_ipv6.ipv6_sa_mask;
            if (is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }

            sal_memset(&ipv6_mask_zero, 0x00, sizeof(ipv6_addr_t));

            if (sal_memcmp(p_vlan_class_in->vlan_class.flex_ipv6.ipv6_da_mask, ipv6_mask_zero, sizeof(ipv6_addr_t)))
            {
                field_key.type = CTC_FIELD_KEY_IPV6_DA;
                field_key.ext_data = p_vlan_class_in->vlan_class.flex_ipv6.ipv6_da;
                field_key.ext_mask = p_vlan_class_in->vlan_class.flex_ipv6.ipv6_da_mask;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            CTC_MAX_VALUE_CHECK(p_vlan_class_in->vlan_class.flex_ipv6.l4_type, MAX_CTC_PARSER_L4_TYPE - 1);

            if (CTC_PARSER_L4_TYPE_TCP == p_vlan_class_in->vlan_class.flex_ipv6.l4_type)
            {
                is_tcp = 1;
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4type                       :tcp\n");
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv6.l4_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else if (CTC_PARSER_L4_TYPE_UDP == p_vlan_class_in->vlan_class.flex_ipv6.l4_type)
            {
                is_udp = 1;
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  l4type                       :udp\n");
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv6.l4_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }
            else if (CTC_PARSER_L4_TYPE_NONE != p_vlan_class_in->vlan_class.flex_ipv6.l4_type)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv6.l4_type;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            if (0 != p_vlan_class_in->vlan_class.flex_ipv6.l4src_port)
            {
                if (!is_tcp && !is_udp)
                {
                    return CTC_E_INVALID_PARAM;
                }
                field_key.type = CTC_FIELD_KEY_L4_SRC_PORT;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv6.l4src_port;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

            if (0 != p_vlan_class_in->vlan_class.flex_ipv6.l4dest_port)
            {
                if (!is_tcp && !is_udp)
                {
                    return CTC_E_INVALID_PARAM;
                }
                field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
                field_key.data = p_vlan_class_in->vlan_class.flex_ipv6.l4dest_port;
                field_key.mask = 0xFFFFFFFF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, ctc->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
                }
            }

        }

        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_get_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t class_type)
{

    uint32           eid;
    uint8            idx;
    uint8            loop_num;
    sys_scl_default_action_t action;
    ctc_vlan_miss_t vlan_class;
    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_miss_t));

    SYS_VLAN_CLASS_INIT_CHECK();

    loop_num = DRV_IS_TSINGMA(lchip)? (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)+2) : MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM);
    for (idx = 0; idx < loop_num; idx++)
    {
        switch (class_type)
        {
            case CTC_VLAN_CLASS_MAC:
                eid = p_usw_vlan_class_master[lchip]->mac_eid[idx];
                break;

            case CTC_VLAN_CLASS_IPV4:
                eid = p_usw_vlan_class_master[lchip]->ipv4_eid[idx];
                break;

            case CTC_VLAN_CLASS_IPV6:
                eid = p_usw_vlan_class_master[lchip]->ipv6_eid[idx];
                break;

            case CTC_VLAN_CLASS_PROTOCOL:
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  %% ERRO: Vlan class default entry only supports Mac Ipv4 Ipv6.\n");
                SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;


            default:
                return CTC_E_INVALID_PARAM;
        }

        if (0 == eid)
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

        }

        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s%-12u",  (class_type==CTC_VLAN_CLASS_MAC)?"Mac Base":((class_type==CTC_VLAN_CLASS_IPV4)?"IPv4 Base":"IPv6 Base"), idx);
        sal_memset(&action, 0, sizeof(action));
        action.action_type = SYS_SCL_ACTION_INGRESS;
        action.vlan_miss = &vlan_class;
        action.eid  = eid;
        action.is_vlan_class = 1;
        CTC_ERROR_RETURN(sys_usw_scl_get_default_action(lchip, &action));

        if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_TO_CPU)
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FWD_TO_CPU\n");
        }
        else if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_DISCARD)
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DISCARD\n");
        }
        else if (action.vlan_miss->flag == CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR)
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"USER_VLANPTR\n");
        }
        else
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FWD\n");
        }
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_vlan_add_default_vlan_class_key(uint8 lchip, ctc_vlan_class_type_t class_type)
{
    int32  ret = 0;
    uint32 gid[6] = {0};
    uint8  index = 0;
    uint8  loop_num = 0;
    uint8  key_type[3] = {0};
    uint8  resolve_conflict[3] = {0};
    uint32* eid = NULL;

    ctc_scl_entry_t scl;
    ctc_field_key_t field_key;
    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;

    sal_memset(&scl, 0, sizeof(scl));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));

    port_data.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
    port_data.port_class_id = MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_VLAN_CLASS);
    port_mask.port_class_id = 0xFFFF;
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &port_data;
    field_key.ext_mask = &port_mask;

    switch(class_type)
    {
        case CTC_VLAN_CLASS_MAC:
            eid = p_usw_vlan_class_master[lchip]->mac_eid;
            key_type[0] = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_MAC : SYS_SCL_KEY_HASH_MAC;
            key_type[1] = SYS_SCL_KEY_TCAM_MAC;
            key_type[2] = SYS_SCL_KEY_TCAM_MAC;
            resolve_conflict[0] = DRV_IS_DUET2(lchip) ? 0 : 1;
            resolve_conflict[1] = 0;
            break;
        case CTC_VLAN_CLASS_IPV4:
            eid = p_usw_vlan_class_master[lchip]->ipv4_eid;
            key_type[0] = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4_SINGLE : SYS_SCL_KEY_HASH_IPV4;
            key_type[1] = SYS_SCL_KEY_TCAM_IPV4_SINGLE;
            key_type[2] = SYS_SCL_KEY_TCAM_IPV4_SINGLE;
            resolve_conflict[0] = DRV_IS_DUET2(lchip) ? 0 : 1;
            resolve_conflict[1] = 0;
            break;
        case CTC_VLAN_CLASS_IPV6:
            eid = p_usw_vlan_class_master[lchip]->ipv6_eid;
            key_type[0] = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV6 : SYS_SCL_KEY_HASH_IPV6;
            key_type[1] = SYS_SCL_KEY_TCAM_IPV6;
            key_type[2] = SYS_SCL_KEY_TCAM_IPV6;
            resolve_conflict[0] = DRV_IS_DUET2(lchip) ? 0 : 1;
            resolve_conflict[1] = 0;
            break;
        case CTC_VLAN_CLASS_PROTOCOL:
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  %% ERRO: Vlan class default entry only supports Mac Ipv4 Ipv6.\n");
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        default:
            return CTC_E_INVALID_PARAM;
    }
    loop_num = DRV_IS_TSINGMA(lchip)? (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)+2) : MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM);
    for (index = 0; index < loop_num; index += 1)
    {
        if(eid[index] != 0)
        {
            continue;
        }
        gid[index] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_DEFAULT+(index%4)), 0, 0, 0);
        scl.resolve_conflict = resolve_conflict[index/2];
        scl.key_type = key_type[index/2];

        ret = sys_usw_scl_add_entry(lchip, gid[index], &scl, 1);
        if (CTC_E_NONE == ret)
        {
            eid[index] = scl.entry_id;
            CTC_ERROR_RETURN(sys_usw_scl_set_entry_priority(lchip, scl.entry_id, 0, 1));
        }
        else
        {
            goto error_0;
        }

        if (DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl.entry_id, &field_key));
        }

        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, eid[index], 1), ret, error_0);
    }

    return CTC_E_NONE;
error_0:

    for (index = 0; index < loop_num; index += 1)
    {
        if(eid[index] == 0)
        {
            continue;
        }
        sys_usw_scl_uninstall_entry(lchip, eid[index], 1);
        sys_usw_scl_remove_entry(lchip, eid[index], 1);
        eid[index] = 0;
    }
    return ret;
}
STATIC int32
_sys_usw_vlan_remove_default_vlan_class_key(uint8 lchip, ctc_vlan_class_type_t class_type)
{
    uint8  index = 0;
    uint8  loop_num = 0;
    uint32* eid = NULL;

    switch(class_type)
    {
        case CTC_VLAN_CLASS_MAC:
            eid = p_usw_vlan_class_master[lchip]->mac_eid;
            break;
        case CTC_VLAN_CLASS_IPV4:
            eid = p_usw_vlan_class_master[lchip]->ipv4_eid;
            break;
        case CTC_VLAN_CLASS_IPV6:
            eid = p_usw_vlan_class_master[lchip]->ipv6_eid;
            break;
        case CTC_VLAN_CLASS_PROTOCOL:
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  %% ERRO: Vlan class default entry only supports Mac Ipv4 Ipv6.\n");
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        default:
            return CTC_E_INVALID_PARAM;
    }
    loop_num = DRV_IS_TSINGMA(lchip)? (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)+2) : MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM);
    for (index = 0; index < loop_num; index += 1)
    {
        if(eid[index] == 0)
        {
            continue;
        }
        CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, eid[index], 1));
        CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, eid[index], 1));

        eid[index] = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_vlan_init_default_vlan_class(uint8 lchip)
{
    int32  ret = 0;
    uint32 gid[4] = {0};
    uint8 index = 0;

    ctc_scl_group_info_t group[4];
    ctc_scl_group_info_t group_base;

    sal_memset(&group_base, 0, sizeof(ctc_scl_group_info_t));

    /* if add entry failed, indicates no resources. thus make eid = 0 */
    group_base.type = CTC_SCL_GROUP_TYPE_NONE;
    group_base.priority = 0;

    for (index = 0; index < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index += 1)     /**< [TM] optimized for 4 tcam lookup */
    {
        sal_memcpy(&group[index], &group_base, sizeof(ctc_scl_group_info_t));
        group[index].priority = index;
        gid[index] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_VLAN, (SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_DEFAULT+index), 0, 0, 0);

        ret = sys_usw_scl_create_group(lchip, gid[index], &group[index], 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            goto error_0;
        }
    }

    /*add vlan class entry while init for duet2*/
    if(DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(_sys_usw_vlan_add_default_vlan_class_key(lchip, CTC_VLAN_CLASS_MAC), ret, error_0);
        CTC_ERROR_GOTO(_sys_usw_vlan_add_default_vlan_class_key(lchip, CTC_VLAN_CLASS_IPV4), ret, error_1);
        CTC_ERROR_GOTO(_sys_usw_vlan_add_default_vlan_class_key(lchip, CTC_VLAN_CLASS_IPV6), ret, error_2);
    }
    return CTC_E_NONE;
error_2:
    _sys_usw_vlan_remove_default_vlan_class_key(lchip, CTC_VLAN_CLASS_IPV4);
error_1:
    _sys_usw_vlan_remove_default_vlan_class_key(lchip, CTC_VLAN_CLASS_MAC);
error_0:
    for (index = 0; index < MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index += 1)
    {
        sys_usw_scl_destroy_group(lchip, gid[index], 1);
    }
    return ret;
}
STATIC int32
_sys_usw_vlan_class_mapping_key_type(uint8 lchip, ctc_vlan_class_t* p_vlan_class, ctc_scl_entry_t* scl)
{
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (p_vlan_class->type)
    {
        case CTC_VLAN_CLASS_MAC:
            if(p_vlan_class->scl_id < 2 && !DRV_IS_DUET2(lchip))
            {
                scl->key_type = SYS_SCL_KEY_HASH_MAC;
            }
            else
            {
                if (CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    scl->key_type = SYS_SCL_KEY_TCAM_MAC;
                }
                else
                {
                    scl->key_type = SYS_SCL_KEY_HASH_MAC;
                }
            }
            break;

        case CTC_VLAN_CLASS_IPV4:
            if(p_vlan_class->scl_id < 2 && !DRV_IS_DUET2(lchip))
            {
                scl->key_type = SYS_SCL_KEY_HASH_IPV4;
            }
            else
            {
                if (CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    scl->key_type = SYS_SCL_KEY_TCAM_IPV4_SINGLE;
                }
                else
                {
                    scl->key_type = SYS_SCL_KEY_HASH_IPV4;
                }
            }
            break;
        case CTC_VLAN_CLASS_IPV6:
            if(p_vlan_class->scl_id < 2 && !DRV_IS_DUET2(lchip))
            {
                scl->key_type = SYS_SCL_KEY_HASH_IPV6;
            }
            else
            {
                if (CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
                {
                    scl->key_type = SYS_SCL_KEY_TCAM_IPV6;
                }
                else
                {
                    scl->key_type = SYS_SCL_KEY_HASH_IPV6;
                }
            }
            break;
        default:
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            SYS_VLAN_CLASS_UNLOCK(lchip);
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}
#define __VLAN_CLASS_API__

int32
sys_usw_vlan_add_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class)
{
    int32 ret = 0;
    uint32 group_id = 0;
    ctc_scl_entry_t scl;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;
    sys_vlan_class_protocol_vlan_t pro_vlan;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO: ------vlan class----------\n");
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  type                         :%d\n", p_vlan_class->type);

    SYS_VLAN_CLASS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_class);
    CTC_VLAN_RANGE_CHECK(p_vlan_class->vlan_id);
    if (CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_class->scl_id, MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)-1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_vlan_class->scl_id, 1);
    }
    if(CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX) && CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT))
    {
        /* use-flex and resolve-conflict can not be configured at the same time */
        return CTC_E_NOT_SUPPORT;
    }
    if(p_vlan_class->scl_id < 2 && !DRV_IS_DUET2(lchip) && CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Flex service is not supported in scl id 0 or 1\n");
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&scl, 0, sizeof(scl));
    sal_memset(&field_action, 0, sizeof(field_action));
    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));

    if (p_vlan_class->flag & CTC_VLAN_CLASS_FLAG_OUTPUT_COS)
    {
        CTC_COS_RANGE_CHECK(p_vlan_class->cos);
    }

    SYS_VLAN_CLASS_LOCK(lchip);

    if (CTC_VLAN_CLASS_PROTOCOL == p_vlan_class->type)
    {
        sal_memset(&pro_vlan, 0, sizeof(sys_vlan_class_protocol_vlan_t));
        pro_vlan.vlan_id        = p_vlan_class->vlan_id;
        pro_vlan.cos            = p_vlan_class->cos;
        pro_vlan.cos_valid      = CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_OUTPUT_COS);

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_protocol_vlan_add(lchip, p_vlan_class->vlan_class.ether_type, &pro_vlan));
    }
    else
    {
        /*vlan class action*/
        sys_scl_lkup_key_t lkup_key;
        sal_memset(&lkup_key, 0, sizeof(lkup_key));

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_mapping_key_type(lchip, p_vlan_class, &scl));
        scl.action_type = SYS_SCL_ACTION_INGRESS;
        scl.resolve_conflict = CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT);

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_get_scl_gid(lchip, scl.key_type, scl.resolve_conflict, p_vlan_class->scl_id, &group_id));

        /* to confirm that vlan classfier can not be overwrited, we do not provide any api for this application */
        lkup_key.key_type = scl.key_type;
        lkup_key.action_type = scl.action_type;
        lkup_key.group_priority = p_vlan_class->scl_id;
        lkup_key.resolve_conflict = scl.resolve_conflict;
        lkup_key.group_id = group_id;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_build_key(lchip, p_vlan_class, NULL, &lkup_key, 0));
        if (!scl.resolve_conflict && !CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
        }

        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

        ret = sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key);
        if (CTC_E_NONE == ret)
        {
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
            SYS_VLAN_CLASS_UNLOCK(lchip);
			return CTC_E_EXIST;

        }
        else if (ret < 0 && ret != CTC_E_NOT_EXIST)
        {
            SYS_VLAN_CLASS_UNLOCK(lchip);
            return ret;
        }

        {
           ctc_scl_group_info_t group;
           int32 ret = 0;
           sal_memset(&group, 0, sizeof(group));
           group.type = CTC_SCL_GROUP_TYPE_NONE;
           group.priority = p_vlan_class->scl_id;
           ret = sys_usw_scl_create_group(lchip, group_id, &group, 1);
           if ((ret < 0 )&& (ret != CTC_E_EXIST))
           {
                SYS_VLAN_CLASS_UNLOCK(lchip);
                return ret;
           }
        }

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_entry(lchip, group_id, &scl, 1));

        CTC_ERROR_GOTO(_sys_usw_vlan_class_build_key(lchip, p_vlan_class, &scl, NULL, 1), ret, error0);

        if (!scl.resolve_conflict && !CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl.entry_id, &field_key), ret, error0);
        }

        /*vlan class action*/
        vlan_edit.svid_new = p_vlan_class->vlan_id;
        vlan_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
        if (p_vlan_class->flag & CTC_VLAN_CLASS_FLAG_OUTPUT_COS)
        {
            vlan_edit.scos_new = p_vlan_class->cos;
            vlan_edit.scos_sl  = CTC_VLAN_TAG_SL_NEW;
        }
        else
        {
            vlan_edit.scos_sl  = CTC_VLAN_TAG_SL_ALTERNATIVE;
        }

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl.entry_id, &field_action), ret, error0);

        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl.entry_id, 1), ret, error0);

    }

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO: %-30s:%d\n", ">>>>>>>>>>>>>>>>>>>>>>vlan", p_vlan_class->vlan_id);

    SYS_VLAN_CLASS_UNLOCK(lchip);
    return CTC_E_NONE;

error0:
    sys_usw_scl_remove_entry(lchip, scl.entry_id, 1);
    SYS_VLAN_CLASS_UNLOCK(lchip);
    return ret;
}


int32
sys_usw_vlan_remove_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class)
{
    ctc_scl_entry_t scl;
    ctc_field_key_t field_key;
    uint32 group_id = 0;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO: ------vlan class----------\n");
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "  %% INFO:  type                         :%d\n", p_vlan_class->type);

    SYS_VLAN_CLASS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_class);
    if (CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_class->scl_id, MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)-1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_vlan_class->scl_id, 1);
    }
    if(CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX) && CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT))
    {
        /* use-flex and resolve-conflict can not be configured at the same time */
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&scl, 0, sizeof(ctc_scl_entry_t));

    SYS_VLAN_CLASS_LOCK(lchip);

    if (CTC_VLAN_CLASS_PROTOCOL == p_vlan_class->type)
    {
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_protocol_vlan_remove(lchip, p_vlan_class->vlan_class.ether_type));
    }
    else
    {
        sys_scl_lkup_key_t lkup_key;
        sal_memset(&lkup_key, 0, sizeof(lkup_key));

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_mapping_key_type(lchip, p_vlan_class, &scl));
        scl.action_type = SYS_SCL_ACTION_INGRESS;
        scl.resolve_conflict = CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT);
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_get_scl_gid(lchip, scl.key_type, scl.resolve_conflict, p_vlan_class->scl_id, &group_id));
        lkup_key.key_type = scl.key_type;
        lkup_key.action_type = scl.action_type;
        lkup_key.group_priority = p_vlan_class->scl_id;
        lkup_key.resolve_conflict = scl.resolve_conflict;
        lkup_key.group_id = group_id;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_class_build_key(lchip, p_vlan_class, NULL, &lkup_key, 0));
        if (!scl.resolve_conflict && !CTC_FLAG_ISSET(p_vlan_class->flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
        }

        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

        scl.entry_id = lkup_key.entry_id;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_uninstall_entry(lchip, scl.entry_id, 1));
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_remove_entry(lchip, scl.entry_id, 1));
    }

    SYS_VLAN_CLASS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_remove_all_vlan_class(uint8 lchip, ctc_vlan_class_type_t type)
{
    int32 ret = 0;
    uint32 entry_num = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    DsProtocolVlan_m ds_pro_vlan;
    uint8 pro_vlan_idx = 0;
    uint32 gid = 0;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_CLASS_INIT_CHECK();

    SYS_VLAN_CLASS_LOCK(lchip);

    switch (type)
    {
    case CTC_VLAN_CLASS_MAC:
        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_MAC+index, ret);
        }
        for (index=0; index<2; index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_MAC+index, ret);
        }

        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_MAC+index, ret);
        }
        break;

    case CTC_VLAN_CLASS_IPV4:
        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV4 + index, ret);
        }

        for (index=0; index<2; index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV4 + index, ret);
        }

        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index+=1)
            {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV4 + index, ret);
        }
        break;

    case CTC_VLAN_CLASS_IPV6:
        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_HASH_NUM); index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_HASH_IPV6 + index, ret);
        }

        for (index=0; index<2; index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_RESOLVE_CONFLICT_HASH0_IPV6 + index, ret);
        }

        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index+=1)
        {
            SYS_VLAN_CLASS_UNINSTALL_GRP_BY_GTYPE(lchip, gid, CTC_FEATURE_VLAN, SYS_VLAN_CLASS_GROUP_SUB_TYPE_TCAM0_IPV6 + index, ret);
        }
        break;

    case CTC_VLAN_CLASS_PROTOCOL:
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_ftm_query_table_entry_num(lchip, DsProtocolVlan_t, &entry_num));



            for (pro_vlan_idx = 0; pro_vlan_idx < entry_num; pro_vlan_idx++)
            {
                sal_memset(&ds_pro_vlan, 0, sizeof(DsProtocolVlan_m));
                cmd = DRV_IOW(DsProtocolVlan_t, DRV_ENTRY_FLAG);
                ret = ret ? ret : DRV_IOCTL(lchip, pro_vlan_idx, cmd, &ds_pro_vlan);
            }


        for (index = 0; index < 8; index++)
        {
            p_usw_vlan_class_master[lchip]->ether_type[index] = 0;
        }

        break;

    default:
        SYS_VLAN_CLASS_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_E_NOT_EXIST == ret)
    {
        ret = CTC_E_NONE;
    }
    SYS_VLAN_CLASS_UNLOCK(lchip);
    return ret;
}

/**
 * vlan class default only support 3 type: mac-sa ipv4-sa ipv6-sa.
 */
int32
sys_usw_vlan_add_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t class_type, ctc_vlan_miss_t* p_def_action)
{
    ctc_scl_action_t action;
    uint32*          eid = NULL;
    uint8 index = 0;
    uint8 loop_num = 0;
    ctc_scl_field_action_t field_action;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_CLASS_INIT_CHECK();

    sal_memset(&action, 0, sizeof(action));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_VLAN_CLASS_LOCK(lchip);
    if(!DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_add_default_vlan_class_key(lchip, class_type));
    }
    switch (class_type)
    {
        case CTC_VLAN_CLASS_MAC:
            eid = p_usw_vlan_class_master[lchip]->mac_eid;
            break;
        case CTC_VLAN_CLASS_IPV4:
            eid = p_usw_vlan_class_master[lchip]->ipv4_eid;
            break;
        case CTC_VLAN_CLASS_IPV6:
            eid = p_usw_vlan_class_master[lchip]->ipv6_eid;
            break;
        case CTC_VLAN_CLASS_PROTOCOL:
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  %% ERRO: Vlan class default entry only supports Mac Ipv4 Ipv6.\n");
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            SYS_VLAN_CLASS_UNLOCK(lchip);
			return CTC_E_NOT_SUPPORT;

        default:
            SYS_VLAN_CLASS_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }

    if (0 == (eid[0] + eid[1] + eid[2] + eid[3]+eid[4] + eid[5]) )      /*Compatible with D2*/
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        SYS_VLAN_CLASS_UNLOCK(lchip);
		return CTC_E_NO_RESOURCE;

    }

    /*because userid 0,1 have two default entry*/
    loop_num = DRV_IS_TSINGMA(lchip)? (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)+2) : MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM);
    for (index=0; index<loop_num; index+=1)
    {
        if(0 == eid[index])
        {
            continue;
        }
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));

    switch (p_def_action->flag)
    {
        case CTC_VLAN_MISS_ACTION_DO_NOTHING:

            break;

            case CTC_VLAN_MISS_ACTION_TO_CPU:
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));
                break;


            case CTC_VLAN_MISS_ACTION_DISCARD:
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));
                break;

        case CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR:
            SYS_MAX_VALUE_CHECK_WITH_UNLOCK(p_def_action->user_vlanptr, 0x1fff, p_usw_vlan_class_master[lchip]->mutex);

                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
                field_action.data0 = p_def_action->user_vlanptr;
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));
                break;

        default:
            SYS_VLAN_CLASS_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;

    }

        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_install_entry(lchip, eid[index], TRUE));
    }

    SYS_VLAN_CLASS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_remove_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t class_type)
{
    ctc_scl_action_t action;
    uint32*          eid = NULL;
    uint8 index = 0;
    ctc_scl_field_action_t field_action;

    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_VLAN_CLASS_INIT_CHECK();

    /* do nothing */
    sal_memset(&action, 0, sizeof(action));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    SYS_VLAN_CLASS_LOCK(lchip);
    switch (class_type)
    {
        case CTC_VLAN_CLASS_MAC:
            eid = p_usw_vlan_class_master[lchip]->mac_eid;
            break;
        case CTC_VLAN_CLASS_IPV4:
            eid = p_usw_vlan_class_master[lchip]->ipv4_eid;
            break;
        case CTC_VLAN_CLASS_IPV6:
            eid = p_usw_vlan_class_master[lchip]->ipv6_eid;
            break;
        case CTC_VLAN_CLASS_PROTOCOL:
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  %% ERRO: Vlan class default entry only supports Mac Ipv4 Ipv6.\n");
            SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            SYS_VLAN_CLASS_UNLOCK(lchip);
			return CTC_E_NOT_SUPPORT;

        default:
            SYS_VLAN_CLASS_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }

    if (0 == (eid[0] + eid[1] + eid[2] + eid[3] + eid[4] + eid[5]))       /*Compatible with D2*/
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        SYS_VLAN_CLASS_UNLOCK(lchip);
		return CTC_E_NOT_EXIST;

    }

    if(!DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(_sys_usw_vlan_remove_default_vlan_class_key(lchip, class_type));
    }
    else
    {
        for (index=0; index<MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM); index+=1)
        {
            if (eid[index])
            {
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL;
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_add_action_field(lchip, eid[index], &field_action));
                CTC_ERROR_RETURN_VLAN_CLASS_UNLOCK(sys_usw_scl_install_entry(lchip, eid[index], TRUE));
            }
        }
    }
    SYS_VLAN_CLASS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_vlan_show_default_vlan_class(uint8 lchip)
{
    SYS_VLAN_CLASS_INIT_CHECK();
    SYS_VLAN_CLASS_LOCK(lchip);
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s%-12s%s\n", "TYPE", "INDEX", "Default-Action");
    SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------\n");
    _sys_usw_vlan_get_default_vlan_class( lchip, CTC_VLAN_CLASS_MAC);
    _sys_usw_vlan_get_default_vlan_class( lchip, CTC_VLAN_CLASS_IPV4);
    _sys_usw_vlan_get_default_vlan_class( lchip, CTC_VLAN_CLASS_IPV6);
    SYS_VLAN_CLASS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32 sys_usw_adv_vlan_wb_sync(uint8 lchip)
{
    uint8 loop = 0;
    uint8 maxloop = 0;
    int32 ret = CTC_E_NONE;
    sys_vlan_mapping_master_t* p_vlan_mapping_master = NULL;
    sys_wb_adv_vlan_master_t* p_wb_adv_vlan_master = NULL;
    sys_wb_adv_vlan_default_entry_id_t* p_wb_adv_vlan_entry_id = NULL;
    sys_wb_adv_vlan_range_group_t* p_wb_adv_vlan_range_group = NULL;
    ctc_wb_data_t wb_data;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);
	if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		return CTC_E_NONE;
	}
    sal_memset(&wb_data, 0, sizeof(ctc_wb_data_t));

    wb_data.buffer = mem_malloc(MEM_VLAN_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }

    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_ERROR_GOTO(sys_usw_vlan_mapping_get_master(lchip, &p_vlan_mapping_master), ret, done);

    /* sync adv vlan master */
    p_wb_adv_vlan_master = (sys_wb_adv_vlan_master_t*)wb_data.buffer;
    p_wb_adv_vlan_master->lchip = lchip;
    p_wb_adv_vlan_master->version = SYS_WB_VERSION_ADV_VLAN;

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_adv_vlan_master_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_MASTER);

    wb_data.valid_cnt = 1;

    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /* sync vlan class default entry id */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_adv_vlan_default_entry_id_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_DEFAULT_ENTRY_ID);
    if(DRV_IS_DUET2(lchip))
    {
        maxloop = MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM);
    }
    else
    {
        maxloop = 6;
    }

    for (loop = 0; loop < maxloop; loop++)
    {
        p_wb_adv_vlan_entry_id = (sys_wb_adv_vlan_default_entry_id_t*)wb_data.buffer + wb_data.valid_cnt;
        p_wb_adv_vlan_entry_id->tcam_id = loop;
        p_wb_adv_vlan_entry_id->mac_eid = p_usw_vlan_class_master[lchip]->mac_eid[loop];
        p_wb_adv_vlan_entry_id->ipv4_eid = p_usw_vlan_class_master[lchip]->ipv4_eid[loop];
        p_wb_adv_vlan_entry_id->ipv6_eid = p_usw_vlan_class_master[lchip]->ipv6_eid[loop];
        wb_data.valid_cnt++;
    }

    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /* sync vlan mapping vlan range group */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_adv_vlan_range_group_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_RANGE_GROUP);

    for (loop = 0; loop < VLAN_RANGE_ENTRY_NUM * CTC_BOTH_DIRECTION; loop++)
    {
        p_wb_adv_vlan_range_group = (sys_wb_adv_vlan_range_group_t*)wb_data.buffer + wb_data.valid_cnt;
        p_wb_adv_vlan_range_group->group_id = loop;
        p_wb_adv_vlan_range_group->group_flag = p_vlan_mapping_master->vrange[loop];
        wb_data.valid_cnt++;
    }

    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}

int32 sys_usw_adv_vlan_wb_restore (uint8 lchip)
{
    uint8 loop = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    DsProtocolVlan_m ds_protocol_vlan;
    sys_vlan_mapping_master_t* p_vlan_mapping_master = NULL;
    sys_wb_adv_vlan_master_t wb_adv_vlan_master;
    sys_wb_adv_vlan_default_entry_id_t wb_adv_vlan_default_entry_id;
    sys_wb_adv_vlan_range_group_t wb_adv_vlan_range_group;
    ctc_wb_query_t wb_query;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);
	if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		return CTC_E_NONE;
	}

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));

    wb_query.buffer = mem_malloc(MEM_VLAN_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /* restore master */
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_adv_vlan_master, 0, sizeof(sys_wb_adv_vlan_master_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_adv_vlan_master_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query adv vlan master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        goto done;
    }

    sal_memcpy((uint8*)&wb_adv_vlan_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_ADV_VLAN, wb_adv_vlan_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    /* restore valn class default entry id */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_adv_vlan_default_entry_id_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_DEFAULT_ENTRY_ID);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_adv_vlan_default_entry_id, 0, sizeof(sys_wb_adv_vlan_default_entry_id_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_adv_vlan_default_entry_id, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_usw_vlan_class_master[lchip]->mac_eid[wb_adv_vlan_default_entry_id.tcam_id] = wb_adv_vlan_default_entry_id.mac_eid;
        p_usw_vlan_class_master[lchip]->ipv4_eid[wb_adv_vlan_default_entry_id.tcam_id] = wb_adv_vlan_default_entry_id.ipv4_eid;
        p_usw_vlan_class_master[lchip]->ipv6_eid[wb_adv_vlan_default_entry_id.tcam_id] = wb_adv_vlan_default_entry_id.ipv6_eid;
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    cmd = DRV_IOR(DsProtocolVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ds_protocol_vlan), ret, done);
    for(loop = 0; loop < 8; loop++)
    {
        p_usw_vlan_class_master[lchip]->ether_type[loop] = GetDsProtocolVlan(V, protocolEtherType0_f + loop, &ds_protocol_vlan);
    }

    CTC_ERROR_GOTO(sys_usw_vlan_mapping_get_master(lchip, &p_vlan_mapping_master), ret, done);

     /* restore vlan mapping range group */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_adv_vlan_range_group_t, CTC_FEATURE_VLAN_MAPPING, SYS_WB_APPID_SUBID_ADV_VLAN_RANGE_GROUP);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_adv_vlan_range_group, 0, sizeof(sys_wb_adv_vlan_range_group_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_adv_vlan_range_group, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        p_vlan_mapping_master->vrange[wb_adv_vlan_range_group.group_id] = wb_adv_vlan_range_group.group_flag;
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }
    return ret;

}

int32
sys_usw_vlan_class_init(uint8 lchip, void* vlan_global_cfg)
{
    uint32 cmd   = 0;
    ctc_scl_group_info_t hash_group;
    ctc_scl_group_info_t class_group;
    ctc_scl_group_info_t global_group;
    DsProtocolVlan_m pro_vlan;
    uint8 work_status = 0;
    LCHIP_CHECK(lchip);

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
	}
    if (NULL != p_usw_vlan_class_master[lchip])
    {
        return CTC_E_NONE;
    }

    CTC_PTR_VALID_CHECK(vlan_global_cfg);
    MALLOC_ZERO(MEM_VLAN_MODULE, p_usw_vlan_class_master[lchip], sizeof(sys_vlan_class_master_t));
    if (NULL == p_usw_vlan_class_master[lchip])
    {
        SYS_VLAN_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;

    }

    sal_memset(&pro_vlan, 0, sizeof(DsProtocolVlan_m));
    sal_memset(&hash_group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&class_group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&global_group, 0, sizeof(ctc_scl_group_info_t));

    cmd = DRV_IOW(DsProtocolVlan_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pro_vlan));

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_VLAN_MAPPING, sys_usw_adv_vlan_wb_sync));
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (work_status != CTC_FTM_MEM_CHANGE_RECOVER))
    {
        /* do not need init default entry */
        CTC_ERROR_RETURN(sys_usw_adv_vlan_wb_restore(lchip));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_vlan_init_default_vlan_class(lchip));
    }

    SYS_VLAN_CLASS_CREATE_LOCK(lchip);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
	}
    return CTC_E_NONE;
}

int32
sys_usw_vlan_class_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if(NULL == p_usw_vlan_class_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(p_usw_vlan_class_master[lchip]->mutex);
    mem_free(p_usw_vlan_class_master[lchip]);

    return CTC_E_NONE;
}



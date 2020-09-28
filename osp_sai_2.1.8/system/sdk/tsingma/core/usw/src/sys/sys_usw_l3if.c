/**
@file sys_usw_l3if.c

@date 2009 - 12 - 7

@version v2.0

---file comments----
*/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_pdu.h"

#include "sys_usw_common.h"
#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_l3if.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_vlan.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_port.h"
#include "sys_usw_ftm.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "usw/include/drv_io.h"

/**********************************************************************************
Defines and Macros
***********************************************************************************/

#define MAX_L3IF_RELATED_PROPERTY       2
#define SYS_SPEC_L3IF_NUM               4096
#define SYS_L3IF_MAX_VRF_ID   0xFFF

#define SYS_L3IF_INIT_CHECK()                                  \
{                                                              \
    LCHIP_CHECK(lchip);                                        \
    if (p_usw_l3if_master[lchip] == NULL)                          \
    {                                                          \
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
		return CTC_E_NOT_INIT;\
    }                                                          \
}

#define SYS_L3IF_CREATE_CHECK(l3if_id)                  \
if (FALSE == p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vaild)   \
{                                                       \
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Not exist! \n");\
	return CTC_E_NOT_EXIST;\
}

#define SYS_L3IF_DBG_OUT(level, FMT, ...)\
    CTC_DEBUG_OUT(l3if, l3if, L3IF_SYS, level, FMT, ##__VA_ARGS__)

#define SYS_L3IF_LOCK \
    if(p_usw_l3if_master[lchip]->mutex) sal_mutex_lock(p_usw_l3if_master[lchip]->mutex)

#define SYS_L3IF_UNLOCK \
    if(p_usw_l3if_master[lchip]->mutex) sal_mutex_unlock(p_usw_l3if_master[lchip]->mutex)

#define CTC_ERROR_RETURN_L3IF_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_l3if_master[lchip]->mutex); \
            return (rv); \
        } \
    } while (0)

#define CTC_RETURN_L3IF_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_usw_l3if_master[lchip]->mutex); \
        return (op); \
    } while (0)

struct sys_l3if_merger_info_s
{
    sys_l3if_rtmac_profile_t *new_profile;
    sys_l3if_rtmac_profile_t* p_profile[64];
    uint8 bitmap[64];
    uint8 cnt;
};
typedef struct sys_l3if_merger_info_s sys_l3if_merger_info_t;

struct sys_l3if_profile_s
{
    union
    {
        DsSrcInterfaceProfile_m src;
        DsDestInterfaceProfile_m dst;
    }profile;
    uint8 dir;
    uint8 rsv;
    uint16 profile_id;
};
typedef struct sys_l3if_profile_s sys_l3if_profile_t;

struct sys_l3if_master_s
{
    sal_mutex_t* mutex;
    sys_l3if_prop_t  l3if_prop[SYS_SPEC_L3IF_NUM];
    ctc_vector_t* ecmp_group_vec;
    ctc_spool_t*  acl_spool;
    ctc_spool_t*  macinner_spool;
    ctc_spool_t* rtmac_spool;
    uint16 max_vrfid_num;
    uint8  opf_type_l3if_profile;
    uint8  opf_type_mac_profile;
    uint8  opf_type_macinner_profile;
    uint8  default_entry_mode;
    uint8  ipv4_vrf_en;
    uint8  ipv6_vrf_en;
    mac_addr_t mac_prefix[SYS_ROUTER_MAC_PREFIX_NUM];
};
typedef struct sys_l3if_master_s sys_l3if_master_t;

/****************************************************************************
Global and Declaration
*****************************************************************************/

sys_l3if_master_t* p_usw_l3if_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

extern int32
sys_usw_l3if_wb_sync(uint8 lchip);

extern int32
sys_usw_l3if_wb_restore(uint8 lchip);

extern int32
sys_usw_nh_update_ecmp_interface_group(uint8 lchip, sys_l3if_ecmp_if_t* p_group);

extern int32
sys_usw_flow_stats_add_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id);
extern int32
sys_usw_flow_stats_remove_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id);
extern int32
sys_usw_port_set_phy_if_en(uint8 lchip, uint32 gport, bool enable);

/**********************************************************************************
Define API function interfaces
***********************************************************************************/
STATIC uint32
_sys_usw_l3if_profile_make(sys_l3if_profile_t* p_profile)
{
    uint32 size = 0;
    uint8  * k = NULL;

    CTC_PTR_VALID_CHECK(p_profile);
    /*profile_id not participate in hash calculate and compare*/
    size = sizeof(sys_l3if_profile_t) - sizeof(uint32);
    k    = (uint8 *) p_profile;

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_l3if_profile_compare(sys_l3if_profile_t* p0,
                                           sys_l3if_profile_t* p1)
{
    if (!p0 || !p1)
    {
        return FALSE;
    }
    /*profile_id not participate in hash calculate and compare*/
    if ((p0->dir == p1->dir) && !sal_memcmp(&p0->profile, &p1->profile, sizeof(p0->profile)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_l3if_profile_build_index(sys_l3if_profile_t* p_profile, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(CTC_INGRESS == p_profile->dir)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_l3if_profile;

    if (p_profile->profile_id)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_profile->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_profile->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

/* free index of HASH ad */
STATIC int32
_sys_usw_l3if_profile_free_index(sys_l3if_profile_t* p_profile, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(CTC_INGRESS == p_profile->dir)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_l3if_profile;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_profile->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_l3if_macinner_profile_make(sys_l3if_macinner_profile_t* p_profile)
{
    uint32 size = 0;
    uint8  * k = NULL;

    CTC_PTR_VALID_CHECK(p_profile);
    /*profile_id not participate in hash calculate and compare*/
    size = 4*sizeof(uint8) + sizeof(mac_addr_t);
    k    = (uint8 *) &p_profile->type;

    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_l3if_macinner_profile_compare(sys_l3if_macinner_profile_t* p0,
                                           sys_l3if_macinner_profile_t* p1)
{
    if (!p0 || !p1)
    {
        return FALSE;
    }
    /*profile_id not participate in hash calculate and compare*/
    if (!sal_memcmp(p0->mac, p1->mac, sizeof(mac_addr_t)) && p0->type ==  p1->type)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_l3if_macinner_profile_build_index(sys_l3if_macinner_profile_t* p_profile, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_profile->type ? 1 : 0;
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_macinner_profile;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_profile->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_profile->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

/* free index of HASH ad */
STATIC int32
_sys_usw_l3if_macinner_profile_free_index(sys_l3if_macinner_profile_t* p_profile, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = p_profile->type ? 1 : 0;
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_macinner_profile;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_profile->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_bmp2num(uint8 bitmap)
{
    uint8 i = 0;
    uint8 number = 0;

    if (0 == bitmap)
    {
        return 0;
    }
    for (i = 0; i < 8 ; i++)
    {
        if (CTC_IS_BIT_SET(bitmap, i))
        {
            number++;
        }
    }

    return number;
}

int32
_sys_usw_l3if_unbinding_egs_rtmac(uint8 lchip, uint16 l3if_id)
{

    sys_l3if_macinner_profile_t* p_profile = NULL;
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_profile = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile;
    if (p_profile)
    {
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_l3if_master[lchip]->macinner_spool,p_profile , NULL));
        p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile = NULL;
        p_usw_l3if_master[lchip]->l3if_prop[l3if_id].user_egs_rtmac = 0;
    }

    return CTC_E_NONE;
}


uint32
_sys_usw_l3if_binding_egs_rtmac(uint8 lchip, uint16 l3if_id, mac_addr_t mac_addr, uint8 by_user)
{
    int32 ret = CTC_E_NONE;
    uint32  cmd = 0;
    uint32  mac[2] = {0};
    uint32  value = 0;
    DsEgressRouterMac_m  egs_router_mac;
    sys_l3if_macinner_profile_t profile_new;
    sys_l3if_macinner_profile_t* profile_get = NULL;
    mac_addr_t old_mac;
    uint8 old_exist = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&profile_new, 0, sizeof(sys_l3if_macinner_profile_t));
    sal_memcpy(profile_new.mac, mac_addr, sizeof(mac_addr_t));
    profile_new.type = 1;

    if(p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile)
    {
        sal_memcpy(old_mac, p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile->mac, sizeof(mac_addr_t));
        old_exist = 1;
    }
    if (!by_user && p_usw_l3if_master[lchip]->l3if_prop[l3if_id].user_egs_rtmac)
    {
        return ret;
    }
    _sys_usw_l3if_unbinding_egs_rtmac( lchip, l3if_id);

    ret = ctc_spool_add(p_usw_l3if_master[lchip]->macinner_spool, &profile_new, NULL, &profile_get);
    if(ret <0)
    {
        if(old_exist)
        {
            _sys_usw_l3if_binding_egs_rtmac(lchip, l3if_id, old_mac, by_user);
        }
        return ret;
    }

    SYS_USW_SET_HW_MAC(mac, mac_addr);
    SetDsEgressRouterMac(A, routerMac_f, &egs_router_mac, mac);
    cmd = DRV_IOW(DsEgressRouterMac_t, DRV_ENTRY_FLAG);
    value = profile_get->profile_id;
    ret = DRV_IOCTL(lchip, value, cmd, &egs_router_mac);
    if (ret < 0)
    {
        ctc_spool_remove(p_usw_l3if_master[lchip]->macinner_spool, profile_get, NULL);
    }
    cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_routerMacProfile_f);
    DRV_IOCTL(lchip, l3if_id, cmd, &value);

    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile = profile_get;
    if (by_user)
    {
        p_usw_l3if_master[lchip]->l3if_prop[l3if_id].user_egs_rtmac = 1;
    }

    return ret;
}



STATIC int32
_sys_usw_l3if_rtmac_match_prefix(uint8 lchip, mac_addr_t new_rtmac, uint8* prefix_idx)
{
    uint8 i = 0;
    mac_addr_t void_mac = {0};

    for(i = 0; i < SYS_ROUTER_MAC_PREFIX_NUM; i++)
    {
        if (0 == sal_memcmp(void_mac, p_usw_l3if_master[lchip]->mac_prefix[i], 5*sizeof(uint8)))
        {
            continue;
        }
        if (0 == sal_memcmp(new_rtmac, p_usw_l3if_master[lchip]->mac_prefix[i], 5*sizeof(uint8)))/* match */
        {
            *prefix_idx = i + 1;
            return CTC_E_NONE;
        }
    }

    return CTC_E_NOT_EXIST;
}

STATIC int32
_sys_usw_l3if_rtmac_match_postfix(uint8 lchip, mac_addr_t new_rtmac, mac_addr_t postfix_mac, uint8* postfix_idx)
{
    uint8 i = 0;

    for(i = 0; i < 6; i++)
    {
        if(new_rtmac[5] == postfix_mac[i])
        {
            *postfix_idx = i;
            return 0;
        }
    }

    return 1;
}

STATIC uint32
_sys_usw_l3if_rtmac_dump_profile(uint8 lchip, sys_l3if_rtmac_profile_t* p_profile)
{
    uint8 i = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "============================\n");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "profile_id=[%d]\n", p_profile->profile_id);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "valid_bitmap =[%02x]\n\n", p_profile->valid_bitmap);
    for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " mac[%d][%s]\n", i, sys_output_mac(p_profile->mac_info[i].mac));
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " postfix_bmp=[%02x]\n", p_profile->mac_info[i].postfix_bmp);
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " prefix_idx=[%d]\n", p_profile->mac_info[i].prefix_idx);
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " mac_ref=[%d]\n\n", p_profile->mac_ref[i]);
    }

    return 0;
}

/*search macs of p0 from p1, if find then unset p0_bmp and set p1_bmp */
STATIC int32
_sys_usw_l3if_rtmac_profile_mapping(sys_l3if_rtmac_profile_t* p0, sys_l3if_rtmac_profile_t* p1, uint8 *p0_bmp, uint8 *p1_bmp)
{
    uint8 i = 0, j = 0;
    uint8 lchip = p0->lchip;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for(i = 0;  i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        if(!CTC_IS_BIT_SET(p0->valid_bitmap, i))
        {
            continue;
        }
        for(j = 0; j < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); j++)
        {
            if(!CTC_IS_BIT_SET(p1->valid_bitmap, j))
            {
                continue;
            }
            if(p0->mac_info[i].prefix_idx != p1->mac_info[j].prefix_idx)
            {
                continue;
            }
            if (!sal_memcmp(p0->mac_info[i].mac, p1->mac_info[j].mac, sizeof(mac_addr_t)))
            {
                CTC_BIT_SET(*p1_bmp, j);
                CTC_BIT_UNSET(*p0_bmp, i);
                break;
            }
        }
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_l3if_rtmac_add_postfix_in_order(uint8 lchip, sys_l3if_router_mac_t * sys_rtmac, uint8 postfix)
{
    int8 j = 0, i = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for(j = 0; j < sizeof(mac_addr_t); j++)
    {
        if(CTC_IS_BIT_SET(sys_rtmac->postfix_bmp, j))
        {
            if(sys_rtmac->mac[j] < postfix)
            {
                continue;
            }
            else
            {
                for(i = sizeof(mac_addr_t)-2; i >= j; i--)
                {
                    if(CTC_IS_BIT_SET(sys_rtmac->postfix_bmp, i))
                    {
                        sys_rtmac->mac[i+1] = sys_rtmac->mac[i];
                        CTC_BIT_SET(sys_rtmac->postfix_bmp, i+1);
                    }
                }
                sys_rtmac->mac[j] = postfix;
                break;
            }
        }
        else
        {
            sys_rtmac->mac[j] = postfix;
            CTC_BIT_SET(sys_rtmac->postfix_bmp, j);
            break;
        }
    }

    for(j = 0; j < sizeof(mac_addr_t); j++)
    {
        if(!CTC_IS_BIT_SET(sys_rtmac->postfix_bmp, j))
        {
            sys_rtmac->mac[j] = postfix;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l3if_rtmac_delete_postfix_in_order(uint8 lchip, sys_l3if_router_mac_t * sys_rtmac, uint8 postfix)
{
    int8 j = 0, i = 0, pad = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for(j = 0; j < sizeof(mac_addr_t); j++)
    {
        if(!CTC_IS_BIT_SET(sys_rtmac->postfix_bmp, j))
        {
            continue;
        }
        if(sys_rtmac->mac[j] != postfix)
        {
            continue;
        }
        pad = j-1;
        for(i = j+1; i < sizeof(mac_addr_t)+1; i++)
        {
            if(CTC_IS_BIT_SET(sys_rtmac->postfix_bmp, i) && i < sizeof(mac_addr_t))
            {
                sys_rtmac->mac[i-1] = sys_rtmac->mac[i];
                pad = i-1;
                CTC_BIT_SET(sys_rtmac->postfix_bmp, i-1);
            }
            else
            {
                if(pad > -1)
                {
                    sys_rtmac->mac[i-1] = sys_rtmac->mac[pad];
                }
                CTC_BIT_UNSET(sys_rtmac->postfix_bmp, i-1);
            }
            CTC_BIT_UNSET(sys_rtmac->postfix_bmp, i);
        }
        return CTC_E_NONE;
    }

    return CTC_E_NOT_EXIST;
}

STATIC int32
_sys_usw_l3if_rtmac_build_profile(uint8 lchip, uint16 l3if_id, sys_l3if_rtmac_profile_t * p_rtmac_profile, mac_addr_t mac_addr, uint8 is_del)
{
    uint8   i = 0;
    uint8   prefix_idx = 0;
    uint8   postfix_idx = 0;
    uint8   insert_idx = 0xff;
    uint8   del_idx = 0xff;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_rtmac_profile->lchip = lchip;
    _sys_usw_l3if_rtmac_match_prefix( lchip, mac_addr, &prefix_idx);

    /*check in-use mac*/
    for(i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        if(!CTC_IS_BIT_SET(p_rtmac_profile->valid_bitmap, i))
        {
            insert_idx = ((insert_idx == 0xff) ?  i : insert_idx);
            sal_memset(p_rtmac_profile->mac_info[i].mac, 0 , sizeof(mac_addr_t));
            continue;
        }
        if(p_rtmac_profile->mac_info[i].prefix_idx != prefix_idx)
        {
            continue;
        }

        if(prefix_idx)
        {
            if(!_sys_usw_l3if_rtmac_match_postfix(lchip, mac_addr, p_rtmac_profile->mac_info[i].mac, &postfix_idx))/*found*/
            {
                del_idx = i;
                if(is_del)
                {
                    break;
                }
                else
                {
                    return CTC_E_NONE;
                }
            }
            else
            {
                if(p_rtmac_profile->mac_info[i].postfix_bmp != 0x3f)
                {
                    insert_idx = i;
                    continue;
                }
            }
        }
        else
        {
            if(0 == sal_memcmp(p_rtmac_profile->mac_info[i].mac, mac_addr, sizeof(mac_addr_t)))
            {
                del_idx = i;
                if(is_del)
                {
                    break;
                }
                else
                {
                    return CTC_E_NONE;
                }
            }
        }
    }

    if(is_del)
    {
        if(del_idx == 0xff)
        {
            return CTC_E_NONE;
        }
        if(prefix_idx)
        {
            _sys_usw_l3if_rtmac_delete_postfix_in_order( lchip, &p_rtmac_profile->mac_info[del_idx], mac_addr[5]);
            if(p_rtmac_profile->mac_info[del_idx].postfix_bmp == 0)
            {
                p_rtmac_profile->mac_info[del_idx].prefix_idx = 0;
                CTC_BIT_UNSET(p_rtmac_profile->valid_bitmap, del_idx);
                p_rtmac_profile->mac_ref[del_idx]--;/*mac_ref is change cnt to +/- by db.mac_ref*/
            }
        }
        else
        {
            sal_memset(p_rtmac_profile->mac_info[del_idx].mac, 0, sizeof(mac_addr_t));
            CTC_BIT_UNSET(p_rtmac_profile->valid_bitmap, del_idx);
            p_rtmac_profile->mac_ref[del_idx]--;
        }
    }
    else
    {
        if(insert_idx == 0xff)
        {
            return CTC_E_NO_RESOURCE;
        }

        if(prefix_idx)
        {
            _sys_usw_l3if_rtmac_add_postfix_in_order( lchip, &p_rtmac_profile->mac_info[insert_idx], mac_addr[5]);
            if(!CTC_IS_BIT_SET(p_rtmac_profile->valid_bitmap, insert_idx))
            {
                p_rtmac_profile->mac_info[insert_idx].prefix_idx = prefix_idx;
                CTC_BIT_SET(p_rtmac_profile->valid_bitmap, insert_idx);
                p_rtmac_profile->mac_ref[insert_idx]++;
            }
        }
        else
        {
            sal_memcpy(p_rtmac_profile->mac_info[insert_idx].mac, mac_addr, sizeof(mac_addr_t));
            CTC_BIT_SET(p_rtmac_profile->valid_bitmap, insert_idx);
            p_rtmac_profile->mac_ref[insert_idx]++;
        }
    }
return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_l3if_rtmac_read_asic_mac(uint8 lchip, uint16 profile_id, sys_l3if_rtmac_profile_t *profile)
{
    uint8 i = 0;
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint32 index = 0;
    DsRouterMac_m ingress_router_mac;
    uint32 routerMacDecode = 0;
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ingress_router_mac, 0, sizeof(DsRouterMac_m));
    cmd = DRV_IOR(DsRouterMac_t, DRV_ENTRY_FLAG);
    index = profile->profile_id;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ingress_router_mac));

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac0_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[0].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac1_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[1].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac2_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[2].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac3_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[3].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac4_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[4].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac5_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[5].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac6_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[6].mac, mac);

    sal_memset(mac, 0, sizeof(mac));
    GetDsRouterMac(A, routerMac7_f, &ingress_router_mac, mac);
    SYS_USW_SET_USER_MAC(profile->mac_info[7].mac, mac);

    for(i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        routerMacDecode = GetDsRouterMac(V, routerMacDecode0_f + i, &ingress_router_mac);
        if(routerMacDecode != 0)
        {
            CTC_BIT_SET(profile->valid_bitmap, i);
            profile->mac_info[i].prefix_idx = routerMacDecode - 1;
        }
        else
        {
            CTC_BIT_UNSET(profile->valid_bitmap, i);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_l3if_rtmac_write_asic_mac(uint8 lchip, uint16 l3if_id, sys_l3if_rtmac_profile_t *profile)
{
    uint8 i = 0;
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint32 index = 0;
    DsRouterMac_m ingress_router_mac;

    for(i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        SYS_USW_SET_HW_MAC(mac, profile->mac_info[i].mac);
        if(!CTC_IS_BIT_SET(profile->valid_bitmap, i))
        {
            SetDsRouterMac(V, routerMacDecode0_f + i, &ingress_router_mac, 0);
        }
        else if(profile->mac_info[i].prefix_idx == 0)
        {
              SetDsRouterMac(V, routerMacDecode0_f + i, &ingress_router_mac, 1);
        }
        else
        {
              SetDsRouterMac(V, routerMacDecode0_f + i, &ingress_router_mac, profile->mac_info[i].prefix_idx + 1);
        }

        switch(i)
        {
        case 0:
            SetDsRouterMac(A, routerMac0_f, &ingress_router_mac, mac);
            break;

        case 1:
            SetDsRouterMac(A, routerMac1_f, &ingress_router_mac, mac);
            break;

        case 2:
            SetDsRouterMac(A, routerMac2_f, &ingress_router_mac, mac);
            break;

        case 3:
            SetDsRouterMac(A, routerMac3_f, &ingress_router_mac, mac);
            break;

        case 4:
            SetDsRouterMac(A, routerMac4_f, &ingress_router_mac, mac);
            break;

        case 5:
            SetDsRouterMac(A, routerMac5_f, &ingress_router_mac, mac);
            break;

        case 6:
            SetDsRouterMac(A, routerMac6_f, &ingress_router_mac, mac);
            break;

        case 7:
            SetDsRouterMac(A, routerMac7_f, &ingress_router_mac, mac);
            break;

        }

    }
    cmd = DRV_IOW(DsRouterMac_t, DRV_ENTRY_FLAG);
    index = profile->profile_id;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ingress_router_mac));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_l3if_rtmac_read_asic_mac(uint8 lchip, uint16 profile_id, sys_l3if_rtmac_profile_t *profile)
{
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 mac_en = 0;
    uint8 i = 0;
    uint8 step1 = 0, step2 = 0;
    DsRouterMac_m ingress_router_mac;
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ingress_router_mac, 0, sizeof(DsRouterMac_m));
    cmd = DRV_IOR(DsRouterMac_t, DRV_ENTRY_FLAG);
    index = profile->profile_id;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ingress_router_mac));

    step1 = DsRouterMac_routerMac1En_f - DsRouterMac_routerMac0En_f;
    step2 = DsRouterMac_routerMac1_f - DsRouterMac_routerMac0_f;
    for(i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        mac_en = 0;
        GetDsRouterMac(A, routerMac0En_f + i*step1, &ingress_router_mac, &mac_en);
        if (mac_en)
        {
            CTC_BIT_SET(profile->valid_bitmap, i);
            sal_memset(mac, 0, sizeof(mac));
            GetDsRouterMac(A, routerMac0_f + i*step2, &ingress_router_mac, mac);
            SYS_USW_SET_USER_MAC(profile->mac_info[i].mac, mac);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_l3if_rtmac_write_asic_mac(uint8 lchip, uint16 l3if_id, sys_l3if_rtmac_profile_t *profile)
{
    uint8 i = 0;
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint32 value_32 = 0;
    DsRouterMac_m ingress_router_mac;

    for(i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        SYS_USW_SET_HW_MAC(mac, profile->mac_info[i].mac);
        value_32 = CTC_IS_BIT_SET(profile->valid_bitmap, i) ? 1 : 0;
        switch(i)
        {
        case 0:
            SetDsRouterMac(A, routerMac0_f, &ingress_router_mac, mac);
            SetDsRouterMac(V, routerMac0En_f, &ingress_router_mac, value_32);
            break;

        case 1:
            SetDsRouterMac(A, routerMac1_f, &ingress_router_mac, mac);
            SetDsRouterMac(V, routerMac1En_f, &ingress_router_mac, value_32);
            break;

        case 2:
            SetDsRouterMac(A, routerMac2_f, &ingress_router_mac, mac);
            SetDsRouterMac(V, routerMac2En_f, &ingress_router_mac, value_32);
            break;

        case 3:
            SetDsRouterMac(A, routerMac3_f, &ingress_router_mac, mac);
            SetDsRouterMac(V, routerMac3En_f, &ingress_router_mac, value_32);
            break;
        default:
            return CTC_E_INVALID_PARAM;
        }

    }
    value_32 = profile->profile_id;
    cmd = DRV_IOW(DsRouterMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, value_32, cmd, &ingress_router_mac));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_rtmac_write_asic_interface(uint8 lchip, uint16 l3if_id, uint8 bmp, uint16 profile_id)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    /*write l3if selbitmap*/
    field_val = bmp;
    cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_routerMacSelBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3if_id, cmd, &field_val));

    /*write profile id*/
    field_val = profile_id;
    cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_routerMacProfile_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3if_id, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_traverse_rtmac_merge_profile(void *node, void* user_data)
{
    sys_l3if_merger_info_t *merger_info = (sys_l3if_merger_info_t *)(user_data);
    sys_l3if_rtmac_profile_t *node_profile = (sys_l3if_rtmac_profile_t *)(((ctc_spool_node_t*)node)->data);

    uint8 p1_bmp = 0, p0_bmp = 0;

    if (node_profile == merger_info->new_profile)
    {
        return CTC_E_NONE;
    }

    /*cmp*/
    p0_bmp = node_profile->valid_bitmap;

    _sys_usw_l3if_rtmac_profile_mapping(node_profile, merger_info->new_profile, &p0_bmp, &p1_bmp);

    if (!p0_bmp)
    {
        merger_info->p_profile[merger_info->cnt] = node_profile;
        merger_info->bitmap[merger_info->cnt] = p1_bmp;
        merger_info->cnt++;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_rtmac_merge_profile(uint8 lchip, uint16 l3if_id, sys_l3if_rtmac_profile_t *get_profile)
{
    uint16 i = 0;
    uint16 j = 0;
    uint8  k = 0;
    uint32 profile_cnt = 0;
    sys_l3if_merger_info_t merger_info;
    sys_l3if_rtmac_profile_t *node_profile = NULL;
    sys_l3if_rtmac_profile_t *merger_new_profile = NULL;
    sal_memset(&merger_info, 0, sizeof(merger_info));

    get_profile->new_node = 0;
    merger_info.new_profile = get_profile;
    ctc_spool_traverse(p_usw_l3if_master[lchip]->rtmac_spool, _sys_usw_l3if_traverse_rtmac_merge_profile , &merger_info);
    if (merger_info.cnt == 0)
    {
        return CTC_E_NONE;
    }

    for (i = 0; i <= MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID); i++)
    {
        if (i == l3if_id)
        {
            continue;
        }

        for (j = 0; j < merger_info.cnt; j++)/*profile*/
        {
            node_profile = p_usw_l3if_master[lchip]->l3if_prop[i].p_rtmac_profile;
            if (merger_info.p_profile[j] != node_profile)
            {
                continue;
            }
            _sys_usw_l3if_rtmac_write_asic_interface( lchip,  i, merger_info.bitmap[j], get_profile->profile_id);

            for (k = 0; k < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY) ; k++)
            {
                if (CTC_IS_BIT_SET(merger_info.bitmap[j], k))
                {
                    get_profile->mac_ref[k] += merger_info.p_profile[j]->mac_ref[k];
                }
            }

            p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_rtmac_profile = get_profile;

        }
    }

    for (j = 0; j < merger_info.cnt; j++)
    {
        _sys_usw_l3if_rtmac_dump_profile(lchip , merger_info.p_profile[j]);
        profile_cnt = ctc_spool_get_refcnt(p_usw_l3if_master[lchip]->rtmac_spool, merger_info.p_profile[j]);

        for (i = 0; i < profile_cnt ; i++)
        {
            CTC_ERROR_RETURN(ctc_spool_remove(p_usw_l3if_master[lchip]->rtmac_spool, merger_info.p_profile[j], NULL));

        }

        for (i = 0; i < profile_cnt ;  i++)
        {
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->rtmac_spool, get_profile, NULL, &merger_new_profile));
            if (merger_new_profile != get_profile)
            {
                _sys_usw_l3if_rtmac_dump_profile(lchip, merger_new_profile);
                _sys_usw_l3if_rtmac_dump_profile(lchip, get_profile);
                return CTC_E_PARAM_CONFLICT;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_l3if_rtmac_profile_make(sys_l3if_rtmac_profile_t* p_profile)
{
    return 0;
}

STATIC bool
_sys_usw_l3if_rtmac_profile_compare(sys_l3if_rtmac_profile_t* p1, /*DB*/
                                    sys_l3if_rtmac_profile_t* p0)
{
    uint8 p0_valid_num = 0;
    uint8 p1_valid_num = 0;
    uint8 p1_bmp = 0, p0_bmp = 0;
    uint8 lchip =  0;

    if (!p0 || !p1)
    {
        return FALSE;
    }

    lchip = p1->lchip;
    p0_bmp = p0->valid_bitmap;

    _sys_usw_l3if_rtmac_profile_mapping(p0, p1, &p0_bmp, &p1_bmp);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p0_bmp= [0x%x], p1_bmp= [0x%x], p1->profile_id=[%d]\n", p0_bmp, p1_bmp, p1->profile_id);
    p0_valid_num = _sys_usw_l3if_bmp2num( p0->valid_bitmap);
    p1_valid_num = _sys_usw_l3if_bmp2num( p1->valid_bitmap);
    if (p0->match_mode == 1)
    {
        if (!p0_bmp && p0_valid_num == p1_valid_num)
        {
            return TRUE;
        }
    }
    else if(p0->match_mode == 0)
    {
        if (!p0_bmp)
        {
            return TRUE;
        }
    }
    else
    {   /*lookup void whole mac position*/
        if (_sys_usw_l3if_bmp2num(p0_bmp) <= (MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY) - p1_valid_num))
        {
            return TRUE;
        }
    }


    return FALSE;
}

STATIC int32
_sys_usw_l3if_rtmac_profile_build_index(sys_l3if_rtmac_profile_t* p_profile, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = 0;
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_mac_profile;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_profile->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_profile->profile_id = value_32 & 0xFFFF;
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc  p_profile->profile_id=[%d]\n",  p_profile->profile_id);
    }

    p_profile->new_node = 1;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_rtmac_profile_free_index(sys_l3if_rtmac_profile_t* p_profile, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_profile);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = 0;
    opf.pool_type  = p_usw_l3if_master[*p_lchip]->opf_type_mac_profile;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free  p_profile->profile_id=[%d]\n",  p_profile->profile_id);
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_profile->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_clear_interface_rtmac(uint8 lchip, uint16 l3if_id)
{
    sys_l3if_rtmac_profile_t *old_profile = NULL;

    old_profile = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_rtmac_profile;

    if (old_profile)
    {
        old_profile->match_mode = 1;/*exatly*/
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_l3if_master[lchip]->rtmac_spool, old_profile, NULL));
        p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_rtmac_profile = NULL;
        p_usw_l3if_master[lchip]->l3if_prop[l3if_id].rtmac_bmp = 0;
    }
    return CTC_E_NONE;
}
int32
_sys_usw_l3if_dump_node_data(void* node, sys_dump_db_traverse_param_t* user_data)
{
    sal_file_t p_file = (sal_file_t)user_data->value0;
    sys_l3if_profile_t* acl_profile = NULL;
    sys_l3if_rtmac_profile_t* rtmac_profile = NULL;
    sys_l3if_macinner_profile_t* egs_rtmac_profile = NULL;
    uint32* ptr = NULL;
    uint8 i = 0;
    uint8 j = 0;
    uint8* mode = (uint8 *)user_data->value2;
    uint32* cnt = (uint32 *)user_data->value1;
    uint32 refcnt = ((ctc_spool_node_t*)node)->ref_cnt;
    /*acl profile*/
    if(*mode == 1)
    {
        acl_profile = (sys_l3if_profile_t*)((ctc_spool_node_t*)node)->data;
        ptr = (uint32 *)&(acl_profile->profile);
        if (refcnt == 0xFFFFFFFF)
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-7s%-15u%-7u0x", *cnt, "STATIC", acl_profile->profile_id, acl_profile->dir);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-15u%-7u0x", *cnt, refcnt, acl_profile->profile_id, acl_profile->dir);
        }
        for(i=0; i<=3; i++)
        {
            SYS_DUMP_DB_LOG(p_file, "%08x", *ptr);
            ptr++;
        }
        SYS_DUMP_DB_LOG(p_file, "\n");
        (*cnt)++;
    }
    /*rtmac profile*/
    if(*mode == 2)
    {
        rtmac_profile = (sys_l3if_rtmac_profile_t*)((ctc_spool_node_t*)node)->data;
        SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-7u0x%02x      %-15u%-10u%-15u%-10u%-12u0x%02x         %02x:", *cnt,
            refcnt,
            rtmac_profile->lchip,
            rtmac_profile->valid_bitmap,
            rtmac_profile->match_mode,
            rtmac_profile->new_node,
            rtmac_profile->profile_id,
            rtmac_profile->mac_ref[0],
            rtmac_profile->mac_info[0].prefix_idx,
            rtmac_profile->mac_info[0].postfix_bmp,
            rtmac_profile->mac_info[0].mac[0]);
        for(i=1; i<5; i++)
        {
            SYS_DUMP_DB_LOG(p_file, "%02x:", rtmac_profile->mac_info[0].mac[i]);
        }
        SYS_DUMP_DB_LOG(p_file, "%02x\n", rtmac_profile->mac_info[0].mac[i]);
        for(i=1; i<SYS_ROUTER_MAC_NUM_PER_ENTRY; i++)
        {
            SYS_DUMP_DB_LOG(p_file, "%-71s%-10u%-12u0x%02x         %02x:", " ", rtmac_profile->mac_ref[i],
                rtmac_profile->mac_info[i].prefix_idx,
                rtmac_profile->mac_info[i].postfix_bmp,
                rtmac_profile->mac_info[i].mac[0]);
            for(j = 1; j < 5; j++)
            {
                SYS_DUMP_DB_LOG(p_file, "%02x:", rtmac_profile->mac_info[i].mac[j]);
            }
            SYS_DUMP_DB_LOG(p_file, "%02x\n", rtmac_profile->mac_info[i].mac[j]);
        }
        (*cnt)++;

    }
    /*egress/inner rtmac profile*/
    if(*mode == 3)
    {
        egs_rtmac_profile = (sys_l3if_macinner_profile_t*)((ctc_spool_node_t*)node)->data;
        SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-7u%-12u%02x:", *cnt, refcnt, egs_rtmac_profile->type, egs_rtmac_profile->profile_id, egs_rtmac_profile->mac[0]);
        for(i=1; i<5; i++)
        {
            SYS_DUMP_DB_LOG(p_file, "%02x:", egs_rtmac_profile->mac[i]);
        }
        SYS_DUMP_DB_LOG(p_file, "%02x\n", egs_rtmac_profile->mac[5]);
        (*cnt)++;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l3if_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 cnt = 1;
    uint8 mode = 1;
    uint16 i = 0;
    uint8 j = 0;
    sys_dump_db_traverse_param_t    cb_data;
    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    cb_data.value1 = &cnt;
    cb_data.value2 = &mode;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IF_LOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# L3IF");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Max vrf number", p_usw_l3if_master[lchip]->max_vrfid_num);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Opf type l3if profile", p_usw_l3if_master[lchip]->opf_type_l3if_profile);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Opf type mac profile", p_usw_l3if_master[lchip]->opf_type_mac_profile);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Opf type macinner profile", p_usw_l3if_master[lchip]->opf_type_macinner_profile);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Default entry mode", p_usw_l3if_master[lchip]->default_entry_mode);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 vrf enable", p_usw_l3if_master[lchip]->ipv4_vrf_en);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 vrf enable", p_usw_l3if_master[lchip]->ipv6_vrf_en);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "Mac prefix:");
    SYS_DUMP_DB_LOG(p_f, "%-15s%-10s\n", "Index", "Mac prefix(40bit)");
    for(i = 0; i < SYS_ROUTER_MAC_PREFIX_NUM; i++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-15u", i);
        for (j = 0; j < 5; j++)
        {
            SYS_DUMP_DB_LOG(p_f, "%02x:", p_usw_l3if_master[lchip]->mac_prefix[i][j]);
        }
        SYS_DUMP_DB_LOG(p_f, "%02x\n", p_usw_l3if_master[lchip]->mac_prefix[i][5]);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "L3if property:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-15s%-20s%-20s\n", "Cnt", "L3if id", "Vlan id", "Gport", "L3if type", "Vlan ptr", "Rtmac prf id", "Rtmac bitmap", "Egs rtmac prf id", "Use egs rtmac");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    for(i = 0; i < SYS_SPEC_L3IF_NUM; i++)
    {
        if(p_usw_l3if_master[lchip]->l3if_prop[i].vaild)
        {
            SYS_DUMP_DB_LOG(p_f, "%-10u%-10u%-10u%-10u%-10u%-10u%-15u0x%02x           %-20u%-20u\n", i,
                p_usw_l3if_master[lchip]->l3if_prop[i].l3if_id,
                p_usw_l3if_master[lchip]->l3if_prop[i].vlan_id,
                p_usw_l3if_master[lchip]->l3if_prop[i].gport,
                p_usw_l3if_master[lchip]->l3if_prop[i].l3if_type,
                p_usw_l3if_master[lchip]->l3if_prop[i].vlan_ptr,
                p_usw_l3if_master[lchip]->l3if_prop[i].p_rtmac_profile ? p_usw_l3if_master[lchip]->l3if_prop[i].p_rtmac_profile->profile_id : 0,
                p_usw_l3if_master[lchip]->l3if_prop[i].rtmac_bmp,
                p_usw_l3if_master[lchip]->l3if_prop[i].p_egs_rtmac_profile ? p_usw_l3if_master[lchip]->l3if_prop[i].p_egs_rtmac_profile->profile_id : 0,
                p_usw_l3if_master[lchip]->l3if_prop[i].user_egs_rtmac);
        }
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");


    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","acl_spool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    mode = 1;
    cnt = 1;
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-15s%-7s%-15s\n", "Cnt", "RefCnt","Profile id", "Dir", "Profile data");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_l3if_master[lchip]->acl_spool, (spool_traversal_fn)_sys_usw_l3if_dump_node_data , (void *)&cb_data);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    mode = 2;
    cnt = 1;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n", "rtmac_spool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-7s%-10s%-15s%-10s%-15s%-10s%-12s%-13s%-10s\n", "Cnt", "RefCnt", "Lchip", "Valid_bmp", "Match_mode", "New node", "Profile id", "Mac ref", "Prefix idx", "Postfix bmp", "Route mac");
    ctc_spool_traverse(p_usw_l3if_master[lchip]->rtmac_spool, (spool_traversal_fn)_sys_usw_l3if_dump_node_data , (void *)&cb_data);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    mode = 3;
    cnt = 1;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n", "macinner_spool");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-7s%-12s%-7s\n", "Cnt", "RefCnt", "Type", "Profile id", "Route mac");
    ctc_spool_traverse(p_usw_l3if_master[lchip]->macinner_spool, (spool_traversal_fn)_sys_usw_l3if_dump_node_data , (void *)&cb_data);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------------");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_l3if_master[lchip]->opf_type_l3if_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_l3if_master[lchip]->opf_type_mac_profile, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_l3if_master[lchip]->opf_type_macinner_profile, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    SYS_L3IF_UNLOCK;
    return ret;
}

/**
@brief    init l3 interface module
*/
int32
sys_usw_l3if_init(uint8 lchip, ctc_l3if_global_cfg_t* l3if_global_cfg)
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0;
    uint32 cmd = 0;
    uint32 entry_num = 0;
    uint32 entry_num1 = 0;
    uint32 src_if_cmd = 0;
    uint32 dest_if_cmd = 0;
    DsSrcInterface_m src_interface;
    DsDestInterface_m dest_interface;
    ctc_spool_t spool;
    sys_usw_opf_t opf;
    sys_l3if_profile_t rsv_profile;
    uint32 value = 0;
    uint16 i=0;

    LCHIP_CHECK(lchip);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (p_usw_l3if_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }

    p_usw_l3if_master[lchip] = (sys_l3if_master_t*)mem_malloc(MEM_L3IF_MODULE, sizeof(sys_l3if_master_t));
    if (NULL == p_usw_l3if_master[lchip])
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_l3if_master[lchip], 0, sizeof(sys_l3if_master_t));
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&rsv_profile, 0, sizeof(sys_l3if_profile_t));

    sal_mutex_create(&p_usw_l3if_master[lchip]->mutex);
    if (NULL == p_usw_l3if_master[lchip]->mutex)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    p_usw_l3if_master[lchip]->default_entry_mode = l3if_global_cfg->max_vrfid_num ? 0 : 1;
    if ((!l3if_global_cfg->max_vrfid_num) || (l3if_global_cfg->max_vrfid_num > SYS_L3IF_MAX_VRF_ID + 1))
    {
        p_usw_l3if_master[lchip]->max_vrfid_num = SYS_L3IF_MAX_VRF_ID + 1;
    }
    else
    {
        p_usw_l3if_master[lchip]->max_vrfid_num = l3if_global_cfg->max_vrfid_num;
    }
    p_usw_l3if_master[lchip]->ipv4_vrf_en  = l3if_global_cfg->ipv4_vrf_en;
    p_usw_l3if_master[lchip]->ipv6_vrf_en  = l3if_global_cfg->ipv6_vrf_en;

    /* set chip_capability */
    if(sys_usw_chip_get_rchain_en())
    {
        MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) = (p_usw_l3if_master[lchip]->max_vrfid_num < 127) ? p_usw_l3if_master[lchip]->max_vrfid_num : 127;
    }
    else
    {
        MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID) = p_usw_l3if_master[lchip]->max_vrfid_num - 1;
    }

    if(!DRV_IS_DUET2(lchip))
    {
        value = 0;
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchIpv6Discard_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchIpv4Discard_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchCheckIpv6En_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchCheckIpv4En_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchIpv4ExceptionEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMacMismatchIpv6ExceptionEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }


    /*RouterMac entry 0 is reserved for system router mac, sdk use system router mac as default router mac*/
    src_if_cmd = DRV_IOR(DsSrcInterface_t, DRV_ENTRY_FLAG);
    dest_if_cmd = DRV_IOR(DsDestInterface_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, src_if_cmd, &src_interface), ret, error0);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, dest_if_cmd, &dest_interface), ret, error0);

    SetDsSrcInterface(V, routerMacProfile_f, &src_interface, 0);
    SetDsDestInterface(V, routerMacProfile_f, &dest_interface, 0);

    src_if_cmd = DRV_IOW(DsSrcInterface_t, DRV_ENTRY_FLAG);
    dest_if_cmd = DRV_IOW(DsDestInterface_t, DRV_ENTRY_FLAG);

    for (index = 0; index < MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM); index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, src_if_cmd, &src_interface), ret, error0);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, dest_if_cmd, &dest_interface), ret, error0);
    }

    for (index = 0; index < MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM); index++)
    {
        p_usw_l3if_master[lchip]->l3if_prop[index].vaild = FALSE;
        p_usw_l3if_master[lchip]->l3if_prop[index].vlan_ptr = 0;
    }

    p_usw_l3if_master[lchip]->ecmp_group_vec = ctc_vector_init(4, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM)/4);

    /*init opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_l3if_master[lchip]->opf_type_l3if_profile, 2, "opf-l3if-profile"), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsSrcInterfaceProfile_t, &entry_num), ret, error0);
    if(entry_num)
    {
        opf.pool_index = 0;
        opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_l3if_profile;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error0);
    }
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsDestInterfaceProfile_t, &entry_num), ret, error0);
    if(entry_num)
    {
        opf.pool_index = 1;
        opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_l3if_profile;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error0);
    }

    /*init l3if profile spool*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = entry_num;
    spool.max_count = entry_num * 2;
    spool.user_data_size = sizeof(sys_l3if_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_l3if_profile_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_l3if_profile_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_l3if_profile_build_index;
    spool.spool_free= (spool_free_fn)_sys_usw_l3if_profile_free_index;
    p_usw_l3if_master[lchip]->acl_spool = ctc_spool_create(&spool);

    rsv_profile.dir = CTC_INGRESS;
    CTC_ERROR_GOTO(ctc_spool_static_add(p_usw_l3if_master[lchip]->acl_spool, &rsv_profile), ret, error0);
    rsv_profile.dir = CTC_EGRESS;
    CTC_ERROR_GOTO(ctc_spool_static_add(p_usw_l3if_master[lchip]->acl_spool, &rsv_profile), ret, error0);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_l3if_master[lchip]->opf_type_mac_profile, 1, "opf-routermac-profile"), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsRouterMac_t, &entry_num), ret, error0);
    if(entry_num)
    {
        opf.pool_index = 0;
        opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_mac_profile;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num- 1), ret, error0);
    }
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = entry_num;
    spool.user_data_size = sizeof(sys_l3if_rtmac_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_l3if_rtmac_profile_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_l3if_rtmac_profile_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_l3if_rtmac_profile_build_index;
    spool.spool_free= (spool_free_fn)_sys_usw_l3if_rtmac_profile_free_index;
    p_usw_l3if_master[lchip]->rtmac_spool = ctc_spool_create(&spool);

    /*init macinner db & egs router mac  db*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_l3if_master[lchip]->opf_type_macinner_profile, 2, "opf-macinner-profile"), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsRouterMacInner_t, &entry_num), ret, error0);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsEgressRouterMac_t, &entry_num1), ret, error0);
    if(entry_num)
    {
        opf.pool_index = 0;
        opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_macinner_profile;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num-1), ret, error0);
    }
    if(entry_num1)
    {
        opf.pool_index = 1;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num1-1), ret, error0);
    }
    spool.lchip = lchip;
    spool.block_num = 16;
    spool.block_size = (entry_num + entry_num1)/16;
    spool.max_count = entry_num + entry_num1;
    spool.user_data_size = sizeof(sys_l3if_macinner_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_l3if_macinner_profile_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_l3if_macinner_profile_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_l3if_macinner_profile_build_index;
    spool.spool_free= (spool_free_fn)_sys_usw_l3if_macinner_profile_free_index;
    p_usw_l3if_master[lchip]->macinner_spool = ctc_spool_create(&spool);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_L3IF, sys_usw_l3if_wb_sync), ret, error0);

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(sys_usw_l3if_wb_restore(lchip), ret, error0);
    }

    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_L3IF, sys_usw_l3if_dump_db), ret, error0);

    if(sys_usw_chip_get_rchain_en() && lchip)
    {
        ctc_l3if_t l3_if;
        ctc_l3if_property_t l3if_prop;
        sal_memset(&l3_if, 0, sizeof(ctc_l3if_t));
        sal_memset(&l3if_prop, 0, sizeof(ctc_l3if_property_t));
        /* rchain mode support 128 vrf */
        for (i = 0; i < 128; i++)
        {
            l3_if.gport = 100 + i;
            l3_if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
            CTC_ERROR_RETURN(sys_usw_l3if_create(lchip, l3_if.gport, &l3_if));
            value = 1;
            l3if_prop = CTC_L3IF_PROP_ROUTE_ALL_PKT;
            CTC_ERROR_RETURN(sys_usw_l3if_set_property(lchip, l3_if.gport, l3if_prop, value));
            l3if_prop = CTC_L3IF_PROP_IPV4_UCAST;
            CTC_ERROR_RETURN(sys_usw_l3if_set_property(lchip, l3_if.gport, l3if_prop, value));
            l3if_prop = CTC_L3IF_PROP_IPV6_UCAST;
            CTC_ERROR_RETURN(sys_usw_l3if_set_property(lchip, l3_if.gport, l3if_prop, value));
            value = i;
            l3if_prop = CTC_L3IF_PROP_VRF_ID;
            CTC_ERROR_RETURN(sys_usw_l3if_set_property(lchip, l3_if.gport, l3if_prop, value));
            value = 1;
            l3if_prop = CTC_L3IF_PROP_VRF_EN;
            CTC_ERROR_RETURN(sys_usw_l3if_set_property(lchip, l3_if.gport, l3if_prop, value));
        }
    }

    return CTC_E_NONE;

error0:
    if (p_usw_l3if_master[lchip]->macinner_spool)
    {
        ctc_spool_free(p_usw_l3if_master[lchip]->macinner_spool);
    }

    if (p_usw_l3if_master[lchip]->opf_type_macinner_profile)
    {
        sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_macinner_profile);
    }

    if (p_usw_l3if_master[lchip]->rtmac_spool)
    {
        ctc_spool_free(p_usw_l3if_master[lchip]->rtmac_spool);
    }

    if (p_usw_l3if_master[lchip]->opf_type_mac_profile)
    {
        sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_mac_profile);
    }

    if (p_usw_l3if_master[lchip]->acl_spool)
    {
        ctc_spool_free(p_usw_l3if_master[lchip]->acl_spool);
    }

    if (p_usw_l3if_master[lchip]->opf_type_l3if_profile)
    {
        sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_l3if_profile);
    }

    if (p_usw_l3if_master[lchip]->ecmp_group_vec)
    {
        ctc_vector_release(p_usw_l3if_master[lchip]->ecmp_group_vec);
    }

    if (p_usw_l3if_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_l3if_master[lchip]->mutex);
    }

    if (p_usw_l3if_master[lchip])
    {
        mem_free(p_usw_l3if_master[lchip]);
    }

    return ret;
}

STATIC int32
_sys_usw_l3if_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

/**
@brief    deinit l3 interface module
*/
int32
sys_usw_l3if_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_l3if_master[lchip])
    {
        return CTC_E_NONE;
    }
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*free ecmp group*/
    ctc_vector_traverse(p_usw_l3if_master[lchip]->ecmp_group_vec, (vector_traversal_fn)_sys_usw_l3if_free_node_data, NULL);
    ctc_vector_release(p_usw_l3if_master[lchip]->ecmp_group_vec);

    ctc_spool_free(p_usw_l3if_master[lchip]->acl_spool);
    ctc_spool_free(p_usw_l3if_master[lchip]->macinner_spool);
    ctc_spool_free(p_usw_l3if_master[lchip]->rtmac_spool);

    sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_l3if_profile);
    sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_mac_profile);
    sys_usw_opf_deinit(lchip, p_usw_l3if_master[lchip]->opf_type_macinner_profile);

    if (p_usw_l3if_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_l3if_master[lchip]->mutex);
    }

    mem_free(p_usw_l3if_master[lchip]);

    return CTC_E_NONE;
}

/**
@brief  Create l3 interfaces, gport should be global logical port
*/
int32
sys_usw_l3if_create(uint8 lchip, uint16 l3if_id, ctc_l3if_t* l3_if)
{
    int32 ret = CTC_E_NONE;
    uint16 vlan_ptr = 0;
    uint16 index;
    uint32 src_if_cmd = 0;
    uint32 dest_if_cmd = 0;
    DsSrcInterface_m src_interface;
    DsDestInterface_m dest_interface;
    ctc_scl_entry_t   scl_entry;
    ctc_field_key_t   field_key = {0};
    ctc_scl_field_action_t field_action = {0};
    ctc_field_port_t   field_port = {0};
    uint16 exception3_en = 0;
    uint8 subif_use_scl = 0;
    uint8 gchip = 0;

    SYS_L3IF_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l3_if);
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    CTC_MAX_VALUE_CHECK(l3if_id, 4092);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_type:%d ,l3if_id :%d port:0x%x, vlan_id:%d \n",
                      l3_if->l3if_type, l3if_id, l3_if->gport, l3_if->vlan_id);

    if (l3_if->l3if_type == CTC_L3IF_TYPE_PHY_IF
        || l3_if->l3if_type == CTC_L3IF_TYPE_SUB_IF)
    {
        SYS_GLOBAL_PORT_CHECK(l3_if->gport);
    }

    if (l3_if->l3if_type == CTC_L3IF_TYPE_VLAN_IF
        || l3_if->l3if_type == CTC_L3IF_TYPE_SUB_IF)
    {
        CTC_VLAN_RANGE_CHECK(l3_if->vlan_id);
    }

    SYS_L3IF_LOCK;
    if (TRUE == p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vaild)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Already exist! \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_EXIST);
    }

    for (index = 0; index < MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM); index++)
    {
        if (CTC_L3IF_TYPE_SERVICE_IF == l3_if->l3if_type)
        {
            break;
        }
        if ((p_usw_l3if_master[lchip]->l3if_prop[index].vaild) &&
            (p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == l3_if->l3if_type) &&
            (p_usw_l3if_master[lchip]->l3if_prop[index].gport == l3_if->gport) &&
            (p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == l3_if->vlan_id))
        {
            SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Already exist! \n");
            CTC_RETURN_L3IF_UNLOCK(CTC_E_EXIST);
        }
    }
    SYS_L3IF_UNLOCK;

    switch (l3_if->l3if_type)
    {
    case CTC_L3IF_TYPE_PHY_IF:
        {
            vlan_ptr  = 0x1000 + l3if_id;
            break;
        }

    case CTC_L3IF_TYPE_SUB_IF:
        {
            vlan_ptr  = 0x1000 + l3if_id;

            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(l3_if->gport);
            if(CTC_IS_LINKAGG_PORT(l3_if->gport) || sys_usw_chip_is_local(lchip, gchip))
            {
                subif_use_scl = 1;
            }
            break;
        }

    case CTC_L3IF_TYPE_VLAN_IF:
        {
            uint32 tmp_l3if_id = 0;
            sys_usw_vlan_get_vlan_ptr(lchip, l3_if->vlan_id, &vlan_ptr);
            if (!vlan_ptr)
            {
                return CTC_E_NOT_EXIST;
            }

            ret = sys_usw_vlan_get_internal_property(lchip, vlan_ptr, SYS_VLAN_PROP_INTERFACE_ID, &tmp_l3if_id);
            if (ret == CTC_E_NONE
                && (tmp_l3if_id != SYS_L3IF_INVALID_L3IF_ID)
                && (tmp_l3if_id != l3if_id))
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Already exist! \n");
                return CTC_E_EXIST;
            }
            else if (ret == CTC_E_NONE)
            {
                CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_ptr, SYS_VLAN_PROP_INTERFACE_ID, l3if_id));
            }
            else
            {
                return ret;
            }
            break;
        }
    case CTC_L3IF_TYPE_SERVICE_IF:
        {
            vlan_ptr  = 0x1000 + l3if_id;
            break;
        }
    default:
        return CTC_E_INVALID_PARAM;
    }

    SYS_L3IF_LOCK;
    /*only save the last port property*/
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].gport = l3_if->gport;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_id = l3_if->vlan_id;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type = l3_if->l3if_type;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_ptr = vlan_ptr;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_id  = l3if_id;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].rtmac_bmp = 0;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_rtmac_profile = NULL;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile = NULL;
    SYS_L3IF_UNLOCK;

    if (CTC_L3IF_TYPE_SUB_IF == l3_if->l3if_type && subif_use_scl)
    {
        uint32 group_id = 0;
        ctc_scl_group_info_t hash_group = {0};

        group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_L3IF, 0, 0, 0, 0);
        hash_group.type = CTC_SCL_GROUP_TYPE_NONE;
        ret = sys_usw_scl_create_group(lchip, group_id, &hash_group, 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            return ret;
        }

        sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

        scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
        CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, group_id, &scl_entry, 1));

        /* KEY */
        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_port.gport = l3_if->gport;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error1);

        sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = l3_if->vlan_id;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error1);

        sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error1);

        /* ACTION */
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
        field_action.data0 = vlan_ptr;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error1);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error1);
    }

    src_if_cmd = DRV_IOR(DsSrcInterface_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, src_if_cmd, &src_interface), ret, error0);
    dest_if_cmd = DRV_IOR(DsDestInterface_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, dest_if_cmd, &dest_interface), ret, error0);

    /*Config SrcInterface default value*/
    exception3_en = 1 << CTC_L3PDU_ACTION_INDEX_RIP;   /* bit 0 always set to 1, 0 reserved for rip to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_LDP;  /* bit 1 always set to 1, 1 reserved for ldp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_OSPF; /* bit 2 always set to 1, 2 reserved for ospf to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_PIM;  /* bit 3 always set to 1, 3 reserved for pim to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_BGP;  /* bit 4 always set to 1, 4 reserved for bgp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_RSVP; /* bit 5 always set to 1, 5 reserved for rsvp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_MSDP; /* bit 6 always set to 1, 6 reserved for msdp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_IGMP; /* bit 7 always set to 1, 7 reserved for igmp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_MLD;  /* bit 7 always set to 1, 7 reserved for mld to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_VRRP; /* bit 8 always set to 1, 8 reserved for vrrp to cpu */
    exception3_en |= 1 << CTC_L3PDU_ACTION_INDEX_ISIS; /* bit 9 always set to 1, 9 reserved for isis to cpu */
    SetDsSrcInterface(V, exception3En_f, &src_interface, exception3_en);
    SetDsSrcInterface(V, routerMacProfile_f, &src_interface, 0);
    SetDsSrcInterface(V, routeLookupMode_f, &src_interface, 0);
    SetDsSrcInterface(V, vrfId_f, &src_interface, 0);
    SetDsSrcInterface(V, v4UcastEn_f, &src_interface, 1);
    SetDsSrcInterface(V, routeDisable_f, &src_interface, 0);
    SetDsSrcInterface(V, routerMacSelBitmap_f, &src_interface, 0x1);/*refer to system router mac*/

    /*Config DestInterface default value*/
    SetDsDestInterface(V, routerMacProfile_f, &dest_interface, 0);
    if (CTC_L3IF_TYPE_VLAN_IF == l3_if->l3if_type || CTC_L3IF_TYPE_SUB_IF == l3_if->l3if_type          /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-21 */
        || (MAX_CTC_L3IF_ID -2) == l3if_id)
    {
        SetDsDestInterface(V, vlanId_f, &dest_interface, l3_if->vlan_id);
        SetDsDestInterface(V, interfaceSvlanTagged_f, &dest_interface, 1);
    }

    src_if_cmd = DRV_IOW(DsSrcInterface_t, DRV_ENTRY_FLAG);
    dest_if_cmd = DRV_IOW(DsDestInterface_t, DRV_ENTRY_FLAG);

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, src_if_cmd, &src_interface), ret, error0);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, dest_if_cmd, &dest_interface), ret, error0);

    SYS_L3IF_LOCK;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vaild = TRUE;
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;

error1:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

error0:
    sys_usw_l3if_delete(lchip, l3if_id, l3_if);

    return ret;
}

/**
@brief    Delete  l3 interfaces
*/
int32
sys_usw_l3if_delete(uint8 lchip, uint16 l3if_id, ctc_l3if_t* l3_if)
{
    uint32 src_if_cmd = 0;
    uint32 dest_if_cmd = 0;
    DsSrcInterface_m src_interface;
    DsDestInterface_m dest_interface;
    ctc_field_key_t field_key = {0};
    ctc_field_port_t   field_port = {0};
    sys_scl_lkup_key_t lkup_key = {0};
    uint8 subif_use_scl = 0;
    uint8 gchip = 0;
    uint32 cmd = 0;
    uint32 profile_id = 0;
    sys_l3if_profile_t  profile;

    SYS_L3IF_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l3_if);
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_type:%d ,l3if_id :%d port:0x%x, vlan_id:%d \n",
                                           l3_if->l3if_type, l3if_id, l3_if->gport, l3_if->vlan_id);

    sal_memset(&src_interface, 0, sizeof(DsSrcInterface_m));
    sal_memset(&dest_interface, 0, sizeof(DsDestInterface_m));

    if (l3_if->l3if_type == CTC_L3IF_TYPE_PHY_IF
        || l3_if->l3if_type == CTC_L3IF_TYPE_SUB_IF)
    {
        SYS_GLOBAL_PORT_CHECK(l3_if->gport);
    }

    if (l3_if->l3if_type == CTC_L3IF_TYPE_VLAN_IF
        || l3_if->l3if_type == CTC_L3IF_TYPE_SUB_IF)
    {
        CTC_VLAN_RANGE_CHECK(l3_if->vlan_id);
    }

    SYS_L3IF_LOCK;
    switch (l3_if->l3if_type)
    {
    case CTC_L3IF_TYPE_PHY_IF:
        {
            if ((p_usw_l3if_master[lchip]->l3if_prop[l3if_id].gport != l3_if->gport)
                || (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != l3_if->l3if_type))
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Information is mismatch \n");
                CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_CONFIG);
            }

            break;
        }

    case CTC_L3IF_TYPE_SUB_IF:
        {
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(l3_if->gport);

            if(CTC_IS_LINKAGG_PORT(l3_if->gport) || sys_usw_chip_is_local(lchip, gchip))
            {
                subif_use_scl = 1;
            }

            if ((p_usw_l3if_master[lchip]->l3if_prop[l3if_id].gport != l3_if->gport)
                || (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_id != l3_if->vlan_id)
                || (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != l3_if->l3if_type))
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Information is mismatch \n");
                CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_CONFIG);
            }

            break;
        }

    case CTC_L3IF_TYPE_VLAN_IF:
        {
            if ((p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_id != l3_if->vlan_id)
               || (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != l3_if->l3if_type))
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Information is mismatch \n");
                CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_CONFIG);
            }

            sys_usw_vlan_set_internal_property(lchip, l3_if->vlan_id, SYS_VLAN_PROP_INTERFACE_ID, 0);
            break;
        }
    case CTC_L3IF_TYPE_SERVICE_IF:
        {
            if (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != l3_if->l3if_type)
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Information is mismatch \n");
                CTC_RETURN_L3IF_UNLOCK(CTC_E_L3IF_MISMATCH);
            }
        }
        break;
    default:
        CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
    }

    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vaild = FALSE;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_id = 0;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].vlan_ptr = 0;
    p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_id  = SYS_L3IF_INVALID_L3IF_ID;

    sal_memset(&profile, 0, sizeof(sys_l3if_profile_t));
    profile.dir = CTC_INGRESS;
    cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_profileId_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
    if(profile_id)
    {
        profile.profile_id = profile_id;
        cmd = DRV_IOR(DsSrcInterfaceProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &(profile.profile.src)));
        ctc_spool_remove(p_usw_l3if_master[lchip]->acl_spool, &profile, NULL);
    }

    sal_memset(&profile, 0, sizeof(sys_l3if_profile_t));
    profile.dir = CTC_EGRESS;
    cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_profileId_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
    if(profile_id)
    {
        profile.profile_id = profile_id;
        cmd = DRV_IOR(DsDestInterfaceProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &(profile.profile.dst)));
        ctc_spool_remove(p_usw_l3if_master[lchip]->acl_spool, &profile, NULL);
    }

    sal_memset(&src_interface, 0, sizeof(DsSrcInterface_m));
    sal_memset(&dest_interface, 0, sizeof(DsDestInterface_m));

    src_if_cmd = DRV_IOW(DsSrcInterface_t, DRV_ENTRY_FLAG);
    dest_if_cmd = DRV_IOW(DsDestInterface_t, DRV_ENTRY_FLAG);

    DRV_IOCTL(lchip, l3if_id, src_if_cmd, &src_interface);
    DRV_IOCTL(lchip, l3if_id, dest_if_cmd, &dest_interface);
    SYS_L3IF_UNLOCK;

    if ((CTC_L3IF_TYPE_SUB_IF == l3_if->l3if_type) && subif_use_scl)
    {
        lkup_key.key_type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        lkup_key.action_type = CTC_SCL_ACTION_INGRESS;
        lkup_key.group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_L3IF, 0, 0, 0, 0);

        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_port.gport = l3_if->gport;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

        sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = l3_if->vlan_id;
        sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

        sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

        sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

        sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key);
        sys_usw_scl_uninstall_entry(lchip, lkup_key.entry_id, 1);
        sys_usw_scl_remove_entry(lchip, lkup_key.entry_id, 1);
    }

    sys_usw_nh_del_ipmc_dsnh_offset(lchip, l3if_id);
    SYS_L3IF_LOCK;
    if (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].rtmac_bmp)
    {
        _sys_usw_l3if_clear_interface_rtmac(lchip, l3if_id);
    }
    _sys_usw_l3if_unbinding_egs_rtmac( lchip, l3if_id);
	SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_get_l3if_id(uint8 lchip, ctc_l3if_t* l3_if, uint16* l3if_id)
{
    uint16 index;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L3IF_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l3_if);
    CTC_PTR_VALID_CHECK(l3if_id);

    SYS_L3IF_LOCK;
    if(l3_if->l3if_type == 0xFF &&  (*l3if_id !=0) )
    { /*get l3if info by L3if id*/

        if (*l3if_id <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1) && p_usw_l3if_master[lchip]->l3if_prop[*l3if_id].vaild)
        {
            index = *l3if_id;
            l3_if->l3if_type = p_usw_l3if_master[lchip]->l3if_prop[*l3if_id].l3if_type;
            l3_if->gport = p_usw_l3if_master[lchip]->l3if_prop[*l3if_id].gport;
            l3_if->vlan_id = p_usw_l3if_master[lchip]->l3if_prop[*l3if_id].vlan_id;
        }
        else
        {
            index = MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM);
        }
    }
    else
    {/*get L3if id by l3if info*/
        for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
        {
            if (CTC_L3IF_TYPE_SERVICE_IF == l3_if->l3if_type)
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Can not get service-if! \n");
                CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
            }
            if ((p_usw_l3if_master[lchip]->l3if_prop[index].vaild) &&
                (p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == l3_if->l3if_type) &&
                (p_usw_l3if_master[lchip]->l3if_prop[index].gport == l3_if->gport) &&
                (p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == l3_if->vlan_id))
            {
                break;
            }
        }
    }
    *l3if_id = index;

    if (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM) == index)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Not exist! \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_EXIST);
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;

}


bool
sys_usw_l3if_is_port_phy_if(uint8 lchip, uint32 gport)
{
    uint16 index = 0;
    bool  ret = FALSE;

    SYS_L3IF_INIT_CHECK();

    SYS_L3IF_LOCK;
    for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
    {
        if (p_usw_l3if_master[lchip]->l3if_prop[index].vaild == TRUE
            && p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_PHY_IF
            && p_usw_l3if_master[lchip]->l3if_prop[index].gport == gport)
        {
            ret = TRUE;
            break;
        }
    }
    SYS_L3IF_UNLOCK;

    return ret;
}

bool
sys_usw_l3if_is_port_sub_if(uint8 lchip, uint32 gport)
{
    uint16 index = 0;
    bool  ret = FALSE;

    SYS_L3IF_INIT_CHECK();

    SYS_L3IF_LOCK;
    for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
    {
        if (p_usw_l3if_master[lchip]->l3if_prop[index].vaild == TRUE
            && p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_SUB_IF
            && p_usw_l3if_master[lchip]->l3if_prop[index].gport == gport)
        {
            ret = TRUE;
            break;
        }
    }
    SYS_L3IF_UNLOCK;

    return ret;
}

int32
sys_usw_l3if_get_l3if_info(uint8 lchip, uint8 query_type, sys_l3if_prop_t* l3if_prop)
{
    int32 ret = CTC_E_NOT_EXIST;
    uint16  index = 0;
    bool find = FALSE;

    SYS_L3IF_INIT_CHECK();

    SYS_L3IF_LOCK;
    if (query_type == 1)
    {
        if (p_usw_l3if_master[lchip]->l3if_prop[l3if_prop->l3if_id].vaild == TRUE)
        {
            find = TRUE;
            index = l3if_prop->l3if_id;
        }
    }
    else
    {
        for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
        {
            if (p_usw_l3if_master[lchip]->l3if_prop[index].vaild == FALSE ||
                p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type != l3if_prop->l3if_type)
            {
                continue;
            }

            if (l3if_prop->l3if_type == CTC_L3IF_TYPE_PHY_IF)
            {
                if (p_usw_l3if_master[lchip]->l3if_prop[index].gport == l3if_prop->gport)
                {
                    find = TRUE;
                    break;
                }
            }
            else if (l3if_prop->l3if_type == CTC_L3IF_TYPE_SUB_IF)
            {
                if (p_usw_l3if_master[lchip]->l3if_prop[index].gport == l3if_prop->gport &&
                    p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == l3if_prop->vlan_id)
                {
                    find = TRUE;
                    break;
                }
            }
            else if (l3if_prop->l3if_type == CTC_L3IF_TYPE_VLAN_IF)
            {
                if (p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == l3if_prop->vlan_id)
                {
                    find = TRUE;
                    break;
                }
            }
            else
            {
                continue;
            }
        }
    }

    if (find)
    {
        sal_memcpy(l3if_prop, &p_usw_l3if_master[lchip]->l3if_prop[index], sizeof(sys_l3if_prop_t));
        ret = CTC_E_NONE;
    }
    SYS_L3IF_UNLOCK;

    return ret;
}

int32
sys_usw_l3if_get_l3if_info_with_port_and_vlan(uint8 lchip, uint32 gport, uint16 vlan_id, sys_l3if_prop_t* l3if_prop)
{
    int32 ret = CTC_E_NOT_EXIST;
    uint16  index = 0;
    bool find = FALSE;

    SYS_L3IF_INIT_CHECK();

    if(sys_usw_chip_get_rchain_en() && lchip)
    {
        /*route chain mode, only chain head chip need l3if info*/
        return CTC_E_NONE;
    }
    SYS_L3IF_LOCK;
    for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
    {
        if (p_usw_l3if_master[lchip]->l3if_prop[index].vaild == FALSE)
        {
            continue;
        }
        if (p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_SERVICE_IF)
        {
            continue;
        }
        if (p_usw_l3if_master[lchip]->l3if_prop[index].gport == gport
            && p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_PHY_IF)
        {
            find = TRUE;
            break;
        }
        else if (p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_SUB_IF)
        {
            if (p_usw_l3if_master[lchip]->l3if_prop[index].gport == gport &&
                p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == vlan_id)
            {
                find = TRUE;
                break;
            }
        }
    }

    if(!find)
    {
        for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
        {
            if (p_usw_l3if_master[lchip]->l3if_prop[index].vaild == FALSE)
            {
                continue;
            }

            if (p_usw_l3if_master[lchip]->l3if_prop[index].vlan_id == vlan_id
                     && p_usw_l3if_master[lchip]->l3if_prop[index].l3if_type == CTC_L3IF_TYPE_VLAN_IF)
            {
                find = TRUE;
                break;
            }
        }
    }

    if (find)
    {
        sal_memcpy(l3if_prop, &p_usw_l3if_master[lchip]->l3if_prop[index], sizeof(sys_l3if_prop_t));
        ret = CTC_E_NONE;
    }
    SYS_L3IF_UNLOCK;

    return ret;
}

/**
@brief    Config  l3 interface's  properties
*/
int32
sys_usw_l3if_set_property(uint8 lchip, uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32 value)
{
    uint32 cmd[MAX_L3IF_RELATED_PROPERTY] = {0};
    uint32 idx = 0;
    uint32 field[MAX_L3IF_RELATED_PROPERTY] = {0};
    drv_work_platform_type_t    platform_type;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id :%d, l3if_prop:%d, value:%d \n", l3if_id, l3if_prop, value);

    SYS_L3IF_LOCK;
    switch (l3if_prop)
    {
    /*src  interface */
    case CTC_L3IF_PROP_IPV4_UCAST:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v4UcastEn_f);
        break;

    case CTC_L3IF_PROP_IPV4_MCAST:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v4McastEn_f);
        break;

    case CTC_L3IF_PROP_IPV4_SA_TYPE:
        if (value >= MAX_CTC_L3IF_IPSA_LKUP_TYPE)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v4UcastSaType_f);
        break;

    case CTC_L3IF_PROP_IPV6_UCAST:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v6UcastEn_f);
        break;

    case CTC_L3IF_PROP_IPV6_MCAST:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v6McastEn_f);
        break;

    case CTC_L3IF_PROP_IPV6_SA_TYPE:
        if (value >= MAX_CTC_L3IF_IPSA_LKUP_TYPE)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_v6UcastSaType_f);
        break;

    case CTC_L3IF_PROP_VRF_ID:
        if (value > MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID))
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_BADID);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_vrfId_f);
        break;

    case CTC_L3IF_PROP_ROUTE_EN:
        field[0] = (value) ? 0 : 1;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_routeDisable_f);
        break;

    case CTC_L3IF_PROP_NAT_IFTYPE:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_l3IfType_f);
        break;

    case CTC_L3IF_PROP_ROUTE_ALL_PKT:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_routeAllPackets_f);
        break;

    case CTC_L3IF_PROP_PBR_LABEL:
        if(DRV_IS_TSINGMA(lchip))
        {
            SYS_L3IF_UNLOCK;
            return CTC_E_NOT_SUPPORT;
        }
        if (value > MAX_CTC_L3IF_PBR_LABEL)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_pbrLabel_f);
        break;

    case CTC_L3IF_PROP_VRF_EN:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_routeLookupMode_f);
        break;

    case CTC_L3IF_PROP_MTU_EN:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_mtuCheckEn_f);
        break;

    case CTC_L3IF_PROP_MTU_EXCEPTION_EN:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_mtuExceptionEn_f);
        break;

    case CTC_L3IF_PROP_EGS_MCAST_TTL_THRESHOLD:
        if (value >= MAX_CTC_L3IF_MCAST_TTL_THRESHOLD)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_mcastTtlThreshold_f);
        break;

    case CTC_L3IF_PROP_MTU_SIZE:
        if (value >= MAX_CTC_L3IF_MTU_SIZE)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_mtuSize_f);
        break;

    case CTC_L3IF_PROP_MPLS_EN:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_mplsEn_f);
        break;

    case CTC_L3IF_PROP_MPLS_LABEL_SPACE:
        if (value > 0xFF)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_mplsLabelSpace_f);
        field[1] = value ? 1 : 0;
        cmd[1] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_interfaceLabelValid_f);
        break;

    case CTC_L3IF_PROP_RPF_CHECK_TYPE:
        if (!(value < SYS_RPF_CHK_TYPE_NUM))
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_rpfType_f);
        break;

    case CTC_L3IF_PROP_RPF_PERMIT_DEFAULT:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_rpfPermitDefault_f);
        break;

    case CTC_L3IF_PROP_IGMP_SNOOPING_EN:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_igmpSnoopEn_f);
        break;

    case CTC_L3IF_PROP_CONTEXT_LABEL_EXIST:
        field[0] = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_contextLabelExist_f);
        break;

    case CTC_L3IF_PROP_STATS:
        if (value == 0) /* disable stats */
        {
            field[0] = 0;
        }
        else    /* enable stats */
        {
            uint32 stats_ptr  = 0;
            CTC_ERROR_RETURN_L3IF_UNLOCK(sys_usw_flow_stats_get_statsptr(lchip, value, &stats_ptr));
            field[0] = stats_ptr;
        }
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_l3IfStatsPtr_f);
        break;

    case CTC_L3IF_PROP_EGS_STATS:
        if (value == 0) /* disable stats */
        {
            field[0] = 0;
        }
        else    /* enable stats */
        {
            uint32 stats_ptr  = 0;
            uint32 bpe_pe_mode = 0;

            CTC_ERROR_RETURN_L3IF_UNLOCK(sys_usw_flow_stats_get_statsptr(lchip, value, &stats_ptr));

            cmd[0] = DRV_IOR(EpeNextHopReserved_t, EpeNextHopReserved_reserved_f);
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0 , cmd[0], &bpe_pe_mode));
            bpe_pe_mode = (bpe_pe_mode&0x1)?1:0;
            if (bpe_pe_mode && stats_ptr)
            {
                /*bpe ecid enable, cannot support nh stats*/
                SYS_L3IF_UNLOCK;
                return CTC_E_INVALID_CONFIG;
            }

            field[0] = stats_ptr;
        }
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_statsPtr_f);
        break;

    case CTC_L3IF_PROP_CID:
        if ((value == CTC_ACL_UNKNOWN_CID) || (value > CTC_MAX_ACL_CID))
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_BADID);
        }
        if (value == 0)
        {
            field[0] = 0;
            field[1] = 0;
        }
        else
        {
            field[0] = 1;
            field[1] = value;
        }

        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_categoryIdValid_f);
        cmd[1] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_categoryId_f);
        break;
    case CTC_L3IF_PROP_PUB_ROUTE_LKUP_EN:
        field[0] = value ? 1: 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_ipPublicLookupEn_f);
        break;

    case CTC_L3IF_PROP_IPMC_USE_VLAN_EN:
        field[0] = value ? 1: 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_ipmcUseVlan_f);
        break;

    case CTC_L3IF_PROP_INTERFACE_LABEL_EN:
        field[0] = value ? 1: 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_interfaceLabelValid_f);
        break;

    case CTC_L3IF_PROP_PHB_USE_TUNNEL_HDR:
        field[0] = value ? 1: 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_phbUseOuterInfo_f);
        break;

    case CTC_L3IF_PROP_TRUST_DSCP:
        field[0] = value ? 1: 0;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_trustDscp_f);
        break;

    case CTC_L3IF_PROP_DSCP_SELECT_MODE:
        if (value >= MAX_CTC_DSCP_SELECT_MODE)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        break;

    case CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN:
        if (value > CTC_MAX_QOS_DSCP_DOMAIN)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        field[0] = value;
        cmd[0] = DRV_IOW(DsSrcInterface_t, DsSrcInterface_dscpPhbPtr_f);
        break;

    case CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN:
        if (value > CTC_MAX_QOS_DSCP_DOMAIN)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        cmd[0] = DRV_IOR(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd[0], &field[0]));
        /* SYSTEM MODIFIED: added by hansf for ignore l3if dscpRemarkMode when set dscp domain */
    #if 0
        if (field[0] != CTC_DSCP_SELECT_MAP)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
    #endif

        field[0] = value;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_uDscp_g1_dscpPhbPtr_f);
        break;

    case CTC_L3IF_PROP_DEFAULT_DSCP:
        if (value > CTC_MAX_QOS_DSCP_VALUE)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        cmd[0] = DRV_IOR(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd[0], &field[0]));
        if (field[0] != CTC_DSCP_SELECT_ASSIGN)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }

        field[0] = value;
        cmd[0] = DRV_IOW(DsDestInterface_t, DsDestInterface_uDscp_g0_dscp_f);
        break;

    case CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE:
    case CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS:
    case CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE:
    case CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS:
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_SUPPORT);

    default:
        CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
    }

    for (idx = 0; idx < MAX_L3IF_RELATED_PROPERTY; idx++)
    {
        if (0 != cmd[idx])
        {
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd[idx], &field[idx]));
        }
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

/**
@brief    Get  l3 interface's properties  according to interface id
*/
int32
sys_usw_l3if_get_property(uint8 lchip, uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32* value)
{
    uint32 cmd = 0;
    uint32 field = 0;

    CTC_PTR_VALID_CHECK(value);
    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);

    SYS_L3IF_LOCK;
    switch (l3if_prop)
    {
    /*src  interface */
    case CTC_L3IF_PROP_IPV4_UCAST:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v4UcastEn_f);
        break;

    case CTC_L3IF_PROP_IPV4_MCAST:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v4McastEn_f);
        break;

    case CTC_L3IF_PROP_IPV4_SA_TYPE:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v4UcastSaType_f);
        break;

    case CTC_L3IF_PROP_IPV6_UCAST:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v6UcastEn_f);
        break;

    case CTC_L3IF_PROP_IPV6_MCAST:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v6McastEn_f);
        break;

    case CTC_L3IF_PROP_IPV6_SA_TYPE:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_v6UcastSaType_f);
        break;

    case CTC_L3IF_PROP_VRF_ID:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_vrfId_f);
        break;

    case CTC_L3IF_PROP_ROUTE_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routeDisable_f);
        break;

    case CTC_L3IF_PROP_NAT_IFTYPE:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_l3IfType_f);
        break;

    case CTC_L3IF_PROP_ROUTE_ALL_PKT:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routeAllPackets_f);
        break;

    case CTC_L3IF_PROP_PBR_LABEL:
        if(DRV_IS_TSINGMA(lchip))
        {
            SYS_L3IF_UNLOCK;
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_pbrLabel_f);
        break;

    case CTC_L3IF_PROP_VRF_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routeLookupMode_f);
        break;

    /*dest  interface */
    case CTC_L3IF_PROP_MTU_EN:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_mtuCheckEn_f);
        break;

    case CTC_L3IF_PROP_MTU_EXCEPTION_EN:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_mtuExceptionEn_f);
        break;

    case CTC_L3IF_PROP_EGS_MCAST_TTL_THRESHOLD:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_mcastTtlThreshold_f);
        break;

    case CTC_L3IF_PROP_MTU_SIZE:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_mtuSize_f);
        break;

    case CTC_L3IF_PROP_MPLS_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_mplsEn_f);
        break;

    case CTC_L3IF_PROP_MPLS_LABEL_SPACE:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_mplsLabelSpace_f);
        break;

    case CTC_L3IF_PROP_RPF_CHECK_TYPE:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_rpfType_f);
        break;

    case CTC_L3IF_PROP_RPF_PERMIT_DEFAULT:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_rpfPermitDefault_f);
        break;

    case CTC_L3IF_PROP_IGMP_SNOOPING_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_igmpSnoopEn_f);
        break;

    case CTC_L3IF_PROP_CONTEXT_LABEL_EXIST:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_contextLabelExist_f);
        break;

    case CTC_L3IF_PROP_CID:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_categoryIdValid_f);
        DRV_IOCTL(lchip, l3if_id, cmd, value);
        if(value)
        {
            cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_categoryId_f);
        }
        break;

    case CTC_L3IF_PROP_PUB_ROUTE_LKUP_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_ipPublicLookupEn_f);
        break;

    case CTC_L3IF_PROP_STATS:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_l3IfStatsPtr_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_EGS_STATS:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_statsPtr_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_IPMC_USE_VLAN_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_ipmcUseVlan_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_INTERFACE_LABEL_EN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_interfaceLabelValid_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_PHB_USE_TUNNEL_HDR:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_phbUseOuterInfo_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_TRUST_DSCP:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_trustDscp_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_DSCP_SELECT_MODE:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN:
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_dscpPhbPtr_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &field));
        if (field != CTC_DSCP_SELECT_MAP)
        {
            *value = 0;
            CTC_RETURN_L3IF_UNLOCK(CTC_E_NONE);
        }
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_uDscp_g1_dscpPhbPtr_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_DEFAULT_DSCP:
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_dscpRemarkMode_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &field));
        if (field != CTC_DSCP_SELECT_ASSIGN)
        {
            *value = 0;
            CTC_RETURN_L3IF_UNLOCK(CTC_E_NONE);
        }
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_uDscp_g0_dscp_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        cmd = 0;
        break;

    case CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE:
    case CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS:
    case CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE:
    case CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS:
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_SUPPORT);

    default:
        CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
    }

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id :%d l3if_prop:%d \n", l3if_id, l3if_prop);

    if (0 != cmd)
    {
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, value));
        if (CTC_L3IF_PROP_ROUTE_EN == l3if_prop)
        {
            *value = !(*value);
        }
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_set_acl_property(uint8 lchip, uint16 l3if_id, ctc_acl_property_t* acl_prop)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 profile_id = 0;
    uint8  step = 0;
    uint8  lkup_type = 0;
    uint8  gport_type = 0;
    uint32 value = 0;
    sys_l3if_profile_t  profile_new;
    sys_l3if_profile_t  profile_old;
    sys_l3if_profile_t* profile_get = NULL;
    sys_l3if_profile_t* p_profile_old = NULL;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);
    CTC_PTR_VALID_CHECK(acl_prop);

    SYS_ACL_PROPERTY_CHECK(acl_prop);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id :%d, acl_prop:\nacl priority: %d, acl en: %d, direction: %d, tcam lookup type: %d\n", l3if_id,
        acl_prop->acl_priority, acl_prop->acl_en, acl_prop->direction, acl_prop->direction);

    sal_memset(&profile_new, 0, sizeof(sys_l3if_profile_t));
    sal_memset(&profile_old, 0, sizeof(sys_l3if_profile_t));

    step = DsSrcInterfaceProfile_gAcl_1_aclLabel_f - DsSrcInterfaceProfile_gAcl_0_aclLabel_f;
    if(acl_prop->tcam_lkup_type == CTC_ACL_TCAM_LKUP_TYPE_VLAN)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP)
        + CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA)
        + CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT) > 1)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port bitmap Metedata and Logic port can not be configured at the same time \n");
        return CTC_E_NOT_SUPPORT;
    }
    lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, acl_prop->tcam_lkup_type);
    if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP) ||
        CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN))
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    else if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
        gport_type = DRV_FLOWPORTTYPE_BITMAP;
    }
    else if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        gport_type = DRV_FLOWPORTTYPE_METADATA;
    }
    else if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
        gport_type = DRV_FLOWPORTTYPE_LPORT;
    }
    else
    {
        gport_type = DRV_FLOWPORTTYPE_GPORT;
    }

    SYS_L3IF_LOCK;
    if(CTC_INGRESS == acl_prop->direction)
    {
        profile_new.dir = CTC_INGRESS;
        profile_old.dir = CTC_INGRESS;
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_profileId_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            cmd = DRV_IOR(DsSrcInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &(profile_old.profile.src)));
            sal_memcpy(&(profile_new.profile.src), &(profile_old.profile.src), sizeof(DsSrcInterfaceProfile_m));
            p_profile_old = &profile_old;
        }


        SetDsSrcInterfaceProfile(V, gAcl_0_aclLabel_f + step * acl_prop->acl_priority,
                                 &(profile_new.profile.src), acl_prop->acl_en?acl_prop->class_id:0);
        SetDsSrcInterfaceProfile(V, gAcl_0_aclLookupType_f + step * acl_prop->acl_priority,
                                 &(profile_new.profile.src), acl_prop->acl_en?lkup_type:0);
        SetDsSrcInterfaceProfile(V, gAcl_0_aclUseGlobalPortType_f + step * acl_prop->acl_priority,
                                 &(profile_new.profile.src), acl_prop->acl_en?gport_type:0);
        SetDsSrcInterfaceProfile(V, gAcl_0_aclUsePIVlan_f + step * acl_prop->acl_priority,
                                 &(profile_new.profile.src),
                                 acl_prop->acl_en?(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0):0);
        /* alloc new profile id*/
        CTC_ERROR_RETURN_L3IF_UNLOCK(ctc_spool_add(p_usw_l3if_master[lchip]->acl_spool, &profile_new, p_profile_old, &profile_get));
        cmd = DRV_IOW(DsSrcInterfaceProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, profile_get->profile_id, cmd, &(profile_get->profile.src)), ret, error0);

        value = profile_get->profile_id;
        cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_profileId_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, cmd, &value), ret, error0);
    }
    else if(CTC_EGRESS == acl_prop->direction)
    {
        profile_new.dir = CTC_EGRESS;
        profile_old.dir = CTC_EGRESS;
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_profileId_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            cmd = DRV_IOR(DsDestInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &(profile_old.profile.dst)));
            sal_memcpy(&(profile_new.profile.dst), &(profile_old.profile.dst), sizeof(DsDestInterfaceProfile_m));
            p_profile_old = &profile_old;
        }

        SetDsDestInterfaceProfile(V, gAcl_0_aclLabel_f + step * acl_prop->acl_priority,
                                  &(profile_new.profile.dst), acl_prop->acl_en?acl_prop->class_id:0);
        SetDsDestInterfaceProfile(V, gAcl_0_aclLookupType_f + step * acl_prop->acl_priority,
                                  &(profile_new.profile.dst), acl_prop->acl_en?lkup_type:0);
        SetDsDestInterfaceProfile(V, gAcl_0_aclUseGlobalPortType_f + step * acl_prop->acl_priority,
                                  &(profile_new.profile.dst), acl_prop->acl_en?gport_type:0);
        SetDsDestInterfaceProfile(V, gAcl_0_aclUsePIVlan_f + step * acl_prop->acl_priority,
                                  &(profile_new.profile.dst),
                                  acl_prop->acl_en?(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0):0);
        CTC_ERROR_RETURN_L3IF_UNLOCK(ctc_spool_add(p_usw_l3if_master[lchip]->acl_spool, &profile_new, p_profile_old, &profile_get));
        cmd = DRV_IOW(DsDestInterfaceProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, profile_get->profile_id, cmd, &(profile_get->profile.dst)), ret, error0);

        value = profile_get->profile_id;
        cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_profileId_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, l3if_id, cmd, &value), ret, error0);
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;

error0:
    if (profile_id)
    {
        ctc_spool_add(p_usw_l3if_master[lchip]->acl_spool, &profile_old, &profile_new, NULL);
    }
    else
    {
        ctc_spool_remove(p_usw_l3if_master[lchip]->acl_spool, &profile_new, NULL);
    }
    SYS_L3IF_UNLOCK;

    return ret;
}

int32
sys_usw_l3if_get_acl_property(uint8 lchip, uint16 l3if_id, ctc_acl_property_t* acl_prop)
{
    uint32 cmd = 0;
    uint32 profile_id = 0;
    uint8  step = 0;
    uint8  lkup_type = 0;
    uint8  gport_type = 0;
    uint8  use_mapped_vlan = 0;
    DsSrcInterfaceProfile_m src_profile;
    DsDestInterfaceProfile_m dst_profile;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);
    CTC_PTR_VALID_CHECK(acl_prop);
    CTC_MAX_VALUE_CHECK(acl_prop->direction, CTC_EGRESS);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&src_profile, 0, sizeof(DsSrcInterfaceProfile_m));
    sal_memset(&dst_profile, 0, sizeof(DsDestInterfaceProfile_m));

    SYS_L3IF_LOCK;
    step = DsSrcInterfaceProfile_gAcl_1_aclLabel_f - DsSrcInterfaceProfile_gAcl_0_aclLabel_f;
    if(CTC_INGRESS == acl_prop->direction)
    {
        if (acl_prop->acl_priority > MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_profileId_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            cmd = DRV_IOR(DsSrcInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &src_profile));

            acl_prop->class_id = GetDsSrcInterfaceProfile(V,
                                gAcl_0_aclLabel_f + step * acl_prop->acl_priority, &src_profile);
            lkup_type = GetDsSrcInterfaceProfile(V,
                                gAcl_0_aclLookupType_f + step * acl_prop->acl_priority, &src_profile);
            use_mapped_vlan = GetDsSrcInterfaceProfile(V,
                                gAcl_0_aclUsePIVlan_f + step * acl_prop->acl_priority, &src_profile);
            gport_type = GetDsSrcInterfaceProfile(V,
                                gAcl_0_aclUseGlobalPortType_f + step * acl_prop->acl_priority, &src_profile);
        }
    }
    else if(CTC_EGRESS == acl_prop->direction)
    {
        if (acl_prop->acl_priority > MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1)
        {
            CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
        }
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_profileId_f);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            cmd = DRV_IOR(DsDestInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, profile_id, cmd, &dst_profile));

            acl_prop->class_id = GetDsDestInterfaceProfile(V,
                                gAcl_0_aclLabel_f + step * acl_prop->acl_priority, &dst_profile);
            lkup_type = GetDsDestInterfaceProfile(V,
                                gAcl_0_aclLookupType_f + step * acl_prop->acl_priority, &dst_profile);
            use_mapped_vlan = GetDsDestInterfaceProfile(V,
                                gAcl_0_aclUsePIVlan_f + step * acl_prop->acl_priority, &dst_profile);
            gport_type = GetDsDestInterfaceProfile(V,
                                gAcl_0_aclUseGlobalPortType_f + step * acl_prop->acl_priority, &dst_profile);
        }
    }

    acl_prop->acl_en = lkup_type ? 1: 0;
    acl_prop->tcam_lkup_type = sys_usw_unmap_acl_tcam_lkup_type(lchip, lkup_type);
    if(use_mapped_vlan)
    {
        CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }

    switch(gport_type)
    {
        case DRV_FLOWPORTTYPE_BITMAP:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
            break;
        case DRV_FLOWPORTTYPE_LPORT:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
            break;
        case DRV_FLOWPORTTYPE_METADATA:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA);
            break;
        default:
            break;
    }

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id :%d, acl_prop:\nacl priority: %d, acl en: %d, direction: %d, tcam lookup type: %d\n", l3if_id,
        acl_prop->acl_priority, acl_prop->acl_en, acl_prop->direction, acl_prop->tcam_lkup_type);
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_set_exception3_en(uint8 lchip, uint16 l3if_id, uint8 index, bool enbale)
{
    uint32  cmd = 0;
    uint32  value = 0;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id = %d, index = %d, enbale[0]= %d\n", l3if_id, index, enbale);

    SYS_L3IF_LOCK;
    cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_exception3En_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &value));
    if (TRUE == enbale)
    {
        value |= (1 << index);

    }
    else
    {
        value &= (~(1 << index));
    }

    cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_exception3En_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &value));
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_get_exception3_en(uint8 lchip, uint16 l3if_id, uint8 index, bool* enbale)
{
    uint32  cmd = 0;
    uint32  value = 0;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);
    CTC_PTR_VALID_CHECK(enbale);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3if_id = %d, index = %d\n", l3if_id, index);

    SYS_L3IF_LOCK;
    cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_exception3En_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, l3if_id, cmd, &value));
    *enbale = (value & (1 << index)) ? TRUE : FALSE;
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

/**
@brief    Config  router mac

*/
int32
sys_usw_l3if_set_router_mac(uint8 lchip, mac_addr_t mac_addr)
{
    uint32  cmd = 0;
    uint32  mac[2] = {0};
    DsRouterMac_m    ingress_router_mac;
    DsEgressRouterMac_m    egress_router_mac;
    IpePreLookupCtl_m ipe_pre_lookup_ctl;
    IpePortMacCtl1_m ipe_port_mac_ctl1;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "system router mac: %s \n", sys_output_mac(mac_addr));

    SYS_L3IF_LOCK;
    cmd = DRV_IOR(DsRouterMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ingress_router_mac));

    SYS_USW_SET_HW_MAC(mac, mac_addr);
    if(DRV_IS_DUET2(lchip))
    {
        SetDsRouterMac(V, routerMac0En_f, &ingress_router_mac, 1);
    }
    else
    {
        SetDsRouterMac(V, routerMacDecode0_f, &ingress_router_mac, 1);
    }
    SetDsRouterMac(A, routerMac0_f, &ingress_router_mac, mac);
    cmd = DRV_IOW(DsRouterMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ingress_router_mac));

    SetDsEgressRouterMac(A, routerMac_f, &egress_router_mac, mac);
    cmd = DRV_IOW(DsEgressRouterMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &egress_router_mac));

    /*route by inner after overlay decap*/
    sal_memset(&ipe_pre_lookup_ctl, 0, sizeof(ipe_pre_lookup_ctl));
    cmd = DRV_IOR(IpePreLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_lookup_ctl));
    SetIpePreLookupCtl(V, systemRouteMacEn_f, &ipe_pre_lookup_ctl, 1);
    SetIpePreLookupCtl(A, systemRouteMac_f, &ipe_pre_lookup_ctl, mac);
    cmd = DRV_IOW(IpePreLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_lookup_ctl));
    sal_memset(&ipe_port_mac_ctl1, 0, sizeof(ipe_port_mac_ctl1));
    cmd = DRV_IOR(IpePortMacCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_port_mac_ctl1));
    SetIpePortMacCtl1(V, systemMacEn_f, &ipe_port_mac_ctl1, 1);
    SetIpePortMacCtl1(A, systemMac_f, &ipe_port_mac_ctl1, mac);
    cmd = DRV_IOW(IpePortMacCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipe_port_mac_ctl1));
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

/**
@brief    Get  router mac
*/
int32
sys_usw_l3if_get_router_mac(uint8 lchip, mac_addr_t mac_addr)
{
    uint32 cmd = 0;
    uint32 mac[2] = {0};
    SYS_L3IF_INIT_CHECK();
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(DsRouterMac_t, DsRouterMac_routerMac0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, mac));

    SYS_USW_SET_USER_MAC(mac_addr, mac);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Router mac:%.2x%.2x.%.2x%.2x.%.2x%.2x \n",
                      mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_rtmac_modify_db_profile(uint8 lchip, uint16 l3if_id, sys_l3if_rtmac_profile_t * p_db_profile, sys_l3if_rtmac_profile_t * p_tmp_profile)
{
    uint8 i = 0, j = 0;
    uint8 free_idx = 0xff;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (p_db_profile->new_node)
    {
        return CTC_E_NONE;
    }

    for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        if (!CTC_IS_BIT_SET(p_tmp_profile->valid_bitmap, i))
        {
            continue;
        }
        for (j = 0; j < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); j++)
        {
            if (!CTC_IS_BIT_SET(p_db_profile->valid_bitmap, j))
            {
                free_idx = j;
                continue;
            }
            if ((p_tmp_profile->mac_info[i].prefix_idx == p_db_profile->mac_info[j].prefix_idx)
                &&( 0 == sal_memcmp(p_tmp_profile->mac_info[i].mac, p_db_profile->mac_info[j].mac, sizeof(mac_addr_t))))
            {
                p_db_profile->mac_ref[j]++;
                break;
            }
        }
        if (j == MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY))/*not find*/
        {
            sal_memcpy(&p_db_profile->mac_info[free_idx], &p_tmp_profile->mac_info[i], sizeof(sys_l3if_router_mac_t));
            CTC_BIT_SET(p_db_profile->valid_bitmap, free_idx);
            p_db_profile->mac_ref[free_idx]++;
        }

    }

    return CTC_E_NONE;
}
/**
     @brief      Config l3 interface router mac
*/
int32
_sys_usw_l3if_set_interface_router_mac(uint8 lchip, uint16 l3if_id, ctc_l3if_router_mac_t router_mac)
{
    sys_l3if_rtmac_profile_t tmp_profile;
    sys_l3if_rtmac_profile_t *old_profile = NULL;
    sys_l3if_rtmac_profile_t *get_profile = NULL;
    sys_l3if_rtmac_profile_t *container_profile = NULL;
    uint32 cmd = 0;
    uint8 bitmap = 0;
    uint8 idx = 0;
    uint8 is_del = 0;
    uint32 field_val = 0;
    uint8 i = 0;
    int32 profile_cnt = 0;
    sys_l3if_prop_t*  p_l3if_prop = &p_usw_l3if_master[lchip]->l3if_prop[l3if_id];
    uint8 update_old = 0;
    sys_l3if_rtmac_profile_t temp_rtmac;
    sys_l3if_rtmac_profile_t* p_temp_rtmac = NULL;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);
    CTC_MAX_VALUE_CHECK(router_mac.num, 4);
    CTC_MAX_VALUE_CHECK(router_mac.mode, CTC_L3IF_DELETE_ROUTE_MAC);
    CTC_MAX_VALUE_CHECK(router_mac.dir, CTC_EGRESS);

    old_profile = p_l3if_prop->p_rtmac_profile;

    /*egress*/
    if ( router_mac.dir)
    {
        if(router_mac.mode == CTC_L3IF_DELETE_ROUTE_MAC)
        {
            if(!p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile)
            {
                return CTC_E_NOT_EXIST;
            }
            if(sal_memcmp(p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile->mac, router_mac.mac[0], sizeof(mac_addr_t)))
            {
                return CTC_E_NOT_EXIST;
            }
            CTC_ERROR_RETURN(_sys_usw_l3if_unbinding_egs_rtmac(lchip, l3if_id));
            cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_routerMacProfile_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3if_id, cmd, &field_val));
        }
        else if(router_mac.mode == CTC_L3IF_APPEND_ROUTE_MAC)
        {
            return CTC_E_INVALID_PARAM;
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_l3if_binding_egs_rtmac(lchip, l3if_id, router_mac.mac[0], 1));
        }
        return CTC_E_NONE;
    }
    if ((NULL == old_profile && CTC_L3IF_APPEND_ROUTE_MAC == router_mac.mode) || (0 != router_mac.num && CTC_L3IF_UPDATE_ROUTE_MAC == router_mac.mode))
    {
        CTC_ERROR_RETURN(_sys_usw_l3if_binding_egs_rtmac(lchip, l3if_id, router_mac.mac[0], 0));
    }

    sal_memset(&tmp_profile, 0 , sizeof(tmp_profile));
    if (old_profile && router_mac.mode)
    {
        sal_memcpy(&tmp_profile, old_profile, sizeof(sys_l3if_rtmac_profile_t));

        /*read l3if selbitmap*/
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routerMacSelBitmap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3if_id, cmd, &field_val));
        tmp_profile.valid_bitmap = field_val;
        for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
        {
            if(!CTC_IS_BIT_SET(p_l3if_prop->rtmac_bmp, i))
            {
                sal_memset(&tmp_profile.mac_info[i],  0 , sizeof(sys_l3if_router_mac_t));
            }
        }
    }

    if (router_mac.mode == CTC_L3IF_DELETE_ROUTE_MAC)
    {
        is_del = 1;
    }

    for (idx = 0; idx < router_mac.num; idx++)
    {
        CTC_ERROR_RETURN(_sys_usw_l3if_rtmac_build_profile( lchip, l3if_id,&tmp_profile, router_mac.mac[idx], is_del));
    }

    if (tmp_profile.valid_bitmap == 0)
    {
        if(NULL == old_profile)
        {
            return CTC_E_NONE;
        }
        profile_cnt = ctc_spool_get_refcnt(p_usw_l3if_master[lchip]->rtmac_spool, old_profile);
        old_profile->match_mode = 1;/*exatly*/
        CTC_ERROR_RETURN(ctc_spool_remove(p_usw_l3if_master[lchip]->rtmac_spool, old_profile, NULL));
        if(profile_cnt>1)
        {
            for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY) ; i++)
            {
                if (!CTC_IS_BIT_SET(p_l3if_prop->rtmac_bmp, i))
                {
                    continue;
                }
                if (--old_profile->mac_ref[i] == 0)
                {
                    CTC_BIT_UNSET(old_profile->valid_bitmap, i);
                    sal_memset(&old_profile->mac_info[i],  0 , sizeof(sys_l3if_router_mac_t));
                }
            }
        }
        p_l3if_prop->rtmac_bmp = 0;
        p_l3if_prop->p_rtmac_profile = NULL;
        CTC_ERROR_RETURN(_sys_usw_l3if_rtmac_write_asic_interface( lchip,  l3if_id, 1, 0));
    }
    else
    {
        tmp_profile.match_mode = 0;
        container_profile = ctc_spool_lookup(p_usw_l3if_master[lchip]->rtmac_spool,  &tmp_profile);
        profile_cnt = ctc_spool_get_refcnt(p_usw_l3if_master[lchip]->rtmac_spool, old_profile);
        if (profile_cnt == 1)
        {
            sal_memcpy(&temp_rtmac, old_profile, sizeof(sys_l3if_rtmac_profile_t));
        }
        if (container_profile && container_profile == old_profile)
        {
            get_profile = container_profile;
        }
        else
        {
            if (NULL == container_profile)
            {
                tmp_profile.match_mode = 2;
            }
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->rtmac_spool, &tmp_profile, old_profile, &get_profile));
        }

        if ( ((profile_cnt > 1) && old_profile) || ((get_profile == old_profile)&&(profile_cnt == 1)))
        {
            update_old = 1;
            p_temp_rtmac = old_profile;
        }
        else if (profile_cnt == 1)
        {
            update_old = 1;
            p_temp_rtmac = &temp_rtmac;
        }

        _sys_usw_l3if_rtmac_modify_db_profile( lchip,  l3if_id, get_profile, &tmp_profile);

        if(update_old && !p_temp_rtmac->new_node)
        {
            /*clear the interface from old profile*/
            for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
            {
                if (CTC_IS_BIT_SET(p_l3if_prop->rtmac_bmp, i))
                {
                    if (--p_temp_rtmac->mac_ref[i] == 0)
                    {
                        CTC_BIT_UNSET(p_temp_rtmac->valid_bitmap, i);
                        sal_memset(&p_temp_rtmac->mac_info[i],  0 , sizeof(sys_l3if_router_mac_t));
                    }
                }
            }
        }

        /*get hw bmp*/
        CTC_ERROR_RETURN(_sys_usw_l3if_rtmac_profile_mapping(&tmp_profile, get_profile, &tmp_profile.valid_bitmap, &bitmap));

        if (!DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN(_sys_tsingma_l3if_rtmac_write_asic_mac(lchip, l3if_id,  get_profile));
            if(update_old)
            {
                CTC_ERROR_RETURN(_sys_tsingma_l3if_rtmac_write_asic_mac(lchip, l3if_id, p_temp_rtmac));
            }
        }
        else
        {
            CTC_ERROR_RETURN(_sys_duet2_l3if_rtmac_write_asic_mac(lchip, l3if_id,  get_profile));
            if(update_old)
            {
                CTC_ERROR_RETURN(_sys_duet2_l3if_rtmac_write_asic_mac(lchip, l3if_id,  p_temp_rtmac));
            }
        }

        get_profile->match_mode = 1;/*exatly*/
        p_l3if_prop->p_rtmac_profile = get_profile;
        p_l3if_prop->rtmac_bmp = bitmap;

        _sys_usw_l3if_rtmac_dump_profile(lchip, &tmp_profile);
        _sys_usw_l3if_rtmac_dump_profile(lchip, get_profile);

        CTC_ERROR_RETURN(_sys_usw_l3if_rtmac_write_asic_interface( lchip,  l3if_id, bitmap, get_profile->profile_id));

        if (get_profile->new_node)
        {
            CTC_ERROR_RETURN(_sys_usw_l3if_rtmac_merge_profile( lchip,  l3if_id, get_profile));
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_l3if_set_interface_router_mac(uint8 lchip, uint16 l3if_id, ctc_l3if_router_mac_t router_mac)
{
    int32 ret = CTC_E_NONE;

    SYS_L3IF_LOCK;
    ret = _sys_usw_l3if_set_interface_router_mac( lchip,  l3if_id,  router_mac);
    SYS_L3IF_UNLOCK;

    return ret;
}

/**
     @brief       Get l3 interface router mac
*/
int32
sys_usw_l3if_get_interface_router_mac(uint8 lchip, uint16 l3if_id, ctc_l3if_router_mac_t *router_mac)
{
    uint8   rtmac_bmp;
    uint8   i = 0, j = 0, k = 0;
    uint8   prefix_idx = 0;
    uint32  query_index = 0;

    sys_l3if_rtmac_profile_t * p_rtmac_profile = NULL;
    sys_l3if_macinner_profile_t* p_egs_profile = NULL;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);

    sal_memset(router_mac->mac, 0, sizeof(mac_addr_t)*4);
    rtmac_bmp = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].rtmac_bmp;
    p_rtmac_profile = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_rtmac_profile;
    router_mac->num = 0;
    p_egs_profile = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].p_egs_rtmac_profile;

    if(router_mac->dir == CTC_INGRESS)
    {
        if (0 == rtmac_bmp || NULL == p_rtmac_profile)
        {
            return CTC_E_NONE;
        }

        for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
        {
            if (!(CTC_IS_BIT_SET(p_rtmac_profile->valid_bitmap, i) && CTC_IS_BIT_SET(rtmac_bmp, i)))
            {
                continue;
            }
            prefix_idx = p_rtmac_profile->mac_info[i].prefix_idx;
            if (prefix_idx)
            {
                for (j = 0; j < 6; j++)
                {
                    if (!CTC_IS_BIT_SET(p_rtmac_profile->mac_info[i].postfix_bmp, j))
                    {
                        continue;
                    }
                    if (query_index++ < router_mac->next_query_index)
                    {
                        continue;
                    }
                    sal_memcpy(router_mac->mac[k], p_usw_l3if_master[lchip]->mac_prefix[prefix_idx - 1], sizeof(uint8)*5);
                    router_mac->mac[k][5] = p_rtmac_profile->mac_info[i].mac[j];
                    router_mac->num++;
                    if (++k == 4)
                    {
                        router_mac->next_query_index = query_index;
                        return CTC_E_NONE;
                    }
                }
            }
            else
            {
                if (query_index++ < router_mac->next_query_index)
                {
                    continue;
                }
                sal_memcpy(router_mac->mac[k], p_rtmac_profile->mac_info[i].mac, sizeof(mac_addr_t));
                router_mac->num++;
                if (++k == 4)
                {
                    router_mac->next_query_index = query_index;
                    return CTC_E_NONE;
                }
            }

        }
    }
    else
    {
        if(p_egs_profile)
        {
            sal_memcpy(router_mac->mac[0], p_egs_profile->mac, sizeof(mac_addr_t));
            router_mac->num = 1;
        }
    }
    router_mac->next_query_index = 0xffffffff;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_traverse_rtmac_check_prefix(void *node, void* user_data)
{
    mac_addr_t* prefix_mac = (mac_addr_t*)(user_data);
    sys_l3if_rtmac_profile_t *node_profile = (sys_l3if_rtmac_profile_t *)(((ctc_spool_node_t*)node)->data);
    uint8 i = 0;
    uint8 lchip = node_profile->lchip;
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"prefix_mac:%.2x%.2x.%.2x%.2x.%.2x \n",
                      (*prefix_mac)[0], (*prefix_mac)[1], (*prefix_mac)[2], (*prefix_mac)[3], (*prefix_mac)[4]);

    for (i = 0; i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        if (!CTC_IS_BIT_SET(node_profile->valid_bitmap, i))
        {
            continue;
        }

        if (node_profile->mac_info[i].prefix_idx)
        {
           if(0 == sal_memcmp(p_usw_l3if_master[lchip]->mac_prefix + node_profile->mac_info[i].prefix_idx-1, *prefix_mac, 5))
           {
                return CTC_E_IN_USE;
           }
        }
        else
        {
           if(0 == sal_memcmp(node_profile->mac_info[i].mac, *prefix_mac, 5))
           {
                return CTC_E_IN_USE;
           }
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3if_check_vmac_prefix_inuse(uint8 lchip, mac_addr_t prefix_mac)
{

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"mac_prefix:%.2x%.2x.%.2x%.2x.%.2x \n",
                      prefix_mac[0], prefix_mac[1], prefix_mac[2], prefix_mac[3], prefix_mac[4]);

    CTC_ERROR_RETURN(ctc_spool_traverse(p_usw_l3if_master[lchip]->rtmac_spool, _sys_usw_l3if_traverse_rtmac_check_prefix , prefix_mac));

    return 0;
}
/**
@brief    Config  40bits  virtual router mac prefix
*/

int32
sys_usw_l3if_set_vmac_prefix(uint8 lchip,  uint32 prefix_idx, mac_addr_t mac_40bit)
{
    uint32  cmd = 0;
    uint32  mac[2] = {0};
    uint8 index = 0;
    mac_addr_t old_prefix = {0};
    mac_addr_t mac_zero = {0};
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"prefix_idx :%d mac_prefix:%.2x%.2x.%.2x%.2x.%.2x \n", prefix_idx,
                      mac_40bit[0], mac_40bit[1], mac_40bit[2], mac_40bit[3], mac_40bit[4]);
    if(DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    for(index = 0; index < SYS_ROUTER_MAC_PREFIX_NUM; index++)
    {
        if (!sal_memcmp(mac_40bit, p_usw_l3if_master[lchip]->mac_prefix + index, 5) &&
            sal_memcmp(mac_40bit, mac_zero, sizeof(mac_addr_t)))
        {
            return CTC_E_EXIST;
        }
    }

    CTC_MAX_VALUE_CHECK(prefix_idx, 5);

    /*check old prefix is not be matched*/
    sys_usw_l3if_get_vmac_prefix(lchip, prefix_idx, old_prefix);
    if(_sys_usw_l3if_check_vmac_prefix_inuse(lchip, old_prefix))
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The old prefix (index [%d]) is in-use,remove those routermacs first\n",prefix_idx);
        return CTC_E_IN_USE;
    }

    if(_sys_usw_l3if_check_vmac_prefix_inuse(lchip, mac_40bit))
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Some routermacs are consistent with  new prefix,remove those routermacs first\n");
        return CTC_E_NOT_READY;
    }

    mac[1] = mac_40bit[0];
    mac[0] = (mac_40bit[1] << 24) | (mac_40bit[2] << 16) | (mac_40bit[3] << 8) | (mac_40bit[4]);

    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    switch (prefix_idx)
    {
        case 0:
            SetIpeIntfMapperCtl(A, vMac_0_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;
        case 1:
            SetIpeIntfMapperCtl(A, vMac_1_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;
        case 2:
            SetIpeIntfMapperCtl(A, vMac_2_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;
        case 3:
            SetIpeIntfMapperCtl(A, vMac_3_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;
        case 4:
            SetIpeIntfMapperCtl(A, vMac_4_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;
        case 5:
            SetIpeIntfMapperCtl(A, vMac_5_prefix_f, &ipe_intf_mapper_ctl, mac);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    sal_memcpy(p_usw_l3if_master[lchip]->mac_prefix + prefix_idx, mac_40bit,
               5);

    return CTC_E_NONE;
}

/**
@brief    Get  40bits   virtual router mac prefix
*/
int32
sys_usw_l3if_get_vmac_prefix(uint8 lchip,  uint32 prefix_idx, mac_addr_t mac_40bit)
{
    SYS_L3IF_INIT_CHECK();
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    CTC_MAX_VALUE_CHECK(prefix_idx, 5);

    sal_memcpy(mac_40bit, p_usw_l3if_master[lchip]->mac_prefix + prefix_idx,
               sizeof(mac_addr_t));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"prefix_idx :%d mac_prefix:%.2x%.2x.%.2x%.2x.%.2x \n", prefix_idx,
                      mac_40bit[0], mac_40bit[1], mac_40bit[2], mac_40bit[3], mac_40bit[4]);

    return CTC_E_NONE;
}

int32
sys_usw_l3if_get_default_entry_mode(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_L3IF_INIT_CHECK();

    return p_usw_l3if_master[lchip]->default_entry_mode;
}

int32
sys_usw_l3if_set_vrf_stats_en(uint8 lchip, ctc_l3if_vrf_stats_t* p_vrf_stats)
{
    DsVrf_m dsvrf;
    uint32 stats_ptr = 0;
    uint32 cmd = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L3IF_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vrf_stats);
    SYS_VRFID_CHECK(p_vrf_stats->vrf_id);

    sal_memset(&dsvrf, 0, sizeof(dsvrf));

    if (p_vrf_stats->enable)
    {
        if (p_vrf_stats->type == CTC_L3IF_VRF_STATS_UCAST)
        {
            SetDsVrf(V, statsType_f, &dsvrf, 1);
        }
        else if (p_vrf_stats->type == CTC_L3IF_VRF_STATS_ALL)
        {
            SetDsVrf(V, statsType_f, &dsvrf, 0);
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_vrf_stats->stats_id, &stats_ptr));
        SetDsVrf(V, statsPtr_f, &dsvrf, stats_ptr);
    }

    SYS_L3IF_LOCK;
    cmd = DRV_IOW(DsVrf_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, p_vrf_stats->vrf_id, cmd, &dsvrf));
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_get_vrf_statsid(uint8 lchip, uint16 vrfid, uint32* p_statsid)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_L3IF_INIT_CHECK();
    SYS_VRFID_CHECK(vrfid);
    CTC_PTR_VALID_CHECK(p_statsid);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L3IF_LOCK;
    cmd = DRV_IOR(DsVrf_t, DsVrf_statsPtr_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, vrfid, cmd, &value));

    CTC_ERROR_RETURN_L3IF_UNLOCK(sys_usw_flow_stats_lookup_statsid(lchip, CTC_STATS_STATSID_TYPE_VRF, value, p_statsid, CTC_INGRESS));
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "stats_id:%u, stats_ptr:0x%04X\n", *p_statsid, value);
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_create_ecmp_if(uint8 lchip, ctc_l3if_ecmp_if_t* p_ecmp_if)
{
    sys_l3if_ecmp_if_t* p_group = NULL;
    uint32 hw_group_id = 0;
    uint16 max_group_num = 0;
    uint32 ecmp_mem_base = 0;
    uint16 max_ecmp = 0;
    uint16 cur_emcp_num = 0;
    int32 ret = CTC_E_NONE;

    SYS_L3IF_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_ecmp_if);
    CTC_VALUE_RANGE_CHECK(p_ecmp_if->ecmp_if_id, 1, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM) - 1);
    CTC_MAX_VALUE_CHECK(p_ecmp_if->ecmp_type, CTC_NH_ECMP_TYPE_DLB);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create ecmp interface %d\n", p_ecmp_if->ecmp_if_id);

    SYS_L3IF_LOCK;
    /* check if ecmp group is exist */
    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, p_ecmp_if->ecmp_if_id);
    if (NULL != p_group)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ECMP interface already exist, if_id;%d \n", p_ecmp_if->ecmp_if_id);
        CTC_RETURN_L3IF_UNLOCK(CTC_E_EXIST);
    }
    SYS_L3IF_UNLOCK;

    max_ecmp = SYS_USW_MAX_DEFAULT_ECMP_NUM;
    CTC_ERROR_RETURN(sys_usw_nh_get_current_ecmp_group_num(lchip, &cur_emcp_num));

    /* check if ecmp group is valid */
    CTC_ERROR_RETURN(sys_usw_nh_get_max_ecmp_group_num(lchip, &max_group_num));
    if (cur_emcp_num  >= max_group_num)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Ecmp Group have no resource\n");
        return CTC_E_NO_RESOURCE;
    }

    /* alloc hw ecmp group id */
    CTC_ERROR_RETURN(sys_usw_nh_alloc_ecmp_offset(lchip, 1, 1, &hw_group_id));
    CTC_ERROR_GOTO(sys_usw_nh_alloc_ecmp_offset(lchip, 0, max_ecmp, &ecmp_mem_base),ret, error0);

    p_group = mem_malloc(MEM_L3IF_MODULE, sizeof(sys_l3if_ecmp_if_t));
    if (NULL == p_group)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }
    sal_memset(p_group, 0, sizeof(sys_l3if_ecmp_if_t));
    p_group->hw_group_id = hw_group_id;
    p_group->ecmp_member_base = ecmp_mem_base;
    p_group->ecmp_group_type = p_ecmp_if->ecmp_type;
    p_group->failover_en = p_ecmp_if->failover_en;
    p_group->stats_id = p_ecmp_if->stats_id;

    SYS_L3IF_LOCK;
    /* add to db */
    if (FALSE == ctc_vector_add(p_usw_l3if_master[lchip]->ecmp_group_vec, p_ecmp_if->ecmp_if_id, (void*)p_group))
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }
    if (p_group->stats_id)
    {
        CTC_ERROR_GOTO(sys_usw_flow_stats_add_ecmp_stats(lchip, p_group->stats_id, p_group->hw_group_id), ret, error3);
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
error3:
    ctc_vector_del(p_usw_l3if_master[lchip]->ecmp_group_vec, p_ecmp_if->ecmp_if_id);
    SYS_L3IF_UNLOCK;

error2:
    mem_free(p_group);

error1:
    sys_usw_nh_free_ecmp_offset(lchip, 0, max_ecmp, ecmp_mem_base);
error0:
    sys_usw_nh_free_ecmp_offset(lchip, 1, 1, hw_group_id);

    return ret;
}

int32
sys_usw_l3if_destory_ecmp_if(uint8 lchip, uint16 ecmp_if_id)
{
    sys_l3if_ecmp_if_t* p_group = NULL;
    uint32 hw_group_id = 0;
    uint8 index = 0;
    uint32 ecmp_mem_base = 0;
    uint16 max_ecmp = 0;
    sys_l3if_ecmp_if_t group;

    SYS_L3IF_INIT_CHECK();
    CTC_VALUE_RANGE_CHECK(ecmp_if_id, 1, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM) - 1);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Destory ecmp interface %d\n", ecmp_if_id);

    sal_memset(&group, 0, sizeof(sys_l3if_ecmp_if_t));
    SYS_L3IF_LOCK;
    /* check if ecmp group is exist */
    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    if (NULL == p_group)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_EXIST);
    }
    sal_memcpy(&group, p_group, sizeof(sys_l3if_ecmp_if_t));
    mem_free(p_group);
    SYS_L3IF_UNLOCK;

    /* free dsfwdptr */
    for (index = 0; index < group.intf_count; index ++)
    {
        sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, group.dsfwd_offset[index]);
    }

    /* reset hw ecmp group */
    group.intf_count = 0;
    sys_usw_nh_update_ecmp_interface_group(lchip, &group);
    if (group.stats_id)
    {
        sys_usw_flow_stats_remove_ecmp_stats(lchip, group.stats_id, group.hw_group_id);
    }

    max_ecmp = SYS_USW_MAX_DEFAULT_ECMP_NUM;
    /* free hw ecmp group id */
    hw_group_id = group.hw_group_id;
    ecmp_mem_base = group.ecmp_member_base;
    sys_usw_nh_free_ecmp_offset(lchip, 1, 1, hw_group_id);
    sys_usw_nh_free_ecmp_offset(lchip, 0, max_ecmp, ecmp_mem_base);

    /* remove db and free memory */
    SYS_L3IF_LOCK;
    ctc_vector_del(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_add_ecmp_if_member(uint8 lchip, uint16 ecmp_if_id, uint16 l3if_id)
{
    sys_l3if_ecmp_if_t* p_group = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    uint16 max_ecmp = 0;
    uint8 index = 0;
    int32 ret = CTC_E_NONE;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 dest_gport = 0;
    uint32 dsfwd_offset = 0;
    sys_l3if_ecmp_if_t group;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    SYS_L3IF_CREATE_CHECK(l3if_id);
    CTC_VALUE_RANGE_CHECK(ecmp_if_id, 1, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM) - 1);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add ecmp interface %d member %d\n", ecmp_if_id, l3if_id);

    sal_memset(&dsfwd_param, 0, sizeof(dsfwd_param));
    sal_memset(&group, 0, sizeof(sys_l3if_ecmp_if_t));

    /* alloc dsfwdptr*/
    CTC_ERROR_RETURN(sys_usw_nh_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                                    &dsfwd_offset));

    SYS_L3IF_LOCK;
    if (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != CTC_L3IF_TYPE_PHY_IF)
    {
        SYS_L3IF_UNLOCK;
        ret = CTC_E_INVALID_PARAM;
        goto error0;
    }

    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    if (NULL == p_group)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        SYS_L3IF_UNLOCK;
        ret = CTC_E_NOT_EXIST;
        goto error0;
    }

    max_ecmp = SYS_USW_MAX_DEFAULT_ECMP_NUM;
    if (p_group->intf_count >= max_ecmp)
    {
        SYS_L3IF_UNLOCK;
        ret = CTC_E_INVALID_PARAM;
        goto error0;
    }

    /* check if the member is exist */
    for (index = 0; index < p_group->intf_count; index ++)
    {
        if (p_group->intf_array[index] == l3if_id)
        {
            break;
        }
    }

    if (index != p_group->intf_count)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Member already exist \n");
        SYS_L3IF_UNLOCK;
        ret = CTC_E_EXIST;
        goto error0;
    }

    /* assign index */
    index = p_group->intf_count;

    p_group->dsfwd_offset[index] = dsfwd_offset;
    dest_gport = p_usw_l3if_master[lchip]->l3if_prop[l3if_id].gport;

    /* add to db */
    p_group->intf_array[index] = l3if_id;
    p_group->intf_count ++;
    sal_memcpy(&group, p_group, sizeof(sys_l3if_ecmp_if_t));
    SYS_L3IF_UNLOCK;

    /* write dsfwd */
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(dest_gport);
    dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dest_gport);
    dsfwd_param.dsfwd_offset = dsfwd_offset;

    ret = sys_usw_nh_add_dsfwd(lchip, &dsfwd_param);
    if (ret < 0)
    {
        goto error1;
    }

    /*set port mapping interface*/
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dest_gport);
    value = l3if_id;
    cmd = DRV_IOR(DsDestPort_t, DsDestPort_portMappedInterfaceId_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &value),ret, error1);

    /* update ecmp group table */
    ret = sys_usw_nh_update_ecmp_interface_group(lchip, &group);
    if (ret < 0)
    {
        goto error1;
    }

    return ret;

error1:
    SYS_L3IF_LOCK;
    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    if (NULL != p_group)
    {
        for (index = 0; index < p_group->intf_count; index ++)
        {
            if (p_group->intf_array[index] == l3if_id)
            {
                break;
            }
        }
        p_group->intf_array[index] = 0;
        p_group->intf_count --;

        p_group->dsfwd_offset[index] = 0;
    }
    SYS_L3IF_UNLOCK;

error0:
    sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, dsfwd_offset);

    return ret;
}

int32
sys_usw_l3if_remove_ecmp_if_member(uint8 lchip, uint16 ecmp_if_id, uint16 l3if_id)
{
    sys_l3if_ecmp_if_t* p_group = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    uint8 index;
    uint32 dsfwd_offset = 0;
    int32 ret = CTC_E_NONE;

    SYS_L3IF_INIT_CHECK();
    SYS_L3IFID_VAILD_CHECK(l3if_id);
    CTC_VALUE_RANGE_CHECK(ecmp_if_id, 1, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM) - 1);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove ecmp interface %d member %d\n", ecmp_if_id, l3if_id);

    sal_memset(&dsfwd_param, 0, sizeof(dsfwd_param));

    SYS_L3IF_LOCK;
    if (p_usw_l3if_master[lchip]->l3if_prop[l3if_id].l3if_type != CTC_L3IF_TYPE_PHY_IF)
    {
        CTC_RETURN_L3IF_UNLOCK(CTC_E_INVALID_PARAM);
    }

    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    if (NULL == p_group)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_EXIST);
    }

    /* find the index of the member removed */
    for (index = 0; index < p_group->intf_count; index ++)
    {
        if (p_group->intf_array[index] == l3if_id)
        {
            break;
        }
    }
    if (index == p_group->intf_count)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Member not exist \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_EXIST);
    }

    dsfwd_offset = p_group->dsfwd_offset[index];

    /* remove db */
    p_group->intf_array[index] = p_group->intf_array[p_group->intf_count-1];
    p_group->dsfwd_offset[index] = p_group->dsfwd_offset[p_group->intf_count-1];
    p_group->intf_count --;

    /* update ecmp group table */
    sys_usw_nh_update_ecmp_interface_group(lchip, p_group);
    SYS_L3IF_UNLOCK;

    /* free dsfwdptr*/
    sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, dsfwd_offset);

    return ret;
}

int32
sys_usw_l3if_get_ecmp_if_info(uint8 lchip, uint16 ecmp_if_id, sys_l3if_ecmp_if_t* p_ecmp_if)
{
    sys_l3if_ecmp_if_t* p_group = NULL;

    SYS_L3IF_INIT_CHECK();
    CTC_VALUE_RANGE_CHECK(ecmp_if_id, 1, MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM) - 1);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L3IF_LOCK;
    p_group = ctc_vector_get(p_usw_l3if_master[lchip]->ecmp_group_vec, ecmp_if_id);
    if (NULL == p_group)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        CTC_RETURN_L3IF_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (p_ecmp_if)
    {
        sal_memcpy(p_ecmp_if, p_group, sizeof(sys_l3if_ecmp_if_t));
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_show_ecmp_if_info(uint8 lchip, uint16 ecmp_if_id)
{
    sys_l3if_ecmp_if_t ecmp_if = {0};
    uint8 index = 0;

    LCHIP_CHECK(lchip);
    SYS_L3IF_INIT_CHECK();

    CTC_ERROR_RETURN(sys_usw_l3if_get_ecmp_if_info(lchip, ecmp_if_id, &ecmp_if));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------L3if Ecmp Info--------------------\r\n");

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "Ecmp If Id", ecmp_if_id);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "Member Count", ecmp_if.intf_count);
    if(ecmp_if.failover_en)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10s \n", "Ecmp Group Type", "Failover");
    }
    else
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10s \n", "Ecmp Group Type", (ecmp_if.ecmp_group_type==CTC_NH_ECMP_TYPE_DLB)?"Dynamic":"Static");
    }
    if (0 != ecmp_if.stats_id)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "Stats Id", ecmp_if.stats_id);
    }
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "HW Group Id", ecmp_if.hw_group_id);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "Ecmp Member Base", ecmp_if.ecmp_member_base);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------\r\n");

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------L3if Ecmp Member Info----------------\r\n");

    for (index = 0; index < ecmp_if.intf_count; index++)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s:%10u \n", "L3if Id", ecmp_if.intf_array[index]);
    }

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------\r\n");

    return CTC_E_NONE;
}

/**
restore inner router mac of warmboot by l3if
*/
uint32
sys_usw_l3if_binding_inner_router_mac(uint8 lchip, mac_addr_t* p_mac_addr, uint8* route_mac_index)
{
    int32 ret = CTC_E_NONE;
    uint32  cmd = 0;
    uint32  mac[2] = {0};
    uint32  ref_cnt = 0;
    DsRouterMacInner_m  inner_router_mac;
    sys_l3if_macinner_profile_t profile_new;
    sys_l3if_macinner_profile_t* profile_get = NULL;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_L3IF_INIT_CHECK();

    sal_memset(&profile_new, 0, sizeof(sys_l3if_macinner_profile_t));
    sal_memcpy(profile_new.mac, p_mac_addr, sizeof(mac_addr_t));

    SYS_L3IF_LOCK;
    CTC_ERROR_RETURN_L3IF_UNLOCK(ctc_spool_add(p_usw_l3if_master[lchip]->macinner_spool, &profile_new, NULL, &profile_get));

    ref_cnt = ctc_spool_get_refcnt(p_usw_l3if_master[lchip]->macinner_spool, profile_get);
    if (1 == ref_cnt)
    {
        SYS_USW_SET_HW_MAC(mac, *p_mac_addr);
        SetDsRouterMacInner(A, routerMac_f, &inner_router_mac, mac);
        cmd = DRV_IOW(DsRouterMacInner_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, profile_get->profile_id, cmd, &inner_router_mac);
        if (ret < 0)
        {
            ctc_spool_remove(p_usw_l3if_master[lchip]->macinner_spool, profile_get, NULL);
        }
    }

    *route_mac_index = profile_get->profile_id;
	SYS_L3IF_UNLOCK;

    return ret;
}


int32
sys_usw_l3if_unbinding_inner_router_mac(uint8 lchip, uint8 route_mac_index)
{
    uint32  cmd = 0;
    uint32  ref_cnt = 0;
    uint32  mac[2] = {0};
    sys_l3if_macinner_profile_t profile_del;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    SYS_L3IF_INIT_CHECK();

    sal_memset(&profile_del, 0, sizeof(sys_l3if_macinner_profile_t));

    SYS_L3IF_LOCK;
    cmd = DRV_IOR(DsRouterMacInner_t, DsRouterMacInner_routerMac_f);
    CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, route_mac_index, cmd, mac));

    SYS_USW_SET_USER_MAC(profile_del.mac, mac);

    ctc_spool_remove(p_usw_l3if_master[lchip]->macinner_spool, &profile_del, NULL);
    ref_cnt = ctc_spool_get_refcnt(p_usw_l3if_master[lchip]->macinner_spool, &profile_del);
    if (0 == ref_cnt)
    {
        cmd = DRV_IOW(DsRouterMacInner_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_L3IF_UNLOCK(DRV_IOCTL(lchip, route_mac_index, cmd, mac));
    }
    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_l3if_traverse_acl_profile(void *node, void* user_data)
{
    uint32 *acl_cnt = (uint32 *)user_data;
    sys_l3if_profile_t *acl_profile = (sys_l3if_profile_t *)(((ctc_spool_node_t*)node)->data);

    acl_cnt[acl_profile->dir]++;

    return CTC_E_NONE;
}

int32
sys_usw_l3if_state_show(uint8 lchip)
{
    uint16 idx1 = 0;
    uint16 idx2 = 0;
    uint16 l3if_cnt[MAX_L3IF_TYPE_NUM] = {0};
    uint16 profile_cnt = 0;
    uint32 acl_cnt[CTC_BOTH_DIRECTION] = {0};
    mac_addr_t system_mac = {0};
    sys_usw_opf_t opf;

    LCHIP_CHECK(lchip);
    SYS_L3IF_INIT_CHECK();
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    SYS_L3IF_LOCK;
    for(idx1 = 0; idx1 <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); idx1++)
    {
        if (TRUE == p_usw_l3if_master[lchip]->l3if_prop[idx1].vaild)
        {
            idx2 = p_usw_l3if_master[lchip]->l3if_prop[idx1].l3if_type;
            l3if_cnt[idx2]++;
        }
    }

    ctc_spool_traverse(p_usw_l3if_master[lchip]->acl_spool, (spool_traversal_fn)_sys_usw_l3if_traverse_acl_profile, acl_cnt);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------ L3if Status --------------------------\n");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","L3if ");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n","--Total count ", DRV_IS_DUET2(lchip) ? "1024(rsv:0, 1023)" : "4096(rsv:0, 4095)");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Phy-if count", l3if_cnt[CTC_L3IF_TYPE_PHY_IF]);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Vlan-if count", l3if_cnt[CTC_L3IF_TYPE_VLAN_IF]);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Sub-if count", l3if_cnt[CTC_L3IF_TYPE_SUB_IF]);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Service-if count", l3if_cnt[CTC_L3IF_TYPE_SERVICE_IF]);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s\n","Ecmp-L3if ");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %d(rsv:0)\n","--Total count ", MCHIP_CAP(SYS_CAP_L3IF_ECMP_GROUP_NUM));
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_usw_l3if_master[lchip]->ecmp_group_vec->used_cnt);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s\n","L3if-Profile ");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n","--Ingress Total count ", "128");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Ingress Used count", acl_cnt[CTC_INGRESS]);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n","--Egress Total count ", "128");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Egress Used count", acl_cnt[CTC_EGRESS]);

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------------ Mac Status ---------------------------\n");
    sys_usw_l3if_get_router_mac(lchip, system_mac);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s%s\n", "System Mac: ", sys_output_mac(system_mac));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_mac_profile;
    profile_cnt = sys_usw_opf_get_alloced_cnt(lchip, &opf);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Mac profile");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n", "--Total count ", DRV_IS_DUET2(lchip) ? "32(rsv:0)" : "64(rsv:0)");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", profile_cnt+1);

    opf.pool_index = 0;
    opf.pool_type  = p_usw_l3if_master[lchip]->opf_type_macinner_profile;
    profile_cnt = sys_usw_opf_get_alloced_cnt(lchip, &opf);
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","Inner Mac ");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n","--Total count ", "64");
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", profile_cnt+1);

    SYS_L3IF_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_l3if_wb_traverse_sync_ecmp_if(void *data, uint32 vec_index, void *user_data)
{
    uint8 loop = 0;
    uint8 loop_cnt = 0;
    uint16 ecmp_cnt = 0;
    uint16 real_mem_cnt = 0;
    sys_l3if_ecmp_if_t *p_ecmp_if = (sys_l3if_ecmp_if_t*)data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t*)user_data;
    sys_wb_l3if_ecmp_if_t    *p_wb_l3if_ecmp_if;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    loop_cnt = (p_ecmp_if->intf_count%64) ? (p_ecmp_if->intf_count/64 + 1) : p_ecmp_if->intf_count/64;

    for (loop = 0; ((loop < loop_cnt) || (loop_cnt == 0)); loop++)
    {
        p_wb_l3if_ecmp_if = (sys_wb_l3if_ecmp_if_t *)wb_data->buffer + wb_data->valid_cnt + loop;

        sal_memset(p_wb_l3if_ecmp_if->rsv, 0, sizeof(uint8)*3);
        p_wb_l3if_ecmp_if->ecmp_if_id = vec_index;
        p_wb_l3if_ecmp_if->index = loop;
        p_wb_l3if_ecmp_if->hw_group_id = p_ecmp_if->hw_group_id;
        p_wb_l3if_ecmp_if->intf_count = p_ecmp_if->intf_count;
        p_wb_l3if_ecmp_if->failover_en = p_ecmp_if->failover_en;

        real_mem_cnt = ((p_ecmp_if->intf_count - ecmp_cnt) > 64) ? 64 : (p_ecmp_if->intf_count - ecmp_cnt);
        sal_memcpy(p_wb_l3if_ecmp_if->intf_array, &p_ecmp_if->intf_array[64*loop], real_mem_cnt * sizeof(uint16));
        sal_memcpy(p_wb_l3if_ecmp_if->dsfwd_offset, &p_ecmp_if->dsfwd_offset[64*loop], real_mem_cnt * sizeof(uint32));
        p_wb_l3if_ecmp_if->ecmp_group_type = p_ecmp_if->ecmp_group_type;     /* refer to ctc_nh_ecmp_type_t */
        p_wb_l3if_ecmp_if->ecmp_member_base = p_ecmp_if->ecmp_member_base;
        p_wb_l3if_ecmp_if->stats_id = p_ecmp_if->stats_id;

        if (++wb_data->valid_cnt == max_buffer_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
            wb_data->valid_cnt = 0;
        }
        ecmp_cnt += 64;

        if (!loop_cnt)
        {
            return CTC_E_NONE;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_l3if_wb_traverse_sync_inner_mac(void *node, void* user_data)
{
    ctc_wb_data_t *wb_data = (ctc_wb_data_t*)user_data;
    sys_wb_l3if_router_mac_t *l3if_mac;
    sys_l3if_macinner_profile_t* profile_get = (sys_l3if_macinner_profile_t*)(((ctc_spool_node_t*)node)->data);
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    l3if_mac = (sys_wb_l3if_router_mac_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(l3if_mac, 0, sizeof(sys_wb_l3if_router_mac_t));

    l3if_mac->profile_id = profile_get->profile_id;
    if(1 == profile_get->type)
    {
        return CTC_E_NONE;
    }
    l3if_mac->is_inner = 1;
    l3if_mac->ref = ((ctc_spool_node_t*)node)->ref_cnt;
    sal_memcpy(l3if_mac->mac, profile_get->mac, sizeof(mac_addr_t));

    if (++wb_data->valid_cnt == max_buffer_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}
int32
sys_usw_l3if_trmac_reset_postfix_bmp(uint8 lchip, sys_l3if_rtmac_profile_t* p_profile)
{
    uint16 i = 0, j = 0, p = 0;
    for(i = 0;  i < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); i++)
    {
        if (0 == p_profile->mac_info[i].prefix_idx)
        {
            continue;
        }
        /*invalid mac entry ,prefix_idx must be 0 */
        for (j = 0; j < sizeof(mac_addr_t); j++)
        {
            if(j == 0)
            {
                CTC_BIT_SET(p_profile->mac_info[i].postfix_bmp, j);
                continue;
            }
            for (p = 0; p < j; p++)
            {
                if(p_profile->mac_info[i].mac[p] == p_profile->mac_info[i].mac[j])
                {
                    CTC_BIT_UNSET(p_profile->mac_info[i].postfix_bmp, j);
                    break;
                }
            }
            if(p == j)
            {
                CTC_BIT_SET(p_profile->mac_info[i].postfix_bmp, j);
            }
        }
    }

    return CTC_E_NONE;
}
int32 sys_usw_l3if_wb_rtmac_restore(uint8 lchip)
{
    uint16 i = 0, mac_idx = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 profile_id = 0;
    uint32  mac[2] = {0};
    sys_l3if_prop_t* p_l3if_prop = NULL;
    sys_l3if_rtmac_profile_t tmp_profile;
    sys_l3if_rtmac_profile_t *p_old_profile = NULL;
    sys_l3if_rtmac_profile_t *p_get_profile = NULL;
    sys_l3if_macinner_profile_t egs_profile;
    sys_l3if_macinner_profile_t *p_egs_get_profile = NULL;

    for (i = 0; i <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); i++)
    {
        if(p_usw_l3if_master[lchip]->l3if_prop[i].vaild == FALSE)
        {
            continue;
        }
        /*restore l3if ingress router_mac */
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routerMacProfile_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &field_val));
        profile_id = field_val;
        p_l3if_prop = &p_usw_l3if_master[lchip]->l3if_prop[i];
        if(profile_id)
        {
            cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_routerMacSelBitmap_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &field_val));
            p_l3if_prop->rtmac_bmp = field_val;

            sal_memset(&tmp_profile, 0, sizeof(tmp_profile));
            tmp_profile.profile_id = profile_id;
            if (!DRV_IS_DUET2(lchip))
            {
                CTC_ERROR_RETURN(_sys_tsingma_l3if_rtmac_read_asic_mac(lchip, profile_id, &tmp_profile));
            }
            else
            {
                CTC_ERROR_RETURN(_sys_duet2_l3if_rtmac_read_asic_mac(lchip, profile_id, &tmp_profile));
            }
            _sys_usw_l3if_rtmac_dump_profile(lchip, &tmp_profile);
            tmp_profile.lchip = lchip;
            tmp_profile.match_mode = 1;
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->rtmac_spool, &tmp_profile, p_old_profile, &p_get_profile));
            p_l3if_prop->p_rtmac_profile = p_get_profile;
            for (mac_idx = 0; mac_idx < MCHIP_CAP(SYS_CAP_L3IF_ROUTER_MAC_NUM_PER_ENTRY); mac_idx++)
            {
                if (CTC_IS_BIT_SET(p_l3if_prop->rtmac_bmp, mac_idx))
                {
                    p_get_profile->mac_ref[mac_idx]++;
                }
            }

            if (p_get_profile->new_node)
            {
                sys_usw_l3if_trmac_reset_postfix_bmp(lchip, p_get_profile);
                p_get_profile->new_node = 0;
            }
        }

        /*restore l3if egress router_mac */
        sal_memset(&egs_profile, 0, sizeof(egs_profile));
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_routerMacProfile_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &field_val));
        egs_profile.profile_id = field_val;
        if(egs_profile.profile_id)
        {
            sal_memset(mac, 0, sizeof(mac));
            cmd = DRV_IOR(DsEgressRouterMac_t, DsEgressRouterMac_routerMac_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, egs_profile.profile_id, cmd, &mac));
            SYS_USW_SET_USER_MAC(egs_profile.mac, mac);
            egs_profile.type = 1;

            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->macinner_spool, &egs_profile, NULL, &p_egs_get_profile));

            p_l3if_prop->p_egs_rtmac_profile = p_egs_get_profile;
        }

    }

    return CTC_E_NONE;
}
int32 sys_usw_l3if_wb_sync(uint8 lchip)
{
    uint32 loop = 0;
    int32 ret = CTC_E_NONE;
    uint32 max_entry_cnt = 0;
    ctc_wb_data_t wb_data;
    sys_wb_l3if_master_t* p_wb_l3if_master = NULL;
    sys_wb_l3if_prop_t   wb_l3if_prop;
    sys_wb_l3if_router_mac_t l3if_mac;
    uint8 work_status = 0;
    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&l3if_mac, 0, sizeof(sys_wb_l3if_router_mac_t));
    sal_memset(&wb_l3if_prop, 0, sizeof(sys_wb_l3if_prop_t));



	sys_usw_ftm_get_working_status(lchip, &work_status);
    /*syncup  l3if master*/
    wb_data.buffer = mem_malloc(MEM_L3IF_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l3if_master_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_MASTER);

    p_wb_l3if_master = (sys_wb_l3if_master_t  *)wb_data.buffer;
    p_wb_l3if_master->lchip = lchip;
    p_wb_l3if_master->version = SYS_WB_VERSION_L3IF;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    /*syncup  l3if PROP*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l3if_prop_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_PROP);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);

    do
    {
        if (!p_usw_l3if_master[lchip]->l3if_prop[loop].vaild)
        {
            continue;
        }

        wb_l3if_prop.l3if_id = p_usw_l3if_master[lchip]->l3if_prop[loop].l3if_id;
        wb_l3if_prop.gport = p_usw_l3if_master[lchip]->l3if_prop[loop].gport;
        wb_l3if_prop.vlan_id = p_usw_l3if_master[lchip]->l3if_prop[loop].vlan_id;
        wb_l3if_prop.vaild = p_usw_l3if_master[lchip]->l3if_prop[loop].vaild;
        wb_l3if_prop.l3if_type = p_usw_l3if_master[lchip]->l3if_prop[loop].l3if_type;
        wb_l3if_prop.vlan_ptr = p_usw_l3if_master[lchip]->l3if_prop[loop].vlan_ptr;

        sal_memcpy( (uint8*)wb_data.buffer + wb_data.valid_cnt * sizeof(sys_wb_l3if_prop_t), (uint8*)&wb_l3if_prop, sizeof(sys_wb_l3if_prop_t));

        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }while ((++loop) <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1));

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*syncup l3if_inner_router_mac*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l3if_router_mac_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_ROUTER_MAC);

    ctc_spool_traverse(p_usw_l3if_master[lchip]->macinner_spool, (spool_traversal_fn)_sys_usw_l3if_wb_traverse_sync_inner_mac, &wb_data);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

	if(work_status != 3)
	{
		/*sync ecmp_if*/
        CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l3if_ecmp_if_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_ECMP_IF);

        ctc_vector_traverse2(p_usw_l3if_master[lchip]->ecmp_group_vec, 0, _sys_usw_l3if_wb_traverse_sync_ecmp_if, &wb_data);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
	}


done:
    if ( wb_data.buffer )
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32 sys_usw_l3if_wb_restore(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 loop = 0;
    uint32 profile_id = 0;
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    uint16 max_ecmp = 0;
    uint8 mac_index = 0;
    uint32 step = 0;
    sys_l3if_profile_t  profile_new;
    sys_l3if_macinner_profile_t  macinner_profile_new;
    sys_wb_l3if_master_t wb_l3if_master = {0};

    ctc_wb_query_t    wb_query;
    sys_wb_l3if_prop_t   wb_l3if_prop = {0};
    sys_wb_l3if_router_mac_t    wb_l3if_mac = {0};
    sys_wb_l3if_ecmp_if_t   wb_ecmp_if = {0};
    sys_l3if_ecmp_if_t   *p_ecmp_if = NULL;
    uint32 last_ecmp_if = 0;
    uint16 ecmp_cnt = 0;
    uint16 real_mem_cnt = 0;
    uint16 ecmp_mem_idx = 0;
    uint32 mac_prefix[2] = {0};
    mac_addr_t user_mac_prefix = {0};

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*restore l3if property*/
    wb_query.buffer = mem_malloc(MEM_L3IF_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_l3if_master_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query l3if master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8*)&wb_l3if_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_L3IF, wb_l3if_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_l3if_prop_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_PROP);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_l3if_prop, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].gport =  wb_l3if_prop.gport;
        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].vlan_id =   wb_l3if_prop.vlan_id;
        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].vaild =   wb_l3if_prop.vaild ;
        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].l3if_type =    wb_l3if_prop.l3if_type;
        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].vlan_ptr =    wb_l3if_prop.vlan_ptr ;
        p_usw_l3if_master[lchip]->l3if_prop[wb_l3if_prop.l3if_id].l3if_id = wb_l3if_prop.l3if_id;

        sal_memset(&profile_new, 0, sizeof(sys_l3if_profile_t));
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, wb_l3if_prop.l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            profile_new.dir = CTC_INGRESS;
            profile_new.profile_id = profile_id;
            cmd = DRV_IOR(DsSrcInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &(profile_new.profile.src)));

            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->acl_spool, &profile_new, NULL, NULL));
        }

        sal_memset(&profile_new, 0, sizeof(sys_l3if_profile_t));
        cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_profileId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, wb_l3if_prop.l3if_id, cmd, &profile_id));
        if(profile_id)
        {
            profile_new.dir = CTC_EGRESS;
            profile_new.profile_id = profile_id;
            cmd = DRV_IOR(DsDestInterfaceProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &(profile_new.profile.dst)));

            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->acl_spool, &profile_new, NULL, NULL));
        }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%d, restore l3if_router_mac\n", __LINE__);
    /*restore l3if_router_mac */
    CTC_ERROR_RETURN(sys_usw_l3if_wb_rtmac_restore(lchip));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%d, restore inner_route_mac\n", __LINE__);
    /*inner_route_mac*/
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_l3if_router_mac_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_ROUTER_MAC);
    sal_memset(&macinner_profile_new, 0, sizeof(sys_l3if_macinner_profile_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_l3if_mac, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        mac_index = wb_l3if_mac.profile_id;
        macinner_profile_new.profile_id = mac_index;
        macinner_profile_new.type = 0;
        sal_memcpy(macinner_profile_new.mac,  wb_l3if_mac.mac, sizeof(mac_addr_t));

        for (loop = 0; loop < wb_l3if_mac.ref; loop++)
        {
            CTC_ERROR_RETURN(ctc_spool_add(p_usw_l3if_master[lchip]->macinner_spool, &macinner_profile_new, NULL, NULL));
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%d, restore ecmp if\n", __LINE__);
    /*restore  ecmp if*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_l3if_ecmp_if_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_ECMP_IF);

    max_ecmp = SYS_USW_MAX_DEFAULT_ECMP_NUM;

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_ecmp_if, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        if (wb_ecmp_if.ecmp_if_id != last_ecmp_if)
        {
            p_ecmp_if = mem_malloc(MEM_L3IF_MODULE, sizeof(sys_l3if_ecmp_if_t));
            if (NULL == p_ecmp_if)
            {
                SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
            sal_memset(p_ecmp_if, 0, sizeof(sys_l3if_ecmp_if_t));

            p_ecmp_if->hw_group_id = wb_ecmp_if.hw_group_id;
            p_ecmp_if->intf_count = wb_ecmp_if.intf_count;
            p_ecmp_if->failover_en = wb_ecmp_if.failover_en;
            sal_memcpy(p_ecmp_if->intf_array, wb_ecmp_if.intf_array, 64 * sizeof(uint16));
            sal_memcpy(p_ecmp_if->dsfwd_offset, wb_ecmp_if.dsfwd_offset, 64 * sizeof(uint32));
            p_ecmp_if->ecmp_group_type = wb_ecmp_if.ecmp_group_type;     /* refer to ctc_nh_ecmp_type_t */
            p_ecmp_if->ecmp_member_base = wb_ecmp_if.ecmp_member_base;
            p_ecmp_if->stats_id = wb_ecmp_if.stats_id;
            CTC_ERROR_GOTO(sys_usw_nh_alloc_ecmp_offset_from_position(lchip, 1, 1, p_ecmp_if->hw_group_id), ret, done);
            CTC_ERROR_GOTO(sys_usw_nh_alloc_ecmp_offset_from_position(lchip, 0, max_ecmp, p_ecmp_if->ecmp_member_base), ret, done);
            last_ecmp_if = wb_ecmp_if.ecmp_if_id;
            ecmp_cnt = 64;

            for (ecmp_mem_idx = 0; ecmp_mem_idx < ((wb_ecmp_if.intf_count > 64) ? 64 : wb_ecmp_if.intf_count); ecmp_mem_idx++)
            {
                CTC_ERROR_GOTO(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, p_ecmp_if->dsfwd_offset[ecmp_mem_idx]), ret, done);
            }

            ret = ctc_vector_add(p_usw_l3if_master[lchip]->ecmp_group_vec,  wb_ecmp_if.ecmp_if_id, p_ecmp_if);
            if (ret == FALSE)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }
        else
        {
            real_mem_cnt = ((wb_ecmp_if.intf_count - ecmp_cnt) > 64) ? 64 : (wb_ecmp_if.intf_count - ecmp_cnt);

            sal_memcpy(&p_ecmp_if->intf_array[ecmp_cnt], wb_ecmp_if.intf_array, real_mem_cnt * sizeof(uint16));
            sal_memcpy(&p_ecmp_if->dsfwd_offset[ecmp_cnt], wb_ecmp_if.dsfwd_offset, real_mem_cnt * sizeof(uint32));

            for (ecmp_mem_idx = 0; ecmp_mem_idx < real_mem_cnt; ecmp_mem_idx++)
            {
                CTC_ERROR_GOTO(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, p_ecmp_if->dsfwd_offset[ecmp_mem_idx]), ret, done);
            }
            ecmp_cnt += 64;
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));
    /* resotre route mac prefix*/
    step = IpeIntfMapperCtl_vMac_1_prefix_f - IpeIntfMapperCtl_vMac_0_prefix_f;
    for(loop = 0; loop < 6; loop++)
    {
        cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_vMac_0_prefix_f + (loop * step));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_prefix));
        SYS_USW_SET_USER_MAC_PREFIX(user_mac_prefix, mac_prefix);
        sal_memcpy(p_usw_l3if_master[lchip]->mac_prefix + loop, &user_mac_prefix, sizeof(mac_addr_t));
    }

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

   return ret;
}

int32 sys_usw_l3if_wb_restore_sub_if(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t    wb_query;
    sys_wb_l3if_prop_t   wb_l3if_prop = {0};
    uint16 entry_cnt = 0;
    uint8 work_status = 0;

    SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_ftm_get_working_status(lchip, &work_status);
    wb_query.buffer = mem_malloc(MEM_L3IF_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_L3IF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_l3if_prop_t, CTC_FEATURE_L3IF, SYS_WB_APPID_L3IF_SUBID_PROP);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_l3if_prop, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        if(work_status == CTC_FTM_MEM_CHANGE_RECOVER && wb_l3if_prop.vaild && (wb_l3if_prop.l3if_type == CTC_L3IF_TYPE_SUB_IF))
        {
            uint8 gchip = 0;
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(wb_l3if_prop.gport);
            if(CTC_IS_LINKAGG_PORT(wb_l3if_prop.gport) || sys_usw_chip_is_local(lchip, gchip))
            {
                uint32 group_id = 0;
                ctc_scl_group_info_t hash_group = {0};
                ctc_scl_entry_t scl_entry;
                ctc_scl_field_action_t field_action;
                ctc_field_key_t field_key = {0};
                ctc_field_port_t   field_port = {0};

                group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_L3IF, 0, 0, 0, 0);
                hash_group.type = CTC_SCL_GROUP_TYPE_NONE;
                ret = sys_usw_scl_create_group(lchip, group_id, &hash_group, 1);
                if ((ret < 0 ) && (ret != CTC_E_EXIST))
                {
                    return ret;
                }

                sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

                scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_SVLAN;
                scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
                CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, group_id, &scl_entry, 1));

                /* KEY */
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = wb_l3if_prop.gport;
                field_key.type = CTC_FIELD_KEY_PORT;
                field_key.ext_data = &field_port;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, done);

                sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
                field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                field_key.data = wb_l3if_prop.vlan_id;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, done);

                sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
                field_key.type = CTC_FIELD_KEY_HASH_VALID;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, done);

                /* ACTION */
                field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
                field_action.data0 = wb_l3if_prop.vlan_ptr;
                CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, done);
                CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, done);
            }
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));
done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}


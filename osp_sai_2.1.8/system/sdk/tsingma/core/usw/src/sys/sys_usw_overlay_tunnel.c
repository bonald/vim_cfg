#if (FEATURE_MODE == 0)
/**
 @file sys_usw_overlay_tunnel.c

 @date 2013-10-25

 @version v2.0
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_const.h"
#include "ctc_hash.h"
#include "ctc_parser.h"

#include "sys_usw_overlay_tunnel.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_vlan.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_dot1ae.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define SYS_OVERLAY_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_overlay_tunnel_master[lchip]) \
        { \
            SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OAM] Module not initialized \n");\
			return CTC_E_NOT_INIT;\
 \
        } \
    } while (0)


#define SYS_OVERLAY_TUNNEL_TYEP_CHECK(PARAM)                \
{                                                           \
    if ((PARAM->type != CTC_OVERLAY_TUNNEL_TYPE_NVGRE) &&   \
        (PARAM->type != CTC_OVERLAY_TUNNEL_TYPE_VXLAN)&& \
        (PARAM->type != CTC_OVERLAY_TUNNEL_TYPE_GENEVE))     \
        return CTC_E_INVALID_PARAM;           \
}

#define SYS_OVERLAY_TUNNEL_VNI_CHECK(VNI)           \
{                                                   \
    if ( VNI > 0xFFFFFF)               \
    {\
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned\n");\
			return CTC_E_BADID;\
    }\
  \
}

#define SYS_OVERLAY_TUNNEL_FID_CHECK(FID)           \
{                                                   \
    if ( FID > 0x3FFF )                 \
    {\
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] This Virtual FID is out of range\n");\
			return CTC_E_BADID;\
    }\
    \
}


#define SYS_OVERLAY_TUNNEL_LOGIC_PORT_CHECK(LOGIC_PORT)    \
{                                                          \
    if ( LOGIC_PORT > MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT) )                             \
        {\
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");\
			return CTC_E_INVALID_PORT;\
        }         \
}

#define SYS_OVERLAY_TUNNEL_SCL_ID_CHECK(SCL_ID)    \
{                                                          \
    if ( SCL_ID > 1 )                             \
    {\
        return CTC_E_INVALID_PARAM;                   \
    }\
}
#define SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX     0xFF

sys_overlay_tunnel_master_t* p_usw_overlay_tunnel_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_OVERLAY_CREATE_LOCK(lchip) \
    do \
    { \
        sal_mutex_create(&p_usw_overlay_tunnel_master[lchip]->mutex); \
        if (NULL == p_usw_overlay_tunnel_master[lchip]->mutex) \
        { \
            return CTC_E_NO_MEMORY; \
        }\
    }while (0)

#define SYS_OVERLAY_LOCK(lchip) \
    sal_mutex_lock(p_usw_overlay_tunnel_master[lchip]->mutex)

#define SYS_OVERLAY_UNLOCK(lchip) \
    sal_mutex_unlock(p_usw_overlay_tunnel_master[lchip]->mutex)

#define CTC_ERROR_RETURN_OVERLAY_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_overlay_tunnel_master[lchip]->mutex); \
            return rv; \
        } \
    }while (0)

enum sys_overlay_group_sub_type_e
{
    SYS_OVERLAY_GROUP_SUB_TYPE_HASH,
    SYS_OVERLAY_GROUP_SUB_TYPE_HASH1,
    SYS_OVERLAY_GROUP_SUB_TYPE_RESOLVE_CONFLICT0,
    SYS_OVERLAY_GROUP_SUB_TYPE_RESOLVE_CONFLICT1,
    SYS_OVERLAY_GROUP_SUB_TYPE_NUM
};
typedef enum sys_overlay_group_sub_type_e sys_overlay_group_sub_type_t;

/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32
_sys_usw_overlay_get_scl_gid(uint8 lchip, uint8 resolve_conflict, uint8 block_id, uint32* gid)
{
    CTC_PTR_VALID_CHECK(gid);

    if (resolve_conflict)
    {
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_OVERLAY, (SYS_OVERLAY_GROUP_SUB_TYPE_RESOLVE_CONFLICT0 + block_id), 0, 0, 0);
    }
    else
    {
        block_id = DRV_IS_DUET2(lchip) ? 0 : block_id;
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_OVERLAY, (SYS_OVERLAY_GROUP_SUB_TYPE_HASH + block_id), 0, 0, 0);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_overlay_tunnel_add_ipda_index(uint8 lchip, uint8 type, uint32 ip_addr , uint32* ip_index)
{
    ds_t ipe_vxlan_nvgre_ipda_ctl;
    uint32 cmd    = 0;
    uint32 valid[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 isVxlan[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 ipda[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint8 i = 0;
    uint8 is_vxlan = (type == CTC_OVERLAY_TUNNEL_TYPE_VXLAN) ? 1 : 0;

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    *ip_index = SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX;
    sal_memset(&ipe_vxlan_nvgre_ipda_ctl, 0, sizeof(ipe_vxlan_nvgre_ipda_ctl));

    cmd = DRV_IOR(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));

    GetIpeVxlanNvgreIpdaCtl(A, array_0_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[3]);

    for ( i = 0 ; i < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX ; i++)
    {
        if (valid[i] && ( is_vxlan == isVxlan[i] ) && ( ip_addr == ipda[i] )) /*return index if hit*/
        {
            *ip_index = i;
            p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[*ip_index]++;
            goto END;
        }
    }

    for ( i = 0 ; i < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX ; i++)
    {
        if (!valid[i])
        {
            *ip_index = i;
            break;
        }
    }

    if (SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX != *ip_index ) /* search a valid position */
    {
        p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[*ip_index] = 1;
        switch ( *ip_index)
        {
            case 0 :
                SetIpeVxlanNvgreIpdaCtl(V, array_0_valid_f, ipe_vxlan_nvgre_ipda_ctl, 1);
                SetIpeVxlanNvgreIpdaCtl(V, array_0_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, is_vxlan);
                SetIpeVxlanNvgreIpdaCtl(V, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, ip_addr);
                break;

            case 1 :
                SetIpeVxlanNvgreIpdaCtl(V, array_1_valid_f, ipe_vxlan_nvgre_ipda_ctl, 1);
                SetIpeVxlanNvgreIpdaCtl(V, array_1_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, is_vxlan);
                SetIpeVxlanNvgreIpdaCtl(V, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, ip_addr);
                break;

            case 2 :
                SetIpeVxlanNvgreIpdaCtl(V, array_2_valid_f, ipe_vxlan_nvgre_ipda_ctl, 1);
                SetIpeVxlanNvgreIpdaCtl(V, array_2_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, is_vxlan);
                SetIpeVxlanNvgreIpdaCtl(V, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, ip_addr);
                break;

            case 3 :
                SetIpeVxlanNvgreIpdaCtl(V, array_3_valid_f, ipe_vxlan_nvgre_ipda_ctl, 1);
                SetIpeVxlanNvgreIpdaCtl(V, array_3_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, is_vxlan);
                SetIpeVxlanNvgreIpdaCtl(V, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, ip_addr);
                break;
        }

        cmd = DRV_IOW(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));
    }

END:
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipda index = %d, isvxlan = %d\n", *ip_index, is_vxlan);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_overlay_tunnel_del_ipda_index(uint8 lchip, uint8 type, uint32 ip_addr , uint32* ip_index)
{
    ds_t ipe_vxlan_nvgre_ipda_ctl;
    uint32 cmd    = 0;
    uint32 valid[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 isVxlan[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 ipda[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint8 i = 0;
    uint8 is_vxlan = (type == CTC_OVERLAY_TUNNEL_TYPE_VXLAN) ? 1 : 0;
    * ip_index = SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX;

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&ipe_vxlan_nvgre_ipda_ctl, 0, sizeof(ipe_vxlan_nvgre_ipda_ctl));

    cmd = DRV_IOR(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));

    GetIpeVxlanNvgreIpdaCtl(A, array_0_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[3]);

    for ( i = 0 ; i < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX ; i++)
    {
        if (valid[i] && ( is_vxlan == isVxlan[i] ) && ( ip_addr == ipda[i] ))
        {
            * ip_index = i;
            break;
        }
    }

    if (i >= SYS_OVERLAT_TUNNEL_MAX_IP_INDEX)
    {
        return CTC_E_NOT_EXIST;
    }

    p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[i]--;
    if ( 0 == p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[i])
    {
        switch (i)
        {
            case 0 :
                SetIpeVxlanNvgreIpdaCtl(V, array_0_valid_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_0_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                break;

            case 1 :
                SetIpeVxlanNvgreIpdaCtl(V, array_1_valid_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_1_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                break;

            case 2 :
                SetIpeVxlanNvgreIpdaCtl(V, array_2_valid_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_2_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                break;

            case 3 :
                SetIpeVxlanNvgreIpdaCtl(V, array_3_valid_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_3_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                SetIpeVxlanNvgreIpdaCtl(V, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, 0);
                break;
        }

        cmd = DRV_IOW(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_overlay_tunnel_get_ipda_index(uint8 lchip, uint8 type, uint32 ip_addr , uint32* ip_index)
{
    ds_t ipe_vxlan_nvgre_ipda_ctl;
    uint32 cmd    = 0;
    uint32 valid[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 isVxlan[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint32 ipda[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint8 i = 0;
    uint8 is_vxlan = (type == CTC_OVERLAY_TUNNEL_TYPE_VXLAN) ? 1 : 0;

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    *ip_index = SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX;
    sal_memset(&ipe_vxlan_nvgre_ipda_ctl, 0, sizeof(ipe_vxlan_nvgre_ipda_ctl));

    cmd = DRV_IOR(IpeVxlanNvgreIpdaCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_vxlan_nvgre_ipda_ctl));

    GetIpeVxlanNvgreIpdaCtl(A, array_0_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_valid_f, ipe_vxlan_nvgre_ipda_ctl, &valid[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_isVxlan_f, ipe_vxlan_nvgre_ipda_ctl, &isVxlan[3]);
    GetIpeVxlanNvgreIpdaCtl(A, array_0_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[0]);
    GetIpeVxlanNvgreIpdaCtl(A, array_1_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[1]);
    GetIpeVxlanNvgreIpdaCtl(A, array_2_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[2]);
    GetIpeVxlanNvgreIpdaCtl(A, array_3_ipDa_f, ipe_vxlan_nvgre_ipda_ctl, &ipda[3]);

    for (i = 0 ; i < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX ; i++)
    {
        if (valid[i] && ( is_vxlan == isVxlan[i] ) && ( ip_addr == ipda[i] ))
        {
            /* return index if hit */
            *ip_index = i;
            break;
        }
    }

    if (*ip_index == SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX)
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_overlay_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action, uint8 key_type, uint8 scl_id, uint8 is_default)
{
    sys_scl_default_action_t default_action;

    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    if(is_default)
    {
        default_action.hash_key_type = key_type;
        default_action.action_type = SYS_SCL_ACTION_TUNNEL;
        default_action.field_action = p_field_action;
        default_action.scl_id = scl_id;
        CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, p_field_action));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_overlay_tunnel_build_key(uint8 lchip, ctc_scl_entry_t* scl_entry, sys_scl_lkup_key_t* p_lkup_key, ctc_overlay_tunnel_param_t* p_tunnel_param, uint32 ipda_info, uint8 is_add)
{
    uint8 ip_ver = CTC_IP_VER_4;
    uint8 with_sa = 0;
    ctc_field_key_t field_key;
    uint32 cmd = 0;
    ds_t ds_data;

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&ds_data, 0, sizeof(ds_t));

    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);

    ip_ver = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IP_VER_6) ? CTC_IP_VER_6 : CTC_IP_VER_4;

    /* build key function must be used after getting entry type */
    if(CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
    {
        with_sa = DRV_IS_DUET2(lchip) ?
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, nvgreTunnelHash2LkpMode_f, ds_data):GetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data)) :
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data):GetIpeUserIdCtl(V, nvgreTunnelHash0LkpMode_f, ds_data));
    }
    else if (CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
    {
        with_sa = DRV_IS_DUET2(lchip) ?
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, vxlanTunnelHash2LkpMode_f, ds_data):GetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data)) :
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data):GetIpeUserIdCtl(V, vxlanTunnelHash0LkpMode_f, ds_data));

    }

    if (CTC_IP_VER_4 == ip_ver)
    {
        field_key.type = CTC_FIELD_KEY_IP_DA;
        field_key.data = ipda_info;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }

        if (with_sa)
        {
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_tunnel_param->ipsa.ipv4;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }
        }

        field_key.type = CTC_FIELD_KEY_VN_ID;
        field_key.data = p_tunnel_param->src_vn_id;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_IPV6_DA;
        field_key.ext_data = p_tunnel_param->ipda.ipv6;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }

        if (with_sa)
        {
            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_data = p_tunnel_param->ipsa.ipv6;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }
        }

        field_key.type = CTC_FIELD_KEY_VN_ID;
        field_key.data = p_tunnel_param->src_vn_id;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }
    }

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key type = %d\n",  scl_entry ? scl_entry->key_type : p_lkup_key->key_type);

    return CTC_E_NONE;
}



STATIC int32
_sys_usw_overlay_tunnel_build_action(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param, uint32 entry_id, uint8 key_type)
{
    sys_nh_info_dsnh_t nh_info;
    uint8 mode = 0;
    uint8 route_mac_index = 0;
    uint8 loop = 0;
    uint8 is_default = 0;
    uint16 dst_fid = 0;
    int32 ret = CTC_E_NONE;
    mac_addr_t mac;
    sys_scl_overlay_t overlay;
    sys_scl_dot1ae_ingress_t dot1ae_ingress;
    ctc_scl_field_action_t field_action;

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&overlay, 0, sizeof(sys_scl_overlay_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&dot1ae_ingress, 0, sizeof(dot1ae_ingress));

    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_VRF))
        +
        (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_HASH_LKUP_PROP)
         ||CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN)
         ||CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_TCAM_LKUP_PROP)
         ||CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN))
        > 1)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_HASH_LKUP_PROP) && !CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_TCAM_LKUP_PROP) && !CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* usw metadata share with stats ptr while gg metadata share with logic src port */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_STATS_EN)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* share u3_g4_isAclFlowTcamOperation */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* usw do not support flow acl lookup using outer-info */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* outer acl look up and flow port acl look up can not be configured at the same time */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* outer ipfix look up and flow port ipfix look up can not be configured at the same time */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_BY_OUTER_HEAD)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_FOLLOW_PORT)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* outer qos look up and flow port qos look up can not be configured at the same time */
    if ((CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_QOS_USE_OUTER_INFO)) && (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_QOS_FOLLOW_PORT)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (p_tunnel_param->inner_taged_chk_mode > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_usw_overlay_tunnel_master[lchip]->could_sec_en == 0) && (p_tunnel_param->dot1ae_chan_id != 0))
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "could sec is disable, cannot bind dot1ae chan id\n");
        return CTC_E_INVALID_PARAM;
    }

    sys_usw_vlan_overlay_get_vni_fid_mapping_mode(lchip, &mode);

    /* fid to vnid N:1 do not support config vnid */
    if(mode && !CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_FID) && !CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* to distinguish whether is default entry or normal entry */
    is_default = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY);

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_STATS_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_tunnel_param->stats_id;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_tunnel_param->action.nh_id;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }
    else
    {
        if(!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_FID))
        {
            /* set dest fid for lookup after decapsulate*/
            SYS_OVERLAY_TUNNEL_VNI_CHECK(p_tunnel_param->action.dst_vn_id);
            ret = sys_usw_vlan_overlay_get_fid(lchip, p_tunnel_param->action.dst_vn_id, &dst_fid);
            if ((ret < 0) && !(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_VRF)))
            {
                return ret;
            }
        }
        else
        {
            CTC_MIN_VALUE_CHECK(p_tunnel_param->fid, 1);
            dst_fid = p_tunnel_param->fid;
        }

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0 = dst_fid;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_SERVICE_ACL_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_METADATA;
        field_action.data0 = p_tunnel_param->metadata;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_VRF))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
        field_action.data0 = p_tunnel_param->vrf_id;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if (p_tunnel_param->policer_id > 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_tunnel_param->policer_id;
        field_action.data1 = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_SERVICE_POLICER_EN);
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DENY_LEARNING))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;
        field_action.data0 = 1;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_MAC_LIMIT_ACTION))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT;
        field_action.data0 = p_tunnel_param->limit_action;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_STATION_MOVE_DISCARD_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    /*Enable Dot1ae*/
    if (0 < p_tunnel_param->dot1ae_chan_id)
    {
        sys_usw_register_dot1ae_bind_sc_t dot1ae_bind_temp;
        sal_memset(&dot1ae_bind_temp, 0, sizeof(dot1ae_bind_temp));
        dot1ae_bind_temp.type = 2;
        dot1ae_bind_temp.dir = CTC_DOT1AE_SC_DIR_RX;
        dot1ae_bind_temp.gport = p_tunnel_param->logic_src_port;
        dot1ae_bind_temp.chan_id = p_tunnel_param->dot1ae_chan_id;
        if (NULL == REGISTER_API(lchip)->dot1ae_bind_sec_chan)
        {
            return CTC_E_NOT_SUPPORT;
        }

        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_bind_sec_chan(lchip,&dot1ae_bind_temp));

        dot1ae_ingress.dot1ae_p2p_mode0_en = 1;
        dot1ae_ingress.dot1ae_p2p_mode1_en = 1;
        dot1ae_ingress.dot1ae_p2p_mode2_en = 1;
        dot1ae_ingress.encrypt_disable_acl = 1;
        dot1ae_ingress.need_dot1ae_decrypt = 1;
        dot1ae_ingress.dsfwdptr = 10;                /* SDK reserved for dot1ae decrypt*/
        dot1ae_ingress.dot1ae_sa_index = (dot1ae_bind_temp.sc_index << 2);
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS;
        field_action.ext_data = &dot1ae_ingress;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }
    else if (0 == p_tunnel_param->dot1ae_chan_id)
    {
        int32 ret1 = sys_usw_scl_get_overlay_tunnel_dot1ae_info(lchip, entry_id, &dot1ae_ingress);
        if (1 == dot1ae_ingress.need_dot1ae_decrypt && (CTC_E_NONE == ret1))
        {
            sys_usw_register_dot1ae_bind_sc_t dot1ae_bind_temp;
            sal_memset(&dot1ae_bind_temp, 0, sizeof(dot1ae_bind_temp));
            dot1ae_bind_temp.type = 2;
            dot1ae_bind_temp.dir = CTC_DOT1AE_SC_DIR_RX;
            dot1ae_bind_temp.gport = dot1ae_ingress.logic_src_port;
            dot1ae_bind_temp.sc_index = (dot1ae_ingress.dot1ae_sa_index >> 2);
            if (NULL == REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
            {
                return CTC_E_NOT_SUPPORT;
            }

            CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip,&dot1ae_bind_temp));

            sal_memset(&dot1ae_ingress, 0, sizeof(dot1ae_ingress));
            field_action.type = SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS;
            field_action.ext_data = &dot1ae_ingress;
            CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
        }
    }

    overlay.is_vxlan = p_tunnel_param->type == CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    overlay.is_tunnel = 1;
    overlay.inner_packet_lookup = 1;
    overlay.ttl_check_en = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_TTL_CHECK);
    overlay.use_outer_ttl = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_OUTER_TTL);
    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD))
    {
        overlay.acl_qos_use_outer_info = 1;
        overlay.acl_qos_use_outer_info_valid = 1;
    }
    else if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT))
    {
        overlay.acl_qos_use_outer_info = 0;
        overlay.acl_qos_use_outer_info_valid = 0;
    }
    else
    {
        /* default action, keep the same with GG default action */
        overlay.acl_qos_use_outer_info = 0;
        overlay.acl_qos_use_outer_info_valid = 1;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_AWARE_TUNNEL_INFO_EN))
    {
        overlay.acl_key_merge_inner_and_outer_hdr = 1;
        overlay.acl_key_merge_inner_and_outer_hdr_valid = 1;
    }
    else if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT))
    {
        overlay.acl_key_merge_inner_and_outer_hdr = 0;
        overlay.acl_key_merge_inner_and_outer_hdr_valid = 0;
    }
    else
    {
        overlay.acl_key_merge_inner_and_outer_hdr = 0;
        overlay.acl_key_merge_inner_and_outer_hdr_valid = 1;
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_BY_OUTER_HEAD))
    {
        overlay.ipfix_and_microflow_use_outer_info = 1;
        overlay.ipfix_and_microflow_use_outer_info_valid = 1;
    }
    else if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_FOLLOW_PORT))
    {
        overlay.ipfix_and_microflow_use_outer_info = 0;
        overlay.ipfix_and_microflow_use_outer_info_valid = 0;
    }
    else
    {
        /* default action, keep the same with GG default action */
        overlay.ipfix_and_microflow_use_outer_info = 0;
        overlay.ipfix_and_microflow_use_outer_info_valid = 1;
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_QOS_USE_OUTER_INFO))
    {
        overlay.classify_use_outer_info = 1;
        overlay.classify_use_outer_info_valid = 1;
    }
    else if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_QOS_FOLLOW_PORT))
    {
        overlay.classify_use_outer_info = 0;
        overlay.classify_use_outer_info_valid = 0;
    }
    else
    {
        /* default action, keep the same with GG default action */
        overlay.classify_use_outer_info = 0;
        overlay.classify_use_outer_info_valid = 1;
    }

    overlay.inner_taged_chk_mode = p_tunnel_param ->inner_taged_chk_mode;
    overlay.logic_src_port = p_tunnel_param ->logic_src_port;
    overlay.logic_port_type = p_tunnel_param ->logic_port_type;
    overlay.payloadOffset = (CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type) ? 4 : 0; /*IpeTunnelDecapCtl.offsetByteShift = 1*/
    overlay.gre_option = (p_tunnel_param->type == CTC_OVERLAY_TUNNEL_TYPE_NVGRE)? 2 : 0;
    overlay.svlan_tpid = p_tunnel_param->tpid_index;

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ARP_ACTIOIN))
    {
        CTC_MAX_VALUE_CHECK(p_tunnel_param->arp_action, MAX_CTC_EXCP_TYPE - 1);
        overlay.arp_ctl_en = 1;
        overlay.arp_exception_type = p_tunnel_param->arp_action;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN))
    {
        ctc_acl_property_t acl;
        sal_memset(&acl, 0, sizeof(ctc_acl_property_t));
        sal_memcpy(&acl, &p_tunnel_param->acl_prop[0], sizeof(ctc_acl_property_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN;
        field_action.ext_data = &acl;
        CTC_ERROR_RETURN(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN))
    {
        /* use new api */
        CTC_MAX_VALUE_CHECK(p_tunnel_param->acl_lkup_num, CTC_MAX_ACL_LKUP_NUM);
        for (loop = 0; loop < p_tunnel_param->acl_lkup_num; loop++)
        {
            SYS_ACL_PROPERTY_CHECK((&p_tunnel_param->acl_prop[loop]));
        }
        sal_memcpy(overlay.acl_pro, p_tunnel_param->acl_prop, sizeof(ctc_acl_property_t) * CTC_MAX_ACL_LKUP_NUM);
        overlay.acl_lkup_num = p_tunnel_param->acl_lkup_num;
    }

    sal_memset(&mac, 0, sizeof(mac_addr_t));
    if (sal_memcmp(&mac, &(p_tunnel_param->route_mac), sizeof(mac_addr_t)))
    {
        CTC_ERROR_RETURN(sys_usw_l3if_binding_inner_router_mac(lchip, &(p_tunnel_param->route_mac), &route_mac_index));
        overlay.router_mac_profile_en = 1;
        overlay.router_mac_profile = route_mac_index;
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_OVERLAY;
    field_action.ext_data = &overlay;
    CTC_ERROR_GOTO(_sys_usw_overlay_add_action_field(lchip, entry_id, &field_action, key_type, p_tunnel_param->scl_id, is_default), ret, error0);

    return CTC_E_NONE;

error0:
    if(overlay.router_mac_profile_en)
    {
        sys_usw_l3if_unbinding_inner_router_mac(lchip, overlay.router_mac_profile);
    }
    return ret;
}

int32
_sys_usw_overlay_tunnel_get_entry_key_type(uint8 lchip, uint8* ip_ver, uint8* packet_type, uint8* with_sa, uint8* by_index, uint8* key_type, ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint32 cmd = 0;
    ds_t ds_data;

    sal_memset(&ds_data, 0, sizeof(ds_t));

    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);

    if (CTC_OVERLAY_TUNNEL_TYPE_GENEVE == p_tunnel_param->type)
    {
        p_tunnel_param->type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    *ip_ver = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_IP_VER_6) ? CTC_IP_VER_6 : CTC_IP_VER_4;

    if (*ip_ver == CTC_IP_VER_4 && ((p_tunnel_param->ipda.ipv4 & 0xF0000000) == 0xE0000000))
    {
        *packet_type = SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC;
    }
    else if (*ip_ver == CTC_IP_VER_6 && ((p_tunnel_param->ipda.ipv6[0] & 0xFF000000) == 0xFF000000))
    {
        *packet_type = SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC;
    }

    if(CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
    {
        *with_sa = DRV_IS_DUET2(lchip) ?
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, nvgreTunnelHash2LkpMode_f, ds_data):GetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data)) :
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data):GetIpeUserIdCtl(V, nvgreTunnelHash0LkpMode_f, ds_data));
    }
    else if (CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
    {
        *with_sa = DRV_IS_DUET2(lchip) ?
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, vxlanTunnelHash2LkpMode_f, ds_data):GetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data)) :
            (p_tunnel_param->scl_id ? GetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data):GetIpeUserIdCtl(V, vxlanTunnelHash0LkpMode_f, ds_data));
    }

    if ((*with_sa != CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA)) && (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
    {
        *by_index = GetIpeUserIdCtl(V, v4UcNvgreUseIpDaIdxKey_f, ds_data);
    }
    else if (CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
    {
        *by_index = GetIpeUserIdCtl(V, v4UcVxlanUseIpDaIdxKey_f, ds_data);
    }

    if (*with_sa)
    {
        /* decap with ip_sa */
        if(CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
        {
            if(CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1;
            }
            else if (CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_V4_MODE1;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1;
            }
        }
        else if(CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
        {
            if(CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1;
            }
            else if (CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_V4_MODE1;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1;
            }
        }
    }
    else
    {
        if(CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
        {
            if(CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0;
            }
            else if (CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0;
            }
        }
        else if(CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
        {
            if(CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0;
            }
            else if (CTC_IP_VER_4 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0;
            }
            else if (CTC_IP_VER_6 == *ip_ver && SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC == *packet_type)
            {
                *key_type = SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0;
            }
        }
    }

    /* one exception, ipv4 && with ipSa mode && ucast && but not use ipda index */
    if (*with_sa && !*by_index && SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == *packet_type && CTC_IP_VER_4 == *ip_ver)
    {
        if (CTC_OVERLAY_TUNNEL_TYPE_NVGRE == p_tunnel_param->type)
        {
            *key_type = SYS_SCL_KEY_HASH_NVGRE_V4_MODE1;
        }
        else if (CTC_OVERLAY_TUNNEL_TYPE_VXLAN == p_tunnel_param->type)
        {
            *key_type = SYS_SCL_KEY_HASH_VXLAN_V4_MODE1;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_overlay_set_default_action(uint8 lchip, uint8 key_type, ctc_overlay_tunnel_param_t* p_tunnel_param, uint8 is_add)
{
    int32 ret = 0;
    ctc_scl_field_action_t   field_action;
    sys_scl_default_action_t default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    default_action.field_action = &field_action;

    /* set pending status */
    default_action.hash_key_type = key_type;
    default_action.action_type = SYS_SCL_ACTION_TUNNEL;
    default_action.scl_id = p_tunnel_param->scl_id;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    if(is_add)
    {
        /* do not care about key, only care about action, entry id is meaningless */
        CTC_ERROR_GOTO(_sys_usw_overlay_tunnel_build_action(lchip, p_tunnel_param, 0, key_type), ret, error0);
    }

    /* clear pending status and install entry*/
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    sys_usw_scl_set_default_action(lchip, &default_action);

    return CTC_E_NONE;

error0:
    /* used for roll back */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    default_action.is_rollback = 1;
    sys_usw_scl_set_default_action(lchip, &default_action);
    return ret;
}

#define __OVERLAY_API__

int32
sys_usw_overlay_tunnel_cloud_sec_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_OVERLAY_INIT_CHECK(lchip);

    field_val = (enable ? 1 : 0);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_encryptVxlanEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    p_usw_overlay_tunnel_master[lchip]->could_sec_en = field_val;

    return CTC_E_NONE;
}

int32
sys_usw_overlay_tunnel_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint8 loop = 0;

    SYS_OVERLAY_INIT_CHECK(lchip);

    SYS_OVERLAY_LOCK(lchip);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Oveylay Tunnel");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "Ipda index count\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-20s: %-20s\n","Ipda index","Count");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------");

    for(loop=0; loop < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX; loop++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-20u: %-20u\n", loop, p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[loop]);
    }

    SYS_DUMP_DB_LOG(p_f, "%s: %s\n", "Cloud sec", p_usw_overlay_tunnel_master[lchip]->could_sec_en?"enable":"disable");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;
}
int32
sys_usw_overlay_tunnel_wb_sync(uint8 lchip)
{
    int32 ret  = CTC_E_NONE;
    uint8 loop = 0;
    ctc_wb_data_t wb_data;
    sys_wb_overlay_tunnel_master_t* p_overlay_wb_master = NULL;

    SYS_USW_FTM_CHECK_NEED_SYNC(lchip);
    /* sync up overlay tunnel matser */
    wb_data.buffer = mem_malloc(MEM_OVERLAY_TUNNEL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    p_overlay_wb_master = (sys_wb_overlay_tunnel_master_t*)wb_data.buffer;

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_overlay_tunnel_master_t, CTC_FEATURE_OVERLAY, SYS_WB_APPID_OVERLAY_SUBID_TUNNEL_MASTER);


    p_overlay_wb_master->lchip = lchip;
    p_overlay_wb_master->version = SYS_WB_VERSION_OVERLAY;
    for(loop = 0; loop < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX; loop++)
    {
        p_overlay_wb_master->ipda_index_cnt[loop] = p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[loop];
    }
    p_overlay_wb_master->could_sec_en = p_usw_overlay_tunnel_master[lchip]->could_sec_en;
    wb_data.valid_cnt = 1;

    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);


done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}

int32
sys_usw_overlay_tunnel_wb_restore(uint8 lchip)
{
    uint8 loop = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_overlay_tunnel_master_t overlay_wb_master;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_OVERLAY_TUNNEL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*restore master*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&overlay_wb_master, 0, sizeof(sys_wb_overlay_tunnel_master_t));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_overlay_tunnel_master_t, CTC_FEATURE_OVERLAY, SYS_WB_APPID_OVERLAY_SUBID_TUNNEL_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query overlay master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy(&overlay_wb_master, wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_OVERLAY, overlay_wb_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    for(loop = 0; loop < SYS_OVERLAT_TUNNEL_MAX_IP_INDEX; loop++)
    {
        p_usw_overlay_tunnel_master[lchip]->ipda_index_cnt[loop] = overlay_wb_master.ipda_index_cnt[loop];
    }
    p_usw_overlay_tunnel_master[lchip]->could_sec_en = overlay_wb_master.could_sec_en;

 done:
     if (wb_query.key)
     {
         mem_free(wb_query.key);
     }

     if (wb_query.buffer)
     {
         mem_free(wb_query.buffer);
     }
     return ret;

}

int32
sys_usw_overlay_tunnel_init(uint8 lchip, void* p_param)
{

    ctc_overlay_tunnel_global_cfg_t* p_config = p_param;
    uint32 cmd = 0;
    ds_t ds_data;
    int32 ret;
    uint32 temp = 0;
    uint32 vxlan_udp_src_port_base = 49152; /* RFC7348*/
    uint8  work_status = 0;
    /* check init */
    if (NULL != p_usw_overlay_tunnel_master[lchip])
    {
        return CTC_E_NONE;
    }

    LCHIP_CHECK(lchip);
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (3==work_status))
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }

    /* config vni and fid mapping mode for vlan module because all related sw table are placed in vlan module */
    CTC_ERROR_RETURN(sys_usw_vlan_overlay_set_vni_fid_mapping_mode(lchip, p_config->vni_mapping_mode));

    if (NULL == p_usw_overlay_tunnel_master[lchip])
    {
        /* init master */
        p_usw_overlay_tunnel_master[lchip] = mem_malloc(MEM_OVERLAY_TUNNEL_MODULE, sizeof(sys_overlay_tunnel_master_t));
        if (!p_usw_overlay_tunnel_master[lchip])
        {
            SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_usw_overlay_tunnel_master[lchip], 0, sizeof(sys_overlay_tunnel_master_t));
    }

    /* config for overlay tunnel decapsulate key type*/
    /* get global config */
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);

    /* config global decap type reg for NvGRE*/
    if (!CTC_FLAG_ISSET(p_config->nvgre_mode, CTC_OVERLAY_TUNNEL_DECAP_IPV4_UC_USE_INDIVIDUL_IPDA))
    {
        SetIpeUserIdCtl(V, v4UcNvgreUseIpDaIdxKey_f, ds_data, 1);
    }
    else
    {
        SetIpeUserIdCtl(V, v4UcNvgreUseIpDaIdxKey_f, ds_data, 0);
    }

    if (!CTC_FLAG_ISSET(p_config->nvgre_mode, CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI))
    {
        if (DRV_IS_DUET2(lchip))
        {
            SetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data, 1);
            SetIpeUserIdCtl(V, nvgreTunnelHash2LkpMode_f, ds_data, 1);
        }
        else
        {
            SetIpeUserIdCtl(V, nvgreTunnelHash0LkpMode_f, ds_data, 1);
            SetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data, 1);
        }
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            SetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data, 0);
            SetIpeUserIdCtl(V, nvgreTunnelHash2LkpMode_f, ds_data, 0);
        }
        else
        {
            SetIpeUserIdCtl(V, nvgreTunnelHash0LkpMode_f, ds_data, 0);
            SetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, ds_data, 0);
        }
    }

    /* config global decap type reg for VxLAN*/
    if (!CTC_FLAG_ISSET(p_config->vxlan_mode, CTC_OVERLAY_TUNNEL_DECAP_IPV4_UC_USE_INDIVIDUL_IPDA))
    {
        SetIpeUserIdCtl(V, v4UcVxlanUseIpDaIdxKey_f, ds_data, 1);
    }
    else
    {
        SetIpeUserIdCtl(V, v4UcVxlanUseIpDaIdxKey_f, ds_data, 0);
    }

    if (!CTC_FLAG_ISSET(p_config->vxlan_mode, CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI))
    {
        if (DRV_IS_DUET2(lchip))
        {
            SetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data, 1);
            SetIpeUserIdCtl(V, vxlanTunnelHash2LkpMode_f, ds_data, 1);
        }
        else
        {
            SetIpeUserIdCtl(V, vxlanTunnelHash0LkpMode_f, ds_data, 1);
            SetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data, 1);
        }
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            SetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data, 0);
            SetIpeUserIdCtl(V, vxlanTunnelHash2LkpMode_f, ds_data, 0);
        }
        else
        {
            SetIpeUserIdCtl(V, vxlanTunnelHash0LkpMode_f, ds_data, 0);
            SetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, ds_data, 0);
        }
    }

    /* write global config*/
    cmd = DRV_IOW(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, OVERLAY_TUNNEL_INIT_ERROR);

    /* global config for UDP dest port and flag*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);
    SetEpePktProcCtl(V, vxlanV4UdpDestPort_f, ds_data, 4789);
    SetEpePktProcCtl(V, vxlanV6UdpDestPort_f, ds_data, 4789);
    SetEpePktProcCtl(V, vxlanUdpSrcPortBase_f, ds_data, vxlan_udp_src_port_base);
    SetEpePktProcCtl(V, vxlanFlagField_f, ds_data , 0x8);
    SetEpePktProcCtl(V, evxlanEnable_f, ds_data , 1);
    SetEpePktProcCtl(V, evxlanFlagField_f, ds_data , 0xC); /*I flag = 1, P flag = 1, O flag = 0*/
    SetEpePktProcCtl(V, evxlanFlagRsv0_f, ds_data , 0);
    SetEpePktProcCtl(V, evxlanFlagRsv2_f, ds_data , 0); /*Ver = 0*/
    SetEpePktProcCtl(V, array_0_vxlanProtocolType_f, ds_data , 1); /*ipv4*/
    SetEpePktProcCtl(V, array_1_vxlanProtocolType_f, ds_data , 2);/*ipv6*/
    SetEpePktProcCtl(V, array_2_vxlanProtocolType_f, ds_data , 3);/*ethernet*/
    SetEpePktProcCtl(V, array_3_vxlanProtocolType_f, ds_data , 4);
    SetEpePktProcCtl(V, geneveEnable_f, ds_data , 1);
    SetEpePktProcCtl(V, geneveV4UdpDestPort_f, ds_data , 6081);
    SetEpePktProcCtl(V, geneveV6UdpDestPort_f, ds_data , 6081);
    SetEpePktProcCtl(V, geneveFlagField_f, ds_data , 0); /*Ver = 0, Open Len = 0*/
    SetEpePktProcCtl(V, geneveFlagRsv0_f, ds_data , 0);/*O flag = 0, C flag = 0*/
    SetEpePktProcCtl(V, geneveFlagRsv2_f, ds_data , 0);
    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, OVERLAY_TUNNEL_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpePktProcVxlanRsvCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);
    SetEpePktProcVxlanRsvCtl(V, gVxlanRsvField_0_vxlanFlags_f, ds_data , 0x8);
    SetEpePktProcVxlanRsvCtl(V, gVxlanRsvField_0_vxlanReserved1_f, ds_data , 0);
    SetEpePktProcVxlanRsvCtl(V, gVxlanRsvField_0_vxlanReserved2_f, ds_data , 0);
    cmd = DRV_IOW(EpePktProcVxlanRsvCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    SetParserLayer4AppCtl(V, vxlanUdpDestPort_f, ds_data, 4789);
    SetParserLayer4AppCtl(V, vxlanEn_f, ds_data , 0x1);
    SetParserLayer4AppCtl(V, evxlanUdpDestPort_f, ds_data, 4789);
    SetParserLayer4AppCtl(V, identifyEvxlanEn_f, ds_data , 0x1);
    SetParserLayer4AppCtl(V, geneveVxlanUdpDestPort_f, ds_data, 6081);
    SetParserLayer4AppCtl(V, geneveVxlanEn_f, ds_data , 0x1);
    SetParserLayer4AppCtl(V, evxlanNextProtocolConvert_f, ds_data, 0x1);
    SetParserLayer4AppCtl(V, evxlanPldIsV4Protocol_f, ds_data, 0x1);
    SetParserLayer4AppCtl(V, evxlanPldIsV6Protocol_f, ds_data, 0x2);
    SetParserLayer4AppCtl(V, evxlanPldIsEthProtocol_f, ds_data, 0x3);
    if (0 == p_config->cloud_sec_en)
    {
        SetParserLayer4AppCtl(V, encryptVxlanEn_f, ds_data, 0);
    }
    else
    {
        SetParserLayer4AppCtl(V, encryptVxlanEn_f, ds_data, 1);
    }
    p_usw_overlay_tunnel_master[lchip]->could_sec_en = p_config->cloud_sec_en;
    SetParserLayer4AppCtl(V, decryptVxlanUdpDestPort_f, ds_data, 4789);
    /*SetParserLayer4AppCtl(V, encryptVxlanUdpDestPort_f, ds_data, 4789);*/
    SetParserLayer4AppCtl(V, vxlanDot1AeCheckBit_f, ds_data, 0x1F);
    SetParserLayer4AppCtl(V, vxlanFlagPolicyGroupCheckEn_f, ds_data, 0);
    SetParserLayer4AppCtl(V, vxlanWithPolicyGidFlagMask_f, ds_data, 0);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret,  OVERLAY_TUNNEL_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserLayer4FlexCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    temp = ((0x0003&0x7) << 2) | (0x0002 >> 6);
    SetParserLayer4FlexCtl(V, layer4ByteSelect0_f, ds_data , temp);
    temp = (0x0003 >> 3)&0x1F;
    SetParserLayer4FlexCtl(V, layer4ByteSelect1_f, ds_data , temp);
    cmd = DRV_IOW(ParserLayer4FlexCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeLoopbackHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);
    SetIpeLoopbackHeaderAdjustCtl(V, disableEloopPktBypassTunnelDecaps_f, ds_data, 0x1);
    cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, OVERLAY_TUNNEL_INIT_ERROR);

    temp = 1;
    cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_vxlanAutoIdentifyPayload_f);
    DRV_IOCTL(lchip, 0, cmd, &temp);

    SYS_OVERLAY_CREATE_LOCK(lchip);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_OVERLAY, sys_usw_overlay_tunnel_wb_sync), ret, OVERLAY_TUNNEL_INIT_ERROR);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_overlay_tunnel_wb_restore(lchip), ret, OVERLAY_TUNNEL_INIT_ERROR);
    }
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_OVERLAY, sys_usw_overlay_tunnel_dump_db), ret, OVERLAY_TUNNEL_INIT_ERROR);
    return CTC_E_NONE;

    OVERLAY_TUNNEL_INIT_ERROR:
    if (p_usw_overlay_tunnel_master[lchip])
    {
        mem_free(p_usw_overlay_tunnel_master[lchip]);
        p_usw_overlay_tunnel_master[lchip] = NULL;
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (3==work_status))
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return ret;

    return CTC_E_NONE;

}

int32
sys_usw_overlay_tunnel_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_overlay_tunnel_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(p_usw_overlay_tunnel_master[lchip]->mutex);
    mem_free(p_usw_overlay_tunnel_master[lchip]);

    return CTC_E_NONE;
}
int32
sys_usw_overlay_tunnel_set_fid (uint8 lchip, uint32 vn_id, uint16 fid )
{
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_OVERLAY_INIT_CHECK(lchip);
    SYS_OVERLAY_TUNNEL_VNI_CHECK(vn_id);
    if(fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
    {
        return CTC_E_BADID;
    }

    SYS_OVERLAY_LOCK(lchip);

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_vlan_overlay_set_fid(lchip, vn_id, fid));

    SYS_OVERLAY_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_overlay_tunnel_get_fid (uint8 lchip,  uint32 vn_id, uint16* p_fid )
{
    SYS_OVERLAY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_fid);
    SYS_OVERLAY_TUNNEL_VNI_CHECK(vn_id);

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_OVERLAY_LOCK(lchip);

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_vlan_overlay_get_fid(lchip, vn_id, p_fid));

    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_overlay_tunnel_get_vn_id(uint8 lchip, uint16 fid, uint32* p_vn_id)
{
    SYS_OVERLAY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vn_id);
    if(fid == 0 || fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
    {
        return CTC_E_BADID;
    }

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_OVERLAY_LOCK(lchip);

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_vlan_overlay_get_vn_id(lchip, fid, p_vn_id));

    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_overlay_tunnel_add_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{

    ctc_scl_entry_t  scl_entry;
    uint32 gid = 0;
    uint32 ipda_info = p_tunnel_param->ipda.ipv4;
    int32 ret = CTC_E_NONE;
    int32 ret1 = CTC_E_NONE;
    uint8 ip_ver = 0;
    uint8 packet_type = SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC;
    uint8 with_sa = 0;
    uint8 by_index = 0;
    ctc_scl_group_info_t group;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    sys_scl_overlay_t overlay;
    mac_addr_t mac;

    SYS_OVERLAY_INIT_CHECK(lchip);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_tunnel_param);
    SYS_OVERLAY_TUNNEL_TYEP_CHECK(p_tunnel_param);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->logic_src_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    CTC_MAX_VALUE_CHECK(p_tunnel_param->logic_port_type, 1);
    SYS_OVERLAY_TUNNEL_SCL_ID_CHECK(p_tunnel_param->scl_id);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->tpid_index, 3);

    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        SYS_OVERLAY_TUNNEL_VNI_CHECK(p_tunnel_param->src_vn_id);
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY)
        && CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_OVERLAY_LOCK(lchip);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&mac, 0, sizeof(mac_addr_t));

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_get_entry_key_type(lchip, &ip_ver, &packet_type, &with_sa, &by_index, &scl_entry.key_type, p_tunnel_param));

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        if (!(DRV_IS_DUET2(lchip)))
        {
            scl_entry.key_type = (scl_entry.key_type == SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 : scl_entry.key_type;
            scl_entry.key_type = (scl_entry.key_type == SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 : scl_entry.key_type;
        }
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_set_default_action(lchip, scl_entry.key_type, p_tunnel_param, 1));
        SYS_OVERLAY_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;
    /* in overlay tunnel, only care about resolve conflict */
    scl_entry.resolve_conflict = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX);

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_get_scl_gid(lchip, scl_entry.resolve_conflict, p_tunnel_param->scl_id, &gid));

    group.type = CTC_SCL_GROUP_TYPE_NONE;
    /* use default priority for inter module */
    group.priority = p_tunnel_param->scl_id;
    ret = sys_usw_scl_create_group(lchip, gid, &group, 1);
    if ((ret < 0 ) && (ret != CTC_E_EXIST))
    {
        SYS_OVERLAY_UNLOCK(lchip);
        return ret;
    }

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1));

    if ((ip_ver == CTC_IP_VER_4) && by_index && (SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == packet_type) && with_sa)
    {
        _sys_usw_overlay_tunnel_add_ipda_index(lchip, p_tunnel_param->type, p_tunnel_param->ipda.ipv4 , &ipda_info);
        if (ipda_info == SYS_OVERLAY_TUNNEL_INVALID_IPDA_INDEX)
        {
            SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [OVERLAY_TUNNEL] The register to store IP DA index is full\n");
            ret =  CTC_E_NO_RESOURCE;
            goto error0;
        }
    }

    /* build scl key */
    CTC_ERROR_GOTO(_sys_usw_overlay_tunnel_build_key(lchip, &scl_entry, NULL, p_tunnel_param, ipda_info, 1), ret, error1);

    if(!scl_entry.resolve_conflict)
    {
        /* hash entry */
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error1);
    }

    /* build scl action */
    CTC_ERROR_GOTO(_sys_usw_overlay_tunnel_build_action(lchip, p_tunnel_param, scl_entry.entry_id, scl_entry.key_type), ret , error1);

    /* install entry */
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error2);

    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;

error2:
    if (sal_memcmp(&mac, &(p_tunnel_param->route_mac), sizeof(mac_addr_t)))
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_OVERLAY;
        field_action.ext_data = &overlay;
        ret1 = sys_usw_scl_get_action_field(lchip, scl_entry.entry_id, &field_action);
        if (ret1 && ret1 != CTC_E_NOT_EXIST)
        {
            /* do nothing */
        }
        else if (ret1 == CTC_E_NONE)
        {
            sys_usw_l3if_unbinding_inner_router_mac(lchip, overlay.router_mac_profile);
        }
    }

error1:
    if ((ip_ver == CTC_IP_VER_4) && by_index && (SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == packet_type) && with_sa)
    {
        _sys_usw_overlay_tunnel_del_ipda_index(lchip, p_tunnel_param->type, p_tunnel_param->ipda.ipv4 , &ipda_info);
    }
error0:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);
    SYS_OVERLAY_UNLOCK(lchip);

    return ret;
}


int32
sys_usw_overlay_tunnel_remove_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    sys_scl_lkup_key_t lkup_key;
    uint32 gid = 0;
    uint8 ip_ver = 0;
    uint8 packet_type = 0;
    uint8 by_index = 0;
    uint8 with_sa = 0;
    uint32 ipda_info = p_tunnel_param->ipda.ipv4;
    int32 ret = CTC_E_NONE;
    int32 ret1 = CTC_E_NONE;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    sys_scl_overlay_t overlay;
    sys_scl_dot1ae_ingress_t dot1ae_ingress;

    SYS_OVERLAY_INIT_CHECK(lchip);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_tunnel_param);
    SYS_OVERLAY_TUNNEL_TYEP_CHECK(p_tunnel_param);
    SYS_OVERLAY_TUNNEL_SCL_ID_CHECK(p_tunnel_param->scl_id);
    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        SYS_OVERLAY_TUNNEL_VNI_CHECK(p_tunnel_param->src_vn_id);
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY)
        && CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_OVERLAY_LOCK(lchip);

    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&overlay, 0, sizeof(sys_scl_overlay_t));
    sal_memset(&dot1ae_ingress, 0, sizeof(dot1ae_ingress));

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_get_entry_key_type(lchip, &ip_ver, &packet_type, &with_sa, &by_index, &lkup_key.key_type, p_tunnel_param));
    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        if (!(DRV_IS_DUET2(lchip)))
        {
            lkup_key.key_type = (lkup_key.key_type == SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 : lkup_key.key_type;
            lkup_key.key_type = (lkup_key.key_type == SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 : lkup_key.key_type;
        }
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_set_default_action(lchip, lkup_key.key_type, p_tunnel_param, 0));
        SYS_OVERLAY_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_get_scl_gid(lchip, CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX), p_tunnel_param->scl_id, &gid));

    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;
    lkup_key.resolve_conflict = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX);
    lkup_key.group_id = gid;
    lkup_key.group_priority = p_tunnel_param->scl_id;

    if ((ip_ver == CTC_IP_VER_4) && by_index && (SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == packet_type) && with_sa)
    {
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_get_ipda_index(lchip, p_tunnel_param->type, p_tunnel_param->ipda.ipv4 , &ipda_info));
    }

    /* build scl key */
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_build_key(lchip, NULL, &lkup_key, p_tunnel_param, ipda_info, 0));

    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX))
    {
        /* hash entry */
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

    /* Unbind Dot1ae channel */
    ret1 = sys_usw_scl_get_overlay_tunnel_dot1ae_info(lchip, lkup_key.entry_id, &dot1ae_ingress);
    if (1 == dot1ae_ingress.need_dot1ae_decrypt && (CTC_E_NONE == ret1))
    {
        sys_usw_register_dot1ae_bind_sc_t dot1ae_bind_temp;
        sal_memset(&dot1ae_bind_temp, 0, sizeof(dot1ae_bind_temp));
        dot1ae_bind_temp.type = 2;
        dot1ae_bind_temp.dir = CTC_DOT1AE_SC_DIR_RX;
        dot1ae_bind_temp.gport = dot1ae_ingress.logic_src_port;
        dot1ae_bind_temp.sc_index = (dot1ae_ingress.dot1ae_sa_index >> 2);
        if (REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
        {
            CTC_ERROR_RETURN_OVERLAY_UNLOCK(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip,&dot1ae_bind_temp));
        }
    }

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_uninstall_entry(lchip, lkup_key.entry_id, 1));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_OVERLAY;
    field_action.ext_data = &overlay;
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_get_action_field(lchip, lkup_key.entry_id, &field_action));
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_remove_entry(lchip, lkup_key.entry_id, 1));

    if ((ip_ver == CTC_IP_VER_4) && by_index && (SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == packet_type) && with_sa)
    {
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_del_ipda_index(lchip, p_tunnel_param->type, p_tunnel_param->ipda.ipv4 , &ipda_info));
    }

    /* unbind inner router mac only after entry has been removed */

    if(ret == CTC_E_NONE && overlay.router_mac_profile)
    {
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_l3if_unbinding_inner_router_mac(lchip, overlay.router_mac_profile));
    }

    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;
}


int32
sys_usw_overlay_tunnel_update_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{

    sys_scl_lkup_key_t lkup_key;
    uint32 gid = 0;
    uint8 ip_ver = 0;
    uint8 packet_type = 0;
    uint8 by_index = 0;
    uint8 with_sa = 0;
    uint32 ipda_info = p_tunnel_param->ipda.ipv4;
    int32 ret = 0;
    int32 ret1 = CTC_E_NONE;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    sys_scl_overlay_t overlay;
    mac_addr_t mac;

    SYS_OVERLAY_INIT_CHECK(lchip);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_tunnel_param);
    SYS_OVERLAY_TUNNEL_TYEP_CHECK(p_tunnel_param);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->logic_src_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    CTC_MAX_VALUE_CHECK(p_tunnel_param->logic_port_type, 1);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->tpid_index, 3);
    SYS_OVERLAY_TUNNEL_SCL_ID_CHECK(p_tunnel_param->scl_id);
    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        SYS_OVERLAY_TUNNEL_VNI_CHECK(p_tunnel_param->src_vn_id);
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY)
        && CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_OVERLAY_LOCK(lchip);

    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&overlay, 0, sizeof(sys_scl_overlay_t));
    sal_memset(&mac, 0, sizeof(mac_addr_t));

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_get_entry_key_type(lchip, &ip_ver, &packet_type, &with_sa, &by_index, &lkup_key.key_type, p_tunnel_param));
    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        if (!(DRV_IS_DUET2(lchip)))
        {
            lkup_key.key_type = (lkup_key.key_type == SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_VXLAN_V4_MODE1 : lkup_key.key_type;
            lkup_key.key_type = (lkup_key.key_type == SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1) ? SYS_SCL_KEY_HASH_NVGRE_V4_MODE1 : lkup_key.key_type;
        }
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_set_default_action(lchip, lkup_key.key_type, p_tunnel_param, 1));
        SYS_OVERLAY_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_get_scl_gid(lchip, CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX), p_tunnel_param->scl_id, &gid));

    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;
    lkup_key.resolve_conflict = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX);
    lkup_key.group_id = gid;
    lkup_key.group_priority = p_tunnel_param->scl_id;

    if ((ip_ver == CTC_IP_VER_4) && by_index && (SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC == packet_type) && with_sa)
    {
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_get_ipda_index(lchip, p_tunnel_param->type, p_tunnel_param->ipda.ipv4 , &ipda_info));
    }

    /* build scl key */
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(_sys_usw_overlay_tunnel_build_key(lchip, NULL, &lkup_key, p_tunnel_param, ipda_info, 0));

    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX))
    {
        /* hash entry */
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    }

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));



    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_add_action_field(lchip, lkup_key.entry_id, &field_action));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON;
    CTC_ERROR_RETURN_OVERLAY_UNLOCK(sys_usw_scl_add_action_field(lchip, lkup_key.entry_id, &field_action));

    ret = _sys_usw_overlay_tunnel_build_action(lchip, p_tunnel_param, lkup_key.entry_id, lkup_key.key_type);
    if (CTC_E_NONE != ret)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, lkup_key.entry_id, &field_action);
        SYS_OVERLAY_UNLOCK(lchip);
        return ret;
    }

    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, lkup_key.entry_id, 1), ret, error0);

    SYS_OVERLAY_UNLOCK(lchip);

    return CTC_E_NONE;

error0:
    if (sal_memcmp(&mac, &(p_tunnel_param->route_mac), sizeof(mac_addr_t)))
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_OVERLAY;
        field_action.ext_data = &overlay;
        ret1 = sys_usw_scl_get_action_field(lchip, lkup_key.entry_id, &field_action);
        if (ret1 && ret1 != CTC_E_NOT_EXIST)
        {
            /* do nothing */
        }
        else if (ret1 == CTC_E_NONE)
        {
            sys_usw_l3if_unbinding_inner_router_mac(lchip, overlay.router_mac_profile);
        }
    }
    SYS_OVERLAY_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_overlay_tunnel_show_status(uint8 lchip)
{
    uint8 loop = 0;
    uint8 index = 0;
    uint32 gid = 0;
    uint32 cmd = 0;
    uint32 entry_count = 0;
    uint32 total = 0;
    IpeUserIdCtl_m ipe_userid_ctl;
    uint8 num = 0;
    char *array[16] = {
    "nvgre_uc_v4_mode0",
    "nvgre_uc_v4_mode1",
    "nvgre_mc_v4_mode0",
    "nvgre_v4_mode1",
    "nvgre_uc_v6_mode0",
    "nvgre_uc_v6_mode1",
    "nvgre_mc_v6_mode0",
    "nvgre_mc_v6_mode1",
    "vxlan_uc_v4_mode0",
    "vxlan_uc_v4_mode1",
    "vxlan_mc_v4_mode0",
    "vxlan_v4_mode1",
    "vxlan_uc_v6_mode0",
    "vxlan_uc_v6_mode1",
    "vxlan_mc_v6_mode0",
    "vxlan_mc_v6_mode1"
    };
    SYS_OVERLAY_INIT_CHECK(lchip);
    SYS_OVERLAY_LOCK(lchip);

    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ipe_userid_ctl);

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "================= Overlay Tunnel Overall Status ==================\n");
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-25s%s\n", "Tunnel Type", "Key", "IPDA Mode");
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------\n");

    if(GetIpeUserIdCtl(V, nvgreTunnelHash1LkpMode_f, &ipe_userid_ctl))
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-25s", "nvgre", "ipda + ipsa + vni");
    }
    else
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-25s", "nvgre", "ipda + vni");
    }

    if(GetIpeUserIdCtl(V, v4UcNvgreUseIpDaIdxKey_f, &ipe_userid_ctl))
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "share");
    }
    else
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "individual");
    }

    if(GetIpeUserIdCtl(V, vxlanTunnelHash1LkpMode_f, &ipe_userid_ctl))
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-25s", "vxlan", "ipda + ipsa + vni");
    }
    else
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-25s", "vxlan", "ipda + vni");
    }

    if(GetIpeUserIdCtl(V, v4UcVxlanUseIpDaIdxKey_f, &ipe_userid_ctl))
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "share");
    }
    else
    {
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "individual");
    }

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    /* group status */
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-15s%s\n", "NO.", "SCL Group Id", "Entry Type");
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------\n");
    _sys_usw_overlay_get_scl_gid(lchip, 0, 0, &gid);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u0x%-.8X     %s\n", num++, gid, "HASH0");
    if (!(DRV_IS_DUET2(lchip)))
    {
        _sys_usw_overlay_get_scl_gid(lchip, 0, 1, &gid);
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u0x%-.8X     %s\n", num++, gid, "HASH1");
    }
    _sys_usw_overlay_get_scl_gid(lchip, 1, 0, &gid);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u0x%-.8X     %s\n", num++, gid, "TCAM0");
    _sys_usw_overlay_get_scl_gid(lchip, 1, 1, &gid);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u0x%-.8X     %s\n", num++, gid, "TCAM1");

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    /* entry status */
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-25s%s\n", "No.", "TYPE", "ENTRY_COUNT");
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------\n");

    for(loop = SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0; loop <= SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1; loop++)
    {
        sys_usw_scl_get_overlay_tunnel_entry_status(lchip, loop, CTC_FEATURE_OVERLAY, &entry_count);
        SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u%-25s%u\n", index, array[index], entry_count);
        index++;
        total += entry_count;
    }

    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total entry number : %u\n", total);
    SYS_OVERLAY_TUNNEL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cloud sec : %s\n", p_usw_overlay_tunnel_master[lchip]->could_sec_en?"enable":"disable");

    SYS_OVERLAY_UNLOCK(lchip);
    return CTC_E_NONE;
}

#endif


#if (FEATURE_MODE == 0)
/**
 @file sys_usw_trill.c

 @date 2015-10-25

 @version v3.0
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

#include "sys_usw_chip.h"
#include "sys_usw_trill.h"
#include "sys_usw_common.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_ftm.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_dma.h"

#include "usw/include/drv_enum.h"
#include "usw/include/drv_io.h"
#include "usw/include/drv_common.h"
#include "sys_usw_l3if.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_TRILL_INIT_CHECK(lchip)    \
    do{ \
        LCHIP_CHECK(lchip); \
            SYS_LCHIP_CHECK_ACTIVE(lchip); \
    }while (0)

#define SYS_TRILL_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(trill, trill, TRILL_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)


#define SYS_TRILL_DBG_DUMP(FMT, ...)               \
    {                                                 \
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__);\
    }

struct sys_trill_route_s
{
    /*hw para*/
    uint32 ad_index;
    uint32 key_index;
    uint32 key_tbl_id;
    DsTrillDa_m ds_trill_da;
    ds_t ds_trill_route_key;

    uint8 is_host1;
    uint8 is_remove;
};
typedef struct sys_trill_route_s sys_trill_route_t;

enum sys_trill_group_sub_type_e
{
    SYS_TRILL_GROUP_SUB_TYPE_HASH,
    SYS_TRILL_GROUP_SUB_TYPE_HASH_1,
    SYS_TRILL_GROUP_SUB_TYPE_RESOLVE_CONFLICT0,
    SYS_TRILL_GROUP_SUB_TYPE_RESOLVE_CONFLICT1,
    SYS_TRILL_GROUP_SUB_TYPE_NUM
};
typedef enum sys_trill_group_sub_type_e sys_trill_group_sub_type_t;

/****************************************************************************
*
* Function
*
*****************************************************************************/
int32
_sys_usw_trill_get_scl_gid(uint8 lchip, uint8 resolve_conflict, uint8 block_id, uint32* gid)
{
    CTC_PTR_VALID_CHECK(gid);

    if (resolve_conflict)
    {
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_TRILL, (SYS_TRILL_GROUP_SUB_TYPE_RESOLVE_CONFLICT0 + block_id), 0, 0, 0);
    }
    else
    {
        *gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_TRILL, (SYS_TRILL_GROUP_SUB_TYPE_HASH+block_id), 0, 0, 0);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_trill_get_entry_key_type(uint8 lchip, uint8* key_type, uint8* scl_id, ctc_trill_tunnel_t* p_trill_tunnel)
{
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_tunnel);
    CTC_PTR_VALID_CHECK(key_type);

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_MCAST))
    {
        *key_type = SYS_SCL_KEY_HASH_TRILL_MC;
        *scl_id = 0;
    }
    else
    {
        *key_type = SYS_SCL_KEY_HASH_TRILL_UC;
        *scl_id = 1;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_trill_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action, uint8 key_type, uint8 scl_id, uint8 is_default)
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
_sys_usw_trill_tunnel_build_key(uint8 lchip, ctc_scl_entry_t* scl_entry, sys_scl_lkup_key_t* p_lkup_key, ctc_trill_tunnel_t* p_trill_tunnel, uint8 is_add)
{
    ctc_field_key_t field_key;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (is_add)
    {
        CTC_PTR_VALID_CHECK(scl_entry);
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key type = %d\n",  scl_entry->key_type);
    }

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    field_key.type = CTC_FIELD_KEY_INGRESS_NICKNAME;
    field_key.data = p_trill_tunnel->ingress_nickname;
    if (is_add)
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
    }

    field_key.type = CTC_FIELD_KEY_EGRESS_NICKNAME;
    field_key.data = p_trill_tunnel->egress_nickname;
    if (is_add)
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_tunnel_build_action(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel, uint32 entry_id, uint8 key_type)
{
    sys_nh_info_dsnh_t nh_info;
    sys_scl_trill_t trill;
    ctc_scl_field_action_t field_action;
    ctc_scl_logic_port_t scl_logic_port;
    mac_addr_t mac;
    uint8 route_mac_index = 0;
    uint8 is_default = 0;
    int32 ret = CTC_E_NONE;
    uint8 scl_id = 0;
    uint8 temp_key_type = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&trill, 0, sizeof(sys_scl_trill_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_entry_key_type(lchip, &temp_key_type, &scl_id, p_trill_tunnel));

    is_default = CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY);

    scl_logic_port.logic_port =  p_trill_tunnel->logic_src_port & 0x3FFF;
    scl_logic_port.logic_port_type = 1;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    field_action.ext_data = &scl_logic_port;
    CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));

    if (p_trill_tunnel->stats_id)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_trill_tunnel->stats_id;
        CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));
    }

    if(p_trill_tunnel->fid)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0 = p_trill_tunnel->fid;
        CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));
    }
    else if(p_trill_tunnel->nh_id)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_trill_tunnel->nh_id;
        CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_SERVICE_ACL_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
        CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));
    }

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_ARP_ACTION))
    {
        CTC_MAX_VALUE_CHECK(p_trill_tunnel->arp_action, MAX_CTC_EXCP_TYPE - 1);
        trill.arp_ctl_en = 1;
        trill.arp_exception_type = p_trill_tunnel->arp_action;
    }

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_DENY_LEARNING))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;
        field_action.data0 = 1;
        CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));
    }

    trill.is_tunnel = 1;
    trill.inner_packet_lookup = 1;

    sal_memset(&mac, 0, sizeof(mac_addr_t));
    if (sal_memcmp(&mac, &(p_trill_tunnel->route_mac), sizeof(mac_addr_t)))
    {
        CTC_ERROR_RETURN(sys_usw_l3if_binding_inner_router_mac(lchip, &(p_trill_tunnel->route_mac), &route_mac_index));
        trill.router_mac_profile_en = 1;
        trill.router_mac_profile = route_mac_index;
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_TRILL;
    field_action.ext_data = &trill;
    CTC_ERROR_GOTO(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default), ret, error0);

    return CTC_E_NONE;

error0:
    if(trill.router_mac_profile_en)
    {
        sys_usw_l3if_unbinding_inner_router_mac(lchip, trill.router_mac_profile);
    }
    return ret;

}

int32
_sys_usw_trill_get_check_entry_key_type(uint8 lchip, uint8* key_type, uint8* scl_id, ctc_trill_route_t* p_trill_route)
{
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_route);

    if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
    {
        if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK))
        {
            *key_type = SYS_SCL_KEY_HASH_TRILL_MC_ADJ;
            *scl_id = 1;
        }
        else if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_SRC_PORT_CHECK))
        {
            *key_type = SYS_SCL_KEY_HASH_TRILL_MC_RPF;
            *scl_id = 0;
        }
    }
    else
    {
        *key_type = SYS_SCL_KEY_HASH_TRILL_UC_RPF;
        *scl_id = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_check_build_key(uint8 lchip, ctc_scl_entry_t* scl_entry, sys_scl_lkup_key_t* p_lkup_key, ctc_trill_route_t* p_trill_route, uint8 is_add)
{
    ctc_field_key_t field_key;
    uint8 key_type;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(is_add)
    {
        key_type = scl_entry->key_type;
    }
    else
    {
        key_type = p_lkup_key->key_type;
    }

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if(SYS_SCL_KEY_HASH_TRILL_UC_RPF  ==  key_type || SYS_SCL_KEY_HASH_TRILL_MC_RPF == key_type)
    {
        field_key.type = CTC_FIELD_KEY_INGRESS_NICKNAME;
        field_key.data = p_trill_route->ingress_nickname;

        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }
    }

    if(SYS_SCL_KEY_HASH_TRILL_MC_RPF == key_type || SYS_SCL_KEY_HASH_TRILL_MC_ADJ == key_type)
    {
        field_key.type = CTC_FIELD_KEY_EGRESS_NICKNAME;
        field_key.data = p_trill_route->egress_nickname;
        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }
    }

    if(SYS_SCL_KEY_HASH_TRILL_MC_ADJ == key_type)
    {
        ctc_field_port_t st_port;

        field_key.type = CTC_FIELD_KEY_PORT;

        st_port.gport = p_trill_route->src_gport;
        st_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_key.ext_data = (void *)&st_port;

        if (is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
        }
    }

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key type = %d\n",  key_type);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_check_build_action(uint8 lchip, ctc_trill_route_t* p_trill_route, uint32 entry_id, uint8 key_type)
{
    sys_scl_trill_t trill;
    ctc_scl_field_action_t field_action;
    uint8 is_default = 0;
    uint8 scl_id = 0;
    uint8 temp_key_type = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&trill, 0, sizeof(sys_scl_trill_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    is_default = CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY);

    if ((CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
        &&(CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK)))
    {
        trill.bind_check_en = 1;
        trill.bind_mac_sa = 1;
        sal_memcpy(&(trill.mac_sa), &(p_trill_route->mac_sa), sizeof(mac_addr_t));
    }
    else
    {
        trill.bind_check_en = 1;
        trill.multi_rpf_check = 1;
        trill.src_gport = p_trill_route->src_gport;
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_TRILL;
    field_action.ext_data = &trill;
    CTC_ERROR_RETURN(_sys_usw_trill_get_check_entry_key_type(lchip, &temp_key_type, &scl_id, p_trill_route));
    CTC_ERROR_RETURN(_sys_usw_trill_add_action_field(lchip, entry_id, &field_action, key_type, scl_id, is_default));

    return CTC_E_NONE;
}

int32
sys_usw_trill_show_route_entry(uint8 lchip, uint32 tbl_id)
{
    int32 ret = CTC_E_NONE;
    uint32 step = 512;
    uint32* p_buffer = NULL;
    uint8 tbl_size = TABLE_ENTRY_SIZE(lchip, tbl_id);
    drv_work_platform_type_t platform_type;
    sys_dma_tbl_rw_t dma_rw;
    uint32 i = 0, j = 0;
    void* ds_key = NULL;

    uint32 valid = 0;
    uint32 hashType = 0;
    uint32 egressNickname = 0;
    uint32 _type = 0;
    uint32 vlanId = 0;
    uint32 dsAdIndex = 0;

    uint32 tbl_index = 0;

    sal_memset(&dma_rw, 0, sizeof(sys_dma_tbl_rw_t));
    p_buffer = (uint32*)mem_malloc(MEM_TRILL_MODULE, step*tbl_size);
    if (NULL == p_buffer)
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    dma_rw.rflag = 1;
    dma_rw.entry_len = tbl_size;
    dma_rw.entry_num =  step;
    dma_rw.buffer = p_buffer;
    drv_get_platform_type(lchip, &platform_type);
    for (i = 0; i < (TABLE_MAX_INDEX(lchip, tbl_id)/step); i++)
    {
        sal_memset(dma_rw.buffer, 0, step*tbl_size);
        if (platform_type == HW_PLATFORM)
        {
            CTC_ERROR_GOTO(drv_usw_table_get_hw_addr(lchip, tbl_id, i*step, &dma_rw.tbl_addr, FALSE), ret, END);
        }
        else
        {
            dma_rw.tbl_addr = (tbl_id << 18) + (i*step);
        }
        CTC_ERROR_GOTO(sys_usw_dma_rw_table(lchip, &dma_rw), ret, END);

        for (j = 0; j < dma_rw.entry_num; j++)
        {
            tbl_index = j + i*step + 32;
            ds_key = (DsFibHost0TrillHashKey_m *)dma_rw.buffer + j;
            if (DsFibHost0TrillHashKey_t == tbl_id)
            {
                 GetDsFibHost0TrillHashKey(A, valid_f, ds_key, &valid);
                 GetDsFibHost0TrillHashKey(A, hashType_f, ds_key, &hashType);
                 if (valid && (FIBHOST0PRIMARYHASHTYPE_TRILL == hashType))
                 {
                     GetDsFibHost0TrillHashKey(A, egressNickname_f, ds_key, &egressNickname);
                     GetDsFibHost0TrillHashKey(A, dsAdIndex_f, ds_key, &dsAdIndex);
                     vlanId = 0;
                 }
                 else
                 {
                     continue;
                 }
            }
            else if(DsFibHost1TrillMcastVlanHashKey_t == tbl_id)
            {
                GetDsFibHost1TrillMcastVlanHashKey(A, hashType_f, ds_key, &hashType);
                GetDsFibHost1TrillMcastVlanHashKey(A, _type_f, ds_key, &_type);
                GetDsFibHost1TrillMcastVlanHashKey(A, valid_f, ds_key, &valid);
                if (valid && (FIBHOST1PRIMARYHASHTYPE_OTHER == hashType) && (1 == _type))
                {
                    GetDsFibHost1TrillMcastVlanHashKey(A, egressNickname_f, ds_key, &egressNickname);
                    GetDsFibHost1TrillMcastVlanHashKey(A, vlanId_f, ds_key, &vlanId);
                    GetDsFibHost1TrillMcastVlanHashKey(A, dsAdIndex_f, ds_key, &dsAdIndex);
                }
                else
                {
                    continue;
                }
            }
            if (DsFibHost1TrillMcastVlanHashKey_t == tbl_id)
            {
                SYS_TRILL_DBG_DUMP("0x%-13x%-15d%-10d%-10d\n", tbl_index, egressNickname, vlanId, dsAdIndex);
            }
            else
            {
                SYS_TRILL_DBG_DUMP("0x%-13x%-15d%-10s%-10d\n", tbl_index, egressNickname, "-", dsAdIndex);
            }
        }
    }

END:
    mem_free(p_buffer);
    return ret;
}

int32
sys_usw_trill_dump_route_entry(uint8 lchip)
{
    SYS_TRILL_INIT_CHECK(lchip);
    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    SYS_TRILL_DBG_DUMP("%-15s%-15s%-10s%-10s\n", "ENTRY_IDX", "EGR_NICKNAME", "VLAN_ID", "AD_INDEX");
    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    CTC_ERROR_RETURN(sys_usw_trill_show_route_entry(lchip, DsFibHost0TrillHashKey_t));
    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    SYS_TRILL_DBG_DUMP("Table:\n");
    SYS_TRILL_DBG_DUMP(" --Key :DsFibHost0TrillHashKey\n");
    SYS_TRILL_DBG_DUMP(" --AD  :DsTrillDa\n");
    SYS_TRILL_DBG_DUMP("\n");

    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    SYS_TRILL_DBG_DUMP("%-15s%-15s%-10s%-10s\n", "ENTRY_IDX", "EGR_NICKNAME", "VLAN_ID", "AD_INDEX");
    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    CTC_ERROR_RETURN(sys_usw_trill_show_route_entry(lchip, DsFibHost1TrillMcastVlanHashKey_t));
    SYS_TRILL_DBG_DUMP("------------------------------------------------\n");
    SYS_TRILL_DBG_DUMP("Table:\n");
    SYS_TRILL_DBG_DUMP(" --Key :DsFibHost1TrillMcastVlanHashKey\n");
    SYS_TRILL_DBG_DUMP(" --AD  :DsTrillDa\n");
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_build_route_key(uint8 lchip, ctc_trill_route_t* p_trill_route, sys_trill_route_t* p_sys_trill_route, uint8 is_add)
{
    drv_acc_in_t  fib_acc_in;
    drv_acc_out_t fib_acc_out;
    drv_acc_in_t  cpu_acc_in;
    drv_acc_out_t cpu_acc_out;
    ds_t* ds_trill_route_key = NULL;
    uint8 is_host1 = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
    sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));
    sal_memset(&cpu_acc_in, 0, sizeof(cpu_acc_in));
    sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));

    ds_trill_route_key = &(p_sys_trill_route->ds_trill_route_key);

    if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST)&&p_trill_route->vlan_id)
    {
        is_host1 = 1;

        SetDsFibHost1TrillMcastVlanHashKey(V, hashType_f, ds_trill_route_key, FIBHOST1PRIMARYHASHTYPE_OTHER);
        SetDsFibHost1TrillMcastVlanHashKey(V, _type_f, ds_trill_route_key, 1);
        SetDsFibHost1TrillMcastVlanHashKey(V, valid_f, ds_trill_route_key, 1);
        SetDsFibHost1TrillMcastVlanHashKey(V, egressNickname_f, ds_trill_route_key, p_trill_route->egress_nickname);
        SetDsFibHost1TrillMcastVlanHashKey(V, vlanId_f, ds_trill_route_key, p_trill_route->vlan_id);
    }
    else
    {
        SetDsFibHost0TrillHashKey(V, hashType_f, ds_trill_route_key, FIBHOST0PRIMARYHASHTYPE_TRILL);
        SetDsFibHost0TrillHashKey(V, valid_f, ds_trill_route_key, 1);
        SetDsFibHost0TrillHashKey(V, egressNickname_f, ds_trill_route_key, p_trill_route->egress_nickname);
        if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
        {
            SetDsFibHost0TrillHashKey(V, trillType_f, ds_trill_route_key, 1);
        }
    }

    if (is_host1)
    {
        cpu_acc_in.type =  DRV_ACC_TYPE_LOOKUP;
        cpu_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        cpu_acc_in.tbl_id = DsFibHost1TrillMcastVlanHashKey_t;
        cpu_acc_in.data = (void*)(ds_trill_route_key);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));

        if (is_add)
        {
            if (1 == cpu_acc_out.is_conflict)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
			return CTC_E_HASH_CONFLICT;

            }
            if (1 == cpu_acc_out.is_hit)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

            }
        }
        else
        {
            if (0 == cpu_acc_out.is_hit)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

            }
        }
        p_sys_trill_route->key_index = cpu_acc_out.key_index;

        cpu_acc_in.index = cpu_acc_out.key_index;
        cpu_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        cpu_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        cpu_acc_in.tbl_id = DsFibHost1TrillMcastVlanHashKey_t;
        cpu_acc_in.data = (void*)(ds_trill_route_key);

        sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));
        CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));
        p_sys_trill_route->ad_index = GetDsFibHost1TrillMcastVlanHashKey(V, dsAdIndex_f, &cpu_acc_out.data);
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"DsFibHost1TrillMcastVlanHashKey index :0x%x, ad_index = 0x%x\n",
                                                                                p_sys_trill_route->key_index, p_sys_trill_route->ad_index);
    }
    else
    {
        fib_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        fib_acc_in.tbl_id = DsFibHost0TrillHashKey_t;
        fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        fib_acc_in.data = (void*)ds_trill_route_key;

        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));
        if (is_add)
        {
            if (1 == fib_acc_out.is_conflict)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
			return CTC_E_HASH_CONFLICT;

            }
            if (1 == fib_acc_out.is_hit)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

            }
        }
        else
        {
            if (0 == fib_acc_out.is_hit)
            {
                SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

            }
        }
        p_sys_trill_route->key_index = fib_acc_out.key_index;
        p_sys_trill_route->ad_index = fib_acc_out.ad_index;
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"DsFibHost0TrillHashKey index :0x%x\n, ad_index = 0x%x\n",
                                                                                 p_sys_trill_route->key_index, p_sys_trill_route->ad_index);
    }

    p_sys_trill_route->is_host1 = is_host1;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_build_route_ad(uint8 lchip, ctc_trill_route_t* p_trill_route, sys_trill_route_t* p_sys_trill_route)
{
    DsTrillDa_m* p_ds_trill_da = NULL;
    sys_nh_info_dsnh_t nh_info;
    uint32 fwd_ptr = 0;
    uint32 fwd_offset = 0;

    p_ds_trill_da = &(p_sys_trill_route->ds_trill_da);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SetDsTrillDa(V, ttlLimit_f, p_ds_trill_da, 1);
    SetDsTrillDa(V, mcastCheckEn_f, p_ds_trill_da, 1);
    SetDsTrillDa(V, esadiCheckEn_f, p_ds_trill_da, 1);


    if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DISCARD))
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, 1, &fwd_offset, 0));
        SetDsTrillDa(V, nextHopPtrValid_f, p_ds_trill_da, 0);
        SetDsTrillDa(V, dsFwdPtr_f, p_ds_trill_da, fwd_offset);
    }
    else if (p_trill_route->nh_id)
    {
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_trill_route->nh_id, &nh_info, 0));
        if (nh_info.merge_dsfwd)
        {
            if (nh_info.ecmp_valid)
            {
                return CTC_E_INVALID_PARAM;
            }
            SetDsTrillDa(V, nextHopPtrValid_f, p_ds_trill_da, 1);
            SetDsTrillDa(V, adDestMap_f, p_ds_trill_da, nh_info.dest_map);
            SetDsTrillDa(V, adNextHopPtr_f, p_ds_trill_da,nh_info.dsnh_offset);
            SetDsTrillDa(V, adNextHopExt_f, p_ds_trill_da,nh_info.nexthop_ext);
            SetDsTrillDa(V, adApsBridgeEn_f, p_ds_trill_da,nh_info.aps_en);
        }
        else
        {
            SetDsTrillDa(V, nextHopPtrValid_f, p_ds_trill_da, 0);
            if (nh_info.ecmp_valid)
            {
                SetDsTrillDa(V, ecmpEn_f, p_ds_trill_da, 1);
                SetDsTrillDa(V, ecmpGroupId_f, p_ds_trill_da, nh_info.ecmp_group_id);
            }
            else
            {
                sys_usw_nh_get_dsfwd_offset(lchip, p_trill_route->nh_id, &fwd_ptr, 0);
                SetDsTrillDa(V, dsFwdPtr_f, p_ds_trill_da, fwd_ptr);
            }
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_write_route_hw(uint8 lchip, sys_trill_route_t* p_sys_trill_route, uint8 is_add)
{
    drv_acc_in_t  fib_acc_in;
    drv_acc_out_t fib_acc_out;
    drv_acc_in_t  cpu_acc_in;
    drv_acc_out_t cpu_acc_out;
    uint32 cmd = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
    sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));
    sal_memset(&cpu_acc_in, 0, sizeof(cpu_acc_in));
    sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));

    /*write AD*/
    if(!is_add)
    {
        sal_memset(&(p_sys_trill_route->ds_trill_da), 0, sizeof(DsTrillDa_m));
    }
    cmd = DRV_IOW(DsTrillDa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_trill_route->ad_index, cmd, &(p_sys_trill_route->ds_trill_da)));

    /*write key*/
    if (p_sys_trill_route->is_host1)
    {
        SetDsFibHost1TrillMcastVlanHashKey(V, dsAdIndex_f, (&(p_sys_trill_route->ds_trill_route_key)), p_sys_trill_route->ad_index);
        cpu_acc_in.data = (void*)(&(p_sys_trill_route->ds_trill_route_key));
        cpu_acc_in.tbl_id = DsFibHost1TrillMcastVlanHashKey_t;
        cpu_acc_in.index = p_sys_trill_route->key_index;
        cpu_acc_in.type = is_add?DRV_ACC_TYPE_ADD:DRV_ACC_TYPE_DEL;
        cpu_acc_in.op_type = DRV_ACC_OP_BY_INDEX;

        CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));
    }
    else
    {
        SetDsFibHost0TrillHashKey(V, dsAdIndex_f, (&(p_sys_trill_route->ds_trill_route_key)), p_sys_trill_route->ad_index);
        if (!is_add)
        {
            SetDsFibHost0TrillHashKey(V, valid_f, (&(p_sys_trill_route->ds_trill_route_key)), 0);
        }
        fib_acc_in.data = (void*)(&(p_sys_trill_route->ds_trill_route_key));
        fib_acc_in.type = DRV_ACC_TYPE_ADD;
        fib_acc_in.tbl_id = DsFibHost0TrillHashKey_t;
        fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_BIT_SET(fib_acc_in.flag, DRV_ACC_OVERWRITE_EN);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_trill_add_route_entry(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    sys_trill_route_t sys_trill_route;
    int32 ret = CTC_E_NONE;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_route);
    sal_memset(&sys_trill_route, 0, sizeof(sys_trill_route_t));

    /*build key*/
    CTC_ERROR_RETURN(_sys_usw_trill_build_route_key(lchip, p_trill_route, &sys_trill_route, 1));

    /*build AD*/
    CTC_ERROR_RETURN(_sys_usw_trill_build_route_ad(lchip, p_trill_route, &sys_trill_route));

    /*alloc AD index*/
    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsTrillDa_t, 0, 1, 1, &sys_trill_route.ad_index));
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"DsTrillDa index :0x%x\n", sys_trill_route.ad_index);

    /*write hw*/
    CTC_ERROR_GOTO(_sys_usw_trill_write_route_hw(lchip, &sys_trill_route, 1), ret, error0);

    return CTC_E_NONE;

error0:
    /*free AD index*/
     sys_usw_ftm_free_table_offset(lchip, DsTrillDa_t, 0, 1, sys_trill_route.ad_index);

    return ret;
}

STATIC int32
_sys_usw_trill_remove_route_entry(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    sys_trill_route_t sys_trill_route;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_route);
    sal_memset(&sys_trill_route, 0, sizeof(sys_trill_route_t));

    /*get ad index by key*/
    CTC_ERROR_RETURN(_sys_usw_trill_build_route_key(lchip, p_trill_route, &sys_trill_route, 0));

    /*free AD index*/
    CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(lchip, DsTrillDa_t, 0, 1, sys_trill_route.ad_index));

    /*write hw*/
    CTC_ERROR_RETURN(_sys_usw_trill_write_route_hw(lchip, &sys_trill_route, 0));

    return CTC_E_NONE;
}

int32
_sys_usw_trill_set_route_default_action(uint8 lchip, uint8 key_type, ctc_trill_route_t* p_trill_route, uint8 is_add, uint8 scl_id)
{
    ctc_scl_field_action_t   field_action;
    sys_scl_default_action_t default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    default_action.field_action = &field_action;

    /* set pending status */
    default_action.hash_key_type = key_type;
    default_action.action_type = SYS_SCL_ACTION_TUNNEL;
    default_action.scl_id = scl_id;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    if(is_add)
    {
        /* do not care about key, only care about action, entry id is meaningless */
        _sys_usw_trill_check_build_action(lchip, p_trill_route, 0, key_type);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_trill_add_check_entry(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    ctc_scl_entry_t  scl_entry;
    uint32 gid = 0;
    ctc_scl_group_info_t group;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    int32 ret = CTC_E_NONE;
    uint8   scl_id = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_route);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_check_entry_key_type(lchip, &scl_entry.key_type, &scl_id, p_trill_route));

    if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_set_route_default_action(lchip, scl_entry.key_type, p_trill_route, 1, scl_id));
        return CTC_E_NONE;
    }

    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;
    scl_entry.resolve_conflict = 0;

    CTC_ERROR_RETURN(_sys_usw_trill_get_scl_gid(lchip, scl_entry.resolve_conflict, scl_id, &gid));

    group.type = CTC_SCL_GROUP_TYPE_NONE;
    group.priority = scl_id;
    ret = sys_usw_scl_create_group(lchip, gid, &group, 1);
    if ((ret < 0 ) && (ret != CTC_E_EXIST))
    {
        return ret;
    }

    CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1));

    /* build scl key */
    CTC_ERROR_GOTO(_sys_usw_trill_check_build_key(lchip, &scl_entry, NULL, p_trill_route, 1), ret, error0);

    if(!scl_entry.resolve_conflict)
    {
        /* hash entry */
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error0);
    }

    /* build scl action */
    CTC_ERROR_GOTO(_sys_usw_trill_check_build_action(lchip, p_trill_route, scl_entry.entry_id, scl_entry.key_type), ret , error0);

    /* install entry */
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error0);

    return CTC_E_NONE;

error0:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

    return ret;
}

STATIC int32
_sys_usw_trill_remove_check_entry(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    uint8  scl_id = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_route);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_route->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_route->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_route->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_route->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "src_gport :0x%x\n", p_trill_route->src_gport);

    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_check_entry_key_type(lchip, &lkup_key.key_type, &scl_id, p_trill_route));
    lkup_key.group_priority = scl_id;

    if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_set_route_default_action(lchip, lkup_key.key_type, p_trill_route, 0, scl_id));
        return CTC_E_NONE;
    }

    /* build scl key */
    CTC_ERROR_RETURN(_sys_usw_trill_check_build_key(lchip, NULL, &lkup_key, p_trill_route, 0));

    /* hash entry */
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    CTC_ERROR_RETURN(_sys_usw_trill_get_scl_gid(lchip, 0, scl_id, &lkup_key.group_id));

    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, lkup_key.entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, lkup_key.entry_id, 1));

    return CTC_E_NONE;
}

int32
_sys_usw_trill_set_tunnel_default_action(uint8 lchip, uint8 key_type, ctc_trill_tunnel_t* p_trill_tunnel, uint8 is_add, uint8 scl_id)
{
    ctc_scl_field_action_t   field_action;
    sys_scl_default_action_t default_action;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&default_action, 0, sizeof(sys_scl_default_action_t));

    default_action.field_action = &field_action;

    /* set pending status */
    default_action.hash_key_type = key_type;
    default_action.action_type = SYS_SCL_ACTION_TUNNEL;
    default_action.scl_id = scl_id;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    if(is_add)
    {
        /* do not care about key, only care about action, entry id is meaningless */
        CTC_ERROR_RETURN(_sys_usw_trill_tunnel_build_action(lchip, p_trill_tunnel, 0, key_type));
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip, &default_action));

    return CTC_E_NONE;
}

#define _______API_______
int32
sys_usw_trill_init(uint8 lchip, void* trill_global_cfg)
{
    return CTC_E_NONE;
}

int32
sys_usw_trill_deinit(uint8 lchip)
{
    return CTC_E_NONE;
}

int32
sys_usw_trill_add_route(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_TRILL_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trill_route);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_route->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_route->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_route->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vlan_id :%d\n", p_trill_route->vlan_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_route->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "src_gport :0x%x\n", p_trill_route->src_gport);

    SYS_GLOBAL_PORT_CHECK(p_trill_route->src_gport);
    if (p_trill_route->vlan_id > CTC_MAX_VLAN_ID)
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");
        return CTC_E_BADID;
    }
    if ((!CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
        && (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_SRC_PORT_CHECK))
        ||
        ((CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
        && (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK))))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_add_check_entry(lchip, p_trill_route));
    }
    else
    {
        if(CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY))
        {
            /* default entry can not be used for this situation */
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(_sys_usw_trill_add_route_entry(lchip, p_trill_route));
    }

    return CTC_E_NONE;
}

int32
sys_usw_trill_remove_route(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_TRILL_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trill_route);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_route->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_route->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_route->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vlan_id :%d\n", p_trill_route->vlan_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_route->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "src_gport :0x%x\n", p_trill_route->src_gport);

    SYS_GLOBAL_PORT_CHECK(p_trill_route->src_gport);
    if (p_trill_route->vlan_id > CTC_MAX_VLAN_ID)
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");
        return CTC_E_BADID;
    }
    if ((!CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MCAST))
        && (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_SRC_PORT_CHECK))
        || (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK)))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_remove_check_entry(lchip, p_trill_route));
    }
    else
    {
        if (CTC_FLAG_ISSET(p_trill_route->flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY))
        {
            /* default entry can not be used for this situation */
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(_sys_usw_trill_remove_route_entry(lchip, p_trill_route));
    }

    return CTC_E_NONE;
}

int32
sys_usw_trill_add_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    ctc_scl_entry_t  scl_entry;
    uint32 gid = 0;
    ctc_scl_group_info_t group;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    int32 ret = CTC_E_NONE;
    int32 ret1 = CTC_E_NONE;
    mac_addr_t mac;
    sys_scl_trill_t trill;
    uint8  scl_id = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_TRILL_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trill_tunnel);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_tunnel->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_tunnel->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_tunnel->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d\n", p_trill_tunnel->fid);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_tunnel->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "logic_src_port :0x%x\n", p_trill_tunnel->logic_src_port);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_id :%d\n", p_trill_tunnel->stats_id);

    if ( p_trill_tunnel->logic_src_port > 0x3FFF )
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");
        return CTC_E_INVALID_PORT;
    }

    if ( p_trill_tunnel->fid > 0x3FFF )
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&group, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_entry_key_type(lchip, &scl_entry.key_type, &scl_id, p_trill_tunnel));

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_set_tunnel_default_action(lchip, scl_entry.key_type, p_trill_tunnel, 1, scl_id));
        return CTC_E_NONE;
    }

    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;
    scl_entry.resolve_conflict = 0;

    CTC_ERROR_RETURN(_sys_usw_trill_get_scl_gid(lchip, scl_entry.resolve_conflict, scl_id, &gid));

    group.type = CTC_SCL_GROUP_TYPE_NONE;
    group.priority = scl_id;
    ret = sys_usw_scl_create_group(lchip, gid, &group, 1);
    if ((ret < 0 ) && (ret != CTC_E_EXIST))
    {
        return ret;
    }

    CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1));

    /* build scl key */
    CTC_ERROR_GOTO(_sys_usw_trill_tunnel_build_key(lchip, &scl_entry, NULL, p_trill_tunnel, 1), ret, error0);

    if(!scl_entry.resolve_conflict)
    {
        /* hash entry */
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error0);
    }

    /* build scl action */
    CTC_ERROR_GOTO(_sys_usw_trill_tunnel_build_action(lchip, p_trill_tunnel, scl_entry.entry_id, scl_entry.key_type), ret , error0);

    /* install entry */
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error1);

    return CTC_E_NONE;

error1:
    if (sal_memcmp(&mac, &(p_trill_tunnel->route_mac), sizeof(mac_addr_t)))
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_TRILL;
        field_action.ext_data = &trill;
        ret1 = sys_usw_scl_get_action_field(lchip, scl_entry.entry_id, &field_action);
        if (ret1 && ret1 != CTC_E_NOT_EXIST)
        {
            /* do nothing */
        }
        else if (ret1 == CTC_E_NONE)
        {
            sys_usw_l3if_unbinding_inner_router_mac(lchip, trill.router_mac_profile);
        }
    }

error0:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

    return ret;
}

int32
sys_usw_trill_remove_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    sys_scl_trill_t trill;
    int32 ret = CTC_E_NONE;
    uint8 scl_id = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_TRILL_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trill_tunnel);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_tunnel->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_tunnel->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_tunnel->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d\n", p_trill_tunnel->fid);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_tunnel->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "logic_src_port :0x%x\n", p_trill_tunnel->logic_src_port);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_id :%d\n", p_trill_tunnel->stats_id);

    if ( p_trill_tunnel->logic_src_port > 0x3FFF )
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");
			return CTC_E_INVALID_PORT;

    }
    if ( p_trill_tunnel->fid > 0x3FFF )
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&trill, 0, sizeof(sys_scl_trill_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_entry_key_type(lchip, &lkup_key.key_type, &scl_id, p_trill_tunnel));
    lkup_key.group_priority = scl_id;

    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_set_tunnel_default_action(lchip, lkup_key.key_type, p_trill_tunnel, 0, scl_id));
        return CTC_E_NONE;
    }

    /* build scl key */
    CTC_ERROR_RETURN(_sys_usw_trill_tunnel_build_key(lchip, NULL, &lkup_key, p_trill_tunnel, 0));

    /* hash entry */
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    CTC_ERROR_RETURN(_sys_usw_trill_get_scl_gid(lchip, 0, scl_id, &lkup_key.group_id));

    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_TRILL;
    field_action.ext_data = &trill;
    ret = sys_usw_scl_get_action_field(lchip, lkup_key.entry_id, &field_action);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        return ret;
    }
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, lkup_key.entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, lkup_key.entry_id, 1));

    /* unbind inner router mac only after entry has been removed */
    if(ret == CTC_E_NONE && trill.router_mac_profile)
    {
        CTC_ERROR_RETURN(sys_usw_l3if_unbinding_inner_router_mac(lchip, trill.router_mac_profile));
    }

    return CTC_E_NONE;
}


int32
sys_usw_trill_update_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    int32 ret = CTC_E_NONE;
    int32 ret1 = CTC_E_NONE;
    mac_addr_t mac;
    sys_scl_trill_t trill;
    sys_scl_lkup_key_t lkup_key;
    uint8 scl_id = 0;

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_TRILL_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trill_tunnel);

    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag :0x%x\n", p_trill_tunnel->flag);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname :%d\n", p_trill_tunnel->egress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname :%d\n", p_trill_tunnel->ingress_nickname);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d\n", p_trill_tunnel->fid);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh_id :%d\n", p_trill_tunnel->nh_id);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "logic_src_port :0x%x\n", p_trill_tunnel->logic_src_port);
    SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_id :%d\n", p_trill_tunnel->stats_id);

    if ( p_trill_tunnel->logic_src_port > 0x3FFF )
    {
        SYS_TRILL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");
        return CTC_E_INVALID_PORT;
    }

    if ( p_trill_tunnel->fid > 0x3FFF )
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));

    CTC_ERROR_RETURN(_sys_usw_trill_get_entry_key_type(lchip, &lkup_key.key_type, &scl_id, p_trill_tunnel));
    lkup_key.group_priority = scl_id;
    if (CTC_FLAG_ISSET(p_trill_tunnel->flag, CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY))
    {
        CTC_ERROR_RETURN(_sys_usw_trill_set_tunnel_default_action(lchip, lkup_key.key_type, p_trill_tunnel, 1, scl_id));
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_trill_tunnel_build_key(lchip, NULL, &lkup_key, p_trill_tunnel, 0));
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    CTC_ERROR_RETURN(_sys_usw_trill_get_scl_gid(lchip, 0, scl_id, &lkup_key.group_id));

    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;
    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));

    /* build scl action */
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, lkup_key.entry_id, &field_action));
    ret = (_sys_usw_trill_tunnel_build_action(lchip, p_trill_tunnel, lkup_key.entry_id, lkup_key.key_type));
    if (CTC_E_NONE != ret)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, lkup_key.entry_id, &field_action);
        return ret;
    }
    /* install entry */
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, lkup_key.entry_id, 1), ret, error);

    return CTC_E_NONE;

error:
    if (sal_memcmp(&mac, &(p_trill_tunnel->route_mac), sizeof(mac_addr_t)))
    {
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_TRILL;
        field_action.ext_data = &trill;
        ret1 = sys_usw_scl_get_action_field(lchip, lkup_key.entry_id, &field_action);
        if (ret1 && ret1 != CTC_E_NOT_EXIST)
        {
            /* do nothing */
        }
        else if (ret1 == CTC_E_NONE)
        {
            sys_usw_l3if_unbinding_inner_router_mac(lchip, trill.router_mac_profile);
        }
    }

    return ret;
}

#endif

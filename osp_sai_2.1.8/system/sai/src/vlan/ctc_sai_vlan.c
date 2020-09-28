#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_vlan.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_notifier_call.h>


static ctc_sai_check_u32_range_t g_range_0_vlan_address_limit =
{
    .min = 0,
    .max = 255,
};

static ctc_sai_check_object_type_range_t g_range_objtype_stp =
{
    .min = SAI_OBJECT_TYPE_STP_INSTANCE,
    .max = SAI_OBJECT_TYPE_STP_INSTANCE,
};

static ctc_sai_attr_entry_info_t g_sai_vlan_attr_entries[] = {
    {
        .id     = SAI_VLAN_ATTR_PORT_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_u32_range_fn,
            .func_parameter = &g_range_0_vlan_address_limit,
        }
    },
    {
        .id     = SAI_VLAN_ATTR_STP_INSTANCE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_object_type_range_fn,
            .func_parameter = &g_range_objtype_stp,
        }
    },
    {
        .id     = SAI_VLAN_ATTR_LEARN_DISABLE,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_META_DATA,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_IGMP_SNOOPING_EN,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_IGMP_SNOOPING_DISCARD_UNKNOWN,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_DHCPSNOOPING,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_INGRESS_MIRROR_SESSION,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_VLAN_ATTR_EGRESS_MIRROR_SESSION,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#define ________SAI_SAI_INNER_API_FUNC

stbl_stp_t*
ctc_sai_vlan_get_stp(sai_vlan_id_t vlan_id)
{
    stbl_vlan_key_t     vlan_key;
    stbl_stp_key_t      stp_key;
    stbl_vlan_t         *p_sdb_vlan = NULL;
    stbl_stp_t          *p_sdb_stp = NULL;

    vlan_key.vid = vlan_id;
    p_sdb_vlan = stbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_sdb_vlan)
    {
        return NULL;
    }

    stp_key.stp_id = CTC_SAI_OBJECT_INDEX_GET(p_sdb_vlan->stp_oid);
    p_sdb_stp = stbl_stp_get_stp(&stp_key);
    
    return p_sdb_stp;
}

sai_status_t
ctc_sai_vlan_create_vlan(
    _In_  sai_vlan_id_t vlan_id)
{
#if defined(GOLDENGATE) || defined(USW)
    int32_t             sdk_ret = CTC_E_NONE;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));

    user_vlan.vlan_id   = vlan_id;
    user_vlan.fid       = vlan_id;
    user_vlan.user_vlanptr = vlan_id;
    /* SYSTEM MODIFIED by wangjj for fix bug 42563, 2017-01-17 */
    /* modified by liwh for bug 49888, 2018-11-07 */
    //user_vlan.flag = CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    /* liwh end */

    sdk_ret = ctc_vlan_create_uservlan(&user_vlan);

    if(sdk_ret == CTC_E_NONE || sdk_ret == CTC_E_EXIST)
    {
        //sdk_ret = ctc_vlan_set_learning_en(vlan_id, TRUE);
        sdk_ret = ctc_vlan_set_bridge_en(vlan_id, TRUE);
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
#else
    uint16 vid = vlan_id;
    uint16 fid = vlan_id;
    uint16 learning_en = 1;
    uint16 bridge_en = 1;
    uint32 groupId = vlan_id;
    ctc_l2dflt_addr_t l2dflt;
    int32_t             sdk_ret = CTC_E_NONE;  
    
    sdk_ret = ctc_vlan_create_vlan(vid);
    sdk_ret = ctc_vlan_set_fid(vid, fid);

    sal_memset(&l2dflt, 0, sizeof(ctc_l2dflt_addr_t));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = groupId;
    /* modified by liwh for bug 49888, 2018-11-07 */
    //l2dflt.flag = CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    /* liwh end */
    ctc_l2_add_default_entry(&l2dflt);

    ctc_vlan_set_learning_en(vid, learning_en);
    ctc_vlan_set_bridge_en(vid, bridge_en);
    
    /*l2 vlan should not send arp packet to cpu*/
    ctc_vlan_set_arp_excp_type(vid, CTC_EXCP_NORMAL_FWD);
    ctc_vlan_set_dhcp_excp_type(vid, CTC_EXCP_NORMAL_FWD);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
#endif
}

sai_status_t
ctc_sai_vlan_remove_vlan(
    _In_  sai_vlan_id_t vlan_id)
{
    stbl_stp_t              *p_sdb_stp = NULL;
    int32_t                 sdk_ret = CTC_E_NONE;

#ifdef GREATBELT
    uint16                  vid = vlan_id;
    ctc_l2dflt_addr_t       l2dflt_addr;
#endif
    sdk_ret = ctc_vlan_destroy_vlan(vlan_id);
#ifdef GREATBELT
    ctc_vlan_set_transmit_en(vid, FALSE);
    ctc_vlan_set_receive_en(vid, FALSE);
    
    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));
    l2dflt_addr.fid = vlan_id;
    ctc_l2_remove_default_entry(&l2dflt_addr);
#endif

    if(CTC_E_NONE == sdk_ret)
    {
        p_sdb_stp = ctc_sai_vlan_get_stp(vlan_id);
        if (p_sdb_stp)
        {
            ctc_sai_stp_remove_vlan(p_sdb_stp, vlan_id);
        }
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_vlan_db_alloc(sai_vlan_id_t vlan_id)
{
    stbl_vlan_t vlan;

    sal_memset(&vlan, 0, sizeof(vlan));
    vlan.key.vid = vlan_id;
    
    stbl_vlan_add_vlan(&vlan);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_get_vlan_port_list(
    _In_ sai_vlan_id_t      vlan_id,
    _Out_ sai_attribute_t   *pattr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_set_max_learned_address(
    _In_  const sai_vlan_id_t   vlan_id,
    _In_  const sai_attribute_t *pattr)
{
    int32_t                     sdk_ret = CTC_E_NONE;
    ctc_security_learn_limit_t  learn_limit;

    sal_memset(&learn_limit,0,sizeof(learn_limit));

    learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN;
    learn_limit.vlan       = vlan_id;
    learn_limit.limit_num  = pattr->value.u32;
    learn_limit.limit_action = CTC_MACLIMIT_ACTION_DISCARD;

    sdk_ret = ctc_mac_security_set_learn_limit(&learn_limit);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_vlan_get_max_learned_address(
    _In_  const sai_vlan_id_t   vlan_id,
    _Inout_  sai_attribute_t    *pattr)
{
    int32_t                     sdk_ret = CTC_E_NONE;
    ctc_security_learn_limit_t  learn_limit;

    sal_memset(&learn_limit,0,sizeof(learn_limit));

    learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN;
    learn_limit.vlan       = vlan_id;

    sdk_ret = ctc_mac_security_get_learn_limit(&learn_limit);

    if(sdk_ret != CTC_E_NONE)
    {
        goto out;
    }

    pattr->value.u32 = learn_limit.limit_num;

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
__ctc_sai_vlan_add_ports_to_vlan(
    _In_  const sai_vlan_id_t   vlan_id,
    _In_  const uint32_t        port_count,
    _In_  const sai_vlan_port_t *port_list)
{
    uint32_t    port_id     = 0;
    int32_t     sdk_ret     = CTC_E_NONE;
    int32_t     taged       = 0;

    for(port_id = 0; port_id < port_count; port_id++)
    {
        CTC_SAI_ERROR_GOTO(ctc_vlan_add_port(vlan_id,
                        CTC_SAI_OBJECT_INDEX_GET(port_list[port_id].port_id)),
                        sdk_ret,out);

        switch(port_list[port_id].tagging_mode)
        {
        case SAI_VLAN_PORT_UNTAGGED:
            taged = 0;
            break;
        case SAI_VLAN_PORT_TAGGED:
            taged = 1;
            break;
        case SAI_VLAN_PORT_PRIORITY_TAGGED:

            break;
        }

        CTC_SAI_ERROR_GOTO(ctc_vlan_set_tagged_port(vlan_id,
                        CTC_SAI_OBJECT_INDEX_GET(port_list[port_id].port_id),
                        taged),
                        sdk_ret,out);
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
__convert_lag_port_to_ports(
    _In_  const sai_vlan_port_t *plag_port,
    _In_        sai_vlan_port_t *pport_arry,
    _In_  const uint32_t        port_max,
    _Out_       uint32_t        *lag_mumber_count)
{
    sai_status_t    ret           = SAI_STATUS_SUCCESS;
    uint32_t        port_count    = 0;
    sai_object_id_t port_oid_array[CTC_MAX_LPORT];
    sai_attribute_t lag_ports;

    sal_memset(port_oid_array, 0, sizeof(port_oid_array));

    lag_ports.value.objlist.list = port_oid_array;
    lag_ports.value.objlist.count = port_max;
    CTC_SAI_ERROR_GOTO(ctc_sai_lag_get_ports(plag_port->port_id,&lag_ports),ret,out);

    for(port_count = 0; port_count < lag_ports.value.objlist.count; port_count++)
    {
        pport_arry[port_count].port_id = lag_ports.value.objlist.list[port_count];
        pport_arry[port_count].tagging_mode = plag_port->tagging_mode;
    }

    *lag_mumber_count = lag_ports.value.objlist.count;
out:
    return ret;
}

sai_status_t
ctc_sai_vlan_add_ports_to_vlan(
    _In_  const sai_vlan_id_t   vlan_id,
    _In_  const uint32_t    port_count,
    _In_  const sai_vlan_port_t *port_list)
{

    sai_status_t    ret           = SAI_STATUS_SUCCESS;
    uint32_t        port_id = 0;
    uint32_t        port_current_idx = 0;
    uint32_t        lag_port_member_count = 0;
    sai_vlan_port_t vlan_port_arry[CTC_MAX_LPORT];
    sai_vlan_port_t *pvlan_port_arry = NULL;
    stbl_port_t     *p_sdb_port = NULL;
	ctc_sai_vlan_notifier_t	vlan_notif = {0};
#ifdef GREATBELT
    ctc_l2dflt_addr_t l2dflt_addr;
    uint32 gport = 0;
    uint32 i = 0;
#endif
    pvlan_port_arry = &vlan_port_arry[0];

    if(NULL == pvlan_port_arry)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }

    sal_memset(pvlan_port_arry, 0, sizeof(vlan_port_arry));

    for (port_id = 0; port_id < port_count; port_id++)
    {
            
        p_sdb_port = ctc_sai_port_get_port(port_list[port_id].port_id);
        if (NULL == p_sdb_port)
        {
            continue;
        }
        else
        {
            CTC_BMP_SET(p_sdb_port->vlan_bmp, vlan_id);
        }

        if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            pvlan_port_arry[port_current_idx].port_id = port_list[port_id].port_id;
            pvlan_port_arry[port_current_idx].tagging_mode = port_list[port_id].tagging_mode;
            port_current_idx++;
			
			vlan_notif.op = CTC_SAI_LAG_NF_NEW_VLAN_MEMBER;
			vlan_notif.vlan_id = vlan_id;
			vlan_notif.vlan_member_port_id = port_list[port_id].port_id;
			ctc_sai_vlan_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_NEW_VLAN_MEMBER, &vlan_notif);
			
            continue;
        }

        if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            CTC_SAI_ERROR_GOTO(
                __convert_lag_port_to_ports(&port_list[port_id],
                pvlan_port_arry + port_current_idx,
                CTC_MAX_LPORT - port_current_idx,
                &lag_port_member_count),
                ret,
                out);
            port_current_idx += lag_port_member_count;

			vlan_notif.op = CTC_SAI_LAG_NF_NEW_VLAN_MEMBER;
			vlan_notif.vlan_id = vlan_id;
			vlan_notif.vlan_member_port_id = port_list[port_id].port_id;
			ctc_sai_vlan_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_NEW_VLAN_MEMBER, &vlan_notif);
			
            continue;

        }

        ret = SAI_STATUS_INVALID_PARAMETER;
        goto out;
    }

    ret = __ctc_sai_vlan_add_ports_to_vlan(vlan_id, port_current_idx, pvlan_port_arry);
#ifdef GREATBELT
    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
    l2dflt_addr.fid = vlan_id;
    l2dflt_addr.l2mc_grp_id = vlan_id;
    
    for(port_id = 0; port_id < port_count; port_id++)
    {
        if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            
            ctc_sai_port_objectid_to_gport(port_list[port_id].port_id, &gport);
            if(ctc_sai_port_get_lag_id(gport))
            {
                /*if interface is AGG interface, it should load agg if to vlan default entry for GB*/
                continue;
            }
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_add_port_to_default_entry(&l2dflt_addr);
        }
        else if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            ctc_sai_port_objectid_to_gport(port_list[port_id].port_id, &gport);
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_add_port_to_default_entry(&l2dflt_addr);

            /*this logic is used for gb, gb need delete all agg member ports from default vlan entry*/
            port_current_idx = 0;
            lag_port_member_count= 0;

                __convert_lag_port_to_ports(&port_list[port_id],
                pvlan_port_arry + port_current_idx,
                CTC_MAX_LPORT - port_current_idx,
                &lag_port_member_count);

            
            for(i=0; i<lag_port_member_count; i++)
            {
                ctc_sai_port_objectid_to_gport(pvlan_port_arry[i].port_id, &gport);
                l2dflt_addr.member.mem_port = gport;
                ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
            }
                

        }
    }
    
#endif

out:
    if(pvlan_port_arry)
    {
        pvlan_port_arry = NULL;
    }
    return ret;
}

sai_status_t
ctc_sai_vlan_remove_ports_to_vlan(
    _In_  const sai_vlan_id_t   vlan_id,
    _In_  const uint32_t        port_count,
    _In_  const sai_vlan_port_t *port_list)
{
    uint32_t        port_id                 = 0;
    sai_status_t    ret                     = SAI_STATUS_SUCCESS;
    uint32_t        lag_port_member_count   = 0;
    uint32_t        port_current_idx        = 0;
    sai_vlan_port_t *pvlan_port_arry        = NULL;
    sai_vlan_port_t vlan_port_arry[CTC_MAX_LPORT];
    stbl_port_t     *p_sdb_port = NULL;
	ctc_sai_vlan_notifier_t	vlan_notif = {0};
#ifdef GREATBELT
    ctc_l2dflt_addr_t l2dflt_addr;
    uint32 gport = 0;
#endif

    sal_memset(vlan_port_arry, 0, sizeof(vlan_port_arry));
    pvlan_port_arry = vlan_port_arry;

    if(NULL == pvlan_port_arry)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }

    for(port_id = 0; port_id < port_count; port_id++)
    {

        p_sdb_port = ctc_sai_port_get_port(port_list[port_id].port_id);
        if (NULL == p_sdb_port)
        {
            continue;
        }
        else
        {
            CTC_BMP_UNSET(p_sdb_port->vlan_bmp, vlan_id);
        }

        if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            CTC_SAI_ERROR_GOTO(
                __convert_lag_port_to_ports(&port_list[port_id],
                pvlan_port_arry + port_current_idx,
                CTC_MAX_LPORT - port_current_idx,
                &lag_port_member_count),
                ret,
                out);

			vlan_notif.op = CTC_SAI_LAG_NF_DEL_VLAN_MEMBER;
			vlan_notif.vlan_id = vlan_id;
			vlan_notif.vlan_member_port_id = port_list[port_id].port_id;
			ctc_sai_vlan_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_DEL_VLAN_MEMBER, &vlan_notif);

            for(port_current_idx = 0; port_current_idx < lag_port_member_count; port_current_idx++)
            {
                 ctc_vlan_remove_port(vlan_id,CTC_SAI_OBJECT_INDEX_GET(pvlan_port_arry[port_id].port_id));
            }
        }
        else
        {
        	vlan_notif.op = CTC_SAI_LAG_NF_DEL_VLAN_MEMBER;
			vlan_notif.vlan_id = vlan_id;
			vlan_notif.vlan_member_port_id = port_list[port_id].port_id;
			ctc_sai_vlan_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_DEL_VLAN_MEMBER, &vlan_notif);
			
            ctc_vlan_remove_port(vlan_id,CTC_SAI_OBJECT_INDEX_GET(port_list[port_id].port_id));
        }        
    }

#ifdef GREATBELT
    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
    l2dflt_addr.fid = vlan_id;
    l2dflt_addr.l2mc_grp_id = vlan_id;
    
    for(port_id = 0; port_id < port_count; port_id++)
    {

        if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            
            ctc_sai_port_objectid_to_gport(port_list[port_id].port_id, &gport);
            if(ctc_sai_port_get_lag_id(gport))
            {
                /*if interface is AGG interface, it should load agg if to vlan default entry for GB*/
                continue;
            }
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
        }
        else if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            ctc_sai_port_objectid_to_gport(port_list[port_id].port_id, &gport);
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
        }
    }
    
#endif
    

out:
    if(pvlan_port_arry)
    {
        pvlan_port_arry = NULL;
    }
    return ret;
}


sai_status_t
__ctc_sai_vlan_remove_ports_to_vlan(
    _In_  const sai_vlan_id_t   vlan_id,
    _In_  const uint32_t        port_count,
    _In_  const sai_vlan_port_t *port_list)
{
    uint32_t    port_id = 0;

    for(port_id = 0; port_id < port_count; port_id++)
    {
        if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            continue;
        }

        if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_list[port_id].port_id))
        {
            continue;
        }

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return __ctc_sai_vlan_remove_ports_to_vlan(vlan_id,port_count,port_list);
}

int32
ctc_sai_vlan_set_igmp_snoop_en(uint16 vlan_id, uint32 enable)
{
    return ctc_vlan_set_igmp_snoop_en(vlan_id, enable);
}

sai_status_t
ctc_sai_vlan_set_igmp_snooping_en(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *pattr)
{
    int32_t    sdk_ret = 0;
    uint32 enable = 0;

    enable = pattr->value.booldata;

    sdk_ret = ctc_sai_vlan_set_igmp_snoop_en(vlan_id, enable);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_vlan_set_igmp_snooping_discard_unknown(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *pattr)
{
    int32_t    sdk_ret = 0;
    uint32 discard = 0;

    discard = pattr->value.booldata;

    sdk_ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN, discard);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_vlan_set_stp_instance(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *pattr)
{
    stbl_stp_t              *p_sdb_stp = NULL;

    p_sdb_stp = ctc_sai_vlan_get_stp(vlan_id);
    if (p_sdb_stp)
    {
        if(p_sdb_stp->stp_oid == pattr->value.oid)
        {
            return SAI_STATUS_SUCCESS;
        }
        ctc_sai_stp_remove_vlan(p_sdb_stp, vlan_id);
    }
    
    p_sdb_stp = ctc_sai_stp_get_by_oid(pattr->value.oid);
    if (!p_sdb_stp)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    ctc_sai_stp_add_vlan(p_sdb_stp, vlan_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_set_attr_dhcpnooping(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *pattr)
{
    uint32 type = 0;

    if (NULL != pattr && SAI_VLAN_ATTR_DHCPSNOOPING == pattr->id)
    {
        type = pattr->value.u32;
    }
    else
    {
        return SAI_STATUS_FAILURE;
    }
    
    ctc_vlan_set_dhcp_excp_type(vlan_id, type);
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_get_stp_instance(
    _In_ sai_vlan_id_t      vlan_id,
    _Out_ sai_attribute_t   *pattr)
{
    stbl_stp_t *p_sdb_stp = NULL;
    
    p_sdb_stp = ctc_sai_vlan_get_stp(vlan_id);
    if (!p_sdb_stp)
    {
        return SAI_STATUS_FAILURE;
    }
    else
    {
        pattr->value.oid = p_sdb_stp->stp_oid;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_set_learn_disable(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ bool                   learn_disable)
{
    return ctc_vlan_set_learning_en(vlan_id, (!learn_disable));
}

sai_status_t
ctc_sai_vlan_set_proto_excep(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ bool                       enable)
{
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    l2dflt_addr.fid = vlan_id;

    if (enable)
    {
        l2dflt_addr.flag = CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    }

    return ctc_l2_set_default_entry_features(&l2dflt_addr);
}


sai_status_t
ctc_sai_vlan_set_attr_learn_disable(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *pattr)
{
    bool    learn_disable = 0;
      
    learn_disable = pattr->value.booldata;
    return ctc_vlan_set_learning_en(vlan_id, (!learn_disable));
}

sai_status_t
ctc_sai_vlan_set_learn_disable_vlan_remove(
    _In_ sai_vlan_id_t          vlan_id)
{
    ctc_sai_vlan_set_learn_disable(vlan_id, 1);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_db_get_vlan_stats(
    _In_    sai_vlan_id_t                   vlan_id,
    _In_    const sai_vlan_stat_counter_t   *counter_ids,
    _In_    uint32_t                        number_of_counters,
    _Out_   uint64_t                        *counters)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_get_learn_disable(
    _In_    sai_vlan_id_t                   vlan_id,
    _Out_   sai_attribute_t                 *p_attr)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t
ctc_sai_vlan_set_ingress_mirror_session(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *attr)
{
    uint32   session_id = 0;
    uint8    sdk_session_id = 0;
    int32_t  sdk_ret = 0;
    bool     enable;
    
    if (0 == attr->value.objlist.count)
    {
        ctc_mirror_get_vlan_info(vlan_id, CTC_INGRESS, &enable, &sdk_session_id);
        sdk_ret = ctc_mirror_set_vlan_en(vlan_id, CTC_INGRESS, FALSE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else if (1 == attr->value.objlist.count)
    {
        session_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
        sdk_session_id = session_id;
        sdk_ret = ctc_mirror_set_vlan_en(vlan_id, CTC_INGRESS, TRUE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }  
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_vlan_get_ingress_mirror_session(
    _In_    sai_vlan_id_t                   vlan_id,
    _Out_   sai_attribute_t                 *attr)
{
    uint8   sdk_session_id = 0 ;
    int32_t sdk_ret = 0;
    bool    enable = FALSE;
    
    if (0 == attr->value.objlist.count) 
    {   
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else
    {   
        sdk_ret = ctc_mirror_get_vlan_info(vlan_id, CTC_INGRESS, &enable, &sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
        if (enable)
        {
            attr->value.objlist.count = 1;
            attr->value.objlist.list[0] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, sdk_session_id);
        }
        else
        {
            attr->value.objlist.count = 0;
            attr->value.objlist.list = NULL;
        }
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);   
}

sai_status_t
ctc_sai_vlan_set_egress_mirror_session(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *attr)
{
    uint32   session_id = 0;
    uint8    sdk_session_id = 0;
    int32_t  sdk_ret = 0;
    bool     enable;
    
    if (0 == attr->value.objlist.count)
    {
        ctc_mirror_get_vlan_info(vlan_id, CTC_EGRESS, &enable, &sdk_session_id);
        sdk_ret = ctc_mirror_set_vlan_en(vlan_id, CTC_EGRESS, FALSE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
       
    }
    else if (1 == attr->value.objlist.count)
    {
        session_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
        sdk_session_id = session_id;
        sdk_ret = ctc_mirror_set_vlan_en(vlan_id, CTC_EGRESS, TRUE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);    
}

sai_status_t
ctc_sai_vlan_set_attr_arpnooping(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *attr)
{
    uint32 type = 0;
    int32_t sdk_ret = 0;

    if (NULL != attr && SAI_VLAN_ATTR_ARPSNOOPING == attr->id)
    {
        type = attr->value.u32;
    }
    else
    {
        return SAI_STATUS_FAILURE;
    }
    sdk_ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_ARP_EXCP_TYPE, type);
    if (sdk_ret < 0)
    {
        return sdk_ret;
    }   
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlan_set_stats_en(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ const sai_attribute_t  *attr)
{
    uint16 fid_id = 0;
    uint16 l3if_id = 0;
    uint32 enable = 0;
    sai_status_t  ret = SAI_STATUS_SUCCESS;
    uint32_t* u32list = NULL;
    uint32 l3if_exist = 0;
    uint32 stats_id_l2_in = 0;
    uint32 stats_id_l2_out = 0;
    uint32 stats_id_l3_in = 0;
    uint32 stats_id_l3_out = 0;
    stbl_vlan_key_t     vlan_key;
    ctc_stats_statsid_t statsid;
    stbl_vlan_t* p_sdb_vlan = NULL;
    stbl_rif_t* p_sdb_rif = NULL;
    stbl_rif_key_t p_rif_key;

    u32list = attr->value.u32list.list;
    enable = u32list[0];
    l3if_id = u32list[1];
    fid_id = vlan_id;
    vlan_key.vid = vlan_id;
    p_rif_key.rif_id = l3if_id;
    p_sdb_rif = stbl_rif_get_rif(&p_rif_key);
    if (p_sdb_rif)
    {
        l3if_exist = 1;
    }
    
    p_sdb_vlan = stbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_sdb_vlan)
    {
        return SAI_STATUS_FAILURE;
    }

    if (enable)
    {
        /* process of l2 statistics*/
        sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
        statsid.type = CTC_STATS_STATSID_TYPE_FID;
        statsid.dir = CTC_INGRESS;
        CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&statsid), ret, out);
        stats_id_l2_in = statsid.stats_id;
        p_sdb_vlan->stats_id_l2_in = statsid.stats_id;

        sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
        statsid.type = CTC_STATS_STATSID_TYPE_FID;
        statsid.dir = CTC_EGRESS;
        CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&statsid), ret, out);
        stats_id_l2_out = statsid.stats_id;
        p_sdb_vlan->stats_id_l2_out= statsid.stats_id;
        
#if defined(USW)
        CTC_SAI_ERROR_GOTO(ctc_vlan_set_direction_property(fid_id, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, CTC_INGRESS, stats_id_l2_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_vlan_set_direction_property(fid_id, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, CTC_EGRESS, stats_id_l2_out), ret, out);
#else
        CTC_SAI_ERROR_GOTO(ctc_l2_set_fid_property(fid_id, CTC_L2_FID_PROP_IGS_STATS_EN, stats_id_l2_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_l2_set_fid_property(fid_id, CTC_L2_FID_PROP_EGS_STATS_EN, stats_id_l2_out), ret, out);
#endif

        /* process of l3 statistics*/
        sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
        statsid.type = CTC_STATS_STATSID_TYPE_L3IF;
        statsid.dir = CTC_INGRESS;
        CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&statsid), ret, out);
        stats_id_l3_in = statsid.stats_id;
        p_sdb_vlan->stats_id_l3_in = statsid.stats_id;

        sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
        statsid.type = CTC_STATS_STATSID_TYPE_L3IF;
        statsid.dir = CTC_EGRESS;
        CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&statsid), ret, out);
        stats_id_l3_out = statsid.stats_id;
        p_sdb_vlan->stats_id_l3_out = statsid.stats_id;

        if (l3if_exist)
        {
            CTC_SAI_ERROR_GOTO(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_STATS, stats_id_l3_in), ret, out);
            CTC_SAI_ERROR_GOTO(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_EGS_STATS, stats_id_l3_out), ret, out);
        }
    }
    else
    {
#if defined(USW)
        CTC_SAI_ERROR_GOTO(ctc_vlan_set_direction_property(fid_id, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, CTC_INGRESS, FALSE), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_vlan_set_direction_property(fid_id, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, CTC_EGRESS, FALSE), ret, out);
#else
        CTC_SAI_ERROR_GOTO(ctc_l2_set_fid_property(fid_id, CTC_L2_FID_PROP_IGS_STATS_EN, FALSE), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_l2_set_fid_property(fid_id, CTC_L2_FID_PROP_EGS_STATS_EN, FALSE), ret, out);
#endif
        
        CTC_SAI_ERROR_GOTO(ctc_stats_destroy_statsid(p_sdb_vlan->stats_id_l2_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_stats_destroy_statsid(p_sdb_vlan->stats_id_l2_out), ret, out);
        
        if (l3if_exist)
        {
            CTC_SAI_ERROR_GOTO(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_STATS, FALSE), ret, out);
            CTC_SAI_ERROR_GOTO(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_EGS_STATS, FALSE), ret, out);
            //p_sdb_vlan->stats_id_l3_in = 0;
            //p_sdb_vlan->stats_id_l3_out = 0;                
        }
        CTC_SAI_ERROR_GOTO(ctc_stats_destroy_statsid(p_sdb_vlan->stats_id_l3_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_stats_destroy_statsid(p_sdb_vlan->stats_id_l3_out), ret, out);
    }

    CTC_SAI_DEBUG("Set vlan stats info:vlan_id = %u, l3if_id = %u, enable = %u, stats_id_l2_in = %u, "
        "stats_id_l2_out = %u, stats_id_l3_in = %u, stats_id_l3_out = %u", vlan_id, l3if_id, enable, 
        stats_id_l2_in, stats_id_l2_out, stats_id_l3_in, stats_id_l3_out);

out:
    return ret;
    
}

sai_status_t
ctc_sai_vlan_get_egress_mirror_session(
    _In_    sai_vlan_id_t                   vlan_id,
    _Out_   sai_attribute_t                 *attr)
{
    uint8   sdk_session_id = 0 ;
    int32_t sdk_ret = 0;
    bool    enable = FALSE;
    
    if (0 == attr->value.objlist.count) 
    {   
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else
    {   
        sdk_ret = ctc_mirror_get_vlan_info(vlan_id, CTC_EGRESS, &enable, &sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
        if (enable)
        {
            attr->value.objlist.count = 1;
            attr->value.objlist.list[0] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, sdk_session_id);
        }
        else
        {
            attr->value.objlist.count = 0;
            attr->value.objlist.list = NULL;
        }
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);   
}


#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_set_vlan_attribute_debug_param(
    _In_ sai_vlan_id_t vlan_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG("in:vlan_id %u", vlan_id);
    switch(attr->id)
    {
    case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
        CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
        break;
    case SAI_VLAN_ATTR_STP_INSTANCE:
        CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_STP_INSTANCE 0x%llx", attr->value.oid);
        break;
    case SAI_VLAN_ATTR_LEARN_DISABLE:
        CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_LEARN_DISABLE %u", attr->value.booldata);
        break;  
    default: /*not support other field */
        CTC_SAI_DEBUG("in:NOT SUPPORT");
        break;
    }
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_vlan_attribute_debug_param(
    _In_ sai_vlan_id_t vlan_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    CTC_SAI_DEBUG("in:vlan_id lag_id %u", vlan_id);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_vlan_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return SAI_STATUS_SUCCESS;
        }

        switch(attr->id)
        {
        case SAI_VLAN_ATTR_PORT_LIST:
            CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_PORT_LIST");
            break;
            
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
            break;
        case SAI_VLAN_ATTR_STP_INSTANCE:
            CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_STP_INSTANCE 0x%llx", attr->value.oid);
            break;

        case SAI_VLAN_ATTR_LEARN_DISABLE:
            CTC_SAI_DEBUG("in:SAI_VLAN_ATTR_LEARN_DISABLE %u", attr->value.booldata);
            break;

        case SAI_VLAN_ATTR_META_DATA:
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC
/*
* Routine Description:
*    Create a VLAN
*
* Arguments:
*    [in] vlan_id - VLAN id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_vlan(
    _In_ sai_vlan_id_t vlan_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;
     
    sal_memset(&attr,0,sizeof(sai_attribute_t));

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id %u", vlan_id);
    CTC_SAI_ERROR_GOTO(ctc_sai_vlan_create_vlan(vlan_id),ret,out);

    ret = ctc_sai_vlan_db_alloc(vlan_id);

    /* set default vlan attribute */
    ctc_sai_vlan_set_learn_disable(vlan_id, FALSE);

#ifdef USW
    /* BUG53979 default enable pim_snooping on vlan */
    ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_PIM_SNOOP_EN, TRUE);
#endif
    /* set default */
    attr.id        = SAI_VLAN_ATTR_STP_INSTANCE;
    attr.value.oid = p_sai_glb->default_stp_id;
    CTC_SAI_ERROR_GOTO(ctc_sai_vlan_set_stp_instance(vlan_id,&attr),ret,out1);

out:
    return ret;

out1:
    ctc_sai_vlan_remove_vlan(vlan_id);
    goto out;
}

/*
* Routine Description:
*    Remove a VLAN
*
* Arguments:
*    [in] vlan_id - VLAN id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_vlan(
    _In_ sai_vlan_id_t vlan_id)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id %u", vlan_id);
    CTC_SAI_ERROR_GOTO(ctc_sai_vlan_remove_vlan(vlan_id),ret,out);

    /* set IGMP Snooping disable */
    ctc_sai_vlan_set_igmp_snoop_en(vlan_id, FALSE);

    /* update vlan attribute when remove vlan */
    ctc_sai_vlan_set_learn_disable_vlan_remove(vlan_id);
out:
    return ret;
}

/*
* Routine Description:
*    Set VLAN attribute Value
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_vlan_attribute(
    _In_ sai_vlan_id_t vlan_id,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(
                            g_sai_vlan_attr_entries,attr),
                       ret,out);
    ctc_sai_set_vlan_attribute_debug_param(vlan_id, attr);
    switch(attr->id)
    {
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            ret = ctc_sai_vlan_set_max_learned_address(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_STP_INSTANCE:
            ret = ctc_sai_vlan_set_stp_instance(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_LEARN_DISABLE:
            ret = ctc_sai_vlan_set_attr_learn_disable(vlan_id, attr);
            break;

        case SAI_VLAN_ATTR_DHCPSNOOPING:
            ret = ctc_sai_vlan_set_attr_dhcpnooping(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_META_DATA:
            break;

        case SAI_VLAN_ATTR_IGMP_SNOOPING_EN:
            ret = ctc_sai_vlan_set_igmp_snooping_en(vlan_id, attr);
            break;

        case SAI_VLAN_ATTR_IGMP_SNOOPING_DISCARD_UNKNOWN:
            ret = ctc_sai_vlan_set_igmp_snooping_discard_unknown(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_INGRESS_MIRROR_SESSION:
            ret = ctc_sai_vlan_set_ingress_mirror_session(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_EGRESS_MIRROR_SESSION:
            ret = ctc_sai_vlan_set_egress_mirror_session(vlan_id, attr);
            break;

        case SAI_VLAN_ATTR_ARPSNOOPING:
            ret = ctc_sai_vlan_set_attr_arpnooping(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_STATS_EN:
            ret = ctc_sai_vlan_set_stats_en(vlan_id, attr);
            break;
    }

out:
    return ret;
}

/*
* Routine Description:
*    Get VLAN attribute Value
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_vlan_attribute(
    _In_ sai_vlan_id_t vlan_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t* attr        = NULL;
    uint32_t        attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_vlan_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_VLAN_ATTR_PORT_LIST:
            ret = ctc_sai_vlan_get_vlan_port_list(vlan_id, attr_list);
            break;
            
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            ret = ctc_sai_vlan_get_max_learned_address(vlan_id, attr);
            break;
        case SAI_VLAN_ATTR_STP_INSTANCE:
            ret = ctc_sai_vlan_get_stp_instance(vlan_id, attr);
            break;

        case SAI_VLAN_ATTR_LEARN_DISABLE:
            ret = ctc_sai_vlan_get_learn_disable(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_INGRESS_MIRROR_SESSION:
            ret = ctc_sai_vlan_get_ingress_mirror_session(vlan_id, attr);
            break;
            
        case SAI_VLAN_ATTR_EGRESS_MIRROR_SESSION:
            ret = ctc_sai_vlan_get_egress_mirror_session(vlan_id, attr);
            break;  
            
        case SAI_VLAN_ATTR_META_DATA:
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    ctc_sai_get_vlan_attribute_debug_param(vlan_id, attr_count, attr_list);
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove VLAN configuration (remove all VLANs).
*
* Arguments:
*    None
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_all_vlans(void)
{
    sai_vlan_id_t vlan_id = 0;

    CTC_SAI_DEBUG_FUNC();

    for(vlan_id = 1; vlan_id < 4096; vlan_id++)
    {
        ctc_sai_remove_vlan(vlan_id);
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Add Port to VLAN
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] port_count - number of ports
*    [in] port_list - pointer to membership structures
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_add_ports_to_vlan(
    _In_ sai_vlan_id_t vlan_id,
    _In_ uint32_t port_count,
    _In_ const sai_vlan_port_t* port_list)
{
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id %u port_count %u", vlan_id, port_count);
    CTC_SAI_PTR_VALID_CHECK(port_list);

    return ctc_sai_vlan_add_ports_to_vlan(vlan_id,port_count,port_list);
}

/*
* Routine Description:
*    Remove Port from VLAN
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] port_count - number of ports
*    [in] port_list - pointer to membership structures
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_ports_from_vlan(
    _In_ sai_vlan_id_t vlan_id,
    _In_ uint32_t port_count,
    _In_ const sai_vlan_port_t* port_list)
{
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id %u port_count %u", vlan_id, port_count);
    CTC_SAI_PTR_VALID_CHECK(port_list);

    return ctc_sai_vlan_remove_ports_to_vlan(vlan_id,port_count,port_list);
}

/*
* Routine Description:
*   Get vlan statistics counters.
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] counter_ids - specifies the array of counter ids
*    [in] number_of_counters - number of counters in the array
*    [out] counters - array of resulting counter values.
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_vlan_stats(
    _In_ sai_vlan_id_t vlan_id,
    _In_ const sai_vlan_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters)
{
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id %u port_count %u number_of_counters %u", 
        vlan_id, number_of_counters);
    CTC_SAI_PTR_VALID_CHECK(counter_ids);
    CTC_SAI_PTR_VALID_CHECK(counters);

    return ctc_sai_vlan_db_get_vlan_stats(vlan_id, counter_ids, number_of_counters, counters);
}

sai_status_t
ctc_sai_get_vlan_stats_info(
    _In_ sai_vlan_id_t vlan_id,
    _Out_ sai_attribute_t *attr
    )
{
    stbl_vlan_t* p_sdb_vlan = NULL;
    stbl_vlan_key_t p_vlan_key;
    sai_status_t  ret = SAI_STATUS_SUCCESS;
    ctc_stats_basic_t l2_in_stats, l2_out_stats, l3_in_stats, l3_out_stats;

    sal_memset(&l2_in_stats, 0, sizeof(ctc_stats_basic_t));
    sal_memset(&l2_out_stats, 0, sizeof(ctc_stats_basic_t));
    sal_memset(&l3_in_stats, 0, sizeof(ctc_stats_basic_t));
    sal_memset(&l3_out_stats, 0, sizeof(ctc_stats_basic_t));

    p_vlan_key.vid = vlan_id;
    p_sdb_vlan = stbl_vlan_get_vlan(&p_vlan_key);
    if (NULL == p_sdb_vlan)
    {
        return SAI_STATUS_FAILURE;
    }
    
    CTC_SAI_ERROR_GOTO(ctc_stats_get_stats(p_sdb_vlan->stats_id_l2_in, &l2_in_stats), ret, out);
    attr[0].value.u64 = l2_in_stats.packet_count;
    attr[1].value.u64 = l2_in_stats.byte_count;

    CTC_SAI_ERROR_GOTO(ctc_stats_get_stats(p_sdb_vlan->stats_id_l2_out, &l2_out_stats), ret, out);
    attr[2].value.u64 = l2_out_stats.packet_count;
    attr[3].value.u64 = l2_out_stats.byte_count;

    CTC_SAI_ERROR_GOTO(ctc_stats_get_stats(p_sdb_vlan->stats_id_l3_in, &l3_in_stats), ret, out);
    attr[4].value.u64 = l3_in_stats.packet_count;
    attr[5].value.u64 = l3_in_stats.byte_count;

    CTC_SAI_ERROR_GOTO(ctc_stats_get_stats(p_sdb_vlan->stats_id_l3_out, &l3_out_stats), ret, out);
    attr[6].value.u64 = l3_out_stats.packet_count;
    attr[7].value.u64 = l3_out_stats.byte_count;
    
out:    
    return ret;
}
sai_status_t
ctc_sai_clear_vlan_stats(
    _In_ sai_vlan_id_t vlan_id)
{
    stbl_vlan_t* p_sdb_vlan = NULL;
    stbl_vlan_key_t p_vlan_key;
    sai_status_t  ret = SAI_STATUS_SUCCESS;

    p_vlan_key.vid = vlan_id;
    p_sdb_vlan = stbl_vlan_get_vlan(&p_vlan_key);

    if (p_sdb_vlan)
    {
        CTC_SAI_ERROR_GOTO(ctc_stats_clear_stats(p_sdb_vlan->stats_id_l2_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_stats_clear_stats(p_sdb_vlan->stats_id_l2_out), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_stats_clear_stats(p_sdb_vlan->stats_id_l3_in), ret, out);
        CTC_SAI_ERROR_GOTO(ctc_stats_clear_stats(p_sdb_vlan->stats_id_l3_out), ret, out);
    }

out:
    return ret;
}
#define ________SAI_VLAN_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    preg->init_status              = INITIALIZED;

out:
    return ret;

    goto out;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_vlan_api_t      g_sai_api_func = {
    .create_vlan            = ctc_sai_create_vlan,
    .remove_vlan            = ctc_sai_remove_vlan,
    .set_vlan_attribute     = ctc_sai_set_vlan_attribute,
    .get_vlan_attribute     = ctc_sai_get_vlan_attribute,
    .add_ports_to_vlan      = ctc_sai_add_ports_to_vlan,
    .remove_ports_from_vlan = ctc_sai_remove_ports_from_vlan,
    .remove_all_vlans       = ctc_sai_remove_all_vlans,
    .get_vlan_stats         = ctc_sai_get_vlan_stats,
    .get_vlan_stats_info         = ctc_sai_get_vlan_stats_info,
    .clear_vlan_stats         = ctc_sai_clear_vlan_stats,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_VLAN,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_FDB_OUTER_FUNC
sai_status_t
ctc_sai_vlan_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}


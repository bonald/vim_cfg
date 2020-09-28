#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include "ctc_sai_router.h"
#include "ctc_sai_port.h"
#include <ctc_sai_routerintf.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_vlan.h>

static ctc_sai_check_object_type_range_t   g_vr_obj =
{
    .min = SAI_OBJECT_TYPE_VIRTUAL_ROUTER,
    .max = SAI_OBJECT_TYPE_VIRTUAL_ROUTER,
};

static ctc_sai_check_u32_range_t g_intf_attr_type =
{
    .min = SAI_ROUTER_INTERFACE_TYPE_PORT,
    .max = SAI_ROUTER_INTERFACE_TYPE_VLAN,
};

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_object_type_range_fn,
            .func_parameter = &g_vr_obj,
        }
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_TYPE,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_intf_attr_type,
        }
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_PORT_ID,
        .type   = SAI_ATTR_FLAG_UNKONW | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID,
        .type   = SAI_ATTR_FLAG_UNKONW | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = true,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = true,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = true,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_MTU,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.u32 = 1514,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
        .default_value.u8 = 0,
    },  
    {
        .id     = SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
        .default_value.u8 = 0,
    },  
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#ifdef USW
extern sai_status_t 
ctc_sai_qos_map_l3if_set_ctc_qos_map(uint32_t  l3if_id, const sai_attribute_t *attr);
#endif

#define ________SAI_SAI_INNER_API_FUNC
sai_status_t
ctc_routerintf_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    sai_status_t ret        = SAI_STATUS_SUCCESS;

#ifdef OFPRODUCT
    p_sai_glb->rif_max_count = MAX_CTC_L3IF_ID - 3; /*1022 is reserved for mpls en, 1021 is reserved for ipuc with l2 port, 1022 is reserved for section oam */
#else
    p_sai_glb->rif_max_count = MAX_CTC_L3IF_ID;
#endif

    CTC_SAI_ERROR_GOTO(
        ctclib_opb_create(&p_port_glb->route_if_opb, 2, p_sai_glb->rif_max_count, "l3if_id"),
        ret,out);

out:
    return ret;
}

int32
ctc_routerintf_alloc_l3if_id(uint32_t *l3if_id)
{
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    return ctclib_opb_alloc_offset(&p_port_glb->route_if_opb, l3if_id);
}

int32
ctc_routerintf_release_l3if_id(uint32_t l3if_id)
{
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    return ctclib_opb_free_offset(&p_port_glb->route_if_opb, l3if_id);
}

sai_status_t
ctc_routerintf_alloc(
    _Out_ stbl_rif_t** pp_sdb_rif, uint32_t wb_index)
{
#ifdef WB_TODO
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
#endif
    stbl_rif_t                  rif;
    stbl_rif_t                  *p_sdb_rif = NULL;
    uint32_t                    index       = 0;

#ifdef WB_TODO
    if (p_master->used_cnt >= p_sai_glb->rif_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }
#endif

    sal_memset(&rif, 0, sizeof(rif));
    ctc_routerintf_alloc_l3if_id(&index);
    rif.key.rif_id = index;
    stbl_rif_add_rif(&rif);

    p_sdb_rif = stbl_rif_get_rif(&rif.key);
    if (NULL == p_sdb_rif)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    
    *pp_sdb_rif = p_sdb_rif;
    return SAI_STATUS_SUCCESS;
}

void
ctc_routerintf_release(
    _In_  stbl_rif_t *p_sdb_rif)
{
    if (NULL == p_sdb_rif)
    {
        return;
    }

    ctc_routerintf_release_l3if_id(p_sdb_rif->key.rif_id);
    stbl_rif_del_rif(&p_sdb_rif->key);
}

stbl_rif_t*
ctc_routerintf_get_by_oid_no_ref(
	_In_  const sai_object_id_t rif_oid)
{
    stbl_rif_key_t key;

    key.rif_id = CTC_SAI_OBJECT_INDEX_GET(rif_oid);
    return stbl_rif_get_rif(&key);
}

stbl_rif_t*
ctc_routerintf_get_by_oid(
	_In_  const sai_object_id_t rif_id)
{
    return ctc_routerintf_get_by_oid_no_ref(rif_id);
}

int32
__ctc_vec_traversal_by_vlan(void *obj, stbl_iter_args_t *pargs)
{
    stbl_rif_t *p_sdb_rif = (stbl_rif_t*)obj;
    sai_vlan_id_t *vlan_id = (sai_vlan_id_t*)pargs->argv[0];
    stbl_rif_t **pp_sdb_rif = (stbl_rif_t**)pargs->argv[1];

    if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        if (p_sdb_rif->vlan_id == *vlan_id)
        {
            *pp_sdb_rif = p_sdb_rif;
        }
    }

    return 0;
}

stbl_rif_t*
ctc_routerintf_get_by_vlan(
	_In_  sai_vlan_id_t vlan_id)
{
    stbl_rif_t* p_sdb_rif = NULL;
    stbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &vlan_id;
    args.argv[1] = &p_sdb_rif;

    stbl_rif_iterate(__ctc_vec_traversal_by_vlan, &args);
    return p_sdb_rif;
}

int32
__ctc_vec_traversal_by_port_oid(void *obj, stbl_iter_args_t *pargs)
{
    stbl_rif_t *p_sdb_rif = (stbl_rif_t*)obj;
    sai_object_id_t *port_oid = (sai_object_id_t*)pargs->argv[0];
    stbl_rif_t **pp_sdb_rif = (stbl_rif_t**)pargs->argv[1];

    if(SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        if (p_sdb_rif->port_oid == *port_oid)
        {
            *pp_sdb_rif = p_sdb_rif;
        }
    }

    return 0;
}

stbl_rif_t*
ctc_routerintf_get_by_port_oid(
	_In_  sai_object_id_t port_oid)
{
    stbl_rif_t* p_sdb_rif = NULL;
    stbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &port_oid;
    args.argv[1] = &p_sdb_rif;

    stbl_rif_iterate(__ctc_vec_traversal_by_port_oid, &args);
    return p_sdb_rif;
}

sai_status_t
__ctc_routerintf_create_l3if(
	_In_  stbl_rif_t* p_sdb_rif)
{
    int32_t     sdk_ret = 0;
    ctc_l3if_t  l3if;
    int32_t  i = 0;
    stbl_vlan_key_t vlan_key;
    uint16_t l3if_id = 0;
    stbl_vlan_t* p_sdb_vlan = NULL;
    uint32 gport[64] = {0};
#ifdef USW
    uint16 cnt = 0;
#else
    uint8 cnt = 0;
#endif
    uint8 tid  = 0;


    sal_memset(&l3if,0,sizeof(ctc_l3if_t));

    if(SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &l3if.gport);
    }
    else if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
        l3if.vlan_id = p_sdb_rif->vlan_id;
    }
    else
    {
        return sdk_ret;
    }

    SAI_CALL_CTC(ctc_l3if_create(
        (uint16_t)p_sdb_rif->key.rif_id,
                &l3if), sdk_ret);

    if(SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        /*modified by hansf, default trust cos 2017/09/13*/
        //ctc_port_set_property(l3if.gport, CTC_PORT_PROP_QOS_POLICY, CTC_QOS_TRUST_DSCP);
#if 0 /* SYSTEM MODIFIED: fix qos bug 40267 add by wangqj */
        ctc_port_set_property(l3if.gport, CTC_PORT_PROP_REPLACE_DSCP_EN, 0);
        ctc_port_set_property(l3if.gport, CTC_PORT_PROP_REPLACE_STAG_COS, 0);
#endif
        if (CTC_IS_LINKAGG_PORT(l3if.gport))
        {
            tid = CTC_SAI_OBJECT_INDEX_GET(p_sdb_rif->port_oid);
            ctc_linkagg_get_member_ports(tid,gport, &cnt);
            for(i=0; i<cnt; i++)
            {
                SAI_CALL_CTC(ctc_port_set_phy_if_en(gport[i],TRUE), sdk_ret);
                SAI_CALL_CTC(ctc_port_set_property(gport[i], CTC_PORT_PROP_BRIDGE_EN, 1), sdk_ret);

                /*modified by hansf, default trust cos 2017/09/13*/
                //ctc_port_set_property(gport[i], CTC_PORT_PROP_QOS_POLICY, CTC_QOS_TRUST_DSCP);
#if 0 /* SYSTEM MODIFIED: fix qos bug 40267 add by wangqj */
                ctc_port_set_property(gport[i], CTC_PORT_PROP_REPLACE_DSCP_EN, 0);
                ctc_port_set_property(gport[i], CTC_PORT_PROP_REPLACE_STAG_COS, 0);
#endif
                ctc_sai_port_set_port_flag(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT,gport[i]),SAI_PORT_FLAG_ROUTED_EN, TRUE);
            }
            
            goto out;
        }
        else
        {
            SAI_CALL_CTC(ctc_port_set_phy_if_en(l3if.gport,TRUE), sdk_ret);        
            SAI_CALL_CTC(ctc_port_set_property(l3if.gport, CTC_PORT_PROP_BRIDGE_EN, 1), sdk_ret);
        }

        ctc_stp_clear_all_inst_state(l3if.gport);
    }
    else if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
#if 0
        /* Delete for should set based on IGMP Snooping enable config */
        ctc_l3if_set_property((uint16_t)CTC_SAI_OBJECT_INDEX_GET(prouterintf_entry->rif_id), CTC_L3IF_PROP_IGMP_SNOOPING_EN, TRUE);    
#endif

        vlan_key.vid = p_sdb_rif->vlan_id;
        p_sdb_vlan = stbl_vlan_get_vlan(&vlan_key);
        l3if_id = (uint16)(p_sdb_rif->key.rif_id);
        
        if ((0 != p_sdb_vlan->stats_id_l3_in) && (0 != p_sdb_vlan->stats_id_l3_out))
        {
            SAI_CALL_CTC(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_STATS, p_sdb_vlan->stats_id_l3_in), sdk_ret);
            SAI_CALL_CTC(ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_EGS_STATS, p_sdb_vlan->stats_id_l3_out), sdk_ret);
        }
        
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_add_entry(
	_In_ stbl_rif_t* p_sdb_rif)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = __ctc_routerintf_create_l3if(p_sdb_rif);
    if (ret < 0)
    {
        goto out;
    }
    
    ret = ctc_vr_add_router_intf(p_sdb_rif->vr_oid, p_sdb_rif);
    if (ret < 0)
    {
        goto out;
    }
out:
    return ret;
}

sai_status_t
__ctc_routerintf_destory_l3if(
	_In_  stbl_rif_t *p_sdb_rif)
{
    int32_t     sdk_ret = 0;
    ctc_l3if_t  l3if;
    int32_t  i = 0;
    int32_t ret = 0;
    uint16_t l3if_id = 0;
    uint32 gport[64] = {0};
    stbl_vlan_t* p_sdb_vlan = NULL;
    stbl_vlan_key_t vlan_key;
#ifdef USW
    uint16 cnt = 0;
#else
    uint8 cnt = 0;
#endif
    uint8 tid = 0;

    sal_memset(&l3if,0,sizeof(l3if));

    if (SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        /* modified by wangjj for fix lag type gport differ from creating, 2016-04-06 */
        ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &l3if.gport);
    }
    else if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
        l3if.vlan_id = p_sdb_rif->vlan_id;
    }
    else
    {
        return sdk_ret;
    }

    if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
#if 0
        /* Delete for should set based on IGMP Snooping enable config */
        ctc_l3if_set_property((uint16_t)CTC_SAI_OBJECT_INDEX_GET(prouterintf_entry->rif_id), CTC_L3IF_PROP_IGMP_SNOOPING_EN, FALSE);
#endif

        vlan_key.vid = p_sdb_rif->vlan_id;
        p_sdb_vlan = stbl_vlan_get_vlan(&vlan_key);
        if ((0 != p_sdb_vlan->stats_id_l3_in) && (0 != p_sdb_vlan->stats_id_l3_out))
        {
            l3if_id = (uint16_t)(p_sdb_rif->key.rif_id);
            ret += ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_STATS, FALSE);
            ret += ctc_l3if_set_property(l3if_id, CTC_L3IF_PROP_EGS_STATS, FALSE);
            if (ret < 0)
            {
                return ret;
            }
        }
    }
    
    sdk_ret = ctc_l3if_destory(
        (uint16_t)p_sdb_rif->key.rif_id, &l3if);

    if (SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        ctc_port_set_phy_if_en(l3if.gport,FALSE);
        /* mod by hansf for bug 47548, destory l3if should not change port trust type */
        /* ctc_port_set_property(l3if.gport, CTC_PORT_PROP_QOS_POLICY, CTC_QOS_TRUST_STAG_COS); */
#if 0 /* SYSTEM MODIFIED: fix qos bug 40267 add by wangqj */
        ctc_port_set_property(l3if.gport, CTC_PORT_PROP_REPLACE_STAG_COS, 0);
        ctc_port_set_property(l3if.gport, CTC_PORT_PROP_REPLACE_DSCP_EN, 0);
#endif
        if (CTC_IS_LINKAGG_PORT(l3if.gport))
        {
            tid = CTC_SAI_OBJECT_INDEX_GET(p_sdb_rif->port_oid);
            ctc_linkagg_get_member_ports(tid,gport, &cnt);
            for(i=0; i<cnt; i++)
            {
                ctc_port_set_phy_if_en(gport[i],FALSE);
                /* mod by hansf for bug 47548, destory l3if should not change port trust type */
                /* ctc_port_set_property(gport[i], CTC_PORT_PROP_QOS_POLICY, CTC_QOS_TRUST_STAG_COS); */
#if 0 /* SYSTEM MODIFIED: fix qos bug 40267 add by wangqj */
                ctc_port_set_property(gport[i], CTC_PORT_PROP_REPLACE_STAG_COS, 0);
                ctc_port_set_property(gport[i], CTC_PORT_PROP_REPLACE_DSCP_EN, 0);
#endif
                ctc_sai_port_set_port_flag(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT,gport[i]),SAI_PORT_FLAG_ROUTED_EN, FALSE);
            }
            
        }
    }
    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_remove_entry(
	_In_  stbl_rif_t *p_sdb_rif)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(__ctc_routerintf_destory_l3if(p_sdb_rif),ret,out);
    ctc_vr_remove_router_intf(p_sdb_rif->vr_oid, p_sdb_rif);
out:
    return ret;
}

sai_status_t
ctc_routerintf_update_src_mac(
	_In_  const stbl_rif_t                  *p_sdb_rif,
	_In_  const uint8_t 					*pmac)
{
    int32_t     sdk_ret = 0;
    ctc_l3if_router_mac_t   smac;

    sal_memset(&smac, 0, sizeof(smac));

    sal_memcpy(smac.mac[0], pmac, sizeof(sai_mac_t));
    smac.num = 1;

    sdk_ret = ctc_l3if_set_interface_router_mac((uint16_t)(p_sdb_rif->key.rif_id),
                smac);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_src_mac(
	stbl_rif_t                  *p_sdb_rif,
	const sai_attribute_t       *attr)
{
    int32_t     sdk_ret = 0;

    sdk_ret = ctc_routerintf_update_src_mac(p_sdb_rif,
            (uint8_t*)attr->value.mac);

    sal_memcpy(p_sdb_rif->smac, attr->value.mac, sizeof(mac_addr_t));

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_vmac(
	stbl_rif_t                  *p_sdb_rif,
	const sai_attribute_t       *attr)
{
    int32_t                 sdk_ret = 0;
    ctc_l3if_router_mac_t   smac;
    sai_object_id_t         vr_id = 0;
    stbl_vr_t              *p_sdb_vr = NULL;
#ifdef GREATBELT
    ctc_l3if_vmac_t         l3if_vmac;
    uint32_t                index = 0;
#endif   

    sal_memset(&smac,0,sizeof(ctc_l3if_router_mac_t));
#ifdef GREATBELT
    sal_memset(&l3if_vmac, 0, sizeof(ctc_l3if_vmac_t));    
#endif   

    sal_memcpy(&smac, &attr->value.vrrp, sizeof(ctc_l3if_router_mac_t));

#ifdef GREATBELT
    /* remove all vmac */
    for (index = 0; index < 3; index++)
    {
        l3if_vmac.low_8bits_mac_index = index;
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0;
    
        ctc_l3if_remove_vmac_low_8bit((uint16_t)(p_sdb_rif->key.rif_id), &l3if_vmac);
    }
    
    l3if_vmac.low_8bits_mac_index = 3;
    l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_RSV_ROUTER_MAC;
    
    ctc_l3if_remove_vmac_low_8bit((uint16_t)(p_sdb_rif->key.rif_id), &l3if_vmac);

    /* if there is vmac, should add router mac */
    if (smac.num > 1)
    {
        l3if_vmac.low_8bits_mac_index = 3;
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_RSV_ROUTER_MAC;
        l3if_vmac.low_8bits_mac = smac.mac[0][5];;

        sdk_ret = ctc_l3if_add_vmac_low_8bit((uint16_t)(p_sdb_rif->key.rif_id), &l3if_vmac);

        if (sdk_ret)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    
    /* add all vmac, smac.mac[1] - smac.mac[3] are vmac */
    for (index = 1; index < smac.num; index++)
    {
        l3if_vmac.low_8bits_mac = smac.mac[index][5];
        l3if_vmac.low_8bits_mac_index = index - 1;
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0;
    
        sdk_ret = ctc_l3if_add_vmac_low_8bit((uint16_t)(p_sdb_rif->key.rif_id), &l3if_vmac);

        if (sdk_ret)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
#else
    sdk_ret = ctc_l3if_set_interface_router_mac((uint16_t)(p_sdb_rif->key.rif_id),
                smac);
#endif

    p_sdb_vr = ctc_vr_get_by_oid(vr_id);
    if (!p_sdb_vr)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);    
    }

#ifdef WB_TODO
    sal_memcpy(p_sdb_vr->smac, prouterintf_entry->smac, sizeof(mac_addr_t));
#endif
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_v4_admin(
	_In_  stbl_rif_t                    *p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
    int32_t    	sdk_ret = 0;

    bool        admin_state = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IPV4_UCAST, admin_state), sdk_ret, out);
    /* modified for IPMC <- Deleted by kcao for case igsp_func_36_forward_with_no_group.tcl, CNOS not support IPMC, enable it will result in IPMC packet do routing process */
#if 0
    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IPV4_MCAST, admin_state), sdk_ret,out);
#endif

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_v4mcast_admin(
	_In_  stbl_rif_t                    *p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
    int32_t    	sdk_ret = 0;

    bool        admin_state = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IPV4_MCAST, admin_state), sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_v6_admin(
	_In_  stbl_rif_t                    *p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
    int32_t    	sdk_ret = 0;

    bool        admin_state = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)CTC_SAI_OBJECT_INDEX_GET(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IPV6_UCAST, admin_state),sdk_ret,out);
    /* Deleted by kcao for case igsp_func_36_forward_with_no_group.tcl, CNOS not support IPMC, enable it will result in IPMC packet do routing process */
#if 0
    CTC_SAI_ERROR_GOTO(
        ctc_l3if_set_property(
            (uint16_t)CTC_SAI_OBJECT_INDEX_GET(prouterintf_entry->rif_id),
                CTC_L3IF_PROP_IPV6_MCAST,admin_state),sdk_ret,out);
#endif

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_igmp_snoop_en(
	_In_  stbl_rif_t                    *p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
    int32_t    	sdk_ret = 0;

    bool        igsp_en = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IGMP_SNOOPING_EN, igsp_en), sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_routerintf_set_mtu(
	stbl_rif_t                  *p_sdb_rif,
	const sai_attribute_t       *attr)
{
    int32_t    sdk_ret = 0;

    SAI_CALL_CTC_ERR_GOTO(
        ctc_l3if_set_property(
            (uint16_t)(p_sdb_rif->key.rif_id),
                CTC_L3IF_PROP_MTU_SIZE,attr->value.u32),sdk_ret,out);

    if (!ctc_sai_get_cut_through_en()) {
        SAI_CALL_CTC_ERR_GOTO(
            ctc_l3if_set_property(
                (uint16_t)(p_sdb_rif->key.rif_id),
                    CTC_L3IF_PROP_MTU_EN,true),sdk_ret,out);
    }

    SAI_CALL_CTC_ERR_GOTO(
        ctc_l3if_set_property(
            (uint16_t)(p_sdb_rif->key.rif_id),
                CTC_L3IF_PROP_MTU_EXCEPTION_EN,true),sdk_ret,out);
out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_urpf(
	stbl_rif_t                  *p_sdb_rif,
	const sai_attribute_t       *attr)
{
    int32_t    sdk_ret = 0;
    uint32     value = CTC_L3IF_IPSA_LKUP_TYPE_NONE;

    /*get ucast sa type*/
    if (attr->value.u32)
        value = CTC_L3IF_IPSA_LKUP_TYPE_RPF;
    else
        value = CTC_L3IF_IPSA_LKUP_TYPE_NONE;

    SAI_CALL_CTC_ERR_GOTO(
        ctc_l3if_set_property(
            (uint16_t)(p_sdb_rif->key.rif_id),
                CTC_L3IF_PROP_IPV4_SA_TYPE, value), sdk_ret, out);
out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_urpf_v6(
	stbl_rif_t                  *p_sdb_rif,
	const sai_attribute_t       *attr)
{
    int32_t    sdk_ret = 0;
    uint32     value = CTC_L3IF_IPSA_LKUP_TYPE_NONE;

    /*get ucast sa type*/
    if (attr->value.u32)
        value = CTC_L3IF_IPSA_LKUP_TYPE_RPF;
    else
        value = CTC_L3IF_IPSA_LKUP_TYPE_NONE;

    SAI_CALL_CTC_ERR_GOTO(
        ctc_l3if_set_property(
            (uint16_t)(p_sdb_rif->key.rif_id),
                CTC_L3IF_PROP_IPV6_SA_TYPE, value), sdk_ret, out);
out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_trust_dscp(
	_In_  stbl_rif_t                    *p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
#ifdef USW
    int32_t    	sdk_ret = 0;

    bool        trust_dscp = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_TRUST_DSCP, trust_dscp), sdk_ret, out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
#else
    return SAI_STATUS_SUCCESS;
#endif
}


/***************************************************************************************************
 * Name         : hsrv_router_interface_set_nat_enable 
 * Purpose      : set router interface nat enable flag
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
extern sai_status_t ctc_sai_set_route_attribute(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ const sai_attribute_t *attr,
    _In_ sai_object_id_t nh_oid);
void
ctc_routerintf_set_nat_enable_refresh_rt(ctclib_hash_backet_t* node, void *para1, void *para2, void *para3)
{
    char str[128];
    stbl_route_t* p_rt = NULL;
    uint32 rif_id      = *(uint32*)para1;
    uint32 enable      = *(uint32*)para2;
    uint32 category_id = *(uint32*)para3;
    uint32 l3iftype    = SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL;
    uint64 rif_oid     = 0;

    CTC_SAI_DEBUG_FUNC();
    switch (enable)
    {
        case SAI_ROUTER_INTERFACE_NAT_INSIDE: /* enable inside */
        case SAI_ROUTER_INTERFACE_NAT_DISABLE: /* disable NAT*/
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL;
            break;
        }
        case SAI_ROUTER_INTERFACE_NAT_OUTSIDE: /* enable outside */
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL;
            break;
        }
        default:
            break;
    }


    rif_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_id);
    p_rt = (stbl_route_t*)node->data;
    CTC_SAI_DEBUG("set l3iftype refresh rt, rif_oid:%llx, node_rif_oid:%llx, enable:%u, category_id:%u\r\n ", rif_oid, p_rt->rif_oid, enable, category_id);
    if ( p_rt 
     && (p_rt->rif_oid == rif_oid)
     && (SAI_IP_ADDR_FAMILY_IPV4 == p_rt->key.destination.addr_family))
    {
        sai_unicast_route_entry_t route;
        sai_attribute_t attr;

        str[0] = 0;
        CTC_SAI_DEBUG("get route node. key:%s\r\n", stbl_route_key_val2str(p_rt, str, 128));

        /*
          set route NAT flags:
          l3iftype: route related to inside or disable NAT interfaces, set to 1; NAT outside, set to 0. 
                    be aware of the NAT disable interface, l3iftype should be set to 1, different with interface l3iftype flag set.
          category_id: NAT outside interface should set valid category_id, else set to zero(invalid).
          
        */
        sal_memset(&route, 0, sizeof(route));
        route.vr_id = p_rt->key.vr_id;
        sal_memcpy(&route.destination, &p_rt->key.destination, sizeof(sai_ip_prefix_t));

        /* case1: nat outside <-> nat disable, l3iftype not changed, but category_id changed.
           case2: nat outside <-> nat inside, l3iftype and category_id all changed.*/
        if (p_rt->category_id != category_id)
        {
            attr.id = SAI_ROUTE_ATTR_CATEGORY_ID;
            attr.value.u8 = (uint8)category_id;
            ctc_sai_set_route_attribute(&route, &attr, p_rt->nh_oid);
        }

        if (p_rt->l3if_type != l3iftype)
        {
            attr.id = SAI_ROUTE_ATTR_L3_IF_TYPE;
            attr.value.u8 = (uint8)l3iftype;
            ctc_sai_set_route_attribute(&route, &attr, p_rt->nh_oid);
        }
    }

    return;
}

extern sai_status_t
ctc_sai_set_neighbor_attribute(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ const sai_attribute_t *attr);

void
ctc_routerintf_set_nat_enable_refresh_nb(ctclib_hash_backet_t* node, void *para1, void *para2, void *para3)
{
    stbl_neighbor_t* p_nb = NULL;
    uint32 rif_id      = *(uint32*)para1;
    uint32 enable      = *(uint32*)para2;
    uint32 category_id = *(uint32*)para3;
    uint32 l3iftype    = SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL;
    uint64 rif_oid     = 0;

    CTC_SAI_DEBUG_FUNC();
    switch (enable)
    {
        case SAI_ROUTER_INTERFACE_NAT_INSIDE: /* enable inside */
        case SAI_ROUTER_INTERFACE_NAT_DISABLE: /* disable NAT*/
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL;
            break;
        }
        case SAI_ROUTER_INTERFACE_NAT_OUTSIDE: /* enable outside */
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL;
            break;
        }
        default:
            break;
    }


    rif_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_id);
    p_nb = (stbl_neighbor_t*)node->data;
    CTC_SAI_DEBUG("set l3iftype refresh nb, rif_oid:0x%lld, enable:%u, category_id:%u. node rif_oid:0x%llx\r\n ",rif_oid, enable, category_id, p_nb->key.rif_id);

    /* sdb neighbor save rif_oid in key.rif_id*/
    if ( p_nb 
     && (p_nb->key.rif_id == rif_oid)
     && (SAI_IP_ADDR_FAMILY_IPV4 == p_nb->key.ip_address.addr_family))
    {
        sai_neighbor_entry_t neigh;
        sai_attribute_t attr;
        CTC_SAI_DEBUG("get node. \r\n");

        /*
          set route NAT flags:
          l3iftype: route related to inside or disable NAT interfaces, set to 1; NAT outside, set to 0. 
                    be aware of the NAT disable interface, l3iftype should be set to 1, different with interface l3iftype flag set.
          category_id: NAT outside interface should set valid category_id, else set to zero(invalid).
          
        */
        sal_memset(&neigh, 0, sizeof(neigh));
        neigh.rif_id = p_nb->key.rif_id;
        sal_memcpy(&neigh.ip_address, &p_nb->key.ip_address, sizeof(sai_ip_address_t));

        /* case1: nat outside <-> nat disable, l3iftype not changed, but category_id changed.
           case2: nat outside <-> nat inside, l3iftype and category_id all changed.*/
        attr.id = SAI_NEIGHBOR_ATTR_CATEGORY_ID;
        attr.value.u8 = (uint8)category_id;
        ctc_sai_set_neighbor_attribute(&neigh, &attr);

        attr.id = SAI_NEIGHBOR_ATTR_L3IFTYPE;
        attr.value.u8 = (uint8)l3iftype;
        ctc_sai_set_neighbor_attribute(&neigh, &attr);
    }

    return;
}

sai_status_t
ctc_routerintf_set_nat_category_id(
    _In_  stbl_rif_t                    *p_sdb_rif,
    _In_  const sai_attribute_t 		*attr)
{
#ifdef USW

    /* --- 3 --- save to DB*/
    p_sdb_rif->nat_category_id = attr->value.u8;

    return SAI_STATUS_SUCCESS;
#else
    return SAI_STATUS_SUCCESS;
#endif
}


sai_status_t
ctc_routerintf_set_nat_enable(
    _In_  stbl_rif_t                    *p_sdb_rif,
    _In_  const sai_attribute_t 		*attr)
{
#ifdef USW
    int32_t     sdk_ret = 0;
    int32_t     l3iftype = 0;
    int32_t     ipv4_sa_type = CTC_L3IF_IPSA_LKUP_TYPE_NONE;
    int32_t     nat_enable = SAI_ROUTER_INTERFACE_NAT_DISABLE;
    stbl_route_master_t *p_db_rt_master = NULL;
    stbl_neighbor_master_t *p_db_nb_master = NULL;

    /* interface nat flags:
       l3iftype: 0 outside, 1 inside; default is 0
       ipv4 sa type: 0 normal, 2 nat. nat inside interface should set it to 2, else set to 0*/
    nat_enable = attr->value.u8;
    switch (nat_enable)
    {
        case SAI_ROUTER_INTERFACE_NAT_INSIDE: /* enable inside */
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL;
            ipv4_sa_type = CTC_L3IF_IPSA_LKUP_TYPE_NAT;
            break;
        }
        case SAI_ROUTER_INTERFACE_NAT_DISABLE: /* disable NAT*/
        case SAI_ROUTER_INTERFACE_NAT_OUTSIDE: /* enable outside */
        {
            l3iftype = SAI_ROUTER_INTERFACE_L3_TYPE_EXTERNAL;
            ipv4_sa_type = CTC_L3IF_IPSA_LKUP_TYPE_NONE;
            break;
        }
        default:
            break;
    }

    /* --- 1 --- set interface flags to chip*/
    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_NAT_IFTYPE, l3iftype), sdk_ret, out);
    CTC_SAI_ERROR_GOTO(ctc_l3if_set_property((uint16_t)(p_sdb_rif->key.rif_id), CTC_L3IF_PROP_IPV4_SA_TYPE, ipv4_sa_type), sdk_ret, out);

    /* --- 2 --- save to DB*/
    p_sdb_rif->nat_enable = nat_enable;

    /* --- 3.1 --- refresh dsIpDA->l3IfType, in route*/
    p_db_rt_master = stbl_route_get_master();
    if (p_db_rt_master)
    {
        ctclib_hash_iterate3(p_db_rt_master->route_hash, ctc_routerintf_set_nat_enable_refresh_rt, (void *)&p_sdb_rif->key.rif_id, (void *)&nat_enable, (void *)&p_sdb_rif->nat_category_id);
    }

    /* --- 3.1 --- refresh dsIpDA->l3IfType, in neighbor*/
    p_db_nb_master = stbl_neighbor_get_master();
    if (p_db_nb_master)
    {
        ctclib_hash_iterate3(p_db_nb_master->neigh_hash, ctc_routerintf_set_nat_enable_refresh_nb, (void *)&p_sdb_rif->key.rif_id, (void *)&nat_enable, (void *)&p_sdb_rif->nat_category_id);
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
#else
    return SAI_STATUS_SUCCESS;
#endif
}

/*add by zhw for ipmc*/
sai_status_t
ctc_routerintf_set_multicast(
    _In_  stbl_rif_t                    *p_sdb_rif,
    _In_  const sai_attribute_t         *attr)
{
    int32_t    	sdk_ret = 0;

    bool        admin_state = attr->value.booldata;

    CTC_SAI_ERROR_GOTO(
        ctc_l3if_set_property(
        
            (uint16_t)CTC_SAI_OBJECT_INDEX_GET(p_sdb_rif->key.rif_id),
                CTC_L3IF_PROP_IPV4_MCAST,admin_state),sdk_ret,out);
    //prouterintf_entry->v4_admin = attr->value.booldata;

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_routerintf_set_igmp( 
	_In_  stbl_rif_t	*p_sdb_rif,
	_In_  const sai_attribute_t 		*attr)
{
	ctc_pdu_global_l3pdu_action_t entry;
	ctc_pdu_l3pdu_type_t l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE;
	sal_memset(&entry, 0, sizeof(ctc_pdu_global_l3pdu_action_t));
	entry.entry_valid = 1;
	entry.action_index = 18;
	ctc_l3pdu_set_global_action(l3pdu_type, 1, &entry);
	
	return SAI_STATUS_SUCCESS;
}
/*end add zhw*/

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_set_router_interface_attribute_debug_param(
    _In_ sai_object_id_t rif_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG("in:rif_id %u", rif_id);
    switch(attr->id)
    {
    case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS");
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE %u", attr->value.booldata);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE %u", attr->value.booldata);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN %u", attr->value.booldata);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE %u", attr->value.booldata);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_MTU:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_MTU %u", attr->value.u32);
        break;
     case SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS");
        break;
    case SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID");
        break;
    case SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE:
        CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE");
        break;
    }
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_router_interface_attribute_debug_param(
    _In_ sai_object_id_t rif_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx    = 0;
    const sai_attribute_t *attr = NULL;

    CTC_SAI_DEBUG("in:rif_id %"PRIu64" attr_count %u", rif_id, attr_count);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS");
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE %u", attr->value.booldata);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE %u", attr->value.booldata);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN %u", attr->value.booldata);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE %u", attr->value.booldata);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            CTC_SAI_DEBUG("in:SAI_ROUTER_INTERFACE_ATTR_MTU %u", attr->value.u32);
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC

/*
* Routine Description:
*    Create router interface.
*
* Arguments:
*    [out] rif_id - router interface id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_router_interface(
    _Out_ sai_object_id_t* rif_oid,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t                 ret 				= SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t    *pattr_entry_list 	= NULL;
    stbl_rif_t   *p_sdb_rif = NULL;
    uint32_t wb_index = 0;
    sai_attribute_t              attr;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(rif_oid);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&attr,0,sizeof(sai_attribute_t));

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    if(pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_TYPE].value.s32 == SAI_ROUTER_INTERFACE_TYPE_PORT)
    {
        if(!pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_PORT_ID].valid)
        {
            ret = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            goto out;
        }
    }

    if(pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_TYPE].value.s32 == SAI_ROUTER_INTERFACE_TYPE_VLAN)
    {
        if(!pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_VLAN_ID].valid)
        {
            ret = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            goto out;
        }
    }

    wb_index = CTC_SAI_OBJECT_INDEX_GET(*rif_oid);
    CTC_SAI_ERROR_GOTO(ctc_routerintf_alloc(&p_sdb_rif, wb_index),ret,out);

    p_sdb_rif->vr_oid = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID].value.oid;
    p_sdb_rif->type = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_TYPE].value.s32;

    if (SAI_ROUTER_INTERFACE_TYPE_PORT == p_sdb_rif->type)
    {
        p_sdb_rif->port_oid = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_PORT_ID].value.oid;
    }
    else if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        p_sdb_rif->vlan_id = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_VLAN_ID].value.u16;
        ctc_sai_vlan_set_proto_excep(p_sdb_rif->vlan_id, TRUE);
    }

    CTC_SAI_ERROR_GOTO(ctc_routerintf_add_entry(p_sdb_rif),ret,out1);

    *rif_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ROUTER_INTERFACE, p_sdb_rif->key.rif_id);

    if (pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS].valid)
    {
        attr.id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
        attr.value = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS].value;
        ctc_sai_set_router_interface_attribute(*rif_oid,
                        &attr);
    }else{
#ifdef WB_TODO
        if (is_valid_ether_addr((uint8_t*)&prouter_intf_entry->pvr->smac))
        {
            ctc_routerintf_update_src_mac(prouter_intf_entry,
                (uint8_t*)&prouter_intf_entry->pvr->smac);
        }
#endif
    }

    if(pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE].valid)
    {
        attr.id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
        attr.value = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE].value;
        ctc_sai_set_router_interface_attribute(*rif_oid,
                        &attr);
    }

    if(pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE].valid)
    {
        attr.id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE;
        attr.value = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE].value;
        ctc_sai_set_router_interface_attribute(*rif_oid,
                        &attr);
    }

    if(pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_MTU].valid)
    {
        attr.id = SAI_ROUTER_INTERFACE_ATTR_MTU;
        attr.value.u32 = pattr_entry_list[SAI_ROUTER_INTERFACE_ATTR_MTU].value.u32;
        ctc_sai_set_router_interface_attribute(*rif_oid,
                        &attr);
    }

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
out1:
    if(p_sdb_rif)
    {
        ctc_routerintf_release(p_sdb_rif);
    }
    goto out;
}

/*
* Routine Description:
*    Remove router interface
*
* Arguments:
*    [in] rif_id - router interface id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_router_interface(
    _In_ sai_object_id_t rif_id)
{
    sai_status_t                 ret = SAI_STATUS_SUCCESS;
    stbl_rif_t   *p_sdb_rif = NULL;

    CTC_SAI_DEBUG_FUNC();

    p_sdb_rif = ctc_routerintf_get_by_oid_no_ref(rif_id);
    if(NULL == p_sdb_rif)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        ctc_sai_vlan_set_proto_excep(p_sdb_rif->vlan_id, FALSE);
    }

    CTC_SAI_ERROR_GOTO(ctc_routerintf_remove_entry(p_sdb_rif),ret,out);
#ifdef WB_TODO
    ctc_vr_release(p_sdb_rif->pvr);
#endif
    ctc_routerintf_release(p_sdb_rif);
out:
    return ret;
}

/*
* Routine Description:
*    Set router interface attribute
*
* Arguments:
*    [in] rif_id - router interface id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_router_interface_attribute(
    _In_ sai_object_id_t rif_oid,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    stbl_rif_t *p_sdb_rif = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr),ret,out);

    p_sdb_rif = ctc_routerintf_get_by_oid_no_ref(rif_oid);
    if(NULL == p_sdb_rif)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ctc_sai_set_router_interface_attribute_debug_param(rif_oid, attr);
    switch(attr->id)
    {
    case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
        ret = ctc_routerintf_set_src_mac(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
        ret = ctc_routerintf_set_v4_admin(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE:
        ret = ctc_routerintf_set_v4mcast_admin(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
        ret = ctc_routerintf_set_v6_admin(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN:
        ret = ctc_routerintf_set_igmp_snoop_en(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_MTU:
        ret = ctc_routerintf_set_mtu(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_URPF:
        ret = ctc_routerintf_set_urpf(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_URPF_V6:
        ret = ctc_routerintf_set_urpf_v6(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS:
        ret = ctc_routerintf_set_vmac(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
    case SAI_ROUTER_INTERFACE_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
    #ifdef USW
        ret = ctc_sai_qos_map_l3if_set_ctc_qos_map(p_sdb_rif->key.rif_id, attr);
    #endif
        break;
    case SAI_ROUTER_INTERFACE_ATTR_TRUST_DSCP:
        ret = ctc_routerintf_set_trust_dscp(p_sdb_rif, attr);
        break;
    /* begin added by taocy for NAT, 2019-06-03*/
    case SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID:
        ret = ctc_routerintf_set_nat_category_id(p_sdb_rif, attr);
        break;
    case SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE:
        ret = ctc_routerintf_set_nat_enable(p_sdb_rif, attr);
        break;
    /* end added by taocy for NAT, 2019-06-03*/
	/*add by zhw for ipmc*/
	case SAI_ROUTER_INTERFACE_ATTR_MULTICAST:
        ret = ctc_routerintf_set_multicast(p_sdb_rif, attr);
        break;
	case SAI_ROUTER_INTERFACE_ATTR_IGMP:
		ret = ctc_routerintf_set_igmp(p_sdb_rif, attr);
	/*end add zhw*/
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }

out:
    return ret;
}

/*
* Routine Description:
*    Get router interface attribute
*
* Arguments:
*    [in] rif_id - router interface id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_router_interface_attribute(
    _In_ sai_object_id_t rif_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;
    stbl_rif_t  *p_sdb_rif = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    p_sdb_rif = ctc_routerintf_get_by_oid_no_ref(rif_id);
    if(NULL == p_sdb_rif)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        default:
            return SAI_STATUS_NOT_SUPPORTED;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    ctc_sai_get_router_interface_attribute_debug_param(rif_id, attr_count, attr_list);
    return ret;
}

#define ________SAI_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_routerintf_init(),ret,out);

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}


static sai_router_interface_api_t      g_sai_api_func = {
    .create_router_interface      		= ctc_sai_create_router_interface,
    .remove_router_interface      		= ctc_sai_remove_router_interface,
    .set_router_interface_attribute     = ctc_sai_set_router_interface_attribute,
    .get_router_interface_attribute     = ctc_sai_get_router_interface_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  		= SAI_API_ROUTER_INTERFACE,
        .init_func 	= __init_mode_fn,
        .exit_func 	= __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_routerintf_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}


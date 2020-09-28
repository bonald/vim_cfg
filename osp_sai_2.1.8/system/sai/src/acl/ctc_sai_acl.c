#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_mirror.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_tap_group.h>
#include <ctc_sai_acl.h>
#include <ctc_sai_copp.h>

#ifdef USW
#include <ctc_sai_d2_mapping.h>
#else
#include <ctc_sai_gx_mapping.h>
#endif

extern int32 ftm_info_profile_type_gb;

extern sai_status_t ctc_sai_policer_acl_set_policer(uint32_t policer_id, uint32_t entry_id, uint32_t enable);

static ctc_sai_attr_entry_info_t g_acl_count_attr_entries[] = {
    {
        .id     = SAI_ACL_COUNTER_ATTR_TABLE_ID,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ACL_COUNTER_ATTR_PACKETS,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ACL_COUNTER_ATTR_BYTES,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

ctc_sai_acl_info_t  g_sai_acl_info;

static uint16	tap_erspan_eloop_port = 0;


#define ________SAI_ACL_SAI_INNER_API_FUNC


sai_int32_t
ctc_sai_acl_get_copp_group_id(sai_object_id_t port_id, uint32 *group_id)
{
    uint32 gport = 0;
    ctc_sai_port_objectid_to_gport(port_id, &gport);
    if (SAI_COPP_REASON_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_REASON_GROUP;
    }
    else if (SAI_COPP_FWD_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_FWD2CPU_GROUP;
    }
    else if (SAI_COPP_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_ACL_GROUP;
    }
    else if(SAI_COPP_HYBIRD_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_OPENFLOW_TO_CONTROLLER_GROUP;
    }
    else if(SAI_COPP_TTL_EXPIRE_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_TTL_EXPIRE_GROUP;
    }
    else if(SAI_COPP_MTU_CHECK_FAIL_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_MTU_CHECK_FAIL_GROUP;
    }
    else if(SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_L2PRO_PROTOCOL_MAC_GROUP;
    }
    else if(SAI_COPP_L2PRO_GROUP_MAC_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_L2PRO_GROUP_MAC_GROUP;
    }
#ifndef GREATBELT
    else if(SAI_COPP_IGMP_PORT_ID == gport)
    {
        *group_id = SAI_ACL_COPP_IGMP_GROUP;
    }
#endif
    else
    {
        *group_id = 0;
    }
    return SAI_STATUS_SUCCESS;
}

void ctc_sai_acl_alloc_entry_id(uint32* p_index)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    
    ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, p_index);
}

void ctc_sai_acl_release_entry_id(uint32 index)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    
    ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, index);
}

sai_int32_t
__tunnel_decap_alloc_erspan_nexthop (stbl_acl_tunnel_t *tun_info)
{
    sai_status_t             ret      = SAI_STATUS_SUCCESS;
    ctc_vlan_edit_nh_param_t nh_param = {0};
    ctc_vlan_egress_edit_info_t *edit_vlan = NULL;

    edit_vlan = &nh_param.vlan_edit_info;

    edit_vlan->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    /* edit_vlan->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE; */
    edit_vlan->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    edit_vlan->output_svid     = 1; //(sai_uint16_t)tun_info->key.fields.erspan_id.data.u32;
    edit_vlan->edit_flag      |= CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;

    nh_param.logic_port_valid  = 1;
    nh_param.logic_port        = tun_info->erspan_service_id;

    nh_param.gport_or_aps_bridge_id = tap_erspan_eloop_port;

    ret = ctc_nh_add_xlate(tun_info->erspan_nexthop_id, &nh_param);

    return ret;
}

sai_int32_t
__tunnel_decap_alloc_erspan_service_id (stbl_acl_tunnel_t *tun_info)
{
    sai_status_t         ret       = SAI_STATUS_SUCCESS;
    ctc_acl_group_info_t acl_group = {0};

    sal_memset(&acl_group, 0, sizeof(acl_group));

    acl_group.un.service_id = tun_info->erspan_service_id;
    acl_group.type     = CTC_ACL_GROUP_TYPE_SERVICE_ACL;
    acl_group.lchip    = 0;
    acl_group.priority = 0;
    acl_group.dir      = CTC_INGRESS;

    tun_info->erspan_service_group_id = 10000 + tun_info->erspan_service_id;

    ctc_acl_create_group(tun_info->erspan_service_group_id, &acl_group);
    ctc_acl_install_group(tun_info->erspan_service_group_id, &acl_group);

    return ret;
}


static stbl_acl_tunnel_t*
__tunnel_decap_alloc (ctc_sai_acl_tunnel_key_t *key, 
                              stbl_acl_entry_t  *p_sdb_entry)
{
    stbl_acl_tunnel_t   *ptunnel_decap = NULL;

    if(key->fields.erspan_id.enable)
    {
        ptunnel_decap = __tunnel_decap_alloc_erspan(key, p_sdb_entry);
    }
    else
    {
        ptunnel_decap = __tunnel_decap_alloc_tunnel(key, p_sdb_entry);
    }

    return ptunnel_decap;
}

void
__tunnel_decap_free_erspan (stbl_acl_tunnel_t *p_sdb_tunnel)
{
	stbl_acl_glb_t          *p_acl_glb = stbl_acl_glb_get_acl_glb();

    sal_assert(p_sdb_tunnel != NULL);

    CTC_SAI_DEBUG("%s%d:free nexthop entry_id = %d\n",
                  __FUNCTION__,__LINE__,p_sdb_tunnel->entry_id);

    ctc_scl_uninstall_entry(p_sdb_tunnel->entry_id);
    ctc_scl_remove_entry(p_sdb_tunnel->entry_id);

    ctc_scl_uninstall_group(p_sdb_tunnel->erspan_service_group_id);
    ctc_scl_destroy_group(p_sdb_tunnel->erspan_service_group_id);

    ctc_nh_remove_xlate(p_sdb_tunnel->erspan_nexthop_id);
	ctc_sai_nexthop_free_offset(p_sdb_tunnel->erspan_nexthop_id);
	
    ctclib_opb_free_offset(&p_acl_glb->erspan_service_opb, p_sdb_tunnel->erspan_service_id);
	ctclib_opb_free_offset(&p_acl_glb->tunnel_opb, p_sdb_tunnel->entry_id);
}


void
__tunnel_decap_free_tunnel (stbl_acl_tunnel_t *p_sdb_tunnel)
{
	stbl_acl_glb_t          *p_acl_glb = stbl_acl_glb_get_acl_glb();

    sal_assert(p_sdb_tunnel != NULL);

    CTC_SAI_DEBUG("%s%d:free nexthop entry_id = %d\n",
                  __FUNCTION__,__LINE__,p_sdb_tunnel->entry_id);

    ctc_scl_uninstall_entry(p_sdb_tunnel->entry_id);
    ctc_scl_remove_entry(p_sdb_tunnel->entry_id);
    ctclib_opb_free_offset(&p_acl_glb->tunnel_opb, p_sdb_tunnel->entry_id);

}

void
__tunnel_decap_free(stbl_acl_tunnel_t *p_sdb_tunnel)
{
    p_sdb_tunnel->count--;

    CTC_SAI_DEBUG("%s%d:dec tunnel count count = %d\n",
            __FUNCTION__,__LINE__, p_sdb_tunnel->count);
    
    if (p_sdb_tunnel->count <= 0)
    {
        CTC_SAI_DEBUG("%s%d:free nexthop entry_id = %d\n",
            __FUNCTION__,__LINE__, p_sdb_tunnel->entry_id);
        
        if(p_sdb_tunnel->key.fields.erspan_id.enable)
        {
            __tunnel_decap_free_erspan(p_sdb_tunnel);
        }
        else
        {
            __tunnel_decap_free_tunnel(p_sdb_tunnel);
        }


		stbl_acl_tunnel_del_acl_tunnel(&p_sdb_tunnel->key);
    }

	
}

#define ________SAI_ACL_SAI_TABLE_FUNC

void
ctc_sai_acl_db_release_table(stbl_acl_tbl_t *p_sdb_tbl)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    if (NULL == p_sdb_tbl)
    {
        return;
    }
    ctclib_opb_free_offset(&p_acl_glb->acl_tbl_opb, p_sdb_tbl->key.tbl_id);
    stbl_acl_tbl_del_acl_tbl(&p_sdb_tbl->key);
}

stbl_acl_tbl_t*
ctc_sai_acl_db_get_table_by_oid(const sai_object_id_t table_oid)
{
    stbl_acl_tbl_key_t key;

    sal_memset(&key, 0, sizeof(key));
    key.tbl_id = CTC_SAI_OBJECT_INDEX_GET(table_oid);
    return stbl_acl_tbl_get_acl_tbl(&key);
}

stbl_acl_entry_t*
ctc_sai_acl_db_get_entry_by_oid(const sai_object_id_t entry_oid)
{
    stbl_acl_entry_key_t key;

    sal_memset(&key, 0, sizeof(key));
    key.entry_id = CTC_SAI_OBJECT_INDEX_GET(entry_oid);
    return stbl_acl_entry_get_acl_entry(&key);
}

#ifdef GREATBELT

stbl_acl_port_t*
ctc_sai_acl_get_port_en_by_oid(const sai_object_id_t acl_port_oid)
{
    stbl_acl_port_key_t key;

    sal_memset(&key, 0, sizeof(key));
    key.port_oid = acl_port_oid;
    return stbl_acl_port_get_acl_port(&key);
}

stbl_acl_port_t*
ctc_sai_acl_port_en_db_alloc(sai_object_id_t acl_port_oid, ctc_direction_t dir, sai_object_id_t acl_lag_oid)
{
    stbl_acl_port_t  aclport;
    stbl_acl_port_t *p_sdb_aclport = NULL;
    uint32 gport = 0;
    uint16 class_id = 0;

    sal_memset(&aclport, 0, sizeof(aclport));
    
    if (acl_port_oid == acl_lag_oid)
    {
        ctc_sai_port_objectid_to_classid(acl_port_oid, &class_id);
    }
    else
    {
        ctc_sai_port_objectid_to_classid(acl_lag_oid, &class_id);
    }

    ctc_sai_port_objectid_to_gport(acl_port_oid, &gport);
    p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(acl_port_oid);
    if (p_sdb_aclport)
    {
        if (CTC_INGRESS == dir)
        {
            p_sdb_aclport->ingress_en_cnt++;
            if (1 == p_sdb_aclport->ingress_en_cnt)
            {
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x1);
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
            }   
        }
        else if (CTC_EGRESS == dir)
        {
            p_sdb_aclport->egress_en_cnt++;
            if (1 == p_sdb_aclport->egress_en_cnt)
            {
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x1);
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
            }
        }
        else if (CTC_BOTH_DIRECTION == dir)
        {
            p_sdb_aclport->ingress_en_cnt++;
            p_sdb_aclport->egress_en_cnt++;
            if (1 == p_sdb_aclport->ingress_en_cnt)
            {
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x1);
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
            }
            if (1 == p_sdb_aclport->egress_en_cnt)
            {
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x1);
                ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
            }
        }
        return p_sdb_aclport;
    }

    /* add a new aclport */
    sal_memset(&aclport, 0, sizeof(aclport));
    aclport.key.port_oid = acl_port_oid;
    ctc_sai_port_objectid_to_gport(acl_port_oid, &gport);
    if (CTC_INGRESS == dir)
    {
        aclport.ingress_en_cnt++;
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x1);
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
    }
    else if (CTC_EGRESS == dir)
    {
        aclport.egress_en_cnt++;
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x1);
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
    }
    else if (CTC_BOTH_DIRECTION == dir)
    {
        aclport.ingress_en_cnt++;
        aclport.egress_en_cnt++;
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x1);
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x1);
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
    }
    
    stbl_acl_port_add_acl_port(&aclport);
    p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(acl_port_oid);
    return p_sdb_aclport;
}

void
ctc_sai_acl_port_en_db_release(stbl_acl_port_t *p_aclport, ctc_direction_t dir)
{
    stbl_acl_port_t *p_sdb_aclport = NULL;
    uint32 gport = 0;
    
    if (NULL == p_aclport)
    {
        return;
    }

    ctc_sai_port_objectid_to_gport(p_aclport->key.port_oid, &gport);
    p_sdb_aclport = stbl_acl_port_get_acl_port(&p_aclport->key);
    if (NULL == p_sdb_aclport)
    {
        return;
    }

    if (CTC_INGRESS == dir && p_sdb_aclport->ingress_en_cnt > 0)
    {
        p_sdb_aclport->ingress_en_cnt--;
    }
    else if (CTC_EGRESS == dir && p_sdb_aclport->egress_en_cnt > 0)
    {
        p_sdb_aclport->egress_en_cnt--;
    }
    else if (CTC_BOTH_DIRECTION == dir && p_sdb_aclport->egress_en_cnt > 0 
        && p_sdb_aclport->ingress_en_cnt > 0)
    {
        p_sdb_aclport->ingress_en_cnt--;
        p_sdb_aclport->egress_en_cnt--;
    }
    
    if (0 == p_sdb_aclport->ingress_en_cnt)
    {
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x0);
    }

    if (0 == p_sdb_aclport->egress_en_cnt)
    {
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x0);
    }

    if (0 == p_sdb_aclport->ingress_en_cnt && 0 == p_sdb_aclport->egress_en_cnt)
    {
        stbl_acl_port_del_acl_port(&p_aclport->key);
    }
    return;
}

int32_t
_ctc_sai_acl_update_port_en_db(stbl_port_t *p_sdb_port, stbl_iter_args_t *pargs)
{ 
    ctc_sai_acl_en_port_node_t *acl_port_en_node = (ctc_sai_acl_en_port_node_t*)pargs->argv[0];
    sai_object_id_t acl_lag_oid;

    if ((NULL == p_sdb_port) || (NULL == acl_port_en_node))
    {
        return 0;
    }

    if (0 != p_sdb_port->lag_id)
    {
        acl_lag_oid = p_sdb_port->lag_id;
    }
    else
    {
        acl_lag_oid = p_sdb_port->key.port_oid;
    }
    
    ctc_sai_acl_port_en_db_alloc(p_sdb_port->key.port_oid, acl_port_en_node->dir, acl_lag_oid);

    return SAI_STATUS_SUCCESS;
}

int32_t
_ctc_sai_acl_dis_port_en_db(stbl_port_t *p_sdb_port, stbl_iter_args_t *pargs)
{
    ctc_sai_acl_en_port_node_t *acl_port_en_node = (ctc_sai_acl_en_port_node_t*)pargs->argv[0];
    stbl_acl_port_t *p_sdb_aclport = NULL;

    if ((NULL == p_sdb_port) || (NULL == acl_port_en_node))
    {
        return 0;
    }
    p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(p_sdb_port->key.port_oid);
    ctc_sai_acl_port_en_db_release(p_sdb_aclport, acl_port_en_node->dir);
    return SAI_STATUS_SUCCESS;
}

int32_t
_ctc_sai_acl_set_lag_en_db(stbl_port_t *p_sdb_port, stbl_iter_args_t *pargs)
{
    ctc_sai_acl_en_port_node_t *acl_port_en_node = (ctc_sai_acl_en_port_node_t*)pargs->argv[0];

    if ((NULL == p_sdb_port) || (NULL == acl_port_en_node))
    {
        return 0;
    }

    if (acl_port_en_node->port_oid == p_sdb_port->lag_id)
    {
        ctc_sai_acl_port_en_db_alloc(p_sdb_port->key.port_oid, acl_port_en_node->dir, p_sdb_port->lag_id);
    }

    return SAI_STATUS_SUCCESS;
}

int32_t
_ctc_sai_acl_unset_lag_en_db(stbl_port_t *p_sdb_port, stbl_iter_args_t *pargs)
{
    ctc_sai_acl_en_port_node_t *acl_port_en_node = (ctc_sai_acl_en_port_node_t*)pargs->argv[0];
    stbl_acl_port_t *p_sdb_aclport = NULL;

    if ((NULL == p_sdb_port) || (NULL == acl_port_en_node))
    {
        return 0;
    }

    if (acl_port_en_node->port_oid == p_sdb_port->lag_id)
    {
        p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(p_sdb_port->key.port_oid);
        ctc_sai_acl_port_en_db_release(p_sdb_aclport, acl_port_en_node->dir);
    }

    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_acl_en_lag_port(sai_object_id_t lag_oid, ctc_direction_t dirction)
{
    ctc_sai_acl_en_port_node_t acl_port_en_node;
    stbl_iter_args_t args;

    acl_port_en_node.port_oid = lag_oid;
    acl_port_en_node.dir = dirction;

    args.argv[0] = &acl_port_en_node;
    stbl_port_iterate((STBL_ITER_CB_FUNC)_ctc_sai_acl_set_lag_en_db, &args);
    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_acl_dis_lag_port(sai_object_id_t lag_oid, ctc_direction_t dirction)
{
    ctc_sai_acl_en_port_node_t acl_port_en_node;
    stbl_iter_args_t args;

    acl_port_en_node.port_oid = lag_oid;
    acl_port_en_node.dir = dirction;

    args.argv[0] = &acl_port_en_node;
    stbl_port_iterate((STBL_ITER_CB_FUNC)_ctc_sai_acl_unset_lag_en_db, &args);
    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_acl_enable_global_port()
{
    ctc_sai_acl_en_port_node_t acl_port_en_node;
    stbl_iter_args_t args;

    acl_port_en_node.dir = CTC_INGRESS;
    args.argv[0] = &acl_port_en_node;
    stbl_port_iterate((STBL_ITER_CB_FUNC)_ctc_sai_acl_update_port_en_db, &args);

    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_acl_disable_global_port()
{
    ctc_sai_acl_en_port_node_t acl_port_en_node;
    stbl_iter_args_t args;

    acl_port_en_node.dir = CTC_INGRESS;
    args.argv[0] = &acl_port_en_node;
    stbl_port_iterate((STBL_ITER_CB_FUNC)_ctc_sai_acl_dis_port_en_db, &args);

    return SAI_STATUS_SUCCESS;
}

int32_t
_ctc_sai_acl_get_entry_lag_dir(stbl_acl_entry_t	*p_sdb_entry, ctc_sai_acl_en_port_node_t *acl_port_en_node)
{
    if ((NULL == p_sdb_entry) || (NULL == acl_port_en_node))
    {
        return 0;
    }

    if (p_sdb_entry->port_oid_list[0] == acl_port_en_node->port_oid &&
        p_sdb_entry->dirction == acl_port_en_node->dir)
    {
        acl_port_en_node->entries_cnt++;
        return 1;
    }
    return 0;
}

int32_t
ctc_sai_acl_get_entry_by_oid_dir(sai_object_id_t acl_port_oid, ctc_direction_t dirction)
{
    stbl_acl_entry_master_t *p_master = stbl_acl_entry_get_master();
    stbl_acl_entry_t *p_sdb_entry = NULL;
    ctc_sai_acl_en_port_node_t acl_port_en;
    uint32_t entry_id = 0;

    /*modified by yejl for bug 49639, 47768, 2018-11-06*/
    sal_memset(&acl_port_en, 0, sizeof(acl_port_en));
    acl_port_en.port_oid = acl_port_oid;
    acl_port_en.dir = dirction;
    acl_port_en.entries_cnt = 0;
    for (entry_id = 0; entry_id < GLB_SAI_ACE_MAX; entry_id++)
    {
        p_sdb_entry = p_master->array[entry_id];
        if (NULL == p_sdb_entry)
        {
            continue;
        }
        _ctc_sai_acl_get_entry_lag_dir(p_sdb_entry, &acl_port_en);
    }

    return acl_port_en.entries_cnt;
}

#endif /* GREATBELT */

int
ctc_sai_acl_worm_filter_init()
{
    ctc_acl_group_info_t acl_group;
    sai_uint32_t group_id = SAI_ACL_WORM_FILTER_GROUP;

    sal_memset(&acl_group, 0, sizeof(acl_group));
    acl_group.type     = CTC_ACL_GROUP_TYPE_GLOBAL;
    acl_group.dir      = CTC_INGRESS;

    ctc_acl_create_group(group_id, &acl_group);
    ctc_acl_install_group(group_id, &acl_group);
    return SAI_STATUS_SUCCESS;

}

int32
ctc_sai_acl_global_tap_egress_default_init()
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    ctc_acl_group_info_t acl_group;
    ctc_acl_entry_t acl_entry;
    stbl_acl_entry_t sai_entry;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_uint32_t ret_ctc = 0;
    sai_uint32_t group_id = 0;
    sai_uint32_t index = 0;

    /*egress default entry*/
    sal_memset(&acl_group, 0, sizeof(acl_group));
    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    sal_memset(&sai_entry, 0, sizeof(sai_entry));

    acl_group.type     = CTC_ACL_GROUP_TYPE_GLOBAL;
    acl_group.dir      = CTC_EGRESS;
    group_id           = SAI_ACL_GLOBAL_EGRESS_GROUP;

    ctc_acl_create_group(group_id, &acl_group);
    ctc_acl_install_group(group_id, &acl_group);

    ctclib_opb_alloc_offset(&p_acl_glb->acl_entry_opb, &index);
    sai_entry.key.entry_id = index;
    CTC_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
    acl_entry.key.type = CTC_ACL_KEY_IPV4;
    acl_entry.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;

    ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
    sai_entry.ctc_entry_id_list[0] = index;
    sai_entry.ctc_entry_id_count = 1;
    sai_entry.dirction = CTC_EGRESS;
    sai_entry.port_oid_list[0] = 0;
    sai_entry.port_oid_count = 1;

    acl_entry.entry_id = sai_entry.ctc_entry_id_list[0];
    acl_entry.priority = index;
    acl_entry.priority_valid = TRUE;
    ret_ctc = ctc_acl_add_entry(SAI_ACL_GLOBAL_EGRESS_GROUP, &acl_entry);
    if (CTC_E_NONE != ret_ctc)
    {        
        ret = SAI_STATUS_FAILURE;
    }
    ret_ctc = ctc_acl_install_entry(acl_entry.entry_id);
    if (CTC_E_NONE != ret_ctc)
    {
        ctc_acl_remove_entry(acl_entry.entry_id);          
        ret = SAI_STATUS_FAILURE;
    }

    sai_entry.enable = TRUE;
    stbl_acl_entry_add_acl_entry(&sai_entry);

    //TODO:need to add ipv6 default egress discard key
    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    sal_memset(&sai_entry, 0, sizeof(sai_entry));
    
    ctclib_opb_alloc_offset(&p_acl_glb->acl_entry_opb, &index);
    sai_entry.key.entry_id = index;
    CTC_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
    acl_entry.key.type = CTC_ACL_KEY_IPV6;

    ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
    sai_entry.ctc_entry_id_list[0] = index;
    sai_entry.ctc_entry_id_count = 1;
    sai_entry.dirction = CTC_EGRESS;
    sai_entry.port_oid_list[0] = 0;
    sai_entry.port_oid_count = 1;

    acl_entry.entry_id = sai_entry.ctc_entry_id_list[0];
    acl_entry.priority = index;
    acl_entry.priority_valid = TRUE;
    ret_ctc = ctc_acl_add_entry(SAI_ACL_GLOBAL_EGRESS_GROUP, &acl_entry);

    if (CTC_E_NONE != ret_ctc)
    {        
        ret = SAI_STATUS_FAILURE;
    }

    ret_ctc = ctc_acl_install_entry(acl_entry.entry_id);
    if (CTC_E_NONE != ret_ctc)
    {
        ctc_acl_remove_entry(acl_entry.entry_id);          
        ret = SAI_STATUS_FAILURE;
    }

    sai_entry.enable = TRUE;
    stbl_acl_entry_add_acl_entry(&sai_entry);
    
    return ret;
}

int32
ctc_sai_acl_port_enable(ctc_direction_t dir, sai_object_id_t port)
{
    ctc_acl_group_info_t acl_group;
    ctc_scl_group_info_t scl_group;
    uint32 group_id = 0;
    uint32 gport    = 0;
    uint16 class_id = 0;
    uint8 priority = 0;

    ctc_port_scl_property_t port_scl_property;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("dir = %d, port = 0x%llx", dir, port);
    
    ctc_sai_port_objectid_to_gport(port, &gport);

    group_id = ctc_sai_port_make_acl_group_id(dir, gport);
    ctc_sai_port_objectid_to_classid(port, &class_id);
    
#ifdef GOLDENGATE
    #ifndef OFPRODUCT
    if (CTC_INGRESS == dir)
    {
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0x1);
        /*must config CTC_ACL_TCAM_LKUP_TYPE_L2, acl key is all ipv4 double key*/
        /*changed for tap support ipv6*/
        #ifdef TAPPRODUCT 
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);
        #else 
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);/*modified by yejl for bug 52519, 2019-06-19*/
        #endif /* TAPPRODUCT */
    }
    else
    {
        #ifndef TAPPRODUCT
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0x1);
        #endif 

        #ifdef TAPPRODUCT 
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);
        #else
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);/*modified by yejl for bug 52519, 2019-06-19*/
        #endif /* TAPPRODUCT */
    }
    #endif /* !OFPRODUCT */
#endif /* GOLDENGATE */

#ifdef USW
    #ifndef OFPRODUCT
    if (CTC_INGRESS == dir)
    {
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0x1);
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);
        #ifdef TAPPRODUCT
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT);
        #endif
    }
    else
    {
        #ifndef TAPPRODUCT
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0x1);
        #endif /* !TAPPRODUCT */
        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);
    }
    #endif /* !OFPRODUCT */   
#endif /* USW */

/* added by hansf for ttp */
#ifdef TTPPRODUCT
    #ifdef GOLDENGATE
        /* for gg ttp system, tradictional acl use acl3, openflow port use acl0 */
        uint32 ttp_group_id = 0;
        if (CTC_INGRESS == dir)
        {
            ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0x8);
            ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID_3, dir, class_id);
            ttp_group_id = ctc_sai_port_get_ttp_acl_group_id(dir, gport);
            sal_memset(&acl_group, 0, sizeof(acl_group));
            acl_group.un.port_class_id = class_id;
            acl_group.type     = CTC_ACL_GROUP_TYPE_PORT_CLASS;
            acl_group.lchip    = 0;
            acl_group.priority = 0;
            acl_group.dir      = dir;
            ctc_acl_create_group(ttp_group_id, &acl_group);
            ctc_acl_install_group(ttp_group_id, &acl_group);

            priority = 3;
        }
    #endif /* GOLDENGATE */
#endif /* TTPPRODUCT */

    ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, class_id);
    ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, dir, 1);

    sal_memset(&acl_group, 0, sizeof(acl_group));
    acl_group.un.port_class_id = class_id;
    acl_group.type     = CTC_ACL_GROUP_TYPE_PORT_CLASS;
    acl_group.lchip    = 0;
    acl_group.priority = priority;
    acl_group.dir      = dir;

    ctc_acl_create_group(group_id, &acl_group);
    ctc_acl_install_group(group_id, &acl_group);

    sal_memset(&port_scl_property, 0, sizeof(port_scl_property));
    port_scl_property.scl_id = 0;
    port_scl_property.direction = CTC_INGRESS;
    port_scl_property.class_id  = class_id;
    port_scl_property.class_id_en = TRUE;
    port_scl_property.tcam_type   = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
    port_scl_property.hash_type   = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    ctc_port_set_scl_property(gport, &port_scl_property);

    sal_memset(&scl_group, 0, sizeof(scl_group));
    scl_group.un.port_class_id = class_id;
    scl_group.type     = CTC_SCL_GROUP_TYPE_PORT_CLASS;
    scl_group.lchip    = 0;
    scl_group.priority = 0;

    ctc_scl_create_group(group_id, &scl_group);
    ctc_scl_install_group(group_id, &scl_group);
    
    return SAI_STATUS_SUCCESS;
}

int
ctc_sai_acl_lag_install(ctc_direction_t dir, sai_object_id_t lag_id)
{
    ctc_acl_group_info_t acl_group;
    ctc_scl_group_info_t scl_group;
    uint32 group_id = 0;
    uint32 gport    = 0;
    uint16 class_id = 0;

    ctc_port_scl_property_t port_scl_property;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("dir = %d, port = 0x%llx", dir, lag_id);

    ctc_sai_port_objectid_to_gport(lag_id, &gport);
    
    group_id = ctc_sai_port_make_acl_group_id(dir, gport);
    ctc_sai_port_objectid_to_classid(lag_id, &class_id);
    
    sal_memset(&acl_group, 0, sizeof(acl_group));
    acl_group.un.port_class_id = class_id;
    acl_group.type     = CTC_ACL_GROUP_TYPE_PORT_CLASS;
    acl_group.lchip    = 0;
    acl_group.priority = 0;
    acl_group.dir      = dir;

    ctc_acl_create_group(group_id, &acl_group);
    ctc_acl_install_group(group_id, &acl_group);

    sal_memset(&port_scl_property, 0, sizeof(port_scl_property));
    port_scl_property.scl_id = 0;
    port_scl_property.direction = CTC_INGRESS;
    port_scl_property.class_id  = class_id;
    port_scl_property.class_id_en = TRUE;
    port_scl_property.tcam_type   = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
    port_scl_property.hash_type   = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    ctc_port_set_scl_property(gport, &port_scl_property);

    sal_memset(&scl_group, 0, sizeof(scl_group));
    scl_group.un.port_class_id = class_id;
    scl_group.type     = CTC_SCL_GROUP_TYPE_PORT_CLASS;
    scl_group.lchip    = 0;
    scl_group.priority = 0;

    ctc_scl_create_group(group_id, &scl_group);
    ctc_scl_install_group(group_id, &scl_group);
    
    return SAI_STATUS_SUCCESS;
}

int
ctc_sai_acl_lag_member_enable(sai_object_id_t lag_id, sai_object_id_t lag_member_port_id)
{
    uint32 lag_member_gport = 0;
    uint16 class_id  = 0;
    uint32 lag_gport = 0;
#ifdef GREATBELT
    uint32 entres_cnt = 0;
    uint32 i = 0;
#endif
    ctc_port_scl_property_t port_scl_property;

    ctc_sai_port_objectid_to_gport(lag_id, &lag_gport);
    ctc_sai_port_objectid_to_gport(lag_member_port_id, &lag_member_gport);

    ctc_sai_port_objectid_to_classid(lag_id, &class_id);

#ifdef GREATBELT
    entres_cnt = ctc_sai_acl_get_entry_by_oid_dir(lag_id, CTC_INGRESS);
    for (i = 0; i < entres_cnt; i++)
    {
        ctc_sai_acl_port_en_db_alloc(lag_member_port_id, CTC_INGRESS, lag_id);
    }
    entres_cnt = ctc_sai_acl_get_entry_by_oid_dir(lag_id, CTC_EGRESS);
    for (i = 0; i < entres_cnt; i++)
    {
        ctc_sai_acl_port_en_db_alloc(lag_member_port_id, CTC_EGRESS, lag_id);
    }
#endif /* GREATBELT */
    
    ctc_port_set_direction_property(lag_member_gport, CTC_PORT_DIR_PROP_ACL_CLASSID, CTC_INGRESS, class_id);

    ctc_port_set_direction_property(lag_member_gport, CTC_PORT_DIR_PROP_ACL_CLASSID, CTC_EGRESS, class_id);

    sal_memset(&port_scl_property, 0, sizeof(port_scl_property));
    port_scl_property.scl_id = 0;
    port_scl_property.direction = CTC_INGRESS;
    ctc_port_get_scl_property(lag_member_gport, &port_scl_property);

    port_scl_property.class_id = class_id;
    ctc_port_set_scl_property(lag_member_gport, &port_scl_property);


    return SAI_STATUS_SUCCESS;
}

int
ctc_sai_acl_lag_member_disable(sai_object_id_t lag_member_port_id, sai_object_id_t lag_id)
{
    uint32 lag_member_gport = 0;
    uint16 class_id  = 0;
    
#ifdef GREATBELT
    stbl_acl_port_t *p_sdb_aclport = NULL;
    uint32 entres_cnt = 0;
    uint32 i = 0;
#endif
    ctc_port_scl_property_t port_scl_property;

    ctc_sai_port_objectid_to_gport(lag_member_port_id, &lag_member_gport);

    ctc_sai_port_objectid_to_classid(lag_member_port_id, &class_id);
    ctc_port_set_direction_property(lag_member_gport, CTC_PORT_DIR_PROP_ACL_CLASSID, CTC_INGRESS, class_id);

    ctc_port_set_direction_property(lag_member_gport, CTC_PORT_DIR_PROP_ACL_CLASSID, CTC_EGRESS, class_id);

    sal_memset(&port_scl_property, 0, sizeof(port_scl_property));
    port_scl_property.scl_id = 0;
    port_scl_property.direction = CTC_INGRESS;
    ctc_port_get_scl_property(lag_member_gport, &port_scl_property);

    port_scl_property.class_id = class_id;
    ctc_port_set_scl_property(lag_member_gport, &port_scl_property);

#ifdef GREATBELT
    p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(lag_member_port_id);
    entres_cnt = ctc_sai_acl_get_entry_by_oid_dir(lag_id, CTC_INGRESS);
    for (i = 0; i < entres_cnt; i++)
    {
        ctc_sai_acl_port_en_db_release(p_sdb_aclport, CTC_INGRESS);
    }
    entres_cnt = ctc_sai_acl_get_entry_by_oid_dir(lag_id, CTC_EGRESS);
    for (i = 0; i < entres_cnt; i++)
    {
        ctc_sai_acl_port_en_db_release(p_sdb_aclport, CTC_EGRESS);
    }
#endif /* GREATBELT */

    return SAI_STATUS_SUCCESS;
}




#define ________SAI_ACL_SAI_DEBUG_FUNC
sai_status_t
sai_create_acl_entry_debug_param(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    const sai_attribute_t *pattr_entry = NULL;
    uint32 i = 0;
    sai_object_id_t     table_id = 0;
    stbl_acl_tbl_t      *p_sdb_tbl = NULL;
    
    for (i = 0; i < attr_count; i++)
    {
        pattr_entry = &attr_list[i];
        switch (pattr_entry->id)
        {
        case SAI_ACL_ENTRY_ATTR_TABLE_ID:
            table_id = pattr_entry->value.oid;
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_TABLE_ID 0x%llx", pattr_entry->value.oid);
            break;
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(table_id);
            if (NULL == p_sdb_tbl)
            {
                CTC_SAI_DEBUG("in:sai_create_acl_entry_debug_param invalid table_id");
            }
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_PRIORITY %"PRIu64, ((pattr_entry->value.u32) & 0xffff) + (((p_sdb_tbl->priority) & 0xffff) << 16));
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT 0x%llx", pattr_entry->value.aclfield.data.oid);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT 0x%llx", pattr_entry->value.aclfield.data.oid);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC %x:%x:%x:%x:%x:%x", 
                pattr_entry->value.aclfield.data.mac[5], pattr_entry->value.aclfield.data.mac[4], pattr_entry->value.aclfield.data.mac[3],
                pattr_entry->value.aclfield.data.mac[2], pattr_entry->value.aclfield.data.mac[1], pattr_entry->value.aclfield.data.mac[0]);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC_MASK %x:%x:%x:%x:%x:%x", 
                pattr_entry->value.aclfield.mask.mac[5], pattr_entry->value.aclfield.mask.mac[4], pattr_entry->value.aclfield.mask.mac[3],
                pattr_entry->value.aclfield.mask.mac[2], pattr_entry->value.aclfield.mask.mac[1], pattr_entry->value.aclfield.mask.mac[0]);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC %x:%x:%x:%x:%x:%x", 
                pattr_entry->value.aclfield.data.mac[5], pattr_entry->value.aclfield.data.mac[4], pattr_entry->value.aclfield.data.mac[3],
                pattr_entry->value.aclfield.data.mac[2], pattr_entry->value.aclfield.data.mac[1], pattr_entry->value.aclfield.data.mac[0]);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC_MASK %x:%x:%x:%x:%x:%x", 
                pattr_entry->value.aclfield.mask.mac[5], pattr_entry->value.aclfield.mask.mac[4], pattr_entry->value.aclfield.mask.mac[3],
                pattr_entry->value.aclfield.mask.mac[2], pattr_entry->value.aclfield.mask.mac[1], pattr_entry->value.aclfield.mask.mac[0]);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID %u", pattr_entry->value.aclfield.data.u16);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID_MAK %u", pattr_entry->value.aclfield.mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI %u", pattr_entry->value.aclfield.data.u8);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI_MASK %u", pattr_entry->value.aclfield.mask.u8);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID %u", pattr_entry->value.aclfield.data.u16);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID_MASK %u", pattr_entry->value.aclfield.mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI %u", pattr_entry->value.aclfield.data.u8);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI_MASK %u", pattr_entry->value.aclfield.mask.u8);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE %u", pattr_entry->value.aclfield.data.u16);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE_MASK %u", pattr_entry->value.aclfield.mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP %u", pattr_entry->value.aclfield.data.ip4);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP_MASK %u", pattr_entry->value.aclfield.mask.ip4);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_DST_IP %u", pattr_entry->value.aclfield.data.ip4);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_DST_IP_MASK %u", pattr_entry->value.aclfield.mask.ip4);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL %u", pattr_entry->value.aclfield.data.u8);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL_MASK %u", pattr_entry->value.aclfield.mask.u8);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT %u", pattr_entry->value.aclfield.data.u16);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT_MASK %u", pattr_entry->value.aclfield.mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT %u", pattr_entry->value.aclfield.data.u16);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT_MASK %u", pattr_entry->value.aclfield.mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_UDF_MATCH:
            /* CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_UDF %u", pattr_entry->value.aclfield.data.u32);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_UDF_MASK %u", pattr_entry->value.aclfield.mask.u32); */
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            /*unsupport*/
            break;

        case SAI_ACL_ENTRY_ATTR_PACKET_ACTION:
            if (pattr_entry->value.aclaction.enable)
            {
                switch(pattr_entry->value.aclaction.parameter.s32)
                {
                    case SAI_PACKET_ACTION_DROP:
                        CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_DROP");
                        break;

                    case SAI_PACKET_ACTION_FORWARD:
                        CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_FORWARD");
                        break;

                    case SAI_PACKET_ACTION_TRAP:
                        CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_TRAP");
                        break;

                    case SAI_PACKET_ACTION_LOG:
                        CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_DROP");
                        break;

                    default:
                        break;            
                }
            }
            break;

        case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER:
            if (pattr_entry->value.aclaction.enable)
            {
                CTC_SAI_DEBUG("in:CTC_ACL_ACTION_FLAG_STATS");
            }
            break;

        case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
            if (pattr_entry->value.aclaction.enable)
            {
                CTC_SAI_DEBUG("in:CTC_ACL_ACTION_FLAG_PRIORITY");
            }
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t  sai_get_acl_entry_attribute_debug_param (
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx = 0;
    const sai_attribute_t *pattr_entry = NULL;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = &attr_list[attr_idx];

        switch (pattr_entry->id)
        {
        case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
            CTC_SAI_DEBUG("out:SAI_ACL_ENTRY_ATTR_ADMIN_STATE %u", pattr_entry->value.booldata);
            break;
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            CTC_SAI_DEBUG("out:SAI_ACL_ENTRY_ATTR_PRIORITY %u", pattr_entry->value.u32);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_acl_counter_attribute_debug_param(
    _In_ sai_object_id_t acl_counter_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx = 0;
    const sai_attribute_t *pattr_entry = NULL;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = &attr_list[attr_idx];

        switch (pattr_entry->id)
        {

        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
        case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
        case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
            break;

        case SAI_ACL_COUNTER_ATTR_PACKETS:
            CTC_SAI_DEBUG("out:SAI_ACL_COUNTER_ATTR_PACKETS %"PRIu64, pattr_entry->value.u64);
            break;
        case SAI_ACL_COUNTER_ATTR_BYTES:
            CTC_SAI_DEBUG("out:SAI_ACL_COUNTER_ATTR_BYTES %"PRIu64, pattr_entry->value.u64);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_ACL_SAI_API_FUNC
/*
* Routine Description:
*   Create an ACL table
*
* Arguments:
 *  [out] acl_table_id - the the acl table id
 *  [in] attr_count - number of attributes
 *  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_acl_table(
    _Out_ sai_object_id_t* acl_table_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const   *pattr_entry = NULL;
    stbl_acl_tbl_t          acl_tbl;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(acl_table_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&acl_tbl, 0, sizeof(acl_tbl));

    /* 1. check and assign parameters */
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_TABLE_ATTR_STAGE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        acl_tbl.stage = pattr_entry->value.s32;
    }
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_TABLE_ATTR_PRIORITY, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        acl_tbl.priority = pattr_entry->value.u32;
    }

    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_ACL_TABLE_ATTR_FIELD_START <= attr_list[index].id) && (SAI_ACL_TABLE_ATTR_FIELD_END >= attr_list[index].id))
        {
            if ((SAI_ACL_TABLE_ATTR_FIELD_TTL == attr_list[index].id) ||
                (SAI_ACL_TABLE_ATTR_FIELD_TOS == attr_list[index].id) ||
                (SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS == attr_list[index].id) ||
                (SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI == attr_list[index].id) ||
                (SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI == attr_list[index].id))
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            if (attr_list[index].value.booldata)
            {
                acl_tbl.field_bitmap |= (1 << (attr_list[index].id - SAI_ACL_TABLE_ATTR_FIELD_START));
            }
        }
    }

    /* pass at least one field */
    if (0 == acl_tbl.field_bitmap)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* 2. alloc offset */
    ctclib_opb_alloc_offset(&p_acl_glb->acl_tbl_opb, &index);
    acl_tbl.key.tbl_id = index;

    /* 3. add to SDB */
    stbl_acl_tbl_add_acl_tbl(&acl_tbl);

    /* 4. return tbl_oid */
    *acl_table_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_TABLE, acl_tbl.key.tbl_id);

    return ret;
}

/*
* Routine Description:
*   Delete an ACL table
*
* Arguments:
*   [in] acl_table_id - the acl table id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_delete_acl_table(
    _In_ sai_object_id_t acl_table_id)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    stbl_acl_tbl_t              *p_sdb_tbl = NULL;

    CTC_SAI_DEBUG_FUNC();

    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(acl_table_id);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ctc_sai_acl_db_release_table(p_sdb_tbl);

    return ret;
}

/*
* Routine Description:
*   Set ACL table attribute
*
* Arguments:
*    [in] acl_table_id - the acl table id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t  sai_set_acl_table_attribute(
    _In_ sai_object_id_t acl_table_id,
    _In_ const sai_attribute_t *attr)
{

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr);

    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

/*
* Routine Description:
*   Get ACL table attribute
*
* Arguments:
*    [in] acl_table_id - acl table id
*    [in] attr_count - number of attributes
*    [Out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_acl_table_attribute(
    _In_ sai_object_id_t acl_table_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t *attr        = NULL;
    uint32_t         attr_idx    = 0;
    stbl_acl_tbl_t  *p_sdb_tbl = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(acl_table_id);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        if (SAI_ACL_TABLE_ATTR_STAGE == attr->id)
        {
            attr->value.s32 = p_sdb_tbl->stage;
        }
        else if (SAI_ACL_TABLE_ATTR_PRIORITY == attr->id)
        {
            attr->value.u32 = p_sdb_tbl->priority;
        }
        else if ((SAI_ACL_TABLE_ATTR_FIELD_START <= attr->id) && (SAI_ACL_TABLE_ATTR_FIELD_END >= attr->id))
        {
            attr->value.booldata = CTC_IS_BIT_SET(p_sdb_tbl->field_bitmap, attr->id - SAI_ACL_TABLE_ATTR_FIELD_START);
        }
    }

    return ret;
}




void
__connect_acl2scl(stbl_acl_tunnel_t           *p_sdb_tunnel,
                  stbl_acl_entry_t                  *p_sdb_entry,
                  sai_object_id_t                   in_port_oid,
                  fields_mapping_res_t              *res)
{
	strip_header_parameter_t	strip_header = {};

#ifdef USW
    ctc_field_key_t         key_field;
    sal_memset(&key_field, 0, sizeof(key_field));
#endif /* USW */

	/* gre + erspan + vxlan, need strip vxlan header */
    if(p_sdb_entry->outer_fields.erspan_id.enable)
    {
        if(p_sdb_entry->action.erspan_strip_inner_vxlan_header.enable)
        {
            strip_header.strip_header.enable = 1;
            __config_strip_header(&p_sdb_entry->inner_fields, &strip_header,res);
        }
        return ;
    }
	
    if (NULL == p_sdb_tunnel)
    {
        if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(in_port_oid))
        {
            if(ctc_sai_port_get_metadata(in_port_oid))
            {
                if (p_sdb_entry->inner_fields.is_ipv6)
                {
                    CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_METADATA);
                    res->aclv6_key.metadata        = 0;
                    res->aclv6_key.metadata_mask   = 0xFFFFFFFF;
                }
                else
                {
                    CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_METADATA);
                    res->acl_key.metadata        = 0;
                    res->acl_key.metadata_mask   = 0xFFFFFFFF;
                
                }
#ifdef USW
                key_field.type = CTC_FIELD_KEY_METADATA;
                key_field.data = 0;
                key_field.mask = 0x3FFF;
                (void)ctc_acl_add_key_field(res->acl.entry_id, &key_field);
#endif /* USW */
            }
        }
    }
    else
    {
        if (p_sdb_entry->inner_fields.is_ipv6)
        {
            CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_METADATA);
            res->aclv6_key.metadata        = p_sdb_tunnel->entry_id;
            res->aclv6_key.metadata_mask   = 0xFFFFFFFF;
        }
        else
        {
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_METADATA);
            res->acl_key.metadata        = p_sdb_tunnel->entry_id;
            res->acl_key.metadata_mask   = 0xFFFFFFFF;
        }
#ifdef USW
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_sdb_tunnel->entry_id;
        key_field.mask = 0x3FFF;
        (void)ctc_acl_add_key_field(res->acl.entry_id, &key_field);
#endif /* USW */
    }

	strip_header.strip_header = p_sdb_entry->action.strip_header;
    strip_header.strip_header_pos = p_sdb_entry->action.strip_header_pos;
    strip_header.strip_header_offset = p_sdb_entry->action.strip_header_offset;
    __config_strip_header(&p_sdb_entry->outer_fields, &strip_header, res);
}

void 
__convert_attr_list_to_sai_acl_entry(
        _In_ uint32_t               attr_count,
        _In_ const sai_attribute_t  *attr_list,
             stbl_acl_entry_t       *p_sdb_entry)
{
#define __ACL_GET_FILED_ATTR(attr_id, member)                       \
    {                                                               \
        attr_stage = ctc_sai_attr_get_attr_by_id(attr_id,           \
                                                attr_list,          \
                                                attr_count,         \
                                                NULL);              \
                                                                    \
        if(attr_stage)                                              \
        {                                                           \
            if( attr_id < SAI_ACL_ENTRY_ATTR_FIELD_INNER_END &&     \
                attr_id > SAI_ACL_ENTRY_ATTR_FIELD_INNER_START )    \
            {                                                       \
                p_sdb_entry->apply_inner_lookup = TRUE;              \
                if (SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPv6 == attr_id ||  \
                    SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPv6 == attr_id ||  \
                    SAI_ACL_ENTRY_ATTR_FIELD_INNER_IPv6_FLOW_LABEL == attr_id)  \
                {                                                       \
                    p_sdb_entry->inner_fields.is_ipv6 = TRUE;           \
                }                                                        \
                sal_memcpy(&p_sdb_entry->inner_fields.member, &attr_stage->value.aclfield, sizeof(sai_acl_field_data_t)); \
            } else if( attr_id >= SAI_ACL_ENTRY_ATTR_FIELD_START && \
                       attr_id <= SAI_ACL_ENTRY_ATTR_FIELD_END )    \
            {                                                       \
                if (SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6 == attr_id ||  \
                    SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6 == attr_id ||  \
                    SAI_ACL_ENTRY_ATTR_FIELD_IPv6_FLOW_LABEL == attr_id || \
                    SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL == attr_id)     \
                {                                                     \
                    p_sdb_entry->outer_fields.is_ipv6 = TRUE;         \
                }                                                     \
                else if (SAI_ACL_ENTRY_ATTR_FIELD_IPV4_PACKET == attr_id)\
                {                                                       \
                    p_sdb_entry->outer_fields.is_ipv4_packet = TRUE;    \
                }                                                       \
                if (SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL != attr_id &&    \
                    SAI_ACL_ENTRY_ATTR_FIELD_IPV4_PACKET != attr_id )   \
                {                                                     \
                    sal_memcpy(&p_sdb_entry->outer_fields.member, &attr_stage->value.aclfield, sizeof(sai_acl_field_data_t)); \
                }                                                   \
            }                                                       \
        }                                                           \
    }

#define __ACL_GET_FILED_ATTR_ACTION(attr_id, member)                \
    {                                                               \
        attr_stage = ctc_sai_attr_get_attr_by_id(attr_id,           \
                                                attr_list,          \
                                                attr_count,         \
                                                NULL);              \
                                                                    \
        if(attr_stage){                                             \
            if( attr_id >= SAI_ACL_ENTRY_ATTR_ACTION_START &&       \
                attr_id <= SAI_ACL_ENTRY_ATTR_ACTION_END ){         \
                sal_memcpy(                                         \
                    &p_sdb_entry->action.member,                 \
                    &attr_stage->value.aclaction,                   \
                    sizeof(sai_acl_action_data_t));                 \
            }                                                       \
        }                                                           \
    }

#define __ACL_GET_ATTR(attr_id, member)                             \
    {                                                               \
        attr_stage = ctc_sai_attr_get_attr_by_id(attr_id,           \
                                                attr_list,          \
                                                attr_count,         \
                                                NULL);              \
                                                                    \
        if(attr_stage){                                             \
                sal_memcpy(                                         \
                    &p_sdb_entry->member,                        \
                    &attr_stage->value.aclfield,                    \
                    sizeof(sai_acl_field_data_t));                  \
        }                                                           \
    }

    const sai_attribute_t         *attr_stage         = NULL;

    __ACL_GET_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT,        in_port);

    /* outer filed */
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC,                  ether_macsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC,                  ether_macda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP,                   ipv4_ipsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_DST_IP,                   ipv4_ipda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6,                 ipv6_ipsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6,                 ipv6_ipda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IPv6_FLOW_LABEL,          ipv6_flow_label);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID,            ether_svlan);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID,            ether_cvlan);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI,           ether_svlan_pri);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI,           ether_svlan_cfi);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI,           ether_cvlan_pri);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI,           ether_cvlan_cfi);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_UNTAG_VLAN,               ether_untag_vlan);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT,              ipv4_src_port);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT,              ipv4_dst_port);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,               ether_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL,              ipv4_ip_protocol);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IP_PRECEDENCE,            ipv4_ip_precedence);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_DSCP,                     ipv4_dscp);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS,                 ipv4_ip_frag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS,                ipv4_tcp_flag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL,                 is_ipv6);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IPV4_PACKET,              is_ipv4_packet);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG,                  ipv4_ip_frag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE,                ipv4_icmp_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE,                ipv4_icmp_code);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IGMP_TYPE,                ipv4_igmp_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_IP_OPTIONS,               ipv4_ip_options);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ROUTED_PKT,               ipv4_routed_packet);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_VXLAN_VNI,                ipv4_vxlan_vni);
    /*added by yejl for bug54032,54049, 2019-10-22*/
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_VXLAN_VNI_DSTPORT,        ipv4_vxlan_vni_dst_port);

    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_GRE_TYPE,                 gre_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_GRE_KEY,                  gre_key);

    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_UDF_ID,                   udf_id);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_UDF_TYPE,                 udf_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_UDF_MATCH,                udf_match_data);
	__ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ERSPAN_ID,                erspan_id);

    /* arp field */
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ARP_OP_TYPE,              arp_op_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ARP_SENDER_IP,            arp_sender_ip);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_ARP_TARGET_IP,            arp_target_ip);

    /* inner filed */
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_MAC,            ether_macsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_MAC,            ether_macda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP,             ipv4_ipsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP,             ipv4_ipda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPv6,           ipv6_ipsa);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPv6,           ipv6_ipda);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IPv6_FLOW_LABEL,    ipv6_flow_label);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_OUTER_VLAN_ID,      ether_svlan);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_INNER_VLAN_ID,      ether_cvlan);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_OUTER_VLAN_PRI,     ether_svlan_pri);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_OUTER_VLAN_CFI,     ether_svlan_cfi);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_INNER_VLAN_PRI,     ether_cvlan_pri);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_INNER_VLAN_CFI,     ether_cvlan_cfi);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_SRC_PORT,        ipv4_src_port);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_DST_PORT,        ipv4_dst_port);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE,         ether_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL,        ipv4_ip_protocol);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PRECEDENCE,      ipv4_ip_precedence);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_DSCP,               ipv4_dscp);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_FLAGS,           ipv4_ip_frag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_TCP_FLAGS,          ipv4_tcp_flag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_FRAG,            ipv4_ip_frag);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_ICMP_TYPE,          ipv4_icmp_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_ICMP_CODE,          ipv4_icmp_code);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IGMP_TYPE,          ipv4_igmp_type);
    __ACL_GET_FILED_ATTR(SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_OPTIONS,         ipv4_ip_options);


    /* action */
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_PACKET_ACTION,                  action);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_COUNTER,                 counter);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS,          mirror_ingress);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS,           mirror_egress);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_COLOR,               set_color);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER,             set_policer);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,                redirect);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_VLAN_ID,    redirect_vlan_id);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_UNTAG,      redirect_vlan_untag);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_TRUNCATION,          trunction);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC,             src_mac);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC,             dst_mac);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP,              src_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP,              dst_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPv6,            src_ipv6);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPv6,            dst_ipv6);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER,        strip_header);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER_POS,    strip_header_pos);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER_OFFSET, strip_header_offset);
	__ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_INNER_VXLAN_HEADER,        erspan_strip_inner_vxlan_header);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID,       outer_vlan);
	__ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID_ACTION,outer_vlan_action);

    /*added by yejl for bug 53475, 2019-09-10*/
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_HEADER,    add_l2gre_header);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_SRC_IP,    l2gre_src_ip);
	__ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_IP,    l2gre_dst_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_MAC,   l2gre_dst_mac);
	__ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_NUM,   l2gre_key_num);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_LEN,   l2gre_key_len);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_HEADER,    add_l3gre_header);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_SRC_IP,    l3gre_src_ip);
	__ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_IP,    l3gre_dst_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_MAC,   l3gre_dst_mac);
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_HEADER,    add_vxlan_header);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_MAC,    vxlan_dst_mac);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRC_IP,    vxlan_src_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_IP,   vxlan_dst_ip);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRCPORT,   vxlan_srcport);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DESTPORT,   vxlan_dstport);
    __ACL_GET_FILED_ATTR_ACTION(SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_VNI,   vxlan_vni);
    /*ended by guoxd*/

	if(p_sdb_entry->outer_fields.erspan_id.enable)
    {
        p_sdb_entry->apply_inner_lookup = true;
        p_sdb_entry->outer_fields.udf_match_data.enable = true;
		p_sdb_entry->outer_fields.udf_match_data.data.objlist.list[0] |= 0x88BE0000;
        p_sdb_entry->outer_fields.udf_match_data.data.objlist.list[1] &= 0x0000FFFF;

        if(p_sdb_entry->action.erspan_strip_inner_vxlan_header.enable)
        {
            p_sdb_entry->inner_fields.ipv4_dst_port.enable       = true;
            p_sdb_entry->inner_fields.ipv4_vxlan_vni_dst_port.enable = true;/*added by yejl for bug54032,54049, 2019-10-22*/
            p_sdb_entry->inner_fields.ipv4_dst_port.data.u16    = 0;
            p_sdb_entry->inner_fields.ipv4_dst_port.mask.u16  = 0xFFFF;
            p_sdb_entry->inner_fields.ipv4_ip_protocol.enable   = true;
            p_sdb_entry->inner_fields.ipv4_ip_protocol.data.u8  = 17;
            p_sdb_entry->inner_fields.ipv4_ip_protocol.mask.u8 = 0;
            p_sdb_entry->inner_fields.ipv4_ipsa.enable   = true;
            p_sdb_entry->inner_fields.ipv4_ipsa.data.u32  =0x0;
            p_sdb_entry->inner_fields.ipv4_ipsa.mask.u32 = 0xFFFFFFFF;
        }
    }
}


void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* L2 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_macsa(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_macda(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ovlan(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ovlan_cos(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ivlan(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ivlan_cos(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_untag_vlan(field_list, p_sdb_entry, res);/*added by yejl for bug 49791, 2018-11-14*/
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_type(field_list, p_sdb_entry, res);
}


void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* L3 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ipsa(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ipda(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_flow_label(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_protocol(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_dscp(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_precedence(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_frag(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_igmp_type(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_icmp_type(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_icmp_code(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_options(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_routed_packet(field_list, p_sdb_entry, res);
}


void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* L3 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer_src_port(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer_dst_port(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l4_vxlan_vni(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_l4_tcp_flags(field_list, p_sdb_entry, res);
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* L3 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_op_type(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_sender_ip(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_target_ip(field_list, p_sdb_entry, res);
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* convert field */

    if (field_list->is_ipv6)
    {
        res->is_ipv6 = TRUE;
    }

    if (field_list->is_ipv4_packet)
    {
        field_list->ether_type.enable = TRUE;
        field_list->ether_type.data.u16 = 0x0800;
        field_list->ether_type.mask.u16 = 0x0;
        __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_type(field_list, p_sdb_entry, res);
    }
    
    /* L2 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer(field_list, p_sdb_entry, res);

    /* L3 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer(field_list, p_sdb_entry, res);

    /* L4 layer */
    __mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer(field_list, p_sdb_entry, res);
    /* convert action */

    __mapping_sai_acl_entry_to_ctc_acl_entry_udf_type(field_list, p_sdb_entry, res);
    __mapping_sai_acl_entry_to_ctc_acl_entry_udf_match(field_list, p_sdb_entry, res);

    /* arp key mapping */
    __mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer(field_list, p_sdb_entry, res);
}

sai_status_t
_ctc_sai_mirror_mapping_func(stbl_acl_entry_t* p_sdb_entry, sai_uint32_t session_id, sai_uint32_t dir)
{
    ctc_sai_mirror_entry_t* p_session = NULL;
    ctc_mirror_dest_t mirror_dest;
    uint8 tid = 0;
    int32 ret = SAI_STATUS_SUCCESS;
    
    p_session = ctc_sai_mirror_entry_get(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, session_id));
    if (NULL == p_session)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
    mirror_dest.session_id = CTC_SAI_OBJECT_INDEX_GET(p_session->sid);
    mirror_dest.dir = dir;
    mirror_dest.type = CTC_MIRROR_ACLLOG_SESSION;
    if (p_session->monitor_type == SAI_MIRROR_TYPE_LOCAL)
    {
        if (p_session->is_multi_dest_mirror)
        {
            mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
            mirror_dest.is_rspan = TRUE;
            mirror_dest.vlan_valid = FALSE; 
        }
        else
        {
            /* port */
            if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
            {
                mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
            }
            /* agg */
            else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
            {
                tid = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                mirror_dest.dest_gport = CTC_MAP_TID_TO_GPORT(tid);
            }
            /*flow mirror*/
            else if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
            {
                mirror_dest.dest_gport = CTC_LPORT_CPU;
            }
            else
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
        }
    }
    else if (p_session->monitor_type == SAI_MIRROR_TYPE_REMOTE)
    {            
        mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
        mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
        mirror_dest.is_rspan = TRUE;
        mirror_dest.vlan_valid = FALSE;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    ret = ctc_mirror_add_session(&mirror_dest);
    if (ret)
    {
        return ret;
    }
    p_sdb_entry->mirror_oid = p_session->sid;
    return ret;
}


#ifdef USW
sai_status_t sai_create_acl_entry_duet2(
    _Out_ sai_object_id_t* acl_entry_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    stbl_acl_entry_t    sai_entry;
    stbl_acl_entry_t    *p_sdb_entry = NULL;
    stbl_acl_tbl_t      *p_sdb_tbl = NULL;
    sai_attribute_t const *pattr_entry = NULL;
    sai_attribute_t *attr_list_db = NULL;
    sai_object_id_t port_id = 0;
    uint32 gport = 0;
    sai_uint32_t group_id = 0;
    sai_uint32_t index = 0;
    sai_uint32_t acl_entry_index = 0;
    sai_uint32_t dir = 0;
    sai_uint32_t session_id = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool is_worm_filter = FALSE;
#ifdef TAPPRODUCT
    ctc_sai_tap_group_edit_group_key    key;
#endif
	ctc_sai_acl_tunnel_key_t tunnel_decap_key;
    fields_mapping_res_t    aclentry = {FIELD_ACL_ENTRY_TYPE};

    CTC_SAI_PTR_VALID_CHECK(acl_entry_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    CTC_SAI_DEBUG_FUNC();
    sai_create_acl_entry_debug_param(attr_count, attr_list);

    sal_memset(&aclentry, 0, sizeof(fields_mapping_res_t));
    sal_memset(&sai_entry, 0, sizeof(sai_entry));

    aclentry.mapping_type = FIELD_ACL_ENTRY_TYPE;

    /* 1. get acl table */
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_TABLE_ID, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.tbl_oid = pattr_entry->value.oid;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(sai_entry.tbl_oid);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 2. get acl priority */
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_PRIORITY, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.priority = pattr_entry->value.u32;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    /* 3. get acl port and direction*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        port_id = pattr_entry->value.aclfield.data.oid;
        dir = CTC_INGRESS;
    }
    else
    {
        pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, attr_list, attr_count, &index);
        if (NULL != pattr_entry)
        {
            port_id = pattr_entry->value.aclfield.data.oid;
            dir = CTC_EGRESS;
        }
        else
        {
            pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, attr_list, attr_count, &index);
            if (NULL != pattr_entry)
            {
                dir = CTC_INGRESS;
            }
            else
            {
                pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, attr_list, attr_count, &index);
                if (NULL != pattr_entry)
                {
                    dir = CTC_EGRESS;
                }
            }
        }
    }

    /*4. is worm filter or not*/
    if (port_id && dir == CTC_INGRESS)
    {
        if (SAI_OBJECT_TYPE_NULL == CTC_SAI_OBJECT_TYPE_GET(port_id)
            && (0 == (port_id & 0xffff)))
        {
            is_worm_filter = TRUE;
        }
    }
    
    /*5. mapping parameter */
    __convert_attr_list_to_sai_acl_entry(attr_count, attr_list, &sai_entry);

    /*6.  alloc offset */
    ctclib_opb_alloc_offset(&p_acl_glb->acl_entry_opb, &acl_entry_index);
    sai_entry.key.entry_id = acl_entry_index;
    stbl_acl_entry_add_acl_entry(&sai_entry);
    p_sdb_entry = stbl_acl_entry_get_acl_entry(&sai_entry.key);
    if (NULL == p_sdb_entry)
    {
        ctclib_opb_free_offset(&p_acl_glb->acl_entry_opb, acl_entry_index);
        return SAI_STATUS_NO_MEMORY;
    }

    /*7. add acl entry*/
    if (is_worm_filter)
    {
        ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
        aclentry.acl.entry_id = index;
        aclentry.acl.priority = index + SAI_ACL_WORM_FILTER_PRIORITY_BASE;
        aclentry.acl.priority_valid = TRUE;
        aclentry.acl.mode = 1;
        aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV4;
        ret = ctc_acl_add_entry(SAI_ACL_WORM_FILTER_GROUP, &aclentry.acl);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    }
    else if (port_id && !is_worm_filter)
    {
        if (sai_entry.apply_inner_lookup)
        {
            if (sai_entry.inner_fields.is_ipv6)
            {
                aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV6;
            }
            else
            {
                aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV4;
                #ifdef TAPPRODUCT
                if (dir == CTC_INGRESS)
                {
                    aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV4_EXT;
                }
                #endif
            }
        }
        else
        {
            if (sai_entry.outer_fields.is_ipv6)
            {
                aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV6;
            }
            else
            {
                aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV4;
                #ifdef TAPPRODUCT
                if (dir == CTC_INGRESS)
                {
                    aclentry.acl.key_type = CTC_ACL_KEY_MAC_IPV4_EXT;
                }
                #endif
            }
        }

        ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
        ctc_sai_port_objectid_to_gport(port_id, &gport);
        group_id = ctc_sai_port_make_acl_group_id(dir, gport);
        aclentry.acl.entry_id = index;
        aclentry.acl.priority = sai_entry.priority;
        aclentry.acl.priority_valid = TRUE; 
        aclentry.acl.mode = 1;

        if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
        {
            ctc_sai_acl_get_copp_group_id (port_id, &group_id);
        }

        ret = ctc_acl_add_entry(group_id, &aclentry.acl);
        if (CTC_E_NONE != ret)
        {     
#ifdef TAPPRODUCT 
            if (p_sdb_entry->tap.trunction_en)
            {
                ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                p_sdb_entry->tap.trunction_en = FALSE;
            } 
#endif
            goto out;
        }
    }

    /*8. get/set the acl entry field*/
    if (sai_entry.apply_inner_lookup)
    {
        __mapping_sai_acl_entry_to_ctc_acl_entry(&sai_entry.inner_fields, &sai_entry, &aclentry);

        /* TODO */
        sal_memcpy(&tunnel_decap_key.fields, &sai_entry.outer_fields, sizeof(ctc_sai_acl_field_list_t));
        tunnel_decap_key.port_oid = port_id;
        sai_entry.tap.ptunnel_info = __tunnel_decap_alloc(&tunnel_decap_key, &sai_entry);

        /* save SCL tunnel info in stbl fix delete scl */
        p_sdb_entry->tap.ptunnel_info = sai_entry.tap.ptunnel_info;
    }
    else
    {
        __mapping_sai_acl_entry_to_ctc_acl_entry(&sai_entry.outer_fields, &sai_entry, &aclentry);
    }

	/* 9 check strip-header */
    /* if inner lookup is invaild, then use acl to strip-packet */
    if (!sai_entry.apply_inner_lookup)
    {
        if (0 != __check_tunnel_header_valid_and_config(&sai_entry.outer_fields, &aclentry))
        {
            goto out;
        }
    }
    __connect_acl2scl(sai_entry.tap.ptunnel_info, &sai_entry, port_id, &aclentry);

    /*10. get the acl action*/
    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_ACL_ENTRY_ATTR_ACTION_START <= attr_list[index].id) && (SAI_ACL_ENTRY_ATTR_ACTION_END >= attr_list[index].id))
        {
            if(NULL == g_sai_acl_info.g_acl_action[attr_list[index].id-SAI_ACL_ENTRY_ATTR_ACTION_START])
            {
                continue;
            }
            
            ret = g_sai_acl_info.g_acl_action[attr_list[index].id-SAI_ACL_ENTRY_ATTR_ACTION_START]
                (&aclentry.acl, &attr_list[index], p_sdb_entry);
            if (ret)
            {
                goto out;
            }
            /* save entry info in db */
            attr_list_db = ctc_sai_acl_get_action_attribute(p_sdb_entry, attr_list[index].id, FALSE);
            sal_memcpy(attr_list_db, &attr_list[index], sizeof(sai_attribute_t));
        }
    }
    
    /*11. creat acl mirror session*/
    if (CTC_FLAG_ISSET(aclentry.acl.action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG))
    {
        session_id = aclentry.acl.action.log_session_id;
        ret = _ctc_sai_mirror_mapping_func(p_sdb_entry, session_id, dir);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    }

#ifdef TAPPRODUCT
    if (p_sdb_entry->tap.ptap_info)
    {
        uint32_t    session_id = 0;
        uint32_t    nexthop_id = 0;
        ctc_acl_field_action_t action_field;
        sal_memset(&action_field, 0, sizeof(action_field));
        
        CTC_SAI_DEBUG("%s%d:tap_info: g_tap_truncation = %d\n",
            __FUNCTION__,__LINE__, p_sdb_entry->tap.trunction_en);
            
        if (p_sdb_entry->tap.trunction_en)
        { 
            ctc_sai_tap_group_get_session_id(p_sdb_entry->tap.tap_group_id,&session_id);

            aclentry.acl.action.flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
            aclentry.acl.action.log_session_id = session_id;
            aclentry.acl.action.log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;

            action_field.type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;
            action_field.data0 = session_id;
            action_field.data1 = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
            (void)ctc_acl_add_action_field(aclentry.acl.entry_id, &action_field);

            p_sdb_entry->tap.trunction_en = TRUE;
        }
        else
        {
            if (p_sdb_entry->tap.ipda_en || p_sdb_entry->tap.ipsa_en || 
                p_sdb_entry->tap.ipv6da_en || p_sdb_entry->tap.ipv6sa_en || 
                p_sdb_entry->tap.macda_en || p_sdb_entry->tap.macsa_en ||
                p_sdb_entry->action.outer_vlan.enable || 
                p_sdb_entry->tap.add_l2gre_header || p_sdb_entry->tap.add_l3gre_header || p_sdb_entry->tap.vxlan_add_header)
            {
                sal_memset(&key, 0, sizeof(ctc_sai_tap_group_edit_group_key));

                if ((p_sdb_entry->tap.strip_header &&
                    (p_sdb_entry->outer_fields.gre_type.enable && 
                    SAI_ACL_GRE_GRE == p_sdb_entry->outer_fields.gre_type.data.u32) &&
                    p_sdb_entry->outer_fields.erspan_id.enable == FALSE) || 
                    (p_sdb_entry->tap.strip_header && p_sdb_entry->outer_fields.ipv4_ip_protocol.data.u8 == 4))
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR);
                }
                else
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                    /*added by yejl for bug 53475, 2019-09-10*/
                    if (p_sdb_entry->tap.add_l2gre_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR);
                    }
                    else if (p_sdb_entry->tap.add_l3gre_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR);
                    }
                    /*ended by yejl*/

                    /*added by guoxd for vxlan*/
                    else if (p_sdb_entry->tap.vxlan_add_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR);
                    }
                    /*ended by guoxd*/
                }

                /*added by yejl for bug 53475, 2019-09-10*/
                if (p_sdb_entry->tap.add_l2gre_header)
                {
                    key.l2gre_ip_sip.addr.ip4 = p_sdb_entry->tap.l2gre_ipsa;
                    key.l2gre_ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.l2gre_ip_dip.addr.ip4 = p_sdb_entry->tap.l2gre_ipda;
                    key.l2gre_ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    sal_memcpy(key.l2gre_mac_da, p_sdb_entry->tap.l2gre_macda, sizeof(sai_mac_t));

                    key.l2gre_key_num = p_sdb_entry->tap.l2gre_key_num;
                    key.l2gre_key_len = p_sdb_entry->tap.l2gre_key_len;
                }

                if (p_sdb_entry->tap.add_l3gre_header)
                {
                    key.l3gre_ip_sip.addr.ip4 = p_sdb_entry->tap.l3gre_ipsa;
                    key.l3gre_ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.l3gre_ip_dip.addr.ip4 = p_sdb_entry->tap.l3gre_ipda;
                    key.l3gre_ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    sal_memcpy(key.l3gre_mac_da, p_sdb_entry->tap.l3gre_macda, sizeof(sai_mac_t));
                }
                /*ended by yejl*/

                /*added by guoxd for vxlan*/
                if (p_sdb_entry->tap.vxlan_add_header)
                {
                    sal_memcpy(key.vxlan_mac_da, p_sdb_entry->tap.vxlan_macda, sizeof(sai_mac_t));
                    
                    key.vxlan_ip_sa.addr.ip4 = p_sdb_entry->tap.vxlan_ipsa;
                    key.vxlan_ip_sa.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.vxlan_ip_da.addr.ip4 = p_sdb_entry->tap.vxlan_ipda;
                    key.vxlan_ip_da.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.vxlan_port_src = p_sdb_entry->tap.vxlan_portsrc;
                    key.vxlan_port_dst = p_sdb_entry->tap.vxlan_portdst;
                    key.vxlan_vni_set = p_sdb_entry->tap.vxlan_vni;
                }
                /*ended by guoxd*/
                
                if (p_sdb_entry->tap.ipda_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DIP);
                    key.ip_dip.addr.ip4 = p_sdb_entry->tap.ipda;
                    key.ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                }
                if (p_sdb_entry->tap.ipsa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SIP);
                    key.ip_sip.addr.ip4 = p_sdb_entry->tap.ipsa;
                    key.ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                }
                if (p_sdb_entry->tap.ipv6da_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DIP);
                    sal_memcpy(&key.ip_dip.addr.ip6, &p_sdb_entry->tap.ipv6da, sizeof(sai_ip6_t));
                    key.ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                }
                if (p_sdb_entry->tap.ipv6sa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SIP);
                    sal_memcpy(&key.ip_sip.addr.ip6, &p_sdb_entry->tap.ipv6sa, sizeof(sai_ip6_t));
                    key.ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                }
                
                if (p_sdb_entry->tap.macda_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DMAC);
                    sal_memcpy(key.mac_dest, p_sdb_entry->tap.macda, sizeof(sai_mac_t));
                }
                if (p_sdb_entry->tap.macsa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SMAC);
                    sal_memcpy(key.mac_src, p_sdb_entry->tap.macsa, sizeof(sai_mac_t));
                }
                if (p_sdb_entry->action.outer_vlan.enable)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_VLAN);
					if(p_sdb_entry->action.outer_vlan_action.enable)
					{
						switch(p_sdb_entry->action.outer_vlan_action.parameter.s32)
                    	{
						case SAI_ACL_VLAN_ACTION_ADD:
							SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_ADD);
							break;
						case SAI_ACL_VLAN_ACTION_ADD_OR_REPLACE:
							SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_REP_OR_ADD);
							break;
						case SAI_ACL_VLAN_ACTION_REPLACE:
						default:
							break;
						}
					}
					
                    key.vlan = p_sdb_entry->action.outer_vlan.parameter.u16;
                }
                p_sdb_entry->tap.edit_group = ctc_sai_tap_group_get_edit_group(key, p_sdb_entry->tap.ptap_info);
                
                if (NULL == p_sdb_entry->tap.edit_group)
                {
                    return SAI_STATUS_FAILURE;
                }
                nexthop_id = ((ctc_sai_tap_group_edit_group*)p_sdb_entry->tap.edit_group)->nexthop_id;
            }
            else
            {
                ctc_sai_tap_group_get_nexthop_id(p_sdb_entry->tap.tap_group_id, &nexthop_id);
            }
            aclentry.acl.action.flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
            aclentry.acl.action.nh_id = nexthop_id;

            action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
            action_field.data0 = CTC_SAI_OBJECT_INDEX_GET(nexthop_id);
            (void)ctc_acl_add_action_field(aclentry.acl.entry_id, &action_field);
        }
    }
#endif /* TAPPRODUCT */

    /*11. install acl entry*/
    if (is_worm_filter)
    {
        ret = ctc_acl_install_entry(aclentry.acl.entry_id);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    
        p_sdb_entry->enable = TRUE;
        p_sdb_entry->ctc_entry_id_list[0] = aclentry.acl.entry_id;
        p_sdb_entry->ctc_entry_id_count = 1;
        p_sdb_entry->dirction = CTC_BOTH_DIRECTION;
        p_sdb_entry->port_oid_list[0] = 0;
        p_sdb_entry->port_oid_count = 1;
        p_sdb_entry->priority = sai_entry.priority;
        *acl_entry_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_ENTRY, p_sdb_entry->key.entry_id);
        return ret;
    }
    else if (port_id && !is_worm_filter)
    {
        ret = ctc_acl_install_entry(aclentry.acl.entry_id);
        if (CTC_E_NONE != ret)
        {
            ctc_acl_remove_entry(aclentry.acl.entry_id);
#ifdef TAPPRODUCT             
            if (p_sdb_entry->tap.trunction_en)
            {
                ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                p_sdb_entry->tap.trunction_en = FALSE;
            }
#endif
            goto out;
        }
    
        p_sdb_entry->enable = TRUE;
        p_sdb_entry->ctc_entry_id_list[0] = aclentry.acl.entry_id;
        p_sdb_entry->ctc_entry_id_count = 1;
        p_sdb_entry->port_oid_list[0] = port_id;
        p_sdb_entry->port_oid_count = 1;
        p_sdb_entry->dirction = dir;
        p_sdb_entry->priority = sai_entry.priority;
        *acl_entry_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_ENTRY, p_sdb_entry->key.entry_id);
        return ret;
    }
    return SAI_STATUS_SUCCESS;
    
out:
    if (p_sdb_entry)
    {
        ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, index);
        ctclib_opb_free_offset(&p_acl_glb->acl_entry_opb, acl_entry_index);
        stbl_acl_entry_del_acl_entry(&p_sdb_entry->key);
    }

    return ret;
}
#else
sai_status_t sai_create_acl_entry_gg_gb(
    _Out_ sai_object_id_t* acl_entry_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    stbl_acl_entry_t    sai_entry;
    stbl_acl_entry_t    *p_sdb_entry = NULL;
    stbl_acl_tbl_t      *p_sdb_tbl = NULL;
    sai_attribute_t const *pattr_entry = NULL;
    sai_attribute_t *attr_list_db = NULL;
    sai_object_id_t port_id = 0;
    uint32 gport = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_uint32_t group_id = 0;
    sai_uint32_t index = 0;
    sai_uint32_t acl_entry_index = 0;
    sai_uint32_t dir = 0;
    sai_uint32_t session_id = 0;
    sai_uint32_t i = 0;
    bool is_worm_filter = FALSE;
#ifdef TAPPRODUCT
    ctc_sai_tap_group_edit_group_key    key;
#endif
	ctc_sai_acl_tunnel_key_t tunnel_decap_key;
    fields_mapping_res_t    aclentry = {FIELD_ACL_ENTRY_TYPE};

    CTC_SAI_PTR_VALID_CHECK(acl_entry_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    CTC_SAI_DEBUG_FUNC();
    sai_create_acl_entry_debug_param(attr_count, attr_list);

    sal_memset(&aclentry, 0, sizeof(fields_mapping_res_t));
    sal_memset(&sai_entry, 0, sizeof(sai_entry));

    aclentry.mapping_type = FIELD_ACL_ENTRY_TYPE;

    /* 1. get acl table */
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_TABLE_ID, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.tbl_oid = pattr_entry->value.oid;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(sai_entry.tbl_oid);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 2. get acl priority */
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_PRIORITY, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.priority = pattr_entry->value.u32;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    /* 3. get acl port and direction*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        port_id = pattr_entry->value.aclfield.data.oid;
        dir = CTC_INGRESS;
    }
    else
    {
        pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, attr_list, attr_count, &index);
        if (NULL != pattr_entry)
        {
            port_id = pattr_entry->value.aclfield.data.oid;
            dir = CTC_EGRESS;
        }
        else
        {
            pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, attr_list, attr_count, &index);
            if (NULL != pattr_entry)
            {
                dir = CTC_INGRESS;
            }
            else
            {
                pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, attr_list, attr_count, &index);
                if (NULL != pattr_entry)
                {
                    dir = CTC_EGRESS;
                }
            }
        }
    }

    /*4. is worm filter or not*/
    if (port_id && dir == CTC_INGRESS)
    {
        if (SAI_OBJECT_TYPE_NULL == CTC_SAI_OBJECT_TYPE_GET(port_id)
            && (0 == (port_id & 0xffff)))
        {
            is_worm_filter = TRUE;
        }
    }
    
    /*5. mapping parameter */
    __convert_attr_list_to_sai_acl_entry(attr_count, attr_list, &sai_entry);

    /*6. set/get the acl entry field*/
    if (sai_entry.apply_inner_lookup)
    {
        __mapping_sai_acl_entry_to_ctc_acl_entry(&sai_entry.inner_fields, &sai_entry, &aclentry);

        /* TODO */
        sal_memcpy(&tunnel_decap_key.fields, &sai_entry.outer_fields, sizeof(ctc_sai_acl_field_list_t));
        tunnel_decap_key.port_oid = port_id;
        sai_entry.tap.ptunnel_info = __tunnel_decap_alloc(&tunnel_decap_key, &sai_entry);
    }
    else
    {
        __mapping_sai_acl_entry_to_ctc_acl_entry(&sai_entry.outer_fields, &sai_entry, &aclentry);
    }

	/*7. check strip-header */
    /* if inner lookup is invaild, then use acl to strip-packet */
    if (!sai_entry.apply_inner_lookup)
    {
        if (0 != __check_tunnel_header_valid_and_config(&sai_entry.outer_fields, &aclentry))
        {
            return SAI_STATUS_FAILURE;
        }
    }
    __connect_acl2scl(sai_entry.tap.ptunnel_info, &sai_entry, port_id, &aclentry);

    /*8. alloc offset */
    ctclib_opb_alloc_offset(&p_acl_glb->acl_entry_opb, &acl_entry_index);
    sai_entry.key.entry_id = acl_entry_index;
    stbl_acl_entry_add_acl_entry(&sai_entry);
    p_sdb_entry = stbl_acl_entry_get_acl_entry(&sai_entry.key);
    if (NULL == p_sdb_entry)
    {
        ctclib_opb_free_offset(&p_acl_glb->acl_entry_opb, acl_entry_index);
        return SAI_STATUS_NO_MEMORY;
    }

    /*9. get the acl action*/
    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_ACL_ENTRY_ATTR_ACTION_START <= attr_list[index].id) && (SAI_ACL_ENTRY_ATTR_ACTION_END >= attr_list[index].id))
        {
            if(NULL == g_sai_acl_info.g_acl_action[attr_list[index].id-SAI_ACL_ENTRY_ATTR_ACTION_START]){
                continue;
            }
            
            ret = g_sai_acl_info.g_acl_action[attr_list[index].id-SAI_ACL_ENTRY_ATTR_ACTION_START]
                (&aclentry.acl, &attr_list[index], p_sdb_entry);
            if (ret)
            {
                goto out;
            }
            /* save entry info in db */
            attr_list_db = ctc_sai_acl_get_action_attribute(p_sdb_entry, attr_list[index].id, FALSE);
            sal_memcpy(attr_list_db, &attr_list[index], sizeof(sai_attribute_t));
        }
    }
    
#ifdef GREATBELT
    /*for gb ingress deny learning*/
    if (CTC_FLAG_ISSET(aclentry.acl.action.flag, CTC_ACL_ACTION_FLAG_DISCARD) && CTC_INGRESS == dir)
    {
        CTC_SET_FLAG(aclentry.acl.action.flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING);
    }
#endif

    /*10. creat acl mirror session*/
    if (CTC_FLAG_ISSET(aclentry.acl.action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG))
    {
        session_id = aclentry.acl.action.log_session_id;
        ret = _ctc_sai_mirror_mapping_func(p_sdb_entry, session_id, dir);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    }
    
#ifdef TAPPRODUCT
    if (p_sdb_entry->tap.ptap_info)
    {
        uint32_t    session_id = 0;
        uint32_t    nexthop_id = 0;
        
        CTC_SAI_DEBUG("%s%d:tap_info: g_tap_truncation = %d\n",
            __FUNCTION__,__LINE__, p_sdb_entry->tap.trunction_en);
            
        if (p_sdb_entry->tap.trunction_en)
        { 
            ctc_sai_tap_group_get_session_id(p_sdb_entry->tap.tap_group_id,&session_id);

            aclentry.acl.action.flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
            aclentry.acl.action.log_session_id = session_id;
            aclentry.acl.action.log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;

            p_sdb_entry->tap.trunction_en = TRUE;
        }
        else
        {
            if (p_sdb_entry->tap.ipda_en || p_sdb_entry->tap.ipsa_en || 
                p_sdb_entry->tap.ipv6da_en || p_sdb_entry->tap.ipv6sa_en || 
                p_sdb_entry->tap.macda_en || p_sdb_entry->tap.macsa_en ||
                p_sdb_entry->action.outer_vlan.enable || 
                p_sdb_entry->tap.add_l2gre_header || p_sdb_entry->tap.add_l3gre_header ||
                p_sdb_entry->tap.vxlan_add_header)
            {
                sal_memset(&key, 0, sizeof(ctc_sai_tap_group_edit_group_key));

                if ((p_sdb_entry->tap.strip_header &&
                    (p_sdb_entry->outer_fields.gre_type.enable && 
                    SAI_ACL_GRE_GRE == p_sdb_entry->outer_fields.gre_type.data.u32) &&
                    p_sdb_entry->outer_fields.erspan_id.enable == FALSE) || 
                    (p_sdb_entry->tap.strip_header && p_sdb_entry->outer_fields.ipv4_ip_protocol.data.u8 == 4))
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR);
                }
                else
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                    /*added by yejl for bug 53475, 2019-09-10*/
                    if (p_sdb_entry->tap.add_l2gre_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR);
                    }
                    else if (p_sdb_entry->tap.add_l3gre_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR);
                    }
                    /*ended by yejl*/

                    /*added by guoxd for vxlan*/
                    else if (p_sdb_entry->tap.vxlan_add_header)
                    {
                        SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR);
                    }
                    /*ended by guoxd*/
                }

                /*added by yejl for bug 53475, 2019-09-10*/
                if (p_sdb_entry->tap.add_l2gre_header)
                {
                    key.l2gre_ip_sip.addr.ip4 = p_sdb_entry->tap.l2gre_ipsa;
                    key.l2gre_ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.l2gre_ip_dip.addr.ip4 = p_sdb_entry->tap.l2gre_ipda;
                    key.l2gre_ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    sal_memcpy(key.l2gre_mac_da, p_sdb_entry->tap.l2gre_macda, sizeof(sai_mac_t));

                    key.l2gre_key_num = p_sdb_entry->tap.l2gre_key_num;
                    key.l2gre_key_len = p_sdb_entry->tap.l2gre_key_len;
                }

                if (p_sdb_entry->tap.add_l3gre_header)
                {
                    key.l3gre_ip_sip.addr.ip4 = p_sdb_entry->tap.l3gre_ipsa;
                    key.l3gre_ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.l3gre_ip_dip.addr.ip4 = p_sdb_entry->tap.l3gre_ipda;
                    key.l3gre_ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    sal_memcpy(key.l3gre_mac_da, p_sdb_entry->tap.l3gre_macda, sizeof(sai_mac_t));
                }
                /*ended by yejl*/

                /*added by guoxd for vxlan*/
                if (p_sdb_entry->tap.vxlan_add_header)
                {
                    sal_memcpy(key.vxlan_mac_da, p_sdb_entry->tap.vxlan_macda, sizeof(sai_mac_t));

                    key.vxlan_ip_sa.addr.ip4 = p_sdb_entry->tap.vxlan_ipsa;
                    key.vxlan_ip_sa.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.vxlan_ip_da.addr.ip4 = p_sdb_entry->tap.vxlan_ipda;
                    key.vxlan_ip_da.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    key.vxlan_port_src = p_sdb_entry->tap.vxlan_portsrc;
                    key.vxlan_port_dst = p_sdb_entry->tap.vxlan_portdst;
                    key.vxlan_vni_set = p_sdb_entry->tap.vxlan_vni;
                }
                /*ended by guoxd*/
                
                if (p_sdb_entry->tap.ipda_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DIP);
                    key.ip_dip.addr.ip4 = p_sdb_entry->tap.ipda;
                    key.ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                }
                if (p_sdb_entry->tap.ipsa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SIP);
                    key.ip_sip.addr.ip4 = p_sdb_entry->tap.ipsa;
                    key.ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                }
                if (p_sdb_entry->tap.ipv6da_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DIP);
                    sal_memcpy(&key.ip_dip.addr.ip6, &p_sdb_entry->tap.ipv6da, sizeof(sai_ip6_t));
                    key.ip_dip.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                }
                if (p_sdb_entry->tap.ipv6sa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SIP);
                    sal_memcpy(&key.ip_sip.addr.ip6, &p_sdb_entry->tap.ipv6sa, sizeof(sai_ip6_t));
                    key.ip_sip.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                }
                
                if (p_sdb_entry->tap.macda_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_DMAC);
                    sal_memcpy(key.mac_dest, p_sdb_entry->tap.macda, sizeof(sai_mac_t));
                }
                if (p_sdb_entry->tap.macsa_en)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_SMAC);
                    sal_memcpy(key.mac_src, p_sdb_entry->tap.macsa, sizeof(sai_mac_t));
                }
                if (p_sdb_entry->action.outer_vlan.enable)
                {
                    SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_VLAN);
					if(p_sdb_entry->action.outer_vlan_action.enable)
					{
						switch(p_sdb_entry->action.outer_vlan_action.parameter.s32)
                    	{
						case SAI_ACL_VLAN_ACTION_ADD:
							SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_ADD);
							break;
						case SAI_ACL_VLAN_ACTION_ADD_OR_REPLACE:
							SAI_SET_BIT(key.edit_flag,CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_REP_OR_ADD);
							break;
						case SAI_ACL_VLAN_ACTION_REPLACE:
						default:
							break;
						}
					}
					
                    key.vlan = p_sdb_entry->action.outer_vlan.parameter.u16;
                }
                p_sdb_entry->tap.edit_group = ctc_sai_tap_group_get_edit_group(key, p_sdb_entry->tap.ptap_info);
                
                if (NULL == p_sdb_entry->tap.edit_group)
                {
                    return SAI_STATUS_FAILURE;
                }
                nexthop_id = ((ctc_sai_tap_group_edit_group*)p_sdb_entry->tap.edit_group)->nexthop_id;
            }
            else
            {
                ctc_sai_tap_group_get_nexthop_id(p_sdb_entry->tap.tap_group_id, &nexthop_id);
            }
            aclentry.acl.action.flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
            aclentry.acl.action.nh_id = nexthop_id;
        }
    }
#endif /* TAPPRODUCT */

    /*add acl entry and install acl entry*/
    if (is_worm_filter)
    {
#ifdef GREATBELT
        /*enable all ports&agg acl enable ingress*/
        ctc_sai_acl_enable_global_port();
#endif
        ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
        aclentry.acl.key.type = CTC_ACL_KEY_IPV4;
        aclentry.acl.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
        aclentry.acl.entry_id = index;
        aclentry.acl.priority = index + SAI_ACL_WORM_FILTER_PRIORITY_BASE;
        aclentry.acl.priority_valid = TRUE;
        ret = ctc_acl_add_entry(SAI_ACL_WORM_FILTER_GROUP, &aclentry.acl);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }

        ret = ctc_acl_install_entry(aclentry.acl.entry_id);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    
        p_sdb_entry->enable = TRUE;
        p_sdb_entry->ctc_entry_id_list[0] = index;
        p_sdb_entry->ctc_entry_id_count = 1;
        p_sdb_entry->dirction = CTC_BOTH_DIRECTION;
        p_sdb_entry->port_oid_list[0] = 0;
        p_sdb_entry->port_oid_count = 1;
        *acl_entry_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_ENTRY, p_sdb_entry->key.entry_id);
        return ret;
    }
    else if (port_id && !is_worm_filter)
    {
        /* add to asic */
        if (aclentry.is_ipv6)
        {
            aclentry.acl.key.type = CTC_ACL_KEY_IPV6;
        }
        else
        {
            aclentry.acl.key.type = CTC_ACL_KEY_IPV4;
            aclentry.acl.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
        }

        ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
        ctc_sai_port_objectid_to_gport(port_id, &gport);
        group_id = ctc_sai_port_make_acl_group_id(dir, gport);
        aclentry.acl.entry_id = index;
        aclentry.acl.priority = p_sdb_entry->priority;
        aclentry.acl.priority_valid = TRUE;   
        
#ifdef GREATBELT
        if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
        {
            ctc_sai_acl_en_lag_port(port_id, dir);
        }
        else if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_id))
        {
            ctc_sai_acl_port_en_db_alloc(port_id, dir, port_id);
        }
#endif /* GREATBELT */

        if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
        {
            ctc_sai_acl_get_copp_group_id (port_id, &group_id);
        }
		
		if (p_sdb_entry->outer_fields.erspan_id.enable)
        {
            group_id = ((stbl_acl_tunnel_t*)p_sdb_entry->tap.ptunnel_info)->erspan_service_group_id;
        }

        ret = ctc_acl_add_entry(group_id, &aclentry.acl);
        if (CTC_E_NONE != ret)
        {
            #ifdef TAPPRODUCT        
            if (p_sdb_entry->tap.trunction_en)
            {
                ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                p_sdb_entry->tap.trunction_en = FALSE;
            }
            #endif            
            goto out;
        }
        
        ret = ctc_acl_install_entry(aclentry.acl.entry_id);
        if (CTC_E_NONE != ret)
        {
            ctc_acl_remove_entry(aclentry.acl.entry_id);
            #ifdef TAPPRODUCT             
            if (p_sdb_entry->tap.trunction_en)
            {
                ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                p_sdb_entry->tap.trunction_en = FALSE;
            }
            #endif
            goto out;
        }
    
        p_sdb_entry->enable = TRUE;
        p_sdb_entry->ctc_entry_id_list[0] = index;
        p_sdb_entry->ctc_entry_id_count = 1;
        p_sdb_entry->port_oid_list[0] = port_id;
        p_sdb_entry->port_oid_count = 1;
        p_sdb_entry->dirction = dir;
        *acl_entry_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_ENTRY, p_sdb_entry->key.entry_id);
        return ret;
    }

    /*set the port bitmap acl*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {

        dir = CTC_INGRESS;
    }
    else
    {
        pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, attr_list, attr_count, &index);
        if (NULL != pattr_entry)
        {
            dir = CTC_EGRESS;
        }
    }
    
    if (NULL != pattr_entry)
    {
        for (i = 0; i < pattr_entry->value.aclfield.data.objlist.count; i++)
        {
            /* add to asic */
            if(aclentry.is_ipv6)
            {
                aclentry.acl.key.type = CTC_ACL_KEY_IPV6;
            }
            else
            {
                aclentry.acl.key.type = CTC_ACL_KEY_IPV4;
                aclentry.acl.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
            }
            ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &index);
            ctc_sai_port_objectid_to_gport(pattr_entry->value.aclfield.data.objlist.list[i], &gport);
            group_id = ctc_sai_port_make_acl_group_id(dir, gport);
            aclentry.acl.entry_id = index;
            aclentry.acl.priority = p_sdb_entry->priority;
            aclentry.acl.priority_valid = TRUE;

            if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                ctc_sai_acl_get_copp_group_id (port_id, &group_id);
            }
            ret = ctc_acl_add_entry(group_id, &aclentry.acl);
            if (CTC_E_NONE != ret)
            {
#ifdef TAPPRODUCT            
                if (p_sdb_entry->tap.trunction_en)
                {
                    ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                    p_sdb_entry->tap.trunction_en = FALSE;
                }
#endif                
                goto out;
            }
        
            ret = ctc_acl_install_entry(aclentry.acl.entry_id);
            if (CTC_E_NONE != ret)
            {
                ctc_acl_remove_entry(aclentry.acl.entry_id);
#ifdef TAPPRODUCT                
                if (p_sdb_entry->tap.trunction_en)
                {
                    ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
                    p_sdb_entry->tap.trunction_en = FALSE;
                }
#endif                
                goto out;
            }
        
            p_sdb_entry->enable = TRUE; 
            p_sdb_entry->ctc_entry_id_list[i] = index;
            p_sdb_entry->ctc_entry_id_count++;
            p_sdb_entry->port_oid_list[i] = port_id;
            p_sdb_entry->port_oid_count++;
            p_sdb_entry->dirction = dir;
        }
        *acl_entry_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_ENTRY, p_sdb_entry->key.entry_id);
    }
    return SAI_STATUS_SUCCESS;
    
out:
    if (p_sdb_entry)
    {
        ctclib_opb_free_offset(&p_acl_glb->acl_entry_opb, acl_entry_index);
        ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, index);
        stbl_acl_entry_del_acl_entry(&p_sdb_entry->key);
    }

    return ret;
}
#endif /* USW */

/*
* Routine Description:
*   Delete an ACL entry
*
* Arguments:
 *  [in] acl_entry_id - the acl entry id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_delete_acl_entry(_In_ sai_object_id_t acl_entry_id)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    stbl_acl_tbl_t              *p_sdb_tbl = NULL;
    stbl_acl_entry_t            *p_sdb_entry = NULL;
    sai_uint32_t i = 0;
    ctc_mirror_dest_t mirror_dest;
    
#ifdef GREATBELT
    stbl_acl_port_t *p_sdb_aclport = NULL;
#endif

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("acl_entry_id = 0x%llx", acl_entry_id);

    p_sdb_entry = ctc_sai_acl_db_get_entry_by_oid(acl_entry_id);
    if (NULL == p_sdb_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

#ifdef TAPPRODUCT
    if (p_sdb_entry->tap.trunction_en)
    {
        ctc_sai_tap_group_put_session_id(p_sdb_entry->tap.tap_group_id);
        p_sdb_entry->tap.trunction_en = FALSE;
    }
    if (p_sdb_entry->tap.edit_group)
    {
        ctc_sai_tap_group_release_edit_group((ctc_sai_tap_group_edit_group*)p_sdb_entry->tap.edit_group, 
            (ctc_sai_tap_info_t*)p_sdb_entry->tap.ptap_info);
        p_sdb_entry->tap.edit_group = NULL;
    }
    if (p_sdb_entry->tap.ptunnel_info)
    {
        __tunnel_decap_free(p_sdb_entry->tap.ptunnel_info);
        p_sdb_entry->tap.ptunnel_info = NULL;
    }
    if (p_sdb_entry->outer_fields.ipv4_vxlan_vni_dst_port.enable)
    {
        uint32 l4_vxlan_dest_port = 0;
        l4_vxlan_dest_port = L4_PROTOCOL_VXLAN_DEST_PORT;
        ret = ctc_global_ctl_set(CTC_GLOBAL_VXLAN_UDP_DEST_PORT, &l4_vxlan_dest_port);
        if (CTC_E_NONE != ret)
        {
            return ret;
        }
    }
#endif /* TAPPRODUCT */
    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(p_sdb_entry->tbl_oid);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    for (i = 0; i < p_sdb_entry->ctc_entry_id_count; i++)
    {
        ret = ctc_acl_uninstall_entry(p_sdb_entry->ctc_entry_id_list[i]);
        if (CTC_E_NONE != ret)
        {
            return ret;
        }
        ret = ctc_acl_remove_entry(p_sdb_entry->ctc_entry_id_list[i]);
        if (CTC_E_NONE != ret)
        {
            return ret;
        }
        ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, p_sdb_entry->ctc_entry_id_list[i]);
    }

#ifdef GREATBELT
    for (i = 0; i < p_sdb_entry->port_oid_count; i++)
    {
        if (CTC_BOTH_DIRECTION == p_sdb_entry->dirction && 0 == p_sdb_entry->port_oid_list[0])
        {
            ctc_sai_acl_disable_global_port();
        }
        else
        {
            if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(p_sdb_entry->port_oid_list[i]))
            {
                ctc_sai_acl_dis_lag_port(p_sdb_entry->port_oid_list[i], p_sdb_entry->dirction);
            }
            else
            {
                p_sdb_aclport = ctc_sai_acl_get_port_en_by_oid(p_sdb_entry->port_oid_list[i]);
                ctc_sai_acl_port_en_db_release(p_sdb_aclport, p_sdb_entry->dirction); 
            }
        }
    }
#endif /* GREATBELT */

    if (p_sdb_entry->mirror_oid)
    {
        mirror_dest.session_id = CTC_SAI_OBJECT_INDEX_GET(p_sdb_entry->mirror_oid);
        ctc_mirror_remove_session(&mirror_dest);
    }

    ctclib_opb_free_offset(&p_acl_glb->acl_entry_opb, p_sdb_entry->key.entry_id);
    stbl_acl_entry_del_acl_entry(&p_sdb_entry->key);
    
    return ret;
}

sai_status_t
ctc_sai_acl_action_map_db_attribute_set_mirror_value(sai_attribute_t *attr_value, const sai_attribute_t *attr)
{
    if (attr->id != SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS && attr->id != SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS)
    {
        return SAI_STATUS_SUCCESS;
    }

    attr_value->value.aclaction.parameter.oid = attr->value.aclaction.parameter.oid;
    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_acl_action_map_db_attribute(const sai_attribute_t *attr, stbl_acl_entry_t *p_sdb_entry)
{
    sai_status_t        ret = SAI_STATUS_SUCCESS;
    sai_attribute_t     *attr_value = NULL;

    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, attr->id, FALSE);
    if (NULL == attr_value)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    sal_memcpy(attr_value, attr, sizeof(sai_attribute_t));
    ctc_sai_acl_action_map_db_attribute_set_mirror_value(attr_value, attr);

    return ret;
}



/*
* Routine Description:
*   Set ACL entry attribute
*
* Arguments:
*    [in] acl_entry_id - the acl entry id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_acl_entry_attribute(
    _In_ sai_object_id_t acl_entry_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();
    stbl_acl_entry_t            *p_sdb_entry = NULL;
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_acl_action_t action;
    uint32 i = 0;
    
    p_sdb_entry = ctc_sai_acl_db_get_entry_by_oid(acl_entry_id);
    if (NULL == p_sdb_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    if (SAI_ACL_ENTRY_ATTR_PRIORITY == attr->id)
    {
        for (i = 0; i < p_sdb_entry->ctc_entry_id_count; i++)
        {
            ret += ctc_acl_set_entry_priority(CTC_SAI_OBJECT_INDEX_GET(p_sdb_entry->ctc_entry_id_list[i]), attr->value.u32);
        }
        p_sdb_entry->priority = attr->value.u32;
        return ret;
    }
    if (SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER == attr->id)
    {
        if (attr->value.aclaction.enable)
        {
            ctc_sai_policer_acl_set_policer(CTC_SAI_OBJECT_INDEX_GET(attr->value.aclaction.parameter.oid), CTC_SAI_OBJECT_INDEX_GET(acl_entry_id), TRUE);
        }
        else
        {
            ctc_sai_policer_acl_set_policer(CTC_SAI_OBJECT_INDEX_GET(attr->value.aclaction.parameter.oid), CTC_SAI_OBJECT_INDEX_GET(acl_entry_id), FALSE);
        }
    }
    sal_memset(&action, 0x0, sizeof(ctc_acl_action_t));
    ctc_sai_acl_action_map_db_attribute(attr, p_sdb_entry);
    ret = ctc_sai_acl_action_map_db_ctc(p_sdb_entry, &action);
#ifndef USW
    for (i = 0; i < p_sdb_entry->ctc_entry_id_count; i++)
    {
        ret += ctc_acl_update_action(CTC_SAI_OBJECT_INDEX_GET(p_sdb_entry->ctc_entry_id_list[i]), &action);
    }
#endif /* no USW */
    return ret;
}

/*
* Routine Description:
*   Get ACL table attribute
*
* Arguments:
*    [in] acl_table_id - acl table id
*    [in] attr_count - number of attributes
*    [Out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/

sai_status_t  sai_get_acl_entry_attribute (
    _In_ sai_object_id_t acl_entry_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    sai_attribute_t             *attr = NULL;
    uint32_t                    attr_idx = 0;
    stbl_acl_entry_t            *p_sdb_entry = NULL;

    CTC_SAI_DEBUG_FUNC();
    sai_get_acl_entry_attribute_debug_param(attr_count, attr_list);

    p_sdb_entry = ctc_sai_acl_db_get_entry_by_oid(acl_entry_id);
    if (NULL == p_sdb_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        
        if (SAI_ACL_ENTRY_ATTR_ADMIN_STATE == attr->id)
        {
            attr->value.booldata = p_sdb_entry->enable;
        }
        else if (SAI_ACL_ENTRY_ATTR_PRIORITY == attr->id)
        {
            attr->value.u32 = p_sdb_entry->priority;
        }

    }

    return ret;
}

sai_status_t
ctc_acl_counter_add_entry(sai_acl_stage_t acl_stage, sai_uint32_t acl_priority, uint32 *counter_id)
{
    ctc_stats_statsid_t stats;
    int32_t             sdk_ret  = 0;

    sal_memset(&stats,0,sizeof(stats));
    stats.statsid.acl_priority = acl_priority & 0xFF;
    stats.type                 = CTC_STATS_STATSID_TYPE_ACL;
    if (SAI_ACL_STAGE_INGRESS == acl_stage)
    {
        stats.dir = CTC_INGRESS;
    }
    else
    {
        stats.dir = CTC_EGRESS;
    }

    CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&stats), sdk_ret, out);
    if(SAI_STATUS_SUCCESS == sdk_ret)
    {
        *counter_id = stats.stats_id;
        ctc_stats_clear_stats(stats.stats_id);
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

/*
* Routine Description:
*   Create an ACL counter
*
* Arguments:
*   [out] acl_counter_id - the acl counter id
*   [in] attr_count - number of attributes
*   [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_acl_counter(
    _Out_ sai_object_id_t *acl_counter_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_acl_tbl_t      *p_sdb_tbl = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32 counter_id = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:attr_count %u", attr_count);

    CTC_SAI_PTR_VALID_CHECK(acl_counter_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    /* 1. get stage */
    p_sdb_tbl = ctc_sai_acl_db_get_table_by_oid(attr_list[SAI_ACL_COUNTER_ATTR_TABLE_ID].value.oid);
    if (NULL == p_sdb_tbl)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* 2. get stage */
    ret = ctc_acl_counter_add_entry(p_sdb_tbl->stage, p_sdb_tbl->priority, &counter_id);
    if (ret)
    {
        return ret;
    }

    *acl_counter_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ACL_COUNTER, counter_id);
    CTC_SAI_DEBUG("out:acl_counter_id 0x%llx", (*acl_counter_id));
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Delete an ACL counter
*
* Arguments:
 *  [in] acl_counter_id - the acl counter id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_delete_acl_counter(
    _In_ sai_object_id_t acl_counter_id)
{
    CTC_SAI_DEBUG_FUNC();
    
    return ctc_stats_destroy_statsid(CTC_SAI_OBJECT_INDEX_GET(acl_counter_id));
}

/*
* Routine Description:
*   Set ACL counter attribute
*
* Arguments:
*    [in] acl_counter_id - the acl counter id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_acl_counter_attribute(
    _In_ sai_object_id_t acl_counter_id,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t      ret      = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:acl_counter_id 0x%llx, attribute_id %u", acl_counter_id, attr->id);

    switch(attr->id)
    {
    case SAI_ACL_COUNTER_ATTR_TABLE_ID:
    case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
    case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
        return SAI_STATUS_NOT_SUPPORTED;
        break;

    case SAI_ACL_COUNTER_ATTR_PACKETS:
    case SAI_ACL_COUNTER_ATTR_BYTES:
        ret = ctc_stats_clear_stats(CTC_SAI_OBJECT_INDEX_GET(acl_counter_id));
        break;
    }

    return ret;
}

/*
* Routine Description:
*   Get ACL counter attribute
*
* Arguments:
*    [in] acl_counter_id - acl counter id
*    [in] attr_count - number of attributes
*    [Out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_acl_counter_attribute(
    _In_ sai_object_id_t acl_counter_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    sai_attribute_t*  attr     = NULL;
    uint32_t          attr_idx = 0;
    sai_status_t      ret      = SAI_STATUS_SUCCESS;
    ctc_stats_basic_t stats_count;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_acl_count_attr_entries,attr);
        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
            break;

        case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
            break;

        case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
            break;

        case SAI_ACL_COUNTER_ATTR_PACKETS:
            sal_memset(&stats_count, 0, sizeof(stats_count));
            ret = ctc_stats_get_stats(CTC_SAI_OBJECT_INDEX_GET(acl_counter_id), &stats_count);
            attr->value.u64 = stats_count.packet_count;
            break;

        case SAI_ACL_COUNTER_ATTR_BYTES:
            sal_memset(&stats_count, 0, sizeof(stats_count));
            ret = ctc_stats_get_stats(CTC_SAI_OBJECT_INDEX_GET(acl_counter_id), &stats_count);
            attr->value.u64 = stats_count.byte_count;
            break;
        }

        if (ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    ctc_sai_get_acl_counter_attribute_debug_param(acl_counter_id, attr_count, attr_list);
    return ret;
}

#define ________SAI_ACL_ACTION_FUNC


sai_status_t
ctc_sai_acl_db_init()
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    
    CTC_SAI_DEBUG_FUNC();
    sal_memset(&g_sai_acl_info, 0, sizeof(g_sai_acl_info));

    p_acl_glb->acl_tbl_max_count = GLB_SAI_ACL_MAX;
    if (0 != ctclib_opb_create(&p_acl_glb->acl_tbl_opb, 0, p_acl_glb->acl_tbl_max_count, "sai_acl_tbl_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    p_acl_glb->acl_entry_max_count = GLB_SAI_ACE_MAX;
    if (0 != ctclib_opb_create(&p_acl_glb->acl_entry_opb, 0, p_acl_glb->acl_entry_max_count, "sai_acl_entry_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    p_acl_glb->tunnel_max_count = 4096;
    if (0 != ctclib_opb_create(&p_acl_glb->tunnel_opb, 1, p_acl_glb->tunnel_max_count, "ctc_scl_entry_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    p_acl_glb->sdk_acl_entry_max_count = 100000;
    if (0 != ctclib_opb_create(&p_acl_glb->sdk_acl_entry_opb, 0, p_acl_glb->sdk_acl_entry_max_count, "ctc_acl_entry_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

	p_acl_glb->erspan_service_opb_max_count = 2048;
    if (0 != ctclib_opb_create(&p_acl_glb->erspan_service_opb, 1, p_acl_glb->erspan_service_opb_max_count, "ctc_acl_entry_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_PACKET_ACTION - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_packet_action;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_COUNTER - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_counter;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_mirror;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_mirror;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_COLOR - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_setcolor;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_TC - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_settc;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_set_policer;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_set_redirect_action;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_VLAN_ID - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_set_redirect_vlan;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_REDIRECT_UNTAG - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_set_redirect_untag;
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_TRUNCATION - SAI_ACL_ENTRY_ATTR_ACTION_START] = _acl_action_set_redirect_truncation;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_src_mac_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_dst_mac_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_src_ip_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_dst_ip_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPv6 - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_src_ipv6_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPv6 - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_dst_ipv6_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_STRIP_HEADER - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_strip_header_action;

    /*added by yejl for bug 53475, 2019-09-10*/
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_HEADER - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_header_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_SRC_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_src_ip_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_dst_ip_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_DST_MAC - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_dst_mac_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_NUM - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_key_num_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L2GRE_KEY_LEN - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l2gre_key_len_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_HEADER - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l3gre_header_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_SRC_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l3gre_src_ip_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l3gre_dst_ip_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_L3GRE_DST_MAC - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_l3gre_dst_mac_action;
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
     g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_HEADER - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_header_action;
    
    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_MAC - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_dst_mac_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRC_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_src_ip_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DST_IP - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_dest_ip_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_SRCPORT - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_src_port_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_DESTPORT - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_dest_port_action;

    g_sai_acl_info.g_acl_action[SAI_ACL_ENTRY_ATTR_ACTION_SET_ADD_VXLAN_VNI - SAI_ACL_ENTRY_ATTR_ACTION_START] 
        = _acl_action_set_add_vxlan_vni_action;
    
    /*ended by guoxd*/

    ctc_sai_acl_worm_filter_init();

#ifdef TAPPRODUCT
    ctc_sai_acl_global_tap_egress_default_init();
#endif


    return SAI_STATUS_SUCCESS;
}

#ifdef TAPPRODUCT
#ifndef GREATBELT
static void 
__tap_erspan_loopback_res_init()
{
    ctc_internal_port_assign_para_t	iloop = {0};
    ctc_internal_port_assign_para_t	eloop = {0};
    ctc_loopback_nexthop_param_t	eloop_nh = {0};
    uint32_t   nexthopid    = 0;
    uint16	iloop_port = 0;
    int32 	sdk_ret = 0;
    uint32 value = 1;
    uint32 action_idx = 0;

    /* create iloop port */
    iloop.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    iloop.gchip = 0;
    sdk_ret = ctc_alloc_internal_port(&iloop);
    iloop_port = iloop.inter_port;
    //OFDPA_LOG_TMP_DEBUG("iloop_port = %d\n",iloop_port);
    /* config iloop use service-acl */
    ctc_port_set_direction_property(iloop_port, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, value);
    ctc_port_set_direction_property(iloop_port, CTC_PORT_DIR_PROP_SERVICE_ACL_EN, CTC_INGRESS, value);
    ctc_port_set_direction_property(iloop_port, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, 
                                    CTC_INGRESS, CTC_ACL_TCAM_LKUP_TYPE_L2);
#ifdef GOLDENGATE
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_EXTERN_ENABLE, FALSE);
#endif
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_SNOOPING_PARSER, TRUE);
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, TRUE);

    /* 1, should not check source port equal to dest port */
    ctc_port_set_port_check_en(iloop_port, FALSE);

    /* 2, l2pdu should forwarding normally */
    for (action_idx = 0; action_idx <= 31; action_idx++)
    {
        ctc_l2pdu_set_port_action(  iloop_port, 
                                    action_idx, 
                                    CTC_PDU_L2PDU_ACTION_TYPE_FWD);
    }

    ctc_l2pdu_set_port_action(  iloop_port, 
                                CTC_L2PDU_ACTION_INDEX_BPDU, 
                                CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU);

    ctc_port_set_direction_property(iloop_port, 
                                    CTC_PORT_DIR_PROP_STAG_TPID_INDEX_EN,CTC_EGRESS, FALSE);

    /* 3, CRC disable */
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_CRC_CHECK_EN, FALSE);

    /* 4, dot1q type */
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH);

    /* 5, dot1q type */
    ctc_port_set_untag_dft_vid(iloop_port, FALSE, FALSE);

    /* 6, don't add default vlan */
    ctc_port_set_property(iloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, TRUE);

    /* 7, disable filter */
    ctc_port_set_vlan_filter_en(iloop_port, CTC_INGRESS, FALSE);
    ctc_port_set_vlan_filter_en(iloop_port, CTC_EGRESS, FALSE);

    /* 8, allow all vlan */
    ctc_port_set_vlan_ctl(iloop_port, CTC_VLANCTL_ALLOW_ALL_PACKETS);

    /* 14, by pass arp dhcp */
    ctc_port_set_property(  iloop_port, 
                            CTC_PORT_PROP_L3PDU_ARP_ACTION, 
                            CTC_PORT_ARP_ACTION_TYPE_FW);
    ctc_port_set_property(  iloop_port, 
                            CTC_PORT_PROP_L3PDU_DHCP_ACTION, 
                            CTC_PORT_DHCP_ACTION_TYPE_FW);

    ctc_port_set_property(  iloop_port, 
                            CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, 
                            TRUE);


    /* create eloop port */
    sdk_ret = ctc_sai_nexthop_alloc_offset(&nexthopid);
    eloop_nh.lpbk_lport = iloop_port;
    sdk_ret = ctc_nh_add_iloop(nexthopid, &eloop_nh);

    eloop.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    eloop.gchip = 0;
    eloop.nhid  = nexthopid;
    sdk_ret = ctc_alloc_internal_port(&eloop);
    tap_erspan_eloop_port = eloop.inter_port;	

    ctc_port_set_property(tap_erspan_eloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, value);

	(void)sdk_ret;
    //OFDPA_LOG_TMP_DEBUG("eloop_port = %d\n",eloop.inter_port);	
}
#endif /* !GOLDENGATE */
#endif /* !TAPPRODUCT */

#define ________SAI_ACL_INNER_FUNC
sai_status_t __acl_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
#ifdef TAPPRODUCT
#ifdef GOLDENGATE
    uint32 value    = 0;
#endif /* GOLDENGATE */
#endif /* TAPPRODUCT */
    ret = ctc_sai_acl_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

#ifdef TAPPRODUCT
#ifdef GOLDENGATE
	value = 0;
    ctc_global_ctl_set(CTC_GLOBAL_CHIP_STRIP_PACKET_SCLFLOW_BYTE_SHIFT, &value);
	value = 1;
    ctc_global_ctl_set(CTC_GLOBAL_CHIP_GRE_AUTO_RECOGNITION, &value);
    value = 0;
    ctc_global_ctl_set(CTC_GLOBAL_CHIP_TAP_RES, &value);
#endif /* GOLDENGATE */

#ifndef GREATBELT
    __tap_erspan_loopback_res_init();
#endif
#endif /* TAPPRODUCT */ 

    //ctc_sai_acl_init_lag_group();

#ifdef USW
//#ifdef OFPRODUCT
#if defined(OFPRODUCT) || defined(TAPPRODUCT)
    ctc_acl_league_t leg;
    sal_memset(&leg, 0 , sizeof(leg));
    leg.dir = GLB_INGRESS;
    leg.acl_priority = 0;
    leg.lkup_level_bitmap = 127;
    ret = ctc_acl_set_league_mode(&leg);

    sal_memset(&leg, 0 , sizeof(leg));
    leg.dir = GLB_EGRESS;
    leg.acl_priority = 0;
    leg.lkup_level_bitmap = 3;
    ret = ctc_acl_set_league_mode(&leg);	
#else
    ctc_acl_league_t leg;
    sal_memset(&leg, 0 , sizeof(leg));
    leg.dir = GLB_INGRESS;
    leg.acl_priority = 0;
    leg.lkup_level_bitmap = 7;
    ret = ctc_acl_set_league_mode(&leg);	

    sal_memset(&leg, 0 , sizeof(leg));
    leg.dir = GLB_EGRESS;
    leg.acl_priority = 0;
    leg.lkup_level_bitmap = 3;
    ret = ctc_acl_set_league_mode(&leg);	
#endif /* OFPRODUCT */ 
#endif /* USW */ 

#ifdef USW
    #if defined(OFPRODUCT) || defined(TAPPRODUCT)
    /* set ipefwdaclctl ForceL2L3ExtKeyToL2L3Key disable for match non-ip packets */
    ctc_global_acl_property_t acl_property;
    sal_memset(&acl_property, 0, sizeof(ctc_global_acl_property_t));
    acl_property.dir = CTC_INGRESS;
    ctc_global_ctl_get(CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    sal_memset(acl_property.l2l3_ext_key_use_l2l3_key, 0, sizeof(uint8)*MAX_CTC_PARSER_L3_TYPE);
    ctc_global_ctl_set(CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    #endif
#endif

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __acl_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_acl_api_t      g_sai_acl_api_func = {
    .create_acl_table           = sai_create_acl_table,
    .delete_acl_table           = sai_delete_acl_table,
    .set_acl_table_attribute    = sai_set_acl_table_attribute,
    .get_acl_table_attribute    = sai_get_acl_table_attribute,
#ifdef USW
    .create_acl_entry           = sai_create_acl_entry_duet2,
#else
    .create_acl_entry           = sai_create_acl_entry_gg_gb,
#endif
    .delete_acl_entry           = sai_delete_acl_entry,
    .set_acl_entry_attribute    = sai_set_acl_entry_attribute,
    .get_acl_entry_attribute    = sai_get_acl_entry_attribute,
    .create_acl_counter         = ctc_sai_create_acl_counter,
    .delete_acl_counter         = ctc_sai_delete_acl_counter,
    .set_acl_counter_attribute  = ctc_sai_set_acl_counter_attribute,
    .get_acl_counter_attribute  = ctc_sai_get_acl_counter_attribute
};

static ctc_sai_api_reg_info_t g_acl_api_reg_info = {
        .id  = SAI_API_ACL,
        .init_func = __acl_init_mode_fn,
        .exit_func = __acl_exit_mode_fn,
        .api_method_table = &g_sai_acl_api_func,
        .private_data     = NULL,
};

#define ________SAI_ACL_OUTER_FUNC
sai_status_t ctc_sai_acl_init()
{
    api_reg_register_fn(&g_acl_api_reg_info);

    return SAI_STATUS_SUCCESS;
}




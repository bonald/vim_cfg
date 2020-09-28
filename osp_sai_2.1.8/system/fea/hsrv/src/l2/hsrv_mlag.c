
#include "hsrv_inc.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "hsrv_brgif.h"

#define HSRV_DISABLE_ISOLATION_GROUP_OID    0

sai_object_id_t g_mlag_oid = HSRV_DISABLE_ISOLATION_GROUP_OID;

int32
hsrv_mlag_init_done_cb()
{
    uint32              attr_count = 1;
    sai_attribute_t     attr_list[1];
    sai_isolation_api_t *api = NULL;

    attr_list[0].id = SAI_ISOLATION_ATTR_MODE;
    attr_list[0].value.s32 = SAI_ISOLATION_GROUP_MODE_MLAG;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ISOLATION,(void**)&api));
    HSRV_IF_ERROR_RETURN(api->create_isolation_group(&g_mlag_oid, attr_count, attr_list));

    return HSRV_E_NONE;
}

static int32
_hsrv_mlag_set_isolate_group(sai_object_id_t port_oid, sai_object_id_t group_oid, sai_port_isolation_direction_t dir)
{
    sai_port_api_t *port_api = NULL;
    sai_attribute_t attr;

    sal_memset(&attr, 0, sizeof(attr));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    attr.id = SAI_PORT_ATTR_ISOLATION_DIRECTION;
    attr.value.s32 = dir;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    attr.id = SAI_PORT_ATTR_ISOLATION_GROUP;
    attr.value.oid = group_oid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_mlag_set_isolate_port(uint32 ifindex, uint32 is_set, sai_port_isolation_direction_t dir)
{
    tbl_interface_t *p_db_if = NULL;
    sai_object_id_t port_oid = 0;
    sai_object_id_t group_oid = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        return HSRV_E_NONE;
    }
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    if (is_set)
    {
        group_oid = g_mlag_oid;
    }
    else
    {
        group_oid = HSRV_DISABLE_ISOLATION_GROUP_OID;
    }

    port_oid = p_db_if->portid;
    HSRV_IF_ERROR_RETURN(_hsrv_mlag_set_isolate_group(port_oid, group_oid, dir));

    return HSRV_E_NONE;
}

int32
hsrv_mlag_set_is_group(uint32 ifindex, uint32 is_group)
{
    tbl_interface_t *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    if (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    hsrv_brgif_set_mlag_is_group(ifindex, is_group);
    
    /* both need flush FDB */
    if (is_group)
    {
        hsrv_fdb_flush_fdb_port(ifindex);
    }
    else
    {
        hsrv_fdb_flush_fdb_port(ifindex);
    }

    return HSRV_E_NONE;
}
    
int32
hsrv_mlag_set_isolate_mlag_port(uint32 ifindex, uint32 is_set)
{
    tbl_interface_t *p_db_if = NULL;
    sai_object_id_t port_oid = 0;
    sai_object_id_t group_oid = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        return HSRV_E_NONE;
    }
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    if (is_set)
    {
        group_oid = g_mlag_oid;
    }
    else
    {
        group_oid = HSRV_DISABLE_ISOLATION_GROUP_OID;
    }

    port_oid = p_db_if->portid;
    HSRV_IF_ERROR_RETURN(_hsrv_mlag_set_isolate_group(port_oid, group_oid, SAI_PORT_ISOLATION_DIRECTION_INGRESS));

    return HSRV_E_NONE;
}

uint32
hsrv_mlag_is_peer_up(tbl_mlag_port_t *p_db_port)
{
    if (p_db_port->peer_conf && p_db_port->peer_if_up)
    {
        return TRUE;
    }

    return FALSE;
}

static int32
_hsrv_mlag_peer_fdb_bmp_sync(tbl_mlag_t *p_mlag, tbl_mlag_t *p_db_mlag)
{
    uint32 i = 0;
    tbl_fdb_t pm_fdb;
    tbl_fdb_t *p_db_static_fdb = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    for (i = 0 ; i < GLB_MAX_VLAN_ID; i++)
    {
        if (GLB_BMP_ISSET(p_mlag->peer_fdb_bmp, i) && (!GLB_BMP_ISSET(p_db_mlag->peer_fdb_bmp, i)))
        {
            pm_fdb.key.fid = i;
            sal_memcpy(pm_fdb.key.mac, p_db_mlag->peer_routemac, MAC_ADDR_LEN);
            pm_fdb.ifindex = p_db_mlag->peer_link_ifindex;
            pm_fdb.flags = GLB_FDB_FLAG_STATIC;
            hsrv_fdb_sync_add_fdb(&pm_fdb);
        }
        else if ((!GLB_BMP_ISSET(p_mlag->peer_fdb_bmp, i)) && (GLB_BMP_ISSET(p_db_mlag->peer_fdb_bmp, i)))
        {
            pm_fdb.key.fid = i;
            sal_memcpy(pm_fdb.key.mac, p_db_mlag->peer_routemac, MAC_ADDR_LEN);
            pm_fdb.ifindex = p_db_mlag->peer_link_ifindex;
            pm_fdb.flags = GLB_FDB_FLAG_STATIC;
            /* Added by kcao for bug 41092 2016-10-13, check static FDB exist */
            p_db_static_fdb = tbl_fdb_get_fdb(&pm_fdb.key);
            if (p_db_static_fdb)
            {
                /* bypass static FDB */
                if (p_db_static_fdb->ifindex == pm_fdb.ifindex)
                {
                    continue;
                }
            }
            hsrv_fdb_sync_del_fdb(&pm_fdb);
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_is_mlag_peer_fdb(tbl_fdb_t *p_fdb)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    
    if (0 == sal_memcmp(p_fdb->key.mac, p_db_mlag->peer_routemac, MAC_ADDR_LEN))
    {
        if (p_fdb->ifindex == p_db_mlag->peer_link_ifindex)
        {
            return HSRV_E_FAIL;
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_mlag_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mlag_t *p_mlag = NULL;
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    if (CDB_OPER_SET != oper)
    {
        return HSRV_E_NONE;
    }
    
    p_mlag = (tbl_mlag_t*)p_tbl;
    (void)p_mlag;
    (void)p_db_mlag;
    switch (field_id)
    {
    case TBL_MLAG_FLD_PEER_LINK_IFINDEX:
        if (0 == p_mlag->peer_link_ifindex)
        {
            /* delete MLAG peer-link, need to flush FDB on peer-link */
            hsrv_fdb_mlag_peer_link_destory_cb(p_db_mlag->peer_link_ifindex);
        }
        break;

    case TBL_MLAG_FLD_PEER_FDB_BMP:
        _hsrv_mlag_peer_fdb_bmp_sync(p_mlag, p_db_mlag);
        break;

    case TBL_MLAG_FLD_FLUSH_FDB_MI:
        hsrv_fdb_db_flush_mlag_mi();
        break;
    
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_mlag_peer_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
#if 0
    tbl_mlag_peer_t *p_peer = NULL;
    tbl_mlag_peer_t *p_db_peer = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    if (CDB_OPER_SET != oper)
    {
        return HSRV_E_NONE;
    }
    
    p_peer = (tbl_mlag_peer_t*)p_tbl;
    switch (field_id)
    {
        case TBL_MLAG_PEER_FLD_TTL:
            HSRV_IF_ERROR_RETURN(hsrv_vlan_set_stp_instance(p_vlan_key->vid, p_vlan->instance));
            break;

        default:
            break;
    }

#endif
    return HSRV_E_NONE;
}

int32
hsrv_mlag_port_local_create(tbl_mlag_port_t *p_port)
{
    hsrv_brgif_set_mlag_peer_conf(p_port->ifindex, p_port->peer_conf);
    hsrv_brgif_set_mlag_peer_if_up(p_port->ifindex, p_port->peer_if_up);
    hsrv_fdb_flush_fdb_port_without_mlag_sync(p_port->ifindex);

    return HSRV_E_NONE;
}

int32
hsrv_mlag_port_local_destory(tbl_mlag_port_t *p_port)
{
    tbl_mlag_port_t *p_db_port = NULL;

    p_db_port = tbl_mlag_port_get_mlag_port(&p_port->key);
    if (p_db_port)
    {
        hsrv_brgif_set_mlag_peer_conf(p_db_port->ifindex, FALSE);
        hsrv_brgif_set_mlag_peer_if_up(p_db_port->ifindex, FALSE);
        hsrv_fdb_flush_fdb_port(p_db_port->ifindex);
        /* Added by kcao for bug 41121, flush protected FDB at peer-link port when MLAG is removed */
        hsrv_fdb_mlag_flush_peer_link_fdb(p_db_port->ifindex);
    }

    return HSRV_E_NONE;
}

int32
hsrv_mlag_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mlag_port_t *p_port = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    if (CDB_OPER_SET != oper)
    {
        return HSRV_E_NONE;
    }
    
    p_port = (tbl_mlag_port_t*)p_tbl;
    switch (field_id)
    {
    case TBL_MLAG_PORT_FLD_IFINDEX:
        /* local is created */
        if (p_port->ifindex)
        {
            hsrv_mlag_port_local_create(p_port);
        }
        else
        {
            hsrv_mlag_port_local_destory(p_port);
        }
        break;
            
    case TBL_MLAG_PORT_FLD_PEER_CONF:
        if (p_port->ifindex)
        {
            hsrv_brgif_set_mlag_peer_conf(p_port->ifindex, p_port->peer_conf);
        }
        break;
        
    case TBL_MLAG_PORT_FLD_PEER_IF_UP:
        if (p_port->ifindex)
        {
            hsrv_brgif_set_mlag_peer_if_up(p_port->ifindex, p_port->peer_if_up);
        }
        break;

    case TBL_MLAG_PORT_FLD_PROTECT_EN:
        if (p_port->ifindex)
        {
            hsrv_fdb_mlag_set_protect_en(p_port->key.id, p_port->protect_en);
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}


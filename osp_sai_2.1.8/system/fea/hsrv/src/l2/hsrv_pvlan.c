#if 0
#include "hsrv_inc.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"

sai_object_id_t g_pvlan_sai_obj_array[GLB_ISOLATE_GRP_MAX];
sai_object_id_t g_pvlan_sai_obj_array_pvlan[GLB_VLAN_MAX][GLB_ISOLATE_GRP_MAX];


int32
hsrv_pvlan_get_group_oid(vid_t primary_vid, uint32 group_id, sai_object_id_t *oid)
{
    if (group_id < GLB_ISOLATE_GRP_MAX && primary_vid <= GLB_VLAN_MAX)
    {
        *oid = g_pvlan_sai_obj_array_pvlan[primary_vid][group_id];
        return HSRV_E_NONE;
    }
    else
    {
        *oid = 0;
        return HSRV_E_NONE;
    }
}

int32
hsrv_pvlan_add_pvlan_community_group_id(tbl_pvlan_t *p_db_pvlan, ds_pvlan_community_t *p_ds_pvlan)
{
    sai_isolation_api_t*  group_api = NULL;
    sai_attribute_t attr[1];
    uint32  attr_count = 1;
    sai_object_id_t  group_oid = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_db_pvlan);
    HSRV_PTR_CHECK(p_ds_pvlan);
    
    group_oid = g_pvlan_sai_obj_array[p_ds_pvlan->group_id];
    if (group_oid == 0)
    {
        attr[0].id = SAI_ISOLATION_ATTR_MODE;
        attr[0].value.s32 = SAI_ISOLATION_GROUP_MODE_PVLAN;
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ISOLATION,(void**)&group_api));
        HSRV_IF_ERROR_RETURN(group_api->create_isolation_group(&group_oid, attr_count, attr));
        g_pvlan_sai_obj_array[p_ds_pvlan->group_id] = group_oid; 
        g_pvlan_sai_obj_array_pvlan[p_db_pvlan->key.primary_vid][p_ds_pvlan->group_id] = group_oid;
    }
    else
    {
        g_pvlan_sai_obj_array_pvlan[p_db_pvlan->key.primary_vid][p_ds_pvlan->group_id] = group_oid;
    }
    return HSRV_E_NONE;
}


int32
hsrv_pvlan_del_pvlan_community_group_id(tbl_pvlan_t *p_db_pvlan, ds_pvlan_community_t *p_ds_pvlan)
{
    sai_isolation_api_t*  group_api = NULL;
    sai_object_id_t  group_oid = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_db_pvlan);
    HSRV_PTR_CHECK(p_ds_pvlan);

    if (p_ds_pvlan->group_id >= GLB_ISOLATE_GRP_MAX)
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    group_oid = g_pvlan_sai_obj_array[p_ds_pvlan->group_id];
    if (group_oid != 0)
    {
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ISOLATION,(void**)&group_api));
        HSRV_IF_ERROR_RETURN(group_api->remove_isolation_group(group_oid));
        g_pvlan_sai_obj_array[p_ds_pvlan->group_id] = 0;
        g_pvlan_sai_obj_array_pvlan[p_db_pvlan->key.primary_vid][p_ds_pvlan->group_id] = 0;
    }
    else
    {
        g_pvlan_sai_obj_array_pvlan[p_db_pvlan->key.primary_vid][p_ds_pvlan->group_id] = 0;
    }
    return HSRV_E_NONE;
}

int32
hsrv_pvlan_community_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_pvlan_t *p_db_pvlan = NULL;
    tbl_pvlan_key_t *p_pvlan_key = NULL;
        
    ds_pvlan_community_t *p_ds_pvlan = NULL;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ds_pvlan = (ds_pvlan_community_t*)p_ds;
        p_pvlan_key = (tbl_pvlan_key_t*)p_tbl;
        p_db_pvlan = tbl_pvlan_get_pvlan(p_pvlan_key);
        HSRV_PTR_CHECK(p_db_pvlan);
        HSRV_IF_ERROR_RETURN(hsrv_pvlan_add_pvlan_community_group_id(p_db_pvlan, p_ds_pvlan));
        break;
    case CDB_OPER_DEL:
        p_ds_pvlan = (ds_pvlan_community_t*)p_ds;
        p_pvlan_key = (tbl_pvlan_key_t*)p_tbl;
        p_db_pvlan = tbl_pvlan_get_pvlan(p_pvlan_key);
        HSRV_PTR_CHECK(p_db_pvlan);
        HSRV_IF_ERROR_RETURN(hsrv_pvlan_del_pvlan_community_group_id(p_db_pvlan, p_ds_pvlan));
        break;
    case CDB_OPER_SET:
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

int32
hsrv_pvlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    if(p_ds_node)
    {
        switch (p_ds_node->id)
        {
            case DS_PVLAN_COMMUNITY:
                HSRV_IF_ERROR_RETURN(hsrv_pvlan_community_sync(p_tbl_node, p_ds_node, 
                    oper, field_id, p_tbl, p_ds));
                break; 
            default:
                return HSRV_E_NONE;  
        }
        return HSRV_E_NONE;
    }
    return HSRV_E_NONE;
}
#endif

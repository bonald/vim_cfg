
#include "hsrv_inc.h"
#include "gen/tbl_isolation_define.h"
#include "gen/tbl_isolation.h"

int32
hsrv_isolation_get_oid(uint32 group_id, sai_object_id_t *oid)
{
    tbl_isolation_key_t key;
    tbl_isolation_t *p_db_iso = NULL;

    if (group_id)
    {
        key.isolation_id = group_id;
        p_db_iso = tbl_isolation_get_isolation(&key);
        if (NULL == p_db_iso)
        {
            *oid = 0;
            return HSRV_E_ENTRY_NOT_EXIST;
        }

        *oid = p_db_iso->isolation_oid;
        return HSRV_E_NONE;
    }
    else
    {
        return HSRV_E_NONE;
    }
}

int32
hsrv_isolation_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_isolation_t *p_iso = NULL;
    tbl_isolation_key_t *p_iso_key = NULL;
    sai_object_id_t     isolation_oid = 0;
    sai_isolation_api_t *api = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_iso_key = (tbl_isolation_key_t*)p_tbl;
        p_iso = tbl_isolation_get_isolation(p_iso_key);
        HSRV_PTR_CHECK(p_iso);
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ISOLATION,(void**)&api));
        isolation_oid = p_iso->isolation_oid;
        HSRV_IF_ERROR_RETURN(api->remove_isolation_group(isolation_oid));
        break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_isolation_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_isolation_t *p_iso = NULL;
    sai_object_id_t     isolation_oid = 0;
    uint32              attr_count = 0;
    sai_attribute_t     *attr_list = NULL;
    sai_isolation_api_t *api = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_iso = (tbl_isolation_t*)p_tbl;
        p_iso = tbl_isolation_get_isolation(&p_iso->key);
        HSRV_PTR_CHECK(p_iso);
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ISOLATION,(void**)&api));
        HSRV_IF_ERROR_RETURN(api->create_isolation_group(&isolation_oid, attr_count, attr_list));
        p_iso->isolation_oid = isolation_oid;
        tbl_isolation_set_isolation_field(p_iso, TBL_ISOLATION_FLD_ISOLATION_OID);
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

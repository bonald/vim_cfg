
#include "proto.h"
#include "gen/tbl_isolation_define.h"
#include "gen/tbl_isolation.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

tbl_isolation_master_t *_g_p_tbl_isolation_master = NULL;

int32
tbl_isolation_add_isolation_sync(tbl_isolation_t *p_iso, uint32 sync)
{
    tbl_isolation_master_t *p_master = _g_p_tbl_isolation_master;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_isolation_get_isolation(&p_iso->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_iso = XCALLOC(MEM_TBL_ISOLATION, sizeof(tbl_isolation_t));
    if (NULL == p_db_iso)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_iso, p_iso, sizeof(tbl_isolation_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_iso->ports, CDB_NODE_TBL, TBL_INTERFACE, tbl_interface_cmp));

    /* 4. add to db */
    p_master->iso_array[p_iso->key.isolation_id] = p_db_iso;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ISOLATION, p_db_iso);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ISOLATION, p_db_iso);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_isolation_del_isolation_sync(tbl_isolation_key_t *p_iso_key, uint32 sync)
{
    tbl_isolation_master_t *p_master = _g_p_tbl_isolation_master;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_iso = tbl_isolation_get_isolation(p_iso_key);
    if (NULL == p_db_iso)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ISOLATION, p_db_iso);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ISOLATION, p_db_iso);
        #endif 
    }

    /* 3. delete from db */
    p_master->iso_array[p_iso_key->isolation_id] = NULL;
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_iso->ports));

    /* 4. free db entry */
    XFREE(MEM_TBL_ISOLATION, p_db_iso);

    return PM_E_NONE;
}

int32
tbl_isolation_set_isolation_field_sync(tbl_isolation_t *p_iso, tbl_isolation_field_id_t field_id, uint32 sync)
{
    tbl_isolation_t *p_db_iso = NULL;

    /* 1. lookup entry exist */
    p_db_iso = tbl_isolation_get_isolation(&p_iso->key);
    if (NULL == p_db_iso)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ISOLATION_FLD_ISOLATION_OID:
        p_db_iso->isolation_oid = p_iso->isolation_oid;
        break;

    case TBL_ISOLATION_FLD_MAX:
        sal_memcpy(p_db_iso, p_iso, sizeof(tbl_isolation_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ISOLATION, field_id, p_db_iso);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ISOLATION, field_id, p_db_iso);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_isolation_add_isolation(tbl_isolation_t *p_iso)
{
    return tbl_isolation_add_isolation_sync(p_iso, TRUE);
}

int32
tbl_isolation_del_isolation(tbl_isolation_key_t *p_iso_key)
{
    return tbl_isolation_del_isolation_sync(p_iso_key, TRUE);
}

int32
tbl_isolation_set_isolation_field(tbl_isolation_t *p_iso, tbl_isolation_field_id_t field_id)
{
    return tbl_isolation_set_isolation_field_sync(p_iso, field_id, TRUE);
}

tbl_isolation_t*
tbl_isolation_get_isolation(tbl_isolation_key_t *p_iso_key)
{
    tbl_isolation_master_t *p_master = _g_p_tbl_isolation_master;

    if (GLB_PORT_ISOLATE_GROUP_MAX <= p_iso_key->isolation_id)
    {
        return NULL;
    }
    return (p_master->iso_array[p_iso_key->isolation_id]);
}

char*
tbl_isolation_key_val2str(tbl_isolation_t *p_iso, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_iso->key.isolation_id);
    return str;
}

int32
tbl_isolation_key_str2val(char *str, tbl_isolation_t *p_iso)
{
    int32 ret = 0;

    p_iso->key.isolation_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_isolation_key_name_dump(tbl_isolation_t *p_iso, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ISOLATION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_iso->key.isolation_id);
    return str;
}

char*
tbl_isolation_key_value_dump(tbl_isolation_t *p_iso, char *str)
{
    sal_sprintf(str, "%u", p_iso->key.isolation_id);
    return str;
}

char*
tbl_isolation_field_name_dump(tbl_isolation_t *p_iso, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ISOLATION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ISOLATION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_isolation_field_value_dump(tbl_isolation_t *p_iso, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ISOLATION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_iso->key.isolation_id);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_ISOLATION_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_iso->isolation_oid);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_PORTS, field_id))
    {
    }
    return str;
}

char**
tbl_isolation_table_dump(tbl_isolation_t *p_iso, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_isolation_key_name_dump(p_iso, buf));
    for(i=1; i<TBL_ISOLATION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_isolation_field_name_dump(p_iso, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_isolation_field_value_dump(p_iso, i, buf));
    }
    return str;
}

int32
tbl_isolation_field_value_parser(char *str, int32 field_id, tbl_isolation_t *p_iso)
{
    if (FLD_MATCH(TBL_ISOLATION_FLD_KEY, field_id))
    {
        int32 ret;
        p_iso->key.isolation_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_ISOLATION_OID, field_id))
    {
        int32 ret;
        p_iso->isolation_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_PORTS, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_isolation_table_parser(char** array, char* key_value,tbl_isolation_t *p_iso)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ISOLATION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_isolation_key_str2val(key_value, p_iso));

    for(i=1; i<TBL_ISOLATION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_isolation_field_value_parser( array[j++], i, p_iso));
    }

    return PM_E_NONE;
}

int32
tbl_isolation_dump_one(tbl_isolation_t *p_iso, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ISOLATION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ISOLATION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ISOLATION_FLD_KEY].name,
            p_iso->key.isolation_id);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_ISOLATION_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ISOLATION_FLD_ISOLATION_OID].name,
            p_iso->isolation_oid);
    }
    if (FLD_MATCH(TBL_ISOLATION_FLD_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_ISOLATION_FLD_PORTS].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_iso->ports));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_isolation_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_isolation_master_t *p_master = _g_p_tbl_isolation_master;
    tbl_isolation_t *p_db_iso = NULL;
    uint32 isolation_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (isolation_id = 0; isolation_id < GLB_PORT_ISOLATE_GROUP_MAX; isolation_id++)
    {
        p_db_iso = p_master->iso_array[isolation_id];
        if (NULL == p_db_iso)
        {
            continue;
        }
        rc |= fn(p_db_iso, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_isolation_master_t*
tbl_isolation_get_master()
{
    return _g_p_tbl_isolation_master;
}

tbl_isolation_master_t*
tbl_isolation_init_isolation()
{
    _g_p_tbl_isolation_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_isolation_master_t));
    return _g_p_tbl_isolation_master;
}

int32
tbl_isolation_set_ports_sync(tbl_isolation_t *p_iso, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_iso = tbl_isolation_get_isolation(&p_iso->key);
    if (NULL == p_db_iso)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_iso->ports, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_iso->ports, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_INTERFACE, add, &refer);
        cdb_sync_tbl_set_refer(TBL_ISOLATION, TBL_ISOLATION_FLD_PORTS, p_iso, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_isolation_set_ports(tbl_isolation_t *p_iso, uint32 add, void *p_object)
{
    return tbl_isolation_set_ports_sync(p_iso, add, p_object, TRUE);
}


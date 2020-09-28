
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_circuit_id_add_circuit_id_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_circuit_id_t *p_db_circuit_id = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_circuit_id = ds_circuit_id_get_circuit_id(p_if, p_circuit_id);
    if (p_db_circuit_id)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_circuit_id = XCALLOC(MEM_DS_CIRCUIT_ID, sizeof(ds_circuit_id_t));
    if (NULL == p_db_circuit_id)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_circuit_id, p_circuit_id, sizeof(ds_circuit_id_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_if->circuit_id, p_db_circuit_id));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_CIRCUIT_ID, p_db_if, p_db_circuit_id);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_CIRCUIT_ID, p_db_if, p_db_circuit_id);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_circuit_id_del_circuit_id_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_circuit_id_t *p_db_circuit_id = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_circuit_id = ds_circuit_id_get_circuit_id(p_if, p_circuit_id);
    if (NULL == p_db_circuit_id)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_CIRCUIT_ID, p_db_if, p_db_circuit_id);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_CIRCUIT_ID, p_db_if, p_db_circuit_id);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_if->circuit_id, p_db_circuit_id));

    /* 4. free db entry */
    XFREE(MEM_DS_CIRCUIT_ID, p_db_circuit_id);

    return PM_E_NONE;
}

int32
ds_circuit_id_set_circuit_id_field_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, ds_circuit_id_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_circuit_id_t *p_db_circuit_id = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_circuit_id = ds_circuit_id_get_circuit_id(p_if, p_circuit_id);
    if (NULL == p_db_circuit_id)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_CIRCUIT_ID_FLD_CIRCUIT:
        sal_memcpy(p_db_circuit_id->circuit, p_circuit_id->circuit, sizeof(p_circuit_id->circuit));
        break;

    case DS_CIRCUIT_ID_FLD_MAX:
        sal_memcpy(p_db_circuit_id, p_circuit_id, sizeof(ds_circuit_id_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_CIRCUIT_ID, field_id, p_db_if, p_db_circuit_id);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_CIRCUIT_ID, field_id, p_db_if, p_db_circuit_id);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_circuit_id_add_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{
    return ds_circuit_id_add_circuit_id_sync(p_if, p_circuit_id, TRUE);
}

int32
ds_circuit_id_del_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{
    return ds_circuit_id_del_circuit_id_sync(p_if, p_circuit_id, TRUE);
}

int32
ds_circuit_id_set_circuit_id_field(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, ds_circuit_id_field_id_t field_id)
{
    return ds_circuit_id_set_circuit_id_field_sync(p_if, p_circuit_id, field_id, TRUE);
}

ds_circuit_id_t*
ds_circuit_id_get_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_if->circuit_id, p_circuit_id);
}

int32
ds_circuit_id_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);

    p_circuit_id->key.vid = cdb_uint_str2val(str, &ret);

    return ret;
}

char*
ds_circuit_id_key_value_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char* str)
{

    sal_sprintf(str, "%u", p_circuit_id->key.vid);
    return str;
}

char*
ds_circuit_id_key_name_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_CIRCUIT_ID);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_circuit_id_key_value_dump(p_if, p_circuit_id, ds_str));
    return str;
}

char*
ds_circuit_id_field_name_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_CIRCUIT_ID);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_CIRCUIT_ID_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_circuit_id_field_value_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_circuit_id->key.vid);
    }
    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_CIRCUIT, field_id))
    {
        sal_sprintf(str, "%s", p_circuit_id->circuit);
    }
    return str;
}

char**
ds_circuit_id_table_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_circuit_id_key_name_dump(p_if, p_circuit_id, buf));
    for(i=1; i<DS_CIRCUIT_ID_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_circuit_id_field_name_dump(p_if, p_circuit_id, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_circuit_id_field_value_dump(p_if, p_circuit_id, i, buf));
    }
    return str;
}

int32
ds_circuit_id_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{
    int32 ret=0;
    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_KEY, field_id))
    {
        p_circuit_id->key.vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_CIRCUIT, field_id))
    {
        sal_strcpy(p_circuit_id->circuit, str);
    }
    return ret;
}

int32
ds_circuit_id_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_CIRCUIT_ID);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_circuit_id_key_str2val(tbl_key_value, ds_key_value, p_if, p_circuit_id));

    for(i=1; i<DS_CIRCUIT_ID_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_circuit_id_field_value_parser( array[j++], i, p_if, p_circuit_id));
    }

    return PM_E_NONE;
}

int32
ds_circuit_id_dump_one(ds_circuit_id_t *p_circuit_id, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_CIRCUIT_ID);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CIRCUIT_ID_FLD_KEY].name,
            p_circuit_id->key.vid);
    }
    if (FLD_MATCH(DS_CIRCUIT_ID_FLD_CIRCUIT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CIRCUIT_ID_FLD_CIRCUIT].name,
            p_circuit_id->circuit);
    }

    return PM_E_NONE;
}
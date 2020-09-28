
#include "proto.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "cdb_data_cmp.h"

int32
ds_ospf_auth_add_ospf_auth_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_ospf_auth_t *p_db_ospf_auth = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_ospf_auth = ds_ospf_auth_get_ospf_auth(p_rtif, p_ospf_auth);
    if (p_db_ospf_auth)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf_auth = XCALLOC(MEM_DS_OSPF_AUTH, sizeof(ds_ospf_auth_t));
    if (NULL == p_db_ospf_auth)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf_auth, p_ospf_auth, sizeof(ds_ospf_auth_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_rtif->ospf_md5_key, p_db_ospf_auth));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_ROUTE_IF, DS_OSPF_AUTH, p_db_rtif, p_db_ospf_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_ROUTE_IF, DS_OSPF_AUTH, p_db_rtif, p_db_ospf_auth);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_ospf_auth_del_ospf_auth_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_ospf_auth_t *p_db_ospf_auth = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_ospf_auth = ds_ospf_auth_get_ospf_auth(p_rtif, p_ospf_auth);
    if (NULL == p_db_ospf_auth)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_ROUTE_IF, DS_OSPF_AUTH, p_db_rtif, p_db_ospf_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_ROUTE_IF, DS_OSPF_AUTH, p_db_rtif, p_db_ospf_auth);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_rtif->ospf_md5_key, p_db_ospf_auth));

    /* 4. free db entry */
    XFREE(MEM_DS_OSPF_AUTH, p_db_ospf_auth);

    return PM_E_NONE;
}

int32
ds_ospf_auth_set_ospf_auth_field_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, ds_ospf_auth_field_id_t field_id, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_ospf_auth_t *p_db_ospf_auth = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_ospf_auth = ds_ospf_auth_get_ospf_auth(p_rtif, p_ospf_auth);
    if (NULL == p_db_ospf_auth)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY:
        sal_memcpy(p_db_ospf_auth->ospf_auth_md5_key, p_ospf_auth->ospf_auth_md5_key, sizeof(p_ospf_auth->ospf_auth_md5_key));
        break;

    case DS_OSPF_AUTH_FLD_MAX:
        sal_memcpy(p_db_ospf_auth, p_ospf_auth, sizeof(ds_ospf_auth_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_ROUTE_IF, DS_OSPF_AUTH, field_id, p_db_rtif, p_db_ospf_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_ROUTE_IF, DS_OSPF_AUTH, field_id, p_db_rtif, p_db_ospf_auth);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_ospf_auth_add_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{
    return ds_ospf_auth_add_ospf_auth_sync(p_rtif, p_ospf_auth, TRUE);
}

int32
ds_ospf_auth_del_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{
    return ds_ospf_auth_del_ospf_auth_sync(p_rtif, p_ospf_auth, TRUE);
}

int32
ds_ospf_auth_set_ospf_auth_field(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, ds_ospf_auth_field_id_t field_id)
{
    return ds_ospf_auth_set_ospf_auth_field_sync(p_rtif, p_ospf_auth, field_id, TRUE);
}

ds_ospf_auth_t*
ds_ospf_auth_get_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{
    tbl_route_if_t *p_db_rtif = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_rtif->ospf_md5_key, p_ospf_auth);
}

int32
ds_ospf_auth_key_str2val(char* tbl_key_value, char* str, tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{

    int32 ret = 0;

    ret = tbl_route_if_key_str2val(tbl_key_value, p_rtif);

    p_ospf_auth->key.id = cdb_uint_str2val(str, &ret);

    return ret;
}

char*
ds_ospf_auth_key_value_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char* str)
{

    sal_sprintf(str, "%u", p_ospf_auth->key.id);
    return str;
}

char*
ds_ospf_auth_key_name_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_ROUTE_IF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_OSPF_AUTH);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_route_if_key_value_dump(p_rtif, tbl_str), p_node_ds->name, ds_ospf_auth_key_value_dump(p_rtif, p_ospf_auth, ds_str));
    return str;
}

char*
ds_ospf_auth_field_name_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_OSPF_AUTH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_OSPF_AUTH_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_ospf_auth_field_value_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_OSPF_AUTH_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_auth->key.id);
    }
    if (FLD_MATCH(DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ospf_auth->ospf_auth_md5_key);
    }
    return str;
}

char**
ds_ospf_auth_table_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_ospf_auth_key_name_dump(p_rtif, p_ospf_auth, buf));
    for(i=1; i<DS_OSPF_AUTH_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_ospf_auth_field_name_dump(p_rtif, p_ospf_auth, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_ospf_auth_field_value_dump(p_rtif, p_ospf_auth, i, buf));
    }
    return str;
}

int32
ds_ospf_auth_field_value_parser(char *str, int32 field_id, tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{
    int32 ret=0;
    if (FLD_MATCH(DS_OSPF_AUTH_FLD_KEY, field_id))
    {
        p_ospf_auth->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY, field_id))
    {
        sal_strcpy(p_ospf_auth->ospf_auth_md5_key, str);
    }
    return ret;
}

int32
ds_ospf_auth_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_OSPF_AUTH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_ospf_auth_key_str2val(tbl_key_value, ds_key_value, p_rtif, p_ospf_auth));

    for(i=1; i<DS_OSPF_AUTH_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_ospf_auth_field_value_parser( array[j++], i, p_rtif, p_ospf_auth));
    }

    return PM_E_NONE;
}

int32
ds_ospf_auth_dump_one(ds_ospf_auth_t *p_ospf_auth, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_OSPF_AUTH);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_OSPF_AUTH_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_OSPF_AUTH_FLD_KEY].name,
            p_ospf_auth->key.id);
    }
    if (FLD_MATCH(DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY].name,
            p_ospf_auth->ospf_auth_md5_key);
    }

    return PM_E_NONE;
}

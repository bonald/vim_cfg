
#include "proto.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "cdb_data_cmp.h"

int32
ds_nd_prefix_add_nd_prefix_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_nd_prefix_t *p_db_nd_prefix = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_nd_prefix = ds_nd_prefix_get_nd_prefix(p_rtif, p_nd_prefix);
    if (p_db_nd_prefix)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nd_prefix = XCALLOC(MEM_DS_ND_PREFIX, sizeof(ds_nd_prefix_t));
    if (NULL == p_db_nd_prefix)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nd_prefix, p_nd_prefix, sizeof(ds_nd_prefix_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_rtif->nd_prefix, p_db_nd_prefix));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_ROUTE_IF, DS_ND_PREFIX, p_db_rtif, p_db_nd_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_ROUTE_IF, DS_ND_PREFIX, p_db_rtif, p_db_nd_prefix);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_nd_prefix_del_nd_prefix_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_nd_prefix_t *p_db_nd_prefix = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_nd_prefix = ds_nd_prefix_get_nd_prefix(p_rtif, p_nd_prefix);
    if (NULL == p_db_nd_prefix)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_ROUTE_IF, DS_ND_PREFIX, p_db_rtif, p_db_nd_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_ROUTE_IF, DS_ND_PREFIX, p_db_rtif, p_db_nd_prefix);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_rtif->nd_prefix, p_db_nd_prefix));

    /* 4. free db entry */
    XFREE(MEM_DS_ND_PREFIX, p_db_nd_prefix);

    return PM_E_NONE;
}

int32
ds_nd_prefix_set_nd_prefix_field_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, ds_nd_prefix_field_id_t field_id, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_nd_prefix_t *p_db_nd_prefix = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_nd_prefix = ds_nd_prefix_get_nd_prefix(p_rtif, p_nd_prefix);
    if (NULL == p_db_nd_prefix)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_ND_PREFIX_FLD_CONFIG:
        p_db_nd_prefix->config = p_nd_prefix->config;
        break;

    case DS_ND_PREFIX_FLD_VLIFETIME:
        p_db_nd_prefix->vlifetime = p_nd_prefix->vlifetime;
        break;

    case DS_ND_PREFIX_FLD_PLIFETIME:
        p_db_nd_prefix->plifetime = p_nd_prefix->plifetime;
        break;

    case DS_ND_PREFIX_FLD_ONLINK:
        p_db_nd_prefix->onlink = p_nd_prefix->onlink;
        break;

    case DS_ND_PREFIX_FLD_AUTOCONFIG:
        p_db_nd_prefix->autoconfig = p_nd_prefix->autoconfig;
        break;

    case DS_ND_PREFIX_FLD_MAX:
        sal_memcpy(p_db_nd_prefix, p_nd_prefix, sizeof(ds_nd_prefix_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_ROUTE_IF, DS_ND_PREFIX, field_id, p_db_rtif, p_db_nd_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_ROUTE_IF, DS_ND_PREFIX, field_id, p_db_rtif, p_db_nd_prefix);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_nd_prefix_add_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{
    return ds_nd_prefix_add_nd_prefix_sync(p_rtif, p_nd_prefix, TRUE);
}

int32
ds_nd_prefix_del_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{
    return ds_nd_prefix_del_nd_prefix_sync(p_rtif, p_nd_prefix, TRUE);
}

int32
ds_nd_prefix_set_nd_prefix_field(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, ds_nd_prefix_field_id_t field_id)
{
    return ds_nd_prefix_set_nd_prefix_field_sync(p_rtif, p_nd_prefix, field_id, TRUE);
}

ds_nd_prefix_t*
ds_nd_prefix_get_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{
    tbl_route_if_t *p_db_rtif = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_rtif->nd_prefix, p_nd_prefix);
}

int32
ds_nd_prefix_key_str2val(char* tbl_key_value, char* str, tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{

    int32 ret = 0;

    ret = tbl_route_if_key_str2val(tbl_key_value, p_rtif);

    ret = cdb_nd_prefix_key_str2val(str, &p_nd_prefix->key);

    return ret;
}

char*
ds_nd_prefix_key_value_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char* str)
{


    cdb_nd_prefix_key_val2str(str, MAX_CMD_STR_LEN, &p_nd_prefix->key);
    return str;
}

char*
ds_nd_prefix_key_name_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_ROUTE_IF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_ND_PREFIX);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_route_if_key_value_dump(p_rtif, tbl_str), p_node_ds->name, ds_nd_prefix_key_value_dump(p_rtif, p_nd_prefix, ds_str));
    return str;
}

char*
ds_nd_prefix_field_name_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_ND_PREFIX);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_ND_PREFIX_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_nd_prefix_field_value_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_ND_PREFIX_FLD_KEY, field_id))
    {
        cdb_nd_prefix_key_val2str(str, MAX_CMD_STR_LEN, &p_nd_prefix->key);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_CONFIG, field_id))
    {
        sal_sprintf(str, "%u", p_nd_prefix->config);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_VLIFETIME, field_id))
    {
        sal_sprintf(str, "%u", p_nd_prefix->vlifetime);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_PLIFETIME, field_id))
    {
        sal_sprintf(str, "%u", p_nd_prefix->plifetime);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_ONLINK, field_id))
    {
        sal_sprintf(str, "%u", p_nd_prefix->onlink);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_AUTOCONFIG, field_id))
    {
        sal_sprintf(str, "%u", p_nd_prefix->autoconfig);
    }
    return str;
}

char**
ds_nd_prefix_table_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_nd_prefix_key_name_dump(p_rtif, p_nd_prefix, buf));
    for(i=1; i<DS_ND_PREFIX_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_nd_prefix_field_name_dump(p_rtif, p_nd_prefix, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_nd_prefix_field_value_dump(p_rtif, p_nd_prefix, i, buf));
    }
    return str;
}

int32
ds_nd_prefix_field_value_parser(char *str, int32 field_id, tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{
    int32 ret=0;
    if (FLD_MATCH(DS_ND_PREFIX_FLD_KEY, field_id))
    {
        cdb_nd_prefix_key_str2val(str, &p_nd_prefix->key);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_CONFIG, field_id))
    {
        p_nd_prefix->config = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_VLIFETIME, field_id))
    {
        p_nd_prefix->vlifetime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_PLIFETIME, field_id))
    {
        p_nd_prefix->plifetime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_ONLINK, field_id))
    {
        p_nd_prefix->onlink = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_AUTOCONFIG, field_id))
    {
        p_nd_prefix->autoconfig = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_nd_prefix_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_ND_PREFIX);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_nd_prefix_key_str2val(tbl_key_value, ds_key_value, p_rtif, p_nd_prefix));

    for(i=1; i<DS_ND_PREFIX_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_nd_prefix_field_value_parser( array[j++], i, p_rtif, p_nd_prefix));
    }

    return PM_E_NONE;
}

int32
ds_nd_prefix_dump_one(ds_nd_prefix_t *p_nd_prefix, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_ND_PREFIX);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_ND_PREFIX_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ND_PREFIX_FLD_KEY].name,
            cdb_nd_prefix_key_val2str(buf, MAX_CMD_STR_LEN, &p_nd_prefix->key));
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_CONFIG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ND_PREFIX_FLD_CONFIG].name,
            p_nd_prefix->config);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_VLIFETIME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ND_PREFIX_FLD_VLIFETIME].name,
            p_nd_prefix->vlifetime);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_PLIFETIME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ND_PREFIX_FLD_PLIFETIME].name,
            p_nd_prefix->plifetime);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_ONLINK, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ND_PREFIX_FLD_ONLINK].name,
            p_nd_prefix->onlink);
    }
    if (FLD_MATCH(DS_ND_PREFIX_FLD_AUTOCONFIG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ND_PREFIX_FLD_AUTOCONFIG].name,
            p_nd_prefix->autoconfig);
    }

    return PM_E_NONE;
}

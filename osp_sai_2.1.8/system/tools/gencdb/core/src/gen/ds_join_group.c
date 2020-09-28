
#include "proto.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "cdb_data_cmp.h"

int32
ds_join_group_add_join_group_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_join_group_t *p_db_join = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_join = ds_join_group_get_join_group(p_if, p_join);
    if (p_db_join)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_join = XCALLOC(MEM_DS_JOIN_GROUP, sizeof(ds_join_group_t));
    if (NULL == p_db_join)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_join, p_join, sizeof(ds_join_group_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_if->join_list, p_db_join));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_IGSP_INTF, DS_JOIN_GROUP, p_db_if, p_db_join);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_IGSP_INTF, DS_JOIN_GROUP, p_db_if, p_db_join);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_join_group_del_join_group_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_join_group_t *p_db_join = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_join = ds_join_group_get_join_group(p_if, p_join);
    if (NULL == p_db_join)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_IGSP_INTF, DS_JOIN_GROUP, p_db_if, p_db_join);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_IGSP_INTF, DS_JOIN_GROUP, p_db_if, p_db_join);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_if->join_list, p_db_join));

    /* 4. free db entry */
    XFREE(MEM_DS_JOIN_GROUP, p_db_join);

    return PM_E_NONE;
}

int32
ds_join_group_set_join_group_field_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, ds_join_group_field_id_t field_id, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_join_group_t *p_db_join = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_join = ds_join_group_get_join_group(p_if, p_join);
    if (NULL == p_db_join)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_JOIN_GROUP_FLD_VID:
        p_db_join->vid = p_join->vid;
        break;

    case DS_JOIN_GROUP_FLD_TYPE:
        p_db_join->type = p_join->type;
        break;

    case DS_JOIN_GROUP_FLD_MAX:
        sal_memcpy(p_db_join, p_join, sizeof(ds_join_group_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_IGSP_INTF, DS_JOIN_GROUP, field_id, p_db_if, p_db_join);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_IGSP_INTF, DS_JOIN_GROUP, field_id, p_db_if, p_db_join);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_join_group_add_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{
    return ds_join_group_add_join_group_sync(p_if, p_join, TRUE);
}

int32
ds_join_group_del_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{
    return ds_join_group_del_join_group_sync(p_if, p_join, TRUE);
}

int32
ds_join_group_set_join_group_field(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, ds_join_group_field_id_t field_id)
{
    return ds_join_group_set_join_group_field_sync(p_if, p_join, field_id, TRUE);
}

ds_join_group_t*
ds_join_group_get_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{
    tbl_igsp_intf_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_if->join_list, p_join);
}

int32
ds_join_group_key_str2val(char* tbl_key_value, char* str, tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{

    int32 ret = 0;

    ret = tbl_igsp_intf_key_str2val(tbl_key_value, p_if);

    ret = cdb_addr_ipv4_str2val(&p_join->key.group, str, 0);

    return ret;
}

char*
ds_join_group_key_value_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char* str)
{


    cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_join->key.group);
    return str;
}

char*
ds_join_group_key_name_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_IGSP_INTF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_JOIN_GROUP);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_igsp_intf_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_join_group_key_value_dump(p_if, p_join, ds_str));
    return str;
}

char*
ds_join_group_field_name_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_JOIN_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_JOIN_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_join_group_field_value_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_JOIN_GROUP_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_join->key.group);
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_join->vid);
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_join->type);
    }
    return str;
}

char**
ds_join_group_table_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_join_group_key_name_dump(p_if, p_join, buf));
    for(i=1; i<DS_JOIN_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_join_group_field_name_dump(p_if, p_join, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_join_group_field_value_dump(p_if, p_join, i, buf));
    }
    return str;
}

int32
ds_join_group_field_value_parser(char *str, int32 field_id, tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{
    int32 ret=0;
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_join->key.group, str, 0);
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_VID, field_id))
    {
        p_join->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_TYPE, field_id))
    {
        p_join->type = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_join_group_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_igsp_intf_t *p_if, ds_join_group_t *p_join)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_JOIN_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_join_group_key_str2val(tbl_key_value, ds_key_value, p_if, p_join));

    for(i=1; i<DS_JOIN_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_join_group_field_value_parser( array[j++], i, p_if, p_join));
    }

    return PM_E_NONE;
}

int32
ds_join_group_dump_one(ds_join_group_t *p_join, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_JOIN_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_JOIN_GROUP_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_JOIN_GROUP_FLD_KEY].name,
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_join->key.group));
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_VID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_JOIN_GROUP_FLD_VID].name,
            p_join->vid);
    }
    if (FLD_MATCH(DS_JOIN_GROUP_FLD_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_JOIN_GROUP_FLD_TYPE].name,
            p_join->type);
    }

    return PM_E_NONE;
}

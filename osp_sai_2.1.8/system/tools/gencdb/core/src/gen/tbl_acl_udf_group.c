
#include "proto.h"
#include "gen/tbl_acl_udf_group_define.h"
#include "gen/tbl_acl_udf_group.h"
#include "cdb_data_cmp.h"

tbl_acl_udf_group_master_t *_g_p_tbl_acl_udf_group_master = NULL;

int32
tbl_acl_udf_group_add_acl_udf_group_sync(tbl_acl_udf_group_t *p_acl_udf_group, uint32 sync)
{
    tbl_acl_udf_group_master_t *p_master = _g_p_tbl_acl_udf_group_master;
    tbl_acl_udf_group_t *p_db_acl_udf_group = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_udf_group_get_acl_udf_group(&p_acl_udf_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_udf_group = XCALLOC(MEM_TBL_ACL_UDF_GROUP, sizeof(tbl_acl_udf_group_t));
    if (NULL == p_db_acl_udf_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_udf_group, p_acl_udf_group, sizeof(tbl_acl_udf_group_t));

    /* 4. add to db */
    p_master->acl_udf_group_array[p_acl_udf_group->key.id] = p_db_acl_udf_group;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_UDF_GROUP, p_db_acl_udf_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_UDF_GROUP, p_db_acl_udf_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_group_del_acl_udf_group_sync(tbl_acl_udf_group_key_t *p_acl_udf_group_key, uint32 sync)
{
    tbl_acl_udf_group_master_t *p_master = _g_p_tbl_acl_udf_group_master;
    tbl_acl_udf_group_t *p_db_acl_udf_group = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_group = tbl_acl_udf_group_get_acl_udf_group(p_acl_udf_group_key);
    if (NULL == p_db_acl_udf_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_UDF_GROUP, p_db_acl_udf_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_UDF_GROUP, p_db_acl_udf_group);
        #endif 
    }

    /* 3. delete from db */
    p_master->acl_udf_group_array[p_acl_udf_group_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_UDF_GROUP, p_db_acl_udf_group);

    return PM_E_NONE;
}

int32
tbl_acl_udf_group_set_acl_udf_group_field_sync(tbl_acl_udf_group_t *p_acl_udf_group, tbl_acl_udf_group_field_id_t field_id, uint32 sync)
{
    tbl_acl_udf_group_t *p_db_acl_udf_group = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_group = tbl_acl_udf_group_get_acl_udf_group(&p_acl_udf_group->key);
    if (NULL == p_db_acl_udf_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE:
        p_db_acl_udf_group->group_type = p_acl_udf_group->group_type;
        break;

    case TBL_ACL_UDF_GROUP_FLD_IP_VERSION:
        p_db_acl_udf_group->ip_version = p_acl_udf_group->ip_version;
        break;

    case TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL:
        p_db_acl_udf_group->l3_protocol = p_acl_udf_group->l3_protocol;
        break;

    case TBL_ACL_UDF_GROUP_FLD_OFFSET:
        p_db_acl_udf_group->offset = p_acl_udf_group->offset;
        break;

    case TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID:
        p_db_acl_udf_group->udf_match_oid = p_acl_udf_group->udf_match_oid;
        break;

    case TBL_ACL_UDF_GROUP_FLD_UDF_OID:
        p_db_acl_udf_group->udf_oid = p_acl_udf_group->udf_oid;
        break;

    case TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID:
        p_db_acl_udf_group->udf_group_oid = p_acl_udf_group->udf_group_oid;
        break;

    case TBL_ACL_UDF_GROUP_FLD_CNT:
        p_db_acl_udf_group->cnt = p_acl_udf_group->cnt;
        break;

    case TBL_ACL_UDF_GROUP_FLD_MAX:
        sal_memcpy(p_db_acl_udf_group, p_acl_udf_group, sizeof(tbl_acl_udf_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_UDF_GROUP, field_id, p_db_acl_udf_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_UDF_GROUP, field_id, p_db_acl_udf_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_group_add_acl_udf_group(tbl_acl_udf_group_t *p_acl_udf_group)
{
    return tbl_acl_udf_group_add_acl_udf_group_sync(p_acl_udf_group, TRUE);
}

int32
tbl_acl_udf_group_del_acl_udf_group(tbl_acl_udf_group_key_t *p_acl_udf_group_key)
{
    return tbl_acl_udf_group_del_acl_udf_group_sync(p_acl_udf_group_key, TRUE);
}

int32
tbl_acl_udf_group_set_acl_udf_group_field(tbl_acl_udf_group_t *p_acl_udf_group, tbl_acl_udf_group_field_id_t field_id)
{
    return tbl_acl_udf_group_set_acl_udf_group_field_sync(p_acl_udf_group, field_id, TRUE);
}

tbl_acl_udf_group_t*
tbl_acl_udf_group_get_acl_udf_group(tbl_acl_udf_group_key_t *p_acl_udf_group_key)
{
    tbl_acl_udf_group_master_t *p_master = _g_p_tbl_acl_udf_group_master;

    if (GLB_ACL_UDF_GROUP_NUM_MAX <= p_acl_udf_group_key->id)
    {
        return NULL;
    }
    return (p_master->acl_udf_group_array[p_acl_udf_group_key->id]);
}

char*
tbl_acl_udf_group_key_val2str(tbl_acl_udf_group_t *p_acl_udf_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_udf_group->key.id);
    return str;
}

int32
tbl_acl_udf_group_key_str2val(char *str, tbl_acl_udf_group_t *p_acl_udf_group)
{
    int32 ret = 0;

    p_acl_udf_group->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_udf_group_key_name_dump(tbl_acl_udf_group_t *p_acl_udf_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_acl_udf_group->key.id);
    return str;
}

char*
tbl_acl_udf_group_key_value_dump(tbl_acl_udf_group_t *p_acl_udf_group, char *str)
{
    sal_sprintf(str, "%u", p_acl_udf_group->key.id);
    return str;
}

char*
tbl_acl_udf_group_field_name_dump(tbl_acl_udf_group_t *p_acl_udf_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_UDF_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_udf_group_field_value_dump(tbl_acl_udf_group_t *p_acl_udf_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->key.id);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->group_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_IP_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->ip_version);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->l3_protocol);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_OFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->offset);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_udf_group->udf_match_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_udf_group->udf_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_udf_group->udf_group_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_group->cnt);
    }
    return str;
}

char**
tbl_acl_udf_group_table_dump(tbl_acl_udf_group_t *p_acl_udf_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_udf_group_key_name_dump(p_acl_udf_group, buf));
    for(i=1; i<TBL_ACL_UDF_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_udf_group_field_name_dump(p_acl_udf_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_udf_group_field_value_dump(p_acl_udf_group, i, buf));
    }
    return str;
}

int32
tbl_acl_udf_group_field_value_parser(char *str, int32 field_id, tbl_acl_udf_group_t *p_acl_udf_group)
{
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_udf_group->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE, field_id))
    {
        int32 ret;
        p_acl_udf_group->group_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_IP_VERSION, field_id))
    {
        int32 ret;
        p_acl_udf_group->ip_version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL, field_id))
    {
        int32 ret;
        p_acl_udf_group->l3_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_OFFSET, field_id))
    {
        int32 ret;
        p_acl_udf_group->offset = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID, field_id))
    {
        int32 ret;
        p_acl_udf_group->udf_match_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_OID, field_id))
    {
        int32 ret;
        p_acl_udf_group->udf_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID, field_id))
    {
        int32 ret;
        p_acl_udf_group->udf_group_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_CNT, field_id))
    {
        int32 ret;
        p_acl_udf_group->cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_udf_group_table_parser(char** array, char* key_value,tbl_acl_udf_group_t *p_acl_udf_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_udf_group_key_str2val(key_value, p_acl_udf_group));

    for(i=1; i<TBL_ACL_UDF_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_udf_group_field_value_parser( array[j++], i, p_acl_udf_group));
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_group_dump_one(tbl_acl_udf_group_t *p_acl_udf_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_KEY].name,
            p_acl_udf_group->key.id);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE].name,
            p_acl_udf_group->group_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_IP_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_IP_VERSION].name,
            p_acl_udf_group->ip_version);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL].name,
            p_acl_udf_group->l3_protocol);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_OFFSET].name,
            p_acl_udf_group->offset);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID].name,
            p_acl_udf_group->udf_match_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_UDF_OID].name,
            p_acl_udf_group->udf_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID].name,
            p_acl_udf_group->udf_group_oid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_GROUP_FLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_GROUP_FLD_CNT].name,
            p_acl_udf_group->cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_udf_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_udf_group_master_t *p_master = _g_p_tbl_acl_udf_group_master;
    tbl_acl_udf_group_t *p_db_acl_udf_group = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_ACL_UDF_GROUP_NUM_MAX; id++)
    {
        p_db_acl_udf_group = p_master->acl_udf_group_array[id];
        if (NULL == p_db_acl_udf_group)
        {
            continue;
        }
        rc |= fn(p_db_acl_udf_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_udf_group_master_t*
tbl_acl_udf_group_get_master()
{
    return _g_p_tbl_acl_udf_group_master;
}

tbl_acl_udf_group_master_t*
tbl_acl_udf_group_init_acl_udf_group()
{
    _g_p_tbl_acl_udf_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_udf_group_master_t));
    return _g_p_tbl_acl_udf_group_master;
}


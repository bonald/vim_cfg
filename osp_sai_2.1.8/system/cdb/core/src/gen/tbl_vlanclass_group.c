
#include "proto.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "cdb_data_cmp.h"

tbl_vlanclass_group_master_t *_g_p_tbl_vlanclass_group_master = NULL;

int32
tbl_vlanclass_group_add_vlanclass_group_sync(tbl_vlanclass_group_t *p_vlanclass_group, uint32 sync)
{
    tbl_vlanclass_group_master_t *p_master = _g_p_tbl_vlanclass_group_master;
    tbl_vlanclass_group_t *p_db_vlanclass_group = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlanclass_group_get_vlanclass_group(&p_vlanclass_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlanclass_group = XCALLOC(MEM_TBL_VLANCLASS_GROUP, sizeof(tbl_vlanclass_group_t));
    if (NULL == p_db_vlanclass_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlanclass_group, p_vlanclass_group, sizeof(tbl_vlanclass_group_t));

    /* 4. add to db */
    p_master->vlanclass_group_array[p_vlanclass_group->key.group_id] = p_db_vlanclass_group;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLANCLASS_GROUP, p_db_vlanclass_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLANCLASS_GROUP, p_db_vlanclass_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_group_del_vlanclass_group_sync(tbl_vlanclass_group_key_t *p_vlanclass_group_key, uint32 sync)
{
    tbl_vlanclass_group_master_t *p_master = _g_p_tbl_vlanclass_group_master;
    tbl_vlanclass_group_t *p_db_vlanclass_group = NULL;

    /* 1. lookup entry exist */
    p_db_vlanclass_group = tbl_vlanclass_group_get_vlanclass_group(p_vlanclass_group_key);
    if (NULL == p_db_vlanclass_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLANCLASS_GROUP, p_db_vlanclass_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLANCLASS_GROUP, p_db_vlanclass_group);
        #endif 
    }

    /* 3. delete from db */
    p_master->vlanclass_group_array[p_vlanclass_group_key->group_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VLANCLASS_GROUP, p_db_vlanclass_group);

    return PM_E_NONE;
}

int32
tbl_vlanclass_group_set_vlanclass_group_field_sync(tbl_vlanclass_group_t *p_vlanclass_group, tbl_vlanclass_group_field_id_t field_id, uint32 sync)
{
    tbl_vlanclass_group_t *p_db_vlanclass_group = NULL;

    /* 1. lookup entry exist */
    p_db_vlanclass_group = tbl_vlanclass_group_get_vlanclass_group(&p_vlanclass_group->key);
    if (NULL == p_db_vlanclass_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLANCLASS_GROUP_FLD_RULE_ID:
        sal_memcpy(p_db_vlanclass_group->rule_id, p_vlanclass_group->rule_id, sizeof(p_vlanclass_group->rule_id));
        break;

    case TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID:
        p_db_vlanclass_group->action_rule_id = p_vlanclass_group->action_rule_id;
        break;

    case TBL_VLANCLASS_GROUP_FLD_GROUP_OID:
        p_db_vlanclass_group->group_oid = p_vlanclass_group->group_oid;
        break;

    case TBL_VLANCLASS_GROUP_FLD_MAX:
        sal_memcpy(p_db_vlanclass_group, p_vlanclass_group, sizeof(tbl_vlanclass_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLANCLASS_GROUP, field_id, p_db_vlanclass_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLANCLASS_GROUP, field_id, p_db_vlanclass_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_group_add_vlanclass_group(tbl_vlanclass_group_t *p_vlanclass_group)
{
    return tbl_vlanclass_group_add_vlanclass_group_sync(p_vlanclass_group, TRUE);
}

int32
tbl_vlanclass_group_del_vlanclass_group(tbl_vlanclass_group_key_t *p_vlanclass_group_key)
{
    return tbl_vlanclass_group_del_vlanclass_group_sync(p_vlanclass_group_key, TRUE);
}

int32
tbl_vlanclass_group_set_vlanclass_group_field(tbl_vlanclass_group_t *p_vlanclass_group, tbl_vlanclass_group_field_id_t field_id)
{
    return tbl_vlanclass_group_set_vlanclass_group_field_sync(p_vlanclass_group, field_id, TRUE);
}

tbl_vlanclass_group_t*
tbl_vlanclass_group_get_vlanclass_group(tbl_vlanclass_group_key_t *p_vlanclass_group_key)
{
    tbl_vlanclass_group_master_t *p_master = _g_p_tbl_vlanclass_group_master;

    if (GLB_VLANCLASS_GROUP_ID_MAX+1 <= p_vlanclass_group_key->group_id)
    {
        return NULL;
    }
    return (p_master->vlanclass_group_array[p_vlanclass_group_key->group_id]);
}

char*
tbl_vlanclass_group_key_val2str(tbl_vlanclass_group_t *p_vlanclass_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlanclass_group->key.group_id);
    return str;
}

int32
tbl_vlanclass_group_key_str2val(char *str, tbl_vlanclass_group_t *p_vlanclass_group)
{
    int32 ret = 0;

    p_vlanclass_group->key.group_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vlanclass_group_key_name_dump(tbl_vlanclass_group_t *p_vlanclass_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vlanclass_group->key.group_id);
    return str;
}

char*
tbl_vlanclass_group_key_value_dump(tbl_vlanclass_group_t *p_vlanclass_group, char *str)
{
    sal_sprintf(str, "%u", p_vlanclass_group->key.group_id);
    return str;
}

char*
tbl_vlanclass_group_field_name_dump(tbl_vlanclass_group_t *p_vlanclass_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLANCLASS_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlanclass_group_field_value_dump(tbl_vlanclass_group_t *p_vlanclass_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_group->key.group_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_RULE_ID, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_vlanclass_group->rule_id, sizeof(p_vlanclass_group->rule_id)/4);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlanclass_group->action_rule_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_GROUP_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlanclass_group->group_oid);
    }
    return str;
}

char**
tbl_vlanclass_group_table_dump(tbl_vlanclass_group_t *p_vlanclass_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlanclass_group_key_name_dump(p_vlanclass_group, buf));
    for(i=1; i<TBL_VLANCLASS_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlanclass_group_field_name_dump(p_vlanclass_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlanclass_group_field_value_dump(p_vlanclass_group, i, buf));
    }
    return str;
}

int32
tbl_vlanclass_group_field_value_parser(char *str, int32 field_id, tbl_vlanclass_group_t *p_vlanclass_group)
{
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_vlanclass_group->key.group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_RULE_ID, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_vlanclass_group->rule_id, sizeof(p_vlanclass_group->rule_id)/4, err);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID, field_id))
    {
        int32 ret;
        p_vlanclass_group->action_rule_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_GROUP_OID, field_id))
    {
        int32 ret;
        p_vlanclass_group->group_oid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vlanclass_group_table_parser(char** array, char* key_value,tbl_vlanclass_group_t *p_vlanclass_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlanclass_group_key_str2val(key_value, p_vlanclass_group));

    for(i=1; i<TBL_VLANCLASS_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlanclass_group_field_value_parser( array[j++], i, p_vlanclass_group));
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_group_dump_one(tbl_vlanclass_group_t *p_vlanclass_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_GROUP_FLD_KEY].name,
            p_vlanclass_group->key.group_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_RULE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLANCLASS_GROUP_FLD_RULE_ID].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_vlanclass_group->rule_id, sizeof(p_vlanclass_group->rule_id)/4));
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID].name,
            p_vlanclass_group->action_rule_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_GROUP_FLD_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLANCLASS_GROUP_FLD_GROUP_OID].name,
            p_vlanclass_group->group_oid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlanclass_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlanclass_group_master_t *p_master = _g_p_tbl_vlanclass_group_master;
    tbl_vlanclass_group_t *p_db_vlanclass_group = NULL;
    uint32 group_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (group_id = 0; group_id < GLB_VLANCLASS_GROUP_ID_MAX+1; group_id++)
    {
        p_db_vlanclass_group = p_master->vlanclass_group_array[group_id];
        if (NULL == p_db_vlanclass_group)
        {
            continue;
        }
        rc |= fn(p_db_vlanclass_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlanclass_group_master_t*
tbl_vlanclass_group_get_master()
{
    return _g_p_tbl_vlanclass_group_master;
}

tbl_vlanclass_group_master_t*
tbl_vlanclass_group_init_vlanclass_group()
{
    _g_p_tbl_vlanclass_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlanclass_group_master_t));
    return _g_p_tbl_vlanclass_group_master;
}


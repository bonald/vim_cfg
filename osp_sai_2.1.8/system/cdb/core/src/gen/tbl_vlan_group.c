
#include "proto.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "cdb_data_cmp.h"

tbl_vlan_group_master_t *_g_p_tbl_vlan_group_master = NULL;

int32
tbl_vlan_group_add_vlan_group_sync(tbl_vlan_group_t *p_vlan_group, uint32 sync)
{
    tbl_vlan_group_master_t *p_master = _g_p_tbl_vlan_group_master;
    tbl_vlan_group_t *p_db_vlan_group = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlan_group_get_vlan_group(&p_vlan_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan_group = XCALLOC(MEM_TBL_VLAN_GROUP, sizeof(tbl_vlan_group_t));
    if (NULL == p_db_vlan_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan_group, p_vlan_group, sizeof(tbl_vlan_group_t));

    /* 4. add to db */
    p_master->vlan_group_array[p_vlan_group->key.group_id] = p_db_vlan_group;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLAN_GROUP, p_db_vlan_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLAN_GROUP, p_db_vlan_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_group_del_vlan_group_sync(tbl_vlan_group_key_t *p_vlan_group_key, uint32 sync)
{
    tbl_vlan_group_master_t *p_master = _g_p_tbl_vlan_group_master;
    tbl_vlan_group_t *p_db_vlan_group = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_group = tbl_vlan_group_get_vlan_group(p_vlan_group_key);
    if (NULL == p_db_vlan_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLAN_GROUP, p_db_vlan_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLAN_GROUP, p_db_vlan_group);
        #endif 
    }

    /* 3. delete from db */
    p_master->vlan_group_array[p_vlan_group_key->group_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VLAN_GROUP, p_db_vlan_group);

    return PM_E_NONE;
}

int32
tbl_vlan_group_set_vlan_group_field_sync(tbl_vlan_group_t *p_vlan_group, tbl_vlan_group_field_id_t field_id, uint32 sync)
{
    tbl_vlan_group_t *p_db_vlan_group = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_group = tbl_vlan_group_get_vlan_group(&p_vlan_group->key);
    if (NULL == p_db_vlan_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLAN_GROUP_FLD_VALID:
        sal_memcpy(p_db_vlan_group->valid, p_vlan_group->valid, sizeof(p_vlan_group->valid));
        break;

    case TBL_VLAN_GROUP_FLD_MIN_VID:
        sal_memcpy(p_db_vlan_group->min_vid, p_vlan_group->min_vid, sizeof(p_vlan_group->min_vid));
        break;

    case TBL_VLAN_GROUP_FLD_MAX_VID:
        sal_memcpy(p_db_vlan_group->max_vid, p_vlan_group->max_vid, sizeof(p_vlan_group->max_vid));
        break;

    case TBL_VLAN_GROUP_FLD_MAX:
        sal_memcpy(p_db_vlan_group, p_vlan_group, sizeof(tbl_vlan_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLAN_GROUP, field_id, p_db_vlan_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLAN_GROUP, field_id, p_db_vlan_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_group_add_vlan_group(tbl_vlan_group_t *p_vlan_group)
{
    return tbl_vlan_group_add_vlan_group_sync(p_vlan_group, TRUE);
}

int32
tbl_vlan_group_del_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key)
{
    return tbl_vlan_group_del_vlan_group_sync(p_vlan_group_key, TRUE);
}

int32
tbl_vlan_group_set_vlan_group_field(tbl_vlan_group_t *p_vlan_group, tbl_vlan_group_field_id_t field_id)
{
    return tbl_vlan_group_set_vlan_group_field_sync(p_vlan_group, field_id, TRUE);
}

tbl_vlan_group_t*
tbl_vlan_group_get_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key)
{
    tbl_vlan_group_master_t *p_master = _g_p_tbl_vlan_group_master;

    if (GLB_VLAN_RANGE_GROUP_NUM+1 <= p_vlan_group_key->group_id)
    {
        return NULL;
    }
    return (p_master->vlan_group_array[p_vlan_group_key->group_id]);
}

char*
tbl_vlan_group_key_val2str(tbl_vlan_group_t *p_vlan_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlan_group->key.group_id);
    return str;
}

int32
tbl_vlan_group_key_str2val(char *str, tbl_vlan_group_t *p_vlan_group)
{
    int32 ret = 0;

    p_vlan_group->key.group_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vlan_group_key_name_dump(tbl_vlan_group_t *p_vlan_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vlan_group->key.group_id);
    return str;
}

char*
tbl_vlan_group_key_value_dump(tbl_vlan_group_t *p_vlan_group, char *str)
{
    sal_sprintf(str, "%u", p_vlan_group->key.group_id);
    return str;
}

char*
tbl_vlan_group_field_name_dump(tbl_vlan_group_t *p_vlan_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLAN_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlan_group_field_value_dump(tbl_vlan_group_t *p_vlan_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_group->key.group_id);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_VALID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_vlan_group->valid, sizeof(p_vlan_group->valid)/4);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MIN_VID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_vlan_group->min_vid, sizeof(p_vlan_group->min_vid)/4);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MAX_VID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_vlan_group->max_vid, sizeof(p_vlan_group->max_vid)/4);
    }
    return str;
}

char**
tbl_vlan_group_table_dump(tbl_vlan_group_t *p_vlan_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlan_group_key_name_dump(p_vlan_group, buf));
    for(i=1; i<TBL_VLAN_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlan_group_field_name_dump(p_vlan_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlan_group_field_value_dump(p_vlan_group, i, buf));
    }
    return str;
}

int32
tbl_vlan_group_field_value_parser(char *str, int32 field_id, tbl_vlan_group_t *p_vlan_group)
{
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_vlan_group->key.group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_VALID, field_id))
    {
        cdb_uint32_array_str2val(str, p_vlan_group->valid);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MIN_VID, field_id))
    {
        cdb_uint32_array_str2val(str, p_vlan_group->min_vid);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MAX_VID, field_id))
    {
        cdb_uint32_array_str2val(str, p_vlan_group->max_vid);
    }
    return PM_E_NONE;
}

int32
tbl_vlan_group_table_parser(char** array, char* key_value,tbl_vlan_group_t *p_vlan_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlan_group_key_str2val(key_value, p_vlan_group));

    for(i=1; i<TBL_VLAN_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlan_group_field_value_parser( array[j++], i, p_vlan_group));
    }

    return PM_E_NONE;
}

int32
tbl_vlan_group_dump_one(tbl_vlan_group_t *p_vlan_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_GROUP_FLD_KEY].name,
            p_vlan_group->key.group_id);
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_GROUP_FLD_VALID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_vlan_group->valid, sizeof(p_vlan_group->valid)/4));
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MIN_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_GROUP_FLD_MIN_VID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_vlan_group->min_vid, sizeof(p_vlan_group->min_vid)/4));
    }
    if (FLD_MATCH(TBL_VLAN_GROUP_FLD_MAX_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_GROUP_FLD_MAX_VID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_vlan_group->max_vid, sizeof(p_vlan_group->max_vid)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlan_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlan_group_master_t *p_master = _g_p_tbl_vlan_group_master;
    tbl_vlan_group_t *p_db_vlan_group = NULL;
    uint32 group_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (group_id = 0; group_id < GLB_VLAN_RANGE_GROUP_NUM+1; group_id++)
    {
        p_db_vlan_group = p_master->vlan_group_array[group_id];
        if (NULL == p_db_vlan_group)
        {
            continue;
        }
        rc |= fn(p_db_vlan_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlan_group_master_t*
tbl_vlan_group_get_master()
{
    return _g_p_tbl_vlan_group_master;
}

tbl_vlan_group_master_t*
tbl_vlan_group_init_vlan_group()
{
    _g_p_tbl_vlan_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlan_group_master_t));
    return _g_p_tbl_vlan_group_master;
}


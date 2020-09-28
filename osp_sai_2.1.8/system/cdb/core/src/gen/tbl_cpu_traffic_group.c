
#include "proto.h"
#include "gen/tbl_cpu_traffic_group_define.h"
#include "gen/tbl_cpu_traffic_group.h"
#include "cdb_data_cmp.h"

tbl_cpu_traffic_group_master_t *_g_p_tbl_cpu_traffic_group_master = NULL;

int32
tbl_cpu_traffic_group_add_cpu_traffic_group_sync(tbl_cpu_traffic_group_t *p_cpu_traffic_group, uint32 sync)
{
    tbl_cpu_traffic_group_master_t *p_master = _g_p_tbl_cpu_traffic_group_master;
    tbl_cpu_traffic_group_t *p_db_cpu_traffic_group = NULL;

    /* 1. lookup entry exist */
    if (tbl_cpu_traffic_group_get_cpu_traffic_group(&p_cpu_traffic_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cpu_traffic_group = XCALLOC(MEM_TBL_CPU_TRAFFIC_GROUP, sizeof(tbl_cpu_traffic_group_t));
    if (NULL == p_db_cpu_traffic_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cpu_traffic_group, p_cpu_traffic_group, sizeof(tbl_cpu_traffic_group_t));

    /* 4. add to db */
    p_master->array[p_cpu_traffic_group->key.class_id] = p_db_cpu_traffic_group;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CPU_TRAFFIC_GROUP, p_db_cpu_traffic_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CPU_TRAFFIC_GROUP, p_db_cpu_traffic_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_del_cpu_traffic_group_sync(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key, uint32 sync)
{
    tbl_cpu_traffic_group_master_t *p_master = _g_p_tbl_cpu_traffic_group_master;
    tbl_cpu_traffic_group_t *p_db_cpu_traffic_group = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(p_cpu_traffic_group_key);
    if (NULL == p_db_cpu_traffic_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CPU_TRAFFIC_GROUP, p_db_cpu_traffic_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CPU_TRAFFIC_GROUP, p_db_cpu_traffic_group);
        #endif 
    }

    /* 3. delete from db */
    p_master->array[p_cpu_traffic_group_key->class_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_CPU_TRAFFIC_GROUP, p_db_cpu_traffic_group);

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_set_cpu_traffic_group_field_sync(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_cpu_traffic_group_field_id_t field_id, uint32 sync)
{
    tbl_cpu_traffic_group_t *p_db_cpu_traffic_group = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(&p_cpu_traffic_group->key);
    if (NULL == p_db_cpu_traffic_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_TRAFFIC_GROUP_FLD_RATE:
        p_db_cpu_traffic_group->rate = p_cpu_traffic_group->rate;
        break;

    case TBL_CPU_TRAFFIC_GROUP_FLD_MAX:
        sal_memcpy(p_db_cpu_traffic_group, p_cpu_traffic_group, sizeof(tbl_cpu_traffic_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_TRAFFIC_GROUP, field_id, p_db_cpu_traffic_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_TRAFFIC_GROUP, field_id, p_db_cpu_traffic_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_add_cpu_traffic_group(tbl_cpu_traffic_group_t *p_cpu_traffic_group)
{
    return tbl_cpu_traffic_group_add_cpu_traffic_group_sync(p_cpu_traffic_group, TRUE);
}

int32
tbl_cpu_traffic_group_del_cpu_traffic_group(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key)
{
    return tbl_cpu_traffic_group_del_cpu_traffic_group_sync(p_cpu_traffic_group_key, TRUE);
}

int32
tbl_cpu_traffic_group_set_cpu_traffic_group_field(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_cpu_traffic_group_field_id_t field_id)
{
    return tbl_cpu_traffic_group_set_cpu_traffic_group_field_sync(p_cpu_traffic_group, field_id, TRUE);
}

tbl_cpu_traffic_group_t*
tbl_cpu_traffic_group_get_cpu_traffic_group(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key)
{
    tbl_cpu_traffic_group_master_t *p_master = _g_p_tbl_cpu_traffic_group_master;

    if (GLB_CPU_TRAFFIC_GROUP_MAX <= p_cpu_traffic_group_key->class_id)
    {
        return NULL;
    }
    return (p_master->array[p_cpu_traffic_group_key->class_id]);
}

char*
tbl_cpu_traffic_group_key_val2str(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_cpu_traffic_group->key.class_id);
    return str;
}

int32
tbl_cpu_traffic_group_key_str2val(char *str, tbl_cpu_traffic_group_t *p_cpu_traffic_group)
{
    int32 ret = 0;

    p_cpu_traffic_group->key.class_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_cpu_traffic_group_key_name_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_cpu_traffic_group->key.class_id);
    return str;
}

char*
tbl_cpu_traffic_group_key_value_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char *str)
{
    sal_sprintf(str, "%u", p_cpu_traffic_group->key.class_id);
    return str;
}

char*
tbl_cpu_traffic_group_field_name_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_TRAFFIC_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_traffic_group_field_value_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_traffic_group->key.class_id);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_traffic_group->rate);
    }
    return str;
}

char**
tbl_cpu_traffic_group_table_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_traffic_group_key_name_dump(p_cpu_traffic_group, buf));
    for(i=1; i<TBL_CPU_TRAFFIC_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_traffic_group_field_name_dump(p_cpu_traffic_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_traffic_group_field_value_dump(p_cpu_traffic_group, i, buf));
    }
    return str;
}

int32
tbl_cpu_traffic_group_field_value_parser(char *str, int32 field_id, tbl_cpu_traffic_group_t *p_cpu_traffic_group)
{
    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_cpu_traffic_group->key.class_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_RATE, field_id))
    {
        int32 ret;
        p_cpu_traffic_group->rate = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_table_parser(char** array, char* key_value,tbl_cpu_traffic_group_t *p_cpu_traffic_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_traffic_group_key_str2val(key_value, p_cpu_traffic_group));

    for(i=1; i<TBL_CPU_TRAFFIC_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_traffic_group_field_value_parser( array[j++], i, p_cpu_traffic_group));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_dump_one(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_TRAFFIC_GROUP_FLD_KEY].name,
            p_cpu_traffic_group->key.class_id);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_GROUP_FLD_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_TRAFFIC_GROUP_FLD_RATE].name,
            p_cpu_traffic_group->rate);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_traffic_group_master_t *p_master = _g_p_tbl_cpu_traffic_group_master;
    tbl_cpu_traffic_group_t *p_db_cpu_traffic_group = NULL;
    uint32 class_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (class_id = 0; class_id < GLB_CPU_TRAFFIC_GROUP_MAX; class_id++)
    {
        p_db_cpu_traffic_group = p_master->array[class_id];
        if (NULL == p_db_cpu_traffic_group)
        {
            continue;
        }
        rc |= fn(p_db_cpu_traffic_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_traffic_group_master_t*
tbl_cpu_traffic_group_get_master()
{
    return _g_p_tbl_cpu_traffic_group_master;
}

tbl_cpu_traffic_group_master_t*
tbl_cpu_traffic_group_init_cpu_traffic_group()
{
    _g_p_tbl_cpu_traffic_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cpu_traffic_group_master_t));
    return _g_p_tbl_cpu_traffic_group_master;
}


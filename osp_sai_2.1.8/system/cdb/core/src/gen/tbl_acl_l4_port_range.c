
#include "proto.h"
#include "gen/tbl_acl_l4_port_range_define.h"
#include "gen/tbl_acl_l4_port_range.h"
#include "cdb_data_cmp.h"

tbl_acl_l4_port_range_master_t *_g_p_tbl_acl_l4_port_range_master = NULL;

int32
tbl_acl_l4_port_range_add_acl_l4_port_range_sync(tbl_acl_l4_port_range_t *p_acl_l4_port_range, uint32 sync)
{
    tbl_acl_l4_port_range_master_t *p_master = _g_p_tbl_acl_l4_port_range_master;
    tbl_acl_l4_port_range_t *p_db_acl_l4_port_range = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_l4_port_range_get_acl_l4_port_range(&p_acl_l4_port_range->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_l4_port_range = XCALLOC(MEM_TBL_ACL_L4_PORT_RANGE, sizeof(tbl_acl_l4_port_range_t));
    if (NULL == p_db_acl_l4_port_range)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_l4_port_range, p_acl_l4_port_range, sizeof(tbl_acl_l4_port_range_t));

    /* 4. add to db */
    p_master->acl_l4_port_range_array[p_acl_l4_port_range->key.id] = p_db_acl_l4_port_range;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_L4_PORT_RANGE, p_db_acl_l4_port_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_L4_PORT_RANGE, p_db_acl_l4_port_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_del_acl_l4_port_range_sync(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key, uint32 sync)
{
    tbl_acl_l4_port_range_master_t *p_master = _g_p_tbl_acl_l4_port_range_master;
    tbl_acl_l4_port_range_t *p_db_acl_l4_port_range = NULL;

    /* 1. lookup entry exist */
    p_db_acl_l4_port_range = tbl_acl_l4_port_range_get_acl_l4_port_range(p_acl_l4_port_range_key);
    if (NULL == p_db_acl_l4_port_range)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_L4_PORT_RANGE, p_db_acl_l4_port_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_L4_PORT_RANGE, p_db_acl_l4_port_range);
        #endif 
    }

    /* 3. delete from db */
    p_master->acl_l4_port_range_array[p_acl_l4_port_range_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_L4_PORT_RANGE, p_db_acl_l4_port_range);

    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_set_acl_l4_port_range_field_sync(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_acl_l4_port_range_field_id_t field_id, uint32 sync)
{
    tbl_acl_l4_port_range_t *p_db_acl_l4_port_range = NULL;

    /* 1. lookup entry exist */
    p_db_acl_l4_port_range = tbl_acl_l4_port_range_get_acl_l4_port_range(&p_acl_l4_port_range->key);
    if (NULL == p_db_acl_l4_port_range)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN:
        p_db_acl_l4_port_range->port_min = p_acl_l4_port_range->port_min;
        break;

    case TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX:
        p_db_acl_l4_port_range->port_max = p_acl_l4_port_range->port_max;
        break;

    case TBL_ACL_L4_PORT_RANGE_FLD_CNT:
        p_db_acl_l4_port_range->cnt = p_acl_l4_port_range->cnt;
        break;

    case TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC:
        p_db_acl_l4_port_range->is_src = p_acl_l4_port_range->is_src;
        break;

    case TBL_ACL_L4_PORT_RANGE_FLD_MAX:
        sal_memcpy(p_db_acl_l4_port_range, p_acl_l4_port_range, sizeof(tbl_acl_l4_port_range_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_L4_PORT_RANGE, field_id, p_db_acl_l4_port_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_L4_PORT_RANGE, field_id, p_db_acl_l4_port_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_add_acl_l4_port_range(tbl_acl_l4_port_range_t *p_acl_l4_port_range)
{
    return tbl_acl_l4_port_range_add_acl_l4_port_range_sync(p_acl_l4_port_range, TRUE);
}

int32
tbl_acl_l4_port_range_del_acl_l4_port_range(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key)
{
    return tbl_acl_l4_port_range_del_acl_l4_port_range_sync(p_acl_l4_port_range_key, TRUE);
}

int32
tbl_acl_l4_port_range_set_acl_l4_port_range_field(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_acl_l4_port_range_field_id_t field_id)
{
    return tbl_acl_l4_port_range_set_acl_l4_port_range_field_sync(p_acl_l4_port_range, field_id, TRUE);
}

tbl_acl_l4_port_range_t*
tbl_acl_l4_port_range_get_acl_l4_port_range(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key)
{
    tbl_acl_l4_port_range_master_t *p_master = _g_p_tbl_acl_l4_port_range_master;

    if (GLB_ACL_L4PORT_ACE_NUM_MAX <= p_acl_l4_port_range_key->id)
    {
        return NULL;
    }
    return (p_master->acl_l4_port_range_array[p_acl_l4_port_range_key->id]);
}

char*
tbl_acl_l4_port_range_key_val2str(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_l4_port_range->key.id);
    return str;
}

int32
tbl_acl_l4_port_range_key_str2val(char *str, tbl_acl_l4_port_range_t *p_acl_l4_port_range)
{
    int32 ret = 0;

    p_acl_l4_port_range->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_l4_port_range_key_name_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_L4_PORT_RANGE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_acl_l4_port_range->key.id);
    return str;
}

char*
tbl_acl_l4_port_range_key_value_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char *str)
{
    sal_sprintf(str, "%u", p_acl_l4_port_range->key.id);
    return str;
}

char*
tbl_acl_l4_port_range_field_name_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_L4_PORT_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_L4_PORT_RANGE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_l4_port_range_field_value_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_l4_port_range->key.id);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_l4_port_range->port_min);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_acl_l4_port_range->port_max);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_l4_port_range->cnt);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC, field_id))
    {
        sal_sprintf(str, "%u", p_acl_l4_port_range->is_src);
    }
    return str;
}

char**
tbl_acl_l4_port_range_table_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_l4_port_range_key_name_dump(p_acl_l4_port_range, buf));
    for(i=1; i<TBL_ACL_L4_PORT_RANGE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_l4_port_range_field_name_dump(p_acl_l4_port_range, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_l4_port_range_field_value_dump(p_acl_l4_port_range, i, buf));
    }
    return str;
}

int32
tbl_acl_l4_port_range_field_value_parser(char *str, int32 field_id, tbl_acl_l4_port_range_t *p_acl_l4_port_range)
{
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_l4_port_range->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN, field_id))
    {
        int32 ret;
        p_acl_l4_port_range->port_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX, field_id))
    {
        int32 ret;
        p_acl_l4_port_range->port_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_CNT, field_id))
    {
        int32 ret;
        p_acl_l4_port_range->cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC, field_id))
    {
        int32 ret;
        p_acl_l4_port_range->is_src = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_table_parser(char** array, char* key_value,tbl_acl_l4_port_range_t *p_acl_l4_port_range)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_L4_PORT_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_l4_port_range_key_str2val(key_value, p_acl_l4_port_range));

    for(i=1; i<TBL_ACL_L4_PORT_RANGE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_l4_port_range_field_value_parser( array[j++], i, p_acl_l4_port_range));
    }

    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_dump_one(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_L4_PORT_RANGE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_L4_PORT_RANGE_FLD_KEY].name,
            p_acl_l4_port_range->key.id);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN].name,
            p_acl_l4_port_range->port_min);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX].name,
            p_acl_l4_port_range->port_max);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_L4_PORT_RANGE_FLD_CNT].name,
            p_acl_l4_port_range->cnt);
    }
    if (FLD_MATCH(TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC].name,
            p_acl_l4_port_range->is_src);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_l4_port_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_l4_port_range_master_t *p_master = _g_p_tbl_acl_l4_port_range_master;
    tbl_acl_l4_port_range_t *p_db_acl_l4_port_range = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_ACL_L4PORT_ACE_NUM_MAX; id++)
    {
        p_db_acl_l4_port_range = p_master->acl_l4_port_range_array[id];
        if (NULL == p_db_acl_l4_port_range)
        {
            continue;
        }
        rc |= fn(p_db_acl_l4_port_range, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_l4_port_range_master_t*
tbl_acl_l4_port_range_get_master()
{
    return _g_p_tbl_acl_l4_port_range_master;
}

tbl_acl_l4_port_range_master_t*
tbl_acl_l4_port_range_init_acl_l4_port_range()
{
    _g_p_tbl_acl_l4_port_range_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_l4_port_range_master_t));
    return _g_p_tbl_acl_l4_port_range_master;
}


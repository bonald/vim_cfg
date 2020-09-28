
#include "proto.h"
#include "gen/tbl_rr_prefix_define.h"
#include "gen/tbl_rr_prefix.h"
#include "cdb_data_cmp.h"

tbl_rr_prefix_master_t *_g_p_tbl_rr_prefix_master = NULL;

int32
tbl_rr_prefix_add_rr_prefix_sync(tbl_rr_prefix_t *p_rr_prefix, uint32 sync)
{
    tbl_rr_prefix_master_t *p_master = _g_p_tbl_rr_prefix_master;
    tbl_rr_prefix_t *p_db_rr_prefix = NULL;

    /* 1. lookup entry exist */
    if (tbl_rr_prefix_get_rr_prefix(&p_rr_prefix->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rr_prefix = XCALLOC(MEM_TBL_RR_PREFIX, sizeof(tbl_rr_prefix_t));
    if (NULL == p_db_rr_prefix)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rr_prefix, p_rr_prefix, sizeof(tbl_rr_prefix_t));

    /* 4. add to db */
    p_master->rr_prefix_array[p_rr_prefix->key.rr_prefix_index] = p_db_rr_prefix;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RR_PREFIX, p_db_rr_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RR_PREFIX, p_db_rr_prefix);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rr_prefix_del_rr_prefix_sync(tbl_rr_prefix_key_t *p_rr_prefix_key, uint32 sync)
{
    tbl_rr_prefix_master_t *p_master = _g_p_tbl_rr_prefix_master;
    tbl_rr_prefix_t *p_db_rr_prefix = NULL;

    /* 1. lookup entry exist */
    p_db_rr_prefix = tbl_rr_prefix_get_rr_prefix(p_rr_prefix_key);
    if (NULL == p_db_rr_prefix)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RR_PREFIX, p_db_rr_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RR_PREFIX, p_db_rr_prefix);
        #endif 
    }

    /* 3. delete from db */
    p_master->rr_prefix_array[p_rr_prefix_key->rr_prefix_index] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_RR_PREFIX, p_db_rr_prefix);

    return PM_E_NONE;
}

int32
tbl_rr_prefix_set_rr_prefix_field_sync(tbl_rr_prefix_t *p_rr_prefix, tbl_rr_prefix_field_id_t field_id, uint32 sync)
{
    tbl_rr_prefix_t *p_db_rr_prefix = NULL;

    /* 1. lookup entry exist */
    p_db_rr_prefix = tbl_rr_prefix_get_rr_prefix(&p_rr_prefix->key);
    if (NULL == p_db_rr_prefix)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RR_PREFIX_FLD_RR_PREFIX:
        sal_memcpy(&p_db_rr_prefix->rr_prefix, &p_rr_prefix->rr_prefix, sizeof(p_rr_prefix->rr_prefix));
        break;

    case TBL_RR_PREFIX_FLD_MAX:
        sal_memcpy(p_db_rr_prefix, p_rr_prefix, sizeof(tbl_rr_prefix_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RR_PREFIX, field_id, p_db_rr_prefix);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RR_PREFIX, field_id, p_db_rr_prefix);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rr_prefix_add_rr_prefix(tbl_rr_prefix_t *p_rr_prefix)
{
    return tbl_rr_prefix_add_rr_prefix_sync(p_rr_prefix, TRUE);
}

int32
tbl_rr_prefix_del_rr_prefix(tbl_rr_prefix_key_t *p_rr_prefix_key)
{
    return tbl_rr_prefix_del_rr_prefix_sync(p_rr_prefix_key, TRUE);
}

int32
tbl_rr_prefix_set_rr_prefix_field(tbl_rr_prefix_t *p_rr_prefix, tbl_rr_prefix_field_id_t field_id)
{
    return tbl_rr_prefix_set_rr_prefix_field_sync(p_rr_prefix, field_id, TRUE);
}

tbl_rr_prefix_t*
tbl_rr_prefix_get_rr_prefix(tbl_rr_prefix_key_t *p_rr_prefix_key)
{
    tbl_rr_prefix_master_t *p_master = _g_p_tbl_rr_prefix_master;

    if (GLB_MAX_ECMP_ROUND_ROBIN+1 <= p_rr_prefix_key->rr_prefix_index)
    {
        return NULL;
    }
    return (p_master->rr_prefix_array[p_rr_prefix_key->rr_prefix_index]);
}

char*
tbl_rr_prefix_key_val2str(tbl_rr_prefix_t *p_rr_prefix, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rr_prefix->key.rr_prefix_index);
    return str;
}

int32
tbl_rr_prefix_key_str2val(char *str, tbl_rr_prefix_t *p_rr_prefix)
{
    int32 ret = 0;

    p_rr_prefix->key.rr_prefix_index = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rr_prefix_key_name_dump(tbl_rr_prefix_t *p_rr_prefix, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RR_PREFIX);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rr_prefix->key.rr_prefix_index);
    return str;
}

char*
tbl_rr_prefix_key_value_dump(tbl_rr_prefix_t *p_rr_prefix, char *str)
{
    sal_sprintf(str, "%u", p_rr_prefix->key.rr_prefix_index);
    return str;
}

char*
tbl_rr_prefix_field_name_dump(tbl_rr_prefix_t *p_rr_prefix, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RR_PREFIX);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RR_PREFIX_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rr_prefix_field_value_dump(tbl_rr_prefix_t *p_rr_prefix, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RR_PREFIX_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rr_prefix->key.rr_prefix_index);
    }
    if (FLD_MATCH(TBL_RR_PREFIX_FLD_RR_PREFIX, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_rr_prefix->rr_prefix);
    }
    return str;
}

char**
tbl_rr_prefix_table_dump(tbl_rr_prefix_t *p_rr_prefix, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rr_prefix_key_name_dump(p_rr_prefix, buf));
    for(i=1; i<TBL_RR_PREFIX_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rr_prefix_field_name_dump(p_rr_prefix, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rr_prefix_field_value_dump(p_rr_prefix, i, buf));
    }
    return str;
}

int32
tbl_rr_prefix_field_value_parser(char *str, int32 field_id, tbl_rr_prefix_t *p_rr_prefix)
{
    if (FLD_MATCH(TBL_RR_PREFIX_FLD_KEY, field_id))
    {
        int32 ret;
        p_rr_prefix->key.rr_prefix_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RR_PREFIX_FLD_RR_PREFIX, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_rr_prefix->rr_prefix);
    }
    return PM_E_NONE;
}

int32
tbl_rr_prefix_table_parser(char** array, char* key_value,tbl_rr_prefix_t *p_rr_prefix)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RR_PREFIX);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rr_prefix_key_str2val(key_value, p_rr_prefix));

    for(i=1; i<TBL_RR_PREFIX_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rr_prefix_field_value_parser( array[j++], i, p_rr_prefix));
    }

    return PM_E_NONE;
}

int32
tbl_rr_prefix_dump_one(tbl_rr_prefix_t *p_rr_prefix, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RR_PREFIX);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RR_PREFIX_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RR_PREFIX_FLD_KEY].name,
            p_rr_prefix->key.rr_prefix_index);
    }
    if (FLD_MATCH(TBL_RR_PREFIX_FLD_RR_PREFIX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RR_PREFIX_FLD_RR_PREFIX].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_rr_prefix->rr_prefix));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rr_prefix_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rr_prefix_master_t *p_master = _g_p_tbl_rr_prefix_master;
    tbl_rr_prefix_t *p_db_rr_prefix = NULL;
    uint32 rr_prefix_index = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (rr_prefix_index = 0; rr_prefix_index < GLB_MAX_ECMP_ROUND_ROBIN+1; rr_prefix_index++)
    {
        p_db_rr_prefix = p_master->rr_prefix_array[rr_prefix_index];
        if (NULL == p_db_rr_prefix)
        {
            continue;
        }
        rc |= fn(p_db_rr_prefix, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rr_prefix_master_t*
tbl_rr_prefix_get_master()
{
    return _g_p_tbl_rr_prefix_master;
}

tbl_rr_prefix_master_t*
tbl_rr_prefix_init_rr_prefix()
{
    _g_p_tbl_rr_prefix_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rr_prefix_master_t));
    return _g_p_tbl_rr_prefix_master;
}


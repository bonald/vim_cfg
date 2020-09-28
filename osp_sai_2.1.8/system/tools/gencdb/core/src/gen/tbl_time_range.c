
#include "proto.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "cdb_data_cmp.h"

tbl_time_range_master_t *_g_p_tbl_time_range_master = NULL;

static uint32
_tbl_time_range_hash_make(tbl_time_range_t *p_time_range)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_time_range->key;
    for (index = 0; index < sizeof(p_time_range->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_time_range_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_time_range_t *p_time_range1 = (tbl_time_range_t*)p_arg1;
    tbl_time_range_t *p_time_range2 = (tbl_time_range_t*)p_arg2;

    if (0 == sal_memcmp(&p_time_range1->key, &p_time_range2->key, sizeof(tbl_time_range_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_time_range_add_time_range_sync(tbl_time_range_t *p_time_range, uint32 sync)
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;
    tbl_time_range_t *p_db_time_range = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_time_range_get_time_range(&p_time_range->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_time_range = XCALLOC(MEM_TBL_TIME_RANGE, sizeof(tbl_time_range_t));
    if (NULL == p_db_time_range)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_time_range, p_time_range, sizeof(tbl_time_range_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->time_range_hash, (void*)p_db_time_range, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->time_range_list, p_db_time_range);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TIME_RANGE, p_db_time_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TIME_RANGE, p_db_time_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_time_range_del_time_range_sync(tbl_time_range_key_t *p_time_range_key, uint32 sync)
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;
    tbl_time_range_t *p_db_time_range = NULL;

    /* 1. lookup entry exist */
    p_db_time_range = tbl_time_range_get_time_range(p_time_range_key);
    if (NULL == p_db_time_range)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TIME_RANGE, p_db_time_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TIME_RANGE, p_db_time_range);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->time_range_hash, (void*)p_db_time_range);
    ctclib_slistnode_delete(p_master->time_range_list, p_db_time_range);

    /* 4. free db entry */
    XFREE(MEM_TBL_TIME_RANGE, p_db_time_range);

    return PM_E_NONE;
}

int32
tbl_time_range_set_time_range_field_sync(tbl_time_range_t *p_time_range, tbl_time_range_field_id_t field_id, uint32 sync)
{
    tbl_time_range_t *p_db_time_range = NULL;

    /* 1. lookup entry exist */
    p_db_time_range = tbl_time_range_get_time_range(&p_time_range->key);
    if (NULL == p_db_time_range)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TIME_RANGE_FLD_TIME:
        sal_memcpy(&p_db_time_range->time, &p_time_range->time, sizeof(p_time_range->time));
        break;

    case TBL_TIME_RANGE_FLD_REF_CNT:
        p_db_time_range->ref_cnt = p_time_range->ref_cnt;
        break;

    case TBL_TIME_RANGE_FLD_RUNNING_TIMER:
        sal_memcpy(&p_db_time_range->running_timer, &p_time_range->running_timer, sizeof(p_time_range->running_timer));
        break;

    case TBL_TIME_RANGE_FLD_MAX:
        sal_memcpy(p_db_time_range, p_time_range, sizeof(tbl_time_range_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TIME_RANGE, field_id, p_db_time_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TIME_RANGE, field_id, p_db_time_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_time_range_add_time_range(tbl_time_range_t *p_time_range)
{
    return tbl_time_range_add_time_range_sync(p_time_range, TRUE);
}

int32
tbl_time_range_del_time_range(tbl_time_range_key_t *p_time_range_key)
{
    return tbl_time_range_del_time_range_sync(p_time_range_key, TRUE);
}

int32
tbl_time_range_set_time_range_field(tbl_time_range_t *p_time_range, tbl_time_range_field_id_t field_id)
{
    return tbl_time_range_set_time_range_field_sync(p_time_range, field_id, TRUE);
}

tbl_time_range_t*
tbl_time_range_get_time_range(tbl_time_range_key_t *p_time_range_key)
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;
    tbl_time_range_t lkp;

    sal_memcpy(&lkp.key, p_time_range_key, sizeof(tbl_time_range_key_t));
    return ctclib_hash_lookup(p_master->time_range_hash, &lkp);
}

char*
tbl_time_range_key_val2str(tbl_time_range_t *p_time_range, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_time_range->key.name);
    return str;
}

int32
tbl_time_range_key_str2val(char *str, tbl_time_range_t *p_time_range)
{
    int32 ret = 0;

    sal_strcpy(p_time_range->key.name, str);
    return ret;
}

char*
tbl_time_range_key_name_dump(tbl_time_range_t *p_time_range, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TIME_RANGE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_time_range->key.name);
    return str;
}

char*
tbl_time_range_key_value_dump(tbl_time_range_t *p_time_range, char *str)
{
    sal_sprintf(str, "%s", p_time_range->key.name);
    return str;
}

char*
tbl_time_range_field_name_dump(tbl_time_range_t *p_time_range, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TIME_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TIME_RANGE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_time_range_field_value_dump(tbl_time_range_t *p_time_range, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_time_range->key.name);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_TIME, field_id))
    {
        cdb_time_range_value_val2str(str, MAX_CMD_STR_LEN, &p_time_range->time);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_time_range->ref_cnt);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_RUNNING_TIMER, field_id))
    {
        cdb_time_range_timer_val2str(str, MAX_CMD_STR_LEN, &p_time_range->running_timer);
    }
    return str;
}

char**
tbl_time_range_table_dump(tbl_time_range_t *p_time_range, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_time_range_key_name_dump(p_time_range, buf));
    for(i=1; i<TBL_TIME_RANGE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_time_range_field_name_dump(p_time_range, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_time_range_field_value_dump(p_time_range, i, buf));
    }
    return str;
}

int32
tbl_time_range_field_value_parser(char *str, int32 field_id, tbl_time_range_t *p_time_range)
{
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_KEY, field_id))
    {
        sal_strcpy(p_time_range->key.name, str);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_TIME, field_id))
    {
        cdb_time_range_value_str2val(str, &p_time_range->time);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_time_range->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_RUNNING_TIMER, field_id))
    {
        cdb_time_range_timer_str2val(str, &p_time_range->running_timer);
    }
    return PM_E_NONE;
}

int32
tbl_time_range_table_parser(char** array, char* key_value,tbl_time_range_t *p_time_range)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TIME_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_time_range_key_str2val(key_value, p_time_range));

    for(i=1; i<TBL_TIME_RANGE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_time_range_field_value_parser( array[j++], i, p_time_range));
    }

    return PM_E_NONE;
}

int32
tbl_time_range_dump_one(tbl_time_range_t *p_time_range, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TIME_RANGE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TIME_RANGE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TIME_RANGE_FLD_KEY].name,
            p_time_range->key.name);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TIME_RANGE_FLD_TIME].name, 
            cdb_time_range_value_val2str(buf, MAX_CMD_STR_LEN, &p_time_range->time));
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TIME_RANGE_FLD_REF_CNT].name,
            p_time_range->ref_cnt);
    }
    if (FLD_MATCH(TBL_TIME_RANGE_FLD_RUNNING_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TIME_RANGE_FLD_RUNNING_TIMER].name, 
            cdb_time_range_timer_val2str(buf, MAX_CMD_STR_LEN, &p_time_range->running_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_time_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_time_range_t *p_db_time_range = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->time_range_list, p_db_time_range, listnode, next)
    {
        rc |= fn(p_db_time_range, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_time_range_master_t*
tbl_time_range_get_master()
{
    return _g_p_tbl_time_range_master;
}

tbl_time_range_master_t*
tbl_time_range_init_time_range()
{
    _g_p_tbl_time_range_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_time_range_master_t));
    _g_p_tbl_time_range_master->time_range_hash = ctclib_hash_create(_tbl_time_range_hash_make, _tbl_time_range_hash_cmp);
    _g_p_tbl_time_range_master->time_range_list = ctclib_slist_create(tbl_time_range_cmp, NULL);
    return _g_p_tbl_time_range_master;
}

tbl_time_range_t*
tbl_time_range_get_time_range_by_name(char *name)
{
    tbl_time_range_key_t key;
    
    sal_memset(&key, 0, sizeof(key));
    sal_strcpy(key.name, name);
    return tbl_time_range_get_time_range(&key);
}


int32
tbl_time_range_get_time_range_count()
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;

    return p_master->time_range_list->count;
}

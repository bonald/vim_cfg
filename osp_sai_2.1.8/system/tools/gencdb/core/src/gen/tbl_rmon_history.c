
#include "proto.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "cdb_data_cmp.h"

tbl_rmon_history_master_t *_g_p_tbl_rmon_history_master = NULL;

static uint32
_tbl_rmon_history_hash_make(tbl_rmon_history_t *p_rmon_history)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmon_history->key;
    for (index = 0; index < sizeof(p_rmon_history->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rmon_history_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rmon_history_t *p_rmon_history1 = (tbl_rmon_history_t*)p_arg1;
    tbl_rmon_history_t *p_rmon_history2 = (tbl_rmon_history_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmon_history1->key, &p_rmon_history2->key, sizeof(tbl_rmon_history_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_history_add_rmon_history_sync(tbl_rmon_history_t *p_rmon_history, uint32 sync)
{
    tbl_rmon_history_master_t *p_master = _g_p_tbl_rmon_history_master;
    tbl_rmon_history_t *p_db_rmon_history = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rmon_history_get_rmon_history(&p_rmon_history->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmon_history = XCALLOC(MEM_TBL_RMON_HISTORY, sizeof(tbl_rmon_history_t));
    if (NULL == p_db_rmon_history)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmon_history, p_rmon_history, sizeof(tbl_rmon_history_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rmon_history_hash, (void*)p_db_rmon_history, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rmon_history_list, p_db_rmon_history);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RMON_HISTORY, p_db_rmon_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RMON_HISTORY, p_db_rmon_history);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_history_del_rmon_history_sync(tbl_rmon_history_key_t *p_rmon_history_key, uint32 sync)
{
    tbl_rmon_history_master_t *p_master = _g_p_tbl_rmon_history_master;
    tbl_rmon_history_t *p_db_rmon_history = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_history = tbl_rmon_history_get_rmon_history(p_rmon_history_key);
    if (NULL == p_db_rmon_history)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RMON_HISTORY, p_db_rmon_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RMON_HISTORY, p_db_rmon_history);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rmon_history_hash, (void*)p_db_rmon_history);
    ctclib_slistnode_delete(p_master->rmon_history_list, p_db_rmon_history);

    /* 4. free db entry */
    XFREE(MEM_TBL_RMON_HISTORY, p_db_rmon_history);

    return PM_E_NONE;
}

int32
tbl_rmon_history_set_rmon_history_field_sync(tbl_rmon_history_t *p_rmon_history, tbl_rmon_history_field_id_t field_id, uint32 sync)
{
    tbl_rmon_history_t *p_db_rmon_history = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_history = tbl_rmon_history_get_rmon_history(&p_rmon_history->key);
    if (NULL == p_db_rmon_history)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RMON_HISTORY_FLD_STATUS:
        p_db_rmon_history->status = p_rmon_history->status;
        break;

    case TBL_RMON_HISTORY_FLD_OWNER:
        sal_memcpy(p_db_rmon_history->owner, p_rmon_history->owner, sizeof(p_rmon_history->owner));
        break;

    case TBL_RMON_HISTORY_FLD_DATA:
        sal_memcpy(p_db_rmon_history->data, p_rmon_history->data, sizeof(p_rmon_history->data));
        break;

    case TBL_RMON_HISTORY_FLD_DATA_SOURCE:
        p_db_rmon_history->data_source = p_rmon_history->data_source;
        break;

    case TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED:
        p_db_rmon_history->buckets_requested = p_rmon_history->buckets_requested;
        break;

    case TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED:
        p_db_rmon_history->buckets_granted = p_rmon_history->buckets_granted;
        break;

    case TBL_RMON_HISTORY_FLD_INTERVAL:
        p_db_rmon_history->interval = p_rmon_history->interval;
        break;

    case TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO:
        p_db_rmon_history->current_sample_no = p_rmon_history->current_sample_no;
        break;

    case TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER:
        p_db_rmon_history->t_under_creation_timer = p_rmon_history->t_under_creation_timer;
        break;

    case TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER:
        p_db_rmon_history->t_coll_history_timer = p_rmon_history->t_coll_history_timer;
        break;

    case TBL_RMON_HISTORY_FLD_MAX:
        sal_memcpy(p_db_rmon_history, p_rmon_history, sizeof(tbl_rmon_history_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RMON_HISTORY, field_id, p_db_rmon_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RMON_HISTORY, field_id, p_db_rmon_history);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_history_add_rmon_history(tbl_rmon_history_t *p_rmon_history)
{
    return tbl_rmon_history_add_rmon_history_sync(p_rmon_history, TRUE);
}

int32
tbl_rmon_history_del_rmon_history(tbl_rmon_history_key_t *p_rmon_history_key)
{
    return tbl_rmon_history_del_rmon_history_sync(p_rmon_history_key, TRUE);
}

int32
tbl_rmon_history_set_rmon_history_field(tbl_rmon_history_t *p_rmon_history, tbl_rmon_history_field_id_t field_id)
{
    return tbl_rmon_history_set_rmon_history_field_sync(p_rmon_history, field_id, TRUE);
}

tbl_rmon_history_t*
tbl_rmon_history_get_rmon_history(tbl_rmon_history_key_t *p_rmon_history_key)
{
    tbl_rmon_history_master_t *p_master = _g_p_tbl_rmon_history_master;
    tbl_rmon_history_t lkp;

    sal_memcpy(&lkp.key, p_rmon_history_key, sizeof(tbl_rmon_history_key_t));
    return ctclib_hash_lookup(p_master->rmon_history_hash, &lkp);
}

char*
tbl_rmon_history_key_val2str(tbl_rmon_history_t *p_rmon_history, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rmon_history->key.index);
    return str;
}

int32
tbl_rmon_history_key_str2val(char *str, tbl_rmon_history_t *p_rmon_history)
{
    int32 ret = 0;

    p_rmon_history->key.index = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rmon_history_key_name_dump(tbl_rmon_history_t *p_rmon_history, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_HISTORY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rmon_history->key.index);
    return str;
}

char*
tbl_rmon_history_key_value_dump(tbl_rmon_history_t *p_rmon_history, char *str)
{
    sal_sprintf(str, "%u", p_rmon_history->key.index);
    return str;
}

char*
tbl_rmon_history_field_name_dump(tbl_rmon_history_t *p_rmon_history, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RMON_HISTORY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rmon_history_field_value_dump(tbl_rmon_history_t *p_rmon_history, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->key.index);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->status);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_OWNER, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_history->owner);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_rmon_history->data, sizeof(p_rmon_history->data)/4);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA_SOURCE, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->data_source);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->buckets_requested);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->buckets_granted);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->interval);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_history->current_sample_no);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_history->t_under_creation_timer);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_history->t_coll_history_timer);
    }
    return str;
}

char**
tbl_rmon_history_table_dump(tbl_rmon_history_t *p_rmon_history, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rmon_history_key_name_dump(p_rmon_history, buf));
    for(i=1; i<TBL_RMON_HISTORY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rmon_history_field_name_dump(p_rmon_history, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rmon_history_field_value_dump(p_rmon_history, i, buf));
    }
    return str;
}

int32
tbl_rmon_history_field_value_parser(char *str, int32 field_id, tbl_rmon_history_t *p_rmon_history)
{
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_KEY, field_id))
    {
        int32 ret;
        p_rmon_history->key.index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_STATUS, field_id))
    {
        int32 ret;
        p_rmon_history->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_OWNER, field_id))
    {
        sal_strcpy(p_rmon_history->owner, str);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA, field_id))
    {
        cdb_uint32_array_str2val(str, p_rmon_history->data);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA_SOURCE, field_id))
    {
        int32 ret;
        p_rmon_history->data_source = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED, field_id))
    {
        int32 ret;
        p_rmon_history->buckets_requested = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED, field_id))
    {
        int32 ret;
        p_rmon_history->buckets_granted = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_INTERVAL, field_id))
    {
        int32 ret;
        p_rmon_history->interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO, field_id))
    {
        int32 ret;
        p_rmon_history->current_sample_no = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_history->t_under_creation_timer);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_history->t_coll_history_timer);
    }
    return PM_E_NONE;
}

int32
tbl_rmon_history_table_parser(char** array, char* key_value,tbl_rmon_history_t *p_rmon_history)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rmon_history_key_str2val(key_value, p_rmon_history));

    for(i=1; i<TBL_RMON_HISTORY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rmon_history_field_value_parser( array[j++], i, p_rmon_history));
    }

    return PM_E_NONE;
}

int32
tbl_rmon_history_dump_one(tbl_rmon_history_t *p_rmon_history, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_HISTORY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_KEY].name,
            p_rmon_history->key.index);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_STATUS].name,
            p_rmon_history->status);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_HISTORY_FLD_OWNER].name,
            p_rmon_history->owner);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_HISTORY_FLD_DATA].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_rmon_history->data, sizeof(p_rmon_history->data)/4));
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_DATA_SOURCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_DATA_SOURCE].name,
            p_rmon_history->data_source);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED].name,
            p_rmon_history->buckets_requested);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED].name,
            p_rmon_history->buckets_granted);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_INTERVAL].name,
            p_rmon_history->interval);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO].name,
            p_rmon_history->current_sample_no);
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_history->t_under_creation_timer));
    }
    if (FLD_MATCH(TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_history->t_coll_history_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rmon_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rmon_history_master_t *p_master = _g_p_tbl_rmon_history_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_history_t *p_db_rmon_history = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_history_list, p_db_rmon_history, listnode, next)
    {
        rc |= fn(p_db_rmon_history, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rmon_history_master_t*
tbl_rmon_history_get_master()
{
    return _g_p_tbl_rmon_history_master;
}

tbl_rmon_history_master_t*
tbl_rmon_history_init_rmon_history()
{
    _g_p_tbl_rmon_history_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rmon_history_master_t));
    _g_p_tbl_rmon_history_master->rmon_history_hash = ctclib_hash_create(_tbl_rmon_history_hash_make, _tbl_rmon_history_hash_cmp);
    _g_p_tbl_rmon_history_master->rmon_history_list = ctclib_slist_create(tbl_rmon_history_cmp, NULL);
    return _g_p_tbl_rmon_history_master;
}


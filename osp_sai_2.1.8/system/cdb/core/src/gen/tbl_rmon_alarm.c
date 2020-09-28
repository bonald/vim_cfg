
#include "proto.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "cdb_data_cmp.h"

tbl_rmon_alarm_master_t *_g_p_tbl_rmon_alarm_master = NULL;

static uint32
_tbl_rmon_alarm_hash_make(tbl_rmon_alarm_t *p_rmon_alarm)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmon_alarm->key;
    for (index = 0; index < sizeof(p_rmon_alarm->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rmon_alarm_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rmon_alarm_t *p_rmon_alarm1 = (tbl_rmon_alarm_t*)p_arg1;
    tbl_rmon_alarm_t *p_rmon_alarm2 = (tbl_rmon_alarm_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmon_alarm1->key, &p_rmon_alarm2->key, sizeof(tbl_rmon_alarm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_alarm_add_rmon_alarm_sync(tbl_rmon_alarm_t *p_rmon_alarm, uint32 sync)
{
    tbl_rmon_alarm_master_t *p_master = _g_p_tbl_rmon_alarm_master;
    tbl_rmon_alarm_t *p_db_rmon_alarm = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rmon_alarm_get_rmon_alarm(&p_rmon_alarm->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmon_alarm = XCALLOC(MEM_TBL_RMON_ALARM, sizeof(tbl_rmon_alarm_t));
    if (NULL == p_db_rmon_alarm)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmon_alarm, p_rmon_alarm, sizeof(tbl_rmon_alarm_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rmon_alarm_hash, (void*)p_db_rmon_alarm, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rmon_alarm_list, p_db_rmon_alarm);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RMON_ALARM, p_db_rmon_alarm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RMON_ALARM, p_db_rmon_alarm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_alarm_del_rmon_alarm_sync(tbl_rmon_alarm_key_t *p_rmon_alarm_key, uint32 sync)
{
    tbl_rmon_alarm_master_t *p_master = _g_p_tbl_rmon_alarm_master;
    tbl_rmon_alarm_t *p_db_rmon_alarm = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_alarm = tbl_rmon_alarm_get_rmon_alarm(p_rmon_alarm_key);
    if (NULL == p_db_rmon_alarm)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RMON_ALARM, p_db_rmon_alarm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RMON_ALARM, p_db_rmon_alarm);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rmon_alarm_hash, (void*)p_db_rmon_alarm);
    ctclib_slistnode_delete(p_master->rmon_alarm_list, p_db_rmon_alarm);

    /* 4. free db entry */
    XFREE(MEM_TBL_RMON_ALARM, p_db_rmon_alarm);

    return PM_E_NONE;
}

int32
tbl_rmon_alarm_set_rmon_alarm_field_sync(tbl_rmon_alarm_t *p_rmon_alarm, tbl_rmon_alarm_field_id_t field_id, uint32 sync)
{
    tbl_rmon_alarm_t *p_db_rmon_alarm = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_alarm = tbl_rmon_alarm_get_rmon_alarm(&p_rmon_alarm->key);
    if (NULL == p_db_rmon_alarm)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RMON_ALARM_FLD_CONF:
        p_db_rmon_alarm->conf = p_rmon_alarm->conf;
        break;

    case TBL_RMON_ALARM_FLD_INTERVAL:
        p_db_rmon_alarm->interval = p_rmon_alarm->interval;
        break;

    case TBL_RMON_ALARM_FLD_VARIABLE:
        sal_memcpy(p_db_rmon_alarm->variable, p_rmon_alarm->variable, sizeof(p_rmon_alarm->variable));
        break;

    case TBL_RMON_ALARM_FLD_VARIABLE_WORD:
        sal_memcpy(p_db_rmon_alarm->variable_word, p_rmon_alarm->variable_word, sizeof(p_rmon_alarm->variable_word));
        break;

    case TBL_RMON_ALARM_FLD_SAMPLE_TYPE:
        p_db_rmon_alarm->sample_type = p_rmon_alarm->sample_type;
        break;

    case TBL_RMON_ALARM_FLD_VALUE:
        p_db_rmon_alarm->value = p_rmon_alarm->value;
        break;

    case TBL_RMON_ALARM_FLD_STARTUP_ALARM:
        p_db_rmon_alarm->startup_alarm = p_rmon_alarm->startup_alarm;
        break;

    case TBL_RMON_ALARM_FLD_RISING_THRESHOLD:
        p_db_rmon_alarm->rising_threshold = p_rmon_alarm->rising_threshold;
        break;

    case TBL_RMON_ALARM_FLD_FALLING_THRESHOLD:
        p_db_rmon_alarm->falling_threshold = p_rmon_alarm->falling_threshold;
        break;

    case TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX:
        p_db_rmon_alarm->rising_event_index = p_rmon_alarm->rising_event_index;
        break;

    case TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX:
        p_db_rmon_alarm->falling_event_index = p_rmon_alarm->falling_event_index;
        break;

    case TBL_RMON_ALARM_FLD_OWNER:
        sal_memcpy(p_db_rmon_alarm->owner, p_rmon_alarm->owner, sizeof(p_rmon_alarm->owner));
        break;

    case TBL_RMON_ALARM_FLD_STATUS:
        p_db_rmon_alarm->status = p_rmon_alarm->status;
        break;

    case TBL_RMON_ALARM_FLD_LAST_EVENT:
        p_db_rmon_alarm->last_event = p_rmon_alarm->last_event;
        break;

    case TBL_RMON_ALARM_FLD_PREV_VALUE:
        p_db_rmon_alarm->prev_value = p_rmon_alarm->prev_value;
        break;

    case TBL_RMON_ALARM_FLD_CURR_VALUE:
        p_db_rmon_alarm->curr_value = p_rmon_alarm->curr_value;
        break;

    case TBL_RMON_ALARM_FLD_T_ALARM_TIMER:
        p_db_rmon_alarm->t_alarm_timer = p_rmon_alarm->t_alarm_timer;
        break;

    case TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER:
        p_db_rmon_alarm->t_under_creation_timer = p_rmon_alarm->t_under_creation_timer;
        break;

    case TBL_RMON_ALARM_FLD_MAX:
        sal_memcpy(p_db_rmon_alarm, p_rmon_alarm, sizeof(tbl_rmon_alarm_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RMON_ALARM, field_id, p_db_rmon_alarm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RMON_ALARM, field_id, p_db_rmon_alarm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_alarm_add_rmon_alarm(tbl_rmon_alarm_t *p_rmon_alarm)
{
    return tbl_rmon_alarm_add_rmon_alarm_sync(p_rmon_alarm, TRUE);
}

int32
tbl_rmon_alarm_del_rmon_alarm(tbl_rmon_alarm_key_t *p_rmon_alarm_key)
{
    return tbl_rmon_alarm_del_rmon_alarm_sync(p_rmon_alarm_key, TRUE);
}

int32
tbl_rmon_alarm_set_rmon_alarm_field(tbl_rmon_alarm_t *p_rmon_alarm, tbl_rmon_alarm_field_id_t field_id)
{
    return tbl_rmon_alarm_set_rmon_alarm_field_sync(p_rmon_alarm, field_id, TRUE);
}

tbl_rmon_alarm_t*
tbl_rmon_alarm_get_rmon_alarm(tbl_rmon_alarm_key_t *p_rmon_alarm_key)
{
    tbl_rmon_alarm_master_t *p_master = _g_p_tbl_rmon_alarm_master;
    tbl_rmon_alarm_t lkp;

    sal_memcpy(&lkp.key, p_rmon_alarm_key, sizeof(tbl_rmon_alarm_key_t));
    return ctclib_hash_lookup(p_master->rmon_alarm_hash, &lkp);
}

char*
tbl_rmon_alarm_key_val2str(tbl_rmon_alarm_t *p_rmon_alarm, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rmon_alarm->key.index);
    return str;
}

int32
tbl_rmon_alarm_key_str2val(char *str, tbl_rmon_alarm_t *p_rmon_alarm)
{
    int32 ret = 0;

    p_rmon_alarm->key.index = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rmon_alarm_key_name_dump(tbl_rmon_alarm_t *p_rmon_alarm, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ALARM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rmon_alarm->key.index);
    return str;
}

char*
tbl_rmon_alarm_key_value_dump(tbl_rmon_alarm_t *p_rmon_alarm, char *str)
{
    sal_sprintf(str, "%u", p_rmon_alarm->key.index);
    return str;
}

char*
tbl_rmon_alarm_field_name_dump(tbl_rmon_alarm_t *p_rmon_alarm, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ALARM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RMON_ALARM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rmon_alarm_field_value_dump(tbl_rmon_alarm_t *p_rmon_alarm, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->key.index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CONF, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->conf);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->interval);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_rmon_alarm->variable, sizeof(p_rmon_alarm->variable)/4);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE_WORD, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_alarm->variable_word);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_SAMPLE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->sample_type);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VALUE, field_id))
    {
        sal_sprintf(str, "%"PRId64, p_rmon_alarm->value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STARTUP_ALARM, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->startup_alarm);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%d", p_rmon_alarm->rising_threshold);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%d", p_rmon_alarm->falling_threshold);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->rising_event_index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->falling_event_index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_OWNER, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_alarm->owner);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->status);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_LAST_EVENT, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_alarm->last_event);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_PREV_VALUE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_alarm->prev_value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CURR_VALUE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_alarm->curr_value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_ALARM_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_alarm->t_alarm_timer);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_alarm->t_under_creation_timer);
    }
    return str;
}

char**
tbl_rmon_alarm_table_dump(tbl_rmon_alarm_t *p_rmon_alarm, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rmon_alarm_key_name_dump(p_rmon_alarm, buf));
    for(i=1; i<TBL_RMON_ALARM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rmon_alarm_field_name_dump(p_rmon_alarm, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rmon_alarm_field_value_dump(p_rmon_alarm, i, buf));
    }
    return str;
}

int32
tbl_rmon_alarm_field_value_parser(char *str, int32 field_id, tbl_rmon_alarm_t *p_rmon_alarm)
{
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_KEY, field_id))
    {
        int32 ret;
        p_rmon_alarm->key.index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CONF, field_id))
    {
        int32 ret;
        p_rmon_alarm->conf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_INTERVAL, field_id))
    {
        int32 ret;
        p_rmon_alarm->interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE, field_id))
    {
        cdb_uint32_array_str2val(str, p_rmon_alarm->variable);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE_WORD, field_id))
    {
        sal_strcpy(p_rmon_alarm->variable_word, str);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_SAMPLE_TYPE, field_id))
    {
        int32 ret;
        p_rmon_alarm->sample_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VALUE, field_id))
    {
        int32 ret;
        p_rmon_alarm->value = cdb_int64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STARTUP_ALARM, field_id))
    {
        int32 ret;
        p_rmon_alarm->startup_alarm = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_THRESHOLD, field_id))
    {
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_THRESHOLD, field_id))
    {
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX, field_id))
    {
        int32 ret;
        p_rmon_alarm->rising_event_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX, field_id))
    {
        int32 ret;
        p_rmon_alarm->falling_event_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_OWNER, field_id))
    {
        sal_strcpy(p_rmon_alarm->owner, str);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STATUS, field_id))
    {
        int32 ret;
        p_rmon_alarm->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_LAST_EVENT, field_id))
    {
        int32 ret;
        p_rmon_alarm->last_event = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_PREV_VALUE, field_id))
    {
        int32 ret;
        p_rmon_alarm->prev_value = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CURR_VALUE, field_id))
    {
        int32 ret;
        p_rmon_alarm->curr_value = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_ALARM_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_alarm->t_alarm_timer);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_alarm->t_under_creation_timer);
    }
    return PM_E_NONE;
}

int32
tbl_rmon_alarm_table_parser(char** array, char* key_value,tbl_rmon_alarm_t *p_rmon_alarm)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ALARM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rmon_alarm_key_str2val(key_value, p_rmon_alarm));

    for(i=1; i<TBL_RMON_ALARM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rmon_alarm_field_value_parser( array[j++], i, p_rmon_alarm));
    }

    return PM_E_NONE;
}

int32
tbl_rmon_alarm_dump_one(tbl_rmon_alarm_t *p_rmon_alarm, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ALARM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RMON_ALARM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_KEY].name,
            p_rmon_alarm->key.index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CONF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_CONF].name,
            p_rmon_alarm->conf);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_INTERVAL].name,
            p_rmon_alarm->interval);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ALARM_FLD_VARIABLE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_rmon_alarm->variable, sizeof(p_rmon_alarm->variable)/4));
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VARIABLE_WORD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ALARM_FLD_VARIABLE_WORD].name,
            p_rmon_alarm->variable_word);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_SAMPLE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_SAMPLE_TYPE].name,
            p_rmon_alarm->sample_type);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRId64, p_tbl_info->field[TBL_RMON_ALARM_FLD_VALUE].name,
            p_rmon_alarm->value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STARTUP_ALARM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_STARTUP_ALARM].name,
            p_rmon_alarm->startup_alarm);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_RMON_ALARM_FLD_RISING_THRESHOLD].name,
            p_rmon_alarm->rising_threshold);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_RMON_ALARM_FLD_FALLING_THRESHOLD].name,
            p_rmon_alarm->falling_threshold);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX].name,
            p_rmon_alarm->rising_event_index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX].name,
            p_rmon_alarm->falling_event_index);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ALARM_FLD_OWNER].name,
            p_rmon_alarm->owner);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_STATUS].name,
            p_rmon_alarm->status);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_LAST_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ALARM_FLD_LAST_EVENT].name,
            p_rmon_alarm->last_event);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_PREV_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ALARM_FLD_PREV_VALUE].name,
            p_rmon_alarm->prev_value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_CURR_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ALARM_FLD_CURR_VALUE].name,
            p_rmon_alarm->curr_value);
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_ALARM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ALARM_FLD_T_ALARM_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_alarm->t_alarm_timer));
    }
    if (FLD_MATCH(TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_alarm->t_under_creation_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rmon_alarm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rmon_alarm_master_t *p_master = _g_p_tbl_rmon_alarm_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_alarm_t *p_db_rmon_alarm = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_alarm_list, p_db_rmon_alarm, listnode, next)
    {
        rc |= fn(p_db_rmon_alarm, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rmon_alarm_master_t*
tbl_rmon_alarm_get_master()
{
    return _g_p_tbl_rmon_alarm_master;
}

tbl_rmon_alarm_master_t*
tbl_rmon_alarm_init_rmon_alarm()
{
    _g_p_tbl_rmon_alarm_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rmon_alarm_master_t));
    _g_p_tbl_rmon_alarm_master->rmon_alarm_hash = ctclib_hash_create(_tbl_rmon_alarm_hash_make, _tbl_rmon_alarm_hash_cmp);
    _g_p_tbl_rmon_alarm_master->rmon_alarm_list = ctclib_slist_create(tbl_rmon_alarm_cmp, NULL);
    return _g_p_tbl_rmon_alarm_master;
}


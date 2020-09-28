
#include "proto.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "cdb_data_cmp.h"

tbl_rmon_event_master_t *_g_p_tbl_rmon_event_master = NULL;

static uint32
_tbl_rmon_event_hash_make(tbl_rmon_event_t *p_rmon_event)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmon_event->key;
    for (index = 0; index < sizeof(p_rmon_event->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rmon_event_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rmon_event_t *p_rmon_event1 = (tbl_rmon_event_t*)p_arg1;
    tbl_rmon_event_t *p_rmon_event2 = (tbl_rmon_event_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmon_event1->key, &p_rmon_event2->key, sizeof(tbl_rmon_event_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_event_add_rmon_event_sync(tbl_rmon_event_t *p_rmon_event, uint32 sync)
{
    tbl_rmon_event_master_t *p_master = _g_p_tbl_rmon_event_master;
    tbl_rmon_event_t *p_db_rmon_event = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rmon_event_get_rmon_event(&p_rmon_event->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmon_event = XCALLOC(MEM_TBL_RMON_EVENT, sizeof(tbl_rmon_event_t));
    if (NULL == p_db_rmon_event)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmon_event, p_rmon_event, sizeof(tbl_rmon_event_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rmon_event_hash, (void*)p_db_rmon_event, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rmon_event_list, p_db_rmon_event);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RMON_EVENT, p_db_rmon_event);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RMON_EVENT, p_db_rmon_event);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_event_del_rmon_event_sync(tbl_rmon_event_key_t *p_rmon_event_key, uint32 sync)
{
    tbl_rmon_event_master_t *p_master = _g_p_tbl_rmon_event_master;
    tbl_rmon_event_t *p_db_rmon_event = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_event = tbl_rmon_event_get_rmon_event(p_rmon_event_key);
    if (NULL == p_db_rmon_event)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RMON_EVENT, p_db_rmon_event);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RMON_EVENT, p_db_rmon_event);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rmon_event_hash, (void*)p_db_rmon_event);
    ctclib_slistnode_delete(p_master->rmon_event_list, p_db_rmon_event);

    /* 4. free db entry */
    XFREE(MEM_TBL_RMON_EVENT, p_db_rmon_event);

    return PM_E_NONE;
}

int32
tbl_rmon_event_set_rmon_event_field_sync(tbl_rmon_event_t *p_rmon_event, tbl_rmon_event_field_id_t field_id, uint32 sync)
{
    tbl_rmon_event_t *p_db_rmon_event = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_event = tbl_rmon_event_get_rmon_event(&p_rmon_event->key);
    if (NULL == p_db_rmon_event)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RMON_EVENT_FLD_CONF:
        p_db_rmon_event->conf = p_rmon_event->conf;
        break;

    case TBL_RMON_EVENT_FLD_TYPE:
        p_db_rmon_event->type = p_rmon_event->type;
        break;

    case TBL_RMON_EVENT_FLD_STATUS:
        p_db_rmon_event->status = p_rmon_event->status;
        break;

    case TBL_RMON_EVENT_FLD_LAST_SENT:
        p_db_rmon_event->last_sent = p_rmon_event->last_sent;
        break;

    case TBL_RMON_EVENT_FLD_DESC:
        sal_memcpy(p_db_rmon_event->desc, p_rmon_event->desc, sizeof(p_rmon_event->desc));
        break;

    case TBL_RMON_EVENT_FLD_COMMUNITY:
        sal_memcpy(p_db_rmon_event->community, p_rmon_event->community, sizeof(p_rmon_event->community));
        break;

    case TBL_RMON_EVENT_FLD_OWNER:
        sal_memcpy(p_db_rmon_event->owner, p_rmon_event->owner, sizeof(p_rmon_event->owner));
        break;

    case TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER:
        p_db_rmon_event->t_under_creation_timer = p_rmon_event->t_under_creation_timer;
        break;

    case TBL_RMON_EVENT_FLD_MAX:
        sal_memcpy(p_db_rmon_event, p_rmon_event, sizeof(tbl_rmon_event_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RMON_EVENT, field_id, p_db_rmon_event);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RMON_EVENT, field_id, p_db_rmon_event);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_event_add_rmon_event(tbl_rmon_event_t *p_rmon_event)
{
    return tbl_rmon_event_add_rmon_event_sync(p_rmon_event, TRUE);
}

int32
tbl_rmon_event_del_rmon_event(tbl_rmon_event_key_t *p_rmon_event_key)
{
    return tbl_rmon_event_del_rmon_event_sync(p_rmon_event_key, TRUE);
}

int32
tbl_rmon_event_set_rmon_event_field(tbl_rmon_event_t *p_rmon_event, tbl_rmon_event_field_id_t field_id)
{
    return tbl_rmon_event_set_rmon_event_field_sync(p_rmon_event, field_id, TRUE);
}

tbl_rmon_event_t*
tbl_rmon_event_get_rmon_event(tbl_rmon_event_key_t *p_rmon_event_key)
{
    tbl_rmon_event_master_t *p_master = _g_p_tbl_rmon_event_master;
    tbl_rmon_event_t lkp;

    sal_memcpy(&lkp.key, p_rmon_event_key, sizeof(tbl_rmon_event_key_t));
    return ctclib_hash_lookup(p_master->rmon_event_hash, &lkp);
}

char*
tbl_rmon_event_key_val2str(tbl_rmon_event_t *p_rmon_event, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rmon_event->key.index);
    return str;
}

int32
tbl_rmon_event_key_str2val(char *str, tbl_rmon_event_t *p_rmon_event)
{
    int32 ret = 0;

    p_rmon_event->key.index = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rmon_event_key_name_dump(tbl_rmon_event_t *p_rmon_event, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_EVENT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rmon_event->key.index);
    return str;
}

char*
tbl_rmon_event_key_value_dump(tbl_rmon_event_t *p_rmon_event, char *str)
{
    sal_sprintf(str, "%u", p_rmon_event->key.index);
    return str;
}

char*
tbl_rmon_event_field_name_dump(tbl_rmon_event_t *p_rmon_event, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_EVENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RMON_EVENT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rmon_event_field_value_dump(tbl_rmon_event_t *p_rmon_event, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_event->key.index);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_CONF, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_event->conf);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_event->type);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_event->status);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_LAST_SENT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_event->last_sent);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_event->desc);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_COMMUNITY, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_event->community);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_OWNER, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_event->owner);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_event->t_under_creation_timer);
    }
    return str;
}

char**
tbl_rmon_event_table_dump(tbl_rmon_event_t *p_rmon_event, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rmon_event_key_name_dump(p_rmon_event, buf));
    for(i=1; i<TBL_RMON_EVENT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rmon_event_field_name_dump(p_rmon_event, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rmon_event_field_value_dump(p_rmon_event, i, buf));
    }
    return str;
}

int32
tbl_rmon_event_field_value_parser(char *str, int32 field_id, tbl_rmon_event_t *p_rmon_event)
{
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_KEY, field_id))
    {
        int32 ret;
        p_rmon_event->key.index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_CONF, field_id))
    {
        int32 ret;
        p_rmon_event->conf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_TYPE, field_id))
    {
        int32 ret;
        p_rmon_event->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_STATUS, field_id))
    {
        int32 ret;
        p_rmon_event->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_LAST_SENT, field_id))
    {
        int32 ret;
        p_rmon_event->last_sent = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_DESC, field_id))
    {
        sal_strcpy(p_rmon_event->desc, str);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_COMMUNITY, field_id))
    {
        sal_strcpy(p_rmon_event->community, str);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_OWNER, field_id))
    {
        sal_strcpy(p_rmon_event->owner, str);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_event->t_under_creation_timer);
    }
    return PM_E_NONE;
}

int32
tbl_rmon_event_table_parser(char** array, char* key_value,tbl_rmon_event_t *p_rmon_event)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_EVENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rmon_event_key_str2val(key_value, p_rmon_event));

    for(i=1; i<TBL_RMON_EVENT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rmon_event_field_value_parser( array[j++], i, p_rmon_event));
    }

    return PM_E_NONE;
}

int32
tbl_rmon_event_dump_one(tbl_rmon_event_t *p_rmon_event, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_EVENT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RMON_EVENT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_EVENT_FLD_KEY].name,
            p_rmon_event->key.index);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_CONF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_EVENT_FLD_CONF].name,
            p_rmon_event->conf);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_EVENT_FLD_TYPE].name,
            p_rmon_event->type);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_EVENT_FLD_STATUS].name,
            p_rmon_event->status);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_LAST_SENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_EVENT_FLD_LAST_SENT].name,
            p_rmon_event->last_sent);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_DESC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_EVENT_FLD_DESC].name,
            p_rmon_event->desc);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_COMMUNITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_EVENT_FLD_COMMUNITY].name,
            p_rmon_event->community);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_EVENT_FLD_OWNER].name,
            p_rmon_event->owner);
    }
    if (FLD_MATCH(TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_EVENT_FLD_T_UNDER_CREATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_event->t_under_creation_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rmon_event_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rmon_event_master_t *p_master = _g_p_tbl_rmon_event_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_event_t *p_db_rmon_event = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_event_list, p_db_rmon_event, listnode, next)
    {
        rc |= fn(p_db_rmon_event, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rmon_event_master_t*
tbl_rmon_event_get_master()
{
    return _g_p_tbl_rmon_event_master;
}

tbl_rmon_event_master_t*
tbl_rmon_event_init_rmon_event()
{
    _g_p_tbl_rmon_event_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rmon_event_master_t));
    _g_p_tbl_rmon_event_master->rmon_event_hash = ctclib_hash_create(_tbl_rmon_event_hash_make, _tbl_rmon_event_hash_cmp);
    _g_p_tbl_rmon_event_master->rmon_event_list = ctclib_slist_create(tbl_rmon_event_cmp, NULL);
    return _g_p_tbl_rmon_event_master;
}



#include "proto.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#include "cdb_data_cmp.h"

tbl_sflow_collector_master_t *_g_p_tbl_sflow_collector_master = NULL;

int32
tbl_sflow_collector_add_sflow_collector_sync(tbl_sflow_collector_t *p_sflow_collector, uint32 sync)
{
    tbl_sflow_collector_master_t *p_master = _g_p_tbl_sflow_collector_master;
    tbl_sflow_collector_t *p_db_sflow_collector = NULL;

    /* 1. lookup entry exist */
    if (tbl_sflow_collector_get_sflow_collector(&p_sflow_collector->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_sflow_collector = XCALLOC(MEM_TBL_SFLOW_COLLECTOR, sizeof(tbl_sflow_collector_t));
    if (NULL == p_db_sflow_collector)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_sflow_collector, p_sflow_collector, sizeof(tbl_sflow_collector_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->sflow_collector_list, p_db_sflow_collector);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SFLOW_COLLECTOR, p_db_sflow_collector);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SFLOW_COLLECTOR, p_db_sflow_collector);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sflow_collector_del_sflow_collector_sync(tbl_sflow_collector_key_t *p_sflow_collector_key, uint32 sync)
{
    tbl_sflow_collector_master_t *p_master = _g_p_tbl_sflow_collector_master;
    tbl_sflow_collector_t *p_db_sflow_collector = NULL;

    /* 1. lookup entry exist */
    p_db_sflow_collector = tbl_sflow_collector_get_sflow_collector(p_sflow_collector_key);
    if (NULL == p_db_sflow_collector)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SFLOW_COLLECTOR, p_db_sflow_collector);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SFLOW_COLLECTOR, p_db_sflow_collector);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->sflow_collector_list, p_db_sflow_collector);

    /* 4. free db entry */
    XFREE(MEM_TBL_SFLOW_COLLECTOR, p_db_sflow_collector);

    return PM_E_NONE;
}

int32
tbl_sflow_collector_set_sflow_collector_field_sync(tbl_sflow_collector_t *p_sflow_collector, tbl_sflow_collector_field_id_t field_id, uint32 sync)
{
    tbl_sflow_collector_t *p_db_sflow_collector = NULL;

    /* 1. lookup entry exist */
    p_db_sflow_collector = tbl_sflow_collector_get_sflow_collector(&p_sflow_collector->key);
    if (NULL == p_db_sflow_collector)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SFLOW_COLLECTOR_FLD_BUF:
        sal_memcpy(p_db_sflow_collector->buf, p_sflow_collector->buf, sizeof(p_sflow_collector->buf));
        break;

    case TBL_SFLOW_COLLECTOR_FLD_PKT_LEN:
        p_db_sflow_collector->pkt_len = p_sflow_collector->pkt_len;
        break;

    case TBL_SFLOW_COLLECTOR_FLD_SENDFD:
        p_db_sflow_collector->sendfd = p_sflow_collector->sendfd;
        break;

    case TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER:
        p_db_sflow_collector->transdelay_timer = p_sflow_collector->transdelay_timer;
        break;

    case TBL_SFLOW_COLLECTOR_FLD_SEQUENCE:
        p_db_sflow_collector->sequence = p_sflow_collector->sequence;
        break;

    case TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER:
        p_db_sflow_collector->t_log_timer = p_sflow_collector->t_log_timer;
        break;

    case TBL_SFLOW_COLLECTOR_FLD_MAX:
        sal_memcpy(p_db_sflow_collector, p_sflow_collector, sizeof(tbl_sflow_collector_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SFLOW_COLLECTOR, field_id, p_db_sflow_collector);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SFLOW_COLLECTOR, field_id, p_db_sflow_collector);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sflow_collector_add_sflow_collector(tbl_sflow_collector_t *p_sflow_collector)
{
    return tbl_sflow_collector_add_sflow_collector_sync(p_sflow_collector, TRUE);
}

int32
tbl_sflow_collector_del_sflow_collector(tbl_sflow_collector_key_t *p_sflow_collector_key)
{
    return tbl_sflow_collector_del_sflow_collector_sync(p_sflow_collector_key, TRUE);
}

int32
tbl_sflow_collector_set_sflow_collector_field(tbl_sflow_collector_t *p_sflow_collector, tbl_sflow_collector_field_id_t field_id)
{
    return tbl_sflow_collector_set_sflow_collector_field_sync(p_sflow_collector, field_id, TRUE);
}

tbl_sflow_collector_t*
tbl_sflow_collector_get_sflow_collector(tbl_sflow_collector_key_t *p_sflow_collector_key)
{
    tbl_sflow_collector_master_t *p_master = _g_p_tbl_sflow_collector_master;
    tbl_sflow_collector_t lkp;

    sal_memcpy(&lkp.key, p_sflow_collector_key, sizeof(tbl_sflow_collector_key_t));
    return ctclib_slistdata_lookup(p_master->sflow_collector_list, &lkp);
}

char*
tbl_sflow_collector_key_val2str(tbl_sflow_collector_t *p_sflow_collector, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_sflow_collector_key_val2str(buf, MAX_CMD_STR_LEN, &p_sflow_collector->key));
    return str;
}

int32
tbl_sflow_collector_key_str2val(char *str, tbl_sflow_collector_t *p_sflow_collector)
{
    int32 ret = 0;

    ret = cdb_sflow_collector_key_str2val(str, &p_sflow_collector->key);
    return ret;
}

char*
tbl_sflow_collector_key_name_dump(tbl_sflow_collector_t *p_sflow_collector, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_sflow_collector_key_val2str(buf, MAX_CMD_STR_LEN, &p_sflow_collector->key));
    return str;
}

char*
tbl_sflow_collector_key_value_dump(tbl_sflow_collector_t *p_sflow_collector, char *str)
{

    cdb_sflow_collector_key_val2str(str, MAX_CMD_STR_LEN, &p_sflow_collector->key);
    return str;
}

char*
tbl_sflow_collector_field_name_dump(tbl_sflow_collector_t *p_sflow_collector, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SFLOW_COLLECTOR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sflow_collector_field_value_dump(tbl_sflow_collector_t *p_sflow_collector, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_KEY, field_id))
    {
        cdb_sflow_collector_key_val2str(str, MAX_CMD_STR_LEN, &p_sflow_collector->key);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_sflow_collector->buf, sizeof(p_sflow_collector->buf));
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_PKT_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_collector->pkt_len);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SENDFD, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_collector->sendfd);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_sflow_collector->transdelay_timer);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SEQUENCE, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_collector->sequence);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_sflow_collector->t_log_timer);
    }
    return str;
}

char**
tbl_sflow_collector_table_dump(tbl_sflow_collector_t *p_sflow_collector, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sflow_collector_key_name_dump(p_sflow_collector, buf));
    for(i=1; i<TBL_SFLOW_COLLECTOR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sflow_collector_field_name_dump(p_sflow_collector, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sflow_collector_field_value_dump(p_sflow_collector, i, buf));
    }
    return str;
}

int32
tbl_sflow_collector_field_value_parser(char *str, int32 field_id, tbl_sflow_collector_t *p_sflow_collector)
{
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_KEY, field_id))
    {
        cdb_sflow_collector_key_str2val(str, &p_sflow_collector->key);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_sflow_collector->buf);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_PKT_LEN, field_id))
    {
        int32 ret;
        p_sflow_collector->pkt_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SENDFD, field_id))
    {
        int32 ret;
        p_sflow_collector->sendfd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_sflow_collector->transdelay_timer);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SEQUENCE, field_id))
    {
        int32 ret;
        p_sflow_collector->sequence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER, field_id))
    {
        ctc_task_str2val(str, p_sflow_collector->t_log_timer);
    }
    return PM_E_NONE;
}

int32
tbl_sflow_collector_table_parser(char** array, char* key_value,tbl_sflow_collector_t *p_sflow_collector)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sflow_collector_key_str2val(key_value, p_sflow_collector));

    for(i=1; i<TBL_SFLOW_COLLECTOR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sflow_collector_field_value_parser( array[j++], i, p_sflow_collector));
    }

    return PM_E_NONE;
}

int32
tbl_sflow_collector_dump_one(tbl_sflow_collector_t *p_sflow_collector, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_KEY].name, 
            cdb_sflow_collector_key_val2str(buf, MAX_CMD_STR_LEN, &p_sflow_collector->key));
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_sflow_collector->buf, sizeof(p_sflow_collector->buf)));
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_PKT_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_PKT_LEN].name,
            p_sflow_collector->pkt_len);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SENDFD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_SENDFD].name,
            p_sflow_collector->sendfd);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_sflow_collector->transdelay_timer));
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_SEQUENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_SEQUENCE].name,
            p_sflow_collector->sequence);
    }
    if (FLD_MATCH(TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_sflow_collector->t_log_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sflow_collector_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sflow_collector_master_t *p_master = _g_p_tbl_sflow_collector_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_sflow_collector_t *p_db_sflow_collector = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->sflow_collector_list, p_db_sflow_collector, listnode, next)
    {
        rc |= fn(p_db_sflow_collector, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sflow_collector_master_t*
tbl_sflow_collector_get_master()
{
    return _g_p_tbl_sflow_collector_master;
}

tbl_sflow_collector_master_t*
tbl_sflow_collector_init_sflow_collector()
{
    _g_p_tbl_sflow_collector_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_sflow_collector_master_t));
    _g_p_tbl_sflow_collector_master->sflow_collector_list = ctclib_slist_create(tbl_sflow_collector_cmp, NULL);
    return _g_p_tbl_sflow_collector_master;
}


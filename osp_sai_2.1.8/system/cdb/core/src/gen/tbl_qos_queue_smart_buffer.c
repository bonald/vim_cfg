
#include "proto.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"
#include "cdb_data_cmp.h"

tbl_qos_queue_smart_buffer_master_t *_g_p_tbl_qos_queue_smart_buffer_master = NULL;

int32
tbl_qos_queue_smart_buffer_add_qos_queue_smart_buffer_sync(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, uint32 sync)
{
    tbl_qos_queue_smart_buffer_master_t *p_master = _g_p_tbl_qos_queue_smart_buffer_master;
    tbl_qos_queue_smart_buffer_t *p_db_qos_queue_smart_buffer = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&p_qos_queue_smart_buffer->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_queue_smart_buffer = XCALLOC(MEM_TBL_QOS_QUEUE_SMART_BUFFER, sizeof(tbl_qos_queue_smart_buffer_t));
    if (NULL == p_db_qos_queue_smart_buffer)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_queue_smart_buffer, p_qos_queue_smart_buffer, sizeof(tbl_qos_queue_smart_buffer_t));

    /* 4. add to db */
    p_master->qos_queue_smart_buffer_array[p_qos_queue_smart_buffer->key.queue_id] = p_db_qos_queue_smart_buffer;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_QUEUE_SMART_BUFFER, p_db_qos_queue_smart_buffer);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_QUEUE_SMART_BUFFER, p_db_qos_queue_smart_buffer);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_del_qos_queue_smart_buffer_sync(tbl_qos_queue_smart_buffer_key_t *p_qos_queue_smart_buffer_key, uint32 sync)
{
    tbl_qos_queue_smart_buffer_master_t *p_master = _g_p_tbl_qos_queue_smart_buffer_master;
    tbl_qos_queue_smart_buffer_t *p_db_qos_queue_smart_buffer = NULL;

    /* 1. lookup entry exist */
    p_db_qos_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(p_qos_queue_smart_buffer_key);
    if (NULL == p_db_qos_queue_smart_buffer)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_QUEUE_SMART_BUFFER, p_db_qos_queue_smart_buffer);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_QUEUE_SMART_BUFFER, p_db_qos_queue_smart_buffer);
        #endif 
    }

    /* 3. delete from db */
    p_master->qos_queue_smart_buffer_array[p_qos_queue_smart_buffer_key->queue_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_QUEUE_SMART_BUFFER, p_db_qos_queue_smart_buffer);

    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_set_qos_queue_smart_buffer_field_sync(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, tbl_qos_queue_smart_buffer_field_id_t field_id, uint32 sync)
{
    tbl_qos_queue_smart_buffer_t *p_db_qos_queue_smart_buffer = NULL;

    /* 1. lookup entry exist */
    p_db_qos_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&p_qos_queue_smart_buffer->key);
    if (NULL == p_db_qos_queue_smart_buffer)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE:
        p_db_qos_queue_smart_buffer->smart_buffer_enable = p_qos_queue_smart_buffer->smart_buffer_enable;
        break;

    case TBL_QOS_QUEUE_SMART_BUFFER_FLD_MAX:
        sal_memcpy(p_db_qos_queue_smart_buffer, p_qos_queue_smart_buffer, sizeof(tbl_qos_queue_smart_buffer_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_QUEUE_SMART_BUFFER, field_id, p_db_qos_queue_smart_buffer);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_QUEUE_SMART_BUFFER, field_id, p_db_qos_queue_smart_buffer);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_add_qos_queue_smart_buffer(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer)
{
    return tbl_qos_queue_smart_buffer_add_qos_queue_smart_buffer_sync(p_qos_queue_smart_buffer, TRUE);
}

int32
tbl_qos_queue_smart_buffer_del_qos_queue_smart_buffer(tbl_qos_queue_smart_buffer_key_t *p_qos_queue_smart_buffer_key)
{
    return tbl_qos_queue_smart_buffer_del_qos_queue_smart_buffer_sync(p_qos_queue_smart_buffer_key, TRUE);
}

int32
tbl_qos_queue_smart_buffer_set_qos_queue_smart_buffer_field(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, tbl_qos_queue_smart_buffer_field_id_t field_id)
{
    return tbl_qos_queue_smart_buffer_set_qos_queue_smart_buffer_field_sync(p_qos_queue_smart_buffer, field_id, TRUE);
}

tbl_qos_queue_smart_buffer_t*
tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(tbl_qos_queue_smart_buffer_key_t *p_qos_queue_smart_buffer_key)
{
    tbl_qos_queue_smart_buffer_master_t *p_master = _g_p_tbl_qos_queue_smart_buffer_master;

    if (GLB_QOS_QUEUE_SMART_BUFFER_NUM <= p_qos_queue_smart_buffer_key->queue_id)
    {
        return NULL;
    }
    return (p_master->qos_queue_smart_buffer_array[p_qos_queue_smart_buffer_key->queue_id]);
}

char*
tbl_qos_queue_smart_buffer_key_val2str(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_queue_smart_buffer->key.queue_id);
    return str;
}

int32
tbl_qos_queue_smart_buffer_key_str2val(char *str, tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer)
{
    int32 ret = 0;

    p_qos_queue_smart_buffer->key.queue_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_qos_queue_smart_buffer_key_name_dump(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_QUEUE_SMART_BUFFER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_qos_queue_smart_buffer->key.queue_id);
    return str;
}

char*
tbl_qos_queue_smart_buffer_key_value_dump(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, char *str)
{
    sal_sprintf(str, "%u", p_qos_queue_smart_buffer->key.queue_id);
    return str;
}

char*
tbl_qos_queue_smart_buffer_field_name_dump(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_QUEUE_SMART_BUFFER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_QUEUE_SMART_BUFFER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_queue_smart_buffer_field_value_dump(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_queue_smart_buffer->key.queue_id);
    }
    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_queue_smart_buffer->smart_buffer_enable);
    }
    return str;
}

char**
tbl_qos_queue_smart_buffer_table_dump(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_queue_smart_buffer_key_name_dump(p_qos_queue_smart_buffer, buf));
    for(i=1; i<TBL_QOS_QUEUE_SMART_BUFFER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_queue_smart_buffer_field_name_dump(p_qos_queue_smart_buffer, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_queue_smart_buffer_field_value_dump(p_qos_queue_smart_buffer, i, buf));
    }
    return str;
}

int32
tbl_qos_queue_smart_buffer_field_value_parser(char *str, int32 field_id, tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer)
{
    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_KEY, field_id))
    {
        int32 ret;
        p_qos_queue_smart_buffer->key.queue_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE, field_id))
    {
        int32 ret;
        p_qos_queue_smart_buffer->smart_buffer_enable = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_table_parser(char** array, char* key_value,tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_QUEUE_SMART_BUFFER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_queue_smart_buffer_key_str2val(key_value, p_qos_queue_smart_buffer));

    for(i=1; i<TBL_QOS_QUEUE_SMART_BUFFER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_queue_smart_buffer_field_value_parser( array[j++], i, p_qos_queue_smart_buffer));
    }

    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_dump_one(tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_QUEUE_SMART_BUFFER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_QUEUE_SMART_BUFFER_FLD_KEY].name,
            p_qos_queue_smart_buffer->key.queue_id);
    }
    if (FLD_MATCH(TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE].name,
            p_qos_queue_smart_buffer->smart_buffer_enable);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_queue_smart_buffer_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_queue_smart_buffer_master_t *p_master = _g_p_tbl_qos_queue_smart_buffer_master;
    tbl_qos_queue_smart_buffer_t *p_db_qos_queue_smart_buffer = NULL;
    uint32 queue_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (queue_id = 0; queue_id < GLB_QOS_QUEUE_SMART_BUFFER_NUM; queue_id++)
    {
        p_db_qos_queue_smart_buffer = p_master->qos_queue_smart_buffer_array[queue_id];
        if (NULL == p_db_qos_queue_smart_buffer)
        {
            continue;
        }
        rc |= fn(p_db_qos_queue_smart_buffer, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_queue_smart_buffer_master_t*
tbl_qos_queue_smart_buffer_get_master()
{
    return _g_p_tbl_qos_queue_smart_buffer_master;
}

tbl_qos_queue_smart_buffer_master_t*
tbl_qos_queue_smart_buffer_init_qos_queue_smart_buffer()
{
    _g_p_tbl_qos_queue_smart_buffer_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_queue_smart_buffer_master_t));
    return _g_p_tbl_qos_queue_smart_buffer_master;
}


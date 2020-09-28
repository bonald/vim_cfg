
#include "proto.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "cdb_data_cmp.h"

tbl_igsp_group_master_t *_g_p_tbl_igsp_group_master = NULL;

static uint32
_tbl_igsp_group_hash_make(tbl_igsp_group_t *p_group)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_group->key;
    for (index = 0; index < sizeof(p_group->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_igsp_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_igsp_group_t *p_group1 = (tbl_igsp_group_t*)p_arg1;
    tbl_igsp_group_t *p_group2 = (tbl_igsp_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_group1->key, &p_group2->key, sizeof(tbl_igsp_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_igsp_group_add_igsp_group_sync(tbl_igsp_group_t *p_group, uint32 sync)
{
    tbl_igsp_group_master_t *p_master = _g_p_tbl_igsp_group_master;
    tbl_igsp_group_t *p_db_group = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_igsp_group_get_igsp_group(&p_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_group = XCALLOC(MEM_TBL_IGSP_GROUP, sizeof(tbl_igsp_group_t));
    if (NULL == p_db_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_group, p_group, sizeof(tbl_igsp_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->group_hash, (void*)p_db_group, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IGSP_GROUP, p_db_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IGSP_GROUP, p_db_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igsp_group_del_igsp_group_sync(tbl_igsp_group_key_t *p_group_key, uint32 sync)
{
    tbl_igsp_group_master_t *p_master = _g_p_tbl_igsp_group_master;
    tbl_igsp_group_t *p_db_group = NULL;

    /* 1. lookup entry exist */
    p_db_group = tbl_igsp_group_get_igsp_group(p_group_key);
    if (NULL == p_db_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IGSP_GROUP, p_db_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IGSP_GROUP, p_db_group);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->group_hash, (void*)p_db_group);

    /* 4. free db entry */
    XFREE(MEM_TBL_IGSP_GROUP, p_db_group);

    return PM_E_NONE;
}

int32
tbl_igsp_group_set_igsp_group_field_sync(tbl_igsp_group_t *p_group, tbl_igsp_group_field_id_t field_id, uint32 sync)
{
    tbl_igsp_group_t *p_db_group = NULL;

    /* 1. lookup entry exist */
    p_db_group = tbl_igsp_group_get_igsp_group(&p_group->key);
    if (NULL == p_db_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC:
        p_db_group->retx_group_lmqc = p_group->retx_group_lmqc;
        break;

    case TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC:
        p_db_group->retx_group_source_lmqc = p_group->retx_group_source_lmqc;
        break;

    case TBL_IGSP_GROUP_FLD_LIVENESS:
        p_db_group->liveness = p_group->liveness;
        break;

    case TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE:
        p_db_group->has_rx_leave = p_group->has_rx_leave;
        break;

    case TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS:
        sal_memcpy(&p_db_group->last_reporter_address, &p_group->last_reporter_address, sizeof(p_group->last_reporter_address));
        break;

    case TBL_IGSP_GROUP_FLD_UPTIME:
        sal_memcpy(&p_db_group->uptime, &p_group->uptime, sizeof(p_group->uptime));
        break;

    case TBL_IGSP_GROUP_FLD_UPDATE_TIME:
        sal_memcpy(&p_db_group->update_time, &p_group->update_time, sizeof(p_group->update_time));
        break;

    case TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME:
        sal_memcpy(&p_db_group->v1_update_time, &p_group->v1_update_time, sizeof(p_group->v1_update_time));
        break;

    case TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST:
        p_db_group->is_v1_host_exist = p_group->is_v1_host_exist;
        break;

    case TBL_IGSP_GROUP_FLD_TYPE:
        p_db_group->type = p_group->type;
        break;

    case TBL_IGSP_GROUP_FLD_INACTIVE:
        p_db_group->inactive = p_group->inactive;
        break;

    case TBL_IGSP_GROUP_FLD_IS_V3_LEAVE:
        p_db_group->is_v3_leave = p_group->is_v3_leave;
        break;

    case TBL_IGSP_GROUP_FLD_T_RETX_QUERY:
        p_db_group->t_retx_query = p_group->t_retx_query;
        break;

    case TBL_IGSP_GROUP_FLD_MAX:
        sal_memcpy(p_db_group, p_group, sizeof(tbl_igsp_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IGSP_GROUP, field_id, p_db_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IGSP_GROUP, field_id, p_db_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igsp_group_add_igsp_group(tbl_igsp_group_t *p_group)
{
    return tbl_igsp_group_add_igsp_group_sync(p_group, TRUE);
}

int32
tbl_igsp_group_del_igsp_group(tbl_igsp_group_key_t *p_group_key)
{
    return tbl_igsp_group_del_igsp_group_sync(p_group_key, TRUE);
}

int32
tbl_igsp_group_set_igsp_group_field(tbl_igsp_group_t *p_group, tbl_igsp_group_field_id_t field_id)
{
    return tbl_igsp_group_set_igsp_group_field_sync(p_group, field_id, TRUE);
}

tbl_igsp_group_t*
tbl_igsp_group_get_igsp_group(tbl_igsp_group_key_t *p_group_key)
{
    tbl_igsp_group_master_t *p_master = _g_p_tbl_igsp_group_master;
    tbl_igsp_group_t lkp;

    sal_memcpy(&lkp.key, p_group_key, sizeof(tbl_igsp_group_key_t));
    return ctclib_hash_lookup(p_master->group_hash, &lkp);
}

char*
tbl_igsp_group_key_val2str(tbl_igsp_group_t *p_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_igsp_group_val2str(buf, MAX_CMD_STR_LEN, &p_group->key));
    return str;
}

int32
tbl_igsp_group_key_str2val(char *str, tbl_igsp_group_t *p_group)
{
    int32 ret = 0;

    ret = cdb_igsp_group_str2val(str, &p_group->key);
    return ret;
}

char*
tbl_igsp_group_key_name_dump(tbl_igsp_group_t *p_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GROUP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_igsp_group_val2str(buf, MAX_CMD_STR_LEN, &p_group->key));
    return str;
}

char*
tbl_igsp_group_key_value_dump(tbl_igsp_group_t *p_group, char *str)
{

    cdb_igsp_group_val2str(str, MAX_CMD_STR_LEN, &p_group->key);
    return str;
}

char*
tbl_igsp_group_field_name_dump(tbl_igsp_group_t *p_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IGSP_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_igsp_group_field_value_dump(tbl_igsp_group_t *p_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_KEY, field_id))
    {
        cdb_igsp_group_val2str(str, MAX_CMD_STR_LEN, &p_group->key);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC, field_id))
    {
        sal_sprintf(str, "%u", p_group->retx_group_lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC, field_id))
    {
        sal_sprintf(str, "%u", p_group->retx_group_source_lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LIVENESS, field_id))
    {
        sal_sprintf(str, "%u", p_group->liveness);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE, field_id))
    {
        sal_sprintf(str, "%u", p_group->has_rx_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_group->last_reporter_address);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_group->uptime);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPDATE_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_group->update_time);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_group->v1_update_time);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST, field_id))
    {
        sal_sprintf(str, "%u", p_group->is_v1_host_exist);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_group->type);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_INACTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_group->inactive);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V3_LEAVE, field_id))
    {
        sal_sprintf(str, "%u", p_group->is_v3_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_T_RETX_QUERY, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_group->t_retx_query);
    }
    return str;
}

char**
tbl_igsp_group_table_dump(tbl_igsp_group_t *p_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_igsp_group_key_name_dump(p_group, buf));
    for(i=1; i<TBL_IGSP_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_igsp_group_field_name_dump(p_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_igsp_group_field_value_dump(p_group, i, buf));
    }
    return str;
}

int32
tbl_igsp_group_field_value_parser(char *str, int32 field_id, tbl_igsp_group_t *p_group)
{
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_KEY, field_id))
    {
        cdb_igsp_group_str2val(str, &p_group->key);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC, field_id))
    {
        int32 ret;
        p_group->retx_group_lmqc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC, field_id))
    {
        int32 ret;
        p_group->retx_group_source_lmqc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LIVENESS, field_id))
    {
        int32 ret;
        p_group->liveness = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE, field_id))
    {
        int32 ret;
        p_group->has_rx_leave = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_group->last_reporter_address, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_group->uptime);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPDATE_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_group->update_time);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_group->v1_update_time);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST, field_id))
    {
        int32 ret;
        p_group->is_v1_host_exist = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_TYPE, field_id))
    {
        int32 ret;
        p_group->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_INACTIVE, field_id))
    {
        int32 ret;
        p_group->inactive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V3_LEAVE, field_id))
    {
        int32 ret;
        p_group->is_v3_leave = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_T_RETX_QUERY, field_id))
    {
        ctc_task_str2val(str, p_group->t_retx_query);
    }
    return PM_E_NONE;
}

int32
tbl_igsp_group_table_parser(char** array, char* key_value,tbl_igsp_group_t *p_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_igsp_group_key_str2val(key_value, p_group));

    for(i=1; i<TBL_IGSP_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_igsp_group_field_value_parser( array[j++], i, p_group));
    }

    return PM_E_NONE;
}

int32
tbl_igsp_group_dump_one(tbl_igsp_group_t *p_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_KEY].name, 
            cdb_igsp_group_val2str(buf, MAX_CMD_STR_LEN, &p_group->key));
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC].name,
            p_group->retx_group_lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC].name,
            p_group->retx_group_source_lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LIVENESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_LIVENESS].name,
            p_group->liveness);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE].name,
            p_group->has_rx_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_group->last_reporter_address));
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_UPTIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_group->uptime));
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_UPDATE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_UPDATE_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_group->update_time));
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_group->v1_update_time));
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST].name,
            p_group->is_v1_host_exist);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_TYPE].name,
            p_group->type);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_INACTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_INACTIVE].name,
            p_group->inactive);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_IS_V3_LEAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GROUP_FLD_IS_V3_LEAVE].name,
            p_group->is_v3_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GROUP_FLD_T_RETX_QUERY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GROUP_FLD_T_RETX_QUERY].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_group->t_retx_query));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_igsp_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_igsp_group_master_t *p_master = _g_p_tbl_igsp_group_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->group_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_igsp_group_master_t*
tbl_igsp_group_get_master()
{
    return _g_p_tbl_igsp_group_master;
}

tbl_igsp_group_master_t*
tbl_igsp_group_init_igsp_group()
{
    _g_p_tbl_igsp_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_igsp_group_master_t));
    _g_p_tbl_igsp_group_master->group_hash = ctclib_hash_create(_tbl_igsp_group_hash_make, _tbl_igsp_group_hash_cmp);
    return _g_p_tbl_igsp_group_master;
}


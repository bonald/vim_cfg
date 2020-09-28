
#include "proto.h"
#include "gen/tbl_ptp_residence_time_cache_define.h"
#include "gen/tbl_ptp_residence_time_cache.h"
#include "cdb_data_cmp.h"

tbl_ptp_residence_time_cache_master_t *_g_p_tbl_ptp_residence_time_cache_master = NULL;

static uint32
_tbl_ptp_residence_time_cache_hash_make(tbl_ptp_residence_time_cache_t *p_residence_time_cache)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_residence_time_cache->key;
    for (index = 0; index < sizeof(p_residence_time_cache->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ptp_residence_time_cache_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ptp_residence_time_cache_t *p_residence_time_cache1 = (tbl_ptp_residence_time_cache_t*)p_arg1;
    tbl_ptp_residence_time_cache_t *p_residence_time_cache2 = (tbl_ptp_residence_time_cache_t*)p_arg2;

    if (0 == sal_memcmp(&p_residence_time_cache1->key, &p_residence_time_cache2->key, sizeof(tbl_ptp_residence_time_cache_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ptp_residence_time_cache_add_ptp_residence_time_cache_sync(tbl_ptp_residence_time_cache_t *p_residence_time_cache, uint32 sync)
{
    tbl_ptp_residence_time_cache_master_t *p_master = _g_p_tbl_ptp_residence_time_cache_master;
    tbl_ptp_residence_time_cache_t *p_db_residence_time_cache = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&p_residence_time_cache->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_residence_time_cache = XCALLOC(MEM_TBL_PTP_RESIDENCE_TIME_CACHE, sizeof(tbl_ptp_residence_time_cache_t));
    if (NULL == p_db_residence_time_cache)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_residence_time_cache, p_residence_time_cache, sizeof(tbl_ptp_residence_time_cache_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ptp_residence_time_cache_hash, (void*)p_db_residence_time_cache, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ptp_residence_time_cache_list, p_db_residence_time_cache);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PTP_RESIDENCE_TIME_CACHE, p_db_residence_time_cache);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PTP_RESIDENCE_TIME_CACHE, p_db_residence_time_cache);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_del_ptp_residence_time_cache_sync(tbl_ptp_residence_time_cache_key_t *p_residence_time_cache_key, uint32 sync)
{
    tbl_ptp_residence_time_cache_master_t *p_master = _g_p_tbl_ptp_residence_time_cache_master;
    tbl_ptp_residence_time_cache_t *p_db_residence_time_cache = NULL;

    /* 1. lookup entry exist */
    p_db_residence_time_cache = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(p_residence_time_cache_key);
    if (NULL == p_db_residence_time_cache)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PTP_RESIDENCE_TIME_CACHE, p_db_residence_time_cache);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PTP_RESIDENCE_TIME_CACHE, p_db_residence_time_cache);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ptp_residence_time_cache_hash, (void*)p_db_residence_time_cache);
    ctclib_slistnode_delete(p_master->ptp_residence_time_cache_list, p_db_residence_time_cache);

    /* 4. free db entry */
    XFREE(MEM_TBL_PTP_RESIDENCE_TIME_CACHE, p_db_residence_time_cache);

    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field_sync(tbl_ptp_residence_time_cache_t *p_residence_time_cache, tbl_ptp_residence_time_cache_field_id_t field_id, uint32 sync)
{
    tbl_ptp_residence_time_cache_t *p_db_residence_time_cache = NULL;

    /* 1. lookup entry exist */
    p_db_residence_time_cache = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&p_residence_time_cache->key);
    if (NULL == p_db_residence_time_cache)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID:
        p_db_residence_time_cache->sequence_id = p_residence_time_cache->sequence_id;
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS:
        p_db_residence_time_cache->ifindex_port_ingress = p_residence_time_cache->ifindex_port_ingress;
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN:
        p_db_residence_time_cache->domain = p_residence_time_cache->domain;
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER:
        p_db_residence_time_cache->ptm_age_task_timer = p_residence_time_cache->ptm_age_task_timer;
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME:
        sal_memcpy(&p_db_residence_time_cache->residence_time, &p_residence_time_cache->residence_time, sizeof(p_residence_time_cache->residence_time));
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED:
        p_db_residence_time_cache->outdated = p_residence_time_cache->outdated;
        break;

    case TBL_PTP_RESIDENCE_TIME_CACHE_FLD_MAX:
        sal_memcpy(p_db_residence_time_cache, p_residence_time_cache, sizeof(tbl_ptp_residence_time_cache_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PTP_RESIDENCE_TIME_CACHE, field_id, p_db_residence_time_cache);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PTP_RESIDENCE_TIME_CACHE, field_id, p_db_residence_time_cache);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_add_ptp_residence_time_cache(tbl_ptp_residence_time_cache_t *p_residence_time_cache)
{
    return tbl_ptp_residence_time_cache_add_ptp_residence_time_cache_sync(p_residence_time_cache, TRUE);
}

int32
tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(tbl_ptp_residence_time_cache_key_t *p_residence_time_cache_key)
{
    return tbl_ptp_residence_time_cache_del_ptp_residence_time_cache_sync(p_residence_time_cache_key, TRUE);
}

int32
tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(tbl_ptp_residence_time_cache_t *p_residence_time_cache, tbl_ptp_residence_time_cache_field_id_t field_id)
{
    return tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field_sync(p_residence_time_cache, field_id, TRUE);
}

tbl_ptp_residence_time_cache_t*
tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(tbl_ptp_residence_time_cache_key_t *p_residence_time_cache_key)
{
    tbl_ptp_residence_time_cache_master_t *p_master = _g_p_tbl_ptp_residence_time_cache_master;
    tbl_ptp_residence_time_cache_t lkp;

    sal_memcpy(&lkp.key, p_residence_time_cache_key, sizeof(tbl_ptp_residence_time_cache_key_t));
    return ctclib_hash_lookup(p_master->ptp_residence_time_cache_hash, &lkp);
}

char*
tbl_ptp_residence_time_cache_key_val2str(tbl_ptp_residence_time_cache_t *p_residence_time_cache, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ptp_residence_time_cache_val2str(buf, MAX_CMD_STR_LEN, &p_residence_time_cache->key));
    return str;
}

int32
tbl_ptp_residence_time_cache_key_str2val(char *str, tbl_ptp_residence_time_cache_t *p_residence_time_cache)
{
    int32 ret = 0;

    ret = cdb_ptp_residence_time_cache_str2val(str, &p_residence_time_cache->key);
    return ret;
}

char*
tbl_ptp_residence_time_cache_key_name_dump(tbl_ptp_residence_time_cache_t *p_residence_time_cache, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_RESIDENCE_TIME_CACHE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ptp_residence_time_cache_val2str(buf, MAX_CMD_STR_LEN, &p_residence_time_cache->key));
    return str;
}

char*
tbl_ptp_residence_time_cache_key_value_dump(tbl_ptp_residence_time_cache_t *p_residence_time_cache, char *str)
{

    cdb_ptp_residence_time_cache_val2str(str, MAX_CMD_STR_LEN, &p_residence_time_cache->key);
    return str;
}

char*
tbl_ptp_residence_time_cache_field_name_dump(tbl_ptp_residence_time_cache_t *p_residence_time_cache, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_RESIDENCE_TIME_CACHE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PTP_RESIDENCE_TIME_CACHE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ptp_residence_time_cache_field_value_dump(tbl_ptp_residence_time_cache_t *p_residence_time_cache, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_KEY, field_id))
    {
        cdb_ptp_residence_time_cache_val2str(str, MAX_CMD_STR_LEN, &p_residence_time_cache->key);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_residence_time_cache->sequence_id);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_residence_time_cache->ifindex_port_ingress);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_residence_time_cache->domain);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_residence_time_cache->ptm_age_task_timer);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME, field_id))
    {
        cdb_glb_ptp_timeinterval_val2str(str, MAX_CMD_STR_LEN, &p_residence_time_cache->residence_time);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED, field_id))
    {
        sal_sprintf(str, "%u", p_residence_time_cache->outdated);
    }
    return str;
}

char**
tbl_ptp_residence_time_cache_table_dump(tbl_ptp_residence_time_cache_t *p_residence_time_cache, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ptp_residence_time_cache_key_name_dump(p_residence_time_cache, buf));
    for(i=1; i<TBL_PTP_RESIDENCE_TIME_CACHE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ptp_residence_time_cache_field_name_dump(p_residence_time_cache, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ptp_residence_time_cache_field_value_dump(p_residence_time_cache, i, buf));
    }
    return str;
}

int32
tbl_ptp_residence_time_cache_field_value_parser(char *str, int32 field_id, tbl_ptp_residence_time_cache_t *p_residence_time_cache)
{
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_KEY, field_id))
    {
        cdb_ptp_residence_time_cache_str2val(str, &p_residence_time_cache->key);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID, field_id))
    {
        int32 ret;
        p_residence_time_cache->sequence_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS, field_id))
    {
        int32 ret;
        p_residence_time_cache->ifindex_port_ingress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN, field_id))
    {
        int32 ret;
        p_residence_time_cache->domain = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER, field_id))
    {
        ctc_task_str2val(str, p_residence_time_cache->ptm_age_task_timer);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME, field_id))
    {
        cdb_glb_ptp_timeinterval_str2val(str, &p_residence_time_cache->residence_time);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED, field_id))
    {
        int32 ret;
        p_residence_time_cache->outdated = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_table_parser(char** array, char* key_value,tbl_ptp_residence_time_cache_t *p_residence_time_cache)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_RESIDENCE_TIME_CACHE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ptp_residence_time_cache_key_str2val(key_value, p_residence_time_cache));

    for(i=1; i<TBL_PTP_RESIDENCE_TIME_CACHE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ptp_residence_time_cache_field_value_parser( array[j++], i, p_residence_time_cache));
    }

    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_dump_one(tbl_ptp_residence_time_cache_t *p_residence_time_cache, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_RESIDENCE_TIME_CACHE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_KEY].name, 
            cdb_ptp_residence_time_cache_val2str(buf, MAX_CMD_STR_LEN, &p_residence_time_cache->key));
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID].name,
            p_residence_time_cache->sequence_id);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS].name,
            p_residence_time_cache->ifindex_port_ingress);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN].name,
            p_residence_time_cache->domain);
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_PTM_AGE_TASK_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_residence_time_cache->ptm_age_task_timer));
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME].name, 
            cdb_glb_ptp_timeinterval_val2str(buf, MAX_CMD_STR_LEN, &p_residence_time_cache->residence_time));
    }
    if (FLD_MATCH(TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED].name,
            p_residence_time_cache->outdated);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ptp_residence_time_cache_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ptp_residence_time_cache_master_t *p_master = _g_p_tbl_ptp_residence_time_cache_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ptp_residence_time_cache_t *p_db_residence_time_cache = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ptp_residence_time_cache_list, p_db_residence_time_cache, listnode, next)
    {
        rc |= fn(p_db_residence_time_cache, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ptp_residence_time_cache_master_t*
tbl_ptp_residence_time_cache_get_master()
{
    return _g_p_tbl_ptp_residence_time_cache_master;
}

tbl_ptp_residence_time_cache_master_t*
tbl_ptp_residence_time_cache_init_ptp_residence_time_cache()
{
    _g_p_tbl_ptp_residence_time_cache_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ptp_residence_time_cache_master_t));
    _g_p_tbl_ptp_residence_time_cache_master->ptp_residence_time_cache_hash = ctclib_hash_create(_tbl_ptp_residence_time_cache_hash_make, _tbl_ptp_residence_time_cache_hash_cmp);
    _g_p_tbl_ptp_residence_time_cache_master->ptp_residence_time_cache_list = ctclib_slist_create(tbl_ptp_residence_time_cache_cmp, NULL);
    return _g_p_tbl_ptp_residence_time_cache_master;
}


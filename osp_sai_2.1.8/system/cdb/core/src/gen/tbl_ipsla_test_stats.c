
#include "proto.h"
#include "gen/tbl_ipsla_test_stats_define.h"
#include "gen/tbl_ipsla_test_stats.h"
#include "cdb_data_cmp.h"

tbl_ipsla_test_stats_master_t *_g_p_tbl_ipsla_test_stats_master = NULL;

static uint32
_tbl_ipsla_test_stats_hash_make(tbl_ipsla_test_stats_t *p_ipsla_test_stats)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ipsla_test_stats->key;
    for (index = 0; index < sizeof(p_ipsla_test_stats->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ipsla_test_stats_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsla_test_stats_t *p_ipsla_test_stats1 = (tbl_ipsla_test_stats_t*)p_arg1;
    tbl_ipsla_test_stats_t *p_ipsla_test_stats2 = (tbl_ipsla_test_stats_t*)p_arg2;

    if (0 == sal_memcmp(&p_ipsla_test_stats1->key, &p_ipsla_test_stats2->key, sizeof(tbl_ipsla_test_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsla_test_stats_add_ipsla_test_stats_sync(tbl_ipsla_test_stats_t *p_ipsla_test_stats, uint32 sync)
{
    tbl_ipsla_test_stats_master_t *p_master = _g_p_tbl_ipsla_test_stats_master;
    tbl_ipsla_test_stats_t *p_db_ipsla_test_stats = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipsla_test_stats_get_ipsla_test_stats(&p_ipsla_test_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipsla_test_stats = XCALLOC(MEM_TBL_IPSLA_TEST_STATS, sizeof(tbl_ipsla_test_stats_t));
    if (NULL == p_db_ipsla_test_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipsla_test_stats, p_ipsla_test_stats, sizeof(tbl_ipsla_test_stats_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ipsla_test_stats_hash, (void*)p_db_ipsla_test_stats, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ipsla_test_stats_list, p_db_ipsla_test_stats);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPSLA_TEST_STATS, p_db_ipsla_test_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPSLA_TEST_STATS, p_db_ipsla_test_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_del_ipsla_test_stats_sync(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key, uint32 sync)
{
    tbl_ipsla_test_stats_master_t *p_master = _g_p_tbl_ipsla_test_stats_master;
    tbl_ipsla_test_stats_t *p_db_ipsla_test_stats = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_test_stats = tbl_ipsla_test_stats_get_ipsla_test_stats(p_ipsla_test_stats_key);
    if (NULL == p_db_ipsla_test_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPSLA_TEST_STATS, p_db_ipsla_test_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPSLA_TEST_STATS, p_db_ipsla_test_stats);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ipsla_test_stats_hash, (void*)p_db_ipsla_test_stats);
    ctclib_slistnode_delete(p_master->ipsla_test_stats_list, p_db_ipsla_test_stats);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPSLA_TEST_STATS, p_db_ipsla_test_stats);

    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_set_ipsla_test_stats_field_sync(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_ipsla_test_stats_field_id_t field_id, uint32 sync)
{
    tbl_ipsla_test_stats_t *p_db_ipsla_test_stats = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_test_stats = tbl_ipsla_test_stats_get_ipsla_test_stats(&p_ipsla_test_stats->key);
    if (NULL == p_db_ipsla_test_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME:
        p_db_ipsla_test_stats->last_send_time = p_ipsla_test_stats->last_send_time;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_TESTID:
        p_db_ipsla_test_stats->testid = p_ipsla_test_stats->testid;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_SEND_CNT:
        p_db_ipsla_test_stats->send_cnt = p_ipsla_test_stats->send_cnt;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT:
        p_db_ipsla_test_stats->receive_cnt = p_ipsla_test_stats->receive_cnt;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT:
        p_db_ipsla_test_stats->overthreshold_cnt = p_ipsla_test_stats->overthreshold_cnt;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT:
        p_db_ipsla_test_stats->sendfail_cnt = p_ipsla_test_stats->sendfail_cnt;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_RTT_MIN:
        p_db_ipsla_test_stats->rtt_min = p_ipsla_test_stats->rtt_min;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_RTT_MAX:
        p_db_ipsla_test_stats->rtt_max = p_ipsla_test_stats->rtt_max;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_RTT_SUM:
        p_db_ipsla_test_stats->rtt_sum = p_ipsla_test_stats->rtt_sum;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_SUCCESS:
        p_db_ipsla_test_stats->success = p_ipsla_test_stats->success;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT:
        p_db_ipsla_test_stats->fail_percent = p_ipsla_test_stats->fail_percent;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_TIMEOUT:
        p_db_ipsla_test_stats->timeout = p_ipsla_test_stats->timeout;
        break;

    case TBL_IPSLA_TEST_STATS_FLD_MAX:
        sal_memcpy(p_db_ipsla_test_stats, p_ipsla_test_stats, sizeof(tbl_ipsla_test_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSLA_TEST_STATS, field_id, p_db_ipsla_test_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPSLA_TEST_STATS, field_id, p_db_ipsla_test_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_add_ipsla_test_stats(tbl_ipsla_test_stats_t *p_ipsla_test_stats)
{
    return tbl_ipsla_test_stats_add_ipsla_test_stats_sync(p_ipsla_test_stats, TRUE);
}

int32
tbl_ipsla_test_stats_del_ipsla_test_stats(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key)
{
    return tbl_ipsla_test_stats_del_ipsla_test_stats_sync(p_ipsla_test_stats_key, TRUE);
}

int32
tbl_ipsla_test_stats_set_ipsla_test_stats_field(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_ipsla_test_stats_field_id_t field_id)
{
    return tbl_ipsla_test_stats_set_ipsla_test_stats_field_sync(p_ipsla_test_stats, field_id, TRUE);
}

tbl_ipsla_test_stats_t*
tbl_ipsla_test_stats_get_ipsla_test_stats(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key)
{
    tbl_ipsla_test_stats_master_t *p_master = _g_p_tbl_ipsla_test_stats_master;
    tbl_ipsla_test_stats_t lkp;

    sal_memcpy(&lkp.key, p_ipsla_test_stats_key, sizeof(tbl_ipsla_test_stats_key_t));
    return ctclib_hash_lookup(p_master->ipsla_test_stats_hash, &lkp);
}

char*
tbl_ipsla_test_stats_key_val2str(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ipsla_test_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_test_stats->key));
    return str;
}

int32
tbl_ipsla_test_stats_key_str2val(char *str, tbl_ipsla_test_stats_t *p_ipsla_test_stats)
{
    int32 ret = 0;

    ret = cdb_ipsla_test_stats_key_str2val(str, &p_ipsla_test_stats->key);
    return ret;
}

char*
tbl_ipsla_test_stats_key_name_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_TEST_STATS);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ipsla_test_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_test_stats->key));
    return str;
}

char*
tbl_ipsla_test_stats_key_value_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char *str)
{

    cdb_ipsla_test_stats_key_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_test_stats->key);
    return str;
}

char*
tbl_ipsla_test_stats_field_name_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_TEST_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPSLA_TEST_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipsla_test_stats_field_value_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_KEY, field_id))
    {
        cdb_ipsla_test_stats_key_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_test_stats->key);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_ipsla_test_stats->last_send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TESTID, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SEND_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->send_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->receive_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->overthreshold_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->sendfail_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_SUM, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->success);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_test_stats->timeout);
    }
    return str;
}

char**
tbl_ipsla_test_stats_table_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipsla_test_stats_key_name_dump(p_ipsla_test_stats, buf));
    for(i=1; i<TBL_IPSLA_TEST_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_test_stats_field_name_dump(p_ipsla_test_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_test_stats_field_value_dump(p_ipsla_test_stats, i, buf));
    }
    return str;
}

int32
tbl_ipsla_test_stats_field_value_parser(char *str, int32 field_id, tbl_ipsla_test_stats_t *p_ipsla_test_stats)
{
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_KEY, field_id))
    {
        cdb_ipsla_test_stats_key_str2val(str, &p_ipsla_test_stats->key);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->last_send_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TESTID, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->testid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SEND_CNT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->send_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->receive_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->overthreshold_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->sendfail_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MIN, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->rtt_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MAX, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->rtt_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_SUM, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->rtt_sum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SUCCESS, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->success = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->fail_percent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_ipsla_test_stats->timeout = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_table_parser(char** array, char* key_value,tbl_ipsla_test_stats_t *p_ipsla_test_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_TEST_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipsla_test_stats_key_str2val(key_value, p_ipsla_test_stats));

    for(i=1; i<TBL_IPSLA_TEST_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipsla_test_stats_field_value_parser( array[j++], i, p_ipsla_test_stats));
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_dump_one(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_TEST_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_KEY].name, 
            cdb_ipsla_test_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_test_stats->key));
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME].name,
            p_ipsla_test_stats->last_send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TESTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_TESTID].name,
            p_ipsla_test_stats->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SEND_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_SEND_CNT].name,
            p_ipsla_test_stats->send_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT].name,
            p_ipsla_test_stats->receive_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT].name,
            p_ipsla_test_stats->overthreshold_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT].name,
            p_ipsla_test_stats->sendfail_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_RTT_MIN].name,
            p_ipsla_test_stats->rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_RTT_MAX].name,
            p_ipsla_test_stats->rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_RTT_SUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_RTT_SUM].name,
            p_ipsla_test_stats->rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_SUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_SUCCESS].name,
            p_ipsla_test_stats->success);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT].name,
            p_ipsla_test_stats->fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_TEST_STATS_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_TEST_STATS_FLD_TIMEOUT].name,
            p_ipsla_test_stats->timeout);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsla_test_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsla_test_stats_master_t *p_master = _g_p_tbl_ipsla_test_stats_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ipsla_test_stats_t *p_db_ipsla_test_stats = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ipsla_test_stats_list, p_db_ipsla_test_stats, listnode, next)
    {
        rc |= fn(p_db_ipsla_test_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipsla_test_stats_master_t*
tbl_ipsla_test_stats_get_master()
{
    return _g_p_tbl_ipsla_test_stats_master;
}

tbl_ipsla_test_stats_master_t*
tbl_ipsla_test_stats_init_ipsla_test_stats()
{
    _g_p_tbl_ipsla_test_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipsla_test_stats_master_t));
    _g_p_tbl_ipsla_test_stats_master->ipsla_test_stats_hash = ctclib_hash_create(_tbl_ipsla_test_stats_hash_make, _tbl_ipsla_test_stats_hash_cmp);
    _g_p_tbl_ipsla_test_stats_master->ipsla_test_stats_list = ctclib_slist_create(tbl_ipsla_test_stats_cmp, NULL);
    return _g_p_tbl_ipsla_test_stats_master;
}



#include "proto.h"
#include "gen/tbl_ipsla_packet_stats_define.h"
#include "gen/tbl_ipsla_packet_stats.h"
#include "cdb_data_cmp.h"

tbl_ipsla_packet_stats_master_t *_g_p_tbl_ipsla_packet_stats_master = NULL;

static uint32
_tbl_ipsla_packet_stats_hash_make(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ipsla_packet_stats->key;
    for (index = 0; index < sizeof(p_ipsla_packet_stats->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ipsla_packet_stats_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsla_packet_stats_t *p_ipsla_packet_stats1 = (tbl_ipsla_packet_stats_t*)p_arg1;
    tbl_ipsla_packet_stats_t *p_ipsla_packet_stats2 = (tbl_ipsla_packet_stats_t*)p_arg2;

    if (0 == sal_memcmp(&p_ipsla_packet_stats1->key, &p_ipsla_packet_stats2->key, sizeof(tbl_ipsla_packet_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsla_packet_stats_add_ipsla_packet_stats_sync(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, uint32 sync)
{
    tbl_ipsla_packet_stats_master_t *p_master = _g_p_tbl_ipsla_packet_stats_master;
    tbl_ipsla_packet_stats_t *p_db_ipsla_packet_stats = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipsla_packet_stats_get_ipsla_packet_stats(&p_ipsla_packet_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipsla_packet_stats = XCALLOC(MEM_TBL_IPSLA_PACKET_STATS, sizeof(tbl_ipsla_packet_stats_t));
    if (NULL == p_db_ipsla_packet_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipsla_packet_stats, p_ipsla_packet_stats, sizeof(tbl_ipsla_packet_stats_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ipsla_packet_stats_hash, (void*)p_db_ipsla_packet_stats, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ipsla_packet_stats_list, p_db_ipsla_packet_stats);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPSLA_PACKET_STATS, p_db_ipsla_packet_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPSLA_PACKET_STATS, p_db_ipsla_packet_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_del_ipsla_packet_stats_sync(tbl_ipsla_packet_stats_key_t *p_ipsla_packet_stats_key, uint32 sync)
{
    tbl_ipsla_packet_stats_master_t *p_master = _g_p_tbl_ipsla_packet_stats_master;
    tbl_ipsla_packet_stats_t *p_db_ipsla_packet_stats = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_packet_stats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(p_ipsla_packet_stats_key);
    if (NULL == p_db_ipsla_packet_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPSLA_PACKET_STATS, p_db_ipsla_packet_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPSLA_PACKET_STATS, p_db_ipsla_packet_stats);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ipsla_packet_stats_hash, (void*)p_db_ipsla_packet_stats);
    ctclib_slistnode_delete(p_master->ipsla_packet_stats_list, p_db_ipsla_packet_stats);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPSLA_PACKET_STATS, p_db_ipsla_packet_stats);

    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_set_ipsla_packet_stats_field_sync(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, tbl_ipsla_packet_stats_field_id_t field_id, uint32 sync)
{
    tbl_ipsla_packet_stats_t *p_db_ipsla_packet_stats = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_packet_stats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&p_ipsla_packet_stats->key);
    if (NULL == p_db_ipsla_packet_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME:
        p_db_ipsla_packet_stats->send_time = p_ipsla_packet_stats->send_time;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_RTT:
        p_db_ipsla_packet_stats->rtt = p_ipsla_packet_stats->rtt;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_TESTID:
        p_db_ipsla_packet_stats->testid = p_ipsla_packet_stats->testid;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_INDEX:
        p_db_ipsla_packet_stats->index = p_ipsla_packet_stats->index;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_STATUS:
        p_db_ipsla_packet_stats->status = p_ipsla_packet_stats->status;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT:
        p_db_ipsla_packet_stats->timeout = p_ipsla_packet_stats->timeout;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD:
        p_db_ipsla_packet_stats->threshold = p_ipsla_packet_stats->threshold;
        break;

    case TBL_IPSLA_PACKET_STATS_FLD_MAX:
        sal_memcpy(p_db_ipsla_packet_stats, p_ipsla_packet_stats, sizeof(tbl_ipsla_packet_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSLA_PACKET_STATS, field_id, p_db_ipsla_packet_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPSLA_PACKET_STATS, field_id, p_db_ipsla_packet_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_add_ipsla_packet_stats(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats)
{
    return tbl_ipsla_packet_stats_add_ipsla_packet_stats_sync(p_ipsla_packet_stats, TRUE);
}

int32
tbl_ipsla_packet_stats_del_ipsla_packet_stats(tbl_ipsla_packet_stats_key_t *p_ipsla_packet_stats_key)
{
    return tbl_ipsla_packet_stats_del_ipsla_packet_stats_sync(p_ipsla_packet_stats_key, TRUE);
}

int32
tbl_ipsla_packet_stats_set_ipsla_packet_stats_field(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, tbl_ipsla_packet_stats_field_id_t field_id)
{
    return tbl_ipsla_packet_stats_set_ipsla_packet_stats_field_sync(p_ipsla_packet_stats, field_id, TRUE);
}

tbl_ipsla_packet_stats_t*
tbl_ipsla_packet_stats_get_ipsla_packet_stats(tbl_ipsla_packet_stats_key_t *p_ipsla_packet_stats_key)
{
    tbl_ipsla_packet_stats_master_t *p_master = _g_p_tbl_ipsla_packet_stats_master;
    tbl_ipsla_packet_stats_t lkp;

    sal_memcpy(&lkp.key, p_ipsla_packet_stats_key, sizeof(tbl_ipsla_packet_stats_key_t));
    return ctclib_hash_lookup(p_master->ipsla_packet_stats_hash, &lkp);
}

char*
tbl_ipsla_packet_stats_key_val2str(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ipsla_packet_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_packet_stats->key));
    return str;
}

int32
tbl_ipsla_packet_stats_key_str2val(char *str, tbl_ipsla_packet_stats_t *p_ipsla_packet_stats)
{
    int32 ret = 0;

    ret = cdb_ipsla_packet_stats_key_str2val(str, &p_ipsla_packet_stats->key);
    return ret;
}

char*
tbl_ipsla_packet_stats_key_name_dump(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_PACKET_STATS);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ipsla_packet_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_packet_stats->key));
    return str;
}

char*
tbl_ipsla_packet_stats_key_value_dump(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, char *str)
{

    cdb_ipsla_packet_stats_key_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_packet_stats->key);
    return str;
}

char*
tbl_ipsla_packet_stats_field_name_dump(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_PACKET_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPSLA_PACKET_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipsla_packet_stats_field_value_dump(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_KEY, field_id))
    {
        cdb_ipsla_packet_stats_key_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_packet_stats->key);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_ipsla_packet_stats->send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_RTT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->rtt);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TESTID, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->index);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->status);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_packet_stats->threshold);
    }
    return str;
}

char**
tbl_ipsla_packet_stats_table_dump(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipsla_packet_stats_key_name_dump(p_ipsla_packet_stats, buf));
    for(i=1; i<TBL_IPSLA_PACKET_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_packet_stats_field_name_dump(p_ipsla_packet_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_packet_stats_field_value_dump(p_ipsla_packet_stats, i, buf));
    }
    return str;
}

int32
tbl_ipsla_packet_stats_field_value_parser(char *str, int32 field_id, tbl_ipsla_packet_stats_t *p_ipsla_packet_stats)
{
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_KEY, field_id))
    {
        cdb_ipsla_packet_stats_key_str2val(str, &p_ipsla_packet_stats->key);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->send_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_RTT, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->rtt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TESTID, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->testid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_INDEX, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_STATUS, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD, field_id))
    {
        int32 ret;
        p_ipsla_packet_stats->threshold = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_table_parser(char** array, char* key_value,tbl_ipsla_packet_stats_t *p_ipsla_packet_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_PACKET_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipsla_packet_stats_key_str2val(key_value, p_ipsla_packet_stats));

    for(i=1; i<TBL_IPSLA_PACKET_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipsla_packet_stats_field_value_parser( array[j++], i, p_ipsla_packet_stats));
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_dump_one(tbl_ipsla_packet_stats_t *p_ipsla_packet_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_PACKET_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_KEY].name, 
            cdb_ipsla_packet_stats_key_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_packet_stats->key));
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME].name,
            p_ipsla_packet_stats->send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_RTT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_RTT].name,
            p_ipsla_packet_stats->rtt);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TESTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_TESTID].name,
            p_ipsla_packet_stats->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_INDEX].name,
            p_ipsla_packet_stats->index);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_STATUS].name,
            p_ipsla_packet_stats->status);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT].name,
            p_ipsla_packet_stats->timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD].name,
            p_ipsla_packet_stats->threshold);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsla_packet_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsla_packet_stats_master_t *p_master = _g_p_tbl_ipsla_packet_stats_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ipsla_packet_stats_t *p_db_ipsla_packet_stats = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ipsla_packet_stats_list, p_db_ipsla_packet_stats, listnode, next)
    {
        rc |= fn(p_db_ipsla_packet_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipsla_packet_stats_master_t*
tbl_ipsla_packet_stats_get_master()
{
    return _g_p_tbl_ipsla_packet_stats_master;
}

tbl_ipsla_packet_stats_master_t*
tbl_ipsla_packet_stats_init_ipsla_packet_stats()
{
    _g_p_tbl_ipsla_packet_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipsla_packet_stats_master_t));
    _g_p_tbl_ipsla_packet_stats_master->ipsla_packet_stats_hash = ctclib_hash_create(_tbl_ipsla_packet_stats_hash_make, _tbl_ipsla_packet_stats_hash_cmp);
    _g_p_tbl_ipsla_packet_stats_master->ipsla_packet_stats_list = ctclib_slist_create(tbl_ipsla_packet_stats_cmp, NULL);
    return _g_p_tbl_ipsla_packet_stats_master;
}


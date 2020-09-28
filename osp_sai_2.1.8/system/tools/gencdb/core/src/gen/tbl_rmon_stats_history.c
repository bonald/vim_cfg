
#include "proto.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "cdb_data_cmp.h"

tbl_rmon_stats_history_master_t *_g_p_tbl_rmon_stats_history_master = NULL;

static uint32
_tbl_rmon_stats_history_hash_make(tbl_rmon_stats_history_t *p_rmon_stats_history)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmon_stats_history->key;
    for (index = 0; index < sizeof(p_rmon_stats_history->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rmon_stats_history_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rmon_stats_history_t *p_rmon_stats_history1 = (tbl_rmon_stats_history_t*)p_arg1;
    tbl_rmon_stats_history_t *p_rmon_stats_history2 = (tbl_rmon_stats_history_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmon_stats_history1->key, &p_rmon_stats_history2->key, sizeof(tbl_rmon_stats_history_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_stats_history_add_rmon_stats_history_sync(tbl_rmon_stats_history_t *p_rmon_stats_history, uint32 sync)
{
    tbl_rmon_stats_history_master_t *p_master = _g_p_tbl_rmon_stats_history_master;
    tbl_rmon_stats_history_t *p_db_rmon_stats_history = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rmon_stats_history_get_rmon_stats_history(&p_rmon_stats_history->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmon_stats_history = XCALLOC(MEM_TBL_RMON_STATS_HISTORY, sizeof(tbl_rmon_stats_history_t));
    if (NULL == p_db_rmon_stats_history)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmon_stats_history, p_rmon_stats_history, sizeof(tbl_rmon_stats_history_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rmon_stats_history_hash, (void*)p_db_rmon_stats_history, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rmon_stats_history_list, p_db_rmon_stats_history);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RMON_STATS_HISTORY, p_db_rmon_stats_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RMON_STATS_HISTORY, p_db_rmon_stats_history);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_del_rmon_stats_history_sync(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key, uint32 sync)
{
    tbl_rmon_stats_history_master_t *p_master = _g_p_tbl_rmon_stats_history_master;
    tbl_rmon_stats_history_t *p_db_rmon_stats_history = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_stats_history = tbl_rmon_stats_history_get_rmon_stats_history(p_rmon_stats_history_key);
    if (NULL == p_db_rmon_stats_history)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RMON_STATS_HISTORY, p_db_rmon_stats_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RMON_STATS_HISTORY, p_db_rmon_stats_history);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rmon_stats_history_hash, (void*)p_db_rmon_stats_history);
    ctclib_slistnode_delete(p_master->rmon_stats_history_list, p_db_rmon_stats_history);

    /* 4. free db entry */
    XFREE(MEM_TBL_RMON_STATS_HISTORY, p_db_rmon_stats_history);

    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_set_rmon_stats_history_field_sync(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_rmon_stats_history_field_id_t field_id, uint32 sync)
{
    tbl_rmon_stats_history_t *p_db_rmon_stats_history = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_stats_history = tbl_rmon_stats_history_get_rmon_stats_history(&p_rmon_stats_history->key);
    if (NULL == p_db_rmon_stats_history)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START:
        p_db_rmon_stats_history->interval_start = p_rmon_stats_history->interval_start;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS:
        p_db_rmon_stats_history->drop_events = p_rmon_stats_history->drop_events;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_OCTETS:
        p_db_rmon_stats_history->octets = p_rmon_stats_history->octets;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_PKTS:
        p_db_rmon_stats_history->pkts = p_rmon_stats_history->pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS:
        p_db_rmon_stats_history->broadcast_pkts = p_rmon_stats_history->broadcast_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS:
        p_db_rmon_stats_history->multicast_pkts = p_rmon_stats_history->multicast_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS:
        p_db_rmon_stats_history->crc_align_errors = p_rmon_stats_history->crc_align_errors;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS:
        p_db_rmon_stats_history->undersize_pkts = p_rmon_stats_history->undersize_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS:
        p_db_rmon_stats_history->oversize_pkts = p_rmon_stats_history->oversize_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS:
        p_db_rmon_stats_history->fragments = p_rmon_stats_history->fragments;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_JABBERS:
        p_db_rmon_stats_history->jabbers = p_rmon_stats_history->jabbers;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_COLLISIONS:
        p_db_rmon_stats_history->collisions = p_rmon_stats_history->collisions;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_UTILIZATION:
        p_db_rmon_stats_history->utilization = p_rmon_stats_history->utilization;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS:
        p_db_rmon_stats_history->total_drop_events = p_rmon_stats_history->total_drop_events;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS:
        p_db_rmon_stats_history->total_octets = p_rmon_stats_history->total_octets;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS:
        p_db_rmon_stats_history->total_pkts = p_rmon_stats_history->total_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS:
        p_db_rmon_stats_history->total_broadcast_pkts = p_rmon_stats_history->total_broadcast_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS:
        p_db_rmon_stats_history->total_multicast_pkts = p_rmon_stats_history->total_multicast_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS:
        p_db_rmon_stats_history->total_crc_align_errors = p_rmon_stats_history->total_crc_align_errors;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS:
        p_db_rmon_stats_history->total_undersize_pkts = p_rmon_stats_history->total_undersize_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS:
        p_db_rmon_stats_history->total_oversize_pkts = p_rmon_stats_history->total_oversize_pkts;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS:
        p_db_rmon_stats_history->total_fragments = p_rmon_stats_history->total_fragments;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS:
        p_db_rmon_stats_history->total_jabbers = p_rmon_stats_history->total_jabbers;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS:
        p_db_rmon_stats_history->total_collisions = p_rmon_stats_history->total_collisions;
        break;

    case TBL_RMON_STATS_HISTORY_FLD_MAX:
        sal_memcpy(p_db_rmon_stats_history, p_rmon_stats_history, sizeof(tbl_rmon_stats_history_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RMON_STATS_HISTORY, field_id, p_db_rmon_stats_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RMON_STATS_HISTORY, field_id, p_db_rmon_stats_history);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_add_rmon_stats_history(tbl_rmon_stats_history_t *p_rmon_stats_history)
{
    return tbl_rmon_stats_history_add_rmon_stats_history_sync(p_rmon_stats_history, TRUE);
}

int32
tbl_rmon_stats_history_del_rmon_stats_history(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key)
{
    return tbl_rmon_stats_history_del_rmon_stats_history_sync(p_rmon_stats_history_key, TRUE);
}

int32
tbl_rmon_stats_history_set_rmon_stats_history_field(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_rmon_stats_history_field_id_t field_id)
{
    return tbl_rmon_stats_history_set_rmon_stats_history_field_sync(p_rmon_stats_history, field_id, TRUE);
}

tbl_rmon_stats_history_t*
tbl_rmon_stats_history_get_rmon_stats_history(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key)
{
    tbl_rmon_stats_history_master_t *p_master = _g_p_tbl_rmon_stats_history_master;
    tbl_rmon_stats_history_t lkp;

    sal_memcpy(&lkp.key, p_rmon_stats_history_key, sizeof(tbl_rmon_stats_history_key_t));
    return ctclib_hash_lookup(p_master->rmon_stats_history_hash, &lkp);
}

char*
tbl_rmon_stats_history_key_val2str(tbl_rmon_stats_history_t *p_rmon_stats_history, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_rmon_stats_history_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmon_stats_history->key));
    return str;
}

int32
tbl_rmon_stats_history_key_str2val(char *str, tbl_rmon_stats_history_t *p_rmon_stats_history)
{
    int32 ret = 0;

    ret = cdb_rmon_stats_history_key_str2val(str, &p_rmon_stats_history->key);
    return ret;
}

char*
tbl_rmon_stats_history_key_name_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_STATS_HISTORY);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_rmon_stats_history_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmon_stats_history->key));
    return str;
}

char*
tbl_rmon_stats_history_key_value_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char *str)
{

    cdb_rmon_stats_history_key_val2str(str, MAX_CMD_STR_LEN, &p_rmon_stats_history->key);
    return str;
}

char*
tbl_rmon_stats_history_field_name_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_STATS_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RMON_STATS_HISTORY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rmon_stats_history_field_value_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_KEY, field_id))
    {
        cdb_rmon_stats_history_key_val2str(str, MAX_CMD_STR_LEN, &p_rmon_stats_history->key);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->interval_start);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->drop_events);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->octets);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->fragments);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_JABBERS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->jabbers);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_COLLISIONS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->collisions);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UTILIZATION, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_stats_history->utilization);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_drop_events);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_octets);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_fragments);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_jabbers);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_stats_history->total_collisions);
    }
    return str;
}

char**
tbl_rmon_stats_history_table_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rmon_stats_history_key_name_dump(p_rmon_stats_history, buf));
    for(i=1; i<TBL_RMON_STATS_HISTORY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rmon_stats_history_field_name_dump(p_rmon_stats_history, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rmon_stats_history_field_value_dump(p_rmon_stats_history, i, buf));
    }
    return str;
}

int32
tbl_rmon_stats_history_field_value_parser(char *str, int32 field_id, tbl_rmon_stats_history_t *p_rmon_stats_history)
{
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_KEY, field_id))
    {
        cdb_rmon_stats_history_key_str2val(str, &p_rmon_stats_history->key);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START, field_id))
    {
        int32 ret;
        p_rmon_stats_history->interval_start = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->drop_events = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->broadcast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->multicast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->crc_align_errors = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->undersize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->oversize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->fragments = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_JABBERS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->jabbers = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_COLLISIONS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->collisions = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UTILIZATION, field_id))
    {
        int32 ret;
        p_rmon_stats_history->utilization = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_drop_events = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_broadcast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_multicast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_crc_align_errors = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_undersize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_oversize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_fragments = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_jabbers = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS, field_id))
    {
        int32 ret;
        p_rmon_stats_history->total_collisions = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_table_parser(char** array, char* key_value,tbl_rmon_stats_history_t *p_rmon_stats_history)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_STATS_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rmon_stats_history_key_str2val(key_value, p_rmon_stats_history));

    for(i=1; i<TBL_RMON_STATS_HISTORY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rmon_stats_history_field_value_parser( array[j++], i, p_rmon_stats_history));
    }

    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_dump_one(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_STATS_HISTORY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_KEY].name, 
            cdb_rmon_stats_history_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmon_stats_history->key));
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START].name,
            p_rmon_stats_history->interval_start);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS].name,
            p_rmon_stats_history->drop_events);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_OCTETS].name,
            p_rmon_stats_history->octets);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_PKTS].name,
            p_rmon_stats_history->pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS].name,
            p_rmon_stats_history->broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS].name,
            p_rmon_stats_history->multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS].name,
            p_rmon_stats_history->crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS].name,
            p_rmon_stats_history->undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS].name,
            p_rmon_stats_history->oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS].name,
            p_rmon_stats_history->fragments);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_JABBERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_JABBERS].name,
            p_rmon_stats_history->jabbers);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_COLLISIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_COLLISIONS].name,
            p_rmon_stats_history->collisions);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_UTILIZATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_UTILIZATION].name,
            p_rmon_stats_history->utilization);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS].name,
            p_rmon_stats_history->total_drop_events);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS].name,
            p_rmon_stats_history->total_octets);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS].name,
            p_rmon_stats_history->total_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS].name,
            p_rmon_stats_history->total_broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS].name,
            p_rmon_stats_history->total_multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS].name,
            p_rmon_stats_history->total_crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS].name,
            p_rmon_stats_history->total_undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS].name,
            p_rmon_stats_history->total_oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS].name,
            p_rmon_stats_history->total_fragments);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS].name,
            p_rmon_stats_history->total_jabbers);
    }
    if (FLD_MATCH(TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS].name,
            p_rmon_stats_history->total_collisions);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rmon_stats_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rmon_stats_history_master_t *p_master = _g_p_tbl_rmon_stats_history_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_stats_history_t *p_db_rmon_stats_history = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_stats_history_list, p_db_rmon_stats_history, listnode, next)
    {
        rc |= fn(p_db_rmon_stats_history, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rmon_stats_history_master_t*
tbl_rmon_stats_history_get_master()
{
    return _g_p_tbl_rmon_stats_history_master;
}

tbl_rmon_stats_history_master_t*
tbl_rmon_stats_history_init_rmon_stats_history()
{
    _g_p_tbl_rmon_stats_history_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rmon_stats_history_master_t));
    _g_p_tbl_rmon_stats_history_master->rmon_stats_history_hash = ctclib_hash_create(_tbl_rmon_stats_history_hash_make, _tbl_rmon_stats_history_hash_cmp);
    _g_p_tbl_rmon_stats_history_master->rmon_stats_history_list = ctclib_slist_create(tbl_rmon_stats_history_cmp, NULL);
    return _g_p_tbl_rmon_stats_history_master;
}


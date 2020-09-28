
#include "proto.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "cdb_data_cmp.h"

tbl_rmon_ether_stats_master_t *_g_p_tbl_rmon_ether_stats_master = NULL;

static uint32
_tbl_rmon_ether_stats_hash_make(tbl_rmon_ether_stats_t *p_rmon_ether_stats)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmon_ether_stats->key;
    for (index = 0; index < sizeof(p_rmon_ether_stats->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rmon_ether_stats_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rmon_ether_stats_t *p_rmon_ether_stats1 = (tbl_rmon_ether_stats_t*)p_arg1;
    tbl_rmon_ether_stats_t *p_rmon_ether_stats2 = (tbl_rmon_ether_stats_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmon_ether_stats1->key, &p_rmon_ether_stats2->key, sizeof(tbl_rmon_ether_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_ether_stats_add_rmon_ether_stats_sync(tbl_rmon_ether_stats_t *p_rmon_ether_stats, uint32 sync)
{
    tbl_rmon_ether_stats_master_t *p_master = _g_p_tbl_rmon_ether_stats_master;
    tbl_rmon_ether_stats_t *p_db_rmon_ether_stats = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rmon_ether_stats_get_rmon_ether_stats(&p_rmon_ether_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmon_ether_stats = XCALLOC(MEM_TBL_RMON_ETHER_STATS, sizeof(tbl_rmon_ether_stats_t));
    if (NULL == p_db_rmon_ether_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmon_ether_stats, p_rmon_ether_stats, sizeof(tbl_rmon_ether_stats_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rmon_ether_stats_hash, (void*)p_db_rmon_ether_stats, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rmon_ether_stats_list, p_db_rmon_ether_stats);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RMON_ETHER_STATS, p_db_rmon_ether_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RMON_ETHER_STATS, p_db_rmon_ether_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_del_rmon_ether_stats_sync(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key, uint32 sync)
{
    tbl_rmon_ether_stats_master_t *p_master = _g_p_tbl_rmon_ether_stats_master;
    tbl_rmon_ether_stats_t *p_db_rmon_ether_stats = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_ether_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(p_rmon_ether_stats_key);
    if (NULL == p_db_rmon_ether_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RMON_ETHER_STATS, p_db_rmon_ether_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RMON_ETHER_STATS, p_db_rmon_ether_stats);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rmon_ether_stats_hash, (void*)p_db_rmon_ether_stats);
    ctclib_slistnode_delete(p_master->rmon_ether_stats_list, p_db_rmon_ether_stats);

    /* 4. free db entry */
    XFREE(MEM_TBL_RMON_ETHER_STATS, p_db_rmon_ether_stats);

    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_set_rmon_ether_stats_field_sync(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_rmon_ether_stats_field_id_t field_id, uint32 sync)
{
    tbl_rmon_ether_stats_t *p_db_rmon_ether_stats = NULL;

    /* 1. lookup entry exist */
    p_db_rmon_ether_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&p_rmon_ether_stats->key);
    if (NULL == p_db_rmon_ether_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RMON_ETHER_STATS_FLD_STATUS:
        p_db_rmon_ether_stats->status = p_rmon_ether_stats->status;
        break;

    case TBL_RMON_ETHER_STATS_FLD_OWNER:
        sal_memcpy(p_db_rmon_ether_stats->owner, p_rmon_ether_stats->owner, sizeof(p_rmon_ether_stats->owner));
        break;

    case TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE:
        sal_memcpy(p_db_rmon_ether_stats->data_source, p_rmon_ether_stats->data_source, sizeof(p_rmon_ether_stats->data_source));
        break;

    case TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX:
        p_db_rmon_ether_stats->data_source_ifindex = p_rmon_ether_stats->data_source_ifindex;
        break;

    case TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS:
        p_db_rmon_ether_stats->drop_events = p_rmon_ether_stats->drop_events;
        break;

    case TBL_RMON_ETHER_STATS_FLD_OCTETS:
        p_db_rmon_ether_stats->octets = p_rmon_ether_stats->octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS:
        p_db_rmon_ether_stats->pkts = p_rmon_ether_stats->pkts;
        break;

    case TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS:
        p_db_rmon_ether_stats->broadcast_pkts = p_rmon_ether_stats->broadcast_pkts;
        break;

    case TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS:
        p_db_rmon_ether_stats->multicast_pkts = p_rmon_ether_stats->multicast_pkts;
        break;

    case TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS:
        p_db_rmon_ether_stats->crc_align_errors = p_rmon_ether_stats->crc_align_errors;
        break;

    case TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS:
        p_db_rmon_ether_stats->undersize_pkts = p_rmon_ether_stats->undersize_pkts;
        break;

    case TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS:
        p_db_rmon_ether_stats->oversize_pkts = p_rmon_ether_stats->oversize_pkts;
        break;

    case TBL_RMON_ETHER_STATS_FLD_FRAGMENTS:
        p_db_rmon_ether_stats->fragments = p_rmon_ether_stats->fragments;
        break;

    case TBL_RMON_ETHER_STATS_FLD_JABBERS:
        p_db_rmon_ether_stats->jabbers = p_rmon_ether_stats->jabbers;
        break;

    case TBL_RMON_ETHER_STATS_FLD_COLLISIONS:
        p_db_rmon_ether_stats->collisions = p_rmon_ether_stats->collisions;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS:
        p_db_rmon_ether_stats->pkts_64_octets = p_rmon_ether_stats->pkts_64_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS:
        p_db_rmon_ether_stats->pkts_65to127_octets = p_rmon_ether_stats->pkts_65to127_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS:
        p_db_rmon_ether_stats->pkts_128to255_octets = p_rmon_ether_stats->pkts_128to255_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS:
        p_db_rmon_ether_stats->pkts_256to511_octets = p_rmon_ether_stats->pkts_256to511_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS:
        p_db_rmon_ether_stats->pkts_512to1023_octets = p_rmon_ether_stats->pkts_512to1023_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS:
        p_db_rmon_ether_stats->pkts_1024to1518_octets = p_rmon_ether_stats->pkts_1024to1518_octets;
        break;

    case TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER:
        p_db_rmon_ether_stats->t_under_creation_timer = p_rmon_ether_stats->t_under_creation_timer;
        break;

    case TBL_RMON_ETHER_STATS_FLD_MAX:
        sal_memcpy(p_db_rmon_ether_stats, p_rmon_ether_stats, sizeof(tbl_rmon_ether_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RMON_ETHER_STATS, field_id, p_db_rmon_ether_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RMON_ETHER_STATS, field_id, p_db_rmon_ether_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_add_rmon_ether_stats(tbl_rmon_ether_stats_t *p_rmon_ether_stats)
{
    return tbl_rmon_ether_stats_add_rmon_ether_stats_sync(p_rmon_ether_stats, TRUE);
}

int32
tbl_rmon_ether_stats_del_rmon_ether_stats(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key)
{
    return tbl_rmon_ether_stats_del_rmon_ether_stats_sync(p_rmon_ether_stats_key, TRUE);
}

int32
tbl_rmon_ether_stats_set_rmon_ether_stats_field(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_rmon_ether_stats_field_id_t field_id)
{
    return tbl_rmon_ether_stats_set_rmon_ether_stats_field_sync(p_rmon_ether_stats, field_id, TRUE);
}

tbl_rmon_ether_stats_t*
tbl_rmon_ether_stats_get_rmon_ether_stats(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key)
{
    tbl_rmon_ether_stats_master_t *p_master = _g_p_tbl_rmon_ether_stats_master;
    tbl_rmon_ether_stats_t lkp;

    sal_memcpy(&lkp.key, p_rmon_ether_stats_key, sizeof(tbl_rmon_ether_stats_key_t));
    return ctclib_hash_lookup(p_master->rmon_ether_stats_hash, &lkp);
}

char*
tbl_rmon_ether_stats_key_val2str(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rmon_ether_stats->key.index);
    return str;
}

int32
tbl_rmon_ether_stats_key_str2val(char *str, tbl_rmon_ether_stats_t *p_rmon_ether_stats)
{
    int32 ret = 0;

    p_rmon_ether_stats->key.index = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rmon_ether_stats_key_name_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ETHER_STATS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rmon_ether_stats->key.index);
    return str;
}

char*
tbl_rmon_ether_stats_key_value_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char *str)
{
    sal_sprintf(str, "%u", p_rmon_ether_stats->key.index);
    return str;
}

char*
tbl_rmon_ether_stats_field_name_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ETHER_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RMON_ETHER_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rmon_ether_stats_field_value_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_ether_stats->key.index);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_ether_stats->status);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OWNER, field_id))
    {
        sal_sprintf(str, "%s", p_rmon_ether_stats->owner);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_rmon_ether_stats->data_source, sizeof(p_rmon_ether_stats->data_source)/4);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_rmon_ether_stats->data_source_ifindex);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->drop_events);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_FRAGMENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->fragments);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_JABBERS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->jabbers);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_COLLISIONS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->collisions);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_64_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_65to127_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_128to255_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_256to511_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_512to1023_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rmon_ether_stats->pkts_1024to1518_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rmon_ether_stats->t_under_creation_timer);
    }
    return str;
}

char**
tbl_rmon_ether_stats_table_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rmon_ether_stats_key_name_dump(p_rmon_ether_stats, buf));
    for(i=1; i<TBL_RMON_ETHER_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rmon_ether_stats_field_name_dump(p_rmon_ether_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rmon_ether_stats_field_value_dump(p_rmon_ether_stats, i, buf));
    }
    return str;
}

int32
tbl_rmon_ether_stats_field_value_parser(char *str, int32 field_id, tbl_rmon_ether_stats_t *p_rmon_ether_stats)
{
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_KEY, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->key.index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_STATUS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OWNER, field_id))
    {
        sal_strcpy(p_rmon_ether_stats->owner, str);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE, field_id))
    {
        cdb_uint32_array_str2val(str, p_rmon_ether_stats->data_source);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->data_source_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->drop_events = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->broadcast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->multicast_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->crc_align_errors = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->undersize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->oversize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_FRAGMENTS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->fragments = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_JABBERS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->jabbers = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_COLLISIONS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->collisions = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_64_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_65to127_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_128to255_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_256to511_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_512to1023_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS, field_id))
    {
        int32 ret;
        p_rmon_ether_stats->pkts_1024to1518_octets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rmon_ether_stats->t_under_creation_timer);
    }
    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_table_parser(char** array, char* key_value,tbl_rmon_ether_stats_t *p_rmon_ether_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ETHER_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rmon_ether_stats_key_str2val(key_value, p_rmon_ether_stats));

    for(i=1; i<TBL_RMON_ETHER_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rmon_ether_stats_field_value_parser( array[j++], i, p_rmon_ether_stats));
    }

    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_dump_one(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RMON_ETHER_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_KEY].name,
            p_rmon_ether_stats->key.index);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_STATUS].name,
            p_rmon_ether_stats->status);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_OWNER].name,
            p_rmon_ether_stats->owner);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_rmon_ether_stats->data_source, sizeof(p_rmon_ether_stats->data_source)/4));
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX].name,
            p_rmon_ether_stats->data_source_ifindex);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS].name,
            p_rmon_ether_stats->drop_events);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_OCTETS].name,
            p_rmon_ether_stats->octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS].name,
            p_rmon_ether_stats->pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS].name,
            p_rmon_ether_stats->broadcast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS].name,
            p_rmon_ether_stats->multicast_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS].name,
            p_rmon_ether_stats->crc_align_errors);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS].name,
            p_rmon_ether_stats->undersize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS].name,
            p_rmon_ether_stats->oversize_pkts);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_FRAGMENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_FRAGMENTS].name,
            p_rmon_ether_stats->fragments);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_JABBERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_JABBERS].name,
            p_rmon_ether_stats->jabbers);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_COLLISIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_COLLISIONS].name,
            p_rmon_ether_stats->collisions);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS].name,
            p_rmon_ether_stats->pkts_64_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS].name,
            p_rmon_ether_stats->pkts_65to127_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS].name,
            p_rmon_ether_stats->pkts_128to255_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS].name,
            p_rmon_ether_stats->pkts_256to511_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS].name,
            p_rmon_ether_stats->pkts_512to1023_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS].name,
            p_rmon_ether_stats->pkts_1024to1518_octets);
    }
    if (FLD_MATCH(TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rmon_ether_stats->t_under_creation_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rmon_ether_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rmon_ether_stats_master_t *p_master = _g_p_tbl_rmon_ether_stats_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_ether_stats_t *p_db_rmon_ether_stats = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rmon_ether_stats_list, p_db_rmon_ether_stats, listnode, next)
    {
        rc |= fn(p_db_rmon_ether_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rmon_ether_stats_master_t*
tbl_rmon_ether_stats_get_master()
{
    return _g_p_tbl_rmon_ether_stats_master;
}

tbl_rmon_ether_stats_master_t*
tbl_rmon_ether_stats_init_rmon_ether_stats()
{
    _g_p_tbl_rmon_ether_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rmon_ether_stats_master_t));
    _g_p_tbl_rmon_ether_stats_master->rmon_ether_stats_hash = ctclib_hash_create(_tbl_rmon_ether_stats_hash_make, _tbl_rmon_ether_stats_hash_cmp);
    _g_p_tbl_rmon_ether_stats_master->rmon_ether_stats_list = ctclib_slist_create(tbl_rmon_ether_stats_cmp, NULL);
    return _g_p_tbl_rmon_ether_stats_master;
}


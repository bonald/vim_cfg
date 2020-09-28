
#include "proto.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "cdb_data_cmp.h"

tbl_port_stats_master_t *_g_p_tbl_port_stats_master = NULL;

static uint32
_tbl_port_stats_hash_make(tbl_port_stats_t *p_port_stats)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_port_stats->key;
    for (index = 0; index < sizeof(p_port_stats->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_port_stats_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_port_stats_t *p_port_stats1 = (tbl_port_stats_t*)p_arg1;
    tbl_port_stats_t *p_port_stats2 = (tbl_port_stats_t*)p_arg2;

    if (0 == sal_memcmp(&p_port_stats1->key, &p_port_stats2->key, sizeof(tbl_port_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_port_stats_add_port_stats_sync(tbl_port_stats_t *p_port_stats, uint32 sync)
{
    tbl_port_stats_master_t *p_master = _g_p_tbl_port_stats_master;
    tbl_port_stats_t *p_db_port_stats = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_port_stats_get_port_stats(&p_port_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port_stats = XCALLOC(MEM_TBL_PORT_STATS, sizeof(tbl_port_stats_t));
    if (NULL == p_db_port_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port_stats, p_port_stats, sizeof(tbl_port_stats_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->port_stats_hash, (void*)p_db_port_stats, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->port_stats_list, p_db_port_stats);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PORT_STATS, p_db_port_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PORT_STATS, p_db_port_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_del_port_stats_sync(tbl_port_stats_key_t *p_port_stats_key, uint32 sync)
{
    tbl_port_stats_master_t *p_master = _g_p_tbl_port_stats_master;
    tbl_port_stats_t *p_db_port_stats = NULL;

    /* 1. lookup entry exist */
    p_db_port_stats = tbl_port_stats_get_port_stats(p_port_stats_key);
    if (NULL == p_db_port_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PORT_STATS, p_db_port_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PORT_STATS, p_db_port_stats);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->port_stats_hash, (void*)p_db_port_stats);
    ctclib_slistnode_delete(p_master->port_stats_list, p_db_port_stats);

    /* 4. free db entry */
    XFREE(MEM_TBL_PORT_STATS, p_db_port_stats);

    return PM_E_NONE;
}

int32
tbl_port_stats_set_port_stats_field_sync(tbl_port_stats_t *p_port_stats, tbl_port_stats_field_id_t field_id, uint32 sync)
{
    tbl_port_stats_t *p_db_port_stats = NULL;

    /* 1. lookup entry exist */
    p_db_port_stats = tbl_port_stats_get_port_stats(&p_port_stats->key);
    if (NULL == p_db_port_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PORT_STATS_FLD_PORT_ID:
        p_db_port_stats->port_id = p_port_stats->port_id;
        break;

    case TBL_PORT_STATS_FLD_OCTETS_RCV:
        p_db_port_stats->octets_rcv = p_port_stats->octets_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_RCV:
        p_db_port_stats->pkts_rcv = p_port_stats->pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_RCV:
        p_db_port_stats->uc_pkts_rcv = p_port_stats->uc_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_BRDC_PKTS_RCV:
        p_db_port_stats->brdc_pkts_rcv = p_port_stats->brdc_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_MC_PKTS_RCV:
        p_db_port_stats->mc_pkts_rcv = p_port_stats->mc_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_OAM_PKTS_RCV:
        p_db_port_stats->oam_pkts_rcv = p_port_stats->oam_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_UNDERSIZE_PKTS:
        p_db_port_stats->undersize_pkts = p_port_stats->undersize_pkts;
        break;

    case TBL_PORT_STATS_FLD_OVERSIZE_PKTS:
        p_db_port_stats->oversize_pkts = p_port_stats->oversize_pkts;
        break;

    case TBL_PORT_STATS_FLD_JABBER_PKTS:
        p_db_port_stats->jabber_pkts = p_port_stats->jabber_pkts;
        break;

    case TBL_PORT_STATS_FLD_MAC_RCV_ERROR:
        p_db_port_stats->mac_rcv_error = p_port_stats->mac_rcv_error;
        break;

    case TBL_PORT_STATS_FLD_BAD_CRC:
        p_db_port_stats->bad_crc = p_port_stats->bad_crc;
        break;

    case TBL_PORT_STATS_FLD_FRAME_ERROR:
        p_db_port_stats->frame_error = p_port_stats->frame_error;
        break;

    case TBL_PORT_STATS_FLD_DROP_EVENTS:
        p_db_port_stats->drop_events = p_port_stats->drop_events;
        break;

    case TBL_PORT_STATS_FLD_PAUSE_RCV:
        p_db_port_stats->pause_rcv = p_port_stats->pause_rcv;
        break;

    case TBL_PORT_STATS_FLD_OCTETS_SEND:
        p_db_port_stats->octets_send = p_port_stats->octets_send;
        break;

    case TBL_PORT_STATS_FLD_PKTS_SEND:
        p_db_port_stats->pkts_send = p_port_stats->pkts_send;
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_SEND:
        p_db_port_stats->uc_pkts_send = p_port_stats->uc_pkts_send;
        break;

    case TBL_PORT_STATS_FLD_BRDC_PKTS_SEND:
        p_db_port_stats->brdc_pkts_send = p_port_stats->brdc_pkts_send;
        break;

    case TBL_PORT_STATS_FLD_MC_PKTS_SEND:
        p_db_port_stats->mc_pkts_send = p_port_stats->mc_pkts_send;
        break;

    case TBL_PORT_STATS_FLD_OAM_PKTS_SEND:
        p_db_port_stats->oam_pkts_send = p_port_stats->oam_pkts_send;
        break;

    case TBL_PORT_STATS_FLD_UNDERRUNS:
        p_db_port_stats->underruns = p_port_stats->underruns;
        break;

    case TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR:
        p_db_port_stats->mac_transmit_err = p_port_stats->mac_transmit_err;
        break;

    case TBL_PORT_STATS_FLD_PAUSE_SEND:
        p_db_port_stats->pause_send = p_port_stats->pause_send;
        break;

    case TBL_PORT_STATS_FLD_FCS_PKTS_RCV:
        p_db_port_stats->fcs_pkts_rcv = p_port_stats->fcs_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_FCS_OCTETS_RCV:
        p_db_port_stats->fcs_octets_rcv = p_port_stats->fcs_octets_rcv;
        break;

    case TBL_PORT_STATS_FLD_FCS_PKTS_SEND:
        p_db_port_stats->fcs_pkts_send = p_port_stats->fcs_pkts_send;
        break;

    case TBL_PORT_STATS_FLD_FCS_OCTETS_SEND:
        p_db_port_stats->fcs_octets_send = p_port_stats->fcs_octets_send;
        break;

    case TBL_PORT_STATS_FLD_FRAGMENTS_PKTS:
        p_db_port_stats->fragments_pkts = p_port_stats->fragments_pkts;
        break;

    case TBL_PORT_STATS_FLD_BAD_PKTS_RCV:
        p_db_port_stats->bad_pkts_rcv = p_port_stats->bad_pkts_rcv;
        break;

    case TBL_PORT_STATS_FLD_BAD_OCTETS_RCV:
        p_db_port_stats->bad_octets_rcv = p_port_stats->bad_octets_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_64_RCV:
        p_db_port_stats->pkts_64_rcv = p_port_stats->pkts_64_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV:
        p_db_port_stats->pkts_65_to_127_rcv = p_port_stats->pkts_65_to_127_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV:
        p_db_port_stats->pkts_128_to_255_rcv = p_port_stats->pkts_128_to_255_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV:
        p_db_port_stats->pkts_256_to_511_rcv = p_port_stats->pkts_256_to_511_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV:
        p_db_port_stats->pkts_512_to_1023_rcv = p_port_stats->pkts_512_to_1023_rcv;
        break;

    case TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV:
        p_db_port_stats->pkts_1024_to_1518_rcv = p_port_stats->pkts_1024_to_1518_rcv;
        break;

    case TBL_PORT_STATS_FLD_LAST_CLEAR_TS:
        p_db_port_stats->last_clear_ts = p_port_stats->last_clear_ts;
        break;

    case TBL_PORT_STATS_FLD_MAX:
        sal_memcpy(p_db_port_stats, p_port_stats, sizeof(tbl_port_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PORT_STATS, field_id, p_db_port_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PORT_STATS, field_id, p_db_port_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_add_port_stats(tbl_port_stats_t *p_port_stats)
{
    return tbl_port_stats_add_port_stats_sync(p_port_stats, TRUE);
}

int32
tbl_port_stats_del_port_stats(tbl_port_stats_key_t *p_port_stats_key)
{
    return tbl_port_stats_del_port_stats_sync(p_port_stats_key, TRUE);
}

int32
tbl_port_stats_set_port_stats_field(tbl_port_stats_t *p_port_stats, tbl_port_stats_field_id_t field_id)
{
    return tbl_port_stats_set_port_stats_field_sync(p_port_stats, field_id, TRUE);
}

tbl_port_stats_t*
tbl_port_stats_get_port_stats(tbl_port_stats_key_t *p_port_stats_key)
{
    tbl_port_stats_master_t *p_master = _g_p_tbl_port_stats_master;
    tbl_port_stats_t lkp;

    sal_memcpy(&lkp.key, p_port_stats_key, sizeof(tbl_port_stats_key_t));
    return ctclib_hash_lookup(p_master->port_stats_hash, &lkp);
}

char*
tbl_port_stats_key_val2str(tbl_port_stats_t *p_port_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_port_stats->key.ifindex);
    return str;
}

int32
tbl_port_stats_key_str2val(char *str, tbl_port_stats_t *p_port_stats)
{
    int32 ret = 0;

    p_port_stats->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_port_stats_key_name_dump(tbl_port_stats_t *p_port_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_port_stats->key.ifindex);
    return str;
}

char*
tbl_port_stats_key_value_dump(tbl_port_stats_t *p_port_stats, char *str)
{
    sal_sprintf(str, "%u", p_port_stats->key.ifindex);
    return str;
}

char*
tbl_port_stats_field_name_dump(tbl_port_stats_t *p_port_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PORT_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_port_stats_field_value_dump(tbl_port_stats_t *p_port_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PORT_STATS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats->key.ifindex);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PORT_ID, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats->port_id);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->uc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->brdc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->mc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->oam_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->undersize_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OVERSIZE_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->oversize_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_JABBER_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->jabber_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_RCV_ERROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->mac_rcv_error);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_CRC, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->bad_crc);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAME_ERROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->frame_error);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_DROP_EVENTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->drop_events);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pause_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->octets_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->uc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->brdc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->mc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->oam_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERRUNS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->underruns);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->mac_transmit_err);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pause_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->fcs_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->fcs_octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->fcs_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_SEND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->fcs_octets_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAGMENTS_PKTS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->fragments_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_PKTS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->bad_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_OCTETS_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->bad_octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_64_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_64_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_65_to_127_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_128_to_255_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_256_to_511_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_512_to_1023_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats->pkts_1024_to_1518_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_LAST_CLEAR_TS, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats->last_clear_ts);
    }
    return str;
}

char**
tbl_port_stats_table_dump(tbl_port_stats_t *p_port_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_port_stats_key_name_dump(p_port_stats, buf));
    for(i=1; i<TBL_PORT_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_port_stats_field_name_dump(p_port_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_port_stats_field_value_dump(p_port_stats, i, buf));
    }
    return str;
}

int32
tbl_port_stats_field_value_parser(char *str, int32 field_id, tbl_port_stats_t *p_port_stats)
{
    if (FLD_MATCH(TBL_PORT_STATS_FLD_KEY, field_id))
    {
        int32 ret;
        p_port_stats->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PORT_ID, field_id))
    {
        int32 ret;
        p_port_stats->port_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->octets_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->uc_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->brdc_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->mc_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->oam_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_port_stats->undersize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OVERSIZE_PKTS, field_id))
    {
        int32 ret;
        p_port_stats->oversize_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_JABBER_PKTS, field_id))
    {
        int32 ret;
        p_port_stats->jabber_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_RCV_ERROR, field_id))
    {
        int32 ret;
        p_port_stats->mac_rcv_error = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_CRC, field_id))
    {
        int32 ret;
        p_port_stats->bad_crc = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAME_ERROR, field_id))
    {
        int32 ret;
        p_port_stats->frame_error = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_DROP_EVENTS, field_id))
    {
        int32 ret;
        p_port_stats->drop_events = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pause_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->octets_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->uc_pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->brdc_pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->mc_pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->oam_pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERRUNS, field_id))
    {
        int32 ret;
        p_port_stats->underruns = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR, field_id))
    {
        int32 ret;
        p_port_stats->mac_transmit_err = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_SEND, field_id))
    {
        int32 ret;
        p_port_stats->pause_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->fcs_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->fcs_octets_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->fcs_pkts_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_SEND, field_id))
    {
        int32 ret;
        p_port_stats->fcs_octets_send = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAGMENTS_PKTS, field_id))
    {
        int32 ret;
        p_port_stats->fragments_pkts = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_PKTS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->bad_pkts_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_OCTETS_RCV, field_id))
    {
        int32 ret;
        p_port_stats->bad_octets_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_64_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_64_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_65_to_127_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_128_to_255_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_256_to_511_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_512_to_1023_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV, field_id))
    {
        int32 ret;
        p_port_stats->pkts_1024_to_1518_rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_LAST_CLEAR_TS, field_id))
    {
        int32 ret;
        p_port_stats->last_clear_ts = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_port_stats_table_parser(char** array, char* key_value,tbl_port_stats_t *p_port_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_port_stats_key_str2val(key_value, p_port_stats));

    for(i=1; i<TBL_PORT_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_port_stats_field_value_parser( array[j++], i, p_port_stats));
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_dump_one(tbl_port_stats_t *p_port_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PORT_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_FLD_KEY].name,
            p_port_stats->key.ifindex);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PORT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_FLD_PORT_ID].name,
            p_port_stats->port_id);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_OCTETS_RCV].name,
            p_port_stats->octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_RCV].name,
            p_port_stats->pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_UC_PKTS_RCV].name,
            p_port_stats->uc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_BRDC_PKTS_RCV].name,
            p_port_stats->brdc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_MC_PKTS_RCV].name,
            p_port_stats->mc_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_OAM_PKTS_RCV].name,
            p_port_stats->oam_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_UNDERSIZE_PKTS].name,
            p_port_stats->undersize_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OVERSIZE_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_OVERSIZE_PKTS].name,
            p_port_stats->oversize_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_JABBER_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_JABBER_PKTS].name,
            p_port_stats->jabber_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_RCV_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_MAC_RCV_ERROR].name,
            p_port_stats->mac_rcv_error);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_CRC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_BAD_CRC].name,
            p_port_stats->bad_crc);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAME_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FRAME_ERROR].name,
            p_port_stats->frame_error);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_DROP_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_DROP_EVENTS].name,
            p_port_stats->drop_events);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PAUSE_RCV].name,
            p_port_stats->pause_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OCTETS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_OCTETS_SEND].name,
            p_port_stats->octets_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_SEND].name,
            p_port_stats->pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UC_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_UC_PKTS_SEND].name,
            p_port_stats->uc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BRDC_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_BRDC_PKTS_SEND].name,
            p_port_stats->brdc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MC_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_MC_PKTS_SEND].name,
            p_port_stats->mc_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_OAM_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_OAM_PKTS_SEND].name,
            p_port_stats->oam_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_UNDERRUNS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_UNDERRUNS].name,
            p_port_stats->underruns);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR].name,
            p_port_stats->mac_transmit_err);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PAUSE_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PAUSE_SEND].name,
            p_port_stats->pause_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FCS_PKTS_RCV].name,
            p_port_stats->fcs_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FCS_OCTETS_RCV].name,
            p_port_stats->fcs_octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_PKTS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FCS_PKTS_SEND].name,
            p_port_stats->fcs_pkts_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FCS_OCTETS_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FCS_OCTETS_SEND].name,
            p_port_stats->fcs_octets_send);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_FRAGMENTS_PKTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_FRAGMENTS_PKTS].name,
            p_port_stats->fragments_pkts);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_PKTS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_BAD_PKTS_RCV].name,
            p_port_stats->bad_pkts_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_BAD_OCTETS_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_BAD_OCTETS_RCV].name,
            p_port_stats->bad_octets_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_64_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_64_RCV].name,
            p_port_stats->pkts_64_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV].name,
            p_port_stats->pkts_65_to_127_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV].name,
            p_port_stats->pkts_128_to_255_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV].name,
            p_port_stats->pkts_256_to_511_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV].name,
            p_port_stats->pkts_512_to_1023_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV].name,
            p_port_stats->pkts_1024_to_1518_rcv);
    }
    if (FLD_MATCH(TBL_PORT_STATS_FLD_LAST_CLEAR_TS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_FLD_LAST_CLEAR_TS].name,
            p_port_stats->last_clear_ts);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_port_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_port_stats_master_t *p_master = _g_p_tbl_port_stats_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_port_stats_t *p_db_port_stats = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_stats_list, p_db_port_stats, listnode, next)
    {
        rc |= fn(p_db_port_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_port_stats_master_t*
tbl_port_stats_get_master()
{
    return _g_p_tbl_port_stats_master;
}

tbl_port_stats_master_t*
tbl_port_stats_init_port_stats()
{
    _g_p_tbl_port_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_port_stats_master_t));
    _g_p_tbl_port_stats_master->port_stats_hash = ctclib_hash_create(_tbl_port_stats_hash_make, _tbl_port_stats_hash_cmp);
    _g_p_tbl_port_stats_master->port_stats_list = ctclib_slist_create(tbl_port_stats_cmp, NULL);
    return _g_p_tbl_port_stats_master;
}


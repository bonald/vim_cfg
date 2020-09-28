
#include "snmpagt_define.h"
#include "snmpagt.h"

snmpagt_row_t RMON_ETHER_STATS_ROW[] = 
{
    {TBL_RMON_ETHER_STATS_INDEX                , ASN_INTEGER,    0},
    {TBL_RMON_ETHER_STATS_DATASOURCE           , ASN_OBJECT_ID,  0},
    {TBL_RMON_ETHER_STATS_DROPEVENTS           , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_OCTETS               , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS                 , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_BROADCASTPKTS        , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_MULTICASTPKTS        , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_CRCALIGNERRORS       , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_UNDERSIZEPKTS        , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_OVERSIZEPKTS         , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_FRAGMENTS            , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_JABBERS              , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_COLLISIONS           , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS64OCTETS         , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS65TO127OCTETS    , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS128TO255OCTETS   , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS256TO511OCTETS   , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS512TO1023OCTETS  , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_PKTS1024TO1518OCTETS , ASN_COUNTER,    0},
    {TBL_RMON_ETHER_STATS_OWNER                , ASN_OCTET_STR,  0},
    {TBL_RMON_ETHER_STATS_STATUS               , ASN_INTEGER,    0},
};

snmpagt_row_t RMON_HISTORY_ROW[] = 
{
    {TBL_RMON_HISTORY_INDEX             , ASN_INTEGER       , 0},
    {TBL_RMON_HISTORY_DATASOURCE        , ASN_OBJECT_ID     , 0},
    {TBL_RMON_HISTORY_BUCKETSREQUESTED  , ASN_INTEGER       , 0},
    {TBL_RMON_HISTORY_BUCKETSGRANTED    , ASN_INTEGER       , 0},
    {TBL_RMON_HISTORY_INTERVAL          , ASN_INTEGER       , 0},
    {TBL_RMON_HISTORY_OWNER             , ASN_OCTET_STR     , 0},
    {TBL_RMON_HISTORY_STATUS            , ASN_INTEGER       , 0},
};

snmpagt_row_t RMON_ETHER_HISTORY_ROW[] = 
{
    {TBL_RMON_ETHER_HISTORY_INDEX          , ASN_INTEGER      , 0},
    {TBL_RMON_ETHER_HISTORY_SAMPLEINDEX    , ASN_INTEGER      , 0},
    {TBL_RMON_ETHER_HISTORY_INTERVALSTART  , ASN_TIMETICKS    , 0},
    {TBL_RMON_ETHER_HISTORY_DROPEVENTS     , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_OCTETS         , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_PKTS           , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_BROADCASTPKTS  , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_MULTICASTPKTS  , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_CRCALIGNERRORS , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_UNDERSIZEPKTS  , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_OVERSIZEPKTS   , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_FRAGMENTS      , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_JABBERS        , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_COLLISIONS     , ASN_COUNTER      , 0},
    {TBL_RMON_ETHER_HISTORY_UTILIZATION    , ASN_COUNTER      , 0},
};

snmpagt_row_t RMON_ALARM_ROW[] = 
{
    {TBL_RMON_ALARM_INDEX                   , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_INTERVAL                , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_VARIABLE                , ASN_OBJECT_ID   , 0},
    {TBL_RMON_ALARM_SAMPLETYPE              , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_VALUE                   , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_STARTUPALARM            , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_RISINGTHRESHOLD         , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_FALLINGTHRESHOLD        , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_RISINGEVENTINDEX        , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_FALLINGEVENTINDEX       , ASN_INTEGER     , 0},
    {TBL_RMON_ALARM_OWNER                   , ASN_OCTET_STR   , 0},
    {TBL_RMON_ALARM_STATUS                  , ASN_INTEGER     , 0},
};

snmpagt_row_t RMON_EVENT_ROW[] = 
{
    {TBL_RMON_EVENT_INDEX,              ASN_INTEGER,      0},
    {TBL_RMON_EVENT_DESCRIPTION,        ASN_OCTET_STR,    1},
    {TBL_RMON_EVENT_TYPE,               ASN_INTEGER,      0},
    {TBL_RMON_EVENT_COMMUNITY,          ASN_OCTET_STR,    0},
    {TBL_RMON_EVENT_LASTTIMESENT,       ASN_TIMETICKS,    0},
    {TBL_RMON_EVENT_OWNER,              ASN_OCTET_STR,    0},
    {TBL_RMON_EVENT_STATUS,             ASN_INTEGER,      0},
};

static snmpagt_table_t *g_p_rmon_ether_stats_tbl = NULL;
static snmpagt_table_t *g_p_rmon_history_tbl = NULL;
static snmpagt_table_t *g_p_rmon_ether_history_tbl = NULL;
static snmpagt_table_t *g_p_rmon_alarm_tbl = NULL;
static snmpagt_table_t *g_p_rmon_event_tbl = NULL;

static oid RMON_ETHER_STATS_OID[] = {RMONMIBOID, 1, 1};
static oid RMON_HISTORY_OID[] = {RMONMIBOID, 2, 1};
static oid RMON_ETHER_HISTORY_OID[] = {RMONMIBOID, 2, 2};
static oid RMON_ALARM_OID[] = {RMONMIBOID, 3, 1};
static oid RMON_EVENT_OID[] = {RMONMIBOID, 9, 1};
//static oid RMON_EVENT_OPER_OID[] = {RMONMIBOID, 9, 1, 1};

#define RMON_ETHER_STATS
int32
rmon_ether_stats_add_row(tbl_rmon_ether_stats_t *p_ether_stats)
{
    netsnmp_table_row *row = NULL;
    int32  index = p_ether_stats->key.index;
    uint32 u32 = 0;
    int32  rc  = 0;
    
    if (NULL == g_p_rmon_ether_stats_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_INDEX, ASN_INTEGER, (char*)(&index), sizeof(index));

    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_DATASOURCE, ASN_OBJECT_ID, 
        (void*)p_ether_stats->data_source, RMON_ETHERSTAT_DATASOURCE_MAXSIZE * sizeof(uint32));

    u32 = p_ether_stats->drop_events;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_DROPEVENTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OCTETS              , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS                , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->broadcast_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_BROADCASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->multicast_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_MULTICASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->crc_align_errors;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_CRCALIGNERRORS      , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->undersize_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_UNDERSIZEPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->oversize_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OVERSIZEPKTS        , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->fragments;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_FRAGMENTS           , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->jabbers;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_JABBERS             , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->collisions;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_COLLISIONS          , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_64_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS64OCTETS        , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_65to127_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS65TO127OCTETS   , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_128to255_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS128TO255OCTETS  , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_256to511_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS256TO511OCTETS  , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_512to1023_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS512TO1023OCTETS , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_stats->pkts_1024to1518_octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS1024TO1518OCTETS, ASN_COUNTER, (char*)(&u32), sizeof(u32));

    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OWNER, ASN_OCTET_STR, (char*)p_ether_stats->owner, sal_strlen(p_ether_stats->owner));
    u32 = p_ether_stats->status;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    rc = netsnmp_table_data_add_row(g_p_rmon_ether_stats_tbl->p_dataset->table, row);
    return rc;
}

int32
rmon_ether_stats_set_row(tbl_rmon_ether_stats_t *p_ether_stats, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32  index = p_ether_stats->key.index;
    uint32 u32 = 0;

    if (NULL == g_p_rmon_ether_stats_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_ether_stats_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE:
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_DATASOURCE, ASN_OBJECT_ID, 
            (char*)p_ether_stats->data_source, RMON_ETHERSTAT_DATASOURCE_MAXSIZE * sizeof(uint32));
        break;
    case TBL_RMON_ETHER_STATS_FLD_OWNER:
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OWNER, ASN_OCTET_STR, (char*)p_ether_stats->owner, sal_strlen(p_ether_stats->owner));
        break;
    case TBL_RMON_ETHER_STATS_FLD_STATUS:
        u32 = p_ether_stats->status;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;
    case TBL_RMON_ETHER_STATS_FLD_MAX:
        /* subscribe ETHER_STATS_FLD_MAX */
        u32 = p_ether_stats->drop_events;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_DROPEVENTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OCTETS              , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS                , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->broadcast_pkts;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_BROADCASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->multicast_pkts;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_MULTICASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->crc_align_errors;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_CRCALIGNERRORS      , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->undersize_pkts;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_UNDERSIZEPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->oversize_pkts;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_OVERSIZEPKTS        , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->fragments;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_FRAGMENTS           , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->jabbers;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_JABBERS             , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->collisions;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_COLLISIONS          , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_64_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS64OCTETS        , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_65to127_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS65TO127OCTETS   , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_128to255_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS128TO255OCTETS  , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_256to511_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS256TO511OCTETS  , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_512to1023_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS512TO1023OCTETS , ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_ether_stats->pkts_1024to1518_octets;
        netsnmp_set_row_column(row, TBL_RMON_ETHER_STATS_PKTS1024TO1518OCTETS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    default:
        break;
    }

    return 0;
}


int32
rmon_ether_stats_del_row(tbl_rmon_ether_stats_key_t *p_ether_stats)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_ether_stats->index;
    
    if (NULL == g_p_rmon_ether_stats_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_ether_stats_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_rmon_ether_stats_tbl->p_dataset, row);
    return 0;
}


int
rmon_ether_stats_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

#define RMON_HISTORY

int32
rmon_history_add_row(tbl_rmon_history_t *p_history)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_history->key.index;
    uint32 u32 = 0;
    int32  rc  = 0;
    
    if (NULL == g_p_rmon_history_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_INDEX, ASN_INTEGER, (char*)(&index), sizeof(index));
    
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_DATASOURCE, ASN_OBJECT_ID, 
        (char*)p_history->data, RMON_ETHERSTAT_DATASOURCE_MAXSIZE * sizeof(uint32));
 
    u32 = p_history->buckets_requested;
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_BUCKETSREQUESTED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_history->buckets_granted;
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_BUCKETSGRANTED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_history->interval;
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_INTERVAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_OWNER, ASN_OCTET_STR, (char*)p_history->owner, sal_strlen(p_history->owner));
    u32 = p_history->status;
    netsnmp_set_row_column(row, TBL_RMON_HISTORY_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    
    rc = netsnmp_table_data_add_row(g_p_rmon_history_tbl->p_dataset->table, row);
    return rc;
}

int32
rmon_history_set_row(tbl_rmon_history_t *p_history, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32  index = p_history->key.index;
    uint32 u32 = 0;

    if (NULL == g_p_rmon_history_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_history_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_RMON_HISTORY_FLD_DATA_SOURCE:
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_DATASOURCE, ASN_OBJECT_ID, 
            (char*)p_history->data, RMON_ETHERSTAT_DATASOURCE_MAXSIZE * sizeof(uint32));
        break;

    case TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED:
        u32 = p_history->buckets_requested;
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_BUCKETSREQUESTED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED:
        u32 = p_history->buckets_granted;
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_BUCKETSGRANTED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_HISTORY_FLD_INTERVAL:
        u32 = p_history->interval;
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_INTERVAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_HISTORY_FLD_OWNER:
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_OWNER, ASN_OCTET_STR, (char*)p_history->owner, sal_strlen(p_history->owner));
        break;

    case TBL_RMON_HISTORY_FLD_STATUS:
        u32 = p_history->status;
        netsnmp_set_row_column(row, TBL_RMON_HISTORY_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    default:
        break;
    }

    return 0;
}


int32
rmon_history_del_row(tbl_rmon_history_key_t *p_history)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_history->index;
    
    if (NULL == g_p_rmon_history_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_history_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_rmon_history_tbl->p_dataset, row);
    return 0;
}


int
rmon_history_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

#define ETHER_HISTORY

int32
rmon_ether_history_add_row(tbl_rmon_stats_history_t *p_ether_history)
{
    netsnmp_table_row *row = NULL;
    int32  his_index = p_ether_history->key.history_index;
    int32  sts_index = p_ether_history->key.stats_index;
    uint32 u32 = 0;
    int32  rc  = 0;
    
    if (NULL == g_p_rmon_ether_history_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &his_index, sizeof(his_index));
    netsnmp_table_row_add_index(row, ASN_INTEGER, &sts_index, sizeof(sts_index));
    
    /* columns */
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_INDEX, ASN_INTEGER, (char*)(&his_index), sizeof(his_index));
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_SAMPLEINDEX, ASN_INTEGER, (char*)(&sts_index), sizeof(sts_index));

    u32 = p_ether_history->interval_start / 1000;
    u32 = u32 * 100;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_INTERVALSTART, ASN_TIMETICKS, (char*)(&u32), sizeof(u32));

    u32 = p_ether_history->drop_events;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_DROPEVENTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->octets;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_OCTETS              , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_PKTS                , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->broadcast_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_BROADCASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->multicast_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_MULTICASTPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->crc_align_errors;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_CRCALIGNERRORS      , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->undersize_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_UNDERSIZEPKTS       , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->oversize_pkts;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_OVERSIZEPKTS        , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->fragments;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_FRAGMENTS           , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->jabbers;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_JABBERS             , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->collisions;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_COLLISIONS          , ASN_COUNTER, (char*)(&u32), sizeof(u32));
    u32 = p_ether_history->utilization;
    netsnmp_set_row_column(row, TBL_RMON_ETHER_HISTORY_UTILIZATION         , ASN_COUNTER, (char*)(&u32), sizeof(u32));

    rc = netsnmp_table_data_add_row(g_p_rmon_ether_history_tbl->p_dataset->table, row);
    return rc;
}

int32
rmon_ether_history_del_row(tbl_rmon_stats_history_key_t *p_ether_history)
{
    netsnmp_table_row *row = NULL;
    int32 index1 = p_ether_history->history_index;
    int32 index2 = p_ether_history->stats_index;
    
    if (NULL == g_p_rmon_ether_history_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_double_index(g_p_rmon_ether_history_tbl, index1, index2);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_rmon_ether_history_tbl->p_dataset, row);
    return 0;
}

int
rmon_ether_history_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

#define RMON_ALARM

int32
rmon_alarm_add_row(tbl_rmon_alarm_t *p_alarm)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_alarm->key.index;
    uint32 u32 = 0;
    int32  rc  = 0;
    
    if (NULL == g_p_rmon_alarm_tbl)
    {
        return -1;
    }

    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));

    /* columns */
    netsnmp_set_row_column(row, TBL_RMON_ALARM_INDEX, ASN_INTEGER, (char*)(&index), sizeof(index));

    netsnmp_set_row_column(row, TBL_RMON_ALARM_VARIABLE, ASN_OBJECT_ID, 
        (char*)p_alarm->variable, RMON_ALARM_VARIABLE_MAXSIZE * sizeof(uint32));

    u32 = p_alarm->interval;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_INTERVAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->sample_type;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_SAMPLETYPE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->value;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_VALUE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->startup_alarm;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_STARTUPALARM, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->rising_threshold;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_RISINGTHRESHOLD, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->falling_threshold;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_FALLINGTHRESHOLD, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->rising_event_index;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_RISINGEVENTINDEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_alarm->falling_event_index;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_FALLINGEVENTINDEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    netsnmp_set_row_column(row, TBL_RMON_ALARM_OWNER, ASN_OCTET_STR, (char*)p_alarm->owner, sal_strlen(p_alarm->owner));
    u32 = p_alarm->status;
    netsnmp_set_row_column(row, TBL_RMON_ALARM_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    rc = netsnmp_table_data_add_row(g_p_rmon_alarm_tbl->p_dataset->table, row);
    return rc;
}

int32
rmon_alarm_set_row(tbl_rmon_alarm_t *p_alarm, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32  index = p_alarm->key.index;
    uint32 u32 = 0;

    if (NULL == g_p_rmon_alarm_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_alarm_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_RMON_ALARM_FLD_VARIABLE:
        netsnmp_set_row_column(row, TBL_RMON_ALARM_VARIABLE, ASN_OBJECT_ID, 
            (char*)p_alarm->variable, RMON_ALARM_VARIABLE_MAXSIZE * sizeof(uint32));
        break;

    case TBL_RMON_ALARM_FLD_INTERVAL:
        u32 = p_alarm->interval;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_INTERVAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_SAMPLE_TYPE:
        u32 = p_alarm->sample_type;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_SAMPLETYPE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_VALUE:
        u32 = p_alarm->value;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_VALUE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_STARTUP_ALARM:
        u32 = p_alarm->startup_alarm;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_STARTUPALARM, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_RISING_THRESHOLD:
        u32 = p_alarm->rising_threshold;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_RISINGTHRESHOLD, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_FALLING_THRESHOLD:
        u32 = p_alarm->falling_threshold;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_FALLINGTHRESHOLD, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX:
        u32 = p_alarm->rising_event_index;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_RISINGEVENTINDEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX:
        u32 = p_alarm->falling_event_index;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_FALLINGEVENTINDEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_ALARM_FLD_OWNER:
        netsnmp_set_row_column(row, TBL_RMON_ALARM_OWNER, ASN_OCTET_STR, (char*)p_alarm->owner, sal_strlen(p_alarm->owner));
        break;

    case TBL_RMON_ALARM_FLD_STATUS:
        u32 = p_alarm->status;
        netsnmp_set_row_column(row, TBL_RMON_ALARM_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    default:
        break;
    }

    return 0;
}


int32
rmon_alarm_del_row(tbl_rmon_alarm_key_t *p_alarm)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_alarm->index;
    
    if (NULL == g_p_rmon_alarm_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_alarm_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_rmon_alarm_tbl->p_dataset, row);
    return 0;
}


int
rmon_alarm_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}


#define RMON_EVENT
int32
rmon_event_add_row(tbl_rmon_event_t *p_event)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_event->key.index;
    uint32 u32 = 0;
    int32  rc  = 0;
    
    if (NULL == g_p_rmon_event_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    netsnmp_set_row_column(row, TBL_RMON_EVENT_INDEX, ASN_INTEGER, (char*)(&index), sizeof(index));
    netsnmp_set_row_column(row, TBL_RMON_EVENT_DESCRIPTION, ASN_OCTET_STR, (char*)p_event->desc, sal_strlen(p_event->desc));
    u32 = p_event->type;
    netsnmp_set_row_column(row, TBL_RMON_EVENT_TYPE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    if (sal_strlen(p_event->community) == 0)
    {
        sal_strcpy(p_event->community, "public");
    }
    netsnmp_set_row_column(row, TBL_RMON_EVENT_COMMUNITY, ASN_OCTET_STR, (char*)p_event->community, sal_strlen(p_event->community));
    u32 = p_event->last_sent / 1000;
    u32 = u32 * 100;
    netsnmp_set_row_column(row, TBL_RMON_EVENT_LASTTIMESENT, ASN_TIMETICKS, (char*)(&u32), sizeof(u32));
    netsnmp_set_row_column(row, TBL_RMON_EVENT_OWNER, ASN_OCTET_STR, (char*)p_event->owner, sal_strlen(p_event->owner));
    u32 = p_event->status;
    netsnmp_set_row_column(row, TBL_RMON_EVENT_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    
    rc = netsnmp_table_data_add_row(g_p_rmon_event_tbl->p_dataset->table, row);
    return rc;
}

int32
rmon_event_set_row(tbl_rmon_event_t *p_event, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32  index = p_event->key.index;
    uint32 u32 = 0;

    if (NULL == g_p_rmon_event_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_event_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_RMON_EVENT_FLD_DESC:
        netsnmp_set_row_column(row, TBL_RMON_EVENT_DESCRIPTION, ASN_OCTET_STR, (char*)p_event->desc, sal_strlen(p_event->desc));
        break;

    case TBL_RMON_EVENT_FLD_TYPE:
        u32 = p_event->type;
        netsnmp_set_row_column(row, TBL_RMON_EVENT_TYPE, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_EVENT_FLD_COMMUNITY:
        if (sal_strlen(p_event->community) == 0)
        {
            sal_strcpy(p_event->community, "public");
        }
        netsnmp_set_row_column(row, TBL_RMON_EVENT_COMMUNITY, ASN_OCTET_STR, (char*)p_event->community, sal_strlen(p_event->community));
        break;

    case TBL_RMON_EVENT_FLD_LAST_SENT:
        u32 = p_event->last_sent / 1000;
        u32 = u32 * 100;
        netsnmp_set_row_column(row, TBL_RMON_EVENT_LASTTIMESENT, ASN_TIMETICKS, (char*)(&u32), sizeof(u32));
        break;

    case TBL_RMON_EVENT_FLD_OWNER:
        netsnmp_set_row_column(row, TBL_RMON_EVENT_OWNER, ASN_OCTET_STR, (char*)p_event->owner, sal_strlen(p_event->owner));
        break;

    case TBL_RMON_EVENT_FLD_STATUS:
        u32 = p_event->status;
        netsnmp_set_row_column(row, TBL_RMON_EVENT_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    default:
        break;
    }

    return 0;
}


int32
rmon_event_del_row(tbl_rmon_event_key_t *p_event)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_event->index;
    
    if (NULL == g_p_rmon_event_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_event_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_rmon_event_tbl->p_dataset, row);
    return 0;
}


int
rmon_event_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    netsnmp_table_request_info *table_info = NULL;
    table_info = netsnmp_extract_table_info(requests);
    char cmd[CMD_BUF_256];

    netsnmp_table_row *row = NULL;
    uint32 index = table_info->index_oid[0];
    row = snmpagt_table_get_row_by_int32_index(g_p_rmon_event_tbl, index);
    if (NULL == row)
    {
        return SNMP_ERR_NOERROR;
    }
    
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;
    case MODE_SET_RESERVE1:
        switch (table_info->colnum)
        {
        case TBL_RMON_EVENT_DESCRIPTION:
            sal_snprintf(cmd, CMD_BUF_256, "cdbctl update/cdb/app/rmon_event/%d/desc/%s", 
                index, requests->requestvb->val.string);
            sal_system(cmd);
            if (requests->status == SNMP_ERR_NOTWRITABLE)
            {
                requests->status = SNMP_ERR_NOERROR;
            }
            return SNMP_ERR_NOERROR;
        default:
            return SNMP_ERR_BADVALUE;
        }
        break;
    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int32
rmon_table_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(RMON_ETHER_STATS_OID, sizeof(RMON_ETHER_STATS_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = RMON_ETHER_STATS_ROW;
    param.row_count = sizeof(RMON_ETHER_STATS_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = rmon_ether_stats_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_rmon_ether_stats_tbl = table;

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(RMON_HISTORY_OID, sizeof(RMON_HISTORY_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = RMON_HISTORY_ROW;
    param.row_count = sizeof(RMON_HISTORY_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = rmon_history_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_rmon_history_tbl = table;

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(RMON_ETHER_HISTORY_OID, sizeof(RMON_ETHER_HISTORY_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_type[1] = ASN_INTEGER;
    param.index_count = 2;
    param.row = RMON_ETHER_HISTORY_ROW;
    param.row_count = sizeof(RMON_ETHER_HISTORY_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = rmon_ether_history_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_rmon_ether_history_tbl = table;

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(RMON_ALARM_OID, sizeof(RMON_ALARM_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = RMON_ALARM_ROW;
    param.row_count = sizeof(RMON_ALARM_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = rmon_alarm_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_rmon_alarm_tbl = table;

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(RMON_EVENT_OID, sizeof(RMON_EVENT_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = RMON_EVENT_ROW;
    param.row_count = sizeof(RMON_EVENT_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = rmon_event_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_rmon_event_tbl = table;

    return 0;
}


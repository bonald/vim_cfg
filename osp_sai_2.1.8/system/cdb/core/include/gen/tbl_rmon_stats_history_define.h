
#ifndef __TBL_RMON_STATS_HISTORY_DEFINE_H__
#define __TBL_RMON_STATS_HISTORY_DEFINE_H__

/* TBL_RMON_STATS_HISTORY field defines */
typedef enum
{
    TBL_RMON_STATS_HISTORY_FLD_KEY                  = 0 ,  /* READ */
    TBL_RMON_STATS_HISTORY_FLD_INTERVAL_START       = 1 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_DROP_EVENTS          = 2 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_OCTETS               = 3 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_PKTS                 = 4 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_BROADCAST_PKTS       = 5 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_MULTICAST_PKTS       = 6 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_CRC_ALIGN_ERRORS     = 7 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_UNDERSIZE_PKTS       = 8 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_OVERSIZE_PKTS        = 9 ,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_FRAGMENTS            = 10,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_JABBERS              = 11,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_COLLISIONS           = 12,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_UTILIZATION          = 13,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_DROP_EVENTS    = 14,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_OCTETS         = 15,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_PKTS           = 16,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_BROADCAST_PKTS = 17,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_MULTICAST_PKTS = 18,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_CRC_ALIGN_ERRORS = 19,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_UNDERSIZE_PKTS = 20,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_OVERSIZE_PKTS  = 21,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_FRAGMENTS      = 22,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_JABBERS        = 23,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_TOTAL_COLLISIONS     = 24,  /* RW */
    TBL_RMON_STATS_HISTORY_FLD_MAX                  = 25
} tbl_rmon_stats_history_field_id_t;

/* TBL_RMON_STATS_HISTORY defines */
typedef rmon_stats_history_key_t tbl_rmon_stats_history_key_t;

typedef struct
{
    tbl_rmon_stats_history_key_t key;
    uint64               interval_start;
    uint64               drop_events;
    uint64               octets;
    uint64               pkts;
    uint64               broadcast_pkts;
    uint64               multicast_pkts;
    uint64               crc_align_errors;
    uint64               undersize_pkts;
    uint64               oversize_pkts;
    uint64               fragments;
    uint64               jabbers;
    uint64               collisions;
    uint32               utilization;
    uint64               total_drop_events;
    uint64               total_octets;
    uint64               total_pkts;
    uint64               total_broadcast_pkts;
    uint64               total_multicast_pkts;
    uint64               total_crc_align_errors;
    uint64               total_undersize_pkts;
    uint64               total_oversize_pkts;
    uint64               total_fragments;
    uint64               total_jabbers;
    uint64               total_collisions;
} tbl_rmon_stats_history_t;

typedef struct
{
    ctclib_hash_t        *rmon_stats_history_hash;
    ctclib_slist_t       *rmon_stats_history_list;
} tbl_rmon_stats_history_master_t;

#endif /* !__TBL_RMON_STATS_HISTORY_DEFINE_H__ */


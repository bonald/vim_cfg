
#ifndef __TBL_RMON_ETHER_STATS_DEFINE_H__
#define __TBL_RMON_ETHER_STATS_DEFINE_H__

/* TBL_RMON_ETHER_STATS field defines */
typedef enum
{
    TBL_RMON_ETHER_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_RMON_ETHER_STATS_FLD_STATUS               = 1 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_OWNER                = 2 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE          = 3 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_DATA_SOURCE_IFINDEX  = 4 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_DROP_EVENTS          = 5 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_OCTETS               = 6 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS                 = 7 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_BROADCAST_PKTS       = 8 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_MULTICAST_PKTS       = 9 ,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_CRC_ALIGN_ERRORS     = 10,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_UNDERSIZE_PKTS       = 11,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_OVERSIZE_PKTS        = 12,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_FRAGMENTS            = 13,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_JABBERS              = 14,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_COLLISIONS           = 15,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_64_OCTETS       = 16,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_65TO127_OCTETS  = 17,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_128TO255_OCTETS = 18,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_256TO511_OCTETS = 19,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_512TO1023_OCTETS = 20,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_PKTS_1024TO1518_OCTETS = 21,  /* RW */
    TBL_RMON_ETHER_STATS_FLD_T_UNDER_CREATION_TIMER = 22,  /* READ */
    TBL_RMON_ETHER_STATS_FLD_MAX                  = 23
} tbl_rmon_ether_stats_field_id_t;

/* TBL_RMON_ETHER_STATS defines */
typedef struct
{
    uint32               index;
} tbl_rmon_ether_stats_key_t;

typedef struct
{
    tbl_rmon_ether_stats_key_t key;
    uint32               status;              /*1-Valid, 2-createRequest, *3-UnderCreation, 4- invalid, *5-nonExistent */
    char                 owner[RMON_OWNER_NAME_SIZE+1];
    uint32               data_source[RMON_ETHERSTAT_DATASOURCE_MAXSIZE];
    uint32               data_source_ifindex;
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
    uint64               pkts_64_octets;
    uint64               pkts_65to127_octets;
    uint64               pkts_128to255_octets;
    uint64               pkts_256to511_octets;
    uint64               pkts_512to1023_octets;
    uint64               pkts_1024to1518_octets;
    ctc_task_t           *t_under_creation_timer;
} tbl_rmon_ether_stats_t;

typedef struct
{
    ctclib_hash_t        *rmon_ether_stats_hash;
    ctclib_slist_t       *rmon_ether_stats_list;
} tbl_rmon_ether_stats_master_t;

#endif /* !__TBL_RMON_ETHER_STATS_DEFINE_H__ */


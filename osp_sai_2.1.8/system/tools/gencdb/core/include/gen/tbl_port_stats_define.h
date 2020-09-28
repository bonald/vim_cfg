
#ifndef __TBL_PORT_STATS_DEFINE_H__
#define __TBL_PORT_STATS_DEFINE_H__

/* TBL_PORT_STATS field defines */
typedef enum
{
    TBL_PORT_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_PORT_STATS_FLD_PORT_ID              = 1 ,  /* READ */
    TBL_PORT_STATS_FLD_OCTETS_RCV           = 2 ,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_RCV             = 3 ,  /* READ */
    TBL_PORT_STATS_FLD_UC_PKTS_RCV          = 4 ,  /* READ */
    TBL_PORT_STATS_FLD_BRDC_PKTS_RCV        = 5 ,  /* READ */
    TBL_PORT_STATS_FLD_MC_PKTS_RCV          = 6 ,  /* READ */
    TBL_PORT_STATS_FLD_OAM_PKTS_RCV         = 7 ,  /* READ */
    TBL_PORT_STATS_FLD_UNDERSIZE_PKTS       = 8 ,  /* READ */
    TBL_PORT_STATS_FLD_OVERSIZE_PKTS        = 9 ,  /* READ */
    TBL_PORT_STATS_FLD_JABBER_PKTS          = 10,  /* READ */
    TBL_PORT_STATS_FLD_MAC_RCV_ERROR        = 11,  /* READ */
    TBL_PORT_STATS_FLD_BAD_CRC              = 12,  /* READ */
    TBL_PORT_STATS_FLD_FRAME_ERROR          = 13,  /* READ */
    TBL_PORT_STATS_FLD_DROP_EVENTS          = 14,  /* READ */
    TBL_PORT_STATS_FLD_PAUSE_RCV            = 15,  /* READ */
    TBL_PORT_STATS_FLD_OCTETS_SEND          = 16,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_SEND            = 17,  /* READ */
    TBL_PORT_STATS_FLD_UC_PKTS_SEND         = 18,  /* READ */
    TBL_PORT_STATS_FLD_BRDC_PKTS_SEND       = 19,  /* READ */
    TBL_PORT_STATS_FLD_MC_PKTS_SEND         = 20,  /* READ */
    TBL_PORT_STATS_FLD_OAM_PKTS_SEND        = 21,  /* READ */
    TBL_PORT_STATS_FLD_UNDERRUNS            = 22,  /* READ */
    TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR     = 23,  /* READ */
    TBL_PORT_STATS_FLD_PAUSE_SEND           = 24,  /* READ */
    TBL_PORT_STATS_FLD_FCS_PKTS_RCV         = 25,  /* READ */
    TBL_PORT_STATS_FLD_FCS_OCTETS_RCV       = 26,  /* READ */
    TBL_PORT_STATS_FLD_FCS_PKTS_SEND        = 27,  /* READ */
    TBL_PORT_STATS_FLD_FCS_OCTETS_SEND      = 28,  /* READ */
    TBL_PORT_STATS_FLD_FRAGMENTS_PKTS       = 29,  /* READ */
    TBL_PORT_STATS_FLD_BAD_PKTS_RCV         = 30,  /* READ */
    TBL_PORT_STATS_FLD_BAD_OCTETS_RCV       = 31,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_64_RCV          = 32,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_65_TO_127_RCV   = 33,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_128_TO_255_RCV  = 34,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_256_TO_511_RCV  = 35,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_512_TO_1023_RCV = 36,  /* READ */
    TBL_PORT_STATS_FLD_PKTS_1024_TO_1518_RCV = 37,  /* READ */
    TBL_PORT_STATS_FLD_LAST_CLEAR_TS        = 38,  /* READ */
    TBL_PORT_STATS_FLD_MAX                  = 39
} tbl_port_stats_field_id_t;

/* TBL_PORT_STATS defines */
typedef struct
{
    uint32               ifindex;
} tbl_port_stats_key_t;

typedef struct
{
    tbl_port_stats_key_t key;
    uint32               port_id;
    uint64               octets_rcv;
    uint64               pkts_rcv;
    uint64               uc_pkts_rcv;
    uint64               brdc_pkts_rcv;
    uint64               mc_pkts_rcv;
    uint64               oam_pkts_rcv;
    uint64               undersize_pkts;
    uint64               oversize_pkts;
    uint64               jabber_pkts;
    uint64               mac_rcv_error;
    uint64               bad_crc;
    uint64               frame_error;
    uint64               drop_events;
    uint64               pause_rcv;
    uint64               octets_send;
    uint64               pkts_send;
    uint64               uc_pkts_send;
    uint64               brdc_pkts_send;
    uint64               mc_pkts_send;
    uint64               oam_pkts_send;
    uint64               underruns;
    uint64               mac_transmit_err;
    uint64               pause_send;
    uint64               fcs_pkts_rcv;
    uint64               fcs_octets_rcv;
    uint64               fcs_pkts_send;
    uint64               fcs_octets_send;
    uint64               fragments_pkts;
    uint64               bad_pkts_rcv;
    uint64               bad_octets_rcv;
    uint64               pkts_64_rcv;
    uint64               pkts_65_to_127_rcv;
    uint64               pkts_128_to_255_rcv;
    uint64               pkts_256_to_511_rcv;
    uint64               pkts_512_to_1023_rcv;
    uint64               pkts_1024_to_1518_rcv;
    uint32               last_clear_ts;       /* store last clear timestamp */
} tbl_port_stats_t;

typedef struct
{
    ctclib_hash_t        *port_stats_hash;
    ctclib_slist_t       *port_stats_list;
} tbl_port_stats_master_t;

#endif /* !__TBL_PORT_STATS_DEFINE_H__ */


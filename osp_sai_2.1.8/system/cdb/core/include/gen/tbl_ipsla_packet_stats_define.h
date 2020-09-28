
#ifndef __TBL_IPSLA_PACKET_STATS_DEFINE_H__
#define __TBL_IPSLA_PACKET_STATS_DEFINE_H__

/* TBL_IPSLA_PACKET_STATS field defines */
typedef enum
{
    TBL_IPSLA_PACKET_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPSLA_PACKET_STATS_FLD_SEND_TIME            = 1 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_RTT                  = 2 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_TESTID               = 3 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_INDEX                = 4 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_STATUS               = 5 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_TIMEOUT              = 6 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_THRESHOLD            = 7 ,  /* RW */
    TBL_IPSLA_PACKET_STATS_FLD_MAX                  = 8 
} tbl_ipsla_packet_stats_field_id_t;

/* TBL_IPSLA_PACKET_STATS defines */
typedef ipsla_packet_stats_key_t tbl_ipsla_packet_stats_key_t;

typedef struct
{
    tbl_ipsla_packet_stats_key_t key;
    uint64               send_time;
    uint32               rtt;
    uint32               testid;
    uint8                index;
    uint8                status;
    uint16               timeout;
    uint32               threshold;
} tbl_ipsla_packet_stats_t;

typedef struct
{
    ctclib_hash_t        *ipsla_packet_stats_hash;
    ctclib_slist_t       *ipsla_packet_stats_list;
} tbl_ipsla_packet_stats_master_t;

#endif /* !__TBL_IPSLA_PACKET_STATS_DEFINE_H__ */


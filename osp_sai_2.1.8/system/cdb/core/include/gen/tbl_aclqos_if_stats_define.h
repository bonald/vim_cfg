
#ifndef __TBL_ACLQOS_IF_STATS_DEFINE_H__
#define __TBL_ACLQOS_IF_STATS_DEFINE_H__

/* TBL_ACLQOS_IF_STATS field defines */
typedef enum
{
    TBL_ACLQOS_IF_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS = 1 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS = 2 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS = 3 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS = 4 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS = 5 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS = 6 ,  /* READ */
    TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT   = 7 ,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE  = 8 ,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT       = 9 ,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE      = 10,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET         = 11,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE           = 12,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET        = 13,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE          = 14,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_RED_PACKET           = 15,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_RED_BYTE             = 16,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT     = 17,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT       = 18,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT    = 19,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT      = 20,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT       = 21,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT         = 22,  /* RW */
    TBL_ACLQOS_IF_STATS_FLD_MAX                  = 23
} tbl_aclqos_if_stats_field_id_t;

/* TBL_ACLQOS_IF_STATS defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_aclqos_if_stats_key_t;

typedef struct
{
    tbl_aclqos_if_stats_key_t key;
    uint32               flags;               /* GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD */
    uint64               queue_transimt_pkt[GLB_QOS_MAX_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint64               queue_transimt_byte[GLB_QOS_MAX_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint64               queue_drop_pkt[GLB_QOS_MAX_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint64               queue_drop_byte[GLB_QOS_MAX_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint64               green_packet;        /*for policer stats*/
    uint64               green_byte;
    uint64               yellow_packet;
    uint64               yellow_byte;
    uint64               red_packet;
    uint64               red_byte;
    uint64               green_packet_out;
    uint64               green_byte_out;
    uint64               yellow_packet_out;
    uint64               yellow_byte_out;
    uint64               red_packet_out;
    uint64               red_byte_out;
} tbl_aclqos_if_stats_t;

typedef struct
{
    ctclib_hash_t        *if_hash;
    ctclib_slist_t       *if_list;
} tbl_aclqos_if_stats_master_t;

#endif /* !__TBL_ACLQOS_IF_STATS_DEFINE_H__ */


#ifndef __GLB_STATS_DEFINE_H__
#define __GLB_STATS_DEFINE_H__

#define GLB_STATS_RSLT_LEN(type)                                               \
    (type == GLB_STATS_TYPE_INTF ? sizeof(glb_if_stats_t) :                     \
    type == GLB_STATS_TYPE_PORT_QUEUE ? sizeof(glb_qos_queue_stats_t) :         \
    type == GLB_STATS_TYPE_PORT_POLICER ? sizeof(glb_qos_policer_stats_t) :     \
    type == GLB_STATS_TYPE_FLOW_POLICER ? sizeof(glb_qos_policer_stats_t) :     \
    type == GLB_STATS_TYPE_AGGREGATE_POLICER ? sizeof(glb_qos_policer_stats_t) :     \
    type == GLB_STATS_TYPE_OPENFLOW_FLOW ? sizeof(glb_openflow_stats_t) :     \
    type == GLB_STATS_TYPE_OPENFLOW_METER ? sizeof(glb_openflow_stats_t) :     \
    type == GLB_STATS_TYPE_OPENFLOW_GROUP ? sizeof(glb_openflow_stats_t) :     \
    sizeof(glb_stats_t))
    /* The newly added Stats's result length should be placed from here */

#define GLB_QOS_POLICER_STATS_SUPPORT_CONFIRM  (1<<0)
#define GLB_QOS_POLICER_STATS_SUPPORT_EXCEED   (1<<1)
#define GLB_QOS_POLICER_STATS_SUPPORT_VIOLATE  (1<<2)

#define GLB_QOS_QUEUE_STATS_SUPPORT_DEQUEUE  (1<<0)
#define GLB_QOS_QUEUE_STATS_SUPPORT_DROP     (1<<1)

#define GLB_STATS_CLEAR_KEY_MAX_NUM 20

enum glb_stats_type_e
{
    GLB_STATS_TYPE_INTF,
    GLB_STATS_TYPE_ACL,
    GLB_STATS_TYPE_SFLOW,
    GLB_STATS_TYPE_PORT_QUEUE,
    GLB_STATS_TYPE_PORT_POLICER,
    GLB_STATS_TYPE_FLOW_POLICER,
    GLB_STATS_TYPE_AGGREGATE_POLICER,
    GLB_STATS_TYPE_FLOW_RULE,
    GLB_STATS_TYPE_MPLS_ALL,
    GLB_STATS_TYPE_MPLS_ILM,
    GLB_STATS_TYPE_MPLS_FTN,
    GLB_STATS_TYPE_MPLS_VC,
    GLB_STATS_TYPE_OAM_EFM,
    GLB_STATS_TYPE_IPMC,    /*Modified by yanxa, for bug 18491, 2012-03-22 */
    /* The newly added Stats type should be placed from here */
    GLB_STATS_TYPE_OPENFLOW_FLOW,
    GLB_STATS_TYPE_OPENFLOW_METER,
    GLB_STATS_TYPE_OPENFLOW_GROUP,

    GLB_STATS_TYPE_MAX
};
typedef enum glb_stats_type_e glb_stats_type_t;

/* basic stats */
struct glb_stats_s
{
    uint64 packet_count;
    uint64 byte_count;
};
typedef struct glb_stats_s glb_stats_t;

/* policing stats */
struct glb_qos_policer_stats_s
{
    glb_stats_t confirm_stats;
    glb_stats_t exceed_stats;
    glb_stats_t violate_stats;
    /* Added by weizj for meter */
    uint32 resource_id;
    /*modified by yanxa, for bug 14870, 2011-06-09*/
    uint32 support_flag; /*bitmap of GLB_QOS_POLICER_STATS_SUPPORT_ */
};
typedef struct glb_qos_policer_stats_s glb_qos_policer_stats_t; 

/* Queue stats */
struct glb_qos_queue_stats_s
{
    glb_stats_t deq_stats;
    glb_stats_t drop_stats;
    /*modified by yanxa, for bug 14870, 2011-06-09*/
    uint32 support_flag;/*bitmap of GLB_QOS_QUEUE_STATS_SUPPORT_ */
};
typedef struct glb_qos_queue_stats_s glb_qos_queue_stats_t;

struct glb_qos_queue_stats_by_info_s
{
    uint32 ifindex;
    glb_qos_queue_stats_t queue_stats0;
    glb_qos_queue_stats_t queue_stats1;
    glb_qos_queue_stats_t queue_stats2;
    glb_qos_queue_stats_t queue_stats3;
    glb_qos_queue_stats_t queue_stats4;
    glb_qos_queue_stats_t queue_stats5;
    glb_qos_queue_stats_t queue_stats6;
    glb_qos_queue_stats_t queue_stats7;
};
typedef struct glb_qos_queue_stats_by_info_s glb_qos_queue_stats_by_info_t;

/* port stats */
struct glb_if_stats_s
{
    uint32 ifindex;/* added by cuixl for bug 17874, 2012-02-09 */
    uint64 good_pkts_rcv;
    uint64 good_octets_rcv;
    
    uint64 bad_octets_rcv;
    uint64 bad_pkts_rcv;

    uint64 uc_pkts_rcv;        /*Added by yanxa, for humber system. 2010-11-26*/
    uint64 mc_pkts_rcv;
    uint64 brdc_pkts_rcv;
    uint64 oam_pkts_rcv;        /*Added by yanxa, for humber system. 2010-11-26*/
    
    uint64 good_pkts_send;
    uint64 good_octets_send;
    
    uint64 uc_pkts_send;        /*Added by yanxa, for humber system. 2010-11-26*/
    uint64 mc_pkts_send;
    uint64 brdc_pkts_send;
    uint64 oam_pkts_send;        /*Added by yanxa, for humber system. 2010-11-26*/
    
    uint64 fc_send;
    uint64 good_fc_rcv;
    
    uint64 fragments_pkts;
    uint64 mac_transmit_err;
    uint64 oversize_pkts;
    uint64 undersize_pkts;
    uint64 jabber_pkts;
    uint64 unrecog_mac_cntr_rcv;
    uint64 mac_rcv_error;
    uint64 bad_crc;
    uint64 collisions;          /*Not used in humber system. 2010-11-26*/
    uint64 late_collisions;     /*Not used humber system. 2010-11-26*/
    
    uint64 pkts_64_octets;
    uint64 pkts_65_127_octets;
    uint64 pkts_128_255_octets;
    uint64 pkts_256_511_octets;
    uint64 pkts_512_1023_octets;
    uint64 pkts_1024_max_octets;
    uint64 excessive_collisions;
    uint64 drop_events;

    uint64 bad_fc_rcv;
    uint64 octets_rcv;
    uint64 pkts_rcv;
    uint64 octets_send;
    uint64 pkts_send;
    uint64 underruns;
    uint64 frame_error;
    uint64 pause_rcv;
    uint64 pause_send;
    uint64 no_carrier;          /*Not used humber system. 2010-11-26*/
    uint64 octets_rcv_rate;
    uint64 pkts_rcv_rate;
    uint64 octets_send_rate;
    uint64 pkts_send_rate;
    uint64 bad_pkts_rcv_sec;

    uint64 fcs_pkts_rcv;
    uint64 fcs_octets_rcv;
    uint64 fcs_pkts_send;
    uint64 fcs_octets_send;
    /* uint64 timeout_state; */ /* commented off by cuixl, 2012-02-28 for bug 17874, we don't use it now */
};
typedef struct glb_if_stats_s glb_if_stats_t;

/* openflow stats */
struct glb_openflow_stats_s
{
    uint64 packet_count;
    uint64 byte_count;
    uint32 resource_id;
    uint32 rsv_1;

    /* Added by weizj for meter */
    uint64 confirm_packets;
    uint64 confirm_bytes;
    uint64 exceed_packets;
    uint64 exceed_bytes;
    uint64 violate_packets;
    uint64 violate_bytes;
};
typedef struct glb_openflow_stats_s glb_openflow_stats_t;

union glb_stats_result_u
{
    glb_stats_t basic;
    glb_if_stats_t port;
    glb_qos_queue_stats_by_info_t queue;
    glb_qos_policer_stats_t policer;
    glb_openflow_stats_t openflow_stats;
    /* The newly added Stats's result ds should be placed from here */
};
typedef union glb_stats_result_u glb_stats_result_t;

#endif

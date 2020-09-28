
#ifndef __DS_ACLQOS_IF_DEFINE_H__
#define __DS_ACLQOS_IF_DEFINE_H__

/* DS_ACLQOS_IF field defines */
typedef enum
{
    DS_ACLQOS_IF_FLD_DEFAULT_COS          = 0 ,  /* RW */
    DS_ACLQOS_IF_FLD_DOMAIN               = 1 ,  /* RW */
    DS_ACLQOS_IF_FLD_TRUST                = 2 ,  /* RW */
    DS_ACLQOS_IF_FLD_REPLACE              = 3 ,  /* RW */
    DS_ACLQOS_IF_FLD_SHAPE_PIR            = 4 ,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE   = 5 ,  /* RW */
    DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP     = 6 ,  /* RW */
    DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP    = 7 ,  /* RW */
    DS_ACLQOS_IF_FLD_INPUT_POLICER        = 8 ,  /* RW */
    DS_ACLQOS_IF_FLD_OUTPUT_POLICER       = 9 ,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE  = 10,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE   = 11,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_CLASS          = 12,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_WEIGHT         = 13,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE      = 14,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE     = 15,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT   = 16,  /* READ */
    DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE  = 17,  /* READ */
    DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT       = 18,  /* READ */
    DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE      = 19,  /* READ */
    DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT  = 20,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE = 21,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT      = 22,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE     = 23,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT  = 24,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE = 25,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT      = 26,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE     = 27,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT  = 28,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE = 29,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT      = 30,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE     = 31,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT  = 32,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE = 33,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT      = 34,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE     = 35,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT  = 36,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE = 37,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT      = 38,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE     = 39,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT  = 40,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE = 41,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT      = 42,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE     = 43,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT  = 44,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE = 45,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT      = 46,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE     = 47,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT  = 48,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE = 49,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT      = 50,  /* RW */
    DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE     = 51,  /* RW */
    DS_ACLQOS_IF_FLD_PHB_ENABLE           = 52,  /* READ */
    DS_ACLQOS_IF_FLD_MAX                  = 53
} ds_aclqos_if_field_id_t;

/* DS_ACLQOS_IF defines */
typedef struct
{
    uint8                default_cos;         /* default port cos value, default: 0 */
    uint8                domain;              /* qos domain, default: 0 */
    uint8                trust;               /* trust state, refer to glb_qos_trust_state_t, default: trust cos */
    uint8                replace;             /* replace state, refter to glb_qos_replace_state_t, default: NONE*/
    uint32               shape_pir;           /* port shape, Unit:kbits/s, default: NONE*/
    uint8                queue_stats_enable;  /* */
    char                 input_policy_map[PMAP_NAME_SIZE]; /* attached police-map nmae */
    char                 output_policy_map[PMAP_NAME_SIZE]; /* attached police-map nmae */
    char                 input_policer[POLICER_NAME_SIZE]; /* policer name */
    char                 output_policer[POLICER_NAME_SIZE]; /* policer name */
    qos_name_t           queue_shape_profile[GLB_QOS_PORT_QUEUE_NUM]; /* queue shape name */
    qos_name_t           queue_drop_profile[GLB_QOS_PORT_QUEUE_NUM]; /* queue drop name */
    uint32               queue_class[GLB_QOS_PORT_QUEUE_NUM]; /* queue class */
    uint32               queue_weight[GLB_QOS_PORT_QUEUE_NUM]; /* queue weight */
    uint32               queue_drop_mode[GLB_QOS_PORT_QUEUE_NUM]; /* queue drop mode: glb_qos_queue_drop_mode_t */
    uint32               queue_ecn_enable[GLB_QOS_PORT_QUEUE_NUM]; /* ecn enable */
    uint32               queue_transimt_pkt[GLB_QOS_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint32               queue_transimt_byte[GLB_QOS_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint32               queue_drop_pkt[GLB_QOS_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint32               queue_drop_byte[GLB_QOS_PORT_QUEUE_NUM]; /* for dump queue stats */
    uint64               queue0_transimt_pkt; /* for dump queue stats */
    uint64               queue0_transimt_byte; /* for dump queue stats */
    uint64               queue0_drop_pkt;     /* for dump queue stats */
    uint64               queue0_drop_byte;    /* for dump queue stats */
    uint64               queue1_transimt_pkt; /* for dump queue stats */
    uint64               queue1_transimt_byte; /* for dump queue stats */
    uint64               queue1_drop_pkt;     /* for dump queue stats */
    uint64               queue1_drop_byte;    /* for dump queue stats */
    uint64               queue2_transimt_pkt; /* for dump queue stats */
    uint64               queue2_transimt_byte; /* for dump queue stats */
    uint64               queue2_drop_pkt;     /* for dump queue stats */
    uint64               queue2_drop_byte;    /* for dump queue stats */
    uint64               queue3_transimt_pkt; /* for dump queue stats */
    uint64               queue3_transimt_byte; /* for dump queue stats */
    uint64               queue3_drop_pkt;     /* for dump queue stats */
    uint64               queue3_drop_byte;    /* for dump queue stats */
    uint64               queue4_transimt_pkt; /* for dump queue stats */
    uint64               queue4_transimt_byte; /* for dump queue stats */
    uint64               queue4_drop_pkt;     /* for dump queue stats */
    uint64               queue4_drop_byte;    /* for dump queue stats */
    uint64               queue5_transimt_pkt; /* for dump queue stats */
    uint64               queue5_transimt_byte; /* for dump queue stats */
    uint64               queue5_drop_pkt;     /* for dump queue stats */
    uint64               queue5_drop_byte;    /* for dump queue stats */
    uint64               queue6_transimt_pkt; /* for dump queue stats */
    uint64               queue6_transimt_byte; /* for dump queue stats */
    uint64               queue6_drop_pkt;     /* for dump queue stats */
    uint64               queue6_drop_byte;    /* for dump queue stats */
    uint64               queue7_transimt_pkt; /* for dump queue stats */
    uint64               queue7_transimt_byte; /* for dump queue stats */
    uint64               queue7_drop_pkt;     /* for dump queue stats */
    uint64               queue7_drop_byte;    /* for dump queue stats */
    uint8                phb_enable;          /* for dump queue weight&class */
} ds_aclqos_if_t;

#endif /* !__DS_ACLQOS_IF_DEFINE_H__ */


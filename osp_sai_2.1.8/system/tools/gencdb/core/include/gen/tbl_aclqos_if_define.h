
#ifndef __TBL_ACLQOS_IF_DEFINE_H__
#define __TBL_ACLQOS_IF_DEFINE_H__

/* TBL_ACLQOS_IF field defines */
typedef enum
{
    TBL_ACLQOS_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACLQOS_IF_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_ACLQOS_IF_FLD_PORT_TYPE            = 2 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN         = 3 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE = 4 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER  = 5 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER = 6 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE = 7 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE = 8 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE = 9 ,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP   = 10,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS    = 11,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_TRUST          = 12,  /* READ */
    TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC        = 13,  /* READ */
    TBL_ACLQOS_IF_FLD_DOMAIN               = 14,  /* RW */
    TBL_ACLQOS_IF_FLD_COS_DOMAIN           = 15,  /* RW */
    TBL_ACLQOS_IF_FLD_DSCP_DOMAIN          = 16,  /* RW */
    TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE   = 17,  /* RW */
    TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP     = 18,  /* RW */
    TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP    = 19,  /* RW */
    TBL_ACLQOS_IF_FLD_INPUT_POLICER        = 20,  /* RW */
    TBL_ACLQOS_IF_FLD_OUTPUT_POLICER       = 21,  /* RW */
    TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE  = 22,  /* RW */
    TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE   = 23,  /* RW */
    TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE      = 24,  /* RW */
    TBL_ACLQOS_IF_FLD_REPLACE_DSCP         = 25,  /* RW */
    TBL_ACLQOS_IF_FLD_REPLACE_COS          = 26,  /* RW */
    TBL_ACLQOS_IF_FLD_TRUST                = 27,  /* RW */
    TBL_ACLQOS_IF_FLD_PORT_TC              = 28,  /* RW */
    TBL_ACLQOS_IF_FLD_MAX                  = 29
} tbl_aclqos_if_field_id_t;

/* TBL_ACLQOS_IF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_aclqos_if_key_t;

typedef struct
{
    tbl_aclqos_if_key_t  key;
    uint32               ifindex;
    uint32               port_type;           /* glb_if_type_t */
    uint32               flags;               /* GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD */
    uint32               domain;              /* qos domain, default: 0 */
    uint32               cos_domain;          /* cos_domain, default: 0 */
    uint32               dscp_domain;         /* dscp_domain, default: 0 */
    char                 port_shape_profile[QOS_NAME_SIZE+1];
    char                 input_policy_map[PMAP_NAME_SIZE+1]; /* attached police-map nmae */
    char                 output_policy_map[PMAP_NAME_SIZE+1]; /* attached police-map nmae */
    char                 input_policer[POLICER_NAME_SIZE+1]; /* policer name */
    char                 output_policer[POLICER_NAME_SIZE+1]; /* policer name */
    qos_name_t           queue_shape_profile[GLB_QOS_PORT_QUEUE_NUM]; /* queue shape name */
    qos_name_t           queue_drop_profile[GLB_QOS_PORT_QUEUE_NUM]; /* queue drop name */
    uint32               queue_drop_mode[GLB_QOS_PORT_QUEUE_NUM]; /* queue drop mode: glb_qos_queue_drop_mode_t */
    uint32               replace_dscp;        /* repalce dscp*/
    uint32               replace_cos;         /* repalce dscp*/
    uint32               trust;               /*port trust state, refer to glb_qos_trust_state_t */
    uint32               port_tc;             /*port default tc*/
} tbl_aclqos_if_t;

typedef struct
{
    ctclib_hash_t        *if_hash;
    ctclib_slist_t       *if_list;
} tbl_aclqos_if_master_t;

#endif /* !__TBL_ACLQOS_IF_DEFINE_H__ */


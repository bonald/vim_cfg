
#ifndef __STBL_QOS_MPLS_FLOW_DEFINE_H__
#define __STBL_QOS_MPLS_FLOW_DEFINE_H__

/* STBL_QOS_MPLS_FLOW field defines */
typedef enum
{
    STBL_QOS_MPLS_FLOW_FLD_KEY                  = 0 ,  /* READ */
    STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN         = 1 ,  /* RW */
    STBL_QOS_MPLS_FLOW_FLD_MAX                  = 2 
} stbl_qos_mpls_flow_field_id_t;

/* STBL_QOS_MPLS_FLOW defines */
typedef struct
{
    uint32               mpls_label;
} stbl_qos_mpls_flow_key_t;

typedef struct
{
    stbl_qos_mpls_flow_key_t key;
    uint32               mpls_flow_en;
} stbl_qos_mpls_flow_t;

typedef struct
{
    ctclib_hash_t        *qos_mpls_flow_hash;
} stbl_qos_mpls_flow_master_t;

#endif /* !__STBL_QOS_MPLS_FLOW_DEFINE_H__ */


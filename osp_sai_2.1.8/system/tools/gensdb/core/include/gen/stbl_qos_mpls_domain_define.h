
#ifndef __STBL_QOS_MPLS_DOMAIN_DEFINE_H__
#define __STBL_QOS_MPLS_DOMAIN_DEFINE_H__

/* STBL_QOS_MPLS_DOMAIN field defines */
typedef enum
{
    STBL_QOS_MPLS_DOMAIN_FLD_KEY                  = 0 ,  /* READ */
    STBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN           = 1 ,  /* RW */
    STBL_QOS_MPLS_DOMAIN_FLD_MAX                  = 2 
} stbl_qos_mpls_domain_field_id_t;

/* STBL_QOS_MPLS_DOMAIN defines */
typedef struct
{
    uint32               mpls_label;
} stbl_qos_mpls_domain_key_t;

typedef struct
{
    stbl_qos_mpls_domain_key_t key;
    uint32               exp_domain;
} stbl_qos_mpls_domain_t;

typedef struct
{
    ctclib_hash_t        *qos_mpls_domain_hash;
} stbl_qos_mpls_domain_master_t;

#endif /* !__STBL_QOS_MPLS_DOMAIN_DEFINE_H__ */


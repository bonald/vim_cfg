
#ifndef __TBL_QOS_MPLS_DOMAIN_DEFINE_H__
#define __TBL_QOS_MPLS_DOMAIN_DEFINE_H__

/* TBL_QOS_MPLS_DOMAIN field defines */
typedef enum
{
    TBL_QOS_MPLS_DOMAIN_FLD_KEY                  = 0 ,  /* READ */
    TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN           = 1 ,  /* RW */
    TBL_QOS_MPLS_DOMAIN_FLD_MAX                  = 2 
} tbl_qos_mpls_domain_field_id_t;

/* TBL_QOS_MPLS_DOMAIN defines */
typedef struct
{
    uint32               mpls_label;
} tbl_qos_mpls_domain_key_t;

typedef struct
{
    tbl_qos_mpls_domain_key_t key;
    uint32               exp_domain;          /* exp domain id */
} tbl_qos_mpls_domain_t;

typedef struct
{
    ctclib_hash_t        *qos_mpls_domain_hash;
    ctclib_slist_t       *qos_mpls_domain_list;
} tbl_qos_mpls_domain_master_t;

#endif /* !__TBL_QOS_MPLS_DOMAIN_DEFINE_H__ */


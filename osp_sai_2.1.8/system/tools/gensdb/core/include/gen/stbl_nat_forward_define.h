
#ifndef __STBL_NAT_FORWARD_DEFINE_H__
#define __STBL_NAT_FORWARD_DEFINE_H__

/* STBL_NAT_FORWARD field defines */
typedef enum
{
    STBL_NAT_FORWARD_FLD_KEY                  = 0 ,  /* READ */
    STBL_NAT_FORWARD_FLD_IIF_OID              = 1 ,  /* READ */
    STBL_NAT_FORWARD_FLD_OIF_OID              = 2 ,  /* READ */
    STBL_NAT_FORWARD_FLD_NHID                 = 3 ,  /* READ */
    STBL_NAT_FORWARD_FLD_USE_CNT              = 4 ,  /* RW */
    STBL_NAT_FORWARD_FLD_MAX                  = 5 
} stbl_nat_forward_field_id_t;

/* STBL_NAT_FORWARD defines */
typedef sai_nat_forward_entry_t stbl_nat_forward_key_t;

typedef struct
{
    stbl_nat_forward_key_t key;
    uint32               iif_oid;
    uint32               oif_oid;
    uint32               nhid;
    uint32               use_cnt;
} stbl_nat_forward_t;

typedef struct
{
    ctclib_hash_t        *nat_forward_hash;
} stbl_nat_forward_master_t;

#endif /* !__STBL_NAT_FORWARD_DEFINE_H__ */


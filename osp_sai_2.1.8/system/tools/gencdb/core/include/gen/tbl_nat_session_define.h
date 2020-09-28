
#ifndef __TBL_NAT_SESSION_DEFINE_H__
#define __TBL_NAT_SESSION_DEFINE_H__

/* TBL_NAT_SESSION field defines */
typedef enum
{
    TBL_NAT_SESSION_FLD_KEY                  = 0 ,  /* READ */
    TBL_NAT_SESSION_FLD_ID                   = 1 ,  /* RW */
    TBL_NAT_SESSION_FLD_TIMESTAMP            = 2 ,  /* RW */
    TBL_NAT_SESSION_FLD_HIT                  = 3 ,  /* RW */
    TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME  = 4 ,  /* RW */
    TBL_NAT_SESSION_FLD_TTL                  = 5 ,  /* RW */
    TBL_NAT_SESSION_FLD_IIF_INDEX            = 6 ,  /* RW */
    TBL_NAT_SESSION_FLD_OIF_INDEX            = 7 ,  /* RW */
    TBL_NAT_SESSION_FLD_HELPER               = 8 ,  /* RW */
    TBL_NAT_SESSION_FLD_SLOW_PATH_FWD        = 9 ,  /* RW */
    TBL_NAT_SESSION_FLD_CREATOR              = 10,  /* RW */
    TBL_NAT_SESSION_FLD_SET_SAI_STATUS       = 11,  /* RW */
    TBL_NAT_SESSION_FLD_MAX                  = 12
} tbl_nat_session_field_id_t;

/* TBL_NAT_SESSION defines */
typedef nat_session_key_t tbl_nat_session_key_t;

typedef struct
{
    tbl_nat_session_key_t key;
    uint32               id;
    sal_time_t           timestamp;           /* time when the session created*/
    sal_time_t           hit;                 /* time when the session hit*/
    sal_time_t           upd_kernel_ttl_time; /* time when update kernel ttl*/
    uint32               ttl;
    uint32               iif_index;
    uint32               oif_index;
    char                 helper[NAT_HELPER_NAME_SIZE];
    uint8                slow_path_fwd;       /* TRUE/FLASE, ICMP and ALG NAT flows should be forwarded by kernel*/
    uint8                creator;             /* refer to GLB_NAT_SESSION_CREATOR_E*/
    uint8                set_sai_status;      /* if set sai failed, must be exceed chip limit.*/
} tbl_nat_session_t;

typedef struct
{
    ctclib_hash_t        *nat_session_hash;
    ctclib_slist_t       *nat_session_list;
} tbl_nat_session_master_t;

#endif /* !__TBL_NAT_SESSION_DEFINE_H__ */


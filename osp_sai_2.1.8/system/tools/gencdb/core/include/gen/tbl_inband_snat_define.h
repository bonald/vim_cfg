
#ifndef __TBL_INBAND_SNAT_DEFINE_H__
#define __TBL_INBAND_SNAT_DEFINE_H__

/* TBL_INBAND_SNAT field defines */
typedef enum
{
    TBL_INBAND_SNAT_FLD_KEY                  = 0 ,  /* READ */
    TBL_INBAND_SNAT_FLD_IFNAME               = 1 ,  /* RW */
    TBL_INBAND_SNAT_FLD_IP                   = 2 ,  /* RW */
    TBL_INBAND_SNAT_FLD_TYPE                 = 3 ,  /* RW */
    TBL_INBAND_SNAT_FLD_NFT_HANDLE           = 4 ,  /* RW */
    TBL_INBAND_SNAT_FLD_SNAT_TIMER           = 5 ,  /* READ */
    TBL_INBAND_SNAT_FLD_MAX                  = 6 
} tbl_inband_snat_field_id_t;

/* TBL_INBAND_SNAT defines */
typedef inband_snat_key_t tbl_inband_snat_key_t;

typedef struct
{
    tbl_inband_snat_key_t key;
    char                 ifname[IFNAME_SIZE];
    addr_ipv4_t          ip;                  /* remote ip address */
    uint32               type;                /* refer to inband_snat_type_t */
    uint32               nft_handle;
    ctc_task_t           *snat_timer;
} tbl_inband_snat_t;

typedef struct
{
    ctclib_hash_t        *inband_snat_hash;
    ctclib_slist_t       *inband_snat_list;
} tbl_inband_snat_master_t;

#endif /* !__TBL_INBAND_SNAT_DEFINE_H__ */


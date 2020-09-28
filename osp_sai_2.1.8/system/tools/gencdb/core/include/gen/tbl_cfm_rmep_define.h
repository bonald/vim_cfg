
#ifndef __TBL_CFM_RMEP_DEFINE_H__
#define __TBL_CFM_RMEP_DEFINE_H__

/* TBL_CFM_RMEP field defines */
typedef enum
{
    TBL_CFM_RMEP_FLD_KEY                  = 0 ,  /* READ */
    TBL_CFM_RMEP_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_CFM_RMEP_FLD_MD_NAME              = 2 ,  /* RW */
    TBL_CFM_RMEP_FLD_MAID                 = 3 ,  /* RW */
    TBL_CFM_RMEP_FLD_MAC_ADDR             = 4 ,  /* READ */
    TBL_CFM_RMEP_FLD_FIRST_PKT_RX         = 5 ,  /* RW */
    TBL_CFM_RMEP_FLD_D_LOC                = 6 ,  /* RW */
    TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD       = 7 ,  /* RW */
    TBL_CFM_RMEP_FLD_MA_SA_MISMATCH       = 8 ,  /* RW */
    TBL_CFM_RMEP_FLD_RMEP_LAST_RDI        = 9 ,  /* RW */
    TBL_CFM_RMEP_FLD_IFNAME               = 10,  /* RW */
    TBL_CFM_RMEP_FLD_LOCAL_MEP_ID         = 11,  /* RW */
    TBL_CFM_RMEP_FLD_MAX                  = 12
} tbl_cfm_rmep_field_id_t;

/* TBL_CFM_RMEP defines */
typedef cfm_rmep_key_t tbl_cfm_rmep_key_t;

typedef struct
{
    tbl_cfm_rmep_key_t   key;
    uint32               ifindex;             /* The interface which RMEP locates */
    char                 md_name[CFM_MD_NAME_MAX_LENGTH_8021AG+1]; /* Domain name */
    char                 maid[CFM_MAID_MAX_LENGTH+1]; /* maid */
    mac_addr_t           mac_addr;            /* Remote mep mac address */
    uint8                first_pkt_rx;        /* Receive first correct packet */
    uint8                d_loc;               /* Don't receive ccm packets in 3.5 */
    uint8                d_unexp_period;      /* Incorrect ccm period */
    uint8                ma_sa_mismatch;      /* Incorrect source mac address for RMEP */
    uint8                rmep_last_rdi;       /* Remote last rdi */
    char                 ifname[IFNAME_SIZE]; /* Interface name */
    uint32               local_mep_id;        /* Local MEP ID */
} tbl_cfm_rmep_t;

typedef struct
{
    ctclib_hash_t        *cfm_rmep_hash;
    ctclib_slist_t       *cfm_rmep_list;
} tbl_cfm_rmep_master_t;

#endif /* !__TBL_CFM_RMEP_DEFINE_H__ */


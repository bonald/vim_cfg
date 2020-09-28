
#ifndef __STBL_CFM_RMEP_DEFINE_H__
#define __STBL_CFM_RMEP_DEFINE_H__

/* STBL_CFM_RMEP field defines */
typedef enum
{
    STBL_CFM_RMEP_FLD_KEY                  = 0 ,  /* READ */
    STBL_CFM_RMEP_FLD_GPORT                = 1 ,  /* RW */
    STBL_CFM_RMEP_FLD_VLAN_ID              = 2 ,  /* RW */
    STBL_CFM_RMEP_FLD_MD_LEVEL             = 3 ,  /* RW */
    STBL_CFM_RMEP_FLD_RMEP_ID              = 4 ,  /* RW */
    STBL_CFM_RMEP_FLD_RMEP_MAC             = 5 ,  /* READ */
    STBL_CFM_RMEP_FLD_IFINDEX              = 6 ,  /* RW */
    STBL_CFM_RMEP_FLD_FIRST_PKT_RX         = 7 ,  /* RW */
    STBL_CFM_RMEP_FLD_D_LOC                = 8 ,  /* RW */
    STBL_CFM_RMEP_FLD_D_UNEXP_PERIOD       = 9 ,  /* RW */
    STBL_CFM_RMEP_FLD_MA_SA_MISMATCH       = 10,  /* RW */
    STBL_CFM_RMEP_FLD_RMEP_LAST_RDI        = 11,  /* RW */
    STBL_CFM_RMEP_FLD_MAX                  = 12
} stbl_cfm_rmep_field_id_t;

/* STBL_CFM_RMEP defines */
typedef struct
{
    uint32               index;
} stbl_cfm_rmep_key_t;

typedef struct
{
    stbl_cfm_rmep_key_t  key;
    uint16               gport;
    uint16               vlan_id;
    uint8                md_level;
    uint32               rmep_id;             /* MEP ID */
    mac_addr_t           rmep_mac;
    uint32               ifindex;
    uint8                first_pkt_rx;        /* Receive first correct packet */
    uint8                d_loc;               /* Don't receive ccm packets in 3.5 */
    uint8                d_unexp_period;      /* Incorrect ccm period */
    uint8                ma_sa_mismatch;      /* Incorrect source mac address for RMEP */
    uint8                rmep_last_rdi;       /* Remote last rdi */
} stbl_cfm_rmep_t;

typedef struct
{
    ctclib_hash_t        *cfm_rmep_hash;
    ctclib_slist_t       *cfm_rmep_list;
} stbl_cfm_rmep_master_t;

#endif /* !__STBL_CFM_RMEP_DEFINE_H__ */


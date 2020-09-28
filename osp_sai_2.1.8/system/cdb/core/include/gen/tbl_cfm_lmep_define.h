
#ifndef __TBL_CFM_LMEP_DEFINE_H__
#define __TBL_CFM_LMEP_DEFINE_H__

/* TBL_CFM_LMEP field defines */
typedef enum
{
    TBL_CFM_LMEP_FLD_KEY                  = 0 ,  /* READ */
    TBL_CFM_LMEP_FLD_MEP_ID               = 1 ,  /* RW */
    TBL_CFM_LMEP_FLD_MAID                 = 2 ,  /* RW */
    TBL_CFM_LMEP_FLD_MD_NAME              = 3 ,  /* RW */
    TBL_CFM_LMEP_FLD_DIRECTION            = 4 ,  /* RW */
    TBL_CFM_LMEP_FLD_D_UNEXP_MEP          = 5 ,  /* RW */
    TBL_CFM_LMEP_FLD_D_MISMERGE           = 6 ,  /* RW */
    TBL_CFM_LMEP_FLD_D_MEG_LVL            = 7 ,  /* RW */
    TBL_CFM_LMEP_FLD_CCI_ENABLED          = 8 ,  /* RW */
    TBL_CFM_LMEP_FLD_CCI_INTERVAL         = 9 ,  /* RW */
    TBL_CFM_LMEP_FLD_RDI                  = 10,  /* RW */
    TBL_CFM_LMEP_FLD_IFNAME               = 11,  /* RW */
    TBL_CFM_LMEP_FLD_IF_MAC_ADDR          = 12,  /* RW */
    TBL_CFM_LMEP_FLD_CCM_VLAN_COS         = 13,  /* RW */
    TBL_CFM_LMEP_FLD_RING_ID              = 14,  /* RW */
    TBL_CFM_LMEP_FLD_MEP_INDEX            = 15,  /* RW */
    TBL_CFM_LMEP_FLD_RMEP_NUM             = 16,  /* RW */
    TBL_CFM_LMEP_FLD_MAX                  = 17
} tbl_cfm_lmep_field_id_t;

/* TBL_CFM_LMEP defines */
typedef cfm_lmep_key_t tbl_cfm_lmep_key_t;

typedef struct
{
    tbl_cfm_lmep_key_t   key;
    uint32               mep_id;              /* MEP ID */
    char                 maid[CFM_MAID_MAX_LENGTH+1]; /* maid */
    char                 md_name[CFM_MD_NAME_MAX_LENGTH_8021AG+1]; /* Domain name */
    uint8                direction;           /* Up or down */
    uint8                d_unexp_mep;         /* Incorrect MEP ID */
    uint8                d_mismerge;          /* Incorrect MAID */
    uint8                d_meg_lvl;           /* Low level */
    uint8                cci_enabled;         /* CCM enabled */
    uint8                cci_interval;        /* CC interval */
    uint8                rdi;                 /* Remote Defect Indication */
    char                 ifname[IFNAME_SIZE]; /* Interface name */
    mac_addr_t           if_mac_addr;         /* Interface mac address */
    uint8                ccm_vlan_cos;        /* Vlan priority of CCM packets */
    uint32               ring_id;             /* G8032 Ring Id */
    uint32               mep_index;           /* Mep index */
    uint32               rmep_num;            /* Remote mep number for this local mep */
} tbl_cfm_lmep_t;

typedef struct
{
    ctclib_hash_t        *cfm_lmep_hash;
    ctclib_slist_t       *cfm_lmep_list;
} tbl_cfm_lmep_master_t;

#endif /* !__TBL_CFM_LMEP_DEFINE_H__ */


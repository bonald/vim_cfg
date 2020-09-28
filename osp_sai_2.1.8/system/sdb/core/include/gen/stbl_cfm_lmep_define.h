
#ifndef __STBL_CFM_LMEP_DEFINE_H__
#define __STBL_CFM_LMEP_DEFINE_H__

/* STBL_CFM_LMEP field defines */
typedef enum
{
    STBL_CFM_LMEP_FLD_KEY                  = 0 ,  /* READ */
    STBL_CFM_LMEP_FLD_GPORT                = 1 ,  /* RW */
    STBL_CFM_LMEP_FLD_VLAN_ID              = 2 ,  /* RW */
    STBL_CFM_LMEP_FLD_MD_LEVEL             = 3 ,  /* RW */
    STBL_CFM_LMEP_FLD_LMEP_NUM             = 4 ,  /* RW */
    STBL_CFM_LMEP_FLD_MEP_ID               = 5 ,  /* RW */
    STBL_CFM_LMEP_FLD_MAID                 = 6 ,  /* RW */
    STBL_CFM_LMEP_FLD_DIRECTION            = 7 ,  /* RW */
    STBL_CFM_LMEP_FLD_D_UNEXP_MEP          = 8 ,  /* RW */
    STBL_CFM_LMEP_FLD_D_MISMERGE           = 9 ,  /* RW */
    STBL_CFM_LMEP_FLD_D_MEG_LVL            = 10,  /* RW */
    STBL_CFM_LMEP_FLD_CCI_ENABLED          = 11,  /* RW */
    STBL_CFM_LMEP_FLD_CCI_INTERVAL         = 12,  /* RW */
    STBL_CFM_LMEP_FLD_RDI                  = 13,  /* RW */
    STBL_CFM_LMEP_FLD_CCM_VLAN_COS         = 14,  /* RW */
    STBL_CFM_LMEP_FLD_IFINDEX              = 15,  /* RW */
    STBL_CFM_LMEP_FLD_MODE                 = 16,  /* RW */
    STBL_CFM_LMEP_FLD_MAX                  = 17
} stbl_cfm_lmep_field_id_t;

/* STBL_CFM_LMEP defines */
typedef struct
{
    uint32               index;
} stbl_cfm_lmep_key_t;

typedef struct
{
    stbl_cfm_lmep_key_t  key;
    uint32               gport;
    uint16               vlan_id;
    uint8                md_level;
    uint8                lmep_num;
    uint32               mep_id;              /* MEP ID */
    uint8                maid[GLB_CFM_MAID_LENGTH_48BYTES]; /* maid */
    uint8                direction;           /* Up or down */
    uint8                d_unexp_mep;         /* Incorrect MEP ID */
    uint8                d_mismerge;          /* Incorrect MAID */
    uint8                d_meg_lvl;           /* Low level */
    uint8                cci_enabled;         /* CCM enabled */
    uint8                cci_interval;        /* CC interval */
    uint8                rdi;                 /* Remote Defect Indication */
    uint8                ccm_vlan_cos;        /* Vlan priority of CCM packets */
    uint32               ifindex;             /* Interface index */
    uint8                mode;                /* 802.1ag or Y.1731 mode */
} stbl_cfm_lmep_t;

typedef struct
{
    ctclib_hash_t        *cfm_lmep_hash;
    ctclib_slist_t       *cfm_lmep_list;
} stbl_cfm_lmep_master_t;

#endif /* !__STBL_CFM_LMEP_DEFINE_H__ */


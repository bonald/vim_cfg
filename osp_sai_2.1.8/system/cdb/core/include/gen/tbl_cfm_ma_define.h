
#ifndef __TBL_CFM_MA_DEFINE_H__
#define __TBL_CFM_MA_DEFINE_H__

/* TBL_CFM_MA field defines */
typedef enum
{
    TBL_CFM_MA_FLD_KEY                  = 0 ,  /* READ */
    TBL_CFM_MA_FLD_NAME                 = 1 ,  /* RW */
    TBL_CFM_MA_FLD_NAME_TYPE            = 2 ,  /* RW */
    TBL_CFM_MA_FLD_CCI_ENABLED          = 3 ,  /* RW */
    TBL_CFM_MA_FLD_CCM_VLAN_COS         = 4 ,  /* RW */
    TBL_CFM_MA_FLD_PKT_NAME             = 5 ,  /* RW */
    TBL_CFM_MA_FLD_MAID                 = 6 ,  /* RW */
    TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE  = 7 ,  /* RW */
    TBL_CFM_MA_FLD_ERROR_NUMBER         = 8 ,  /* RW */
    TBL_CFM_MA_FLD_MAX                  = 9 
} tbl_cfm_ma_field_id_t;

/* TBL_CFM_MA defines */
typedef cfm_ma_key_t tbl_cfm_ma_key_t;

typedef struct
{
    tbl_cfm_ma_key_t     key;
    char                 name[CFM_MA_NAME_MAX_LENGTH_8021AG+1]; /* MA name */
    uint8                name_type;           /* MA name type */
    uint8                cci_enabled;         /* CCM enabled */
    uint8                ccm_vlan_cos;        /* Vlan priority of CCM packets */
    char                 pkt_name[CFM_MA_NAME_MAX_LENGTH_8021AG+1]; /* valid when ma name type is string/icc-based */
    char                 maid[CFM_MAID_MAX_LENGTH+1]; /* maid */
    uint8                error_write_disable; /* Reserve error write disable */
    uint8                error_number;
} tbl_cfm_ma_t;

typedef struct
{
    ctclib_hash_t        *cfm_ma_hash;
    ctclib_slist_t       *cfm_ma_list;
} tbl_cfm_ma_master_t;

#endif /* !__TBL_CFM_MA_DEFINE_H__ */


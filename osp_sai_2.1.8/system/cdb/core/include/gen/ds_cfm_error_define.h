
#ifndef __DS_CFM_ERROR_DEFINE_H__
#define __DS_CFM_ERROR_DEFINE_H__

/* DS_CFM_ERROR field defines */
typedef enum
{
    DS_CFM_ERROR_FLD_KEY                  = 0 ,  /* READ */
    DS_CFM_ERROR_FLD_LEVEL                = 1 ,  /* RW */
    DS_CFM_ERROR_FLD_VLAN_ID              = 2 ,  /* RW */
    DS_CFM_ERROR_FLD_MD_NAME              = 3 ,  /* RW */
    DS_CFM_ERROR_FLD_MEP_ID               = 4 ,  /* RW */
    DS_CFM_ERROR_FLD_ERR_REASON           = 5 ,  /* RW */
    DS_CFM_ERROR_FLD_RMEP_MAC             = 6 ,  /* RW */
    DS_CFM_ERROR_FLD_MAX                  = 7 
} ds_cfm_error_field_id_t;

/* DS_CFM_ERROR defines */
typedef cfm_time_key_t ds_cfm_error_key_t;

typedef struct
{
    ds_cfm_error_key_t   key;
    uint16               level;
    uint16               vlan_id;
    char                 md_name[CFM_MD_NAME_MAX_LENGTH_8021AG]; /* MA name */
    uint32               mep_id;
    uint8                err_reason;
    mac_addr_t           rmep_mac;
} ds_cfm_error_t;

#endif /* !__DS_CFM_ERROR_DEFINE_H__ */


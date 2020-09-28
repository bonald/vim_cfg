
#ifndef __TBL_LLDP_GLOBAL_DEFINE_H__
#define __TBL_LLDP_GLOBAL_DEFINE_H__

/* TBL_LLDP_GLOBAL field defines */
typedef enum
{
    TBL_LLDP_GLOBAL_FLD_ENABLE               = 0 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN        = 1 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG          = 2 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC           = 3 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP        = 4 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_TX_HOLD              = 5 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_TX_INTERVAL          = 6 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_TX_DELAY             = 7 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_REINIT_DELAY         = 8 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY    = 9 ,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME          = 10,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC          = 11,  /* RW */
    TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT  = 12,  /* RW */
    TBL_LLDP_GLOBAL_FLD_MAX                  = 13
} tbl_lldp_global_field_id_t;

/* TBL_LLDP_GLOBAL defines */
typedef struct
{
    uint8                enable;
    uint8                system_mac_en;
    uint8                config_flag;
    mac_addr_t           system_mac;
    addr_t               management_ip;
    uint32               tx_hold;
    uint32               tx_interval;
    uint32               tx_delay;
    uint32               reinit_delay;
    uint32               system_capability;   /* bitmap of lldp_sys_cap_t */
    char                 system_name[LLDP_NAME_SIZE+1];
    char                 system_desc[LLDP_DESCRIPTION_SIZE+1];
    char                 system_desc_default[LLDP_DESCRIPTION_SIZE+1];
} tbl_lldp_global_t;

#endif /* !__TBL_LLDP_GLOBAL_DEFINE_H__ */


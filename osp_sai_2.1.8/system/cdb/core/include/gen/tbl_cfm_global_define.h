
#ifndef __TBL_CFM_GLOBAL_DEFINE_H__
#define __TBL_CFM_GLOBAL_DEFINE_H__

/* TBL_CFM_GLOBAL field defines */
typedef enum
{
    TBL_CFM_GLOBAL_FLD_CFM_ENABLE           = 0 ,  /* RW */
    TBL_CFM_GLOBAL_FLD_CFM_MODE             = 1 ,  /* RW */
    TBL_CFM_GLOBAL_FLD_MAX                  = 2 
} tbl_cfm_global_field_id_t;

/* TBL_CFM_GLOBAL defines */
typedef struct
{
    uint8                cfm_enable;          /* cfm enable */
    uint8                cfm_mode;            /* 802.1ag or Y.1731 */
} tbl_cfm_global_t;

#endif /* !__TBL_CFM_GLOBAL_DEFINE_H__ */


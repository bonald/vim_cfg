
#ifndef __TBL_SNMP_TARGET_PARAMS_DEFINE_H__
#define __TBL_SNMP_TARGET_PARAMS_DEFINE_H__

/* TBL_SNMP_TARGET_PARAMS field defines */
typedef enum
{
    TBL_SNMP_TARGET_PARAMS_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_TARGET_PARAMS_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME            = 2 ,  /* RW */
    TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL            = 3 ,  /* RW */
    TBL_SNMP_TARGET_PARAMS_FLD_MAX                  = 4 
} tbl_snmp_target_params_field_id_t;

/* TBL_SNMP_TARGET_PARAMS defines */
typedef struct
{
    char                 params_name[SNMP_NAME_SIZE];
} tbl_snmp_target_params_key_t;

typedef struct
{
    tbl_snmp_target_params_key_t key;                 /* target params name */
    uint8                flags;               /* delete entry 0xFF */
    char                 user_name[SNMP_NAME_SIZE]; /* user name */
    uint8                usm_level;           /* usm level noauth or auth or priv */
} tbl_snmp_target_params_t;

typedef struct
{
    ctclib_hash_t        *snmp_target_params_hash;
    ctclib_slist_t       *snmp_target_params_list;
} tbl_snmp_target_params_master_t;

#endif /* !__TBL_SNMP_TARGET_PARAMS_DEFINE_H__ */


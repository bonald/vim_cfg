
#ifndef __TBL_SNMP_GROUP_USM_DEFINE_H__
#define __TBL_SNMP_GROUP_USM_DEFINE_H__

/* TBL_SNMP_GROUP_USM field defines */
typedef enum
{
    TBL_SNMP_GROUP_USM_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_GROUP_USM_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_GROUP_USM_FLD_SECURITY_NAME        = 2 ,  /* RW */
    TBL_SNMP_GROUP_USM_FLD_MAX                  = 3 
} tbl_snmp_group_usm_field_id_t;

/* TBL_SNMP_GROUP_USM defines */
typedef struct
{
    snmp_group_usm_key_t group_usm;
} tbl_snmp_group_usm_key_t;

typedef struct
{
    tbl_snmp_group_usm_key_t key;                 /* usm group name+security name */
    uint8                flags;               /* delete entry 0xFF */
    char                 security_name[SNMP_NAME_SIZE]; /* usm security name */
} tbl_snmp_group_usm_t;

typedef struct
{
    ctclib_hash_t        *snmp_group_usm_hash;
    ctclib_slist_t       *snmp_group_usm_list;
} tbl_snmp_group_usm_master_t;

#endif /* !__TBL_SNMP_GROUP_USM_DEFINE_H__ */


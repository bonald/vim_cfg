
#ifndef __TBL_SNMP_ACCESS_USM_DEFINE_H__
#define __TBL_SNMP_ACCESS_USM_DEFINE_H__

/* TBL_SNMP_ACCESS_USM field defines */
typedef enum
{
    TBL_SNMP_ACCESS_USM_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL            = 2 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_READ_VIEW            = 3 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW           = 4 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW          = 5 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME         = 6 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_ISPREFIX             = 7 ,  /* RW */
    TBL_SNMP_ACCESS_USM_FLD_MAX                  = 8 
} tbl_snmp_access_usm_field_id_t;

/* TBL_SNMP_ACCESS_USM defines */
typedef struct
{
    char                 usm_access_name[SNMP_NAME_SIZE];
} tbl_snmp_access_usm_key_t;

typedef struct
{
    tbl_snmp_access_usm_key_t key;                 /* access group name */
    uint8                flags;               /* delete entry 0xFF */
    uint8                usm_level;           /* usm level noauth or auth or priv */
    char                 read_view[SNMP_NAME_SIZE]; /* read view name */
    char                 write_view[SNMP_NAME_SIZE]; /* write view name */
    char                 notify_view[SNMP_NAME_SIZE]; /* notify view name */
    char                 context_name[SNMP_NAME_SIZE]; /* context name */
    uint8                isprefix;            /* exact or prefix, default is exact */
} tbl_snmp_access_usm_t;

typedef struct
{
    ctclib_hash_t        *snmp_access_usm_hash;
    ctclib_slist_t       *snmp_access_usm_list;
} tbl_snmp_access_usm_master_t;

#endif /* !__TBL_SNMP_ACCESS_USM_DEFINE_H__ */



#ifndef __TBL_SNMP_NOTIFY_DEFINE_H__
#define __TBL_SNMP_NOTIFY_DEFINE_H__

/* TBL_SNMP_NOTIFY field defines */
typedef enum
{
    TBL_SNMP_NOTIFY_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_NOTIFY_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_NOTIFY_FLD_TAG_NAME             = 2 ,  /* RW */
    TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE          = 3 ,  /* RW */
    TBL_SNMP_NOTIFY_FLD_MAX                  = 4 
} tbl_snmp_notify_field_id_t;

/* TBL_SNMP_NOTIFY defines */
typedef struct
{
    char                 notify_name[SNMP_NAME_SIZE];
} tbl_snmp_notify_key_t;

typedef struct
{
    tbl_snmp_notify_key_t key;                 /* notify name */
    uint8                flags;               /* delete entry 0xFF */
    char                 tag_name[SNMP_INFO_SIZE]; /* tag name, max 255, not allow 0x20,0x9,0xd,0xb */
    uint8                notify_type;         /* trap-0 or inform-1, default is trap */
} tbl_snmp_notify_t;

typedef struct
{
    ctclib_hash_t        *snmp_notify_hash;
    ctclib_slist_t       *snmp_notify_list;
} tbl_snmp_notify_master_t;

#endif /* !__TBL_SNMP_NOTIFY_DEFINE_H__ */


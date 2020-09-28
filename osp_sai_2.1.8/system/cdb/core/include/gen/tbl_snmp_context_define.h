
#ifndef __TBL_SNMP_CONTEXT_DEFINE_H__
#define __TBL_SNMP_CONTEXT_DEFINE_H__

/* TBL_SNMP_CONTEXT field defines */
typedef enum
{
    TBL_SNMP_CONTEXT_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_CONTEXT_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_CONTEXT_FLD_MAX                  = 2 
} tbl_snmp_context_field_id_t;

/* TBL_SNMP_CONTEXT defines */
typedef struct
{
    char                 context_name[SNMP_NAME_SIZE];
} tbl_snmp_context_key_t;

typedef struct
{
    tbl_snmp_context_key_t key;
    uint8                flags;               /* delete entry 0xFF */
} tbl_snmp_context_t;

typedef struct
{
    ctclib_hash_t        *snmp_context_hash;
    ctclib_slist_t       *snmp_context_list;
} tbl_snmp_context_master_t;

#endif /* !__TBL_SNMP_CONTEXT_DEFINE_H__ */


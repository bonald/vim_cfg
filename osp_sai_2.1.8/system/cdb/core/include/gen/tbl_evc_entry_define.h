
#ifndef __TBL_EVC_ENTRY_DEFINE_H__
#define __TBL_EVC_ENTRY_DEFINE_H__

/* TBL_EVC_ENTRY field defines */
typedef enum
{
    TBL_EVC_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_EVC_ENTRY_FLD_MAPPED_SVID          = 1 ,  /* RW */
    TBL_EVC_ENTRY_FLD_MAPPED_CVID          = 2 ,  /* RW */
    TBL_EVC_ENTRY_FLD_EVC_TYPE             = 3 ,  /* RW */
    TBL_EVC_ENTRY_FLD_REF_COUNT            = 4 ,  /* RW */
    TBL_EVC_ENTRY_FLD_MAX                  = 5 
} tbl_evc_entry_field_id_t;

/* TBL_EVC_ENTRY defines */
typedef struct
{
    char                 name[GLB_VLAN_EVC_NAME_LEN+1];
} tbl_evc_entry_key_t;

typedef struct
{
    tbl_evc_entry_key_t  key;
    uint16               mapped_svid;         /* mapped svid */
    uint16               mapped_cvid;         /* mapped svid */
    uint32               evc_type;            /* evc type */
    uint32               ref_count;           /* ref count */
} tbl_evc_entry_t;

typedef struct
{
    ctclib_hash_t        *evc_entry_hash;
    ctclib_slist_t       *evc_entry_list;
} tbl_evc_entry_master_t;

#endif /* !__TBL_EVC_ENTRY_DEFINE_H__ */


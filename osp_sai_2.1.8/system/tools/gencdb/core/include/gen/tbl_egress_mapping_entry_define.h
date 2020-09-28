
#ifndef __TBL_EGRESS_MAPPING_ENTRY_DEFINE_H__
#define __TBL_EGRESS_MAPPING_ENTRY_DEFINE_H__

/* TBL_EGRESS_MAPPING_ENTRY field defines */
typedef enum
{
    TBL_EGRESS_MAPPING_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID           = 1 ,  /* RW */
    TBL_EGRESS_MAPPING_ENTRY_FLD_MAX                  = 2 
} tbl_egress_mapping_entry_field_id_t;

/* TBL_EGRESS_MAPPING_ENTRY defines */
typedef egress_mapping_entry_key_t tbl_egress_mapping_entry_key_t;

typedef struct
{
    tbl_egress_mapping_entry_key_t key;
    uint16               mapped_vid;          /* mapped vlan id */
} tbl_egress_mapping_entry_t;

typedef struct
{
    ctclib_hash_t        *egress_mapping_entry_hash;
    ctclib_slist_t       *egress_mapping_entry_list;
} tbl_egress_mapping_entry_master_t;

#endif /* !__TBL_EGRESS_MAPPING_ENTRY_DEFINE_H__ */


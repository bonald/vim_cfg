
#ifndef __TBL_VLAN_MAPPING_ENTRY_DEFINE_H__
#define __TBL_VLAN_MAPPING_ENTRY_DEFINE_H__

/* TBL_VLAN_MAPPING_ENTRY field defines */
typedef enum
{
    TBL_VLAN_MAPPING_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID          = 1 ,  /* RW */
    TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID          = 2 ,  /* RW */
    TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY            = 3 ,  /* RW */
    TBL_VLAN_MAPPING_ENTRY_FLD_MAX                  = 4 
} tbl_vlan_mapping_entry_field_id_t;

/* TBL_VLAN_MAPPING_ENTRY defines */
typedef vlan_mapping_entry_key_t tbl_vlan_mapping_entry_key_t;

typedef struct
{
    tbl_vlan_mapping_entry_key_t key;
    uint16               mapped_svid;         /* mapped svid */
    uint16               mapped_cvid;         /* mapped cvid */
    char                 evc_entry[GLB_VLAN_EVC_NAME_LEN+1];
} tbl_vlan_mapping_entry_t;

typedef struct
{
    ctclib_hash_t        *vlan_mapping_entry_hash;
    ctclib_slist_t       *vlan_mapping_entry_list;
} tbl_vlan_mapping_entry_master_t;

#endif /* !__TBL_VLAN_MAPPING_ENTRY_DEFINE_H__ */


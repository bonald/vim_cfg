
#ifndef __TBL_VLAN_MAPPING_TABLE_DEFINE_H__
#define __TBL_VLAN_MAPPING_TABLE_DEFINE_H__

/* TBL_VLAN_MAPPING_TABLE field defines */
typedef enum
{
    TBL_VLAN_MAPPING_TABLE_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS        = 1 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM            = 2 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM     = 3 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID             = 4 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID    = 5 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME = 6 ,  /* RW */
    TBL_VLAN_MAPPING_TABLE_FLD_MAX                  = 7 
} tbl_vlan_mapping_table_field_id_t;

/* TBL_VLAN_MAPPING_TABLE defines */
typedef struct
{
    char                 name[GLB_VLAN_MAPPING_TAB_NAME_MAX+1];
} tbl_vlan_mapping_table_key_t;

typedef struct
{
    tbl_vlan_mapping_table_key_t key;
    port_bmp_t           applied_ports;       /* ports which applied the mapping table */
    uint32               entry_num;           /* mapping entry numble */
    uint32               egress_entry_num;    /* egress mapping entry numble */
    uint32               group_id;            /* raw vlan group id ,0 is not set */
    uint16               group_mapped_svid;   /* group mapped svid */
    char                 group_evc_entry_name[GLB_VLAN_EVC_NAME_LEN+1];
} tbl_vlan_mapping_table_t;

typedef struct
{
    ctclib_hash_t        *vlan_mapping_table_hash;
    ctclib_slist_t       *vlan_mapping_table_list;
} tbl_vlan_mapping_table_master_t;

#endif /* !__TBL_VLAN_MAPPING_TABLE_DEFINE_H__ */


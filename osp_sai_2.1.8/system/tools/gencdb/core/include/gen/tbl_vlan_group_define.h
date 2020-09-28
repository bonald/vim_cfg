
#ifndef __TBL_VLAN_GROUP_DEFINE_H__
#define __TBL_VLAN_GROUP_DEFINE_H__

/* TBL_VLAN_GROUP field defines */
typedef enum
{
    TBL_VLAN_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLAN_GROUP_FLD_VALID                = 1 ,  /* RW */
    TBL_VLAN_GROUP_FLD_MIN_VID              = 2 ,  /* RW */
    TBL_VLAN_GROUP_FLD_MAX_VID              = 3 ,  /* RW */
    TBL_VLAN_GROUP_FLD_MAX                  = 4 
} tbl_vlan_group_field_id_t;

/* TBL_VLAN_GROUP defines */
typedef struct
{
    uint32               group_id;
} tbl_vlan_group_key_t;

typedef struct
{
    tbl_vlan_group_key_t key;
    uint32               valid[GLB_VLAN_RANGE_ENTRY_NUM+1];
    uint32               min_vid[GLB_VLAN_RANGE_ENTRY_NUM+1];
    uint32               max_vid[GLB_VLAN_RANGE_ENTRY_NUM+1];
} tbl_vlan_group_t;

typedef struct
{
    tbl_vlan_group_t     *vlan_group_array[GLB_VLAN_RANGE_GROUP_NUM+1];
} tbl_vlan_group_master_t;

#endif /* !__TBL_VLAN_GROUP_DEFINE_H__ */


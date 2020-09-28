
#ifndef __TBL_VLANCLASS_GROUP_DEFINE_H__
#define __TBL_VLANCLASS_GROUP_DEFINE_H__

/* TBL_VLANCLASS_GROUP field defines */
typedef enum
{
    TBL_VLANCLASS_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLANCLASS_GROUP_FLD_RULE_ID              = 1 ,  /* RW */
    TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID       = 2 ,  /* RW */
    TBL_VLANCLASS_GROUP_FLD_GROUP_OID            = 3 ,  /* RW */
    TBL_VLANCLASS_GROUP_FLD_MAX                  = 4 
} tbl_vlanclass_group_field_id_t;

/* TBL_VLANCLASS_GROUP defines */
typedef struct
{
    uint32               group_id;
} tbl_vlanclass_group_key_t;

typedef struct
{
    tbl_vlanclass_group_key_t key;
    vclass_bmp_t         rule_id;             /*the bitmap of vlanclass rule id*/
    uint64               action_rule_id;      /*the action of add or delete rule id*/
    uint64               group_oid;           /*group object id*/
} tbl_vlanclass_group_t;

typedef struct
{
    tbl_vlanclass_group_t *vlanclass_group_array[GLB_VLANCLASS_GROUP_ID_MAX+1];
} tbl_vlanclass_group_master_t;

#endif /* !__TBL_VLANCLASS_GROUP_DEFINE_H__ */



#ifndef __TBL_TAP_GROUP_INGRESS_FLOW_DEFINE_H__
#define __TBL_TAP_GROUP_INGRESS_FLOW_DEFINE_H__

/* TBL_TAP_GROUP_INGRESS_FLOW field defines */
typedef enum
{
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_KEY                  = 0 ,  /* READ */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID           = 1 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX               = 2 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN         = 3 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN     = 4 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID    = 5 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID = 6 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLOW_FLD_MAX                  = 7 
} tbl_tap_group_ingress_flow_field_id_t;

/* TBL_TAP_GROUP_INGRESS_FLOW defines */
typedef tap_group_ingress_flow_key_t tbl_tap_group_ingress_flow_key_t;

typedef struct
{
    tbl_tap_group_ingress_flow_key_t key;
    uint32               tap_grp_id;
    uint32               if_idx;
    uint32               ing_untag_en;
    uint32               ing_mark_vlan_en;
    uint32               ing_mark_vlan_vid;
    uint64               ing_tap_group_member_oid; /* tap_group_member_oid id for ASIC mapping*/
} tbl_tap_group_ingress_flow_t;

typedef struct
{
    ctclib_hash_t        *tap_group_ingress_hash;
    ctclib_slist_t       *tap_group_ingress_list;
} tbl_tap_group_ingress_flow_master_t;

#endif /* !__TBL_TAP_GROUP_INGRESS_FLOW_DEFINE_H__ */


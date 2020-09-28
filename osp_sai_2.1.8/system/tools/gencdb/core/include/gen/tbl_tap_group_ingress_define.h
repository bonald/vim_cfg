
#ifndef __TBL_TAP_GROUP_INGRESS_DEFINE_H__
#define __TBL_TAP_GROUP_INGRESS_DEFINE_H__

/* TBL_TAP_GROUP_INGRESS field defines */
typedef enum
{
    TBL_TAP_GROUP_INGRESS_FLD_KEY                  = 0 ,  /* READ */
    TBL_TAP_GROUP_INGRESS_FLD_ING_MARK_VLAN_EN     = 1 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_MARK_VLAN_VID    = 2 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_UNTAG_EN         = 3 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_TRUNCTION_EN     = 4 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_TAP_GROUP_MEMBER_OID = 5 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_DEST_MAC_EN = 6 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_DEST_MAC    = 7 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_SRC_MAC_EN  = 8 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_SRC_MAC     = 9 ,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_IPDA_EN     = 10,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_IPDA        = 11,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_IPSA_EN     = 12,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_IPSA        = 13,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_VLAN_EN     = 14,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_ING_EDIT_VLAN        = 15,  /* RW */
    TBL_TAP_GROUP_INGRESS_FLD_MAX                  = 16
} tbl_tap_group_ingress_field_id_t;

/* TBL_TAP_GROUP_INGRESS defines */
typedef tap_group_ingress_key_t tbl_tap_group_ingress_key_t;

typedef struct
{
    tbl_tap_group_ingress_key_t key;
    uint32               ing_mark_vlan_en;
    uint32               ing_mark_vlan_vid;
    uint32               ing_untag_en;
    uint32               ing_trunction_en;
    uint64               ing_tap_group_member_oid; /* tap_group_member_oid id for ASIC mapping*/
    uint32               ing_edit_dest_mac_en;
    mac_addr_t           ing_edit_dest_mac;
    uint32               ing_edit_src_mac_en;
    mac_addr_t           ing_edit_src_mac;
    uint32               ing_edit_ipda_en;
    addr_t               ing_edit_ipda;       /* ip address v4/v6 */
    uint32               ing_edit_ipsa_en;
    addr_t               ing_edit_ipsa;       /* ip address v4/v6 */
    uint32               ing_edit_vlan_en;
    uint16               ing_edit_vlan;
} tbl_tap_group_ingress_t;

typedef struct
{
    ctclib_hash_t        *tap_group_ingress_hash;
    ctclib_slist_t       *tap_group_ingress_list;
} tbl_tap_group_ingress_master_t;

#endif /* !__TBL_TAP_GROUP_INGRESS_DEFINE_H__ */


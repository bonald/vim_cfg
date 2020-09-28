
#ifndef __STBL_VLAN_CLASS_DEFINE_H__
#define __STBL_VLAN_CLASS_DEFINE_H__

/* STBL_VLAN_CLASS field defines */
typedef enum
{
    STBL_VLAN_CLASS_FLD_KEY                  = 0 ,  /* READ */
    STBL_VLAN_CLASS_FLD_GROUP_OID            = 1 ,  /* READ */
    STBL_VLAN_CLASS_FLD_MEMBER_OID           = 2 ,  /* READ */
    STBL_VLAN_CLASS_FLD_ENTRY_TYPE           = 3 ,  /* READ */
    STBL_VLAN_CLASS_FLD_PROTO_TYPE           = 4 ,  /* READ */
    STBL_VLAN_CLASS_FLD_VLAN_ID              = 5 ,  /* READ */
    STBL_VLAN_CLASS_FLD_IPADDR               = 6 ,  /* READ */
    STBL_VLAN_CLASS_FLD_MAC                  = 7 ,  /* READ */
    STBL_VLAN_CLASS_FLD_MAX                  = 8 
} stbl_vlan_class_field_id_t;

/* STBL_VLAN_CLASS defines */
typedef struct
{
    uint32               rule_id;
} stbl_vlan_class_key_t;

typedef struct
{
    stbl_vlan_class_key_t key;
    sai_object_id_t      group_oid;
    sai_object_id_t      member_oid;
    uint32               entry_type;          /* sai_vlanclass_entry_type_t */
    uint32               proto_type;          /* sai_vlanclass_protocol_type_t */
    vid_t                vlan_id;
    addr_t               ipaddr;
    mac_addr_t           mac;
} stbl_vlan_class_t;

typedef struct
{
    stbl_vlan_class_t    *array[GLB_VLANCLASS_RULE_ID_MAX];
} stbl_vlan_class_master_t;

#endif /* !__STBL_VLAN_CLASS_DEFINE_H__ */


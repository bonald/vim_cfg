
#ifndef __STBL_ACL_ENTRY_DEFINE_H__
#define __STBL_ACL_ENTRY_DEFINE_H__

/* STBL_ACL_ENTRY field defines */
typedef enum
{
    STBL_ACL_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    STBL_ACL_ENTRY_FLD_TBL_OID              = 1 ,  /* READ */
    STBL_ACL_ENTRY_FLD_MIRROR_OID           = 2 ,  /* READ */
    STBL_ACL_ENTRY_FLD_PRIORITY             = 3 ,  /* READ */
    STBL_ACL_ENTRY_FLD_DIRCTION             = 4 ,  /* READ */
    STBL_ACL_ENTRY_FLD_ENABLE               = 5 ,  /* READ */
    STBL_ACL_ENTRY_FLD_APPLY_INNER_LOOKUP   = 6 ,  /* READ */
    STBL_ACL_ENTRY_FLD_IN_PORT              = 7 ,  /* READ */
    STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_LIST    = 8 ,  /* READ */
    STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_COUNT   = 9 ,  /* READ */
    STBL_ACL_ENTRY_FLD_PORT_OID_LIST        = 10,  /* READ */
    STBL_ACL_ENTRY_FLD_PORT_OID_COUNT       = 11,  /* READ */
    STBL_ACL_ENTRY_FLD_OUTER_FIELDS         = 12,  /* READ */
    STBL_ACL_ENTRY_FLD_INNER_FIELDS         = 13,  /* READ */
    STBL_ACL_ENTRY_FLD_ACTION               = 14,  /* READ */
    STBL_ACL_ENTRY_FLD_ACTION_ATTR          = 15,  /* READ */
    STBL_ACL_ENTRY_FLD_TAP                  = 16,  /* READ */
    STBL_ACL_ENTRY_FLD_MAX                  = 17
} stbl_acl_entry_field_id_t;

/* STBL_ACL_ENTRY defines */
typedef struct
{
    uint32               entry_id;
} stbl_acl_entry_key_t;

typedef struct
{
    stbl_acl_entry_key_t key;
    sai_object_id_t      tbl_oid;
    sai_object_id_t      mirror_oid;
    uint32               priority;
    uint32               dirction;
    uint32               enable;
    uint32               apply_inner_lookup;
    sai_acl_field_data_t in_port;
    uint32               ctc_entry_id_list[64];
    uint32               ctc_entry_id_count;
    uint64               port_oid_list[64];
    uint32               port_oid_count;
    ctc_sai_acl_field_list_t outer_fields;
    ctc_sai_acl_field_list_t inner_fields;
    ctc_sai_acl_action_list_t action;
    ctc_sai_acl_action_attrlist_t action_attr;
    ctc_sai_acl_tap_t    tap;
} stbl_acl_entry_t;

typedef struct
{
    stbl_acl_entry_t     *array[GLB_SAI_ACE_MAX];
} stbl_acl_entry_master_t;

#endif /* !__STBL_ACL_ENTRY_DEFINE_H__ */


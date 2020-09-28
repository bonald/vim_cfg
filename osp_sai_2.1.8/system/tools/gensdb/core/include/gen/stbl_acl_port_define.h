
#ifndef __STBL_ACL_PORT_DEFINE_H__
#define __STBL_ACL_PORT_DEFINE_H__

/* STBL_ACL_PORT field defines */
typedef enum
{
    STBL_ACL_PORT_FLD_KEY                  = 0 ,  /* READ */
    STBL_ACL_PORT_FLD_INGRESS_EN_CNT       = 1 ,  /* READ */
    STBL_ACL_PORT_FLD_EGRESS_EN_CNT        = 2 ,  /* READ */
    STBL_ACL_PORT_FLD_MAX                  = 3 
} stbl_acl_port_field_id_t;

/* STBL_ACL_PORT defines */
typedef struct
{
    sai_object_id_t      port_oid;
} stbl_acl_port_key_t;

typedef struct
{
    stbl_acl_port_key_t  key;
    uint32               ingress_en_cnt;
    uint32               egress_en_cnt;
} stbl_acl_port_t;

typedef struct
{
    ctclib_slist_t       *port_list;
} stbl_acl_port_master_t;

#endif /* !__STBL_ACL_PORT_DEFINE_H__ */


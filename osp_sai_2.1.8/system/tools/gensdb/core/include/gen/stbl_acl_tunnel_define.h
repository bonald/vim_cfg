
#ifndef __STBL_ACL_TUNNEL_DEFINE_H__
#define __STBL_ACL_TUNNEL_DEFINE_H__

/* STBL_ACL_TUNNEL field defines */
typedef enum
{
    STBL_ACL_TUNNEL_FLD_KEY                  = 0 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_COUNT                = 1 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_ENTRY_ID             = 2 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_ID    = 3 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_GROUP_ID = 4 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_ERSPAN_NEXTHOP_ID    = 5 ,  /* READ */
    STBL_ACL_TUNNEL_FLD_MAX                  = 6 
} stbl_acl_tunnel_field_id_t;

/* STBL_ACL_TUNNEL defines */
typedef ctc_sai_acl_tunnel_key_t stbl_acl_tunnel_key_t;

typedef struct
{
    stbl_acl_tunnel_key_t key;
    int32                count;
    uint32               entry_id;
    uint32               erspan_service_id;
    uint32               erspan_service_group_id; /* base(10000) + erspan_service_id */
    uint32               erspan_nexthop_id;
} stbl_acl_tunnel_t;

typedef struct
{
    ctclib_hash_t        *tunnel_hash;
} stbl_acl_tunnel_master_t;

#endif /* !__STBL_ACL_TUNNEL_DEFINE_H__ */


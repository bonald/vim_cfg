
#ifndef __STBL_ROUTE_DEFINE_H__
#define __STBL_ROUTE_DEFINE_H__

/* STBL_ROUTE field defines */
typedef enum
{
    STBL_ROUTE_FLD_KEY                  = 0 ,  /* READ */
    STBL_ROUTE_FLD_RIF_OID              = 1 ,  /* READ */
    STBL_ROUTE_FLD_NH_OID               = 2 ,  /* READ */
    STBL_ROUTE_FLD_PRIORITY             = 3 ,  /* READ */
    STBL_ROUTE_FLD_ACTION               = 4 ,  /* READ */
    STBL_ROUTE_FLD_L3IF_TYPE            = 5 ,  /* READ */
    STBL_ROUTE_FLD_CATEGORY_ID          = 6 ,  /* READ */
    STBL_ROUTE_FLD_MAX                  = 7 
} stbl_route_field_id_t;

/* STBL_ROUTE defines */
typedef sai_unicast_route_entry_t stbl_route_key_t;

typedef struct
{
    stbl_route_key_t     key;
    sai_object_id_t      rif_oid;
    sai_object_id_t      nh_oid;
    uint32               priority;
    uint32               action;              /* refer to sai_packet_action_t */
    uint32               l3if_type;           /* refer to sai_router_interface_nat_type_t*/
    uint32               category_id;         /* 1~254, bigger than 255 or equals to 0 is invalid*/
} stbl_route_t;

typedef struct
{
    ctclib_hash_t        *route_hash;
} stbl_route_master_t;

#endif /* !__STBL_ROUTE_DEFINE_H__ */



#ifndef __STBL_NEIGHBOR_DEFINE_H__
#define __STBL_NEIGHBOR_DEFINE_H__

/* STBL_NEIGHBOR field defines */
typedef enum
{
    STBL_NEIGHBOR_FLD_KEY                  = 0 ,  /* READ */
    STBL_NEIGHBOR_FLD_MAC                  = 1 ,  /* READ */
    STBL_NEIGHBOR_FLD_PORT_OID             = 2 ,  /* READ */
    STBL_NEIGHBOR_FLD_NH_OID               = 3 ,  /* READ */
    STBL_NEIGHBOR_FLD_ACTION               = 4 ,  /* READ */
    STBL_NEIGHBOR_FLD_MAX                  = 5 
} stbl_neighbor_field_id_t;

/* STBL_NEIGHBOR defines */
typedef sai_neighbor_entry_t stbl_neighbor_key_t;

typedef struct
{
    stbl_neighbor_key_t  key;
    mac_addr_t           mac;
    sai_object_id_t      port_oid;
    sai_object_id_t      nh_oid;
    uint32               action;              /* sai_packet_action_t */
} stbl_neighbor_t;

typedef struct
{
    ctclib_hash_t        *neigh_hash;
    ctclib_slist_t       *neigh_list;
} stbl_neighbor_master_t;

#endif /* !__STBL_NEIGHBOR_DEFINE_H__ */


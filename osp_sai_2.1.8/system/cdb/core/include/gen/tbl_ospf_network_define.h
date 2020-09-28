
#ifndef __TBL_OSPF_NETWORK_DEFINE_H__
#define __TBL_OSPF_NETWORK_DEFINE_H__

/* TBL_OSPF_NETWORK field defines */
typedef enum
{
    TBL_OSPF_NETWORK_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF_NETWORK_FLD_AREA_ID              = 1 ,  /* READ */
    TBL_OSPF_NETWORK_FLD_MAX                  = 2 
} tbl_ospf_network_field_id_t;

/* TBL_OSPF_NETWORK defines */
typedef prefix_t tbl_ospf_network_key_t;

typedef struct
{
    tbl_ospf_network_key_t key;
    uint32               area_id;
} tbl_ospf_network_t;

typedef struct
{
    ctclib_hash_t        *ospf_network_hash;
    ctclib_slist_t       *ospf_network_list;
} tbl_ospf_network_master_t;

#endif /* !__TBL_OSPF_NETWORK_DEFINE_H__ */


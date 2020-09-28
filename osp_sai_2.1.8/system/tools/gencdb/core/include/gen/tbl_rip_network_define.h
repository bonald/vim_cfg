
#ifndef __TBL_RIP_NETWORK_DEFINE_H__
#define __TBL_RIP_NETWORK_DEFINE_H__

/* TBL_RIP_NETWORK field defines */
typedef enum
{
    TBL_RIP_NETWORK_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIP_NETWORK_FLD_MAX                  = 1 
} tbl_rip_network_field_id_t;

/* TBL_RIP_NETWORK defines */
typedef prefix_t tbl_rip_network_key_t;

typedef struct
{
    tbl_rip_network_key_t key;
} tbl_rip_network_t;

typedef struct
{
    ctclib_hash_t        *rip_network_hash;
    ctclib_slist_t       *rip_network_list;
} tbl_rip_network_master_t;

#endif /* !__TBL_RIP_NETWORK_DEFINE_H__ */


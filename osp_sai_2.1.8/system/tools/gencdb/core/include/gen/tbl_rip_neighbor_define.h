
#ifndef __TBL_RIP_NEIGHBOR_DEFINE_H__
#define __TBL_RIP_NEIGHBOR_DEFINE_H__

/* TBL_RIP_NEIGHBOR field defines */
typedef enum
{
    TBL_RIP_NEIGHBOR_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIP_NEIGHBOR_FLD_MAX                  = 1 
} tbl_rip_neighbor_field_id_t;

/* TBL_RIP_NEIGHBOR defines */
typedef addr_t tbl_rip_neighbor_key_t;

typedef struct
{
    tbl_rip_neighbor_key_t key;
} tbl_rip_neighbor_t;

typedef struct
{
    ctclib_hash_t        *rip_neighbor_hash;
    ctclib_slist_t       *rip_neighbor_list;
} tbl_rip_neighbor_master_t;

#endif /* !__TBL_RIP_NEIGHBOR_DEFINE_H__ */


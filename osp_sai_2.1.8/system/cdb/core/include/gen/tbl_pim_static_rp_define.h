
#ifndef __TBL_PIM_STATIC_RP_DEFINE_H__
#define __TBL_PIM_STATIC_RP_DEFINE_H__

/* TBL_PIM_STATIC_RP field defines */
typedef enum
{
    TBL_PIM_STATIC_RP_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_STATIC_RP_FLD_RP_ADDR              = 1 ,  /* RW */
    TBL_PIM_STATIC_RP_FLD_MAX                  = 2 
} tbl_pim_static_rp_field_id_t;

/* TBL_PIM_STATIC_RP defines */
typedef struct
{
    prefix_t             group_range;
} tbl_pim_static_rp_key_t;

typedef struct
{
    tbl_pim_static_rp_key_t key;
    addr_ipv4_t          rp_addr;
} tbl_pim_static_rp_t;

typedef struct
{
    ctclib_hash_t        *pim_static_rp_hash;
    ctclib_slist_t       *pim_static_rp_list;
} tbl_pim_static_rp_master_t;

#endif /* !__TBL_PIM_STATIC_RP_DEFINE_H__ */


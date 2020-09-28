
#ifndef __TBL_PIM_SM_DEFINE_H__
#define __TBL_PIM_SM_DEFINE_H__

/* TBL_PIM_SM field defines */
typedef enum
{
    TBL_PIM_SM_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_SM_FLD_RP_ADDR              = 1 ,  /* RW */
    TBL_PIM_SM_FLD_HELLO_TIME           = 2 ,  /* RW */
    TBL_PIM_SM_FLD_SPT_INFINITY         = 3 ,  /* RW */
    TBL_PIM_SM_FLD_JP_TIMER             = 4 ,  /* RW */
    TBL_PIM_SM_FLD_MAX                  = 5 
} tbl_pim_sm_field_id_t;

/* TBL_PIM_SM defines */
typedef struct
{
    uint32               id;
} tbl_pim_sm_key_t;

typedef struct
{
    tbl_pim_sm_key_t     key;
    pim_static_rp_addr_t rp_addr;
    uint32               hello_time;
    uint8                spt_infinity;
    uint32               jp_timer;
} tbl_pim_sm_t;

typedef struct
{
    ctclib_hash_t        *pim_sm_hash;
    ctclib_slist_t       *pim_sm_list;
} tbl_pim_sm_master_t;

#endif /* !__TBL_PIM_SM_DEFINE_H__ */


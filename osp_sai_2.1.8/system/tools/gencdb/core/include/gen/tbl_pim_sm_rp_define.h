
#ifndef __TBL_PIM_SM_RP_DEFINE_H__
#define __TBL_PIM_SM_RP_DEFINE_H__

/* TBL_PIM_SM_RP field defines */
typedef enum
{
    TBL_PIM_SM_RP_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_SM_RP_FLD_PRIORITY             = 1 ,  /* RW */
    TBL_PIM_SM_RP_FLD_INTERVAL             = 2 ,  /* RW */
    TBL_PIM_SM_RP_FLD_MAX                  = 3 
} tbl_pim_sm_rp_field_id_t;

/* TBL_PIM_SM_RP defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_pim_sm_rp_key_t;

typedef struct
{
    tbl_pim_sm_rp_key_t  key;
    uint32               priority;
    uint32               interval;
} tbl_pim_sm_rp_t;

typedef struct
{
    ctclib_hash_t        *pim_sm_rp_hash;
    ctclib_slist_t       *pim_sm_rp_list;
} tbl_pim_sm_rp_master_t;

#endif /* !__TBL_PIM_SM_RP_DEFINE_H__ */


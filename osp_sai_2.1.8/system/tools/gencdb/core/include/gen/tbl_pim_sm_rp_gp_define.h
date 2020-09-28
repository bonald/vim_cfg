
#ifndef __TBL_PIM_SM_RP_GP_DEFINE_H__
#define __TBL_PIM_SM_RP_GP_DEFINE_H__

/* TBL_PIM_SM_RP_GP field defines */
typedef enum
{
    TBL_PIM_SM_RP_GP_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX         = 1 ,  /* RW */
    TBL_PIM_SM_RP_GP_FLD_MAX                  = 2 
} tbl_pim_sm_rp_gp_field_id_t;

/* TBL_PIM_SM_RP_GP defines */
typedef struct
{
    uint32               id;
} tbl_pim_sm_rp_gp_key_t;

typedef struct
{
    tbl_pim_sm_rp_gp_key_t key;
    pim_sm_gprefix_t     group_prefix;
} tbl_pim_sm_rp_gp_t;

typedef struct
{
    ctclib_hash_t        *pim_sm_rp_gp_hash;
    ctclib_slist_t       *pim_sm_rp_gp_list;
} tbl_pim_sm_rp_gp_master_t;

#endif /* !__TBL_PIM_SM_RP_GP_DEFINE_H__ */



#ifndef __TBL_FEA_G8032_RING_MC_GROUP_DEFINE_H__
#define __TBL_FEA_G8032_RING_MC_GROUP_DEFINE_H__

/* TBL_FEA_G8032_RING_MC_GROUP field defines */
typedef enum
{
    TBL_FEA_G8032_RING_MC_GROUP_FLD_KEY                  = 0 ,  /* RW */
    TBL_FEA_G8032_RING_MC_GROUP_FLD_MC_GROUP_ID          = 1 ,  /* RW */
    TBL_FEA_G8032_RING_MC_GROUP_FLD_MAX                  = 2 
} tbl_fea_g8032_ring_mc_group_field_id_t;

/* TBL_FEA_G8032_RING_MC_GROUP defines */
typedef struct
{
    uint32               ring_id;
} tbl_fea_g8032_ring_mc_group_key_t;

typedef struct
{
    tbl_fea_g8032_ring_mc_group_key_t key;
    uint32               mc_group_id;
} tbl_fea_g8032_ring_mc_group_t;

typedef struct
{
    ctclib_slist_t       *fea_g8032_list;
} tbl_fea_g8032_ring_mc_group_master_t;

#endif /* !__TBL_FEA_G8032_RING_MC_GROUP_DEFINE_H__ */


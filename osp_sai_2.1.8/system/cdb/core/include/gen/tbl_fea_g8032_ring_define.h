
#ifndef __TBL_FEA_G8032_RING_DEFINE_H__
#define __TBL_FEA_G8032_RING_DEFINE_H__

/* TBL_FEA_G8032_RING field defines */
typedef enum
{
    TBL_FEA_G8032_RING_FLD_KEY                  = 0 ,  /* RW */
    TBL_FEA_G8032_RING_FLD_MC_GROUP_ID          = 1 ,  /* RW */
    TBL_FEA_G8032_RING_FLD_APS_GROUP_ID         = 2 ,  /* RW */
    TBL_FEA_G8032_RING_FLD_NH_ID                = 3 ,  /* RW */
    TBL_FEA_G8032_RING_FLD_NHOFFSET             = 4 ,  /* RW */
    TBL_FEA_G8032_RING_FLD_MAX                  = 5 
} tbl_fea_g8032_ring_field_id_t;

/* TBL_FEA_G8032_RING defines */
typedef fea_g8032_ring_key_t tbl_fea_g8032_ring_key_t;

typedef struct
{
    tbl_fea_g8032_ring_key_t key;
    uint32               mc_group_id;
    uint32               aps_group_id;
    uint32               nh_id;
    uint32               nhoffset;
} tbl_fea_g8032_ring_t;

typedef struct
{
    ctclib_slist_t       *fea_g8032_list;
} tbl_fea_g8032_ring_master_t;

#endif /* !__TBL_FEA_G8032_RING_DEFINE_H__ */


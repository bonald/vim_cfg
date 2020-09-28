
#ifndef __TBL_PIM_SM_BSR_DEFINE_H__
#define __TBL_PIM_SM_BSR_DEFINE_H__

/* TBL_PIM_SM_BSR field defines */
typedef enum
{
    TBL_PIM_SM_BSR_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_SM_BSR_FLD_PRIORITY             = 1 ,  /* RW */
    TBL_PIM_SM_BSR_FLD_INTERVAL             = 2 ,  /* RW */
    TBL_PIM_SM_BSR_FLD_DR_PRI               = 3 ,  /* RW */
    TBL_PIM_SM_BSR_FLD_MAX                  = 4 
} tbl_pim_sm_bsr_field_id_t;

/* TBL_PIM_SM_BSR defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_pim_sm_bsr_key_t;

typedef struct
{
    tbl_pim_sm_bsr_key_t key;
    uint32               priority;
    uint32               interval;
    uint32               dr_pri;
} tbl_pim_sm_bsr_t;

typedef struct
{
    ctclib_hash_t        *pim_sm_bsr_hash;
    ctclib_slist_t       *pim_sm_bsr_list;
} tbl_pim_sm_bsr_master_t;

#endif /* !__TBL_PIM_SM_BSR_DEFINE_H__ */



#ifndef __TBL_PIM_DM_DEFINE_H__
#define __TBL_PIM_DM_DEFINE_H__

/* TBL_PIM_DM field defines */
typedef enum
{
    TBL_PIM_DM_FLD_KEY                  = 0 ,  /* READ */
    TBL_PIM_DM_FLD_MAX                  = 1 
} tbl_pim_dm_field_id_t;

/* TBL_PIM_DM defines */
typedef struct
{
    uint32               id;
} tbl_pim_dm_key_t;

typedef struct
{
    tbl_pim_dm_key_t     key;
} tbl_pim_dm_t;

typedef struct
{
    ctclib_hash_t        *pim_dm_hash;
    ctclib_slist_t       *pim_dm_list;
} tbl_pim_dm_master_t;

#endif /* !__TBL_PIM_DM_DEFINE_H__ */


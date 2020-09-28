
#ifndef __TBL_CFM_MAID_DEFINE_H__
#define __TBL_CFM_MAID_DEFINE_H__

/* TBL_CFM_MAID field defines */
typedef enum
{
    TBL_CFM_MAID_FLD_KEY                  = 0 ,  /* READ */
    TBL_CFM_MAID_FLD_MAX                  = 1 
} tbl_cfm_maid_field_id_t;

/* TBL_CFM_MAID defines */
typedef struct
{
    char                 name[CFM_MAID_MAX_LENGTH+1];
} tbl_cfm_maid_key_t;

typedef struct
{
    tbl_cfm_maid_key_t   key;
} tbl_cfm_maid_t;

typedef struct
{
    ctclib_hash_t        *cfm_maid_hash;
    ctclib_slist_t       *cfm_maid_list;
} tbl_cfm_maid_master_t;

#endif /* !__TBL_CFM_MAID_DEFINE_H__ */


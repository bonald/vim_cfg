
#ifndef __TBL_L2EDIT_DEFINE_H__
#define __TBL_L2EDIT_DEFINE_H__

/* TBL_L2EDIT field defines */
typedef enum
{
    TBL_L2EDIT_FLD_KEY                  = 0 ,  /* READ */
    TBL_L2EDIT_FLD_COUNT                = 1 ,  /* RW */
    TBL_L2EDIT_FLD_MAX                  = 2 
} tbl_l2edit_field_id_t;

/* TBL_L2EDIT defines */
typedef l2edit_key_t tbl_l2edit_key_t;

typedef struct
{
    tbl_l2edit_key_t     key;
    uint32               count;
} tbl_l2edit_t;

typedef struct
{
    ctclib_hash_t        *l2edit_hash;
} tbl_l2edit_master_t;

#endif /* !__TBL_L2EDIT_DEFINE_H__ */


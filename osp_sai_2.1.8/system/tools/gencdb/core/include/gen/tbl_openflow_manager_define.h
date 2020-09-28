
#ifndef __TBL_OPENFLOW_MANAGER_DEFINE_H__
#define __TBL_OPENFLOW_MANAGER_DEFINE_H__

/* TBL_OPENFLOW_MANAGER field defines */
typedef enum
{
    TBL_OPENFLOW_MANAGER_FLD_KEY                  = 0 ,  /* READ */
    TBL_OPENFLOW_MANAGER_FLD_MODE                 = 1 ,  /* RW */
    TBL_OPENFLOW_MANAGER_FLD_IS_INBAND            = 2 ,  /* RW */
    TBL_OPENFLOW_MANAGER_FLD_MAX                  = 3 
} tbl_openflow_manager_field_id_t;

/* TBL_OPENFLOW_MANAGER defines */
typedef openflow_manager_key_t tbl_openflow_manager_key_t;

typedef struct
{
    tbl_openflow_manager_key_t key;
    uint32               mode;                /* passive or active, refrence to openflow_manager_mode_t */
    uint32               is_inband;           /* only used by active */
} tbl_openflow_manager_t;

typedef struct
{
    ctclib_hash_t        *openflow_manager_hash;
    ctclib_slist_t       *openflow_manager_list;
} tbl_openflow_manager_master_t;

#endif /* !__TBL_OPENFLOW_MANAGER_DEFINE_H__ */


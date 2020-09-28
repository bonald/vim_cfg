
#ifndef __TBL_CONTROLLER_ATTR_DEFINE_H__
#define __TBL_CONTROLLER_ATTR_DEFINE_H__

/* TBL_CONTROLLER_ATTR field defines */
typedef enum
{
    TBL_CONTROLLER_ATTR_FLD_KEY                  = 0 ,  /* READ */
    TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME      = 1 ,  /* RW */
    TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF          = 2 ,  /* RW */
    TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE     = 3 ,  /* RW */
    TBL_CONTROLLER_ATTR_FLD_MAX                  = 4 
} tbl_controller_attr_field_id_t;

/* TBL_CONTROLLER_ATTR defines */
typedef ns_route_key_t tbl_controller_attr_key_t;

typedef struct
{
    tbl_controller_attr_key_t key;
    char                 controller_name[CONTROLLER_NAME_SIZE];
    uint32               max_backoff;
    uint32               inactivity_probe;
} tbl_controller_attr_t;

typedef struct
{
    ctclib_hash_t        *controller_attr_hash;
} tbl_controller_attr_master_t;

#endif /* !__TBL_CONTROLLER_ATTR_DEFINE_H__ */


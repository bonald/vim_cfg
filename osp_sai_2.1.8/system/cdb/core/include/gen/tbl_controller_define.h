
#ifndef __TBL_CONTROLLER_DEFINE_H__
#define __TBL_CONTROLLER_DEFINE_H__

/* TBL_CONTROLLER field defines */
typedef enum
{
    TBL_CONTROLLER_FLD_KEY                  = 0 ,  /* READ */
    TBL_CONTROLLER_FLD_IP                   = 1 ,  /* RW */
    TBL_CONTROLLER_FLD_PORT                 = 2 ,  /* RW */
    TBL_CONTROLLER_FLD_MAX                  = 3 
} tbl_controller_field_id_t;

/* TBL_CONTROLLER defines */
typedef struct
{
    char                 name[16];
} tbl_controller_key_t;

typedef struct
{
    tbl_controller_key_t key;
    addr_ipv4_t          ip;
    uint16               port;
} tbl_controller_t;

typedef struct
{
    ctclib_slist_t       *controller_list;
} tbl_controller_master_t;

#endif /* !__TBL_CONTROLLER_DEFINE_H__ */



#ifndef __TBL_AUTHEN_DEFINE_H__
#define __TBL_AUTHEN_DEFINE_H__

/* TBL_AUTHEN field defines */
typedef enum
{
    TBL_AUTHEN_FLD_KEY                  = 0 ,  /* READ */
    TBL_AUTHEN_FLD_METHODS              = 1 ,  /* RW */
    TBL_AUTHEN_FLD_MAX                  = 2 
} tbl_authen_field_id_t;

/* TBL_AUTHEN defines */
typedef struct
{
    char                 name[LINE_AUTH_METHOD_NAME_SIZE+1];
} tbl_authen_key_t;

typedef struct
{
    tbl_authen_key_t     key;
    char                 methods[LINE_AUTH_METHOD_LINE_SIZE]; /* auth method list */
} tbl_authen_t;

typedef struct
{
    ctclib_slist_t       *authen_list;
} tbl_authen_master_t;

#endif /* !__TBL_AUTHEN_DEFINE_H__ */


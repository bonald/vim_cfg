
#ifndef __TBL_ACCOUNT_DEFINE_H__
#define __TBL_ACCOUNT_DEFINE_H__

/* TBL_ACCOUNT field defines */
typedef enum
{
    TBL_ACCOUNT_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACCOUNT_FLD_START_STOP           = 1 ,  /* RW */
    TBL_ACCOUNT_FLD_STOP_ONLY            = 2 ,  /* RW */
    TBL_ACCOUNT_FLD_METHODS              = 3 ,  /* RW */
    TBL_ACCOUNT_FLD_MAX                  = 4 
} tbl_account_field_id_t;

/* TBL_ACCOUNT defines */
typedef struct
{
    char                 name[LINE_AUTH_METHOD_NAME_SIZE+1];
} tbl_account_key_t;

typedef struct
{
    tbl_account_key_t    key;
    uint32               start_stop;
    uint32               stop_only;
    char                 methods[LINE_AUTH_METHOD_LINE_SIZE]; /* auth method list */
} tbl_account_t;

typedef struct
{
    ctclib_slist_t       *account_list;
} tbl_account_master_t;

#endif /* !__TBL_ACCOUNT_DEFINE_H__ */


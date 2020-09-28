
#ifndef __TBL_ACCOUNTCMD_DEFINE_H__
#define __TBL_ACCOUNTCMD_DEFINE_H__

/* TBL_ACCOUNTCMD field defines */
typedef enum
{
    TBL_ACCOUNTCMD_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACCOUNTCMD_FLD_METHODS              = 1 ,  /* RW */
    TBL_ACCOUNTCMD_FLD_MAX                  = 2 
} tbl_accountcmd_field_id_t;

/* TBL_ACCOUNTCMD defines */
typedef struct
{
    char                 name[LINE_AUTH_METHOD_NAME_SIZE+1];
} tbl_accountcmd_key_t;

typedef struct
{
    tbl_accountcmd_key_t key;
    char                 methods[LINE_AUTH_METHOD_LINE_SIZE]; /* auth method list */
} tbl_accountcmd_t;

typedef struct
{
    ctclib_slist_t       *accountcmd_list;
} tbl_accountcmd_master_t;

#endif /* !__TBL_ACCOUNTCMD_DEFINE_H__ */


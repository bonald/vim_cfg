
#ifndef __TBL_AUTHOR_DEFINE_H__
#define __TBL_AUTHOR_DEFINE_H__

/* TBL_AUTHOR field defines */
typedef enum
{
    TBL_AUTHOR_FLD_KEY                  = 0 ,  /* READ */
    TBL_AUTHOR_FLD_METHODS              = 1 ,  /* RW */
    TBL_AUTHOR_FLD_MAX                  = 2 
} tbl_author_field_id_t;

/* TBL_AUTHOR defines */
typedef struct
{
    char                 name[LINE_AUTH_METHOD_NAME_SIZE+1];
} tbl_author_key_t;

typedef struct
{
    tbl_author_key_t     key;
    char                 methods[LINE_AUTH_METHOD_LINE_SIZE]; /* auth method list */
} tbl_author_t;

typedef struct
{
    ctclib_slist_t       *author_list;
} tbl_author_master_t;

#endif /* !__TBL_AUTHOR_DEFINE_H__ */


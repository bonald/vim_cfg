
#ifndef __TBL_ENABLE_DEFINE_H__
#define __TBL_ENABLE_DEFINE_H__

/* TBL_ENABLE field defines */
typedef enum
{
    TBL_ENABLE_FLD_KEY                  = 0 ,  /* READ */
    TBL_ENABLE_FLD_ENC_PASSWD           = 1 ,  /* RW */
    TBL_ENABLE_FLD_PASSWD               = 2 ,  /* RW */
    TBL_ENABLE_FLD_MAX                  = 3 
} tbl_enable_field_id_t;

/* TBL_ENABLE defines */
typedef struct
{
    uint8                id;
} tbl_enable_key_t;

typedef struct
{
    tbl_enable_key_t     key;
    char                 enc_passwd[PASSWD_SIZE]; /* encrypted password */
    char                 passwd[PASSWD_SIZE]; /* enable password */
} tbl_enable_t;

typedef struct
{
    tbl_enable_t         *enable_array[GLB_MAX_PRIVILEAGE_NUM];
} tbl_enable_master_t;

#endif /* !__TBL_ENABLE_DEFINE_H__ */


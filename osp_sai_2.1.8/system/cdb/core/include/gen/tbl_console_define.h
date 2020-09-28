
#ifndef __TBL_CONSOLE_DEFINE_H__
#define __TBL_CONSOLE_DEFINE_H__

/* TBL_CONSOLE field defines */
typedef enum
{
    TBL_CONSOLE_FLD_TIMEOUT_MIN          = 0 ,  /* RW */
    TBL_CONSOLE_FLD_TIMEOUT_SEC          = 1 ,  /* RW */
    TBL_CONSOLE_FLD_LOGIN                = 2 ,  /* RW */
    TBL_CONSOLE_FLD_PRIVILEGE            = 3 ,  /* RW */
    TBL_CONSOLE_FLD_ENC_PASSWD           = 4 ,  /* RW */
    TBL_CONSOLE_FLD_PASSWD               = 5 ,  /* RW */
    TBL_CONSOLE_FLD_AUTH_METHOD          = 6 ,  /* RW */
    TBL_CONSOLE_FLD_STOPBITS             = 7 ,  /* RW */
    TBL_CONSOLE_FLD_PARITY               = 8 ,  /* RW */
    TBL_CONSOLE_FLD_DATABITS             = 9 ,  /* RW */
    TBL_CONSOLE_FLD_BAUDRATE             = 10,  /* RW */
    TBL_CONSOLE_FLD_INUSE                = 11,  /* READ */
    TBL_CONSOLE_FLD_MAX                  = 12
} tbl_console_field_id_t;

/* TBL_CONSOLE defines */
typedef struct
{
    uint32               timeout_min;         /* line's timeout minutes */
    uint32               timeout_sec;         /* line's timeout seconds */
    uint8                login;               /* login method type: nologin,login,local,auth */
    uint8                privilege;           /* console's privilege <1-4> */
    char                 enc_passwd[PASSWD_SIZE]; /* encrypted line password */
    char                 passwd[PASSWD_SIZE]; /* line password */
    char                 auth_method[LINE_AUTH_METHOD_NAME_SIZE+1]; /* line auth method list name */
    uint8                stopbits;            /* 1 or 2 */
    uint8                parity;              /* even or none or odd */
    uint8                databits;            /* 7 or 8 */
    uint32               baudrate;            /* baud rate */
    uint8                inuse;               /* console currently in use flag */
} tbl_console_t;

#endif /* !__TBL_CONSOLE_DEFINE_H__ */


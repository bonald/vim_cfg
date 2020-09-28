
#ifndef __TBL_LOGIN_DEFINE_H__
#define __TBL_LOGIN_DEFINE_H__

/* TBL_LOGIN field defines */
typedef enum
{
    TBL_LOGIN_FLD_KEY                  = 0 ,  /* READ */
    TBL_LOGIN_FLD_USER                 = 1 ,  /* RW */
    TBL_LOGIN_FLD_VTY                  = 2 ,  /* READ */
    TBL_LOGIN_FLD_PRIVILEGE            = 3 ,  /* READ */
    TBL_LOGIN_FLD_IPADDR               = 4 ,  /* RW */
    TBL_LOGIN_FLD_DEST_IPADDR          = 5 ,  /* RW */
    TBL_LOGIN_FLD_REMOTE_PORT          = 6 ,  /* RW */
    TBL_LOGIN_FLD_DEST_PORT            = 7 ,  /* RW */
    TBL_LOGIN_FLD_PROTOCOL             = 8 ,  /* RW */
    TBL_LOGIN_FLD_EXPIRE_TIME          = 9 ,  /* RW */
    TBL_LOGIN_FLD_PID                  = 10,  /* RW */
    TBL_LOGIN_FLD_MAX                  = 11
} tbl_login_field_id_t;

/* TBL_LOGIN defines */
typedef struct
{
    char                 name[VTY_BUF_SIZE];
} tbl_login_key_t;

typedef struct
{
    tbl_login_key_t      key;
    user_t               user;                /* username, maybe '\0' */
    char                 vty[VTY_BUF_SIZE];   /* vty name: console/vty0/vty1/.../vty7 */
    uint8                privilege;           /* session's privilege <1-4> */
    char                 ipaddr[IPADDR_BUF_SIZE]; /* ipv4/6 address */
    char                 dest_ipaddr[IPADDR_BUF_SIZE];
    uint32               remote_port;
    uint32               dest_port;
    char                 protocol[LINE_PROTOCOL_BUF_SIZE]; /* telnet / ssh / web */
    uint32               expire_time;         /* expire time for web session */
    uint32               pid;                 /* clish pid */
} tbl_login_t;

typedef struct
{
    ctclib_slist_t       *login_list;
} tbl_login_master_t;

#endif /* !__TBL_LOGIN_DEFINE_H__ */


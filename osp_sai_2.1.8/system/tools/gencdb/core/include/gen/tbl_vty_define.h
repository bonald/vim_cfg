
#ifndef __TBL_VTY_DEFINE_H__
#define __TBL_VTY_DEFINE_H__

/* TBL_VTY field defines */
typedef enum
{
    TBL_VTY_FLD_KEY                  = 0 ,  /* READ */
    TBL_VTY_FLD_TIMEOUT_MIN          = 1 ,  /* RW */
    TBL_VTY_FLD_TIMEOUT_SEC          = 2 ,  /* RW */
    TBL_VTY_FLD_PROTOCOL             = 3 ,  /* RW */
    TBL_VTY_FLD_LOGIN                = 4 ,  /* RW */
    TBL_VTY_FLD_PRIVILEGE            = 5 ,  /* RW */
    TBL_VTY_FLD_ENC_PASSWD           = 6 ,  /* RW */
    TBL_VTY_FLD_PASSWD               = 7 ,  /* RW */
    TBL_VTY_FLD_AUTH_METHOD          = 8 ,  /* RW */
    TBL_VTY_FLD_INUSE                = 9 ,  /* READ */
    TBL_VTY_FLD_AUTHOR_METHOD        = 10,  /* RW */
    TBL_VTY_FLD_ACCOUNT_METHOD       = 11,  /* RW */
    TBL_VTY_FLD_ACCOUNTCMD_METHOD    = 12,  /* RW */
    TBL_VTY_FLD_IPV4_ACL             = 13,  /* RW */
    TBL_VTY_FLD_IPV6_ACL             = 14,  /* RW */
    TBL_VTY_FLD_AAA_PRIVILEGE        = 15,  /* RW */
    TBL_VTY_FLD_CONFIG_PRIVILEGE     = 16,  /* RW */
    TBL_VTY_FLD_AAA_START_TIME       = 17,  /* RW */
    TBL_VTY_FLD_CMD_BUF              = 18,  /* RW */
    TBL_VTY_FLD_OLD_CMD_BUF          = 19,  /* RW */
    TBL_VTY_FLD_ACCT_STOP_IPADDR     = 20,  /* RW */
    TBL_VTY_FLD_ACCT_STOP_PID        = 21,  /* RW */
    TBL_VTY_FLD_ACCT_STOP_PRIVILEGE  = 22,  /* RW */
    TBL_VTY_FLD_IS_NOT_SSH_KEY       = 23,  /* RW */
    TBL_VTY_FLD_IS_SSH_LOGIN         = 24,  /* RW */
    TBL_VTY_FLD_MAX                  = 25
} tbl_vty_field_id_t;

/* TBL_VTY defines */
typedef struct
{
    uint8                id;
} tbl_vty_key_t;

typedef struct
{
    tbl_vty_key_t        key;
    uint32               timeout_min;         /* line's timeout minutes */
    uint32               timeout_sec;         /* line's timeout seconds */
    char                 protocol[LINE_PROTOCOL_BUF_SIZE]; /* protocol's enabled on this line, ssh,telnet... */
    uint8                login;               /* login method type: nologin,login,local,auth */
    uint8                privilege;           /* vty's privilege <1-4> */
    char                 enc_passwd[PASSWD_SIZE]; /* encrypted line password */
    char                 passwd[PASSWD_SIZE]; /* line password */
    char                 auth_method[LINE_AUTH_METHOD_NAME_SIZE+1]; /* line auth method list name */
    uint8                inuse;               /* vty currently in use flag */
    char                 author_method[LINE_AUTH_METHOD_NAME_SIZE+1]; /* authorization method list name */
    char                 account_method[LINE_AUTH_METHOD_NAME_SIZE+1]; /* accounting method list name */
    char                 accountcmd_method[LINE_AUTH_METHOD_NAME_SIZE+1]; /* accounting commands method list name */
    char                 ipv4_acl[ACL_NAME_FULL_NAME_SIZE+1];
    char                 ipv6_acl[ACL_NAME_FULL_NAME_SIZE+1];
    uint8                aaa_privilege;
    uint8                config_privilege;
    uint32               aaa_start_time;
    uint8                cmd_buf[GLB_MAX_CMD_BUFSZ];
    uint8                old_cmd_buf[GLB_MAX_CMD_BUFSZ];
    char                 acct_stop_ipaddr[IPADDR_BUF_SIZE]; /* ipv4/6 address */
    uint32               acct_stop_pid;       /* clish pid */
    uint8                acct_stop_privilege;
    uint32               is_not_ssh_key;      /* not ssh key mode*/
    uint32               is_ssh_login;        /* is ssh login mode*/
} tbl_vty_t;

typedef struct
{
    tbl_vty_t            *vty_array[GLB_MAX_VTY_NUM];
} tbl_vty_master_t;

#endif /* !__TBL_VTY_DEFINE_H__ */



#ifndef __TBL_SSH_CFG_DEFINE_H__
#define __TBL_SSH_CFG_DEFINE_H__

/* TBL_SSH_CFG field defines */
typedef enum
{
    TBL_SSH_CFG_FLD_VERSION              = 0 ,  /* RW */
    TBL_SSH_CFG_FLD_ENABLE               = 1 ,  /* RW */
    TBL_SSH_CFG_FLD_AUTH_RETRY           = 2 ,  /* RW */
    TBL_SSH_CFG_FLD_AUTH_TIMEOUT         = 3 ,  /* RW */
    TBL_SSH_CFG_FLD_REKEY_INTERVAL       = 4 ,  /* RW */
    TBL_SSH_CFG_FLD_AUTH_TYPE            = 5 ,  /* RW */
    TBL_SSH_CFG_FLD_LISTEN_PORT          = 6 ,  /* RW */
    TBL_SSH_CFG_FLD_HOSTKEY              = 7 ,  /* RW */
    TBL_SSH_CFG_FLD_NETCONF_PORT         = 8 ,  /* RW */
    TBL_SSH_CFG_FLD_NETCONF_SERVICE      = 9 ,  /* RW */
    TBL_SSH_CFG_FLD_MAX                  = 10
} tbl_ssh_cfg_field_id_t;

/* TBL_SSH_CFG defines */
typedef struct
{
    uint8                version;
    uint8                enable;
    uint8                auth_retry;          /* authentication retry counts */
    uint8                auth_timeout;        /* authentication timeout in seconds */
    uint16               rekey_interval;      /* rekey interval in minutes */
    uint16               auth_type;           /* authentication types: SSH_AUTH_TYPE_ */
    uint16               listen_port;         /* listen port */
    char                 hostkey[M_RSA_KEY_NAMESZ+1]; /* host RSA key */
    uint16               netconf_port;        /* netconf service port */
    uint8                netconf_service;     /* netconf service on/off */
} tbl_ssh_cfg_t;

#endif /* !__TBL_SSH_CFG_DEFINE_H__ */


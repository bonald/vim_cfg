
#include <openssl/bn.h>
#include <assert.h>

#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_rsa_define.h"
#include "appcfg.h"
#include "app_ssh.h"
#include "lib_fs.h"
#include "rsa_derhex.h"

extern tbl_rsa_t *find_rsa_key(char *rsaname);
extern int32 _rsa_key_reference(tbl_rsa_t *p_rsa, int nCnt);
extern int check_rsa_key(char *rsaname, int bPrivate, int bNoPasswd, char *pszErr, size_t err_bufsz);
extern int32 _rsa_key_reference(tbl_rsa_t *p_rsa, int nCnt);

static int32
_app_ssh_cmd_get_ssh(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ssh_cfg_iterate((TBL_ITER_CB_FUNC)tbl_ssh_cfg_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_ssh_cmd_set_version(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */ 
    if (p_cfg->version >= SSH_VERSION_MAX) 
    {
        CFG_INVALID_PARAM_RET("Invalid version");
    }

    /* 2. check duplicate config */
    if (p_db_cfg->version == p_cfg->version)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_version(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_enable(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable == p_cfg->enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_enable(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_auth_retry(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */    
    if (p_cfg->auth_retry < 1 || p_cfg->auth_retry > 6) 
    {
        CFG_INVALID_PARAM_RET("Invalid authentication retries");
    }

    /* 2. check duplicate config */
    if (p_db_cfg->auth_retry == p_cfg->auth_retry)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_auth_retry(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_auth_timeout(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */    
    if (p_cfg->auth_timeout < 1 || p_cfg->auth_timeout > 120) 
    {
        CFG_INVALID_PARAM_RET("Invalid authentication timeout");
    }

    /* 2. check duplicate config */
    if (p_db_cfg->auth_timeout == p_cfg->auth_timeout)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_auth_timeout(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_rekey_interval(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */    
    if (p_cfg->rekey_interval < 1 || p_cfg->rekey_interval > 1440) 
    {
        CFG_INVALID_PARAM_RET("Invalid authentication timeout");
    }
    
    /* 2. check duplicate config */
    if (p_db_cfg->rekey_interval == p_cfg->rekey_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_rekey_interval(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_auth_type(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */    
    if ((p_cfg->auth_type & 
        (SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY | SSH_AUTH_TYPE_RSA)) == 0) {
        CFG_INVALID_PARAM_RET("Invalid authentication type %d", p_cfg->auth_type);
    }

    /* 2. check duplicate config */
    if (p_db_cfg->auth_type == p_cfg->auth_type)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_auth_type(p_cfg));
    return rc;
}

static int
_app_ssh_cmd_parse_auth_type(char *pszType) 
{
    int type = 0;
    char **ppTypeArr = NULL;
    unsigned long nCnt = 0;
    int i;

    if (!strcasecmp(pszType, "all")) {
        //type = (SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY | SSH_AUTH_TYPE_RSA);
        type = (SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY);
        return type;
    }
    
    if (split_tokens(pszType, strlen(pszType), 2, ",", &nCnt, &ppTypeArr) != 0) {
        return -1;
    }

    for (i = 0; i < nCnt; i++) {
        if (!strcasecmp(ppTypeArr[i], "password")) {
            type |= SSH_AUTH_TYPE_PASSWORD;
        } else if (!strcasecmp(ppTypeArr[i], "public-key")) {
            type |= SSH_AUTH_TYPE_PUBKEY;
        } 
        /*else if (!strcasecmp(ppTypeArr[i], "rsa")) {
            type |= SSH_AUTH_TYPE_RSA;
        }*/
        else {
            goto err_out;
        }
    }

err_out:
    free_tokens(&ppTypeArr);
    return type;
}

static int32
_app_ssh_cmd_set_hostkey(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_rsa_t *pRsa = NULL;
    tbl_rsa_t *pRsaOld = NULL;
    char szErr[256];
    char *pszkey = p_cfg->hostkey;

    if (!strcmp(p_cfg->hostkey, p_db_cfg->hostkey)) {
        return PM_E_NONE;
    }

    if ('\0' != p_cfg->hostkey[0]) {
        pRsa = find_rsa_key(pszkey);
        assert(NULL != pRsa);
        if (BN_num_bits(((RSA *)pRsa->pstRsa)->n) <= (768 + SSH_KEY_BITS_RESERVED)) {
            CFG_INVALID_PARAM_RET("SSH host RSA key length must larger than %d", 
                (768 + SSH_KEY_BITS_RESERVED));
        }
        unlink(M_SSHD_HOST_RSA_KEY_FILE);
        if (export_rsa_to_file((RSA *)pRsa->pstRsa, 1, NULL, NULL, "pem",
                                M_SSHD_HOST_RSA_KEY_FILE, szErr, 256) != 0) {
            CFG_FUNC_ERR_RET("%s", szErr)
        }

        _rsa_key_reference(pRsa, 1);
    } else {

              system("cp "M_SSHD_HOST_RSA_DEFULT_KEY_FILE" "M_SSHD_HOST_RSA_KEY_FILE);      
    }

    if ('\0' != p_db_cfg->hostkey[0]) {
        pRsaOld = find_rsa_key(p_db_cfg->hostkey);
        _rsa_key_reference(pRsaOld, -1);
    }
            
    PM_E_RETURN(app_ssh_set_hostkey(p_cfg));
    return rc;
}

static int32
_app_ssh_cmd_set_listen_port(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */    
    if (p_cfg->listen_port != 22 
        && (p_cfg->listen_port < 1025 || p_cfg->listen_port > 65535)) 
    {
        CFG_INVALID_PARAM_RET("Invalid listen port");
    }

    PM_E_RETURN(lib_cmd_check_port(p_cfg->listen_port, LIB_SERVICE_PORT_SSH, para));
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_listen_port(p_db_cfg, p_cfg));
    return rc;
}

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:20 */
static int32
_app_ssh_cmd_set_netconf_service(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_cfg->netconf_port != 830 
        && (p_cfg->netconf_port < 1025 || p_cfg->netconf_port > 65535)) 
    {
        CFG_INVALID_PARAM_RET("Invalid NETCONF port");
    }

    if (p_db_cfg->netconf_service == p_cfg->netconf_service) {
        if (p_db_cfg->netconf_port == p_cfg->netconf_port) {
            return PM_E_NONE;
        } else {
            CFG_CONFLICT_RET("Please disable NETCONF service at first");
        }
    }
    
    PM_E_RETURN(lib_cmd_check_port(p_cfg->netconf_port, LIB_SERVICE_PORT_NETCONF, para));
    
    /* 3. call API */
    PM_E_RETURN(app_ssh_set_netconf_service(p_cfg, p_db_cfg));
    return rc;
}
#endif /* !HAVE_NETCONF */

int32
app_ssh_cmd_process_ssh(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErr[256];
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    int32 value = 0;
    field_parse_t field;
    tbl_ssh_cfg_t cfg;
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SSH_CFG);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        sal_memset(&cfg, 0, sizeof(cfg));
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        if (1 == argc && !strcasecmp(argv[i], "hostkey")) {
            /* clear hostkey setting */
            cfg.hostkey[0] = '\0';
            rc = _app_ssh_cmd_set_hostkey(&cfg, p_db_cfg, para);
            return rc;
        }

        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
        switch (field_id)
        {
#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:39 */
        case TBL_SSH_CFG_FLD_NETCONF_SERVICE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.netconf_service = value;
            PM_CFG_GET_UINT(value, argv[i+2], para->p_rs);
            cfg.netconf_port = value;
            rc = _app_ssh_cmd_set_netconf_service(&cfg, p_db_cfg, para);
            break;
#endif /* !HAVE_NETCONF */        
        case TBL_SSH_CFG_FLD_VERSION:
            PM_CFG_GET_ENUM(value, ssh_version_strs, SSH_VERSION_MAX, argv[i+1], para->p_rs);
            cfg.version = value;
            rc = _app_ssh_cmd_set_version(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_ENABLE:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable = value;
            rc = _app_ssh_cmd_set_enable(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_AUTH_RETRY:
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            cfg.auth_retry = value;
            rc = _app_ssh_cmd_set_auth_retry(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_AUTH_TIMEOUT:
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            cfg.auth_timeout = value;
            rc = _app_ssh_cmd_set_auth_timeout(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_REKEY_INTERVAL:
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            cfg.rekey_interval= value;
            rc = _app_ssh_cmd_set_rekey_interval(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_AUTH_TYPE:
            if (!(value = _app_ssh_cmd_parse_auth_type(argv[i+1]))) {
                CFG_INVALID_PARAM_RET("Invalid authentication type %s", argv[i+1]);
            }
            cfg.auth_type = value;
            rc = _app_ssh_cmd_set_auth_type(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_HOSTKEY:
            if (sal_strlen(argv[i+1]) > M_RSA_KEY_NAMESZ || sal_strlen(argv[i+1]) <= 0)
            {
                CFG_INVALID_PARAM_RET("RSA key length range is [1, 32]");
            }

            if (check_rsa_key(argv[i+1], KEY_TYPE_PRIVATE, 1, szErr, 256) != 0) {
                CFG_INVALID_PARAM_RET("%s", szErr);
            }

            snprintf(cfg.hostkey, M_RSA_KEY_NAMESZ, "%s", argv[i+1]);
            rc = _app_ssh_cmd_set_hostkey(&cfg, p_db_cfg, para);
            break;

        case TBL_SSH_CFG_FLD_LISTEN_PORT:
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            cfg.listen_port = value;
            rc = _app_ssh_cmd_set_listen_port(&cfg, p_db_cfg, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_ssh_cmd_get_ssh(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


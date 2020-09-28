
#include <assert.h>

#include "proto.h"
#include <sys/dir.h>
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_enable.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_login_record_define.h"
#include "gen/tbl_login_record.h"
#include "gen/tbl_login_security_glb_define.h"
#include "gen/tbl_login_security_glb.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "appcfg.h"
#include "app_auth.h"
#include "lib_fs.h"
#include "cfg_cmd.h"
#include "app_serial.h"
#include "rsa_derhex.h"
#include <sshkey.h>
//#include <authfile.h>
#include "gen/tbl_ssh_cfg_define.h"
#include "app_ssh.h"
#include "glb_tempfile_define.h"
#include "lib_snmp.h"

#define  MAX_LINE_CHAR    50
#define  MAX_CPU_LINE_CHAR    1000
#define  IPV4_PREFIX_LEN  7

#define M_WEB_SESSION_EXPIRE_TIME   600

#define CONNTRACK "nf_conntrack"

extern tbl_cpu_utilization_master_t *_g_p_tbl_cpu_utilization_master;
extern int32 app_syslog_terminal_monitor(int onoff, char *pszTty);
extern int app_is_password_encryption();
extern int app_get_max_vty();
int check_rsa_key(char *rsaname, int bPrivate, int bNoPasswd, char *pszErr, size_t err_bufsz);
tbl_rsa_t *find_rsa_key(char *rsaname);
int32 _rsa_key_reference(tbl_rsa_t *p_rsa, int nCnt);
int export_rsa_for_user(RSA *pRsa, char *pszFilename, char *pszErr, size_t err_bufsz);
int32 _console_cmd_set(tbl_console_field_id_t field_id, tbl_console_t *pitem, 
    tbl_console_t *p_db_item, cfg_cmd_para_t *para);
int32 _vty_cmd_set(tbl_vty_field_id_t field_id, tbl_vty_t *pitem, 
    tbl_vty_t *p_db_item, cfg_cmd_para_t *para);
int32
app_add_login_record(tbl_login_record_t *p_login_record);
int32
app_del_login_record(tbl_login_record_t *p_login_record);
static int g_web_sessions = 0;
#define M_WEB_SESSION_MAX   3

/* --- user --- */

static int32
tbl_user_dump_filter(tbl_user_t *p_user, tbl_iter_args_t *pargs)
{
    tbl_user_t *p_user_filter = (tbl_user_t *)pargs->argv[2];

    if (NULL == p_user_filter || p_user == p_user_filter) {
        return tbl_user_dump_one(p_user, pargs);
    }

    return PM_E_NONE;
}

static int32
_user_cmd_get(tbl_user_t *puser, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = puser;
    tbl_user_iterate((TBL_ITER_CB_FUNC)tbl_user_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

uint32
auth_api_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }

    return FALSE;
}


static int
_check_user_field(char *pszkey, char *pszValue, tbl_user_t *p_user, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_USER);
    cdb_field_t *p_field = NULL;
    char szTmp[PASSWD_SIZE];
    char *psz_pwd = NULL;
    int32 field_id;
    int nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_user_field_id_t)field_id) {
    case TBL_USER_FLD_RSAKEY:
        if (sal_strlen(pszValue) > M_RSA_KEY_NAMESZ || sal_strlen(pszValue) <= 0)
        {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "RSA key length range is [1, 32]");
            goto err_out;
        }
        if (check_rsa_key(pszValue, KEY_TYPE_PUBLIC, 1, para->p_rs->desc, MAX_CMD_RET_LEN) != 0) {
            goto err_out;
        }
        sal_snprintf(p_user->rsakey, M_RSA_KEY_NAMESZ+1, "%s", pszValue);
        break;
    case TBL_USER_FLD_PRIVILEGE:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) || (nValue < 1 || nValue > 4)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid privilege value"); 
            goto err_out;
        }
        p_user->privilege = nValue;
        break;
    case TBL_USER_FLD_ENC_PASSWD:
    case TBL_USER_FLD_PASSWD:
        if (field_id == TBL_USER_FLD_PASSWD)
        {
            psz_pwd = pszValue;

        }
        else
        {
            sal_memset(szTmp, 0, sizeof(szTmp));

            /* modified by liwh for bug 53521, 2019-09-17 */
            if (strlen(pszValue) > PASSWD_SIZE)
            {
                snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
                goto err_out;
            }
            /* liwh end */
            
            if (do_decrypt(M_DEFAULT_PW_STR, pszValue, strlen(pszValue), szTmp, PASSWD_SIZE) != 0)
            {
                snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
                goto err_out;
            }
            psz_pwd = szTmp;
        }
        if (check_user_password(psz_pwd) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid password string: %s",
                    M_PASSWD_LIMIT_STR); 
            goto err_out;
        }
        
        if (field_id == TBL_USER_FLD_PASSWD) {
            sal_snprintf(p_user->passwd, PASSWD_SIZE, "%s", pszValue);
            sal_memset(p_user->enc_passwd, 0x00, PASSWD_SIZE);
        } else {
            sal_snprintf(p_user->enc_passwd, PASSWD_SIZE, "%s", pszValue);
            sal_memset(p_user->passwd, 0x00, PASSWD_SIZE);
        }
        break;
    default:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unsupportted field: %d", field_id); 
        goto err_out;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

int32 _user_get_uid()
{
    tbl_user_master_t *p_master = tbl_user_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_user_t *p_db_user = NULL;
    int exist;
    int i;

    for (i = 100; i <= 2000; i++) {
        exist = 0;
        
        CTCLIB_SLIST_LOOP(p_master->user_list, p_db_user, listnode)
        {
            if (i == p_db_user->uid) {
                exist = 1;
                break;
            }
        }

        if (!exist) {
            return i;
        }
    }
    
    return 2001;
}

static int
_user_validate(char *pszUsername, char *pszPasswd, char *pszErrMsg, size_t nBufsz)
{
    tbl_user_t user;
    tbl_user_t *p_user = &user;

    sal_memset(&user, 0x00, sizeof(tbl_user_t));
    sal_snprintf(user.key.name, USERNAME_SIZE, "%s", pszUsername);

    if ((p_user = tbl_user_get_user(&user.key)) == NULL) {
        sal_snprintf(pszErrMsg, nBufsz, "No such user");
        return -1;
    }

    if (4 != p_user->privilege) {
        sal_snprintf(pszErrMsg, nBufsz, "Privilege denied");
        return -1;
    }

    if ('\0' != p_user->passwd[0]) {
        if (sal_strcmp(pszPasswd, p_user->passwd)) {
            sal_snprintf(pszErrMsg, nBufsz, "Permission denied");
            return -1;
        }
    } else if ('\0' != p_user->enc_passwd[0]) {
        if (gen_passwd_validate(p_user->enc_passwd, pszPasswd) != 1) {
            sal_snprintf(pszErrMsg, nBufsz, "Permission denied");
            return -1;
        }
    }

    return 0;
}

static int32
_user_add(char *username, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szFileName[PATH_MAX];
    char szErr[256];
    char szTmp[PASSWD_SIZE];
    tbl_user_master_t *pmaster;
    tbl_user_t user;
    tbl_user_t *p_user = &user;
    tbl_rsa_t *p_rsa;
    int i;
    int rc;

    sal_memset(p_user, 0x00, sizeof(tbl_user_t));
    sal_snprintf(p_user->key.name, USERNAME_SIZE, "%s", username);

    if (tbl_user_get_user(&p_user->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_user_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->user_list) >= GLB_MAX_USER_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum user limit");
    }

    p_user->privilege = 1; /* setting default privilege to 1 */
    p_user->service_type = HOST_USER_SERVICE_TYPE_ALL; /* setting default service-type to all */
    for (i = 1; i < argc; i += 2) {
        if (_check_user_field(argv[i], argv[i + 1], p_user, para) == -1) {
            goto err_out;
        }
    }
    p_user->uid = _user_get_uid();

    if ('\0' != p_user->rsakey[0]) {
        p_rsa = find_rsa_key(p_user->rsakey);
        snprintf(szFileName, PATH_MAX, "%s/authkeys.%s", 
                M_SSHD_AUTH_KEY_PATH, p_user->key.name);
        unlink(szFileName);
        if (export_rsa_for_user(p_rsa->pstRsa, szFileName, szErr, 256) != 0) {
            CFG_FUNC_ERR_RET("Export Rsa key failed: %s", szErr);
        }   
        _rsa_key_reference(p_rsa, 1);
    }

    if (app_is_password_encryption()
            && '\0' != p_user->passwd[0])
    {
        gen_passwd_crypt(p_user->passwd, szTmp, PASSWD_SIZE);
        memset(p_user->passwd, 0x00, PASSWD_SIZE);
        snprintf(p_user->enc_passwd, PASSWD_SIZE, "%s", szTmp);
    }
    
    rc  = tbl_user_add_user(p_user);
    return rc;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_user_del(char *username, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szFileName[PATH_MAX];
    tbl_user_t user;
    tbl_user_t *p_user;
    tbl_rsa_t *p_rsa;
    
    sal_snprintf(user.key.name, USERNAME_SIZE, "%s", username);
    p_user = tbl_user_get_user(&user.key);
    if (NULL == p_user) {
        CFG_INVALID_PARAM_RET("User %s not found", username);
    }

    if ('\0' != p_user->rsakey[0]) {
        p_rsa = find_rsa_key(p_user->rsakey);
        if (NULL != p_rsa) {
            _rsa_key_reference(p_rsa, -1);
            snprintf(szFileName, PATH_MAX, "%s/authkeys.%s", 
                M_SSHD_AUTH_KEY_PATH, p_user->key.name);
            unlink(szFileName);
        }
    }
    
    return tbl_user_del_user(&user.key);
}

static int32
_user_cmd_set_privilege(tbl_user_t *p_user, tbl_user_t *p_db_user, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (p_user->privilege == p_db_user->privilege) {
        return PM_E_NONE;
    }

    if (p_user->privilege < 1 || p_user->privilege > 4) {
        CFG_INVALID_PARAM_RET("Invalid privilege value %d", p_user->privilege);
    }

    PM_E_RETURN(user_set_privilege(p_user));
    return rc;
}

static int32
_user_cmd_set_service_type(tbl_user_t *p_user, tbl_user_t *p_db_user)
{
   
    int32 rc = PM_E_NONE;

    if (p_user->service_type != p_db_user->service_type) {
        user_set_service_type(p_user, TBL_USER_FLD_SERVICE_TYPE);
    }

    return rc;
}

static int32
_user_cmd_set_passwd(tbl_user_t *p_user, tbl_user_t *p_db_user, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != p_user->passwd[0] && app_is_password_encryption()) {
        gen_passwd_crypt(p_user->passwd, szTmp, PASSWD_SIZE);
        snprintf(p_user->enc_passwd, PASSWD_SIZE, "%s", szTmp);
        memset(p_user->passwd, 0x00, PASSWD_SIZE);
    }

    if (sal_strcmp(p_user->passwd, p_db_user->passwd)) {
        user_set_passwd(p_user, TBL_USER_FLD_PASSWD);
    }

    if (sal_strcmp(p_user->enc_passwd, p_db_user->enc_passwd)) {
        user_set_passwd(p_user, TBL_USER_FLD_ENC_PASSWD);
    }
    return rc;
}

static int32
_user_cmd_set_rsakey(tbl_user_t *p_user, tbl_user_t *p_db_user, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_rsa_t *p_rsa;
    tbl_rsa_t *p_rsa_old;
    char szFileName[PATH_MAX];
    char szErr[256];

    if (!strcasecmp(p_user->rsakey, p_db_user->rsakey)) {
        return PM_E_NONE;
    }

    /* p_user->rsakey[0] == '\0' is for unassign rsa key */
    snprintf(szFileName, PATH_MAX, "%s/authkeys.%s", 
             M_SSHD_AUTH_KEY_PATH, p_user->key.name);
    if ('\0' != p_user->rsakey[0]) {
        if (check_rsa_key(p_user->rsakey, -1, 0, szErr, 256) != 0) {
            return CFG_INVALID_PARAM_RET("%s", szErr);
        }
        unlink(szFileName);
        p_rsa = find_rsa_key(p_user->rsakey);
        if (export_rsa_for_user(p_rsa->pstRsa, szFileName, szErr, 256) != 0) {
            CFG_FUNC_ERR_RET("Export Rsa key for user %s failed: %s", 
                             p_user->key.name, szErr);
        }        
        _rsa_key_reference(p_rsa, 1);
    } else {
        unlink(szFileName);
    }
    
    if ('\0' != p_db_user->rsakey[0]) {
        p_rsa_old = find_rsa_key(p_db_user->rsakey);
        if (p_rsa_old) {
            _rsa_key_reference(p_rsa_old, -1);
        }
    }

    rc = tbl_user_set_user_field(p_user, TBL_USER_FLD_RSAKEY);
    return rc;
}

int32
app_check_username(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        sal_sprintf(err_desc, "Invalid username: the first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }
    
    /*check the first character*/    
    if(!sal_isalpha(*name))
    {
        sal_sprintf(err_desc, "Invalid username: the first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_FIRST_CHAR;
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }

        sal_sprintf(err_desc, "Invalid username: the first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_INVALID_CHAR;
    }

    /*check the last character*/
    p--;
    if(!(sal_isdigit(*p) || sal_isalpha(*p)))
    {
        sal_sprintf(err_desc, "Invalid username: the last character should be 0-9 or a-z or A-Z and the max len is %d.", max_len);
        return CDB_NAME_ERR_LAST_CHAR;
    }
    
    return CDB_NAME_ERR_NONE;
}

static int32
_user_cmd_get_auth(cfg_cmd_para_t *para)
{
    tbl_user_master_t *p_master = tbl_user_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_user_t *p_db_user = NULL;
    FILE *fp = NULL;
    char szTmp[PASSWD_SIZE];
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    CTCLIB_SLIST_LOOP(p_master->user_list, p_db_user, listnode)
    {
        if (p_db_user->service_type&HOST_USER_SERVICE_TYPE_RPC_API)
        {
            if (sal_strlen(p_db_user->passwd))
            {
                sal_fprintf(fp, "%s/%s/%u\n", p_db_user->key.name, p_db_user->passwd, p_db_user->privilege);
            }
            else if (sal_strlen(p_db_user->enc_passwd))
            {
                if (do_decrypt(M_DEFAULT_PW_STR, p_db_user->enc_passwd, sal_strlen(p_db_user->enc_passwd),
    						   szTmp, PASSWD_SIZE) == 0)
                {
                    sal_fprintf(fp, "%s/%s/%u\n", p_db_user->key.name, szTmp, p_db_user->privilege);
                }
            }
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
app_user_cmd_get_auth_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = _user_cmd_get_auth(para);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int
line_port_match(uint32 l4_port_type, uint32 l4_port, uint32 l4_port_mask, u_int16_t port)
{
    switch (l4_port_type)
    {
    case GLB_ACE_PORT_TYEP_EQ:
        {
            if (l4_port == port)
            {  
                return 0;
            }

            break;
        }
    case GLB_ACE_PORT_TYEP_GT:
        {
            if (l4_port <= port)              /*modify by zhaoqd for bug 53653 2019-09-20*/
            {
                return 0;
            }

            break;
        }
    case GLB_ACE_PORT_TYEP_LT:
        {
            if (l4_port_mask >= port)       /*modify by zhaoqd for bug 53649 2019-09-20*/
            {
                return 0;
            }

            break;
        }
    case GLB_ACE_PORT_TYEP_RANGE:
        {
            if (l4_port <= port && port <= l4_port_mask)
            {
                return 0;
            }

            break;
        }
    case GLB_ACE_PORT_TYEP_ANY:
        return 0;
    default:
        {
            break;
        }

    }

    return LINE_ACL_NOT_MATCH;
}

static int32
line_acl_access_list_match(tbl_ace_config_t* p_db_ace_config, u_int32_t client_ip, u_int32_t dest_ip, u_int16_t client_port, u_int16_t dest_port)
{

    /* only care tcp */
    if (p_db_ace_config->ip_protocol && p_db_ace_config->ip_protocol != TCP_PROTOCAL_NUM)
    {
        return LINE_ACL_NOT_MATCH;
    }
  
    /* check source ip */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_IPSA))
    {
        if (( p_db_ace_config->src_ip.u.prefix4.s_addr & (~p_db_ace_config->src_ip_mask.u.prefix4.s_addr)) != (client_ip & (~p_db_ace_config->src_ip_mask.u.prefix4.s_addr))) 
        {
            return LINE_ACL_NOT_MATCH;
        }
    }
    
    /* check dest ip */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_IPDA))
    {
        if ((p_db_ace_config->dst_ip.u.prefix4.s_addr & (~p_db_ace_config->dst_ip_mask.u.prefix4.s_addr)) != (dest_ip & (~p_db_ace_config->dst_ip_mask.u.prefix4.s_addr))) 
        {
            return LINE_ACL_NOT_MATCH;
        }
    }

    /* check source port */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (line_port_match(p_db_ace_config->l4_src_port_type, p_db_ace_config->l4_src_port, p_db_ace_config->l4_src_port_mask, client_port))
        {
            return LINE_ACL_NOT_MATCH;
        }
    }

    /* check dest port */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (line_port_match(p_db_ace_config->l4_dst_port_type, p_db_ace_config->l4_dst_port, p_db_ace_config->l4_dst_port_mask, dest_port))
        {
            return LINE_ACL_NOT_MATCH;
        }
    }

    /* return action*/
    return p_db_ace_config->deny;
}

static int32
line_acl_ipv6_access_list_match(tbl_ace_config_t* p_db_ace_config, struct sal_in6_addr* client_ip, struct sal_in6_addr* dest_ip, u_int16_t client_port, u_int16_t dest_port)
{
    uint8 i = 0;
    
    /* only care tcp */
    if (p_db_ace_config->ip_protocol && p_db_ace_config->ip_protocol != TCP_PROTOCAL_NUM)
    {
        return LINE_ACL_NOT_MATCH;
    }

    /* check source ip */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPSA))
    {
        for (i = 0; i < 4; i++)
        {
            if ((p_db_ace_config->src_ip.u.prefix6.s6_addr32[i] & (~p_db_ace_config->src_ip_mask.u.prefix6.s6_addr32[i])) != (client_ip->s6_addr32[i] & (~p_db_ace_config->src_ip_mask.u.prefix6.s6_addr32[i]))) 
            {
                return LINE_ACL_NOT_MATCH;
            }
        }
    }
    
    /* check dest ip */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_ipv6_flags, GLB_IPV6_ACE_IPDA))
    {
        for (i = 0; i < 4; i++)
        {
            if ((p_db_ace_config->dst_ip.u.prefix6.s6_addr32[i] & (~p_db_ace_config->dst_ip_mask.u.prefix6.s6_addr32[i])) != (dest_ip->s6_addr32[i] & (~p_db_ace_config->dst_ip_mask.u.prefix6.s6_addr32[i]))) 
            {
                return LINE_ACL_NOT_MATCH;
            }
        }
    }
    
    /* check source port */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_SRCPORT))
    {
        if (line_port_match(p_db_ace_config->l4_src_port_type, p_db_ace_config->l4_src_port, p_db_ace_config->l4_src_port_mask, client_port))
        {
            return LINE_ACL_NOT_MATCH;
        }
    }

    /* check dest port */
    if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_DSTPORT))
    {
        if (line_port_match(p_db_ace_config->l4_dst_port_type, p_db_ace_config->l4_dst_port, p_db_ace_config->l4_dst_port_mask, dest_port))
        {
            return LINE_ACL_NOT_MATCH;
        }
    }
    
    /* return action*/
    return p_db_ace_config->deny;
}

static int32
line_vty_acl_check(tbl_vty_t * p_vty, tbl_login_t * p_login)
{
    tbl_ace_config_master_t *p_master = tbl_ace_config_get_master();
    tbl_acl_config_t* p_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;
    u_int32_t client_addr, dest_addr, client_port, dest_port = 0;
    struct sal_in6_addr  ipv6_client_addr;
    struct sal_in6_addr  ipv6_dest_addr;
    enum acl_result ret = 0;
    uint8 is_not_match = 1;
    
    sal_memset(acl_config_key.name, 0x0, ACL_NAME_FULL_NAME_SIZE+1);
    if (NULL == p_master)
    {
        return LINEACL_PERMIT;
    }
    
    /* get client ip address */
    if (1 == inet_pton(AF_INET, p_login->ipaddr, (void*)&client_addr))
    {
        if (NULL == p_vty->ipv4_acl)       /*not enable line vty acl*/
        {
            return LINEACL_PERMIT;
        }
        sal_memcpy(acl_config_key.name, p_vty->ipv4_acl, strlen(p_vty->ipv4_acl));
        p_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
        if (p_acl_config)
        {
            if (sal_strcmp((glb_acl_type_str[p_acl_config->type_identifying]), "IP ACL"))
            {
                return LINEACL_PERMIT;        /*not match mac information for bug 53527 and bug 53530*/
            }
        }
        /* get dest ip address */
        ret = inet_pton (AF_INET, p_login->dest_ipaddr, (void*)&dest_addr);
       if (ret <= 0)
       {
           log_sys(M_MOD_APP, E_ERROR, "Invlid destination IP address when building connection.\n");
           return LINEACL_DENY;
       }

       /* get client port */
       GET_INTEGER_RANGE("l4 client port", client_port, p_login->remote_port, 0, 65535);
       
       /* get dest port */
       GET_INTEGER_RANGE("l4 dest port", dest_port, p_login->dest_port, 0, 65535);

       CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace_config, listnode, next)
       {
            if (sal_strcmp(p_db_ace_config->key.acl_name, p_vty->ipv4_acl))
            {
                continue;
            }
            is_not_match = 0;       /*This acl is already exist*/
            ret = line_acl_access_list_match(p_db_ace_config, client_addr, dest_addr, client_port, dest_port);
            if (ret == LINEACL_PERMIT || ret == LINEACL_DENY)
            {
                return ret;
            }
        }

    }
    else if (1 == inet_pton(AF_INET6, p_login->ipaddr, &ipv6_client_addr))
    {
        if (NULL == p_vty->ipv6_acl)
        {
            return LINEACL_PERMIT;
        }

        sal_memcpy(acl_config_key.name, p_vty->ipv6_acl, strlen(p_vty->ipv6_acl));
        p_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
        if (p_acl_config)
        {
            if (sal_strcmp((glb_acl_type_str[p_acl_config->type_identifying]), "IPV6 ACL"))
            {
                return LINEACL_PERMIT;        /*not match mac information for bug 53527 and bug 53530*/
            }
        }
        
        /*get dest ip address*/
        ret = inet_pton (AF_INET6, p_login->dest_ipaddr, &ipv6_dest_addr);
        if (ret <= 0)
        {
            log_sys(M_MOD_APP, E_ERROR, "Invlid destination IP address when building connection.\n");
            return LINEACL_DENY;
        }
  
        /* get client port */
        GET_INTEGER_RANGE("l4 client port", client_port, p_login->remote_port, 0, 65535);
         
        /* get dest port */
        GET_INTEGER_RANGE("l4 dest port", dest_port, p_login->dest_port, 0, 65535);

        CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace_config, listnode, next)
        {
              if (sal_strcmp(p_db_ace_config->key.acl_name, p_vty->ipv6_acl))
              {
                  continue;
              }
              is_not_match = 0;
              ret = line_acl_ipv6_access_list_match(p_db_ace_config, &ipv6_client_addr, &ipv6_dest_addr, client_port, dest_port);
              if (ret == LINEACL_PERMIT || ret == LINEACL_DENY)
              {
                  return ret;
              }
         }
    }
    else
   {
      log_sys(M_MOD_APP, E_ERROR, "Invlid client IP address when building connection.\n");
      return LINEACL_DENY;
   }
   if (is_not_match && NULL == p_acl_config)                   /*aclname is not create*/
   {
       return LINEACL_PERMIT;
   }
   else
   {
        return LINE_ACL_NOT_MATCH;
   }
}

int32
app_act_line_acl_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_login_t * p_login = NULL;
    tbl_login_key_t login_key;
    tbl_vty_t * p_vty = NULL;
    tbl_vty_key_t vty_key;
    enum acl_result rc = 0;
    uint8 vtynum = 0;
    FILE * fp = 0;

    CFG_DBG_FUNC();
    /*for uml */
    if (!sal_strncmp(argv[0], "tty", 3))
    {
        rc = LINEACL_PERMIT;
        goto out;
    }
    sal_memset(&vty_key, 0, sizeof(tbl_vty_key_t));
    sal_memset(&login_key, 0, sizeof(tbl_login_key_t));
    sal_memcpy(login_key.name, argv[0], strlen(argv[0]));

    p_login = tbl_login_get_login(&login_key);
    if (!p_login)
    {
        log_sys(M_MOD_APP, E_ERROR, "TBL_LOGIN get failure.");
        return PM_E_INVALID_PARAM;
    }
    if (0 == sal_strncmp(p_login->vty, "vty", 3))
    {
        vtynum = atoi(p_login->vty + 3);
    }

    vty_key.id = vtynum;
    p_vty = tbl_vty_get_vty(&vty_key);
    if (!p_vty)
    {   
        log_sys(M_MOD_APP, E_ERROR, "TBL_VTY get failure.");
        return PM_E_INVALID_PARAM;
    }

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = line_vty_acl_check(p_vty, p_login);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

out:
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (rc == LINEACL_PERMIT)
    {
        sal_fprintf(fp, "permit");
    }
    else
    {
        sal_fprintf(fp, "deny");
    }
    
    sal_fclose(fp);
    
    return rc;
}

int32
app_user_get_service_type(char **argv, int32 argc, int8 *service_type)
{
    int8 is_telnet = 0;
    int8 is_ssh = 0;
    int8 is_rpc_api = 0;
    int8 is_web = 0;
    int8 type = 0;
    int8 i = 0;
    
    if (NULL == service_type)
    {
        return PM_E_NONE;
    }
    
    if (argc < 3)
    {
        return PM_E_NONE;
    }
    
    if ((3 == argc)&&!sal_strncmp(argv[2],"all", sal_strlen(argv[1])))
    {
        type = HOST_USER_SERVICE_TYPE_ALL;
    }
    else if ((3 == argc)&&!sal_strncmp(argv[2],"none", sal_strlen(argv[1])))
    {
        type = HOST_USER_SERVICE_TYPE_NONE;
    }
    else
    {
        for (i = 2; i<argc; i++)
        {
            if (!sal_strncmp(argv[i],"telnet", sal_strlen(argv[i])))
            {
                is_telnet = HOST_USER_SERVICE_TYPE_TELNET;
            }
            if (!sal_strncmp(argv[i],"ssh", sal_strlen(argv[i])))
            {
                is_ssh = HOST_USER_SERVICE_TYPE_SSH;
            }
            if (!sal_strncmp(argv[i],"rpc-api", sal_strlen(argv[i])))
            {
                is_rpc_api = HOST_USER_SERVICE_TYPE_RPC_API;
            }
            if (!sal_strncmp(argv[i],"web", sal_strlen(argv[i])))
            {
                is_web = HOST_USER_SERVICE_TYPE_WEB;
            }
        }
        type = is_telnet | is_ssh | is_rpc_api | is_web;
    }
    
    *service_type = type;
    return PM_E_NONE;
}

int32 
app_validate_password(char *pass,tbl_user_t *p_user)
{
    int bEnc = 0;

    if (NULL == pass || '\0' == pass[0]) 
    {
        return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
    }
    if ((!sal_strlen(p_user->passwd))&&(!sal_strlen(p_user->enc_passwd))) 
    {  
        return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
    }
    else if (sal_strlen(p_user->enc_passwd))
    {
        bEnc = 1;
    }

    if (bEnc) 
    {
        if (gen_passwd_validate(p_user->enc_passwd, (char *)pass) != 1) 
        {
            return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;	
        }
    } 
    else 
    {
        if (sal_strcmp(pass, p_user->passwd)) 
        {
            return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
        }
    }
 
    return PM_E_NONE;
}

int32 app_user_login_fail_action(tbl_login_record_t *p_login_record)
{
    tbl_login_security_glb_t *p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    tbl_login_record_t *p_db_login_record = tbl_login_record_get_login_record(&p_login_record->key);
    if (NULL == p_db_login_record)
    {
        return PM_E_NONE;
    }
    if (0 == p_login_record->fail_count)
    {
        p_login_record->fail_count++;
        p_login_record->first_fail_ts = (uint32)sal_time_sys_current(NULL);
        if (p_db_login_security->max_fail_num == p_login_record->fail_count)
        {
            p_login_record->locked = 1;
            p_login_record->lock_ts= (uint32)sal_time_sys_current(NULL);
            log_sys(M_MOD_APP, E_NOTICE, "The username %s has been locked.", p_login_record->key.name);
        }
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_FAIL_COUNT);
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS);
    }
    else if (p_login_record->fail_count < p_db_login_security->max_fail_num)
    {
        p_login_record->fail_count++;
        if (p_db_login_security->max_fail_num == p_login_record->fail_count)
        {
            p_login_record->locked = 1;
            p_login_record->lock_ts= (uint32)sal_time_sys_current(NULL);
            log_sys(M_MOD_APP, E_NOTICE, "The username %s has been locked.", p_login_record->key.name);
        }
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_FAIL_COUNT);
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_LOCKED);
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_LOCK_TS);
    }
    return PM_E_NONE;
}

int32 app_user_username_password_check(char **argv, char argc)
{
    int32 is_ssh_or_telnet = 0; 
    int32 connect_service = 0;
    tbl_user_t user;
    tbl_user_t *p_db_user = NULL;
    tbl_login_record_t login_record;
    tbl_login_record_t *p_db_login_record = NULL;
    tbl_login_security_glb_t *p_db_login_security = NULL;
    char pszPassword[PASSWD_SIZE];

    if (argc < 4)
    {
        return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
    }
    
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    is_ssh_or_telnet= sal_atoi(argv[2]);
    
    if (is_ssh_or_telnet)
    {
        connect_service = HOST_USER_SERVICE_TYPE_SSH;
    }
    else
    {
        connect_service = HOST_USER_SERVICE_TYPE_TELNET;
    }
    
    sal_memset(&login_record, 0, sizeof(login_record));
    sal_strncpy(login_record.key.name, argv[0], USERNAME_SIZE);
    p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
    sal_memset(&user, 0, sizeof(user));
    sal_strncpy(user.key.name, argv[0], USERNAME_SIZE);
    p_db_user = tbl_user_get_user(&user.key);
    
    if (p_db_login_security->enable)
    {
        if (NULL == p_db_login_record)
        {
             if (NULL != p_db_user)
             {
                app_add_login_record(&login_record);
             }
             else if (p_db_login_security->curr_invalid_user < p_db_login_security->max_invalid_user)
             {
                app_add_login_record(&login_record);
                p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
                p_db_login_record->invalid_user = TRUE;
                tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
                p_db_login_security->curr_invalid_user++;
                tbl_login_security_glb_set_login_security_glb_field(p_db_login_security,TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER);
             }
             else 
             {
                return APP_USR_PSSWD_CHECK_OVER_INVALID_LIMIT;
             }
        }
        
        p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
        if (p_db_login_record->locked)
        {
            if (NULL == p_db_user)
            {
                p_db_login_record->invalid_user = TRUE;
                tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            }
            else
            {
                p_db_login_record->invalid_user = FALSE;
                tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            }
            return APP_USR_PSSWD_CHECK_USER_LOCKED;
        }
        if (!sal_strncmp(argv[3],"none-password", sal_strlen(argv[3])))
        {
            if (NULL == p_db_user)
            {
                p_db_login_record->invalid_user = TRUE;
                tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            }
            else
            {
                p_db_login_record->invalid_user = FALSE;
                tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            }
            app_user_login_fail_action(p_db_login_record);
            return APP_USR_PSSWD_CHECK_USER_NONE_PASSWORD_GET;
        }
        if (NULL == p_db_user)
        {
            p_db_login_record->invalid_user = TRUE;
            tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            app_user_login_fail_action(p_db_login_record);
            return APP_USR_PSSWD_CHECK_USER_NOT_FOUND;
        }
        if (!(p_db_user->service_type&connect_service))
        {
            p_db_login_record->invalid_user = FALSE;
            tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            app_user_login_fail_action(p_db_login_record);
            return APP_USR_PSSWD_CHECK_SERVICE_TYPE_MISMATCH;
        }
        
        sal_strncpy(pszPassword, argv[3], PASSWD_SIZE);
        if (!app_validate_password(pszPassword, p_db_user))
        {
            app_del_login_record(p_db_login_record);
            return APP_USR_PSSWD_CHECK_LOGIN_SUCESS;
        }
        else
        {
            p_db_login_record->invalid_user = FALSE;
            tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_INVALID_USER);
            app_user_login_fail_action(p_db_login_record);
            return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
        }
    }
    else
    {
        if (!sal_strncmp(argv[3],"none-password", sal_strlen(argv[3])))
        {
            return APP_USR_PSSWD_CHECK_USER_NONE_PASSWORD_GET;
        }
        if (NULL == p_db_user)
        {
            return APP_USR_PSSWD_CHECK_USER_NOT_FOUND;
        }
        if (!(p_db_user->service_type&connect_service))
        {
            return APP_USR_PSSWD_CHECK_SERVICE_TYPE_MISMATCH;
        }
        sal_strncpy(pszPassword, argv[3], PASSWD_SIZE);
        if (!app_validate_password(pszPassword, p_db_user))
        {
            return APP_USR_PSSWD_CHECK_LOGIN_SUCESS;
        }
        else
        {
            return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
        }
        
    }
    
    return APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD;
}

int32
app_user_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_USER_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_user_t user;
    tbl_user_t *p_db_user = NULL;
    int8 service_type = 0;
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_USER);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        rc = app_check_username(argv[0], (USERNAME_CLI_SIZE - 1), para->p_rs->desc);
        if (rc < 0 || !strcasecmp(argv[0], "root")) {
            if (CDB_OPER_GET == para->oper) {
                CFG_INVALID_PARAM_RET("User root not found");
            } else {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                if (!strcasecmp(argv[0], "root")) {
                    CFG_INVALID_PARAM_RET("User root is forbidden");
                }
                return rc;
            }
        }
    
        sal_memset(&user, 0, sizeof(user));
        sal_strncpy(user.key.name, argv[0], USERNAME_SIZE);
        p_db_user = tbl_user_get_user(&user.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_user) {
            CFG_INVALID_PARAM_RET("User %s exist", user.key.name);
        }        
        rc = _user_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_user) {
            CFG_INVALID_PARAM_RET("User %s not found", user.key.name);
        } 
        /* modified by liwh for bug 51875, 2018-06-10 */
        /*else if (!strcasecmp(p_db_user->key.name, "admin")) {
            CFG_INVALID_PARAM_RET("Can't delete factory default user");
        }*/
        /* liwh end */
        rc = _user_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        /* modified by liwh for bug 46150, 2018-01-16 */
        /* liwh end */
        if ((argc >= 2) && !sal_strncmp(argv[1],"service-type", sal_strlen(argv[1])))
        {
            if (NULL == p_db_user) 
            {
                CFG_INVALID_PARAM_RET("User %s not found", user.key.name);
            }
            app_user_get_service_type(argv, argc, &service_type);
            user.service_type = service_type;
            _user_cmd_set_service_type(&user, p_db_user);
        }
        else if ((argc >= 2) && !sal_strncmp(argv[1],"user_password_check", sal_strlen(argv[1])))
        {
           rc = app_user_username_password_check(argv, argc);
           if (rc == APP_USR_PSSWD_CHECK_OVER_INVALID_LIMIT)
           {
                CFG_INVALID_PARAM_RET("Invalid user over limit");
           }
           if (rc == APP_USR_PSSWD_CHECK_USER_NONE_PASSWORD_GET)
           {
                CFG_INVALID_PARAM_RET("Can not get password");
           }   
           if (rc == APP_USR_PSSWD_CHECK_USER_LOCKED)
           {
                CFG_INVALID_PARAM_RET("The user is locked");
           }
           if (rc == APP_USR_PSSWD_CHECK_USER_NOT_FOUND)
           {
                CFG_INVALID_PARAM_RET("User not found");
           }
           if (rc == APP_USR_PSSWD_CHECK_SERVICE_TYPE_MISMATCH)
           {
                CFG_INVALID_PARAM_RET("Service type mismatch");
           }
           if (rc == APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD)
           {
                CFG_INVALID_PARAM_RET("Invalid user or password");
           }
           if (rc == APP_USR_PSSWD_CHECK_LOGIN_SUCESS)
           {
                return rc;
           }
        }
        else
        {   
            if (NULL == p_db_user) 
            {
                CFG_INVALID_PARAM_RET("User %s not found", user.key.name);
            }
            if (2 == argc) {
                if (!strcasecmp(argv[1], "passwd") || !strcasecmp(argv[1], "enc_passwd")) { 
                    field_id = TBL_USER_FLD_PASSWD;
                    memset(user.passwd, 0x00, PASSWD_SIZE);
                    memset(user.enc_passwd, 0x00, PASSWD_SIZE);
                    rc = _user_cmd_set_passwd(&user, p_db_user, para);
                } else if (!strcasecmp(argv[1], "rsakey")) {
                    field_id = TBL_USER_FLD_RSAKEY;
                    memset(user.rsakey, 0x00, M_RSA_KEY_NAMESZ);
                    rc = _user_cmd_set_rsakey(&user, p_db_user, para);
                } else {
                    CFG_INVALID_PARAM_RET("Invalid parameters");
                }
                
                return rc;
            }
            
            if ((argc % 2) != 1) {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            
            for (i = 1; i < argc; i += 2) {
                if (_check_user_field(argv[i], argv[i + 1], &user, para) == -1) {
                    return PM_E_CFG_INVALID_PARAM; 
                }

                cdb_get_field_id(argv[i], p_node, &p_field);
                if (CDB_ACCESS_RD == p_field->access) {
                    CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
                }
            }

            for (i = 1; i < argc; i += 2) {
                field_id = cdb_get_field_id(argv[i], p_node, &p_field);
                switch ((tbl_user_field_id_t)field_id) {
                case TBL_USER_FLD_PRIVILEGE:
                    rc = _user_cmd_set_privilege(&user, p_db_user, para);
                    break;
                case TBL_USER_FLD_ENC_PASSWD:
                case TBL_USER_FLD_PASSWD:
                    rc = _user_cmd_set_passwd(&user, p_db_user, para);
                    break;
                case TBL_USER_FLD_RSAKEY:
                    rc = _user_cmd_set_rsakey(&user, p_db_user, para);
                    
                default:
                    break;
                }
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_user && sal_strcmp(argv[0],"service-type")) {
            CFG_INVALID_PARAM_RET("User %s not found", argv[0]);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        if ((argc > 0) && (!sal_strcmp(argv[0],"service-type")))
        {
            rc = _user_cmd_get(NULL, para, p_node, &field);   
        }
        else
        {
            rc = _user_cmd_get(p_db_user, para, p_node, &field);   
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}
int32
app_set_login_security_max_fail_num(tbl_login_security_glb_t *p_login_security)
{
    if (NULL == p_login_security)
    {
        return PM_E_NONE;
    }
    tbl_login_security_glb_set_login_security_glb_field(p_login_security,TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM);
    tbl_login_security_glb_set_login_security_glb_field(p_login_security,TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD);
    return PM_E_NONE;
}

int32
app_set_login_security_lock_duration(tbl_login_security_glb_t *p_login_security)
{
    tbl_login_record_t *p_db_login_record = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_record_master_t *p_login_record_master = NULL;
    
    if (NULL == p_login_security)
    {
        return PM_E_NONE;
    }
    tbl_login_security_glb_set_login_security_glb_field(p_login_security,TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION);
    p_login_record_master = tbl_login_record_get_master();
    CTCLIB_SLIST_LOOP(p_login_record_master->user_list, p_db_login_record, listnode)
    {
        if (p_db_login_record->locked)
        {
           p_db_login_record->lock_ts =(uint32)sal_time_sys_current(NULL);
           tbl_login_record_set_login_record_field(p_db_login_record, TBL_LOGIN_RECORD_FLD_LOCK_TS);
        }
    }
    return PM_E_NONE;
}

int32
app_set_login_security_curr_invalid_user(tbl_login_security_glb_t *p_login_security)
{
    if (NULL == p_login_security)
    {
        return PM_E_NONE;
    }
    tbl_login_security_glb_set_login_security_glb_field(p_login_security,TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER);
    return PM_E_NONE;
}

int32
app_set_login_security_en(tbl_login_security_glb_t *p_db_login_security,uint32 is_enable)
{
    tbl_login_record_t *p_db_login_record = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_record_master_t *p_login_record_master = NULL;

    p_login_record_master = tbl_login_record_get_master();
    
    if (NULL == p_db_login_security)
    {
        return PM_E_NONE;
    }
    if (is_enable)
    {
        p_db_login_security ->enable = APP_LOGIN_DEFAULT_LOGIN_SECURITY_ENABLE;
        p_db_login_security->max_invalid_user = APP_LOGIN_DEFAULT_MAX_INVALID_USER;
        p_db_login_security->max_fail_num = APP_LOGIN_DEFAULT_MAX_FAIL_NUM;
        p_db_login_security->lock_duration = APP_LOGIN_DEFAULT_LOCKED_DURATION;
        p_db_login_security->fail_period = APP_LOGIN_DEFAULT_FAIL_PERIOD;
    }
    else
    {
        p_db_login_security ->enable = APP_LOGIN_DEFAULT_LOGIN_SECURITY_DISABLE;
        p_db_login_security->max_invalid_user = APP_LOGIN_DEFAULT_MAX_INVALID_USER;
        p_db_login_security->max_fail_num = APP_LOGIN_DEFAULT_MAX_FAIL_NUM;
        p_db_login_security->lock_duration = APP_LOGIN_DEFAULT_LOCKED_DURATION;
        p_db_login_security->fail_period = APP_LOGIN_DEFAULT_FAIL_PERIOD;
        CTCLIB_SLIST_LOOP_DEL(p_login_record_master->user_list, p_db_login_record, listnode, next)
        {
            tbl_login_record_del_login_record(&p_db_login_record->key);
        }
        
    }
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_ENABLE);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD);
    return PM_E_NONE;
}

static int32
tbl_login_security_dump_filter(tbl_login_security_glb_t *p_login_security, tbl_iter_args_t *pargs)
{
    tbl_login_security_glb_t *p_login_security_filter = (tbl_login_security_glb_t *)pargs->argv[2];

    if (NULL == p_login_security_filter || p_login_security == p_login_security_filter) {
        return tbl_login_security_glb_dump_one(p_login_security, pargs);
    }

    return PM_E_NONE;
}
int32
app_get_login_security(tbl_login_security_glb_t *p_login_security, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = p_login_security;
    tbl_login_security_glb_iterate((TBL_ITER_CB_FUNC)tbl_login_security_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}
int32
app_login_security_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_login_security_glb_t *p_db_login_security = NULL;
    int32 login_security_enable = 0;
    int32 max_fail_num          = 0;
    int32 period                = 0;
    int32 duration              = 0;
    int32 rc                    = 0;
    int32 curr_invalid_user     = 0;
    cdb_node_t* p_node          = NULL;
    field_parse_t field;
    
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    p_node = cdb_get_tbl(TBL_LOGIN_SECURITY_GLB);
    switch (para->oper) 
    {
     case CDB_OPER_ADD:
        break;
     case CDB_OPER_SET:
        if (argc < 2)
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        
        if (!sal_strcmp(argv[0], "enable"))
        {
           login_security_enable = sal_atoi(argv[1]);
           if (login_security_enable == p_db_login_security->enable)
           {
                return PM_E_NONE;
           }
           app_set_login_security_en(p_db_login_security, login_security_enable);
           
        }
        else if (!sal_strcmp(argv[0], "max-fail-num"))
        {
            if (!p_db_login_security->enable)
            {
                CFG_INVALID_PARAM_RET("Please enable login security first");
            }
            max_fail_num = sal_atoi(argv[1]);
            period = sal_atoi(argv[2]);
            if (max_fail_num == p_db_login_security->max_fail_num && period == p_db_login_security->fail_period)
           {
                return PM_E_NONE;
           }
           p_db_login_security->max_fail_num = max_fail_num;
           p_db_login_security->fail_period = period;
           app_set_login_security_max_fail_num(p_db_login_security);
        }
        else if (!sal_strcmp(argv[0], "lock-duration"))
        {
           if (!p_db_login_security->enable)
           {
               CFG_INVALID_PARAM_RET("Please enable login security first");
           }
           duration= sal_atoi(argv[1]);
           if (duration== p_db_login_security->lock_duration)
           {
                return PM_E_NONE;
           }
           p_db_login_security->lock_duration = duration;
           app_set_login_security_lock_duration(p_db_login_security);
        }
        else if (!sal_strcmp(argv[0], "curr_invalid_user"))
        {
            if (!p_db_login_security->enable)
           {
               CFG_INVALID_PARAM_RET("Please enable login security first");
           }
           curr_invalid_user= sal_atoi(argv[1]);
           if (curr_invalid_user== p_db_login_security->curr_invalid_user)
           {
                return PM_E_NONE;
           }
           p_db_login_security->curr_invalid_user = curr_invalid_user;
           app_set_login_security_curr_invalid_user(p_db_login_security);
        }
        break;
     case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_login_security) {
            CFG_INVALID_PARAM_RET("User %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = app_get_login_security(p_db_login_security, para, p_node, &field);    
        break;
        break;
     default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}

int32
app_add_login_record(tbl_login_record_t *p_login_record)
{
    if (NULL == p_login_record)
    {
        return PM_E_NONE;
    }
    tbl_login_record_add_login_record(p_login_record);
    return PM_E_NONE;
}

int32
app_del_login_record(tbl_login_record_t *p_login_record)
{
    tbl_login_security_glb_t *p_db_login_security = NULL;
    
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    if (NULL == p_login_record)
    {
        return PM_E_NONE;
    }
    
    if (p_login_record->invalid_user)
    {
        p_db_login_security ->curr_invalid_user--;
        tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER);
    }
    
    tbl_login_record_del_login_record(&p_login_record->key);
    return PM_E_NONE;
}

int32
app_set_login_record(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_login_record_t *p_login_record = NULL;
    tbl_login_record_t login_record;
    uint32 u32 = 0;
    
    sal_strncpy(login_record.key.name, argv[0], USERNAME_SIZE);
    p_login_record = tbl_login_record_get_login_record(&login_record.key);
    if (NULL == p_login_record)
    {
        CFG_CONFLICT_RET("Not found this record");
    }
    if (!sal_strcmp(argv[1], "locked"))
    {
        u32 = sal_atoi(argv[2]);
        p_login_record->locked = u32;
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_LOCKED);
    }
    else if (!sal_strcmp(argv[1], "lock_ts"))
    {
        u32 = sal_atoi(argv[2]);
        p_login_record->lock_ts = u32;
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_LOCK_TS);
    }
    else if (!sal_strcmp(argv[1], "fail_count"))
    {
        u32 = sal_atoi(argv[2]);
        p_login_record->fail_count = u32;
        tbl_login_record_set_login_record_field(p_login_record, TBL_LOGIN_RECORD_FLD_FAIL_COUNT);
    }
    
    return PM_E_NONE;
}

static int32
tbl_login_record_dump_filter(tbl_login_record_t *p_login_record, tbl_iter_args_t *pargs)
{
    tbl_login_record_t *p_login_record_filter = (tbl_login_record_t *)pargs->argv[2];

    if (NULL == p_login_record_filter || p_login_record == p_login_record_filter) {
        return tbl_login_record_dump_one(p_login_record, pargs);
    }

    return PM_E_NONE;
}

int32
app_get_login_record(tbl_login_record_t *p_login_record, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = p_login_record;
    tbl_login_record_iterate((TBL_ITER_CB_FUNC)tbl_login_record_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

 int32
app_login_record_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_login_record_t login_record;
    tbl_login_record_t *p_db_login_record = NULL;
    tbl_login_record_master_t *pmaster = NULL;
    cdb_node_t* p_node          = NULL;
    int32 rc                    = 0;
    field_parse_t field;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_record_master_t *p_login_record_master = NULL;
    
    p_node = cdb_get_tbl(TBL_LOGIN_RECORD);
    sal_memset(&login_record, 0, sizeof(login_record));
    if (0 == argc) 
    {
        if (CDB_OPER_GET != para->oper) 
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    }
    switch (para->oper) 
    {
     case CDB_OPER_ADD:
        if (1 != argc)
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
        sal_strncpy(login_record.key.name, argv[0], USERNAME_SIZE);
        p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
        if (NULL != p_db_login_record) {
            CFG_INVALID_PARAM_RET("Login record %s exist", login_record.key.name);
        }
        pmaster = tbl_login_record_get_master();
        if (CTCLIB_SLISTCOUNT(pmaster->user_list) >= GLB_MAX_USER_NUM) 
        {
            CFG_CONFLICT_RET("Exceeding maximum user limit");
        }
        app_add_login_record(&login_record);
        break;
     case CDB_OPER_DEL:
        if (1 != argc)
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
        if (!sal_strcmp(argv[0], "all"))
        {
            p_login_record_master = tbl_login_record_get_master();
            CTCLIB_SLIST_LOOP_DEL(p_login_record_master->user_list, p_db_login_record, listnode, next)
            {
                app_del_login_record(p_db_login_record);
            }
        }
        else
        {
            sal_strncpy(login_record.key.name, argv[0], USERNAME_SIZE);
            p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
            if (NULL == p_db_login_record)
            {
                CFG_CONFLICT_RET("Not found this record");
            }
            app_del_login_record(p_db_login_record);
        }
        break;
     case CDB_OPER_SET:
        if (0 == argc || argc > 3)
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
        
        app_set_login_record(argv, argc, para);
        break;
     case CDB_OPER_GET:
        if (argc > 0)
        {
            sal_strncpy(login_record.key.name, argv[0], USERNAME_SIZE);
            p_db_login_record = tbl_login_record_get_login_record(&login_record.key);
            if (NULL == p_db_login_record)
            {
                CFG_INVALID_PARAM_RET("User %s not found", argv[0]);
            }
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = app_get_login_record(p_db_login_record, para, p_node, &field); 
        break;
     default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
       }
    return PM_E_NONE;
}
int32
app_act_show_login_security(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32    rc            = PM_E_NONE;
    FILE    *fp            = NULL;
    int32 resumetime = 0;
    tbl_login_record_t *p_db_login_record = NULL;
    tbl_login_security_glb_t *p_db_login_security = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_record_master_t *p_login_record_master;
    
    p_login_record_master = tbl_login_record_get_master();
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    if (argc)
    {
        CFG_INVALID_PARAM_RET("Invalid action");
    }
    
    if (!p_db_login_security->enable)
    {
        CFG_INVALID_PARAM_RET("Please enable login security first");
    }
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fprintf(fp, " Login Security:             %s\n", p_db_login_security->enable == APP_LOGIN_DEFAULT_LOGIN_SECURITY_ENABLE ? "Enable" : "Disable");
    sal_fprintf(fp, " Max Fail Number:            %d\n", p_db_login_security->max_fail_num);
    sal_fprintf(fp, " Fail Period:                %d min\n", p_db_login_security->fail_period);
    sal_fprintf(fp, " Lock Duration:              %d min\n", p_db_login_security->lock_duration);
    sal_fprintf(fp, " Current Invalid Users:      %d/5\n", p_db_login_security->curr_invalid_user);
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "Login Security Records:\n");
    sal_fprintf(fp, "User name                        Local   Locked     Resume Time(s)     Fail Count\n");
    sal_fprintf(fp, "--------------------------------+-------+----------+------------------+-----------\n");
    CTCLIB_SLIST_LOOP(p_login_record_master->user_list, p_db_login_record, listnode)
    {
      resumetime = p_db_login_record->lock_ts + 60 * p_db_login_security->lock_duration 
                        - (uint32)sal_time_sys_current(NULL);
      if (resumetime < 0)
      {
        resumetime = 0;
      }
      if (p_db_login_record->locked)
      {
            sal_fprintf(fp, "%-32s %-7s %-10s %-18d %-11s\n", p_db_login_record->key.name, 
                    p_db_login_record->invalid_user ? "No" : "Yes",
                    "Yes", 
                    resumetime,
                    "-"); 
      }
      else
      {
            sal_fprintf(fp, "%-32s %-7s %-10s %-18s %-11d\n", p_db_login_record->key.name, 
                    p_db_login_record->invalid_user ? "No" : "Yes",
                    "No", 
                    "-",
                    p_db_login_record->fail_count); 
      }
    }
    
    sal_fprintf(fp, "\n");
    sal_fclose(fp);
    fp = NULL;
    return rc;
}

/* -- authen -- */
static int32
tbl_authen_dump_filter(tbl_authen_t *p_item, tbl_iter_args_t *pargs)
{
    tbl_authen_t *p_filter = (tbl_authen_t *)pargs->argv[2];

    if (NULL == p_filter || p_item == p_filter) {
        return tbl_authen_dump_one(p_item, pargs);
    }

    return PM_E_NONE;
}

static int32
_authen_cmd_get(tbl_authen_t *pauthen, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pauthen;
    tbl_authen_iterate((TBL_ITER_CB_FUNC)tbl_authen_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
_check_authen_methods(char *pszValue, tbl_authen_t *p_authen)
{
    char szTmp[LINE_AUTH_METHOD_LINE_SIZE];
    int ret = -1;
    char **ppTypeArr = NULL;
    unsigned long nCnt = 0;
    int type;
    int size = 0;
    int len;
    int i;

    if (split_tokens(pszValue, strlen(pszValue), AUTHEN_METHOD_MAX, ",", &nCnt, &ppTypeArr) != 0) {
        return -1;
    }

    szTmp[0] = '\0';
    for (i = 0; i < nCnt; i++) {
        type = cdb_enum_str2val(authen_method_strs, AUTHEN_METHOD_MAX, ppTypeArr[i]);
        if (-1 == type) {
            goto err_out;
        }
        len = strlen(ppTypeArr[i]);
        if ((size + len + 1) >= LINE_AUTH_METHOD_LINE_SIZE
                || NULL != strstr(szTmp, ppTypeArr[i])) {
            goto err_out;
        }

        size += snprintf(szTmp + size, LINE_AUTH_METHOD_LINE_SIZE - size, 
                "%s%s", (0 != i) ? "," : "", ppTypeArr[i]);
    }

    snprintf(p_authen->methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", szTmp);
    ret = 0;
err_out:
    free_tokens(&ppTypeArr);
    return ret;
}

static int
_check_authen_field(char *pszkey, char *pszValue, tbl_authen_t *p_authen, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_AUTHEN);
    cdb_field_t *p_field = NULL;
    int32 field_id = -1;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_authen_field_id_t)field_id) {
    case TBL_AUTHEN_FLD_METHODS:
        if (_check_authen_methods(pszValue, p_authen) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication method string");
            goto err_out;
        }
        break;
    default:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unsupportted field %d", field_id);
        goto err_out;
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_authen_add(char *authenname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_authen_master_t *pmaster;
    tbl_authen_t authen;
    tbl_authen_t *p_authen = &authen;
    int i;

    sal_memset(p_authen, 0x00, sizeof(tbl_authen_t));
    sal_snprintf(p_authen->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", authenname);

    if (tbl_authen_get_authen(&p_authen->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_authen_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->authen_list) >= GLB_MAX_AUTHEN_LIST_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum authentication limit");
    }

    for (i = 1; i < argc; i += 2) {
        if (_check_authen_field(argv[i], argv[i + 1], p_authen, para) == -1) {
            goto err_out;
        }
    }    

    return tbl_authen_add_authen(p_authen);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

/* modified by liwh for bug 39920, 2016-08-03 */
int32
authen_clear_console_vty_for_methods(char *authenname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_console_t  *p_db_item = NULL;
    tbl_console_t  console_item;
    int32          rc = PM_E_NONE;
    tbl_vty_t      *p_db_vty_item = NULL;
    tbl_vty_t      vty_item;
    uint32         id = 0;

    sal_memset(&console_item, 0, sizeof(tbl_console_t));
    sal_memset(&vty_item, 0, sizeof(tbl_vty_t));
    
    p_db_item = tbl_console_get_console();
    if (0 == sal_memcmp(p_db_item->auth_method, authenname, sal_strlen(authenname)))
    {
        rc |= _console_cmd_set(TBL_CONSOLE_FLD_AUTH_METHOD, &console_item, p_db_item, para);
    }

    for (id = 0; id < GLB_MAX_VTY_NUM; id++)
    {
        vty_item.key.id = id;
        p_db_vty_item = tbl_vty_get_vty(&vty_item.key);

        /*added by yejl to fix bug41177, set the auth_method is default*/
        snprintf(vty_item.auth_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
        if (0 == sal_memcmp(p_db_vty_item->auth_method, authenname, sal_strlen(authenname)))
        {
            rc |= _vty_cmd_set(TBL_VTY_FLD_AUTH_METHOD, &vty_item, p_db_vty_item, para);
        }
    }

    return PM_E_NONE;
}
/* liwh end */

static int32
_authen_del(char *authenname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_authen_t authen;
    tbl_authen_t *p_authen = &authen;

    /* modified by liwh for bug 39920, 2016-08-03 */
    authen_clear_console_vty_for_methods(authenname, argv, argc, para);
    /* liwh end */
    
    sal_snprintf(p_authen->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", authenname);
    return tbl_authen_del_authen(&p_authen->key);
}

static int32
_authen_cmd_set_methods(tbl_authen_t *p_authen, tbl_authen_t *p_db_authen, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (!strcasecmp(p_authen->methods, p_db_authen->methods)) {
        return PM_E_NONE;
    }

    PM_E_RETURN(authen_set_methods(p_authen));
    return rc;
}

int32
app_authen_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_AUTHEN_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_authen_t item;
    tbl_authen_t *p_db_item = NULL;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    CFG_DBG_FUNC();

    if (!p_db_glb->aaa_new_model)
    {
        CFG_CONFLICT_RET("Please enable aaa first");
        return rc;
    }

    p_node = cdb_get_tbl(TBL_AUTHEN);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        rc = cdb_check_name(argv[0], (LINE_AUTH_METHOD_NAME_SIZE - 1), para->p_rs->desc);
        if (rc < 0) {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    
        sal_memset(&item, 0, sizeof(tbl_authen_t));
        sal_strncpy(item.key.name, argv[0], LINE_AUTH_METHOD_NAME_SIZE);
        p_db_item = tbl_authen_get_authen(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication list %s exist", item.key.name);
        }        
        rc = _authen_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication list %s not found", item.key.name);
        }
        rc = _authen_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication list %s not found", item.key.name);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_authen_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            switch ((tbl_authen_field_id_t)field_id) {
            case TBL_AUTHEN_FLD_METHODS:
                rc = _authen_cmd_set_methods(&item, p_db_item, para);
                break;
                
            default:
                break;
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication list %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _authen_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- author -- */
static int32
tbl_author_dump_filter(tbl_author_t *p_item, tbl_iter_args_t *pargs)
{
    tbl_author_t *p_filter = (tbl_author_t *)pargs->argv[2];

    if (NULL == p_filter || p_item == p_filter) {
        return tbl_author_dump_one(p_item, pargs);
    }

    return PM_E_NONE;
}

static int32
_author_cmd_get(tbl_author_t *pauthor, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pauthor;
    tbl_author_iterate((TBL_ITER_CB_FUNC)tbl_author_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
_check_author_methods(char *pszValue, tbl_author_t *p_author)
{
    char szTmp[LINE_AUTH_METHOD_LINE_SIZE];
    int ret = -1;
    char **ppTypeArr = NULL;
    unsigned long nCnt = 0;
    int type;
    int size = 0;
    int len;
    int i;

    if (split_tokens(pszValue, strlen(pszValue), AUTHOR_METHOD_MAX, ",", &nCnt, &ppTypeArr) != 0) {
        return -1;
    }

    szTmp[0] = '\0';
    for (i = 0; i < nCnt; i++) {
        type = cdb_enum_str2val(author_method_strs, AUTHOR_METHOD_MAX, ppTypeArr[i]);
        if (-1 == type) {
            goto err_out;
        }
        len = strlen(ppTypeArr[i]);
        if ((size + len + 1) >= LINE_AUTH_METHOD_LINE_SIZE
                || NULL != strstr(szTmp, ppTypeArr[i])) {
            goto err_out;
        }

        size += snprintf(szTmp + size, LINE_AUTH_METHOD_LINE_SIZE - size, 
                "%s%s", (0 != i) ? "," : "", ppTypeArr[i]);
    }

    snprintf(p_author->methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", szTmp);
    ret = 0;
err_out:
    free_tokens(&ppTypeArr);
    return ret;
}

static int
_check_author_field(char *pszkey, char *pszValue, tbl_author_t *p_author, cfg_cmd_para_t *para)
{
    cdb_node_t  *p_node = cdb_get_tbl(TBL_AUTHOR);
    cdb_field_t *p_field = NULL;
    int32       field_id = -1;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_author_field_id_t)field_id) {
    case TBL_AUTHOR_FLD_METHODS:
        if (_check_author_methods(pszValue, p_author) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authorization method string");
            goto err_out;
        }
        break;
    default:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unsupportted field %d", field_id);
        goto err_out;
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_author_add(char *authorname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_author_master_t *pmaster;
    tbl_author_t        author;
    tbl_author_t        *p_author = &author;
    int i;

    sal_memset(p_author, 0x00, sizeof(tbl_author_t));
    sal_snprintf(p_author->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", authorname);

    if (tbl_author_get_author(&p_author->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_author_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->author_list) >= GLB_MAX_AUTHOR_LIST_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum authorization limit");
    }

    for (i = 1; i < argc; i += 2) {
        if (_check_author_field(argv[i], argv[i + 1], p_author, para) == -1) {
            goto err_out;
        }
    }    

    return tbl_author_add_author(p_author);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

int32
author_clear_vty_for_methods(char *authorname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32          rc = PM_E_NONE;
    tbl_vty_t      *p_db_vty_item = NULL;
    tbl_vty_t      vty_item;
    uint32         id = 0;

    sal_memset(&vty_item, 0, sizeof(tbl_vty_t));
    
    for (id = 0; id < GLB_MAX_VTY_NUM; id++)
    {
        vty_item.key.id = id;
        p_db_vty_item = tbl_vty_get_vty(&vty_item.key);

        /*added by yejl to fix bug42291, set the author_method is default*/
        snprintf(vty_item.author_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
        if (0 == sal_memcmp(p_db_vty_item->author_method, authorname, sal_strlen(authorname)))
        {
            rc |= _vty_cmd_set(TBL_VTY_FLD_AUTHOR_METHOD, &vty_item, p_db_vty_item, para);
        }
    }

    return PM_E_NONE;
}

static int32
_author_del(char *authorname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_author_t author;
    tbl_author_t *p_author = &author;

    author_clear_vty_for_methods(authorname, argv, argc, para);
    
    sal_snprintf(p_author->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", authorname);
    return tbl_author_del_author(&p_author->key);
}

static int32
_author_cmd_set_methods(tbl_author_t *p_author, tbl_author_t *p_db_author, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (!strcasecmp(p_author->methods, p_db_author->methods)) {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_author_set_author_field(p_author, TBL_AUTHOR_FLD_METHODS));

    return rc;
}

int32
app_author_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t       *p_node = NULL;
    int32            field_id = TBL_AUTHOR_FLD_MAX;
    cdb_field_t      *p_field = NULL;
    int32            i = 0;
    field_parse_t    field;
    int32            rc = PM_E_NONE;
    tbl_author_t     item;
    tbl_author_t     *p_db_item = NULL;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    CFG_DBG_FUNC();

    if (!p_db_glb->aaa_new_model)
    {
        CFG_CONFLICT_RET("Please enable aaa first");
        return rc;
    }

    p_node = cdb_get_tbl(TBL_AUTHOR);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        rc = cdb_check_name(argv[0], (LINE_AUTH_METHOD_NAME_SIZE -1), para->p_rs->desc);
        if (rc < 0) {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    
        sal_memset(&item, 0, sizeof(tbl_author_t));
        sal_strncpy(item.key.name, argv[0], LINE_AUTH_METHOD_NAME_SIZE);
        p_db_item = tbl_author_get_author(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Authorization list %s exist", item.key.name);
        }        
        rc = _author_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authorization list %s not found", item.key.name);
        }
        rc = _author_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authorization list %s not found", item.key.name);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_author_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            switch ((tbl_author_field_id_t)field_id) {
            case TBL_AUTHOR_FLD_METHODS:
                rc = _author_cmd_set_methods(&item, p_db_item, para);
                break;
                
            default:
                break;
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authorization list %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _author_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- account -- */
int32
account_clear_vty_for_methods(char *accountname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32          rc = PM_E_NONE;
    tbl_vty_t      *p_db_vty_item = NULL;
    tbl_vty_t      vty_item;
    uint32         id = 0;

    sal_memset(&vty_item, 0, sizeof(tbl_vty_t));
    
    for (id = 0; id < GLB_MAX_VTY_NUM; id++)
    {
        vty_item.key.id = id;
        p_db_vty_item = tbl_vty_get_vty(&vty_item.key);

        /*added by yejl to fix bug42291, set the account_method is default*/
        snprintf(vty_item.account_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
        if (0 == sal_memcmp(p_db_vty_item->account_method, accountname, sal_strlen(accountname)))
        {
            rc |= _vty_cmd_set(TBL_VTY_FLD_ACCOUNT_METHOD, &vty_item, p_db_vty_item, para);
        }
    }

    return PM_E_NONE;
}

static int32
_account_del(char *accountname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_account_t account;
    tbl_account_t *p_account = &account;

    account_clear_vty_for_methods(accountname, argv, argc, para);
    
    sal_snprintf(p_account->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", accountname);
    return tbl_account_del_account(&p_account->key);
}

static int
_check_account_methods(char *pszValue, tbl_account_t *p_account)
{
    char           szTmp[LINE_AUTH_METHOD_LINE_SIZE];
    int            ret = -1;
    char           **ppTypeArr = NULL;
    unsigned long  nCnt = 0;
    int            type = 0;
    int            size = 0;
    int            len = 0;
    int            i = 0;

    if (split_tokens(pszValue, strlen(pszValue), ACCOUNT_METHOD_MAX, ",", &nCnt, &ppTypeArr) != 0) {
        return -1;
    }

    szTmp[0] = '\0';
    for (i = 0; i < nCnt; i++) {
        type = cdb_enum_str2val(account_method_strs, ACCOUNT_METHOD_MAX, ppTypeArr[i]);
        if (-1 == type) {
            goto err_out;
        }
        len = strlen(ppTypeArr[i]);
        if ((size + len + 1) >= LINE_AUTH_METHOD_LINE_SIZE
                || NULL != strstr(szTmp, ppTypeArr[i])) {
            goto err_out;
        }

        size += snprintf(szTmp + size, LINE_AUTH_METHOD_LINE_SIZE - size, 
                "%s%s", (0 != i) ? "," : "", ppTypeArr[i]);
    }

    snprintf(p_account->methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", szTmp);
    ret = 0;
err_out:
    free_tokens(&ppTypeArr);
    return ret;
}

static int
_check_account_field(char *pszkey, char *pszValue, tbl_account_t *p_account, cfg_cmd_para_t *para)
{
    cdb_node_t  *p_node = cdb_get_tbl(TBL_ACCOUNT);
    cdb_field_t *p_field = NULL;
    int32       field_id = -1;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_account_field_id_t)field_id) {
    case TBL_ACCOUNT_FLD_METHODS:
        if (_check_account_methods(pszValue, p_account) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid accounting method string");
            goto err_out;
        }
        break;
    case TBL_ACCOUNT_FLD_START_STOP:
        if (0 == sal_strcmp(pszValue, "1"))
        {
            p_account->start_stop = TRUE;
        }
        break;
    case TBL_ACCOUNT_FLD_STOP_ONLY:
        if (0 == sal_strcmp(pszValue, "1"))
        {            
            p_account->stop_only = TRUE;
        }
        break;
    default:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unsupportted field %d", field_id);
        goto err_out;
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}


static int32
_account_add(char *accountname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_account_master_t *pmaster;
    tbl_account_t        account;
    tbl_account_t        *p_account = &account;
    int i;

    sal_memset(p_account, 0x00, sizeof(tbl_account_t));
    sal_snprintf(p_account->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", accountname);

    if (tbl_account_get_account(&p_account->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_account_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->account_list) >= GLB_MAX_ACCOUNT_LIST_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum accounting limit");
    }

    for (i = 1; i < argc; i += 2) {
        if (_check_account_field(argv[i], argv[i + 1], p_account, para) == -1) {
            goto err_out;
        }
    }    

    return tbl_account_add_account(p_account);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_account_cmd_set_methods(tbl_account_t *p_account, tbl_account_t *p_db_account, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (!strcasecmp(p_account->methods, p_db_account->methods)) {
        return PM_E_NONE;
    }
    
    PM_E_RETURN(tbl_account_set_account_field(p_account, TBL_ACCOUNT_FLD_METHODS));
    return rc;
}

/*added by yejl to fix bug41139, 2016-12-13*/
static int32
_account_cmd_set_start_stop(uint32 value, tbl_account_t *p_account, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (1 == value)
    {
        p_account->start_stop = TRUE;
    }
    else if (0 == value)
    {
        p_account->start_stop = FALSE;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }
     
    PM_E_RETURN(tbl_account_set_account_field(p_account, TBL_ACCOUNT_FLD_START_STOP));
    return rc;
}


static int32
_account_cmd_set_stop_only(uint32 value, tbl_account_t *p_account, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (1 == value)
    {
        p_account->stop_only = TRUE;
    }
    else if (0 == value)
    {
        p_account->stop_only = FALSE;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    PM_E_RETURN(tbl_account_set_account_field(p_account, TBL_ACCOUNT_FLD_STOP_ONLY));
    return rc;
}
/*end*/
 
static int32
tbl_account_dump_filter(tbl_account_t *p_item, tbl_iter_args_t *pargs)
{
    tbl_account_t *p_filter = (tbl_account_t *)pargs->argv[2];

    if (NULL == p_filter || p_item == p_filter) {
        return tbl_account_dump_one(p_item, pargs);
    }

    return PM_E_NONE;
}

static int32
_account_cmd_get(tbl_account_t *paccount, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = paccount;
    tbl_account_iterate((TBL_ITER_CB_FUNC)tbl_account_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
app_account_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t        *p_node = NULL;
    int32             field_id = TBL_ACCOUNT_FLD_MAX;
    cdb_field_t       *p_field = NULL;
    int32             i = 0;
    uint32            value = 0;
    field_parse_t     field;
    int32             rc = PM_E_NONE;
    tbl_account_t     item;
    tbl_account_t     *p_db_item = NULL;
    tbl_sys_global_t  *p_db_glb = tbl_sys_global_get_sys_global();
    
    CFG_DBG_FUNC();

    if (!p_db_glb->aaa_new_model)
    {
        CFG_CONFLICT_RET("Please enable aaa first");
        return rc;
    }

    p_node = cdb_get_tbl(TBL_ACCOUNT);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        rc = cdb_check_name(argv[0], (LINE_AUTH_METHOD_NAME_SIZE - 1), para->p_rs->desc);
        if (rc < 0) {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    
        sal_memset(&item, 0, sizeof(tbl_account_t));
        sal_strncpy(item.key.name, argv[0], LINE_AUTH_METHOD_NAME_SIZE);
        p_db_item = tbl_account_get_account(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting list %s exist", item.key.name);
        }        
        rc = _account_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting list %s not found", item.key.name);
        }
        rc = _account_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting list %s not found", item.key.name);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_account_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            switch ((tbl_account_field_id_t)field_id) {
            case TBL_ACCOUNT_FLD_METHODS:
                rc = _account_cmd_set_methods(&item, p_db_item, para);
                break;
            /*added by yejl to fix bug41139, 2016-12-13*/
            case TBL_ACCOUNT_FLD_START_STOP:
                PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
                rc = _account_cmd_set_start_stop(value, &item, para);
                break;
            case TBL_ACCOUNT_FLD_STOP_ONLY:
                PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
                rc = _account_cmd_set_stop_only(value, &item, para);
                break;
            /*end*/    
            default:
                break;
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting list %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _account_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- accountcmd -- */
static int32
tbl_accountcmd_dump_filter(tbl_accountcmd_t *p_item, tbl_iter_args_t *pargs)
{
    tbl_accountcmd_t *p_filter = (tbl_accountcmd_t *)pargs->argv[2];

    if (NULL == p_filter || p_item == p_filter) {
        return tbl_accountcmd_dump_one(p_item, pargs);
    }

    return PM_E_NONE;
}

static int32
_accountcmd_cmd_get(tbl_accountcmd_t *paccountcmd, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = paccountcmd;
    tbl_accountcmd_iterate((TBL_ITER_CB_FUNC)tbl_accountcmd_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
_check_accountcmd_methods(char *pszValue, tbl_accountcmd_t *p_accountcmd)
{
    char szTmp[LINE_AUTH_METHOD_LINE_SIZE];
    int ret = -1;
    char **ppTypeArr = NULL;
    unsigned long nCnt = 0;
    int type;
    int size = 0;
    int len;
    int i;

    if (split_tokens(pszValue, strlen(pszValue), ACCOUNTCMD_METHOD_MAX, ",", &nCnt, &ppTypeArr) != 0) {
        return -1;
    }

    szTmp[0] = '\0';
    for (i = 0; i < nCnt; i++) {
        type = cdb_enum_str2val(accountcmd_method_strs, ACCOUNTCMD_METHOD_MAX, ppTypeArr[i]);
        if (-1 == type) {
            goto err_out;
        }
        len = strlen(ppTypeArr[i]);
        if ((size + len + 1) >= LINE_AUTH_METHOD_LINE_SIZE
                || NULL != strstr(szTmp, ppTypeArr[i])) {
            goto err_out;
        }

        size += snprintf(szTmp + size, LINE_AUTH_METHOD_LINE_SIZE - size, 
                "%s%s", (0 != i) ? "," : "", ppTypeArr[i]);
    }

    snprintf(p_accountcmd->methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", szTmp);
    ret = 0;
err_out:
    free_tokens(&ppTypeArr);
    return ret;
}

static int
_check_accountcmd_field(char *pszkey, char *pszValue, tbl_accountcmd_t *p_accountcmd, cfg_cmd_para_t *para)
{
    cdb_node_t  *p_node = cdb_get_tbl(TBL_ACCOUNTCMD);
    cdb_field_t *p_field = NULL;
    int32       field_id = -1;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_accountcmd_field_id_t)field_id) {
    case TBL_ACCOUNTCMD_FLD_METHODS:
        if (_check_accountcmd_methods(pszValue, p_accountcmd) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid accounting method string");
            goto err_out;
        }
        break;
    default:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unsupportted field %d", field_id);
        goto err_out;
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_accountcmd_add(char *accountcmdname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_accountcmd_master_t *pmaster;
    tbl_accountcmd_t        accountcmd;
    tbl_accountcmd_t        *p_accountcmd = &accountcmd;
    int i;

    sal_memset(p_accountcmd, 0x00, sizeof(tbl_accountcmd_t));
    sal_snprintf(p_accountcmd->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", accountcmdname);

    if (tbl_accountcmd_get_accountcmd(&p_accountcmd->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_accountcmd_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->accountcmd_list) >= GLB_MAX_ACCOUNTCMD_LIST_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum accountcmd limit");
    }

    for (i = 1; i < argc; i += 2) {
        if (_check_accountcmd_field(argv[i], argv[i + 1], p_accountcmd, para) == -1) {
            goto err_out;
        }
    }    

    return tbl_accountcmd_add_accountcmd(p_accountcmd);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

int32
accountcmd_clear_vty_for_methods(char *accountcmd_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32          rc = PM_E_NONE;
    tbl_vty_t      *p_db_vty_item = NULL;
    tbl_vty_t      vty_item;
    uint32         id = 0;

    sal_memset(&vty_item, 0, sizeof(tbl_vty_t));
    
    for (id = 0; id < GLB_MAX_VTY_NUM; id++)
    {
        vty_item.key.id = id;
        p_db_vty_item = tbl_vty_get_vty(&vty_item.key);

        /*added by yejl to fix bug42291, set the accountcmd_method is default*/
        snprintf(vty_item.accountcmd_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
        if (0 == sal_memcmp(p_db_vty_item->accountcmd_method, accountcmd_name, sal_strlen(accountcmd_name)))
        {
            rc |= _vty_cmd_set(TBL_VTY_FLD_ACCOUNTCMD_METHOD, &vty_item, p_db_vty_item, para);
        }
    }

    return PM_E_NONE;
}

static int32
_accountcmd_del(char *accountcmd_name, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_accountcmd_t accountcmd;
    tbl_accountcmd_t *p_accountcmd = &accountcmd;

    accountcmd_clear_vty_for_methods(accountcmd_name, argv, argc, para);
    
    sal_snprintf(p_accountcmd->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", accountcmd_name);
    return tbl_accountcmd_del_accountcmd(&p_accountcmd->key);
}

static int32
_accountcmd_cmd_set_methods(tbl_accountcmd_t *p_accountcmd, tbl_accountcmd_t *p_db_accountcmd, 
    cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (!strcasecmp(p_accountcmd->methods, p_db_accountcmd->methods)) {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_accountcmd_set_accountcmd_field(p_accountcmd, TBL_ACCOUNTCMD_FLD_METHODS));

    return rc;
}

int32
app_accountcmd_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t           *p_node = NULL;
    int32                field_id = TBL_ACCOUNTCMD_FLD_MAX;
    cdb_field_t          *p_field = NULL;
    int32                i = 0;
    field_parse_t        field;
    int32                rc = PM_E_NONE;
    tbl_accountcmd_t     item;
    tbl_accountcmd_t     *p_db_item = NULL;
    tbl_sys_global_t     *p_db_glb = tbl_sys_global_get_sys_global();

    CFG_DBG_FUNC();

    if (!p_db_glb->aaa_new_model)
    {
        CFG_CONFLICT_RET("Please enable aaa first");
        return rc;
    }

    p_node = cdb_get_tbl(TBL_ACCOUNTCMD);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        rc = cdb_check_name(argv[0], (LINE_AUTH_METHOD_NAME_SIZE - 1), para->p_rs->desc);
        if (rc < 0) {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    
        sal_memset(&item, 0, sizeof(tbl_accountcmd_t));
        sal_strncpy(item.key.name, argv[0], LINE_AUTH_METHOD_NAME_SIZE);
        p_db_item = tbl_accountcmd_get_accountcmd(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting commands list %s exist", item.key.name);
        }        
        rc = _accountcmd_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting commands list %s not found", item.key.name);
        }
        rc = _accountcmd_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting commands list %s not found", item.key.name);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_accountcmd_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            switch ((tbl_accountcmd_field_id_t)field_id) {
            case TBL_ACCOUNTCMD_FLD_METHODS:
                rc = _accountcmd_cmd_set_methods(&item, p_db_item, para);
                break;
                
            default:
                break;
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Accounting commands list %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _accountcmd_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*modify by zhaoqd for bug 53534 2019-09-21*/
static int
check_is_set_time_range(char* pszValue, cfg_cmd_para_t *para)
{
    tbl_ace_config_master_t *p_master = tbl_ace_config_get_master();
    tbl_ace_config_t * p_db_ace_config = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    if (NULL == p_master)
    {
        CFG_INVALID_PARAM_RET("Invalid Master!");
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace_config, listnode, next)
       {
            if (sal_strcmp(p_db_ace_config->key.acl_name, pszValue))
            {
                continue;
            }
            if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_TIME_RANGE))
            {
                return 1;
            }
        }
    return 0;
}

/*modify by zhaoqd for bug 53682 2019-09-24*/
static int
check_is_same_aclname(tbl_vty_t* p_vty, tbl_iter_args_t *pargs)
{
    char* aclname = pargs->argv[0];
    int32* type = pargs->argv[1];
    
    if (GLB_ACL_CONFIG_TYPE_NORMAL == *type)
    {
        if (!sal_strcmp(p_vty->ipv6_acl, aclname))
        {
            return 1;
        }
    }
    else if (GLB_ACL_CONFIG_TYPE_IPV6 == *type)
    {
        if (!sal_strcmp(p_vty->ipv4_acl, aclname))
        {
            return 1;
        }
    }
    return 0;
}

/* --- vty --- */
static int
_check_vty_field(char *pszkey, char *pszValue, tbl_vty_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_VTY);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_VTY_FLD_MAX;
    int32 type = 0;
    tbl_iter_args_t args;
    int nValue;
    char szTmp[PASSWD_SIZE];

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_vty_field_id_t)field_id) {
    case TBL_VTY_FLD_TIMEOUT_MIN:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 0 || nValue > M_EXEC_TIMEOUT_MIN_MAX)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid timeout value"); 
            goto err_out;
        }
        pitem->timeout_min = nValue;
        break;
    case TBL_VTY_FLD_TIMEOUT_SEC:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 0 || nValue > M_EXEC_TIMEOUT_SEC_MAX)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid timeout value"); 
            goto err_out;
        }
        pitem->timeout_sec = nValue;
        break;
    case TBL_VTY_FLD_LOGIN:
        nValue = cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, pszValue);
        if (-1 == nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid login method: %s", pszValue); 
            goto err_out;
        }
        pitem->login = nValue;
        break;
    case TBL_VTY_FLD_PRIVILEGE:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 1 || nValue > 4)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid privilege value"); 
            goto err_out;
        }
        pitem->privilege = nValue;
        break;
    case TBL_VTY_FLD_PASSWD:
        if (check_user_password(pszValue) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWD_LIMIT_STR); 
            goto err_out;
        }
        sal_snprintf(pitem->passwd, PASSWD_SIZE, "%s", pszValue);
        memset(pitem->enc_passwd, 0x00, PASSWD_SIZE);
        break;
    case TBL_VTY_FLD_ENC_PASSWD:
        sal_memset(szTmp, 0, sizeof(szTmp));
        if (do_decrypt(M_DEFAULT_PW_STR, pszValue, strlen(pszValue), szTmp, PASSWD_SIZE) != 0)
        {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
            goto err_out;
        }
        if (check_user_password(szTmp) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWD_LIMIT_STR); 
            goto err_out;
        }

        sal_snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", pszValue);
        memset(pitem->passwd, 0x00, PASSWD_SIZE);
        break;
    case TBL_VTY_FLD_AUTH_METHOD:
        if (check_name_character_and_len(pszValue, LINE_AUTH_METHOD_NAME_SIZE) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid authentication method string: %s", pszValue); 
            goto err_out;
        }
        
        sal_snprintf(pitem->auth_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", pszValue);
        break;
    case TBL_VTY_FLD_PROTOCOL:
        sal_snprintf(pitem->protocol, LINE_PROTOCOL_BUF_SIZE, "%s", pszValue);
        break;
    case TBL_VTY_FLD_AUTHOR_METHOD:
        if (check_name_character_and_len(pszValue, LINE_AUTH_METHOD_NAME_SIZE) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid authorization method string: %s", pszValue); 
            goto err_out;
        }
        
        sal_snprintf(pitem->author_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", pszValue);
        break;
    case TBL_VTY_FLD_ACCOUNT_METHOD:
        if (check_name_character_and_len(pszValue, LINE_AUTH_METHOD_NAME_SIZE) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid accounting method string: %s", pszValue); 
            goto err_out;
        }
        
        sal_snprintf(pitem->account_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", pszValue);
        break;
    case TBL_VTY_FLD_ACCOUNTCMD_METHOD:
        if (check_name_character_and_len(pszValue, LINE_AUTH_METHOD_NAME_SIZE) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid accounting commands method string: %s", pszValue); 
            goto err_out;
        }
        
        sal_snprintf(pitem->accountcmd_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", pszValue);
        break;
    case TBL_VTY_FLD_CONFIG_PRIVILEGE:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                /* modified by liwh for bug 46712, 2018-04-03 */
                || (nValue < 0 || nValue > 4)) {
                /* liwh end */
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid config privilege value"); 
            goto err_out;
        }
        pitem->config_privilege = nValue;
        break;
    case TBL_VTY_FLD_CMD_BUF:
        sal_memcpy(pitem->cmd_buf, pszValue, strlen(pszValue));
        break;
    case TBL_VTY_FLD_IS_NOT_SSH_KEY:
        nValue = atoi(pszValue);
        pitem->is_not_ssh_key = nValue;
        break;
    /* modified by yejl for bug 46694 to support ssh aaa, 2018-04-03 */
    case TBL_VTY_FLD_IS_SSH_LOGIN:
        nValue = atoi(pszValue);
        pitem->is_ssh_login = nValue;
        break;
    /*ended by yejl*/
    case TBL_VTY_FLD_IPV4_ACL:
        type = GLB_ACL_CONFIG_TYPE_NORMAL;        /*modify by zhaoqd for bug 53515 2019-09-16*/
        if (check_name_character_and_len(pszValue, ACL_NAME_SIZE) != 0)
        {
            CFG_CONFLICT_RET("The initial character of %s name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20",
                                glb_acl_type_str[type]);
        }
        /*modify by zhaoqd for bug 53534 2019-09-21*/
        if (check_is_set_time_range(pszValue, para))
        {
            CFG_CONFLICT_RET("This ACL is configured time-range, please remove it first!");
        }
        /*modify by zhaoqd for bug 53682 2019-09-24*/
        args.argv[0] = pszValue;
        args.argv[1] = &type;
        if (tbl_vty_iterate((TBL_ITER_CB_FUNC)check_is_same_aclname, &args))
        {
            CFG_CONFLICT_RET("Can not configure the same ACL name with IPv6 aclname!");
        }
        if (!sal_strcmp(pszValue, "default"))
        {
            sal_memset(pszValue, 0, sizeof(pitem->ipv4_acl));
        }
        sal_memcpy(pitem->ipv4_acl, pszValue, strlen(pszValue));
        break;
    case TBL_VTY_FLD_IPV6_ACL:
        
        /*modify by zhaoqd for bug 53701 2019-09-24*/
        if (FALSE == auth_api_is_ipv6_profile())
        {
            CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
        } 
        type = GLB_ACL_CONFIG_TYPE_IPV6;
        if (check_name_character_and_len(pszValue, ACL_NAME_SIZE) != 0)
        {
            CFG_CONFLICT_RET("The initial character of %s name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20",
                                glb_acl_type_str[type]);
        }
        if (check_is_set_time_range(pszValue, para))
        {
            CFG_CONFLICT_RET("This ACL is configured time-range, please remove it first!");
        }
        args.argv[0] = pszValue;
        args.argv[1] = &type;
        if (tbl_vty_iterate((TBL_ITER_CB_FUNC)check_is_same_aclname, &args))
        {
            CFG_CONFLICT_RET("Can not configure the same ACL name with IPv4 aclname!");
        }
        if (!sal_strcmp(pszValue, "default"))
        {
            sal_memset(pszValue, 0, sizeof(pitem->ipv6_acl));
        }
        sal_memcpy(pitem->ipv6_acl, pszValue, strlen(pszValue));
        break;
    case TBL_VTY_FLD_KEY:
    case TBL_VTY_FLD_INUSE:
    case TBL_VTY_FLD_MAX:
    case TBL_VTY_FLD_AAA_PRIVILEGE:
    case TBL_VTY_FLD_AAA_START_TIME:
    case TBL_VTY_FLD_OLD_CMD_BUF:
    
        break;
    default:
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

int32
tbl_vty_dump_one_vty(tbl_vty_t *p_vty, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VTY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (FLD_MATCH(TBL_VTY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_KEY].name,
            p_vty->key.id);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_TIMEOUT_MIN].name,
            p_vty->timeout_min);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_SEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_TIMEOUT_SEC].name,
            p_vty->timeout_sec);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_PROTOCOL].name,
            p_vty->protocol);
    }
    if (FLD_MATCH(TBL_VTY_FLD_LOGIN, p_field->field_id[0]))
    {
        if (!p_db_glb->aaa_new_model)
        {
            sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_LOGIN].name, 
                cdb_enum_val2str(login_method_strs, LOGIN_METHOD_MAX, p_vty->login));
        }
        else
        {
            sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_LOGIN].name, "invalid");
        }
    }
    if (FLD_MATCH(TBL_VTY_FLD_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_PRIVILEGE].name,
            p_vty->privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ENC_PASSWD].name,
            p_vty->enc_passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_PASSWD].name,
            p_vty->passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTH_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_AUTH_METHOD].name,
            p_vty->auth_method);
    }    
    if (FLD_MATCH(TBL_VTY_FLD_INUSE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_INUSE].name,
            p_vty->inuse);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTHOR_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_AUTHOR_METHOD].name,
            p_vty->author_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNT_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ACCOUNT_METHOD].name,
            p_vty->account_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNTCMD_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ACCOUNTCMD_METHOD].name,
            p_vty->accountcmd_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV4_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_IPV4_ACL].name,
            p_vty->ipv4_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV6_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_IPV6_ACL].name,
            p_vty->ipv6_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_AAA_PRIVILEGE].name,
            p_vty->aaa_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CONFIG_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_CONFIG_PRIVILEGE].name,
            p_vty->config_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_START_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_AAA_START_TIME].name,
            p_vty->aaa_start_time);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_NOT_SSH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_IS_NOT_SSH_KEY].name,
            p_vty->is_not_ssh_key);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_SSH_LOGIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_IS_SSH_LOGIN].name,
            p_vty->is_ssh_login);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
tbl_vty_dump_filter(tbl_vty_t *p_vty, tbl_iter_args_t *pargs)
{
    tbl_vty_t *p_filter = (tbl_vty_t *)pargs->argv[2];

    if (NULL == p_filter || p_vty == p_filter) {
        return tbl_vty_dump_one_vty(p_vty, pargs);
    }

    return PM_E_NONE;
}

static int32
_vty_cmd_get(tbl_vty_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_vty_iterate((TBL_ITER_CB_FUNC)tbl_vty_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_vty_set_passwd(tbl_vty_t *pitem, 
    tbl_vty_t *p_db_item, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != pitem->passwd[0] && app_is_password_encryption()) {
        gen_passwd_crypt(pitem->passwd, szTmp, PASSWD_SIZE);
        snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", szTmp);
        memset(pitem->passwd, 0x00, PASSWD_SIZE);
    }

    if (sal_strcmp(pitem->passwd, p_db_item->passwd)) {
        vty_set(TBL_VTY_FLD_PASSWD, pitem);
    }

    if (sal_strcmp(pitem->enc_passwd, p_db_item->enc_passwd)) {
        sal_memset(szTmp, 0, sizeof(szTmp));
        if (strlen(pitem->enc_passwd))
        {
            if (do_decrypt(M_DEFAULT_PW_STR, pitem->enc_passwd, strlen(pitem->enc_passwd), szTmp, PASSWD_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid encrypt password");
            }
        }
        vty_set(TBL_VTY_FLD_ENC_PASSWD, pitem);
    }
    return rc;
}

int32
_vty_cmd_set(tbl_vty_field_id_t field_id, tbl_vty_t *pitem, 
    tbl_vty_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_VTY_FLD_TIMEOUT_MIN:
        if (pitem->timeout_min == p_db_item->timeout_min) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_TIMEOUT_SEC:
        if (pitem->timeout_sec == p_db_item->timeout_sec) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_LOGIN:
        if (pitem->login == p_db_item->login) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_PRIVILEGE:
        if (pitem->privilege == p_db_item->privilege) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_AUTH_METHOD:
        if (!strcasecmp(pitem->auth_method, p_db_item->auth_method)) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_PROTOCOL:
        if (!strcasecmp(pitem->protocol, p_db_item->protocol)) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_AUTHOR_METHOD:
        if (!strcasecmp(pitem->author_method, p_db_item->author_method)) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_ACCOUNT_METHOD:
        if (!strcasecmp(pitem->account_method, p_db_item->account_method)) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_ACCOUNTCMD_METHOD:
        if (!strcasecmp(pitem->accountcmd_method, p_db_item->accountcmd_method)) {
            return PM_E_NONE;
        }
        break;
    case TBL_VTY_FLD_AAA_PRIVILEGE:
    case TBL_VTY_FLD_CONFIG_PRIVILEGE:
    case TBL_VTY_FLD_AAA_START_TIME:
    case TBL_VTY_FLD_CMD_BUF:
    case TBL_VTY_FLD_OLD_CMD_BUF:
    case TBL_VTY_FLD_IS_NOT_SSH_KEY:
    case TBL_VTY_FLD_IS_SSH_LOGIN:
    case TBL_VTY_FLD_IPV4_ACL:
    case TBL_VTY_FLD_IPV6_ACL:
        break;
    case TBL_VTY_FLD_ENC_PASSWD:
    case TBL_VTY_FLD_PASSWD:      
    case TBL_VTY_FLD_INUSE:
    case TBL_VTY_FLD_MAX:  
    case TBL_VTY_FLD_KEY:
    default:
        return PM_E_NONE;
    }

    PM_E_RETURN(vty_set(field_id, pitem));
    return rc;
}


static int32
_cmd_get_max_vty(cfg_cmd_para_t *para)
{
	tbl_vty_master_t *p_master;
	tbl_vty_t *p_db_vty;
    FILE *fp = NULL;
	int max_vty = 0;
	int id;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

	p_master = tbl_vty_get_master();
	for (id = (GLB_MAX_VTY_NUM - 1); id >= 0; id--) {
		p_db_vty = p_master->vty_array[id];
		if (NULL == p_db_vty) {
			continue;
		}

		if (p_db_vty->inuse) {
			max_vty++;
		}
	}


    fprintf(fp, "/max=%d\r\n", max_vty);
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
app_vty_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_vty_field_id_t field_id = TBL_VTY_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_vty_t item;
    tbl_vty_t *p_db_item = NULL;
    int nId;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_authen_t authen;
    tbl_authen_t *p_authen = &authen;
    tbl_author_t author;
    tbl_author_t *p_author = &author;
    tbl_account_t account;
    tbl_account_t *p_account = &account;
    tbl_accountcmd_t  accountcmd;
    tbl_accountcmd_t  *p_accountcmd = &accountcmd;
    char              buf[GLB_MAX_CMD_BUFSZ];
    int               buf_length = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_VTY);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
		if (CDB_OPER_GET == para->oper && 1 == argc && !strcasecmp(argv[0], "max")) {
			/* get currently use vty index max */
			return _cmd_get_max_vty(para);
		}

        nId = atoi(argv[0]);
        if (strspn(argv[0], "0123456789") != strlen(argv[0]) 
                || (nId < 0 || nId >= GLB_MAX_VTY_NUM)) {
            CFG_INVALID_PARAM_RET("Invalid vty index: %s", argv[0]);
        }
    
        sal_memset(&item, 0, sizeof(tbl_vty_t));
        item.key.id = nId;
        p_db_item = tbl_vty_get_vty(&item.key);
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Internal error: can't found vty %s", argv[0]);
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;

    case CDB_OPER_SET:
        if (2 == argc) {
            if (!strcasecmp(argv[1], "passwd") || !strcasecmp(argv[1], "enc_passwd")) { 

                /* modified by liwh for bug 46658, 2018-06-11 */
                if (LOGIN_METHOD_LOGIN == p_db_item->login)
                {
                    CFG_CONFLICT_RET("Can't delete line-password when login");
                    return PM_E_CFG_CONFLICT;
                }
                /* liwh end */
                
                field_id = TBL_VTY_FLD_PASSWD;
                memset(item.passwd, 0x00, PASSWD_SIZE);
                memset(item.enc_passwd, 0x00, PASSWD_SIZE);
                return _vty_set_passwd(&item, p_db_item, para);
            } else if (!strcasecmp(argv[1], "auth_method")) {
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                field_id = TBL_VTY_FLD_AUTH_METHOD;
                memset(item.auth_method, 0x00, LINE_AUTH_METHOD_NAME_SIZE);                
            } else if (!strcasecmp(argv[1], "author_method")) {
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                field_id = TBL_VTY_FLD_AUTHOR_METHOD;
                memset(item.author_method, 0x00, LINE_AUTH_METHOD_NAME_SIZE);                
            } else if (!strcasecmp(argv[1], "account_method")) {
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                field_id = TBL_VTY_FLD_ACCOUNT_METHOD;
                memset(item.account_method, 0x00, LINE_AUTH_METHOD_NAME_SIZE);                
            } else if (!strcasecmp(argv[1], "accountcmd_method")) {
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                field_id = TBL_VTY_FLD_ACCOUNTCMD_METHOD;
                memset(item.accountcmd_method, 0x00, LINE_AUTH_METHOD_NAME_SIZE);  
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _vty_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }

        if (!strcasecmp(argv[1], "cmd_buf")) {
            for (i = 2; i < argc; i += 1) {
                if (i > 2)
                {
                    sal_memcpy(&buf[buf_length], "/", 1);
                    buf_length = buf_length + 1;
                }
                
                sal_memcpy(&buf[buf_length], argv[i], sal_strlen(argv[i]));
                buf_length = buf_length + sal_strlen(argv[i]);               
            }

            field_id = TBL_VTY_FLD_CMD_BUF;
            sal_memcpy(item.cmd_buf, buf, buf_length);
            rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
        
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_vty_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            if (TBL_VTY_FLD_ENC_PASSWD == field_id 
                    || TBL_VTY_FLD_PASSWD == field_id) {
                rc |= _vty_set_passwd(&item, p_db_item, para);    
            } else if (TBL_VTY_FLD_AUTH_METHOD == field_id) {
                /*1. enable aaa new-model */
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                /*2. auth_method list must exist */
                sal_memset(p_authen, 0x00, sizeof(tbl_authen_t));
                sal_snprintf(p_authen->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", argv[2]);
                 
                if (tbl_authen_get_authen(&p_authen->key) == NULL) {
                    CFG_INVALID_PARAM_RET("Authentication list \"%s\" is not defined for LOGIN", argv[2]);
                    return PM_E_NONE;
                }

                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            } else if (TBL_VTY_FLD_AUTHOR_METHOD == field_id) {
                /*1. enable aaa new-model */
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                /*2. author_method list must exist */
                sal_memset(p_author, 0x00, sizeof(tbl_author_t));
                sal_snprintf(p_author->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", argv[2]);
                 
                if (tbl_author_get_author(&p_author->key) == NULL) {
                    CFG_INVALID_PARAM_RET("Authorization list \"%s\" is not defined for LOGIN", argv[2]);
                    return PM_E_NONE;
                }

                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            } else if (TBL_VTY_FLD_ACCOUNT_METHOD == field_id) {
                /*1. enable aaa new-model */
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                /*2. accounting_method list must exist */
                sal_memset(p_account, 0x00, sizeof(tbl_account_t));
                sal_snprintf(p_account->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", argv[2]);
                 
                if (tbl_account_get_account(&p_account->key) == NULL) {
                    CFG_INVALID_PARAM_RET("Account list \"%s\" is not defined for LOGIN", argv[2]);
                    return PM_E_NONE;
                }

                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            } else if (TBL_VTY_FLD_ACCOUNTCMD_METHOD == field_id) {
                /*1. enable aaa new-model */
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }

                /*2. accountingcmd_method list must exist */
                sal_memset(p_accountcmd, 0x00, sizeof(tbl_accountcmd_t));
                sal_snprintf(p_accountcmd->key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", argv[2]);
                 
                if (tbl_accountcmd_get_accountcmd(&p_accountcmd->key) == NULL) {
                    CFG_INVALID_PARAM_RET("Account commands list \"%s\" is not defined for LOGIN", argv[2]);
                    return PM_E_NONE;
                }

                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            } else if (TBL_VTY_FLD_LOGIN == field_id) {
                if (p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please disable aaa first");
                    return PM_E_CFG_CONFLICT;
                }
                /* modified by liwh for bug 46658, 2018-06-11 */
                else if ((LOGIN_METHOD_LOGIN == item.login) && ('\0'== p_db_item->passwd[0]) &&('\0'== p_db_item->enc_passwd[0]))
                {
                    CFG_CONFLICT_RET("Please set line-password first");
                    return PM_E_CFG_CONFLICT;
                }
                /* liwh end */
                
                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            /* modified by liwh for bug 42448, 2017-1-9 */
            } else if (TBL_VTY_FLD_PRIVILEGE == field_id) {
                if (p_db_item->config_privilege)
                {
                    item.config_privilege = item.privilege;
                    rc |= _vty_cmd_set(TBL_VTY_FLD_CONFIG_PRIVILEGE, &item, p_db_item, para);                  
                }
                else
                {
                    rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
                }
            /* liwh end */
            } else {
                rc |= _vty_cmd_set(field_id, &item, p_db_item, para);
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Vty %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vty_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* --- console --- */
static int
_check_console_field(char *pszkey, char *pszValue, tbl_console_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_CONSOLE);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int nValue;
    char szTmp[PASSWD_SIZE];

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_console_field_id_t)field_id) {
    case TBL_CONSOLE_FLD_TIMEOUT_MIN:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 0 || nValue > M_EXEC_TIMEOUT_MIN_MAX)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid timeout value"); 
            goto err_out;
        }
        pitem->timeout_min = nValue;
        break;
    case TBL_CONSOLE_FLD_TIMEOUT_SEC:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 0 || nValue > M_EXEC_TIMEOUT_SEC_MAX)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid timeout value"); 
            goto err_out;
        }
        pitem->timeout_sec = nValue;
        break;
    case TBL_CONSOLE_FLD_LOGIN:
        nValue = cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, pszValue);
        if (-1 == nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid login method"); 
            goto err_out;
        }
        pitem->login = nValue;
        break;
    case TBL_CONSOLE_FLD_PRIVILEGE:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 1 || nValue > 4)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid privilege value"); 
            goto err_out;
        }
        pitem->privilege = nValue;
        break;

    case TBL_CONSOLE_FLD_PASSWD:
        if (check_user_password(pszValue) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWD_LIMIT_STR); 
            goto err_out;
        }
        
        sal_snprintf(pitem->passwd, PASSWD_SIZE, "%s", pszValue);
        memset(pitem->enc_passwd, 0x00, PASSWD_SIZE);
        break;

    case TBL_CONSOLE_FLD_ENC_PASSWD:
        sal_memset(szTmp, 0, sizeof(szTmp));
        if (do_decrypt(M_DEFAULT_PW_STR, pszValue, strlen(pszValue), szTmp, PASSWD_SIZE) != 0)
        {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
            goto err_out;
        }
        if (check_user_password(szTmp) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWD_LIMIT_STR); 
            goto err_out;
        }
        sal_snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", pszValue);
        memset(pitem->passwd, 0x00, PASSWD_SIZE);
        break;
        
    case TBL_CONSOLE_FLD_AUTH_METHOD:
        if (check_name_character_and_len(pszValue, LINE_AUTH_METHOD_NAME_SIZE) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication method"); 
            goto err_out;
        }
        
        sal_snprintf(pitem->auth_method, LINE_AUTH_METHOD_NAME_SIZE, "%s", pszValue);
        break;
    case TBL_CONSOLE_FLD_STOPBITS:
        if (strcasecmp(pszValue, "1") && strcasecmp(pszValue, "2")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid stop bits"); 
            goto err_out;
        }
        pitem->stopbits = atoi(pszValue);
        break;
    case TBL_CONSOLE_FLD_PARITY:
        nValue = cdb_enum_str2val(parity_strs, PARITY_MAX, pszValue);
        if (-1 == nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid parity value"); 
            goto err_out;
        }
        pitem->parity = nValue;
        break;
    case TBL_CONSOLE_FLD_DATABITS:
        if (strcasecmp(pszValue, "7") && strcasecmp(pszValue, "8")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid data bits"); 
            goto err_out;
        }
        pitem->databits = atoi(pszValue);
        break;
    case TBL_CONSOLE_FLD_BAUDRATE:
        if (0 == sal_strcmp(pszValue, "default"))
        {
            pitem->baudrate = GLB_CONSOLE_RATE_DEFAULT;
        }
        else
        {
            nValue = atoi(pszValue);
            if (strspn(pszValue, "0123456789") != strlen(pszValue)
                    || (check_serial_baudrate(nValue) != 0)) {
                snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid buad rate"); 
                goto err_out;
            }
            pitem->baudrate = nValue;
        }
        break;
    case TBL_CONSOLE_FLD_INUSE:
    case TBL_CONSOLE_FLD_MAX:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_console_cmd_get(tbl_console_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_console_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_console_set_passwd(tbl_console_t *pitem, 
    tbl_console_t *p_db_item, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != pitem->passwd[0] && app_is_password_encryption()) {
        gen_passwd_crypt(pitem->passwd, szTmp, PASSWD_SIZE);
        snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", szTmp);
        memset(pitem->passwd, 0x00, PASSWD_SIZE);
    }

    if (sal_strcmp(pitem->passwd, p_db_item->passwd)) {
        console_set(TBL_CONSOLE_FLD_PASSWD, pitem);
    }

    if (sal_strcmp(pitem->enc_passwd, p_db_item->enc_passwd)) {
        sal_memset(szTmp, 0, sizeof(szTmp));
        if (strlen(pitem->enc_passwd))
        {
            if (do_decrypt(M_DEFAULT_PW_STR, pitem->enc_passwd, strlen(pitem->enc_passwd), szTmp, PASSWD_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("Invalid encrypt password");
            }
        }
        console_set(TBL_CONSOLE_FLD_ENC_PASSWD, pitem);
    }
    return rc;
}

int32
_console_cmd_set(tbl_console_field_id_t field_id, tbl_console_t *pitem, 
    tbl_console_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_CONSOLE_FLD_TIMEOUT_MIN:
        if (pitem->timeout_min == p_db_item->timeout_min) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_TIMEOUT_SEC:
        if (pitem->timeout_sec == p_db_item->timeout_sec) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_LOGIN:
        if (pitem->login == p_db_item->login) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_PRIVILEGE:
        if (pitem->privilege == p_db_item->privilege) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_AUTH_METHOD:
        if (!strcasecmp(pitem->auth_method, p_db_item->auth_method)) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_STOPBITS:
        if (pitem->stopbits == p_db_item->stopbits) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_PARITY:
        if (pitem->parity == p_db_item->parity) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_DATABITS:
        if (pitem->databits == p_db_item->databits) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_BAUDRATE:
        if (pitem->baudrate == p_db_item->baudrate) {
            return PM_E_NONE;
        }
        break;
    case TBL_CONSOLE_FLD_ENC_PASSWD:
    case TBL_CONSOLE_FLD_PASSWD:      
    case TBL_CONSOLE_FLD_INUSE:
    case TBL_CONSOLE_FLD_MAX:
        break;
    }

    PM_E_RETURN(console_set(field_id, pitem));
    return rc;
}

int32
app_console_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_console_field_id_t field_id = TBL_CONSOLE_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_console_t item;
    tbl_console_t *p_db_item = NULL;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    //tbl_authen_t authen;
    //tbl_authen_t *p_authen = &authen;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CONSOLE);    
    p_db_item = tbl_console_get_console();
    switch (para->oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        if (1 == argc) {
            if (!strcasecmp(argv[0], "passwd") || !strcasecmp(argv[0], "enc_passwd")) { 
                field_id = TBL_CONSOLE_FLD_PASSWD;
                memset(item.passwd, 0x00, PASSWD_SIZE);
                memset(item.enc_passwd, 0x00, PASSWD_SIZE);
                return _console_set_passwd(&item, p_db_item, para);
            } else if (!strcasecmp(argv[0], "auth_method")) {
                if (!p_db_glb->aaa_new_model)
                {
                    CFG_CONFLICT_RET("Please enable aaa first");
                    return PM_E_CFG_CONFLICT;
                }
                field_id = TBL_CONSOLE_FLD_AUTH_METHOD;
                memset(item.auth_method, 0x00, LINE_AUTH_METHOD_NAME_SIZE);                
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _console_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_console_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) 
        {
            /*modified by yejl to fix bug42301, 2016-12-27*/
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            if (TBL_CONSOLE_FLD_ENC_PASSWD == field_id || TBL_CONSOLE_FLD_PASSWD == field_id) 
            {
                rc |= _console_set_passwd(&item, p_db_item, para);    
            } 
            else if (TBL_CONSOLE_FLD_AUTH_METHOD == field_id ) 
            {
                CFG_INVALID_PARAM_RET("Can't configure 'login authentication' on console");
                return PM_E_NONE;
            } 
            else
            {
                rc |= _console_cmd_set(field_id, &item, p_db_item, para);
            } 
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _console_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- login -- */
static int32
tbl_login_dump_filter(tbl_login_t *p_login, tbl_iter_args_t *pargs)
{
    tbl_login_t *p_login_filter = (tbl_login_t *)pargs->argv[2];

    if (NULL == p_login_filter || p_login == p_login_filter) {
        return tbl_login_dump_one(p_login, pargs);
    }

    return PM_E_NONE;
}

static int32
_login_cmd_get(tbl_login_t *plogin, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = plogin;
    tbl_login_iterate((TBL_ITER_CB_FUNC)tbl_login_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
_check_login_field(char *pszkey, char *pszValue, tbl_login_t *p_login, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_LOGIN);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_LOGIN_FLD_MAX;
    int nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_login_field_id_t)field_id) {
    case TBL_LOGIN_FLD_USER:
        if (cdb_user_str2val(&p_login->user, pszValue, para->p_rs->desc)
                != PM_E_NONE) {
            goto err_out;
        }
        break;
    case TBL_LOGIN_FLD_PID:
        if (strlen(pszValue) != strspn(pszValue, "0123456789")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid pid"); 
            goto err_out;
        }
        p_login->pid = strtoul(pszValue, 0x00, 10);
        break;
    case TBL_LOGIN_FLD_VTY:
        if (strlen(pszValue) >= VTY_BUF_SIZE 
                || strlen(pszValue) != strspn(pszValue, "[0-9a-zA-Z/]")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid vty name"); 
            goto err_out;
        }
        sal_snprintf(p_login->vty, VTY_BUF_SIZE, "%s", pszValue);
        break;
    case TBL_LOGIN_FLD_PRIVILEGE:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)
                || (nValue < 1 || nValue > 4)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid privilege value"); 
            goto err_out;
        }
        p_login->privilege = nValue;
        break;
    case TBL_LOGIN_FLD_EXPIRE_TIME:
        nValue = strtoul(pszValue, NULL, 10);
        if (strspn(pszValue, "0123456789") != strlen(pszValue)) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid expire time"); 
            goto err_out;
        }
        p_login->expire_time = nValue;
        break;
    case TBL_LOGIN_FLD_PROTOCOL:
        snprintf(p_login->protocol, LINE_PROTOCOL_BUF_SIZE, "%s", pszValue);
        break;
    case TBL_LOGIN_FLD_IPADDR:
        snprintf(p_login->ipaddr, IPADDR_BUF_SIZE, "%s", pszValue);
        break;
    case TBL_LOGIN_FLD_DEST_IPADDR:
        snprintf(p_login->dest_ipaddr, IPADDR_BUF_SIZE, "%s", pszValue);
        break;
    case TBL_LOGIN_FLD_REMOTE_PORT:
        p_login->remote_port = atoi(pszValue);
        break;
    case TBL_LOGIN_FLD_DEST_PORT:
        p_login->dest_port = atoi(pszValue);        
        break;
    case TBL_LOGIN_FLD_MAX:
    case TBL_LOGIN_FLD_KEY:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_login_get_max_vty(int max_vty)
{
	tbl_vty_master_t *p_master = tbl_vty_get_master();
	tbl_vty_t *p_db_vty = NULL;
	int cur_vty = 0;
	int id;

	for (id = (GLB_MAX_VTY_NUM - 1); id >= 0; id--)
    {
		p_db_vty = p_master->vty_array[id];
		if (NULL == p_db_vty) {
			continue;
		}

		if (p_db_vty->inuse) {
			cur_vty++;
		}
	}
    if (cur_vty >= max_vty)
    {
        return PM_E_RESOURCE_FULL;
    }
    return PM_E_NONE;
}

int32
login_alloc_vty(char *pszTty, char *pszVtyRet)
{
    tbl_vty_master_t *p_master;
    tbl_vty_t *p_db_vty;
    uint32 id = 0;
	int max_vty = app_get_max_vty();

    /* for console */
    /* Modified by liuht for bug44331, 2017-0619 */
    if (strstr(pszTty, "console") 
            || strstr(pszTty, "ttyS0") 
            || strstr(pszTty, "ttyS1")
            || strstr(pszTty, "ttyAMA0")
            || strstr(pszTty, "ttyAMA1")
            || strstr(pszTty, "ttyS000")) {
        /* always return success for console */
        tbl_console_t console;
        
        memset(&console, 0x00, sizeof(tbl_console_t));
        console.inuse = 1;
        tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_INUSE);
        snprintf(pszVtyRet, VTY_BUF_SIZE, "%s", "con0");
        return 0;
    }

    if (_login_get_max_vty(max_vty) < 0)
    {
        return -1;
    }
    
    /* for vty */
    p_master = tbl_vty_get_master();
    for (id = 0; id < max_vty; id++) {
        p_db_vty = p_master->vty_array[id];
        if (NULL == p_db_vty) {
            continue;
        }

        if (!p_db_vty->inuse) {
            tbl_vty_t vty;
            
            memset(&vty, 0x00, sizeof(tbl_vty_t));
            vty.key.id = id;
            vty.inuse = 1;
            tbl_vty_set_vty_field(&vty, TBL_VTY_FLD_INUSE);
            snprintf(pszVtyRet, VTY_BUF_SIZE, "vty%d", id);
            return 0;
        }
    }

    return -1;
}


int32
login_dealloc_vty(char *pszVty)
{
    tbl_vty_master_t *p_master;
    tbl_vty_t *p_db_vty;
    tbl_vty_t vty;
    uint32 id = 0;

    /* for console */
    if (!strcasecmp(pszVty, "con0")) {
        tbl_console_t console;
        
        memset(&console, 0x00, sizeof(tbl_console_t));
        console.inuse = 0;
        tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_INUSE);
        return 0;
    }

    /* for vty */
    p_master = tbl_vty_get_master();
    if (strncasecmp(pszVty, "vty", 3)) {
        return -1;
    }

    id = atoi(pszVty + 3);
    if (id < 0 || id >= GLB_MAX_VTY_NUM) {
        /* out of range */
        return -1;
    }

    p_db_vty = p_master->vty_array[id];
    if (!p_db_vty->inuse) {
        return 0;
    }
    
    memset(&vty, 0x00, sizeof(tbl_vty_t));
    vty.key.id = id;
    vty.inuse = 0;
    tbl_vty_set_vty_field(&vty, TBL_VTY_FLD_INUSE);
    return 0;
}

/* connection from mgmt-if
  * ip conntrack maybe timeout 
  */
int refineRemoteIP(char *pszRemoteIP, int nSrcPort, char *pszRet, size_t bufsz)
{
    char szBuf[256];
    char szIp[128];
    addr_ipv6_t addrv6;

    if (!strncmp(pszRemoteIP, "::ffff:", IPV4_PREFIX_LEN))
    {
        pszRemoteIP += IPV4_PREFIX_LEN;
    }
    
    if (!strncmp(pszRemoteIP, "169.254.1.", strlen("169.254.1.")) 
            || !strncmp(pszRemoteIP, "169.254.2.", strlen("169.254.2."))
            || !strncmp(pszRemoteIP, "169.254.3.", strlen("169.254.3."))) {
        if (sal_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/" CONNTRACK
                " | "
                "grep \"sport=%d\" | awk '{print $7}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "src=", 4)) {
                snprintf(pszRet, bufsz, "%s", szBuf + 4);
                return 0;
            }
        }
    }

    if (!strncmp(pszRemoteIP, "fec0:fffe:fffe::1:", strlen("fec0:fffe:fffe::1:")) 
            || !strncmp(pszRemoteIP, "fec0:fffe:fffe::2:", strlen("fec0:fffe:fffe::2:")))
    {
        if (sal_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/nf_conntrack"
                " | grep ipv6 | grep \"sport=%d\" | awk '{print $7}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "src=", 4))
            {
                inet_pton(AF_INET6, szBuf + 4, (void*)&addrv6);
                inet_ntop(AF_INET6, (void*)&addrv6, pszRet, bufsz);
                return 0;
            }
        }
    }
    
    snprintf(szIp, 128, pszRemoteIP);
    snprintf(pszRet, bufsz, szIp);
    return 0;
}

static int
_login_add_web_session(char *pszSession, char *pszUser, char *pszPassword,
        char *pszIPAddr, int rmtPort, char *pszErrBuf, size_t nBufsz)
{
    tbl_login_t login;
    tbl_login_t *p_login = &login;
    struct sal_timeval tv;
    int line_index = 0;
    char szIp[128];
    tbl_sys_global_t *p_db_glb = NULL;
    
    sal_memset(p_login, 0x00, sizeof(tbl_login_t));
    sal_snprintf(p_login->key.name, VTY_BUF_SIZE, "%s", pszSession);
    if (tbl_login_get_login(&p_login->key) != NULL) {
        sal_snprintf(pszErrBuf, nBufsz, "Web session exist");
        return -1;
    }

    if (g_web_sessions >= M_WEB_SESSION_MAX) {
        sal_snprintf(pszErrBuf, nBufsz, "Too many web users");
        return -1;
    }
    
    if(!syscmd_file_exist(STARTUP_CFG_DONE_FILE))
    {
        return -1;
    }

    g_web_sessions++;
    sal_snprintf(p_login->user.name, USERNAME_SIZE, "%s", pszUser);
    sal_snprintf(p_login->vty, VTY_BUF_SIZE, "web"); 
    if (NULL == pszIPAddr) {
        pszIPAddr = "unknown";
        memcpy(szIp,"unknown", sizeof("unknown"));
    }
    else
    {
        refineRemoteIP(pszIPAddr, rmtPort, szIp, sizeof(szIp));
    }
    sal_snprintf(p_login->ipaddr, IPADDR_BUF_SIZE, "%s", szIp);
    sal_snprintf(p_login->protocol, LINE_PROTOCOL_BUF_SIZE, "web");
    p_db_glb = tbl_sys_global_get_sys_global();
    
    ctc_task_update_boottime(&tv);
    p_login->expire_time = tv.tv_sec + p_db_glb->http_timeout;
    sal_sscanf(p_login->vty, "vty%d", &line_index);
    snmp_trap_telnet_session_trap(TRUE, line_index, p_login->user.name, p_login->ipaddr);
    logid_sys(LOG_SYSTEM_4_USERLOGIN, p_login->user.name, p_login->ipaddr,
              "login", "web");
    tbl_login_add_login(p_login);
    return 0;
}

static int32
_login_add(char *loginname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szVty[VTY_BUF_SIZE];
    tbl_login_t login;
    tbl_login_t *p_login = &login;
    int i;
    
    sal_memset(p_login, 0x00, sizeof(tbl_login_t));
    sal_snprintf(p_login->key.name, VTY_BUF_SIZE, "%s", loginname);

    if (tbl_login_get_login(&p_login->key) != NULL) {
        return PM_E_NONE;
    }
    
    for (i = 1; i < argc; i += 2) {
        if (_check_login_field(argv[i], argv[i + 1], p_login, para) == -1) {
            goto err_out;
        }
    }
    /* alloca vty for this login session */
    szVty[0] = '\0';
    if (!strcasecmp(p_login->protocol, "web")) {
        if (g_web_sessions >= M_WEB_SESSION_MAX) {
            sal_sprintf(para->p_rs->desc, "Too many web users");
            para->p_rs->rc = PM_E_EXCEED_MAX_NUM;
            return PM_E_EXCEED_MAX_NUM;
        }
        snprintf(szVty, VTY_BUF_SIZE, "%s", "web");
        g_web_sessions++;
    } else if (login_alloc_vty(loginname, szVty) != 0) {
        sal_sprintf(para->p_rs->desc, "Too many users");
        para->p_rs->rc = PM_E_EXCEED_MAX_NUM;
        return PM_E_EXCEED_MAX_NUM;
    }

    sal_snprintf(p_login->vty, VTY_BUF_SIZE, "%s", szVty);
    return tbl_login_add_login(p_login);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_login_del(char *loginname)
{
    tbl_login_t login;
    tbl_login_t *p_login = &login;
    tbl_login_t *p_db_login;
    int line_index = 0;
    
    sal_snprintf(p_login->key.name, VTY_BUF_SIZE, "%s", loginname);
    p_db_login = tbl_login_get_login(&p_login->key);
    sal_sscanf(p_db_login->vty, "vty%d", &line_index);
    if (strcasecmp(p_db_login->protocol, "web") 
            && p_db_login && '\0' != p_db_login->vty[0]) {
        /* terminal no monitor */
        char szFullTty[VTY_BUF_SIZE];
        char *p = szFullTty;
        
        snprintf(szFullTty, VTY_BUF_SIZE, "/dev/%s", p_db_login->key.name);
        while ('\0' != *p) {
            if ('-' == *p) {
                *p = '/';
            }
            p++;
        }
        app_syslog_terminal_monitor(0, szFullTty);
        
        login_dealloc_vty(p_db_login->vty);
    } else {
        if (g_web_sessions > 0) {
            g_web_sessions--;
        }
    }

    if (!sal_strstr(p_db_login->vty, "con")) {
        if ('\0' != p_db_login->protocol[0]) {
            snmp_trap_telnet_session_trap(FALSE, line_index, p_db_login->user.name, p_db_login->ipaddr);
            logid_sys(LOG_SYSTEM_4_USERLOGIN, p_db_login->user.name, p_db_login->ipaddr,
                      "logoff", p_db_login->protocol);
        }
    }
    return tbl_login_del_login(&p_login->key);
}

/* delete tmp file when CLI abnormal exit */
void del_tmpfile_timeout(void *pArg)
{
    u_int32_t *ppid = (u_int32_t *)pArg;
    char szName[64];

    snprintf(szName, 64, "/tmp/recv%u", *ppid);
    sal_free(ppid);
    sal_unlink(szName);
    return;
}

/* delete session when CLI abnormal exit */
int
login_session_del_all_web_session()
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_t *p_db_login = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_master->login_list, p_db_login, listnode, next) {
        if (strcasecmp(p_db_login->protocol, "web")) {
            continue;
        }
        _login_del(p_db_login->key.name);
    }
    
    return PM_E_NONE;
}

/* delete session when CLI abnormal exit */
int
login_session_del_bypid(u_int32_t spid)
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_t *p_db_login = NULL;
    //char szBuf[1024];

    CTCLIB_SLIST_LOOP(p_master->login_list, p_db_login, listnode) {
        if (spid == p_db_login->pid) {
            u_int32_t *ppid = sal_malloc(sizeof(u_int32_t));
            if (NULL != ppid) {
                *ppid = spid;
                ctc_task_add_timer(TASK_PRI_NORMAL, del_tmpfile_timeout, ppid, 60); 
            }
            _login_del(p_db_login->key.name);
            return PM_E_NONE;
        }
    }
    return PM_E_FAIL;
}

/* Expire timer for web sessions */
void
web_session_expire_cb(void *pArg)
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_t *p_db_login = NULL;
    struct sal_timeval tv;
    
    ctc_task_update_boottime(&tv);
    CTCLIB_SLIST_LOOP_DEL(p_master->login_list, p_db_login, listnode, next) {
        if (strcasecmp(p_db_login->protocol, "web")) {
            continue;
        }
        if (p_db_login->expire_time <= tv.tv_sec) {
            /* web session expired */
            _login_del(p_db_login->key.name);
        }
    }
    
    return;
}

static int32
_login_cmd_set(tbl_login_t *p_login, tbl_login_t *p_db_login, 
    cfg_cmd_para_t *para, tbl_login_field_id_t field_id)
{
    int32 rc = PM_E_NONE;
    switch (field_id) {
    case TBL_LOGIN_FLD_USER:
        if (!strcasecmp(p_login->user.name, p_db_login->user.name)) {
            return rc;
        }
        break;
    case TBL_LOGIN_FLD_VTY:
        if (!strcasecmp(p_login->vty, p_db_login->vty)) {
            return rc;
        }
        break;
    case TBL_LOGIN_FLD_PRIVILEGE:
        if (p_login->privilege == p_db_login->privilege) {
            return rc;
        }    
        break;
    case TBL_LOGIN_FLD_PID:
        if (p_login->pid == p_db_login->pid) {
            return rc;
        } 
        break;
    case TBL_LOGIN_FLD_EXPIRE_TIME:
        if (p_login->expire_time == p_db_login->expire_time) {
            return rc;
        } 
        break;
    case TBL_LOGIN_FLD_PROTOCOL:
        if (!strcasecmp(p_login->protocol, p_db_login->protocol)) {
            return rc;
        }    
        break;
    case TBL_LOGIN_FLD_IPADDR:
        if (!strcasecmp(p_login->ipaddr, p_db_login->ipaddr)) {
            return rc;
        }   
        break;
    case TBL_LOGIN_FLD_DEST_IPADDR:
        if (!strcasecmp(p_login->dest_ipaddr, p_db_login->dest_ipaddr)) {
            return rc;
        }   
        break;
    case TBL_LOGIN_FLD_REMOTE_PORT:
         if (p_login->remote_port == p_db_login->remote_port) {
            return rc;
        } 
        break;
    case TBL_LOGIN_FLD_DEST_PORT:
         if (p_login->dest_port == p_db_login->dest_port) {
            return rc;
        } 
        break;
    case TBL_LOGIN_FLD_KEY:
    case TBL_LOGIN_FLD_MAX:
        return rc;
    }

    rc = tbl_login_set_login_field(p_login, field_id);
    return rc;
}

int32
app_login_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_LOGIN_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_login_t item;
    tbl_login_t *p_db_item = NULL;
    int line_index = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_LOGIN);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        if (CDB_OPER_GET == para->oper && 1 == argc 
                && !strcasecmp(argv[0], "web")) {
        } 
        else if (CDB_OPER_GET == para->oper && 1 == argc 
                && !strcasecmp(argv[0], "reboot")) {
        } else {    
            rc = cdb_check_str(argv[0], VTY_BUF_SIZE, para->p_rs->desc);
            if (rc < 0) {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return rc;
            }
        
            sal_memset(&item, 0, sizeof(tbl_login_t));
            sal_strncpy(item.key.name, argv[0], VTY_BUF_SIZE);
            p_db_item = tbl_login_get_login(&item.key);
        }
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Login session for %s exist", item.key.name);
        }        
        rc = _login_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Login session for %s not found", item.key.name);
        }
        rc = _login_del(argv[0]);
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Login session for %s not found", item.key.name);
        }

        if (2 == argc && !strcasecmp(argv[1], "expire_time")) {
            struct sal_timeval tv;
            ctc_task_update_boottime(&tv);
            item.expire_time = tv.tv_sec + M_WEB_SESSION_EXPIRE_TIME;
            rc = _login_cmd_set(&item, p_db_item, para, TBL_LOGIN_FLD_EXPIRE_TIME);;
            break;
        } else if (2 == argc && !strcasecmp(argv[1], "login")) {
            if (!sal_strstr(p_db_item->vty, "con")) {
                sal_sscanf(p_db_item->vty, "vty%d", &line_index);
                snmp_trap_telnet_session_trap(TRUE, line_index, 
                    p_db_item->user.name, p_db_item->ipaddr);
                logid_sys(LOG_SYSTEM_4_USERLOGIN, p_db_item->user.name,
                    p_db_item->ipaddr, "login", p_db_item->protocol);
            }
            break;
        }
        
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_login_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _login_cmd_set(&item, p_db_item, para, field_id);
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && !strcasecmp(argv[0], "web")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else if (1 == argc && !strcasecmp(argv[0], "reboot")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else {
            if (argc > 0 && NULL == p_db_item) {
                CFG_INVALID_PARAM_RET("Login session %s not found", argv[0]);
            }
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
       
        rc = _login_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* -- rsa -- */

static int32
_cmd_get_count(cfg_cmd_para_t *para, int count)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    fprintf(fp, "/count=%d\r\n", count);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
tbl_rsa_dump_filter(tbl_rsa_t *p_rsa, tbl_iter_args_t *pargs)
{
    tbl_rsa_t *p_rsa_filter = (tbl_rsa_t *)pargs->argv[2];

    if (NULL == p_rsa_filter || p_rsa == p_rsa_filter) {
        return tbl_rsa_dump_one(p_rsa, pargs);
    }

    return PM_E_NONE;
}

static int32
_rsa_cmd_get(tbl_rsa_t *prsa, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = prsa;
    tbl_rsa_iterate((TBL_ITER_CB_FUNC)tbl_rsa_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
_check_rsa_field(char *pszkey, char *pszValue, tbl_rsa_t *p_rsa, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_RSA);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_RSA_FLD_MAX;
    int nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_rsa_field_id_t)field_id) {
    case TBL_RSA_FLD_PEM:
        if (sal_b64_pton(pszValue, (unsigned char *)p_rsa->pem.buf, M_RSA_PEM_BUF_SIZE) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid pem string"); 
            goto err_out;
        }
        break;
    case TBL_RSA_FLD_ENC_PASSWD:
    case TBL_RSA_FLD_PASSWD:
        if (check_password(pszValue) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWORD_LIMIT_STR); 
            goto err_out;
        }
        
        if (field_id == TBL_RSA_FLD_PASSWD) {
            sal_snprintf(p_rsa->passwd, PASSWD_SIZE, "%s", pszValue);
            memset(p_rsa->enc_passwd, 0x00, PASSWD_SIZE);
        } else {
            sal_snprintf(p_rsa->enc_passwd, PASSWD_SIZE, "%s", pszValue);
            memset(p_rsa->passwd, 0x00, PASSWD_SIZE);
        }        
        break;
    case TBL_RSA_FLD_TYPE:
        nValue = cdb_enum_str2val(key_type_strs, KEY_TYPE_MAX, pszValue);
        if (-1 == nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid rsa key type"); 
            goto err_out;
        }
        p_rsa->type = nValue;
        break;
    default:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_rsa_key_pem_validate(tbl_rsa_t *p_rsa,  char *pszPem, 
    char *pszErr, size_t err_bufsz, RSA **ppstRsa)
{
    int ret = -1;
    char szPasswd[PASSWD_SIZE];
    char *pszPasswd = NULL;

    if ('\0' != p_rsa->passwd[0]) {
        pszPasswd = p_rsa->passwd;
    } else if ('\0' != p_rsa->enc_passwd[0]) {
        if (do_decrypt(M_DEFAULT_PW_STR, p_rsa->enc_passwd, 
                strlen(p_rsa->enc_passwd), szPasswd, PASSWD_SIZE) != 0) {
            snprintf(pszErr, err_bufsz, "invalid rsa encrypted password");
            goto err_out;
        }
        pszPasswd = szPasswd;
    }

    if (validate_rsa_key_pem((const char *)pszPasswd, pszPem, strlen(pszPem),
            (KEY_TYPE_PRIVATE == p_rsa->type), ppstRsa, pszErr, err_bufsz) != 0) {
        goto err_out;
    }
    ret = 0;
    
err_out:
    return ret;
}

static int32
_rsa_add(char *rsaname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    tbl_rsa_master_t *pmaster;
    tbl_rsa_t rsa;
    tbl_rsa_t *p_rsa = &rsa;
    char szErr[256];
    RSA *pstRsa = NULL;
    int i;
    int rc;

    sal_memset(p_rsa, 0x00, sizeof(tbl_rsa_t));
    sal_snprintf(p_rsa->key.name, M_RSA_KEY_NAMESZ+1, "%s", rsaname);
    if (tbl_rsa_get_rsa(&p_rsa->key) != NULL) {
        return PM_E_NONE;
    }

    pmaster = tbl_rsa_get_master();
    if (CTCLIB_SLISTCOUNT(pmaster->rsa_list) >= GLB_MAX_RSA_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum rsa limit");
    }

    p_rsa->type = KEY_TYPE_PUBLIC;

    for (i = 1; i < argc; i += 2) {
        if (_check_rsa_field(argv[i], argv[i + 1], p_rsa, para) == -1) {
            goto err_out;
        }
    }

    /* validate pem */
    if ('\0' != p_rsa->pem.buf[0] 
            && _rsa_key_pem_validate(p_rsa,  p_rsa->pem.buf, 
                    szErr, 256, &pstRsa) != 0) {
        sal_sprintf(para->p_rs->desc, "Invalid pem string: %s", szErr);
        goto err_out;
    }

    if ('\0' != p_rsa->passwd[0] && app_is_password_encryption())
    {
        gen_passwd_crypt(p_rsa->passwd, szTmp, PASSWD_SIZE);
        memset(p_rsa->passwd, 0x00, PASSWD_SIZE);
        snprintf(p_rsa->enc_passwd, PASSWD_SIZE, "%s", szTmp);
    }

    if (pstRsa) {
        p_rsa->pstRsa = pstRsa; /* save RSA key pointer if have */
    }
    rc = tbl_rsa_add_rsa(p_rsa);
    if (PM_E_NONE != rc && pstRsa) {
        RSA_free(pstRsa);
    }
    return rc;
    
err_out:
    if (pstRsa) {
        RSA_free(pstRsa);
    }
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

int check_rsa_key(char *rsaname, int type, int bNoPasswd, char *pszErr, size_t err_bufsz)
{
    int ret = -1;
    tbl_rsa_key_t key;
    tbl_rsa_t *pRsa;

    memset(&key, 0x00, sizeof(tbl_rsa_key_t));
    sal_snprintf(key.name, M_RSA_KEY_NAMESZ+1, "%s", rsaname);
    pRsa = tbl_rsa_get_rsa(&key);
    if (NULL == pRsa) {
        sal_snprintf(pszErr, err_bufsz, "RSA key %s not found", rsaname);
        goto err_out;
    }
    if (NULL == pRsa->pstRsa) {
        sal_snprintf(pszErr, err_bufsz, "RSA key %s incomplete", rsaname);
        goto err_out;
    }
    if (-1 != type && type != pRsa->type) {
        sal_snprintf(pszErr, err_bufsz, "Please specify RSA %s key", 
            (type == KEY_TYPE_PRIVATE) ? "private" : "public");
        goto err_out;
    }
    if (bNoPasswd && ('\0' != pRsa->passwd[0] || '\0' != pRsa->enc_passwd[0])) {
        sal_snprintf(pszErr, err_bufsz, 
            "Can't use this RSA key for it contain password");
        goto err_out;
    }
    
    ret = 0;
err_out:
    return ret;
}

tbl_rsa_t *find_rsa_key(char *rsaname)
{
    tbl_rsa_t rsa;
    
    sal_snprintf(rsa.key.name, M_RSA_KEY_NAMESZ+1, "%s", rsaname);
    return tbl_rsa_get_rsa(&rsa.key);
}

static int32
_rsa_del(char *rsaname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_rsa_t rsa;
    tbl_rsa_t *p_rsa;
    
    sal_snprintf(rsa.key.name, M_RSA_KEY_NAMESZ+1, "%s", rsaname);
    p_rsa = tbl_rsa_get_rsa(&rsa.key);
    if (NULL == p_rsa) {
        CFG_INVALID_PARAM_RET("Rsa key %s not found", rsaname);
    }

    if (p_rsa->pstRsa) {
        RSA_free(p_rsa->pstRsa);
        p_rsa->pstRsa = NULL;
    }
    return tbl_rsa_del_rsa(&p_rsa->key);
}

static int32
_rsa_cmd_set_pem(tbl_rsa_t *p_rsa, tbl_rsa_t *p_db_rsa, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char szErr[256];
    RSA *pstRsa = NULL;

    if (!strcasecmp(p_rsa->pem.buf, p_db_rsa->pem.buf)) {
        return PM_E_NONE;
    }

    /* validate pem */
    if (_rsa_key_pem_validate(p_db_rsa, p_rsa->pem.buf, szErr, 256, &pstRsa) != 0) {
        CFG_INVALID_PARAM_RET("Invalid pem string: %s", szErr);
    }

    if (p_db_rsa->pstRsa) {
        RSA_free(p_db_rsa->pstRsa);
    }
    p_db_rsa->pstRsa = pstRsa;
    pstRsa = NULL;
    PM_E_RETURN(tbl_rsa_set_rsa_field(p_rsa, TBL_RSA_FLD_PEM));
    return rc;
}

static int32
_rsa_cmd_set_key_type(tbl_rsa_t *p_rsa, tbl_rsa_t *p_db_rsa, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (p_rsa->type == p_db_rsa->type) {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_rsa_set_rsa_field(p_rsa, TBL_RSA_FLD_TYPE));
    return rc;
}

int32
_rsa_key_reference(tbl_rsa_t *p_rsa, int nCnt)
{
    tbl_rsa_t *p_db_rsa = tbl_rsa_get_rsa(&p_rsa->key);

    if (NULL == p_db_rsa) {
        return -1;
    }

    p_rsa->refcnt = p_db_rsa->refcnt + nCnt;
    tbl_rsa_set_rsa_field(p_rsa, TBL_RSA_FLD_REFCNT);
    return 0;
}

static int32
_rsa_cmd_set_passwd(tbl_rsa_t *p_rsa, tbl_rsa_t *p_db_rsa, 
    cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != p_rsa->passwd[0] && app_is_password_encryption()) {
        gen_passwd_crypt(p_rsa->passwd, szTmp, PASSWD_SIZE);
        snprintf(p_rsa->enc_passwd, PASSWD_SIZE, "%s", szTmp);
        memset(p_rsa->passwd, 0x00, PASSWD_SIZE);
    }

    if (sal_strcmp(p_rsa->passwd, p_db_rsa->passwd)) {
        tbl_rsa_set_rsa_field(p_rsa, TBL_RSA_FLD_PASSWD);
    }

    if (sal_strcmp(p_rsa->enc_passwd, p_db_rsa->enc_passwd)) {
        tbl_rsa_set_rsa_field(p_rsa, TBL_RSA_FLD_ENC_PASSWD);
    }
    return rc;
}

int32
app_rsa_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_RSA_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_rsa_t rsa;
    tbl_rsa_t *p_db_rsa = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_RSA);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        if (CDB_OPER_GET == para->oper && 1 == argc && !strcasecmp(argv[0], "count")) {
            tbl_rsa_master_t *pmaster = tbl_rsa_get_master();
            return _cmd_get_count(para, CTCLIB_SLISTCOUNT(pmaster->rsa_list));
        }
        
        rc = cdb_check_name(argv[0], M_RSA_KEY_NAMESZ, para->p_rs->desc);
        if (rc < 0) {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    
        sal_memset(&rsa, 0, sizeof(rsa));
        sal_strncpy(rsa.key.name, argv[0], M_RSA_KEY_NAMESZ);
        p_db_rsa = tbl_rsa_get_rsa(&rsa.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_rsa) {
            CFG_INVALID_PARAM_RET("Rsa key %s exist", rsa.key.name);
        }        
        rc = _rsa_add(argv[0], argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_rsa) {
            CFG_INVALID_PARAM_RET("Rsa key %s not found", rsa.key.name);
        }
        if (p_db_rsa->refcnt > 0) {
            CFG_CONFLICT_RET("Rsa key %s in use", rsa.key.name);
        }
        rc = _rsa_del(argv[0], argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_rsa) {
            CFG_INVALID_PARAM_RET("Rsa key %s not found", rsa.key.name);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_rsa_field(argv[i], argv[i + 1], &rsa, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            switch ((tbl_rsa_field_id_t)field_id) {
            case TBL_RSA_FLD_PEM:
                rc = _rsa_cmd_set_pem(&rsa, p_db_rsa, para);
                break;
            case TBL_RSA_FLD_TYPE:
                rc = _rsa_cmd_set_key_type(&rsa, p_db_rsa, para);
                break;
            case TBL_RSA_FLD_ENC_PASSWD:
            case TBL_RSA_FLD_PASSWD:
                rc = _rsa_cmd_set_passwd(&rsa, p_db_rsa, para);
                break;
                
            default:
                break;
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_rsa) {
            CFG_INVALID_PARAM_RET("Rsa key %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rsa_cmd_get(p_db_rsa, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*******************************************************************************
 * Name:    export_rsa_for_user
 * Purpose: export rsa key ssh and write to to file (rsa1 and rsa)
 * Input: 
 *   pRsa: rsa key
 *   pszFilename: file to export the key
 * Output:
 *   pszErr: error message
 *   err_bufsz: error buffer size
 * Return:
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
export_rsa_for_user(RSA *pRsa, char *pszFilename, char *pszErr, size_t err_bufsz)
{
    int nRet = -1;
    struct sshkey key;
    FILE *fp = NULL;
    
    if (NULL == pRsa || NULL == pszErr || NULL == pszFilename) {
        return -1;
    }

    memset(&key, 0x00, sizeof(struct sshkey));
    /* prepare file */
    if (NULL == (fp = fopen(pszFilename, "w"))) {
        snprintf(pszErr, err_bufsz, "Open file for write failed: %s", strerror(errno));
        goto error_out;
    }
    key.type = KEY_RSA;
    key.rsa = pRsa;
    if (sshkey_write(&key, fp)) {
        snprintf(pszErr, err_bufsz, "Export RSA public key failed");
        goto error_out;
    }
    /*key.type = KEY_RSA1;
    fprintf(fp, "\n");
    if (!sshkey_write(&key, fp)) {
        snprintf(pszErr, err_bufsz, "Export RSA1 public key failed");
        goto error_out;
    }*/
    nRet = 0;
    
error_out:
    if (NULL != fp) {
        fclose(fp);
    }

    return nRet;
}

int32
tbl_cpu_utilization_iterate_update(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_utilization_master_t *p_master = _g_p_tbl_cpu_utilization_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnodenext = NULL;
    tbl_cpu_utilization_t *p_db_cpu_utilize = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP_DEL(p_master->cpu_utilize_list, p_db_cpu_utilize, listnode, listnodenext)
    {
        rc |= fn(p_db_cpu_utilize, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

static int32
tbl_cpu_utilization_check_update(tbl_cpu_utilization_t *p_cpu_utilize, tbl_iter_args_t *pargs)
{
    char comm_name[100][COMM_NAME_SIZE];
    int *ti = pargs->argv[2];
    int i, j;
    bool updated = FALSE;

    i = *ti;
    sal_memcpy(comm_name, pargs->argv[1], sizeof(comm_name));
    for(j = 0; j < i; j++)
    {
        if(!sal_strcmp(p_cpu_utilize->key.name, comm_name[j]))
        {
            updated = TRUE;
        }
    }

    if(updated == FALSE)
    {
        tbl_cpu_utilization_del_cpu_utilization(&p_cpu_utilize->key);
    }

    return PM_E_NONE;
}

typedef enum 
{
    APP_PY_TYPE_WEB = 0,
    APP_PY_TYPE_SNMP = 1,
    APP_PY_TYPE_TRAP = 2,
    APP_PY_TYPE_MAX = 3
} app_python_type_t;

int32
app_cmd_get_python_pid(uint32 *py_pids)
{
    char line[CMD_BUF_256];
    char user[CMD_BUF_16];
    FILE *fp = NULL;
    uint32 pid = 0;
    sal_system("ps -ef | grep python > "PYTHON_PS_PATH);

    fp = sal_fopen(PYTHON_PS_PATH, "r");
    while(sal_fgets(line, MAX_CPU_LINE_CHAR, fp) != NULL)
    {
        pid = 0;
        if (sal_strstr(line, "web"))
        {
            sal_sscanf(line, "%s%u", user, &pid);
            py_pids[APP_PY_TYPE_WEB] = pid;
        }
        else if (sal_strstr(line, "snmp_agent"))
        {
            sal_sscanf(line, "%s%u", user, &pid);
            py_pids[APP_PY_TYPE_SNMP] = pid;
        }
        else if (sal_strstr(line, "trap_agent"))
        {
            sal_sscanf(line, "%s%u", user, &pid);
            py_pids[APP_PY_TYPE_TRAP] = pid;
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

char*
app_cmd_get_python_pid_desc(uint32 *py_pids, uint32 pid)
{
    if (py_pids[APP_PY_TYPE_WEB] == pid)
    {
        return "python(web)";
    }
    else if (py_pids[APP_PY_TYPE_SNMP] == pid)
    {
        return "python(snmp)";
    }
    else if (py_pids[APP_PY_TYPE_TRAP] == pid)
    {
        return "python(trap)";
    }

    return "python";
}

int32
app_cmd_get_cpu_utilization_ctctop(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    FILE *pFile = NULL;
    char strLine[MAX_CPU_LINE_CHAR];
    char strName[MAX_LINE_CHAR];
    tbl_cpu_utilization_t* p_cpu_info = NULL;
    tbl_cpu_utilization_t cpu_info;
    tbl_iter_args_t args;
    float usage; 
    char comm_name[100][COMM_NAME_SIZE];
    int i = 0;
    
    uint32 parse_ok = FALSE;
    uint32 cpu_total_line = FALSE;
    uint32 pid_line = FALSE;    
    uint32 user_time = 0;
    uint32 nice_time = 0;
    uint32 sys_time = 0;
    uint32 idle_time = 0;
    uint32 iow_time = 0;
    uint32 irq_time = 0;
    uint32 sirq_time = 0;
    uint32 total_time = 0;
    uint32 pid = 0;
    uint32 py_pids[APP_PY_TYPE_MAX];
    char *updateName = NULL;

    sal_memset(py_pids, 0, sizeof(py_pids));    
    sal_memset(&cpu_info, 0, sizeof(cpu_info));
    sal_memset(comm_name, 0, sizeof(comm_name));

    pFile = sal_fopen(CPU_INFO_PATH, "r");
    
    if((NULL == pFile) || (sal_feof(pFile)))
    {
        /* system modified by liwh for bug 52951, 2019-08-08 */
        //CFG_INVALID_PARAM_RET("Read file /tmp/cpu_usage failed");
        CFG_INVALID_PARAM_RET("CPU utilization is counting");
        /* liwh end */
        return -1;
    }

    app_cmd_get_python_pid(py_pids);

    sal_memset(strLine, 0, sizeof(strLine));
    sal_memset(strName, 0, sizeof(strName));

    while(sal_fgets(strLine, MAX_CPU_LINE_CHAR, pFile) != NULL)
    {
        if (0 == sal_strncmp(strLine, "User  Nice", 10))
        {
            cpu_total_line = TRUE;
            continue;
        }

        if (0 == sal_strncmp(strLine, "PID   CPU", 9))
        {
            pid_line = TRUE;
            continue;
        }
        
        if (cpu_total_line)
        {
            sal_sscanf(strLine, "%d%d%d%d%d%d%d%d", &user_time, &nice_time, &sys_time, &idle_time, &iow_time, &irq_time, &sirq_time, &total_time);
            cpu_total_line = FALSE;
            if (total_time)
            {
                sal_strcpy(comm_name[i], "Total");
                i++;
                sal_memset(&cpu_info, 0, sizeof(cpu_info));
                sal_strcpy(cpu_info.key.name, "Total");
                cpu_info.cpu_usage = (100.0 * (total_time - idle_time)) / total_time;
                p_cpu_info = tbl_cpu_utilization_get_cpu_utilization(&cpu_info.key);
                if (NULL == p_cpu_info)
                {
                    tbl_cpu_utilization_add_cpu_utilization(&cpu_info);
                }
                else
                {
                    p_cpu_info->cpu_usage = cpu_info.cpu_usage;
                    tbl_cpu_utilization_set_cpu_utilization_field(p_cpu_info, TBL_CPU_UTILIZATION_FLD_CPU_USAGE);
                }
                parse_ok = TRUE;
            }
        }
        
        if (pid_line)
        {
            sal_memset(&cpu_info, 0, sizeof(cpu_info));
            sal_sscanf(strLine, "%d%f%s", &pid, &usage, strName);
            if (0 == sal_strcmp("ctctop", strName))
            {
                continue;
            }
            sal_memset(&cpu_info, 0, sizeof(cpu_info));
            sal_snprintf(cpu_info.key.name, COMM_NAME_SIZE, "%d#", pid);
            if (0 == sal_strcmp(strName, "python"))
            {
                updateName = app_cmd_get_python_pid_desc(py_pids, pid);
            }
            else
            {
                updateName = strName;
            }
            sal_strcat(cpu_info.key.name, updateName);
            sal_strcpy(comm_name[i], cpu_info.key.name);
            i++;
            cpu_info.cpu_usage = usage;
            p_cpu_info = tbl_cpu_utilization_get_cpu_utilization(&cpu_info.key);
            if (NULL == p_cpu_info)
            {
                tbl_cpu_utilization_add_cpu_utilization(&cpu_info);
            }
            else
            {
                p_cpu_info->cpu_usage = cpu_info.cpu_usage;
                tbl_cpu_utilization_set_cpu_utilization_field(p_cpu_info, TBL_CPU_UTILIZATION_FLD_CPU_USAGE);
            }
        }
    }

    sal_fclose(pFile);
  
    sal_memset(&args, 0 ,sizeof(args));
 
    args.argv[1] = comm_name;
    args.argv[2] = &i;

    if (parse_ok)
    {
        tbl_cpu_utilization_iterate_update((TBL_ITER_CB_FUNC)tbl_cpu_utilization_check_update, &args);
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_cpu_utilization_iterate((TBL_ITER_CB_FUNC)tbl_cpu_utilization_dump_one, &args);

    sal_fclose(fp);
 
    return PM_E_NONE;
}

int32
app_cmd_get_cpu_process_desc(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-16s %-s\n", "Process", "Description");
    sal_fprintf(fp, "----------------+------------------------------------------------------------\n");
    sal_fprintf(fp, "%-16s %-s\n", "authd",         "Authentication process");
    sal_fprintf(fp, "%-16s %-s\n", "appcfg",        "Application configuration process");
    sal_fprintf(fp, "%-16s %-s\n", "bhm",           "Heart Beat Monitor process");
//    sal_fprintf(fp, "%-16s %-s\n", "bgpd",          "Border Gateway Protocol process");
    sal_fprintf(fp, "%-16s %-s\n", "ccs",           "Central configure service process");
    sal_fprintf(fp, "%-16s %-s\n", "cds",           "Central data service process");
    sal_fprintf(fp, "%-16s %-s\n", "chsm",          "Board and devices (fan, fiber, sensor) management process");
    sal_fprintf(fp, "%-16s %-s\n", "dhclient",      "Dhcp client process");
    sal_fprintf(fp, "%-16s %-s\n", "dhcrelay",      "Dhcp relay process");
    sal_fprintf(fp, "%-16s %-s\n", "dhcsnooping",   "Dhcp snooping process");
    sal_fprintf(fp, "%-16s %-s\n", "fea",           "Forwarding engine abstraction process, include adapter and SDK");
    sal_fprintf(fp, "%-16s %-s\n", "ntpd",          "NTP server process");
    sal_fprintf(fp, "%-16s %-s\n", "opm",           "Features need fast switchover (e.g. ERPS) process");
    sal_fprintf(fp, "%-16s %-s\n", "ospfd",         "Open Shortest Path First process");
#ifdef OFPRODUCT
    sal_fprintf(fp, "%-16s %-s\n", "ovsdb-server",  "OVS database server process");
    sal_fprintf(fp, "%-16s %-s\n", "ovs-vswitchd",  "OVS vswitch process");
#endif
    sal_fprintf(fp, "%-16s %-s\n", "ptp",           "Precision Time Protocol process");
    sal_fprintf(fp, "%-16s %-s\n", "python",        "Python process for WEB or SNMP agent");
//    sal_fprintf(fp, "%-16s %-s\n", "ripd",          "Routing Information Protocol process");
    sal_fprintf(fp, "%-16s %-s\n", "routed",        "Layer3 features (IPUC, ARP) management process");
    sal_fprintf(fp, "%-16s %-s\n", "snmpd",         "SNMP server process");
    sal_fprintf(fp, "%-16s %-s\n", "switch",        "Interface and some layer2 features management process");
    sal_fprintf(fp, "%-16s %-s\n", "sshd",          "SSH server process");
    sal_fprintf(fp, "%-16s %-s\n", "syslog-ng",     "System log server process");
    sal_fprintf(fp, "%-16s %-s\n", "telnetd",       "Telnet server process");
    sal_fprintf(fp, "%-16s %-s\n", "zebra",         "Layer3 RIB/FIB process");

    sal_fclose(fp);
 
    return PM_E_NONE;
}

int32
app_cmd_process_cpu_utilization(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CPU_UTILIZATION);

    switch (para->oper)
    {
    
    case CDB_OPER_GET:
        if ((1 == argc) && (0 == sal_strcmp("description", argv[0])))
        {
            rc = app_cmd_get_cpu_process_desc(para);
        }
        else
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = app_cmd_get_cpu_utilization_ctctop(para, p_node, &field);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;

}

static int32
tbl_cpu_limit_update(tbl_cpu_limit_t *p_cpu_limit, tbl_iter_args_t *pargs)
{
    char cmd[1024];
    pid_t pid;

    app_get_pid_by_name(p_cpu_limit->key.name, &pid);
    
    if(pid == 0)
    {
        return PM_E_NONE;
    }
    
    sprintf(cmd, "cpulimit -p %d -l %d -b", pid, p_cpu_limit->percent);
    system(cmd);
          
    return PM_E_NONE;
}


static int 
app_control_pm_cpu_limit()
{  
    tbl_iter_args_t args;
   
 
    system("killall cpulimit");
 
    sal_memset(&args, 0 ,sizeof(args));
    tbl_cpu_limit_iterate((TBL_ITER_CB_FUNC)tbl_cpu_limit_update, &args);
         
    return 0;
}
static int app_read_to_buf(const char *filename, void *buf)
{
    FILE *fd;

    fd = sal_fopen(filename, "r");
    if(fd < 0)
        return -1;
    sal_fgets(buf, 1023, fd );
    
    sal_fclose(fd);
    return 0;
}

int app_get_pid_by_name(char* name, pid_t* ppid)
{
    static DIR *dir;
    struct dirent *entry;
    int n;
    char *comm;
    char status[32];
    char *status_tail;
    char buf[1024];
    int pid;
    struct stat sb;

    dir = opendir("/proc");
    do 
    {
        if((entry = readdir(dir)) == NULL) 
        {
            closedir(dir);
            return -1;
        }
        comm = entry->d_name;
        if (!(*comm >= '0' && *comm <= '9'))
            continue;
        pid = atoi(comm);

        status_tail = status + sprintf(status, "/proc/%d", pid);
        if(stat(status, &sb))
            continue;

        sal_strcpy(status_tail, "/stat");
        n = app_read_to_buf(status, buf);
        		
        if(n < 0)
           continue;
        		
        comm = strrchr(buf, ')'); /*split into "PID (cmd" and "<rest>" */
        if(comm == 0 || comm[1] != ' ')
            continue;
        *comm = 0;	/*replace ')' with '\0'*/ 
        comm = sal_strrchr(buf, '(');	/*get comm start point*/
        comm++;			/*skip '('*/

        if( sal_strcmp(comm, name)==0 )
        {
            *ppid=pid;
            return 0;
        }
    }
    while(1);

}

static int32
_app_cmd_get_cpu_limit(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_cpu_limit_iterate((TBL_ITER_CB_FUNC)tbl_cpu_limit_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_cpu_limit_set_cpu_limit(char *pm_name, char *percent)
{
    tbl_cpu_limit_key_t cpu_limit_key;
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;
    tbl_cpu_limit_t  p_cpu_limit;
    int32 rc = PM_E_NONE;

    sal_strncpy(cpu_limit_key.name, pm_name, COMM_NAME_SIZE);
    p_db_cpu_limit = tbl_cpu_limit_get_cpu_limit(&cpu_limit_key);
    
    if(!sal_strcmp(percent, "0"))
    {
        if(p_db_cpu_limit != NULL)
        {
            tbl_cpu_limit_del_cpu_limit(&cpu_limit_key);
        }
        else
        {
            return PM_E_NONE;
        }
    }
    else if(p_db_cpu_limit != NULL)
    {
        p_cpu_limit.percent = cdb_int_str2val(percent, &rc);

        if (rc < 0)
        {
            return PM_E_NONE;
        }
        if(p_db_cpu_limit->percent == p_cpu_limit.percent)
        {
            return PM_E_NONE;
        }
        p_db_cpu_limit->percent = p_cpu_limit.percent;
        rc = tbl_cpu_limit_set_cpu_limit_field(p_db_cpu_limit, TBL_CPU_LIMIT_FLD_PERCENT);
        
    }
    else
    {
         sal_strncpy(p_cpu_limit.key.name, pm_name, COMM_NAME_SIZE);
         p_cpu_limit.percent = cdb_int_str2val(percent, &rc);
         if (rc < 0)
         {
             return PM_E_NONE;
         }
         rc = tbl_cpu_limit_add_cpu_limit(&p_cpu_limit);
    }
    app_control_pm_cpu_limit();
    return rc;
   
}

int32
app_cmd_process_cpu_limit(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CPU_LIMIT);
    if (0 == argc)
    {
        if (CDB_OPER_GET != para->oper) 
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } 
    else 
    {
        rc = cdb_check_name(argv[0], COMM_NAME_SIZE, para->p_rs->desc);
        if (rc < 0) 
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
     }
    switch (para->oper)
    {
    case CDB_OPER_SET:
        rc = _app_cpu_limit_set_cpu_limit(argv[0], argv[1]);
        break;

    case CDB_OPER_GET:

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _app_cmd_get_cpu_limit(para, p_node, &field);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;

}

/* web login */
/*
 * input format:
 *
 *  cdbctl update/cdb/sys/web_login/SESSIONID/user/USERNAME/passwd/PASSWORD/ipaddr/IPADDR/sport/PORT
 *
 *   SESSIONID is md5 string
 */
int32
app_web_login_act_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char szErrBuf[256];
    int32 rc = PM_E_NONE;
    int i;
    char *pszSession = NULL;
    char *pszUser = NULL;
    char *pszPassword = NULL;
    char *pszIPAddr = NULL;
    int RmtPort = 0;
    char szDecUser[USERNAME_SIZE];
    tbl_user_t user;
    tbl_user_t *p_db_user = NULL;
    CFG_DBG_FUNC();

    switch (para->oper) {
    case CDB_OPER_SET:
        if ((argc % 2) != 1 || (argc < 7)) {
            /* create session */
            CFG_INVALID_PARAM_RET("Invalid parameter");
        } 
        pszSession = argv[0];
        for (i = 1; i < argc; i += 2)
        {
            if (!strcasecmp(argv[i], "user")) 
            {
                pszUser = argv[i + 1];
               
                if (sal_b64_pton(pszUser, (unsigned char *)szDecUser, USERNAME_SIZE) == -1) 
                {
                    CFG_INVALID_PARAM_RET("Invalid user name");
                }
                 sal_strcpy(user.key.name, szDecUser);
            } 
            else if (!strcasecmp(argv[i], "passwd")) 
            {
                pszPassword = argv[i + 1];
            } 
            else if (!strcasecmp(argv[i], "ipaddr")) 
            {
                pszIPAddr = argv[i + 1];
            }
            else if (!strcasecmp(argv[i], "sport")) 
            {
                PM_CFG_GET_UINT(RmtPort, argv[i + 1], para->p_rs);
            }
        }

        p_db_user = tbl_user_get_user(&user.key);
        if (NULL == p_db_user)
        {
            CFG_INVALID_PARAM_RET("User is not found");
        }
        
        if (strspn(pszSession, "0123456789abcdefABCDEF") != strlen(pszSession)
                || strlen(pszSession) >= VTY_BUF_SIZE) {
            CFG_INVALID_PARAM_RET("Invalid session");
        }

        if ('\0' == szDecUser[0]|| NULL == pszPassword) {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }

        if (g_web_sessions >= M_WEB_SESSION_MAX) {
            CFG_INVALID_PARAM_RET("Too many web users");
        }

        if (_user_validate(szDecUser, pszPassword, szErrBuf, 256) != 0) {
            CFG_INVALID_PARAM_RET(szErrBuf);
        }

        if (!(p_db_user->service_type&HOST_USER_SERVICE_TYPE_WEB))
        {
            CFG_INVALID_PARAM_RET("User is not supported for web");
        }

        if (_login_add_web_session(pszSession, szDecUser, pszPassword, 
                pszIPAddr, RmtPort, szErrBuf, 256) != 0) {
            CFG_INVALID_PARAM_RET(szErrBuf);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

/* Change username: for CLI re-username */
/*
 * input format:
 *
 *  cdbctl update/cdb/sys/change_username/OLD-NAME/NEW-NAME
 */
int32
app_change_username_act_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char *pszOldName = NULL;
    char *pszNewName = NULL;
    char err_msg[MAX_CMD_RET_LEN] = {0};
    tbl_user_t user;
    tbl_user_t *p_db_user_old = NULL;
    tbl_user_t *p_db_user_new;


    CFG_DBG_FUNC();

    switch (para->oper) {
    case CDB_OPER_SET:
        if (2 != argc) {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }

        pszOldName = argv[0];
        pszNewName = argv[1];
        
        rc = cdb_check_name(pszOldName, USERNAME_CLI_SIZE-1,err_msg);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid old username: %s.", err_msg);
        }
        if (!strcasecmp(pszOldName, "admin")) { /* can't change user admin */
            CFG_INVALID_PARAM_RET("Can't rename factory default user");
        }

        rc = cdb_check_name(pszNewName, USERNAME_CLI_SIZE-1, err_msg);
        if (rc < 0 || !strcasecmp(pszNewName, "root")) {
            CFG_INVALID_PARAM_RET("Invalid new username: %s.", err_msg);
        }

        sal_memset(&user, 0, sizeof(user));
        sal_strncpy(user.key.name, pszOldName, USERNAME_SIZE);
        p_db_user_old = tbl_user_get_user(&user.key);
        if (NULL == p_db_user_old) {
            CFG_INVALID_PARAM_RET("Username %s doesn't exist", pszOldName);
        }
        
        sal_memset(&user, 0, sizeof(user));
        sal_strncpy(user.key.name, pszNewName, USERNAME_SIZE);
        p_db_user_new = tbl_user_get_user(&user.key);
        if (NULL != p_db_user_new) {
            CFG_INVALID_PARAM_RET("Username %s is already exist", pszNewName);
        }

        sal_memcpy(&user, p_db_user_old, sizeof(tbl_user_t));
        if (tbl_user_del_user(&p_db_user_old->key) != PM_E_NONE) {
            CFG_FUNC_ERR_RET("Can't delete old username %s", pszOldName);
        }

        sal_snprintf(user.key.name, USERNAME_SIZE, "%s", pszNewName); 
        if (tbl_user_add_user(&user) != PM_E_NONE) {
            CFG_FUNC_ERR_RET("Can't create new username %s", pszNewName);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}


int32
app_send_trap_act_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_act(ACT_SEND_TRAP);

    switch (para->oper) {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown sub-action %s", argv[0]);
        }

        if (0 == field_id) /* write_success */
        {
            snmp_trap_write_config(TRUE);
            logid_sys(LOG_SYSTEM_4_WRITECONFIGURE, "success");
        }
        else if (1 == field_id) /* write_failure */
        {
            snmp_trap_write_config(FALSE);
            logid_sys(LOG_SYSTEM_4_WRITECONFIGURE, "failure");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

/* --- enable table --- */
static int
_check_enable_field(char *pszkey, char *pszValue, tbl_enable_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_ENABLE);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_ENABLE_FLD_MAX;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_enable_field_id_t)field_id) {
    case TBL_ENABLE_FLD_ENC_PASSWD:
    case TBL_ENABLE_FLD_PASSWD:
        if (check_user_password(pszValue) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, 
                "Invalid password string: %s", M_PASSWD_LIMIT_STR); 
            goto err_out;
        }

        if (field_id == TBL_ENABLE_FLD_PASSWD) {
            sal_snprintf(pitem->passwd, PASSWD_SIZE, "%s", pszValue);
            memset(pitem->enc_passwd, 0x00, PASSWD_SIZE);
        } else {
            sal_snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", pszValue);
            memset(pitem->passwd, 0x00, PASSWD_SIZE);
        }
        break;
    case TBL_ENABLE_FLD_KEY:
    case TBL_ENABLE_FLD_MAX:
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
tbl_enable_dump_filter(tbl_enable_t *p_enable, tbl_iter_args_t *pargs)
{
    tbl_enable_t *p_filter = (tbl_enable_t *)pargs->argv[2];

    if (NULL == p_filter || p_enable == p_filter) {
        return tbl_enable_dump_one(p_enable, pargs);
    }

    return PM_E_NONE;
}

static int32
_enable_cmd_get(tbl_enable_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_enable_iterate((TBL_ITER_CB_FUNC)tbl_enable_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_enable_set_passwd(tbl_enable_t *pitem, 
    tbl_enable_t *p_db_item, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    sal_memset(szTmp, 0, sizeof(szTmp));
    if (sal_strlen(pitem->enc_passwd)) {
        rc = do_decrypt(M_DEFAULT_PW_STR, pitem->enc_passwd, strlen(pitem->enc_passwd), szTmp, PASSWD_SIZE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid encrypt password");
        }
    }
    
    if ('\0' != pitem->passwd[0] && app_is_password_encryption()) {
        gen_passwd_crypt(pitem->passwd, szTmp, PASSWD_SIZE);
        snprintf(pitem->enc_passwd, PASSWD_SIZE, "%s", szTmp);
        memset(pitem->passwd, 0x00, PASSWD_SIZE);
    }

    if (sal_strcmp(pitem->passwd, p_db_item->passwd)) {
        tbl_enable_set_enable_field(pitem, TBL_ENABLE_FLD_PASSWD);
    }

    if (sal_strcmp(pitem->enc_passwd, p_db_item->enc_passwd)) {
        tbl_enable_set_enable_field(pitem, TBL_ENABLE_FLD_ENC_PASSWD);
    }
    return rc;
}

static int32
_enable_cmd_set(tbl_enable_field_id_t field_id, tbl_enable_t *pitem, 
    tbl_enable_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_ENABLE_FLD_ENC_PASSWD:
    case TBL_ENABLE_FLD_PASSWD: 
        break;
    case TBL_ENABLE_FLD_KEY:
    case TBL_ENABLE_FLD_MAX:
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_enable_set_enable_field(pitem, field_id));
    return rc;
}

int32
app_enable_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_enable_field_id_t field_id = TBL_ENABLE_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_enable_t item;
    tbl_enable_t *p_db_item = NULL;
    int nId;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_ENABLE);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        nId = atoi(argv[0]);
        if (strspn(argv[0], "0123456789") != strlen(argv[0]) 
                || (nId < 0 || nId >= GLB_MAX_PRIVILEAGE_NUM)) {
            CFG_INVALID_PARAM_RET("Invalid enable privilege level: %s", argv[0]);
        }
    
        sal_memset(&item, 0, sizeof(tbl_enable_t));
        item.key.id = nId;
        p_db_item = tbl_enable_get_enable(&item.key);
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Internal error: can't found enable %s", argv[0]);
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;

    case CDB_OPER_SET:
        if (2 == argc) {
            if (!strcasecmp(argv[1], "passwd") || !strcasecmp(argv[1], "enc_passwd")) { 
                field_id = TBL_ENABLE_FLD_PASSWD;
                memset(item.passwd, 0x00, PASSWD_SIZE);
                memset(item.enc_passwd, 0x00, PASSWD_SIZE);
                return _enable_set_passwd(&item, p_db_item, para);             
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _enable_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_enable_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            if (TBL_ENABLE_FLD_ENC_PASSWD == field_id 
                    || TBL_ENABLE_FLD_PASSWD == field_id) {
                rc |= _enable_set_passwd(&item, p_db_item, para);
            }
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("enable privilege level %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _enable_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


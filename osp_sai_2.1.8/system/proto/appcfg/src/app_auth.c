
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "proto.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_enable.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_login_security_glb_define.h"
#include "gen/tbl_login_security_glb.h"
#include "gen/tbl_login_record_define.h"
#include "gen/tbl_login_record.h"
#include "appcfg.h"
#include "app_auth.h"
#include "app_serial.h"
#include "param_check.h"
#include "rsa_derhex.h"
#include "glb_tempfile_define.h"
#include "ctc_sock.h"


/* socket for determine CLI abnormal exit 
 * when telnetd kill (SIGKILL) klish the socket will disconnect
 * we need close the login session and process terminla monitor
 */
static int g_CliSock = -1;
ctc_task_t           *t_login_security_timer = NULL;
/* web session expire timer */
ctc_task_t *g_p_web_session_timeout = NULL;
extern void web_session_expire_cb(void *);

void cli_sock_read(void* p_data)
{
    ctc_sock_session_t *p_session = (ctc_sock_session_t *)p_data;
    int sock = p_session->fd;
    int nRead = 0;
    char szBuf[32];
    u_int32_t peer;

    while (1) {
        memset(szBuf, 0x00, sizeof(szBuf));
        nRead = read(sock, szBuf, 32);
        if (nRead > 0) {
            break;
        }

        if (0 == nRead) { /* Connection closed */
            _ctc_sock_session_close(p_session);
            return;
        }

        switch (errno) {
        case EINTR:
        case EAGAIN:
        case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)        
        case EWOULDBLOCK:
#endif
            continue;
        }

        return;
    }

    /* save peer pid */
    peer = strtoul(szBuf, NULL, 10);
    p_session->peer = peer;
    return;
}

extern int login_session_del_bypid(u_int32_t spid);
int32 cli_session_done(ctc_sock_session_t* p_session)
{
    login_session_del_bypid(p_session->peer);
    return 0;
}

int cli_sock_init()
{
    if (g_CliSock >= 0) {
        return -1;
    }

    g_CliSock = ctc_socket(CTC_SOCK_IPC, 0, "cliSocketSrv");
    if (-1 == g_CliSock) {
        return -1;
    }

    ctc_sock_register_read_fn(g_CliSock, cli_sock_read);
    ctc_sock_register_sess_del_fn(g_CliSock, cli_session_done);
    ctc_sock_bind(g_CliSock, GLB_CLI_SOCK_PATH);
    return 0;
}

int32
user_set_privilege(tbl_user_t *p_user)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_user_set_user_field(p_user, TBL_USER_FLD_PRIVILEGE));
    return rc;
}

int32
user_set_service_type(tbl_user_t *p_user, tbl_user_field_id_t field_id)
{
    int32 rc = PM_E_NONE;
    //tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    //char szCmd[512];
    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_user_set_user_field(p_user, field_id));

    /*modifiedby yangl for bug 52333*/
    /*if (!p_db_glb->rpcapi_disable)
    {
#ifdef HAVE_DEBIAN
        snprintf(szCmd, 512, 
            "python /var/rpcapi/run_server.py restart");
#else
        snprintf(szCmd, 512, 
            "export PYTHONHOME=/usr; python /var/rpcapi/run_server.py restart");
#endif
        system(szCmd);
        sleep(1);
    }
    */
    /*yangl end*/
    return rc;
}


int32
user_set_passwd(tbl_user_t *p_user, tbl_user_field_id_t field_id)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_user_set_user_field(p_user, field_id));
    return rc;
}

int32
authen_set_methods(tbl_authen_t *p_item)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_authen_set_authen_field(p_item, TBL_AUTHEN_FLD_METHODS));
    return rc;
}

int32
vty_set(tbl_vty_field_id_t field_id, tbl_vty_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_VTY_FLD_TIMEOUT_MIN:
    case TBL_VTY_FLD_TIMEOUT_SEC:
    case TBL_VTY_FLD_LOGIN:
    case TBL_VTY_FLD_PRIVILEGE:
    case TBL_VTY_FLD_ENC_PASSWD:
    case TBL_VTY_FLD_PASSWD:
    case TBL_VTY_FLD_AUTH_METHOD:
    case TBL_VTY_FLD_INUSE:
    case TBL_VTY_FLD_PROTOCOL:
    case TBL_VTY_FLD_AUTHOR_METHOD:
    case TBL_VTY_FLD_ACCOUNT_METHOD:
    case TBL_VTY_FLD_ACCOUNTCMD_METHOD:
    case TBL_VTY_FLD_AAA_PRIVILEGE:
    case TBL_VTY_FLD_CONFIG_PRIVILEGE:  
    case TBL_VTY_FLD_AAA_START_TIME:
    case TBL_VTY_FLD_CMD_BUF:
    case TBL_VTY_FLD_OLD_CMD_BUF:
    /* modified by liwh for bug 46991 to support ssh aaa, 2018-03-30 */
    case TBL_VTY_FLD_IS_NOT_SSH_KEY:
    /* liwh end */
    
    /* modified by yejl for bug 46694 to support ssh aaa, 2018-04-03 */
    case TBL_VTY_FLD_IS_SSH_LOGIN:
        break;
    /* yejl end */
    case TBL_VTY_FLD_IPV4_ACL:
    case TBL_VTY_FLD_IPV6_ACL:
        break;
    default:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_vty_set_vty_field(pitem, field_id));
    return rc;
}

int32
console_set(tbl_console_field_id_t field_id, tbl_console_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_CONSOLE_FLD_TIMEOUT_MIN:
    case TBL_CONSOLE_FLD_TIMEOUT_SEC:
    case TBL_CONSOLE_FLD_LOGIN:
    case TBL_CONSOLE_FLD_PRIVILEGE:
    case TBL_CONSOLE_FLD_ENC_PASSWD:
    case TBL_CONSOLE_FLD_PASSWD:
    case TBL_CONSOLE_FLD_AUTH_METHOD:
    case TBL_CONSOLE_FLD_INUSE:
        break;
    case TBL_CONSOLE_FLD_STOPBITS:
        set_serial_stopbits(pitem->stopbits);
        break;
    case TBL_CONSOLE_FLD_PARITY:
        set_serial_parity(pitem->parity);
        break;
    case TBL_CONSOLE_FLD_DATABITS:
        set_serial_databits(pitem->databits);
        break;
    case TBL_CONSOLE_FLD_BAUDRATE:
        set_serial_baudrate(pitem->baudrate);
        break;
    case TBL_CONSOLE_FLD_MAX:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_console_set_console_field(pitem, field_id));
    return rc;
}

static int32
tbl_user_password_encryption(void *data, tbl_iter_args_t *pargs)
{
    tbl_user_t *pUser = (tbl_user_t *)data;
    tbl_user_t user;

    if (NULL != pUser && '\0' != pUser->passwd[0]) {
        char szTmp[PASSWD_SIZE];
        gen_passwd_crypt(pUser->passwd, szTmp, PASSWD_SIZE);
        memset(&user, 0x00, sizeof(tbl_user_t));
        snprintf(user.key.name, USERNAME_SIZE, "%s", pUser->key.name);
        snprintf(user.enc_passwd, PASSWD_SIZE, "%s", szTmp);
        tbl_user_set_user_field(&user, TBL_USER_FLD_ENC_PASSWD);
        tbl_user_set_user_field(&user, TBL_USER_FLD_PASSWD);
    }

    return PM_E_NONE;
}

static int32
tbl_snmp_usm_user_password_encryption(void *data, tbl_iter_args_t *pargs)
{
    tbl_snmp_usm_user_t *p_user = (tbl_snmp_usm_user_t *)data;
    tbl_snmp_usm_user_t user;
    char szTmp[PASSWD_SIZE] = {0};

    if (NULL != p_user && '\0' != p_user->auth_pwd[0]) 
    {
        gen_passwd_crypt(p_user->auth_pwd, szTmp, PASSWD_SIZE);
        memset(&user, 0x00, sizeof(tbl_snmp_usm_user_t));
        snprintf(user.key.usm_user_name, SNMP_NAME_SIZE, "%s", p_user->key.usm_user_name);
        snprintf(user.enc_auth_pwd, PASSWD_SIZE, "%s", szTmp);
        tbl_snmp_usm_user_set_snmp_usm_user_field(&user, TBL_SNMP_USM_USER_FLD_AUTH_PWD);
        tbl_snmp_usm_user_set_snmp_usm_user_field(&user, TBL_SNMP_USM_USER_FLD_ENC_AUTH_PWD);

        if ('\0' != p_user->priv_pwd[0])
        {
            gen_passwd_crypt(p_user->priv_pwd, szTmp, PASSWD_SIZE);
            memset(&user, 0x00, sizeof(tbl_snmp_usm_user_t));
            snprintf(user.key.usm_user_name, SNMP_NAME_SIZE, "%s", p_user->key.usm_user_name);
            snprintf(user.enc_priv_pwd, PASSWD_SIZE, "%s", szTmp);
            tbl_snmp_usm_user_set_snmp_usm_user_field(&user, TBL_SNMP_USM_USER_FLD_PRIV_PWD);
            tbl_snmp_usm_user_set_snmp_usm_user_field(&user, TBL_SNMP_USM_USER_FLD_ENC_PRIV_PWD);
        }
    }

    return PM_E_NONE;
}


static int32
tbl_vty_password_encryption(void *data, tbl_iter_args_t *pargs)
{
    tbl_vty_t *pVty = (tbl_vty_t *)data;
    tbl_vty_t vty;

    if (NULL != pVty && '\0' != pVty->passwd[0]) {
        char szTmp[PASSWD_SIZE];
        gen_passwd_crypt(pVty->passwd, szTmp, PASSWD_SIZE);
        memset(&vty, 0x00, sizeof(tbl_vty_t));
        vty.key.id = pVty->key.id;
        snprintf(vty.enc_passwd, PASSWD_SIZE, "%s", szTmp);
        tbl_vty_set_vty_field(&vty, TBL_VTY_FLD_ENC_PASSWD);
        tbl_vty_set_vty_field(&vty, TBL_VTY_FLD_PASSWD);
    }

    return PM_E_NONE;
}

static int32
tbl_enable_password_encryption(void *data, tbl_iter_args_t *pargs)
{
    tbl_enable_t *pEnable = (tbl_enable_t *)data;
    tbl_enable_t enable;

    if (NULL != pEnable && '\0' != pEnable->passwd[0]) {
        char szTmp[PASSWD_SIZE];
        gen_passwd_crypt(pEnable->passwd, szTmp, PASSWD_SIZE);
        memset(&enable, 0x00, sizeof(tbl_enable_t));
        enable.key.id = pEnable->key.id;
        snprintf(enable.enc_passwd, PASSWD_SIZE, "%s", szTmp);
        tbl_enable_set_enable_field(&enable, TBL_ENABLE_FLD_ENC_PASSWD);
        tbl_enable_set_enable_field(&enable, TBL_ENABLE_FLD_PASSWD);
    }

    return PM_E_NONE;
}


int32
app_auth_password_encryption(int onoff)
{
    char szTmp[PASSWD_SIZE];
    
    tbl_console_t *pConsole;
    tbl_console_t console;
    
    if (!onoff) {
        return 0;
    }

    /* user */
    tbl_user_iterate((TBL_ITER_CB_FUNC)tbl_user_password_encryption, NULL);

    /* console */
    pConsole = tbl_console_get_console();
    if ('\0' != pConsole->passwd[0]) {
        gen_passwd_crypt(pConsole->passwd, szTmp, PASSWD_SIZE);
        memset(&console, 0x00, sizeof(tbl_console_t));
        snprintf(console.enc_passwd, PASSWD_SIZE, "%s", szTmp);
        tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_ENC_PASSWD);
        tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_PASSWD);
    }

    /* vty */
    tbl_vty_iterate((TBL_ITER_CB_FUNC)tbl_vty_password_encryption, NULL);

    /* enable */
    tbl_enable_iterate((TBL_ITER_CB_FUNC)tbl_enable_password_encryption, NULL);

    /* snmp usm user */
    tbl_snmp_usm_user_iterate((TBL_ITER_CB_FUNC)tbl_snmp_usm_user_password_encryption, NULL);
    
    return 0;
}

void _app_login_security_alarm()
{
    tbl_login_security_glb_t *p_db_login_security = NULL;
    tbl_login_record_t *p_db_login_record = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_record_master_t *p_login_record_master = NULL;
    uint32 current_time_sec = 0;
    uint32 current_invalid_user = 0;

    p_login_record_master = tbl_login_record_get_master();
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();

    CTC_TASK_STOP_TIMER(t_login_security_timer); 
    
    current_time_sec = (uint32)sal_time_sys_current(NULL);
    CTCLIB_SLIST_LOOP_DEL(p_login_record_master->user_list, p_db_login_record, listnode, next)
    {
        if ((!p_db_login_record->locked)&&(current_time_sec - p_db_login_record->first_fail_ts)>p_db_login_security->fail_period * 60)
        { 
            tbl_login_record_del_login_record(&p_db_login_record->key);
        }
    }
    
    p_login_record_master = tbl_login_record_get_master();
    current_time_sec = (uint32)sal_time_sys_current(NULL);
    CTCLIB_SLIST_LOOP_DEL(p_login_record_master->user_list, p_db_login_record, listnode, next)
    {
        if ((p_db_login_record->locked)&&(current_time_sec - p_db_login_record->lock_ts)>p_db_login_security->lock_duration * 60)
        {
            if (p_db_login_record->invalid_user)
            {
                p_db_login_security ->curr_invalid_user--;
                tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER);
            }
            
            tbl_login_record_del_login_record(&p_db_login_record->key);
        }
    }
  
    p_login_record_master = tbl_login_record_get_master();
    CTCLIB_SLIST_LOOP(p_login_record_master->user_list, p_db_login_record, listnode)
    {
        if (p_db_login_record->invalid_user)
        {
           current_invalid_user++;
        }
    }
    p_db_login_security ->curr_invalid_user = current_invalid_user;
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER);
    t_login_security_timer = CTC_TASK_ADD_TIME(_app_login_security_alarm, NULL, APP_LOGIN_SECURITY_ALARM_INTERVAL); 

}

void web_session_expire_timeout(void *pArg)
{
    if (NULL != g_p_web_session_timeout) {
        web_session_expire_cb(NULL);
    }

    g_p_web_session_timeout = ctc_task_add_timer(TASK_PRI_NORMAL,
            web_session_expire_timeout, NULL, 1);
    return;
}

int32
app_auth_init()
{
    /* modified by liwh for bug 51875, 2018-06-10 */
    //tbl_user_t user;
    /* liwh end */
    tbl_console_t console;
    tbl_vty_t vty;
    tbl_authen_t authen;
    tbl_enable_t enable;
    tbl_author_t author;
    tbl_account_t account;
    tbl_accountcmd_t accountcmd;
    int i;
    tbl_login_security_glb_t *p_db_login_security = NULL;
    tbl_sys_global_t *p_db_glb = NULL;
    
    cli_sock_init();
    t_login_security_timer = CTC_TASK_ADD_TIME(_app_login_security_alarm, NULL, APP_LOGIN_SECURITY_ALARM_INTERVAL); 

/* modified by liwh for bug 51875, 2018-06-10 */
#if 0
    /* -- user -- */
    memset(&user, 0x00, sizeof(tbl_user_t));    
    snprintf(user.key.name, USERNAME_SIZE, "%s", "admin");
    snprintf(user.passwd, PASSWD_SIZE, "%s", "admin");
    user.privilege = 4;
    user.service_type = HOST_USER_SERVICE_TYPE_ALL;
    tbl_user_add_user(&user);
#endif
/* liwh end */

    /* -- console -- */
    memset(&console, 0x00, sizeof(tbl_console_t));
    console.timeout_min = 10; /* 10 minutes */
    console.timeout_sec = 0;
    console.login = LOGIN_METHOD_NOLOGIN;
    console.privilege = 1;
    console.stopbits = 1;
    console.parity = PARITY_NONE;
    console.databits = 8;
    console.baudrate = GLB_CONSOLE_RATE_DEFAULT;
    
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_TIMEOUT_MIN);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_TIMEOUT_SEC);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_LOGIN);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_PRIVILEGE);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_STOPBITS);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_PARITY);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_DATABITS);
    tbl_console_set_console_field(&console, TBL_CONSOLE_FLD_BAUDRATE);

    /* -- vty -- */
    memset(&vty, 0x00, sizeof(tbl_vty_t));
    vty.timeout_min = 10;
    vty.timeout_sec = 0;
#ifdef _GLB_UML_SYSTEM_
    vty.login = LOGIN_METHOD_NOLOGIN;
#else
    vty.login = LOGIN_METHOD_LOCAL;
#endif
    vty.privilege = 1;
    for (i = 0; i < GLB_MAX_VTY_NUM; i++) {
        vty.key.id = i;
        tbl_vty_add_vty(&vty);
    }

    /* -- authen -- */
    memset(&authen, 0x00, sizeof(tbl_authen_t));
    snprintf(authen.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
    snprintf(authen.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "local");
    tbl_authen_add_authen(&authen);

    /* -- author -- */
    memset(&author, 0x00, sizeof(tbl_author_t));
    snprintf(author.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
#ifdef TAPPRODUCT    
    snprintf(author.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "none");
#else
    snprintf(author.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "local");
#endif    
    tbl_author_add_author(&author);

    /* -- account -- */
    memset(&account, 0x00, sizeof(tbl_account_t));
    snprintf(account.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
    snprintf(account.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "none");
    tbl_account_add_account(&account);

    /* -- accountcmd -- */
    /*added by yejl to fix bug41145, 2016-12-13*/
    memset(&accountcmd, 0x00, sizeof(tbl_accountcmd_t));
    snprintf(accountcmd.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
    snprintf(accountcmd.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "none");
    tbl_accountcmd_add_accountcmd(&accountcmd);
    
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    p_db_login_security ->enable = APP_LOGIN_DEFAULT_LOGIN_SECURITY_ENABLE;
    p_db_login_security->max_invalid_user = APP_LOGIN_DEFAULT_MAX_INVALID_USER;
    p_db_login_security->max_fail_num = APP_LOGIN_DEFAULT_MAX_FAIL_NUM;
    p_db_login_security->lock_duration = APP_LOGIN_DEFAULT_LOCKED_DURATION;
    p_db_login_security->fail_period = APP_LOGIN_DEFAULT_FAIL_PERIOD;
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_ENABLE);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION);
    tbl_login_security_glb_set_login_security_glb_field(p_db_login_security, TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD);
    /* -- enable -- */
    memset(&enable, 0x00, sizeof(tbl_enable_t));
    for (i = 0; i < GLB_MAX_PRIVILEAGE_NUM; i++) {
        enable.key.id = i;
        tbl_enable_add_enable(&enable);
    }
    /* -- web session */
    p_db_glb = tbl_sys_global_get_sys_global();
    p_db_glb->http_timeout = 60*(APP_LOGIN_HTTP_DEFAULT_TIME_OUT);
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT);
    web_session_expire_timeout(NULL);
    return PM_E_NONE;
}

/* Added by kcao for bug 48333 2018-08-27, monitor login session timeout */
void
app_login_polling_timer_fn(void *p_data)
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_t *p_db_login = NULL;
    tbl_vty_t *p_db_vty = NULL;
    tbl_console_t *p_db_con = tbl_console_get_console();
    char *p = NULL;
    int32 vty_id = 0;
    int32 timeout = 0;
    int32 idle_time = 0;
    tbl_vty_key_t vty_key;
    struct stat vty_stat;
    char vty_path[CMD_BUF_64];
    char cmd[256];

    CTCLIB_SLIST_LOOP(p_master->login_list, p_db_login, listnode)
    {
        if (!sal_strcmp(p_db_login->protocol, "rpc"))
        {
            continue;
        }

        if (sal_strstr(p_db_login->vty, "vty"))
        {
            vty_id = 0;
            sal_sscanf(p_db_login->vty, "vty%d", &vty_id);
            vty_key.id = vty_id;
            p_db_vty = tbl_vty_get_vty(&vty_key);
            if (NULL == p_db_vty)
            {
                continue;
            }
            timeout = p_db_vty->timeout_min * 60 + p_db_vty->timeout_sec;
        }
        else
        {
            timeout = p_db_con->timeout_min * 60 + p_db_con->timeout_sec;
        }
        
        if (0 == timeout)
        {
            /* 0 means not timeout */
            continue;
        }
        /* idle time */
        snprintf(vty_path, 128, "/dev/%s", p_db_login->key.name);
        p = vty_path;
        while ('\0' != *p)
        {
            if ('-' == *p)
            {
                *p = '/';
            }
            p++;
        }
        memset(&vty_stat, 0, sizeof(vty_stat));
        if (!sal_stat(vty_path, &vty_stat))
        {
            idle_time = sal_time(NULL) - vty_stat.st_atime;
        }
        else
        {
            idle_time = 0;
        }
        if (idle_time > timeout + 30)
        {
            log_sys(M_MOD_AUTHD, E_WARNING, "Detect user from %s is timed out and kick away", p_db_login->ipaddr);
#ifndef HAVE_DEBIAN
            sal_snprintf(cmd, 256, "kill -9 %d crond 1>/dev/null 2>&1", p_db_login->pid);
#else
//            sal_snprintf(cmd, 256, "pstree -p %d | awk -F \"\\(|\\)\" '{for (i=1; i<=NF; i++) print $i; }' "
//                                   "| grep -v -e [-a-z] | xargs kill 1>/dev/null 2>&1", p_db_login->pid);
            sal_snprintf(cmd, 256,
                         "pstree -p %d | sed -e 's/---/\\n/g' |"
                         " sed 's/(/ /g' | sed -e 's/)//g' |"
                         " awk '{print \"kill -9 \"$2\" 2>/dev/null\"}' | sh",
                         p_db_login->pid);
#endif
            sal_system(cmd);
        }
    }

    ctc_task_add_timer(TASK_PRI_NORMAL, app_login_polling_timer_fn, NULL, CPU_LOGIN_POLLING_INTERVAL);
    
    return;
}


#ifndef __APP_AUTH_H__
#define __APP_AUTH_H__

#define CPU_LOGIN_POLLING_INTERVAL    5
#define HOST_USER_SERVICE_TYPE_TELNET	 (1 << 0)
#define HOST_USER_SERVICE_TYPE_SSH	     (1 << 1)
#define HOST_USER_SERVICE_TYPE_WEB  	 (1 << 2)
#define HOST_USER_SERVICE_TYPE_RPC_API	 (1 << 3)
#define HOST_USER_SERVICE_TYPE_ALL	     (0x0f)
#define HOST_USER_SERVICE_TYPE_NONE	     0
#define APP_USR_PSSWD_CHECK_LOGIN_SUCESS 0 
#define APP_USR_PSSWD_CHECK_USER_NONE_PASSWORD_GET  (-7)
#define APP_USR_PSSWD_CHECK_USER_LOCKED  (-6)
#define APP_USR_PSSWD_CHECK_OVER_INVALID_LIMIT    (-5)
#define APP_USR_PSSWD_CHECK_USER_NOT_FOUND    (-4)
#define APP_USR_PSSWD_CHECK_SERVICE_TYPE_MISMATCH    (-3)
#define APP_USR_PSSWD_CHECK_INVALID_USER_OR_PASSWORD  (-2)
#define APP_LOGIN_DEFAULT_LOGIN_SECURITY_DISABLE  0
#define APP_LOGIN_DEFAULT_LOGIN_SECURITY_ENABLE  1
#define APP_LOGIN_DEFAULT_MAX_INVALID_USER  5
#define APP_LOGIN_DEFAULT_MAX_FAIL_NUM  5
#define APP_LOGIN_DEFAULT_LOCKED_DURATION 5
#define APP_LOGIN_DEFAULT_FAIL_PERIOD 5
#define APP_LOGIN_SECURITY_ALARM_INTERVAL  2
#define APP_LOGIN_HTTP_DEFAULT_TIME_OUT 10
#define sal_time_sys_current(z) time(z)

enum acl_result {
 LINEACL_PERMIT, 
 LINEACL_DENY,
 LINE_ACL_NOT_MATCH,
 LINE_ACL_POINTER_ERR,
 LINEACL_MAX
};

#define GET_INTEGER_RANGE(NAME,V,STR,MIN,MAX)                             \
    do {                                                                      \
      (V) = (STR);                                       \
      if ((V) < (MIN) || (V) > (MAX))                             \
        {                                                                     \
          log_sys(M_MOD_APP, E_ERROR, "Invlid %s when building connection.\n", NAME);              \
          return LINEACL_DENY;                                                   \
        }                                                                     \
    } while (0)

int32
app_auth_init();

int32
app_auth_password_encryption(int onoff);

int32
app_login_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_act_show_login_security(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_rsa_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_authen_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_author_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_account_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_accountcmd_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_user_cmd_get_auth_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_act_line_acl_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_user_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_login_record_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_login_security_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_console_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_vty_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_enable_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);


int32
user_set_privilege(tbl_user_t *p_user);

int32
user_set_encrypted(tbl_user_t *p_user);

int32
user_set_passwd(tbl_user_t *p_user, tbl_user_field_id_t field_id);

int32
authen_set_methods(tbl_authen_t *p_item);

int32
console_set(tbl_console_field_id_t field_id, tbl_console_t *pitem);

int32
app_cmd_process_cpu_utilization(char **argv, int32 argc, cfg_cmd_para_t *para);

int app_get_pid_by_name(char* name, pid_t* ppid);

int32
app_cmd_process_cpu_limit(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
tbl_cpu_utilization_iterate_update(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
user_set_service_type(tbl_user_t *p_user, tbl_user_field_id_t field_id);

int32
vty_set(tbl_vty_field_id_t field_id, tbl_vty_t *pitem);

int32
app_web_login_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_change_username_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_send_trap_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);

void
app_login_polling_timer_fn(void *p_data);

#endif /* !__APP_AUTH_H__ */


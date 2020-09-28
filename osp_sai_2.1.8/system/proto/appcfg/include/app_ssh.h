#ifndef __APP_SSH_H__
#define __APP_SSH_H__

#ifdef _GLB_UML_SYSTEM_
#define M_SSHD_PROG             "/centec_switch/sbin/sshd"
#else
#define M_SSHD_PROG             SWITCH_PREFIX"/usr/sbin/sshd"
#endif
#define M_SSHD_CFGFILE          "/etc/ssh/sshd_config"
//#define M_SSHD_HOST_KEY_FILE    "/etc/ssh/keys/sshd_host_key"
#define M_SSHD_AUTH_KEY_PATH    "/etc/ssh/keys"
#define M_SSHD_HOST_RSA_KEY_FILE    "/etc/ssh/ssh_host_rsa_key"
#define M_SSHD_HOST_RSA_DEFULT_KEY_FILE    "/etc/ssh/ssh_host_rsa_defult_key"
#define M_SSHD_HOST_ECDSA_KEY_FILE    "/etc/ssh/ssh_host_ecdsa_key"
#define M_SSHD_HOST_ED25519_KEY_FILE    "/etc/ssh/ssh_host_ed25519_key"
#define M_NETCONF_SSH_PORT      830

int32
app_ssh_init();

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:20 */
int32
app_ssh_set_netconf_service(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg);
int app_netconf_generate_cfg(FILE *fp, tbl_ssh_cfg_t *p_db_cfg);
#endif /* !HAVE_NETCONF */

int32
app_ssh_proc_startup_done();

int32
app_ssh_set_version(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_enable(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_auth_retry(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_auth_timeout(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_rekey_interval(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_auth_type(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_hostkey(tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_set_listen_port(tbl_ssh_cfg_t *p_db_cfg, tbl_ssh_cfg_t *p_cfg);

int32
app_ssh_cmd_process_ssh(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__APP_SSH_H__ */


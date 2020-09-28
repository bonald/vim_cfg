
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "proto.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "appcfg.h"
#include "app_ssh.h"

extern int app_is_startup_done();

static int
_app_sshd_stop(void)
{
    char szCmd[512];

    if (!app_is_startup_done()) {
        return 0;
    }

    snprintf(szCmd, 512, "ps -ax 2>/dev/null | grep sshd | grep -v grep | awk '{ print $1}' | xargs kill");
    system(szCmd);
    snprintf(szCmd, 512, "cdbctl delete/cdb/sys/ns_port_forwarding/0#0#22#0.0.0.0/ip/0.0.0.0");
    return system(szCmd);
}

static int
_app_sshd_start(void)
{
    char szCmd[512];

    if (!app_is_startup_done()) {
        return 0;
    }

    snprintf(szCmd, 512, "%s && "
        "cdbctl create/cdb/sys/ns_port_forwarding/0#0#22#0.0.0.0/ip/0.0.0.0",
        M_SSHD_PROG);
    return system(szCmd);
}

static int
_app_sshd_restart(void)
{
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();

    if (!app_is_startup_done() || !p_db_cfg->enable) {
        return 0;
    }
    
    /* send SIGHUP to sshd */
    return system("ps -ax 2>/dev/null | grep sshd | grep -v grep | awk '{ print $1}' | xargs kill -1");
}

static int
_app_sshd_generate_cfg(char *pszErrMsg, size_t nBufLen)
{
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();
    FILE *fp = NULL;
    int ret = -1;

    if (!app_is_startup_done()) {
        return 0;
    }

    assert(NULL != p_db_cfg);
    if ((fp = fopen(M_SSHD_CFGFILE, "w")) == NULL) {
        if (pszErrMsg) {
            snprintf(pszErrMsg, nBufLen, 
                "Can't open sshd configuration file: %s",
                strerror(errno));
        }
        goto err_out;
    }

    /* output default configuration */
    fprintf(fp, "# Automatically generated file, please don't edit\r\n\r\n");
    fprintf(fp, "PermitRootLogin yes\r\n");

#ifdef _GLB_UML_SYSTEM_
    fprintf(fp, "PermitUserEnvironment yes\r\n");
#endif
    fprintf(fp, "AuthorizedKeysFile %s/authkeys.%%u\r\n", M_SSHD_AUTH_KEY_PATH);

    /* output version */
        //fprintf(fp, "Protocol %d\r\n", 2);
        //fprintf(fp, "HostKey for protocol version 2\r\n");
        fprintf(fp, "HostKey %s\r\n", M_SSHD_HOST_RSA_KEY_FILE);
        //fprintf(fp, "HostKey %s\r\n", M_SSHD_HOST_ECDSA_KEY_FILE);
        //fprintf(fp, "HostKey %s\r\n", M_SSHD_HOST_ED25519_KEY_FILE);
    /*Add for update ssh to 7.1 ==> del for update ssh to 7.9 */
    /*
    fprintf(fp, "Ciphers chacha20-poly1305@openssh.com,aes256-ctr,aes192-ctr,aes128-ctr,aes128-cbc,aes192-cbc,aes256-cbc,3des-cbc,"
        "arcfour256,arcfour128,arcfour,blowfish-cbc,cast128-cbc  \r\n");
    fprintf(fp, "MACs umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha1-96-etm@openssh.com,hmac-sha2-256-etm@openssh.com,"
        "hmac-sha2-512-etm@openssh.com,umac-128@openssh.com,hmac-md5-etm@openssh.com,hmac-md5-96-etm@openssh.com,"
        "hmac-ripemd160-etm@openssh.com,hmac-sha1-etm@openssh.com,hmac-md5,hmac-md5-96,hmac-ripemd160,hmac-sha1,"
        "hmac-sha1-96,hmac-sha2-256,hmac-sha2-512,umac-64@openssh.com \r\n");
    fprintf(fp, "KexAlgorithms curve25519-sha256@libssh.org,diffie-hellman-group1-sha1,diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha1,"
        "diffie-hellman-group-exchange-sha256,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521 \r\n"); */
    fprintf(fp, "Ciphers chacha20-poly1305@openssh.com,aes256-ctr,aes192-ctr,aes128-ctr,aes128-cbc,aes192-cbc,aes256-cbc,3des-cbc,aes128-gcm@openssh.com,aes256-gcm@openssh.com \r\n");
    fprintf(fp,"\r\n");
    fprintf(fp, "MACs umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,"
        "hmac-sha2-512-etm@openssh.com,umac-128@openssh.com,"
        "hmac-sha1-etm@openssh.com,hmac-sha1,"
        "hmac-sha2-256,hmac-sha2-512 \r\n");
    fprintf(fp,"\r\n");
    fprintf(fp, "KexAlgorithms curve25519-sha256@libssh.org,diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha1,"
        "ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521 \r\n");
    /* output auth retries */
    fprintf(fp, "MaxAuthTries %d\r\n", p_db_cfg->auth_retry);

    /* output auth timeouts */
    fprintf(fp, "LoginGraceTime %d\r\n", p_db_cfg->auth_timeout);

    /* output auth type */
    if (p_db_cfg->auth_type & SSH_AUTH_TYPE_PASSWORD) {
        fprintf(fp, "PasswordAuthentication no\r\n");
        fprintf(fp, "UsePAM yes\r\n"); /*Modified by yejl for bug 44994, 2018-02-06*/
    } else {
        fprintf(fp, "PasswordAuthentication no\r\n");
        fprintf(fp, "UsePAM no\r\n");
    }
    
    /*if (p_db_cfg->auth_type & SSH_AUTH_TYPE_RSA) {
        //fprintf(fp, "RSAAuthentication yes\r\n");
    } else {
        fprintf(fp, "RSAAuthentication no\r\n");
    }*/
    
    if (p_db_cfg->auth_type & SSH_AUTH_TYPE_PUBKEY) {
        fprintf(fp, "PubkeyAuthentication yes\r\n");
    } else {
        fprintf(fp, "PubkeyAuthentication no\r\n");
    }   

    /* output rekey interval */
    /* del for update ssh to 7.9 */
    //fprintf(fp, "KeyRegenerationInterval %d\r\n", p_db_cfg->rekey_interval * 60);

    /* listen port */
    fprintf(fp, "Port %d\r\n", p_db_cfg->listen_port);

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:20 */
    app_netconf_generate_cfg(fp, p_db_cfg);
#endif

    fprintf(fp, "LogLevel ERROR\r\n");

    fprintf(fp, "MaxStartups 3\r\n");

    ret = 0;
    
err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    return ret;
}

static int
_app_ssh_set(tbl_ssh_cfg_field_id_t cfgid, tbl_ssh_cfg_t *p_cfg)
{
    int32 rc = PM_E_NONE;
        
    PM_E_RETURN(tbl_ssh_cfg_set_ssh_cfg_field(p_cfg, cfgid));

    rc = _app_sshd_generate_cfg(NULL, 0);
    rc = _app_sshd_restart();
    return rc;
}

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:20 */
int32
app_netconf_service_set(int32 enable, uint16 netconf_port);
int32
app_netconf_generate_cfg(FILE *fp, tbl_ssh_cfg_t *p_db_cfg);

int32
app_ssh_set_netconf_service(tbl_ssh_cfg_t *p_cfg, tbl_ssh_cfg_t *p_db_cfg)
{
    uint16 netconf_port = 0;
    if (p_db_cfg->netconf_service == p_cfg->netconf_service) {
        return PM_E_NONE;
    }

    netconf_port = (p_cfg->netconf_service) ? p_cfg->netconf_port : p_db_cfg->netconf_port;
    if (app_netconf_service_set(p_cfg->netconf_service, netconf_port) != 0) {
        return PM_E_FAIL;
    }
    
    tbl_ssh_cfg_set_ssh_cfg_field(p_cfg, TBL_SSH_CFG_FLD_NETCONF_PORT);
    if (_app_ssh_set(TBL_SSH_CFG_FLD_NETCONF_SERVICE, p_cfg) != 0) {
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

#endif /* !HAVE_NETCONF */

int32
app_ssh_set_version(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_VERSION, p_cfg);
}

int32
app_ssh_set_enable(tbl_ssh_cfg_t *p_cfg)
{
    int32 rc = PM_E_NONE;

    tbl_ssh_cfg_set_ssh_cfg_field(p_cfg, TBL_SSH_CFG_FLD_ENABLE);
    if (p_cfg->enable) {
        rc = _app_sshd_generate_cfg(NULL, 0);
        rc = _app_sshd_start();
    } else {
        _app_sshd_stop();
    }
    
    return rc;
}

int32
app_ssh_set_auth_retry(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_AUTH_RETRY, p_cfg);
}

int32
app_ssh_set_auth_timeout(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_AUTH_TIMEOUT, p_cfg);
}

int32
app_ssh_set_rekey_interval(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_REKEY_INTERVAL, p_cfg);
}

int32
app_ssh_set_auth_type(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_AUTH_TYPE, p_cfg);
}

int32
app_ssh_set_hostkey(tbl_ssh_cfg_t *p_cfg)
{
    return _app_ssh_set(TBL_SSH_CFG_FLD_HOSTKEY, p_cfg);
}

int32
_swth_sys_set_ssh_ns_port(uint32 old_port, uint32 new_port)
{
    char cmd[CMD_BUF_128];

    sal_memset(cmd, 0, sizeof(cmd));
    
    /* 1. delete old NS port forwarding */
    if (GLB_SSH_PORT_DEF != old_port)
    {
        sal_snprintf(cmd, CMD_BUF_128, "cdbctl delete/cdb/sys/ns_port_forwarding/0#0#%u#0.0.0.0/ip/0.0.0.0", old_port);
        sal_system(cmd);
    }
    
    /* 2. add new NS port forwarding */
    if (GLB_SSH_PORT_DEF != new_port)
    {
        sal_snprintf(cmd, CMD_BUF_128, "cdbctl create/cdb/sys/ns_port_forwarding/0#0#%u#0.0.0.0/ip/0.0.0.0", new_port);
        sal_system(cmd);
    }

    return PM_E_NONE;
}

int32
app_ssh_set_listen_port(tbl_ssh_cfg_t *p_db_cfg, tbl_ssh_cfg_t *p_cfg)
{
    _swth_sys_set_ssh_ns_port(p_db_cfg->listen_port, p_cfg->listen_port);
    return _app_ssh_set(TBL_SSH_CFG_FLD_LISTEN_PORT, p_cfg);
}

int32
app_netconf_init(tbl_ssh_cfg_t *pCfg)
{
    pCfg->netconf_port = M_NETCONF_SSH_PORT;
    tbl_ssh_cfg_set_ssh_cfg_field(pCfg, TBL_SSH_CFG_FLD_NETCONF_PORT);
    return PM_E_NONE;
}

int32
app_ssh_init()
{
    tbl_ssh_cfg_t defcfg;

    memset(&defcfg, 0x00, sizeof(tbl_ssh_cfg_t));
    defcfg.auth_retry = 6;
    defcfg.auth_timeout = 120;
    defcfg.auth_type = (SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY);
    defcfg.rekey_interval = 60;
    defcfg.version = SSH_VERSION_V2;
    defcfg.listen_port = 22;
    defcfg.enable = 1;

    tbl_ssh_cfg_set_ssh_cfg_field(&defcfg, TBL_SSH_CFG_FLD_LISTEN_PORT);
    app_ssh_set_version(&defcfg); 
    app_ssh_set_auth_retry(&defcfg);
    app_ssh_set_auth_timeout(&defcfg);
    app_ssh_set_auth_type(&defcfg);
    app_ssh_set_rekey_interval(&defcfg);
    app_ssh_set_enable(&defcfg);
    app_netconf_init(&defcfg);
    
    return PM_E_NONE;
}

int32
app_ssh_proc_startup_done()
{
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();

    _app_sshd_generate_cfg(NULL, 0);
    if (p_db_cfg->enable) {
        _app_sshd_start();
        system("cp "M_SSHD_HOST_RSA_KEY_FILE" "M_SSHD_HOST_RSA_DEFULT_KEY_FILE);
    }
    /* publish port to update service iptables chain */
    tbl_ssh_cfg_set_ssh_cfg_field(p_db_cfg, TBL_SSH_CFG_FLD_LISTEN_PORT);
    return PM_E_NONE;
}

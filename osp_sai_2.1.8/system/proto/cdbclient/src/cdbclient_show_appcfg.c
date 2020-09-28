#include "proto.h"
#include "cdbclient.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_accountcmd_define.h"
#include "param_check.h"
#include "rsa_derhex.h"
#include "lib_fs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#define M_APP_SPLIT_OUT sal_printf("------------------------------------------------------------\n")
#define   NAME_SERVER_TAG         "nameserver"
#define   DOMAIN_TAG             "domain"

extern char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

int32
cdbctl_show_app_ssh_cfg(FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    tbl_ssh_cfg_t sshcfg;
    char *pszValue;
    int i;

    memset(&sshcfg, 0x00, sizeof(tbl_ssh_cfg_t));
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        pszValue = info.value[TBL_SSH_CFG_FLD_VERSION];
        if (!strcasecmp(pszValue, "v2")) {
            sshcfg.version = SSH_VERSION_V2;
        } else if (!strcasecmp(pszValue, "v1")) {
            sshcfg.version = SSH_VERSION_V1;
        } else {
            sshcfg.version = SSH_VERSION_ALL;
        }
            
        sshcfg.enable = atoi(info.value[TBL_SSH_CFG_FLD_ENABLE]);
        sshcfg.auth_retry = atoi(info.value[TBL_SSH_CFG_FLD_AUTH_RETRY]);
        sshcfg.auth_timeout = atoi(info.value[TBL_SSH_CFG_FLD_AUTH_TIMEOUT]);
        sshcfg.rekey_interval = atoi(info.value[TBL_SSH_CFG_FLD_REKEY_INTERVAL]);
        sshcfg.auth_type |= atoi(info.value[TBL_SSH_CFG_FLD_AUTH_TYPE]);
#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-25 13:59 */
        sshcfg.netconf_service = atoi(info.value[TBL_SSH_CFG_FLD_NETCONF_SERVICE]);
        sshcfg.netconf_port = atoi(info.value[TBL_SSH_CFG_FLD_NETCONF_PORT]);
#endif /* !HAVE_NETCONF */

        pszValue = info.value[TBL_SSH_CFG_FLD_HOSTKEY];
        if (NULL != pszValue && strcasecmp(pszValue, "")) {
            snprintf(sshcfg.hostkey, M_RSA_KEY_NAMESZ + 1, 
                "%s", pszValue);
        }
    }

    sal_printf(" SSH server %s\n", sshcfg.enable ? "enabled" : "disabled");
    sal_printf(" Version: %s\n", 
        (SSH_VERSION_V2 == sshcfg.version) ? "v2" : 
            ((SSH_VERSION_V1 == sshcfg.version) ? "v1" : "all"));
    sal_printf(" Authentication timeout: %u second(s)\n", sshcfg.auth_timeout);
    sal_printf(" Authentication retries: %u time(s)\n", sshcfg.auth_retry);
    //sal_printf(" Server key lifetime: %u minute(s)\n", sshcfg.rekey_interval);
    i = 0;
    sal_printf(" Authentication type: ");
    if (sshcfg.auth_type & SSH_AUTH_TYPE_PASSWORD) {
        sal_printf("password");
        i++;
    }
    if (sshcfg.auth_type & SSH_AUTH_TYPE_PUBKEY) {
        if (!i) {
            sal_printf("public-key");
        } else {
            sal_printf(", public-key");
        }
        i++;
    }
    if (sshcfg.auth_type & SSH_AUTH_TYPE_RSA) {
        if (!i) {
            sal_printf("rsa");
        } else {
            sal_printf(", rsa");
        }
        i++;
    }
    sal_printf("\n");

    if ('\0' != sshcfg.hostkey[0] && strcasecmp(sshcfg.hostkey, "")) {
        sal_printf(" Host key: %s\n", sshcfg.hostkey);
    }

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-25 14:00 */
    sal_printf(" netconf-yang ssh service %s\n", 
            sshcfg.netconf_service ? "enabled" : "disabled");
    sal_printf(" netconf-yang ssh service port %d\n", sshcfg.netconf_port);
#endif /* !HAVE_NETCONF */

    return PM_E_NONE;
}

static int
__print_server_addr_item(void *pArg, void *pArg2, int vrf, const char *pszAddr, int nIdx)
{
    FILE *fp = (FILE *)pArg;
#if 0
    fprintf(fp, "logging server address %s%s\n", 
        (MGMT_VRF == vrf) ? "mgmt-if " : "", pszAddr);
#else
    fprintf(fp, "logging server address %s %s\n", 
        pszAddr,
        (MGMT_VRF == vrf) ? "" : "inband");
#endif
    return 0;
}

int32
cdbctl_syslog_write_cfg(cfg_parse_get_info_t *pGetInfo, int bIgnoreDefault, FILE *fp)
{
    const char *pszValue;
    int nValue;
    int nLines = 0;

    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_LOGGING_LINES]);
    if (!bIgnoreDefault || 500 != nValue) {
        fprintf(fp, "logging buffer %s\n", 
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_LOGGING_LINES]);
        nLines++;
    }

    if (!bIgnoreDefault 
            || strcasecmp(pGetInfo->value[TBL_SYSLOG_CFG_FLD_TIMESTAMP], "bsd")) {
        fprintf(fp, "logging timestamp %s\n", 
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_TIMESTAMP]);
        nLines++;
    }
        
    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE]);
    if (!bIgnoreDefault || !nValue) {
        fprintf(fp, "logging file %s\n", nValue ? "enable" : "disable");
        nLines++;
    }

    if (!bIgnoreDefault 
            || strcasecmp(pGetInfo->value[TBL_SYSLOG_CFG_FLD_FILE_SEVERITY], "warning")) {
        nLines++;
        fprintf(fp, "logging level file %s\n", 
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_FILE_SEVERITY]);
    }

    if (!bIgnoreDefault 
            || strcasecmp(pGetInfo->value[TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY], "debug")) {        
        nLines++;
        fprintf(fp, "logging level module %s\n",
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY]);
    }

    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER]);
    if (!bIgnoreDefault || nValue) {
        nLines++;
        fprintf(fp, "logging server %s\n", nValue ? "enable" : "disable");
    }

    if (!bIgnoreDefault 
            || strcasecmp(pGetInfo->value[TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY], "warning")) {
        nLines++;
        fprintf(fp, "logging server severity %s\n", 
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY]);
    }

    if (!bIgnoreDefault 
            || strcasecmp(pGetInfo->value[TBL_SYSLOG_CFG_FLD_SERVER_FACILITY], "local4")) {
        nLines++;
        fprintf(fp, "logging server facility %s\n", 
            pGetInfo->value[TBL_SYSLOG_CFG_FLD_SERVER_FACILITY]);
    }
    
    pszValue = pGetInfo->value[TBL_SYSLOG_CFG_FLD_SERVER_ADDR];
    if (NULL != pszValue && '\0' != pszValue[0]) {
        nLines++;
        logging_server_addr_iter(pszValue, __print_server_addr_item, fp, NULL);
    }

    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_ENABLE_MERGE]);
    if (!bIgnoreDefault || 0 == nValue) {
        fprintf(fp, "logging merge %s\n", nValue ? "enable" : "disable");
        nLines++;
    }

    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE]);
    if (!bIgnoreDefault || 1024 != nValue) {
        fprintf(fp, "logging merge fifo-size %d\n", nValue);
        nLines++;
    }

    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT]);
    if (!bIgnoreDefault || 10 != nValue) {
        fprintf(fp, "logging merge timeout %d\n", nValue);
        nLines++;
    }
    
#if 0
    nValue = atoi(pGetInfo->value[TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP]);
    sal_printf("logging alarm-trap %s\n", nValue ? "enable" : "disable");
    sal_printf("logging alarm-trap level %s\n", 
        pGetInfo->value[TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY]);
#endif

    if (bIgnoreDefault && nLines) {
        fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32
cdbctl_syslog_show_stats(cfg_parse_get_info_t *pGetInfo)
{
    sal_printf("Total processed %s entries\n", pGetInfo->value[0]);
    sal_printf("Total dropped %s entries\n", pGetInfo->value[1]);
    sal_printf("Current have %s entries\n", pGetInfo->value[2]);
    return PM_E_NONE;
}

int32
cdbctl_show_app_syslog_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char *keyword = (char *)p_data;
    
    if (data_len == 0 || !strcasecmp(keyword, "statistics")) {
        sal_fgets(line, FILE_READ_LINE_LEN, fp);
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        if (0 == data_len) {
            sal_printf("Current logging configuration:\n");
            M_APP_SPLIT_OUT;
            cdbctl_syslog_write_cfg(&info, 0, stdout);
        } else {
            sal_printf("Logging buffer statistics:\n");
            M_APP_SPLIT_OUT;
            cdbctl_syslog_show_stats(&info);
        }
        return PM_E_NONE;
    } 

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        sal_printf(line);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_user(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char service[FILE_READ_LINE_LEN];
    char *keyword = (char *)p_data;
    int idx = 0;
    int service_type = 0;
    char *p;
    char *q;
    if (data_len == 0 || !strcasecmp(keyword, "usernames")) {
        sal_printf("Number   User name                        Privilege   Password   Rsa Key\n");
        sal_printf("--------+--------------------------------+-----------+----------+-------\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
            cfg_get_parse(line, &info, 0);
            idx++;
            p = info.value[TBL_USER_FLD_PASSWD];
            q = info.value[TBL_USER_FLD_RSAKEY];
            sal_printf("%-8d %-32s %-11s %-9s  %s\n",
                idx, info.value[TBL_USER_FLD_KEY], info.value[TBL_USER_FLD_PRIVILEGE],
                (NULL == p || '\0' == *p) ? " " : "*", 
                (NULL == q || '\0' == *q) ? "" : q);
        }
        return PM_E_NONE;
    }
    else{
        sal_printf("Number   User name                        Service-type  \n");
        sal_printf("--------+--------------------------------+--------------------\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
            cfg_get_parse(line, &info, 0);
            idx++;
            service_type = sal_atoi(info.value[TBL_USER_FLD_SERVICE_TYPE]);
            switch(service_type)
            {
                case 0:
                    sal_strcpy(service, "none");
                    break;
                case 1:
                    sal_strcpy(service, "telnet");
                    break;
                case 2:
                    sal_strcpy(service, "ssh");
                    break;
                case 3:
                    sal_strcpy(service, "ssh telnet");
                    break;
                case 4:
                    sal_strcpy(service, "web");
                    break;
                case 5:
                    sal_strcpy(service, "web telnet");
                    break;
                case 6:
                    sal_strcpy(service, "web ssh");
                    break;
                case 7:
                    sal_strcpy(service, "web ssh telnet");
                    break;
                case 8:
                    sal_strcpy(service, "rpc-api");
                    break;
                case 9:
                    sal_strcpy(service, "rpc-api telnet");
                    break;
                case 10:
                    sal_strcpy(service, "rpc-api ssh");
                    break;
                case 11:
                    sal_strcpy(service, "rpc-api ssh telnet");
                    break;
                case 12:
                    sal_strcpy(service, "rpc-api web");
                    break;
                case 13:
                    sal_strcpy(service, "rpc-api web telnet");
                    break;
                case 14:
                    sal_strcpy(service, "rpc-api web ssh");
                    break;
                case 15:
                    sal_strcpy(service, "all");
                    break;
                default:
                    break;   
            }
            sal_printf("%-8d %-32s %-20s\n",
                idx, info.value[TBL_USER_FLD_KEY], service);
        }
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

extern int cdbctl_get_rsa_key(const char *pszEncodedKeyStr, const char *pszPasswd, 
    int bPrivate, RSA **ppRsa, char *pszErr, size_t err_bufsz);

int
cdbctl_show_rsa_key(RSA *pRsa, int bPrivate)
{
    char *pszFmt = getenv("SHOW_RSA_KEY_FMT");
    char *pszEnc = getenv("SHOW_RSA_KEY_ENC");
    char *pszEncPasswd = getenv("SHOW_RSA_KEY_PASSWD");
    const EVP_CIPHER *pEnc = NULL; /* don't free this reference */
    BIO *bio = NULL;
    int ret = -1;
    char *pszBuf;
    int nValue;
    
    if (NULL == pszFmt) {
        pszFmt = "der";
    }

    pszBuf = malloc(M_RSA_KEYSTR_BUFSZ);
    if (NULL == pszBuf) {
        fprintf(stderr, "%% Warning: out of memory\r\n");
        return -1;
    }
    
    if (!strcasecmp(pszFmt, "der")) {
        /* get der string for key */
        if (rsa_convert_key_der2str(pRsa, 0, pszBuf, M_RSA_KEYSTR_BUFSZ, &nValue) != 0)
        {
            /* error */
            fprintf(stderr, "%% Can't get RSA key DER string\r\n");
            goto err_out;
        }
        pszBuf[nValue] = '\0';
        if (!bPrivate) {
            sal_printf("DER code:\n%s\n", pszBuf);
        } else {
            sal_printf("Private key DER code:\n%s\n", pszBuf);

            if (rsa_convert_key_der2str(pRsa, 1, pszBuf, M_RSA_KEYSTR_BUFSZ, &nValue) != 0)
            {
                /* error */
                fprintf(stderr, "%% Can't get RSA key DER string\r\n");
                goto err_out;
            }
            pszBuf[nValue] = '\0';
            sal_printf("Public key DER code:\n%s\n", pszBuf);
        }
    } else {
        if ((bio = BIO_new(BIO_s_mem())) == NULL)
        {
            fprintf(stderr, "%% WARNING: out of memory\n");
            goto err_out;
        }

        if (NULL != pszEnc) {
            if (!strcasecmp(pszEnc, "3des"))
            {
                pEnc = EVP_get_cipherbyname("des3");
            }
            else
            {
                pEnc = EVP_get_cipherbyname(pszEnc);
            }

            if (NULL == pEnc)
            {
                fprintf(stderr, "%% Unsupported encrypt algorithm: %s\n", pszEnc);
                goto err_out;
            }

            if (NULL == pszEncPasswd || 
                    check_password(pszEncPasswd) != 0 
                    || strlen(pszEncPasswd) < M_RSA_MIN_PASSWORD_LEN) {
                fprintf(stderr, "%% Invalid ecnrypt password. Please don't less than %d.\n",
                        M_RSA_MIN_PASSWORD_LEN);
                goto err_out;
            }
        }

        /* show public key code */
        if (!PEM_write_bio_RSA_PUBKEY(bio, pRsa))
        {
            fprintf(stderr, "%% Unable to extract public key\n");
            goto err_out;
        }
        sal_printf("Public key PEM code:\n");
        while (BIO_gets(bio, pszBuf, M_RSA_KEYSTR_BUFSZ) > 0)
        {
            sal_printf("%s", pszBuf);
        }

        /* show private key code */
        if (bPrivate)
        {
            if (!PEM_write_bio_RSAPrivateKey(bio, pRsa,
                        pEnc, NULL, 0, NULL, pszEncPasswd))
            {
                fprintf(stderr, "%% Unable to output private key\n");
                goto err_out;
            }

            sal_printf("Private key PEM code:\n");
            while (BIO_gets(bio, pszBuf, M_RSA_KEYSTR_BUFSZ) > 0)
            {
                sal_printf("%s", pszBuf);
            }
        }
    }

    ret = 0;
err_out:
    if (NULL != pszBuf) {
        free(pszBuf);
    }
    if (bio) {
        BIO_free(bio);
    }
    return ret;
}

int32
cdbctl_show_app_rsa(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char szErr[256];
    char szPasswd[PASSWD_SIZE];
    char *pszPasswd = NULL;
    char *p;
    char *pszKey;
    RSA *pRsa = NULL;

    if (data_len && strchr((char *)p_data, '/')) {
        /* get key field */
        return 0;
    }

    pszKey = malloc(M_RSA_KEYSTR_BUFSZ);
    if (NULL == pszKey) {
        fprintf(stderr, "%% cdbctl_buildcfg_build_app_rsa: Out of memory\r\n");
        return -1;
    }

    if (0 == data_len) {
        sal_printf("Current RSA key configuration:\r\n");
        sal_printf("Name                              Type     Usage   Modulus\r\n");
        sal_printf("---------------------------------+--------+-------+---------\r\n");
    } else {
        sal_printf("RSA key information:\r\n");
        sal_printf("------------------------------------------------------------\r\n");
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (NULL != pRsa) {
            RSA_free(pRsa);
            pRsa = NULL;
        }
        
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        if (0 != data_len && strcasecmp(info.value[TBL_RSA_FLD_KEY], (char *)p_data)) {
            continue;
        }

        p = info.value[TBL_RSA_FLD_PEM];
        if (NULL == p || '\0' == *p) {
            continue;
        }

        p = info.value[TBL_RSA_FLD_ENC_PASSWD];
        if (NULL != p && '\0' != *p) {
            szPasswd[0] = '\0';
            do_decrypt(M_DEFAULT_PW_STR, p, strlen(p), szPasswd, PASSWD_SIZE);
            pszPasswd = szPasswd;
        }
        p = info.value[TBL_RSA_FLD_PASSWD];
        if (NULL != p && '\0' != *p) {
            pszPasswd = p;
        }

        p = info.value[TBL_RSA_FLD_PEM];
        if (NULL != p && '\0' != *p) {
            if (cdbctl_get_rsa_key(p, pszPasswd, 
                    !strcasecmp(info.value[TBL_RSA_FLD_TYPE], "private"),
                    &pRsa, szErr, 256) != 0) {
                /* error */
                fprintf(stderr, "%% Get RSA key %s failed: %s\n",
                        info.value[TBL_RSA_FLD_KEY], szErr);
                continue;
            }
        }

        if (!data_len) {
            sal_printf("%-33s %-8s %-8s%d\n", 
                    info.value[TBL_RSA_FLD_KEY], info.value[TBL_RSA_FLD_TYPE],
                    info.value[TBL_RSA_FLD_REFCNT], BN_num_bits(pRsa->n));
        } else {
            sal_printf("Name: %s\r\n", info.value[TBL_RSA_FLD_KEY]);
            sal_printf("Type: %s\r\n", info.value[TBL_RSA_FLD_TYPE]);
            sal_printf("Modulus: %d bit\r\n", BN_num_bits(pRsa->n));
            sal_printf("Usage count: %s\r\n", info.value[TBL_RSA_FLD_REFCNT]);
            if (NULL != pszPasswd && !strcasecmp(info.value[TBL_RSA_FLD_TYPE], "private"))
            {
                sal_printf("Password: ******\r\n");
            }
            cdbctl_show_rsa_key(pRsa, !strcasecmp(info.value[TBL_RSA_FLD_TYPE], "private"));
        }
    }

    if (NULL != pszKey) {
        free(pszKey);
    }

    if (NULL != pRsa) {
        RSA_free(pRsa);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_authen(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char tmp[FILE_READ_LINE_LEN];
    char *p;
    
    sal_printf("Authen queue = AAA_ML_AUTHEN_LOGIN\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        snprintf(tmp, FILE_READ_LINE_LEN, "  Name = %s   state = ALIVE: %s",
            info.value[TBL_AUTHEN_FLD_KEY], info.value[TBL_AUTHEN_FLD_METHODS]);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        sal_printf(" %s\n", tmp);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_author(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char tmp[FILE_READ_LINE_LEN];
    char *p;
    
    sal_printf("Author queue = AAA_ML_AUTHOR_SHELL\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        snprintf(tmp, FILE_READ_LINE_LEN, "  Name = %s   state = ALIVE: %s",
            info.value[TBL_AUTHOR_FLD_KEY], info.value[TBL_AUTHOR_FLD_METHODS]);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        sal_printf(" %s\n", tmp);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_account(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char tmp[FILE_READ_LINE_LEN];
    char *p;
    
    sal_printf("Account queue = AAA_ML_ACCT_SHELL\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        snprintf(tmp, FILE_READ_LINE_LEN, "  Name = %s   state = ALIVE: %s",
            info.value[TBL_ACCOUNT_FLD_KEY], info.value[TBL_ACCOUNT_FLD_METHODS]);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        sal_printf(" %s\n", tmp);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_accountcmd(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char tmp[FILE_READ_LINE_LEN];
    char *p;
    
    sal_printf("Account queue = AAA_ML_ACCT_COMMAND\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        snprintf(tmp, FILE_READ_LINE_LEN, "  Name = %s   state = ALIVE: %s",
            info.value[TBL_ACCOUNTCMD_FLD_KEY], info.value[TBL_ACCOUNTCMD_FLD_METHODS]);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        sal_printf(" %s\n", tmp);
    }
    return PM_E_NONE;
}


int32
cdbctl_show_app_console(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char *p;
    int nValue;

    sal_printf("Current console configuration:\n");
    M_APP_SPLIT_OUT;
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        sal_printf("line console 0\n");
        sal_printf(" speed %s\n", info.value[TBL_CONSOLE_FLD_BAUDRATE]);
        sal_printf(" parity %s\n", info.value[TBL_CONSOLE_FLD_PARITY]);
        sal_printf(" databits %s\n", info.value[TBL_CONSOLE_FLD_DATABITS]);
        sal_printf(" stopbits %s\n", info.value[TBL_CONSOLE_FLD_STOPBITS]);

        sal_printf(" exec-timeout %s %s\n", 
            info.value[TBL_CONSOLE_FLD_TIMEOUT_MIN],
            info.value[TBL_CONSOLE_FLD_TIMEOUT_SEC]);
        sal_printf(" privilege level %s\n", info.value[TBL_CONSOLE_FLD_PRIVILEGE]);

        p = info.value[TBL_CONSOLE_FLD_PASSWD];
        if (NULL != p && '\0' != *p) {
            sal_printf(" line-password %s\n", p);
        } else {
             p = info.value[TBL_CONSOLE_FLD_ENC_PASSWD];
            if (NULL != p && '\0' != *p) {
                sal_printf(" line-password 8 %s\n", p);
            } else {
                sal_printf(" no line-password\n");
            }
        }

        p = info.value[TBL_CONSOLE_FLD_AUTH_METHOD];
        if (NULL != p && '\0' != *p) {
            sal_printf(" login authentication %s\n", p);
        }

        p = info.value[TBL_CONSOLE_FLD_LOGIN];
        nValue = cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, p);
        if (LOGIN_METHOD_NOLOGIN == nValue) {
            sal_printf(" no login\n");
        } else if (LOGIN_METHOD_LOGIN == nValue) {
            sal_printf(" login\n");
        } else if (LOGIN_METHOD_LOCAL == nValue) {
            sal_printf(" login local\n");
        }
        
        break;
    }

    return PM_E_NONE;
}

extern int32 
cdbctl_output_vty_config(FILE* cfg_fp, int nStart, int nEnd, char *pszLine);

int32
cdbctl_show_app_vty(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
#define _M_VTY_LINE_BUF_SIZE    1024
    char  line[_M_VTY_LINE_BUF_SIZE];
    char  newline[_M_VTY_LINE_BUF_SIZE];
    char *p;
    char *q;
    int nStart = 0;
    int nEnd = 0;
    char szBuf[256];
    /* modified by liwh for bug 43587, 2017-06-21 */
    char *p_aaa = NULL;
    /* liwh end */
    
    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/sys/sys_global/max_vty") == 0) {
        char szVtys[32];
        if (sal_cmd_get_token(szBuf, "max_vty", szVtys, 32) == 0) {
            sal_fprintf(stdout, "line vty maximum %d\n", atoi(szVtys));
        }
    }

    line[0] = '\0';
    while (NULL != sal_fgets(newline, _M_VTY_LINE_BUF_SIZE, fp))
    {
        p = strstr(newline, "inuse=");

        /* modified by liwh for bug 41166, 2016-10-18 */
        if (!p)
        {
            continue;
        }
        /* liwh end */
        
        *(p + 6) = '*';

        /* modified by liwh for bug 43587, 2017-06-21 */
        p_aaa = strstr(newline, "aaa_privilege=");
        if (p_aaa)
        {
            *(p_aaa + 14) = '\0';     
        }
        /* liwh end */
        
        nEnd = atoi(newline + 5);
        if ('\0' == line[0]) {
            snprintf(line, _M_VTY_LINE_BUF_SIZE, "%s", newline);
            continue;
        }

        p = strchr(line + 1, '/');
        q = strchr(newline + 1, '/');        
        if (!strcasecmp(p, q)) {
            /* the same line configuration */
            continue;
        }

        /* line configuration mismatch */
        cdbctl_output_vty_config(stdout, nStart, nEnd - 1, line);
        nStart = nEnd;
        snprintf(line, _M_VTY_LINE_BUF_SIZE, "%s", newline);
    }

    cdbctl_output_vty_config(stdout, nStart, nEnd, line);
    return PM_E_NONE;
}

int32
cdbctl_reboot_log(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info, char *pszCurTty)
{
    char  line[FILE_READ_LINE_LEN];
    char szCurTty[VTY_BUF_SIZE];
    cfg_parse_get_info_t info;
    int bCurUser = 0;
    char szUser[USERNAME_SIZE * 2];
    char *p_vty = "";
    char *p_ipaddr = "";
    char *p = NULL;

    sal_memset(szUser, 0, sizeof(szUser));
    
    if (pszCurTty) {
        snprintf(szCurTty, VTY_BUF_SIZE, "%s", pszCurTty);
        p = szCurTty;
        while ('\0' != *p) {
            if ('/' == *p) {
                *p = '-';
            }
            p++;
        }
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {        
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        p = info.value[TBL_LOGIN_FLD_KEY];
        if (pszCurTty && NULL != sal_strstr(szCurTty, p)) {
            bCurUser = 1;
            p_vty = info.value[TBL_LOGIN_FLD_VTY];
            p_ipaddr = info.value[TBL_LOGIN_FLD_IPADDR];
            break;
        }
    }

    if (bCurUser)
    {
        p = info.value[TBL_LOGIN_FLD_USER];
        if (NULL != p && '\0' != p[0] && strcasecmp(p, "")) {
            sal_b64_pton(p, (unsigned char *)szUser, USERNAME_SIZE * 2);
        } else {
            snprintf(szUser, USERNAME_SIZE * 2, "%s", "");
        }
        log_sys(M_MOD_CFG, E_WARNING, "The user chose yes when deciding whether to reboot the system. (Line=%s, Location=%s, User=%s)", p_vty, p_ipaddr, szUser);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_login(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char *pszCurTty = ttyname(STDIN_FILENO);
    char szTmp[128];
    struct stat vtyStatus;
    int bCurUser;
    int nValue;
    int nIdxOffset;
    char *p;
    char szCurTty[VTY_BUF_SIZE];
    char *pszKeyword = (char *)p_data;
    struct sal_timeval tv;
    struct sal_timeval tv_abs;
    struct sal_timeval tv_sub;
    char szUser[USERNAME_SIZE * 2];
    char *q;
    
    gettimeofday(&tv_abs, NULL);
    ctc_task_update_boottime(&tv);

    szCurTty[0] = '\0';

    if (!strcasecmp(pszKeyword, "reboot")) {
        cdbctl_reboot_log(p_data, data_len, fp, p_info, pszCurTty);
        return PM_E_NONE;
    } 
    
    if (!strcasecmp(pszKeyword, "web")) {
        sal_printf("%-32s %-19s %-15s %s\n",
            "Session Id", "Expire Time", "Client IP", "User Name");
        sal_printf("--------------------------------+-------------------+---------------+-----------\n");
    } else {
        if (pszCurTty) {
            snprintf(szCurTty, VTY_BUF_SIZE, "%s", pszCurTty);
            p = szCurTty;
            while ('\0' != *p) {
                if ('/' == *p) {
                    *p = '-';
                }
                p++;
            }
        }
        sal_printf(" %-10s%5s%-11s%-11s%-16s%-11s\n",
             "Line", " ", "Host(s)", "Idle", "Location", "User");
        sal_printf("---------------+----------+----------+---------------+----------\n");
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {        
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        p = info.value[TBL_LOGIN_FLD_PROTOCOL];
        if (!strcasecmp(pszKeyword, "web")) {
            if (NULL != p && strcasecmp(p, "web")) {
                continue;
            }

            q = info.value[TBL_LOGIN_FLD_USER];
            if (NULL != q && '\0' != q[0] && strcasecmp(q, "")) {
                sal_b64_pton(q, (unsigned char *)szUser, USERNAME_SIZE * 2);
            } else {
                snprintf(szUser, USERNAME_SIZE * 2, "%s", "");
            }
            nValue = atoi(info.value[TBL_LOGIN_FLD_EXPIRE_TIME]);
            timersub(&tv_abs, &tv, &tv_sub);
            format_time_str(szTmp, 128, "%Y-%m-%d %H:%M:%S",
                tv_sub.tv_sec + nValue);
            sal_printf("%-32s %-19s %-15s %s\r\n",
                info.value[TBL_LOGIN_FLD_KEY], szTmp,
                info.value[TBL_LOGIN_FLD_IPADDR],
                szUser);
            continue;
        }
        
        if (NULL != p && !strcasecmp(p, "web")) {
            continue;
        }

        p = info.value[TBL_LOGIN_FLD_KEY];
        if (pszCurTty && NULL != strstr(szCurTty, p)) {
            bCurUser = 1;
        } else {
            bCurUser = 0;
        }
        /* vty */
        p = info.value[TBL_LOGIN_FLD_VTY];
        memcpy(szTmp, p, 3);
        szTmp[3] = '\0';
        nValue = atoi(p + 3);
        if (strstr(p, "vty")) {
            nIdxOffset = 130;
        } else {
            nIdxOffset = 0;
        }
        sal_printf("%s%3d%4s%3d", bCurUser ? "*" : " ", 
            (nValue + nIdxOffset), szTmp, nValue);

        /* user */
        p = info.value[TBL_LOGIN_FLD_USER];
        if (NULL != p && '\0' != p[0] && strcasecmp(p, "")) {
            sal_b64_pton(p, (unsigned char *)szUser, USERNAME_SIZE * 2);
        } else {
            snprintf(szUser, USERNAME_SIZE * 2, "%s", "");
        }        
       sal_printf("%5s%-11s", " ", "idle");

        /* idle time */
        snprintf(szTmp, 128, "/dev/%s", info.value[TBL_LOGIN_FLD_KEY]);
        p = szTmp;
        while ('\0' != *p) {
            if ('-' == *p) {
                *p = '/';
            }
            p++;
        }
        memset(&vtyStatus, 0x00, sizeof(struct stat));
        if (!stat(szTmp, &vtyStatus)) {
            /* modified by wangyb for bug 44763, 2017-10-18 */
            nValue = time(NULL) - vtyStatus.st_atime;
            
            if (nValue <= 1) {
                snprintf(szTmp, 32, "%s", "00:00:00");
            } else if (nValue > 86400) {
                snprintf(szTmp, 32, "%dd%02dh%02dm",
                    (nValue / 86400) % 365, (nValue / 3600) % 24, (nValue / 60) % 60);
            } else {
                snprintf(szTmp, 32, "%02d:%02d:%02d",
                    (nValue / 3600) % 24, (nValue / 60) % 60, nValue % 60);
            }
        } else {
            snprintf(szTmp, 32, "%s", "--:--:--");
        }
        sal_printf("%-11s", bCurUser ? "00:00:00" : szTmp);

        /* ip */
        if (strstr(info.value[TBL_LOGIN_FLD_KEY], "con")
            || strstr(info.value[TBL_LOGIN_FLD_KEY], "ttyS0")
            || strstr(info.value[TBL_LOGIN_FLD_KEY], "ttyAMA0")
            || strstr(info.value[TBL_LOGIN_FLD_KEY], "ttyAMA1")
            || strstr(info.value[TBL_LOGIN_FLD_KEY], "ttyS000")) {
            snprintf(szTmp, 128, "%s", "127.0.0.1");
        } else {
            p = info.value[TBL_LOGIN_FLD_IPADDR];
            if (p && '\0' != *p) {
                snprintf(szTmp, 128, "%s", p);
            } else {
                snprintf(szTmp, 128, "%s", " ");
            }
        }
        sal_printf("%-16s%-11s\n", szTmp, szUser);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    unsigned char buf[SNMP_INFO_SIZE * 2];
    char *keyword = (char *)p_data;
    int nValue;
    char *p;

    if (0 == data_len) {
        return PM_E_NONE;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        
        if (!strcasecmp(keyword, "server_enable")) {
            nValue = atoi(info.value[0]); /* XXX: be careful */
            sal_printf("SNMP services: %s\n", nValue ? "enable" : "disable");
        } else if (!strcasecmp(keyword, "version")) {
            p = info.value[0];
            if (!strcasecmp(p, "all")) 
            {
                sal_printf("SNMP version: SNMPv1/SNMPv2c/SNMPv3\n");
            } 
            else if (!strcasecmp(p, "v1")) 
            {
                sal_printf("SNMP version: SNMPv1\n");
            } 
            else if (!strcasecmp(p, "v2c"))
            {
                sal_printf("SNMP version: SNMPv2c\n");
            } 
            else if (!strcasecmp(p, "v3"))
            {
                sal_printf("SNMP version: SNMPv3\n");
            }
        } else if (!strcasecmp(keyword, "engineid")) {
            p = info.value[0];
            if (NULL == p || '\0' == *p) {
                sal_printf("Engine ID   : 30383038303830383038\n");
            } else {
                char *pe = p;
                for (; *pe != '\0'; pe++)
                {
                    if (sal_isupper(*pe))
                    {
                        *pe = sal_tolower(*pe);
                    }
                }
                sal_printf("Engine ID   : %s\n", p);
            }
        } else if (!strcasecmp(keyword, "sys-info")) {
            p = info.value[TBL_SNMP_CFG_FLD_CONTACT];
            if (NULL == p || '\0' == *p) {
                sal_printf("Contact: \n");
            } else {
                sal_b64_pton(p, buf, SNMP_INFO_SIZE * 2);
                sal_printf("Contact: %s\n", buf);
            }
            p = info.value[TBL_SNMP_CFG_FLD_LOCATION];
            if (NULL == p || '\0' == *p) {
                sal_printf("Location: \n");
            } else {
                sal_b64_pton(p, buf, SNMP_INFO_SIZE * 2);
                sal_printf("Location: %s\n", buf);
            }
        } 
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_view(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t view_info;
    char line[FILE_READ_LINE_LEN];
    char *q = NULL;
    char *type = NULL;

    if (0 == data_len) {
        sal_printf("View-name                       View-type         Subtree\r\n");
        sal_printf("-------------------------------+-----------------+---------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_VIEW_FLD_KEY], &view_info);
        
        type = info.value[TBL_SNMP_VIEW_FLD_TYPE];
        
        if (0 == data_len) {
            sal_printf("%-31s %-17s %s\r\n", view_info.argv[0],
                type, view_info.argv[1]);
        } else {
            sal_printf("View name:     %s\r\n", view_info.argv[0]);
            sal_printf("View type:     %s\r\n", type);
            q = info.value[TBL_SNMP_VIEW_FLD_MASK];
            if (NULL == q || '\0' == *q) {
                sal_printf("Subtree mask:  N/A\r\n");
            } else {
                sal_printf("Subtree mask:  0x%s\r\n", q);
            }
            sal_printf("Subtree OID:   %s\r\n", view_info.argv[1]);
            sal_printf("Storage Type:  permanent\r\n");
            sal_printf("Row status:    active\r\n");
            sal_printf("Refcnt:        %s\r\n", info.value[TBL_SNMP_VIEW_FLD_REFCNT]);

        }
    }


    if (0 == data_len) {
        sal_printf("%-31s %-17s %s\r\n", "_all_", "included", ".0");
        sal_printf("%-31s %-17s %s\r\n", "_all_", "included", ".1");
        sal_printf("%-31s %-17s %s\r\n", "_all_", "included", ".2");
        sal_printf("%-31s %-17s %s\r\n", "_none_", "excluded", ".0");
        sal_printf("%-31s %-17s %s\r\n", "_none_", "excluded", ".1");
        sal_printf("%-31s %-17s %s\r\n", "_none_", "excluded", ".2");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_usm_user(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t info_snmp;
    cfg_parse_get_info_t usm_user_info;
    char line_snmp[FILE_READ_LINE_LEN];
    char line[FILE_READ_LINE_LEN];
    char *q = NULL;
    char *p = NULL;
    char *engineid = NULL;
    int rc = PM_E_NONE;

    PM_E_RETURN(cdbclt_get_tbl_line(TBL_SNMP_CFG, NULL, 0, line_snmp, FILE_READ_LINE_LEN));
    sal_memset(&info_snmp, 0, sizeof(info_snmp));
    cfg_get_parse(line_snmp, &info_snmp, 0);

    if (0 == data_len) 
    {
        sal_printf("Usm-user-name                   Auth Protocol\r\n");
        sal_printf("-------------------------------+---------------------------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_USM_USER_FLD_KEY], &usm_user_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s %s\r\n", usm_user_info.argv[0],
                cdb_enum_val2str(snmp_authen_strs , SNMP_AUTHENTICATION_MAX, atoi(info.value[TBL_SNMP_USM_USER_FLD_AUTH_TYPE])));
        } 
        else 
        {
            sal_printf("User Name:     %s\r\n", usm_user_info.argv[0]);
            if (sal_strlen(info.value[TBL_SNMP_USM_USER_FLD_REMO_ENGID]))
            {
                engineid = info.value[TBL_SNMP_USM_USER_FLD_REMO_ENGID];
            }
            else if (sal_strlen(info_snmp.value[TBL_SNMP_CFG_FLD_ENGINEID]))
            {
                engineid = info_snmp.value[TBL_SNMP_CFG_FLD_ENGINEID];
            }
            else
            {
                sal_printf("EnginedID:     30383038303830383038\r\n");
            }
            if (NULL != engineid)
            {
                char *pe = engineid;
                for (; *pe != '\0'; pe++)
                {
                    if (sal_isupper(*pe))
                    {
                        *pe = sal_tolower(*pe);
                    }
                }
                sal_printf("EnginedID:     %s\r\n", engineid);
            }

            sal_printf("Auth Protocol: %s\r\n", cdb_enum_val2str(snmp_authen_strs , SNMP_LEVEL_MAX, atoi(info.value[TBL_SNMP_USM_USER_FLD_AUTH_TYPE])));
            q = info.value[TBL_SNMP_USM_USER_FLD_AUTH_PWD];
            if (NULL == q || '\0' == *q) 
            {
                sal_printf("Auth encryption password: %s\r\n", info.value[TBL_SNMP_USM_USER_FLD_ENC_AUTH_PWD]);
            } 
            else 
            {
                sal_printf("Auth password: %s\r\n", q);
            }
            sal_printf("Priv Protocol: %s\r\n", cdb_enum_val2str(snmp_privacy_strs , SNMP_PRIVACY_MAX, atoi(info.value[TBL_SNMP_USM_USER_FLD_PRIV_TYPE])));
            p = info.value[TBL_SNMP_USM_USER_FLD_PRIV_PWD];
            q = info.value[TBL_SNMP_USM_USER_FLD_ENC_PRIV_PWD];
            if ((NULL == p || '\0' == *p) && ('\0' != *q))
            {
                sal_printf("Priv encryption password: %s\r\n", q);
            } 
            else 
            {
                sal_printf("Priv password: %s\r\n", p);
            }
            sal_printf("Storage Type:  nonvolatile\r\n");
            sal_printf("Row status:    active\r\n");
        }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_group_usm(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t group_info;
    char line[FILE_READ_LINE_LEN];

    if (0 == data_len) 
    {
        sal_printf("Group-name                      model  Security-name\r\n");
        sal_printf("-------------------------------+------+---------------------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_GROUP_USM_FLD_KEY], &group_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s %-6s %s\r\n", group_info.argv[0],"usm", group_info.argv[1]);
        } 
        else
        {
            sal_printf("Group name:     %s\r\n", group_info.argv[0]);
            sal_printf("Security model: v3\r\n");
            sal_printf("Security name:  %s\r\n", group_info.argv[1]);
            sal_printf("Storage Type:   permanent\r\n");
            sal_printf("Row status:     active\r\n");
        }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_access_usm(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t access_info;
    char line[FILE_READ_LINE_LEN];
    char *q = NULL;

    if (0 == data_len) 
    {
        sal_printf("Group-name                      model\r\n");
        sal_printf("-------------------------------+---------------------------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_ACCESS_USM_FLD_KEY], &access_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s usm\r\n", access_info.argv[0]);
        } 
        else
        {
            sal_printf("Group name:     %s\r\n", access_info.argv[0]);
            sal_printf("Context:        %s\r\n", info.value[TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME]);
            sal_printf("Security model: usm\r\n");
            sal_printf("Security level: %s\r\n", 
                cdb_enum_val2str(snmp_usm_level_strs , SNMP_LEVEL_MAX, atoi(info.value[TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL])));
            sal_printf("Context Match:  %s\r\n", atoi(info.value[TBL_SNMP_ACCESS_USM_FLD_ISPREFIX])? "prefix":"exact");
            
            q = info.value[TBL_SNMP_ACCESS_USM_FLD_READ_VIEW];
            if (NULL == q || '\0' == *q)
            {
                sal_printf("Read view:      _all_\r\n");
            } 
            else
            {
                sal_printf("Read view:      %s\r\n", q);
            }

            q = info.value[TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW];
            if (NULL == q || '\0' == *q)
            {
                sal_printf("Write view:     none\r\n");
            } 
            else
            {
                sal_printf("Write view:     %s\r\n", q);
            }
            q = info.value[TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW];
            if (NULL == q || '\0' == *q)
            {
                sal_printf("Notify view:    none\r\n");
            } 
            else
            {
                sal_printf("Notify view:    %s\r\n", q);
            }
            sal_printf("Storage Type:   permanent\r\n");
            sal_printf("Row status:     active\r\n");
        }
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_notify(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t notify_info;
    char line[FILE_READ_LINE_LEN];

    if (0 == data_len) 
    {
        sal_printf("Notify-name                     Notify-type\r\n");
        sal_printf("-------------------------------+---------------------------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_NOTIFY_FLD_KEY], &notify_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s %s\r\n", notify_info.argv[0],
                atoi(info.value[TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE])?"inform":"trap");
        } 
        else
        {
            sal_printf("Notify name:   %s\r\n", notify_info.argv[0]);
            sal_printf("Notify tag:    %s\r\n", info.value[TBL_SNMP_NOTIFY_FLD_TAG_NAME]);
            sal_printf("Notify type:   %s\r\n", atoi(info.value[TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE])?"inform":"trap");
            sal_printf("Storage Type:  nonvolatile\r\n");
            sal_printf("Row status:    active\r\n");
        }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_target_addr(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t target_info;
    char line[FILE_READ_LINE_LEN];

    if (0 == data_len) 
    {
        sal_printf("Targetaddress-name              IP-address        Mgmt-If Parameters\r\n");
        sal_printf("-------------------------------+-----------------+-------+----------\r\n");
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_TARGET_ADDR_FLD_KEY], &target_info);
        
        if (0 == data_len) 
        {
            if (sal_strlen(info.value[TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR]) <= 17)
            {
                sal_printf("%-31s %-17s %-7s %s\r\n", target_info.argv[0],
                    info.value[TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR],
                    atoi(info.value[TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF])? "yes":"no",
                    info.value[TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME]);
            }
            else
            {
                 sal_printf("%-31s %-40s \n", target_info.argv[0],
                    info.value[TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR]);
                 sal_printf("%-31s %-17s %-7s %s\r\n", " ", " ",
                    atoi(info.value[TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF])? "yes":"no",
                    info.value[TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME]);
            }
        } 
        else
        {
            sal_printf("Targetaddr name:     %s\r\n", target_info.argv[0]);
            sal_printf("IP address:          %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR]);
            sal_printf("Mgmt-If:             %s\r\n", atoi(info.value[TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF])?"yes":"no");
            sal_printf("UDP Port:            %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT]);    
            sal_printf("Timeout:             %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT]);
            sal_printf("Retry count:         %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_RETRIES]);
            sal_printf("Tag List:            %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST]);
            sal_printf("Parameters:          %s\r\n", info.value[TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME]);
            sal_printf("Storage Type:        nonvolatile\r\n");
            sal_printf("Row status:          active\r\n");
        }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_target_params(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t params_info;
    char line[FILE_READ_LINE_LEN];

    if (0 == data_len) 
    {
        sal_printf("Target-parameter-name           Security-name\r\n");
        sal_printf("-------------------------------+---------------------------\r\n");        
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_TARGET_PARAMS_FLD_KEY], &params_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s %s\r\n", params_info.argv[0],
                info.value[TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME]);
        } 
        else
        {
            sal_printf("Target parameter name:     %s\r\n", params_info.argv[0]);
            sal_printf("Message processing model:  v3\r\n");
            sal_printf("Security model:            v3\r\n");
            sal_printf("Security name:             %s\r\n", info.value[TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME]);
            sal_printf("Security level:            %s\r\n", 
                cdb_enum_val2str(snmp_usm_level_strs , SNMP_LEVEL_MAX, atoi(info.value[TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL])));
            sal_printf("Storage Type:              nonvolatile\r\n");
            sal_printf("Row status:                active\r\n");
        }
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_context(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t context_info;
    char line[FILE_READ_LINE_LEN];

    if (0 == data_len) 
    {
        sal_printf("Context-name\r\n");
        sal_printf("-------------------------------\r\n");      
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_CONTEXT_FLD_KEY], &context_info);
        
        if (0 == data_len) 
        {
            sal_printf("%-31s\r\n", context_info.argv[0]);
        } 
        else
        {
            sal_printf("Context name:   %s\r\n", context_info.argv[0]);
        }
    }

    return PM_E_NONE;
}



int32
cdbctl_show_app_snmp_community(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char buf[SNMP_INFO_SIZE];
    int nValue;

    sal_printf("Community-Access   Community-String     Security-name\r\n");
    sal_printf("------------------+--------------------+-------------------\r\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        nValue = atoi(info.value[TBL_SNMP_COMMUNITY_FLD_SECIDX]);
        snprintf(buf, SNMP_INFO_SIZE, "comm%d", nValue + 1);
        sal_printf("%-18s %-20s %-18s\r\n", info.value[TBL_SNMP_COMMUNITY_FLD_TYPE],
            info.value[TBL_SNMP_COMMUNITY_FLD_KEY], buf);
        
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_trap(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t trap_info;
    char line[FILE_READ_LINE_LEN];
    char is_mgmt[5] = "yes";


    sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", "Target-ipaddress", "mgmt-if", "udpport", "version", "pdu-type", "community");
    sal_printf("----------------+-------+-------+-------+--------+------------------------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        memset(&trap_info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_TRAP_FLD_KEY], &trap_info);

        if (trap_info.argc == 3)
        {   
#if 1 /* address optimization by wangqj */
            sal_strcpy(is_mgmt, sal_atoi(info.value[1]) ? "no" : "yes");

            if (sal_strlen(trap_info.argv[0]) <= 17)
            {
                sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", trap_info.argv[0], is_mgmt,
                  trap_info.argv[2], "v1", "trap",trap_info.argv[1]); 
                sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", trap_info.argv[0], is_mgmt,
                  trap_info.argv[2], "v2c", "trap",trap_info.argv[1]); 
            }
            else
            {
                 sal_printf("%-40s  \n",  trap_info.argv[0]);
                 sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", 
                    " ", is_mgmt, trap_info.argv[2], "v1", "trap", trap_info.argv[1]);
                 sal_printf("%-40s  \n",  trap_info.argv[0]);
                 sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", 
                    " ", is_mgmt, trap_info.argv[2], "v2c", "trap", trap_info.argv[1]);
            }
#else
            if (sal_strlen(trap_info.argv[0]) <= 17)
            {
                sal_printf("%-17s %-8s %-8s %-9s %-32s\n", trap_info.argv[0],
                  trap_info.argv[2], "v1", "trap",trap_info.argv[1]); 
                sal_printf("%-17s %-8s %-8s %-9s %-32s\n", trap_info.argv[0],
                  trap_info.argv[2], "v2c", "trap",trap_info.argv[1]); 
            }
            else
            {
                 sal_printf("%-40s  \n",  trap_info.argv[0]);
                 sal_printf("%-17s %-8s %-8s %-9s %-32s\n", 
                    " ", trap_info.argv[2], "v1", "trap", trap_info.argv[1]);
                 sal_printf("%-40s  \n",  trap_info.argv[0]);
                 sal_printf("%-17s %-8s %-8s %-9s %-32s\n", 
                    " ", trap_info.argv[2], "v2c", "trap", trap_info.argv[1]);
            }
#endif
        }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_snmp_inform(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t inform_info;
    char line[FILE_READ_LINE_LEN];
    char is_mgmt[5] = "yes";


    sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", "Target-ipaddress", "mgmt-if", "udpport", "version", "pdu-type", "community");
    sal_printf("----------------+-------+-------+-------+--------+------------------------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        memset(&inform_info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        cfg_get_parse_comma(info.value[TBL_SNMP_TRAP_FLD_KEY], &inform_info);

        if(inform_info.argc == 3)
        {   
#if 1 /* address optimization by wangqj */
            sal_strcpy(is_mgmt, sal_atoi(info.value[1]) ? "no" : "yes");

            if (sal_strlen(inform_info.argv[0]) <= 17)
            {
                sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", inform_info.argv[0], is_mgmt,
                  inform_info.argv[2], "v2c", "inform",inform_info.argv[1]); 
            }
            else
            {
                 sal_printf("%-40s  \n",  inform_info.argv[0]);
                 sal_printf("%-16s %-7s %-7s %-7s %-8s %-30s\n", 
                    " ", is_mgmt, inform_info.argv[2], "v2c", "inform", inform_info.argv[1]);
            }
#else
            if (sal_strlen(inform_info.argv[0]) <= 17)
            {
                sal_printf("%-17s %-8s %-8s %-9s %-32s\n", inform_info.argv[0],
                  inform_info.argv[2], "v2c", "inform",inform_info.argv[1]); 
            }
            else
            {
                 sal_printf("%-40s  \n",  inform_info.argv[0]);
                 sal_printf("%-17s %-8s %-8s %-9s %-32s\n", 
                    " ", inform_info.argv[2], "v2c", "inform", inform_info.argv[1]);
            }
#endif
        }
    }

    return PM_E_NONE;
}

static int 
_show_dhcsrvgrp_addr(int idx, int vrf, gensock_t *psu, void *pArg)
{
    char szAddrBuf[GENSOCK_ADDRSTRLEN + 1];

    sal_printf("    [%u] %s\n", idx + 1, 
               gensock2str(psu, szAddrBuf, GENSOCK_ADDRSTRLEN + 1));
    return 0;
}

int32
cdbctl_show_app_dhcsrvgrp(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    int nValue;
    char *p;

    sal_printf("DHCP server group information:\r\n");
    sal_printf("============================================================\r\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        p = info.value[TBL_DHCSRVGRP_FLD_ADDRS];
        if (NULL == p || '\0' == *p) {
            continue;
        }

        nValue = atoi(info.value[TBL_DHCSRVGRP_FLD_KEY]) + 1;
        sal_printf("group %u ip address list:\n", nValue);
        dhcsrvgrp_addr_iter(p, _show_dhcsrvgrp_addr, NULL);
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_app_dhcrelay(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *pszKeyword = (char *)p_data;
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char szCnt[64];
    char ifname_ext[64];
    unsigned long nValue;
    unsigned long nValue2;
    int rc;
 
    if (!data_len) {
        return PM_E_NONE;
    }
    
    if (!strcasecmp(pszKeyword, "statistics")) {
        sal_printf("DHCP relay packet statistics:\r\n");
        sal_printf("============================================================\r\n");
        if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "client_packets_relayed", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Client relayed packets          :  %lu\n", nValue);
            
            sal_cmd_get_token(line, "server_packets_relayed", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Server relayed packets          :  %lu\n\n", nValue);

            sal_cmd_get_token(line, "client_packet_errors", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Client error packets            :  %lu\n", nValue);

            sal_cmd_get_token(line, "server_packet_errors", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Server error packets            :  %lu\n", nValue);

            sal_cmd_get_token(line, "bogus_giaddr_drops", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Bogus GIADDR drop packets       :  %lu\n", nValue);

            sal_cmd_get_token(line, "bad_circuit_id", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Bad circuit ID packets          :  %lu\n", nValue);

            sal_cmd_get_token(line, "corrupt_agent_options", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Corrupted agent option packets  :  %lu\n", nValue);

            sal_cmd_get_token(line, "missing_agent_option", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Missing agent option packets    :  %lu\n", nValue);

            sal_cmd_get_token(line, "missing_circuit_id", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("Missing circuit ID packets      :  %lu\n", nValue);
        }
    } else if (!strcasecmp(pszKeyword, "interfaces")) {
        rc = cdbclt_get_tbl_line(TBL_SYS_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE]);
        nValue2 = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE]);
        sal_printf("List of DHCP relay enabled interface(s):\n");
        sal_printf("DHCP relay service status: %s\n",
                (nValue && nValue2) ?  "enabled" : "disabled");
        sal_printf("%-24s%s\n", "Interface Name", "DHCP server group");
        sal_printf("============================================================\r\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "dhcp_server_group", szCnt, 64);
            nValue = atoi(szCnt);
            if (nValue > 0 && nValue <= GLB_MAX_DHCSRVGRP_NUM) {
                sal_cmd_get_token(line, "key", szCnt, 64);
                /*added by yejl to fix bug 43485, 2017-06-13*/
                IFNAME_ETH2SHORT(szCnt, ifname_ext);
                /*ended by yejl*/
                sal_printf("%-24s%ld\n", ifname_ext, nValue);
            }
        }
    } else if (!strcasecmp(pszKeyword, "trusted")) {
        rc = cdbclt_get_tbl_line(TBL_DHCRELAY, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue = atoi(info.value[TBL_DHCRELAY_FLD_TRUST_ALL]);
        sal_printf("List of trusted sources of relay agent information option:\n");
        sal_printf("============================================================\r\n");
        if (nValue) {
            sal_printf("All interfaces are trusted source of relay agent information option\n");
            return PM_E_NONE;
        }

        rc = 1;
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "dhcp_relay_option_trust", szCnt, 64);
            nValue = atoi(szCnt);
            if (nValue) {
                sal_cmd_get_token(line, "key", szCnt, 64);
                IFNAME_ETH2SHORT(szCnt, ifname_ext);
                sal_printf("%-22s", ifname_ext);
                if ((rc % 3) == 0) { /* three interface name per line */
                    sal_printf("\n");
                }
                rc++;
            }
        }
        if ((rc % 3) != 1) {
            sal_printf("\n");
        }
    } else if (!strcasecmp(pszKeyword, "config")) {
        sal_printf("DHCP relay agent information configuration:\n");
        sal_printf("============================================================\r\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
            cfg_get_parse(line, &info, 0);
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS]);
            sal_printf("%sdhcp relay information option\n", nValue ? "" : "no ");
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH]);
            sal_printf("%sdhcp relay information check\n", nValue ? "" : "no ");
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_AGENT_PROC_OPTION]);
            sal_printf("dhcp relay information policy %s\n",
                       cdb_enum_val2str(agent_proc_option_strs, 
                                        AGENT_PROC_OPTION_MAX, nValue));
            break;
        }        
    }
    return PM_E_NONE;
}

int32
cdbctl_show_app_dhclient(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *pszKeyword = (char *)p_data;
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char szCnt[64];
    unsigned long nValue = 0;
    unsigned long nValue2 = 0;
    int32 rc = PM_E_NONE;
 
    if (!data_len) {
        return PM_E_NONE;
    }
    
    if (!strcasecmp(pszKeyword, "statistics")) {
        sal_printf("DHCP client packet statistics:\r\n");
        sal_printf("============================================================\r\n");
        if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "DHCP OFFER", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP OFFER     received:  %lu\n", nValue);
            
            sal_cmd_get_token(line, "DHCP ACK", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP ACK       received:  %lu\n\n", nValue);

            sal_cmd_get_token(line, "DHCP NAK", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP NAK       received:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP Others", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP Others    received:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP DISCOVER", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP DISCOVER      sent:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP DECLINE", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP DECLINE       sent:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP RELEASE", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP RELEASE       sent:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP REQUEST", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP REQUEST       sent:  %lu\n", nValue);

            sal_cmd_get_token(line, "DHCP packet send failed", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("DHCP packet send failed:  %lu\n", nValue);
        }
    } else if (!strcasecmp(pszKeyword, "interfaces")) {
        rc = cdbclt_get_tbl_line(TBL_SYS_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE]);
        nValue2 = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE]);
        sal_printf("List of DHCP relay enabled interface(s):\n");
        sal_printf("DHCP relay service status: %s\n",
                (nValue && nValue2) ?  "enabled" : "disabled");
        sal_printf("%-24s%s\n", "Interface Name", "DHCP server group");
        sal_printf("============================================================\r\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "dhcp_server_group", szCnt, 64);
            nValue = atoi(szCnt);
            if (nValue > 0 && nValue <= GLB_MAX_DHCSRVGRP_NUM) {
                sal_cmd_get_token(line, "key", szCnt, 64);
                sal_printf("%-24s%ld\n", szCnt, nValue);
            }
        }
    } else if (!strcasecmp(pszKeyword, "verbose")) {
        sal_printf("DHCP client informations:\n");
        sal_printf("============================================================\r\n");

        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {

            break;
        }
        
    } else if (!strcasecmp(pszKeyword, "config")) {
        sal_printf("DHCP relay agent information configuration:\n");
        sal_printf("============================================================\r\n");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
            cfg_get_parse(line, &info, 0);
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS]);
            sal_printf("%sdhcp relay information option\n", nValue ? "" : "no ");
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH]);
            sal_printf("%sdhcp relay information check\n", nValue ? "" : "no ");
            nValue = atoi(info.value[TBL_DHCRELAY_FLD_AGENT_PROC_OPTION]);
            sal_printf("dhcp relay information policy %s\n",
                       cdb_enum_val2str(agent_proc_option_strs, 
                                        AGENT_PROC_OPTION_MAX, nValue));
            break;
        }        
    }
    return rc;
}

int32
cdbctl_show_app_dhcsnooping(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *pszKeyword = (char *)p_data;
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    char szCnt[64];
    char ifname_ext[64];
    unsigned long long nValue;
    unsigned long long nValue2;
    unsigned long long nValue3;
    int32 rc = PM_E_NONE;
    char *p = NULL;
 
    if (!data_len) {
        return PM_E_NONE;
    }
    
    if (!strcasecmp(pszKeyword, "statistics")) {
        sal_printf("DHCP snooping statistics:\n");
        sal_printf("============================================================\n");
        if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {        
            sal_cmd_get_token(line, "dhcp_packets", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "DHCP packets received", nValue);
            
            sal_cmd_get_token(line, "bootp_packets", szCnt, 64);
            nValue = strtoull(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "BOOTP packets received", nValue);

            sal_cmd_get_token(line, "forwarded_packets", szCnt, 64);
            nValue = strtoull(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "Packets forwarded", nValue);

            sal_cmd_get_token(line, "invalid_packets", szCnt, 64);
            nValue = strtoull(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "Packets invalid", nValue);

            sal_cmd_get_token(line, "mac_verify_failed_packets", szCnt, 64);
            nValue = strtoull(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "Packets MAC address verify failed", nValue);

            sal_cmd_get_token(line, "dropped_packets", szCnt, 64);
            nValue = strtoull(szCnt, NULL, 10);
            sal_printf("%-42s%lld\n", "Packets dropped", nValue);
        }
    } else if (!strcasecmp(pszKeyword, "trusted")) {
        rc = cdbclt_get_tbl_line(TBL_DHCRELAY, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue = atoi(info.value[TBL_DHCRELAY_FLD_TRUST_ALL]);
        sal_printf("List of DHCP snooping trusted interface(s):\n");
        sal_printf("============================================================\r\n");
        rc = 1;
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            sal_cmd_get_token(line, "dhcp_snooping_trust", szCnt, 64);
            nValue = atoi(szCnt);
            if (nValue) {
                sal_cmd_get_token(line, "key", szCnt, 64);
                IFNAME_ETH2SHORT(szCnt, ifname_ext);
                sal_printf("%-22s", ifname_ext);
                if ((rc % 3) == 0) { /* three interface name per line */
                    sal_printf("\n");
                }
                rc++;
            }
        }
        if ((rc % 3) != 1) {
            sal_printf("\n");
        } else if (1 == rc) {
            sal_printf("none\n");
        }
    } else if (!strcasecmp(pszKeyword, "config")) {

        rc = cdbclt_get_tbl_line(TBL_DHCSNOOPING, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue3 = atoi(info.value[TBL_DHCSNOOPING_FLD_VLANS]);

        sal_memset(&info, 0, sizeof(info));
        rc = cdbclt_get_tbl_line(TBL_SYS_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
        rc = cfg_get_parse(line, &info, 0);
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE]);
        nValue2 = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE]);
        sal_printf("dhcp snooping service: %s\n", 
            (nValue && nValue2 && nValue3) ? "enabled" : "disabled");
        sal_printf("dhcp snooping switch: %s\n", nValue2 ? "enabled" : "disabled");
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
            memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
            cfg_get_parse(line, &info, 0);
            nValue = atoi(info.value[TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS]);
            sal_printf("Verification of hwaddr field: %s\n", 
                nValue ? "enabled" : "disabled");

            nValue = atoi(info.value[TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS]);
            sal_printf("Insertion of relay agent information (option 82): ");
            if (nValue) {
                sal_printf("enabled\n");
            } else {
                sal_printf("disabled\n");
            }

            nValue = atoi(info.value[TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED]);
            sal_printf("Relay agent information (option 82) on untrusted port: %s\n", 
                nValue ? "allowed" : "not allowed");

            p = info.value[TBL_DHCSNOOPING_FLD_VLANS];
            if (NULL != p && '\0' != *p) { 
                sal_printf("dhcp snooping vlan %s\n", p);
            }
            break;
        }        
    }
    return rc;
}

#define DHCPSNOOPING_MAX_TOKEN_CNT      6

int32
cdbctl_show_app_dhcbinding(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    struct sal_timeval tv; 
    sal_time_t nCurTime;
    char *pszKeyword = (char *)p_data;
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    char szCnt[64];
    u_int32_t nValue;
    u_int32_t nValue2;
    char szLease[32];
    char *p;
    char szMac[15];
    char szType[32];
 
    if (!data_len) {
        return PM_E_NONE;
    }

    if (!strcasecmp(pszKeyword, "summary")) {
        if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {        
            sal_cmd_get_token(line, "dhcp_binding_learning_count", szCnt, 64);
            nValue = strtoul(szCnt, NULL, 10);
            
            sal_cmd_get_token(line, "dhcp_binding_configed_count", szCnt, 64);
            nValue2 = strtoul(szCnt, NULL, 10);
            sal_printf("Total %u DHCP snooping binding entries:\n",
                        nValue + nValue2);
            sal_printf("    %u learning entry, %u configured entry\n",
                        nValue, nValue2);
        }
        return PM_E_NONE;
    }

    if (!strcasecmp(pszKeyword, "database")) 
    {
        FILE *fp = NULL;
        char szBuf[256] = {'\0'};
        char *p = NULL;
        char *q = NULL;  
        char **ppToks = NULL;
        int nCurLine = 0;
        unsigned long nCnt = 0;
        int type = 0;
        sal_time_t nLeaseTime = 0;
        sal_time_t absTime = sal_time(NULL);
        
        fp = sal_fopen(DHCPSNOOPING_SAVE_BINDING_FILE, "r");
        if (!fp)
        {
            sal_printf("DHCP snooping binding file not exist");
            return PM_E_NONE;
        }

        sal_printf("DHCP snooping binding database:\n");
        sal_printf("VLAN MAC Address    Interface  Lease(s)   IP Address      Type\n");
        sal_printf("====================================================================\n");

        while (sal_fgets(szBuf, 256, fp) != NULL)
        {
            nCurLine++;
            p = szBuf;
            /* trim the last \n & \r */
            if ('\n' == p[sal_strlen(p) - 1])
            {
                p[sal_strlen(p) - 1] = '\0';
            }
            if ('\r' == p[sal_strlen(p) - 1])
            {
                p[sal_strlen(p) - 1] = '\0';
            }
            
            /* trim space */
            while ('\0' != *p && ' ' == *p)
            {
                p++;
            }

            /* space line */
            if ('\0' == *p || '#' == *p)
            {
                continue;
            }

            /* trim # comment */
            if ((q = sal_strchr(p, '#')) != NULL)
            {
                *q = '\0';
            }
            if (sal_strlen(p) <= 0)
            {
                continue;
            }

            if (split_tokens(p, sal_strlen(p), DHCPSNOOPING_MAX_TOKEN_CNT, 
                             " \t", &nCnt, &ppToks) != 0)
            {
                sal_printf("%% Invalid line %d: %s\n", nCurLine, p);
                return PM_E_NONE;
            }       

            if (DHCPSNOOPING_MAX_TOKEN_CNT != nCnt)
            {
                sal_printf("%% Invalid line %d: %s\n", nCurLine, p);
                free_tokens(&ppToks);
                return PM_E_NONE;
            }

            type = sal_atoi(ppToks[0]);
            if (DHCPSNOOPING_ITEM_TYPE_LEARNING != type && DHCPSNOOPING_ITEM_TYPE_CONFIGED != type)
            {
                sal_printf("Ivalid binding item for not exist type %s at line %d\n", ppToks[0], nCurLine);
                return PM_E_NONE;
            }

            nLeaseTime = sal_strtou32(ppToks[4], NULL, 10);
            if (nLeaseTime != 0)
            {
                nLeaseTime = nLeaseTime - absTime;
                if (nLeaseTime <= 0)
                {
                    continue;
                }
            }
            
            sal_printf("%-4s %-8s %-10s %-10d %-15s %-8s\n",
                ppToks[1], ppToks[2], ppToks[3], nLeaseTime, ppToks[5], (DHCPSNOOPING_ITEM_TYPE_CONFIGED == type) ? "manual" : "learning");

            free_tokens(&ppToks);
        }

        sal_fclose(fp);
        return PM_E_NONE;
    }
    
    ctc_task_update_boottime(&tv);
    nCurTime = ctc_time_boottime_sec(NULL);

    char ifname_ext[64];
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_printf("DHCP snooping binding table:\n");
    sal_printf("VLAN MAC Address    Interface  Lease(s)   IP Address      Type\n");
    sal_printf("====================================================================\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        p = info.value[TBL_DHCBINDING_FLD_KEY];
        if (NULL == p || '\0' == *p) {
            sal_printf("    Internal error!!!\n");
            break;
        }
        sal_snprintf(szMac, 15, p);

        nValue = strtoul(info.value[TBL_DHCBINDING_FLD_LEASE], NULL, 10);
        if (nValue) {
            sal_snprintf(szLease, 32, "%ld", 
                ((nValue < nCurTime) ? 0 : (nValue - nCurTime)));
        } else if (!strcasecmp(info.value[TBL_DHCBINDING_FLD_TYPE], "configed")) {
            sal_snprintf(szLease, 32, "static");
                      
        } else {
            /*add by weij, for bug 4029, learning a infinite binding*/
            sal_snprintf(szLease, 32, "infinite");
            
        }

        /* modified by liwh for bug 50963, 2019-01-30 */
        if (!strcasecmp(info.value[TBL_DHCBINDING_FLD_TYPE], "configed")) {
            sal_snprintf(szType, 32, "manual");    
        }
        else
        {
            sal_snprintf(szType, 32, "learning");
        }
        /* liwh end */

        IFNAME_ETH2SHORT(info.value[TBL_DHCBINDING_FLD_IFNAME], ifname_ext);
        sal_printf("%-4s %s %-10s %-10s %-15s %-8s\n", 
              info.value[TBL_DHCBINDING_FLD_VID],
              szMac, ifname_ext,
              szLease, info.value[TBL_DHCBINDING_FLD_CLIENT_IP], 
              szType);
    }        

    return PM_E_NONE;
}

int32
cdbctl_show_app_static_dns(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int host_num = 1;
    sal_printf("Current IP host configuration:\n");
    sal_printf("%-4s %-63s %s\n", "", "Host", "Address");
    sal_printf("----+---------------------------------------------------------------+-------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        /*modified by yejl for bug 54022, 2019-10-17*/
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        sal_printf("%-4d %-63s %s\n", host_num++, info.value[TBL_STATIC_DNS_FLD_KEY],
            info.value[TBL_STATIC_DNS_FLD_IP_ADDR]);
        
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_ntp_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int nlines = 0;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
#if 0        
        if(sal_strcmp(info.value[TBL_NTP_CFG_FLD_BCASTDELAY], "0"))
        {
             sal_printf( "Broadcast delay: %s\n", info.value[TBL_NTP_CFG_FLD_BCASTDELAY]);
        }
#endif        
        if(sal_strcmp(info.value[TBL_NTP_CFG_FLD_AUTHENTICATION], "0"))
        {
             sal_printf( "Authentication: enabled\n");
        }
        
        if(sal_strcmp(info.value[TBL_NTP_CFG_FLD_MINIMUMDISTANCE], "0"))
        {
             sal_printf( "minimum-distance: %s\n", info.value[TBL_NTP_CFG_FLD_MINIMUMDISTANCE]);
        }
#if 0        
        sal_printf( "Local reference clock: \n");
        if(sal_strcmp(info.value[TBL_NTP_CFG_FLD_BREFCLOCK], "0"))
        {
             sal_printf( "  enabled, stratum %s\n\n", info.value[TBL_NTP_CFG_FLD_REFCLOCK_STRATUM]);
        }
#endif        
        nlines++;
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_ntp_key(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];

    sal_printf("Current NTP key configuration: \n");
    sal_printf("Flags: * - Trusted\n");
    sal_printf("  %-6s %s\n", "ID", "Value");
    sal_printf("-+------+--------------------------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        /* modified by huxd for bug 53987, 2019-10-22 */
        if(sal_strcmp(info.value[TBL_NTP_KEY_FLD_TRUSTED], "0"))
        {
            sal_printf("* %-6s %s\n", info.value[TBL_NTP_KEY_FLD_KEY], info.value[TBL_NTP_KEY_FLD_VALUE]);
        }
        else
        {
            sal_printf("  %-6s %s\n", info.value[TBL_NTP_KEY_FLD_KEY], info.value[TBL_NTP_KEY_FLD_VALUE]);
        }
        /* end huxd */
    }

    return PM_E_NONE;
}


int32
cdbctl_show_app_ntp_ace(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int nlines = 0;
    uint32 option = 0;
    int i;

    static char * option_name[] =
    {
        "version",
        "kod",
        "ignore",
        "noquery",
        "nomodify",
        "notrap",
        "lowpriotrap",
        "noserve",
        "nopeer",
        "notrust",
        "limited",
        "ntpport"
    };

    sal_printf("Current NTP configuration: \n");
    sal_printf("NTP access control list:\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        
        sal_printf("%s ", 
            info.value[TBL_NTP_ACE_FLD_KEY]);

        if(sal_strcmp(info.value[TBL_NTP_ACE_FLD_MASKLEN], "32"))
        {
             sal_printf( "mask %s ", info.value[TBL_NTP_ACE_FLD_MASKLEN]);
        }
        
        option = sal_strtou32(info.value[TBL_NTP_ACE_FLD_OPTION], NULL, 10);

             
        for (i=0; i<13; i++) /*note 13 is BITOFFSET_END*/
        {
            if(((option>>i)&0x01))
            {
                sal_printf( "%s ", option_name[i]);
            }
        }
        sal_printf( "\n");
        
        nlines++;
    }

    return PM_E_NONE;
}


int32
cdbctl_show_app_ntp_server(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int nlines = 0;

    sal_printf("Current NTP configuration: \n");
    sal_printf("=========================================================== \n");
    sal_printf( "Unicast server: \n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);
        if(!sal_strcmp(info.value[TBL_NTP_SERVER_FLD_TYPE], "2")) /*M_NTP_TYPE_UCAST_CLIENT*/
        {
            sal_printf( "%s ", info.value[TBL_NTP_SERVER_FLD_KEY]);
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_KEYID], "0"))
            {
                sal_printf( "key %s ", info.value[TBL_NTP_SERVER_FLD_KEYID]);
            }
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_VERSION], "0"))
            {
                sal_printf( "version %s ", info.value[TBL_NTP_SERVER_FLD_VERSION]);
            }
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_PREFER], "0"))
            {
                sal_printf( "prefer ");
            }  
            sal_printf( "peer ");
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_IS_INBAND], "1"))
            {
                sal_printf( "(mgmt-if) ");
            }
            sal_printf( "\n");
            nlines++;
        }
        else
        {
            sal_printf( "%s ", info.value[TBL_NTP_SERVER_FLD_KEY]);
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_KEYID], "0"))
            {
                sal_printf( "key %s ", info.value[TBL_NTP_SERVER_FLD_KEYID]);
            }
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_VERSION], "0"))
            {
                sal_printf( "version %s ", info.value[TBL_NTP_SERVER_FLD_VERSION]);
            }
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_PREFER], "0"))
            {
                sal_printf( "prefer ");
            } 
            sal_printf( "server ");
            if(sal_strcmp(info.value[TBL_NTP_SERVER_FLD_IS_INBAND], "1"))
            {
                sal_printf( "(mgmt-if) ");
            }
            sal_printf( "\n");
            nlines++;
        }
        
    }

    return PM_E_NONE;
}



int32
cdbctl_show_app_dynamic_dns_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int domain_num = 1;

    sal_printf("Current DNS domain configuration:\n");
    sal_printf("%-4s %-20s %s\n", "", "Domain", "Suffix");
    sal_printf("----+--------------------+------------------------------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        sal_printf("%-4d %-20s %s\n", domain_num++, DOMAIN_TAG, info.value[TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY]);
        
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_dynamic_dns_server(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    int server_num = 1;

    sal_printf("Current DNS name server configuration:\n");
    sal_printf("%-4s %-20s %s\n", "", "Server", "IP Address");
    sal_printf("----+--------------------+------------------------------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        sal_printf("%-4d %-20s %s\n", server_num++, NAME_SERVER_TAG, info.value[TBL_DYNAMIC_DNS_SERVER_FLD_KEY]);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_app_cpu_utilization(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char line[FILE_READ_LINE_LEN];
    app_cpu_utilization_t tmp_info[256];
    app_cpu_utilization_t tmp;
    app_cpu_utilization_t total;
    float other_cpu_usage = 0;
    float process_cpu_usage = 0;
    char key[CMD_BUF_64];
    char *p = NULL;
    int i = 0, j = 0, k = 0;

    if (0 == sal_strcmp("description", p_data))
    {
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
        {
            sal_printf(line);
        }
        return PM_E_NONE;
    }
                
    sal_printf("%-20s %-15s\n", "Process", "Usage(%)");
    sal_printf("--------------------+----------\n");

    sal_memset(&tmp_info, 0, sizeof(tmp_info));
    sal_memset(&tmp, 0, sizeof(tmp));
    sal_memset(&total, 0, sizeof(total));
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp)) 
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        /* Added by kcao 2018-12-18 for bug 50433, check parse result */
        if (info.argc < TBL_CPU_UTILIZATION_FLD_MAX)
        {
            continue;
        }
        if (sal_strcmp(info.value[TBL_CPU_UTILIZATION_FLD_CPU_USAGE], "0.000000"))
        {
            if (0 == sal_strcmp(info.value[TBL_CPU_UTILIZATION_FLD_KEY], "Total"))
            {
                sal_sscanf(info.value[TBL_CPU_UTILIZATION_FLD_CPU_USAGE], "%f", &total.cpu_usage);
                sal_strcpy(total.name, info.value[TBL_CPU_UTILIZATION_FLD_KEY]);
            }
            else
            {
                sal_sscanf(info.value[TBL_CPU_UTILIZATION_FLD_CPU_USAGE], "%f", &tmp_info[i].cpu_usage);
                sal_memset(key, 0, sizeof(key));
                sal_strcpy(key, info.value[TBL_CPU_UTILIZATION_FLD_KEY]);
                p = sal_strchr(key, '#');
                if (p)
                {
                    p = p + 1;
                }
                else
                {
                    p = key;
                }
                sal_strcpy(tmp_info[i].name, p);
                i++;
                k++;
            }
        }
    }

    for (i = 0; i < k; i++)
    {
        for (j = i + 1; j < k; j++)
        {
            if (tmp_info[i].cpu_usage < tmp_info[j].cpu_usage)
            {
                sal_memcpy(&tmp, &tmp_info[i], sizeof(app_cpu_utilization_t)); 
                sal_memcpy(&tmp_info[i], &tmp_info[j], sizeof(app_cpu_utilization_t)); 
                sal_memcpy(&tmp_info[j], &tmp, sizeof(app_cpu_utilization_t)); 
            }
        }
    }

    for (i = 0; i < k; i++)
    {
        sal_printf("%-20s %-15.2f\n", tmp_info[i].name, tmp_info[i].cpu_usage);
        process_cpu_usage += tmp_info[i].cpu_usage;
    }
    other_cpu_usage = total.cpu_usage - process_cpu_usage;
    other_cpu_usage = (other_cpu_usage >= 0) ? other_cpu_usage : 0;
    sal_printf("%-20s %-15.2f\n", "others", other_cpu_usage);
    sal_printf("-------------------------------\n");
    sal_printf("%-20s %-15.2f\n", total.name, total.cpu_usage);
        
    return PM_E_NONE;
}

int32
cdbctl_show_sys_enable(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
#define _M_ENABLE_LINE_BUF_SIZE    512
    char  line[_M_ENABLE_LINE_BUF_SIZE];
    cfg_parse_get_info_t info;
    char *p;
    char *q;
    int pri;

    sal_printf("%-20s%-20s%s\r\n", "Privilege Level", "Enable Password", "Encrypt");
    sal_printf("============================================================\r\n");
    
    while (NULL != sal_fgets(line, _M_ENABLE_LINE_BUF_SIZE, fp)) {
        memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
        cfg_get_parse(line, &info, 0);

        if (NULL != info.value[TBL_ENABLE_FLD_KEY]) {
            pri = atoi(info.value[TBL_ENABLE_FLD_KEY]) + 1;
        } else {
            pri = 1;
        }
        p = info.value[TBL_ENABLE_FLD_PASSWD];
        q = info.value[TBL_ENABLE_FLD_ENC_PASSWD];
        if (NULL != p && sal_strcasecmp(p, "")) {
            sal_printf("%-20d%-20s\r\n", pri, p);
        } else if (NULL != q && sal_strcasecmp(q, "")) {
            sal_printf("%-20d%-20s%s\r\n", pri, q, "*");
        }
    }

    return PM_E_NONE;
}





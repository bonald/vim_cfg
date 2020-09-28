/*
 * shell_execute.c
 */
#include "private.h"
#include "lub/string.h"
#include "lub/argv.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "lib_fs.h"
#include "sal.h"
#include "proto.h"
#include "cdb_const.h"
#include "rsa_derhex.h"
#include <openssl/rand.h>
#include <net/if.h>
#include <errno.h>
#include "lib_syslimit.h"
#include "glb_tempfile_define.h"

/* var name for RSA key */
#define M_VAR_KEY_STRING        "rsa-key-string"
#define M_VAR_KEY_FORMAT        "rsa-key-format"
#define M_VAR_KEY_ENCPASSWORD   "rsa-key-encpassword"
#define M_VAR_KEY_PASSWORD      "rsa-key-password"
#define M_VAR_KEY_TYPE          "rsa-key-type"
#define PASSWORD_LEN            512
#define MAX_LINE_CHAR           50

/* var name for global enable */
#define DEFAULT_ENABLE_PRIVILEGE        4
#define DEFAULT_ENABLE_TIMEOUT          600000 /*600000(s), vty default timeout value*/

typedef struct {
	char *pszFmt;
	char *pszKey;
	char *pszKeyType;
	char *pszPasswd;
	char *pszEncPasswd;
	char szPassword[PASSWD_SIZE];
} rsa_var_config_t;

static char *g_rsa_key_buf = NULL;
static int g_is_startup_cfg = 0;
static int g_is_cdbclient_ok = 0;
static int g_is_show_diag = 0;
static int g_log_oper_cmd_level = LOG_OPER_CMD_LEVEL_CLI;

extern int glb_pri;
extern int glb_timeout;
extern int glb_is_rpc;
extern int glb_is_mnt_valid;

extern int clish_update_vty_cmd_buf(char *cmd_buf);

int isStartupCfg()
{
    return g_is_startup_cfg;
}

int isCdbClientOK()
{
    return g_is_cdbclient_ok;
}

void setStartupCfgFlag(int onoff)
{
    int32 rc = 0;
    lib_init(PM_ID_CTL);
    ctc_task_init();
    rc = ipc_connect_ccs(3);
    if (rc < 0)
    {
        g_is_cdbclient_ok = 0;
    }
    g_is_cdbclient_ok = 1;
    g_is_startup_cfg = onoff;
}

void setShowDiagFlag(int onoff)
{
    g_is_show_diag = onoff;
}

void setLogOperCMDLevel(int level)
{
    g_log_oper_cmd_level = level;
}

/*
 * These are the internal commands for this framework.
 */
static clish_shell_builtin_fn_t
    clish_close,
    clish_overview,
    clish_source,
    clish_source_nostop,
    clish_history,
    clish_nested_up,
    clish_nop,
    clish_wdog,
    clish_macros,
    clish_terminal_length,
    clish_chdir,
    clish_ctc_chdir_mnt,
    clish_enter_rsakey_mode,
    clish_leave_rsakey_mode,
    clish_validate_rsakey,
    clish_generate_rsakey,
    clish_chvar,
    clish_interface_range,
    clish_ctc_cfgtrace,
    clish_ctc_get_privilege,
    clish_banner,
    clish_enable,
    clish_disable,
    clish_rpcapi_user;

static clish_shell_builtin_t clish_cmd_list[] = {
	{"clish_close", clish_close},
	{"clish_overview", clish_overview},
	{"clish_source", clish_source},
	{"clish_source_nostop", clish_source_nostop},
	{"clish_history", clish_history},
	{"clish_nested_up", clish_nested_up},
	{"clish_nop", clish_nop},
	{"clish_wdog", clish_wdog},
	{"clish_macros", clish_macros},
	{"clish_ctc_cfgtrace", clish_ctc_cfgtrace},
	{"clish_terminal_length", clish_terminal_length},
	{"clish_chdir", clish_chdir},
	{"clish_ctc_chdir_mnt", clish_ctc_chdir_mnt},
	{"clish_enter_rsakey_mode", clish_enter_rsakey_mode},
	{"clish_leave_rsakey_mode", clish_leave_rsakey_mode},
	{"clish_validate_rsakey", clish_validate_rsakey},
	{"clish_generate_rsakey", clish_generate_rsakey},
	{"clish_chvar", clish_chvar},
	{"clish_interface_range", clish_interface_range},
	{"clish_ctc_get_privilege", clish_ctc_get_privilege},
	{"clish_banner", clish_banner},
	{"clish_enable", clish_enable},
	{"clish_disable", clish_disable},
	{"clish_rpcapi_user", clish_rpcapi_user},
	{NULL, NULL}
};

static clish_var_t *__find_var(clish_context_t *context, const char *varname)
{
    clish_var_t *var = NULL;
    int depth = 0;

    depth = clish_shell__get_depth(context->shell);
    var = lub_bintree_find(&context->shell->pwdv[depth]->viewid, varname);
    if (NULL == var) {
        var = lub_bintree_find(&context->shell->var_tree, varname);
    }
    return var;
}

static const char *__get_var_value(clish_context_t *context, const char *varname)
{
    clish_var_t *var = __find_var(context, varname);

    if (NULL != var) {
        return clish_var__get_value(var);
    }
    return NULL;
}

static int __set_var(clish_context_t *context, const char *name, const char *value, int bReplace)
{
    clish_var_t *var = __find_var(context, name);
    char *pszCurValue;
    int alen;
    char *p;

    if (NULL == var || NULL == g_rsa_key_buf) {
        fprintf(stderr, "%% __set_var: Can't find var %s\n", name);
        return -1;
    }

    if (bReplace || NULL == (pszCurValue = clish_var__get_value(var))
            || !strcasecmp(pszCurValue, "")) {
        clish_var__set_value(var, value);
        return 0;
    }

    alen = strlen(pszCurValue) + strlen(value) + 3;
    if (alen >= M_RSA_KEYSTR_BUFSZ) {
        fprintf(stderr, "%% String too long\n");
        return -1;
    }
    p = g_rsa_key_buf + M_RSA_KEYSTR_BUFSZ;
    snprintf(p, M_RSA_KEYSTR_BUFSZ, "%s\n%s", pszCurValue, value);
    clish_var__set_value(var, p);
    return 0;
}

static int clish_check_password(const char *pszInput, char *pszCfg, int bEnc)
{
    if (NULL == pszInput || '\0' == pszInput[0]) {
        printf("%% Invalid password\n");
        return -1;
    }
    
    if (bEnc) {
        if (gen_passwd_validate(pszCfg, (char *)pszInput) != 1) {
            printf("%% Invalid password\n");
            return -1;
        }
    } else if (strcmp(pszInput, pszCfg)) {
        printf("%% Invalid password\n");
        return -1;
    }
 
    return 0;
}
static int
check_key_limits()
{
    char szBuf[64];
    char szCount[32];

    /* check key count */
    if (sal_cmd_exec(szBuf, 64, "cdbctl read/cdb/app/rsa/count") != 0) {
        fprintf(stderr, "%% Check RSA key count failed\n");
        return -1;
    }
    
    sal_cmd_get_token(szBuf, "count", szCount, 32); /* XXX */
    if (atoi(szCount) >= GLB_MAX_USER_NUM) {
        fprintf(stderr, "%% Too many keys. Please delete some keys first\n");
        return -1;
    }
    
    return 0;
}

int __validate_rsa_keystring(clish_context_t *context, 
    rsa_var_config_t *cfg, RSA ** pprsa, char *pszErr, size_t err_bufsz)
{
	const char *pszFmt;
	const char *pszKey;
	const char *pszKeyType;
	const char *pszPasswd;
	const char *pszEncPasswd;
	char szPassword[PASSWD_SIZE];
	int ret = -1;
	char *p;

    pszKey = __get_var_value(context, M_VAR_KEY_STRING);
    if (NULL == pszKey) {
        snprintf(pszErr, err_bufsz, "Can't get key string");
        goto err_out;
    }

    p = (char *)pszKey;
    while ('\0' != *p) {
        if (' ' == *p || '\t' == *p || '\r' == *p || '\n' == *p) {
            p++;
            continue;
        }
        break;
    }
    if ('\0' == *p) {
        snprintf(pszErr, err_bufsz, "Empty key string");
        goto err_out;
    }
    
    pszKeyType = __get_var_value(context, M_VAR_KEY_TYPE);
    if (NULL == pszKeyType) {
        snprintf(pszErr, err_bufsz, "Can't get key type");
        goto err_out;
    }
    pszFmt = __get_var_value(context, M_VAR_KEY_FORMAT);
    if (NULL == pszFmt) {
        snprintf(pszErr, err_bufsz, "Can't get key format");
        goto err_out;
    }
    pszPasswd = __get_var_value(context, M_VAR_KEY_PASSWORD);
    pszEncPasswd = __get_var_value(context, M_VAR_KEY_PASSWORD);
    if (NULL == pszPasswd || NULL == pszEncPasswd) {
        snprintf(pszErr, err_bufsz, "Can't get key password");
        goto err_out;
    }
    if (!strcasecmp(pszPasswd, "") && !strcasecmp(pszEncPasswd, "")) {
        /* no password */
    } else if (strcasecmp(pszEncPasswd, "")) {
        /* have encrypted password */
        if (do_decrypt(M_DEFAULT_PW_STR, (char *)pszEncPasswd, strlen(pszEncPasswd),
                       szPassword, PASSWD_SIZE) == 0) {
            pszPasswd = szPassword;
        }
    }

    /* validate rsa key string */
    if (!strcasecmp(pszFmt, "der")) {
        if (validate_rsa_key_der((char *)pszKey, strlen(pszKey), 
                !strcasecmp(pszKeyType, "private"), pprsa, pszErr, err_bufsz) != 0) {
            goto err_out;
        }
    } else {
        if (validate_rsa_key_pem(pszPasswd, (char *)pszKey, strlen(pszKey), 
                !strcasecmp(pszKeyType, "private"), pprsa, pszErr, err_bufsz) != 0) {
            goto err_out;
        }
    }
    
	ret = 0;
	if (cfg) {
        cfg->pszFmt = (char *)pszFmt;
        cfg->pszKey = (char *)pszKey;
        cfg->pszKeyType = (char *)pszKeyType;
        cfg->pszPasswd = (char *)pszPasswd;
        cfg->pszEncPasswd = (char *)pszEncPasswd;
        snprintf(cfg->szPassword, PASSWD_SIZE, "%s", pszPasswd);
	}

err_out:
    return ret;
}

static int __clish_proc_rsakey(const char *keyname, 
    rsa_var_config_t *pCfg, int bAdd, RSA *pRsa)
{
    char *pszField = NULL;
    int nLen = 0;
    char szErr[256];

    if (NULL == g_rsa_key_buf || NULL == pRsa || NULL == pCfg || NULL == keyname) {
        assert(0);
        return -1;
    }

    pszField = g_rsa_key_buf + M_RSA_KEYSTR_BUFSZ;
    if (!strcasecmp(pCfg->pszKeyType, "public")) {
        if (rsa_pub_convert_format(pRsa, "pem", pszField, M_RSA_KEYSTR_BUFSZ, 
                szErr, 256, NULL) != 0) {
            fprintf(stderr, "%% Convert RSA to pem failed: %s\r\n", szErr);
            return -1;
        }
    } else {
        if (rsa_pri_convert_format(pRsa, "pem", NULL, pCfg->szPassword,
                pszField, M_RSA_KEYSTR_BUFSZ, szErr, 256, NULL) != 0) {
            fprintf(stderr, "%% Convert RSA to pem failed: %s\r\n", szErr);
            return -1;
        }

    }
    if (sal_b64_ntop((unsigned char *)pszField, strlen(pszField), 
            g_rsa_key_buf, M_RSA_KEYSTR_BUFSZ) == -1) {
        fprintf(stderr, "%% Encoding pem string failed\r\n");
        return -1;
    }

    nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen, 
                    "cdbctl %s/cdb/app/rsa/%s", 
                    bAdd ? "create" : "update", keyname);
    if (bAdd) {
        nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen, 
                        "/type/%s", pCfg->pszKeyType);
    }

    if ('\0' != pCfg->pszPasswd[0] && strcasecmp(pCfg->pszPasswd, "")) {
        nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen, 
                        "/passwd/%s", pCfg->pszPasswd);
    } else if ('\0' != pCfg->pszEncPasswd[0] && strcasecmp(pCfg->pszEncPasswd, "")) {
        nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen, 
                        "/enc_passwd/%s", pCfg->pszPasswd);
    }
    
    nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen, 
                        "/pem/%s", g_rsa_key_buf);

    g_rsa_key_buf[0] = '\0';
    if (sal_cmd_exec(g_rsa_key_buf, M_RSA_KEYSTR_BUFSZ, pszField) != 0) {
        if ('\0' != g_rsa_key_buf[0]) {
            fwrite(g_rsa_key_buf, strlen(g_rsa_key_buf), 1, stderr);
            fprintf(stderr, "\r\n");
        }
        return -1;
    }
    return 0;
}


static int clish_enter_rsakey_mode(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *keyname;
	char *pszField = NULL;
	int ret = -1;
	int bIsPub = 0;
	int cnt = lub_argv__get_count(argv);
	const char *pszCmd = NULL;
    RSA *pRsa = NULL;
    char szErr[256];
    rsa_var_config_t cfg;

	if (cnt < 3 || cnt > 5
	        || strcmp(lub_argv__get_arg(argv, 0), "rsa") 
	        || strcmp(lub_argv__get_arg(argv, 1), "key")) {
        fprintf(stderr, "%% clish_enter_rsakey_mode: Internal error\n");
        return -1;
	}

    keyname = lub_argv__get_arg(argv, 2);
    if (cnt >= 4) {
        pszCmd = lub_argv__get_arg(argv, 3);
    }
	if ((4 == cnt || 5 == cnt) && !strcmp(pszCmd, "generate")) {
	    /* generate rsa key */
	    
        lub_argv_t *myargv = lub_argv_new(NULL, 0);
        if (myargv) {
            lub_argv_add(myargv, keyname);
            if (4 == cnt) {
                lub_argv_add(myargv, "1024");            
            } else {
                lub_argv_add(myargv, lub_argv__get_arg(argv, 4));
            }
            ret = clish_generate_rsakey(context, myargv, script, out);
            lub_argv_delete(myargv);
        }
        
        return -1;
    }

    if (3 != cnt) {
        fprintf(stderr, "%% clish_enter_rsakey_mode: Internal error\n");
        return -1;
    }

    if (NULL == g_rsa_key_buf) {
        g_rsa_key_buf = malloc(M_RSA_KEYSTR_BUFSZ * 2);
        if (NULL == g_rsa_key_buf) {
            fprintf(stderr, "%% clish_enter_rsakey_mode: Out of memory\r\n");
            goto err_out;
        }
    }

    g_rsa_key_buf[0] = '\0';
    pszField = g_rsa_key_buf + M_RSA_KEYSTR_BUFSZ;
    if (sal_cmd_exec(g_rsa_key_buf, M_RSA_KEYSTR_BUFSZ, "cdbctl read/cdb/app/rsa/%s", keyname) == 0) {
        /* rsa key exist */
        if (sal_cmd_get_token(g_rsa_key_buf, "type", pszField, M_RSA_KEYSTR_BUFSZ) == -1) {
            fprintf(stderr, "%% clish_enter_rsakey_mode: can't get rsa key type\r\n");
            goto err_out;
        } else {
            __set_var(context, M_VAR_KEY_TYPE, pszField, 1);
        }    

        sal_cmd_get_token(g_rsa_key_buf, "passwd", pszField, M_RSA_KEYSTR_BUFSZ); /* XXX */
        if ('\0' != pszField[0]) {
            __set_var(context, M_VAR_KEY_PASSWORD, pszField, 1);
        } else {
            __set_var(context, M_VAR_KEY_PASSWORD, "", 1);
        }

        sal_cmd_get_token(g_rsa_key_buf, "enc_passwd", pszField, M_RSA_KEYSTR_BUFSZ); /* XXX */
        if ('\0' != pszField[0]) {
            __set_var(context, M_VAR_KEY_ENCPASSWORD, pszField, 1);
        } else {
            __set_var(context, M_VAR_KEY_ENCPASSWORD, "", 1);
        }

        sal_cmd_get_token(g_rsa_key_buf, "type", pszField, M_RSA_KEYSTR_BUFSZ); /* XXX */
        if ('\0' != pszField[0]) {
            __set_var(context, M_VAR_KEY_TYPE, pszField, 1);
            bIsPub = !strcasecmp(pszField, "public");
        } else {
            __set_var(context, M_VAR_KEY_TYPE, "public", 1);
            bIsPub = 1;
        }

        sal_cmd_get_token(g_rsa_key_buf, "pem", pszField, M_RSA_KEYSTR_BUFSZ); /* XXX */
        if ('\0' == pszField) {
            __set_var(context, M_VAR_KEY_STRING, "", 1);
            __set_var(context, M_VAR_KEY_FORMAT, "der", 1);
        } else {
            if (sal_b64_pton(pszField, (unsigned char *)g_rsa_key_buf, M_RSA_KEYSTR_BUFSZ) == -1) {
                fprintf(stderr, "%% Invalid RSA key pem string encoding\n");
                goto err_out;
            }
            __set_var(context, M_VAR_KEY_STRING, g_rsa_key_buf, 1);
            __set_var(context, M_VAR_KEY_FORMAT, "pem", 1);
            
            /* get RSA and convert to der-hex format */
            memset(&cfg, 0x00, sizeof(rsa_var_config_t));
            if (__validate_rsa_keystring(context, &cfg, &pRsa, szErr, 256) != 0) {
                fprintf(stderr, "%% Invalid RSA key pem string\n");
                goto err_out;
            }

            if (bIsPub) {
                if (rsa_pub_convert_format(pRsa, "der-hex", g_rsa_key_buf,
                        M_RSA_KEYSTR_BUFSZ, szErr, 256, NULL) != 0) {
                    fprintf(stderr, "%% Convert RSA to der-hex failed\n");
                    goto err_out;
                }
            } else {
                if (rsa_pri_convert_format(pRsa, "der-hex", 
                        NULL, cfg.szPassword, g_rsa_key_buf, 
                        M_RSA_KEYSTR_BUFSZ, szErr, 256, NULL) != 0) {
                    fprintf(stderr, "%% Convert RSA to der-hex failed\n");
                    goto err_out;
                }
            }
            __set_var(context, M_VAR_KEY_FORMAT, "der", 1);
            __set_var(context, M_VAR_KEY_STRING, g_rsa_key_buf, 1);
        }

        if (!isStartupCfg()) {
            printf("Modify %s key %s\n", bIsPub ? "public" : "private", keyname);
        }
    } else if (strstr(g_rsa_key_buf, "not found")) {
        /* check key count */
        if (check_key_limits() != 0) {
            goto err_out;
        }
        g_rsa_key_buf[0] = '\0';

        __set_var(context, M_VAR_KEY_PASSWORD, "", 1);
        __set_var(context, M_VAR_KEY_ENCPASSWORD, "", 1);
        __set_var(context, M_VAR_KEY_STRING, "", 1);
        __set_var(context, M_VAR_KEY_TYPE, "public", 1);
        __set_var(context, M_VAR_KEY_FORMAT, "der", 1);

        if (!isStartupCfg()) {
            printf("Create new key %s.\r\n", keyname);
            printf("Please specify key type and format. Default is public key of DER format\r\n");
        }
    } else {
        if (strlen(g_rsa_key_buf) > 0) {
            fwrite(g_rsa_key_buf, strlen(g_rsa_key_buf), 1, stderr);
            fprintf(stderr, "\r\n");
        } else {
            fprintf(stderr, "%% clish_enter_rsakey_mode: Internal error\n");
        }
        goto err_out;
    }

	ret = 0;

err_out:
    if (NULL != pRsa) {
        RSA_free(pRsa);
        pRsa = NULL;
    }
    return ret;
}

static int clish_leave_rsakey_mode(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *keyname = lub_argv__get_arg(argv, 0);
	const char *action = lub_argv__get_arg(argv, 1);
	rsa_var_config_t cfg;
	char *pszField = NULL;
	RSA *pRsa = NULL;
	char szErr[256];
	int ret = -1;
	int bAdd = 0;

    if (NULL == keyname || NULL == action || NULL == context || NULL == context->shell) {
        fprintf(stderr, "%% clish_leave_rsakey_mode: Internal error\n");
        return -1;
    }

    if (strcasecmp(action, "quit") && strcasecmp(action, "apply")) {
        fprintf(stderr, "%% Invalid action: %s. Should be quit or apply\r\n", action);
        return -1;
    }

    if (!strcasecmp(action, "quit")) {
        fprintf(stderr, "%% Process for RSA key '%s' being cancelled\r\n", keyname);
        free(g_rsa_key_buf);
        g_rsa_key_buf = NULL;
        return clish_nested_up(context, argv, NULL, NULL);
    }

    /* apply changes and leave mode */
    assert(NULL != g_rsa_key_buf);
    pszField = g_rsa_key_buf + M_RSA_KEYSTR_BUFSZ;
    pszField[0] = '\0';
    if ('\0' == g_rsa_key_buf[0]) {
        /* add new rsa key */
        bAdd = 1;
    }

    /* validate key string */
    memset(&cfg, 0x00, sizeof(rsa_var_config_t));
    if (__validate_rsa_keystring(context, &cfg, &pRsa, szErr, 256) != 0) {
        fprintf(stderr, "%% %s\r\n", szErr);
        goto err_out;
    }
    
	ret = __clish_proc_rsakey(keyname, &cfg, bAdd, pRsa);

err_out:
    if (!isStartupCfg()) {
        fprintf(stderr, "%% %s RSA key '%s' %s\n",
            bAdd ? "Add" : "Update", keyname,
            (0 == ret) ? "successed" : "failed");
    }

    if (NULL != pRsa) {
        RSA_free(pRsa);
        pRsa = NULL;
    }
    free(g_rsa_key_buf);
    g_rsa_key_buf = NULL;
    return clish_nested_up(context, argv, NULL, NULL);
}

static int clish_validate_rsakey(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
    char szErr[256];
    
    if (__validate_rsa_keystring(context, NULL, NULL, szErr, 256) != 0) {
        fprintf(stderr, "%% %s\r\n", szErr);
        return -1;
    }

    printf("%% Validated Ok\n") ;
    return 0;
}

static int clish_chvar(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *varname = lub_argv__get_arg(argv, 0);
	const char *action = lub_argv__get_arg(argv, 1);
	const char *value = lub_argv__get_arg(argv, 2);

    if (NULL == varname || NULL == action || NULL == value 
            || NULL == context || NULL == context->shell) {
        fprintf(stderr, "%% clish_chvar: Invalid parameters. Format: varname (replace|added) value\n");
        return -1;
    }

    if (!strcasecmp(varname, M_VAR_KEY_TYPE) && '\0' != g_rsa_key_buf[0]) {
        fprintf(stderr, "%% Can't change key type when updating key\n");
        return -1;
    }

    if (strcasecmp(action, "replace") && strcasecmp(action, "added")) {
        fprintf(stderr, "%% Invalid action: %s. Should be replace or added\r\n", action);
        return -1;
    }

    return __set_var(context, varname, value, !strcasecmp(action, "replace"));
}

#define M_IF_RANGE_PORT_NUMS    4096
#define M_IF_RANGE_SEGMENTS     32
#define CLISH_MAX_IF_NUM    4096
typedef char ifname_t[32];

typedef struct
{
    ifname_t ifname_array[CLISH_MAX_IF_NUM];
    int count;
} clish_if_info_t;

lub_argv_t *g_p_interface_list = NULL;
clish_if_info_t g_all_if_info;

static int clish_if_lookup(char *ifname)
{
    int i = 0;
    
    for (i = 0; i < g_all_if_info.count; i++)
    {
        if (0 == strcmp(ifname, g_all_if_info.ifname_array[i]))
        {
            return 0;
        }
    }
    
    return -1;
}

int clish_if_info_init(void)
{
    int str_len = 0;
    char line[1024];
    char ifname_cmd[256];
    FILE *fp = NULL;

    sal_memset(ifname_cmd, 0, sizeof(ifname_cmd));
    sal_sprintf(ifname_cmd, "cdbctl show/cdb/interface/key > /tmp/klish_ifname_check");

    g_all_if_info.count = 0;
    
    system(ifname_cmd);
    fp = fopen("/tmp/klish_ifname_check", "r");
    if (NULL == fp)
    {
        return -1;
    }

    while (NULL != sal_fgets(line, 1024, fp))
    {
        str_len = sal_strlen(line) - 1;
        if ('\n' == line[str_len])
        {
            line[str_len] = '\0';
        }
        sal_strncpy(g_all_if_info.ifname_array[g_all_if_info.count], line, 32);
        g_all_if_info.count++;
    }
    
    fclose(fp);
    return 0;
}

int clish_if_list_init(void)
{
    if (NULL != g_p_interface_list) {
        lub_argv_delete(g_p_interface_list);
    }
    g_p_interface_list = lub_argv_new(NULL, 0);
    if (NULL == g_p_interface_list) {
        fprintf(stderr, "%% Out of memory\r\n");
        return -1;
    }

    clish_if_info_init();

    return 0;
}

int clish_if_list_add(char *pszIntfName, char *pszErrMsg, size_t err_bufsz)
{
    int i;
    int cnt;

    if (clish_if_lookup(pszIntfName) != 0) {
        snprintf(pszErrMsg, err_bufsz, 
                "Command rejected: interface missing (%s)",
                pszIntfName);
        return -1;
    }

    if (NULL != g_p_interface_list) {
        cnt = lub_argv__get_count(g_p_interface_list);
        for (i = 0; i < cnt; i++) {
            if (!strcasecmp(pszIntfName, (char *)lub_argv__get_arg(g_p_interface_list, i))) {
                /* interface exist */
                return 0;
            }
        }
    
        if (cnt < M_IF_RANGE_PORT_NUMS) {
            lub_argv_add(g_p_interface_list, pszIntfName); /* XXX */
            return 0;
        }

        snprintf(pszErrMsg, err_bufsz, "Too many interfaces");
    } else {
        snprintf(pszErrMsg, err_bufsz, "Out of memory\r\n");
    }
    
    return -1;
}


static int clish_if_range_check(char *pszIntfName, int nEndNum, char *pszErrMsg, size_t err_bufsz)
{
    char szPrefix[IF_NAMESIZE];
    char szName[IF_NAMESIZE];
    int nStartNum;
    int find = 0;
    char *p;
    int i;

    if (strlen(pszIntfName) >= IF_NAMESIZE) {
        snprintf(pszErrMsg, err_bufsz, 
                "Command rejected: interface name too long (%s)",
                pszIntfName);
        return -1;
    }

    p = pszIntfName + strlen(pszIntfName) - 1;
    while (p > pszIntfName) {
        if (isdigit(*p)) {
            find = 1;
            p--;
        } else {
            break;
        }
    }

    if (!find || p <= pszIntfName) {
        snprintf(pszErrMsg, err_bufsz, 
                "Command rejected: interface missing (%s)",
                pszIntfName);
        return -1;
    }

    nStartNum = atoi(p + 1);
    snprintf(szPrefix, IF_NAMESIZE, "%s", pszIntfName);
    szPrefix[p - pszIntfName  + 1] = '\0';
    if (nEndNum <= nStartNum) {
        snprintf(pszErrMsg, err_bufsz, 
                "Invalid interface range string: %s - %d", 
                pszIntfName, nEndNum);
        return -1;
    }
    
    for (i = nStartNum; i <= nEndNum; i++) {
        snprintf(szName, IF_NAMESIZE, "%s%d", szPrefix, i);
        if (clish_if_list_add(szName, pszErrMsg, err_bufsz) != 0) {
            return -1;
        }
    }
    return 0;
}

static int clish_interface_range(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *intflist = lub_argv__get_line(argv);
    int ret = -1;
    char **ppSegArr = NULL;
    unsigned long nSegCnt = 0;
    char **ppIntfArr = NULL;
    unsigned long nIntfCnt;
    char szErrMsg[256];
    char szIntfName[IF_NAMESIZE];
    int nEndNum;
    int i;

    if (NULL == intflist || clish_if_list_init() != 0) {
        return -1;
    }

    /* split segments
     * format:    eth-0-1 - 10 , vlan300 - 400 , vlan 10 , eth-0-1
     */
    errno = 0;
    if (split_tokens((char *)intflist, strlen(intflist), M_IF_RANGE_SEGMENTS, ",", &nSegCnt, &ppSegArr) != 0) {
        if (EINVAL == errno) {
            fprintf(stderr, "%% Too many interface segments\r\n");
        } else {
            fprintf(stderr, "%% Out of memory\r\n");
        }
        goto err_out;
    }

    if (!nSegCnt) {
        fprintf(stderr, "%% Invalid interface range string: %s\r\n", intflist);
        goto err_out;
    }

    /* extract interface:
     * format: 
     *   A B - C eg: vlan 3 - 10
     *   A - B   eg: eth-0-1 - 3
     *   A B     eg: vlan 300
     *   A       eg: agg3
     */
    for (i = 0; i < nSegCnt; i++) {
        char *p = ppSegArr[i];

        if (NULL != ppIntfArr) {
            free_tokens(&ppIntfArr);
            ppIntfArr = NULL;
        }

        if (split_tokens(p, strlen(p), 4, " ", &nIntfCnt, &ppIntfArr) != 0) {
            if (EINVAL == errno) {
                fprintf(stderr, "%% Invalid interface range string: %s\r\n", p);
            } else {
                fprintf(stderr, "%% Out of memory\r\n");
            }
            goto err_out;
        } 

        if ((nIntfCnt % 2)) {
            snprintf(szIntfName, IF_NAMESIZE, "%s", ppIntfArr[0]);
        } else {
            /* fix bug 47229, not allowed eth-0-1 3 / vlan10 3 */
            if (2 == nIntfCnt) {
                if ((0 == strncmp(ppIntfArr[0], "eth-", 4)) || (0 != strcmp(ppIntfArr[0], "vlan"))) {
                    fprintf(stderr, "%% Invalid interface range string: %s\r\n", p);
                    goto err_out;
                }
            }
            snprintf(szIntfName, IF_NAMESIZE, "%s%s", ppIntfArr[0], ppIntfArr[1]);
        }

        if (1 == nIntfCnt || 2 == nIntfCnt) { 
            /* format: 
             *   agg3 or eth-0-1
             *   vlan 300
             */
            if (clish_if_list_add(szIntfName, szErrMsg, 256) != 0) {
                fprintf(stderr, "%% %s\r\n", szErrMsg);
                goto err_out;
            }
        } else if ((3 == nIntfCnt && !strcasecmp(ppIntfArr[1], "-"))
                    || (4 == nIntfCnt && !strcasecmp(ppIntfArr[2], "-"))) {
            if (strspn(ppIntfArr[nIntfCnt - 1], "0123456789") != strlen(ppIntfArr[nIntfCnt - 1])) {
                fprintf(stderr, "%% Invalid interface range string: %s\r\n", p);
                goto err_out;
            }
            nEndNum = atoi(ppIntfArr[nIntfCnt - 1]);
            if (clish_if_range_check(szIntfName, nEndNum, szErrMsg, 256) != 0) {
                fprintf(stderr, "%% %s\r\n", szErrMsg);
                goto err_out;
            }
        } else {
            fprintf(stderr, "%% Invalid interface range string: %s\r\n", p);
            goto err_out;
        }
        
    }
    
    ret = 0;
    
err_out:
    if (intflist) {
        lub_string_free((char *)intflist);
    }
    
    if (NULL != ppIntfArr) {
        free_tokens(&ppIntfArr);
    }    
    if (NULL != ppSegArr) {
        free_tokens(&ppSegArr);
    }    	
    return ret;
}



static void clish_genrsa_cb(int p, int n, void *cb)
{
    static int pos;

    sal_printf("%c\b", "|/-\\"[pos++ & 3]);
    fflush(stdout);
    return;
}

static int clish_generate_rsakey(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *keyname = lub_argv__get_arg(argv, 0);
	const char *bits = lub_argv__get_arg(argv, 1);
    char szErr[256];
    char *pszBuf = NULL;
    RSA *rsa = NULL;
    int ret = -1;
    int nbits;
    long l;
    int i;

    if (NULL == keyname || NULL == bits
            || NULL == context || NULL == context->shell) {
        return -1;
    }

    szErr[0] = '\0';
    if (cdb_check_name(keyname, M_RSA_KEY_NAMESZ, szErr) < 0) {
        fprintf(stderr, "%% %s\r\n", szErr);
        return -1;
    }

    nbits = atoi(bits);
    if (strspn(bits, "0123456789") != strlen(bits)
            || (nbits < M_RSA_IMPORT_KEY_MIN_MODULUS)
            || (nbits > M_RSA_IMPORT_KEY_MAX_MODULUS)) {
        fprintf(stderr, "%% Invalid bits: %s\r\n", bits);
        return -1;
    }

    if (check_key_limits() != 0) {
        return -1;
    }

    if (NULL == (pszBuf = malloc(M_RSA_KEYSTR_BUFSZ * 2))) {
        fprintf(stderr, "%% Out of memory\r\n");
        goto err_out;
    }
    if (sal_cmd_exec(pszBuf, M_RSA_KEYSTR_BUFSZ, 
                "cdbctl read/cdb/app/rsa/%s/type", keyname) == 0) {
        fprintf(stderr, "%% RSA key %s exist\r\n", keyname);
        goto err_out;
    }

    RAND_load_file("/dev/urandom", -1);
    sal_printf("Generating RSA private key, %d bit long modulus\n", nbits);
    sal_printf("Please waiting for a moment: ");
    if ((rsa = RSA_generate_key(nbits, RSA_F4, clish_genrsa_cb, NULL)) == NULL)
    {
        fprintf(stderr, "%% Genreate RSA private key failed\n");
        goto err_out;
    }
    sal_printf("done!\n");

    l = 0L;
    for (i = 0; i < rsa->e->top; i++) {
        l <<= BN_BITS4;
        l <<= BN_BITS4;
        l += rsa->e->d[i];
    }
    sal_printf("Public exponent is %ld (0x%lX)\n", l, l);

    if (rsa_pri_convert_format(rsa, "pem", NULL, NULL,
                pszBuf, M_RSA_KEYSTR_BUFSZ, szErr, 256, NULL) != 0) {
        fprintf(stderr, "%% Convert RSA to pem failed: %s\r\n", szErr);
        goto err_out;
    }

    if (sal_b64_ntop((unsigned char *)pszBuf, strlen(pszBuf), 
            pszBuf + M_RSA_KEYSTR_BUFSZ, M_RSA_KEYSTR_BUFSZ) == -1) {
        fprintf(stderr, "%% Encoding pem string failed\r\n");
        goto err_out;
    }
    if (sal_cmd_exec(pszBuf, M_RSA_KEYSTR_BUFSZ, 
                "cdbctl create/cdb/app/rsa/%s/type/private/pem/%s",
                keyname, pszBuf + M_RSA_KEYSTR_BUFSZ) != 0) {
        fwrite(pszBuf, strlen(pszBuf), 1, stderr);
        fprintf(stderr, "\r\n");
        goto err_out;
    }
    ret = 0;
    sal_printf("Generate RSA key successfully\n");
    
err_out:
    if (NULL != pszBuf) {
        free(pszBuf);
    }
    if (NULL != rsa) {
        RSA_free(rsa);
    }
    return ret;
}

int clish_term_winsize_get (int sock, u_int16_t *row, u_int16_t *col)
{
    struct winsize w;
    int ret;

    ret = ioctl (sock, TIOCGWINSZ, &w);
    if (ret < 0) {
        return -1;
    }

    *row = w.ws_row;
    *col = w.ws_col;
    return 0;
}

int clish_term_winsize_set (int sock, u_int16_t row, u_int16_t col)
{
    struct winsize w;
    int ret;

    w.ws_row = row;
    w.ws_col = col;

    ret = ioctl (sock, TIOCSWINSZ, &w);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

u_int16_t g_term_lines = 0xFFFF;
static u_int16_t g_term_lines_orig = 0xFFFF;

static void __clish_sigwinch (int sig)
{
    u_int16_t row;
    u_int16_t col;

    clish_term_winsize_get (0, &row, &col);
    if (row != g_term_lines)
    {
        g_term_lines_orig = row;
        clish_term_winsize_set (0, g_term_lines, col);
    }

    return;
}

static int clish_terminal_length(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
    const char *arg = lub_argv__get_arg(argv, 0);
    int nlines = atoi(arg);
    u_int16_t row;
    u_int16_t col;

    if (1 == nlines)
    {
        printf("%% Not support length 1\n");
        return -1;
    }
    
    clish_term_winsize_get (0, &row, &col);
    if (-1 == nlines) {
        if (0xFFFF != g_term_lines) {
            __signal_set (SIGWINCH, SIG_IGN);      
            clish_term_winsize_set (0, g_term_lines_orig, col);
            g_term_lines = 0xFFFF;
        }
        return 0;
    }
    
    if (nlines >= 0 || nlines <= 512) {
        g_term_lines = nlines;
        if (0xFFFF == g_term_lines_orig) {
            g_term_lines_orig = row;
        }
        clish_term_winsize_set (0, g_term_lines, col);
        __signal_set (SIGWINCH, __clish_sigwinch);
    }
    return 0;
}

static int clish_chdir(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *filename = lub_argv__get_arg(argv, 0);
    char szFullName[M_FULLPATH_MAX_LEN];   

    if (glb_is_mnt_valid)
    {
        gen_set_dir_mnt_valid(1);
    }
	/*
	 * Check file specified isdirectory 
	 */
    if (0 != gen_check_and_get_filename((char *)filename, szFullName, M_FULLPATH_MAX_LEN)) {
        if (glb_is_mnt_valid)
        {
            gen_set_dir_mnt_valid(0);
        }
        printf("%% Invalid diretory name: %s\n", filename);
        return -1;
    }
    if (glb_is_mnt_valid)
    {
        gen_set_dir_mnt_valid(0);
    }
	if (0 != chdir(szFullName)) {
        printf("%% Failed to cd %s: %s\n", filename, strerror(errno));
        return -1;
    }

	return 0;
}

static int clish_ctc_chdir_mnt(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
    char szFullName[M_FULLPATH_MAX_LEN];

    snprintf(szFullName, M_FULLPATH_MAX_LEN, "/mnt");
    if (0 != chdir(szFullName)) {
        printf("%% Failed to cd %s: %s\n", szFullName, strerror(errno));
        return -1;
    }

    return 0;
}

/*----------------------------------------------------------- */
/* Terminate the current shell session */
static int clish_close(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	/* the exception proves the rule... */
	clish_shell_t *this = (clish_shell_t *)context->shell;

	argv = argv; /* not used */
	this->state = SHELL_STATE_CLOSING;

    if (0xFFFF != g_term_lines_orig) {
        u_int16_t row;
        u_int16_t col;

        clish_term_winsize_get (0, &row, &col);
        __signal_set (SIGWINCH, SIG_IGN);      
        clish_term_winsize_set (0, g_term_lines_orig, col);
        g_term_lines_orig = 0xFFFF;
    }

	return 0;
}
/*----------------------------------------------------------- */
/* Enable the current shell session */
extern int isConsole(char *pszTty);
static int clish_enable(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
    char szMsg[M_FULLPATH_MAX_LEN] = {"Password:"};
    char g_buf[PASSWORD_LEN + 1];
    char szBuf[512];
    int count = 3;
    int nr = 0;
    int temp_timeout = DEFAULT_ENABLE_TIMEOUT;
    char szCmd[256];
    const char *pszPri = lub_argv__get_arg(argv, 0);
    int nPri = (NULL == pszPri) ? 4 : atoi(pszPri);
    int bEnc = 0;
    
    if (NULL == pszPri || strspn(pszPri, "0123456789") != strlen(pszPri)
            || nPri <= 0 || nPri > GLB_MAX_PRIVILEAGE_NUM) {
        printf("%% Internal error. Invalid privilege level\n");
        return -1;
    }

    if (nPri <= glb_pri) {
        /* enable privilege is smaller than current privilege, no password need */
        glb_pri = nPri;
        return 0;
    }

    snprintf(szCmd, 256, "cdbctl read/cdb/sys/enable/%d", (nPri - 1));
    if (sal_cmd_exec(szBuf, 512, szCmd) != 0) {
        printf("%% Internal error, access enable table failed\n");
        return -1;
    }

    if (sal_cmd_get_token(szBuf, "passwd", szCmd, 256) != 0) {
        printf("%% Internal error, access enable table failed\n");
        return -1;
    }
    if ('\0' == szCmd[0] || !strcasecmp(szCmd, "")) {
        if (sal_cmd_get_token(szBuf, "enc_passwd", szCmd, 256) != 0) {
            printf("%% Internal error, access enable table failed\n");
            return -1;
        }

        if ('\0' == szCmd[0] || !strcasecmp(szCmd, "")) {
            /* if no enable password is set for privilege max in console
             * cannot stop user to enter privilege exec mode
             */
            char *curtty = ttyname(STDIN_FILENO);
            if (4 == nPri && isConsole(curtty)) {
                glb_pri = nPri;
                return 0;
            }
            printf("%% No password has been assigned yet\n");
            return -1;
        }
        bEnc = 1;
    }

    for (count = 0; count < 3; count++) {
        if (glb_timeout > 0) {
            nr = cli_readpassphrase(szMsg, g_buf, PASSWORD_LEN, 
                    CLI_RPP_ECHO_OFF| CLI_RPP_WITH_TIMEOUT|CLI_RPP_LINE_MODE, glb_timeout);
        } else {
            nr = cli_readpassphrase(szMsg, g_buf, PASSWORD_LEN, 
                    CLI_RPP_ECHO_OFF| CLI_RPP_WITH_TIMEOUT, temp_timeout);
        }

        if (0 == clish_check_password((char *)g_buf, szCmd, bEnc)) {
            glb_pri = nPri;
            return 0;
        }
    }

    (void)nr;
    return -1;
}
/*----------------------------------------------------------- */
/* Disable the current shell session */
static int clish_disable(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
    if (glb_pri != 1) {
        glb_pri = 1;
    }
	return 0;
}

extern void log_init_line(const char *ip_addr, const char *usr_name);
/*----------------------------------------------------------- */
/* Disable the current shell session */
static int clish_rpcapi_user(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
    const char *ip_addr = lub_argv__get_arg(argv, 0);
    char *usr_name = NULL;
    
    usr_name = strstr(ip_addr, "/");
    if (usr_name)
    {
        usr_name[0] = '\0';
        usr_name++;
    }
    if (ip_addr && usr_name)
    {
        log_init_line(ip_addr, usr_name);
    }
    else
    {
        log_init_line(ip_addr, "");
    }
    
    return 0;
}

/*----------------------------------------------------------- */
/*
 Open a file and interpret it as a script in the context of a new
 thread. Whether the script continues after command, but not script, 
 errors depends on the value of the stop_on_error flag.
*/
static int clish_source_internal(clish_context_t *context,
	const lub_argv_t * argv, bool_t stop_on_error)
{
	int result = -1;
	const char *filename = lub_argv__get_arg(argv, 0);
	struct stat fileStat;

	/* the exception proves the rule... */
	clish_shell_t *this = (clish_shell_t *)context->shell;

	/*
	 * Check file specified is not a directory 
	 */
	if ((0 == stat((char *)filename, &fileStat)) &&
		(!S_ISDIR(fileStat.st_mode))) {
		/*
		 * push this file onto the file stack associated with this
		 * session. This will be closed by clish_shell_pop_file() 
		 * when it is finished with.
		 */
		result = clish_shell_push_file(this, filename,
			stop_on_error);
	}

	return result ? -1 : 0;
}

/*----------------------------------------------------------- */
/*
 Open a file and interpret it as a script in the context of a new
 thread. Invoking a script in this way will cause the script to
 stop on the first error
*/
static int clish_source(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	return (clish_source_internal(context, argv, BOOL_TRUE));
}

/*----------------------------------------------------------- */
/*
 Open a file and interpret it as a script in the context of a new
 thread. Invoking a script in this way will cause the script to
 continue after command, but not script, errors.
*/
static int clish_source_nostop(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	return (clish_source_internal(context, argv, BOOL_FALSE));
}

int g_trace_cmd = BOOL_FALSE;

/*----------------------------------------------------------- */
/*
 Show the shell overview
*/
static int clish_ctc_cfgtrace(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
    const char *arg = lub_argv__get_arg(argv, 0);
    if (!arg)
    {
        return 0;
    }
    if (!strcmp(arg, "on"))
    {
        g_trace_cmd = BOOL_TRUE;
    }
    else
    {
        g_trace_cmd = BOOL_FALSE;
    }
    return 0;
}

#define IMISH_RPP_ECHO_OFF          0x01    /* Turn off echo (default). */
#define IMISH_RPP_ECHO_ON           0x02    /* Leave echo on. */
#define IMISH_RPP_WITH_TIMEOUT      0x04    /* fail if timeout */
#define IMISH_RPP_CONFIRM_MODE      0x08    /* confirm mode: only accept 1 char */

#define CLISH_BANNER_BUFSIZ         1024
#define BANNER_MAX_LINES            99

static int g_sigint = 0; /* timeout flag */

int rl_sigint(void)
{
    return g_sigint;
}

void rl_set_sigint_flag(int bflag)
{
    g_sigint = bflag;
}

int
clish_unset_banner(char *filename)
{
    sal_unlink(filename);
    return 0;
}

int
clish_set_banner(char *input, char *filename)
{
    char endc;
    char buf[CLISH_BANNER_BUFSIZ+1];
    int nr = 0; /* modify by wulx for bug 39766, 2018-01-31 */
    int timeout = 30; // imish_get_timeout();
    
    char szTmpName[] = "/tmp/banner_tmp";
    FILE *fp = NULL;
    int line = 0;
    char *p = NULL;
    char *word = NULL;

    sal_unlink(szTmpName);
    fp = sal_fopen(szTmpName, "w+");
    if (!fp)
    {
        sal_printf("%% Open temporary file failed\n");
        return -1;
    }

    endc = input[0];
    if (sal_strlen(input) != 1)
    {
        word = input;
        word++;
        p = sal_strchr(word, endc);
        if (p == NULL)
        {
            sal_fprintf(fp, "%s\n", word);
        }
    }

    if (sal_strlen(input) == 1 || p == NULL)
    {
        sal_printf("Enter TEXT message.  End with the character '%c'.\n", endc);
    }

    if (p)
    {
        *p = '\0';
        sal_fprintf(fp, "%s\n", word);
    }
    else
    {
        do
        {
            nr = cli_readpassphrase(NULL, buf, CLISH_BANNER_BUFSIZ, CLI_RPP_ECHO_ON | CLI_RPP_WITH_TIMEOUT, timeout);
            if (nr < 0)
            {
                break;
            }
            p = sal_strchr(buf, endc);
            if (p)
            {
                *p = '\0';
                if (p != buf)
                {
                    sal_fprintf(fp, "%s\n", buf);
                }
                break;
            }

            sal_fprintf(fp, "%s\n", buf);
            line++;
        } while(line < BANNER_MAX_LINES);
    }

    sal_fclose(fp);

    /* add by wulx for bug 39766, 2017-11-25, if get SIGINT / SIGTSTP or timeout ..., then return */
    if (nr < 0)
    {
        if (rl_sigint())
        {
            rl_set_sigint_flag(0);
        }
        /* del tmp file */
        if (syscmd_file_exist(szTmpName))
        {
            if (sal_unlink(szTmpName))
            {
                sal_printf("%% Internal error.\n");
                return -1;
            }
        }

        return 0;
    }

    if (syscmd_file_exist(filename))
    {
        if (sal_unlink(filename))
        {
            sal_printf("%% Internal error.\n");
            return -1;
        }
    }

    if (sal_rename(szTmpName, filename))
    {
        sal_printf("%% Internal error: %s\n", sal_strerror(errno));
        return -1;
    }

    return 0;
}

int clish_banner_show(char *mode)
{
    char buf[CLISH_BANNER_BUFSIZ+1];
    FILE *fp = NULL;
    char *fname = NULL;

    if (0 == sal_strcmp(mode, "motd"))
    {
        fname = BANNER_MOTD_FILE;
    }
    else if (0 == sal_strcmp(mode, "exec"))
    {
        fname = BANNER_EXEC_FILE;
    }
    else if (0 == sal_strcmp(mode, "login"))
    {
        fname = BANNER_LOGIN_FILE;
    }
    else
    {
        return 0;
    }
    
    fp = sal_fopen(fname, "r");
    if (fp)
    {
        while (sal_fgets(buf, CLISH_BANNER_BUFSIZ, fp))
        {
            sal_printf("%s", buf);
        }
        sal_fclose(fp);
    }

    return 0;    
}

static int clish_banner(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{
	const char *mode = script;
	char *text = NULL;

    text = strchr(script, '/');
    if (text)
    {
        text[0] = '\0';
        text++;

        if (!sal_strcmp(mode, "exec"))
        {
            return clish_set_banner(text, BANNER_EXEC_FILE);
        }
        else if (!sal_strcmp(mode, "motd"))
        {
            return clish_set_banner(text, BANNER_MOTD_FILE);
        }
        else if (!sal_strcmp(mode, "login"))
        {
            return clish_set_banner(text, BANNER_LOGIN_FILE);
        }
    }
    else
    {
        if (!sal_strcmp(mode, "exec"))
        {
            return clish_unset_banner(BANNER_EXEC_FILE);
        }
        else if (!sal_strcmp(mode, "motd"))
        {
            return clish_unset_banner(BANNER_MOTD_FILE);
        }
        else if (!sal_strcmp(mode, "login"))
        {
            return clish_unset_banner(BANNER_LOGIN_FILE);
        }
    }

    return 0;
}

static int clish_ctc_get_privilege(clish_context_t *context, const lub_argv_t *argv,
    const char *script, char **out)
{    
    sal_printf("Current privilege level is %u\n", glb_pri);
    return 0;
}

/*----------------------------------------------------------- */
/*
 Show the shell overview
*/
static int clish_overview(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	clish_shell_t *this = context->shell;
	argv = argv; /* not used */

	tinyrl_printf(this->tinyrl, "%s\n", context->shell->overview);

	return 0;
}

/*----------------------------------------------------------- */
static int clish_history(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	clish_shell_t *this = context->shell;
	tinyrl_history_t *history = tinyrl__get_history(this->tinyrl);
	tinyrl_history_iterator_t iter;
	const tinyrl_history_entry_t *entry;
	unsigned limit = 0;
	const char *arg = lub_argv__get_arg(argv, 0);

	if (arg && ('\0' != *arg)) {
		limit = (unsigned)atoi(arg);
		if (0 == limit) {
			/* unlimit the history list */
			(void)tinyrl_history_unstifle(history);
		} else {
			/* limit the scope of the history list */
			tinyrl_history_stifle(history, limit);
		}
	}
	for (entry = tinyrl_history_getfirst(history, &iter);
		entry; entry = tinyrl_history_getnext(&iter)) {
		/* dump the details of this entry */
		tinyrl_printf(this->tinyrl,
			"%5d  %s\n",
			tinyrl_history_entry__get_index(entry),
			tinyrl_history_entry__get_line(entry));
	}
	return 0;
}

/*----------------------------------------------------------- */
/*
 * Searches for a builtin command to execute
 */
static clish_shell_builtin_fn_t *find_builtin_callback(const
	clish_shell_builtin_t * cmd_list, const char *name)
{
	const clish_shell_builtin_t *result;

	/* search a list of commands */
	for (result = cmd_list; result && result->name; result++) {
		if (0 == strcmp(name, result->name))
			break;
	}
	return (result && result->name) ? result->callback : NULL;
}

/*----------------------------------------------------------- */
void clish_shell_cleanup_script(void *script)
{
	/* simply release the memory */
	lub_string_free(script);
}

/*-------------------------------------------------------- */
static int clish_shell_lock(const char *lock_path)
{
	int i;
	int res;
	int lock_fd = -1;
	struct flock lock;

	if (!lock_path)
		return -1;
	lock_fd = open(lock_path, O_WRONLY | O_CREAT, 00644);
	if (-1 == lock_fd) {
		fprintf(stderr, "Can't open lockfile %s.\n", lock_path);
		return -1;
	}
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	for (i = 0; i < CLISH_LOCK_WAIT; i++) {
		res = fcntl(lock_fd, F_SETLK, &lock);
		if (res != -1)
			break;
		if (EINTR == errno)
			continue;
		if ((EAGAIN == errno) || (EACCES == errno)) {
			if (0 == i)
				fprintf(stderr,
					"Another user is running this command. Please wait...\n");
			sleep(1);
			continue;
		}
		break;
	}
	if (res == -1) {
		fprintf(stderr, "Can't get lock.\n");
		close(lock_fd);
		return -1;
	}
	return lock_fd;
}

/*-------------------------------------------------------- */
static void clish_shell_unlock(int lock_fd)
{
	struct flock lock;

	if (lock_fd == -1)
		return;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	fcntl(lock_fd, F_SETLK, &lock);
	close(lock_fd);
}

/*----------------------------------------------------------- */
int clish_shell_execute(clish_context_t *context, char **out)
{
	clish_shell_t *this = context->shell;
	const clish_command_t *cmd = context->cmd;
	clish_action_t *action;
	int result = 0;
    char *access = NULL;
    const char *name = NULL;
    int allowed = 1;
	char *lock_path = NULL;
    char szLockPath[PATH_MAX]; 
    char strLine[MAX_LINE_CHAR];
    uint32 mem_states = 0;
    FILE *p_file = NULL;
    uint32 state = 0;   /* modified by liwh for bug 51819, 2019-09-10 */

	int lock_fd = -1;
	//sigset_t old_sigs;
	struct sigaction old_sigint, old_sigquit, old_sighup;
	clish_view_t *cur_view = clish_shell__get_view(this);
	unsigned int saved_wdog_timeout = this->wdog_timeout;
    access = clish_command__get_access(cmd);
    name = clish_command__get_name(cmd);
    
    if (g_is_startup_cfg || g_is_show_diag)
    {
        glb_pri = 4;
    } else if (clish_command__get_lock(cmd)) {
        /* setting per command line lock path */
        char *pidx;
        snprintf(szLockPath, PATH_MAX, "/tmp/.clish.%s", name);
        pidx = strchr(szLockPath, ' ');
        if (NULL != pidx) {
            while ('\0' != *pidx) {
                if (' ' == *pidx) {
                    *pidx = '-';
                }
                pidx++;
            }
        }

        lock_path = szLockPath;
    }
    if (access) {
		allowed = 0;
		if (this->client_hooks->access_fn)
			allowed = this->client_hooks->access_fn(this, access);
	}
	if (!allowed)
    {
        printf("%% Authorization denied for command '%s',the privilege should not be less than %s\n", name , access);
        goto error;
    }
    
    /*added by yejl, read the system memory check state from the file*/
    p_file = fopen(GLB_MEMCHK_STATE_FILE, "r");
    if ((NULL == p_file) || (feof(p_file)))
    {
        /*if the p_file is NULL, continue*/
    }
    else
    {
        while (sal_fgets(strLine, MAX_LINE_CHAR, p_file) != NULL)
        {
            sal_sscanf(strLine, "%d", &mem_states);
        }
        fclose(p_file);
    }
    /*end*/

    /*added by chenyq for rpc mode cli disable some cli*/
    if (glb_is_rpc && cur_view && (0 == strncmp(clish_view__get_name(cur_view), "Exec",4)))
    {
        if ((!strncmp(name, "logout", 6)) || (!strncmp(name, "exit", 4)) 
            ||(! strncmp(name, "quit", 4))||(! strncmp(name, "disable", 7)))
        {
            printf("%% The RPC mode, this command is not allowed to configure in rpc vty\n");
            return -1;
        }
    }
/*added by yejl, check the system memory state, if the state is CRITICAL or ALERT, the process logic is follow*/
    if (mem_states == SYSLIMIT_MEMCHECK_STATE_CRITICAL)
    {
        if (mem_states &&(strncmp(name, "startup", 7) && strncmp(name, "reboot", 6) && strncmp(name, "reload", 6) 
            && strncmp(name, "end", 3) && strncmp(name, "logout", 6) && strncmp(name, "exit", 4) 
            && strncmp(name, "quit", 4) && strncmp(name, "memory-threshold", 16) && strncmp(name, "start shell", 11) 
            && strncmp(name, "delete", 6) && strncmp(name, "show memory-threshold", 21) 
            && strncmp(name, "show memory summary total", 25) && strncmp(name, "show version", 12)))
        {
            printf("%% The system memory check state is critical, this command is not allowed to configure\n");
            return -1;
        }
    }
    else if (mem_states == SYSLIMIT_MEMCHECK_STATE_ALERT)
    {
        if (mem_states && (strncmp(name, "reboot", 6) && strncmp(name, "reload", 6) && strncmp(name, "end", 3) 
            && strncmp(name, "exit", 4) && strncmp(name, "quit", 4) && strncmp(name, "memory-threshold", 16)
            && strncmp(name, "logout", 6)&& strncmp(name, "start shell", 11) && strncmp(name, "delete", 6) 
            && strncmp(name, "show memory-threshold", 21) && strncmp(name, "clear", 5) && strncmp(name, "interface", 9)
            && strncmp(name, "shutdown", 8) && strncmp(name, "configure ", 10) && strncmp(name, "no ", 3)
            && strncmp(name, "startup", 7) && strncmp(name, "show memory summary total", 25) && strncmp(name, "show version", 12)))
        {
            printf("%% The system memory check state is alert, this command is not allowed to configure\n");
            return -1;
        }
        if ((strncmp(name, "no shutdown", 11) == 0) || (strncmp(name, "no switchport", 13) == 0))
        {
            printf("%% The system memory check state is alert, this command is not allowed to configure\n");
            return -1;
        }
    }
    /*end*/

    /* modified by liwh for bug 51819, 2019-09-10 */
    p_file = sal_fopen(SYS_LOAD_STATE_FILE, "r");
    if (p_file)
    {
        sal_fgets(strLine, MAX_LINE_CHAR, p_file);
        sal_sscanf(strLine, "%u", &state);
        sal_fclose(p_file);
    }

    if (SYS_LOAD_CRITICAL == state)
    {
        if ((strncmp(name, "startup", 7) && strncmp(name, "reboot", 6) && strncmp(name, "reload", 6) 
            && strncmp(name, "end", 3) && strncmp(name, "logout", 6) && strncmp(name, "exit", 4) 
            && strncmp(name, "quit", 4) && strncmp(name, "delete", 6) && strncmp(name, "shutdown", 8) 
            && strncmp(name, "show memory summary", 18) && strncmp(name, "show version", 12)   
            && strncmp(name, "service", 7) && strncmp(name, "show cpu utilization", 20) 
            && strncmp(name, "start shell", 11) && strncmp(name, "ctc_shell", 9)
            && strncmp(name, "ovs-ofctl", 9) && strncmp(name, "control-plane", 13)
            && strncmp(name, "reason", 6) && strncmp(name, "class", 5)
            && strncmp(name, "total", 5) && strncmp(name, "interface", 9)
            && strncmp(name, "configure terminal", 18)  && strncmp(name, "mac learning", 12)))   
        {
            printf("%% Command is forbidden for system is busying now\n");
            return -1;
        }

        if ((strncmp(name, "no shutdown", 11) == 0) || (strncmp(name, "no switchport", 13) == 0))
        {
            printf("%% Command is forbidden for system is busying now\n");
            return -1;
        }
    }
    /* liwh end */
    
	assert(cmd);
	action = clish_command__get_action(cmd);

	/* Pre-change view if the command is from another depth/view */
	{
		clish_view_restore_t restore = clish_command__get_restore(cmd);
		if ((CLISH_RESTORE_VIEW == restore) &&
			(clish_command__get_pview(cmd) != cur_view)) {
			clish_view_t *view = clish_command__get_pview(cmd);
			clish_shell__set_pwd(this, NULL, view, NULL, context);
		} else if ((CLISH_RESTORE_DEPTH == restore) &&
			(clish_command__get_depth(cmd) < this->depth)) {
			this->depth = clish_command__get_depth(cmd);
		}
	}
	/* Lock the lockfile */
	if (lock_path && clish_command__get_lock(cmd)) {
		lock_fd = clish_shell_lock(lock_path);
		if (-1 == lock_fd) {
			result = -1;
			goto error; /* Can't set lock */
		}
	}

	/* Ignore and block SIGINT, SIGQUIT, SIGHUP */
	if (!clish_command__get_interrupt(cmd)) {
		struct sigaction sa;
		//sigset_t sigs;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = SIG_IGN;
		sigaction(SIGINT, &sa, &old_sigint);
		sigaction(SIGHUP, &sa, &old_sighup);

		sigaction(SIGQUIT, &sa, &old_sigquit);
#if 0
		sigemptyset(&sigs);
		sigaddset(&sigs, SIGINT);
		sigaddset(&sigs, SIGHUP);
		sigaddset(&sigs, SIGQUIT);
		sigprocmask(SIG_BLOCK, &sigs, &old_sigs);
#endif
	}

	/* Execute ACTION */
	result = clish_shell_exec_action(action, context, out);

	/* Restore SIGINT, SIGQUIT, SIGHUP */
	if (!clish_command__get_interrupt(cmd)) {
#if 0
		sigprocmask(SIG_SETMASK, &old_sigs, NULL);
#endif
		/* Is the signals delivery guaranteed here (before
		   sigaction restore) for previously blocked and
		   pending signals? The simple test is working well.
		   I don't want to use sigtimedwait() function bacause
		   it needs a realtime extensions. The sigpending() with
		   the sleep() is not nice too. Report bug if clish will
		   get the SIGINT after non-interruptable action.
		*/
		sigaction(SIGINT, &old_sigint, NULL);
	    sigaction(SIGHUP, &old_sighup, NULL);
		sigaction(SIGQUIT, &old_sigquit, NULL);
	}

	/* Call config callback */
	if (!result && this->client_hooks->config_fn)
		this->client_hooks->config_fn(context);

	/* Call logging callback */
	if (clish_shell__get_log(this) && this->client_hooks->log_fn) {
		char *full_line = clish_shell__get_full_line(context);
		this->client_hooks->log_fn(context, full_line, result);
		lub_string_free(full_line);
	}

	/* Unlock the lockfile */
	if (lock_fd != -1) {
		clish_shell_unlock(lock_fd);
        unlink(lock_path);
    }
	/* Move into the new view */
	if (!result) {
		clish_view_t *view = clish_command__get_view(cmd);
		/* Save the PWD */
		if (view) {
			char *line = clish_shell__get_line(context);
			clish_shell__set_pwd(this, line, view,
				clish_command__get_viewid(cmd), context);
			lub_string_free(line);
		}
	}

	/* Set appropriate timeout. Workaround: Don't turn on  watchdog
	on the "set watchdog <timeout>" command itself. */
	if (this->wdog_timeout && saved_wdog_timeout) {
		tinyrl__set_timeout(this->tinyrl, this->wdog_timeout);
		this->wdog_active = BOOL_TRUE;
		fprintf(stderr, "Warning: The watchdog is active. Timeout is %u "
			"seconds.\nWarning: Press any key to stop watchdog.\n",
			this->wdog_timeout);
	} else
		tinyrl__set_timeout(this->tinyrl, this->idle_timeout);

error:
	return result;
}

/* Added by kcao for bug 38824 */
int clish_shell_log(clish_context_t *context, int result)
{
#define M_MOD_LOG       1
#define E_WARNING       4
#define E_ERROR         3
#define E_NOTICE         5   /* modified by liwh for bug 52764, 2019-08-08 */

#define OPRT_SUCCESS    0
#define OPRT_FAIL       1

    const char *line = NULL;

    line = tinyrl__get_last_buffer(context->shell->tinyrl);
    if (NULL == line)
    {
        return 0;
    }

    /* modified by liwh for bug 40534, 2016-09-26 */
    clish_update_vty_cmd_buf((char*)line);
    /* liwh end */

    /* Merge centecOS bug 43435, show command also should do syslog */
#if 0
    if (0 == sal_strncmp(line, "show", 4))
    {
        return 0;
    }
    if (0 == sal_strncmp(line, "do show", 7))
    {
        return 0;
    }
#endif

    if (0 == result)
    {
        /* modified by liwh for bug 52764, 2019-08-08 */
        //log_oper(M_MOD_LOG, E_WARNING, glb_pri, OPRT_SUCCESS, line);
        log_oper(M_MOD_LOG, E_NOTICE, glb_pri, OPRT_SUCCESS, line);
        /* liwh end */
    }
    else                    
    {
        log_oper(M_MOD_LOG, E_ERROR, glb_pri, OPRT_FAIL, line);
    }    
    return 0;
}

/*----------------------------------------------------------- */
int clish_shell_exec_action(clish_action_t *action,
	clish_context_t *context, char **out)
{
	clish_shell_t *this = context->shell;
	int result = 0;
	const char *builtin;
	char *script;
	const char *postproc;
	char cli_name[4];

    /* XXX: bypass the klish cli execute processing, delegate to the ovs utilities to
       execute. */
    if (!strcasecmp(clish_command__get_name(context->cmd), "ovs-ofctl"))
    {
#if 1
        int len = 0;
        char command_with_abs_path[1024]; /* max shell command length busybox support is 1024B. */

        memset(&command_with_abs_path, '\0', sizeof(command_with_abs_path));

        /* Concat command with abs path for workaround sshd does not load /etc/profile properly. */
#ifdef _GLB_UML_SYSTEM_
        len = snprintf(command_with_abs_path, 1024, "%s", 
            "/centec_switch/sbin/");
#else
        len = snprintf(command_with_abs_path, 1024, "%s", 
            SWITCH_PREFIX"/usr/local/openvswitch/bin/");
#endif
        snprintf(command_with_abs_path + len, 1024 - len, "%s", 
            clish_parg__get_value(clish_pargv__get_parg(context->pargv, 0)));
#endif
        /* Use NULL as action to use default shebang line "!/bin/sh",
           and extract command line from the pargv. */
        result = this->client_hooks->script_fn(context, NULL, 
                        clish_parg__get_value(clish_pargv__get_parg(context->pargv, 0)), out);
        /* Added by kcao 2019-03-05 for bug 44281, log ovs cmd to log oper */
        clish_shell_log(context, result);
        return result;
    }
        
	builtin = clish_action__get_builtin(action);
	script = clish_shell_expand(clish_action__get_script(action), SHELL_VAR_ACTION, context);
	if (builtin) {
		clish_shell_builtin_fn_t *callback;
		lub_argv_t *argv = script ? lub_argv_new(script, 0) : NULL;
		result = -1;
		/* search for an internal command */
		callback = find_builtin_callback(clish_cmd_list, builtin);
		if (!callback) {
			/* search for a client command */
			callback = find_builtin_callback(
				this->client_hooks->cmd_list, builtin);
		}
		/* invoke the builtin callback */
		if (callback)
			result = callback(context, argv, script, out);
		if (argv)
			lub_argv_delete(argv);

        /* modified by liwh for bug 40534, 2016-09-26 */
        if (clish_command__get_name(context->cmd))
        {
            clish_update_vty_cmd_buf((char*)clish_command__get_name(context->cmd));
        }
        /* liwh end */
	} else if (script) {
        if (g_trace_cmd)
        {
            fprintf(stderr, "%s\n", script);
        }
		/* now get the client to interpret the resulting script */
		result = this->client_hooks->script_fn(context, action, script, out);
		clish_shell_log(context, result);
		if (0 == result) { /* execute postproc bultin function if needed */
    	    postproc = clish_action__get_postproc(action);
    	    if (NULL != postproc) {
    	        clish_shell_builtin_fn_t *cbf;

    	        cbf = find_builtin_callback(clish_cmd_list, postproc);
    	        if (cbf) {
    	            char *pline = clish_shell__get_line(context);
    	            lub_argv_t *argvf = pline ? lub_argv_new(pline, 0) : NULL;
    	            
    	            lub_string_free(pline);
                    result = cbf(context, argvf, postproc, out);
                    lub_argv_delete(argvf);
                }
    	    }
	    }
	} else {
        /* modified by liwh for bug 40534, 2016-09-26 */
        if (context->cmd)
        {
            sal_memset(cli_name, 0, 4);
            sal_memcpy(cli_name, (char*)clish_command__get_name(context->cmd), 4);
            if ((0 == sal_memcmp(cli_name, "quit", 4)) || (0 == sal_memcmp(cli_name, "exit", 4))
                || (0 == sal_memcmp(cli_name, "end", 3)))
            {
                clish_update_vty_cmd_buf(cli_name);
            }
        }
        /* liwh end */
    }

	lub_string_free(script);

	return result;
}

/*----------------------------------------------------------- */
/*
 * Find out the previous view in the stack and go to it
 */
static int clish_nested_up(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	clish_shell_t *this = context->shell;

	if (!this)
		return -1;

	argv = argv; /* not used */

	/* If depth=0 than exit */
	if (0 == this->depth) {
		this->state = SHELL_STATE_CLOSING;
		return 0;
	}
	this->depth--;

	return 0;
}

/*----------------------------------------------------------- */
/*
 * Builtin: NOP function
 */
static int clish_nop(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	return 0;
}

/*----------------------------------------------------------- */
/*
 * Builtin: Set watchdog timeout. The "0" to turn watchdog off.
 */
static int clish_wdog(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	const char *arg = lub_argv__get_arg(argv, 0);
	clish_shell_t *this = context->shell;

	/* Turn off watchdog if no args */
	if (!arg || ('\0' == *arg)) {
		this->wdog_timeout = 0;
		return 0;
	}

	this->wdog_timeout = (unsigned int)atoi(arg);

	return 0;
}

/*--------------------------------------------------------- */
/*
 * Get the ACTION context as a macros
 */
static int clish_macros(clish_context_t *context, const lub_argv_t *argv,
	const char *script, char **out)
{
	if (!script) /* Nothing to do */
		return 0;
	*out = lub_string_dup(script);
	return 0;
}

/*----------------------------------------------------------- */
const char *clish_shell__get_fifo(clish_shell_t * this)
{
	char *name;
	int res;

	if (this->fifo_name) {
		if (0 == access(this->fifo_name, R_OK | W_OK))
			return this->fifo_name;
		unlink(this->fifo_name);
		lub_string_free(this->fifo_name);
		this->fifo_name = NULL;
	}

	do {
		char template[] = "/tmp/klish.fifo.XXXXXX";
        int fd = mkstemp(template);
        if (-1 == fd) {
			return NULL;
        }
        unlink(name);
        close(fd);
        fd = -1;
        name = template;
        
		res = mkfifo(name, 0600);
		if (res == 0)
			this->fifo_name = lub_string_dup(name);
	} while ((res < 0) && (EEXIST == errno));

	return this->fifo_name;
}

/*--------------------------------------------------------- */
void *clish_shell__get_client_cookie(const clish_shell_t * this)
{
	return this->client_cookie;
}

/*-------------------------------------------------------- */
void clish_shell__set_log(clish_shell_t *this, bool_t log)
{
	assert(this);
	this->log = log;
}

/*-------------------------------------------------------- */
bool_t clish_shell__get_log(const clish_shell_t *this)
{
	assert(this);
	return this->log;
}

/*----------------------------------------------------------- */

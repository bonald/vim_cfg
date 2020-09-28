
#include "proto.h"
#include "cdbclient.h"
#include "gen/tbl_vty_define.h"
#include "rsa_derhex.h"

typedef struct {
    int et_minutes;
    int et_seconds; 
    int privilege;
    
} vty_cfg_t;

int32 
cdbctl_output_vty_config(FILE* cfg_fp, int nStart, int nEnd, char *pszLine)
{
    cfg_parse_get_info_t info;
    char *p;
    int nValue;
    int et_minutes;
    int et_seconds;

    memset(&info, 0x00, sizeof(cfg_parse_get_info_t));
    cfg_get_parse(pszLine, &info, 0);

    if (nStart == nEnd) {
        fprintf(cfg_fp, "line vty %d\n", nStart);
    } else {
        fprintf(cfg_fp, "line vty %d %d\n", nStart, nEnd);
    }

    et_minutes = atoi(info.value[TBL_VTY_FLD_TIMEOUT_MIN]);
    et_seconds = atoi(info.value[TBL_VTY_FLD_TIMEOUT_SEC]);
    if (!(M_EXEC_TIMEOUT_MIN_DEFAULT == et_minutes && 0 == et_seconds)) {
        fprintf(cfg_fp, " exec-timeout %d %d\n", et_minutes, et_seconds);
    }

    nValue = atoi(info.value[TBL_VTY_FLD_PRIVILEGE]);
    if (1 != nValue) {
        fprintf(cfg_fp, " privilege level %d\n", nValue);
    }

    p = info.value[TBL_VTY_FLD_PASSWD];
    if (NULL != p && '\0' != *p) {
        fprintf(cfg_fp, " line-password %s\n", p);
    } else {
        p = info.value[TBL_VTY_FLD_ENC_PASSWD];
        if (NULL != p && '\0' != *p) {
            fprintf(cfg_fp, " line-password 8 %s\n", p);
        } else {
            fprintf(cfg_fp, " no line-password\n");
        }
    }
    
    p = info.value[TBL_VTY_FLD_IPV4_ACL];
    if (NULL != p && '\0' != *p)
    {
        fprintf(cfg_fp, " ip access-class %s in\n", p);
    }

    p = info.value[TBL_VTY_FLD_IPV6_ACL];
    if (NULL != p && '\0' != *p)
    {
        fprintf(cfg_fp, " ipv6 access-class %s in\n", p);
    }
    
    p = info.value[TBL_VTY_FLD_AUTH_METHOD];
    if (NULL != p && '\0' != *p && strcasecmp(p, "default"))
    {
        fprintf(cfg_fp, " login authentication %s\n", p);
    }

    p = info.value[TBL_VTY_FLD_LOGIN];
    if (NULL != p && '\0' != *p)
    {
        if (0 == strcasecmp(p, "nologin"))
        {
            fprintf(cfg_fp, " no login\n");
        }
        else if (0 == strcasecmp(p, "login"))
        {
            fprintf(cfg_fp, " login\n");
        }
        else if (0 == strcasecmp(p, "local"))
        {
            fprintf(cfg_fp, " login local\n");
        }
    }
    
    p = info.value[TBL_VTY_FLD_AUTHOR_METHOD];
    if (NULL != p && '\0' != *p && strcasecmp(p, "default")) {
        fprintf(cfg_fp, " authorization exec %s\n", p);
    }

    p = info.value[TBL_VTY_FLD_ACCOUNT_METHOD];
    if (NULL != p && '\0' != *p && strcasecmp(p, "default")) {
        fprintf(cfg_fp, " accounting exec %s\n", p);
    }

    p = info.value[TBL_VTY_FLD_ACCOUNTCMD_METHOD];
    if (NULL != p && '\0' != *p && strcasecmp(p, "default")) {
        fprintf(cfg_fp, " accounting commands %s\n", p);
    }
    fprintf(cfg_fp, " !\n");
    return 0;
}

int cdbctl_get_rsa_key(const char *pszEncodedKeyStr, const char *pszPasswd, 
    int bPrivate, RSA **ppRsa, char *pszErr, size_t err_bufsz)
{
    int ret = -1;
    char *pszKey = NULL;

    pszKey = malloc(M_RSA_KEYSTR_BUFSZ);
    if (NULL == pszKey) {
        snprintf(pszErr, err_bufsz, "out of memory");
        return -1;
    }
    if (sal_b64_pton(pszEncodedKeyStr, (unsigned char *)pszKey, M_RSA_KEYSTR_BUFSZ) == -1) {
        snprintf(pszErr, err_bufsz, "Decoding RSA key string failed");
        goto err_out;
    }
    if (validate_rsa_key_pem(pszPasswd, pszKey, strlen(pszKey), 
            bPrivate, ppRsa, pszErr, err_bufsz) != 0) {
        snprintf(pszErr, err_bufsz, "Invalid PEM RSA key string");
        goto err_out;
    }

    ret = 0;
    
err_out:
    if (pszKey) {
        free(pszKey);
    }
    return ret;
}


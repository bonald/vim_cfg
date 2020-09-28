/****************************************************************************
* $Id$
*  Clish PAM support
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-28 14:19
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "sal.h"
#include "proto.h"
#include "cdb_const.h"
#include "cdb_ds.h"
#include "genlog.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#define CONNTRACK "nf_conntrack"

extern int
syscmd_file_exist(char *filename);
extern int clish_banner_show(char *mode);

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
static int g_vty_alloced = 0;
static char g_login_vtyid[VTY_BUF_SIZE];
static char g_login_key[VTY_BUF_SIZE] = {'\0' };
static char g_login_user[USERNAME_SIZE];
static char g_login_remote_ip[USERNAME_SIZE];
int g_nEnablePri = -1;
extern int glb_is_rpc ;

static struct pam_conv conv = {
    misc_conv,
    NULL
};

/* connection from mgmt-if
  * ip conntrack maybe timeout 
  */
int refineRemoteIP(char *pszRemoteIP, int nSrcPort, char *pszRet, size_t bufsz)
{
    char szBuf[256];
    char szIp[128];
    addr_ipv6_t addrv6;

    sal_memset(szBuf, 0, sizeof(szBuf));
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

int refineDestIP(char *pszDestIP, int nSrcPort, char *pszRet, size_t bufsz)
{
    char szBuf[256];
    char szIp[128];
    addr_ipv6_t addrv6;

    sal_memset(szBuf, 0, sizeof(szBuf));
    if (!strncmp(pszDestIP, "169.254.1.", strlen("169.254.1.")) 
            || !strncmp(pszDestIP, "169.254.2.", strlen("169.254.2."))
            || !strncmp(pszDestIP, "169.254.3.", strlen("169.254.3."))) {
        if (sal_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/" CONNTRACK
                " | "
                "grep \"sport=%d\" | awk '{print $8}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "dst=", 4)) {
                snprintf(pszRet, bufsz, "%s", szBuf + 4);
                return 0;
            }
        }
    }

    if (!strncmp(pszDestIP, "fec0:fffe:fffe::1:", strlen("fec0:fffe:fffe::1:")) 
            || !strncmp(pszDestIP, "fec0:fffe:fffe::2:", strlen("fec0:fffe:fffe::2:")))
    {
        if (sal_cmd_exec(szBuf, 256, 
                "ip netns exec mgmt cat /proc/net/nf_conntrack"
                " | grep ipv6 | grep \"sport=%d\" | awk '{print $8}'",
                nSrcPort) == 0) {
            if (!strncmp(szBuf, "dst=", 4))
            {
                inet_pton(AF_INET6, szBuf + 4, (void*)&addrv6);
                inet_ntop(AF_INET6, (void*)&addrv6, pszRet, bufsz);
                return 0;
            }
        }
    }
    
    snprintf(szIp, 128, pszDestIP);
    snprintf(pszRet, bufsz, szIp);
    return 0;
}
        
void loginout_audit(int mModule, int nServerity, char *pszMsg)
{
    char szRemoteIp[128];
    char *pszLoginType = NULL;
    char *p;

    if ('\0' == g_login_vtyid[0]) {
        /* for allocate vty failed */
        snprintf(g_login_vtyid, VTY_BUF_SIZE, "---NA");
    }

    szRemoteIp[0] = '\0';
    p = getenv("SSH_CONNECTION");
    if (NULL == p) {
        p = getenv("TELNET_CONNECTION");
        if (NULL != p) {
            pszLoginType = "telnet";
        }
    } else {
        pszLoginType = "SSH";
    }
    if (NULL != p) {
        snprintf(szRemoteIp, 64, "%s", p);        
        p = strchr(szRemoteIp, ' ');
        if (NULL != p) {
            *p = '\0';
        }
        /* refine remote ip for mgmt-if */
        refineRemoteIP(szRemoteIp, atoi(p + 1), szRemoteIp, 128);        
    }

    if ('\0' != g_login_user[0]) {
        if ('\0' != szRemoteIp[0]) {
            log_sys(mModule, nServerity, "%s, vty %s, user %s, location %s%s%s", 
                    pszMsg, ('c' == g_login_vtyid[0]) ? "console" : g_login_vtyid + 3,
                    g_login_user, szRemoteIp, 
                    (NULL != pszLoginType) ? ", by " : "",
                    (NULL != pszLoginType) ? pszLoginType : "");
        } else {
            log_sys(mModule, nServerity, "%s, vty %s, user %s%s%s",
                    pszMsg, ('c' == g_login_vtyid[0]) ? "console" : g_login_vtyid + 3,
                    g_login_user, 
                    (NULL != pszLoginType) ? ", by " : "",
                    (NULL != pszLoginType) ? pszLoginType : "");
        }
        
        return;
    } 

    if ('c' == g_login_vtyid[0]) {
        /* don't print console login/out message if no user */
        return;
    }
    
    if ('\0' != szRemoteIp[0]) {
        log_sys(mModule, nServerity, "%s, vty %s, location %s%s%s", 
                pszMsg, g_login_vtyid + 3, szRemoteIp, 
                    (NULL != pszLoginType) ? ", by " : "",
                    (NULL != pszLoginType) ? pszLoginType : "");
    } else {
        log_sys(mModule, nServerity, "%s, vty %s%s%s",
                pszMsg, g_login_vtyid + 3,
                (NULL != pszLoginType) ? ", by " : "",
                (NULL != pszLoginType) ? pszLoginType : "");
    }
    return;
}

/****************************************************************************
 *
* Function
* 
****************************************************************************/
int isConsole(char *pszTty)
{
    /* Added by kcao for bug 39143 2016-06-23 */
    if (NULL == pszTty)
    {
        return 0;
    }

    /* Modified by liuht for bug44331, 2017-0619 */
    if (strstr(pszTty, "console") || strstr(pszTty, "ttyS0") || strstr(pszTty, "ttyS1") 
        || strstr(pszTty, "ttyAMA0") || strstr(pszTty, "ttyAMA1") || strstr(pszTty, "ttyS000")) {
        return 1;
    }
    return 0;
}

static int _clish_extract_tty(char *pszTty, char *pszBuf, size_t bufsz)
{
    char *p;
    
    if (!strncasecmp(pszTty, "/dev/", 5)) {
        snprintf(pszBuf, bufsz, "%s", pszTty + 5);
    } else {
        snprintf(pszBuf, bufsz, "%s", pszTty);
    }
    p = pszBuf;
    while ('\0' != *p) {
        if ('/' == *p) {
            *p = '-';
        }
        p++;
    }
    
    return 0;
}

int clish_get_login_authen(char *pszAuthList, size_t nBufSz)
{
    char szAuthListName[128];
    char szBuf[256];
    char *p;
    int idx;

    /* check vty setting */
    if ('\0' == g_login_vtyid[0]) {
        fprintf(stderr, "%% Internal error, can't get login session\r\n");
        return -1;
    }

    if (!strcasecmp(g_login_vtyid, "con0")) {
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/console/login") != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }

        pszAuthList[0] = '\0';
        sal_cmd_get_token(szBuf, "login", pszAuthList, nBufSz);
        return 0;
    } else if (!strncasecmp(g_login_vtyid, "vty", 3)) {
        idx = atoi(g_login_vtyid + 3);
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/vty/%d/auth_method", idx) != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }
    } else {
        fprintf(stderr, "%% Internal error, Invalid login session: %s\r\n", g_login_vtyid);
        return -1;
    }

    p = strrchr(szBuf, '=');
    if (NULL == p) {
        return -1;
    }

    if ('\0' == *(p + 1)) {
        snprintf(szAuthListName, 128, "default");
    } else {
        snprintf(szAuthListName, 128, "%s", p + 1);
    }

    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/authen/%s", szAuthListName) != 0) {
        fprintf(stderr, "%% Internal error, can't get AAA authentication list configuration\r\n");
        return -1;
    }

    pszAuthList[0] = '\0';
    sal_cmd_get_token(szBuf, "methods", pszAuthList, nBufSz);
    return 0;
}

int clish_get_author(char *pszAuthList, size_t nBufSz)
{
    char szAuthListName[128];
    char szBuf[256];
    char *p;
    int idx;

    /* check vty setting */
    if ('\0' == g_login_vtyid[0]) {
        fprintf(stderr, "%% Internal error, can't get login session\r\n");
        return -1;
    }

    if (!strncasecmp(g_login_vtyid, "vty", 3)) {
        idx = atoi(g_login_vtyid + 3);
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/vty/%d/author_method", idx) != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }
    } else {
        fprintf(stderr, "%% Internal error, Invalid login session: %s\r\n", g_login_vtyid);
        return -1;
    }

    p = strrchr(szBuf, '=');
    if (NULL == p) {
        return -1;
    }

    if ('\0' == *(p + 1)) {
        snprintf(szAuthListName, 128, "default");
    } else {
        snprintf(szAuthListName, 128, "%s", p + 1);
    }

    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/author/%s", szAuthListName) != 0) {
        fprintf(stderr, "%% Internal error, can't get AAA authorization list configuration\r\n");
        return -1;
    }

    pszAuthList[0] = '\0';
    sal_cmd_get_token(szBuf, "methods", pszAuthList, nBufSz);
    return 0;
}

int clish_get_account(char *pszAccountList, size_t nBufSz)
{
    char szAccountListName[128];
    char szBuf[256];
    char *p;
    int idx;

    /* check vty setting */
    if ('\0' == g_login_vtyid[0]) {
        fprintf(stderr, "%% Internal error, can't get login session\r\n");
        return -1;
    }

    if (!strncasecmp(g_login_vtyid, "vty", 3)) {
        idx = atoi(g_login_vtyid + 3);
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/vty/%d/account_method", idx) != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }
    } else {
        fprintf(stderr, "%% Internal error, Invalid login session: %s\r\n", g_login_vtyid);
        return -1;
    }

    p = strrchr(szBuf, '=');
    if (NULL == p) {
        return -1;
    }

    if ('\0' == *(p + 1)) {
        snprintf(szAccountListName, 128, "default");
    } else {
        snprintf(szAccountListName, 128, "%s", p + 1);
    }

    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/account/%s", szAccountListName) != 0) {
        fprintf(stderr, "%% Internal error, can't get AAA authentication list configuration\r\n");
        return -1;
    }

    pszAccountList[0] = '\0';

    sal_cmd_get_token(szBuf, "start_stop", pszAccountList, nBufSz);
        
    if (0 == sal_memcmp(pszAccountList, "1", sal_strlen("1")))
    {
        sal_cmd_get_token(szBuf, "methods", pszAccountList, nBufSz);
    }
    
    return 0;
}

/* return -1: deny
 *        0: permit
 */
int clish_acl_check(char *szTty)
{
    char szBuf[1024] = {0};
    int ret  = 0;
    
    if (sal_cmd_exec(szBuf, 32, "cdbctl read/cdb/app/line_acl_check/%s", szTty) != 0) {
        fprintf(stderr, "%% Internal error, can't execute acl check\r\n");
        return -1;
    }
    if (strstr(szBuf, "deny"))
    {
        return -1;
    }
    return ret;
}

/* return 1: need auth
 *        0: no need auth
 *       -1: error
 */
int clish_get_auth_cfg(char *pszLoginSess, int *pnAAA, int *pnLoginMethod)
{
    char szBuf[256];
    char *p;
    int idx;

    *pnAAA = 0;
    *pnLoginMethod = LOGIN_METHOD_NOLOGIN;

    /* check aaa_new_model setting */
    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/sys/sys_global/aaa_new_model") != 0) {
        fprintf(stderr, "%% Internal error, can't get aaa status\r\n");
        return -1;
    }

    p = strrchr(szBuf, '=');
    if (NULL != p && '\0' != *(p + 1)) {
        if (atoi(p + 1) == 1) {
            *pnAAA = 1;
            return 1;
        }
    }

    /* check vty setting */
    if ('\0' == g_login_vtyid[0]) {
        fprintf(stderr, "%% Internal error, can't get login session\r\n");
        return -1;
    }

    if (!strcasecmp(g_login_vtyid, "con0")) {
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/console/login") != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }
    } else if (!strncasecmp(g_login_vtyid, "vty", 3)) {
        idx = atoi(g_login_vtyid + 3);
        if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/vty/%d/login", idx) != 0) {
            fprintf(stderr, "%% Internal error, can't get vty configuration\r\n");
            return -1;
        }
    } else {
        fprintf(stderr, "%% Internal error, Invalid login session: %s\r\n", g_login_vtyid);
        return -1;
    }

    p = strrchr(szBuf, '=');
    if (NULL != p && '\0' != *(p + 1)) {
        idx = cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, p + 1);
        if (-1 == idx) {
            fprintf(stderr, "%% Internal error, Invalid line login method: %s\r\n", p + 1);
            return -1;
        }

        *pnLoginMethod = idx;
        switch (idx) {
        case LOGIN_METHOD_LOCAL:
        case LOGIN_METHOD_LOGIN:
            return 1;
        case LOGIN_METHOD_NOLOGIN:
        case LOGIN_METHOD_MAX:
            break;
        }
    } 
    
    return 0;
}

int clish_update_vty_cmd_buf(char *cmd_buf)
{
    int ret;
    char szBuf[1024];
    char szAccountListName[128];
    char pszAccountList[128];
    char *p;

    if(!syscmd_file_exist("/tmp/aaa_new_model"))
    {
        return 0;
    }

    if (!strcasecmp(g_login_vtyid, "con0")
        || !strncasecmp(cmd_buf, "startup", 7)) {
        return 0;
    } else {
        ret = sal_cmd_exec(szBuf, 1024, "cdbctl read/cdb/app/vty/%d/accountcmd_method",
                           atoi(g_login_vtyid + 3));
    }

    p = strrchr(szBuf, '=');
    if (NULL == p) {
        return -1;
    }

    if ('\0' == *(p + 1)) {
        snprintf(szAccountListName, 128, "default");
    } else {
        snprintf(szAccountListName, 128, "%s", p + 1);
    }

    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/accountcmd/%s", szAccountListName) != 0) {
        return 0;
    }

    pszAccountList[0] = '\0';

    sal_cmd_get_token(szBuf, "methods", pszAccountList, 128);
        
    if (!strncasecmp(pszAccountList, "tacplus", 7)) {

        ret = sal_cmd_exec(szBuf, 1024, "cdbctl update/cdb/app/vty/%d/cmd_buf/\"%s\"",
                               atoi(g_login_vtyid + 3), cmd_buf);

        if (strncasecmp(pszAccountList, "none", 4)) {
            if (0 != ret) {
                fprintf(stderr, "%% Internal error, can't update cmd_buf for %s\r\n", g_login_vtyid);
                return -1;
            } 
        }

        
    }

    return 0;
}

int clish_get_vty_config(int *pnPrivilege, int *pnTimeout)
{
    int ret;
    char szBuf[512];
    char aaaBuf[256];
    char szUserBuf[512];
    char *p;
    char *q;
    char *v;
    int et_minutes = 10;
    int et_seconds = 0;
    int aaa_privilege = 0;
    int AAA_status = 0;

    if (!strcasecmp(g_login_vtyid, "con0")) {
        ret = sal_cmd_exec(szBuf, 512, "cdbctl read/cdb/app/console");
    } else {
        ret = sal_cmd_exec(szBuf, 512, "cdbctl read/cdb/app/vty/%d",
                           atoi(g_login_vtyid + 3));
    }
    if (0 != ret) {
        fprintf(stderr, "%% Internal error, can't get privilege of %s\r\n", g_login_vtyid);
        return -1;
    } 

    /* get vty's privilege */
    p = strstr(szBuf, "privilege=");
    if (p) {
        ret = atoi(p + strlen("privilege="));
        *pnPrivilege = ret;
    }

    /* get vty's privilege */
    p = strstr(szBuf, "aaa_privilege=");
    if (p) {
        ret = atoi(p + strlen("aaa_privilege="));
        aaa_privilege = ret;
    }

    /*added by yejl to fix bug41212 and 41238, 2016-12-22*/
    /* check aaa_new_model setting */
    if (sal_cmd_exec(aaaBuf, 256, "cdbctl read/cdb/sys/sys_global/aaa_new_model") != 0) {
        fprintf(stderr, "%% Internal error, can't get aaa status\r\n");
        return -1;
    }
    v = strrchr(aaaBuf, '=');
    if (NULL != v && '\0' != *(v + 1)) 
    {
        if (atoi(v + 1) == 1) 
        {
            AAA_status = 1;
        }
        else
        {
            AAA_status = 0;
        }
    }

     /* use user's privilege if have */
    if ((0 != strcmp(g_login_user,"")) && (0 == aaa_privilege))
    {
        if (AAA_status == 0)
        {
            ret = sal_cmd_exec(szUserBuf, 512, "cdbctl read/cdb/app/user/%s", g_login_user);
            if (0 != ret) 
            {
                return -1;
            } 
            q = strstr(szUserBuf, "privilege=");
            if (q) 
            {
                ret = atoi(q + strlen("privilege="));
                *pnPrivilege = ret;         
            }
        }
    }
    else if ((-1 != g_nEnablePri) && (0 == aaa_privilege))
    {
        if (AAA_status == 0)
        {
            /* use enable privilege if login by enable password (level 1 ~ 4) */
            *pnPrivilege= g_nEnablePri;
        }
    }
    /*end*/

    p = strstr(szBuf, "timeout_min=");
    if (p) {
        et_minutes = atoi(p + strlen("timeout_min="));
    }
    p = strstr(szBuf, "timeout_sec=");
    if (p) {
        et_seconds = atoi(p + strlen("timeout_min="));
    }

    *pnTimeout = et_minutes * 60 + et_seconds;
    return 0;
}

/* return 0 for success */
int clish_auth(char *pszTty)
{
    pam_handle_t *pamh = NULL;
    int aaa_status = 0;
    int login_method = 0;
    char *pszUser = NULL;
    char *pszShorTty;
    int result = -1;
    int count = 3;
    char szTty[64];
    char szEnv[128];
    char szAuthList[128];
    char szAuthorList[128];
    char szAccountList[128];
    int ret;
    char *p;
    sal_memset(szTty, 0, 64);
    sal_memset(szEnv, 0, 128);
    sal_memset(szAuthList, 0, 128);
    sal_memset(szAuthorList, 0, 128);
    sal_memset(szAccountList, 0, 128);

    /*modify by zhaoqd for bug 53693 2019-09-23*/
    if(glb_is_rpc)/*add by chenyq for bug 43254*/
    {
        return 0;
    }
    
    /*modify by zhaoqd for bug 53532 2019-09-20*/
    _clish_extract_tty(pszTty, szTty, 64);
    
    if (clish_acl_check(szTty))
    {
        printf("%% Connection is refused by remote administrator!\n");
        return -2;
    }
    
    if (getenv("SSH_CONNECTION") != NULL) {
        return 0;
    }

    /*if (!strcasecmp(g_login_vtyid, "con0")) {
        return 0;
    }*/

    if (clish_get_auth_cfg(szTty, &aaa_status, &login_method) != 1) {
        return 0;
    }
    
    if (1 == aaa_status) {
        /* get authentication list for AAA */
        if (clish_get_login_authen(szAuthList, 128) != 0) {
            printf("%% Authentication list unspecified error");
            return -1;
        }
    }

    if (!strcasecmp(g_login_vtyid, "con0")) {
        if (!strcasecmp(szAuthList, "nologin"))
        {       
            return 0;
        }
    }

    if ((1 == aaa_status) && strcasecmp(g_login_vtyid, "con0")) {
        /* get authorization list for AAA */
        if (clish_get_author(szAuthorList, 128) != 0) {
            printf("%% Authorization list unspecified error");
            return -1;
        }
    }

    if ((1 == aaa_status) && strcasecmp(g_login_vtyid, "con0")) {
        /* get accounting list for AAA */
        if (clish_get_account(szAccountList, 128) != 0) {
            printf("%% Accounting list unspecified error");
            return -1;
        }
    }

    if (!strncasecmp(pszTty, "/dev/", 5)) {
        pszShorTty = pszTty + 5;
    } else {
        pszShorTty = pszTty;
    }

    clish_banner_show("login");
    
    do {
        ret = pam_start("clish", pszUser, &conv, &pamh);
        if (PAM_SUCCESS != ret) {
            printf("%% pam_start failed\r\n");
            return -1;
        }

        pam_set_item(pamh, PAM_USER_PROMPT, "Username: ");
        if ('\0' != g_login_user[0]) {
            pam_set_item(pamh, PAM_USER, g_login_user);
        }
        
        /* set TTY (so things like securetty work) */
        ret = pam_set_item(pamh, PAM_TTY, pszShorTty);
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }

        if (aaa_status) {
            snprintf(szEnv, 64, "aaa_status=%d", aaa_status);
            ret = pam_putenv(pamh, szEnv);
            if (PAM_SUCCESS != ret) {
                goto auth_failed;
            }
            snprintf(szEnv, 64, "login_method=%s", szAuthList);
            ret = pam_putenv(pamh, szEnv);
            if (PAM_SUCCESS != ret) {
                goto auth_failed;
            }
        } else {
            snprintf(szEnv, 64, "login_method=%d", login_method);
            ret = pam_putenv(pamh, szEnv);
            if (PAM_SUCCESS != ret) {
                goto auth_failed;
            }
        }

        snprintf(szEnv, 64, "login_vtyid=%s", g_login_vtyid);
        ret = pam_putenv(pamh, szEnv);
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }
        snprintf(szEnv, 64, "login_session=%s", szTty);
        ret = pam_putenv(pamh, szEnv);
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }

        if ((0 == sal_memcmp(szAuthorList, "0", sal_strlen("0")))
            || (!strcasecmp(g_login_vtyid, "con0")))
        {
            snprintf(szEnv, 64, "author_method=%s", "none");
        }
        else
        {
            snprintf(szEnv, 64, "author_method=%s", szAuthorList);
        }
        ret = pam_putenv(pamh, szEnv);
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }

        if ((0 == sal_memcmp(szAccountList, "0", sal_strlen("0")))
            || (!strcasecmp(g_login_vtyid, "con0")))
        {
            snprintf(szEnv, 64, "account_method=%s", "none");
        }
        else
        {
            snprintf(szEnv, 64, "account_method=%s", szAccountList);
        }
        ret = pam_putenv(pamh, szEnv);
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }

        ret = pam_authenticate(pamh, 0); /* is user really user ? */
        if (PAM_SUCCESS != ret) {
            goto auth_failed;
        }
        
        ret = pam_acct_mgmt(pamh, 0); /* permitted access ? */
        if (PAM_SUCCESS == ret) {
            const char *pszUsername = NULL;
            const char *pszEnabelPri;
            if (PAM_SUCCESS == pam_get_item(pamh, PAM_USER, (const void **)&pszUsername)
                    && NULL != pszUsername) {
                snprintf(g_login_user, USERNAME_SIZE, "%s", pszUsername);
            }
            g_nEnablePri = -1;
            pszEnabelPri = (char *)pam_getenv(pamh, "LOGINBYENABLEPRI");
            if (NULL != pszEnabelPri) {
                g_nEnablePri = atoi(pszEnabelPri);
            }
            pam_end(pamh, ret); /* close Linux-PAM */
            result = 0;
            break;
        }

auth_failed:
        p = (char *)pam_getenv(pamh, "ERRINFO");
        if (NULL != p) {
            printf("%% %s (%d)\r\n", p, ret);
        } else {
            printf("%% %s (%d)\r\n", pam_strerror(pamh, ret), ret);
        }
        pam_end(pamh, ret); /* close Linux-PAM */
        --count;
    } while (count > 0);

        
    return (result == 0) ? 0 : -1;
}

void clish_del_login_sess(void)
{
    /* we close the session by socket close signal
     * please reference clish_connect_login and login_session_del_bypid
     */
#if 0 
    char szCmd[512];
    
    if ('\0' != g_login_key[0]) {
        snprintf(szCmd, 512, "cdbctl delete/cdb/app/login/%s", g_login_key);
        system (szCmd);
    }
#endif
    return;
}

int clish_alloc_vty(char *pszTty)
{
    char szLogname[64];
    char szRemoteIp[64];
    char szCmd[256];
    char szBuf[256];
    char szTty[64];
    char szDestIp[64] = {0};
    char szRemotePort[64] = {0};
    char szDestPort[64] = {0};
    char *p;
    char p_copy[256] = {0};
    int isSsh = 0;
    int isTelnet = 0;

    g_login_vtyid[0] = '\0';
    g_login_user[0] = '\0';
    _clish_extract_tty(pszTty, szTty, 64);
    snprintf(g_login_key, VTY_BUF_SIZE, szTty);
    
    szLogname[0] = '\0';
    szRemoteIp[0] = '\0';
    g_login_remote_ip[0] = '\0';
    p = getenv("SSH_CONNECTION");
    if (NULL == p) {
        p = getenv("TELNET_CONNECTION");
        if (NULL != p) {
            isTelnet = 1;
        }
    } else {
        isSsh = 1;
    }
    if (NULL != p) {
#if 0
        snprintf(szRemoteIp, 64, "%s", p);
        p = strchr(szRemoteIp, ' ');
        if (NULL != p) {
            *p = '\0';
        }
#endif
        strcpy(p_copy, p);
        strcpy(szRemoteIp, strtok(p_copy, " "));
        strcpy(szRemotePort, strtok(NULL, " "));
        strcpy(szDestIp, strtok(NULL, " "));
        strcpy(szDestPort, strtok(NULL, " "));

        /* refine remote ip and dest ip for mgmt-if */
        refineRemoteIP(szRemoteIp, atoi(szRemotePort), szRemoteIp, 64);
        refineDestIP(szDestIp, atoi(szRemotePort), szDestIp, 64);
    }

    if (isSsh) {
        p = getenv("LOGNAME");
        if (NULL != p) {
            snprintf(szLogname, 64, "%s", p);
        }
    }
    
    szBuf[0] = '\0';
    if (sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/login/%s", szTty) == 0) {
        /* login session exist */
        if (sal_cmd_get_token(szBuf, "pid", szCmd, 256) == 0) {
            if (kill(atoi(szCmd), 0) == 0) {
                fprintf(stderr, "%% Please exit to CLI\r\n");
                exit (1);
            }
        }
        snprintf(szCmd, 256, "cdbctl update/cdb/app/login/%s/pid/%u", szTty, getpid());
    } else {
        snprintf(szCmd, 256, "cdbctl create/cdb/app/login/%s/pid/%u", szTty, getpid());
    } 
    
    if ('\0' != szRemoteIp[0]) {
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/ipaddr/%s", szRemoteIp);
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/dest_ipaddr/%s", szDestIp);
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/remote_port/%s", szRemotePort);
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/dest_port/%s", szDestPort);
        strncpy(g_login_remote_ip, szRemoteIp, USERNAME_SIZE);
    } else if (!isConsole(pszTty)) { /*add by chenyq for bug 43218*/
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/ipaddr/%s", "Local");
        strncpy(g_login_remote_ip, "Local", USERNAME_SIZE);
    } else {
        strncpy(g_login_remote_ip, "127.0.0.1", USERNAME_SIZE);
    }
    
    if ('\0' != szLogname[0]) {
        snprintf(g_login_user, USERNAME_SIZE, "%s", szLogname);
    }
    if (isSsh) {
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/protocol/%s", "ssh");
    } else if (isTelnet) {
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/protocol/%s", "telnet");
    } else if (glb_is_rpc){/*add by chenyq for bug 43218*/
        snprintf(szCmd + strlen(szCmd), 256 - strlen(szCmd), "/protocol/%s", "rpc");
    }


    szBuf[0] = '\0';
    if (sal_cmd_exec(szBuf, 256, "%s", szCmd) != 0) {
        fprintf(stderr, "%s\r\n", szBuf);
        loginout_audit(M_MOD_APP, E_ERROR, "login failed (too many users)");
        return -1;
    }
    g_vty_alloced = 1;

    sal_cmd_exec(szBuf, 256, "cdbctl read/cdb/app/login/%s/vty", szTty);
    p = strrchr(szBuf, '=');
    if (p && '\0' != *(p + 1)) {
        snprintf(g_login_vtyid, VTY_BUF_SIZE, "%s", p + 1);
    }
    return 0;
}

int clish_session_update_login_user(void)
{
    char szBuf[256];

    if ('\0' == g_login_key[0]) {
        return -1;
    }

    if ('\0' != g_login_user[0]) {
        char szEncUser[USERNAME_SIZE * 2];
        sal_b64_ntop((unsigned char *)g_login_user, strlen(g_login_user), 
                     szEncUser, USERNAME_SIZE * 2);
        sal_cmd_exec(szBuf, 256, "cdbctl update/cdb/app/login/%s/user/%s", 
                     g_login_key, szEncUser);
    }

    /* generate login accomplished record */
    sal_cmd_exec(szBuf, 256, "cdbctl update/cdb/app/login/%s/login", g_login_key);

    log_init_line(g_login_remote_ip, g_login_user);
    return 0;
}


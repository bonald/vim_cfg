/****************************************************************************
* $Id$
*  pam module for centec switch
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-10-28 15:45
* Reason        : First Create.
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <crypt.h>
#include <ctype.h>
#include <pwd.h>

#include "pam_ctc.h"
#include "sal.h"
#include "rsa_derhex.h"

#ifdef _GLB_UML_SYSTEM_
#define M_CDBCTL_PROG    "/centec_switch/sbin/cdbctl"
#elif defined _CTC_X86_64_
#define M_CDBCTL_PROG    "/switch/usr/sbin/cdbctl"
#elif defined HAVE_DEBIAN
#define M_CDBCTL_PROG    "/switch/usr/sbin/cdbctl"
#else
#define M_CDBCTL_PROG    "cdbctl"
#endif

#ifndef USERNAME_SIZE
#define USERNAME_SIZE       64
#endif

#ifndef PASSWD_SIZE
#define PASSWD_SIZE         256
#endif

/*
 * here, we make a definition for the externally accessible function
 * in this file (this definition is required for static a module
 * but strongly encouraged generally) it is used to instruct the
 * modules include file to define the function prototypes.
 */

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT

#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/_pam_macros.h>

static const char *aaa_status = NULL;
static const char *login_method = NULL;
static const char *login_vtyid = NULL;
static const char *login_session = NULL;
static const char *author_method = NULL;
static const char *account_method = NULL;
static char login_password[PASSWD_SIZE * 2];
static char szVty[32];
char is_ssh_connect = 0;
int do_aaa_auth_local(pam_handle_t * pamh);
int do_aaa_auth_line(pam_handle_t * pamh);

#ifdef HAVE_DEBIAN
void import_debian_path(void)
{
    char temp_path[1024] = {0};
    strcpy(temp_path, "/switch/sbin:/switch/usr/sbin:/switch/bin:/switch/usr/bin:/switch/usr/local/openvswitch/bin:/sbin:/usr/sbin:/bin:/usr/bin");
    setenv("PATH", temp_path, 1);

    char temp_ld_library_path[1024] = {0};
#if (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    strcpy(temp_ld_library_path, "/lib/aarch64-linux-gnu:/usr/lib/aarch64-linux-gnu:/switch/lib:/switch/usr/lib");
#else
    strcpy(temp_ld_library_path, "/lib/powerpc-linux-gnu:/usr/lib/powerpc-linux-gnu:/switch/lib:/switch/usr/lib");
#endif
    setenv("LD_LIBRARY_PATH", temp_ld_library_path, 1);

    const char *ovs_dir = getenv("OVS_DIR");
    if (!ovs_dir)
    {
        putenv("OVS_DIR=/usr/local/openvswitch");
    }
}
#endif

#if 0
static int pam_check_password(char *pin)
{
    char *p = pin;
    
    if (NULL == pin) {
        return -1;
    }
    
    if (strlen(pin) >= 64) {
        return -1;
    }

    /* check the chars */
    while ('\0' != *p) {
        if ((isdigit(*p)) 
            || (isupper(*p))
            || (islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)) {
            p++;
            continue;
        }

        return -1;
    }
    return 0;
}
#endif

static int32
pam_check_username(char *username)
{
    char *p = username;

    if (NULL == p || !isalpha(*p)) {
        return -1;
    }

    if (strlen(username) > 31) {
        return -1;
    }
    
    /* check the chars */
    while ('\0' != *p) {
        if ((isdigit(*p)) 
            || (isupper(*p))
            || (islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)) {
            p++;
            continue;
        }

        return -1;
    }

    return 0;
}

static int _putenv(pam_handle_t * pamh, const char *pszCmdFmt, ...)
{
    va_list ap;
    char szEnv[256];

    va_start(ap, pszCmdFmt);
    vsnprintf(szEnv, 256, pszCmdFmt, ap);
    va_end(ap);

    return pam_putenv(pamh, szEnv);
}

/*
 * Conversation function to obtain the user's password
 */
static int obtain_authtok(pam_handle_t * pamh)
{
	char *resp;
	const void *item;
	int retval;

	retval = pam_prompt(pamh, PAM_PROMPT_ECHO_OFF, &resp, "Password: ");
	if (retval != PAM_SUCCESS)
		return retval;

	if (resp == NULL)
		return PAM_CONV_ERR;

	/* set the auth token */
	retval = pam_set_item(pamh, PAM_AUTHTOK, resp);

	/* clean it up */
	_pam_overwrite(resp);
	_pam_drop(resp);

	if ((retval != PAM_SUCCESS) ||
		(retval = pam_get_item(pamh, PAM_AUTHTOK, &item))
		!= PAM_SUCCESS) {
		return retval;
	}

	return retval;
}

static int
_pam_parse(pam_handle_t * pamh, int argc, const char **argv)
{
	int ctrl;

	/* step through arguments */
	for (ctrl = 0; argc-- > 0; ++argv) {
		/* generic options */

		if (!strcmp(*argv, "debug")) {
			ctrl |= PAM_DEBUG_ARG;
		} else if (!strcasecmp(*argv, "icase")) {
			ctrl |= PAM_ICASE_ARG;
		} else if (!strcasecmp(*argv, "dump")) {
			ctrl |= PAM_DUMP_ARG;
		} else if (!strcasecmp(*argv, "unknown_ok")) {
			ctrl |= PAM_UNKNOWN_OK_ARG;
		} else if (!strcasecmp(*argv, "use_first_pass")) {
			ctrl |= PAM_USE_FPASS_ARG;
		} else if (!strcasecmp(*argv, "try_first_pass")) {
			ctrl |= PAM_TRY_FPASS_ARG;
		} else {
			pam_syslog(pamh, LOG_ERR, "unknown option: %s", *argv);
		}
	}

	return ctrl;
}

/* return values:
 * 0:  okay
 * -1: invalid password
 * -2: No password has been assigned yet
 */
static int validate_password(pam_handle_t *pamh, char *pszBuf, 
    const char *pass, const char *pkey)
{
    char szTmp[512];
    int bEnc = 0;

    if (NULL == pass || '\0' == pass[0]) {
        _putenv(pamh, "ERRINFO=Invalid password");
        return -1;
    }

    if (sal_cmd_get_token(pszBuf, (char *)pkey, szTmp, 512) != 0) {
        _putenv(pamh, "ERRINFO=Internal error, access enable password failed");
        return -1;
    }

    if ('\0' == szTmp[0] || !strcasecmp(szTmp, "")) {
        if (sal_cmd_get_token(pszBuf, "enc_passwd", szTmp, 512) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, access enable password failed");
            return -1;
        }
        if ('\0' == szTmp[0] || !strcasecmp(szTmp, "")) {
            _putenv(pamh, "ERRINFO=No password has been assigned yet");
            return -2;
        }
        bEnc = 1;
    }

    if (bEnc) {
        if (gen_passwd_validate(szTmp, (char *)pass) != 1) {
            _putenv(pamh, "ERRINFO=Access denied (incorrect password)");	
            return -1;
        }
    } else {
        if (strcmp(pass, szTmp)) {
            _putenv(pamh, "ERRINFO=Access denied (incorrect password)");	
            return -1;
        }
    }
 
    return 0;
}

int get_auth_method(pam_handle_t *pamh)
{
	aaa_status = pam_getenv(pamh, "aaa_status");
	login_method = pam_getenv(pamh, "login_method");
	login_vtyid = pam_getenv(pamh, "login_vtyid");
	login_session = pam_getenv(pamh, "login_session");
    author_method = pam_getenv(pamh, "author_method");
    account_method = pam_getenv(pamh, "account_method");

    if (NULL == login_vtyid || NULL == login_session) {
        _putenv(pamh, "ERRINFO=Can not get login session id");
		return PAM_SERVICE_ERR;
    }

    if (NULL == aaa_status && NULL == login_method) {
        _putenv(pamh, "ERRINFO=Unknown authentication method");
		return PAM_SERVICE_ERR;
    }
    
	if (NULL != aaa_status && !strcasecmp(aaa_status, "1")) {
		return PAM_SUCCESS;
	} 

	if (NULL != login_method) {
	    if (!strcasecmp(login_method, "0")) { /* no login */
            _putenv(pamh, "ERRINFO=Invalid login method: no login");
		    return PAM_SERVICE_ERR;
	    }
	}

    if (NULL != author_method) {
        if (!strcasecmp(author_method, "0")) { /* no author */
            _putenv(pamh, "ERRINFO=Invalid author method: no author");
            return PAM_SERVICE_ERR;
        }
    }

    if (NULL != account_method) {
        if (!strcasecmp(account_method, "0")) { /* no account */
            _putenv(pamh, "ERRINFO=Invalid account method: no account");
            return PAM_SERVICE_ERR;
        }
    }

    return PAM_SUCCESS;
}

/* modified by liwh for bug 38380, 2016-06-13 */
//int _get_username(pam_handle_t * pamh, char **ppUsername)
int _get_username(pam_handle_t * pamh, char **ppUsername, bool check_username)
/* liwh end */
{
    int ret;
    const char *pUsername = NULL;
    
	/* Get the username */
	ret = pam_get_user(pamh, &pUsername, NULL);
	if ((ret != PAM_SUCCESS) || (!pUsername)) {
		_putenv(pamh, "ERRINFO=Please input user name");
		return PAM_SERVICE_ERR;
	}

    /* modified by liwh for bug 38380, 2016-06-13 */
    if (check_username)
    /* liwh end */
    {
        if (pam_check_username((char *)pUsername) != 0) {
            _putenv(pamh, "ERRINFO=Invalid user name");
            return PAM_AUTH_ERR;
        }
    }
    /* modified by liwh for bug 38380, 2016-06-13 */
    else
    {
        if (strlen((char *)pUsername) > 31) {
            _putenv(pamh, "ERRINFO=Invalid user name");
            return PAM_AUTH_ERR;
        }
    }
    /* liwh end */

    *ppUsername = (char *)pUsername;
	return PAM_SUCCESS;
}

/* modified by liwh for bug 38380, 2016-06-13 */
//int _get_password(pam_handle_t * pamh, char **ppPasswd)
int _get_password(pam_handle_t * pamh, char **ppPasswd, bool check_password)
/* liwh end */
{
    int retval;
    const void *password = NULL;
    
	/* Check if we got a password */
	retval = pam_get_item(pamh, PAM_AUTHTOK, &password);
	if (retval != PAM_SUCCESS || NULL == password || '\0' == ((char *)password)[0]) {
		/* Converse to obtain a password */   
		retval = obtain_authtok(pamh);
		if (retval != PAM_SUCCESS) {
			_putenv(pamh, "ERRINFO=Can not obtain password from user");
			return retval;
		}
		retval = pam_get_item(pamh, PAM_AUTHTOK, &password);
		if (retval != PAM_SUCCESS || NULL == password || '\0' == ((char *)password)[0]) {
			_putenv(pamh, "ERRINFO=Can not get user password");
			return PAM_AUTH_ERR;
		}
	}

#if 0
    /* modified by liwh for bug 38380, 2016-06-13 */
    if (check_password)
    /* liwh end */
    {
        if (pam_check_password((char *)password) != 0) {
                _putenv(pamh, "ERRINFO=Invalid password string");
                return PAM_AUTH_ERR;
        }
    }
    /* modified by liwh for bug 38380, 2016-06-13 */
    else
#endif
    {
        if (strlen((char *)password) >= 64) {
                _putenv(pamh, "ERRINFO=Invalid password string");
                return PAM_AUTH_ERR;
        }
    }
    /* liwh end */

    *ppPasswd = (char *)password;
	return PAM_SUCCESS;
}

/* only check password for privilege 4 */
int do_aaa_auth_enable(pam_handle_t * pamh)
{
    char *pszPassword = NULL;
    char szBuf[1024];
    char szCmd[256];
    int i;

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* read enable password from user */
    if (_get_password(pamh, &pszPassword, TRUE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    
    for (i = 1; i <= 4; i++) {
        snprintf(szCmd, 256, "cdbctl read/cdb/sys/enable/%d", (i - 1));
        if (sal_cmd_exec(szBuf, 512, szCmd) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, get enable password failed");
            return PAM_AUTH_ERR;
        }
        /*modified by yejl to fix bug41235, 2016-12-22*/
        if (i == 4)
        {
            if (validate_password(pamh, szBuf, pszPassword, "passwd") == 0) {
            _putenv(pamh, "LOGINBYENABLEPRI=%d", i);
            return PAM_SUCCESS;
            }
        }
        /*end*/
    }
    
    return PAM_AUTH_ERR;
}

int do_aaa_auth_line(pam_handle_t * pamh)
{
    char *pszPassword = NULL;
    char szBuf[1024];

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* read line password from user */
    if (_get_password(pamh, &pszPassword, TRUE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    /* check line password */
    if (!strcasecmp(login_vtyid, "con0")) {
        if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/console", M_CDBCTL_PROG) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, can't get vty configuration");
            return PAM_AUTH_ERR;
        }
    } else {
        if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/vty/%s", 
                    M_CDBCTL_PROG, login_vtyid + 3) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, can't get vty configuration");
            return PAM_AUTH_ERR;
        }
    }

    if (validate_password(pamh, szBuf, pszPassword, "passwd") == 0) {
        return PAM_SUCCESS;
    }
    
    return PAM_AUTH_ERR;
}

int pam_get_service_type(char *service_type, int8 *pam_service_type)
{
    if (NULL == service_type || NULL == pam_service_type || 0 == sal_strlen(service_type))
    {
        return PAM_SUCCESS;
    }

    if (!sal_strncmp(service_type, "telnet", sal_strlen(service_type)))
    {
       *pam_service_type =  PAM_SERVICE_TYPE_TELNET;
    }
    if (!sal_strncmp(service_type, "ssh", sal_strlen(service_type)))
    {
        *pam_service_type =  PAM_SERVICE_TYPE_SSH;
    }
    if (!sal_strncmp(service_type, "web", sal_strlen(service_type)))
    {
        *pam_service_type =  PAM_SERVICE_TYPE_WEB;
    }
    if (!sal_strncmp(service_type, "rpc-api", sal_strlen(service_type)))
    {
        *pam_service_type =  PAM_SERVICE_TYPE_RPC_API;
    }
    return PAM_SUCCESS;
}


#if 0  /* add by wus for bug52967 modify password "admin" in 2019-07-15*/
#define CONFIG_MOD_PASSWORD

#ifdef CONFIG_MOD_PASSWORD
int do_local_change_password(pam_handle_t * pamh, char *new_password)
{
        char *resp;
        const void *item;
        int retval;
        char old_password[PASSWD_SIZE];
        char new_password1[PASSWD_SIZE];
        char new_password2[PASSWD_SIZE];     
        
        /*1.old password check*/
        retval = pam_prompt(pamh, PAM_PROMPT_ECHO_OFF, &resp, "Current password: ");
        if (retval != PAM_SUCCESS)
            return retval;
    
        if (resp == NULL)
            return PAM_CONV_ERR;
        /* set the auth token */
        retval = pam_set_item(pamh, PAM_AUTHTOK, resp);
    
        /* clean it up */
        _pam_overwrite(resp);
        _pam_drop(resp);
    
        if ((retval != PAM_SUCCESS) ||
            (retval = pam_get_item(pamh, PAM_AUTHTOK, &item)) != PAM_SUCCESS) 
        {
            return retval;
        }
        else
        {
            pam_get_item(pamh, PAM_AUTHTOK, &item);

            if (NULL != item && '\0' != ((char *)item)[0])
            {           
                if (strcmp(item, "admin"))
                {                
                    sal_printf("%% Old password is wrong!\n");
                    return PAM_AUTH_ERR;
                }
                else
                {
                    strcpy(old_password, item);
                }
            }
            else
            {
                sal_printf("%% Old password is wrong!\n");
                return PAM_AUTH_ERR;
            }
            
        }

        /*2.new password first enter*/    
        retval = pam_prompt(pamh, PAM_PROMPT_ECHO_OFF, &resp, "Enter new password: ");
        if (retval != PAM_SUCCESS)
            return retval;
    
        if (resp == NULL)
            return PAM_CONV_ERR;
    
        /* set the auth token */
        retval = pam_set_item(pamh, PAM_AUTHTOK, resp);
    
        /* clean it up */
        _pam_overwrite(resp);
        _pam_drop(resp);
    
        if ((retval != PAM_SUCCESS) ||
            (retval = pam_get_item(pamh, PAM_AUTHTOK, &item)) != PAM_SUCCESS) 
        {
            return retval;
        }
        else
        {
            pam_get_item(pamh, PAM_AUTHTOK, &item);
            if (NULL != item && '\0' != ((char *)item)[0])
            {
                strcpy(new_password1, item);
            }
            else
            {
                sal_printf("Invalid password string !\n");
                return PAM_AUTH_ERR;
            }

            if (strlen(new_password1) >= 64) 
            {
                sal_printf("Invalid password string !\n");
                return PAM_AUTH_ERR;
            }           
        }

        /*3. retype new password*/    
        retval = pam_prompt(pamh, PAM_PROMPT_ECHO_OFF, &resp, "Retype new password: ");
        if (retval != PAM_SUCCESS)
            return retval;
    
        if (resp == NULL)
            return PAM_CONV_ERR;
    
        /* set the auth token */
        retval = pam_set_item(pamh, PAM_AUTHTOK, resp);
    
        /* clean it up */
        _pam_overwrite(resp);
        _pam_drop(resp);
    
        if ((retval != PAM_SUCCESS) ||
            (retval = pam_get_item(pamh, PAM_AUTHTOK, &item)) != PAM_SUCCESS) 
        {
            return retval;
        }
        else
        {
            pam_get_item(pamh, PAM_AUTHTOK, &item);
            if (NULL != item && '\0' != ((char *)item)[0])
            {
                strcpy(new_password2, item);
            }
            else
            {
                sal_printf("Invalid password string !\n");
                return PAM_AUTH_ERR;
            }
            
             if (strlen(new_password2) >= 64) 
             {
                sal_printf("Invalid password string !\n");
                return PAM_AUTH_ERR;
             }
        }
              
        if (!strcmp(old_password, new_password1))
        {
            sal_printf("%% New password can not be the same to old one !\n");
            return PAM_AUTH_ERR;
        }
        else if (strcmp(new_password1, new_password2))
        {
            sal_printf("%% Two new passwords you entered did not match !\n");
            return PAM_AUTH_ERR;
        }

        if (new_password2[0] != '\0')
        {
              strcpy(new_password, new_password2);
        } 
        
        return retval;

}

int do_aaa_auth_local_change_password(pam_handle_t * pamh)
{
    char *pszUser = NULL;
    char *pszPassword = NULL;
    char pszPasswordcpy[PASSWD_SIZE];
    char szBuf[1024];
    //char szEncUser[USERNAME_SIZE * 2];
    char *p = NULL;
    int i = 0;
    int retval = 0;
    char new_password[PASSWD_SIZE];
    
#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, TRUE) != PAM_SUCCESS)
    {
        return PAM_AUTH_ERR;
    }

    /* read password from user */
    if (_get_password(pamh, &pszPassword, TRUE) != PAM_SUCCESS) 
    {
        sal_strcpy(pszPasswordcpy,"none-password");
        pszPassword = pszPasswordcpy;
    }
    else
    {       
            /* Check if we got a "admin" password */
            if (NULL != pszPassword)
            {
                if (!strcmp(pszPassword, "admin") && !strcmp(pszUser, "admin"))
                {
                    sal_printf("You are required to change your password immediately !\n");
                    sal_printf("Changing password for admin...\n");
                    
                    retval = do_local_change_password(pamh, new_password);
                    
                    if (PAM_SUCCESS != retval) 
                    {
                        pam_end(pamh, retval); 
                        exit(0);
                    }
                    if (NULL != new_password && new_password[0] != '\0')
                    {
                         pszPassword = new_password;
                    }
                }    
            }
                           
            p = pszPassword;
            i = 0;
            while ('\0' != *p)
            {
                if (('<' == *p)
                    || ('>' == *p)
                    || (',' == *p)
                    || ('(' == *p)
                    || (')' == *p))
                {
                    pszPasswordcpy[i] = '\\';
                    i++;
                    pszPasswordcpy[i] = *p;
                    i++;
                    p++;
                }
                else
                { 
                    pszPasswordcpy[i] = *p;
                    i++;
                    p++;
                }
            }

            pszPasswordcpy[i] = '\0';
            pszPassword = pszPasswordcpy;
        }

        sal_cmd_exec(szBuf, 512, "%s update/cdb/app/user/%s/passwd/%s", M_CDBCTL_PROG, pszUser, pszPassword);
        
         return PAM_SUCCESS;
}
#endif /* config_mod_password end*/
#endif /* wus end*/

int do_aaa_auth_local(pam_handle_t * pamh)
{
    char *pszUser = NULL;
    char *pszPassword = NULL;
    char pszPasswordcpy[PASSWD_SIZE];
    char szBuf[1024];
    char szEncUser[USERNAME_SIZE * 2];
    char *p = NULL;
    int i = 0;
#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, TRUE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    /* read password from user */
    if (_get_password(pamh, &pszPassword, TRUE) != PAM_SUCCESS) {
        sal_strcpy(pszPasswordcpy,"none-password");
        pszPassword = pszPasswordcpy;
    }
    else
    {       
        p = pszPassword;
        i = 0;
        while ('\0' != *p)
        {
            /*add &|; by wus for bug 52555 in 2019-11-01*/
            if (('<' == *p)
                || ('>' == *p)
                || (',' == *p)
                || ('(' == *p)
                || (')' == *p)
                || ('|' == *p)
                || ('&' == *p)
                || (';' == *p))
            {
                pszPasswordcpy[i] = '\\';
                i++;
                pszPasswordcpy[i] = *p;
                i++;
                p++;
            }
            else
            { 
                pszPasswordcpy[i] = *p;
                i++;
                p++;
            }
        }

        pszPasswordcpy[i] = '\0';
        pszPassword = pszPasswordcpy;
    }
    
    if (!sal_strncmp(login_session, "ttyS", 4))
    {
        if (sal_cmd_exec(szBuf, 512, "%s update/cdb/app/user/%s/user_password_check/%d/%s", M_CDBCTL_PROG, pszUser, is_ssh_connect, pszPassword))
        {
            if (strstr(szBuf, "not found")) {
                _putenv(pamh, "ERRINFO=Access denied (no such user)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "not get password")) {
                _putenv(pamh, "ERRINFO=Access denied (can not get password)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "is locked")) {
                _putenv(pamh, "ERRINFO=Access denied (user is locked)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "over limit")) {
                _putenv(pamh, "ERRINFO=Access denied (invalid user over limit)");
                return PAM_AUTH_ERR;
            }

            if (strstr(szBuf, "mismatch")) {
                sal_cmd_exec(szBuf, 512, "%s delete/cdb/app/login_record/%s", M_CDBCTL_PROG, pszUser);
            }

            if (strstr(szBuf, "or password")) {
                _putenv(pamh, "ERRINFO=Access denied (invalid user or password)");
                return PAM_AUTH_ERR;
            } 
        }

        sal_cmd_exec(szBuf, 512, "%s update/cdb/app/login/%s/user/%s",
                        M_CDBCTL_PROG, login_session, szEncUser);
        
         is_ssh_connect = 0;
         return PAM_SUCCESS;

    }
    else
    {
        if (sal_cmd_exec(szBuf, 512, "%s update/cdb/app/user/%s/user_password_check/%d/%s", M_CDBCTL_PROG, pszUser, is_ssh_connect, pszPassword))
        {
            if (strstr(szBuf, "not found")) {
                _putenv(pamh, "ERRINFO=Access denied (no such user)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "not get password")) {
                _putenv(pamh, "ERRINFO=Access denied (can not get password)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "is locked")) {
                _putenv(pamh, "ERRINFO=Access denied (user is locked)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "over limit")) {
                _putenv(pamh, "ERRINFO=Access denied (invalid user over limit)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "mismatch")) {
                _putenv(pamh, "ERRINFO=Access denied (service type mismatch)");
                return PAM_AUTH_ERR;
            }
            if (strstr(szBuf, "or password")) {
                _putenv(pamh, "ERRINFO=Access denied (invalid user or password)");
                return PAM_AUTH_ERR;
            } 
        }
        else
        {  
#if 0  /* add by wus for bug52967 modify password "admin" in 2019-07-15*/
#ifdef CONFIG_MOD_PASSWORD
            do_aaa_auth_local_change_password(pamh);
#endif
#endif
            
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/login/%s/user/%s",
                        M_CDBCTL_PROG, login_session, szEncUser);
             is_ssh_connect = 0;
             return PAM_SUCCESS;
        } 
    }
    
    
    is_ssh_connect = 0;
    
    return PAM_AUTH_ERR;
}



int do_aaa_author_local(pam_handle_t * pamh)
{
    char *pszUser = NULL;
    char szBuf[1024];
    int   user_privilege = 0;
    int   privilege = 0;
    int   vty_id=0;

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, TRUE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    sal_sscanf(login_vtyid, "vty%d", &vty_id);

    if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/vty/%d/privilege", M_CDBCTL_PROG, vty_id) != 0) {
        if (strstr(szBuf, "not found")) {
            _putenv(pamh, "ERRINFO=can't get vty privilege configuration");
            return PAM_AUTH_ERR;
        }

        _putenv(pamh, "ERRINFO=Internal error, can't get user configuration");
        return PAM_SERVICE_ERR;
    }
    
    sal_sscanf(szBuf, "/privilege=%d", &privilege);
    
    if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/user/%s/privilege", M_CDBCTL_PROG, pszUser) != 0) {
        if (strstr(szBuf, "not found")) {
            _putenv(pamh, "ERRINFO=can't get user privilege configuration");
            return PAM_AUTH_ERR;
        }

        _putenv(pamh, "ERRINFO=Internal error, can't get user configuration");
        return PAM_SERVICE_ERR;
    }

    sal_sscanf(szBuf, "/privilege=%d", &user_privilege);
    sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%d/privilege/%d",
        M_CDBCTL_PROG, vty_id, user_privilege); 

    /* modified by liwh for bug 46991 to support ssh aaa, 2018-03-30 */
    if (privilege != user_privilege)
    /* liwh end */
    {
        sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%d/config_privilege/%d",
            M_CDBCTL_PROG, vty_id, privilege);
    }

    return PAM_SUCCESS;
}


static int get_vty_index()
{
    if (!strcasecmp(login_vtyid, "con0")) {
        return 0;
    } 

    return atoi(login_vtyid + 3) + 1;
}

int do_aaa_auth_server(pam_handle_t * pamh, char *pszType)
{
    char *pszUser = NULL;
    char *pszPassword = NULL;
    char szBuf[1024];
    char szStatus[32];
    int ret = PAM_AUTH_ERR;
    int cnt;
    int vtyidx = get_vty_index();
    char szEncUser[USERNAME_SIZE * 2];
    char szEncPin[PASSWD_SIZE * 2];

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, FALSE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    /* read password from user */
    if (_get_password(pamh, &pszPassword, FALSE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    sal_memcpy(login_password, pszPassword, PASSWD_SIZE * 2);

    /* check user */
    sal_b64_ntop((unsigned char *)pszUser, strlen(pszUser), szEncUser, USERNAME_SIZE * 2);
    sal_b64_ntop((unsigned char *)pszPassword, strlen(pszPassword), szEncPin, PASSWD_SIZE * 2);
    cnt = 10;
do_create:
    if (sal_cmd_exec(szBuf, 512, 
            "%s create/cdb/app/auth_session/klish#%d#%s/user/%s/passwd/%s",
            M_CDBCTL_PROG, vtyidx, pszType, szEncUser, szEncPin) != 0) {
        if (strstr(szBuf, "limit")) {
            _putenv(pamh, "ERRINFO=Internal error, authentication session overflow");
            return PAM_SERVICE_ERR;
        }

        sal_cmd_exec(szBuf, 512, 
            "%s delete/cdb/app/auth_session/klish#%d#%s",
            M_CDBCTL_PROG, vtyidx, pszType);
        if ((--cnt) <= 0) {
            _putenv(pamh, "ERRINFO=Create authentication session faield");
            return PAM_SERVICE_ERR;
        }
        goto do_create;
    }

    /* check status */
    cnt = 60;
    while ((--cnt) > 0) {
        sleep(1);
        
        if (sal_cmd_exec(szBuf, 512, 
                "%s read/cdb/app/auth_session/klish#%d#%s",
                M_CDBCTL_PROG, vtyidx, pszType) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, can't read authentication status");
            ret = PAM_SERVICE_ERR;
            goto finish_out;
        }

        sal_cmd_get_token(szBuf, "status", szStatus, 32);
        if (!strcasecmp(szStatus, "success")) {
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/login/%s/user/%s",
                M_CDBCTL_PROG, login_session, szEncUser);
            ret = PAM_SUCCESS;
            goto finish_out;
        } else if (!strcasecmp(szStatus, "failed")) {
            _putenv(pamh, "ERRINFO=Authentication failed");
            ret = PAM_AUTH_ERR;
            goto finish_out;
        /* modified by liwh for bug 37986, 2016-06-01 */
        } else if (!strcasecmp(szStatus, "denied")) {
            _putenv(pamh, "ERRINFO=Authentication denied");
            ret = PAM_PERM_DENIED;
            goto finish_out;
        }
        /* liwh end */
    }

    if (cnt <= 0) {
        _putenv(pamh, "ERRINFO=Authentication timeout");
        ret = PAM_AUTH_ERR;
    }

finish_out:
    sal_cmd_exec(szBuf, 512, 
        "%s delete/cdb/app/auth_session/klish#%d#%s",
        M_CDBCTL_PROG, vtyidx, pszType);
    return ret;
}

int do_aaa_author_tacplus(pam_handle_t * pamh, char *pszType, bool get_passward)
{
    char *pszUser = NULL;
    char *pszPassword = NULL;
    char szBuf[1024];
    char szStatus[32];
    int  ret = PAM_AUTH_ERR;
    int  cnt;
    int  vtyidx = get_vty_index();
    char szEncUser[USERNAME_SIZE * 2];
    char szEncPin[PASSWD_SIZE * 2];

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, FALSE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    /* read password from user */
    if (get_passward)
    {
        if (_get_password(pamh, &pszPassword, FALSE) != PAM_SUCCESS) {
            return PAM_AUTH_ERR;
        }
    
        sal_memcpy(login_password, pszPassword, PASSWD_SIZE * 2);
    }
    else
    {
        pszPassword = login_password;
        sal_memcpy(pszPassword, "password", sal_strlen("password"));
    }

    //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "do_aaa_author_tacplus vtyidx %d", vtyidx);

    /* check user */
    sal_b64_ntop((unsigned char *)pszUser, strlen(pszUser), szEncUser, USERNAME_SIZE * 2);
    sal_b64_ntop((unsigned char *)pszPassword, strlen(pszPassword), szEncPin, PASSWD_SIZE * 2);
    
    cnt = 10;
do_create:
    if (sal_cmd_exec(szBuf, 512, 
            "%s create/cdb/app/auth_session/klish#%d#%s/user/%s/passwd/%s/author_start/1",
            M_CDBCTL_PROG, vtyidx, pszType, szEncUser, szEncPin) != 0) {
        if (strstr(szBuf, "limit")) {
            _putenv(pamh, "ERRINFO=Internal error, authorization session overflow");
            return PAM_SERVICE_ERR;
        }

        sal_cmd_exec(szBuf, 512, 
            "%s delete/cdb/app/auth_session/klish#%d#%s",
            M_CDBCTL_PROG, vtyidx, pszType);
        if ((--cnt) <= 0) {
            _putenv(pamh, "ERRINFO=Create authorization session faield");
            return PAM_SERVICE_ERR;
        }
        goto do_create;
    }

    /* check status */
    cnt = 60;
    while ((--cnt) > 0) {
        sleep(1);
        
        if (sal_cmd_exec(szBuf, 512, 
                "%s read/cdb/app/auth_session/klish#%d#%s",
                M_CDBCTL_PROG, vtyidx, pszType) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, can't read authorization status");
            ret = PAM_SERVICE_ERR;
            goto finish_out;
        }

        sal_cmd_get_token(szBuf, "status", szStatus, 32);
        if (!strcasecmp(szStatus, "success")) {
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/login/%s/user/%s",
                M_CDBCTL_PROG, login_session, szEncUser);
            ret = PAM_SUCCESS;
            goto finish_out;
        } else if (!strcasecmp(szStatus, "failed")) {
            _putenv(pamh, "ERRINFO=authorization failed");
            ret = PAM_AUTH_ERR;
            goto finish_out;
        } else if (!strcasecmp(szStatus, "denied")) {
            _putenv(pamh, "ERRINFO=authorization denied");
            ret = PAM_PERM_DENIED;
            goto finish_out;
        }
    }

    if (cnt <= 0) {
        _putenv(pamh, "ERRINFO=authorization timeout");
        ret = PAM_AUTH_ERR;
    }

finish_out:
    sal_cmd_exec(szBuf, 512, 
        "%s delete/cdb/app/auth_session/klish#%d#%s",
        M_CDBCTL_PROG, vtyidx, pszType);
    return ret;
}

/* return values:
 *	 1  = User not found
 *	 0  = OK
 *	-1  = Password incorrect
 *	-2  = System error
*/

int do_aaa_auth(pam_handle_t * pamh)
{
    char *pszMethod = NULL;
    char *pSavePtr = NULL;
    char szList[128];
    int  vtyidx = get_vty_index();
    char vty_szBuf[256];
    int  ret = PAM_SUCCESS;
    int  ret_radius = PAM_AUTHINFO_UNAVAIL;
    
#ifdef HAVE_DEBIAN
    import_debian_path();
#endif
    
    if (NULL == login_method) {
        _putenv(pamh, "ERRINFO=Internal error, please setting authentication list");
        return PAM_SERVICE_ERR;
    }
    snprintf(szList, 128, "%s", login_method);
    _putenv(pamh, "ERRINFO=Access denied");
    pszMethod = strtok_r(szList, ",", &pSavePtr);
    while (NULL != pszMethod) {
        ret = PAM_AUTH_ERR;
        if (!strcasecmp(pszMethod, "none")) {
            ret = PAM_SUCCESS;
            break;
        } else if (!strcasecmp(pszMethod, "enable")) {
            ret = do_aaa_auth_enable(pamh);
        } else if (!strcasecmp(pszMethod, "line")) {
            ret = do_aaa_auth_line(pamh);
        } else if (!strcasecmp(pszMethod, "local")) {
            ret = do_aaa_auth_local(pamh);

        } else if (!strcasecmp(pszMethod, "tacplus") 
                || !strcasecmp(pszMethod, "radius")) {
            ret = do_aaa_auth_server(pamh, pszMethod);

            /* modified by yejl for bug 46694, 2017-04-03 */
            if ((PAM_SUCCESS != ret) && !strcasecmp(pszMethod, "radius"))
            {
                sal_cmd_exec(vty_szBuf, 256, "%s update/cdb/app/vty/%d/is_ssh_login/0",
                            M_CDBCTL_PROG, vtyidx - 1);
            }

            /* modified by liwh for bug 37986, 2016-06-01 */
            if (PAM_PERM_DENIED == ret)
            {
                return ret;    
            }
            /* liwh end */

            if (!strcasecmp(pszMethod, "radius"))
            {
                ret_radius = ret;   
            }
        } else {
            _putenv(pamh, "ERRINFO=Internal error, invalid authentication method: %s",
                pszMethod);
            return PAM_SERVICE_ERR;
        }

        if (PAM_SUCCESS == ret) {
            //return PAM_SUCCESS;
            break;
        }

        /* process next method */
        pszMethod = strtok_r(NULL, ",", &pSavePtr);
    }

    if (PAM_SUCCESS != ret)
    {
        return PAM_AUTH_ERR;    
    }

    if (NULL == author_method) {
        _putenv(pamh, "ERRINFO=Internal error, please setting authorization list");
        return PAM_SERVICE_ERR;
    }
    snprintf(szList, 128, "%s", author_method);

    _putenv(pamh, "ERRINFO=Access denied");
    pszMethod = strtok_r(szList, ",", &pSavePtr); 
    while (NULL != pszMethod) {
        ret = PAM_AUTH_ERR;
        if (!strcasecmp(pszMethod, "none")) {
            return PAM_SUCCESS;
        } else if (!strcasecmp(pszMethod, "local")) {
            ret = do_aaa_author_local(pamh);
        } else if (!strcasecmp(pszMethod, "radius")) {

            if (PAM_AUTHINFO_UNAVAIL != ret_radius)
            {
                ret = ret_radius;    
            }
            else
            {
                ret = do_aaa_auth_server(pamh, pszMethod);
            }
        } else if (!strcasecmp(pszMethod, "tacplus")) {
            ret = do_aaa_author_tacplus(pamh, pszMethod, TRUE);

            if (PAM_PERM_DENIED == ret)
            {
                return ret;    
            }
        } else {
            _putenv(pamh, "ERRINFO=Internal error, invalid authorization method: %s",
                pszMethod);
            return PAM_SERVICE_ERR;
        }

        if (PAM_SUCCESS == ret) {
            return PAM_SUCCESS;
        }

        /* process next method */
        pszMethod = strtok_r(NULL, ",", &pSavePtr);
    }

    return PAM_AUTH_ERR;
}

int do_ssh_local_login(pam_handle_t *pamh, const char *pszUser, const char *pszPin)
{
    struct passwd *pw;
    char *pszEnc;

    pw = getpwnam(pszUser);
    if (NULL == pw) {
        pam_prompt(pamh, PAM_ERROR_MSG, NULL, "Access denied (no such user)");
        return PAM_USER_UNKNOWN;
    }
    pszEnc = crypt(pszPin, pw->pw_passwd);
    if (NULL == pszEnc || strcmp(pszEnc, pw->pw_passwd)) {
        pam_prompt(pamh, PAM_ERROR_MSG, NULL, "Access denied (incorrect password)");
	    return PAM_AUTH_ERR;
    }

    return PAM_SUCCESS;
}

int get_ssh_login_cfg(pam_handle_t *pamh, bool is_authenticate, bool* is_not_ssh_key)
{
    
    char        szBuf[256];
    static char szTty[32];
    static char szAaaStatus[32];
    static char szLoginMethod[128];
    static char szAuthorMethod[128];
    static char szAccountMethod[128];
    char        vty_szBuf[256];
    int         nAaa;

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif
   
    /* check aaa_new_model setting */
    if (sal_cmd_exec(szBuf, 256, "%s read/cdb/sys/sys_global/aaa_new_model",
            M_CDBCTL_PROG) != 0) {
        pam_prompt(pamh, PAM_ERROR_MSG, NULL, "Internal error, can't get aaa status");
        return PAM_AUTH_ERR;
    }
    sal_cmd_get_token(szBuf, "aaa_new_model", szAaaStatus, 32);
    nAaa = atoi(szAaaStatus);

    if (0 == nAaa) {
        aaa_status = NULL;
    } else {
        aaa_status = szAaaStatus;
    }
    
    /* fake tty */
    snprintf(szTty, 32, "ssh%u", getpid());
    if (sal_cmd_exec(szBuf, 256, "%s create/cdb/app/login/%s", 
            M_CDBCTL_PROG, szTty) != 0) {
        if (NULL == strstr(szBuf, " exist")) {
            pam_prompt(pamh, PAM_ERROR_MSG, NULL, "Access denied (too many users)");
            return PAM_AUTH_ERR;
        }
    }

    /* get vty config */
    if (sal_cmd_exec(szBuf, 256, "%s read/cdb/app/login/%s/vty", 
            M_CDBCTL_PROG, szTty) != 0) {
        pam_prompt(pamh, PAM_ERROR_MSG, NULL, "Internal error, allocate vty failed");
        return PAM_AUTH_ERR;
    }
    sal_cmd_get_token(szBuf, "vty", szVty, 32);

    /* XXX: delete login session, this may cause issues (multi login) */
    sal_cmd_exec(szBuf, 256, "%s delete/cdb/app/login/%s", 
            M_CDBCTL_PROG, szTty);

    if (sal_cmd_exec(vty_szBuf, 256, "%s read/cdb/app/vty/%s", 
            M_CDBCTL_PROG, &szVty[3]) != 0) {
        pam_prompt(pamh, PAM_ERROR_MSG, NULL, 
            "Internal error, read vty configuration failed");
        return PAM_AUTH_ERR;
    }
    login_vtyid = &szVty[0];
    
    //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "get_ssh_login_cfg login_vtyid %c vty_szBuf %s", &szVty[3], vty_szBuf);
    if (1 == nAaa) {

        if (is_authenticate)
        {
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%c/is_not_ssh_key/%d",
                M_CDBCTL_PROG, szVty[3], (int)is_authenticate);

            //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "get_ssh_login_cfg is_authenticate %d", (int)is_authenticate);
        }
        else
        {
            if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/vty/%c/is_not_ssh_key", M_CDBCTL_PROG, szVty[3]) != 0) {
                     if (strstr(szBuf, "not found")) {
                         pam_prompt(pamh, PAM_ERROR_MSG, NULL, "ERRINFO=can't get vty privilege configuration");
                         return PAM_AUTH_ERR;
                     }
        
                pam_prompt(pamh, PAM_ERROR_MSG, NULL, "ERRINFO=Internal error, can't get user configuration");
                return PAM_SERVICE_ERR;
            }
    
            sal_sscanf(szBuf, "/is_not_ssh_key=%d", (int*)is_not_ssh_key);
             
            //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "get_ssh_login_cfg is_not_ssh_key %d", (int)*is_not_ssh_key);
            
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%c/is_not_ssh_key/0",
                            M_CDBCTL_PROG, szVty[3]);
        }
        
        /*modified by yejl to fix bug 46694, 2018-04-03*/
        sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%c/is_ssh_login/1",
                M_CDBCTL_PROG, szVty[3]);
        /*ended by yejl*/
        
        /* get auth method */
        sal_cmd_get_token(vty_szBuf, "auth_method", szLoginMethod, 128);
        if (!strcasecmp(szLoginMethod, "")) {
            snprintf(szLoginMethod, 128, "default");
        }
        if (sal_cmd_exec(szBuf, 256, "%s read/cdb/app/authen/%s", 
                M_CDBCTL_PROG, szLoginMethod) != 0) {
            pam_prompt(pamh, PAM_ERROR_MSG, NULL, 
                "Internal error, read authentication method string failed");
            return PAM_AUTH_ERR;
        }

        sal_cmd_get_token(szBuf, "methods", szLoginMethod, 128);
        if (!strcasecmp(szLoginMethod, "")) {
            snprintf(szLoginMethod, 128, "local");
        }

        /* get author method */
        sal_cmd_get_token(vty_szBuf, "author_method", szAuthorMethod, 128);
        if (!strcasecmp(szAuthorMethod, "")) {
            snprintf(szAuthorMethod, 128, "default");
        }
        if (sal_cmd_exec(szBuf, 256, "%s read/cdb/app/author/%s", 
                M_CDBCTL_PROG, szAuthorMethod) != 0) {
            pam_prompt(pamh, PAM_ERROR_MSG, NULL, 
                "Internal error, read authorization method string failed");
            return PAM_AUTH_ERR;
        }

        sal_cmd_get_token(szBuf, "methods", szAuthorMethod, 128);
        if (!strcasecmp(szAuthorMethod, "")) {
            snprintf(szAuthorMethod, 128, "local");
        }

        /* get account method */
        sal_cmd_get_token(vty_szBuf, "account_method", szAccountMethod, 128);
        if (!strcasecmp(szAccountMethod, "")) {
            snprintf(szAccountMethod, 128, "default");
        }
        if (sal_cmd_exec(szBuf, 256, "%s read/cdb/app/account/%s", 
                M_CDBCTL_PROG, szAccountMethod) != 0) {
            pam_prompt(pamh, PAM_ERROR_MSG, NULL, 
                "Internal error, read accounting method string failed");
            return PAM_AUTH_ERR;
        }

        sal_cmd_get_token(szBuf, "start_stop", szAccountMethod, 128);
        if (0 == sal_memcmp(szAccountMethod, "1", sal_strlen("1")))
        {
            sal_cmd_get_token(szBuf, "methods", szAccountMethod, 128);
        }
        else
        {
            snprintf(szAccountMethod, 128, "none");
        }
    } else {
        /* Fix bug 41424 */
        sal_cmd_get_token(vty_szBuf, "login", szLoginMethod, 128);
        if (!strcasecmp(szLoginMethod, "nologin")) {
            snprintf(szLoginMethod, 128, "2");
        } else if (!strcasecmp(szLoginMethod, "login")) {
            snprintf(szLoginMethod, 128, "2");
        } else if (!strcasecmp(szLoginMethod, "local")) {
            snprintf(szLoginMethod, 128, "2");
        } else {
            pam_prompt(pamh, PAM_ERROR_MSG, NULL, 
                "Internal error, unknown login authentication method %s", szLoginMethod);
            return PAM_AUTH_ERR;
        }

        snprintf(szAuthorMethod, 128, "local");
        snprintf(szAccountMethod, 128, "none");
    }

    
    //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "get_ssh_login_cfg szLoginMethod %s szAuthorMethod %s szAccountMethod %s", 
    //    szLoginMethod, szAuthorMethod, szAccountMethod);

    login_method = szLoginMethod;
    author_method = szAuthorMethod;
    account_method = szAccountMethod;
	login_session = "unknown";
	_putenv(pamh, "LOGINTTY=%s", szTty);
    return PAM_SUCCESS;
}

/* modified by liwh for bug 53357, 2019-08-29 */
#define BANNER_LOGIN_FILE   "/tmp/banner_login"

int pam_ssh_banner_show(char *mode, pam_handle_t * pamh)
{
    char buf[1024];
    char buf1[1024];
    FILE *fp = NULL;
    char *fname = NULL;

    sal_memset(buf, 0, 1024);

    if (0 == sal_strcmp(mode, "login"))
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
        while (sal_fgets(buf, 1024, fp))
        {
            sal_memset(buf1, 0, 1024);
            sal_memcpy(buf1, buf, (sal_strlen(buf)-1));

            pam_prompt(pamh, PAM_ERROR_MSG, NULL, "%s", buf1);
        }
        sal_fclose(fp);
    }

    return 0;    
}
/* liwh end */

/* --- authentication management functions (only) --- */
PAM_EXTERN int
pam_sm_authenticate(pam_handle_t * pamh, int flags,
					int argc, const char **argv)
{
	int retval = PAM_AUTH_ERR;
	int ctrl;
	const char *pszTty = NULL;
    bool is_not_ssh_key = TRUE;
    char        szBuf[256];
    (void)ctrl;

	/* parse arguments */
	ctrl = _pam_parse(pamh, argc, argv);
    if (pam_get_item(pamh, PAM_TTY, (const void **)&pszTty) == PAM_SUCCESS
            && !strcasecmp(pszTty, "ssh")) {
            
        /* modified by liwh for bug 53357, 2019-08-29 */
        pam_ssh_banner_show("login", pamh);
        /* liwh end */
    
            is_ssh_connect = 1;
        retval = get_ssh_login_cfg(pamh, TRUE, &is_not_ssh_key);
    } else {
        retval = get_auth_method(pamh);
    }

    if (PAM_SUCCESS != retval) {
        return retval;
    }

    if (NULL != aaa_status) {
        retval = do_aaa_auth(pamh);
    } else {
        if (!strcasecmp(login_method, "1")) { /* check line password */
            retval = do_aaa_auth_line(pamh);
        } else if (!strcasecmp(login_method, "2")) { /* check local user */
            retval = do_aaa_auth_local(pamh);
        } else {
            retval = PAM_AUTH_ERR;
        }
    }

    /* modified by liwh for bug 50098, 2018-11-27 
         when authenticate fails, should set is_not_ssh_key to default value 0 */
    if (PAM_SUCCESS != retval) {       
        if (is_not_ssh_key)
        {
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%c/is_not_ssh_key/0",
                                M_CDBCTL_PROG, szVty[3]);
        }
    }
    /* liwh end */
    
	if (PAM_SUCCESS == retval) {
#ifndef HAVE_DEBIAN
        _putenv(pamh, "LD_LIBRARY_PATH=/centec_switch/lib");
#endif
	}
    
	return retval;
}

int do_aaa_account_server(pam_handle_t * pamh, char *pszType)
{
    char *pszUser = NULL;
    char szBuf[1024];
    char szStatus[32];
    int ret = PAM_AUTH_ERR;
    int cnt;
    int vtyidx = get_vty_index();
    char szEncUser[USERNAME_SIZE * 2];

#ifdef HAVE_DEBIAN
    import_debian_path();
#endif

    /* get user name */
    if (_get_username(pamh, &pszUser, FALSE) != PAM_SUCCESS) {
        return PAM_AUTH_ERR;
    }

    /* check user */
    sal_b64_ntop((unsigned char *)pszUser, sal_strlen(pszUser), szEncUser, USERNAME_SIZE * 2);
    cnt = 10;
do_create:
    if (sal_cmd_exec(szBuf, 512, 
            "%s create/cdb/app/auth_session/klish#%d#%s/user/%s/acct_start/1",
            M_CDBCTL_PROG, vtyidx, pszType, szEncUser) != 0) {
        if (strstr(szBuf, "limit")) {
            _putenv(pamh, "ERRINFO=Internal error, accounting session overflow");
            return PAM_SERVICE_ERR;
        }

        sal_cmd_exec(szBuf, 512, 
            "%s delete/cdb/app/auth_session/klish#%d#%s",
            M_CDBCTL_PROG, vtyidx, pszType);
        if ((--cnt) <= 0) {
            _putenv(pamh, "ERRINFO=Create accounting session faield");
            return PAM_SERVICE_ERR;
        }
        goto do_create;
    }

    /* check status */
    cnt = 60;
    while ((--cnt) > 0) {
        sleep(1);
        
        if (sal_cmd_exec(szBuf, 512, 
                "%s read/cdb/app/auth_session/klish#%d#%s",
                M_CDBCTL_PROG, vtyidx, pszType) != 0) {
            _putenv(pamh, "ERRINFO=Internal error, can't read accounting status");
            ret = PAM_SERVICE_ERR;
            goto finish_out;
        }

        sal_cmd_get_token(szBuf, "status", szStatus, 32);
        if (!strcasecmp(szStatus, "success")) {
            ret = PAM_SUCCESS;
            goto finish_out;
        } else if (!strcasecmp(szStatus, "failed")) {
            _putenv(pamh, "ERRINFO=Accounting failed");
            ret = PAM_AUTH_ERR;
            goto finish_out;
        } else if (!strcasecmp(szStatus, "denied")) {
            _putenv(pamh, "ERRINFO=Accounting denied");
            ret = PAM_PERM_DENIED;
            goto finish_out;
        }
    }

    if (cnt <= 0) {
        _putenv(pamh, "ERRINFO=Accounting timeout");
        ret = PAM_AUTH_ERR;
    }

finish_out:

    /* modified by liwh for bug 46712, 2018-04-02 */
    if (PAM_SUCCESS!= ret)
    {
        int config_privilege = 0;
        if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/vty/%d/config_privilege", M_CDBCTL_PROG, (vtyidx -1)) != 0) {
            if (strstr(szBuf, "not found")) {
                _putenv(pamh, "ERRINFO=can't get user config_privilege configuration");
                return PAM_AUTH_ERR;
            }
    
            _putenv(pamh, "ERRINFO=Internal error, can't get user configuration");
            return PAM_SERVICE_ERR;
        }

        sal_sscanf(szBuf, "/config_privilege=%d", &config_privilege);

        if (config_privilege)
        {
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%d/config_privilege/0",
                    M_CDBCTL_PROG, (vtyidx -1));
            
            sal_cmd_exec(szBuf, 512, "%s update/cdb/app/vty/%d/privilege/%d",
                M_CDBCTL_PROG, (vtyidx -1), config_privilege); 
        }
    }
    /* liwh end */
    
    sal_cmd_exec(szBuf, 512, 
        "%s delete/cdb/app/auth_session/klish#%d#%s",
        M_CDBCTL_PROG, vtyidx, pszType);
    return ret;
}

/* return values:
 *	 1  = User not found
 *	 0  = OK
 *	-1  = Password incorrect
 *	-2  = System error
*/

int do_aaa_account(pam_handle_t * pamh)
{
    char *pszMethod = NULL;
    char *pSavePtr = NULL;
    char szList[128];
    int ret;
    
    if (NULL == account_method) {
        _putenv(pamh, "ERRINFO=Internal error, please setting accounting list");
        return PAM_SERVICE_ERR;
    }
    snprintf(szList, 128, "%s", account_method);

    _putenv(pamh, "ERRINFO=Access denied");
    pszMethod = strtok_r(szList, ",", &pSavePtr);
    while (NULL != pszMethod) {
        ret = PAM_AUTH_ERR;
        if (!strcasecmp(pszMethod, "none")) {
            return PAM_SUCCESS;
        } else if (!strcasecmp(pszMethod, "tacplus") 
            || !strcasecmp(pszMethod, "radius")) {
            ret = do_aaa_account_server(pamh, pszMethod);

            if (PAM_PERM_DENIED == ret)
            {
                return ret;    
            }
        } else {
            _putenv(pamh, "ERRINFO=Internal error, invalid authentication method: %s",
                pszMethod);
            return PAM_SERVICE_ERR;
        }

        if (PAM_SUCCESS == ret) {
            return PAM_SUCCESS;
        }

        /* process next method */
        pszMethod = strtok_r(NULL, ",", &pSavePtr);
    }

    return PAM_AUTH_ERR;
}

/* return values:
 *	 1  = User not found
 *	 0  = OK
 *	-1  = Password incorrect
 *	-2  = System error
*/

int do_aaa_author(pam_handle_t * pamh)
{
    char *pszMethod = NULL;
    char *pSavePtr = NULL;
    char szList[128];
    int  ret = PAM_SUCCESS;
    int  ret_radius = PAM_AUTHINFO_UNAVAIL;
    
    if (NULL == author_method) {
        _putenv(pamh, "ERRINFO=Internal error, please setting authorization list");
        return PAM_SERVICE_ERR;
    }
    snprintf(szList, 128, "%s", author_method);

    //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "do_aaa_author szList %s", szList);

    _putenv(pamh, "ERRINFO=Access denied");
    pszMethod = strtok_r(szList, ",", &pSavePtr); 
    while (NULL != pszMethod) {
        ret = PAM_AUTH_ERR;
        if (!strcasecmp(pszMethod, "none")) {
            return PAM_SUCCESS;
        } else if (!strcasecmp(pszMethod, "local")) {
            ret = do_aaa_author_local(pamh);
        } else if (!strcasecmp(pszMethod, "radius")) {

            if (PAM_AUTHINFO_UNAVAIL != ret_radius)
            {
                ret = ret_radius;    
            }
            else
            {
                ret = do_aaa_auth_server(pamh, pszMethod);
            }
        } else if (!strcasecmp(pszMethod, "tacplus")) {
            ret = do_aaa_author_tacplus(pamh, pszMethod, FALSE);

            if (PAM_PERM_DENIED == ret)
            {
                return ret;    
            }
        } else {
            _putenv(pamh, "ERRINFO=Internal error, invalid authorization method: %s",
                pszMethod);
            return PAM_SERVICE_ERR;
        }

        if (PAM_SUCCESS == ret) {
            return PAM_SUCCESS;
        }

        /* process next method */
        pszMethod = strtok_r(NULL, ",", &pSavePtr);
    }

    return PAM_AUTH_ERR;
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t * pamh, int flags, int argc, const char **argv)
{
    int retval = PAM_AUTH_ERR;
	int ctrl;
	const char *pszTty = NULL;
    bool is_not_ssh_key = TRUE;

    (void)ctrl;
	/* parse arguments */
	ctrl = _pam_parse(pamh, argc, argv);
    if (pam_get_item(pamh, PAM_TTY, (const void **)&pszTty) == PAM_SUCCESS
            && !strcasecmp(pszTty, "ssh")) {
        retval = get_ssh_login_cfg(pamh, FALSE, &is_not_ssh_key);
    } else {
        retval = get_auth_method(pamh);
    }

    if (PAM_SUCCESS != retval) {
        return retval;
    }

    //pam_prompt(pamh, PAM_ERROR_MSG, NULL, "pam_sm_acct_mgmt is_not_ssh_key %d", (int)is_not_ssh_key);

    if (!is_not_ssh_key)
    {
        if (NULL != aaa_status) {
            do_aaa_author(pamh);        
        }
    }

    if (PAM_SUCCESS != retval) {
        return retval;
    }

    if (NULL != aaa_status) {
        retval = do_aaa_account(pamh);
    }

    if (PAM_SUCCESS == retval) {
#ifndef HAVE_DEBIAN
        _putenv(pamh, "LD_LIBRARY_PATH=/centec_switch/lib");
#endif
    }

    return retval;
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t * pamh, int flags, int argc, const char **argv)
{
	return PAM_SUCCESS;
}

PAM_EXTERN int 
pam_sm_open_session(pam_handle_t * pamh, 	int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

PAM_EXTERN int 
pam_sm_close_session(pam_handle_t * pamh, 	int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}


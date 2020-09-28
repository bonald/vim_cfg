/****************************************************************************
* $Id$
*  NSS cdb module
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-11-04 08:47
* Reason        : First Create.
****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <nss.h>
#include <pwd.h>
#include <string.h>

#include "nss_cdb.h"
#include "sal.h"
#include "rsa_derhex.h"

#ifdef _GLB_UML_SYSTEM_
#define _IO_MTSAFE_IO
#include <bits/libc-lock.h>
#undef _IO_MTSAFE_IO
#define M_KLISH_PROG     "/centec_switch/sbin/klish"
#define M_CDBCTL_PROG    "/centec_switch/sbin/cdbctl"
#else /* !_GLB_UML_SYSTEM_ */
#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
#include <pthread.h>
#else
#define _IO_MTSAFE_IO
#include <bits/libc-lock.h>
#undef _IO_MTSAFE_IO
#endif
/* Added by kcao for bug46052, nss cdb used by linux utils with undefined PATH enviroment, must uses absolute path */
#if (defined _CTC_X86_64_)
#define CTC_ETC_BASE_DIR "/switch"
#define M_KLISH_PROG     CTC_ETC_BASE_DIR"/usr/sbin/klish"
#define M_CDBCTL_PROG    CTC_ETC_BASE_DIR"/usr/sbin/cdbctl"
#else
#ifdef HAVE_DEBIAN
#define M_KLISH_PROG     "/switch/usr/sbin/klish"
#define M_CDBCTL_PROG    "/switch/usr/sbin/cdbctl"
#else
#define M_KLISH_PROG     "/usr/sbin/klish"
#define M_CDBCTL_PROG    "/usr/sbin/cdbctl"
#endif
#endif
#endif

#ifndef USERNAME_SIZE
#define USERNAME_SIZE       64
#endif

#ifndef PASSWD_SIZE
#define PASSWD_SIZE         256
#endif

//#define NSS_CDB_DEBUG
#ifndef NSS_CDB_DEBUG
#define CONSOLE_PRINT(fmt, args...) 
#else
#define CONSOLE_PRINT(fmt, args...)                      \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    fprintf(fp_console, fmt, ##args);   \
    fclose(fp_console);                                        \
}
#endif

/* Get the declaration of the parser function.  */
#define ENTNAME pwent
#define STRUCTURE passwd
#define EXTERN_PARSER
#include "files-parse.c"

/* Protect global state against multiple changers */
#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#else
__libc_lock_define_initialized(static, lock)
#endif

static char gszOldUser[USERNAME_SIZE] = { '\0' };

/* sshd will return badpw when use PAM to perform radius / tacplus auth
  * to avoid this need fake a pwd entry 
  */
static int needFakePwd(void)
{
    extern char *__progname;

    if (strstr(__progname, "sshd")) {
        return 1;
    }
    
    return 0;
}

static int getFakeUser(char *pszUser, int uid, char *pszRet, size_t nBufsz)
{
    char *pszTmp = pszUser;
    char szEncPin[PASSWD_SIZE * 2];

    pszRet[0] = '\0';
    if (!needFakePwd()) {
        return -1;
    }

    if (NULL == pszTmp) {
        pszTmp = "EKAFXELA";
    }

    if (gen_passwd_crypt("\b\r\nEKAFXELA", szEncPin, PASSWD_SIZE * 2) != 0) {
        return -1;
    }
    
    CONSOLE_PRINT("getFakeUser %s\r\n", szEncPin);
    snprintf(pszRet, nBufsz, 
        "/key=%s/uid=%d/privilege=1/enc_passwd=%s/passwd=/rsakey=",
        pszTmp, uid, szEncPin);
    return 0;
}

static void internal_cdb_trim_crlf(char *pszBuf)
{
    if (NULL != pszBuf) {
        char *p = pszBuf + (strlen(pszBuf) - 1);

        if ('\n' == *p) {
            *p = '\0';
            p--;
        }
        if (p >= pszBuf && '\r' == *p) {
            *p = '\0';
        }
    }
    return;
}

static void internal_cdb_endpwent(void)
{
    gszOldUser[0] = '\0';
    return;
}

enum nss_status _nss_cdb_endpwent(void)
{
#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_lock(&mutex);
#else
    __libc_lock_lock(lock);
#endif

    internal_cdb_endpwent();

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_unlock(&mutex);
#else
    __libc_lock_unlock(lock);
#endif

    return NSS_STATUS_SUCCESS;
}

enum nss_status internal_cdb_setpwent(void)
{
    return NSS_STATUS_SUCCESS;
}

enum nss_status _nss_cdb_setpwent(int stayopen)
{
    enum nss_status result;

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_lock(&mutex);
#else
    __libc_lock_lock(lock);
#endif

    internal_cdb_endpwent();

    result = internal_cdb_setpwent();

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_unlock(&mutex);
#else
    __libc_lock_unlock(lock);
#endif

    return result;
}

static enum nss_status
internal_cdb_build_pwd(char *pszBuf, struct passwd *pwd, 
    char *buffer, size_t buflen, int *errnop)
{
    struct parser_data *data = (void *) buffer;
    int ret = NSS_STATUS_UNAVAIL;
    char szBuf[512];
    char szUsername[USERNAME_SIZE];
    char szPasswd[PASSWD_SIZE];
    char szTmp[PASSWD_SIZE];
    char szUid[16];
    int retlen;
    char *p;

    sal_cmd_get_token(pszBuf, "key", szUsername, USERNAME_SIZE);
    if (sal_cmd_get_token(pszBuf, "passwd", szPasswd, PASSWD_SIZE) != 0) {
        return NSS_STATUS_UNAVAIL;
    }
    if ('\0' == szPasswd[0]) {
        if (sal_cmd_get_token(pszBuf, "enc_passwd", szTmp, PASSWD_SIZE) != 0) {
            return NSS_STATUS_UNAVAIL;
        }
        if ('\0' == szTmp[0]) {
            /* empty password */
            snprintf(szPasswd, PASSWD_SIZE, "%s", "x");
        } else if (convert_passwd_for_nss(szTmp, strlen(szTmp), szPasswd, PASSWD_SIZE) != 0) {
            return NSS_STATUS_UNAVAIL;
        }
    } else if (gen_passwd_crypt(szPasswd, szTmp, PASSWD_SIZE) != 0
        || convert_passwd_for_nss(szTmp, strlen(szTmp), szPasswd, PASSWD_SIZE) != 0) {
        return NSS_STATUS_UNAVAIL;
    }

    if (sal_cmd_get_token(pszBuf, "uid", szUid, 16) != 0) {
        snprintf(szUid, 16, "2001");
    }
    retlen = snprintf(szBuf, 512, "%s:%s:0:%s:Cdb User:/mnt/flash:%s",
            szUsername, szPasswd, szUid, M_KLISH_PROG);
    if (buflen <= retlen) {
        *errnop = ERANGE;
        ret = NSS_STATUS_TRYAGAIN;
    } else {  
        p = buffer;
        snprintf(buffer, buflen, "%s", szBuf);
        retlen = parse_line (p, pwd, data, buflen, errnop);
        if (retlen > 0) {
            ret = NSS_STATUS_SUCCESS;
            snprintf(gszOldUser, USERNAME_SIZE, "%s", szUsername);
            CONSOLE_PRINT("pw_name: %s\r\n", pwd->pw_name);
            CONSOLE_PRINT("pw_passwd: %s\r\n", pwd->pw_passwd);
            CONSOLE_PRINT("pw_uid: %d\r\n", pwd->pw_uid);
            CONSOLE_PRINT("pw_gid: %d\r\n", pwd->pw_gid);
            CONSOLE_PRINT("pw_gecos: %s\r\n", pwd->pw_gecos);
            CONSOLE_PRINT("pw_dir: %s\r\n", pwd->pw_dir);
            CONSOLE_PRINT("pw_shell: %s\r\n", pwd->pw_shell);
        } else if (retlen < 0) {
            pwd = NULL;
            ret = NSS_STATUS_NOTFOUND;
        } else {
            /* XXX: this one is ill-formed */
        }
    }

    return ret;
}

static enum nss_status
internal_cdb_getpwent_r(struct passwd *pwd, char *buffer, size_t buflen, int *errnop)
{
    char szFilename[] = "/tmp/.nss_cdbXXXXXX";
    int ret = NSS_STATUS_UNAVAIL;
    FILE *fp = NULL;
    char szUsername[USERNAME_SIZE];
    char szBuf[1024];
    int fd = -1;
    int find = 0;

    if ((fd = mkstemp(szFilename)) < 0) {
        goto err_out;
    }
    close(fd);

    if (sal_cmd_exec_file(szFilename, "%s read/cdb/app/user", M_CDBCTL_PROG) != 0
            || NULL == (fp = fopen(szFilename, "r"))) {
        goto err_out;
    }

    while (fgets(szBuf, 1024, fp)) {
        internal_cdb_trim_crlf(szBuf);
        
        if (sal_cmd_get_token(szBuf, "key", szUsername, USERNAME_SIZE) != 0
            || '\0' == szUsername[0]) {
            goto err_out;
        }

        if ('\0' == gszOldUser[0]) {
            find = 1;
            break;
        } else if (!strcmp(gszOldUser, szUsername)) {
            if (fgets(szBuf, 1024, fp)) {
                internal_cdb_trim_crlf(szBuf);
                find = 1;
            }
            break;
        }
    }

    if (!find) {
        pwd = NULL;
        ret = NSS_STATUS_SUCCESS;
    } else {
        ret = internal_cdb_build_pwd(szBuf, pwd, buffer, buflen, errnop);
        if (NSS_STATUS_SUCCESS == ret && NULL != pwd) {
            snprintf(gszOldUser, USERNAME_SIZE, "%s", pwd->pw_name);
        }
    }

err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    unlink(szFilename);
    return ret;
}

enum nss_status
_nss_cdb_getpwent_r(struct passwd *result, char *buffer, size_t buflen,
                    int *errnop)
{
    int status;

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_lock(&mutex);
#else
    __libc_lock_lock(lock);
#endif

    status = internal_cdb_getpwent_r(result, buffer, buflen, errnop);

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_unlock(&mutex);
#else
    __libc_lock_unlock(lock);
#endif

    return status;
}

static enum nss_status
internal_cdb_getpwnam_r(const char *name, struct passwd *pwd,
                    char *buffer, size_t buflen, int *errnop)
{
    int ret = NSS_STATUS_UNAVAIL;
    char szBuf[512];
    int find = 0;

    extern char *__progname;
    CONSOLE_PRINT("call internal_cdb_getpwnam_r: %d %s\r\n", getpid(), __progname);
    if (sal_cmd_exec(szBuf, 512, "%s read/cdb/app/user/%s", M_CDBCTL_PROG, name) == 0) {
        find = 1;
    } else {
        if (getFakeUser((char *)name, 2001, szBuf, 512) == 0) {
            find = 1;
        } else {
            if (strstr(szBuf, "not found")) {
                ret = NSS_STATUS_NOTFOUND;
            }
        }
    }


    if (find) {
        ret = internal_cdb_build_pwd(szBuf, pwd, buffer, buflen, errnop);
    }
    return ret;
}
                    

enum nss_status
_nss_cdb_getpwnam_r(const char *name, struct passwd *pwd,
                    char *buffer, size_t buflen, int *errnop)
{
    int status;

    if (name == NULL) {
        *errnop = EINVAL;
        return NSS_STATUS_UNAVAIL;
    }

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_lock(&mutex);
#else
    __libc_lock_lock(lock);
#endif

    status = internal_cdb_getpwnam_r(name, pwd, buffer, buflen, errnop);

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_unlock(&mutex);
#else
    __libc_lock_unlock(lock);
#endif

    return status;
}

static enum nss_status
intern_nss_cdb_getpwuid_r(uid_t uid, struct passwd *pwd,
    char *buffer, size_t buflen, int *errnop)
{
    char szFilename[] = "/tmp/.nss_cdbXXXXXX";
    int ret = NSS_STATUS_UNAVAIL;
    FILE *fp = NULL;
    char szBuf[1024];
    char szUid[16];
    int fd = -1;
    int find = 0;   

    if ((fd = mkstemp(szFilename)) < 0) {
        goto err_out;
    }
    close(fd);

    if (sal_cmd_exec_file(szFilename, "%s read/cdb/app/user", M_CDBCTL_PROG) != 0
            || NULL == (fp = fopen(szFilename, "r"))) {
        goto err_out;
    }

    while (fgets(szBuf, 1024, fp)) {
        internal_cdb_trim_crlf(szBuf);
        
        if (sal_cmd_get_token(szBuf, "uid", szUid, 16) != 0
            || '\0' == szUid[0]) {
            goto err_out;
        }

        if (atoi(szUid) == uid) {
            find = 1;
            break;
        }
    }

    if (!find) {
        pwd = NULL;
        ret = NSS_STATUS_NOTFOUND;
        goto err_out;
    } else {
        ret = internal_cdb_build_pwd(szBuf, pwd, buffer, buflen, errnop);
    }

err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    unlink(szFilename);
    return ret;
}

enum nss_status
_nss_cdb_getpwuid_r(uid_t uid, struct passwd *pwd,
                    char *buffer, size_t buflen, int *errnop)
{
    int status;

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_lock(&mutex);
#else
    __libc_lock_lock(lock);
#endif

    status = intern_nss_cdb_getpwuid_r(uid, pwd, buffer, buflen, errnop);

#if (defined _CTC_X86_64_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    pthread_mutex_unlock(&mutex);
#else
    __libc_lock_unlock(lock);
#endif

    return status;
}


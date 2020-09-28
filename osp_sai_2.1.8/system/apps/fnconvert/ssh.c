/****************************************************************************
* $Id$
*  implement ssh client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2015-06-16 09:12
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "sal.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <sshkey.h>
#include <openssl/rsa.h>
#include <authfile.h>
//#include <entropy.h>

#include "lib_fs.h"
#include "param_check.h"
#include "fn_ssh.h"
#include "rsa_derhex.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
extern char *g_pszOutFilename;
extern char *g_pszKeyFmt;

/* hmac array, for SSHv2 only */
struct {
    int version;
    char *pszName;
    char *pszSshName;
} g_sshHmacArr[] = {
    {2, "hmac-md5-128",  "hmac-md5"},
    {2, "hmac-md5-96",   "hmac-md5-96"},
    {2, "hmac-sha1-160", "hmac-sha1"},
    {2, "hmac-sha1-96",  "hmac-sha1-96"},
    {-1, NULL, NULL}
};

/* cipher array */
struct {
    int version;
    char *pszName;
} g_sshClientCipherArr[] = {
    {1, "3des"},
    {1, "des"},
    {2, "3des-cbc"},
    {2, "aes128-cbc"},
    {2, "aes192-cbc"},
    {2, "aes256-cbc"},
    {-1, NULL}
};

/****************************************************************************
 *
* Function
* 
****************************************************************************/

/* check and get hmac name
 * for SSHv2 only
 * return NULL for error
 */
static char *
_ssh_get_hmac_byname(char *pszName)
{
    int i = 0;

    while (NULL != g_sshHmacArr[i].pszName)
    {
        if (!sal_strcasecmp(g_sshHmacArr[i].pszName, pszName))
        {
            return g_sshHmacArr[i].pszSshName;
        }
        i++;
    }
    return NULL;
}

/* check and get ciphers version number 
 * return 1 or 2 for success
 * return -1 for error
 */
static int
_ssh_get_cipher_version(char *pszName)
{
    int i = 0;

    while (NULL != g_sshClientCipherArr[i].pszName)
    {
        if (!sal_strcasecmp(g_sshClientCipherArr[i].pszName, pszName))
        {
            return g_sshClientCipherArr[i].version;
        }
        i++;
    }

    return -1;
}

/* SSH client support start { */
#define M_SSH_CLIENT_LOGIN_PARAMCHECK(cnt, msg) \
do  \
{ \
    if ((i + (cnt)) == argc) \
    { \
        sal_printf((msg)); \
        goto err_out; \
    } \
} \
while (0)


static int _ssh_get_rsa_key(char *pszKeyName, char *pszTmpName)
{
    char szTmpFile[] = "/tmp/rsacXXXXXX"; /* for rsa key */
    int fd;
    int ret = -1;
    /* create temporary files */
    if ((fd = mkstemp(szTmpFile)) < 0) {
        sal_printf("%% Generate temporary file failed: %s\n", sal_strerror(errno));
        return -1;
    }
    sal_close(fd);
    
    if (NULL == pszKeyName || NULL == pszTmpName) {
        assert(0);
        goto err_out;
    }

    if (sal_cmd_exec_file(szTmpFile, "cdbctl read/cdb/app/rsa/%s", 
            pszKeyName) != 0) {
        goto err_out;
    }

    g_pszOutFilename = pszTmpName;
    g_pszKeyFmt = "private";
    ret = do_rsa_convert("Process SSH key", szTmpFile, E_TYPE_PEM);
    
err_out:
    sal_unlink(szTmpFile);
    return ret;
}

int ssh_login(char *argv[], int argc)
{
    int nRet = -1;
    char *pszKeyName = NULL;
    char *pszVersion = M_SSH_DEFAULT_PROTOCOL_VERSION;
    char szPrompts[M_SSH_OPTIONS_BUFSIZE];
    char *pszHost = NULL;
    char *pszPort = M_SSH_DEFAULT_PORT;
    char szCipherBuf[M_SSH_CIPHER_BUFSIZE];
    char *pCipherArr[M_SSH_MAX_CIPHER_ARG_NUM];
    int nCipherArrIdx = 0;
    char szHmacBuf[M_SSH_HMAC_BUFSIZE];
    char *pHmacArr[M_SSH_MAX_HMAC_ARR_NUM];
    int nHmacArrIdx = 0;
    char szCmd[M_SSH_CLIENT_CMD_BUFSIZE];
    char *p = NULL;
    int nLeftLen = 0;
    int nPrtLen = 0;
    int nTmpLen = 0;
    int i = 0;
    char szTmpName[] = "/tmp/prsaXXXXXX"; /* for rsa key */
    char szTmpName2[] = "/tmp/sshkXXXXXX.pub  "; /* for ssh */
    int fd = -1;
    struct sshkey *pSshKey = NULL;
    struct sshkey *pSshPubKey = NULL;
    int argco = 0;
    char *argvo[M_SSH_CLIENT_MAX_ARGS];
    FILE *fp = NULL;
    int mgmtif = 0;

    if (argc < 4) {
        sal_printf("%% Invalid command line\n");
        return -1;
    }

    if (strcasecmp(argv[0], "ssh") || strcasecmp(argv[1], "-l")) {
        sal_printf("%% Invalid argument: please start with ssh -l\n");
        return -1;
    }
    
    /* create temporary files */
    if ((fd = mkstemp(szTmpName)) < 0) {
        sal_printf("%% Generate temporary file failed: %s\n", sal_strerror(errno));
        goto err_out;
    }
    sal_close(fd);

    /* process username */
    if (check_username(argv[2]) != 0) {
        sal_printf("%% Invalid user name: %s\n", M_USERNAME_LIMIT_STR);
        goto err_out;
    }

    /* parse command options */
    i = 3;
    pCipherArr[0] = NULL;
    nCipherArrIdx = 0;
    pHmacArr[0] = NULL;
    nHmacArrIdx = 0;
    szCmd[0] = '\0';
    szCipherBuf[0] = '\0';
    szHmacBuf[0] = '\0';
    szPrompts[0] = '\0';
    while (i < argc) {
        /* process key file */
        if (!sal_strcasecmp(argv[i], "-i")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(1, "%% Please specify RSA private key name\n");
            pszKeyName = argv[i + 1];
            i += 2;
            continue;
        }

        /* process port */
        if (!sal_strcasecmp(argv[i], "-p")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(1, "%% Please specify remote port number to connect\n");
            pszPort = argv[i + 1];
            i += 2;
            continue;
        }

        /* process protocol version */
        if (!sal_strcasecmp(argv[i], "-v")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(1, "%% Please specify protocol version number\n");
            pszVersion = argv[i + 1];
            i += 2;
            continue;
        }

        /* process cipher list */
        if (!sal_strcasecmp(argv[i], "-c")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(1, "%% Please specify encryption algorithms\n");
            i++;
            while (i < argc) {
                if (-1 == _ssh_get_cipher_version(argv[i])) {
                    break;
                }
                
                if ((nCipherArrIdx + 1) >= M_SSH_MAX_CIPHER_ARG_NUM) {
                    sal_printf("%% Please don't select too many encryption algorithms\n");
                    goto err_out;
                }

                pCipherArr[nCipherArrIdx++] = argv[i];
                pCipherArr[nCipherArrIdx] = NULL;
                i++;
            }

            continue;
        }

        /* process hmac list */
        if (!sal_strcasecmp(argv[i], "-m")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(1, "%% Please specify HMAC algorithms\n");
            i++;
            while (i < argc) {
                if (NULL == _ssh_get_hmac_byname(argv[i])) {
                    break;
                }
                
                if ((nHmacArrIdx + 1) >= M_SSH_MAX_HMAC_ARR_NUM) {
                    sal_printf("%% Please don't select too many HMAC algorithms\n");
                    goto err_out;
                }

                pHmacArr[nHmacArrIdx++] = argv[i];
                pHmacArr[nHmacArrIdx] = NULL;
                i++;
            }

            continue;
        }

        /* process password prompts number */
        if (!sal_strcasecmp(argv[i], "-o")) {
            M_SSH_CLIENT_LOGIN_PARAMCHECK(2, "%% Please specify number of password prompts\n");
            if (sal_strcasecmp(argv[i + 1], "number-of-password-prompts")) {
                sal_printf("%% Please specify number of password prompts\n");
                goto err_out;
            }
            sal_snprintf(szPrompts, M_SSH_OPTIONS_BUFSIZE, "%s=%s", 
                         "NumberOfPasswordPrompts", argv[i + 2]);
            i += 3;
            continue;
        }

        /* it's host/ip and command */
        break;
    }

    /* process host/ip */
    if (i >= argc) {
        sal_printf("%% Please specify IP address or hostname of remote system\n");
        goto err_out;
    }

    if (!sal_strcasecmp(argv[i], "mgmt-if")) {
        mgmtif = 1;
        i++;
    }
    
    pszHost = argv[i];
    i++;
    if (check_ipaddress(pszHost)) {
        sal_printf("%% Invalid IPv4/IPv6 address or hostname of remote system\n");
        goto err_out;
    }

    /* process command */
    nLeftLen = M_SSH_CLIENT_CMD_BUFSIZE;
    p = szCmd;
    while (i < argc) {
        /* we don't known how to check the command string */
        nTmpLen = sal_strlen(argv[i]);
            
        if ((nTmpLen + 2) > nLeftLen) {
            /* for \0 & 1 space */
            sal_printf("%% Command string too long\n");
            goto err_out;
        }
        nPrtLen = sal_snprintf(p, nLeftLen, "%s%s",
                               ('\0' == szCmd[0]) ? "" : " ", argv[i]);
        p += nPrtLen;
        nLeftLen -= nPrtLen;
        i++;
    }

    /* check ciper array */
    for (i = 0; i < nCipherArrIdx; i++) {
        if (_ssh_get_cipher_version(pCipherArr[i]) != sal_atoi(pszVersion)) {
            sal_printf("%% Encryption algorithm %s for protocol version %s only\n",
                    pCipherArr[i], (!sal_strcmp(pszVersion, "1")) ? "2" : "1");
            goto err_out;
        }
    }

    /* check version 1 with cipher list & HMAC list */
    if (1 == sal_atoi(pszVersion)) {
        if (nHmacArrIdx > 0) {
            sal_printf("%% HMAC algorithms only supported by SSHv2\n");
            goto err_out;
        }
        if (nCipherArrIdx > 1) {
            sal_printf("%% Can only specify one encryption algorithm for protocol version 1\n");
            goto err_out;
        }
        if (nCipherArrIdx > 0) {
            sal_snprintf(szCipherBuf, M_SSH_CIPHER_BUFSIZE, 
                         "%s", pCipherArr[0]);
        }
    } else {
        /* build HMAC list */
        nLeftLen = M_SSH_HMAC_BUFSIZE;
        p = szHmacBuf;
        for (i = 0; i < nHmacArrIdx; i++) {
            char *pTmp = _ssh_get_hmac_byname(pHmacArr[i]);
            nTmpLen = sal_strlen(pTmp);
            if ((nTmpLen + 2) > nLeftLen) {
                sal_printf("%% HMAC algorithms buffer overflow\n");
                goto err_out;
            }
            nPrtLen = sal_snprintf(p, nLeftLen, "%s%s",
                    ('\0' == szHmacBuf[0]) ? "" : ",", pTmp);
            p += nPrtLen;
            nLeftLen -= nPrtLen;
        }
        
        /* build cipher list */
        nLeftLen = M_SSH_CIPHER_BUFSIZE;
        p = szCipherBuf;
        for (i = 0; i < nCipherArrIdx; i++) {
            nTmpLen = sal_strlen(pCipherArr[i]);
            if ((nTmpLen + 2) > nLeftLen) {
                sal_printf("%% Encrypt algorithms buffer overflow\n");
                goto err_out;
            }
            nPrtLen = sal_snprintf(p, nLeftLen, "%s%s",
                    ('\0' == szCipherBuf[0]) ? "" : ",", pCipherArr[i]);
            p += nPrtLen;
            nLeftLen -= nPrtLen;
        }
    }

    /* get rsa private key */
    if (NULL != pszKeyName) {
        if (_ssh_get_rsa_key(pszKeyName, szTmpName) != 0) {
            sal_printf("%% No such RSA private key error\n");
            goto err_out;
        }
        
        /* load private key */
        sshkey_load_private(szTmpName, "", &pSshKey, NULL);
        if (NULL == pSshKey) {
            sal_printf("%% Invalid RSA private key\n");
            goto err_out;
        }

        /* generate ssh key */
        if (1 == sal_atoi(pszVersion)) {
            /* generate RSA1 key for ssh */
            //sal_snprintf(szTmpName2, sizeof(szTmpName2), "%s", szTmpName);
            //pSshKey->type = KEY_RSA1;
            //if (!sshkey_save_private(pSshKey, szTmpName2, "", pszHost, 0, NULL, 0)) {
                //sal_printf("%% Generate RSA1 private key for SSH failed\n");
                //goto err_out;
            //}
        } else {
            /* generate RSA public key for ssh */
            sal_snprintf(szTmpName2, sizeof(szTmpName2), "%s.pub", szTmpName);
            sshkey_from_private(pSshKey, &pSshPubKey);
            if (pSshPubKey == NULL) {
                sal_printf("%% Extract SSH public key failed\n");
                goto err_out;
            }
            if ((fp = sal_fopen(szTmpName2, "w")) == NULL) {
                sal_printf("%% Generate SSH public key failed: %s\n",
                        sal_strerror(errno));
                goto err_out;
            }
            if (sshkey_write(pSshPubKey, fp)) {
                sal_printf("%% Generate SSH public key failed\n");
                goto err_out;
            }
            sal_fclose(fp);
            fp = NULL;
        }
    }

    /* build ssh command argument array */
    argco = 0;
    
#ifdef _KERNEL_VER_2_6_
    argvo[argco++] = "chvrf";
    if (mgmtif  == 1) {
        argvo[argco++] = "64";
    } else {
    	argvo[argco++] = "0";
    }
#else
    argvo[argco++] = "ip";
    argvo[argco++] = "netns";
    argvo[argco++] = "exec";
    if (mgmtif  == 1) {
        argvo[argco++] = "mgmt";
    } else {
        argvo[argco++] = "default";
    }
#endif
    
#if _GLB_UML_SYSTEM_
    argvo[argco++] = "/centec_switch/sbin/ssh";
#else
    argvo[argco++] = "/usr/bin/ssh";
#endif
    /* login name */
    argvo[argco++] = "-l";
    argvo[argco++] = argv[2];

    /* version */
    if (sal_strcmp(pszVersion, M_SSH_DEFAULT_PROTOCOL_VERSION)) {
        if (1 == sal_atoi(pszVersion)) {
            argvo[argco++] = "-1";
        } else {
            argvo[argco++] = "-2";
        }
    }

    /* identity file */
    if (NULL != pszKeyName) {
        argvo[argco++] = "-i";
        if (1 == sal_atoi(pszVersion)) {
            argvo[argco++] = szTmpName2;
        } else {
            argvo[argco++] = szTmpName;
        }
    }

    /* port */
    if (sal_strcmp(pszPort, M_SSH_DEFAULT_PORT)) {
        argvo[argco++] = "-p";
        argvo[argco++] = pszPort;
    }

    /* password prompts */
    if ('\0' != szPrompts[0]) {
        argvo[argco++] = "-o";
        argvo[argco++] = szPrompts;
    }

    /* Cipher list */
    if ('\0' != szCipherBuf[0]) {
        argvo[argco++] = "-c";
        argvo[argco++] = szCipherBuf;
    }

    /* HMAC list */
    if ('\0' != szHmacBuf[0] && 1 != sal_atoi(pszVersion)) {
        argvo[argco++] = "-m";
        argvo[argco++] = szHmacBuf;
    }

    /* host/ip */
    argvo[argco++] = pszHost;

    /* commands */
    if ('\0' != szCmd[0]) {
        argvo[argco++] = szCmd;
    }

#if 0 /* enable this to debug */
    argvo[argco++] = "-vvv";
#endif
    argvo[argco] = NULL;
    gen_execvp(argco, argvo);
    nRet = 0;

err_out:
    sal_unlink(szTmpName);
    sal_unlink(szTmpName2);
    if (pSshPubKey) {
        sshkey_free(pSshPubKey);
    }
    if (pSshKey) {
        sshkey_free(pSshKey);
    }
    if (fp) {
        sal_fclose(fp);
    }
    return nRet;
}


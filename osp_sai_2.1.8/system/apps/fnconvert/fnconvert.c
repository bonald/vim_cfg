/****************************************************************************
* $Id$
*  file name convert
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-09-17 08:47
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <assert.h>
#include <sshkey.h>
#include <authfile.h>
#if (defined  _CTC_X86_64_) || (defined HAVE_DEBIAN)
#include <shadow.h>
#endif

#include "lib_fs.h"
#include "sal.h"
#include "rsa_derhex.h"
#include "proto.h"
#include "cdb_const.h"
#include "tftp.h"
#include "ftp.h"
#include "xmodem.h"
#include "fn_ssh.h"
#include "scp.h"
#include "fnconvert.h"

#define M_MSG_BUFSZ     1024
/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#ifdef _GLB_UML_SYSTEM_
extern int
read_keyfile_line(FILE *f, const char *filename, char *buf, size_t bufsz,
   u_long *lineno);
#else
/*
 * Borrowed from OpenSSH misc.c
 * Read an entire line from a public key file into a static buffer, discarding
 * lines that exceed the buffer size.  Returns 0 on success, -1 on failure.
 */
int
read_keyfile_line(FILE *f, const char *filename, char *buf, size_t bufsz,
   u_long *lineno)
{
	while (fgets(buf, bufsz, f) != NULL) {
		if (buf[0] == '\0')
			continue;
		(*lineno)++;
		if (buf[strlen(buf) - 1] == '\n' || feof(f)) {
			return 0;
		} else {
			/* discard remainder of line */
			while (fgetc(f) != '\n' && !feof(f))
				;	/* nothing */
		}
	}
	return -1;
}
#endif

int get_security_crt_public_keyline(char *filename, char *keyline);
int get_secure_shell_public_keyline(char *filename, char *keyline);

typedef int (*LOAD_KEY_CB) (char *filename, char *keyline);

#define GEN_KEY_TYPE 3
#define SSH_MAX_PUBKEY_BYTES		8192

struct load_key_callback {
	int genkeytype;
	char *genkeyname;
	LOAD_KEY_CB load_public_key_cb;
};

struct load_key_callback g_stloadkeycb[] = {
	{0, "OpenSSH", NULL},
	{1, "Security CRT", get_security_crt_public_keyline},
	{2, "Secure shell", get_secure_shell_public_keyline}
};

char *g_pszOutFilename = NULL;
char *g_pszKeyFmt = NULL;
static char *g_pszUrlAction = NULL;
static char *g_pszUrl = NULL;

static char *g_pszUser = NULL;

/* operation array for xmodem */
network_op_t g_cdbctl_xmodem_ops = 
{
    .init                   = NULL,
    .release                = NULL,
    .parseurl               = gen_xmodem_op_parseurl,
    .interactive_get_param  = gen_xmodem_op_interactive_get_param,
    .append_dir             = gen_xmodem_op_append_dir,
    .sendto                 = gen_xmodem_op_sendto,
    .recvfrom               = gen_xmodem_op_recvfrom
};

/* operation array for FTP */
network_op_t g_cdbctl_ftp_ops = 
{
    .init                   = gen_ftp_op_init,
    .release                = gen_ftp_op_release,
    .parseurl               = gen_ftp_op_parseurl,
    .interactive_get_param  = gen_ftp_op_interactive_get_param,
    .append_dir             = gen_ftp_op_append_dir,
    .sendto                 = gen_ftp_op_sendto,
    .recvfrom               = gen_ftp_op_recvfrom
};

/* operation array for TFTP */
network_op_t g_cdbctl_tftp_ops = 
{
    .init                   = gen_tftp_op_init,
    .release                = gen_tftp_op_release,
    .parseurl               = gen_tftp_op_parseurl,
    .interactive_get_param  = gen_tftp_op_interactive_get_param,
    .append_dir             = gen_tftp_op_append_dir,
    .sendto                 = gen_tftp_op_sendto,
    .recvfrom               = gen_tftp_op_recvfrom
};

/* operation array for SCP */
network_op_t g_cdbctl_scp_ops = 
{
    .init                   = gen_scp_op_init,
    .release                = gen_scp_op_release,
    .parseurl               = gen_scp_op_parseurl,
    .interactive_get_param  = gen_scp_op_interactive_get_param,
    .append_dir             = gen_scp_op_append_dir,
    .sendto                 = gen_scp_op_sendto,
    .recvfrom               = gen_scp_op_recvfrom
};

/****************************************************************************
 *
* Function
* 
****************************************************************************/
void usage()
{
	printf("Usage: fnconvert -c cmd parameters...\r\n");
	printf("  cmd options:\r\n");
	printf
		("    showname       convert to show name, eg /mnt/flash/xx --> flash:/xx\r\n");
	printf
		("    acccessname    convert to acccess name, eg flash:/xx --> /mnt/flash/xx\r\n");
	printf("    fileprocconfirm overwrite confirm\r\n");
	printf("    isbinfile      check binary file\r\n");
	printf("    confirm        confirm action (default yes)\r\n");
	printf("    yesno_no       confirm yes or no (default no)\r\n");
	printf("    rsa            convert RSA key \r\n");
	printf("    import         import RSA key \r\n");
	printf("    url            Send/Recv file by URL\r\n");
	printf("    filecopy       Copy file with progress\r\n");
	printf("    encoding       Do string encoding (modified base 64)\r\n");
	printf("    decoding       Do string decoding (modified base 64)\r\n");
	printf("    ssh            SSH client login\r\n");
	printf("  parameters:\r\n");
	printf("    -p    path/filename for check, convert, rsa, import, url, filecopy\r\n");
	printf("    -a    action for url (send, recv)\r\n");
	printf("    -u    URL for send/recv\r\n");
	printf("    -m    msg for overwrite or confirm or encoding/decoding, SSH command line string\r\n");
	printf("    -t    format for convert or import (pem, der, der-hex, ssh1, ssh2)\r\n");
	printf("    -h    print this help message\r\n");
	printf("    -o    Output file (for rsa, filecopy etc)\r\n");
	printf("    -f    Key format (for rsa, public or private)\r\n");
	printf("    -U    User name (for login)\r\n");
	printf("\r\n");
	printf
		("    RSA command environment: RSA_KEYNAME, RSA_ENC, RSA_PASSWORD\r\n");
	printf("\r\n");
	exit(2);
}

fnc_cmd_e parse_cmd_arg(const char *pszCmd)
{
	fnc_cmd_e cmd;

	if (!strcasecmp(pszCmd, "showname")) {
		cmd = E_FNC_CMD_TO_SHOWNAME;
	} else if (!strcasecmp(pszCmd, "accessname")) {
		cmd = E_FNC_CMD_TO_ACCESSNAME;
	} else if (!strcasecmp(pszCmd, "fileprocconfirm")) {
		cmd = E_FNC_CMD_FILE_PROC_CONFIRM;
	} else if (!strcasecmp(pszCmd, "isbinfile")) {
		cmd = E_FNC_CMD_ISBINFILE;
	} else if (!strcasecmp(pszCmd, "confirm")) {
		cmd = E_FNC_CMD_CONFIRM;
	} else if (!strcasecmp(pszCmd, "yesno_no")) {
		cmd = E_FNC_CMD_YESNO_NO;
	} else if (!strcasecmp(pszCmd, "rsa")) {
		cmd = E_FNC_CMD_CONVERT_RSA_KEY;
	} else if (!strcasecmp(pszCmd, "import")) {
		cmd = E_FNC_CMD_IMPORT_RSA_KEY;
	} else if (!strcasecmp(pszCmd, "url")) {
		cmd = E_FNC_CMD_URL;
	} else if (!strcasecmp(pszCmd, "filecopy")) {
		cmd = E_FNC_CMD_FILE_COPY;
	} else if (!strcasecmp(pszCmd, "encoding")) {
		cmd = E_FNC_CMD_B64_NTOP;
	} else if (!strcasecmp(pszCmd, "decoding")) {
		cmd = E_FNC_CMD_B64_PTON;
	} else if (!strcasecmp(pszCmd, "login")) {
		cmd = E_FNC_CMD_LOGIN;
	} else if (!strcasecmp(pszCmd, "ssh")) {
		cmd = E_FNC_CMD_SSH;
	} else if (!strcasecmp(pszCmd, "encrypt")) {
		cmd = E_FNC_CMD_ENCRYPT;
	} else if (!strcasecmp(pszCmd, "decrypt")) { /* djd add for decrypt password*/
		cmd = E_FNC_CMD_DECRYPT;
	} else {
		cmd = E_FNC_CMD_NONE;
	}

	return cmd;
}

int __check_filesystem_exist(char *pszPath) 
{
    int64_t llsize = 0ll;
    
    if(!strncmp(pszPath, "/mnt/udisk", strlen("/mnt/udisk")))
    {
        llsize = check_disk_free_space("/mnt/udisk");
    }

    if(!strncmp(pszPath, "udisk:", strlen("udisk:")))
    {
        llsize += check_disk_free_space("/mnt/udisk");
    }

    if(llsize < 0) {
        return -1;
    }
    
    return 0;
}

int cmd_arg_validate(fnc_cmd_e cmd, char *pszMsg, char *pszPath)
{
	if (E_FNC_CMD_NONE == cmd) {
		return -1;
	}
	/* check path */
	switch (cmd) {
	case E_FNC_CMD_TO_SHOWNAME:
	case E_FNC_CMD_TO_ACCESSNAME:
	case E_FNC_CMD_ISBINFILE:
	case E_FNC_CMD_FILE_PROC_CONFIRM:
	case E_FNC_CMD_CONVERT_RSA_KEY:
	case E_FNC_CMD_IMPORT_RSA_KEY:
	case E_FNC_CMD_URL:
	case E_FNC_CMD_FILE_COPY:
		if ('\0' == *pszPath) {
			return -1;
		}
		break;
	case E_FNC_CMD_YESNO_NO:
	case E_FNC_CMD_CONFIRM:
	case E_FNC_CMD_B64_NTOP:
	case E_FNC_CMD_B64_PTON:
	case E_FNC_CMD_SSH:
	case E_FNC_CMD_ENCRYPT:
	case E_FNC_CMD_DECRYPT: /* djd */
		break;
	case E_FNC_CMD_LOGIN:
	    if (NULL == g_pszUser) {
            return -1;
	    }
	    break;
	default:
		break;
	}

	/* check msg */
	switch (cmd) {
	case E_FNC_CMD_TO_SHOWNAME:
	case E_FNC_CMD_TO_ACCESSNAME:
	case E_FNC_CMD_ISBINFILE:
	case E_FNC_CMD_CONVERT_RSA_KEY:
	case E_FNC_CMD_IMPORT_RSA_KEY:
	case E_FNC_CMD_URL:
	case E_FNC_CMD_FILE_COPY:
		break;
	case E_FNC_CMD_YESNO_NO:
	case E_FNC_CMD_CONFIRM:
	case E_FNC_CMD_FILE_PROC_CONFIRM:
	case E_FNC_CMD_B64_NTOP:
	case E_FNC_CMD_B64_PTON:
	case E_FNC_CMD_SSH:
	case E_FNC_CMD_ENCRYPT:
	case E_FNC_CMD_DECRYPT: /* djd */
		if ('\0' == *pszMsg) {
			return -1;
		}
		break;
	default:
		break;
	}

	return 0;
}

int do_file_proc_confirm(const char *pszMsg, const char *pszPath)
{
	int ret;
	char szOutput[PATH_MAX];

	ret = gen_check_and_get_filename((char *) pszPath, szOutput, PATH_MAX);
	if (0 != ret) {
		ret =
			gen_check_and_get_filename_bycwd((char *) pszPath, szOutput,
											 PATH_MAX);
	}

	if (0 != ret) {
		fprintf(stderr, "%% Check failed: invalid path: %s\r\n", pszPath);
		return -1;
	}

	return gen_file_proc_confirm((char *) pszMsg, szOutput, (char *) pszPath);
}

int do_filename_convert(fnc_cmd_e cmd, char *pszPath)
{
	char szOutput[PATH_MAX];
	int ret;

	assert(E_FNC_CMD_TO_SHOWNAME == cmd || E_FNC_CMD_TO_ACCESSNAME == cmd);

	szOutput[0] = '\0';
	if (E_FNC_CMD_TO_SHOWNAME == cmd) {
        if (0 == sal_strcmp(FULL_NAME_MNT, pszPath))
        {
            sal_strncpy(szOutput, FULL_NAME_MNT, PATH_MAX);
            ret = 0;
        }
        else
        {
            ret = gen_check_and_gen_showname((char *) pszPath, szOutput);
        }
	} else {
		ret =
			gen_check_and_get_filename((char *) pszPath, szOutput, PATH_MAX);
		if (0 != ret) {
			ret =
				gen_check_and_get_filename_bycwd((char *) pszPath, szOutput,
												 PATH_MAX);
		}
	}

	if (0 != ret) {
		fprintf(stderr, "%% Convert failed: invalid path: %s\r\n", pszPath);
		return -1;
	}

	printf("%s", szOutput);
	return 0;
}

static char *_get_otype_name(key_type_e otype)
{
	switch (otype) {
	case E_TYPE_PEM:
		return "pem";
	case E_TYPE_DER:
		return "der";
	case E_TYPE_DERHEX:
		return "der-hex";
	//case E_TYPE_SSH1:
		//return "ssh1";
	case E_TYPE_SSH2:
		return "ssh2";
	default:
		break;
	}
	return "unknown";
}

int get_security_crt_public_keyline(char *filename, char *keyline)
{
	FILE *f;
	char *cp;
	u_long linenum = 0;
	char tmpline[SSH_MAX_PUBKEY_BYTES];
	int charnum;
	char header[] = "---- BEGIN SSH2 PUBLIC KEY";

	cp = keyline;
	f = fopen(filename, "r");
	if (f != NULL) {
		/*File header */
		if (read_keyfile_line(f, filename, tmpline, sizeof(tmpline),
							  &linenum) != -1) {
			if (strncmp(tmpline, header, strlen(header)) != 0) {
				fclose(f);
				return 0;
			}
		}

		while (read_keyfile_line(f, filename, tmpline, sizeof(tmpline),
								 &linenum) != -1) {
			if (strncmp(tmpline, "Subject:", 8) == 0) {
				continue;
			}
			if (strncmp(tmpline, "Comment:", 8) == 0) {
				continue;
			}
			if (strncmp(tmpline, "ModBitSize:", 11) == 0) {
				continue;
			}
			if (strncmp(tmpline, "---- END", 8) == 0) {
				*cp = '\n';
				break;
			}
			if (strncmp(tmpline, "----", 4) == 0) {
				continue;
			}

			charnum = strlen(tmpline);
			strcpy(cp, tmpline);
			cp = cp + charnum - 1;	/*delete '\n' */
		}
		fclose(f);
		return 1;
	}

	return 0;
}

int get_secure_shell_public_keyline(char *filename, char *keyline)
{
	FILE *f;
	char *cp;
	u_long linenum = 0;
	char tmpline[SSH_MAX_PUBKEY_BYTES];
	int charnum;
	char header[] = "---- BEGIN SSH2 PUBLIC KEY";

	cp = keyline;
	f = fopen(filename, "r");
	if (f != NULL) {
		/*File header */
		if (read_keyfile_line(f, filename, tmpline, sizeof(tmpline),
							  &linenum) != -1) {
			if (strncmp(tmpline, header, strlen(header)) != 0) {
				fclose(f);
				return 0;
			}
		}

		while (read_keyfile_line(f, filename, tmpline, sizeof(tmpline),
								 &linenum) != -1) {
			if (strncmp(tmpline, "Comment:", 8) == 0) {
				continue;
			}
			if (strncmp(tmpline, "---- END", 8) == 0) {
				*cp = '\n';
				break;
			}
			if (strncmp(tmpline, "----", 4) == 0) {
				continue;
			}

			charnum = strlen(tmpline);
			strcpy(cp, tmpline);
			cp = cp + charnum - 1;	/*delete '\n' */
		}
		fclose(f);
		return 1;
	}

	return 0;
}

int get_public_file_keyline(char *pszSrcName, char *keyline)
{
	int loop;
	int ret;

	for (loop = 0; loop < GEN_KEY_TYPE; loop++) {
		if (*(g_stloadkeycb[loop].load_public_key_cb) != NULL) {
			ret =
				(*(g_stloadkeycb[loop].load_public_key_cb)) (pszSrcName,
															 keyline);
			if (ret == 1) {
				return 1;
			}
		}
	}
	return 0;
}

int create_tmp_public_file(char *pszSrcName, char *keyline)
{
	FILE *f = NULL;

	f = fopen(pszSrcName, "w");
	if (f == NULL) {
		return -1;
	}
	fprintf(f, "ssh-rsa ");
	fprintf(f, "%s", keyline);

	fclose(f);

	return 0;
}

struct sshkey *ssh_load_standard_public_key(char *filename)
{
	char szTmpName[] = "/tmp/pkey123465";
	char keyline[SSH_MAX_PUBKEY_BYTES];
	struct sshkey *pSshPubKey = NULL;

	/*Get key */
	if (get_public_file_keyline(filename, keyline) == 0) {
		return NULL;
	}

	/*Write public key file */
	if (create_tmp_public_file(szTmpName, keyline)) {
		unlink(szTmpName);
		return NULL;
	}
	sshkey_load_public(szTmpName, &pSshPubKey, NULL);
	unlink(szTmpName);
	return pSshPubKey;
}

int
rsa_load_ssh_file(char *pszFilename, int bPublicKey, int bSshv2,
				  char *pszPassword, RSA ** ppRsa, char *pszErr,
				  size_t err_bufsz)
{
	int nRet = 0;
	struct sshkey *pSshKey = NULL;
	struct stat st;
	int bLoadAsPrivateKey = 1;
    char sys_cmd[256];
	if (NULL == pszErr || NULL == pszFilename) {
		return -1;
	}
    sal_memset(sys_cmd,0,sizeof(sys_cmd));
	/* load as private key */
    if (bPublicKey) {
        stat(pszFilename, &st);
        if ((st.st_uid == getuid()) && (st.st_mode & 077) != 0) {
            bLoadAsPrivateKey = 0;
        }
    }
    if (bLoadAsPrivateKey) {
        sprintf(sys_cmd,"chmod 600 %s",pszFilename);
        system(sys_cmd);
        sshkey_load_private(pszFilename,
            (NULL == pszPassword) ? "" : (const char *)pszPassword, &pSshKey, NULL);
    }
	if (NULL == pSshKey && bPublicKey) {
		/* load as publick key */
		sshkey_load_public(pszFilename, &pSshKey, NULL);
		if (NULL == pSshKey) {
			pSshKey = ssh_load_standard_public_key(pszFilename);
		}
	}

	if (NULL == pSshKey) {
		snprintf(pszErr, err_bufsz, "Load SSH key file failed, the passphrase missing or the file format error");
		goto err_out;
	}

	if (KEY_DSA == pSshKey->type) {
		snprintf(pszErr, err_bufsz, "Not supported DSA key error");
		goto err_out;
	}

	/*if (!bSshv2 && KEY_RSA1 != pSshKey->type) {
		snprintf(pszErr, err_bufsz, "It's SSHv2 key error");
		goto err_out;
	}*/

	if (bSshv2 && KEY_RSA != pSshKey->type) {
		snprintf(pszErr, err_bufsz, "It's SSHv1 key error");
		goto err_out;
	}

	if (rsa_key_bits_check(pSshKey->rsa, pszErr, err_bufsz) != 0) {
		goto err_out;
	}

	if (ppRsa) {
		*ppRsa = pSshKey->rsa;
		pSshKey->rsa = NULL;
	}

	nRet = 0;

  err_out:
	if (pSshKey) {
		sshkey_free(pSshKey);
	}
	return nRet;
}


int
rsa_convert_ssh(RSA * pRsa, int bExportPrivate, int bSshv2,
				char *pszEnc, char *pszPassword, char *pszOut,
				size_t out_bufsz, char *pszErr, size_t err_bufsz,
				size_t * pOutBytes)
{
	char szLine[1024];
	char szFileName[] = "/tmp/rsasshXXXXXX";
	int nRet = -1;
	int nSize = 0;
	struct sshkey stkey;
	FILE *fp = NULL;
	int nTmp;
	int fd;

	if (NULL == pRsa || NULL == pszErr || NULL == pszOut) {
		return -1;
	}

	if ((fd = mkstemp(szFileName)) < 0) {
		snprintf(pszErr, err_bufsz, "Create temporary file failed: %s",
				 strerror(errno));
		goto err_out;
	}
	close(fd);

	memset(&stkey, 0x00, sizeof(struct sshkey));
	stkey.rsa = pRsa;
	//stkey.type = (bSshv2 ? KEY_RSA : KEY_RSA1);
	stkey.type = KEY_RSA;
	if (!bExportPrivate) {
		if (NULL == (fp = fopen(szFileName, "w"))) {
			snprintf(pszErr, err_bufsz, "Write temporary file failed: %s",
					 strerror(errno));
			goto err_out;
		}
		nTmp = sshkey_write(&stkey, fp);
		fclose(fp);
		fp = NULL;
	} else {
		if (NULL == pszPassword) {
			pszPassword = "";
		}
		nTmp = sshkey_save_private(&stkey, szFileName, pszPassword, "", 0, NULL, 0);
	}
	if (0 != nTmp) {
		snprintf(pszErr, err_bufsz, "Convert RSA key to %s format failed",
				 bSshv2 ? "SSHv2" : "SSHv1");
		goto err_out;
	}
	if (NULL == (fp = fopen(szFileName, "r"))) {
		snprintf(pszErr, err_bufsz, "Read temporary file failed: %s",
				 strerror(errno));
		goto err_out;
	}

	nTmp = 0;
	while ((nSize = fread(szLine, 1, 1024, fp)) > 0) {
		if ((nTmp + nSize) >= out_bufsz) {
			snprintf(pszErr, err_bufsz, "output buffer too small");
			goto err_out;
		}

		memcpy(pszOut + nTmp, szLine, nSize);
		nTmp += nSize;
	}
	pszOut[nTmp] = '\0';

	if (pOutBytes) {
		*pOutBytes = nTmp;
	}
	nRet = 0;

  err_out:
	if (NULL != fp) {
		fclose(fp);
	}
	unlink(szFileName);
	return nRet;
}

static int
_convert_rsa_key(RSA * pRsa, int bPrivate,
				 key_type_e otype, char *pszErr, size_t err_bufsz)
{
	char *pszOut = NULL;
	char *pszPasswd = getenv("RSA_PASSWD");
	char *pszEnc = getenv("RSA_ENC");
	int ret = -1;
	int nResult = -1;
	size_t nDestBytes = 0;

	if (NULL == (pszOut = malloc(M_RSA_KEYSTR_BUFSZ))) {
		snprintf(pszErr, err_bufsz, "Out of memory");
		goto err_out;
	}

	if (!strcasecmp(g_pszKeyFmt, "private")) {
		//if (E_TYPE_SSH1 == otype || E_TYPE_SSH2 == otype) {
		if ( E_TYPE_SSH2 == otype) {
			nResult =
				rsa_convert_ssh(pRsa, 1, (E_TYPE_SSH2 == otype), pszEnc,
								pszPasswd, pszOut, M_RSA_KEYSTR_BUFSZ, pszErr,
								err_bufsz, &nDestBytes);
		} else {
			nResult =
				rsa_pri_convert_format(pRsa, _get_otype_name(otype), pszEnc,
									   pszPasswd, pszOut, M_RSA_KEYSTR_BUFSZ,
									   pszErr, err_bufsz, &nDestBytes);
		}
	} else {
		if ( E_TYPE_SSH2 == otype) {
			nResult =
				rsa_convert_ssh(pRsa, 0, (E_TYPE_SSH2 == otype), pszEnc,
								pszPasswd, pszOut, M_RSA_KEYSTR_BUFSZ, pszErr,
								err_bufsz, &nDestBytes);
		} else {
			nResult =
				rsa_pub_convert_format(pRsa, _get_otype_name(otype), pszOut,
									   M_RSA_KEYSTR_BUFSZ, pszErr, err_bufsz,
									   &nDestBytes);
		}
	}
	if (0 != nResult) {
		goto err_out;
	}

	if (g_pszOutFilename) {
		FILE *fp = fopen(g_pszOutFilename, "w");

		if (NULL == fp) {
			snprintf(pszErr, err_bufsz, "open output file failed: %s",
					 strerror(errno));
			goto err_out;
		}
		fwrite(pszOut, nDestBytes, 1, fp);
		fclose(fp);
		chmod(g_pszOutFilename, S_IRUSR | S_IWUSR);
	} else {
		fwrite(pszOut, nDestBytes, 1, stdout);
	}

    ret = 0;

  err_out:
	if (NULL != pszOut) {
		free(pszOut);
	}
	return ret;
}

int do_rsa_convert(const char *pszMsg, const char *pszPath, key_type_e otype)
{
	char szErr[256];
	char szPasswd[PASSWD_SIZE];
	char *pszLine = NULL;
	int ret = -1;
	FILE *fp = NULL;
	char *pszField;
	char *pszKey;
	RSA *pRsa = NULL;
	int bPrivate = 0;

	if (NULL == (pszLine = malloc(M_RSA_KEYSTR_BUFSZ * 3))) {
		fprintf(stderr, "%% %s: out of memory\r\n", pszMsg);
		goto err_out;
	}

	pszKey = pszLine + M_RSA_KEYSTR_BUFSZ;
	pszField = pszLine + M_RSA_KEYSTR_BUFSZ * 2;
	if (NULL == (fp = fopen(pszPath, "r"))) {
		fprintf(stderr, "%% Open RSA key file failed: %s\r\n",
				strerror(errno));
		goto err_out;
	}
	while (fgets(pszLine, M_RSA_KEYSTR_BUFSZ, fp)) {
		/* process password */
		szPasswd[0] = '\0';
		if (sal_cmd_get_token(pszLine, "passwd", szPasswd, PASSWD_SIZE) != 0) {
			fprintf(stderr, "%% Invalid RSA key file\r\n");
			goto err_out;
		}
		if ('\0' == szPasswd[0]) {
			if (sal_cmd_get_token
				(pszLine, "enc_passwd", pszField, M_RSA_KEYSTR_BUFSZ) != 0) {
				fprintf(stderr, "%% Invalid RSA key file\r\n");
				goto err_out;
			}
			if ('\0' != pszField[0]) {
				if (do_decrypt(M_DEFAULT_PW_STR, pszField, strlen(pszField),
							   szPasswd, PASSWD_SIZE) != 0) {
					fprintf(stderr, "%% Invalid encrypt password\r\n");
					goto err_out;
				}
			}
		}

		/* check rsa key type */
		if (sal_cmd_get_token(pszLine, "type", pszField, M_RSA_KEYSTR_BUFSZ)
			!= 0) {
			fprintf(stderr, "%% Invalid RSA key file\r\n");
			goto err_out;
		}
		bPrivate = (!strcasecmp(pszField, "private"));

		if (!strcasecmp(g_pszKeyFmt, "private") && !bPrivate) {
			fprintf(stderr, "%% Specified key is public key error\r\n");
			goto err_out;
		}

		/* get RSA key */
		if (sal_cmd_get_token(pszLine, "pem", pszField, M_RSA_KEYSTR_BUFSZ) !=
			0) {
			fprintf(stderr,
					"%% Invalid file: please use cdbctl to generate it\r\n");
			goto err_out;
		}
		if (sal_b64_pton(pszField, (unsigned char *)pszKey, M_RSA_KEYSTR_BUFSZ) == -1) {
			fprintf(stderr, "%% Invalid file: unknown pem encoding\r\n");
			goto err_out;
		}
		if (validate_rsa_key_pem
			(szPasswd, pszKey, strlen(pszKey), bPrivate, &pRsa, szErr,
			 256) != 0) {
			fprintf(stderr, "%% %s\r\n", szErr);
			goto err_out;
		}

		if (_convert_rsa_key(pRsa, bPrivate, otype, szErr, 256) != 0) {
			fprintf(stderr, "%% %s\r\n", szErr);
			goto err_out;
		}

		RSA_free(pRsa);
		pRsa = NULL;
	}

	ret = 0;

  err_out:
	if (NULL != fp) {
		fclose(fp);
	}
	if (NULL != pszLine) {
		free(pszLine);
	}
	if (NULL != pRsa) {
		RSA_free(pRsa);
	}
	return ret;
}

int _import_rsa_key(RSA * pRsa, int bPublicKey,
					char *pszPassword, char *pszErr, size_t err_bufsz)
{
	char *pszKeyname = getenv("RSA_KEYNAME");
	int ret = -1;
	char *pszBuf = NULL;
	char *pszField = NULL;
	int nLen = 0;

	if (NULL == pszKeyname) {
		snprintf(pszErr, err_bufsz, "Please setting env RSA_KEYNAME");
		goto err_out;
	}

	if (NULL == (pszBuf = malloc(M_RSA_KEYSTR_BUFSZ * 2))) {
		snprintf(pszErr, err_bufsz, "out of memory");
		goto err_out;
	}

	pszField = pszBuf + M_RSA_KEYSTR_BUFSZ;
	if (bPublicKey) {
		if (rsa_pub_convert_format(pRsa, "pem", pszField, M_RSA_KEYSTR_BUFSZ,
								   pszErr, err_bufsz, NULL) != 0) {
			goto err_out;
		}
	} else {
		if (rsa_pri_convert_format(pRsa, "pem", NULL, pszPassword,
								   pszField, M_RSA_KEYSTR_BUFSZ, pszErr,
								   err_bufsz, NULL) != 0) {
			goto err_out;
		}

	}
	if (sal_b64_ntop((unsigned char *)pszField, strlen(pszField),
					 pszBuf, M_RSA_KEYSTR_BUFSZ) == -1) {
		snprintf(pszErr, err_bufsz, "Encoding pem string failed");
		return -1;
	}

	nLen = snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen,
					"cdbctl create/cdb/app/rsa/%s", pszKeyname);
	nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen,
					 "/type/%s", bPublicKey ? "public" : "private");

	if (pszPassword && '\0' != pszPassword[0] && strcasecmp(pszPassword, "")) {
		nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen,
						 "/passwd/%s", pszPassword);
	}

	nLen += snprintf(pszField + nLen, M_RSA_KEYSTR_BUFSZ - nLen,
					 "/pem/%s", pszBuf);

	pszBuf[0] = '\0';
	if (sal_cmd_exec(pszBuf, M_RSA_KEYSTR_BUFSZ, pszField) != 0) {
		if ('\0' != pszBuf[0]) {
			snprintf(pszErr, err_bufsz, "%s", pszBuf);
		} else {
			snprintf(pszErr, err_bufsz, "save RSA key failed");
		}
		goto err_out;
	}

	ret = 0;

  err_out:
	if (pszBuf) {
		free(pszBuf);
	}
	return ret;
}

int do_rsa_import(const char *pszMsg, char *pszPath, key_type_e otype)
{
	char *pszPassword = getenv("RSA_PASSWD");
	char szErr[256];
	int ret = -1;
	RSA *pRsa = NULL;
	int nTmp = -1;

	/* load key file */
	if (E_TYPE_SSH2 == otype) {
		nTmp = rsa_load_ssh_file(pszPath, !strcasecmp(g_pszKeyFmt, "public"),
								 (E_TYPE_SSH2 == otype), pszPassword, &pRsa,
								 szErr, 256);
	} else {
		if (E_TYPE_PEM == otype) {
			nTmp =
				rsa_load_pem_file(pszPath, !strcasecmp(g_pszKeyFmt, "public"),
								  pszPassword, &pRsa, szErr, 256);
		} else if (E_TYPE_DER == otype) {
			nTmp =
				rsa_load_der_file(pszPath, !strcasecmp(g_pszKeyFmt, "public"),
								  &pRsa, szErr, 256);
		} else if (E_TYPE_DERHEX == otype) {
			nTmp =
				rsa_load_derhex_file(pszPath,
									 !strcasecmp(g_pszKeyFmt, "public"),
									 &pRsa, szErr, 256);
		} else {
			fprintf(stderr, "%% Unsupportted key type %d\r\n", otype);
			goto err_out;
		}
	}
	if (0 != nTmp) {
		fprintf(stderr, "%% %s\r\n", szErr);
		goto err_out;
	}

	/* import RSA key to cdb database */
	if (_import_rsa_key(pRsa, !strcasecmp(g_pszKeyFmt, "public"),
						pszPassword, szErr, 256) != 0) {
		fprintf(stderr, "%% %s\r\n", szErr);
		goto err_out;
	}

	ret = 0;

  err_out:
	if (NULL != pRsa) {
		RSA_free(pRsa);
	}
	return ret;
}


int do_url_proc(char *pszPath)
{
    char *pszVrf = getenv("CUR_VRF");
    char szErr[256];
    int ret = -1;
    int nTmp = 0;

    if (NULL == pszVrf) {
        pszVrf = MANAGEMENT_VRF_NAME;
    }

    /* for bug 47539, by tongzb ,2018-06-07 */
    sal_memset(szErr,0,sizeof(szErr));
    if (!strcasecmp(g_pszUrlAction, "send")) {
        nTmp = gen_send_to_url(pszPath, g_pszUrl, pszVrf);
    } else {
        nTmp = gen_recv_from_url(szErr, 256, pszPath, g_pszUrl, pszVrf);
        if (0 != nTmp) {
            fprintf(stderr,szErr);
        }
    }
    
    ret = nTmp;
    
//err_out:
    return ret;
}

int do_b64_encdec(fnc_cmd_e cmd, char *pszMsg)
{
    unsigned char *pszBuf = NULL;
    int ret;
    int bufsz;

    if (E_FNC_CMD_B64_PTON == cmd) {
        bufsz = strlen(pszMsg) + 1;
    } else {
        bufsz = 2 * strlen(pszMsg) + 1;
    }

    if (bufsz < 128) {
        bufsz = 128;
    }

    pszBuf = malloc(bufsz);
    if (NULL == pszBuf) {
        fprintf(stderr, "%% Encoding/decoding: out of memory");
        return -1;
    }

    if (E_FNC_CMD_B64_PTON == cmd) {
        ret = sal_b64_pton(pszMsg, pszBuf, bufsz);
    } else {
        ret = sal_b64_ntop((unsigned char *)pszMsg, strlen(pszMsg), (char *)pszBuf, bufsz);
    }
    if (-1 != ret) {
        fwrite(pszBuf, strlen((char *)pszBuf), 1, stdout); 
    }

    if (NULL != pszBuf) {
        free(pszBuf);
    }
    
    return (-1 == ret) ? -1 : 0;
}


int do_login(void)
{
#ifndef _GLB_UML_SYSTEM_
#if (defined  _CTC_X86_64_) || (defined HAVE_DEBIAN)
    struct spwd *sp = NULL;
    char *pszPin = NULL;
    char *pszEnc = NULL;

    if (NULL == g_pszUser 
            || NULL == (sp = getspnam(g_pszUser))) {
        fprintf(stderr, "%% No such user: %s\n", g_pszUser);
        return -1;
    }

    pszPin = getpass("Password: ");
    pszEnc = crypt(pszPin, sp->sp_pwdp);
    if (NULL == pszEnc || 0 != sal_strcmp(pszEnc, sp->sp_pwdp)) {
        fprintf(stderr, "%% Authentication failed\n");
        return -1;
    }
#else
    struct passwd *pw = NULL;
    char *pszPin = NULL;
    char *pszEnc = NULL;   
    
    if (NULL == g_pszUser
            || NULL == (pw = getpwnam(g_pszUser))) {
        fprintf(stderr, "%% No such user: %s\n", g_pszUser);
        return -1;
    }

    pszPin = getpass("Password: ");
    pszEnc = crypt(pszPin, pw->pw_passwd);
    if (NULL == pszEnc || 0 != sal_strcmp(pszEnc, pw->pw_passwd)) {
        fprintf(stderr, "%% Authentication failed\n");
        return -1;
    }
#endif
#endif
    return 0;
}

/* Added by liuht for bug 33608, 2015-06-02 */
int do_filecopy(char *src_file, char *dst_file)
{
    int ret = 0;
    int need_progress = 1;

    if (NULL != getenv("NO_NEED_PROGRESS")) {
        need_progress = 0;
    }

    ret = check_space_available(src_file, dst_file, 0);
    if (ret == -1)
    {
        fprintf(stderr, "%% Get free space on device failed.\n");
    }
    else if (ret == -2)
    {
        fprintf(stderr, "%% Open temporary file failed\n");
    }
    else if (ret == -3)
    {
        fprintf(stderr, "%% No enough space left on device.\n");
    }

    if(ret < 0)
        return -1;

    if ((ret = copy_file_with_progress(src_file, dst_file, need_progress)) < 0) {
        if (-3 == ret) {
            fprintf(stderr, "%s%% There is not enough memory.\n", need_progress ? "\n" : "");
        } else if (-5 == ret) {
            fprintf(stderr, "%s%% Interrupted by user\n", need_progress ? "\n" : "");
        } else if (-6 == ret) {
            fprintf(stderr, "%s%% Read operation abnormal accomplished\n", need_progress ? "\n" : "");
        } else {
            fprintf(stderr, "%s%% Copy file failed: %s\n", need_progress ? "\n" : "",
                    sal_strerror(errno));
        }
        ret = -1;
    } else {
        ret = 0;
    }

    return ret;
}

int32
check_string(char *pin)
{
    char *p = pin;
    
    if (NULL == pin)
    {
        return -1;
    }
    
    if (sal_strlen(pin) >= 128)
    {
        return -1;
    }

    while ('\0' != *p)
    {
        if (!sal_isprint(*p)) 
        {
            return -1;
        }

        p++;
    }
    return 0;
}

int do_string_encrypt(char *pszString)
{
    char szBuf[M_PASSWORD_BUFSIZE];
    char passwd[M_PASSWORD_BUFSIZE];
    char *p = pszString;
    char *q = passwd;
    
    if (check_string(pszString) != 0)
    {
        return -1;
    }

    /* Added by kcao for bug 45756, bypass '\' char */
    sal_memset(passwd, 0, sizeof(passwd));
    while ('\0' != *p)
    {
        if ('\\' != *p)
        {
            *q = *p;
            p++;
            q++;
        }
        else
        {
            p++;
        }
    }
    *q = '\0';

    if (do_encrypt(M_DEFAULT_PW_STR, passwd, strlen(passwd),
                   szBuf, M_PASSWORD_BUFSIZE) != 0) 
    {
        /* encrypt failed */
        return -1;
    }

    printf("%s", szBuf);
    return 0;
}

/* djd add */
int do_string_decrypt(char *pszString)
{
    char szBuf[M_PASSWORD_BUFSIZE];
    
    if(check_string(pszString) != 0)
        return -1;

    if (do_decrypt(M_DEFAULT_PW_STR, pszString, strlen(pszString), szBuf, M_PASSWORD_BUFSIZE) != 0) 
        return -1;

    printf("%s", szBuf);
    return 0;
}

int do_ssh(char *pszCmd)
{
    char **argv = NULL;
    unsigned long argc = 0;
    int nRet = -1;

    if (split_tokens(pszCmd, strlen(pszCmd), M_SSH_CLIENT_MAX_ARGS,
                     " \t\r\n", &argc, &argv) != 0) {
        sal_printf("%% Internal error: parse command line failed\r\n");
        goto error_out;
    }

    nRet = ssh_login(argv, argc);
    
error_out:    
    if (NULL != argv) {
        free_tokens(&argv);
    }
    return nRet;
}

void sig_handler(int s)
{
    /* delete copy temp file */
    char szCmd[256];
    
    snprintf(szCmd, 256, "rm -fr /tmp/recv%d", getpid());
    system(szCmd);
    return;
}

void sigint_handler(int sig)
{
    exit(0);
}

int main(int argc, char **argv)
{
	int opt;

	char szPath[PATH_MAX];
	fnc_cmd_e cmd = E_FNC_CMD_NONE;
	char szMsg[M_MSG_BUFSZ];
	key_type_e otype = E_TYPE_MAX;
	szPath[0] = '\0';
	szMsg[0] = '\0';
    memory_init(0);
    gen_network_register_interactive_func(NETWORK_FTP, &g_cdbctl_ftp_ops);
    gen_network_register_interactive_func(NETWORK_TFTP, &g_cdbctl_tftp_ops);
    gen_network_register_interactive_func(NETWORK_SCP, &g_cdbctl_scp_ops);
    gen_network_register_interactive_func(NETWORK_XMODEM, &g_cdbctl_xmodem_ops);

	rsa_init_crypto();
	while (1) {
		if (EOF == (opt = getopt(argc, argv, "c:hp:m:t:o:f:a:u:U:"))) {
			break;
		}

		switch (opt) {
		case 'U':
		    g_pszUser = optarg;
		    break;
		case 'u':
		    g_pszUrl = optarg;
		    break;
		case 'a':
		    g_pszUrlAction = optarg;
		    if (strcasecmp(optarg, "send") && strcasecmp(optarg, "recv")) {
				usage();
			}
		    break;
		case 'o':
			g_pszOutFilename = optarg;
			break;
		case 'f':
			g_pszKeyFmt = optarg;
			if (strcasecmp(optarg, "public") && strcasecmp(optarg, "private")) {
				usage();
			}
			break;
		case 't':		/* output format */
			if (!strcasecmp(optarg, "pem")) {
				otype = E_TYPE_PEM;
			} else if (!strcasecmp(optarg, "der")) {
				otype = E_TYPE_DER;
			} else if (!strcasecmp(optarg, "der-hex")) {
				otype = E_TYPE_DERHEX;
			} else if (!strcasecmp(optarg, "ssh2")){
				otype = E_TYPE_SSH2;
			} else {
				usage();
			}
			break;
		case 'c':		/* command */
			cmd = parse_cmd_arg(optarg);
			if (cmd == E_FNC_CMD_NONE) {
				usage();
			}
			break;
		case 'p':		/* path */
			snprintf(szPath, PATH_MAX, "%s", optarg);
			break;
		case 'm':		/* overwrite confirm message */
		    if (strlen(optarg) >= M_MSG_BUFSZ) {
                printf("Message too long. Should less than %d\r\n", M_MSG_BUFSZ);
                exit(2);
		    }
			snprintf(szMsg, M_MSG_BUFSZ, "%s", optarg);
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	if (cmd_arg_validate(cmd, szMsg, szPath) != 0) {
		usage();
	}

    signal(SIGHUP, sig_handler);

    switch (cmd) {
    case E_FNC_CMD_URL:
        /* fix bug 39169 by xuwj: check udisk path */
        if (__check_filesystem_exist(szPath) != 0) {
            fprintf(stderr, "%% No enough space left on device.\r\n");
            exit(1);
        }
        break;
        
    default:
        break;
    }
    
	switch (cmd) {
	case E_FNC_CMD_ENCRYPT:
	    if (0 != do_string_encrypt(szMsg)) {
            exit(1);
	    }
	    break;
	case E_FNC_CMD_DECRYPT:
        if (0 != do_string_decrypt(szMsg)) {
            exit(1);
        }
        break;
	case E_FNC_CMD_SSH:
        /* Added by kcao for bug 45623 2018-08-14 */
        signal(SIGINT, sigint_handler);        
	    if (0 != do_ssh(szMsg)) {
            exit(1);
	    }
	    break;
	case E_FNC_CMD_LOGIN:
	    if (0 != do_login()) {
            exit(1);
	    }
	    break;
	case E_FNC_CMD_B64_PTON:
	case E_FNC_CMD_B64_NTOP:
	    if (0 != do_b64_encdec(cmd, szMsg)) {
            exit (1);
	    }
	    break;
	case E_FNC_CMD_TO_SHOWNAME:
	case E_FNC_CMD_TO_ACCESSNAME:
		if (0 != do_filename_convert(cmd, szPath)) {
			exit(1);
		}
		break;
	case E_FNC_CMD_FILE_PROC_CONFIRM:
		if (0 != do_file_proc_confirm(szMsg, szPath)) {
			exit(1);
		}
		break;
	case E_FNC_CMD_ISBINFILE:
		if (gen_is_bin_file(szPath) != 1) {
			exit(1);
		}
		break;
	case E_FNC_CMD_CONFIRM:
		if (cli_confirm("%s", szMsg) != 1) {
			exit(1);
		}
		break;
	case E_FNC_CMD_YESNO_NO:
		if (cli_yes_no_default_no("%s", szMsg) != 1) {
			exit(1);
		}
		break;
	case E_FNC_CMD_CONVERT_RSA_KEY:
		if (NULL == g_pszKeyFmt) {
			usage();
		}
		if (0 != do_rsa_convert(szMsg, szPath, otype)) {
			exit(1);
		}
		break;
	case E_FNC_CMD_IMPORT_RSA_KEY:
		if (NULL == g_pszKeyFmt) {
			usage();
		}
		if (0 != do_rsa_import(szMsg, szPath, otype)) {
			exit(1);
		}
		break;
	case E_FNC_CMD_URL:
		if (NULL == g_pszUrlAction || NULL == g_pszUrl) {
		    fprintf(stderr, "%% Please specify URL and action parameters\r\n");
			usage();
		}
		if (0 != do_url_proc(szPath)) {
			exit(1);
		}
		break;
	case E_FNC_CMD_FILE_COPY:
		if (NULL == g_pszOutFilename) {
		    fprintf(stderr, "%% Please specify destination file name\r\n");
			usage();
		}
        /* Modified by liuht for bug 33608, 2015-06-02 */
		if (0 != do_filecopy(szPath, g_pszOutFilename)) {
			exit(1);
		}
		break;
	default:
		usage();
	}

	exit(0);
}

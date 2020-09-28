#include "proto.h"
#include "param_check.h"
#include "lib_fs.h"
#include "ftp.h"
#include "rsa_derhex.h"
#include "fnconvert.h"

#define M_FTP_PORT_BUF  32

static char *g_plast_ftp_server = NULL;

/****************************************************************************
 *
* Function
* 
****************************************************************************/

/* is current ftp passvie mode */
int is_ftp_passive_mode(void)
{
    char *pszFtpMode = getenv("FTP_MODE");
    if (NULL != pszFtpMode) {
        return atoi(pszFtpMode);
    }
    
	return 0;
}

/* get current ftp username */
char *get_global_ftp_username(void)
{
    char *pszFtpUser = getenv("FTP_USER");
    return pszFtpUser;
}

/* get current ftp password */
char *get_global_ftp_password(void)
{
    char *pszFtpPin = getenv("FTP_PASSWD");
   
    return pszFtpPin;
}

/* set last ftp server */
void set_last_ftp_server(char *server)
{
	if (NULL == server) {
		return;
	}

	if (NULL != g_plast_ftp_server) {
		XFREE(MEM_TEMP, g_plast_ftp_server);
	}

	g_plast_ftp_server = XSTRDUP(MEM_TEMP, server);
	return;
}

/* get last ftp server */
char *get_last_ftp_server(void)
{
	return g_plast_ftp_server;
}

/* operation functions for ftp */
/*******************************************************************************
 * Name:    gen_ftp_op_init
 * Purpose: initialize ftp method, show use gen_ftp_op_release to free memory
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int gen_ftp_op_init(network_handle_t * h)
{
	cb_arg_ftp_t *pcb = NULL;

	if (NULL == h) {
		SAL_ASSERT(0);
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	SAL_ASSERT(GEN_NETWORK_GET_CB_ARG(h) == NULL);
	if ((pcb = XCALLOC(MEM_TEMP, sizeof(cb_arg_ftp_t))) == NULL) {
		return -1;
	}

	GEN_NETWORK_SET_CB_ARG(h, pcb);
	return 0;
}

/*******************************************************************************
 * Name:    gen_ftp_op_release
 * Purpose: release callback argument alloced by gen_ftp_op_init
 * Input: 
 *   h: handle
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void gen_ftp_op_release(network_handle_t * h)
{
	cb_arg_ftp_t *pcb = NULL;

	if (NULL == h) {
		return;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) != NULL) {
		XFREE(MEM_TEMP, pcb);
		GEN_NETWORK_SET_CB_ARG(h, NULL);
	}
	return;
}

int __gen_ftp_op_parserurl_hostname_filename_port(cb_arg_ftp_t * pcb, char *hostname)
{
    char *port = NULL;
    char *p = NULL;
    char *q = NULL;
    char *filename = NULL;

    if ((p = sal_strchr(hostname, '/')) != NULL) {
        /* have file name and hostname */
        *p = '\0';
        filename = p + 1;
    }

    if ('[' == *hostname) {
        /* ipv6 address */
        if ((p = sal_strchr(hostname, ']')) == NULL) {
            return -1;
        }
        pcb->bIPv6 = 1;
        hostname++;
        *p = '\0';
        if (':' == *(p + 1)) {
            port = p + 2;
        }
    } else if ((q = sal_strchr(hostname, ':')) != NULL) {
        *q = '\0';
        if ('\0' != *(q + 1)) {
            port = q + 1;
        }
    }

    /* check hostname */
    if (check_ipaddress(hostname) != 0) {
        return -1;
    }
    snprintf(pcb->hostname, MAXHOSTNAMELEN, "%s", hostname);
    set_last_ftp_server(pcb->hostname);

    /* check port value */
    if (port) {
        if (check_tcpudp_portnumber(port) == -1) {
            return -1;
        }

        pcb->port = sal_atoi(port);
    }


    if (filename && '\0' != filename[0]) {
        if (lib_filesys_check_filename(filename) == 0) {
            sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, filename);
            return 0;
        }
    }

    return -1;
}

/*******************************************************************************
 * Name:    gen_ftp_op_parseurl
 * Purpose:
 *   parse url to get parameters,
 *   format is: ftp://username:password@host:port/file_name
 * Input: 
 *   h: handle 
 *   url: url string
 * Output: 
 * Return: 
 *   success: 0 for complete parse
 *   failed : -1 for error or incomplete
 * Note: 
 ******************************************************************************/
int gen_ftp_op_parseurl(network_handle_t * h, char *url)
{
	cb_arg_ftp_t *pcb = NULL;
	network_dir_t *ndir = NULL;
	int nUrlLen = 0;
	int nMethodLen = 0;
	char *hostname = NULL;
	char *username = NULL;
	char *p = NULL;
	char *q = NULL;
	char line[CMD_BUF_256];

	if (NULL == h || NULL == url || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	ndir = h->ndir;
	pcb = GEN_NETWORK_GET_CB_ARG(h);
	if (NULL == pcb) {
		SAL_ASSERT(0);
		return -1;
	}

	pcb->port = 21;		/* ftp port */
	nUrlLen = sal_strlen(url);
	nMethodLen = sal_strlen(ndir->name);
	if (nMethodLen >= nUrlLen) {
		/* incomplete url */
		return -1;
	}

	if (sal_strncmp(ndir->name, url, nMethodLen)) {
		/* unknown method */
		return -1;
	}

	sal_snprintf(line, CMD_BUF_256, "%s", url);

	/* parse username and password */
	username = line + nMethodLen;
	if ((p = sal_strchr(username, '@')) == NULL) {
        hostname = username;	/* set hostname starting point */

        /* parse host name */
        if ('\0' == *hostname) {
            return -1;
        }

        /* first find hostname, port, filename */
        if (__gen_ftp_op_parserurl_hostname_filename_port(pcb, hostname) != 0) {
            return -1;
        }

		/* not include username and password string */
		if ((p = get_global_ftp_username()) == NULL) {
			/* can't find username */
			return -1;
		}
		sal_snprintf(pcb->username, M_MAX_USERNAME_LEN, p);

		if ((p = get_global_ftp_password()) == NULL) {
			/* can't find password */
			return -1;
		}
		sal_snprintf(pcb->password, M_PASSWORD_BUFSIZE, p);

	} else {
		hostname = p + 1;	/* set hostname starting point */

		/* parse password */
		if ((q = sal_strchr(username, ':')) == NULL || q > p) {
			/* no password string */
			if ((p - username) <= 0 || (p - username) >= M_MAX_USERNAME_LEN) {
				return -1;
			}
			sal_memcpy(pcb->username, username, (p - username));
			pcb->username[p - username] = '\0';
			if (check_username(pcb->username) != 0) {
				return -1;
			}

			if ((p = get_global_ftp_password()) == NULL) {
				/* can't find password */
				return -1;
			}
			sal_snprintf(pcb->password, M_PASSWORD_BUFSIZE, p);
		} else {
			/* get username */
			if ((q - username) <= 0 || (q - username) >= M_MAX_USERNAME_LEN) {
				return -1;
			}
			sal_memcpy(pcb->username, username, (q - username));
			pcb->username[q - username] = '\0';
			if (check_username(pcb->username) != 0) {
				return -1;
			}

			/* get password */
			if ((p - q - 1) <= 0 || (p - q - 1) >= M_MAX_PASSWORD_LEN) {
				return -1;
			}
			sal_memcpy(pcb->password, q + 1, (p - q - 1));
			pcb->password[p - q - 1] = '\0';
			if (check_password(pcb->password) != 0) {
				return -1;
			}

			if (do_encrypt(M_DEFAULT_PW_STR, q + 1, (p - q - 1),
						   pcb->password, M_PASSWORD_BUFSIZE) != 0) {
				/* encrypt failed */
				return -1;
			}
		}
	}

	/* parse host name */
	if ('\0' == *hostname) {
		return -1;
	}
    
    if (__gen_ftp_op_parserurl_hostname_filename_port(pcb, hostname) != 0) {
        return -1;
    }

	return 0;
}

/*******************************************************************************
 * Name:    gen_ftp_op_interactive_get_param
 * Purpose: get ftp parameters interactive
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int gen_ftp_op_interactive_get_param(network_handle_t * h)
{
	char szFileName[PATH_MAX];
	char szMsg[M_FULLPATH_MAX_LEN];
	cb_arg_ftp_t *pcb = NULL;
	char *pline = NULL;
	int ncnt = 3;

	if (NULL == h || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		sal_printf
			("%% Internal Error at gen_ftp_op_interactive_get_param.\n");
		SAL_ASSERT(0);
		return -1;
	}

	/* get hostname */
	if ('\0' == pcb->hostname[0] && get_last_ftp_server() != NULL) {
		sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, get_last_ftp_server());
	}
	if ('\0' != pcb->hostname[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "FTP server [%s] ",
					 pcb->hostname);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "FTP server [] ");
	}

	if (cli_readline(szFileName, PATH_MAX, szMsg) != 0) {
        return -1;
	}
	pline = szFileName;
	SKIP_WHITE_SPACE(pline);
	if ('\0' == *pline) {
		if ('\0' == pcb->hostname[0]) {
			sal_printf("%% Bad IPv4/IPv6 address or hostname\n");
			return -1;
		}
	} else {
		if (check_ipaddress(pline) != 0) {
			sal_printf("%% Bad IPv4/IPv6 address or hostname\n");
			return -1;
		}
		if (sal_strchr(pline, ':')) {
			pcb->bIPv6 = 1;
		}
		sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, pline);
		set_last_ftp_server(pcb->hostname);
	}

	/* get user name */
    if ('\0' != pcb->username[0] && 0 != check_username(pcb->username))
    {
        sal_printf("%% Bad user name\n");
        return -1;
    }
    
	if ('\0' == pcb->username[0] && get_global_ftp_username() != NULL) {
		sal_snprintf(pcb->username, M_MAX_USERNAME_LEN,
					 get_global_ftp_username());
	}
	if ('\0' != pcb->username[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "User name [%s] ",
					 pcb->username);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "User name [] ");
	}

	if (cli_readline(szFileName, PATH_MAX, szMsg) != 0) {
        return -1;
	}
	pline = szFileName;
	SKIP_WHITE_SPACE(pline);
	if ('\0' == *pline) {
		if ('\0' == pcb->username[0]) {
			sal_printf("%% Bad user name\n");
			return -1;
		}
	} else {
		if (check_username(pline) != 0) {
			sal_printf("%% Bad user name\n");
			return -1;
		}
		sal_snprintf(pcb->username, M_MAX_USERNAME_LEN, pline);
	}

	/* get password */
	if ('\0' == pcb->password[0] && get_global_ftp_password() != NULL) {
		sal_snprintf(pcb->password, M_MAX_PASSWORD_LEN,
					 get_global_ftp_password());
	}
	if ('\0' != pcb->password[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "Password [******] ");
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "Password [] ");
	}
	pline = getpass(szMsg);
	SKIP_WHITE_SPACE(pline);
	if ('\0' == *pline) {
		if ('\0' == pcb->password[0]) {
			sal_printf("%% Bad password\n");
			return -1;
		}
	} else {
		if (check_password(pline) != 0) {
			sal_printf("%% Bad password\n");
			return -1;
		}
		if (do_encrypt(M_DEFAULT_PW_STR, pline, sal_strlen(pline),
					   pcb->password, M_PASSWORD_BUFSIZE) != 0) {
			sal_printf("%% Encrypt password failed\n");
			return -1;
		}
	}

	/* get file name */
	if ('\0' != pcb->filename[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN,
					 "Name of the FTP file to access [%s] ", pcb->filename);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN,
					 "Name of the FTP file to access [] ");
	}
	while (ncnt > 0) {
		ncnt--;
		if (cli_readline(szFileName, PATH_MAX, szMsg) != 0) {
            return -1;
    	}
    	pline = szFileName;
		SKIP_WHITE_SPACE(pline);
		if ('\0' == *pline) {
			if ('\0' == pcb->filename[0]) {
                /* fix bug xuwj: has filename now */
                sal_printf("%% Invalid file name.\n");
                continue;
			}

            pline = pcb->filename;
		}

		if (check_filename(pline) != 0) {
			sal_printf("%% Invalid file name.\n");
			continue;
		}
        
        if(pline != pcb->filename) {
		    sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, pline);
        }
		return 0;
	}

	return -1;
}

/*******************************************************************************
 * Name:    gen_ftp_op_append_dir
 * Purpose: add file name to the dir. If the dir file name is unexist
 * Input: 
 *   h: handle
 *   dir:file dir
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note:  add by weij for bug 4662
 ******************************************************************************/
int gen_ftp_op_append_dir(network_handle_t * h, char *dir)
{
	char *name;
	int length = 0;
	char show_name[M_FULLPATH_MAX_LEN] = { '\0' };

	if (!sal_strncmp(dir, "/tmp", 4)) {
		return 0;
	}

	if (lib_fs_is_directory(dir)) {
		name = ((cb_arg_ftp_t *) (h->cb_arg))->filename;

		/*check length */
		length = sal_strlen(name) + sal_strlen(dir);
		if ('/' != dir[sal_strlen(dir) - 1]) {
			length++;
		}
		if (length >= M_FULLPATH_MAX_LEN) {
			return -1;
		}
		/*add name */
		if ('/' == dir[sal_strlen(dir) - 1]) {
			sal_strcat(dir, name);
		} else {
			sal_strcat(dir, "/");
			sal_strcat(dir, name);
		}
	}

	if (gen_fullname_to_showname(dir, show_name)) {
		return -1;
	}
	if (gen_file_proc_confirm("overwrite", dir, show_name) != 0) {
		return -1;
	}
	return 0;
}


/*******************************************************************************
 * Name:    gen_ftp_op_sendto
 * Purpose: send file to ftp server
 * Input: 
 *   h: handle
 *   src: file to send
 * Output: 
 * Return: 
 *   E_GEN_NETWORK_SUCCESS: for success
 *   E_GEN_NETWORK_TIMEOUT: for timeouted
 *   E_GEN_NETWORK_ACCESSDENY: for access being denied
 *   E_GEN_NETWORK_FAILED: for other errors
 * Note: 
 ******************************************************************************/
int gen_ftp_op_sendto(network_handle_t * h, char *src, char *vrf)
{
	cb_arg_ftp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;
	char szPort[M_FTP_PORT_BUF];

	if (NULL == h || NULL == src || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]
		|| '\0' == pcb->username[0] || '\0' == pcb->password[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	if (vrf != NULL) {
#ifdef _KERNEL_VER_2_6_
		argvi[argci++] = "chvrf";
		argvi[argci++] = "-n";
		argvi[argci++] = vrf;
#else
		argvi[argci++] = "ip";
		argvi[argci++] = "netns";
		argvi[argci++] = "exec";
		if (!strcasecmp(vrf, "ZERO_VRF")) {
            argvi[argci++] = "default";
		} else {
            argvi[argci++] = "mgmt";
        }
#endif
	}

	argvi[argci++] = CTC_FTP_PROG;
	if (is_ftp_passive_mode()) {
		argvi[argci++] = "-p";
	}
	argvi[argci++] = "-c";
	argvi[argci++] = "put";
	argvi[argci++] = "-u";
	argvi[argci++] = pcb->username;
	argvi[argci++] = "-w";
	argvi[argci++] = pcb->password;
	argvi[argci++] = "-l";
	argvi[argci++] = src;
	argvi[argci++] = "-r";
	argvi[argci++] = pcb->filename;
	argvi[argci++] = pcb->hostname;
	if (pcb->port != 21) {
		sal_snprintf(szPort, M_FTP_PORT_BUF, "%u", pcb->port);
		argvi[argci++] = szPort;
	}
	argvi[argci] = NULL;

	sal_printf("Send file to %s%s@%s:%u/%s\n",
			   h->ndir->name, pcb->username,
			   pcb->hostname, pcb->port, pcb->filename);

	/* Execute ftp. */
	if (gen_execvp(argci, argvi) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	return ret;
}

/*******************************************************************************
 * Name:    gen_ftp_op_recvfrom
 * Purpose: get file from ftp
 * Input: 
 *   h: handle
 * Output: 
 *   dst: dst file
 * Return: 
 *   success: E_GEN_NETWORK_SUCCESS
 *   failed : others
 * Note: 
 ******************************************************************************/
int gen_ftp_op_recvfrom(network_handle_t * h, char *dst, char *vrf)
{
    printf("gen_ftp_op_recvfrom!\n");
	cb_arg_ftp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;
	char szPort[M_FTP_PORT_BUF];

	if (NULL == h || NULL == dst || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_FTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]
		|| '\0' == pcb->username[0] || '\0' == pcb->password[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	if (vrf != NULL) {
#ifdef _KERNEL_VER_2_6_
		argvi[argci++] = "chvrf";
		if (!strcasecmp(vrf, "ZERO_VRF")) {
		    argvi[argci++] = "0";
		} else {
    		argvi[argci++] = "-n";
    		argvi[argci++] = vrf;
		}
#else
		argvi[argci++] = "ip";
		argvi[argci++] = "netns";
		argvi[argci++] = "exec";
		if (!strcasecmp(vrf, "ZERO_VRF")) {
            argvi[argci++] = "default";
		} else {
            argvi[argci++] = "mgmt";
        }
#endif
	}

	argvi[argci++] = CTC_FTP_PROG;
	if (is_ftp_passive_mode()) {
		argvi[argci++] = "-p";
	}
	argvi[argci++] = "-c";
	argvi[argci++] = "get";
	argvi[argci++] = "-u";
	argvi[argci++] = pcb->username;
	argvi[argci++] = "-w";
	argvi[argci++] = pcb->password;
	argvi[argci++] = "-l";
	argvi[argci++] = dst;
	argvi[argci++] = "-r";
	argvi[argci++] = pcb->filename;
	argvi[argci++] = pcb->hostname;
	if (pcb->port != 21) {
		sal_snprintf(szPort, M_FTP_PORT_BUF, "%u", pcb->port);
		argvi[argci++] = szPort;
	}
	argvi[argci] = NULL;

	sal_printf("Get file from %s%s@%s:%u/%s\n",
			   h->ndir->name, pcb->username,
			   pcb->hostname, pcb->port, pcb->filename);

	/* Execute ftp. */
	if (gen_execvp(argci, argvi) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	return ret;
}

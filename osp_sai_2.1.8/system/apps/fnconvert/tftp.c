#include "proto.h"
#include "param_check.h"
#include "lib_fs.h"
#include "tftp.h"
#include "fnconvert.h"

#include <sys/wait.h>

#define TFTP_PROG_IPV4  "ctctftp"
#define TFTP_PROG_IPV6  "ctctftp6"

/* Default "tftp" parameter.  */
/* Segment size */
#define TFTP_SEGSIZE_DEFAULT    512
#define TFTP_SEGSIZE_MIN        8
#define TFTP_SEGSIZE_MAX        65464

/* packet rexmt timeout */
#define TFTP_TIMEOUT_DEFAULT    5
#define TFTP_TIMEOUT_MIN        1
#define TFTP_TIMEOUT_MAX        255

/* tansfer mode */
#define TFTP_MODE_DEFAULT       "binary"
#define TFTP_MODE_BINARY        "binary"
#define TFTP_MODE_ASCII         "ascii"
#define TFTP_MODE_NETASCII      "netascii"
#define TFTP_MODE_IMAGE         "image"
#define TFTP_MODE_OCTET         "octet"
//#define TFTP_MODE_MAIL          "mail"

/* commands */
#define TFTP_CMD_GET            "get"
#define TFTP_CMD_PUT            "put"

#define TFTP_PORT_DEFAULT       69

/* "tftp" options.  */
/* XXX: family option not implemented yet */
#define TFTP_FAMILY_OPTION             "-A"
#define TFTP_FAMILY_IPV4               "inet"
#define TFTP_FAMILY_IPV6               "inet6"
#define TFTP_SEGSIZE_OPTION            "-s"
#define TFTP_TIMEOUT_OPTION            "-w"
#define TFTP_MODE_OPTION               "-m"
#define TFTP_PORT_OPTION               "-p"
#define TFTP_ADDRESS_OPTION            "-Z"
#define TFTP_VERBOSE_OPTION            "-v"
#define TFTP_REMOTE_FILENAME_OPTION    "-r"
#define TFTP_LOCAL_FILENAME_OPTION     "-l"
/* get or put command */
#define TFTP_COMMAND_OPTION            "-c"


static char g_last_tftp_server[MAXHOSTNAMELEN] = { '\0' };

static int32
_gen_tftp_mapping_args(int32 argci, char **argvi, char *vrf, int32 * argco,
					   char ***argvo)
{
	int opt;

#define M_MAX_TFTP_ARGS 32
	int count = M_MAX_TFTP_ARGS;
	int cnt = 0;

	if (argci < 1 || NULL == argvi || NULL == argco || NULL == argvo) {
		return -1;
	}

	/* Reset optind. */
	optind = 1;

	/* Malloc size for argv. */
	*argvo = (char **) XMALLOC(MEM_TEMP, count * sizeof(char *));
	if (NULL == *argvo) {
		return -1;
	}
	if (vrf != NULL) {
#ifdef _KERNEL_VER_2_6_
		(*argvo)[cnt++] = "chvrf";
		if (!strcasecmp(vrf, "ZERO_VRF")) {
		    (*argvo)[cnt++] = "0";
		} else {
    		(*argvo)[cnt++] = "-n";
    		(*argvo)[cnt++] = vrf;
		}
#else
		(*argvo)[cnt++] = "ip";
		(*argvo)[cnt++] = "netns";
		(*argvo)[cnt++] = "exec";
		if (!strcasecmp(vrf, "ZERO_VRF")) {
		    (*argvo)[cnt++] = "default";
		} else {
            (*argvo)[cnt++] = "mgmt";
        }
#endif
	}

	/* Now fill up the options for output. */
	while (1) {
		opt = getopt(argci, argvi, "A:Z:s:w:p:m:r:l:c:v");
		if (opt == EOF) {
			break;
		}

		if (cnt >= (M_MAX_TFTP_ARGS - 3)) {
			/* too many arguments */
			XFREE(MEM_TEMP, *argvo);
			return -1;
		}

		switch (opt) {
		case 'A':
			/* tftp program name. */
			if (!strcmp(optarg, TFTP_FAMILY_IPV4)) {
				(*argvo)[cnt++] = TFTP_PROG_IPV4;
			} else if (!strcmp(optarg, TFTP_FAMILY_IPV6)) {
				(*argvo)[cnt++] = TFTP_PROG_IPV6;
			}
			break;
		case 'v':
			/* verbose option, default is quiet */
			(*argvo)[cnt++] = "-v";
			break;
		case 'Z':
			/* host address */
			(*argvo)[cnt++] = "-Z";
			(*argvo)[cnt++] = optarg;
			break;
		case 'm':
			/* mode */
			(*argvo)[cnt++] = "-m";
			(*argvo)[cnt++] = optarg;
			break;
		case 's':
			/* segment size */
			(*argvo)[cnt++] = "-s";
			(*argvo)[cnt++] = optarg;
			break;
		case 'w':
			/* rextm Timeout */
			(*argvo)[cnt++] = "-w";
			(*argvo)[cnt++] = optarg;
			break;
		case 'c':
			/* command: get or put */
			(*argvo)[cnt++] = "-c";
			(*argvo)[cnt++] = optarg;
			break;
		case 'p':
			/* port */
			(*argvo)[cnt++] = "-p";
			(*argvo)[cnt++] = optarg;
			break;
		case 'l':
			/* local filename */
			(*argvo)[cnt++] = "-l";
			(*argvo)[cnt++] = optarg;
			break;
		case 'r':
			/* remote filename  */
			(*argvo)[cnt++] = "-r";
			(*argvo)[cnt++] = optarg;
			break;
		default:
			/* unknown argument */
			XFREE(MEM_TEMP, *argvo);
			return -1;
			break;
		}
	}					/* while */

	/* Nullify last element. */
	(*argvo)[cnt] = NULL;
	*argco = cnt;

	return 0;
}

/* process functions for TFTP */
/*******************************************************************************
 * Name:    gen_tftp_op_init
 * Purpose: initialize tftp method, show use gen_tftp_op_release to free memory
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int32 gen_tftp_op_init(network_handle_t * h)
{
	cb_arg_tftp_t *pcb = NULL;

	if (NULL == h) {
		SAL_ASSERT(0);
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	SAL_ASSERT(GEN_NETWORK_GET_CB_ARG(h) == NULL);
	if ((pcb = XCALLOC(MEM_TEMP, sizeof(cb_arg_tftp_t))) == NULL) {
		return -1;
	}

	GEN_NETWORK_SET_CB_ARG(h, pcb);
	return 0;
}

/*******************************************************************************
 * Name:    gen_tftp_op_release
 * Purpose: release callback argument alloced by gen_tftp_op_init
 * Input: 
 *   h: handle
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void gen_tftp_op_release(network_handle_t * h)
{
	cb_arg_tftp_t *pcb = NULL;

	if (NULL == h) {
		return;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) != NULL) {
		XFREE(MEM_TEMP, pcb);
		GEN_NETWORK_SET_CB_ARG(h, NULL);
	}
	return;
}

/*******************************************************************************
 * Name:    gen_tftp_op_parseurl
 * Purpose:
 *   parse url to get parameters,
 *   format is: tftp://hostname_or_ip:port/file_name
 * Input: 
 *   h: handle 
 *   url: url string
 * Output: 
 * Return: 
 *   success: 0 for complete parse
 *   failed : -1 for error or incomplete
 * Note: 
 ******************************************************************************/
int32 gen_tftp_op_parseurl(network_handle_t * h, char *url)
{
	cb_arg_tftp_t *pcb = NULL;
	network_dir_t *ndir = NULL;
	int32 nUrlLen = 0;
	int32 nMethodLen = 0;
	char *hostname = NULL;
	char *p = NULL;
	char *port = NULL;
	char *filename = NULL;
	char line[CMD_BUF_256];

	if (NULL == h || NULL == url || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	ndir = h->ndir;
	pcb = GEN_NETWORK_GET_CB_ARG(h);
	if (NULL == pcb) {
		SAL_ASSERT(0);
		return -1;
	}

	sal_snprintf(line, CMD_BUF_256, "%s", url);
	nUrlLen = sal_strlen(line);
	nMethodLen = sal_strlen(ndir->name);
	if (nMethodLen >= nUrlLen) {
		/* incomplete url */
		return -1;
	}

	if (sal_strncmp(ndir->name, line, nMethodLen)) {
		/* unknown method */
		return -1;
	}

	/* split filename and host */
	hostname = line + nMethodLen;
	if ((filename = sal_strchr(hostname, '/')) != NULL) {
		*filename = '\0';
		filename++;
	}

	/* get hostname and port */
	if ('[' == *hostname) {
		/* Is ipv6 address */
		hostname++;
		if ((p = sal_strchr(hostname, ']')) == NULL
			|| (p - hostname) <= 1 || (p - hostname) >= MAXHOSTNAMELEN) {
			return -1;
		}

		pcb->bIPv6 = 1;

		*p = '\0';
		if (':' == *p) {
			/* get port */
			port = p + 1;
			*port = '\0';
			port++;
			if ('\0' == port) {
				return -1;
			}

		}
	} else if ((port = sal_strchr(hostname, ':')) != NULL) {
		*port = 0;
		port++;
	}

	/* validate hostname */
	if (check_ipaddress(hostname) != 0) {
		return -1;
	}
	sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, hostname);
	sal_snprintf(g_last_tftp_server, MAXHOSTNAMELEN, hostname);

	/* validate port number */
	if (port && check_tcpudp_portnumber(port) == -1) {
		return -1;
	}
	sal_snprintf(pcb->port, MAX_PORT_NUM, port);

	/* validate filename */
	if (NULL == filename) {
		return -1;
	}

	if (lib_filesys_check_filename(filename) != 0) {
		return -1;
	}
	sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, filename);
	return 0;
}

/*******************************************************************************
 * Name:    gen_tftp_op_append_dir
 * Purpose: add file name to the dir. If the dir file name is unexist
 * Input: 
 *   h: handle
 *   dir:file dir
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note:  delete by weij for bug 4662
 ******************************************************************************/
int32 gen_tftp_op_append_dir(network_handle_t * h, char *dir)
{
	char *name = NULL;
	char *p = NULL;
	int32 length = 0;
	char show_name[M_FULLPATH_MAX_LEN] = { '\0' };

	if (!sal_strncmp(dir, "/tmp", 4)) {
		return 0;
	}

	if (lib_fs_is_directory(dir)) {
		name = ((cb_arg_tftp_t *) (h->cb_arg))->filename;
		if (NULL != (p = sal_strrchr(name, '/')))
			name = p + 1;

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
 * Name:    gen_tftp_op_interactive_get_param
 * Purpose: get tftp parameters interactive
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int32 gen_tftp_op_interactive_get_param(network_handle_t * h)
{
	char szFileName[PATH_MAX];
	char szMsg[M_FULLPATH_MAX_LEN];
	cb_arg_tftp_t *pcb = NULL;
	char *pline = NULL;
	int32 ncnt = 3;
	int32 ret = -1;

	if (NULL == h || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		sal_printf
			("%% Internal Error at gen_tftp_op_interactive_get_param.\n");
		SAL_ASSERT(0);
		return -1;
	}

	/* get hostname */
	if ('\0' != g_last_tftp_server[0] && '\0' == pcb->hostname[0]) {
		sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, g_last_tftp_server);
	}
	if ('\0' != pcb->hostname[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "TFTP server [%s] ",
					 pcb->hostname);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "TFTP server [] ");
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
		sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, pline);
		sal_snprintf(g_last_tftp_server, MAXHOSTNAMELEN, pline);
		if (sal_strchr(pline, ':')) {
			pcb->bIPv6 = 1;
		}
	}

	/* get file name */
	if ('\0' != pcb->filename[0]) {
		sal_snprintf(szMsg, PATH_MAX, "Name of the TFTP file to access [%s] ",
					 pcb->filename);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN,
					 "Name of the TFTP file to access [] ");
	}
	while (ncnt > 0) {
		ncnt--;
		if (cli_readline(szFileName, PATH_MAX, szMsg) != 0) {
            continue;
		}
		
		pline = szFileName;
		SKIP_WHITE_SPACE(pline);
		if ('\0' == *pline) {
			if ('\0' != pcb->filename[0]) {
				break;
			}
			sal_printf("%% Invalid file name\n");
			continue;
		}

		/* for bug 17649 modified by liuyang 2012-4-18 */
		ret = lib_filesys_check_filename(pline);
		if (ret) {
			if (-2 == ret)
				sal_printf
					("%% The max length of file name should be little than 48\n");
			else
				sal_printf("%% Invalid file name\n");
			continue;
		}
		sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, pline);
		return 0;
	}

	return -1;
}

/*******************************************************************************
 * Name:    gen_tftp_op_sendto
 * Purpose: send file to tftp server
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
int32 gen_tftp_op_sendto(network_handle_t * h, char *src, char *vrf)
{
	cb_arg_tftp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	char **argvo = NULL;
	int argci = 0;
	int argco;
	int ret = E_GEN_NETWORK_SUCCESS;

	if (NULL == h || NULL == src || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	argvi[argci++] = CTC_TFTP_PROG;
	argvi[argci++] = TFTP_FAMILY_OPTION;
	argvi[argci++] = TFTP_FAMILY_IPV4;
	argvi[argci++] = TFTP_MODE_OPTION;
	argvi[argci++] = TFTP_MODE_BINARY;
	argvi[argci++] = TFTP_SEGSIZE_OPTION;
	/* add by weij, 2012 for IVI */
	argvi[argci++] = "1280";
	argvi[argci++] = TFTP_ADDRESS_OPTION;
	argvi[argci++] = pcb->hostname;
	argvi[argci++] = TFTP_COMMAND_OPTION;
	argvi[argci++] = TFTP_CMD_PUT;
	if (pcb->port && sal_strlen(pcb->port)) {
		argvi[argci++] = TFTP_PORT_OPTION;
		argvi[argci++] = pcb->port;
	}
	argvi[argci++] = TFTP_LOCAL_FILENAME_OPTION;
	argvi[argci++] = src;
	argvi[argci++] = TFTP_REMOTE_FILENAME_OPTION;
	argvi[argci++] = pcb->filename;
	argvi[argci] = NULL;

	if (pcb->bIPv6) {
		sal_printf("Send file to %s[%s]/%s\n",
				   h->ndir->name, pcb->hostname, pcb->filename);
	} else {
		sal_printf("Send file to %s%s/%s\n",
				   h->ndir->name, pcb->hostname, pcb->filename);
	}

	/* Map tftp options. */
	if (_gen_tftp_mapping_args(argci, argvi, vrf, &argco, &argvo) != 0) {
		return E_GEN_NETWORK_INVALID;
	}

	/* Execute tftp. */
	if (gen_execvp(argco, argvo) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	/* Free argvo. */
	XFREE(MEM_TEMP, argvo);
	return ret;
}

/*******************************************************************************
 * Name:    gen_tftp_op_recvfrom
 * Purpose: get file from tftp
 * Input: 
 *   h: handle
 * Output: 
 *   dst: dst file
 * Return: 
 *   success: E_GEN_NETWORK_SUCCESS
 *   failed : others
 * Note: 
 ******************************************************************************/
int32 gen_tftp_op_recvfrom(network_handle_t * h, char *dst, char *vrf)
{
	cb_arg_tftp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	char **argvo = NULL;
	int32 argci = 0;
	int32 argco;
	int32 ret = E_GEN_NETWORK_SUCCESS;

	if (NULL == h || NULL == dst || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_TFTP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	argvi[argci++] = CTC_TFTP_PROG;
	argvi[argci++] = TFTP_FAMILY_OPTION;
	argvi[argci++] = TFTP_FAMILY_IPV4;
	argvi[argci++] = TFTP_MODE_OPTION;
	argvi[argci++] = TFTP_MODE_BINARY;
	argvi[argci++] = TFTP_SEGSIZE_OPTION;
	/* add by weij, 2012 for IVI */
	argvi[argci++] = "1280";
	argvi[argci++] = TFTP_ADDRESS_OPTION;
	argvi[argci++] = pcb->hostname;
	argvi[argci++] = TFTP_COMMAND_OPTION;
	argvi[argci++] = TFTP_CMD_GET;
	if (pcb->port && sal_strlen(pcb->port)) {
		argvi[argci++] = TFTP_PORT_OPTION;
		argvi[argci++] = pcb->port;
	}
	argvi[argci++] = TFTP_LOCAL_FILENAME_OPTION;
	argvi[argci++] = dst;
	argvi[argci++] = TFTP_REMOTE_FILENAME_OPTION;
	argvi[argci++] = pcb->filename;
	argvi[argci] = NULL;

	if (vrf && !strcasecmp(vrf, COMMON_VRF_NAME)) {
		sal_printf("Get file from active supervisor\n");
	} else if (pcb->bIPv6) {
		sal_printf("Get file from %s[%s]/%s\n",
				   h->ndir->name, pcb->hostname, pcb->filename);
	} else {
		sal_printf("Get file from %s%s/%s\n",
				   h->ndir->name, pcb->hostname, pcb->filename);
	}

	/* Map tftp options. */
	if (_gen_tftp_mapping_args(argci, argvi, vrf, &argco, &argvo) != 0) {
		return E_GEN_NETWORK_INVALID;
	}

	/* Execute tftp. */
	if (gen_execvp(argco, argvo) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	/* Free argvo. */
	XFREE(MEM_TEMP, argvo);
	return ret;
}

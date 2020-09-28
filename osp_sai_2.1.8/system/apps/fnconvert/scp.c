#include "proto.h"
#include "param_check.h"
#include "lib_fs.h"
#include "scp.h"
#include "rsa_derhex.h"
#include "fnconvert.h"

#define M_SCP_PORT_BUF  32

static char *global_scp_username = NULL;
static char *g_plast_scp_server = NULL;

/* get current scp username */
char *get_global_scp_username(void)
{
	return global_scp_username;
}


/* set last scp server */
void set_last_scp_server(char *server)
{
	if (NULL == server) {
		return;
	}

	if (NULL != g_plast_scp_server) {
		XFREE(MEM_TEMP, g_plast_scp_server);
	}

	g_plast_scp_server = XSTRDUP(MEM_TEMP, server);
	return;
}

/* get last scp server */
char *get_last_scp_server(void)
{
	return g_plast_scp_server;
}

/*allocate/free memory, same as ftp and tftp*/

/* operation functions for SCP */
/*
 * Name:    gen_scp_op_init
 * Purpose: initialize scp method, show use gen_scp_op_release to free memory
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 */
int gen_scp_op_init(network_handle_t * h)
{
	cb_arg_scp_t *pcb = NULL;

	if (NULL == h) {
		SAL_ASSERT(0);
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	SAL_ASSERT(GEN_NETWORK_GET_CB_ARG(h) == NULL);
	if ((pcb = XCALLOC(MEM_TEMP, sizeof(cb_arg_scp_t))) == NULL) {
		return -1;
	}

	GEN_NETWORK_SET_CB_ARG(h, pcb);
	return 0;
}

/*
 * Name:    gen_scp_op_release
 * Purpose: release callback argument alloced by gen_scp_op_init
 * Input: 
 *   h: handle
 * Output: 
 * Return: N/A
 * Note: 
 */
void gen_scp_op_release(network_handle_t * h)
{
	cb_arg_scp_t *pcb = NULL;

	if (NULL == h) {
		return;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) != NULL) {
		XFREE(MEM_TEMP, pcb);
		GEN_NETWORK_SET_CB_ARG(h, NULL);
	}
	return;
}

/*******************************************************************************
 * Name:    scp_check_filename
 * Purpose:
 *   helper function. Check if target directory or file name is valid
 * Input: 
 *   pszFilename: file name
 * Output: 
 * Return: 
 *   success: 0 for valid file name
 *   failed : -1 for invalid file name
 * Note: 
 ******************************************************************************/
int
scp_check_filename(const char *pszFilename)
{
    int nLen = 0;
    const char *p = NULL;
    
    if (NULL == pszFilename)
    {
        return -1;
    }

    nLen = strlen(pszFilename);
    while (nLen >= 0)
    {
        if (('/' == pszFilename[nLen]))
        {
            break;
        }
        nLen--;
    }
    p = &(pszFilename[nLen+1]);

    /* check the string length */
    nLen = strlen(p);
    if (nLen < M_FILENAME_MINSIZE || nLen >= M_FILENAME_MAXSIZE)
    { 
        return -2;
    }

    /* check the chars */
    p = (char *)pszFilename;
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || ('(' == *p)
            || (')' == *p)
            || ('/' == *p)
            || ('~' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    gen_scp_op_parseurl
 * Purpose:
 *   parse url to get parameters,
 *   format is: scp://username@host:[/directory][/filename]
 * Input: 
 *   h: handle 
 *   url: url string
 * Output: 
 * Return: 
 *   success: 0 for complete parse
 *   failed : -1 for error or incomplete
 * Note: 
 ******************************************************************************/
int gen_scp_op_parseurl(network_handle_t * h, char *url)
{
	cb_arg_scp_t *pcb = NULL;
	network_dir_t *ndir = NULL;
	int nUrlLen = 0;
	int nMethodLen = 0;
	char *hostname = NULL;
	char *username = NULL;
	char *filename = NULL;
	char *p = NULL;
	//char *port = NULL;
	char line[CMD_BUF_256];

	if (NULL == h || NULL == url || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	ndir = h->ndir;
	pcb = GEN_NETWORK_GET_CB_ARG(h);
	if (NULL == pcb) {
		SAL_ASSERT(0);
		return -1;
	}

	pcb->port = 22;		/* SCP port */
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

	/* parse username */
	username = line + nMethodLen;
	if ((p = sal_strchr(username, '@')) == NULL) {
		/* not include username and password string */
		if ((p = get_global_scp_username()) == NULL) {
			/* can't find username */
			return -1;
		}
		sal_snprintf(pcb->username, M_MAX_USERNAME_LEN, p);

		hostname = username;	/* set hostname starting point */
	} else {
		hostname = p + 1;	/* set hostname starting point */

		sal_memcpy(pcb->username, username, (p - username));
		pcb->username[p - username] = '\0';
		if (check_username(pcb->username) != 0) {
			return -1;
		}

	}

	/* parse host name */
	if ('\0' == *hostname) {
		return -1;
	}
    if ('[' != *hostname){
    	if ((p = sal_strchr(hostname, ':')) != NULL) {
    		/* have file name and hostname */
    		*p = '\0';
    		filename = p + 1;
    	}
	}

	if ('[' == *hostname) {
		/* ipv6 address */
		if ((p = sal_strchr(hostname, ']')) == NULL) {
			return -1;
		}
		pcb->bIPv6 = 1;
		hostname++;
		*p = '\0';
		filename = p + 2;
	}

	/* check hostname */
	if (check_ipaddress(hostname) != 0) {
		return -1;
	}
	snprintf(pcb->hostname, MAXHOSTNAMELEN, "%s", hostname);
	set_last_scp_server(pcb->hostname);
	sal_printf("pcb->hostname:%s\n",pcb->hostname);
   sal_printf("get_last_scp_server():%s\n",get_last_scp_server());
   
	if (filename && '\0' != filename[0]) {
		if (scp_check_filename(filename) == 0) {
			sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, filename);
			return 0;
		}
	}

	return -1;
}


/*******************************************************************************
 * Name:    gen_scp_op_interactive_get_param
 * Purpose: get scp parameters interactive
 * Input: 
 *   h: handle
 *   cli: client pointer
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int gen_scp_op_interactive_get_param(network_handle_t * h)
{
	char szFileName[PATH_MAX];
	char szMsg[M_FULLPATH_MAX_LEN];
	cb_arg_scp_t *pcb = NULL;
	char *pline = NULL;
	int ncnt = 3;

	if (NULL == h || NULL == h->ndir) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		sal_printf
			("%% Internal Error at gen_scp_op_interactive_get_param.\n");
		SAL_ASSERT(0);
		return -1;
	}

	/* get hostname */
	if ('\0' == pcb->hostname[0] && get_last_scp_server() != NULL) {
		sal_snprintf(pcb->hostname, MAXHOSTNAMELEN, get_last_scp_server());
	}
	
	if ('\0' != pcb->hostname[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "SCP server [%s] ",
					 pcb->hostname);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN, "SCP server [] ");
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
		set_last_scp_server(pcb->hostname);
	}

	/* get user name */
	if ('\0' == pcb->username[0] && get_global_scp_username() != NULL) {
		sal_snprintf(pcb->username, M_MAX_USERNAME_LEN,
					 get_global_scp_username());
	}
	if ('\0' != pcb->username[0]) {
        if (NULL != strstr(pcb->username, "%"))
        {
            return -1;
        }
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


	/* get file name */
	if ('\0' != pcb->filename[0]) {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN,
					 "Name of the SCP file to access [%s] ", pcb->filename);
	} else {
		sal_snprintf(szMsg, M_FULLPATH_MAX_LEN,
					 "Name of the SCP file to access [] ");
	}
	while (ncnt > 0) {
		ncnt--;
		if (cli_readline(szFileName, PATH_MAX, szMsg) != 0) {
			return -1;
		}
		pline = szFileName;
		SKIP_WHITE_SPACE(pline);
		if ('\0' == *pline) {
			if ('\0' != pcb->filename[0]) {
				break;
			}
			sal_printf("%% Invalid file name.\n");
			continue;
		}

		if (check_filename(pline) != 0) {
			sal_printf("%% Invalid file name.\n");
			continue;
		}
		sal_snprintf(pcb->filename, M_FULLPATH_MAX_LEN, pline);
		return 0;
	}

	return -1;
}


/*******************************************************************************
 * Name:    gen_scp_op_append_dir
 * Purpose: add file name to the dir. If the dir file name is unexist
 * Input: 
 *   h: handle
 *   cli: client pointer
 *   dir:file dir
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note:  add by weij for bug 4662
 ******************************************************************************/
int gen_scp_op_append_dir(network_handle_t * h, char *dir)
{
	char *name;
	int length = 0;
	char show_name[M_FULLPATH_MAX_LEN] = { '\0' };
    char *p = NULL;

	if (!sal_strncmp(dir, "/tmp", 4)) {
		return 0;
	}

	if (lib_fs_is_directory(dir)) {
        name = ((cb_arg_scp_t *) (h->cb_arg))->filename;
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
 * Name:    gen_scp_op_sendto
 * Purpose: send file to scp server
 * Input: 
 *   h: handle
 *   src: file to send
 *   cli: client pointer
 * Output: 
 * Return: 
 *   E_GEN_NETWORK_SUCCESS: for success
 *   E_GEN_NETWORK_TIMEOUT: for timeouted
 *   E_GEN_NETWORK_ACCESSDENY: for access being denied
 *   E_GEN_NETWORK_FAILED: for other errors
 * Note: 
 ******************************************************************************/
int gen_scp_op_sendto(network_handle_t * h, char *src, char *vrf)
{
	cb_arg_scp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;
	char szPort[M_SCP_PORT_BUF];
	char szTmp[BUFF_SIZE_256];

	if (NULL == h || NULL == src || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]
		|| '\0' == pcb->username[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	argvi[argci++] = "ip";
	argvi[argci++] = "netns";
	argvi[argci++] = "exec";
	if (vrf && !strcasecmp(vrf, "ZERO_VRF")) {
		argvi[argci++] = "default";
	} else {
		argvi[argci++] = "mgmt";
	}

	argvi[argci++] = CTC_SCP_PROG;
	if (pcb->port != 22) {
		sal_snprintf(szPort, M_SCP_PORT_BUF, "%u", pcb->port);
		argvi[argci++] = "-P";
		argvi[argci++] = szPort;
	}
	//argvi[argci++] = "-t";    //to

	argvi[argci++] = src;

	if (!pcb->bIPv6) {
        sal_snprintf(szTmp, BUFF_SIZE_256, "%s@%s:%s",
        			 pcb->username, pcb->hostname, pcb->filename);
	}
	else {
	    sal_snprintf(szTmp, BUFF_SIZE_256, "%s@[%s]:%s",
        			 pcb->username, pcb->hostname, pcb->filename);
	}
	argvi[argci++] = szTmp;
	argvi[argci] = NULL;
	
	sal_printf("Send file to %s%s@%s:%u/%s\n",
			   h->ndir->name, pcb->username,
			   pcb->hostname, pcb->port, pcb->filename);

	/* Execute scp. */
	if (gen_execvp(argci, argvi) != 0) {
		ret = E_GEN_NETWORK_FAILED;
	}
	return ret;
}

/*******************************************************************************
 * Name:    gen_scp_op_recvfrom
 * Purpose: get file from scp
 * Input: 
 *   h: handle
 *   cli: client pointer
 * Output: 
 *   dst: dst file
 * Return: 
 *   success: E_GEN_NETWORK_SUCCESS
 *   failed : others
 * Note: 
 ******************************************************************************/
int gen_scp_op_recvfrom(network_handle_t * h, char *dst, char *vrf)
{
	cb_arg_scp_t *pcb = NULL;
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;
	char szPort[M_SCP_PORT_BUF];
	char szTmp[BUFF_SIZE_256];

	if (NULL == h || NULL == dst || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_SCP);
	if ((pcb = GEN_NETWORK_GET_CB_ARG(h)) == NULL) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}
	if ('\0' == pcb->hostname[0] || '\0' == pcb->filename[0]
		|| '\0' == pcb->username[0]) {
		SAL_ASSERT(0);
		return E_GEN_NETWORK_INVALID;
	}

	argvi[argci++] = "ip";
	argvi[argci++] = "netns";
	argvi[argci++] = "exec";
	if (vrf && !strcasecmp(vrf, "ZERO_VRF")) {
		argvi[argci++] = "default";
	} else {
		argvi[argci++] = "mgmt";
	}

	argvi[argci++] = CTC_SCP_PROG;

	if (pcb->port != 22) {
		sal_snprintf(szPort, M_SCP_PORT_BUF, "%u", pcb->port);
		argvi[argci++] = "-P";
		argvi[argci++] = szPort;
	}
    if (!pcb->bIPv6) {
        sal_snprintf(szTmp, BUFF_SIZE_256, "%s@%s:%s",
        			 pcb->username, pcb->hostname, pcb->filename);
	}
	else {
	    sal_snprintf(szTmp, BUFF_SIZE_256, "%s@[%s]:%s",
        			 pcb->username, pcb->hostname, pcb->filename);
	}
	argvi[argci++] = szTmp;
	argvi[argci++] = dst;
	argvi[argci] = NULL;

	sal_printf("Get file from %s%s@%s:%u/%s\n",
			   h->ndir->name, pcb->username,
			   pcb->hostname, pcb->port, pcb->filename);

	/* Execute scp. */
	if (gen_execvp(argci, argvi) != 0) {
		ret = E_GEN_NETWORK_FAILED;
	}
	return ret;
}

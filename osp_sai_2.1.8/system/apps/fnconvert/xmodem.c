#include "proto.h"
#include "param_check.h"
#include "lib_fs.h"
#include "xmodem.h"
#include "fnconvert.h"

#include <sys/wait.h>

#define XMODEM_PROG_SEND    "lsz"
#define XMODEM_PROG_RECV    "rx"

/* process functions for XMODEM */
/*******************************************************************************
 * Name:    gen_xmodem_op_parseurl
 * Purpose:
 *   parse url to get parameters,
 *   format is: xmodem:
 * Input: 
 *   h: handle 
 *   url: url string
 * Output: 
 * Return: 
 *   success: 0 for complete parse
 *   failed : -1 for error or incomplete
 * Note: 
 ******************************************************************************/
int32 gen_xmodem_op_parseurl(network_handle_t * h, char *url)
{
	if (NULL == h || NULL == url) {
		return -1;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_XMODEM);
	if (strcmp(url, "xmodem:")) {
        return -1;
	}

	return 0;
}

/*******************************************************************************
 * Name:    gen_xmodem_op_append_dir
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
int32 gen_xmodem_op_append_dir(network_handle_t * h, char *dir)
{
	char show_name[M_FULLPATH_MAX_LEN] = { '\0' };

	if (!sal_strncmp(dir, "/tmp", 4)) {
		return 0;
	}

	if (lib_fs_is_directory(dir)) {
        return -1;
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
 * Name:    gen_xmodem_op_interactive_get_param
 * Purpose: get xmodem parameters interactive
 * Input: 
 *   h: handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int32 gen_xmodem_op_interactive_get_param(network_handle_t * h)
{
    /* always failed */
    return -1;
}

/*******************************************************************************
 * Name:    gen_xmodem_op_sendto
 * Purpose: send file to xmodem server
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
int32 gen_xmodem_op_sendto(network_handle_t * h, char *src, char *vrf)
{
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;

	if (NULL == h || NULL == src || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_XMODEM);
	argvi[argci++] = XMODEM_PROG_SEND;
	argvi[argci++] = "-Z"; /* use ZMODEM protocol */
	argvi[argci++] = "-b"; /* binary transfer */
	argvi[argci++] = "-y"; /* overwrite existing files */
	argvi[argci++] = src;
	argvi[argci] = NULL;

	sal_printf("Send file to %s\n", h->ndir->name);

	/* Execute xmodem. */
	if (gen_execvp(argci, argvi) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	
	return ret;
}

/*******************************************************************************
 * Name:    gen_xmodem_op_recvfrom
 * Purpose: get file from xmodem
 * Input: 
 *   h: handle
 * Output: 
 *   dst: dst file
 * Return: 
 *   success: E_GEN_NETWORK_SUCCESS
 *   failed : others
 * Note: 
 ******************************************************************************/
int32 gen_xmodem_op_recvfrom(network_handle_t * h, char *dst, char *vrf)
{
	char *argvi[SYSCMD_MAX_ARG_NUM];
	int argci = 0;
	int ret = E_GEN_NETWORK_SUCCESS;

	if (NULL == h || NULL == dst || NULL == h->ndir) {
		return E_GEN_NETWORK_INVALID;
	}

	SAL_ASSERT(GEN_NETWORK_GET_TYPE(h) == NETWORK_XMODEM);
	argvi[argci++] = "rx";
	argvi[argci++] = dst;
	argvi[argci++] = ">";
	argvi[argci++] = "/dev/null";
	argvi[argci++] = "2>&1";

	argvi[argci] = NULL;

	sal_printf("Get file from %s\n", h->ndir->name);

	/* Execute xmodem. */
	if (gen_execvp(argci, argvi) != 0) {
        ret = E_GEN_NETWORK_FAILED;
	}
	
	return ret;
}

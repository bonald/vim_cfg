/****************************************************************************
 * syscmd_readline.h:    
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Xiaobo Yan
 * Date         :       2007-1-22
 * Reason       :       First Create.
 ****************************************************************************/
#ifndef _SYSCMD_READLINE_H_
#define _SYSCMD_READLINE_H_

//#include <sys/param.h>

#include "cli_mode_types.h"
#include "cli.h"
#include "param_check.h"
//#include "cfgmgr.h"


#define SYSCMD_MAXARGS               10
#define TAR_FILE_MAX_NUM             20
#define SYSCMD_MAX_ARG_NUM           32
#define MAX_PORT_NUM           10


#define MAX_PATH_NAME_LEN           M_FILENAME_MAXSIZE 
#define MAX_FILE_NAME_LEN           M_FILENAME_MAXSIZE 

/* Modified by Percy Wang 2007-12-27 */
#define FOLD(c) (isupper ((unsigned char)c) \
	? tolower ((unsigned char)c) \
	: ((unsigned char)c))
	
/* Bits set in the FLAGS argument to `fnmatch'.  */
/* standard flags */
#define	FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
#define	FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
#define	FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */

/* extended flags */
#define FNM_LEADING_DIR	(1 << 3) /* Ignore `/...' after a match. */
#define FNM_CASEFOLD	(1 << 4) /* Compare without regard to case. */
#define FNM_EXTMATCH	(1 << 5) /* Use ksh-like extended matching. */

enum network_file_e
{
    NETWORK_TFTP,
    NETWORK_FTP,
    NETWORK_MAX_TYPE
};
typedef enum network_file_e network_file_t;

enum tar_type
{
    TAR_CREATE,
    TAR_LIST,
    TAR_XTRACT
};

/* operation function and handle for network method */
struct network_dir_s;
typedef struct network_handle_s
{
    struct network_dir_s *ndir; /* network struct pointer */
    void *cb_arg; /* call back argument */
} network_handle_t;
#define GEN_NETWORK_HANDLE_INIT(h)     memset(h, 0x00, sizeof(network_handle_t))
#define GEN_NETWORK_SET_CB_ARG(h, x)    h->cb_arg = ((void *)(x))
#define GEN_NETWORK_GET_CB_ARG(h)       (h->cb_arg)
#define GEN_NETWORK_GET_TYPE(h)         ((h)->ndir ? (h)->ndir->network_file : NETWORK_MAX_TYPE)

#define E_GEN_NETWORK_SUCCESS       0x01
#define E_GEN_NETWORK_TIMEOUT       0x02
#define E_GEN_NETWORK_ACCESSDENY    0x04
#define E_GEN_NETWORK_FAILED        0x08
#define E_GEN_NETWORK_INVALID       0x10
typedef struct network_op_s
{
    /* init handle */
    int (*init)(network_handle_t *h); /* 0 for success */
    
    /* release handle */
    void (*release)(network_handle_t *h);
    
    /* parse url and get parameter */
    int (*parseurl)(network_handle_t *h, char *url); /* 0 for success */
    
    /* get parameter interactive */
    int (*interactive_get_param)(network_handle_t *h, struct cli *cli); /* 0 for success */

    /* append dir , add by weij for bug 4662*/
    int (*append_dir)(network_handle_t *h, struct cli *cli, char *dir); /* 0 for success */

    /* send object to url 
     * return E_GEN_NETWORK_*
     */
    int (*sendto)(network_handle_t *h, char *src, struct cli *cli);

    /* get object from url
     * return E_GEN_NETWORK_*
     */
    int (*recvfrom)(network_handle_t *h, char *dst, struct cli *cli);
} network_op_t;

int gen_network_register_interactive_func(network_file_t etype, network_op_t *ops);
#define gen_network_unregister_interactive_func gen_network_register_interactive_func

struct network_dir_s
{
    char *name; /* network method name */
    char *usage; /* format string */
    network_file_t network_file; /* network method type */
    network_op_t *ops; /* process functions */
};
typedef struct network_dir_s network_dir_t;

/* callback argument for TFTP */
typedef struct cb_arg_tftp_s
{
    unsigned char hostname[32]; /* hostname or ip address */
    unsigned char filename[M_FULLPATH_MAX_LEN]; /* file name to access */
    unsigned char port[MAX_PORT_NUM];
} cb_arg_tftp_t;

/* callback argument for FTP */
typedef struct cb_arg_ftp_s
{
    unsigned char username[M_MAX_USERNAME_LEN]; /* username */
    unsigned char password[M_PASSWORD_BUFSIZE]; /* password */
    unsigned char hostname[32]; /* hostname or ip address */
    unsigned char filename[M_FULLPATH_MAX_LEN]; /* file name to access */
    unsigned short port; /* service port */
} cb_arg_ftp_t;

typedef enum diskfile
{
    E_DISK_FILE_FLASH = 0,
    E_DISK_FILE_CFCARD = 1,
    E_DISK_FILE_UDISK = 2,
    E_DISK_BOOT_FLASH = 3,/* for bug 28631 added by liuyang 2014-5-29*/
    E_DISK_FILE_MAX
} diskfile_e;

struct rootdir_s
{
    diskfile_e etype; /* disk file type */
    char *real_name;
    char *show_name;
};
typedef struct rootdir_s rootdir_t;

/* callback function for iterate root dirs */
typedef void DISK_ITER_FUNC(rootdir_t *rdir, void *arg);

/* Added by Alexander Liu 2007-02-28 09:14 { */
#define M_GEN_FLASHNAME_STR     "GFLASHFILE"
#define M_GEN_CFCARDNAME_STR    "GCFCARDFILE"
#define M_GEN_FILENAME_STR      "GFILENAME"
#define M_GEN_URLNAME_STR       "GURLNAME"
typedef enum gen_match_type
{
    E_GEN_MATCH_NONE = 0,
    E_GEN_MATCH_INCOMPLETE,
    E_GEN_MATCH_ALL
} gen_match_e;
typedef struct gen_path_match_arg
{
    /* for expand command */
    char **ptr; /* output buffer */
    int *pcnt; /* current item count */

    /* common item */
    rootdir_t *rdir; /* root dir */
    
    /* for calculate help string max length */
    int *plen;
    
    /* for output help string */
    struct cli *cli; 
    /* for long format */
    int max;
    int width;
    /* for short format */
    int firstcnt; /* first ouput cnt */
    int *curcnt; /* current output cnt */
    int *maxarr;
    int maxsize; /* max arr size */
} gen_path_match_arg_t;

gen_match_e gen_filename_match(char *cp, int token, int child_node);
gen_match_e gen_url_match(char *cp);
int gen_refine_helpstr_len(char *str);
int gen_output_helpstr(struct cli *cli, struct cli_node *node,
                       int max, int width);
int gen_output_helpstr_short(struct cli *cli, char *str, int *cnt, int *max, int maxsize);
int gen_set_last_token(char *str);
void gen_output_matchstr(char **ptr, char *oldtext, char *type, int *nCntRet);
void gen_set_partly_match(int on);
int gen_partly_match(void);
int gen_get_match_count(char *type);
int gen_check_and_get_filename(struct cli *cli, char *filename, char *outfile, size_t outsize);
int gen_check_and_get_filename_bytype(struct cli *cli, char *filename, char *outfile, size_t outsize, int type);
int gen_check_and_gen_showname(char *szOrig, char *szShow);
int gen_send_to_url(struct cli *cli, char *szSrc, char *url);
int gen_recv_from_url(struct cli *cli, char *szDst, char *url);
network_dir_t * network_name2ops(const char *url);
int gen_is_bin_file(char *file);
void gen_iter_disk_cfg(DISK_ITER_FUNC pfunc, void *arg);
int gen_check_and_get_filename_bycwd(char *filename, char *outfile, size_t outsize);
/* Added by Alexander Liu 2007-02-28 09:14 } */

int network_name2type(const char *str);
char * syscmd_get_url(char *para, network_file_t type);
int syscmd_check_actual_directory_exist(struct cli *cli, char * filename);
int syscmd_copy_file(char *src_file, char *dst_file);
int syscmd_is_directory(char *filename);
int syscmd_is_file(char *filename);
char * syscmd_get_filename(char *path);
int syscmd_file_exist(char *filename);
int syscmd_file_can_read(char *filename);
int syscmd_mkdir_protect_root_dir(struct cli * cli, char * filename);
int gen_is_file_system_root(char *pszName);
//static int extmatch (int xc, char *s, char *se, char *p, char *pe, int flags);
int file_name_match (char *pattern, char *string, int flags);
int gen_fullname_to_showname(char *fullname, char *showname);
#endif

/****************************************************************************
 * lib_fs.h:    
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Xiaobo Yan
 * Date         :       2007-1-22
 * Reason       :       First Create.
 ****************************************************************************/
#ifndef __LIB_FS_H__
#define __LIB_FS_H__

#include <sys/param.h>
#include "param_check.h"

#define BANNER_MOTD_FILE    "/tmp/banner_motd"
#define BANNER_EXEC_FILE    "/tmp/banner_exec"
#define BANNER_LOGIN_FILE   "/tmp/banner_login"
#define BANNER_KEY_WORD     "^C"

/* filename buffer size */
#define M_FILENAME_MAXSIZE      48
#define M_FILENAME_MINSIZE      1
#define M_FILENAME_LIMIT_STR    "can only include [0-9a-zA-Z.-_]. Its length range is [1, 48)."

#define FULL_NAME_MNT                "/mnt"
#define FULL_NAME_FLASH              "/mnt/flash"
/* Added by liuht for second subarea for bug25870, 2013-11-22 */
#define FULL_NAME_FLASH_BOOT         "/mnt/flash/boot"
#define FULL_NAME_CFCARD             "/mnt/cf"
#define FULL_NAME_UDISK              "/mnt/udisk"
#define SHOW_NAME_FLASH              "flash:"
#define SHOW_NAME_FLASH_BOOT         "flash:/boot"

#define MANAGEMENT_VRF_NAME     "MANG_VRF"
#define COMMON_VRF_NAME         "COMM_VRF"
#define ZERO_VRF_NAME           "ZERO_VRF"

#ifndef M_FULLPATH_MAX_LEN
#define M_FULLPATH_MAX_LEN      256
#define M_FULLPATH_DEPTH_MAX    32
#endif
#define CPU_HIGH_DEFULT_THRESHOLD   90
#define CPU_LOW_DEFULT_THRESHOLD   50
#define SYSCMD_MAXARGS              10
#define TAR_FILE_MAX_NUM            20
#define SYSCMD_MAX_ARG_NUM          32
#define MAX_PORT_NUM                10

#define MAX_PATH_NAME_LEN           M_FILENAME_MAXSIZE 
#define MAX_FILE_NAME_LEN           M_FILENAME_MAXSIZE 

#define MANAGEMENT_IP_STR            "management ip address"

#define M_SYSTEM_LOG_FILE            FULL_NAME_FLASH"/syslog"

#define DHCPSNOOPING_SAVE_BINDING_FILE  FULL_NAME_FLASH"/dhcpsnooping"
#define DHCPSNOOPING_SAVE_BINDING_FILE6  FULL_NAME_FLASH"/dhcpv6snooping"

#define LOGIN_DIRECTORY              FULL_NAME_FLASH
#define STARTUP_CONFIG_FILE_NAME     "startup-config.conf"
#define STARTUP_CONFIG_FILE_PATH     FULL_NAME_FLASH"/"STARTUP_CONFIG_FILE_NAME

#define FACTORY_CONFIG_FILE_NAME     ".factory-config.conf"
#define FACTORY_CONFIG_FILE_PATH     M_BOOT_IMAGE_PATH"/"FACTORY_CONFIG_FILE_NAME

#define M_BOOT_IMAGE_PATH               FULL_NAME_FLASH"/boot"
#define M_BOOT_IMAGE_PATH_SHOW               SHOW_NAME_FLASH"/boot"
#define M_CUR_BOOT_IMAGE_LINK_NAME      "uImage"
#define M_CUR_BOOT_IMAGE_LINK_FULLNAME  FULL_NAME_FLASH"/boot/"M_CUR_BOOT_IMAGE_LINK_NAME

/* Modified by Percy Wang 2007-12-27 */
#define FOLD(c) (isupper ((unsigned char)c) \
    ? tolower ((unsigned char)c) \
    : ((unsigned char)c))

/* Bits set in the FLAGS argument to `fnmatch'.  */
/* standard flags */
#define FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
#define FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
#define FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */

/* extended flags */
#define FNM_LEADING_DIR	(1 << 3) /* Ignore `/...' after a match. */
#define FNM_CASEFOLD	(1 << 4) /* Compare without regard to case. */
#define FNM_EXTMATCH	(1 << 5) /* Use ksh-like extended matching. */

enum network_file_e
{
    NETWORK_TFTP,
    NETWORK_FTP,
    NETWORK_SCP,
    NETWORK_XMODEM,
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

#define BUFFER_COUNT                4096

typedef struct network_op_s
{
    /* init handle */
    int (*init)(network_handle_t *h); /* 0 for success */
    
    /* release handle */
    void (*release)(network_handle_t *h);
    
    /* parse url and get parameter */
    int (*parseurl)(network_handle_t *h, char *url); /* 0 for success */
    
    /* get parameter interactive */
    int (*interactive_get_param)(network_handle_t *h); /* 0 for success */

    /* append dir , add by weij for bug 4662*/
    int (*append_dir)(network_handle_t *h, char *dir); /* 0 for success */

    /* send object to url 
     * return E_GEN_NETWORK_*
     */
    int (*sendto)(network_handle_t *h, char *src, char *vrf);

    /* get object from url
     * return E_GEN_NETWORK_*
     */
    int (*recvfrom)(network_handle_t *h, char *dst, char *vrf);
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
    char hostname[MAXHOSTNAMELEN]; /* hostname or ip address */
    char filename[M_FULLPATH_MAX_LEN]; /* file name to access */
    char port[MAX_PORT_NUM];
    int bIPv6;
} cb_arg_tftp_t;

/* callback argument for FTP */
typedef struct cb_arg_ftp_s
{
    char username[M_MAX_USERNAME_LEN]; /* username */
    char password[M_PASSWORD_BUFSIZE]; /* password */
    char hostname[MAXHOSTNAMELEN]; /* hostname or ip address */
    char filename[M_FULLPATH_MAX_LEN]; /* file name to access */
    unsigned short port; /* service port */
    int bIPv6;
} cb_arg_ftp_t;

/* callback argument for SCP */
typedef struct cb_arg_scp_s
{
    char username[M_MAX_USERNAME_LEN]; /* username */
    char hostname[MAXHOSTNAMELEN]; /* hostname or ip address */
    char filename[M_FULLPATH_MAX_LEN]; /* file name to access */
    unsigned short port; /* service port */
    int bIPv6;
} cb_arg_scp_t;

typedef enum diskfile
{
    E_DISK_FILE_FLASH = 0,
    E_DISK_FILE_CFCARD = 1,
    E_DISK_FILE_UDISK = 2,
    E_DISK_BOOT_FLASH = 3,/* for bug 28631 modified by liuyang 2014-5-29*/
    E_DISK_FILE_MAX
} diskfile_e;

typedef enum {
    E_TOKEN_GFLASHNAME,
    E_TOKEN_GCFCARDNAME,
    E_TOKEN_GFILENAME,
    E_TOKEN_GURLNAME,
    E_TOKEN_UNKNOWN,
} param_token_e;


struct rootdir_s
{
    diskfile_e etype; /* disk file type */
    char *real_name;
    /* for bug 28631 modified by liuyang 2014-5-29*/
    /* Added by liuht for second subarea for bug25870, 2013-11-22 */	
    //char *real_name_second;
    char *show_name;
};
typedef struct rootdir_s rootdir_t;

/* callback function for iterate root dirs */
typedef void DISK_ITER_FUNC(rootdir_t *rdir, void *arg);
typedef int HELP_OUT_FUNC(void *pArg, const char *pszMatch, const char *pszHelp);

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
    HELP_OUT_FUNC *pCB; /* callback function */
    void *pArg;
    char *oldtext; /* old text */
    int nMatched; /* matched count */

    /* common item */
    rootdir_t *rdir; /* root dir */
    
    /* for calculate help string max length */
    int *plen;
    
    /* for long format */
    int max;
    int width;
    /* for short format */
    int firstcnt; /* first ouput cnt */
    int *curcnt; /* current output cnt */
    int *maxarr;
    int maxsize; /* max arr size */
} gen_path_match_arg_t;

typedef struct system_file_s
{
    char *filename; /* file name */
    char *usage; /* usage string */
} system_file_t;

#define CLI_RPP_ECHO_OFF          0x01    /* Turn off echo (default). */
#define CLI_RPP_ECHO_ON           0x02    /* Leave echo on. */
#define CLI_RPP_WITH_TIMEOUT      0x04    /* fail if timeout */
#define CLI_RPP_CONFIRM_MODE      0x08    /* confirm mode: only accept 1 char */
#define CLI_RPP_LINE_MODE         0x10    /* line mode */

int cli_readpassphrase(const char *prompt, char *buf, size_t bufsiz, int flags, int timeout);
int cli_readline(char *pszBuf, size_t bufsz, char *pszFmt, ...);
int cli_confirm(char *pszFmt, ...); /* default yes */
int cli_yes_no_default_no(char *pszFmt, ...); /* default no */

int gen_file_proc_confirm(char *action, char *file, char *origfile);

int copy_file_with_progress(char *src_file, char *dst_file, int need_progress);

int check_username(char *username);

int check_password(char *pin);

int check_user_password(char *pin);

int check_tcpudp_portnumber(const char *pszPort);

int split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
             char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);

int free_tokens(char ***pppTokArr);

int lib_fs_is_file(char *filename);

int lib_fs_is_directory(char *filename);
int lib_fs_is_chardev(char *filename);

int lib_fs_is_exist(char *filename);

int lib_filesys_check_filename(const char *pszFilename);

int gen_validate_path(char *pszPath);

void gen_path_getparents(char *name);

int gen_validate_relative_path(char *pszFullName);

int syscmd_check_actual_directory_exist(char *filename);

int gen_fullname_to_showname(char *fullname, char *showname);

int gen_recv_from_url(char *errmsg, uint size, char *szDst, char *url, char *vrf);
int gen_send_to_url(char *szSrc, char *url, char *vrf);

int gen_network_register_interactive_func(network_file_t etype, network_op_t *ops);

int gen_check_and_get_filename_bycwd(char *filename, char *outfile, size_t outsize);
int gen_check_and_get_filename(char *filename, char *outfile, size_t outsize);
int gen_set_dir_mnt_valid(int valid);
int gen_check_and_get_filename_bytype(char *filename, char *outfile,
                                  size_t outsize, int type);

int gen_check_and_gen_showname(char *szOrig, char *szShow);

gen_match_e gen_filename_match(char *cp, int token, int child_node);
gen_match_e gen_url_match(char *cp);
int gen_set_last_token(char *str);
char *gen_get_last_token_str(void);
void gen_output_matchstr(HELP_OUT_FUNC *pCB, void *pArg, char *oldtext, char *type);
void gen_clear_match_status(void);
int gen_get_match_status(void);
void gen_set_match_status(int status);
param_token_e gen_parse_token(const char *token);

int gen_is_bin_file(char *file);
int gen_execvp(int argc, char **argv);

void
gen_dbg_print_hex_emacs(int modid, const u_char *cp, unsigned int length, unsigned int offset);

const char *gen_get_url_helpstr(const char *pszUrl);
void match_boot_and_remount(char *matchdir, int vfsflags);
int match_boot_and_refuse(char *matchdir);

int lib_fs_file_large(char *filename);
#endif /* !__LIB_FS_H__ */


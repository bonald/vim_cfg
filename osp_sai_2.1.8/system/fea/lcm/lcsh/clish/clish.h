#ifndef _CENTECOS_CLISH_H
#define _CENTECOS_CLISH_H

#ifdef CNOS_OS
#include "sal.h"
#include "ctc_task.h"
#else
#include "sal_common.h"
#include "ctclib_thread.h"
#endif
#include "param_check.h"

//#include "cfgmgr.h"
//#include "glb_define.h"
//#include "sys_param.h"

#define CLISH_ADMIN_NAME        M_SYSTEM_ADMIN_NAME
#define CLISH_LOGIN_FAILURE_MAX		3
#define MTYPE_TMP 0
#define XSTRDUP(type,str)       strdup(str)
#define XFREE(type,ptr)         free(ptr)

#define LINE_MESSAGE_MAX        512
#define CLISH_BUFSIZE 8192
#define MAX_TTY_NAME_LEN     50
#define LOGGING_MONITOR_DIR             SWITCH_PREFIX"/etc/monitor" 

/* output a split line */
#define CLISH_OUT_SPLIT   \
cli_out(cli, "======================================================================\n")
#define CLISH_BUF_MAXLEN           1024
#define ZONE_CONFIG_PATH    "/tmp/zone.conf"
#define ZONE_CONFIG_DEST    "/mnt/flash/.zone.conf"

#if CTC_IS_UML
#define M_TTY_DIR  "/dev"
#else
#define M_TTY_DIR  "/dev/pts"
#endif

struct globals
{
    char username[M_MAX_USERNAME_LEN];
    char vpsname[M_MAX_VPSNAME_LEN];
    char logindir[M_FULLPATH_MAX_LEN];
    
    char port[30];
    int timeout; /* timeout value */
};


struct clish_master_s
{
#ifdef CNOS_OS
    ctc_task_master_t  *p_thread_master;
#else
    ctclib_thread_master_t *p_thread_master;
#endif
};
typedef struct clish_master_s clish_master_t;

#define CLISH_MALLOC(type,size) sal_malloc(size)
#define CLISH_CALLOC(type,size) sal_calloc(size)
#define CLISH_FREE(type, ptr)    sal_free(ptr)

char *getCurPort(void);

//extern cfm_clnt_t *cfm_clnt;
extern struct globals g_globals;

char *clish_prompt(void);
void clish_parse_line(char *line);
int clish_parse (char *line, int mode);

int rl_timed_out(void);
void rl_set_timeout_flag(int bflag);
int rl_sigint(void);
void rl_set_sigint_flag(int bflag);

int clish_get_timeout(void);
char *clish_get_timeout_str();

typedef int TOFunction(void);

extern TOFunction *rl_timeout_function;

int clish_confirm(char *str);
int clish_yes_or_no (char *str);
int clish_yes_or_no_default_no (char *str);

int get_addr_by_dns (int family, char *host, char *ipaddr, int size);

/* smart input flags */
#define M_SINPUT_WITHOUT_TIMEOUT    0x01 /* without timeout. (default have) */
#define M_SINPUT_CONFIRM            0x02 /* input one char */
char *clish_smart_input(char *prompt, int flags);

char *get_current_logindir(void);

#endif

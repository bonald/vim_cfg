
#ifndef _LCSH_CLI_H
#define _LCSH_CLI_H

#include <time.h>
#include "vector.h"
#include "cli_mode_types.h" 

//#define _CENTEC_

#define SYSTEM_DEFAULT_NAME "CentecEX"
#define M_SYSTEM_ADMIN_NAME "admin"
#define M_SYSTEM_ADMIN_DEFAULT_PASSWD "admin"
#define M_DEFAULT_VPS_NAME "default"
/* for bug 28631 added by liuyang 2014-5-29*/
#define FULL_NAME_FLASH_BOOT              "/mnt/flash/boot"
#define FULL_NAME_FLASH              "/mnt/flash"
#define FULL_NAME_CFCARD             "/mnt/cf"
#define FULL_NAME_UDISK              "/mnt/udisk"
#define SHOW_NAME_FLASH              "flash:"

#define M_SYSTEM_LOG_FILE            FULL_NAME_FLASH"/syslog"

#define DHCPSNOOPING_SAVE_BINDING_FILE  FULL_NAME_FLASH"/dhcpsnooping"

#define LOGIN_DIRECTORY              FULL_NAME_FLASH
#define STARTUP_CONFIG_FILE_NAME     "startup-config.conf"
#define STARTUP_CONFIG_FILE_PATH     FULL_NAME_FLASH"/"STARTUP_CONFIG_FILE_NAME
#define FACTORY_CONFIG_FILE_NAME     ".factory-config.conf"
#define FACTORY_CONFIG_FILE_PATH     FULL_NAME_FLASH"/"FACTORY_CONFIG_FILE_NAME

#define M_BOOT_IMAGE_PATH               FULL_NAME_FLASH"/boot"
#define M_BOOT_IMAGE_PATH_SHOW               SHOW_NAME_FLASH"/boot"
#define M_CUR_BOOT_IMAGE_LINK_NAME      "uImage"
#define M_CUR_BOOT_IMAGE_LINK_FULLNAME  FULL_NAME_FLASH"/boot/"M_CUR_BOOT_IMAGE_LINK_NAME
#define M_CUR_BOOT_WEB_IMAGE_LINK_NAME      ".webImage"
#define M_CUR_BOOT_WEB_IMAGE_LINK_FULLNAME  FULL_NAME_FLASH"/"M_CUR_BOOT_WEB_IMAGE_LINK_NAME
/*added by kexl for bug 45079, 2017-11-09*/
#define M_CUR_BOOT_WEB_IMAGE_NAME      ".webImagename"
#define M_CUR_BOOT_WEB_IMAGE_FULLNAME  FULL_NAME_FLASH"/"M_CUR_BOOT_WEB_IMAGE_NAME

//#define DEFAULT_LOW_TMPR    5
//#define DEFAULT_HIGH_TMPR   70
//#define DEFAULT_CRIT_TMPR   85

#define LINE_TIMEOUT_DEFAULT_SEC 300

/*Serial parameter*/
#define SERIAL_PARITY_NONE    0   
#define SERIAL_PARITY_ODD      1   
#define SERIAL_PARITY_EVEN     2

#define SERIAL_DEFAULT_SPEED_PARA 9600
#define SERIAL_DEFAULT_DATABITS_PARA 8
#define SERIAL_DEFAULT_STOPBITS_PARA 1

/* CLI macro.  Macro defines a CLI structure and a CLI function.  C99
   C preprocessor variadic macros is used to define arbitrary number
   of help string definition.  */

/* Perl script is used for extracting CLI and ALI macro from source
   code.  In that case EXTRACT_CLI should be defined to prevent macro
   expansion.  */

#ifndef EXTRACT_CLI

#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-04-11 15:24 */
struct cli_tree;
struct cli;

int cli_set_arg (struct cli_tree *ctree, char *cp, int len, int merge);

struct gblock_element 
{
    char *type; /* type string */
    char *desc; /* description for this block */
    int (*func)(struct cli *, int, char **);
    u_int32_t modules;
};

#define GENBLOCK(funcname, bname, btype, bdesc)         \
/* General block process function */                    \
int funcname (struct cli *, int argc, char ** argv);    \
                                                        \
/* Define general block structure */                    \
struct gblock_element bname =                           \
{                                                       \
    btype, bdesc, funcname, 0                           \
};                                                      \
                                                        \
/* Start Function body at here */                       \
int funcname (struct cli *cli, int argc, char **argv)

#define GENBLOCKREF(funcname, bname, btype, bdesc)      \
/* Define general block structure */                    \
struct gblock_element bname =                           \
{                                                       \
    btype, bdesc, funcname, 0                           \
}

int genblock_install_imi(struct cli_tree *, int, u_int32_t, struct gblock_element *);
struct gblock_element *genblock_parse(struct cli_tree *ctree, int mode, char *str);
#endif /* !_CENTEC_ */

#ifdef HAVE_ISO_MACRO_VARARGS
#define CLI(func_name, cli_name, cmd_str, ...)                          \
                                                                        \
/* Function prototype.  */                                              \
int func_name (struct cli *, int, char**);                              \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {__VA_ARGS__, NULL};                        \
                                                                        \
/* Define CLI structure.  */                                            \
struct cli_element cli_name =                                           \
{                                                                       \
  /* Command line string.  */                                           \
  cmd_str,                                                              \
                                                                        \
   /* Function pointer.  */                                             \
  func_name,                                                            \
                                                                        \
  /* Help string is defined as an array.  Last must be NULL.  */        \
  cli_name ## _help                                                     \
};                                                                      \
                                                                        \
/* Start function body at here.  */                                     \
int func_name (struct cli *cli, int argc, char** argv)
#else
#define CLI(func_name, cli_name, cmd_str, ARGS...)                      \
                                                                        \
/* Function prototype.  */                                              \
int func_name (struct cli *, int, char**);                              \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {ARGS, NULL};                               \
                                                                        \
/* Define CLI structure.  */                                            \
struct cli_element cli_name =                                           \
{                                                                       \
  /* Command line string.  */                                           \
  cmd_str,                                                              \
                                                                        \
   /* Function pointer.  */                                             \
  func_name,                                                            \
                                                                        \
  /* Help string is defined as an array.  Last must be NULL.  */        \
  cli_name ## _help                                                     \
};                                                                      \
                                                                        \
/* Start function body at here.  */                                     \
int func_name (struct cli *cli, int argc, char** argv)
#endif /* HAVE_ISO_MACRO_VARARGS. */

/* ALIAS to CLI macro.  Define CLI structure only.  There is no
   function body.  */

#ifdef HAVE_ISO_MACRO_VARARGS
#define ALI(func_name, cli_name, cmd_str, ...)                          \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {__VA_ARGS__, NULL};                        \
                                                                        \
struct cli_element cli_name =                                           \
{                                                                       \
  cmd_str,                                                              \
  func_name,                                                            \
  cli_name ## _help                                                     \
}
#else
#define ALI(func_name, cli_name, cmd_str, ARGS...)                      \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {ARGS, NULL};                               \
                                                                        \
struct cli_element cli_name =                                           \
{                                                                       \
  cmd_str,                                                              \
  func_name,                                                            \
  cli_name ## _help                                                     \
}
#endif /* HAVE_ISO_MACRO_VARARGS. */
#endif /* EXTRACT_CLI */

/* CLI output function.  We can switch over CLI output function
   defining cli->out_func.  */

typedef int (*CLI_OUT_FUNC) (void *, char *, ...);

/* CLI output function macro.  Instead of defining a function, user
   can specify CLI output function dynamically.  */
#ifdef HAVE_ISO_MACRO_VARARGS
#define cli_out(cli, ...)                                                     \
        (*(cli)->out_func)((cli)->out_val, __VA_ARGS__)
#else
#define cli_out(cli, ARGS...)                                                 \
        (*(cli)->out_func)((cli)->out_val, ##ARGS)
#endif /* HAVE_ISO_MACRO_VARARGS. */

/* CLI parsing tree.  This structure has all of CLI commands and run
   time variables such as mode and privilege.  Mode and privilege
   level will be passed to cli_parse() function then set to this
   structure.  */

struct cli_tree
{
  /* Current mode.  */
  int mode;

  /* Privilege level.  */
  u_char privilege;

#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-04-12 08:04 */
  vector genblock;
#endif /* !_CENTEC_ */

  /* Vector of modes.  */
  vector modes;

  /* Vector of configuration output function.  */
  vector config_write;

  /* Node to be executed.  */
  struct cli_node *exec_node;

  /* Possibly matched cel.  Function internal use only.  */
  vector v;

  /* Traverse nodes. */
  vector t;

  /* Expand is needed.  Used by describe function.  */
  int expand_node;

  /* Show node match result.  */
  int show_node;

  /* Parsed argc and argv.  */
  int argc;
#define CLI_ARGC_MAX               128
#define CLI_ARGV_MAX_LEN           256
  char *argv[CLI_ARGC_MAX];

  /* For Output Modifier.  */
  struct cli_node *modifier_node;
  int argc_modifier;
  char *argv_modifier[CLI_ARGC_MAX];
  char *rem;

  /* Parse failed character pointer to show invalid input.  */
  char *invalid;

  /* To show pipe pointer.  */
  char *pipe;

  /* Advanced mode.  */
  int advanced;
};

/* Argument to cli functions.  */
struct cli
{
  /* CLI element.  */
  struct cli_element *cel;

  /* User input string.  */
  char *str;

  /* Output function to be used by cli_out().  */
  CLI_OUT_FUNC out_func;

  /* Output function's first argument.  */
  void *out_val;

  /* Arbitrary information for line.  */
  void *line;

  /* Auth required.  */
  int auth;

  /* Input source.  */
  int source;
#define CLI_SOURCE_USER                 0
#define CLI_SOURCE_FILE                 1

  /* For "line". */
  int line_type;
  int min;
  int max;

  /* Real CLI.  */
#if 0 
  /* Deleted by Alexander Liu */
  void *index;
  void *index_sub;
#endif  
  int mode;

  /* Current CLI status.  */
  enum {
    CLI_NORMAL,
    CLI_CLOSE,
    CLI_MORE,
    CLI_CONTINUE,
    CLI_MORE_CONTINUE,
    CLI_WAIT
  } status;

  /* Flags. */
  u_char flags;
#define CLI_FROM_PVR    (1 << 0)

  void *self;
  u_char privilege;
  struct cli_tree *ctree;

  /* Global variable.  */
  struct lib_globals *zg;
  struct ipi_vr *vr;

  /* Terminal length.  */
  int lines;

  /* Call back function.  */
  int (*callback) (struct cli *);
  int (*cleanup) (struct cli *);
  int32_t (*show_func) (struct cli *);
  int type;
  u_int32_t count;
  void *current;
  void *arg;
  u_int16_t/*afi_t*/ afi;
  u_int8_t/*safi_t*/ safi;

#ifdef HAVE_CUSTOM1
  /* L2 handling.  */
  u_int64_t port_range;
#endif /* HAVE_CUSTOM1 */
};

/* Configuration output function.  */

typedef int (*CLI_CONFIG_FUNC) (struct cli *cli);

/* CLI element.  */
struct cli_element
{
  /* Command line string.  */
  char *str;

  /* Function to execute this command.  */
  int (*func) (struct cli *, int, char **);

  /* Help strings array. */
  char **help;

  /* Unique key. */
  int key;

  /* Flags of the commands.  */
  u_int16_t flags;

  /* When CLI_FLAG_HIDDEN is defined, the command isn't used for
     completion.  */
#define CLI_FLAG_HIDDEN            (1 << 0)

  /* When CLI_FLAG_MODIFIER is define, output modifier '|' can be used
     for the command.  Usually the flag is set to show commands.  */
#define CLI_FLAG_MODIFIER          (1 << 1)

  /* This is only used by struct cli_node.  When the node is "show" or
     "*s=show" this flag should be set.  */
#define CLI_FLAG_SHOW              (1 << 2)

  /* This is only used by struct cli_node.  When the node is in
     parenthesis this flag is set.  */
#define CLI_FLAG_PAREN             (1 << 3)

  /* This node is in brace.  */
#define CLI_FLAG_TRAVERSE          (1 << 4)

  /* This node is recursive.  */
#define CLI_FLAG_RECURSIVE         (1 << 5)

  /* Execute CLI function before send it to IMI.  */
#define CLI_FLAG_LOCAL_FIRST       (1 << 6)

  /* This enforce not apply the pager to the output.  */
#define CLI_FLAG_NO_PAGER          (1 << 7)

  /* This flag is used for interface name match.  */
#define CLI_FLAG_IFNAME            (1 << 8)
  
  /* This is only used by WMI (Web Management Interface). */
#define CLI_FLAG_WMI               (1 << 9)

  /* Set if ignore PM command error is required at the IMI level. */
  /* NOTE: Generally, if a command is sent to PM and the PM is down 
           or not responsive, the IMI returns and error to the IMISH,
           and the command is aborted.
           This works in all the cases, where the IMI does not process
           the same commands as PM, except CLI mode change. 
           In other cases, when the IMI executes the same command as the protocol 
           daemon - e.g. "hostname WORD" goes to the ISIS (host.c) and IMI - 
           we need to ignore the error when the PM is down so the IMI can 
           process it. The IMISH will not get the "PM down" error.
   */

#define CLI_FLAG_IGN_PM_DOWN_ERR   (1 << 9)

  /* Send this command to all on the list, regardless of failures returned by some. */
#define CLI_FLAG_MCAST   (1 << 10)

#define CLI_PLUS_SIGN_FORBIDDEN   (1 << 11)

  /* Protocol module to which this command belongs. */
  u_int32_t module;

};

/* CLI tree node.  */
struct cli_node
{
  /* CLI token types.  */
  enum cli_token
    {
      cli_token_paren_open,
      cli_token_paren_close,
      cli_token_cbrace_open,
      cli_token_cbrace_close,
      cli_token_brace_open,
      cli_token_brace_close,
      cli_token_ifname_open,
      cli_token_ifname_close,
      cli_token_separator,
      cli_token_pipe,
      cli_token_redirect,
      cli_token_dot,
      cli_token_question,
      cli_token_range,
      cli_token_keyword,
      cli_token_alias,
      cli_token_line,
      cli_token_word,
      cli_token_ipv4,
      cli_token_ipv4_prefix,
      cli_token_ipv6,
      cli_token_ipv6_prefix,
      cli_token_time,
      cli_token_community,
      cli_token_mac_address,
      cli_token_ifname,
#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-02-28 09:01 */
      cli_token_gflashname,
      cli_token_gcfcardname,
      cli_token_gfilename,
      cli_token_gurl,
      cli_token_hour_minute,
      cli_token_chinese_date,
      cli_token_english_date,
      cli_token_month_day,
      cli_token_port,
#endif /* !_CENTEC_ */
      cli_token_unknown
    } type;

  /* String to be matched.  */
  char *str;

  /* Help string.  */
  char *help;

  /* Max and min.  */
  u_int32_t max;
  u_int32_t min;

  /* Pointer to CLI element.  */
  struct cli_element *cel;

  /* Vector of next nodes.  */
  vector keywords;

  /* Privilege node.  */
  u_char privilege;

  /* Same as cli_element's flags.  */
  u_int16_t flags;

  /* Reference count.  */
  u_int32_t refcnt;
};

/* Return value.  */
#define CLI_SUCCESS           0
#define CLI_ERROR             1
#define CLI_AUTH_REQUIRED     2
#define CLI_EOL               3

/* Privilege level.  */
#define PRIVILEGE_MIN         1
#define PRIVILEGE_VR_MAX     15
#define PRIVILEGE_PVR_MAX    16

#define PRIVILEGE_LEVEL_TWO          2
#define PRIVILEGE_LEVEL_THREE        3

#define PRIVILEGE_NORMAL        PRIVILEGE_MIN
#ifdef HAVE_VR
#define PRIVILEGE_MAX           (PRIVILEGE_PVR_MAX)
#define PRIVILEGE_ENABLE(V)                                                   \
    ((V)->id == 0 ? PRIVILEGE_PVR_MAX : PRIVILEGE_VR_MAX)
#else /* HAVE_VR */
#define PRIVILEGE_MAX           (PRIVILEGE_VR_MAX)
#define PRIVILEGE_ENABLE(V)     (PRIVILEGE_VR_MAX)
#endif /* HAVE_VR */

/* Max length of each token.  */
#define MAX_TOKEN_LENGTH   256
#ifndef CNOS_OS
#define PM_LOCAL        0ULL
#define PM_NSM          (1ULL << IPI_PROTO_NSM)
#define PM_RIP          (1ULL << IPI_PROTO_RIP)
#define PM_RIPNG        (1ULL << IPI_PROTO_RIPNG)
#define PM_OSPF         (1ULL << IPI_PROTO_OSPF)
#define PM_OSPF6        (1ULL << IPI_PROTO_OSPF6)
#define PM_ISIS         (1ULL << IPI_PROTO_ISIS)
#define PM_BGP          (1ULL << IPI_PROTO_BGP)
#define PM_LDP          (1ULL << IPI_PROTO_LDP)
#define PM_RSVP         (1ULL << IPI_PROTO_RSVP)
#define PM_PDM          (1ULL << IPI_PROTO_PIMDM)
#define PM_PIM          (1ULL << IPI_PROTO_PIMSM)
#define PM_PIM6         (1ULL << IPI_PROTO_PIMSM6)
#define PM_DVMRP        (1ULL << IPI_PROTO_DVMRP)
#define PM_AUTH         (1ULL << IPI_PROTO_8021X)
#define PM_ONM          (1ULL << IPI_PROTO_ONM)
#define PM_LACP         (1ULL << IPI_PROTO_LACP)
#define PM_STP          (1ULL << IPI_PROTO_STP)
#define PM_RSTP         (1ULL << IPI_PROTO_RSTP)
#define PM_MSTP         (1ULL << IPI_PROTO_MSTP)
#define PM_IMI          (1ULL << IPI_PROTO_IMI)
#define PM_IMISH        (1ULL << IPI_PROTO_IMISH)
#define PM_PIMPKTGEN    (1ULL << IPI_PROTO_PIMPKTGEN)
#define PM_RMON         (1ULL << IPI_PROTO_RMON)
#define PM_HSL          (1ULL << IPI_PROTO_HSL)

#define PM_UCAST        PM_RIP|PM_RIPNG|PM_OSPF|PM_OSPF6|PM_ISIS|PM_BGP
#define PM_MCAST        PM_PDM|PM_PIM|PM_PIM6|PM_DVMRP
#define PM_MCAST_VR     PM_PDM|PM_PIM|PM_PIM6
#define PM_MPLS         PM_LDP|PM_RSVP
#define PM_L2           PM_AUTH|PM_LACP|PM_STP|PM_RSTP|PM_MSTP|PM_RMON|PM_ONM
#define PM_ALL          PM_NSM|PM_UCAST|PM_MCAST|PM_MPLS|PM_L2|PM_PIMPKTGEN|PM_RMON | PM_HSL

#define PM_IF           PM_NSM|PM_RIP|PM_RIPNG|PM_OSPF|PM_OSPF6|PM_ISIS|PM_MCAST|PM_MPLS|PM_L2
#define PM_IFDESC       PM_NSM|PM_STP|PM_RSTP|PM_MSTP|PM_MCAST|PM_RIP|PM_RIPNG|PM_OSPF|PM_ONM
#define PM_TUN_IF       PM_NSM|PM_MCAST|PM_OSPF|PM_OSPF6|PM_RIP|PM_RIPNG|PM_ISIS|PM_MPLS
#define PM_VR           PM_NSM|PM_IMI|PM_UCAST|PM_MCAST_VR
#define PM_VRF          PM_NSM|PM_RIP|PM_OSPF|PM_BGP|PM_MCAST_VR
#define PM_VRRP         PM_NSM
#define PM_RMM          PM_NSM
#define PM_KEYCHAIN     PM_RIP
#define PM_ACCESS       PM_NSM|PM_UCAST|PM_LDP|PM_MCAST
#define PM_PREFIX       PM_RIP|PM_RIPNG|PM_OSPF|PM_OSPF6|PM_BGP
#define PM_DISTRIBUTE   PM_RIP|PM_RIPNG
#define PM_RMAP         PM_NSM|PM_UCAST
#define PM_IFRMAP       PM_RIPNG
#define PM_LOG          PM_ALL
#define PM_HOSTNAME     PM_ISIS
#define PM_NPF          PM_NSM
#define PM_IGMP         PM_NSM
#define PM_MLD          PM_NSM
#define PM_CAL          PM_NSM|PM_RIP|PM_LACP|PM_STP|PM_RSTP|PM_MSTP
#define PM_FM           PM_ALL
#ifdef HAVE_SNMP
#define PM_SNMP_DBG     PM_NSM|PM_RIP|PM_OSPF|PM_OSPF6|PM_BGP|PM_LDP\
|PM_RSVP|PM_PIM|PM_ISIS|PM_RSTP|PM_STP|PM_AUTH|PM_LACP|PM_PIM6\
|PM_DVMRP|PM_RMON
#endif /* HAVE_SNMP */

#define MODULE_BIT2ID(PM)                                                     \
    (CHECK_FLAG ((PM), PM_NSM)   ? IPI_PROTO_NSM    :                         \
     CHECK_FLAG ((PM), PM_RIP)   ? IPI_PROTO_RIP    :                         \
     CHECK_FLAG ((PM), PM_RIPNG) ? IPI_PROTO_RIPNG  :                         \
     CHECK_FLAG ((PM), PM_OSPF)  ? IPI_PROTO_OSPF   :                         \
     CHECK_FLAG ((PM), PM_OSPF6) ? IPI_PROTO_OSPF6  :                         \
     CHECK_FLAG ((PM), PM_ISIS)  ? IPI_PROTO_ISIS   :                         \
     CHECK_FLAG ((PM), PM_BGP)   ? IPI_PROTO_BGP    :                         \
     CHECK_FLAG ((PM), PM_LDP)   ? IPI_PROTO_LDP    :                         \
     CHECK_FLAG ((PM), PM_RSVP)  ? IPI_PROTO_RSVP   :                         \
     CHECK_FLAG ((PM), PM_PDM)   ? IPI_PROTO_PIMDM  :                         \
     CHECK_FLAG ((PM), PM_PIM)   ? IPI_PROTO_PIMSM  :                         \
     CHECK_FLAG ((PM), PM_PIM6)  ? IPI_PROTO_PIMSM6 :                         \
     CHECK_FLAG ((PM), PM_DVMRP) ? IPI_PROTO_DVMRP  :                         \
     CHECK_FLAG ((PM), PM_MSTP)  ? IPI_PROTO_MSTP   :                         \
     CHECK_FLAG ((PM), PM_RSTP)  ? IPI_PROTO_RSTP   :                         \
     CHECK_FLAG ((PM), PM_STP)   ? IPI_PROTO_STP    :                         \
     CHECK_FLAG ((PM), PM_AUTH)  ? IPI_PROTO_8021X  :                         \
     CHECK_FLAG ((PM), PM_ONM)   ?  IPI_PROTO_ONM   :                         \
     CHECK_FLAG ((PM), PM_LACP)  ? IPI_PROTO_LACP   :                         \
     CHECK_FLAG ((PM), PM_IMI)   ? IPI_PROTO_IMI    :                         \
     CHECK_FLAG ((PM), PM_IMISH) ? IPI_PROTO_IMISH  :                         \
     CHECK_FLAG ((PM), PM_PIMPKTGEN) ? IPI_PROTO_PIMPKTGEN :                  \
     CHECK_FLAG ((PM), PM_RMON)  ? IPI_PROTO_RMON   :                          \
     CHECK_FLAG ((PM), PM_HSL)   ? IPI_PROTO_HSL :                      \
     CHECK_FLAG ((PM), PM_WLAN)   ? IPI_PROTO_WLAN :                  \
     IPI_PROTO_UNSPEC)

#define MODULE_ID2BIT(MODULE_ID)        (1ULL << (MODULE_ID))

#define MODULE_ID2MODE(MODULE_ID)                                             \
    ((MODULE_ID) == IPI_PROTO_RIP    ? RIP_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_RIPNG  ? RIPNG_MODE  :                          \
     (MODULE_ID) == IPI_PROTO_OSPF   ? OSPF_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_OSPF6  ? OSPF6_MODE  :                          \
     (MODULE_ID) == IPI_PROTO_ISIS   ? ISIS_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_BGP    ? BGP_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_LDP    ? LDP_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_RSVP   ? RSVP_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_PIMDM  ? PDM_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_PIMSM  ? PIM_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_PIMSM6 ? PIM6_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_DVMRP  ? DVMRP_MODE  :                          \
     (MODULE_ID) == IPI_PROTO_8021X  ? AUTH8021X_MODE :                       \
     (MODULE_ID) == IPI_PROTO_ONM    ? ONMD_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_STP    ? STP_MODE    :                          \
     (MODULE_ID) == IPI_PROTO_RSTP   ? RSTP_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_MSTP   ? MSTP_MODE   :                          \
     (MODULE_ID) == IPI_PROTO_RMON   ? RMON_MODE   :                        \
     PAL_FALSE)
#endif
/* CLI parser result codes.  */
#define CLI_PARSE_SUCCESS             0
#define CLI_PARSE_INCOMPLETE          1
#define CLI_PARSE_INCOMPLETE_PIPE     2
#define CLI_PARSE_EMPTY_LINE          3
#define CLI_PARSE_AMBIGUOUS           4
#define CLI_PARSE_NO_MATCH            5
#define CLI_PARSE_NO_MODE             6
#define CLI_PARSE_ARGV_TOO_LONG       7

/* Common descriptions. */
#define CLI_IFNAME_STR  "Interface name"
#define CLI_SHOW_STR "Show running system information"
#define CLI_STATS_STR "Statistics"
#define CLI_SHOW_MEMORY_STR  "Memory information"
#define CLI_SHOW_BRIDGE_STR  "Bridge information"
#define CLI_NO_STR "Negate a command or set its defaults"
#define CLI_WRITE_STR "Write running configuration to memory, file or terminal"
#define CLI_CLEAR_STR "Reset functions"
#define CLI_FIB_STR "FIB information"
#define CLI_IP_STR "Internet Protocol (IP)"
#define CLI_ALL_IP "All multi-destination for the specified tunnel"
#define CLI_ARP_STR "Address Resolution Protocol (ARP)"
#define CLI_IPV6_STR "Internet Protocol version 6 (IPv6)"
#define CLI_IMI_STR "Integrated Management Interface (IMI)"
#define CLI_NSM_STR "Network Service Module (NSM)"
#define CLI_NSM_MCAST_STR "IPv4 multicast"
#define CLI_NSM_MCAST6_STR "IPv6 multicast"
#define CLI_RIP_STR "Routing Information Protocol (RIP)"
#define CLI_RIPNG_STR "Routing Information Protocol (RIP) for IPv6"
#define CLI_OSPF_STR "Open Shortest Path First (OSPF)"
#define CLI_OSPF6_STR "Open Shortest Path First (OSPF) for IPv6"
#define CLI_OSPF6_IF_STR "OSPFv3 interface commands"
#define CLI_ISIS_STR "Intermediate System - Intermediate System (IS-IS)"
#define CLI_CLNS_STR "Connectionless-Mode Network Service (CLNS)"
#define CLI_BGP_STR "Border Gateway Protocol (BGP)"
#define CLI_8021X_STR "IEEE 802.1X Port-Based Access Control"
#define CLI_ONMD_STR "LLDP and EFM_OAM"
/*MAC-based auth Enhancement*/
#define CLI_MAC_AUTH_STR "IPI MAC-Based Authentication"
#define CLI_LACP_STR "Link Aggregation Control Protocol (LACP)"
#define CLI_GVRP_STR "GARP VLAN Registration Protocol (GVRP)"
#define CLI_GMRP_STR "GARP Multicast Registration Protocol (GMRP)"
#define CLI_STP_STR "Spanning Tree Protocol (STP)"
#define CLI_RSTP_STR "Rapid Spanning Tree Protocol (RSTP)"
#define CLI_MSTP_STR "Multiple Spanning Tree Protocol (MSTP)"
#define CLI_CFM_STR "Connectivity Fault Management Protocol (CFM)"
#define CLI_MPLS_STR "Multi-Protocol Label Switching (MPLS)"
#define CLI_MPLS_CMD_STR "Multi-Protocol Label Switching (MPLS)"
#define CLI_LDP_STR  "Label Distribution Protocol (LDP)"
#define CLI_RSVP_STR "Resource Reservation Protocol (RSVP)"
#define CLI_IGMP_STR "Internet Group Management Protocol (IGMP)"
#define CLI_IGMP_SNOOP_STR "Layer 2 Snooping"
#define CLI_MLD_STR "Multicast Listener Discovery (MLD)"
#define CLI_MLD_SNOOP_STR "Layer 2 Snooping"
#define CLI_PIM_STR "Protocol Independent Multicast (PIM)"
#define CLI_PIMSM_STR "Sparse Mode (PIM-SM)"
#define CLI_PIMDM_STR "Dense Mode (PIM-DM)"
#define CLI_PIM6_STR "Protocol Independent Multicast (PIM) for IPv6"
#define CLI_PIMSM6_STR "Sparse Mode (PIM-SM) for IPv6"
#define CLI_PIMDM6_STR "Dense Mode (PIM-DM) for IPv6"
#define CLI_DVMRP_STR "Distance Vector Multicast Routing Protocol (DVMRP)"
#define CLI_NEIGHBOR_STR "Specify a neighbor router"
#define CLI_AF_STR "Address family"
#define CLI_AFM_STR "Address family modifier"
#define CLI_DEBUG_STR "Debugging functions (see also 'undebug')"
#define CLI_UNDEBUG_STR "Disable debugging functions (see also 'debug')"
#define CLI_UNDEBUG_ALL_STR "Turn off all Debugging"
#define CLI_ROUTER_STR "Enable a routing process"
#define CLI_AS_STR "AS number"
#define CLI_MATCH_STR "Match values from routing table"
#define CLI_SET_STR "Set values in destination routing protocol"
#define CLI_OUT_STR "Filter outgoing routing updates"
#define CLI_IN_STR  "Filter incoming routing updates"
#define CLI_V4NOTATION_STR "specify by IPv4 address notation(e.g. 0.0.0.0)"
#define CLI_INTERFACE_STR "Interface information"
#define CLI_SECONDS_STR "<1-65535> Seconds"
#define CLI_ROUTE_STR "Routing Table"
#define CLI_PREFIX_LIST_STR "Build a prefix list"
#define CLI_CLEAR_PREFIX_LIST_STR "Clear a prefix list"
#define CLI_VRF_STR "VPN Routing/Forwarding instance"
#define CLI_VRF_DISP_STR "Display routes from a VPN Routing/Forwarding instance"
#define CLI_VRF_NAME_STR "VPN Routing/Forwarding instance name"
#define CLI_VR_STR "Virtual-router configuration"
#define CLI_VR_NAME_STR "Virtual-router name"
#define CLI_VR_ID_STR "Virtual Router ID"
#define CLI_VR_EXPORT_STR "Export routes between virtual-routers"
#define CLI_ND_STR "IPv6 interface Neighbor Discovery subcommands"
#define CLI_MIP_STR "Mobile IP"
#define CLI_HA_STR "Home agent"
#define CLI_NAT_STR "Network Address Translation (NAT)"
#define CLI_TUNNEL_STR "protocol-over-protocol tunneling"
#define CLI_TUNNEL_MODE_STR "tunnel encapsulation method"
#define CLI_FILTER_STR "Firewall"
#define CLI_ACCESS_GROUP_STR "Access Group "
#define CLI_GMPLS_STR "Generalized MPLS (GMPLS)"
#define CLI_GMPLS_LINK_ID_STR "Link Local/Remote Identifiers"
#define CLI_GMPLS_PROTECTION_TYPE_STR "Link Protection Type"
#define CLI_GMPLS_CAPABILITY_STR "Switching Capability Type"
#define CLI_GMPLS_ENCODING_TYPE_STR "LSP Encoding"
#define CLI_GMPLS_MIN_LSP_BW_STR "Minimum LSP Bandwidth"
#define CLI_GMPLS_SDH_INDICATION_STR "SONET/SDH Indication"
#define CLI_GMPLS_RISK_GROUP_STR "Shared Risk Link Group"
#define CLI_DHCP_STR "Dynamic Host Configuration Protocol (DHCP)"
#define CLI_PPPOE_STR "Point-to-Point over Ethernet (PPPoE)"
#define CLI_VRRP_STR "Virtual Router Redundancy Protocol (VRRP)"
#define CLI_KEY_STR "Authentication key management"
#define CLI_CHAIN_STR "Key-chain management"
#define CLI_KEYSTRING_STR "Set key string"
#define CLI_KEYCHAIN_ACCEPT_LIFETIME_STR "Set accept lifetime of the key"
#define CLI_KEYCHAIN_SEND_LIFETIME_STR "Set send lifetime of the key"
#define CLI_ROUTEMAP_STR "Create route-map or enter route-map command mode"
#define CLI_ACCESS_STR "Add an access list entry"
#define CLI_ACCESS_GRP_STR "Add an access group entry"
#define CLI_ACCESS_LIST_NAME_STR "Access-list name"
#define CLI_ACCESS_ZEBOS_STR "IP ZebOS access-list name"
#define CLI_ACCESS6_ZEBOS_STR "IPv6 ZebOS access-list name"
#define CLI_ACCESS_PERMIT_STR "Specify packets to forward"
#define CLI_ACCESS_DENY_STR "Specify packets to reject"
#define CLI_PREFIX_LIST_NAME_STR "Name of a prefix list"
#define CLI_PREFIX_NETWORK_STR "IP prefix <network>/<length>, e.g., 35.0.0.0/8"
#define CLI_PREFIX6_NETWORK_STR "IPv6 prefix <network>/<length>, e.g., 3ffe::/16"
#define CLI_PREFIX_LENGTH_MIN "Minimum prefix length"
#define CLI_PREFIX_LENGTH_MAX "Maximum prefix length"
#define CLI_PREFIX_LENGTH_MIN_MATCH "Minimum prefix length to be matched"
#define CLI_PREFIX_LENGTH_MAX_MATCH "Maximum prefix length to be matched"
#define CLI_PREFIX_SEQ_STR "Sequence number of an entry"
#define CLI_PREFIX_SEQNUM_STR "Sequence number"
#define CLI_DISTRIBUTE_LIST_STR "Filter networks in routing updates"
#define CLI_DISTRIBUTE_PREFIX_STR "Filter prefixes in routing updates"
#define CLI_DISTRIBUTE_IN_STR "Filter incoming routing updates"
#define CLI_DISTRIBUTE_OUT_STR "Filter outgoing routing updates"
#define CLI_USER_STR "User management"
#define CLI_USER_NAME_STR "User login name"
#define CLI_VLAN_STR "Add, delete, or modify values associated with a single VLAN"
#define CLI_RMON_STR "Remote Monitoring Protocol (RMON)"

#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-01-10 09:14 */
#define CLI_DHCPRELAY_STR   "DHCP relay agent"
#define CLI_DHCPSNOOPING_STR   "DHCP snooping"
#define CLI_HOUR_MINUTE_STR     "Specify the hour and minute"
#define CLI_CHINESE_DATE_STR    "Specify the date (YYYY/MM/DD format, year range is [2000, 2037])"
#define CLI_ENGLISH_DATE_STR    "Specify the date (MM/DD/YYYY format, year range is [2000, 2037])"
#define CLI_MONTH_DAY_STR       "Specify the date for current year (MM/DD format)"
#define CLI_SCHEDULE_STR        "Schedule parameters"
#define CLI_SHOW_TCAM_STR  "Show TCAM  information"
#define CLI_SNMPD_STR   "snmp agent"
#define CLI_SSHD_STR    "SSH server"
#define CLI_ENABLE_STR   "Enable a command "/* Modified by Zhichuan 2008-01-19 09:14 */
#define CLI_DISABLE_STR    "Disable a command or set its defaults "
#define CLI_ETHERNET      "Configure layer-2"

#endif /* !_CENTEC_ */

#define CONFIG_MPLS_STR    "Configure MPLS specific attributes"
#define CONFIG_MPLSTP_STR  "Configure MPLS-TP specific attributes"
#define NO_CONFIG_MPLS_STR "Unset MPLS specific attributes"
#define SHOW_MPLS_STR      "Show MPLS specific data"
#define CLEAR_MPLS_STR     "Clear MPLS specific data"
#define CLI_TUNNEL_NAME_STR "Name of MPLS tunnel"
/*add by dingyi for mlag, 2011-07-17*/
#define CONFIG_MLAG_STR    "Configure MLAG specific attributes"
#define CLI_MLAG_STR "Multi-Chassis Link Aggregation"

#define CLI_MTRACE_SRC_STR "Source to trace route from"
#define CLI_MTRACE_DST_STR "Destination of multicast trace"
#define CLI_MTRACE_GRP_STR "Group for multicast trace or 0.0.0.0"
#define CLI_MTRACE_TTL_STR "Time-To-Live for multicast trace request"
#define CLI_MTRACE_STR "Trace multicast path from source to destination"
#define CLI_MSTAT_STR "Show statistics after multiple multicast traceroutes"

/* CPU Database related strings */
#define CLI_STACKING_STR "Switch box Stacking"
#define CLI_STACK_NUM_CPU_STR "Number of CPUs in the system"
#define CLI_STACK_DB_STR "Number of CPUs in the Database"
#define CLI_STACK_MASTER_DEV_STR "Master CPU Device in the Database"


#ifdef HAVE_RSVPD
#define PRI_STR            "Configure primary LSP option"
#define SEC_STR            "Configure secondary LSP option"
#define NO_PRI_STR         "Unset primary LSP option"
#define NO_SEC_STR         "Unset secondary LSP option"
#endif /* HAVE_RSVPD */

#ifdef _CENTEC_ /* Added by chengw 2011-09-23 */
#define CLI_MPLS_TP_STR "Mpls TP"
#endif

#ifndef _CENTEC_ /*should be use ifdef HAVE_IPV6*/
#define NEIGHBOR_CMD       "neighbor (A.B.C.D|X:X::X:X) "
#define NO_NEIGHBOR_CMD    "no neighbor (A.B.C.D|X:X::X:X) "
#define NEIGHBOR_ADDR_STR  "IP address","IPv6 address"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address","Neighbor IPv6 address","Neighbor tag"
#else
#define NEIGHBOR_CMD       "neighbor A.B.C.D "
#define NO_NEIGHBOR_CMD    "no neighbor A.B.C.D "
#define NEIGHBOR_ADDR_STR  "IP address"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address","Neighbor tag"
#endif /* HAVE_IPV6 */

/* Multicast static route CLI strings */
#define CLI_MROUTE_STR      "Configure static multicast routes"
#define CLI_HOST_NAME      "This system's network name, must be less than 64 characters"/* modify by wulx for bug 37849, 2017-11-07 */

/*modified by huangxt for json.*/
#define CLI_JSON_STR "Show information with json format"

/* Allocate a new CLI stree.  */

struct cli_tree *cli_tree_new ();

int cli_parse (struct cli_tree *, int, u_char, char *, int, int);
void cli_describe (struct cli *, struct cli_tree *, int cli_mode, u_char,
                   char *, int);
char **cli_complete (struct cli_tree *, int cli_mode, u_char, char *);

int cli_install_gen (struct cli_tree *, int cli_mode, u_char, u_int16_t,
                     struct cli_element *);
int cli_install (struct cli_tree *, int cli_mode, struct cli_element *);
int cli_install_hidden (struct cli_tree *, int cli_mode,
                        struct cli_element *);
int cli_install_imi (struct cli_tree *, int cli_mode, u_int32_t, u_char,
                     u_int16_t, struct cli_element *);

void cli_install_shortcut (struct cli_tree *, int cli_mode, char *, char *,
                           char *);

int cli_install_config (struct cli_tree *, int, CLI_CONFIG_FUNC func);

void cli_set_node_flag (struct cli_tree *, int, char *,
                        u_int16_t);
void cli_sort (struct cli_tree *);
void cli_free_arguments (struct cli_tree *);

void cli_install_default (struct cli_tree *, int);
void cli_install_default_family (struct cli_tree *, int);

char *cli_interface_resolve (char *, int, char *, char *);

const char *cli_mode_str (int);
const char *cli_prompt_str (int);

/* Utility function.  */
char *argv_concat (char **, int, int);
int cmd_str2int (char *, int *);
int cmd_str2num (const char *str, int *ret);
int cmd_hexstr2uint_array (const char *str, u_int32_t *buf, u_int32_t buf_len);
int cli_mask_check (struct in_addr);

#ifdef _CENTEC_ /* Modified by Alexander Liu 2007-03-09 16:45 */
int cli_str2tm(char *, struct tm*);
#endif /* !_CENTEC_ */

#if 0
#define CLI_INSTALL(Z, M, PM, C)                                              \
    do {                                                                      \                                                               \
        cli_install_gen ((Z)->ctree, M, PRIVILEGE_NORMAL, 0, C);              \
    } while (0)

#define CLI_INSTALL_HIDDEN(Z, M, PM, C)                                       \
    do {                                                                      \
        cli_install_gen ((Z)->ctree, M, PRIVILEGE_NORMAL, CLI_FLAG_HIDDEN, C);\
    } while (0)
#endif

#define CLI_GET_INTEGER(NAME,V,STR)                                           \
    do {                                                                      \
      int retv;                                                               \
      (V) = cmd_str2int ((STR), &retv);                                       \
      if (retv < 0)                                                           \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_INTEGER_RANGE(NAME,V,STR,MIN,MAX)                             \
    do {                                                                      \
      int retv;                                                               \
      (V) = cmd_str2int ((STR), &retv);                                       \
      if (retv < 0 || (V) < (MIN) || (V) > (MAX))                             \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_HEX_INTEGER_RANGE(NAME,V,STR,MIN,MAX)                         \
    do {                                                                      \
      int retv;                                                               \
      (V) = cmd_str2num ((STR), &retv);                                       \
      if (retv < 0 || (V) < (MIN) || (V) > (MAX))                             \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_HEX_UINT32_RANGE(NAME,V,STR,MIN,MAX)                          \
    do {                                                                      \
      int retv;                                                               \
      if (strncmp ((STR), "-", 1) == 0)                                      \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
      (V) = cmd_str2num ((STR), &retv);                                       \
      if (retv < 0 || (V) < (MIN) || (V) > (MAX))                             \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_INTEGER64(NAME,V,INT,STR,MIN,MAX)                             \
    do {                                                                      \
      int retv;                                                               \
      int val = 0;                                                            \
      (V) = cmd_str2int ((STR), &retv);                                       \
      pal_mem_cpy (&INT.l[0], &val, 4);                                       \
      pal_mem_cpy (&INT.l[1], &(V), 4);                                       \
      if (retv < 0 || (V) < (MIN) || (V) > (MAX))                             \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_UINT32(NAME,V,STR)                                            \
    do {                                                                      \
      int retv;                                                               \
      if (pal_strncmp ((STR), "-", 1) == 0)                                   \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
      (V) = cmd_str2int ((STR), &retv);                                       \
      if (retv < 0)                                                           \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_UINT32_RANGE(NAME,V,STR,MIN,MAX)                              \
    do {                                                                      \
      char *endptr = NULL;                                                    \
      (V) = pal_strtou32 ((STR), &endptr, 10);                                \
      if (*endptr != '\0' || (V) < (u_int32_t)(MIN)                           \
          || (V) > (u_int32_t)(MAX))                                          \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_IPV4_ADDRESS(NAME,V,STR)                                      \
    do {                                                                      \
      int retv;                                                               \
      retv = pal_inet_pton (AF_INET, (STR), &(V));                            \
      if (!retv)                                                              \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_IPV4_PREFIX(NAME,V,STR)                                       \
    do {                                                                      \
      int retv;                                                               \
      retv = str2prefix_ipv4 ((STR), &(V));                                   \
      if (retv <= 0)                                                          \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#ifdef HAVE_IPV6
#define CLI_GET_IPV6_ADDRESS(NAME,V,STR)                                      \
    do {                                                                      \
      int retv;                                                               \
      retv = pal_inet_pton (AF_INET6, (STR), &(V));                           \
      if (!retv)                                                              \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)

#define CLI_GET_IPV6_PREFIX(NAME,V,STR)                                       \
    do {                                                                      \
      int retv;                                                               \
      retv = str2prefix_ipv6 ((STR), &(V));                                   \
      if (retv <= 0)                                                          \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)
#endif /* HAVE_IPV6 */

#define CLI_GET_RMAP_ORIGIN(NAME,V,ARGC,STR)                                  \
    do {                                                                      \
      if ((ARGC) == 0)                                                        \
        (V) = NULL;                                                           \
      else if (pal_strncmp ((STR), "ig", 2) == 0)                             \
        (V) = "igp";                                                          \
      else if (pal_strncmp ((STR), "e", 1) == 0)                              \
        (V) = "egp";                                                          \
      else if (pal_strncmp ((STR), "in", 2) == 0)                             \
        (V) = "incomplete";                                                   \
      else                                                                    \
        {                                                                     \
          cli_out (cli, "%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    } while (0)


#define M_HELPSTR_MODULE_NUMBER "Module number"
#define M_HELPSTR_SLOT_NUMBER   "Slot number"
#define M_HELPSTR_PORT_NUMBER   "Port number"

#define M_PORT_CONN_STRING          "."
#define M_PORT_CONN_CHAR            '.'
#define M_PORT_RANGE_CHAR           '-'
#define M_PORT_SPLIT_CHAR           ','

#define M_PORT_BUFSIZE              64

#define E_GEN_FILE_SUCCESS  0x00
#define E_GEN_FILE_INALID   0x01

#define CLISH_RPP_PASSWORD_LEN      128     /* max length, not counting NUL */

#define CLISH_RPP_BUF_LEN      1024     /* buffer length*/

#define CLISH_RPP_ECHO_OFF          0x01    /* Turn off echo (default). */
#define CLISH_RPP_ECHO_ON           0x02    /* Leave echo on. */
#define CLISH_RPP_WITH_TIMEOUT      0x04    /* fail if timeout */
#define CLISH_RPP_CONFIRM_MODE      0x08    /* confirm mode: only accept 1 char */

typedef enum {
    E_MATCH_NONE = 0,
    E_MATCH_MODULE_INCOMPLETE,
    E_MATCH_MODULE_FULL,
    E_MATCH_SLOT_CON_FULL,
    E_MATCH_SLOT_INCOMPLETE,
    E_MATCH_SLOT_FULL,
    E_MATCH_PORT_CON_FULL,
    E_MATCH_PORT_INCOMPLETE,
    E_MATCH_PORT_FULL,
    E_MATCH_MAX
} PortMatch_e;

/* process tokens */
int split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
                 char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);
int free_tokens(char ***pppTokArr);
/* create temp file */
int ctc_create_tempfile(char *szTemplate);

/* begin added by zongyi for bug 5409 */
extern int check_space_available(char * filename, int64_t size);
#endif /* _LCSH_CLI_H */

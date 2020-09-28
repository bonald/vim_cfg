/****************************************************************************
 * ctc_cmd.h :         ctc_cmd header
 *
 * Copyright (C) 2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         V1.0
 * Date           :         2010-7-28
 * Reason         :         First Create
 ****************************************************************************/

#ifndef _CTC_CMD_H
#define _CTC_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ctc_vti_vec.h"
#include "ctc_vti.h"

#define HOST_NAME 32

#define MAX_OPTIONAL_CMD_NUM 256 /* max optional cmd number in {} */

#define MTYPE_VECTOR 0
#define MTYPE_VTI_HIST 0

typedef enum best_match_type_s
{
    CTC_CMD_EXACT_MATCH = 0,
    CTC_CMD_PARTLY_MATCH,
    CTC_CMD_EXTEND_MATCH,
    CTC_CMD_IMCOMPLETE_MATCH
} best_match_type_t;

/* There are some command levels which called from command node. */
enum ctc_node_type_e
{
    CTC_AUTH_NODE,              /* Authentication mode of vty interface. */
    CTC_VIEW_NODE,              /* View node. Default mode of vty interface. */
    CTC_AUTH_ENABLE_NODE,       /* Authentication mode for change enable. */
    CTC_ENABLE_NODE,            /* Enable node. */
    CTC_CONFIG_NODE,            /* Config node. Default mode of config file. */
    CTC_DEBUG_NODE,             /* Debug node. */
    CTC_AAA_NODE,               /* AAA node. */
    CTC_KEYCHAIN_NODE,          /* Key-chain node. */
    CTC_KEYCHAIN_KEY_NODE,      /* Key-chain key node. */
    CTC_INTERFACE_NODE,         /* Interface mode node. */
    CTC_MASC_NODE,              /* MASC for multicast.  */
    CTC_IRDP_NODE,              /* ICMP Router Discovery Protocol mode. */
    CTC_IP_NODE,                /* Static ip route node. */
    CTC_ACCESS_NODE,            /* Access list node. */
    CTC_PREFIX_NODE,            /* Prefix list node. */
    CTC_ACCESS_IPV6_NODE,       /* Access list node. */
    CTC_PREFIX_IPV6_NODE,       /* Prefix list node. */
    CTC_AS_LIST_NODE,           /* AS list node. */
    CTC_COMMUNITY_LIST_NODE,    /* Community list node. */
    CTC_RMAP_NODE,              /* Route map node. */
    CTC_SMUX_NODE,              /* SNMP configuration node. */
    CTC_DUMP_NODE,              /* Packet dump node. */
    CTC_FORWARDING_NODE,        /* IP forwarding node. */
    CTC_VTI_NODE                /* Vty node. */
};
typedef enum ctc_node_type_e ctc_node_type_t;

/* Completion match types. */
enum ctc_match_type_e
{
    CTC_CTC_NO_MATCH,
    CTC_EXTEND_MATCH,
    CTC_IPV4_PREFIX_MATCH,
    CTC_IPV4_MATCH,
    CTC_IPV6_PREFIX_MATCH,
    CTC_IPV6_MATCH,
    CTC_RANGE_MATCH,
    CTC_VARARG_MATCH,
    CTC_PARTLY_MATCH,
    CTC_EXACT_MATCH,
    CTC_OPTION_MATCH,
    CTC_INCOMPLETE_CMD
};
typedef enum ctc_match_type_e ctc_match_type_t;

/* Node which has some commands and prompt string and configuration
   function pointer . */
struct ctc_cmd_node_s
{
    /* Node index. */
    ctc_node_type_t node;

    /* Prompt character at vty interface. */
    char prompt[HOST_NAME];

    /* Is this node's configuration goes to vtysh ? */
    int32 vtysh;

    /* Node's configuration write function */
    int32 (* func)(ctc_vti_t*);

    /* Vector of this node's command list. */
    vector cmd_vector;
};
typedef struct ctc_cmd_node_s ctc_cmd_node_t;

/* Structure of command element. */
struct ctc_cmd_element_s
{
    char* string;       /* Command specification by string. */
    int32 (* func) (struct ctc_cmd_element_s*, ctc_vti_t*, int, char**);
    char** doc;         /* Documentation of this command. */
    int32 daemon;         /* Daemon to which this command belong. */
    vector strvec;      /* Pointing out each description vector. */
    int32 cmdsize;        /* Command index count. */
    char* config;       /* Configuration string */
    vector subconfig;   /* Sub configuration string */
};
typedef struct ctc_cmd_element_s ctc_cmd_element_t;

/* Command description structure. */
struct ctc_cmd_desc_s
{
    char* cmd;      /* Command string. */
    char* str;        /* Command's description. */
    uint8 is_arg;
    uint8 is_var;
};
typedef struct ctc_cmd_desc_s ctc_cmd_desc_t;

/* Return value of the commands. */
#define CMD_SUCCESS              0
#define CMD_WARNING              1
#define CMD_ERR_NO_MATCH         2
#define CMD_ERR_AMBIGUOUS        3
#define CMD_ERR_INCOMPLETE       4
#define CMD_ERR_EXEED_ARGC_MAX   5
#define CMD_ERR_NOTHING_TODO     6
#define CMD_COMPLETE_FULL_MATCH  7
#define CMD_COMPLETE_MATCH       8
#define CMD_COMPLETE_LIST_MATCH  9
#define CMD_SUCCESS_DAEMON      10
#define CMD_SYS_ERROR 11

/* Argc max counts. */
#define CMD_ARGC_MAX   256

/* Turn off these macros when uisng cpp with extract.pl */
#ifndef VTISH_EXTRACT_PL

/* DEFUN for vti command interafce. Little bit hacky ;-). */
#define DEFUN(funcname, cmdname, cmdstr, helpstr) \
    int32 funcname(ctc_cmd_element_t*, ctc_vti_t*, int32, char**); \
    ctc_cmd_element_t cmdname = \
    { \
        cmdstr, \
        funcname, \
        helpstr \
    }; \
    int32 funcname \
        (ctc_cmd_element_t * self, ctc_vti_t * vty, int32 argc, char** argv)

#endif /* VTISH_EXTRACT_PL */

/* Some macroes */
#define CTC_CMD_OPTION(S)   ((S[0]) == '[')
extern int32 ctc_is_cmd_var(char* cmd);
#define CTC_CMD_VARIABLE(S) ctc_is_cmd_var(S)
#define CTC_CMD_VARARG(S)   ((S[0]) == '.')
#define CTC_CMD_RANGE(S)    ((S[0] == '<'))
#define CTC_CMD_NUMBER(S) ((S[0] <= '9') && (S[0] >= '0'))

#define CTC_CMD_IPV4(S)        ((sal_strcmp((S), "A.B.C.D") == 0))
#define CTC_CMD_IPV4_PREFIX(S) ((sal_strcmp((S), "A.B.C.D/M") == 0))
#define CTC_CMD_IPV6(S)        ((sal_strcmp((S), "X:X::X:X") == 0))
#define CTC_CMD_IPV6_PREFIX(S) ((sal_strcmp((S), "X:X::X:X/M") == 0))

#define MTYPE_VECTOR 0
#define MTYPE_VTI_HIST 0

extern char* ctc_strdup(char* str);
#define XSTRDUP(mtype, str) ctc_strdup(str)
#define install_element(ntype, cmd)  ctc_install_element(ntype, cmd)
#define uninstall_element(ntype, cmd) ctc_uninstall_element(ntype, cmd)

/*for-ctrlc some info*/
#define CTC_CMD_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit-1))) ? 1 : 0)
#define CTC_CMD_FOR_COMMAND    1  /*print cli cmd*/
#define CTC_CMD_FOR_TIME            2  /*print performace test time*/
#define CTC_CMD_FOR_QUIT            3  /*stop for-ctrlc after reporting error at once*/

/* Prototypes. */
void ctc_install_node(ctc_cmd_node_t*, int32 (*)(ctc_vti_t*));
void install_default(ctc_node_type_t);
void ctc_install_element(ctc_node_type_t, ctc_cmd_element_t*);
void ctc_uninstall_element(ctc_node_type_t, ctc_cmd_element_t*);
void ctc_sort_node();
vector ctc_cmd_make_strvec(char*);
void ctc_cmd_free_strvec(vector);
vector ctc_cmd_describe_command();
char** ctc_cmd_complete_command();
char* ctc_cmd_prompt(ctc_node_type_t);
int32 ctc_cmd_execute_command(vector, ctc_vti_t*, ctc_cmd_element_t * *);
int32 ctc_cmd_execute_command_strict(vector, ctc_vti_t*, ctc_cmd_element_t * *);
void ctc_config_replace_string(ctc_cmd_element_t*, char*, ...);
void ctc_cmd_init(int32);


int32 ctc_cmd_for_command(ctc_vti_t* vti);

int32 ctc_cmd_grep_command(ctc_vti_t* vti);

int32 ctc_cmd_grep(ctc_vti_t* vti);

int32 ctc_cmd_sort(struct ctc_vti_struct_s* vti);


#ifdef __cplusplus
}
#endif

#endif /* _CTC_CMD_H */


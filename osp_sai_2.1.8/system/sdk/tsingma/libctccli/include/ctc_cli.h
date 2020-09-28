/**
 @file ctc_cli.h

 @date 2010-7-9

 @version v2.0

  The file defines Macro, stored data structure for ctc cli
*/
#ifndef _CTC_CLI_H
#define _CTC_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "sal_types.h"

#include "ctc_vti.h"
#include "ctc_cmd.h"
#include "ctc_regex.h"
#include "ctc_common_cli.h"

/* CLI modes.  */

#define EXEC_MODE 0
#define CTC_SDK_MODE 1
#define CTC_CMODEL_MODE 2
#define CTC_SDK_OAM_CHAN_MODE 3
#define CTC_DEBUG_MODE 4
#define CTC_DBG_TOOL_MODE 5
#define CTC_CTCCLI_MODE 6
#define CTC_INTERNAL_MODE 7
#define CTC_APP_MODE 8
#define CTC_DKITS_MODE 9
#define CTC_STRESS_MODE 10

/* Return value.  */
#define CLI_SUCCESS           0
#define CLI_ERROR             1
#define CLI_AUTH_REQUIRED     2
#define CLI_EOL               3

/* Max length of each token.  */
#define MAX_TOKEN_LENGTH   256

/* Used for shield system API other than show command */
#undef SDK_INTERNAL_CLIS

/* Used for shield system API in show command */
#define SDK_INTERNAL_CLI_SHOW

/* Common descriptions. */
#define CTC_CLI_SHOW_STR "Show running system information"
#define CTC_CLI_NO_STR      "Negate a command or set its defaults"
#define CTC_CLI_CLEAR_STR   "Clear functions"
#define CTC_CLI_ENABLE      "Enable functions"
#define CTC_CLI_DISABLE     "Disable functions"
#define CTC_CLI_SHOW_SYS_MEM_STR  "Memory information"
#define CTC_CLI_DEBUG_STR  "Debugging functions"
#define CTC_CLI_BOOL_VAR_STR  "Boolean variable"


/* Multicast static route CLI strings */
#define CTC_CLI_GET_ARGC_INDEX_ENHANCE(str) ctc_cli_get_prefix_item(&argv[0], argc, str, 0)
#define CTC_CLI_GET_ARGC_INDEX(str) CTC_CLI_GET_ARGC_INDEX_ENHANCE(str)

#define CTC_CLI_GET_SPECIFIC_INDEX(str, idx) ctc_cli_get_prefix_item(&argv[idx], (argc - (idx)), str, sal_strlen(str))
#define CLI_CLI_STR_EQUAL(str, idx)  (0 == sal_strncmp((str), argv[(idx)], sal_strlen(str)))
#define CTC_CLI_STR_EQUAL_ENHANCE(str, idx)  \
    ((0 == sal_strncmp((str), argv[(idx)], sal_strlen(str))) && (sal_strlen(argv[idx]) == sal_strlen(str)))
#define INDEX_VALID(index)  (0xFF != (index))

#define CLI_MROUTE_STR      "Configure static multicast routes"

#define UINT64_STR_LEN      21

#define CTC_MAX_UINT16_VALUE 0xFFFF
#define CTC_MAX_UINT32_VALUE 0xFFFFFFFF
#define CTC_MAX_UINT8_VALUE 0xFF

#define MAX_FILE_NAME_SIZE      256

#define MAX_CLI_STRING_LEN 1000

struct ctc_l2_write_info_para_s
{
    char file[MAX_FILE_NAME_SIZE];
    void* pdata;
};
typedef struct ctc_l2_write_info_para_s ctc_l2_write_info_para_t;

typedef int (* CTC_CLI_OUT_FUNC) (const char* str, ...);
int ctc_cli_out(const char* fmt, ...);


#ifdef HAVE_ISO_MACRO_VARARGS
#define CTC_CLI(func_name, cli_name, cmd_str, ...)  \
    char* cli_name ## _help[] = {__VA_ARGS__, NULL}; \
    DEFUN(func_name, cli_name, cmd_str, cli_name ## _help)
#else
#define CTC_CLI(func_name, cli_name, cmd_str, ARGS...) \
    char* cli_name ## _help[] = {ARGS, NULL}; \
    DEFUN(func_name, cli_name, cmd_str, cli_name ## _help)
#endif

extern uint8 g_cli_parser_param_en;
extern uint32
ctc_cmd_get_value(int32* ret, char* name, char* str, uint32 min, uint32 max, uint8 type);
extern int32
ctc_cmd_str2int(char* str, int32* ret);
extern uint32
ctc_cmd_str2uint(char* str, int32* ret);
extern uint64
ctc_cmd_str2uint64(char* str, int32* ret);
extern void
    ctc_uint64_to_str(uint64 src, char dest[UINT64_STR_LEN]);
extern int32
ctc_cmd_str2nint (char *str, int32 *ret, uint32 *val);


extern int32
ctc_cmd_judge_is_num(char* str);

extern char*
ctc_cli_get_debug_desc(unsigned char level);


#define CTC_CLI_CHK_ARGV(argx) \
{\
         if((&argx - &argv[0]) >= argc)\
         {\
            ctc_cli_out("%% Invalid value\n"); \
            return CLI_ERROR;\
         }\
}

#define CTC_CLI_GET_INTEGER(NAME, V, STR)                \
    {                                                  \
        int32 retv;                                      \
        CTC_CLI_CHK_ARGV(STR);                          \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:%d\n",NAME,V);            \
        }                                                  \
        if (retv < 0)                                    \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_INTEGER_N(NAME,V,STR)                             \
    {                                                                      \
      int32 retv;                                                               \
      int32 val = 0;                                                            \
      CTC_CLI_CHK_ARGV(STR);                          \
      val = ctc_cmd_str2nint ((STR), &retv, V);                              \
      if (g_cli_parser_param_en)                      \
      {                                               \
        ctc_cli_out("Param %s:%d\n",NAME,V);            \
      }                                                  \
      if ((retv < 0) || (val < 0))                             \
        {                                                                     \
          ctc_cli_out ("%% Invalid %s value\n", NAME);                       \
          return CLI_ERROR;                                                   \
        }                                                                     \
    }

#define CTC_CLI_GET_INTEGER_RANGE(NAME, V, STR, MIN, MAX)  \
    {                                                  \
        int32 retv;                                      \
        CTC_CLI_CHK_ARGV(STR);                          \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:%d\n",NAME,V);            \
        }                                                  \
        if (retv < 0 || (V) < (MIN) || (V) > (MAX))      \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_INTEGER64(NAME, V, INT, STR, MIN, MAX)  \
    {                                                  \
        int32 retv;                                      \
        int32 val = 0;                                   \
        CTC_CLI_CHK_ARGV(STR);                          \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        sal_memcpy(&INT.l[0], &val, 4);                 \
        sal_memcpy(&INT.l[1], &(V), 4);                 \
        if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:%PRId64\n",NAME,V);            \
        }                                                  \
        if (retv < 0 || (V) < (MIN) || (V) > (MAX))      \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_UINT32(NAME, V, STR)                                      \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) =  ctc_cmd_get_value(&ret, (NAME), (STR), 0, CTC_MAX_UINT32_VALUE, 2);  \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)   \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT64(NAME, V, STR)                    \
    {                                                       \
        int32 retv;                                         \
        CTC_CLI_CHK_ARGV(STR);                          \
        (V) = ctc_cmd_str2uint64((STR), &retv);             \
         if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:%PRIu64\n",NAME,V);            \
        }                                                  \
        if (retv < 0)                                       \
        {                                                   \
            ctc_cli_out("%% Invalid %s value\n", NAME);     \
            return CLI_ERROR;                               \
        }                                                   \
    }
#define CTC_CLI_GET_UINT32_ARRAY(NAME, V, STR)                    \
    {                                                       \
        int32 retv;                                         \
        CTC_CLI_CHK_ARGV(STR);                          \
        ctc_cmd_str2nint((STR), &retv, (V));             \
        if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:0x%x\n",NAME,V[0]);            \
            ctc_cli_out("Param %s:0x%x\n",NAME,V[1]);            \
        }                                                  \
        if (retv < 0)                                       \
        {                                                   \
            ctc_cli_out("%% Invalid %s value\n", NAME);     \
            return CLI_ERROR;                               \
        }                                                   \
    }

#define CTC_CLI_GET_UINT32_RANGE(NAME, V, STR, MIN, MAX)                        \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) = ctc_cmd_get_value(&ret, (NAME), (STR), (MIN), (MAX), 2);               \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)                                                         \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT16(NAME, V, STR)                                            \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) = ctc_cmd_get_value(&ret, (NAME), (STR), 0, CTC_MAX_UINT16_VALUE, 1);   \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)   \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT16_RANGE(NAME, V, STR, MIN, MAX)                            \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) = ctc_cmd_get_value(&ret, (NAME), (STR), (MIN), (MAX), 1);              \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)   \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT8(NAME, V, STR)                                              \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) = ctc_cmd_get_value(&ret, (NAME), (STR), 0, CTC_MAX_UINT8_VALUE, 0);   \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)   \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT8_RANGE(NAME, V, STR, MIN, MAX)                       \
{                                                                               \
    int32 ret;                                                                  \
    CTC_CLI_CHK_ARGV(STR);                          \
    (V) = ctc_cmd_get_value(&ret, (NAME), (STR), (MIN), (MAX), 0);             \
    if (g_cli_parser_param_en)                      \
    {                                               \
        ctc_cli_out("Param %s:%u\n",NAME,V);            \
    }                                                  \
    if(0 != ret)   \
    {                                                                               \
        return CLI_ERROR;                                                           \
    }                                                                               \
}

#define CTC_CLI_GET_UINT8_NO_MAPPING(NAME, V, STR)           \
    {                                                  \
        int32 retv;                                      \
        uint32 tmp = 0;                                   \
        CTC_CLI_CHK_ARGV(STR);                          \
        tmp = ctc_cmd_str2uint((STR), &retv);           \
        if (g_cli_parser_param_en)                      \
        {                                               \
            ctc_cli_out("Param %s:%s\n",NAME,STR);            \
        }                                                  \
        if (retv < 0 || tmp > CTC_MAX_UINT8_VALUE)       \
        {                                               \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                               \
        (V) = (uint8)tmp;                                \
    }

#define CTC_CLI_GET_MAC_ADDRESS(NAME, V, STR)                                                                \
    {                                                                                                        \
        int32 retv = 0;                                                                                      \
        uint8 __index = 0;                                                                                   \
        CTC_CLI_CHK_ARGV(STR);                                                                              \
        while (STR[__index] != '\0')                                                                         \
        {                                                                                                    \
            if (STR[__index] == '.'){ retv++; }                                                              \
            __index++;                                                                                       \
        }                                                                                                    \
        if (retv != 2)                                                                                       \
        {                                                                                                    \
            ctc_cli_out("%% invalid %s value\n", NAME);                                                      \
            return CLI_ERROR;                                                                                \
        }                                                                                                    \
        retv = sal_sscanf((STR), "%4hx.%4hx.%4hx", (uint16*)&(V[0]), (uint16*)&(V[2]), (uint16*)&(V[4]));   \
        if (retv != 3)                                                                                       \
        {                                                                                                    \
            ctc_cli_out("%% invalid %s value\n", NAME);                                                      \
            return CLI_ERROR;                                                                                \
        }                                                                                                    \
        *(uint16*)&(V[0]) = sal_htons(*(uint16*)&(V[0]));                                                   \
        *(uint16*)&(V[2]) = sal_htons(*(uint16*)&(V[2]));                                                   \
        *(uint16*)&(V[4]) = sal_htons(*(uint16*)&(V[4]));                                                   \
        if (g_cli_parser_param_en)                                                                            \
        {                                                                                                       \
            ctc_cli_out("Param %s:%.2x%.2x.%.2x%.2x.%.2x%.2x\n",NAME,V[0],V[1],V[2],V[3],V[4],V[5]);                                      \
        }                                                                                                        \
    }

#define CTC_CLI_GET_IPV4_ADDRESS(NAME, V, STR)               \
    {                                                      \
        int32 retv;                                          \
        CTC_CLI_CHK_ARGV(STR);                          \
        retv = sal_inet_pton(AF_INET, (STR), &(V));         \
        if (!retv)                                           \
        {                                                  \
            ctc_cli_out("%% Invalid %s value\n", NAME);     \
            return CLI_ERROR;                                \
        }                                                  \
        (V) = sal_htonl(V);                                  \
        if (g_cli_parser_param_en)                                                                            \
        {                                                                                                       \
            ctc_cli_out("Param %s:%d.%d.%d.%d\n",NAME,(V >> 24 & 0xFF),(V >> 16 & 0xFF),(V >> 8 & 0xFF),(V & 0xFF));  \
        }                                                                                                        \
    }

#define CTC_CLI_GET_IPV6_ADDRESS(NAME, V, STR)           \
    {                                                  \
        int32 retv;                                      \
        CTC_CLI_CHK_ARGV(STR);                          \
        retv = sal_inet_pton(AF_INET6, (STR), &(V));    \
        if (g_cli_parser_param_en)                                                                            \
        {                                                       \
            ctc_cli_out("Param %s:%s\n",NAME,STR);  \
        }                                                   \
        if (!retv)                                       \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_IS_NUM(STR)                           \
    {                                                 \
        int32 retv;                                   \
        retv = ctc_cmd_judge_is_num((STR));          \
        if (0 != retv)                                  \
        {                                             \
            ctc_cli_out("%% Incomplete command\n");  \
            return CLI_ERROR;                         \
        }                                             \
    }

enum cmd_token_type_s
{
    cmd_token_paren_open,
    cmd_token_paren_close,
    cmd_token_cbrace_open,
    cmd_token_cbrace_close,
    cmd_token_brace_open,
    cmd_token_brace_close,
    cmd_token_separator,
    cmd_token_keyword,
    cmd_token_var,
    cmd_token_unknown
};
typedef enum cmd_token_type_s cmd_token_type;

/* APIs, user shall also call  CTC_CLI(), install_element() */
extern ctc_vti_t* g_ctc_vti;

extern int
ctc_vti_read(int8* buf, uint32 buf_size,uint32 mode);
extern int
ctc_vti_read_cmd(ctc_vti_t* vty, const int8* szbuf, const uint32 buf_len);
extern void
ctc_cmd_init(int terminal);
extern void ctc_cli_register_print_fun(CTC_CLI_OUT_FUNC func);
extern unsigned char
ctc_cli_get_prefix_item(char** argv, unsigned char argc, char* prefix, unsigned char prefix_len);
extern int
ctc_vti_command(ctc_vti_t* vti, char* buf);

extern void
set_terminal_raw_mode(uint32 mode);

extern void
restore_terminal_mode(uint32 mode);

extern void
ctc_cli_enable_cmd_debug(int enable);

extern void
ctc_cli_enable_arg_debug(int enable);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_CLI_H */


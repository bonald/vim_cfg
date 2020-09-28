
#ifndef __CFG_CMD_H__
#define __CFG_CMD_H__

#define CFG_PRINTF  printf

#define CFG_DBG_PRINTF(...)

#define DECIMAL_STRLEN_MIN 1
#define DECIMAL_STRLEN_MAX 10
#define UINT64_DECIMAL_STRLEN_MAX 20

#define MAX_ON_DEMAND_CMD_RET_LEN         512

#define MAX_CMD_LINE_LEN        80
#define MAX_CMD_STR_LEN         1024 * 32
#define MAX_CMD_RET_LEN         128
#define MAX_CMD_ARGS            64
#define MAX_CMD_SHOW_FNAME_LEN  32
#define MAX_CMD_SHOW_KEY_LEN    64
#define MAX_RANGE_SEGMENTS      (MAX_CMD_STR_LEN/2)

#define FILE_READ_LINE_LEN      MAX_CMD_STR_LEN + 2048
#define FILE_READ_LINE_32K_LEN  1024*32
#define FILE_READ_LINE_64K_LEN  1024*64

#define TABLE_STR "TABLE_"
#define CHANNEL_STR "CHANNEL_"
#define KEY_CONNECT_STR ":"
#define RDB_PARTITION_CHAR "//"

#define INTEND_0_STR            ""
#define INTEND_1_STR            " "
#define INTEND_2_STR            "  "
#define INTEND_4_STR            "    "

#define CMD_FLAG_2_CHAR(_buf, _len, _val, _flag, _char) \
    if (GLB_FLAG_ISSET((_val), (_flag))) \
    { \
        sal_strncat((_buf), (_char), (_len)); \
    } \
    else \
    { \
        sal_strncat((_buf), " ", (_len)); \
    }

#define CMD_REMOVE_LAST_CHAR(_str) \
do { \
    (_str)[sal_strlen(_str) - 1] = CMD_ZERO_CHAR; \
} while (0)

#define CFG_DBG_FUNC() \
    CFG_DBG_PRINTF("Enter into (%s, %d)\n", __FUNCTION__, __LINE__);

#define CFG_FUNC_ERR_RET(FMT, ARGS...) \
    PM_CFG_E_RET(para->p_rs, PM_E_CFG_PM_ERROR, FMT, ##ARGS)

#define CFG_CONFLICT_RET(FMT, ARGS...) \
    PM_CFG_E_RET(para->p_rs, PM_E_CFG_CONFLICT, FMT, ##ARGS)

#define CFG_INVALID_PARAM_RET(FMT, ARGS...) \
    PM_CFG_E_RET(para->p_rs, PM_E_CFG_INVALID_PARAM, FMT, ##ARGS)

#define CFG_UNSUPPORT_PRODUCT_RET() \
    CFG_INVALID_PARAM_RET("Not supported on this product");

#define PM_CFG_E_RET(PRS, ERR, FMT, ARGS...) \
    (PRS)->rc = (ERR); \
    sal_snprintf((PRS)->desc, MAX_CMD_RET_LEN, FMT, ##ARGS); \
    return (ERR);

#define CFG_PROMPT_RET(FMT, ARGS...) \
    para->p_rs->rc = PM_E_CFG_PROMPT; \
    sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, FMT, ##ARGS); \
    return PM_E_CFG_PROMPT;

/* for conitnue send to CCS for loopback/linktrace */
#define PM_CFG_E_RET_256(PRS, ERR, FMT, ARGS...) \
    (PRS)->rc = (ERR); \
    sal_snprintf((PRS)->desc, MAX_ON_DEMAND_CMD_RET_LEN, FMT, ##ARGS); \
    return (ERR);
    
#define CFG_CONTINUE_RET(FMT, ARGS...) \
    PM_CFG_E_RET_256(para->p_rs, PM_E_CFG_CONTINUE, FMT, ##ARGS);

#define CFG_CONTINUE_RESULT(PRS, ERR, FMT, ARGS...) \
    (PRS)->rc = (ERR); \
    sal_snprintf((PRS)->desc, MAX_ON_DEMAND_CMD_RET_LEN, FMT, ##ARGS);

#define PTP_CLI_ERROR_RETURN(_op) \
do \
{ \
    int32 rv; \
    if ((rv = (_op)) < 0) \
    { \
        char *p_errstr = NULL; \
        if (NULL != (p_errstr = ptp_error_api_desc(_op))) \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "%s", p_errstr);  \
        } \
        else \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown Error %d", _op); \
        } \
        para->p_rs->rc = rv; \
        return rv;  \
    } \
} \
while(0)

#define TPOAM_CLI_ERROR_RETURN(_op) \
do \
{ \
    int32 rv; \
    if ((rv = (_op)) < 0) \
    { \
        char *p_errstr = NULL; \
        if (NULL != (p_errstr = tpoam_err_desc(_op))) \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "%s", p_errstr);  \
        } \
        else \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown Error %d", _op); \
        } \
        para->p_rs->rc = rv; \
        return rv;  \
    } \
} \
while(0)

#define TPOAM_CLI_ERROR_RETURN_WITH_N(_op) \
do \
{ \
    int32 rv; \
    if ((rv = (_op)) < 0) \
    { \
        char *p_errstr = NULL; \
        if (NULL != (p_errstr = tpoam_err_desc(_op))) \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "%s\n", p_errstr);  \
        } \
        else \
        { \
            sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown Error %d\n", _op); \
        } \
        para->p_rs->rc = rv; \
        return rv;  \
    } \
} \
while(0)


#define CFG_FUNC_UNSUPPORT_OPENFLOW()     CFG_FUNC_ERR_RET("Unsupport openflow feature")

#define PM_CFG_GET_ENUM(V, ARRAY, SIZE, STR, PRS) \
do { \
    (V) = cdb_enum_str2val((ARRAY), (SIZE), (STR)); \
    if ((V) < 0) \
    { \
        PM_CFG_E_RET(PRS, PM_E_CFG_INVALID_PARAM, "Invalid enum %s", (STR)); \
    } \
} while (0)

#define PM_CFG_GET_INTEGER(V, STR, PRS) \
do { \
    (V) = cdb_int_str2val((STR), &rc); \
    if (rc < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to integer", (STR)); \
    } \
} while (0)


#define PM_CFG_GET_INT64(V, STR, PRS) \
do { \
    (V) = cdb_int64_str2val((STR), &rc); \
    if (rc < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to integer64", (STR)); \
    } \
} while (0)

#define PM_CFG_GET_UINT(V, STR, PRS) \
do { \
    if (!sal_strncmp((STR), "-", 1)) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to unsigned integer", (STR)); \
    } \
    PM_CFG_GET_INTEGER(V, STR, (PRS)); \
} while (0)

#define PM_CFG_GET_UINTEGER64(V, STR, PRS) \
do { \
    (V) = cdb_uint64_str2val((STR), &rc); \
    if (rc < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to uint64", (STR)); \
    } \
} while (0)

#define PM_CFG_GET_UINT_RANGE(V, MIN, MAX, STR, PRS) \
do { \
    PM_CFG_GET_UINT(V, STR, (PRS)); \
    if (V < MIN || V > MAX) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Value %u is out of range", (V)); \
    } \
} while (0)

#define PM_CFG_GET_UINT64(V, STR, PRS) \
do { \
    if (!sal_strncmp((STR), "-", 1)) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to uint64", (STR)); \
    } \
    PM_CFG_GET_UINTEGER64(V, STR, (PRS)); \
} while (0)

#define PM_CFG_GET_BOOL(V, STR, PRS) \
do { \
    PM_CFG_GET_INTEGER(V, STR, PRS); \
    if ((FALSE != V) && (TRUE != V)) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Can't convert %s to bool", (STR)); \
    } \
} while (0)

#define PM_CFG_CHECK_EXPECT_PARAMS(ARGC, EXPARGC, PRS) \
do { \
    if (((ARGC) != (EXPARGC))) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "The parameter number %u is not expected %u", (ARGC), (EXPARGC)); \
    } \
} while (0)

#define PM_CFG_CHECK_EXPECT_2PARAMS(ARGC, EXPARGC1, EXPARGC2, PRS) \
do { \
    if (((ARGC) != (EXPARGC1)) && ((ARGC) != (EXPARGC2))) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "The parameter number %u is not expected %u or %u", (ARGC), (EXPARGC1), (EXPARGC2)); \
    } \
} while (0)

#define PM_CFG_CHECK_MIN_PARAMS(ARGC, EXPARGC, PRS) \
do { \
    if (((ARGC) < (EXPARGC))) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "The parameter number %u is < expected %u", (ARGC), (EXPARGC)); \
    } \
} while (0)

#define CLI_GET_INTEGER(NAME,V,STR)                     \
do {                                                    \
    (V) = cdb_int_str2val((STR), &rc);                  \
    if (rc < 0)                                         \
    {                                                   \
        CFG_PRINTF("%% Invalid %s value\n", NAME);      \
        return -1;                                      \
    }                                                   \
} while (0)

/*ipv4 addr str to addr_ipv4_t*/
#define CFG_PARSER_IP_ADDR(ADDR_STR, IP, PRS) \
do { \
    rc = sal_inet_pton(AF_INET, ADDR_STR, IP); \
    if (rc != 1) \
    { \
        CFG_INVALID_PARAM_RET("Invalid IP address %s", ADDR_STR); \
    } \
} while (0)

/*ipv6 addr str to addr_ipv6_t*/
#define CFG_PARSER_IPV6_ADDR(ADDR_STR, IP, PRS) \
do { \
    rc = sal_inet_pton(AF_INET6, ADDR_STR, IP); \
    if (rc != 1) \
    { \
        CFG_INVALID_PARAM_RET("Invalid IPv6 address %s", ADDR_STR); \
    } \
} while (0)

/*ipv4 addr str and masklen str to prefix_ipv4_t*/
#define CFG_PARSER_PREFIXV4(ADDR_STR, MASK_STR, PREFIX, PRS) \
do { \
    rc = cdb_prefix_ipv4_str2val(ADDR_STR, MASK_STR, PREFIX, PRS->desc); \
    if (rc < 0) \
    { \
        PRS->rc = rc; \
        return rc; \
    } \
} while (0)

/*ipv4 v6 addr str and masklen str to prefix_t*/
#define CFG_PARSER_PREFIX(ADDR_STR, MASK_STR, PREFIX, PRS) \
do { \
    rc = cdb_prefix_str2val(ADDR_STR, MASK_STR, PREFIX, PRS->desc); \
    if (rc < 0) \
    { \
        PRS->rc = rc; \
        return rc; \
    } \
} while (0)

#define CFG_PARSER_MAC(MAC, MAC_STR, PRS) \
do { \
    if (cdb_mac_addr_str2val((MAC), (MAC_STR)) < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Invalid MAC address %s", (MAC_STR)); \
    } \
} while (0)

#define CFG_GET_MAC_MASKED(MAC, MASK, PRS) \
do { \
    if (cdb_mac_addr_masked((MAC), (MASK)) < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Invalid MAC address mask"); \
    } \
} while (0)

#define CFG_GET_IP_MASKED(IP, MASK, PRS) \
do { \
    if (cdb_ip_addr_masked((IP), (MASK)) < 0) \
    { \
        PM_CFG_E_RET((PRS), PM_E_CFG_INVALID_PARAM, "Invalid IP address mask"); \
    } \
} while (0)

typedef struct
{
    int32           rc;
    char            desc[MAX_ON_DEMAND_CMD_RET_LEN];
} cfg_result_t;

typedef struct
{
    int32               rc;
    uint32              req_count;
    uint32              resp_count;
    uint32              banner;
    uint32              detail;
    char                path[MAX_CMD_SHOW_FNAME_LEN];
    char                key[MAX_CMD_SHOW_KEY_LEN];
} cfg_show_t;


#define DB_MAX_DEPTH            256
#define DB_MAX_PATH_LEN         1024

typedef struct
{
    char       *argv[DB_MAX_DEPTH];
    char       *value[DB_MAX_DEPTH];
    uint32      argc;
} cfg_parse_get_info_t;

typedef struct
{
    char       *argv[DB_MAX_DEPTH];
    char       *value[DB_MAX_DEPTH];
    char       *left;
    uint32      argc;
} cfg_parse_get_info_cycle_t;

typedef struct
{
    cdb_oper_t  oper;
    char       *path;
    char       *argv[DB_MAX_DEPTH];
    uint32      argc;
    char        buf[DB_MAX_PATH_LEN];   /* buffer for command need append or changed */
} cfg_parse_info_t;

typedef struct
{
    cdb_oper_t      oper;
    int32           sock;
    int32           pid;
    cdb_cfg_t       cfg;
    ctc_msg_t      *p_msg;
    uint8           ret_buf[MAX_CMD_RET_LEN+4];
    cfg_result_t   *p_rs;
    cfg_show_t     *p_show;
} cfg_cmd_para_t;

typedef struct
{
    int32 lic_check_en; /* 1: check enable, 0: check disable */
    int32 erps_en;      /* 0: on, -1: off */
    int32 mlag_en;
    int32 ospf_en;
    int32 openflow_en;
    int32 tap_en;
    int32 bgp_en;
    int32 vrrp_en;
} cfg_lic_info_t;

typedef struct
{
#if 0
    int32       argc;
    const char *argv[MAX_CMD_ARGS];
    char        buf[MAX_CMD_STR_LEN];
#else
    int32       len;
    char        buf[MAX_CMD_STR_LEN];
#endif
} cfg_db_cmd_item_t;

#define DB_CMD_CAT(_p_cmd, _str, _len) \
do { \
    _len = sal_strlen(_str); \
    sal_strcpy(&(_p_cmd->buf[_p_cmd->len]), _str); \
    _p_cmd->len += _len; \
    _p_cmd->buf[_p_cmd->len++] += CMD_DIVISION_CHAR; \
    _p_cmd->buf[_p_cmd->len] = CMD_ZERO_CHAR; \
} while (0)

#define CFG_DB_CMDS_COUNT   2

typedef struct
{
    cfg_db_cmd_item_t   cmd;
    int32               pre_cmd_count;
    int32               post_cmd_count;
    cfg_db_cmd_item_t  *pre_cmds[CFG_DB_CMDS_COUNT];
    cfg_db_cmd_item_t  *post_cmds[CFG_DB_CMDS_COUNT];
} cfg_db_cmds_t;

typedef int32 (*CMD_FUNC)(char **argv, int32 argc, cfg_cmd_para_t *para);

typedef struct
{
    char *name; /* cmd key string */
    char *value;    /* cmd value string, pointer to argv[i] */ 
}cmd_args_key_value_t;

int32
cfg_cmd_parse_para(char *para_str, cfg_parse_info_t *p_info);

int32
cfg_cmd_strip_quotation(cfg_parse_info_t *p_info);

int32
cfg_cmd_parse(char *cmd, cfg_parse_info_t *p_info);

char*
cfg_strchr(char *s1, char c);

char*
cfg_get_strip_bra(char *line);

int32
cfg_get_parse(char *line, cfg_parse_get_info_t *p_info, uint32 level);

int32
cfg_get_parse_maxargc(char *line, cfg_parse_get_info_t *p_info, uint32 level, uint32 maxargc);

int32
cfg_get_parse_referlist(char *line, uint32 is_ds, cfg_parse_get_info_t *p_info);

int32
cfg_get_parse_comma_ds(char *line, cfg_parse_get_info_t *p_info);

int32
cfg_get_parse_comma(char *line, cfg_parse_get_info_t *p_info);

int32
cfg_get_parse_comma_cycle(char *line, cfg_parse_get_info_cycle_t *p_info);

int32
cfg_cmd_process(int32 sock, ctc_msg_t *p_msg);

int32
cfg_cmd_show_file(char *file_name);

int32
cfg_parser_cmd_args(char **argv, int32 argc, cfg_cmd_para_t *para, 
                              cmd_args_key_value_t key_value[], int32 key_num, bool skip_key);

char*
cfg_get_parser_value(cmd_args_key_value_t key_value[], int32 key_num, char* key);

#define CFG_GET_ARGC_INDEX_ENHANCE(str) cfg_get_prefix_item(argv, argc, str, 0)

unsigned char
cfg_get_prefix_item(char** argv, unsigned char argc, char* prefix, unsigned char prefix_len);

extern int
checkLicense(char *pszFeature);
int32
cfg_get_lic_check_en();
cfg_lic_info_t*
cfg_get_lic_info();

int cfg_board_match_check(int board_series, int board_type);

#endif /* !__CFG_CMD_H__ */


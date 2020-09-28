
#include "proto.h"
#include "sdb_defines.h"
#include "glb_tempfile_define.h"

/* both ccs and ncs need it */
static cfg_lic_info_t g_lic_info;

/* bypass "" sub string */
char*
cfg_strchr(char *s, char c)
{
    char *p = s;
    uint32 quotation_count = 0;

    while (CMD_ZERO_CHAR != *p) 
    { 
        if (CMD_QUOTATION_CHAR == *p)
        {
            quotation_count++;
        }

        /* bypass '/' in the pair of " " */
        if (0 == (quotation_count % 2))
        {
            if (c == *p)  
            {
                return p;
            }
        }
        p++;
    }

    return NULL;
}

/* bypass {} sub string */
char*
cfg_bra_strchr_ds(char *s, char c)
{
    uint32 bracket_depth = 0;
    char *pp = s;
    char *p = s;
    
    while (CMD_ZERO_CHAR != *p) 
    {
        if (CMD_DS_LEFT_BRA_CHAR == *p)
        {
            bracket_depth++;
        }

        if (CMD_DS_RIGHT_BRA_CHAR == *p)
        {
            bracket_depth--;
        }
    
        /* bypass '/' in the pair of {} */
        if (0 == bracket_depth)
        {
            if (c == *p && '/' == *pp)  
            {
                return p;
            }
        }
        pp = p;
        p++;
    }

    return NULL;
}

/* bypass {} sub string */
char*
cfg_bra_strchr(char *s, char c)
{
    uint32 bracket_depth = 0;
    
    char *p = s;
    
    while (CMD_ZERO_CHAR != *p) 
    {
        if (CMD_DS_LEFT_BRA_CHAR == *p)
        {
            bracket_depth++;
        }

        if (CMD_DS_RIGHT_BRA_CHAR == *p)
        {
            bracket_depth--;
        }
    
        /* bypass '/' in the pair of {} */
        if (0 == bracket_depth)
        {
            if (c == *p)  
            {
                return p;
            }
        }
        p++;
    }

    return NULL;
}

char*
cfg_get_strip_bra(char *line)
{
    char *p = line;
    char *new_line = line;
    
    if (NULL == p)
    {
        return p;
    }

    while (CMD_ZERO_CHAR != p[0])
    {
        if (CMD_DS_LEFT_BRA_CHAR == p[0])
        {
            p++;
            new_line = p;
        }
        if (CMD_DS_RIGHT_BRA_CHAR == p[0])
        {
            p[0] = CMD_ZERO_CHAR;
        }
        p++;
    }

    return new_line;
}

/* e.g. /key=eth-0-1/ifindex=1/desc=/speed=auto */
int32
cfg_get_parse(char *line, cfg_parse_get_info_t *p_info, uint32 level)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;
    char           *value = NULL;
    char            div_char = 0;

    if (level)
    {
        line = cfg_get_strip_bra(line);
        div_char = CMD_DIVISION_CHAR;
    }
    else
    {
        div_char = CMD_DIVISION_CHAR;
    }

    sal_memset(p_info, 0, sizeof(cfg_parse_get_info_t));
    if (!line)
    {
        return PM_E_NONE;
    }
    
    line_strlen = sal_strlen(line);
    if (0 == line_strlen)
    {
        return PM_E_NONE;
    }
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == div_char)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }

    pos = cfg_bra_strchr(line, div_char);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc] = pos + 1;
        value = cfg_bra_strchr(p_info->argv[p_info->argc], CMD_EQUAL_CHAR);
        if (NULL == value)
        {
            p_info->value[p_info->argc] = NULL;
        }
        p_info->value[p_info->argc] = value+1;
        p_info->argc++;
        pos = cfg_bra_strchr(pos + 1, div_char);
    }
    
    return PM_E_NONE;
}

int32
cfg_get_parse_maxargc(char *line, cfg_parse_get_info_t *p_info, uint32 level, uint32 maxargc)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;
    char           *value = NULL;
    char            div_char = 0;

    if (level)
    {
        line = cfg_get_strip_bra(line);
        div_char = CMD_DIVISION_CHAR;
    }
    else
    {
        div_char = CMD_DIVISION_CHAR;
    }

    sal_memset(p_info, 0, sizeof(cfg_parse_get_info_t));
    if (!line)
    {
        return PM_E_NONE;
    }
    
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == div_char)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }

    pos = cfg_bra_strchr(line, div_char);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc] = pos + 1;
        value = cfg_bra_strchr(p_info->argv[p_info->argc], CMD_EQUAL_CHAR);
        if (NULL == value)
        {
            p_info->value[p_info->argc] = NULL;
        }
        p_info->value[p_info->argc] = value+1;
        p_info->argc++;
        if (p_info->argc >= maxargc)
        {
            break;
        }
        pos = cfg_bra_strchr(pos + 1, div_char);
    }
    
    return PM_E_NONE;
}

int32
cfg_get_parse_referlist(char *line, uint32 is_ds, cfg_parse_get_info_t *p_info)
{
    char *p = NULL;
    uint32 str_len = sal_strlen(line);
    int32 rc = PM_E_NONE;
    
    if (str_len > 0 && line[str_len-1] == CMD_DS_RIGHT_BRA_CHAR)
    {
        line[str_len-1] = CMD_ZERO_CHAR;
    }

    p = line;
    if (p[0] == CMD_DS_LEFT_BRA_CHAR)
    {
        p++;
    }
    if (0 == sal_strlen(p))
    {
        sal_memset(p_info, 0, sizeof(cfg_parse_get_info_t));
        return rc;
    }

    if (is_ds)
    {
        rc = cfg_get_parse_comma_ds(p, p_info);
    }
    else
    {
        rc = cfg_get_parse_comma(p, p_info);
    }
    return rc;
}

// TODO: note: line=",,,,,,,", returc argc==7
int32
cfg_get_parse_comma_ds(char *line, cfg_parse_get_info_t *p_info)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(cfg_parse_get_info_t));  
    if (!line)
    {
        return PM_E_NONE;
    }
        
    line_strlen = sal_strlen(line);

    /* check line == "" */
    if (0 == line_strlen)
    {
        return PM_E_NONE;
    }
    
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_COMMA_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
  
    p_info->argv[p_info->argc++] = line;

    pos = cfg_bra_strchr_ds(line, CMD_COMMA_CHAR);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_bra_strchr_ds(pos + 1, CMD_COMMA_CHAR);
    }
    
    return PM_E_NONE;
}

// TODO: note: line=",,,,,,,", returc argc==7
int32
cfg_get_parse_comma(char *line, cfg_parse_get_info_t *p_info)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(cfg_parse_get_info_t));  
    if (!line)
    {
        return PM_E_NONE;
    }
        
    line_strlen = sal_strlen(line);

    /* check line == "" */
    if (0 == line_strlen)
    {
        return PM_E_NONE;
    }
    
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_COMMA_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
  
    p_info->argv[p_info->argc++] = line;

    pos = cfg_bra_strchr(line, CMD_COMMA_CHAR);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_bra_strchr(pos + 1, CMD_COMMA_CHAR);
    }
    
    return PM_E_NONE;
}

int32
cfg_get_parse_comma_cycle(char *line, cfg_parse_get_info_cycle_t *p_info)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(cfg_parse_get_info_cycle_t));  
    if (!line)
    {
        return PM_E_NONE;
    }
        
    line_strlen = sal_strlen(line);

    /* check line == "" */
    if (0 == line_strlen)
    {
        return PM_E_NONE;
    }
    
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_COMMA_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
  
    p_info->argv[p_info->argc++] = line;

    pos = cfg_bra_strchr(line, CMD_COMMA_CHAR);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_bra_strchr(pos + 1, CMD_COMMA_CHAR);
        if (DB_MAX_DEPTH <= p_info->argc)
        {
            if (NULL == pos)
            {
                return PM_E_NONE;
            }
            else
            {
                *pos = 0;
                p_info->left = pos + 1;
                return PM_E_EXCEED_MAX_NUM;
            }
        }
    }
    
    return PM_E_NONE;
}

int32
cfg_cmd_parse(char *cmd, cfg_parse_info_t *p_info)
{
    uint32          cmdstrlen = 0;
    int32           oper = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(cfg_parse_info_t));
    cmdstrlen = sal_strlen(cmd);
    if (cmd[cmdstrlen-1] == CMD_DIVISION_CHAR)
    {
        cmd[cmdstrlen-1] = 0;
    }

    /* divide oper and dir */
    pos = cfg_strchr(cmd, CMD_DIVISION_CHAR);
    if (!pos)
    {
        CFG_PRINTF("%% Invalid command: %s, no CDB path\n", cmd);
        return PM_E_FAIL;
    }

    *pos = CMD_ZERO_CHAR;
    
    /* parse oper */
    oper = cdb_enum_str2val(cdb_oper_str_array, CDB_OPER_MAX, cmd);
    if (oper < 0)
    {
        CFG_PRINTF("%% Invalid command oper: %s\n", cmd);
        return PM_E_FAIL;
    }
    p_info->oper = oper;
    
    p_info->path = pos + 1;

    if (NULL == p_info->path || CMD_ZERO_CHAR == p_info->path[0])
    {
        CFG_PRINTF("%% Invalid command with NULL CDB path\n");
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
cfg_cmd_parse_para(char *para_str, cfg_parse_info_t *p_info)
{
    uint32          cmdstrlen = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(cfg_parse_info_t));
    if (NULL == para_str)
    {
        return PM_E_NONE;
    }

    /* fix cdbctl crash problem */
    cmdstrlen = sal_strlen(para_str);
    if (0 == cmdstrlen)
    {
        return PM_E_NONE;
    }
    if (para_str[cmdstrlen-1] == CMD_DIVISION_CHAR)
    {
        para_str[cmdstrlen-1] = 0;
    }

    pos = para_str;
    p_info->argv[p_info->argc++] = pos;
    pos = cfg_strchr(para_str, CMD_DIVISION_CHAR);
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_strchr(pos + 1, CMD_DIVISION_CHAR);
    }
    
    return PM_E_NONE;
}

int32
cfg_cmd_strip_quotation(cfg_parse_info_t *p_info)
{
    char *p = NULL;
    uint32 i = 0;
    uint32 str_len = 0;

    for (i = 0; i < p_info->argc; i++)
    {
        p = p_info->argv[i];
        if (CMD_QUOTATION_CHAR == p[0])
        {
            p[0] = CMD_ZERO_CHAR;
            p++;
        }
        
        str_len = sal_strlen(p);
        if (CMD_QUOTATION_CHAR == p[str_len-1])
        {
            p[str_len-1] = CMD_ZERO_CHAR;
        }    
        p_info->argv[i] = p;
    }
    
    return PM_E_NONE;
}

int32
cfg_cmd_process(int32 sock, ctc_msg_t *p_msg)
{
    char           *para_str = NULL;
    cfg_parse_info_t parse_info;
    cfg_cmd_para_t  para;
    int32 rs_len = 0;
    cdb_node_t* p_node = NULL;
    sdb_node_t* p_sdb_node = NULL;
    int32 rc = PM_E_NONE;

    para.sock = sock;
    para.oper = p_msg->u_hdr.cfg.oper;
    para.pid = p_msg->u_hdr.cfg.pid;
    para.cfg = p_msg->u_hdr.cfg.cfg;
    para.p_msg = p_msg;
    sal_memset(para.ret_buf, 0, sizeof(para.ret_buf));
    para.p_rs = (cfg_result_t*)para.ret_buf;
    para.p_show = (cfg_show_t*)para.ret_buf;
    
    if (p_msg->data_len > MAX_CMD_STR_LEN)
    {
        SAL_ASSERT(0);
        return PM_E_EXCEED_CMD_LEN;
    }

    if (p_msg->u_hdr.cfg.type >= SDB_NODE_BASE)
    {
        p_sdb_node = sdb_get_node_by_id(p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
        if (NULL == p_sdb_node)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Invalid node type %d, id %d", 
                p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
            goto error_return;
        }
        if (SDB_NODE_TBL != p_msg->u_hdr.cfg.type && SDB_NODE_ACT != p_msg->u_hdr.cfg.type)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Only support to visit table or action");
            goto error_return;
        }
        if (NULL == p_sdb_node->tbl_info->ops.cmd_fn)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Process function for SDB %s %s is unregistered", 
                sdb_node_type_str(p_sdb_node->type), p_sdb_node->name);
            goto error_return;
        }
        
        para_str = p_msg->data;
        rc = cfg_cmd_parse_para(para_str, &parse_info);
        if (rc < 0)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Parse parameter fail, return code %d", rc);
            goto error_return;
        }

        rc = p_sdb_node->tbl_info->ops.cmd_fn(parse_info.argv, parse_info.argc, &para);
        if (rc < 0)
        {
            rs_len = sizeof(cfg_result_t);
        }
        else
        {
            rs_len = ((CDB_OPER_GET == para.oper) || (CDB_OPER_GETJSON == para.oper)) ? sizeof(cfg_show_t) : sizeof(cfg_result_t);
        }
        return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
    }
    else
    {
        p_node = cdb_get_node_by_id(p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
        if (NULL == p_node)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Invalid node type %d, id %d", 
                p_msg->u_hdr.cfg.type, p_msg->u_hdr.cfg.id);
            goto error_return;
        }

        if (CDB_NODE_TBL != p_msg->u_hdr.cfg.type && CDB_NODE_ACT != p_msg->u_hdr.cfg.type)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Only support to visit table or action");
            goto error_return;
        }

        if (NULL == p_node->tbl_info->ops.cmd_fn)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Process function for CDB %s %s is unregistered", 
                cdb_node_type_str(p_node->type), p_node->name);
            goto error_return;
        }

#if 0
        /* not change here for will result in time-range crash */
        /* Added by kcao for bug 41408 2016-10-27, change p_msg->data[data_len] = '\0' for maybe uses it do lookup */
        if (p_msg->data)
        {
            ((uint8*)p_msg->data)[p_msg->data_len-1] = CMD_ZERO_CHAR;
        }
#endif
        para_str = p_msg->data;
        rc = cfg_cmd_parse_para(para_str, &parse_info);
        if (rc < 0)
        {
            sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Parse parameter fail, return code %d", rc);
            goto error_return;
        }

        rc = p_node->tbl_info->ops.cmd_fn(parse_info.argv, parse_info.argc, &para);
        if (rc < 0)
        {
            rs_len = sizeof(cfg_result_t);
        }
        else
        {
            rs_len = ((CDB_OPER_GET == para.oper) || (CDB_OPER_GETJSON == para.oper)) ? sizeof(cfg_show_t) : sizeof(cfg_result_t);
        }
        return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
    }


error_return:
    para.p_rs->rc = PM_E_CFG_PM_ERROR;
    rs_len = sizeof(cfg_result_t);
    return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
    
}

int32
cfg_cmd_show_file(char *file_name)
{
#define SHOW_LINE_LEN   256
    char  line[SHOW_LINE_LEN];

    FILE* ipc_fp = NULL;
    ipc_fp = sal_fopen(file_name, "r");
    if (!ipc_fp)
    {
        return -1;
    }
    
    while (NULL != sal_fgets(line, SHOW_LINE_LEN, ipc_fp))
    {
        CFG_PRINTF("%s", line);
    }
    sal_fclose(ipc_fp);
    sal_unlink(file_name);

    return 0;
}

#define CHECK_RANGE_VALUE(_value, _min, _max) \
if ((_value) < (_min) || (_value) > (_max)) \
{ \
    sal_snprintf(p_rslt->desc, MAX_CMD_RET_LEN, "Value %d is out of [%d, %d]", (_value), (_min), (_max)); \
    return PM_E_CFG_INVALID_PARAM; \
}

static inline int32
get_char_count(const char *str, const char *c)
{
    int32 count = 0;
    int32 len = sal_strlen(str);
    int32 i = 0;

    for (i = 0; i < len; i++)
    {
        if (str[i] == c[0])
        {
            count++;
        }
    }

    return count;
}

int32
cfg_split_token(char *str, const char *delimiter, uint32 max_token, char **token_array, uint32 *token_count)
{
    char *token = NULL;
    char *saveptr = NULL;
    uint32 count = 0;
    
    token = sal_strtok_r(str, delimiter, &saveptr);
    while (NULL != token)
    {
        if (count >= max_token)
        {
            return PM_E_FAIL;
        }
        token_array[count] = token;
        count++;
        token = sal_strtok_r(NULL, delimiter, &saveptr);
    }
    *token_count = count;

    return PM_E_NONE;
}

/*  for tbl, skip_key = TRUE;
    for global, skip_key = FALSE*/
int32
cfg_parser_cmd_args(char **argv, int32 argc, cfg_cmd_para_t *para, 
                              cmd_args_key_value_t key_value[], int32 key_num, bool skip_key)
{
    int32 rc = PM_E_NONE;

    int32 i = 0;
    int32 j = 0;
    
    for (skip_key ? (i = 1): (i = 0); i < argc; i++)
    {
        for (j = 0; j < key_num; j++)
        {
        /* match key name */
            if (0 == sal_strcmp(argv[i], key_value[j].name))
            {
                i++;
                /* store pointer to value */
                key_value[j].value = argv[i];
                break;
            }
        }
        #if 0
        /* key not found, error occured */
        if (j >= key_num)
        {
            CFG_INVALID_PARAM_RET("Parser args failed: \"%s\" is unknown", argv[i]);
        }
        #endif
    }

    /* develop debug */
    #if 0
    for (key_idx = 0; key_idx < key_num; key_idx++)  
    {
        if (key_value[key_idx].value)
        {
            sal_printf("%s: %s\n", key_value[key_idx].name, key_value[key_idx].value);
        }
    }
    
    #endif
    return rc;
}

char*
cfg_get_parser_value(cmd_args_key_value_t key_value[], int32 key_num, char* key)
{
    int32 i = 0;

    for (i = 0; i < key_num; i++)
    {
    /* match key name */
        if (0 == sal_strcmp(key, key_value[i].name))
        {
            return key_value[i].value;
        }
    }
    return NULL;
}

unsigned char
cfg_get_prefix_item(char** argv, unsigned char argc, char* prefix, unsigned char prefix_len)
{
    /*added by yejl for bug49652, 2018-10-24*/
    unsigned char index = 1;

    while (index < argc)
    {
        if ((prefix_len && !sal_strncmp(argv[index], prefix, prefix_len)) ||
            (!sal_strncmp(argv[index], prefix, sal_strlen(argv[index])) &&
             (sal_strlen(argv[index]) == sal_strlen(prefix))) )
        {
            return index;
        }

        index++;
    }

    return 0xFF;
}

int32
cfg_lic_check_cdb_node(cdb_node_t* p_node)
{
    cfg_lic_info_t* p_lic_info = cfg_get_lic_info();

    if (0 == p_lic_info->lic_check_en)
    {
        return PM_E_NONE;
    }
    
    if (CDB_NODE_TBL == p_node->type)
    {
        switch (p_node->id)
        {
        case TBL_OPM_GLOBAL:
        case TBL_ERPS_DOMAIN:
        case TBL_ERPS_RING:
            return p_lic_info->erps_en;

        case TBL_MLAG:
        case TBL_MLAG_PEER:
        case TBL_MLAG_PORT:
            return p_lic_info->mlag_en;

        case TBL_OSPF:
        case TBL_OSPF_NETWORK:
        case TBL_OSPF_AREA_AUTH:
        case TBL_OSPF_AREA_RANGE:
            return p_lic_info->ospf_en;

        case TBL_OPENFLOW:
        case TBL_OPENFLOW_MANAGER:
        case TBL_OPENFLOW_INTERFACE:
        case TBL_OPENFLOW_TUNNEL_INTERFACE:
        case TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT:
            return p_lic_info->openflow_en;

        case TBL_TAP_GROUP:
        case TBL_TAP_GROUP_INGRESS:
        case TBL_TAP_GROUP_EGRESS:
        case TBL_TAP_GROUP_INGRESS_FLOW:
            return p_lic_info->tap_en;

        default:
            return PM_E_NONE;
        }
    }
    else if (CDB_NODE_ACT == p_node->type)
    {
        switch (p_node->id)
        {
        case ACT_SHOW_ERPS_LIST:
        case ACT_SHOW_ERPS:
            return p_lic_info->erps_en;

        case ACT_SHOW_MLAG:
        case ACT_CLEAR_MLAG_COUNTERS:
            return p_lic_info->mlag_en;

#if 0
            return p_lic_info->ospf_en;
#endif
        case ACT_CHECK_OPENFLOW:
            return p_lic_info->openflow_en;

        case ACT_SHOW_TAP:
            return p_lic_info->tap_en;

        default:
            return PM_E_NONE;
        }
    }
    
    return PM_E_NONE;
}

int cfg_board_match_check(int board_series, int board_type)
{
    char buf[CMD_BUF_256];
    char tmp[CMD_BUF_256];
    FILE *fp = NULL;
    uint32 boardtype = 0;
    uint32 product_series = 0;
    
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    while(sal_fgets(buf, CMD_BUF_256, fp) != NULL)
    {
        if (!sal_strncmp(buf, GLB_BOARD_TYPE_STRING, sal_strlen(GLB_BOARD_TYPE_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &boardtype);
        }
        if (!sal_strncmp(buf, GLB_PRODUCT_SERIES_STRING, sal_strlen(GLB_PRODUCT_SERIES_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &product_series);
        }
    }

    if((boardtype == board_type) && (product_series == board_series))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int32
cfg_get_lic_check_en()
{
    char buf[CMD_BUF_256];
    char tmp[CMD_BUF_256];
    FILE *fp = NULL;
    uint32 board_type = 0;
    uint32 product_series = 0;
    int32 lic_check_en = 0;
    
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    while(sal_fgets(buf, CMD_BUF_256, fp) != NULL)
    {
        if (!sal_strncmp(buf, GLB_BOARD_TYPE_STRING, sal_strlen(GLB_BOARD_TYPE_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &board_type);
        }
        if (!sal_strncmp(buf, GLB_PRODUCT_SERIES_STRING, sal_strlen(GLB_PRODUCT_SERIES_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &product_series);
        }
    }

    if (GLB_SERIES_E550 == product_series)
    {
        switch (board_type)
        {
        /* ONLY CENTEC BOARD NEED NOT CHECK LICENSE */
        case GLB_BOARD_E550_24X8Y2C:
        case GLB_BOARD_E550_48T4Y2Q:
        case GLB_BOARD_E550_28M4Y2C:
        case GLB_BOARD_E550_48M4Y2C:
        case GLB_BOARD_E550_24T16Y:
        case GLB_BOARD_E550_24T16X:
            break;

        default:
            lic_check_en = 1;
            break;
        }
    }
    else if (GLB_SERIES_E530 == product_series)
    {
        switch (board_type)
        {
/* modified by liwh for bug 54105, 2019-10-24 */
#ifndef OFPRODUCT
/* liwh end */
        /* ONLY CENTEC BOARD NEED NOT CHECK LICENSE */
        case GLB_BOARD_E530_24X2C_D:
        case GLB_BOARD_E530_48T4X2Q_D:
        case GLB_BOARD_E530_48M4Y2D:
        case GLB_BOARD_E530_48X:
        case GLB_BOARD_E530_24X2C:
        case GLB_BOARD_E530_48T4X:
        case GLB_BOARD_E530_48S4X:
        case GLB_BOARD_E530_24X2Q:            
            break;
/* modified by liwh for bug 54105, 2019-10-24 */
#endif
/* liwh end */
        default:
            lic_check_en = 1;
            break;
        }
    }
    else if (GLB_SERIES_E580 == product_series)
    {
        switch (board_type)
        {
        /* ONLY CENTEC BOARD NEED NOT CHECK LICENSE */
        case GLB_BOARD_E580_24Q:
        case GLB_BOARD_E580_48X2Q4Z:
        case GLB_BOARD_E580_48X6Q:
        case GLB_BOARD_E580_32X2Q:
        case GLB_BOARD_E580_32X:
        case GLB_BOARD_E580_48XT6Q:
            break;

        default:
            lic_check_en = 1;
            break;
        }
    }
    else if (GLB_SERIES_E350 == product_series)
    {
        switch (board_type)
        {
        /* ONLY CENTEC BOARD NEED NOT CHECK LICENSE */
        case GLB_BOARD_E350_48T4XG:
        case GLB_BOARD_E350_8TS12X:
            break;

        default:
            lic_check_en = 1;
            break;
        }
    
    }
    else if (GLB_SERIES_P580 == product_series)
    {
        lic_check_en = 1;
    }
    else if(GLB_SERIES_EMU == product_series)
    {
        lic_check_en = 0;
    }
    else 
    {
        lic_check_en = 1;
    }
    

#ifdef TAPPRODUCT
    lic_check_en = 1;
#endif

#ifdef LICENSE
    lic_check_en = 1;
#endif

    return lic_check_en;
}

cfg_lic_info_t*
cfg_get_lic_info()
{
    return &g_lic_info;
}


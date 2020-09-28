
#include "proto.h"
#include "cdbclient.h"
#include "glb_tempfile_define.h"
#include "pb_cdb_inc.h"
#include "tbl.pb-c.h"
#include "ds.pb-c.h"
#include "pb_common.h"
#include "pb_tbl.h"
#include "pb_ds.h"

struct timeval g_last_tv;

cdb_subscribe_format_t g_out_format = CDB_SUB_FORMAT_RAW;
static uint32 g_subscribe_count = 0;
static CDB_SUBSCRIBE_ACTION_FUNC g_subscribe_action_fn = NULL;
static CDB_SUBSCRIBE_USER_FUNC   g_subscribe_user_fn = NULL;
char g_cdbctl_subscribe_script_path[CMD_BUF_256];

int32
cdbctl_process_split_and_dump_msg(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg, 
                        cdbctl_line_t *p_line)
{
    uint32 rand_value = 0;
    char  tmp_fname[MAX_CMD_LINE_LEN];
    FILE *fp = NULL;
    tbl_iter_args_t args;
    field_parse_t field;
    void *p_tbl = NULL;
    void *p_ds = NULL;
    void *p_ds2 = NULL;
    cdb_tbl_info_t *p_tbl_info = p_tbl_node->tbl_info;
    cdb_tbl_info_t *p_ds_info = NULL;
    cdb_tbl_info_t *p_ds2_info = NULL;
    
    rand_value = rand();
    sal_memset(&field, 0, sizeof(field));

    if (p_ds_node)
    {
        /* only key */
        field.field_id[0] = 0;
    }
    else
    {
        if (CDB_OPER_DEL == p_msg->u_hdr.data.oper)
        {
            /* only key */
            field.field_id[0] = 0;
        }
        else
        {
            field.field_id[0] = FLD_ID_ALL;
        }
    }
    
    sal_snprintf(tmp_fname, MAX_CMD_LINE_LEN, "/tmp/react%u", rand_value);
    fp = sal_fopen(tmp_fname, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &field;
    args.argv[1] = fp;

    p_tbl = p_msg->data;
    p_tbl_info->ops.dump_fn(p_tbl, &args);
    
    if (p_ds_node)
    {
        field.field_id[1] = FLD_ID_ALL;
        p_ds_info = p_ds_node->tbl_info;
        p_ds = p_msg->data + p_tbl_node->tbl_info->key_size;
        p_ds_info->ops.dump_fn(p_ds, &args);
        
        sal_fprintf(fp, "\n");
        if (p_ds2_node)
        {
            p_ds2 = p_ds + p_ds_info->data_size;
            p_ds2_info = p_ds2_node->tbl_info;
            p_ds2_info->ops.dump_fn(p_ds2, &args);
        }
    }

    sal_fseek(fp, 0, SEEK_SET);

    sal_fgets(p_line->tbl_line, FILE_READ_LINE_32K_LEN, fp);
    if (p_ds_node)
    {
        sal_fgets(p_line->ds_line, FILE_READ_LINE_32K_LEN, fp);
        if (p_ds2_node)
        {
            sal_fgets(p_line->ds2_line, FILE_READ_LINE_32K_LEN, fp);
        }
    }

    sal_fclose(fp);
    sal_unlink(tmp_fname);
    
    return PM_E_NONE;
}

int32
cdbctl_process_format_subscribe_msg(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
{
    struct timeval tv;
    uint64 diff = 0;
    char  output[FILE_READ_LINE_32K_LEN];
    cdbctl_line_t lines;
    char *action = NULL;
    char path[CMD_BUF_256];
    char field_str[CMD_BUF_256];
    cdb_node_t *p_fld_node = NULL;
    uint32 field_id = 0;
    int32 rc = PM_E_NONE;

    path[0] = CMD_ZERO_CHAR;
    field_str[0] = CMD_ZERO_CHAR;

    sal_gettimeofday(&tv);
    
    if (CDB_OPER_SYNC_REFER != p_msg->u_hdr.data.oper)
    {
        cdbctl_process_split_and_dump_msg(p_tbl_node, p_ds_node, p_ds2_node, p_msg, &lines);
    }

    switch (p_msg->u_hdr.data.oper)
    {
    case CDB_OPER_ADD:
        action = "create";
        sal_sprintf(output, "sequence=%u, action=create\n", p_msg->hdr.seq);
        break;
    case CDB_OPER_DEL:
        action = "delete";
        sal_sprintf(output, "sequence=%u, action=delete\n", p_msg->hdr.seq);
        break;
    case CDB_OPER_SET:
        action = "update";
        field_id = p_msg->u_hdr.data.fields[0];
        p_fld_node = p_tbl_node;
        if (p_ds_node)
        {
            p_fld_node = p_ds_node;
            if (p_ds2_node)
            {
                p_fld_node = p_ds2_node;
            }
        }
        sal_sprintf(field_str, ", field=%u(%s)", field_id, p_fld_node->tbl_info->field[field_id].name);
        break;
    case CDB_OPER_SYNC_REFER:
        action = "sync_refer";
        break;
            
    default:
        action = "unknown";
        break;
    }

    if (p_ds_node)
    {
        sal_sprintf(path, "%s/%s", p_tbl_node->name, p_ds_node->name);    
        if (p_ds2_node)
        {
            sal_sprintf(path, "%s/%s/%s", p_tbl_node->name, p_ds_node->name, p_ds2_node->name);
        }
    }
    else
    {
        sal_sprintf(path, "%s", p_tbl_node->name);   
    }


    if (g_subscribe_action_fn)
    {
        diff = (tv.tv_sec - g_last_tv.tv_sec)*1000000 + (tv.tv_usec - g_last_tv.tv_usec);
        sal_sprintf(output, "------------------------------------------------------------\n"
            "TS %u.%06u s, delay %"PRIu64" us, length=%u, sequence=%u, action=%s, path=%s%s\n", 
            (uint32)tv.tv_sec, (uint32)tv.tv_usec, diff,
            p_msg->hdr.length, p_msg->hdr.seq, action, path, field_str);
        g_last_tv = tv;
        g_subscribe_action_fn(g_out_format, output);

        if (CDB_OPER_SYNC_REFER != p_msg->u_hdr.data.oper)
        {
            sal_sprintf(output, "%-12s %s", p_tbl_node->name, lines.tbl_line);
            g_subscribe_action_fn(g_out_format, output);

            if (p_ds_node)
            {
                sal_sprintf(output, "%-12s %s", p_ds_node->name, lines.ds_line);
                g_subscribe_action_fn(g_out_format, output);
            
                if (p_ds2_node)
                {
                    sal_sprintf(output, "%-12s %s", p_ds2_node->name, lines.ds2_line);
                    g_subscribe_action_fn(g_out_format, output);
                }
            }
        }

        sal_sprintf(output, "\n");
        g_subscribe_action_fn(g_out_format, output);
    }

    return rc;
}

static int32
_cdbctl_process_react_msg_internal(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    cdb_node_t* p_tbl_node = NULL;
    cdb_node_t* p_ds_node = NULL;
    cdb_node_t* p_ds2_node = NULL;
    int32 rc = PM_E_NONE;

    g_subscribe_count++;

    /* 1. get node */
    p_tbl_node = cdb_get_tbl(p_msg->u_hdr.data.tbl_id);
    if (NULL == p_tbl_node)
    {
        log_sys(M_MOD_CDS, E_ERROR, "Invalid tbl id %u\n", p_msg->u_hdr.data.tbl_id);
        return PM_E_FAIL;
    }
    
    if (CDB_NODE_DS == p_msg->u_hdr.data.type)
    {
        p_ds_node = cdb_get_ds(p_msg->u_hdr.data.ds_id);
        if (NULL == p_ds_node)
        {
            log_sys(M_MOD_CDS, E_ERROR, "Invalid ds id %u\n", p_msg->u_hdr.data.ds_id);
            return PM_E_FAIL;
        }
        if (0 != p_msg->u_hdr.data.ds2_id && INVALID_CDB_ID != p_msg->u_hdr.data.ds2_id)
        {
            p_ds2_node = cdb_get_ds(p_msg->u_hdr.data.ds2_id);
            if (NULL == p_ds2_node)
            {
                log_sys(M_MOD_CDS, E_ERROR, "Invalid ds2 id %u\n", p_msg->u_hdr.data.ds2_id);
                return PM_E_FAIL;
            }
        }
    }

    if (g_subscribe_user_fn)
    {
        g_subscribe_user_fn(p_tbl_node, p_ds_node, p_ds2_node, p_msg);
    }
    else
    {
        cdbctl_process_format_subscribe_msg(p_tbl_node, p_ds_node, p_ds2_node, p_msg);
    }

    return rc;
}

static int32
_cdbctl_process_react_msg_protobuf(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    struct timeval tv;
    uint64 diff = 0;
    Cdb__SubscribeResponse *p_hdr = NULL;
    uint8 *hdr_data = NULL;
    uint8 *payload_data = NULL;    
    char  out[MSG_SIZE];
    int32 payload_len = 0;
    void *object = NULL;
    uint32 tbl_id = 0;
    uint32 ds_id = 0;
    uint32 only_key = FALSE;
    cdb_node_t *p_tbl_node = NULL;
    cdb_node_t *p_ds_node  = NULL;
    int32 rc = PM_E_NONE;
    
    g_subscribe_count++;
    sal_gettimeofday(&tv);
    diff = (tv.tv_sec - g_last_tv.tv_sec)*1000000 + (tv.tv_usec - g_last_tv.tv_usec);
    sal_printf("------------------------------------------------------------\n");
    sal_printf("TS %u.%06u s, delay %"PRIu64" us, length=%u, sequence=%u,", (uint32)tv.tv_sec, (uint32)tv.tv_usec, diff, p_msg->hdr.length, p_msg->hdr.seq);
    g_last_tv = tv;
    
    if (p_msg->data_len < CDB_SUBSCRIBE_RESP_HDR_LEN)
    {
        sal_printf("receive %u bytes data, less than minimal subscribe header length %u\n", 
            p_msg->data_len, CDB_SUBSCRIBE_RESP_HDR_LEN);
        return PM_E_FAIL;
    }
    
    //unpack student info from buf
    hdr_data = p_msg->data;
    p_hdr = cdb__subscribe_response__unpack(&osp_pbc_allocator, CDB_SUBSCRIBE_RESP_HDR_LEN, hdr_data);
    if (NULL == p_hdr)
    {
        sal_printf("decode subscribe header failed\n");
        return PM_E_FAIL;
    }

    /* dump hdr */
    pb_subscribe_response_dump(p_hdr, out);
    sal_printf("%s\n", out);

    payload_len = p_msg->data_len - CDB_SUBSCRIBE_RESP_HDR_LEN;
    payload_data = hdr_data + CDB_SUBSCRIBE_RESP_HDR_LEN;
    if (payload_len < 0)
    {
        sal_printf("decode subscribe payload has invalid payload length %d\n", payload_len);
        return PM_E_FAIL;
    }
    
    if (INVALID_UINT32_ID != p_hdr->ds_id)
    {
        if (INVALID_UINT32_ID != p_hdr->ds2_id)
        {
            ds_id = p_hdr->ds2_id;
        }
        else
        {
            ds_id = p_hdr->ds_id;
        }
        p_ds_node = cdb_get_ds(ds_id);
        if (NULL == p_ds_node)
        {
            goto free_hdr;
        }
        object = pb_ds_unpack(ds_id, &osp_pbc_allocator, payload_len, payload_data);
        if (NULL == p_hdr)
        {
            sal_printf("decode subscribe payload ds failed\n");
            goto free_hdr;
        }
        
        /* dump payload ds */
        sal_printf("DS: %-s  ", p_ds_node->name);
        pb_ds_dump(ds_id, object, out);
        sal_printf("%s", out);
        //free payload msg
        pb_ds_free_unpacked(ds_id, object, &osp_pbc_allocator);
    }
    else
    {
        tbl_id = p_hdr->tbl_id;
        p_tbl_node = cdb_get_tbl(tbl_id);
        if (NULL == p_tbl_node)
        {
            goto free_hdr;
        }
        object = pb_tbl_unpack(tbl_id, &osp_pbc_allocator, payload_len, payload_data);
        if (NULL == object)
        {
            sal_printf("decode subscribe payload table failed, payload_len=%u\n", payload_len);
            goto free_hdr;
        }
        
        /* dump payload tbl */
        if (CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_DEL == p_hdr->action)
        {
            only_key = TRUE;
        }
        else
        {
            only_key = FALSE;
        }
        sal_printf("TBL: %-s  ", p_tbl_node->name);
        pb_tbl_dump(tbl_id, object, only_key, out);
        sal_printf("%s", out);
        //free payload msg
        pb_tbl_free_unpacked(tbl_id, object, &osp_pbc_allocator);
    }

free_hdr:    
    //free hdr msg
    cdb__subscribe_response__free_unpacked(p_hdr, &osp_pbc_allocator);
    
    return rc;
}

int32 
cdbctl_subscribe_action_log(cdb_subscribe_format_t format, char *output)
{
    char buf[FILE_READ_LINE_32K_LEN];

    sal_snprintf(buf, FILE_READ_LINE_32K_LEN, "logger -p 4 %s", output);
    system(buf);
    
    return PM_E_NONE;
}

int32 
cdbctl_subscribe_action_print(cdb_subscribe_format_t format, char *output)
{
    sal_printf("%s", output);
    return PM_E_NONE;
}

int32 
cdbctl_subscribe_action_script(cdb_subscribe_format_t format, char *output)
{
    char buf[FILE_READ_LINE_32K_LEN];

    sal_snprintf(buf, FILE_READ_LINE_32K_LEN, "%s %s\n", g_cdbctl_subscribe_script_path, output);
    system(buf);
    return PM_E_NONE;
}

int32
cdbclt_subscribe_register_action_cb(CDB_SUBSCRIBE_ACTION_FUNC fn)
{
    g_subscribe_action_fn = fn;
    return PM_E_NONE;
}

int32
cdbclt_subscribe_register_user_cb(CDB_SUBSCRIBE_USER_FUNC fn)
{
    g_subscribe_user_fn = fn;
    return PM_E_NONE;
}

int32
cdbclt_subscribe_init_cdb(char *path, cdb_subscribe_format_t format)
{
    cfg_parse_info_t parse_info;
    char new_path[MAX_CMD_STR_LEN];
    char key[MAX_CMD_STR_LEN];
    cdb_node_t *p_node = NULL;
    char *p_data = NULL;
    field_parse_t field;
    pb_subscribe_request_t req;
    uint8 buf[MAX_CMD_STR_LEN];
    int32 len = 0;
    int32 rc = PM_E_NONE;

    g_out_format = format;

    /* 1. set react callback function */
    if ((CDB_SUB_FORMAT_RAW == format) || (CDB_SUB_FORMAT_RAWSNMP == format))
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_internal);
    }
    else if (CDB_SUB_FORMAT_PROTOBUF == format)
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_protobuf);
    }
    
    sal_strncpy(new_path, path, MAX_CMD_STR_LEN);
    if (0 == sal_strlen(path) || CMD_DIVISION_CHAR != path[0])
    {
        sal_printf("%% Command: set react path failed, path should start with '/'\n");
        return PM_E_FAIL;
    }

    p_node = cdb_get_node_by_name(new_path);
    if (NULL == p_node)
    {
        sal_printf("%% Command: set react path failed, invalid path %s\n", path);
        return PM_E_FAIL;
    }

    p_data = path;
    p_data = sal_strstr(p_data, p_node->full_name);
    if (NULL == p_data)
    {
        SAL_ASSERT(0);
    }
    p_data += sal_strlen(p_node->full_name);
    if (CMD_DIVISION_CHAR == p_data[0])
    {
        p_data++;
    }

    if (sal_strlen(p_data) == 0)
    {
        sal_memset(&parse_info, 0, sizeof(parse_info));
    }
    else
    {
        rc = cfg_cmd_parse_para(p_data, &parse_info);
    }

    key[0] = CMD_ZERO_CHAR;
    if (0 == parse_info.argc)
    {
        sal_memset(&field, FLD_ID_ALL, sizeof(field));
        sal_strcat(key, "*");
    }
    else
    {
        if (CDB_NODE_TBL == p_node->type)
        {
            if (1 == parse_info.argc)
            {
                sal_memset(&field, FLD_ID_ALL, sizeof(field));
                sal_strcat(key, parse_info.argv[0]);
            }
            else if (2 == parse_info.argc)
            {
                sal_strcat(key, parse_info.argv[0]);
                rc = cfg_cmd_parse_field(parse_info.argv, parse_info.argc, p_node, &field, TRUE);
                if (rc < 0)
                {
                    sal_printf("Invalid field name %s\n", parse_info.argv[1]);
                    return PM_E_FAIL;
                }
            }
        }
    }

    if ((CDB_SUB_FORMAT_RAW == format) || (CDB_SUB_FORMAT_RAWSNMP == format))
    {
        ipc_ctl_to_cds_sub_cdb(p_node, NULL, &field, key, sal_strlen(key));
    }
    else
    {
        sal_memset(&req, 0, sizeof(req));

        req.type = CDB_SUB_CDB;
        req.pid = sal_getpid();
        req.src = PM_ID_CTL;
        req.format = format;
        req.cdb.node_type = p_node->type;
        req.cdb.tbl_id = p_node->id;
        req.cdb.field_id = field.field_id[0];
        req.cdb.key = key;
        len = pb_subscribe_request_pack(&req, buf, MAX_CMD_STR_LEN);
        ipc_ctl_to_ccs_sub_req(buf, len);
    }

    return PM_E_NONE;
}

int32
cdbclt_subscribe_init_tbl(char *tables, cdb_subscribe_format_t format, uint32 sync)
{
    cdb_node_t *p_node = NULL;
    cfg_parse_get_info_t parse_info;
    uint32 i = 0;
    uint32 tbl_id[TBL_MAX];
    uint32 tbl_num = 0;
    uint32 snmp = 0;
    int32 rc = PM_E_NONE;

    g_out_format = format;

    /* 1. set react callback function */
    if ((CDB_SUB_FORMAT_RAW == format) || (CDB_SUB_FORMAT_RAWSNMP == format))
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_internal);
    }
    else if (CDB_SUB_FORMAT_PROTOBUF == format)
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_protobuf);
    }

    rc = cfg_get_parse_comma(tables, &parse_info);
    if (rc < 0)
    {
        sal_printf("%% Command: Invalid tables list\n");
        return PM_E_FAIL;
    }

    for (i = 0; i < parse_info.argc; i++)
    {
        p_node = cdb_get_tbl_by_name(parse_info.argv[i]);
        if (NULL == p_node)
        {
            sal_printf("%% Command: Unknown table %s\n", parse_info.argv[i]);
            return PM_E_FAIL;
        }
        tbl_id[i] = p_node->id;
        tbl_num++;
    }

    snmp = (CDB_SUB_FORMAT_RAWSNMP == format) ? 1 : 0;
    ipc_ctl_to_cds_sub_tbl(tbl_id, tbl_num, sync, snmp);
    return PM_E_NONE;
}

int32
cdbclt_subscribe_init_tbl_new(char *tables, cdb_subscribe_format_t format, uint32 sync)
{
    cdb_node_t *p_node = NULL;
    cfg_parse_get_info_t parse_info;
    pb_subscribe_request_t req;
    uint32 i = 0;
    uint32 tbl_id[TBL_MAX];
    uint32 tbl_num = 0;
    uint8 buf[MAX_CMD_STR_LEN];
    int32 len = 0;
    int32 rc = PM_E_NONE;

    g_out_format = format;

    /* 1. set react callback function */
    if ((CDB_SUB_FORMAT_RAW == format) || (CDB_SUB_FORMAT_RAWSNMP == format))
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_internal);
    }
    else if (CDB_SUB_FORMAT_PROTOBUF == format)
    {
        ctc_sock_register_rx_fn(_cdbctl_process_react_msg_protobuf);
    }
    
    rc = cfg_get_parse_comma(tables, &parse_info);
    if (rc < 0)
    {
        sal_printf("%% Command: Invalid tables list\n");
        return PM_E_FAIL;
    }

    for (i = 0; i < parse_info.argc; i++)
    {
        p_node = cdb_get_tbl_by_name(parse_info.argv[i]);
        if (NULL == p_node)
        {
            sal_printf("%% Command: Unknown table %s\n", parse_info.argv[i]);
            return PM_E_FAIL;
        }
        tbl_id[i] = p_node->id;
        tbl_num++;
    }

    sal_memset(&req, 0, sizeof(req));

    req.type = CDB_SUB_TBL;
    req.pid = sal_getpid();
    req.src = PM_ID_CTL;
    req.format = format;
    req.tbl.sync = sync;
    req.tbl.tbl_id_list = tbl_id;
    req.tbl.num = tbl_num;
    
    len = pb_subscribe_request_pack(&req, buf, MAX_CMD_STR_LEN);
    ipc_ctl_to_ccs_sub_req(buf, len);
    
    return PM_E_NONE;
}



#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_ipsla_packet_stats_define.h"
#include "gen/tbl_ipsla_packet_stats.h"
#include "gen/tbl_ipsla_test_stats_define.h"
#include "gen/tbl_ipsla_test_stats.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "ipsla.h"
#include "ipsla_api.h"


extern char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

static char *
ipsla_cli_return(int ret)
{
    char *str = NULL;

    switch (ret)
    {
    case IPSLA_CREATE_ENTRY_ERR:
        str = "IP sla create entry error";
        break;
    case IPSLA_SET_ERR:
        str = "IP sla set error";
        break;
    case IPSLA_CREATE_SOCKET_ERR:
        str = "IP sla create socket error";
        break;
    case IPSLA_SHOW_ERR:
        str = "IP sla entry not found";
        break;
    case IPSLA_ENTRY_NOT_EXIST_ERR:
        str = "IP sla entry not exist";
        break;
    case IPSLA_ENTRY_NOT_ENABLE_ERR:
        str = "IP sla entry must be enabled first";
        break;
    case IPSLA_ENTRY_NOT_DISABLE_ERR:
        str = "IP sla entry must be disabled first";
        break;
    case IPSLA_ENTRY_TYPE_ERR:
        str = "IP sla entry type unknown";
        break;
    case IPSLA_ENTRY_ADDR_ERR:
        str = "IP sla entry address error";  
        break;
    case IPSLA_ENTRY_INPUT_IPADDRESS_ERR:
        str = "IP sla input ip address error";
        break;
    case IPSLA_ENTRY_FREQUENCY_ERR:
        str = "IP sla input frequency is less than timeout";  
        break;
    case IPSLA_ENTRY_TIMEOUT_ERR:
        str = "IP sla input timeout is less than threshold";  
        break;
    case IPSLA_ENTRY_THRESHOLD_ERR:
        str = "IP sla input threshold is larger than timeout";  
        break;
    case IPSLA_ENTRY_BROADCAST_DISALLOWED_ERR:
        str = "IP sla broadcast address is not allowed";
        break;
    case IPSLA_ENTRY_SOCKET_SET_ERR:
        str = "IP sla socket configure error";
        break;
    case IPSLA_ENTRY_PKT_NEW_ERR:
        str = "IP sla ICMP packet create error";
        break;
    case IPSLA_ENTRY_DATA_PATTERN_ERR:
        str = "Wrong value of data pattern";
        break;
    case IPSLA_SOCKET_BIND_ERR:
        str = "Failed to set source ip";
        break;
    case IPSLA_GLOBAL_NOT_FOUND_ERR:
        str = "IP sla global table not found";
        break;
    case IPSLA_CREATE_ENTRY_OVER_ERR:
        str = "IP sla create entry exceeding maximum entry limit";
        break;
    case IPSLA_ENTRY_SET_OUTRANGE_ERR:
        str = "IP sla parameter illegal";
        break;
    default:
        break;
    }

    return str;
}

static int32
_ipsla_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;
    tbl_ipsla_global_t     *p_db_ipsla_glb = NULL;
    int32                   field_id = 0;
    field_parse_t           field;

    sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;

    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_ipsla_global_dump_one(p_db_ipsla_glb, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_ipsla_cmd_set_snmp_trap(tbl_ipsla_global_t *p_glb, tbl_ipsla_global_t *p_db_glb,
                         int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(tbl_ipsla_global_set_ipsla_global_field(p_glb, field_id));
    return rc;
}

int32
ipsla_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    cdb_node_t            *p_node = NULL;
    tbl_ipsla_global_t     ipsla_glb;
    tbl_ipsla_global_t    *p_db_ipsla_glb = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field = NULL;
    int32                  value = 0;
    int32                  i = 0;

    CFG_DBG_FUNC();

    sal_memset(&ipsla_glb, 0, sizeof(tbl_ipsla_global_t));

    p_node = cdb_get_tbl(TBL_IPSLA_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
        if (!p_db_ipsla_glb)
        {
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
        }

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            ipsla_glb.trap_test_failure = value;
            _ipsla_cmd_set_snmp_trap(&ipsla_glb, p_db_ipsla_glb, field_id, para);
            break;

        case TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            ipsla_glb.trap_test_success = value;
            _ipsla_cmd_set_snmp_trap(&ipsla_glb, p_db_ipsla_glb, field_id, para);
            break;

        case TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            ipsla_glb.trap_pkt_timeout = value;
            _ipsla_cmd_set_snmp_trap(&ipsla_glb, p_db_ipsla_glb, field_id, para);
            break;

        case TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            ipsla_glb.trap_overthreshold = value;
            _ipsla_cmd_set_snmp_trap(&ipsla_glb, p_db_ipsla_glb, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _ipsla_cmd_get_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_ipsla_cmd_create_entry(uint32 entry_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_ipsla_entry_t         entry;
    tbl_ipsla_entry_t        *p_db_entry     = NULL;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    sal_memset(&entry, 0, sizeof(tbl_ipsla_entry_t));

    if (IPSLA_ENTRYID_MIN > entry_id || IPSLA_ENTRYID_MAX < entry_id)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    /* if ipsla entry is exist, return */
    entry.key.entry_id = entry_id;
    p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&entry.key);
    if (p_db_entry)
    {
        return PM_E_NONE;
    }

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
    }

    /* check ipsla entry number */
    if (p_db_ipsla_glb->ipsla_entry_num >= GLB_IPSLA_ENTRY_MAX)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_CREATE_ENTRY_OVER_ERR));
    }

    /* set default parameter */
    entry.type         = IPSLA_TYPE_UNKNOWN;
    entry.admin_state  = IPSLA_DISABLE;
    entry.inaddr.s_addr  = 0;
    sal_memset(entry.desc, 0, IPSLA_DES_MAX_SIZE+1);
    entry.sock         = 0;
    entry.send_sock    = 0;
    entry.t_ipsla_read = NULL;

    entry.frequency    = GLB_IPSLA_FREQUENCY_DEFAULT;
    entry.timeout      = GLB_IPSLA_TIMEOUT_DEFAULT;
    entry.threshold    = GLB_IPSLA_THRESHOLD_DEFAULT;
    entry.interval     = GLB_IPSLA_INTERVAL_DEFAULT;
    entry.packets_per_test = GLB_IPSLA_PING_PROBE_COUNT_DEFAULT;
    entry.fail_percent = GLB_IPSLA_FAIL_PERCENT_DEFAULT;

    entry.running_frequency = 0;
    entry.running_timeout   = 0;
    entry.running_interval  = 0;
    entry.running_probe_cnt = 0;

    entry.testid   = 0;
    entry.sequence = 0;
    entry.rtt      = 0;

    entry.returncode = ECHO_UNKNOWN;
    entry.echo_reply = IPSLA_NO_ECHO_REPLY;
    entry.src_inaddr.s_addr = 0;
    entry.src_ifname[0] = 0;

    entry.ttl = GLB_IPSLA_PING_TTL_DEFAULT;
    entry.tos = GLB_IPSLA_PING_TOS_DEFAULT;
    entry.data_size    = GLB_IPSLA_DATA_SIZE_DEFAULT;
    entry.data_pattern = GLB_IPSLA_DATA_PATTERN_DEFAULT;
    sal_memset(entry.buf, 0, GLB_IPSLA_PKT_BUF);

    entry.probe_result_config_cnt = GLB_IPSLA_RESULT_PROBE_NUM_DEFAULT;
    entry.test_result_config_cnt  = GLB_IPSLA_RESULT_TEST_NUM_DEFAULT;
    entry.probe_result_index = 1;
    entry.test_result_index  = 1;

    rc = tbl_ipsla_entry_add_ipsla_entry(&entry);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET(ipsla_cli_return(IPSLA_CREATE_ENTRY_ERR));
    }

    if (0 == p_db_ipsla_glb->ipsla_entry_num && !p_db_ipsla_glb->t_ipsla_timer)
    {
        /* start ipsla timer */
        ipsla_sys_init_timer(p_db_ipsla_glb, TRUE);
    }

    p_db_ipsla_glb->ipsla_entry_num++;
    PM_E_RETURN(tbl_ipsla_global_set_ipsla_global_field(p_db_ipsla_glb, TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM));

    return rc;
}

static int32
_ipsla_cmd_delete_entry(uint32 entry_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_ipsla_entry_key_t     entry_key;
    tbl_ipsla_entry_t        *p_db_entry = NULL;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    sal_memset(&entry_key, 0, sizeof(entry_key));

    entry_key.entry_id = entry_id;
    p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&entry_key);
    if (!p_db_entry)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_EXIST_ERR));
    }

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
    }

    PM_E_RETURN(ipsla_disable_entry(p_db_entry));

    PM_E_RETURN(tbl_ipsla_entry_del_ipsla_entry(&p_db_entry->key));

    p_db_ipsla_glb->ipsla_entry_num--;
    PM_E_RETURN(tbl_ipsla_global_set_ipsla_global_field(p_db_ipsla_glb, TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM));

    if (0 == p_db_ipsla_glb->ipsla_entry_num)
    {
        /* stop ipsla timer */
        ipsla_sys_init_timer(p_db_ipsla_glb, FALSE);
    }

    return rc;
}

static int32
_ipsla_cmd_set_entry_type(tbl_ipsla_entry_t *p_db_entry, tbl_ipsla_entry_t *p_entry, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    tbl_interface_t        intf;
    tbl_interface_t       *p_db_if = NULL;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_TYPE_UNKNOWN == p_entry->type)
    {
        p_db_entry->type = IPSLA_TYPE_UNKNOWN;
        p_db_entry->inaddr.s_addr = 0;
        p_db_entry->src_inaddr.s_addr = 0;
        sal_memset(p_db_entry->src_ifname, 0, IFNAME_SIZE);
    }
    else
    {
        if (ipsla_check_ip(&(p_entry->inaddr)))
        {
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_INPUT_IPADDRESS_ERR));
        }
        if (p_entry->src_inaddr.s_addr && ipsla_check_ip(&(p_entry->src_inaddr)))
        {
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_INPUT_IPADDRESS_ERR));
        }

        p_db_entry->type = IPSLA_TYPE_ECHO;
        p_db_entry->inaddr.s_addr = p_entry->inaddr.s_addr;
        p_db_entry->src_inaddr.s_addr = p_entry->src_inaddr.s_addr;

        if(sal_strlen(p_entry->src_ifname))
        {
            sal_memset(&intf, 0, sizeof(tbl_interface_t));
            sal_strncpy(intf.key.name, p_entry->src_ifname, IFNAME_SIZE);

            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                CFG_CONFLICT_RET("Not found interface %s", p_entry->src_ifname);
            }

            /* check src intf */
            if ((GLB_IF_MODE_L2 == p_db_if->mode)
                || ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
                && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
                && (GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type))
                || ((INTERFACE_IS_LAG_MEMBER(p_db_if)) && sal_memcmp(p_entry->src_ifname, "agg", sal_strlen("agg"))))
            {
                CFG_CONFLICT_RET("Source interface should be routed port (not agg member port), vlan interface or layer3 agg interface!");
            }

            sal_strncpy(p_db_entry->src_ifname, p_entry->src_ifname, IFNAME_SIZE);
        }
        else
        {
            sal_memset(p_db_entry->src_ifname, 0, IFNAME_SIZE);
        }
    }

    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_INADDR));
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_SRC_INADDR));
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_SRC_IFNAME));
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_TYPE));

    return rc;
}

static int32
_ipsla_cmd_set_entry_schedule(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32   rc = PM_E_NONE;

    if (IPSLA_ENABLE != value && IPSLA_DISABLE != value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (IPSLA_ENABLE == value && IPSLA_ENABLE == p_db_entry->admin_state)
    {
        CFG_PROMPT_RET("Notice: IP sla entry is already scheduled");
    }
    else if (IPSLA_DISABLE == value && IPSLA_DISABLE == p_db_entry->admin_state)
    {
        CFG_PROMPT_RET("Notice: IP sla entry is not scheduled");
    }

    if (p_db_entry->type == IPSLA_TYPE_UNKNOWN)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_TYPE_ERR));
    }

    if (IPSLA_ENABLE == value)
    {
        PM_E_RETURN(ipsla_enable_entry(p_db_entry));
    }
    else
    {
        p_db_entry->total_send = 0;
        p_db_entry->total_receive = 0;
        p_db_entry->rtt_sum = 0;
        p_db_entry->rtt_max = 0;
        p_db_entry->rtt_min = 0;

        PM_E_RETURN(ipsla_disable_entry(p_db_entry));
    }

    p_db_entry->admin_state = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_ADMIN_STATE));

    return rc;
}

static int32
_ipsla_cmd_set_entry_frequency(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    tbl_sys_global_t* p_glb = tbl_sys_global_get_sys_global();

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_FREQUENCY_MIN > value || IPSLA_FREQUENCY_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->frequency == value)
    {
        return rc;
    }

    if (p_glb && p_glb->startup_done)
    {
        IPSLA_TIME_CHECK(p_db_entry->timeout, p_db_entry->interval, value, p_db_entry->packets_per_test);
    }

    p_db_entry->frequency = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_FREQUENCY));

    return rc;
}

static int32
_ipsla_cmd_set_entry_timeout(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32   rc = PM_E_NONE;
    tbl_sys_global_t* p_glb = tbl_sys_global_get_sys_global();

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_TIMEOUT_MIN > value || IPSLA_TIMEOUT_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->timeout == value)
    {
        return rc;
    }

    if (p_glb && p_glb->startup_done)
    {
        IPSLA_TIME_CHECK(value, p_db_entry->interval, p_db_entry->frequency, p_db_entry->packets_per_test);
    }

    p_db_entry->timeout = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_TIMEOUT));

    return rc;
}

static int32
_ipsla_cmd_set_entry_interval(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    tbl_sys_global_t* p_glb = tbl_sys_global_get_sys_global();

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_INTERVAL_MIN > value || IPSLA_INTERVAL_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->interval == value)
    {
        return rc;
    }

    if (p_glb && p_glb->startup_done)
    {
        IPSLA_TIME_CHECK(p_db_entry->timeout, value, p_db_entry->frequency, p_db_entry->packets_per_test);
    }

    p_db_entry->interval = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_INTERVAL));

    return rc;
}

static int32
_ipsla_cmd_set_entry_threshold(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_THRESHOLD_MIN > value || IPSLA_THRESHOLD_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->threshold == value)
    {
        return rc;
    }

    p_db_entry->threshold = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_THRESHOLD));

    return rc;
}

static int32
_ipsla_cmd_set_entry_ttl(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_TTL_MIN > value || IPSLA_TTL_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->ttl == value)
    {
        return rc;
    }

    p_db_entry->ttl = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_TTL));

    return rc;
}

static int32
_ipsla_cmd_set_entry_tos(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_TOS_MIN > value || IPSLA_TOS_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->tos == value)
    {
        return rc;
    }

    p_db_entry->tos = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_TOS));

    return rc;
}

static int32
_ipsla_cmd_set_entry_data_size(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_DATA_SIZE_MIN > value || IPSLA_DATA_SIZE_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->data_size == value)
    {
        return rc;
    }

    p_db_entry->data_size = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_DATA_SIZE));

    return rc;
}

static int32
_ipsla_cmd_set_entry_data_pattern(tbl_ipsla_entry_t *p_db_entry, char *value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    uint32    patternLen   = sal_strlen(value);
    uint32    data_pattern = 0;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (patternLen > 8 || sal_strspn(value, "0123456789AaBbCcDdEeFf") != patternLen)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }
    data_pattern = strtoul(value, NULL, 16);

    if (p_db_entry->data_pattern == data_pattern)
    {
        return rc;
    }

    p_db_entry->data_pattern = data_pattern;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_DATA_PATTERN));

    return rc;
}

static int32
_ipsla_cmd_set_entry_fail_percent(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_FAIL_PERCENT_MIN > value || IPSLA_FAIL_PERCENT_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->fail_percent == value)
    {
        return rc;
    }

    p_db_entry->fail_percent = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT));

    return rc;
}

static int32
_ipsla_cmd_set_entry_packets_per_test(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    tbl_sys_global_t* p_glb = tbl_sys_global_get_sys_global();

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_PACKETS_PER_TEST_MIN > value || IPSLA_PACKETS_PER_TEST_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (p_db_entry->packets_per_test == value)
    {
        return rc;
    }

    if (p_glb && p_glb->startup_done)
    {
        IPSLA_TIME_CHECK(p_db_entry->timeout, p_db_entry->interval, p_db_entry->frequency, value);
    }

    p_db_entry->packets_per_test = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST));

    return rc;
}

static int32
_ipsla_cmd_set_entry_description(tbl_ipsla_entry_t *p_db_entry, char *desc, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (IPSLA_DISABLE != p_db_entry->admin_state)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_DISABLE_ERR));
    }

    if (IPSLA_DES_MAX_SIZE < sal_strlen(desc))
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    if (!sal_strcmp(p_db_entry->desc, desc))
    {
        return rc;
    }

    sal_strncpy(p_db_entry->desc, desc, IPSLA_DES_MAX_SIZE);
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_DESC));

    return rc;
}

static int32
_ipsla_cmd_get_entry(uint32  entry_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_ipsla_entry_key_t   key;
    tbl_ipsla_entry_t      *p_db_entry = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == entry_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ipsla_entry_dump_one, &args);
    }
    else
    {
        key.entry_id = entry_id;
        p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&key);
        if (!p_db_entry)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("ipsla entry %u does not exist", entry_id);
        }

        tbl_ipsla_entry_dump_one(p_db_entry, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_ipsla_cmd_set_entry_probe_result_cnt(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (p_db_entry->probe_result_config_cnt == value)
    {
        return rc;
    }

    if (IPSLA_PROBE_RESULT_CNT_MIN > value || IPSLA_PROBE_RESULT_CNT_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    ipsla_adjust_probe_result(p_db_entry, value);

    p_db_entry->probe_result_config_cnt = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT));

    return rc;
}

static int32
_ipsla_cmd_set_entry_test_result_cnt(tbl_ipsla_entry_t *p_db_entry, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;

    if (p_db_entry->test_result_config_cnt == value)
    {
        return rc;
    }

    if (IPSLA_TEST_RESULT_CNT_MIN > value || IPSLA_TEST_RESULT_CNT_MAX < value)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_SET_OUTRANGE_ERR));
    }

    ipsla_adjust_test_result(p_db_entry, value);

    p_db_entry->test_result_config_cnt = value;
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT));

    return rc;
}

int32
ipsla_cmd_process_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_ipsla_entry_t      entry;
    tbl_ipsla_entry_t     *p_db_entry  = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  value    = 0;
    int32                  i        = 0;
    uint32                 entry_id = 0;
    char                   desc[IPSLA_DES_MAX_SIZE+1];
    int32                  desc_len = 0;
    field_parse_t          field;

    CFG_DBG_FUNC();

    sal_memset(&entry, 0, sizeof(tbl_ipsla_entry_t));

    p_node = cdb_get_tbl(TBL_IPSLA_ENTRY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);

        rc = _ipsla_cmd_create_entry(entry_id, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);

        rc = _ipsla_cmd_delete_entry(entry_id, para);
        break;

    case CDB_OPER_SET:
        PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);

        entry.key.entry_id = entry_id;
        p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&entry.key);
        if (!p_db_entry)
        {
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_EXIST_ERR));
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_IPSLA_ENTRY_FLD_TYPE:
            if (argc > 2)
            {
                if (cdb_addr_ipv4_str2val(&entry.inaddr, argv[2], sal_strlen(argv[2])) < 0)
                {
                    CFG_CONFLICT_RET("Dst_inaddr pton error");
                }

                if (sal_strlen(argv[4]))
                {
                    if (cdb_addr_ipv4_str2val(&entry.src_inaddr, argv[4], sal_strlen(argv[4])) < 0)
                    {
                        CFG_CONFLICT_RET("Src_inaddr pton error");
                    }

                    if (ipsla_check_ip(&entry.src_inaddr))
                    {
                        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_INPUT_IPADDRESS_ERR));
                    }
                }

                if (argv[6])
                {
                    sal_strncpy(entry.src_ifname, argv[6], IFNAME_SIZE);
                }

                entry.type = IPSLA_TYPE_ECHO;
            }
            else
            {
                entry.type = IPSLA_TYPE_UNKNOWN;
            }

            rc = _ipsla_cmd_set_entry_type(p_db_entry, &entry, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_ADMIN_STATE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_schedule(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_DESC:
            if (argc == 3)
            {
                desc_len = sal_strlen(argv[i+1]);
                if (desc_len > IPSLA_DES_MAX_SIZE)
                {
                    CFG_INVALID_PARAM_RET("Description length should be [1, 255]");
                }

                sal_memset(desc, 0, IPSLA_DES_MAX_SIZE+1);
                sal_strncpy(desc, argv[i+1], IPSLA_DES_MAX_SIZE+1);
            }
            else
            {
                sal_memset(desc, 0, IPSLA_DES_MAX_SIZE+1);
            }

            rc = _ipsla_cmd_set_entry_description(p_db_entry, desc, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_FREQUENCY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_frequency(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_TIMEOUT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_timeout(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_THRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_threshold(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_interval(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_TTL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_ttl(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_TOS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_tos(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_packets_per_test(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_fail_percent(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_DATA_SIZE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_data_size(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_DATA_PATTERN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

            rc = _ipsla_cmd_set_entry_data_pattern(p_db_entry, argv[i+1], field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_probe_result_cnt(p_db_entry, value, field_id, para);
            break;

        case TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipsla_cmd_set_entry_test_result_cnt(p_db_entry, value, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _ipsla_cmd_get_entry(entry_id, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static void
ipsla_show_entry(cfg_cmd_para_t *para, FILE *fp, tbl_ipsla_entry_t *p_db_entry)
{
    char   ip_str[256];
    char   ifname_ext[IFNAME_EXT_SIZE];

    sal_fprintf(fp, "Entry %d\n", p_db_entry->key.entry_id);
    if (p_db_entry->type == IPSLA_TYPE_ECHO)
        sal_fprintf(fp, "    %-20s : %s\n", "Type", "Echo");
    else
        sal_fprintf(fp, "    %-20s : %s\n", "Type", "Unknown");

    if (sal_strlen(p_db_entry->desc))
    {
        sal_fprintf(fp, "    %-20s : %s\n", "Description", p_db_entry->desc);
    }

    if (p_db_entry->admin_state == IPSLA_ENABLE)
        sal_fprintf(fp, "    %-20s : %s\n", "Admin State", "Enable");
    else
        sal_fprintf(fp, "    %-20s : %s\n", "Admin State", "Disable");

    if (p_db_entry->inaddr.s_addr)
    {
        sal_inet_ntop(AF_INET, &p_db_entry->inaddr, ip_str, sizeof(ip_str));
        sal_fprintf(fp, "    %-20s : %s\n", "Destination Address", ip_str);
    }
    else
    {
        sal_fprintf(fp, "    %-20s : %s\n", "Destination Address", "0.0.0.0");
    }

    if (p_db_entry->src_inaddr.s_addr)
    {
        sal_inet_ntop(AF_INET, &p_db_entry->src_inaddr, ip_str, sizeof(ip_str));
        sal_fprintf(fp, "    %-20s : %s\n", "Source Address", ip_str);
    }

    if (sal_strlen(p_db_entry->src_ifname))
    {
        sal_fprintf(fp, "    %-20s : %s\n", "Source Interface", IFNAME_ETH2FULL(p_db_entry->src_ifname, ifname_ext));
    }

    sal_fprintf(fp, "    %-20s : %ds\n", "Frequency", p_db_entry->frequency);
    sal_fprintf(fp, "    %-20s : %ds\n", "Timeout", p_db_entry->timeout);
    sal_fprintf(fp, "    %-20s : %dms\n", "Threshold", p_db_entry->threshold);
    sal_fprintf(fp, "    %-20s : %ds\n", "Interval", p_db_entry->interval);
    sal_fprintf(fp, "    %-20s : %d \n", "Packet Per Test", p_db_entry->packets_per_test);
    sal_fprintf(fp, "    %-20s : %d \n", "TTL", p_db_entry->ttl);
    sal_fprintf(fp, "    %-20s : %d \n", "TOS", p_db_entry->tos);
    sal_fprintf(fp, "    %-20s : %d bytes\n", "Data Size", p_db_entry->data_size);
    sal_fprintf(fp, "    %-20s : %08x\n", "Data Pattern", p_db_entry->data_pattern);
    sal_fprintf(fp, "    %-20s : %d%%\n", "Fail Percent", p_db_entry->fail_percent);
    sal_fprintf(fp, "    %-20s : %d \n", "Packet Item Cnt", p_db_entry->probe_result_config_cnt);
    sal_fprintf(fp, "    %-20s : %d \n", "Test Item Cnt", p_db_entry->test_result_config_cnt);

    if (p_db_entry->running_frequency != 0)
        sal_fprintf(fp, "    %-20s : %ds\n", "Running Frequency", p_db_entry->running_frequency);

    if (p_db_entry->running_timeout != 0)
        sal_fprintf(fp, "    %-20s : %ds\n", "Running Timeout", p_db_entry->running_timeout);

    if (p_db_entry->running_interval != 0)
        sal_fprintf(fp, "    %-20s : %ds\n", "Running Interval", p_db_entry->running_interval);

    switch (p_db_entry->returncode)
    {
    case ECHO_OK:
        sal_fprintf(fp, "    %-20s : %s\n", "Return Code", "OK");
        break;
    case ECHO_OVERTHRESHOLD:
        sal_fprintf(fp, "    %-20s : %s\n", "Return Code", "OverThreshold");
        break;
    case ECHO_TIMEOUT:
        sal_fprintf(fp, "    %-20s : %s\n", "Return Code", "Timeout");
        break;
    case ECHO_UNREACHABLE:
        sal_fprintf(fp, "    %-20s : %s\n", "Return Code", "Unreachable");
        break;
    default:
        sal_fprintf(fp, "    %-20s : %s\n", "Return Code", "Unknown");
        break;
    }

    return;
}

static void
ipsla_packet_stats_show(cfg_cmd_para_t *para, FILE *fp, tbl_ipsla_entry_t *p_db_entry)
{
    tbl_ipsla_packet_stats_t  ipsla_pstats;
    tbl_ipsla_packet_stats_t *p_db_ipsla_pstats = NULL;
    int32 packet_cnt   = p_db_entry->probe_result_config_cnt;
    int32 packet_index = p_db_entry->probe_result_index - 1;
    int32 packet_index_show = 0;
    char label[20] = {0};
    char time[256] = {0};
    char rtt[20]   = {0};
    struct sal_timeval  tv;

    char *ipsla_echo_replay_code_str[IPSLA_ECHO_REPLY_CODE_MAX] = {
        "Timeout", "Success", "Overthreshold", "SendFail"
    };

    if (!p_db_entry->probe_result_config_cnt)
    {
        sal_fprintf(fp, "Notice: The configured number of packet results is zero\n");
        return;
    }

    sal_fprintf(fp, "%-7s %-10s %-10s %-15s %s", "No", "Test", "RTT", "Status", "Send-time\n");
    sal_fprintf(fp, "-------+----------+----------+---------------+-------------------\n");
    for ( ; packet_cnt > 0 && packet_index > 0; --packet_cnt, --packet_index)
    {
        ipsla_pstats.key.entry_id = p_db_entry->key.entry_id;
        ipsla_pstats.key.packet_index = packet_index;

        p_db_ipsla_pstats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&ipsla_pstats.key);
        if (p_db_ipsla_pstats)
        {
            packet_index_show++;
            tv.tv_sec = p_db_ipsla_pstats->send_time / 1000;
            format_time_str(time, 256, "%Y-%m-%d %H:%M:%S", tv.tv_sec);

            sal_snprintf(label, 20,"%d(%d)", p_db_ipsla_pstats->testid, p_db_ipsla_pstats->index);
            sal_snprintf(rtt, 20,"%dms", (p_db_ipsla_pstats->rtt
                     || ((IPSLA_GET_ECHO_REPLY == p_db_ipsla_pstats->status)
                      || (IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_ipsla_pstats->status))) ?
                         p_db_ipsla_pstats->rtt : p_db_ipsla_pstats->timeout * 1000);
            sal_fprintf(fp, "%-7d %-10s %-10s %-15s %s\n",
                        packet_index_show, label, rtt,
                        ipsla_echo_replay_code_str[p_db_ipsla_pstats->status], time);
        }
    }

    return;
}

static void
ipsla_test_stats_show(cfg_cmd_para_t *para, FILE *fp, tbl_ipsla_entry_t *p_db_entry)
{
    tbl_ipsla_test_stats_t  ipsla_tstats;
    tbl_ipsla_test_stats_t *p_db_ipsla_tstats = NULL;
    int32 test_cnt   = p_db_entry->test_result_config_cnt;
    int32 test_index = p_db_entry->test_result_index - 1;
    int32 test_index_show = 0;
    char time[256] = {0};
    struct sal_timeval  tv;

    for ( ; test_cnt > 0 && test_index > 0; --test_cnt, --test_index)
    {
        ipsla_tstats.key.entry_id = p_db_entry->key.entry_id;
        ipsla_tstats.key.test_index = test_index;

        p_db_ipsla_tstats = tbl_ipsla_test_stats_get_ipsla_test_stats(&ipsla_tstats.key);
        if (p_db_ipsla_tstats)
        {
            test_index_show++;
            tv.tv_sec = p_db_ipsla_tstats->last_send_time / 1000;
            format_time_str(time, 256, "%Y-%m-%d %H:%M:%S", tv.tv_sec);

            sal_fprintf(fp, "No : %d  Test ID : %u  Result: %s\n", test_index_show, p_db_ipsla_tstats->testid,
                        p_db_ipsla_tstats->success ? "success" : "fail");
            sal_fprintf(fp, "===========================================\n");
            sal_fprintf(fp, "Transmitted       : %u\n", p_db_ipsla_tstats->send_cnt);
            sal_fprintf(fp, "Received          : %u\n", p_db_ipsla_tstats->receive_cnt);
            sal_fprintf(fp, "Overthreshold     : %u\n", p_db_ipsla_tstats->overthreshold_cnt);
            sal_fprintf(fp, "Rtt min/max/avg   : %u/%u/%u ms\n",
                (p_db_ipsla_tstats->rtt_min || p_db_ipsla_tstats->success) ? p_db_ipsla_tstats->rtt_min : p_db_ipsla_tstats->timeout * 1000,
                (p_db_ipsla_tstats->rtt_max || p_db_ipsla_tstats->success) ? p_db_ipsla_tstats->rtt_max : p_db_ipsla_tstats->timeout * 1000,
                p_db_ipsla_tstats->receive_cnt ? p_db_ipsla_tstats->rtt_sum/p_db_ipsla_tstats->receive_cnt
              : p_db_ipsla_tstats->timeout * 1000);
            sal_fprintf(fp, "Last transmitted  : %s\n", time);
            sal_fprintf(fp, "Packet loss ratio : %u%%\n\n", p_db_ipsla_tstats->fail_percent);
        }
    }

    return;
}

int32
ipsla_cmd_show_ipsla(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc        = PM_E_NONE;
    FILE                     *fp        = NULL;
    tbl_ipsla_entry_master_t *p_master  = NULL;
    uint32                    entry_id  = 0;
    tbl_ipsla_entry_t        *p_db_entry     = NULL;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
    }
    uint16 ipsla_entry_num = p_db_ipsla_glb->ipsla_entry_num;

    p_master = tbl_ipsla_entry_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_EXIST_ERR));
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        sal_fprintf(fp, "ipsla entry count : %d\n", p_db_ipsla_glb->ipsla_entry_num);

        for (entry_id = 1; entry_id <= GLB_IPSLA_ENTRY_MAX && ipsla_entry_num > 0; entry_id++)
        {
            p_db_entry = p_master->ipslaentry_array[entry_id];
            if (!p_db_entry)
            {
                continue;
            }

            ipsla_show_entry(para, fp, p_db_entry);
            ipsla_entry_num--;
        }
    }
    else
    {
        PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);
        p_db_entry = p_master->ipslaentry_array[entry_id];

        if (!p_db_entry)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_SHOW_ERR));
        }

        if (1 == argc)
        {
            ipsla_show_entry(para, fp, p_db_entry);
        }
        else
        {
            if (!sal_strcmp("packet", argv[1]))
            {
                ipsla_packet_stats_show(para, fp, p_db_entry);
            }
            else if (!sal_strcmp("test", argv[1]))
            {
                ipsla_test_stats_show(para, fp, p_db_entry);
            }
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

static void
ipsla_clear_entry_stats(cfg_cmd_para_t *para, tbl_ipsla_entry_t *p_db_entry)
{
    tbl_ipsla_packet_stats_t  ipsla_pstats;
    tbl_ipsla_packet_stats_t *p_db_ipsla_pstats = NULL;
    int32 packet_cnt   = p_db_entry->probe_result_config_cnt;
    int32 packet_index = p_db_entry->probe_result_index - 1;
    tbl_ipsla_test_stats_t    ipsla_tstats;
    tbl_ipsla_test_stats_t   *p_db_ipsla_tstats = NULL;
    int32 test_cnt     = p_db_entry->test_result_config_cnt;
    int32 test_index   = p_db_entry->test_result_index - 1;

    for ( ; packet_cnt > 0 && packet_index > 0; --packet_cnt, --packet_index)
    {
        ipsla_pstats.key.entry_id = p_db_entry->key.entry_id;
        ipsla_pstats.key.packet_index = packet_index;

        p_db_ipsla_pstats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&ipsla_pstats.key);
        if (p_db_ipsla_pstats)
        {
            tbl_ipsla_packet_stats_del_ipsla_packet_stats(&ipsla_pstats.key);
        }
    }

    for ( ; test_cnt > 0 && test_index > 0; --test_cnt, --test_index)
    {
        ipsla_tstats.key.entry_id = p_db_entry->key.entry_id;
        ipsla_tstats.key.test_index = test_index;

        p_db_ipsla_tstats = tbl_ipsla_test_stats_get_ipsla_test_stats(&ipsla_tstats.key);
        if (p_db_ipsla_tstats)
        {
            tbl_ipsla_test_stats_del_ipsla_test_stats(&ipsla_tstats.key);
        }
    }

    return;
}

int32
ipsla_cmd_process_act_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32         rc         = PM_E_NONE;
    uint32        entry_id   = 0;
    tbl_ipsla_entry_master_t *p_master  = NULL;
    tbl_ipsla_entry_t        *p_db_entry = NULL;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_GLOBAL_NOT_FOUND_ERR));
    }
    uint16 ipsla_entry_num = p_db_ipsla_glb->ipsla_entry_num;

    p_master = tbl_ipsla_entry_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_ENTRY_NOT_EXIST_ERR));
    }

    if (0 == argc)
    {
        for (entry_id = 1; entry_id <= GLB_IPSLA_ENTRY_MAX && ipsla_entry_num > 0; entry_id++)
        {
            p_db_entry = p_master->ipslaentry_array[entry_id];
            if (!p_db_entry)
            {
                continue;
            }

            ipsla_clear_entry_stats(para, p_db_entry);
            ipsla_entry_num--;
        }
    }
    else
    {
        PM_CFG_GET_UINT(entry_id, argv[0], para->p_rs);
        p_db_entry = p_master->ipslaentry_array[entry_id];

        if (!p_db_entry)
        {
            CFG_CONFLICT_RET(ipsla_cli_return(IPSLA_SHOW_ERR));
        }

        ipsla_clear_entry_stats(para, p_db_entry);
    }

    return PM_E_NONE;
}



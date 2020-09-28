
#include "proto.h"
#include "gen/tbl_sys_load_define.h"
#include "gen/tbl_sys_load.h"

#include "switch.h"
#include "swth_load.h"

static int32
_swth_cmd_set_load_sys_load_en(uint32 en, cfg_cmd_para_t *para)
{
    tbl_sys_load_t *p_sys_load = tbl_sys_load_get_sys_load();

    p_sys_load->sys_load_en = en;
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_SYS_LOAD_EN);
    
    return PM_E_NONE;
}

static int32
_swth_cmd_set_load_back_pressure_en(uint32 en, cfg_cmd_para_t *para)
{
    tbl_sys_load_t *p_sys_load = tbl_sys_load_get_sys_load();

    p_sys_load->cli_back_pressure_en = en;
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN);
    
    return PM_E_NONE;
}

static int32
_swth_cmd_set_load_notice_threshold(uint32 en, cfg_cmd_para_t *para)
{
    tbl_sys_load_t *p_sys_load = tbl_sys_load_get_sys_load();

    p_sys_load->cdb_queue_notice_threshold = en;
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD);
    
    return PM_E_NONE;
}

static int32
_swth_cmd_set_load_warning_threshold(uint32 en, cfg_cmd_para_t *para)
{
    tbl_sys_load_t *p_sys_load = tbl_sys_load_get_sys_load();

    p_sys_load->cdb_queue_warning_threshold = en;
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD);
    
    return PM_E_NONE;
}

static int32
_swth_cmd_get_load(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    tbl_sys_load_t *p_db_load = tbl_sys_load_get_sys_load();
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_sys_load_dump_one(p_db_load, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
swth_cmd_process_load(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    uint32 value = 0;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(TBL_SYS_LOAD);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        rc = cfg_cmd_parse_field(argv, 1, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        PM_CFG_GET_UINT(value, argv[1], para->p_rs);

        switch (field.field_id[0])
        {
        case TBL_SYS_LOAD_FLD_SYS_LOAD_EN:
            _swth_cmd_set_load_sys_load_en(value, para);
            break;
        case TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN:
            _swth_cmd_set_load_back_pressure_en(value, para);
            break;
        case TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD:
            _swth_cmd_set_load_notice_threshold(value, para);
            break;
        case TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD:
            _swth_cmd_set_load_warning_threshold(value, para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        break;
        
    case CDB_OPER_GET:
        if (0 == argc)
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        else if (1 == argc)
        {
            rc = cfg_cmd_parse_field(argv, 1, p_node, &field, FALSE);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid argc count %u", argc);
        }
        rc = _swth_cmd_get_load(para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


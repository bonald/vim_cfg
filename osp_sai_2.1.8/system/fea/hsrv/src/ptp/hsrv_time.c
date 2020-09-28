#include "hsrv_inc.h"
#include "hsrv_ptp.h"
#include "hsrv_time.h"
#include "gen/tbl_fea_time_define.h"
#include "gen/tbl_fea_time.h"

int32
hsrv_time_cmd_process_sync_systime(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_fea_time_t *p_db_time = tbl_fea_time_get_fea_time();
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (GLB_TIME_SYNC_SYSTIME == p_db_time->sync_type)
        {
            rc = hsrv_ptp_sync_systime();
            p_db_time->sync_count++;
            p_db_time->last_sync_time = ctc_time_boottime_sec(NULL);
        }
        break;

    default:
        break;
    }

    return rc;
}

int32
_hsrv_time_cmd_set_sync_type(uint32 type, cfg_cmd_para_t *para)
{
    tbl_fea_time_t *p_db_time = tbl_fea_time_get_fea_time();

    if (p_db_time->sync_type == type)
    {
        return PM_E_NONE;
    }

    if(GLB_TIME_SYNC_SYSTIME == type)
    {
        hsrv_ptp_sync_systime();
        p_db_time->sync_count++;
        p_db_time->last_sync_time = ctc_time_boottime_sec(NULL);
    }
    
    p_db_time->sync_type = type;
    tbl_fea_time_set_fea_time_field(p_db_time, TBL_FEA_TIME_FLD_SYNC_TYPE);
    
    return PM_E_NONE;
}

static int32
_hsrv_ptp_cmd_get_fea_time(uint32 is_show, cfg_cmd_para_t *para, cdb_node_t* p_node)
{
    tbl_fea_time_t *p_db_time = tbl_fea_time_get_fea_time();
    tbl_iter_args_t args;
    FILE *fp = NULL;
    sal_systime_t current_tv;
    sal_time_t tp;

    field_parse_t field;
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &field;
    args.argv[1] = fp;

    if (is_show)
    {
        sal_gettime(&current_tv);
        tp = current_tv.tv_sec + p_db_time->last_sync_time - ctc_time_boottime_sec(NULL);
        if (GLB_TIME_SYNC_NONE == p_db_time->sync_type)
        {
            sal_fprintf(fp, "%-16s : %s\n", "Sync Type", "Disabled");
        }
        else if (GLB_TIME_SYNC_SYSTIME == p_db_time->sync_type)
        {
            sal_fprintf(fp, "%-16s : %s\n", "Sync Type", "System Time");
        }
        else if (GLB_TIME_SYNC_PTP == p_db_time->sync_type)
        {
            sal_fprintf(fp, "%-16s : %s\n", "Sync Type", "PTP");
        }
        sal_fprintf(fp, "%-16s : %u\n", "Sync Count", p_db_time->sync_count);
        sal_fprintf(fp, "%-16s : %s\n", "Last Sync Time", sal_ctime(&tp));        
    }
    else
    {
        tbl_fea_time_dump_one(p_db_time, &args);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_time_cmd_process_fea_time(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 is_show = FALSE;
    int32 value = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_FEA_TIME);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_FEA_TIME_FLD_SYNC_TYPE:
            value = cdb_enum_str2val(glb_time_sync_type_strs, GLB_TIME_SYNC_TYPE_MAX, argv[1]);
            if (value < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid sync type %s", argv[1]);
            }
            rc = _hsrv_time_cmd_set_sync_type(value, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (argc == 1)
        {
            is_show = TRUE;
        }
        rc = _hsrv_ptp_cmd_get_fea_time(is_show, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_time_start()
{
    cdb_register_cfg_act_cb(ACT_SYNC_SYSTIME, hsrv_time_cmd_process_sync_systime);
    cdb_register_cfg_tbl_cb(TBL_FEA_TIME, hsrv_time_cmd_process_fea_time);

    return HSRV_E_NONE;
}


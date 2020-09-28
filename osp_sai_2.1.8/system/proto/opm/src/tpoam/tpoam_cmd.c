#include "proto.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"

#include "lib_fs.h"

#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_api.h"
#include "tpoam_debug.h"
#include "tpoam_error.h"
#include "tpoam_event.h"
#include "tpoam_lm.h"
#include "tpoam_dm.h"
#include "tpoam_lb.h"

#ifndef tpoam_cfg
static int32
_tpoam_cmd_set_fast_aps_enable(uint32 fast_aps_en, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (fast_aps_en == p_db_tpoam_glb->fast_aps_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_fast_aps_enable(fast_aps_en));

    return rc;
}

static int32
_tpoam_cmd_set_dm_global_enable(uint32 enable, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (enable == p_db_tpoam_glb->dm_enable_global)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_dm_global_enable(enable));

    return rc;
}

static int32
_tpoam_cmd_set_lm_global_enable(uint32 enable, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (enable == p_db_tpoam_glb->lm_enable_global)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_lm_global_enable(enable));

    return rc;
}

static int32
_tpoam_cmd_set_dm_calc_interval(uint32 interval, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (interval == p_db_tpoam_glb->dm_calc_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_dm_calc_interval(interval));

    return rc;
}

static int32
_tpoam_cmd_set_dm_delay_threshold(uint32 threshold, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (threshold == p_db_tpoam_glb->dm_delay_threshold)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_dm_delay_threshold(threshold));

    return rc;
}

static int32
_tpoam_cmd_set_lm_local_threshold(uint32 threshold, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (threshold == p_db_tpoam_glb->lm_local_threshold)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_lm_local_threshold(threshold));

    return rc;
}

static int32
_tpoam_cmd_set_lm_remote_threshold(uint32 threshold, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (threshold == p_db_tpoam_glb->lm_remote_threshold)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_lm_remote_threshold(threshold));

    return rc;
}


static int32
_tpoam_cmd_set_lm_calc_interval(uint32 interval, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (interval == p_db_tpoam_glb->lm_calc_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_lm_calc_interval(interval));

    return rc;
}

static int32
_tpoam_cmd_set_node_id(uint32 node_id, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (node_id == p_db_tpoam_glb->node_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_node_id(node_id));

    return rc;
}

static int32
_tpoam_cmd_set_channel_type(uint16 channel_type, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if ((0 == channel_type)
        || (GLB_MPLS_OAM_ACH_CHAN_TYPE_MCC == channel_type)
        || (GLB_MPLS_OAM_ACH_CHAN_TYPE_SCC == channel_type))
    {
        return TPOAM_E_INVALID_CHAN_TYPE;
    }

    /*channel type show set before oam session creat*/
    if (p_db_tpoam_glb->resouce_count_curr.megid_cnt)
    {
        return TPOAM_E_SET_WITH_NO_MEG;
    }
    
    /* 2. check duplicate config */
    if (channel_type == p_db_tpoam_glb->channel_type)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_channel_type(channel_type));

    return rc;
}


static int32
_tpoam_get_rdi_reason_value(uint32 argc, char** argv, uint32 para_index_base, 
                                uint8* p_rdi_reason)
{
    uint32 arg_index = 0;

    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "loc", 3);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_rdi_reason, GLB_TPOAM_RDI_RSN_LOC);
    }    
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "cc-e", 4);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_rdi_reason, GLB_TPOAM_RDI_RSN_CC_ERR);
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "cc-m", 4);
    if (arg_index)
    {
        GLB_SET_FLAG(*p_rdi_reason, GLB_TPOAM_RDI_RSN_CC_MISMERGE);
    }
    return PM_E_NONE;
}

static int32
_tpoam_cmd_set_rdi_reason(char **argv, int32 argc, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint8 rdi_reason = 0;
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();

    /* 1. config check */
    if (1 == argc)/*no oper*/
    {
        rdi_reason = TPOAM_RDI_RSN_DEF;
    }
    else
    {
        if (!sal_strncmp(argv[0], "all", 3))
        {
            rdi_reason = TPOAM_RDI_RSN_ALL;
        }
        else if (!sal_strncmp(argv[0], "non", 3))
        {
            rdi_reason = TPOAM_RDI_RSN_NONE;
        }
        else
        {
            _tpoam_get_rdi_reason_value(argc, argv, 0, &rdi_reason);
        }
    }
    
    /* 2. check duplicate config */
    if (rdi_reason == p_db_tpoam_glb->rdi_reason)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_rdi_reason(rdi_reason, TRUE));

    if (!p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }
    else
    {
        return TPOAM_E_RDI_REASON_SET;
    }
    
    return rc;
}

static int32
_tpoam_cmd_set_sf_reason(char **argv, int32 argc, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    uint32 sf_reason = 0;
    int32 rc = PM_E_NONE;
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();

    /* 1. config check */
    if (1 == argc)/*no oper*/
    {
        sf_reason = TPOAM_SF_RSN_DEF;
    }
    else
    {
        if (!sal_strncmp(argv[0], "all", 3))
        {
            sf_reason = TPOAM_SF_RSN_ALL;
        }
        else if (!sal_strncmp(argv[0], "non", 3))
        {
            sf_reason = TPOAM_SF_RSN_NONE;
        }
        else
        {
            _tpoam_get_event_rule_value(argc, argv, 0, &sf_reason);
        }
    }
    
    /* 2. check duplicate config */
    if (sf_reason == p_db_tpoam_glb->sf_reason_mask)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_sf_reason(sf_reason));

    if (!p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }
    else
    {
        return TPOAM_E_SF_REASON_SET;
    }
    
    return rc;
}

static int32
_tpoam_cmd_set_sd_reason(char **argv, int32 argc, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    uint32 sd_reason = 0;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (1 == argc)/*no oper*/
    {
        sd_reason = TPOAM_SD_RSN_DEF;
    }
    else
    {
        if (!sal_strncmp(argv[0], "all", 3))
        {
            sd_reason = TPOAM_SD_RSN_ALL;
        }
        else if (!sal_strncmp(argv[0], "non", 3))
        {
            sd_reason = TPOAM_SD_RSN_NONE;
        }
        else
        {
            _tpoam_get_event_rule_value(argc, argv, 0, &sd_reason);
        }
    }
    
    /* 2. check duplicate config */
    if (sd_reason == p_db_tpoam_glb->sd_reason_mask)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_sd_reason(sd_reason));

    if (!p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }
    else
    {
        return TPOAM_E_SD_REASON_SET;
    }
    
    return rc;
}


static int32
_tpoam_cmd_set_event_rule_map(char **argv, int32 argc, tbl_tpoam_global_t* p_db_tpoam_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tpoam_evt_dest_t dest = TPOAM_EVT_DEST_MAX;
    uint32 event_rule = 0;
    uint32 exclude_event_rule = 0;

    /*parse para*/
    /*default is no oper, argc is 2*/
    if (!sal_strncmp(argv[1], "log", 3))
    {
        dest = TPOAM_EVT_DEST_LOG;
        event_rule = TPOAM_EVT_LOG_DEF;
    }
    else if (!sal_strncmp(argv[1], "trap", 4))
    {
        dest = TPOAM_EVT_DEST_TRAP;
        event_rule = TPOAM_EVT_TRAP_DEF;
    }
    else if (!sal_strncmp(argv[1], "cache", 5))
    {
        dest = TPOAM_EVT_DEST_CACHE;
        event_rule = TPOAM_EVT_CACHE_DEF;
    }
    else
    {
        return PM_E_FAIL;
    }
    
    /*set oper*/
    if (2 != argc)
    {
        if (!sal_strncmp(argv[2], "all", 3))
        {
            event_rule = TPOAM_EVT_ALL;
        }
        else if (!sal_strncmp(argv[2], "none", 4))
        {
            event_rule = TPOAM_EVT_NONE;
        }
        else if (!sal_strncmp(argv[2], "exclude", 7))
        {
            _tpoam_get_event_rule_value(argc, argv, 3, &exclude_event_rule);
            event_rule = (~exclude_event_rule) & TPOAM_EVT_ALL;
        }    
        else
        {
            _tpoam_get_event_rule_value(argc, argv, 2, &event_rule);
        }
    }
    
    /* 1. config check */
    if (dest >= TPOAM_EVT_DEST_MAX)
    {
        return PM_E_FAIL;
    }
    
    /* 2. check duplicate config */
    if (event_rule == p_db_tpoam_glb->event_rule_map[dest])
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(tpoam_api_set_event_rule_map(event_rule, dest));

    return rc;
}


static int32
_tpoam_global_cmd_get_tpoam_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_tpoam_global_t* p_db_tpoam_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_tpoam_global_dump_one(p_db_tpoam_glb, &args);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}


int32
tpoam_cmd_process_tpoam_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_tpoam_global_t* p_tpoam_glb = NULL;
    cdb_node_t* p_node = NULL;
    uint32 value = 0;
    //int32 signed_value = 0;
    int32 rc = PM_E_NONE;
    addr_ipv4_t node_id = {0};

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_TPOAM_GLOBAL);
    p_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
          //  CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        else
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }
        
        switch (field_id)
        {
        case TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN:/*ok*//*default is disable*/
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_fast_aps_enable(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_dm_global_enable(value, p_tpoam_glb, para);
            break;
         case TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL:
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_dm_calc_interval(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD:
            value = cdb_uint_str2val(argv[1], &rc);
            if ((rc != 0) || (value > TPOAM_SESSION_DM_DELAY_THRESHOLD_MAX))
            {
                CFG_CONFLICT_RET("Syntax error: Illegal parameter" );
            }
            rc = _tpoam_cmd_set_dm_delay_threshold(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_lm_global_enable(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD:
            rc = cdb_percent_point_str2value(argv[1], &value);
            if (rc != 0)
            {
                CFG_INVALID_PARAM_RET("Syntax error: Illegal parameter");
            }
            rc = _tpoam_cmd_set_lm_local_threshold(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD:
            rc = cdb_percent_point_str2value(argv[1], &value);
            if (rc != 0)
            {
                CFG_INVALID_PARAM_RET("Syntax error: Illegal parameter");
            }
            rc = _tpoam_cmd_set_lm_remote_threshold(value, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL:/*lm_calc_interval/${interval}*/
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_lm_calc_interval(value, p_tpoam_glb, para);
            break;
            
        case TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE:/*ok*//*default is 0x8902*/
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT_RANGE(value, 0, 0xffff, argv[1], para->p_rs);
            rc = _tpoam_cmd_set_channel_type(value, p_tpoam_glb, para);
            break;

        case TBL_TPOAM_GLOBAL_FLD_NODE_ID:/*ok*//*default is 0*/
            //PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (2 == argc)
            {
                CFG_PARSER_IP_ADDR(argv[1], &node_id, para->p_rs);
                value = node_id.s_addr;
            }
            else if (1 == argc)
            {
                value = 0;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
            }
            rc = _tpoam_cmd_set_node_id(value, p_tpoam_glb, para);
            break;

        /*rdi_reason/all*/
        /*rdi_reason/none*/
        /*rdi_reason/${cc-error}/${cc-mismerge}/${loc}*/
        /*rdi_reason*/
        case TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK:
            rc = _tpoam_cmd_set_rdi_reason(argv, argc, p_tpoam_glb, para);
            break;

        /*sf_reason_mask/all*/
        /*sf_reason_mask/none*/
        /*sf_reason_mask/${ais-defect}/${ais-rx}/${csf-defect}/${loc}/${rdi-rx}*/
        /*sf_reason_mask*/
        case TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK:
            rc = _tpoam_cmd_set_sf_reason(argv, argc, p_tpoam_glb, para);
            break;

        /*sd_reason_mask/all*/
        /*sd_reason_mask/none*/
        /*sd_reason_mask/${ais-defect}/${ais-rx}/${crc}*/
        /*sd_reason_mask*/
        case TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK:
            rc = _tpoam_cmd_set_sd_reason(argv, argc, p_tpoam_glb, para);
            break;
        case TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP:/*ok*/
            rc = _tpoam_cmd_set_event_rule_map(argv, argc, p_tpoam_glb, para);
            break;
        default:
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _tpoam_global_cmd_get_tpoam_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    TPOAM_CLI_ERROR_RETURN(rc);
    return PM_E_NONE;
}
#endif

#ifndef tpoam_show
void
_tpoam_cmd_show_glb(cfg_cmd_para_t *para)
{
    addr_ipv4_t node_id = {0};
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    tbl_openflow_t*   p_glb_openflow = tbl_openflow_get_openflow();;
    //uint32 i = 0;
    uint8 is_first = 0;
    uint8 flag_count = 0;
    char show_str[512];
    char buf[CMD_BUF_32];

    FILE *fp = NULL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }
    
    /* config */
    sal_fprintf(fp, "%-27s : %s\n", "Fast APS", tpoam_api_en_desc(FALSE, pst_master->fast_aps_en));
    sal_fprintf(fp, "%-27s : %u(DMR packets)\n","DM upload interval", pst_master->dm_calc_interval);
    sal_fprintf(fp, "%-27s : %u(ns)\n", "Global DM threshold", pst_master->dm_delay_threshold);
    sal_fprintf(fp, "%-27s : %u(s)\n", "LM upload interval", pst_master->lm_calc_interval);
    sal_fprintf(fp, "%-27s : %.3f(%%)\n", "Global LM local threshold", (float)pst_master->lm_local_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    sal_fprintf(fp, "%-27s : %.3f(%%)\n", "Global LM remote threshold", (float)pst_master->lm_remote_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    node_id.s_addr = pst_master->node_id;
    sal_fprintf(fp, "%-27s : %-15s\n", "Node ID", cdb_addr_ipv4_val2str(buf, CMD_BUF_32, &node_id));
    sal_fprintf(fp, "%-27s : 0x%04X\n", "Channel Type", pst_master->channel_type);
    
    if (p_glb_openflow && p_glb_openflow->pw_not_standard_mode == TRUE)
    {
        sal_fprintf(fp, "%-27s : No (without control word)\n", "Standard PW mode");
    }
    else if (p_glb_openflow && p_glb_openflow->pw_not_standard_mode == FALSE)
    {
        sal_fprintf(fp, "%-27s : Yes (with control word)\n", "Standard PW mode");
    }
    
#ifdef support_rdi
    /* RDI */
    sal_memset(show_str, 0, sizeof(show_str));
    if (GLB_TPOAM_RDI_RSN_NONE == pst_master->rdi_reason)
    {
        sal_strcat(show_str, tpoam_api_rdi_reason_desc(GLB_TPOAM_RDI_RSN_NONE));
    }
    else
    {
        is_first = TRUE;
        if (GLB_FLAG_ISSET(pst_master->rdi_reason, GLB_TPOAM_RDI_RSN_LOC))
        {
            if (!is_first)
            {
                sal_strcat(show_str, ", ");
            }
            is_first = FALSE;
            sal_strcat(show_str, tpoam_api_rdi_reason_desc(GLB_TPOAM_RDI_RSN_LOC));
        }
        if (GLB_FLAG_ISSET(pst_master->rdi_reason, GLB_TPOAM_RDI_RSN_CC_ERR))
        {
            if (!is_first)
            {
                sal_strcat(show_str, ", ");
            }
            is_first = FALSE;
            sal_strcat(show_str, tpoam_api_rdi_reason_desc(GLB_TPOAM_RDI_RSN_CC_ERR));
        }
        if (GLB_FLAG_ISSET(pst_master->rdi_reason, GLB_TPOAM_RDI_RSN_CC_MISMERGE))
        {
            if (!is_first)
            {
                sal_strcat(show_str, ", ");
            }
            is_first = FALSE;
            sal_strcat(show_str, tpoam_api_rdi_reason_desc(GLB_TPOAM_RDI_RSN_CC_MISMERGE));
        }
    }
    sal_fprintf(fp, "RDI Reason    :   %s\n", show_str);
#endif

    /* SF */
    sal_memset(show_str, 0, sizeof(show_str));
    if (GLB_TPOAM_EVT_NONE == pst_master->sf_reason_mask)
    {
        sal_strcat(show_str, tpoam_api_event_desc(GLB_TPOAM_EVT_NONE));
    }
    else
    {
        is_first = TRUE;
        flag_count = 0;
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sf_reason_mask, GLB_TPOAM_EVT_LOC);
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sf_reason_mask, GLB_TPOAM_EVT_RDI_RX);
        #ifdef support_many_event
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sf_reason_mask, GLB_TPOAM_EVT_AIS_RX);
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sf_reason_mask, GLB_TPOAM_EVT_AIS_DEFECT);
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sf_reason_mask, GLB_TPOAM_EVT_CSF_DEFECT);
        #endif
    }
    //sal_fprintf(fp, "SF Reason     :   %s\n", show_str);

    #ifdef support_sd
    /* modified by yaom for CRC trigger AIS 20140919 */
    /* SD */
    sal_memset(show_str, 0, sizeof(show_str));
    if (GLB_TPOAM_EVT_NONE == pst_master->sd_reason_mask)
    {
        sal_strcat(show_str, tpoam_api_event_desc(GLB_TPOAM_EVT_NONE));
    }
    else
    {
        is_first = TRUE;
        flag_count = 0;
        #ifdef support_many_event
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sd_reason_mask, GLB_TPOAM_EVT_AIS_RX);
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sd_reason_mask, GLB_TPOAM_EVT_AIS_DEFECT);
        #endif
        TPOAM_API_SHOW_EVT_BITMAP(pst_master->sd_reason_mask, GLB_TPOAM_EVT_CRC_DEFECT);
    }
    sal_fprintf(fp, "SD Reason     :   %s\n", show_str);
    #endif
    
    #ifdef support_event
    /* Event */
    sal_fprintf(fp, "\nEvent Switch\n");
    sal_fprintf(fp, "%-17s %-10s %-10s %-10s\n", "Event", "Log", "Trap", "Cache");
    sal_fprintf(fp, "------------------------------------------------\n");

    for (i = 0; i < GLB_TPOAM_EVT_NUM; i++)
    {
        sal_fprintf(fp, "%-17s %-10s %-10s %-10s\n", 
            tpoam_api_event_desc((1 << i)), 
            tpoam_api_status_desc(GLB_IS_BIT_SET(pst_master->event_rule_map[TPOAM_EVT_DEST_LOG], i)),
            tpoam_api_status_desc(GLB_IS_BIT_SET(pst_master->event_rule_map[TPOAM_EVT_DEST_TRAP], i)),
            tpoam_api_status_desc(GLB_IS_BIT_SET(pst_master->event_rule_map[TPOAM_EVT_DEST_CACHE], i))
            );
    }  
    #endif

    #ifdef support_resource_count
    /* Count */
    sal_fprintf(fp, "\nResource Count\n");
    sal_fprintf(fp, "%-17s %-10s %-10s\n", "Resouce", "Current", "Max");
    sal_fprintf(fp, "------------------------------------------------\n");
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "Section", pst_master->resouce_count_curr.section_cnt, pst_master->resouce_count_max.section_cnt);
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "MEG", pst_master->resouce_count_curr.megid_cnt, pst_master->resouce_count_max.megid_cnt);
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "Local MEP", pst_master->resouce_count_curr.lmep_cnt, pst_master->resouce_count_max.lmep_cnt);
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "Remote MEP", pst_master->resouce_count_curr.rmep_cnt, pst_master->resouce_count_max.rmep_cnt);    
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "MIP", pst_master->resouce_count_curr.mip_cnt, pst_master->resouce_count_max.mip_cnt);
    sal_fprintf(fp, "%-17s %-10d %-10d\n", "CSF", pst_master->resouce_count_curr.csf_cnt, pst_master->resouce_count_max.csf_cnt);
    #endif
    /* Stats */
    _tpoam_api_show_stats(fp, &pst_master->oam_glb_stats, TRUE, 0, TRUE);

    sal_fclose(fp);
    fp = NULL;
    
    return;
}


/*/mp/${service_type}/${part}*/
int32
_tpoam_cmd_show_mp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    //tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    char* pst_service_name = NULL;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "all"))
    {
        rc = tpoam_api_show_mp_section(fp, pst_service_name);
        rc = tpoam_api_show_mp_lsp(fp, pst_service_name);
        rc = tpoam_api_show_mp_pw_tpe(fp, pst_service_name);
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        if (argc >= 3 && 0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
        //rc = tpoam_api_show_mp_lsp(fp, pst_service_name);
        tpoam_api_show_mp_service_session(fp, GLB_OAM_TYPE_MPLS_LSP);
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        if (argc >= 3 && 0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
        //rc = tpoam_api_show_mp_section(fp, pst_service_name);
        tpoam_api_show_mp_service_session(fp, GLB_OAM_TYPE_MPLS_SECTION);
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        if (argc >= 3 && 0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
        //rc = tpoam_api_show_mp_pw_tpe(fp, pst_service_name);
        tpoam_api_show_mp_service_session(fp, GLB_OAM_TYPE_MPLS_PW);
    }
    else
    {
        return PM_E_FAIL;
    }
        
    /* config */
    //sal_fprintf(fp, "Channel Type  :   0x%04X\n", pst_master->channel_type);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/mp_session/${session_id}*/
int32
_tpoam_cmd_show_mp_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    //tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    uint32 session_id = 0;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "all"))
    {
        rc = tpoam_api_show_mp_session(fp, session_id, para);
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
        rc = tpoam_api_show_mp_session(fp, session_id, para);
    }
    
    /* config */
    //sal_fprintf(fp, "Channel Type  :   0x%04X\n", pst_master->channel_type);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/mp_status/${service_type}*/
int32
_tpoam_cmd_show_mp_status(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 type = 0;
    tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "all"))
    {
        type = 0;
        pw_type = TPOAM_PW_TYPE_MAX;
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        type = GLB_OAM_TYPE_MPLS_LSP;
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        type = GLB_OAM_TYPE_MPLS_SECTION;
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        type = GLB_OAM_TYPE_MPLS_PW;
        pw_type = TPOAM_PW_TYPE_TPE;
    }
    else
    {
        return PM_E_FAIL;
    }

    rc = tpoam_api_show_mp_status(fp, type, pw_type);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/statistics_session/${session_id}*/
int32
_tpoam_cmd_show_statistics_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 session_id = 0;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "all"))
    {
        rc = tpoam_api_show_statistics_session(fp, session_id, para);
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
        rc = tpoam_api_show_statistics_session(fp, session_id, para);
    }

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/mp_status_session/${session_id}*/
int32
_tpoam_cmd_show_mp_status_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 session_id = 0;
    
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "all"))
    {
        rc = tpoam_api_show_mp_status_session(fp, session_id, para);
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
        rc = tpoam_api_show_mp_status_session(fp, session_id, para);
    }

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


void
_tpoam_cmd_show_section(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();

    FILE *fp = NULL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }
    
    /* config */
    sal_fprintf(fp, "Channel Type  :   0x%04X\n", pst_master->channel_type);

    sal_fclose(fp);
    fp = NULL;
    
    return;
}

/*/event/${service_type}/${part}*/
int32
_tpoam_cmd_show_event(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char* pst_service_name = NULL;
    FILE *fp = NULL;
    uint32 type = 0;
    tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
        type = 0;
        pw_type = TPOAM_PW_TYPE_MAX;
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        type = GLB_OAM_TYPE_MPLS_LSP;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        type = GLB_OAM_TYPE_MPLS_SECTION;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        type = GLB_OAM_TYPE_MPLS_PW;
        pw_type = TPOAM_PW_TYPE_TPE;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else
    {
        return PM_E_FAIL;
    }

    rc = tpoam_api_show_event(fp, type, pw_type, pst_service_name);
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/session/${session_id}*/
int32
_tpoam_cmd_show_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    uint32 session_id = 0;
    uint32 oam_type = 0;
    uint32 mp_type = TPOAM_MP_TYPE_MAX;
    uint32 show_session_detail = FALSE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
        oam_type = GLB_OAM_TYPE_MPLS_ALL;
    }
    else if (0 == sal_strcmp(argv[1], "lsp_pe"))
    {
        oam_type = GLB_OAM_TYPE_MPLS_LSP;
        mp_type = TPOAM_MP_TYPE_MEP;
    }
    else if (0 == sal_strcmp(argv[1], "lsp_p"))
    {
        oam_type = GLB_OAM_TYPE_MPLS_LSP;
        mp_type = TPOAM_MP_TYPE_MIP;
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        oam_type = GLB_OAM_TYPE_MPLS_PW;
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        oam_type = GLB_OAM_TYPE_MPLS_SECTION;
    }
    else if (0 == sal_strcmp(argv[1], "unbind"))
    {
        oam_type = 0;
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
        if ((argc == 3) && (0 == sal_strcmp(argv[2], "detail")))
        {
            show_session_detail = TRUE;
        }
    }

    if (show_session_detail == FALSE)
    {
        rc = tpoam_api_show_session(fp, session_id, oam_type, mp_type, para);
    }
    else
    {
        rc = tpoam_api_show_session_detail(fp, session_id, para);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/binding_flow_session/${session_id}*/
int32
_tpoam_cmd_show_binding_flow_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    uint32 session_id = 0;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
    }
    
    rc = tpoam_api_show_binding_flow_session(fp, session_id, para);
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/event_session/${session_id}*/
int32
_tpoam_cmd_show_event_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    uint32 session_id = 0;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
    }
    

    rc = tpoam_api_show_event_session(fp, session_id);
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/dmm_session/${session_id}*/
int32
_tpoam_cmd_show_dmm_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    uint32 session_id = 0;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
    }
    

    rc = tpoam_api_show_dmm_session(fp, session_id, para);
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/1dm/${service_type}/${part}*/
int32
_tpoam_cmd_show_1dm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 type = 0;
    tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    char* pst_service_name = NULL;
    
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
        type = 0;
        pw_type = TPOAM_PW_TYPE_MAX;
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        type = GLB_OAM_TYPE_MPLS_LSP;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        type = GLB_OAM_TYPE_MPLS_SECTION;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        type = GLB_OAM_TYPE_MPLS_PW;
        pw_type = TPOAM_PW_TYPE_TPE;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else
    {
        return PM_E_FAIL;
    }

    rc = tpoam_api_show_1dm(fp, type, pw_type, pst_service_name);    
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/lm/${service_type}/${part}/${exp_value}*/
int32
_tpoam_cmd_show_lm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 type = 0;
    tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    char* pst_service_name = NULL;
    tpoam_lm_dual_enable_input_t lm_dual_inputs;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&lm_dual_inputs, 0, sizeof(tpoam_lm_dual_enable_input_t));
    lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_ALL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
        type = 0;
        pw_type = TPOAM_PW_TYPE_MAX;
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        type = GLB_OAM_TYPE_MPLS_LSP;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }

        if (4 == argc && 0 != sal_strlen(argv[3]))
        {
            lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_SPECIFIED_COS;
            PM_CFG_GET_UINT(lm_dual_inputs.cos_value, argv[3], para->p_rs);
        }
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        type = GLB_OAM_TYPE_MPLS_SECTION;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }

        if (4 == argc && 0 != sal_strlen(argv[3]))
        {
            lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_SPECIFIED_COS;
            PM_CFG_GET_UINT(lm_dual_inputs.cos_value, argv[3], para->p_rs);
        }
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        type = GLB_OAM_TYPE_MPLS_PW;
        pw_type = TPOAM_PW_TYPE_TPE;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
        else
        {
            //pw_type = TPOAM_PW_TYPE_MAX;
        }

        if (4 == argc && 0 != sal_strlen(argv[3]))
        {
            lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_SPECIFIED_COS;
            PM_CFG_GET_UINT(lm_dual_inputs.cos_value, argv[3], para->p_rs);
        }
    }
    else
    {
        return PM_E_FAIL;
    }

    rc = tpoam_api_show_lm_dual(fp, type, pw_type, pst_service_name, &lm_dual_inputs); 
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/lm_session/${session_id}/${exp_value}*/
int32
_tpoam_cmd_show_lm_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tpoam_lm_dual_enable_input_t lm_dual_inputs;
    uint32 session_id = 0;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&lm_dual_inputs, 0, sizeof(tpoam_lm_dual_enable_input_t));
    lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_ALL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
    
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);

        if (3 == argc && 0 != sal_strlen(argv[2]))
        {
            lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_SPECIFIED_COS;
            PM_CFG_GET_UINT(lm_dual_inputs.cos_value, argv[2], para->p_rs);
        }
    }
    
    rc = tpoam_api_show_lm_dual_session(fp, session_id, &lm_dual_inputs, para); 
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

/*/lm_upload_session/${session_id}/${exp_value}*/
int32
_tpoam_cmd_show_lm_upload_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tpoam_lm_dual_enable_input_t lm_dual_inputs;
    uint32 session_id = 0;

    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&lm_dual_inputs, 0, sizeof(tpoam_lm_dual_enable_input_t));
    lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_ALL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
    
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);

        if (3 == argc && 0 != sal_strlen(argv[2]))
        {
            lm_dual_inputs.cos_type = TPOAM_LM_COS_TYPE_SPECIFIED_COS;
            PM_CFG_GET_UINT(lm_dual_inputs.cos_value, argv[2], para->p_rs);
        }
    }
    
    rc = tpoam_api_show_lm_dual_upload_session(fp, session_id, &lm_dual_inputs, para); 
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


/*/single_lm/${service_type}/${part}*/
int32
_tpoam_cmd_show_single_lm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    //tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    //uint32 type = 0;
    //tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    char* pst_service_name = NULL;

    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t* pst_lsp_pe = NULL;
    tbl_lsp_p_t* pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    int32 rc = TPOAM_E_NONE;

    tbl_oam_session_t *p_db_oam_session = NULL;
    
    FILE *fp = NULL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    pst_service_name = argv[2];
    if (0 == sal_strcmp(argv[1], "lsp"))
    {
        rc = tpoam_api_get_lsp_pe(pst_service_name, &pst_lsp_pe);
        if (rc < 0)
        {
            rc = tpoam_api_get_lsp_p(pst_service_name, &pst_lsp_p);
            if (rc < 0)
            {
                return rc;
            }
            else
            {
                if (!pst_lsp_p->oam_session_id)
                {
                    return TPOAM_E_SERVICE_NON_OAM;
                }
                
                p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
            }
        }
        else
        {
            if (!pst_lsp_pe->oam_session_id)
            {
                return TPOAM_E_SERVICE_NON_OAM;
            }
            
            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
        }
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        rc = tpoam_api_get_section(pst_service_name, &pst_section);
        if (rc < 0)
        {
            return rc;
        }
        if (!pst_section->oam_session_id)
        {
            return TPOAM_E_SERVICE_NON_OAM;
        }

        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        rc = tpoam_api_get_pw_tpe(pst_service_name, &pst_tpe);
        if (rc < 0)
        {
            return rc;
        }
        if (!pst_tpe->oam_session_id)
        {
            return TPOAM_E_SERVICE_NON_OAM;
        }

        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
    }
    else
    {
        return PM_E_FAIL;
    }

    if (TPOAM_LM_TYPE_SINGLE != p_db_oam_session->oam_lm.lm_type)
    {
        return TPOAM_E_LM_SINGLE_NOT_ENABLED;
    }

    if (!p_db_oam_session->oam_lm.lm_single.is_proactive)
    {
        return TPOAM_E_LM_SINGLE_NOT_PROACTIVE;
    }

    tpoam_api_show_oam_lm_single(fp, p_db_oam_session, &p_db_oam_session->oam_lm.lm_single);

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

void
_tpoam_cmd_exec_show_lb(cfg_cmd_para_t *para)
{
    addr_ipv4_t node_id = {0};
    char buf[CMD_BUF_32];
    uint32 time_out = 0;
    uint32 repeat_cnt = 0;
    uint32 exp = 0;

    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    FILE *fp = NULL;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return ;
    }

    if (0 == p_tpoam_exec_global->oam_session_id)
    {
        sal_fprintf(fp, "No loopback test is running!\n");
        
        sal_fclose(fp);
        fp = NULL;
        
        return;
    }

    sal_fprintf(fp, "%17s %u\n", "Session ID      :", p_tpoam_exec_global->oam_session_id);
    sal_fprintf(fp, "%17s %s\n", "LB Type         :", tpoam_api_lb_testtype_desc(p_tpoam_exec_global->test_type));
    
    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(p_tpoam_exec_global->oam_session_id);
    if (p_db_oam_session)
    {
        sal_fprintf(fp, "%17s %s\n", "MEGID           :", p_db_oam_session->megid_str);
        sal_fprintf(fp, "%17s %d\n", "MEPID           :", p_db_oam_session->mepid);
        sal_fprintf(fp, "%17s %s\n", "Session State   :", tpoam_api_signal_status_desc(p_db_oam_session->signal_status));
    }

    if (TPOAM_EXEC_TYPE_LB_MIP == p_tpoam_exec_global->test_type)
    {
        sal_fprintf(fp, "TTL             : %d\n", p_tpoam_exec_global->exec_lb_mip.para.ttl);
        time_out = p_tpoam_exec_global->exec_lb_mip.para.time_out;
        repeat_cnt = p_tpoam_exec_global->exec_lb_mip.para.repeat_cnt;
        exp = p_tpoam_exec_global->exec_lb_mip.para.exp;
        node_id.s_addr = p_tpoam_exec_global->exec_lb_mip.para.node_id;
        sal_fprintf(fp, "%-16s: %-15s\n", "Node ID", cdb_addr_ipv4_val2str(buf, CMD_BUF_32, &node_id));
        sal_fprintf(fp, "Timeout         : %d\n", time_out);
        sal_fprintf(fp, "Repeat Count    : %d/%d\n", p_db_oam_session ? p_db_oam_session->oam_lb.lbm_tx_count : 0, repeat_cnt);
        sal_fprintf(fp, "EXP             : %d\n", exp);
    }
    else if (TPOAM_EXEC_TYPE_LB_DISCOVERY == p_tpoam_exec_global->test_type)
    {
        sal_fprintf(fp, "TTL             : [%d-%d]\n", p_tpoam_exec_global->exec_lb_discovery.para.ttl, p_tpoam_exec_global->exec_lb_discovery.para.ttl_end);
        time_out = p_tpoam_exec_global->exec_lb_discovery.para.time_out;
        repeat_cnt = p_tpoam_exec_global->exec_lb_discovery.para.repeat_cnt;
        exp = p_tpoam_exec_global->exec_lb_discovery.para.exp;
        sal_fprintf(fp, "Timeout         : %d\n", time_out);
        //sal_fprintf(fp, "Repeat Count    : %d\n", repeat_cnt);
        sal_fprintf(fp, "EXP             : %d\n", exp);
    }
    else if (TPOAM_EXEC_TYPE_LB_MEP == p_tpoam_exec_global->test_type)
    {
        sal_fprintf(fp, "Remote MEP      : %d\n", p_tpoam_exec_global->exec_lb_rmep.para.rmep_id);
        time_out = p_tpoam_exec_global->exec_lb_rmep.para.time_out;
        repeat_cnt = p_tpoam_exec_global->exec_lb_rmep.para.repeat_cnt;
        exp = p_tpoam_exec_global->exec_lb_rmep.para.exp;
        sal_fprintf(fp, "Timeout         : %d\n", time_out);
        sal_fprintf(fp, "Repeat Count    : %d/%d\n", p_db_oam_session ? p_db_oam_session->oam_lb.lbm_tx_count : 0, repeat_cnt);
        sal_fprintf(fp, "EXP             : %d\n", exp);
    }
    else
    {
        
    }

    sal_fclose(fp);
    fp = NULL;
    
    return;
}

int32
tpoam_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == sal_strcmp(argv[0], "global"))/* show mpls-tp oam-y1731 *//*/global*/
        {
            _tpoam_cmd_show_glb(para);
        }
        else if (0 == sal_strcmp(argv[0], "statistics_session"))/* show mpls-tp oam-y1731 statistics*//*/statistics_session/${session_id}*/
        {
            rc = _tpoam_cmd_show_statistics_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "mp"))/* show mpls-tp oam-y1731 mp *//*/mp/${service_type}/${part}*/
        {
            rc = _tpoam_cmd_show_mp(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "mp_session"))/* show mpls-tp oam-y1731 mp session*//*/mp_session/${session_id}*/
        {
            rc = _tpoam_cmd_show_mp_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "session"))/* show mpls-tp oam-y1731 session*//*/session/${session_id}*/
        {
            rc = _tpoam_cmd_show_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "binding_flow_session"))/* show mpls-tp oam-y1731 session*//*/session/${session_id}*/
        {
            rc = _tpoam_cmd_show_binding_flow_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "mp_status"))/*show mpls-tp oam-y1731 mp status*//*/mp_status/${service_type}*/
        {
            rc = _tpoam_cmd_show_mp_status(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "mp_status_session"))/*show mpls-tp oam-y1731 mp status session*//*/mp_status_session*/
        {
            rc = _tpoam_cmd_show_mp_status_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "section"))/* show mpls-tp oam-y1731 section *//*/section/${part}*/
        {
             _tpoam_cmd_show_section(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "event"))/*show mpls-tp oam-y1731 event*//*/event/${service_type}/${part}*/
        {
            rc = _tpoam_cmd_show_event(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "event_session"))/*show mpls-tp oam-y1731 event session*//*/event_session/${session_id}*/
        {
            rc = _tpoam_cmd_show_event_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "dmm_session"))/*show mpls-tp oam-y1731 dmm session*//*/dmm_session/${session_id}*/
        {
            rc = _tpoam_cmd_show_dmm_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "1dm"))/*show mpls-tp oam-y1731 1dm*//*/1dm/${service_type}/${part}*/
        {
            rc = _tpoam_cmd_show_1dm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "lm"))/*show mpls-tp oam-y1731 lm*//*/lm/${service_type}/${part}/${exp_value}*/
        {
            rc = _tpoam_cmd_show_lm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "lm_session"))/*show mpls-tp oam-y1731 lm session*//*/lm_session/${session_id}/${exp_value}*/
        {
            rc = _tpoam_cmd_show_lm_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "lm_upload_session"))/*show mpls-tp oam-y1731 lm session upload*//*/lm_upload_session/${session_id}/${exp_value}*/
        {
            rc = _tpoam_cmd_show_lm_upload_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "single_lm"))/*show mpls-tp oam-y1731 single-lm*//*//single_lm/${service_type}/${part}*/
        {
            rc = _tpoam_cmd_show_single_lm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "loopback_show"))
        {
            _tpoam_cmd_exec_show_lb(para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid parameter");
        break;
    }

    //TPOAM_CLI_ERROR_RETURN(rc);
    return rc;
}
#endif

#ifndef tpoam_clear

int32
tpoam_cmd_clear_statistics_one(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t           *p_db_oam_session = obj;

    if (NULL == pargs)
    {
        sal_memset(&p_db_oam_session->oam_stats, 0x00, sizeof(tpoam_stats_t));
    }

    return PM_E_NONE;
}

int32
_tpoam_cmd_clear_lsp_pe_statistics(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_pe_t           *p_db_lsp_pe = obj;
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;

    oam_session_key.session_id = p_db_lsp_pe->oam_session_id;
    if (oam_session_key.session_id != 0)
    {
        p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL != p_db_oam_session)
        {
            tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
        }
    }

    return PM_E_NONE;
}


int32
_tpoam_cmd_clear_lsp_p_statistics(void *obj, tbl_iter_args_t *pargs)
{
    tbl_lsp_p_t           *p_db_lsp_p = obj;
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;

    oam_session_key.session_id = p_db_lsp_p->oam_session_id;
    if (oam_session_key.session_id != 0)
    {
        p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL != p_db_oam_session)
        {
            tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_tpoam_cmd_clear_section_oam_statistics(void *obj, tbl_iter_args_t *pargs)
{
    tbl_section_oam_t           *p_db_section_oam = obj;
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;

    oam_session_key.session_id = p_db_section_oam->oam_session_id;
    if (oam_session_key.session_id != 0)
    {
        p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL != p_db_oam_session)
        {
            tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_tpoam_cmd_clear_pw_tpe_statistics(void *obj, tbl_iter_args_t *pargs)
{
    tbl_pw_tpe_t           *p_db_pw_tpe = obj;
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;

    oam_session_key.session_id = p_db_pw_tpe->oam_session_id;
    if (oam_session_key.session_id != 0)
    {
        p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL != p_db_oam_session)
        {
            tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
        }
    }

    return PM_E_NONE;
}

/*/statistics/${service_type}/${part}*/
int32
_tpoam_cmd_clear_statistics(char **argv, int32 argc)
{
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;
    tbl_lsp_p_t *p_db_lsp_p = NULL;
    tbl_section_oam_t *p_db_section_oam = NULL;
    tbl_oam_session_t *p_db_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;

    oam_session_key.session_id = 0;

    /*clear all statistics*/
    if (0 == sal_strcmp(argv[1], "all"))
    {
        tbl_oam_session_iterate(tpoam_cmd_clear_statistics_one, NULL);
    }
    else if (0 == sal_strcmp(argv[2], "all"))
    {
        /*clear all lsp statistics*/
        if (0 == sal_strcmp(argv[1], "lsp"))
        {
            tbl_lsp_pe_iterate(_tpoam_cmd_clear_lsp_pe_statistics, NULL);
            tbl_lsp_p_iterate(_tpoam_cmd_clear_lsp_p_statistics, NULL);
        }
        /*clear all section statistics*/
        else if (0 == sal_strcmp(argv[1], "section"))
        {
            tbl_section_oam_iterate(_tpoam_cmd_clear_section_oam_statistics, NULL);
        }
        /*clear all pw-tpe statistics*/
        else if (0 == sal_strcmp(argv[1], "pw-tpe"))
        {
            tbl_pw_tpe_iterate(_tpoam_cmd_clear_pw_tpe_statistics, NULL);
        }
        else
        {
            return PM_E_FAIL;
        }
    }
    else
    {
        if (0 == sal_strcmp(argv[1], "lsp"))
        {
            p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe((tbl_lsp_pe_key_t *)(argv[2]));
            if (NULL != p_db_lsp_pe)
            {
                oam_session_key.session_id = p_db_lsp_pe->oam_session_id;
            }
            else
            {   /*lsp and lsp-pe cannot same*/
                p_db_lsp_p = tbl_lsp_p_get_lsp_p((tbl_lsp_p_key_t *)(argv[2]));
                if (NULL != p_db_lsp_p)
                {
                    oam_session_key.session_id = p_db_lsp_p->oam_session_id;
                }
            }
        }
        else if (0 == sal_strcmp(argv[1], "section"))
        {
            p_db_section_oam = tbl_section_oam_get_section_oam((tbl_section_oam_key_t *)(argv[2]));
            if (NULL != p_db_section_oam)
            {
                oam_session_key.session_id = p_db_section_oam->oam_session_id;
            }
        }
        else if (0 == sal_strcmp(argv[1], "pw-tpe"))
        {
            p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe((tbl_pw_tpe_key_t *)(argv[2]));
            if (NULL != p_db_pw_tpe)
            {
                oam_session_key.session_id = p_db_pw_tpe->oam_session_id;
            }
        }
        else
        {
            return PM_E_FAIL;
        }

        if (oam_session_key.session_id != 0)
        {
            p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
            if (NULL != p_db_oam_session)
            {
                tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
            }
        }
    }
    
    return PM_E_NONE;
}


/*/statistics_session/${session_id}*/
int32
_tpoam_cmd_clear_statistics_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    tbl_oam_session_t *p_db_oam_session = NULL;
    uint32 session_id = 0;
    int32 rc = 0;

    /*clear all statistics*/
    if (0 == sal_strcmp(argv[1], "all"))
    {
        tbl_oam_session_iterate(tpoam_cmd_clear_statistics_one, NULL);

        /*clear global statistics*/
        sal_memset(&pst_master->oam_glb_stats, 0x00, sizeof(tpoam_stats_t));
    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
        p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(session_id);
        if (NULL != p_db_oam_session)
        {
            tpoam_cmd_clear_statistics_one(p_db_oam_session, NULL);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Oam session %u does not exist", session_id);
        }
    }
    
    return PM_E_NONE;
}

/*/event/${service_type}/${part}*/
/*/1dm/${service_type}/${part}*/
int32
_tpoam_cmd_clear_event_or_1dm(char **argv, int32 argc)
{
    uint32 clear_type = 0;
    uint32 type = 0;
    tpoam_pw_type_t pw_type = TPOAM_PW_TYPE_MAX;
    int32 rc = PM_E_NONE;

    char* pst_service_name = NULL;
    
    if (!sal_strncmp(argv[0], "evt", 3))
    {
        clear_type = TPOAM_API_CLEAR_TYPE_EVENT;
    }
    else if (!sal_strncmp(argv[0], "1dm", 3))
    {
        clear_type = TPOAM_API_CLEAR_TYPE_1DM;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {
        type = 0;
        pw_type = TPOAM_PW_TYPE_MAX;
    }
    else if (0 == sal_strcmp(argv[1], "lsp"))
    {
        type = GLB_OAM_TYPE_MPLS_LSP;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        type = GLB_OAM_TYPE_MPLS_SECTION;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        type = GLB_OAM_TYPE_MPLS_PW;
        pw_type = TPOAM_PW_TYPE_TPE;
        if (0 != sal_strcmp(argv[2], "all"))
        {
            pst_service_name = argv[2];
        }
    }
    else
    {
        return PM_E_FAIL;
    }

    rc = tpoam_api_clear_evt_or_1dm(type, pw_type, pst_service_name, clear_type);
    if (rc < 0)
    {
        //sal_fprintf(fp, "%% %s \n", tpoam_err_desc(rc));
        return PM_E_NONE;
    }
    
    return PM_E_NONE;
}


/*/event_session/${session_id}*/
/*/1dm_session/${session_id}*/
int32
_tpoam_cmd_clear_event_or_1dm_session(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 clear_type = 0;
    int32 rc = PM_E_NONE;

    uint32 session_id = 0;
    
    if (!sal_strncmp(argv[0], "evt", 3))
    {
        clear_type = TPOAM_API_CLEAR_TYPE_EVENT;
    }
    else if (!sal_strncmp(argv[0], "1dm", 3))
    {
        clear_type = TPOAM_API_CLEAR_TYPE_1DM;
    }
    
    if (0 == sal_strcmp(argv[1], "all"))
    {

    }
    else
    {
        PM_CFG_GET_UINT(session_id, argv[1], para->p_rs);
    }

    rc = tpoam_api_clear_evt_or_1dm_session(session_id, clear_type);
    if (rc < 0)
    {
        //sal_fprintf(fp, "%% %s \n", tpoam_err_desc(rc));
        return PM_E_NONE;
    }
    
    return PM_E_NONE;
}

int32
tpoam_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == sal_strcmp(argv[0], "event"))/* clear mpls-tp oam-y1731 event *//*/event/${service_type}/${part}*/
        {
            _tpoam_cmd_clear_event_or_1dm(argv, argc);
        }
        else if (0 == sal_strcmp(argv[0], "event_session"))/* clear mpls-tp oam-y1731 event *//*/event_session/${session_id}*/
        {
            _tpoam_cmd_clear_event_or_1dm_session(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "1dm"))/* clear mpls-tp oam-y1731 1dm *//*/1dm/${service_type}/${part}*/
        {
            _tpoam_cmd_clear_event_or_1dm(argv, argc);
        }
        else if (0 == sal_strcmp(argv[0], "statistics"))/* clear mpls-tp oam-y1731 statistics *//*/statistics/${service_type}/${part}*/
        {
            _tpoam_cmd_clear_statistics(argv, argc);
        }
        else if (0 == sal_strcmp(argv[0], "statistics_session"))/* clear mpls-tp oam-y1731 statistics session*//*/statistics_session/${session_id}*/
        {
            rc = _tpoam_cmd_clear_statistics_session(argv, argc, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}
#endif

#ifndef tpoam_exec

int32
tpoam_api_service_get_oam(uint32 mode, char* pch_service_name, tbl_oam_session_t** ppst_oam)
{
    tbl_oam_session_t* pst_oam = NULL;
    int32 rc = TPOAM_E_NONE;

    tbl_section_oam_t *pst_section = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_lsp_p_t *pst_lsp_p = NULL;
    tbl_pw_tpe_t * pst_tpe = NULL;

    //TPOAM_DBG_API("mode %d (%s) service name %s", mode, cli_mode_str(mode), pch_service_name);

    *ppst_oam = NULL;
    switch (mode)
    {
    case MPLS_TP_OAM_SECTION_MODE:
        rc = tpoam_api_get_section(pch_service_name, &pst_section);
        if (rc < 0)
        {
            return rc;
        }
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_section->oam_session_id);
        break;
    case MPLS_LSP_PE_MODE:
        rc = tpoam_api_get_lsp_pe(pch_service_name, &pst_lsp_pe);
        if (rc < 0)
        {
            return rc;
        }
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_pe->oam_session_id);
        break;
    case MPLS_LSP_P_MODE:
        rc = tpoam_api_get_lsp_p(pch_service_name, &pst_lsp_p);
        if (rc < 0)
        {
            return rc;
        }
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_lsp_p->oam_session_id);
        break;
    case MPLS_PW_TPE_MODE:
        rc = tpoam_api_get_pw_tpe(pch_service_name, &pst_tpe);
        if (rc < 0)
        {
            return rc;
        }
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_tpe->oam_session_id);
        break;        
    default:
        return TPOAM_E_INVALID_TYPE;
    }
    
    /* 1. Check LSP exist megid */
    if (!pst_oam)
    {
        return TPOAM_E_MEGID_SEV_NOT_EXIST;
    }
    
    *ppst_oam = pst_oam;
    return TPOAM_E_NONE;
}



#ifndef exec_lmm
void tpoam_clear_lmm_all(void)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lm.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_lm, 0, sizeof(tpoam_lmm_t));
    }
}


uint32   
tpoam_set_lmm_service_type(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_lm.service_type = int_val;
    return SNMP_ERR_NOERROR;
}

static int32
_tpoam_set_lmm_para_value(uint32 argc, char** argv, uint32 para_index_base, 
                                tpoam_lmm_para_t* pst_para)
{
    uint32 arg_index = 0;

    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "100ms", 5);
    if (arg_index)
    {
        pst_para->interval = TPOAM_LM_INTERVAL_100MS;
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "1s", 2);
    if (arg_index)
    {
        pst_para->interval = TPOAM_LM_INTERVAL_1S;
    }
    arg_index = tpoam_get_prefix_item(&argv[para_index_base], argc-para_index_base, "10s", 3);
    if (arg_index)
    {
        pst_para->interval = TPOAM_LM_INTERVAL_10S;
    }    
    
    return PM_E_NONE;
}

static int32
_tpoam_get_lmm_para_value(char* interval_str,  tpoam_lmm_para_t* pst_para)
{
    if (0 == sal_strcmp(interval_str, "100ms"))
    {
        pst_para->interval = TPOAM_LM_INTERVAL_100MS;
    }
    else if (0 == sal_strcmp(interval_str, "1s"))
    {
        pst_para->interval = TPOAM_LM_INTERVAL_1S;
    }
    else if (0 == sal_strcmp(interval_str, "10s"))
    {
        pst_para->interval = TPOAM_LM_INTERVAL_10S;
    }
    else
    {
        pst_para->interval = TPOAM_LM_INTERVAL_1S;
    }
    
    return PM_E_NONE;
}


uint32   
tpoam_set_lmm_service_name(u_char *var_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    sal_strcpy (p_tpoam_exec_global->exec_lm.service_name, (char*)var_val);           
    return SNMP_ERR_NOERROR;
}


uint32   
tpoam_set_lmm_repeat(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_lm.para.repeat_cnt = int_val;
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_lmm_interval(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_lm.para.interval = int_val;
    return SNMP_ERR_NOERROR;
}

/* modified by huwx for 722 mib, bug31133, 2014-12-29 */
uint32   
tpoam_set_lmm_exist(bool val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    p_tpoam_exec_global->exec_lm.exist = val;
    return SNMP_ERR_NOERROR;
}

/*/lmm/${service_type}/${part}/${repeat_value}/${part2}*/
int32
_tpoam_cmd_exec_lmm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char* pst_service_name = NULL;
    uint32 mode = 0;
    tpoam_lmm_para_t lmm_para;
    tbl_oam_session_t* pst_oam = NULL;

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp(argv[1], "lsp-pe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_LSP_PE_MODE;
        tpoam_set_lmm_service_type(GLB_OAM_TYPE_MPLS_LSP);
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        pst_service_name = argv[2];
        mode = MPLS_TP_OAM_SECTION_MODE;
        tpoam_set_lmm_service_type(GLB_OAM_TYPE_MPLS_SECTION);
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_PW_TPE_MODE;
        tpoam_set_lmm_service_type(GLB_OAM_TYPE_MPLS_PW);
    }
    else
    {
        tpoam_clear_lmm_all();
        CFG_INVALID_PARAM_RET("Invalid service type");
        return PM_E_FAIL;
    }
        
    rc = tpoam_api_service_get_oam(mode, pst_service_name, &pst_oam);
    if (rc < 0)
    {
        tpoam_clear_lmm_all();
        CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
        return PM_E_FAIL;
    }

    sal_memset(&lmm_para, 0, sizeof(lmm_para));
    lmm_para.interval = TPOAM_LM_INTERVAL_1S;

    PM_CFG_GET_UINT(lmm_para.repeat_cnt, argv[3], para->p_rs);

    rc = _tpoam_set_lmm_para_value(argc, argv, 4, &lmm_para);
    _tpoam_get_lmm_para_value(argv[4], &lmm_para);
    if (rc < 0)
    {
        tpoam_clear_lmm_all();
        return PM_E_FAIL;
    }

    /* modified by huwx for 722 mib, bug31133, 2014-12-29 */
    tpoam_set_lmm_service_name((u_char*)pst_service_name);
    tpoam_set_lmm_repeat(lmm_para.repeat_cnt);
    tpoam_set_lmm_interval(lmm_para.interval + 1);
    tpoam_set_lmm_exist(TRUE);

    // TODO:
    #if 0
    rc = _tpoam_cli_lmm(cli, pst_oam, &para);
    if (rc < 0)
    {
        tpoam_clear_lmm_all();
        cli_out(cli, "%% %s \n", tpoam_err_desc(rc));
        return PM_E_FAIL;
    }
    #endif
    
    return PM_E_NONE;
}
#endif

#ifndef exec_dm
uint32   
tpoam_set_dmm_request_service_type(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_dmm.service_type = int_val;
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_dmm_request_service_name(u_char *var_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    sal_strcpy (p_tpoam_exec_global->exec_dmm.service_name, (char*)var_val);           
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_dmm_request_repeat(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_dmm.para.repeat_cnt = int_val;
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_dmm_request_exp(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_dmm.para.exp = int_val + 1;
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_dmm_request_interval(uint32 int_val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    
    p_tpoam_exec_global->exec_dmm.para.interval = int_val;
    return SNMP_ERR_NOERROR;
}

uint32   
tpoam_set_dmm_request_exist(bool val)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    p_tpoam_exec_global->exec_dmm.exist = val;
    return SNMP_ERR_NOERROR;
}


void tpoam_clear_lb_1dm_dmm_all(void)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    if (p_tpoam_exec_global->exec_lb_discovery.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_lb_discovery, 0, sizeof(tpoam_loopback_t));
    }

    if (p_tpoam_exec_global->exec_lb_rmep.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_lb_rmep, 0, sizeof(tpoam_loopback_t));
    }

    if (p_tpoam_exec_global->exec_lb_mip.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_lb_mip, 0, sizeof(tpoam_loopback_t));
    }

    if (p_tpoam_exec_global->exec_1dm.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_1dm, 0, sizeof(tpoam_1dm_dmm_t));  
    }

    if (p_tpoam_exec_global->exec_dmm.exist)
    {
        sal_memset(&p_tpoam_exec_global->exec_dmm, 0, sizeof(tpoam_1dm_dmm_t));
    }
}

static int32
_tpoam_get_dm_para_value(char* interval_str,  tpoam_dm_para_t* pst_para)
{
    if (0 == sal_strcmp(interval_str, "100ms"))
    {
        pst_para->interval = TPOAM_DM_INTERVAL_100MS;
    }
    else if (0 == sal_strcmp(interval_str, "1s"))
    {
        pst_para->interval = TPOAM_DM_INTERVAL_1S;
    }
    else if (0 == sal_strcmp(interval_str, "10s"))
    {
        pst_para->interval = TPOAM_DM_INTERVAL_10S;
    }
    
    return PM_E_NONE;
}

int32
tpoam_api_dm_start_1dm(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para)
{
    return tpoam_dm_start(pst_oam, pst_para, TRUE);
}

static int32
_tpoam_cli_1dm(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para)
{
    return tpoam_api_dm_start_1dm(pst_oam, pst_para);
}

int32
tpoam_api_dm_start_dmm(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para)
{
    return tpoam_dm_start(pst_oam, pst_para, TRUE);
}

static int32
_tpoam_cli_dmm(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para)
{
    int32 ret = PM_E_NONE;

    #if 0
    if (_nsm_tpoam_cli_exec_check(cli) < 0)
    {
        return CLI_SUCCESS;
    }
    #endif
    ret = tpoam_api_dm_start_dmm(pst_oam, pst_para);
    if (ret < 0)
    {
        return ret;
    }

    /*
    cli->status = CLI_NORMAL;
    cli->count = 0;
    cli->current = &g_p_tpoam_master->exec;
    cli->callback = _nsm_tpoam_cli_dmm_reply_cb;
    cli_out(cli, "Sending %d MPLS-TP OAM Y.1731 DMM messages:\n",
              pst_para->repeat_cnt);
    cli_out(cli, "(! Receive a DMR) \n");
*/
    //nsm_tpoam_api_exec_init(pst_oam, NSM_TPOAM_EXEC_TYPE_DM);
    return PM_E_NONE;
}

/*/lmm/${service_type}/${part}/${repeat_value}/${part2}*/
/*/1dm/${service_type}/${part}/${repeat_value}/${part2}/${exp_value}*/
/*/dmm/${service_type}/${part}/${repeat_value}/${part2}/${exp_value}*/
int32
_tpoam_cmd_exec_dm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char* pst_service_name = NULL;
    uint32 mode = 0;
    tpoam_dm_para_t dm_para;
    tbl_oam_session_t* pst_oam = NULL;
    int service_type;

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    sal_memset(&dm_para, 0, sizeof(dm_para));
    dm_para.interval = TPOAM_LM_INTERVAL_1S;
    
    if (0 == sal_strcmp(argv[0], "1dm"))
    {
        dm_para.type = TPOAM_DM_TYPE_1DM;
    }
    else if (0 == sal_strcmp(argv[0], "dmm"))
    {
        dm_para.type = TPOAM_DM_TYPE_DMM;
    }
    else
    {
        tpoam_clear_lb_1dm_dmm_all();
        CFG_INVALID_PARAM_RET("Invalid DM type");
    }
    
    if (0 == sal_strcmp(argv[1], "lsp-pe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_LSP_PE_MODE;
        service_type = GLB_OAM_TYPE_MPLS_LSP;
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        pst_service_name = argv[2];
        mode = MPLS_TP_OAM_SECTION_MODE;
        service_type = GLB_OAM_TYPE_MPLS_SECTION;
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_PW_TPE_MODE;
        service_type = GLB_OAM_TYPE_MPLS_PW;
    }
    else
    {
        tpoam_clear_lb_1dm_dmm_all();
        CFG_INVALID_PARAM_RET("Invalid service type");
    }
        
    rc = tpoam_api_service_get_oam(mode, pst_service_name, &pst_oam);
    if (rc < 0)
    {
        tpoam_clear_lb_1dm_dmm_all();
        CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
    }

    PM_CFG_GET_UINT(dm_para.repeat_cnt, argv[3], para->p_rs);
    
    dm_para.interval = TPOAM_DM_INTERVAL_1S;
    dm_para.exp = TPOAM_EXP_DEF;

    if (argc >= 6)
    {
        PM_CFG_GET_UINT(dm_para.exp, argv[5], para->p_rs);
    }
    
    _tpoam_get_dm_para_value(argv[4], &dm_para);
    if (rc < 0)
    {
        tpoam_clear_lb_1dm_dmm_all();
        return PM_E_FAIL;
    }

    
    if (TPOAM_DM_TYPE_1DM == dm_para.type)
    {
        rc = _tpoam_cli_1dm(pst_oam, &dm_para);
        if (rc < 0)
        {
            tpoam_clear_lb_1dm_dmm_all();
            CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
        }
    }
    else if (TPOAM_DM_TYPE_DMM == dm_para.type)
    {
        tpoam_set_dmm_request_service_type(service_type);
        tpoam_set_dmm_request_service_name((u_char*)pst_service_name);
        tpoam_set_dmm_request_repeat(dm_para.repeat_cnt);
        tpoam_set_dmm_request_exp(dm_para.exp);
        tpoam_set_dmm_request_interval(dm_para.interval);
        tpoam_set_dmm_request_exist(TRUE);
    
        rc = _tpoam_cli_dmm(pst_oam, &dm_para);
        if (rc < 0)
        {
            tpoam_clear_lb_1dm_dmm_all();
            CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
        }
    }

    return PM_E_NONE;
}

int32
tpoam_api_dm_stop_dmm(tbl_oam_session_t* pst_oam)
{
    if (!pst_oam->dm_active || (TPOAM_DM_TYPE_DMM != pst_oam->oam_dm.para.type))
    {
        return TPOAM_E_DM_DMM_IS_NOT_RUNNING;
    }   
    return tpoam_dm_stop(pst_oam);
}

int32
tpoam_api_dm_stop_1dm(tbl_oam_session_t* pst_oam)
{
    if (!pst_oam->dm_active || (TPOAM_DM_TYPE_1DM != pst_oam->oam_dm.para.type))
    {
        return TPOAM_E_DM_1DM_IS_NOT_RUNNING;
    }
    return tpoam_dm_stop(pst_oam);
}

int32
_tpoam_cmd_exec_dm_stop(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char* pst_service_name = NULL;
    uint32 mode = 0;
    tpoam_dm_para_t dm_para;
    tbl_oam_session_t* pst_oam = NULL;
    uint32 dm_type = 0;

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    sal_memset(&dm_para, 0, sizeof(dm_para));
    dm_para.interval = TPOAM_LM_INTERVAL_1S;
    
    if (0 == sal_strcmp(argv[0], "1dm"))
    {
        dm_type = TPOAM_DM_TYPE_1DM;
    }
    else if (0 == sal_strcmp(argv[0], "dmm"))
    {
        dm_type = TPOAM_DM_TYPE_DMM;
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid DM type");
    }
    
    if (0 == sal_strcmp(argv[1], "lsp-pe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_LSP_PE_MODE;
    }
    else if (0 == sal_strcmp(argv[1], "section"))
    {
        pst_service_name = argv[2];
        mode = MPLS_TP_OAM_SECTION_MODE;
    }
    else if (0 == sal_strcmp(argv[1], "pw-tpe"))
    {
        pst_service_name = argv[2];
        mode = MPLS_PW_TPE_MODE;
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid service type");
    }
        
    rc = tpoam_api_service_get_oam(mode, pst_service_name, &pst_oam);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
    }

    if (TPOAM_DM_TYPE_1DM == dm_type)
    {
        rc = tpoam_api_dm_stop_1dm(pst_oam);
    }
    else
    {
        rc = tpoam_api_dm_stop_dmm(pst_oam);
    }
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("%s", tpoam_err_desc(rc));
    }
    
    return PM_E_NONE;
}
#endif

#ifndef exec_lb
#define use_new
cfg_result_t g_lb_result_str = {0};

static void
_tpoam_send_msg_to_ccs_cb(void *p_data)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    //tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    //tpoam_lb_reply_t lb_reply;
    //int32 ret = TPOAM_E_NONE;
    
    p_tpoam_exec_global->p_t_send_lb_msg_timer = NULL;

    ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &g_lb_result_str);

    if (g_lb_result_str.rc == PM_E_NONE)
    {
        tbl_oam_session_t* pst_oam = NULL;

        pst_oam = tbl_oam_session_get_oam_session_by_session_id(p_tpoam_exec_global->oam_session_id);
        
        //g_exec_new.inprocess = FALSE;
        p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
        p_tpoam_exec_global->oam_session_id = 0;
        p_tpoam_exec_global->pid = 0;
        p_tpoam_exec_global->sock = 0;

        if (pst_oam)
        {
            tpoam_api_exec_deinit(pst_oam);
        }
    }

    sal_memset(&g_lb_result_str, 0x00, sizeof(g_lb_result_str));
    
    return;
}

void 
_tpoam_send_msg_to_ccs_on_timer(cfg_result_t *p_result)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    uint32 strlen1 = 0;
    uint32 strlen2 = 0;
    strlen1 = sal_strlen(g_lb_result_str.desc);
    strlen2 = sal_strlen(p_result->desc);
    
    if (strlen1 == 0)
    {
        //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, strlen1);
        //TPOAM_LOG_ERR("(%s, %d) p_result:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, p_result->desc, p_result->rc, strlen2);
        sal_memcpy(&g_lb_result_str, p_result, sizeof(cfg_result_t));

        if (!p_tpoam_exec_global->p_t_send_lb_msg_timer)
        {
            /*start timer*/
            p_tpoam_exec_global->p_t_send_lb_msg_timer = tpoam_util_add_timer_ms(_tpoam_send_msg_to_ccs_cb, NULL, GLB_TPOAM_EXEC_SEND_LB_MSG_TIMER);
        }
        
        return;
    }
    else if ((strlen1 + strlen2) < MAX_ON_DEMAND_CMD_RET_LEN)
    {
        //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, strlen1);
        //TPOAM_LOG_ERR("(%s, %d) p_result:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, p_result->desc, p_result->rc, strlen2);
        if (g_lb_result_str.rc == p_result->rc)
        {
            sal_memcpy(g_lb_result_str.desc + strlen1, p_result->desc, strlen2);
            //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, sal_strlen(g_lb_result_str.desc));
            /*start timer*/
            //p_tpoam_exec_global->p_t_send_lb_msg_timer = tpoam_util_add_timer_ms(_tpoam_send_msg_to_ccs_cb, NULL, GLB_TPOAM_EXEC_SEND_LB_MSG_TIMER);

            return;
        }
        else
        {
            //tpoam_util_cancel_timer(&p_tpoam_exec_global->p_t_send_lb_msg_timer);
            
            /*send the old msg*/
            //ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &g_lb_result_str);

            //sal_memset(&g_lb_result_str, 0x00, sizeof(g_lb_result_str));
            //sal_memcpy(&g_lb_result_str, p_result, sizeof(cfg_result_t));
            /*start timer*/
            //p_tpoam_exec_global->p_t_send_lb_msg_timer = tpoam_util_add_timer_ms(_tpoam_send_msg_to_ccs_cb, NULL, GLB_TPOAM_EXEC_SEND_LB_MSG_TIMER);

            sal_memcpy(g_lb_result_str.desc + strlen1, p_result->desc, strlen2);
            g_lb_result_str.rc  = p_result->rc;
            //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, sal_strlen(g_lb_result_str.desc));
        }
    }
    else if ((strlen1 + strlen2) >= MAX_ON_DEMAND_CMD_RET_LEN)
    {
        //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, strlen1);
        //TPOAM_LOG_ERR("(%s, %d) p_result:%s, rc %d, len %d\n", __FUNCTION__, __LINE__, p_result->desc, p_result->rc, strlen2);
        
        tpoam_util_cancel_timer(&p_tpoam_exec_global->p_t_send_lb_msg_timer);
        
        /*send the old msg*/
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &g_lb_result_str);

        sal_memset(&g_lb_result_str, 0x00, sizeof(g_lb_result_str));
        sal_memcpy(&g_lb_result_str, p_result, sizeof(cfg_result_t));
        /*start timer*/
        p_tpoam_exec_global->p_t_send_lb_msg_timer = tpoam_util_add_timer_ms(_tpoam_send_msg_to_ccs_cb, NULL, GLB_TPOAM_EXEC_SEND_LB_MSG_TIMER);
    }
    else
    {
        //TPOAM_LOG_ERR("(%s, %d) g_lb_result_str:%s, rc %d, len %d, error\n", __FUNCTION__, __LINE__, g_lb_result_str.desc, g_lb_result_str.rc, strlen1);
        //TPOAM_LOG_ERR("(%s, %d) p_result:%s, rc %d, len %d, error\n", __FUNCTION__, __LINE__, p_result->desc, p_result->rc, strlen2);
    }

    return;
}

static int32
_tpoam_cli_exec_check()
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    if (TPOAM_EXEC_TYPE_NONE != p_tpoam_exec_global->test_type)
    {
        //cli_out(cli, "%% OAM session %u is running on-demand %s.\n", p_tpoam_exec_global->oam_session_id, 
        //    tpoam_api_exec_type_desc(p_tpoam_exec_global->test_type));
        return TPOAM_E_CLI_EXEC_IS_RUNNING;
    }
    
    return TPOAM_E_NONE;
}

void
_tpoam_cli_lb_mep_reply_cb(void *p_data)
{
    #ifndef use_new
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    #endif
    //tpoam_exec_t* pst_exec = (nsm_tpoam_exec_t*)cli->current;
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_lb_t* pst_lb = NULL;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    int len = 0;
    cfg_result_t result;

    //pst_oam = pst_exec->pst_oam;
    if (!pst_oam)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        return;
    }
    tpoam_util_cancel_timer(&pst_oam->oam_lb.p_t_lb_exec_timer);
    pst_oam->oam_lb.p_t_lb_exec_timer = NULL;
    
    pst_lb = &pst_oam->oam_lb;
    //if (cli->status == CLI_CLOSE)
    {
        //tpoam_lb_stop_lb(pst_oam);
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        //return TPOAM_E_NONE;
    }

    if (!pst_lb->active)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            sal_sprintf(lb_result_str + len,  "!\n");
            
            //TPOAM_DBG_LB_LBR("RMEP test end print %s", lb_result_str);
            break;
        case TPOAM_PER_LB_STATUS_TO:
            sal_sprintf(lb_result_str + len,  ".\n");

            //TPOAM_DBG_LB_LBR("RMEP test end print %s", lb_result_str);
            break;
        default:
            break;
        }

        len = sal_strlen(lb_result_str);

        /*output result summary*/
        sal_sprintf(lb_result_str + len,  "\n Loopback completed, takes %.2f seconds.\n", 
            ((pst_lb->lbm_stop.tv_sec - pst_lb->lbm_start.tv_sec)*1.0 + (pst_lb->lbm_stop.tv_usec - pst_lb->lbm_start.tv_usec)/1000000.0));
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " ----------------------------------- \n" );
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " Success rate is %d percent(%d/%d)\n", 
                   (( (pst_lb->lbr_rx_count) * 100 ) / pst_lb->lbm_tx_count),
                   pst_lb->lbr_rx_count, pst_lb->lbm_tx_count);

        //usleep(500000);
        sal_memset(&result, 0, sizeof(result));
        CFG_CONTINUE_RESULT(&result, PM_E_NONE, "%s", lb_result_str);
        #ifdef use_new
        _tpoam_send_msg_to_ccs_on_timer(&result);
        #else
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);

        //g_exec_new.inprocess = FALSE;
        p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
        p_tpoam_exec_global->oam_session_id = 0;
        p_tpoam_exec_global->pid = 0;
        p_tpoam_exec_global->sock = 0;
        
        tpoam_api_exec_deinit(pst_oam);
        #endif
        
    }
    else
    {
        //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_mep_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
        //cli->count = 0;
        //cli->callback = _tpoam_cli_lb_mep_reply_cb;
        //cli->status = CLI_WAIT_FLUSH;
        sal_memset(&result, 0, sizeof(result));
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "!");
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            break;
        case TPOAM_PER_LB_STATUS_TO:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, ".");
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            break;
        default:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "wait");
            break;
        }
        
        //usleep(100000);
        #ifdef use_new
        _tpoam_send_msg_to_ccs_on_timer(&result);
        #else
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
        #endif
        //TPOAM_DBG_LB_LBR("RMEP test print %s", result.desc);
        return;
    }
  
    return;
}

void
_tpoam_cli_lb_mip_reply_cb(void *p_data)
{
    #ifndef use_new
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    #endif
    //tpoam_exec_t* pst_exec = (nsm_tpoam_exec_t*)cli->current;
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_lb_t* pst_lb = NULL;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    int len = 0;
    cfg_result_t result;

    //pst_oam = pst_exec->pst_oam;
    if (!pst_oam)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        return;
    }
    tpoam_util_cancel_timer(&pst_oam->oam_lb.p_t_lb_exec_timer);
    pst_oam->oam_lb.p_t_lb_exec_timer = NULL;
    
    pst_lb = &pst_oam->oam_lb;
    //if (cli->status == CLI_CLOSE)
    {
        //tpoam_lb_stop_lb(pst_oam);
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        //return TPOAM_E_NONE;
    }

    if (!pst_lb->active)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            sal_sprintf(lb_result_str + len,  "!\n");
            
            //TPOAM_DBG_LB_LBR("MIP test end print %s", lb_result_str);
            break;
        case TPOAM_PER_LB_STATUS_TO:
            sal_sprintf(lb_result_str + len,  ".\n");
            
            //TPOAM_DBG_LB_LBR("MIP test end print %s", lb_result_str);
            break;
        default:
            break;
        }

        len = sal_strlen(lb_result_str);

        /*output result summary*/
        sal_sprintf(lb_result_str + len,  "\n Loopback completed, takes %.2f seconds.\n", 
            ((pst_lb->lbm_stop.tv_sec - pst_lb->lbm_start.tv_sec)*1.0 + (pst_lb->lbm_stop.tv_usec - pst_lb->lbm_start.tv_usec)/1000000.0));
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " ----------------------------------- \n" );
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " Success rate is %d percent(%d/%d)\n", 
                   (( (pst_lb->lbr_rx_count) * 100 ) / pst_lb->lbm_tx_count),
                   pst_lb->lbr_rx_count, pst_lb->lbm_tx_count);

        //usleep(500000);
        sal_memset(&result, 0, sizeof(result));
        CFG_CONTINUE_RESULT(&result, PM_E_NONE, "%s", lb_result_str);
        #ifdef use_new
        _tpoam_send_msg_to_ccs_on_timer(&result);
        #else
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
        //g_exec_new.inprocess = FALSE;
        p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
        p_tpoam_exec_global->oam_session_id = 0;
        p_tpoam_exec_global->pid = 0;
        p_tpoam_exec_global->sock = 0;
        
        tpoam_api_exec_deinit(pst_oam);
        #endif
    }
    else
    {  
        //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_mip_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
        //cli->count = 0;
        //cli->callback = _tpoam_cli_lb_mep_reply_cb;
        //cli->status = CLI_WAIT_FLUSH;
        sal_memset(&result, 0, sizeof(result));
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "!");
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            break;
        case TPOAM_PER_LB_STATUS_TO:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, ".");
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            break;
        default:
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "wait");
            break;
        }
        #ifdef use_new
        _tpoam_send_msg_to_ccs_on_timer(&result);
        #else
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
        #endif

        //TPOAM_DBG_LB_LBR("MIP test print %s", result.desc);
        return;
    }
  
    return;
}

void
_tpoam_cli_lb_discovery_reply_cb(void *p_data)
{
    #ifndef use_new
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    #endif
    //tpoam_exec_t* pst_exec = (nsm_tpoam_exec_t*)cli->current;
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_lb_t* pst_lb = NULL;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    int len = 0;
    cfg_result_t result;

    tpoam_lb_discovery_t* pst_lb_discovery = NULL;
    addr_ipv4_t node_id = {0};
    char        buf[CMD_BUF_16];

    //pst_oam = pst_exec->pst_oam;
    if (!pst_oam)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        return;
    }
    tpoam_util_cancel_timer(&pst_oam->oam_lb.p_t_lb_exec_timer);
    pst_oam->oam_lb.p_t_lb_exec_timer = NULL;
    
    pst_lb = &pst_oam->oam_lb;
    pst_lb_discovery = &pst_lb->discovery;
    //if (cli->status == CLI_CLOSE)
    {
        //tpoam_lb_stop_lb(pst_oam);
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        //return TPOAM_E_NONE;
    }

    if (!pst_lb->active)
    {
        //cli->callback = NULL;
        //cli->status = CLI_NORMAL;
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            sal_memset(&result, 0, sizeof(result));
            switch (pst_lb_discovery->reply.lb_sub_type)
            {
            case TPOAM_LB_MEP:
                sal_sprintf(lb_result_str + len, 
                    "%-3d %-8s %-8d\n", pst_lb_discovery->reply.ttl, "MEP", pst_lb_discovery->reply.mep_id);
                break;
            case TPOAM_LB_MIP:
                node_id.s_addr = pst_lb_discovery->reply.node_id;
                #ifdef support_ifnum
                sal_sprintf(lb_result_str + len, 
                    "%-3d %-8s %-8s %-13s %-15s %-5d\n", pst_lb_discovery->reply.ttl, "MIP", "", 
                    pst_lb_discovery->reply.icc, 
                    cdb_addr_ipv4_val2str(buf, sizeof(buf), &node_id), 
                    pst_lb_discovery->reply.if_num);
                #else
                sal_sprintf(lb_result_str + len, 
                    "%-3d %-8s %-8s %-13s %-15s\n", pst_lb_discovery->reply.ttl, "MIP", "", 
                    pst_lb_discovery->reply.icc, 
                    cdb_addr_ipv4_val2str(buf, sizeof(buf), &node_id));
                #endif
                break;
            default:
                sal_sprintf(lb_result_str + len, 
                    "%-3d %-8s\n", pst_lb_discovery->reply.ttl, "Invalid");
                break;
            }

            //TPOAM_DBG_LB_LBR("Discovery test end print %s", lb_result_str);
            break;
        case TPOAM_PER_LB_STATUS_TO:
            sal_sprintf(lb_result_str + len, "%-3d %-8s\n", pst_lb_discovery->timeout_ttl, "Timeout");

            //TPOAM_DBG_LB_LBR("Discovery test end print %s", lb_result_str);
            break;
        default:
            break;
        }

        len = sal_strlen(lb_result_str);

        /*output result summary*/
        sal_sprintf(lb_result_str + len,  "\n Loopback completed, takes %.2f seconds.\n", 
            ((pst_lb->lbm_stop.tv_sec - pst_lb->lbm_start.tv_sec)*1.0 + (pst_lb->lbm_stop.tv_usec - pst_lb->lbm_start.tv_usec)/1000000.0));
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " ----------------------------------- \n" );
        len = sal_strlen(lb_result_str);
        sal_sprintf(lb_result_str + len,  " Success rate is %d percent(%d/%d)\n", 
                   (( (pst_lb->lbr_rx_count) * 100 ) / pst_lb->lbm_tx_count),
                   pst_lb->lbr_rx_count, pst_lb->lbm_tx_count);

        //usleep(500000);
        sal_memset(&result, 0, sizeof(result));
        CFG_CONTINUE_RESULT(&result, PM_E_NONE, "%s", lb_result_str);
        #ifdef use_new
        _tpoam_send_msg_to_ccs_on_timer(&result);
        #else
        ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);

        //g_exec_new.inprocess = FALSE;
        p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
        p_tpoam_exec_global->oam_session_id = 0;
        p_tpoam_exec_global->pid = 0;
        p_tpoam_exec_global->sock = 0;
        
        tpoam_api_exec_deinit(pst_oam);
        #endif
        
    }
    else
    {  
        //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_mip_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
        //cli->count = 0;
        //cli->callback = _tpoam_cli_lb_mep_reply_cb;
        //cli->status = CLI_WAIT_FLUSH;
        switch (pst_lb->per_lb_status)
        {
        case TPOAM_PER_LB_STATUS_RX_LBR:
            sal_memset(&result, 0, sizeof(result));
            
            switch (pst_lb_discovery->reply.lb_sub_type)
            {
            case TPOAM_LB_MEP:
                CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, 
                    "%-3d %-8s %-8d\n", pst_lb_discovery->reply.ttl, "MEP", pst_lb_discovery->reply.mep_id);
                break;
            case TPOAM_LB_MIP:
                node_id.s_addr = pst_lb_discovery->reply.node_id;
                #ifdef support_ifnum
                CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, 
                    "%-3d %-8s %-8s %-13s %-15s %-5d\n", pst_lb_discovery->reply.ttl, "MIP", "", 
                    pst_lb_discovery->reply.icc, 
                    cdb_addr_ipv4_val2str(buf, sizeof(buf), &node_id), 
                    pst_lb_discovery->reply.if_num);
                #else
                CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, 
                    "%-3d %-8s %-8s %-13s %-15s\n", pst_lb_discovery->reply.ttl, "MIP", "", 
                    pst_lb_discovery->reply.icc, 
                    cdb_addr_ipv4_val2str(buf, sizeof(buf), &node_id));
                #endif
                break;
            default:
                CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, 
                    "%-3d %-8s\n", pst_lb_discovery->reply.ttl, "Invalid");
                break;
            }

            #ifdef use_new
            _tpoam_send_msg_to_ccs_on_timer(&result);
            #else
            ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
            #endif
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            //TPOAM_DBG_LB_LBR("Discovery test print %s", result.desc);
            break;
        case TPOAM_PER_LB_STATUS_TO:
            sal_memset(&result, 0, sizeof(result));
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, 
                "%-3d %-8s\n", pst_lb_discovery->timeout_ttl, "Timeout");
            #ifdef use_new
            _tpoam_send_msg_to_ccs_on_timer(&result);
            #else
            ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
            #endif
            pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
            //TPOAM_DBG_LB_LBR("Discovery test print %s", result.desc);
            break;
        default:
            sal_memset(&result, 0, sizeof(result));
            CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "wait");
            #ifdef use_new
            _tpoam_send_msg_to_ccs_on_timer(&result);
            #else
            ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
            #endif
            //TPOAM_DBG_LB_LBR("Discovery test print %s", result.desc);
            break;
        }
        return;
    }
  
    return;
}

void 
tpoam_exec_lb_init()
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    p_tpoam_exec_global->pid = 0;
    p_tpoam_exec_global->sock = 0;
    p_tpoam_exec_global->oam_session_id = 0;
    p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
}

static int32
_tpoam_cli_lb_mep(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para, cfg_cmd_para_t *para)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    int32 ret = TPOAM_E_NONE;
    cfg_result_t result;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    uint32 len = 0;
    
    ret = tpoam_lb_start_lb_check(pst_oam, pst_lbm_para);
    if (ret < 0)
    {
        TPOAM_CLI_ERROR_RETURN_WITH_N(ret);
        return ret;
    }

    tpoam_exec_lb_init();

    p_tpoam_exec_global->pid = para->pid;
    p_tpoam_exec_global->sock = para->sock;
    p_tpoam_exec_global->oam_session_id = pst_oam->key.session_id;
    p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_LB_MEP;
    
    //cli->status = CLI_NORMAL;
    //cli->count = 0;
    //cli->current = &g_p_tpoam_master->exec;
    //cli->callback = _tpoam_cli_lb_mep_reply_cb;

    sal_memset(&result, 0, sizeof(result));
    sal_sprintf(lb_result_str + len, "Sending MPLS-TP OAM Y.1731 loopback MEP messages\n");
    len = sal_strlen(lb_result_str);

    tpoam_api_lb_show_para(lb_result_str, &len, pst_lbm_para);

    sal_sprintf(lb_result_str + len, "(! Pass . Wait)\n");
    len = sal_strlen(lb_result_str);

    CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "%s", lb_result_str);
    ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);

    //usleep(100000);
    ret = tpoam_lb_start_lb_send(pst_oam, pst_lbm_para);

    //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_mep_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
    return TPOAM_E_NONE;
}

static int32
_tpoam_cli_lb_mip(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para, cfg_cmd_para_t *para)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    int32 ret = TPOAM_E_NONE;
    cfg_result_t result;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    uint32 len = 0;
    
    ret = tpoam_lb_start_lb_check(pst_oam, pst_lbm_para);
    if (ret < 0)
    {
        TPOAM_CLI_ERROR_RETURN_WITH_N(ret);
        return ret;
    }

    tpoam_exec_lb_init();

    p_tpoam_exec_global->pid = para->pid;
    p_tpoam_exec_global->sock = para->sock;
    p_tpoam_exec_global->oam_session_id = pst_oam->key.session_id;
    p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_LB_MIP;
    
    //cli->status = CLI_NORMAL;
    //cli->count = 0;
    //cli->current = &g_p_tpoam_master->exec;
    //cli->callback = _tpoam_cli_lb_mep_reply_cb;

    sal_memset(&result, 0, sizeof(result));
    sal_sprintf(lb_result_str + len, "Sending MPLS-TP OAM Y.1731 loopback MIP messages\n");
    len = sal_strlen(lb_result_str);

    tpoam_api_lb_show_para(lb_result_str, &len, pst_lbm_para);

    sal_sprintf(lb_result_str + len, "(! Pass . Wait)\n");
    len = sal_strlen(lb_result_str);

    CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "%s", lb_result_str);
    ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);

    //usleep(100000);
    ret = tpoam_lb_start_lb_send(pst_oam, pst_lbm_para);
    
    //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_mip_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
    return TPOAM_E_NONE;
}

static int32
_tpoam_cli_lb_discovery(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para, cfg_cmd_para_t *para)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    
    int32 ret = TPOAM_E_NONE;
    cfg_result_t result;
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    uint32 len = 0;
    
    ret = tpoam_lb_start_lb_check(pst_oam, pst_lbm_para);
    if (ret < 0)
    {
        TPOAM_CLI_ERROR_RETURN_WITH_N(ret);
        return ret;
    }

    tpoam_exec_lb_init();

    p_tpoam_exec_global->pid = para->pid;
    p_tpoam_exec_global->sock = para->sock;
    p_tpoam_exec_global->oam_session_id = pst_oam->key.session_id;
    p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_LB_DISCOVERY;
    
    //cli->status = CLI_NORMAL;
    //cli->count = 0;
    //cli->current = &g_p_tpoam_master->exec;
    //cli->callback = _tpoam_cli_lb_mep_reply_cb;

    sal_memset(&result, 0, sizeof(result));
    sal_sprintf(lb_result_str + len, "Sending MPLS-TP OAM Y.1731 loopback discovery messages\n");
    len = sal_strlen(lb_result_str);

    tpoam_api_lb_show_para(lb_result_str, &len, pst_lbm_para);

#ifdef support_ifnum
    sal_sprintf(lb_result_str + len, 
        "%-3s %-8s %-8s %-13s %-15s %-5s\n", "TTL", "Reply", "MEPID", "ICC", "NodeID", "IFNUM");
    len = sal_strlen(lb_result_str);

    sal_sprintf(lb_result_str + len, 
        "---+--------+--------+-------------+---------------+-----\n");
    len = sal_strlen(lb_result_str);
#else
    sal_sprintf(lb_result_str + len, 
        "%-3s %-8s %-8s %-13s %-15s\n", "TTL", "Reply", "MEPID", "ICC", "NodeID");
    len = sal_strlen(lb_result_str);

    sal_sprintf(lb_result_str + len, 
        "---+--------+--------+-------------+---------------\n");
    len = sal_strlen(lb_result_str);
#endif
    CFG_CONTINUE_RESULT(&result, PM_E_CFG_CONTINUE, "%s", lb_result_str);
    ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);

    //usleep(100000);
    ret = tpoam_lb_start_lb_send(pst_oam, pst_lbm_para);
    
    //pst_oam->oam_lb.p_t_lb_exec_timer = tpoam_util_add_timer_ms(_tpoam_cli_lb_discovery_reply_cb, pst_oam, TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF);
    return TPOAM_E_NONE;
}

int32
_tpoam_exe_lb(uint32 oam_session_id, tpoam_lbm_para_t* pst_lbm_para, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    tbl_oam_session_t *p_db_oam_session = NULL;
    //FILE *fp = NULL;
    
    rc = _tpoam_cli_exec_check();
    if (rc)
    {
        TPOAM_CLI_ERROR_RETURN_WITH_N(rc);
        return rc;
    }

    p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(oam_session_id);
    if (!p_db_oam_session)
    {
        CFG_INVALID_PARAM_RET("Tpoam session %u does not exist\n", oam_session_id);
    }

    //if (GLB_OAM_TYPE_MPLS_SECTION == p_db_oam_session->oam_type && TPOAM_LB_MIP == pst_lbm_para->lb_sub_type)
    if (GLB_OAM_TYPE_MPLS_SECTION == p_db_oam_session->oam_type)
    {
        //CFG_INVALID_PARAM_RET("Section OAM does not support loopback MIP\n");
        CFG_INVALID_PARAM_RET("Section OAM does not support loopback\n");
    }

    if (GLB_OAM_TYPE_MPLS_PW == p_db_oam_session->oam_type)
    {
        CFG_INVALID_PARAM_RET("PW OAM does not support loopback\n");
    }

    if (TPOAM_MP_TYPE_MIP == p_db_oam_session->mp_type)
    {
        CFG_INVALID_PARAM_RET("Cannot start loopback test on MIP\n");
    }

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    //CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    //fp = sal_fopen(para->p_show->path, "w+");
    //if (NULL == fp)
    {
        //TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        //return PM_E_FAIL;
    }
    
    switch (pst_lbm_para->lb_sub_type)
    {
    case TPOAM_LB_MEP:
        rc = _tpoam_cli_lb_mep(p_db_oam_session, pst_lbm_para, para);
        break;
    case TPOAM_LB_MIP:
        rc = _tpoam_cli_lb_mip(p_db_oam_session, pst_lbm_para, para);
        break;
    case TPOAM_LB_DISCOVERY:
        rc = _tpoam_cli_lb_discovery(p_db_oam_session, pst_lbm_para, para);
        break;
    default:
        break;
    }
    return rc;
}

/*cdbctl update/cdb/opm/exec_tpoam/loopback_rmep/${rmep_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${data_tlv_length}*/
/*cdbctl update/cdb/opm/exec_tpoam/loopback_rmep/${rmep_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${test_tlv_length}/${pattern_type}*/
int32
_tpoam_cmd_exec_lb_rmep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tpoam_lbm_para_t lbm_para;
    uint32 oam_session_id = 0;

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    sal_memset(&lbm_para, 0, sizeof(lbm_para));
    lbm_para.lb_sub_type = TPOAM_LB_MEP;
    lbm_para.time_out = TPOAM_Y1731_LBM_TIMEOUT_DEF;
    lbm_para.repeat_cnt = TPOAM_Y1731_LBM_TX_CNT_DEF;
    lbm_para.is_req_mep_id = FALSE;
    lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_END_TLV;
    lbm_para.opt_tlv_len = 0;
    lbm_para.ttl = TPOAM_TTL_DEF;
    lbm_para.exp = TPOAM_EXP_DEF;

    PM_CFG_GET_UINT(lbm_para.rmep_id, argv[1], para->p_rs);
    PM_CFG_GET_UINT(oam_session_id, argv[2], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.ttl, argv[3], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.exp, argv[4], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.repeat_cnt, argv[5], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.time_out, argv[6], para->p_rs);
    if (0 != sal_strcmp(argv[7], "not"))
    {
        lbm_para.is_req_mep_id = TRUE;
    }

    /*data-tlv*/
    if (9 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_DATA_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[8], para->p_rs);
    }
    /*test tlv*/
    else if  (10 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_TEST_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[8], para->p_rs);
        /*TPOAM_TEST_MODE_3_PRBS_WI_CRC*/
        PM_CFG_GET_UINT(lbm_para.test_tlv_mode, argv[9], para->p_rs);
    }
    /*not configured*/
    else
    {
    
    }

    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    /*for show mpls-tp oam-y1731 loobback*/
    sal_memcpy(&p_tpoam_exec_global->exec_lb_rmep.para, &lbm_para, sizeof(lbm_para));
    
    rc = _tpoam_exe_lb(oam_session_id, &lbm_para, para);

    return rc;
}

/*cdbctl update/cdb/opm/exec_tpoam/loopback_mip/${node_id_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}*/
/*cdbctl update/cdb/opm/exec_tpoam/loopback_mip/${node_id_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${data_tlv_length}*/
/*cdbctl update/cdb/opm/exec_tpoam/loopback_mip/${node_id_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${test_tlv_length}/${pattern_type}*/
int32
_tpoam_cmd_exec_lb_mip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tpoam_lbm_para_t lbm_para;
    uint32 oam_session_id = 0;
    addr_ipv4_t node_id = {0};

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    sal_memset(&lbm_para, 0, sizeof(lbm_para));
    lbm_para.lb_sub_type = TPOAM_LB_MIP;
    lbm_para.time_out = TPOAM_Y1731_LBM_TIMEOUT_DEF;
    lbm_para.repeat_cnt = TPOAM_Y1731_LBM_TX_CNT_DEF;
    lbm_para.is_req_mep_id = FALSE;
    lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_END_TLV;
    lbm_para.opt_tlv_len = 0;
    //lbm_para.ttl = TPOAM_TTL_DEF;
    lbm_para.exp = TPOAM_EXP_DEF;

    //PM_CFG_GET_UINT(lbm_para.rmep_id, argv[1], para->p_rs);
    //node id
    CFG_PARSER_IP_ADDR(argv[1], &node_id, para->p_rs);
    lbm_para.node_id = node_id.s_addr;
                
    PM_CFG_GET_UINT(oam_session_id, argv[2], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.ttl, argv[3], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.exp, argv[4], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.repeat_cnt, argv[5], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.time_out, argv[6], para->p_rs);
    if (0 != sal_strcmp(argv[7], "not"))
    {
        lbm_para.is_req_mep_id = TRUE;
    }

    /*data-tlv*/
    if (9 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_DATA_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[8], para->p_rs);
    }
    /*test tlv*/
    else if  (10 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_TEST_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[8], para->p_rs);
        /*TPOAM_TEST_MODE_3_PRBS_WI_CRC*/
        PM_CFG_GET_UINT(lbm_para.test_tlv_mode, argv[9], para->p_rs);
    }
    /*not configured*/
    else
    {
    
    }
    
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    /*for show mpls-tp oam-y1731 loobback*/
    sal_memcpy(&p_tpoam_exec_global->exec_lb_mip.para, &lbm_para, sizeof(lbm_para));
    
    rc = _tpoam_exe_lb(oam_session_id, &lbm_para, para);

    return rc;
}

/*cdbctl update/cdb/opm/exec_tpoam/loopback_discovery/${from_ttl_value}/${session_id}/${to_ttl_value}/${exp_value}/${timeout_value}/${request-mepid-tlv}*/
/*cdbctl update/cdb/opm/exec_tpoam/loopback_discovery/${from_ttl_value}/${session_id}/${to_ttl_value}/${exp_value}/${timeout_value}/${request-mepid-tlv}/${data_tlv_length}*/
/*cdbctl update/cdb/opm/exec_tpoam/loopback_discovery/${from_ttl_value}/${session_id}/${to_ttl_value}/${exp_value}/${timeout_value}/${request-mepid-tlv}/${test_tlv_length}/${pattern_type}*/
int32
_tpoam_cmd_exec_lb_discovery(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tpoam_lbm_para_t lbm_para;
    uint32 oam_session_id = 0;

    int32 rc = PM_E_NONE;

    if (NULL == para)
    {
        TPOAM_LOG_ERR("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }

    sal_memset(&lbm_para, 0, sizeof(lbm_para));
    lbm_para.lb_sub_type = TPOAM_LB_DISCOVERY;
    lbm_para.time_out = TPOAM_Y1731_LBM_TIMEOUT_DEF;
    lbm_para.repeat_cnt = TPOAM_Y1731_LBM_TX_CNT_DEF;
    lbm_para.is_req_mep_id = FALSE;
    lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_END_TLV;
    lbm_para.opt_tlv_len = 0;
    //lbm_para.ttl = TPOAM_TTL_DEF;
    lbm_para.exp = TPOAM_EXP_DEF;

    //PM_CFG_GET_UINT(lbm_para.rmep_id, argv[1], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.ttl_end, argv[3], para->p_rs);
    
    PM_CFG_GET_UINT(oam_session_id, argv[2], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.ttl, argv[1], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.exp, argv[4], para->p_rs);
    //PM_CFG_GET_UINT(lbm_para.repeat_cnt, argv[5], para->p_rs);
    PM_CFG_GET_UINT(lbm_para.time_out, argv[5], para->p_rs);
    if (0 != sal_strcmp(argv[6], "not"))
    {
        lbm_para.is_req_mep_id = TRUE;
    }

    if (lbm_para.ttl > lbm_para.ttl_end)
    {
        CFG_INVALID_PARAM_RET("From TTL must <= To TTL\n");
    }

    /*data-tlv*/
    if (8 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_DATA_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[7], para->p_rs);
    }
    /*test tlv*/
    else if  (9 == argc)
    {
        lbm_para.opt_tlv_type = TPOAM_Y1731_PDU_TEST_TLV;
        PM_CFG_GET_UINT(lbm_para.opt_tlv_len, argv[7], para->p_rs);
        /*TPOAM_TEST_MODE_3_PRBS_WI_CRC*/
        PM_CFG_GET_UINT(lbm_para.test_tlv_mode, argv[8], para->p_rs);
    }
    /*not configured*/
    else
    {
    
    }

    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();

    /*for show mpls-tp oam-y1731 loobback*/
    sal_memcpy(&p_tpoam_exec_global->exec_lb_discovery.para, &lbm_para, sizeof(lbm_para));
    
    rc = _tpoam_exe_lb(oam_session_id, &lbm_para, para);

    return rc;
}

#endif

int32
tpoam_cmd_process_exec(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    
    switch (para->oper)
    {
    case CDB_OPER_SET:

        if (0 == sal_strcmp(argv[0], "lmm"))/* mpls-tp oam-y1731 lmm *//*/lmm/${service_type}/${part}/${repeat_value}/${part2}*/
        {
            _tpoam_cmd_exec_lmm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "1dm"))/* mpls-tp oam-y1731 lmm *//*/1dm/${service_type}/${part}/${repeat_value}/${part2}/${exp_value}*/
        {
            _tpoam_cmd_exec_dm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "dmm"))/* mpls-tp oam-y1731 lmm *//*/dmm/${service_type}/${part}/${repeat_value}/${part2}/${exp_value}*/
        {
            _tpoam_cmd_exec_dm(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "1dm_stop"))/* mpls-tp oam-y1731 1dm stop *//*/1dm_stop/${service_type}/${part}*/
        {
            _tpoam_cmd_exec_dm_stop(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "dmm_stop"))/* mpls-tp oam-y1731 dmm stop *//*/dmm_stop/${service_type}/${part}*/
        {
            _tpoam_cmd_exec_dm_stop(argv, argc, para);
        }
        /* mpls-tp oam-y1731 loopback rmep */
        /*cdbctl update/cdb/opm/exec_tpoam/loopback_rmep/${rmep_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${data_tlv_length}*/
        /*cdbctl update/cdb/opm/exec_tpoam/loopback_rmep/${rmep_value}/${session_id}/${ttl_value}/${exp_value}/${repeat_value}/${timeout_value}/${request-mepid-tlv}/${test_tlv_length}/${pattern_type}*/
        else if (0 == sal_strcmp(argv[0], "loopback_rmep"))
        {
            rc = _tpoam_cmd_exec_lb_rmep(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "loopback_mip"))
        {
            rc = _tpoam_cmd_exec_lb_mip(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "loopback_discovery"))
        {
            rc = _tpoam_cmd_exec_lb_discovery(argv, argc, para);
        }
        else if (0 == sal_strcmp(argv[0], "clean"))/*press ctrl+c when lb*/
        {
            uint32 oam_session_id = 0;
            tbl_oam_session_t *p_db_oam_session = NULL;
            tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
            
            oam_session_id = p_tpoam_exec_global->oam_session_id;
            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(oam_session_id);
            if (p_db_oam_session)
            {
                tpoam_lb_clean(p_db_oam_session);
            }
        }
        else if (0 == sal_strcmp(argv[0], "loopback_clear"))
        {
            uint32 oam_session_id = 0;
            tbl_oam_session_t *p_db_oam_session = NULL;
            tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
            
            oam_session_id = p_tpoam_exec_global->oam_session_id;
            p_db_oam_session = tbl_oam_session_get_oam_session_by_session_id(oam_session_id);
            if (p_db_oam_session)
            {
                tpoam_lb_clean(p_db_oam_session);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    //TPOAM_CLI_ERROR_RETURN(rc);
    return rc;
}
#endif

static int32
_tpoam_exec_global_cmd_get_tpoam_exec_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_tpoam_exec_global_t* p_db_tpoam_exec_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    p_db_tpoam_exec_glb = tbl_tpoam_exec_global_get_tpoam_exec_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_tpoam_exec_global_dump_one(p_db_tpoam_exec_glb, &args);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
tpoam_cmd_process_tpoam_exec_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_TPOAM_EXEC_GLOBAL);
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _tpoam_exec_global_cmd_get_tpoam_exec_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    TPOAM_CLI_ERROR_RETURN(rc);
    return PM_E_NONE;
}

static int32
_adpt_tpoam_cmd_get_oam_session_upload(tbl_oam_session_upload_t* p_db_oam_session_upload, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_oam_session_upload_master_t *p_master = tbl_oam_session_upload_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == p_db_oam_session_upload)
    {
        CTCLIB_SLIST_LOOP(p_master->oam_session_list, p_db_oam_session_upload, listnode)
        {
            tbl_oam_session_upload_dump_one(p_db_oam_session_upload, &args);
        }
    }
    else
    {
        tbl_oam_session_upload_dump_one(p_db_oam_session_upload, &args);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
tpoam_cmd_process_oam_session_upload(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    p_node = cdb_get_tbl(TBL_OAM_SESSION_UPLOAD);

    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;
    tbl_oam_session_upload_key_t oam_session_upload_key;
    
    uint32 oam_session_id = 0;

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;
    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        if (0 == argc)
        {
            oam_session_id = 0;
        }
        else if (argc >= 1)
        {
            PM_CFG_GET_UINT(oam_session_id, argv[0], para->p_rs);
            if (oam_session_id <= 0)
            {
                CFG_CONFLICT_RET("Invalid oam session id");
            }
            
            /* check exist */
            sal_memset(&oam_session_upload_key, 0, sizeof(oam_session_upload_key));
            oam_session_upload_key.session_id= oam_session_id;
            p_db_oam_session_upload = tbl_oam_session_upload_get_oam_session_upload(&oam_session_upload_key);
            if (!p_db_oam_session_upload)
            {
                CFG_CONFLICT_RET("Oam session upload %d not found", oam_session_id);
            }
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _adpt_tpoam_cmd_get_oam_session_upload(p_db_oam_session_upload, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    TPOAM_CLI_ERROR_RETURN(rc);
    return rc;
}


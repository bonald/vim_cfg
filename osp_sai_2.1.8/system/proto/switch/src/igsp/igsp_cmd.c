
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "igsp_api.h"
#include "brg_fdb.h"

static int32
_igsp_cmd_global_get(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_igsp_global_t *p_igsp_glb = tbl_igsp_global_get_igsp_global();
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;    
    tbl_igsp_global_dump_one(p_igsp_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_global_set_enable(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 vid = 0;
    uint32 static_mrouter_count = 0;

    /* 1. config check */
    if (!p_glb->enable)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            static_mrouter_count = igsp_api_intf_get_static_mrouter_count(p_db_igsp_if);
            if (static_mrouter_count)
            {
                CFG_CONFLICT_RET("Can't disable IGMP snooping on the VLAN which has configured static mrouter port");
            }
        }
    }
    else
    {
        /* modified by wangjj for fix erps bug 41473, 2016-12-03 */
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            if (igsp_api_get_vlan_erps_enable(vid))
            {
                CFG_CONFLICT_RET("Vlan %d is used as ERPS control VLAN, can not enable igmp snooping", vid);
            }
        }
    }

    /* 2. check duplicate config */
    /* global enable should update vlan's enable, then should not check duplicate config */

    /* 3. call API */
    if (p_glb->enable)
    {
        return igsp_api_global_enable();
    }
    else
    {
        return igsp_api_global_disable();
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_global_set_fast_leave(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_fast_leave(p_glb);
}

static int32
_igsp_cmd_global_set_discard_unknown(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_discard_unknown(p_glb);
}

static int32
_igsp_cmd_global_set_report_suppress(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    /* Bug53992 add check for report-suppression enable when IGMPv3 */
    if (IGMP_VERSION_3 == p_db_glb->version && TRUE == p_glb->report_suppress)
    {
        CFG_CONFLICT_RET("Report suppression not supported in version 3");
    }

    /* 2. check duplicate config */
    /* Delete by kcao for bug 47422 2018-06-14, not check duplicate value for need to update port's version */
    
    /* 3. call API */
    return igsp_api_global_set_report_suppress(p_glb);
}

static int32
_igsp_cmd_global_set_host_join(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */

    /* 3. call API */
    return igsp_api_global_set_host_join(p_glb, p_db_glb);
}

static int32
_igsp_cmd_global_set_version(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    /* Delete by kcao for bug 47425 2018-06-14, not check duplicate value for need to update port's version */
    
    /* 3. call API */
    return igsp_api_global_set_version(p_glb, p_db_glb);
}

static int32
_igsp_cmd_global_set_querier_max_response_time(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_glb->querier_max_response_time >= p_db_glb->query_interval)
    {
        CFG_CONFLICT_RET("Query Response Interval should be less than Query Interval");
    }
    
    /* 2. check duplicate config */
    /* Delete by kcao for bug 47432 2018-06-14, not check duplicate value for need to update port's version */
    
    /* 3. call API */
    return igsp_api_global_set_querier_max_response_time(p_glb);
}

static int32
_igsp_cmd_global_set_query_interval(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_glb->query_interval <= p_db_glb->querier_max_response_time)
    {
        CFG_CONFLICT_RET("Query interval should be greater than Query Response Interval");
    }
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_query_interval(p_glb);
}

static int32
_igsp_cmd_global_set_lmqc(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_lmqc(p_glb);
}

static int32
_igsp_cmd_global_set_lmqi(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_lmqi(p_glb);
}

static int32
_igsp_cmd_global_set_robustness_var(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_robustness_var(p_glb);
}

static int32
_igsp_cmd_global_set_max_member_number(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_db_glb->curr_group_member > p_glb->max_member_number)
    {
        CFG_CONFLICT_RET("Total current group number %u large than limitation", p_db_glb->curr_group_member);
    }
#ifdef GREATBELT
    if (IGMP_LIMIT_DEF < p_glb->max_member_number)
    {
        CFG_CONFLICT_RET("Max member number can not large than %d on this product", IGMP_LIMIT_DEF);
    }
#endif
    
    /* 2. check duplicate config */
    if (p_glb->max_member_number == p_db_glb->max_member_number)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_global_set_max_member_number(p_glb);
}

static int32
_igsp_cmd_global_set_global_src(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    return igsp_api_global_set_global_src(p_glb);
}

static int32
_igsp_cmd_global_set_tcn_enable(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->tcn_enable == p_db_glb->tcn_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_global_set_tcn_enable(p_glb);
}

static int32
_igsp_cmd_global_set_tcn_query_count(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->tcn_query_count == p_db_glb->tcn_query_count)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_global_set_tcn_query_count(p_glb);
}

static int32
_igsp_cmd_global_set_tcn_query_interval(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->tcn_query_interval == p_db_glb->tcn_query_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_global_set_tcn_query_interval(p_glb);
}

static int32
_igsp_cmd_global_set_tcn_query_max_response_time(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->tcn_query_max_response_time == p_db_glb->tcn_query_max_response_time)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_global_set_tcn_query_max_response_time(p_glb);
}

int32
igsp_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 i = 0;
    uint32 value = 0;
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_global_t glb;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IGSP_GLOBAL);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:

        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
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
        case TBL_IGSP_GLOBAL_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.enable = value;
            rc = _igsp_cmd_global_set_enable(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_FAST_LEAVE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.fast_leave = value;
            rc = _igsp_cmd_global_set_fast_leave(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.discard_unknown = value;
            rc = _igsp_cmd_global_set_discard_unknown(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.report_suppress = value;
            rc = _igsp_cmd_global_set_report_suppress(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_VERSION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.version = value;
            rc = _igsp_cmd_global_set_version(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.querier_max_response_time = value;
            rc = _igsp_cmd_global_set_querier_max_response_time(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.query_interval = value;
            rc = _igsp_cmd_global_set_query_interval(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_LMQC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.lmqc = value;
            rc = _igsp_cmd_global_set_lmqc(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_LMQI:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.lmqi = value;
            rc = _igsp_cmd_global_set_lmqi(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.robustness_var = value;
            rc = _igsp_cmd_global_set_robustness_var(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.max_member_number = value;
            rc = _igsp_cmd_global_set_max_member_number(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (2 == argc)
            {
                cdb_addr_ipv4_str2val(&glb.global_src, argv[i+1], sal_strlen(argv[i+1]));
                /* check sender ip validation */
                if (IN_BADCLASS(sal_ntoh32(glb.global_src.s_addr))  ||
                    IN_MULTICAST(sal_ntoh32(glb.global_src.s_addr)) ||
                    IN_LOOPBACK(sal_ntoh32(glb.global_src.s_addr))  ||
                    IPV4_NET0(sal_ntoh32(glb.global_src.s_addr)))
                {
                    CFG_CONFLICT_RET("Invalid IP address %s", argv[i+1]);
                }
            }
            else
            {
                glb.global_src.s_addr = sal_hton32(INADDR_QUERY_ADDR);;
            }
            rc = _igsp_cmd_global_set_global_src(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_TCN_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.tcn_enable = value;
            rc = _igsp_cmd_global_set_tcn_enable(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.tcn_query_count = value;
            rc = _igsp_cmd_global_set_tcn_query_count(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.tcn_query_interval = value;
            rc = _igsp_cmd_global_set_tcn_query_interval(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.tcn_query_max_response_time = value;
            rc = _igsp_cmd_global_set_tcn_query_max_response_time(&glb, p_db_glb, para);
            break;

        case TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.host_join_enable = value;
            rc = _igsp_cmd_global_set_host_join(&glb, p_db_glb, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _igsp_cmd_global_get(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_igsp_cmd_intf_set_acl_name(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (sal_strlen(p_igsp_if->acl_name))
    {
        rc = check_name_character_and_len(p_igsp_if->acl_name, ACL_NAME_SIZE);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid ACL name");
        }
    }
    
    /* 2. check duplicate config */
    if (0 == sal_strcmp(p_igsp_if->acl_name, p_db_igsp_if->acl_name))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_acl_name(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_enable(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    uint32 static_mrouter_count = 0;
    uint32 has_mcfdb = FALSE;
    tbl_vlan_t *p_db_vlan = NULL;
    
    /* 1. config check */
    if (!p_igsp_if->enable)
    {
        static_mrouter_count = igsp_api_intf_get_static_mrouter_count(p_db_igsp_if);
        if (static_mrouter_count)
        {
            CFG_CONFLICT_RET("Can't disable IGMP snooping on the VLAN which has configured static mrouter port");
        }
    }
    else
    {
        has_mcfdb = brg_mcfdb_vlan_has_manual_mcfdb(p_igsp_if->key.vid);
        if (has_mcfdb)
        {
            CFG_CONFLICT_RET("There are statically configured mcast entries on the vlan. IGMP Snooping can't be enabled");
        }

        /* modified by wangjj for fix erps bug 41473, 2016-12-03 */
        if (igsp_api_get_vlan_erps_enable(p_igsp_if->key.vid))
        {
            CFG_CONFLICT_RET("Vlan %d is used as ERPS control VLAN, can not enable igmp snooping", p_igsp_if->key.vid);
        }

        p_db_vlan = tbl_vlan_get_vlan((tbl_vlan_key_t *)(&p_igsp_if->key.vid));
        if (p_db_vlan && p_db_vlan->g8032_ring_id)
        {
            CFG_CONFLICT_RET("Vlan %d is used as G8032 control VLAN, can not enable igmp snooping", p_igsp_if->key.vid);
        }
    }

    /* 2. check duplicate config */
    if (p_igsp_if->enable == p_db_igsp_if->enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    if (p_igsp_if->enable)
    {
        return igsp_api_intf_enable(p_igsp_if);
    }
    else
    {
        return igsp_api_intf_disable(p_igsp_if);
    }
}

static int32
_igsp_cmd_intf_set_version(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->version == p_db_igsp_if->version)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_version(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_fast_leave(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->fast_leave == p_db_igsp_if->fast_leave)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_fast_leave(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_lmqi(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->lmqi == p_db_igsp_if->lmqi)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_lmqi(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_lmqc(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->lmqc == p_db_igsp_if->lmqc)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_lmqc(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_max_member_number(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_db_igsp_if->curr_group_member > p_igsp_if->max_member_number)
    {
        CFG_CONFLICT_RET("VLAN %u current group number %u large than limitation", p_db_igsp_if->key.vid, p_db_igsp_if->curr_group_member);
    }

#ifdef GREATBELT
    if (IGMP_LIMIT_DEF < p_igsp_if->max_member_number)
    {
        CFG_CONFLICT_RET("Max member number can not large than %d on this product", IGMP_LIMIT_DEF);
    }
#endif
    
    /* 2. check duplicate config */
    if (p_igsp_if->max_member_number == p_db_igsp_if->max_member_number)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_max_member_number(p_igsp_if);
}


static int32
_igsp_cmd_intf_set_robustness_var(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->robustness_var == p_db_igsp_if->robustness_var)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_robustness_var(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_discard_unknown(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->discard_unknown == p_db_igsp_if->discard_unknown)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_discard_unknown(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_report_suppress(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (IGMP_VERSION_3 == p_db_igsp_if->version)
    {
        if (p_igsp_if->report_suppress)
        {
            CFG_CONFLICT_RET("Report suppression not supported in version 3");
        }
    }
    
    /* 2. check duplicate config */
    if (p_igsp_if->report_suppress == p_db_igsp_if->report_suppress)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_report_suppress(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_querier_enable(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->querier_enable == p_db_igsp_if->querier_enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    igsp_api_intf_set_querier_enable(p_igsp_if);

    if (p_igsp_if->querier_enable)
    {
        CFG_PROMPT_RET("Warning: Please confirm that no other querier is configured on the network, this command may cause querier re-election");
    }
    return PM_E_NONE;
}

static int32
_igsp_cmd_intf_set_query_interval(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_igsp_if->query_interval <= p_db_igsp_if->querier_max_response_time)
    {
        CFG_CONFLICT_RET("Query interval should be greater than Query Response Interval");
    }
    
    /* 2. check duplicate config */
    if (p_igsp_if->query_interval == p_db_igsp_if->query_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_query_interval(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_admin_other_query_interval(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->admin_other_query_interval == p_db_igsp_if->admin_other_query_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_admin_other_query_interval(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_querier_max_response_time(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    if (p_igsp_if->querier_max_response_time >= p_db_igsp_if->query_interval)
    {
        CFG_CONFLICT_RET("Query Response Interval should be less than Query Interval");
    }
    
    /* 2. check duplicate config */
    if (p_igsp_if->querier_max_response_time == p_db_igsp_if->querier_max_response_time)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_querier_max_response_time(p_igsp_if);
}

static int32
_igsp_cmd_intf_set_querier_config_addr(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    /* 2. check duplicate config */
    if (p_igsp_if->querier_config_address.s_addr == p_db_igsp_if->querier_config_address.s_addr)
    {
        return PM_E_NONE;
    }
    /* 3. call API */
    return igsp_api_intf_set_querier_config_addr(p_igsp_if,p_db_igsp_if);
}

static int32
_igsp_cmd_intf_set_mrouter_aging_interval(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_igsp_if->mrouter_aging_interval == p_db_igsp_if->mrouter_aging_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    return igsp_api_intf_set_mrouter_aging_interval(p_igsp_if);
}

static int32
_igsp_cmd_intf_add_mrouter_port(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, 
                                char *ifname, cfg_cmd_para_t *para)
{
    char                ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    ds_mrouter_port_t mr_port;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    
    /* 1. config check */
    IFNAME_ETH2FULL(ifname, ifname_ext);
    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not found", ifname_ext);
    }

    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        CFG_CONFLICT_RET("Cannot configure mirror destination port");
    }
    
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        CFG_CONFLICT_RET("Interface %s is not switchport", ifname_ext);
    }

    if (!GLB_BMP_ISSET(p_db_brgif->allowed_vlan.vlan_bmp, p_db_igsp_if->key.vid))
    {
        CFG_CONFLICT_RET("Interface %s is not belong to vlan %u", ifname_ext, p_db_igsp_if->key.vid);
    }

    /* 2. check duplicate config */
    sal_memset(&mr_port, 0, sizeof(mr_port));
    sal_strcpy(mr_port.key.name, ifname);
    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_db_igsp_if, &mr_port);
    if (p_db_mr_port)
    {
        if (IGMP_MROUTE_STATIC == p_db_mr_port->type)
        {
            return PM_E_NONE;
        }
        else
        {
            igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);
        }
    }

    mr_port.type = IGMP_MROUTE_STATIC;
    mr_port.uptime = ctc_time_boottime_sec(NULL);
    mr_port.expire_timer = NULL;
        
    /* 3. call API */
    return igsp_api_intf_add_static_mrouter_port(p_db_igsp_if, &mr_port);
}

static int32
_igsp_cmd_intf_del_mrouter_port(tbl_igsp_intf_t *p_igsp_if, tbl_igsp_intf_t *p_db_igsp_if, 
                                char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    ds_mrouter_port_t mr_port;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    
    /* 1. config check */

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not found", ifname);
    }
    
    /* 2. check duplicate config */
    sal_memset(&mr_port, 0, sizeof(mr_port));
    sal_strcpy(mr_port.key.name, ifname);
    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_db_igsp_if, &mr_port);
    if (NULL == p_db_mr_port)
    {
        return PM_E_NONE;
    }

    if (IGMP_MROUTE_DYNAMIC == p_db_mr_port->type)
    {
        CFG_CONFLICT_RET("Cannot delete dynamic mrouter port %s", ifname);
    }
    
    /* 3. call API */
    return igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);
}

static int32
_igsp_cmd_add_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_group, cfg_cmd_para_t *para)
{
    ds_join_group_t *p_db_group = NULL;
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    /* 2. check duplicate config */
    p_db_group = ds_join_group_get_join_group(p_db_igsp_if, p_group);
    if (p_db_group)
    {
        return PM_E_NONE;
    }

    if (p_glb->host_joined_num >= IGMP_HOST_JOIN_GROUP_MAX)
    {
        CFG_CONFLICT_RET("IGMP host-joined group exceed max number %d", IGMP_HOST_JOIN_GROUP_MAX);
    }

    PM_E_RETURN(igsp_api_add_join_group(p_db_igsp_if, p_group));
    p_glb->host_joined_num++;
    PM_E_RETURN(tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM));

    return rc;
}

static int32
_igsp_cmd_del_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_group, cfg_cmd_para_t *para)
{
    ds_join_group_t *p_db_group = NULL;
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    p_db_group = ds_join_group_get_join_group(p_db_igsp_if, p_group);
    if (NULL == p_db_group)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(igsp_api_del_join_group(p_db_igsp_if, p_db_group));

    return rc;
}

static int32
_igsp_cmd_intf_check_exist(vid_t vid, cdb_node_t* p_node, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_igsp_intf_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;

    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (!p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u not exist", vid);
    }
    
    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (!p_db_igsp_if)
    {
        CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
    }
    /* modified by wangjj for fix erps bug 41473, 2016-12-03 */
    else
    {
        if (igsp_api_get_vlan_erps_enable(vid))
        {
            CFG_CONFLICT_RET("Vlan %d is used as ERPS control VLAN, can not enable igmp snooping", vid);
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_intf_get(vid_t vid, cdb_node_t* p_node, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (GLB_VLAN_NONE == vid)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_igsp_intf_dump_one, &args);
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        tbl_igsp_intf_dump_one(p_db_igsp_if, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
igsp_cmd_process_intf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    vid_t vid = GLB_VLAN_NONE;
    uint32 value = 0;
    tbl_igsp_intf_t igsp_if;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 i = 0;
    ds_join_group_t group;
    char *ifname = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IGSP_INTF);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        i = 0;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        i++;
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        PM_CFG_GET_UINT_RANGE(value, GLB_DEF_VLAN_ID, GLB_MAX_VLAN_ID-1, argv[i], para->p_rs);
        i++;
        igsp_if.key.vid = value;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&igsp_if.key);
        if (NULL == p_db_igsp_if)
        {
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", igsp_if.key.vid);
        }
            
        switch (field_id)
        {
        case TBL_IGSP_INTF_FLD_ACL_NAME:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            if (3 == argc)
            {
                sal_strncpy(igsp_if.acl_name, argv[i], ACL_NAME_FULL_NAME_SIZE+1);
            }
            else
            {
                sal_memset(igsp_if.acl_name, 0, sizeof(igsp_if.acl_name));
            }
            rc = _igsp_cmd_intf_set_acl_name(&igsp_if, p_db_igsp_if, para);
            if(3 == argc)
            {
                CFG_PROMPT_RET("Info: Only source and destination IP address(including mapped mac-da) take effect on dynamic entries.");
            }
            break;
            
        case TBL_IGSP_INTF_FLD_ENABLE:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.enable = value;
            rc = _igsp_cmd_intf_set_enable(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_VERSION:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.version = value;
            rc = _igsp_cmd_intf_set_version(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_FAST_LEAVE:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.fast_leave = value;
            rc = _igsp_cmd_intf_set_fast_leave(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_LMQI:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.lmqi = value;
            rc = _igsp_cmd_intf_set_lmqi(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_LMQC:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.lmqc = value;
            rc = _igsp_cmd_intf_set_lmqc(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.max_member_number = value;
            rc = _igsp_cmd_intf_set_max_member_number(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.robustness_var= value;
            rc = _igsp_cmd_intf_set_robustness_var(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.discard_unknown = value;
            rc = _igsp_cmd_intf_set_discard_unknown(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_REPORT_SUPPRESS:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.report_suppress = value;
            rc = _igsp_cmd_intf_set_report_suppress(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_QUERIER_ENABLE:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.querier_enable = value;
            rc = _igsp_cmd_intf_set_querier_enable(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_QUERY_INTERVAL:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.query_interval = value;
            rc = _igsp_cmd_intf_set_query_interval(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.admin_other_query_interval = value;
            rc = _igsp_cmd_intf_set_admin_other_query_interval(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.querier_max_response_time = value;
            rc = _igsp_cmd_intf_set_querier_max_response_time(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            if (3 == argc)
            {
                cdb_addr_ipv4_str2val(&igsp_if.querier_config_address, argv[i], sal_strlen(argv[i]));
                /* check sender ip validation */
                if (IN_BADCLASS(sal_ntoh32(igsp_if.querier_config_address.s_addr))  ||
                    IN_MULTICAST(sal_ntoh32(igsp_if.querier_config_address.s_addr)) ||
                    IN_LOOPBACK(sal_ntoh32(igsp_if.querier_config_address.s_addr))  ||
                    IPV4_NET0(sal_ntoh32(igsp_if.querier_config_address.s_addr)))
                {
                    CFG_CONFLICT_RET("Invalid IP address %s", argv[i]);
                }
            }
            else
            {
                igsp_if.querier_config_address.s_addr = 0;
            }
            rc = _igsp_cmd_intf_set_querier_config_addr(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i], para->p_rs);
            igsp_if.mrouter_aging_interval = value;
            rc = _igsp_cmd_intf_set_mrouter_aging_interval(&igsp_if, p_db_igsp_if, para);
            break;

        case TBL_IGSP_INTF_FLD_MROUTER_LIST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            ifname = argv[i+1];
            if (0 == sal_strcmp("add", argv[i]))
            {
                rc = _igsp_cmd_intf_add_mrouter_port(&igsp_if, p_db_igsp_if, ifname, para);
            
            }
            else if (0 == sal_strcmp("del", argv[i]))
            {
                rc = _igsp_cmd_intf_del_mrouter_port(&igsp_if, p_db_igsp_if, ifname, para);
            }
            else 
            {
                CFG_INVALID_PARAM_RET("Invalid action");
            }
            break;

        case TBL_IGSP_INTF_FLD_JOIN_LIST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            sal_memset(&group, 0, sizeof(group));
            cdb_addr_ipv4_str2val(&group.key.group, argv[i+1], sal_strlen(argv[i]));

            if ((sal_ntoh32(group.key.group.s_addr) <= INADDR_MAX_LOCAL_GROUP)
                    || !IN_MULTICAST(sal_ntoh32(group.key.group.s_addr)))
            {
                CFG_CONFLICT_RET("Invalid group address");
            }
            if (0 == sal_strcmp("add", argv[i]))
            {
                rc = _igsp_cmd_add_join_group(p_db_igsp_if, &group, para);
            
            }
            else if (0 == sal_strcmp("del", argv[i]))
            {
                rc = _igsp_cmd_del_join_group(p_db_igsp_if, &group, para);
            }
            else 
            {
                CFG_INVALID_PARAM_RET("Invalid action");
            }
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        if (0 == argc)
        {
            rc = _igsp_cmd_intf_get(vid, p_node, para, &field);
        }
        else if (1 <= argc)
        {
            PM_CFG_GET_UINT(value, argv[0], para->p_rs);
            vid = value;
            if ((2 == argc) && (0 == sal_strcmp("check_exist", argv[1])))
            {
                rc = _igsp_cmd_intf_check_exist(vid, p_node, para);
            }
            else
            {
                rc = _igsp_cmd_intf_get(vid, p_node, para, &field);
            }
        }
        else if (2 == argc)
        {
            
            PM_CFG_GET_UINT(value, argv[0], para->p_rs);
            vid = value;
            rc = _igsp_cmd_intf_get(vid, p_node, para, &field);
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_igsp_cmd_group_get(cdb_node_t* p_node, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_igsp_group_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_add_group(tbl_igsp_group_t *p_igsp_grp, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_igsp_intf_key_t key;
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;

    /* 1. config check */
    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        CFG_CONFLICT_RET("Interface %s is not physical or LAG port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* group should be valid */
    if ((sal_ntoh32(p_igsp_grp->key.ip.s_addr) <= INADDR_MAX_LOCAL_GROUP)
            || !IN_MULTICAST(sal_ntoh32(p_igsp_grp->key.ip.s_addr)))
    {
        CFG_CONFLICT_RET("Invalid group address");
    }
    
    key.vid = p_igsp_grp->key.vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        CFG_CONFLICT_RET("IGMP Snooping vlan %u not exist", key.vid);
    }
    
    vlan_key.vid = p_igsp_grp->key.vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("VLAN %u not exist", key.vid);
    }
    
    if (!GLB_BMP_ISSET(p_db_vlan->member_port, p_db_if->ifindex))
    {
        CFG_CONFLICT_RET("Interface is not VLAN %u member port", key.vid);
    }

    if (p_glb->curr_group_member >= p_glb->max_member_number)
    {
        CFG_CONFLICT_RET("Total current group number reached limitation %u", p_glb->max_member_number);
    }
    if (p_db_igsp_if->curr_group_member >= p_db_igsp_if->max_member_number)
    {
        CFG_CONFLICT_RET("VLAN %u current group number reached limitation %u", p_db_igsp_if->key.vid, p_db_igsp_if->max_member_number);
    }

    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        CFG_CONFLICT_RET("Cannot configure mirror destination port");
    }
    
    p_db_igsp_grp = tbl_igsp_group_get_igsp_group(&p_igsp_grp->key);
    if (p_db_igsp_grp)
    {
        if (IGMP_TYPE_STATIC == p_db_igsp_grp->type)
        {
            return PM_E_NONE;
        }
        else
        {
            igsp_api_group_del_dynamic(p_igsp_grp);
        }
    }

    igsp_api_group_add_static(p_igsp_grp, p_db_igsp_if);

    return PM_E_NONE;
}

static int32
_igsp_cmd_del_group(tbl_igsp_group_t *p_igsp_grp, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_igsp_group_t *p_db_igsp_grp = NULL;

    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        CFG_CONFLICT_RET("Interface %s is not physical or LAG port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    p_db_igsp_grp = tbl_igsp_group_get_igsp_group(&p_igsp_grp->key);
    if (NULL == p_db_igsp_grp)
    {
        return PM_E_NONE;
    }

    igsp_api_group_del_static(p_igsp_grp);

    return PM_E_NONE;
}


int32
igsp_cmd_process_group(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_igsp_group_t igsp_grp;
    tbl_interface_t *p_db_if = NULL;
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    uint32 value = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IGSP_GROUP);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
        sal_memset(&igsp_grp, 0, sizeof(igsp_grp));
        PM_CFG_GET_UINT(value, argv[0], para->p_rs);
        igsp_grp.key.vid = value;
        cdb_addr_ipv4_str2val(&igsp_grp.key.ip, argv[1], sal_strlen(argv[1]));
        p_db_if = tbl_interface_get_interface_by_name(argv[2]);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET("Invalid interface %s", argv[2]);
        }
        igsp_grp.key.ifindex = p_db_if->ifindex;
        rc = _igsp_cmd_add_group(&igsp_grp, p_db_if, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
        sal_memset(&igsp_grp, 0, sizeof(igsp_grp));
        PM_CFG_GET_UINT(value, argv[0], para->p_rs);
        igsp_grp.key.vid = value;
        cdb_addr_ipv4_str2val(&igsp_grp.key.ip, argv[1], sal_strlen(argv[1]));
        p_db_if = tbl_interface_get_interface_by_name(argv[2]);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET("Invalid interface %s", argv[2]);
        }
        igsp_grp.key.ifindex = p_db_if->ifindex;
        rc = _igsp_cmd_del_group(&igsp_grp, p_db_if, para);
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _igsp_cmd_group_get(p_node, para, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_basic_global(tbl_igsp_global_t *p_igsp_glb, FILE *fp)
{
    sal_fprintf(fp, "Global IGMP Snooping configuration:\n");
    sal_fprintf(fp, "------------------------------------+-----------\n");
    sal_fprintf(fp, "%-35s : %-10s\n", "Igmp Snooping", enabled_str(p_igsp_glb->enable));
    sal_fprintf(fp, "%-35s : %-10s\n", "Fast-Leave", enabled_str(p_igsp_glb->fast_leave));
    sal_fprintf(fp, "%-35s : %-10s\n", "Report-Suppression", enabled_str(p_igsp_glb->report_suppress));
    sal_fprintf(fp, "%-35s : %-10u\n", "Version", p_igsp_glb->version);
    sal_fprintf(fp, "%-35s : %-10u\n", "Robustness Variable", p_igsp_glb->robustness_var);
    sal_fprintf(fp, "%-35s : %-10u\n", "Max Member Number",  p_igsp_glb->max_member_number);
    sal_fprintf(fp, "%-35s : %-10s\n", "Unknown Multicast Behavior", p_igsp_glb->discard_unknown ? "Discard" : "Flood");
#ifdef OFPRODUCT
    sal_fprintf(fp, "%-35s : %-10s\n", "Host Join", p_igsp_glb->host_join_enable ? "Enabled" : "Disabled");
#endif
    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_querier_global(tbl_igsp_global_t *p_igsp_glb, FILE *fp)
{
    char glb_src_str[CMD_BUF_32];
    
    cdb_addr_ipv4_val2str(glb_src_str, CMD_BUF_32, &p_igsp_glb->global_src);

    sal_fprintf(fp, "Global Igmp Snooping Querier Configuration:\n");
    sal_fprintf(fp, "------------------------------------+-----------\n");
    sal_fprintf(fp, "%-35s : %-10u\n", "Version", p_igsp_glb->version);
    sal_fprintf(fp, "%-35s : %-10u\n", "Last Member Query Count", p_igsp_glb->lmqc);
    sal_fprintf(fp, "%-35s : %-10u\n", "Last Member Query Interval (msec)", p_igsp_glb->lmqi);
    sal_fprintf(fp, "%-35s : %-10u\n", "Max Query Response Time (sec)", p_igsp_glb->querier_max_response_time);
    sal_fprintf(fp, "%-35s : %-10u\n", "Query Interval (sec)", p_igsp_glb->query_interval);
    sal_fprintf(fp, "%-35s : %-10s\n", "Global Source Address", glb_src_str);
    sal_fprintf(fp, "%-35s : %-10s\n", "TCN Query", enabled_str(p_igsp_glb->tcn_enable));
    sal_fprintf(fp, "%-35s : %-10u\n", "TCN Query Count", p_igsp_glb->tcn_query_count);
    sal_fprintf(fp, "%-35s : %-10u\n", "TCN Query Interval (sec)", p_igsp_glb->tcn_query_interval);
    sal_fprintf(fp, "%-35s : %-10u\n", "TCN Query Max Respose Time (sec)", p_igsp_glb->tcn_query_max_response_time);
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_basic_intf(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp)
{
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];

    sal_fprintf(fp, "VLAN %u:\n", p_db_igsp_if->key.vid);
    sal_fprintf(fp, "--------\n");
    sal_fprintf(fp, "%-35s : %-10s\n", "Igmp Snooping", enabled_str(p_db_igsp_if->enable));
    sal_fprintf(fp, "%-35s : %-10s\n", "Fast-Leave", enabled_str(p_db_igsp_if->fast_leave));
    sal_fprintf(fp, "%-35s : %-10s\n", "Report-Suppression", enabled_str(p_db_igsp_if->report_suppress));
    sal_fprintf(fp, "%-35s : %-10u\n", "Version", p_db_igsp_if->version);
    sal_fprintf(fp, "%-35s : %-10u\n", "Robustness Variable", p_db_igsp_if->robustness_var);
    sal_fprintf(fp, "%-35s : %-10u\n", "Max Member Number",  p_db_igsp_if->max_member_number);
    sal_fprintf(fp, "%-35s : %-10s\n", "Unknown Multicast Behavior", p_db_igsp_if->discard_unknown ? "Discard" : "Flood");
    sal_fprintf(fp, "%-35s : %-10s\n", "Group Access-list", sal_strlen(p_db_igsp_if->acl_name) ? p_db_igsp_if->acl_name : "N/A");

    /* mrouter info */
    sal_fprintf(fp, "%-35s : %-10u\n", "Mrouter Port Aging Interval (sec)",  p_db_igsp_if->mrouter_aging_interval);
    sal_fprintf(fp, "%-35s : ", "Mrouter Port");
    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            IFNAME_ETH2FULL(p_db_mrouter_port->key.name, ifname_ext);
            if (IGMP_MROUTE_STATIC == p_db_mrouter_port->type)
            {
                sal_fprintf(fp, "%s(static) ", ifname_ext);
            }
            else
            {
                sal_fprintf(fp, "%s(dynamic) ", ifname_ext);
            }
        }
    }
    sal_fprintf(fp, "\n");

    
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_groups_intf_detail(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp, uint32 *banner)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char ipaddr_str[CMD_BUF_32];
    char last_reporter_str[CMD_BUF_32];        
    char uptime_str[CMD_BUF_32];
    char expires_str[CMD_BUF_32];
    char v1host_expires_str[CMD_BUF_32];
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    sal_time_t now;
    sal_time_t expires;
    uint32 i = 0;
    
    now = ctc_time_boottime_sec(NULL);
    
    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if (p_db_group->key.vid != p_db_igsp_if->key.vid)
            {
                continue;
            }
            tbl_interface_get_name_by_ifindex(p_db_group->key.ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2FULL(ifname, ifname_ext);
            cdb_addr_ipv4_val2str(ipaddr_str, CMD_BUF_32, &p_db_group->key.ip);
            
            uptime_str[0] = CMD_ZERO_CHAR;
            ctc_task_uptime_to_str(p_db_group->uptime, uptime_str, CMD_BUF_32);
            expires_str[0] = CMD_ZERO_CHAR;
            expires = p_db_group->liveness - (now - p_db_group->update_time);
            ctc_task_time_to_str(expires, expires_str, CMD_BUF_32);

            sal_fprintf(fp, "%-15s : %s\n", "Interface", ifname_ext);
            sal_fprintf(fp, "%-15s : %s\n", "Group", ipaddr_str);
            sal_fprintf(fp, "%-15s : %s\n", "Uptime", uptime_str);
            if (IGMP_TYPE_STATIC == p_db_group->type)
            {
                sal_fprintf(fp, "%-15s : %s\n", "Group mode", "Exclude");
                sal_fprintf(fp, "%-15s : %s\n", "Last reporter", "-");
            }
            else
            {
                cdb_addr_ipv4_val2str(last_reporter_str, CMD_BUF_32, &p_db_group->last_reporter_address);
                sal_fprintf(fp, "%-15s : %s (Expires: %s)\n", "Group mode", "Exclude", expires_str);
                sal_fprintf(fp, "%-15s : %s\n", "Last reporter", last_reporter_str);
            }
            if (TRUE == p_db_group->is_v1_host_exist)
            {
                v1host_expires_str[0] = CMD_ZERO_CHAR;
                expires = p_db_group->liveness - (now - p_db_group->v1_update_time);
                ctc_task_time_to_str(expires, v1host_expires_str, CMD_BUF_32);
                sal_fprintf(fp, "%-25s : %s\n", "V1 host present expires", v1host_expires_str);
            }
            else
            {
                sal_fprintf(fp, "%-25s : %s\n", "V2 host present expires", expires_str);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_groups_intf(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp, uint32 *banner)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char ipaddr_str[CMD_BUF_32];
    char uptime_str[CMD_BUF_32];
    char expires_str[CMD_BUF_32];
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    sal_time_t now;
    sal_time_t expires;
    uint32 i = 0;
    
    if (*banner)
    {
        sal_fprintf(fp, "%-8s %-14s %-16s %-14s %-14s\n", 
            "VLAN", "Interface", "Group-Address", "Uptime", "Expires-time");
        sal_fprintf(fp, "--------+--------------+----------------+--------------+--------------\n");
        *banner = FALSE;
    }
    now = ctc_time_boottime_sec(NULL);
    
    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if (p_db_group->key.vid != p_db_igsp_if->key.vid)
            {
                continue;
            }
            if (p_db_group->inactive)
            {
                continue;
            }
            tbl_interface_get_name_by_ifindex(p_db_group->key.ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2FULL(ifname, ifname_ext);
            cdb_addr_ipv4_val2str(ipaddr_str, CMD_BUF_32, &p_db_group->key.ip);
            
            uptime_str[0] = CMD_ZERO_CHAR;
            ctc_task_uptime_to_str(p_db_group->uptime, uptime_str, CMD_BUF_32);
            expires_str[0] = CMD_ZERO_CHAR;
            expires = p_db_group->liveness - (now - p_db_group->update_time);
            if (IGMP_TYPE_DYNAMIC == p_db_group->type)
            {
                ctc_task_time_to_str(expires, expires_str, CMD_BUF_32);
            }
            else
            {
                sal_sprintf(expires_str, "stopped");
            }
            
            sal_fprintf(fp, "%-8u %-14s %-16s %-14s %-14s\n", 
                p_db_group->key.vid,
                ifname_ext,
                ipaddr_str,
                uptime_str, 
                expires_str);
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_querier_intf(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp)
{
    char config_addr_str[CMD_BUF_32];
    char oper_querier_addr_str[CMD_BUF_32];
    char elected_querier_addr_str[CMD_BUF_32];

    config_addr_str[0] = CMD_ZERO_CHAR;
    oper_querier_addr_str[0] = CMD_ZERO_CHAR;
    elected_querier_addr_str[0] = CMD_ZERO_CHAR;
    
    if (p_db_igsp_if->querier_config_address.s_addr)
    {
        cdb_addr_ipv4_val2str(config_addr_str, CMD_BUF_32, &p_db_igsp_if->querier_config_address);
    }
    else
    {
        sal_strcpy(config_addr_str, "N/A");
    }

    cdb_addr_ipv4_val2str(oper_querier_addr_str, CMD_BUF_32, &p_db_igsp_if->querier_oper_address);
    if (p_db_igsp_if->querier_operate)
    {
        cdb_addr_ipv4_val2str(elected_querier_addr_str, CMD_BUF_32, &p_db_igsp_if->querier_oper_address);
    }
    else
    {
        cdb_addr_ipv4_val2str(elected_querier_addr_str, CMD_BUF_32, &p_db_igsp_if->other_querier_address);
    }
    cdb_addr_ipv4_val2str(oper_querier_addr_str, CMD_BUF_32, &p_db_igsp_if->querier_oper_address);
        
    sal_fprintf(fp, "VLAN %u:\n", p_db_igsp_if->key.vid);
    sal_fprintf(fp, "------------------------------------+-----------\n");
    sal_fprintf(fp, "%-35s : %-10s\n", "Elected querier is", elected_querier_addr_str);
    sal_fprintf(fp, "------------------------------------+-----------\n");
    sal_fprintf(fp, "%-35s : %-10s\n", "Admin State", enabled_str(p_db_igsp_if->querier_enable));
    sal_fprintf(fp, "%-35s : %-10u\n", "Admin Version", p_db_igsp_if->version);
    sal_fprintf(fp, "%-35s : %-10s\n", "Operational State", p_db_igsp_if->querier_operate ? "Querier" : "Non-Querier");
    sal_fprintf(fp, "%-35s : %-10s\n", "Querier Configure Address", config_addr_str);
    sal_fprintf(fp, "%-35s : %-10s\n", "Querier Operational Address", oper_querier_addr_str);
    sal_fprintf(fp, "%-35s : %-10u\n", "Last Member Query Count",  p_db_igsp_if->lmqc);
    sal_fprintf(fp, "%-35s : %-10u\n", "Last Member Query Interval (msec)",  p_db_igsp_if->lmqi);
    sal_fprintf(fp, "%-35s : %-10u\n", "Max Query Response Time (sec)", p_db_igsp_if->querier_max_response_time);
    sal_fprintf(fp, "%-35s : %-10u\n", "Query Interval (sec)", p_db_igsp_if->query_interval);
    sal_fprintf(fp, "%-35s : %-10u\n", "Querier Timeout (sec)", p_db_igsp_if->other_query_interval);
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_mrouter_intf(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp, uint32 *banner)
{
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];
    char                uptime_str[CMD_BUF_32];
    char                expires_str[CMD_BUF_32];
    int32 expires_sec = 0;
    
    if (*banner)
    {
        *banner = FALSE;
        sal_fprintf(fp, "%-8s %-16s %-8s %-15s %-15s\n", "VLAN", "Interface", "Mode", "Uptime", "Expires-time");
        sal_fprintf(fp, "--------+----------------+--------+---------------+---------------\n");   
    }

    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            if(FALSE == p_db_igsp_if->enable)
            {
                continue;
            }
            IFNAME_ETH2FULL(p_db_mrouter_port->key.name, ifname_ext);
            uptime_str[0] = CMD_ZERO_CHAR;
            expires_str[0] = CMD_ZERO_CHAR;
            ctc_task_uptime_to_str(p_db_mrouter_port->uptime, uptime_str, CMD_BUF_32);
            if (IGMP_MROUTE_STATIC == p_db_mrouter_port->type)
            {
                sal_fprintf(fp, "%-8u %-16s %-8s %-15s %-15s\n", 
                    p_db_igsp_if->key.vid, ifname_ext, "static", uptime_str, "-");
            }
            else
            {
                expires_sec = ctc_timer_get_remain_sec(p_db_mrouter_port->expire_timer);
                ctc_task_time_to_str((sal_time_t) expires_sec, expires_str, CMD_BUF_32);
                sal_fprintf(fp, "%-8u %-16s %-8s %-15s %-15s\n", 
                    p_db_igsp_if->key.vid, ifname_ext, "dynamic", uptime_str, expires_str);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_global_t *p_igsp_glb = tbl_igsp_global_get_igsp_global();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _igsp_cmd_act_do_show_basic_global(p_igsp_glb, fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_basic(vid_t vid, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_global_t *p_igsp_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _igsp_cmd_act_do_show_basic_global(p_igsp_glb, fp);
    
    if (GLB_VLAN_NONE == vid)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            _igsp_cmd_act_do_show_basic_intf(p_db_igsp_if, fp);
        }
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        _igsp_cmd_act_do_show_basic_intf(p_db_igsp_if, fp);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_querier(vid_t vid, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_global_t *p_igsp_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _igsp_cmd_act_do_show_querier_global(p_igsp_glb, fp);
    
    if (GLB_VLAN_NONE == vid)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            _igsp_cmd_act_do_show_querier_intf(p_db_igsp_if, fp);
        }
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        _igsp_cmd_act_do_show_querier_intf(p_db_igsp_if, fp);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_mrouter(vid_t vid, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 banner = TRUE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (GLB_VLAN_NONE == vid)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            _igsp_cmd_act_do_show_mrouter_intf(p_db_igsp_if, fp, &banner);
        }
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        _igsp_cmd_act_do_show_mrouter_intf(p_db_igsp_if, fp, &banner);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_do_show_hostjoin(tbl_igsp_intf_t *p_db_igsp_if, FILE *fp, uint32 *banner)
{
    ds_join_group_t   *p_db_join = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char                ipaddr_str[CMD_BUF_32];
    
    if (*banner)
    {
        *banner = FALSE;
        sal_fprintf(fp, "%-8s %-20s\n", "VLAN", "Host Join Group");
        sal_fprintf(fp, "--------+--------------------\n");
    }

    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->join_list.obj_list, p_db_join, listnode)
        {
            if (FALSE == p_db_igsp_if->enable)
            {
                continue;
            }
            cdb_addr_ipv4_val2str(ipaddr_str, CMD_BUF_32, &p_db_join->key.group);
            sal_fprintf(fp, "%-8u %-20s\n", p_db_igsp_if->key.vid, ipaddr_str);
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_hostjoin(vid_t vid, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 banner = TRUE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (GLB_VLAN_NONE == vid)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            _igsp_cmd_act_do_show_hostjoin(p_db_igsp_if, fp, &banner);
        }
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        _igsp_cmd_act_do_show_hostjoin(p_db_igsp_if, fp, &banner);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_groups_count(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;
    uint32 dynamic_count = 0;
    uint32 active_static_count = 0;
    uint32 static_count = 0;
    tbl_interface_t *igsp_if = NULL;
    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            igsp_if = tbl_interface_get_interface_by_ifindex(p_db_group->key.ifindex);
            if (tbl_interface_is_running(igsp_if))
            {
                if (IGMP_TYPE_DYNAMIC == p_db_group->type)
                {
                    dynamic_count++;
                }
                else if (IGMP_TYPE_STATIC == p_db_group->type)
                {
                    static_count++;
                    if (!p_db_group->inactive)
                    {
                        active_static_count++;
                    }
                }
            }
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-30s : %u\n", "Dynamic Multicast Groups Count", dynamic_count);
    sal_fprintf(fp, "%-30s : %u (Active : %u)\n", "Static Multicast Groups Count", static_count, active_static_count);
    sal_fprintf(fp, "%-30s : %u (Active : %u)\n", "Total Multicast Groups Count", 
        (dynamic_count + static_count), (dynamic_count + active_static_count));

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_igsp_cmd_act_show_groups(vid_t vid, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 banner = TRUE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (GLB_VLAN_NONE == vid)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_igsp_if = p_master->igsp_array[vid];
            if (NULL == p_db_igsp_if)
            {
                continue;
            }
            _igsp_cmd_act_do_show_groups_intf(p_db_igsp_if, fp, &banner);
        }
    }
    else
    {
        key.vid = vid;
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
        if (!p_db_igsp_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("IGMP snooping not configured on vlan %u", vid);
        }
        
        sal_fprintf(fp, "IGMP Snooping groups for vlan%u\n", vid);
        _igsp_cmd_act_do_show_groups_intf_detail(p_db_igsp_if, fp, &banner);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
igsp_cmd_process_act_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 value = 0;
    vid_t vid = GLB_VLAN_NONE;
    int32 rc = PM_E_NONE;

    if (0 == argc)
    {
        rc = _igsp_cmd_act_show_basic(vid, para);
    }
    else if (1 <= argc)
    {
        if (0 == sal_strcmp("global", argv[0]))
        {
            _igsp_cmd_act_show_global(argv, argc, para);
        }
        else if (0 == sal_strcmp("vlan", argv[0]))
        {
            if (2 != argc)
            {
                CFG_INVALID_PARAM_RET("Invalid argc, should be 2");
            }
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            vid = value;
            rc = _igsp_cmd_act_show_basic(vid, para);
        }
        else if (0 == sal_strcmp("querier", argv[0]))
        {
            if (1 == argc)
            {
                vid = GLB_VLAN_NONE;
                rc = _igsp_cmd_act_show_querier(vid, para);
            }
            else if (2 == argc)
            {
                PM_CFG_GET_UINT(value, argv[1], para->p_rs);
                vid = value;
                rc = _igsp_cmd_act_show_querier(vid, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter number");
            }
        }
        else if (0 == sal_strcmp("groups", argv[0]))
        {
            if (1 == argc)
            {
                rc = _igsp_cmd_act_show_groups(vid, para);
            }
            else if (2 == argc)
            {
                if (0 == sal_strcmp("count", argv[1]))
                {
                    _igsp_cmd_act_show_groups_count(para);
                }
                else
                {
                    PM_CFG_GET_UINT(value, argv[1], para->p_rs);
                    vid = value;
                    rc = _igsp_cmd_act_show_groups(vid, para);
                }
            }
            else if (3 == argc)
            {
                CFG_INVALID_PARAM_RET("Invalid action");
            }
        }
        else if (0 == sal_strcmp("mrouter", argv[0]))
        {
            if (1 == argc)
            {
                vid = GLB_VLAN_NONE;
                rc = _igsp_cmd_act_show_mrouter(vid, para);
            }
            else if (2 == argc)
            {
                PM_CFG_GET_UINT(value, argv[1], para->p_rs);
                vid = value;
                rc = _igsp_cmd_act_show_mrouter(vid, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter number");
            }
        }
        else if (0 == sal_strcmp("hostjoin", argv[0]))
        {
            if (1 == argc)
            {
                vid = GLB_VLAN_NONE;
                rc = _igsp_cmd_act_show_hostjoin(vid, para);
            }
            else if (2 == argc)
            {
                PM_CFG_GET_UINT(value, argv[1], para->p_rs);
                vid = value;
                rc = _igsp_cmd_act_show_hostjoin(vid, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter number");
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
    }

    return rc;
}
            
int32
igsp_cmd_process_act_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 value = 0;
    vid_t vid = GLB_VLAN_NONE;
    int32 rc = PM_E_NONE;
    addr_ipv4_t group_addr;
    addr_ipv4_t *p_group_addr = NULL;

    if (2 <= argc)
    {
        if (0 == sal_strcmp("groups", argv[0]))
        {
            if (0 == sal_strcmp("all", argv[1]))
            {
                group_addr.s_addr = 0;
            }
            else
            {
                rc = cdb_addr_ipv4_str2val(&group_addr, argv[1], sal_strlen(argv[1]));
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid address %s", argv[1]);
                }
                p_group_addr = &group_addr;
            }

            if (3 <= argc)
            {
                PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                vid = value;
            }
            igsp_api_clear_groups_by_vlan(p_group_addr, vid);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid action args");
    }
        
    return rc;
}


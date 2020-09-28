/**
 @date 2009-12-22

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_usw_qos_cli.h"

extern int32 sys_usw_qos_domain_map_default(uint8 lchip);
extern int32 sys_usw_qos_domain_map_dump(uint8 lchip, uint8 domain, uint8 type);
extern int32 sys_usw_qos_cpu_reason_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32 sys_usw_qos_queue_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32 sys_usw_qos_group_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32 sys_usw_qos_port_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32 sys_usw_qos_policer_dump(uint8 type, uint8 dir, uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32 sys_usw_qos_dump_status(uint8 lchip);
extern int32 sys_usw_queue_set_egs_resrc_guarantee(uint8 lchip, uint8 enable);
extern int32 sys_usw_qos_set_shape_mode(uint8 lchip, uint8 mode);
extern int32 sys_usw_qos_table_map_dump(uint8 lchip, uint8 domain, uint8 type);
extern int32 sys_usw_qos_service_dump(uint8 lchip, uint16 start, uint16 end);
extern int32 sys_usw_qos_set_drop_resrc_check_mode(uint8 lchip, uint8 mode);
extern int32 sys_usw_qos_set_aqmscan_high_priority_en(uint8 lchip, bool enable);
/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/


CTC_CLI(ctc_cli_usw_qos_show_element_info,
        ctc_cli_usw_qos_show_element_info_cmd,
        "show qos (((port-policer (in|out) |flow-policer| vlan-policer (in|out)|copp| cpu-reason | queue | group | port) (range START END)) | (service-id SERVICE port GPHYPORT_ID )) (lchip LCHIP|)",
        "Show",
        CTC_CLI_QOS_STR,
        "Port policer ",
        "Ingress port",
        "Egress port",
        "Flow policer",
        "Vlan policer",
        "Ingress vlan",
        "Egress vlan",
        CTC_CLI_QOS_COPP,
        "Cpu reason range <0-max>",
        "Queue range",
        "Group range",
        "local port range<0~MAX Local Port ID>",
        "Range",
        "Range start",
        "Range end",
        "Service id",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint8 index  = 0;
    uint16 start = 0;
    uint16 end   = 0;
    uint8 dir   = CTC_INGRESS;
    uint16 service_id = 0;
    uint32 gport = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    index = CTC_CLI_GET_ARGC_INDEX("range");
    if (0xFF != index)
    {
        /* Get Start queue-id */
        CTC_CLI_GET_UINT16("start", start, argv[index + 1]);

        /* Get End queue-id */
        CTC_CLI_GET_UINT16("end", end, argv[index + 2]);

        if (start > end)
        {
            ctc_cli_out("%% %s \n", "Invalid start end value.");
            return CLI_ERROR;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        /* service id */
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        CTC_CLI_GET_UINT32("port", gport, argv[index + 3]);

        ret = sys_usw_qos_service_dump(lchip, service_id, gport);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port-policer");
        if (0xFF != index)
        {
            index = CTC_CLI_GET_ARGC_INDEX("out");
            if (0xFF != index)
            {
                dir = CTC_EGRESS;
            }

            ret = sys_usw_qos_policer_dump(CTC_QOS_POLICER_TYPE_PORT, dir, lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-policer");
        if (0xFF != index)
        {
            ret = sys_usw_qos_policer_dump(CTC_QOS_POLICER_TYPE_FLOW, dir, lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("vlan-policer");
        if (0xFF != index)
        {
            index = CTC_CLI_GET_ARGC_INDEX("out");
            if (0xFF != index)
            {
                dir = CTC_EGRESS;
            }
            ret = sys_usw_qos_policer_dump(CTC_QOS_POLICER_TYPE_VLAN, dir, lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("copp");
        if (0xFF != index)
        {
            ret = sys_usw_qos_policer_dump(CTC_QOS_POLICER_TYPE_COPP, dir, lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("cpu-reason");
        if (0xFF != index)
        {
            ret = sys_usw_qos_cpu_reason_dump(lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("queue");
        if (0xFF != index)
        {
            ret = sys_usw_qos_queue_dump(lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("group");
        if (0xFF != index)
        {
            ret = sys_usw_qos_group_dump(lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            ret = sys_usw_qos_port_dump(lchip, start, end, detail);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
        }
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_usw_qos_use_default_domain,
        ctc_cli_usw_qos_use_default_domain_cmd,
        "qos domain default (lchip LCHIP|)",
        CTC_CLI_QOS_STR,
        "QoS domain",
        "Default value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_qos_domain_map_default(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_qos_dump_status,
        ctc_cli_usw_qos_dump_status_cmd,
        "show qos status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_QOS_STR,
        "Resource used status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_qos_dump_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_qos_show_qos_domain,
        ctc_cli_usw_qos_show_qos_domain_cmd,
        "show qos domain DOAMIN type (cos-map-pri|ip-prec-map-pri|dscp-map-pri|exp-map-pri|pri-map-cos|pri-map-dscp|pri-map-exp) (lchip LCHIP|)",
        "Show",
        CTC_CLI_QOS_STR,
        "QoS domain",
        CTC_CLI_QOS_DOMAIN_VALUE,
        "Maping type",
        "Cos map to priority color",
        "Ip-prec map to priority color",
        "Dscp map to priority color",
        "Exp map to priority color",
        "Priority color map to cos",
        "Priority color map to dscp",
        "Priority color map to exp",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 domain = 0;
    uint8 type = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT8_RANGE("domain", domain, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if (CLI_CLI_STR_EQUAL("cos-map-pri", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR;
    }

    if (CLI_CLI_STR_EQUAL("ip-prec-map-pri", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR;
    }

    if (CLI_CLI_STR_EQUAL("dscp-map-pri", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR;
    }

    if (CLI_CLI_STR_EQUAL("exp-map-pri", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR;
    }

    if (CLI_CLI_STR_EQUAL("pri-map-cos", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;
    }

    if (CLI_CLI_STR_EQUAL("pri-map-dscp", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;
    }

    if (CLI_CLI_STR_EQUAL("pri-map-exp", 1))
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_qos_domain_map_dump(lchip, domain, type);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_qos_set_resrc_mgr_guarantee_enable,
        ctc_cli_usw_qos_set_resrc_mgr_guarantee_enable_cmd,
        "qos resrc-mgr guarantee (enable | disable) (lchip LCHIP|)",
        CTC_CLI_QOS_STR,
        "Resource manage",
        "Guarantee resource",
        "Globally enable",
        "Globally disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if (CLI_CLI_STR_EQUAL("e", 0))
    {
        ret = sys_usw_queue_set_egs_resrc_guarantee(lchip, 1);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        ret = sys_usw_queue_set_egs_resrc_guarantee(lchip, 0);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_qos_set_shape_mode,
        ctc_cli_usw_qos_set_shape_mode_cmd,
        "qos shape mode MODE (lchip LCHIP|)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Shape mode",
        "mode <0-1>",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 mode = 0;

    CTC_CLI_GET_UINT8_RANGE("mode", mode, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_qos_set_shape_mode(lchip, mode);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_usw_qos_set_igs_table_map,
        ctc_cli_usw_qos_set_igs_table_map_cmd,
        "qos (table-map-id TABLE_MAP_ID) (cos2cos |cos2dscp | dscp2cos | dscp2dscp|) \
        (in IN_VALUE) (out OUT_VALUE) ((action-type (none |copy | map|))|)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_TABLE_MAP_STR,
        CTC_CLI_QOS_TABLE_MAP_VALUE,
        "Cos2cos",
        "Cos2dscp",
        "Dscp2cos",
        "Dscp2dscp",
        "In",
        "Input value",
        "Out",
        "Output value",
        "Action type",
        "None",
        "Keep original value",
        "Use value from table map")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_table_map_t table_map;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    sal_memset(&table_map, 0, sizeof(ctc_qos_table_map_t));

    index = CTC_CLI_GET_ARGC_INDEX("table-map-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("table-map-id", table_map.table_map_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cos2cos");
    if (0xFF != index)
    {
        table_map.type = CTC_QOS_TABLE_MAP_IGS_COS_TO_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cos2dscp");
    if (0xFF != index)
    {
        table_map.type = CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp2cos");
    if (0xFF != index)
    {
        table_map.type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp2dscp");
    if (0xFF != index)
    {
        table_map.type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("in");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("in", table_map.in, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("out");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("out", table_map.out, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("action-type");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("none");
        if (0xFF != index)
        {
            table_map.action_type = CTC_QOS_TABLE_MAP_ACTION_NONE;
        }

        index = CTC_CLI_GET_ARGC_INDEX("copy");
        if (0xFF != index)
        {
            table_map.action_type = CTC_QOS_TABLE_MAP_ACTION_COPY;
        }

        index = CTC_CLI_GET_ARGC_INDEX("map");
        if (0xFF != index)
        {
            table_map.action_type = CTC_QOS_TABLE_MAP_ACTION_MAP;
        }
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TABLE_MAP;
    sal_memcpy(&glb_cfg.u.table_map, &table_map, sizeof(ctc_qos_table_map_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_global_config(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_qos_set_global_config(&glb_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_qos_show_qos_table_map,
        ctc_cli_usw_qos_show_qos_table_map_cmd,
        "show qos table-map-id TABLE_MAP_ID (cos2cos |cos2dscp | dscp2cos | dscp2dscp|)(in IN_VALUE|) (lchip LCHIP|)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_TABLE_MAP_STR,
        CTC_CLI_QOS_TABLE_MAP_VALUE,
        "Cos2cos",
        "Cos2dscp",
        "Dscp2cos",
        "Dscp2dscp",
        "In",
        "Input value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 table_map_id = 0;
    uint8 type = 0;
    uint8 index = 0;
    ctc_qos_table_map_t table_map;
    ctc_qos_glb_cfg_t glb_cfg;
    char*  str_action[3] = {"None", "Copy", "Map"};

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    sal_memset(&table_map, 0, sizeof(ctc_qos_table_map_t));

    CTC_CLI_GET_UINT8_RANGE("table-map-id", table_map_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("cos2cos");
    if (0xFF != index)
    {
        type = CTC_QOS_TABLE_MAP_IGS_COS_TO_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cos2dscp");
    if (0xFF != index)
    {
        type = CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp2cos");
    if (0xFF != index)
    {
        type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp2dscp");
    if (0xFF != index)
    {
        type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    table_map.type = type;
    table_map.table_map_id = table_map_id;
    
    index = CTC_CLI_GET_ARGC_INDEX("in");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("in", table_map.in, argv[index + 1]);
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TABLE_MAP;
        sal_memcpy(&glb_cfg.u.table_map, &table_map, sizeof(ctc_qos_table_map_t));
        if(g_ctcs_api_en)
        {
            ret = ctcs_qos_get_global_config(g_api_lchip, &glb_cfg);
        }
        else
        {
            ret = ctc_qos_get_global_config(&glb_cfg);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        
        switch (type)
        {
        case CTC_QOS_TABLE_MAP_IGS_COS_TO_COS:
            ctc_cli_out("ingress cos -> cos mapping table\n");
            ctc_cli_out("----------------------------------------\n");
            ctc_cli_out("   cos(%d) -> cos(%d) + action(%s)\n",
                                  glb_cfg.u.table_map.in, glb_cfg.u.table_map.out, str_action[glb_cfg.u.table_map.action_type]);
            break;
        case CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP:
            ctc_cli_out("ingress cos -> dscp mapping table\n");
            ctc_cli_out("----------------------------------------\n");
            ctc_cli_out("   cos(%d) -> dscp(%d) + action(%s)\n",
                                  glb_cfg.u.table_map.in, glb_cfg.u.table_map.out, str_action[glb_cfg.u.table_map.action_type]);
            break;
        case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS:
            ctc_cli_out("ingress dscp -> cos mapping table\n");
            ctc_cli_out("----------------------------------------\n");
            ctc_cli_out("   dscp(%d) -> cos(%d) + action(%s)\n",
                                  glb_cfg.u.table_map.in, glb_cfg.u.table_map.out, str_action[glb_cfg.u.table_map.action_type]);
            break;
        case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP:
            ctc_cli_out("ingress dscp -> dscp mapping table\n");
            ctc_cli_out("----------------------------------------\n");
            ctc_cli_out("   dscp(%d) -> dscp(%d) + action(%s)\n",
                                  glb_cfg.u.table_map.in, glb_cfg.u.table_map.out, str_action[glb_cfg.u.table_map.action_type]);
            break;
        }
    }
    else
    {
        lchip = g_ctcs_api_en?g_api_lchip:lchip;
        ret = sys_usw_qos_table_map_dump(lchip, table_map_id, type);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_qos_set_ecn_en,
        ctc_cli_usw_qos_set_ecn_en_cmd,
        "qos ecn-en (enable|disable)",
        CTC_CLI_QOS_STR,
        "Ecn en",
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;
	sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

	index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        glb_cfg.u.value = 1;
    }
	glb_cfg.cfg_type = CTC_QOS_GLB_CFG_ECN_EN;
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_global_config(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_qos_set_global_config(&glb_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_qos_get_ecn_en,
        ctc_cli_usw_qos_get_ecn_en_cmd,
        "show qos global-cfg (ecn-en)",
        "Show",
        CTC_CLI_QOS_STR,
        "Global config",
        "Ecn en")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("ecn-en");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_ECN_EN;
        ctc_cli_out("Show qos ecn-en info\n");
        ctc_cli_out("==================================\n");
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_global_config(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_qos_get_global_config(&glb_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("value : %d\n", glb_cfg.u.value?1:0);
    ctc_cli_out("==================================\n");

    return ret;
}

CTC_CLI(ctc_cli_usw_qos_set_resrc_check_mode,
        ctc_cli_usw_qos_set_resrc_check_mode_cmd,
        "qos resrc-mgr check-mode (continuous | discrete) (lchip LCHIP|)",
        "CTC_CLI_QOS_STR",
        "Resource manage",
        "Resource Check Mode",
        "Continuous mode",
        "Discrete mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 mode = 0;
    uint8 lchip = 0;
    uint16 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("discrete");
    if (0xFF != index)
    {
        mode = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_qos_set_drop_resrc_check_mode(lchip, mode);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_qos_set_aqmscan_highpri_en,
        ctc_cli_usw_qos_set_aqmscan_highpri_en_cmd,
        "qos aqmscan high-priority (enable|disable)(lchip LCHIP|)",
        CTC_CLI_QOS_STR,
        "Aqm scan",
        "High priority",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    bool enable = 0;
    uint8 lchip = 0;

	index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        enable = 1;
    }
    
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_qos_set_aqmscan_high_priority_en(lchip,enable);

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_qos_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_use_default_domain_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_qos_domain_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_element_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_dump_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_resrc_mgr_guarantee_enable_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_qos_set_shape_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_igs_table_map_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_qos_table_map_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_ecn_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_get_ecn_en_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_qos_set_resrc_check_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_aqmscan_highpri_en_cmd);
    return CLI_SUCCESS;
}

int32
ctc_usw_qos_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_use_default_domain_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_qos_domain_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_element_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_dump_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_resrc_mgr_guarantee_enable_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_qos_set_shape_mode_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_igs_table_map_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_show_qos_table_map_cmd);
	uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_ecn_en_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_get_ecn_en_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_qos_set_resrc_check_mode_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_qos_set_aqmscan_highpri_en_cmd);
    return CLI_SUCCESS;
}


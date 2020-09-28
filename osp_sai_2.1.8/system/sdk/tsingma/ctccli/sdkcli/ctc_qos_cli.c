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
#include "ctc_acl_cli.h"
#include "ctc_qos_cli.h"
#include "ctc_mirror.h"


#define CTC_QUEUE_NUM_PER_PORT 8
uint16 g_ctc_cli_drop[CTC_QUEUE_NUM_PER_PORT][3];
ctc_qos_drop_array g_qos_drop_array;

int32 ctc_cli_queue_drop_thrd_init()
{
    uint8 i = 0;
    uint8 j = 0;
    uint16 drop_base = 256;

    for (i = 0; i < CTC_QUEUE_NUM_PER_PORT; i++)
    {
        for (j = 0; j < 3; j++)
        {
            g_ctc_cli_drop[i][j] = drop_base + j*128 + i*512;
        }
    }

    return CLI_SUCCESS;
}

#define CTC_CLI_QOS_QUEUE_ID_STR  "((service-id SERVICE|) port GPHYPORT_ID | reason-id REASON) (queue-id QUEUE_ID|)"
#define CTC_CLI_QOS_QUEUE_ID_DSCP  \
        "Service id",\
        CTC_CLI_QOS_SERVICE_VAL,\
        CTC_CLI_GPORT_DESC,\
        CTC_CLI_GPHYPORT_ID_DESC,\
        "Cpu reason",\
        "Cpu reason id",\
        CTC_CLI_QOS_QUEUE_STR,\
        CTC_CLI_QOS_QUEUE_VAL


int32  _ctc_cli_qos_queue_id_parser(ctc_qos_queue_id_t *p_queue, char** argv, uint16 argc)
{
    uint16 gport = 0;
    uint16  qid = 0;
    uint8 index = 0;
    uint16 service_id = 0;
    uint16 reason_id = 0;
    uint8 queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    int32 ret = CLI_SUCCESS;

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        /* service id */
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF != index)
    {
        /* queue-id */
        CTC_CLI_GET_UINT16("queue-id", qid, argv[index+1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("reason-id");
    if (0xFF != index)
    {
        /* cpu reason id */
        CTC_CLI_GET_UINT16("reason-id", reason_id, argv[index + 1]);
        queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    }

    p_queue->queue_type = queue_type;
    p_queue->gport      = gport;
    p_queue->service_id = service_id;
    p_queue->queue_id   = qid;
    p_queue->cpu_reason = reason_id;

    return ret;

}

void _ctc_cli_queue_drop_wred_show(ctc_qos_drop_t drop_param)
{
    ctc_cli_out("%-30s: %s\n", "Drop Mode", "WRED");
    ctc_cli_out("%-30s: %u\n", "WTD and WRED coexist ", drop_param.drop.is_coexist);

    ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                "MaxThrd", drop_param.drop.max_th[0],
                 drop_param.drop.max_th[1],
                 drop_param.drop.max_th[2],
                 drop_param.drop.max_th[3]);
    ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                "MinThrd", drop_param.drop.min_th[0],
                 drop_param.drop.min_th[1],
                 drop_param.drop.min_th[2],
                 drop_param.drop.min_th[3]);
    ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                "DropProb", drop_param.drop.drop_prob[0],
                 drop_param.drop.drop_prob[1],
                 drop_param.drop.drop_prob[2],
                 drop_param.drop.drop_prob[3]);
}
void _ctc_cli_queue_drop_wtd_show(ctc_qos_drop_t drop_param)
{
    ctc_cli_out("%-30s: %s\n", "Drop Mode", "WTD");
    ctc_cli_out("%-30s: %u\n", "WTD and WRED coexist ", drop_param.drop.is_coexist);
    if(drop_param.drop.is_dynamic == 0)
    {
        ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                    "DropThrd", drop_param.drop.max_th[0],
                     drop_param.drop.max_th[1],
                     drop_param.drop.max_th[2],
                     drop_param.drop.max_th[3]);
        ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                    "EcnThrdColor", drop_param.drop.ecn_th[0],
                     drop_param.drop.ecn_th[1],
                     drop_param.drop.ecn_th[2],
                     drop_param.drop.ecn_th[3]);
        ctc_cli_out("%-30s: %-8u\n",
                    "EcnThrd", drop_param.drop.ecn_mark_th);
    }
    else
    {
        ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                    "DropFactor", drop_param.drop.drop_factor[0],
                     drop_param.drop.drop_factor[1],
                     drop_param.drop.drop_factor[2],
                     drop_param.drop.drop_factor[3]);
        ctc_cli_out("%-30s: %-8u%-8u%-8u%-8u\n",
                    "EcnThrdColor", drop_param.drop.ecn_th[0],
                     drop_param.drop.ecn_th[1],
                     drop_param.drop.ecn_th[2],
                     drop_param.drop.ecn_th[3]);
        ctc_cli_out("%-30s: %-8u\n",
                    "EcnThrd", drop_param.drop.ecn_mark_th);
    }
}
int32 ctc_cli_update_queue_drop_thrd(ctc_qos_queue_id_t *p_queue, uint32 weight)
{
    int32 ret  = CLI_SUCCESS;
    uint8 i = 0;
    ctc_qos_drop_t drop_param;
    ctc_qos_queue_drop_t drop;

    if (weight >= 256)
    {
        ctc_cli_out("WARNING weight exceed max  255!!, schedule not correct!!\n");
        return CLI_SUCCESS;
    }

    sal_memset(&drop_param, 0, sizeof(ctc_qos_drop_t));
    sal_memset(&drop, 0, sizeof(ctc_qos_queue_drop_t));

    i = weight/32;

    drop.mode = CTC_QUEUE_DROP_WTD;
    drop.max_th[0] = g_ctc_cli_drop[i][0];
    drop.max_th[1] = g_ctc_cli_drop[i][1];
    drop.max_th[2] = g_ctc_cli_drop[i][2];
    drop.max_th[3] = 0x44;

    sal_memcpy(&drop_param.queue, p_queue, sizeof(ctc_qos_queue_id_t));
    sal_memcpy(&drop_param.drop, &drop, sizeof(ctc_qos_queue_drop_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_drop_scheme(g_api_lchip, &drop_param);
    }
    else
    {
        ret = ctc_qos_set_drop_scheme(&drop_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


#define policer_cli ""

CTC_CLI(ctc_cli_qos_set_policer_first,
        ctc_cli_qos_set_policer_first_cmd,
        "qos policer (in | out | both) (flow-first | port-first)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Ingress direction",
        "Egress direction",
        "Both direction",
        "Flow policer first",
        "Port policer first")
{
    uint8 dir = CTC_INGRESS;
    int32 ret  = CLI_SUCCESS;
    uint8 flow_first_en = 0;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    /* parse direction */
    if (CLI_CLI_STR_EQUAL("i", 0))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("o", 0))
    {
        dir = CTC_EGRESS;
    }
    else
    {
        dir = CTC_BOTH_DIRECTION;
    }

    /* parse policer first */
    if (CLI_CLI_STR_EQUAL("flow", 1))
    {
        flow_first_en = 1;
    }
    else
    {
        flow_first_en = 0;
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_FLOW_FIRST_EN;
    glb_cfg.u.value = ((dir << 16) | flow_first_en);
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

CTC_CLI(ctc_cli_qos_set_resrc_mgr_enable,
        ctc_cli_qos_set_resrc_mgr_enable_cmd,
        "qos resrc-mgr (enable | disable)",
        CTC_CLI_QOS_STR,
        "Resource manage",
        "Globally enable",
        "Globally disable")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    if (CLI_CLI_STR_EQUAL("e", 0))
    {
        glb_cfg.u.value = 1;
    }
    else
    {
        glb_cfg.u.value = 0;
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_RESRC_MGR_EN;

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

CTC_CLI(ctc_cli_qos_set_policer_configure,
        ctc_cli_qos_set_policer_configure_cmd,
        "qos policer (sequential | ipg | update |hbwp-share| ecn-mark| port-stbm) (enable | disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Sequential policer",
        "Ipg used for policer",
        "Update token",
        "HBWP share mode",
        "ECN mark",
        "Port policer mode stbm",
        "Globally enable ",
        "Globally disable")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    if (CLI_CLI_STR_EQUAL("ipg", 0))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_IPG_EN;
    }
    else if (CLI_CLI_STR_EQUAL("update", 0))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_EN;
    }
    else if (CLI_CLI_STR_EQUAL("hbwp-share", 0))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_HBWP_SHARE_EN;
    }
    else if (CLI_CLI_STR_EQUAL("ecn-mark", 0))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_MARK_ECN_EN;
    }
    else if (CLI_CLI_STR_EQUAL("port-stbm", 0))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_PORT_STBM_EN;
    }
    else
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_SEQENCE_EN;
    }

    if (CLI_CLI_STR_EQUAL("e", 1))
    {
        glb_cfg.u.value = 1;
    }
    else
    {
        glb_cfg.u.value = 0;
    }

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


CTC_CLI(ctc_cli_qos_set_phb_configure,
        ctc_cli_qos_set_phb_configure_cmd,
        "qos map priority PRIORITY to phb PHB",
        CTC_CLI_QOS_STR,
        "Map",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "To",
        "PHB,regard as cos index",
        "Value")
{
    int32 ret = CLI_SUCCESS;
    uint8 priority = 0;
    uint8 phb = 0;
    ctc_qos_glb_cfg_t glb_cfg;
    ctc_qos_phb_map_t phb_map;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    sal_memset(&phb_map, 0, sizeof(ctc_qos_phb_map_t));
    CTC_CLI_GET_UINT8("priority", priority, argv[0]);
    CTC_CLI_GET_UINT8("phb", phb, argv[1]);

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_PHB_MAP;
    phb_map.map_type = CTC_QOS_PHB_MAP_PRI;
    phb_map.priority = priority;
    phb_map.cos_index = phb;
    sal_memcpy(&glb_cfg.u.phb_map, &phb_map, sizeof(ctc_qos_phb_map_t));
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

CTC_CLI(ctc_cli_qos_get_phb_configure,
        ctc_cli_qos_get_phb_configure_cmd,
        "show qos map priority PRIORITY",
        "Show",
        CTC_CLI_QOS_STR,
        "Map",
        "Priority",
        CTC_CLI_PRIORITY_VALUE)
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    CTC_CLI_GET_UINT8("priority", glb_cfg.u.phb_map.priority, argv[0]);

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_PHB_MAP;
    glb_cfg.u.phb_map.map_type = CTC_QOS_PHB_MAP_PRI;
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
    ctc_cli_out("\nPriority to Phb Information:\n");
    ctc_cli_out("%-10s %-10s\n","Priority", "Phb");
    ctc_cli_out("-----------------------------------------\n");
    ctc_cli_out("%-10d %-10d\n",glb_cfg.u.phb_map.priority,glb_cfg.u.phb_map.cos_index);
    return ret;
}
CTC_CLI(ctc_cli_qos_create_policer,
        ctc_cli_qos_create_policer_cmd,
        "qos policer attach (port GPORT (in|out) |flow POLICER_ID|service SERVICE_ID|vlan VLAN_ID(in|out)|copp POLICER_ID|mfp POLICER_ID) \
         (mode (rfc2697|rfc2698|rfc4115|bwp|stbm)) ((color-blind|color-aware)|) \
          (cir CIR) (cbs CBS |) ((pir PIR|eir EIR)|) ((pbs PBS|ebs EBS)|) (cf-en|)(color-merge-mode VALUE|)(drop-color (none|red|yellow)|) (use-l3-length|) (stats-en|) \
         ((hbwp-en schedule-mode (sp|wdrr weight WEIGHT) cos-index INDEX max-cir CIR_MAX (max-eir EIR_MAX|)(sf-en|) (cf-total-dis|) (triple-play|))|) \
         (action {priority-green VALUE | priority-yellow VALUE | priority-red VALUE | drop-green | drop-yellow | drop-red}| ) (level LEVEL|) (pps-en|)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Attach policer",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_FLOW_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service policer",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_QOS_VLAN_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Micro flow policer for ipfix",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Policer mode",
        "RFC2697, srTCM",
        "RFC2698, trTCM",
        "RFC4115, enhaned trTCM",
        "MEF 10.1,bandwidth profile",
        "STBM, Single Token Bucket Meter",
        "Color blind mode, default policing mode",
        "Color aware mode",
        CTC_CLI_QOS_PLC_CIR_STR,
        CTC_CLI_QOS_PLC_CIR_VAL,
        CTC_CLI_QOS_PLC_CBS_STR,
        CTC_CLI_QOS_PLC_CBS_VAL,
        CTC_CLI_QOS_PLC_PIR_STR,
        CTC_CLI_QOS_PLC_PIR_VAL,
        CTC_CLI_QOS_PLC_EIR_STR,
        CTC_CLI_QOS_PLC_EIR_VAL,
        CTC_CLI_QOS_PLC_PBS_STR,
        CTC_CLI_QOS_PLC_PBS_VAL,
        CTC_CLI_QOS_PLC_EBS_STR,
        CTC_CLI_QOS_PLC_EBS_VAL,
        "Couple flag enable, only for BWP",
        "Color merge mode,only for 2 level policer",
        "Value",
        "Drop color config",
        "None",
        "Red, default drop color",
        "Yellow",
        "Use packet length from layer 3 header for metering",
        "Stats enable",
        "HBWP enable, only support service policer on GB",
        "scheulde mode",
        "Strict priority",
        "Wdrr, GB only support cos 0 and cos 1, other is SP",
        "Weight",
        "Weight value, GB only support cos 1 weight, cos 0 is (0x3FF - Weight)",
        "Cos index",
        "Value",
        "Cir max",
        CTC_CLI_QOS_PLC_EIR_STR,
        "Pir max",
        CTC_CLI_QOS_PLC_EIR_STR,
        "Share flag enable",
        "Coupling total disable",
        "Triple play mode, if set, cos_max=3",
        "Policer action",
        "Green priority",
        CTC_CLI_PRIORITY_VALUE,
        "Yellow priority",
        CTC_CLI_PRIORITY_VALUE,
        "Red priority",
        CTC_CLI_PRIORITY_VALUE,
        "Drop green",
        "Drop yellow",
        "Drop red",
        "Policer level",
        "Policer level value",
        "PPS enable")
{
    ctc_qos_policer_t policer_param;
    uint32 gport = 0;
    int16 idx = 0;
    int16 idx1 = 0;
    uint8 dir = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&policer_param, 0, sizeof(ctc_qos_policer_t));

    /* policer id */
    policer_param.dir = CTC_INGRESS;
    policer_param.enable = 1;
    /* init default params */
    policer_param.policer.is_color_aware = 0;
    policer_param.policer.cbs = 64;
    policer_param.policer.pbs = 64;
    policer_param.policer.drop_color = CTC_QOS_COLOR_RED;
    policer_param.policer.use_l3_length = 0;

    /*------------------------------------------
     ##policer type
    --------------------------------------------*/

    /* port policer */
    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("port", gport, argv[idx + 1]);
        policer_param.id.gport = gport;
        policer_param.type = CTC_QOS_POLICER_TYPE_PORT;

        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        policer_param.dir = dir;
    }

    /* flow policer */
    idx = CTC_CLI_GET_ARGC_INDEX("flow");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    /* service policer */
    idx = CTC_CLI_GET_ARGC_INDEX("service");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("service id", policer_param.id.service_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_SERVICE;
    }

    /* vlan policer */
    idx = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.vlan_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_VLAN;
        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        policer_param.dir = dir;
    }
    /* copp */
    idx = CTC_CLI_GET_ARGC_INDEX("copp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_COPP;
    }
    /* mfp */
    idx = CTC_CLI_GET_ARGC_INDEX("mfp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_MFP;
    }

    /*------------------------------------------
     ##policer mode
    --------------------------------------------*/

    idx = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != idx)
    {
        if (CLI_CLI_STR_EQUAL("rfc2697", idx + 1))
        {
            policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
        }
        else if(CLI_CLI_STR_EQUAL("rfc2698", idx + 1))
        {
            policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_RFC2698;
        }
        else if(CLI_CLI_STR_EQUAL("rfc4115", idx + 1))
        {
            policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_RFC4115;
        }
        else if(CLI_CLI_STR_EQUAL("bwp", idx + 1))
        {
            policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_MEF_BWP;
        }
        else if(CLI_CLI_STR_EQUAL("stbm", idx + 1))
        {
            policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_STBM;
        }
    }


    /*------------------------------------------
     ##policer common paramete
    --------------------------------------------*/

    /* color blind */
    idx = CTC_CLI_GET_ARGC_INDEX("color-blind");
    if (0xFF != idx)
    {
        policer_param.policer.is_color_aware = 0;
    }

    /* color aware */
    idx = CTC_CLI_GET_ARGC_INDEX("color-aware");
    if (0xFF != idx)
    {
        policer_param.policer.is_color_aware = 1;
    }

    /* cir */
    idx = CTC_CLI_GET_ARGC_INDEX("cir");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("cir", policer_param.policer.cir, argv[idx + 1]);
    }

    /* cbs */
    idx = CTC_CLI_GET_ARGC_INDEX("cbs");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("cbs", policer_param.policer.cbs, argv[idx + 1]);
    }

    /* pir */
    idx = CTC_CLI_GET_ARGC_INDEX("pir");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("pir", policer_param.policer.pir, argv[idx + 1]);
    }

    /* eir */
    idx = CTC_CLI_GET_ARGC_INDEX("eir");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("eir", policer_param.policer.pir, argv[idx + 1]);
    }

    /* pbs */
    idx = CTC_CLI_GET_ARGC_INDEX("pbs");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("pbs", policer_param.policer.pbs, argv[idx + 1]);
    }

    /* ebs */
    idx = CTC_CLI_GET_ARGC_INDEX("ebs");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("ebs", policer_param.policer.pbs, argv[idx + 1]);
    }

    /* couple flag */
    idx = CTC_CLI_GET_ARGC_INDEX("cf-en");
    if (0xFF != idx)
    {
        policer_param.policer.cf_en = 1;
    }
    /* couple flag */
    idx = CTC_CLI_GET_ARGC_INDEX("color-merge-mode");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("color-merge-mode", policer_param.policer.color_merge_mode, argv[idx + 1]);
    }

    /* drop color */
    idx = CTC_CLI_GET_ARGC_INDEX("drop-color");
    if (0xFF != idx)
    {
        if (CLI_CLI_STR_EQUAL("red", idx + 1))
        {
            policer_param.policer.drop_color = CTC_QOS_COLOR_RED;
        }
        else if (CLI_CLI_STR_EQUAL("yel", idx + 1))
        {
            policer_param.policer.drop_color = CTC_QOS_COLOR_YELLOW;
        }
        else if (CLI_CLI_STR_EQUAL("none", idx + 1))
        {
            policer_param.policer.drop_color = CTC_QOS_COLOR_NONE;
        }
    }

    /* use l3 length */
    idx = CTC_CLI_GET_ARGC_INDEX("use-l3-length");
    if (0xFF != idx)
    {
        policer_param.policer.use_l3_length = 1;
    }

    /* enable stats */
    idx = CTC_CLI_GET_ARGC_INDEX("stats-en");
    if (0xFF != idx)
    {
        policer_param.policer.stats_en = TRUE;
    }

    if (policer_param.policer.policer_mode == CTC_QOS_POLICER_MODE_RFC2697)
    {
        policer_param.policer.pir = policer_param.policer.cir;
    }


    /* hbwp policer */
    idx1 = CTC_CLI_GET_ARGC_INDEX("hbwp-en");
    if (0xFF != idx1)
    {
        policer_param.hbwp_en = 1;

        idx = CTC_CLI_GET_SPECIFIC_INDEX("wdrr", idx1);
        if (0xFF != idx)
        {
            policer_param.hbwp.sp_en = 0;
            CTC_CLI_GET_UINT16("weigth", policer_param.hbwp.weight, argv[idx1 + idx + 2]);
        }
        else
        {
            policer_param.hbwp.sp_en = 1;
        }

        idx = CTC_CLI_GET_SPECIFIC_INDEX("cos-index", idx1);
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("cos-index", policer_param.hbwp.cos_index, argv[idx1 + idx + 1]);
        }


        idx = CTC_CLI_GET_SPECIFIC_INDEX("max-cir", idx1);
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("cir-max", policer_param.hbwp.cir_max, argv[idx1 + idx + 1]);
        }

        idx = CTC_CLI_GET_SPECIFIC_INDEX("max-eir", idx1);
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("pir-max", policer_param.hbwp.pir_max, argv[idx1 + idx + 1]);
        }

        idx = CTC_CLI_GET_SPECIFIC_INDEX("sf-en", idx1);
        if (0xFF != idx)
        {
            policer_param.hbwp.sf_en = 1;
        }

        idx = CTC_CLI_GET_SPECIFIC_INDEX("cf-total-dis", idx1);
        if (0xFF != idx)
        {
            policer_param.hbwp.cf_total_dis = 1;
        }

        idx = CTC_CLI_GET_SPECIFIC_INDEX("triple-play", idx1);
        if (0xFF != idx)
        {
            policer_param.hbwp.triple_play = 1;
        }

    }

    /* action */
    idx = CTC_CLI_GET_ARGC_INDEX("action");
    if (0xFF != idx)
    {
        /* priority-green */
        idx = CTC_CLI_GET_ARGC_INDEX("priority-green");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("priority-green", policer_param.action.prio_green, argv[idx + 1]);
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID);
        }

        /* priority-yellow */
        idx = CTC_CLI_GET_ARGC_INDEX("priority-yellow");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("priority-yellow", policer_param.action.prio_yellow, argv[idx + 1]);
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID);
        }

        /* priority-red */
        idx = CTC_CLI_GET_ARGC_INDEX("priority-red");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("priority-red", policer_param.action.prio_red, argv[idx + 1]);
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID);
        }
        /* drop-green */
        idx = CTC_CLI_GET_ARGC_INDEX("drop-green");
        if (0xFF != idx)
        {
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN);
        }

        /* drop-yellow */
        idx = CTC_CLI_GET_ARGC_INDEX("drop-yellow");
        if (0xFF != idx)
        {
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW);
        }

        /* drop-red */
        idx = CTC_CLI_GET_ARGC_INDEX("drop-red");
        if (0xFF != idx)
        {
            SET_FLAG(policer_param.action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_RED);
        }

    }

    /* policer level */
    idx = CTC_CLI_GET_ARGC_INDEX("level");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("level", policer_param.level, argv[idx + 1]);
    }

    /* pps mode */
    idx = CTC_CLI_GET_ARGC_INDEX("pps-en");
    if (0xFF != idx)
    {
        policer_param.pps_en = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_policer(g_api_lchip, &policer_param);
    }
    else
    {
        ret = ctc_qos_set_policer(&policer_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_remove_policer,
        ctc_cli_qos_remove_policer_cmd,
        "qos policer detach (port GPORT (in|out) |flow POLICER_ID|service SERVICE_ID|vlan VLAN_ID(in|out)|copp POLICER_ID|mfp POLICER_ID) \
        (hbwp-en cos-index INDEX |)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Detach policer",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_FLOW_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service policer",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_QOS_VLAN_PLC_STR,
        CTC_CLI_QOS_VLAN_PLC_VAL,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Micro flow policer for ipfix",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Hbwp policer",
        "Specify the cos index",
        "Value")
{
    int32 ret = CLI_SUCCESS;
    int16 idx = 0;
    int16 idx1 = 0;
    uint8 dir = 0;
    uint32 gport = 0;
    ctc_qos_policer_t policer_param;

    sal_memset(&policer_param, 0, sizeof(ctc_qos_policer_t));

    /* policer id */
    policer_param.dir = CTC_INGRESS;
    policer_param.enable = 0;

    /*------------------------------------------
     ##policer type
    --------------------------------------------*/

    /* port policer */
    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("port", gport, argv[idx + 1]);
        policer_param.id.gport = gport;
        policer_param.type = CTC_QOS_POLICER_TYPE_PORT;

        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        policer_param.dir = dir;

    }

    /* flow policer */
    idx = CTC_CLI_GET_ARGC_INDEX("flow");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    /* service policer */
    idx = CTC_CLI_GET_ARGC_INDEX("service");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("service id", policer_param.id.service_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_SERVICE;
    }

    /* vlan policer */
    idx = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("vlan id", policer_param.id.vlan_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_VLAN;
        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        policer_param.dir = dir;
    }
    /* copp */
    idx = CTC_CLI_GET_ARGC_INDEX("copp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_COPP;
    }
    /* mfp */
    idx = CTC_CLI_GET_ARGC_INDEX("mfp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_MFP;
    }

    /* hbwp policer */
    idx1 = CTC_CLI_GET_ARGC_INDEX("hbwp-en");
    if (0xFF != idx1)
    {
        policer_param.hbwp_en = 1;
        CTC_CLI_GET_UINT16("cos-index", policer_param.hbwp.cos_index, argv[idx1 + 2]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_policer(g_api_lchip, &policer_param);
    }
    else
    {
        ret = ctc_qos_set_policer(&policer_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_show_policer_stats,
        ctc_cli_qos_show_policer_stats_cmd,
        "show qos policer stats (port GPORT (in|out) |flow POLICER_ID|service SERVICE_ID|copp POLICER_ID|vlan VLAN_ID(in|out)) \
        (hbwp-en cos-index INDEX |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_FLOW_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service policer",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        CTC_CLI_QOS_VLAN_PLC_STR,
        CTC_CLI_QOS_VLAN_PLC_VAL,
        "Ingress direction",
        "Egress direction",
        "Hbwp policer",
        "Specify the cos index",
        "Value")
{

    ctc_qos_policer_stats_t stats_param;
    char   stats_pkts[UINT64_STR_LEN], stats_bytes[UINT64_STR_LEN];
    int32 ret = CLI_SUCCESS;
    int16 idx = 0;
    int16 idx1 = 0;
    uint8 dir = 0;
    uint16 gport = 0;

    sal_memset(&stats_param, 0, sizeof(ctc_qos_policer_stats_t));

    /* port policer */
    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("port", gport, argv[idx + 1]);
        stats_param.id.gport = gport;
        stats_param.type = CTC_QOS_POLICER_TYPE_PORT;

        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        stats_param.dir = dir;

    }

    /* flow policer */
    idx = CTC_CLI_GET_ARGC_INDEX("flow");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("policer id", stats_param.id.policer_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    /* service policer */
    idx = CTC_CLI_GET_ARGC_INDEX("service");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("service id", stats_param.id.service_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_SERVICE;
    }

    /* copp policer */
    idx = CTC_CLI_GET_ARGC_INDEX("copp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("copp id", stats_param.id.policer_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_COPP;
    }

    /* vlan policer */
    idx = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("vlan id", stats_param.id.vlan_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_VLAN;
        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }
        stats_param.dir = dir;
    }
    /* hbwp policer */
    idx1 = CTC_CLI_GET_ARGC_INDEX("hbwp-en");
    if (0xFF != idx1)
    {
        stats_param.hbwp_en = 1;
        CTC_CLI_GET_UINT16("cos-index", stats_param.cos_index, argv[idx1 + 2]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_query_policer_stats(g_api_lchip, &stats_param);
    }
    else
    {
        ret = ctc_qos_query_policer_stats(&stats_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("policer id = %u\n", stats_param.id.policer_id);
    ctc_cli_out("===========================\n");
    ctc_uint64_to_str(stats_param.stats.confirm_pkts, stats_pkts);
    ctc_uint64_to_str(stats_param.stats.confirm_bytes, stats_bytes);
    ctc_cli_out("confirm stats, packet = %s, bytes = %s\n", stats_pkts, stats_bytes);
    ctc_uint64_to_str(stats_param.stats.exceed_pkts, stats_pkts);
    ctc_uint64_to_str(stats_param.stats.exceed_bytes, stats_bytes);
    ctc_cli_out("exceed stats, packet = %s, bytes = %s\n", stats_pkts, stats_bytes);
    ctc_uint64_to_str(stats_param.stats.violate_pkts, stats_pkts);
    ctc_uint64_to_str(stats_param.stats.violate_bytes, stats_bytes);
    ctc_cli_out("violate stats, packet = %s, bytes = %s\n", stats_pkts, stats_bytes);

    return ret;
}

CTC_CLI(ctc_cli_qos_clear_policer_stats,
        ctc_cli_qos_clear_policer_stats_cmd,
        "clear qos policer stats (port GPORT (in|out) |flow POLICER_ID|service SERVICE_ID|copp POLICER_ID|vlan VLAN_ID(in|out)) \
        (hbwp-en cos-index INDEX |)",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        "Statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_FLOW_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service policer",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        CTC_CLI_QOS_VLAN_PLC_STR,
        CTC_CLI_QOS_VLAN_PLC_VAL,
        "Ingress direction",
        "Egress direction",
        "Hbwp policer",
        "Specify the cos index",
        "Value")
{
    ctc_qos_policer_stats_t stats_param;
    int32 ret = CLI_SUCCESS;
    int16 idx = 0;
    int16 idx1 = 0;
    uint8 dir = 0;
    uint16 gport = 0;
    sal_memset(&stats_param, 0, sizeof(ctc_qos_policer_stats_t));

    /* port policer */
    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("port", gport, argv[idx + 1]);
        stats_param.id.gport = gport;
        stats_param.type = CTC_QOS_POLICER_TYPE_PORT;

        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }

        stats_param.dir = dir;
    }

    /* flow policer */
    idx = CTC_CLI_GET_ARGC_INDEX("flow");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("policer id", stats_param.id.policer_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_FLOW;
    }

    /* service policer */
    idx = CTC_CLI_GET_ARGC_INDEX("service");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("service id", stats_param.id.service_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_SERVICE;
    }

    /* copp policer */
    idx = CTC_CLI_GET_ARGC_INDEX("copp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("copp id", stats_param.id.policer_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_COPP;
    }

    /* vlan policer */
    idx = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("vlan id", stats_param.id.vlan_id, argv[idx + 1]);
        stats_param.type = CTC_QOS_POLICER_TYPE_VLAN;
        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }

        stats_param.dir = dir;
    }
    /* hbwp policer */
    idx1 = CTC_CLI_GET_ARGC_INDEX("hbwp-en");
    if (0xFF != idx1)
    {
        stats_param.hbwp_en = 1;
        CTC_CLI_GET_UINT16("cos-index", stats_param.cos_index, argv[idx1 + 2]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_clear_policer_stats(g_api_lchip, &stats_param);
    }
    else
    {
        ret = ctc_qos_clear_policer_stats(&stats_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_show_policer,
        ctc_cli_qos_show_policer_cmd,
        "show qos policer (port GPORT (in|out) |flow POLICER_ID|service SERVICE_ID|vlan VLAN_ID(in|out)|copp POLICER_ID | mfp POLICER_ID) \
        (hbwp-en cos-index INDEX |)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_FLOW_PLC_STR,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service policer",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_QOS_VLAN_PLC_STR,
        CTC_CLI_QOS_VLAN_PLC_VAL,
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Ingress direction",
        "Egress direction",
        CTC_CLI_QOS_COPP,
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Micro flow policer for ipfix",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Hbwp policer",
        "Specify the cos index",
        "Cos-index Value")
{
    ctc_qos_policer_t policer_param;
    int32 ret = CLI_SUCCESS;
    int16 idx = 0;
    int16 idx1 = 0;
    uint8 dir = 0;
    uint16 gport = 0;
    uint32 policer_id = 0;
    char policer_mode[5][32] = {"stbm","rfc2697", "rfc2698", "rfc4115", "bwp"};
    sal_memset(&policer_param, 0, sizeof(ctc_qos_policer_t));

    /* port policer */
    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("port", gport, argv[idx + 1]);
        policer_param.id.gport = gport;
        policer_param.type = CTC_QOS_POLICER_TYPE_PORT;
        policer_id = gport;

        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }

        policer_param.dir = dir;
    }

    /* flow policer */
    idx = CTC_CLI_GET_ARGC_INDEX("flow");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_FLOW;
        policer_id = policer_param.id.policer_id;
    }

    /* service policer */
    idx = CTC_CLI_GET_ARGC_INDEX("service");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("service id", policer_param.id.service_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_SERVICE;
        policer_id = policer_param.id.service_id;
    }

    /* vlan policer */
    idx = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("vlan id", policer_param.id.vlan_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_VLAN;
        /* direction */
        if (CLI_CLI_STR_EQUAL("in", idx + 2))
        {
            dir = CTC_INGRESS;
        }
        else
        {
            dir = CTC_EGRESS;
        }

        policer_param.dir = dir;
        policer_id = policer_param.id.vlan_id;
    }

    /* copp */
    idx = CTC_CLI_GET_ARGC_INDEX("copp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_COPP;
        policer_id = policer_param.id.policer_id;
    }

    /* copp */
    idx = CTC_CLI_GET_ARGC_INDEX("mfp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("policer id", policer_param.id.policer_id, argv[idx + 1]);
        policer_param.type = CTC_QOS_POLICER_TYPE_MFP;
        policer_id = policer_param.id.policer_id;
    }

    /* hbwp policer */
    idx1 = CTC_CLI_GET_ARGC_INDEX("hbwp-en");
    if (0xFF != idx1)
    {
        policer_param.hbwp_en = 1;
        CTC_CLI_GET_UINT8("cos-index", policer_param.hbwp.cos_index, argv[idx1 + 2]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_policer(g_api_lchip, &policer_param);
    }
    else
    {
        ret = ctc_qos_get_policer(&policer_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if (policer_param.type == CTC_QOS_POLICER_TYPE_PORT)
    {
        ctc_cli_out("%-6s %-7s %-7s", "port", "cos_idx", "enable");
    }
    else if (policer_param.type == CTC_QOS_POLICER_TYPE_VLAN)
    {
        ctc_cli_out("%-6s %-7s %-7s", "vlan-id", "cos_idx", "enable");
    }
    else if ((policer_param.type == CTC_QOS_POLICER_TYPE_FLOW) || (policer_param.type == CTC_QOS_POLICER_TYPE_COPP) || (policer_param.type == CTC_QOS_POLICER_TYPE_MFP))
    {
        ctc_cli_out( "%-6s %-7s %-7s", "plc-id", "cos_idx", "enable");
    }
    else
    {
        ctc_cli_out( "%-6s %-7s %-7s", "service-id", "cos_idx", "enable");
    }
    ctc_cli_out( "%-13s %-13s %-12s %-12s %-8s %-7s\n", "cir(kbps/pps)", "pir(kbps/pps)", "cbs(kb/pkt)", "pbs(kb/pkt)",
                             "stats-en", "pps-en");
    ctc_cli_out("--------------------------------------------------------------------------------------------\n");
    ctc_cli_out("0x%04x %-7d %-6d ", policer_id, policer_param.hbwp.cos_index, 1);
    ctc_cli_out("%-13d %-13d %-12d %-12d %-8d %-7d\n", policer_param.policer.cir, policer_param.policer.pir, policer_param.policer.cbs,
                policer_param.policer.pbs, policer_param.policer.stats_en,policer_param.pps_en);
    ctc_cli_out("\nDetail information:\n");
    ctc_cli_out("----------------------------------------------------------\n");
    ctc_cli_out("%-7s %-11s %-11s %-4s %-8s %-8s %-8s %-8s %-7s %-8s\n","stats-mode","policer-mode","color-aware","level","use-l3-length","drop-color","action-pri-green","action-pri-yellow","action-pri-red","color-merge-mod");
    ctc_cli_out("-----------------------------------------------------------------------------------------------------------------------\n");
    ctc_cli_out("%-11d %-11s %-11d %-4d %-17d %-7d %-17d %-17d%-17d%-17d\n",policer_param.policer.stats_mode,policer_mode[policer_param.policer.policer_mode],
                policer_param.policer.is_color_aware,policer_param.level-1,policer_param.policer.use_l3_length, policer_param.policer.drop_color, policer_param.action.prio_green,
                policer_param.action.prio_yellow,policer_param.action.prio_red,policer_param.policer.color_merge_mode);
    if(policer_param.hbwp_en == 1)
    {
        ctc_cli_out("%-7s %-7s %-11s %-7s %-8s %-8s\n","hbwp-en","sf-en","cf-total-dis","cir_max(kbps/pps)","pir_max(kbps/pps)","cf_en");
        ctc_cli_out("------------------------------------------------------------------------------------\n");
        ctc_cli_out("%-7d %-7d %-17d %-17d %-14d %-7d \n",policer_param.hbwp_en,policer_param.hbwp.sf_en,policer_param.hbwp.cf_total_dis,
                    policer_param.hbwp.cir_max,policer_param.hbwp.pir_max, policer_param.policer.cf_en);
    }

    return ret;
}

#define debug_cli ""

CTC_CLI(ctc_cli_qos_debug_on,
        ctc_cli_qos_debug_on_cmd,
        "debug qos (ctc | sys) (policer | class| queue) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_QOS_STR,
        "Ctc layer",
        "Sys layer",
        "QoS policer",
        "QoS classification",
        "QoS queue",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        if (CLI_CLI_STR_EQUAL("pol", 1))
        {
            ctc_debug_set_flag("qos", "policer", QOS_PLC_CTC, level, TRUE);
        }
        else if (CLI_CLI_STR_EQUAL("cla", 1))
        {
            ctc_debug_set_flag("qos", "class", QOS_CLASS_CTC, level, TRUE);
        }
        else
        {
            ctc_debug_set_flag("qos", "queue", QOS_QUE_CTC, level, TRUE);
        }
    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        if (CLI_CLI_STR_EQUAL("pol", 1))
        {
            ctc_debug_set_flag("qos", "policer", QOS_PLC_SYS, level, TRUE);
        }
        else if (CLI_CLI_STR_EQUAL("cla", 1))
        {
            ctc_debug_set_flag("qos", "class", QOS_CLASS_SYS, level, TRUE);
        }
        else
        {
            ctc_debug_set_flag("qos", "queue", QOS_QUE_SYS, level, TRUE);
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_debug_off,
        ctc_cli_qos_debug_off_cmd,
        "no debug qos (ctc | sys) (policer | class| queue)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_QOS_STR,
        "Ctc layer",
        "Sys layer",
        "QoS policer",
        "QoS classification",
        "QoS queue")
{
    uint8 level = 0;

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        if (CLI_CLI_STR_EQUAL("pol", 1))
        {
            ctc_debug_set_flag("qos", "policer", QOS_PLC_CTC, level, FALSE);
        }
        else if (CLI_CLI_STR_EQUAL("cla", 1))
        {
            ctc_debug_set_flag("qos", "class", QOS_CLASS_CTC, level, FALSE);
        }
        else
        {
            ctc_debug_set_flag("qos", "queue", QOS_QUE_CTC, level, FALSE);
        }
    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        if (CLI_CLI_STR_EQUAL("pol", 1))
        {
            ctc_debug_set_flag("qos", "policer", QOS_PLC_SYS, level, FALSE);
        }
        else if (CLI_CLI_STR_EQUAL("cla", 1))
        {
            ctc_debug_set_flag("qos", "class", QOS_CLASS_SYS, level, FALSE);
        }
        else
        {
            ctc_debug_set_flag("qos", "queue", QOS_QUE_SYS, level, FALSE);
        }
    }

    return CLI_SUCCESS;
}

#define domain_map_cli ""

CTC_CLI(ctc_cli_qos_set_igs_domain_map,
        ctc_cli_qos_set_igs_domain_map_cmd,
        "qos (domain DOMAIN) map (cos COS (dei DEI |)| dscp DSCP (ecn ECN |)| ip-prec IP_PREC |exp EXP) \
        to (priority PRIORITY) (color (green |yellow | red))",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_DOMAIN_STR,
        CTC_CLI_QOS_DOMAIN_VALUE,
        "Mapping",
        "Cos",
        "<0-7>",
        "Dei",
        "<0-1>",
        "Dscp",
        "<0-63>",
        "Ecn",
        "ECN value",
        "Ip precedence",
        "<0-7>",
        "Exp",
        "<0-7>",
        "To",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Color",
        "Green",
        "Yellow",
        "Red")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_domain_map_t domain_map;
    uint16 index = 0;


    sal_memset(&domain_map, 0, sizeof(ctc_qos_domain_map_t));

    index = CTC_CLI_GET_ARGC_INDEX("domain");
    if (0xFF != index)
    {
        /* qos domain */
        CTC_CLI_GET_UINT8("qos domain", domain_map.domain_id, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        /* cos */
        CTC_CLI_GET_UINT8("cos", domain_map.hdr_pri.dot1p.cos, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR;

        index = CTC_CLI_GET_ARGC_INDEX("dei");
        if (0xFF != index)
        {
            /* dei */
            CTC_CLI_GET_UINT8("dei", domain_map.hdr_pri.dot1p.dei, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != index)
    {
        /* dscp */
        CTC_CLI_GET_UINT8("dscp", domain_map.hdr_pri.tos.dscp, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR;

        index = CTC_CLI_GET_ARGC_INDEX("ecn");
        if (0xFF != index)
        {
            /* ecn */
            CTC_CLI_GET_UINT8("ecn", domain_map.hdr_pri.tos.ecn, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-prec");
    if (0xFF != index)
    {
        /* ip-prec */
        CTC_CLI_GET_UINT8("ip-prec", domain_map.hdr_pri.ip_prec, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR;
    }


    index = CTC_CLI_GET_ARGC_INDEX("exp");
    if (0xFF != index)
    {
        /* exp */
        CTC_CLI_GET_UINT8("exp", domain_map.hdr_pri.exp, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR;
    }


    /* priority */
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        /* priority */
        CTC_CLI_GET_UINT8("priority", domain_map.priority, argv[index + 1]);
    }


    /* color */
    index = CTC_CLI_GET_ARGC_INDEX("green");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_GREEN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("yellow");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_YELLOW;
    }

    index = CTC_CLI_GET_ARGC_INDEX("red");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_RED;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_domain_map(g_api_lchip, &domain_map);
    }
    else
    {
        ret = ctc_qos_set_domain_map(&domain_map);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_qos_set_egs_domain_map,
        ctc_cli_qos_set_egs_domain_map_cmd,
        "qos (domain DOMAIN) map (priority PRIORITY) (color (green | yellow | red)) to \
        (cos COS (dei DEI |)| dscp DSCP | exp EXP)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_DOMAIN_STR,
        CTC_CLI_QOS_DOMAIN_VALUE,
        "Mapping",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Color",
        "Green",
        "Yellow",
        "Red",
        "To",
        "Cos",
        "<0-7>",
        "Dei",
        "<0-1>",
        "Dscp",
        "<0-63>",
        "Exp",
        "<0-7>")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_domain_map_t domain_map;
    uint16 index = 0;

    sal_memset(&domain_map, 0, sizeof(ctc_qos_domain_map_t));

    index = CTC_CLI_GET_ARGC_INDEX("domain");
    if (0xFF != index)
    {
        /* qos domain */
        CTC_CLI_GET_UINT8("qos domain", domain_map.domain_id, argv[index + 1]);
    }

    /* priority */
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("priority", domain_map.priority, argv[index + 1]);
    }


    /* color */
    index = CTC_CLI_GET_ARGC_INDEX("green");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_GREEN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("yellow");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_YELLOW;
    }

    index = CTC_CLI_GET_ARGC_INDEX("red");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_RED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        /* cos */
        CTC_CLI_GET_UINT8("cos", domain_map.hdr_pri.dot1p.cos, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;

        index = CTC_CLI_GET_ARGC_INDEX("dei");
        if (0xFF != index)
        {
            /* dei */
            CTC_CLI_GET_UINT8("dei", domain_map.hdr_pri.dot1p.dei, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != index)
    {
        /* dscp */
        CTC_CLI_GET_UINT8("dscp", domain_map.hdr_pri.tos.dscp, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("exp");
    if (0xFF != index)
    {
        /* exp */
        CTC_CLI_GET_UINT8("exp", domain_map.hdr_pri.exp, argv[index + 1]);
        domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_domain_map(g_api_lchip, &domain_map);
    }
    else
    {
        ret = ctc_qos_set_domain_map(&domain_map);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_show_qos_domain,
        ctc_cli_qos_show_qos_domain_cmd,
        "show qos (domain DOAMIN) type (cos-map-pri(cos COS_VALUE (dei DEI|))|ip-prec-map-pri(ip-prec IP_PREC) \
        |dscp-map-pri(dscp DSCP_VAlUE (ecn ECN|))|exp-map-pri(exp EXP_VALUE)|pri-map-cos(priority PRI_VALUE color (green | yellow | red)) \
        |pri-map-dscp(priority PRI_VALUE color (green | yellow | red))|pri-map-exp(priority PRI_VALUE color (green | yellow | red)))",
        "Show",
        CTC_CLI_QOS_STR,
        "QoS domain",
        CTC_CLI_QOS_DOMAIN_VALUE,
        "Maping type",
        "Cos map to priority color",
        "Cos",
        "Cos vaule",
        "Dei",
        "Dei vaule",
        "Ip-prec map to priority color",
        "Ip precedence",
        "Ip precedence value",
        "Dscp map to priority color",
        "Dscp",
        "Dscp value",
        "Ecn",
        "ECN value",
        "Exp map to priority color",
        "Exp",
        "Exp value",
        "Priority color map to cos",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Color",
        "Green",
        "Yellow",
        "Red",
        "Priority color map to dscp",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Color",
        "Green",
        "Yellow",
        "Red",
        "Priority color map to exp",
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Color",
        "Green",
        "Yellow",
        "Red")
{
    int32 ret = CLI_SUCCESS;
    uint8 type = 0;
    uint8 index = 0;
    ctc_qos_domain_map_t domain_map;

    sal_memset(&domain_map, 0, sizeof(ctc_qos_domain_map_t));

    index = CTC_CLI_GET_ARGC_INDEX("domain");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("qos domain", domain_map.domain_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cos-map-pri");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-prec-map-pri");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dscp-map-pri");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("exp-map-pri");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pri-map-cos");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pri-map-dscp");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pri-map-exp");
    if (0xFF != index)
    {
        type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP;
    }

    domain_map.type = type;

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        /* cos */
        CTC_CLI_GET_UINT8("cos", domain_map.hdr_pri.dot1p.cos, argv[index + 1]);

        index = CTC_CLI_GET_ARGC_INDEX("dei");
        if (0xFF != index)
        {
            /* dei */
            CTC_CLI_GET_UINT8("dei", domain_map.hdr_pri.dot1p.dei, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != index)
    {
        /* dscp */
        CTC_CLI_GET_UINT8("dscp", domain_map.hdr_pri.tos.dscp, argv[index + 1]);

        index = CTC_CLI_GET_ARGC_INDEX("ecn");
        if (0xFF != index)
        {
            /* ecn */
            CTC_CLI_GET_UINT8("ecn", domain_map.hdr_pri.tos.ecn, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-prec");
    if (0xFF != index)
    {
        /* ip-prec */
        CTC_CLI_GET_UINT8("ip-prec", domain_map.hdr_pri.ip_prec, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("exp");
    if (0xFF != index)
    {
        /* exp */
        CTC_CLI_GET_UINT8("exp", domain_map.hdr_pri.exp, argv[index + 1]);
    }


    /* priority */
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        /* priority */
        CTC_CLI_GET_UINT8("priority", domain_map.priority, argv[index + 1]);
    }


    /* color */
    index = CTC_CLI_GET_ARGC_INDEX("green");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_GREEN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("yellow");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_YELLOW;
    }

    index = CTC_CLI_GET_ARGC_INDEX("red");
    if (0xFF != index)
    {
        domain_map.color = CTC_QOS_COLOR_RED;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_domain_map(g_api_lchip, &domain_map);
    }
    else
    {
        ret = ctc_qos_get_domain_map(&domain_map);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    switch (type)
    {
         case CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR:
            {
                 ctc_cli_out("ingress cos + dei -> priority + color mapping table\n");
                 ctc_cli_out("-----------------------------------------------------\n");
                 ctc_cli_out("   cos(%d)  +  dei(%d) -> priority(%d)  + color(%d)\n",
                            domain_map.hdr_pri.dot1p.cos, domain_map.hdr_pri.dot1p.dei, domain_map.priority, domain_map.color);
            }
            break;

         case CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR:
            {
                ctc_cli_out("ingress ip_prec -> priority + color mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out("    ip_prec(%d) -> priority(%d)  + color(%d)\n",
                            domain_map.hdr_pri.ip_prec, domain_map.priority, domain_map.color);
            }
            break;

         case CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR:
            {
                ctc_cli_out("ingress dscp + ecn -> priority + color mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out("    dscp(%d) + ecn(%d) -> priority(%d)  + color(%d)\n",
                            domain_map.hdr_pri.tos.dscp, domain_map.hdr_pri.tos.ecn, domain_map.priority, domain_map.color);
            }
            break;

         case CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR:
            {
                ctc_cli_out("ingress exp -> priority + color mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out("    exp(%d) -> priority(%d)  + color(%d)\n",
                            domain_map.hdr_pri.exp, domain_map.priority, domain_map.color);
            }
            break;
         case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS:
            {
                ctc_cli_out("egress  priority + color -> cos  mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out(" priority(%d)  + color(%d)  -> cos(%d) + dei(%d)\n",
                              domain_map.priority, domain_map.color, domain_map.hdr_pri.dot1p.cos, domain_map.hdr_pri.dot1p.dei);
            }
            break;

         case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP:
            {
                ctc_cli_out("egress  priority + color -> dscp  mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out(" priority(%d)  + color(%d)  -> dscp(%d)\n",
                              domain_map.priority, domain_map.color, domain_map.hdr_pri.tos.dscp);
            }
            break;

         case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP:
            {
                ctc_cli_out("egress  priority + color -> exp  mapping table\n");
                ctc_cli_out("-----------------------------------------------------\n");
                ctc_cli_out(" priority(%d)  + color(%d)  -> exp(%d)\n",
                              domain_map.priority, domain_map.color, domain_map.hdr_pri.exp);
            }
            break;

    }

    return ret;
}
#define queue_cli ""

CTC_CLI(ctc_cli_qos_set_queue_pri_map,
        ctc_cli_qos_set_queue_pri_map_cmd,
        "qos queue (priority-mapping PRIORITY COLOR) to (qsel QSEL) (drop-prec DROP_PREC)",
        CTC_CLI_QOS_STR,
        "QoS Queue",
        "Priority Mapping",
        CTC_CLI_PRIORITY_VALUE,
        CTC_CLI_COLOR_VALUE,
        "To",
        "Queue select",
        CTC_CLI_QOS_QUEUE_VAL,
        "Drop precedence",
        CTC_CLI_QOS_DROP_PRECEDENCE)
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));

    que_cfg.type = CTC_QOS_QUEUE_CFG_PRI_MAP;

    index = CTC_CLI_GET_ARGC_INDEX("priority-mapping");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("priority", que_cfg.value.pri_map.priority, argv[index + 1]);

        CTC_CLI_GET_UINT16("color", que_cfg.value.pri_map.color, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("qsel");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("qsel", que_cfg.value.pri_map.queue_select, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop-prec");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("drop-prec", que_cfg.value.pri_map.drop_precedence, argv[index + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#define sched_cli ""

CTC_CLI(ctc_cli_qos_set_port_queue_class,
        ctc_cli_qos_set_port_queue_class_cmd,
        "qos schedule queue-class" CTC_CLI_QOS_QUEUE_ID_STR "(confirm-class CFMCLASS | exceed-class ECDCLASS)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Queue class",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Confirm green class",
        CTC_CLI_QOS_CLASS_VAL,
        "Exceed yellow class",
        CTC_CLI_QOS_CLASS_VAL)
{
    uint8  queue_class = 0;
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));

	sched_param.type = CTC_QOS_SCHED_QUEUE;

    _ctc_cli_qos_queue_id_parser(&sched_param.sched.queue_sched.queue, &argv[0], argc);

    index = CTC_CLI_GET_ARGC_INDEX("confirm-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
        CTC_CLI_GET_UINT8_RANGE("confirm class", queue_class, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        sched_param.sched.queue_sched.confirm_class = queue_class;

    }

    index = CTC_CLI_GET_ARGC_INDEX("exceed-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
        CTC_CLI_GET_UINT8_RANGE("exceed class", queue_class, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        sched_param.sched.queue_sched.exceed_class = queue_class;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_set_sched(&sched_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_port_group_class_priority,
        ctc_cli_qos_set_port_group_class_priority_cmd,
        "qos schedule pri-propagation" CTC_CLI_QOS_QUEUE_ID_STR "(queue-class CLASS) (priority PRIORITY)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Queue class propagation to Priority",
         CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Queue class",
        CTC_CLI_QOS_CLASS_VAL,
        "Priority",
        "Value <0-3>")
{
    uint16 gport = 0;
    uint16 service_id = 0;
    uint8  queue_class = 0;
    uint8  priority = 0;
    uint8  index = 0;

    int32 ret = CLI_SUCCESS;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));
    sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
    sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;


    /* service id */
    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_SERVICE;
        sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
        sched_param.sched.group_sched.queue.service_id = service_id;

    }

    /* port */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        sched_param.sched.group_sched.queue.gport = gport;
        sched_param.sched.group_sched.queue.queue_id = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF != index)
    {
        /* queue-id */
        CTC_CLI_GET_UINT16("queue-id", sched_param.sched.group_sched.queue.queue_id, argv[index + 1]);
    }

   index = CTC_CLI_GET_ARGC_INDEX("reason-id");
    if (0xFF != index)
    {
        /* reason id */
        CTC_CLI_GET_UINT16("reason-id", gport, argv[index+1]);
        sched_param.sched.group_sched.queue.cpu_reason = gport;
	    sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    }

    /* queue class */
    index = CTC_CLI_GET_ARGC_INDEX("queue-class");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("queue-class", queue_class, argv[index + 1]);
    }


    /* queue class priority*/
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("priority", priority, argv[index + 1]);
    }


    sched_param.type = CTC_QOS_SCHED_GROUP;
    sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    sched_param.sched.group_sched.service_id = service_id;
    sched_param.sched.group_sched.gport = gport;
    sched_param.sched.group_sched.queue_class = queue_class;
    sched_param.sched.group_sched.class_priority = priority;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_set_sched(&sched_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_qos_set_queue_wdrr_weight,
        ctc_cli_qos_set_queue_wdrr_weight_cmd,
        "qos schedule queue-wdrr (service-id SERVICE |) (port GPHYPORT_ID)  (confirm-class| exceed-class) \
    (wdrr-weight WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT WEIGHT |queue-id QUEUE_ID weight WEIGHT)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Queue wdrr",
        CTC_CLI_QOS_SERVICE_STR,
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Confirm green class weight",
        "Exceed yellow class weight",
        "Config WDRR weight",
        "Weight of queue 0, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 1, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 2, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 3, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 4, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 5, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 6, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Weight of queue 7, "CTC_CLI_QOS_WDRR_WEIGHT_VAL,
        "Queue id",
        CTC_CLI_QOS_QUEUE_VAL,
        "Config WDRR weight",
        CTC_CLI_QOS_WDRR_WEIGHT_VAL)
{
    uint16 gport = 0;
    uint16 weight = 0;
    uint16 service_id = 0;
    int8   qid = 0;
    int32  ret = 0;
    uint16 index = 0;
    uint8 is_confirm_class = 0;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));

    sched_param.type = CTC_QOS_SCHED_QUEUE;
    sched_param.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;


    /* service id */
    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        sched_param.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
        sched_param.sched.queue_sched.queue.service_id = service_id;
    }

    /* port */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        sched_param.sched.queue_sched.queue.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("confirm-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT;
        is_confirm_class = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("exceed-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("wdrr-weight");
    if (0xFF != index)
    {
        /* weight values */
        for (qid = 0; qid < CTC_QUEUE_NUM_PER_PORT; qid++)
        {
            CTC_CLI_GET_UINT16_RANGE("weight", weight, argv[qid + index + 1], 0, CTC_MAX_UINT16_VALUE);

            sched_param.sched.queue_sched.queue.queue_id = qid;

            if (is_confirm_class)
            {
                sched_param.sched.queue_sched.confirm_weight = weight;
            }
            else
            {
                sched_param.sched.queue_sched.exceed_weight = weight;
            }

            if(g_ctcs_api_en)
            {
                 ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
            }
            else
            {
                ret = ctc_qos_set_sched(&sched_param);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            /*ctc_cli_update_queue_drop_thrd(&sched_param.sched.queue_sched.queue,weight);*/

        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("queueId", qid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT16_RANGE("weight", weight, argv[index + 3], 0, CTC_MAX_UINT16_VALUE);

        sched_param.sched.queue_sched.queue.queue_id = qid;
        if (is_confirm_class)
        {
            sched_param.sched.queue_sched.confirm_weight = weight;
        }
        else
        {
            sched_param.sched.queue_sched.exceed_weight = weight;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
        }
        else
        {
            ret = ctc_qos_set_sched(&sched_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        /*ctc_cli_update_queue_drop_thrd(&sched_param.sched.queue_sched.queue, weight);*/
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_qos_set_queue_wdrr_cpu_reason_weight,
        ctc_cli_qos_set_queue_wdrr_cpu_reason_weight_cmd,
        "qos schedule queue-wdrr (reason-id REASON) (confirm-class| exceed-class) (weight WEIGHT)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Queue wdrr",
        "Cpu reason",
        "Cpu reason id",
        "Confirm green class weight",
        "Exceed yellow class weight",
        "Config WDRR weight",
        CTC_CLI_QOS_WDRR_WEIGHT_VAL)
{
    uint16 gport = 0;
    uint16 weight = 0;


    int32  ret = 0;
    uint16 index = 0;
    uint8 is_confirm_class = 0;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));

    sched_param.type = CTC_QOS_SCHED_QUEUE;

    index = CTC_CLI_GET_ARGC_INDEX("reason-id");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT16("reason-id", gport, argv[index+1]);
        sched_param.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        sched_param.sched.queue_sched.queue.cpu_reason = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("confirm-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT;
        is_confirm_class = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("exceed-class");
    if (0xFF != index)
    {
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("weight");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("weight", weight, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        if (is_confirm_class)
        {
            sched_param.sched.queue_sched.confirm_weight = weight;
        }
        else
        {
            sched_param.sched.queue_sched.exceed_weight = weight;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
        }
        else
        {
            ret = ctc_qos_set_sched(&sched_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        /*ctc_cli_update_queue_drop_thrd(&sched_param.sched.queue_sched.queue, weight);*/
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_qos_set_group_wdrr_weight,
        ctc_cli_qos_set_group_wdrr_weight_cmd,
        "qos schedule group-wdrr" CTC_CLI_QOS_QUEUE_ID_STR "(weight WEIGHT)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Group",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Config WDRR weight",
        CTC_CLI_QOS_WDRR_WEIGHT_VAL)
{
    uint16 gport = 0;
    uint16 service_id = 0;
    uint16 weight = 0;
    int32  ret = 0;
    uint16 index = 0;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));

    sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
    sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    /* service id */
    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_SERVICE;
        sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
        sched_param.sched.group_sched.queue.service_id = service_id;
    }

    /* port */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        sched_param.sched.group_sched.queue.gport = gport;
        sched_param.sched.group_sched.queue.queue_id = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF != index)
    {
        /* queue-id */
        CTC_CLI_GET_UINT16("queue-id", sched_param.sched.group_sched.queue.queue_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("reason-id");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT16("reason-id", gport, argv[index+1]);
        sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        sched_param.sched.group_sched.queue.cpu_reason = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("weight");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("weight", weight, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    sched_param.type = CTC_QOS_SCHED_GROUP;
    sched_param.sched.group_sched.service_id = service_id;
    sched_param.sched.group_sched.gport = gport;
    sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT;
    sched_param.sched.group_sched.weight = weight;

    if(g_ctcs_api_en)
    {
         ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_set_sched(&sched_param);
    }
    if (ret < 0)
   {
       ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
       return CLI_ERROR;
   }


    return CLI_SUCCESS;
}




CTC_CLI(ctc_cli_qos_set_queue_wdrr_weight_mtu,
        ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd,
        "qos schedule wdrr-weight-mtu MTU",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Set queue wdrr weight mtu",
        "<1-0xFFFF>")
{
    uint32 mtu = 0;
    int32  ret = CLI_SUCCESS;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    que_cfg.type = CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU;

    /* mtu */
    CTC_CLI_GET_UINT16_RANGE("mtu", mtu, argv[0], 0, CTC_MAX_UINT16_VALUE);
    que_cfg.value.value_32 = mtu;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_qos_set_schedule_wrr_en,
        ctc_cli_qos_set_schedule_wrr_en_cmd,
        "qos schedule wrr (enable|disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "WRR Schedule",
        "Enable",
        "Disable")
{
    ctc_qos_glb_cfg_t glb_cfg;
    int32 ret = CLI_SUCCESS;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SCH_WRR_EN;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        glb_cfg.u.value = 1;
    }

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
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_port_sub_group_id,
        ctc_cli_qos_set_port_sub_group_id_cmd,
        "qos schedule (port GPHYPORT_ID )(queue-class CLASS sub-group GROUP_ID | sub-group GROUP_ID weight WEIGHT)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Queue class",
        CTC_CLI_QOS_CLASS_VAL,
        "Sub group id",
        "Group id value",
        "Sub group id",
        "Group id value",
        "Config WDRR weight",
        CTC_CLI_QOS_WDRR_WEIGHT_VAL)
{
    uint16 gport = 0;
    uint8  queue_class = 0;
    uint8  index = 0;

    int32 ret = CLI_SUCCESS;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));
    sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    /* port */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        sched_param.sched.group_sched.queue.gport = gport;
        sched_param.sched.group_sched.queue.queue_id = 0;
    }

    /* queue class */
    index = CTC_CLI_GET_ARGC_INDEX("queue-class");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("queue-class", queue_class, argv[index + 1]);
        sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_SUB_GROUP_ID;
    }

     /* sub group */
    index = CTC_CLI_GET_ARGC_INDEX("sub-group");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("sub-group", sched_param.sched.group_sched.sub_group_id, argv[index + 1]);
    }

     /* weight */
    index = CTC_CLI_GET_ARGC_INDEX("weight");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("weight", sched_param.sched.group_sched.weight, argv[index + 1]);
        sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_SUB_GROUP_WEIGHT;
    }

    sched_param.type = CTC_QOS_SCHED_GROUP;
    sched_param.sched.group_sched.queue_class = queue_class;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_set_sched(&sched_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_qos_show_port_sub_group_id,
        ctc_cli_qos_show_port_sub_group_id_cmd,
        "show qos schedule (port GPHYPORT_ID )(queue-class CLASS)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Queue class",
        CTC_CLI_QOS_CLASS_VAL)
{
    uint16 gport = 0;
    uint8  queue_class = 0;
    uint8  index = 0;

    int32 ret = CLI_SUCCESS;
    ctc_qos_sched_t sched_param;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));
    sched_param.sched.group_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    /* port */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        sched_param.sched.group_sched.queue.gport = gport;
        sched_param.sched.group_sched.queue.queue_id = 0;
    }

    /* queue class */
    index = CTC_CLI_GET_ARGC_INDEX("queue-class");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("queue-class", queue_class, argv[index + 1]);
        sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_SUB_GROUP_ID;
    }

    sched_param.type = CTC_QOS_SCHED_GROUP;
    sched_param.sched.group_sched.queue_class = queue_class;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_get_sched(&sched_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\nSub Group Scheduler:\n");
    ctc_cli_out("-------------------------------\n");
    ctc_cli_out("%-12s %-10s %-6s\n", "Queue-Class", "Sub group ID", "Weight");
    ctc_cli_out("%-12d %-10d %-6d\n", sched_param.sched.group_sched.queue_class, sched_param.sched.group_sched.sub_group_id,
                sched_param.sched.group_sched.weight);
    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_qos_show_sched,
        ctc_cli_qos_show_sched_cmd,
        "show qos schedule (queue|(group class CLASS))" CTC_CLI_QOS_QUEUE_ID_STR,
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SCHED_STR,
        "Queue",
        "Group",
        "Class",
        "Class value",
        CTC_CLI_QOS_QUEUE_ID_DSCP)
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_sched_t sched_param;
    uint16 service_id = 0;

    sal_memset(&sched_param, 0, sizeof(ctc_qos_sched_t));

    sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
    index = CTC_CLI_GET_ARGC_INDEX("queue");
    if (0xFF != index)
    {
        sched_param.type = CTC_QOS_SCHED_QUEUE;
        _ctc_cli_qos_queue_id_parser(&sched_param.sched.queue_sched.queue, &argv[0], argc);
        sched_param.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("class");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("class", sched_param.sched.group_sched.queue_class, argv[index + 1]);
        }
        sched_param.type = CTC_QOS_SCHED_GROUP;
        _ctc_cli_qos_queue_id_parser(&sched_param.sched.group_sched.queue, &argv[0], argc);
        index = CTC_CLI_GET_ARGC_INDEX("service-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
            sched_param.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_SERVICE;
            sched_param.sched.group_sched.service_id = service_id;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("gport", sched_param.sched.group_sched.gport, argv[index + 1]);
        }
        sched_param.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    }
    if(g_ctcs_api_en)
    {
         ret = ctcs_qos_get_sched(g_api_lchip, &sched_param);
    }
    else
    {
        ret = ctc_qos_get_sched(&sched_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if(sched_param.type == CTC_QOS_SCHED_GROUP)
    {
        ctc_cli_out("\nGroup Scheduler:\n");
        ctc_cli_out("-------------------------------\n");
        ctc_cli_out("%-12s %-10s %-6s\n", "Queue-Class", "Class-PRI", "Weight");
        ctc_cli_out("%-12d %-10d %-6d\n", sched_param.sched.group_sched.queue_class, sched_param.sched.group_sched.class_priority,
                    sched_param.sched.group_sched.weight);
    }
    else if(sched_param.type == CTC_QOS_SCHED_QUEUE)
    {
        ctc_cli_out("\nQueue Scheduler:\n");
        ctc_cli_out("-------------------------------\n");
        ctc_cli_out("%-30s: %d \n", "Queue Class(Green)", sched_param.sched.queue_sched.confirm_class);
        ctc_cli_out("%-30s: %d \n", "Weight", sched_param.sched.queue_sched.confirm_weight);
        ctc_cli_out("%-30s: %d \n", "Queue Class(Yellow)", sched_param.sched.queue_sched.exceed_class);
        ctc_cli_out("%-30s: %d \n", "Weight", sched_param.sched.queue_sched.exceed_weight);
    }

    return ret;
}
#define shape_cli ""

CTC_CLI(ctc_cli_qos_set_port_queue_shape,
        ctc_cli_qos_set_port_queue_shape_cmd,
        "qos shape queue" CTC_CLI_QOS_QUEUE_ID_STR "(none | {cir CIR (cbs CBS|) | pir PIR (pbs PBS|)})",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Queue shaping",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Cancel queue shaping",
        CTC_CLI_QOS_PLC_CIR_STR,
        CTC_CLI_QOS_PLC_CIR_VAL,
        CTC_CLI_QOS_PLC_CBS_STR,
        CTC_CLI_QOS_SHAPE_CBS_STR,
        CTC_CLI_QOS_PLC_PIR_STR,
        CTC_CLI_QOS_PLC_PIR_VAL,
        CTC_CLI_QOS_PLC_PBS_STR,
        CTC_CLI_QOS_SHAPE_PBS_STR)
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_shape_t qos_shape;
    ctc_qos_shape_queue_t shape;
    uint8 index = 0;

    sal_memset(&qos_shape, 0, sizeof(qos_shape));
    sal_memset(&shape, 0, sizeof(shape));

    _ctc_cli_qos_queue_id_parser(&shape.queue, &argv[0], argc);

    qos_shape.type = CTC_QOS_SHAPE_QUEUE;


    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (0xFF != index)
    {
        shape.enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cir");
    if (0xFF != index)
    {
        /* cir */
        CTC_CLI_GET_UINT32("cir", shape.cir, argv[index + 1]);
        shape.enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cbs");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("cbs", shape.cbs, argv[index + 1]);
    }
    else
    {
        shape.cbs = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pir");
    if (0xFF != index)
    {
        /* cir */
        CTC_CLI_GET_UINT32("pir", shape.pir, argv[index + 1]);
        shape.enable = 1;
    }
    else
    {
        shape.pir = shape.cir;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbs");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("pbs", shape.pbs, argv[index + 1]);
    }
    else
    {
        shape.pbs = 0;
    }

    sal_memcpy(&qos_shape.shape, &shape, sizeof(shape));
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_shape(g_api_lchip, &qos_shape);
    }
    else
    {
        ret = ctc_qos_set_shape(&qos_shape);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_port_shape,
        ctc_cli_qos_set_port_shape_cmd,
        "qos shape port GPHYPORT_ID (none | pir PIR (pbs PBS|)) (ecn-mark-rate ECN_MARK_RATE|)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Cancel port shaping",
        CTC_CLI_QOS_PLC_PIR_STR,
        CTC_CLI_QOS_PLC_PIR_VAL,
        CTC_CLI_QOS_PLC_PBS_STR,
        CTC_CLI_QOS_SHAPE_PBS_STR,
        "Config ecn mark rate",
        CTC_CLI_QOS_PLC_PIR_VAL)
{
    uint32 gport = 0;
    uint8 index  = 0 ;
    int32 ret = CLI_SUCCESS;
    ctc_qos_shape_t qos_shape;
    ctc_qos_shape_port_t shape;

    sal_memset(&qos_shape, 0, sizeof(ctc_qos_shape_t));
    sal_memset(&shape, 0, sizeof(ctc_qos_shape_port_t));
    /* port */
    CTC_CLI_GET_UINT32("gport", gport, argv[0]);


    shape.gport = gport;
    qos_shape.type = CTC_QOS_SHAPE_PORT;

    if (CLI_CLI_STR_EQUAL("none", 1))
    {
        shape.enable = 0;
    }
    else
    {
        /* rate */
        CTC_CLI_GET_UINT32("rate", shape.pir, argv[2]);
        shape.pbs = 0;
        shape.enable = 1;

        index = CTC_CLI_GET_ARGC_INDEX("pbs");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("pbs", shape.pbs, argv[index+1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn-mark-rate");
    if (0xFF != index)
    {
        /* ecn-mark-rate */
        CTC_CLI_GET_UINT32("ecn mark rate", shape.ecn_mark_rate, argv[index + 1]);
    }

    sal_memcpy(&qos_shape.shape, &shape, sizeof(ctc_qos_shape_port_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_shape(g_api_lchip, &qos_shape);
    }
    else
    {
        ret = ctc_qos_set_shape(&qos_shape);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_qos_set_group_shape,
        ctc_cli_qos_set_group_shape_cmd,
        "qos shape group" CTC_CLI_QOS_QUEUE_ID_STR "(none | pir PIR (pbs PBS|))",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Group shaping",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Cancel port shaping",
        CTC_CLI_QOS_PLC_PIR_STR,
        CTC_CLI_QOS_PLC_PIR_VAL,
        CTC_CLI_QOS_PLC_PBS_STR,
        CTC_CLI_QOS_SHAPE_PBS_STR)
{
    uint16 gport = 0;
    uint16 service_id = 0;
    int32 ret = CLI_SUCCESS;
    ctc_qos_shape_t qos_shape;
    ctc_qos_shape_group_t shape;
    uint8 index = 0;

    sal_memset(&qos_shape, 0, sizeof(qos_shape));
    sal_memset(&shape, 0, sizeof(shape));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT16("gport", gport, argv[index+1]);
        shape.group_type = CTC_QOS_SCHED_GROUP_PORT;
        shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        shape.queue.gport = gport;
        shape.queue.queue_id = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF != index)
    {
        /* queue-id */
        CTC_CLI_GET_UINT16("queue-id", shape.queue.queue_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("reason-id");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT16("reason-id", shape.queue.cpu_reason, argv[index+1]);
        shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;

    }
    shape.gport = gport;

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        /* service id */
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
        shape.service_id = service_id;
        shape.group_type = CTC_QOS_SCHED_GROUP_SERVICE;
        shape.queue.service_id = service_id;
        shape.queue.queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
    }


    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (0xFF != index)
    {
        shape.enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pir");
    if (0xFF != index)
    {
        /* cir */
        CTC_CLI_GET_UINT32("pir", shape.pir, argv[index + 1]);
        shape.pbs = 0;
        shape.enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbs");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("pbs", shape.pbs, argv[index + 1]);
    }

    qos_shape.type = CTC_QOS_SHAPE_GROUP;
    sal_memcpy(&qos_shape.shape, &shape, sizeof(shape));
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_shape(g_api_lchip, &qos_shape);
    }
    else
    {
        ret = ctc_qos_set_shape(&qos_shape);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_qos_set_shape_ipg_en,
        ctc_cli_qos_set_shape_ipg_en_cmd,
        "qos shape ipg (enable|disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Ipg",
        "Enable",
        "Disable")
{
    ctc_qos_glb_cfg_t glb_cfg;
    int32 ret = CLI_SUCCESS;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SHAPE_IPG_EN;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        glb_cfg.u.value = 1;
    }

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
    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_qos_set_reason_shape,
        ctc_cli_qos_set_reason_shape_cmd,
        "qos shape cpu-reason REASON_ID (none | pir PIR)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Cpu reason id",
        "<0-MAX>",
        "Cancel cpu reason shaping",
        CTC_CLI_QOS_PLC_PIR_STR,
        CTC_CLI_QOS_PLC_PIR_VAL)
{
    uint16 reason_id = 0;
    int32 ret = CLI_SUCCESS;
    ctc_qos_shape_t qos_shape;
    ctc_qos_shape_queue_t shape;

    sal_memset(&qos_shape, 0, sizeof(qos_shape));
    sal_memset(&shape, 0, sizeof(shape));

    /* reason-id */
    CTC_CLI_GET_UINT16("cpu-reason", reason_id, argv[0]);

    shape.queue.cpu_reason = reason_id;
    shape.queue.queue_id = 0;
    shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    qos_shape.type = CTC_QOS_SHAPE_QUEUE;

    if (CLI_CLI_STR_EQUAL("none", 1))
    {
        shape.enable = 0;
    }
    else
    {
        /* rate */
        CTC_CLI_GET_UINT32("rate", shape.pir, argv[2]);
        shape.pbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;
        shape.enable = 1;
    }

    sal_memcpy(&qos_shape.shape, &shape, sizeof(shape));
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_shape(g_api_lchip, &qos_shape);
    }
    else
    {
        ret = ctc_qos_set_shape(&qos_shape);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_qos_set_queue_pkt_len,
        ctc_cli_qos_set_queue_pkt_len_cmd,
        "qos shape pkt-len-adjust" CTC_CLI_QOS_QUEUE_ID_STR "(pkt-adjust-len LEN)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "Packet Length adjust",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Packet adjust len",
        "Value")
{
    uint8 index = 0;
    uint8 adjust_len = 0;
    int32 ret = CLI_SUCCESS;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(que_cfg));

    _ctc_cli_qos_queue_id_parser(&que_cfg.value.pkt.queue, &argv[0], argc);

    index = CTC_CLI_GET_ARGC_INDEX("pkt-adjust-len");
    if (0xFF != index)
    {
        /* queue packet len adjust */
        CTC_CLI_GET_UINT8("pkt-adjust-len", adjust_len, argv[index+1]);
    }

    que_cfg.type = CTC_QOS_QUEUE_CFG_LENGTH_ADJUST;
    que_cfg.value.pkt.adjust_len       = adjust_len;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_show_shape,
        ctc_cli_qos_show_shape_cmd,
        "show qos shape ((port GPHYPORT_ID)|((queue | group)" CTC_CLI_QOS_QUEUE_ID_STR"))",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Queue shaping",
        "Group shaping",
        CTC_CLI_QOS_QUEUE_ID_DSCP)
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    uint16 service_id = 0;
    ctc_qos_shape_t qos_shape;
    uint8 index = 0;

    sal_memset(&qos_shape, 0, sizeof(qos_shape));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        /* port */
        CTC_CLI_GET_UINT32("gport", gport, argv[index+1]);
        qos_shape.type = CTC_QOS_SHAPE_PORT;
        qos_shape.shape.port_shape.gport = gport;
    }
    index = CTC_CLI_GET_ARGC_INDEX("queue");
    if (0xFF != index)
    {
        _ctc_cli_qos_queue_id_parser(&qos_shape.shape.queue_shape.queue, &argv[0], argc);
        qos_shape.type = CTC_QOS_SHAPE_QUEUE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        _ctc_cli_qos_queue_id_parser(&qos_shape.shape.group_shape.queue, &argv[0], argc);
        qos_shape.type = CTC_QOS_SHAPE_GROUP;
        index = CTC_CLI_GET_ARGC_INDEX("service-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
            qos_shape.shape.group_shape.group_type = CTC_QOS_SCHED_GROUP_SERVICE;
            qos_shape.shape.group_shape.service_id = service_id;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("gport", qos_shape.shape.group_shape.gport, argv[index + 1]);
        }
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_shape(g_api_lchip, &qos_shape);
    }
    else
    {
        ret = ctc_qos_get_shape(&qos_shape);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if(qos_shape.type == CTC_QOS_SHAPE_QUEUE)
    {
        ctc_cli_out("\nQueue shape:\n");
        ctc_cli_out("%-10s %-15s %-15s %-15s %-15s\n","shp-en", "cir(kbps/pps)", "cbs(kb/pkt)","pir(kbps/pps)", "pbs(kb/pkt)");
        ctc_cli_out("-----------------------------------------------------------------\n");
        ctc_cli_out("%-10d %-15u %-15u %-15u %-15u\n",qos_shape.shape.queue_shape.enable,qos_shape.shape.queue_shape.cir,
                    qos_shape.shape.queue_shape.cbs, qos_shape.shape.queue_shape.pir, qos_shape.shape.queue_shape.pbs);
    }
    else if(qos_shape.type == CTC_QOS_SHAPE_PORT)
    {
        ctc_cli_out("\nPort shape:\n");
        ctc_cli_out("%-10s %-15s %-15s \n","shp-en","pir(kbps/pps)", "pbs(kb/pkt)");
        ctc_cli_out("-----------------------------------------------------------------\n");
        ctc_cli_out("%-10d %-15u %-15u \n",qos_shape.shape.port_shape.enable, qos_shape.shape.port_shape.pir, qos_shape.shape.port_shape.pbs);
    }
    else if(qos_shape.type == CTC_QOS_SHAPE_GROUP)
    {
        ctc_cli_out("\nGroup shape:\n");
        ctc_cli_out("%-10s %-15s %-15s \n","shp-en","pir(kbps/pps)", "pbs(kb/pkt)");
        ctc_cli_out("-----------------------------------------------------------------\n");
        ctc_cli_out("%-10d %-15u %-15u \n",qos_shape.shape.group_shape.enable, qos_shape.shape.group_shape.pir, qos_shape.shape.group_shape.pbs);
    }

    return ret;
}
#define drop_cli ""

CTC_CLI(ctc_cli_qos_get_shape_en,
        ctc_cli_qos_get_shape_en_cmd,
        "show qos shape enable (port | queue | group)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "enable",
        "port shape",
        "queue shape",
        "group shape")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    ctc_cli_out(" Show qos shape enable\n");
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_PORT_SHAPE_EN;
        ctc_cli_out(" ==================================\n");
        ctc_cli_out(" port : ");
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUE_SHAPE_EN;
        ctc_cli_out(" ==================================\n");
        ctc_cli_out(" queue : ");
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUE_SHAPE_EN;
        ctc_cli_out(" ==================================\n");
        ctc_cli_out(" group : ");
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

    if(glb_cfg.u.value == 1)
    {
        ctc_cli_out("enable\n ");
    }
    else
    {
        ctc_cli_out("disable\n ");
    }

    ctc_cli_out("==================================\n");

    return ret;
}

CTC_CLI(ctc_cli_qos_set_port_queue_drop,
        ctc_cli_qos_set_port_queue_drop_cmd,
        "qos drop" CTC_CLI_QOS_QUEUE_ID_STR "\
    (wtd (threshold THRESH1 THRESH2 THRESH3| factor FACTOR1 FACTOR2 FACTOR3) | \
    wred min-threshold THRESH1 THRESH2 THRESH3 \
    max-threshold THRESH1 THRESH2 THRESH3 \
    drop-probability PROB1 PROB2 PROB3 \
    (weight WEIGHT|)) (level-profile LEVEL|) ((ecn-color-threshold ECN_THRD1 ECN_THRD2 ECN_THRD3|ecn-threshold ECN_THRD)|) (coexist-mode|)",
        CTC_CLI_QOS_STR,
        "Config queue drop",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Weighted tail drop",
        "Drop threshold",
        "Drop threshold for red-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Drop threshold for yellow-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Drop threshold for green-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Drop factor",
        "Drop factor for red-colored packets",
        "Drop factor for yellow-colored packets",
        "Drop factor for green-colored packets",
        "Weighted random early detection",
        "Minimum threshold",
        "Minimum threshold for red-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Minimum threshold for yellow-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Minimum threshold for green-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Maximum threshold",
        "Maximum threshold for red-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Maximum threshold for yellow-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Maximum threshold for green-colored packets, "CTC_CLI_QOS_DROP_THRESHOLD,
        "Drop probability",
        "Drop probability for red-colored packets, "CTC_CLI_QOS_DROP_PROB_VAL,
        "Drop probability for yellow-colored packets, "CTC_CLI_QOS_DROP_PROB_VAL,
        "Drop probability for green-colored packets, "CTC_CLI_QOS_DROP_PROB_VAL,
        "Weight [GB not support]",
        "WRED weight for computing average queue size, <0-15>",
        "Congestion level drop profile",
        "<0-7>",
        "ECN color mark threshold",
        "Ecn threshod for red-colored should less than max threshold",
        "Ecn threshod for yellow-colored should less than max threshold",
        "Ecn threshod for green-colored should less than max threshold",
        "ECN mark threshold",
        "Ecn threshod should less than max threshold",
        "WTD and WRED coexist")
{
    uint8  index = 0;
    uint8  level = 0;
    ctc_qos_drop_t drop_param;
    ctc_qos_queue_drop_t drop;
    int32 ret = CLI_SUCCESS;

    sal_memset(&drop_param, 0, sizeof(ctc_qos_drop_t));
    sal_memset(&drop, 0, sizeof(ctc_qos_queue_drop_t));

    _ctc_cli_qos_queue_id_parser(&drop_param.queue, &argv[0], argc);

    index = CTC_CLI_GET_ARGC_INDEX("wtd");
    if (0xFF != index)
    {
        drop.mode = CTC_QUEUE_DROP_WTD;
        index = CTC_CLI_GET_ARGC_INDEX("threshold");
        if (0xFF != index)
        {
            /* red threshold */
            CTC_CLI_GET_UINT16_RANGE("red threshold", drop.max_th[0], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

            /* yellow threshold */
            CTC_CLI_GET_UINT16_RANGE("yellow threshold", drop.max_th[1], argv[index + 2], 0, CTC_MAX_UINT16_VALUE);

            /* green threshold */
            CTC_CLI_GET_UINT16_RANGE("green threshold", drop.max_th[2], argv[index + 3], 0, CTC_MAX_UINT16_VALUE);

            /* critical threshold */
            drop.max_th[3] = 0x44;
        }
        index = CTC_CLI_GET_ARGC_INDEX("factor");
        if (0xFF != index)
        {
            drop.is_dynamic = 1;
            /* red factor */
            CTC_CLI_GET_UINT8_RANGE("red factor", drop.drop_factor[0], argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

            /* yellow factor */
            CTC_CLI_GET_UINT8_RANGE("yellow factor", drop.drop_factor[1], argv[index + 2], 0, CTC_MAX_UINT8_VALUE);

            /* green factor */
            CTC_CLI_GET_UINT8_RANGE("green factor", drop.drop_factor[2], argv[index + 3], 0, CTC_MAX_UINT8_VALUE);

            /* critical factor */
            drop.drop_factor[3] = 7;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("wred");
    if (0xFF != index)
    {
        drop.mode = CTC_QUEUE_DROP_WRED;

        /* red min-threshold */
        CTC_CLI_GET_UINT16_RANGE("red min-threshold", drop.min_th[0], argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        /* yellow min-threshold */
        CTC_CLI_GET_UINT16_RANGE("yellow min-threshold", drop.min_th[1], argv[index + 3], 0, CTC_MAX_UINT16_VALUE);
        /* green min-threshold */
        CTC_CLI_GET_UINT16_RANGE("green min-threshold", drop.min_th[2], argv[index + 4], 0, CTC_MAX_UINT16_VALUE);

        /* red max-threshold */
        CTC_CLI_GET_UINT16_RANGE("red max-threshold", drop.max_th[0], argv[index + 6], 0, CTC_MAX_UINT16_VALUE);
        /* yellow max-threshold */
        CTC_CLI_GET_UINT16_RANGE("yellow max-threshold", drop.max_th[1], argv[index + 7], 0, CTC_MAX_UINT16_VALUE);
        /* green max-threshold */
        CTC_CLI_GET_UINT16_RANGE("green max-threshold", drop.max_th[2], argv[index + 8], 0, CTC_MAX_UINT16_VALUE);

        /* red drop probability */
        CTC_CLI_GET_UINT16_RANGE("red drop probability", drop.drop_prob[0], argv[index + 10], 0, CTC_MAX_UINT16_VALUE);
        /* yellow drop probability */
        CTC_CLI_GET_UINT16_RANGE("yellow drop probability", drop.drop_prob[1], argv[index + 11], 0, CTC_MAX_UINT16_VALUE);
        /* green drop probability */
        CTC_CLI_GET_UINT16_RANGE("green drop probability", drop.drop_prob[2], argv[index + 12], 0, CTC_MAX_UINT16_VALUE);

        /* critical threshold */
        drop.max_th[3] = 0x44;
        drop.drop_prob[3] = 10;
    }


    index = CTC_CLI_GET_ARGC_INDEX("ecn-threshold");
    if (0xFF != index)
    {
        /*ecn mark threshold*/
        CTC_CLI_GET_UINT16("ecn threshold", drop.ecn_mark_th, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ecn-color-threshold");
    if (0xFF != index)
    {
        /*red ecn mark threshold*/
        CTC_CLI_GET_UINT16("red ecn threshold", drop.ecn_th[0], argv[index + 1]);
        /*yellow mark threshold*/
        CTC_CLI_GET_UINT16("yellow ecn threshold", drop.ecn_th[1], argv[index + 2]);
        /*green mark threshold*/
        CTC_CLI_GET_UINT16("green ecn threshold", drop.ecn_th[2], argv[index + 3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("coexist-mode");
    if (0xFF != index)
    {
        drop.is_coexist = 1;
    }

    sal_memcpy(&drop_param.drop, &drop, sizeof(ctc_qos_queue_drop_t));

    index = CTC_CLI_GET_ARGC_INDEX("level-profile");
    if (0xFF != index)
    {
        /* when use level profile, only set global varible */
        CTC_CLI_GET_UINT8_RANGE("level", level, argv[index + 1], 0, 7);
        sal_memcpy(&g_qos_drop_array[level], &drop_param, sizeof(drop_param));
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_qos_set_drop_scheme(g_api_lchip, &drop_param);
        }
        else
        {
            ret = ctc_qos_set_drop_scheme(&drop_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_get_port_queue_drop,
        ctc_cli_qos_get_port_queue_drop_cmd,
        "show qos drop" CTC_CLI_QOS_QUEUE_ID_STR "(level-profile|)",
        "Show",
        CTC_CLI_QOS_STR,
        "Config queue drop",
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Congestion level drop profile")
{
    uint8  index = 0;
    ctc_qos_drop_t drop_param;
    ctc_qos_drop_t tempdrop_param;
    int32 ret = CLI_SUCCESS;
    ctc_qos_resrc_t resrc;

    sal_memset(&drop_param, 0, sizeof(ctc_qos_drop_t));
    sal_memset(&tempdrop_param, 0, sizeof(ctc_qos_drop_t));
    sal_memset(&resrc, 0, sizeof(ctc_qos_resrc_t));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_QUEUE_DROP;

    _ctc_cli_qos_queue_id_parser(&drop_param.queue, &argv[0], argc);
    sal_memcpy(&tempdrop_param, &drop_param, sizeof(ctc_qos_drop_t));
    sal_memcpy(resrc.u.queue_drop, &drop_param, sizeof(ctc_qos_drop_t));
    drop_param.drop.mode = 2;
    ctc_cli_out("%-10s\n", "Queue drop information");
    ctc_cli_out("----------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("level-profile");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_qos_get_resrc(g_api_lchip, &resrc);
        }
        else
        {
            ret = ctc_qos_get_resrc(&resrc);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if(resrc.u.queue_drop[0].drop.mode == 1)
        {
            ctc_cli_out("%-30s: %s\n", "Drop Mode", "WRED");
            ctc_cli_out("%-30s: %u\n", "WTD and WRED coexist ", resrc.u.queue_drop[0].drop.is_coexist);
            for (index = 0; index < CTC_RESRC_MAX_CONGEST_LEVEL_NUM; index++)
            {
                ctc_cli_out("Congest:%d %-19s : %-8u%-8u%-8u%-8u\n",
                             index, "MaxThrd", resrc.u.queue_drop[index].drop.max_th[0],
                             resrc.u.queue_drop[index].drop.max_th[1],
                             resrc.u.queue_drop[index].drop.max_th[2],
                             resrc.u.queue_drop[index].drop.max_th[3]);
                ctc_cli_out("Congest:%d %-19s : %-8u%-8u%-8u%-8u\n",
                             index, "MinThrd", resrc.u.queue_drop[index].drop.min_th[0],
                             resrc.u.queue_drop[index].drop.min_th[1],
                             resrc.u.queue_drop[index].drop.min_th[2],
                             resrc.u.queue_drop[index].drop.min_th[3]);
                ctc_cli_out("Congest:%d %-19s : %-8u%-8u%-8u%-8u\n",
                             index, "DropProb", resrc.u.queue_drop[index].drop.drop_prob[0],
                             resrc.u.queue_drop[index].drop.drop_prob[1],
                             resrc.u.queue_drop[index].drop.drop_prob[2],
                             resrc.u.queue_drop[index].drop.drop_prob[3]);
            }
        }
        else
        {
            ctc_cli_out("%-30s: %s\n", "Drop Mode", "WTD");
            ctc_cli_out("%-30s: %u\n", "WTD and WRED coexist ", resrc.u.queue_drop[0].drop.is_coexist);
            for (index = 0; index < CTC_RESRC_MAX_CONGEST_LEVEL_NUM; index++)
            {
                ctc_cli_out("Congest:%d %-19s : %-8u%-8u%-8u%-8u\n",
                             index, "DropThrd", resrc.u.queue_drop[index].drop.max_th[0],
                             resrc.u.queue_drop[index].drop.max_th[1],
                             resrc.u.queue_drop[index].drop.max_th[2],
                             resrc.u.queue_drop[index].drop.max_th[3]);
                ctc_cli_out("Congest:%d %-19s : %-8u%-8u%-8u%-8u\n",
                             index, "EcnThrdColor", resrc.u.queue_drop[index].drop.ecn_th[0],
                             resrc.u.queue_drop[index].drop.ecn_th[1],
                             resrc.u.queue_drop[index].drop.ecn_th[2],
                             resrc.u.queue_drop[index].drop.ecn_th[3]);
                ctc_cli_out("Congest:%d %-19s : %-8u\n",
                             index, "EcnThrd", resrc.u.queue_drop[index].drop.ecn_mark_th);

            }
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_qos_get_drop_scheme(g_api_lchip, &drop_param);
        }
        else
        {
            ret = ctc_qos_get_drop_scheme(&drop_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if(drop_param.drop.mode == 1)
        {
            _ctc_cli_queue_drop_wred_show(drop_param);
        }
        else if(drop_param.drop.mode == 0)
        {
            _ctc_cli_queue_drop_wtd_show(drop_param);
        }
        else
        {
            _ctc_cli_queue_drop_wtd_show(drop_param);
            tempdrop_param.drop.mode = 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_qos_get_drop_scheme(g_api_lchip, &tempdrop_param);
            }
            else
            {
                ret = ctc_qos_get_drop_scheme(&tempdrop_param);
            }
            if (ret < 0 && ret != CTC_E_NOT_SUPPORT)
            {
                ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            if(ret == 0)
            {
                _ctc_cli_queue_drop_wred_show(tempdrop_param);
            }
        }
    }
    ctc_cli_out("----------------\n");
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_resrc_ingress,
        ctc_cli_qos_set_resrc_ingress_cmd,
        "qos resrc-mgr ingress (classify port GPHYPORT_ID priority PRIORITY pool POOL|port-min port GPHYPORT_ID threshold THRD)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Ingress",
        "Classify",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Pool",
        "Pool value <0-3>",
        "Port minimum guarantee",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Threshold",
        "Threshold value")
{
    uint8  index = 0;
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;

    sal_memset(&resrc, 0, sizeof(resrc));

    index = CTC_CLI_GET_ARGC_INDEX("classify");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.port_min.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("priority", resrc.u.pool.priority, argv[index + 4], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("pool", resrc.u.pool.pool, argv[index + 6], 0, CTC_MAX_UINT8_VALUE);
        resrc.u.pool.dir = CTC_INGRESS;
        resrc.cfg_type = CTC_QOS_RESRC_CFG_POOL_CLASSIFY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-min");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.port_min.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("threshold", resrc.u.port_min.threshold, argv[index + 4], 0, CTC_MAX_UINT16_VALUE);
        resrc.u.port_min.dir = CTC_INGRESS;
        resrc.cfg_type = CTC_QOS_RESRC_CFG_PORT_MIN;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_set_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_resrc_egress,
        ctc_cli_qos_set_resrc_egress_cmd,
        "qos resrc-mgr egress (classify port GPHYPORT_ID priority PRIORITY pool POOL|port-min port GPHYPORT_ID threshold THRD)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Egress",
        "Classify",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Pool",
        "Pool value <0-3>",
        "Port minimum guarantee",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Threshold",
        "Threshold value")
{
    uint8  index = 0;
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;

    sal_memset(&resrc, 0, sizeof(resrc));

    index = CTC_CLI_GET_ARGC_INDEX("classify");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.pool.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("priority", resrc.u.pool.priority, argv[index + 4], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("pool", resrc.u.pool.pool, argv[index + 6], 0, CTC_MAX_UINT8_VALUE);
        resrc.u.pool.dir = CTC_EGRESS;
        resrc.cfg_type = CTC_QOS_RESRC_CFG_POOL_CLASSIFY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-min");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.port_min.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("threshold", resrc.u.port_min.threshold, argv[index + 4], 0, CTC_MAX_UINT16_VALUE);
        resrc.u.port_min.dir = CTC_EGRESS;
        resrc.cfg_type = CTC_QOS_RESRC_CFG_PORT_MIN;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_set_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_qos_get_resrc,
        ctc_cli_qos_get_resrc_cmd,
        "show qos resrc-mgr (ingress|egress) (classify port GPHYPORT_ID priority PRIORITY pool|port-min port GPHYPORT_ID threshold)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Ingress",
        "Egress",
        "Classify",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Priority",
        CTC_CLI_PRIORITY_VALUE,
        "Pool",
        "Port minimum guarantee",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Threshold")
{
    uint8  index = 0;
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;

    sal_memset(&resrc, 0, sizeof(resrc));
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        resrc.u.pool.dir = CTC_INGRESS;
        resrc.u.port_min.dir = CTC_INGRESS;
    }
    else
    {
        resrc.u.pool.dir = CTC_EGRESS;
        resrc.u.port_min.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("classify");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.pool.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("priority", resrc.u.pool.priority, argv[index + 4], 0, CTC_MAX_UINT8_VALUE);
        resrc.cfg_type = CTC_QOS_RESRC_CFG_POOL_CLASSIFY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-min");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("port", resrc.u.port_min.gport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        resrc.cfg_type = CTC_QOS_RESRC_CFG_PORT_MIN;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_get_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if(resrc.cfg_type == CTC_QOS_RESRC_CFG_POOL_CLASSIFY)
    {
        ctc_cli_out("%-10s\n", "Pool information");
        ctc_cli_out("----------------\n");
        ctc_cli_out("%-19s : %-8u\n",
                        "Pool type", resrc.u.pool.pool);
    }
    else if(resrc.cfg_type == CTC_QOS_RESRC_CFG_PORT_MIN)
    {
        ctc_cli_out("%-10s\n", "Port-min information");
        ctc_cli_out("----------------\n");
        ctc_cli_out("%-19s : %-8u\n",
                        "Threshold", resrc.u.port_min.threshold);
    }
    ctc_cli_out("----------------\n");

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_qos_set_resrc_queue_drop,
        ctc_cli_qos_set_resrc_queue_drop_cmd,
        "qos resrc-mgr queue-drop level-profile",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Queue drop with congest level",
        "Use level profile")
{
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;

    sal_memset(&resrc, 0, sizeof(resrc));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_QUEUE_DROP;
    sal_memcpy(&resrc.u.queue_drop, &g_qos_drop_array, sizeof(g_qos_drop_array));

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_set_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_resrc_flow_ctl,
        ctc_cli_qos_set_resrc_flow_ctl_cmd,
        "qos resrc-mgr flow-ctl (port GPHYPORT_ID) (priority-class PRI|) (xon-thrd TRHD xoff-thrd THRD drop-thrd THRD)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Flow control",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Cos used for PFC",
        "<0-7>",
        "Xon thresthold",
        "Threshold",
        "Xoff thresthold",
        "Threshold",
        "Drop thresthold",
        "Threshold")
{
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;
    uint8 index =0;

    sal_memset(&resrc, 0, sizeof(resrc));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_FLOW_CTL;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("port", resrc.u.flow_ctl.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-class");
    if (0xFF != index)
    {
        resrc.u.flow_ctl.is_pfc = 1;
        CTC_CLI_GET_UINT16("priority class", resrc.u.flow_ctl.priority_class, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("xon-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("xon-thrd", resrc.u.flow_ctl.xon_thrd, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("xoff-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("xoff-thrd", resrc.u.flow_ctl.xoff_thrd, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("drop-thrd", resrc.u.flow_ctl.drop_thrd, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_set_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_get_resrc_flow_ctl,
        ctc_cli_qos_get_resrc_flow_ctl_cmd,
        "show qos resrc-mgr flow-ctl (port GPHYPORT_ID) (priority-class PRI|)",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Flow control",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Cos used for PFC",
        "<0-7>")
{
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;
    uint8 index =0;

    sal_memset(&resrc, 0, sizeof(resrc));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_FLOW_CTL;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("port", resrc.u.flow_ctl.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-class");
    if (0xFF != index)
    {
        resrc.u.flow_ctl.is_pfc = 1;
        CTC_CLI_GET_UINT16("priority class", resrc.u.flow_ctl.priority_class, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_get_resrc(&resrc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-10s\n", "Flow-control information");
    ctc_cli_out("----------------\n");
    ctc_cli_out("%-19s : %-8u\n","Xon threshold", resrc.u.flow_ctl.xon_thrd);
    ctc_cli_out("%-19s : %-8u\n","Xoff threshold", resrc.u.flow_ctl.xoff_thrd);
    ctc_cli_out("%-19s : %-8u\n","Drop threshold", resrc.u.flow_ctl.drop_thrd);
    ctc_cli_out("----------------\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_show_resrc_usage,
        ctc_cli_qos_show_resrc_usage_cmd,
        "show qos resrc-mgr (pool-count|queue-count" CTC_CLI_QOS_QUEUE_ID_STR ")",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Pool count",
        "Queue instant count",
        CTC_CLI_QOS_QUEUE_ID_DSCP)
{
    int32 ret = CLI_SUCCESS;
    uint8  index = 0;
    uint8  pool = 0;
    ctc_qos_resrc_pool_stats_t stats;

    sal_memset(&stats, 0, sizeof(stats));

    index = CTC_CLI_GET_ARGC_INDEX("pool-count");
    if (0xFF != index)
    {
        ctc_cli_out("%-10s\n", "Ingress pool");
        ctc_cli_out("----------------\n");
        for (pool = 0; pool < 4; pool ++)
        {
            stats.type = CTC_QOS_RESRC_STATS_IGS_POOL_COUNT;
            stats.pool = pool;

            if (g_ctcs_api_en)
            {
                ret = ctcs_qos_query_pool_stats(g_api_lchip, &stats);
            }
            else
            {
                ret = ctc_qos_query_pool_stats(&stats);
            }

            ctc_cli_out("%-5s%u: %5u\n", "Pool", pool, stats.count);
        }

        stats.type = CTC_QOS_RESRC_STATS_IGS_TOTAL_COUNT;
        if (g_ctcs_api_en)
        {
            ret = ctcs_qos_query_pool_stats(g_api_lchip, &stats);
        }
        else
        {
            ret = ctc_qos_query_pool_stats(&stats);
        }
        ctc_cli_out("%-5s : %5u\n", "Total", stats.count);
        ctc_cli_out("\n");
        ctc_cli_out("%-5s\n", "Egress pool");
        ctc_cli_out("----------------\n");
        for (pool = 0; pool < 4; pool ++)
        {
            stats.type = CTC_QOS_RESRC_STATS_EGS_POOL_COUNT;
            stats.pool = pool;
            if (g_ctcs_api_en)
            {
                ret = ctcs_qos_query_pool_stats(g_api_lchip, &stats);
            }
            else
            {
                ret = ctc_qos_query_pool_stats(&stats);
            }
            ctc_cli_out("%-5s%u: %5u\n", "Pool", pool, stats.count);
        }

        stats.type = CTC_QOS_RESRC_STATS_EGS_TOTAL_COUNT;
        if (g_ctcs_api_en)
        {
            ret = ctcs_qos_query_pool_stats(g_api_lchip, &stats);
        }
        else
        {
            ret = ctc_qos_query_pool_stats(&stats);
        }
        ctc_cli_out("%-5s : %5u\n", "Total", stats.count);
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-count");
    if (0xFF != index)
    {
        _ctc_cli_qos_queue_id_parser(&stats.queue, &argv[0], argc);

        stats.type = CTC_QOS_RESRC_STATS_QUEUE_COUNT;

        if(g_ctcs_api_en)
        {
             ret = ctcs_qos_query_pool_stats(g_api_lchip, &stats);
        }
        else
        {
            ret = ctc_qos_query_pool_stats(&stats);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("%-15s   :  %u\n", "queue-count", stats.count);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_resrc_fcdl,
        ctc_cli_qos_set_resrc_fcdl_cmd,
        "qos resrc-mgr fcdl detect (interval VALUE| (port GPHYPORT_ID (priority-class PRI PRI|) mult MULT (enable|disable)))",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Flow control dead-lock",
        "Detect",
        "Detect Interval",
        "Value",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Cos used for PFC",
        "<0-7>",
        "<0-7>",
        "Detect multiplier",
        "Value",
        "Enable",
        "Disable")
{
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;
    uint8 flag = 0;

    sal_memset(&resrc, 0, sizeof(resrc));
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_FCDL_DETECT;
    resrc.u.fcdl_detect.valid_num = 2;

    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_FCDL_INTERVAL;
        CTC_CLI_GET_UINT32("interval", glb_cfg.u.value, argv[index + 1]);
        flag = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("port", resrc.u.fcdl_detect.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-class");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("priority class", resrc.u.fcdl_detect.priority_class[0], argv[index + 1]);
        CTC_CLI_GET_UINT8("priority class", resrc.u.fcdl_detect.priority_class[1], argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mult");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mult", resrc.u.fcdl_detect.detect_mult, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        resrc.u.fcdl_detect.enable = 1;
    }
    if(flag == 0)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_qos_set_resrc(g_api_lchip, &resrc);
        }
        else
        {
            ret = ctc_qos_set_resrc(&resrc);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_qos_set_global_config(g_api_lchip, &glb_cfg);
        }
        else
        {
            ret = ctc_qos_set_global_config(&glb_cfg);
        }
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_show_resrc_fcdl,
        ctc_cli_qos_show_resrc_fcdl_cmd,
        "show qos resrc-mgr fcdl detect (port GPHYPORT_ID) ",
        "Show",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_RESRC_STR,
        "Flow control dead-lock",
        "Detect",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    ctc_qos_resrc_t resrc;
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;

    sal_memset(&resrc, 0, sizeof(resrc));
    resrc.cfg_type = CTC_QOS_RESRC_CFG_FCDL_DETECT;


    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("port", resrc.u.fcdl_detect.gport, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_get_resrc(g_api_lchip, &resrc);
    }
    else
    {
        ret = ctc_qos_get_resrc(&resrc);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-10s\n", "Flow-control deadlock detect information");
    ctc_cli_out("------------------------\n");
    ctc_cli_out("%-19s : %-8u\n","enable", resrc.u.fcdl_detect.enable);
    ctc_cli_out("%-19s : %-8u\n","valid num", resrc.u.fcdl_detect.valid_num);
    ctc_cli_out("%-19s : %-8u\n","Priority class0:", resrc.u.fcdl_detect.priority_class[0]);
    ctc_cli_out("%-19s : %-8u\n","Priority class1:", resrc.u.fcdl_detect.priority_class[1]);
    ctc_cli_out("------------------------\n");
    return CLI_SUCCESS;
}
#define stats_cli ""

CTC_CLI(ctc_cli_qos_show_port_queue_stats,
        ctc_cli_qos_show_port_queue_stats_cmd,
        "show qos stats ((service-id SERVICE port GPHYPORT_ID ) | (port GPHYPORT_ID (end-port GPHYPORT_ID|) | all-port) \
        | (reason-id REASON (end-reason-id REASON|) | all-reason)) (queue-id QUEUE_ID|) (lchip LCHIP| )",
        CTC_CLI_SHOW_STR,
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_STATS_STR,
        "Service id",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "End port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "All port",
        "Cpu reason",
        "Cpu reason id",
        "End cpu reason",
        "End cpu reason id",
        "All reason",
        CTC_CLI_QOS_QUEUE_STR,
        CTC_CLI_QOS_QUEUE_VAL,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = CLI_SUCCESS;
    ctc_qos_queue_stats_t stats_param;
    uint8 index = 0;
    uint16 start = 0;
    uint16 end = 0;
    uint16 qid = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint32 max_port_num_per_chip = 0;
    uint16 gport_start = 0;
    uint16 gport_end = 0;
    uint16 lport_start = 0;
    uint16 lport_end = 0;
    uint16 loop = 0;
    uint8  loop_flag = 0;
    uint8 gchip = 0;
    uint8 lchip = 0;
    uint16 reason_start = 0;
    uint16 reason_end = 0;
    uint8  cli_flag = 0;
    uint8  is_all = 0;

    sal_memset(&stats_param, 0, sizeof(ctc_qos_queue_stats_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    _ctc_cli_qos_queue_id_parser(&stats_param.queue, &argv[0], argc);


    index = CTC_CLI_GET_ARGC_INDEX("all-port");
    if (0xFF != index)
    {
        stats_param.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        lport_start = 0;
        lport_end = max_port_num_per_chip - 1;
        ctc_get_gchip_id(lchip, &gchip);
        is_all = 1;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("gport_start", gport_start, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            gport_end = gport_start;
        }
        index = CTC_CLI_GET_ARGC_INDEX("end-port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("gport_end", gport_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        if ((CTC_MAP_GPORT_TO_GCHIP(gport_start) != CTC_MAP_GPORT_TO_GCHIP(gport_end)) || (gport_start > gport_end))
        {
            ctc_cli_out("%% GPort range error !\n");
            return CLI_ERROR;
        }
        lport_start = CTC_MAP_GPORT_TO_LPORT(gport_start);
        lport_end = CTC_MAP_GPORT_TO_LPORT(gport_end);
        gchip = CTC_MAP_GPORT_TO_GCHIP(gport_start);
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-reason");
    if (0xFF != index)
    {
        stats_param.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        reason_start = 0;
        reason_end = CTC_PKT_CPU_REASON_MAX_COUNT - 1;
        is_all = 1;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("reason-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("reason id", reason_start, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            reason_end = reason_start;
        }
        index = CTC_CLI_GET_ARGC_INDEX("end-reason-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("end reason id", reason_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        if (reason_end < reason_start)
        {
            ctc_cli_out("%% Reason id range error !\n");
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF == index && ((stats_param.queue.queue_type == CTC_QUEUE_TYPE_NETWORK_EGRESS)
        || (stats_param.queue.queue_type == CTC_QUEUE_TYPE_SERVICE_INGRESS)))
    {
        start = 0;
        end = 8;
    }
    else
    {
        start = stats_param.queue.queue_id;
        end = start + 1;
    }

    loop_flag = (stats_param.queue.queue_type == CTC_QUEUE_TYPE_EXCP_CPU);

    ctc_cli_out("%-8s %-10s %-15s %-15s %-15s %-15s\n", loop_flag ? "Reason" : "Gport","Queue id", "Deq(Pkts)", "Deq(Bytes)", "Drop(Pkts)", "Drop(Bytes)");
    ctc_cli_out("--------------------------------------------------------------------------------\n");


    for(loop = (loop_flag ? reason_start : lport_start); loop <= (loop_flag ? reason_end : lport_end); loop++)
    {
        cli_flag = 0;
        if (loop_flag)
        {
            stats_param.queue.cpu_reason = loop;
        }
        else
        {
            stats_param.queue.gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        }
        for (qid = start ; qid < end ; qid++)
        {
            stats_param.queue.queue_id = qid;
            if (g_ctcs_api_en)
            {
                ret = ctcs_qos_query_queue_stats(g_api_lchip, &stats_param);
            }
            else
            {
                ret = ctc_qos_query_queue_stats(&stats_param);
            }
            if (ret < 0)
            {
                continue;
            }

            if (is_all
                && (stats_param.stats.deq_packets == 0)
                && (stats_param.stats.drop_packets == 0))
            {
                continue;
            }
            if (loop_flag)
            {
                ctc_cli_out("%-8d ", stats_param.queue.cpu_reason);
            }
            else if (qid == start)
            {
                ctc_cli_out("0x%-6x ", stats_param.queue.gport);
            }
            else
            {
                ctc_cli_out("%-8s ", " ");
            }

            ctc_cli_out("%-10d %-15"PRIu64" %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n",
                        stats_param.queue.queue_id,
                        stats_param.stats.deq_packets, stats_param.stats.deq_bytes,
                        stats_param.stats.drop_packets, stats_param.stats.drop_bytes);
            cli_flag = 1;

        }
        if (cli_flag)
        {
            ctc_cli_out("--------------------------------------------------------------------------------\n");
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_clear_port_queue_stats,
        ctc_cli_qos_clear_port_queue_stats_cmd,
        "clear qos stats ((service-id SERVICE port GPHYPORT_ID ) | (port GPHYPORT_ID (end-port GPHYPORT_ID|) | all-port) \
        | (reason-id REASON (end-reason-id REASON|) | all-reason)) (queue-id QUEUE_ID|) (lchip LCHIP| )",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_STATS_STR,
        "Service id",
        CTC_CLI_QOS_SERVICE_VAL,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "End port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "All port",
        "Cpu reason",
        "Cpu reason id",
        "End cpu reason",
        "End cpu reason id",
        "All reason",
        CTC_CLI_QOS_QUEUE_STR,
        CTC_CLI_QOS_QUEUE_VAL
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = CLI_SUCCESS;
    ctc_qos_queue_stats_t stats_param;
    uint8 index = 0;
    uint16 start = 0;
    uint16 end = 0;
    uint16 qid = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint32 max_port_num_per_chip = 0;
    uint16 gport_start = 0;
    uint16 gport_end = 0;
    uint16 lport_start = 0;
    uint16 lport_end = 0;
    uint16 loop = 0;
    uint8  loop_flag = 0;
    uint8 gchip = 0;
    uint8 lchip = 0;
    uint16 reason_start = 0;
    uint16 reason_end = 0;

    sal_memset(&stats_param, 0, sizeof(ctc_qos_queue_stats_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    _ctc_cli_qos_queue_id_parser(&stats_param.queue, &argv[0], argc);

    index = CTC_CLI_GET_ARGC_INDEX("all-port");
    if (0xFF != index)
    {
        stats_param.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        lport_start = 0;
        lport_end = max_port_num_per_chip - 1;
        ctc_get_gchip_id(lchip, &gchip);
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("gport_start", gport_start, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            gport_end = gport_start;
        }
        index = CTC_CLI_GET_ARGC_INDEX("end-port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("gport_end", gport_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        if ((CTC_MAP_GPORT_TO_GCHIP(gport_start) != CTC_MAP_GPORT_TO_GCHIP(gport_end)) || (gport_start > gport_end))
        {
            ctc_cli_out("%% GPort range error !\n");
            return CLI_ERROR;
        }
        lport_start = CTC_MAP_GPORT_TO_LPORT(gport_start);
        lport_end = CTC_MAP_GPORT_TO_LPORT(gport_end);
        gchip = CTC_MAP_GPORT_TO_GCHIP(gport_start);
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-reason");
    if (0xFF != index)
    {
        stats_param.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        reason_start = 0;
        reason_end = CTC_PKT_CPU_REASON_MAX_COUNT - 1;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("reason-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("reason id", reason_start, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            reason_end = reason_start;
        }
        index = CTC_CLI_GET_ARGC_INDEX("end-reason-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("end reason id", reason_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        if (reason_end < reason_start)
        {
            ctc_cli_out("%% Reason id range error !\n");
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue-id");
    if (0xFF == index && ((stats_param.queue.queue_type == CTC_QUEUE_TYPE_NETWORK_EGRESS)
        || (stats_param.queue.queue_type == CTC_QUEUE_TYPE_SERVICE_INGRESS)))
    {
        start = 0;
        end = 8;
    }
    else
    {
        start = stats_param.queue.queue_id;
        end = start + 1;
    }
    loop_flag = (stats_param.queue.queue_type == CTC_QUEUE_TYPE_EXCP_CPU);
    for(loop = (loop_flag ? reason_start : lport_start); loop <= (loop_flag ? reason_end : lport_end); loop++)
    {
        if (loop_flag)
        {
            stats_param.queue.cpu_reason = loop;
        }
        else
        {
            stats_param.queue.gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        }

        for (qid = start ; qid < end ; qid++)
        {
            stats_param.queue.queue_id = qid;
            if (g_ctcs_api_en)
            {
                ret = ctcs_qos_clear_queue_stats(g_api_lchip, &stats_param);
            }
            else
            {
                ret = ctc_qos_clear_queue_stats(&stats_param);
            }
            if (ret < 0)
            {
                continue;
            }
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_qos_set_policer_stats_enable,
        ctc_cli_qos_set_policer_stats_enable_cmd,
        "qos policer stats (enable | disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_PLC_STR,
        CTC_CLI_QOS_STATS_STR,
        "Globally enable QoS policer statistics",
        "Globally disable QoS policer statistics")
{
    bool  enable = FALSE;
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    if (CLI_CLI_STR_EQUAL("e", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_STATS_EN;
    glb_cfg.u.value = enable ? 1 : 0;

    if (g_ctcs_api_en)
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

CTC_CLI(ctc_cli_qos_set_port_queue_stats_en,
        ctc_cli_qos_set_port_queue_stats_en_cmd,
        "qos stats" CTC_CLI_QOS_QUEUE_ID_STR "(enable|disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_STATS_STR,
        CTC_CLI_QOS_QUEUE_ID_DSCP,
        "Enable",
        "Disable")
{
    int32  ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));

    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;

    _ctc_cli_qos_queue_id_parser(&que_cfg.value.stats.queue, &argv[0], argc);


    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        que_cfg.value.stats.stats_en = 1;
    }
    else
    {
        que_cfg.value.stats.stats_en = 0;
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


#define cpu_traffic ""

CTC_CLI(ctc_cli_qos_set_cpu_reason_map,
        ctc_cli_qos_set_cpu_reason_map_cmd,
        "qos (cpu-reason REASON) map-to (queue-id QUEUE reason-group GROUP|cos COS)",
        CTC_CLI_QOS_STR,
        "Cpu Reason Id",
        "<0-MAX> refer to cli show qos cpu-reason>",
        "Mapping to",
        "Queue id",
        "<0-7>",
        "Reason group",
        CTC_CLI_QOS_MAX_CPU_REASON_GRP_ID,
        "Cos",
        "<0-7>")
{
    int32 ret = CLI_SUCCESS;
    uint16 reason_id = 0;
    uint16 queue_id = 0;
    uint8 group_id = 0;
    uint16 index = 0;
    uint8 cos = 0;

    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));

    /* Get Cpu Reason Id */
    index = CTC_CLI_GET_ARGC_INDEX("cpu-reason");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("cpu-reason", reason_id, argv[index + 1]);
    }


    /* Get Queue Id */
    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("cos", cos, argv[index + 1]);
        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_PRIORITY;
        que_cfg.value.reason_pri.cpu_reason = reason_id;
        que_cfg.value.reason_pri.cos = cos;
    }
    else
    {

        /* Get Queue Id */
        index = CTC_CLI_GET_ARGC_INDEX("queue-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("queue-id", queue_id, argv[index + 1]);
        }

        /* Get Group Id */
        index = CTC_CLI_GET_ARGC_INDEX("reason-group");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("reason-group", group_id, argv[index + 1]);
        }


        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
        que_cfg.value.reason_map.cpu_reason = reason_id;
        que_cfg.value.reason_map.queue_id = queue_id;
        que_cfg.value.reason_map.reason_group = group_id;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_set_cpu_reason_dest,
        ctc_cli_qos_set_cpu_reason_dest_cmd,
        "qos cpu-reason REASON dest-to (nexthop NHID | eth-cpu | local-cpu | remote-cpu-port GPORT | local-port GPORT | drop | discard-cancel-to-cpu (enable|disable))",
        CTC_CLI_QOS_STR,
        "Cpu Reason Id",
        "<0-MAX> refer to cli [show qos cpu-reason]",
        "Destination",
        "Nexthop",
        CTC_CLI_NH_ID_STR,
        "Local CPU by network port",
        "Local CPU",
        "Remote CPU",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Local port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Drop",
        "Cancel switch logic discard's packet exception to CPU",
        CTC_CLI_ENABLE,CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint16 reason_id = 0;
    uint8 index = 0;
    uint8 dest_type = 0;
    uint32 dest_port = 0;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));

    /* Get Cpu Reason Id */
    CTC_CLI_GET_UINT16("cpu-reason", reason_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("local-cpu");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
       dest_type = CTC_PKT_CPU_REASON_TO_NHID;
       CTC_CLI_GET_UINT32("nexthop", que_cfg.value.reason_dest.nhid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-cpu");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-cpu-port");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_TO_REMOTE_CPU;
        CTC_CLI_GET_UINT32("remote-cpu port", dest_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-port");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_PORT;
        CTC_CLI_GET_UINT32("gport", dest_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_TO_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-cancel-to-cpu");
    if (0xFF != index)
    {
        dest_type = CTC_PKT_CPU_REASON_DISCARD_CANCEL_TO_CPU;
        if (CLI_CLI_STR_EQUAL("enable", index+1))
        {
            dest_port = 1;
        }
        else
        {
            dest_port = 0;
        }
    }

    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    que_cfg.value.reason_dest.cpu_reason = reason_id;
    que_cfg.value.reason_dest.dest_port = dest_port;
    que_cfg.value.reason_dest.dest_type = dest_type;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_qos_set_cpu_reason_misc_param,
        ctc_cli_qos_set_cpu_reason_misc_param_cmd,
        "qos cpu-reason REASON misc-param truncation (enable|disable)",
        CTC_CLI_QOS_STR,
        "Cpu Reason Id",
        "<0-MAX> refer to cli [show qos cpu-reason]",
        "cpu reason other misc paramer",
        "Packet will be truncated",
        CTC_CLI_ENABLE,CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint16 reason_id = 0;
    uint8 index = 0;
	uint32 value = 0;

    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));

    /* Get Cpu Reason Id */
    CTC_CLI_GET_UINT16("cpu-reason", reason_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
      value = 1;
    }

    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MISC;
    que_cfg.value.reason_misc.cpu_reason = reason_id;
    que_cfg.value.reason_misc.truncation_en= value;

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_qos_set_reason_truncation_len,
        ctc_cli_qos_set_reason_truncation_len_cmd,
        "qos cpu-reason truncation-length LENGTH",
        CTC_CLI_QOS_STR,
        "Cpu Resaon ",
        "The length of truncation",
        "The length of truncation ")
{
    int32 ret = CLI_SUCCESS;
	int32 value = 0;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    CTC_CLI_GET_UINT32("truncation-length", value, argv[0]);
    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TRUNCATION_LEN;
    glb_cfg.u.value  = value;
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
CTC_CLI(ctc_cli_qos_set_reason_shp_base_pkt,
        ctc_cli_qos_set_reason_shp_base_pkt_cmd,
        "qos shape (reason-shape-pkt | port-shape-pkt) (enable | disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "CPU Shape base on packet",
        "Network port shape base on packet",
        "Globally enable ",
        "Globally disable")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;
    uint8 index = 0;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("reason-shape-pkt");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_REASON_SHAPE_PKT_EN;
    }
    else
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SHAPE_PKT_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        glb_cfg.u.value = 1;
    }
    else
    {
        glb_cfg.u.value = 0;
    }

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


CTC_CLI(ctc_cli_qos_set_queue_shape_base,
        ctc_cli_qos_set_queue_shape_base_cmd,
        "qos shape (port | group |queue) (enable | disable)",
        CTC_CLI_QOS_STR,
        CTC_CLI_QOS_SHAPE_STR,
        "port shape",
        "queue shape",
        "group shape",
        "Globally enable ",
        "Globally disable")
{
    int32 ret = CLI_SUCCESS;
    ctc_qos_glb_cfg_t glb_cfg;
    uint8 index = 0;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_PORT_SHAPE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_GROUP_SHAPE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue");
    if (0xFF != index)
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUE_SHAPE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        glb_cfg.u.value = 1;
    }
    else
    {
        glb_cfg.u.value = 0;
    }

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

#define service_cli ""
CTC_CLI(ctc_cli_qos_add_del_service,
        ctc_cli_qos_add_del_service_cmd,
        "qos service (create | destroy) (service-id SERVICE) ",
        CTC_CLI_QOS_STR,
        "Service",
        "Binding queue",
        "Unbinding queue",
        "Serive id",
        CTC_CLI_QOS_SERVICE_VAL)
{
    int32 ret = CLI_SUCCESS;
    uint16 service_id = 0;
    uint8 index = 0;
    ctc_qos_queue_cfg_t que_cfg;
    ctc_qos_service_info_t srv_queue_info;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    sal_memset(&srv_queue_info, 0, sizeof(ctc_qos_service_info_t));


    index = CTC_CLI_GET_ARGC_INDEX("create");
    if (0xFF != index)
    {
       srv_queue_info.opcode = CTC_QOS_SERVICE_ID_CREATE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("destroy");
    if (0xFF != index)
    {
       srv_queue_info.opcode = CTC_QOS_SERVICE_ID_DESTROY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
    }

    srv_queue_info.service_id = service_id;

    sal_memcpy(&que_cfg.value.srv_queue_info, &srv_queue_info, sizeof(srv_queue_info));

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_qos_bind_service,
        ctc_cli_qos_bind_service_cmd,
        "qos service  (bind | unbind) (service-id SERVICE) (dest-port GPORT)(egress|)",
        CTC_CLI_QOS_STR,
        "Service",
        "Binding queue",
        "Unbinding queue",
        "Serive id",
        CTC_CLI_QOS_SERVICE_VAL,
        "dest port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Egress direction")
{
    int32 ret = CLI_SUCCESS;
    uint16 service_id = 0;
    uint16 dest_port = 0;
    uint8 index = 0;
    ctc_qos_queue_cfg_t que_cfg;
    ctc_qos_service_info_t srv_queue_info;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    sal_memset(&srv_queue_info, 0, sizeof(ctc_qos_service_info_t));


    index = CTC_CLI_GET_ARGC_INDEX("bind");
    if (0xFF != index)
    {
       srv_queue_info.opcode = CTC_QOS_SERVICE_ID_BIND_DESTPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unbind");
    if (0xFF != index)
    {
       srv_queue_info.opcode = CTC_QOS_SERVICE_ID_UNBIND_DESTPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("service-id", service_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dest-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", dest_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        srv_queue_info.dir = CTC_EGRESS;
    }

    srv_queue_info.service_id = service_id;
    srv_queue_info.gport = dest_port;

    sal_memcpy(&que_cfg.value.srv_queue_info, &srv_queue_info, sizeof(srv_queue_info));

    if(g_ctcs_api_en)
    {
        ret = ctcs_qos_set_queue(g_api_lchip, &que_cfg);
    }
    else
    {
        ret = ctc_qos_set_queue(&que_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
#define monitor_drop_cli ""
CTC_CLI(ctc_cli_qos_monitor_drop,
        ctc_cli_qos_monitor_drop_cmd,
        "qos monitor-drop (src-port GPHYPORT_ID )(dst-port GPHYPORT_ID )(enable|disable)",
        CTC_CLI_QOS_STR,
        "Queue monitor drop",
        "Source global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Destination global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;
    uint32 port = 0;
    uint8 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUEUE_DROP_MONITOR_EN;
    index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != index)
    {
       CTC_CLI_GET_UINT32("gport", port, argv[index + 1]);
       glb_cfg.u.drop_monitor.src_gport = port;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-port");
    if (0xFF != index)
    {
       CTC_CLI_GET_UINT32("gport", port, argv[index + 1]);
       glb_cfg.u.drop_monitor.dst_gport = port;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        glb_cfg.u.drop_monitor.enable = 1;
    }

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

CTC_CLI(ctc_cli_qos_show_monitor_drop,
        ctc_cli_qos_show_monitor_drop_cmd,
        "show qos monitor-drop (src-port GPHYPORT_ID )",
        "Show",
        CTC_CLI_QOS_STR,
        "Queue monitor drop",
        "Source global port",
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 port = 0;
    uint8 index = 0;
    ctc_qos_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUEUE_DROP_MONITOR_EN;
    index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != index)
    {
       CTC_CLI_GET_UINT32("gport", port, argv[index + 1]);
       glb_cfg.u.drop_monitor.src_gport = port;
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
    ctc_cli_out("\nMonitor Drop Information:\n");
    ctc_cli_out("%-10s %-10s\n","enable", "dst-port");
    ctc_cli_out("-----------------------------------------\n");
    ctc_cli_out("%-10d 0x%-10x\n",glb_cfg.u.drop_monitor.enable,glb_cfg.u.drop_monitor.dst_gport);
    return ret;
}
int32
ctc_qos_cli_init(void)
{
    /*policer cli*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_create_policer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_remove_policer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_policer_first_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_policer_configure_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_policer_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_clear_policer_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_policer_stats_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_policer_cmd);

    /*domain mapping*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_igs_domain_map_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_egs_domain_map_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_phb_configure_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_get_phb_configure_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_qos_domain_cmd);

    /* queue */
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_pri_map_cmd);

    /*shaping*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_shape_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_queue_shape_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_group_shape_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_shape_ipg_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_reason_shape_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_queue_stats_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_port_queue_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_clear_port_queue_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_shape_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_shape_base_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_get_shape_en_cmd);

    /*schedule*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_wdrr_weight_mtu_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_wdrr_weight_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_wdrr_cpu_reason_weight_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_queue_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_group_class_priority_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_group_wdrr_weight_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_schedule_wrr_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_sub_group_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_port_sub_group_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_sched_cmd);

    /*service*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_add_del_service_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_bind_service_cmd);

    /*drop*/
    ctc_cli_queue_drop_thrd_init();
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_port_queue_drop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_get_port_queue_drop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_ingress_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_egress_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_get_resrc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_queue_drop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_resrc_usage_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_mgr_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_flow_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_get_resrc_flow_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_queue_pkt_len_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_resrc_fcdl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_resrc_fcdl_cmd);

    /*cpu reason*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_cpu_reason_map_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_cpu_reason_dest_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_reason_shp_base_pkt_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_reason_truncation_len_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_set_cpu_reason_misc_param_cmd);

    /*debug*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_debug_off_cmd);

    /*queueu monitor drop*/
    install_element(CTC_SDK_MODE, &ctc_cli_qos_monitor_drop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_qos_show_monitor_drop_cmd);

    return CLI_SUCCESS;
}


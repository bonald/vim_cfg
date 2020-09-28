/**
 @file ctc_monitor_cli.c

 @date 2010-4-1

 @version v2.0
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_monitor_cli.h"

CTC_CLI(ctc_cli_monitor_set_buffer_monitor,
        ctc_cli_monitor_set_monitor_buffer_cmd,
        "monitor buffer (((port GPORT_ID) (inform|log))|((port GPORT_ID(queue|)|sc|total) direction (ingress|egress) scan)) (enable|disable)",
        CTC_CLI_MONITOR_M_STR,
        "Buffer Monitor",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Inform and generate event",
        "Log to cpu",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Queue",
        "Sc",
        "Total",
        "Flow direction",
        "Ingress",
        "Egress",
        "Scan and generate stats",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 gport = 0;
    ctc_monitor_config_t config;
    sal_memset(&config, 0, sizeof(config));

    config.monitor_type = CTC_MONITOR_BUFFER;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        config.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("inform");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_EN;
    }


    index = CTC_CLI_GET_ARGC_INDEX("scan");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_MON_SCAN_EN;
    }


    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_LOG_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        config.value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("queue");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_QUEUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sc");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_SC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("total");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        config.dir = CTC_EGRESS;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_monitor_set_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_config(&config);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_monitor_set_latency_monitor,
        ctc_cli_monitor_set_monitor_latency_cmd,
        "monitor latency ((port GPORT_ID) (inform|log|discard|scan))(enable|disable)",
        CTC_CLI_MONITOR_M_STR,
        "Latency Monitor",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Inform and generate event",
        "Log to cpu",
        "Discard",
        "Scan and generate stats",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 gport = 0;
    ctc_monitor_config_t config;
    sal_memset(&config, 0, sizeof(config));

    config.monitor_type = CTC_MONITOR_LATENCY;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        config.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("inform");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_EN;
    }


    index = CTC_CLI_GET_ARGC_INDEX("scan");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_MON_SCAN_EN;
    }


    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_LOG_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_CONFIG_LANTENCY_DISCARD_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        config.value = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_monitor_set_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_config(&config);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_monitor_set_timer,
        ctc_cli_monitor_set_timer_cmd,
        "monitor (buffer|latency) (scan-interval TIMER)",
        CTC_CLI_MONITOR_M_STR,
        "Buffer Monitor",
        "Latency Monitor",
         "Scan interval",
         "Value <ms>")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 interval = 0;
    ctc_monitor_config_t config;
    sal_memset(&config, 0, sizeof(config));

    index = CTC_CLI_GET_ARGC_INDEX("buffer");
    if (0xFF != index)
    {
        config.monitor_type = CTC_MONITOR_BUFFER;
    }
    else
    {
        config.monitor_type = CTC_MONITOR_LATENCY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scan-interval");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("scan interval", interval, argv[index + 1]);
        config.value = interval;
    }

    config.cfg_type = CTC_MONITOR_CONFIG_MON_INTERVAL;

    if(g_ctcs_api_en)
    {
        ret = ctcs_monitor_set_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_config(&config);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return ret;
}

CTC_CLI(ctc_cli_monitor_op_watermark,
        ctc_cli_monitor_op_watermark_cmd,
        "(show|clear) monitor ((buffer direction (ingress|egress))(port GPORT_ID | sc SC_ID | total)|latency port GPORT_ID) watermark",
        "Show",
        "Clear",
        CTC_CLI_MONITOR_M_STR,
        "Buffer Monitor",
        "Flow direction",
        "Ingress",
        "Egress",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "SC",
        "SC ID",
        "Total packet",
        "Latency Monitor",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Watermark")
{
    int32 ret = 0;
    ctc_monitor_watermark_t watermark;
    uint8 index = 0;
    uint16 gport = 0;
    bool  clear_op = 0;
    sal_memset(&watermark, 0, sizeof(watermark));

    index = CTC_CLI_GET_ARGC_INDEX("clear");
    if (0xFF != index)
    {
         clear_op = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("buffer");
    if (0xFF != index)
    {
        watermark.monitor_type = CTC_MONITOR_BUFFER;
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
            watermark.gport = gport;
            watermark.u.buffer.buffer_type = CTC_MONITOR_BUFFER_PORT;
        }
    }
    else
    {
        watermark.monitor_type = CTC_MONITOR_LATENCY;
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
            watermark.gport = gport;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        watermark.u.buffer.dir = CTC_INGRESS;
    }
    else
    {
        watermark.u.buffer.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sc");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("sc", watermark.u.buffer.sc, argv[index + 1]);
        watermark.u.buffer.buffer_type = CTC_MONITOR_BUFFER_SC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("total");
    if (0xFF != index)
    {
        watermark.u.buffer.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
    }
    if (clear_op == 0)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_watermark(g_api_lchip, &watermark);
        }
        else
        {
            ret = ctc_monitor_get_watermark(&watermark);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (watermark.monitor_type == CTC_MONITOR_LATENCY)
        {
            ctc_cli_out("Max Latency:%"PRIu64"ns\n", watermark.u.latency.max_latency);
        }
        else if(watermark.u.buffer.dir == CTC_EGRESS)
        {
            if(watermark.u.buffer.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                ctc_cli_out("Max UcCnt:%d, McCnt:%d, TotoalCnt:%d\n",
                            watermark.u.buffer.max_uc_cnt,
                            watermark.u.buffer.max_mc_cnt,
                            watermark.u.buffer.max_total_cnt);
            }
            else
            {
                ctc_cli_out("Max TotoalCnt:%d\n",watermark.u.buffer.max_total_cnt);
            }
        }
        else if(watermark.u.buffer.dir == CTC_INGRESS)
        {
            ctc_cli_out("Max TotoalCnt:%d\n", watermark.u.buffer.max_total_cnt);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_clear_watermark(g_api_lchip, &watermark);
        }
        else
        {
            ret = ctc_monitor_clear_watermark(&watermark);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

return ret;
}

CTC_CLI(ctc_cli_monitor_set_inform_thrd,
        ctc_cli_monitor_set_inform_thrd_cmd,
        "monitor (buffer (port GPORT_ID | sc SC_ID | total | c2c )|latency port GPORT_ID) (inform-thrd MIN MAX) ",
        CTC_CLI_MONITOR_M_STR,
        "Buffer Monitor",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "SC",
        "SC ID",
        "Total packet",
        "c2c packet",
        "Latency Monitor",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Inform when greater or less than threshold",
        "Min value , for buffer is buffer cell, for GG latency is thrd level<0-7>,for D2 latency is threshold time <ns>",
        "Max value , for buffer is buffer cell, for GG latency is thrd level<0-7>,for D2 latency is threshold time <ns>")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 gport = 0;
    uint32 min_value = 0;
    uint32 max_value = 0;
    ctc_monitor_config_t config;
    sal_memset(&config, 0, sizeof(config));


    index = CTC_CLI_GET_ARGC_INDEX("buffer");
    if (0xFF != index)
    {
        config.monitor_type = CTC_MONITOR_BUFFER;
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != index)
        {
            config.buffer_type = CTC_MONITOR_BUFFER_PORT;
        }

        index = CTC_CLI_GET_ARGC_INDEX("sc");
        if (0xFF != index)
        {
            config.buffer_type = CTC_MONITOR_BUFFER_SC;
            CTC_CLI_GET_UINT32("sc id", config.sc, argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX("total");
        if (0xFF != index)
        {
            config.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
        }
        index = CTC_CLI_GET_ARGC_INDEX("c2c");
        if (0xFF != index)
        {
            config.buffer_type = CTC_MONITOR_BUFFER_C2C;
        }
    }
    else
    {
        config.monitor_type = CTC_MONITOR_LATENCY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
        config.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX("inform-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("min", min_value, argv[index + 1]);
        CTC_CLI_GET_UINT32("max", max_value, argv[index + 2]);
    }

    config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MIN;
    config.value = min_value;
    if (g_ctcs_api_en)
    {
         ret = ctcs_monitor_set_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_config(&config);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MAX;
    config.value = max_value;
    if (g_ctcs_api_en)
    {
         ret = ctcs_monitor_set_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_config(&config);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_monitor_set_latency_level_thrd,
        ctc_cli_monitor_set_latency_level_thrd_cmd,
        "monitor (latency | mburst) level-thrd {level0 THRD0|level1 THRD1|level2 THRD2|level3 THRD3|level4 THRD4|level5 THRD5|level6 THRD6|level7 THRD7|}",
        CTC_CLI_MONITOR_M_STR,
        "Latency Monitor",
        "Micro burst",
        "Threshold zone",
        "Level 0 threshold",
        "unit:ns",
        "Level 1 threshold",
        "unit:ns",
        "Level 2 threshold",
        "unit:ns",
        "Level 3 threshold",
        "unit:ns",
        "Level 4 threshold",
        "unit:ns",
        "Level 5 threshold",
        "unit:ns",
        "Level 6 threshold",
        "unit:ns",
        "Level 7 threshold",
        "unit:ns"
        )
{

    int32 ret = 0;
    uint8 level = 0;
    uint32 value = 0;
    uint8 index = 0;
    uint8 strlevel = 0;
    char  *str[8] = {"level0","level1","level2","level3","level4","level5","level6","level7"};
    ctc_monitor_glb_cfg_t config;
    sal_memset(&config, 0, sizeof(config));

    config.cfg_type = CTC_MONITOR_GLB_CONFIG_MBURST_THRD;
    index = CTC_CLI_GET_ARGC_INDEX("latency");
    if (0xFF != index)
    {
        config.cfg_type = CTC_MONITOR_GLB_CONFIG_LATENCY_THRD;
    }

    for (level = 0; level < 8; level++)
    {
        strlevel = CTC_CLI_GET_ARGC_INDEX(str[level]);
        if(0xFF != strlevel)
        {
            CTC_CLI_GET_UINT32("thrd", value, argv[strlevel+1]);
            if(config.cfg_type == CTC_MONITOR_GLB_CONFIG_LATENCY_THRD)
            {
                config.u.thrd.level = level;
                config.u.thrd.threshold = value;
            }
            else
            {
                config.u.mburst_thrd.level = level;
                config.u.mburst_thrd.threshold = value;
            }

            if (g_ctcs_api_en)
            {
                ret = ctcs_monitor_set_global_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_set_global_config(&config);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            }
        }
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_monitor_set_buffer_log_mode,
        ctc_cli_monitor_set_buffer_log_mode_cmd,
        "monitor buffer log-mode (causer|victim|all)",
        CTC_CLI_MONITOR_M_STR,
        "Buffer Monitor",
        "Buffer log mode",
        "Log packets which genrate buffer congestion",
        "Log congestion packets",
        "all")
{

    int32 ret = 0;
    uint8 index = 0;
    ctc_monitor_glb_cfg_t config;
    sal_memset(&config, 0, sizeof(config));

    config.cfg_type = CTC_MONITOR_GLB_CONFIG_BUFFER_LOG_MODE;

    index = CTC_CLI_GET_ARGC_INDEX("causer");
    if (0xFF != index)
    {
        config.u.value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("victim");
    if (0xFF != index)
    {
        config.u.value = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF != index)
    {
        config.u.value = 2;
    }

    if (g_ctcs_api_en)
    {
         ret = ctcs_monitor_set_global_config(g_api_lchip, &config);
    }
    else
    {
        ret = ctc_monitor_set_global_config(&config);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_monitor_show_port,
        ctc_cli_monitor_show_port_cmd,
        "show monitor (port GPORT_ID | sc SC_ID | total | c2c) direction (ingress|egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MONITOR_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "SC",
        "SC ID",
        "Total packet",
        "c2c packet",
        "Flow direction",
        "Ingress",
        "Egress")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_monitor_watermark_t watermark;
    ctc_monitor_config_t config;
    uint32 index = 0;
    uint32 type[5] = {CTC_MONITOR_CONFIG_MON_INFORM_EN, CTC_MONITOR_CONFIG_MON_INFORM_MIN,
                     CTC_MONITOR_CONFIG_MON_INFORM_MAX, CTC_MONITOR_CONFIG_MON_SCAN_EN,
                     CTC_MONITOR_CONFIG_LOG_EN};
    char* str[6] = {"inform en", "inform min", "inform max", "scan en", "log en","queue scan en"};
    sal_memset(&config, 0, sizeof(config));
    sal_memset(&watermark, 0, sizeof(watermark));

    ctc_cli_out("----------------------------------------\n");
    ctc_cli_out("Buffer Monitor:\n");
    ctc_cli_out("----------------------------------------\n");
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        config.dir = CTC_INGRESS;
    }
    else
    {
        config.dir = CTC_EGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("port", gport, argv[index + 1]);
        config.gport = gport;
        watermark.gport = gport;
        config.monitor_type = CTC_MONITOR_BUFFER;
        watermark.monitor_type = CTC_MONITOR_BUFFER;
        for (index = 0; index < 6; index++)
        {
            if(index == 5)
            {
                config.cfg_type = type[3];
                config.buffer_type = CTC_MONITOR_BUFFER_QUEUE;
            }
            else
            {
                config.cfg_type = type[index];
            }
            if (g_ctcs_api_en)
            {
                 ret = ctcs_monitor_get_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_get_config(&config);
            }
            if (ret >= 0)
            {
                ctc_cli_out("%-20s:%-10d\n", str[index], config.value);
            }
        }
        watermark.u.buffer.dir = 0;
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_watermark(g_api_lchip, &watermark);
        }
        else
        {
            ret = ctc_monitor_get_watermark(&watermark);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-20s:%-10d\n", "ingress max buffer", watermark.u.buffer.max_total_cnt);
        }
        watermark.u.buffer.dir = 1;
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_watermark(g_api_lchip, &watermark);
        }
        else
        {
            ret = ctc_monitor_get_watermark(&watermark);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-20s:%-10d\n", "egress max buffer", watermark.u.buffer.max_total_cnt);
        }

        ctc_cli_out("\n----------------------------------------\n");
        ctc_cli_out("Latency Monitor:\n");
        ctc_cli_out("----------------------------------------\n");
        config.monitor_type = CTC_MONITOR_LATENCY;
        watermark.monitor_type = CTC_MONITOR_LATENCY;

        for (index = 0; index < 5; index++)
        {
            config.cfg_type = type[index];
            if (g_ctcs_api_en)
            {
                 ret = ctcs_monitor_get_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_get_config(&config);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-20s :%-10u\n", str[index], config.value);
        }

        config.cfg_type = CTC_MONITOR_CONFIG_LOG_THRD_LEVEL;
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_config(g_api_lchip, &config);
        }
        else
        {
            ret = ctc_monitor_get_config(&config);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if(config.level == 0)
        {
            ctc_cli_out("%-20s :%-10d\n", "log level", config.value);
        }
        else
        {
            ctc_cli_out("%-20s :%-10d\n", "log level", config.level);
        }

        config.cfg_type = CTC_MONITOR_CONFIG_LANTENCY_DISCARD_EN;
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_config(g_api_lchip, &config);
        }
        else
        {
            ret = ctc_monitor_get_config(&config);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-20s :%-10d\n", "discard en", config.value);
        }

        config.cfg_type = CTC_MONITOR_CONFIG_LANTENCY_DISCARD_THRD_LEVEL;
        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_config(g_api_lchip, &config);
        }
        else
        {
            ret = ctc_monitor_get_config(&config);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-20s :%-10d\n", "discard level", config.level);
        }

        if (g_ctcs_api_en)
        {
             ret = ctcs_monitor_get_watermark(g_api_lchip, &watermark);
        }
        else
        {
            ret = ctc_monitor_get_watermark(&watermark);
        }
        ctc_cli_out("%-20s :%-10"PRIu64"\n", "max latency", watermark.u.latency.max_latency);
    }

    index = CTC_CLI_GET_ARGC_INDEX("total");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
        config.monitor_type = CTC_MONITOR_BUFFER;
        for (index = 0; index < 4; index++)
        {
            config.cfg_type = type[index];
            if (g_ctcs_api_en)
            {
                 ret = ctcs_monitor_get_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_get_config(&config);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-20s:%-10d\n", str[index], config.value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("c2c");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_C2C;
        config.monitor_type = CTC_MONITOR_BUFFER;
        for (index = 0; index < 3; index++)
        {
            config.cfg_type = type[index];
            if (g_ctcs_api_en)
            {
                 ret = ctcs_monitor_get_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_get_config(&config);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-20s:%-10d\n", str[index], config.value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("sc");
    if (0xFF != index)
    {
        config.buffer_type = CTC_MONITOR_BUFFER_SC;
        CTC_CLI_GET_UINT32("sc id", config.sc, argv[index + 1]);
        config.monitor_type = CTC_MONITOR_BUFFER;
        for (index = 0; index < 4; index++)
        {
            config.cfg_type = type[index];
            if (g_ctcs_api_en)
            {
                 ret = ctcs_monitor_get_config(g_api_lchip, &config);
            }
            else
            {
                ret = ctc_monitor_get_config(&config);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-20s:%-10d\n", str[index], config.value);
        }
    }
    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_monitor_debug_on,
        ctc_cli_monitor_debug_on_cmd,
        "debug monitor (ctc|sys) (debug-level {func|param|info|error|export} |) (log LOG_FILE |)",
        CTC_CLI_DEBUG_STR,
        "Monitor module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR,
        CTC_CLI_DEBUG_LEVEL_EXPORT,
        CTC_CLI_DEBUG_MODE_LOG,
        CTC_CLI_LOG_FILE)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR | CTC_DEBUG_LEVEL_EXPORT;
    uint8 index = 0;
    char file[MAX_FILE_NAME_SIZE];

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

        index = CTC_CLI_GET_ARGC_INDEX("export");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_EXPORT;
        }

    }

    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (index != 0xFF)
    {
        sal_strcpy((char*)&file, argv[index + 1]);
        level |= CTC_DEBUG_LEVEL_LOGFILE;
        ctc_debug_set_log_file("monitor", "monitor", (void*)&file, 1);
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MONITOR_CTC;
    }
    else
    {
        typeenum = MONITOR_SYS;

    }

    ctc_debug_set_flag("monitor", "monitor", typeenum, level, TRUE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_monitor_debug_off,
        ctc_cli_monitor_debug_off_cmd,
        "no debug monitor (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Monitor Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MONITOR_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MONITOR_SYS;
    }

    ctc_debug_set_flag("monitor", "monitor", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_monitor_show_debug,
        ctc_cli_monitor_show_debug_cmd,
        "show debug monitor (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Monitor Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MONITOR_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MONITOR_SYS;
    }
    en = ctc_debug_get_flag("monitor", "monitor", typeenum, &level);
    ctc_cli_out("Monitor:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off",
                ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

int32
ctc_monitor_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_monitor_buffer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_monitor_latency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_timer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_op_watermark_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_inform_thrd_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_latency_level_thrd_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_set_buffer_log_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_show_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_monitor_show_port_cmd);

    return CLI_SUCCESS;
}



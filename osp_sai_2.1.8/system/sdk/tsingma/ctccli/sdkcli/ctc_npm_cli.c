#if (FEATURE_MODE == 0)
/**
 @file Ctc_npm_cli.c

 @date 2016-04-26

 @version v1.0

 This file defines functions for npm cli module

*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_npm.h"

#define CTC_CLI_NPM_M_STR "NPM Module"

struct sample_thread_s
{
    uint8 lchip;
    uint8 session_id ;
    uint8 rsv[2];
    uint16 interval;
    uint16 num;
    sal_task_t* p_sample_task;
};
typedef struct sample_thread_s sample_thread_t;

#if defined(SDK_IN_USERMODE)
struct sample_cache_s
{
    uint8  valid;
    uint8  session_id;
    uint16 interval;
    uint8  num;
    uint8  tx_en;
    uint16 local_count;
    double ir_local;
    uint64 fd_local;
    uint64 tx_pkts;
    uint64 rx_pkts;
};
typedef struct sample_cache_s sample_cache_t;

sample_cache_t sample_cache[256];
double tx_rate[8] = {0};
#endif

uint8 ipg[8] = {0};
sample_thread_t sample_thread;

int32 ctc_npm_time_delay_transfer(uint64 time, uint32* time_s, uint32* time_ms, uint32* time_us, uint32* time_ns)
{
    *time_s = time / 1000000000;
    *time_ms = (time - ((*time_s) * 1000000000)) / 1000000;
    *time_us = (time - ((*time_s) * 1000000000) - ((*time_ms) * 1000000)) / 1000;
    *time_ns = time - (*time_s) * 1000000000 - (*time_ms) * 1000000 - (*time_us) * 1000;

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_npm_set_session_transmit_en,
        ctc_cli_npm_set_session_transmit_en_cmd,
        "npm session SESSION transmit (enable | disable) (lchip LCHIP_ID | )",
        CTC_CLI_NPM_M_STR,
        "Session id",
        "Value",
        "Transmit",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 enable = 0;
    uint8 session_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if (0 == sal_memcmp(argv[1], "enable", sal_strlen("enable")))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_npm_set_transmit_en(g_api_lchip,  session_id, enable);
    }
    else
    {
        ret = ctc_npm_set_transmit_en(lchip, session_id, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


#if defined(SDK_IN_USERMODE)
STATIC void
_npm_cli_sat_sample_thread(void* param)
{
    int32 ret = 0;
    uint16 loop = 0;
    uint64 ts_tmp = 0;
    uint64 last_ts_tmp = 0;
    uint64 last_rx_bytes = 0;
    uint64 last_rx_pkts = 0;
    uint64 last_tx_pkts = 0;
    uint64 delta_rx_pkts = 0;
    uint64 delta_rx_bytes = 0;
    uint64 delta_tx_pkts = 0;
    uint64 last_total_delay = 0;
    double ir_max = 0;
    double ir_min = 0;
    double ir_tmp = 0;
    uint64 fd_max = 0;
    uint64 fd_min = 0;
    uint64 fd_average = 0;
    uint64 fd_tmp = 0;
    uint8  count = 0;

    sample_thread_t* sample_thread = (sample_thread_t*)param;
    ctc_npm_stats_t stats;
    sal_memset(&stats, 0, sizeof(ctc_npm_stats_t));
    sal_memset(&sample_cache, 0, sizeof(sample_cache));

    for (loop = 0; loop < sample_thread->num + 1; loop++)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_npm_get_stats(g_api_lchip,  sample_thread->session_id, &stats);
        }
        else
        {
            ret = ctc_npm_get_stats(sample_thread->lchip, sample_thread->session_id, &stats);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        }

        if (0 == loop)
        {
            last_ts_tmp = stats.last_ts;
            last_rx_bytes = stats.rx_bytes;
            last_rx_pkts = stats.rx_pkts;
            last_tx_pkts = stats.tx_pkts;
            last_total_delay = stats.total_delay;
            sal_task_sleep(sample_thread->interval);
            continue;
        }

        /*IR during curent interval :bps*/
        ts_tmp = (stats.last_ts >= last_ts_tmp )?(stats.last_ts - last_ts_tmp):(stats.last_ts + (0xFFFFFFFFFFFFFFFFLLU - last_ts_tmp));
        if((stats.rx_bytes >= last_rx_bytes) && (stats.rx_pkts > last_rx_pkts))
        {
            delta_rx_bytes = stats.rx_bytes - last_rx_bytes;
            ir_tmp = (delta_rx_bytes + (stats.rx_pkts - last_rx_pkts)* ipg[sample_thread->session_id]) * 8 / (double)ts_tmp *1000000000;
        }

        ir_max = (ir_tmp > ir_max)?ir_tmp:ir_max;
        ir_min = (1 == loop)? ir_tmp:ir_min;
        ir_min = (ir_min < ir_tmp)?ir_min:ir_tmp;

        /*FD, ns per PDU*/
        if ((stats.rx_pkts>= last_rx_pkts)&& (stats.total_delay >= last_total_delay))
        {
            fd_tmp = (stats.rx_pkts - last_rx_pkts) ? (stats.total_delay - last_total_delay) / (stats.rx_pkts- last_rx_pkts) : 0;
        }
        fd_max = (fd_tmp > fd_max)?fd_tmp:fd_max;
        fd_min = (1 == loop)? fd_tmp:fd_min;
        fd_min = (fd_min < fd_tmp)?fd_min:fd_tmp;

        fd_average = stats.rx_pkts ? (stats.total_delay) / stats.rx_pkts : 0;

        if ((stats.rx_pkts >= last_rx_pkts)&& (stats.tx_pkts >= last_tx_pkts))
        {
            delta_rx_pkts = stats.rx_pkts - last_rx_pkts;
            delta_tx_pkts = stats.tx_pkts - last_tx_pkts;
        }

        /*update*/
        last_ts_tmp = stats.last_ts;
        last_rx_bytes = stats.rx_bytes;
        last_rx_pkts = stats.rx_pkts;
        last_tx_pkts = stats.tx_pkts;
        last_total_delay = stats.total_delay;


        /*add to sample_cache*/
        sample_cache[count].valid = 1;
        sample_cache[count].fd_local = fd_tmp;
        sample_cache[count].ir_local = ir_tmp;
        sample_cache[count].tx_pkts = delta_tx_pkts;
        sample_cache[count].rx_pkts = delta_rx_pkts;
        sample_cache[count].session_id = sample_thread->session_id;
        sample_cache[count].interval = sample_thread->interval;
        sample_cache[count].num = sample_thread->num;
        sample_cache[count].tx_en = stats.tx_en;
        sample_cache[count].local_count = loop;
        if (255 == count)
        {
            count = 0;
        }
        else
        {
            count++;
        }

        sal_task_sleep(sample_thread->interval);
    }

    ctc_cli_out("-------------------------Session %d SAT Report-------------------------\n");
    ctc_cli_out("IR:Information Rate    FTD:Frame Transfer Delay    FL:Frame Loss \n");
    ctc_cli_out("-----------------------------------------------------------------------\n");
    ctc_cli_out("%-30s: %d\n", "Interval(ms)", sample_thread->interval);
    ctc_cli_out("%-30s: %d\n", "Sample num", sample_thread->num);
    if (ir_max > 1000*1000)
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Max(Mbps)", ir_max / 1000 / 1000);
    }
    else if(ir_max > 1000)
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Max(Kbps)", ir_max / 1000);
    }
    else
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Max(bps)", ir_max);
    }

    if (ir_min > 1000*1000)
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Min(Mbps)", ir_min / 1000 / 1000);
    }
    else if(ir_min > 1000)
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Min(Kbps)", ir_min / 1000);
    }
    else
    {
        ctc_cli_out("%-30s: %.2lf\n", "Rx-rate Min(bps)", ir_min);
    }

    ctc_cli_out("%-30s: %"PRIu64"\n", "FTD Max(ns)", fd_max);
    ctc_cli_out("%-30s: %"PRIu64"\n", "FTD Min(ns)", fd_min);
    ctc_cli_out("%-30s: %"PRIu64"\n", "FTD Average(ns)", fd_average);
    ctc_cli_out("%-30s: %s\n", "Transmit pkts enable", stats.tx_en?"Y":"N");
}



CTC_CLI(ctc_cli_npm_show_session_stats,
        ctc_cli_npm_show_session_stats_cmd,
        "show npm session SESSION stats (eth-slm | eth-dmm | eth-lbm | eth-1sl | eth-1dm | eth-tst | flex)\
        (sample-enable INTERVAL NUM|)(lchip LCHIP_ID|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NPM_M_STR,
        "Session id",
        "Value",
        "Stats",
        "SLM",
        "DMM",
        "LBM",
        "1SL",
        "1DM",
        "TST",
        "Flexible packet(OWAMP/TWAMP,FLPDU,UDP ECHO,etc.)",
        "Sample enable",
        "Sample interval ",
        "Sample number ",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 session_id = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_npm_stats_t stats;
    uint32 time_s = 0;
    uint32 time_ms = 0;
    uint32 time_us = 0;
    uint32 time_ns = 0;
    uint64 duration_ts = 0;
    uint64 fl = 0;
    uint16 interval = 0;
    uint8  num = 0;
    double fl_ratio = 0;
    char first_ts[64] = {0};
    char last_ts[64] = {0};
    sal_time_t tv;
    sal_time_t tv1;

    sal_memset(&stats, 0, sizeof(ctc_npm_stats_t));
    sal_memset(&sample_thread, 0, sizeof(sample_thread));

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_npm_get_stats(g_api_lchip,  session_id, &stats);
    }
    else
    {
        ret = ctc_npm_get_stats(lchip, session_id, &stats);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-slm");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for SLM-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);
        ctc_npm_time_delay_transfer(stats.total_far_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total far delay", time_s, time_ms, time_us, time_ns);
        ctc_npm_time_delay_transfer(stats.total_near_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total near delay", time_s, time_ms, time_us, time_ns);

        ctc_cli_out("%-30s: %"PRIu64"\n", "Tx Fcf", stats.tx_fcf);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Tx Fcb", stats.tx_fcb);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Rx Fcl", stats.rx_fcl);
        /*average delay*/
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay/stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total far delay average(ns)", stats.rx_pkts ? stats.total_far_delay/stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total near delay average(ns)", stats.rx_pkts ? stats.total_near_delay/stats.rx_pkts : 0);

    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-1sl");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for 1SL-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);

        /*need seq-en to cal tx_fcf*/
        ctc_cli_out("%-30s: %"PRIu64"\n", "Tx Fcf", stats.tx_fcf);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Rx Fcl", stats.rx_fcl);
        /*average delay*/
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay/stats.rx_pkts : 0);

    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-lbm");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for LBM-------------------------\n", session_id);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-dmm");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for DMM-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);
        ctc_npm_time_delay_transfer(stats.total_far_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total far delay", time_s, time_ms, time_us, time_ns);
        ctc_npm_time_delay_transfer(stats.total_near_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total near delay", time_s, time_ms, time_us, time_ns);

        /*average delay per PDU*/
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay / stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total far delay average(ns)", stats.rx_pkts ? stats.total_far_delay/stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total near delay average(ns)", stats.rx_pkts ? stats.total_near_delay / stats.rx_pkts : 0);

    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-1dm");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for 1DM-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);

        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay / stats.rx_pkts : 0);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-tst");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for TST-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);

        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay / stats.rx_pkts : 0);

    }

    index = CTC_CLI_GET_ARGC_INDEX("flex");
    if (0xFF != index)
    {
        ctc_cli_out("-------------------------Session %d stats for FLEX-------------------------\n", session_id);
        ctc_npm_time_delay_transfer(stats.total_delay, &time_s, &time_ms, &time_us, &time_ns);
        ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Total delay", time_s, time_ms, time_us, time_ns);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total delay average(ns)", stats.rx_pkts ? stats.total_delay / stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total far delay average(ns)", stats.rx_pkts ? stats.total_far_delay / stats.rx_pkts : 0);
        ctc_cli_out("%-30s: %"PRIu64"\n", "Total near delay average(ns)", stats.rx_pkts ? stats.total_near_delay / stats.rx_pkts : 0);

    }
    /*duration*/
    tv = stats.first_ts /1000000000;
    tv1 = stats.last_ts /1000000000;
    sal_strncpy(first_ts, sal_ctime(&tv), sal_strlen(sal_ctime(&tv))-1);
    sal_strncpy(last_ts, sal_ctime(&tv1), sal_strlen(sal_ctime(&tv1))-1);
    ctc_cli_out("%-30s: [%s]\n", "First timestamp", first_ts);
    ctc_cli_out("%-30s: [%s]\n", "Last timestamp", last_ts);
    duration_ts = (stats.last_ts >= stats.first_ts)?(stats.last_ts - stats.first_ts):0;
    ctc_npm_time_delay_transfer(duration_ts, &time_s, &time_ms, &time_us, &time_ns);
    ctc_cli_out("%-30s: %us : %ums : %uus : %uns\n", "Time of duration", time_s, time_ms, time_us, time_ns);

    /*FL*/
    ctc_cli_out("%-30s: %"PRIu64"\n", "Tx packets", stats.tx_pkts);
    ctc_cli_out("%-30s: %"PRIu64"\n", "Rx packets", stats.rx_pkts);
    ctc_cli_out("%-30s: %"PRIu64"\n", "Tx bytes", stats.tx_bytes);
    ctc_cli_out("%-30s: %"PRIu64"\n", "Rx bytes", stats.rx_bytes);
    if (0 == stats.tx_en)
    {
        if (stats.tx_pkts >= stats.rx_pkts)
        {
            fl = stats.tx_pkts - stats.rx_pkts;
            fl_ratio = fl? ((double)fl / stats.tx_pkts):0;
        }
        ctc_cli_out("%-30s: %"PRIu64"\n", "Frame lost", fl);
        ctc_cli_out("%-30s: %.4Lf%%\n", "Frame lost ratio", fl_ratio * 100);
    }
    ctc_cli_out("%-30s: %"PRIu64"\n", "Max delay", stats.max_delay);
    ctc_cli_out("%-30s: %"PRIu64"\n", "Min delay", stats.min_delay);
    ctc_cli_out("%-30s: %u\n", "Max jitter", stats.max_jitter);
    ctc_cli_out("%-30s: %u\n", "Min jitter", stats.min_jitter);
    ctc_cli_out("%-30s: %"PRIu64"\n", "Total jitter", stats.total_jitter);

    ctc_cli_out("%-30s: %s\n", "Transmit pkts enable", stats.tx_en?"Y":"N");

    index = CTC_CLI_GET_ARGC_INDEX("sample-enable");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("interval", interval, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("num", num, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        if (!interval || interval > 1023)
        {
            ctc_cli_out("ERROR: Interval value must be <1-1023>\n");
            return CLI_ERROR;
        }

        if (!num)
        {
            ctc_cli_out("ERROR: NUM must must be <1-255>\n");
            return CLI_ERROR;
        }

        sample_thread.lchip = lchip;
        sample_thread.interval = interval;
        sample_thread.num = num;
        sample_thread.session_id = session_id;

        ret = sal_task_create(&sample_thread.p_sample_task, "sample thread",
                              SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _npm_cli_sat_sample_thread, (void*)&sample_thread);
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}
#endif

CTC_CLI(ctc_cli_npm_clear_session_stats,
        ctc_cli_npm_clear_session_stats_cmd,
        "npm clear stats session SESSION (lchip LCHIP_ID|)",
        CTC_CLI_NPM_M_STR,
        "clear",
        "stats",
        "Session id",
        "Value",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 session_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_npm_clear_stats(g_api_lchip, session_id);
    }
    else
    {
        ret = ctc_npm_clear_stats(lchip, session_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_npm_set_global_cfg,
        ctc_cli_npm_set_global_cfg_cmd,
        "npm global-cfg {session-mode (max-session-number-8 | max-session-number-6 | max-session-number-4) | frame-size-array SIZE1 SIZE2 SIZE3 SIZE4 SIZE5 SIZE6 SIZE7 SIZE8 SIZE9 SIZE10 SIZE11 SIZE12 SIZE13 SIZE14 SIZE15 SIZE16} (lchip LCHIP_ID | )",
        CTC_CLI_NPM_M_STR,
        "Global config",
        "Session mode",
        "Max session number 8",
        "Max session number 6",
        "Max session number 4",
        "Frame size array",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 i = 0;
    uint8 lchip = 0;

    ctc_npm_global_cfg_t npm;

    sal_memset(&npm, 0, sizeof(ctc_npm_global_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_npm_get_global_config(g_api_lchip, &npm);
    }
    else
    {
        ret = ctc_npm_get_global_config(lchip, &npm);
    }

    index = CTC_CLI_GET_ARGC_INDEX("max-session-number-8");
    if (0xFF != index)
    {
        npm.session_mode = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("max-session-number-6");
    if (0xFF != index)
    {
        npm.session_mode = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("max-session-number-4");
    if (0xFF != index)
    {
        npm.session_mode = 2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("frame-size-array");
    if (0xFF != index)
    {
        for (i = 0; i < CTC_NPM_MAX_EMIX_NUM; i++)
        {
            CTC_CLI_GET_UINT16_RANGE("frame size value", npm.emix_size[i], argv[index + 1 + i], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_npm_set_global_config(g_api_lchip, &npm);
    }
    else
    {
        ret = ctc_npm_set_global_config(lchip, &npm);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_npm_debug_on,
        ctc_cli_npm_debug_on_cmd,
        "debug npm (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "NPM module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
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

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = NPM_CTC;

    }
    else
    {
        typeenum = NPM_SYS;

    }

    ctc_debug_set_flag("npm", "npm", typeenum, level, TRUE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_npm_debug_off,
        ctc_cli_npm_debug_off_cmd,
        "no debug npm (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "NPM Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = NPM_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = NPM_SYS;
    }

    ctc_debug_set_flag("npm", "npm", typeenum, level, FALSE);

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_npm_show_global_cfg,
        ctc_cli_npm_show_global_cfg_cmd,
        "show npm global-cfg (lchip LCHIP_ID | )",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NPM_M_STR,
        "Global config",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret = 0;
    uint8 index = 0;
    uint8 i = 0;
    uint8 lchip = 0;
    ctc_npm_global_cfg_t npm;

    sal_memset(&npm, 0, sizeof(ctc_npm_global_cfg_t));
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    if (g_ctcs_api_en)
    {
        ret = ctcs_npm_get_global_config(g_api_lchip, &npm);
    }
    else
    {
        ret = ctc_npm_get_global_config(lchip, &npm);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (0 == npm.session_mode)
    {
        ctc_cli_out("Max Session Number 8\n");
    }
    if (1 == npm.session_mode)
    {
        ctc_cli_out("Max Session Number 6\n");
    }
    if (2 == npm.session_mode)
    {
        ctc_cli_out("Max Session Number 4\n");
    }
    ctc_cli_out("The EMIX Size Array is:\n");
    for (i = 0; i < CTC_NPM_MAX_EMIX_NUM; i++)
    {
        if (npm.emix_size[i] != 0)
        {
            ctc_cli_out("%d     ", npm.emix_size[i]);
        }
    }
    ctc_cli_out("\n");
    return CLI_SUCCESS;

}




#define CTC_NPM_MAX_PKTHDR_LEN 384

#if defined(SDK_IN_USERMODE)
STATIC int32
_npm_cli_pkthdr_get_from_file(char* file, uint8* buf, uint16* pkt_len)
{
    sal_file_t fp = NULL;
    int8  line[128];
    int32 lidx = 0, cidx = 0, tmp_val = 0;

    /* read packet from file */
    fp = sal_fopen(file, "r");
    if (NULL == fp)
    {
        ctc_cli_out("%% Failed to open the file <%s>\n", file);
        return CLI_ERROR;
    }

    sal_memset(line, 0, 128);
    while (sal_fgets((char*)line, 128, fp))
    {
        for (cidx = 0; cidx < 16; cidx++)
        {
            if (1 == sal_sscanf((char*)line + cidx * 2, "%02x", &tmp_val))
            {
                if((lidx * 16 + cidx) >= CTC_NPM_MAX_PKTHDR_LEN)
                {
                    ctc_cli_out("%% header_len larger than %d\n", CTC_NPM_MAX_PKTHDR_LEN);
                    sal_fclose(fp);
                    fp = NULL;
                    return CLI_ERROR;
                }
                buf[lidx * 16 + cidx] = tmp_val;
                (*pkt_len) += 1;
            }
            else
            {
                break;
            }
        }

        lidx++;
    }

    sal_fclose(fp);
    fp = NULL;

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_npm_set_cfg,
        ctc_cli_npm_set_cfg_cmd,
        "npm session SESSION cfg dest-gport GPORT (is-mcast|)(pkt-sample (eth-lbm | eth-tst | eth-slm | eth-1sl | eth-dmm | eth-1dm) | pkt-file FILENAME) \
        frame-size-mode (fixed FRAME_SIZE | increased MIN_FRAME_SIZE MAX_FRAME_SIZE | emixed SIZE1 SIZE2 SIZE3 SIZE4 SIZE5 SIZE6 SIZE7 SIZE8 SIZE9 SIZE10 SIZE11 SIZE12 SIZE13 SIZE14 SIZE15 SIZE16) \
        rate RATE {vlan-id VLAN | cvlan-domain | tx-mode (continuous | pkt-num NUM | tx-period PERIOD) | patter-type (repeat REPEAT_VALUE | random | increase-by-byte | decrease-by-byte | increase-by-word | decrease-by-word) | \
        timeout TIME | burst-en BURSTCNT (ibg IBG_VALUE | ) | ts-en TSOFFSET | seq-en SEQOFFSET | ipg IPG_VALUE | iloop | dm-stats-mode (far|near)|is-ntp-ts|}",
        CTC_CLI_NPM_M_STR,
        "Session id",
        "Value",
        "Config",
        "Destination global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Is dest group id",
        "Sample packet",
        "LBM",
        "TST",
        "SLM",
        "1SL",
        "DMM",
        "1DM",
        "File store packet",
        "File name",
        "Frame size mode",
        "Fixed frame size",
        "Frame size value",
        "Increased frame size",
        "Min frame size",
        "Max frame size",
        "Emixed frame size",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Frame size value",
        "Packet rate",
        "Rate value",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Cvlan domain",
        "Transmit mode",
        "Continuous",
        "Packet number",
        "Number value",
        "Transmit period",
        "Period value",
        "Pattern type",
        "Repeat mode",
        "Repeat value",
        "Random mode",
        "Increase by byte mode",
        "Decrease by byte mode",
        "Increase by word mode",
        "Decrease by word mode",
        "Timeout",
        "Time value",
        "Burst enable",
        "Burst count",
        "Inter burst gap",
        "IBG value",
        "Timestamp enable",
        "Timestamp offset",
        "Sequence enable",
        "Sequence offset",
        "Inter packet gap",
        "IPG value",
        "Iloop",
        "DM stats mode",
        "Far end dm stats mode",
        "Near end dm stats mode",
        "Timestamp format use NTP")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 temp = 0;
    uint8 lchip = 0;
    uint8 i = 0;
    uint8 count = 0;
    ctc_npm_cfg_t cfg;
    static char file[256] = {0};
    uint8 pkt_buf[CTC_NPM_MAX_PKTHDR_LEN] = {0};
    uint16 header_len = 0;
    static uint8 lbm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    static uint8 tst_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x25, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static uint8 slm_pkthd[80] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x37, 0x00, 0x10, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

#if 0
    static uint8 slm_pkthd[80] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x37, 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };
#endif
    static uint8 onesl_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x35, 0x00, 0x10, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };


    static uint8 onedm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x61, 0x2d, 0x00, 0x10, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };


    static uint8 dmm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x61, 0x2f, 0x00, 0x20, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };


    sal_memset(&cfg, 0, sizeof(ctc_npm_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("session id", cfg.session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("dest gport", cfg.dest_gport, argv[1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("is-mcast");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-lbm");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)lbm_pkthd;
        cfg.pkt_format.header_len = 27;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-tst");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)tst_pkthd;
        cfg.pkt_format.header_len = 30;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-slm");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)slm_pkthd;
        cfg.pkt_format.header_len = 74;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-1sl");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)onesl_pkthd;
        cfg.pkt_format.header_len = 39;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-dmm");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)dmm_pkthd;
        cfg.pkt_format.header_len = 55;
        cfg.flag |= CTC_NPM_CFG_FLAG_DMR_NOT_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-1dm");
    if (0xFF != index)
    {
        cfg.pkt_format.pkt_header = (void*)onedm_pkthd;
        cfg.pkt_format.header_len = 31;
    }


    sal_memset(pkt_buf, 0, CTC_NPM_MAX_PKTHDR_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("pkt-file");
    if (0xFF != index)
    {
        /* get packet from file */
        sal_strcpy((char*)file, argv[index + 1]);
        /* get packet from file */
        ret = _npm_cli_pkthdr_get_from_file(file, pkt_buf, &header_len);

        if (CLI_ERROR == ret)
        {
            return CLI_ERROR;
        }

        cfg.pkt_format.pkt_header = (void*)pkt_buf;
        cfg.pkt_format.header_len = header_len;
    }


    index = CTC_CLI_GET_ARGC_INDEX("fixed");
    if (0xFF != index)
    {
        cfg.pkt_format.frame_size_mode = 0;
        CTC_CLI_GET_UINT32_RANGE("frame size", cfg.pkt_format.frame_size, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        temp = index + 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("increased");
    if (0xFF != index)
    {
        cfg.pkt_format.frame_size_mode = 1;
        CTC_CLI_GET_UINT32_RANGE("min frame size", cfg.pkt_format.min_frame_size, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        CTC_CLI_GET_UINT32_RANGE("max frame size", cfg.pkt_format.frame_size, argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
        temp = index + 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("emixed");
    if (0xFF != index)
    {
        cfg.pkt_format.frame_size_mode = 2;
        for (i = 0; i < CTC_NPM_MAX_EMIX_NUM; i++)
        {
            CTC_CLI_GET_UINT16_RANGE("emix size array", cfg.pkt_format.emix_size[i], argv[index + 1 + i], 0, CTC_MAX_UINT16_VALUE);
            if (cfg.pkt_format.emix_size[i] != 0)
            {
                count = count + 1;
            }

        }
        cfg.pkt_format.emix_size_num = count;
        temp = index + 16;
    }

    CTC_CLI_GET_UINT32_RANGE("transmit rate", cfg.rate, argv[temp + 1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("vlan-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", cfg.vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-domain");
    if (0xFF != index)
    {
        cfg.vlan_domain = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("continuous");
    if (0xFF != index)
    {
        cfg.tx_mode = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pkt-num");
    if (0xFF != index)
    {
        cfg.tx_mode = 1;
        CTC_CLI_GET_UINT32_RANGE("packet number", cfg.packet_num, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx-period");
    if (0xFF != index)
    {
        cfg.tx_mode = 2;
        CTC_CLI_GET_UINT32_RANGE("transmit period", cfg.tx_period, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }



    index = CTC_CLI_GET_ARGC_INDEX("repeat");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 0;
        CTC_CLI_GET_UINT32_RANGE("repeat pattern value", cfg.pkt_format.repeat_pattern, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("increase-by-byte");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decrease-by-byte");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 3;
    }

    index = CTC_CLI_GET_ARGC_INDEX("increase-by-word");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decrease-by-word");
    if (0xFF != index)
    {
        cfg.pkt_format.pattern_type = 5;
    }

    index = CTC_CLI_GET_ARGC_INDEX("timeout");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("timeout", cfg.timeout, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("burst-en");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_BURST_EN;
        CTC_CLI_GET_UINT32_RANGE("burst count", cfg.burst_cnt, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ibg");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("inter burst gap", cfg.ibg, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ts-en");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_TS_EN;
        CTC_CLI_GET_UINT16_RANGE("timestamp offset", cfg.pkt_format.ts_offset, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-en");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_SEQ_EN;
        CTC_CLI_GET_UINT8_RANGE("sequence offset", cfg.pkt_format.seq_num_offset, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    cfg.pkt_format.ipg = 20;
    index = CTC_CLI_GET_ARGC_INDEX("ipg");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("inter packet gap", cfg.pkt_format.ipg, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("iloop");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_ILOOP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("far");
    if (0xFF != index)
    {
        cfg.dm_stats_mode= 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("near");
    if (0xFF != index)
    {
        cfg.dm_stats_mode = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-ntp-ts");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_NTP_TS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_npm_set_config(g_api_lchip, &cfg);
    }
    else
    {
        ret = ctc_npm_set_config(lchip, &cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if (cfg.session_id <= 7)
    {
        tx_rate[cfg.session_id] = (float)cfg.rate *1000;   /*bps*/
        ipg[cfg.session_id] = cfg.pkt_format.ipg;
    }
    else
    {
        ctc_cli_out("Invalid npm session id\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;


}

CTC_CLI(ctc_cli_npm_show_sample_cache,
        ctc_cli_npm_show_sample_cache_cmd,
        "show npm sample cache",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NPM_M_STR,
        "Sample",
        "Cache")
{
    uint8 loop = 0;
    uint8 session_id = 0;
    char  rx_rate_str[64] = {0};
    session_id = sample_cache[0].session_id;

    ctc_cli_out("-------------------------NPM Sample Cache----------------------------------------\n");
    ctc_cli_out("Tx-rate:Transmit rate\n");
    ctc_cli_out("Rx-rate:Receive Rate\n");
    ctc_cli_out("FTD:Frame Transfer Delay(unit:ns)\n");
    ctc_cli_out("Tx-en:Autogen transmit pkts enable\n");
    ctc_cli_out("Tx-pkts:Autogen transmit pkts number\n");
    ctc_cli_out("Rx-pkts:Autogen receive pkts number\n");
    ctc_cli_out("----------------------------------------------------------------------------------\n");
    if (sample_cache[0].valid)
    {
        ctc_cli_out("%-15s: %d\n", "session id", session_id);
    }
    else
    {
        ctc_cli_out("%-15s: %s\n", "session id", "N/A");
    }
    ctc_cli_out("%-15s: %d\n", "interval", sample_cache[0].interval);
    ctc_cli_out("%-15s: %d\n", "total num", sample_cache[0].num);
    ctc_cli_out("%-15s: %s\n", "Tx-en", sample_cache[0].tx_en?"Y":"N");
    if (tx_rate[session_id] > 1000*1000)
    {
        ctc_cli_out("%-15s: %.2lf\n", "Tx-rate(Mbps)", tx_rate[session_id]/1000/1000);
    }
    else if(tx_rate[session_id] > 1000)
    {
        ctc_cli_out("%-15s: %.2lf\n", "Tx-rate(Kbps)", tx_rate[session_id]/1000);
    }
    else
    {
        ctc_cli_out("%-15s: %.2lf\n", "Tx-rate(bps)", tx_rate[session_id]);
    }

    ctc_cli_out("----------------------------------------------------------------------------------\n");
    ctc_cli_out("%-10s%-15s%-10s%-10s%-10s\n", "NO.", "Rx-rate", "FTD", "Tx-pkts", "Rx-pkts");
    ctc_cli_out("----------------------------------------------------------------------------------\n");

    if (sample_cache[loop].ir_local > 1000*1000)
    {
        sal_sprintf(rx_rate_str, "%.2f(Mbps)", sample_cache[loop].ir_local / 1000 / 1000);
    }
    else if(sample_cache[loop].ir_local > 1000)
    {
        sal_sprintf(rx_rate_str, "%.2f(Kbps)", sample_cache[loop].ir_local / 1000);
    }
    else
    {
        sal_sprintf(rx_rate_str, "%.2f(bps)", sample_cache[loop].ir_local);
    }

    for (loop = 0; loop < 255; loop++)
    {
        if (sample_cache[loop].valid)
        {
            ctc_cli_out("%-10d%-15s%-10"PRIu64"%-10"PRIu64"%-10"PRIu64"\n", sample_cache[loop].local_count, rx_rate_str,
                        sample_cache[loop].fd_local, sample_cache[loop].tx_pkts, sample_cache[loop].rx_pkts);
        }
    }

    return CLI_SUCCESS;
}
#endif


CTC_CLI(ctc_cli_npm_set_cfg_rx,
        ctc_cli_npm_set_cfg_rx_cmd,
        "npm session SESSION cfg rx (enable {ts-en TSOFFSET|seq-en SEQOFFSET}| disable)",
        CTC_CLI_NPM_M_STR,
        "Session id",
        "Value",
        "Config",
        "Rx roler",
        "Enable",
        "Timestamp enable",
        "Timestamp offset",
        "Sequence enable",
        "Sequence offset",
        "Disable")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_npm_cfg_t cfg;
    sal_memset(&cfg, 0, sizeof(ctc_npm_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("session id", cfg.session_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    cfg.flag |= CTC_NPM_CFG_FLAG_RX_ROLE_EN;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        cfg.rx_role_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        cfg.rx_role_en = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ts-en");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_TS_EN;
        CTC_CLI_GET_UINT16_RANGE("timestamp offset", cfg.pkt_format.ts_offset, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-en");
    if (0xFF != index)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_SEQ_EN;
        CTC_CLI_GET_UINT8_RANGE("sequence offset", cfg.pkt_format.seq_num_offset, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_npm_set_config(g_api_lchip, &cfg);
    }
    else
    {
        ret = ctc_npm_set_config(lchip, &cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


int32
ctc_npm_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_npm_set_session_transmit_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_clear_session_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_set_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_show_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_debug_off_cmd);

#if defined(SDK_IN_USERMODE)
    install_element(CTC_SDK_MODE, &ctc_cli_npm_show_session_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_set_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_set_cfg_rx_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_npm_show_sample_cache_cmd);
#endif

    return CLI_SUCCESS;
}


#endif


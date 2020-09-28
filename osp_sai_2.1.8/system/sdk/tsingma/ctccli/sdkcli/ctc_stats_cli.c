/**
 @file ctc_stats_cli.c

 @date 2009-12-29

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_stats.h"
#include "ctc_stats_cli.h"

#define CTC_STATS_CLI_PRINT(str, pktcnt, byte, mode)                                              \
    do                                                                                            \
    {                                                                                             \
        if (mode == 0)                                                                            \
        {                                                                                         \
            ret = pktcnt ? ctc_cli_out("%-30s%-22"PRIu64"%-22s\n", str, pktcnt, "-") : 0;         \
        }                                                                                         \
        else if (mode == 1)                                                                       \
        {                                                                                         \
            ret = pktcnt ? ctc_cli_out("%-30s%-22"PRIu64"%-22"PRIu64"\n", str, pktcnt, byte) : 0; \
        }                                                                                         \
        else if (mode == 2)                                                                       \
        {                                                                                         \
            ctc_cli_out("%-30s%-22"PRIu64"%-22s\n", str, pktcnt, "-");                            \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
            ctc_cli_out("%-30s%-22"PRIu64"%-22"PRIu64"\n", str, pktcnt, byte);                    \
        }                                                                                         \
    }while(0)

int32
ctc_stats_show_mac_stats(uint16 gport, ctc_mac_stats_dir_t dir, ctc_mac_stats_t stats, uint8 cli_mode)
{
    int32 ret = 0;
    uint8 mode = (cli_mode ? 2 : 0);
    ctc_mac_stats_t stats_tmp;
    uint8 hd_print = 0;

    sal_memset(&stats_tmp, 0, sizeof(stats_tmp));

    if (sal_memcmp(&stats.u, &stats_tmp.u, sizeof(stats_tmp.u)) || cli_mode)
    {
        ctc_cli_out("GPort 0x%04x, %-20s\n", gport, (CTC_STATS_MAC_STATS_RX == dir) ? "Mac Receive:" : "Mac Transmit:");
        ctc_cli_out("-------------------------------------------------------------------\n");
        ctc_cli_out("%-30s%-22s%-22s\n", "Type", "PktCnt", "Byte");
        ctc_cli_out("-------------------------------------------------------------------\n");
        hd_print = 1;
    }
    if (CTC_STATS_MAC_STATS_RX == dir)
    {
        if ((CTC_STATS_MODE_PLUS == stats.stats_mode))
        {
            CTC_STATS_CLI_PRINT("all receive       (total)", stats.u.stats_plus.stats.rx_stats_plus.all_pkts, stats.u.stats_plus.stats.rx_stats_plus.all_octets, (mode + 1));
            CTC_STATS_CLI_PRINT("unicast           (total)", stats.u.stats_plus.stats.rx_stats_plus.ucast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("multicast         (total)", stats.u.stats_plus.stats.rx_stats_plus.mcast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("broadcast         (total)", stats.u.stats_plus.stats.rx_stats_plus.bcast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("undersize         (total)", stats.u.stats_plus.stats.rx_stats_plus.runts_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("oversize          (total)", stats.u.stats_plus.stats.rx_stats_plus.giants_pkts,    0, mode);
            CTC_STATS_CLI_PRINT("fcs erorr         (total)", stats.u.stats_plus.stats.rx_stats_plus.crc_pkts,       0, mode);
            CTC_STATS_CLI_PRINT("overrun           (total)", stats.u.stats_plus.stats.rx_stats_plus.overrun_pkts,   0, mode);
            CTC_STATS_CLI_PRINT("pause             (total)", stats.u.stats_plus.stats.rx_stats_plus.pause_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("fragment          (total)", stats.u.stats_plus.stats.rx_stats_plus.fragments_pkts, 0, mode);
            CTC_STATS_CLI_PRINT("jabber            (total)", stats.u.stats_plus.stats.rx_stats_plus.jabber_pkts,    0, mode);
            CTC_STATS_CLI_PRINT("jumbo             (total)", stats.u.stats_plus.stats.rx_stats_plus.jumbo_events,   0, mode);
            CTC_STATS_CLI_PRINT("drop              (total)", stats.u.stats_plus.stats.rx_stats_plus.drop_events,    0, mode);
            CTC_STATS_CLI_PRINT("fcs erorr, overrun(total)", stats.u.stats_plus.stats.rx_stats_plus.error_pkts,     0, mode);

        }
        if ((CTC_STATS_MODE_DETAIL == stats.stats_mode))
        {
            CTC_STATS_CLI_PRINT("good unicast", stats.u.stats_detail.stats.rx_stats.good_ucast_pkts, stats.u.stats_detail.stats.rx_stats.good_ucast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good multicast", stats.u.stats_detail.stats.rx_stats.good_mcast_pkts, stats.u.stats_detail.stats.rx_stats.good_mcast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good broadcast", stats.u.stats_detail.stats.rx_stats.good_bcast_pkts, stats.u.stats_detail.stats.rx_stats.good_bcast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good pause", stats.u.stats_detail.stats.rx_stats.good_pause_pkts, stats.u.stats_detail.stats.rx_stats.good_pause_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good normal pause", stats.u.stats_detail.stats.rx_stats.good_normal_pause_pkts, stats.u.stats_detail.stats.rx_stats.good_normal_pause_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good pfc pause", stats.u.stats_detail.stats.rx_stats.good_pfc_pause_pkts, stats.u.stats_detail.stats.rx_stats.good_pfc_pause_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good control", stats.u.stats_detail.stats.rx_stats.good_control_pkts, stats.u.stats_detail.stats.rx_stats.good_control_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("fcs error", stats.u.stats_detail.stats.rx_stats.fcs_error_pkts, stats.u.stats_detail.stats.rx_stats.fcs_error_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("mac overrun packets", stats.u.stats_detail.stats.rx_stats.mac_overrun_pkts, stats.u.stats_detail.stats.rx_stats.mac_overrun_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good oversize", stats.u.stats_detail.stats.rx_stats.good_oversize_pkts, stats.u.stats_detail.stats.rx_stats.good_oversize_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good undersize", stats.u.stats_detail.stats.rx_stats.good_undersize_pkts, stats.u.stats_detail.stats.rx_stats.good_undersize_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good 63B", stats.u.stats_detail.stats.rx_stats.good_63_pkts, stats.u.stats_detail.stats.rx_stats.good_63_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("bad 63B", stats.u.stats_detail.stats.rx_stats.bad_63_pkts, stats.u.stats_detail.stats.rx_stats.bad_63_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("64B", stats.u.stats_detail.stats.rx_stats.pkts_64, stats.u.stats_detail.stats.rx_stats.bytes_64, (mode + 1));
            CTC_STATS_CLI_PRINT("65B~127B", stats.u.stats_detail.stats.rx_stats.pkts_65_to_127, stats.u.stats_detail.stats.rx_stats.bytes_65_to_127, (mode + 1));
            CTC_STATS_CLI_PRINT("128B~255B", stats.u.stats_detail.stats.rx_stats.pkts_128_to_255, stats.u.stats_detail.stats.rx_stats.bytes_128_to_255, (mode + 1));
            CTC_STATS_CLI_PRINT("256B~511B", stats.u.stats_detail.stats.rx_stats.pkts_256_to_511, stats.u.stats_detail.stats.rx_stats.bytes_256_to_511, (mode + 1));
            CTC_STATS_CLI_PRINT("512B~1023B", stats.u.stats_detail.stats.rx_stats.pkts_512_to_1023, stats.u.stats_detail.stats.rx_stats.bytes_512_to_1023, (mode + 1));
            CTC_STATS_CLI_PRINT("1024B~1518B", stats.u.stats_detail.stats.rx_stats.pkts_1024_to_1518, stats.u.stats_detail.stats.rx_stats.bytes_1024_to_1518, (mode + 1));
            CTC_STATS_CLI_PRINT("good 1519B", stats.u.stats_detail.stats.rx_stats.good_1519_pkts, stats.u.stats_detail.stats.rx_stats.good_1519_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("bad 1519B", stats.u.stats_detail.stats.rx_stats.bad_1519_pkts, stats.u.stats_detail.stats.rx_stats.bad_1519_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good jumbo", stats.u.stats_detail.stats.rx_stats.good_jumbo_pkts, stats.u.stats_detail.stats.rx_stats.good_jumbo_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("bad jumbo", stats.u.stats_detail.stats.rx_stats.bad_jumbo_pkts, stats.u.stats_detail.stats.rx_stats.bad_jumbo_bytes, (mode + 1));

        }
    }
    else
    {
        if ((CTC_STATS_MODE_PLUS == stats.stats_mode))
        {
            CTC_STATS_CLI_PRINT("all transmit (total)", stats.u.stats_plus.stats.tx_stats_plus.all_pkts, stats.u.stats_plus.stats.tx_stats_plus.all_octets, (mode + 1));
            CTC_STATS_CLI_PRINT("unicast      (total)", stats.u.stats_plus.stats.tx_stats_plus.ucast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("multicast    (total)", stats.u.stats_plus.stats.tx_stats_plus.mcast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("broadcast    (total)", stats.u.stats_plus.stats.tx_stats_plus.bcast_pkts,     0, mode);
            CTC_STATS_CLI_PRINT("underrun     (total)", stats.u.stats_plus.stats.tx_stats_plus.underruns_pkts, 0, mode);
            CTC_STATS_CLI_PRINT("jumbo        (total)", stats.u.stats_plus.stats.tx_stats_plus.jumbo_events,   0, mode);
            CTC_STATS_CLI_PRINT("fcs erorr    (total)", stats.u.stats_plus.stats.tx_stats_plus.error_pkts,     0, mode);
        }
        if ((CTC_STATS_MODE_DETAIL == stats.stats_mode))
        {
            CTC_STATS_CLI_PRINT("good unicast", stats.u.stats_detail.stats.tx_stats.good_ucast_pkts, stats.u.stats_detail.stats.tx_stats.good_ucast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good multicast", stats.u.stats_detail.stats.tx_stats.good_mcast_pkts, stats.u.stats_detail.stats.tx_stats.good_mcast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good broadcast", stats.u.stats_detail.stats.tx_stats.good_bcast_pkts, stats.u.stats_detail.stats.tx_stats.good_bcast_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good pause", stats.u.stats_detail.stats.tx_stats.good_pause_pkts, stats.u.stats_detail.stats.tx_stats.good_pause_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("good control", stats.u.stats_detail.stats.tx_stats.good_control_pkts, stats.u.stats_detail.stats.tx_stats.good_control_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("63B", stats.u.stats_detail.stats.tx_stats.pkts_63, stats.u.stats_detail.stats.tx_stats.bytes_63, (mode + 1));
            CTC_STATS_CLI_PRINT("64B", stats.u.stats_detail.stats.tx_stats.pkts_64, stats.u.stats_detail.stats.tx_stats.bytes_64, (mode + 1));
            CTC_STATS_CLI_PRINT("65B~127B", stats.u.stats_detail.stats.tx_stats.pkts_65_to_127, stats.u.stats_detail.stats.tx_stats.bytes_65_to_127, (mode + 1));
            CTC_STATS_CLI_PRINT("128B~255B", stats.u.stats_detail.stats.tx_stats.pkts_128_to_255, stats.u.stats_detail.stats.tx_stats.bytes_128_to_255, (mode + 1));
            CTC_STATS_CLI_PRINT("256B~511B", stats.u.stats_detail.stats.tx_stats.pkts_256_to_511, stats.u.stats_detail.stats.tx_stats.bytes_256_to_511, (mode + 1));
            CTC_STATS_CLI_PRINT("512B~1023B", stats.u.stats_detail.stats.tx_stats.pkts_512_to_1023, stats.u.stats_detail.stats.tx_stats.bytes_512_to_1023, (mode + 1));
            CTC_STATS_CLI_PRINT("1024B~1518B", stats.u.stats_detail.stats.tx_stats.pkts_1024_to_1518, stats.u.stats_detail.stats.tx_stats.bytes_1024_to_1518, (mode + 1));
            CTC_STATS_CLI_PRINT("1519B", stats.u.stats_detail.stats.tx_stats.pkts_1519, stats.u.stats_detail.stats.tx_stats.bytes_1519, (mode + 1));
            CTC_STATS_CLI_PRINT("jumbo", stats.u.stats_detail.stats.tx_stats.jumbo_pkts, stats.u.stats_detail.stats.tx_stats.jumbo_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("mac underrun", stats.u.stats_detail.stats.tx_stats.mac_underrun_pkts, stats.u.stats_detail.stats.tx_stats.mac_underrun_bytes, (mode + 1));
            CTC_STATS_CLI_PRINT("fcs error", stats.u.stats_detail.stats.tx_stats.fcs_error_pkts, stats.u.stats_detail.stats.tx_stats.fcs_error_bytes, (mode + 1));

        }
    }
    if (hd_print)
    {
        ctc_cli_out("-------------------------------------------------------------------\n");
        ctc_cli_out("\n");
    }
    return ret;
}

CTC_CLI(ctc_cli_stats_show_mac,
        ctc_cli_stats_show_mac_cmd,
        "show stats (mac-rx | mac-tx | mac-all) (port GPHYPORT_ID (end-port END_PORT | ) | all) (mode (plus | detail) | ) (lchip LCHIP| )",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        "Mac reception statistics",
        "Mac transmission statistics",
        "Mac reception and transmission statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "End GPort",
        CTC_CLI_GPHYPORT_ID_DESC,
        "All port",
        "Statistics information from mode",
        "Read info from Asic",
        "Read info from Table",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint16 gport_id = 0;
    uint16 gport_start = 0;
    uint16 gport_end = 0;
    uint16 lport_start = 0;
    uint16 lport_end = 0;
    uint16 loop = 0;
    uint8 loop_dir = 0;
    uint8 loop_dir_start = 0;
    uint8 loop_dir_end = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint32 max_port_num_per_chip = 0;
    ctc_mac_stats_t* p_stats = NULL;
    ctc_mac_stats_dir_t dir = CTC_STATS_MAC_STATS_MAX;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint64 stats_temp[4] = {0};
    uint8 cli_mode = 1;
    uint8 stats_mode = 0;

    stats_mode = CTC_STATS_MODE_DETAIL;
    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("mac-rx");
    if (0xFF != index)
    {
        dir = CTC_STATS_MAC_STATS_RX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-tx");
    if (0xFF != index)
    {
        dir = CTC_STATS_MAC_STATS_TX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("plus");
    if (0xFF != index)
    {
        stats_mode = CTC_STATS_MODE_PLUS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        stats_mode = CTC_STATS_MODE_DETAIL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF != index)
    {
        lport_start = 0;
        lport_end = max_port_num_per_chip - 1;
        if (g_ctcs_api_en)
        {
            ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
        }
        else
        {
            ret = ctc_get_gchip_id(lchip, &gchip);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        cli_mode = 0;
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
            cli_mode = 0;
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

    if (dir == CTC_STATS_MAC_STATS_MAX)
    {
        ctc_cli_out("\n");
        ctc_cli_out("Show Mac Stats Briefly\n");
        ctc_cli_out("-------------------------------------------------------------------------------------\n");
        ctc_cli_out("%-8s%-21s%-21s%-21s%-21s\n", "Gport", "RxCnt", "RxByte", "TxCnt", "TxByte");
        ctc_cli_out("-------------------------------------------------------------------------------------\n");
        loop_dir_start = CTC_STATS_MAC_STATS_RX;
        loop_dir_end   = CTC_STATS_MAC_STATS_TX;

        stats_mode = CTC_STATS_MODE_PLUS;
    }
    else
    {
        loop_dir_start = dir;
        loop_dir_end = dir;
    }

    p_stats = (ctc_mac_stats_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_mac_stats_t));
    if (NULL == p_stats)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_stats, 0, sizeof(ctc_mac_stats_t));
    p_stats->stats_mode = stats_mode;
    for (loop = lport_start; loop <= lport_end; loop++)
    {
        gport_id = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        for (loop_dir = loop_dir_start; loop_dir <= loop_dir_end; loop_dir++)
        {

            if(g_ctcs_api_en)
            {
                ret = ctcs_stats_get_mac_stats(g_api_lchip, gport_id, loop_dir, p_stats);
            }
            else
            {
                ret = ctc_stats_get_mac_stats(gport_id, loop_dir, p_stats);
            }

            if (ret < 0)
            {
                continue;
            }
            if (loop_dir == CTC_STATS_MAC_STATS_RX)
            {
                stats_temp[0] = p_stats->u.stats_plus.stats.rx_stats_plus.all_pkts;
                stats_temp[1] = p_stats->u.stats_plus.stats.rx_stats_plus.all_octets;
            }
            if (loop_dir == CTC_STATS_MAC_STATS_TX)
            {
                stats_temp[2] = p_stats->u.stats_plus.stats.tx_stats_plus.all_pkts;
                stats_temp[3] = p_stats->u.stats_plus.stats.tx_stats_plus.all_octets;
            }
            if (dir == CTC_STATS_MAC_STATS_MAX)
            {
                if (loop_dir == CTC_STATS_MAC_STATS_TX)
                {
                    ctc_cli_out("0x%04x  %-21"PRIu64"%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"\n", gport_id, stats_temp[0], stats_temp[1], stats_temp[2], stats_temp[3]);
                }
            }
            else
            {
                ctc_stats_show_mac_stats(gport_id, loop_dir, *p_stats, cli_mode);
            }
        }

    }
    if (dir == CTC_STATS_MAC_STATS_MAX)
    {
        ctc_cli_out("-------------------------------------------------------------------------------------\n");
        ctc_cli_out("\n");
    }

    mem_free(p_stats);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_reset_mac,
        ctc_cli_stats_reset_mac_cmd,
        "clear stats ( mac-rx | mac-tx | mac-all) (port GPHYPORT_ID (end-port GPHYPORT_ID|) | all) (lchip LCHIP|)",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STATS_STR,
        "Mac reception statistics",
        "Mac transmission statistics",
        "Mac reception and transmission statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC
        "End GPort",
        CTC_CLI_GPHYPORT_ID_DESC,
        "All port",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint16 gport_id = 0;
    ctc_mac_stats_dir_t dir = CTC_STATS_MAC_STATS_MAX;
    uint32 max_port_num_per_chip = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint16 loop = 0;
    uint8 loop_dir = 0;
    uint8 loop_dir_start = 0;
    uint8 loop_dir_end = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint16 gport_start = 0;
    uint16 gport_end = 0;
    uint16 lport_start = 0;
    uint16 lport_end = 0;


    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    if (CLI_CLI_STR_EQUAL("mac-rx", 0))
    {
        dir = CTC_STATS_MAC_STATS_RX;
    }
    else if (CLI_CLI_STR_EQUAL("mac-tx", 0))
    {
        dir = CTC_STATS_MAC_STATS_TX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF != index)
    {
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

    if (dir == CTC_STATS_MAC_STATS_MAX)
    {
        loop_dir_start = CTC_STATS_MAC_STATS_RX;
        loop_dir_end = CTC_STATS_MAC_STATS_TX;
    }
    else
    {
        loop_dir_start = dir;
        loop_dir_end = dir;
    }

    for (loop_dir = loop_dir_start; loop_dir <= loop_dir_end; loop_dir++)
    {
        for (loop = lport_start; loop <= lport_end; loop++)
        {
            gport_id = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
            if(g_ctcs_api_en)
            {
                ret = ctcs_stats_clear_mac_stats(g_api_lchip, gport_id, loop_dir);
            }
            else
            {
                ret = ctc_stats_clear_mac_stats(gport_id, loop_dir);
            }
            if (ret < 0)
            {
                continue;
            }
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_reset_cpu_mac,
        ctc_cli_stats_reset_cpu_mac_cmd,
        "clear stats cpu-stats (port GPHYPORT_ID|)",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STATS_STR,
        "cpu mac statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_LPORT_CPU)
{
    int32  ret = 0;
    uint16 gport = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_clear_cpu_mac_stats(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_stats_clear_cpu_mac_stats(gport);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_cpu_mac_stats,
        ctc_cli_stats_show_cpu_mac_stats_cmd,
        "show stats cpu-mac-stats (port GPHYPORT_ID|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        "CPU Statistics",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_LPORT_CPU)
{
    uint8 index = 0;
    int32 ret = 0;
    uint16 gport = 0;
    ctc_cpu_mac_stats_t stats;

    sal_memset(&stats, 0, sizeof(ctc_cpu_mac_stats_t));
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_cpu_mac_stats(g_api_lchip, gport, &stats);
    }
    else
    {
        ret = ctc_stats_get_cpu_mac_stats(gport, &stats);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Mac Receive:\n");
    ctc_cli_out("\tgood packets:%"PRIu64", bytes:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_rx_good_pkts, stats.cpu_mac_stats.cpu_mac_rx_good_bytes);
    ctc_cli_out("\tbad packets:%"PRIu64", bytes:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_rx_bad_pkts, stats.cpu_mac_stats.cpu_mac_rx_bad_bytes);
    ctc_cli_out("\tfcs error packets:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_rx_fcs_error_pkts);
    ctc_cli_out("\tfragment packets:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_rx_fragment_pkts);
    ctc_cli_out("\tmac overrun packets:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_rx_overrun_pkts);

    ctc_cli_out("Mac Transmit:\n");
    ctc_cli_out("\ttotal packets:%"PRIu64", bytes:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_tx_total_pkts, stats.cpu_mac_stats.cpu_mac_tx_total_bytes);
    ctc_cli_out("\tfcs error packets:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_tx_fcs_error_pkts);
    ctc_cli_out("\tmac underrun packets:%"PRIu64"\n", stats.cpu_mac_stats.cpu_mac_tx_underrun_pkts);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_mtu1_packet_length,
        ctc_cli_stats_set_mtu1_packet_length_cmd,
        "stats mtu1-pkt-length port GPHYPORT_ID length PKT_LENGTH",
        CTC_CLI_STATS_STR,
        "MTU1 packet length,packet length larger than this consider as oversized packet.Default is 1518B",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Packet length",
        CTC_CLI_STATS_MTU1_DESC)
{
    int32 ret = 0;
    uint16 length = 0;
    uint16 gport_id = 0;
    ctc_mac_stats_prop_type_t mac_stats_prop_type = CTC_STATS_MAC_STATS_PROP_MAX;
    ctc_mac_stats_property_t prop_data;

    mac_stats_prop_type = CTC_STATS_PACKET_LENGTH_MTU1;
    sal_memset(&prop_data, 0, sizeof(ctc_mac_stats_property_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("length", length, argv[1], 0, CTC_MAX_UINT16_VALUE);


    prop_data.data.length = length;
    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_mac_stats_cfg(g_api_lchip, gport_id, mac_stats_prop_type, prop_data);
    }
    else
    {
        ret = ctc_stats_set_mac_stats_cfg(gport_id, mac_stats_prop_type, prop_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_mtu2_packet_length,
        ctc_cli_stats_set_mtu2_packet_length_cmd,
        "stats mtu2-pkt-length port GPHYPORT_ID length PKT_LENGTH",
        CTC_CLI_STATS_STR,
        "MTU2 packet length,packet length larger than this consider as jumbo packet.Default is 1536B",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Packet length",
        CTC_CLI_STATS_MTU2_DESC)
{
    int32 ret = 0;
    uint16 length = 0;
    uint16 gport_id = 0;
    ctc_mac_stats_prop_type_t mac_stats_prop_type = CTC_STATS_MAC_STATS_PROP_MAX;
    ctc_mac_stats_property_t prop_data;

    mac_stats_prop_type = CTC_STATS_PACKET_LENGTH_MTU2;
    sal_memset(&prop_data, 0, sizeof(ctc_mac_stats_property_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("length", length, argv[1], 0, CTC_MAX_UINT16_VALUE);


    prop_data.data.length = length;
    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_mac_stats_cfg(g_api_lchip, gport_id, mac_stats_prop_type, prop_data);
    }
    else
    {
        ret = ctc_stats_set_mac_stats_cfg(gport_id, mac_stats_prop_type, prop_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_mac_cfg,
        ctc_cli_stats_show_mac_cfg_cmd,
        "show stats ( mtu1-pkt-length | mtu2-pkt-length ) port GPHYPORT_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        "MTU1 packet length,packet length larger than this consider as oversized packet.Default is 1518B",
        "MTU2 packet length,packet length larger than this consider as jumbo packet.Default is 1536B,must greater than MTU1",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 gport_id = 0;
    ctc_mac_stats_prop_type_t mac_stats_prop_type = CTC_STATS_MAC_STATS_PROP_MAX;
    ctc_mac_stats_property_t prop_data;

    sal_memset(&prop_data, 0, sizeof(ctc_mac_stats_property_t));

    if (CLI_CLI_STR_EQUAL("mtu1-pkt-length", 0))
    {
        mac_stats_prop_type = CTC_STATS_PACKET_LENGTH_MTU1;
    }
    else if (CLI_CLI_STR_EQUAL("mtu2-pkt-length", 0))
    {
        mac_stats_prop_type = CTC_STATS_PACKET_LENGTH_MTU2;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_mac_stats_cfg(g_api_lchip, gport_id, mac_stats_prop_type, &prop_data);
    }
    else
    {
        ret = ctc_stats_get_mac_stats_cfg(gport_id, mac_stats_prop_type, &prop_data);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (CTC_STATS_PACKET_LENGTH_MTU1 == mac_stats_prop_type)
    {
        ctc_cli_out("===============================\n");
        ctc_cli_out("MTU1 packet length:       %u\n", prop_data.data.length);
        ctc_cli_out("===============================\n");
    }
    else if (CTC_STATS_PACKET_LENGTH_MTU2 == mac_stats_prop_type)
    {
        ctc_cli_out("===============================\n");
        ctc_cli_out("MTU2 packet length:       %u\n", prop_data.data.length);
        ctc_cli_out("===============================\n");
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_flow_stats_ram_bmp,
        ctc_cli_stats_set_flow_stats_ram_bmp_cmd,
        "stats flow-stats ram-bitmap {( vlan | vrf | acl PRIORITY | ip-ipmc | mpls-lsp | nhp-lsp | mpls-pw | nhp-pw | tunnel | scl | nexthop | nhp-mcast | l3if | ecmp | mac | flow-hash | port-log | policer0 | policer1) (ingress|egress) (ram-bmp BMP) | end }",
        CTC_CLI_STATS_STR,
        "flow-stats",
        "stats ram bitmap",
        "CTC_STATS_STATSID_TYPE_VLAN",
        "CTC_STATS_STATSID_TYPE_VRF",
        "Acl lookup",
        "priority value",
        "CTC_STATS_STATSID_TYPE_IPMC and CTC_STATS_STATSID_TYPE_IP",
        "CTC_STATS_STATSID_TYPE_MPLS_LSP",
        "CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP",
        "CTC_STATS_STATSID_TYPE_MPLS_PW",
        "CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW",
        "CTC_STATS_STATSID_TYPE_TUNNEL",
        "CTC_STATS_STATSID_TYPE_SCL",
        "CTC_STATS_STATSID_TYPE_NEXTHOP",
        "CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST",
        "CTC_STATS_STATSID_TYPE_L3IF",
        "CTC_STATS_STATSID_TYPE_ECMP",
        "CTC_STATS_STATSID_TYPE_MAC",
        "CTC_STATS_STATSID_TYPE_FLOW_HASH",
        "CTC_STATS_STATSID_TYPE_PORT_LOG",
        "CTC_STATS_STATSID_TYPE_POLICER0",
        "CTC_STATS_STATSID_TYPE_POLICER1",
        "Ingress direction",
        "Egress direction",
        "Ram bitmap",
        "Bitmap value bit 0-3",
        "End of config")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 stats_type = CTC_STATS_STATSID_TYPE_MAX;
    uint8 dir = CTC_BOTH_DIRECTION;
    static uint8 is_new = 1;
    static ctc_stats_property_param_t stats_param;
    ctc_stats_property_t data;
    uint8 acl_priority = 0;

    sal_memset(&data, 0, sizeof(ctc_stats_property_t));
    if(is_new)
    {
        sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
        is_new = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_VLAN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_VRF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_ACL;
        CTC_CLI_GET_UINT8("acl", acl_priority, argv[index + 1]);
        if(acl_priority > 7)
        {
            ret = CTC_E_INTR_INVALID_PARAM;
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-ipmc");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_IPMC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls-lsp");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_MPLS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nhp-lsp");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls-pw");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_MPLS_PW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nhp-pw");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tunnel");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_TUNNEL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("scl");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_SCL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_NEXTHOP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nhp-mcast");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l3if");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_L3IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_ECMP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-hash");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_FLOW_HASH;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_MAC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("port-log");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_PORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("policer0");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_POLICER0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("policer1");
    if (index != 0xFF)
    {
        stats_type = CTC_STATS_STATSID_TYPE_POLICER1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ram-bmp");
    if (index != 0xFF)
    {
        if(stats_type == CTC_STATS_STATSID_TYPE_ACL)
        {
            CTC_CLI_GET_UINT16("acl ram bmp", stats_param.acl_ram_bmp[acl_priority][dir], argv[index + 1]);
        }
        else
        {
            CTC_CLI_GET_UINT16("ram bmp", stats_param.flow_ram_bmp[stats_type][dir], argv[index + 1]);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("end");
    if (index != 0xFF)
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_CLASSIFY_FLOW_STATS_RAM;
        if (g_ctcs_api_en)
        {
            ret = ctcs_stats_set_global_cfg(g_api_lchip, stats_param, data);
        }
        else
        {
            ret = ctc_stats_set_global_cfg(stats_param, data);
        }
        sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_global_cfg,
        ctc_cli_stats_set_global_cfg_cmd,
        "stats global ( saturate | hold | clear-after-read ) (igs-port|egs-port|igs-global-fwd|egs-global-fwd|fwd|gmac|sgmac|xgmac|xqmac|cgmac|cpumac) (enable|disable)",
        CTC_CLI_STATS_STR,
        "Global config",
        "Saturate when at max value",
        "StatsPtrFifo on hold",
        "Clear after read",
        "Ingress port",
        "Egress port",
        "Ingress global fwd",
        "Egress global fwd",
        "Forward",
        "Gmac",
        "Sgmac",
        "Xgmac",
        "Xqmac",
        "Cgmac",
        "Cpumac",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    ctc_stats_property_param_t stats_param;
    ctc_stats_property_t data;

    sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
    sal_memset(&data, 0, sizeof(ctc_stats_property_t));

    if (CLI_CLI_STR_EQUAL("saturate", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_SATURATE;
    }
    else if (CLI_CLI_STR_EQUAL("hold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_HOLD;
    }

    if (CLI_CLI_STR_EQUAL("fwd", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("gmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_GMAC;
    }
    else if (CLI_CLI_STR_EQUAL("sgmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_SGMAC;
    }
    else if (CLI_CLI_STR_EQUAL("xqmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_XQMAC;
    }
    else if (CLI_CLI_STR_EQUAL("cgmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_CGMAC;
    }
    else if (CLI_CLI_STR_EQUAL("cpumac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_CPUMAC;
    }

    if (CLI_CLI_STR_EQUAL("enable", 2))
    {
        data.data.enable = TRUE;
    }
    else if (CLI_CLI_STR_EQUAL("disable", 2))
    {
        data.data.enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_global_cfg(g_api_lchip, stats_param, data);
    }
    else
    {
        ret = ctc_stats_set_global_cfg(stats_param, data);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_threshold_cfg,
        ctc_cli_stats_set_threshold_cfg_cmd,
        "stats global ( packet-threshold | byte-threshold | fifo-depth-threshold ) THRESHOLD",
        CTC_CLI_STATS_STR,
        "Global config",
        "deal with general options pkt cnt threshold",
        "deal with general options byte cnt threshold",
        "deal with general options fifo depth threshold",
        "packet-threshold or byte-threshold maxsize: 4095, fifo-depth-threshold maxsize: 0~31")
{
    int32 ret = 0;
    ctc_stats_property_param_t stats_param;
    ctc_stats_property_t data;

    sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
    sal_memset(&data, 0, sizeof(ctc_stats_property_t));
    if (CLI_CLI_STR_EQUAL("packet-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_PKT_CNT_THREASHOLD;
        CTC_CLI_GET_UINT16_RANGE("threshold", data.data.threshold_16byte, argv[1], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("byte-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_BYTE_CNT_THREASHOLD;
        CTC_CLI_GET_UINT16_RANGE("threshold", data.data.threshold_16byte, argv[1], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("fifo-depth-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_FIFO_DEPTH_THREASHOLD;
        CTC_CLI_GET_UINT8_RANGE("threshold", data.data.threshold_8byte, argv[1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_global_cfg(g_api_lchip, stats_param, data);
    }
    else
    {
        ret = ctc_stats_set_global_cfg(stats_param, data);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_get_threshold_cfg,
        ctc_cli_stats_get_threshold_cfg_cmd,
        "show stats global ( packet-threshold | byte-threshold | fifo-depth-threshold )",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        "Global config",
        "deal with general options pkt cnt threshold",
        "deal with general options byte cnt threshold",
        "deal with general options fifo depth threshold")
{
    int32 ret = 0;
    ctc_stats_property_param_t stats_param;
    ctc_stats_property_t data;
    char str1[30];

    sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
    sal_memset(&data, 0, sizeof(ctc_stats_property_t));
    sal_memset(str1, 0, sizeof(str1));
    if (CLI_CLI_STR_EQUAL("packet-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_PKT_CNT_THREASHOLD;
        sal_memcpy(str1, "packet-threshold", sizeof("packet-threshold"));
        if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_global_cfg(g_api_lchip, stats_param, &data);
    }
    else
    {
        ret = ctc_stats_get_global_cfg(stats_param, &data);
    }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("===============================\n");
        ctc_cli_out("%s, threshold:%d\n", str1, data.data.threshold_16byte);
        ctc_cli_out("===============================\n");

    }
    else if (CLI_CLI_STR_EQUAL("byte-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_BYTE_CNT_THREASHOLD;
        sal_memcpy(str1, "byte-threshold", sizeof("byte-threshold"));
        if(g_ctcs_api_en)
        {
             ret = ctcs_stats_get_global_cfg(g_api_lchip, stats_param, &data);
        }
        else
        {
            ret = ctc_stats_get_global_cfg(stats_param, &data);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("===============================\n");
        ctc_cli_out("%s, threshold:%d\n", str1, data.data.threshold_16byte);
        ctc_cli_out("===============================\n");

    }
    else if (CLI_CLI_STR_EQUAL("fifo-depth-threshold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_FIFO_DEPTH_THREASHOLD;
        sal_memcpy(str1, "fifo-depth-threshold", sizeof("fifo-depth-threshold"));
        if(g_ctcs_api_en)
        {
             ret = ctcs_stats_get_global_cfg(g_api_lchip, stats_param, &data);
        }
        else
        {
            ret = ctc_stats_get_global_cfg(stats_param, &data);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("===============================\n");
        ctc_cli_out("%s, threshold:%d\n", str1, data.data.threshold_8byte);
        ctc_cli_out("===============================\n");
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_global_cfg,
        ctc_cli_stats_show_global_cfg_cmd,
        "show stats global ( saturate | hold | clear-after-read ) (igs-port|egs-port|igs-global-fwd|egs-global-fwd|fwd|gmac|sgmac|xgmac|xqmac|cgmac|cpumac)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        "Golbal config",
        "Saturate when at max value",
        "StatsPtrFifo on hold",
        "Clear after read",
        "Ingress port",
        "Egress port",
        "Ingress global fwd",
        "Egress global fwd",
        "Forward",
        "Gmac",
        "Sgmac",
        "Xgmac",
        "Xqmac",
        "Cgmac",
        "Cpumac")
{
    int32 ret = 0;
    ctc_stats_property_param_t stats_param;
    ctc_stats_property_t data;
    char str1[30];
    char str2[30];

    sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));
    sal_memset(str1, 0, sizeof(str1));
    sal_memset(str2, 0, sizeof(str2));

    if (CLI_CLI_STR_EQUAL("saturate", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_SATURATE;
        sal_memcpy(str1, "Saturate", sizeof("Saturate"));
    }
    else if (CLI_CLI_STR_EQUAL("hold", 0))
    {
        stats_param.prop_type = CTC_STATS_PROPERTY_HOLD;
        sal_memcpy(str1, "Hold", sizeof("Hold"));
    }

    if (CLI_CLI_STR_EQUAL("fwd", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_FWD;
        sal_memcpy(str2, "fwd", sizeof("fwd"));
    }
    else if (CLI_CLI_STR_EQUAL("gmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_GMAC;
        sal_memcpy(str2, "gmac", sizeof("gmac"));
    }
    else if (CLI_CLI_STR_EQUAL("sgmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_SGMAC;
        sal_memcpy(str2, "sgmac", sizeof("sgmac"));
    }
    else if (CLI_CLI_STR_EQUAL("xqmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_XQMAC;
        sal_memcpy(str2, "xqmac", sizeof("xqmac"));
    }
    else if (CLI_CLI_STR_EQUAL("cgmac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_CGMAC;
        sal_memcpy(str2, "cgmac", sizeof("cgmac"));
    }
    else if (CLI_CLI_STR_EQUAL("cpumac", 1))
    {
        stats_param.stats_type = CTC_STATS_TYPE_CPUMAC;
        sal_memcpy(str2, "cpumac", sizeof("cpumac"));
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_global_cfg(g_api_lchip, stats_param, &data);
    }
    else
    {
        ret = ctc_stats_get_global_cfg(stats_param, &data);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("===============================\n");
    ctc_cli_out("%s, stats type:%s, enable:%d\n", str1, str2, data.data.enable);
    ctc_cli_out("===============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_debug_on,
        ctc_cli_stats_debug_on_cmd,
        "debug stats (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_STATS_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
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

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = STATS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = STATS_SYS;
    }

    ctc_debug_set_flag("stats", "stats", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_debug_off,
        ctc_cli_stats_debug_off_cmd,
        "no debug stats (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_STATS_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level  = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = STATS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = STATS_SYS;
    }

    ctc_debug_set_flag("stats", "stats", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_debug,
        ctc_cli_stats_show_debug_cmd,
        "show debug stats (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_STATS_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level  = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = STATS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = STATS_SYS;
    }

    en = ctc_debug_get_flag("stats", "stats", typeenum, &level);
    ctc_cli_out("stats:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_log,
        ctc_cli_stats_show_log_cmd,
        "show stats port GPHYPORT_ID ( ingress | egress ) log",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ingress direction",
        "egress direction",
        "log statis")
{
    int32 ret = 0;
    uint16 gport_id = 0;
    ctc_stats_basic_t stats;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;

    sal_memset(&stats, 0, sizeof(ctc_stats_basic_t));

    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_port_log_stats(g_api_lchip, gport_id, dir, &stats);
    }
    else
    {
        ret = ctc_stats_get_port_log_stats(gport_id, dir, &stats);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (CTC_INGRESS == dir)
    {
        ctc_cli_out("Ingress stats:\n");
        ctc_cli_out("\tpacket count:%"PRIu64"\n", stats.packet_count);
        ctc_cli_out("\tbyte count:%"PRIu64"\n", stats.byte_count);

    }
    else if (CTC_EGRESS == dir)
    {
        ctc_cli_out("egress stats:\n");
        ctc_cli_out("\tpacket count:%"PRIu64"\n", stats.packet_count);
        ctc_cli_out("\tbyte count:%"PRIu64"\n", stats.byte_count);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_clear_log,
        ctc_cli_stats_clear_log_cmd,
        "clear stats port GPHYPORT_ID ( ingress | egress ) log",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ingress direction",
        "egress direction",
        "log statis")
{
    int32 ret = 0;
    uint16 gport_id = 0;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;

    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_clear_port_log_stats(g_api_lchip, gport_id, dir);
    }
    else
    {
        ret = ctc_stats_clear_port_log_stats(gport_id, dir);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_drop_log,
        ctc_cli_stats_drop_log_cmd,
        "stats drop-packet {log|flow} (enable | disable)",
        CTC_CLI_STATS_STR,
        "drop paket",
        "log",
        "flow",
        "enable log drop",
        "disable log drop")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_stats_discard_t bitmap = 0;
    bool enable = FALSE;

    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (index != 0xFF)
    {
        bitmap = bitmap | CTC_STATS_RANDOM_LOG_DISCARD_STATS;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("flow");
    if (index != 0xFF)
    {
        bitmap = bitmap | CTC_STATS_FLOW_DISCARD_STATS;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = TRUE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_drop_packet_stats_en(g_api_lchip, bitmap, enable);
    }
    else
    {
        ret = ctc_stats_set_drop_packet_stats_en(bitmap, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_show_drop_log,
        ctc_cli_stats_show_drop_log_cmd,
        "show stats port GPHYPORT_ID drop-packet (log|flow)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "drop paket",
        "log",
        "flow")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_stats_discard_t bitmap = 0;
    bool enable;
    char str[20];

    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (index != 0xFF)
    {
        bitmap = bitmap | CTC_STATS_RANDOM_LOG_DISCARD_STATS;
        sal_memcpy(str, "random-log", sizeof("random-log"));
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("flow");
    if (index != 0xFF)
    {
        bitmap = bitmap | CTC_STATS_FLOW_DISCARD_STATS;
        sal_memcpy(str, "flow-stats", sizeof("flow-stats"));
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_drop_packet_stats_en(g_api_lchip, bitmap, &enable);
    }
    else
    {
        ret = ctc_stats_get_drop_packet_stats_en(bitmap, &enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("stats discard pkt type:%s, enable:%d\n", str, enable);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_create_statsid,
        ctc_cli_stats_create_statsid_cmd,
        "stats create statsid STATS_ID (vlan VLAN_ID | vrf VRF_ID | ipmc | ip | mac | flow-hash | mpls (vc-label|) | scl | acl (priority PRIORITY|) (color-aware|) | tunnel | nexthop | nh-mpls-pw | nh-mpls-lsp | nh-mcast | l3if | fid | ecmp) ((ingress|egress)|)",
        CTC_CLI_STATS_STR,
        "Create",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vrf",
        CTC_CLI_VRFID_ID_DESC,
        "Ipmc",
        "Ip",
        "Mac",
        "Flow hash",
        "Mpls",
        "Vc label",
        "Scl",
        "Acl",
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Color aware",
        "Tunnel",
        "Nexthop",
        "Nexthop mpls pw",
        "Nexthop mpls lsp",
        "Nexthop mcast",
        "L3if",
        "Fid",
        "Ecmp",
        "Ingress",
        "Egress")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_stats_statsid_t stats_statsid;
    uint32 stats_id =0 ;

    sal_memset(&stats_statsid, 0, sizeof(ctc_stats_statsid_t));

    CTC_CLI_GET_UINT32_RANGE("stats id", stats_statsid.stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    stats_id = stats_statsid.stats_id;

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VLAN;
        CTC_CLI_GET_UINT16("vlan id", stats_statsid.statsid.vlan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VRF;
        CTC_CLI_GET_UINT16("vrf id", stats_statsid.statsid.vrf_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipmc");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IPMC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MAC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-hash");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FLOW_HASH;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("vc-label");
        if (index != 0xFF)
        {
            stats_statsid.statsid.is_vc_label = 1;
        }
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MPLS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("scl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_SCL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ACL;
        index = CTC_CLI_GET_ARGC_INDEX("priority");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8("acl priority", stats_statsid.statsid.acl_priority, argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX("color-aware");
        if (index != 0xFF)
        {
            stats_statsid.color_aware = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("tunnel");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_TUNNEL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-pw");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-lsp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mcast");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l3if");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_L3IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ECMP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_EGRESS;
    }
    else
    {
        stats_statsid.dir = CTC_INGRESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_create_statsid(g_api_lchip, &stats_statsid);
    }
    else
    {
        ret = ctc_stats_create_statsid(&stats_statsid);
    }

    if(stats_id != stats_statsid.stats_id)
    {
        ctc_cli_out("stats id: %d\n",stats_statsid.stats_id);
    }


    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;


}

CTC_CLI(ctc_cli_stats_destroy_statsid,
        ctc_cli_stats_destroy_statsid_cmd,
        "stats destroy statsid STATS_ID",
        CTC_CLI_STATS_STR,
        "Destroy",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL)
{
    int32 ret = 0;
    uint32 stats_id = 0;

    CTC_CLI_GET_UINT32_RANGE("stats id", stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_destroy_statsid(g_api_lchip, stats_id);
    }
    else
    {
        ret = ctc_stats_destroy_statsid(stats_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;


}

CTC_CLI(ctc_cli_stats_show_stats,
        ctc_cli_stats_show_stats_cmd,
        "show stats stats-id STATS_ID (color-aware|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL)
{
    int32 ret = 0;
    uint32 stats_id = 0;
    uint8 index = 0xFF;
    ctc_stats_basic_t p_stats[3];

    sal_memset(&p_stats, 0, sizeof(p_stats));
    CTC_CLI_GET_UINT32_RANGE("stats id", stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_get_stats(g_api_lchip, stats_id, p_stats);
    }
    else
    {
        ret = ctc_stats_get_stats(stats_id, p_stats);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("color-aware");
    if (index != 0xFF)
    {
        ctc_cli_out("Green  packets: %"PRId64", bytes: %"PRId64"\n", p_stats[0].packet_count, p_stats[0].byte_count);
        ctc_cli_out("Yellow packets: %"PRId64", bytes: %"PRId64"\n", p_stats[1].packet_count, p_stats[1].byte_count);
        ctc_cli_out("Red    packets: %"PRId64", bytes: %"PRId64"\n", p_stats[2].packet_count, p_stats[2].byte_count);
    }
    else
    {
        ctc_cli_out("packets: %"PRId64", bytes: %"PRId64"\n", p_stats[0].packet_count, p_stats[0].byte_count);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_clear_stats,
        ctc_cli_stats_clear_stats_cmd,
        "clear stats STATS_ID",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_STATS_ID_VAL)
{
    int32 ret = 0;
    uint32 stats_id = 0;

    CTC_CLI_GET_UINT32_RANGE("stats id", stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_clear_stats(g_api_lchip, stats_id);
    }
    else
    {
        ret = ctc_stats_clear_stats(stats_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stats_set_syncup_internal,
        ctc_cli_stats_set_syncup_internal_cmd,
        "stats set syncup internal TIME",
        CTC_CLI_STATS_STR,
        "Set",
        "Sync up",
        "Internal",
        "Second")

{
    int32 ret = 0;
    uint32 stats_interval = 0;

    CTC_CLI_GET_UINT32_RANGE("stats interval", stats_interval, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_set_syncup_cb_internal(g_api_lchip, stats_interval);
    }
    else
    {
        ret = ctc_stats_set_syncup_cb_internal(stats_interval);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_stats_cli_init(void)
{

    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_reset_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_reset_cpu_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_cpu_mac_stats_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_mtu1_packet_length_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_mtu2_packet_length_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_mac_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_flow_stats_ram_bmp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_threshold_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_get_threshold_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_log_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_clear_log_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_drop_log_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_drop_log_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_stats_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_debug_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_stats_create_statsid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_destroy_statsid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_show_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_stats_clear_stats_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_stats_set_syncup_internal_cmd);

    return CLI_SUCCESS;
}


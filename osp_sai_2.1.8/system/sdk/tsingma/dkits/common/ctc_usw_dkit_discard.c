#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_discard.h"
#include "ctc_usw_dkit_discard_type.h"


#define ADD_DISCARD_REASON(p_discard_info,id,discard_module,port_num,slice_id, cnt)    \
            if (p_discard_info->discard_count < CTC_DKIT_DISCARD_REASON_NUM)\
            {\
                uint32 disacrd_bitmap = *(((uint32*)g_usw_dkit_master[lchip]->discard_enable_bitmap) + (id/32)); \
                if(DKITS_IS_BIT_SET(disacrd_bitmap, id%32)){                     \
                p_discard_info->discard_reason[p_discard_info->discard_count].reason_id = id;\
                p_discard_info->discard_reason[p_discard_info->discard_count].module = discard_module;\
                p_discard_info->discard_reason[p_discard_info->discard_count].port = port_num;\
                p_discard_info->discard_reason[p_discard_info->discard_count].stats = cnt;\
                p_discard_info->slice_ipe = slice_id;\
                p_discard_info->slice_epe = slice_id;\
                p_discard_info->discard_count++;}\
            }\

#define DKITS_QUEUE_BASE_MISC          512
#define DKITS_QUEUE_BASE_EXCP          640
#define DKITS_QUEUE_BASE_NETWORK_MISC  768
#define DKITS_QUEUE_BASE_EXT           1024
#define DKITS_QUEUE_NUM                (DRV_IS_DUET2(lchip) ? 1280:4096)
#define DKITS_CHANID_DMA_RX0           79
#define DKITS_IS_NETWORK_BASE_QUEUE(queue_id) (queue_id < DKITS_QUEUE_BASE_MISC)
#define DKITS_IS_MISC_CHANNEL_QUEUE(queue_id) ((queue_id >= DKITS_QUEUE_BASE_MISC) && (queue_id < DKITS_QUEUE_BASE_EXCP))
#define DKITS_IS_CPU_QUEUE(queue_id) ((queue_id >= DKITS_QUEUE_BASE_EXCP) && (queue_id < DKITS_QUEUE_BASE_NETWORK_MISC))
#define DKITS_IS_NETWORK_CTL_QUEUE(queue_id) ((queue_id >= DKITS_QUEUE_BASE_NETWORK_MISC) && (queue_id < DKITS_QUEUE_BASE_EXT))
#define DKITS_IS_EXT_QUEUE(queue_id) ((queue_id >= DKITS_QUEUE_BASE_EXT) && (queue_id < DKITS_QUEUE_NUM))
#define DKITS_DISCARD_ALL_PKT_LOG     (DRV_IS_DUET2(lchip) ?  0x37:0x1)

extern ctc_dkit_master_t* g_usw_dkit_master[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

STATIC int32
_ctc_usw_dkit_get_channel_by_queue_id(uint8 lchip, uint32 queue_id, uint32 *channel)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint16 group_id = 0;
    uint8 max_dma_rx_num = 0;
    uint8 loop = 0;
    uint8 subquenum = 0;

    if (DKITS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        *channel = queue_id / 8;
    }
    else if (DKITS_IS_MISC_CHANNEL_QUEUE(queue_id))
    {
        *channel = queue_id / 8;
    }
    else if (DKITS_IS_CPU_QUEUE(queue_id))
    {
        for (loop = 0; loop < 4; loop++)
        {
            cmd = DRV_IOR(DmaStaticInfo_t, DmaStaticInfo_chanEn_f);
            DRV_IOCTL(lchip, loop, cmd, &field_val);
            if (field_val)
            {
                max_dma_rx_num++;
            }
        }
        if (max_dma_rx_num == 4)
        {
            *channel = DKITS_CHANID_DMA_RX0 + (queue_id - DKITS_QUEUE_BASE_EXCP) / 32;
        }
        else
        {
            *channel = DKITS_CHANID_DMA_RX0 + (queue_id - DKITS_QUEUE_BASE_EXCP) / 64;
        }
    }
    else if (DKITS_IS_NETWORK_CTL_QUEUE(queue_id))
    {
        *channel = (queue_id - DKITS_QUEUE_BASE_NETWORK_MISC) / 4;
    }
    else if (DKITS_IS_EXT_QUEUE(queue_id))
    {
        subquenum = DRV_IS_DUET2(lchip) ? 4:8;
        group_id = (queue_id - DKITS_QUEUE_BASE_EXT) / subquenum;
        cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
        DRV_IOCTL(lchip, group_id, cmd, &field_val);
        *channel = field_val;
    }
    else
    {
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

STATIC uint32
_ctc_usw_dkit_get_lport_by_channel(uint8 lchip, uint32 channel)
{
    uint32 loop = 0;
    uint32 lport = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    for (loop = 0; loop < 256; loop++)
    {
        cmd = DRV_IOR(DsQWriteDestPortChannelMap_t, DsQWriteDestPortChannelMap_channelId_f);
        DRV_IOCTL(lchip, loop, cmd, &value);
        if (value == channel)
        {
            lport = loop;
            break;
        }
    }
    return lport;
}

STATIC int32
_ctc_usw_dkit_discard_display_reason(uint8 lchip, ctc_dkit_discard_reason_t* discard_reason, sal_file_t p_wf)
{
    char reason_desc[256] = {0};
    char* temp_desc = NULL;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint8 log_en = 0;
    DKITS_PTR_VALID_CHECK(discard_reason);
    CTC_DKIT_GET_GCHIP(lchip, gchip);
    temp_desc = &reason_desc[0];
    if (discard_reason->port != CTC_DKIT_INVALID_PORT)
    {
        gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, discard_reason->port);
        CTC_DKITS_PRINT_FILE(p_wf, "0x%04x    ", gport);
    }
    else
    {
        CTC_DKITS_PRINT_FILE(p_wf, "%-9s ", "-");
    }
    sal_sprintf(temp_desc, "%s", ctc_usw_dkit_get_reason_desc(discard_reason->reason_id));
    switch (discard_reason->module)
    {
        case CTC_DKIT_DISCARD_FLAG_IPE:
            cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogType_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if ((0x1c == value) || ((discard_reason->reason_id - IPE_START) == value))
            {
                log_en = TRUE;
            }
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n","IPE", discard_reason->stats, log_en ? "Y" : "-",
                sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        case CTC_DKIT_DISCARD_FLAG_EPE:
            cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogType_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if ((37 == value) || ((discard_reason->reason_id - EPE_START) == value))
            {
                log_en = TRUE;
            }
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n", "EPE", discard_reason->stats, log_en ? "Y" : "-",
                    sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        case CTC_DKIT_DISCARD_FLAG_BSR:
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n","BSR", discard_reason->stats, "-",
                sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        case CTC_DKIT_DISCARD_FLAG_OAM:
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n","OAM", discard_reason->stats, "-",
                sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        case CTC_DKIT_DISCARD_FLAG_NET_RX:
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n","NETRX", discard_reason->stats, "-",
                sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        case CTC_DKIT_DISCARD_FLAG_NET_TX:
            CTC_DKITS_PRINT_FILE(p_wf, "%-8s %-7d %-7s %s(%d)\n","NETTX", discard_reason->stats, "-",
                sal_strtok(temp_desc, ","), discard_reason->reason_id);
            break;
        default:
            break;
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_discard_display(ctc_dkit_discard_info_t* p_discard_info, ctc_dkit_discard_para_t* p_para, sal_file_t p_wf)
{
    char yes_no[2][4]={" -", "YES"};
    uint32 i = 0;
    uint32 bsr_match_port_cnt = 0;
    uint32 ipe_match_port_cnt = 0;
    uint32 epe_match_port_cnt = 0;
    uint32 netrx_match_port_cnt = 0;
    uint32 reason_id = 0;

    DKITS_PTR_VALID_CHECK(p_discard_info);

    for (i = 0; i < p_discard_info->discard_count; i++)
    {
        if ((p_para->match_port && (p_para->gport != p_discard_info->discard_reason[i].port)))
        {
            continue;
        }
        else
        {
            reason_id = p_discard_info->discard_reason[i].reason_id;
            if (reason_id >= IPE_FEATURE_START && reason_id <= IPE_FEATURE_END)
            {
                ipe_match_port_cnt++;
            }
            if (reason_id >= EPE_FEATURE_START && reason_id <= EPE_FEATURE_END)
            {
                epe_match_port_cnt++;
            }
            if ((reason_id >= BUFSTORE_ABORT_TOTAL && reason_id <= BUFSTORE_SLIENT_TOTAL)
                || (reason_id >= ENQ_WRED_DROP && reason_id <= ENQ_FWD_DROP_FROM_STACKING_LAG))
            {
                bsr_match_port_cnt++;
            }
            if (reason_id >= NETRX_NO_BUFFER && reason_id <= NETRX_FRAME_ERROR)
            {
                netrx_match_port_cnt++;
            }
        }
    }

    /*1. display total*/
    CTC_DKITS_PRINT_FILE(p_wf, "\n");
    CTC_DKITS_PRINT_FILE(p_wf, "TIME: %s", p_discard_info->systime_str);
    CTC_DKITS_PRINT_FILE(p_wf, "Total info:\n");
    CTC_DKITS_PRINT_FILE(p_wf, "============================================================\n");
    CTC_DKITS_PRINT_FILE(p_wf, " %-10s %-10s %-10s %-10s %-10s %-10s\n", "NETRX", "IPE",  "BSR", "EPE", "NETTX", "OAM");
    CTC_DKITS_PRINT_FILE(p_wf, "------------------------------------------------------------\n");
    if (p_para->match_port)
    {
        CTC_DKITS_PRINT_FILE(p_wf, " %-10s %-10s %-10s %-10s %-10s %-10s\n",
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_NET_RX) && netrx_match_port_cnt],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_IPE) && ipe_match_port_cnt],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_BSR) && bsr_match_port_cnt],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_EPE) && epe_match_port_cnt],
                             yes_no[0],
                             yes_no[0]);
        CTC_DKITS_PRINT_FILE(p_wf, "============================================================\n");
        if (!bsr_match_port_cnt && !ipe_match_port_cnt && !epe_match_port_cnt && !netrx_match_port_cnt)
        {
            return CLI_SUCCESS;
        }
    }
    else
    {
        CTC_DKITS_PRINT_FILE(p_wf, " %-10s %-10s %-10s %-10s %-10s %-10s\n",
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_NET_RX)],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_IPE)],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_BSR)],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_EPE)],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_NET_TX)],
                             yes_no[DKITS_FLAG_ISSET(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_OAM)]);
        CTC_DKITS_PRINT_FILE(p_wf, "============================================================\n");
        if (!p_discard_info->discard_count)
        {
            return CLI_SUCCESS;
        }
    }
    CTC_DKITS_PRINT_FILE(p_wf, "Detail info\n");
    CTC_DKITS_PRINT_FILE(p_wf, "============================================================\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-6s %-9s %-8s %-7s %-7s %-10s\n", "NO.", "PORT",  "MODULE", "CNT", "LOG", "REASON");
    CTC_DKITS_PRINT_FILE(p_wf, "------------------------------------------------------------\n");

    /*2. display by reason id*/
    for (i = 0; i < p_discard_info->discard_count; i++)
    {
        if (p_para->match_port && (p_para->gport != p_discard_info->discard_reason[i].port))
        {
            continue;
        }
        CTC_DKITS_PRINT_FILE(p_wf, "%-6d ", i + 1);
        _ctc_usw_dkit_discard_display_reason(p_discard_info->lchip, &p_discard_info->discard_reason[i], p_wf);
    }
    CTC_DKITS_PRINT_FILE(p_wf, "============================================================\n");
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_discard_get_stats(uint8 lchip, ctc_dkit_discard_stats_t* p_stats, uint32  fliter_flag)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tbl_id = 0;
    uint32 i =0;
    uint8 channel_id = 0;
    uint8 discard_type  = 0;
    NetTxDebugStats_m      net_tx_debug_stats;
    NetRxDebugStatsTable_m net_rx_debug_stats_table;
    PktErrStatsMem_m       pkt_err_stats_mem;
    EpeScheduleDebugStats_m epe_sche_debug_stats;
    BufStoreInputDebugStats_m buf_store_in_debug_stats;
    BufStoreOutputDebugStats_m buf_store_out_debug_stats;
    BufStoreStallDropCntSaturation_m buf_store_stall_drop;
    BufRetrvInputDebugStats_m buf_retrv_in_dbg_stats;
    BufRetrvOutputPktDebugStats_m buf_retrv_out_dbg_stats;
    QMgrEnqResrcDropCntSaturation_m qmgr_enq_resrc_drop;
    QMgrEnqForwardDropCntSaturation_m qmgr_enq_fwd_drop;

    DKITS_PTR_VALID_CHECK(p_stats);

    /* IPE feature discard*/
    tbl_id = DsIngressDiscardStats_t;
    cmd = DRV_IOR(tbl_id, DsIngressDiscardStats_packetCount_f);
    for (i = IPE_FEATURE_START; i <= IPE_FEATURE_END; i++)
    {
        discard_type = i - IPE_FEATURE_START;
        for (channel_id = 0; channel_id < CTC_DKIT_STATS_CHANNEL_NUM; channel_id++)
        {
            DRV_IOCTL(lchip, channel_id*64 + discard_type, cmd, &value);
            p_stats->ipe_stats[channel_id][discard_type] = value;
        }
    }

    /* EPE feature discard*/
    tbl_id = DsEgressDiscardStats_t;
    cmd = DRV_IOR(tbl_id, DsEgressDiscardStats_packetCount_f);

    for (i = EPE_FEATURE_START; i <= EPE_FEATURE_END; i++)
    {
        discard_type = i - EPE_FEATURE_START;
        for (channel_id = 0; channel_id < CTC_DKIT_STATS_CHANNEL_NUM; channel_id++)
        {
            DRV_IOCTL(lchip, ((channel_id << 5) + (channel_id << 4) + (channel_id << 3) + discard_type ), cmd, &value);
            p_stats->epe_stats[channel_id][discard_type] = value;
        }
    }

    /*EPE other discard*/
    sal_memset(&epe_sche_debug_stats, 0, sizeof(epe_sche_debug_stats));
    tbl_id = EpeScheduleDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_sche_debug_stats);
    GetEpeScheduleDebugStats(A, minPktLenErrorCnt_f, &epe_sche_debug_stats, &value);
    p_stats->cnt[EPE_MIN_PKT_LEN_ERR] = value;
    GetEpeScheduleDebugStats(A, elogAbortedPktCnt_f, &epe_sche_debug_stats, &value);
    p_stats->cnt[EPE_ELOG_ABORTED_PKT] = value;
    GetEpeScheduleDebugStats(A, elogDroppedPktCnt_f, &epe_sche_debug_stats, &value);
    p_stats->cnt[EPE_ELOG_DROPPED_PKT] = value;


    /*3. read bsr discard stats*/
    /*3.1 bufstore per channel discard, PktErrStatsMem*/
    for (i = 0; i < (CTC_DKIT_CHANNEL_NUM + CTC_DKIT_MISC_CHANNEL_NUM); i++)
    {
        sal_memset(&pkt_err_stats_mem, 0, sizeof(pkt_err_stats_mem));
        tbl_id = PktErrStatsMem_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, i, cmd, &pkt_err_stats_mem);
        GetPktErrStatsMem(A, pktAbortCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_ABORT_TOTAL - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktAbortOverLenErrorCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_LEN_ERROR - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktSilentDropUnderLenError_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_LEN_ERROR - BUFSTORE_ABORT_TOTAL] += value;
        GetPktErrStatsMem(A, pktSilentDropResrcFailCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_IRM_RESRC - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktAbortDataErrorCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_DATA_ERR - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktSilentDropDataErrorCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_DATA_ERR - BUFSTORE_ABORT_TOTAL] += value;
        GetPktErrStatsMem(A, pktSilentDropChipIdMismatchCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_CHIP_MISMATCH - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktAbortNoBufCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_NO_BUFF - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktSilentDropNoBufCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_NO_BUFF - BUFSTORE_ABORT_TOTAL] += value;

        GetPktErrStatsMem(A, pktAbortFramingErrorCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_NO_EOP - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktAbortMetFifoDropCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][METFIFO_STALL_TO_BS_DROP - BUFSTORE_ABORT_TOTAL] = value;
        GetPktErrStatsMem(A, pktSilentDropCnt_f, &pkt_err_stats_mem, &value);
        p_stats->bsr_stats[i][BUFSTORE_SLIENT_TOTAL - BUFSTORE_ABORT_TOTAL] = value;

    }

    /*3.2 BufStoreInputDebugStats */
    sal_memset(&buf_store_in_debug_stats, 0, sizeof(buf_store_in_debug_stats));
    tbl_id = BufStoreInputDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_store_in_debug_stats);
    GetBufStoreInputDebugStats(A, frDmaDataErrCnt_f, &buf_store_in_debug_stats, &value);
    p_stats->cnt[TO_BUFSTORE_FROM_DMA] = value;
    GetBufStoreInputDebugStats(A, frNetRxDataErrCnt_f, &buf_store_in_debug_stats, &value);
    p_stats->cnt[TO_BUFSTORE_FROM_NETRX] = value;
    GetBufStoreInputDebugStats(A, frOamDataErrCnt_f, &buf_store_in_debug_stats, &value);
    p_stats->cnt[TO_BUFSTORE_FROM_OAM] = value;

    /*3.3 BufStoreOutputDebugStats*/
    sal_memset(&buf_store_out_debug_stats, 0, sizeof(buf_store_out_debug_stats));
    tbl_id = BufStoreOutputDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_store_out_debug_stats);
    GetBufStoreOutputDebugStats(A, sopPktLenErrorCnt_f, &buf_store_out_debug_stats, &value);
    p_stats->cnt[BUFSTORE_OUT_SOP_PKT_LEN_ERR] = value;

    /*3.4 BufStoreStallDropCntSaturation*/
    sal_memset(&buf_store_stall_drop, 0, sizeof(buf_store_stall_drop));
    tbl_id = BufStoreStallDropCntSaturation_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_store_stall_drop);
    GetBufStoreStallDropCntSaturation(A, dmaMcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_MC_FROM_DMA] = value;
    GetBufStoreStallDropCntSaturation(A, dmaUcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_UC_FROM_DMA] = value;
    GetBufStoreStallDropCntSaturation(A, eLoopMcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_MC_FROM_ELOOP_MCAST] = value;
    GetBufStoreStallDropCntSaturation(A, eLoopUcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_UC_FROM_ELOOP_UCAST] = value;
    GetBufStoreStallDropCntSaturation(A, ipeHdrMcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_MC_FROM_IPE_CUT_THR] = value;
    GetBufStoreStallDropCntSaturation(A, ipeHdrUcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_UC_FROM_IPE_CUT_THR] = value;
    GetBufStoreStallDropCntSaturation(A, ipeMcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_MC_FROM_IPE] = value;
    GetBufStoreStallDropCntSaturation(A, ipeUcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_UC_FROM_IPE] = value;
    GetBufStoreStallDropCntSaturation(A, oamMcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_MC_FROM_OAM] = value;
    GetBufStoreStallDropCntSaturation(A, oamUcastDropCnt_f, &buf_store_stall_drop, &value);
    p_stats->cnt[METFIFO_UC_FROM_OAM] = value;

    /*3.5 BufRetrvInputDebugStats*/
    sal_memset(&buf_retrv_in_dbg_stats, 0, sizeof(buf_retrv_in_dbg_stats));
    tbl_id = BufRetrvInputDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_retrv_in_dbg_stats);
    GetBufRetrvInputDebugStats(A, frDeqMsgDiscardCnt_f, &buf_retrv_in_dbg_stats, &value);
    p_stats->cnt[BUFRETRV_FROM_DEQ_MSG_ERR] = value;
    GetBufRetrvInputDebugStats(A, frQMgrLenErrorCnt_f, &buf_retrv_in_dbg_stats, &value);
    p_stats->cnt[BUFRETRV_FROM_QMGR_LEN_ERR] = value;

    /*3.6 BufRetrvOutputPktDebugStats*/

    sal_memset(&buf_retrv_out_dbg_stats, 0, sizeof(buf_retrv_out_dbg_stats));
    tbl_id = BufRetrvOutputPktDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_retrv_out_dbg_stats);
    GetBufRetrvOutputPktDebugStats(A, toIntfFifoErrorCnt_f, &buf_retrv_out_dbg_stats, &value);
    p_stats->cnt[BUFRETRV_OUT_DROP] = value;

    /*3.7 BSR per queue discard stats*/
    for (i = 0; i < CTC_DKIT_STATS_QUEUE_NUM; i++)
    {
        /*QMgrEnqResrcDropCntSaturation*/
        sal_memset(&qmgr_enq_resrc_drop, 0, sizeof(qmgr_enq_resrc_drop));
        tbl_id = QMgrEnqResrcDropCntSaturation_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, i, cmd, &qmgr_enq_resrc_drop);
        GetQMgrEnqResrcDropCntSaturation(A, dropCntAmQueueDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_WRED_DROP - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntFinalDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_TOTAL_DROP - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntFreePtrEmptyDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_NO_QUEUE_ENTRY - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntPortDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_PORT_NO_BUFF - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntQueueDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_QUEUE_NO_BUFF - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntScDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_SC_NO_BUFF - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntSpanOnDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_SPAN_NO_BUFF - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntTotalDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_TOTAL_NO_BUFF - ENQ_WRED_DROP] = value;
        GetQMgrEnqResrcDropCntSaturation(A, dropCntForwardDropRd_f, &qmgr_enq_resrc_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP - ENQ_WRED_DROP] = value;
        /*QMgrEnqForwardDropCntSaturation*/
        sal_memset(&qmgr_enq_fwd_drop, 0, sizeof(qmgr_enq_fwd_drop));
        tbl_id = QMgrEnqForwardDropCntSaturation_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, i, cmd, &qmgr_enq_fwd_drop);
        GetQMgrEnqForwardDropCntSaturation(A, dropCntCFlexIsolateBlockRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_CFLEX_ISOLATE_BLOCK - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntChanIdInvalidRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_CHAN_INVALID - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntForwardDropValidRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_TOTAL - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntFrLagDiscardRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_FROM_LAG - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntFrLagErrorRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_FROM_LAG_ERR - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntMcastLinkAggDiscardRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_FROM_LAG_MC - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntNoLinkAggMemDiscardRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_FROM_LAG_NO_MEM - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntPortIsolateBlockRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_PORT_ISOLATE_BLOCK - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntRsvChanDropValidRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_RSV_CHAN_DROP - ENQ_WRED_DROP] = value;
        GetQMgrEnqForwardDropCntSaturation(A, dropCntStackingDiscardRd_f, &qmgr_enq_fwd_drop, &value);
        p_stats->qmgr_enq_stats[i][ENQ_FWD_DROP_FROM_STACKING_LAG - ENQ_WRED_DROP] = value;
    }

    /*4. read net rx discard stats*/
    for (i = 0; i < CTC_DKIT_CHANNEL_NUM; i++)
    {
        sal_memset(&net_rx_debug_stats_table, 0, sizeof(net_rx_debug_stats_table));
        tbl_id = NetRxDebugStatsTable_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, i, cmd, &net_rx_debug_stats_table);
        GetNetRxDebugStatsTable(A, fullDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        p_stats->netrx_stats[i][NETRX_NO_BUFFER - NETRX_NO_BUFFER] = value;
        if (DRV_IS_DUET2(lchip))
        {
            GetNetRxDebugStatsTable(A, lenErrDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        }
        else
        {
            uint32 buf[2] = {0};
            GetNetRxDebugStatsTable(A, overLenErrDropCntSaturation_f, &net_rx_debug_stats_table, &buf[0]);
            GetNetRxDebugStatsTable(A, underLenErrDropCntSaturation_f, &net_rx_debug_stats_table, &buf[1]);
            value = buf[0] + buf[1];
        }
        p_stats->netrx_stats[i][NETRX_LEN_ERROR - NETRX_NO_BUFFER] = value;
        if (DRV_IS_DUET2(lchip))
        {
            GetNetRxDebugStatsTable(A, pktErrDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        }
        else
        {
            uint32 buf[2] = {0};
            GetNetRxDebugStatsTable(A, pktErrDropCntSaturation_f, &net_rx_debug_stats_table, &buf[0]);
            GetNetRxDebugStatsTable(A, pktWithErrorCntSaturation_f, &net_rx_debug_stats_table, &buf[1]);
            value = buf[0] + buf[1];
        }
        p_stats->netrx_stats[i][NETRX_PKT_ERROR - NETRX_NO_BUFFER] = value;
        GetNetRxDebugStatsTable(A, pauseDiscardCntSaturation_f, &net_rx_debug_stats_table, &value);
        p_stats->netrx_stats[i][NETRX_BPDU_ERROR - NETRX_NO_BUFFER] = value;
        GetNetRxDebugStatsTable(A, frameErrDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        p_stats->netrx_stats[i][NETRX_FRAME_ERROR - NETRX_NO_BUFFER] = value;
    }

    /*5. read net tx discard stats*/
    sal_memset(&net_tx_debug_stats, 0, sizeof(net_tx_debug_stats));
    tbl_id = NetTxDebugStats_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &net_tx_debug_stats);
    GetNetTxDebugStats(A, minLenDropCnt_f, &net_tx_debug_stats, &value);
    p_stats->cnt[NETTX_MIN_LEN] = value;
    GetNetTxDebugStats(A, rxNoBufDropCnt_f, &net_tx_debug_stats, &value);
    p_stats->cnt[NETTX_NO_BUFFER] = value;
    GetNetTxDebugStats(A, rxNoEopDropCnt_f, &net_tx_debug_stats, &value);
    p_stats->cnt[NETTX_SOP_EOP] = value;
    GetNetTxDebugStats(A, rxNoSopDropCnt_f, &net_tx_debug_stats, &value);
    p_stats->cnt[NETTX_SOP_EOP] += value;
    GetNetTxDebugStats(A, txErrorCnt_f, &net_tx_debug_stats, &value);
    p_stats->cnt[NETTX_TX_ERROR] = value;


    /*6. read oam discard stats*/
    cmd = DRV_IOR(OamFwdDebugStats_t, OamFwdDebugStats_asicHardDiscardCnt_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    p_stats->cnt[OAM_HW_ERROR] = value;
    cmd = DRV_IOR(OamFwdDebugStats_t, OamFwdDebugStats_exceptionDiscardCnt_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    p_stats->cnt[OAM_EXCEPTION] = value;

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_discard_get_reason(const ctc_dkit_discard_stats_t* p_last_stats,
                                        const ctc_dkit_discard_stats_t* p_cur_stats,
                                        ctc_dkit_discard_info_t* p_discard_info)
{
    uint32 i = 0;
    uint8 slice = 0;
    sal_time_t tv;
    char* p_time_str = NULL;
    uint8 lchip = 0;
    uint8 channel_id = 0;
    uint8 discard_type  = 0;
    uint8 reason_type  = 0;
    uint32 lport = 0;
    uint32 channel = 0;

    DKITS_PTR_VALID_CHECK(p_last_stats);
    DKITS_PTR_VALID_CHECK(p_cur_stats);
    DKITS_PTR_VALID_CHECK(p_discard_info);

    lchip = p_discard_info->lchip;
    /*get systime*/
    sal_time(&tv);
    p_time_str = sal_ctime(&tv);
    if(p_time_str)
    {
        sal_strncpy(p_discard_info->systime_str, p_time_str, 50);
    }

    /*IPE discard*/
    for (i = IPE_FEATURE_START; i <= IPE_FEATURE_END; i++)
    {
        discard_type = i - IPE_FEATURE_START;
        for(channel_id = 0; channel_id < CTC_DKIT_STATS_CHANNEL_NUM; channel_id++)
        {
            if (p_last_stats->ipe_stats[channel_id][discard_type] != p_cur_stats->ipe_stats[channel_id][discard_type])
            {
                DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_IPE);
                ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_IPE, channel_id, 0, p_cur_stats->ipe_stats[channel_id][discard_type]);
            }
        }
    }

    /*EPE discard*/
    for (i = EPE_FEATURE_START; i <= EPE_FEATURE_END; i++)
    {
        discard_type = i - EPE_FEATURE_START;
        for(channel_id = 0; channel_id < CTC_DKIT_STATS_CHANNEL_NUM; channel_id++)
        {
            if (p_last_stats->epe_stats[channel_id][discard_type] != p_cur_stats->epe_stats[channel_id][discard_type])
            {
                DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_EPE);
                ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_EPE, channel_id, 0, p_cur_stats->epe_stats[channel_id][discard_type]);
            }
        }
    }
    /*EPE other discard*/
    for (i = EPE_MIN_PKT_LEN_ERR; i <= EPE_ELOG_DROPPED_PKT; i++)
    {
        if (p_last_stats->cnt[i] != p_cur_stats->cnt[i])
        {
            DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_EPE);
            ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_EPE, CTC_DKIT_INVALID_PORT, 0, p_cur_stats->cnt[i]);
        }
    }

    /* BSR discard*/
    for (i = BSR_START; i <= BSR_END; i++)
    {
        if (p_last_stats->cnt[i] != p_cur_stats->cnt[i])
        {
            DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_BSR);
            ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_BSR, CTC_DKIT_INVALID_PORT, 0, p_cur_stats->cnt[i]);
        }
    }
    /* BSR per channel discard*/
    for(i = 0; i < (CTC_DKIT_CHANNEL_NUM+CTC_DKIT_MISC_CHANNEL_NUM); i++)
    {
        for(reason_type = BUFSTORE_ABORT_TOTAL; reason_type <= BUFSTORE_SLIENT_TOTAL; reason_type++)
        {
            if(p_last_stats->bsr_stats[i][reason_type - BUFSTORE_ABORT_TOTAL]
                    != p_cur_stats->bsr_stats[i][reason_type - BUFSTORE_ABORT_TOTAL])
            {
                lport = _ctc_usw_dkit_get_lport_by_channel(lchip, i);   /*map channel to lport*/
                DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_BSR);
                ADD_DISCARD_REASON(p_discard_info, reason_type, CTC_DKIT_DISCARD_FLAG_BSR,
                    lport, 0, p_cur_stats->bsr_stats[i][reason_type - BUFSTORE_ABORT_TOTAL]);
            }

        }
    }
    /*BSR per queue discard*/
    for(i = 0; i < CTC_DKIT_STATS_QUEUE_NUM; i++)
    {
        for(reason_type = ENQ_WRED_DROP; reason_type <= ENQ_FWD_DROP_FROM_STACKING_LAG; reason_type++)
        {
            if(p_last_stats->qmgr_enq_stats[i][reason_type - ENQ_WRED_DROP]
                    != p_cur_stats->qmgr_enq_stats[i][reason_type - ENQ_WRED_DROP])
            {
                /*map queue to lport*/
                _ctc_usw_dkit_get_channel_by_queue_id(lchip, i, &channel);
                lport = _ctc_usw_dkit_get_lport_by_channel(lchip, channel);
                DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_BSR);
                ADD_DISCARD_REASON(p_discard_info, reason_type, CTC_DKIT_DISCARD_FLAG_BSR,
                    lport, 0, p_cur_stats->qmgr_enq_stats[i][reason_type - ENQ_WRED_DROP]);
            }

        }
    }
    /*NETRX per channel discard*/
    for(i = 0; i < CTC_DKIT_CHANNEL_NUM; i++)
    {
        for(reason_type = NETRX_NO_BUFFER; reason_type <= NETRX_FRAME_ERROR; reason_type++)
        {
            if(p_last_stats->netrx_stats[i][reason_type - NETRX_NO_BUFFER]
                != p_cur_stats->netrx_stats[i][reason_type - NETRX_NO_BUFFER])
            {
                lport = _ctc_usw_dkit_get_lport_by_channel(lchip, i);
                DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_NET_RX);
                ADD_DISCARD_REASON(p_discard_info, reason_type, CTC_DKIT_DISCARD_FLAG_NET_RX,
                    lport, 0, p_cur_stats->netrx_stats[i][reason_type - NETRX_NO_BUFFER]);
            }

        }
    }

    /*NETTX discard*/
    for (i = NETTX_START; i <= NETTX_END; i++)
    {
        if (p_last_stats->cnt[i] != p_cur_stats->cnt[i])
        {
            DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_NET_TX);
            ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_NET_TX, CTC_DKIT_INVALID_PORT, slice, p_cur_stats->cnt[i]);
        }
    }

    /*OAM discard*/
    for (i = OAM_START; i <= OAM_END; i++)
    {
        if (p_last_stats->cnt[i] != p_cur_stats->cnt[i])
        {
            DKITS_SET_FLAG(p_discard_info->discard_flag, CTC_DKIT_DISCARD_FLAG_OAM);
            ADD_DISCARD_REASON(p_discard_info, i, CTC_DKIT_DISCARD_FLAG_OAM, CTC_DKIT_INVALID_PORT, slice, p_cur_stats->cnt[i]);
        }
    }

    return CLI_SUCCESS;

}


int32
ctc_usw_dkit_discard_process(void* p_para)
{
    ctc_dkit_discard_para_t* p_discard_para = (ctc_dkit_discard_para_t *)p_para;
    ctc_dkit_discard_stats_t* cur_stats = NULL;
    ctc_dkit_discard_info_t* p_discard_info = NULL;
    sal_file_t  p_wf = NULL;
    uint8 front = 0;
    uint8 lchip = 0;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_discard_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);
    DKITS_PTR_VALID_CHECK(g_usw_dkit_master[lchip]);

    p_wf = p_discard_para->p_wf;
    cur_stats = (ctc_dkit_discard_stats_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_discard_stats_t));
    if (NULL == cur_stats)
    {
        return CLI_ERROR;
    }

    p_discard_info = (ctc_dkit_discard_info_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_discard_info_t));
    if (NULL == p_discard_info)
    {
        mem_free(cur_stats);
        return CLI_ERROR;
    }


    sal_memset(cur_stats, 0, sizeof(ctc_dkit_discard_stats_t));
    sal_memset(p_discard_info, 0, sizeof(ctc_dkit_discard_info_t));

    if (p_discard_para->history)/*show history*/
    {
        front = g_usw_dkit_master[lchip]->discard_history.front;
        while (g_usw_dkit_master[lchip]->discard_history.rear != front)
        {
            _ctc_usw_dkit_discard_display(&(g_usw_dkit_master[lchip]->discard_history.discard[front++]), p_discard_para, p_wf);
            front %= CTC_DKIT_DISCARD_HISTORY_NUM;
        }
    }
    else
    {
        /*1. read discard stats*/
        _ctc_usw_dkit_discard_get_stats(lchip,cur_stats, 0);

        /*2. compare discard stats and get reason id*/
        _ctc_usw_dkit_discard_get_reason(&(g_usw_dkit_master[lchip]->discard_stats), cur_stats,  p_discard_info);
        sal_memcpy(&(g_usw_dkit_master[lchip]->discard_stats), cur_stats, sizeof(ctc_dkit_discard_stats_t));

        /*3. display result*/
        _ctc_usw_dkit_discard_display(p_discard_info, p_discard_para, p_wf);

        /*4. store result history*/
        if (p_discard_info->discard_flag != 0)
        {
            sal_memcpy(&(g_usw_dkit_master[lchip]->discard_history.discard[g_usw_dkit_master[lchip]->discard_history.rear++]),
                       p_discard_info, sizeof(ctc_dkit_discard_info_t));
            g_usw_dkit_master[lchip]->discard_history.rear %= CTC_DKIT_DISCARD_HISTORY_NUM;
            if (g_usw_dkit_master[lchip]->discard_history.rear == g_usw_dkit_master[lchip]->discard_history.front)
            {
                g_usw_dkit_master[lchip]->discard_history.front++;
                g_usw_dkit_master[lchip]->discard_history.front %= CTC_DKIT_DISCARD_HISTORY_NUM;
            }
        }
    }

    mem_free(cur_stats);
    mem_free(p_discard_info);

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_discard_print_type_by_reason(uint32 reason_id, char* module)
{
    char buff[256] = {0};
    char* temp_desc = NULL;
    char* temp_empty = NULL;
    temp_desc = &buff[0];
    sal_sprintf(temp_desc, "%s", ctc_usw_dkit_get_reason_desc(reason_id));
    CTC_DKIT_PRINT("Module     : %s\n", module);
    CTC_DKIT_PRINT("ReasonId   : %d\n", reason_id);
    CTC_DKIT_PRINT("DiscardType: %s\n", sal_strtok(temp_desc, ","));
    CTC_DKIT_PRINT("Description:%s\n", sal_strtok(temp_empty, ","));
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_discard_print_type(uint32 reason_id, char* module)
{
#define PRINTF_LEN 57
    uint8 desc_len = 0;
    char buff0[64]= {0};
    char buff[256] = {0};
    desc_len = sal_strlen(ctc_usw_dkit_get_reason_desc(reason_id));
    sal_sprintf(buff, "%s", ctc_usw_dkit_get_reason_desc(reason_id));
    sal_memcpy(buff0, buff, PRINTF_LEN);
    if (desc_len > PRINTF_LEN)
    {
        if ((((buff0[PRINTF_LEN - 1]>='a')&&(buff0[PRINTF_LEN - 1]<='z'))||((buff0[PRINTF_LEN - 1]>='A')&&(buff0[PRINTF_LEN - 1]<='Z')))
            &&((buff[PRINTF_LEN]>='a')&&(buff[PRINTF_LEN]<='z')))
        {
            buff0[PRINTF_LEN] = '-';
            buff0[PRINTF_LEN+1] = '\0';
            CTC_DKIT_PRINT(" %-10d %-9s %-58s\n", reason_id, module, buff0);
        }
        else
        {
            buff0[PRINTF_LEN] = '\0';
            CTC_DKIT_PRINT(" %-10d %-9s %-58s\n", reason_id, module, buff0);
        }
        if (' ' == buff[PRINTF_LEN])
        {
            CTC_DKIT_PRINT(" %-20s %s\n", "", buff + PRINTF_LEN + 1);
        }
        else
        {
            CTC_DKIT_PRINT(" %-20s %s\n", "", buff + PRINTF_LEN);
        }
    }
    else
    {
        CTC_DKIT_PRINT(" %-10d %-9s %-58s\n", reason_id, module, buff);
    }
    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_discard_show_type(void* p_para)
{
    uint32 i = 0;
    ctc_dkit_discard_para_t* p_discard_para = (ctc_dkit_discard_para_t *)p_para;
    char* module = "  -";

    DKITS_PTR_VALID_CHECK(p_para);
    DRV_INIT_CHECK(p_discard_para->lchip);
    if (0xFFFFFFFF == p_discard_para->reason_id) /*all*/
    {
        CTC_DKIT_PRINT("--------------------------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s %-9s %s\n", "ReasonId", "Module", "Description");
        for (i = IPE_START; i <= IPE_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "IPE");
        }
        for (i = EPE_START; i <= EPE_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "EPE");
        }
        for (i = BSR_START; i <= BSR_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "BSR");
        }
        for (i = NETRX_START; i <= NETRX_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "NETRX");
        }
        for (i = NETTX_START; i <= NETTX_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "NETTX");
        }
        for (i = OAM_START; i <= OAM_END; i++)
        {
            _ctc_usw_dkit_discard_print_type(i, "OAM");
        }
        CTC_DKIT_PRINT("--------------------------------------------------------------------------------\n");
    }
    else
    {
        CTC_DKIT_PRINT("--------------------------------------------------------------\n");
        if ((p_discard_para->reason_id >= IPE_START) && (p_discard_para->reason_id <= IPE_END))
        {
            module = "IPE";
        }
        else if ((p_discard_para->reason_id >= EPE_START) && (p_discard_para->reason_id <= EPE_END))
        {
            module = "EPE";
        }
        else if ((p_discard_para->reason_id >= BSR_START) && (p_discard_para->reason_id <= BSR_END))
        {
            module = "BSR";
        }
        else if ((p_discard_para->reason_id >= NETRX_START) && (p_discard_para->reason_id <= NETRX_END))
        {
            module = "NETRX";
        }
        else if ((p_discard_para->reason_id >= NETTX_START) && (p_discard_para->reason_id <= NETTX_END))
        {
            module = "NETTX";
        }
        else if ((p_discard_para->reason_id >= OAM_START) && (p_discard_para->reason_id <= OAM_END))
        {
            module = "OAM";
        }
        else
        {
            CTC_DKIT_PRINT(" Invalid reason id\n");
            CTC_DKIT_PRINT("--------------------------------------------------------------\n");
            return CLI_SUCCESS;
        }
        _ctc_usw_dkit_discard_print_type_by_reason(p_discard_para->reason_id, module);
        CTC_DKIT_PRINT("--------------------------------------------------------------\n");
    }
    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_discard_to_cpu_en(void* p_para)
{
    uint8 lchip = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 all_discard = 0;
    ctc_dkit_discard_para_t* p_discard_para = (ctc_dkit_discard_para_t *)p_para;
    DKITS_PTR_VALID_CHECK(p_para);
    DRV_INIT_CHECK(p_discard_para->lchip);

    lchip = p_discard_para->lchip;
    if (0xFFFFFFFF == p_discard_para->reason_id)
    {
        all_discard = 1;
    }
    else
    {
        if (!((p_discard_para->reason_id >= IPE_START && p_discard_para->reason_id <= IPE_END)
            || (p_discard_para->reason_id >= EPE_START && p_discard_para->reason_id <= EPE_END)))
        {
            CTC_DKIT_PRINT("Not support!!!\n");
            return CLI_ERROR;
        }
    }

    if ((p_discard_para->reason_id >= IPE_START && p_discard_para->reason_id <= IPE_END)
        ||all_discard)
    {
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogEn_f);
        value = p_discard_para->discard_to_cpu_en;
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogType_f);
        value = all_discard? 0x1c :(p_discard_para->reason_id - IPE_START);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    if ((p_discard_para->reason_id >= EPE_START && p_discard_para->reason_id <= EPE_END)
        || all_discard)
    {
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogEn_f);
        value = p_discard_para->discard_to_cpu_en;
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogType_f);
        value = all_discard? DKITS_DISCARD_ALL_PKT_LOG :(p_discard_para->reason_id - EPE_START);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_discard_print_stats(uint8 lchip, uint32 reason_id, char* module)
{
#define BUF_LEN 128
    char buff[BUF_LEN] = {0};
    const char* desc = NULL;
    uint16 gport = 0;
    uint8 gchip = 0;
    uint16 stats = 0;
    uint8 i = 0;
    uint32 cmd = 0;
    uint8 chan = 0;
    uint32 local_phy_port = 0;
    ctc_dkit_discard_stats_t* cur_stats = NULL;

    cur_stats = (ctc_dkit_discard_stats_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_discard_stats_t));
    if (NULL == cur_stats)
    {
        return CLI_ERROR;
    }
    sal_memset(cur_stats, 0, sizeof(ctc_dkit_discard_stats_t));
    _ctc_usw_dkit_discard_get_stats(lchip,cur_stats, 0);

    desc = ctc_usw_dkit_get_reason_desc(reason_id);
    CTC_DKIT_GET_GCHIP(lchip, gchip);
    for (i = 0; i < BUF_LEN; i++)
    {
        if (desc[i] == ',')
        {
            break;
        }
        buff[i] = desc[i];
    }

    cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
    if (reason_id >= BUFSTORE_ABORT_TOTAL && reason_id <= BUFSTORE_SLIENT_TOTAL)
    {
        for (chan = 0; chan < CTC_DKIT_CHANNEL_NUM; chan++ )
        {
            DRV_IOCTL(lchip, chan, cmd, &local_phy_port);
            gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port);
            stats = cur_stats->bsr_stats[chan][reason_id - BUFSTORE_ABORT_TOTAL];
            if (stats)
            {
                CTC_DKIT_PRINT(" %-10d %-8s 0x%04x   %-8d %s\n", reason_id, module, gport, stats, buff);
            }
        }
        goto End;
    }
    else if (reason_id >= NETRX_NO_BUFFER && reason_id <= NETRX_FRAME_ERROR)
    {
        for (chan = 0; chan < CTC_DKIT_CHANNEL_NUM; chan++ )
        {
            DRV_IOCTL(lchip, chan, cmd, &local_phy_port);
            gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port);
            stats = cur_stats->netrx_stats[chan][reason_id - NETRX_NO_BUFFER];
            if (stats)
            {
                CTC_DKIT_PRINT(" %-10d %-8s 0x%04x   %-8d %s\n", reason_id, module, gport, stats, buff);
            }
        }
        goto End;
    }
    else if (reason_id >= IPE_FEATURE_START && reason_id <= IPE_FEATURE_END)
    {
        for (chan = 0; chan < CTC_DKIT_CHANNEL_NUM; chan++ )
        {
            DRV_IOCTL(lchip, chan, cmd, &local_phy_port);
            gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port);
            stats = cur_stats->ipe_stats[chan][reason_id - IPE_FEATURE_START];
            if (stats)
            {
                CTC_DKIT_PRINT(" %-10d %-8s 0x%04x   %-8d %s\n", reason_id, module, gport, stats, buff);
            }
        }
        goto End;
    }
    else if (reason_id >= EPE_FEATURE_START && reason_id <= EPE_FEATURE_END)
    {
        for (chan = 0; chan < CTC_DKIT_CHANNEL_NUM; chan++ )
        {
            DRV_IOCTL(lchip, chan, cmd, &local_phy_port);
            gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port);
            stats = cur_stats->epe_stats[chan][reason_id - EPE_FEATURE_START];
            if (stats)
            {
                CTC_DKIT_PRINT(" %-10d %-8s  0x%04x   %-8d %s\n", reason_id, module, gport, stats, buff);
            }
        }
        goto End;
    }

    stats = cur_stats->cnt[reason_id];
    if (stats)
    {
        CTC_DKIT_PRINT(" %-10d %-8s %-8s %-8d %s\n", reason_id, module, "-", stats, buff);
    }

End:
    mem_free(cur_stats);
    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_discard_show_stats(void* p_para)
{
    uint32 i = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t* p_discard_para = (ctc_dkit_discard_para_t *)p_para;

    DKITS_PTR_VALID_CHECK(p_para);

    lchip = p_discard_para->lchip;
    DRV_INIT_CHECK(lchip);
    CTC_DKIT_PRINT("--------------------------------------------------------------------------------\n");
    CTC_DKIT_PRINT(" %-10s %-8s %-8s %-8s %s\n", "ReasonId", "Module", "Port", "Stats", "Reason");
    CTC_DKIT_PRINT("--------------------------------------------------------------------------------\n");

    for (i = IPE_START; i <= IPE_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "IPE");
    }
    for (i = EPE_START; i <= EPE_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "EPE");
    }
    for (i = BSR_START; i <= BSR_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "BSR");
    }
    for (i = NETRX_START; i <= NETRX_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "NETRX");
    }
    for (i = NETTX_START; i <= NETTX_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "NETTX");
    }
    for (i = OAM_START; i <= OAM_END; i++)
    {
        _ctc_usw_dkit_discard_print_stats(lchip, i, "OAM");
    }
    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_discard_display_en(void* p_para)
{

    uint8 lchip = 0;
    uint32* disacrd_bitmap = NULL;
    ctc_dkit_discard_para_t* p_discard_para = (ctc_dkit_discard_para_t *)p_para;
    DKITS_PTR_VALID_CHECK(p_para);

    lchip = p_discard_para->lchip;
    DRV_INIT_CHECK(lchip);
    if (0xFFFFFFFF == p_discard_para->reason_id)
    {
        if (p_discard_para->discard_display_en)
        {
            sal_memset(g_usw_dkit_master[lchip]->discard_enable_bitmap, 0xFF , ((CTC_DKIT_DISCARD_MAX / 32) + 1)*4);
        }
        else
        {
            sal_memset(g_usw_dkit_master[lchip]->discard_enable_bitmap, 0 , ((CTC_DKIT_DISCARD_MAX / 32) + 1)*4);
        }
    }
    else
    {
        disacrd_bitmap = (((uint32*)g_usw_dkit_master[lchip]->discard_enable_bitmap) + (p_discard_para->reason_id / 32));
        if (p_discard_para->discard_display_en)
        {
            DKITS_BIT_SET(*disacrd_bitmap, p_discard_para->reason_id % 32);
        }
        else
        {
            DKITS_BIT_UNSET(*disacrd_bitmap, p_discard_para->reason_id % 32);
        }
    }

    return CLI_SUCCESS;
}



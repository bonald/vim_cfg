#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_monitor.h"
#include "ctc_dkit_common.h"
//#include "common/duet2/ctc_dt2_dkit_misc.h"
#include "common/tsingma/ctc_tm_dkit_misc.h"

#define CTC_DKIT_MONITOR_CONGEST_LEVEL_NUM  3
#define CTC_DKIT_MONITOR_TC_NUM  8

#define CTC_DKIT_MONITOR_NETWORK_CHANNEL_QUEUE_BASE  0
#define CTC_DKIT_MONITOR_MISC_CHANNEL_QUEUE_BASE  512
#define CTC_DKIT_MONITOR_EXCP_QUEUE_BASE  640
#define CTC_DKIT_MONITOR_NETWORK_MISC_QUEUE_BASE  768
#define CTC_DKIT_MONITOR_EXTENDER_QUEUE_BASE  1024
#define CTC_DKIT_MONITOR_MAX_QUEUE_NUM    (DRV_IS_DUET2(lchip)?1280:4096)

#define CTC_DKIT_MONITOR_DMA_CHANNEL_ID_BASE 79
#define CTC_DKIT_MONITOR_NETWORK_CHANNEL_NUM 64

extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

STATIC uint8
_ctc_usw_dkit_monitor_get_congest_level(uint8 lchip, bool is_igress, uint8 sc)
{
    uint8 i = 0;
    uint32 total = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tbl_id = MaxTblId_t;
    uint32 field_id = 0;

    if (is_igress)
    {
        cmd = DRV_IOR(DsIrmScCnt_t, DsIrmScCnt_g_0_scCnt_f );
        DRV_IOCTL(lchip, 0, cmd, &total);
        tbl_id = DsIrmScCngThrdProfile_t;
        field_id = DsIrmScCngThrdProfile_g_0_scCngThrd_f;
    }
    else
    {
        cmd = DRV_IOR(DsErmScCnt_t, DsErmScCnt_g_0_scCnt_f + sc);
        DRV_IOCTL(lchip, 0, cmd, &total);
        tbl_id = DsErmScCngThrdProfile_t;
        field_id = DsErmScCngThrdProfile_g_0_scCngThrd_f;
    }

    for (i = 0; i < CTC_DKIT_MONITOR_CONGEST_LEVEL_NUM; i++)
    {
        cmd = DRV_IOR(tbl_id, field_id + i);
        DRV_IOCTL(lchip, sc, cmd, &value);
        if ((total>>5) < value)
        {
            return i;
        }
    }

    return i;
}

STATIC uint32
_ctc_usw_dkit_monitor_get_channel_by_gport(uint8 lchip, uint16 gport)
{
    uint16 local_phy_port = 0;
    uint32 channel = 0xFFFFFFFF;
    uint8 i = 0;
    uint32 cmd = 0;
    uint32 value = 0;

    local_phy_port = gport;
    for (i = 0; i < CTC_DKIT_ONE_SLICE_CHANNEL_NUM; i++)
    {
        cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
        DRV_IOCTL(lchip, i, cmd, &value);
        if (value == local_phy_port)
        {
            channel = i;
            break;
        }
    }

    return channel;
}

STATIC uint32
_ctc_usw_dkit_monitor_get_channel_by_queue_id(uint8 lchip, uint16 queue_id)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint16 group_id = 0;
    uint8 channel = 0;
    uint8 cpu_queue_mode = 0;
    uint8 subquenum = 0;

    if (queue_id < CTC_DKIT_MONITOR_MISC_CHANNEL_QUEUE_BASE)
    {
        channel = queue_id / 8;
    }
    else if(queue_id < CTC_DKIT_MONITOR_EXCP_QUEUE_BASE)
    {
        channel = queue_id / 8;
    }
    else if(queue_id < CTC_DKIT_MONITOR_NETWORK_MISC_QUEUE_BASE)
    {
        cmd = DRV_IOR(QWriteCtl_t, QWriteCtl_cpuQueueMode_f);
        DRV_IOCTL(lchip, 0, cmd, &field_val);
        cpu_queue_mode = field_val;

        if (cpu_queue_mode == 1)
        {
            channel = CTC_DKIT_MONITOR_DMA_CHANNEL_ID_BASE + (queue_id - CTC_DKIT_MONITOR_EXCP_QUEUE_BASE) / 32;
        }
        else
        {
            channel = CTC_DKIT_MONITOR_DMA_CHANNEL_ID_BASE + (queue_id - CTC_DKIT_MONITOR_EXCP_QUEUE_BASE) / 64;
        }
    }
    else if(queue_id < CTC_DKIT_MONITOR_EXTENDER_QUEUE_BASE)
    {
        channel = (queue_id - CTC_DKIT_MONITOR_NETWORK_MISC_QUEUE_BASE) / 4;
    }
    else if(queue_id < CTC_DKIT_MONITOR_MAX_QUEUE_NUM)
    {
        subquenum = DRV_IS_DUET2(lchip) ? 4:8;
        group_id = (queue_id - CTC_DKIT_MONITOR_EXTENDER_QUEUE_BASE) / subquenum;
        cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
        DRV_IOCTL(lchip, group_id, cmd, &field_val);
        channel = field_val;
    }
    else
    {
        CTC_DKIT_PRINT("Invalid queue_id:%d!!!\n", queue_id);
    }

    return channel;
}

int32
ctc_usw_dkit_monitor_show_queue_id(void* p_para)
{
    uint32 cmd = 0;
    uint32 qid = 0;
    uint32 chanid = 0;
    uint8 i = 0;
    uint32 tbl_idx = 0;
    uint32 local_phy_port = 0;
    uint16 gport  = 0;
    uint8 gchip = 0;
    uint8 lchip = 0;
    ctc_dkit_monitor_para_t* p_monitor_para = (ctc_dkit_monitor_para_t*)p_para;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_monitor_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);
    CTC_DKIT_GET_GCHIP(lchip, gchip);
    CTC_DKIT_PRINT("%-6s %-10s %-10s %-10s\n", "NO.", "QueID", "ChanId","Port");
    CTC_DKIT_PRINT("-----------------------------------\n");
    for (i = 0; i < 5; i++)
    {
        cmd = DRV_IOR(QMgrEnqDebugInfo_t, QMgrEnqDebugInfo_queueIdMon0_f + i);
        DRV_IOCTL(lchip, 0, cmd, &qid);

        chanid = _ctc_usw_dkit_monitor_get_channel_by_queue_id(lchip, qid);

        if (chanid < CTC_DKIT_MONITOR_NETWORK_CHANNEL_NUM)
        {
            cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
            tbl_idx = chanid;
            DRV_IOCTL(lchip, tbl_idx, cmd, &local_phy_port);
            gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port);
            CTC_DKIT_PRINT("%-6d %-10d %-10d 0x%04x\n", i + 1, qid, chanid, gport);
        }
        else
        {
            CTC_DKIT_PRINT("%-6d %-10d %-10d %-6s\n", i + 1, qid, chanid, "-");
        }

    }

    CTC_DKIT_PRINT("-----------------------------------\n");
    CTC_DKIT_PRINT("Tips: NO.1 is the newest QueueID\n\n");

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_igr_summary(uint8 lchip, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint8 i = 0;
    uint32 value0 = 0;
    char desc[16] = {0};
    uint8 congest_level = 0;
    uint32 c2c_packet_cnt = 0;
    uint32 critical_packet_cnt = 0;
    uint32 igr_total_cnt = 0;
    uint32 c2c_packet_thrd = 0;
    uint32 critical_packet_thrd = 0;
    uint32 igr_total_thrd = 0;
    DsIrmMiscCnt_m irm_misc_cnt;
    DsIrmMiscThrd_m irm_misc_thrd;

    sal_memset(&irm_misc_cnt, 0, sizeof(DsIrmMiscCnt_m));
    cmd = DRV_IOR(DsIrmMiscCnt_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &irm_misc_cnt);
    GetDsIrmMiscCnt(A, c2cCnt_f, &irm_misc_cnt, &c2c_packet_cnt);
    GetDsIrmMiscCnt(A, criticalCnt_f, &irm_misc_cnt, &critical_packet_cnt);
    GetDsIrmMiscCnt(A, totalCnt_f, &irm_misc_cnt, &igr_total_cnt);

    sal_memset(&irm_misc_thrd, 0, sizeof(DsIrmMiscThrd_m));
    cmd = DRV_IOR(DsIrmMiscThrd_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &irm_misc_thrd);
    GetDsIrmMiscThrd(A, c2cThrd_f, &irm_misc_thrd, &c2c_packet_thrd);
    GetDsIrmMiscThrd(A, criticalThrd_f, &irm_misc_thrd, &critical_packet_thrd);
    GetDsIrmMiscThrd(A, totalThrd_f, &irm_misc_thrd, &igr_total_thrd);

    congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, TRUE, 0);
    CTC_DKITS_PRINT_FILE(p_wf, "\n----------congest config-----------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10s\n", "level", "Thrd");
    CTC_DKITS_PRINT_FILE(p_wf, "-----------------------------------\n");
    for (i = 0; i < CTC_DKIT_MONITOR_CONGEST_LEVEL_NUM; i++)
    {
        cmd = DRV_IOR(DsIrmScCngThrdProfile_t, DsIrmScCngThrdProfile_g_0_scCngThrd_f + i);
        DRV_IOCTL(lchip, 0, cmd, &value0);
        if (i == congest_level)
        {
            sal_sprintf(desc, "%d%s", i, " (*)");
        }
        else
        {
            sal_sprintf(desc, "%d", i);
        }
        CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d\n", desc, value0<<5);
    }

    CTC_DKITS_PRINT_FILE(p_wf, "\n-----------------------------------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10s %-10s\n", "Type", "Depth", "Thrd");
    CTC_DKITS_PRINT_FILE(p_wf, "-----------------------------------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Total", igr_total_cnt, igr_total_thrd);

     if ((igr_total_cnt >= (critical_packet_cnt + c2c_packet_cnt))
         && (igr_total_thrd >= (critical_packet_thrd + c2c_packet_thrd)))
     {
         CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Normal",
                        (igr_total_cnt - critical_packet_cnt - c2c_packet_cnt),
                        igr_total_thrd - critical_packet_thrd - c2c_packet_thrd);
     }

    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Critical", critical_packet_cnt, critical_packet_thrd);
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "C2C", c2c_packet_cnt, c2c_packet_thrd);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_igr_detail_port_tc(uint8 lchip, uint16 channel, sal_file_t p_wf)
{
    uint8 i = 0;
    char desc[16] = {0};
    uint32 depth = 0 ;
    uint32 index= 0;
    uint32 cmd = 0;
    uint8 congest_level = 0;
    uint32 profId = 0;
    uint32 port_tc_thrd = 0;

    congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, TRUE, 0);
    CTC_DKITS_PRINT_FILE(p_wf, "\n--------------------------------------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-10s%-10s%-10s\n", "TC",  "Depth", "Thrd");
    CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
    index = channel;
    for (i = 0; i < CTC_DKIT_MONITOR_TC_NUM; i++)
    {
        cmd = DRV_IOR(DsIrmPortTcCnt_t, DsIrmPortTcCnt_u_g1_portTcCnt_f);
        DRV_IOCTL(lchip, (((index & 0x3f) << 3) + i), cmd, &depth);

        cmd = DRV_IOR(DsIrmPortTcCfg_t, DsIrmPortTcCfg_u_g1_portTcLimitedThrdProfId_f);
        DRV_IOCTL(lchip, (((index & 0x3f) << 3) + i), cmd, &profId);

        cmd = DRV_IOR(DsIrmPortTcLimitedThrdProfile_t, DsIrmPortTcLimitedThrdProfile_g_0_portTcLimitedThrd_f);
        DRV_IOCTL(lchip, (((profId&0xF) << 2) + congest_level), cmd, &port_tc_thrd);

        sal_sprintf(desc, "TC%d", i);
        CTC_DKITS_PRINT_FILE(p_wf, "%-10s%-10d%-10d\n", desc,  depth, port_tc_thrd << 3);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_igr_detail(uint8 lchip, uint16 gport, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint32 channel = 0;
    uint32 index = 0;
    uint32 depth = 0;
    uint32 local_phy_port = 0;
    uint32 gchip = 0;
    uint32 i = 0;
    uint8 congest_level = 0;
    uint32 profId = 0;
    uint32 port_thrd = 0;
    bool b_all_zero = TRUE;

    congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, TRUE, 0);
    if (0xFFFF == gport)
    {
        CTC_DKITS_PRINT_FILE(p_wf, "\n--------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_wf, "%-10s%-10s%-10s%-10s\n",
                       "Port", "Channel", "Depth", "Thrd");
        CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
        for (i = 0; i < CTC_DKIT_ONE_SLICE_CHANNEL_NUM; i++)
        {
            cmd = DRV_IOR(DsIrmPortCnt_t, DsIrmPortCnt_portCnt_f);
            DRV_IOCTL(lchip, i, cmd, &depth);
            if (depth)
            {
                cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
                DRV_IOCTL(lchip, i, cmd, &local_phy_port);
                CTC_DKIT_GET_GCHIP(lchip, gchip);

                cmd = DRV_IOR(DsIrmPortCfg_t, DsIrmPortCfg_portLimitedThrdProfId_f);
                DRV_IOCTL(lchip, i, cmd, &profId);

                cmd = DRV_IOR(DsIrmPortLimitedThrdProfile_t, DsIrmPortLimitedThrdProfile_g_0_portLimitedThrd_f);
                DRV_IOCTL(lchip, (((profId&0xf) << 2) + congest_level), cmd, &port_thrd);
                CTC_DKITS_PRINT_FILE(p_wf, "0x%04x    %-10d%-10d%-10d\n",
                               CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port),
                               i, depth, (port_thrd<<3));
                b_all_zero = FALSE;
            }

        }
        if (b_all_zero)
        {
            CTC_DKITS_PRINT_FILE(p_wf, "All port depth is zero!!!\n");
        }
        CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
    }
    else
    {
        channel = _ctc_usw_dkit_monitor_get_channel_by_gport(lchip, gport);
        if (channel != 0xFFFFFFFF)
        {
            cmd = DRV_IOR(DsIrmPortCnt_t, DsIrmPortCnt_portCnt_f);
            index = channel;
            DRV_IOCTL(lchip, index, cmd, &depth);

            cmd = DRV_IOR(DsIrmPortCfg_t, DsIrmPortCfg_portLimitedThrdProfId_f);
            DRV_IOCTL(lchip, index, cmd, &profId);

            cmd = DRV_IOR(DsIrmPortLimitedThrdProfile_t, DsIrmPortLimitedThrdProfile_g_0_portLimitedThrd_f);
            DRV_IOCTL(lchip, (((profId&0xf) << 2) + congest_level), cmd, &port_thrd);

            CTC_DKITS_PRINT_FILE(p_wf, "\n--------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_wf, "%-10s%-10s%-10s%-10s\n", "Port", "Channel", "Depth", "Thrd");
            CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_wf, "0x%04x    %-10d%-10d%-10d\n", gport, channel, depth, (port_thrd<<3));

            _ctc_usw_dkit_monitor_q_depth_igr_detail_port_tc(lchip, channel, p_wf);

        }
        else
        {
            CTC_DKITS_PRINT_FILE(p_wf, "Invalid Gport:0x%x!!!\n", gport);
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_igr(uint8 lchip, uint16 gport, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint32 resrc_mgr_en = 0;
    cmd = DRV_IOR(IrmMiscCtl_t, IrmMiscCtl_resourceCheckEn_f);
    DRV_IOCTL(lchip, 0, cmd, &resrc_mgr_en);
    if (!resrc_mgr_en)
    {
        CTC_DKITS_PRINT_FILE(p_wf, "Ingress Resouce Mamager is Disable!!!\n");
        return CLI_SUCCESS;
    }

    if (0xFFFF == gport)
    {
        _ctc_usw_dkit_monitor_q_depth_igr_summary(lchip, p_wf);
    }
    _ctc_usw_dkit_monitor_q_depth_igr_detail(lchip, gport, p_wf);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_egr_summary(uint8 lchip, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint8 i = 0, j = 0;
    uint32 value = 0;
    uint8 congest_level = 0;
    char desc[16] = {0};
    uint32 totalCnt = 0;
    uint32 egr_total_thrd = 0;
    uint32 c2c_packet_cnt = 0;
    uint32 critical_packet_cnt = 0;
    uint32 c2c_packet_thrd = 0;
    uint32 critical_packet_thrd = 0;
    DsErmMiscCnt_m erm_misc_cnt;
    DsErmMiscThrd_m erm_misc_thrd;

    sal_memset(&erm_misc_cnt, 0, sizeof(DsErmMiscCnt_m));
    cmd = DRV_IOR(DsErmMiscCnt_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &erm_misc_cnt);
    GetDsErmMiscCnt(A, c2cCnt_f, &erm_misc_cnt, &c2c_packet_cnt);
    GetDsErmMiscCnt(A, criticalCnt_f, &erm_misc_cnt, &critical_packet_cnt);
    GetDsErmMiscCnt(A, totalCnt_f, &erm_misc_cnt, &totalCnt);

    sal_memset(&erm_misc_thrd, 0, sizeof(DsErmMiscThrd_m));
    cmd = DRV_IOR(DsErmMiscThrd_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &erm_misc_thrd);
    GetDsErmMiscThrd(A, c2cThrd_f, &erm_misc_thrd, &c2c_packet_thrd);
    GetDsErmMiscThrd(A, criticalThrd_f, &erm_misc_thrd, &critical_packet_thrd);
    GetDsErmMiscThrd(A, totalThrd_f, &erm_misc_thrd, &egr_total_thrd);

    for (j = 0; j <= 3; j++)
    {
        congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, FALSE, j);
        CTC_DKITS_PRINT_FILE(p_wf, "\n-------SC%d congest config---------\n", j);
        CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10s\n", "level", "Thrd");
        CTC_DKITS_PRINT_FILE(p_wf, "-----------------------------------\n");
        for (i = 0; i < CTC_DKIT_MONITOR_CONGEST_LEVEL_NUM; i++)
        {
            cmd = DRV_IOR(DsErmScCngThrdProfile_t, DsErmScCngThrdProfile_g_0_scCngThrd_f + i);
            DRV_IOCTL(lchip, j, cmd, &value);
            if (i == congest_level)
            {
                sal_sprintf(desc, "%d%s", i, " (*)");
            }
            else
            {
                sal_sprintf(desc, "%d", i);
            }
            CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d\n", desc, value << 5);
        }
    }

    CTC_DKITS_PRINT_FILE(p_wf, "\n-----------------------------------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10s %-10s\n", "Type", "Depth", "Drop-Thrd");
    CTC_DKITS_PRINT_FILE(p_wf, "-----------------------------------\n");
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Total", totalCnt, egr_total_thrd);

    if ((totalCnt >= (critical_packet_cnt + c2c_packet_cnt))
        && (egr_total_thrd >= (critical_packet_thrd + c2c_packet_thrd)))
    {
        CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Normal",
                       (totalCnt - critical_packet_cnt - c2c_packet_cnt),
                       egr_total_thrd - critical_packet_thrd - c2c_packet_thrd);
    }

    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "Critical", critical_packet_cnt, critical_packet_thrd);
    CTC_DKITS_PRINT_FILE(p_wf, "%-14s %-10d %-10d\n", "C2C", c2c_packet_cnt, c2c_packet_thrd);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_egr_detail_queue(uint8 lchip, uint16 gport, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint32 channel = 0;
    uint32 chan_id = 0;
    uint32 depth = 0;
    uint32 i = 0;
    uint32 sc = 0;
    uint32 q_id = 0;
    uint32 profId = 0;
    uint8  congest_level = 0;
    uint32 queue_thrd = 0;
    uint32 field_val = 0;

    channel = _ctc_usw_dkit_monitor_get_channel_by_gport(lchip, gport);
    for (i = 0; i < CTC_DKIT_MONITOR_MAX_QUEUE_NUM; i++)
    {
        chan_id = _ctc_usw_dkit_monitor_get_channel_by_queue_id(lchip, i);
        if (chan_id == channel)
        {
            CTC_DKITS_PRINT_FILE(p_wf, "\n-----------------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_wf, "%-10s%-10s%-10s\n", "Q-Id", "Depth", "Queue-Drop-Thrd");
            CTC_DKITS_PRINT_FILE(p_wf, "------------------------------------------------\n");

            q_id = i;
            cmd = DRV_IOR(DsErmQueueCnt_t, DsErmQueueCnt_queueCnt_f);
            DRV_IOCTL(lchip, q_id, cmd, &depth);

            cmd = DRV_IOR(DsErmQueueCfg_t, DsErmQueueCfg_queueLimitedThrdProfId_f);
            DRV_IOCTL(lchip, q_id, cmd, &profId);

            cmd = DRV_IOR(DsErmChannel_t, DsErmChannel_ermProfId_f);
            DRV_IOCTL(lchip, chan_id, cmd, &field_val);

            cmd = DRV_IOR(DsErmPrioScTcMap_t, DsErmPrioScTcMap_g1_0_mappedSc_f + field_val);
            DRV_IOCTL(lchip, i % 16, cmd, &sc);
            congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, FALSE, sc);
            cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DsErmQueueLimitedThrdProfile_g_0_queueLimitedThrd_f);
            DRV_IOCTL(lchip, (((profId&0xF) << 2) + congest_level), cmd, &queue_thrd);
            CTC_DKITS_PRINT_FILE(p_wf, "%-10d%-10d%-10d\n", q_id, depth, (queue_thrd << 3));
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_egr_detail(uint8 lchip, uint16 gport, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint32 channel = 0;
    uint32 depth = 0;
    uint32 local_phy_port = 0;
    uint32 gchip = 0;
    uint32 i = 0;
    uint8 congest_level = 0;
    uint32 port_thrd = 0;
    uint32 profId = 0;
    bool b_all_zero = TRUE;

    congest_level = _ctc_usw_dkit_monitor_get_congest_level(lchip, FALSE, 0);
    if (0xFFFF == gport)
    {
        CTC_DKITS_PRINT_FILE(p_wf, "\n--------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_wf, "%-12s%-12s%-12s%-12s\n",
                       "Port", "Channel", "Depth", "Thrd");
        CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
        for (i = 0; i < CTC_DKIT_ONE_SLICE_CHANNEL_NUM; i++)
        {
            cmd = DRV_IOR(DsErmPortCnt_t, DsErmPortCnt_portCnt_f);
            DRV_IOCTL(lchip, i , cmd, &depth);
            if (depth)
            {
                cmd = DRV_IOR(EpeHeaderAdjustPhyPortMap_t, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
                DRV_IOCTL(lchip, i, cmd, &local_phy_port);
                CTC_DKIT_GET_GCHIP(lchip, gchip);

                cmd = DRV_IOR(DsErmPortCfg_t, DsErmPortCfg_portLimitedThrdProfId_f);
                DRV_IOCTL(lchip, i, cmd, &profId);

                cmd = DRV_IOR(DsErmPortLimitedThrdProfile_t, DsErmPortLimitedThrdProfile_g_0_portLimitedThrd_f);
                DRV_IOCTL(lchip, (((profId&0xf) << 2) + congest_level), cmd, &port_thrd);
                CTC_DKITS_PRINT_FILE(p_wf, "0x%04x    %-12d%-12d%-12d\n",
                               CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, local_phy_port),
                               i, depth, (port_thrd<<3));
                b_all_zero = FALSE;
            }
        }
        if (b_all_zero)
        {
            CTC_DKITS_PRINT_FILE(p_wf, "All port depth is zero!!!\n");
        }
        CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
    }
    else
    {
        channel = _ctc_usw_dkit_monitor_get_channel_by_gport(lchip, gport);
        if (channel != 0xFFFFFFFF)
        {
            cmd = DRV_IOR(DsErmPortCnt_t, DsErmPortCnt_portCnt_f);
            DRV_IOCTL(lchip, channel, cmd, &depth);

            cmd = DRV_IOR(DsErmPortCfg_t, DsErmPortCfg_portLimitedThrdProfId_f);
            DRV_IOCTL(lchip, channel, cmd, &profId);

            cmd = DRV_IOR(DsErmPortLimitedThrdProfile_t, DsErmPortLimitedThrdProfile_g_0_portLimitedThrd_f);
            DRV_IOCTL(lchip, (((profId&0xf) << 2) + congest_level), cmd, &port_thrd);

            CTC_DKITS_PRINT_FILE(p_wf, "\n--------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_wf, "%-12s%-12s%-12s%-12s\n", "Port", "Channel", "Depth", "Thrd");
            CTC_DKITS_PRINT_FILE(p_wf, "--------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_wf, "0x%04x      %-12d%-12d%-12d\n", gport, channel, depth, (port_thrd<<3));

            _ctc_usw_dkit_monitor_q_depth_egr_detail_queue(lchip, channel, p_wf);
        }
        else
        {
            CTC_DKITS_PRINT_FILE(p_wf, "Invalid Gport:0x%x!!!\n", gport);
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_monitor_q_depth_egr(uint8 lchip, uint16 gport, sal_file_t p_wf)
{
    uint32 cmd = 0;
    uint32 egr_resrc_mgr_en = 0;

    cmd = DRV_IOR(ErmMiscCtl_t, ErmMiscCtl_resourceCheckEn_f);
    DRV_IOCTL(lchip, 0, cmd, &egr_resrc_mgr_en);
    if (0 == egr_resrc_mgr_en)
    {
        CTC_DKITS_PRINT_FILE(p_wf, "Egress Resouce Mamager is Disable!!!\n");
        return CLI_SUCCESS;
    }

    if (0xFFFF == gport)
    {
        _ctc_usw_dkit_monitor_q_depth_egr_summary(lchip, p_wf);
    }
    _ctc_usw_dkit_monitor_q_depth_egr_detail(lchip, gport, p_wf);

    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_monitor_show_queue_depth(void* p_para)
{
    uint32 entry_num = 0;
    uint32 cmd = 0;
    uint32 idx = 0;
    uint32 value = 0;
    bool b_all_zero = TRUE;
    uint8 lchip = 0;
    ctc_dkit_monitor_para_t* p_monitor_para = (ctc_dkit_monitor_para_t*)p_para;
    sal_file_t p_wf = p_monitor_para->p_wf;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_monitor_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);

    if (CTC_DKIT_INGRESS == p_monitor_para->dir)
    {
        _ctc_usw_dkit_monitor_q_depth_igr(lchip, p_monitor_para->gport, p_wf);
    }
    else if(CTC_DKIT_EGRESS == p_monitor_para->dir)
    {
        _ctc_usw_dkit_monitor_q_depth_egr(lchip, p_monitor_para->gport, p_wf);
    }
    else
    {
        CTC_DKITS_PRINT_FILE(p_wf, "%-20s %-10s\n", "QueueID", "Depth(unit:288B)");
        CTC_DKITS_PRINT_FILE(p_wf, "---------------------------------------------\n");

        entry_num = TABLE_MAX_INDEX(lchip, DsErmQueueCnt_t);
        cmd = DRV_IOR(DsErmQueueCnt_t, DsErmQueueCnt_queueCnt_f);
        for (idx = 0; idx < entry_num; idx++)
        {

            DRV_IOCTL(lchip, idx, cmd, &value);
            if (0 != value)
            {
                CTC_DKITS_PRINT_FILE(p_wf, "%-20d %-10d\n", idx, value);
                b_all_zero = FALSE;
            }
        }
        if (b_all_zero)
        {
            CTC_DKITS_PRINT_FILE(p_wf, "All Queue depth is zero!!!\n");
        }
        CTC_DKITS_PRINT_FILE(p_wf, "---------------------------------------------\n");
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_monitor_show_sensor_result(void* p_para)
{
    int32 ret = CLI_SUCCESS;
    ctc_dkit_monitor_para_t* p_monitor_para = (ctc_dkit_monitor_para_t*)p_para;
    DKITS_PTR_VALID_CHECK(p_monitor_para);
    CTC_DKIT_LCHIP_CHECK(p_monitor_para->lchip);
    DRV_INIT_CHECK(p_monitor_para->lchip);

    if(g_dkit_chip_api[p_monitor_para->lchip]->dkits_sensor_result)
    {
        ret = g_dkit_chip_api[p_monitor_para->lchip]->dkits_sensor_result(p_para);
    }

    return ret;
}

int32
ctc_usw_dkit_monitor_sensor_register(uint8 lchip)
{
#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_sensor_result = ctc_dt2_dkit_monitor_show_sensor_result;
    }
#endif
#ifdef TSINGMA
    if (DRV_IS_TSINGMA(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_sensor_result = ctc_tm_dkit_monitor_show_sensor_result;
    }
#endif
    return 0;

}



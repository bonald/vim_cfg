/**
 @file sys_duet2_datapath.c

 @date 2018-09-12

 @version v1.0
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_port.h"
#include "sys_usw_chip.h"
#include "sys_usw_datapath.h"
#include "sys_usw_mac.h"
#include "sys_usw_peri.h"
#include "sys_usw_dmps.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_mcu.h"
#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"
#include "sys_duet2_datapath.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SUP_RESET_CTL_ADDR              0x00011070
#define RLM_HS_CTL_EN_CLK_ADDR          0x001200a0
#define RLM_CS_CTL_EN_CLK_ADDR          0x00140050
#define RLM_MAC_CTL_EN_CLK_ADDR        0x00e00020
#define RLM_BSR_CTL_EN_CLK_ADDR        0x00a0002c
#define RLM_HS_CTL_RESET_ADDR            0x00120040
#define RLM_CS_CTL_RESET_ADDR            0x00140020
#define RLM_MAC_CTL_RESET_ADDR         0x00e00000
#define RLM_NET_CTL_RESET_ADDR          0x00400020
#define RLM_IPERX_CTL_RESET_ADDR      0x00200020
#define RLM_IPETX_CTL_RESET_ADDR      0x00800020
#define RLM_EPERX_CTL_RESET_ADDR      0x00300020
#define RLM_EPETX_CTL_RESET_ADDR      0x00c00020
#define RLM_BSR_CTL_RESET_ADDR          0x00a00028
#define RLM_QMGR_CTL_RESET_ADDR       0x00600020
#define RLM_KEY_CTL_RESET_ADDR          0x04000020
#define RLM_TCAM_CTL_RESET_ADDR        0x01000020
#define RLM_AD_CTL_RESET_ADDR           0x02000020
#define RLM_SECURITY_CTL_RESET_ADDR      0x00160020
#define RLM_CAPWAP_CTL_RESET_ADDR        0x00180020
#define SUP_INTR_FUNC_ADDR                 0x00011080

#define SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% SERDES_NUM_PER_SLICE) - 24)/HSS28G_LANE_NUM + (((serdes_id)/SERDES_NUM_PER_SLICE)?3:3)) \
    :(((serdes_id)% SERDES_NUM_PER_SLICE)/HSS15G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*7)

#define SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?(((serdes_id% SERDES_NUM_PER_SLICE) - 24)%HSS28G_LANE_NUM) \
    :((serdes_id% SERDES_NUM_PER_SLICE)%HSS15G_LANE_NUM)
    
#define SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% SERDES_NUM_PER_SLICE) - 24)/HSS28G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*HSS28G_NUM_PER_SLICE) \
    :(((serdes_id)% SERDES_NUM_PER_SLICE)/HSS15G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*HSS15G_NUM_PER_SLICE)


/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
extern sys_datapath_master_t* p_usw_datapath_master[CTC_MAX_LOCAL_CHIP_NUM];
extern uint8 g_usw_mac_port_map[64];
extern uint8 g_usw_hss15g_lane_map[HSS15G_LANE_NUM];
extern uint8 g_usw_hss_tx_addr_map[HSS15G_LANE_NUM];

extern int32
sys_usw_datapath_build_serdes_info(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 group, uint8 is_dyn, uint16 overclocking_speed);

extern int32
sys_usw_datapath_set_hss_internal(uint8 lchip, uint8 serdes_id, uint8 mode);

extern int32
sys_usw_datapath_hss28g_cfg_clktree(uint8 lchip, uint8 hss_idx, uint8 pll_type);

extern int32
_sys_usw_datapath_hss28g_cfg_lane_clktree(uint8 lchip, uint8 idx, sys_datapath_hss_attribute_t* p_hss);

extern int32
sys_usw_datapath_hss15g_cfg_clktree(uint8 lchip, uint8 hss_idx, uint8 pll_type);

extern int32
_sys_usw_datapath_hss15g_cfg_lane_clktree(uint8 lchip, uint8 idx, sys_datapath_hss_attribute_t* p_hss);

extern int32
sys_usw_datapath_set_misc_cfg(uint8 lchip);

extern int32
sys_usw_datapath_datapth_init(uint8 lchip);

extern int32
sys_usw_datapath_set_impedance_calibration(uint8 lchip);

extern int32
sys_usw_datapath_wb_restore(uint8 lchip);

extern int16
sys_usw_datapath_hss15g_serdes_aec_aet(uint8 lchip, uint8 hss_id, uint8 lane_id, sys_datapath_hss_attribute_t* p_hss_vec, bool enable, uint16 serdes_id);

extern int16
sys_usw_datapath_hss28g_serdes_aec_aet(uint8 lchip, uint8 hss_id, uint8 lane_id, sys_datapath_hss_attribute_t* p_hss_vec, bool enable, uint16 serdes_id);
/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_duet2_datapath_sup_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 index = 0;
    uint8  step  = 0;
    SupDskCfg_m sup_dsk;
    SupMiscCfg_m sup_misc;

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }

    value = 0;
    /*release DSK reset*/
    cmd = DRV_IOR(SupDskCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgClkResetClkOobFc_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgClkResetClkMiscA_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgClkResetClkMiscB_f, &value, &sup_dsk);

    cmd = DRV_IOW(SupDskCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_dsk);

    /*release Tod/led/mdio reset*/
    cmd = DRV_IOR(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_cfgResetClkLedDiv_f, &value, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioDiv_f, &value, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioXgDiv_f, &value, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_cfgResetClkTodDiv_f, &value, &sup_misc);
    cmd = DRV_IOW(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);

    /*release sup reset core */
    cmd = DRV_IOR(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_softResetCore_f, &value, &sup_misc);
    cmd = DRV_IOW(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);

    /*do SupResetCtl reset*/
    if (0 == sys_usw_chip_get_reset_hw_en(lchip))
    {
        drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0xffffffff);
    }
    else
    {
        drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0xfffcffff);

    }
    sal_task_sleep(1);
    drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0x00000000);

    value = 1;
    cmd = DRV_IOW(SupRegCtl_t, SupRegCtl_regReqCrossEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    /*init cfgTsUseIntRefClk */
    value = 1;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgTsUseIntRefClk_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 0;
    cmd = DRV_IOW(RefDivCoppUpdPulse_t, RefDivCoppUpdPulse_cfgResetDivCoppUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivDlbPulse_t, RefDivDlbPulse_cfgResetDivDlbPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivEfdPulse_t, RefDivEfdPulse_cfgResetDivEfdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivLinkAggDrePulse_t, RefDivLinkAggDrePulse_cfgResetDivLinkAggDrePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivMfpUpdPulse_t, RefDivMfpUpdPulse_cfgResetDivMfpUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivNetTxEeePulse_t, RefDivNetTxEeePulse_cfgResetDivNetTxEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivOamTokenUpd_t, RefDivOamTokenUpd_cfgResetDivOamTokenUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsEeePulse_t, RefDivPcsEeePulse_cfgResetDivCsEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsEeePulse_t, RefDivPcsEeePulse_cfgResetDivHsEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgResetDivCsLinkFilterPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgResetDivHsLinkFilterPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgResetDivCsLinkPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgResetDivHsLinkPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivEpePolicing1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivEpePolicing0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivIpePolicing1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivIpePolicing0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    cmd = DRV_IOW(RefDivSgmacPauseLockPulse_t, RefDivSgmacPauseLockPulse_cfgResetDivSgmacPauseLockPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer3Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer2Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer1Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer0Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivStormCtlUpdPulse_t, RefDivStormCtlUpdPulse_cfgResetDivStormCtl1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivStormCtlUpdPulse_t, RefDivStormCtlUpdPulse_cfgResetDivStormCtl0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    /*do RlmBsrCtlEnClk reset*/
    drv_usw_chip_write(lchip, RLM_BSR_CTL_EN_CLK_ADDR, 0xffffffff);

    /* move from sup_init */
    /*enable RlmHsCtlEnClk */
    drv_usw_chip_write(lchip, RLM_HS_CTL_EN_CLK_ADDR, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_HS_CTL_EN_CLK_ADDR+4, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_HS_CTL_EN_CLK_ADDR+8, 0xffffffff);

    /*enable RlmCsCtlEnClk */
    drv_usw_chip_write(lchip, RLM_CS_CTL_EN_CLK_ADDR, 0xffffffff);

    /*enable RlmMacCtlEnClk */
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR+4, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR+8, 0xffffffff);


    /* move from sup_init */
    /*release RlmHsCtlReset*/
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR+4, 0);
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR+8, 0);
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR+0x0c, 0);
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR+0x10, 0);
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR+0x14, 0);

    /*release RlmCsCtlReset*/
    drv_usw_chip_write(lchip, RLM_CS_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_CS_CTL_RESET_ADDR+4, 0);
    drv_usw_chip_write(lchip, RLM_CS_CTL_RESET_ADDR+8, 0);

    /*
     *  #1, cfg RlmMacCtlReset.resetCoreTxqm0..3 = 0
     *  #2, cfg RlmMacCtlReset.resetCoreQuadSgmac0..15Reg = 0
     *  #3, cfg RlmMacCtlReset.resetCoreQuadSgmac0..15 = 0
     * the 3 step must be done in sys_usw_datapath_mac_init();
     * other field in RlmMacCtlReset won't be released unless mac module used, mac-en/dis, an/fec, in sys_usw_mac.c
     */
    for (index = 0; index < 16; index++)
    {
        value = 0;

        step = RlmMacCtlReset_resetCoreQuadSgmac1Reg_f - RlmMacCtlReset_resetCoreQuadSgmac0Reg_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmac0Reg_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);

        step = RlmMacCtlReset_resetCoreQuadSgmac1_f - RlmMacCtlReset_resetCoreQuadSgmac0_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmac0_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }
    value = 0;
    for (index = 0; index < 4; index++)
    {
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreTxqm0_f+index);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    /*release RlmNetCtlReset*/
    drv_usw_chip_write(lchip, RLM_NET_CTL_RESET_ADDR, 0);

    /*release RlmIpeCtlReset */
    drv_usw_chip_write(lchip, RLM_IPERX_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_IPETX_CTL_RESET_ADDR, 0);

    /*do RlmBsrCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_BSR_CTL_RESET_ADDR, 0);

    /*release RlmEpeCtlReset */
    drv_usw_chip_write(lchip, RLM_EPERX_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_EPETX_CTL_RESET_ADDR, 0);

    /*do RlmQMgrCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_QMGR_CTL_RESET_ADDR, 0);

    /*do RlmKeyCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_KEY_CTL_RESET_ADDR, 0);

    /*do RlmTcamCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_TCAM_CTL_RESET_ADDR, 0);

    /*release RlmAdCtlReset */
    drv_usw_chip_write(lchip, RLM_AD_CTL_RESET_ADDR, 0);

    /*do RlmSecurityCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_SECURITY_CTL_RESET_ADDR, 0);

    /*do RlmCapwapCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_CAPWAP_CTL_RESET_ADDR, 0);

    /*do SupIntrFunc reset*/
    drv_usw_chip_write(lchip, SUP_INTR_FUNC_ADDR+4, 0xffffffff);

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_datapath_core_clock_init(uint8 lchip, uint16 core_a, uint16 core_b)
{
    PllCoreCfg_m core_cfg;
    PllCoreMon_m core_mon;
    uint32 cmd = 0;
    uint32 value = 0;

    sal_memset(&core_cfg, 0, sizeof(PllCoreCfg_m));
    sal_memset(&core_mon, 0, sizeof(PllCoreMon_m));

    /*reset core pll*/
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_cfg);
    SetPllCoreCfg(V, cfgPllCoreReset_f, &core_cfg, 1);
    value = 1;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreReset_f, &value, &core_cfg);
    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_cfg);

    if ((core_a == 600) && (core_b == 500))
    {
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMult124_f, &value, &core_cfg);
        value = 60;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRange124_f, &value, &core_cfg);
        value = 29;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeA_f, &value, &core_cfg);
        value = 28;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeB_f, &value, &core_cfg);
    }
    else if ((core_a == 550) && (core_b == 550))
    {
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMult124_f, &value, &core_cfg);
        value = 44;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 3;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRange124_f, &value, &core_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeA_f, &value, &core_cfg);
    }
    else if ((core_a == 500) && (core_b == 500))
    {
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMult124_f, &value, &core_cfg);
        value = 60;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRange124_f, &value, &core_cfg);
        value = 28;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeA_f, &value, &core_cfg);
    }
    else if ((core_a == 450) && (core_b == 450))
    {
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMult124_f, &value, &core_cfg);
        value = 54;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRange124_f, &value, &core_cfg);
        value = 28;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeA_f, &value, &core_cfg);
    }
    else if ((core_a == 400) && (core_b == 400))
    {
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMult124_f, &value, &core_cfg);
        value = 48;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRange124_f, &value, &core_cfg);
        value = 28;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreRangeA_f, &value, &core_cfg);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_cfg);

    sal_task_sleep(2);

    /*reset core pll*/
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_cfg);
    SetPllCoreCfg(V, cfgPllCoreReset_f, &core_cfg, 0);
    value = 0;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreReset_f, &value, &core_cfg);
    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_cfg);

    sal_task_sleep(1);

    /*check pll lock*/
    cmd = DRV_IOR(PllCoreMon_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &core_mon);

    if (0 == SDK_WORK_PLATFORM)
    {
        if (!GetPllCoreMon(V,monPllCoreLock_f, &core_mon))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] pll cannot lock \n");
            return CTC_E_HW_NOT_LOCK;

        }
    }

    return CTC_E_NONE;
}

/*
   this interface is user to init hss15g, notice dynamic can not using this interface to config hss,
   because here will do hss core reset, will affect other serdes.
*/
STATIC int32
_sys_duet2_datapath_hss15g_init(uint8 lchip, uint8 hss_idx)
{
    uint32 tb_id = 0;
    uint32 cmd = 0;
    uint32 fld_id = 0;
    uint32 value = 0;
    HsCfg0_m hs_cfg;
    HsMacroMiscMon0_m hs_mon;
    uint32 mult = 0;
    uint32 timeout = 0x4000;
    uint8 plla_ready = 0;
    uint8 pllb_ready = 0;
    uint8 plla_used = 0;
    uint8 pllb_used = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 hss_id = 0;
    RlmHsCtlReset_m hsctl_reset;

    sal_memset(&hs_cfg, 0, sizeof(HsCfg0_m));
    sal_memset(&hs_mon, 0, sizeof(HsMacroMiscMon0_m));

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss %d not used\n", hss_idx);
        return CTC_E_NONE;
    }

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_15G)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss parameter error, not match, hss_idx:%d \n", hss_idx);
        return CTC_E_INVALID_PARAM;
    }

    hss_id = p_hss->hss_id;

    /*1. release sup reset for hss15g, Notice hss_id is 0~2 for hss15g, not hss_idx */
    tb_id = RlmHsCtlReset_t;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hsctl_reset));
    fld_id = RlmHsCtlReset_resetCoreHss15GUnitWrapper0_f+hss_id%3;
    value = 0;
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &hsctl_reset);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hsctl_reset));

    /*2. set hss15g core reset */
    tb_id = HsCfg0_t + hss_id;
    value = 1;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
    DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssReset_f, &value, &hs_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

    plla_used = (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)?1:0;
    pllb_used = (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)?1:0;

    /*3. set hss15g pin input */
    if (plla_used)
    {

        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssPdwnPllA_f, &value, &hs_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidA_f, &value, &hs_cfg);
        SYS_DATAPATH_GET_HSS15G_MULT(p_hss->plla_mode, mult);
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssDivSelA_f, &mult, &hs_cfg);

        if ((p_hss->plla_mode == SYS_DATAPATH_HSSCLK_515DOT625) || (p_hss->plla_mode == SYS_DATAPATH_HSSCLK_500))
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelA_f, &value, &hs_cfg);
        }
        else
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelA_f, &value, &hs_cfg);
        }
    }
    else
    {
        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidA_f, &value, &hs_cfg);
    }

    if (pllb_used)
    {

        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssPdwnPllB_f, &value, &hs_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidB_f, &value, &hs_cfg);
        SYS_DATAPATH_GET_HSS15G_MULT(p_hss->pllb_mode, mult);
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssDivSelB_f, &mult, &hs_cfg);
        if (p_hss->pllb_mode == SYS_DATAPATH_HSSCLK_644DOT53125)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelB_f, &value, &hs_cfg);
        }
        else
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelB_f, &value, &hs_cfg);
        }
    }
    else
    {
        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidB_f, &value, &hs_cfg);
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

    sal_task_sleep(1);

    /*4. release hss15g core reset */
    tb_id = HsCfg0_t + hss_id;
    value = 0;
    DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssReset_f, &value, &hs_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

    sal_task_sleep(1);

    /*5. wait hssprtready assert*/
    tb_id = HsMacroMiscMon0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_mon);

    if (0 == SDK_WORK_PLATFORM)
    {
        while(--timeout)
        {
            DRV_IOCTL(lchip, 0, cmd, &hs_mon);
            if (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            {
                if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyA_f, &hs_mon))
                {
                    plla_ready = 1;
                }
            }

            if (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            {
                if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyB_f, &hs_mon))
                {
                    pllb_ready = 1;
                }
            }

            if (((plla_used && plla_ready) || (!plla_used)) &&
                ((pllb_used && pllb_ready) || (!pllb_used)))
            {
                break;
            }
        }

        /*check ready*/
        if ((plla_used && !plla_ready) || (pllb_used && !pllb_ready))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HssIdx %d \n", hss_idx);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;
        }
    }

    return CTC_E_NONE;
}

/*
   this interface is user to init hss28g, notice dynamic can not using this interface to config hss,
   because here will do hss core reset, will affect other serdes.
*/
STATIC int32
_sys_duet2_datapath_hss28g_init(uint8 lchip, uint8 hss_idx)
{
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 hss_id = 0;
    uint32 tb_id = 0;
    uint32 cmd = 0;
    uint32 fld_id = 0;
    uint32 value = 0;
    CsCfg0_m cs_cfg;
    CsMacroMiscMon0_m cs_mon;
    uint32 mult = 0;
    uint32 timeout = 0x4000;
    uint8 plla_ready = 0;
    uint8 pllb_ready = 0;
    uint8 plla_used = 0;
    uint8 pllb_used = 0;
    RlmCsCtlReset_m csctl_reset;

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_28G)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss parameter error, not match, hss_idx:%d \n", hss_idx);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cs_cfg, 0, sizeof(CsCfg0_m));
    sal_memset(&cs_mon, 0, sizeof(CsMacroMiscMon0_m));

    hss_id = p_hss->hss_id;

    /*1. reset sup config to make hss28g marco can access*/
    tb_id = RlmCsCtlReset_t;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &csctl_reset));
    fld_id = RlmCsCtlReset_resetCoreHss28GUnitWrapper0_f + hss_id%HSS28G_NUM_PER_SLICE;
    value = 0;
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &csctl_reset);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &csctl_reset));

    /*2. set hss28g core reset */
    tb_id = CsCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssReset_f, &value, &cs_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

    /*3. set hss28g pin input */
    plla_used = (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)?1:0;
    pllb_used = (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)?1:0;

    if (plla_used)
    {
        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPdwnPllA_f, &value, &cs_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidA_f, &value, &cs_cfg);

        /*plla just using VCO:20G,20.625G*/
        value = 1;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelA_f, &value, &cs_cfg);

        if (p_hss->plla_mode == SYS_DATAPATH_HSSCLK_500)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2A_f, &value, &cs_cfg);
        }
        else
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2A_f, &value, &cs_cfg);
        }

        SYS_DATAPATH_GET_HSS28G_MULT(p_hss->plla_mode, mult);
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssDivSelA_f, &mult, &cs_cfg);
    }
    else
    {
        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidA_f, &value, &cs_cfg);
    }

    if (pllb_used)
    {

        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPdwnPllB_f, &value, &cs_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidB_f, &value, &cs_cfg);

        if (p_hss->pllb_mode == SYS_DATAPATH_HSSCLK_625)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
        }
        else if (p_hss->pllb_mode == SYS_DATAPATH_HSSCLK_500)
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
        }
        else if (p_hss->pllb_mode == SYS_DATAPATH_HSSCLK_515DOT625)
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
        }
        else
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
        }

        SYS_DATAPATH_GET_HSS28G_MULT(p_hss->pllb_mode, mult);
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssDivSelB_f, &mult, &cs_cfg);
    }
    else
    {
        value = 0;
        DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidB_f, &value, &cs_cfg);
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

    sal_task_sleep(1);

    /*4. release hss28g core reset */
    tb_id = CsCfg0_t + hss_id;
    value = 0;
    DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssReset_f, &value, &cs_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

    sal_task_sleep(1);

    /*5. wait hssprtready ready assert */
    tb_id = CsMacroMiscMon0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_mon);
    if (0 == SDK_WORK_PLATFORM)
    {
        while(--timeout)
        {
            DRV_IOCTL(lchip, 0, cmd, &cs_mon);
            if (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            {
                if (GetCsMacroMiscMon0(V, monHss28gHssPrtReadyA_f, &cs_mon))
                {
                    plla_ready = 1;
                }
            }

            if (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            {
                if (GetCsMacroMiscMon0(V, monHss28gHssPrtReadyB_f, &cs_mon))
                {
                    pllb_ready = 1;
                }
            }

            if (((plla_used && plla_ready) || (!plla_used)) &&
                ((pllb_used && pllb_ready) || (!pllb_used)))
            {
                break;
            }
        }

        /*check ready*/
        if ((plla_used && !plla_ready) || (pllb_used && !pllb_ready))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HssIdx %d \n", hss_idx);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_datapath_mcu_mem_init(uint8 lchip)
{
    uint32 i = 0;

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }

    for (i = 0; i < SYS_USW_DATA_MEM_ENTRY_NUM; i++)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (SYS_USW_DATA_MEM_BASE_ADDR0 + i*4), 0));
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (SYS_USW_DATA_MEM_BASE_ADDR1 + i*4), 0));
    }

    for (i = 0; i < SYS_USW_PROG_MEM_ENTRY_NUM; i++)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (SYS_USW_PROG_MEM_BASE_ADDR0 + i*4), 0));
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (SYS_USW_PROG_MEM_BASE_ADDR1 + i*4), 0));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_datapath_check_serdes_mode(uint8 lchip)
{
    uint8 i = 0, j = 0, flag_25g_50g = 0, flag_qu = 0;
    uint8 serdes_id = 0;
    uint8 tmp_serdes = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 arr_serdes_mode[MAX_SERDES_NUM_PER_SLICE] = {0};
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    sal_memset(arr_serdes_mode, 0, MAX_SERDES_NUM_PER_SLICE * sizeof(uint8));

    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }

        p_serdes = &p_hss_vec->serdes_info[lane_id];
        arr_serdes_mode[serdes_id] = p_serdes->mode;
    }

    for (i = 0; i < HSS15G_NUM_PER_SLICE*2; i++)
    {
        flag_qu = 0;
        for (j = 0; j < 4; j++)
        {
            tmp_serdes = (j + 4 * i);
            if ((CTC_CHIP_SERDES_QSGMII_MODE == arr_serdes_mode[tmp_serdes])
                || (CTC_CHIP_SERDES_USXGMII0_MODE == arr_serdes_mode[tmp_serdes])
                || (CTC_CHIP_SERDES_USXGMII1_MODE == arr_serdes_mode[tmp_serdes])
                || (CTC_CHIP_SERDES_USXGMII2_MODE == arr_serdes_mode[tmp_serdes]))
            {
                flag_qu = arr_serdes_mode[tmp_serdes];
            }
        }
        for (j = 0; j < 4; j++)
        {
            tmp_serdes = j+4*i;
            if (flag_qu && (flag_qu != arr_serdes_mode[tmp_serdes])
                && (CTC_CHIP_SERDES_NONE_MODE != arr_serdes_mode[tmp_serdes]))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " Serdes mode %d and %d CANNOT exist on every 4 lanes simultaneously\n",
                    flag_qu, arr_serdes_mode[tmp_serdes]);
                return CTC_E_INVALID_CONFIG;
            }
        }
    }

    for (i = 0; i < HSS28G_NUM_PER_SLICE; i++)
    {
        flag_25g_50g = 0;
        for (j = 0; j < 4; j++)
        {
            tmp_serdes = (j + 4 * i);
            tmp_serdes += HSS15G_NUM_PER_SLICE*HSS15G_LANE_NUM;
            if (CTC_CHIP_SERDES_XXVG_MODE == arr_serdes_mode[tmp_serdes])
            {
                flag_25g_50g = 25;   /* 25G exist */
            }
            else if (CTC_CHIP_SERDES_LG_MODE == arr_serdes_mode[tmp_serdes])
            {
                flag_25g_50g = 50;   /* 50G exist */
            }
        }
        for (j = 0; j < 4; j++)
        {
            tmp_serdes = j+4*i;
            tmp_serdes += HSS15G_NUM_PER_SLICE*HSS15G_LANE_NUM;
            if (((CTC_CHIP_SERDES_LG_MODE == arr_serdes_mode[tmp_serdes]) && (25 == flag_25g_50g))
                || ((CTC_CHIP_SERDES_XXVG_MODE == arr_serdes_mode[tmp_serdes]) && (50 == flag_25g_50g)))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " Serdes mode 25G and 50G cannot exist on every 4 lanes simultaneously\n");
                return CTC_E_INVALID_CONFIG;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_datapath_init_db(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8 serdes_id = 0;
    uint8 slice_id = 0;
    uint16 lport = 0;

    p_usw_datapath_master[lchip]->core_plla = p_datapath_cfg->core_frequency_a;
    p_usw_datapath_master[lchip]->core_pllb = p_datapath_cfg->core_frequency_b;

    p_usw_datapath_master[lchip]->wlan_enable = p_datapath_cfg->wlan_enable;
    p_usw_datapath_master[lchip]->dot1ae_enable = p_datapath_cfg->dot1ae_enable;

    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_build_serdes_info(lchip, serdes_id, p_datapath_cfg->serdes[serdes_id].mode, p_datapath_cfg->serdes[serdes_id].group,
            p_datapath_cfg->serdes[serdes_id].is_dynamic, CTC_CHIP_SERDES_OCS_MODE_NONE));
    }

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }
            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                port_attr->mac_id = g_usw_mac_port_map[lport];
                port_attr->chan_id = g_usw_mac_port_map[lport];
                port_attr->serdes_num = 0;
                port_attr->serdes_id = 0xff;
                port_attr->speed_mode = CTC_PORT_SPEED_MAX;
            }
        }
    }

    /* check initial serdes mode illegal or not */
    CTC_ERROR_RETURN(_sys_duet2_datapath_check_serdes_mode(lchip));

    /*internal channel alloc */
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_OAM_CPU_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_OAM);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_OAM_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_ILOOP_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_ILOOP_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_ELOOP_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_ELOOP_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT0_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT1_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT2_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT3_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT0_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT1_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT2_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT3_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_REASSEMBLE_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANREASSEMBLE_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_MAC_DECRYPT_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->dot1ae_enable ? SYS_DMPS_MAC_DECRYPT_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_MAC_ENCRYPT_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->dot1ae_enable ? SYS_DMPS_MAC_ENCRYPT_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;
    }

    p_usw_datapath_master[lchip]->oam_chan = MCHIP_CAP(SYS_CAP_CHANID_OAM);
    p_usw_datapath_master[lchip]->dma_chan = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);
    p_usw_datapath_master[lchip]->iloop_chan = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
    p_usw_datapath_master[lchip]->eloop_chan = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
    p_usw_datapath_master[lchip]->cawapdec0_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0);
    p_usw_datapath_master[lchip]->cawapdec1_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1);
    p_usw_datapath_master[lchip]->cawapdec2_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2);
    p_usw_datapath_master[lchip]->cawapdec3_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3);
    p_usw_datapath_master[lchip]->cawapenc0_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0);
    p_usw_datapath_master[lchip]->cawapenc1_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1);
    p_usw_datapath_master[lchip]->cawapenc2_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2);
    p_usw_datapath_master[lchip]->cawapenc3_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3);
    p_usw_datapath_master[lchip]->cawapreassemble_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE);
    p_usw_datapath_master[lchip]->macdec_chan = MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT);
    p_usw_datapath_master[lchip]->macenc_chan = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
    p_usw_datapath_master[lchip]->elog_chan = MCHIP_CAP(SYS_CAP_CHANID_ELOG);
    p_usw_datapath_master[lchip]->qcn_chan = MCHIP_CAP(SYS_CAP_CHANID_QCN);

    return CTC_E_NONE;
}

int32
sys_duet2_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    switch (chip_prop)
    {
    case CTC_CHIP_PROP_SERDES_PRBS:
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_prbs(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_FFE:
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe(lchip, p_value));
        break;
    case CTC_CHIP_PEOP_SERDES_POLARITY:
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_LOOPBACK:
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_loopback(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_P_FLAG:
    case CTC_CHIP_PROP_SERDES_PEAK:
    case CTC_CHIP_PROP_SERDES_DPC:
    case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_cfg(lchip, chip_prop, p_value));
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_duet2_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    uint16 coefficient[SYS_USW_CHIP_FFE_PARAM_NUM] = {0};
    uint8 index = 0;
    uint8 value = 0;
    ctc_chip_serdes_ffe_t* p_ffe = NULL;
    
    switch (chip_prop)
    {
    case CTC_CHIP_PROP_SERDES_FFE:
        p_ffe = (ctc_chip_serdes_ffe_t*)p_value;
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe(lchip, p_ffe->serdes_id, coefficient, p_ffe->mode, &value));
        for (index = 0; index < SYS_USW_CHIP_FFE_PARAM_NUM; index++)
        {
            p_ffe->coefficient[index] = coefficient[index];
        }
        p_ffe->status = value;
        break;
    case CTC_CHIP_PEOP_SERDES_POLARITY:
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_polarity(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_LOOPBACK:
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_P_FLAG:
    case CTC_CHIP_PROP_SERDES_PEAK:
    case CTC_CHIP_PROP_SERDES_DPC:
    case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_cfg(lchip, chip_prop, p_value));
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32 
sys_duet2_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info)
{
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_serdes_dynamic_switch(lchip, \
                                    p_serdes_info->serdes_id, \
                                    p_serdes_info->serdes_mode, \
                                    p_serdes_info->overclocking_speed));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_duet2_serdes_get_link_training_status(uint8 lchip, uint16 serdes_id, uint16* p_value)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint8   hss_type = 0;
    uint32  addr     = 0;

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    /* get training result */
    addr = 6;
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        drv_chip_read_hss15g_aec_aet(lchip, hss_id, lane_id, 0, addr, p_value);
    }
    else
    {
        drv_chip_read_hss28g_aec_aet(lchip, hss_id, lane_id, 0, addr, p_value);
    }

    return CTC_E_NONE;
}

/* Support 802.3ap, auto link training for Backplane Ethernet */
int32
sys_duet2_serdes_set_link_training_en(uint8 lchip, uint16 serdes_id, bool enable)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   hss_type = 0;
    ctc_chip_serdes_ffe_t ffe;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d\n", serdes_id, enable);

    sal_memset(&ffe, 0, sizeof(ctc_chip_serdes_ffe_t));

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    /* get serdes info from sw table */
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }

    /* operation */
    ffe.mode = CTC_CHIP_SERDES_FFE_MODE_3AP;
    ffe.serdes_id = serdes_id;
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        ffe.coefficient[0] = 0x0;
        ffe.coefficient[1] = 43;
        ffe.coefficient[2] = 0x0;
        ffe.coefficient[3] = 0x0;
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe(lchip, &ffe));
        CTC_ERROR_RETURN(sys_usw_datapath_hss15g_serdes_aec_aet(lchip, hss_id, lane_id, p_hss_vec, enable, serdes_id));
    }
    else
    {
        ffe.coefficient[0] = 0x0;
        ffe.coefficient[1] = 0x0;
        ffe.coefficient[2] = 34;
        ffe.coefficient[3] = 0x0;
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe(lchip, &ffe));
        CTC_ERROR_RETURN(sys_usw_datapath_hss28g_serdes_aec_aet(lchip, hss_id, lane_id, p_hss_vec, enable, serdes_id));
    }

    return CTC_E_NONE;
}

int32
sys_duet2_datapath_init(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8 serdes_id = 0;
    uint8 hss_idx = 0;
    uint8 hss_id = 0;
    uint8 internal_lane = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8 slice_id = 0;
    uint32 cmd = 0;
    uint8 mac_id = 0;
    NetTxPortDynamicInfo_m dyn_info;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 lane_id = 0;
    uint8 lane_idx = 0;
    uint16 address = 0;
    uint16 value = 0;
    ctc_chip_serdes_polarity_t serdes_polarity;

    /*init port attribute to rsv port default*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                return CTC_E_INVALID_PARAM;
            }
            port_attr->port_type =  SYS_DMPS_RSV_PORT;
        }
    }

    CTC_ERROR_RETURN(_sys_duet2_datapath_sup_init(lchip));
    /*pre-0. init MCU data mem*/
    if (0 == sys_usw_chip_get_reset_hw_en(lchip))
    {
        CTC_ERROR_RETURN(_sys_duet2_datapath_mcu_mem_init(lchip));
    }

    /*0. prepare data base*/
    CTC_ERROR_RETURN(_sys_duet2_datapath_init_db(lchip, p_datapath_cfg));

    /*1. do module release */
    CTC_ERROR_RETURN(_sys_duet2_datapath_core_clock_init(lchip, p_datapath_cfg->core_frequency_a, p_datapath_cfg->core_frequency_b));


    CTC_ERROR_RETURN(sys_usw_datapath_module_init(lchip));

    /*init NetTxPortDynamicInfo0 default*/
    sal_memset(&dyn_info, 0, sizeof(NetTxPortDynamicInfo_m));
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
        {
            cmd = DRV_IOW((NetTxPortDynamicInfo_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, mac_id, cmd, &dyn_info);
        }
    }

    /*2. do hss init */
    for (hss_idx = 0; hss_idx < SYS_DATAPATH_HSS_NUM; hss_idx++)
    {
        if (SYS_DATAPATH_HSS_IS_HSS28G(hss_idx))
        {
            CTC_ERROR_RETURN(_sys_duet2_datapath_hss28g_init(lchip, hss_idx));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_duet2_datapath_hss15g_init(lchip, hss_idx));
        }
    }

    /*3. do serdes init */
    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_hss_internal(lchip, serdes_id, p_datapath_cfg->serdes[serdes_id].mode));
    }

    /*4. do clktree init */
    for (hss_idx = 0; hss_idx < SYS_DATAPATH_HSS_NUM; hss_idx++)
    {
        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss)
        {
            continue;
        }

        if (SYS_DATAPATH_HSS_IS_HSS28G(hss_idx))
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_cfg_clktree(lchip, hss_idx,SYS_DATAPATH_PLL_SEL_BOTH));

            for (lane_idx = 0; lane_idx < HSS28G_LANE_NUM; lane_idx++)
            {
                CTC_ERROR_RETURN(_sys_usw_datapath_hss28g_cfg_lane_clktree(lchip, lane_idx, p_hss));
            }
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_cfg_clktree(lchip, hss_idx,SYS_DATAPATH_PLL_SEL_BOTH));

            for (lane_idx = 0; lane_idx < HSS15G_LANE_NUM; lane_idx++)
            {
                CTC_ERROR_RETURN(_sys_usw_datapath_hss15g_cfg_lane_clktree(lchip, lane_idx, p_hss));
            }
        }
    }

    /* 4.0.1 after HSS clktree config, toggle AnethWrapper reset/release */
    CTC_ERROR_RETURN(sys_usw_datapath_set_misc_cfg(lchip));

    /*5. set serdes poly*/
    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_link_reset(lchip, (serdes_id), TRUE, 1));
        sal_task_sleep(1);
        CTC_ERROR_RETURN(sys_usw_datapath_set_link_reset(lchip, (serdes_id), FALSE, 1));
        sal_task_sleep(1);

        if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
            lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
            internal_lane = g_usw_hss15g_lane_map[lane_id];
            /*hss15g using sum mode*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 2);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value &= 0xff7f;
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value |= 0x01;
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
        serdes_polarity.serdes_id = serdes_id;

        serdes_polarity.dir = 0;
        serdes_polarity.polarity_mode= (p_datapath_cfg->serdes[serdes_id].rx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &serdes_polarity));

        serdes_polarity.dir = 1;
        serdes_polarity.polarity_mode = (p_datapath_cfg->serdes[serdes_id].tx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &serdes_polarity));

        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, serdes_id, FALSE));
    }

    /*6. do datapath init */
    CTC_ERROR_RETURN(sys_usw_datapath_datapth_init(lchip));

    /*7. do Impedance Calibration */
    CTC_ERROR_RETURN(sys_usw_datapath_set_impedance_calibration(lchip));

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_datapath_wb_restore(lchip));
    }

    return CTC_E_NONE;
}


#if (FEATURE_MODE == 0)
/**
 @file sys_usw_sync_ether.c

 @date 2012-10-18

 @version v2.0

 sync ethernet function of Duet2
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_dmps.h"
#include "sys_usw_sync_ether.h"

#include "drv_api.h"
/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
/* DUET2 IS 2; TM IS 3 */
#define SYS_SYNC_ETHER_CLOCK_MAX 3

/**
 @brief  Define sys layer synce global configure data structure
*/
struct sys_sync_ether_master_s
{
    sal_mutex_t* p_sync_ether_mutex;
    uint8 recovered_clock_lport[SYS_SYNC_ETHER_CLOCK_MAX];
};
typedef struct sys_sync_ether_master_s sys_sync_ether_master_t;

#define SYNCE_LOCK \
    if (p_usw_sync_ether_master[lchip]->p_sync_ether_mutex) sal_mutex_lock(p_usw_sync_ether_master[lchip]->p_sync_ether_mutex)
#define SYNCE_UNLOCK \
    if (p_usw_sync_ether_master[lchip]->p_sync_ether_mutex) sal_mutex_unlock(p_usw_sync_ether_master[lchip]->p_sync_ether_mutex)

#define SYS_SYNC_ETHER_INIT_CHECK() \
do { \
    SYS_LCHIP_CHECK_ACTIVE(lchip); \
    if (NULL == p_usw_sync_ether_master[lchip]) \
    { \
        return CTC_E_NOT_INIT; \
    } \
} while (0)

#define SYS_SYNC_ETHER_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(sync_ether, sync_ether, SYNC_ETHER_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

sys_sync_ether_master_t* p_usw_sync_ether_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern sys_chip_master_t* p_usw_chip_master;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#define __1_SYNC_ETHER_WB__
STATIC int32
_sys_usw_sync_ether_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_syncether_master_t *p_wb_syncether_master;
    uint8 sync_ether_clock_id;

    if (NULL == p_usw_sync_ether_master[lchip])
    {
        return CTC_E_NONE;
    }

    SYNCE_LOCK;

    /*syncup buffer*/
    wb_data.buffer = mem_malloc(MEM_SYNC_ETHER_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*syncup statsid*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_syncether_master_t, CTC_FEATURE_SYNC_ETHER, SYS_WB_APPID_SYNCETHER_SUBID_MASTER);

    p_wb_syncether_master = (sys_wb_syncether_master_t *)wb_data.buffer;
    sal_memset(p_wb_syncether_master, 0, sizeof(sys_wb_syncether_master_t));

    p_wb_syncether_master->lchip = lchip;
    p_wb_syncether_master->version = SYS_WB_VERSION_SYNCETHER;

    for (sync_ether_clock_id = 0; sync_ether_clock_id < MCHIP_CAP(SYS_CAP_SYNC_ETHER_CLOCK); sync_ether_clock_id++)
    {
        if (sync_ether_clock_id >= SYS_WB_SYNCETHER_CLOCK_MAX)
        {
            break;
        }

        p_wb_syncether_master->recovered_clock_lport[sync_ether_clock_id] = p_usw_sync_ether_master[lchip]->recovered_clock_lport[sync_ether_clock_id];
    }

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    SYNCE_UNLOCK;

    return ret;
}

STATIC int32
_sys_usw_sync_ether_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_syncether_master_t wb_syncether_master;
    uint8 sync_ether_clock_id;

    SYNCE_LOCK;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_SYNC_ETHER_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_syncether_master_t, CTC_FEATURE_SYNC_ETHER, SYS_WB_APPID_SYNCETHER_SUBID_MASTER);

    sal_memset(&wb_syncether_master, 0, sizeof(sys_wb_syncether_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if ((wb_query.valid_cnt != 1) || (wb_query.is_end != 1))
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query sync ether master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        CTC_ERROR_GOTO(CTC_E_INVALID_CONFIG, ret, done);
    }

    sal_memcpy((uint8 *)&wb_syncether_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_SYNCETHER, wb_syncether_master.version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    for (sync_ether_clock_id = 0; sync_ether_clock_id < SYS_WB_SYNCETHER_CLOCK_MAX; sync_ether_clock_id++)
    {
        p_usw_sync_ether_master[lchip]->recovered_clock_lport[sync_ether_clock_id] = wb_syncether_master.recovered_clock_lport[sync_ether_clock_id];
    }

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    SYNCE_UNLOCK;

    return ret;
}

#define __2_SYNC_ETHER_MII__
int32
_sys_usw_sync_ether_mii_bit_get(uint16 serdes_id, uint16 *mii_bit)
{
    uint16 remainder = 0;

    /* only serdes 0,1,4,5,8,9,12,13,16,17,20,21 support */

    if (serdes_id >= 24)
    {
        return CTC_E_NOT_SUPPORT;
    }

    remainder = serdes_id % 4;

    if (0 == remainder)
    {
        *mii_bit = serdes_id / 2;
    }
    else if (1 == remainder)
    {
        *mii_bit = (serdes_id / 2) + 1;
    }
    else
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

#define __3_SYNC_ETHER_API__
int32
sys_usw_sync_ether_set_cfg(uint8 lchip, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 cfg = 0;
    uint32 reset_offset = 0;
    uint32 divider_offset = 0;
    uint32 link_status_disable_offset = 0;
    uint32 clock_select_offset = 0;
    uint32 link_select_offset = 0;
    uint32 use_qsgmii = 0;
    uint32 use_usxgmii = 0;
    uint32 value = 0;
    uint8 clock_select = 0;
    uint8 gchip = 0;
    uint32 recovered_clock_gport = 0;
    uint32 port_type = 0;
    uint16 mii_bit = 0;
    uint32 value_qsgmii = 0;
    uint32 value_usxgmii = 0;
    sys_dmps_serdes_info_t serdes_info;

    SYS_SYNC_ETHER_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_sync_ether_cfg);

    /*debug info*/
    SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set sync-ether config, lchip:%d, clock_id:%d, divider:%d,\
                            link_status_detect_en:%d, clock_output_en:%d, recovered_clock_lport: %d\n", lchip,
                           sync_ether_clock_id, p_sync_ether_cfg->divider, p_sync_ether_cfg->link_status_detect_en,
                           p_sync_ether_cfg->clock_output_en, p_sync_ether_cfg->recovered_clock_lport);

    if (sync_ether_clock_id >= MCHIP_CAP(SYS_CAP_SYNC_ETHER_CLOCK))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* SYSTEM MODIFIED by tongzb for support syncE on TM, 2019-02-01 */
#if 0    
    if (p_sync_ether_cfg->divider >= MCHIP_CAP(SYS_CAP_SYNC_ETHER_DIVIDER))
    {
        return CTC_E_INVALID_PARAM;
    }
#endif
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    SYNCE_LOCK;

    recovered_clock_gport = CTC_MAP_LPORT_TO_GPORT(gchip, p_sync_ether_cfg->recovered_clock_lport);

    CTC_ERROR_GOTO(sys_usw_dmps_get_port_property(lchip, recovered_clock_gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type), ret, out);
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
	ret = CTC_E_INVALID_CONFIG;
	goto out;
    }

    sal_memset(&serdes_info, 0, sizeof(sys_dmps_serdes_info_t));
    CTC_ERROR_GOTO(sys_usw_dmps_get_port_property(lchip, recovered_clock_gport, SYS_DMPS_PORT_PROP_SERDES, (void *)&serdes_info), ret, out);

    /* RlmHsCtlSyncECfg  serdes_id 0~23 */
    if (serdes_info.serdes_id[0] < 24)
    {
        cfg = RlmHsCtlSyncECfg_t;
        reset_offset = RlmHsCtlSyncECfg_cfgClkResetEtherClkPrimary_f + sync_ether_clock_id;
        divider_offset = RlmHsCtlSyncECfg_cfgClkDividerEtherClkPrimary_f + sync_ether_clock_id;
        link_status_disable_offset = RlmHsCtlSyncECfg_cfgFastLinkFailureDisPrimary_f + sync_ether_clock_id;
        clock_select_offset = RlmHsCtlSyncECfg_cfgEtherClkSelectPrimary_f + sync_ether_clock_id;
        link_select_offset = RlmHsCtlSyncECfg_cfgLinkStatusSelectPrimary_f + sync_ether_clock_id;
        use_qsgmii = RlmHsCtlSyncECfg_cfgUseQsgmiiPrimary_f + sync_ether_clock_id;
        use_usxgmii = RlmHsCtlSyncECfg_cfgUseUsxgmiiPrimary_f + sync_ether_clock_id;

        clock_select = serdes_info.serdes_id[0];
    }
    /* RlmCsCtlSyncECfg  serdes_id 24~39 */
    else
    {
        cfg = RlmCsCtlSyncECfg_t;
        reset_offset = RlmCsCtlSyncECfg_cfgClkResetEtherClkPrimary_f + sync_ether_clock_id;
        divider_offset = RlmCsCtlSyncECfg_cfgClkDividerEtherClkPrimary_f + sync_ether_clock_id;
        link_status_disable_offset = RlmCsCtlSyncECfg_cfgFastLinkFailureDisPrimary_f + sync_ether_clock_id;
        clock_select_offset = RlmCsCtlSyncECfg_cfgEtherClkSelectPrimary_f + sync_ether_clock_id;
        link_select_offset = RlmCsCtlSyncECfg_cfgLinkStatusSelectPrimary_f + sync_ether_clock_id;

        clock_select = serdes_info.serdes_id[0] - 24;
    }

    /* set reset */
    value = 1;
    cmd = DRV_IOW(cfg, reset_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    /* set config */
    value = p_sync_ether_cfg->divider;
    cmd = DRV_IOW(cfg, divider_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    value = p_sync_ether_cfg->link_status_detect_en ? 0 : 1;
    cmd = DRV_IOW(cfg, link_status_disable_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    value = p_sync_ether_cfg->clock_output_en ?  (1 << clock_select) : 0;
    cmd = DRV_IOW(cfg, clock_select_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    value = clock_select;
    cmd = DRV_IOW(cfg, link_select_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    ret = _sys_usw_sync_ether_mii_bit_get(serdes_info.serdes_id[0], &mii_bit);
    if (CTC_E_NONE == ret)
    {
        value_qsgmii = 0;
        cmd = DRV_IOR(cfg, use_qsgmii);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_qsgmii), ret, out);

        value_usxgmii = 0;
        cmd = DRV_IOR(cfg, use_usxgmii);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_usxgmii), ret, out);

        switch (serdes_info.serdes_mode)
        {
            case CTC_CHIP_SERDES_QSGMII_MODE:
            {
                /* unset bit must be first */
                value_usxgmii &= ~(1 << mii_bit);
                cmd = DRV_IOW(cfg, use_usxgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_usxgmii), ret, out);

                value_qsgmii |= 1 << mii_bit;
                cmd = DRV_IOW(cfg, use_qsgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_qsgmii), ret, out);
                break;
            }
            case CTC_CHIP_SERDES_USXGMII0_MODE:
            case CTC_CHIP_SERDES_USXGMII1_MODE:
            case CTC_CHIP_SERDES_USXGMII2_MODE:
            {
                /* unset bit must be first */
                value_qsgmii &= ~(1 << mii_bit);
                cmd = DRV_IOW(cfg, use_qsgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_qsgmii), ret, out);

                value_usxgmii |= 1 << mii_bit;
                cmd = DRV_IOW(cfg, use_usxgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_usxgmii), ret, out);
                break;
            }
            default:
            {
                value_qsgmii &= ~(1 << mii_bit);
                cmd = DRV_IOW(cfg, use_qsgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_qsgmii), ret, out);

                value_usxgmii &= ~(1 << mii_bit);
                cmd = DRV_IOW(cfg, use_usxgmii);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value_usxgmii), ret, out);
                break;
            }
        }
    }

    p_usw_sync_ether_master[lchip]->recovered_clock_lport[sync_ether_clock_id] = p_sync_ether_cfg->recovered_clock_lport;

    /* set reset */
    value = 0;
    cmd = DRV_IOW(cfg, reset_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);

    SYNCE_UNLOCK;
    return CTC_E_NONE;

out:
    SYNCE_UNLOCK;
    return ret;
}

int32
sys_usw_sync_ether_get_cfg(uint8 lchip, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 cfg = 0;
    uint32 divider_offset = 0;
    uint32 link_status_disable_offset = 0;
    uint32 clock_select_offset = 0;
    uint8 gchip = 0;
    uint32 recovered_clock_gport = 0;
    uint32 port_type = 0;
    sys_dmps_serdes_info_t serdes_info;

    SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get sync-ether config, lchip:%d, clock_id:%d\n", lchip, sync_ether_clock_id);

    SYS_SYNC_ETHER_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_sync_ether_cfg);

    if (sync_ether_clock_id >= MCHIP_CAP(SYS_CAP_SYNC_ETHER_CLOCK))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    SYNCE_LOCK;

    p_sync_ether_cfg->recovered_clock_lport = p_usw_sync_ether_master[lchip]->recovered_clock_lport[sync_ether_clock_id];

    /*read config*/
    recovered_clock_gport = CTC_MAP_LPORT_TO_GPORT(gchip, p_sync_ether_cfg->recovered_clock_lport);

    CTC_ERROR_GOTO(sys_usw_dmps_get_port_property(lchip, recovered_clock_gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type), ret, out);
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        ret = CTC_E_INVALID_CONFIG;
	goto out;
    }

    sal_memset(&serdes_info, 0, sizeof(sys_dmps_serdes_info_t));
    CTC_ERROR_GOTO(sys_usw_dmps_get_port_property(lchip, recovered_clock_gport, SYS_DMPS_PORT_PROP_SERDES, (void *)&serdes_info), ret, out);

    if (serdes_info.serdes_id[0] < 24)
    {
        cfg = RlmHsCtlSyncECfg_t;
        divider_offset = RlmHsCtlSyncECfg_cfgClkDividerEtherClkPrimary_f + sync_ether_clock_id;
        link_status_disable_offset = RlmHsCtlSyncECfg_cfgFastLinkFailureDisPrimary_f + sync_ether_clock_id;
        clock_select_offset = RlmHsCtlSyncECfg_cfgEtherClkSelectPrimary_f + sync_ether_clock_id;
    }
    else
    {
        cfg = RlmCsCtlSyncECfg_t;
        divider_offset = RlmCsCtlSyncECfg_cfgClkDividerEtherClkPrimary_f + sync_ether_clock_id;
        link_status_disable_offset = RlmCsCtlSyncECfg_cfgFastLinkFailureDisPrimary_f + sync_ether_clock_id;
        clock_select_offset = RlmCsCtlSyncECfg_cfgEtherClkSelectPrimary_f + sync_ether_clock_id;
    }

    cmd = DRV_IOR(cfg, divider_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);
    p_sync_ether_cfg->divider = value;

    cmd = DRV_IOR(cfg, link_status_disable_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);
    p_sync_ether_cfg->link_status_detect_en = value ? 0 : 1;

    cmd = DRV_IOR(cfg, clock_select_offset);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, out);
    p_sync_ether_cfg->clock_output_en = value ? 1 : 0;

    SYNCE_UNLOCK;
    return CTC_E_NONE;

out:
    SYNCE_UNLOCK;
    return ret;
}

#define __4_SYNC_ETHER_INIT__
int32
sys_usw_sync_ether_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd0 = 0;
    uint32 cmd1 = 0;
    uint32 cmd = 0;
    uint32 field_value = 0;
    RlmHsCtlSyncECfg_m hs_sync_ether_clk_cfg0;
    RlmCsCtlSyncECfg_m cs_sync_ether_clk_cfg0;

    if (p_usw_sync_ether_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }

    /* create SyncE master */
    p_usw_sync_ether_master[lchip] = (sys_sync_ether_master_t*)mem_malloc(MEM_SYNC_ETHER_MODULE, sizeof(sys_sync_ether_master_t));
    if (NULL == p_usw_sync_ether_master[lchip])
    {
        SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error_malloc;
    }

    sal_memset(p_usw_sync_ether_master[lchip], 0, sizeof(sys_sync_ether_master_t));

    ret = sal_mutex_create(&p_usw_sync_ether_master[lchip]->p_sync_ether_mutex);
    if (ret || !p_usw_sync_ether_master[lchip]->p_sync_ether_mutex)
    {
        SYS_SYNC_ETHER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        ret = CTC_E_NO_MEMORY;
        goto error_mutex;
    }

    /*set default: link status detect enable, clock divider by 1*/
    sal_memset(&hs_sync_ether_clk_cfg0, 0, sizeof(hs_sync_ether_clk_cfg0));
    SetRlmHsCtlSyncECfg(V, cfgClkDividerEtherClkPrimary_f, &hs_sync_ether_clk_cfg0, 0);
    SetRlmHsCtlSyncECfg(V, cfgFastLinkFailureDisPrimary_f, &hs_sync_ether_clk_cfg0, 0);
    SetRlmHsCtlSyncECfg(V, cfgClkDividerEtherClkSecondary_f, &hs_sync_ether_clk_cfg0, 0);
    SetRlmHsCtlSyncECfg(V, cfgFastLinkFailureDisSecondary_f, &hs_sync_ether_clk_cfg0, 0);

    sal_memset(&cs_sync_ether_clk_cfg0, 0, sizeof(cs_sync_ether_clk_cfg0));
    SetRlmCsCtlSyncECfg(V, cfgClkDividerEtherClkPrimary_f, &cs_sync_ether_clk_cfg0, 0);
    SetRlmCsCtlSyncECfg(V, cfgFastLinkFailureDisPrimary_f, &cs_sync_ether_clk_cfg0, 0);
    SetRlmCsCtlSyncECfg(V, cfgClkDividerEtherClkSecondary_f, &cs_sync_ether_clk_cfg0, 0);
    SetRlmCsCtlSyncECfg(V, cfgFastLinkFailureDisSecondary_f, &cs_sync_ether_clk_cfg0, 0);

    cmd0 = DRV_IOW(RlmHsCtlSyncECfg_t, DRV_ENTRY_FLAG);
    cmd1 = DRV_IOW(RlmCsCtlSyncECfg_t, DRV_ENTRY_FLAG);

    if (TRUE)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd0, &hs_sync_ether_clk_cfg0), ret, error_iotcl);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd1, &cs_sync_ether_clk_cfg0), ret, error_iotcl);
    }

    /*init cfgTsUseIntRefClk */
    field_value = 0;
    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgTsUseIntRefClk_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_iotcl);
    }
    /* warmboot register */
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_SYNC_ETHER, _sys_usw_sync_ether_wb_sync), ret, error_wb);

    /* warmboot data restore */
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(_sys_usw_sync_ether_wb_restore(lchip), ret, error_wb);
    }

    return CTC_E_NONE;

error_wb:
error_iotcl:
    sal_mutex_destroy(p_usw_sync_ether_master[lchip]->p_sync_ether_mutex);
error_mutex:
    mem_free(p_usw_sync_ether_master[lchip]);
error_malloc:
    return ret;
}

int32
sys_usw_sync_ether_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_usw_sync_ether_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(p_usw_sync_ether_master[lchip]->p_sync_ether_mutex);
    mem_free(p_usw_sync_ether_master[lchip]);

    return CTC_E_NONE;
}

#endif


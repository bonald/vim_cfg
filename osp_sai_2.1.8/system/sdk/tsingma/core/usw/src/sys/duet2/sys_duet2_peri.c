/**
 @file sys_usw_peri.c

 @date 2009-10-19

 @version v2.0

 The file define APIs of chip of sys layer
*/
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_error.h"
#include "ctc_register.h"
#include "sys_usw_peri.h"
#include "sys_usw_chip_global.h"
#include "sys_usw_mac.h"
#include "sys_usw_dmps.h"
#include "sys_usw_chip.h"
#include "sys_usw_datapath.h"
#include "sys_usw_common.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_dma.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and static
 *
 *****************************************************************************/
 #define SYS_DUET2_MDIO_CTLR_NUM 4

extern sys_peri_master_t* p_usw_peri_master[CTC_MAX_LOCAL_CHIP_NUM];

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

#define __MDIO_INTERFACE__

/* called by MDIO scan init and dynamic switch */
int32
sys_duet2_peri_set_phy_scan_cfg(uint8 lchip)
{
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 mac_id = 0;
    uint32 use_phy0 = 0;
    uint32 use_phy1 = 0;
    uint32 mdio_sel0 = 0;
    uint32 mdio_sel1 = 0;
    uint8 mdio_bus  = 0;
    uint32 cmd = 0;
    uint32 arr_phy[2] = {0};
    uint32 arr_mdiosel[2] = {0};
    ctc_chip_phy_mapping_para_t* p_phy_mapping = NULL;
    RlmBsrCtlMdioSel_m    mdio_sel;
    MdioUserPhy0_m        user_phy0;
    MdioUserPhy1_m        user_phy1;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 field_value = 0;
    uint8 phy_addr = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;
    uint32 phy_bitmap0[SYS_CHIP_MAX_MDIO_BUS] = {0};
    uint32 phy_bitmap1[SYS_CHIP_MAX_MDIO_BUS] = {0};
    MdioScanCtl00_m mdio_scan;

    sal_memset(&mdio_sel , 0, sizeof(RlmBsrCtlMdioSel_m));
    sal_memset(&user_phy0 , 0, sizeof(MdioUserPhy0_m));
    sal_memset(&user_phy1 , 0, sizeof(MdioUserPhy0_m));

    p_phy_mapping = (ctc_chip_phy_mapping_para_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_chip_phy_mapping_para_t));
    if (NULL == p_phy_mapping)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_phy_mapping, 0, sizeof(ctc_chip_phy_mapping_para_t));

    CTC_ERROR_GOTO(sys_usw_peri_get_phy_mapping(lchip, p_phy_mapping), ret, out);
    CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, out);

    /*2. cfg mdio use phy or not to select link status source */
    for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
    {
        if (p_phy_mapping->port_phy_mapping_tbl[lport] != 0xff)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
            mac_id = SYS_GET_MAC_ID(lchip, gport);
            if (0xFF == mac_id)
            {
                continue;
            }
            mdio_bus = p_phy_mapping->port_mdio_mapping_tbl[lport];

            CTC_ERROR_GOTO(sys_usw_mac_get_port_capability(lchip, lport, &port_attr), ret, out);
            if (SYS_DMPS_NETWORK_PORT != port_attr->port_type)
            {
                continue;
            }

            field_value = mac_id;
            phy_addr = p_phy_mapping->port_phy_mapping_tbl[lport];
            if (0xFF == phy_addr)
            {
                continue;
            }
            if (0 == mdio_bus)
            {
                cmd = DRV_IOW(MdioPortIdMap00_t, MdioPortIdMap00_macId_f);
            }
            else if (1 == mdio_bus)
            {
                cmd = DRV_IOW(MdioPortIdMap01_t, MdioPortIdMap01_macId_f);
            }
            else if (2 == mdio_bus)
            {
                cmd = DRV_IOW(MdioPortIdMap10_t, MdioPortIdMap10_macId_f);
            }
            else if (3 == mdio_bus)
            {
                cmd = DRV_IOW(MdioPortIdMap11_t, MdioPortIdMap11_macId_f);
            }
            DRV_IOCTL(lchip, phy_addr, cmd, &field_value);

            if ((CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_USXGMII0_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_USXGMII1_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_USXGMII2_MODE != port_attr->pcs_mode))
            {
                /* Has no PHY */
                continue;
            }

            if (mac_id < 32)
            {
                use_phy0  |= (1 << mac_id);
                if (mdio_bus < 4)
                {
                    mdio_sel0 &= ~(1 << mac_id);
                }
                else
                {
                    mdio_sel0 |= (1 << mac_id);
                }
            }
            else
            {
                use_phy1  |= (1 << (mac_id - 32));
                if (mdio_bus < 4)
                {
                    mdio_sel1 &= ~(1 << (mac_id - 32));
                }
                else
                {
                    mdio_sel1 |= (1 << (mac_id - 32));
                }
            }
            if (phy_addr < 32)
            {
                phy_bitmap0[mdio_bus] |= (1<<phy_addr);
            }
            else
            {
                phy_bitmap1[mdio_bus]  |= (1 << (phy_addr - 32));
            }
        }
    }

    /* config use phy or not */
    arr_phy[0] = use_phy0;
    arr_phy[1] = use_phy1;
    cmd = DRV_IOR(MdioUserPhy0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy0);
    SetMdioUserPhy0(A, usePhy_f, &user_phy0, arr_phy);
    cmd = DRV_IOW(MdioUserPhy0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy0);

    cmd = DRV_IOR(MdioUserPhy1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy1);
    SetMdioUserPhy1(A, usePhy_f, &user_phy1, arr_phy);
    cmd = DRV_IOW(MdioUserPhy1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy1);

    /* config mdioSel */
    cmd = DRV_IOR(RlmBsrCtlMdioSel_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_sel);
    arr_mdiosel[0] = mdio_sel0;
    arr_mdiosel[1] = mdio_sel1;
    SetRlmBsrCtlMdioSel(A, mdioSel_f, &mdio_sel, arr_mdiosel);
    cmd = DRV_IOW(RlmBsrCtlMdioSel_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_sel);

    /* config mdio phy bitmap */
    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl00_m));
    tbl_id = MdioScanCtl00_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &phy_bitmap0[0], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &phy_bitmap1[0], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);

    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl00_m));
    tbl_id = MdioScanCtl01_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &phy_bitmap0[1], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &phy_bitmap1[1], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);

    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl00_m));
    tbl_id = MdioScanCtl10_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &phy_bitmap0[2], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &phy_bitmap1[2], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);

    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl00_m));
    tbl_id = MdioScanCtl11_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &phy_bitmap0[3], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &phy_bitmap1[3], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
out:
    if (p_phy_mapping)
    {
        mem_free(p_phy_mapping);
    }

    return ret;
}

/**
 @brief smi interface for set mdio phy type
*/
STATIC int32
_sys_duet2_peri_set_mdio_phy_type(uint8 lchip, uint8 ctlr_id, uint32 type)
{
    uint32 cmd = 0;
    uint32 mdio_phy_type = 0;
    uint32 tbl_id = 0;

    if (0 == ctlr_id)
    {
        tbl_id = MdioCfg00_t;
    }
    else if (1 == ctlr_id)
    {
        tbl_id = MdioCfg01_t;
    }
    else if (2 == ctlr_id)
    {
        tbl_id = MdioCfg10_t;
    }
    else if (3 == ctlr_id)
    {
        tbl_id = MdioCfg11_t;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, MdioCfg00_mdioIsXgModeLane0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mdio_phy_type));

    if (mdio_phy_type != type)
    {
        cmd = DRV_IOW(tbl_id, MdioCfg00_mdioIsXgModeLane0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &type));
    }

    return CTC_E_NONE;
}

/**
 @brief smi interface for get mdio phy type
*/
STATIC int32
_sys_duet2_peri_get_mdio_phy_type(uint8 lchip, uint8 ctlr_id, uint32* type)
{
    uint32 cmd = 0;
    uint32 mdio_phy_type = 0;
    uint32 tbl_id = 0;

    if (0 == ctlr_id)
    {
        tbl_id = MdioCfg00_t;
    }
    else if (1 == ctlr_id)
    {
        tbl_id = MdioCfg01_t;
    }
    else if (2 == ctlr_id)
    {
        tbl_id = MdioCfg10_t;
    }
    else if (3 == ctlr_id)
    {
        tbl_id = MdioCfg11_t;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, MdioCfg00_mdioIsXgModeLane0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mdio_phy_type));

    *type = mdio_phy_type;

    return CTC_E_NONE;
}

/**
 @brief smi interface for set auto scan para
*/
int32
sys_duet2_peri_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint32 cmd = 0;
    MdioScanCtl00_m  scan_ctrl;
    uint32 tmp_val32 = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 index = 0;
    uint32 arr_phy[2] = {0};
    uint32 mdio_sel0 = 0;
    uint32 mdio_sel1 = 0;
    uint32 arr_mdiosel[2] = {0};
    MdioUserPhy0_m        user_phy0;
    MdioUserPhy1_m        user_phy1;
    RlmBsrCtlMdioSel_m    mdio_sel;
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_scan_para);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Set Scan Para op_flag:0x%x interval:0x%x   \
        bitmask:0x%x scan_twice:%d mdio_ctlr_id:%d mdio_ctlr_type:%d phybitmap0:0x%x phybitmap1:0x%x\n",
                     p_scan_para->op_flag, p_scan_para->scan_interval,
                     p_scan_para->xgphy_link_bitmask, p_scan_para->xgphy_scan_twice,
                     p_scan_para->mdio_ctlr_id, p_scan_para->mdio_ctlr_type, p_scan_para->scan_phy_bitmap0, p_scan_para->scan_phy_bitmap1);

    if((SYS_DUET2_MDIO_CTLR_NUM <= p_scan_para->mdio_ctlr_id) ||
       (1 < p_scan_para->mdio_ctlr_type) ||
       (0xffff < p_scan_para->scan_interval) ||
       (0x7 < p_scan_para->xgphy_link_bitmask) ||
       (0x1 < p_scan_para->xgphy_scan_twice))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_set_phy_scan_para: out of bound! %d %d %d %d %d\n",
                         p_scan_para->mdio_ctlr_id, p_scan_para->mdio_ctlr_type, p_scan_para->scan_interval,
                         p_scan_para->xgphy_link_bitmask, p_scan_para->xgphy_scan_twice);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scan_ctrl, 0, sizeof(MdioScanCtl00_m));

    SYS_PERI_SMI_LOCK(lchip);

    mdio_ctlr_id = p_scan_para->mdio_ctlr_id;

    _sys_duet2_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, p_scan_para->mdio_ctlr_type);

    /* MdioScanCtl  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &scan_ctrl);

    /* config scan interval */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_INTERVAL_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        tmp_val32 = p_scan_para->scan_interval;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_scanIntervalLane0_f,
            &tmp_val32, &scan_ctrl);
    }
    /* config scan phy bitmap */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_PHY_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag) )
    {
        tmp_val32 = p_scan_para->scan_phy_bitmap0;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &tmp_val32, &scan_ctrl);
        tmp_val32 = p_scan_para->scan_phy_bitmap1;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &tmp_val32, &scan_ctrl);
    }
    /* for XG phy only */
    if (CTC_CHIP_MDIO_XG == p_scan_para->mdio_ctlr_type)
    {
        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_LINK_MASK_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            tmp_val32 = p_scan_para->xgphy_link_bitmask;
            DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_xgLinkBmpMaskLane0_f,
                &tmp_val32, &scan_ctrl);
        }

        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_TWICE_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            tmp_val32 = p_scan_para->xgphy_scan_twice;
            DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl00_scanXGTwiceLane0_f,
                &tmp_val32, &scan_ctrl);
        }
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &scan_ctrl);

    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_USE_PHY_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        /* config use phy or not */

        arr_phy[0] = p_scan_para->mdio_use_phy0;
        arr_phy[1] = p_scan_para->mdio_use_phy1;
        if (mdio_ctlr_id<2)
        {
            sal_memset(&user_phy0, 0, sizeof(MdioUserPhy0_m));
            cmd = DRV_IOR(MdioUserPhy0_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &user_phy0);
            SetMdioUserPhy0(A, usePhy_f, &user_phy0, arr_phy);
            cmd = DRV_IOW(MdioUserPhy0_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &user_phy0);
         }
        else
        {
            sal_memset(&user_phy1, 0, sizeof(MdioUserPhy1_m));
            cmd = DRV_IOR(MdioUserPhy1_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &user_phy1);
            SetMdioUserPhy1(A, usePhy_f, &user_phy1, arr_phy);
            cmd = DRV_IOW(MdioUserPhy1_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &user_phy1);
        }

        for (index=0; index<32; index++)
        {
            if (CTC_IS_BIT_SET(p_scan_para->mdio_use_phy0, index))
            {
                if (mdio_ctlr_id < 2)
                {
                    mdio_sel0 &= ~(1 << index);
                }
                else
                {
                    mdio_sel0 |= (1 << index);
                }
            }
            if (CTC_IS_BIT_SET(p_scan_para->mdio_use_phy1, index))
            {
                if (mdio_ctlr_id < 2)
                {
                    mdio_sel1 &= ~(1 << index);
                }
                else
                {
                    mdio_sel1 |= (1 << index);
                }
            }
        }
        /* config mdioSel */
        sal_memset(&mdio_sel, 0, sizeof(RlmBsrCtlMdioSel_m));
        cmd = DRV_IOR(RlmBsrCtlMdioSel_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &mdio_sel);
        arr_mdiosel[0] = mdio_sel0;
        arr_mdiosel[1] = mdio_sel1;
        SetRlmBsrCtlMdioSel(A, mdioSel_f, &mdio_sel, arr_mdiosel);
        cmd = DRV_IOW(RlmBsrCtlMdioSel_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &mdio_sel);
    }

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
 @brief smi interface for sgt auto scan para
*/
int32
sys_duet2_peri_get_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint32 cmd = 0;
    MdioScanCtl00_m  scan_ctrl;
    uint32 tmp_val32 = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 arr_phy[2] = {0};
    uint32 tbl_id = 0;
    uint32 type = 0;

    CTC_PTR_VALID_CHECK(p_scan_para);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Get Scan Para op_flag:0x%x mdio_ctlr_id:%d\n",
                     p_scan_para->op_flag, p_scan_para->mdio_ctlr_id);

    if(SYS_DUET2_MDIO_CTLR_NUM <= p_scan_para->mdio_ctlr_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_get_phy_scan_para: out of bound! %d\n",
                         p_scan_para->mdio_ctlr_id);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scan_ctrl, 0, sizeof(MdioScanCtl00_m));

    SYS_PERI_SMI_LOCK(lchip);

    mdio_ctlr_id = p_scan_para->mdio_ctlr_id;

    _sys_duet2_peri_get_mdio_phy_type(lchip, mdio_ctlr_id, &type);
    p_scan_para->mdio_ctlr_type = type;

    /* MdioScanCtl  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &scan_ctrl);

    /* config scan interval */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_INTERVAL_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl00_scanIntervalLane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_interval = tmp_val32;
    }
    /* config scan phy bitmap */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_PHY_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag) )
    {
        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp0Lane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_phy_bitmap0 = tmp_val32;

        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl00_mdioScanBmp1Lane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_phy_bitmap1 = tmp_val32;
    }
    /* for XG phy only */
    if (CTC_CHIP_MDIO_XG == p_scan_para->mdio_ctlr_type)
    {
        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_LINK_MASK_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl00_xgLinkBmpMaskLane0_f,
                &tmp_val32, &scan_ctrl);
            p_scan_para->xgphy_link_bitmask = tmp_val32;
        }

        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_TWICE_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl00_scanXGTwiceLane0_f,
                &tmp_val32, &scan_ctrl);
            p_scan_para->xgphy_scan_twice = tmp_val32;
        }
    }

    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_USE_PHY_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        /* get use phy or not */
        if (mdio_ctlr_id<2)
        {
            cmd = DRV_IOR(MdioUserPhy0_t, MdioUserPhy0_usePhy_f);
            DRV_IOCTL(lchip, 0, cmd, arr_phy);
         }
        else
        {
            cmd = DRV_IOR(MdioUserPhy1_t, MdioUserPhy1_usePhy_f);
            DRV_IOCTL(lchip, 0, cmd, arr_phy);
        }
        p_scan_para->mdio_use_phy0 = arr_phy[0];
        p_scan_para->mdio_use_phy1 = arr_phy[1];
    }
    SYS_PERI_SMI_UNLOCK(lchip);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Scan Para op_flag:0x%x interval:0x%x bitmask:0x%x scan_twice:%d \
        mdio_ctlr_id:%d mdio_ctlr_type:%d phybitmap0:0x%x phybitmap1:0x%x usephy0: 0x%x usephy1: 0x%x\n",
                     p_scan_para->op_flag, p_scan_para->scan_interval, p_scan_para->xgphy_link_bitmask, \
                     p_scan_para->xgphy_scan_twice, p_scan_para->mdio_ctlr_id, p_scan_para->mdio_ctlr_type, \
                     p_scan_para->scan_phy_bitmap0, p_scan_para->scan_phy_bitmap1, p_scan_para->mdio_use_phy0, p_scan_para->mdio_use_phy1);

    return CTC_E_NONE;
}

/**
 @brief mdio interface for read ge phy
*/
int32
sys_duet2_peri_read_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    MdioCmd00_m mdio_cmd;
    MdioStatus00_m mdio_status;
    uint8 mdio1_g_cmd_done = 0;
    uint32 timeout = SYS_CHIP_MAX_TIME_OUT;
    uint32 cmd = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 mdio_intf_id = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_para);
    CTC_MAX_VALUE_CHECK(p_para->bus, 7);

    /*printf("### [MDIO READ] MDIO type: %d, Lane: %d, MDIO bus: %d, PHY addr: 0x%x, PHY reg: 0x%x\n",
        type, p_para->ctl_id, p_para->bus, p_para->phy_addr, p_para->reg);*/

    if (CTC_CHIP_MDIO_XGREG_BY_GE == type)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_read_phy_reg: type (%d) invalid!\n", type);
        return CTC_E_INTR_INVALID_PARAM;
    }

    SYS_PERI_SMI_LOCK(lchip);

    sal_memset(&mdio_cmd, 0, sizeof(MdioCmd00_m));
    sal_memset(&mdio_status, 0, sizeof(MdioStatus00_m));

    mdio_ctlr_id = p_para->bus / 2;
    mdio_intf_id = p_para->bus % 2;
    _sys_duet2_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, type);

    /* #2, write ctl_id/bus/address */
    /* mdiocmd  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);

    if (CTC_CHIP_MDIO_GE == type)
    {
        SetMdioCmd00(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_1G_MDIO_READ);
        SetMdioCmd00(V, startCmdLane0_f, &mdio_cmd, 0x1);
        SetMdioCmd00(V, regAddCmdXgLane0_f, &mdio_cmd, p_para->dev_no);
        SetMdioCmd00(V, devAddCmdLane0_f, &mdio_cmd, p_para->reg);
    }
    else if (CTC_CHIP_MDIO_XG == type)
    {
        SetMdioCmd00(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_XG_MDIO_READ);
        SetMdioCmd00(V, startCmdLane0_f, &mdio_cmd, 0x0);
        SetMdioCmd00(V, regAddCmdXgLane0_f, &mdio_cmd, p_para->reg);
        SetMdioCmd00(V, devAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
    }
    SetMdioCmd00(V, intfSelCmdLane0_f, &mdio_cmd, mdio_intf_id);
    SetMdioCmd00(V, portAddCmdLane0_f, &mdio_cmd, p_para->phy_addr);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_read_phy_reg: write table fail! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    /* #3, check status done */
    /*mdiostatus*/
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_read_phy_reg: write MdioStatus fail! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    mdio1_g_cmd_done = GetMdioStatus00(V, mdioCmdDoneLane0_f, &mdio_status);
    while ((mdio1_g_cmd_done == 0) && (timeout--))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
        if (ret < 0)
        {
            SYS_PERI_SMI_UNLOCK(lchip);
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_read_phy_reg: write mdioCmdDoneLane fail! ret %d\n", ret);
            return CTC_E_INVALID_CONFIG;
        }
        mdio1_g_cmd_done = GetMdioStatus00(V, mdioCmdDoneLane0_f, &mdio_status);
    }

    if (mdio1_g_cmd_done == 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_read_phy_reg: mdio1_g_cmd_done %d\n", mdio1_g_cmd_done);
        return CTC_E_INVALID_CONFIG;
    }

    /* #4, read data */
    p_para->value = GetMdioStatus00(V, mdioReadDataLane0_f, &mdio_status);

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}


/**
 @brief mdio interface for write ge phy
*/
int32
sys_duet2_peri_write_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    MdioCmd00_m mdio_cmd;
    MdioStatus00_m mdio_status;
    uint8 mdio1_g_cmd_done = 0;
    uint32 timeout = SYS_CHIP_MAX_TIME_OUT;
    uint32 cmd = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 mdio_intf_id = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_para);
    CTC_MAX_VALUE_CHECK(p_para->bus, 7);

    /*printf("### [MDIO WRITE] MDIO type: %d, Lane: %d, MDIO bus: %d, PHY addr: 0x%x, PHY reg: 0x%x, Value: 0x%x\n",
        type, p_para->ctl_id, p_para->bus, p_para->phy_addr, p_para->reg, p_para->value);*/

    SYS_PERI_SMI_LOCK(lchip);

    sal_memset(&mdio_cmd, 0, sizeof(MdioCmd00_m));
    sal_memset(&mdio_status, 0, sizeof(MdioStatus00_m));

    mdio_ctlr_id = p_para->bus / 2;
    mdio_intf_id = p_para->bus % 2;
    _sys_duet2_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, type);

    /* #2, write ctl_id/bus/address */
    /* mdiocmd  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);

    if (CTC_CHIP_MDIO_GE == type)
    {
        SetMdioCmd00(V, startCmdLane0_f, &mdio_cmd, 0x1);
        SetMdioCmd00(V, regAddCmdXgLane0_f, &mdio_cmd, p_para->dev_no);
        SetMdioCmd00(V, devAddCmdLane0_f, &mdio_cmd, p_para->reg);
    }
    else if (CTC_CHIP_MDIO_XG == type)
    {
        SetMdioCmd00(V, startCmdLane0_f, &mdio_cmd, 0x0);
        SetMdioCmd00(V, regAddCmdXgLane0_f, &mdio_cmd, p_para->reg);
        SetMdioCmd00(V, devAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
    }

    SetMdioCmd00(V, intfSelCmdLane0_f, &mdio_cmd, mdio_intf_id);
    SetMdioCmd00(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_MDIO_WRITE);
    SetMdioCmd00(V, portAddCmdLane0_f, &mdio_cmd, p_para->phy_addr);
    SetMdioCmd00(V, dataCmdLane0_f, &mdio_cmd, p_para->value);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_write_phy_reg: MdioCmd error! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    /* #3, check status done */
    /*mdiostatus*/
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus00_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus01_t;
    }
    else if (2 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus10_t;
    }
    else if (3 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus11_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_write_phy_reg: MdioStatus error! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    mdio1_g_cmd_done = GetMdioStatus00(V, mdioCmdDoneLane0_f, &mdio_status);
    while ((mdio1_g_cmd_done == 0) && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
        if (ret < 0)
        {
            SYS_PERI_SMI_UNLOCK(lchip);
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_write_phy_reg: error ret %d\n", ret);
            return CTC_E_INVALID_CONFIG;
        }
        mdio1_g_cmd_done = GetMdioStatus00(V, mdioCmdDoneLane0_f, &mdio_status);
    }

    if (mdio1_g_cmd_done == 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_write_phy_reg: mdio1_g_cmd_done %d\n", mdio1_g_cmd_done);
        return CTC_E_INVALID_CONFIG;
    }

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_duet2_peri_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 core_freq = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((type != CTC_CHIP_MDIO_GE) && (type != CTC_CHIP_MDIO_XG))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(0xffff < ((core_freq*250)/freq))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == CTC_CHIP_MDIO_GE)
    {
        core_freq = sys_usw_datapath_get_core_clock(lchip, 1);
        field_value = (core_freq*250)/freq;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgClkMdioDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

        field_value = 1;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        field_value = 0;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }
    else
    {
        core_freq = sys_usw_datapath_get_core_clock(lchip, 1);
        field_value = (core_freq*250)/freq;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgClkMdioXgDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

        field_value = 1;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioXgDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        field_value = 0;
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioXgDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }
    return CTC_E_NONE;
}

/**
 @brief mdio interface to get mdio clock frequency
*/
int32
sys_duet2_peri_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 core_freq = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(SDK_WORK_PLATFORM == 0)
    {

    if ((type != CTC_CHIP_MDIO_GE) && (type != CTC_CHIP_MDIO_XG))
    {
        return CTC_E_INVALID_PARAM;
    }

    core_freq = sys_usw_get_core_freq(lchip, 1);

    if (type == CTC_CHIP_MDIO_GE)
    {
        /* for 1g mdio bus */
        cmd = DRV_IOR(SupMiscCfg_t, SupMiscCfg_cfgClkMdioDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    else
    {
        /* for xg mdio bus */
        cmd = DRV_IOR(SupMiscCfg_t, SupMiscCfg_cfgClkMdioXgDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    *freq = core_freq*250/field_val;

    }

    return CTC_E_NONE;
}

#define __MAC_LED_INTERFACE__
/**
 @brief mac led interface to get mac led mode cfg
*/
STATIC int32
_sys_duet2_peri_get_mac_led_mode_cfg(ctc_chip_led_mode_t mode, uint8* p_value)
{
    uint8 temp = 0;

    switch(mode)
    {
        case CTC_CHIP_RXLINK_MODE:
            temp = (SYS_CHIP_LED_ON_RX_LINK & 0x3)
                | ((SYS_CHIP_LED_BLINK_OFF & 0x3) << 2);
            break;

        case CTC_CHIP_TXLINK_MODE:
            temp = (SYS_CHIP_LED_ON_TX_LINK & 0x3)
                | ((SYS_CHIP_LED_BLINK_OFF & 0x3) << 2);
            break;

        case CTC_CHIP_RXLINK_RXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_ON_RX_LINK & 0x3)
                | ((SYS_CHIP_LED_BLINK_RX_ACT & 0x3) << 2);
            break;

        case CTC_CHIP_TXLINK_TXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_ON_TX_LINK & 0x3)
                | ((SYS_CHIP_LED_BLINK_TX_ACT & 0x3) << 2);
            break;

        case CTC_CHIP_RXLINK_BIACTIVITY_MODE:
            temp = (SYS_CHIP_LED_ON_RX_LINK & 0x3)
                | ((SYS_CHIP_LED_BLINK_ON & 0x3) << 2);
            break;

        case CTC_CHIP_TXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_OFF & 0x3)
                | ((SYS_CHIP_LED_BLINK_TX_ACT & 0x3) << 2);
            break;

        case CTC_CHIP_RXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_OFF & 0x3)
                | ((SYS_CHIP_LED_BLINK_RX_ACT & 0x3) << 2);
            break;

        case CTC_CHIP_BIACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_OFF & 0x3)
                | ((SYS_CHIP_LED_BLINK_ON & 0x3) << 2);
            break;

        case CTC_CHIP_FORCE_ON_MODE:
            temp = (SYS_CHIP_LED_FORCE_ON & 0x3)
                | ((SYS_CHIP_LED_BLINK_OFF & 0x3) << 2);
            break;

        case CTC_CHIP_FORCE_OFF_MODE:
            temp = (SYS_CHIP_LED_FORCE_OFF & 0x3)
                | ((SYS_CHIP_LED_BLINK_OFF & 0x3) << 2);
            break;

       case CTC_CHIP_FORCE_ON_TXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_ON & 0x3)
                | ((SYS_CHIP_LED_BLINK_TX_ACT & 0x3) << 2);
            break;

       case CTC_CHIP_FORCE_ON_RXACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_ON & 0x3)
                | ((SYS_CHIP_LED_BLINK_RX_ACT & 0x3) << 2);
            break;

       case CTC_CHIP_FORCE_ON_BIACTIVITY_MODE:
            temp = (SYS_CHIP_LED_FORCE_ON & 0x3)
                | ((SYS_CHIP_LED_BLINK_ON & 0x3) << 2);
            break;

        default:
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_duet2_chip_get_mac_led_mode_cfg: mode (%d) illegal!\n", mode);
            return CTC_E_INVALID_PARAM;
    }

    *p_value = temp;

    return CTC_E_NONE;
}

int32
sys_duet2_port_store_led_mode_to_lport_attr(uint8 lchip, uint8 mac_id, uint8 first_led_mode, uint8 sec_led_mode)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /*get gport from datapath*/
    lport = sys_usw_datapath_get_lport_with_mac(lchip, 0, mac_id);
    if (lport == SYS_COMMON_USELESS_MAC)
    {
        return CTC_E_INVALID_PARAM;
    }

    lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_MAC_NOT_USED;
    }

    port_attr->first_led_mode = first_led_mode;
    port_attr->sec_led_mode = sec_led_mode;

    return CTC_E_NONE;
}

/**
 @brief mac led interface
*/
int32
sys_duet2_peri_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner)
{
    uint32 cmd = 0;
    uint32 field_index = 0;
    LedCfgPortMode0_m led_mode;
    LedPolarityCfg0_m led_polarity;
    uint8 led_cfg = 0;
    uint16 lport;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_PTR_VALID_CHECK(p_led_para);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&led_mode, 0, sizeof(LedCfgPortMode0_m));
    sal_memset(&led_polarity, 0, sizeof(LedPolarityCfg0_m));

    CTC_PTR_VALID_CHECK(p_led_para);

    if (led_type >= CTC_CHIP_MAX_LED_TYPE)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_set_mac_led_mode: led_type (%d) illegal!\n", led_type);
        return CTC_E_INVALID_PARAM;
    }

    if (1 < p_led_para->ctl_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_set_mac_led_mode: ctl_id (%d) illegal!\n", p_led_para->ctl_id);
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_CHIP_FLAG_ISSET(p_led_para->op_flag, SYS_CHIP_LED_MODE_SET_OP)
        || SYS_CHIP_FLAG_ISZERO(p_led_para->op_flag))
    {
    CTC_ERROR_RETURN(_sys_duet2_peri_get_mac_led_mode_cfg(p_led_para->first_mode, &led_cfg));

    SetLedCfgPortMode0(V, primaryLedMode_f, &led_mode, led_cfg);

    if (led_type == CTC_CHIP_USING_TWO_LED)
    {
        CTC_ERROR_RETURN(_sys_duet2_peri_get_mac_led_mode_cfg(p_led_para->sec_mode, &led_cfg));
        SetLedCfgPortMode0(V, secondaryLedMode_f, &led_mode, led_cfg);
        SetLedCfgPortMode0(V, secondaryLedModeEn_f, &led_mode, 1);
    }

    if (p_led_para->ctl_id == 0)
    {
        cmd = DRV_IOW(LedCfgPortMode0_t, DRV_ENTRY_FLAG);
    }
    else if (p_led_para->ctl_id == 1)
    {
        cmd = DRV_IOW(LedCfgPortMode1_t, DRV_ENTRY_FLAG);
    }

    if (p_led_para->lport_en)
    {
        lport = p_led_para->port_id;
        MAC_LOCK;
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
        if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
        {
            MAC_UNLOCK;
            return CTC_E_INVALID_PARAM;
        }
        field_index = port_attr->mac_id;
        MAC_UNLOCK;
    }
    else
    {
        if (p_led_para->port_id >= SYS_CHIP_MAX_PHY_PORT)
        {
            return CTC_E_INVALID_PARAM;
        }
        field_index = p_led_para->port_id;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_index, cmd, &led_mode));
    if (0 == inner)
    {
        CTC_ERROR_RETURN(sys_duet2_port_store_led_mode_to_lport_attr(lchip, field_index, p_led_para->first_mode, (led_type == CTC_CHIP_USING_TWO_LED)?p_led_para->sec_mode:CTC_CHIP_MAC_LED_MODE));
    }
    }

    if (SYS_CHIP_FLAG_ISSET(p_led_para->op_flag, SYS_CHIP_LED_POLARITY_SET_OP)
        || SYS_CHIP_FLAG_ISZERO(p_led_para->op_flag))
    {
    /* set polarity for driving led 1: low driver led 0: high driver led */
        SetLedPolarityCfg0(V, polarityInv_f, &led_polarity, p_led_para->polarity);
        if (p_led_para->ctl_id == 0)
        {
            cmd = DRV_IOW(LedPolarityCfg0_t, DRV_ENTRY_FLAG);
        }
        else if (p_led_para->ctl_id == 1)
        {
            cmd = DRV_IOW(LedPolarityCfg1_t, DRV_ENTRY_FLAG);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_polarity));
    }
    return CTC_E_NONE;
}

/**
 @brief mac led interface for mac and led mapping
*/
int32
sys_duet2_peri_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map)
{
    uint32 index = 0;
    uint32 cmd = 0;

    LedCfgPortSeqMap0_m seq_map;
    LedPortRange0_m port_range;

    uint16 lport;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 mac_id;
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_led_map);
    if (0 == p_led_map->lport_en)
    {
        CTC_PTR_VALID_CHECK(p_led_map->p_mac_id);
    }
    else
    {
        CTC_PTR_VALID_CHECK(p_led_map->port_list);
    }
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac num:%d \n", p_led_map->mac_led_num);

    for (index = 0; index < p_led_map->mac_led_num; index++)
    {
        if (p_led_map->p_mac_id)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac seq %d:%d;  ", index, p_led_map->p_mac_id[index]);
        }
        else
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lport seq %d:%d;  ", index, p_led_map->port_list[index]);
        }
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "\n");

    sal_memset(&seq_map, 0, sizeof(LedCfgPortSeqMap0_m));
    sal_memset(&port_range, 0, sizeof(LedPortRange0_m));

    if ((p_led_map->mac_led_num > SYS_CHIP_MAX_PHY_PORT) || (p_led_map->mac_led_num < 1))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_duet2_peri_set_mac_led_mapping: mac_led_num (%d) illegal!\n", p_led_map->mac_led_num);
        return CTC_E_INVALID_PARAM;
    }

    for (index = 0; index < p_led_map->mac_led_num; index++)
    {

        if (p_led_map->lport_en)
        {
            lport = (*((uint16*)p_led_map->port_list + index));
            MAC_LOCK;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
            if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
            {
                MAC_UNLOCK;
                return CTC_E_INVALID_PARAM;
            }
            mac_id = port_attr->mac_id;
            MAC_UNLOCK;
        }
        else
        {
            mac_id = *((uint8*)p_led_map->p_mac_id + index);
            if (mac_id >= SYS_CHIP_MAX_PHY_PORT)
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        if (p_led_map->ctl_id == 0)
        {
            cmd = DRV_IOW(LedCfgPortSeqMap0_t, DRV_ENTRY_FLAG);
            SetLedCfgPortSeqMap0(V, macId_f, &seq_map, mac_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &seq_map));
        }
        else if (p_led_map->ctl_id == 1)
        {
            cmd = DRV_IOW(LedCfgPortSeqMap1_t, DRV_ENTRY_FLAG);
            SetLedCfgPortSeqMap0(V, macId_f, &seq_map, mac_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &seq_map));
        }
    }

    /* set MacLedPortRange */
    SetLedPortRange0(V, portStartIndex_f, &port_range, 0);
    SetLedPortRange0(V, portEndIndex_f, &port_range,
                                                    p_led_map->mac_led_num - 1);
    if (p_led_map->ctl_id == 0)
    {
        cmd = DRV_IOW(LedPortRange0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_range));
    }
    else if (p_led_map->ctl_id == 1)
    {
        cmd = DRV_IOW(LedPortRange1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_range));
    }
    return CTC_E_NONE;
}

/**
 @brief begin mac led function
*/
int32
sys_duet2_peri_set_mac_led_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    LedSampleInterval0_m led_sample;
    LedRawStatusCfg0_m raw_cfg;
    LedCfgCalCtl0_m cal_ctrl;
    uint32 field_value = (TRUE == enable) ? 1 : 0;
    uint32 refresh_interval = 0x27ffff6;
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac led Enable enable:%d\n", ((TRUE == enable) ? 1 : 0));

    sal_memset(&led_sample, 0, sizeof(LedSampleInterval0_m));
    sal_memset(&raw_cfg, 0, sizeof(LedRawStatusCfg0_m));
    sal_memset(&cal_ctrl, 0, sizeof(LedCfgCalCtl0_m));


    cmd = DRV_IOW(LedCfgCalCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cal_ctrl));
    cmd = DRV_IOW(LedCfgCalCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cal_ctrl));

    cmd = DRV_IOW(LedRawStatusCfg0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &raw_cfg));
    cmd = DRV_IOW(LedRawStatusCfg1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &raw_cfg));

    cmd = DRV_IOR(LedSampleInterval0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));
    cmd = DRV_IOR(LedSampleInterval1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));

    SetLedSampleInterval0(V, histogramEn_f, &led_sample, field_value);
    SetLedSampleInterval0(V, sampleEn_f, &led_sample, field_value);
    SetLedSampleInterval0(V, sampleInterval_f, &led_sample, 0x1039de00);

    cmd = DRV_IOW(LedSampleInterval0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));
    cmd = DRV_IOW(LedSampleInterval1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));

    cmd = DRV_IOW(LedRefreshInterval0_t, LedRefreshInterval0_refreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(LedRefreshInterval1_t, LedRefreshInterval0_refreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    cmd = DRV_IOW(LedRefreshInterval0_t, LedRefreshInterval0_refreshInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &refresh_interval));
    cmd = DRV_IOW(LedRefreshInterval1_t, LedRefreshInterval0_refreshInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &refresh_interval));

    return CTC_E_NONE;
}

/**
 @brief get mac led function
*/
int32
sys_duet2_peri_get_mac_led_en(uint8 lchip, bool* enable)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(LedRefreshInterval0_t, LedRefreshInterval0_refreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get mac led Enable enable:%d\n", field_value);

    *enable = field_value;

    return CTC_E_NONE;
}

#define __GPIO_INTERFACE__
/**
 @brief gpio interface
*/
int32
sys_duet2_peri_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode)
{
    uint32 cmd = 0;

    if ((gpio_id > SYS_CHIP_MAC_GPIO_ID)
        || ((CTC_CHIP_INPUT_MODE != mode) && (CTC_CHIP_OUTPUT_MODE != mode)))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio mode, gpio_id:%d mode:%d\n", gpio_id, mode);

    switch (gpio_id)
    {
        case 1:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio0OutEn_f);
            break;
        case 2:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio1OutEn_f);
            break;
        case 3:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio2OutEn_f);
            break;
        case 4:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio3OutEn_f);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mode));

    return CTC_E_NONE;
}

/**
 @brief gpio output
*/
int32
sys_duet2_peri_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para)
{
    uint32 cmd = 0;
    uint32 drv_out_para = out_para;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio out, gpio_id:%d out_para:%d\n", gpio_id, out_para);

    switch (gpio_id)
    {
        case 1:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio0Out_f);
            break;
        case 2:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio1Out_f);
            break;
        case 3:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio2Out_f);
            break;
        case 4:
            cmd = DRV_IOW(GpioMacroCtl_t, GpioMacroCtl_gpio3Out_f);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &drv_out_para));
    return CTC_E_NONE;
}


/**
 @brief gpio input
*/
int32
sys_duet2_peri_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (gpio_id)
    {
        case 1:
            cmd = DRV_IOR(GpioMacroMon_t, GpioMacroMon_gpio0In_f);
            break;
        case 2:
            cmd = DRV_IOR(GpioMacroMon_t, GpioMacroMon_gpio1In_f);
            break;
        case 3:
            cmd = DRV_IOR(GpioMacroMon_t, GpioMacroMon_gpio2In_f);
            break;
        case 4:
            cmd = DRV_IOR(GpioMacroMon_t, GpioMacroMon_gpio3In_f);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    *in_value = (uint8)value;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get gpio in, gpio_id:%d in_para:%d\n", gpio_id, *in_value);
    return CTC_E_NONE;
}

#define __INIT__
STATIC int32
_sys_duet2_peri_led_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    uint32 core_freq = 0;
    uint32 div = 0;

    core_freq = sys_usw_get_core_freq(lchip, 1);

    cmd = DRV_IOR(SupDskCfg_t, SupDskCfg_cfgClkDividerClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &div));

    /*
        #2, Toggle the clockOobfc divider reset
        Cfg SupDskCfg.cfgClkResetClkOobFc = 1'b1;
        Cfg SupDskCfg.cfgClkResetClkOobFc = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkResetClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkResetClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #3, Cfg interface MACLED clock Frequency=25MHz,@clockSlow=125MHz
        Cfg SupMiscCfg.cfgClkLedDiv[15:0] = 16'd50;
    */
    /*Merge bug40305, liangf, 2016-09-01*/
    field_value = (((core_freq * 2) / (div + 1)) / (SYS_CHIP_MAC_LED_CLK_TWOFOLD));
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #4, Toggle the clockLed divider reset
        Cfg SupMiscCfg.cfgResetClkLedDiv = 1'b1;
        Cfg SupMiscCfg.cfgResetClkLedDiv = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0x0d1cef00;
    cmd = DRV_IOW(LedBlinkCfg0_t, LedBlinkCfg0_blinkOffInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(LedBlinkCfg0_t, LedBlinkCfg0_blinkOnInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0x0d1cef00;
    cmd = DRV_IOW(LedBlinkCfg1_t, LedBlinkCfg1_blinkOffInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(LedBlinkCfg1_t, LedBlinkCfg1_blinkOnInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_peri_i2c_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    /*
        #1, Cfg I2C interface MSCL clock Frequency=400KHz, supper super freq @100MHz
    */

    field_value = 0xFA0;
    cmd = DRV_IOW(I2CMasterCfg0_t, I2CMasterCfg0_clkDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0xFA0;
    cmd = DRV_IOW(I2CMasterCfg1_t, I2CMasterCfg0_clkDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 1;
    cmd = DRV_IOW(I2CMasterInit0_t, I2CMasterInit0_init_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 1;
    cmd = DRV_IOW(I2CMasterInit1_t, I2CMasterInit1_init_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    return CTC_E_NONE;
}

int32
sys_duet2_peri_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 core_freq = 0;


    core_freq = sys_usw_datapath_get_core_clock(lchip, 1);

    /*
        #1, Cfg clockSlow Frequency=125MHz at core PLLOUTB Frequency=500MHz
        Cfg SupDskCfg.cfgClkDividerClkOobFc[4:0] = 5'd3;
    */
    field_value = (core_freq / SYS_REG_CLKSLOW_FREQ) - 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkDividerClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    /*
        #2, Toggle the clockOobfc divider reset
        Cfg SupDskCfg.cfgClkResetClkOobFc = 1'b1;
        Cfg SupDskCfg.cfgClkResetClkOobFc = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkResetClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkResetClkOobFc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));


    CTC_ERROR_RETURN(_sys_duet2_peri_led_init(lchip));
    CTC_ERROR_RETURN(_sys_duet2_peri_i2c_init(lchip));

    /* CTC_ERROR_RETURN(sys_usw_mdio_init(lchip)); */

    return CTC_E_NONE;
}

int32
sys_duet2_peri_mdio_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    ctc_chip_phy_scan_ctrl_t scan_ctl;
    uint16 mac_id = 0;
    MdioChanMap0_m mdio_map0;
    MdioChanMap1_m mdio_map1;
    uint32 value = 0;
    uint32 i = 0;

    sal_memset(&scan_ctl , 0, sizeof(ctc_chip_phy_scan_ctrl_t));

    /*
        #3, Cfg SMI 1G interface MDC clock Frequency=2.5MHz,@clockSlow=125MHz
    */
    field_value = 50;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgClkMdioDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #4, Toggle the clockMdio divider reset
    */
    field_value = 1;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #P2.Configure MDCXG clock
        #1, Cfg SMI 1G interface MDCXG clock Frequency=25MHz,@clockSlow=125MHz
    */

    field_value = 50;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgClkMdioXgDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #2, Toggle the clockMdio divider reset
    */
    field_value = 1;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioXgDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgResetClkMdioXgDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        reset MDIO module in slice0 and slice1
    */
    field_value = 1;
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreMdio0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreMdio1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreMdio0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreMdio1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 1;
    cmd = DRV_IOW((MdioInit0_t), MdioInit0_init_f);
    DRV_IOCTL(lchip, 0, cmd, &field_value);

    field_value = 1;
    cmd = DRV_IOW((MdioInit1_t), MdioInit1_init_f);
    DRV_IOCTL(lchip, 0, cmd, &field_value);

    /*
        Init mdio scan. For no phy on demo board, just set interval
    */
    for (i = 0; i < SYS_DUET2_MDIO_CTLR_NUM; i++)
    {
        scan_ctl.op_flag = CTC_CHIP_INTERVAL_OP;
        scan_ctl.scan_interval = 100;
        scan_ctl.mdio_ctlr_id  = i;
        CTC_ERROR_RETURN(sys_duet2_peri_set_phy_scan_para(lchip, &scan_ctl));
    }

    CTC_ERROR_RETURN(sys_duet2_peri_set_phy_scan_cfg(lchip));

    /*cfg mdio channel map*/
    for (mac_id = 0; mac_id < 64; mac_id++)
    {
        sal_memset(&mdio_map0, 0, sizeof(MdioChanMap0_m));
        sal_memset(&mdio_map1, 0, sizeof(MdioChanMap1_m));
        {
            value = mac_id;
        }

        SetMdioChanMap0(V, chanId_f, &mdio_map0, value);
        cmd = DRV_IOW((MdioChanMap0_t), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &mdio_map0);

        SetMdioChanMap1(V, chanId_f, &mdio_map1, value);
        cmd = DRV_IOW((MdioChanMap1_t), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &mdio_map1);
    }

    sys_usw_peri_set_phy_scan_en(lchip, TRUE);

    return CTC_E_NONE;
}

#define __OTHER_INTERFACE__

int32
sys_duet2_peri_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 timeout = 20;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_value);
    SYS_PERI_INIT_CHECK(lchip);
    if ((CTC_CHIP_SENSOR_TEMP != type) && (CTC_CHIP_SENSOR_VOLTAGE != type))
    {
        return CTC_E_INVALID_PARAM;
    }

    field_val =  0;
    cmd = DRV_IOW(TvSensorCfg_t, TvSensorCfg_cfgSleepSen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* set sensor type*/
    field_val =  (CTC_CHIP_SENSOR_TEMP == type)?0:1;
    cmd = DRV_IOW(TvSensorCfg_t, TvSensorCfg_cfgSenSv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* reset sensor */
    field_val =  1;
    cmd = DRV_IOW(TvSensorCfg_t, TvSensorCfg_cfgResetSen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    sal_task_sleep(10);
    field_val =  0;
    cmd = DRV_IOW(TvSensorCfg_t, TvSensorCfg_cfgResetSen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    sal_task_sleep(3);

    cmd = DRV_IOR(TvSensorMon_t, TvSensorMon_monOutputValidSen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    while ((field_val == 0) && (timeout--))
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        sal_task_sleep(1);
    }

    if (field_val)
    {
        cmd = DRV_IOR(TvSensorMon_t, TvSensorMon_monOutputSen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    else
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sensor is not Ready!! \n");
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_CHIP_SENSOR_TEMP == type)
    {
        if ((field_val & 0xff) >= 118)
        {
            *p_value = (field_val & 0xff) - 118;
        }
        else
        {
            *p_value = 118 - (field_val & 0xff) + (1 << 31);
        }
    }
    else
    {
        *p_value =  (field_val*38 +5136)/10;
    }

    return CTC_E_NONE;
}

int32
sys_duet2_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void* p_data)
{
    CTC_INTERRUPT_EVENT_FUNC event_cb;
    uint32 cmd;
    uint32 cmd_w = 0;
    uint32 table_id;
    uint32 phy_enable[(SYS_USW_MAX_PHY_PORT+31)/32] = {0};
    uint8  loop = 0;
    uint8  gchip_id = 0;
    uint8  phy_addr = 0;
    uint8  bus = 0;
    uint8  mdio_ctlr_id = 0;
    uint32 value_a = 0;
    uint32 gport = 0;
    int32 ret = 0;

    ctc_port_phy_status_t   phy_link_status;
    sys_intr_type_t    intr_type;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PHY_STATUS_CHANGE, &event_cb));
    switch(intr)
    {
        case SYS_INTR_FUNC_MDIO_CHANGE_0:
            table_id = MdioLinkChange00_t;
            mdio_ctlr_id = 0;
            break;
        case SYS_INTR_FUNC_MDIO_CHANGE_1:
            table_id = MdioLinkChange01_t;
            mdio_ctlr_id = 1;
            break;
        case SYS_INTR_FUNC_MDIO_XG_CHANGE_0:
            table_id = MdioLinkChange10_t;
            mdio_ctlr_id = 2;
            break;
        case SYS_INTR_FUNC_MDIO_XG_CHANGE_1:
            table_id = MdioLinkChange11_t;
            mdio_ctlr_id = 3;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /*read which mac use phy*/
    cmd = DRV_IOR(MdioUserPhy0_t+(mdio_ctlr_id>=2 ? 1:0), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, phy_enable);

    sal_memset(&intr_type, 0, sizeof(intr_type));
    intr_type.intr = intr;
    intr_type.sub_intr = INVG;
    intr_type.low_intr = INVG;
    CTC_ERROR_RETURN(sys_usw_interrupt_set_en(lchip, &intr_type, 0));

    cmd = DRV_IOR(table_id, MdioLinkChange00_changeStatus_f);
    cmd_w = DRV_IOW(table_id, MdioLinkChange00_changeStatus_f);
    sal_memset(&phy_link_status, 0, sizeof(phy_link_status));

    for(loop=0; loop < SYS_USW_MAX_PHY_PORT; loop++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &value_a), ret, end_proc);

        if(!value_a)
        {
            continue;
        }

        if (loop >= 32)
        {
            bus = mdio_ctlr_id*2 + 1;
            phy_addr = loop - 32;
        }
        else
        {
            bus = mdio_ctlr_id*2;
            phy_addr = loop;
        }
        CTC_ERROR_GOTO(sys_usw_peri_get_gport_by_mdio_para(lchip, bus, phy_addr, &gport), ret, end_proc);

        if(CTC_BMP_ISSET(phy_enable, SYS_GET_MAC_ID(lchip, gport)))
        {
            /*clear status*/
            value_a = 0;
            DRV_IOCTL(lchip, loop, cmd_w, &value_a);

            phy_link_status.gport = gport;
            phy_link_status.bus = bus;
            phy_link_status.phy_addr = phy_addr;
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport 0x%x, mdio bus %d, phy address: 0x%x\n",
                                                   phy_link_status.gport,phy_link_status.bus, phy_link_status.phy_addr);
            if(event_cb)
            {
                event_cb(gchip_id, &phy_link_status);
            }
        }
    }
end_proc:
    sys_usw_interrupt_set_en(lchip, &intr_type, 1);
    return ret;
}


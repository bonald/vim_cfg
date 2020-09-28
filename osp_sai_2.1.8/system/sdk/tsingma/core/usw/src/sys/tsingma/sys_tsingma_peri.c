/**
 @file sys_tsingma_peri.c

 @date 2018-03-06

 @version v1.0

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
#include "sys_tsingma_peri.h"
#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
 * Global and static
 *
 *****************************************************************************/
#define SYS_TSINGMA_MDIO_CTLR_NUM 2
#define SYS_TSINGMA_TEMP_TABLE_NUM 166
#define SYS_TSINGMA_SENSOR_TIMEOUT 1000

extern sys_peri_master_t* p_usw_peri_master[];

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
#define __MDIO_INTERFACE__

/* called by MDIO scan init and dynamic switch */
int32
sys_tsingma_peri_set_phy_scan_cfg(uint8 lchip)
{
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 mac_id = 0;
    uint32 use_phy0 = 0;
    uint32 use_phy1 = 0;
    uint8 mdio_bus  = 0;
    uint32 cmd = 0;
    uint32 arr_phy[2] = {0};
    ctc_chip_phy_mapping_para_t* p_phy_mapping = NULL;
    MdioUserPhy_m        user_phy;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 field_value = 0;
    uint8 phy_addr = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;
    uint32 phy_bitmap0[SYS_CHIP_MAX_MDIO_BUS] = {0};
    uint32 phy_bitmap1[SYS_CHIP_MAX_MDIO_BUS] = {0};
    MdioScanCtl0_m mdio_scan;

    sal_memset(&user_phy , 0, sizeof(MdioUserPhy_m));

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
                cmd = DRV_IOW(MdioPortIdMap0_t, MdioPortIdMap0_macId_f);
            }
            else if (1 == mdio_bus)
            {
                cmd = DRV_IOW(MdioPortIdMap1_t, MdioPortIdMap1_macId_f);
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
            }
            else
            {
                use_phy1  |= (1 << (mac_id - 32));
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
    cmd = DRV_IOR(MdioUserPhy_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy);
    SetMdioUserPhyTM(A, usePhy_f, &user_phy, arr_phy);
    cmd = DRV_IOW(MdioUserPhy_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &user_phy);

    /* config mdio phy bitmap */
    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl0_m));
    tbl_id = MdioScanCtl0_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp0Lane0_f,
            &phy_bitmap0[0], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp1Lane0_f,
            &phy_bitmap1[0], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);

    sal_memset(&mdio_scan, 0, sizeof(MdioScanCtl0_m));
    tbl_id = MdioScanCtl1_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp0Lane0_f,
            &phy_bitmap0[1], &mdio_scan);
    DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp1Lane0_f,
            &phy_bitmap1[1], &mdio_scan);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mdio_scan);
out:
    if (p_phy_mapping)
    {
        mem_free(p_phy_mapping);
    }

    return ret;
}

STATIC int32
_sys_tsingma_peri_set_mdio_phy_type(uint8 lchip, uint8 ctlr_id, uint32 type)
{
    uint32 cmd = 0;
    uint32 mdio_phy_type = 0;
    uint32 tbl_id = 0;

    if (0 == ctlr_id)
    {
        tbl_id = MdioCfg0_t;
    }
    else if (1 == ctlr_id)
    {
        tbl_id = MdioCfg1_t;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, MdioCfg0_mdioIsXgModeLane0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mdio_phy_type));

    if (mdio_phy_type != type)
    {
        cmd = DRV_IOW(tbl_id, MdioCfg0_mdioIsXgModeLane0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &type));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_peri_get_mdio_phy_type(uint8 lchip, uint8 ctlr_id, uint32* type)
{
    uint32 cmd = 0;
    uint32 mdio_phy_type = 0;
    uint32 tbl_id = 0;

    if (0 == ctlr_id)
    {
        tbl_id = MdioCfg0_t;
    }
    else if (1 == ctlr_id)
    {
        tbl_id = MdioCfg1_t;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOR(tbl_id, MdioCfg0_mdioIsXgModeLane0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mdio_phy_type));

    *type = mdio_phy_type;

    return CTC_E_NONE;
}

/**
 @brief smi interface for set auto scan para
*/
int32
sys_tsingma_peri_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint32 cmd = 0;
    MdioScanCtl0_m  scan_ctrl;
    uint32 tmp_val32 = 0;
    uint8 mdio_ctlr_id = 0;
    MdioUserPhy_m        user_phy;
    uint32 arr_phy[2] = {0};
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_scan_para);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Set Scan Para op_flag:0x%x interval:0x%x   \
        bitmask:0x%x scan_twice:%d mdio_ctlr_id:%d mdio_ctlr_type:%d phybitmap0:0x%x phybitmap1:0x%x\n",
                     p_scan_para->op_flag, p_scan_para->scan_interval,
                     p_scan_para->xgphy_link_bitmask, p_scan_para->xgphy_scan_twice,
                     p_scan_para->mdio_ctlr_id, p_scan_para->mdio_ctlr_type, p_scan_para->scan_phy_bitmap0, p_scan_para->scan_phy_bitmap1);

    if((SYS_TSINGMA_MDIO_CTLR_NUM <= p_scan_para->mdio_ctlr_id) ||
       (1 < p_scan_para->mdio_ctlr_type) ||
       (0xffff < p_scan_para->scan_interval) ||
       (0x7 < p_scan_para->xgphy_link_bitmask) ||
       (0x1 < p_scan_para->xgphy_scan_twice))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_set_phy_scan_para: out of bound! %d %d %d %d %d\n",
                         p_scan_para->mdio_ctlr_id, p_scan_para->mdio_ctlr_type, p_scan_para->scan_interval,
                         p_scan_para->xgphy_link_bitmask, p_scan_para->xgphy_scan_twice);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scan_ctrl, 0, sizeof(MdioScanCtl0_m));

    SYS_PERI_SMI_LOCK(lchip);

    mdio_ctlr_id = p_scan_para->mdio_ctlr_id;

    _sys_tsingma_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, p_scan_para->mdio_ctlr_type);

    /* MdioScanCtl  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl1_t;
    }
    else
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }
    /* MdioScanCtl  */
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &scan_ctrl);

    /* config scan interval */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_INTERVAL_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        tmp_val32 = p_scan_para->scan_interval;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_scanIntervalLane0_f,
            &tmp_val32, &scan_ctrl);
    }
    /* config scan phy bitmap */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_PHY_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag) )
    {
        tmp_val32 = p_scan_para->scan_phy_bitmap0;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp0Lane0_f,
            &tmp_val32, &scan_ctrl);
        tmp_val32 = p_scan_para->scan_phy_bitmap1;
        DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp1Lane0_f,
            &tmp_val32, &scan_ctrl);
    }
    /* for XG phy only */
    if (CTC_CHIP_MDIO_XG == p_scan_para->mdio_ctlr_type)
    {
        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_LINK_MASK_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            tmp_val32 = p_scan_para->xgphy_link_bitmask;
            DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_xgLinkBmpMaskLane0_f,
                &tmp_val32, &scan_ctrl);
        }

        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_TWICE_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            tmp_val32 = p_scan_para->xgphy_scan_twice;
            DRV_IOW_FIELD(lchip, tbl_id, MdioScanCtl0_scanXGTwiceLane0_f,
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

        sal_memset(&user_phy, 0, sizeof(MdioUserPhy_m));
        cmd = DRV_IOR(MdioUserPhy_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &user_phy);
        SetMdioUserPhyTM(A, usePhy_f, &user_phy, arr_phy);
        cmd = DRV_IOW(MdioUserPhy_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &user_phy);
    }

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_tsingma_peri_get_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint32 cmd = 0;
    MdioScanCtl0_m  scan_ctrl;
    uint32 tmp_val32 = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 arr_phy[2] = {0};
    uint32 tbl_id = 0;
    uint32 type = 0;

    CTC_PTR_VALID_CHECK(p_scan_para);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Get Scan Para op_flag:0x%x mdio_ctlr_id:%d\n",
                     p_scan_para->op_flag, p_scan_para->mdio_ctlr_id);

    if(SYS_TSINGMA_MDIO_CTLR_NUM <= p_scan_para->mdio_ctlr_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_get_phy_scan_para: out of bound! %d\n",
                         p_scan_para->mdio_ctlr_id);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scan_ctrl, 0, sizeof(MdioScanCtl0_m));

    SYS_PERI_SMI_LOCK(lchip);

    mdio_ctlr_id = p_scan_para->mdio_ctlr_id;

    _sys_tsingma_peri_get_mdio_phy_type(lchip, mdio_ctlr_id, &type);
    p_scan_para->mdio_ctlr_type = type;

    /* MdioScanCtl  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioScanCtl1_t;
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
        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl0_scanIntervalLane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_interval = tmp_val32;
    }
    /* config scan phy bitmap */
    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_PHY_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag) )
    {
        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp0Lane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_phy_bitmap0 = tmp_val32;

        DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl0_mdioScanBmp1Lane0_f,
            &tmp_val32, &scan_ctrl);
        p_scan_para->scan_phy_bitmap1 = tmp_val32;
    }
    /* for XG phy only */
    if (CTC_CHIP_MDIO_XG == p_scan_para->mdio_ctlr_type)
    {
        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_LINK_MASK_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl0_xgLinkBmpMaskLane0_f,
                &tmp_val32, &scan_ctrl);
            p_scan_para->xgphy_link_bitmask = tmp_val32;
        }

        if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_XG_TWICE_OP)
            || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
        {
            DRV_IOR_FIELD(lchip, tbl_id, MdioScanCtl0_scanXGTwiceLane0_f,
                &tmp_val32, &scan_ctrl);
            p_scan_para->xgphy_scan_twice = tmp_val32;
        }
    }

    if (SYS_CHIP_FLAG_ISSET(p_scan_para->op_flag, CTC_CHIP_USE_PHY_OP)
        || SYS_CHIP_FLAG_ISZERO(p_scan_para->op_flag))
    {
        /* get use phy or not */
        cmd = DRV_IOR(MdioUserPhy_t, MdioUserPhy_usePhy_f);
        DRV_IOCTL(lchip, 0, cmd, arr_phy);

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
sys_tsingma_peri_read_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    MdioCmd0_m mdio_cmd;
    MdioStatus0_m mdio_status;
    uint8 mdio1_g_cmd_done = 0;
    uint32 timeout = SYS_CHIP_MAX_TIME_OUT;
    uint32 cmd = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 mdio_intf_id = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_para);

    /*printf("### [MDIO READ] MDIO type: %d, Lane: %d, MDIO bus: %d, PHY addr: 0x%x, PHY reg: 0x%x\n",
        type, p_para->ctl_id, p_para->bus, p_para->phy_addr, p_para->reg);*/

    if (CTC_CHIP_MDIO_XGREG_BY_GE == type)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_tsingma_peri_read_phy_reg: type (%d) invalid!\n", type);
        return CTC_E_INTR_INVALID_PARAM;
    }

    SYS_PERI_SMI_LOCK(lchip);

    sal_memset(&mdio_cmd, 0, sizeof(MdioCmd0_m));
    sal_memset(&mdio_status, 0, sizeof(MdioStatus0_m));

    mdio_ctlr_id = p_para->bus / 2;
    mdio_intf_id = p_para->bus % 2;
    _sys_tsingma_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, type);

    /* #2, write ctl_id/bus/address */
    /* mdiocmd  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd1_t;
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
        SetMdioCmd0(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_1G_MDIO_READ);
        SetMdioCmd0(V, startCmdLane0_f, &mdio_cmd, 0x1);
        SetMdioCmd0(V, regAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
        SetMdioCmd0(V, devAddCmdLane0_f, &mdio_cmd, p_para->reg);
    }
    else if (CTC_CHIP_MDIO_XG == type)
    {
        SetMdioCmd0(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_XG_MDIO_READ);
        SetMdioCmd0(V, startCmdLane0_f, &mdio_cmd, 0x0);
        SetMdioCmd0(V, regAddCmdLane0_f, &mdio_cmd, p_para->reg);
        SetMdioCmd0(V, devAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
    }
    SetMdioCmd0(V, intfSelCmdLane0_f, &mdio_cmd, mdio_intf_id);
    SetMdioCmd0(V, portAddCmdLane0_f, &mdio_cmd, p_para->phy_addr);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_read_phy_reg: write table fail! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    /* #3, check status done */
    /*mdiostatus*/
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus1_t;
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
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_read_phy_reg: write MdioStatus fail! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    mdio1_g_cmd_done = GetMdioStatus0(V, mdioCmdDoneLane0_f, &mdio_status);
    while ((mdio1_g_cmd_done == 0) && (timeout--))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
        if (ret < 0)
        {
            SYS_PERI_SMI_UNLOCK(lchip);
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_read_phy_reg: write mdioCmdDoneLane fail! ret %d\n", ret);
            return CTC_E_INVALID_CONFIG;
        }
        mdio1_g_cmd_done = GetMdioStatus0(V, mdioCmdDoneLane0_f, &mdio_status);
    }

    if (mdio1_g_cmd_done == 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_read_phy_reg: mdio1_g_cmd_done %d\n", mdio1_g_cmd_done);
        return CTC_E_INVALID_CONFIG;
    }

    /* #4, read data */
    p_para->value = GetMdioStatus0(V, mdioReadDataLane0_f, &mdio_status);

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
 @brief mdio interface for write ge phy
*/
int32
sys_tsingma_peri_write_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    MdioCmd0_m mdio_cmd;
    MdioStatus0_m mdio_status;
    uint8 mdio1_g_cmd_done = 0;
    uint32 timeout = SYS_CHIP_MAX_TIME_OUT;
    uint32 cmd = 0;
    uint8 mdio_ctlr_id = 0;
    uint32 mdio_intf_id = 0;
    int32 ret = 0;
    uint32 tbl_id = 0;

    CTC_PTR_VALID_CHECK(p_para);

    /*printf("### [MDIO WRITE] MDIO type: %d, Lane: %d, MDIO bus: %d, PHY addr: 0x%x, PHY reg: 0x%x, Value: 0x%x\n",
        type, p_para->ctl_id, p_para->bus, p_para->phy_addr, p_para->reg, p_para->value);*/

    SYS_PERI_SMI_LOCK(lchip);

    sal_memset(&mdio_cmd, 0, sizeof(MdioCmd0_m));
    sal_memset(&mdio_status, 0, sizeof(MdioStatus0_m));

    mdio_ctlr_id = p_para->bus / 2;
    mdio_intf_id = p_para->bus % 2;
    _sys_tsingma_peri_set_mdio_phy_type(lchip, mdio_ctlr_id, type);

    /* #2, write ctl_id/bus/address */
    /* mdiocmd  */
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioCmd1_t;
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
        SetMdioCmd0(V, startCmdLane0_f, &mdio_cmd, 0x1);
        SetMdioCmd0(V, regAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
        SetMdioCmd0(V, devAddCmdLane0_f, &mdio_cmd, p_para->reg);
    }
    else if (CTC_CHIP_MDIO_XG == type)
    {
        SetMdioCmd0(V, startCmdLane0_f, &mdio_cmd, 0x0);
        SetMdioCmd0(V, regAddCmdLane0_f, &mdio_cmd, p_para->reg);
        SetMdioCmd0(V, devAddCmdLane0_f, &mdio_cmd, p_para->dev_no);
    }

    SetMdioCmd0(V, intfSelCmdLane0_f, &mdio_cmd, mdio_intf_id);
    SetMdioCmd0(V, opCodeCmdLane0_f, &mdio_cmd, SYS_CHIP_MDIO_WRITE);
    SetMdioCmd0(V, portAddCmdLane0_f, &mdio_cmd, p_para->phy_addr);
    SetMdioCmd0(V, dataCmdLane0_f, &mdio_cmd, p_para->value);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mdio_cmd);
    if (ret < 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_write_phy_reg: MdioCmd error! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    /* #3, check status done */
    /*mdiostatus*/
    if (0 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus0_t;
    }
    else if (1 == mdio_ctlr_id)
    {
        tbl_id = MdioStatus1_t;
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
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_write_phy_reg: MdioStatus error! mdio_ctlr_id %d, ret %d\n", mdio_ctlr_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    mdio1_g_cmd_done = GetMdioStatus0(V, mdioCmdDoneLane0_f, &mdio_status);
    while ((mdio1_g_cmd_done == 0) && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &mdio_status);
        if (ret < 0)
        {
            SYS_PERI_SMI_UNLOCK(lchip);
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_write_phy_reg: error ret %d\n", ret);
            return CTC_E_INVALID_CONFIG;
        }
        mdio1_g_cmd_done = GetMdioStatus0(V, mdioCmdDoneLane0_f, &mdio_status);
    }

    if (mdio1_g_cmd_done == 0)
    {
        SYS_PERI_SMI_UNLOCK(lchip);
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_write_phy_reg: mdio1_g_cmd_done %d\n", mdio1_g_cmd_done);
        return CTC_E_INVALID_CONFIG;
    }

    SYS_PERI_SMI_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
 @brief mdio interface to set mdio clock frequency
*/
int32
sys_tsingma_peri_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 core_freq = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((type != CTC_CHIP_MDIO_GE) && (type != CTC_CHIP_MDIO_XG))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(0xfff < ((core_freq*1000)/freq))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == CTC_CHIP_MDIO_GE)
    {
        core_freq = sys_usw_get_core_freq(lchip, 0);
        field_value = (core_freq*1000)/freq;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkMdioADiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        field_value = 1;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioADiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        field_value = 0;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioADiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }
    else
    {
        core_freq = sys_usw_get_core_freq(lchip, 0);
        field_value = (core_freq*1000)/freq;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkMdioBDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

        field_value = 1;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioBDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        field_value = 0;
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioBDiv_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }
    return CTC_E_NONE;
}

/**
 @brief mdio interface to get mdio clock frequency
*/
int32
sys_tsingma_peri_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 core_freq = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((type != CTC_CHIP_MDIO_GE) && (type != CTC_CHIP_MDIO_XG))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(SDK_WORK_PLATFORM == 0)
    {
        core_freq = sys_usw_get_core_freq(lchip, 0);

        if (type == CTC_CHIP_MDIO_GE)
        {
            /* for 1g mdio bus */
            cmd = DRV_IOR(SupDskCfg_t, SupDskCfg_cfgClkMdioADiv_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            /* for xg mdio bus */
            cmd = DRV_IOR(SupDskCfg_t, SupDskCfg_cfgClkMdioBDiv_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }

        *freq = core_freq*1000/field_val;
    }

    return CTC_E_NONE;
}

#define __MAC_LED_INTERFACE__

/**
 @brief mac led interface to get mac led mode cfg
*/
STATIC int32
_sys_tsingma_peri_get_mac_led_mode_cfg(ctc_chip_led_mode_t mode, uint8* p_value)
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
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_tsingma_peri_get_mac_led_mode_cfg: mode (%d) illegal!\n", mode);
            return CTC_E_INVALID_PARAM;
    }

    *p_value = temp;

    return CTC_E_NONE;
}


/**
 @brief mac led interface
*/
int32
sys_tsingma_peri_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner)
{
    uint32 cmd = 0;
    uint32 field_index = 0;
    LedCfgPortMode_m led_mode;
    LedPolarityCfg_m led_polarity;
    uint8 led_cfg = 0;
    uint16 lport;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_PTR_VALID_CHECK(p_led_para);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&led_mode, 0, sizeof(LedCfgPortMode_m));
    sal_memset(&led_polarity, 0, sizeof(LedPolarityCfg_m));

    CTC_PTR_VALID_CHECK(p_led_para);

    if (led_type >= CTC_CHIP_MAX_LED_TYPE)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_set_mac_led_mode: led_type (%d) illegal!\n", led_type);
        return CTC_E_INVALID_PARAM;
    }

    if (0 != p_led_para->ctl_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_set_mac_led_mode: ctl_id (%d) illegal!\n", p_led_para->ctl_id);
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_CHIP_FLAG_ISSET(p_led_para->op_flag, SYS_CHIP_LED_MODE_SET_OP)
        || SYS_CHIP_FLAG_ISZERO(p_led_para->op_flag))
    {
    CTC_ERROR_RETURN(_sys_tsingma_peri_get_mac_led_mode_cfg(p_led_para->first_mode, &led_cfg));

    SetLedCfgPortMode(V, primaryLedMode_f, &led_mode, led_cfg);

    if (led_type == CTC_CHIP_USING_TWO_LED)
    {
        CTC_ERROR_RETURN(_sys_tsingma_peri_get_mac_led_mode_cfg(p_led_para->sec_mode, &led_cfg));
        SetLedCfgPortMode(V, secondaryLedMode_f, &led_mode, led_cfg);
        SetLedCfgPortMode(V, secondaryLedModeEn_f, &led_mode, 1);
    }

    cmd = DRV_IOW(LedCfgPortMode_t, DRV_ENTRY_FLAG);

    if (p_led_para->lport_en)
    {
        lport = CTC_MAP_GPORT_TO_LPORT(p_led_para->port_id);
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
    }

    if (SYS_CHIP_FLAG_ISSET(p_led_para->op_flag, SYS_CHIP_LED_POLARITY_SET_OP)
        || SYS_CHIP_FLAG_ISZERO(p_led_para->op_flag))
    {
        /* set polarity for driving led 1: low driver led 0: high driver led */
        SetLedPolarityCfg(V, polarityInv_f, &led_polarity, p_led_para->polarity);
        cmd = DRV_IOW(LedPolarityCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_polarity));
    }
    return CTC_E_NONE;
}

/**
 @brief mac led interface for mac and led mapping
*/
int32
sys_tsingma_peri_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map)
{
    uint32 index = 0;
    uint32 cmd = 0;
    LedCfgPortSeqMap_m seq_map;
    LedPortRange_m port_range;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 mac_id = 0;

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

    sal_memset(&seq_map, 0, sizeof(LedCfgPortSeqMap_m));
    sal_memset(&port_range, 0, sizeof(LedPortRange_m));

    if ((p_led_map->mac_led_num > SYS_CHIP_MAX_PHY_PORT) || (p_led_map->mac_led_num < 1))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_set_mac_led_mapping: mac_led_num (%d) illegal!\n", p_led_map->mac_led_num);
        return CTC_E_INVALID_PARAM;
    }

    if (0 != p_led_map->ctl_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_tsingma_peri_set_mac_led_mode: ctl_id (%d) illegal!\n", p_led_map->ctl_id);
        return CTC_E_INVALID_PARAM;
    }

    for (index = 0; index < p_led_map->mac_led_num; index++)
    {

        if (p_led_map->lport_en)
        {
            lport = (*((uint16*)p_led_map->port_list+ index));
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

        cmd = DRV_IOW(LedCfgPortSeqMap_t, DRV_ENTRY_FLAG);
        SetLedCfgPortSeqMap(V, macId_f, &seq_map, mac_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &seq_map));
    }

    /* set MacLedPortRange */
    SetLedPortRange(V, portStartIndex_f, &port_range, 0);
    SetLedPortRange(V, portEndIndex_f, &port_range, p_led_map->mac_led_num - 1);

    cmd = DRV_IOW(LedPortRange_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_range));

    return CTC_E_NONE;
}

/**
 @brief begin mac led function
*/
int32
sys_tsingma_peri_set_mac_led_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    LedSampleInterval_m led_sample;
    LedRawStatusCfg_m raw_cfg;
    LedCfgCalCtl_m cal_ctrl;
    uint32 field_value = (TRUE == enable) ? 1 : 0;
    uint32 refresh_interval = 0x27ffff6;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac led Enable enable:%d\n", ((TRUE == enable) ? 1 : 0));

    sal_memset(&led_sample, 0, sizeof(LedSampleInterval_m));
    sal_memset(&raw_cfg, 0, sizeof(LedRawStatusCfg_m));
    sal_memset(&cal_ctrl, 0, sizeof(LedCfgCalCtl_m));


    cmd = DRV_IOW(LedCfgCalCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cal_ctrl));

    cmd = DRV_IOW(LedRawStatusCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &raw_cfg));

    cmd = DRV_IOR(LedSampleInterval_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));

    SetLedSampleInterval(V, histogramEn_f, &led_sample, field_value);
    SetLedSampleInterval(V, sampleEn_f, &led_sample, field_value);
    SetLedSampleInterval(V, sampleInterval_f, &led_sample, 0x1039de00);

    cmd = DRV_IOW(LedSampleInterval_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &led_sample));

    cmd = DRV_IOW(LedRefreshInterval_t, LedRefreshInterval0_refreshEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    cmd = DRV_IOW(LedRefreshInterval_t, LedRefreshInterval0_refreshInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &refresh_interval));

    return CTC_E_NONE;
}

/**
 @brief get mac led function
*/
int32
sys_tsingma_peri_get_mac_led_en(uint8 lchip, bool* enable)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    CTC_PTR_VALID_CHECK(enable);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(LedRefreshInterval_t, LedRefreshInterval_refreshEn_f);
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
sys_tsingma_peri_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode)
{
    uint32 cmd = 0;
    uint32 tbl_out_id = 0;
    uint32 tbl_intr_id = 0;
    uint32 tbl_intr_level_id = 0;
    uint32 field_out_id = 0;
    uint32 field_intr_id = 0;
    uint32 field_intr_level_id = 0;
    uint32 field_out_val = 0;
    uint32 field_intr_val = 0;
    uint32 field_intr_level_val = 0;
    uint8 gpio_act_id = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio mode, gpio_id:%d mode:%d\n", gpio_id, mode);

    if (gpio_act_id >= SYS_PERI_HS_MAX_GPIO_ID)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "gpio_id:%d over max gpio id\n", gpio_id);
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_PERI_GPIO_IS_HS(gpio_id))
    {
        gpio_act_id = gpio_id-SYS_PERI_MAX_GPIO_ID;
        tbl_out_id = GpioHsOutCtl_t;
        field_out_id = GpioHsOutCtl_cfgHsOutEn_f;
        tbl_intr_id = GpioHsIntrEn_t;
        field_intr_id = GpioHsIntrEn_cfgHsIntrEn_f;
        tbl_intr_level_id = GpioHsIntrLevel_t;
        field_intr_level_id = GpioHsIntrLevel_cfgHsIntrLevel_f;
    }
    else
    {
        gpio_act_id = gpio_id;
        tbl_out_id = GpioOutCtl_t;
        field_out_id = GpioOutCtl_cfgOutEn_f;
        tbl_intr_id = GpioIntrEn_t;
        field_intr_id = GpioIntrEn_cfgIntrEn_f;
        tbl_intr_level_id = GpioIntrLevel_t;
        field_intr_level_id = GpioIntrLevel_cfgIntrLevel_f;
    }

    cmd = DRV_IOR(tbl_out_id, field_out_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_out_val));
    cmd = DRV_IOR(tbl_intr_id, field_intr_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_intr_val));
    cmd = DRV_IOR(tbl_intr_level_id, field_intr_level_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_intr_level_val));

    switch(mode)
    {
        case CTC_CHIP_INPUT_MODE:
            CTC_BIT_UNSET(field_out_val, gpio_act_id);
            CTC_BIT_UNSET(field_intr_val, gpio_act_id);
            CTC_BIT_UNSET(field_intr_level_val, gpio_act_id);
            break;
        case CTC_CHIP_OUTPUT_MODE:
            CTC_BIT_SET(field_out_val, gpio_act_id);
            CTC_BIT_UNSET(field_intr_val, gpio_act_id);
            CTC_BIT_UNSET(field_intr_level_val, gpio_act_id);
            break;
        case CTC_CHIP_EDGE_INTR_INPUT_MODE:
            CTC_BIT_UNSET(field_out_val, gpio_act_id);
            CTC_BIT_SET(field_intr_val, gpio_act_id);
            CTC_BIT_UNSET(field_intr_level_val, gpio_act_id);
            break;
        case CTC_CHIP_LEVEL_INTR_INPUT_MODE:
            CTC_BIT_UNSET(field_out_val, gpio_act_id);
            CTC_BIT_SET(field_intr_val, gpio_act_id);
            CTC_BIT_SET(field_intr_level_val, gpio_act_id);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(tbl_out_id, field_out_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_out_val));
    cmd = DRV_IOW(tbl_intr_id, field_intr_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_intr_val));
    cmd = DRV_IOW(tbl_intr_level_id, field_intr_level_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_intr_level_val));

    return CTC_E_NONE;
}

/**
 @brief gpio output
*/
int32
sys_tsingma_peri_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint8 gpio_act_id = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio out, gpio_id:%d out_para:%d\n", gpio_id, out_para);

    if (gpio_act_id >= SYS_PERI_HS_MAX_GPIO_ID)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "gpio_id:%d over max gpio id\n", gpio_id);
        return CTC_E_INVALID_PARAM;
    }
    if (SYS_PERI_GPIO_IS_HS(gpio_id))
    {
        gpio_act_id = gpio_id-SYS_PERI_MAX_GPIO_ID;
        tbl_id = GpioHsDataCtl_t;
        field_id = GpioHsDataCtl_cfgHsData_f;
    }
    else
    {
        gpio_act_id = gpio_id;
        tbl_id = GpioDataCtl_t;
        field_id = GpioDataCtl_cfgData_f;
    }

    cmd = DRV_IOR(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if (out_para)
    {
        CTC_BIT_SET(field_val, gpio_act_id);
    }
    else
    {
        CTC_BIT_UNSET(field_val, gpio_act_id);
    }
    cmd = DRV_IOW(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

/**
 @brief gpio input
*/
int32
sys_tsingma_peri_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint8 gpio_act_id = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (gpio_act_id >= SYS_PERI_HS_MAX_GPIO_ID)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "gpio_id:%d over max gpio id\n", gpio_id);
        return CTC_E_INVALID_PARAM;
    }
    if (SYS_PERI_GPIO_IS_HS(gpio_id))
    {
        gpio_act_id = gpio_id-SYS_PERI_MAX_GPIO_ID;
        tbl_id = GpioHsDataCtl_t;
        field_id = GpioHsDataCtl_cfgHsData_f;
    }
    else
    {
        gpio_act_id = gpio_id;
        tbl_id = GpioDataCtl_t;
        field_id = GpioDataCtl_cfgData_f;
    }

    cmd = DRV_IOR(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if (CTC_IS_BIT_SET(field_val, gpio_act_id))
    {
        *in_value = TRUE;
    }
    else
    {
        *in_value = FALSE;
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get gpio in, gpio_id:%d in_para:%d\n", gpio_id, *in_value);
    return CTC_E_NONE;
}

#define __INIT__

STATIC int32
_sys_tsingma_peri_led_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 core_freq = 0;

    core_freq = sys_usw_get_core_freq(lchip, 0);

    /*
       #1, Cfg interface MACLED clock Frequency=25MHz,@clockCore=600MHz
    */
    field_value = (core_freq/25);
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #2, Toggle the clockLed divider reset
            cfg SupDskCfg.cfgResetClkLedDiv = 1'b1;
            cfg SupDskCfg.cfgResetClkLedDiv = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #3 RlmBsr RLM level reset
    */
    field_value = 0;
    cmd = DRV_IOW(SupResetCtl_t, SupResetCtl_resetRlmBsr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    /*
        #4, Cfg interface MACLED clock Frequency=25MHz,@clockCore=600MHz
            cfg RlmBsrCtlReset.resetCoreLed0  = 1'b0;
    */
    field_value = 0;
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreLed0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0x0d1cef00;
    cmd = DRV_IOW(LedBlinkCfg_t, LedBlinkCfg_blinkOffInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    cmd = DRV_IOW(LedBlinkCfg_t, LedBlinkCfg_blinkOnInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_peri_i2c_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    /*
        #1, Cfg I2C interface MSCL clock Frequency=25KHz, supper super freq @500MHz
    */

    field_value = 0x4E20;
    cmd = DRV_IOW(I2CMasterCfg0_t, I2CMasterCfg0_clkDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0x4E20;
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
sys_tsingma_peri_mdio_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    ctc_chip_phy_scan_ctrl_t scan_ctl;
    uint16 mac_id = 0;
    MdioChanMap_m mdio_map;
    uint32 value = 0;
    uint32 core_freq = 0;

    core_freq = sys_usw_get_core_freq(lchip, 0);

    sal_memset(&scan_ctl , 0, sizeof(ctc_chip_phy_scan_ctrl_t));

    /*
        #1, Cfg SMI 1G interface MDC clock Frequency=2.5MHz,@clockCore=600MHz
    */
    field_value = core_freq*2/5;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkMdioADiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #2, Toggle the clockMdio divider reset
            Cfg SupDskCfg.cfgResetClkMdioADiv = 1'b1;
            Cfg SupDskCfg.cfgResetClkMdioADiv = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioADiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioADiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #P2.Configure MDCXG clock
        #1, Cfg SMI 10G interface MDC clock Frequency=25MHz,@clockCore=600MHz
    */

    field_value = core_freq*2/5;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkMdioBDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #2, Toggle the clockMdio divider reset
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioBDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkMdioBDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        reset MDIO module
    */
    field_value = 0;
    cmd = DRV_IOW(SupResetCtl_t, SupResetCtl_resetMdio_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 1;
    cmd = DRV_IOW((MdioInit_t), MdioInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &field_value);

    /*
        MdioLinkDownDetecEn0_linkDownDetcEn_f
    */
    /* cmd = DRV_IOW(MdioLinkDownDetecEn0_t, MdioLinkDownDetecEn0_linkDownDetcEn_f);*/
    /* CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, detect_en));*/
    /* cmd = DRV_IOW(MdioLinkDownDetecEn1_t, MdioLinkDownDetecEn1_linkDownDetcEn_f);*/
    /* CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, detect_en));*/

    /*
        Init mdio scan. For no phy on demo board, just set interval
    */
    scan_ctl.op_flag = CTC_CHIP_INTERVAL_OP;
    scan_ctl.scan_interval = 100;
    scan_ctl.ctl_id = 0;
    CTC_ERROR_RETURN(sys_tsingma_peri_set_phy_scan_para(lchip, &scan_ctl));

    scan_ctl.op_flag = CTC_CHIP_INTERVAL_OP;
    scan_ctl.scan_interval = 100;
    scan_ctl.ctl_id = 1;
    CTC_ERROR_RETURN(sys_tsingma_peri_set_phy_scan_para(lchip, &scan_ctl));

    CTC_ERROR_RETURN(sys_tsingma_peri_set_phy_scan_cfg(lchip));

    /*cfg mdio channel map*/
    for (mac_id = 0; mac_id < 64; mac_id++)
    {
        sal_memset(&mdio_map, 0, sizeof(MdioChanMap_m));
        {
            value = mac_id;
        }

        SetMdioChanMap(V, chanId_f, &mdio_map, value);
        cmd = DRV_IOW((MdioChanMap_t), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &mdio_map);
    }

    sys_usw_peri_set_phy_scan_en(lchip, TRUE);

    return CTC_E_NONE;
}

int32
sys_tsingma_peri_init(uint8 lchip)
{
    CTC_ERROR_RETURN(_sys_tsingma_peri_led_init(lchip));
    CTC_ERROR_RETURN(_sys_tsingma_peri_i2c_init(lchip));

    /*CTC_ERROR_RETURN(sys_tsingma_peri_mdio_init(lchip));*/

    return CTC_E_NONE;
}

#define __OTHER_INTERFACE__

STATIC int32
_sys_tsingma_peri_get_temp_with_code(uint8 lchip, uint32 temp_code, uint32* p_temp_val)
{
    uint16 temp_mapping_tbl[SYS_TSINGMA_TEMP_TABLE_NUM+1] = {
        804, 801, 798, 795, 792, 790, 787, 784, 781, 778, 775, 772, 769, 766, 763, 761, 758, 755, 752, 749, /*-40~-21*/
        746, 743, 740, 737, 734, 731, 728, 725, 722, 719, 717, 714, 711, 708, 705, 702, 699, 696, 693, 690, /*-20~-1*/
        687, 684, 681, 678, 675, 672, 669, 666, 663, 660, 658, 655, 652, 649, 646, 643, 640, 637, 634, 631, /*0~19*/
        628, 625, 622, 619, 616, 613, 610, 607, 604, 601, 599, 596, 593, 590, 587, 584, 581, 578, 575, 572, /*20~39*/
        569, 566, 563, 560, 557, 554, 551, 548, 545, 542, 540, 537, 534, 531, 528, 525, 522, 519, 516, 513, /*40~59*/
        510, 507, 504, 501, 498, 495, 492, 489, 486, 483, 481, 478, 475, 472, 469, 466, 463, 460, 457, 454, /*60~79*/
        451, 448, 445, 442, 439, 436, 433, 430, 427, 424, 421, 418, 415, 412, 409, 406, 403, 400, 397, 394, /*80~99*/
        391, 388, 385, 382, 379, 376, 373, 370, 367, 364, 361, 358, 355, 352, 349, 346, 343, 340, 337, 334, /*100~119*/
        331, 328, 325, 322, 319, 316, 0};                                                                   /*120~125*/
    uint8 index = 0;

    /*if ((temp_code > temp_mapping_tbl[0]) || (temp_code < temp_mapping_tbl[SYS_TSINGMA_TEMP_TABLE_NUM-1]))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "temp code error %d\n", temp_code);
        return CTC_E_HW_INVALID_INDEX;
    }*/

    for (index = 0; index< SYS_TSINGMA_TEMP_TABLE_NUM; index++)
    {
        if ((temp_code <= temp_mapping_tbl[index]) && (temp_code > temp_mapping_tbl[index+1]))
        {
            break;
        }
    }

    if (index < 39)
    {
        *p_temp_val = 40 - index + (1 << 31);
    }
    else
    {
        *p_temp_val = index - 40;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_peri_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 timeout = SYS_TSINGMA_SENSOR_TIMEOUT;
    uint32 index = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_value);
    SYS_PERI_INIT_CHECK(lchip);
    if ((CTC_CHIP_SENSOR_TEMP != type) && (CTC_CHIP_SENSOR_VOLTAGE != type))
    {
        return CTC_E_INVALID_PARAM;
    }

    /*config RCLK_EN=1,RAVG_SEL,RCLK_DIV*/
    /*mask_write tbl-reg OmcMem 0xf offset 0x0 0x00001000 0x00001000*/
    index = 0xf;
    //cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    //CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 12);
    field_val = 0x310c7;
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    /*config RTHMC_RST=1*/
    /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000010 0x00000010*/
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 4);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    /*wait RTHMC_RST=1*/
    /*read tbl-reg OmcMem 0x10 offset 0x0*/
    timeout = SYS_TSINGMA_SENSOR_TIMEOUT;
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    while(timeout)
    {
        timeout--;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        if (0 == CTC_IS_BIT_SET(field_val, 4))
        {
            break;
        }
        sal_task_sleep(1);
    }
    if (0 == timeout)
    {
        return CTC_E_HW_TIME_OUT;
    }

    /*config ENBIAS=1£¬ENVR=1£¬ENAD=1*/
    /*mask_write tbl-reg OmcMem 0x11 offset 0x0 0x02000007 0x03000007*/
    index = 0x11;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 0);
    CTC_BIT_SET(field_val, 1);
    CTC_BIT_SET(field_val, 2);
    CTC_BIT_UNSET(field_val, 24);
    CTC_BIT_SET(field_val, 25);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    sal_task_sleep(10);

    /*set RTHM_MODE=1,RSAMPLE_DONE_INTEN=1,RDUMMY_THMRD=1,RV_SAMPLE_EN=1*/
    /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000203 0x00000003*/
    /*mask_write tbl-reg OmcMem 0x8 offset 0x0 0x00000004 0x00000004*/
    /*mask_write tbl-reg OmcMem 0x12 offset 0x0 0x00000001 0x00000001*/
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 0);
    CTC_BIT_SET(field_val, 1);
    CTC_BIT_SET(field_val, 9);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    index = 0x8;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 2);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    index = 0x12;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 0);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    if (CTC_CHIP_SENSOR_TEMP == type)
    {
        /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000001 0x00000001*/
        index = 0x10;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        CTC_BIT_SET(field_val, 0);
        cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }
    else
    {
        /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000000 0x00000001*/
        index = 0x10;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        CTC_BIT_UNSET(field_val, 0);
        cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }
    sal_task_sleep(10);

    /*mask_write tbl-reg OmcMem 0x12 offset 0x0 0x00000001 0x00000001*/
    index = 0x12;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 0);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));


    /*Wait((mask_read tbl-reg OmcMem 0xa offset 0x0 0x00000004) =1)*/
    index = 0xa;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    timeout = SYS_TSINGMA_SENSOR_TIMEOUT;
    while(timeout)
    {
        timeout--;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        if (CTC_IS_BIT_SET(field_val, 2))
        {
            break;
        }
        sal_task_sleep(1);
    }
    if (0 == timeout)
    {
        return CTC_E_HW_TIME_OUT;
    }

    /*mask_write tbl-reg OmcMem 0x11 offset 0x0 0x00000006 0x00000006*/
    index = 0x11;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    CTC_BIT_SET(field_val, 1);
    CTC_BIT_SET(field_val, 2);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    sal_task_sleep(10);

    if (CTC_CHIP_SENSOR_TEMP == type)
    {
        /*read-reg OmcMem 0xd offset 0x0*/
        index = 0xd;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        CTC_ERROR_RETURN(_sys_tsingma_peri_get_temp_with_code(lchip, field_val, p_value));
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "temp code %d\n", field_val);
    }
    else
    {
        /*read-reg OmcMem 0xc offset 0x0*/
        index = 0xc;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "voltage code %d\n", field_val);
        *p_value =  (field_val*1232)/1023;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void* p_data)
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
        case SYS_INTR_FUNC_MDIO_XG_CHANGE_0:
            table_id = MdioLinkChange1_t;
            mdio_ctlr_id = 1;
            break;
        case SYS_INTR_FUNC_MDIO_XG_CHANGE_1:
            table_id = MdioLinkChange0_t;
            mdio_ctlr_id = 0;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /*read which mac use phy*/
    cmd = DRV_IOR(MdioUserPhy_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, phy_enable);

    sal_memset(&intr_type, 0, sizeof(intr_type));
    intr_type.intr = intr;
    intr_type.sub_intr = INVG;
    intr_type.low_intr = INVG;
    CTC_ERROR_RETURN(sys_usw_interrupt_set_en(lchip, &intr_type, 0));

    cmd = DRV_IOR(table_id, MdioLinkChange0_changeStatus_f);
    cmd_w = DRV_IOW(table_id, MdioLinkChange0_changeStatus_f);
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


/****************************************************************************
* $Id$
* phy VSC8211 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2012-05-04
* Reason        : First Create.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_vtss8x58.h"

/*vsc8211*/
#define PHY_IDENTIFIER1_REG     0x2
#define AUTO_NEGO_PAGE_TRANSMIT 0x7
#define AUTO_NEGO_PAGE_RECEIVE  0x8
#define VSC8211_LED_CTL         0x1B
/*end vsc8211*/

static int32 
vtss8211_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
{
    phy_info_t* pphy_info;
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == phy_handle || NULL == phy_handle->mdio_hdl || NULL == value)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    sal_memset(&para, 0, sizeof(mdio_bus_para_t)); 
    para.gephy_para.phy_addr = pphy_info->phy_addr;
    para.gephy_para.reg = phyreg_param->dat.regaddr8;
    ret = phy_handle->mdio_hdl->read(phy_handle->mdio_hdl, &para);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }
     
    *value = para.gephy_para.val;
    //DRV_LOG_DEBUG(DRV_PHY,"VTSS8211 phy addr %d: read reg 0x%x: 0x%x", 
    //                pphy_info->phy_addr, phyreg_param->dat.regaddr8, *value);

    return RESULT_OK;
}


static int32 
vtss8211_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
{
    phy_info_t *pphy_info;
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == phy_handle || NULL == phy_handle->mdio_hdl)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    //DRV_LOG_DEBUG(DRV_PHY,"VTSS8211 phy addr %d: write reg 0x%x value 0x%x", 
    //                pphy_info->phy_addr, phyreg_param->dat.regaddr8, value);
    sal_memset(&para, 0, sizeof(mdio_bus_para_t)); 
    para.gephy_para.phy_addr = pphy_info->phy_addr;
    para.gephy_para.reg = phyreg_param->dat.regaddr8;
    para.gephy_para.val = value;
    ret = phy_handle->mdio_hdl->write(phy_handle->mdio_hdl, &para);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

static int32
vtss8211_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
{
    phyreg_param_t phyreg_para;
    uint16 val;
    int32 ret = 0;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }
    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    value = (value & mask) | (val & ~mask);
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

//static int32 vtss8211_read(phy_handle_t* phy_handle, uint16 reg, uint16 *value)
//{
//    phyreg_param_t phyreg_para;
//    int32 ret = 0;
//
//    phyreg_para.dat.regaddr8 = reg;
//    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);
//
//    return ret;
//}
static int32 vtss8211_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

static int32 vtss8211_read(phy_handle_t* phy_handle, uint16 reg, uint16* value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}

static int32
vtss8211_phy_page_std(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8211_write(phy_handle, PHY_EXTENDED_PAGE, 0x0);

    return ret;    
}

static int32
vtss8211_phy_page_ext1(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8211_write(phy_handle, PHY_EXTENDED_PAGE, 0x1);

    return ret;
}

static int32
vtss8211_phy_page_tr(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8211_write(phy_handle, PHY_EXTENDED_PAGE, 0x52B5); /* Token ring registers */

    return ret;
}

static int32
vtss8211_phy_page_test(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8211_write(phy_handle, PHY_EXTENDED_PAGE, 0x2A30); /* Test registers */

    return ret;
}

static int32 vtss8211_enab_smrt_premphasis (phy_handle_t* phy_handle)
{
    int32 ret = 0;
    
    /* BZ 2675 */
    ret += vtss8211_phy_page_test(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 8, 0x0200, 0x0200);//Ensure RClk125 enabled even in powerdown
    ret += vtss8211_phy_page_tr(phy_handle);    
    ret += vtss8211_write(phy_handle, 16, 0xa7fa);
    ret += vtss8211_write_mask(phy_handle, 18, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 17, 0x0008, 0x0008);
    ret += vtss8211_write(phy_handle, 16, 0x87fa);
    ret += vtss8211_phy_page_test(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 8, 0x0000, 0x0200);//Restore RClk125 gating
    ret += vtss8211_phy_page_std(phy_handle);

    return ret;
}

/* Init scripts for VSC8211 aka Cobra */
static int32 vtss8211_init_seq_cobra (phy_handle_t* phy_handle)
{
    int32 ret = 0;

    /* Enable token-ring register access for entire init script */
    ret += vtss8211_phy_page_test(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 8, 0x0200, 0x0200);
    ret += vtss8211_phy_page_tr(phy_handle);
    /* BZ 1769 */
    /* Performance robustness improvement on 50m loops */
    ret += vtss8211_write(phy_handle, 0, 0xafa4);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x4000, 0xf000);
    ret += vtss8211_write(phy_handle, 0, 0x8fa4);

    /* BZ 2337 */
    /* Inter-operability with Intel 82547E1 L322SQ96 */
    ret += vtss8211_write(phy_handle, 0, 0xafae);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0600, 0x7f80);
    ret += vtss8211_write(phy_handle, 0, 0x8fae);
    ret += vtss8211_phy_page_std(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 18, 0x0040, 0x0040);
    ret += vtss8211_phy_page_tr(phy_handle);
    
    /* BZ 1800 */
    /* 100BT Jumbo packet mode script */
    ret += vtss8211_write(phy_handle, 0, 0xb786);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0e20, 0x1fe0);
    ret += vtss8211_write(phy_handle, 0, 0x9786);

    /* BZ 2331 */
    /* Insufficient RGMII drive strength on long traces */
    ret += vtss8211_phy_page_test(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 3, 0x2000, 0x2000);
    ret += vtss8211_write_mask(phy_handle, 3, 0x1080, 0x1ff8);
    ret += vtss8211_phy_page_tr(phy_handle);
 
    /* BZ 2232 */
    /* DSP Optimization script */
    ret += vtss8211_write(phy_handle, 0, 0xaf8a);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0008, 0x000c);
    ret += vtss8211_write(phy_handle, 0, 0x8f8a);
    ret += vtss8211_write(phy_handle, 0, 0xaf86);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0008, 0x000c);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0000, 0x0000);
    ret += vtss8211_write(phy_handle, 0, 0x8f86);
    ret += vtss8211_write(phy_handle, 0, 0xaf82);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0100, 0x0180);
    ret += vtss8211_write(phy_handle, 0, 0x8f82);
    
    /* New bugzilla 11377 */
    /* Forced-10BASE-T PHY falsely linking up with forced-100BASE-TX link partner */
    ret += vtss8211_write(phy_handle, 0, 0xa7f4);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0002, 0x0006);
    ret += vtss8211_write(phy_handle, 0, 0x87f4);
    ret += vtss8211_write(phy_handle, 0, 0xa7f8);
    ret += vtss8211_write_mask(phy_handle, 2, 0x0000, 0x0000);
    ret += vtss8211_write_mask(phy_handle, 1, 0x0800, 0x0800);
    ret += vtss8211_write(phy_handle, 0, 0x87f8);

    /* Restore normal clock gating and return to main page after init script */
    ret += vtss8211_phy_page_test(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 8, 0x0000, 0x0200);
    ret += vtss8211_phy_page_std(phy_handle);


    /* Enable Smart Pre-emphasis */
    vtss8211_enab_smrt_premphasis(phy_handle);

    return ret;
}

int32 vtss8211_phy_reset(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_type)
{
    uint16 mac_media_mode_sel_15_12=0;
    uint16 mac_media_mode_sel_2_1=0;
    uint16 reg=0, value, count= 0;
    int32 ret=0;
    
    if(mac_if == GLB_MAC_INTERFACE_SGMII)
    {
        switch(media_type)
        {
            case GLB_MEDIA_RJ45:
                mac_media_mode_sel_15_12 = 0xb;
                mac_media_mode_sel_2_1 = 0x0;
                break;
            case GLB_MEDIA_1000BX:
            case GLB_MEDIA_PASS_THROUGH:    
                mac_media_mode_sel_15_12 = 0x9;
                mac_media_mode_sel_2_1 = 0x1;
                break;
            default:
                DRV_LOG_WARN("VSC8211 media interface [%d] is unknown.", media_type);
                return RESULT_ERROR;
        }
    }
    else if(mac_if == GLB_MAC_INTERFACE_SERDES)
    {
        switch(media_type)
        {
            case GLB_MEDIA_RJ45:
                mac_media_mode_sel_15_12 = 0xf;
                mac_media_mode_sel_2_1 = 0x0;
                break;
            case GLB_MEDIA_1000BX:
            case GLB_MEDIA_PASS_THROUGH:    
                mac_media_mode_sel_15_12 = 0x9;
                mac_media_mode_sel_2_1 = 0x1;
                break;
            default:
                DRV_LOG_WARN("VSC8211 media interface [%d] is unknown.", media_type);
                return RESULT_ERROR;
        }
    }
    else
    {
        DRV_LOG_WARN("VSC8211 mac interface [%d] is not supported.", mac_if);
        return RESULT_ERROR;
    }
    reg = 0x0a20;
    reg |= mac_media_mode_sel_15_12 << 12;
    reg |= mac_media_mode_sel_2_1 << 1;
    ret += vtss8211_write(phy_handle, EXTENDED_PHY_CTL1, reg);
    ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x8000, 0x8000);
    sal_udelay(40 * 1000);/* pause after reset */
    /* wait up to 5 seconds */
    while(1)
    {        
        ret += vtss8211_read(phy_handle, PHY_CTL_REG, &value);
        if(!ret && (value & 0x8000) == 0)
        {
            break;
        }
        sal_udelay(20000);
        count++;
        if(count >= 250)
        {
            DRV_LOG_ERR("VSC8211 PHY [addr: ]reset timeout!",phy_handle->phy_info.phy_addr);
            return RESULT_ERROR;
        }
    }
    ret += vtss8211_init_seq_cobra(phy_handle);
    return ret;
}

/*Now only use MDIX-Auto.*/
static int32 vtss8211_phy_mdi_setup(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    ret += vtss8211_write_mask(phy_handle, BYPASS_CTL, 0x0, 0xa0);
    ret += vtss8211_phy_page_ext1(phy_handle);
    ret += vtss8211_write_mask(phy_handle, 19, 0x0, 0xc);
    ret += vtss8211_phy_page_std(phy_handle);
    return ret;
}

static int32 vtss8211_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    int32 ret=0;
    uint16 reg4=1, reg9=0;

    /*Auto mode*/
    if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
    {
        switch(speed)
        {
            case GLB_SPEED_AUTO:
                if(duplex == GLB_DUPLEX_AUTO)
                {
                /* Modified by liuht to disable 1000BASE-T HDX mode for bug25644, 2013-11-06 */
                    reg4 = 0x1e0;
                    reg9 = 0x200;
                }
                else if(duplex == GLB_DUPLEX_FULL)
                {
                    reg4 = 0x140;
                    reg9 = 0x200;
                }
                else
                {
                /* Modified by liuht to disable 1000BASE-T HDX mode for bug25644, 2013-11-06 */                
                    reg4 = 0xa0;
                    reg9 = 0x0;
                }                
                break;
            case GLB_SPEED_1G:
                /* Modified by liuht to disable 1000BASE-T HDX mode for bug25644, 2013-11-06 */				
                reg4 = 0x0;
                reg9 = 0x200;
                break;
            case GLB_SPEED_100M:
                reg4 = 0x180;
                reg9 = 0;
                break;
            case GLB_SPEED_10M:
                reg4 = 0x60;
                reg9 = 0;
                break;
            default:
                /* Modified by liuht to disable 1000BASE-T HDX mode for bug25644, 2013-11-06 */				
                reg4 = 0x1e0;
                reg9 = 0x200;
                break;
        }
        if(phy_handle->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause)
        {
            reg4 |= (1<<10);
        }
        if(phy_handle->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause)
        {
            reg4 |= (1<<11);
        }
        ret += vtss8211_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0);
        ret += vtss8211_write_mask(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300);        
        ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);        
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
	     /* Modified by liuht to powerdown more time to notify link parter for bug25796, 2013-11-14 */		
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0100, 0x3140);
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x2100, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x2000, 0x3140);
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0140, 0x3140);
        }
        else /*1G Half*/
        {    
            ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0040, 0x3140);
        }
        if(speed != GLB_SPEED_1G)
        {
            // Work around bugzilla8953 - Port is not coming UP without auto negotiation
            ret += vtss8211_phy_page_test(phy_handle);
            ret += vtss8211_write_mask(phy_handle, 8, 0x0200, 0x0200);
            ret += vtss8211_phy_page_tr(phy_handle);
            ret += vtss8211_write(phy_handle, 2, 0x0012);
            ret += vtss8211_write(phy_handle, 1, 0x2803);
            ret += vtss8211_write(phy_handle, 0, 0x87fa);
            ret += vtss8211_phy_page_test(phy_handle);
            ret += vtss8211_write_mask(phy_handle,  8, 0x0000, 0x0200);
            ret += vtss8211_phy_page_std(phy_handle);            
        }
    }
    ret += vtss8211_phy_mdi_setup(phy_handle);
    return ret;    
}

int32 vtss8211_init(phy_handle_t* phy_handle)
{
    phy_info_t* pphy_info;    
    phyreg_param_t phyreg_para;
    int32 ret = 0;
    uint16 value;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VTSS8211 Init phy addr %d", pphy_info->phy_addr);

    ret += vtss8211_phy_reset(phy_handle, pphy_info->phy_manage_info.mac_if, pphy_info->phy_manage_info.media_type);
    ret += vtss8211_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed,phy_handle->phy_info.phy_manage_info.duplex);
   
    /*power down*/
    ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
    //Bugzilla#7489 - Interrupt must be disable when powered down
    ret += vtss8211_write_mask(phy_handle, PHY_INT_MASK, 0x0000, 0xffff);
    
    /*no sgmii preamble, change 0xc000 into 0xe000*/
    phyreg_para.dat.regaddr8 = EXTENDED_PHY_CTL2;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe000);

    phyreg_para.dat.regaddr8 = VSC8211_LED_CTL;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x10);
    /*Clear interrupts*/
    phyreg_para.dat.regaddr8 = PHY_INT_STATUS;
    phy_handle->reg_read(phy_handle, &phyreg_para, &value);

    return ret;
}


int32 vtss8211_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = 0;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VTSS8211 phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        //Bugzilla#7489 - Interrupt must be disable when powered down
        ret += vtss8211_write_mask(phy_handle, PHY_INT_MASK, 0xf000, 0xffff);
    }
    else 
    {
        /*power down*/
        ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
        //Bugzilla#7489 - Interrupt must be disable when powered down
        ret += vtss8211_write_mask(phy_handle, PHY_INT_MASK, 0x0000, 0xffff);
    }
    return ret;
}

int32 vtss8211_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8211 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = vtss8211_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}


int32 vtss8211_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8211 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = vtss8211_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
int32
vtss8211_config_master_slave(phy_handle_t* phy_handle, uint8 master_slave)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8211 phy addr %d config master-slave mode %d, orig %d", pphy_info->phy_addr, master_slave, pphy_info->phy_manage_info.master_slave);
    ret += vtss8211_phy_page_std(phy_handle);
    switch(master_slave)
    {
    case GLB_MODE_AUTO:
        ret |= vtss8211_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x0000, 0x1000);
        break;
    case GLB_MODE_MASTER:
        ret |= vtss8211_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1800, 0x1800);
        break;
    case GLB_MODE_SLAVE:
        ret |= vtss8211_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1000, 0x1800);
        break;
    default:
        return RESULT_ERROR;
        break;
    }
    ret = vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    return ret;
}
/* End of liuht modified */

int32 vtss8211_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = 0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VTSS8211 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= vtss8211_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);
        break;

    case GLB_LB_PHY_FAR:
        ret |= vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= vtss8211_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0008, 0x0008);        
        break;

    case GLB_LB_PHY_NEAR:
        ret |= vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= vtss8211_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);         
        break;

    /* Added by liuht for bug 27508, 2014-06-30 */
    case GLB_LB_PHY_NEAR_DIAG:
        ret |= vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= vtss8211_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);
        ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
        break;
    /* End of liuht added */

    default:
        return RESULT_ERROR;
    }

    return ret;
}

int32 vtss8211_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8211 phy addr %d config medium mac_if %d media_if %d", pphy_info->phy_addr, mac_if, media_if);
    ret = vtss8211_phy_reset(phy_handle, mac_if, media_if);
    /*After medium config, speed/duplex/loopback need re-config. */
    ret |= vtss8211_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex);
    if(GLB_LB_NONE != pphy_info->phy_manage_info.lb_mode)
    {
        ret |= vtss8211_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }
    /*Fix bug30023. After medium config, enable need re-config*/
    if(0 == pphy_info->phy_manage_info.enable)
    {
        ret |= vtss8211_enable(phy_handle, 0);
    }
    return ret;
}

int32
vtss8211_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    int32 ret = RESULT_OK;    
    phy_info_t *pphy_info;
    uint16 value=0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8211 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += vtss8211_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += vtss8211_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}


int32 vtss8211_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media,
                                glb_port_master_slave_t master_slave)
{
    return RESULT_OK;
}

int32 
vtss8211_link_down_setting(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;
    uint16 value; 
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vtss8211_link_down_setting ");

    phyreg_para.dat.regaddr8 = PHY_CTL_REG;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    if ((value & (1<<11))==0) {
        /* Briefly power down PHY to avoid problem sometimes seen when
           changing speed on SmartBit */
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, value | (1<<11));  
        ret += vtss8211_write_mask(phy_handle, 1, 0x0, 0x4); 
        sal_udelay(100);
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
     }


    return ret;
} 

static int32 
vtss8211_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
    phyreg_param_t phyreg_para;
    uint16 cfg_symmetric, cfg_asymmetric, lp_symmetric, lp_asymmetric;
    uint16 mediaSel;
    int32 ret = 0;
    uint16 value;
    
    if(NULL == phy_handle || NULL == phy_val)
    {
        return RESULT_ERROR;
    }

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    
    phyreg_para.dat.regaddr8 = PHY_STATUS_REG;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);

    phy_val->link_up = (value&PHY_STATUS_LINK) ? GLB_LINK_UP : GLB_LINK_DOWN;

    /*if port is up, get real media*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        phyreg_para.dat.regaddr8 = AUX_PHY_STATUS_REG;
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);

        if((value&PHY_STATUS_SPEED) == 0x00)
        {
            phy_val->speed = GLB_SPEED_10M;
        }
        else if((value&PHY_STATUS_SPEED) == 0x8)
        {
            phy_val->speed = GLB_SPEED_100M;
        }
        else if((value&PHY_STATUS_SPEED) == 0x10)
        {
            phy_val->speed = GLB_SPEED_1G;
        }   

        if((value&PHY_STATUS_FDX) == 0)
        {
            phy_val->duplex = GLB_DUPLEX_HALF;
        }
        else if((value&PHY_STATUS_FDX) == PHY_STATUS_FDX)
        {
            phy_val->duplex = GLB_DUPLEX_FULL;
        }
        /*If auto enable and auto complete, need get flow control info.*/
        if((value & 0xc000)==0x8000)
        {
            cfg_symmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause;
            cfg_asymmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause;
            ret += vtss8211_read(phy_handle, AUTO_NEGO_LP_ADVERTI_REG, &value);
            lp_symmetric = ((value>>10) & 0x1);
            lp_asymmetric = ((value>>11) & 0x1);
            phy_val->flowctrl.recv = (cfg_symmetric && (lp_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
            phy_val->flowctrl.send = (lp_symmetric && (cfg_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
        }
        else
        {
            phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
            phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        }
        /*get currently media mode status*/
        phyreg_para.dat.regaddr8 = PHY_EXTENDED_PAGE;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 1);
        
        phyreg_para.dat.regaddr8 = 0x14;
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &mediaSel);
        
        phyreg_para.dat.regaddr8 = PHY_EXTENDED_PAGE;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0);
        
        /*if media mode is copper*/
        if((mediaSel&0x00c0) == 0x40)
        {
            /*Note: vsc8211 datasheet have an error here for copper and fiber type define */
            phy_val->linkup_media = GLB_PORT_TYPE_COPPER;
        }
        /*if media mode is fiber*/
        else
        {
            phy_val->linkup_media = GLB_PORT_TYPE_FIBER;            
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        /*get currently mode : slave or master*/ 
        ret += vtss8211_phy_page_std(phy_handle);
        ret += vtss8211_read(phy_handle, 0xa, &value);
        /*if mode is master*/
        if((value & 0x4000) == 0x4000)
        {
            phy_val->master_slave= GLB_MODE_MASTER;
        }
        /*if mode is slave*/
        else
        {
            phy_val->master_slave= GLB_MODE_SLAVE;            
        }
        /* End of liuht modified */
    }
    else
    {
        phy_val->speed = GLB_SPEED_AUTO;
        phy_val->duplex = GLB_DUPLEX_AUTO;
        phy_val->linkup_media = GLB_PORT_TYPE_AUTO; 
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        phy_val->master_slave = GLB_MODE_AUTO;
        /* End of liuht modified */
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }    

    return ret;
}

int32 vtss8211_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    vtss8211_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8211 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
        phy_change->flowctrl_change = 1;
        }

        vtss8211_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8211 phy addr %d link down",pphy_info->phy_addr) 
        vtss8211_link_down_setting(phy_handle);
    }
    /*link always up*/
    else if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_change->action = -1;
        if(pphy_info->phy_stat_flag.speed != phy_val->speed)
        {            
            phy_change->speed_change = 1;
        }
        if(pphy_info->phy_stat_flag.duplex!= phy_val->duplex)
        {
            phy_change->duplex_change = 1;
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        if(pphy_info->phy_stat_flag.master_slave!= phy_val->master_slave)
        {
            phy_change->master_slave_change = 1;
        }
        /* End of liuht modified */
        if(pphy_info->phy_stat_flag.linkup_media!= phy_val->linkup_media)
        {
            phy_change->media_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change)|| (1==phy_change->media_change) || (phy_change->master_slave_change))
        {
            vtss8211_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
        }
        /* End of liuht modified */
    }
    /*link always down*/
    else
    {
        phy_change->action = -1;
    }

    pphy_info->phy_stat_flag.link_up = phy_val->link_up;
    pphy_info->phy_stat_flag.speed = phy_val->speed;
    pphy_info->phy_stat_flag.duplex = phy_val->duplex;
    pphy_info->phy_stat_flag.linkup_media = phy_val->linkup_media;
    /* Modified by liuht for bug 28155, 2014-04-17 */
    /* Support force master or slave mode */
    pphy_info->phy_stat_flag.master_slave= phy_val->master_slave;
    /* End of liuht modified */
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return RESULT_OK;

}

int32
vtss8211_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phyreg_param_t phyreg_para;
    phy_info_t *pphy_info;
    uint16 val;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    /*if NO link change, return*/
    phyreg_para.dat.regaddr8 = PHY_INT_STATUS;
    phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    if(0 == val)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    if(PHY_WORK_MODE_PASSTHROUGH == pphy_info->phy_manage_info.mode)
    {
         return RESULT_OK;
    }
    vtss8211_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"vtss8211_get_cur_status speed: %d duplex: %d",phy_val->speed, phy_val->duplex);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {   
        phy_change->action = 1;

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        vtss8211_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);

    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        vtss8211_link_down_setting(phy_handle);
    }
    /*link always up*/
    else if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_change->action = -1;
        if(pphy_info->phy_stat_flag.speed != phy_val->speed)
        {
            phy_change->speed_change = 1;            
        }
        if(pphy_info->phy_stat_flag.duplex!= phy_val->duplex)
        {
            phy_change->duplex_change = 1;
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        if(pphy_info->phy_stat_flag.master_slave!= phy_val->master_slave)
        {
            phy_change->master_slave_change = 1;
        }
        /* End of liuht modified */
        if(pphy_info->phy_stat_flag.linkup_media!= phy_val->linkup_media)
        {
            phy_change->media_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change)|| (1==phy_change->media_change) || (phy_change->master_slave_change))
        {
            vtss8211_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
        }
        /* End of liuht modified */
    }
    /*link always down*/
    else
    {
        phy_change->action = -1;
    }
    pphy_info->phy_stat_flag.link_up = phy_val->link_up;
    pphy_info->phy_stat_flag.speed = phy_val->speed;
    pphy_info->phy_stat_flag.duplex = phy_val->duplex;
    pphy_info->phy_stat_flag.linkup_media = phy_val->linkup_media;
    /* Modified by liuht for bug 28155, 2014-04-17 */
    /* Support force master or slave mode */
    pphy_info->phy_stat_flag.master_slave= phy_val->master_slave;
    /* End of liuht modified */
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return RESULT_OK;

}

phy_handle_t* phy_vtss8211_dev_register(phy_info_t* pphy_info)
{

    phy_handle_t* phdl = NULL;
    mdio_handle_t* p_mdio_hdl = NULL;
    mdio_info_t mdio_info;
     
    sal_memset(&mdio_info, 0, sizeof(mdio_info));
    
    if(NULL == pphy_info)
    {
        return NULL;
    }
    
    phdl = (phy_handle_t* )DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(phy_handle_t));
    if(NULL == phdl)
    {
        DRV_LOG_ERR("malloc failed");
        return NULL;
    }

    memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));

    phdl->phy_init = vtss8211_init;
    phdl->config_enable = vtss8211_enable;
    phdl->config_speed = vtss8211_config_speed;
    phdl->config_duplex = vtss8211_config_duplex;
    phdl->config_master_slave = vtss8211_config_master_slave;
    phdl->config_loopback = vtss8211_config_loopback;
    phdl->config_medium = vtss8211_config_medium;
    phdl->config_flowctrl = vtss8211_config_flowctrl;
    phdl->get_link_poll = vtss8211_get_link_poll;
    phdl->get_link_interupt = vtss8211_get_link_interrput;
    phdl->get_cur_status = vtss8211_get_cur_status; /* modified by liuht for bug 26641, 2014-0422 */
    phdl->reg_read = vtss8211_reg_read;
    phdl->reg_write = vtss8211_reg_write;
    
    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type  = pphy_info->mdio_bus;
    /* set slot_card_idx for each phy_addr, and send to MDIO driver */
    mdio_info.slot_card_idx = pphy_info->phy_manage_info.slot_card_idx;
    p_mdio_hdl = mdio_create_handle(&mdio_info);
    
    if( NULL == p_mdio_hdl)
    {
        goto err_out;
    }
    
    phdl->mdio_hdl = p_mdio_hdl;

    return phdl;
    
err_out:
    if (NULL != phdl)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, phdl);
        phdl = NULL;
    }

    return NULL;

}




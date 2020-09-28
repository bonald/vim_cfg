/****************************************************************************
* $Id$
* phy qt2x25 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-08-18
* Reason        : First Create.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_vtss8x58.h"
#include "phy_veriphy.h"

static int32 
vtss8x58_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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
    //DRV_LOG_DEBUG(DRV_PHY,"VTSS8x58 phy addr %d: read reg 0x%x: 0x%x", 
    //                pphy_info->phy_addr, phyreg_param->dat.regaddr8, *value);

    return RESULT_OK;
}


static int32 
vtss8x58_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
{
    phy_info_t *pphy_info;
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == phy_handle || NULL == phy_handle->mdio_hdl)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    //DRV_LOG_DEBUG(DRV_PHY,"VTSS8x58 phy addr %d: write reg 0x%x value 0x%x", 
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
vtss8x58_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
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

static int32 vtss8x58_read(phy_handle_t* phy_handle, uint16 reg, uint16 *value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}
static int32 vtss8x58_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

static int32
vtss8x58_phy_page_std(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8x58_write(phy_handle, PHY_EXTENDED_PAGE, 0x0);

    return ret;    
}

static int32
vtss8x58_phy_page_ext1(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8x58_write(phy_handle, PHY_EXTENDED_PAGE, 0x1);

    return ret;
}

static int32
vtss8x58_phy_page_tr(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8x58_write(phy_handle, PHY_EXTENDED_PAGE, 0x52B5); /* Token ring registers */

    return ret;
}

static int32
vtss8x58_phy_page_test(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8x58_write(phy_handle, PHY_EXTENDED_PAGE, 0x2A30); /* Test registers */

    return ret;
}

static int32
vtss8x58_phy_page_gpio(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vtss8x58_write(phy_handle, PHY_EXTENDED_PAGE, 0x10);

    return ret;
}

static int32 vtss8x58_phy_optimize_dsp(phy_handle_t* phy_handle)
{
    int32 ret=0;
    /* BZ 2226/2227/2228/2229/2230/2231/2232/2294 */
    ret += vtss8x58_write(phy_handle,  31, 0x52b5);
    ret += vtss8x58_write(phy_handle,  16, 0xaf8a);
    ret += vtss8x58_write_mask(phy_handle,  18, 0x0000, 0x0000);
    ret += vtss8x58_write_mask(phy_handle,  17, 0x0008, 0x000c);    
    ret += vtss8x58_write(phy_handle,  16, 0x8f8a);
    ret += vtss8x58_write(phy_handle,  16, 0xaf86);    
    ret += vtss8x58_write_mask(phy_handle,  18, 0x0008, 0x000c);
    ret += vtss8x58_write_mask(phy_handle,  17, 0x0000, 0x0000);
    ret += vtss8x58_write(phy_handle,  16, 0x8f86);
    ret += vtss8x58_write(phy_handle,  16, 0xaf82);
    ret += vtss8x58_write_mask(phy_handle,  18, 0x0000, 0x0000);
    ret += vtss8x58_write_mask(phy_handle,  17, 0x0100, 0x0180);    
    ret += vtss8x58_write(phy_handle,  16, 0x8f82);
    ret += vtss8x58_write(phy_handle,  31, 0); 
    return ret;
}

static int32 vtss8x58_phy_optimize_receiver_init(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    /* BZ 1776/1860/2095/2107, part 1/3 and 2/3 */
    ret += vtss8x58_phy_page_test(phy_handle);
    ret += vtss8x58_write_mask(phy_handle, 12, 0x0200, 0x0300); // Not needed for ATOM/TESLA
    return ret;
}

static int32 vtss8x58_enab_smrt_premphasis (phy_handle_t* phy_handle)
{
    int32 ret = 0;
    
    /* BZ 2675 */
    ret += vtss8x58_write(phy_handle, 31, 0x2a30);
    ret += vtss8x58_write_mask(phy_handle, 8, 0x0200, 0x0200);//Ensure RClk125 enabled even in powerdown
    ret += vtss8x58_write(phy_handle, 31, 0x52B5);
    ret += vtss8x58_write(phy_handle, 16, 0xa7fa);
    ret += vtss8x58_write_mask(phy_handle, 18, 0x0000, 0x0000);
    ret += vtss8x58_write_mask(phy_handle, 17, 0x0008, 0x0008);
    ret += vtss8x58_write(phy_handle, 16, 0x87fa);
    ret += vtss8x58_write(phy_handle, 31, 0x2a30);
    ret += vtss8x58_write_mask(phy_handle, 8, 0x0000, 0x0200);//Restore RClk125 gating
    ret += vtss8x58_write(phy_handle, 31, 0x0);

    return ret;
}

/* Init scripts for VSC8538/VSC8558/VSC8658 aka Spyder/GTO */
static int32 vtss8x58_init_seq_spyder (phy_handle_t* phy_handle)
{
    int32 ret = 0;
    uint16 reg;

    /* Init scripts common to all Octal PHY devices */
    /* BZ 2486/2487 */
    ret += vtss8x58_phy_page_std(phy_handle);

    /* BZ 2112 */
    /* Turn off Carrier Extensions */
    ret += vtss8x58_phy_page_ext1(phy_handle);
    ret += vtss8x58_write_mask(phy_handle, 20, 0x8000, 0x8000);
    ret += vtss8x58_phy_page_std(phy_handle);
    if(phy_handle->phy_info.part_num == VTSS_8558_PART_NUMBER)
    {
        if(phy_handle->phy_info.phy_revision == 0)
        {
            /* BZ 2020 */
            ret += vtss8x58_phy_page_test(phy_handle);
            ret += vtss8x58_write_mask(phy_handle, 27, 0x8000, 0x8000);
            ret += vtss8x58_write_mask(phy_handle, 19, 0x0300, 0x0f00);
            ret += vtss8x58_phy_page_std(phy_handle);

            /* BZ 2063 */
            ret += vtss8x58_phy_page_tr(phy_handle);
            ret += vtss8x58_write(phy_handle, 16, 0xa60c);
            ret += vtss8x58_write(phy_handle, 16, 0xa60c);
            ret += vtss8x58_read(phy_handle, 17, &reg);
            if ((reg & (1<<3)) == 0) {
                /* !RxTrLock */
                ret += vtss8x58_write(phy_handle, 17, 0x0010);
                ret += vtss8x58_write(phy_handle, 16, 0x8604);
                ret += vtss8x58_write(phy_handle, 17, 0x00df);
                ret += vtss8x58_write(phy_handle, 16, 0x8600);
                ret += vtss8x58_write(phy_handle, 17, 0x00ff);
                ret += vtss8x58_write(phy_handle, 16, 0x8600);
                ret += vtss8x58_write(phy_handle, 17, 0x0000);
                ret += vtss8x58_write(phy_handle, 16, 0x8604);
                ret += vtss8x58_write(phy_handle, 16, 0xa60c);
                ret += vtss8x58_write(phy_handle, 16, 0xa60c);
            }
            ret += vtss8x58_phy_page_std(phy_handle);

            /* BZ 2069/2086 */
            ret += vtss8x58_phy_page_ext1(phy_handle);
            ret += vtss8x58_write_mask(phy_handle, 17, 0x0000, 0x0001);
            ret += vtss8x58_read(phy_handle, 23, &reg); /* PHY address at bit 11-15 */
            ret += vtss8x58_phy_page_std(phy_handle);

            /* BZ 2084 */
            if ((reg & (0x7 << 11)) == 0) {
                ret += vtss8x58_phy_page_gpio(phy_handle);
                ret += vtss8x58_write(phy_handle, 0, 0x7009); /*- Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock */
                ret += vtss8x58_write(phy_handle, 12, 0x5000); /*- Dummy write to start off */
                ret += vtss8x58_write(phy_handle, 11, 0xffff); /*- Dummy write to start off */
                ret += vtss8x58_write(phy_handle, 12, 0x5002); /*- Dummy write to addr 16384= 02 */
                ret += vtss8x58_write(phy_handle, 12, 0x5040); /*- Dummy write to addr 16385= 40 */
                ret += vtss8x58_write(phy_handle, 12, 0x500C); /*- Dummy write to addr 16386= 0C */
                ret += vtss8x58_write(phy_handle, 12, 0x5002); /*- Dummy write to addr 16387= 02 */
                ret += vtss8x58_write(phy_handle, 12, 0x5040); /*- Dummy write to addr 16388= 40 */
                ret += vtss8x58_write(phy_handle, 12, 0x5021); /*- Dummy write to addr 16389= 21 */
                ret += vtss8x58_write(phy_handle, 12, 0x5002); /*- Dummy write to addr 16390= 02 */
                ret += vtss8x58_write(phy_handle, 12, 0x5040); /*- Dummy write to addr 16391= 40 */
                ret += vtss8x58_write(phy_handle, 12, 0x5022); /*- Dummy write to addr 16392= 22 */
                ret += vtss8x58_write(phy_handle, 12, 0x5002); /*- Dummy write to addr 16393= 02 */
                ret += vtss8x58_write(phy_handle, 12, 0x5040); /*- Dummy write to addr 16391= 40 */
                ret += vtss8x58_write(phy_handle, 12, 0x5023); /*- Dummy write to addr 16392= 23 */
                ret += vtss8x58_write(phy_handle, 12, 0x50C2); /*- Dummy write to addr 16396= C2 */
                ret += vtss8x58_write(phy_handle, 12, 0x50AD); /*- Dummy write to addr 16397= AD */
                ret += vtss8x58_write(phy_handle, 12, 0x50C2); /*- Dummy write to addr 16396= C2 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CA); /*- Dummy write to addr 16399= CA */
                ret += vtss8x58_write(phy_handle, 12, 0x5075); /*- Dummy write to addr 16400= 75 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CB); /*- Dummy write to addr 16401= CB */
                ret += vtss8x58_write(phy_handle, 12, 0x509A); /*- Dummy write to addr 16402= 9A */
                ret += vtss8x58_write(phy_handle, 12, 0x5075); /*- Dummy write to addr 16400= 75 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CA); /*- Dummy write to addr 16399= CA */
                ret += vtss8x58_write(phy_handle, 12, 0x5046); /*- Dummy write to addr 16405= 46 */
                ret += vtss8x58_write(phy_handle, 12, 0x5085); /*- Dummy write to addr 16406= 85 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CB); /*- Dummy write to addr 16401= CB */
                ret += vtss8x58_write(phy_handle, 12, 0x50CD); /*- Dummy write to addr 16408= CD */
                ret += vtss8x58_write(phy_handle, 12, 0x5085); /*- Dummy write to addr 16406= 85 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CA); /*- Dummy write to addr 16399= CA */
                ret += vtss8x58_write(phy_handle, 12, 0x50CC); /*- Dummy write to addr 16411= CC */
                ret += vtss8x58_write(phy_handle, 12, 0x50D2); /*- Dummy write to addr 16412= D2 */
                ret += vtss8x58_write(phy_handle, 12, 0x50CA); /*- Dummy write to addr 16399= CA */
                ret += vtss8x58_write(phy_handle, 12, 0x50D2); /*- Dummy write to addr 16414= D2 */
                ret += vtss8x58_write(phy_handle, 12, 0x50AD); /*- Dummy write to addr 16415= AD */
                ret += vtss8x58_write(phy_handle, 12, 0x5022); /*- Dummy write to addr 16416= 22 */
                ret += vtss8x58_write(phy_handle, 12, 0x5022); /*- Dummy write to addr 16416= 22 */
                ret += vtss8x58_write(phy_handle, 12, 0x5022); /*- Dummy write to addr 16416= 22 */
                ret += vtss8x58_write(phy_handle, 12, 0x5022); /*- Dummy write to addr 16416= 22 */
                ret += vtss8x58_write(phy_handle, 12, 0x0000); /*- Clear internal memory access */
                ret += vtss8x58_write(phy_handle, 0, 0x4099); /*- Allow 8051 to run again, with patch enabled */
                ret += vtss8x58_write(phy_handle, 0, 0xc099); /*- Allow 8051 to run again, with patch enabled */
                ret += vtss8x58_phy_page_std(phy_handle);
            }
                        /* BZ 2087 */
            ret += vtss8x58_phy_page_tr(phy_handle);            
            ret += vtss8x58_write(phy_handle, 16, 0xa606);    /*- Request read, Media SerDes Control */
            ret += vtss8x58_read(phy_handle, 17, &reg);
            reg = (reg & 0xfff8) | 5;                        /*- Optimize sample delay setting */
            ret += vtss8x58_write(phy_handle, 18, 0x0000);    /*- Is this OK? */
            ret += vtss8x58_write(phy_handle, 17, reg);
            ret += vtss8x58_write(phy_handle,16, 0x8606);    /* Write Media SerDes Control Word */

            ret += vtss8x58_write(phy_handle, 16, 0xae06);       /*- Request read, MAC SerDes control */
            ret += vtss8x58_read(phy_handle, 17, &reg);
            reg = (reg & 0xfff8) | 5;                           /*- Optimize sample delay setting */
            ret += vtss8x58_write(phy_handle, 18, 0x0000);       /*- Is this OK? */
            ret += vtss8x58_write(phy_handle, 17, reg);
            ret += vtss8x58_write(phy_handle,16, 0x8e06);        /* Write MAC SerDes Control Word */
            ret += vtss8x58_phy_page_std(phy_handle);
        }
        /* BZ 2411 - PLL Error Detector Bit Enable */
        ret += vtss8x58_phy_page_test(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 2, 0x8000, 0x8000);
        ret += vtss8x58_phy_page_std(phy_handle);

        /* BZ 2230 - DSP Optimization, BZ 2230 */
        ret += vtss8x58_phy_optimize_dsp(phy_handle);

        /* BZ 1971 */
        ret += vtss8x58_write_mask(phy_handle, 18, 0x0040, 0x0040);

        /* BZ 1860 */
        ret += vtss8x58_phy_optimize_receiver_init(phy_handle);

        /* BZ 2114 */
        ret += vtss8x58_phy_page_tr(phy_handle);
        ret += vtss8x58_write(phy_handle, 16, 0xaf82);
        ret += vtss8x58_read(phy_handle, 17, &reg);
        reg = (reg & 0xffef) | 0;           /*- Enable DFE in 100BT */
        ret += vtss8x58_write(phy_handle, 17, reg);
        ret += vtss8x58_write(phy_handle, 16, 0x8f82);
        ret += vtss8x58_phy_page_std(phy_handle);

        /* BZ 2221*/
        ret += vtss8x58_phy_page_ext1(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 20, 0x1800, 0x1800);
        ret += vtss8x58_phy_page_std(phy_handle);

        /* BZ 2012 */
        ret += vtss8x58_phy_page_test(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 20, 0x0000, 0x1000);  /*- Clear Force Bit */
        ret += vtss8x58_write_mask(phy_handle, 20, 0x6000, 0x6000);  /* MDI Impedance setting = +2 */
        ret += vtss8x58_phy_page_std(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 24, 0xa002, 0xe00e);  /* 100-BT Amplitude/Slew Rate Control */
    
    }
    else if(phy_handle->phy_info.part_num == VTSS_8658_PART_NUMBER)
    {
        if (phy_handle->phy_info.phy_revision == 0) 
        {
            /* BZ 2545 */
            /* 100-Base FX Clock Data Recovery Improvement */
            {
                uint16 reg17, reg18;

                ret += vtss8x58_phy_page_tr(phy_handle);
                ret += vtss8x58_write(phy_handle, 16, 0xae0e);
                ret += vtss8x58_read(phy_handle, 18, &reg18);
                ret += vtss8x58_read(phy_handle, 17, &reg17);
                ret += vtss8x58_write(phy_handle, 18, reg18);
                ret += vtss8x58_write(phy_handle, 17, (reg17&0xffef));
                ret += vtss8x58_write(phy_handle, 16, 0x8e0e);
                ret += vtss8x58_phy_page_std(phy_handle);
            }
        } /* ps->revision == 0 */
    }
    // Improve 100BASE-TX link startup robustness to address interop issue
    ret += vtss8x58_phy_page_tr(phy_handle);
    ret += vtss8x58_write(phy_handle, 18, 0x0060);
    ret += vtss8x58_write(phy_handle, 17, 0x0980);
    ret += vtss8x58_write(phy_handle, 16, 0x8f90);
    ret += vtss8x58_phy_page_std(phy_handle);

    if (phy_handle->phy_info.phy_addr % 8 == 0) 
    {
        /* disable iCPU */
        ret += vtss8x58_phy_page_gpio(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 0|0x10<<5, 0, 0x8000);
        ret += vtss8x58_phy_page_std(phy_handle);
    }
    /* Enable Smart Pre-emphasis */
    ret += vtss8x58_enab_smrt_premphasis(phy_handle);

    return ret;
}

int32 vtss8x58_phy_reset(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_type)
{
    int32 ret = 0;
    uint16 reg = 0;
    uint16 value, count=0;
    
    if(phy_handle->phy_info.phy_revision == 0)
    {
        /* BZ 2027 */
        ret += vtss8x58_phy_page_gpio(phy_handle);
        ret += vtss8x58_write(phy_handle, 0, 0x4c19);
        ret += vtss8x58_write_mask(phy_handle, 0|0x10<<5, 0, 0x8000);
        ret += vtss8x58_phy_page_std(phy_handle);
    }
    if(phy_handle->phy_info.part_num == VTSS_8558_PART_NUMBER)
    {
        switch(mac_if)
        {
            case GLB_MAC_INTERFACE_SGMII:
                reg = ((1<<13) | (0<<12));
                break;
            case GLB_MAC_INTERFACE_SERDES:
                reg = ((1<<13) | (1<<12));
                break;
            default:
                DRV_LOG_WARN("VSC8x58 mac interface [%d] is unknown.", mac_if);
                return RESULT_ERROR;
        }
        switch(media_type)
        {
            case GLB_MEDIA_RJ45:
                reg |= (0<<8);
                break;
            case GLB_MEDIA_PASS_THROUGH:
                reg |= (1<<8);
                break;
            case GLB_MEDIA_1000BX:
                reg |= (2<<8);
                break;
            case GLB_MEDIA_AMS_CU_PASSTHRU:
                reg |= (1<<11) | (5<<8);
                break;
            case GLB_MEDIA_AMS_FI_PASSTHRU:
                reg |= (0<<11) | (5<<8);
                break;
            case GLB_MEDIA_AMS_CU_1000BX:
                reg |= (1<<11) | (6<<8);
                break;
            case GLB_MEDIA_AMS_FI_1000BX:
                reg |= (0<<11) | (6<<8);
                break;    
            default:
                DRV_LOG_WARN("VSC8558 media interface [%d] is unknown.", media_type);
                return RESULT_ERROR;
        }
    }
    else if(phy_handle->phy_info.part_num == VTSS_8658_PART_NUMBER)
    {
        if(mac_if == GLB_MAC_INTERFACE_SGMII)
        {            
            reg = ((1<<13) | (0<<12));
            switch(media_type)
            {
                case GLB_MEDIA_RJ45:
                    reg |= (0<<8);
                    break;
                case GLB_MEDIA_PASS_THROUGH:
                    reg |= (1<<8);
                    break;
                case GLB_MEDIA_1000BX:
                    reg |= (2<<8);
                    break;
                case GLB_MEDIA_FI_100FX:
                    reg |= (3<<8);
                    break;    
                case GLB_MEDIA_AMS_CU_PASSTHRU:
                    reg |= (1<<11) | (5<<8);
                    break;
                case GLB_MEDIA_AMS_FI_PASSTHRU:
                    reg |= (0<<11) | (5<<8);
                    break;
                case GLB_MEDIA_AMS_CU_1000BX:
                    reg |= (1<<11) | (6<<8);
                    break;
                case GLB_MEDIA_AMS_FI_1000BX:
                    reg |= (0<<11) | (6<<8);
                    break;
                case GLB_MEDIA_AMS_CU_100FX:
                    reg |= (1<<11) | (7<<8);
                    break;
                case GLB_MEDIA_AMS_FI_100FX:
                    reg |= (0<<11) | (7<<8);
                    break;
                default:
                    DRV_LOG_WARN("VSC8658 media interface [%d] is unknown.", media_type);
                    return RESULT_ERROR;
            }
        }
        else if(mac_if == GLB_MAC_INTERFACE_SERDES)
        {
            reg = ((1<<13) | (1<<12));
            switch(media_type)
            {
                case GLB_MEDIA_RJ45:
                    reg |= (0<<8);
                    break;
                case GLB_MEDIA_PASS_THROUGH:
                    reg |= (1<<8);
                    break;
                case GLB_MEDIA_1000BX:
                    reg |= (2<<8);
                    break;
                case GLB_MEDIA_AMS_CU_PASSTHRU:
                    reg |= (1<<11) | (5<<8);
                    break;
                case GLB_MEDIA_AMS_FI_PASSTHRU:
                    reg |= (0<<11) | (5<<8);
                    break;
                case GLB_MEDIA_AMS_CU_1000BX:
                    reg |= (1<<11) | (6<<8);
                    break;
                case GLB_MEDIA_AMS_FI_1000BX:
                    reg |= (0<<11) | (6<<8);
                    break;
                default:
                    DRV_LOG_WARN("VSC8658 media interface [%d] is unknown.", media_type);
                    return RESULT_ERROR;    
            }
        }
        else
        {
            DRV_LOG_WARN("VSC8x58 mac interface [%d] is unknown.", mac_if);
            return RESULT_ERROR;
        }
    }
    ret += vtss8x58_write(phy_handle, EXTENDED_PHY_CTL1, reg);
    ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x8000, 0x8000);
    sal_udelay(40 * 1000);/* pause after reset */
    /* wait up to 5 seconds */
    while(1)
    {        
        ret += vtss8x58_read(phy_handle, PHY_CTL_REG, &value);
        if(!ret && (value & 0x8000) == 0)
        {
            break;
        }
        sal_udelay(20000);
        count++;
        if(count >= 250)
        {
            DRV_LOG_ERR("VSC8x58 PHY [addr: ]reset timeout!",phy_handle->phy_info.phy_addr);
            return RESULT_ERROR;
        }
    }
    vtss8x58_init_seq_spyder(phy_handle);
    return ret;
}

/*Now only use MDIX-Auto.*/
static int32 vtss8x58_phy_mdi_setup(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    ret += vtss8x58_write_mask(phy_handle, BYPASS_CTL, 0x0, 0xa0);
    ret += vtss8x58_phy_page_ext1(phy_handle);
    ret += vtss8x58_write_mask(phy_handle, 19, 0x0, 0xc);
    ret += vtss8x58_phy_page_std(phy_handle);
    return ret;
}

static int32 vtss8x58_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
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
        ret += vtss8x58_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0);
        ret += vtss8x58_write_mask(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300);        
        ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);        
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
	     /* Modified by liuht to powerdown more time to notify link parter for bug25796, 2013-11-14 */		
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0100, 0x3140);
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x2100, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x2000, 0x3140);
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0140, 0x3140);
        }
        else /*1G Half*/
        {    
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0040, 0x3140);
        }
        if(speed != GLB_SPEED_1G)
        {
            ret += vtss8x58_write_mask(phy_handle, 18, 0x0000, 0x0080);
        }
    }
    ret += vtss8x58_phy_mdi_setup(phy_handle);
    return ret;    
}

int32 vtss8x58_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = 0;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VTSS8x58 phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
    }
    else 
    {
        /*When do vct, not powerdown phy.bug25277*/
        if((phy_handle->phy_info.phy_veriphy_tsk != NULL)&&(phy_handle->phy_info.phy_veriphy_tsk->ret==GLB_VCT_WAIT))
        {            
        }
        else
        {
            /*power down*/
            ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
        }
    }
    return ret;
}

int32 vtss8x58_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media,
                                glb_port_master_slave_t master_slave)
{
    phy_info_t *pphy_info;    
    phyreg_param_t phyreg_para;
    uint16 vga_stat;
    int32 ret = 0, max_vga_state_to_optimize=-12;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vtss8x58_link_up_setting speed: %d duplex: %d media %d master-slave: %d", speed,duplex, media, master_slave);
    pphy_info = &phy_handle->phy_info;
    
    if(GLB_SPEED_1G == speed)
    {
        /* BZ 1776/1860/2095/2107 part 3/3 */
        ret += vtss8x58_phy_page_tr(phy_handle);
        ret += vtss8x58_write(phy_handle, 16, 0xaff0);
        ret += vtss8x58_read(phy_handle, 17, &vga_stat);
        vga_stat = (vga_stat >> 4) & 0x1f;
        if((vga_stat < 16) ||(vga_stat > (max_vga_state_to_optimize & 0x1f)))
        {
            ret += vtss8x58_phy_page_test(phy_handle);
            ret += vtss8x58_write_mask(phy_handle, 12, 0x0000, 0x0300);
        }
        ret += vtss8x58_phy_page_std(phy_handle);
    }
    else if(GLB_SPEED_100M == speed)
    {
        ret += vtss8x58_phy_page_test(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 20, 0x0000, 0x1000); /*- Clear Force Bit */
        ret += vtss8x58_write_mask(phy_handle, 20, 0x2000, 0x6000); /*- MDI Impedance offset -1 */
        
        ret += vtss8x58_phy_page_std(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL2, 0xa004, 0xe00e); /*- 10/100 Edge Rate/Amplitude Control */
    }
    else if(GLB_SPEED_10M == speed)
    {
        ret += vtss8x58_phy_page_test(phy_handle);
        ret += vtss8x58_write_mask(phy_handle, 20, 0x0000, 0x1000); /*- Clear Force Bit */
        ret += vtss8x58_write_mask(phy_handle,  20, 0x6000, 0x6000); /*- MDI Impedance offset -1 */
        ret += vtss8x58_phy_page_std(phy_handle);
    }

    if(GLB_PHY_LED_3 == pphy_info->led_type)
    {
        /*when linkup, for rj45, cfg led*/
        if(GLB_PORT_TYPE_COPPER == media)
        {
            phyreg_para.dat.regaddr8 = PHY_LED_MODE;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xee61);
        }
        else /* when linkup, for sfp, cfg led */
        {
            phyreg_para.dat.regaddr8 = PHY_LED_MODE;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe8eb);
        }
    }
    else if(GLB_PHY_LED_7 == pphy_info->led_type)
    {
        /*when linkup, for rj45, cfg led*/
        if(GLB_PORT_TYPE_COPPER == media)
        {
            phyreg_para.dat.regaddr8 = PHY_LED_MODE;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xee16);
            phyreg_para.dat.regaddr8 = PHY_LED_BEHAVIOR;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x8000);
        }
        else /* when linkup, for sfp, cfg led, default 1000M */
        {
            /* Modified by qicx for bug29756, combo port 100/1000 fiber LED issue, 2014-09-05 */
            if(phy_handle->phy_info.part_num == VTSS_8658_PART_NUMBER)
            {
                if(pphy_info->phy_manage_info.mac_if == GLB_MAC_INTERFACE_SGMII)
                {
                    switch(pphy_info->phy_manage_info.media_type)
                    {
                    case GLB_MEDIA_AMS_FI_1000BX:
                    case GLB_MEDIA_1000BX:
                        /* Adapt for GLB_PHY_LED_7, Fiber port 100M/1000M are two LEDs */
                        if (GLB_PHY_LED_7 == pphy_info->led_type)
                        {
                            phyreg_para.dat.regaddr8 = PHY_LED_MODE;
                            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe7ee);
                        }
                        break;
                    case GLB_MEDIA_AMS_FI_100FX:
                    case GLB_MEDIA_FI_100FX:
                        /* Adapt for GLB_PHY_LED_7, Fiber port 100M/1000M are two LEDs */
                        if (GLB_PHY_LED_7 == pphy_info->led_type)
                        {
                            phyreg_para.dat.regaddr8 = PHY_LED_MODE;
                            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x7eee);
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            phyreg_para.dat.regaddr8 = PHY_LED_BEHAVIOR;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x8000);
        }
    }
    return ret;
}

int32 
vtss8x58_link_down_setting(phy_handle_t* phy_handle)
{
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vtss8x58_link_down_setting ");

    /* BZ 1776/1860/2095/2107, part 1/3 and 2/3 */
    ret += vtss8x58_phy_page_test(phy_handle);
    ret += vtss8x58_write_mask(phy_handle, 12, 0x0200, 0x0300);
    ret += vtss8x58_phy_page_std(phy_handle);
    return ret;
} 

static int32 
vtss8x58_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
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
    if(phy_val->link_up == GLB_LINK_DOWN)
    {
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
        phy_val->link_up = (value&PHY_STATUS_LINK) ? GLB_LINK_UP : GLB_LINK_DOWN;
    }
    
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
            ret += vtss8x58_read(phy_handle, AUTO_NEGO_LP_ADVERTI_REG, &value);
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
            /*Note: vsc8558 datasheet have an error here for copper and fiber type define */
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
        ret += vtss8x58_phy_page_std(phy_handle);
        ret += vtss8x58_read(phy_handle, 0xa, &value);
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
        phy_val->master_slave= GLB_MODE_AUTO;
        /* End of liuht modified */
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }

    return ret;
}

int32 vtss8x58_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    /*fix bug 27981. When do veriPHY, the PHY register will not be availble(From VSC veriPHY guide), so we shall avoid 
      acccess register when do veriPHY. */
    if((NULL != (phy_handle->phy_info.phy_veriphy_tsk))&&(VTSS8XX8_VERIPHY_STATE_IDLE!=phy_handle->phy_info.phy_veriphy_tsk->task_state))
    {
        phy_val->link_up = pphy_info->phy_stat_flag.link_up;
        phy_val->speed = pphy_info->phy_stat_flag.speed;
        phy_val->duplex = pphy_info->phy_stat_flag.duplex;
        phy_val->linkup_media = pphy_info->phy_stat_flag.linkup_media;
        phy_val->flowctrl.send = pphy_info->phy_stat_flag.flowctrl.send;
        phy_val->flowctrl.recv = pphy_info->phy_stat_flag.flowctrl.recv;
        phy_change->action = -1;
        phy_change->speed_change = 0;
        phy_change->duplex_change = 0;
        phy_change->flowctrl_change = 0;
        phy_change->media_change = 0;
        return RESULT_OK;
    }
    vtss8x58_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8X58 phy addr %d link up",pphy_info->phy_addr); 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
        phy_change->flowctrl_change = 1;
        }

        vtss8x58_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8X58 phy addr %d link down",pphy_info->phy_addr); 
        vtss8x58_link_down_setting(phy_handle);
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
            vtss8x58_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
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
vtss8x58_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
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
    
    vtss8x58_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"vtss8x58_get_cur_status speed: %d duplex: %d master-slave: %d",phy_val->speed, phy_val->duplex, phy_val->master_slave);

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

        vtss8x58_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);

    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        vtss8x58_link_down_setting(phy_handle);
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
            vtss8x58_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
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

int32 vtss8x58_set_vct_mod(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    
    /*fix bug40964, Testing vct will lead to hsrvd crash, liangf, 2016-09-29*/
    if(NULL == phy_handle || (!phy_handle->phy_info.vct_support))
    {
        return RESULT_ERROR;
     }

    DRV_LOG_DEBUG(DRV_PHY,"set_vct_mod. ");    
    ret = vtss8xx8_veriphy_task_start(phy_handle, 0);

    return ret;
}


int32 vtss8x58_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
{
    uint16 max_length;
    phy_veriphy_result_t *result;
    int32 ret = 0;

    DRV_LOG_DEBUG(DRV_PHY,"vtss8x58_get_vct_info.");

    /*fix bug40964, Testing vct will lead to hsrvd crash, liangf, 2016-09-29*/
    if(NULL == p_cable_info || NULL == phy_handle ||(!phy_handle->phy_info.vct_support))
    {
        return RESULT_ERROR;
    }

    if(NULL == (result = &(phy_handle->phy_info.phy_veriphy_tsk->result))) 
    {        
        DRV_LOG_ERR("Get VCT info: invalid ptr!");
        return RESULT_ERROR;
    }
    
    ret = vtss8xx8_veriphy_get_result(phy_handle);
    /*After VCT, re-config enable.bug25277*/
    if(ret != GLB_VCT_WAIT)
    {
        vtss8x58_enable(phy_handle, phy_handle->phy_info.phy_manage_info.enable);
    }
    p_cable_info->port_link_up = result->link;
    p_cable_info->pair_A.pair_length = result->length[0];
    p_cable_info->pair_B.pair_length = result->length[1];
    p_cable_info->pair_C.pair_length = result->length[2];
    p_cable_info->pair_D.pair_length = result->length[3];
    p_cable_info->pair_A.pair_status = result->status[0];
    p_cable_info->pair_B.pair_status = result->status[1];
    p_cable_info->pair_C.pair_status = result->status[2];
    p_cable_info->pair_D.pair_status = result->status[3];


    p_cable_info->pair_A.pair_accuracy = (p_cable_info->pair_A.pair_status == GLB_PAIR_STATE_NORMAL) ? GLB_VCT_PAIR_NORMAL_ACCURACY : \
        (p_cable_info->pair_A.pair_status == GLB_PAIR_STATE_ABNORMAL_OPEN || p_cable_info->pair_A.pair_status == GLB_PAIR_STATE_ABNORMAL_SHORTED) ? \
        GLB_VCT_PAIR_OPEN_SHORT_ACCURACY : GLB_VCT_PAIR_CROSS_ACCURACY;
    p_cable_info->pair_B.pair_accuracy = (p_cable_info->pair_B.pair_status == GLB_PAIR_STATE_NORMAL) ? GLB_VCT_PAIR_NORMAL_ACCURACY : \
        (p_cable_info->pair_B.pair_status == GLB_PAIR_STATE_ABNORMAL_OPEN || p_cable_info->pair_B.pair_status == GLB_PAIR_STATE_ABNORMAL_SHORTED) ? \
        GLB_VCT_PAIR_OPEN_SHORT_ACCURACY : GLB_VCT_PAIR_CROSS_ACCURACY;
    p_cable_info->pair_C.pair_accuracy = (p_cable_info->pair_C.pair_status == GLB_PAIR_STATE_NORMAL) ? GLB_VCT_PAIR_NORMAL_ACCURACY : \
        (p_cable_info->pair_C.pair_status == GLB_PAIR_STATE_ABNORMAL_OPEN || p_cable_info->pair_C.pair_status == GLB_PAIR_STATE_ABNORMAL_SHORTED) ? \
        GLB_VCT_PAIR_OPEN_SHORT_ACCURACY : GLB_VCT_PAIR_CROSS_ACCURACY;
    p_cable_info->pair_D.pair_accuracy = (p_cable_info->pair_D.pair_status == GLB_PAIR_STATE_NORMAL) ? GLB_VCT_PAIR_NORMAL_ACCURACY : \
        (p_cable_info->pair_D.pair_status == GLB_PAIR_STATE_ABNORMAL_OPEN || p_cable_info->pair_D.pair_status == GLB_PAIR_STATE_ABNORMAL_SHORTED) ? \
        GLB_VCT_PAIR_OPEN_SHORT_ACCURACY : GLB_VCT_PAIR_CROSS_ACCURACY;
    
    if (p_cable_info->port_link_up)
    {
        p_cable_info->remote_pair = GLB_REMOTE_PAIR_PRESENT;
    }
    else
    {
        if (((0 == p_cable_info->pair_A.pair_length) && (0 == p_cable_info->pair_B.pair_length)
            && (0 == p_cable_info->pair_C.pair_length) && (0 == p_cable_info->pair_D.pair_length))
            || ((GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_A.pair_status) 
            && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_B.pair_status)
            && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_C.pair_status) 
            && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_D.pair_status)))
        {
            p_cable_info->remote_pair = GLB_REMOTE_PAIR_ABSENT;
        }
        else
        {
            p_cable_info->remote_pair = GLB_REMOTE_PAIR_PRESENT;
        }
    }

    if(GLB_PAIR_STATE_NORMAL == p_cable_info->pair_A.pair_status 
        && GLB_PAIR_STATE_NORMAL == p_cable_info->pair_B.pair_status
        && GLB_PAIR_STATE_NORMAL == p_cable_info->pair_C.pair_status
        && GLB_PAIR_STATE_NORMAL == p_cable_info->pair_D.pair_status)
    {
        max_length = p_cable_info->pair_A.pair_length;
        if(max_length < p_cable_info->pair_B.pair_length)
        {
            max_length = p_cable_info->pair_B.pair_length;
        }
        if(max_length < p_cable_info->pair_C.pair_length)
        {
            max_length = p_cable_info->pair_C.pair_length;
        }
        if(max_length < p_cable_info->pair_D.pair_length)
        {
            max_length = p_cable_info->pair_D.pair_length;
        }
        p_cable_info->pair_A.pair_length = max_length;
        p_cable_info->pair_B.pair_length = max_length;
        p_cable_info->pair_C.pair_length = max_length;
        p_cable_info->pair_D.pair_length = max_length;
    }
    return ret;
}

int32
vtss8x58_enable_syncE(phy_handle_t* phy_handle, int8 enable)
{
#if 0    
    phy_info_t* pphy_info;
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "phy addr %d syncE enable %d", pphy_info->phy_addr, enable);
    if(enable)
    {
        ret += vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0500, 0x07c0);
        ret += vtss8x58_reset(phy_handle);
        ret += vtss8x58_enable_link(phy_handle, GLB_SPEED_1G, GLB_DUPLEX_FULL);
        /*When work on pass-through mode, must disable auto. */
        ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x1000);
    }
    else
    {
        /* return back to original mode. */
        if(phy_handle->phy_info.phy_manage_info.mode == PHY_WORK_MODE_NORMAL)
        {
            ret = vtss8x58_config_media_type(phy_handle, phy_handle->phy_info.phy_manage_info.media_type, 
                                        phy_handle->phy_info.phy_manage_info.real_phy_media_mode);
        }
        else
        {
            ret = vtss8x58_config_workmode(phy_handle, phy_handle->phy_info.phy_manage_info.mode);
        }
    }
    return ret;
#endif
    return RESULT_OK;
}

int32
vtss8x58_get_chip_version(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    uint16 value;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    ret += vtss8x58_read(phy_handle, PHY_IDENTIFIER2_REG, &value);
    phy_handle->phy_info.phy_revision = value & 0xf;
    phy_handle->phy_info.part_num = ((value>>4) & 0x3f);
    return ret;
}

int32 vtss8x58_init(phy_handle_t* phy_handle)
{
    phy_info_t* pphy_info;    
    phyreg_param_t phyreg_para;
    int32 ret = 0;
    uint16 value;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VTSS8x58 Init phy addr %d", pphy_info->phy_addr);
    
    vtss8x58_get_chip_version(phy_handle);

    ret += vtss8x58_phy_reset(phy_handle, pphy_info->phy_manage_info.mac_if, pphy_info->phy_manage_info.media_type);
    ret += vtss8x58_conf_set(phy_handle, pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex);

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    
    if(phy_handle->phy_info.gpio_init)
    {
        /*cfg gpio mode*/
        phyreg_para.dat.regaddr8 = PHY_EXTENDED_PAGE;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x10);
        phyreg_para.dat.regaddr8 = 0xd;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xffff);
        phyreg_para.dat.regaddr8 = 0x11;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xff);
        /*cfg sigDet */
        phyreg_para.dat.regaddr8 = PHY_EXTENDED_PAGE;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x1);
        phyreg_para.dat.regaddr8 = 0x13;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x0);
    }
    /*Make sure select main register space by default*/
    phyreg_para.dat.regaddr8 = PHY_EXTENDED_PAGE;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x0);
    
    /*power down*/
    ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
    
    /*no sgmii preamble*/
    phyreg_para.dat.regaddr8 = EXTENDED_PHY_CTL2;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xc000);
    
    /*Config LED mode*/
    phyreg_para.dat.regaddr8 = PHY_LED_MODE;
    if(GLB_MEDIA_RJ45 == pphy_info->phy_manage_info.media_type)
    {
        if(GLB_PHY_LED_5== pphy_info->led_type)
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xdddd);       
        }
        else
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x7e61);
        }        
    }    
    else if(GLB_MEDIA_1000BX == pphy_info->phy_manage_info.media_type)
    {
        if(GLB_PHY_LED_2 == pphy_info->led_type)
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x7b61);       
        }
        else if(GLB_PHY_LED_1 == pphy_info->led_type)
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x7e61);
        }
        else
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe761);
        }        
    }
    else
    {
        if(GLB_PHY_LED_3 == pphy_info->led_type)
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xee61);
        }
        else if(GLB_PHY_LED_7 == pphy_info->led_type)
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xee16);
        }
        else    
        {
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe761);
        }
    }
    phyreg_para.dat.regaddr8 = PHY_LED_BEHAVIOR;

    if(GLB_PHY_LED_2 == pphy_info->led_type)
    {
         ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x800c);               
    }
    if(GLB_PHY_LED_5 == pphy_info->led_type)
    {
         ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x800f);               
    }
    else
    {
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x8000);
    }       

    /* added by liuht for parallel led for bug 29099, 2014-06-23 */
    if(GLB_PHY_LED_6 == pphy_info->led_type)
    {
        phyreg_para.dat.regaddr8 = PHY_LED_MODE;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0x0);
        phyreg_para.dat.regaddr8 = PHY_LED_BEHAVIOR;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0xe);
    }
    /* end of liuht added */

    
    /* added by tongzb for bug 42160, up the frequency of led 2016-10-16 */
    /* 2.5hz(0x000), 5hz(0x400,default), 10hz(0x800), 20hz(0xc00)*/
    /* added by tongzb, for config led freq, 2017-01-06 */
    if(GLB_PHY_LED_FREQ_DEFAULT == pphy_info->led_freq)
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_2P5 == pphy_info->led_freq)
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x000, 0xc00);
    else if(GLB_PHY_LED_FREQ_5 == pphy_info->led_freq)
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_10 == pphy_info->led_freq)
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x800, 0xc00);
    else if(GLB_PHY_LED_FREQ_20 == pphy_info->led_freq)
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0xc00, 0xc00);
    else
        vtss8x58_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    
    /*Clear interrupts*/
    phyreg_para.dat.regaddr8 = PHY_INT_STATUS;
    phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    /* open interrupt detect */
    phyreg_para.dat.regaddr8 = PHY_INT_MASK;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0xf000);
    
		
    return ret;
}


int32 vtss8x58_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8x58 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = vtss8x58_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}


int32 vtss8x58_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8x58 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = vtss8x58_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
int32
vtss8x58_config_master_slave(phy_handle_t* phy_handle, uint8 master_slave)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "vtss8x58 phy addr %d config master-slave mode %d, orig %d", pphy_info->phy_addr, master_slave, pphy_info->phy_manage_info.master_slave);
    ret += vtss8x58_phy_page_std(phy_handle);
    switch(master_slave)
    {
    case GLB_MODE_AUTO:
        ret |= vtss8x58_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x0000, 0x1000);
        break;
    case GLB_MODE_MASTER:
        ret |= vtss8x58_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1800, 0x1800);
        break;
    case GLB_MODE_SLAVE:
        ret |= vtss8x58_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1000, 0x1800);
        break;
    default:
        return RESULT_ERROR;
        break;
    }
    ret = vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    return ret;
}
/* End of liuht modified */

int32
vtss8x58_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = 0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VTSS8x58 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);
        break;

    case GLB_LB_PHY_FAR:
        ret |= vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0008, 0x0008);        
        break;

    case GLB_LB_PHY_NEAR:
        ret |= vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);         
        break;

    /* Added by liuht for bug 27508, 2014-06-30 */
    case GLB_LB_PHY_NEAR_DIAG:
        ret |= vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= vtss8x58_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  
        ret |= vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
        break;
    /* End of liuht added */

    default:
        return RESULT_ERROR;
    }

    return ret;
}


int32 vtss8x58_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8x58 phy addr %d config medium mac_if %d media_if %d", pphy_info->phy_addr, mac_if, media_if);
    ret = vtss8x58_phy_reset(phy_handle, mac_if, media_if);
    /* Bug 29767. When work on pass through mode, must disable auto, then led can work well. 
       GLB_MEDIA_AMS_CU_PASSTHRU   and GLB_MEDIA_AMS_FI_PASSTHRU not support. */
    if(media_if == GLB_MEDIA_PASS_THROUGH)
    {
        ret |= vtss8x58_conf_set(phy_handle, GLB_SPEED_1G, GLB_DUPLEX_FULL);
    }
    else
    {
        /*After medium config, speed/duplex/loopback need re-config. */
        ret |= vtss8x58_conf_set(phy_handle, pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex);
    }
    if(GLB_LB_NONE != pphy_info->phy_manage_info.lb_mode)
    {
        ret |= vtss8x58_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }
    /*Fix bug30023. After medium config, enable need re-config*/
    if(0 == pphy_info->phy_manage_info.enable)
    {
        ret |= vtss8x58_enable(phy_handle, 0);
    }
    return ret;
}

int32
vtss8x58_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
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
        "VSC8x58 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += vtss8x58_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += vtss8x58_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}

int32 vtss8x58_config_phy_sig_test_mode(phy_handle_t* phy_handle, uint8 serdes_id, uint8 mode)
{
    uint16 value;

    if(mode == PHY_SIG_TEST_MODE_NORMAL)
    {
        vtss8x58_write_mask(phy_handle, 9, 0x0000, 0xf800);
    }
    else
    {
        /*set 9 bit 15-13, select mode*/        
        if(mode == PHY_SIG_TEST_MODE_1)
            value = 0x2000;
        else if(mode == PHY_SIG_TEST_MODE_2)
            value = 0x4000;
        else if(mode == PHY_SIG_TEST_MODE_3)
            value = 0x6000;
        else 
            value = 0;
        vtss8x58_write_mask(phy_handle, 9, value, 0xe000);
    }
    return RESULT_OK;
}

phy_handle_t* phy_vtss8x58_dev_register(phy_info_t* pphy_info)
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

    sal_memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));
    if(phdl->phy_info.vct_support)
    {
        phdl->phy_info.phy_veriphy_tsk = (phy_veriphy_task_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(phy_veriphy_task_t));
        memset(phdl->phy_info.phy_veriphy_tsk, 0, sizeof(phy_veriphy_task_t));
        if(NULL == phdl->phy_info.phy_veriphy_tsk)
        {
            DRV_LOG_ERR("malloc failed");
            return NULL;
        }
    }
    else
    {
        phdl->phy_info.phy_veriphy_tsk = NULL;
    }

    phdl->phy_init = vtss8x58_init;
    phdl->config_enable = vtss8x58_enable;
    phdl->config_speed = vtss8x58_config_speed;
    phdl->config_duplex = vtss8x58_config_duplex;
    phdl->config_master_slave = vtss8x58_config_master_slave;
    phdl->config_loopback = vtss8x58_config_loopback;
    phdl->config_medium = vtss8x58_config_medium;
    phdl->config_flowctrl = vtss8x58_config_flowctrl;
    phdl->get_link_poll = vtss8x58_get_link_poll;
    phdl->get_link_interupt = vtss8x58_get_link_interrput;
    phdl->get_cur_status = vtss8x58_get_cur_status; /* modified by liuht for bug 26641, 2014-0422 */
    phdl->reg_read = vtss8x58_reg_read;
    phdl->reg_write = vtss8x58_reg_write;
    phdl->phy_set_vct_mod = vtss8x58_set_vct_mod;
    phdl->phy_get_vct_info = vtss8x58_get_vct_info;
    phdl->enable_syncE = vtss8x58_enable_syncE;
    phdl->config_phy_sig_test_mode = vtss8x58_config_phy_sig_test_mode;
    
    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type  = pphy_info->mdio_bus;
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

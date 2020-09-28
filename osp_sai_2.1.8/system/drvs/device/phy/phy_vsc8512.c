/****************************************************************************
* $Id$
* phy VSC8512 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-10-08
* Reason        : First Create.
*                2014-10-28    Update to VitesseAPI 4_60
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_vsc8512.h"
#include "phy_veriphy.h"

static uint16 tr_base_reg_write_cmd = 0x8486;
static uint16 tr_last_reg_write_cmd = 0x84b4;

#define VSC8512_ib_cterm_ena 0 /* AC coupled */
#define VTSS_FEATURE_SERDES_MACRO_SETTINGS

#ifdef VTSS_10BASE_TE // TODO:This macro will be defined when 10BASE-TE mode is supported
static uint8 tr_reg_bits_msb[] = 
{
    0x00,   0x65,   0x00,   0xeb,   0x00,   0x65,   0x59,   0x48,   //0x8486 - 0x8494
    0x00,   0xeb,   0x00,   0x55,   0x00,   0x00,   0x51,   0x30,   //0x8496 - 0x84a4
    0x10,   0x00,   0x00,   0x29,   0x79,   0x71,   0x61,   0x59    //0x84a6 - 0x84b4
};
static uint16 tr_reg_bits_lsw[] = 
{
    0xf410, 0xb719, 0x0914, 0xcefb, 0x0eb9, 0x6596, 0x2492, 0xa02a, //0x8486 - 0x8494
    0x0ab6, 0xcef9, 0x0caa, 0x5555, 0x0555, 0x0015, 0x240e, 0xa206, //0x8496 - 0x84a4
    0x2000, 0x0000, 0x0000, 0x2659, 0xf79d, 0xb699, 0x6596, 0x6596  //0x84a6 - 0x84b4
};
#else //System uses regular 10BASE-T to support Cat3 (and uglier) cables
static uint8 tr_reg_bits_msb[] = 
{
    0x20,   0x75,   0x00,   0xf7,   0x00,   0x49,   0x41,   0x40,   //0x8486 - 0x8494
    0x00,   0xf7,   0x00,   0x49,   0x00,   0x00,   0x38,   0x00,   //0x8496 - 0x84a4
    0x00,   0x00,   0x00,   0x29,   0x7d,   0x75,   0x41,   0x41    //0x84a6 - 0x84b4
};
static uint16 tr_reg_bits_lsw[] = 
{
    0xa497, 0xf71a, 0x0a14, 0xff7a, 0x0db2, 0x2410, 0x0410, 0xca39, //0x8486 - 0x8494
    0x0eff, 0xff7a, 0x0db2, 0x2492, 0x0410, 0x0010, 0xc244, 0x0000, //0x8496 - 0x84a4
    0x0000, 0x0000, 0x0000, 0x265d, 0xf7df, 0x9551, 0x0410, 0x0410  //0x84a6 - 0x84b4
};
#endif /* VTSS_10BASE_TE */
static int32
vsc8512_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex);

static int32 
vsc8512_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
{
    phy_info_t* pphy_info;
    int32 ret;
    mdio_bus_para_t para;
    //uint32 fpga_smi = 0;
    //uint32 val;

    if(NULL == phy_handle || NULL == phy_handle->mdio_hdl || NULL == value)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    sal_memset(&para, 0, sizeof(mdio_bus_para_t)); 

#if 0   /* On GBv6 Board, read/write PHY thru i2c/fpga */
    /******************************************************************************************
     * StartCmd1g   [1:0]	    0	29:28	R/W	Start of Frame                                *
     * OpCodeCmd1g  [1:0]	    0	27:26	R/W	Operation code: 2'b10: read; 2'b01: write бн  *
     * DevAddCmd1g  [4:0]	    0	25:21	R/W	Device address                                *
     * PortAddCmd1g [4:0]	    0	20:16	R/W	Port address                                  *
     * DataCmd1g    [15:0]	    0	15:0	R/W	Write data when MDIO write accesses           *
     ******************************************************************************************/
    fpga_smi |= ((phyreg_param->dat.regaddr8) & 0x1f) << 16;   /* Register address */
    fpga_smi |= ((pphy_info->phy_addr) & 0x1f) << 21;     /* PHY address */
    fpga_smi |= (0x2 & 0x3) << 26;                             /* OpCode, 2'b10: read */
    fpga_smi |= (0x1 & 0x3) << 28;                             /* SFD */
    ret += asic_i2c_reg_write(0, GBV6_I2C_WRITE_ASIC, fpga_smi);
    ret += asic_i2c_reg_read(0, GBV6_I2C_READ_ASIC, &val);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }
    *value = val & 0xffff;
#else  /* Otherwise, use mdio interface */
    para.gephy_para.phy_addr = pphy_info->phy_addr;
    para.gephy_para.reg = phyreg_param->dat.regaddr8;
    ret = phy_handle->mdio_hdl->read(phy_handle->mdio_hdl, &para);
    if(ret < 0)
    {
        return ret;
    }
     
    *value = para.gephy_para.val;
#endif
    //DRV_LOG_DEBUG(DRV_PHY,"VTSS8x58 phy addr %d: read reg 0x%x: 0x%x", 
    //                pphy_info->phy_addr, phyreg_param->dat.regaddr8, *value);

    return RESULT_OK;
}


static int32 
vsc8512_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
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

#if 0   /* On GBv6 Board, read/write PHY thru i2c/fpga */
    /******************************************************************************************
     * StartCmd1g   [1:0]	    0	29:28	R/W	Start of Frame                                *
     * OpCodeCmd1g  [1:0]	    0	27:26	R/W	Operation code: 2'b10: read; 2'b01: write бн  *
     * DevAddCmd1g  [4:0]	    0	25:21	R/W	Device address                                *
     * PortAddCmd1g [4:0]	    0	20:16	R/W	Port address                                  *
     * DataCmd1g    [15:0]	    0	15:0	R/W	Write data when MDIO write accesses           *
     ******************************************************************************************/
    fpga_smi |= value & 0xffff;
    fpga_smi |= ((phyreg_param->dat.regaddr8) & 0x1f) << 16;   /* Register address */
    fpga_smi |= ((pphy_info->phy_addr) & 0x1f) << 21;     /* PHY address */
    fpga_smi |= (0x1 & 0x3) << 26;                             /* OpCode, 2'b01: write */
    fpga_smi |= (0x1 & 0x3) << 28;                             /* SFD */
    ret = asic_i2c_reg_write(0, GBV6_I2C_WRITE_ASIC, fpga_smi);
#else  /* Otherwise, use mdio interface */

    para.gephy_para.phy_addr = pphy_info->phy_addr;
    para.gephy_para.reg = phyreg_param->dat.regaddr8;
    para.gephy_para.val = value;
    ret = phy_handle->mdio_hdl->write(phy_handle->mdio_hdl, &para);
#endif
    if(ret < 0)
    {
        return ret;
    }

    return RESULT_OK;
}

static int32
vsc8512_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
{
    phyreg_param_t phyreg_para;
    uint16 val;
    int32 ret = RESULT_OK;

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

static int32 vsc8512_read(phy_handle_t* phy_handle, uint16 reg, uint16 *value)
{
    phyreg_param_t phyreg_para;
    int32 ret = RESULT_OK;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}
static int32 vsc8512_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = RESULT_OK;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

static int32
vsc8512_phy_page_std(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x0);

    return ret;    
}

static int32
vsc8512_phy_page_ext1(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x1);

    return ret;
}

static int32
vsc8512_phy_page_ext2(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x2);

    return ret;
}

static int32
vsc8512_phy_page_ext3(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x3);

    return ret;
}


static int32
vsc8512_phy_page_tr(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x52B5); /* Token ring registers */

    return ret;
}

static int32
vsc8512_phy_page_test(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x2A30); /* Test registers */

    return ret;
}

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
static int32 vsc8512_mmd_reg_read(phy_handle_t* phy_handle, uint16 devaddr, uint16 reg, uint16 *value)
{
    int ret = 0;
    uint16 current_reg_val;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write(phy_handle, 13, devaddr);
    ret += vsc8512_write(phy_handle, 14, reg);
    ret += vsc8512_write(phy_handle, 13, (1 << 14) + devaddr);
    ret += vsc8512_read(phy_handle, 14, &current_reg_val);

    *value = current_reg_val;
    
    return ret;
}
/* End of liuht modified */

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
static int32 vsc8512_mmd_reg_write(phy_handle_t* phy_handle, uint16 devaddr, uint16 reg, uint16 value)
{
    int ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write(phy_handle, 13, devaddr);
    ret += vsc8512_write(phy_handle, 14, reg);
    ret += vsc8512_write(phy_handle, 13, (1 << 14) + devaddr);
    ret += vsc8512_write(phy_handle, 14, value);
    
    return ret;
}
/* End of liuht modified */

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
static int32 vsc8512_mmd_reg_wr_mask(phy_handle_t* phy_handle, uint16 devaddr, uint16 reg, uint16 value, uint16 mask)
{
    int32 ret = RESULT_OK;
    uint16 val;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    ret += vsc8512_mmd_reg_read(phy_handle, devaddr, reg, &val);
    value = (value & mask) | (val & ~mask);
    ret += vsc8512_mmd_reg_write(phy_handle, devaddr, reg, value);

    return ret;
}
/* End of liuht modified */

static int32
vsc8512_phy_page_gpio(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_write(phy_handle, PHY_EXTENDED_PAGE, 0x10);

    return ret;
}

/* Function that wait for the micro code to complete */
static int32
vsc8512_wait_for_micro_complete(phy_handle_t* phy_handle) 
{
    uint8  timeout = 200;    
    uint16 reg18g = 0;
    int32 ret = 0;
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    ret += vsc8512_phy_page_gpio(phy_handle);

    /* Wait for micro to complete MCB command (bit 15) */
    ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
    while (reg18g & 0x8000 && timeout > 0)
    {
        ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
        timeout--; /* Make sure that we don't run forever */
        sal_udelay(1000); 
        DRV_LOG_DEBUG(DRV_PHY, "MCB timer, phy_addr: 0x%x, timeout = %d", 
            phy_handle->phy_info.phy_addr, timeout); 
    }
    
    if (timeout == 0) {
        DRV_LOG_DEBUG(DRV_PHY, "MCB timeout, phy_addr 0x%x", phy_handle->phy_info.phy_addr); 
        ret = RESULT_ERROR;
    }

    ret += vsc8512_phy_page_std(phy_handle);
    return ret;
}

/*
 * Function that is called any time the micro reset is being asserted
 * This function specifically avoids bugzilla #5731
 * IN : port_no - Any port within the chip.
 */
static int32
vsc8512_micro_assert_reset(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    // Bugzilla#5731 - The following code-sequence is needed for Luton26 Rev. A and B where
    // a consumptive patch disables interrupts inside a micro-command patch.

    // Set to micro/GPIO-page
    ret = vsc8512_phy_page_gpio(phy_handle);

    //----------------------------------------------------------------------
    // Pass the NOP cmd to Micro to insure that any consumptive patch exits
    // There is no issue with doing this on any revision since it is just a NOP on any Vitesse PHY.
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x800f);
    
    // Poll on 18G.15 to clear
    ret += vsc8512_wait_for_micro_complete(phy_handle);
    //----------------------------------------------------------------------

    // Set to micro/GPIO-page (Has been set to std page by vtss_phy_wait_for_micro_complete
    ret += vsc8512_phy_page_gpio(phy_handle);

    // force micro into a loop, preventing any SMI accesses
    ret += vsc8512_write_mask(phy_handle, 12, 0x0000, 0x0800); // Disable patch vector 3 (just in case)
    ret += vsc8512_write(phy_handle,  9, 0x005b);     // Setup patch vector 3 to trap MicroWake interrupt
    ret += vsc8512_write(phy_handle, 10, 0x005b);     // Loop forever on MicroWake interrupts
    ret += vsc8512_write_mask(phy_handle, 12, 0x0800, 0x0800); // Enable patch vector 3
    ret += vsc8512_write(phy_handle, 18, 0x800f);     // Trigger MicroWake interrupt to make safe to reset

    // Assert reset after micro is trapped in a loop (averts micro-SMI access deadlock at reset)
    ret += vsc8512_write_mask(phy_handle, 0, 0x0000, 0x8000);
    ret += vsc8512_write(phy_handle, 18, 0x0000);     // Make sure no MicroWake persists after reset
    ret += vsc8512_write_mask(phy_handle, 12, 0x0000, 0x0800); // Disable patch vector 3

    ret += vsc8512_phy_page_std(phy_handle);

    return ret;
}


/*
 * Function for downloading code into the internal 8051 CPU.
 *     code_array - Pointer to array containing the 8051 code
 *     code_size  - The size of the code to be downloaded.
 * Return : RESULT_OK if download was done else error code. 
 */
static int32
vsc8512_download_8051_code(phy_handle_t* phy_handle, uint8 const *code_array, uint16 code_size) 
{
    uint16 i;
    int32 ret = 0;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    /*
     * Note that the micro/GPIO-page, Reg31=0x10, is a global page, one per PHY chip
     * thus even though broadcast is turned off, it is still sufficient to do once`
     */

    /* Hold the micro in reset during patch download */
    ret += vsc8512_micro_assert_reset(phy_handle);
    ret += vsc8512_phy_page_gpio(phy_handle);
    /*  Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock */
    ret += vsc8512_write(phy_handle, 0, 0x7009);

    ret += vsc8512_write(phy_handle, 12, 0x5002);     // write to addr 4000= 02
    ret += vsc8512_write(phy_handle, 11, 0x0000);     // write to address reg.

    for(i = 0; i < code_size; i++)
    {
        ret += vsc8512_write(phy_handle, 12, 0x5000 | code_array[i]);   
    }

    ret += vsc8512_write(phy_handle, 12, 0x0000);     // Clear internal memory access
    return ret;
}

/*
 * Atom  Rev. A. Internal 8051 patch.(From James M, 13-10-2010)
 * In     : port_no - any port within the chip where to load the 8051 code.
 * Return : VTSS_RC_OK if configuration done else error code. 
 */

/* Download patch into PRAM for the internal 8051 */
static int32
vsc8512_8051_patch_13_10_2010(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    const uint8 vga_arr[] = {
        0x45, 0xe9, 0x02, 0x45, 0x16, 0x02, 0x40, 0xeb, 0x02, 0x45,
        0xa5, 0x90, 0x47, 0xd3, 0xe0, 0x54, 0x7f, 0x44, 0x10, 0xf0,
        0x90, 0x47, 0xd7, 0xe0, 0x54, 0xdf, 0x44, 0x1c, 0xf0, 0xa3,
        0xe0, 0x54, 0x7f, 0xf0, 0xa3, 0xe0, 0x54, 0xfe, 0x44, 0xe6,
        0xf0, 0xa3, 0xe0, 0x54, 0xc6, 0x44, 0xd0, 0xf0, 0xa3, 0xe0,
        0x54, 0xbf, 0x44, 0x31, 0xf0, 0xa3, 0xa3, 0xe5, 0x51, 0x30,
        0xe0, 0x0e, 0xe0, 0x54, 0xbe, 0x44, 0x86, 0xf0, 0xa3, 0xa3,
        0xe0, 0x44, 0x02, 0xf0, 0x80, 0x06, 0xe0, 0x54, 0xfe, 0x44,
        0x06, 0xf0, 0x12, 0x2c, 0xbb, 0x90, 0x47, 0xd5, 0xe0, 0x44,
        0x80, 0xf0, 0x12, 0x27, 0x12, 0x02, 0x2c, 0xb4, 0x90, 0x47,
        0xd2, 0xe0, 0x44, 0x02, 0xf0, 0x90, 0x47, 0xd7, 0xe0, 0x54,
        0x1f, 0xf0, 0xa3, 0xe0, 0x54, 0xfe, 0x44, 0x06, 0xf0, 0xa3,
        0xe0, 0x54, 0xdf, 0x44, 0x1c, 0xf0, 0x90, 0x47, 0xdc, 0xe0,
        0x44, 0x58, 0xf0, 0xa3, 0xe5, 0x50, 0x30, 0xe0, 0x0e, 0xe0,
        0x54, 0xbb, 0x44, 0x83, 0xf0, 0xa3, 0xe0, 0x44, 0x71, 0x54,
        0x7d, 0x80, 0x0a, 0xe0, 0x44, 0xc7, 0xf0, 0xa3, 0xe0, 0x44,
        0x73, 0x54, 0x7f, 0xf0, 0xa3, 0xe0, 0x44, 0xc0, 0xf0, 0xa3,
        0xe5, 0x50, 0x30, 0xe0, 0x07, 0xe0, 0x54, 0xdb, 0x44, 0x9b,
        0x80, 0x03, 0xe0, 0x44, 0xbf, 0xf0, 0xa3, 0x74, 0x24, 0xf0,
        0xa3, 0xe0, 0x54, 0xdf, 0x44, 0x50, 0xf0, 0x12, 0x29, 0xfe,
        0x90, 0x47, 0xd6, 0xe0, 0x44, 0x08, 0xf0, 0x12, 0x27, 0x12,
        0x90, 0x47, 0xdc, 0xe0, 0x54, 0xbf, 0xf0, 0x02, 0x29, 0xf7,
        0x04, 0x76, 0x05, 0xf8, 0x76, 0x02, 0x75, 0x51, 0x02, 0x74,
        0x03, 0x02, 0x06, 0xa6, 0xd5, 0x25, 0x03, 0x75, 0x25, 0x0c,
        0xc2, 0x01, 0x7b, 0xbb, 0x7d, 0x0e, 0x7f, 0x04, 0x12, 0x31,
        0x16, 0xef, 0x60, 0x03, 0x02, 0x44, 0xbf, 0xf5, 0x18, 0x75,
        0x1d, 0xa1, 0xe5, 0x18, 0xc3, 0x94, 0x0c, 0x40, 0x03, 0x02,
        0x44, 0xbf, 0xa8, 0x1d, 0xe6, 0x20, 0xe7, 0x03, 0x02, 0x44,
        0xb8, 0xd2, 0x01, 0x85, 0x18, 0xfb, 0x7f, 0x80, 0x7e, 0x06,
        0x12, 0x2f, 0x0b, 0xe5, 0x4a, 0x54, 0x07, 0x60, 0x05, 0xe5,
        0x4a, 0x30, 0xe2, 0x06, 0x75, 0x24, 0x88, 0x02, 0x44, 0xb4,
        0x7d, 0x1c, 0xe4, 0xff, 0x12, 0x31, 0xee, 0x8e, 0x21, 0x8f,
        0x22, 0xe5, 0x21, 0x13, 0x13, 0x13, 0x54, 0x0c, 0x44, 0x80,
        0xf5, 0x24, 0xe4, 0xf5, 0x19, 0x7f, 0x82, 0x7e, 0x07, 0x12,
        0x2f, 0x0b, 0xe5, 0x4a, 0x54, 0x3f, 0x24, 0xfd, 0x60, 0x4b,
        0x24, 0xf7, 0x60, 0x39, 0x24, 0xdc, 0x70, 0x5b, 0xa8, 0x1d,
        0xe6, 0x54, 0x03, 0x64, 0x03, 0x60, 0x1d, 0x7d, 0x0a, 0xe4,
        0xff, 0x12, 0x31, 0xee, 0xee, 0x30, 0xe6, 0x08, 0x43, 0x24,
        0x0b, 0x75, 0x19, 0x0d, 0x80, 0x6e, 0xe5, 0x24, 0x54, 0x04,
        0x44, 0x83, 0xf5, 0x24, 0x80, 0x07, 0xa8, 0x1d, 0xe6, 0x54,
        0x8f, 0xf5, 0x24, 0xe5, 0x4b, 0x54, 0x0c, 0x70, 0x57, 0x80,
        0x52, 0x43, 0x24, 0x02, 0xe5, 0x4b, 0x54, 0x03, 0x70, 0x08,
        0x75, 0x19, 0x03, 0x80, 0x03, 0x43, 0x24, 0x01, 0xa8, 0x1d,
        0xe6, 0xff, 0x65, 0x24, 0x54, 0x03, 0x60, 0x05, 0x75, 0x19,
        0x09, 0x80, 0x35, 0xef, 0x20, 0xe4, 0x31, 0x80, 0x2c, 0xe5,
        0x4a, 0x54, 0x0c, 0x64, 0x04, 0x70, 0x24, 0x43, 0x24, 0x20,
        0xa8, 0x1d, 0xe6, 0x30, 0xe5, 0x1b, 0x7f, 0xf8, 0x7e, 0x0f,
        0x12, 0x2f, 0x0b, 0xe5, 0x4a, 0x54, 0x0f, 0x64, 0x03, 0x60,
        0x04, 0x7f, 0x01, 0x80, 0x02, 0x7f, 0x00, 0xef, 0x60, 0x03,
        0x43, 0x24, 0x40, 0x75, 0x19, 0x03, 0xe5, 0x24, 0x20, 0xe5,
        0x14, 0xa8, 0x1d, 0xe6, 0x54, 0x60, 0xff, 0xbf, 0x20, 0x0b,
        0xe5, 0x24, 0x54, 0x03, 0x64, 0x02, 0x60, 0x03, 0x43, 0x24,
        0x40, 0xa8, 0x1d, 0xe5, 0x24, 0x44, 0x10, 0xff, 0x66, 0x70,
        0x0c, 0x8f, 0x24, 0xe5, 0x25, 0x14, 0x65, 0x18, 0x60, 0x03,
        0xe4, 0xf5, 0x19, 0x74, 0x95, 0x25, 0x18, 0xf5, 0x1e, 0xe5,
        0x24, 0x54, 0x03, 0xff, 0xbf, 0x03, 0x05, 0x75, 0x1a, 0xff,
        0x80, 0x0d, 0xe5, 0x24, 0x30, 0xe2, 0x04, 0x7f, 0x11, 0x80,
        0x02, 0x7f, 0x22, 0x8f, 0x1a, 0xe4, 0xfd, 0x7f, 0x01, 0x12,
        0x31, 0xee, 0xef, 0xf5, 0x17, 0xaf, 0x19, 0x15, 0x19, 0xef,
        0xd3, 0x94, 0x00, 0x50, 0x03, 0x02, 0x44, 0x94, 0xe5, 0x18,
        0x54, 0x0c, 0xc4, 0x54, 0xf0, 0x44, 0x20, 0xfa, 0x7b, 0x00,
        0x7d, 0x1c, 0x7f, 0x04, 0x12, 0x32, 0x06, 0xe4, 0xf5, 0x1b,
        0xf5, 0x1c, 0x74, 0x01, 0x7e, 0x00, 0xa8, 0x1c, 0x08, 0x80,
        0x05, 0xc3, 0x33, 0xce, 0x33, 0xce, 0xd8, 0xf9, 0xff, 0xef,
        0x55, 0x1a, 0x70, 0x03, 0x02, 0x43, 0x13, 0xe5, 0x1c, 0x75,
        0xf0, 0x40, 0xa4, 0x24, 0x08, 0xff, 0xe4, 0x35, 0xf0, 0xef,
        0xfa, 0xe5, 0x17, 0xfb, 0xe4, 0xfd, 0x7f, 0x01, 0x12, 0x32,
        0x06, 0xe5, 0x18, 0x54, 0x0c, 0xf5, 0xfb, 0x7d, 0x03, 0x7f,
        0x01, 0x12, 0x31, 0xee, 0xee, 0x54, 0x80, 0xf5, 0x21, 0xef,
        0x54, 0x1f, 0xf5, 0x22, 0xe5, 0x21, 0x44, 0x04, 0x12, 0x44,
        0xd8, 0xf5, 0x23, 0xe5, 0x21, 0x44, 0x20, 0x12, 0x44, 0xd8,
        0x25, 0x23, 0xf5, 0x27, 0xe4, 0x33, 0xf5, 0x26, 0xc3, 0xe5,
        0x27, 0x94, 0xea, 0xe5, 0x26, 0x94, 0x01, 0x50, 0x0f, 0xaf,
        0x1c, 0x74, 0x01, 0xa8, 0x07, 0x08, 0x80, 0x02, 0xc3, 0x33,
        0xd8, 0xfc, 0x42, 0x1b, 0xab, 0x22, 0xaa, 0x21, 0x7d, 0x03,
        0x7f, 0x01, 0x12, 0x32, 0x06, 0x85, 0x18, 0xfb, 0x30, 0x02,
        0x14, 0xe5, 0x1c, 0x54, 0x02, 0xff, 0xe5, 0x1c, 0x24, 0x19,
        0x2f, 0xfd, 0xab, 0x27, 0xaa, 0x26, 0x7f, 0x03, 0x12, 0x32,
        0x06, 0xe5, 0x17, 0x12, 0x44, 0xff, 0x05, 0x1c, 0xe5, 0x1c,
        0xc3, 0x94, 0x04, 0x50, 0x03, 0x02, 0x42, 0x6f, 0xe5, 0x1a,
        0x54, 0x0f, 0xb5, 0x1b, 0x0a, 0x43, 0x24, 0x10, 0xa8, 0x1e,
        0xe4, 0xf6, 0x02, 0x44, 0x94, 0xe5, 0x19, 0x70, 0x58, 0xa8,
        0x1e, 0x06, 0xe6, 0xc3, 0x94, 0x50, 0x50, 0x03, 0x02, 0x44,
        0x94, 0xe5, 0x24, 0x54, 0x0b, 0xff, 0x60, 0x0c, 0x64, 0x09,
        0x60, 0x08, 0xef, 0x64, 0x0a, 0x60, 0x03, 0x02, 0x44, 0x94,
        0x30, 0x02, 0x15, 0x7d, 0x18, 0x7f, 0x03, 0x12, 0x31, 0xee,
        0xef, 0x24, 0x01, 0xfb, 0xe4, 0x33, 0xfa, 0x7d, 0x18, 0x7f,
        0x03, 0x12, 0x32, 0x06, 0xe5, 0x24, 0x7a, 0x1b, 0x30, 0xe2,
        0x04, 0x7b, 0x30, 0x80, 0x02, 0x7b, 0x38, 0x7d, 0x05, 0x7f,
        0x01, 0x12, 0x32, 0x06, 0x7b, 0x20, 0x7a, 0x1b, 0x7d, 0x05,
        0x7f, 0x01, 0x12, 0x32, 0x06, 0x80, 0x9d, 0xc2, 0x00, 0xe5,
        0x24, 0x54, 0x03, 0x24, 0xfe, 0x60, 0x26, 0x14, 0x70, 0x45,
        0xe5, 0x1b, 0x75, 0xf0, 0x10, 0xa4, 0xff, 0xad, 0x1b, 0x7c,
        0x00, 0xe5, 0xf0, 0xfa, 0xed, 0x4f, 0xfb, 0xed, 0xc4, 0x54,
        0xf0, 0x4a, 0xf5, 0x26, 0xeb, 0xf5, 0x27, 0x75, 0x21, 0x00,
        0x75, 0x22, 0x0f, 0x80, 0x5c, 0xe5, 0x24, 0x30, 0xe2, 0x06,
        0x7e, 0xec, 0x7f, 0x00, 0x80, 0x04, 0x7e, 0xdc, 0x7f, 0x00,
        0x8e, 0x26, 0x8f, 0x27, 0xe5, 0x24, 0x7e, 0x00, 0x30, 0xe2,
        0x04, 0x7f, 0x01, 0x80, 0x23, 0x7f, 0x02, 0x80, 0x1f, 0xe4,
        0xf5, 0x26, 0xf5, 0x27, 0xe5, 0x24, 0x20, 0xe5, 0x06, 0xa8,
        0x1e, 0xe6, 0x30, 0xe3, 0x15, 0xe5, 0x24, 0x30, 0xe2, 0x06,
        0x7e, 0xce, 0x7f, 0x81, 0x80, 0x04, 0x7e, 0x3d, 0x7f, 0x42,
        0x8e, 0x21, 0x8f, 0x22, 0x80, 0x15, 0xe5, 0x24, 0x30, 0xe2,
        0x06, 0x7e, 0xce, 0x7f, 0x90, 0x80, 0x04, 0x7e, 0x3d, 0x7f,
        0x60, 0x8e, 0x21, 0x8f, 0x22, 0xd2, 0x00, 0x12, 0x45, 0x0a,
        0x12, 0x44, 0xf3, 0x7d, 0x1c, 0xe4, 0xff, 0x12, 0x31, 0xee,
        0xee, 0x13, 0x13, 0x13, 0x54, 0x1f, 0xff, 0xef, 0x65, 0x24,
        0x20, 0xe2, 0x62, 0x7b, 0x00, 0x7a, 0x80, 0x7d, 0x0f, 0x7f,
        0x01, 0x12, 0x32, 0x06, 0xe5, 0x17, 0x44, 0x01, 0x12, 0x44,
        0xff, 0x7f, 0x1e, 0x12, 0x45, 0xd6, 0xe5, 0x1a, 0x42, 0x27,
        0xe4, 0x12, 0x44, 0xf3, 0x7f, 0x14, 0x12, 0x45, 0xd6, 0x30,
        0x00, 0x05, 0x43, 0x21, 0x0f, 0x80, 0x08, 0xe5, 0x1a, 0x54,
        0xf0, 0xfe, 0xee, 0x42, 0x26, 0xab, 0x27, 0xaa, 0x26, 0x7d,
        0x0d, 0x7f, 0x01, 0x12, 0x32, 0x06, 0x12, 0x45, 0x0a, 0x7f,
        0x1e, 0x12, 0x45, 0xd6, 0x7a, 0x00, 0xe5, 0x17, 0x54, 0xfe,
        0xfb, 0xe4, 0xfd, 0x7f, 0x01, 0x12, 0x32, 0x06, 0xe4, 0xfb,
        0xfa, 0x7d, 0x0f, 0x7f, 0x01, 0x12, 0x32, 0x06, 0x02, 0x42,
        0x4a, 0xe5, 0x24, 0x30, 0xe1, 0x0a, 0xa8, 0x1d, 0xe6, 0x20,
        0xe1, 0x15, 0x7f, 0x02, 0x80, 0x0e, 0xe5, 0x24, 0x54, 0x60,
        0xff, 0xbf, 0x20, 0x04, 0x7f, 0x01, 0x80, 0x02, 0xe4, 0xff,
        0x12, 0x45, 0x7d, 0xa8, 0x1d, 0xa6, 0x24, 0x05, 0x18, 0x05,
        0x1d, 0x02, 0x41, 0x07, 0x30, 0x01, 0x0e, 0x7b, 0x00, 0x7a, 
        0x40, 0x7d, 0x1c, 0x7f, 0x04, 0x12, 0x32, 0x06, 0x12, 0x45,
        0xc0, 0xd0, 0x23, 0xd0, 0x23, 0x02, 0x30, 0x2c, 0x22, 0xfa,
        0xab, 0x22, 0x7d, 0x03, 0x7f, 0x01, 0x12, 0x32, 0x06, 0x12,
        0x45, 0xc0, 0x7f, 0x06, 0x12, 0x45, 0xd6, 0x7d, 0x1c, 0x7f,
        0x04, 0x12, 0x31, 0xee, 0xef, 0x22, 0xab, 0x27, 0xaa, 0x26,
        0x7d, 0x0d, 0x7f, 0x01, 0x12, 0x32, 0x06, 0x22, 0xfb, 0x7a,
        0x00, 0xe4, 0xfd, 0x7f, 0x01, 0x12, 0x32, 0x06, 0x22, 0xab,
        0x22, 0xaa, 0x21, 0x7d, 0x0e, 0x7f, 0x01, 0x12, 0x32, 0x06,
        0x22, 0x7d, 0x12, 0x7f, 0x04, 0x12, 0x31, 0xee, 0x8e, 0x21,
        0x8f, 0x22, 0xe5, 0x22, 0x54, 0x0f, 0x24, 0xfb, 0x60, 0x3e,
        0x04, 0x70, 0x42, 0xe5, 0x21, 0x54, 0x0f, 0xf5, 0x23, 0x24,
        0xa1, 0xf5, 0x1f, 0xe5, 0x22, 0x64, 0x24, 0x70, 0x08, 0x85,
        0x23, 0xfb, 0x7f, 0x02, 0x12, 0x45, 0x7d, 0xe5, 0x22, 0x64,
        0x04, 0xa8, 0x1f, 0x70, 0x06, 0x74, 0x80, 0x46, 0xf6, 0x80,
        0x04, 0x74, 0x7f, 0x56, 0xf6, 0x05, 0x1f, 0x05, 0x23, 0xe5,
        0x23, 0xc3, 0x94, 0x0c, 0x50, 0x1b, 0xe5, 0x21, 0x20, 0xe4,
        0xd0, 0x22, 0xe5, 0x22, 0xc4, 0x13, 0x92, 0x02, 0x22, 0xe5,
        0x21, 0x44, 0x40, 0xfa, 0xab, 0x22, 0x7d, 0x12, 0x7f, 0x04,
        0x12, 0x32, 0x06, 0x22, 0x8f, 0x2a, 0xe4, 0xf5, 0x4b, 0xf5,
        0x4c, 0xb5, 0x2a, 0x01, 0x04, 0xf5, 0x4a, 0x7f, 0x88, 0x7e,
        0x0f, 0x12, 0x31, 0x67, 0x7b, 0x80, 0x7a, 0x99, 0xe5, 0x2a,
        0x30, 0xe1, 0x04, 0x7b, 0x00, 0x7a, 0x00, 0x7d, 0x17, 0x7f,
        0x01, 0x02, 0x32, 0x06, 0xe5, 0xed, 0xc4, 0x54, 0x0f, 0xf5,
        0xed, 0x85, 0xea, 0xe9, 0xe4, 0xf5, 0xed, 0xf5, 0xe9, 0xc2,
        0x61, 0x53, 0x91, 0xdf, 0xd2, 0xe9, 0xd0, 0x23, 0xd0, 0x23,
        0x22, 0x7b, 0xc0, 0x7a, 0x00, 0x7d, 0x1a, 0x7f, 0x04, 0x12,
        0x32, 0x06, 0x7b, 0x80, 0x7a, 0x00, 0x7d, 0x1a, 0x7f, 0x04,
        0x02, 0x32, 0x06, 0x8f, 0x28, 0x75, 0x29, 0x07, 0x7d, 0x1c,
        0xe4, 0xff, 0x12, 0x31, 0xee, 0xd5, 0x29, 0xf6, 0xd5, 0x28,
        0xf0, 0x22, 0x78, 0xa1, 0x79, 0x95, 0x7d, 0x0c, 0x76, 0x88,
        0x77, 0x00, 0x08, 0x09, 0xdd, 0xf8, 0xc2, 0x02, 0x22,
        0};
     
    ret += vsc8512_download_8051_code(phy_handle, &vga_arr[0], sizeof(vga_arr));
    return ret;
}

/*
 * Init. script for Luton26/Atom12 PHY
 * Turn on broadcast writes so that all PHYs within an Atom12 or Luton26
 * package are initialized at the same time
 */
static int32
vsc8512_vga_patch_27_09_2010(phy_handle_t* phy_handle)
{
	int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    // Init. script for Luton26/Atom12 PHY
//----------------------------------------------------------------------------------
// Derived from matlab script in
// /group/val/cvs/luton26_atom12/matlab/scripts/luton26_atom12/revA/init_script.m,v
// at the following CVS revision:
//
// with the micro patch coming from
// /group/val/cvs/luton26_atom12/matlab/scripts/luton26_atom12/revA/vga_patch.m,v
// at the following CVS revision:
//----------------------------------------------------------------------------------

// Turn on broadcast writes so that all PHYs within an Atom12 or Luton26
// package are initialized at the same time    
    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write(phy_handle, 22, 0x3201);

    ret += vsc8512_phy_page_test(phy_handle); // Switch to test-register page
    ret += vsc8512_write(phy_handle, 22, 0x0088); // Set band-gap, CAC, and VAC trims
    ret += vsc8512_write(phy_handle, 24, 0x0c00); // Set input bias current to the 10 setting & Set 1000BASE-T edge-rate to 000

    ret += vsc8512_phy_page_std(phy_handle); // Switch to main register page
    ret += vsc8512_write(phy_handle, 24, 0x0040); // Set 100BASE-TX edge-rate to 000

    ret += vsc8512_phy_page_ext2(phy_handle); // Switch to extended-page 2
    ret += vsc8512_write(phy_handle, 16, 0x67f0); // Set Cu default amplitude

    ret += vsc8512_phy_page_test(phy_handle); // Switch to test-register page
    //vsc8512_write(phy_handle,  0, 0x0055); // Disable VGA start-up circuit
    ret += vsc8512_write(phy_handle,  8, 0x8012); // Enable token-ring during coma-mode

    ret += vsc8512_phy_page_tr(phy_handle); // Switch to token-ring register page
    ret += vsc8512_write(phy_handle, 18, 0xb);
    ret += vsc8512_write(phy_handle, 17, 0xa0);
    ret += vsc8512_write(phy_handle, 16, 0x8fe0);
    ret += vsc8512_write(phy_handle, 18, 0xf);
    ret += vsc8512_write(phy_handle, 17, 0x8f87);
    ret += vsc8512_write(phy_handle, 16, 0x97d6);
    ret += vsc8512_write(phy_handle, 18, 0x5);
    ret += vsc8512_write(phy_handle, 17, 0x1900);
    ret += vsc8512_write(phy_handle, 16, 0x8fe4);
    ret += vsc8512_write(phy_handle, 18, 0x1);
    ret += vsc8512_write(phy_handle, 17, 0x9689);
    ret += vsc8512_write(phy_handle, 16, 0x8f92);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0xc);
    ret += vsc8512_write(phy_handle, 16, 0x8fe2);
    ret += vsc8512_write(phy_handle, 18, 0xd);
    ret += vsc8512_write(phy_handle, 17, 0x292a);
    ret += vsc8512_write(phy_handle, 16, 0x8fa4);
    ret += vsc8512_write(phy_handle, 18, 0xd);
    ret += vsc8512_write(phy_handle, 17, 0x1c);
    ret += vsc8512_write(phy_handle, 16, 0x8fac);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x2160);
    ret += vsc8512_write(phy_handle, 16, 0x8258);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x2960);
    ret += vsc8512_write(phy_handle, 16, 0x825a);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x3000);
    ret += vsc8512_write(phy_handle, 16, 0x825c);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x38b0);
    ret += vsc8512_write(phy_handle, 16, 0x825e);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x2150);
    ret += vsc8512_write(phy_handle, 16, 0x8250);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x28d3);
    ret += vsc8512_write(phy_handle, 16, 0x8252);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x37dd);
    ret += vsc8512_write(phy_handle, 16, 0x8254);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x3fdd);
    ret += vsc8512_write(phy_handle, 16, 0x8256);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x2150);
    ret += vsc8512_write(phy_handle, 16, 0x8248);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x28d3);
    ret += vsc8512_write(phy_handle, 16, 0x824a);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x37dd);
    ret += vsc8512_write(phy_handle, 16, 0x824c);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x3fdd);
    ret += vsc8512_write(phy_handle, 16, 0x824e);
    ret += vsc8512_write(phy_handle, 18, 0x1);
    ret += vsc8512_write(phy_handle, 17, 0x40bb);
    ret += vsc8512_write(phy_handle, 16, 0x8fda);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x15b);
    ret += vsc8512_write(phy_handle, 16, 0x8fd6);
    ret += vsc8512_write(phy_handle, 18, 0x8);
    ret += vsc8512_write(phy_handle, 17, 0xef3f);
    ret += vsc8512_write(phy_handle, 16, 0x83ae);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x1d00);
    ret += vsc8512_write(phy_handle, 16, 0x8fa0);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x28);
    ret += vsc8512_write(phy_handle, 16, 0x8f80);
    ret += vsc8512_write(phy_handle, 18, 0x3a);
    ret += vsc8512_write(phy_handle, 17, 0xe90a);
    ret += vsc8512_write(phy_handle, 16, 0x8f82);
    ret += vsc8512_write(phy_handle, 18, 0x35);
    ret += vsc8512_write(phy_handle, 17, 0x545f);
    ret += vsc8512_write(phy_handle, 16, 0x968c);
    ret += vsc8512_write(phy_handle, 18, 0xff);
    ret += vsc8512_write(phy_handle, 17, 0xd00);
    ret += vsc8512_write(phy_handle, 16, 0x96b0);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x7100);
    ret += vsc8512_write(phy_handle, 16, 0x96b2);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x8fff);
    ret += vsc8512_write(phy_handle, 16, 0x8fd2);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x500);
    ret += vsc8512_write(phy_handle, 16, 0x8fc0);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x112);
    ret += vsc8512_write(phy_handle, 16, 0x96a0);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x7000);
    ret += vsc8512_write(phy_handle, 16, 0x96a2);
    ret += vsc8512_write(phy_handle, 18, 0x20);
    ret += vsc8512_write(phy_handle, 17, 0x2f3f);
    ret += vsc8512_write(phy_handle, 16, 0x96a4);
    ret += vsc8512_write(phy_handle, 18, 0x0);
    ret += vsc8512_write(phy_handle, 17, 0x83df);
    ret += vsc8512_write(phy_handle, 16, 0x8fd4);
    ret += vsc8512_write(phy_handle, 18, 0x2b);
    ret += vsc8512_write(phy_handle, 17, 0x162e);

    ret += vsc8512_phy_page_test(phy_handle); // Switch to test-register page
    ret += vsc8512_write(phy_handle,  8, 0x0012); // Undo enable token-ring during coma-mode

    /* Turn off broadcast writes */
    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write(phy_handle, 22, 0x3200);

    /* Download patch into PRAM for the internal 8051 */
    ret += vsc8512_8051_patch_13_10_2010(phy_handle);


    /* Set patch trap address and patch address for the 2 necessary patches for MCB and a 3rd for VeriPHY */
    ret += vsc8512_phy_page_gpio(phy_handle);
    ret += vsc8512_write(phy_handle, 3, 0x2caa);      // ROM addr. to trap for patch0
    ret += vsc8512_write(phy_handle, 4, 0x400c);      // PRAM jump addr. for patch0
    ret += vsc8512_write(phy_handle, 5, 0x29ed);      // ROM addr. to trap for patch1
    ret += vsc8512_write(phy_handle, 6, 0x4063);      // PRAM jump addr. for patch1
    ret += vsc8512_write(phy_handle, 7, 0x06a2);      // ROM addr. to trap for patch2
    ret += vsc8512_write(phy_handle, 8, 0x40dd);      // PRAM jump addr. for patch2

    /* Assert/release AuxADC reset to make ready for patch to use */
    ret += vsc8512_write(phy_handle, 26, 0x0000);
    ret += vsc8512_write(phy_handle, 26, 0x0080);

    /* Enable the three traps being used and release the PHY 8051 */
    ret += vsc8512_write(phy_handle, 12, 0x0700);     // set bit 10:8 to enable patch0-2 trapping
    ret += vsc8512_write(phy_handle, 0, 0x4098);      // Enable 8051 clock; indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    ret += vsc8512_write(phy_handle, 0, 0xc098);      // Release 8051 SW Reset
    
    return ret;
}

static int32
vsc8512_65nm_10BASE_init(phy_handle_t* phy_handle, const uint8 invert_pulse)
{
    int32 i;
    int32 ret = 0;
    uint16 msb, lsw, tr_cmd;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    /* Configure 10BASE-Te (EEE reduced transmit amplitude spec for 10Mb) */
    ret += vsc8512_phy_page_ext2(phy_handle);
#ifdef VTSS_10BASE_TE // TODO:This macro will be defined when 10BASE-TE mode is supported
    ret += vsc8512_write_mask(phy_handle, 16, 0x0007, 0x000f);
    ret += vsc8512_write_mask(phy_handle, 17, 0x8000, 0x8000);
#else /* System uses regular 10BASE-T to support Cat3 (and uglier) cables */
    ret += vsc8512_write_mask(phy_handle, 17, 0x0000, 0x8000);
#endif /* VTSS_10BASE_TE */

    ret += vsc8512_phy_page_tr(phy_handle);
    
    for ( tr_cmd = tr_base_reg_write_cmd, i = 0; tr_cmd <= tr_last_reg_write_cmd;
          tr_cmd += 2, ++i ) 
    {
        msb = tr_reg_bits_msb[i];
        lsw = tr_reg_bits_lsw[i];
    	if (invert_pulse) 
        {
    	    if (tr_cmd == 0x848a)
    	    {
                lsw ^= 0x1040;
    	    }
    	    else 
            {
                msb ^= 0x82;
                lsw ^= 0x0820;
    	    }
    	}
        ret += vsc8512_write(phy_handle, 18, msb);
        ret += vsc8512_write(phy_handle, 17, lsw);
        ret += vsc8512_write(phy_handle, 16, tr_cmd);
    }
    
    return ret;
}


/*
 * Function for checking that the 8051 code is loaded correctly.
 */
static int32
vsc8512_is_8051_crc_ok_private (phy_handle_t* phy_handle, uint16 start_addr, uint16 code_length, uint16 expected_crc) 
{
    uint16 crc_calculated = 0;
    int32 ret = 0;
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    ret += vsc8512_phy_page_ext1(phy_handle);
    ret += vsc8512_write(phy_handle, 25, start_addr);
    ret += vsc8512_write(phy_handle, 26, code_length); 

    ret += vsc8512_phy_page_gpio(phy_handle); /* Change to GPIO page */
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8008); /* Start Mirco command */
    
    /* MISSING VTSS_RC() is not an omission, it is intended so we get a CRC error */
    (void)vsc8512_wait_for_micro_complete(phy_handle);
    
    /* Get the CRC */
    ret += vsc8512_phy_page_ext1(phy_handle);
    ret += vsc8512_read(phy_handle, 25, &crc_calculated);
    
    ret += vsc8512_phy_page_std(phy_handle); /* return to standard page */
    
    if (crc_calculated == expected_crc) 
    {
        DRV_LOG_DEBUG(DRV_PHY, "8051 crc_calculated = 0x%X, expected_crc = 0x%X", 
            crc_calculated, expected_crc); 
        return ret;
    } 
    else 
    {
        DRV_LOG_DEBUG(DRV_PHY, "dsd8051 crc_calculated = 0x%X, expected_crc = 0x%X", 
            crc_calculated, expected_crc); 
        return RESULT_ERROR;
    }
}

/*
 * Atom  Rev. B. Internal 8051 patch.
 * Return : VTSS_RC_OK if configuration done else error code. 
 * Id: fastpatch.m,v 1.5 2012/01/05 13:41:53 jimb Exp 
 *
 * Update Date : 24-04-2012
 */

/* Initializes micro patch for entire 12-port chip containing specified PHY */
static int32
vsc8512_rev_b_patch(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    static uint8 patch_arr[] = 
    {
        0x43, 0x40, 0x02, 0x40, 0x55, 0x02, 0x40, 0x4e, 0x02, 0x43,
        0x55, 0x02, 0x42, 0x20, 0x02, 0x41, 0x77, 0x74, 0x05, 0xfb,
        0x02, 0x08, 0x21, 0x94, 0x14, 0xec, 0x64, 0x80, 0x94, 0x80,
        0x50, 0x12, 0xe5, 0x51, 0x24, 0x18, 0x12, 0x16, 0xea, 0x12,
        0x17, 0x54, 0xe4, 0xf0, 0xa3, 0x74, 0x24, 0x02, 0x0a, 0xf6,
        0xe9, 0x12, 0x17, 0xbd, 0xe5, 0x51, 0x24, 0x18, 0x12, 0x16,
        0xea, 0x12, 0x17, 0x54, 0xe4, 0x75, 0xf0, 0x10, 0x02, 0x0b,
        0x0b, 0x12, 0x42, 0xc4, 0x02, 0x09, 0xde, 0xd0, 0x1d, 0xd0,
        0x1d, 0x02, 0x39, 0xb1, 0xe4, 0xf5, 0x27, 0xf5, 0x28, 0xe5,
        0x1e, 0x54, 0x0f, 0x64, 0x09, 0x60, 0x03, 0x02, 0x41, 0x6c,
        0xfb, 0xfa, 0x7d, 0xff, 0x7f, 0x92, 0x12, 0x35, 0xdb, 0x53,
        0x91, 0x7f, 0xa2, 0xaf, 0x92, 0x00, 0xc2, 0xaf, 0x75, 0xff,
        0x10, 0x75, 0xfd, 0x84, 0x75, 0xfe, 0xb7, 0x75, 0xfc, 0x94,
        0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x12, 0x43, 0x54, 0x75, 0xff,
        0x11, 0xe4, 0xff, 0xf5, 0xfb, 0xe5, 0xfb, 0xd3, 0x94, 0x0b,
        0x50, 0x25, 0x75, 0xfc, 0xd0, 0xe5, 0xfc, 0x30, 0xe0, 0xfb,
        0xe5, 0xfd, 0x54, 0x3c, 0xfe, 0xbe, 0x2c, 0x11, 0xae, 0x07,
        0x0f, 0x74, 0x21, 0x2e, 0xf8, 0xa6, 0xfb, 0xc3, 0xef, 0x64,
        0x80, 0x94, 0x86, 0x50, 0x04, 0x05, 0xfb, 0x80, 0xd4, 0xd3,
        0xef, 0x64, 0x80, 0x94, 0x80, 0x50, 0x03, 0x02, 0x41, 0x60,
        0xe4, 0xf5, 0xfb, 0x75, 0xff, 0x10, 0x75, 0xfd, 0x84, 0x75,
        0xfe, 0xb7, 0x75, 0xfc, 0x94, 0xe5, 0xfc, 0x30, 0xe0, 0xfb,
        0x12, 0x43, 0x54, 0xe4, 0xfe, 0x74, 0x21, 0x2e, 0xf8, 0xe6,
        0xf5, 0xfb, 0x75, 0xff, 0x11, 0x75, 0xfc, 0xd0, 0xe5, 0xfc,
        0x30, 0xe0, 0xfb, 0xe5, 0xfd, 0x54, 0x3c, 0x64, 0x2c, 0x70,
        0x4d, 0x75, 0xfe, 0x10, 0xf5, 0xfd, 0x75, 0xfc, 0x90, 0xe5,
        0xfc, 0x30, 0xe0, 0xfb, 0x75, 0xff, 0x10, 0x75, 0xfe, 0x8f,
        0x75, 0xfd, 0x88, 0x75, 0xfc, 0x90, 0xe5, 0xfc, 0x30, 0xe0,
        0xfb, 0x12, 0x43, 0x54, 0x75, 0xff, 0x11, 0xe4, 0xf5, 0xfe,
        0xf5, 0xfd, 0x75, 0xfc, 0x90, 0xe5, 0xfc, 0x30, 0xe0, 0xfb,
        0x75, 0xff, 0x10, 0x75, 0xfe, 0x8f, 0x75, 0xfd, 0x88, 0x75,
        0xfc, 0x90, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x12, 0x43, 0x54,
        0x05, 0x28, 0xe5, 0x28, 0x70, 0x02, 0x05, 0x27, 0x0e, 0xc3,
        0xef, 0x64, 0x80, 0xf8, 0xee, 0x64, 0x80, 0x98, 0x40, 0x8d,
        0xab, 0x28, 0xaa, 0x27, 0x7d, 0x0a, 0x7f, 0x04, 0x12, 0x3c,
        0x26, 0xe5, 0x91, 0x20, 0xe7, 0x03, 0x02, 0x40, 0x77, 0xa2,
        0x00, 0x92, 0xaf, 0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee, 0x7f,
        0x92, 0x02, 0x35, 0xdb, 0xe5, 0x1e, 0x20, 0xe5, 0x03, 0x02,
        0x42, 0x1f, 0x90, 0x47, 0xd5, 0x30, 0xe4, 0x33, 0xe0, 0x54,
        0xc7, 0xf0, 0x44, 0x10, 0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x54,
        0x1f, 0xf0, 0xf0, 0xa3, 0xe0, 0x54, 0xf8, 0xf0, 0xf0, 0x90,
        0x47, 0xd5, 0xe0, 0x54, 0x3f, 0xf0, 0x44, 0xc0, 0xf0, 0xa3,
        0xe0, 0x54, 0xfc, 0xf0, 0x44, 0x01, 0xf0, 0x90, 0x47, 0xdc,
        0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x28, 0xf0, 0x80, 0x2e, 0xe0,
        0x54, 0xc7, 0xf0, 0x44, 0x08, 0xf0, 0x90, 0x47, 0xda, 0xe0,
        0x44, 0x04, 0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x54, 0x1f, 0xf0,
        0x44, 0x40, 0xf0, 0xa3, 0xe0, 0x54, 0xf8, 0xf0, 0xf0, 0x90,
        0x47, 0xd4, 0xe0, 0x54, 0x1f, 0xf0, 0xf0, 0xa3, 0xe0, 0x54,
        0xf8, 0xf0, 0x44, 0x03, 0xf0, 0x90, 0x47, 0xd6, 0xe0, 0x44,
        0x40, 0xf0, 0x90, 0x47, 0xe0, 0xe0, 0x54, 0xf8, 0xf0, 0x44,
        0x05, 0xf0, 0x54, 0x1f, 0xf0, 0x44, 0x40, 0xf0, 0xa3, 0xe0,
        0x54, 0xf8, 0xf0, 0x44, 0x06, 0xf0, 0x90, 0x47, 0xde, 0xe0,
        0x54, 0x1f, 0xf0, 0xf0, 0x54, 0xe3, 0xf0, 0xf0, 0x90, 0x47,
        0xdd, 0xe0, 0x54, 0x7f, 0xf0, 0xf0, 0xa3, 0xe0, 0x54, 0xfc,
        0xf0, 0xf0, 0x22, 0x90, 0x47, 0xd0, 0xe0, 0x54, 0xef, 0xf0,
        0x44, 0x80, 0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x54, 0x7f, 0xf0,
        0x44, 0x80, 0xf0, 0xa3, 0xe0, 0x54, 0xfc, 0xf0, 0xf0, 0x90,
        0x47, 0xd7, 0xe0, 0x54, 0x7f, 0xf0, 0x90, 0x47, 0xda, 0xe0,
        0x54, 0xc7, 0xf0, 0x44, 0x10, 0xf0, 0x90, 0x47, 0xd6, 0xe0,
        0x54, 0xe7, 0xf0, 0x44, 0x18, 0xf0, 0x90, 0x47, 0xd5, 0xe0,
        0x54, 0x8f, 0xf0, 0x44, 0x20, 0xf0, 0x90, 0x47, 0xd4, 0xe0,
        0x54, 0x3f, 0xf0, 0xf0, 0xa3, 0xe0, 0x54, 0xfc, 0xf0, 0x44,
        0x01, 0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x44, 0x04, 0xf0, 0x90,
        0x47, 0xd7, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x18, 0xf0, 0x7c,
        0x00, 0xe5, 0x1e, 0x54, 0x0f, 0xfd, 0x64, 0x01, 0x70, 0x19,
        0xe5, 0x1e, 0x30, 0xe4, 0x14, 0xa3, 0xe0, 0x44, 0x02, 0xf0,
        0x90, 0x47, 0xda, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x18, 0xf0,
        0x54, 0xf8, 0xf0, 0xf0, 0x22, 0xed, 0x64, 0x01, 0x4c, 0x70,
        0x12, 0x90, 0x47, 0xd5, 0xe0, 0x54, 0x7f, 0xf0, 0x44, 0x80,
        0xf0, 0xa3, 0xe0, 0x54, 0xf8, 0xf0, 0x44, 0x07, 0xf0, 0x90,
        0x47, 0xd8, 0xe0, 0x54, 0xfd, 0xf0, 0x22, 0xe4, 0xf5, 0x4b,
        0x75, 0x4c, 0x01, 0xf5, 0x4d, 0x7f, 0x40, 0x7e, 0x02, 0x12,
        0x3b, 0x4e, 0xe4, 0xf5, 0x29, 0xe5, 0x29, 0x25, 0xe0, 0x24,
        0x20, 0xf5, 0x82, 0xe4, 0x34, 0x43, 0xf5, 0x83, 0xe4, 0x93,
        0xfe, 0x74, 0x01, 0x93, 0xf5, 0x4b, 0xee, 0xff, 0x8f, 0x4c,
        0xe5, 0x4c, 0x30, 0xe7, 0x04, 0x7f, 0xff, 0x80, 0x02, 0x7f,
        0x00, 0x8f, 0x4d, 0xe5, 0x29, 0x25, 0xe0, 0xff, 0xe4, 0x33,
        0x44, 0x02, 0xfe, 0x12, 0x3b, 0x4e, 0x05, 0x29, 0xe5, 0x29,
        0xc3, 0x94, 0x10, 0x40, 0xc4, 0xe4, 0xf5, 0x4b, 0xf5, 0x4c,
        0xf5, 0x4d, 0x7f, 0x40, 0x7e, 0x02, 0x02, 0x3b, 0x4e, 0x17,
        0x55, 0xfc, 0xaf, 0xeb, 0x8c, 0xd9, 0xec, 0x7f, 0xff, 0x45,
        0x31, 0xbb, 0x3e, 0x15, 0xee, 0xf8, 0x8a, 0xd9, 0x60, 0xf4,
        0x6b, 0xf5, 0xde, 0x01, 0xa4, 0xfb, 0xf4, 0xeb, 0xf4, 0xfc,
        0x03, 0xc2, 0xe9, 0xc2, 0x61, 0x75, 0xe9, 0xff, 0x75, 0xed,
        0x0f, 0x90, 0x47, 0xfe, 0xe0, 0x54, 0xf8, 0x44, 0x02, 0xf0,
        0x22, 0x22, 0x22
    };

    ret += vsc8512_download_8051_code(phy_handle, &patch_arr[0], sizeof(patch_arr));
    ret += vsc8512_write(phy_handle,  0, 0x4018);     // Enable 8051 clock; clear patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    ret += vsc8512_write(phy_handle,  0, 0xc018);     // Release 8051 SW Reset

    // Check that code is downloaded correctly.
    ret += vsc8512_is_8051_crc_ok_private(phy_handle, 
                                          FIRMWARE_START_ADDR,
                                          sizeof(patch_arr) + 1 /* Add one for the byte auto-added in the download function */, 
                                          0x1901);
    
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_micro_assert_reset(phy_handle);
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle,  3, 0x081c);     // ROM addr. to trap for patch0
    ret += vsc8512_write(phy_handle,  4, 0x4012);     // PRAM jump addr. for patch0
    ret += vsc8512_write(phy_handle,  5, 0x0ade);     // ROM addr. to trap for patch1

    ret += vsc8512_write(phy_handle,  6, 0x4018);     // PRAM jump addr. for patch1
    ret += vsc8512_write(phy_handle,  7, 0x06df);     // ROM addr. to trap for patch2
    ret += vsc8512_write(phy_handle,  8, 0x06fd);     // PRAM jump addr. to patch around invalid x-short preservation
    ret += vsc8512_write(phy_handle,  9, 0x09db);     // ROM addr. to trap for patch3
    ret += vsc8512_write(phy_handle, 10, 0x4048);     // PRAM jump addr. for patch3
    ret += vsc8512_write(phy_handle, 12, 0x0f00);     // set bit 11:8 to enable patch0-3 trapping
    ret += vsc8512_write(phy_handle,  0, 0x4098);     // Enable 8051 clock; indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    ret += vsc8512_write(phy_handle,  0, 0xc098);     // Release 8051 SW Reset
    ret += vsc8512_phy_page_std(phy_handle);     // Switch back to main register-page

    return ret;
}

/*
 * Initialization need for ATOM12 rev. B. 
 * (For whole chip and must be done before init. of the individual ports)
 */

/* Initializes entire 12-port chip containing specified PHY */
static int32
vsc8512_rev_b_init_script(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    ret += vsc8512_phy_page_std(phy_handle);
    /* for centec use, do not enable SMI broadcast write */
    ret += vsc8512_write_mask(phy_handle, 22, 0x0001, 0x0001); //Turn ON broadcast to all 12 PHYs
    ret += vsc8512_write(phy_handle, 0, 0x1000);
    ret += vsc8512_write(phy_handle, 24, 0x2040);
    ret += vsc8512_phy_page_ext2(phy_handle);
    ret += vsc8512_write(phy_handle, 16, 0x02f0);
    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write(phy_handle, 20, 0x6530);
    ret += vsc8512_write(phy_handle,  9, 0x1800);
    ret += vsc8512_write(phy_handle,  8, 0x8212);
    ret += vsc8512_phy_page_tr(phy_handle);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0001);
    ret += vsc8512_write(phy_handle, 16, 0x8fe6);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x1b00);
    ret += vsc8512_write(phy_handle, 16, 0x8fa0);
    ret += vsc8512_write(phy_handle, 18, 0x0001);
    ret += vsc8512_write(phy_handle, 17, 0xe489);
    ret += vsc8512_write(phy_handle, 16, 0x8f92);
    ret += vsc8512_write(phy_handle, 18, 0x000f);
    ret += vsc8512_write(phy_handle, 17, 0x0151);
    ret += vsc8512_write(phy_handle, 16, 0x8fe0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0011);
    ret += vsc8512_write(phy_handle, 16, 0x96a0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x7100);
    ret += vsc8512_write(phy_handle, 16, 0x96a2);
    ret += vsc8512_write(phy_handle, 18, 0x00d2);
    ret += vsc8512_write(phy_handle, 17, 0x547f);
    ret += vsc8512_write(phy_handle, 16, 0x968c);
    ret += vsc8512_write(phy_handle, 18, 0x00f0);
    ret += vsc8512_write(phy_handle, 17, 0xf00d);
    ret += vsc8512_write(phy_handle, 16, 0x96b0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x7620);
    ret += vsc8512_write(phy_handle, 16, 0x96b2);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x345f);
    ret += vsc8512_write(phy_handle, 16, 0x96b4);
    ret += vsc8512_write(phy_handle, 18, 0x0012);
    ret += vsc8512_write(phy_handle, 17, 0x481a);
    ret += vsc8512_write(phy_handle, 16, 0x8f82);
    ret += vsc8512_write(phy_handle, 18, 0x000e);
    ret += vsc8512_write(phy_handle, 17, 0x2d00);
    ret += vsc8512_write(phy_handle, 16, 0x8fb0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x83a6);
    ret += vsc8512_write(phy_handle, 18, 0x0007);
    ret += vsc8512_write(phy_handle, 17, 0xd6bb);
    ret += vsc8512_write(phy_handle, 16, 0x83ae);
    ret += vsc8512_write(phy_handle, 18, 0x0004);
    ret += vsc8512_write(phy_handle, 17, 0xbf81);
    ret += vsc8512_write(phy_handle, 16, 0x9688);
    ret += vsc8512_write(phy_handle, 18, 0x0013);
    ret += vsc8512_write(phy_handle, 17, 0x292a);
    ret += vsc8512_write(phy_handle, 16, 0x8fa4);
    ret += vsc8512_write(phy_handle, 18, 0x003c);
    ret += vsc8512_write(phy_handle, 17, 0x3800);
    ret += vsc8512_write(phy_handle, 16, 0x8f8a);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0422);
    ret += vsc8512_write(phy_handle, 16, 0x8f86);

    /* update to vtss api 4.22 */
    
    // Fix for bugzilla#9870 (Atheros EEE interop) and bugzilla#9885 (1000BT too long to link up)
    ret += vsc8512_write(phy_handle, 18, 0x0004);
    ret += vsc8512_write(phy_handle, 17, 0x01bd);
    ret += vsc8512_write(phy_handle, 16, 0x8fae);
    ret += vsc8512_write(phy_handle, 18, 0x000f);
    ret += vsc8512_write(phy_handle, 17, 0x000f);
    ret += vsc8512_write(phy_handle, 16, 0x8fac);

    // Improve 100BASE-TX link startup robustness to address interop issue
    ret += vsc8512_write(phy_handle, 18, 0x0068);
    ret += vsc8512_write(phy_handle, 17, 0x8980);
    ret += vsc8512_write(phy_handle, 16, 0x8f90);
    
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0xf7df);
    ret += vsc8512_write(phy_handle, 16, 0x8fd4);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0xffdf);
    ret += vsc8512_write(phy_handle, 16, 0x8fd2);
    ret += vsc8512_write(phy_handle, 18, 0x0085);
    ret += vsc8512_write(phy_handle, 17, 0xf3c6);
    ret += vsc8512_write(phy_handle, 16, 0x97a0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x07c2);
    ret += vsc8512_write(phy_handle, 16, 0x97a2);
    ret += vsc8512_write(phy_handle, 18, 0x002b);
    ret += vsc8512_write(phy_handle, 17, 0x2dff);
    ret += vsc8512_write(phy_handle, 16, 0x9794);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x00ba);
    ret += vsc8512_write(phy_handle, 16, 0x8fe2);

    // Bugzilla#8183 - Ports with different speeds can get link up.
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0004);
    ret += vsc8512_write(phy_handle, 16, 0x87fe);


    ret += vsc8512_65nm_10BASE_init(phy_handle, 0);

    // Work-around solution for bugzilla#6859 from James McIntosh
    //Our solution is to force the BlindSampleTimer to 0x080 by setting
    //BlindSampleTimerForce to '1' (bit 22) and BlindSampleTimerForceVal to 0x080
    //(bits 21:13) in register 0x3d in the AutoNegDbgNode.  This increases the time
    //to wait for the link to come up before allowing the crossover state to switch
    //to ~511 ms.
    ret += vsc8512_write(phy_handle, 18, 0x0050);
    ret += vsc8512_write(phy_handle, 17, 0x100f);
    ret += vsc8512_write(phy_handle, 16, 0x87fa);
    
    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write(phy_handle,  8, 0x0212);
    ret += vsc8512_rev_b_patch(phy_handle); // Download Luton26/Atom12 Rev. B patch

    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 22, 0x0000, 0x0001); //Turn off broadcast to all 12 PHYs

    return ret;
}

/*
 *  Atom  Rev. C. Internal 8051 patch.
 *  In     : port_no - any port within the chip where to load the 8051 code.
 *  Return : VTSS_RC_OK if configuration done else error code. 
 * Id: fastpatch.m,v 1.3 2012/01/05 00:54:05 jimb Exp 
 * Revision 1.3  2012/01/05 00:54:05  jimb
 * Regenerated patch with jump-table (even though unused) for compatibility
 * with all other patches that begin with a jump-table.
 * 
 * Revision 1.2  2012/01/05 00:25:35  jimb
 * Add patch to greatly improve VeriPHY performance for 80-140m cables.  Patch
 * also clears up a bug where the current PHY was changed during the algorithm
 * based on the subchannel being searched (0-3) instead of the specified PHY!
 * 
 * Initializes micro patch for entire 12-port chip containing specified PHY
 */
static int32
vsc8512_rev_c_patch(phy_handle_t* phy_handle)
{
    int32 ret = 0;

    const uint8 patch_arr[] = {
        0x42, 0x5e, 0x02, 0x42, 0x53, 0x02, 0x42, 0x68, 0x02, 0x42,
        0x6b, 0x02, 0x40, 0xd1, 0x02, 0x41, 0x62, 0xef, 0x03, 0x03,
        0xf5, 0x62, 0x02, 0x1d, 0xf2, 0x12, 0x41, 0xcb, 0x02, 0x09,
        0xbb, 0xed, 0xff, 0xe5, 0xfc, 0x54, 0x38, 0x64, 0x20, 0x70,
        0x08, 0x65, 0xff, 0x70, 0x04, 0xed, 0x44, 0x80, 0xff, 0x22,
        0xe4, 0xf5, 0xfb, 0x7d, 0x1c, 0xe4, 0xff, 0x12, 0x3c, 0x2a,
        0xad, 0x07, 0xac, 0x06, 0xec, 0x54, 0xc0, 0xff, 0xed, 0x54,
        0x3f, 0x4f, 0xf5, 0x20, 0x30, 0x06, 0x2c, 0x30, 0x01, 0x08,
        0xa2, 0x04, 0x72, 0x03, 0x92, 0x07, 0x80, 0x21, 0x30, 0x04,
        0x06, 0x7b, 0xcc, 0x7d, 0x11, 0x80, 0x0d, 0x30, 0x03, 0x06,
        0x7b, 0xcc, 0x7d, 0x10, 0x80, 0x04, 0x7b, 0x66, 0x7d, 0x16,
        0xe4, 0xff, 0x12, 0x3b, 0x49, 0xee, 0x4f, 0x24, 0xff, 0x92,
        0x07, 0xaf, 0xfb, 0x74, 0xb6, 0x2f, 0xf8, 0xe6, 0xff, 0xa6,
        0x20, 0x20, 0x07, 0x39, 0x8f, 0x20, 0x30, 0x07, 0x34, 0x30,
        0x00, 0x31, 0x20, 0x04, 0x2e, 0x20, 0x03, 0x2b, 0xe4, 0xf5,
        0xff, 0x75, 0xfc, 0xc2, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0xaf,
        0xfe, 0xef, 0x20, 0xe3, 0x1a, 0xae, 0xfd, 0x44, 0x08, 0xf5,
        0xfe, 0x75, 0xfc, 0x80, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x8f,
        0xfe, 0x8e, 0xfd, 0x75, 0xfc, 0x80, 0xe5, 0xfc, 0x30, 0xe0,
        0xfb, 0x05, 0xfb, 0xaf, 0xfb, 0xef, 0xc3, 0x94, 0x0c, 0x50,
        0x03, 0x02, 0x40, 0x36, 0xe4, 0xf5, 0xfb, 0x22, 0x90, 0x47,
        0xd0, 0xe0, 0x54, 0xef, 0xf0, 0x44, 0x80, 0xf0, 0x90, 0x47,
        0xd8, 0xe0, 0x54, 0x7f, 0xf0, 0x44, 0x80, 0xf0, 0xa3, 0xe0,
        0x54, 0xfc, 0xf0, 0xf0, 0x90, 0x47, 0xd7, 0xe0, 0x54, 0x7f,
        0xf0, 0x90, 0x47, 0xda, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x10,
        0xf0, 0x90, 0x47, 0xd6, 0xe0, 0x54, 0xe7, 0xf0, 0x44, 0x18,
        0xf0, 0x90, 0x47, 0xd5, 0xe0, 0x54, 0x8f, 0xf0, 0x44, 0x20,
        0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x44, 0x04, 0xf0, 0x90, 0x47,
        0xd7, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x18, 0xf0, 0x7c, 0x00,
        0xe5, 0x6f, 0x54, 0x0f, 0xfd, 0x64, 0x01, 0x70, 0x19, 0xe5,
        0x6f, 0x30, 0xe4, 0x14, 0xa3, 0xe0, 0x44, 0x02, 0xf0, 0x90,
        0x47, 0xda, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x18, 0xf0, 0x54,
        0xf8, 0xf0, 0xf0, 0x22, 0xed, 0x64, 0x01, 0x4c, 0x70, 0x0f,
        0x90, 0x47, 0xd5, 0xe0, 0x54, 0x7f, 0xf0, 0x44, 0x80, 0x12,
        0x42, 0x4c, 0x44, 0x07, 0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x54,
        0xfd, 0xf0, 0x22, 0xe5, 0x6f, 0x30, 0xe5, 0x63, 0x90, 0x47,
        0xd5, 0x30, 0xe4, 0x30, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x10,
        0xf0, 0x90, 0x47, 0xd8, 0xe0, 0x54, 0x1f, 0xf0, 0x12, 0x42,
        0x4c, 0xf0, 0x90, 0x47, 0xd5, 0xe0, 0x54, 0x3f, 0xf0, 0x44,
        0xc0, 0xf0, 0xa3, 0xe0, 0x54, 0xfc, 0xf0, 0x44, 0x01, 0xf0,
        0x90, 0x47, 0xdc, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x28, 0xf0,
        0x80, 0x1d, 0xe0, 0x54, 0xc7, 0xf0, 0x44, 0x08, 0xf0, 0x90,
        0x47, 0xda, 0xe0, 0x44, 0x04, 0xf0, 0x90, 0x47, 0xd8, 0xe0,
        0x12, 0x42, 0x47, 0xf0, 0x90, 0x47, 0xd6, 0xe0, 0x44, 0x40,
        0xf0, 0x90, 0x47, 0xe0, 0xe0, 0x54, 0xf8, 0xf0, 0x44, 0x05,
        0xf0, 0x12, 0x42, 0x47, 0x44, 0x06, 0xf0, 0x22, 0xe4, 0xf5,
        0x4b, 0x75, 0x4c, 0x01, 0xf5, 0x4d, 0x7f, 0x40, 0x7e, 0x02,
        0x12, 0x3b, 0x72, 0xe4, 0xf5, 0x21, 0xe5, 0x21, 0x25, 0xe0,
        0x24, 0x27, 0xf5, 0x82, 0xe4, 0x34, 0x42, 0xf5, 0x83, 0xe4,
        0x93, 0xfe, 0x74, 0x01, 0x93, 0xf5, 0x4b, 0xee, 0xff, 0x8f,
        0x4c, 0xe5, 0x4c, 0x30, 0xe7, 0x04, 0x7f, 0xff, 0x80, 0x02,
        0x7f, 0x00, 0x8f, 0x4d, 0xe5, 0x21, 0x25, 0xe0, 0xff, 0xe4,
        0x33, 0x44, 0x02, 0xfe, 0x12, 0x3b, 0x72, 0x05, 0x21, 0xe5,
        0x21, 0xc3, 0x94, 0x10, 0x40, 0xc4, 0xe4, 0xf5, 0x4b, 0xf5,
        0x4c, 0xf5, 0x4d, 0x7f, 0x40, 0x7e, 0x02, 0x02, 0x3b, 0x72,
        0x17, 0x55, 0xfc, 0xaf, 0xeb, 0x8c, 0xd9, 0xec, 0x7f, 0xff,
        0x45, 0x31, 0xbb, 0x3e, 0x15, 0xee, 0xf8, 0x8a, 0xd9, 0x60,
        0xf4, 0x6b, 0xf5, 0xde, 0x01, 0xa4, 0xfb, 0xf4, 0xeb, 0xf4,
        0xfc, 0x03, 0x54, 0x1f, 0xf0, 0x44, 0x40, 0xf0, 0xa3, 0xe0,
        0x54, 0xf8, 0xf0, 0x22, 0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee,
        0x7f, 0x92, 0x02, 0x36, 0x7f, 0x90, 0x47, 0xfe, 0xe0, 0x54,
        0xf8, 0x44, 0x02, 0xf0, 0x22, 0x02, 0x40, 0x33, 0x22
    };
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    ret = vsc8512_download_8051_code(phy_handle, &patch_arr[0], sizeof(patch_arr));
    ret += vsc8512_phy_page_gpio(phy_handle);
    ret += vsc8512_write(phy_handle,  0, 0x4018);     // Enable 8051 clock; clear patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    ret += vsc8512_write(phy_handle,  0, 0xc018);     // Release 8051 SW Reset

    // Check that code is downloaded correctly.
    ret += vsc8512_is_8051_crc_ok_private(phy_handle,
                                            FIRMWARE_START_ADDR,
                                            sizeof(patch_arr) + 1, // Add one for the byte auto-added in the download function
                                            0x1E5E);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_micro_assert_reset(phy_handle);
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle,  3, 0x1def);     // ROM addr. to trap for patch0
    ret += vsc8512_write(phy_handle,  4, 0x4012);     // PRAM jump addr. for patch0
    ret += vsc8512_write(phy_handle,  5, 0x09b8);     // ROM addr. to trap for patch1
    ret += vsc8512_write(phy_handle,  6, 0x401a);     // PRAM jump addr. for patch1
    ret += vsc8512_write(phy_handle,  7, 0x3c47);     // Trap ROM at _MicroSmiRead+0x1d to spoof patch-presence
    ret += vsc8512_write(phy_handle,  8, 0x4020);     // Branch to starting address of SpoofPatchPresence
    ret += vsc8512_write(phy_handle, 12, 0x0700);     // set bits 8-10 to enable patch0-2 trapping
    ret += vsc8512_write(phy_handle,  0, 0x4018);     // Enable 8051 clock; clear patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    ret += vsc8512_write(phy_handle,  0, 0xc018);     // Release 8051 SW Reset
    ret += vsc8512_phy_page_std(phy_handle);     // Switch back to main register-page

    return ret;
}

// Initialization need for ATOM12 rev. C & D. (For whole chip and must be done before init. of the individual ports)
//
// In : phy - Any port with the chip. 
//
// Return : VTSS_RC_OK if configuration done else error code.
// Date   : 30-10-2012
static int32
vsc8512_rev_cd_init_script(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    phy_info_t* pphy_info;
    
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    pphy_info = &phy_handle->phy_info;
    
    ret = vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 22, 0x0001, 0x0001);
    ret += vsc8512_write(phy_handle, 24, 0x0040);
    ret += vsc8512_phy_page_ext2(phy_handle);
    ret += vsc8512_write(phy_handle, 16, 0x02be);
    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write(phy_handle, 20, 0x4420);
    ret += vsc8512_write(phy_handle, 24, 0x0c00);
    ret += vsc8512_write(phy_handle,  9, 0x18c8);
    ret += vsc8512_write_mask(phy_handle,  8, 0x8000, 0x8000);
    ret += vsc8512_write(phy_handle,  5, 0x1320);

    ret += vsc8512_phy_page_tr(phy_handle);
    if (GLB_VTSS_PHY_ATOM_REV_D == pphy_info->phy_revision) 
    {
        ret += vsc8512_write(phy_handle, 18, 0x0027);
        ret += vsc8512_write(phy_handle, 17, 0x303d);
        ret += vsc8512_write(phy_handle, 16, 0x9792);
    }
    else  /* GLB_VTSS_PHY_ATOM_REV_C */
    {
        ret += vsc8512_write(phy_handle, 18, 0x0032);
        ret += vsc8512_write(phy_handle, 17, 0x303d);
        ret += vsc8512_write(phy_handle, 16, 0x9792);
    }
    ret += vsc8512_write(phy_handle, 18, 0x00a0);
    ret += vsc8512_write(phy_handle, 17, 0xf147);
    ret += vsc8512_write(phy_handle, 16, 0x97a0);
    ret += vsc8512_write(phy_handle, 18, 0x0005);
    ret += vsc8512_write(phy_handle, 17, 0x2f54);
    ret += vsc8512_write(phy_handle, 16, 0x8fe4);
    ret += vsc8512_write(phy_handle, 18, 0x0004);
    ret += vsc8512_write(phy_handle, 17, 0x01bd);
    ret += vsc8512_write(phy_handle, 16, 0x8fae);
    ret += vsc8512_write(phy_handle, 18, 0x000f);
    ret += vsc8512_write(phy_handle, 17, 0x000f);
    ret += vsc8512_write(phy_handle, 16, 0x8fac);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0004);
    ret += vsc8512_write(phy_handle, 16, 0x87fe);
    ret += vsc8512_write(phy_handle, 18, 0x0006);
    ret += vsc8512_write(phy_handle, 17, 0x0150);
    ret += vsc8512_write(phy_handle, 16, 0x8fe0);
    ret += vsc8512_write(phy_handle, 18, 0x0012);
    ret += vsc8512_write(phy_handle, 17, 0x480a);
    ret += vsc8512_write(phy_handle, 16, 0x8f82);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0034);
    ret += vsc8512_write(phy_handle, 16, 0x8f80);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0012);
    ret += vsc8512_write(phy_handle, 16, 0x82e0);
    ret += vsc8512_write(phy_handle, 18, 0x0005);
    ret += vsc8512_write(phy_handle, 17, 0x0208);
    ret += vsc8512_write(phy_handle, 16, 0x83a2);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x9186);
    ret += vsc8512_write(phy_handle, 16, 0x83b2);
    ret += vsc8512_write(phy_handle, 18, 0x000e);
    ret += vsc8512_write(phy_handle, 17, 0x3700);
    ret += vsc8512_write(phy_handle, 16, 0x8fb0);
    ret += vsc8512_write(phy_handle, 18, 0x0004);
    ret += vsc8512_write(phy_handle, 17, 0x9fa1);
    ret += vsc8512_write(phy_handle, 16, 0x9688);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0xffff);
    ret += vsc8512_write(phy_handle, 16, 0x8fd2);
    ret += vsc8512_write(phy_handle, 18, 0x0003);
    ret += vsc8512_write(phy_handle, 17, 0x9fa0);
    ret += vsc8512_write(phy_handle, 16, 0x968a);
    ret += vsc8512_write(phy_handle, 18, 0x0020);
    ret += vsc8512_write(phy_handle, 17, 0x640b);
    ret += vsc8512_write(phy_handle, 16, 0x9690);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x2220);
    ret += vsc8512_write(phy_handle, 16, 0x8258);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x2a20);
    ret += vsc8512_write(phy_handle, 16, 0x825a);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x3060);
    ret += vsc8512_write(phy_handle, 16, 0x825c);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x3fa0);
    ret += vsc8512_write(phy_handle, 16, 0x825e);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0xe0f0);
    ret += vsc8512_write(phy_handle, 16, 0x83a6);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x4489);
    ret += vsc8512_write(phy_handle, 16, 0x8f92);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x7000);
    ret += vsc8512_write(phy_handle, 16, 0x96a2);
    ret += vsc8512_write(phy_handle, 18, 0x0010);
    ret += vsc8512_write(phy_handle, 17, 0x2048);
    ret += vsc8512_write(phy_handle, 16, 0x96a6);
    ret += vsc8512_write(phy_handle, 18, 0x00ff);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x96a0);
    ret += vsc8512_write(phy_handle, 18, 0x0091);
    ret += vsc8512_write(phy_handle, 17, 0x9880);
    ret += vsc8512_write(phy_handle, 16, 0x8fe8);
    ret += vsc8512_write(phy_handle, 18, 0x0004);
    ret += vsc8512_write(phy_handle, 17, 0xd602);
    ret += vsc8512_write(phy_handle, 16, 0x8fea);
    ret += vsc8512_write(phy_handle, 18, 0x00ef);
    ret += vsc8512_write(phy_handle, 17, 0xef00);
    ret += vsc8512_write(phy_handle, 16, 0x96b0);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x7100);
    ret += vsc8512_write(phy_handle, 16, 0x96b2);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x5064);
    ret += vsc8512_write(phy_handle, 16, 0x96b4);
    ret += vsc8512_write(phy_handle, 18, 0x0050);
    ret += vsc8512_write(phy_handle, 17, 0x100f);
    ret += vsc8512_write(phy_handle, 16, 0x87fa);
#ifndef VTSS_10BASE_TE 
    ret += vsc8512_phy_page_tr(phy_handle);
    ret += vsc8512_write(phy_handle, 18, 0x0071);
    ret += vsc8512_write(phy_handle, 17, 0xf6d9);
    ret += vsc8512_write(phy_handle, 16, 0x8488);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0db6);
    ret += vsc8512_write(phy_handle, 16, 0x848e);
    ret += vsc8512_write(phy_handle, 18, 0x0059);
    ret += vsc8512_write(phy_handle, 17, 0x6596);
    ret += vsc8512_write(phy_handle, 16, 0x849c);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0514);
    ret += vsc8512_write(phy_handle, 16, 0x849e);
    ret += vsc8512_write(phy_handle, 18, 0x0041);
    ret += vsc8512_write(phy_handle, 17, 0x0280);
    ret += vsc8512_write(phy_handle, 16, 0x84a2);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a4);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a6);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a8);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84aa);
    ret += vsc8512_write(phy_handle, 18, 0x007d);
    ret += vsc8512_write(phy_handle, 17, 0xf7dd);
    ret += vsc8512_write(phy_handle, 16, 0x84ae);
    ret += vsc8512_write(phy_handle, 18, 0x006d);
    ret += vsc8512_write(phy_handle, 17, 0x95d4);
    ret += vsc8512_write(phy_handle, 16, 0x84b0);
    ret += vsc8512_write(phy_handle, 18, 0x0049);
    ret += vsc8512_write(phy_handle, 17, 0x2410);
    ret += vsc8512_write(phy_handle, 16, 0x84b2);
#else //using 10BASE-Te
    ret += vsc8512_phy_page_ext2(phy_handle);
    ret += vsc8512_write(phy_handle, 17, 0x8000);
    ret += vsc8512_phy_page_tr(phy_handle);
    ret += vsc8512_write(phy_handle, 18, 0x0008);
    ret += vsc8512_write(phy_handle, 17, 0xa499);
    ret += vsc8512_write(phy_handle, 16, 0x8486);
    ret += vsc8512_write(phy_handle, 18, 0x0075);
    ret += vsc8512_write(phy_handle, 17, 0xf759);
    ret += vsc8512_write(phy_handle, 16, 0x8488);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0914);
    ret += vsc8512_write(phy_handle, 16, 0x848a);
    ret += vsc8512_write(phy_handle, 18, 0x00f7);
    ret += vsc8512_write(phy_handle, 17, 0xff7b);
    ret += vsc8512_write(phy_handle, 16, 0x848c);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0eb9);
    ret += vsc8512_write(phy_handle, 16, 0x848e);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x85d6);
    ret += vsc8512_write(phy_handle, 16, 0x8490);
    ret += vsc8512_write(phy_handle, 18, 0x0055);
    ret += vsc8512_write(phy_handle, 17, 0x44d2);
    ret += vsc8512_write(phy_handle, 16, 0x8492);
    ret += vsc8512_write(phy_handle, 18, 0x0044);
    ret += vsc8512_write(phy_handle, 17, 0xa8aa);
    ret += vsc8512_write(phy_handle, 16, 0x8494);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0cb9);
    ret += vsc8512_write(phy_handle, 16, 0x8496);
    ret += vsc8512_write(phy_handle, 18, 0x00f7);
    ret += vsc8512_write(phy_handle, 17, 0xff79);
    ret += vsc8512_write(phy_handle, 16, 0x8498);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0caa);
    ret += vsc8512_write(phy_handle, 16, 0x849a);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x8618);
    ret += vsc8512_write(phy_handle, 16, 0x849c);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0618);
    ret += vsc8512_write(phy_handle, 16, 0x849e);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0018);
    ret += vsc8512_write(phy_handle, 16, 0x84a0);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x848a);
    ret += vsc8512_write(phy_handle, 16, 0x84a2);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a4);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a6);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84a8);
    ret += vsc8512_write(phy_handle, 18, 0x0000);
    ret += vsc8512_write(phy_handle, 17, 0x0000);
    ret += vsc8512_write(phy_handle, 16, 0x84aa);
    ret += vsc8512_write(phy_handle, 18, 0x0029);
    ret += vsc8512_write(phy_handle, 17, 0x265d);
    ret += vsc8512_write(phy_handle, 16, 0x84ac);
    ret += vsc8512_write(phy_handle, 18, 0x007d);
    ret += vsc8512_write(phy_handle, 17, 0xd658);
    ret += vsc8512_write(phy_handle, 16, 0x84ae);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x8618);
    ret += vsc8512_write(phy_handle, 16, 0x84b0);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x8618);
    ret += vsc8512_write(phy_handle, 16, 0x84b2);
    ret += vsc8512_write(phy_handle, 18, 0x0061);
    ret += vsc8512_write(phy_handle, 17, 0x8618);
    ret += vsc8512_write(phy_handle, 16, 0x84b4);
#endif

    // Improve 100BASE-TX link startup robustness to address interop issue
    ret += vsc8512_write(phy_handle, 18, 0x0068);
    ret += vsc8512_write(phy_handle, 17, 0x8980);
    ret += vsc8512_write(phy_handle, 16, 0x8f90);
    
    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write_mask(phy_handle,  8, 0x0000, 0x8000);
    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 22, 0x0000, 0x0001);

    if (ret)
    {
        return RESULT_ERROR;
    }

    //VTSS_I("luton26_atom12_revCD_init_script done");
    return vsc8512_rev_c_patch(phy_handle);
}


static int32
vsc8512_get_chip_revision(phy_handle_t* phy_handle, uint16* phy_revision)
{
    int32 ret = 0;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    ret += vsc8512_read(phy_handle, 0x3, phy_revision);
    (*phy_revision) &= 0xf;
    
    return ret;
}

#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS) 
// function cfg_ib(port_no, ib_cterm_ena, ib_eq_mode);
//   set the ib_cterm_ena and ib_eq_mode bits to the input values (0 or 1)
//   ib_cterm_ena => cfg_vec[91] => cfg_buf[11].3
//   ib_eq_mode    => cfg_vec[90] => cfg_buf[11].2
//
//   ib_rst       => cfg_vec[86] => cfg_buf[10].6
//
static int32
vtss_phy_atom12_cfg_ib_cterm_ena_private(phy_handle_t* phy_handle, uint8 ib_cterm_ena, uint8 ib_eq_mode)
{
    uint8 tmp, tmp1;
    uint16 reg18g = 0;
    int32 ret = RESULT_OK;

    //VTSS_I("ib_eq_mode:%d, ib_cterm_ena:%d", ib_eq_mode, ib_cterm_ena);

    // fetch MCB data
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8113);     // Read MCB for 6G macro 1 into PRAM
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    // Assert ib_rst
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0xd7d9);     // set mem_addr to cfg_buf[10] (0x47d9)
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8007);     // read the value of cfg_buf[10] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and OR in ib_rst (bit 6)
    ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
    tmp1 = (uint8)(((reg18g >> 4) & 0xff) | 0x40);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9006 | (tmp1 << 4));    // write the value back to cfg_buf[10] w/ post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    // update ib_cterm_ena and ib_eq_mode
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8007);     // read the value of cfg_buf[11] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and mask off bits 2 & 3
    ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
    tmp = (uint8)((reg18g >> 4) & 0xf3);
    // OR in ib_cterm_ena shifted by 3 bits
    tmp |= ((ib_cterm_ena & 1) << 3);
    // OR in ib_eq_mode shifted by 2 bits
    tmp |= ((ib_eq_mode & 1) << 2);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp << 4));    // write the value back to cfg_buf[11] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    // Update MCB w/ ib_rst asserted
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0xd7ce);     // set mem_addr to 0x47ce (addr_vec)
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x80e6);     // set addr_vec for macros 1-3
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9cc0);     // Write MCB for 6G macros 1-3 from PRAM
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    // Clear ib_rst
    // mask off ib_rst (bit 6)
    tmp1 &= 0xbf;

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0xd7d9);     // set mem_addr to cfg_buf[10] (0x47d9)
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp1 << 4));    // write the value back to cfg_buf[10] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    // Update MCB w/ ib_rst cleared
    ret += vsc8512_phy_page_gpio(phy_handle);     // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9cc0);     // Write MCB for 6G macros 1-3 from PRAM
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    //Switch to std page.
    ret += vsc8512_phy_page_std(phy_handle);
    return ret;
}

// Function to set the ob_post0 parameter after configuring the 6G macro for QSGMII for ATOM12
// In : port_no - Port to configure
//      val     - Value to configure
static int32
vtss_phy_cfg_ob_post0_private(phy_handle_t* phy_handle, uint8 val)
{
    uint8 tmp;
    uint16 reg18g = 0;
    int32 ret = RESULT_OK;

    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8113);  // Read MCB for 6G macro 1 into PRAM
    (void)  vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0xd7d8);  // set mem_addr to cfg_buf[9] (0x47d8)
    (void)vsc8512_wait_for_micro_complete(phy_handle);
    
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8007);  // read the value of cfg_buf[9] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and mask off upper 3 bits
    ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
    tmp = (uint8)((reg18g >> 4) & 0x1f);
    // OR in lower 3 bits of val shifted aprropriately
    tmp |= ((val & 7) << 5);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    // write the value back to cfg_buf[9] w/ post-incr.
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9006 | ((uint16)tmp << 4));
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8007);  // read the value of cfg_buf[10] w/o post-incr.
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and mask off lower 3 bits
    ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &reg18g);
    tmp = (uint8)((reg18g >> 4) & 0xf8);
    // OR in upper 3 bits of val shifted aprropriately
    tmp |= ((val >> 3) & 7);
    
    ret = vsc8512_phy_page_gpio(phy_handle);;       // Switch back to micro/GPIO register-page
    // write the value back to cfg_buf[10] w/o post-incr.
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8006 | ((uint16)tmp << 4));
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0xd7ce);  // set mem_addr to 0x47ce (addr_vec)
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x80e6);  // set addr_vec for macros 1-3
    (void) vsc8512_wait_for_micro_complete(phy_handle);
    
    ret = vsc8512_phy_page_gpio(phy_handle);       // Switch back to micro/GPIO register-page
    ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9cc0);  // Write MCB for 6G macros 1-3 from PRAM
    (void) vsc8512_wait_for_micro_complete(phy_handle);

    //Switch to std page.
    ret += vsc8512_phy_page_std(phy_handle);
    return ret;
}
#endif


/* Function for suspending / resuming the 8051 patch.
 *
 * In : iport - Any port within the chip where to supend 8051 patch
 *      suspend - True if 8051 patch shall be suspended, else patch is resumed.
 * Return : VTSS_RC_OK if patch was suspended else error code. 
 */
int32
vsc8512_patch_suspend(phy_handle_t* phy_handle, uint8 suspend) 
{
    uint16 word;
    int32 ret = RESULT_OK;

    ret += vsc8512_phy_page_gpio(phy_handle); // Change to GPIO page
    // We must disable the EEE patch when VeriPHY is running,  When VeriPHY  is running, the patch  needs to be suspended by 
    // writing 0x800f to register 18 on the Micro/GPIO page (0x10).  0x800f is chosen as this is an unimplemented micro-command 
    // and issuing any micro command when the patch is running, causes the patch to stop. 
    // The micro patch must not be enabled until all pending VeriPHY requests have been completed on all ports in the relevant Luton26 or Atom12 chip. 
    // When all have been completed, the micro patch should re-enable.  Note that this is necessary only because the patch for EEE consumes 
    //the micro continually to service all 12 PHYs in a timely manner and workaround one of the weaknesses in gigabit EEE in Luton26/Atom12.
    if ((phy_handle->phy_info.phy_veriphy_tsk != NULL)&&(phy_handle->phy_info.phy_veriphy_tsk->ret==GLB_VCT_WAIT)) {
        ret +=vsc8512_write(phy_handle, 18, 0x800F); // Suspend 8051 EEE patch 
        ret +=vsc8512_wait_for_micro_complete(phy_handle);
        ret +=vsc8512_phy_page_std(phy_handle); // Back to standard page
        return ret;
    } 

    if (suspend) 
    {
        /* Suspending 8051 patch */
        if(phy_handle->phy_info.phy_revision == GLB_VTSS_PHY_ATOM_REV_A)
        {
            // From JimB
            // You are suspending the micro patch momentarily by writing
            // 0x9014 to GPIO-page register 18 and resuming by writing 0x9004 to 
            // GPIO-page register 18.. 
            ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9014); // Suspend vga patch 
            ret += vsc8512_wait_for_micro_complete(phy_handle);
        }
        else
        {
            ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &word);
            if (!(word & 0x4000)) 
            {
                ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x800F); // Suspend 8051 EEE patch 
            }
            ret += vsc8512_wait_for_micro_complete(phy_handle);
        }
        ret += vsc8512_wait_for_micro_complete(phy_handle);
    } 
    else 
    {
        if(phy_handle->phy_info.phy_revision == GLB_VTSS_PHY_ATOM_REV_A)
        {
            ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x9004);
            ret += vsc8512_wait_for_micro_complete(phy_handle);
        }
        else
        {
            /*
             * Resuming 8051 patch
             * On page 0x10 (Reg31 = 0x10), write register 18 with 0x8009 to turn on the EEE patch.  
             * Once this is done all code that might attempt to access page 0x52b5 will fail and likely cause issues.  
             * If you need to access page 0x52b5 or run another page 0x10, register 18 function, you must first disable 
             * the patch by writing 0x8009 again to register 18.  In response, the error bit (bit 14) will be set, 
             * but the micro is now freed from running the EEE patch and you may issue your other micro requests. 
             * When the other requests are complete, you will want to rerun the EEE patch by writing 0x8009 to register 18 on page 0x10.  
             * The events that are handled by the micro patch occur occasionally, say one event across 12 ports every 30 seconds.
             * As a result, suspending the EEE patch for short durations Is unlikely to result in link drops, but it is possible.
    		 */
            ret += vsc8512_read(phy_handle, VTSS_PHY_MICRO_PAGE, &word);
            if (word & 0x4000) 
            {
                ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8009); 
            }
            ret += vsc8512_wait_for_micro_complete(phy_handle);
        }
        ret += vsc8512_wait_for_micro_complete(phy_handle);
    }

    ret += vsc8512_phy_page_std(phy_handle); // Change to standard page
    
    return ret;
}

static int32 vsc8512_phy_set_private_atom(phy_handle_t* phy_handle, uint8 is_force)
{
    int32 ret=0;
    ret += vsc8512_patch_suspend(phy_handle, 1);
    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write_mask(phy_handle,  8, 0x8000, 0x8000); //Ensure RClk125 enabled even in powerdown
    // Clear Cl40AutoCrossover in forced-speed mode, but set it in non-forced modes
    ret += vsc8512_phy_page_tr(phy_handle);
    ret += vsc8512_write(phy_handle, 16, 0xa7fa);  // issue token-ring read request
    ret += vsc8512_write_mask(phy_handle, 17, (is_force)? 0x0000 : 0x1000, 0x1000);
    ret += vsc8512_write(phy_handle, 16, 0x87fa);  // issue token-ring write request

    ret += vsc8512_write(phy_handle, 16, 0xaf82);
    ret += vsc8512_write_mask(phy_handle, 17, 0x2, 0xf);
    ret += vsc8512_write(phy_handle, 16, 0x8f82);

    ret += vsc8512_phy_page_test(phy_handle);
    ret += vsc8512_write_mask(phy_handle,  8, 0x0000, 0x8000); //Restore RClk125 gating
    ret += vsc8512_patch_suspend(phy_handle, 0);
    ret += vsc8512_phy_page_std(phy_handle);

    // Enable HP Auto-MDIX in forced-mode (by clearing disable bit)
    ret += vsc8512_write_mask(phy_handle, BYPASS_CTL, 0x0000, 0x80); 
    return ret;
}

/*Now only use MDIX-Auto.*/
static int32 vsc8512_phy_mdi_setup(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    ret += vsc8512_write_mask(phy_handle, BYPASS_CTL, 0x0, 0xa0);
    ret += vsc8512_phy_page_ext1(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 19, 0x0, 0xc);
    //Switch to std page.
    ret += vsc8512_phy_page_std(phy_handle);
    return ret;
}


/*
 * Function that is called at boot, before port reset.
 * Reset stuff needed before PHY port reset, any port will do (Only at start-up initialization) 
 */
static int32
vsc8512_phy_pre_reset(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    phy_info_t* pphy_info;
    
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
    pphy_info = &phy_handle->phy_info;
    switch (pphy_info->phy_revision)
    {
    case GLB_VTSS_PHY_ATOM_REV_A:
        ret += vsc8512_vga_patch_27_09_2010(phy_handle);
        
        /* Start the vga patch */
        ret += vsc8512_phy_page_gpio(phy_handle);
        ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8025);
        
        ret += vsc8512_wait_for_micro_complete(phy_handle);        
        break;
        
    case GLB_VTSS_PHY_ATOM_REV_B:
        ret += vsc8512_rev_b_init_script(phy_handle);
        break;
        
    case GLB_VTSS_PHY_ATOM_REV_C:
    case GLB_VTSS_PHY_ATOM_REV_D:
        ret += vsc8512_rev_cd_init_script(phy_handle);
        break;
        
    default:
        return RESULT_ERROR;
    }
    
    return ret;
}

static int32 vsc8512_phy_soft_reset_port(phy_handle_t* phy_handle)
{
    uint16 reg_val = 0, count = 0;
    int32 ret=0;
    
    ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x8000, 0x8000);
    usleep(40000);
    while(1)
    {        
        ret += vsc8512_read(phy_handle, PHY_CTL_REG, &reg_val);
        if((reg_val & 0x8000) == 0)
        {
            break;
        }
        usleep(20000);
        count++;
        if(count >= 250)
        {
            break;
        }
    }
    ret += vsc8512_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed,phy_handle->phy_info.phy_manage_info.duplex);
    return ret;
}

int32 vsc8512_phy_mac_media_if_atom_setup(phy_handle_t* phy_handle,glb_mac_interface_t mac_if,glb_media_interface_t  media_type,uint8 first_init)
{
    phy_info_t* pphy_info;    
    uint16 reg23  = 0;   /* register addr 23 */
    uint16 reg19g = 0;   /* register addr 19G */
    uint8 at_least_one_fiber_port=0, fi_1000_mode=0, fi_100_mode=0;
    int32 ret = 0;

    pphy_info = &phy_handle->phy_info;
    
    switch (mac_if)
    {
        case GLB_MAC_INTERFACE_SGMII:
            reg19g = (1 << 14);     /* Mode 1: SGMII to CAT5 mode */
            reg23  = (0 << 12);     /* SGMII MAC */            
            break;
        case GLB_MAC_INTERFACE_SERDES:
            reg19g = (1 << 14);     /* Mode 1: SGMII to CAT5 mode */
            reg23  = (1 << 12);     /* SerDes MAC, for 1000BASE-X only */
            break;
        case GLB_MAC_INTERFACE_QSGMII:
            switch (media_type)
            {
                case GLB_MEDIA_FI_100FX:
                case GLB_MEDIA_AMS_CU_100FX:
                case GLB_MEDIA_AMS_FI_100FX:
                case GLB_MEDIA_1000BX:
                case GLB_MEDIA_AMS_FI_PASSTHRU:    
                case GLB_MEDIA_AMS_CU_1000BX:    
                case GLB_MEDIA_AMS_FI_1000BX:
                    at_least_one_fiber_port = 1;
                    break;
                default:
                    break;
            }
            if(at_least_one_fiber_port)
            {
                reg19g = 2<<14; //QSGMII to CAT5 & Fiber mode                
            }
            else
            {
                reg19g = 0<<14;// QSGMII to CAT5 mode
            }
            break;
        default:
            DRV_LOG_WARN("VSC mac interface [%d] is unknown.", mac_if);
            return RESULT_ERROR;
    }
    
    switch (media_type)
    {
        case GLB_MEDIA_RJ45:
            reg23 |= 0<<8;
            break;
        case GLB_MEDIA_PASS_THROUGH:
            reg23 |= 1<<8;
            break;
        case GLB_MEDIA_1000BX:
            reg23 |= 2<<8;
            fi_1000_mode = 1;
            break;
        case GLB_MEDIA_FI_100FX:
            reg23 |= 3<<8;
            fi_100_mode = 1;
            break;
        case GLB_MEDIA_AMS_CU_PASSTHRU:
            reg23 |= 4<<8;
            reg23 |= 1<<11;
            break;
        case GLB_MEDIA_AMS_FI_PASSTHRU:
            reg23 |= 5<<8;
            reg23 |= 0<<11;
            fi_1000_mode = 1;
            break;
        case GLB_MEDIA_AMS_CU_1000BX:
            reg23 |= 6<<8;
            reg23 |= 1<<11;
            fi_1000_mode = 1;
            break;
        case GLB_MEDIA_AMS_FI_1000BX:
            reg23 |= 6<<8;
            reg23 |= 0<<11;
            fi_1000_mode = 1;
            break;
        case GLB_MEDIA_AMS_CU_100FX:
            reg23 |= 7<<8;
            reg23 |= 1<<11;
            fi_100_mode = 1;
            break;
        case GLB_MEDIA_AMS_FI_100FX:
            reg23 |= 7<<8;
            reg23 |= 0<<11;
            fi_100_mode = 1;
            break;
        default:
            DRV_LOG_WARN("VSC media interface [%d] is unknown.", media_type);
            break;
    }
    /* for bug 49724,49862, modified by tongzb 2018-11-07 */
    if((first_init)||(pphy_info->phy_manage_info.mac_if != mac_if))
    {
        /* Do the register accesses */
        ret += vsc8512_phy_page_gpio(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 19, reg19g, 0xC000);
    }
    
    ret += vsc8512_patch_suspend(phy_handle, TRUE);  /* Suspend 8051 Patch. */

    if((first_init)||(pphy_info->phy_manage_info.mac_if != mac_if))
    {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        /*Only used when sync_calling_private == 1*/
//        ret += vsc8512_phy_warmstart_chk_micro_patch_mac_mode(phy_handle);
#endif        
        if(mac_if == GLB_MAC_INTERFACE_QSGMII)
        {
            ret += vsc8512_phy_page_gpio(phy_handle);

            //  Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
            ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x80A0); // See TN1085

#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)                
            // ib_cterm and ob_post0 patches is only designed to run on Atom12 devices where the 8051 controls the MCB (otherwise the dreaded timeout will occur).
            // ib_cterm called to override the ROM default.
            ret += vtss_phy_atom12_cfg_ib_cterm_ena_private(phy_handle, VSC8512_ib_cterm_ena, 1);

            // ob post called to override the ROM default of 2.
            ret += vtss_phy_cfg_ob_post0_private(phy_handle, pphy_info->phy_manage_info.trace_length);
#endif
        } 
        else 
        {
            ret += vsc8512_phy_page_gpio(phy_handle);
            //  Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
            ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x80B0);// Seee TN1085
        }
            
            // Wait for micro to complete MCB command to configure for QSGMII
            ret += vsc8512_wait_for_micro_complete(phy_handle);
    }

    ret += vsc8512_phy_page_gpio(phy_handle);

    if (fi_100_mode)
    {
        ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE , 0x8091 | 0x0100 << ((pphy_info->phy_addr) % 4));
    }
    
    if (fi_1000_mode) 
    {
        ret += vsc8512_write(phy_handle, VTSS_PHY_MICRO_PAGE, 0x8081 | 0x0100 << ((pphy_info->phy_addr) % 4));
    }

    /* Wait for micro to complete MCB command */
    ret += vsc8512_wait_for_micro_complete(phy_handle);
    ret += vsc8512_patch_suspend(phy_handle, FALSE); /* Resume 8051 Patch. */

    ret += vsc8512_phy_page_std(phy_handle);
    DRV_LOG_DEBUG(DRV_PHY, "reg23 = 0x%X, media type = %d, mac_if = %d", 
           reg23, pphy_info->phy_manage_info.media_type, pphy_info->phy_manage_info.mac_if);
    ret += vsc8512_write(phy_handle, 23, reg23);

    // Port must be reset in order to update the media setting for register 23
    ret += vsc8512_phy_soft_reset_port(phy_handle);
    sal_udelay(10 * 1000);
    
    // Bugzilla#8714 - LabQual failures in QSGMII operation on FFFF parts 
    // The sequence of PHY writes setting register 20E3.2 must be after the soft-reset.  
    // This is because comma-bypass is envisioned as a debug bit that
    // should be reset when soft-reset is asserted; however, in this case, when the
    // QSGMII MAC interface is selected, it is desired to always bypass the
    // comma-detection logic to avoid issues on corner silicon. 

    if (GLB_MAC_INTERFACE_QSGMII == mac_if)
    {
        ret += vsc8512_phy_page_ext3(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 20, 0x0004, 0x0004);
    }

    ret += vsc8512_phy_page_std(phy_handle);    
    return ret;
}

// Function for resetting a phy port
// In: port_no : The phy port number to be reset
static int vsc8512_port_reset(phy_handle_t* phy_handle)
{
    int32 ret=0;
    ret += vsc8512_patch_suspend(phy_handle, 1);// Suspend the micro patch while resetting
    ret += vsc8512_phy_soft_reset_port(phy_handle);
    ret += vsc8512_patch_suspend(phy_handle, 0);// Restart micro patch
    return ret;
}

// Initializing 8512/8522/Luton26 PHY for the individual ports.
// In  : port_no       - Phy port number
//       luton26_mode  - Set to TRUE if the phy is the internal Luton26 phy.
int32 vsc8512_phy_init_seq_atom(phy_handle_t* phy_handle)
{
    int32 ret=0;
    if (phy_handle->phy_info.phy_revision == GLB_VTSS_PHY_ATOM_REV_A) 
    {
        // Workaround for REV A
        ret += vsc8512_phy_page_test(phy_handle);
        ret += vsc8512_write(phy_handle, 25, 1) ; //Inhibit low-power features that may or may not be causing trouble
        ret += vsc8512_write(phy_handle, 22, 0x1800); // band-gap boost +3 setting, leave cac and vac trims alone for now
        ret += vsc8512_write(phy_handle, 8, 0x8012); // Make sure TRClk125 is running (needed during COMA)
        ret += vsc8512_phy_page_tr(phy_handle);
        ret += vsc8512_write(phy_handle, 18, 3);
        ret += vsc8512_write(phy_handle, 17, 0); // Use minimum setting for 10BT line-drivers as all higher-power setting increase 200MHz noise on transmitter
        ret += vsc8512_write(phy_handle, 16, 0x8fe0);
        ret += vsc8512_phy_page_test(phy_handle);
        ret += vsc8512_write(phy_handle, 8, 0x8012); // Restore normal gating for TRClk125
        ret += vsc8512_phy_page_std(phy_handle);
    }
    return ret;
}

/* 
 * in vtss_phy_sdk, this function is called by vtss_phy_reset, 
 * must do this after pre-reset at boot time 
 */
static int32
vsc8512_phy_reset(phy_handle_t* phy_handle, glb_mac_interface_t mac_if,glb_media_interface_t  media_type, uint8 first_init)
{
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    /* -- Step 2: Pre-reset setup of MAC and Media interface -- */
    ret += vsc8512_phy_mac_media_if_atom_setup(phy_handle, mac_if, media_type, first_init);
    /* -- Step 3: Reset PHY -- */
    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_port_reset(phy_handle);
    /* -- Step 4: Run startup scripts -- */
    ret += vsc8512_phy_init_seq_atom(phy_handle);
    return ret;
}

// Function for setting phy in pass through mode according to "Application Note : Protocol transfer mode guide"
static int32 vsc8512_phy_pass_through_speed_mode(phy_handle_t* phy_handle)
{
    int32 ret=0;
    ret += vsc8512_phy_page_std(phy_handle);
    //Protocol Transfer mode Guide : Section 4.1.1 - Aneg must be enabled
    ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x1000, 0x1000);

    ret += vsc8512_phy_page_ext3(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 16, 0x0080, 0x0880); // Default clear "force advertise ability" bit as well

    // Protocol Transfer mode Guide : Section 4.1.3
    if((phy_handle->phy_info.phy_manage_info.speed != GLB_SPEED_AUTO)
        &&(phy_handle->phy_info.phy_manage_info.duplex != GLB_DUPLEX_AUTO))
    {
        ret += vsc8512_write_mask(phy_handle, 16, 0x0800, 0x0800);
        switch (phy_handle->phy_info.phy_manage_info.speed) 
        {
            case GLB_SPEED_100M:
                ret += vsc8512_write(phy_handle, 18, 0x8401);
                break;
            case GLB_SPEED_10M:
                ret += vsc8512_write(phy_handle, 18, 0x8001);
                break;
            case GLB_SPEED_1G:
                ret += vsc8512_write(phy_handle, 18, 0x8801);
                break;
            default:
                DRV_LOG_ERR("Unexpected port speed:%d defaulting to 1G", phy_handle->phy_info.phy_manage_info.speed);
                ret += vsc8512_write(phy_handle, 18, 0x8801);
                break;
        }
    }
    ret += vsc8512_phy_page_std(phy_handle);
    return ret;
}


static int32
vsc8512_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
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
        ret += vsc8512_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0);
        ret += vsc8512_write_mask(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300);
        ret += vsc8512_phy_set_private_atom(phy_handle, 0);
        ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);        
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
            /* Modified by liuht to powerdown more time to notify link parter for bug25796, 2013-11-14 */		
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0100, 0x3140);
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x2100, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x2000, 0x3140);
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0140, 0x3140);
        }
        else /*1G Half*/
        {    
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0040, 0x3140);
        }
        if(speed != GLB_SPEED_1G)
        {
            ret += vsc8512_phy_set_private_atom(phy_handle, 1);
        }
    }
    ret += vsc8512_phy_mdi_setup(phy_handle);

    if(phy_handle->phy_info.phy_manage_info.media_type == GLB_MEDIA_PASS_THROUGH)
    {
        ret += vsc8512_phy_pass_through_speed_mode(phy_handle);
    }
    else
    {
        ret += vsc8512_phy_page_ext3(phy_handle);
        // Recommended by chip designers - This should allow link-up if the MAC is not doing auto-neg.
        // Enable "MAC interface autonegotiation parallel detect", else data flow is stopped for the CU ports if PHY has MAC ANEG enabled and the switch is connected to isn't
        ret += vsc8512_write_mask(phy_handle, 16, 0x2000, 0x2000);
        ret += vsc8512_phy_page_std(phy_handle);
    } 
    return ret;
}    

/*
 * Function for pulling the coma mode pin low
 * Do reset stuff that is needed after port reset, any port will do (Only at start-up initialization) 
 */
static int32
vsc8512_coma_mode_disable(phy_handle_t* phy_handle)
{
    int32 ret = RESULT_OK;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    
/* 
 * COMA mode is a new concept for Luton26/Atom12 for the PHYs and when the COMA_MODE pin is asserted, the PHY will 
 * remain powered down which is the ideal state to configure the PHY and SerDes MAC interfaces.  C
 * COMA_MODE is designed to also synchronize LED clocks upon deassertion of COMA_MODE.  T
 * The board designer should tie all COMA_MODE pins together. When the pin floats, an internal pull-up asserts the COMA_MDOE pin.  
 * After all internal Luton26 and external Atom12 PHYs have been configured, the initialization scripts should configure one of the devices to 
 * drive COMA_MODE pin low by changing COMA_MODE from an input to an output and setting the output to 0.  This will be sensed simultaneously 
 * at all PHYs and synchronize LED clocks between these PHYs.  Note that the LED blink on reset will occur, if configured, at the time that COMA_MODE goes low.
 *  
 *  
 * The COMA_MODE pin may be configured using micro/GPIO register 14G.[13:12]. 
 * Bit 13 is an active-low output enable which defaults on reset to 1 (e.g. output disabled). 
 * Bit 12 is the bit to drive the COMA_MODE pin to when the output is enabled.    
 */
    ret += vsc8512_phy_page_gpio(phy_handle);
    ret += vsc8512_write_mask(phy_handle, VTSS_PHY_GPIO_CONTROL_2, 0x0000, 
                               VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE | VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_DATA);
    ret += vsc8512_phy_page_std(phy_handle);
    
    return ret;
}

static int32 
vsc8512_phy_post_reset(phy_handle_t* phy_handle)
{
    int32 ret = RESULT_OK;
    ret = vsc8512_coma_mode_disable(phy_handle);
    return ret;
}

static int32
vsc8512_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VSC8512 phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
    }
    else 
    {
        /*When do vct, not powerdown phy. bug25277*/
        if((phy_handle->phy_info.phy_veriphy_tsk != NULL)&&(phy_handle->phy_info.phy_veriphy_tsk->ret==GLB_VCT_WAIT))
        {            
        }
        else
        {
            /*power down*/
            ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
        }
    }
    return ret;
}

int32 vsc8512_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media,
                                glb_port_master_slave_t master_slave)
{
    uint16 vga_stat;
    int16 max_vga_state_to_opimize = -9;
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8512_link_up_setting speed: %d duplex: %d", speed,duplex);
    if(speed == GLB_SPEED_1G)
    {
        /* BZ 1776/1860/2095/2107 part 3/3 */
        ret += vsc8512_phy_page_tr(phy_handle);
        ret += vsc8512_write(phy_handle, 16, 0xaff0);
        ret += vsc8512_read(phy_handle, 17, &vga_stat);
        vga_stat = ((vga_stat >> 4) & 0x01f);
        /* vga_stat is a 2's complement signed number ranging from -13 to +8 */
        /* Test for vga_stat < 16 is really a check for positive vga_stat */
        /* for PHY VSC8512, max_vga_state_to_optimize is equal to -9 */
        if ((vga_stat < 16) || (vga_stat > (max_vga_state_to_opimize & 0x1f))) 
        {
            DRV_LOG_DEBUG(DRV_PHY, "vga_stat = %u", vga_stat);
            ret += vsc8512_phy_page_test(phy_handle);
            ret += vsc8512_write_mask(phy_handle, 12, 0x0000, 0x0300);
        }
        ret += vsc8512_phy_page_std(phy_handle);
    }
    return ret;
}


int32 
vsc8512_link_down_setting(phy_handle_t* phy_handle)
{
    phy_info_t *pphy_info;    
    uint16 value; 
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8512_link_down_setting ");
    pphy_info = &phy_handle->phy_info;

    if (GLB_VTSS_PHY_ATOM_REV_B == pphy_info->phy_revision
        || GLB_VTSS_PHY_ATOM_REV_A == pphy_info->phy_revision) 
    {
        ret += vsc8512_read(phy_handle, PHY_CTL_REG, &value);
        if (!(value & 0x0800))  /* Power-down enabled. */
        {
            /* pt. This doesn't work for SFP ports */
            if (GLB_MEDIA_RJ45 == pphy_info->phy_manage_info.media_type 
                && (value & 0x1000)) /* auto-negotiation enabled. */ 
            {
                /* Work-around for Vittesse PHY chip problem with going from 10/100 mode to 1000 auto-neg */
                ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
                DRV_LOG_DEBUG(DRV_PHY, "media type = %d, phy addr 0x%x", 
                    pphy_info->phy_manage_info.media_type, pphy_info->phy_addr);
                ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
            }
        }
    }
    // Work-around for bugzilla#8381 - Link not coming up if disconnected while transmitting (only in 10 Mbit forced mode).
    //We agree that the issue is still present and recommend the following workaround for bugzilla 8381:
    //When the link drops in any copper PHY speed, read MII register 1 and if the link status is still down, do the following:
    //If in forced 10BASE-T mode, momentarily force 10BASE-T link status (after you are sure the MAC is no longer transmitting to the PHY) and then unforce 10BASE-T link status using MII register 22, bit 15 toggling 1/0.
    //    Else momentarily force power-down using MII register 0, bit 11 toggling 1/0.
    // Note that if link status comes back up, the workaround wasnt required.  The only remaining danger is if link status just came up after reading link status and before power-down.  This is unlikely and unlikely to repeat.
    if ((pphy_info->phy_manage_info.speed != GLB_SPEED_AUTO)&&(pphy_info->phy_manage_info.duplex != GLB_DUPLEX_AUTO))
    {
        ret += vsc8512_read(phy_handle, PHY_STATUS_REG, &value);
        if (value & 0x4) 
        {
            
        } 
        else 
        {
            if (pphy_info->phy_manage_info.speed == GLB_SPEED_10M) 
            {
                ret += vsc8512_write_mask(phy_handle, 22, 0x8000, 0x8000);

                ret += vsc8512_write_mask(phy_handle, 22, 0x0000, 0x8000);
                usleep(20000); // Make sure that status register is updated before we clear it (20 mschosen due to link pulse interval timer).
                ret += vsc8512_read(phy_handle, PHY_STATUS_REG, &value); // Clear the status register, to get rid of the false link up, due to that we have just forced the link up/down.
            } 
            else 
            {
                ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
                /* Added by liuht for bug 38393, 2016-05-11 */
                vsc8512_enable(phy_handle, pphy_info->phy_manage_info.enable);
            }
        }
    }
    return ret;
} 

/**********************************************************************
 *  Name:       : phy_vsc8512_get_cur_status
 *  Purpose     : get current link status, speed and duplex per port
 *  Input       : mdio_bus_no: MDIO bus ID
                  phy_addr:    PHY address per MDIO bus
                  phy_status:  PHY status(link_up, speed, duplex)
 *  Output      : N/A
 *  Return      : N/A
 *  Note        : N/A
 ***********************************************************************/
int32
vsc8512_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t *phy_status)
{
    int32 ret = RESULT_OK;
    uint16 cfg_symmetric, cfg_asymmetric, lp_symmetric, lp_asymmetric;
    uint16 value;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    /* Get link status */
    ret += vsc8512_read(phy_handle, PHY_STATUS_REG, &value);
    phy_status->link_up = (value & 0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;
    
    /*if link down, need read again.*/
    if(phy_status->link_up == GLB_LINK_DOWN)
    {
        ret += vsc8512_read(phy_handle, PHY_STATUS_REG, &value);
        phy_status->link_up = (value & 0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;
    }
    
    if(phy_status->link_up == GLB_LINK_UP)
    {
        /* Get Speed */
        ret += vsc8512_read(phy_handle, AUX_PHY_STATUS_REG, &value);
        if((value & 0x18) == 0x00)
        {
            phy_status->speed = GLB_SPEED_10M;
        }
        else if((value & 0x18) == 0x8)
        {
            phy_status->speed = GLB_SPEED_100M;
        }
        else if((value & 0x18) == 0x10)
        {
            phy_status->speed = GLB_SPEED_1G;
        }

        /* Get Duplex */
        if((value & 0x20) == 0x00)
        {
            phy_status->duplex = GLB_DUPLEX_HALF;
        }
        else if((value & 0x20) == 0x20)
        {
            phy_status->duplex = GLB_DUPLEX_FULL;
        }
        /*If auto enable and auto complete, need get flow control info.*/
        if((value & 0xc000)==0x8000)
        {
            cfg_symmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause;
            cfg_asymmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause;
            ret += vsc8512_read(phy_handle, AUTO_NEGO_LP_ADVERTI_REG, &value);
            lp_symmetric = ((value>>10) & 0x1);
            lp_asymmetric = ((value>>11) & 0x1);
            phy_status->flowctrl.recv = (cfg_symmetric && (lp_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
            phy_status->flowctrl.send = (lp_symmetric && (cfg_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
        }
        else
        {
            phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
            phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        }
        /*get currently media mode status*/
        ret += vsc8512_phy_page_ext1(phy_handle);
        ret += vsc8512_read(phy_handle, 0x14, &value); /* Read Register Address 20E1 */
        ret += vsc8512_phy_page_std(phy_handle);

        /*if media mode is copper*/
        if((value & 0x00c0) == 0x40)
        {
            phy_status->linkup_media = GLB_PORT_TYPE_COPPER;
        }
        /*if media mode is fiber*/
        else
        {
            phy_status->linkup_media = GLB_PORT_TYPE_FIBER;            
        }
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        /*get currently mode : slave or master*/ 
        ret += vsc8512_phy_page_std(phy_handle);
        ret += vsc8512_read(phy_handle, 0xa, &value);
        /*if mode is master*/
        if((value & 0x4000) == 0x4000)
        {
            phy_status->master_slave= GLB_MODE_MASTER;
        }
        /*if mode is slave*/
        else
        {
            phy_status->master_slave= GLB_MODE_SLAVE;            
        }
        /* End of liuht modified */
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        /* get local phy's eee capability */
        vsc8512_mmd_reg_read(phy_handle, 7, 60, &value);
        if((value & 0x0002) == 0x2)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX;
        }
        if((value & 0x0004) == 0x4)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T;
        }
        
        /* get link partner phy's eee capability */
        vsc8512_mmd_reg_read(phy_handle, 7, 61, &value);
        if((value & 0x0002) == 0x2)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_100BASE_TX;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_100BASE_TX;
        }
        if((value & 0x0004) == 0x4)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_T;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_T;
        }
        /* get eee result */
        vsc8512_mmd_reg_read(phy_handle, 3, 1, &value);
        if((value & 0x0004) == 0x4)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LOCAL_LINK_STATUS;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LOCAL_LINK_STATUS;
        }
        if((value & 0x0100) == 0x0100)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LOCAL_RX_LPI_STATUS;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LOCAL_RX_LPI_STATUS;
        }
        if((value & 0x0200) == 0x0200)
        {
            phy_status->eee_status.status |= GLB_PHY_EEE_LOCAL_TX_LPI_STATUS;
        }
        else
        {
            phy_status->eee_status.status &= ~GLB_PHY_EEE_LOCAL_TX_LPI_STATUS;
        }
        vsc8512_mmd_reg_read(phy_handle, 3, 22, &value);
        phy_status->eee_status.wake_error_count = value & 0xffff;
        /* End of liuht modified */
    }
    else
    {
        phy_status->speed = GLB_SPEED_AUTO;
        phy_status->duplex = GLB_DUPLEX_AUTO;
        phy_status->linkup_media = GLB_PORT_TYPE_AUTO;
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        phy_status->master_slave = GLB_MODE_AUTO;
        /* End of liuht modified */
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        phy_status->eee_status.status= 0;
        phy_status->eee_status.wake_error_count = 0;
        /* End of liuht modified */
        phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }
    if(phy_handle->phy_info.phy_manage_info.media_type == GLB_MEDIA_PASS_THROUGH)
    {
        phy_status->speed = GLB_SPEED_1G;
        phy_status->duplex = GLB_DUPLEX_FULL;
        /* Modified by liuht for bug 28155, 2014-04-17 */
        /* Support force master or slave mode */
        phy_status->master_slave = GLB_MODE_MAX;
        /* End of liuht modified */
        phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }
    return ret;
}

int32 vsc8512_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    vsc8512_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8512 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        if(pphy_info->phy_stat_flag.eee_status.status != phy_val->eee_status.status ||
            pphy_info->phy_stat_flag.eee_status.wake_error_count != phy_val->eee_status.wake_error_count)
        {
            phy_change->eee_status_change = 1;
        }
        /* End of liuht modified */
        vsc8512_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8512 phy addr %d link down",pphy_info->phy_addr) 
        vsc8512_link_down_setting(phy_handle);
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
            vsc8512_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
        }
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        if(pphy_info->phy_stat_flag.eee_status.status != phy_val->eee_status.status ||
            pphy_info->phy_stat_flag.eee_status.wake_error_count != phy_val->eee_status.wake_error_count)
        {
            phy_change->eee_status_change = 1;
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
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    /* store eee status */
    pphy_info->phy_stat_flag.eee_status.status= phy_val->eee_status.status;
    pphy_info->phy_stat_flag.eee_status.wake_error_count = phy_val->eee_status.wake_error_count;
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
vsc8512_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phyreg_param_t phyreg_para;
    phy_info_t *pphy_info;
    int32 ret = RESULT_OK;
    uint16 val;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    /*if NO link change, return*/
    ret += vsc8512_read(phy_handle, PHY_INT_STATUS, &val);

    if(0 == val)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    if(PHY_WORK_MODE_PASSTHROUGH == pphy_info->phy_manage_info.mode)
    {
         return RESULT_OK;
    }
    ret += vsc8512_get_cur_status(phy_handle, phy_val);
    /*if fast link down init*/
    /*for bug 38162,guhg,2016-04-27 */
    if((val & 0x80) != 0)
    {
        phy_val->link_up = GLB_LINK_DOWN;
    }

    DRV_LOG_DEBUG(DRV_PHY,"vsc8512_get_cur_status speed: %d duplex: %d",phy_val->speed, phy_val->duplex);
    
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
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        if(pphy_info->phy_stat_flag.eee_status.status != phy_val->eee_status.status ||
            pphy_info->phy_stat_flag.eee_status.wake_error_count != phy_val->eee_status.wake_error_count)
        {
            phy_change->eee_status_change = 1;
        }
        /* End of liuht modified */

        ret += vsc8512_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        ret += vsc8512_link_down_setting(phy_handle);
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
            vsc8512_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media, phy_val->master_slave);
        }
        /* End of liuht modified */
        /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
        if(pphy_info->phy_stat_flag.eee_status.status != phy_val->eee_status.status ||
            pphy_info->phy_stat_flag.eee_status.wake_error_count != phy_val->eee_status.wake_error_count)
        {
            phy_change->eee_status_change = 1;
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
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    /* store eee status */
    pphy_info->phy_stat_flag.eee_status.status= phy_val->eee_status.status;
    pphy_info->phy_stat_flag.eee_status.wake_error_count = phy_val->eee_status.wake_error_count;
    /* End of liuht modified */
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }
    return ret;

}

int32
vsc8512_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = vsc8512_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

int32
vsc8512_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = vsc8512_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
int32
vsc8512_config_master_slave(phy_handle_t* phy_handle, uint8 master_slave)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config master-slave mode %d, orig %d", pphy_info->phy_addr, master_slave, pphy_info->phy_manage_info.master_slave);
    ret += vsc8512_phy_page_std(phy_handle);
    switch(master_slave)
    {
    case GLB_MODE_AUTO:
        ret |= vsc8512_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x0000, 0x1000);
        break;
    case GLB_MODE_MASTER:
        ret |= vsc8512_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1800, 0x1800);
        break;
    case GLB_MODE_SLAVE:
        ret |= vsc8512_write_mask(phy_handle, DIAG_1G_CTL_REG, 0x1000, 0x1800);
        break;
    default:
        return RESULT_ERROR;
        break;
    }
    ret = vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    return ret;
}
/* End of liuht modified */

int32
vsc8512_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= vsc8512_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        break;

    case GLB_LB_PHY_FAR:
        ret |= vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= vsc8512_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0008, 0x0008);  /* Enable Far-End Loopback */
        break;

    case GLB_LB_PHY_NEAR:
        ret |= vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= vsc8512_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        break;

    /* Added by liuht for bug 27508, 2014-06-30 */
    case GLB_LB_PHY_NEAR_DIAG:
        ret |= vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= vsc8512_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        ret |= vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
        break;
    /* End of liuht added */

    default:
        return RESULT_ERROR;
        break;
    }

    return ret;
}

int32
vsc8512_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config medium mac_if %d media_if %d", pphy_info->phy_addr, mac_if, media_if);
    ret = vsc8512_phy_reset(phy_handle, mac_if, media_if, 0);
    /*After medium config, speed/duplex/loopback need re-config. */
    ret |= vsc8512_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex);
    if(GLB_LB_NONE != pphy_info->phy_manage_info.lb_mode)
    {
        ret |= vsc8512_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }
    /*Fix bug30023. After medium config, enable need re-config*/
    if(0 == pphy_info->phy_manage_info.enable)
    {
        ret |= vsc8512_enable(phy_handle, 0);
    }
    return ret;
}

int32
vsc8512_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
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
        "VSC8512 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += vsc8512_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}
/*Bug25277, support VCT.*/
int32 vsc8512_set_vct_mod(phy_handle_t* phy_handle)
{
    int32 ret = 0;

    /*fix bug40964, Testing vct will lead to hsrvd crash, liangf, 2016-09-29*/
    if(NULL == phy_handle || (!phy_handle->phy_info.vct_support))
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8512_set_vct_mod. ");
   
    ret = vtss8xx8_veriphy_task_start(phy_handle, 0);
    
    return ret;
}
/*Bug25277, support VCT.*/
int32 vsc8512_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
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
        vsc8512_enable(phy_handle, phy_handle->phy_info.phy_manage_info.enable);
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

/* support eee function, modified by liuht for bug 28298, 2014-05-16 */
int32 vsc8512_enable_eee(phy_handle_t* phy_handle, int8 enable)
{
    int32 ret = RESULT_OK;
    uint16 val;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8512 phy addr %d config eee enable %d, orig %d", pphy_info->phy_addr, enable, pphy_info->phy_manage_info.eee_enable);

    vsc8512_phy_page_gpio(phy_handle);

    if (GLB_EEE_ENABLE == enable)
    {
        /* patch */
        vsc8512_read(phy_handle, 18, &val);
        if(val & 0x4000)
        {
            vsc8512_write(phy_handle, 18, 0x8009);
        }
        vsc8512_wait_for_micro_complete(phy_handle);
        ret += vsc8512_phy_page_std(phy_handle);
        /* enable 10BASE-Te EEE capability */
        ret += vsc8512_phy_page_ext2(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 17, 0x8000, 0x8000);
        ret += vsc8512_phy_page_std(phy_handle);
        /* enable 100BASE-TX and 1000BASE-T EEE capability */
        ret += vsc8512_mmd_reg_wr_mask(phy_handle, 7, 60, 0x6, 0x6); // Enable advertisement
        /* test page */
        ret += vsc8512_phy_page_test(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 25, 0, 1);
    }
    else
    {
        /* patch */
        vsc8512_read(phy_handle, 18, &val);
        if(val & 0x4000)
        {
            vsc8512_write(phy_handle, 18, 0x800f);
        }
        vsc8512_wait_for_micro_complete(phy_handle);
        ret += vsc8512_phy_page_std(phy_handle);
        /* disable 10BASE-Te EEE capability */
        ret += vsc8512_phy_page_ext2(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 17, 0, 0x8000);
        ret += vsc8512_phy_page_std(phy_handle);
        /* disable 100BASE-TX and 1000BASE-T EEE capability */
        ret += vsc8512_mmd_reg_wr_mask(phy_handle, 7, 60, 0x0, 0x6); // Disable advertisement
    }

    ret += vsc8512_phy_page_std(phy_handle);
    ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
    return ret;
}
/* end of liuht modified */

int32 vsc8512_config_phy_sig_test_mode(phy_handle_t* phy_handle, uint8 serdes_id, uint8 mode)
{
    uint16 value;

    if(mode == PHY_SIG_TEST_MODE_NORMAL)
    {
        vsc8512_phy_page_ext2(phy_handle);
        vsc8512_write_mask(phy_handle, 17, 0x0000, 0x0020);
        vsc8512_phy_page_std(phy_handle);
        vsc8512_write_mask(phy_handle, 9, 0x0000, 0xf800);
    }
    else
    {
        /*set 17E2 bit 5=1*/
        vsc8512_phy_page_ext2(phy_handle);
        vsc8512_write_mask(phy_handle, 17, 0x0020, 0x0020);
        vsc8512_phy_page_std(phy_handle);
        /*set 0 bit 6, 13 = 10*/
        vsc8512_write_mask(phy_handle, 17, 0x0040, 0x2040);
        /*set 9 bit 15-13, select mode*/        
        if(mode == PHY_SIG_TEST_MODE_1)
            value = 0x2000;
        else if(mode == PHY_SIG_TEST_MODE_2)
            value = 0x4000;
        else if(mode == PHY_SIG_TEST_MODE_3)
            value = 0x6000;
        else 
            value = 0;
        vsc8512_write_mask(phy_handle, 9, value, 0xe000);
    }
    return RESULT_OK;
}

int32
vsc8512_enable_syncE(phy_handle_t* phy_handle, int8 enable) 
{
    phy_info_t* pphy_info;
    int32 ret = 0;
    uint16 value = 0;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "phy addr %d syncE enable %d", pphy_info->phy_addr, enable);     
    if(enable)
    {
       /*23G, 0x17, bit15 enable/disable; bit14:11 source phy; bit5:4  10, bit2:0   001*/
        value |= (pphy_info->phy_addr%12) <<11;
       // value |= 0x8021; 
        value |= 0x8001; 
        vsc8512_phy_page_gpio(phy_handle);
        ret += vsc8512_write(phy_handle, 0x17, value); 
        vsc8512_phy_page_std(phy_handle);
    }
    else    
    {
        vsc8512_phy_page_gpio(phy_handle);
        /*23G, 0x17, bit15 enable/disable; bit14:11 source phy */
        ret += vsc8512_write_mask(phy_handle, 0x17, 0x0, 0xf800);
        vsc8512_phy_page_std(phy_handle);
    }
    return ret;
}

int32 vsc8512_init(phy_handle_t* phy_handle)
{
    int32 ret = 0;
    phy_info_t* pphy_info;
    phyreg_param_t phyreg_para;
    uint16 phy_revision;
    uint16 value;
    
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VSC8512 Init phy addr %d", pphy_info->phy_addr);

    /* First to get VSC8512 PHY revision */
    ret += vsc8512_get_chip_revision(phy_handle, &phy_revision);
    pphy_info->phy_revision = phy_revision;

    /* Operate on the first phy port per chip */
    if (GLB_PRE_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag)
    {
        ret += vsc8512_phy_pre_reset(phy_handle);
    }
    /* set mode and reset */
    ret += vsc8512_phy_reset(phy_handle, pphy_info->phy_manage_info.mac_if, pphy_info->phy_manage_info.media_type, 1);
    /* set speed/duplex */
    ret += vsc8512_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex);
    
    /* Operate on the last phy port per chip */
    if (GLB_POST_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag)
    {
        ret += vsc8512_phy_post_reset(phy_handle);
    }

    /* No effect on the start of packet */
    ret += vsc8512_phy_page_ext3(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 0x10, 0x0000, 0x0700);
    ret += vsc8512_phy_page_std(phy_handle);

    /* Enable PHY MAC serdes Auto-Negotiation */
    ret += vsc8512_phy_page_ext3(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 0x10, 0x0080, 0x0080);
    ret += vsc8512_phy_page_std(phy_handle);

    /*enable speed downshift when 1000M autonegation succ, but link fail*/
    ret += vsc8512_phy_page_ext1(phy_handle);
    ret += vsc8512_write_mask(phy_handle, 0x14, 0x8010, 0x8010);
    ret += vsc8512_phy_page_std(phy_handle);

    /* config signal detect for combo port */
    if(phy_handle->phy_info.gpio_init)
    {
        ret += vsc8512_phy_page_ext1(phy_handle);
        ret += vsc8512_write_mask(phy_handle, 0x13, phy_handle->phy_info.gpio_info.sigdet_polarity, 0x0001);
        ret += vsc8512_phy_page_std(phy_handle);
    }

    /*fix bug36076, support SG8348_48T46X, liangf, 2015-11-28*/
    if(GLB_PHY_LED_5 == pphy_info->led_type)
    {
        ret += vsc8512_write(phy_handle, PHY_LED_MODE, 0xdddd);
    }  
    else if(GLB_PHY_LED_8 == pphy_info->led_type)
    {
        ret += vsc8512_write(phy_handle, PHY_LED_MODE, 0x8061);
    } 
    else if(GLB_PHY_LED_9 == pphy_info->led_type)
    {
        ret += vsc8512_write(phy_handle, PHY_LED_MODE, 0x806e);
    }
    /* for board hf */
    else if(GLB_PHY_LED_12 == pphy_info->led_type)
    {
        ret += vsc8512_write(phy_handle, PHY_LED_MODE, 0x8016);
    }     
    /*fix bug40588, SG8348C port use one led, liangf, 2016-09-02*/
    else if((GLB_PHY_LED_6 == pphy_info->led_type)  || (0xFF == pphy_info->led_type))
    {
        /*fix bug40726, SG8348C port range 1 to 8 use led1, the rest port use led0*/
        ret += vsc8512_write(phy_handle, PHY_LED_MODE, 0xEE00);
    }
   
    /* added by tongzb for bug 42160, up the frequency of led 2016-10-16 */
    /* 2.5hz(0x000), 5hz(0x400,default), 10hz(0x800), 20hz(0xc00)*/
    /* added by tongzb, for config led freq, 2017-01-06 */
    if(GLB_PHY_LED_FREQ_DEFAULT == pphy_info->led_freq)
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_2P5 == pphy_info->led_freq)
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x000, 0xc00);
    else if(GLB_PHY_LED_FREQ_5 == pphy_info->led_freq)
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_10 == pphy_info->led_freq)
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x800, 0xc00);
    else if(GLB_PHY_LED_FREQ_20 == pphy_info->led_freq)
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0xc00, 0xc00);
    else
        ret += vsc8512_write_mask(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    
    /*Clear interrupts*/
    ret += vsc8512_read(phy_handle, PHY_INT_STATUS, &value);
    /* open interrupt detect, open fast link interrupt for bug 38162,guhg,2016-04-27 */
    ret += vsc8512_write(phy_handle, PHY_INT_MASK, 0xf080);
    /* Jqiu added. init to power down*/
    ret += vsc8512_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);

    return ret;
}

phy_handle_t* phy_vsc8512_dev_register(phy_info_t* pphy_info)
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
    /*Bug25277, support VCT.*/
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
    phdl->phy_init = vsc8512_init;
    phdl->config_enable = vsc8512_enable;
    phdl->config_speed = vsc8512_config_speed;
    phdl->config_duplex = vsc8512_config_duplex;
    phdl->config_master_slave = vsc8512_config_master_slave;
    phdl->config_loopback = vsc8512_config_loopback;
    phdl->config_medium = vsc8512_config_medium;
    phdl->config_flowctrl = vsc8512_config_flowctrl;
    phdl->get_link_poll = vsc8512_get_link_poll;
    phdl->get_link_interupt = vsc8512_get_link_interrput;
    phdl->get_cur_status = vsc8512_get_cur_status; /* modified by liuht for bug 26641, 2014-0422 */
    phdl->reg_read = vsc8512_reg_read;
    phdl->reg_write = vsc8512_reg_write;
    phdl->phy_set_vct_mod = vsc8512_set_vct_mod;
    phdl->phy_get_vct_info = vsc8512_get_vct_info;
    /* support eee function, modified by liuht for bug 28298, 2014-04-21 */
    phdl->mmd_reg_write_mask = vsc8512_mmd_reg_wr_mask;
    phdl->enable_eee = vsc8512_enable_eee;
    phdl->enable_syncE = vsc8512_enable_syncE;
    /* end of liuht modified */
    phdl->config_phy_sig_test_mode = vsc8512_config_phy_sig_test_mode;
    
    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type  = pphy_info->mdio_bus;
    /* set slot_card_idx for each phy_addr, and send to MDIO driver */
    mdio_info.slot_card_idx = pphy_info->phy_manage_info.slot_card_idx;
    p_mdio_hdl = mdio_create_handle(&mdio_info);
    
    if(NULL == p_mdio_hdl)
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


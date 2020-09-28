/****************************************************************************
* $Id$
* phy VSC8504 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Jianfeng Qiu
* Date          : 2013-07-26
* Reason        : First Create.
* 2014-10-28    Update to VitesseAPI 4_60
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_vsc8504.h"
#include "phy_veriphy.h"

#define VTSS_RC(expr) { int32 __rc__ = (expr); if (__rc__ < 0) return __rc__; }
#define VTSS_RC_OK 0
#define VTSS_RC_ERROR -1
// Tesla family
#define VTSS_PHY_TESLA_REV_A 0
#define VTSS_PHY_TESLA_REV_B 1
#define VTSS_PHY_TESLA_REV_D 2

/* The address at where the first byte of the internal 8051 firmware is placed. */
#define FIRMWARE_START_ADDR 0x4000

/* PHY register pages */
#define VTSS_PHY_PAGE_STANDARD 0x0000 /* Standard registers */
#define VTSS_PHY_PAGE_EXTENDED 0x0001 /* Extended registers */
#define VTSS_PHY_PAGE_EXTENDED_2 0x0002 /* Extended registers */
#define VTSS_PHY_PAGE_EXTENDED_3 0x0003 /* Extended registers */
#define VTSS_PHY_PAGE_GPIO     0x0010 /* GPIO registers */
#define VTSS_PHY_PAGE_TEST     0x2A30 /* Test registers */
#define VTSS_PHY_PAGE_TR       0x52B5 /* Token ring registers */

/* PHY register page access for a single register can be done 
   using an OR operation of the register address and the page below */
#define VTSS_PHY_REG_STANDARD (VTSS_PHY_PAGE_STANDARD<<5)
#define VTSS_PHY_REG_EXTENDED (VTSS_PHY_PAGE_EXTENDED<<5)
#define VTSS_PHY_REG_GPIO     (VTSS_PHY_PAGE_GPIO<<5)
#define VTSS_PHY_REG_TEST     (VTSS_PHY_PAGE_TEST<<5)
#define VTSS_PHY_REG_TR       (VTSS_PHY_PAGE_TR<<5)

/* Register 14G + Bit fields */
#define VTSS_PHY_GPIO_CONTROL_2 14
#define VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE  (1U << 13)
#define VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_DATA    (1U << 12)


static int32 
vsc8504_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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

    return RESULT_OK;
}


static int32 
vsc8504_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
{
    phy_info_t *pphy_info;
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == phy_handle || NULL == phy_handle->mdio_hdl)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
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
vsc8504_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
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

static int32 vsc8504_read(phy_handle_t* phy_handle, uint16 reg, uint16 *value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}
static int32 vsc8504_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}


int32 vsc8504_phy_rd(phy_handle_t* phy_handle, uint8 reg, uint16* p_value)
{
    int32 ret;
    ret = vsc8504_read(phy_handle, reg, p_value);
    return ret;
}

int32 vsc8504_phy_wr(phy_handle_t* phy_handle, uint8 reg, uint16 value)
{
    int32 ret;
    ret = vsc8504_write(phy_handle, reg, value);
    return ret;    
}

int32 vsc8504_phy_wr_masked(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
{
    uint16 val;
    int32 ret = 0;
    VTSS_RC(vsc8504_phy_rd(phy_handle, reg, &val));
    value = (value & mask) | (val & ~mask);
    VTSS_RC(vsc8504_phy_wr(phy_handle, reg, value));
    return ret;
}

int32 vsc8504_phy_page_std(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_STANDARD);
}

int32 vsc8504_phy_page_ext(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_EXTENDED);
}

int32 vsc8504_phy_page_ext2(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_EXTENDED_2);
}

int32 vsc8504_phy_page_ext3(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_EXTENDED_3);
}

int32 vsc8504_phy_page_gpio(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_GPIO);
}

int32 vsc8504_phy_page_test(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_TEST);
}

int32 vsc8504_phy_page_tr(phy_handle_t* phy_handle)
{
    return vsc8504_phy_wr(phy_handle, 31, VTSS_PHY_PAGE_TR);
}


/*****************phy type detect start************************/
/*check phy is vsc8504.*/
int32 vsc8504_phy_detect(phy_handle_t* phy_handle, uint16* revision)
{
    uint16 reg2, reg3, model;
    uint32 oui;
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    VTSS_RC(vsc8504_phy_rd(phy_handle, 2, &reg2));
    VTSS_RC(vsc8504_phy_rd(phy_handle, 3, &reg3));
    oui = ((reg2 << 6) | ((reg3 >> 10) & 0x3F));
    model = ((reg3 >> 4) & 0x3F);
    *revision = (reg3 & 0xF);
    if ((oui == 0x0001C1) &&(model == 0xc))
    {
        return VTSS_RC_OK;
    }
    else
    {
        return -1;
    }
}
/*****************phy type detect end************************/

/*****************phy pre reset start************************/
// Function that wait for the micro code to complete 
// In : port_no : port number staring from 0.
int32 vsc8504_phy_wait_for_micro_complete(phy_handle_t* phy_handle) 
{
    uint16 timeout = 500;    
    uint16 reg18g = 0;
    int32 rc = VTSS_RC_OK;

    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));

    // Wait for micro to complete MCB command (bit 15)
    VTSS_RC(vsc8504_phy_rd(phy_handle, 18, &reg18g));
    while (reg18g & 0x8000 && timeout > 0)
    {
        VTSS_RC(vsc8504_phy_rd(phy_handle, 18, &reg18g));
        timeout--; // Make sure that we don't run forever 
        usleep(1000);
    }
    
    if (timeout == 0) {
        rc = VTSS_RC_ERROR;
    }

    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    return rc;
}

int32 vsc8504_phy_micro_assert_reset(phy_handle_t* phy_handle)
{
    // Bugzilla#5731 - The following code-sequence is needed for Luton26 Rev. A and B where
    // a consumptive patch disables interrupts inside a micro-command patch.

    // Set to micro/GPIO-page
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));

    //----------------------------------------------------------------------
    // Pass the NOP cmd to Micro to insure that any consumptive patch exits
    // There is no issue with doing this on any revision since it is just a NOP on any Vitesse PHY.
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x800f));

    // Poll on 18G.15 to clear
    VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle));
    //----------------------------------------------------------------------

    // Set to micro/GPIO-page (Has been set to std page by vsc8504_phy_wait_for_micro_complete
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));

    // force micro into a loop, preventing any SMI accesses
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 12, 0x0000, 0x0800)); // Disable patch vector 3 (just in case)
    VTSS_RC(vsc8504_phy_wr(phy_handle,  9, 0x005b));     // Setup patch vector 3 to trap MicroWake interrupt
    VTSS_RC(vsc8504_phy_wr(phy_handle, 10, 0x005b));     // Loop forever on MicroWake interrupts
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 12, 0x0800, 0x0800)); // Enable patch vector 3
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x800f));     // Trigger MicroWake interrupt to make safe to reset

    // Assert reset after micro is trapped in a loop (averts micro-SMI access deadlock at reset)
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 0, 0x0000, 0x8000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));     // Make sure no MicroWake persists after reset
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 12, 0x0000, 0x0800)); // Disable patch vector 3

    return vsc8504_phy_page_std(phy_handle);
}
// Function for downloading code into the internal 8051 CPU.
// In : phy_handle - Any port within the chip where to download the 8051 code
//      code_array - Pointer to array containing the 8051 code
//      code_size  - The size of the code to be downloaded.
// Return : VTSS_RC_OK if download was done else error code. 
int32 atom_download_8051_code(phy_handle_t* phy_handle, uint8 const *code_array, uint16 code_size) 
{
    uint16 i;

    // Note that the micro/GPIO-page, Reg31=0x10, is a global page, one per PHY chip
    // thus even though broadcast is turned off, it is still sufficient to do once`

    // Hold the micro in reset during patch download
    VTSS_RC(vsc8504_phy_micro_assert_reset(phy_handle));
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 0, 0x7009));      //  Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock

    VTSS_RC(vsc8504_phy_wr(phy_handle, 12, 0x5002));     // write to addr 4000= 02
    VTSS_RC(vsc8504_phy_wr(phy_handle, 11, 0x0000));     // write to address reg.

    for(i = 0; i < code_size; i++)
    {
        VTSS_RC(vsc8504_phy_wr(phy_handle, 12, 0x5000 | code_array[i]));   
    }

    VTSS_RC(vsc8504_phy_wr(phy_handle, 12, 0x0000));     // Clear internal memory access

    return VTSS_RC_OK;
}

//Function for checking that the 8051 code is loaded correctly.

//In : port_no : port number staring from 0. 
//     code_length : The length of the downloaded 8051 code 
//     expected_crc: The expected CRC for the downloaded 78051 code
int32 vsc8504_phy_is_8051_crc_ok_private (phy_handle_t* phy_handle, uint16 start_addr, uint16 code_length, uint16 expected_crc) 
{
    uint16 crc_calculated = 0;
    
    VTSS_RC(vsc8504_phy_page_ext(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 25, start_addr));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 26, code_length)); 
    
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle)); // Change to GPIO page
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8008)); // Start Mirco command
    
    //MISSING VTSS_RC() is not an omission, it is intended so we get a CRC error
    (void) vsc8504_phy_wait_for_micro_complete(phy_handle);
    
    // Get the CRC 
    VTSS_RC(vsc8504_phy_page_ext(phy_handle));
    VTSS_RC(vsc8504_phy_rd(phy_handle, 25, &crc_calculated));
    
    VTSS_RC(vsc8504_phy_page_std(phy_handle)); // return to standard page
    
    
    if (crc_calculated == expected_crc) 
    {
        return VTSS_RC_OK;
    } 
    else 
    {
        return VTSS_RC_ERROR;
    }
}


int32 tesla_revA_8051_patch_9_27_2011(phy_handle_t* phy_handle) 
{
    static const uint8 patch_arr[] = {
        0x44, 0xdd, 0x02, 0x41, 0xff, 0x02, 0x42, 0xc3, 0x02, 0x45,
        0x06, 0x02, 0x45, 0x07, 0x02, 0x45, 0x08, 0x8f, 0x27, 0x7b,
        0xbb, 0x7d, 0x0e, 0x7f, 0x04, 0x12, 0x3c, 0x20, 0xef, 0x4e,
        0x60, 0x03, 0x02, 0x41, 0xe6, 0xe4, 0xf5, 0x19, 0x74, 0x01,
        0x7e, 0x00, 0xa8, 0x19, 0x08, 0x80, 0x05, 0xc3, 0x33, 0xce,
        0x33, 0xce, 0xd8, 0xf9, 0xff, 0xef, 0x55, 0x27, 0x70, 0x03,
        0x02, 0x41, 0xda, 0x85, 0x19, 0xfb, 0x7b, 0xbb, 0xe4, 0xfd,
        0xff, 0x12, 0x3c, 0x20, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41,
        0xda, 0xe5, 0x19, 0x54, 0x02, 0x75, 0x55, 0x00, 0x25, 0xe0,
        0x25, 0xe0, 0xf5, 0x54, 0xe4, 0x78, 0x93, 0xf6, 0xd2, 0x02,
        0x12, 0x41, 0xe7, 0x7b, 0xff, 0x7d, 0x12, 0x7f, 0x07, 0x12,
        0x3c, 0x20, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xd4, 0xc2,
        0x02, 0x74, 0x95, 0x25, 0x19, 0xf9, 0x74, 0xb5, 0x25, 0x19,
        0xf8, 0xe6, 0x27, 0xf5, 0x28, 0xe5, 0x55, 0x24, 0x5b, 0x12,
        0x44, 0x8c, 0x12, 0x3d, 0x48, 0x7b, 0xfc, 0x7d, 0x11, 0x7f,
        0x07, 0x12, 0x3c, 0x20, 0x78, 0x9a, 0xef, 0xf6, 0x78, 0x8f,
        0xe6, 0xfe, 0xef, 0xd3, 0x9e, 0x40, 0x06, 0x78, 0x9a, 0xe6,
        0x78, 0x8f, 0xf6, 0x12, 0x41, 0xe7, 0x7b, 0xec, 0x7d, 0x12,
        0x7f, 0x07, 0x12, 0x3c, 0x20, 0x78, 0x99, 0xef, 0xf6, 0xbf,
        0x07, 0x06, 0x78, 0x91, 0x76, 0x1a, 0x80, 0x1f, 0x78, 0x93,
        0xe6, 0xff, 0x60, 0x0f, 0xc3, 0xe5, 0x28, 0x9f, 0xff, 0x78,
        0x99, 0xe6, 0x85, 0x28, 0xf0, 0xa4, 0x2f, 0x80, 0x07, 0x78,
        0x99, 0xe6, 0x85, 0x28, 0xf0, 0xa4, 0x78, 0x91, 0xf6, 0xe4,
        0x78, 0x90, 0xf6, 0x78, 0x90, 0xe6, 0xff, 0xc3, 0x08, 0x96,
        0x40, 0x03, 0x02, 0x41, 0xbe, 0xef, 0x54, 0x03, 0x60, 0x33,
        0x14, 0x60, 0x46, 0x24, 0xfe, 0x60, 0x42, 0x04, 0x70, 0x4b,
        0xef, 0x24, 0x02, 0xff, 0xe4, 0x33, 0xfe, 0xef, 0x78, 0x02,
        0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0xd8, 0xf8, 0xff, 0xe5,
        0x55, 0x24, 0x5c, 0xcd, 0xe5, 0x54, 0x34, 0xf0, 0xcd, 0x2f,
        0xff, 0xed, 0x3e, 0xfe, 0x12, 0x44, 0xaf, 0x7d, 0x11, 0x80,
        0x0b, 0x78, 0x90, 0xe6, 0x70, 0x04, 0x7d, 0x11, 0x80, 0x02,
        0x7d, 0x12, 0x7f, 0x07, 0x12, 0x3d, 0x08, 0x8e, 0x29, 0x8f,
        0x2a, 0x80, 0x03, 0xe5, 0x29, 0xff, 0x78, 0x93, 0xe6, 0x06,
        0x24, 0x9b, 0xf8, 0xa6, 0x07, 0x78, 0x90, 0x06, 0xe6, 0xb4,
        0x1a, 0x0a, 0xe5, 0x55, 0x24, 0x5c, 0x12, 0x44, 0x8c, 0x12,
        0x3d, 0x48, 0x78, 0x93, 0xe6, 0x65, 0x28, 0x70, 0x82, 0x75,
        0xdb, 0x20, 0x75, 0xdb, 0x28, 0x12, 0x44, 0xa4, 0x12, 0x44,
        0xa4, 0xe5, 0x19, 0x12, 0x44, 0x97, 0xe5, 0x19, 0xc3, 0x13,
        0x12, 0x44, 0x97, 0x78, 0x93, 0x16, 0xe6, 0x24, 0x9b, 0xf8,
        0xe6, 0xff, 0x7e, 0x08, 0x1e, 0xef, 0xa8, 0x06, 0x08, 0x80,
        0x02, 0xc3, 0x13, 0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0,
        0xf5, 0xdb, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02, 0xc3, 0x13,
        0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0x44, 0x08, 0xf5,
        0xdb, 0xee, 0x70, 0xd8, 0x78, 0x93, 0xe6, 0x70, 0xc8, 0x75,
        0xdb, 0x10, 0x02, 0x40, 0xea, 0x78, 0x90, 0xe6, 0xc3, 0x94,
        0x17, 0x50, 0x0e, 0xe5, 0x55, 0x24, 0x62, 0x12, 0x41, 0xf5,
        0xe5, 0x55, 0x24, 0x5c, 0x12, 0x41, 0xf5, 0x20, 0x02, 0x03,
        0x02, 0x40, 0x63, 0x05, 0x19, 0xe5, 0x19, 0xc3, 0x94, 0x04,
        0x50, 0x03, 0x02, 0x40, 0x27, 0x22, 0xe5, 0x55, 0x24, 0x5c,
        0xff, 0xe5, 0x54, 0x34, 0xf0, 0xfe, 0x12, 0x44, 0xaf, 0x22,
        0xff, 0xe5, 0x54, 0x34, 0xf0, 0xfe, 0x12, 0x44, 0xaf, 0x22,
        0xd2, 0x00, 0x75, 0xfb, 0x03, 0xab, 0x59, 0xaa, 0x58, 0x7d,
        0x19, 0x7f, 0x03, 0x12, 0x3d, 0x48, 0xe5, 0x59, 0x54, 0x0f,
        0x24, 0xf1, 0x70, 0x03, 0x02, 0x42, 0xb8, 0x24, 0x02, 0x60,
        0x03, 0x02, 0x42, 0xad, 0x12, 0x44, 0xc8, 0x12, 0x44, 0xcf,
        0xd8, 0xfb, 0xff, 0x20, 0xe2, 0x2a, 0x13, 0x92, 0x04, 0xef,
        0xa2, 0xe1, 0x92, 0x03, 0x30, 0x04, 0x1f, 0xe4, 0xf5, 0x21,
        0xe5, 0x21, 0x24, 0x17, 0xfd, 0x7b, 0x54, 0x7f, 0x04, 0x12,
        0x3c, 0x20, 0x74, 0xbd, 0x25, 0x21, 0xf8, 0xa6, 0x07, 0x05,
        0x21, 0xe5, 0x21, 0xc3, 0x94, 0x02, 0x40, 0xe4, 0x12, 0x44,
        0xc8, 0x12, 0x44, 0xcf, 0xd8, 0xfb, 0x54, 0x05, 0x64, 0x04,
        0x70, 0x27, 0x78, 0x92, 0xe6, 0x78, 0x94, 0xf6, 0xe5, 0x58,
        0xff, 0x33, 0x95, 0xe0, 0xef, 0x54, 0x0f, 0x78, 0x92, 0xf6,
        0x12, 0x43, 0x85, 0x20, 0x04, 0x0c, 0x12, 0x44, 0xc8, 0x12,
        0x44, 0xcf, 0xd8, 0xfb, 0x13, 0x92, 0x05, 0x22, 0xc2, 0x05,
        0x22, 0x12, 0x44, 0xc8, 0x12, 0x44, 0xcf, 0xd8, 0xfb, 0x54,
        0x05, 0x64, 0x05, 0x70, 0x1e, 0x78, 0x92, 0x7d, 0xb8, 0x12,
        0x42, 0xb9, 0x78, 0x8f, 0x7d, 0x74, 0x12, 0x42, 0xb9, 0xe4,
        0x78, 0x8f, 0xf6, 0x22, 0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee,
        0x7f, 0x92, 0x12, 0x37, 0x1c, 0x22, 0xe6, 0xfb, 0x7a, 0x00,
        0x7f, 0x92, 0x12, 0x37, 0x1c, 0x22, 0x30, 0x04, 0x03, 0x12,
        0x44, 0x3d, 0x78, 0x92, 0xe6, 0xff, 0x60, 0x03, 0x12, 0x40,
        0x12, 0xe4, 0xf5, 0x19, 0x12, 0x44, 0xd6, 0x20, 0xe7, 0x03,
        0x02, 0x43, 0x78, 0x85, 0x19, 0xfb, 0x7d, 0x1c, 0xe4, 0xff,
        0x12, 0x3d, 0x08, 0x8e, 0x54, 0x8f, 0x55, 0xe5, 0x54, 0x13,
        0x13, 0x13, 0x54, 0x0c, 0x44, 0x80, 0xf5, 0x1b, 0xe5, 0x54,
        0x54, 0xc0, 0x70, 0x06, 0xe5, 0x55, 0x54, 0x18, 0x60, 0x12,
        0x7e, 0x00, 0xe5, 0x55, 0x54, 0x18, 0x78, 0x03, 0xce, 0xc3,
        0x13, 0xce, 0x13, 0xd8, 0xf9, 0x04, 0x42, 0x1b, 0xe5, 0x1b,
        0x54, 0x03, 0x64, 0x03, 0x70, 0x33, 0x12, 0x44, 0xd6, 0x54,
        0x03, 0x64, 0x03, 0x60, 0x21, 0x75, 0x14, 0xc4, 0x75, 0x15,
        0x06, 0x7d, 0x0a, 0xe4, 0xff, 0x12, 0x3d, 0x08, 0xee, 0x30,
        0xe6, 0x08, 0x43, 0x1b, 0x10, 0x75, 0x16, 0x02, 0x80, 0x2b,
        0x53, 0x1b, 0xef, 0x75, 0x16, 0x03, 0x80, 0x23, 0x12, 0x44,
        0xd6, 0x54, 0x10, 0x42, 0x1b, 0x80, 0x21, 0xe5, 0x1b, 0x54,
        0x03, 0x64, 0x03, 0x60, 0x19, 0x12, 0x44, 0xd6, 0x54, 0x03,
        0xff, 0xbf, 0x03, 0x10, 0x75, 0x14, 0xc4, 0x75, 0x15, 0x06,
        0xe4, 0xf5, 0x16, 0x7f, 0xfe, 0x7e, 0x0e, 0x12, 0x3c, 0x49,
        0x74, 0xb9, 0x25, 0x19, 0xf8, 0xa6, 0x1b, 0x05, 0x19, 0xe5,
        0x19, 0xc3, 0x94, 0x04, 0x50, 0x03, 0x02, 0x42, 0xd5, 0x22,
        0x78, 0x8f, 0xe6, 0xfb, 0x7a, 0x00, 0x7d, 0x74, 0x7f, 0x92,
        0x12, 0x37, 0x1c, 0xe4, 0x78, 0x8f, 0xf6, 0xf5, 0x22, 0x74,
        0x01, 0x7e, 0x00, 0xa8, 0x22, 0x08, 0x80, 0x05, 0xc3, 0x33,
        0xce, 0x33, 0xce, 0xd8, 0xf9, 0xff, 0x78, 0x92, 0xe6, 0xfd,
        0xef, 0x5d, 0x60, 0x44, 0x85, 0x22, 0xfb, 0xe5, 0x22, 0x54,
        0x02, 0x25, 0xe0, 0x25, 0xe0, 0xfe, 0xe4, 0x24, 0x5b, 0xfb,
        0xee, 0x12, 0x44, 0x8f, 0x12, 0x3d, 0x48, 0x7b, 0x40, 0x7d,
        0x11, 0x7f, 0x07, 0x12, 0x3c, 0x20, 0x74, 0x95, 0x25, 0x22,
        0xf8, 0xa6, 0x07, 0x7b, 0x11, 0x7d, 0x12, 0x7f, 0x07, 0x12,
        0x3c, 0x20, 0xef, 0x4e, 0x60, 0x09, 0x74, 0xb5, 0x25, 0x22,
        0xf8, 0x76, 0x04, 0x80, 0x07, 0x74, 0xb5, 0x25, 0x22, 0xf8,
        0x76, 0x0a, 0x05, 0x22, 0xe5, 0x22, 0xc3, 0x94, 0x04, 0x40,
        0x9a, 0x78, 0x94, 0xe6, 0x70, 0x15, 0x78, 0x92, 0xe6, 0x60,
        0x10, 0x75, 0xd9, 0x38, 0x75, 0xdb, 0x10, 0x7d, 0xfe, 0x12,
        0x44, 0x33, 0x7d, 0x76, 0x12, 0x44, 0x33, 0x79, 0x94, 0xe7,
        0x78, 0x92, 0x66, 0xff, 0x60, 0x03, 0x12, 0x40, 0x12, 0x78,
        0x92, 0xe6, 0x70, 0x09, 0xfb, 0xfa, 0x7d, 0xfe, 0x7f, 0x8e,
        0x12, 0x37, 0x1c, 0x22, 0x7b, 0x01, 0x7a, 0x00, 0x7f, 0x8e,
        0x12, 0x37, 0x1c, 0x22, 0xe4, 0xf5, 0x4b, 0x74, 0xbd, 0x25,
        0x4b, 0xf8, 0xe6, 0x64, 0x03, 0x60, 0x38, 0xe5, 0x4b, 0x24,
        0x17, 0xfd, 0x7b, 0xeb, 0x7f, 0x04, 0x12, 0x3c, 0x20, 0x8f,
        0x19, 0x85, 0x19, 0xfb, 0x7b, 0x22, 0x7d, 0x18, 0x7f, 0x06,
        0x12, 0x3c, 0x20, 0xef, 0x64, 0x01, 0x4e, 0x70, 0x07, 0x12,
        0x44, 0xed, 0x7b, 0x03, 0x80, 0x0a, 0x12, 0x44, 0xed, 0x74,
        0xbd, 0x25, 0x4b, 0xf8, 0xe6, 0xfb, 0x7a, 0x00, 0x7d, 0x54,
        0x12, 0x37, 0x1c, 0x05, 0x4b, 0xe5, 0x4b, 0xc3, 0x94, 0x02,
        0x40, 0xb5, 0x22, 0xfb, 0xe5, 0x54, 0x34, 0xf0, 0xfa, 0x7d,
        0x10, 0x7f, 0x07, 0x22, 0x54, 0x01, 0xc4, 0x33, 0x54, 0xe0,
        0xf5, 0xdb, 0x44, 0x08, 0xf5, 0xdb, 0x22, 0xf5, 0xdb, 0x75,
        0xdb, 0x08, 0xf5, 0xdb, 0x75, 0xdb, 0x08, 0x22, 0xab, 0x07,
        0xaa, 0x06, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3d, 0x48, 0x7b,
        0xff, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3c, 0x20, 0xef, 0x4e,
        0x60, 0xf3, 0x22, 0xe5, 0x59, 0xae, 0x58, 0x78, 0x04, 0x22,
        0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0x22, 0x74, 0xb9, 0x25,
        0x19, 0xf8, 0xe6, 0x22, 0x7f, 0x04, 0x78, 0xb9, 0x74, 0x88,
        0xf6, 0x08, 0xdf, 0xfc, 0x12, 0x45, 0x01, 0x02, 0x44, 0xf7,
        0xe5, 0x4b, 0x24, 0x17, 0x54, 0x1f, 0x44, 0x80, 0xff, 0x22,
        0xe4, 0x78, 0x92, 0xf6, 0xc2, 0x05, 0x78, 0x8f, 0xf6, 0x22,
        0xc2, 0x04, 0xc2, 0x03, 0x22, 0x22, 0x22, 0x22
    };
    VTSS_RC(atom_download_8051_code(phy_handle, &patch_arr[0], sizeof(patch_arr)));

    VTSS_RC(vsc8504_phy_wr(phy_handle,  0, 0x4098));     // Enable 8051 clock; indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    VTSS_RC(vsc8504_phy_wr(phy_handle,  0, 0xc098));     // Release 8051 SW Reset

    // Check that code is downloaded correctly.
    VTSS_RC(vsc8504_phy_is_8051_crc_ok_private(phy_handle,
                                            FIRMWARE_START_ADDR,
                                            sizeof(patch_arr) + 1 /* Add one for the byte auto-added in the download function */, 
                                            0x4EE4));

    return VTSS_RC_OK;
}

int32 vsc8504_phy_pre_init_seq_tesla_4_april_2011 (phy_handle_t* phy_handle) 
{
    // MII register writes and test-page register writes go here
    //using broadcast flag to speed things up
    VTSS_RC(vsc8504_phy_page_std(phy_handle)); //Switch to main register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,22,0x0001,0x0001)); // turn on broadcast writes

    // Set 100BASE-TX edge rate to optimal setting
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,24,0x2000,0xe000));

    // Set 100BASE-TX amplitude to optimal setting after MDI-cal tweak
    VTSS_RC(vsc8504_phy_page_ext2(phy_handle)); //Switch to extended register page 2
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,16,0x0200,0x0f00));

    // Set MDI Cal offset 100 to 0
    // Set MDI Cal offset 1000 to 0
    VTSS_RC(vsc8504_phy_page_test(phy_handle)); //Switch to test register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 20,0x0000,0x6030));

    //Switch to hardware energy-detector in 1000BASE-T mode
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 9,0x0000,0x4000));

    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 8,0x8000,0x8000)); // Enable token-ring during coma-mode

    VTSS_RC(vsc8504_phy_page_tr(phy_handle)); // Switch to token-ring register page

    //TrWrite( base_phy+1, 'eee_TrKp1Short_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Long_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Short_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp3Long_1000', -1, 1);
    //TrWrite( base_phy+1, 'eee_TrKp3Short_1000', -1, 1);
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //eee_Tr... Kp1Long,Kp1Short ..._1000
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0011 )); //eee_Tr... Kp2Long,Kp2Short,Kp3Long,Kp3Short ..._1000
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a0 )); //Write eee_TrKp*_1000

    //TrWrite( base_phy+1, 'ph_shift_num1000_eee1', -1, 1); //Double-step when Kp1 selected
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //(no fields)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7100 )); //eee_TrKf1000,ph_shift_num1000_... eee1,eee2,std
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a2 )); //Write eee_TrKf1000,ph_shift_num1000_*

    //TrWrite( base_phy+1, 'SSTrKf1000Slv',-1, 7); //Disable frequency-offset updates at showtime
    //TrWrite( base_phy+1, 'SSTrKp100',-1, 13); //Speed up steady-state to 2*gain of 1 - UNH setting
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00d2 )); //SSTrKp100,SSTrKf100
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x547f )); //SSTrKp1000Mas,SSTrKp1000Slv,SSTrKf1000Slv,Ss[EN]cUpdGain1000
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x968c )); //Write SSTrK*,SS[EN]cUpdGain1000

    //TrWrite( base_phy+1, 'eee_TrKp1Short_100',-1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Long_100',-1, 15);
    //TrWrite( base_phy+1, 'eee_TrKp2Short_100',-1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp3Short_100',-1, 13);
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00f0 )); //eee_Tr... Kp1Long,Kp1Short ..._100
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf00d )); //eee_Tr... Kp2Long,Kp2Short,Kp3Long,Kp3Short ..._100
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b0 )); //Write eee_TrKp*_100

    //TrWrite( base_phy+1, 'ph_shift_num100_eee1', -1, 6); //Quad-step when Kp1 selected
    //TrWrite( base_phy+1, 'ph_shift_num100_eee2', -1, 2); //Double-step when Kp2 selected
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //(no fields)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7100 )); //eee_TrKf100,ph_shift_num100_... eee1,eee2,std
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b2 )); //Write eee_TrKf100,ph_shift_num100_*

    //TrWrite( base_phy+1, 'lpi_tr_tmr_val1',-1, '34'); //lengthen tmr1 by 1/2 us
    //TrWrite( base_phy+1, 'lpi_tr_tmr_val2',-1, '5f'); //lengthen tmr2 by 1/2 us
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //(no fields)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x345f )); //lpi_tr_tmr_... val1,val2
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b4 )); //Write lpi_tr_tmr_val*

    //TrWrite( base_phy+1, 'nonzero_det_thr1000',-1, 15); // Increase from 8 to reduce noise
    //TrWrite( base_phy+1, 'zero_det_thr_nzd1000',-1, 31); // Increase from 15 to handle 1000BT scrambler length in case 31 zeroes OK
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //nonzero_det_thr1000[5:4]
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf7df )); //nonzero_det_thr1000[3:0],zero_det_thr_... nzd1000,zd1000
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fd4 )); //Write non/zero_det_thr*1000

    //TrWrite( base_phy+1, 'nonzero_det_thr100',-1, 15); // Increase from 8 to reduce noise
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //nonzero_det_thr100[5:4]
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf3df )); //nonzero_det_thr100[3:0],zero_det_thr_... nzd100,zd100
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fd2 )); //Write non/zero_det_thr*100

    //TrWrite( base_phy+1, 'zero_detect_thresh_zd',-1, 31); // Detect link-partner going quiet more quickly
    //TrWrite( base_phy+1, 'rem_lpi_req_thresh',-1, 15);
    //TrWrite( base_phy+1, 'rem_upd_done_thresh',-1, 15); // Speed up remote update-done detection faster than zero-detection
    //31, 15, and 15 are the default, thus no write

    //TrWrite( base_phy+1, 'LongVgaThresh100',-1, 21); //same as -11 -- UNH setting
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x000e )); //DSPreadyTime100[7:2]
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2b00 )); //DSPreadyTime100[1:0],LongVgaThresh100,EnabRandUpdTrig,CMAforces(8b)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fb0 )); //Write DSPreadyTime100,LongVgaThresh100,EnabRandUpdTrig,CMAforces

    //10BT power level optimization for 10BT MAU

    //TrWrite( base_phy+1, 'ld10_pwrlvl_actiphy', -1, 1);
    //TrWrite( base_phy+1, 'ld10_pwrlvl_10tx', -1, 1);
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x000b )); //SwitchToLD10,PwrUpBothDAC_LD10/G,dac10_keepalive_en
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x05a0 )); //ld10_pwrlvl_... act_ct,g_ct,actiphy,10tx,10rx,100rx,10idle,g
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe0 )); //Write SwitchToLD10,PwrUpBoth*,dac10_keepalive_en,ld10_pwrlvl_*

    //TrWrite( base_phy+1, 'ld10_edge_ctrl1', -1, 3);
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //(no fields)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x00ba )); //ld10_edge_ctrl... 0,1,2,3
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe2 )); //Write ld10_edge_ctrl*

    //TrWrite( base_phy+1, 'VgaGain10',-1, 4);
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000 )); //EnabIndpDfe/Dc/XcUpdTrig,SSFfeDecCntrl,SSEnabFfeUpdTapSel,VgaGain10[4]
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x4689 )); //VgaGain10[3:0],SSVgaSlowDecRate,SSVgaWindow1000/100,SSVgaSerialize
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f92 )); //Write register containing VgaGain10

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0060));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0980));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f90));

    //Note that when Tesla is configured for 10BASE-T (not Te), it needs no 10BASE-T configuration as its pulse codes already match
#ifdef VTSS_10BASE_TE
    VTSS_RC(vsc8504_phy_65nm_10BASE_init(phy_handle, 0));
#endif

    VTSS_RC(vsc8504_phy_page_test(phy_handle)); //Switch to test register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 8,0x0000,0x8000)); // Disable token-ring after complete


    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 22, 0x0000, 0x0001)); // Turn off broadcast writes
    
    VTSS_RC(tesla_revA_8051_patch_9_27_2011(phy_handle)); //Load micro patch
    return VTSS_RC_OK;
}

// Tesla  Rev. B. Internal 8051 patch.
// In     : port_no - any port within the chip where to load the 8051 code.
// Return : VTSS_RC_OK if configuration done else error code. 

// Download patch into PRAM for the internal 8051 
// Date: 3Oct2013
int32 tesla_revB_8051_patch(phy_handle_t* phy_handle) {
    static const uint8 patch_arr[] = {
        0x46, 0x18, 0x02, 0x43, 0x37, 0x02, 0x45, 0xf4, 0x02, 0x46,
        0x45, 0x02, 0x45, 0xbe, 0x02, 0x45, 0x60, 0xed, 0xff, 0xe5,
        0xfc, 0x54, 0x38, 0x64, 0x20, 0x70, 0x08, 0x65, 0xff, 0x70,
        0x04, 0xed, 0x44, 0x80, 0xff, 0x22, 0x8f, 0x19, 0x7b, 0xbb,
        0x7d, 0x0e, 0x7f, 0x04, 0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60,
        0x03, 0x02, 0x41, 0xf9, 0xe4, 0xf5, 0x1a, 0x74, 0x01, 0x7e,
        0x00, 0xa8, 0x1a, 0x08, 0x80, 0x05, 0xc3, 0x33, 0xce, 0x33,
        0xce, 0xd8, 0xf9, 0xff, 0xef, 0x55, 0x19, 0x70, 0x03, 0x02,
        0x41, 0xed, 0x85, 0x1a, 0xfb, 0x7b, 0xbb, 0xe4, 0xfd, 0xff,
        0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xed,
        0xe5, 0x1a, 0x54, 0x02, 0x75, 0x1d, 0x00, 0x25, 0xe0, 0x25,
        0xe0, 0xf5, 0x1c, 0xe4, 0x78, 0xc5, 0xf6, 0xd2, 0x0a, 0x12,
        0x41, 0xfa, 0x7b, 0xff, 0x7d, 0x12, 0x7f, 0x07, 0x12, 0x3d,
        0xd7, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xe7, 0xc2, 0x0a,
        0x74, 0xc7, 0x25, 0x1a, 0xf9, 0x74, 0xe7, 0x25, 0x1a, 0xf8,
        0xe6, 0x27, 0xf5, 0x1b, 0xe5, 0x1d, 0x24, 0x5b, 0x12, 0x45,
        0x9b, 0x12, 0x3e, 0xda, 0x7b, 0xfc, 0x7d, 0x11, 0x7f, 0x07,
        0x12, 0x3d, 0xd7, 0x78, 0xcc, 0xef, 0xf6, 0x78, 0xc1, 0xe6,
        0xfe, 0xef, 0xd3, 0x9e, 0x40, 0x06, 0x78, 0xcc, 0xe6, 0x78,
        0xc1, 0xf6, 0x12, 0x41, 0xfa, 0x7b, 0xec, 0x7d, 0x12, 0x7f,
        0x07, 0x12, 0x3d, 0xd7, 0x78, 0xcb, 0xef, 0xf6, 0xbf, 0x07,
        0x06, 0x78, 0xc3, 0x76, 0x1a, 0x80, 0x1f, 0x78, 0xc5, 0xe6,
        0xff, 0x60, 0x0f, 0xc3, 0xe5, 0x1b, 0x9f, 0xff, 0x78, 0xcb,
        0xe6, 0x85, 0x1b, 0xf0, 0xa4, 0x2f, 0x80, 0x07, 0x78, 0xcb,
        0xe6, 0x85, 0x1b, 0xf0, 0xa4, 0x78, 0xc3, 0xf6, 0xe4, 0x78,
        0xc2, 0xf6, 0x78, 0xc2, 0xe6, 0xff, 0xc3, 0x08, 0x96, 0x40,
        0x03, 0x02, 0x41, 0xd1, 0xef, 0x54, 0x03, 0x60, 0x33, 0x14,
        0x60, 0x46, 0x24, 0xfe, 0x60, 0x42, 0x04, 0x70, 0x4b, 0xef,
        0x24, 0x02, 0xff, 0xe4, 0x33, 0xfe, 0xef, 0x78, 0x02, 0xce,
        0xa2, 0xe7, 0x13, 0xce, 0x13, 0xd8, 0xf8, 0xff, 0xe5, 0x1d,
        0x24, 0x5c, 0xcd, 0xe5, 0x1c, 0x34, 0xf0, 0xcd, 0x2f, 0xff,
        0xed, 0x3e, 0xfe, 0x12, 0x45, 0xdb, 0x7d, 0x11, 0x80, 0x0b,
        0x78, 0xc2, 0xe6, 0x70, 0x04, 0x7d, 0x11, 0x80, 0x02, 0x7d,
        0x12, 0x7f, 0x07, 0x12, 0x3e, 0x9a, 0x8e, 0x1e, 0x8f, 0x1f,
        0x80, 0x03, 0xe5, 0x1e, 0xff, 0x78, 0xc5, 0xe6, 0x06, 0x24,
        0xcd, 0xf8, 0xa6, 0x07, 0x78, 0xc2, 0x06, 0xe6, 0xb4, 0x1a,
        0x0a, 0xe5, 0x1d, 0x24, 0x5c, 0x12, 0x45, 0x9b, 0x12, 0x3e,
        0xda, 0x78, 0xc5, 0xe6, 0x65, 0x1b, 0x70, 0x82, 0x75, 0xdb,
        0x20, 0x75, 0xdb, 0x28, 0x12, 0x45, 0xb3, 0x12, 0x45, 0xb3,
        0xe5, 0x1a, 0x12, 0x45, 0xa6, 0xe5, 0x1a, 0xc3, 0x13, 0x12,
        0x45, 0xa6, 0x78, 0xc5, 0x16, 0xe6, 0x24, 0xcd, 0xf8, 0xe6,
        0xff, 0x7e, 0x08, 0x1e, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02,
        0xc3, 0x13, 0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0xf5,
        0xdb, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02, 0xc3, 0x13, 0xd8,
        0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0x44, 0x08, 0xf5, 0xdb,
        0xee, 0x70, 0xd8, 0x78, 0xc5, 0xe6, 0x70, 0xc8, 0x75, 0xdb,
        0x10, 0x02, 0x40, 0xfd, 0x78, 0xc2, 0xe6, 0xc3, 0x94, 0x17,
        0x50, 0x0e, 0xe5, 0x1d, 0x24, 0x62, 0x12, 0x42, 0x08, 0xe5,
        0x1d, 0x24, 0x5c, 0x12, 0x42, 0x08, 0x20, 0x0a, 0x03, 0x02,
        0x40, 0x76, 0x05, 0x1a, 0xe5, 0x1a, 0xc3, 0x94, 0x04, 0x50,
        0x03, 0x02, 0x40, 0x3a, 0x22, 0xe5, 0x1d, 0x24, 0x5c, 0xff,
        0xe5, 0x1c, 0x34, 0xf0, 0xfe, 0x12, 0x45, 0xdb, 0x22, 0xff,
        0xe5, 0x1c, 0x34, 0xf0, 0xfe, 0x12, 0x45, 0xdb, 0x22, 0xe4,
        0xf5, 0x19, 0x12, 0x46, 0x11, 0x20, 0xe7, 0x1e, 0x7b, 0xfe,
        0x12, 0x42, 0xf9, 0xef, 0xc4, 0x33, 0x33, 0x54, 0xc0, 0xff,
        0xc0, 0x07, 0x7b, 0x54, 0x12, 0x42, 0xf9, 0xd0, 0xe0, 0x4f,
        0xff, 0x74, 0x2a, 0x25, 0x19, 0xf8, 0xa6, 0x07, 0x12, 0x46,
        0x11, 0x20, 0xe7, 0x03, 0x02, 0x42, 0xdf, 0x54, 0x03, 0x64,
        0x03, 0x70, 0x03, 0x02, 0x42, 0xcf, 0x7b, 0xcb, 0x12, 0x43,
        0x2c, 0x8f, 0xfb, 0x7b, 0x30, 0x7d, 0x03, 0xe4, 0xff, 0x12,
        0x3d, 0xd7, 0xc3, 0xef, 0x94, 0x02, 0xee, 0x94, 0x00, 0x50,
        0x2a, 0x12, 0x42, 0xec, 0xef, 0x4e, 0x70, 0x23, 0x12, 0x43,
        0x04, 0x60, 0x0a, 0x12, 0x43, 0x12, 0x70, 0x0c, 0x12, 0x43,
        0x1f, 0x70, 0x07, 0x12, 0x46, 0x07, 0x7b, 0x03, 0x80, 0x07,
        0x12, 0x46, 0x07, 0x12, 0x46, 0x11, 0xfb, 0x7a, 0x00, 0x7d,
        0x54, 0x80, 0x3e, 0x12, 0x42, 0xec, 0xef, 0x4e, 0x70, 0x24,
        0x12, 0x43, 0x04, 0x60, 0x0a, 0x12, 0x43, 0x12, 0x70, 0x0f,
        0x12, 0x43, 0x1f, 0x70, 0x0a, 0x12, 0x46, 0x07, 0xe4, 0xfb,
        0xfa, 0x7d, 0xee, 0x80, 0x1e, 0x12, 0x46, 0x07, 0x7b, 0x01,
        0x7a, 0x00, 0x7d, 0xee, 0x80, 0x13, 0x12, 0x46, 0x07, 0x12,
        0x46, 0x11, 0x54, 0x40, 0xfe, 0xc4, 0x13, 0x13, 0x54, 0x03,
        0xfb, 0x7a, 0x00, 0x7d, 0xee, 0x12, 0x38, 0xbd, 0x7b, 0xff,
        0x12, 0x43, 0x2c, 0xef, 0x4e, 0x70, 0x07, 0x74, 0x2a, 0x25,
        0x19, 0xf8, 0xe4, 0xf6, 0x05, 0x19, 0xe5, 0x19, 0xc3, 0x94,
        0x02, 0x50, 0x03, 0x02, 0x42, 0x15, 0x22, 0xe5, 0x19, 0x24,
        0x17, 0xfd, 0x7b, 0x20, 0x7f, 0x04, 0x12, 0x3d, 0xd7, 0x22,
        0xe5, 0x19, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12, 0x3d, 0xd7,
        0x22, 0x7b, 0x22, 0x7d, 0x18, 0x7f, 0x06, 0x12, 0x3d, 0xd7,
        0xef, 0x64, 0x01, 0x4e, 0x22, 0x7d, 0x1c, 0xe4, 0xff, 0x12,
        0x3e, 0x9a, 0xef, 0x54, 0x1b, 0x64, 0x0a, 0x22, 0x7b, 0xcc,
        0x7d, 0x10, 0xff, 0x12, 0x3d, 0xd7, 0xef, 0x64, 0x01, 0x4e,
        0x22, 0xe5, 0x19, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12, 0x3d,
        0xd7, 0x22, 0xd2, 0x08, 0x75, 0xfb, 0x03, 0xab, 0x7e, 0xaa,
        0x7d, 0x7d, 0x19, 0x7f, 0x03, 0x12, 0x3e, 0xda, 0xe5, 0x7e,
        0x54, 0x0f, 0x24, 0xf3, 0x60, 0x03, 0x02, 0x43, 0xe9, 0x12,
        0x46, 0x28, 0x12, 0x46, 0x2f, 0xd8, 0xfb, 0xff, 0x20, 0xe2,
        0x35, 0x13, 0x92, 0x0c, 0xef, 0xa2, 0xe1, 0x92, 0x0b, 0x30,
        0x0c, 0x2a, 0xe4, 0xf5, 0x10, 0x7b, 0xfe, 0x12, 0x43, 0xff,
        0xef, 0xc4, 0x33, 0x33, 0x54, 0xc0, 0xff, 0xc0, 0x07, 0x7b,
        0x54, 0x12, 0x43, 0xff, 0xd0, 0xe0, 0x4f, 0xff, 0x74, 0x2a,
        0x25, 0x10, 0xf8, 0xa6, 0x07, 0x05, 0x10, 0xe5, 0x10, 0xc3,
        0x94, 0x02, 0x40, 0xd9, 0x12, 0x46, 0x28, 0x12, 0x46, 0x2f,
        0xd8, 0xfb, 0x54, 0x05, 0x64, 0x04, 0x70, 0x27, 0x78, 0xc4,
        0xe6, 0x78, 0xc6, 0xf6, 0xe5, 0x7d, 0xff, 0x33, 0x95, 0xe0,
        0xef, 0x54, 0x0f, 0x78, 0xc4, 0xf6, 0x12, 0x44, 0x0a, 0x20,
        0x0c, 0x0c, 0x12, 0x46, 0x28, 0x12, 0x46, 0x2f, 0xd8, 0xfb,
        0x13, 0x92, 0x0d, 0x22, 0xc2, 0x0d, 0x22, 0x12, 0x46, 0x28,
        0x12, 0x46, 0x2f, 0xd8, 0xfb, 0x54, 0x05, 0x64, 0x05, 0x70,
        0x1e, 0x78, 0xc4, 0x7d, 0xb8, 0x12, 0x43, 0xf5, 0x78, 0xc1,
        0x7d, 0x74, 0x12, 0x43, 0xf5, 0xe4, 0x78, 0xc1, 0xf6, 0x22,
        0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee, 0x7f, 0x92, 0x12, 0x38,
        0xbd, 0x22, 0xe6, 0xfb, 0x7a, 0x00, 0x7f, 0x92, 0x12, 0x38,
        0xbd, 0x22, 0xe5, 0x10, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12,
        0x3d, 0xd7, 0x22, 0x78, 0xc1, 0xe6, 0xfb, 0x7a, 0x00, 0x7d,
        0x74, 0x7f, 0x92, 0x12, 0x38, 0xbd, 0xe4, 0x78, 0xc1, 0xf6,
        0xf5, 0x11, 0x74, 0x01, 0x7e, 0x00, 0xa8, 0x11, 0x08, 0x80,
        0x05, 0xc3, 0x33, 0xce, 0x33, 0xce, 0xd8, 0xf9, 0xff, 0x78,
        0xc4, 0xe6, 0xfd, 0xef, 0x5d, 0x60, 0x44, 0x85, 0x11, 0xfb,
        0xe5, 0x11, 0x54, 0x02, 0x25, 0xe0, 0x25, 0xe0, 0xfe, 0xe4,
        0x24, 0x5b, 0xfb, 0xee, 0x12, 0x45, 0x9e, 0x12, 0x3e, 0xda,
        0x7b, 0x40, 0x7d, 0x11, 0x7f, 0x07, 0x12, 0x3d, 0xd7, 0x74,
        0xc7, 0x25, 0x11, 0xf8, 0xa6, 0x07, 0x7b, 0x11, 0x7d, 0x12,
        0x7f, 0x07, 0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60, 0x09, 0x74,
        0xe7, 0x25, 0x11, 0xf8, 0x76, 0x04, 0x80, 0x07, 0x74, 0xe7,
        0x25, 0x11, 0xf8, 0x76, 0x0a, 0x05, 0x11, 0xe5, 0x11, 0xc3,
        0x94, 0x04, 0x40, 0x9a, 0x78, 0xc6, 0xe6, 0x70, 0x15, 0x78,
        0xc4, 0xe6, 0x60, 0x10, 0x75, 0xd9, 0x38, 0x75, 0xdb, 0x10,
        0x7d, 0xfe, 0x12, 0x44, 0xb8, 0x7d, 0x76, 0x12, 0x44, 0xb8,
        0x79, 0xc6, 0xe7, 0x78, 0xc4, 0x66, 0xff, 0x60, 0x03, 0x12,
        0x40, 0x25, 0x78, 0xc4, 0xe6, 0x70, 0x09, 0xfb, 0xfa, 0x7d,
        0xfe, 0x7f, 0x8e, 0x12, 0x38, 0xbd, 0x22, 0x7b, 0x01, 0x7a,
        0x00, 0x7f, 0x8e, 0x12, 0x38, 0xbd, 0x22, 0xe4, 0xf5, 0xfb,
        0x7d, 0x1c, 0xe4, 0xff, 0x12, 0x3e, 0x9a, 0xad, 0x07, 0xac,
        0x06, 0xec, 0x54, 0xc0, 0xff, 0xed, 0x54, 0x3f, 0x4f, 0xf5,
        0x20, 0x30, 0x06, 0x2c, 0x30, 0x01, 0x08, 0xa2, 0x04, 0x72,
        0x03, 0x92, 0x07, 0x80, 0x21, 0x30, 0x04, 0x06, 0x7b, 0xcc,
        0x7d, 0x11, 0x80, 0x0d, 0x30, 0x03, 0x06, 0x7b, 0xcc, 0x7d,
        0x10, 0x80, 0x04, 0x7b, 0x66, 0x7d, 0x16, 0xe4, 0xff, 0x12,
        0x3d, 0xd7, 0xee, 0x4f, 0x24, 0xff, 0x92, 0x07, 0xaf, 0xfb,
        0x74, 0x26, 0x2f, 0xf8, 0xe6, 0xff, 0xa6, 0x20, 0x20, 0x07,
        0x39, 0x8f, 0x20, 0x30, 0x07, 0x34, 0x30, 0x00, 0x31, 0x20,
        0x04, 0x2e, 0x20, 0x03, 0x2b, 0xe4, 0xf5, 0xff, 0x75, 0xfc,
        0xc2, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0xaf, 0xfe, 0xef, 0x20,
        0xe3, 0x1a, 0xae, 0xfd, 0x44, 0x08, 0xf5, 0xfe, 0x75, 0xfc,
        0x80, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x8f, 0xfe, 0x8e, 0xfd,
        0x75, 0xfc, 0x80, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x05, 0xfb,
        0xaf, 0xfb, 0xef, 0xc3, 0x94, 0x04, 0x50, 0x03, 0x02, 0x44,
        0xc5, 0xe4, 0xf5, 0xfb, 0x22, 0xe5, 0x7e, 0x30, 0xe5, 0x35,
        0x30, 0xe4, 0x0b, 0x7b, 0x02, 0x7d, 0x33, 0x7f, 0x35, 0x12,
        0x36, 0x29, 0x80, 0x10, 0x7b, 0x01, 0x7d, 0x33, 0x7f, 0x35,
        0x12, 0x36, 0x29, 0x90, 0x47, 0xd2, 0xe0, 0x44, 0x04, 0xf0,
        0x90, 0x47, 0xd2, 0xe0, 0x54, 0xf7, 0xf0, 0x90, 0x47, 0xd1,
        0xe0, 0x44, 0x10, 0xf0, 0x7b, 0x05, 0x7d, 0x84, 0x7f, 0x86,
        0x12, 0x36, 0x29, 0x22, 0xfb, 0xe5, 0x1c, 0x34, 0xf0, 0xfa,
        0x7d, 0x10, 0x7f, 0x07, 0x22, 0x54, 0x01, 0xc4, 0x33, 0x54,
        0xe0, 0xf5, 0xdb, 0x44, 0x08, 0xf5, 0xdb, 0x22, 0xf5, 0xdb,
        0x75, 0xdb, 0x08, 0xf5, 0xdb, 0x75, 0xdb, 0x08, 0x22, 0xe5,
        0x7e, 0x54, 0x0f, 0x64, 0x01, 0x70, 0x0d, 0xe5, 0x7e, 0x30,
        0xe4, 0x08, 0x90, 0x47, 0xd0, 0xe0, 0x44, 0x02, 0xf0, 0x22,
        0x90, 0x47, 0xd0, 0xe0, 0x54, 0xfd, 0xf0, 0x22, 0xab, 0x07,
        0xaa, 0x06, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3e, 0xda, 0x7b,
        0xff, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3d, 0xd7, 0xef, 0x4e,
        0x60, 0xf3, 0x22, 0x12, 0x44, 0xc2, 0x30, 0x0c, 0x03, 0x12,
        0x42, 0x12, 0x78, 0xc4, 0xe6, 0xff, 0x60, 0x03, 0x12, 0x40,
        0x25, 0x22, 0xe5, 0x19, 0x24, 0x17, 0x54, 0x1f, 0x44, 0x80,
        0xff, 0x22, 0x74, 0x2a, 0x25, 0x19, 0xf8, 0xe6, 0x22, 0x12,
        0x46, 0x40, 0x12, 0x46, 0x36, 0x90, 0x47, 0xfa, 0xe0, 0x54,
        0xf8, 0x44, 0x02, 0xf0, 0x22, 0xe5, 0x7e, 0xae, 0x7d, 0x78,
        0x04, 0x22, 0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0x22, 0xe4,
        0x78, 0xc4, 0xf6, 0xc2, 0x0d, 0x78, 0xc1, 0xf6, 0x22, 0xc2,
        0x0c, 0xc2, 0x0b, 0x22, 0x22
    };
    VTSS_RC(atom_download_8051_code(phy_handle, &patch_arr[0], sizeof(patch_arr)));

    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle,  3, 0x3eb7));     // Trap ROM at _MicroSmiRead+0x1d to spoof patch-presence
    VTSS_RC(vsc8504_phy_wr(phy_handle,  4, 0x4012));     // Branch to starting address of SpoofPatchPresence
    VTSS_RC(vsc8504_phy_wr(phy_handle, 12, 0x0100));     // Enable patch fram trap described in register 3-4
    VTSS_RC(vsc8504_phy_wr(phy_handle,  0, 0x4018));     // Enable 8051 clock; no longer indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    VTSS_RC(vsc8504_phy_wr(phy_handle,  0, 0xc018));     // Release 8051 SW Reset
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    
    // Check that code is downloaded correctly.
    VTSS_RC(vsc8504_phy_is_8051_crc_ok_private(phy_handle,
                                            FIRMWARE_START_ADDR,
                                            sizeof(patch_arr) + 1, // Add one for the byte auto-added in the download function
                                            0x2BB0));

    return VTSS_RC_OK;
}


static int32 vsc8504_phy_pre_init_tesla_revB_1588(phy_handle_t* phy_handle)
{
    /* Pass the cmd to Micro to initialize all 1588 analyzer registers to default */
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x801A));
    /* Poll on 18G.15 to clear */
    VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle));
    /* return to standard page */
    VTSS_RC(vsc8504_phy_page_std(phy_handle));

    return VTSS_RC_OK;
}


// Initialization needed for Tesla. (For whole chip and must be done before init. of the individual ports)
//
// In : iport : Port number (MUST be the first port for the chip).. 
//
// Return : VTSS_RC_OK if configuration done else error code.
// Based on matlab init_script.m,  Revision 1.13  2011/08/17 15:20:37  jimb
// Calls James McIntosh' latest patch for the 1588 SPI issue dated 9/12/2011
// On 27-09-2012 - updated to address bugzilla #9871 and #9886

int32 vsc8504_phy_pre_init_seq_tesla_17_august_2011 (phy_handle_t* phy_handle) 
{
    // MII register writes and test-page register writes go here
    //using broadcast flag to speed things up
    VTSS_RC(vsc8504_phy_page_std(phy_handle));                   //Switch to main register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 22,0x0001,0x0001)); //turn on broadcast writes

    VTSS_RC(vsc8504_phy_page_test(phy_handle));             //Switch to test register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 8,0x8000,0x8000)); //Enable token-ring during coma-mode

    VTSS_RC(vsc8504_phy_page_tr(phy_handle));             //Switch to token-ring register page

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x1));

/* It is found that the settings commented out below can give a few frames with CRC (one every 1-2 hours), so
   they are left out. The settings are related to EEE, and this may give some EEE problems, but they are going to 
fixed in chip rev. C.
      VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x9fa2));
      VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x968a));

      VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0xd2));
      VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x545a));
      VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x968c)); */

    // Fix for bugzilla#9871 (Atheros EEE interop) and bugzilla#9886 (1000BT too long to link up)
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0004));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x01bd));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fae));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x000f));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x000f));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fac));


    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x4));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xe004));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a8));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7200));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a2));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x5d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xdddd));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8ee0));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0xee));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xffff));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a0));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x3f));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x5e48));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a6));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x2a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2d2e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x9794));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x7));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x150));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe0));

    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b2));

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0068));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8980));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f90));

    //Note that when Tesla is configured for 10BASE-T (not Te), it needs no 10BASE-T configuration as its pulse codes already match
#ifdef VTSS_10BASE_TE
    VTSS_RC(vsc8504_phy_65nm_10BASE_init(phy_handle, 0));
#endif

    VTSS_RC(vsc8504_phy_page_test(phy_handle));             //Switch to test register page
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 8,0x0000,0x8000)); //Disable token-ring after complete


    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 22, 0x0000, 0x0001)); // Turn off broadcast writes
    
    VTSS_RC(tesla_revB_8051_patch(phy_handle)); //Load micro patch Tesla RevB

    VTSS_RC(vsc8504_phy_pre_init_tesla_revB_1588(phy_handle)); //Init 1588 register using Tesla RevB micro patch
   
    return VTSS_RC_OK;
}

// Initialization needed for Tesla. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no : Port number (MUST be the first port for the chip).. 
//
// Return : VTSS_RC_OK if configuration done else error code.
// Date : 27-09-2012 - updated to address bugzilla #9871 and #9886
int32 vsc8504_phy_pre_init_seq_tesla_rev_d (phy_handle_t* phy_handle) 
{
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 22, 0x0001, 0x0001));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 24, 0x0040));
    VTSS_RC(vsc8504_phy_page_ext2(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x02be));
    VTSS_RC(vsc8504_phy_page_test(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 20, 0x4420));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 24, 0x0c00));
    VTSS_RC(vsc8504_phy_wr(phy_handle,  9, 0x18cc));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,  8, 0x8000, 0x8000));
    VTSS_RC(vsc8504_phy_wr(phy_handle,  5, 0x1320));
    VTSS_RC(vsc8504_phy_page_tr(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0004));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x01bd));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fae));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x000f));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x000f));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fac));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00a0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf147));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x97a0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0005));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2f54));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe4));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0027));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x303d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x9792));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0704));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x87fe));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0006));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0150));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0012));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x480a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f82));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0034));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f80));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0012));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x82e0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0005));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0208));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x83a2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x9186));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x83b2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x000e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x3700));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fb0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0004));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x9fa0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x9688));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xffff));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fd2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0003));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x9fa0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x968a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0020));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x640b));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x9690));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2220));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8258));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2a20));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x825a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x3060));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x825c));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x3fa0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x825e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xe0f0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x83a6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x4489));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f92));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0010));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2048));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00ff));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96a0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0091));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x9880));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fe8));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0004));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xd602));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8fea));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00ef));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xef00));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x7100));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x5064));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x96b4));

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0068));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8980));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f90));

#ifndef VTSS_10BASE_TE
    VTSS_RC(vsc8504_phy_page_tr(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0071));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf6d9));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8488));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0db6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x848e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0059));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x6596));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x849c));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0514));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x849e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0041));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0280));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a4));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a8));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84aa));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x007d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf7dd));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84ae));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x006d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x95d4));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84b0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0049));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x2410));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84b2));
#else //using 10BASE-Te
    VTSS_RC(vsc8504_phy_page_ext2(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8000));
    VTSS_RC(vsc8504_phy_page_tr(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0008));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xa499));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8486));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0075));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xf759));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8488));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0914));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x848a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00f7));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xff7b));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x848c));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0eb9));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x848e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x85d6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8490));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0055));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x44d2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8492));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0044));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xa8aa));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8494));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0cb9));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8496));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x00f7));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xff79));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8498));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0caa));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x849a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8618));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x849c));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0618));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x849e));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0018));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x848a));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a4));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a6));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84a8));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x0000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84aa));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0029));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x265d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84ac));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x007d));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0xd658));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84ae));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8618));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84b0));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8618));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84b2));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x0061));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 17, 0x8618));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x84b4));
#endif //VTSS_10BASE_TE
    VTSS_RC(vsc8504_phy_page_test(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,  8, 0x0000, 0x8000));
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 22, 0x0000, 0x0001));

    VTSS_RC(tesla_revB_8051_patch(phy_handle)); // Rev D. uses the same patch as rev B.

    VTSS_RC(vsc8504_phy_pre_init_tesla_revB_1588(phy_handle)); //Init 1588 register using Tesla RevB micro patch
   
    return VTSS_RC_OK;

}


int32 vsc8504_phy_pre_reset(phy_handle_t* phy_handle, uint16 revision)
{
    switch(revision)
    {
        case VTSS_PHY_TESLA_REV_A:
            VTSS_RC(vsc8504_phy_pre_init_seq_tesla_4_april_2011(phy_handle));
            break;
        case VTSS_PHY_TESLA_REV_B:
            VTSS_RC(vsc8504_phy_pre_init_seq_tesla_17_august_2011(phy_handle));
            break;
        case VTSS_PHY_TESLA_REV_D:
            VTSS_RC(vsc8504_phy_pre_init_seq_tesla_rev_d(phy_handle));
            break;
        default:
            VTSS_RC(vsc8504_phy_pre_init_seq_tesla_rev_d(phy_handle));
            break;
    }        
    return VTSS_RC_OK;
}
/*****************phy pre reset end************************/

/*****************phy reset start**************************/
//Function for suspending / resuming the 8051 patch.
//
// In : iport - Any port within the chip where to supend 8051 patch
//      suspend - True if 8051 patch shall be suspended, else patch is resumed.
// Return : VTSS_RC_OK if patch was suspended else error code. 
int32 vsc8504_patch_suspend(phy_handle_t* phy_handle, uint8 suspend) 
{
    uint16 word;

    if (phy_handle->phy_info.phy_revision == VTSS_PHY_TESLA_REV_A) 
    {
        // No patch suspending for Rev. A.
        return VTSS_RC_OK;
    }
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle)); // Change to GPIO page
    // We must disable the EEE patch when VeriPHY is running,  When VeriPHY  is running, the patch  needs to be suspended by 
    // writing 0x800f to register 18 on the Micro/GPIO page (0x10).  0x800f is chosen as this is an unimplemented micro-command 
    // and issuing any micro command when the patch is running, causes the patch to stop. 
    // The micro patch must not be enabled until all pending VeriPHY requests have been completed on all ports in the relevant Luton26 or Atom12 chip. 
    // When all have been completed, the micro patch should re-enable.  Note that this is necessary only because the patch for EEE consumes 
    //the micro continually to service all 12 PHYs in a timely manner and workaround one of the weaknesses in gigabit EEE in Luton26/Atom12.
    if ((phy_handle->phy_info.phy_veriphy_tsk != NULL)&&(phy_handle->phy_info.phy_veriphy_tsk->ret==GLB_VCT_WAIT)) {
        VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x800F)); // Suspend 8051 EEE patch 
        VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle));
        VTSS_RC(vsc8504_phy_page_std(phy_handle)); // Back to standard page
        return VTSS_RC_OK;
    } 

    if (suspend) 
    {
        // Suspending 8051 patch
        // See comment below.
        VTSS_RC(vsc8504_phy_rd(phy_handle, 18, &word));
        if (!(word & 0x4000)) 
        {
            VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x800F)); // Suspend 8051 EEE patch 
        }
        VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle)); 
    } 
    else 
    {
        // Resuming 8051 patch
        // On page 0x10 (Reg31 = 0x10), write register 18 with 0x8009 to turn on the EEE patch.  
        // Once this is done all code that might attempt to access page 0x52b5 will fail and likely cause issues.  
        // If you need to access page 0x52b5 or run another page 0x10, register 18 function, you must first disable 
        // the patch by writing 0x8009 again to register 18.  In response, the error bit (bit 14) will be set, 
        // but the micro is now freed from running the EEE patch and you may issue your other micro requests. 
        // When the other requests are complete, you will want to rerun the EEE patch by writing 0x8009 to register 18 on page 0x10.  
        // The events that are handled by the micro patch occur occasionally, say one event across 12 ports every 30 seconds.
        // As a result, suspending the EEE patch for short durations Is unlikely to result in link drops, but it is possible.
        VTSS_RC(vsc8504_phy_rd(phy_handle, 18, &word));
        if (word & 0x4000) 
        {
            VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8009)); 
        }
        VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle)); 
    }
    VTSS_RC(vsc8504_phy_page_std(phy_handle)); // Change to standard page
   
    return VTSS_RC_OK;
}

static int32 vsc8504_phy_set_private_atom(phy_handle_t* phy_handle, uint8 is_force)
{
    VTSS_RC(vsc8504_patch_suspend(phy_handle, 1));
    VTSS_RC(vsc8504_phy_page_test(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,  8, 0x8000, 0x8000)); //Ensure RClk125 enabled even in powerdown
    // Clear Cl40AutoCrossover in forced-speed mode, but set it in non-forced modes
    VTSS_RC(vsc8504_phy_page_tr(phy_handle));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0xa7fa));  // issue token-ring read request
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 17, (is_force)? 0x0000 : 0x1000, 0x1000));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x87fa));  // issue token-ring write request

    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0xaf82));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 17, 0x2, 0xf));
    VTSS_RC(vsc8504_phy_wr(phy_handle, 16, 0x8f82));

    VTSS_RC(vsc8504_phy_page_test(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle,  8, 0x0000, 0x8000)); //Restore RClk125 gating
    VTSS_RC(vsc8504_patch_suspend(phy_handle, 0));
    VTSS_RC(vsc8504_phy_page_std(phy_handle));

    // Enable HP Auto-MDIX in forced-mode (by clearing disable bit)
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, BYPASS_CTL, 0x0000, 0x80)); 
    return VTSS_RC_OK;
}

/*Now only use MDIX-Auto.*/
static int32 vsc8504_phy_mdi_setup(phy_handle_t* phy_handle)
{
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, BYPASS_CTL, 0x0, 0xa0));
    VTSS_RC(vsc8504_phy_page_ext(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 19, 0x0, 0xc));
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    return VTSS_RC_OK;
}

static int32 vsc8504_phy_bugzero_48512_workaround(phy_handle_t* phy_handle, uint8 start)
{
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    if(start)
    {
        VTSS_RC(vsc8504_phy_wr(phy_handle, 0, 0x4040));
    }
    else
    {
        VTSS_RC(vsc8504_phy_wr(phy_handle, 0, 0x0));
    }
    return VTSS_RC_OK;
}

// Function for setting phy in pass through mode according to "Application Note : Protocol transfer mode guide"
static int32 vsc8504_phy_pass_through_speed_mode(phy_handle_t* phy_handle)
{
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    //Protocol Transfer mode Guide : Section 4.1.1 - Aneg must be enabled
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x1000, 0x1000));

    VTSS_RC(vsc8504_phy_page_ext3(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 16, 0x0080, 0x0880)); // Default clear "force advertise ability" bit as well

    // Protocol Transfer mode Guide : Section 4.1.3
    if((phy_handle->phy_info.phy_manage_info.speed != GLB_SPEED_AUTO)
        &&(phy_handle->phy_info.phy_manage_info.duplex != GLB_DUPLEX_AUTO))
    {
        VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 16, 0x0800, 0x0800));
        switch (phy_handle->phy_info.phy_manage_info.speed) 
        {
            case GLB_SPEED_100M:
                VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8401));
                break;
            case GLB_SPEED_10M:
                VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8001));
                break;
            case GLB_SPEED_1G:
                VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8801));
                break;
            default:
                DRV_LOG_ERR("Unexpected port speed:%d defaulting to 1G", phy_handle->phy_info.phy_manage_info.speed);
                VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x8801));
                break;
        }
    }
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    return VTSS_RC_OK;
}


/*****************phy config set start**************************/
int32 vsc8504_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    uint16 reg4=1, reg9=0;

    VTSS_RC(vsc8504_phy_page_std(phy_handle));  
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
        VTSS_RC(vsc8504_phy_wr_masked(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0));
        VTSS_RC(vsc8504_phy_wr_masked(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300));
        VTSS_RC(vsc8504_phy_set_private_atom(phy_handle, 0));
        /* Use register 0 to restart auto negotiation */
        VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG,    0x1200, 0x1200));        
    }
    else /* Force mode*/
    {        
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0800, 0x0800));
	     /* Modified by liuht to powerdown more time to notify link parter for bug25796, 2013-11-14 */		
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0000, 0x0800));
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0100, 0x3140));
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0000, 0x3140));
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x2100, 0x3140));
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x2000, 0x3140));
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0140, 0x3140));
        }
        else /*1G Half*/
        {    
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x0040, 0x3140));
        }
        if(speed != GLB_SPEED_1G)
        {
            /* Enable Auto MDI/MDI-X in forced 10/100 mode */
            VTSS_RC(vsc8504_phy_set_private_atom(phy_handle, 1));
        }
    }
    VTSS_RC(vsc8504_phy_mdi_setup(phy_handle));

    if(phy_handle->phy_info.phy_manage_info.media_type == GLB_MEDIA_PASS_THROUGH)
    {
        VTSS_RC(vsc8504_phy_pass_through_speed_mode(phy_handle));
    }
    else
    {
        VTSS_RC(vsc8504_phy_page_ext3(phy_handle));
        // Recommended by chip designers - This should allow link-up if the MAC is not doing auto-neg.
        // Enable "MAC interface autonegotiation parallel detect", else data flow is stopped for the CU ports if PHY has MAC ANEG enabled and the switch is connected to isn't
        VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 16, 0x2000, 0x2000));
        VTSS_RC(vsc8504_phy_page_std(phy_handle));
    }    
    return VTSS_RC_OK;
}
/*****************phy config set end**************************/

static int32 vsc8504_phy_soft_reset_port(phy_handle_t* phy_handle)
{
    uint16 reg_val = 0, count = 0;
    
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, PHY_CTL_REG, 0x8000, 0x8000));
    usleep(40000);
    while(1)
    {        
        VTSS_RC(vsc8504_phy_rd(phy_handle, PHY_CTL_REG, &reg_val));
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
    vsc8504_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed,phy_handle->phy_info.phy_manage_info.duplex);
    return VTSS_RC_OK;
}

static int vsc8504_port_reset(phy_handle_t* phy_handle)
{
    VTSS_RC(vsc8504_patch_suspend(phy_handle, 1));
    VTSS_RC(vsc8504_phy_soft_reset_port(phy_handle));
    VTSS_RC(vsc8504_patch_suspend(phy_handle, 0));
    return VTSS_RC_OK;
}

static int32 vsc8504_phy_mac_media_if_tesla_setup(phy_handle_t* phy_handle,glb_mac_interface_t mac_if,glb_media_interface_t  media_type, uint8 first_init)
{
    uint16 micro_cmd_100fx = 0; // Use to signal to micro program if the fiber is 100FX (Bit 4). Default is 1000BASE-x 
    uint8 media_operating_mode = 0;
    uint8 mac_interface_mode = 0;
    uint8 cu_prefered = 0;
    uint16 reg_val;
    
    // Setup MAC Configuration
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));
    switch(mac_if)
    {
        case GLB_MAC_INTERFACE_SGMII:
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 19,  0x0000, 0xc000));
            break;
        case GLB_MAC_INTERFACE_QSGMII:
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 19,  0x4000, 0xc000));
            break;
        case GLB_MAC_INTERFACE_RGMII:
            VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 19,  0x8000, 0xc000));
            break;    
        default:
            return VTSS_RC_ERROR;
    }
    // Setup media interface
    switch(media_type)
    {
        case GLB_MEDIA_RJ45:
            media_operating_mode = 0;
            cu_prefered = 1;  
            break;
        case GLB_MEDIA_PASS_THROUGH:
//            mac_interface_mode = 1;   jqiu. set this cause packet not go through.
            media_operating_mode = 1;
//            cu_prefered = 1;          jqiu
            break;
        case GLB_MEDIA_1000BX:
//            mac_interface_mode = 1;  jqiu
            media_operating_mode = 2;
            cu_prefered = 0;  
            break;
        case GLB_MEDIA_FI_100FX:
            media_operating_mode = 3;
            cu_prefered = 0;  
            micro_cmd_100fx = 1 << 4;
            break;
        case GLB_MEDIA_AMS_CU_PASSTHRU:
            media_operating_mode = 5;
            cu_prefered = 1;  
            break;
        case GLB_MEDIA_AMS_FI_PASSTHRU:
            media_operating_mode = 5;
            cu_prefered = 0;  
            break;
        case GLB_MEDIA_AMS_CU_1000BX:
            media_operating_mode = 6;
            cu_prefered = 1;  
            break;
        case GLB_MEDIA_AMS_FI_1000BX:
            media_operating_mode = 6;
            cu_prefered = 0;  
            break;
        case GLB_MEDIA_AMS_CU_100FX:
            media_operating_mode = 7;
            cu_prefered = 1;  
            micro_cmd_100fx = 1 << 4;
            break;
        case GLB_MEDIA_AMS_FI_100FX:
            media_operating_mode = 7;
            cu_prefered = 0;  
            micro_cmd_100fx = 1 << 4;
            break;
        default:
            return VTSS_RC_ERROR;
    }        
    
    vsc8504_phy_bugzero_48512_workaround(phy_handle, 1);//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.    
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));  
    if((first_init)||(phy_handle->phy_info.phy_manage_info.mac_if != mac_if))
    {
        if(mac_if == GLB_MAC_INTERFACE_QSGMII)
        {
            VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x80e0));
        }
        else
        {
            VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x80f0));
        }    
        VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle));
        /*Below function just for VTSS_PHY_FAMILY_VIPER and VTSS_PHY_FAMILY_ELISE */
        /*if (VTSS_PHY_BASE_PORTS_FOUND == VTSS_RC_OK) {
            VTSS_RC(vtss_phy_sd6g_patch_private(vtss_state, port_no));
        }*/
    }
    usleep(10000);
    vsc8504_phy_bugzero_48512_workaround(phy_handle, 0);//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    
    if(media_type != GLB_MEDIA_RJ45)
    {
        vsc8504_phy_bugzero_48512_workaround(phy_handle, 1);//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
        // Setup media in micro program. Bit 8-11 is bit for the corresponding port (See TN1080)
        VTSS_RC(vsc8504_phy_page_gpio(phy_handle));        
        VTSS_RC(vsc8504_phy_wr(phy_handle, 18, 0x80C1 | (0x0100 << (phy_handle->phy_info.phy_addr% 4)) | micro_cmd_100fx));
        VTSS_RC(vsc8504_phy_wait_for_micro_complete(phy_handle)); 
        usleep(10000);

        vsc8504_phy_bugzero_48512_workaround(phy_handle, 0);//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    }
    // Setup Media interface
    VTSS_RC(vsc8504_phy_page_std(phy_handle));    
    reg_val = (media_operating_mode << 8) | (cu_prefered <<11) |(mac_interface_mode <<12);
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, 23, reg_val, 0x1f00));
    
    // Port must be reset in order to update the media operating mode for register 23    
    VTSS_RC(vsc8504_phy_soft_reset_port(phy_handle));
    usleep(10000);   
    return VTSS_RC_OK;
}

int32 vsc8504_phy_reset(phy_handle_t* phy_handle,glb_mac_interface_t mac_if,glb_media_interface_t  media_type, uint8 first_init)
{
    /* -- Step 2: Pre-reset setup of MAC and Media interface -- */
    VTSS_RC(vsc8504_phy_mac_media_if_tesla_setup(phy_handle, mac_if, media_type, first_init));
    /* -- Step 3: Reset PHY -- */
    VTSS_RC(vsc8504_phy_page_std(phy_handle));    
    VTSS_RC(vsc8504_port_reset(phy_handle));
    /* -- Step 4: Run startup scripts -- */
    /*Nothing to do for tesla*/
    return VTSS_RC_OK;
    
}
/*****************phy reset end**************************/

/*****************phy post reset start************************/
//  Function for pulling the coma mode pin high or low (The coma mode pin MUST be pulled high by an external pull up resistor)
//
// In : port  : Phy port (Any port within the PHY chip to pull down coma mode pin).
// Return :  VTSS_RC_OK is setup was preformed correct else VTSS_RC_ERROR
int32 vsc8504_phy_coma_mode_private(phy_handle_t* phy_handle)
{
// COMA mode is a new concept for Luton26/Atom12 for the PHYs and when the COMA_MODE pin is asserted, the PHY will 
// remain powered down which is the ideal state to configure the PHY and SerDes MAC interfaces.  C
// COMA_MODE is designed to also synchronize LED clocks upon deassertion of COMA_MODE.  T
// The board designer should tie all COMA_MODE pins together. When the pin floats, an internal pull-up asserts the COMA_MDOE pin.  
// After all internal Luton26 and external Atom12 PHYs have been configured, the initialization scripts should configure one of the devices to 
// drive COMA_MODE pin low by changing COMA_MODE from an input to an output and setting the output to 0.  This will be sensed simultaneously 
// at all PHYs and synchronize LED clocks between these PHYs.  Note that the LED blink on reset will occur, if configured, at the time that COMA_MODE goes low.
    
    
// The COMA_MODE pin may be configured using micro/GPIO register 14G.[13:12]. 
// Bit 13 is an active-low output enable which defaults on reset to 1 (e.g. output disabled). 
// Bit 12 is the bit to drive the COMA_MODE pin to when the output is enabled.    
    VTSS_RC(vsc8504_phy_page_gpio(phy_handle));
    VTSS_RC(vsc8504_phy_wr_masked(phy_handle, VTSS_PHY_GPIO_CONTROL_2, 0x0000, 
                               VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE | 
                               VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_DATA));
    VTSS_RC(vsc8504_phy_page_std(phy_handle));
    return VTSS_RC_OK;
}

int32 vsc8504_phy_post_reset(phy_handle_t* phy_handle)
{
    vsc8504_phy_coma_mode_private(phy_handle);
    return RESULT_OK;
}
/*****************phy post reset end************************/
static int32
vsc8504_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = 0;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VSC8504 phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
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
            ret += vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
        }
    }
    return ret;
}

int32 vsc8504_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media)
{
    uint16 vga_stat;
    int16 max_vga_state_to_opimize = -9;
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8504_link_up_setting speed: %d duplex: %d", speed,duplex);
    if(speed == GLB_SPEED_1G)
    {
        /* BZ 1776/1860/2095/2107 part 3/3 */
        ret += vsc8504_phy_page_tr(phy_handle);
        ret += vsc8504_write(phy_handle, 16, 0xaff0);
        ret += vsc8504_read(phy_handle, 17, &vga_stat);
        vga_stat = ((vga_stat >> 4) & 0x01f);
        /* vga_stat is a 2's complement signed number ranging from -13 to +8 */
        /* Test for vga_stat < 16 is really a check for positive vga_stat */
        /* for PHY VSC8504, max_vga_state_to_optimize is equal to -9 */
        if ((vga_stat < 16) || (vga_stat > (max_vga_state_to_opimize & 0x1f))) 
        {
            DRV_LOG_DEBUG(DRV_PHY, "vga_stat = %u", vga_stat);
            ret += vsc8504_phy_page_test(phy_handle);
            ret += vsc8504_write_mask(phy_handle, 12, 0x0000, 0x0300);
        }
        ret += vsc8504_phy_page_std(phy_handle);
    }
    
    return ret;
}

int32 
vsc8504_link_down_setting(phy_handle_t* phy_handle)
{
#if 0
    phy_info_t *pphy_info;    
    uint16 value; 
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8504_link_down_setting ");
    pphy_info = &phy_handle->phy_info;

    if((pphy_info->phy_manage_info.speed == GLB_SPEED_10M)
        &&(pphy_info->phy_manage_info.duplex != GLB_DUPLEX_AUTO))
    {
        ret += vsc8504_phy_wr_masked(phy_handle, 22, 0x8000, 0x8000);
        ret += vsc8504_phy_wr_masked(phy_handle, 22, 0x0000, 0x8000);
        usleep(20000);
        ret += vsc8504_read(phy_handle, PHY_STATUS_REG, &value);
    }
    else if(((pphy_info->phy_manage_info.speed == GLB_SPEED_100M)||(pphy_info->phy_manage_info.speed == GLB_SPEED_1G))
        &&(pphy_info->phy_manage_info.duplex != GLB_DUPLEX_AUTO))
    {
        ret += vsc8504_phy_wr_masked(phy_handle, 0, 0x0800, 0x0800);
        ret += vsc8504_phy_wr_masked(phy_handle, 0, 0x0000, 0x0800);
    }
    return ret;
#else
    return 0;
#endif
} 

/**********************************************************************
 *  Name:       : phy_vsc8504_get_cur_status
 *  Purpose     : get current link status, speed and duplex per port
 *  Input       : mdio_bus_no: MDIO bus ID
                  phy_addr:    PHY address per MDIO bus
                  phy_status:  PHY status(link_up, speed, duplex)
 *  Output      : N/A
 *  Return      : N/A
 *  Note        : N/A
 ***********************************************************************/
int32
vsc8504_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t *phy_status)
{
    int32 ret = RESULT_OK;
    uint16 cfg_symmetric, cfg_asymmetric, lp_symmetric, lp_asymmetric;
    uint16 value;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    /* Get link status */
    ret += vsc8504_read(phy_handle, PHY_STATUS_REG, &value);
    phy_status->link_up = (value & 0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;
    /*if link down, need read again.*/
    if(phy_status->link_up == GLB_LINK_DOWN)
    {
        ret += vsc8504_read(phy_handle, PHY_STATUS_REG, &value);
        phy_status->link_up = (value & 0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;
    }

    if(phy_status->link_up == GLB_LINK_UP)
    {
        /* Get Speed */
        ret += vsc8504_read(phy_handle, AUX_PHY_STATUS_REG, &value);
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
            ret += vsc8504_read(phy_handle, AUTO_NEGO_LP_ADVERTI_REG, &value);
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
        ret += vsc8504_phy_page_ext(phy_handle);
        ret += vsc8504_read(phy_handle, 0x14, &value); /* Read Register Address 20E1 */
        ret += vsc8504_phy_page_std(phy_handle);

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
    }
    else
    {
        phy_status->speed = GLB_SPEED_AUTO;
        phy_status->duplex = GLB_DUPLEX_AUTO;
        phy_status->linkup_media = GLB_PORT_TYPE_AUTO;
        phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }
    //Need be optimized. jqiu.
    if(phy_handle->phy_info.phy_manage_info.media_type == GLB_MEDIA_PASS_THROUGH)
    {
        phy_status->speed = GLB_SPEED_1G;
        phy_status->duplex = GLB_DUPLEX_FULL;
    }
    return ret;
}

int32 vsc8504_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    vsc8504_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8504 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        vsc8504_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "VTSS8504 phy addr %d link down",pphy_info->phy_addr) 
        vsc8504_link_down_setting(phy_handle);
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
        if(pphy_info->phy_stat_flag.linkup_media!= phy_val->linkup_media)
        {
            phy_change->media_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change)|| (1==phy_change->media_change))
        {
            vsc8504_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
        }

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
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return RESULT_OK;

}

int32
vsc8504_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phyreg_param_t phyreg_para;
    phy_info_t *pphy_info;
    int32 ret = 0;
    uint16 val;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    /*if NO link change, return*/
    ret += vsc8504_read(phy_handle, PHY_INT_STATUS, &val);
    if(0 == val)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    if(PHY_WORK_MODE_PASSTHROUGH == pphy_info->phy_manage_info.mode)
    {
         return RESULT_OK;
    }
    ret += vsc8504_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"vsc8504_get_cur_status speed: %d duplex: %d",phy_val->speed, phy_val->duplex);
    
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

        ret += vsc8504_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        ret += vsc8504_link_down_setting(phy_handle);
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
        if(pphy_info->phy_stat_flag.linkup_media!= phy_val->linkup_media)
        {
            phy_change->media_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change)|| (1==phy_change->media_change))
        {
            ret += vsc8504_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
        }

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
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return ret;

}
int32
vsc8504_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8504 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = vsc8504_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

int32
vsc8504_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8504 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = vsc8504_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

static int32
vsc8504_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8504 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= vsc8504_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        break;

    case GLB_LB_PHY_FAR:
        ret |= vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= vsc8504_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0008, 0x0008);  /* Enable Far-End Loopback */
        break;

    case GLB_LB_PHY_NEAR:
        ret |= vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= vsc8504_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        break;

    /* Added by liuht for bug 27508, 2014-05-21 */
    case GLB_LB_PHY_NEAR_DIAG:
        ret |= vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= vsc8504_write_mask(phy_handle, EXTENDED_PHY_CTL1, 0x0000, 0x0008);  /* Disable Far-End Loopback */
        ret += vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
    /* End of liuht added */
        break;

    default:
        return RESULT_ERROR;
        break;
    }

    return ret;
}

int32
vsc8504_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8504 phy addr %d config medium mac_if %d media_if %d", pphy_info->phy_addr, mac_if, media_if);
    ret = vsc8504_phy_reset(phy_handle, mac_if, media_if, 0);
    /*After medium config, speed/duplex/loopback need re-config. speed/duplex has been done in phy_reset.*/
    if(GLB_LB_NONE != pphy_info->phy_manage_info.lb_mode)
    {
        ret |= vsc8504_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }
    /*Fix bug30023. After medium config, enable need re-config*/
    if(0 == pphy_info->phy_manage_info.enable)
    {
        ret |= vsc8504_enable(phy_handle, 0);
    }
    return ret;
}

int32
vsc8504_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
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
        "VSC8504 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += vsc8504_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}

/*Bug25277, support VCT.*/
int32 vsc8504_set_vct_mod(phy_handle_t* phy_handle)
{
    int32 ret = 0;

    /*fix bug40964, Testing vct will lead to hsrvd crash, liangf, 2016-09-29*/
    if(NULL == phy_handle || (!phy_handle->phy_info.vct_support))
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"vsc8504_set_vct_mod. ");
   
    ret = vtss8xx8_veriphy_task_start(phy_handle, 0);
    
    return ret;
}
/*Bug25277, support VCT.*/
int32 vsc8504_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
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
        vsc8504_enable(phy_handle, phy_handle->phy_info.phy_manage_info.enable);
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

int32 vsc8504_init(phy_handle_t* phy_handle)
{
    uint16 revision;
    phy_info_t* pphy_info;
    uint16 value;
    uint8 tmp;
    
    pphy_info = &phy_handle->phy_info;
    if(vsc8504_phy_detect(phy_handle, &revision) != VTSS_RC_OK)
        return RESULT_ERROR;
    phy_handle->phy_info.phy_revision = revision;
    if(GLB_PRE_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag)
    {
        vsc8504_phy_pre_reset(phy_handle, revision);
    }    if(GLB_DUAL_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag )
    {/* force pre init */
        tmp = pphy_info->phy_addr;
        pphy_info->phy_addr = 0;
        vsc8504_phy_pre_reset(phy_handle, revision);
        pphy_info->phy_addr = tmp;
    }
    vsc8504_phy_reset(phy_handle, phy_handle->phy_info.phy_manage_info.mac_if, phy_handle->phy_info.phy_manage_info.media_type, 1);
    vsc8504_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed, phy_handle->phy_info.phy_manage_info.duplex);
    if (GLB_POST_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag)
    {
        vsc8504_phy_post_reset(phy_handle);
    }
    /* No effect on the start of packet */
    vsc8504_phy_page_ext3(phy_handle);
    vsc8504_phy_wr_masked(phy_handle, 0x10, 0x0000, 0x0700);
    /* Enable PHY MAC serdes Auto-Negotiation */    
    vsc8504_phy_wr_masked(phy_handle, 0x10, 0x0080, 0x0080);
    vsc8504_phy_page_std(phy_handle);
    /*enable speed downshift when 1000M autonegation succ, but link fail*/
    vsc8504_phy_page_ext(phy_handle);
    vsc8504_phy_wr_masked(phy_handle, 0x14, 0x8010, 0x8010);
    vsc8504_phy_page_std(phy_handle);            
    /*set SIGDET to GPIO*/
    vsc8504_phy_page_gpio(phy_handle);
    vsc8504_phy_wr_masked(phy_handle, 0xd, 0xff, 0xff);
    vsc8504_phy_page_std(phy_handle);
    /*Clear interrupts*/
    vsc8504_read(phy_handle, PHY_INT_STATUS, &value);
    /* open interrupt detect */
    vsc8504_write(phy_handle, PHY_INT_MASK, 0xf000);
    /* Jqiu added. init to power down*/
    vsc8504_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
    /* added by tongzb for bug 42160, up the frequency of led 2016-10-16 */
    /* 2.5hz(0x000), 5hz(0x400,default), 10hz(0x800), 20hz(0xc00)*/
    /* added by tongzb, for config led freq, 2017-01-06 */
    if(GLB_PHY_LED_FREQ_DEFAULT == pphy_info->led_freq)
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_2P5 == pphy_info->led_freq)
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0x000, 0xc00);
    else if(GLB_PHY_LED_FREQ_5 == pphy_info->led_freq)
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    else if(GLB_PHY_LED_FREQ_10 == pphy_info->led_freq)
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0x800, 0xc00);
    else if(GLB_PHY_LED_FREQ_20 == pphy_info->led_freq)
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0xc00, 0xc00);
    else
        vsc8504_phy_wr_masked(phy_handle, PHY_LED_BEHAVIOR, 0x400, 0xc00);
    if (GLB_DUAL_INIT_FLAG == pphy_info->phy_manage_info.phy_init_seq_flag)
    {/* force post init */
        tmp = pphy_info->phy_addr;
        pphy_info->phy_addr = 3;
        vsc8504_phy_post_reset(phy_handle);
        pphy_info->phy_addr = tmp;
    }
    return RESULT_OK;
}

int32 vsc8504_config_phy_sig_test_mode(phy_handle_t* phy_handle, uint8 serdes_id, uint8 mode)
{
    uint16 value;

    if(mode == PHY_SIG_TEST_MODE_NORMAL)
    {
        vsc8504_phy_page_ext2(phy_handle);
        vsc8504_write_mask(phy_handle, 17, 0x0000, 0x0020);
        vsc8504_phy_page_std(phy_handle);
        vsc8504_write_mask(phy_handle, 9, 0x0000, 0xf800);
    }
    else
    {
        /*set 17E2 bit 5=1*/
        vsc8504_phy_page_ext2(phy_handle);
        vsc8504_write_mask(phy_handle, 17, 0x0020, 0x0020);
        vsc8504_phy_page_std(phy_handle);
        /*set 0 bit 6, 13 = 10*/
        vsc8504_write_mask(phy_handle, 17, 0x0040, 0x2040);
        /*set 9 bit 15-13, select mode*/        
        if(mode == PHY_SIG_TEST_MODE_1)
            value = 0x2000;
        else if(mode == PHY_SIG_TEST_MODE_2)
            value = 0x4000;
        else if(mode == PHY_SIG_TEST_MODE_3)
            value = 0x6000;
        else 
            value = 0;
        vsc8504_write_mask(phy_handle, 9, value, 0xe000);
    }
    return RESULT_OK;
}


phy_handle_t* phy_vsc8504_dev_register(phy_info_t* pphy_info)
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
    phdl->phy_init = vsc8504_init;
    phdl->config_enable = vsc8504_enable;
    phdl->config_speed = vsc8504_config_speed;
    phdl->config_duplex = vsc8504_config_duplex;
    phdl->config_loopback = vsc8504_config_loopback;
    phdl->config_medium = vsc8504_config_medium;
    phdl->config_flowctrl = vsc8504_config_flowctrl;
    phdl->get_link_poll = vsc8504_get_link_poll;
    phdl->get_link_interupt = vsc8504_get_link_interrput;
    phdl->reg_read = vsc8504_reg_read;
    phdl->reg_write = vsc8504_reg_write;
    phdl->phy_set_vct_mod = vsc8504_set_vct_mod;
    phdl->phy_get_vct_info = vsc8504_get_vct_info;
    phdl->config_phy_sig_test_mode = vsc8504_config_phy_sig_test_mode;
    
    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type  = pphy_info->mdio_bus;
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







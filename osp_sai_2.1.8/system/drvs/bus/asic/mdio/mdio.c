/****************************************************************************
* $Id$
* mdio bus driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-08-18
* Reason        : First Create.
****************************************************************************/
#include "mdio.h"
#include "epld_api.h"
#include "drv_debug.h"
#include "sal_usrctrl.h"
#include "ctc_hw.h"
#include "ctc_api.h"

#define ASIC_MDIO_CMD           0x80
#define ASIC_MDIO_CMD_STATUS    0x2c4
#define ASIC_MDIO_RD_DATA       0x18

int32 fpga_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{

    return RESULT_OK;    
}

int32 fpga_gephy_mdio_write(mdio_handle_t *phdl, mdio_bus_para_t *ppara)
{

    return RESULT_OK;    
}

int32 fpga_xgphy_mdio_read(mdio_handle_t *phdl, mdio_bus_para_t *ppara)
{

    return RESULT_OK;    
}

int32 fpga_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{

    return RESULT_OK;    
}

int32 aisc_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    uintptr_t base_addr;
    uint8 phy_addr;
    uint8 reg_offset;
    uint32 rdreg = 0;     /* register working value */
    
    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    base_addr = phdl->data.base_addr;
    phy_addr = ppara->gephy_para.phy_addr;
    reg_offset = ppara->gephy_para.reg;

    mdio_cmd_union_t mdio_cmd;
    uint32 cmd_done;
    uint32 timeout = 256;
            
    mdio_cmd.field.mdiocmd_st = MDIO_ST_1G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_1G_READ;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = reg_offset;
    mdio_cmd.field.mdiocmd_data = 0xffff;
    mdio_cmd.field.reserved = 0x0;

    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr+ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));    
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }

    ret = ctckal_usrctrl_write_bay(0, 0, base_addr+ASIC_MDIO_CMD, mdio_cmd.val);
    if(ret < 0)
    {
        return ret;
    }
    
    timeout = 256;
    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr+ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
    
    ret = ctckal_usrctrl_read_bay(0, 0, base_addr+ASIC_MDIO_RD_DATA, (uintptr_t)(&rdreg));    
    if(ret < 0)
    {
        return ret;
    }
    ppara->gephy_para.val = (uint16_t)rdreg;

    return RESULT_OK;    
}

int32 aisc_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    uintptr_t base_addr;
    uint8 phy_addr;
    uint8 reg_offset;
    uint16 value;
    
    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    base_addr = phdl->data.base_addr;
    phy_addr = ppara->gephy_para.phy_addr;
    reg_offset = ppara->gephy_para.reg;
    value = ppara->gephy_para.val;

    mdio_cmd_union_t mdio_cmd;
    uint32 cmd_done;
    uint32 timeout = 256;
            
    mdio_cmd.field.mdiocmd_st = MDIO_ST_1G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_1G_WRITE;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = reg_offset;
    mdio_cmd.field.mdiocmd_data = value;
    mdio_cmd.field.reserved = 0x0;

    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr+ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }

    
    return ctckal_usrctrl_write_bay(0, 0, base_addr+ASIC_MDIO_CMD, mdio_cmd.val);
}

int32 aisc_xgphy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    mdio_cmd_union_t mdio_cmd;
    int32 ret = 0;
    uintptr_t base_addr;    
    uint16 reg_offset;
    uint8 phy_addr;
    uint8 devtype;
    uint32 rdreg;     /* register working value */
    uint32 cmd_done;
    uint32 timeout = 256;
    
    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    base_addr = phdl->data.base_addr;
    phy_addr = ppara->xgphy_para.phy_addr;
    devtype = ppara->xgphy_para.devno;
    reg_offset = ppara->xgphy_para.reg;

    //DRV_LOG_DEBUG(DRV_MDIO, "ASIC read: base addr:0x%x  phy addr: 0x%x %d.%xh",
    //                            base_addr, phy_addr, devtype, reg_offset);
    /*send addr*/
    mdio_cmd.field.mdiocmd_st = MDIO_ST_10G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_10G_ADDR;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = devtype;
    mdio_cmd.field.mdiocmd_data = reg_offset;
    mdio_cmd.field.reserved = 0x0;

    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));    
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
    ret = ctckal_usrctrl_write_bay(0, 0, base_addr + ASIC_MDIO_CMD, mdio_cmd.val);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write ASIC MDIO cmd failed\n");
        return ret;
    }
    /*send read command*/
    mdio_cmd.field.mdiocmd_st = MDIO_ST_10G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_10G_READ;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = devtype;
    mdio_cmd.field.mdiocmd_data = 0xffff;
    mdio_cmd.field.reserved = 0x0;
    timeout = 256;

    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));    
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
     
    ret = ctckal_usrctrl_write_bay(0, 0, base_addr + ASIC_MDIO_CMD, mdio_cmd.val);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write ASIC MDIO cmd failed\n");
        return ret;
    }
         
    /*read  value*/    
    timeout = 256;
    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
    ret = ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_RD_DATA, (uintptr_t)(&rdreg));    
    if(ret < 0)
    {
        DRV_LOG_ERR("Read ASIC MDIO cmd failed\n");
        return ret;
    }
    
    ppara->xgphy_para.val = (uint16_t)rdreg;

    return RESULT_OK;    
}

int32 aisc_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    uintptr_t base_addr;    
    uint16 reg_offset;
    uint8 phy_addr;
    uint8 devtype;
    uint16 value;
    uint32 cmd_done;
    uint32 timeout = 256;
    mdio_cmd_union_t mdio_cmd;
    
    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    base_addr = phdl->data.base_addr;
    phy_addr = ppara->xgphy_para.phy_addr;
    devtype = ppara->xgphy_para.devno;
    reg_offset = ppara->xgphy_para.reg;
    value = ppara->xgphy_para.val;

    /*send addr*/
    mdio_cmd.field.mdiocmd_st = MDIO_ST_10G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_10G_ADDR;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = devtype;
    mdio_cmd.field.mdiocmd_data = reg_offset;
    mdio_cmd.field.reserved = 0x0; 

    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));        
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
    
    ret = ctckal_usrctrl_write_bay(0, 0, base_addr + ASIC_MDIO_CMD, mdio_cmd.val);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write ASIC MDIO cmd failed\n");
        return ret;
    }
    
    /*send data*/
    mdio_cmd.field.mdiocmd_st = MDIO_ST_10G;
    mdio_cmd.field.mdiocmd_op = MDIO_OP_10G_WRITE;
    mdio_cmd.field.mdiocmd_prtad = phy_addr;
    mdio_cmd.field.mdiocmd_devad = devtype;
    mdio_cmd.field.mdiocmd_data = value;
    mdio_cmd.field.reserved = 0x0; 

    timeout = 256;
    do {
        ctckal_usrctrl_read_bay(0, 0, base_addr + ASIC_MDIO_CMD_STATUS, (uintptr_t)(&cmd_done));
    } while((!cmd_done)&&(timeout--));        
    if (!timeout)
    {
        DRV_LOG_ERR("ASIC MDIO busy, timeout!\n");
        return RESULT_ERROR;
    }
    
    ret = ctckal_usrctrl_write_bay(0, 0, base_addr + ASIC_MDIO_CMD, mdio_cmd.val);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write ASIC MDIO cmd failed\n");
        return ret;
    }
    return RESULT_OK;    
}

int mdio_hw_fd = -1;
int32 cpu_gephy_mdio_init()
{
    /*it means has been inited*/
    if(mdio_hw_fd != -1)
        return RESULT_OK;
    mdio_hw_fd = open(CTC_HW_DEV_NAME, O_RDWR|O_SYNC);
    if (mdio_hw_fd < 0)
    {
        DRV_LOG_ERR("opening %s char device failed.", CTC_HW_DEV_NAME);
        return RESULT_ERROR;
    }
    return RESULT_OK;    
}

int32 cpu_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    hw_cpu_mdio_cfg_ioctl_t gephy_info;

    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    gephy_info.phy_addr    = ppara->gephy_para.phy_addr;
    gephy_info.reg         = ppara->gephy_para.reg;
    gephy_info.value       = 0;

    ret = ioctl(mdio_hw_fd, CTC_CPU_MDIO_READ, (uintptr_t)&gephy_info);
    if(ret < 0)
    {
        DRV_LOG_ERR("Read GEPHY failed\n");
        return ret;
    }
    ppara->gephy_para.val = gephy_info.value;

    return RESULT_OK;    
}

int32 cpu_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{    
    int32 ret = 0;
    hw_cpu_mdio_cfg_ioctl_t gephy_info;

    if (NULL == phdl || NULL == ppara)
    {
        return RESULT_ERROR;
    }

    gephy_info.phy_addr    = ppara->gephy_para.phy_addr;
    gephy_info.reg         = ppara->gephy_para.reg;
    gephy_info.value       = ppara->gephy_para.val;

    ret = ioctl(mdio_hw_fd, CTC_CPU_MDIO_WRITE, (uintptr_t)&gephy_info);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write GEPHY failed\n");
        return ret;
    }

    return RESULT_OK;    
}

int32 asic_gb_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data; 
    
    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_GE, &mdio_data);
    ppara->gephy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gb_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data; 
    
    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    mdio_data.value = ppara->gephy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_GE, &mdio_data);    
    if (ret < 0)
    {
        return ret;
    }

    return RESULT_OK;
}

int32 asic_gb_xgphy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XG, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gb_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XG, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gb_gephy_xgmdio_read(uint8 bus, uint8 phy_addr, uint16 reg, uint8 dev, uint16* value)
{
#if 0/*Commented by xgu for compile pass, 2014-4-21*/
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = bus;
    mdio_data.phy_addr = phy_addr;
    mdio_data.reg = reg;
    mdio_data.dev_no = dev;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    *value = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
#else
    return -1;
#endif
    return RESULT_OK;
}

int32 asic_gb_gephy_xgmdio_write(uint8 bus, uint8 phy_addr, uint16 reg, uint8 dev, uint16 value)
{
#if 0/*Commented by xgu for compile pass, 2014-4-21*/
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = bus;
    mdio_data.phy_addr = phy_addr;
    mdio_data.reg = reg;
    mdio_data.dev_no = dev;
    mdio_data.value = value;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
#else
    return -1;
#endif
    return RESULT_OK;
}

int32 asic_gb_xg_by_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;    
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gb_xg_by_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.bus = phdl->data.base_addr;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;

}
/*fix bug36076, support SG8348_48T46X, liangf, 2015-11-28*/
int32 asic_gg_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 1:0;
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_GE, &mdio_data);
    ppara->gephy_para.val = mdio_data.value;
    if (ret < 0)
    {
        DRV_LOG_ERR("GG read gephy failed. ret:%d", ret);
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gg_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 1:0;
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    mdio_data.value = ppara->gephy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_GE, &mdio_data);
    if (ret < 0)
    {
        DRV_LOG_ERR("GG write gephy failed. ret:%d", ret);
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gg_xgphy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 3:2;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XG, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gg_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 3:2;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XG, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gg_xgphy_gemdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 1:0;    
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_gg_xgphy_gemdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = phdl->data.base_addr>1 ? 1:0;
    mdio_data.bus = phdl->data.base_addr%2 ? 1:0;
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XGREG_BY_GE, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_d2_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;      
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_GE, &mdio_data);
    ppara->gephy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_d2_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    mdio_data.value = ppara->gephy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_GE, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_d2_xgphy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XG, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_d2_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XG, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_tm_gephy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = ((phdl->data.base_addr)&0xf)%4;    
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;      
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_GE, &mdio_data);
    ppara->gephy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_tm_gephy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = ((phdl->data.base_addr)&0xf)%4;      
    mdio_data.phy_addr = ppara->gephy_para.phy_addr;
    mdio_data.reg = ppara->gephy_para.reg;
    mdio_data.value = ppara->gephy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_GE, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_tm_xgphy_mdio_read(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;        
    ret = ctc_chip_mdio_read(0, CTC_CHIP_MDIO_XG, &mdio_data);
    ppara->xgphy_para.val = mdio_data.value;
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

int32 asic_tm_xgphy_mdio_write(mdio_handle_t* phdl, mdio_bus_para_t* ppara)
{
    int32 ret = 0;
    ctc_chip_mdio_para_t mdio_data;

    //mdio_data.ctl_id = (phdl->data.base_addr>>4)&0xf;
    mdio_data.bus = phdl->data.base_addr;    
    mdio_data.phy_addr = ppara->xgphy_para.phy_addr;
    mdio_data.reg = ppara->xgphy_para.reg;
    mdio_data.dev_no = ppara->xgphy_para.devno;
    mdio_data.value = ppara->xgphy_para.val;
    ret = ctc_chip_mdio_write(0, CTC_CHIP_MDIO_XG, &mdio_data);
    if (ret < 0)
    {
        return ret;
    }
    return RESULT_OK;
}

mdio_handle_t* mdio_create_handle(mdio_info_t* pinfo)
{
    mdio_handle_t* phdl;
    
    if(NULL == pinfo)
    {
        DRV_LOG_WARN("Invalid arguments.\n");
        return NULL;
    }
    phdl = (mdio_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_MDIO_INFO, sizeof(mdio_handle_t));
    if (NULL == phdl)
    {
        DRV_LOG_ERR("malloc failed.\n");
        return NULL;
    }    
    memcpy(&phdl->data, pinfo, sizeof(mdio_info_t));

    switch(pinfo->bus_type)
    {
        case FPGA_PHY_1G_TYPE:
            phdl->read = fpga_gephy_mdio_read;
            phdl->write = fpga_gephy_mdio_write;
            break;
        case FPGA_PHY_10G_TYPE:
            phdl->read = fpga_xgphy_mdio_read;
            phdl->write = fpga_xgphy_mdio_write;         
            break;
        case ASIC_PHY_1G_TYPE:
            phdl->read = aisc_gephy_mdio_read;
            phdl->write = aisc_gephy_mdio_write;
            break;
        case ASIC_PHY_10G_TYPE:
            phdl->read = aisc_xgphy_mdio_read;
            phdl->write = aisc_xgphy_mdio_write;
            break;
        case CPU_PHY_1G_TYPE:
            cpu_gephy_mdio_init();
            phdl->read = cpu_gephy_mdio_read;
            phdl->write = cpu_gephy_mdio_write;
            break;
        case ASIC_GB_PHY_10G_BY_1G_TYPE:
            phdl->read =  asic_gb_xg_by_gephy_mdio_read;
            phdl->write = asic_gb_xg_by_gephy_mdio_write;
            break;
        case ASIC_GB_PHY_1G_TYPE:
            phdl->read = asic_gb_gephy_mdio_read;
            phdl->write = asic_gb_gephy_mdio_write;
            break;
        case ASIC_GB_PHY_10G_TYPE:
            phdl->read = asic_gb_xgphy_mdio_read;
            phdl->write = asic_gb_xgphy_mdio_write;
            break;
        case ASIC_GG_PHY_1G_TYPE:
            phdl->read = asic_gg_gephy_mdio_read;
            phdl->write = asic_gg_gephy_mdio_write;
            break;
        case ASIC_GG_PHY_10G_TYPE:
            phdl->read = asic_gg_xgphy_mdio_read;
            phdl->write = asic_gg_xgphy_mdio_write;
            break;
        case ASIC_GG_PHY_1G_10G_TYPE:
            phdl->read = asic_gg_xgphy_gemdio_read;
            phdl->write = asic_gg_xgphy_gemdio_write;
            break;
        case ASIC_D2_PHY_1G_TYPE:
            phdl->read = asic_d2_gephy_mdio_read;
            phdl->write = asic_d2_gephy_mdio_write;
            break;
        case ASIC_D2_PHY_10G_TYPE:
            phdl->read = asic_d2_xgphy_mdio_read;
            phdl->write = asic_d2_xgphy_mdio_write;
            break;
        case ASIC_TM_PHY_1G_TYPE:
            phdl->read = asic_tm_gephy_mdio_read;
            phdl->write = asic_tm_gephy_mdio_write;
            break;
        case ASIC_TM_PHY_10G_TYPE:
            phdl->read = asic_tm_xgphy_mdio_read;
            phdl->write = asic_tm_xgphy_mdio_write;
            break;
        default:
            DRV_FREE(CTCLIB_MEM_DRIVER_MDIO_INFO, phdl);
            DRV_LOG_ERR("Invalid MDIO bus type %d.\n");
            phdl = NULL;
            return NULL;
    }
    
     return phdl;

}

int32 cpu_debug_gephy_mdio_read(unsigned int phy_addr, unsigned int reg, unsigned int* value)
{
    int32 ret = 0;
    hw_cpu_mdio_cfg_ioctl_t gephy_info;

    gephy_info.phy_addr    = phy_addr;
    gephy_info.reg         = reg;
    gephy_info.value       = 0;

    ret = ioctl(mdio_hw_fd, CTC_CPU_MDIO_READ, (uintptr_t)&gephy_info);
    if(ret < 0)
    {
        DRV_LOG_ERR("Read GEPHY failed\n");
        return ret;
    }
    *value = gephy_info.value;

    return RESULT_OK;    
}

int32 cpu_debug_gephy_mdio_write(unsigned int phy_addr, unsigned int reg, unsigned int value)
{    
    int32 ret = 0;
    hw_cpu_mdio_cfg_ioctl_t gephy_info;

    gephy_info.phy_addr    = phy_addr;
    gephy_info.reg         = reg;
    gephy_info.value       = value;

    ret = ioctl(mdio_hw_fd, CTC_CPU_MDIO_WRITE, (uintptr_t)&gephy_info);
    if(ret < 0)
    {
        DRV_LOG_ERR("Write GEPHY failed\n");
        return ret;
    }

    return RESULT_OK;    
}



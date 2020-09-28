/****************************************************************************
* $Id$
* fiber module driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-09-9
* Reason        : First Create.
****************************************************************************/
#include "fiber_drv.h"
#include "drv_specific.h"
#include "drv_debug.h"
#include "glb_phy_define.h"
#include "sal_common.h"
//#include "phy_api.h"
#include "epld_api.h"
#include "gpio_api.h"
#include "ctc_i2c.h"
#include "ctc_api.h"

int32 fiber_ctc_chip_read(fiber_handle_t* pfiberhdl, fiber_para_t* paccess)
{
    int32 ret = 0;
    uint32 slave_bitmap;
    ctc_chip_i2c_read_t sfp_para;
    
    if(NULL == pfiberhdl || NULL == pfiberhdl->data || NULL == paccess || NULL == paccess->val)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }
    sal_memset(&sfp_para, 0, sizeof(ctc_chip_i2c_read_t));

#if (defined GOLDENGATE) || (defined DUET2) || (defined TSINGMA)
    slave_bitmap = ((fiber_ctc_gen_t *)(pfiberhdl->data))->slave_bitmap;
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    
    sfp_para.slave_dev_id = slave_bitmap; 
    sfp_para.slave_bitmap = 0; 
    sfp_para.lchip= ((fiber_ctc_gen_t *)(pfiberhdl->data))->lchip;
    sfp_para.ctl_id = ((fiber_ctc_gen_t *)(pfiberhdl->data))->bus_id;
    sfp_para.dev_addr = paccess->subdev;        
    sfp_para.offset = paccess->offset;
    sfp_para.length = paccess->len;
    sfp_para.buf_length = paccess->len;
    sfp_para.p_buf = paccess->val;
    sfp_para.i2c_switch_id = 0xf; /* when access_switch is 1 this no useful */ 
    sfp_para.access_switch = 0;
#else
    slave_bitmap = ((fiber_ctc_gen_t *)(pfiberhdl->data))->slave_bitmap;
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    
    sfp_para.slave_bitmap = (1<<slave_bitmap);    
    sfp_para.dev_addr = paccess->subdev;        
    sfp_para.offset = paccess->offset;
    sfp_para.length = paccess->len;
    sfp_para.p_buf = paccess->val;
    /* for bug 45745*/
    sfp_para.lchip= ((fiber_ctc_gen_t *)(pfiberhdl->data))->lchip;
#endif

    ret = ctc_chip_i2c_read(&sfp_para);
    if (ret < 0)
    {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

int32 fiber_ctc_chip_write(fiber_handle_t* phdl, fiber_para_t* para)
{
    int32 ret = 0;
    uint32 slave_bitmap;
    ctc_chip_i2c_write_t sfp_para;
    uint8 i;

    if(NULL == phdl || NULL == phdl->data || NULL == para || NULL == para->val)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }
    sal_memset(&sfp_para, 0, sizeof(ctc_chip_i2c_write_t));

#if (defined GOLDENGATE) || (defined DUET2) || (defined TSINGMA)
    slave_bitmap = ((fiber_ctc_gen_t *)(phdl->data))->slave_bitmap;
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    sfp_para.dev_addr = para->subdev;
    sfp_para.slave_id = slave_bitmap;
    sfp_para.lchip= ((fiber_ctc_gen_t *)(phdl->data))->lchip;
    sfp_para.ctl_id = ((fiber_ctc_gen_t *)(phdl->data))->bus_id;
    sfp_para.i2c_switch_id = 0xf; /* when access_switch is 1 this no useful */   
    sfp_para.access_switch = 0;/* 1 means disable i2c bridge */
#else
    slave_bitmap = *((uint32 *)phdl->data);
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    sfp_para.dev_addr = para->subdev;
    sfp_para.slave_id = slave_bitmap;
    /* for bug 45745*/
    sfp_para.lchip= ((fiber_ctc_gen_t *)(phdl->data))->lchip;    
#endif
    for(i=0; i<para->len; i++)
    {
        /*For SFP internal PHY, write 16 bits need do two i2c write, and addr must be same*/
        if(sfp_para.dev_addr == FIBER_DEV_ADDR3)
        {
            sfp_para.offset = para->offset;
        }
        else
        {
            sfp_para.offset = para->offset+i;
        }
        sfp_para.data = para->val[i];
        
        ret = ctc_chip_i2c_write(&sfp_para);
        if (ret < 0)
        {
            return RESULT_ERROR;
        }    
    }
    return RESULT_OK;
}

int32 fiber_i2c_write(fiber_handle_t* pfiberhdl, fiber_para_t* paccess)
{
    i2c_handle_t* i2c_hdl = NULL;
    i2c_op_para_t op_para;
    int32 ret;
    
    if(!pfiberhdl || !paccess)
    {
        DRV_LOG_ERR("FIBER write: Invalid parameter");
        return RESULT_ERROR;
    }
    
    if(FIBER_DEV_ADDR1 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev1;
    }
    else if(FIBER_DEV_ADDR2 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev2;
    }
    else if(FIBER_DEV_ADDR3 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev3;
    }
    if(!i2c_hdl || !i2c_hdl->write)
    {
        DRV_LOG_ERR("FIBER write: Invalid parameter");
        return RESULT_ERROR;
    }

    op_para.offset = paccess->offset;
    op_para.len = paccess->len;   
    op_para.p_val = paccess->val;
        
    ret = i2c_hdl->write(i2c_hdl, &op_para);
    if(ret<0)
    {
        DRV_LOG_ERR("FIBER write: i2c write failed");
        return RESULT_ERROR;
    }

    return RESULT_OK;

}


int32 fiber_i2c_read(fiber_handle_t* pfiberhdl, fiber_para_t* paccess)
{
    i2c_handle_t* i2c_hdl = NULL;
    i2c_op_para_t op_para;
    int32 ii,ret;
    uint8* tmpval_8;
    
    if(!pfiberhdl || !paccess)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }
    
    if(FIBER_DEV_ADDR1 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev1;
    }
    else if(FIBER_DEV_ADDR2 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev2;
    }
    else if(FIBER_DEV_ADDR3 == paccess->subdev)
    {
        i2c_hdl = pfiberhdl->phdl_dev3;
    }
    if(!i2c_hdl || !i2c_hdl->read)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }

    op_para.offset = paccess->offset;
    op_para.len = paccess->len;
    tmpval_8 = DRV_MALLOC(CTCLIB_MEM_DRIVER_FIBER_INFO, op_para.len);
    if(!tmpval_8)
    {
        DRV_LOG_ERR("FIBER read: out of memory");
        return RESULT_ERROR;
    }
    op_para.p_val = tmpval_8;
        
    ret = i2c_hdl->read(i2c_hdl, &op_para);
    if(ret<0)
    {
        /* remove for bug 46734, when fiber do not insert we will get error return but do not print error info */
        //DRV_LOG_ERR("FIBER read: i2c read failed");
        DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, tmpval_8);
        return RESULT_ERROR;
    }
    for(ii = 0; ii < op_para.len; ii++)
    {
        paccess->val[ii] = tmpval_8[ii];
        //DRV_LOG_DEBUG(DRV_FIBER, "fiber_i2c_read offset 0x%x 0x%x", op_para.offset + ii, tmpval_8[ii]);

    }
    
    DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, tmpval_8);
    
    return RESULT_OK;

}

/*****************************************************************************
 * Name        : fiber_ctc_chip_epld_read
 * Purpose     : access fiber by Centec Switch ASIC I2C master and chip select 
               : by epld
 * Input       : fiber_handle_t - pfiberhdl
               : fiber_para_t   - paccess
 * Output      : N/A
 * Return      : SUCCESS
 * Note        :
*****************************************************************************/
int32 fiber_ctc_chip_epld_read(fiber_handle_t* pfiberhdl, fiber_para_t* paccess)
{
    int32 ret = 0;
    uint32 slave_bitmap;
    ctc_chip_i2c_read_t sfp_para;
    
    if(NULL == pfiberhdl || NULL == pfiberhdl->data || NULL == paccess || NULL == paccess->val)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }
    sal_memset(&sfp_para, 0, sizeof(ctc_chip_i2c_read_t));

    slave_bitmap = ((fiber_ctc_gen_t *)(pfiberhdl->data))->slave_bitmap;
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    
    sfp_para.slave_dev_id = 1;  /* useless */
    sfp_para.slave_bitmap = 0;  /* useless */
    sfp_para.lchip= ((fiber_ctc_gen_t *)(pfiberhdl->data))->lchip;
    sfp_para.ctl_id = ((fiber_ctc_gen_t *)(pfiberhdl->data))->bus_id;
    sfp_para.dev_addr = paccess->subdev;        
    sfp_para.offset = paccess->offset;
    sfp_para.length = paccess->len;
    sfp_para.buf_length = paccess->len;
    sfp_para.p_buf = paccess->val;
    sfp_para.i2c_switch_id = 0; /* when access_switch is 1 this no useful */
    sfp_para.access_switch = 0;
    /* 1. disable all i2c channel by epld */
    epld_item_write(0, EPLD_FIBER_I2C_SCL_SWITCH1, 0);
    epld_item_write(0, EPLD_FIBER_I2C_SCL_SWITCH2, 0);
    /* 2. enable one channel */
    epld_item_write(0, ((fiber_ctc_gen_t *)(pfiberhdl->data))->epld_switch_reg, 1<<slave_bitmap);

    /* 2. access fiber by ctc chip */
    ret = ctc_chip_i2c_read(&sfp_para);
    if (ret < 0)
    {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

/*****************************************************************************
 * Name        : fiber_ctc_chip_epld_write
 * Purpose     : access fiber by Centec Switch ASIC I2C master and chip select 
               : by epld
 * Input       : fiber_handle_t - pfiberhdl
               : fiber_para_t   - paccess
 * Output      : N/A
 * Return      : SUCCESS
 * Note        :
*****************************************************************************/
int32 fiber_ctc_chip_epld_write(fiber_handle_t* phdl, fiber_para_t* para)
{
    int32 ret = 0;
    uint32 slave_bitmap;
    ctc_chip_i2c_write_t sfp_para;
    uint8 i;

    if(NULL == phdl || NULL == phdl->data || NULL == para || NULL == para->val)
    {
        DRV_LOG_ERR("FIBER read: Invalid parameter");
        return RESULT_ERROR;
    }
    sal_memset(&sfp_para, 0, sizeof(ctc_chip_i2c_write_t));
    slave_bitmap = ((fiber_ctc_gen_t *)(phdl->data))->slave_bitmap;
    if (slave_bitmap < 0)
    {
        return RESULT_ERROR;
    }
    sfp_para.dev_addr = para->subdev;
    sfp_para.slave_id = 1;   /* useless */
    sfp_para.lchip= ((fiber_ctc_gen_t *)(phdl->data))->lchip;
    sfp_para.ctl_id = ((fiber_ctc_gen_t *)(phdl->data))->bus_id;
    sfp_para.i2c_switch_id = 0; /* when access_switch is 1 this no useful */  
    sfp_para.access_switch = 0;
    /* 1. disable all i2c channel by epld */
    epld_item_write(0, EPLD_FIBER_I2C_SCL_SWITCH1, 0);
    epld_item_write(0, EPLD_FIBER_I2C_SCL_SWITCH2, 0);
    /* 2. enable one channel */
    epld_item_write(0, ((fiber_ctc_gen_t *)(phdl->data))->epld_switch_reg, 1<<slave_bitmap);

    /* 2. access fiber by ctc chip */
    for(i=0; i<para->len; i++)
    {
        /*For SFP internal PHY, write 16 bits need do two i2c write, and addr must be same*/
        if(sfp_para.dev_addr == FIBER_DEV_ADDR3)
        {
            sfp_para.offset = para->offset;
        }
        else
        {
            sfp_para.offset = para->offset+i;
        }
        sfp_para.data = para->val[i];
        
        ret = ctc_chip_i2c_write(&sfp_para);
        if (ret < 0)
        {
            return RESULT_ERROR;
        }    
    }
    return RESULT_OK;
}

#if 0
int32 fiber_epld_get_present_info(uint32 fiber_id, fiber_handle_t* phdl, uint32* present)
{
    
    epld_get_fiber_present(fiber_id, present);
    
    return RESULT_OK;
}

/* raw value 0 means present.*/
int32 fiber_gpio_get_present_info(uint32 fiber_id, fiber_handle_t* phdl, uint32* present)
{
    int32 ret;
    uint8 present_chip, present_no, value;
    
    present_chip = ((fiber_ctc_gen_t *)(phdl->data))->present_chip;
    present_no = ((fiber_ctc_gen_t *)(phdl->data))->present_no;
    
    ret = gpio_get_scan_special_bit_value(present_chip, present_no, &value);
    if(value)
    {
        *present = 0;
    }
    else
    {
        *present = 1;
    }
    return ret;
}
#endif
int32 fiber_get_present_info(uint32 fiber_id, fiber_handle_t* phdl, uint32* present)
{
    int ret = 0;
    uint8 val = 0;
    fiber_para_t sfp_para;

    sfp_para.subdev = FIBER_DEV_ADDR1;   /* Fiber basic information */
    sfp_para.offset = 1;    
    sfp_para.len = 1;
    sfp_para.val = &val;

    ret = phdl->read(phdl, &sfp_para);  
    if(ret < 0)
    {
        *present = 0;
    }
    else
    {
        if(val == 0xff)
        {
            *present = 0;
        }
        else
        {
            *present = 1;
        }
    }
    return RESULT_OK;
}

int32 fiber_epld_enable(uint32 fiber_id, fiber_handle_t* phdl, uint32 enable)
{
    /* now enable fiber by fiber table with epld info in board file, so remove this */
    //epld_set_sfp_enable(fiber_id, enable);
    epld_para_t para;
    uint8 enable_no,epld_index;
    
    para.addr = ((fiber_ctc_gen_t *)(phdl->data))->enable_reg;
    
    epld_index = ((fiber_ctc_gen_t *)(phdl->data))->enable_chip;
    enable_no = ((fiber_ctc_gen_t *)(phdl->data))->enable_no;
    
    epld_read(epld_index, &para);
    if(!enable)
    {
        para.val |= 0x1<<enable_no;
    }
    else
    {
        para.val &= ~(0x1<<enable_no);
    }
    epld_write(epld_index, &para);
    
    return RESULT_OK;
}

/*raw value 1 means disable*/
int32 fiber_gpio_enable(uint32 fiber_id, fiber_handle_t* phdl, uint32 enable)
{
    int32 ret;
    uint8 enable_chip, enable_no, value;
    
    enable_chip = ((fiber_ctc_gen_t *)(phdl->data))->enable_chip;
    enable_no = ((fiber_ctc_gen_t *)(phdl->data))->enable_no;

    if(enable)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }
    ret = gpio_set_special_bit(enable_chip, enable_no, value);
    
    return ret;
}

int32 fiber_ctc_qsfp_enable(uint32 fiber_id, fiber_handle_t* phdl, uint32 enable)
{
    int ret = 0;
    uint8 val = 0;
    uint8 channel;
    fiber_para_t sfp_para;

    if(phdl->run_info.is_coper)
    {
        return RESULT_OK;
    }

    sfp_para.subdev = FIBER_DEV_ADDR1;   /* Fiber basic information */
    sfp_para.offset = 0x56;    
    sfp_para.len = 1;
    sfp_para.val = &val;

    ret = phdl->read(phdl, &sfp_para);
    if(ret < 0)
    {
       return RESULT_ERROR;
    }

    channel = ((fiber_ctc_gen_t *)(phdl->data))->channel;
    if((channel>4) || (channel<0))
    {
        return RESULT_ERROR;
    }

    if(channel)//4 X 10G
    {
        if(enable == TRUE)
        {
            val &= ~(1<<(channel-1));
        }
        else
        {
            val |= (1<<(channel-1));
        }
    }
    else
    {
        if(enable == TRUE)
        {
            val &= 0xf0;
        }
        else
        {
            val |= 0x0f;
        }
    }
    
    sfp_para.subdev = FIBER_DEV_ADDR1;   /* Fiber basic information */
    sfp_para.offset = 0x56;    
    sfp_para.len = 1;
    sfp_para.val = &val;
    ret = phdl->write(phdl, &sfp_para);
    if(ret < 0)
    {
       return RESULT_ERROR;
    }
    /* delay 40ms according SFF8436 */
    usleep(40000);
    
    return RESULT_OK;
}

fiber_handle_t* fiber_create_handle(uint32 port_id, fiber_port_info_t* p_info)
{
    //phy_handle_t* p_phyhdl;
    i2c_handle_t* i2c_hdl_dev1;
    i2c_handle_t* i2c_hdl_dev2;
    i2c_handle_t* i2c_hdl_dev3;
    fiber_handle_t* phdl;
    i2c_gen_t i2c_gen;
    fiber_ctc_gen_t *para;
    
    if(NULL == p_info)
    {
        DRV_LOG_ERR("Fiber: Invalid parameter");
        return NULL;
    }
    
    phdl = DRV_MALLOC(CTCLIB_MEM_DRIVER_FIBER_INFO, sizeof(fiber_handle_t));
    if(NULL == phdl)
    {
        DRV_LOG_ERR("Fiber: out of memory");
        return NULL;
    }
    phdl->data = (fiber_ctc_gen_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_FIBER_INFO ,sizeof(fiber_ctc_gen_t));
    if (NULL == phdl->data)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, phdl);
        return NULL;
    }
    para = ((fiber_ctc_gen_t *)(phdl->data));

    switch(p_info->mode)
    {
        case E_FIBER_ASIC_ASIC:
            para->channel      = p_info->fiber_channel;
            para->bus_id       = p_info->bus_id;
            para->lchip        = p_info->cs.asic.lchip;
            para->slave_bitmap = p_info->cs.asic.slave_bitmap;
#ifdef GREATBELT
            /*For bitmap 24~31, GB default is gpio mode, need set to SCL mode GB GPIO 4~11 map to SCL 24-31 */
            if(para->slave_bitmap >= 24)
            {
                ctc_chip_set_gpio_mode(para->slave_bitmap - 24 + 4, CTC_CHIP_SPECIAL_MODE);
            }
#endif            
            phdl->read          = fiber_ctc_chip_read;
            phdl->write         = fiber_ctc_chip_write;
            phdl->fiber_present = fiber_get_present_info;
            break;
        case E_FIBER_BRIDGE_CPU:
            sal_memset(&i2c_gen, 0, sizeof(i2c_gen_t));
            i2c_gen.i2c_type = E_I2C_CPM;
            i2c_gen.bridge_flag = 1;
            i2c_gen.alen = 1;
            i2c_gen.i2c_bus_idx = p_info->bus_id;
            i2c_gen.p_br.i2c_br_type  = p_info->cs.br_cpu.bridge_type;
            i2c_gen.p_br.bridge_addr  = p_info->cs.br_cpu.bridge_addr;
            i2c_gen.p_br.channel      = p_info->cs.br_cpu.bridge_chan;
            i2c_gen.addr = FIBER_DEV_ADDR1;
            i2c_hdl_dev1 = i2c_create_handle(&i2c_gen);
            i2c_gen.addr = FIBER_DEV_ADDR2;
            i2c_hdl_dev2 = i2c_create_handle(&i2c_gen);
            i2c_gen.addr = FIBER_DEV_ADDR3;
            i2c_hdl_dev3 = i2c_create_handle(&i2c_gen);

            if((!i2c_hdl_dev1)||(!i2c_hdl_dev2)||(!i2c_hdl_dev3))
            {
                DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, phdl);
                return NULL;
            }
            phdl->phdl_dev1 = i2c_hdl_dev1;
            phdl->phdl_dev2 = i2c_hdl_dev2;
            phdl->phdl_dev3 = i2c_hdl_dev3;
            phdl->phdl_dev4 = NULL;
   
            para->channel = p_info->fiber_channel;
            phdl->read          = fiber_i2c_read;
            phdl->write         = fiber_i2c_write;
            phdl->fiber_present = fiber_get_present_info;
            break;
        case E_FIBER_EPLD_ASIC:
            para->channel      = p_info->fiber_channel;
            para->bus_id       = p_info->bus_id;
            para->lchip        = p_info->cs.asic.lchip;
            para->slave_bitmap = p_info->cs.asic.slave_bitmap;
            para->epld_switch_reg = p_info->cs.asic.epld_switch_reg;
            phdl->read          = fiber_ctc_chip_epld_read;
            phdl->write         = fiber_ctc_chip_epld_write;
            phdl->fiber_present = fiber_get_present_info;
            break;
            
        default:
            DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, phdl);
            return phdl;
    }

    phdl->fiber_los = NULL;/* not support get los of fiber */
    if(p_info->fiber_flg == E_FIBER_QSFP_P)
    {
        phdl->fiber_enable  = fiber_ctc_qsfp_enable;
    }
    else if(p_info->en_mode == E_FIBER_EN_GPIO)
    {
        para->enable_chip   = p_info->en.en_gpio.enable_chip;
        para->enable_no     = p_info->en.en_gpio.enable_no;
        phdl->fiber_enable  = fiber_gpio_enable;
    }
    else if(p_info->en_mode == E_FIBER_EN_EPLD)
    {
        para->enable_chip   = p_info->en.en_epld.enable_chip;
        para->enable_reg    = p_info->en.en_epld.enable_reg;
        para->enable_no     = p_info->en.en_epld.enable_no;
        phdl->fiber_enable  = fiber_epld_enable;                
    }
    
    return phdl;
}


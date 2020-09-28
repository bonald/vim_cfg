/****************************************************************************
* $Id$
* phy LSI1082 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-10-08
* Reason        : First Create.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "epld_api.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
//#include "humber_api.h"
#include "phy_lsi1082.h"

#define LSI1082_PHY_EMI_ADDR 16
#define LSI1082_PHY_EMI_DATA 17
#define LSI1082_PHY_LB_CTRL   19
#define LSI1082_PHY_MI_CTRL   21
#define LSI1082_PHY_INT_MASK 24
#define LSI1082_PHY_INT_STATUS   25
#define LSI1082_PHY_STATUS_REG     26
#define LSI1082_PHY_LED_CTRL_1     27
#define LSI1082_PHY_LED_CTRL_2     28
#define LSI1082_PHY_LED_CTRL_3     29

static int32 
lsi1082_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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
lsi1082_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
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
lsi1082_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
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

static int32 lsi1082_read(phy_handle_t* phy_handle, uint16 reg, uint16 *value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}
static int32 lsi1082_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

static int32
lsi1082_init_chip(phy_handle_t* phy_handle, phy_info_t* pphy_info)
{
    int32 ret=0;

    if((pphy_info->phy_addr%8) != 0)
        return 0;
    /*Init led polarity to active low. bit 7*/
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_ADDR, 64774);
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_DATA, 0x3c80);
    /*Init chip led to output enable, bit0-7 LEDA 8 ports, bit8-15 LEDB 8 ports.*/
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_ADDR, 64782);
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_DATA, 0xffff);
    /*open global phy Interrupt Mask*/
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_ADDR, 64780);
    ret += lsi1082_write(phy_handle, LSI1082_PHY_EMI_DATA, 0x4);
    return ret;
}

/*This led mode must re-init after soft-reset.*/
static int32
lsi1082_init_led_mode(phy_handle_t* phy_handle, uint8_t led_mode)
{
    int32 ret;
    /*Init Led A/B to Ext mode, adjust pulse duration to 100ms*/
    ret = lsi1082_write(phy_handle, LSI1082_PHY_LED_CTRL_1, 0x1c0b);
    /*Init Led A as 10/100 Link and active, Led B as 1000 Link and active*/
    ret += lsi1082_write(phy_handle, LSI1082_PHY_LED_CTRL_2, 0x45);
    return ret;
}

static int32
lsi1082_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "LSI1082 phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
    }
    else 
    {
        /*power down*/
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
    }
    return ret;
}

int32 lsi1082_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media)
{
    return 0;
}

int32 
lsi1082_link_down_setting(phy_handle_t* phy_handle)
{
    return 0;
} 


/**********************************************************************
 *  Name:       : phy_lsi1082_get_cur_status
 *  Purpose     : get current link status, speed and duplex per port
 *  Input       : mdio_bus_no: MDIO bus ID
                  phy_addr:    PHY address per MDIO bus
                  phy_status:  PHY status(link_up, speed, duplex)
 *  Output      : N/A
 *  Return      : N/A
 *  Note        : N/A
 ***********************************************************************/
int32
lsi1082_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t *phy_status)
{
    int32 ret = RESULT_OK;
    uint16 cfg_symmetric, cfg_asymmetric, lp_symmetric, lp_asymmetric;
    uint16 value;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    ret += lsi1082_read(phy_handle, LSI1082_PHY_STATUS_REG, &value);

    /* Get Link */
    phy_status->link_up = (value & 0x40) ? GLB_LINK_UP : GLB_LINK_DOWN;

    /* Get Speed */
    if((value & 0x300) == 0x0)
    {
        phy_status->speed = GLB_SPEED_10M;
    }
    else if((value & 0x300) == 0x100)
    {
        phy_status->speed = GLB_SPEED_100M;
    }
    else if((value & 0x300) == 0x200)
    {
        phy_status->speed = GLB_SPEED_1G;
    }
    else
    {
        phy_status->speed = GLB_SPEED_1G;
    }
        
    /* Get Duplex */
    if((value & 0x80) == 0x00)
    {
        phy_status->duplex = GLB_DUPLEX_HALF;
    }
    else
    {
        phy_status->duplex = GLB_DUPLEX_FULL;
    }
    phy_status->linkup_media = GLB_PORT_TYPE_COPPER;

    if(phy_status->link_up == GLB_LINK_UP)
    {
        /*If auto enable and auto complete, need get flow control info.*/
        if((value & 0x1000)==0x1000)
        {
            cfg_symmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause;
            cfg_asymmetric = phy_handle->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause;
            lp_symmetric = ((value>>1) & 0x1);
            lp_asymmetric = (value & 0x1);
            phy_status->flowctrl.recv = (cfg_symmetric && (lp_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
            phy_status->flowctrl.send = (lp_symmetric && (cfg_symmetric || (cfg_asymmetric && lp_asymmetric)) ? 1 : 0);
        }
        else
        {
            phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
            phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        }
    }    
    else
    {
        phy_status->speed = GLB_SPEED_AUTO;
        phy_status->duplex = GLB_DUPLEX_AUTO;
        phy_status->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_status->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }
    /*For test*/
#if 0    
    phy_status->link_up = GLB_LINK_UP;
    phy_status->speed = GLB_SPEED_100M;
    phy_status->duplex = GLB_DUPLEX_FULL;
#endif    
    return ret;
}

int32 lsi1082_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    lsi1082_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "LSI1082 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        lsi1082_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "LSI1082 phy addr %d link down",pphy_info->phy_addr) 
        lsi1082_link_down_setting(phy_handle);
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
        
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change))
        {
            lsi1082_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex,phy_val->linkup_media);
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
lsi1082_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
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
    ret += lsi1082_read(phy_handle, LSI1082_PHY_INT_STATUS, &val);
    if(0 == val)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    ret += lsi1082_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"LSI1082_get_cur_status speed: %d duplex: %d",phy_val->speed, phy_val->duplex);
    
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

        ret += lsi1082_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        ret += lsi1082_link_down_setting(phy_handle);
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
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change))
        {
            ret += lsi1082_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
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

static int32 lsi1082_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
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
        ret += lsi1082_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0);
        ret += lsi1082_write_mask(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300);        
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);        
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
	     /* Modified by liuht to powerdown more time to notify link parter for bug25796, 2013-11-14 */		
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0100, 0x3140);
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x2100, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x2000, 0x3140);
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0140, 0x3140);
        }
        else /*1G Half*/
        {    
            ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0040, 0x3140);
        }
        /* first power down and power up the port for notify the link partner */
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
        sal_udelay(50000);            
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
    }
    return ret;  
}

static int32
lsi1082_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "LSI1082 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = lsi1082_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

static int32 
lsi1082_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "LSI1082 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = lsi1082_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

static int32
lsi1082_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "LSI1082 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_LB_CTRL, 0x0000, 0x0200);  /* Disable Far-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_MI_CTRL, 0x0008, 0x0008);  /* Enable Energy-Detect Powerdown*/
        break;

    case GLB_LB_PHY_FAR:
        ret |= lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);        /* Disable Nead-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_LB_CTRL, 0x0200, 0x0200);  /* Enable Far-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_MI_CTRL, 0x0000, 0x0008);  /* Disable Energy-Detect Powerdown*/
        break;

    case GLB_LB_PHY_NEAR:
        ret |= lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_LB_CTRL, 0x0000, 0x0200);  /* Disable Far-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_MI_CTRL, 0x0000, 0x0008);  /* Disable Energy-Detect Powerdown*/
        break;

    /* Added by liuht for bug 27508, 2014-06-30 */
    case GLB_LB_PHY_NEAR_DIAG:
        ret |= lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);        /* Enable Nead-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_LB_CTRL, 0x0000, 0x0200);  /* Disable Far-End Loopback */
        ret |= lsi1082_write_mask(phy_handle, LSI1082_PHY_MI_CTRL, 0x0000, 0x0008);  /* Disable Energy-Detect Powerdown*/
        ret |= lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
        break;
    /* End of liuht added */ 

    default:
        return RESULT_ERROR;
        break;
    }

    return ret;
}

int32
lsi1082_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
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
        "LSI1082 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += lsi1082_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += lsi1082_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}

int32 lsi1082_set_vct_mod(phy_handle_t* phy_handle)
{
    int32 ret = 0;
#if 0
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"lsi1082_set_vct_mod. ");
   
    ret += lsi1082_phy_page_ext1(phy_handle);
    ret += lsi1082_write(phy_handle, 0x18, 0x8000);
    ret += lsi1082_phy_page_std(phy_handle);
#endif    
    return ret;
}

int32 lsi1082_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
{
#if 0    
    uint16 val1, val2, val3;
    int32 ret = 0;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"lsi1082_get_vct_info. ");

    ret += lsi1082_phy_page_ext1(phy_handle);

    ret += lsi1082_read(phy_handle, 0x18, &val1);
    ret += lsi1082_read(phy_handle, 0x19, &val2);
    ret += lsi1082_read(phy_handle, 0x1a, &val3);
    ret += lsi1082_write(phy_handle, PHY_EXTENDED_PAGE, 0x0);

    DRV_LOG_DEBUG(DRV_PHY, "phy get vct info: val1=%x val2=%x val3=%x\n", val1, val2, val3);
    
    p_cable_info->pair_A.pair_length = ((val1 >> 8) & 0x3F);
    p_cable_info->pair_B.pair_length = (val1 & 0x3F);
    p_cable_info->pair_C.pair_length = ((val2 >> 8) & 0x3F);
    p_cable_info->pair_D.pair_length = (val2 & 0x3F);
    p_cable_info->pair_A.pair_status = ((val3 >> 12) & 0xF);
    p_cable_info->pair_B.pair_status = ((val3 >> 8)  & 0xF);
    p_cable_info->pair_C.pair_status = ((val3 >> 4)  & 0xF);
    p_cable_info->pair_D.pair_status = ((val3 >> 0)  & 0xF);
    /* when the length of pair A/B/C/D are 0x3F, the length of pair A/B/C/D is zero */
    if ((0x3F == p_cable_info->pair_A.pair_length) || (0x3F == p_cable_info->pair_B.pair_length) 
        || (0x3F == p_cable_info->pair_C.pair_length)  || (0x3F == p_cable_info->pair_D.pair_length) )
    {
        p_cable_info->pair_A.pair_length = 0;
        p_cable_info->pair_B.pair_length = 0;
        p_cable_info->pair_C.pair_length = 0;
        p_cable_info->pair_D.pair_length = 0;
    }
    else
    {
        p_cable_info->pair_A.pair_length = p_cable_info->pair_A.pair_length * GLB_VCT_PAIR_ACCURACY;
        p_cable_info->pair_B.pair_length = p_cable_info->pair_B.pair_length * GLB_VCT_PAIR_ACCURACY;
        p_cable_info->pair_C.pair_length = p_cable_info->pair_C.pair_length * GLB_VCT_PAIR_ACCURACY;
        p_cable_info->pair_D.pair_length = p_cable_info->pair_D.pair_length * GLB_VCT_PAIR_ACCURACY;
    } 
    
    if (phy_handle->phy_info.phy_stat_flag.link_up)
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
    return ret;
#else
    return 0;
#endif
}

int32 lsi1082_init(phy_handle_t* phy_handle)
{
    int32 ret = RESULT_OK;
    phy_info_t* pphy_info;
    uint16 value = 0;
    uint16 count = 0;
    
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "LSI1082 Init phy addr %d", pphy_info->phy_addr);
    
    /*soft-reset, LSI phy return to power down*/
    ret += lsi1082_write(phy_handle, PHY_CTL_REG, 0x9040);
    /* wait up to 5 seconds */
    while(1)
    {
        sal_udelay(10);
        ret += lsi1082_read(phy_handle, PHY_CTL_REG, &value);
        if(!ret && (value & PHY_CTL_SW_RESET) == 0)
        {
            break;
        }
        
        count++;
        if(count >= 1000)
        {
            DRV_LOG_ERR("LSI1082 PHY [addr: ]reset timeout!",pphy_info->phy_addr);
            return RESULT_ERROR;
        }
    }    
    ret += lsi1082_init_chip(phy_handle, pphy_info);
    ret += lsi1082_init_led_mode(phy_handle, pphy_info->led_type);
    /*read Int status to clear Int*/
    ret += lsi1082_read(phy_handle, LSI1082_PHY_INT_STATUS, &value);
    /*open Int, include link/speed change.*/
    ret += lsi1082_write(phy_handle, LSI1082_PHY_INT_MASK, 0x7);
    return ret;
}

phy_handle_t* phy_lsi1082_dev_register(phy_info_t* pphy_info)
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

    phdl->phy_init = lsi1082_init;
    phdl->config_enable = lsi1082_enable;
    phdl->config_speed = lsi1082_config_speed;
    phdl->config_duplex = lsi1082_config_duplex;
    phdl->config_loopback = lsi1082_config_loopback;
    phdl->config_flowctrl = lsi1082_config_flowctrl;
    phdl->get_link_poll = lsi1082_get_link_poll;
    phdl->get_link_interupt = lsi1082_get_link_interrput;
    phdl->reg_read = lsi1082_reg_read;
    phdl->reg_write = lsi1082_reg_write;
    
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


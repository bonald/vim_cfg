/****************************************************************************
* $Id$
* phy 88E15XX device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht 
* Date          : 2017-03-15
* Reason        : First Create.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_mv88e15xx.h"

static int32 
mv88e15xx_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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
mv88e15xx_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
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
mv88e15xx_write_mask(phy_handle_t* phy_handle, uint16 reg, uint16 value, uint16 mask)
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

static int32 mv88e15xx_write(phy_handle_t* phy_handle, uint16 reg, uint16 value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

#if 0
static int32 mv88e15xx_read(phy_handle_t* phy_handle, uint16 reg, uint16* value)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;

    phyreg_para.dat.regaddr8 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, value);

    return ret;
}
#endif

static int32 mv88e15xx_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
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
                    reg4 = 0xa0;
                    reg9 = 0x0;
                }                
                break;
            case GLB_SPEED_1G:				
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
        ret += mv88e15xx_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, reg4, 0xde0);
        ret += mv88e15xx_write_mask(phy_handle, DIAG_1G_CTL_REG, reg9, 0x300);        
        ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);        
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);	
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0100, 0x3140);
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x2100, 0x3140);
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x2000, 0x3140);
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0140, 0x3140);
        }
        else /*1G Half*/
        {    
            ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0040, 0x3140);
        }
    }
    
    return ret;    
}

int32 mv88e15xx_init(phy_handle_t* phy_handle)
{
    mv88e15xx_write(phy_handle, 22, 0x00ff);
    mv88e15xx_write(phy_handle, 17, 0x2148);
    mv88e15xx_write(phy_handle, 16, 0x2144);
    mv88e15xx_write(phy_handle, 17, 0x0c28);
    mv88e15xx_write(phy_handle, 16, 0x2146);
    mv88e15xx_write(phy_handle, 17, 0xb233);
    mv88e15xx_write(phy_handle, 16, 0x214d);
    mv88e15xx_write(phy_handle, 17, 0xdc0c);
    mv88e15xx_write(phy_handle, 16, 0x2159);
    mv88e15xx_write(phy_handle, 22, 0x00fb);
    mv88e15xx_write(phy_handle,  7, 0xc00d);
    mv88e15xx_write(phy_handle, 22, 0x0000);

    return RESULT_OK;
}


int32 mv88e15xx_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = 0;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "MV88E15XX phy addr %d enable: speed %d duplex %d", 
                pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);
    if(enable)
    {
        /*power on*/
        ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x0800);
    }
    else 
    {
        /*power down*/
        ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0800, 0x0800);
    }
    
    return ret;
}

int32 mv88e15xx_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E15XX phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = mv88e15xx_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}


int32 mv88e15xx_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E15XX phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = mv88e15xx_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

int32 mv88e15xx_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = 0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E15XX phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret |= mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x2, 0xffff);
        ret |= mv88e15xx_write_mask(phy_handle, MV_EXTENDED_PHY_CTL1, 0x0000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x0, 0xffff);
        break;

    case GLB_LB_PHY_FAR:
        ret |= mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x2, 0xffff);
        ret |= mv88e15xx_write_mask(phy_handle, MV_EXTENDED_PHY_CTL1, 0x4000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x0, 0xffff);      
        break;

    case GLB_LB_PHY_NEAR:
        ret |= mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x2, 0xffff);
        ret |= mv88e15xx_write_mask(phy_handle, MV_EXTENDED_PHY_CTL1, 0x0000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x0, 0xffff);         
        break;

    case GLB_LB_PHY_NEAR_DIAG:
        ret |= mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x4000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x2, 0xffff);
        ret |= mv88e15xx_write_mask(phy_handle, MV_EXTENDED_PHY_CTL1, 0x0000, 0x4000);
        ret |= mv88e15xx_write_mask(phy_handle, 22, 0x0, 0xffff);
        ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x1200, 0x1200);
        break;

    default:
        return RESULT_ERROR;
    }

    return ret;
}

int32 mv88e15xx_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    return RESULT_OK; 
}

int32
mv88e15xx_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
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
        "MV88E15XX phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    
    if(asymmetric)
    {
        value |= 1<<11;
    }
    if(symmetric)
    {
        value |= 1<<10;
    }
    ret += mv88e15xx_write_mask(phy_handle, AUTO_NEGO_ADVERTI_REG, value, 0x0c00);
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed==GLB_SPEED_AUTO)||(pphy_info->phy_manage_info.duplex==GLB_DUPLEX_AUTO)))
    {
        ret += mv88e15xx_write_mask(phy_handle, PHY_CTL_REG, 0x0200, 0x0200);
    }
    return ret;
}


int32 mv88e15xx_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex)
{
    return RESULT_OK;
}

int32 
mv88e15xx_link_down_setting(phy_handle_t* phy_handle)
{
    return RESULT_OK;
} 

static int32 
mv88e15xx_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
    phyreg_param_t phyreg_para;
    int32 ret = 0;
    uint16 value;
    
    if(NULL == phy_handle || NULL == phy_val)
    {
        return RESULT_ERROR;
    }

    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    
    phyreg_para.dat.regaddr8 = PHY_STATUS_REG;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);

    phy_val->link_up = (value&0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;

    /*if port is up, get real media*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        phyreg_para.dat.regaddr8 = 0x11;
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);

        if((value&MV_PHY_STATUS_SPEED) == 0x0000)
        {
            phy_val->speed = GLB_SPEED_10M;
        }
        else if((value&MV_PHY_STATUS_SPEED) == 0x4000)
        {
            phy_val->speed = GLB_SPEED_100M;
        }
        else if((value&MV_PHY_STATUS_SPEED) == 0x8000)
        {
            phy_val->speed = GLB_SPEED_1G;
        }   

        if((value&MV_PHY_STATUS_FDX) == 0)
        {
            phy_val->duplex = GLB_DUPLEX_HALF;
        }
        else
        {
            phy_val->duplex = GLB_DUPLEX_FULL;
        }
        
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        phy_val->linkup_media = GLB_PORT_TYPE_COPPER;
    }
    else
    {
        phy_val->speed = GLB_SPEED_AUTO;
        phy_val->duplex = GLB_DUPLEX_AUTO;
        phy_val->linkup_media = GLB_PORT_TYPE_COPPER; 
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }    

    return ret;
}

int32 mv88e15xx_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    mv88e15xx_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E15XX phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        mv88e15xx_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E15XX phy addr %d link down",pphy_info->phy_addr) 
        mv88e15xx_link_down_setting(phy_handle);
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
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change))
        {
            mv88e15xx_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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
mv88e15xx_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
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
    phyreg_para.dat.regaddr8 = MV_PHY_INT_STATUS;
    phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    if((val & 0x0400) == 0)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    mv88e15xx_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"mv88e15xx_get_cur_status speed: %d duplex: %d",phy_val->speed, phy_val->duplex);
    
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

        mv88e15xx_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);

    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        mv88e15xx_link_down_setting(phy_handle);
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
        if((phy_change->speed_change==1) || (1==phy_change->duplex_change))
        {
            mv88e15xx_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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

phy_handle_t* phy_mv88e15xx_dev_register(phy_info_t* pphy_info)
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

    phdl->phy_init = mv88e15xx_init;
    phdl->config_enable = mv88e15xx_enable;
    phdl->config_speed = mv88e15xx_config_speed;
    phdl->config_duplex = mv88e15xx_config_duplex;
    phdl->config_loopback = mv88e15xx_config_loopback;
    phdl->config_medium = mv88e15xx_config_medium;
    phdl->config_flowctrl = mv88e15xx_config_flowctrl;
    phdl->get_link_poll = mv88e15xx_get_link_poll;
    phdl->get_link_interupt = mv88e15xx_get_link_interrput;
    phdl->get_cur_status = mv88e15xx_get_cur_status;
    phdl->reg_read = mv88e15xx_reg_read;
    phdl->reg_write = mv88e15xx_reg_write;
    
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


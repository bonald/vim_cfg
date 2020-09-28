/****************************************************************************
* $Id$
* phy 88E1680 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-11-01
* Reason        : First Create.
****************************************************************************/

#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_mv88e1680.h"
#include "ctc_api.h"
#include "madApiInternal.h"
#include "madDrvPhyRegs.h"
#include "madHwCntl.h"

//phyAddr is real mdio addr, based on zero
MAD_BOOL _mad_mv88e1680_readMii(MAD_DEV* dev, unsigned int phyAddr, unsigned int miiReg, unsigned int* value)
{
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == dev || NULL == dev->phdl || NULL== dev->phdl->mdio_hdl || NULL == value)
    {
        return MAD_FAIL;
    }

    sal_memset(&para, 0, sizeof(mdio_bus_para_t)); 
    para.gephy_para.phy_addr = phyAddr;
    para.gephy_para.reg = miiReg;
    ret = dev->phdl->mdio_hdl->read(dev->phdl->mdio_hdl, &para);
    if(ret < 0)
    {
        return MAD_FAIL;
    }
     
    *value = para.gephy_para.val;

    return MAD_TRUE;
}


//phyAddr is read mdio addr, based on zero
MAD_BOOL _mad_mv88e1680_writeMii(MAD_DEV* dev, unsigned int phyAddr, unsigned int miiReg, unsigned int value)
{
    int32 ret;
    mdio_bus_para_t para;

    if(NULL == dev || NULL == dev->phdl || NULL== dev->phdl->mdio_hdl)
    {
        return MAD_FAIL;
    }

    sal_memset(&para, 0, sizeof(mdio_bus_para_t)); 
    para.gephy_para.phy_addr = phyAddr;
    para.gephy_para.reg = miiReg;
    para.gephy_para.val = value;
    ret = dev->phdl->mdio_hdl->write(dev->phdl->mdio_hdl, &para);
    if(ret < 0)
    {
        return MAD_FAIL;
    }

    return MAD_TRUE;
}

/*****************************************************************************/

static int32 
mv88e1680_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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
mv88e1680_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
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


static int32 mv88e1680_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    MAD_STATUS ret = MAD_OK;
    MAD_U32 mode = 0x0;

    /*Auto mode*/
    if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
    {
        switch(speed)
        {
            case GLB_SPEED_AUTO:
                if(duplex == GLB_DUPLEX_AUTO)
                {
                    mode = MAD_AUTO_AD_1000FDX | MAD_AUTO_AD_100FDX | MAD_AUTO_AD_100HDX | MAD_AUTO_AD_10FDX | MAD_AUTO_AD_10HDX;
                }
                else if(duplex == GLB_DUPLEX_FULL)
                {
                    mode = MAD_AUTO_AD_1000FDX | MAD_AUTO_AD_100FDX | MAD_AUTO_AD_10FDX;
                }
                else
                {               
                    mode = MAD_AUTO_AD_100HDX | MAD_AUTO_AD_10HDX;
                }                
                break;
            case GLB_SPEED_1G:				
                mode = MAD_AUTO_AD_1000FDX;
                break;
            case GLB_SPEED_100M:
                mode = MAD_AUTO_AD_100FDX | MAD_AUTO_AD_100HDX;
                break;
            case GLB_SPEED_10M:
                mode = MAD_AUTO_AD_10FDX | MAD_AUTO_AD_10HDX;
                break;
            default:				
                mode = MAD_AUTO_AD_1000FDX | MAD_AUTO_AD_100FDX | MAD_AUTO_AD_100HDX | MAD_AUTO_AD_10FDX | MAD_AUTO_AD_10HDX;
                break;
        }
        if(phy_handle->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause)
        {
            mode |= MAD_AUTO_AD_PAUSE;
        }
        if(phy_handle->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause)
        {
            mode |= MAD_AUTO_AD_ASYM_PAUSE;
        }
        ret = mdCopperSetAutoNeg(phy_handle->mad_dev, 
                                                     mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                     MAD_TRUE,
                                                     mode);
    }
    else /* Force mode*/
    {
        /*When config force mode, need to powerdown and powerup port to notify link parter.*/
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            phy_handle->config_enable(phy_handle, 0);
            if(phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_10M)
            {
                sal_udelay(100000); 
            }
            else
            {
                sal_udelay(20000);
            }
            phy_handle->config_enable(phy_handle, 1);
        }
        if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_FULL))
        {
            mode = MAD_PHY_10FDX;
        }
        else if((speed == GLB_SPEED_10M)&&(duplex == GLB_DUPLEX_HALF))
        {
            mode = MAD_PHY_10HDX;
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_FULL))
        {
            mode = MAD_PHY_100FDX;
        }
        else if((speed == GLB_SPEED_100M)&&(duplex == GLB_DUPLEX_HALF))
        {
            mode = MAD_PHY_100HDX;
        }
        else if((speed == GLB_SPEED_1G)&&(duplex == GLB_DUPLEX_FULL))
        {
            mode = MAD_PHY_1000FDX;
        }
        else /*1G Half*/
        {    
            mode = MAD_PHY_1000HDX;
        }
        ret += mdCopperSetAutoNeg(phy_handle->mad_dev, 
                                                       mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                       MAD_FALSE,
                                                       mode);
    }


    /*enable qsgmii phy-mac auto neg*/
    ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                               phy_handle->phy_info.phy_addr%8,
                                                               MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 12, 1, 1);

    /* sync loopback status */
    if (phy_handle->phy_info.phy_manage_info.lb_mode != GLB_LB_NONE)
    {
        phy_handle->config_loopback(phy_handle, phy_handle->phy_info.phy_manage_info.lb_mode);
    }

    if (ret == MAD_OK)
        return RESULT_OK;
    else
        return RESULT_ERROR;
}

int32 mv88e1680_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1680 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = mv88e1680_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

int32 mv88e1680_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1680 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = mv88e1680_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

int32 mv88e1680_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    MAD_STATUS ret = MAD_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1680 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);

    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%8,
                                                                 0, MAD_REG_CONTROL, 14, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%8,
                                                                  2, MAD_REG_MAC_CONTROL, 14, 1, 0);
        break;

    case GLB_LB_PHY_FAR:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%8,
                                                                 0, MAD_REG_CONTROL, 14, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%8,
                                                                  2, MAD_REG_MAC_CONTROL, 14, 1, 1);     
        break;

    case GLB_LB_PHY_NEAR:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%8,
                                                                 0, MAD_REG_CONTROL, 14, 1, 1);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%8,
                                                                  2, MAD_REG_MAC_CONTROL, 14, 1, 0); 
        break;

    case GLB_LB_PHY_NEAR_DIAG:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%8,
                                                                 0, MAD_REG_CONTROL, 14, 1, 1);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%8,
                                                                  2, MAD_REG_MAC_CONTROL, 14, 1, 0);
        break;

    default:
        return RESULT_ERROR;
    }

    if (ret == MAD_OK)
    {
        pphy_info->phy_manage_info.lb_mode = lb_mode;
        return RESULT_OK;
    }
    else
    {
        return RESULT_ERROR;
    }
}

int32 mv88e1680_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    return RESULT_OK; 
}

int32
mv88e1680_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    MAD_STATUS ret = MAD_OK;
    MAD_U32 mode = 0x0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1680 phy addr %d config flowctrl symmetric %d asymmetric %d", pphy_info->phy_addr, symmetric, asymmetric);    

    ret =  mdGetPhyCopperReg(phy_handle->mad_dev,
                                                phy_handle->phy_info.phy_addr%8,
                                                MAD_REG_AUTO_AD,
                                                &mode);
    if(asymmetric)
    {
        mode |= MAD_AUTO_AD_ASYM_PAUSE;
    }
    if(symmetric)
    {
        mode |= MAD_AUTO_AD_PAUSE;
    }
    
    ret += mdSetPhyCopperReg(phy_handle->mad_dev,
                                                 phy_handle->phy_info.phy_addr%8,
                                                 MAD_REG_AUTO_AD,
                                                 mode);
                                                 
    /*If phy is linkup and auto enable, need re-auto*/
    if((pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
        &&((pphy_info->phy_manage_info.speed == GLB_SPEED_AUTO) || (pphy_info->phy_manage_info.duplex == GLB_DUPLEX_AUTO)))
    {
        ret += mdCopperSetAutoRestart(phy_handle->mad_dev,
                                                             mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8));
    }

    /* sync loopback status */
    if (phy_handle->phy_info.phy_manage_info.lb_mode != GLB_LB_NONE)
    {
        phy_handle->config_loopback(phy_handle, phy_handle->phy_info.phy_manage_info.lb_mode);
    }

    if (ret == MAD_OK)
    {
        return RESULT_OK;
    }
    else
    {
        return RESULT_ERROR;
    }
}

int32 mv88e1680_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex)
{
    return RESULT_OK;
}

int32 
mv88e1680_link_down_setting(phy_handle_t* phy_handle)
{
    return RESULT_OK;
} 

static int32 
mv88e1680_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
    MAD_STATUS ret = MAD_OK;
    MAD_BOOL link_status;
    MAD_SPEED_MODE speed_status;
    MAD_DUPLEX_MODE duplex_status;
    
    if(NULL == phy_handle || NULL == phy_val)
    {
        return RESULT_ERROR;
    }

    if (phy_handle->phy_info.phy_manage_info.lb_mode == GLB_LB_PHY_NEAR || phy_handle->phy_info.phy_manage_info.lb_mode == GLB_LB_PHY_NEAR_DIAG)
    {
        phy_val->link_up = GLB_LINK_UP;
        phy_val->speed = (phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_AUTO) ? GLB_SPEED_1G : phy_handle->phy_info.phy_manage_info.speed;
        phy_val->duplex = (phy_handle->phy_info.phy_manage_info.duplex == GLB_DUPLEX_AUTO) ? GLB_DUPLEX_FULL : phy_handle->phy_info.phy_manage_info.duplex;
        phy_val->linkup_media = GLB_PORT_TYPE_COPPER;
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        return RESULT_OK;
    }

    ret = mdCopperGetLinkStatus(phy_handle->mad_dev,
                                                     mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                     &link_status);
    phy_val->link_up = (link_status == MAD_TRUE) ? GLB_LINK_UP : GLB_LINK_DOWN;

    /*if port is up, get real media*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        ret += mdCopperGetSpeedStatus(phy_handle->mad_dev,
                                                          mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                          &speed_status);
        if(speed_status == MAD_SPEED_10M)
        {
            phy_val->speed = GLB_SPEED_10M;
        }
        else if(speed_status == MAD_SPEED_100M)
        {
            phy_val->speed = GLB_SPEED_100M;
        }
        else if(speed_status == MAD_SPEED_1000M)
        {
            phy_val->speed = GLB_SPEED_1G;
        }   

        ret += mdCopperGetDuplexStatus(phy_handle->mad_dev,
                                                          mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                          &duplex_status);
        if(duplex_status == MAD_HALF_DUPLEX)
        {
            phy_val->duplex = GLB_DUPLEX_HALF;
        }
        else if(duplex_status == MAD_FULL_DUPLEX)
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

int32 mv88e1680_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    mv88e1680_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E1680 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        mv88e1680_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E1680 phy addr %d link down",pphy_info->phy_addr) 
        mv88e1680_link_down_setting(phy_handle);
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
            mv88e1680_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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
mv88e1680_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;
    MAD_INT_TYPE interrupt_status;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    sal_memset(&interrupt_status, 0, sizeof(MAD_INT_TYPE));

    /*if NO link change, return, read clean interrupt*/
    mdIntGetStatus(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                               &interrupt_status);
    if(interrupt_status.intGroup0 == 0 && interrupt_status.intGroup1 == 0)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    mv88e1680_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"port %d mv88e1680_get_cur_status group0 %d group1 %d speed: %d duplex: %d",
                    phy_handle->phy_info.phy_addr,interrupt_status.intGroup0,interrupt_status.intGroup1,phy_val->speed, phy_val->duplex);
    
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

        mv88e1680_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);

    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        mv88e1680_link_down_setting(phy_handle);
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
            mv88e1680_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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
mv88e1680_set_vct_mod(phy_handle_t* phy_handle)
{
    return RESULT_OK;
}

int32
mv88e1680_vct_status_remapping(glb_pair_info_t *pair_info, MAD_ADV_CABLE_STATUS *advCableStatus, int channel)
{
    int i = 0;
    
    switch(advCableStatus->cableStatus[channel])
    {
        case MAD_ADV_VCT_FAIL:
            pair_info->pair_status = GLB_PAIR_STATE_ABNORMAL_TERMINATION;
            //pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_length = 0xffff;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_NORMAL:
            pair_info->pair_status = GLB_PAIR_STATE_NORMAL;
            //pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_length = 0xffff;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_IMP_GREATER_THAN_115:
            pair_info->pair_status = GLB_PAIR_STATE_HIGH_IMPEDANCE;
            pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_IMP_LESS_THAN_85:
            pair_info->pair_status = GLB_PAIR_STATE_LOW_IMPEDANCE;
            pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_OPEN:
            pair_info->pair_status = GLB_PAIR_STATE_ABNORMAL_OPEN;
            pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_SHORT:
            pair_info->pair_status = GLB_PAIR_STATE_ABNORMAL_SHORTED;
            pair_info->pair_length = advCableStatus->u[channel].dist2fault;
            pair_info->pair_accuracy = 1;
            break;
        case MAD_ADV_VCT_CROSS_PAIR_SHORT:
            for(i=0; i<MAD_MDI_PAIR_NUM; i++)
            {
                if(advCableStatus->u[channel].crossShort.channel[i] == MAD_TRUE)
                {
                    switch (i)
                    {
                        case 0:
                            pair_info->pair_status = GLB_PAIR_STATE_CROSS_PAIR_SHORT_A;
                            break;
                        case 1:
                            pair_info->pair_status = GLB_PAIR_STATE_CROSS_PAIR_SHORT_B;
                            break;
                        case 2:
                            pair_info->pair_status = GLB_PAIR_STATE_CROSS_PAIR_SHORT_C;
                            break;
                        case 3:
                            pair_info->pair_status = GLB_PAIR_STATE_CROSS_PAIR_SHORT_D;
                            break;
                        default:
                            break;
                    }
                    pair_info->pair_length = advCableStatus->u[channel].crossShort.dist2fault[i];
                    pair_info->pair_accuracy = 1;
                }
            }
            break;
        default:
            break;
    }

    return RESULT_OK;
}

int32 
mv88e1680_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
{
    MAD_STATUS status;
    MAD_ADV_CABLE_STATUS advCableStatus;
    MAD_ADV_VCT_MODE mode;
    
    mode.mode = MAD_ADV_VCT_MAX_PEAK;
    mode.transChanSel = MAD_ADV_VCT_TCS_NO_CROSSPAIR;
    mode.sampleAvg = 0;
    mode.peakDetHyst = 0;
    mode.samplePointDist = 0;
    memset(&advCableStatus, 0, sizeof(advCableStatus));

    status = MAD_OK;
    status = mdDiagGetAdvCableStatus(phy_handle->mad_dev,
                                                             mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                             mode,
                                                             &advCableStatus);
    if (status != MAD_OK)
    {
        if ((status&MAD_RUN_ST_MASK) == MAD_PENDING)
        {
            return GLB_VCT_WAIT;
        }
        else
        {
            return GLB_VCT_ERROR;
        }
    }
    
    mv88e1680_vct_status_remapping(&(p_cable_info->pair_A), &advCableStatus, 0);
    mv88e1680_vct_status_remapping(&(p_cable_info->pair_B), &advCableStatus, 1);
    mv88e1680_vct_status_remapping(&(p_cable_info->pair_C), &advCableStatus, 2);
    mv88e1680_vct_status_remapping(&(p_cable_info->pair_D), &advCableStatus, 3);
    p_cable_info->port_link_up = 1;
    if ((GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_A.pair_status) 
        && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_B.pair_status)
        && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_C.pair_status) 
        && (GLB_PAIR_STATE_ABNORMAL_OPEN == p_cable_info->pair_D.pair_status))
    {
        p_cable_info->remote_pair = GLB_REMOTE_PAIR_ABSENT;
    }
    else
    {
        p_cable_info->remote_pair = GLB_REMOTE_PAIR_PRESENT;
    }

    mdSysSoftReset (phy_handle->mad_dev,
                                mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8));

    return GLB_VCT_OK;
}


static int32
mv88e1680_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = MAD_OK;
    MAD_BOOL en = MAD_FALSE;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "MV88E1680 phy addr %d enable %d: speed %d duplex %d", 
                pphy_info->phy_addr, enable, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);

    if (enable)
    {
        en = MAD_TRUE;
    }
    else
    {
        en = MAD_FALSE;
    }
    
    ret = mdSysSetPhyEnable(phy_handle->mad_dev,
                                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                               en);

    /* sync loopback status */
    if (enable && pphy_info->phy_manage_info.lb_mode != GLB_LB_NONE)
    {
        phy_handle->config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }

    if (ret == MAD_OK)
    {
        return RESULT_OK;
    }
    else
    {
        return RESULT_ERROR;
    }
}

/* marvell init phy by phy device, but we init phy by port, 
 * this function only do marvell init, for mdLoadDriver may operate all port in a phy,
 * for example, we do some operate after mdLoadDriver on port 1, then when we do
 * mdLoadDriver on port 2, our operate may be overwritted by marvell */
int32 mv88e1680_init(phy_handle_t* phy_handle)
{
    MAD_SYS_CONFIG mad_sys_config;
    MAD_STATUS status = MAD_OK;
    
    phy_handle->mad_dev = (MAD_DEV *)DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(MAD_DEV));
    if(NULL == phy_handle->mad_dev)
    {
        DRV_LOG_ERR("malloc failed");
        goto err_out;
    }
    memset(&mad_sys_config, 0, sizeof(MAD_SYS_CONFIG));
    mad_sys_config.smiBaseAddr = phy_handle->phy_info.phy_addr - phy_handle->phy_info.phy_addr%8;
    mad_sys_config.port = phy_handle->phy_info.phy_addr%8;
    mad_sys_config.switchType = MAD_SYS_SW_TYPE_NO;
    mad_sys_config.phdl = phy_handle;
    mad_sys_config.BSPFunctions.readMii = _mad_mv88e1680_readMii;
    mad_sys_config.BSPFunctions.writeMii = _mad_mv88e1680_writeMii;
    mad_sys_config.BSPFunctions.readXMii = NULL;
    mad_sys_config.BSPFunctions.writeXMii = NULL;
    mad_sys_config.BSPFunctions.semCreate = NULL;
    mad_sys_config.BSPFunctions.semDelete = NULL;
    mad_sys_config.BSPFunctions.semTake = NULL;
    mad_sys_config.BSPFunctions.semGive = NULL;
    if ((status = mdLoadDriver(&mad_sys_config, phy_handle->mad_dev)) != MAD_OK)
    {
        DRV_LOG_ERR("init phy failed");
        goto err_out;
    }

    return RESULT_OK;
    
err_out:
    if (NULL != phy_handle->mad_dev)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, phy_handle->mad_dev);
        phy_handle->mad_dev = NULL;
    }

    return RESULT_ERROR;
}

int32 mv88e1680_post_init(phy_handle_t* phy_handle)
{
    MAD_INT_TYPE interrupt_status;
    MAD_INT_TYPE interrupt_type;
    MAD_LED_CTRL_DATA led_mode;

    /* set downshift enable*/
    mdCopperSetDownshiftEnable(phy_handle->mad_dev,
                                                      mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                      MAD_TRUE,
                                                      1);

    /* set mdi/midx crossover auto */
    mdCopperSetMDIMode(phy_handle->mad_dev,
                                          mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                          MAD_AUTO_MDI_MDIX);

    /* set fast link down enable and link down delay 20ms */
    mdCopperSetLinkDownDelay(phy_handle->mad_dev,
                                                    mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                    MAD_1G_LINKDOWN_DELAY_20);
    mdCopperSetLinkDownDelayEnable(phy_handle->mad_dev,
                                                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                                               MAD_TRUE);
 
    /*config force or auto*/
    mv88e1680_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed, phy_handle->phy_info.phy_manage_info.duplex);

    /*Clear interrupts*/
    mdIntGetStatus(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                               &interrupt_status);

    /* open interrupt enable:speed , duplex ,link status */
    interrupt_type.intGroup0 = MAD_COPPER_SPEED_CHANGED | MAD_COPPER_DUPLEX_CHANGED | MAD_COPPER_LINK_STATUS_CHANGED;
    interrupt_type.intGroup1 = 0;
    mdIntSetEnable(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                               &interrupt_type);

    /* config led mode */
    memset(&led_mode, 0, sizeof(MAD_LED_CTRL_DATA));
    if((GLB_PHY_LED_16 == phy_handle->phy_info.led_type)||(GLB_PHY_LED_DEFAULT == phy_handle->phy_info.led_type))
    {
        led_mode.ledCtrl[0] = 0xf;
        led_mode.ledCtrl[1] = 0xf;
        /* for bug 53255, by tongzb, 2019-10-15 */
        led_mode.ledMixPercen0 = 0x0;
        led_mode.ledMixPercen1 = 0x8;
    }
    if(GLB_PHY_LED_FREQ_DEFAULT == phy_handle->phy_info.led_freq)
    {
        led_mode.ledBlinkRate = 0x2;//170ms
        led_mode.ledPulseDuration = 0x2;
    }
    else if(GLB_PHY_LED_FREQ_2P5 == phy_handle->phy_info.led_freq)
    {
        led_mode.ledBlinkRate = 0x3;//340ms
        led_mode.ledPulseDuration = 0x3;
    }
    mdSysSetLEDCtrl(phy_handle->mad_dev,
                                mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%8),
                                &led_mode);
    
    /*init to power down*/
    phy_handle->config_enable(phy_handle, 0);
    
    return RESULT_OK;
}

phy_handle_t* phy_mv88e1680_dev_register(phy_info_t* pphy_info)
{
    phy_handle_t* phdl = NULL;
    mdio_handle_t* p_mdio_hdl = NULL;
    mdio_info_t mdio_info;

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
    memset(phdl, 0, sizeof(phy_handle_t));
    memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));

    phdl->phy_init = mv88e1680_init;
    phdl->phy_post_init = mv88e1680_post_init;
    phdl->config_enable = mv88e1680_enable;
    phdl->config_speed = mv88e1680_config_speed;
    phdl->config_duplex = mv88e1680_config_duplex;
    phdl->config_loopback = mv88e1680_config_loopback;
    phdl->config_medium = mv88e1680_config_medium;
    phdl->config_flowctrl = mv88e1680_config_flowctrl;
    phdl->get_link_poll = mv88e1680_get_link_poll;
    phdl->get_link_interupt = mv88e1680_get_link_interrput;
    phdl->get_cur_status = mv88e1680_get_cur_status;
    phdl->phy_set_vct_mod = mv88e1680_set_vct_mod;
    phdl->phy_get_vct_info = mv88e1680_get_vct_info;
    phdl->reg_read = mv88e1680_reg_read;
    phdl->reg_write = mv88e1680_reg_write;

    mdio_info.base_addr = pphy_info->base_addr;             //bus 0 or bus 1
    mdio_info.bus_type = pphy_info->mdio_bus;                //bus type
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


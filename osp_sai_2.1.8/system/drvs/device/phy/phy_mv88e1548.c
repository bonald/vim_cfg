/****************************************************************************
* $Id$
* phy 88E1548 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-11-24
* Reason        : First Create.
****************************************************************************/

#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_mv88e1548.h"
#include "ctc_api.h"
#include "madApiInternal.h"
#include "madDrvPhyRegs.h"
#include "madHwCntl.h"


MAD_BOOL _mad_mv88e1548_readMii(MAD_DEV* dev, unsigned int phyAddr, unsigned int miiReg, unsigned int* value)
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


MAD_BOOL _mad_mv88e1548_writeMii(MAD_DEV* dev, unsigned int phyAddr, unsigned int miiReg, unsigned int value)
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
mv88e1548_reg_read(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16* value)
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
mv88e1548_reg_write(phy_handle_t* phy_handle, phyreg_param_t* phyreg_param, uint16 value)
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

static int32 mv88e1548_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    MAD_STATUS ret = MAD_OK;
    MAD_U32 mode = 0x0;
    MAD_U16 media_mode = 0x0;
    phy_info_t* pphy_info;
    pphy_info = &phy_handle->phy_info;

    ret = mdSysGetMediaMode(phy_handle->mad_dev,
                                                mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                                &media_mode);
    if ((media_mode == MAD_MEDIA_QSGMII_2_1000X)||(media_mode == MAD_MEDIA_QSGMII_2_SGMII))
    {
        if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
        {
            mode = MAD_FIBER_AUTO_AD_1000FDX;
            ret += mdFiberSetAutoNeg(phy_handle->mad_dev, 
                                                           mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                                           MAD_TRUE,
                                                           mode);
        }
        else
        {
            mode = MAD_PHY_1000FDX;
            ret += mdFiberSetAutoNeg(phy_handle->mad_dev, 
                                                           mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                                           MAD_FALSE,
                                                           mode);
        }
    }
    else
    {
        mode = MAD_PHY_100FDX;
        ret += mdFiberSetAutoNeg(phy_handle->mad_dev, 
                                                  mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                                  MAD_FALSE,
                                                  mode);
    }


    /*sync qsgmii phy-mac auto neg*/
    ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                               phy_handle->phy_info.phy_addr%4,
                                                               MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 12, 1, 1);
    /* for bug 54114, modified by tongzb, 2019-11-07 */
    /* sync qsgmii phy-mac power enable */
    ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,phy_handle->phy_info.phy_addr%4,
                                        MAD_PAGE1, MAD_REG_FIBER_CONTROL, 11, 1, pphy_info->phy_manage_info.enable?0:1);

    /* sync loopback status */
    if (phy_handle->phy_info.phy_manage_info.lb_mode != GLB_LB_NONE)
    {
        phy_handle->config_loopback(phy_handle, phy_handle->phy_info.phy_manage_info.lb_mode);
    }

    if (ret == MAD_OK)
        return RESULT_OK;
    else
        return RESULT_ERROR;


    return ret;
}

int32 mv88e1548_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1548 phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = mv88e1548_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

int32 mv88e1548_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1548 phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = mv88e1548_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

int32 mv88e1548_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    MAD_STATUS ret = MAD_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "MV88E1548 phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);

    switch (lb_mode)
    {
    case GLB_LB_NONE:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_SPEC_CTRL_2, 7, 1, 0);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 15, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 10, 1, 0);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 14, 1, 0);

        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 8, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 12, 1, 0);
        break;

    case GLB_LB_PHY_FAR:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_SPEC_CTRL_2, 7, 1, 0);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 15, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 10, 1, 0);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 14, 1, 0);

        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 8, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 12, 1, 1);
        break;

    case GLB_LB_PHY_NEAR:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_SPEC_CTRL_2, 7, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 15, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 10, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 14, 1, 1);

        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 8, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 12, 1, 0);
        break;

    case GLB_LB_PHY_NEAR_DIAG:
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_SPEC_CTRL_2, 7, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 15, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 10, 1, 1);
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                 phy_handle->phy_info.phy_addr%4,
                                                                 MAD_PAGE1, MAD_REG_CONTROL, 14, 1, 1);

        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 8, 1, 0);
        ret += madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                                  phy_handle->phy_info.phy_addr%4,
                                                                  MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 12, 1, 0);
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

int32 mv88e1548_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    if (media_if == GLB_MEDIA_1000BX)
    {
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        0xfe, 19, 7, 2, 0x0);
        mdSysSetMediaMode(phy_handle->mad_dev,
                                           mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                           MAD_MEDIA_QSGMII_2_1000X);
    }
    else if (media_if == GLB_MEDIA_FI_100FX)
    {
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        0xfe, 19, 7, 2, 0x1);
        mdSysSetMediaMode(phy_handle->mad_dev,
                                           mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                           MAD_MEDIA_QSGMII_2_100FX);
    }
    else if (media_if == GLB_MEDIA_PASS_THROUGH)
    {
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        0xfe, 19, 7, 2, 0x0);
        mdSysSetMediaMode(phy_handle->mad_dev,
                                           mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                           MAD_MEDIA_QSGMII_2_SGMII);
        /* set QSGMII mode configure to SGMII media */
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        MAD_PAGE4, MAD_REG_QSGMII_CONTROL_1, 0, 1, 0x1);
        /* set QSGMII speed select 1000M */
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 13, 1, 0x0);
        madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                        phy_handle->phy_info.phy_addr%4,
                                                        MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 6, 1, 0x1);     
    }

    phy_handle->config_enable(phy_handle, 0);
    sal_udelay(20000);
    phy_handle->config_enable(phy_handle, phy_handle->phy_info.phy_manage_info.enable);

    /* config force or auto */
    mv88e1548_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed, phy_handle->phy_info.phy_manage_info.duplex);

    /* sync loopback status */
    if (phy_handle->phy_info.phy_manage_info.lb_mode != GLB_LB_NONE)
    {
        phy_handle->config_loopback(phy_handle, phy_handle->phy_info.phy_manage_info.lb_mode);
    }

    return RESULT_OK; 
}

int32
mv88e1548_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    return RESULT_OK;
}

/*add by chenjr for bug 53838,for manual configure led mode*/
int32
mv88e1548_config_ledctrl(phy_handle_t*phy_handle,uint8 first_mode ,uint8 second_mode)
{
    MAD_U16 reg_val = 0;
    MAD_U8 led_mode[2] = {0x8,0x1};
    MAD_U8 shift = 0; 
    MAD_U8 led_ctrl = 0;
    phyreg_param_t phyreg_param;
    
	
    if(NULL == phy_handle)
    {
       return RESULT_ERROR;
    }

    led_ctrl = phy_handle->phy_info.port_led_ctrl;
    shift = ((led_ctrl & 0xf0) >> 4)*4 ;
    reg_val =  (uint16)led_mode[first_mode] << shift;
    shift = (led_ctrl & 0x0f)*4 ;
    reg_val |=  (uint16)led_mode[second_mode] << shift;
	
    memset(&phyreg_param, 0, sizeof(phyreg_param_t));

    phyreg_param.dat.regaddr8 = 0x16;
    mv88e1548_reg_write(phy_handle, &phyreg_param, 0x3);
    phyreg_param.dat.regaddr8 = 0x10;
    mv88e1548_reg_write(phy_handle , &phyreg_param, reg_val);
    phyreg_param.dat.regaddr8 = 0x16;
    mv88e1548_reg_write(phy_handle , &phyreg_param, 0x0);

    return RESULT_OK;
}

int32 mv88e1548_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex)
{
    /* for system side power enable */
    madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 11, 1, 0);
    return RESULT_OK;
}

int32 
mv88e1548_link_down_setting(phy_handle_t* phy_handle)
{
    /* for system side power disable */
    /* do not disable system side power always, we keep system side always power on */
    //madHwSetPagedPhyRegField(phy_handle->mad_dev,
    //                                                phy_handle->phy_info.phy_addr%4,
    //                                                MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 11, 1, 1);
    return RESULT_OK;
} 

static int32 
mv88e1548_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
    MAD_STATUS ret = MAD_OK;
    MAD_U16 link_status = 0x0;
    MAD_U16 speed_status = 0x0;
    MAD_U16 duplex_status = 0x0;
    MAD_U16 is_resolved = 0x0;
    
    if(NULL == phy_handle || NULL == phy_val)
    {
        return RESULT_ERROR;
    }
    
    if (phy_handle->phy_info.phy_manage_info.lb_mode == GLB_LB_PHY_NEAR || phy_handle->phy_info.phy_manage_info.lb_mode == GLB_LB_PHY_NEAR_DIAG)
    {
        phy_val->link_up = GLB_LINK_UP;
        phy_val->speed = (phy_handle->phy_info.phy_manage_info.speed == GLB_SPEED_AUTO) ? GLB_SPEED_1G : phy_handle->phy_info.phy_manage_info.speed;
        phy_val->duplex = (phy_handle->phy_info.phy_manage_info.duplex == GLB_DUPLEX_AUTO) ? GLB_DUPLEX_FULL : phy_handle->phy_info.phy_manage_info.duplex;
        phy_val->linkup_media = GLB_PORT_TYPE_FIBER;
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        return RESULT_OK;
    }

    ret += madHwGetPagedPhyRegField(phy_handle->mad_dev,
                                                               phy_handle->phy_info.phy_addr%4,
                                                               MAD_PAGE1,MAD_REG_FIBER_STATUS_1, 10, 1, &link_status);
    phy_val->link_up = (link_status == 1) ? GLB_LINK_UP : GLB_LINK_DOWN;

    /*if port is up, get real media*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        ret += madHwGetPagedPhyRegField(phy_handle->mad_dev,
                                                                   phy_handle->phy_info.phy_addr%4,
                                                                   MAD_PAGE1,MAD_REG_FIBER_STATUS_1, 11, 1, &is_resolved);
        if(is_resolved)
        {
            ret += madHwGetPagedPhyRegField(phy_handle->mad_dev,
                                                                       phy_handle->phy_info.phy_addr%4,
                                                                       MAD_PAGE1,MAD_REG_FIBER_STATUS_1, 14, 2, &speed_status);
            if(speed_status == 0)
            {
                phy_val->speed = GLB_SPEED_10M;
            }
            else if(speed_status == 1)
            {
                phy_val->speed = GLB_SPEED_100M;
            }
            else if(speed_status == 2)
            {
                phy_val->speed = GLB_SPEED_1G;
            }   

            ret += madHwGetPagedPhyRegField(phy_handle->mad_dev,
                                                                       phy_handle->phy_info.phy_addr%4,
                                                                       MAD_PAGE1,MAD_REG_FIBER_STATUS_1, 13, 1, &duplex_status);
            if(duplex_status == 0)
            {
                phy_val->duplex = GLB_DUPLEX_HALF;
            }
            else if(duplex_status == 1)
            {
                phy_val->duplex = GLB_DUPLEX_FULL;
            }
        }
        else
        {
            phy_val->speed = GLB_SPEED_AUTO;
            phy_val->duplex = GLB_DUPLEX_AUTO;
            phy_val->link_up = GLB_LINK_DOWN;
        }

        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
        phy_val->linkup_media = GLB_PORT_TYPE_FIBER;
    }
    else
    {
        phy_val->speed = GLB_SPEED_AUTO;
        phy_val->duplex = GLB_DUPLEX_AUTO;
        phy_val->linkup_media = GLB_PORT_TYPE_FIBER; 
        phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
        phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    }

    return ret;
}

int32 mv88e1548_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    mv88e1548_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E1548 phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        mv88e1548_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "MV88E1548 phy addr %d link down",pphy_info->phy_addr) 
        mv88e1548_link_down_setting(phy_handle);
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
            mv88e1548_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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
mv88e1548_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;
    MAD_INT_TYPE interrupt_status;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;

    /*if NO link change, return, read clean interrupt*/
    mdIntGetStatus(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                               &interrupt_status);
    if(interrupt_status.intGroup0 == 0 && interrupt_status.intGroup1 == 0)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }
    
    mv88e1548_get_cur_status(phy_handle, phy_val);

    DRV_LOG_DEBUG(DRV_PHY,"mv88e1548_get_cur_status speed: %d duplex: %d, old_link %d, new_link %d",
        phy_val->speed, phy_val->duplex,pphy_info->phy_stat_flag.link_up,phy_val->link_up);
    
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

        mv88e1548_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);

    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {        
        phy_change->action = 0;
        mv88e1548_link_down_setting(phy_handle);
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
            mv88e1548_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex);
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


static int32
mv88e1548_enable(phy_handle_t* phy_handle, int8 enable)
{
    phy_info_t* pphy_info;    
    int32 ret = MAD_OK;
    MAD_BOOL en = MAD_FALSE;

    if(NULL == phy_handle)
    {

        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "MV88E1548 phy addr %d enable %d: speed %d duplex %d", 
                pphy_info->phy_addr, enable, pphy_info->phy_manage_info.speed, 
                pphy_info->phy_manage_info.duplex);

    if (enable)
    {
        en = MAD_TRUE;
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 11, 1, 0);
    }
    else
    {
        en = MAD_FALSE;
        ret = madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE4, MAD_REG_QSGMII_CONTROL, 11, 1, 1);
    }
    
    ret += mdSysSetPhyEnable(phy_handle->mad_dev,
                                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
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
int32 mv88e1548_init(phy_handle_t* phy_handle)
{
    MAD_SYS_CONFIG mad_sys_config;
    MAD_STATUS status = MAD_OK;

#if 0
    mv88e1548_drv_write(phy_handle, 22, 0x00ff);
    mv88e1548_drv_write(phy_handle, 17, 0x2148);
    mv88e1548_drv_write(phy_handle, 16, 0x2144);
    mv88e1548_drv_write(phy_handle, 17, 0x0c28);
    mv88e1548_drv_write(phy_handle, 16, 0x2146);
    mv88e1548_drv_write(phy_handle, 17, 0xb233);
    mv88e1548_drv_write(phy_handle, 16, 0x214d);
    mv88e1548_drv_write(phy_handle, 17, 0xdc0c);
    mv88e1548_drv_write(phy_handle, 16, 0x2159);
    mv88e1548_drv_write(phy_handle, 22, 0x00fb);
    mv88e1548_drv_write(phy_handle,  7, 0xc00d);
    mv88e1548_drv_write(phy_handle, 22, 0x0000);
#endif 

    phy_handle->mad_dev = (MAD_DEV *)DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(MAD_DEV));
    if(NULL == phy_handle->mad_dev)
    {
        DRV_LOG_ERR("malloc failed");
        goto err_out;
    }
    memset(&mad_sys_config, 0, sizeof(MAD_SYS_CONFIG));
    mad_sys_config.smiBaseAddr = phy_handle->phy_info.phy_addr - phy_handle->phy_info.phy_addr%4;
    mad_sys_config.port = phy_handle->phy_info.phy_addr%4;
    mad_sys_config.switchType = MAD_SYS_SW_TYPE_NO;
    mad_sys_config.phdl = phy_handle;
    mad_sys_config.BSPFunctions.readMii = _mad_mv88e1548_readMii;
    mad_sys_config.BSPFunctions.writeMii = _mad_mv88e1548_writeMii;
    mad_sys_config.BSPFunctions.readXMii = NULL;
    mad_sys_config.BSPFunctions.writeXMii = NULL;
    mad_sys_config.BSPFunctions.semCreate = NULL;
    mad_sys_config.BSPFunctions.semDelete = NULL;
    mad_sys_config.BSPFunctions.semTake = NULL;
    mad_sys_config.BSPFunctions.semGive = NULL;
    if ((status = mdLoadDriver(&mad_sys_config, phy_handle->mad_dev)) != MAD_OK)
    {
        DRV_LOG_ERR("init phy failed");
        //return RESULT_OK;
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

int32 mv88e1548_post_init(phy_handle_t* phy_handle)
{
    MAD_INT_TYPE interrupt_status;
    MAD_INT_TYPE interrupt_type;
    MAD_LED_CTRL_DATA led_mode;

    /* force MAC interface always power up */
    madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE1, MAD_REG_FIBER_CONTROL_1, 3, 1, 1);

    /* enable noise filtering in 1000BASE-X and 100BASE-FX */
    madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE1, MAD_REG_FIBER_SPEC_CTRL_2, 13, 2, 0x3);

    /* set default mode to MAD_MEDIA_QSGMII_2_1000X */
    madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    0xfe, 19, 7, 2, 0x0);
    mdSysSetMediaMode(phy_handle->mad_dev,
                                       mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                       MAD_MEDIA_QSGMII_2_1000X);

    /*config force or auto*/
    mv88e1548_phy_conf_set(phy_handle, phy_handle->phy_info.phy_manage_info.speed, phy_handle->phy_info.phy_manage_info.duplex);

    /*Clear interrupts*/
    mdIntGetStatus(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                               &interrupt_status);

    /* open interrupt enable fiber: speed , duplex ,link status */
    madHwSetPagedPhyRegField(phy_handle->mad_dev,
                                                    phy_handle->phy_info.phy_addr%4,
                                                    MAD_PAGE4, MAD_REG_QSGMII_INT_ENABLE, 15, 1, 0x1);
    interrupt_type.intGroup0 = 0;
    interrupt_type.intGroup1 = MAD_FIBER_SPEED_CHANGED | MAD_FIBER_DUPLEX_CHANGED | MAD_FIBER_LINK_STATUS_CHANGED;
    mdIntSetEnable(phy_handle->mad_dev,
                               mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                               &interrupt_type);
                               
    /* config led mode */
    if(GLB_PHY_LED_16 == phy_handle->phy_info.led_type)
    {
        memset(&led_mode, 0, sizeof(MAD_LED_CTRL_DATA));        
        led_mode.ledCtrl[0] = 0xf;
        led_mode.ledPolarity[0] = 0x0;
        led_mode.ledPolarity[1] = 0x0;
        led_mode.ledPulseDuration = 0x5;
        led_mode.ledBlinkRate = 0x4;

        mdSysSetLEDCtrl(phy_handle->mad_dev,
                                    mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                    &led_mode);
    }
    
    /*add by chenjr for bug 53838*/
    if(GLB_PHY_LED_8 == phy_handle->phy_info.led_type)
       {
           memset(&led_mode, 0, sizeof(MAD_LED_CTRL_DATA));        
           led_mode.ledCtrl[0] = 0x1;
           led_mode.ledCtrl[3] = 0x8;
           led_mode.ledPolarity[0] = 0x0;
           led_mode.ledPolarity[3] = 0x0;
           led_mode.ledPulseDuration = 0x5;
           led_mode.ledBlinkRate = 0x4;

           mdSysSetLEDCtrl(phy_handle->mad_dev,
                                       mad_port2lport(phy_handle->mad_dev->validPortVec, phy_handle->phy_info.phy_addr%4),
                                       &led_mode);
       }

    /*init to power down */
    phy_handle->config_enable(phy_handle, 0);

    return RESULT_OK;
}

phy_handle_t* phy_mv88e1548_dev_register(phy_info_t* pphy_info)
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
    memset(phdl, 0, sizeof(phy_handle_t));
    memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));
    phdl->phy_init = mv88e1548_init;
    phdl->phy_post_init = mv88e1548_post_init;
    phdl->config_enable = mv88e1548_enable;
    phdl->config_speed = mv88e1548_config_speed;
    phdl->config_duplex = mv88e1548_config_duplex;
    phdl->config_loopback = mv88e1548_config_loopback;
    phdl->config_medium = mv88e1548_config_medium;
    phdl->config_flowctrl = mv88e1548_config_flowctrl;
    phdl->config_led_mode= mv88e1548_config_ledctrl;
    phdl->get_link_poll = mv88e1548_get_link_poll;
    phdl->get_link_interupt = mv88e1548_get_link_interrput;
    phdl->get_cur_status = mv88e1548_get_cur_status;
    phdl->reg_read = mv88e1548_reg_read;
    phdl->reg_write = mv88e1548_reg_write;

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


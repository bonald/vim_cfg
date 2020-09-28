/****************************************************************************
* $Id$
* phy aqr40x device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Liuht
* Date          : 2016-06-25
* Reason        : First Create.
*                
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_aqr40x.h"
#include "AQ_API.h"
#include "AQ_PhyInterface.h"
#include "AQ_Flash_Internal.h"
#include "ctc_api.h"

static int32 
aqr40x_reg_read(phy_handle_t* phy_handle, phyreg_param_t* op_param, uint16* value)
{
    *value=AQ_API_MDIO_Read(  phy_handle->phy_info.port_num,op_param->dev_no, op_param->dat.regaddr16);
    return RESULT_OK;
}

static int32 
aqr40x_reg_write(phy_handle_t* phy_handle, phyreg_param_t* op_param, uint16 value)
{
    if(RESULT_OK==AQ_API_MDIO_Write(phy_handle->phy_info.port_num, op_param->dev_no, op_param->dat.regaddr16, value))
        return RESULT_OK;
    else
        return RESULT_ERROR;
}

static int32
aqr40x_write_mask(phy_handle_t* phy_handle, uint8 dev, uint16 reg, uint16 value, uint16 mask)
{
    phyreg_param_t phyreg_para;
    uint16 val;
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    phyreg_para.dev_no=dev;
    phyreg_para.dat.regaddr16 = reg;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    value = (value & mask) | (val & ~mask);
    ret += phy_handle->reg_write(phy_handle, &phyreg_para, value);

    return ret;
}

static int32
aqr40x_get_eee_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
#if 0
    phyreg_param_t phyreg_para;
    int32 ret = 0;
    uint16 value;

    /* 1. get local eee capability */
    phyreg_para.dev_no=0x7;
    phyreg_para.dat.regaddr16 = 0x3c;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    if((value & 0x0040) == 0x40)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KR;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KR;
    }
    if((value & 0x0020) == 0x20)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KX4;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KX4;
    }
    if((value & 0x0010) == 0x10)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_KX;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_KX;
    }
    if((value & 0x0008) == 0x8)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_T;
    }
    if((value & 0x0004) == 0x4)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T;
    }
    if((value & 0x0002) == 0x2)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX;
    }
    phyreg_para.dev_no=0x7;
    phyreg_para.dat.regaddr16 = 0x3e;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    if((value & 0x0001) == 0x1)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_2G5BASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_2G5BASE_T;
    }
    if((value & 0x0002) == 0x2)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_CAPABILITY_5GBASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_CAPABILITY_5GBASE_T;
    }

    /* 2. get link partner eee capability */
    phyreg_para.dev_no=0x3;
    phyreg_para.dat.regaddr16 = 0x3d;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);  
    if((value & 0x0040) == 0x40)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KR;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KR;
    }
    if((value & 0x0020) == 0x20)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KX4;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KX4;
    }
    if((value & 0x0010) == 0x10)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_KX;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_KX;
    }
    if((value & 0x0008) == 0x8)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_T;
    }
    if((value & 0x0004) == 0x4)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_T;
    }
    if((value & 0x0002) == 0x2)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_100BASE_TX;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_100BASE_TX;
    }
    phyreg_para.dev_no=0x3;
    phyreg_para.dat.regaddr16 = 0x3f;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);  
    if((value & 0x0001) == 0x01)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_2G5BASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_2G5BASE_T;
    }
    if((value & 0x0002) == 0x02)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_5GBASE_T;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_5GBASE_T;
    }

    /* 3. get LPI status */
    phyreg_para.dev_no=0x3;
    phyreg_para.dat.regaddr16 = 1;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);    
    if((value & 0x0004) == 0x4)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_LINK_STATUS;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_LINK_STATUS;
    }
    if((value & 0x0100) == 0x0100)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_RX_LPI_STATUS;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_RX_LPI_STATUS;
    }
    if((value & 0x0200) == 0x0200)
    {
        phy_val->eee_status.status |= GLB_PHY_EEE_LOCAL_TX_LPI_STATUS;
    }
    else
    {
        phy_val->eee_status.status &= ~GLB_PHY_EEE_LOCAL_TX_LPI_STATUS;
    }

    phyreg_para.dev_no=0x3;
    phyreg_para.dat.regaddr16 = 0x16;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    phy_val->eee_status.wake_error_count = value & 0xffff;

    return ret;
#endif
    return 0;
}


static int32 
aqr40x_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{   
    phyreg_param_t phyreg_para;
    int32 ret = 0;
    uint16 value,status;

    if(NULL == phy_handle || NULL == phy_val)
    {
        return RESULT_ERROR;
    }
    
    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    phyreg_para.dev_no=0x7;
    phyreg_para.dat.regaddr16 = 0xc810;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &status);
    if(((status&0x3e00)>>9)==0x7)
    {
        phy_val->link_up = GLB_LINK_UP;
        phyreg_para.dev_no=0x4;
        phyreg_para.dat.regaddr16 = 0xc444;
        ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
        if((value&0x7) == 0x01)
        {
            phy_val->speed = GLB_SPEED_100M;
        }
        else if((value&0x7) == 0x02)
        {
            phy_val->speed = GLB_SPEED_1G;
        }
        else if((value&0x7) == 0x03)
        {
            phy_val->speed = GLB_SPEED_10G;
        }
        else if((value&0x7) == 0x04)
        {
            phy_val->speed = GLB_SPEED_2_5G;
        }
        else if((value&0x7) == 0x05)
        {
            phy_val->speed = GLB_SPEED_5G;
        }
        else
        {
            phy_val->speed = GLB_SPEED_10G;
        }
        phy_val->duplex = GLB_DUPLEX_FULL;
    }
    else
    {
        phy_val->link_up = (((status&0x3e00)>>9)==0x4) ? GLB_LINK_UP : GLB_LINK_DOWN;
        if(phy_val->link_up == GLB_LINK_UP)
        {
            phyreg_para.dev_no=0x7;
            phyreg_para.dat.regaddr16 = 0xc800;
            ret += phy_handle->reg_read(phy_handle, &phyreg_para, &value);
            
            if(((value&0xf)>>1)== 0x00)
            {
                phy_val->speed = GLB_SPEED_10M;
            }
            else if(((value&0xf)>>1) == 0x01)
            {
                phy_val->speed = GLB_SPEED_100M;
            }
            else if(((value&0xf)>>1)== 0x2)
            {
                phy_val->speed = GLB_SPEED_1G;
            }   
            else if(((value&0xf)>>1)== 0x3)
            {
                phy_val->speed = GLB_SPEED_10G;
            }
            else if(((value&0xf)>>1)== 0x4)
            {
                phy_val->speed = GLB_SPEED_2_5G;
            }
            else if(((value&0xf)>>1)== 0x5)
            {
                phy_val->speed = GLB_SPEED_5G;
            }

            if((value&0x1) == 0)
            {
                phy_val->duplex = GLB_DUPLEX_HALF;
            }
            else if((value&0x1) == 1)
            {
                phy_val->duplex = GLB_DUPLEX_FULL;
            }
            ret += aqr40x_get_eee_status(phy_handle, phy_val);
    	}
        else
        {
            phy_val->speed = GLB_SPEED_AUTO;
            phy_val->duplex = GLB_DUPLEX_AUTO;
            phy_val->eee_status.status= 0;
            phy_val->eee_status.wake_error_count = 0;
        }
    }
    phy_val->linkup_media = GLB_PORT_TYPE_COPPER; 
    phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
    phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;

    return ret;
}

int32 
aqr40x_link_up_setting(phy_handle_t* phy_handle, 
                                glb_port_speed_t speed, 
                                glb_port_duplex_t duplex,
                                glb_port_interface_t media)
{
    int ret=0;
    phy_info_t *pphy_info;
    ctc_chip_serdes_info_t serdes_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    serdes_info.serdes_id = pphy_info->port_info.serdes_id;
    ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
    if((GLB_SPEED_10M == speed) || (GLB_SPEED_100M == speed) || (GLB_SPEED_1G== speed))
    {
        if(CTC_CHIP_SERDES_SGMII_MODE != pphy_info->serdes_mode)
        {
            /* Change serdes to sgmii mode */
            serdes_info.serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            ret += ctc_chip_set_serdes_mode(pphy_info->port_info.lchip, &serdes_info);
            if(!ret)
            {
                pphy_info->serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            }
        }
        /* modified by tongzb for bug 43005 */
        ret += ctc_port_set_auto_neg(pphy_info->port_info.port_id, 1);
        /* enable system side autonegotiation */
        ret += aqr40x_write_mask(phy_handle, 30, 0x31b, 0x0008, 0x0008);
    }
    else if(GLB_SPEED_2_5G == speed)
    {
        /* Change serdes to xfi mode */
        if(CTC_CHIP_SERDES_2DOT5G_MODE != pphy_info->serdes_mode)
        {
            /* Change serdes to sgmii mode */
            serdes_info.serdes_mode = CTC_CHIP_SERDES_2DOT5G_MODE;
            ret += ctc_chip_set_serdes_mode(pphy_info->port_info.lchip, &serdes_info);
            if(!ret)
            {
                pphy_info->serdes_mode = CTC_CHIP_SERDES_2DOT5G_MODE;
            }
        }
        /* modified by tongzb for bug 43005 */
        ret += ctc_port_set_auto_neg(pphy_info->port_info.port_id, 0);
        /* disable system side autonegotiation */
        ret += aqr40x_write_mask(phy_handle, 30, 0x31b, 0x0000, 0x0008);
    }       
    else if(GLB_SPEED_5G == speed)
    {
        if(CTC_CHIP_SERDES_XFI_MODE != pphy_info->serdes_mode)
        {
            /* Change serdes to xfi mode */
            serdes_info.serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            ret += ctc_chip_set_serdes_mode(pphy_info->port_info.lchip, &serdes_info);
            if(!ret)
            {
                pphy_info->serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            }
        }
        ctc_port_fc_prop_t fc;
        fc.gport = pphy_info->port_info.port_id;
        fc.is_pfc = FALSE;
        fc.priority_class   = 0;/* tongzb */
        fc.enable = 1;
        fc.dir = CTC_BOTH_DIRECTION;
        ret += ctc_port_set_flow_ctl_en(&fc);
    }
    else
    {
        if(CTC_CHIP_SERDES_XFI_MODE != pphy_info->serdes_mode)
        {
            /* Change serdes to xfi mode */
            serdes_info.serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            ret += ctc_chip_set_serdes_mode(pphy_info->port_info.lchip, &serdes_info);
            if(!ret)
            {
                pphy_info->serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            }
        }
    }
    ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
    /* modified by tongzb for bug 43005 */
    if(phy_handle->phy_info.led_type == GLB_PHY_LED_11)
    {
        if(GLB_SPEED_10G == speed)
        {
            /* Led0 for 10G mode link and rxtxactivity */
            ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0x008f, 0xffff);
            /* Led 1 for 5G/2.5G/1G/100M force off */
            ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0x0000, 0xffff);
        }
        else
        {
            /* Led0 for 10G mode force off */
            ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0x0000, 0xffff);
            /* Led 1 for 5G/2.5G/1G/100M link and rxtxactivity */
            ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0xc06f, 0xffff);
        }
    }
    else if(phy_handle->phy_info.led_type == GLB_PHY_LED_10)
    {
        /* Led 0 for 10G/5G/2.5G/1G/100M link and rxtxactivity */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0xc0ef, 0xffff);
        /* Led 1 force off */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0x0000, 0xffff);
    }
    
    return RESULT_OK;
}

int32 
aqr40x_link_down_setting(phy_handle_t* phy_handle)
{
    int ret=0;
    if(phy_handle->phy_info.led_type == GLB_PHY_LED_11)
    {
        /* Led0 for 10G mode link and rxtxactivity */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0x0000, 0xffff);
        /* Led 1 for 5G/2.5G/1G/100M link and rxtxactivity */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0x0000, 0xffff);
    }
    else if(phy_handle->phy_info.led_type == GLB_PHY_LED_10)
    {
        /* Led 0 for 10G/5G/2.5G/1G/100M link and rxtxactivity */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0x0000, 0xffff);
        /* Led 1 force off */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0x0000, 0xffff);
    }

    return RESULT_OK;
} 

int32
aqr40x_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    aqr40x_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "AQR40X phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        aqr40x_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "AQR40X phy addr %d link down",pphy_info->phy_addr) 
        aqr40x_link_down_setting(phy_handle);
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
            aqr40x_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
        }
        if(pphy_info->phy_stat_flag.eee_status.status != phy_val->eee_status.status ||
            pphy_info->phy_stat_flag.eee_status.wake_error_count != phy_val->eee_status.wake_error_count)
        {
            phy_change->eee_status_change = 1;
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
    pphy_info->phy_stat_flag.eee_status.status= phy_val->eee_status.status;
    pphy_info->phy_stat_flag.eee_status.wake_error_count = phy_val->eee_status.wake_error_count;
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }
    return RESULT_OK;
}

int32
aqr40x_get_link_interrput(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;
    AQ_API_Port * port;
    AQ_API_AlarmStatus alarmStatus;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }

    /* if no state change, just return */
    port = (AQ_API_Port *)(phy_handle->phy_info.data); 
    AQ_API_GetAlarmStatus(port, &alarmStatus);

    if(alarmStatus.globalAlarms.stateChange == False)
    {
        phy_change->action = -1;
        return RESULT_OK;
    }

    pphy_info = &phy_handle->phy_info;
    aqr40x_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        DRV_LOG_DEBUG(DRV_PHY, "AQR40X phy addr %d link up",pphy_info->phy_addr) 

        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }

        aqr40x_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "AQR40X phy addr %d link down",pphy_info->phy_addr) 
        aqr40x_link_down_setting(phy_handle);
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
            aqr40x_link_up_setting(phy_handle, phy_val->speed, phy_val->duplex, phy_val->linkup_media);
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
aqr40x_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    int32 ret = AQ_RET_OK;
    phy_info_t *pphy_info;
    AQ_API_Port *port;
    AQ_API_FWManagedLoopbackControl loopback;
    AQ_API_FWManagedLoopbackRate rate = AQ_FWMLRate_10G;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }

    port = (AQ_API_Port *)(phy_handle->phy_info.data);

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "AQR40X phy addr %d config loopback mode %d", pphy_info->phy_addr, lb_mode);
    
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        loopback = AQ_FWMLC_No_LB;
        break;

    case GLB_LB_PHY_FAR:
        loopback = AQ_FWMLC_SysIntf_NetLB;
        break;

    case GLB_LB_PHY_NEAR:
        loopback = AQ_FWMLC_NetIntf_SysLB;
        break;

    case GLB_LB_PHY_NEAR_DIAG:
        loopback = AQ_FWMLC_NetIntf_SysLB;
        break;

    default:
        loopback = AQ_FWMLC_No_LB;
        break;
    }

    ret = AQ_API_SetFWManagedLoopbackControl(port, loopback, rate);
    if(ret != AQ_RET_OK)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

static int32
aqr40x_enable(phy_handle_t* phy_handle, int8 enable)
{
    int ret = RESULT_OK;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }
    
    if(enable)
    {
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0x0000, 0x0000, 0x0800);
        /* enter normal mode */
        if(phy_handle->phy_info.phy_manage_info.lb_mode != GLB_LB_NONE)
        {
            aqr40x_config_loopback(phy_handle,phy_handle->phy_info.phy_manage_info.lb_mode);
        }
    }
    else
    {
        /* enter low-power mode with only the MDIO and microprocessor functioning  */
        ret += aqr40x_write_mask(phy_handle, 0x1e, 0x0000, 0x0800, 0x0800);
    }
    if(ret)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

static int32 aqr40x_phy_speed_disable_all(phy_handle_t* phy_handle)
{
    int32 ret=0;
    if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
    {
        /* No advertise 40G capability */
        ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0000, 0x0800);
        /* No advertise 25G capability */
        ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0000, 0x0400);
    }
    /* No advertise 10G capability */
    ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0000, 0x1000);
    /* No advertise 5G capability */
    ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0000, 0x0800);
    if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
        ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0000, 0x0100);
    /* No advertise 2.5G capability */
    ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0000, 0x0400);
    if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
        ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0000, 0x0080);
    /* No advertise 1000M(full/half) capability */    
    ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0000, 0xc000);
    /* No advertise 100M capability */
    ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0000, 0x03e0);

    return ret;
}
static int32
aqr40x_phy_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    int32 ret=0;

    ret += aqr40x_write_mask(phy_handle, 7, 0, 0x0000, 0x1000);
    if(speed == GLB_SPEED_AUTO)
    {
        if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_10G)
        {
            /* Advertise 10G capability */    
            ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x1000, 0x1000); 
        }
        if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_1G)
        {
            /* Advertise 1000M(full) capability */    
            ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x8000, 0xc000);
        }
        if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_2_5G)
        {
            /* Advertise 2.5G capability */    
            ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0400, 0x0400);
            if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
                ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0080, 0x0080);
        }
        if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_5G)
        {
            /* Advertise 5G capability */    
            ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0800, 0x0800);
            if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
                ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0100, 0x0100);
        }
        if(duplex == GLB_DUPLEX_AUTO)
        {

            if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_100M)
            {
                /* Advertise 100M(full/half) capability */    
                ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0180, 0x03e0);
            }
        }
        else if(duplex == GLB_DUPLEX_FULL)
        {
            if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_100M)
            {
                /* Advertise 100M(full/half) capability */    
                ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0180, 0x03e0);
            }
        }
        else
        {
            if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_100M)
            {
                /* Advertise 100M(half) capability */
                ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0080, 0x03e0);
            }
        }
    }
    else
    {
        if(speed == GLB_SPEED_10G)
        {
            aqr40x_phy_speed_disable_all(phy_handle);
            /* Advertise 10G capability */
            ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x1000, 0x1000);
        }
        else if(speed == GLB_SPEED_5G)
        {
            aqr40x_phy_speed_disable_all(phy_handle);
            /* advertise 5G capability */
            ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0800, 0x0800);
            if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
                ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0100, 0x0100);

        }
        else if(speed == GLB_SPEED_2_5G)
        {
            aqr40x_phy_speed_disable_all(phy_handle);
            /* advertise 2.5G capability */
            ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0400, 0x0400);
            if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
                ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0080, 0x0080);

        }
        else if(speed == GLB_SPEED_1G)
        {
            if(duplex == GLB_DUPLEX_AUTO)
            {
                aqr40x_phy_speed_disable_all(phy_handle);
                /* advertise 1000M(full/half) capability */    
                ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0xc000, 0xc000);

            }
            else if(duplex == GLB_DUPLEX_FULL)
            {
                aqr40x_phy_speed_disable_all(phy_handle);                    
                /* Advertise 1000M(full) capability */
                ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x8000, 0xc000);
            }
            else
            {
                aqr40x_phy_speed_disable_all(phy_handle);                    
                /* Advertise 1000M(half) capability */
                ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x4000, 0xc000);
            }
        }
        else if(speed == GLB_SPEED_100M)
        {
            if(duplex == GLB_DUPLEX_AUTO)
            {
                aqr40x_phy_speed_disable_all(phy_handle);
                /* Advertise 100M(full/half) capability */
                ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0180, 0x03e0);
            }
            else if(duplex == GLB_DUPLEX_FULL)
            {
                aqr40x_phy_speed_disable_all(phy_handle);
                /* Advertise 100M(full) capability */
                ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0100, 0x03e0);
            }
            else
            {
                aqr40x_phy_speed_disable_all(phy_handle);
                /* Advertise 100M(half) capability */
                ret  += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0080, 0x03e0);
            }
        }
    }
    
    /* Restart Autonegotiation */
    ret += aqr40x_write_mask(phy_handle, 7, 0, 0x1000, 0x1000);
    /* modified by tongzb for bug 44531 */
    if(phy_handle->phy_info.phy_manage_info.enable)
    {
        ret += aqr40x_write_mask(phy_handle, 7, 0, 0x0200, 0x0200);
    }
    
    return ret;
}    

int32
aqr40x_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "AQR40X phy addr %d config speed %d, orig %d", pphy_info->phy_addr, speed, pphy_info->phy_manage_info.speed);
    ret = aqr40x_phy_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;
}

int32
aqr40x_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "AQR40X phy addr %d config duplex %d, orig %d", pphy_info->phy_addr, duplex, pphy_info->phy_manage_info.duplex);
    ret = aqr40x_phy_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;
}

int32
aqr40x_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    return RESULT_OK;
}

int32
aqr40x_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    return RESULT_OK;
}

int32 
aqr40x_set_vct_mod(phy_handle_t* phy_handle)
{
    int32 ret = AQ_RET_OK;
    AQ_API_Port *port;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_PHY,"aqr40x_set_vct_mod.");
    
    port = (AQ_API_Port *)(phy_handle->phy_info.data);
    ret = AQ_API_RunBasicCableDiags(port);
    if(ret == AQ_RET_OK)
        return RESULT_OK;
    else
        return RESULT_ERROR;
}

int32
aqr40x_vct_mode_remapping(AQ_API_BasicPairStatus status)
{
    switch(status)
    {
    case AQ_API_BasicPairStatus_OK:
        return GLB_PAIR_STATE_NORMAL;

    case AQ_API_BasicPairStatus_Short:
        return GLB_PAIR_STATE_ABNORMAL_SHORTED;

    case AQ_API_BasicPairStatus_LowMismatch:
        return GLB_PAIR_STATE_LOW_IMPEDANCE;

    case AQ_API_BasicPairStatus_HighMismatch:
        return GLB_PAIR_STATE_HIGH_IMPEDANCE;

    case AQ_API_BasicPairStatus_Open:
        return GLB_PAIR_STATE_ABNORMAL_OPEN;

    case AQ_API_BasicPairStatus_CrossoverA:
        return GLB_PAIR_STATE_CROSS_PAIR_SHORT_A;

    case AQ_API_BasicPairStatus_CrossoverB:
        return GLB_PAIR_STATE_CROSS_PAIR_SHORT_B;

    case AQ_API_BasicPairStatus_CrossoverC:
        return GLB_PAIR_STATE_CROSS_PAIR_SHORT_C;

    case AQ_API_BasicPairStatus_CrossoverD:
        return GLB_PAIR_STATE_CROSS_PAIR_SHORT_C;
    }
    return RESULT_ERROR;
}

int32 
aqr40x_get_vct_info(phy_handle_t* phy_handle, glb_port_cable_info_t* p_cable_info)
{
    int32 ret = AQ_RET_OK;
    AQ_API_Port *port;
    AQ_API_BasicCableDiagResults results;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }

    port = (AQ_API_Port *)(phy_handle->phy_info.data);
    ret = AQ_API_GetBasicCableDiagsResults(port, &results);
    if(ret == AQ_RET_CABLEDIAG_STILL_RUNNING)
    {
        return GLB_VCT_WAIT;
    }

    p_cable_info->pair_A.pair_length = results.pairAResult.reflection_1_Distance;
    p_cable_info->pair_B.pair_length = results.pairBResult.reflection_1_Distance;
    p_cable_info->pair_C.pair_length = results.pairCResult.reflection_1_Distance;
    p_cable_info->pair_D.pair_length = results.pairDResult.reflection_1_Distance;
    p_cable_info->pair_A.pair_status = aqr40x_vct_mode_remapping(results.pairAResult.status);
    p_cable_info->pair_B.pair_status = aqr40x_vct_mode_remapping(results.pairBResult.status);
    p_cable_info->pair_C.pair_status = aqr40x_vct_mode_remapping(results.pairCResult.status);
    p_cable_info->pair_D.pair_status = aqr40x_vct_mode_remapping(results.pairDResult.status);
    p_cable_info->pair_A.pair_accuracy = 1;
    p_cable_info->pair_B.pair_accuracy = 1;
    p_cable_info->pair_C.pair_accuracy = 1;
    p_cable_info->pair_D.pair_accuracy = 1;
    p_cable_info->port_link_up = 0;

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

    return GLB_VCT_OK;
}

int32 
aqr40x_enable_eee(phy_handle_t* phy_handle, int8 enable)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    
    DRV_LOG_DEBUG(DRV_PHY,
        "AQR40X phy addr %d config eee enable %d, orig %d", pphy_info->phy_addr, enable, pphy_info->phy_manage_info.eee_enable);

    if (GLB_EEE_ENABLE == enable)
    {
        ret = aqr40x_write_mask(phy_handle, 7, 0x003c, 0x000c, 0x000c);
    } 
    else
    {
        ret = aqr40x_write_mask(phy_handle, 7, 0x003c, 0x0000, 0x000c);
    }

    /* Restart Autonegotiation */
    ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0000, 0x0200);
    ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0200, 0x0200);
    
    return ret;
}

int32 
aqr40x_config_phy_sig_test_mode(phy_handle_t* phy_handle, uint8 serdes_id, uint8 mode)
{
    uint8 testPattern;
    AQ_API_Port *port;

    if((NULL == phy_handle)||(NULL == phy_handle->phy_info.data))
    {
        return RESULT_ERROR;
    }

    port = (AQ_API_Port *)(phy_handle->phy_info.data);

    testPattern = mode & 0xf;
    if((mode & 0xf0) == 0x10)
    {
        AQ_API_Set10G_LineTestPattern(port, testPattern);
    }
    else if((mode & 0xf0) == 0x20)
    {
        AQ_API_Set1G_LineTestPattern(port, testPattern);
    }
    else if((mode & 0xf0) == 0x30)
    {
        AQ_API_Set100M_LineTestPattern(port, testPattern);
    }
    
    return RESULT_OK;
}

//static AQ_API_Port *g_phy_aqr40x_pp_ports[PHY_AQR40X_MAX_NUM_PHY_IDS];
int aqr40x_check_flash_type(phy_handle_t* phy_handle, AQ_API_Port* port)
{
    uint16_t globalNVR_InterfacePrevious_word0;
    uint16_t globalNVR_ProvisioningPrevious_word0;
    uint16_t globalNVR_ProvisioningPrevious_word1;
    uint16_t globalControlPrevious_word1;
    AQ_API_FLASH flashType;
    AQ_Retcode retcode;
    
    if (NULL == port)
    {
        return RESULT_ERROR;
    }
    /* Take control of the FLASH and determine its type */
    AQ_TakeControlOfFLASH(port, &globalNVR_InterfacePrevious_word0, 
      &globalNVR_ProvisioningPrevious_word0, &globalNVR_ProvisioningPrevious_word1, 
      &globalControlPrevious_word1, True, AQ_FLASH_NVR_CLOCK_DIVIDE);
    
    retcode = AQ_API_DetermineFLASH_Type(port, &flashType, NULL);
    
    /* Return the PHY to its previous state */
    AQ_ReturnControlOfFLASH(port, &globalNVR_InterfacePrevious_word0, 
        &globalNVR_ProvisioningPrevious_word0, &globalNVR_ProvisioningPrevious_word1,
        &globalControlPrevious_word1);
    FILE *fp_console;
    fp_console = fopen("/dev/console", "a+"); 
    switch (flashType)
    {
        case AQ_API_F_AT25F512A:
            fprintf(fp_console, "Port %d: FLASH is Atmel 64K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_AT45DB041D:
            fprintf(fp_console, "Port %d: FLASH is Atmel Smart (Adesto) 512K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_AT45DB041E:
            fprintf(fp_console, "Port %d: FLASH is Atmel Smart (Adesto) 512K byte FLASH new revision.\n", port->PHY_ID);
            break;
        case AQ_API_F_M25P05A:
            fprintf(fp_console, "Port %d: FLASH is Numonyx Microelectronics 64K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_AT25DF021A:
            fprintf(fp_console, "Port %d: FLASH is Atmel 256K byte byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_AT25DF041A:
            fprintf(fp_console, "Port %d: FLASH is Atmel 512K byte byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_M25P40:
            fprintf(fp_console, "Port %d: FLASH is Numonyx 512K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_MX25V400:
        case AQ_API_F_MX25V4035F:
            fprintf(fp_console, "Port %d: FLASH is Macronix 512K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_AT45DB321E:
            fprintf(fp_console, "Port %d: FLASH is Adesto Smart 4M byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_MX25L32:
            fprintf(fp_console, "Port %d: FLASH is Macronix 4M byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_W25Q32:
            fprintf(fp_console, "Port %d: FLASH is Winbond 4M byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_W25Q16DW:
            fprintf(fp_console, "Port %d: FLASH is Winbond 2M byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_W25X40:
            fprintf(fp_console, "Port %d: FLASH is Winbond 512K byte FLASH.\n", port->PHY_ID);
            break;
        case AQ_API_F_DaisyChainSlave:
            fprintf(fp_console, "Port %d: FLASH is not present as this is a daisy-chain slave device.\n", port->PHY_ID);
            break;
        case AQ_API_F_Indeterminate:
        default:
            fprintf(fp_console, "Port %d: FLASH is either absent or the type is unknown.\n", port->PHY_ID);
            break;
    }
    if (AQ_RET_OK != retcode)      
    {
        fprintf(fp_console, "Port %d: Check Flash Type and return error_no %d\n", port->PHY_ID, retcode);
        fclose(fp_console);
        return RESULT_ERROR;
    }
    fclose(fp_console);
    return RESULT_OK;
}
int aqr40x_load_firmware(phy_handle_t* phy_handle, const char *pathname)
{
    uint32  i,len,count=0,ret;
    uint32  imageSize;   
    uint8    *image=NULL;
    AQ_API_Port   *p_port;    
    uint8 tmpbuf[512];
    FILE* fp=NULL;

    p_port = (AQ_API_Port*)DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(AQ_API_Port));
    if (NULL == p_port)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, p_port);
        DRV_LOG_ERR("calloc memory for port failed.");
        return RESULT_ERROR;
    }

    phy_handle->phy_info.data = (void*)p_port;
    p_port->device = AQ_DEVICE_HHD;
    p_port->PHY_ID = (uint32)phy_handle->phy_info.port_num;

    /* check flash type fisrt, this will redo in AQ_API_WriteAndVerifyFlashImage*/
    ret = aqr40x_check_flash_type(phy_handle,p_port);
    if(ret != RESULT_OK)
        return RESULT_ERROR;

    fp = sal_fopen(pathname, "rb");
    if(fp ==NULL) 
    {
        DRV_LOG_ERR("Open %s failed.",pathname);
        DRV_CONSOLE_PRINT( "Open %s failed.",pathname); 
        return RESULT_ERROR;
    }
    /*cal the length of image*/
    sal_fseek(fp,0L,SAL_SEEK_END);
    imageSize=sal_ftell(fp);
    sal_fclose(fp);
    image=(uint8 *)malloc(sizeof(uint8)*(imageSize+1));
    /*copy the value of image to *image*/
    fp = sal_fopen(pathname, "rb");
    if(fp ==NULL) 
    {
        DRV_LOG_ERR("Open %s failed.",pathname);
        DRV_CONSOLE_PRINT("Open %s failed.",pathname);
        return RESULT_ERROR;
    }
    while((len = sal_fread(tmpbuf, 1, 512, fp))>0)
    {
        for(i=0; i<len; i++)
        {
            image[count++] = tmpbuf[i];
        }        
    }
    sal_fclose(fp);

    ret = AQ_API_WriteAndVerifyFlashImage(p_port, &imageSize, image);
    DRV_CONSOLE_PRINT("AQ_API_WriteAndVerifyFlashImage:\tret=%d\n", ret);
    free(image);
    
    if(ret == AQ_RET_OK)    
        return RESULT_OK;
    else
        return RESULT_ERROR;
}

int check_firmware_version(phy_handle_t* phy_handle, firmware_ver_t *version)
{
    int ret=0;
    phyreg_param_t phyreg_para;
    uint16 val;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    sal_memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    phyreg_para.dev_no=30;
    phyreg_para.dat.regaddr16 = 0x20;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    version->major_num = ((val >> 8) & 0xff);
    version->minor_num = (val & 0xff);
    
    phyreg_para.dat.regaddr16 = 0xc885;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    version->build_id = ((val >> 4) & 0xf);
    version->provision_id = (val & 0xf);
    
    phyreg_para.dev_no=1;
    phyreg_para.dat.regaddr16 = 0xc41d;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    version->platform_id = val & 0xffff;

    phyreg_para.dat.regaddr16 = 0xc41e;
    ret += phy_handle->reg_read(phy_handle, &phyreg_para, &val);
    version->ver_id = val & 0xffff;

    DRV_CONSOLE_PRINT( "Firmware Release:v%d.%d.%d\nProvision ID:%d\nPlatform ID:%ld\nProvision Ver:%ld\n", 
          version->major_num,version->minor_num,version->build_id,version->provision_id,version->platform_id,version->ver_id);

    return 0;
}


uint8 *boot_image = NULL;
uint32 boot_imageSize = 0;
int bootimage_buffer(const char *pathname,uint8 *image_buf, uint32 *imageSize)
{
    uint8 tmpbuf[512];
    uint32  i,len,count=0;
    FILE* fp=NULL;

    fp = sal_fopen(pathname, "rb");
    if(fp ==NULL) 
    {
        DRV_LOG_ERR("Open %s failed.",pathname);
        DRV_CONSOLE_PRINT("Open %s failed.",pathname); 
        return RESULT_ERROR;
    }
    /*cal the length of image*/
    sal_fseek(fp,0L,SAL_SEEK_END);
    boot_imageSize=sal_ftell(fp);
    sal_fclose(fp);
    boot_image=(uint8 *)malloc(sizeof(uint8)*(boot_imageSize+1));
    /*copy the value of image to *image*/
    fp = sal_fopen(pathname, "rb");
    if(fp ==NULL) 
    {
        DRV_LOG_ERR("Open %s failed.",pathname);
        free(image_buf);
        DRV_CONSOLE_PRINT("Open %s failed.",pathname);
        return RESULT_ERROR;
    }
    while((len = sal_fread(tmpbuf, 1, 512, fp))>0)
    {
        for(i=0; i<len; i++)
        {
            boot_image[count++] = tmpbuf[i];
        }        
    }
    sal_fclose(fp);
    DRV_CONSOLE_PRINT( "FW image size : %d\nBuffer the image success!\n",boot_imageSize); 

    return RESULT_OK;
}
static AQ_API_Port *g_phy_aqr40x_pp_ports[PHY_AQR40X_MAX_NUM_PHY_IDS];
static uint8 is_image_buffer = 0;
static uint8 port_index = 0;
static uint8 gangload_MDIO_address;
static AQ_API_Port  gangloadPort;
static unsigned int   provisioningAddresses[PHY_AQR40X_MAX_NUM_PHY_IDS];
int aqr40x_load_bootimage(phy_handle_t* phy_handle, const char *pathname)
{
    uint32  i;
    uint32  ret = 0;
    AQ_Retcode resultCodes[PHY_AQR40X_MAX_NUM_PHY_IDS];
    uint8 aq_port_index;
    uint8 mdio_port_num;
    AQ_API_Port   *p_port; 

    #if 1
    uint8 *image = NULL;
    uint32 imageSize = 0;
    #endif
    p_port = (AQ_API_Port*)DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(AQ_API_Port));
    if (NULL == p_port)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, p_port);
        DRV_LOG_ERR("calloc memory for port failed.");
        return RESULT_ERROR;
    }
    mdio_port_num = phy_handle->phy_info.ports_per_mdio;
    aq_port_index = phy_handle->phy_info.phy_addr - phy_handle->phy_info.fw_start_addr;
    g_phy_aqr40x_pp_ports[aq_port_index] = p_port;
    provisioningAddresses[aq_port_index] = phy_handle->phy_info.hop_count;
    phy_handle->phy_info.data = (void*)p_port;
    p_port->device = AQ_DEVICE_HHD;
    p_port->PHY_ID = (uint32)phy_handle->phy_info.port_num;

    if(phy_handle->phy_info.phy_addr == phy_handle->phy_info.fw_start_addr)
    {
        gangloadPort.device = AQ_DEVICE_HHD;
        gangloadPort.PHY_ID = ((uint32)phy_handle->phy_info.port_num) | 0x8000; /* set this port to be BC port, set mdio addr BC_address */
        gangload_MDIO_address = phy_handle->phy_info.mdio_BC_addr;
    }
    //DRV_CONSOLE_PRINT( "PORT:%d MDIO_Addr:%d\n", phy_handle->phy_info.port_num, phy_handle->phy_info.phy_addr);
    if(phy_handle->phy_info.phy_manage_info.phy_init_seq_flag != GLB_POST_INIT_FLAG)
    {
        return RESULT_OK;
    }
    port_index = 0;
    DRV_CONSOLE_PRINT("gangloadPort: %d gangload_MDIO_address: %d\n", gangloadPort.PHY_ID, gangload_MDIO_address);
    for(i=0;i<PHY_AQR40X_MAX_NUM_PHY_IDS;i++)
    {
        resultCodes[i] = 0;
    }

    if(!is_image_buffer)
        ret = bootimage_buffer(pathname,image,&imageSize);
    if(!ret)
        is_image_buffer = 1;

    /* load bootload image */
    ret=AQ_API_WriteBootLoadImage(
    g_phy_aqr40x_pp_ports,  mdio_port_num,
    provisioningAddresses,  resultCodes,  &boot_imageSize,
    boot_image,  gangload_MDIO_address,  &gangloadPort);

    for(i=0;i<mdio_port_num;i++)
    {
        DRV_CONSOLE_PRINT("AQ_PORT: %d write bootload image result:%u\n",i,resultCodes[i]);
    }
    /* free image at last */
    if((phy_handle->phy_info.port_num == (phy_handle->phy_info.port_sum_num-1)) && boot_image)
    {
        free(boot_image);
        is_image_buffer = 0;
    }
    if(ret == AQ_RET_OK)    
        return RESULT_OK;
    else
        return RESULT_ERROR;
}

static uint8 check_ver_pass = 0;
static int
aqr40x_init(phy_handle_t* phy_handle)
{
    AQ_API_Port   *p_port; 
    firmware_ver_t version;
    int ret = RESULT_OK;
    const char *image;
    
    p_port = (AQ_API_Port*)DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(AQ_API_Port));
    if (NULL == p_port)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, p_port);
        DRV_LOG_ERR("calloc memory for port failed.");
        return RESULT_ERROR;
    }
    phy_handle->phy_info.data = (void*)p_port;
    p_port->device = AQ_DEVICE_HHD;
    p_port->PHY_ID = (uint32)phy_handle->phy_info.port_num;
    
    if(phy_handle->phy_info.fw_load_mode == PHY_FW_MDIO)
    {
        image = phy_handle->phy_info.firmware_name;
        ret = aqr40x_load_bootimage(phy_handle, image);
        if(ret != RESULT_OK)
        {
            DRV_CONSOLE_PRINT("PORT:%d load bootimage fail\n", phy_handle->phy_info.port_num);
        }
    }
    else if(phy_handle->phy_info.fw_load_mode == PHY_FW_FLASH)
    {  
        if((check_ver_pass == 0) && (phy_handle->phy_info.port_num == phy_handle->phy_info.fw_fix_port))
        {
            ret = check_firmware_version(phy_handle,&version);
            if((phy_handle->phy_info.fw_ver.build_id == version.build_id) && (phy_handle->phy_info.fw_ver.major_num == version.major_num) 
                && (phy_handle->phy_info.fw_ver.minor_num == version.minor_num))
            {
                check_ver_pass = 1;
                DRV_CONSOLE_PRINT("Check firmware version pass\n");
            }
            else
            {
                DRV_CONSOLE_PRINT("Expected Firmware Release:v%d.%d.%d\n", phy_handle->phy_info.fw_ver.major_num,
                    phy_handle->phy_info.fw_ver.minor_num,phy_handle->phy_info.fw_ver.build_id);

            }
        }
    }

    /* enter low-power mode with only the MDIO and microprocessor functioning  */
    ret += aqr40x_write_mask(phy_handle, 0x1e, 0x0000, 0x0800, 0x0800);
    /* Enable link connect/disconnect interrupt */
    ret += aqr40x_write_mask(phy_handle, 0x1e, 0xff00, 0x0001, 0x0001);
    ret += aqr40x_write_mask(phy_handle, 0x1e, 0xff01, 0x1000, 0x1000);
    ret += aqr40x_write_mask(phy_handle, 0x07, 0xd401, 0x0001, 0x0001);
    /* MDI Reversed (ABCD -> DCBA) */
    if(phy_handle->phy_info.pair_reverse)
    {
        ret += aqr40x_write_mask(phy_handle, 1, 0xe400, 3, 3);
    }
    else
    {
        ret += aqr40x_write_mask(phy_handle, 1, 0xe400, 2, 3);
    }
    aqr40x_phy_speed_disable_all(phy_handle);

    if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_100M)
    {
        /* Advertise 100M(full/half) capability */    
        ret += aqr40x_write_mask(phy_handle, 7, 0x0010, 0x0180, 0x03e0);
    }
    if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_10G)
    {
        /* Advertise 10G capability */    
        ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x1000, 0x1000); 
    }
    if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_1G)
    {
        /* Advertise 1000M(full) capability */    
        ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x8000, 0xc000);
    }
    if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_2_5G)
    {
        /* Advertise 2.5G capability */    
        ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0400, 0x0400);
        if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
            ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0080, 0x0080);
    }
    if(phy_handle->phy_info.speed_ability & GLB_SUPPORT_SPEED_5G)
    {
        /* Advertise 5G capability */    
        ret += aqr40x_write_mask(phy_handle, 7, 0xc400, 0x0800, 0x0800);
        if(PORT_PHY_AQR405 != phy_handle->phy_info.phy_device_type)
            ret += aqr40x_write_mask(phy_handle, 7, 0x0020, 0x0100, 0x0100);
    }
    
    /* modified by tongzb for bug 43005 */
    /* Led 0 for 10G/5G/2.5G/1G/100M link and rxtxactivity */
    ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc430, 0x0000, 0xffff);
    /* Led 0 for 10G/5G/2.5G/1G/100M link and rxtxactivity */
    ret += aqr40x_write_mask(phy_handle, 0x1e, 0xc431, 0x0000, 0xffff);

    if(PORT_PHY_AQR409 == phy_handle->phy_info.phy_device_type)
    {
        ret += aqr40x_write_mask(phy_handle, 30, 0x31a, 0x4, 0xffff);
    }
    else if(PORT_PHY_AQR408 == phy_handle->phy_info.phy_device_type)
    {
        ret += aqr40x_write_mask(phy_handle, 30, 0x31a, 0x5, 0xffff);
    }
    else if(PORT_PHY_AQR407 == phy_handle->phy_info.phy_device_type)
    {
        ret += aqr40x_write_mask(phy_handle, 30, 0x31a, 0x3, 0xffff);
    }
    /* disable system side autonegotiation */
    /* added by tongzb for bug 43005 */
    ret += aqr40x_write_mask(phy_handle, 30, 0x31b, 0x0000, 0x0008);
    /* Restart Autonegotiation */
    ret += aqr40x_write_mask(phy_handle, 7, 0, 0x1000, 0x1000);
    /* modified by tongzb for bug 44531 */
    //ret += aqr40x_write_mask(phy_handle, 7, 0, 0x0200, 0x0200);
      
    return ret;
}

phy_handle_t* phy_aqr40x_dev_register(phy_info_t* pphy_info)
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
    phdl->phy_init = aqr40x_init;
    phdl->reg_read = aqr40x_reg_read;
    phdl->reg_write = aqr40x_reg_write;
    phdl->get_cur_status = aqr40x_get_cur_status;
    phdl->get_link_poll = aqr40x_get_link_poll;
    phdl->get_link_interupt = aqr40x_get_link_interrput;
    phdl->config_enable = aqr40x_enable;	
    phdl->config_flowctrl = aqr40x_config_flowctrl;
    phdl->config_speed = aqr40x_config_speed;
    phdl->config_duplex = aqr40x_config_duplex;
    phdl->config_loopback = aqr40x_config_loopback;
    phdl->config_medium = aqr40x_config_medium;
    phdl->phy_set_vct_mod = aqr40x_set_vct_mod;
    phdl->phy_get_vct_info = aqr40x_get_vct_info;
    phdl->enable_eee = aqr40x_enable_eee;
    phdl->config_phy_sig_test_mode = aqr40x_config_phy_sig_test_mode;

    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type = pphy_info->mdio_bus;
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


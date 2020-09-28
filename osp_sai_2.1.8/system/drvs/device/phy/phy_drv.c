/****************************************************************************
* $Id$
* phy driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-08-18
* Reason        : First Create.
****************************************************************************/
#include "phy_drv.h"
#include "drv_debug.h"
#include "phy_null.h"
#include "phy_sfp.h"
#include "phy_vtss8x58.h"
#include "phy_vsc8488.h"
#include "phy_vsc8211.h"
#include "phy_lsi1082.h"
#include "phy_vsc8504.h"
#include "phy_vsc8512.h"
#include "phy_aqr40x.h"
#include "phy_aqr405.h"
#include "phy_mv88e15xx.h"
#include "phy_mv88e1680.h"
#include "phy_mv88e1548.h"

void
flowctrl_admin_to_ability(glb_port_flowctrl_t* admin_flowctrl, flowctrl_adv_ability_t* ability)
{
    if((admin_flowctrl->send==GLB_FLOWCTRL_ENABLE)&&(admin_flowctrl->recv==GLB_FLOWCTRL_ENABLE))
    {
        ability->symmetric_pause = 1;
        ability->asymmetric_pause = 0;
    }
    else if(admin_flowctrl->send==GLB_FLOWCTRL_ENABLE)
    {
        ability->asymmetric_pause = 1;
        ability->symmetric_pause = 0;
    }
    else if(admin_flowctrl->recv==GLB_FLOWCTRL_ENABLE)
    {
        ability->asymmetric_pause = 1;
        ability->symmetric_pause = 1;
    }
    else
    {
        ability->asymmetric_pause = 0;
        ability->symmetric_pause = 0;
    }
    return;
}

void
flowctrl_ability_to_admin(flowctrl_adv_ability_t* ability, glb_port_flowctrl_t* admin_flowctrl)
{
    if((ability->asymmetric_pause)&&(ability->symmetric_pause))
    {
        admin_flowctrl->recv = GLB_FLOWCTRL_ENABLE;
        admin_flowctrl->send = GLB_FLOWCTRL_DISABLE;
    }
    else if(ability->asymmetric_pause)
    {
        admin_flowctrl->recv = GLB_FLOWCTRL_DISABLE;
        admin_flowctrl->send = GLB_FLOWCTRL_ENABLE;
    }
    else if(ability->symmetric_pause)
    {
        admin_flowctrl->recv = GLB_FLOWCTRL_ENABLE;
        admin_flowctrl->send = GLB_FLOWCTRL_ENABLE;
    }
    else
    {
        admin_flowctrl->recv = GLB_FLOWCTRL_DISABLE;
        admin_flowctrl->send = GLB_FLOWCTRL_DISABLE;
    }
    return;
}

phy_handle_t* phy_dev_register(phy_info_t* p_phy_info)
{
    phy_handle_t* p_phy_hdl;
   
    if(NULL == p_phy_info)
    {
        DRV_LOG_WARN("Register a phy driver,Invalid arguments.\n");
        return NULL;
    }
    if(sal_mutex_create(&p_phy_info->pmutex))
    {
        return NULL;    
    }
    
    switch(p_phy_info->phy_device_type)
    {
        case PORT_PHY_VSC8x58:
            p_phy_hdl = phy_vtss8x58_dev_register(p_phy_info);
            break;
        case PORT_PHY_VSC8211:
            p_phy_hdl = phy_vtss8211_dev_register(p_phy_info);
            break;
        case PORT_PHY_VSC8488:
            p_phy_hdl = phy_vsc8488_dev_register(p_phy_info);
            break;
        case PORT_PHY_VSC8512:
            p_phy_hdl = phy_vsc8512_dev_register(p_phy_info);
            break;
        case PORT_PHY_NULL:
            p_phy_hdl = phy_null_dev_register(p_phy_info);
            break;
        case PORT_PHY_LSI1082:
            p_phy_hdl = phy_lsi1082_dev_register(p_phy_info);
            break;
        case PORT_PHY_VSC8504:
            p_phy_hdl = phy_vsc8504_dev_register(p_phy_info);
            break;
        case PORT_PHY_SFP:
            p_phy_hdl = phy_sfp_dev_register(p_phy_info);
            break;
        case PORT_PHY_AQR405:
        case PORT_PHY_AQR407:
        case PORT_PHY_AQR408:
        case PORT_PHY_AQR409: 
        case PORT_PHY_AQR411: 
            p_phy_hdl = phy_aqr40x_dev_register(p_phy_info);
            break;
        case PORT_PHY_MV88E154X:
            p_phy_hdl = phy_mv88e15xx_dev_register(p_phy_info);
            break;
        case PORT_PHY_MV88E1680:
            p_phy_hdl = phy_mv88e1680_dev_register(p_phy_info);      
            break;
        case PORT_PHY_MV88E1548:
            p_phy_hdl = phy_mv88e1548_dev_register(p_phy_info);
            break;
        default:
            DRV_LOG_ERR("Phy dev register: Unknow device type %d\n", p_phy_info->phy_device_type);
            p_phy_hdl = NULL;
            break;        
    }
    
    return p_phy_hdl;
}
/*bug23865. support copper sfp.*/
int32 phy_dev_unregister(phy_handle_t* phy_handle)
{
    int32 ret = RESULT_OK;
    
    if(phy_handle==NULL)
    {
        return ret;
    }
    sal_mutex_destroy(phy_handle->phy_info.pmutex);
    if((phy_handle->phy_info.vct_support)&&(phy_handle->phy_info.phy_veriphy_tsk != NULL))
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, phy_handle->phy_info.phy_veriphy_tsk);
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, phy_handle);
    return ret;        
}


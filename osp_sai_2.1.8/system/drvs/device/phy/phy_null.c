/****************************************************************************
* $Id$
* phy Null device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jqiu 
* Date          : 2013-03-18
* Reason        : First Create.
* Revision      : R0.02
* Author        : jqiu 
* Date          : 2016-07-08
* Reason        : Support Clause73 Auto-neg for 40/100G. see bug39456.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "phy_null.h"
#include "ctc_api.h"
#include "ctc_port.h"
#include "epld_api.h"
#include "fiber_api.h"

char *
phy_port_speed_str(ctc_port_speed_t speed)
{
    switch (speed)
    {
        case CTC_PORT_SPEED_10M:
            return "10M";
        case CTC_PORT_SPEED_100M:
            return "100M";
        case CTC_PORT_SPEED_1G:
            return "1G";
        case CTC_PORT_SPEED_2G5:
            return "2.5G";
#if (defined DUET2) || (defined TSINGMA)             
        case CTC_PORT_SPEED_5G:
            return "5G";
#endif            
        case CTC_PORT_SPEED_10G:
            return "10G";
        case CTC_PORT_SPEED_40G:
            return "40G";
#if (defined DUET2) || (defined TSINGMA)            
        case CTC_PORT_SPEED_25G:
            return "25G";            
        case CTC_PORT_SPEED_50G:
            return "50G";     
#endif            
        case CTC_PORT_SPEED_100G:
            return "100G";
        default:
            return "NO";
    }
}
ctc_port_speed_t
phy_speed_map(glb_port_speed_t speed)
{
    switch (speed)
    {
        case GLB_SPEED_10M:
            return CTC_PORT_SPEED_10M;
        case GLB_SPEED_100M:
            return CTC_PORT_SPEED_100M;
        case GLB_SPEED_1G:
            return CTC_PORT_SPEED_1G;
        case GLB_SPEED_2_5G:
            return CTC_PORT_SPEED_2G5;
#if (defined DUET2) || (defined TSINGMA)             
        case GLB_SPEED_5G:
            return CTC_PORT_SPEED_5G;
#endif            
        case GLB_SPEED_10G:
            return CTC_PORT_SPEED_10G;
        case GLB_SPEED_40G:
            return CTC_PORT_SPEED_40G;
#if (defined DUET2) || (defined TSINGMA)            
        case GLB_SPEED_25G:
            return CTC_PORT_SPEED_25G;            
        case GLB_SPEED_50G:
            return CTC_PORT_SPEED_50G;     
#endif            
        case GLB_SPEED_100G:
            return CTC_PORT_SPEED_100G;
        default:
            return CTC_PORT_SPEED_MAX;
    }
}

#if (defined DUET2) || (defined TSINGMA)
static ctc_port_speed_t phy_config_speed_get(glb_port_speed_t speed, glb_port_speed_t fiber_ability)
{
    ctc_port_speed_t config_speed;
    if(speed != GLB_SPEED_AUTO)
    {
        config_speed = phy_speed_map(speed);
    }
    else
    {
        config_speed = phy_speed_map(fiber_ability);
    }
    return config_speed;
}

static int32 is_config_speed_valid(ctc_port_speed_t ctc_speed,ctc_port_speed_t config_speed)
{
    /* For TsingMa, we only support 12G serders for 10G, the 28G serdes do not for 10G/25G */
    /* For TsingMa 28G serdes limitition: each four serdes shared one freq, so each four serdes need configure to one speed */
    /* For TsingMa 28G serdes, we only support for 100G/40G which can be split into 4*25G/4*10G */
    int32 valid = 0;
    switch (ctc_speed)
    {
        case CTC_PORT_SPEED_10M:
        case CTC_PORT_SPEED_100M:
        case CTC_PORT_SPEED_1G:
            if((config_speed == CTC_PORT_SPEED_10M)||(config_speed == CTC_PORT_SPEED_100M)
                ||(config_speed == CTC_PORT_SPEED_1G)||(config_speed == CTC_PORT_SPEED_10G)
#if (defined DUET2) || (defined TSINGMA)
                ||(config_speed == CTC_PORT_SPEED_25G)
#endif                
                )
                valid = 1;
            break;
        case CTC_PORT_SPEED_2G5:
#if (defined DUET2) || (defined TSINGMA)
        case CTC_PORT_SPEED_5G:            
            if((config_speed == CTC_PORT_SPEED_10M)||(config_speed == CTC_PORT_SPEED_100M)
                ||(config_speed == CTC_PORT_SPEED_1G)||(config_speed == CTC_PORT_SPEED_2G5)              
                ||(config_speed == CTC_PORT_SPEED_5G)              
                )
                valid = 1;
            break;
#endif            
        case CTC_PORT_SPEED_10G:
            if((config_speed == CTC_PORT_SPEED_1G)||(config_speed == CTC_PORT_SPEED_10G)
#if (defined DUET2) || (defined TSINGMA)
                ||(config_speed == CTC_PORT_SPEED_25G)||(config_speed == CTC_PORT_SPEED_5G)
#endif                
                )
                valid = 1;
            break;
#if (defined DUET2) || (defined TSINGMA)
        case CTC_PORT_SPEED_25G:   
            if((config_speed == CTC_PORT_SPEED_1G)||(config_speed == CTC_PORT_SPEED_10G)               
                ||(config_speed == CTC_PORT_SPEED_25G))
                valid = 1;
            break;            
        case CTC_PORT_SPEED_50G:
            if((config_speed == CTC_PORT_SPEED_50G))
                valid = 1;
            break;
#endif            
        case CTC_PORT_SPEED_40G:
        case CTC_PORT_SPEED_100G:
            if((config_speed == CTC_PORT_SPEED_100G)||(config_speed == CTC_PORT_SPEED_40G))
                valid = 1;
            break;
        default:
            valid = 0;
            break;
    }
    return valid;
}
#endif

/*SDK requirement:
  1. be sure mac enable;
  2. cfg auto;
  3. roll back mac config;
*/
int32 ctc_phy_port_set_auto_neg(phy_info_t *pphy_info, uint8 enable)
{
    int32 ret;

    if(pphy_info->phy_manage_info.enable)
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set autoneg %d\n ",pphy_info->port_info.port_id,enable);
        ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, enable);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
    }
    else
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d reset mac and set autoneg %d\n ",pphy_info->port_info.port_id,enable);
        ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
        ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, enable);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
        ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
    }
    return ret;
}

int32 ctc_phy_port_restart_auto(phy_info_t *pphy_info, int8 enable)
{
    int32 ret=0;
    ctc_port_speed_t ctc_speed;

    /* bug44010, When serdes loopback enable, skip do cl73 auto, qicx, 2017-05-15 */
    if (pphy_info->serdes_lb_en)
    {
        return RESULT_OK;
    }

    if(enable && (pphy_info->phy_manage_info.cl73_enable))
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set autoneg disable\n ",pphy_info->port_info.port_id);
        ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, 0);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
 
        if(pphy_info->phy_manage_info.cl73_speed_ability != 0)
        {
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set autoneg enable\n ",pphy_info->port_info.port_id);
            ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, 1);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
        }
        /* when set speed to 1G(auto) or speed is 1G(auto), the cl73_speed_ability is 0, but we need open auto neg */
        else
        {
            /*get new ctc speed*/
            ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
            if(ctc_speed == CTC_PORT_SPEED_1G)
            {
                if((pphy_info->phy_manage_info.speed == GLB_SPEED_AUTO) 
                    ||(pphy_info->phy_manage_info.duplex == GLB_DUPLEX_AUTO))
                {
                    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d speed is 1G, set autoneg enable\n ",pphy_info->port_info.port_id);
                    ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, 1);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }                
                }
            }
            else
            {
                if((pphy_info->phy_manage_info.speed == GLB_SPEED_1G) 
                    &&(pphy_info->phy_manage_info.duplex == GLB_DUPLEX_AUTO))
                {
                    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d speed config 1G set autoneg enable\n ",pphy_info->port_info.port_id);
                    ret = ctc_port_set_auto_neg(pphy_info->port_info.port_id, 1);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }
                }
            }
        }
    }
    return ret;
}

/* Added by qicx for bug44301, rollback auto except 10G status, 2017-06-09 */
int32 ctc_phy_port_rollback_auto(phy_info_t *pphy_info)
{
    int32 ret = 0;
    glb_port_speed_t speed, fiber_ability;
    glb_port_duplex_t duplex;

    speed = pphy_info->phy_manage_info.speed;
    duplex = pphy_info->phy_manage_info.duplex;
    fiber_ability = pphy_info->phy_manage_info.fiber_ability;

    (void)fiber_ability;
    /* Auto mode */
    if ((GLB_SPEED_AUTO == speed) || (GLB_DUPLEX_AUTO == duplex))
    {
        ctc_port_speed_t ctc_speed;
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
#if (defined DUET2) || (defined TSINGMA)
        /* for rollback auto_neg also need to judge the cl73 speed and cl73 enable*/
        if(pphy_info->phy_manage_info.cl73_speed_ability&&pphy_info->phy_manage_info.cl73_enable)
        {
            ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
        }
        else
        {
            if(CTC_PORT_SPEED_1G == ctc_speed)
            {
                ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
            }
            else
            {
                ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
            }
        }
#else
        /* If pcs speed mode is 10G, cannot enable AN */
        if(CTC_PORT_SPEED_10G != ctc_speed)
        {
            ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
        }
#endif        
    }

    return ret;
}

/*Old SDK requirement:
  1. clear old ability, set to 0;
  2. cfg real ability;
*/
#if (defined DUET2) || (defined TSINGMA)
uint32 _phy_port_get_autoneg_ability(phy_info_t *pphy_info)
{
    uint32 ability;
    ability = pphy_info->phy_manage_info.fec_ability; /* change only config speed or insert one media */

    /* when config force fec, that mean disable auto fec ability and requeset */
    //if((pphy_info->phy_manage_info.fec_en&GLB_FEC_AUTO_MASK)== GLB_FEC_AUTO)
    {
        if((pphy_info->phy_manage_info.fec_en&GLB_FEC_MASK)== GLB_FEC_OFF)
        {
            ability &= (~CTC_PORT_CL73_25G_BASER_FEC_REQUESTED);
            ability &= (~CTC_PORT_CSTM_BASER_FEC_ABILITY);
            ability &= (~CTC_PORT_CSTM_BASER_FEC_REQUESTED);
            ability &= (~CTC_PORT_CL73_25G_RS_FEC_REQUESTED);
            ability &= (~CTC_PORT_CSTM_RS_FEC_ABILITY);
            ability &= (~CTC_PORT_CSTM_RS_FEC_REQUESTED);
            ability &= (~CTC_PORT_CL73_FEC_ABILITY);
            ability &= (~CTC_PORT_CL73_FEC_REQUESTED);
        }
        else if((pphy_info->phy_manage_info.fec_en&GLB_FEC_MASK)== GLB_FEC_RS)
        {
            ability &= (~CTC_PORT_CL73_25G_BASER_FEC_REQUESTED);
            ability &= (~CTC_PORT_CSTM_BASER_FEC_ABILITY);
            ability &= (~CTC_PORT_CSTM_BASER_FEC_REQUESTED); 
            ability &= (~CTC_PORT_CL73_FEC_ABILITY);
            ability &= (~CTC_PORT_CL73_FEC_REQUESTED);            
        }
        else if((pphy_info->phy_manage_info.fec_en&GLB_FEC_MASK)== GLB_FEC_BASE_R)
        {
            ability &= (~CTC_PORT_CL73_25G_RS_FEC_REQUESTED);
            ability &= (~CTC_PORT_CSTM_RS_FEC_ABILITY);
            ability &= (~CTC_PORT_CSTM_RS_FEC_REQUESTED);        
        }
    }

    ability |= pphy_info->phy_manage_info.cl73_speed_ability;
    return ability;
}

int32 phy_null_get_autoneg_ability(phy_handle_t* phy_handle, uint32 *ability)
{
    *ability = _phy_port_get_autoneg_ability(&(phy_handle->phy_info));

    return 0;
}
#endif

int32 ctc_phy_port_update_cl73_ability(phy_info_t *pphy_info)
{
    int32 ret;

#if (!defined DUET2)&&(!defined TSINGMA)
    uint32 ability;
    ability = pphy_info->phy_manage_info.cl73_speed_ability;    
    if((ability & (CTC_PORT_CL73_100GBASE_CR4|CTC_PORT_CL73_100GBASE_KR4)) != 0)
    {
        ability |= CTC_PORT_CL73_FEC_ABILITY;
    }
    if(pphy_info->phy_manage_info.fec_en)
    {
        ability |= CTC_PORT_CL73_FEC_REQUESTED;
    }
    ret = ctc_port_set_property(pphy_info->port_info.port_id, CTC_PORT_PROP_CL73_ABILITY, ability);
#else
    ret = ctc_port_set_property(pphy_info->port_info.port_id, CTC_PORT_PROP_CL73_ABILITY,
                           _phy_port_get_autoneg_ability(pphy_info));
#endif
    if(ret != CTC_E_NONE)
    {
        DRV_LOG_ERR("%s %d gport %d  ret %d", 
             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }
    return ret;
}

/*SDK requirement:
  1. mac disable;
  2. change speed;
  3. mac roll back;
*/
int32 ctc_phy_port_set_speed(phy_info_t *pphy_info, ctc_port_speed_t speed_mode)
{
    int32 ret=0;
    /*Before change speed, must disable mac first.*/
    if(pphy_info->phy_manage_info.enable)
    {
        ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
    }
#if defined(GREATBELT)
    ctc_chip_serdes_info_t serdes_info;
    if(speed_mode == CTC_PORT_SPEED_1G)
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XSGMII_MODE;
    else if(speed_mode == CTC_PORT_SPEED_10G)
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
    serdes_info.serdes_id = pphy_info->port_info.serdes_id;
    ret=ctc_chip_set_serdes_mode(pphy_info->port_info.lchip, &serdes_info);
    pphy_info->serdes_mode = serdes_info.serdes_mode;
#elif (defined DUET2) || (defined TSINGMA)
    ctc_port_if_mode_t if_mode;
    ctc_port_if_type_t if_type = CTC_PORT_IF_NONE;
    if(speed_mode  == CTC_PORT_SPEED_10G)
        if_type = CTC_PORT_IF_XFI;
    else if(speed_mode  == CTC_PORT_SPEED_1G)
        if_type = CTC_PORT_IF_SGMII;
    else if(speed_mode  == CTC_PORT_SPEED_25G)
        if_type = CTC_PORT_IF_CR;
    else if(speed_mode  == CTC_PORT_SPEED_40G)
        if_type = CTC_PORT_IF_CR4;
    else if(speed_mode  == CTC_PORT_SPEED_100G)
        if_type = CTC_PORT_IF_CR4;
    else if(speed_mode  == CTC_PORT_SPEED_50G)
        if_type = CTC_PORT_IF_CR2;
#if defined(TSINGMA)    
    else if(speed_mode  == CTC_PORT_SPEED_100M)
        if_type = CTC_PORT_IF_FX;
#endif    
    if_mode.speed = speed_mode;
    if_mode.interface_type = if_type;
    ret = ctc_port_set_interface_mode(pphy_info->port_info.port_id, &if_mode);

    /* for bug 53296, when configure speed to 1G, need do both serdes mode and mac speed change */
    if(speed_mode  == CTC_PORT_SPEED_1G)
    {
        ret += ctc_port_set_speed(pphy_info->port_info.port_id, CTC_PORT_SPEED_1G);
    }
#else
    ret += ctc_port_set_speed(pphy_info->port_info.port_id, speed_mode);
#endif
    if(ret != CTC_E_NONE)
    {                        
        DRV_LOG_ERR("%s %d gport %d  ret %d",
            __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }
    /*After change speed, mac roll back config.*/
    if(pphy_info->phy_manage_info.enable)
    {
        ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
    }
    return ret;
}

int32 ctc_phy_port_set_auto_fec(phy_info_t *pphy_info, int8 fec_en)
{
    int32 ret = 0;
#if (defined DUET2)||(defined TSINGMA)
    uint32 auto_fec = CTC_PORT_FEC_TYPE_NONE;
    if(GLB_FEC_RS == (fec_en&GLB_FEC_MASK))
        auto_fec = (1<<CTC_PORT_FEC_TYPE_RS);
    else if(GLB_FEC_BASE_R == (fec_en&GLB_FEC_MASK))
        auto_fec = (1<<CTC_PORT_FEC_TYPE_BASER);
    else if(GLB_FEC_OFF == (fec_en&GLB_FEC_MASK))
        auto_fec = CTC_PORT_FEC_TYPE_NONE; 
    else if(GLB_FEC_ALL == (fec_en&GLB_FEC_MASK))
        auto_fec = (1<<CTC_PORT_FEC_TYPE_RS) | (1<<CTC_PORT_FEC_TYPE_BASER); 
    /* for bug 47962, do not mac disable/enable on duet2/tsingma */
    ret = ctc_port_set_property(pphy_info->port_info.port_id, CTC_PORT_PROP_AUTO_NEG_FEC, auto_fec);
    DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d set auto_fec %d", pphy_info->port_info.port_id, auto_fec);
    if(ret != CTC_E_NONE)
    {
        DRV_LOG_ERR("%s %d gport %d  ret %d", 
             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }    
#endif
    return ret; 
}

int32 ctc_phy_port_set_fec_en(phy_info_t *pphy_info, int8 fec_en)
{
    int32 ret = 0;
#if (defined DUET2)||(defined TSINGMA)
    ctc_port_fec_type_t ctc_fec_en = CTC_PORT_FEC_TYPE_NONE;
    if(GLB_FEC_RS == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_RS;
    else if(GLB_FEC_BASE_R == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_BASER;
    else if(GLB_FEC_OFF == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_NONE; 
    else if(GLB_FEC_ALL == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = GLB_FEC_RS;     
    /* for bug 47962, do not mac disable/enable on duet2/tsingma */
    ctc_port_set_property(pphy_info->port_info.port_id, CTC_PORT_PROP_FEC_EN, ctc_fec_en);
    //if(ret != CTC_E_NONE)
    //{
    //    DRV_LOG_ERR("%s %d gport %d  ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    //}    
#else
    int8 ctc_fec_en;
    ctc_fec_en = fec_en;
    if(pphy_info->phy_manage_info.enable)
    {
        ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
    }
    ret = ctc_port_set_property(pphy_info->port_info.port_id, CTC_PORT_PROP_FEC_EN, ctc_fec_en);
    if(ret != CTC_E_NONE)
    {
        DRV_LOG_ERR("%s %d gport %d  ret %d", 
             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }
    if(pphy_info->phy_manage_info.enable)
    {
        /*Bug39827 After config FEC, roll back mac config.*/
        ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d  ret %d", 
                 __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
    }
#endif   
    DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d set fec en %d", pphy_info->port_info.port_id, ctc_fec_en);
    return ret;
}

#if (defined DUET2)||(defined TSINGMA)
int32 ctc_phy_port_get_fec_en(phy_info_t *pphy_info, uint32 *fec_status)
{
    int32 ret = 0;
    uint32 status;
    ret = ctc_port_get_property(pphy_info->port_info.port_id, CTC_PORT_PROP_FEC_EN, &status);
    if(CTC_PORT_FEC_TYPE_RS == status)
        *fec_status = GLB_FEC_RS;
    else if(CTC_PORT_FEC_TYPE_BASER == status)
        *fec_status = GLB_FEC_BASE_R;
    else if(CTC_PORT_FEC_TYPE_NONE == status)
        *fec_status = GLB_FEC_OFF;
    else
        *fec_status = GLB_FEC_OFF;
    return ret;
}

int32 ctc_phy_is_support_fec(ctc_port_speed_t ctc_speed)
{
    if((CTC_PORT_SPEED_100G == ctc_speed)||(CTC_PORT_SPEED_50G == ctc_speed)||(CTC_PORT_SPEED_25G == ctc_speed)
#ifdef TSINGMA
      ||(CTC_PORT_SPEED_40G == ctc_speed)||(CTC_PORT_SPEED_10G == ctc_speed)
#endif
    )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

int32 ctc_phy_is_fec_invalid(ctc_port_speed_t ctc_speed, uint8 fec_en)
{
#if (defined DUET2)
    ctc_port_fec_type_t ctc_fec_en = CTC_PORT_FEC_TYPE_NONE;
    if(GLB_FEC_RS == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_RS;
    else if(GLB_FEC_BASE_R == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_BASER;
    else if(GLB_FEC_OFF == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_NONE;
    else if(GLB_FEC_ALL == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = GLB_FEC_RS;

    if (CTC_PORT_SPEED_25G != ctc_speed
     && CTC_PORT_SPEED_50G != ctc_speed
     && CTC_PORT_SPEED_100G != ctc_speed)
    {
        return 1;
    }

    if (CTC_PORT_FEC_TYPE_BASER == ctc_fec_en
     && CTC_PORT_SPEED_100G == ctc_speed)
    {
        return 1;
    }

    return 0;
#elif (defined TSINGMA)
    ctc_port_fec_type_t ctc_fec_en = CTC_PORT_FEC_TYPE_NONE;
    if(GLB_FEC_RS == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_RS;
    else if(GLB_FEC_BASE_R == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_BASER;
    else if(GLB_FEC_OFF == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = CTC_PORT_FEC_TYPE_NONE;
    else if(GLB_FEC_ALL == (fec_en&GLB_FEC_MASK))
        ctc_fec_en = GLB_FEC_RS;

    if (CTC_PORT_FEC_TYPE_BASER == ctc_fec_en
     && CTC_PORT_SPEED_25G != ctc_speed
     && CTC_PORT_SPEED_50G != ctc_speed
     && CTC_PORT_SPEED_40G != ctc_speed
     && CTC_PORT_SPEED_10G != ctc_speed)
    {
        return 1;
    }

    if (CTC_PORT_FEC_TYPE_RS == ctc_fec_en
     && CTC_PORT_SPEED_25G != ctc_speed
     && CTC_PORT_SPEED_50G != ctc_speed
     && CTC_PORT_SPEED_100G != ctc_speed)
    {
        return 1;
    }

    return 0;
#else
    return 0;
#endif
}

int32 ctc_phy_port_do_cl73_auto(phy_info_t *pphy_info)
{
    int is_up;
    uint16 gport;
    int32 ret = RESULT_OK;
    uint32 auto_en;

   gport = pphy_info->port_info.port_id;

    if(pphy_info->phy_manage_info.enable)
    {
        if(pphy_info->phy_manage_info.cl73_enable)
        {   
            if(pphy_info->phy_manage_info.cl73_speed_ability != 0)
            {
                ret = ctc_port_get_mac_link_up(gport, &is_up);
                if(ret != CTC_E_NONE)
                {
                    DRV_LOG_ERR("%s %d gport %d  ret %d", 
                         __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                }
                if(!is_up)
                {
                    ret = ctc_port_get_property(pphy_info->port_info.port_id, CTC_PORT_PROP_AUTO_NEG_EN, &auto_en);
                    /*From auto to training, need wait more.*/
                    if((auto_en == 2)&&(pphy_info->phy_stat_flag.cl73_status == 1))
                    {
                        pphy_info->phy_stat_flag.cl73_status = auto_en;
                    }
                    else
                    //if((auto_en == 0) || (auto_en == 1) || ((auto_en == 2)&&(pphy_info->phy_stat_flag.cl73_status == 2))||(auto_en == 3))
                    //if((auto_en == 0) || ((auto_en == 2)&&(pphy_info->phy_stat_flag.cl73_status == 2)))
                    {
                        /*restart auto-negotiation*/
                        ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
                        pphy_info->phy_stat_flag.cl73_status = 1;
                        DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d trigger auto. ability 0x%x auto %d",
                            pphy_info->port_info.port_id, pphy_info->phy_manage_info.cl73_speed_ability, auto_en) 
                    }
                }
                else
                {
                    pphy_info->phy_stat_flag.cl73_status = 3;
                }
            }
        }
    }
    return ret;    
}
int32 phy_null_enable(phy_handle_t* phy_handle, int8 enable)
{
    int32 ret=0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set mac %d\n ",pphy_info->port_info.port_id,enable);
    ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, enable);
    if(enable)
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d restart auto when set mac enable\n ",pphy_info->port_info.port_id);
        ctc_phy_port_restart_auto(pphy_info,enable);
    }
    return ret;
}

/***********************************************************
  1. FEC config won't be lost when speed change between 100G and 40G.
  2. After config FEC, if port enable CL73, need update ability and restart auto.
  3. FEC config must do when mac disable.
************************************************************/
int32 phy_null_config_fec_en(phy_handle_t* phy_handle, uint8 fec_en)
{
    int32 ret = 0;
    phy_info_t *pphy_info = NULL;
        
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set fec 0x%x\n ",pphy_info->port_info.port_id,fec_en);    
#if (!defined DUET2)&&(!defined TSINGMA)
    ctc_phy_port_set_fec_en(pphy_info, fec_en);
    /*need restart auto if enable auto*/
    ctc_phy_port_update_cl73_ability(pphy_info);
    ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
#else
    ctc_port_speed_t ctc_speed;
    
    ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
    if(ret != CTC_E_NONE)
    {
        DRV_LOG_ERR("%s %d gport %d get speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }  
    
    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d update cl73_ability\n ",pphy_info->port_info.port_id);
    /* 1. update cl73 ability */
    ret = ctc_phy_port_update_cl73_ability(pphy_info);
    if(ctc_phy_is_support_fec(ctc_speed))
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d update fec\n ",pphy_info->port_info.port_id);
        /* 2. config fec status */
        if (!ctc_phy_is_fec_invalid(ctc_speed, fec_en))
        {
            ret = ctc_phy_port_set_fec_en(pphy_info, fec_en);
        }
        /* 3. config auto fec filter */
        ret += ctc_phy_port_set_auto_fec(pphy_info, fec_en);
    }

    if(pphy_info->phy_manage_info.cl73_speed_ability != 0)
    {
        /* for bug 47962, restart autoneg when cl73_enable is true with cl73 ability is no NULL */
        if(pphy_info->phy_manage_info.cl73_enable)
        {
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d restart auto when update fec\n ",pphy_info->port_info.port_id);
            /* if enable auto cl73, when we set fec xxx, we need restart auto */
            ret += ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
        }
    }    
#endif    

    return ret;    
}

int32 phy_null_config_unidir(phy_handle_t* phy_handle, uint8 enable)
{
    int ret;
    uint16 gport;
    phy_info_t *pphy_info;

    pphy_info = &phy_handle->phy_info;
    gport = pphy_info->port_info.port_id;   

    /* Added by liuht for bug 37450, 2016-03-11 */
    /* If unidir function is enabled, then disable port interrupt */
    if(GLB_PORT_UNIDIR_DISABLE == enable)
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set intrrupt true\n ",pphy_info->port_info.port_id);
        ret = ctc_port_set_property(gport,CTC_PORT_PROP_LINK_INTRRUPT_EN,TRUE);
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set unidir 0\n ",pphy_info->port_info.port_id);
        ret += ctc_port_set_property(gport, CTC_PORT_PROP_UNIDIR_EN, 0);
    }
    else
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set intrrupt false\n ",pphy_info->port_info.port_id);
        ret = ctc_port_set_property(gport,CTC_PORT_PROP_LINK_INTRRUPT_EN,FALSE);
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d set unidir 1\n ",pphy_info->port_info.port_id);
        ret += ctc_port_set_property(gport, CTC_PORT_PROP_UNIDIR_EN, 1);
    }

    /* Modified by liuht for bug42776, 2017-02-17 */
    /* when mac is enabled, if unidirectional rx-only is set,
     * then disable mac tx */
    if((GLB_PORT_UNIDIR_RX_ONLY == enable))
    {
#ifndef DUET2
        //ctc_port_set_property(gport,CTC_PORT_PROP_NET_TX, FALSE);
#endif
    }
    else
    {
#ifndef DUET2
        /* Modified by liuht for bug 44007, 2017-05-12*/
        /* E350 don't support this property, so now just ignor return with error */
        //ctc_port_set_property(gport,CTC_PORT_PROP_NET_TX, TRUE);
#endif
    }
    
    return ret;
}

static int32 phy_null_conf_set(phy_handle_t* phy_handle, glb_port_speed_t speed, glb_port_duplex_t duplex)
{
    int32 ret = 0;
    ctc_port_speed_t ctc_speed;
    phy_info_t *pphy_info;
    
    pphy_info = &phy_handle->phy_info;
#if (!defined DUET2)&&(!defined TSINGMA)
    /*Auto mode*/
    if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
    {
        /*Get real speed.*/
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }                
        /*For 1G/10G auto, first change speed to aim speed, then open auto.*/
        /* speed 100M same as 1G, for bug 46698 by tongzb 2018.05.10 */
        if((CTC_PORT_SPEED_1G == ctc_speed)||(CTC_PORT_SPEED_100M == ctc_speed))
        {
            if((GLB_SPEED_10G == speed) 
                || ((GLB_SPEED_AUTO == speed)&&(GLB_SPEED_10G == pphy_info->phy_manage_info.fiber_ability)))
            {
                ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_10G);
                if(ret != CTC_E_NONE)
                {
                    DRV_LOG_ERR("%s %d gport %d set speed ret %d",
                        __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                }
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d switch to 10G",pphy_info->port_info.port_id)                
            }
            else
            {
                /* for bug 49294, when mac is keep to speed 100M, do auto-neg enable only can not change speed to 1G .
                *  we need do speed set when mac speed is not equal to config speed
                */
                if(ctc_speed != CTC_PORT_SPEED_1G)
                {
                    ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_1G);
                }
                ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
            }
        }
        else if(CTC_PORT_SPEED_10G == ctc_speed)
        {
            if((GLB_SPEED_1G == speed) 
                || ((GLB_SPEED_AUTO == speed)&&(GLB_SPEED_1G == pphy_info->phy_manage_info.fiber_ability)))
            {
                ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_1G);
                ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d switch to 1G",pphy_info->port_info.port_id)
            }            
        }
        else if((CTC_PORT_SPEED_40G == ctc_speed)||(CTC_PORT_SPEED_100G == ctc_speed))
        {
            pphy_info->phy_manage_info.cl73_enable = 1;
            ret += ctc_phy_port_update_cl73_ability(pphy_info);
            /*Bug42145. Be same as force mode when fiber insert.*/
            if(pphy_info->phy_manage_info.cl73_speed_ability == 0)
            {
                /* Fix bug43932, when fiber insert, must disable AN first */
                ret += ctc_phy_port_set_auto_neg(pphy_info, 0);  
                /*cfg speed is auto, real speed depend on fiber speed.*/
                if(speed == GLB_SPEED_AUTO)
                {
                    if((ctc_speed == CTC_PORT_SPEED_40G) && (pphy_info->phy_manage_info.fiber_ability == GLB_SPEED_100G))
                    {
                        ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_100G);
                    }
                    else if((ctc_speed == CTC_PORT_SPEED_100G) && (pphy_info->phy_manage_info.fiber_ability == GLB_SPEED_40G))
                    {
                        ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_40G);
                    }
                }                
                /*cfg speed is force, real speed depend on CFG speed.*/
                else
                {
                    if((ctc_speed == CTC_PORT_SPEED_40G) && (speed == GLB_SPEED_100G))
                    {
                        ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_100G);
                    }
                    else if((ctc_speed == CTC_PORT_SPEED_100G) && (speed == GLB_SPEED_40G))
                    {
                        ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_40G);
                    }
                }
                /*get new ctc speed*/
                ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
                /*if 100G enable fec, when change to force, need disable/enable mac to avoid fec hang.*/
                if((pphy_info->phy_manage_info.enable)&&(ctc_speed == CTC_PORT_SPEED_100G)&&(pphy_info->phy_manage_info.fec_en))
                {
                    ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }
                    ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }
                }
            }
            else
            {
                ret += ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d start clause73 auto",pphy_info->port_info.port_id)
            }
        }        
    }
    else /*Force mode*/
    {
        /*1. disable auto. Note: if current speed is 10G, and need change to 1G, then this disable won't affect 1G 
            auto. because sdk will check pcs mode before disable auto.*/
        pphy_info->phy_manage_info.cl73_enable = 0;
        ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
        /*2. Get real speed.*/
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
        /*3. decide to change speed*/      
        if(speed == GLB_SPEED_1G)
        {
            if(ctc_speed != CTC_PORT_SPEED_1G)
            {
                ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_1G);
                ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d force to 1G",pphy_info->port_info.port_id)
            }
            else
            {
                /*When 1G disable auto, force port down/up to notify link parter.*/
                if(pphy_info->phy_manage_info.enable)
                {
                    ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }
                    /*Bug39941, delay 20ms between down/up to notify link partner.*/
                    sal_udelay(20000);
                    ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
                    if(ret != CTC_E_NONE)
                    {
                        DRV_LOG_ERR("%s %d gport %d  ret %d", 
                             __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                    }
                } 
            }
        }        
        else if((speed == GLB_SPEED_10G) && (ctc_speed != CTC_PORT_SPEED_10G))
        {
            ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_10G);
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d force to 10G",pphy_info->port_info.port_id)
        }
        else if((speed == GLB_SPEED_40G) && (ctc_speed != CTC_PORT_SPEED_40G))
        {
            ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_40G);
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d force to 40G",pphy_info->port_info.port_id)
        }
        else if((speed == GLB_SPEED_100G) && (ctc_speed != CTC_PORT_SPEED_100G))
        {
            ret += ctc_phy_port_set_speed(pphy_info, CTC_PORT_SPEED_100G);
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d force to 100G",pphy_info->port_info.port_id)
        }
        /*if 100G enable fec, when change to force, need disable/enable mac to avoid fec hang.*/
        if((pphy_info->phy_manage_info.enable)&&(speed == GLB_SPEED_100G)&&(pphy_info->phy_manage_info.fec_en))
        {
            ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
            ret = ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
        }
    }
#else  /* Here for DUET2 or TSINGMA */
    ctc_port_speed_t cfg_speed;
    /*Auto mode*/
    if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
    {
        /*1. decide to change speed when fiber insert */ 
        /*Bug42145. Be same as force mode when fiber insert.*/
        /* when config force, we need set speed, but when config auto, we just need config cl73_ability */
        if(pphy_info->phy_manage_info.cl73_speed_ability == 0)
        {
            /* Fix bug43932, when fiber insert, must disable AN first */
            ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d disable clause73 auto",pphy_info->port_info.port_id);
        }       
        /* for bug 50064, config speed no matter enable cl73-auto or disable */
        /*Get real speed.*/
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }            
        cfg_speed = phy_config_speed_get(speed, pphy_info->phy_manage_info.fiber_ability);
        if((cfg_speed != ctc_speed) && (is_config_speed_valid(ctc_speed,cfg_speed)))
        {
            ret += ctc_phy_port_set_speed(pphy_info, cfg_speed);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d set speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d switch to %s",pphy_info->port_info.port_id, 
                phy_port_speed_str(cfg_speed))
        }
        /* 2. config auto-neg when DAC/copper insert */
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }   
       
        if((CTC_PORT_SPEED_100G == ctc_speed)||(CTC_PORT_SPEED_50G == ctc_speed)||(CTC_PORT_SPEED_40G == ctc_speed)
            ||(CTC_PORT_SPEED_25G == ctc_speed)||(CTC_PORT_SPEED_10G == ctc_speed))
        {
            if(CTC_PORT_SPEED_10G != ctc_speed)//10G do not support auto
            {
                pphy_info->phy_manage_info.cl73_enable = 1;
            }
            else
            {
                pphy_info->phy_manage_info.cl73_enable = 0;
            }
            /* 1. update cl73 ability */
            ret += ctc_phy_port_update_cl73_ability(pphy_info);
            if(ctc_phy_is_support_fec(ctc_speed))
            {
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d update fec when fec auto xxx\n ",pphy_info->port_info.port_id);
                /* 2. config fec status */
                if (!ctc_phy_is_fec_invalid(ctc_speed, pphy_info->phy_manage_info.fec_en))
                {
                    ctc_phy_port_set_fec_en(pphy_info, pphy_info->phy_manage_info.fec_en);
                }
                /* 3. config auto fec filter */
                ret += ctc_phy_port_set_auto_fec(pphy_info, pphy_info->phy_manage_info.fec_en);
            }
            if(pphy_info->phy_manage_info.cl73_speed_ability != 0)
            {
                if(pphy_info->phy_manage_info.cl73_enable)
                {
                    ret += ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
                    DRV_LOG_DEBUG(DRV_PHY, "phy null port %d start clause73 auto",pphy_info->port_info.port_id)
                }
            }
        }
    }
    else /*Force mode*/
    {
        /*1. disable auto*/
        pphy_info->phy_manage_info.cl73_enable = 0;
        ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
        /*2. Get real speed.*/
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",
                __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }
        /*3. decide to change speed*/      
        cfg_speed = phy_config_speed_get(speed, pphy_info->phy_manage_info.fiber_ability);
        if((cfg_speed != ctc_speed) && (is_config_speed_valid(ctc_speed,cfg_speed)))
        {
            /* set speed */
            ret += ctc_phy_port_set_speed(pphy_info, cfg_speed);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d set speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
            DRV_LOG_DEBUG(DRV_PHY, "phy null port %d force to %s",pphy_info->port_info.port_id, 
                phy_port_speed_str(cfg_speed))
            /* for bug 53301,53364, when change speed from 10G/25G to 1G, need do disable auto again */
            if(cfg_speed == CTC_PORT_SPEED_1G)
            {
                ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
            }
        }
        /* 4. get real speed after speed config, try to config fec status */
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(ret != CTC_E_NONE)
        {
            DRV_LOG_ERR("%s %d gport %d get speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
        }         
        /* for bug 45831, need reset fen_en after speed set */
        if(ctc_phy_is_support_fec(ctc_speed))
        {
            //if((pphy_info->phy_manage_info.fec_en&GLB_FEC_AUTO_MASK) != GLB_FEC_AUTO)
            {
                DRV_LOG_DEBUG(DRV_PHY, "phy null port %d update fec when fec force xxx\n ",pphy_info->port_info.port_id);
                if (!ctc_phy_is_fec_invalid(ctc_speed, pphy_info->phy_manage_info.fec_en))
                {
                    ctc_phy_port_set_fec_en(pphy_info, pphy_info->phy_manage_info.fec_en);
                }
                ret += ctc_phy_port_set_auto_fec(pphy_info, pphy_info->phy_manage_info.fec_en);
            }
        }
    }
    /*get new ctc speed*/
    ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
    if(ctc_speed == CTC_PORT_SPEED_1G)
    {
        /* when 1G, always enable auto*/
        /* for bug 45625, disable auto, when force speed 1G , enable auto when auto speed 1G */
        //ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
        if((speed == GLB_SPEED_1G)&&(duplex != GLB_DUPLEX_AUTO))
        {
            /*When 1G disable auto, force port down/up to notify link parter.*/
            if(pphy_info->phy_manage_info.enable)
            {
                ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 0);
                if(ret != CTC_E_NONE)
                {
                    DRV_LOG_ERR("%s %d gport %d  ret %d", 
                         __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                }
                /*Bug39941, delay 20ms between down/up to notify link partner.*/
                sal_udelay(200000);
                ret += ctc_port_set_mac_en(pphy_info->port_info.port_id, 1);
                if(ret != CTC_E_NONE)
                {
                    DRV_LOG_ERR("%s %d gport %d  ret %d", 
                         __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
                }
            } 
        }
        /* when speed 1G, auto, need enable auto */
        else
        {
            ret += ctc_phy_port_set_auto_neg(pphy_info, 1);
        }
    }
#endif
    return ret;
}

/*GB not support config flowctrl ability. GG support, but need sdk give API.*/
int32 phy_null_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

/* added by tongzb for bug 44368, support loopback on CNOS */
int32 phy_null_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    uint8 i;
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;
    uint8 lchip = 0;
    ctc_chip_serdes_loopback_t  lb_param;
    uint8 il, el; /* current internal/external loopback status */
    ctc_port_speed_t ctc_speed;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "phy null port %d config loopback mode %d", pphy_info->port_info.port_id, lb_mode);
    
    sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));

    lb_param.serdes_id = pphy_info->port_info.serdes_id;
    /* get internal loopback status */
    lb_param.mode = 0;
    ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
    il = lb_param.enable;

    /* get external loopback status */
    lb_param.mode = 1;
    ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
    el = lb_param.enable;

    switch (lb_mode)
    {
    case GLB_LB_NONE: /* disable loopback */
        if (il)   /* disable internal loopback if exist */
        {
            for (i = 0; i < pphy_info->port_info.serdes_num; i++)
            {
                lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
                lb_param.mode = 0;
                lb_param.enable = 0;
                ret += ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
        }
        if (el)   /* disable external loopback if exist */
        {
            for (i = 0; i < pphy_info->port_info.serdes_num; i++)
            {
                lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
                lb_param.mode = 1;
                lb_param.enable = 0;
                ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
        }
        /* Rollback AN */
        ret += ctc_phy_port_rollback_auto(pphy_info);
        /* update serdes loopback flag to 0 */
        pphy_info->serdes_lb_en = 0;
        break;
    case GLB_LB_PHY_NEAR: /* internal loopback */
        if (el)   /* disable external loopback if exist */ 
        {
            for (i = 0; i < pphy_info->port_info.serdes_num; i++)
            {
                lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
                lb_param.mode = 1;
                lb_param.enable = 0;
                ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
        }
        /* update serdes loopback flag to 1 */
        pphy_info->serdes_lb_en = 1;
        /* Before config loopback, need disable AN first */
        /* modified by tongzb for bug 44296 */
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(CTC_PORT_SPEED_1G != ctc_speed)
        {
            ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
        }
        for (i = 0; i < pphy_info->port_info.serdes_num; i++)
        {        
            lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
            lb_param.mode = 0;
            lb_param.enable = 1;
            ret += ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            if (ret)
            {
                /* set serdes loopback failed, Rollback AN and lb flag */
                ret += ctc_phy_port_rollback_auto(pphy_info);
                pphy_info->serdes_lb_en = 0;
            }
        }
        break;
    case GLB_LB_PHY_FAR:  /* external loopback */
        if (il)   /* disable internal loopback if exist */
        {
            for (i = 0; i < pphy_info->port_info.serdes_num; i++)
            {           
                lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
                lb_param.mode = 0;
                lb_param.enable = 0;
                ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
        }
        /* update serdes loopback flag to 1 */
        pphy_info->serdes_lb_en = 1;
        /* Before config loopback, need disable AN first */
        /* modified by tongzb for bug 44326, if port is 1G, do not disable autoneg */
        ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
        if(CTC_PORT_SPEED_1G != ctc_speed)
        {
            ret += ctc_phy_port_set_auto_neg(pphy_info, 0);
        }
        for (i = 0; i < pphy_info->port_info.serdes_num; i++)
        {
            lb_param.serdes_id = pphy_info->port_info.serdes_id+i;
            lb_param.mode = 1;
            lb_param.enable = 1;
            ret += ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            if (ret)
            {
                /* set serdes loopback failed, Rollback AN and lb flag */
                ret += ctc_phy_port_rollback_auto(pphy_info);
                pphy_info->serdes_lb_en = 0;
            }
        }
        break;
    default:
        return RESULT_ERROR;
        break;
    }
    
    return ret;
}

int32 phy_null_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    return RESULT_OK;
}
/*Bug39456 jqiu 2016-07-19, support CL73 auto.*/
int32 phy_null_do_cl73_auto(phy_handle_t* phy_handle)
{
    phy_info_t *pphy_info;
    int32 ret = RESULT_OK;

    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    if(pphy_info->phy_manage_info.enable 
        && pphy_info->phy_manage_info.cl73_enable
        && pphy_info->phy_manage_info.cl73_speed_ability)
    {
        ret = ctc_phy_port_do_cl73_auto(pphy_info);
    }
    return ret;    
}

int32 phy_null_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t *phy_val)
{
    phy_info_t *pphy_info;
    ctc_port_speed_t port_speed;
    int is_up, link_up;
    uint16 gport;
    int32 ret = RESULT_OK;
    int32 val = 0;
    if (NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    gport = pphy_info->port_info.port_id;
    if(pphy_info->phy_manage_info.enable)
    {
        /* if unidir is enabled, then force link up */
        /* Modfied by liuht for bug45110, 2017-0912 */
        if(GLB_PORT_UNIDIR_DISABLE != phy_handle->phy_info.phy_manage_info.unidir)
            is_up = GLB_LINK_UP;
        else if(phy_handle->phy_info.phy_manage_info.bypass)
        {
            epld_item_read(0, EPLD_BYPASS, &val);
            if(val) /* bypass enable */
            {
                ret = ctc_port_get_mac_link_up(gport, &is_up);
            }
            else
                is_up = GLB_LINK_UP;
        }
        else
            ret = ctc_port_get_mac_link_up(gport, &is_up);
    }
    else
    {
        is_up = GLB_LINK_DOWN;
    }
    
    if(is_up)
    {
        ret += ctc_port_get_speed(gport, &port_speed);
        /* Modified by liuht for bug41686, 2016-11-23 */
        /* 100G port need get link status (with fault signal) from epld */
        /* Modfied by liuht for bug45110, 2017-0912 */
        if((port_speed == CTC_PORT_SPEED_100G) 
            && (GLB_PORT_UNIDIR_DISABLE == phy_handle->phy_info.phy_manage_info.unidir))
        {
            epld_get_txlink_status(phy_handle->phy_info.zg_port_id, &is_up);
        }
    }

    if(is_up)
    {
        if(port_speed == CTC_PORT_SPEED_100G)
            phy_val->speed = GLB_SPEED_100G;
        else if(port_speed == CTC_PORT_SPEED_40G)
            phy_val->speed = GLB_SPEED_40G;
        else if(port_speed == CTC_PORT_SPEED_10G)
            phy_val->speed = GLB_SPEED_10G;
        else if(port_speed == CTC_PORT_SPEED_2G5)
            phy_val->speed = GLB_SPEED_2_5G;
        else if(port_speed == CTC_PORT_SPEED_1G)
            phy_val->speed = GLB_SPEED_1G;  
#if (defined DUET2)||(defined TSINGMA)
        else if(port_speed == CTC_PORT_SPEED_25G)
            phy_val->speed = GLB_SPEED_25G;
        else if(port_speed == CTC_PORT_SPEED_50G)
            phy_val->speed = GLB_SPEED_50G; 
#endif   

        link_up = GLB_LINK_UP;
    }
    else
    {
        phy_val->speed = pphy_info->phy_manage_info.speed;
        link_up = GLB_LINK_DOWN;
    }

    phy_val->link_up = link_up;

    phy_val->duplex = GLB_DUPLEX_FULL;
    phy_val->linkup_media = GLB_PORT_TYPE_FIBER;
    phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
    phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
#if (defined DUET2)||(defined TSINGMA)
    /* update fec status */
    if(ctc_phy_is_support_fec(port_speed))
    {
        ctc_phy_port_get_fec_en(pphy_info,&phy_val->fec_status); 
    }
    else
    {
        phy_val->fec_status = GLB_FEC_OFF;
    }
    phy_val->cl73_ability = _phy_port_get_autoneg_ability(pphy_info);
#endif    
    return ret;
}

int32 phy_null_get_link_interrupt(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;

    if (pphy_info->phy_manage_info.link_tolerable_time == 0)
    {
        phy_val->link_up = GLB_LINK_DOWN;
    }
    else
    {
        sal_task_sleep(pphy_info->phy_manage_info.link_tolerable_time);
        phy_null_get_cur_status(phy_handle, phy_val);
    }

    /*link change from up to down*/
    //if(pphy_info->phy_stat_flag.link_up == GLB_LINK_UP)
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d link down",pphy_info->port_info.port_id)
        /*Bug4038, jqiu optimize. When use chip link, must disable intr when link is down, or it will report intr continully. uni-dir won't use intr.*/
        if(GLB_PORT_UNIDIR_ENABLE != phy_handle->phy_info.phy_manage_info.unidir)
        {
            DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d disable link interrupt",pphy_info->port_info.port_id);
            ret = ctc_port_set_property(pphy_info->port_info.port_id,CTC_PORT_PROP_LINK_INTRRUPT_EN, FALSE);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
        }
#if (!defined DUET2)&&(!defined TSINGMA)       
        ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
#endif
        pphy_info->phy_stat_flag.link_up = phy_val->link_up;
    }    
    /*link not change*/
    else
    {
        phy_change->action = -1;
    }
#if (defined DUET2)||(defined TSINGMA)
    if(pphy_info->phy_stat_flag.cl73_ability!= phy_val->cl73_ability)
    {            
        phy_change->cl73_ability_change = 1;
    }
    if(pphy_info->phy_stat_flag.fec_status!= phy_val->fec_status)
    {            
        phy_change->fec_status_change = 1;
    }    
    pphy_info->phy_stat_flag.cl73_ability = phy_val->cl73_ability;
    pphy_info->phy_stat_flag.fec_status = phy_val->fec_status;
#else
    /* phy_stat_flag.fec_status only change after link poll, so we can get it here */
    phy_val->fec_status = pphy_info->phy_stat_flag.fec_status;
#endif
    return RESULT_OK;    
}

int32 phy_null_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;
    int32 ret = RESULT_OK;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    phy_null_get_cur_status(phy_handle, phy_val);
    
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
        /*When use chip link, only can enable intr when link is up, uni-dir won't use intr.*/
        if(GLB_PORT_UNIDIR_ENABLE != phy_handle->phy_info.phy_manage_info.unidir)
        {
            DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d enable link interrupt",pphy_info->port_info.port_id);
            ret = ctc_port_set_property(pphy_info->port_info.port_id,CTC_PORT_PROP_LINK_INTRRUPT_EN,TRUE);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
        }
        DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d link up",pphy_info->port_info.port_id) 
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d link down",pphy_info->port_info.port_id)
        /*Bug4038, jqiu optimize. When use chip link, must disable intr when link is down, or it will report intr continully. uni-dir won't use intr.*/
        if(GLB_PORT_UNIDIR_ENABLE != phy_handle->phy_info.phy_manage_info.unidir)
        {
            DRV_LOG_DEBUG(DRV_PHY, "phy null addr %d disable link interrupt",pphy_info->port_info.port_id);
            ret = ctc_port_set_property(pphy_info->port_info.port_id,CTC_PORT_PROP_LINK_INTRRUPT_EN, FALSE);
            if(ret != CTC_E_NONE)
            {
                DRV_LOG_ERR("%s %d gport %d  ret %d", 
                     __FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
            }
        }
#if (!defined DUET2)&&(!defined TSINGMA)       
        ctc_phy_port_restart_auto(pphy_info,pphy_info->phy_manage_info.enable);
#endif
    }
    /*link always up*/
    else if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_change->action = -1;
        if(pphy_info->phy_stat_flag.speed != phy_val->speed)
        {            
            phy_change->speed_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
    }
    /*link always down*/
    else
    {
        phy_change->action = -1;
    }
    if(pphy_info->phy_stat_flag.cl73_ability!= phy_val->cl73_ability)
    {            
        phy_change->cl73_ability_change = 1;
    }
    if(pphy_info->phy_stat_flag.fec_status!= phy_val->fec_status)
    {            
        phy_change->fec_status_change = 1;
    }    
    pphy_info->phy_stat_flag.cl73_ability = phy_val->cl73_ability;
    pphy_info->phy_stat_flag.fec_status = phy_val->fec_status;
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
phy_null_config_phy_sig_test_mode(phy_handle_t* phy_handle, uint8 serdes_id, uint8 mode)
{
// TODO: by weij for merge GB trunk to GG
#if 0
    ctc_chip_serdes_reg_access_t reg_access;
    
    if(mode == PHY_SIG_TEST_MODE_NORMAL)
    {
        reg_access.serdes_id = serdes_id;
        reg_access.direction = 1;
        reg_access.addr = 1;
        reg_access.value = 0;
        ctc_chip_set_property(0, CTC_CHIP_REG_ACCESS, (void *)&reg_access);
    }
    else if(mode == PHY_SIG_TEST_MODE_PRBS9)
    {
        reg_access.serdes_id = serdes_id;
        reg_access.direction = 1;
        reg_access.addr = 1;
        reg_access.value = 0x1008;
        ctc_chip_set_property(0, CTC_CHIP_REG_ACCESS, (void *)&reg_access);
    }
    else if(mode == PHY_SIG_TEST_MODE_PRBS31)
    {
        reg_access.serdes_id = serdes_id;
        reg_access.direction = 1;
        reg_access.addr = 1;
        reg_access.value = 0xe;
        ctc_chip_set_property(0, CTC_CHIP_REG_ACCESS, (void *)&reg_access);
    }
#endif
    return RESULT_OK;
}

int32 phy_null_config_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = 0;
    phy_info_t *pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    /* Modified by liuht for bug43257, 2017-03-31 */
    /* before set speed, must disable undir function */
    if(GLB_PORT_UNIDIR_DISABLE != pphy_info->phy_manage_info.unidir)
    {
        phy_null_config_unidir(phy_handle, GLB_PORT_UNIDIR_DISABLE);
    }
    ret = phy_null_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    DRV_LOG_DEBUG(DRV_PHY,
        "phy null gport %d config speed %d, orig %d, duplex %d, ret %d", pphy_info->port_info.port_id, speed, 
            pphy_info->phy_manage_info.speed, pphy_info->phy_manage_info.duplex,ret);
    /* after set speed, must re-enable undir function if needed */
    if(GLB_PORT_UNIDIR_DISABLE != pphy_info->phy_manage_info.unidir)
    {
        phy_null_config_unidir(phy_handle, pphy_info->phy_manage_info.unidir);
    }    
    return ret;
}

int32 phy_null_config_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = 0;
    phy_info_t *pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    ret = phy_null_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    DRV_LOG_DEBUG(DRV_PHY,
        "phy null gport %d config duplex %d, orig %d, speed %d, ret %d", pphy_info->port_info.port_id, duplex, 
            pphy_info->phy_manage_info.duplex,pphy_info->phy_manage_info.speed,ret);
    return ret;
}

#if (defined DUET2)||(defined TSINGMA)
int32 phy_null_init(phy_handle_t* phy_handle)
{
    int32 ret=0;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    ctc_port_speed_t ctc_speed;
    
    ret = ctc_port_get_speed(pphy_info->port_info.port_id, &ctc_speed);
    if(ret != CTC_E_NONE)
    {
        DRV_LOG_ERR("%s %d gport %d get speed ret %d",__FUNCTION__, __LINE__,pphy_info->port_info.port_id, ret);
    }      
    /* 1. update cl73 ability */
    ret = ctc_phy_port_update_cl73_ability(pphy_info);
    if(ctc_phy_is_support_fec(ctc_speed))
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy null port %d update fec when fec auto xxx\n ",pphy_info->port_info.port_id);
        /* 2. config fec status */
        if (!ctc_phy_is_fec_invalid(ctc_speed, pphy_info->phy_manage_info.fec_en))
        {
            ret = ctc_phy_port_set_fec_en(pphy_info, pphy_info->phy_manage_info.fec_en);
        }
        /* 3. config auto fec filter */
        ret += ctc_phy_port_set_auto_fec(pphy_info, pphy_info->phy_manage_info.fec_en);
    }    
    ret = ctc_phy_port_set_auto_neg(pphy_info,pphy_info->phy_manage_info.cl73_enable);
    return ret;
}
#endif

/*
*  mode: 0-disable tx, 1-enable tx, 2-check rx 
*  polynome_type: 0-PRBS7_+, 1-PRBS7_-, 2-PRBS15_+, 3-PRBS15_-, 4-PRBS23_+, 5-PRBS23_-, 6-PRBS31_+, 7-PRBS31_-
*  result: when check rx, this is the check result for prbs check
*/
int32 phy_null_config_prbs(phy_handle_t* phy_handle, uint8 mode,uint8 polynome_type,uint8 *result)
{
    int32 ret = 0;
    phy_info_t *pphy_info;
    ctc_chip_serdes_prbs_t prbs_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 i = 0;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    serdes_id = pphy_info->port_info.serdes_id;

    for (i = 0; i < pphy_info->port_info.serdes_num; i++)
    {
        prbs_param.serdes_id = serdes_id+i;
        prbs_param.value = (mode == 1)?1:0;
        prbs_param.mode = (mode == 2)?0:1;
        prbs_param.polynome_type = polynome_type;
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_PRBS, (void*)&prbs_param);
    }

    if(mode == 2) /* check rx */
        *result = prbs_param.value;
    else /* configure tx, result has no mean */
        *result = 0;

    return ret;
} 

phy_handle_t* phy_null_dev_register(phy_info_t* pphy_info)
{
    phy_handle_t* phdl = NULL;
    
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

    sal_memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));
#if (defined DUET2)||(defined TSINGMA)
    phdl->phy_init = phy_null_init;
#endif
    phdl->config_enable = phy_null_enable;
    phdl->config_speed = phy_null_config_speed;
    phdl->config_duplex = phy_null_config_duplex;
    phdl->config_medium = phy_null_config_medium;
    phdl->config_flowctrl = phy_null_config_flowctrl;
    phdl->config_loopback = phy_null_config_loopback; /* bug44010, support serdes loopback, qicx, 2017-05-15 */
    phdl->get_link_poll = phy_null_get_link_poll;
    phdl->get_link_interupt = phy_null_get_link_interrupt;
    phdl->config_master_slave= NULL;
    phdl->get_cur_status = phy_null_get_cur_status; /* modified by liuht for bug 26641, 2014-0422 */
    phdl->config_phy_sig_test_mode = phy_null_config_phy_sig_test_mode;
    phdl->config_unidir = phy_null_config_unidir;
    phdl->config_fec_en = phy_null_config_fec_en;
    phdl->config_prbs = phy_null_config_prbs;
#if (defined DUET2) || (defined TSINGMA)
    phdl->get_autoneg_ability = phy_null_get_autoneg_ability;
#endif
    return phdl;
}





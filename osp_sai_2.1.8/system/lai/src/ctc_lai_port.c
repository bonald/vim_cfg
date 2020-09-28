/****************************************************************************
* $Id$
*  Centec function information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-011
* Reason        : First Create.
****************************************************************************/

#include "laiinc.h"

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_tempfile_define.h"
#include "lcapi.h"
#include "lcm_specific.h"
#include "lcm_mgt.h"
#include "lcm_client.h"
#include "lcm_error.h"
#include "lcm_card.h"
#include "lcm_port.h"
#include "lcm_dpll.h"
#include "lcm_debug.h"
#include "lcm_srv.h"
#include "lcm_lai.h"

lai_status_t
ctc_lai_get_port_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_port_info_t* port)
{
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ];
    //unsigned char mac[GLB_ETH_ADDR_LEN];
    //int mac_length;
    uint32 portid, slot;

    if(NULL == port)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    slot = oid >> 16;
    portid =  oid & 0x000000FF;

    if(portid > p_card->port_num)
    {
        LCM_LOG_BRINGUP("ctc_lai_get_port_info, portid = %d, port num is %d", portid, p_card->port_num);
        return LAI_STATUS_INVALID_PARAMETER;
    }
    if(slot != p_card->phy_slot_no)
    {
        LCM_LOG_BRINGUP("ctc_lai_get_port_info, slot = %d, slot num is %d", slot, p_card->phy_slot_no);
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_port = p_card->pp_port[portid];
    if(p_port == NULL)
    {
        LCM_LOG_BRINGUP("ctc_lai_get_port_info, pp_port is NULL");
        return LAI_STATUS_PORT_NOT_INIT;
    }


    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(port, 0, sizeof(lai_port_info_t));
    port->oid = oid;
    port->slot_id = p_card->phy_slot_no;
    port->pannel_port_id = p_port->panel_port_no;
    port->pannel_sub_port_id = p_port->panel_sub_port_no;
    port->portid = p_port->logic_port_idx;

    if_build_port_shortname_by_slot_port(ifname, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
    //lcm_mgt_get_if_hwaddr(ifname, (unsigned char *)mac, &mac_length);
    sal_memcpy(port->mac, p_port->mac, GLB_ETH_ADDR_LEN);
    sal_strncpy(port->name, ifname, GLB_PHY_PORT_FULL_NAME_SZ);
    
    switch(p_port->port_cfg.media)
    {
        case GLB_PORT_TYPE_AUTO:
            port->media =  LAI_PORT_MEDIA_AUTO;
            break;
        case GLB_PORT_TYPE_COPPER:
            port->media =  LAI_PORT_MEDIA_COPPER;
            break;
        case GLB_PORT_TYPE_FIBER:
            port->media =  LAI_PORT_MEDIA_FIBER;
            break;
        case GLB_PORT_TYPE_SGMII:
            port->media =  LAI_PORT_MEDIA_SGMII;
            break; 
        default:
            return LAI_STATUS_PORT_NOT_INIT;
    }

    switch(p_port->port_media_type)
    {
        case GLB_PORT_MEDIA_TYPE_DEFAULT:
            port->commber =  LAI_PORT_COMMBER_DEFAULT;
            break;
        case GLB_PORT_MEDIA_TYPE_SFP:
            port->commber =  LAI_PORT_COMMBER_SFP;
            break;
        case GLB_PORT_MEDIA_TYPE_QSFP:
            port->commber =  LAI_PORT_COMMBER_QSFP;
            break;
        default:
            return LAI_STATUS_PORT_NOT_INIT;
    }
#if 0
    if(GLB_SUPPORT_SPEED_100G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_100G;    
    }
    else if(GLB_SUPPORT_SPEED_40G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_40G;    
    } 
    else if(GLB_SUPPORT_SPEED_10G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_10G;    
    } 
    else if(GLB_SUPPORT_SPEED_5G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_5G;    
    } 
    else if(GLB_SUPPORT_SPEED_2_5G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_2_5G;    
    } 
    else if(GLB_SUPPORT_SPEED_1G & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_1G;    
    } 
    else if(GLB_SUPPORT_SPEED_100M & p_port->port_speed_real)
    {
       port->speed = LAI_PORT_SPEED_100M;    
    } 
    else 
    {
        port->speed = LAI_PORT_SPEED_1G;    
    }
#endif
    if(GLB_SUPPORT_SPEED_100G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_100G;    
    }
    if(GLB_SUPPORT_SPEED_40G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_40G;    
    } 
    if(GLB_SUPPORT_SPEED_10G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_10G;    
    } 
    if(GLB_SUPPORT_SPEED_50G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_50G;    
    } 
    if(GLB_SUPPORT_SPEED_25G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_25G;    
    }     
    if(GLB_SUPPORT_SPEED_5G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_5G;    
    } 
    if(GLB_SUPPORT_SPEED_2_5G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_2_5G;    
    } 
    if(GLB_SUPPORT_SPEED_1G & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_1G;    
    } 
    if(GLB_SUPPORT_SPEED_100M & p_port->port_speed_ability)
    {
       port->speed_caps |= LAI_PORT_SPEED_100M;    
    } 

    port->phy_type = p_port->phy_type;
    /* save port flag */
    port->phyport_flag |= (p_port->is_combo)       ? GLB_PHYPORT_FLAG_COMBO     : 0;
    port->phyport_flag |= (p_port->lp_support)     ? GLB_PHYPORT_FLAG_LOOPBACK  : 0;
    port->phyport_flag |= (p_port->eee_support)    ? GLB_PHYPORT_FLAG_EEE       : 0;
    port->phyport_flag |= (p_port->poe_support)    ? GLB_PHYPORT_FLAG_POE       : 0;
    port->phyport_flag |= (p_port->unidir_support) ? GLB_PHYPORT_FLAG_UNIDIR    : 0;
    port->support_media_switch = p_port->support_media_switch;
    port->bypass = p_port->bypass;
    port->duplex = p_port->port_status.duplex;
    port->speed = p_port->port_status.speed;
    port->fec_status = p_port->port_status.fec_status;
    port->cl73_ability = p_port->port_status.cl73_ability;   
    return LAI_STATUS_SUCCESS;
}

static lai_port_api_t      g_lai_port_api_func = {
    .get_port_info      = ctc_lai_get_port_info,
};

static ctc_lai_api_reg_info_t g_port_api_reg_info = {
    .id         = LAI_API_PORT,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_port_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_port_init()
{
    lai_api_reg_register_fn(&g_port_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}


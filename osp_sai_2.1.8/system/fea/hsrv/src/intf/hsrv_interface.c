/****************************************************************************
* $Id$
*  Centec function information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-01
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_msg_if.h"
#include "glb_ip_define.h"
#include "lcm.h"
#include "hsrv_interface.h"
#include "hsrv_isolation.h"
#include "hsrv_if.h"
#include "hsrv_mlag.h"
#include "hsrv_fdb.h"
#include "hsrv_vlan_mapping.h"
#include "lib_ioctl.h"
#include "glb_l2_define.h"
#include "ctc_sai_port.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_fiber_define.h"
#include "gen/tbl_port_define.h"
#include "gen/tbl_fiber.h"
#include "saiisolation.h"
#include "saivlanmap.h"
#include "lib_sysctl.h"
#include "lib_netlink.h"
#include "hsrv_msg_openflow.h"


/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/
extern int32
hsrv_interface_set_openflow_en(tbl_interface_t* p_if);

extern int32
hsrv_interface_set_openflow_operation_agg(tbl_interface_t* p_if);


int32
hsrv_interface_set_crc_error_check_en(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PORT_CRC_ERROR_CHECK_EN;

    if(p_if->crc_error_check_en == 1)
    {
        attr[0].value.u32 = 1;
    }else{
        attr[0].value.u32 = 0;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
}

int32
hsrv_interface_set_crc_recalculation_en(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PORT_CRC_RECALCULATION_EN;

    if(p_if->crc_recalculation_en == 1)
    {
        attr[0].value.u32 = 1;
    }else{
        attr[0].value.u32 = 0;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
}

/*added by yejl for bug 44935, 2019-03-07*/
tbl_interface_t*
hsrv_interface_get_agg_interface_by_lag_member_port(tbl_interface_t* p_if)
{
    tbl_interface_t     *p_if_lag  = NULL;
    tbl_interface_key_t if_lag_key;
    if ((NULL == p_if) || (NULL == p_if->lag))
    {
        return NULL;
    }
    if (!INTERFACE_IS_LAG_MEMBER(p_if))
    {
        return NULL;
    }
    
    sal_memset(&if_lag_key, 0, sizeof(if_lag_key));
    sal_snprintf(if_lag_key.name, IFNAME_SIZE, "agg%u", p_if->lag->lag_id);
    p_if_lag = tbl_interface_get_interface(&if_lag_key);
    if (NULL == p_if_lag)
    {
        return NULL;
    }
    else
    {
        return p_if_lag;
    }
}
/*end*/

/***************************************************************************************************
 * Name         : hsrv_interface_set_speed 
 * Purpose      : set interface speed        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_speed(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = p_if->speed;
    HSRV_IF_ERROR_RETURN(lcm_port_set_speed(&portstatus));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_svlan_tpid 
 * Purpose      : set interface svlan-tpid        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_svlan_tpid(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr;
    sai_object_id_t port = 0;
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
#ifdef TAPPRODUCT
    attr.id = SAI_PORT_ATTR_CUSTOM_SVLAN_TPID;
#else
    attr.id = SAI_PORT_ATTR_CUSTOM_DOT1Q_ETHERTYPE;

    /* ctcos bug 47581 remove check */
#if 0
    /* check qinq or vlan class for switchport port */
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
    sal_strcpy(if_key.name, p_if->key.name);
    p_db_if = tbl_interface_get_interface(&if_key);
    HSRV_PTR_CHECK(p_db_if);

    if (p_db_if->brgif)
    {
        if (p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ || p_db_if->vlanclass_active_type)
        {
            return HSRV_E_NONE;
        }
    }
#endif
#endif

    attr.value.u32 = p_if->svlan_tpid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &attr)); 
    
    return HSRV_E_NONE;
    
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_duplex
 * Purpose      : set interface duplex        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_duplex(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = p_if->duplex;
    HSRV_IF_ERROR_RETURN(lcm_port_set_duplex(&portstatus));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_loopback
 * Purpose      : set interface loopback        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
/* added by tongzb for bug 44368, support loopback on CNOS */
int32
hsrv_interface_set_loopback(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;
    uint16 srcGport = 0;
    uint16 dstGport = 0;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));

    /* added by hansf for bug 47956 */
    if (p_if->internal_lp_ifindex > 0)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_if->internal_lp_ifindex, &dstGport));
    }
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_if->ifindex, &srcGport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = p_if->phy_loopback_mode;
    portstatus.lp_dstport = dstGport;
    portstatus.lp_srcport = srcGport;
    HSRV_IF_ERROR_RETURN(lcm_port_set_lpbk(&portstatus));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_unidir
 * Purpose      : set interface unidir        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_unidir(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = p_if->unidir;
    HSRV_IF_ERROR_RETURN(lcm_port_set_unidir(&portstatus));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_fec
 * Purpose      : set interface fec        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_fec(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_fec_enable_t fec;
    
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    fec.slot_idx = slot;
    fec.port_idx = pannelport;
    fec.subport_idx = pannelsubport;
    fec.enable = p_if->fec;
    HSRV_IF_ERROR_RETURN(lcm_port_set_fec_en(&fec));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_poe
 * Purpose      : set interface poe        
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_poe(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_poe_t poe_cfg;
    
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    poe_cfg.slot_idx = slot;
    poe_cfg.port_idx = pannelport;
    poe_cfg.subport_idx = pannelsubport;
    poe_cfg.enable = p_if->poe.enable;
    poe_cfg.force_power = p_if->poe.force_power;
    poe_cfg.mode = p_if->poe.mode;
    poe_cfg.priority = p_if->poe.priority;
    poe_cfg.power_limit = p_if->poe.power_limit;    
    HSRV_IF_ERROR_RETURN(lcm_port_set_poe(&poe_cfg));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_media
 * Purpose      : set interface media      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_media(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = p_if->media;
    HSRV_IF_ERROR_RETURN(lcm_port_set_media(&portstatus));

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_holdtime(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t tolerable;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    /*Currently we use LCM port API to set port speed, because we need driver function for stable platform*/
    /*Later we should use SAI interface to set speed*/

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    tolerable.slot_idx = slot;
    tolerable.port_idx = pannelport;
    tolerable.subport_idx = pannelsubport;
    tolerable.status = p_if->carrier_down_holdtime_interval;
    HSRV_IF_ERROR_RETURN(lcm_port_set_link_tolerable_time(&tolerable));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_jumboframe_en
 * Purpose      : set interface jumboframe enable      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_jumboframe_en(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    tbl_sys_global_t* p_sys_global = NULL;
    sai_attribute_t attr[1];

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    p_sys_global = tbl_sys_global_get_sys_global();
    HSRV_PTR_CHECK(p_sys_global);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_MTU;
    attr[0].value.u32 = p_if->jumboframe_en? p_sys_global->jumbo_frame_size : p_sys_global->max_frame_size;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_if->portid, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_flow_control
 * Purpose      : set interface flow control    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_flow_control(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;    
#ifdef GREATBELT
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_flowctrl_t port_status;
#endif

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL;
    /*added by yejl to fix bug40234*/
    if (GLB_FLAG_ISSET(p_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        attr[0].value.s32 = SAI_PORT_FLOW_CONTROL_DISABLE;
    }
    else if((GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
        &&(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE)))
    { 
        attr[0].value.s32 = SAI_PORT_FLOW_CONTROL_BOTH_ENABLE;
    }
    else if(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    { 
        attr[0].value.s32 = SAI_PORT_FLOW_CONTROL_TX_ONLY;
    }                    
    else if(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))
    {
        attr[0].value.s32 = SAI_PORT_FLOW_CONTROL_RX_ONLY;
    } 
    else
    {
        attr[0].value.s32 = SAI_PORT_FLOW_CONTROL_DISABLE;   
    }  
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

#ifdef GREATBELT
    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    port_status.slot_idx = slot;
    port_status.port_idx = pannelport;
    port_status.subport_idx = pannelsubport;

    if((GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
        &&(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE)))
    { 
        port_status.direction = GLB_FLOWCTRL_RECV;
        port_status.enable = TRUE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));

        port_status.direction = GLB_FLOWCTRL_SEND;
        port_status.enable = TRUE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));       
    }
    else if(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    { 
        port_status.direction = GLB_FLOWCTRL_RECV;
        port_status.enable = FALSE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));

        port_status.direction = GLB_FLOWCTRL_SEND;
        port_status.enable = TRUE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));   
    }                    
    else if(GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))
    {
        port_status.direction = GLB_FLOWCTRL_RECV;
        port_status.enable = TRUE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));

        port_status.direction = GLB_FLOWCTRL_SEND;
        port_status.enable = FALSE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));   
    } 
    else
    {
        port_status.direction = GLB_FLOWCTRL_RECV;
        port_status.enable = FALSE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status));

        port_status.direction = GLB_FLOWCTRL_SEND;
        port_status.enable = FALSE;
        HSRV_IF_ERROR_RETURN(lcm_port_set_flowctrl(&port_status)); 
    }  
#endif

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_priority_flow_control
 * Purpose      : set priority flow control priority 
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_priority_flow_control(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;
    attr[0].value.u8 = p_if->pfc_priority;
    
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
    
}

int32
hsrv_interface_set_egress_acl_en(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if ((GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type) 
        && (p_if->hw_type != GLB_IF_TYPE_PORT_IF))
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_EGRESS_ACL_EN;

    if(0 != sal_strlen(p_if->egs_acl))
    {
        attr[0].value.u32 = TRUE;
    }
    else
    {
        attr[0].value.u32 = FALSE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
    
}

int32
hsrv_interface_set_latency_event_enable(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port_oid = 0;

    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port_oid = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_MONITOR_LATENCY_EN;

    if (p_if->latency_event_enable == TRUE)
    {
         attr[0].value.u32 = TRUE;
    }
    else
    {
        attr[0].value.u32 = FALSE;
    }
	
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, attr));

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_latency_event_threshold(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    uint32_t threshold[2];
    sai_object_id_t port= 0;

    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port= p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_MONITOR_LATENCY_THRESHOLD;
    attr[0].value.u32list.list = threshold;
    attr[0].value.u32list.list[0] = p_if->latency_event_low -1;
    attr[0].value.u32list.list[1] = p_if->latency_event_high -1;
    	
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;	
}

int32
hsrv_interface_set_tap_used(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_CUSTOM_TAP_USED;

    if(p_if->tap_used == TRUE)
    {
        attr[0].value.u32 = TRUE;
    }else{
        attr[0].value.u32 = FALSE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
    
}

int32
hsrv_interface_set_metadata(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if((p_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF))
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_CUSTOM_METADATA;

    if(p_if->metadata_enable == TRUE)
    {
        attr[0].value.u32 = TRUE;
    }else{
        attr[0].value.u32 = FALSE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
    
}

int32
hsrv_interface_set_reflective_en(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    if((p_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF))
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_CUSTOM_RELFECTIVE_EN;

    if(p_if->reflective_bridge_en == 1)
    {
        attr[0].value.u32 = 1;
    }else{
        attr[0].value.u32 = 0;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
    
}

/* system modified by liwh for bug 53010, 2019-07-29 */
#ifdef GREATBELT
int32
hsrv_interface_set_port_mac(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    
    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF) 
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PORT_OAM_MAC;

    sal_memcpy(attr[0].value.mac, p_if->mac_addr, 6);
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr)); 
    
    return HSRV_E_NONE;
}
#endif
/* liwh end */

/***************************************************************************************************
 * Name         : hsrv_interface_set_ip_source_guard
 * Purpose      : set interface ip source guard eanable or disable    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_ip_source_guard(tbl_interface_t* p_if, uint32 field_id)
{
    sai_port_api_t  *port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    u_int8_t        scl_ley_type;

    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    
    if (TBL_INTERFACE_FLD_IP_SOURCE_TYPE == field_id)
    {
        if (IP_NONE == p_if->ip_source_type)
        {
            attr[0].id = SAI_PORT_ATTR_IP_SOURCE_GUARD;
            attr[0].value.s32 = 0;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

            return HSRV_E_NONE;
        }

        if (p_if->ip_source_type == IP_MAC_VLAN_FILTERING)
        {
            scl_ley_type = GLB_IP_BINDING_SCL_TYPE_MAC;
        }
        else if (IP_NONE != p_if->ip_source_type)
        {
            scl_ley_type = GLB_IP_BINDING_SCL_TYPE_IP;
        }

        attr[0].id = SAI_PORT_ATTR_IP_SOURCE_GUARD;
        attr[0].value.s32 = scl_ley_type;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

        return HSRV_E_NONE;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_admin_state
 * Purpose      : set interface admin state    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_admin_state(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    uint8 slot = 0, pannelport = 0, pannelsubport = 0;
    lcapi_lcm_set_port_status_t portstatus;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        lib_ioctl_if_set_up(p_if->key.name, TRUE);
    }

    hsrv_brgif_set_flags(p_if->ifindex, p_if->flags);
    
    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        attr[0].id = SAI_PORT_ATTR_ADMIN_STATE;
        attr[0].value.booldata = 1;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }
    else
    {
        attr[0].id = SAI_PORT_ATTR_ADMIN_STATE;
        attr[0].value.booldata = 0;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }

    HSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(p_if->key.name, &slot, &pannelport, &pannelsubport));
    portstatus.slot_idx = slot;
    portstatus.port_idx = pannelport;
    portstatus.subport_idx = pannelsubport;
    portstatus.status = GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP);
    HSRV_IF_ERROR_RETURN(lcm_port_set_enable(&portstatus)); 

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_vif_admin_state
 * Purpose      : set interface admin state    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vif_admin_state(tbl_interface_t* p_if)
{
    sai_router_interface_api_t* rif_api = NULL;  
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif; 
   
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        lib_ioctl_if_set_up(p_if->key.name, TRUE);
    }

    sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
    attr.value.booldata = (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))?TRUE:FALSE;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}

extern int32
hsrv_router_kernel_update_local(char *ifname, int32 is_running);

int32
hsrv_interface_update_if_state_for_vlan(tbl_interface_t* p_if)
{
    uint32    enable  = FALSE;
    
    HSRV_PTR_CHECK(p_if);
    
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING) && GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        enable = TRUE;
    }
    
/*
    hsrv_brgif_set_flags(p_if->ifindex, p_if->flags);
    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_RUNNING, enable));
    lib_ioctl_if_set_running(p_if->key.name, enable);
*/

    MY_PRINTF("set %s %d", p_if->key.name, enable);
    hsrv_router_kernel_update_local(p_if->key.name, enable);
    return HSRV_E_NONE;
}

int32
hsrv_interface_update_if_state(tbl_interface_t* p_if)
{
    uint32    enable  = FALSE;
    
    HSRV_PTR_CHECK(p_if);
    
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING) && GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        enable = TRUE;
    }

    hsrv_brgif_set_flags(p_if->ifindex, p_if->flags);
    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_RUNNING, enable));
    lib_ioctl_if_set_running(p_if->key.name, enable);

    MY_PRINTF("set %s %d", p_if->key.name, enable);
    hsrv_router_kernel_update_local(p_if->key.name, enable);
    return HSRV_E_NONE;
}

/*startup is shutdown and linkdown, then no shutdown, kernel will creat connect route, so here delete it*/
int32
hsrv_interface_update_if_state_by_up(tbl_interface_t* p_if)
{
    HSRV_PTR_CHECK(p_if);

    MY_PRINTF("update interface up %s %d", p_if->key.name, GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP));
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP) && !GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING))
    {
        MY_PRINTF("set %s %d", p_if->key.name, 0);
        hsrv_router_kernel_update_local(p_if->key.name, 0);
    }
    return HSRV_E_NONE;
}

int32
hsrv_interface_update_phy_type(tbl_interface_t* p_if)
{
    tbl_fiber_master_t *p_master = tbl_fiber_get_master();
    tbl_fiber_t *p_db_fiber = NULL;
    uint32 port_id = 0;

    /* has phy_type, need not updated */
    if (GLB_PHY_TYPE_UNKNOWN != p_if->phy_type)
    {
        return HSRV_E_NONE;
    }
        
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (port_id = 0; port_id < GLB_CHSM_PORT_NUM_MAX; port_id++)
    {
        p_db_fiber = p_master->fiber[port_id];
        if (NULL == p_db_fiber)
        {
            continue;
        }

        if (0 == sal_strcmp(p_if->key.name, p_db_fiber->interface_name))
        {
            p_if->phy_type = p_db_fiber->fiber_type;
            tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PHY_TYPE);
            break;
        }
    }
        
    return HSRV_E_NONE;
}
    
/* modified by yejl for bug 40393(37364), 2016-08-30 */
int32
hsrv_interface_update_default_vlan(tbl_interface_t* p_if)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_t* p_db_if_lag = NULL;
    ds_brgif_t *p_brgif = NULL;
    uint16     tagging_mode = 0;
    sai_object_id_t oid = 0;
    HSRV_PTR_CHECK(p_if);

    /* 1. check default vlan is belong to this port */
    p_db_if = tbl_interface_get_interface_by_ifindex(p_if->ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    p_brgif = p_db_if->brgif;
    if (NULL == p_brgif)
    {
        return HSRV_E_NONE;
    }
    if (!GLB_BMP_ISSET(p_brgif->allowed_vlan.vlan_bmp, p_brgif->pvid))
    {
        return HSRV_E_NONE;
    }

    /* 2. do add/remove action */
    if ((p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS))
    {
        tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    }
    else if (p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK || 
             p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
    {
        tagging_mode = SAI_VLAN_PORT_TAGGED;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &oid));

    if (tbl_interface_is_running(p_if))
    {
        /*added by yejl for bug 44993, 2019-03-07*/
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            /*get the agg interface table*/
            p_db_if_lag = hsrv_interface_get_agg_interface_by_lag_member_port(p_db_if);
            if (p_db_if_lag)
            {
                /*if agg interface is exist, judge the agg port is the mirror dest port or not*/
                if (!GLB_FLAG_ISSET(p_db_if_lag->mirror_enable, MIRROR_DEST_FLAG))
                {
                    HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(p_brgif->pvid, tagging_mode, oid));
                }
            }
        }
        else
        {
            /*added by yejl to fix bug41992, 2017-01-16*/
            if (!GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
            {
                HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(p_brgif->pvid, tagging_mode, oid)); 
            }    
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(p_brgif->pvid, tagging_mode, oid));        
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_update_if_dot1x_state(tbl_interface_t* p_if)
{
    uint32    enable  = FALSE;
    
    HSRV_PTR_CHECK(p_if);
    
    if (GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED))
    {
        enable = TRUE;
    }

    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_DOT1X_UNAUTHED, enable));

    if (enable)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_DOT1X_DIR_IN, enable));  

        if (GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT))
        {   
            HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_DOT1X_DIR_OUT, enable));  
        }
        else
        {
            HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_DOT1X_DIR_OUT, FALSE));              
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_interface_set_dot1x_guest_vlan(tbl_interface_t *p_if)
{ 
    sai_port_api_t    *port_api = NULL;
    sai_attribute_t   attr[1];
    sai_object_id_t   port = 0;
    tbl_interface_t   *p_db_if = NULL;
    ds_brgif_t        *p_brgif = NULL;
    uint16            old_pvid = 0;
    uint16            new_pvid = 0;
    uint16            tagging_mode = 0;
  
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);

    p_db_if = tbl_interface_get_interface_by_ifindex(p_if->ifindex);
    if (!p_db_if)
    {
        return HSRV_E_INVALID_PARAM;
    }

    if (p_db_if->dot1x_guest_vlan == p_if->dot1x_guest_vlan)
    {
        return HSRV_E_NONE;    
    }

    p_brgif = p_db_if->brgif;
    if (!p_brgif)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /* get port_id and tagging_mode */
    port = p_if->portid;

    if(p_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
    {
        tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    }
    else if(p_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK || p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
    {
        tagging_mode = SAI_VLAN_PORT_TAGGED;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }

    /* for dot1x port mode */
    if (!GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC))
    {
        if (p_db_if->dot1x_guest_vlan)
        {
            old_pvid = p_db_if->dot1x_guest_vlan;   
        }
        else
        {
            old_pvid = p_brgif->pvid;
        }
    
        if (p_if->dot1x_guest_vlan)
        {
            new_pvid = p_if->dot1x_guest_vlan;
        }
        else
        {
            new_pvid = p_brgif->pvid;    
        }

        /* remove old pvid */
        HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(old_pvid, tagging_mode, port));

        /* set new pvid */
        sal_memset(attr, 0, sizeof(attr));
        attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
        attr[0].value.u16 = new_pvid;
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

        HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(new_pvid, tagging_mode, port)); 
    }
    /* for dot1x mac mode, pvid won't be changed, only add guest_vlan to port
       or remove guest_vlan from port */
    else
    {
        if ((p_db_if->dot1x_guest_vlan)
            && (p_db_if->dot1x_guest_vlan != p_brgif->pvid))
        {
            old_pvid = p_db_if->dot1x_guest_vlan;  
            
            HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(old_pvid, tagging_mode, port));
        }

        if (p_if->dot1x_guest_vlan)
        {
            new_pvid = p_if->dot1x_guest_vlan;

            HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(new_pvid, tagging_mode, port)); 
        }
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_vlan_type
 * Purpose      : set interface vlan type, access or trunk   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlan_type(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_DEBUG("interface %s change to %d", p_if->key.name, p_if->vlan_type);
        
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    if(p_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS )
    {
        attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
        attr[0].value.u16 = GLB_VLAN_DEFAULT_VID;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
        
        attr[0].id = SAI_PORT_ATTR_DROP_TAGGED;
        attr[0].value.booldata = TRUE;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    } 
    else
    {
        attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
        attr[0].value.u16 = GLB_VLAN_DEFAULT_VID;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
        
        attr[0].id = SAI_PORT_ATTR_DROP_TAGGED;
        attr[0].value.booldata = FALSE;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }  

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_enable
 * Purpose      : set interface enable    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_enable(tbl_interface_t* p_if)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_FDB_LEARNING;
    attr[0].value.s32 = SAI_PORT_LEARN_MODE_HW;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
    attr[0].value.u16 = GLB_VLAN_DEFAULT_VID;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    
    attr[0].id = SAI_PORT_ATTR_DROP_TAGGED;
    attr[0].value.booldata = TRUE;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    attr[0].id = SAI_PORT_ATTR_DROP_UNTAGGED;
    attr[0].value.booldata = FALSE;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_disable
 * Purpose      : set interface disable
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_disable(tbl_interface_t* p_if)
{ 

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_default_vlan
 * Purpose      : set interface default vlan
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_default_vlan(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t*  port_api = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t* p_db_brgif = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t oid = 0;
    sai_object_id_t port = 0;
    uint32 tagging_mode = 0;
    /* modified by liwh for bug 54048, 2019-11-18 */
    uint32 is_set = FALSE;
    /* liwh end */
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
    attr[0].value.u16 = p_brgif->pvid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    /* update for basic qinq and vlan translation default entry */
    HSRV_IF_ERROR_RETURN(hsrv_vlanmap_if_set_pvid(p_if, p_brgif->pvid));

    /* added by yejl for bug 40393 (and 37364), 2016-08-30 */
    if (!(tbl_interface_is_running(p_if)))
    {
        HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &oid));
        if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK || p_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
        {
            tagging_mode = GLB_VLAN_PORT_TYPE_TRUNK;
            
            HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(p_brgif->pvid, tagging_mode, oid));

            p_db_if = tbl_interface_get_interface_by_ifindex(p_if->ifindex);
            if (NULL == p_db_if)
            {
                return HSRV_E_NO_MEMORY;
            }
            p_db_brgif = p_db_if->brgif;
            if (NULL == p_db_brgif)
            {
                return HSRV_E_NO_MEMORY;
            }
            if (p_db_brgif->pvid)
            {
                /* modified by liwh for bug 54048, 2019-11-18 
                   if old pvid is not allowed in interface, shouldn't add port to old pvid */
                HSRV_IF_ERROR_RETURN(hsrv_vlan_check_port_in_enable_vlan(p_db_brgif->pvid, p_if->ifindex, &is_set));
                if (is_set)
                /* liwh end */
                {
                    HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(p_db_brgif->pvid, tagging_mode, oid));
                }
            }
        }
        else if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
        {
            tagging_mode = GLB_VLAN_PORT_TYPE_ACCESS;
            HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(p_brgif->pvid, tagging_mode, oid));
            return HSRV_E_NONE;
        } 
        else
        {
            return HSRV_E_INVALID_PARAM;
        }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_inner_native_vid
 * Purpose      : set naitve inner vlan for qinq port
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_inner_native_vid(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    /* 1. set port vlan id using inner native vid */
    attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
    attr[0].value.u16 = p_brgif->inner_native_vid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    /* update for basic qinq and vlan translation default entry */
    HSRV_IF_ERROR_RETURN(hsrv_vlanmap_if_set_pvid(p_if, p_brgif->inner_native_vid));

    /* 2. set tag native property */
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_UNTAG_PVID;
    if (GLB_VLAN_NONE != p_brgif->inner_native_vid)
    {
        if (p_if->brgif->dot1q_tag_native_en)
        {
            attr[0].value.u16 = SAI_PORT_UNTAG_SVLAN;
        }
        else
        {
            attr[0].value.u16 = SAI_PORT_TAG_SVLAN;
        }
    }
    else
    {
        if (p_if->brgif->dot1q_tag_native_en)
        {
            attr[0].value.u16 = SAI_PORT_UNTAG_PVID;
        }
        else
        {
            attr[0].value.u16 = SAI_PORT_TAG_PVID;
        }
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_qinq_type
 * Purpose      : set interface qinq type
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_qinq_type(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_VLANMAP_TYPE;
    if (GLB_VLAN_QINQ_TYPE_BASIC == p_brgif->qinq_type)
    {
        /* no need to check vlan type qinq, switch to non-qinq port clear to basic qinq first */
        attr[0].value.u16 = SAI_VLAN_MAP_TYPE_BASIC_QINQ;
    }
    else if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_brgif->qinq_type)
    {
        attr[0].value.u16 = SAI_VLAN_MAP_TYPE_SELECTIVE_QINQ;
    }
    else
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

#if 0
    /* process default entry in SAI */
    if (GLB_VLAN_QINQ_TYPE_BASIC == p_brgif->qinq_type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_basic_qinq_add_delete_default_entry(p_if, p_if->brgif->pvid, TRUE, FALSE));
    }
    else if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_brgif->qinq_type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_basic_qinq_add_delete_default_entry(p_if, p_if->brgif->pvid, FALSE, FALSE));
    }
#endif
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_vlan_translation_en
 * Purpose      : set interface vlan translation enable
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlan_translation_en(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_VLANMAP_TYPE;
    if (p_brgif->vlan_translation_en)
    {
        attr[0].value.u16 = SAI_VLAN_MAP_TYPE_VLAN_REPLACE;
    }
    else
    {
        attr[0].value.u16 = SAI_VLAN_MAP_TYPE_NONE;
    }

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_vlan_range_en(tbl_interface_t *p_if, uint32 group_id, uint32 enable)
{
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_VLAN_RANGE_EN;
    attr[0].value.u32 = group_id + (enable << 16);

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_vlan_mapping_table
 * Purpose      : set interface qinq type
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlan_mapping_table(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t    *port_api    = NULL;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    sai_object_id_t    port = 0;
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt;
    tbl_vlan_mapping_entry_t    *p_db_vme;
    tbl_egress_mapping_entry_t  *p_db_egress_vme;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    tbl_egress_mapping_entry_master_t *p_egress_master = tbl_egress_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_group_key_t vg_key;
    tbl_vlan_group_t    *p_db_vg;
    ds_lag_t            *p_lag = NULL;
    tbl_interface_t     *p_if_mem = NULL;
    ctclib_slistnode_t  *listnode1 = NULL;
    uint32  is_add = FALSE;
    uint32  i = 0;
    uint32  group_evc_valid = FALSE;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));

#if 0
    /* remove for Bug 48048 when switch vlan translation enabled to access port */
    if (!p_if->brgif->vlan_translation_en && GLB_VLAN_QINQ_TYPE_SELECTIVE != p_if->brgif->qinq_type)
    {
        HSRV_IF_DEBUG("Interface %s not configured vlan translation enable or selective QinQ", p_if->key.name);
        return HSRV_E_INVALID_PARAM;
    }
#endif
    sal_memset(attr, 0, sizeof(attr));
    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));

    if (!sal_strlen(p_brgif->vlan_mapping_table))
    {
        /* delete vmt */
        is_add = FALSE;
        if (!sal_strlen(p_if->brgif->vlan_mapping_table))
        {
            HSRV_IF_DEBUG("Interface %s not configured vlan mapping table yet", p_if->key.name);
            return HSRV_E_INVALID_PARAM;
        }
        /* get configured vmt to be delete*/
        sal_strncpy(vmt_key.name, p_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
        if (NULL == p_db_vmt)
        {
            HSRV_IF_DEBUG("VMT %s not found", p_if->brgif->vlan_mapping_table);
            return HSRV_E_INVALID_PARAM;
        }
    }
    else
    {
        /* add vmt */
        is_add = TRUE;
        sal_strncpy(vmt_key.name, p_brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
        if (NULL == p_db_vmt)
        {
            HSRV_IF_DEBUG("VMT %s not found", p_if->brgif->vlan_mapping_table);
            return HSRV_E_INVALID_PARAM;
        }
    }

    sai_vm_entry.def_vid = p_if->brgif->pvid;
    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vme, listnode, next)
    {
        if (!sal_strncmp(p_db_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            if (GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type)
            {
                if (is_add)
                {
                    //HSRV_IF_ERROR_RETURN(hsrv_vlanmap_add_nexthop(port, p_db_vme->mapped_svid, p_db_vme->raw_vid));
                }
                else
                {
                   // HSRV_IF_ERROR_RETURN(hsrv_vlanmap_del_nexthop(port, p_db_vme->mapped_svid, p_db_vme->raw_vid));
                }
            }

            if ((p_db_vme->mapped_svid == p_if->brgif->pvid || p_db_vme->key.raw_vid == p_if->brgif->pvid)
                && GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type)
            {
                if (!is_add)
                {
                    hsrv_vlanmap_apply_vm_entry_for_defvid(p_if, p_if->brgif->pvid, FALSE);
                }
                else if (is_add && p_db_vme->key.raw_vid == p_if->brgif->pvid)
                {
                    hsrv_vlanmap_apply_vm_entry_for_defvid(p_if, p_db_vme->mapped_svid, TRUE);
                }
            }
            sai_vm_entry.port_id = port;
            sai_vm_entry.raw_vid = p_db_vme->key.raw_vid;
            sai_vm_entry.mapped_cvlan = p_db_vme->mapped_cvid;
            sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
            sai_vm_entry.default_fwd = p_if->brgif->qinq_default_fwd || p_if->brgif->vlan_trans_default_fwd;

            if (GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type)
            {
                sai_vm_entry.replace = TRUE;
            }
            else if (GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type)
            {
                sai_vm_entry.replace = FALSE;
            }
            else
            {
                HSRV_IF_DEBUG("Interface %s not configured trunk port or selective QinQ", p_if->key.name);
            }

            /* out of range entry */
            if (GLB_MAX_VLAN_ID == p_db_vme->key.raw_vid)
            {
                if (!is_add)
                {
                    sai_vm_entry.mapped_svlan = p_if->brgif->pvid;
                }
            }

            if (is_add)
            {
                HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
            }

            if (GLB_MAX_VLAN_ID == sai_vm_entry.raw_vid)
            {
                if (INTERFACE_IS_LAG_GROUP(p_if))
                {
                    p_lag = p_if->lag;
                    CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode1)
                    {
                        sai_vm_entry.port_id = p_if_mem->portid;
                        if (is_add)
                        {
                            HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                        }
                        else
                        {
                            HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                        }
                    }
                }
            }
        }
    }

    /* add raw vlan group entry for selective qinq */
    if (p_db_vmt->group_id && GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type)
    {
        group_evc_valid = TRUE;
        vg_key.group_id = p_db_vmt->group_id;
        p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
        if (NULL == p_db_vg)
        {
            HSRV_IF_DEBUG("VLAN group profile %d not exist", p_db_vmt->group_id);
        }

        sai_vm_entry.port_id  = port;
        sai_vm_entry.replace  = FALSE;
        sai_vm_entry.group_id = p_db_vmt->group_id;
        sai_vm_entry.mapped_svlan = p_db_vmt->group_mapped_svid;

        for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
        {
            if (p_db_vg->valid[i])
            {
                sai_vm_entry.raw_vid = p_db_vg->min_vid[i];
                sai_vm_entry.raw_vid_max = p_db_vg->max_vid[i];
            }

            if (is_add)
            {
                HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
            }
        }

        /* set vlanRangeProfile on phyport */
        HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_range_en(p_if, p_db_vg->key.group_id, is_add));
    }

    /* support for selective qinq egress scl */
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    tbl_mapped_vlan_info_t *p_mapped_info = NULL;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
    sai_vm_entry.egress  = TRUE;
    sai_vm_entry.port_id = port;
    if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_if->brgif->qinq_type)
    {
        CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_mapped_info, listnode, next)
        {
            if (!sal_strncmp(p_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                sai_vm_entry.mapped_svlan = p_mapped_info->mapped_svid;
                sai_vm_entry.mapped_cvlan = GLB_INVALID_VLANID;
                if (is_add)
                {
                    HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                }
                else
                {
                    HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                }

                if (group_evc_valid)
                {
                    if (p_db_vmt->group_mapped_svid == p_mapped_info->mapped_svid)
                    {
                        group_evc_valid = FALSE;
                    }
                }
            }
        }

        CTCLIB_SLIST_LOOP_DEL(p_egress_master->egress_mapping_entry_list, p_db_egress_vme, listnode, next)
        {
            if (!sal_strncmp(p_db_egress_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                sai_vm_entry.mapped_cvlan = p_db_egress_vme->key.raw_cvid;
                sai_vm_entry.mapped_svlan = p_db_egress_vme->key.raw_svid;
                sai_vm_entry.raw_vid = p_db_egress_vme->mapped_vid;

                if (is_add)
                {
                    HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                }
                else
                {
                    HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                }
            }
        }
    }

    if (group_evc_valid)
    {
        sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
        sai_vm_entry.mapped_svlan = p_db_vmt->group_mapped_svid;
        sai_vm_entry.mapped_cvlan = GLB_INVALID_VLANID;
        if (is_add)
        {
            HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_vlan_mapping_table_after(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    if (sal_strlen(p_brgif->vlan_mapping_table))
    {
        hsrv_vlanmap_if_set_pvid(p_if, p_if->brgif->pvid);
    }

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_vlanmap_default_fwd
 * Purpose      : set default forward action for selective qinq or vlan translation
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlanmap_default_fwd(tbl_interface_t *p_if, ds_brgif_t* p_brgif, uint32 isqinq)
{ 
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;
    uint32 out_of_range_entry_exist = 0;
    tbl_vlan_mapping_table_key_t vm_table_key;
    tbl_vlan_mapping_table_t    *p_db_vm_table = NULL;
    tbl_vlan_mapping_entry_t    *p_db_vm_entry = NULL;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
      
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD;

    /* get if there is out_of_range entry exist for selective qinq default fwd */
    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
    sal_strncpy(vm_table_key.name, p_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vm_table = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (NULL == p_db_vm_table)
    {
        out_of_range_entry_exist = 0;
    }
    else
    {
        CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
        {
            if (!sal_strncmp(p_db_vm_entry->key.table_name, p_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                if (GLB_MAX_VLAN_ID == p_db_vm_entry->key.raw_vid)
                {
                    out_of_range_entry_exist = 1;
                }
            }
        }
    }

    attr[0].value.u16 = (isqinq ? p_brgif->qinq_default_fwd : p_brgif->vlan_trans_default_fwd)
                        + (isqinq << 1) + (out_of_range_entry_exist << 1);

    if (GLB_VLAN_PORT_TYPE_ACCESS == p_if->vlan_type)
    {
        attr[0].value.u16 = 1;
    }

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_dot1q_tag_native_en
 * Purpose      : set default forward action for vlan translation enabled trunk port
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_dot1q_tag_native_en(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t  *port_api   = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_UNTAG_PVID;

    if (p_if->brgif->inner_native_vid)
    {
        if (p_brgif->dot1q_tag_native_en)
        {
            attr[0].value.u16 = SAI_PORT_UNTAG_SVLAN;
        }
        else
        {
            attr[0].value.u16 = SAI_PORT_TAG_SVLAN;
        }
    }
    else
    {
        if (p_brgif->dot1q_tag_native_en)
        {
            attr[0].value.u16 = SAI_PORT_UNTAG_PVID;
        }
        else
        {
            attr[0].value.u16 = SAI_PORT_TAG_PVID;
        }
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    hsrv_vlanmap_if_set_untag_pvid(p_if, !p_brgif->dot1q_tag_native_en);

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_ingress_filter_en
 * Purpose      : set interface ingress filter enable
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_ingress_filter_en(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_INGRESS_FILTERING;
    attr[0].value.booldata = p_brgif->ingress_filter;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_oper_mac_learning_en
 * Purpose      : set interface mac larning enable or disable
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_oper_mac_learning_en(tbl_interface_t *p_if, ds_brgif_t* p_brgif)
{ 
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    /* bypass LAG port, LAG is looped at PM  */
    if (GLB_IF_TYPE_PORT_IF == p_if->hw_type)
    {
        port = p_if->portid;
        sal_memset(attr, 0, sizeof(attr));
        attr[0].id = SAI_PORT_ATTR_FDB_LEARNING;
        if (p_brgif->oper_mac_learning_en)
        {
            attr[0].value.s32 = SAI_PORT_LEARN_MODE_HW;
        }
        else
        {
            attr[0].value.s32 = SAI_PORT_LEARN_MODE_DISABLE;
        }
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }
    
    /* Fix bug 47141 LAG mac_learning_en should store in DB for ARP MAC learning check */
    HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mac_learning_en(p_if->ifindex, p_brgif->oper_mac_learning_en));

    if (!p_brgif->oper_mac_learning_en)
    {
        /* if disable learning, should flush FDB */
        hsrv_fdb_flush_fdb_port(p_if->ifindex);
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_port_security_mac_learning
 * Purpose      : set interface mac learning enable or disable
 * Input        : p_if: table interfade; enable: true or false
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_port_security_mac_learning(tbl_interface_t *p_if, uint32 enable)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_FDB_LEARNING;
    if (enable)
    {
        attr[0].value.s32 = SAI_PORT_LEARN_MODE_HW;
    }
    else
    {
        attr[0].value.s32 = SAI_PORT_LEARN_MODE_DISABLE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mac_learning_en(p_if->ifindex, enable));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_port_security
 * Purpose      : set interface port security
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_port_security(tbl_interface_t *p_if, ds_brgif_t* p_brgif, bool is_max)
{ 
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    sai_object_id_t port = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_brgif);
    HSRV_PTR_CHECK(p_if);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    hsrv_interface_get_objectid(p_if, &port);
    sal_memset(attr, 0, sizeof(attr));
    if (p_brgif->port_security_en)
    {
        attr[0].id = SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES;
        attr[0].value.u32 = p_brgif->max_mac;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
        attr[0].id = SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION;

#if GOLDENGATE 
        if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT)
        {
            attr[0].value.s32 = SAI_PACKET_ACTION_DROP;
        }
        else if (GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_brgif->violate_mode ||
                 GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_brgif->violate_mode)
        {
            attr[0].value.s32 = SAI_PACKET_ACTION_TRAP;
        }
#elif defined USW
        if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT)
        {
            attr[0].value.s32 = SAI_PACKET_ACTION_DROP;
        }
        else if (GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_brgif->violate_mode ||
                 GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_brgif->violate_mode)
        {
            attr[0].value.s32 = SAI_PACKET_ACTION_TRAP;
        }    
#else
        if (p_brgif->violate_mode == GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT)
        {
            if (is_max)
            {
                attr[0].value.s32 = SAI_PACKET_ACTION_DROP;
            }
            else
            {
                attr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;
            }
        }
        else if (GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_brgif->violate_mode ||
                 GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_brgif->violate_mode)
        {
            if (is_max)
            {
                attr[0].value.s32 = SAI_PACKET_ACTION_TRAP;
            }
            else
            {
                attr[0].value.s32 = SAI_PACKET_ACTION_LOG;    
            }
        }
#endif
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }
    else
    {
        attr[0].id = SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION;
        attr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

        attr[0].id = SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES;
        attr[0].value.u32 = 0xFFFFFFFF;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_mode
 * Purpose      : set interface mode enable      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_mode(tbl_interface_t* p_if)
{
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    
    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    
    if (GLB_IF_MODE_L2== p_if->mode)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid,SAI_PORT_FLAG_ROUTED_EN, FALSE));
    }
    else if(GLB_IF_MODE_L3== p_if->mode)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid,SAI_PORT_FLAG_ROUTED_EN, TRUE));
    }
    
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_sflow_en
 * Purpose      : set interface sflow
 * Input        : p_if: table interfade; p_brgif: table bridge interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_sflow_en(sai_object_id_t port_id, bool dir, uint16 threshold, bool enable)
{ 
    sai_attribute_t attr[1];
    sai_port_api_t*  port_api = NULL;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    attr[0].id = SAI_PORT_ATTR_SFLOW_EN;
    attr[0].value.sflow.dir = dir;
    attr[0].value.sflow.threshold = threshold;
    attr[0].value.sflow.enable = enable;

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_id, attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_get_sflow_threshold_from_rate
 * Purpose      : get sflow threshold from rate    
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
uint16
hsrv_interface_get_sflow_threshold(tbl_interface_t  *p_db_if)
{
    uint32  weight = 0;
    uint32  index = 0;
    uint32  from = 0;
    uint16  threshold = 0;

    HSRV_PTR_CHECK(p_db_if);
#ifndef _GLB_UML_SYSTEM_/*modify by yangl for bug 45988*/
/* modified by liwh for bug 52440, 2019-06-18 */
//#ifdef  GOLDENGATE
#if defined(GOLDENGATE) || defined(DUET2) || defined(TSINGMA)
/* liwh end */
    from = p_db_if->sflow_rate;
    for (index = 0; from; index++)
    {
        from = from >> 1;
    }

    weight = GLB_ACL_LOG_BIT_LEN - (uint8)(index -1);
    threshold = 1 << weight;
    threshold = threshold > GLB_SFLOW_MAX_LOG_THRESHOLD ? GLB_SFLOW_MAX_LOG_THRESHOLD : threshold;
    return threshold;
#elif defined (GREATBELT)
    from = p_db_if->sflow_rate;
    for (index = 0; from; index++)
    {
        from = from >> 1;
    }

    weight = GLB_ACL_LOG_GB_BIT_LEN - (uint8)(index -1);
    threshold = 1 << weight;
    threshold = threshold > GLB_SFLOW_MAX_LOG_GB_THRESHOLD ? GLB_SFLOW_MAX_LOG_GB_THRESHOLD  : threshold;
    return threshold;
#else
    from = p_db_if->sflow_rate;
    for (index = 0; from; index++)
    {
        from = from >> 1;
    }

    weight = GLB_ACL_LOG_UML_BIT_LEN - (uint8)(index -1);
    threshold = 1 << weight;
    threshold = threshold > GLB_SFLOW_MAX_LOG_UML_THRESHOLD ? GLB_SFLOW_MAX_LOG_UML_THRESHOLD  : threshold;
    return threshold;
#endif
#else
    from = p_db_if->sflow_rate>>5;
    for (index = 0; from; index++)
    {
        from = from >> 1;
    }

    weight = GLB_ACL_LOG_UML_BIT_LEN - (uint8)(index -1);
    threshold = 1 << weight;
    threshold = threshold > GLB_SFLOW_MAX_LOG_UML_THRESHOLD ? GLB_SFLOW_MAX_LOG_UML_THRESHOLD  : threshold;
    return threshold;
#endif
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_sflow_ingress
 * Purpose      : set interface sflow_ingress enable      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_sflow_ingress(tbl_interface_t* p_if)
{
    tbl_interface_t  *p_db_if = NULL;
    uint16           threshold = 0;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    HSRV_PTR_CHECK(p_db_if);

    if (0 == p_db_if->sflow_rate)
    {
        p_db_if->sflow_rate = p_if->sflow_rate;
    }

    threshold = hsrv_interface_get_sflow_threshold(p_db_if);
    hsrv_interface_set_sflow_en(p_if->portid, GLB_INGRESS, threshold, p_if->sflow_flow_ingress);

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_sflow_egress
 * Purpose      : set interface sflow egress enable      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_sflow_egress(tbl_interface_t* p_if)
{
    tbl_interface_t  *p_db_if = NULL;
    uint16           threshold = 0;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    HSRV_PTR_CHECK(p_db_if);

    if (0 == p_db_if->sflow_rate)
    {
        p_db_if->sflow_rate = p_if->sflow_rate;
    }
    
    threshold = hsrv_interface_get_sflow_threshold(p_db_if);
    hsrv_interface_set_sflow_en(p_if->portid, GLB_EGRESS, threshold, p_if->sflow_flow_egress);
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_sflow_rate
 * Purpose      : set interface sflow rate      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_sflow_rate(tbl_interface_t* p_if)
{
    tbl_interface_t  *p_db_if = NULL;
    uint16           threshold = 0;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    HSRV_PTR_CHECK(p_db_if);

    p_db_if->sflow_rate = p_if->sflow_rate;

    if (p_db_if->sflow_flow_ingress)
    { 
        threshold = hsrv_interface_get_sflow_threshold(p_db_if);
        hsrv_interface_set_sflow_en(p_if->portid, GLB_INGRESS, threshold, p_if->sflow_flow_ingress);
    }
    
    if (p_db_if->sflow_flow_egress)
    {
        threshold = hsrv_interface_get_sflow_threshold(p_db_if);
        hsrv_interface_set_sflow_en(p_if->portid, GLB_EGRESS, threshold, p_if->sflow_flow_egress);    
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_isolate_group
 * Purpose      : set interface sflow egress enable      
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_isolate_group(tbl_interface_t* p_if)
{
    sai_port_api_t* port_api;
    sai_object_id_t port_oid = 0;
    sai_attribute_t attr;

    sal_memset(&attr, 0, sizeof(attr));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    if (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    port_oid = p_if->portid;

    attr.id = SAI_PORT_ATTR_ISOLATION_DIRECTION;
    attr.value.s32 = SAI_PORT_ISOLATION_DIRECTION_BOTH;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    /*added by yejl to fix bug40904, 2016-09-27*/
    attr.id = SAI_PORT_ATTR_ISOLATION_TYPE;
    attr.value.s32 = SAI_PORT_ISOLATION_TYPE_DEFAULT;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
    
    attr.id = SAI_PORT_ATTR_ISOLATION_GROUP;
    HSRV_IF_ERROR_RETURN(hsrv_isolation_get_oid(p_if->isolate_group_id, &attr.value.oid));
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_interface_set_pvlan_enable(tbl_interface_t* p_if, sai_object_id_t port_oid, sai_object_id_t group_oid)
{
    sai_attribute_t attr;
    sai_port_api_t* port_api = NULL;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    /*set the pvlan type to SAI*/
    attr.id = SAI_PORT_ATTR_ISOLATION_TYPE;
    if (GLB_IF_PVLAN_TYPE_COMMUNITY == p_if->pvlan_type)
    {
        attr.value.s32 = SAI_PORT_ISOLATION_PVLAN_TYPE_COMMUNITY;
    }
    else if (GLB_IF_PVLAN_TYPE_ISOLATED == p_if->pvlan_type)
    {
        attr.value.s32 = SAI_PORT_ISOLATION_PVLAN_TYPE_ISOLATE;
    }
    else if (GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_if->pvlan_type)
    {
        attr.value.s32 = SAI_PORT_ISOLATION_PVLAN_TYPE_PROMISCUOUS;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    /*set the pvlan group id to SAI*/
    attr.id = SAI_PORT_ATTR_ISOLATION_GROUP;
    attr.value.oid = group_oid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_interface_set_pvlan_disable(tbl_interface_t* p_if, sai_object_id_t port_oid)
{
    sai_attribute_t attr;
    sai_port_api_t* port_api = NULL;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    /* only set the pvlan type to SAI*/
    attr.id = SAI_PORT_ATTR_ISOLATION_TYPE;
    if (GLB_IF_PVLAN_TYPE_NONE == p_if->pvlan_type)
    {
        attr.value.s32 = SAI_PORT_ISOLATION_TYPE_DEFAULT;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_pvlan(tbl_interface_t* p_if)
{
    sai_object_id_t port_oid = 0;
    sai_object_id_t group_oid = 0;
    sai_attribute_t attr;

    sal_memset(&attr, 0, sizeof(attr));

    if (GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type && GLB_IF_TYPE_PORT_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    /*get the port object id*/
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &port_oid));
    group_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ISOLATION_GROUP, p_if->pvlan_group_id);
    
    if (GLB_IF_PVLAN_TYPE_NONE == p_if->pvlan_type)
    {
        hsrv_interface_set_pvlan_disable(p_if, port_oid);
    }
    else
    {
        hsrv_interface_set_pvlan_enable(p_if, port_oid, group_oid);
    }
   
    return HSRV_E_NONE;
}

int32
hsrv_interface_set_mlag_peer_link(tbl_interface_t* p_if, ds_brgif_t* p_brgif)
{
#ifdef CONFIG_MLAG
    HSRV_IF_ERROR_RETURN(hsrv_mlag_set_isolate_port(p_if->ifindex, p_brgif->mlag_is_peer_link, SAI_PORT_ISOLATION_DIRECTION_INGRESS));
    /* flushed flush at hsrv_interface_set_oper_mac_learning_en()
    if (p_brgif->mlag_is_peer_link)
    {
        hsrv_fdb_flush_fdb_port(p_if->ifindex);
    }
    */
#endif /* !CONFIG_MLAG */

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_mlag_port_block(tbl_interface_t* p_if, ds_brgif_t* p_brgif)
{
#ifdef CONFIG_MLAG
    HSRV_IF_ERROR_RETURN(hsrv_mlag_set_isolate_port(p_if->ifindex, p_brgif->mlag_is_port_block, SAI_PORT_ISOLATION_DIRECTION_EGRESS));
#endif /* !CONFIG_MLAG */

    return HSRV_E_NONE;
}

int32
hsrv_interface_set_untagged_vlan(tbl_interface_t* p_if, ds_brgif_t* p_brgif)

{
    ds_brgif_t *p_db_brgif = NULL;
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[2];
    sai_object_id_t portoid = 0;
    vid_t vid;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_DEBUG("Interface %s untagged %d", p_if->key.name);

    p_db_brgif = ds_brgif_get_brgif(p_if);
    if (NULL == p_db_brgif)
    {
        return HSRV_E_NONE;
    }
    
    /*get the port object id*/
    hsrv_interface_get_objectid(p_if, &portoid);
    
    GLB_BMP_ITER_BEGIN(p_db_brgif->untagged_vlan.vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_brgif->untagged_vlan.vlan_bmp, vid))
        {   
            attr[0].id = SAI_PORT_ATTR_UNTAGGED_VLAN;
            attr[0].value.u32 = FALSE;
            attr[1].value.u16 = vid;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(portoid, attr));
        }
    }
    GLB_BMP_ITER_END(p_db_brgif->untagged_vlan.vlan_bmp, vid);
    

    GLB_BMP_ITER_BEGIN(p_brgif->untagged_vlan.vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, vid))
        {
            attr[0].id = SAI_PORT_ATTR_UNTAGGED_VLAN;
            attr[0].value.u32 = TRUE;
            attr[1].value.u16 = vid;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(portoid, attr));
        }
    }
    GLB_BMP_ITER_END(p_brgif->untagged_vlan.vlan_bmp, vid);
    
    return HSRV_E_NONE;
}


int32
hsrv_interface_set_mlag_is_group(tbl_interface_t* p_if, ds_brgif_t* p_brgif)
{
#ifdef CONFIG_MLAG
    HSRV_IF_ERROR_RETURN(hsrv_mlag_set_is_group(p_if->ifindex, p_brgif->mlag_is_group));
#endif /* !CONFIG_MLAG */

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_erps_pdu_action
 * Purpose      : set interface erps pdu action   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_erps_pdu_action(tbl_interface_t* p_if)
{
    /*if (INTERFACE_IS_LAG_GROUP(p_if))
    {
        return HSRV_E_NONE;
    }*/
    
    if (p_if->erps_pdu_action < GLB_PDU_L2PDU_PORT_ACTION_MAX)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_erps_pdu_action(p_if->portid, p_if->erps_pdu_action));
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : _hsrv_interface_set_vlanclass_type_and_group_id
 * Purpose      : set interface vlanclass active type and groupid  
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
_hsrv_interface_set_vlanclass_type_and_group_id(tbl_interface_t* p_if)
{
    sai_object_id_t port_oid = 0;
    sai_port_api_t* port_api;
    sai_attribute_t attr;
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    /*get the port object id*/
    HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_if, &port_oid));

    /*set the vlanclass active type*/
    attr.id = SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE;
    if (GLB_VLAN_CLASS_MAC_BASED == p_if->vlanclass_active_type)
    {
        attr.value.s32 = SAI_PORT_VLANCLASS_ACTIVE_TYPE_MAC;
    }
    else if (GLB_VLAN_CLASS_IP_BASED == p_if->vlanclass_active_type)
    {
        attr.value.s32 = SAI_PORT_VLANCLASS_ACTIVE_TYPE_IP;
    }
    else if (GLB_VLAN_CLASS_PROTOCOL_BASED == p_if->vlanclass_active_type)
    {
        attr.value.s32 = SAI_PORT_VLANCLASS_ACTIVE_TYPE_PROTOCOL;
    }
    else
    {
        attr.value.s32 = SAI_PORT_VLANCLASS_ACTIVE_TYPE_NONE;
    }
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    /*set vlanclass group id*/
    attr.id = SAI_PORT_ATTR_VLANCLASS_GROUP_ID;
    attr.value.s32 = p_if->vlanclass_group_id;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    return HSRV_E_NONE;
}   


/***************************************************************************************************
 * Name         : hsrv_interface_set_vlanclass_type_and_group_id
 * Purpose      : set interface vlanclass active t 
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlanclass_type_and_group_id(tbl_interface_t* p_if)
{
    tbl_interface_t* p_if_mem = NULL;
    tbl_interface_t* p_db_lagif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ds_lag_t       *p_lag = NULL;

    if ((GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type) && (GLB_IF_TYPE_PORT_IF != p_if->hw_type))
    {
        return HSRV_E_NONE;
    }
    
    /*If the p_if is linkagg port, get the member port of agg and set the vlanclass with the member port*/
    if ((INTERFACE_IS_LAG_GROUP(p_if)))
    {
        p_db_lagif = tbl_interface_get_interface(&p_if->key);
        p_lag = p_db_lagif->lag;
        if (NULL == p_lag)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_if_mem->vlanclass_active_type = p_if->vlanclass_active_type;
            p_if_mem->vlanclass_group_id = p_if->vlanclass_group_id;
            HSRV_IF_ERROR_RETURN(_hsrv_interface_set_vlanclass_type_and_group_id(p_if_mem));
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(_hsrv_interface_set_vlanclass_type_and_group_id(p_if));
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : _hsrv_interface_set_vlanclass_default_action
 * Purpose      : set interface vlanclass default action   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
_hsrv_interface_set_vlanclass_default_action(tbl_interface_t* p_if)
{
    sai_port_api_t* port_api;
    sai_object_id_t port_oid = 0;
    sai_attribute_t attr;

    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    /*get the port object id*/
    HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_if, &port_oid));

    /*set the vlanclass default action*/
    attr.id = SAI_PORT_ATTR_VLANCLASS_DEFAULT_ACTION;
    attr.value.s32 = p_if->vlanclass_default_action;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_vlanclass_default_action
 * Purpose      : set interface vlanclass default action   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_vlanclass_default_action(tbl_interface_t* p_if)
{
    tbl_interface_t* p_if_mem = NULL;
    tbl_interface_t* p_db_lagif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ds_lag_t       *p_lag = NULL;

    if ((GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type) && (GLB_IF_TYPE_PORT_IF != p_if->hw_type))
    {
        return HSRV_E_NONE;
    }

    /*If the p_if is linkagg port, get the member port of agg and set the vlanclass with the member port*/
    if ((INTERFACE_IS_LAG_GROUP(p_if)))
    {
        p_db_lagif = tbl_interface_get_interface(&p_if->key);
        p_lag = p_db_lagif->lag;
        if (NULL == p_lag)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_if_mem->vlanclass_default_action = p_if->vlanclass_default_action;
            HSRV_IF_ERROR_RETURN(_hsrv_interface_set_vlanclass_default_action(p_if_mem));
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(_hsrv_interface_set_vlanclass_default_action(p_if));
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_dot1x_enable
 * Purpose      : set interface dot1x enable   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_dot1x_enable(tbl_interface_t* p_if)
{
    if (p_if->dot1x_enable)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_dot1x_pdu_action(p_if->portid, GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_dot1x_pdu_action(p_if->portid, GLB_PDU_L2PDU_PORT_ACTION_DISCARD));
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_set_uplink_enable
 * Purpose      : set interface uplink enable   
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_uplink_enable(tbl_interface_t* p_if)
{
    if (p_if->l2pro_uplink)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_dot1x_pdu_action(p_if->portid, GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_dot1x_pdu_action(p_if->portid, GLB_PDU_L2PDU_PORT_ACTION_DISCARD));
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_creat_port_if 
 * Purpose      : creat table port interface, it is used for DB lookup in FEA  
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_creat_port_if(tbl_interface_t* p_if)
{
    tbl_fea_port_if_t port_if;
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t attr[1];
    int32 rc = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_DEBUG("interface %s create %d", p_if->key.name);

    if(GLB_IF_TYPE_PORT_IF == p_if->hw_type)
    {
        port_if.key.portid = p_if->portid;
        port_if.ifindex = p_if->ifindex;
        sal_memcpy(port_if.name, p_if->key.name, sizeof(p_if->key.name));

        sal_mutex_lock(g_hsrv_brgif_mutex);
        rc = tbl_fea_port_if_add_fea_port_if(&port_if);
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        if (rc < 0)
        {
            return rc;
        }

        attr[0].id = SAI_PORT_ATTR_DROP_TAGGED;
        attr[0].value.booldata = TRUE;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_if->portid, attr));
    
        attr[0].id = SAI_PORT_ATTR_DROP_UNTAGGED;
        attr[0].value.booldata = FALSE;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_if->portid, attr));
    }

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_remove_port_if 
 * Purpose      : remove table port interface, it is used for DB lookup in FEA  
 * Input        : p_if: table interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_remove_port_if(tbl_interface_t* p_if)
{
    tbl_fea_port_if_t port_if;
    int32 rc = 0;
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    if(GLB_IF_TYPE_PORT_IF == p_if->hw_type)
    {
        port_if.key.portid = p_if->portid;
        sal_mutex_lock(g_hsrv_brgif_mutex);
        rc = tbl_fea_port_if_del_fea_port_if(&port_if.key);
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        if (rc < 0)
        {
            return rc;
        }
        hsrv_hostif_remove(p_if->hostifid);
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_hash_value_profile_select(tbl_interface_t* p_if)
{
    sai_port_api_t* port_api;
    sai_object_id_t port_oid = 0;
    sai_attribute_t attr;
    tbl_interface_t *p_db_if = NULL;

    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;

    /* 1. feature process */ 
    if ((GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type) && (GLB_IF_TYPE_PORT_IF != p_if->hw_type))
    {
        return HSRV_E_NONE;
    }

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        p_lag = p_db_if->lag;
        if (NULL == p_lag)
        {
            return HSRV_E_NONE;
        }
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            /*get the port object id*/
            HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_if_mem, &port_oid));
            
            /*set the vlanclass default action*/
            attr.id = SAI_PORT_ATTR_HASH_VALUE_PROFILE_ID;
            attr.value.s32 = p_if->hash_value_profile_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        }
    }
    else
    {
        /*get the port object id*/
        HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_if, &port_oid));

        /*set the vlanclass default action*/
        attr.id = SAI_PORT_ATTR_HASH_VALUE_PROFILE_ID;
        attr.value.s32 = p_if->hash_value_profile_id;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_hash_value_set_agg_hash(tbl_interface_t* p_if)
{
    sai_port_api_t* port_api;
    sai_object_id_t port_oid = 0;
    sai_attribute_t attr;

    if ((GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type) && (GLB_IF_TYPE_PORT_IF != p_if->hw_type))
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    /*get the port object id*/
    HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_if, &port_oid));

    /*set the vlanclass default action*/
    attr.id = SAI_PORT_ATTR_HASH_VALUE_AGG_HASH;
    attr.value.s32 = p_if->hash_value_agg_hash;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_brgif_sync
 * Purpose      : load interface bridge cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_brgif_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t *p_if_key = NULL;
    ds_brgif_t* p_brgif = NULL;
    uint32 oper_mac_learning_en = TRUE;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_brgif = (ds_brgif_t*)p_ds;
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        p_if_key = (tbl_interface_key_t*)p_tbl;
        hsrv_brgif_add(p_db_if, p_brgif);
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        hsrv_brgif_del(p_db_if);
        break;

    case CDB_OPER_SET:
        p_brgif = (ds_brgif_t*)p_ds;
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_db_if);
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            switch(field_id)
            {
            case DS_BRGIF_FLD_OPER_MAC_LEARNING_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_oper_mac_learning_en(p_db_if, p_brgif));
                break;
                
            case DS_BRGIF_FLD_PVID:
                if ((0 != p_brgif->pvid) && ((0 == p_db_if->dot1x_guest_vlan)
                    || GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC)))
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_default_vlan(p_db_if, p_brgif));
                }
                break;

            case DS_BRGIF_FLD_INNER_NATIVE_VID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_inner_native_vid(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_QINQ_TYPE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_qinq_type(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_VLAN_TRANSLATION_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_translation_en(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_mapping_table(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_QINQ_DEFAULT_FWD:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanmap_default_fwd(p_db_if, p_brgif, TRUE));
                break;

            case DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanmap_default_fwd(p_db_if, p_brgif, FALSE));
                break;

            case DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_dot1q_tag_native_en(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_INGRESS_FILTER:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_ingress_filter_en(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_PORT_SECURITY_EN:
                if (p_db_if->brgif)
                {
                    oper_mac_learning_en = p_db_if->brgif->oper_mac_learning_en;
                }
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, FALSE));
                HSRV_IF_ERROR_RETURN(hsrv_brgif_set_port_security_en(p_db_if->ifindex, p_brgif->port_security_en));
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_db_if->ifindex));
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, oper_mac_learning_en));
                break;
            case DS_BRGIF_FLD_MAX_MAC:
                HSRV_IF_ERROR_RETURN(hsrv_brgif_set_max_mac(p_db_if->ifindex, p_brgif->max_mac));
                if (p_brgif->port_security_en)
                {
                    if (p_db_if->brgif)
                    {
                        oper_mac_learning_en = p_db_if->brgif->oper_mac_learning_en;
                    }
                    /*1.learning disable*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, FALSE));

#ifdef GOLDENGATE                    
                    /*2.set port-security*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#elif defined USW
                    /*2.set port-security*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#endif
                    /*3.flush dynamic fdb*/
                    HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_db_if->ifindex));

                    /*modified by yejl for GB port-security bug48233, 2018-07-26*/
#ifdef GREATBELT
                    if (p_brgif->max_mac)
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                    }
                    else
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, TRUE));
                    }
#endif
                    /*ended by yejl*/

                    /*4. learning enable*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, oper_mac_learning_en));
                }
#ifdef GOLDENGATE     
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                } 
#elif defined USW
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                }
#endif
                break;
            case DS_BRGIF_FLD_VIOLATE_MODE:
#ifdef GOLDENGATE
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#elif defined USW
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#else
                HSRV_IF_ERROR_RETURN(hsrv_security_op_security(p_db_if->ifindex, p_brgif));
#endif
                break;

            case DS_BRGIF_FLD_MLAG_IS_PEER_LINK:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_mlag_peer_link(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_mlag_port_block(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_UNTAGGED_VLAN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_untagged_vlan(p_db_if, p_brgif));

            default:
                break;
            }
        }
        else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        {
            switch(field_id)
            {
            case DS_BRGIF_FLD_OPER_MAC_LEARNING_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_oper_mac_learning_en(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_MLAG_IS_GROUP:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_mlag_is_group(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_PORT_SECURITY_EN:
                if (p_db_if->brgif)
                {
                    oper_mac_learning_en = p_db_if->brgif->oper_mac_learning_en;
                }
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, FALSE));
                HSRV_IF_ERROR_RETURN(hsrv_brgif_set_port_security_en(p_db_if->ifindex, p_brgif->port_security_en));
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_db_if->ifindex));
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, oper_mac_learning_en));
                break;
            case DS_BRGIF_FLD_MAX_MAC:
                HSRV_IF_ERROR_RETURN(hsrv_brgif_set_max_mac(p_db_if->ifindex, p_brgif->max_mac));
                if (p_brgif->port_security_en)
                {
                    if (p_db_if->brgif)
                    {
                        oper_mac_learning_en = p_db_if->brgif->oper_mac_learning_en;
                    }
                    /*1.learning disable*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, FALSE));

#ifdef GOLDENGATE                    
                    /*2.set port-security*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#elif defined USW
                    /*2.set port-security*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#endif                    
                    /*3.flush dynamic fdb*/
                    HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_db_if->ifindex));

                    /*4. learning enable*/
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_db_if, oper_mac_learning_en));
                }
#ifdef GOLDENGATE     
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                } 
#elif defined USW
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
                }
#endif
                break;
            case DS_BRGIF_FLD_VIOLATE_MODE:
#ifdef GOLDENGATE
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#elif defined USW
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_db_if, p_brgif, FALSE));
#else
                HSRV_IF_ERROR_RETURN(hsrv_security_op_security(p_db_if->ifindex, p_brgif));
#endif
                break;

            /* bug48149 add process for agg pvid to sync default entry for vlan mapping */
            case DS_BRGIF_FLD_PVID:
                if ((0 != p_brgif->pvid) && ((0 == p_db_if->dot1x_guest_vlan)
                    || GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC)))
                {
                    HSRV_IF_ERROR_RETURN(hsrv_interface_set_default_vlan(p_db_if, p_brgif));
                }
                break;

            case DS_BRGIF_FLD_INNER_NATIVE_VID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_inner_native_vid(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_QINQ_TYPE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_qinq_type(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_VLAN_TRANSLATION_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_translation_en(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_mapping_table(p_db_if, p_brgif));
                break;

            case DS_BRGIF_FLD_QINQ_DEFAULT_FWD:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanmap_default_fwd(p_db_if, p_brgif, TRUE));
                break;

            case DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanmap_default_fwd(p_db_if, p_brgif, FALSE));
                break;

            case DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_dot1q_tag_native_en(p_db_if, p_brgif));
                break;

            default:
                break;
            }
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_brgif_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t *p_if_key = NULL;
    ds_brgif_t* p_brgif = NULL;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_SET:
        p_brgif = (ds_brgif_t*)p_ds;
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_db_if);
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            switch(field_id)
            {
            case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_mapping_table_after(p_db_if, p_brgif));
                break;

            default:
                break;
            }
        }
        else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        {
            switch(field_id)
            {
            case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_mapping_table_after(p_db_if, p_brgif));
                break;

            default:
                break;
            }
        }
        break;

    default:
        return HSRV_E_NONE;
    }

    return HSRV_E_NONE;
}

extern int32
hsrv_openflow_msg_port_set_protected_vlan(uint32 ifindex, bool enable, uint32 vlan_id);
extern int32
hsrv_openflow_msg_port_ignore_vlan_filter(uint32 ifindex, bool ignore_vlan_filter);
extern int32
hsrv_openflow_msg_port_tunnel_mpls_disable(uint32 ifindex, bool tunnel_mpls_disable);
extern int32
hsrv_openflow_msg_port_native_vlan_config(uint32 ifindex, uint32 ingress_add_native_vlan, uint32 egress_remove_native_vlan);
extern int32
hsrv_openflow_msg_port_set_port_act(uint32 ifindex, uint32 *packet_action);
extern int32
fei_api_openflow_tunnel_port_decap_en(fei_openflow_port_decap_req_t* pst_req);

int32 hsrv_openflow_interface_set_protected_vlan(tbl_openflow_interface_t* p_openflow_if)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_set_protected_vlan(p_openflow_if->ifindex, p_openflow_if->protected_vlan_change_is_set, p_openflow_if->protected_vlan_change));
    return 0;
}

int32 hsrv_openflow_interface_set_vlan_filter(tbl_openflow_interface_t* p_openflow_if)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_ignore_vlan_filter(p_openflow_if->ifindex, !p_openflow_if->obey_vlan_filter));
    return 0;
}

int32 hsrv_openflow_interface_set_tunnel_mpls_disable(tbl_openflow_interface_t* p_openflow_if)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_tunnel_mpls_disable(p_openflow_if->ifindex, p_openflow_if->tunnel_mpls_disable));
    return 0;
}

int32 hsrv_openflow_interface_set_native_vlan_config(tbl_openflow_interface_t* p_openflow_if)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_native_vlan_config(p_openflow_if->ifindex, p_openflow_if->ingress_add_native_vlan_enable, p_openflow_if->egress_remove_native_vlan_enable));
    return 0;
}

int32 hsrv_openflow_interface_set_port_act(tbl_openflow_interface_t* p_openflow_if)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_set_port_act(p_openflow_if->ifindex, p_openflow_if->action));
    return 0;
}

int32 hsrv_openflow_interface_set_tunnel_port_decap_en(tbl_openflow_interface_t* p_openflow_if)
{
    fei_openflow_port_decap_req_t req;
    sal_memset(&req, 0x0, sizeof(fei_openflow_port_decap_req_t));
    
    if(GLB_IF_TYPE_NVGRE_IF == p_openflow_if->bind_tunnel_type)
    {
        req.type = GLB_OPENFLOW_TUNNEL_TYPE_NVGRE;
    }
    else if(GLB_IF_TYPE_VXLAN_IF == p_openflow_if->bind_tunnel_type)
    {
        req.type = GLB_OPENFLOW_TUNNEL_TYPE_VXLAN;
    }
    else if(GLB_IF_TYPE_L2GRE_IF == p_openflow_if->bind_tunnel_type)
    {
        req.type = GLB_OPENFLOW_TUNNEL_TYPE_L2GRE;
    }
    req.ifindex = p_openflow_if->ifindex;
    req.enable = 1;
    
    HSRV_IF_ERROR_RETURN(fei_api_openflow_tunnel_port_decap_en(&req));

    return 0;
}

int32 hsrv_openflow_interface_set_openflow_enable(tbl_openflow_interface_t* p_openflow_if)
{

    fei_openflow_port_decap_req_t req;
    sal_memset(&req, 0x0, sizeof(fei_openflow_port_decap_req_t));
    req.type = GLB_OPENFLOW_TUNNEL_TYPE_VXLAN;
    req.ifindex = p_openflow_if->ifindex;
    req.enable = p_openflow_if->openflow_enable;
    
    HSRV_IF_ERROR_RETURN(fei_api_openflow_tunnel_port_decap_en(&req));
    return 0;
}

int32
hsrv_openflow_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_openflow_interface_t* p_openflow_if = NULL;
    p_openflow_if = (tbl_openflow_interface_t*)p_tbl;
    if(NULL == p_openflow_if)
    {
        return -1;
    }
    switch (oper)
    {
        case CDB_OPER_SET:
            switch (field_id)
            {
                case TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_protected_vlan(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_vlan_filter(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_tunnel_mpls_disable(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE:
                case TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_native_vlan_config(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_ACTION:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_port_act(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_tunnel_port_decap_en(p_openflow_if));
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE:
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_interface_set_openflow_enable(p_openflow_if));
                    break;
                default:
                    break;
            }
        break;
            
    }
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_sync
 * Purpose      : load interface cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_if = NULL;
    tbl_port_stats_t port_stats;
    tbl_interface_t     *pst_if_lag     = NULL;
    tbl_interface_key_t if_key;

    /*1. debug out infor*/
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    sal_memset(&port_stats, 0, sizeof(port_stats));
    /*2. process ds */
    if(p_ds_node)
    {
        switch (p_ds_node->id)
        {
            case DS_BRGIF:
                HSRV_IF_ERROR_RETURN(hsrv_interface_brgif_sync(p_tbl_node, p_ds_node, 
                    oper, field_id, p_tbl, p_ds));
                break;
            case DS_LAG:
                HSRV_IF_ERROR_RETURN(hsrv_lag_sync(p_tbl_node, p_ds_node, 
                    oper, field_id, p_tbl, p_ds));
                break;   
            case DS_STORM_CONTROL:
                 HSRV_IF_ERROR_RETURN(hsrv_interface_storm_control_sync(p_tbl_node, p_ds_node, 
                    oper, field_id, p_tbl, p_ds));
                break;  
            default:
                return HSRV_E_NONE;  
        }
        return HSRV_E_NONE;
    }
    
    /*3. process tbl*/
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_interface_t*)p_tbl;
        HSRV_PTR_CHECK(p_if);
        if(p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_creat_port_if(p_if));
            port_stats.key.ifindex = p_if->ifindex;
            port_stats.port_id = p_if->portid;
            HSRV_IF_ERROR_RETURN(tbl_port_stats_add_port_stats(&port_stats));
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_enable(p_if));
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_admin_state(p_if));
        }
        else if(p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            port_stats.key.ifindex = p_if->ifindex;
            HSRV_IF_ERROR_RETURN(tbl_port_stats_add_port_stats(&port_stats));
        }
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_if);
        if(p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_disable(p_if));
            HSRV_IF_ERROR_RETURN(hsrv_interface_remove_port_if(p_if));
            port_stats.key.ifindex = p_if->ifindex;
            HSRV_IF_ERROR_RETURN(tbl_port_stats_del_port_stats(&port_stats.key));
            hsrv_fdb_flush_fdb_port(p_if->ifindex);
        }
        else if(p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            port_stats.key.ifindex = p_if->ifindex;
            HSRV_IF_ERROR_RETURN(tbl_port_stats_del_port_stats(&port_stats.key));
        }
        break;

    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        HSRV_PTR_CHECK(p_if);

        if (p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_SPEED:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_speed(p_if));
                break;
                
            case TBL_INTERFACE_FLD_SVLAN_TPID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_svlan_tpid(p_if));
                break;

            case TBL_INTERFACE_FLD_DUPLEX:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_duplex(p_if));
                break;

            case TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_loopback(p_if));
                break;

            case TBL_INTERFACE_FLD_UNIDIR:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_unidir(p_if));
                break;

            case TBL_INTERFACE_FLD_FEC:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_fec(p_if));
                break;

            case TBL_INTERFACE_FLD_MEDIA:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_media(p_if));
                break;
                
            case TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_holdtime(p_if));
                break;

            case TBL_INTERFACE_FLD_JUMBOFRAME_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_jumboframe_en(p_if));
                break;

            case TBL_INTERFACE_FLD_FLOWCONTROL_SEND:
            case TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_flow_control(p_if));
                break;
                
            case TBL_INTERFACE_FLD_PFC_ADMIN:
                break;

            case TBL_INTERFACE_FLD_IP_SOURCE_EN:
            case TBL_INTERFACE_FLD_IP_SOURCE_TYPE:
            case TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER:
                hsrv_interface_set_ip_source_guard(p_if, field_id);
                break;
             
            case TBL_INTERFACE_FLD_PFC_PRIORITY:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_priority_flow_control(p_if));
                break;
                
            case TBL_INTERFACE_FLD_UP:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_admin_state(p_if));
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state_by_up(p_if));
                break;
                
            case TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE:
                ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_AGG_IS_ACTIVE, p_if->static_agg_is_active);
                /*ADD by yangl for bug 53272*/
                if (INTERFACE_IS_LAG_MEMBER(p_if))
                {
                    sal_memset(&if_key, 0, sizeof(if_key));
                    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_if->lag->lag_id);
                    pst_if_lag = tbl_interface_get_interface(&if_key);
                    if(!pst_if_lag)
                    {
                        return HSRV_E_NONE;
                    }
                    
                    if (pst_if_lag->mirror_dest_id)
                    {
                        ctc_sai_port_set_port_flag(p_if->portid, SAI_PORT_FLAG_MIRROR_DEST, TRUE);
                    }
                    
                }
                break;

            case TBL_INTERFACE_FLD_RUNNING:
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state(p_if));
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_default_vlan(p_if));
                break;

            case TBL_INTERFACE_FLD_VLAN_TYPE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_type(p_if));
                break;
                
            case TBL_INTERFACE_FLD_MODE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_mode(p_if));
                break;

            case TBL_INTERFACE_FLD_SFLOW_RATE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_sflow_rate(p_if));
                break;
                
            case TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_sflow_ingress(p_if));
                break;
                
            case TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_sflow_egress(p_if));
                break;

            case TBL_INTERFACE_FLD_ISOLATE_GROUP_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_isolate_group(p_if));
                break;
                
            case TBL_INTERFACE_FLD_PVLAN_GROUP_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_pvlan(p_if));
                break;

            case TBL_INTERFACE_FLD_ERPS_PDU_ACTION:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_erps_pdu_action(p_if));
                break;

            case TBL_INTERFACE_FLD_DOT1X_ENABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_dot1x_enable(p_if));
                break;

            case TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED:
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_dot1x_state(p_if));
                break;

            case TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_dot1x_guest_vlan(p_if));
                break;

            case TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_type_and_group_id(p_if));
                break;

            case TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_default_action(p_if));
                break;
                
            case TBL_INTERFACE_FLD_EGS_ACL_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_egress_acl_en(p_if));
                break;

            case TBL_INTERFACE_FLD_TAP_USED:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_tap_used(p_if));
                break;

            case TBL_INTERFACE_FLD_METADATA_ENABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_metadata(p_if));
                break;
            case TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE:
		        HSRV_IF_ERROR_RETURN(hsrv_interface_set_latency_event_enable(p_if));
	            break;
 
            case TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH:
		        HSRV_IF_ERROR_RETURN(hsrv_interface_set_latency_event_threshold(p_if));
                break;

            case TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_reflective_en(p_if));
                break;
                
            case TBL_INTERFACE_FLD_L2PRO_UPLINK:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_uplink_enable(p_if));
                break;
            case TBL_INTERFACE_FLD_OPENFLOW_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_openflow_en(p_if));
                break;
                
            case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_hash_value_profile_select(p_if));
                break;
                
            case TBL_INTERFACE_FLD_POE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_poe(p_if));
                break;    

/* system modified by liwh for bug 53010, 2019-07-29 */
#ifdef GREATBELT
            case TBL_INTERFACE_FLD_MAC_ADDR:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_mac(p_if));
                break;
#endif
/* liwh end */

            case TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_crc_error_check_en(p_if));
                break;

            case TBL_INTERFACE_FLD_CRC_RECALCULATION_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_crc_recalculation_en(p_if));
                break;
                
            default:
                break;
            }
            break;
        }
        else if (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_MLAG_ID:
                HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mlag_id(p_if->ifindex, p_if->mlag_id));
                break;
            case TBL_INTERFACE_FLD_RUNNING:
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state(p_if));
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_default_vlan(p_if));
                break;
            case TBL_INTERFACE_FLD_UP:
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state_by_up(p_if));
                break;
            case TBL_INTERFACE_FLD_PVLAN_GROUP_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_pvlan(p_if));
                break;
            case TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_type_and_group_id(p_if));
                break;
            case TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_default_action(p_if));
                break;         

            case TBL_INTERFACE_FLD_EGS_ACL_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_egress_acl_en(p_if));
                break;

            case TBL_INTERFACE_FLD_METADATA_ENABLE:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_metadata(p_if));
                break;

            case TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_reflective_en(p_if));
                break;
                
            case TBL_INTERFACE_FLD_L2PRO_UPLINK:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_uplink_enable(p_if));
                break;
            case TBL_INTERFACE_FLD_OPENFLOW_EN:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_openflow_en(p_if));
                break;
            case TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_openflow_operation_agg(p_if));
                break;
            case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID:
                HSRV_IF_ERROR_RETURN(hsrv_interface_hash_value_profile_select(p_if));
                break;
            case TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH:
                HSRV_IF_ERROR_RETURN(hsrv_interface_hash_value_set_agg_hash(p_if));
                break;
            default:
                break;
            }
            break;
        }
        else if (p_if->hw_type == GLB_IF_TYPE_VLAN_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_UP:
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vif_admin_state(p_if));
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state_by_up(p_if));
                break;
                /*for first add vlan interface, config ip address, then access this vlan, no shutdown this eth port, kernel not has vlan connect route*/
            case TBL_INTERFACE_FLD_RUNNING:
                HSRV_IF_ERROR_RETURN(hsrv_interface_update_if_state_for_vlan(p_if));
                break;
            }
            
        }
    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_interface_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_if = NULL;
    int32 ifindex = 0;
    uint64 hostif_id = 0;

    /*1. debug out infor*/
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    /* Added by kcao for bug 39271, bypass DS add operation */
    if (p_ds_node)
    {
        switch (p_ds_node->id)
        {
            case DS_BRGIF:
                HSRV_IF_ERROR_RETURN(hsrv_interface_brgif_sync_after(p_tbl_node, p_ds_node, 
                    oper, field_id, p_tbl, p_ds));
                break;
            default:
                return HSRV_E_NONE;  
        }
        return HSRV_E_NONE;
    }
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_interface_t*)p_tbl;
        HSRV_PTR_CHECK(p_if);
        if(p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            hsrv_hostif_create(p_if->key.name, p_if->portid, &hostif_id);
            p_if->hostifid = hostif_id;
            tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HOSTIFID);  
            
            /* Added by kcao for bug 39030, update phy_type from TBL_FIBER */
            HSRV_IF_ERROR_RETURN(hsrv_interface_update_phy_type(p_if));       
        }
        break;

    /* Added by yejl for bug 39502, flush the fdb of DB by port */
    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        ifindex = tbl_interface_get_ifindex_by_name(p_if_key->name);
        if (ifindex > 0)
        {
            hsrv_fdb_db_flush_by_port(ifindex);
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_port_set_bypass(tbl_port_t* p_port)
{
    lcapi_lcm_set_port_status_t portstatus;
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);

    portstatus.slot_idx = p_port->slot_no;
    portstatus.port_idx = p_port->panel_port_no;
    portstatus.subport_idx = p_port->panel_sub_port_no;
    portstatus.status = (uint32)((p_port->bypass<<4)|p_port->bypass_slot);
    HSRV_IF_ERROR_RETURN(lcm_port_set_bypass(&portstatus));

    return HSRV_E_NONE;
}

int32
hsrv_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_port_t *p_port = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_port = (tbl_port_t*)p_tbl;
        switch (field_id)
        {
            case TBL_PORT_FLD_BYPASS:
                HSRV_IF_ERROR_RETURN(hsrv_port_set_bypass(p_port));
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_get_objectid
 * Purpose      : get objectid from interface        
 * Input        : p_db_if, interface in db structrue
 * Output       : port_oid, port objectid                
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_get_objectid(tbl_interface_t *p_db_if, sai_object_id_t *port_oid)
{
    tbl_fea_lag_key_t fea_lag_key;
    tbl_fea_lag_t*    p_fea_lag = NULL;
    sai_object_id_t   oid = 0;
    sal_memset(&fea_lag_key, 0, sizeof(tbl_fea_lag_key_t));
    
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) /*port*/
    {
        oid = p_db_if->portid;
    }
    else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type) /*link_agg*/        
    {
        fea_lag_key.id = p_db_if->ifindex - GLB_AGG_IFINDEX_MIN;
        p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
        if (NULL == p_fea_lag)
        {
            return HSRV_E_INVALID_PARAM;
        }
        oid = p_fea_lag->lag_id;
    }
    else if (GLB_IF_TYPE_CPU_IF == p_db_if->hw_type) /*port*/
    {
        oid = p_db_if->portid;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    *port_oid = oid;
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_ifindex_to_objectid
 * Purpose      : mapping interface ifindex to objectid        
 * Input        : ifindex, interface index
 * Output       : port_oid, port objectid                
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_ifindex_to_objectid(uint32 ifindex, sai_object_id_t *port_oid)
{
    tbl_interface_t*  p_db_if = NULL;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    return hsrv_interface_get_objectid(p_db_if, port_oid);
}

/***************************************************************************************************
 * Name         : hsrv_interface_mapping_port_status
 * Purpose      : mapping port status from sai to system      
 * Input        : sai_port_status, port physical status
 * Output       : hsrv_port_status, system port status          
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_mapping_port_status(sai_port_oper_status_t sai_port_status, uint8 *hsrv_port_status)
{
    switch(sai_port_status)
    {
        case SAI_PORT_OPER_STATUS_UNKNOWN:
            *hsrv_port_status = GLB_PORT_OPER_STATUS_UNKNOWN; 
            break;

        case SAI_PORT_OPER_STATUS_UP:
            *hsrv_port_status = GLB_PORT_OPER_STATUS_UP; 
            break;

        case SAI_PORT_OPER_STATUS_DOWN:
            *hsrv_port_status = GLB_PORT_OPER_STATUS_DOWN; 
            break;

        case SAI_PORT_OPER_STATUS_TESTING:
            *hsrv_port_status = GLB_PORT_OPER_STATUS_TESTING; 
            break;

        case SAI_PORT_OPER_STATUS_NOT_PRESENT:
            *hsrv_port_status = GLB_PORT_OPER_STATUS_NOT_PRESENT; 
            break;

        default:
            return HSRV_E_INVALID_PARAM;
    
    }
        
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_interface_get_if_by_oid
 * Purpose      : get interface structure by object id
 * Input        : oid, port object id
 * Output       : if, tbl_interface if              
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_get_if_by_oid(sai_object_id_t oid, tbl_interface_t** pp_if)
{
    tbl_fea_port_if_t* p_port_if = NULL;
    tbl_fea_port_if_key_t  fea_port_if_key;
    tbl_interface_key_t if_key;
    uint32 port_type;

    port_type = CTC_SAI_OBJECT_TYPE_GET(oid);
    if(SAI_OBJECT_TYPE_PORT != port_type && SAI_OBJECT_TYPE_LAG != port_type)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }

    fea_port_if_key.portid = oid;
    p_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key); 
    if(!p_port_if)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }
    sal_memcpy(if_key.name, p_port_if->name, sizeof(p_port_if->name));
    *pp_if = tbl_interface_get_interface(&if_key);
    if (NULL == *pp_if)
    {
        return HSRV_E_INTERFACE_NOT_EXIT;
    }
    
    return HSRV_E_NONE; 
}

/***************************************************************************************************
 * Name         : hsrv_interface_port_state_change_notification_cb
 * Purpose      : process port up/down state change notification event callback 
 * Input        : count, event count; noti_data, portid and change state
 * Output       :              
 * Return       : void
 * Note         : N/A
***************************************************************************************************/
void hsrv_interface_port_state_change_notification_cb(
    uint32 count,
    sai_port_oper_status_notification_t *noti_data)
{
    uint32 i;
    tbl_interface_t* p_if = NULL;
    
    /* Modified by kcao for bug 37806, 2016-03-29
    char* phy_port_status[] = {"UNKNOWN", "UP", "DOWN", "TESTING", "NOT PRESENT"};
    */

    if( 0 == count )
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "% hsrv_interface_port_state_change_notification FAIL, count = 0 \n");
        return;
    }
    if( NULL == noti_data)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "% hsrv_interface_port_state_change_notification FAIL, noti data is NULL\n");
        return;
    }
    for(i=0; i<count; i++)
    {
        HSRV_L2IF_DEBUG("port_state_change_notification_cb state is %d, port_id is %d",
           noti_data->port_state,  (uint32)(noti_data->port_id));
        
        if(noti_data->port_state > SAI_PORT_OPER_STATUS_NOT_PRESENT)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "% hsrv_interface_port_state_change_notification state is out of scope!\n");
            continue;
        }
        hsrv_interface_get_if_by_oid(noti_data->port_id, &p_if);
        if(NULL == p_if)
        {
            HSRV_L2IF_DEBUG("% hsrv_interface_port_state_change_notification fail, if is null\n");
            continue;
        }

        /* Modified by kcao for bug 37806, 2016-03-29
         * all port notify should though LAI but not SAI, for we need send port duplex status before link status
        HSRV_L2IF_DEBUG("port state change notify, %s, state is %s\n", p_if->key.name, phy_port_status[noti_data->port_state]);
        hsrv_interface_mapping_port_status(noti_data->port_state, &p_if->phy_status);
        ret = tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PHY_STATUS);
        if(ret)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "% hsrv_interface_port_state_change_notification fail, ret = %d!\n", ret);
        }
        */
    }

    return;
}

int32
_hsrv_interface_cmd_dump_fea_port_if(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
    field_parse_t field;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    tbl_fea_port_if_iterate((TBL_ITER_CB_FUNC)tbl_fea_port_if_dump_one, &args);
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
hsrv_interface_cmd_process_fea_port_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32          rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        CFG_INVALID_PARAM_RET("Not support to set FDB");
        break;

    case CDB_OPER_GET:
        _hsrv_interface_cmd_dump_fea_port_if(para);
        
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define NL_SOCK_NAME_LEN    20
typedef struct
{
    int32 sock;
    int32 seq;
    struct sal_sockaddr_nl snl;
    char name[NL_SOCK_NAME_LEN];
    ctc_task_t *t_read;
} lib_nl_sock_t;

typedef struct
{
    lib_nl_sock_t *listen[GLB_MAX_VRFID];
    lib_nl_sock_t *cmd;
    lib_nl_sock_t *arp;
} lib_nl_master_t;

lib_nl_master_t *g_nl_master = NULL;

int32
hsrv_interface_start()
{
    cdb_register_cfg_tbl_cb(TBL_FEA_PORT_IF, hsrv_interface_cmd_process_fea_port_if);
    
    g_nl_master = XCALLOC(MEM_PM_LIB_MASTER, sizeof(lib_nl_master_t));
    if (NULL == g_nl_master)
    {
        return PM_E_NO_MEMORY;
    }

    /* Initialize cmd sock */
    (lib_netlink_init_rt_cmd());
    return HSRV_E_NONE;
}

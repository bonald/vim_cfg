/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "lsrv_inc.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

extern int32
hsrv_lag_memberport_down_fast_protect(tbl_interface_t *p_db_if);

extern int32
hsrv_port_stats_clear_stats(uint32 ifindex);

int32
lsrv_port_100g_speed_clear_port_stats(tbl_interface_t *p_db_if)
{
#ifdef GOLDENGATE
    char ifname_ext[IFNAME_SIZE];
    char cmd_buf[CMD_BUF_128];
    
    if ((GLB_SPEED_40G == p_db_if->oper_speed) || (GLB_SPEED_100G == p_db_if->oper_speed))
    {
        log_sys(M_MOD_INTF, E_WARNING, "Interface %s operate speed changes, and clear counters on it", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        hsrv_port_stats_clear_stats(p_db_if->ifindex);
        sal_snprintf(cmd_buf, CMD_BUF_128, "cdbctl update/cdb/sys/clear_if_stats_rate/%s", p_db_if->key.name);
        sal_system(cmd_buf);
    }

#endif /* !GOLDENGATE */
    return 0;    
}

/***************************************************************************************************
 * Name         : lsrv_port_event_cb
 * Purpose      : creat or delete port from lai notification event    
 * Input        : event, add or delete; port, port information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_port_event_cb(lai_port_event_t  event,lai_port_info_t* p_port)
{
    tbl_port_t port;
    tbl_port_t* p_db_port = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    tbl_interface_t *p_db_if = NULL;

    LSRV_PORT_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_port)
    {
       log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_port_evnent_cb, PORT is null!\n");
       return ;
    }
    sal_memset(&port, 0, sizeof(port));

    if(event == LAI_PORT_EVENT_ADD)
    {
        LSRV_PORT_DEBUG("PORT insert in, slot %d, id %d\n", p_port->slot_id, (int32)p_port->oid);
        port.key.port_id = p_port->oid;
        p_db_port = tbl_port_get_port(&port.key);
        if(p_db_port)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_port_evnent_cb, PORT add but port is already exist!\n");
            return;
        }
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "port");
        sal_sprintf(str, "%d", port.key.port_id);
        sal_strcat(buff, str);
        //sal_strcpy(port.name, buff);
        sal_strncpy(port.name, p_port->name, IFNAME_SIZE);

        port.logic_port_id = p_port->portid;
        port.panel_port_no = p_port->pannel_port_id;
        port.media = p_port->media;
        port.port_media_type = p_port->commber;
        
        port.phy_type = p_port->phy_type;
        port.phyinfo_flag = p_port->phyport_flag;
        port.panel_sub_port_no = p_port->pannel_sub_port_id;
        port.slot_no = p_port->slot_id;
        port.speed_capbility = p_port->speed_caps;
        port.support_media_switch = p_port->support_media_switch;
        port.bypass = p_port->bypass;
        sal_memcpy(port.mac, p_port->mac,6);

    LSRV_PORT_DEBUG("PORT insert in, logic_port %d, pannel_port_no %d\n", port.logic_port_id, port.panel_port_no);
		
        tbl_port_add_port(&port);
    }
    else if(event == LAI_PORT_EVENT_DELETE)
    {
        LSRV_PORT_DEBUG("PORT removed, slot %d, id %d\n", p_port->slot_id,(int32)p_port->oid);
        port.key.port_id = p_port->oid;
        tbl_port_del_port(&port.key);
    }
    else if(event == LAI_PORT_EVENT_UP)
    {
        LSRV_PORT_DEBUG("PORT status up, slot %d, id %d\n", p_port->slot_id,(int32)p_port->oid);

        if_build_ethname_by_slot_port(ifname, p_port->slot_id, p_port->pannel_port_id, p_port->pannel_sub_port_id);
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if(NULL == p_db_if)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "there are interface exist for Port slot %d, id %d", p_port->slot_id,(int32)p_port->oid);
            return;
        }
        
        /* Added by kcao for bug 37806, 2016-03-29, must before sync PHY_STATUS */
        if (p_db_if->oper_duplex != p_port->duplex)
        {
            p_db_if->oper_duplex = p_port->duplex;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPER_DUPLEX);
        }

        if (p_db_if->oper_speed != p_port->speed)
        {
            p_db_if->oper_speed = p_port->speed;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPER_SPEED);
        }
        
        if (p_db_if->prev_oper_speed != p_db_if->oper_speed)
        {
            lsrv_port_100g_speed_clear_port_stats(p_db_if);
        }
        p_db_if->prev_oper_speed = p_db_if->oper_speed;
        
        p_db_if->phy_status = GLB_PORT_OPER_STATUS_UP;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_STATUS);

        /* for bug 45768, by tongzb, update the phy_type when config media_type */
        if (p_db_if->phy_type!= p_port->phy_type)
        {        
            p_db_if->phy_type = p_port->phy_type;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_TYPE);  
        }

        if (p_db_if->fec_status!= p_port->fec_status)
        {        
            p_db_if->fec_status = p_port->fec_status;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_FEC_STATUS);  
        }
        if (p_db_if->cl73_ability!= p_port->cl73_ability)
        {        
            p_db_if->cl73_ability = p_port->cl73_ability;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_CL73_ABILITY);  
        }
    }
    else if(event == LAI_PORT_EVENT_DOWN)
    {
        LSRV_PORT_DEBUG("PORT status down, slot %d, id %d\n", p_port->slot_id,(int32)p_port->oid);

        if_build_ethname_by_slot_port(ifname, p_port->slot_id, p_port->pannel_port_id, p_port->pannel_sub_port_id);
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if(NULL == p_db_if)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "there are interface exist for Port slot %d, id %d", p_port->slot_id,(int32)p_port->oid);
            return;
        }
        /* Added by kcao for bug 43619, do LAG member port fast protection */
        hsrv_lag_memberport_down_fast_protect(p_db_if);
        
        p_db_if->phy_status = GLB_PORT_OPER_STATUS_DOWN;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_STATUS);
        
        /* Added by kcao for bug 37806, 2016-03-29 */
        if (p_db_if->oper_duplex != p_port->duplex)
        {
            p_db_if->oper_duplex = p_port->duplex;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPER_DUPLEX);
        }
        /* for bug 45768, by tongzb, update the phy_type when config media_type */
        if (p_db_if->phy_type!= p_port->phy_type)
        {        
            p_db_if->phy_type = p_port->phy_type;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_TYPE);  
        }        
    }

    return;
}

/***************************************************************************************************
 * Name         : lsrv_port_update_port
 * Purpose      : update port info in cdb
 * Input        : p_db_port, port info in db, p_lai_port, real port info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_port_update_port(tbl_port_t * p_db_port, lai_port_info_t* p_lai_port)
{
    int32 rc = 0;

    LSRV_PORT_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_port || NULL == p_lai_port)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_port_update_port, PORT is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_port->slot_id != p_db_port->slot_no)
    {
        return PM_E_NONE;
    }

    if((GLB_FLAG_ISSET(p_lai_port->status,LAI_PORT_STATUS_PRESENT))&&
        (p_db_port->present_status == 1))
    {
        return PM_E_NONE;
    }

    if((!GLB_FLAG_ISSET(p_lai_port->status,LAI_PORT_STATUS_PRESENT))&&
        (p_db_port->present_status == 0))
    {
        return PM_E_NONE;
    }   
    
    if(GLB_FLAG_ISSET(p_lai_port->status,LAI_PORT_STATUS_PRESENT))
    {
        p_db_port->present_status = 1;
    }
    {
        p_db_port->present_status = 0;
    }
    
    LSRV_PORT_DEBUG("PORT update, slot %d, id %d\n", p_lai_port->slot_id, (int32)p_lai_port->oid); 
    PM_E_RETURN(tbl_port_set_port_field(p_db_port, TBL_PORT_FLD_PRESENT_STATUS));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_port_set_iter_status
 * Purpose      : set port info from lai layer to db
 * Input        : p_db_port, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_port_get_iter_status(tbl_port_t* p_db_port, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_port_api_t*  port_api = NULL;
    lai_port_info_t lai_port;
    lai_object_id_t oid;

    LSRV_PORT_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_port)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_PORT,(void**)&port_api));
    oid = p_db_port->key.port_id;
    

    PM_E_RETURN(port_api->get_port_info(oid, &lai_port));
    PM_E_RETURN(lsrv_port_update_port(p_db_port, &lai_port));

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : lsrv_port_routine_handle
 * Purpose      : poll all port status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_port_routine_handle()
{
    tbl_port_iterate((TBL_ITER_CB_FUNC) lsrv_port_get_iter_status, NULL);

    return PM_E_NONE;
}






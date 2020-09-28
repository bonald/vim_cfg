/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "ctc_sai_port.h"
/******************************************************************************
* Global and Declaration
******************************************************************************/
#define HSRV_RATELIMIT_THRESHOLD_RATIO    (1.0)
/******************************************************************************
* Defines and Macros
******************************************************************************/


/***************************************************************************************************
 * Name         : hsrv_interface_storm_control_get_link_speed 
 * Purpose      : get kbps according to port speed
 * Input        : speed: port speed
 * Output       : N/A                   
 * Return       : kbps 
 * Note         : N/A
***************************************************************************************************/
uint32
hsrv_interface_storm_control_get_link_speed(uint8 speed)
{
  switch (speed)
    {
      case GLB_SPEED_AUTO:
        return 0;
      case GLB_SPEED_10M:
        return 10000;
      case GLB_SPEED_100M:
        return 100000;
      case GLB_SPEED_1G:
        return 1000000;
      case GLB_SPEED_2_5G:
        return 2500000;
      case GLB_SPEED_5G:
        return 5000000;
      case GLB_SPEED_10G:
        return 10000000;
      case GLB_SPEED_40G:
        return 40000000;
      case GLB_SPEED_100G:
        return 100000000; 
      case GLB_SPEED_50G:
        return 50000000;
      case GLB_SPEED_25G:
        return 25000000;        
      default:
        return 0;
    }
}


/***************************************************************************************************
 * Name         : hsrv_interface_storm_control_get_level_rate 
 * Purpose      : get storm control level_rate
 * Input        : p_if: table interface, level: value of storm_control level
 * Output       : N/A                   
 * Return       : level_rate
 * Note         : N/A
***************************************************************************************************/
uint32
hsrv_interface_storm_control_get_level_rate(tbl_interface_t* p_if, uint32 level)
{
    uint32 level_rate = 0;
    uint32 bw = 0;
    float  percent = 0;
    percent = (float) level/10000.0;
    bw = hsrv_interface_storm_control_get_link_speed(p_if->oper_speed);
    level_rate = (uint32) (((HSRV_RATELIMIT_THRESHOLD_RATIO * bw * 1000.0 / 8.0) * percent)/1000.0);
    return level_rate;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_storm_control_broadcast 
 * Purpose      : set storm control broadcast
 * Input        : p_if: table interface, p_stormctl: table storm_control interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_storm_control_broadcast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl)
{
    sai_port_api_t*  port_api = NULL;
    sai_policer_api_t*  policer_api = NULL;
    sai_attribute_t attr[5];
    sai_attribute_t storm_stl_attr;
    sai_object_id_t port = 0;
    sai_object_id_t policer_oid = 0;
    uint32          attr_count = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_stormctl);
    HSRV_PTR_CHECK(p_if);
    sal_memset(&storm_stl_attr, 0, sizeof(storm_stl_attr));
    sal_memset(attr, 0x0, sizeof(attr));
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    storm_stl_attr.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;

    if (GLB_RATELIMIT_MODE_NONE == p_stormctl->bcast_mode)
    {
        storm_stl_attr.value.oid = SAI_NULL_OBJECT_ID;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
        return HSRV_E_NONE;
    }
    else if (GLB_RATELIMIT_MODE_PPS == p_stormctl->bcast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
        attr_count ++;    
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = p_stormctl->bcast_rate;
        attr_count ++;
    }
    else if (GLB_RATELIMIT_MODE_LEVEL == p_stormctl->bcast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
        attr_count ++;
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = hsrv_interface_storm_control_get_level_rate(p_if, p_stormctl->bcast_rate);
        attr_count ++;
    }
    else 
    {
        return HSRV_E_INVALID_PARAM;
    }

    attr[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr[attr_count].value.s32 = SAI_POLICER_MODE_STORM_CONTROL;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;    
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_oid, attr_count, attr));
    
    storm_stl_attr.value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_storm_control_multicast
 * Purpose      : set storm control multicast
 * Input        : p_if: table interface, p_stormctl: table storm_control interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_storm_control_multicast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl)
{
    sai_port_api_t*  port_api = NULL;
    sai_policer_api_t*  policer_api = NULL;
    sai_attribute_t attr[5];
    sai_attribute_t storm_stl_attr;
    sai_object_id_t port = 0;
    sai_object_id_t policer_oid = 0;
    uint32          attr_count = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_stormctl);
    HSRV_PTR_CHECK(p_if);
    sal_memset(&storm_stl_attr, 0, sizeof(storm_stl_attr));
    sal_memset(attr, 0x0, sizeof(attr));
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    storm_stl_attr.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;

    if (GLB_RATELIMIT_MODE_NONE == p_stormctl->mcast_mode)
    {
        storm_stl_attr.value.oid = SAI_NULL_OBJECT_ID;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
        return HSRV_E_NONE;
    }
    else if (GLB_RATELIMIT_MODE_PPS == p_stormctl->mcast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
        attr_count ++;   
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = p_stormctl->mcast_rate;
        attr_count ++;
    }
    else if (GLB_RATELIMIT_MODE_LEVEL == p_stormctl->mcast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
        attr_count ++;
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = hsrv_interface_storm_control_get_level_rate(p_if, p_stormctl->mcast_rate);
        attr_count ++;
    }
    else 
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    attr[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr[attr_count].value.s32 = SAI_POLICER_MODE_STORM_CONTROL;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;    
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_oid,attr_count,attr));
    
    storm_stl_attr.value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
    
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_set_storm_control_unicast 
 * Purpose      : set storm control unicast
 * Input        : p_if: table interface, p_stormctl: table storm_control interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_interface_set_storm_control_unicast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl)
{
    sai_port_api_t*  port_api = NULL;
    sai_policer_api_t*  policer_api = NULL;
    sai_attribute_t attr[5];
    sai_attribute_t storm_stl_attr;
    sai_object_id_t port = 0;
    sai_object_id_t policer_oid = 0;
    uint32          attr_count = 0;
    
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_stormctl);
    HSRV_PTR_CHECK(p_if);
    sal_memset(&storm_stl_attr, 0, sizeof(storm_stl_attr));
    sal_memset(attr, 0x0, sizeof(attr));
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    storm_stl_attr.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;

    if (GLB_RATELIMIT_MODE_NONE == p_stormctl->ucast_mode)
    {
        storm_stl_attr.value.oid = SAI_NULL_OBJECT_ID;
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
        return HSRV_E_NONE;
    }
    else if (GLB_RATELIMIT_MODE_PPS == p_stormctl->ucast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
        attr_count ++; 
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = p_stormctl->ucast_rate;
        attr_count ++;
    }
    else if (GLB_RATELIMIT_MODE_LEVEL == p_stormctl->ucast_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
        attr_count ++;
        attr[attr_count].id = SAI_POLICER_ATTR_CIR;
        attr[attr_count].value.u64 = hsrv_interface_storm_control_get_level_rate(p_if, p_stormctl->ucast_rate);
        attr_count ++;
    }
    else 
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    attr[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr[attr_count].value.s32 = SAI_POLICER_MODE_STORM_CONTROL;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;
    attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;    
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_oid,attr_count,attr));
    
    storm_stl_attr.value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, &storm_stl_attr));
    
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_interface_storm_control_sync
 * Purpose      : load interface storm control cdb to sai          
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
hsrv_interface_storm_control_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_t *p_if = NULL;
    tbl_interface_key_t* p_if_key = NULL;
    ds_storm_control_t* p_stormctl = NULL;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_stormctl = (ds_storm_control_t*)p_ds;
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_if);
        HSRV_PTR_CHECK(p_stormctl);
        if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            return HSRV_E_NONE;
        }
        
        if (p_stormctl->bcast_mode)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_broadcast(p_if, p_stormctl));
        }
        if (p_stormctl->mcast_mode)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_multicast(p_if, p_stormctl));
        }
        if (p_stormctl->ucast_mode)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_unicast(p_if, p_stormctl));
        }
        break;
        
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_stormctl = (ds_storm_control_t*)p_ds;
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_if);
        if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            return HSRV_E_NONE;
        }
        
        switch(field_id)
        {
        case DS_STORM_CONTROL_FLD_BCAST_RATE:
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_broadcast(p_if, p_stormctl));
            break;

        case DS_STORM_CONTROL_FLD_MCAST_RATE:    
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_multicast(p_if, p_stormctl));
            break;

        case DS_STORM_CONTROL_FLD_UCAST_RATE: 
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_storm_control_unicast(p_if, p_stormctl));
            break;
            
        default:  
            break;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}


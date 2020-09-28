

/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
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
#include "hsrv_brgif.h"
#include "glb_tempfile_define.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"
#include "sal_mutex.h"

extern sal_mutex_t *g_hsrv_brgif_mutex;

mac_addr_t l2protocol_group_mac[GLB_L2_PROTOCOL_NUM];
mac_addr_t l2protocol_protocol_mac[GLB_L2_PROTOCOL_NUM];
char       l2protocol_protocol_name[GLB_L2_PROTOCOL_NUM][GLB_L2_PROTOCOL_NAME_SIZE];

/******************************************************************************
* Global and Declaration
******************************************************************************/


/***************************************************************************************************
 * Name         : hsrv_l2_protocol_add_l2proto_fdb_entry 
 * Purpose      : creat static blackhole mac for protocol-mac or group-mac        
 * Input        : p_l2protocol: table l2 protocol
                  is_protocol_mac: is protocol-mac or group-mac
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_add_l2proto_fdb_entry(tbl_l2_protocol_t* p_l2protocol, uint32 is_protocol_mac)
{
    int32 rc = HSRV_E_NONE;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    sai_attribute_t macfilter_attr[2];
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    
    macfilter_attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    macfilter_attr[0].value.s32 = SAI_FDB_ENTRY_MACFILTER;

    macfilter_attr[1].id = SAI_FDB_ENTRY_L2PROTOCOL_FLAG;
    if (is_protocol_mac)
    {
        macfilter_attr[1].value.s32 = SAI_FDB_L2PROTOCOL_FLAG_PROTOCOL_MAC;
        sal_memcpy(l2_addr.mac_address, p_l2protocol->protocol_mac, sizeof(mac_addr_t));
    }
    else
    {
        macfilter_attr[1].value.s32 = SAI_FDB_L2PROTOCOL_FLAG_GROUP_MAC;
        sal_memcpy(l2_addr.mac_address, p_l2protocol->group_mac, sizeof(mac_addr_t));
    }
    HSRV_IF_ERROR_RETURN(fdb_api->create_l2protocol_fdb(&l2_addr, sizeof(macfilter_attr)/sizeof(macfilter_attr[0]), macfilter_attr));
    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_del_l2pro_fdb_entry 
 * Purpose      : del static blackhole mac for protocol-mac or group-mac        
 * Input        : p_l2protocol: table l2 protocol
                  is_protocol_mac: is protocol-mac or group-mac
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_del_l2pro_fdb_entry(tbl_l2_protocol_t* p_l2protocol, uint32 is_protocol_mac)
{
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    if (is_protocol_mac)
    {
        sal_memcpy(l2_addr.mac_address, p_l2protocol->protocol_mac, sizeof(mac_addr_t));
    }
    else
    {
        sal_memcpy(l2_addr.mac_address, p_l2protocol->group_mac, sizeof(mac_addr_t));
    }
    HSRV_IF_ERROR_RETURN(fdb_api->remove_l2protocol_fdb(&l2_addr));
    
    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_add_user_defined_protocol 
 * Purpose      : add user defined protocol         
 * Input        : p_l2protocol: table l2 protocol
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_add_user_defined_protocol(tbl_l2_protocol_t* p_l2protocol)
{
    int32 rc = HSRV_E_NONE;

    if (p_l2protocol->protocol_index >= GLB_L2_PROTO_INDEX_4)
    {
        sal_memcpy(l2protocol_protocol_mac[p_l2protocol->protocol_index], p_l2protocol->protocol_mac, MAC_ADDR_LEN);
        sal_strncpy(l2protocol_protocol_name[p_l2protocol->protocol_index], p_l2protocol->key.name, GLB_L2_PROTOCOL_NAME_SIZE);
    }
    HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_add_l2proto_fdb_entry(p_l2protocol, TRUE));
    
    return rc;   
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_add_standard_protocol 
 * Purpose      : add standard protocol      
 * Input        : p_l2protocol: table l2 protocol
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_add_standard_protocol(tbl_l2_protocol_t* p_l2protocol)
{
    int32 rc = HSRV_E_NONE;
    
    if (p_l2protocol->protocol_index < GLB_L2_PROTO_INDEX_4)
    {
        sal_memcpy(l2protocol_protocol_mac[p_l2protocol->protocol_index], p_l2protocol->protocol_mac, MAC_ADDR_LEN);
        sal_strncpy(l2protocol_protocol_name[p_l2protocol->protocol_index], p_l2protocol->key.name, GLB_L2_PROTOCOL_NAME_SIZE);
    }
    
    return rc;    
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_del_user_defined_protocol 
 * Purpose      : del user defined protocol      
 * Input        : p_l2protocol: table l2 protocol
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_del_user_defined_protocol(tbl_l2_protocol_t* p_db_l2protocol)
{
    int32 rc = HSRV_E_NONE;

    if (p_db_l2protocol->protocol_index >= GLB_L2_PROTO_INDEX_4)
    {
        sal_memset(l2protocol_protocol_mac[p_db_l2protocol->protocol_index], 0, MAC_ADDR_LEN);
        sal_memset(l2protocol_group_mac[p_db_l2protocol->protocol_index], 0, MAC_ADDR_LEN);
        sal_memset(l2protocol_protocol_name[p_db_l2protocol->protocol_index], 0, GLB_L2_PROTOCOL_NAME_SIZE);
    }
    if (!GLB_IS_ZERO_MAC(p_db_l2protocol->group_mac))
    {
        HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_del_l2pro_fdb_entry(p_db_l2protocol, FALSE));
    }
    HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_del_l2pro_fdb_entry(p_db_l2protocol, TRUE));
    
    return rc; 
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_set_standard_protocol_group_mac 
 * Purpose      : set standard protocol group_mac      
 * Input        : p_db_l2protocol: table l2 protocol
                  p_l2protocol: table l2 protocol
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_set_standard_protocol_group_mac(tbl_l2_protocol_t* p_db_l2protocol, tbl_l2_protocol_t* p_l2protocol)
{
    int32 rc = HSRV_E_NONE;

    if (p_l2protocol->protocol_index < GLB_L2_PROTO_INDEX_4)
    {
        sal_memcpy(l2protocol_group_mac[p_l2protocol->protocol_index], p_l2protocol->group_mac, MAC_ADDR_LEN);
    }
    if (!GLB_IS_ZERO_MAC(p_db_l2protocol->group_mac))
    {
        HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_del_l2pro_fdb_entry(p_db_l2protocol, FALSE));
    }
    if (!GLB_IS_ZERO_MAC(p_l2protocol->group_mac))
    {
        HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_add_l2proto_fdb_entry(p_l2protocol, FALSE));
    }
    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_l2_protocol_set_user_defined_protocol_group_mac 
 * Purpose      : set user defined protocol group_mac      
 * Input        : p_db_l2protocol: table l2 protocol
                  p_l2protocol: table l2 protocol
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_l2_protocol_set_user_defined_protocol_group_mac(tbl_l2_protocol_t* p_db_l2protocol, tbl_l2_protocol_t* p_l2protocol)
{
    int32 rc = HSRV_E_NONE;

    if (p_l2protocol->protocol_index >= GLB_L2_PROTO_INDEX_4)
    {
        sal_memcpy(l2protocol_group_mac[p_l2protocol->protocol_index], p_l2protocol->group_mac, MAC_ADDR_LEN);
    }
    if (!GLB_IS_ZERO_MAC(p_db_l2protocol->group_mac))
    {
        HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_del_l2pro_fdb_entry(p_db_l2protocol, FALSE));
    }
    if (!GLB_IS_ZERO_MAC(p_l2protocol->group_mac))
    {
        HSRV_IF_ERROR_RETURN(hsrv_l2_protocol_add_l2proto_fdb_entry(p_l2protocol, FALSE));
    }
    
    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_l2protocol_sync
 * Purpose      : load l2protocol cdb to sai          
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
hsrv_l2protocol_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_l2_protocol_t* p_l2protocol = NULL;
    tbl_l2_protocol_t* p_db_l2protocol = NULL;
    tbl_l2_protocol_key_t *l2protocol_key = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    { 
    case CDB_OPER_ADD:
        break;
        
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }   
        l2protocol_key = (tbl_l2_protocol_key_t*)p_tbl;
        p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(l2protocol_key);
        if (NULL == p_db_l2protocol)
        {
            return HSRV_E_NONE;
        }
        if (!p_db_l2protocol->is_standard_pro)
        {
            hsrv_l2_protocol_del_user_defined_protocol(p_db_l2protocol);
        }
        break;
    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }   
        p_l2protocol = (tbl_l2_protocol_t*)p_tbl;
        l2protocol_key = (tbl_l2_protocol_key_t*)p_tbl;
        p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(l2protocol_key);
        if (NULL == p_db_l2protocol)
        {
            return HSRV_E_NONE;
        }
        if (p_db_l2protocol->is_standard_pro)
        {
            hsrv_l2_protocol_set_standard_protocol_group_mac(p_db_l2protocol, p_l2protocol);    
        }
        else
        {
            hsrv_l2_protocol_set_user_defined_protocol_group_mac(p_db_l2protocol, p_l2protocol);
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_l2protocol_sync_after
 * Purpose      : load l2protocol cdb to sai          
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
hsrv_l2protocol_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_l2_protocol_t* p_l2protocol = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    switch (oper)
    { 
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }   
        p_l2protocol = (tbl_l2_protocol_t*)p_tbl;
        if (NULL == p_l2protocol)
        {
            return HSRV_E_NONE;
        }
        if (p_l2protocol->is_standard_pro)
        {
            hsrv_l2_protocol_add_standard_protocol(p_l2protocol);
        }
        else
        {
            hsrv_l2_protocol_add_user_defined_protocol(p_l2protocol);    
        }     
        break;
        
    case CDB_OPER_DEL:
        break;
        
    default:
        break;
    }

    return HSRV_E_NONE;
}

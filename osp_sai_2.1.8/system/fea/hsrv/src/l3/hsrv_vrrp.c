/****************************************************************************
* $Id$
*  Centec vrrp information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2017-09-27
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_if.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "hsrv_msg.h"
#include "hsrv_message.h"
#include "sai.h"

/******************************************************************************
* Defines and Macros
******************************************************************************/

/******************************************************************************
* Global and Declaration
******************************************************************************/

/***************************************************************************************************
 * Name         : hsrv_vrrp_create_vrrp_vip_entry
 * Purpose      : creat vrrp vip entry
 * Input        : tbl_vrrp_vip
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vrrp_create_vrrp_vip_entry(tbl_vrrp_vip_t *p_vrrp_vip)
{
    sai_vrrp_api_t* vrrp_api = NULL;
    sai_vrrp_vip_entry_t vrrp_vip;
    sai_attribute_t attr[2];

    sal_memset(attr, 0, sizeof(attr));

    HSRV_PTR_CHECK(p_vrrp_vip);
    sal_memset(&vrrp_vip, 0, sizeof(sai_vrrp_vip_entry_t));

    /* process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VRRP,(void**)&vrrp_api));

    /* set vmac */
    attr[0].id = SAI_VRRP_ATTR_VMAC_ADDRESS;
    sal_memcpy(attr[0].value.mac, p_vrrp_vip->vmac, sizeof(attr[0].value.mac)); 

    /* set inteface mac */
    attr[1].id = SAI_VRRP_ATTR_IF_MAC_ADDRESS;
    sal_memcpy(attr[1].value.mac, p_vrrp_vip->interface_mac, sizeof(attr[0].value.mac)); 

    vrrp_vip.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    vrrp_vip.ip_address.addr.ip4 = p_vrrp_vip->key.u.prefix4.s_addr;
    HSRV_IF_ERROR_RETURN(vrrp_api->create_vrrp_vip_entry(&vrrp_vip, 2, attr));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vrrp_remove_vrrp_vip_entry
 * Purpose      : remove vrrp vip entry
 * Input        : tbl_vrrp_vip;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vrrp_remove_vrrp_vip_entry(tbl_vrrp_vip_t *p_vrrp_vip)
{
    sai_vrrp_api_t* vrrp_api = NULL;
    sai_vrrp_vip_entry_t vrrp_vip;
    sai_attribute_t attr[2];

    sal_memset(attr, 0, sizeof(attr));

    HSRV_PTR_CHECK(p_vrrp_vip);
    sal_memset(&vrrp_vip, 0, sizeof(sai_vrrp_vip_entry_t));

    /* process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VRRP,(void**)&vrrp_api));

    vrrp_vip.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    vrrp_vip.ip_address.addr.ip4 = p_vrrp_vip->key.u.prefix4.s_addr;
    HSRV_IF_ERROR_RETURN(vrrp_api->remove_vrrp_vip_entry(&vrrp_vip));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vrrp_sync 
 * Purpose      : load vrrp vip cdb to sai          
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
hsrv_vrrp_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_vrrp_vip_t      *p_vrrp_vip =NULL;
    
    /*1. debug out infor*/
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vrrp_vip = (tbl_vrrp_vip_t*)p_tbl;
        HSRV_PTR_CHECK(p_vrrp_vip);
        HSRV_IF_ERROR_RETURN(hsrv_vrrp_create_vrrp_vip_entry(p_vrrp_vip));
        break;

    case CDB_OPER_DEL:
        p_vrrp_vip = (tbl_vrrp_vip_t*)p_tbl;
        HSRV_PTR_CHECK(p_vrrp_vip);
        HSRV_IF_ERROR_RETURN(hsrv_vrrp_remove_vrrp_vip_entry(p_vrrp_vip));
        break;

    case CDB_OPER_SET:
        return HSRV_E_NONE;
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}




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
#include "saivlanclass.h"



/******************************************************************************
* Global and Declaration
******************************************************************************/

int32 
hsrv_vlanclass_get_protocol_type(uint32 protocol_type)
{
    if (VLANCLASS_PROTYPE_IPV4 == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV4;
    }
    else if (VLANCLASS_PROTYPE_IPV6 == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV6;
    }
    else if (VLANCLASS_PROTYPE_MPLS == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS;
    }
    else if (VLANCLASS_PROTYPE_MPLSMCAST == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS_MCAST;
    }   
    else if (VLANCLASS_PROTYPE_IPV4ARP == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_ARP;
    }
    else if (VLANCLASS_PROTYPE_IPV4RARP == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_RARP;
    }
    else if (VLANCLASS_PROTYPE_PPPOE == protocol_type)
    {
        return SAI_VLAN_CLASS_PROTOCOL_TYPE_PPPOE;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_entry_add
 * Purpose      : add rule to group
 * Input        : p_vclass_group: table vlanclass group; 
                  p_db_vclass_rule: table vlanclass rule
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlanclass_entry_add(tbl_vlanclass_group_t* p_vclass_group, tbl_vlanclass_rule_t* p_db_vclass_rule)
{
    sai_vlanclass_api_t*  vlanclass_api = NULL;
    sai_object_id_t   sai_rule_oid = 0; 
    addr_t            ip_address;
    sai_attribute_t   attr[5];

    sal_memset(&ip_address, 0, sizeof(addr_t));
    
    if ((NULL == p_vclass_group) || (NULL == p_db_vclass_rule))
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN_CLASS,(void**)&vlanclass_api));

    attr[0].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID;
    attr[0].value.oid = p_vclass_group->group_oid;
    attr[1].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID;
    attr[1].value.s32 = p_db_vclass_rule->key.rule_id;
    attr[2].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID;
    attr[2].value.s16 = p_db_vclass_rule->vlan_id;
    
    switch (p_db_vclass_rule->rule_type)
    {
     case GLB_VLAN_CLASSIFIER_MAC:
        attr[3].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE;
        attr[3].value.s32 = SAI_VLAN_CLASS_ENTRY_TYPE_MAC;
        attr[4].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS;
        sal_memcpy(attr[4].value.mac, p_db_vclass_rule->mac, sizeof(mac_addr_t));
        break;
     case GLB_VLAN_CLASSIFIER_IPV4:
        attr[3].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE;
        attr[3].value.s32 = SAI_VLAN_CLASS_ENTRY_TYPE_IPV4;
        attr[4].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS;
        ip_address.u.prefix4.s_addr = sal_htonl(p_db_vclass_rule->ip_address.u.prefix4.s_addr);
        sal_memcpy(&attr[4].value.ipaddr.addr.ip4, &ip_address.u.prefix4, sizeof(sai_ip4_t));
        break;
     case GLB_VLAN_CLASSIFIER_IPV6:
        attr[3].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE;
        attr[3].value.s32 = SAI_VLAN_CLASS_ENTRY_TYPE_IPV6;
        attr[4].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS;
        sal_memcpy(&attr[4].value.ipaddr.addr.ip6, &p_db_vclass_rule->ip_address.u.prefix6, sizeof(sai_ip6_t));
        break;
     case GLB_VLAN_CLASSIFIER_PROTOCOL:
        attr[3].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE;
        attr[3].value.s32 = SAI_VLAN_CLASS_ENTRY_TYPE_PROTOCOL;
        attr[4].id = SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE;
        attr[4].value.s32 = hsrv_vlanclass_get_protocol_type(p_db_vclass_rule->protocol_type);
        break;
        
     default:
        break;
    }
    HSRV_IF_ERROR_RETURN(vlanclass_api->create_vlanclass_group_member(&sai_rule_oid, 5, attr));

    /*set the vlanclass rule table field: rule_oid*/
    p_db_vclass_rule->rule_oid = sai_rule_oid;
    HSRV_IF_ERROR_RETURN(tbl_vlanclass_rule_set_vlanclass_rule_field(p_db_vclass_rule, TBL_VLANCLASS_RULE_FLD_RULE_OID));
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_entry_del
 * Purpose      : delete rule from group
 * Input        : p_vclass_group: table vlanclass group; 
                  p_db_vclass_rule: table vlanclass rule
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlanclass_entry_del(tbl_vlanclass_group_t* p_vclass_group, tbl_vlanclass_rule_t* p_db_vclass_rule)
{ 
    sai_vlanclass_api_t*  vlanclass_api = NULL;
    sai_object_id_t   sai_rule_oid = 0; 

    if (NULL == p_db_vclass_rule)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN_CLASS,(void**)&vlanclass_api));
    sai_rule_oid = p_db_vclass_rule->rule_oid;
    if (0 == sai_rule_oid)
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(vlanclass_api->delete_vlanclass_group_member(sai_rule_oid));
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_entry_process
 * Purpose      : If add rule to group, hsrv_vlanclass_entry_add
                  if delete rule from group, hsrv_vlanclass_entry_del
 * Input        : p_vclass_group: table vlanclass group
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlanclass_entry_process(tbl_vlanclass_group_t* p_vclass_group)
{
    tbl_vlanclass_rule_t* p_db_vclass_rule = NULL;
    tbl_vlanclass_rule_key_t p_vclass_rkey;

    if (NULL == p_vclass_group)
    {   
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    sal_memset(&p_vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
    if ((p_vclass_group->action_rule_id >> 32) == GLB_VLAN_CLASS_RULE_ACTION_ADD)
    {
        /*add entry to group*/
        p_vclass_rkey.rule_id = p_vclass_group->action_rule_id&0xffffffff;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&p_vclass_rkey);
        HSRV_PTR_CHECK(p_db_vclass_rule);
        hsrv_vlanclass_entry_add(p_vclass_group, p_db_vclass_rule);
    }
    else if ((p_vclass_group->action_rule_id >> 32) == GLB_VLAN_CLASS_RULE_ACTION_DEL)
    {
        /*delete entry from group*/
        p_vclass_rkey.rule_id = p_vclass_group->action_rule_id&0xffffffff;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&p_vclass_rkey);
        HSRV_PTR_CHECK(p_db_vclass_rule);
        hsrv_vlanclass_entry_del(p_vclass_group, p_db_vclass_rule);
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_group_add
 * Purpose      : creat vlanclass group object ID          
 * Input        : p_vclass_group: table vlanclass group
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlanclass_group_add(tbl_vlanclass_group_t* p_vclass_group)
{
    sai_vlanclass_api_t*  vlanclass_api = NULL;
    sai_object_id_t sai_group_oid = 0;

    if (NULL == p_vclass_group)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN_CLASS,(void**)&vlanclass_api));
    HSRV_IF_ERROR_RETURN(vlanclass_api->create_vlanclass_group(&sai_group_oid, 0, NULL));
    
    p_vclass_group->group_oid = sai_group_oid;
    HSRV_IF_ERROR_RETURN(tbl_vlanclass_group_set_vlanclass_group_field(p_vclass_group, TBL_VLANCLASS_GROUP_FLD_GROUP_OID));  
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_group_del
 * Purpose      : delete vlanclass group object ID          
 * Input        : p_vclass_group: table vlanclass group
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlanclass_group_del(tbl_vlanclass_group_t* p_vclass_group)
{
    sai_vlanclass_api_t*  vlanclass_api = NULL;
    sai_object_id_t   sai_group_oid = 0;

    if (NULL == p_vclass_group)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN_CLASS,(void**)&vlanclass_api));
    sai_group_oid = p_vclass_group->group_oid;
    if (0 == sai_group_oid)
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(vlanclass_api->delete_vlanclass_group(sai_group_oid));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_sync
 * Purpose      : load vlanclass cdb to sai          
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
hsrv_vlanclass_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlanclass_group_t* p_vclass_group = NULL;
    tbl_vlanclass_group_key_t *vclass_gkey = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        vclass_gkey = (tbl_vlanclass_group_key_t*)p_tbl;
        p_vclass_group = tbl_vlanclass_group_get_vlanclass_group(vclass_gkey);
        HSRV_PTR_CHECK(p_vclass_group);
        HSRV_IF_ERROR_RETURN(hsrv_vlanclass_group_del(p_vclass_group));
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanclass_sync_after
 * Purpose      : load vlanclass cdb to sai          
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
hsrv_vlanclass_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlanclass_group_t* p_vclass_group = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vclass_group = (tbl_vlanclass_group_t*)p_tbl;
        p_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&p_vclass_group->key);
        HSRV_PTR_CHECK(p_vclass_group);
        HSRV_IF_ERROR_RETURN(hsrv_vlanclass_group_add(p_vclass_group));
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_vclass_group = (tbl_vlanclass_group_t*)p_tbl;
        p_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&p_vclass_group->key);
        HSRV_PTR_CHECK(p_vclass_group);
        switch (field_id)
        {
            case TBL_VLANCLASS_GROUP_FLD_RULE_ID:
                break;
            case TBL_VLANCLASS_GROUP_FLD_ACTION_RULE_ID:
                HSRV_IF_ERROR_RETURN(hsrv_vlanclass_entry_process(p_vclass_group));
                break;
            default:
                break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}



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
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "ctc_sai_port.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"

extern int32
hsrv_fdb_set_learning_en(uint32 enable);

extern int32
hsrv_fdb_reset_learning_timer(void);

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

/* modified by liwh for bug 37200, 2016-08-24 */
int32
hsrv_fdb_set_all_flushing(bool set)
{
    tbl_brg_global_t  *p_brg_glb = NULL;
    
    /* 1. process */
    p_brg_glb = tbl_brg_global_get_brg_global();

    if (set)
    {
        p_brg_glb->fdb_all_flushing = TRUE;
    }
    else if (p_brg_glb->fdb_all_flushing)
    {
        p_brg_glb->fdb_all_flushing = FALSE;
    }
 
    return PM_E_NONE;
}
/* liwh end */

/* added by liwh for bug 37200, 2016-08-24 
       for erps performance, should flush hardware fdb first */   
/***************************************************************************************************
 * Name         : _hsrv_fdb_flush_fdb_all
 * Purpose      : clear all dynamic fdb entry 
 * Input        : N/A
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_erps()
{  
    sai_fdb_api_t*  fdb_api = NULL;
    sai_attribute_t   attr[1];
    /* modified by liwh for bug 53553, 2019-10-23 */
    int32                 rc = HSRV_E_NONE;
    /* liwh end */
    
    attr[0].id = SAI_FDB_FLUSH_ATTR_HARDWARE;
    attr[0].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));     

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();
    /* liwh end */
    
    hsrv_fdb_set_all_flushing(TRUE);
    hsrv_fdb_set_learning_en(FALSE);

    /* modified by liwh for bug 53553, 2019-10-23 */
    //HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);
    }

    hsrv_fdb_unlock();
    /* liwh end */

    hsrv_fdb_reset_learning_timer();
 
    return HSRV_E_NONE;
}
/* liwh end */

/***************************************************************************************************
 * Name         : _hsrv_fdb_flush_fdb_all
 * Purpose      : clear all dynamic fdb entry 
 * Input        : N/A
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_all()
{  
    sai_fdb_api_t*  fdb_api = NULL;
    sai_attribute_t   attr[1];
    /* modified by liwh for bug 53553, 2019-10-23 */
    int32                 rc = HSRV_E_NONE;
    /* liwh end */
    
    attr[0].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
    attr[0].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api)); 

    /*YEJL:
    The order is: flush software table first, then flush the hardware table, 
    it is not allowed to modify the order, otherwise it will result in pending entry*/

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();
    /* liwh end */

    /* modified by liwh for bug 53553, 2019-10-23 */
    //HSRV_IF_ERROR_RETURN(hsrv_fdb_db_flush_all());
    //HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));
    rc = hsrv_fdb_db_flush_all();
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_db_flush_all failure \n", __func__, __LINE__);
    }
    
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);
    }

    hsrv_fdb_unlock();

    hsrv_fdb_mlag_tx_flush_all();
    /* liwh end */
    
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_fdb_flush_fdb_vlan
 * Purpose      : clear dynamic fdb entry by vlan
 * Input        : vid: VLAN_ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_vlan(vid_t vid)
{   
    sai_fdb_api_t*  fdb_api = NULL;
    sai_attribute_t   attr[2];
    /* modified by liwh for bug 53553, 2019-10-23 */
    int32                 rc = HSRV_E_NONE;
    uint32               has_entry = FALSE;
    /* liwh end */

    if (vid > GLB_VLAN_MAX)
    {
        return HSRV_E_INVALID_PARAM;
    }

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api)); 

    attr[0].id = SAI_FDB_FLUSH_ATTR_VLAN_ID;
    attr[0].value.u16 = vid;
    attr[1].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
    attr[1].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();
    /* liwh end */

    /*YEJL:
    The order is: flush software table first, then flush the hardware table, 
    it is not allowed to modify the order, otherwise it will result in pending entry*/

    /* modified by liwh for bug 53553, 2019-10-23 */
    //HSRV_IF_ERROR_RETURN(hsrv_fdb_db_flush_vlan(vid));
    //HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));    
    rc = hsrv_fdb_db_flush_vlan(vid, &has_entry);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_db_flush_vlan failure \n", __func__, __LINE__);    
    }
    
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);    
    }

    hsrv_fdb_unlock();

    if (has_entry)
    {
         hsrv_fdb_mlag_tx_flush_vlan(vid);   
    }
    /* liwh end */
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_fdb_flush_fdb_port
 * Purpose      : clear dynamic fdb entry by port
 * Input        : ifindex: PORT_ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
__hsrv_fdb_flush_fdb_port(uint32 ifindex, uint32 mlag_sync)
{
    sai_fdb_api_t*    fdb_api = NULL;
    sai_object_id_t   port_oid = 0;
    sai_attribute_t   attr[2];
    int32 rc = 0;
    /* modified by liwh for bug 53553, 2019-10-23 */
    uint32 has_entry = FALSE;
    /* liwh end */

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);     
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    rc = hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
    if (rc < 0)
    {
        return HSRV_E_NONE;
    }
    
    attr[0].id = SAI_FDB_FLUSH_ATTR_PORT_ID;
    attr[0].value.oid = port_oid;
    attr[1].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
    attr[1].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();

    /*YEJL:
    The order is: flush software table first, then flush the hardware table, 
    it is not allowed to modify the order, otherwise it will result in pending entry*/
    // HSRV_IF_ERROR_RETURN(hsrv_fdb_mlag_sync_flush_peer_link(ifindex));
    // HSRV_IF_ERROR_RETURN(hsrv_fdb_db_flush_port(ifindex, mlag_sync));
    // HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));
    rc = hsrv_fdb_mlag_sync_flush_peer_link(ifindex);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_mlag_sync_flush_peer_link failure \n", __func__, __LINE__);        
    }  
	
    rc = hsrv_fdb_db_flush_port(ifindex, mlag_sync, &has_entry);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_db_flush_port failure \n", __func__, __LINE__);        
    }
	
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);         
    }

    hsrv_fdb_unlock();

    if (has_entry && mlag_sync)
    {
        hsrv_fdb_mlag_tx_flush_port(ifindex);
    }
    /* liwh end */

    return HSRV_E_NONE;
}

int32
hsrv_fdb_flush_fdb_port(uint32 ifindex)
{
	return __hsrv_fdb_flush_fdb_port(ifindex, TRUE);
}

int32
hsrv_fdb_flush_fdb_port_without_mlag_sync(uint32 ifindex)
{
	return __hsrv_fdb_flush_fdb_port(ifindex, FALSE);
}

/***************************************************************************************************
 * Name         : hsrv_fdb_flush_fdb_port_vlan
 * Purpose      : clear dynamic fdb entry 
 * Input        : ifindex: PORT_ID; vid:VLAN_ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_port_vlan(uint32 ifindex, vid_t vid)
{
    sai_fdb_api_t*    fdb_api = NULL;
    sai_object_id_t   port_oid = 0;
    sai_attribute_t   attr[3];
    /* modified by liwh for bug 53553, 2019-10-23 */
    int32                 rc = HSRV_E_NONE;
    uint32               has_entry = FALSE;
    /* liwh end */

    if (vid > GLB_VLAN_MAX)
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
    
    attr[0].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
    attr[0].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;
    attr[1].id = SAI_FDB_FLUSH_ATTR_PORT_ID;
    attr[1].value.oid = port_oid;
    attr[2].id = SAI_FDB_FLUSH_ATTR_VLAN_ID;
    attr[2].value.u16 = vid;

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();

    /*YEJL:
    The order is: flush software table first, then flush the hardware table, 
    it is not allowed to modify the order, otherwise it will result in pending entry*/
    //HSRV_IF_ERROR_RETURN(hsrv_fdb_db_flush_port_vlan(ifindex, vid));
    //HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));   
    rc = hsrv_fdb_db_flush_port_vlan(ifindex, vid, &has_entry);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_db_flush_port_vlan failure \n", __func__, __LINE__);            
    }
    
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);
    }

    hsrv_fdb_unlock();

    if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_port_vlan(ifindex, vid);
    }
    /* liwh end */
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_fdb_flush_fdb_mac
 * Purpose      : clear dynamic fdb entry by mac
 * Input        : mac: MAC address
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_mac(mac_addr_t mac)
{
    sai_fdb_api_t*    fdb_api = NULL;
    sai_attribute_t   attr[2];
    /* modified by liwh for bug 53553, 2019-10-23 */
    int32                 rc = HSRV_E_NONE;
    uint32               has_entry = FALSE;
    /* liwh end */

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);     
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    attr[0].id = SAI_FDB_FLUSH_ATTR_MAC;
    sal_memcpy(attr[0].value.mac, mac, sizeof(mac_addr_t));
    attr[1].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
    attr[1].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;

    /* modified by liwh for bug 53553, 2019-10-23 */
    hsrv_fdb_lock();
    
    /*YEJL:
    The order is: flush software table first, then flush the hardware table, 
    it is not allowed to modify the order, otherwise it will result in pending entry*/
    // HSRV_IF_ERROR_RETURN(hsrv_fdb_db_flush_mac(mac));
    // HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));
    rc = hsrv_fdb_db_flush_mac(mac, &has_entry);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call hsrv_fdb_db_flush_mac failure \n", __func__, __LINE__);
    }
    
    rc = fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr);
    if (rc)
    {
        HSRV_LOG_ERR("%s %d call fdb_api->flush_fdb_entries failure \n", __func__, __LINE__);   
    }

    hsrv_fdb_unlock();

    if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_mac(mac);
    }
    /* liwh end */
    
    return HSRV_E_NONE;
}

/* added by yejl for delete pending entry for DUET2, 2017-08-25 */
/***************************************************************************************************
 * Name         : hsrv_fdb_flush_fdb_pending
 * Purpose      : clear all dynamic fdb entry 
 * Input        : N/A
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_flush_fdb_pending(vid_t vid, mac_addr_t mac)
{  
    sai_fdb_api_t*  fdb_api = NULL;
    sai_attribute_t   attr[3];
   
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));     

    attr[0].id = SAI_FDB_FLUSH_ATTR_PENDING;
    attr[0].value.s32 = SAI_FDB_FLUSH_ENTRY_DYNAMIC;

    attr[1].id = SAI_FDB_FLUSH_ATTR_VLAN_ID;
    attr[1].value.u16 = vid;

    attr[2].id = SAI_FDB_FLUSH_ATTR_MAC;
    sal_memcpy(attr[2].value.mac, mac, sizeof(mac_addr_t));
    
    HSRV_IF_ERROR_RETURN(fdb_api->flush_fdb_entries(sizeof(attr)/sizeof(attr[0]), attr));
 
    return HSRV_E_NONE;
}
/* yejl end */

/***************************************************************************************************
 * Name         : hsrv_l2_action_flush_fdb_sync 
 * Purpose      : load l2 action flush fdb cdb to sai          
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
hsrv_l2_action_flush_fdb_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    ds_flush_fdb_t* p_flush_fdb = NULL;

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;
    case CDB_OPER_SET:
        p_flush_fdb = (ds_flush_fdb_t*)p_ds;
        HSRV_PTR_CHECK(p_flush_fdb);
        switch(field_id)
        {
        case DS_FLUSH_FDB_FLD_MAX:
            if (FLUSH_FDB_TYPE_FDB == p_flush_fdb->type)
            {
                if (FLUSH_FDB_SUBTYPE_DYNAMIC == p_flush_fdb->subtype)
                {
                   if (FLUSH_FDB_MODE_ALL == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_all());
                   }
                   else if (FLUSH_FDB_MODE_PORT == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_flush_fdb->ifindex));
                   }
                   else if (FLUSH_FDB_MODE_VLAN == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_vlan(p_flush_fdb->vid));
                   }
                   else if (FLUSH_FDB_MODE_PORT_VLAN == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port_vlan(p_flush_fdb->ifindex,p_flush_fdb->vid));
                   }
                   else if (FLUSH_FDB_MODE_MAC == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_mac(p_flush_fdb->mac));
                   }
                   /* modified by liwh for bug 37200, 2016-08-25 */
                   else if (FLUSH_FDB_MODE_ERPS == p_flush_fdb->mode)
                   {
                       HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_erps());
                   }
                   /* liwh end */
                }
                else
                {
                   /*TODO*/
                }
            }
            else
            {
               /*TODO*/
            }
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

/***************************************************************************************************
 * Name         : hsrv_l2_action_sync 
 * Purpose      : load l2 action cdb to sai          
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
hsrv_l2_action_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{ 
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    
    if(p_ds_node)
    {
        switch (p_ds_node->id)
        {
        case DS_FLUSH_FDB:
            HSRV_IF_ERROR_RETURN(hsrv_l2_action_flush_fdb_sync(p_tbl_node, p_ds_node, 
                oper, field_id, p_tbl, p_ds));
            break;
        default:
            return HSRV_E_INVALID_PARAM;  
        }
        return HSRV_E_NONE;
    }
        
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:       
    case CDB_OPER_SET:
        break;
    default:
        break;
    }
    return HSRV_E_NONE;
}


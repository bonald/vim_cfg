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
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "hsrv_fdb.h"
#include "hsrv_mcfdb.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"

int32
hsrv_mcfdb_sai_add_entry(tbl_mcfdb_t *p_mcfdb) 
{
    sai_status_t rc = 0;
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;

    sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
    
    sal_memcpy(l2mc_addr.mac_address, p_mcfdb->key.mac, sizeof(mac_addr_t));
    l2mc_addr.vlan_id = p_mcfdb->key.fid;
    
    rc = l2mc_api->create_l2mc_entry(&l2mc_addr, 0, NULL);

    return rc;
}

int32
hsrv_mcfdb_sai_del_entry(tbl_mcfdb_t *p_mcfdb) 
{
    sai_status_t rc = 0;
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;

    sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
    
    sal_memcpy(l2mc_addr.mac_address, p_mcfdb->key.mac, sizeof(mac_addr_t));
    l2mc_addr.vlan_id = p_mcfdb->key.fid;
    
    rc = l2mc_api->remove_l2mc_entry(&l2mc_addr);

    return rc;
}

int32
hsrv_mcfdb_sai_add_member(tbl_mcfdb_t *p_mcfdb, uint32 ifindex)
{
    sai_status_t rc = 0;
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;
    sai_object_id_t port_oid[1];
    sai_attribute_t attr[1] =
    {
        [0] = {
           .id = SAI_L2MC_ATTR_PORT_LIST,
        }
    };

    sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
    
    sal_memcpy(l2mc_addr.mac_address, p_mcfdb->key.mac, sizeof(mac_addr_t));
    l2mc_addr.vlan_id = p_mcfdb->key.fid;
    
    attr[0].value.objlist.count = 1;
    attr[0].value.objlist.list = port_oid;
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, port_oid));
    
    rc = l2mc_api->add_ports_to_l2mc(&l2mc_addr, 1, attr);

    return rc;
}

int32
hsrv_mcfdb_sai_del_member(tbl_mcfdb_t *p_mcfdb, uint32 ifindex)
{
    sai_status_t rc = 0;
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;
    sai_object_id_t port_oid[1];
    sai_attribute_t attr[1] =
    {
        [0] = {
           .id = SAI_L2MC_ATTR_PORT_LIST,
        }
    };

    sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
    
    sal_memcpy(l2mc_addr.mac_address, p_mcfdb->key.mac, sizeof(mac_addr_t));
    l2mc_addr.vlan_id = p_mcfdb->key.fid;
    
    attr[0].value.objlist.count = 1;
    attr[0].value.objlist.list = port_oid;
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, port_oid));
    
    rc = l2mc_api->remove_ports_from_l2mc(&l2mc_addr, 1, attr);

    return rc;
}

int32
hsrv_mcfdb_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mcfdb_t *      p_mcfdb = NULL;
    uint32             action = 0;
    uint32             ifindex = 0;
    
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_mcfdb = (tbl_mcfdb_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_FDB_DEBUG("MCFDB add entry mac = %02X%02X.%02X%02X.%02X%02X, vlan = %u",
            p_mcfdb->key.mac[0], p_mcfdb->key.mac[1], p_mcfdb->key.mac[2], p_mcfdb->key.mac[3], p_mcfdb->key.mac[4], p_mcfdb->key.mac[5],
            p_mcfdb->key.fid);
        hsrv_mcfdb_sai_add_entry(p_mcfdb);
        hsrv_igsp_mcfdb_mrouter_list_cb(p_mcfdb, TRUE);
        break;

    case CDB_OPER_DEL:
        HSRV_FDB_DEBUG("MCFDB del entry mac = %02X%02X.%02X%02X.%02X%02X, vlan = %u",
            p_mcfdb->key.mac[0], p_mcfdb->key.mac[1], p_mcfdb->key.mac[2], p_mcfdb->key.mac[3], p_mcfdb->key.mac[4], p_mcfdb->key.mac[5],
            p_mcfdb->key.fid);
        hsrv_igsp_mcfdb_mrouter_list_cb(p_mcfdb, FALSE);
        hsrv_mcfdb_sai_del_entry(p_mcfdb);
        break;

    case CDB_OPER_SET:
        if (TBL_MCFDB_FLD_ACTION_IFINDEX == field_id)
        {
            action = (p_mcfdb->action_ifindex >> 16);
            ifindex = (p_mcfdb->action_ifindex & 0xFFFF);
            HSRV_FDB_DEBUG("MCFDB %s member mac = %02X%02X.%02X%02X.%02X%02X, vlan = %u, ifindex=%u",
                (action) ? "add" : "del",
                p_mcfdb->key.mac[0], p_mcfdb->key.mac[1], p_mcfdb->key.mac[2], p_mcfdb->key.mac[3], p_mcfdb->key.mac[4], p_mcfdb->key.mac[5],
                p_mcfdb->key.fid,
                ifindex);
            if (action)
            {
                hsrv_mcfdb_sai_add_member(p_mcfdb, ifindex);
            }
            else
            {
                hsrv_mcfdb_sai_del_member(p_mcfdb, ifindex);
            }
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_mcfdb_start()
{
    return HSRV_E_NONE;
}


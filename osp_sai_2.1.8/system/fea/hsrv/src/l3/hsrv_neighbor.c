/****************************************************************************
* $Id$
*  Centec neighbor information related MACRO, ENUM, Date Structure defines file
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
#include "hsrv_if.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "hsrv_msg.h"
#include "hsrv_message.h"
#include "sai.h"
#include "ctclib_hash.h"
#include "sdb_defines.h"
#include "gen/stbl_neighbor_define.h"
#include "gen/stbl_neighbor.h"
#include "../../../../sai/src/neighbor/ctc_sai_neighbor.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "glb_openflow_define.h"

/******************************************************************************
* Defines and Macros
******************************************************************************/

/******************************************************************************
* Global and Declaration
******************************************************************************/
extern int32
sai_nei_register_fdb(SAI_NEI_CB fn);


extern int32
hsrv_tunneldb_cdb_arp_oper_add_function(tbl_arp_fib_t * p_arp_fib);

extern int32
hsrv_tunneldb_cdb_arp_oper_del_function(tbl_arp_fib_t * p_arp_fib);

/* modified by liwh for bug 48486, 2018-08-14 */
extern int32
sai_nei_register_nd(SAI_NEI_ND_CB fn);
/* liwh end */

int32
hsrv_neighbor_fdb_age_cb(sai_neighbor_entry_t* p_neighbor_entry)
{
    uint32 ip_addr = 0;
    int32 size;
    int32 op;
    int32 rc = SAI_STATUS_SUCCESS;

    sal_memset(&ip_addr, 0, sizeof(ip_addr));

    if (SAI_IP_ADDR_FAMILY_IPV4 == p_neighbor_entry->ip_address.addr_family)
    {
        ip_addr = p_neighbor_entry->ip_address.addr.ip4;
        op = ROUTED_FEI_FDB_AGE_ARP_NOTIFY;
        size = sizeof(uint32);
        rc = HSRV2ROUTED_MSG_SEND_NOTIFY(op, &ip_addr, size);
    }
    else
    {
        op = ROUTED_FEI_FDB_AGE_ND_NOTIFY;
        size = 16;
        rc = HSRV2ROUTED_MSG_SEND_NOTIFY(op, &p_neighbor_entry->ip_address.addr, size);
    }

    
    return rc;
}

int32
hsrv_neighbor_arp_update_by_dynamic_tunnel_remote_ip(uint32 remote_ip, char*direct_connection_ifname, uint32 is_add)
{
    int32 rc = SAI_STATUS_SUCCESS;
    glb_openflow_dynamic_tunnel_msg_t req;
    sal_memset(&req, 0x0, sizeof(glb_openflow_dynamic_tunnel_msg_t));

    sal_strncpy(req.ifname, direct_connection_ifname, GLB_IFNAME_SZ);
    req.ipv4.s_addr = remote_ip;
    req.ipv4.s_addr = sal_htonl(req.ipv4.s_addr);
    req.is_add = is_add;

    rc = HSRV2ROUTED_MSG_SEND_NOTIFY(ROUTED_FEI_DYNAMIC_TUNNEL_UPDATE_NOTIFY, &req, sizeof(glb_openflow_dynamic_tunnel_msg_t));

    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_neighbor_create_neighbor_entry
 * Purpose      : creat neighbor entry
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_neighbor_create_neighbor_entry(tbl_arp_fib_t* p_arp)
{
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    sai_attribute_t attr[1];

    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_arp);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    if (ARP_TYPE_DYNAMIC != p_arp->type && ARP_TYPE_STATIC != p_arp->type)
    {
        return HSRV_E_NONE;
    }

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_arp->ifname, sizeof(p_arp->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    sal_memcpy(attr[0].value.mac, p_arp->mac_addr, sizeof(attr[0].value.mac)); 
    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor.ip_address.addr.ip4 = sal_htonl(p_arp->key.ip.s_addr);
    HSRV_IF_ERROR_RETURN(neighbor_api->create_neighbor_entry(&neighbor, 1, attr));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_neighbor_remove_neighbor_entry
 * Purpose      : remove neighbor entry
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_neighbor_remove_neighbor_entry(tbl_arp_fib_t* p_arp)
{
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;

    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_arp);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    if (ARP_TYPE_DYNAMIC != p_arp->type && ARP_TYPE_STATIC != p_arp->type)
    {
        return HSRV_E_NONE;
    }

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_arp->ifname, sizeof(p_arp->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor.ip_address.addr.ip4 = sal_htonl(p_arp->key.ip.s_addr);
    HSRV_IF_ERROR_RETURN(neighbor_api->remove_neighbor_entry(&neighbor));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_neighbor_check_neighbor_exist
 * Purpose      : check neighbor entry exist or not
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_neighbor_check_neighbor_exist(tbl_arp_fib_t* p_arp)
{
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    sai_attribute_t attr[1];
    int32 rc = HSRV_E_NONE;
    
    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_arp);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    if (ARP_TYPE_DYNAMIC != p_arp->type && ARP_TYPE_STATIC != p_arp->type)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_arp->ifname, sizeof(p_arp->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    sal_memcpy(attr[0].value.mac, p_arp->mac_addr, sizeof(attr[0].value.mac)); 
    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor.ip_address.addr.ip4 = sal_htonl(p_arp->key.ip.s_addr);
    
    rc = neighbor_api->get_neighbor_attribute(&neighbor, 1, attr);

    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_neighbor_sync 
 * Purpose      : load neighbor cdb to sai          
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
hsrv_neighbor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_arp_fib_t      *p_arp_fib =NULL;
    tbl_arp_fib_t      *p_db_arp_fib = NULL;
    tbl_arp_fib_key_t  *p_key_arp = NULL;
    int32 rc = HSRV_E_NONE;

    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arp_fib = (tbl_arp_fib_t*)p_tbl;
        HSRV_PTR_CHECK(p_arp_fib);
        rc = hsrv_neighbor_check_neighbor_exist(p_arp_fib);
        if (SAI_STATUS_ITEM_NOT_FOUND != rc)
        {
            HSRV_IF_ERROR_RETURN(hsrv_neighbor_remove_neighbor_entry(p_arp_fib));
        }
        
        HSRV_IF_ERROR_RETURN(hsrv_neighbor_create_neighbor_entry(p_arp_fib));
        break;

    case CDB_OPER_DEL:
        p_key_arp = (tbl_arp_fib_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_key_arp);
        p_db_arp_fib = tbl_arp_fib_get_arp_fib(p_key_arp);
        HSRV_PTR_CHECK(p_db_arp_fib);
        rc = hsrv_neighbor_check_neighbor_exist(p_db_arp_fib);
        if (HSRV_E_NONE == rc)
        {
            HSRV_IF_ERROR_RETURN(hsrv_neighbor_remove_neighbor_entry(p_db_arp_fib));
        }
        else
        {
            return rc;
        }

        break;

    case CDB_OPER_SET:
        return HSRV_E_NONE;
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_neighbor_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    #ifndef OFPRODUCT
    return 0;
    #endif
    tbl_arp_fib_t      *p_arp_fib =NULL;
    tbl_arp_fib_key_t  *p_key_arp = NULL;
    int32 rc = HSRV_E_NONE;
    tbl_arp_fib_t arp_fib_del;

    if(CDB_OPER_ADD == oper)
    {
        p_arp_fib = (tbl_arp_fib_t*)p_tbl;
        HSRV_PTR_CHECK(p_arp_fib);
        HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_arp_oper_add_function(p_arp_fib));
    }else if(CDB_OPER_DEL == oper)
    {
        p_key_arp = (tbl_arp_fib_key_t*)p_tbl;
        sal_memset(&arp_fib_del, 0x0, sizeof(tbl_arp_fib_t));
        sal_memcpy(&arp_fib_del.key, p_key_arp, sizeof(tbl_arp_fib_key_t));
        HSRV_PTR_CHECK(p_key_arp);
        HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_arp_oper_del_function(&arp_fib_del));
    }

    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_ipv6_neighbor_check_neighbor_exist
 * Purpose      : check neighbor entry exist or not
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_ipv6_neighbor_check_neighbor_exist(tbl_nd_fib_t* p_nd)
{
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    sai_attribute_t attr[1];
    int32 rc = HSRV_E_NONE;
    
    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_nd);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_nd->ifname, sizeof(p_nd->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    sal_memcpy(attr[0].value.mac, p_nd->mac_addr, sizeof(attr[0].value.mac)); 
    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    sal_memcpy(&neighbor.ip_address.addr.ip6,  p_nd->key.addr.u.val, GLB_IPV6_MAX_BYTELEN);
    
    rc = neighbor_api->get_neighbor_attribute(&neighbor, 1, attr);

    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_ipv6_neighbor_create_neighbor_entry
 * Purpose      : creat neighbor entry
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_ipv6_neighbor_create_neighbor_entry(tbl_nd_fib_t* p_nd)
{
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    sai_attribute_t attr[1];

    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_nd);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_nd->ifname, sizeof(p_nd->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    sal_memcpy(attr[0].value.mac, p_nd->mac_addr, sizeof(attr[0].value.mac)); 
    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    sal_memcpy(&neighbor.ip_address.addr.ip6,  p_nd->key.addr.u.val, GLB_IPV6_MAX_BYTELEN);
    HSRV_IF_ERROR_RETURN(neighbor_api->create_neighbor_entry(&neighbor, 1, attr));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_ipv6_neighbor_remove_neighbor_entry
 * Purpose      : remove neighbor entry
 * Input        : p_arp, table arp;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_ipv6_neighbor_remove_neighbor_entry(tbl_nd_fib_t* p_nd)
{
    sai_neighbor_api_t* neighbor_api = NULL;
    sai_neighbor_entry_t neighbor;
    tbl_rif_t rif;
    tbl_rif_t* p_rif = NULL;

    /*1. debug out infor*/
    HSRV_NEIGHBOR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_nd);
    sal_memset(&neighbor, 0, sizeof(neighbor));

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR,(void**)&neighbor_api));

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_nd->ifname, sizeof(p_nd->ifname));
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    neighbor.rif_id = p_rif->rif_id;
    neighbor.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    sal_memcpy(&neighbor.ip_address.addr.ip6,  p_nd->key.addr.u.val, GLB_IPV6_MAX_BYTELEN);
    HSRV_IF_ERROR_RETURN(neighbor_api->remove_neighbor_entry(&neighbor));
    
    return HSRV_E_NONE;
}

/* support nd modified by liwh for bug 47547, 2018-07-06 */
/***************************************************************************************************
 * Name         : hsrv_ipv6_neighbor_sync 
 * Purpose      : load neighbor cdb to sai          
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
hsrv_ipv6_neighbor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_nd_fib_t        *p_nd_fib =NULL;
    tbl_nd_fib_t        *p_db_nd_fib =NULL;
    tbl_nd_fib_key_t  *p_key_nd = NULL;
    int32                  rc = HSRV_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        p_nd_fib = (tbl_nd_fib_t*)p_tbl;

        HSRV_PTR_CHECK(p_nd_fib);

        if (TBL_ND_FIB_FLD_ACTIVE == field_id)
        {
            p_db_nd_fib = tbl_nd_fib_get_nd_fib(&p_nd_fib->key);
            HSRV_PTR_CHECK(p_db_nd_fib);
    
            if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE)
                && !GLB_FLAG_ISSET(p_db_nd_fib->flags, GLB_ND_FLAG_ACTIVE))
            {
                if (0 != p_nd_fib->ifindex)
                {
                    rc = hsrv_ipv6_neighbor_check_neighbor_exist(p_nd_fib);
                    if (SAI_STATUS_ITEM_NOT_FOUND != rc)
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_ipv6_neighbor_remove_neighbor_entry(p_nd_fib));
                    }
                }
                
                HSRV_IF_ERROR_RETURN(hsrv_ipv6_neighbor_create_neighbor_entry(p_nd_fib));
            }
            else if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE)
                && GLB_FLAG_ISSET(p_db_nd_fib->flags, GLB_ND_FLAG_ACTIVE))
            {
                HSRV_IF_ERROR_RETURN(hsrv_ipv6_neighbor_remove_neighbor_entry(p_nd_fib));
            }
        }
        break;

    case CDB_OPER_DEL:
        p_key_nd = (tbl_nd_fib_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_key_nd);
        p_db_nd_fib = tbl_nd_fib_get_nd_fib(p_key_nd);
        HSRV_PTR_CHECK(p_db_nd_fib);

        if (0 != p_db_nd_fib->ifindex)
        {
            rc = hsrv_ipv6_neighbor_check_neighbor_exist(p_db_nd_fib);
            if (HSRV_E_NONE == rc)
            {
                HSRV_IF_ERROR_RETURN(hsrv_ipv6_neighbor_remove_neighbor_entry(p_db_nd_fib));
            }
            else
            {
                return rc;
            }
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
/* liwh end */

int32
hsrv_neighbor_fdb_age_cb_init()
{
    return sai_nei_register_fdb(hsrv_neighbor_fdb_age_cb);
}

/* modified by liwh for bug 48486, 2018-08-14 */
enum nd_state
{
    NES_NONE,
    NES_INCOMPLETE,
    NES_REACHABLE,
    NES_STALE,
    NES_DELAY,
    NES_PROBE,
    NES_STATE_MAX,
};

#define IPV6_PREFIX_IS_LINK_LOCAL(p) \
    (((p)->prefixlen >= 10) && (((p)->u.val[0] & 0xff) == 0xFE) && (((p)->u.val[1] & 0xC0) == 0x80))

int32
hsrv_neighbor_nd_cb(uint8 *p_daddr, char *ifname)
{
    int32                        rc = SAI_STATUS_SUCCESS;
    tbl_nd_fib_t              *p_nd_fib = NULL;
    tbl_nd_fib_t              nd_fib;
    int32                        size;
    int32                        op;
    tbl_interface_t          *p_db_if = NULL;
    tbl_interface_t          intf;

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));

    nd_fib.key.addr.family = AF_INET6;
    nd_fib.key.addr.prefixlen = IPV6_MAX_BITLEN;
    sal_memcpy(&nd_fib.key.addr.u.prefix6, p_daddr, sizeof(addr_ipv6_t));

    if (IPV6_PREFIX_IS_LINK_LOCAL(&nd_fib.key.addr))
    {
        sal_strncpy(intf.key.name, ifname, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (p_db_if)
        {
            nd_fib.key.ifindex = p_db_if->ifindex;
        }
    }
    else
    {
        nd_fib.key.ifindex = GLB_INVALID_IFINDEX;
    }
     
    p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
    if (!p_nd_fib)
    {
        return  HSRV_E_NONE;
    }

    /* modified by liwh for bug 49577, 2018-10-19 
         maybe nd_fib state is not sync to fea, don't check in fea */
    /*if (NES_STALE != p_nd_fib->state)
    {
        return  HSRV_E_NONE;    
    }*/
    /* liwh end */

    op = ROUTED_FEI_KERNEL_UPDATE_ND_NOTIFY;
    size = sizeof(tbl_nd_fib_key_t);
    rc = HSRV2ROUTED_MSG_SEND_NOTIFY(op,&nd_fib.key, size);

    p_nd_fib->state = NES_DELAY;
    
    return rc;
}

int32
hsrv_neighbor_nd_cb_init()
{
    return sai_nei_register_nd(hsrv_neighbor_nd_cb);
}
/* liwh end */

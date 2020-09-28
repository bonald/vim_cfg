/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2016-03-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_debug.h"
#include "hsrv_ip_binding.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "ctc_sai_port.h"
#include "hsrv_interface.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

int32
hsrv_port_ifindex_to_gport(u_int32_t if_index, uint32 *pgport)
{

    tbl_interface_t* p_db_if = NULL;
    sai_object_id_t port_oid = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(if_index);
    if (NULL != p_db_if)
    {
        hsrv_interface_ifindex_to_objectid(if_index, &port_oid);
        ctc_sai_port_objectid_to_gport(port_oid, pgport);
    }

    return HSRV_E_NONE;    
}

int32
hsrv_ip_binding_entry_add_ip_port(prefix_t *ip_addr, u_int32_t if_index)
{
    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;
    
    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP;

    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip+port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;


    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_add_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_add_ip_mac_port(prefix_t *ip_addr, u_int8_t *mac, u_int32_t if_index)
{

    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;
    
    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    sal_memcpy(st_elem.mac_sa, mac, sizeof(mac_addr_t));
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip+mac+port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;

    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_add_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_add_ip_vlan_port(prefix_t *ip_addr, vid_t vlan_id, u_int32_t if_index)
{

    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;
    
    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    st_elem.vid = vlan_id;
#ifndef USW
    st_elem.is_svlan = TRUE;
#endif
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_VLAN;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip+vlan+port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;

    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_add_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_add_ip_mac_vlan_port(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index)
{

    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;
    
    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    sal_memcpy(st_elem.mac_sa, mac, sizeof(mac_addr_t));
    st_elem.vid = vlan_id;
#ifndef USW
    st_elem.is_svlan = TRUE;
#endif
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip+mac+vlan+port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;

    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_add_entry(&st_elem));
    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_del_ip_port(prefix_t *ip_addr, u_int32_t if_index)
{

    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;
    
    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP;

    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip-port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;

    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_remove_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_del_ip_mac_port(prefix_t *ip_addr, u_int8_t *mac, u_int32_t if_index)
{
    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;

    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    sal_memcpy(st_elem.mac_sa, mac, sizeof(mac_addr_t));
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip-mac-port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;

    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_remove_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_del_ip_vlan_port(prefix_t *ip_addr, vid_t vlan_id, u_int32_t if_index)
{
    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;

    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    st_elem.vid = vlan_id;
#ifndef USW
    st_elem.is_svlan = TRUE;
#endif
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_VLAN;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip-vlan-port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;
    
    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_remove_entry(&st_elem));

    return HSRV_E_NONE;
}

int32
hsrv_ip_binding_entry_del_ip_mac_vlan_port(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index)
{
    ctc_security_ip_source_guard_elem_t st_elem;
    uint32 us_gport = 0;

    sal_memset(&st_elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    /*message parse*/
    hsrv_port_ifindex_to_gport(if_index, &us_gport);
    st_elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    st_elem.ipv4_sa = ip_addr->u.prefix4.s_addr;
    sal_memcpy(st_elem.mac_sa, mac, sizeof(mac_addr_t));
    st_elem.vid = vlan_id;
#ifndef USW
    st_elem.is_svlan = TRUE;
#endif    
    st_elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN;
    
    /*set to SDK*/
    HSRV_IP_BINDING_LOG_DEBUG("[ip-mac-vlan-port] us_gport = %d, ip_src_guard_type = %d, "
        "mac_sa = %02X:%02X:%02X:%02X:%02X:%02X, ipv4_sa = %d.%d.%d.%d, vid = %d\n",
        us_gport,
        st_elem.ip_src_guard_type,
        st_elem.mac_sa[0], st_elem.mac_sa[1], st_elem.mac_sa[2], 
        st_elem.mac_sa[3], st_elem.mac_sa[4], st_elem.mac_sa[5],
        (st_elem.ipv4_sa >> 0) & 0xFF, (st_elem.ipv4_sa >> 8) & 0xFF,
        (st_elem.ipv4_sa >> 16) & 0xFF, (st_elem.ipv4_sa >> 24) & 0xFF,              
        st_elem.vid);
    
    st_elem.ipv4_sa = sal_hton32(st_elem.ipv4_sa);
    st_elem.gport = us_gport;
    
    HSRV_IF_ERROR_RETURN(ctc_ip_source_guard_remove_entry(&st_elem));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_sync
 * Purpose      : load vlan cdb to sai          
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
hsrv_ip_source_guard_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ipsg_fib_key_t  *p_ipsg_key = NULL;
    tbl_ipsg_fib_t      *p_ipsg = NULL;
    tbl_ipsg_fib_t      *p_db_ipsg_fib = NULL;
    int32               rc = PM_E_NONE;
        
    HSRV_IPSOUR_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsg = (tbl_ipsg_fib_t*)p_tbl;
        if (NULL != p_ipsg)
        {
            switch (p_ipsg->ipsg_type)
            {
                case IP_NONE:
                case IP_FILTERING:
                    rc = hsrv_ip_binding_entry_add_ip_port(&p_ipsg->ip_addr, p_ipsg->ifindex);
                    break;
                    
                case IP_MAC_FILTERING:
                    rc = hsrv_ip_binding_entry_add_ip_mac_port(&p_ipsg->ip_addr, p_ipsg->mac_addr, p_ipsg->ifindex);
                    break;
                    
                case IP_VLAN_FILTERING:
                    rc = hsrv_ip_binding_entry_add_ip_vlan_port(&p_ipsg->ip_addr, p_ipsg->vid, p_ipsg->ifindex);
                    break;

                case IP_MAC_VLAN_FILTERING:
                    rc = hsrv_ip_binding_entry_add_ip_mac_vlan_port(&p_ipsg->ip_addr, p_ipsg->mac_addr, p_ipsg->vid, p_ipsg->ifindex);
                    break;

                default:
                        HSRV_IP_BINDING_LOG_DEBUG("no such ip source guard type when adding item\n");
                    break;
            }
        }

        break;

    case CDB_OPER_DEL:
        p_ipsg_key = (tbl_ipsg_fib_key_t*)p_tbl;
        p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(p_ipsg_key);            
        if (NULL != p_db_ipsg_fib)
        {
            switch (p_db_ipsg_fib->ipsg_type)
            {
                case IP_NONE:
                case IP_FILTERING:
                    {
                        rc = hsrv_ip_binding_entry_del_ip_port(&p_db_ipsg_fib->ip_addr, p_db_ipsg_fib->ifindex);
                    }
                    break;
                    
                case IP_MAC_FILTERING:
                    {
                        rc = hsrv_ip_binding_entry_del_ip_mac_port(&p_db_ipsg_fib->ip_addr, p_db_ipsg_fib->mac_addr, p_db_ipsg_fib->ifindex);
                    }
                    break;

                case IP_VLAN_FILTERING:
                    {
                        rc = hsrv_ip_binding_entry_del_ip_vlan_port(&p_db_ipsg_fib->ip_addr, p_db_ipsg_fib->vid, p_db_ipsg_fib->ifindex);   
                    }
                    break;

                case IP_MAC_VLAN_FILTERING:
                    {
                        rc = hsrv_ip_binding_entry_del_ip_mac_vlan_port(&p_db_ipsg_fib->ip_addr, p_db_ipsg_fib->mac_addr, p_db_ipsg_fib->vid, p_db_ipsg_fib->ifindex);
                    }
                    break;

                default:
                    HSRV_IP_BINDING_LOG_DEBUG("no such ip source guard type when adding item\n");
                    break;
            }
        }

        break;

    case CDB_OPER_SET:
        p_ipsg = (tbl_ipsg_fib_t*)p_tbl;
        
        switch (field_id)
        {
            case TBL_IPSG_FIB_FLD_IFINDEX:
                break;

            case TBL_IPSG_FIB_FLD_IFNAME:
                break;
                
            case TBL_IPSG_FIB_FLD_MAC_ADDR:
                break;

            case TBL_IPSG_FIB_FLD_IP_ADDR:
                break;
                
            default:
                break;
        }
        break;

    default:
        break;
    }

    (void)rc;
    return HSRV_E_NONE;
}


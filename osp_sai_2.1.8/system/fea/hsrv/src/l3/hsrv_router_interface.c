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
#include "ctc_port.h"
#include "ctc_qos.h"
#include "ctc_api.h"
#include "lib_ioctl.h"
#include "lib_netlink.h"
#include "lib_sysctl.h"
#include "ctc_task.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"

#define HSRV_ROUTER_INTERFACE_MTU_DEFAULT 1500
extern sal_mutex_t *g_hsrv_brgif_mutex;
/* modified by liwh for bug 47781, 2018-09-17 */
extern int32
hsrv_dhcp_relay_update_service_when_add_delete_rtif(tbl_route_if_t *p_router_if, bool add);
/* liwh end */

extern int32
hsrv_aclqos_if_update_l3if_qos_attr(tbl_rif_t *rif);

int32
hsrv_router_kernel_update_local(char *ifname, int32 is_running)
{
    ctclib_slistnode_t *listnode = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;
    prefix_t p;

    p_db_rtif = tbl_route_if_get_route_if_by_name(ifname);
    if (NULL == p_db_rtif)
    {
        return HSRV_E_NONE;
    }

    sal_memset(&p, 0x00, sizeof(p));

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        sal_memcpy(&p, &p_db_conn->key.address, sizeof(p));
        if (is_running)
        {
            kernel_add_local(&p, p_db_rtif->kernel_ifindex);
        }
        else
        {
            kernel_del_local(&p);
        }
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_add 
 * Purpose      : add router interface      
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_add(tbl_route_if_t* p_rif)
{
    tbl_interface_t *p_if = NULL;
    tbl_rif_t rif;
    tbl_fea_global_t* p_fea_global = NULL;
    sai_router_interface_api_t* rif_api = NULL;
    sai_object_id_t rif_id = 0;
    sai_attribute_t attr[4];
    tbl_fea_lag_key_t fea_lag_key;
    tbl_fea_lag_t* p_fea_lag = NULL;
    tbl_rif_t *p_old_rif = NULL;
    sai_object_id_t portid = 0;
    uint32_t is_admin_up = 0;
    int32 ret = HSRV_E_NONE;

 
    /*1. debug out infor*/
    HSRV_L3IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rif);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    p_if = tbl_interface_get_interface_by_name(p_rif->key.name);
    HSRV_PTR_CHECK(p_if);

    sal_memset(&rif, 0, sizeof(rif));
    sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));
    p_old_rif = tbl_rif_get_rif(&rif.key);
    if (NULL != p_old_rif)
    {
        tbl_rif_del_rif(&rif.key);
    }

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);
    attr[0].value.oid = p_fea_global->router_id;
    
    if(p_if->hw_type == GLB_IF_TYPE_VLAN_IF)
    {
        attr[1].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
        attr[1].value.s32 = SAI_ROUTER_INTERFACE_TYPE_VLAN;
        attr[2].id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID;        
        attr[2].value.u16 = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
        attr[3].id = SAI_ROUTER_INTERFACE_ATTR_MTU;
        attr[3].value.u32 = HSRV_ROUTER_INTERFACE_MTU_DEFAULT;
                
        HSRV_IF_ERROR_RETURN(rif_api->create_router_interface(&rif_id, 4, attr));
        rif.rif_id = rif_id;
        portid = p_if->ifindex;     /* added by kcao for bug 40033, virtual interface uses ifindex as portid */
        sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));
        tbl_rif_add_rif(&rif);
    }
    else if(p_if->hw_type == GLB_IF_TYPE_PORT_IF)
    {
        attr[1].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
        attr[1].value.s32 = SAI_ROUTER_INTERFACE_TYPE_PORT;
        attr[2].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
        attr[2].value.oid = p_if->portid;
        attr[3].id = SAI_ROUTER_INTERFACE_ATTR_MTU;
        attr[3].value.u32 = HSRV_ROUTER_INTERFACE_MTU_DEFAULT;

        sal_mutex_lock(g_hsrv_brgif_mutex);
        ret = rif_api->create_router_interface(&rif_id, 4, attr);
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        rif.rif_id = rif_id;
        portid = p_if->portid;
        sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));
        tbl_rif_add_rif(&rif);

        /* added by hansf for update l3_if dscp domain and trust attr 2018/09/25 */
        hsrv_aclqos_if_update_l3if_qos_attr(&rif);

        /* modified by liwh for bug 51182, 2019-03-04 
             intialize dhcp exception to FALSE */
        hsrv_dhcprelay_set_excp_type(p_if->ifindex, FALSE);
        /* liwh end */
    }
    else if(p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
    {
        attr[1].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
        attr[1].value.s32 = SAI_ROUTER_INTERFACE_TYPE_PORT;
        attr[2].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
        sal_sscanf(p_if->key.name, "agg""%u", &fea_lag_key.id);
        p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
        HSRV_PTR_CHECK(p_fea_lag);
        attr[2].value.oid = p_fea_lag->lag_id;
        portid = p_fea_lag->lag_id;
        attr[3].id = SAI_ROUTER_INTERFACE_ATTR_MTU;
        attr[3].value.u32 = HSRV_ROUTER_INTERFACE_MTU_DEFAULT;   

        HSRV_IF_ERROR_RETURN(rif_api->create_router_interface(&rif_id, 4, attr));
        rif.rif_id = rif_id;
        sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));
        tbl_rif_add_rif(&rif);
    }
    else if(p_if->hw_type == GLB_IF_TYPE_LOOPBACK_IF)
    {
        sal_memcpy(rif.key.name, p_if->key.name, sizeof(p_if->key.name));
        tbl_rif_add_rif(&rif);
        portid = p_if->ifindex;     /* added by kcao for bug 40033, virtual interface uses ifindex as portid */
    }
     /*modified by wangl for openflow*/
    if ((p_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF))
    {
        hsrv_hostif_create(rif.key.name, portid, &rif.hostif_id);
        p_if->hostifid = rif.hostif_id;
        tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HOSTIFID);
    }
    
    tbl_rif_set_rif_field(&rif, TBL_RIF_FLD_HOSTIF_ID);

    /* SYSTEM MODIFIED by wangjj for fix kernel hw mac updating for bug44457, 2017-06-28 */
    is_admin_up = tbl_interface_is_up(p_if);
    if (is_admin_up)
    {
        lib_ioctl_if_set_up(p_if->key.name, !is_admin_up);
    }
    
    lib_ioctl_if_set_hw_addr(rif.key.name, p_rif->mac, MAC_ADDR_LEN);
    
    if (is_admin_up)
    {
        lib_ioctl_if_set_up(p_if->key.name, is_admin_up);
    }
    lib_sysctl_set_intf_enable_arp(rif.key.name, TRUE);
    lib_sysctl_set_intf_arp_retrans_time(rif.key.name, p_rif->arp_retrans_time);
    lib_sysctl_set_intf_arp_timeout(rif.key.name, p_rif->arp_timeout);
    lib_ioctl_if_set_mtu(rif.key.name, p_rif->mtu);

    /* init ip redirect */
    if (p_rif->ip_redirects_en)
    {
        lib_sysctl_set_intf_ip_redirects_en(GLB_DEF_VRFID, rif.key.name, TRUE);
    }
    else
    {
        lib_sysctl_set_intf_ip_redirects_en(GLB_DEF_VRFID, rif.key.name, FALSE);
    }

    if (p_rif->ip_unreachable_en)
    {
        lib_sysctl_set_intf_ip_unreachable_en(GLB_DEF_VRFID, rif.key.name, TRUE);
    }
    else
    {
        lib_sysctl_set_intf_ip_unreachable_en(GLB_DEF_VRFID, rif.key.name, FALSE);
    }

    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        lib_ioctl_if_set_up(p_if->key.name, TRUE);
    }
    else
    {
        lib_ioctl_if_set_up(p_if->key.name, FALSE);
    }
    
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP) && GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING))
    {
        lib_ioctl_if_set_running(p_if->key.name, TRUE);
    }
    else
    {
        lib_ioctl_if_set_running(p_if->key.name, FALSE);
    }

    MY_PRINTF("set %s %d", p_if->key.name, GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING));
    hsrv_router_kernel_update_local(p_if->key.name, GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING));
   
    return ret;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_remove
 * Purpose      : remove router interface      
 * Input        : p_srif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_remove(tbl_route_if_t*  p_srif)
{
    sai_router_interface_api_t* rif_api = NULL; 
    tbl_rif_t* p_db_rif = NULL;
    tbl_rif_t rif;
    int ret = HSRV_E_NONE;
    tbl_interface_t *p_db_if = NULL;

    /*1. debug out infor*/
    HSRV_L3IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_srif);
    

    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    sal_memcpy(rif.key.name, p_srif->key.name, sizeof(p_srif->key.name));
    p_db_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_db_rif);

    sal_mutex_lock(g_hsrv_brgif_mutex);
    ret = rif_api->remove_router_interface(p_db_rif->rif_id);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    p_db_if = tbl_interface_get_interface_by_name(p_srif->key.name);
    if (NULL == p_db_if)
    {
        /* p_if maybe removed at first */
        ret = hsrv_hostif_remove(p_db_rif->hostif_id);
    }
    else if((p_db_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_db_if->hw_type != GLB_IF_TYPE_LINKAGG_IF))
    {
        ret = hsrv_hostif_remove(p_db_rif->hostif_id);
    }

    /* modified by liwh for bug 47781, 2018-09-17 */
    hsrv_dhcp_relay_update_service_when_add_delete_rtif(p_srif, FALSE);
    /* liwh end */

    /* modified by chenc for delete tbl_rif when delete l3 if */
    HSRV_IF_ERROR_RETURN(tbl_rif_del_rif(&rif.key));
    return ret;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_set_mtu 
 * Purpose      : set router interface mtu
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_mtu(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_interface_t *p_if   = NULL;    
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    p_if = tbl_interface_get_interface_by_name(p_router_if->key.name);
    HSRV_PTR_CHECK(p_if);

    if (GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type)
    {
        lib_ioctl_if_set_mtu(p_router_if->key.name, p_router_if->mtu);
        return HSRV_E_NONE;
    }
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_MTU;
    attr.value.u32 = p_router_if->mtu;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    /* set net device's mtu */
    lib_ioctl_if_set_mtu(p_router_if->key.name, p_router_if->mtu);
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_router_interface_set_redirect 
 * Purpose      : set router interface mtu
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_redirect(tbl_route_if_t* p_router_if)
{

    HSRV_PTR_CHECK(p_router_if);

    if (p_router_if->ip_redirects_en)
    {
        lib_sysctl_set_intf_ip_redirects_en(GLB_DEF_VRFID, p_router_if->key.name, TRUE);
    }
    else
    {
        lib_sysctl_set_intf_ip_redirects_en(GLB_DEF_VRFID, p_router_if->key.name, FALSE);
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_set_unreachable
 * Purpose      : set router interface mtu
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_unreachable(tbl_route_if_t* p_router_if)
{

    HSRV_PTR_CHECK(p_router_if);

    if (p_router_if->ip_unreachable_en)
    {
        lib_sysctl_set_intf_ip_unreachable_en(GLB_DEF_VRFID, p_router_if->key.name, TRUE);
    }
    else
    {
        lib_sysctl_set_intf_ip_unreachable_en(GLB_DEF_VRFID, p_router_if->key.name, FALSE);
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_set_mtu 
 * Purpose      : set router interface mtu
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_urpf(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_URPF;
    attr.value.u32 = p_router_if->unicast_rpf_en;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}

int32
hsrv_router_interface_set_urpf_v6(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_URPF_V6;
    attr.value.u32 = p_router_if->unicast_rpf_en_v6;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}

int32
hsrv_router_interface_set_unicast_en_v6(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_router_if);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    /* bypass loopback interface */
    if (sal_strstr(p_router_if->key.name, GLB_IFNAME_LOOPBACK_PREFIX))
    {
        return HSRV_E_NONE;
    }
#if 0
    if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
    {
        lib_ioctl_if_set_up(p_if->key.name, TRUE);
    }
#endif
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE;
    attr.value.booldata = p_router_if->unicast_en_v6;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_set_dhcp_server_group 
 * Purpose      : set router interface dhcp server group
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_dhcp_server_group(tbl_route_if_t* p_router_if)
{
/* modified by liwh for bug 51199, 2019-03-05 
     when configure "dhcp relay", dhcp_pdu_enabled will be set TRUE */
#if 0
    tbl_rif_t       *p_rif  = NULL;
    tbl_rif_t       rif;
    tbl_sys_global_t *p_sys_global = tbl_sys_global_get_sys_global();
    uint8_t enabled = (0 != p_router_if->dhcp_server_group);
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    if (p_router_if->dhcp_pdu_enabled != enabled && p_sys_global 
            && p_sys_global->dhcp_service_enable && p_sys_global->dhcp_relay_enable) 
    {
        hsrv_dhcprelay_set_excp_type(p_router_if->ifindex, enabled);
        p_router_if->dhcp_pdu_enabled = enabled;
        HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_router_if, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
    }
#endif
/* liwh end */

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_router_interface_set_dhcp_client_enable 
 * Purpose      : set router interface dhcp server group
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
extern sai_status_t
ctc_sai_hostif_db_dhcp_sys_syn(uint32 enable);

int32
hsrv_router_interface_set_dhcp_client_enable(tbl_route_if_t* p_router_if, uint8 dhcpv6)
{
    uint8                  enabled =0;
    tbl_route_if_t      *p_db_rif= NULL;
    
    p_db_rif = tbl_route_if_get_route_if(&p_router_if->key);
    HSRV_PTR_CHECK(p_db_rif);

    if (dhcpv6)
    {
        enabled = (0 != p_router_if->dhcpv6_client_enable);  
    }
    else
    {
        enabled = (0 != p_router_if->dhcp_client_enable);  
    }

    /* dhcp to cpu can be set by "ip address dhcp" and "ipv6 address dhcp" at same port */
    if (enabled)
    {
        /* dhcp pdu is set to cpu already */
        if (p_db_rif->dhcp_pdu_enabled)
        {
            p_db_rif->dhcp_pdu_enabled++;
        }
        /* dhcp pdu should be set to cpu first time */
        else
        {
            hsrv_dhcprelay_set_excp_type(p_db_rif->ifindex, enabled);
            p_db_rif->dhcp_pdu_enabled = 1;    
        }

        HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_db_rif, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
    }
    else
    {
        if (p_db_rif->dhcp_pdu_enabled)
        {
            p_db_rif->dhcp_pdu_enabled--;

            if (0 == p_db_rif->dhcp_pdu_enabled)
            {
                hsrv_dhcprelay_set_excp_type(p_db_rif->ifindex, enabled);
            }

            HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_db_rif, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
        }
    }
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_interface_connect_sync
 * Purpose      : load interface ip address cdb to sai          
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
hsrv_router_interface_connect_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    HSRV_L3IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

/* support vrrp modified by liwh for bug 45215, 2017-09-17 */
/***************************************************************************************************
 * Name         : hsrv_router_interface_set_vmac 
 * Purpose      : set router interface vmac
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_vmac(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t   *rif_api = NULL;
    tbl_rif_t                    *p_rif  = NULL;
    sai_attribute_t              attr;
    tbl_rif_t                    rif;
    sai_mac_t                    vmac_zero;

    sal_memset(vmac_zero, 0, sizeof(sai_mac_t));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    /* set router mac to interface */
    attr.id = SAI_ROUTER_INTERFACE_ATTR_VMAC_ADDRESS;
    sal_memcpy(attr.value.vrrp.mac[0], p_router_if->mac, sizeof(sai_mac_t));
    attr.value.vrrp.num++;

    /* if vmac1 is valid, set vmac1 to chip */
    if (sal_memcmp(p_router_if->vmac1, vmac_zero, sizeof(sai_mac_t)))
    {
        sal_memcpy(attr.value.vrrp.mac[attr.value.vrrp.num], p_router_if->vmac1, sizeof(sai_mac_t));
        attr.value.vrrp.num++;
    }

    /* if vmac2 is valid, set vmac2 to chip */
    if (sal_memcmp(p_router_if->vmac2, vmac_zero, sizeof(sai_mac_t)))
    {
        sal_memcpy(attr.value.vrrp.mac[attr.value.vrrp.num], p_router_if->vmac2, sizeof(sai_mac_t));
        attr.value.vrrp.num++;
    }

    /* if vmac2 is valid, set vmac3 to chip */
    if (sal_memcmp(p_router_if->vmac3, vmac_zero, sizeof(sai_mac_t)))
    {
        sal_memcpy(attr.value.vrrp.mac[attr.value.vrrp.num], p_router_if->vmac3, sizeof(sai_mac_t));
        attr.value.vrrp.num++;
    }

    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));
    
    return HSRV_E_NONE;
}
/* liwh end */


/* begin added by taocy for NAT, 2019-06-03 */
/***************************************************************************************************
 * Name         : hsrv_router_interface_set_nat_enable 
 * Purpose      : set router interface nat enable flag
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_interface_set_nat_enable(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t   *rif_api = NULL;
    tbl_rif_t                    *p_rif  = NULL;
    tbl_rif_key_t                 rif_key;
    sai_attribute_t               attr ;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    sal_memcpy(rif_key.name, p_router_if->key.name, IFNAME_SIZE); 

    /* get old configuration*/
    p_rif = tbl_rif_get_rif(&rif_key);
    HSRV_PTR_CHECK(p_rif);

    /* set category id first*/
    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_ROUTER_INTERFACE_ATTR_NAT_CATEGORY_ID;
    attr.value.u8 = p_router_if->nat_category_id;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    /* set nat enable second. make sure when sai process this attr, it can get category from its own db.*/
    attr.id = SAI_ROUTER_INTERFACE_ATTR_NAT_ENABLE;
    attr.value.u8 = p_router_if->nat_enable;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));
    return HSRV_E_NONE;
}

/*add by zhw for mc*/
/***************************************************************************************************
 *  * Name         : hsrv_router_interface_set_multicast 
 *   * Purpose      : set router interface multicast
 *    * Input        : p_rif: table router interface
 *     * Output       : N/A                  
 *      * Return       : HSRV_E_XXXX
 *       * Note         : N/A
 *       ***************************************************************************************************/
int32
hsrv_router_interface_set_multicast(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_MULTICAST;
    attr.value.booldata= p_router_if->multicast;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}
/*end add zhw*/


/*add by zhw for ipmc*/
/***************************************************************************************************
 * Name         : hsrv_router_interface_set_igmp 
 * Purpose      : set router interface igmp
 * Input        : p_rif: table router interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_route_interface_set_igmp(tbl_route_if_t* p_router_if)
{
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));
    
    sal_memcpy(rif.key.name, p_router_if->key.name, sizeof(p_router_if->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_IGMP;
	if(p_router_if->igmp == 1)
		attr.value.booldata = TRUE;
	else
		attr.value.booldata = FALSE;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

    return HSRV_E_NONE;
}

/*zhw end add*/
/* end added by taocy for NAT, 2019-06-03 */


/***************************************************************************************************
 * Name         : hsrv_router_interface_sync
 * Purpose      : load router interface cdb to sai          
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
hsrv_router_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_route_if_key_t *p_srif_key = NULL;
    tbl_route_if_t*  p_srif= NULL;
    
    /*1. debug out infor*/
    HSRV_L3IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    /*2. process ds */
    if(p_ds_node)
    {
        switch (p_ds_node->id)
        {
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
        p_srif = (tbl_route_if_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_router_interface_add(p_srif));
        break;

    case CDB_OPER_DEL:
        p_srif_key = (tbl_route_if_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_srif_key);
        p_srif = tbl_route_if_get_route_if(p_srif_key);
        HSRV_PTR_CHECK(p_srif);
        HSRV_IF_ERROR_RETURN(hsrv_router_interface_remove(p_srif));
        break;

    case CDB_OPER_SET:
        p_srif = (tbl_route_if_t*)p_tbl;
        switch (field_id)
        {
            case TBL_ROUTE_IF_FLD_MTU:
                HSRV_IF_ERROR_RETURN(hsrv_router_interface_set_mtu(p_srif));
                break;

            case TBL_ROUTE_IF_FLD_UNICAST_RPF_EN:
                HSRV_IF_ERROR_RETURN(hsrv_router_interface_set_urpf(p_srif));
                break;

            case TBL_ROUTE_IF_FLD_UNICAST_RPF_EN_V6:
                HSRV_IF_ERROR_RETURN(hsrv_router_interface_set_urpf_v6(p_srif));
                break;

            case TBL_ROUTE_IF_FLD_UNICAST_EN_V6:
                HSRV_IF_ERROR_RETURN(hsrv_router_interface_set_unicast_en_v6(p_srif));
                break;
                
            case TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME:
                lib_sysctl_set_intf_arp_retrans_time(p_srif->key.name, p_srif->arp_retrans_time);
                break;

            case TBL_ROUTE_IF_FLD_ARP_TIMEOUT:
                lib_sysctl_set_intf_arp_timeout(p_srif->key.name, p_srif->arp_timeout);
                break;
                
            case TBL_ROUTE_IF_FLD_IP_REDIRECTS_EN:
                hsrv_router_interface_set_redirect(p_srif);
                break;

            case TBL_ROUTE_IF_FLD_IP_UNREACHABLE_EN:
                hsrv_router_interface_set_unreachable(p_srif);
                break;

            case TBL_ROUTE_IF_FLD_DHCP_SERVER_GROUP:
                hsrv_router_interface_set_dhcp_server_group(p_srif);
                break;

            case TBL_ROUTE_IF_FLD_DHCP_CLIENT_ENABLE:
                hsrv_router_interface_set_dhcp_client_enable(p_srif, FALSE);
                break;
                
            case TBL_ROUTE_IF_FLD_DHCPV6_CLIENT_ENABLE:
                hsrv_router_interface_set_dhcp_client_enable(p_srif, TRUE);
                break;

            /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
            case TBL_ROUTE_IF_FLD_VMAC1:
            case TBL_ROUTE_IF_FLD_VMAC2:
            case TBL_ROUTE_IF_FLD_VMAC3:
                hsrv_router_interface_set_vmac(p_srif);
                break;
            /* liwh end */
            case TBL_ROUTE_IF_FLD_NAT_ENABLE:
                hsrv_router_interface_set_nat_enable(p_srif);
                break;
			/*add by zhw for ipmc*/
			case TBL_ROUTE_IF_FLD_MULTICAST:
				hsrv_router_interface_set_multicast(p_srif);
				break;
			case TBL_ROUTE_IF_FLD_IGMP:
				hsrv_route_interface_set_igmp(p_srif);
				break;
			/*end add zhw*/
            default:
                break;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

/* modified by liwh for bug 47781, 2018-09-17 */
/***************************************************************************************************
 * Name         : hsrv_router_interface_after_sync
 * Purpose      : load router interface cdb to sai          
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
hsrv_router_interface_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_route_if_key_t *p_srif_key = NULL;
    tbl_route_if_t*  p_srif= NULL;
    
    /*1. debug out infor*/
    HSRV_L3IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    /*2. process ds */
    if(p_ds_node)
    {
        switch (p_ds_node->id)
        {
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
        p_srif_key = (tbl_route_if_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_srif_key);
        p_srif = tbl_route_if_get_route_if(p_srif_key);
        HSRV_PTR_CHECK(p_srif);
        hsrv_dhcp_relay_update_service_when_add_delete_rtif(p_srif, TRUE);
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
/*liwh end */

int32
_rif_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_rif_iterate((TBL_ITER_CB_FUNC)tbl_rif_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rif_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _rif_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}


    

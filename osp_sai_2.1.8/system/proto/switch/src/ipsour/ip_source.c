/****************************************************************************
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Justin
 * Date          : 2007-011-14
 * Reason        : First Create.
 ****************************************************************************/

/****************************************************************************
 *
 * Header Files
 * 
 ****************************************************************************/

#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "ipsour/ip_source.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_ipsg_s_ip_define.h"
#include "gen/tbl_ipsg_s_ip.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "dhcsnooping_func.h"
#include "dhcsnooping.h"
#include "genlog.h"
/****************************************************************************
 *  
 * Defines and Macros
 * 
 *****************************************************************************/

int32
ipsour_add_item(addr_t *addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type)
{
    tbl_ipsg_fib_t *p_db_ipsg_fib = NULL;
    tbl_ipsg_fib_t  ipsg_fib;
    int32           rc = IPSOUR_OK;

    if (NULL == mac)
    {
        return IPSOUR_NULL_POINTER;
    }
    
    sal_memset(&ipsg_fib, 0, sizeof(ipsg_fib));
    switch (new_type)
    {
    case IP_NONE:
    case IP_FILTERING:
    case IP_MAC_FILTERING:
    case IP_VLAN_FILTERING:
    case IP_MAC_VLAN_FILTERING:
        ipsg_fib.key.fid = vlan_id;
        sal_memcpy(ipsg_fib.key.mac, mac, sizeof(mac_addr_t));
        sal_memcpy(&ipsg_fib.ip_addr.u, &addr->u, sizeof(addr_u));
        ipsg_fib.ip_addr.family = addr->family;
        
        sal_memcpy(ipsg_fib.mac_addr, mac, sizeof(mac_addr_t));
        sal_memcpy(ipsg_fib.ifname, ifname, sizeof(ipsg_fib.ifname));
        ipsg_fib.ifindex = if_index;
        ipsg_fib.vid =  vlan_id;
        ipsg_fib.ipsg_type = new_type;

        p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib.key);
        if (NULL == p_db_ipsg_fib)
        {
            rc = tbl_ipsg_fib_add_ipsg_fib(&ipsg_fib);
            (void)rc;
            return IPSOUR_OK;
        }
        break;
        
    default:
        {
            IPSOUR_DBG_ERR("No such ip source guard type when adding item\n");
            return IPSOUR_NO_SUCH_TYPE;
        }
        break;
    }

    return IPSOUR_FAILED;
}

int32
ipsour_del_item(addr_t *addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t old_type)
{

    tbl_ipsg_fib_t      *p_db_ipsg_fib = NULL;
    tbl_ipsg_fib_key_t  ipsg_fib_key;
    int32               rc = IPSOUR_OK;

    if (NULL == mac)
    {
        return IPSOUR_NULL_POINTER;
    }
    
    sal_memset(&ipsg_fib_key, 0, sizeof(ipsg_fib_key));
    switch (old_type)
    {
    case IP_NONE:        
    case IP_FILTERING:
    case IP_MAC_FILTERING:
    case IP_VLAN_FILTERING:
    case IP_MAC_VLAN_FILTERING:
        sal_memcpy(ipsg_fib_key.mac, mac, sizeof(mac_addr_t));
        ipsg_fib_key.fid = vlan_id;
        p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib_key);            
        if (NULL != p_db_ipsg_fib)
        {
            rc = tbl_ipsg_fib_del_ipsg_fib(&ipsg_fib_key);
            return IPSOUR_OK;
        }
        else
        {
            return IPSOUR_NO_SUCH_ITEM;
        }
        break;

    default:
        {
            IPSOUR_DBG_ERR("No such ip source guard type when deleting item\n");
            return IPSOUR_NO_SUCH_TYPE;
        }
        break;
    }

    return rc;
}

static int
ipsour_dynamic_check_static(tbl_dhcbinding_t *pItem)
{   
    tbl_ipsg_fib_t ipsg_fib;

    /* input check */
    if (NULL == pItem)
    {
        return IPSOUR_NULL_POINTER;
    }

    sal_memset(&ipsg_fib, 0x00, sizeof(ipsg_fib));

    ipsg_fib.ifindex = (uint32)tbl_interface_get_ifindex_by_name(pItem->ifname);
    ipsg_fib.key.fid = pItem->vid;
    ipsg_fib.vid = pItem->vid;
    sal_memcpy(ipsg_fib.key.mac, pItem->client_mac, sizeof(mac_addr_t));
    sal_memcpy(ipsg_fib.mac_addr, pItem->client_mac, sizeof(mac_addr_t));
    sal_memcpy(&ipsg_fib.ip_addr.u, &pItem->client_ip.u, sizeof(addr_u));
    ipsg_fib.ip_addr.family = pItem->client_ip.family;
    sal_memcpy(ipsg_fib.ifname, pItem->ifname, sizeof(ipsg_fib.ifname));
    
    /* search mac in the static item hash */
    if (NULL != tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(ipsg_fib.mac_addr))
    {
        return IPSOUR_DUPLICATE_MAC;
    }

    /* search IP in the static item hash */
    if (NULL != tbl_ipsg_s_ip_get_ipsg_s_ip_by_ipaddr(&ipsg_fib.ip_addr))
    {
        return IPSOUR_DUPLICATE_IP;
    }

    return IPSOUR_OK;
}

static int
ipsour_dynamic_add_check(tbl_dhcbinding_t *pItem)
{
    tbl_dhcbinding_t stItem;

    /*initialize*/
    sal_memset(&stItem, 0x00, sizeof(tbl_dhcbinding_t));
    
    /* input check */
    if (NULL == pItem)
    {
        return IPSOUR_NULL_POINTER;
    }

    /* only need two key value: ip and mac */
    sal_memcpy(stItem.client_mac, pItem->client_mac, ETH_ALEN);
    stItem.vid = pItem->vid;
    /*modified by weij for bug 14292, 2011-03-18*/    
    if (pItem->key.ipv6)
    {
        stItem.client_ip.family = AF_INET6;
        sal_memcpy(&(stItem.client_ip.u.prefix6), &(pItem->client_ip.u.prefix6), sizeof(addr_ipv6_t));
    }
    else
    {
        stItem.client_ip.family = AF_INET;
        stItem.client_ip.u.prefix4.s_addr = pItem->client_ip.u.prefix4.s_addr;
    }

    /* search mac in the static item hash */
    if (IPSOUR_DUPLICATE_MAC == ipsour_dynamic_check_static(&stItem))
    {
        return IPSOUR_DUPLICATE;
    }

    /* search IP in the static item hash */
    if (IPSOUR_DUPLICATE_IP == ipsour_dynamic_check_static(&stItem))
    {
        return IPSOUR_DUPLICATE;
    }

    return IPSOUR_OK;
}

static int
ipsour_dynamic_add_check_dynamic(tbl_dhcbinding_t *pItem, u_int8_t type)
{    
   tbl_dhcbinding_t *pdItem_mac = NULL;
   tbl_dhcbinding_t *pdItem_ip = NULL;
   tbl_dhcbinding_t *pdItem = NULL;      
   ctclib_slistnode_t *listnode = NULL;
   int nRet = 0;
   tbl_interface_t *p_db_if = NULL;
   char szIpBuf[128];

    char   buf[CMD_BUF_128];

    sal_memset(buf,0x00, sizeof(buf));
   sal_memset(szIpBuf, 0, sizeof(szIpBuf));
   /* input check */
   if (NULL == pItem)
   {
       return IPSOUR_NULL_POINTER;
   }

   /* according ipsg type to check*/
   CTCLIB_SLIST_LOOP(DHCSNOOPING_QUEUE, pdItem, listnode) 
   {
       if (DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR == pdItem->cIpsour
           || DHCBINDING_STATE_RUNNING != pdItem->state)
       {
           /* not an ip source guard item */
           continue;
       }
       
       if (pdItem_ip && pdItem_mac)
       {
           break;
       }
       
       switch (type)
       {
           /* since the limitation, we just do ip and mac check for all type*/
           case IP_VLAN_FILTERING:
           case IP_MAC_VLAN_FILTERING:
           case IP_MAC_FILTERING:
           case IP_FILTERING:
           case IP_NONE:
               if (!pdItem_ip && !sal_memcmp(&pdItem->client_ip, &pItem->client_ip, sizeof(addr_t)))
               {
                   pdItem_ip = pdItem;
               }

               if (!pdItem_mac && !sal_memcmp(&pdItem->client_mac, &pItem->client_mac, ETH_ALEN))
               {
                   pdItem_mac = pdItem;
               }
               break;
           default:  
               IPSOUR_DBG_ERR("Unknown ipsg type %d when check snooping entry\n", type);
               return IPSOUR_NO_SUCH_TYPE;           
       }
   }
       
   if ((pdItem_ip && (DHCPSNOOPING_ITEM_TYPE_CONFIGED == pdItem_ip->type))
       || (pdItem_mac && (DHCPSNOOPING_ITEM_TYPE_CONFIGED == pdItem_mac->type)))
   {
       IPSOUR_DEBUG(IPSG_EVENT, "Ipsg entry from dhcpsnooping configured entry can't be replaced "
               "by the one from other dhcpsnooping entry\n");
       return IPSOUR_DUPLICATE; 
   }

   if (pdItem_ip)
   {
       p_db_if = tbl_interface_get_interface_by_name((char*)pdItem_ip->ifname);
       if (NULL == p_db_if)
       {
           IPSOUR_DBG_ERR("Wrong interface name %s when delete Ipsg entry with same ip\n",
               (char*)pdItem_ip->ifname);
           return IPSOUR_DUPLICATE;
       }
       
       nRet = ipsour_dynamic_del_callback(pdItem_ip, p_db_if);
       if (nRet)
       {
           IPSOUR_DBG_ERR("Delete failed for Ipsg entry with same ip : %s\n",
               cdb_addr_val2str(buf, 128, &pItem->client_ip));
           return nRet;
       }

       pdItem_ip->cIpsour = DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR;
       tbl_dhcbinding_set_dhcbinding_field(pdItem_ip, TBL_DHCBINDING_FLD_CIPSOUR);
   }

   if (pdItem_mac && (pdItem_mac != pdItem_ip))
   {
       p_db_if = tbl_interface_get_interface_by_name((char*)pdItem_mac->ifname);
       if (NULL == p_db_if)
       {
           IPSOUR_DBG_ERR("Wrong interface name %s when delete Ipsg entry with same mac\n",
               (char*)pdItem_mac->ifname);
           return IPSOUR_DUPLICATE;
       }
       
       nRet = ipsour_dynamic_del_callback(pdItem_mac, p_db_if);
       if (nRet)
       {
           IPSOUR_DBG_ERR("Delete failed for Ipsg entry with same mac : %02x:%02x:%02x:%02x:%02x:%02x\n",
               pdItem_mac->client_mac[0], pdItem_mac->client_mac[1],
               pdItem_mac->client_mac[2], pdItem_mac->client_mac[3],
               pdItem_mac->client_mac[4], pdItem_mac->client_mac[5]);
           return nRet;
       }

       pdItem_mac->cIpsour = DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR;
       tbl_dhcbinding_set_dhcbinding_field(pdItem_mac, TBL_DHCBINDING_FLD_CIPSOUR);       
   }
   
   return IPSOUR_OK;
}

int32
ipsour_is_static_binding(prefix_t *ip_addr, uint8 *mac)
{
    tbl_ipsg_s_mac_t    *p_db_ipsg_s_mac = NULL;
    tbl_ipsg_s_mac_t    ipsg_s_mac;
    tbl_ipsg_s_ip_t     *p_db_ipsg_s_ip = NULL;
    tbl_ipsg_s_ip_t     ipsg_s_ip;

    sal_memset(&ipsg_s_mac, 0, sizeof(ipsg_s_mac));
    sal_memcpy(ipsg_s_mac.key.mac, mac, sizeof(mac_addr_t));
    p_db_ipsg_s_mac = tbl_ipsg_s_mac_get_ipsg_s_mac(&ipsg_s_mac.key);
    if (NULL != p_db_ipsg_s_mac)
    {
        return TRUE;
    }

    sal_memset(&ipsg_s_ip, 0, sizeof(ipsg_s_ip));
    sal_memcpy(&ipsg_s_ip.key.ip, ip_addr, sizeof(prefix_t));
    p_db_ipsg_s_ip = tbl_ipsg_s_ip_get_ipsg_s_ip(&ipsg_s_ip.key);
    if (NULL != p_db_ipsg_s_ip)
    {
        return TRUE;
    }

    return FALSE;
}

int32 
ipsour_dynamic_add_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if)
{ 
    int iRet = 0;
    char   buf[CMD_BUF_128];

    sal_memset(buf,0x00, sizeof(buf));
    /* input check */
    if ((NULL == pItem) || (NULL == p_db_if))
    {
        return IPSOUR_NULL_POINTER;               
    }

    IPSOUR_DEBUG(IPSG_EVENT, "DHCP snooping add item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s %s ", 
              pItem->vid,
              pItem->client_mac[0], pItem->client_mac[1],
              pItem->client_mac[2], pItem->client_mac[3],
              pItem->client_mac[4], pItem->client_mac[5],
              cdb_addr_val2str(buf, 128, &pItem->client_ip),
              pItem->ifname);

    /* check if the same item has been add to ip souce guard*/
    if (IPSOUR_OK != ipsour_dynamic_add_check(pItem))
    {
        IPSOUR_DBG_ERR("IP source guard binding duplicated when adding dhcpsnooping binding item");
        return IPSOUR_DUPLICATE;
    }
    /* check if the same item has been add to dhcp snooping 
    ** for bug 19695 modified by liuyang 2012-8-22 
    */
    if (IPSOUR_OK != ipsour_dynamic_add_check_dynamic(pItem, p_db_if->ip_source_type))
    {
        IPSOUR_DBG_ERR("IP source guard binding duplicated when adding dhcpsnooping binding item");
        return IPSOUR_DUPLICATE;
    }

    /* if port has limitation*/
    tbl_brg_global_t *p_db_glb = tbl_brg_global_get_brg_global();
    if (0 != p_db_glb->ipsg_max_port_binding)
    {
        /* number is reach max */
        if (p_db_glb->ipsg_max_port_binding <= p_db_if->ip_source_item_number)
        {
            IPSOUR_DBG_ERR("IP source guard binding reach port max when adding dhcpsnooping binding item");
            return IPSOUR_REACH_PORT_MAX;
        }
    }

    if (ipsour_ipsg_count_get() >= ipsour_ipsg_count_get_max())
    {
        IPSOUR_DBG_ERR("IP source guard binding reach total max when adding dhcpsnooping binding item");
        return IPSOUR_REACH_TOTAL_MAX;
    }

    /* call function to add to TCAM*/
    if (pItem->client_ip.family == AF_INET)
    {
        /* call function to add to tcam */
        iRet = ipsour_add_item(&pItem->client_ip, pItem->client_mac, pItem->vid, 
                    p_db_if->ifindex, pItem->ifname, p_db_if->ip_source_type);
    }

    if (IPSOUR_OK == iRet)
    {
        if (pItem->client_ip.family == AF_INET)
        {
            ipsour_ipsg_count_inc();
        }

        p_db_if->ip_source_item_number++;
    }

    return iRet;
}

int32
ipsour_dynamic_del_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if)
{
    char   buf[CMD_BUF_128];
    int32  iRet = IPSOUR_MAX;
    int32 iIsRequestLeft = 0;/*fix bug 43821: after reqest ,binding ip is 0, not rcv ack, when time out, entry will not delete, by chenc*/
    prefix_t ip_addr = {0};

    sal_memset(buf, 0x00, sizeof(buf));
    /* input check */
    if ((NULL == pItem) || (NULL == p_db_if))
    {
        return IPSOUR_NULL_POINTER;               
    }

    /*fix bug 43816:static binding ,not delete , add by chenc start*/
    sal_memset(&ip_addr, 0, sizeof(ip_addr));
    ip_addr.family = pItem->client_ip.family;
    sal_memcpy(&ip_addr.u, &pItem->client_ip.u, sizeof(addr_u));
    if (TRUE == ipsour_is_static_binding(&ip_addr, pItem->client_mac))
    {
        return PM_E_NONE;
    }
    /*add by chenc end*/
    
    IPSOUR_DEBUG(IPSG_EVENT, "DHCP snooping delete item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s %s ", 
              pItem->vid,
              pItem->client_mac[0], pItem->client_mac[1],
              pItem->client_mac[2], pItem->client_mac[3],
              pItem->client_mac[4], pItem->client_mac[5],
              cdb_addr_val2str(buf, 128, &pItem->client_ip),
              pItem->ifname);

    /*fix bug43821: could delete item with 0 ip and 0 family , by chenc*/
    if (0 == pItem->client_ip.family)
    {
        iIsRequestLeft = 1;
    }
    
    /* call function to delete from TCAM */
    if (pItem->client_ip.family == AF_INET || 1 == iIsRequestLeft)
    {
        iRet = ipsour_del_item(&pItem->client_ip, pItem->client_mac, pItem->vid, 
                    p_db_if->ifindex, pItem->ifname, p_db_if->ip_source_type);
    }

   /* for bug 19748 add check before dec by liuyang 2012-7-4*/
    if (IPSOUR_OK == iRet)
    {
        if (pItem->client_ip.family == AF_INET || 1 == iIsRequestLeft)
        {  
           /* need to check by wangjj */
           if (ipsour_ipsg_count_get() > 0)
           {
               ipsour_ipsg_count_dec();
           }
        }

       if (p_db_if->ip_source_item_number > 0)
       {
           p_db_if->ip_source_item_number--;
       }
    }
    else if (IPSOUR_NO_SUCH_ITEM == iRet)
    {
        //IPSOUR_DBG_ERR("No such dhcp snooping binding item existent in ip source guard table\n");
    }
    else
    {
        IPSOUR_DBG_ERR("Delete dhcp snooping binding item from ip source guard table failed");
    }

    return iRet;
}

int32
ipsour_dynamic_callback_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_dhcbinding_t    *p_dhcbinding      = NULL;
    tbl_interface_t     *p_db_if           = NULL;
    int32 iRet = 0;

    tbl_ipsg_fib_t *p_db_ipsg_fib = NULL;
    tbl_ipsg_fib_t  ipsg_fib;

    if (NULL == p_tbl)
    {
        return PM_E_FAIL;
    }
    
    p_dhcbinding = (tbl_dhcbinding_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        if (NULL != p_dhcbinding)
        {
            goto do_add_tbl;
        }
        
        break;

    case CDB_OPER_DEL:
        
        break;

    case CDB_OPER_SET:
        if (TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT == field_id) 
        {
            ipsg_fib.key.fid = p_dhcbinding->vid;
            sal_memcpy(ipsg_fib.key.mac, p_dhcbinding->key.mac, sizeof(mac_addr_t));

            p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib.key);
            if (NULL != p_db_ipsg_fib)
            {
                return PM_E_NONE;
            }
            
            if (TRUE == p_dhcbinding->ipsg_tbl_exsit)
            {
                /*todo : add tbl*/
                goto do_add_tbl;
            }
        }
        
        if (TBL_DHCBINDING_FLD_IFNAME == field_id) 
        {
            p_db_if = tbl_interface_get_interface_by_name(p_dhcbinding->ifname);
            ipsg_fib.key.fid = p_dhcbinding->vid;
            sal_memcpy(ipsg_fib.key.mac, p_dhcbinding->key.mac, sizeof(mac_addr_t));

            p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib.key);
            if (NULL == p_db_ipsg_fib)
            {
                return PM_E_NONE;
            }
            p_db_ipsg_fib->ifindex = p_db_if->ifindex;
            sal_strcpy(p_db_ipsg_fib->ifname, p_dhcbinding->ifname);
            tbl_ipsg_fib_set_ipsg_fib_field(p_db_ipsg_fib, TBL_IPSG_FIB_FLD_IFNAME);
            tbl_ipsg_fib_set_ipsg_fib_field(p_db_ipsg_fib, TBL_IPSG_FIB_FLD_IFINDEX);
        }

        if (TBL_DHCBINDING_FLD_CLIENT_IP == field_id) 
        {
            ipsg_fib.key.fid = p_dhcbinding->vid;
            sal_memcpy(ipsg_fib.key.mac, p_dhcbinding->key.mac, sizeof(mac_addr_t));

            p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib.key);
            if (NULL == p_db_ipsg_fib)
            {
                goto do_add_tbl;
            }
            else
            {
                p_db_ipsg_fib->ip_addr.family = p_dhcbinding->client_ip.family;
                sal_memcpy(&p_db_ipsg_fib->ip_addr.u.prefix4, &p_dhcbinding->client_ip.u.prefix4, sizeof(addr_ipv4_t));
                tbl_ipsg_fib_set_ipsg_fib_field(p_db_ipsg_fib, TBL_IPSG_FIB_FLD_IP_ADDR);
            }
        }

        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;

do_add_tbl:
    if (p_dhcbinding->type == DHCBINDING_TYPE_LEARNING && p_dhcbinding->state != DHCBINDING_STATE_RUNNING)
    {
        return PM_E_NONE;
    }
    
    p_db_if = tbl_interface_get_interface_by_name(p_dhcbinding->ifname);
    iRet = ipsour_dynamic_add_callback(p_dhcbinding, p_db_if);
    if (IPSOUR_DUPLICATE == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_INFORMATIONAL, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "there is a same item in ip source guard already");
    }
    else if (IPSOUR_REACH_PORT_MAX == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_WARNING, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "reach ip source guard port number limitation");
    }
    else if (IPSOUR_REACH_TOTAL_MAX == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_WARNING, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "reach ip source guard total number limitation");
    }
    else if (IPSOUR_HAL_FAILED == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "add ip source binding to tcam error");
    }
    else if (IPSOUR_NO_SUCH_TYPE == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "ip source guard type error");
    }
    else if (-1 == iRet)
    {
        log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
            "bad interface name");
    }
    else if (IPSOUR_OK == iRet)
    {
        tbl_dhcbinding_t    *p_db_dhcbinding   = NULL;    

        p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&p_dhcbinding->key);
        if (NULL == p_db_dhcbinding)
        {
            return PM_E_FAIL;
        }

        p_db_dhcbinding->cIpsour = DHCPSNOOPING_ITEM_ADD_IPSOUR;
        tbl_dhcbinding_set_dhcbinding_field(p_db_dhcbinding, TBL_DHCBINDING_FLD_CIPSOUR);
    }

    return PM_E_NONE;
}

int32
ipsour_dynamic_callback_before(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_dhcbinding_t    *p_dhcbinding      = NULL;
    tbl_dhcbinding_t    *p_db_dhcbinding   = NULL;    
    tbl_interface_t     *p_db_if           = NULL;
    int iRet = 0;

    if (NULL == p_tbl)
    {
        return PM_E_FAIL;
    }
    
    p_dhcbinding = (tbl_dhcbinding_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        
        break;

    case CDB_OPER_DEL:
        if (NULL != p_dhcbinding)
        {
            p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&p_dhcbinding->key);
            if (NULL == p_db_dhcbinding)
            {
                return PM_E_FAIL;
            }
            
            p_db_if = tbl_interface_get_interface_by_name(p_db_dhcbinding->ifname);
            if (NULL == p_db_if) {
                return PM_E_FAIL;
            }
            
            iRet = ipsour_dynamic_del_callback(p_db_dhcbinding, p_db_if);    
            if (IPSOUR_HAL_FAILED == iRet)
            {
                IPSOUR_DBG_ERR("Delete ip source binding from tcam error.");
            }
            else if (IPSOUR_NO_SUCH_TYPE == iRet)
            {
                IPSOUR_DBG_ERR("IP source guard type error.");
            }
            else if (-1 == iRet)
            {
                IPSOUR_DBG_ERR("Bad interface name.");
            }
            else if (IPSOUR_OK == iRet)
            {
                p_db_dhcbinding->cIpsour = DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR;
                tbl_dhcbinding_set_dhcbinding_field(p_db_dhcbinding, TBL_DHCBINDING_FLD_CIPSOUR);
            }            
        }
        
        break;

    case CDB_OPER_SET:        
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}



int32
ipsour_type_change_process(uint32 new_type, uint32 old_type, tbl_interface_t *p_db_if)
{
    tbl_dhcbinding_t    *pdItem     = NULL;
    ctclib_slistnode_t  *listnode   = NULL;    
    int32   rc      = PM_E_NONE;
    int32   iRet    = 0;

    rc = ipsour_update_item_by_interface2(p_db_if, new_type);

    /* delete all dynamic ip source guard items related to this interface */
    /* need to check by wangjj for add the dynamic ipsg entry  */
    CTCLIB_SLIST_LOOP(DHCSNOOPING_QUEUE, pdItem, listnode) 
    {
        if (DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR == pdItem->cIpsour)
        {
           /* not an ip source guard item */
           continue;
        }

        /*not this interface, continue*/
        if (sal_strcasecmp((char*)p_db_if->key.name, (char*)pdItem->ifname))
        {
           continue;
        }

        if (!pdItem->key.ipv6)
        {
            iRet = ipsour_del_item(&pdItem->client_ip, pdItem->client_mac, pdItem->vid, 
                        p_db_if->ifindex, pdItem->ifname, p_db_if->ip_source_type);
        }

        /* call function to delete from tcam 
        ** for bug 19748 add check before dec by liuyang 2012-7-4 
        */
        if (IPSOUR_OK == iRet)
        {
            if (!pdItem->key.ipv6)
            {
                if (ipsour_ipsg_count_get() > 0)
                {
                    ipsour_ipsg_count_dec();
                }
            }
          
            if (p_db_if->ip_source_item_number > 0)
            {
                p_db_if->ip_source_item_number--;
            }
        }
        else
        {
            IPSOUR_DBG_ERR("Delete dhcp%s snooping item to ip source guard failed when type changes",
                 pdItem->key.ipv6 ? "v6" : "");
            /* delete failed, so continue. maybe we should find way to handle it, now log it (seldom occur) */
            IPSOUR_DBG_ERR("Delete dhcp%s item in ip source guard failed",
                 pdItem->key.ipv6 ? "v6" : "");
            continue;
        }

        if (!pdItem->key.ipv6)
        {
            iRet = ipsour_add_item(&pdItem->client_ip, pdItem->client_mac, pdItem->vid, 
                        p_db_if->ifindex, pdItem->ifname, new_type);
        }

        if (IPSOUR_OK == iRet)
        {
           if (!pdItem->key.ipv6)
           {
                if (ipsour_ipsg_count_get() > 0)
                {
                    ipsour_ipsg_count_inc();
                }
           }

           p_db_if->ip_source_item_number++;
        }
        else
        {
            IPSOUR_DBG_ERR("Add dhcp%s snooping item to ip source guard failed when type changes",
                 pdItem->key.ipv6 ? "v6" : "");
            IPSOUR_DBG_ERR("Add dhcp%s item in ip source guard failed",
                 pdItem->key.ipv6 ? "v6" : "");
            continue;
        }
    }

     return rc;
}

int32
ipsour_update_item_by_interface2(tbl_interface_t *p_db_if, u_int8_t new_type)
{
    tbl_ipsg_s_mac_master_t *p_master           = tbl_ipsg_s_mac_get_master();
    ctclib_slistnode_t      *listnode           = NULL;    
    tbl_ipsg_s_mac_t        *p_db_ipsg_s_mac    = NULL;
    int32                   rc                  = IPSOUR_OK;

    CTCLIB_SLIST_LOOP(p_master->ipsg_s_mac_list, p_db_ipsg_s_mac, listnode)
    {
        if(0 == sal_strncmp(p_db_ipsg_s_mac->ifname, p_db_if->key.name, sizeof(p_db_if->key.name)))
        {
            if (new_type)
            {
                rc = ipsour_cmd_del_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname,p_db_if->ip_source_type);

                rc = ipsour_cmd_add_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, new_type);

                 if (IPSOUR_OK == rc)
                 {
                    rc = ipsg_cmd_add_db_entry(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, new_type);
                 }
                 else
                 {
                    IPSOUR_DEBUG(IPSG_EVENT, "Add static item to ip source guard failed when type changes\n");
                 }
            }
            else
            {
                if (IP_FILTERING == p_db_if->ip_source_type || IP_NONE == p_db_if->ip_source_type)
                {
                    return rc;
                }

                rc = ipsour_cmd_del_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, p_db_if->ip_source_type);                

                ipsour_cmd_add_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, new_type);

                if (IPSOUR_OK == rc)
                {
                    rc = ipsg_cmd_add_db_entry(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                            p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, new_type);
                }
                else
                {
                IPSOUR_DEBUG(IPSG_EVENT, "Delete static item in ip source guard failed when type changes\n");                    
                }
            }
        }
    }
        
    return IPSOUR_OK;
}

int32
ipsour_port_mode_update(uint32 cType, tbl_interface_t *p_db_if)
{
    ctclib_slistnode_t  *listnode       = NULL;
    tbl_interface_t     *p_if_mem       = NULL;  
    ds_lag_t            *p_lag          = NULL;
    int32               rc              = PM_E_NONE;
    
    /* input check */
    if (NULL == p_db_if)
    {
        return IPSOUR_NULL_POINTER;
    }

    if (cType >= IP_TYPE_MAX)
    {
        IPSOUR_DBG_ERR("IP source guard type error\n");
        return IPSOUR_NO_SUCH_TYPE;
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            if (cType != p_if_mem->ip_source_type)
            {
                rc = ipsour_type_change_process(cType, p_if_mem->ip_source_type, p_if_mem);
            }
            
            p_if_mem->ip_source_type = cType;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));            
        }
    }    
    
    if (cType != p_db_if->ip_source_type)
    {
        rc = ipsour_type_change_process(cType, p_db_if->ip_source_type, p_db_if);
    }

    p_db_if->ip_source_type = cType;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));

    return rc;
}

int
ipsour_static_add_check_static(tbl_ipsg_fib_t *p_ipsg_fib)
{    
    /* input check */
    if (NULL == p_ipsg_fib)
    {
        return IPSOUR_NULL_POINTER;
    }

    /* search mac in the static item hash */
    if (NULL != tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(p_ipsg_fib->mac_addr))
    {
        return IPSOUR_DUPLICATE_MAC;
    }

    /* search IP in the static item hash */
    if (NULL != tbl_ipsg_s_ip_get_ipsg_s_ip_by_ipaddr(&p_ipsg_fib->ip_addr))
    {
        return IPSOUR_DUPLICATE_IP;
    }

    return IPSOUR_OK;
}

int
ipsour_static_del_check_static(tbl_ipsg_fib_t *p_ipsg_fib)
{    
    tbl_ipsg_s_mac_t *temp_pItem = NULL;

    /* input check */
    if (NULL == p_ipsg_fib)
    {
        return IPSOUR_NULL_POINTER;
    }

    temp_pItem = tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(p_ipsg_fib->mac_addr);
    if (temp_pItem != NULL)
    {
        if (temp_pItem->vid != p_ipsg_fib->vid)
        {
            return IPSOUR_OK;
        }
        if (sal_strcasecmp((char*)temp_pItem->ifname, (char*)p_ipsg_fib->ifname))
        {
            return IPSOUR_OK;
        }
        if (temp_pItem->ip_addr.u.prefix4.s_addr != p_ipsg_fib->ip_addr.u.prefix4.s_addr)
        {
            return IPSOUR_OK;
        }
        
        return IPSOUR_DUPLICATE;
    }

    return IPSOUR_OK;
}

/*no called*/
int32 
ipsour_if_delete_cb(tbl_interface_t *p_db_if)
{
    tbl_ipsg_s_mac_master_t *p_master           = tbl_ipsg_s_mac_get_master();
    ctclib_slistnode_t      *listnode           = NULL;    
    ctclib_slistnode_t      *next               = NULL;
    tbl_ipsg_s_mac_t        *p_db_ipsg_s_mac    = NULL;
    int32                   ret                 = IPSOUR_OK;

    if (NULL == p_db_if)
    {
        return IPSOUR_FAILED;
    }
    
    ipsour_port_disable(IP_NONE, p_db_if);
    CTCLIB_SLIST_LOOP_DEL(p_master->ipsg_s_mac_list, p_db_ipsg_s_mac, listnode, next)
    {
        if(0 == sal_strncmp(p_db_ipsg_s_mac->ifname, p_db_if->key.name, sizeof(p_db_if->key.name)))
        {
            ret = ipsour_cmd_del_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                        p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname,p_db_if->ip_source_type);

            if (IPSOUR_OK == ret)
            {
                ret = ipsg_cmd_del_db_entry(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                        p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, p_db_if->ip_source_type);
            }
        }
    }

    return ret;
}

/*delete agg port, called, delete static bind and dynamic bind*/
int32 
ipsour_if_delete_cb2(tbl_interface_t *p_db_if)
{
    tbl_ipsg_fib_master_t   *p_master           = tbl_ipsg_fib_get_master();
    ctclib_slistnode_t      *listnode           = NULL;    
    ctclib_slistnode_t      *next               = NULL;
    tbl_ipsg_fib_t          *p_db_ipsg_fib      = NULL;
    tbl_dhcbinding_t        *p_db_dhcbinding    = NULL; 
    int32                   ret                 = IPSOUR_OK;
    tbl_ipsg_fib_t          ipsg_fib;
    tbl_dhcbinding_key_t    dhcbinding_key;

    if (NULL == p_db_if)
    {
        return IPSOUR_FAILED;
    }

    sal_memset(&ipsg_fib, 0, sizeof(ipsg_fib));
    ipsour_port_disable(IP_NONE, p_db_if);
    CTCLIB_SLIST_LOOP_DEL(p_master->ipsg_fib_list, p_db_ipsg_fib, listnode, next)
    {
        if(0 == sal_strncmp(p_db_ipsg_fib->ifname, p_db_if->key.name, sizeof(p_db_if->key.name)))
        {
            sal_memcpy(&ipsg_fib, p_db_ipsg_fib, sizeof(ipsg_fib));
			
			sal_memset(&dhcbinding_key, 0x00, sizeof(dhcbinding_key));
            sal_memcpy(dhcbinding_key.mac, ipsg_fib.mac_addr, sizeof(mac_addr_t));
            dhcbinding_key.vid = p_db_ipsg_fib->vid;
			
            ret  = ipsour_static_del_check_static(&ipsg_fib);
            if (IPSOUR_DUPLICATE == ret) 
            {
                ret = ipsg_cmd_del_static_binding_callback(&ipsg_fib, p_db_if, FALSE);
            }

            p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&dhcbinding_key);
            if (NULL == p_db_dhcbinding) 
            {
                continue;
            }

            ret  = ipsour_dynamic_del_callback(p_db_dhcbinding, p_db_if);    
        }
    }

    return ret;
}

int32
ipsour_port_enable(uint32 cType, tbl_interface_t *p_db_if)
{
    ctclib_slistnode_t  *listnode       = NULL;
    tbl_interface_t     *p_if_mem       = NULL;  
    ds_lag_t            *p_lag          = NULL;
    int32               rc              = PM_E_NONE;
    
    /* input check */
    if (NULL == p_db_if)
    {
        return IPSOUR_NULL_POINTER;
    }

    if (cType >= IP_TYPE_MAX)
    {
        IPSOUR_DBG_ERR("IP source guard type error\n");
        return IPSOUR_NO_SUCH_TYPE;
    }

    IPSOUR_DEBUG(IPSG_EVENT, "Enable ip source guard on %s with type %c\n", p_db_if->key.name, cType);
    
    /* if type is changed, delete all items and add them with new type again */
    if (cType != p_db_if->ip_source_type)
    {
        rc = ipsour_type_change_process(cType, p_db_if->ip_source_type, p_db_if);
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            if (cType != p_if_mem->ip_source_type)
            {
                rc = ipsour_type_change_process(cType, p_if_mem->ip_source_type, p_if_mem);
            }
            
            p_if_mem->ip_source_type = cType;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));            
        }
    }    

    p_db_if->ip_source_type = cType;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));

    return rc;
}

int32 
ipsour_port_disable(uint32 cType, tbl_interface_t *p_db_if)
{
    ctclib_slistnode_t  *listnode       = NULL;
    tbl_interface_t     *p_if_mem       = NULL;  
    ds_lag_t            *p_lag          = NULL;
    int32               rc              = PM_E_NONE;
    
    /* input check */
    if (NULL == p_db_if)
    {
        return IPSOUR_NULL_POINTER;
    }

    IPSOUR_DEBUG(IPSG_EVENT, "Disable ip source guard on %s\n", p_db_if->key.name);

    /* if type is changed, delete all items and add them with new type again */
    if (cType != p_db_if->ip_source_type)
    {
        rc = ipsour_type_change_process(cType, p_db_if->ip_source_type, p_db_if);
    }
    
    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            if (cType != p_if_mem->ip_source_type)
            {
                rc = ipsour_type_change_process(cType, p_if_mem->ip_source_type, p_if_mem);
            }
            
            p_if_mem->ip_source_type = cType;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));            
        }
    }    

    p_db_if->ip_source_type = cType;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IP_SOURCE_TYPE));
    return rc;
}

int32
ipsour_api_init()
{
    cdb_pm_subscribe_tbl(TBL_DHCBINDING, ipsour_dynamic_callback_before, ipsour_dynamic_callback_after);

    return PM_E_NONE;
}

int32
ipsour_api_deinit()
{
    return PM_E_NONE;
}

uint32
ipsour_ipsg_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) { 
        return 0;
    }

    type = p_chassis->cur_stm_mode;    
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
            return p_sys_spec->ipv4_source_guard;
        }
    }
            
    return 0;
}

uint32
ipsour_ipsg_count_get()
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();

    if (NULL != p_brg_glb) {
        return p_brg_glb->cur_ipv4_source_guard;
    }

    return 0;
}

int32
ipsour_ipsg_count_inc()
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();

    if (NULL != p_brg_glb) {
        p_brg_glb->cur_ipv4_source_guard ++;
    }

    return PM_E_NONE;
}

int32
ipsour_ipsg_count_dec()
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();

    if (NULL != p_brg_glb) {
        p_brg_glb->cur_ipv4_source_guard --;
    }

    return PM_E_NONE;
}



/****************************************************************************
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : centecer
 * Date          : 2016-03-14
 * Reason        : First Create.
 ****************************************************************************/

/****************************************************************************
 *
 * Header Files
 * 
 ****************************************************************************/

#include "proto.h"
#include "glb_l2_define.h"
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
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "dhcsnooping_func.h"
#include "dhcsnooping.h"
/****************************************************************************
 *  
 * Defines and Macros
 * 
 *****************************************************************************/

#define     M_TYPE_STR  15

int         ip_number_limit = IPSOUR_DEFAULT_NUMBER;

int32
ipsour_check_item_by_ifname(tbl_ipsg_s_mac_t *p_db_ipsg_s_mac, tbl_iter_args_t *pargs)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = pargs->argv[0];
    u_int8_t new_type = *(u_int8_t *)(pargs->argv[1]);
    
    if (NULL == p_db_ipsg_s_mac || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_memcmp(p_db_ipsg_s_mac->ifname, p_db_if->key.name, sizeof(p_db_if->key.name)))
    {
        rc = ipsour_cmd_add_item(&p_db_ipsg_s_mac->ip_addr, p_db_ipsg_s_mac->mac_addr, 
                    p_db_ipsg_s_mac->vid, p_db_if->ifindex, p_db_ipsg_s_mac->ifname, new_type);
    }

    return rc;
}

int32
ipsour_update_item_by_interface(tbl_interface_t *p_db_if, u_int8_t new_type)
{
    tbl_iter_args_t     args;
    
    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_db_if;
    args.argv[1] = &new_type;
    
    tbl_ipsg_s_mac_iterate((TBL_ITER_CB_FUNC)ipsour_check_item_by_ifname, &args);

    return PM_E_NONE;
}


int32 
ipsour_set_dhcsnooping_ipsg_flag(uint32 flag, u_int8_t *mac, vid_t vid)
{
    tbl_ipsg_fib_t      *p_db_ipsg_fib = NULL;
    tbl_ipsg_fib_key_t  ipsg_fib_key;

    sal_memcpy(ipsg_fib_key.mac, mac, sizeof(mac_addr_t));
    ipsg_fib_key.fid = vid;
    p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib_key);            
    if (NULL != p_db_ipsg_fib)
    {
        if (p_db_ipsg_fib->set_dhcbinding_ipsg_tbl_exsit != flag)
        {
            p_db_ipsg_fib->set_dhcbinding_ipsg_tbl_exsit = flag;
            return tbl_ipsg_fib_set_ipsg_fib_field(p_db_ipsg_fib, TBL_IPSG_FIB_FLD_SET_DHCBINDING_IPSG_TBL_EXSIT);
        }
    }

    return PM_E_NONE;
}

int32
ipsour_cmd_add_item(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type)
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
        sal_memcpy(&ipsg_fib.ip_addr, ip_addr, sizeof(prefix_t));
        sal_memcpy(ipsg_fib.mac_addr, mac, sizeof(mac_addr_t));
        sal_memcpy(ipsg_fib.ifname, ifname, sizeof(ipsg_fib.ifname));
        ipsg_fib.ifindex = if_index;
        ipsg_fib.vid =  vlan_id;
        ipsg_fib.ipsg_type = new_type;

        p_db_ipsg_fib = tbl_ipsg_fib_get_ipsg_fib(&ipsg_fib.key);
        if (NULL == p_db_ipsg_fib)
        {
            rc = tbl_ipsg_fib_add_ipsg_fib(&ipsg_fib);
            if (PM_E_NONE == rc)
            {
                ipsour_set_dhcsnooping_ipsg_flag(TRUE, mac, vlan_id);
            }
            return IPSOUR_OK;
        }

        break;
        
    default:
        {
            IPSOUR_DBG_ERR("no such ip source guard type when adding item\n");
            return IPSOUR_NO_SUCH_TYPE;
        }
        break;
    
    }

    return IPSOUR_FAILED;
}

int32
ipsour_cmd_del_item(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t old_type)
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
            ipsour_set_dhcsnooping_ipsg_flag(FALSE, mac, vlan_id);
            rc = tbl_ipsg_fib_del_ipsg_fib(&ipsg_fib_key);
            
            return IPSOUR_OK;
        }
        break;

    default:
        {
            IPSOUR_DBG_ERR("no such ip source guard type when deleting item\n");
            return IPSOUR_NO_SUCH_TYPE;
        }
        break;
    }

    return rc;
}

static int
ipsour_static_add_check_dynamic(tbl_ipsg_fib_t *pItem, u_int8_t type)
{
    /* S_DhcpSnoopingBindingItem stItem; */
    tbl_dhcbinding_t    *pdItem_mac = NULL;
    tbl_dhcbinding_t    *pdItem_ip  = NULL;
    tbl_dhcbinding_t    *pdItem     = NULL;     
    tbl_interface_t     *p_db_if    = NULL;
    int     nRet = 0;
    char    buf[CMD_BUF_128];

    sal_memset(buf,0x00, sizeof(buf));
    if (NULL == pItem)
    {
        return IPSOUR_NULL_POINTER;
    }

  /* according ipsg type to check*/
   ctclib_slistnode_t *listnode = NULL;    
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
               if (!pdItem_ip && !sal_memcmp(&pdItem->client_ip.u, &pItem->ip_addr.u, sizeof(addr_u)))
               {
                   pdItem_ip = pdItem;
               }

               if (!pdItem_mac && !sal_memcmp(&pdItem->client_mac, &pItem->mac_addr, ETH_ALEN))
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
        IPSOUR_DBG_ERR("Ipsg entry from dhcpsnooping configured entry will be replaced "
                "by the ipsg static entry\n");
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
            IPSOUR_DBG_ERR("Delete failed for Ipsg entry with same ip: %s\n",
               cdb_addr_ipv4_val2str(buf, 128, &pItem->ip_addr.u.prefix4));
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
ipsg_cmd_add_db_entry(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type)
{
    tbl_ipsg_s_mac_t    *p_db_ipsg_s_mac = NULL;
    tbl_ipsg_s_mac_t    ipsg_s_mac;
    tbl_ipsg_s_ip_t     *p_db_ipsg_s_ip = NULL;
    tbl_ipsg_s_ip_t     ipsg_s_ip;
    int32               rc = IPSOUR_OK;

    sal_memset(&ipsg_s_mac, 0, sizeof(ipsg_s_mac));

    sal_memcpy(ipsg_s_mac.key.mac, mac, sizeof(mac_addr_t));
    sal_memcpy(&ipsg_s_mac.ip_addr, ip_addr, sizeof(prefix_t));
    sal_memcpy(ipsg_s_mac.mac_addr, mac, sizeof(mac_addr_t));
    sal_memcpy(ipsg_s_mac.ifname, ifname, sizeof(ipsg_s_mac.ifname));
    ipsg_s_mac.ifindex = if_index;
    ipsg_s_mac.vid=  vlan_id;

    p_db_ipsg_s_mac = tbl_ipsg_s_mac_get_ipsg_s_mac(&ipsg_s_mac.key);
    if (NULL == p_db_ipsg_s_mac)
    {
        rc = tbl_ipsg_s_mac_add_ipsg_s_mac(&ipsg_s_mac);
    }

    sal_memset(&ipsg_s_ip, 0, sizeof(ipsg_s_ip));

    sal_memcpy(&ipsg_s_ip.key.ip, ip_addr, sizeof(prefix_t));
    sal_memcpy(&ipsg_s_ip.ip_addr, ip_addr, sizeof(prefix_t));
    sal_memcpy(ipsg_s_ip.mac_addr, mac, sizeof(mac_addr_t));
    sal_memcpy(ipsg_s_ip.ifname, ifname, sizeof(ipsg_s_ip.ifname));
    ipsg_s_ip.ifindex = if_index;
    ipsg_s_ip.vid=  vlan_id;

    p_db_ipsg_s_ip = tbl_ipsg_s_ip_get_ipsg_s_ip(&ipsg_s_ip.key);
    if (NULL == p_db_ipsg_s_ip)
    {
        rc = tbl_ipsg_s_ip_add_ipsg_s_ip(&ipsg_s_ip);
    }
    
    return rc;
}

int32
ipsg_cmd_del_db_entry(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type)
{
    tbl_ipsg_s_mac_t    *p_db_ipsg_s_mac = NULL;
    tbl_ipsg_s_mac_t    ipsg_s_mac;
    tbl_ipsg_s_ip_t     *p_db_ipsg_s_ip = NULL;
    tbl_ipsg_s_ip_t     ipsg_s_ip;
    int32               rc = IPSOUR_OK;

    sal_memset(&ipsg_s_mac, 0, sizeof(ipsg_s_mac));

    sal_memcpy(ipsg_s_mac.key.mac, mac, sizeof(mac_addr_t));
    sal_memcpy(&ipsg_s_mac.ip_addr, ip_addr, sizeof(prefix_t));
    sal_memcpy(ipsg_s_mac.mac_addr, mac, sizeof(mac_addr_t));
    sal_memcpy(ipsg_s_mac.ifname, ifname, sizeof(ipsg_s_mac.ifname));
    ipsg_s_mac.ifindex = if_index;
    ipsg_s_mac.vid=  vlan_id;

    p_db_ipsg_s_mac = tbl_ipsg_s_mac_get_ipsg_s_mac(&ipsg_s_mac.key);
    if (NULL != p_db_ipsg_s_mac)
    {
        rc = tbl_ipsg_s_mac_del_ipsg_s_mac(&ipsg_s_mac.key);
    }

    sal_memset(&ipsg_s_ip, 0, sizeof(ipsg_s_ip));

    sal_memcpy(&ipsg_s_ip.key.ip, ip_addr, sizeof(prefix_t));
    sal_memcpy(&ipsg_s_ip.ip_addr, ip_addr, sizeof(prefix_t));
    sal_memcpy(ipsg_s_ip.mac_addr, mac, sizeof(mac_addr_t));
    sal_memcpy(ipsg_s_ip.ifname, ifname, sizeof(ipsg_s_ip.ifname));
    ipsg_s_ip.ifindex = if_index;
    ipsg_s_ip.vid=  vlan_id;

    p_db_ipsg_s_ip = tbl_ipsg_s_ip_get_ipsg_s_ip(&ipsg_s_ip.key);
    if (NULL != p_db_ipsg_s_ip)
    {
        rc = tbl_ipsg_s_ip_del_ipsg_s_ip(&ipsg_s_ip.key);
    }

    return rc;
}

int32
ipsg_cmd_add_static_binding_callback(tbl_ipsg_fib_t *p_ipsg_fib, tbl_interface_t *p_db_if)
{
    tbl_brg_global_t    *p_db_glb   = tbl_brg_global_get_brg_global();
    int32               iRet        = 0;
    char                buf[CMD_BUF_256];
    char                ifname_ext[IFNAME_EXT_SIZE];

     /* input check */
    if ( NULL == p_ipsg_fib || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_ipsg_fib->ifname, ifname_ext);

     /* check if this item is exist in static binding table */
     iRet = ipsour_static_add_check_static(p_ipsg_fib);
     if (IPSOUR_DUPLICATE_MAC == iRet)
     {
         return IPSOUR_DUPLICATE_MAC;
     }
     else if (IPSOUR_DUPLICATE_IP == iRet)
     {
         return IPSOUR_DUPLICATE_IP;
     }
     else if (IPSOUR_NULL_POINTER == iRet)
     {
         return IPSOUR_NULL_POINTER;
     }

     /* check if this item is exist in dhcpsnooping binding table and add to ip source guard 
     ** for bug 19695 modified by liuyang 2012-8-27
     */

     /* need to check by wangjj */
     iRet = ipsour_static_add_check_dynamic(p_ipsg_fib, p_db_if->ip_source_type);
     if (IPSOUR_DUPLICATE_MAC == iRet)
     {
         return IPSOUR_DUPLICATE_MAC;
     }
     else if (IPSOUR_DUPLICATE_IP == iRet)
     {
         return IPSOUR_DUPLICATE_IP;
     }
     else if (IPSOUR_NULL_POINTER == iRet)
     {
         return IPSOUR_NULL_POINTER;
     }

     if (0 != p_db_glb->ipsg_max_port_binding)
     {
         /* number is reach max */
         if (p_db_glb->ipsg_max_port_binding <= p_db_if->ip_source_item_number)
         {
             return IPSOUR_REACH_PORT_MAX;
         }
     }

    if(AF_INET == p_ipsg_fib->ip_addr.family)
    {
        /* need check by wangjj */
        //if (nm->cur_info.ipv4_source_guard >= nm->sys_info.ipv4_source_guard)
        if (ipsour_ipsg_count_get() >= ipsour_ipsg_count_get_max())    
        {
            IPSOUR_DBG_ERR("IP source guard binding reach total max when adding dhcpsnooping binding item\n");
            return IPSOUR_REACH_TOTAL_MAX;
        }
    }

    IPSOUR_DEBUG(IPSG_EVENT,"CLI add item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s %s \n", 
       p_ipsg_fib->vid,
       p_ipsg_fib->mac_addr[0], p_ipsg_fib->mac_addr[1],
       p_ipsg_fib->mac_addr[2], p_ipsg_fib->mac_addr[3],
       p_ipsg_fib->mac_addr[4], p_ipsg_fib->mac_addr[5],
       cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_ipsg_fib->ip_addr.u.prefix4), ifname_ext);

     if(p_ipsg_fib->ip_addr.family == AF_INET)
     {
         /* call function to add to tcam */
         iRet = ipsour_cmd_add_item(&p_ipsg_fib->ip_addr, p_ipsg_fib->mac_addr, p_ipsg_fib->vid, 
                    p_db_if->ifindex, p_ipsg_fib->ifname, p_db_if->ip_source_type);

         if (IPSOUR_OK == iRet)
         {
            ipsg_cmd_add_db_entry(&p_ipsg_fib->ip_addr, p_ipsg_fib->mac_addr, p_ipsg_fib->vid, 
                                    p_db_if->ifindex, p_ipsg_fib->ifname, p_db_if->ip_source_type);
         }         
     }

     if (IPSOUR_OK == iRet)
     {
        if(p_ipsg_fib->ip_addr.family == AF_INET)
        {
            ipsour_ipsg_count_inc();
        }
        
        p_db_if->ip_source_item_number++;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER);
     }

    return iRet;
 }

int32
ipsg_cmd_del_static_binding_callback(tbl_ipsg_fib_t *p_ipsg_fib, tbl_interface_t *p_db_if, uint32 show_log)
{
    int32   iRet = 0;
    char    buf[CMD_BUF_256];
    char    ifname_ext[IFNAME_EXT_SIZE];

    /* input check */
    if ( NULL == p_ipsg_fib || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_ipsg_fib->ifname, ifname_ext);

     /* if it is existent in static item hash table*/
     if (IPSOUR_DUPLICATE == ipsour_static_del_check_static(p_ipsg_fib))
     {
         if(p_ipsg_fib->ip_addr.family == AF_INET)
         {
            IPSOUR_DEBUG(IPSG_EVENT,"CLI delete item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s %s \n", 
               p_ipsg_fib->vid,
               p_ipsg_fib->mac_addr[0], p_ipsg_fib->mac_addr[1],
               p_ipsg_fib->mac_addr[2], p_ipsg_fib->mac_addr[3],
               p_ipsg_fib->mac_addr[4], p_ipsg_fib->mac_addr[5],
               cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_ipsg_fib->ip_addr.u.prefix4), ifname_ext);
       
             /* call function to del from tcam */
             iRet = ipsour_cmd_del_item(&p_ipsg_fib->ip_addr, p_ipsg_fib->mac_addr, 
                 p_ipsg_fib->vid, p_db_if->ifindex, p_ipsg_fib->ifname, p_db_if->ip_source_type);
 
             if (IPSOUR_OK == iRet)
             {
                ipsg_cmd_del_db_entry(&p_ipsg_fib->ip_addr, p_ipsg_fib->mac_addr, 
                    p_ipsg_fib->vid, p_db_if->ifindex, p_ipsg_fib->ifname, p_db_if->ip_source_type);
             }  
         }
         
         if (IPSOUR_OK == iRet)
         {
             if(p_ipsg_fib->ip_addr.family == AF_INET)
             {
                 /*  need to check by wangjj */
                 if (ipsour_ipsg_count_get() > 0)
                 {
                     //nm->cur_info.ipv4_source_guard--;
                     ipsour_ipsg_count_dec();
                 }
             }
            
             if (p_db_if->ip_source_item_number > 0)
             {
                p_db_if->ip_source_item_number--;
                tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER);
             }
         }
 
         return iRet;
     }
     else
     {
        if (show_log)
        {
            IPSOUR_DBG_ERR("No such static item existent in ip source guard table\n");         
            return IPSOUR_NO_SUCH_ITEM;
         }
         else
         {
            return PM_E_NONE;
         }
     }
}

int32
ipsg_cmd_clear_ip_source_guard_entry_one(tbl_ipsg_fib_t *p_db_ipsg_fib)
{
    tbl_interface_t         *p_db_if            = NULL;
    tbl_ipsg_fib_t          ipsg_fib;

    if (NULL == p_db_ipsg_fib)
    {
        return PM_E_FAIL;
    }
    
    sal_memcpy(&ipsg_fib, p_db_ipsg_fib, sizeof(ipsg_fib));    
    p_db_if = tbl_interface_get_interface_by_name(p_db_ipsg_fib->ifname);
    if (NULL != p_db_if)
    {
        ipsg_cmd_del_static_binding_callback(&ipsg_fib, p_db_if, FALSE);
    }
    
    return PM_E_NONE;
}

int32
ipsg_cmd_clear_ip_source_guard_entry(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ipsg_fib_t          *p_db_ipsg_fib      = obj;
    tbl_ipsg_fib_t          ipsg_fib;
    char    ifname[IFNAME_SIZE];
    vid_t   vlanid;

    sal_memset(&ipsg_fib, 0, sizeof(ipsg_fib));
    sal_memset(ifname, 0, sizeof(ifname));
    if (NULL != pargs->argv[0] && NULL == pargs->argv[1])
    {
        sal_memcpy(ifname, pargs->argv[0], sizeof(ifname));
        if (0 != sal_strcmp(ifname, p_db_ipsg_fib->ifname))
        {
            return PM_E_NONE;
        }

        return ipsg_cmd_clear_ip_source_guard_entry_one(p_db_ipsg_fib);
    }

    if (NULL == pargs->argv[0] && NULL != pargs->argv[1])
    {
        vlanid =  *((vid_t *)pargs->argv[1]);
        if (vlanid != p_db_ipsg_fib->vid)
        {
            return PM_E_NONE;
        }

        return ipsg_cmd_clear_ip_source_guard_entry_one(p_db_ipsg_fib);
    }

    if (NULL == pargs->argv[0] && NULL == pargs->argv[1])
    {
        return ipsg_cmd_clear_ip_source_guard_entry_one(p_db_ipsg_fib);
    }
    
    return PM_E_NONE;
}


int32
ipsg_cmd_clear_ip_source_guard_all()
{
    tbl_iter_args_t args;
    args.argv[0] = NULL;
    args.argv[1] = NULL;    

    tbl_ipsg_fib_iterate((TBL_ITER_CB_FUNC)ipsg_cmd_clear_ip_source_guard_entry, &args);

    return PM_E_NONE;
}

int32
ipsg_cmd_clear_ip_source_guard_by_vlan(vid_t vlanid)
{
    tbl_iter_args_t args;
    args.argv[0] = NULL;    
    args.argv[1] = &vlanid;

    tbl_ipsg_fib_iterate((TBL_ITER_CB_FUNC)ipsg_cmd_clear_ip_source_guard_entry, &args);

    return PM_E_NONE;
}


int32
ipsg_cmd_clear_ip_source_guard_by_interface(char *if_name)
{
    tbl_iter_args_t args;
    args.argv[0] = if_name;
    args.argv[1] = NULL;    

    tbl_ipsg_fib_iterate((TBL_ITER_CB_FUNC)ipsg_cmd_clear_ip_source_guard_entry, &args);

    return PM_E_NONE;
}

int32
ipsg_cmd_mac_addr_str2val(uint8 *mac, const char *str, cfg_cmd_para_t *para)
{
    char    *mac_str    = NULL;
    char    buf[64];
    int32   rc = PM_E_NONE;

    sal_memset(buf, 0, sizeof(buf));
    sal_strncpy(buf, str, 64);
    mac_str = buf;

    rc = cdb_arp_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("%%Mac address is invalid!");
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        CFG_INVALID_PARAM_RET("%%Mac address is invalid!");
    }
    
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);
    
    return PM_E_NONE;
}


int32
ipsg_cmd_process_ip_source_guard_data_trans(char **argv, int32 argc, cfg_cmd_para_t *para, 
            prefix_t *prefix, vid_t *vlan_id, uint8 *macaddr, uint32 *ifindex, char *ifname)
{
    int32               retv;
    char                buf[64];
    mac_addr_t          mac;    
    int32               rc = PM_E_NONE;
    char                ipsg_ifname[IFNAME_SIZE];
    tbl_interface_t     *p_db_if = NULL;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(ipsg_ifname, 0, sizeof(ipsg_ifname));
    if (5 == argc)
    {
        retv = sal_inet_pton(AF_INET, argv[3], buf);   
        if (!retv)                                       
        {                                                
            CFG_INVALID_PARAM_RET("Invalid cli IP address");
            return PM_E_INVALID_PARAM; 
        }

        if (0 && 0 == sal_strncmp(argv[1], "ffff.ffff.ffff", sizeof("ffff.ffff.ffff")))
        {
             return PM_E_INVALID_PARAM; 
        }
        
        sal_memcpy(&prefix->u.prefix4, buf, sizeof(addr_ipv4_t));
        prefix->family = AF_INET;
        rc = ipsg_cmd_mac_addr_str2val(mac, argv[1], para);
        sal_memcpy(macaddr, mac, sizeof(mac));

        *vlan_id = sal_atoi(argv[2]);
        sal_strcpy(ipsg_ifname, argv[4]);
        sal_strcpy(ifname, argv[4]);
        p_db_if = tbl_interface_get_interface_by_name(ipsg_ifname);
        if (NULL != p_db_if)
        {
            *ifindex = p_db_if->ifindex;
        }
    }
    else 
    {
        CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
    }    
    

    return rc;
}

int32
ipsour_get_verify_type(tbl_interface_t *p_db_if, char *mode)
{
    if (NULL != p_db_if)
    {
        switch (p_db_if->ip_source_type)
        {
            case IP_NONE:
                sal_snprintf(mode, M_TYPE_STR, "Disabled");
                break;
            case IP_FILTERING:
                sal_snprintf(mode,M_TYPE_STR, "ip");
                break;
            case IP_MAC_FILTERING:
                sal_snprintf(mode, M_TYPE_STR, "ip-mac");
                break;
            case IP_VLAN_FILTERING:
                sal_snprintf(mode, M_TYPE_STR, "ip-vlan");
                break;
            case IP_MAC_VLAN_FILTERING:
                sal_snprintf(mode, M_TYPE_STR,"ip-mac-vlan");
                break;
             default:
                 sal_snprintf(mode, M_TYPE_STR, "None");
                 break;
        }
    }
    else
    {
        sal_snprintf(mode, M_TYPE_STR, "None");
    }
    
    return PM_E_NONE;
}

int32
ipsg_cmd_show_dynamic_ipsg_one(void *obj, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_db_if = NULL;
    FILE            *fp = pargs->argv[0];
    tbl_dhcbinding_t    *p_db_dhcbinding = obj;    
    char    ifname_ext[IFNAME_EXT_SIZE];
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];
    char    mode[CMD_BUF_128];

    sal_memset(mode, 0, sizeof(mode));    
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));

    if (DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR == p_db_dhcbinding->cIpsour)
    {
        /* unready item */
        return PM_E_NONE;
    }

    p_db_if = tbl_interface_get_interface_by_name(p_db_dhcbinding->ifname);
    if (NULL != p_db_if)
    {
        ipsour_get_verify_type(p_db_if, mode);
    }
    else
    {
        return PM_E_NONE;
    }

    IFNAME_ETH2FULL(p_db_dhcbinding->ifname, ifname_ext);

    sal_fprintf(fp, "%-4u %12s %-8s %-10s %-13s %s\n",
        p_db_dhcbinding->vid,
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_db_dhcbinding->client_mac),
        "dynamic",
        ifname_ext, 
        mode,
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_dhcbinding->client_ip.u.prefix4));

    return PM_E_NONE;
    
}

int32
ipsg_cmd_show_static_ipsg_one(void *obj, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_db_if = NULL;
    FILE            *fp = pargs->argv[0];
    tbl_ipsg_s_mac_t  *p_db_ipsg_mac = obj;    
    char    ifname_ext[IFNAME_EXT_SIZE];
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];
    char    mode[CMD_BUF_128];

    sal_memset(mode, 0, sizeof(mode));    
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));

    p_db_if = tbl_interface_get_interface_by_name(p_db_ipsg_mac->ifname);
    if (NULL != p_db_if)
    {
        ipsour_get_verify_type(p_db_if, mode);
    }
    else
    {
        return PM_E_NONE;
    }

    IFNAME_ETH2FULL(p_db_ipsg_mac->ifname, ifname_ext);

    sal_fprintf(fp, "%-4u %12s %-8s %-10s %-13s %s\n",
        p_db_ipsg_mac->vid,
        cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_db_ipsg_mac->mac_addr),
        "static",
        ifname_ext, 
        mode,
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_ipsg_mac->ip_addr.u.prefix4));

    return PM_E_NONE;
    
}

int32
ipsg_cmd_show_dynamic_ipsg_by_interface(void *obj, tbl_iter_args_t *pargs)
{
    tbl_dhcbinding_t    *p_db_dhcbinding = obj;
    char                *ifname = pargs->argv[1];
    int32               rc  = PM_E_NONE;

    if (0 == sal_strncmp(p_db_dhcbinding->ifname, ifname, sizeof(p_db_dhcbinding->ifname)))
    {
        rc = ipsg_cmd_show_dynamic_ipsg_one(obj, pargs);
    }

    return rc;
}

int32
ipsg_cmd_show_static_ipsg_by_interface(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ipsg_s_mac_t *p_db_ipsg_mac = obj;
    char            *ifname = pargs->argv[1];
    int32           rc  = PM_E_NONE;

    if (0 == sal_strncmp(p_db_ipsg_mac->ifname, ifname, sizeof(p_db_ipsg_mac->ifname)))
    {
        rc = ipsg_cmd_show_static_ipsg_one(obj, pargs);
    }

    return rc;
}

uint32
ipsg_cmd_get_ipv4_max_count()
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

int32
ipsg_cmd_show_ipsg_entry(char *ifname, cfg_cmd_para_t *para)
{
    tbl_brg_global_t        *p_brg_glb  = tbl_brg_global_get_brg_global();
    tbl_ipsg_fib_master_t   *p_db_ipsg  = tbl_ipsg_fib_get_master();
    FILE                    *fp         = NULL;
    int32                   rc          = PM_E_NONE;
    tbl_iter_args_t         args;
    uint32                  ipsg_count_max = 0;
    
    if(NULL == p_brg_glb || NULL == p_db_ipsg)
    {
        return PM_E_FAIL;
    }
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;

    if (ipsg_cmd_get_ipv4_max_count())
    {
        ipsg_count_max = ipsg_cmd_get_ipv4_max_count();
    }
    
    sal_fprintf(fp, "The total number of ip binding is %d, the max ip number limit is %d\n",
         p_db_ipsg->ipsg_fib_hash->count, ipsg_count_max);
    sal_fprintf(fp, "IP source guard binding table:\n");
    sal_fprintf(fp, "VLAN MAC Address    Type     Interface  State         IP Address\n");
    sal_fprintf(fp, "================================================================================\n");

    if (NULL == ifname)
    {
        rc = tbl_dhcbinding_iterate(ipsg_cmd_show_dynamic_ipsg_one, &args);
        rc = tbl_ipsg_s_mac_iterate(ipsg_cmd_show_static_ipsg_one, &args);
    }
    else
    {
        args.argv[1] = ifname;
        rc = tbl_dhcbinding_iterate(ipsg_cmd_show_dynamic_ipsg_by_interface, &args);
        rc = tbl_ipsg_s_mac_iterate(ipsg_cmd_show_static_ipsg_by_interface, &args);
    }

    sal_fclose(fp);
    return rc;
}


static int32
_ipsg_cmd_get_ipsg(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ipsg_fib_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_ipsg_cmd_get_ipsg_mac(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ipsg_s_mac_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_ipsg_cmd_get_ipsg_ip(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ipsg_s_ip_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
ipsg_cmd_process_ip_source_guard(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    char            sys_cmd[128];
    char            if_name[IFNAME_SIZE];
    tbl_brg_global_t *p_db_glb = tbl_brg_global_get_brg_global();
    tbl_interface_t *p_db_if = NULL;
    tbl_ipsg_fib_t  ipsg_fib;
    prefix_t    prefix;
    vid_t       vlan_id;
    mac_addr_t  mac_addr;    
    uint32      ifindex;
    int32       rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    sal_memset(&ipsg_fib, 0, sizeof(tbl_ipsg_fib_t));
    sal_memset(if_name,0, sizeof(if_name));
    sal_memset(&prefix, 0, sizeof(prefix_t));
    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    p_node = cdb_get_tbl(TBL_IPSG_FIB);
    if (NULL == p_db_glb)
    {
        return PM_E_FAIL;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (5 == argc)
        {
            if (0 == sal_strcmp(argv[0], "add_static"))
            {
                rc = ipsg_cmd_process_ip_source_guard_data_trans(argv, argc, para, &prefix,  &vlan_id, mac_addr, &ifindex, if_name);
                if (rc)    
                {
                    CFG_INVALID_PARAM_RET("Please check configuration!");
                }
                
                ipsg_fib.ifindex = ifindex;
                ipsg_fib.key.fid = vlan_id;
                ipsg_fib.vid = vlan_id;
                sal_memcpy(ipsg_fib.key.mac, mac_addr, sizeof(mac_addr_t));
                sal_memcpy(ipsg_fib.mac_addr, mac_addr, sizeof(mac_addr_t));
                sal_memcpy(&ipsg_fib.ip_addr, &prefix,sizeof(prefix));
                sal_memcpy(ipsg_fib.ifname, if_name, sizeof(if_name));

                p_db_if = tbl_interface_get_interface_by_name(if_name);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("No such interface or not supported ip source binding");
                }
                else if (0 && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type) && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
                {
                    CFG_INVALID_PARAM_RET("This port has been added to a aggregation port");
                }
                
                rc = ipsg_cmd_add_static_binding_callback(&ipsg_fib, p_db_if);
                if (IPSOUR_DUPLICATE_MAC == rc)
                {
                    CFG_INVALID_PARAM_RET("The MAC is existent in ip source guard binding table");
                }

                if (IPSOUR_DUPLICATE_IP == rc)
                {
                    CFG_INVALID_PARAM_RET("The IP is existent in ip source guard binding table");
                }

                if (IPSOUR_REACH_PORT_MAX == rc)
                {
                    CFG_INVALID_PARAM_RET("The ip source binding item on a port has reached maximal number: %d", p_db_glb->ipsg_max_port_binding);
                }

                if (IPSOUR_REACH_TOTAL_MAX == rc)
                {
                    CFG_INVALID_PARAM_RET(" The total ip source binding item has reached maximal number: %d", ipsour_ipsg_count_get_max());
                }                
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter!");
        }
        
        break;

    case CDB_OPER_DEL:
        if (5 == argc)
        {
            if (0 == sal_strcmp(argv[0], "del_static"))
            {
                rc = ipsg_cmd_process_ip_source_guard_data_trans(argv, argc, para, &prefix,  &vlan_id, mac_addr, &ifindex, if_name);
                ipsg_fib.ifindex = ifindex;
                ipsg_fib.key.fid = vlan_id;
                ipsg_fib.vid = vlan_id;
                sal_memcpy(ipsg_fib.key.mac, mac_addr, sizeof(mac_addr_t));
                sal_memcpy(ipsg_fib.mac_addr, mac_addr, sizeof(mac_addr_t));
                sal_memcpy(&ipsg_fib.ip_addr, &prefix, sizeof(prefix));
                sal_memcpy(ipsg_fib.ifname, if_name, sizeof(if_name));

                p_db_if = tbl_interface_get_interface_by_name(if_name);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("No such interface or not supported ip source binding");
                }
                
                rc = ipsg_cmd_del_static_binding_callback(&ipsg_fib, p_db_if, TRUE);
                if (IPSOUR_NO_SUCH_ITEM == rc)
                {
                    CFG_INVALID_PARAM_RET("No such item in ip source guard binding table");
                }
            }
        }
        else if (4 == argc)
        {
            if (0 == sal_strcmp(argv[0], "clear_ipsg"))
            {
                if (0 == sal_strcmp(argv[1], ""))
                {
                    sal_strcpy(if_name, argv[2]);
                    rc = ipsg_cmd_clear_ip_source_guard_by_interface(if_name);
                }

                if (0 == sal_strcmp(argv[2], ""))
                {
                    vlan_id = sal_atoi(argv[1]);
                    rc = ipsg_cmd_clear_ip_source_guard_by_vlan(vlan_id);
                }

                if (0 == sal_strcmp(argv[1], "all"))
                {
                    rc = ipsg_cmd_clear_ip_source_guard_all();
                }
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter!");
        }
        
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:       
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _ipsg_cmd_get_ipsg(para, p_node, &field);

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
ipsg_cmd_process_ipsg_mac(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    int32           rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IPSG_S_MAC);

    switch (para->oper)
    {
    case CDB_OPER_ADD:

        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:

        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _ipsg_cmd_get_ipsg_mac(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
ipsg_cmd_process_ipsg_ip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    int32           rc = PM_E_NONE;
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IPSG_S_IP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:

        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:

        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _ipsg_cmd_get_ipsg_ip(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

/* show ip source binding ...*/
int32
ipsg_cmd_process_ipsg_dhcs(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char            if_name[IFNAME_SIZE];
    int32           rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (2 == argc)
        {
            if (0  == sal_strcmp(argv[0], "interface"))
            {
                sal_memset(if_name, 0, sizeof(if_name));
                sal_strcpy(if_name, argv[1]);
                ipsg_cmd_show_ipsg_entry(if_name, para);
            }

            return rc;
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "all"))
            {
                ipsg_cmd_show_ipsg_entry(NULL, para);
            }

            return rc;
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;   
}



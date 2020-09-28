/****************************************************************************
* $Id: dhcrelay6_func.c,v 1.8 2007/09/03 04:13:09 xliu Exp $
*  process functions for dhcpv6 relay
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-09 18:35
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "dhcpd.h"

#include <ifaddrs.h>

#include "ctclib_hash.h"
#include "dhcrelay6_func.h"
#include "dhcrelay_snmp.h"
#include "genlog.h"
#ifdef _CENTEC_  /* Added by Percy Wang for optimize BHM module, 2008-07-17 */
#ifdef HAVE_BM            
#include "bmlib.h"
#endif
#endif       
/* for index to name*/
#include <string.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#undef DHCPRELAY6_DEBUG 

#ifndef NSM_IF_TYPE_L2
#define NSM_IF_TYPE_L2        1
#endif

typedef void (*HASH_CLEAN_FUNC) (void *);
extern unsigned int g_nOrigIfIdx;

#define DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pdclient, addr) \
    do \
    {\
        if ((addr)->max_life != 0xffffffff)\
        {\
            if (!(pdclient)->expire)   \
            { \
                (pdclient)->expire = (addr)->starts + (addr)->max_life; \
            }  \
            else\
            {\
                (pdclient)->expire = ((pdclient)->expire < ((addr)->starts + (addr)->max_life)) ? \
                            (pdclient)->expire : ((addr)->starts + (addr)->max_life);       \
            }\
        }\
    } while (0)

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
/* dhcpv6 service switch */
int g_dhcprelay6Switch = 0x00;
static u_int8_t g_pdroute_timeout_running = 0;

u_int16_t g_dhcprelay6_remoteid_format = DHCPRELAY6_REMOTE_ID_KEYWORD_DEFAULT;
u_int8_t g_dhcprelay6_pdroute_distance = DHCPRELAY6_PDROUTE_DISTANCE_DEFAULT;

u_int32_t g_dhcprelay6_pdclient_time_interval = DHCPRELAY6_PDCLIENT_TIME_INTERVAL_DEFAULT;
u_int32_t g_dhcprelay6_pdroute_time_interval = DHCPRELAY6_PDROUTE_TIME_INTERVAL_DEFAULT;

/* LIST */
S_Dhcpv6PD g_dhcprelay6_pdclient;
S_Dhcpv6PDRoute g_dhcprelay6_pdroute;

/* Relay DUID */
static struct data_string g_relay_duid;
/* for remote-id */
static u_int32_t g_enterprise_id;

/*
 * DUID layout, as defined in RFC 3315, section 9.
 * use type 3 (hardware address) DUID.
static int g_relay_duid_type = DUID_LL;
 */
S_Dhcpv6Relay g_dhcprelay6Cfg; 
int g_dhcprelay6_initialized = 0; /* dhcpv6 relay initialized flag */
ctclib_hash_t *g_dhcprelay6Intfhash = NULL; /* only hold L3 interfaces */

extern omapi_io_object_t omapi_io_states;
extern int timeval_cmp (struct timeval a, struct timeval b);

struct interface_info *dhcprelay6_register_interface(S_Dhcpv6RelayInterface *pItem);
void dhcprelay6_unregister_interface(char *name);
struct interface_info *dhcprelay6_locate_interface(char *name);
struct interface_info *dhcprelay6_locate_interface_by_id(int id);

S_Dhcpv6RelayInterface *dhcprelay6_intf_calloc(void);
void dhcprelay6_intf_free(S_Dhcpv6RelayInterface *pItem);
void dhcprelay6_intf_hash_add(ctclib_hash_t *pHash, S_Dhcpv6RelayInterface *pIntf);
void dhcprelay6_intf_hash_del(ctclib_hash_t *pHash, S_Dhcpv6RelayInterface *pIntf);
void dhcprelay6_do_register_interface(ctclib_hash_backet_t *hb, void *arg);
void dhcprelay6_intf_hash_finish (ctclib_hash_t *pHash);
void dhcprelay6_pdclient_timeout(void *arg);
void dhcprelay6_pdclient_del_wait(void *arg);
int dhcprelay6_serveritem_del_byintf(char *name);
/****************************************************************************
 *
* Function
* 
****************************************************************************/

/*******************************************************************************
 * Name:    dhcprelay6_stop
 * Purpose: shutdown dhcprelay6 PM
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_stop(void)
{
    dhcprelay6_module_free();
    /* Stop the system. */
    return;
}

/*******************************************************************************
 * Name:    sighup 
 * Purpose: SIGHUP handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sighup (int sig)
{
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "SIGHUP received");
}

/*******************************************************************************
 * Name:    sigint 
 * Purpose: SIGINT handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sigint (int sig)
{
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "SIGINT received");
    
    dhcprelay6_stop ();
    exit(0);
}

/*******************************************************************************
 * Name:    sigusr1 
 * Purpose: SIGUSR1 handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sigusr1 (int sig)
{
}

/*******************************************************************************
 * Name:    signal_init 
 * Purpose: Initialize signal handles
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_signal_init ()
{
    signal(SIGHUP, sighup);
    signal(SIGINT, sigint);
    signal(SIGTERM, sigint);
    signal(SIGUSR1, sigusr1);
}

#ifdef DHCPRELAY6_DEBUG
/*******************************************************************************
 * Name:    dhcprelay6_debug_show_interface
 * Purpose: show this interface's status
 * Input: 
 *   hb: 
 *   arg: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_debug_show_interface(ctclib_hash_backet_t *hb, void *arg)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;

    if (NULL == hb)
    {
        return;
    }
    
    pIntf = (S_Dhcpv6RelayInterface *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }
    fprintf(stderr, "--debug dhcprelay6 interface %s group %lu ifindex %lu\n",
               pIntf->szName, pIntf->nGroupIdx, pIntf->nIfIndex);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_debug_show_interfaces
 * Purpose: show all interfaces current status
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_debug_show_interfaces(void)
{
    ctclib_hash_iterate(g_dhcprelay6Intfhash, dhcprelay6_debug_show_interface, NULL);
    return;
}
#endif

/*******************************************************************************
 * Name:    dhcprelay6_do_unregister_interface
 * Purpose: unregister interface for dhcpv6 relay
 * Input: 
 *   hb: hash backet 
 *   arg: callback argument
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_do_unregister_interface(ctclib_hash_backet_t *hb, void *arg)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;
    
    if (NULL == hb)
    {
        return;
    }

    /* all interfaces in hash is L3 interface, so does't care it here */
    pIntf = (S_Dhcpv6RelayInterface *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }

    dhcprelay6_unregister_interface(pIntf->szName);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_do_register_interface
 * Purpose: register interface for dhcpv6 relay
 * Input: 
 *   hb: hash backet
 *   arg: callback argument
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_do_register_interface(ctclib_hash_backet_t *hb, void *arg)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;
    
    if (NULL == hb)
    {
        return;
    }

    if (!DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)))
    {
        return;
    }
    
    /* all interfaces in hash is L3 interface, so does't care it here */
    pIntf = (S_Dhcpv6RelayInterface *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }
    
    if (pIntf->up && pIntf->nGroupIdx > 0)
    {
        dhcprelay6_register_interface(pIntf);
    }
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_service_onoff
 * Purpose: process service on/off
 * Input: 
 *   bOn: On or off flag
 * Output: 
 * Return:
 *   success: 0
 *   failed:  -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_service_onoff(unsigned char bOn)
{
    static int g_relay_service_on = 0;

    if (bOn)
    {
        if (g_relay_service_on)
        {
            return 0;
        }

        /* service on */
        g_relay_service_on = 1;
        ctclib_hash_iterate(g_dhcprelay6Intfhash, dhcprelay6_do_register_interface, NULL);
        DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS,
                  "initialize DHCPv6 relay interfaces success");
        log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "turn DHCPv6 relay function on");
        return 0;
    }

    /* service off */
    if (!g_relay_service_on)
    {
        return 0;
    }
    g_relay_service_on = 0;
    ctclib_hash_iterate(g_dhcprelay6Intfhash, dhcprelay6_do_unregister_interface, NULL);
    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS,
              "deinitialize DHCPv6 relay interfaces success");
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "turn DHCPv6 relay function off");
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_get_intf_by_ifindex
 * Purpose: find interface by ifindex
 * Input: 
 *   ifindex: interface index
 * Output: 
 * Return:
 *   success: interface pointer
 *   fail:    NULL
 * Note: 
 ******************************************************************************/
S_Dhcpv6RelayInterface *dhcprelay6_get_intf_by_ifindex(unsigned long ifindex)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface *pTmp = NULL;
    ctclib_hash_t *hash = g_dhcprelay6Intfhash;
    ctclib_hash_backet_t *hb = NULL;
    int i;

    if (NULL == hash)
    {
        return NULL;
    }

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = (S_Dhcpv6RelayInterface *)hb->data;
            if (ifindex == pTmp->nIfIndex)
            {
                pIntf = pTmp;
                break;
            }
        }
    }
    
    return pIntf;
}

S_Dhcpv6RelayInterface *dhcprelay6_get_intf_by_name(char *ifname)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface *pTmp = NULL;
    ctclib_hash_t *hash = g_dhcprelay6Intfhash;
    ctclib_hash_backet_t *hb = NULL;
    int i;

    if (NULL == hash || NULL == ifname)
    {
        return NULL;
    }

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = (S_Dhcpv6RelayInterface *)hb->data;
            if (!strncmp(ifname, pTmp->szName, INTERFACE_NAMSIZ))
            {
                pIntf = pTmp;
                break;
            }
        }
    }
    
    return pIntf;
}

void
dhcprelay6_leave_multicast_group(struct interface_info *info)
{
    struct ipv6_mreq mreq;

    if (info->rfdesc <= 0)
    {
        return;
    }
    memset (&mreq, 0, sizeof (struct ipv6_mreq));
    inet_pton(AF_INET6, All_DHCP_Relay_Agents_and_Servers, &mreq.ipv6mr_multiaddr);
    mreq.ipv6mr_interface = if_nametoindex(info->name);;

    if (setsockopt (info->rfdesc, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (char *) &mreq,
            sizeof (struct ipv6_mreq)) < 0)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "setsockopt: IPV6_LEAVE_GROUP: %s, for interface : %s",
                strerror(errno), info->name);
    }
    return;
}

void
dhcprelay6_join_multicast_group(struct interface_info *info)
{
    struct ipv6_mreq mreq;

    if (info->rfdesc <= 0)
    {
        return;
    }
    memset (&mreq, 0, sizeof (struct ipv6_mreq));
    inet_pton(AF_INET6, All_DHCP_Relay_Agents_and_Servers, &mreq.ipv6mr_multiaddr);
    mreq.ipv6mr_interface = if_nametoindex(info->name);;

    if (setsockopt (info->rfdesc, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq,
            sizeof (mreq)) < 0)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "setsockopt: IPV6_JOIN_GROUP: %s, for interface : %s",
                strerror(errno), info->name);
    }
    return;
}

#if 0
/*******************************************************************************
 * Name:    dhcprelay6_recv_ipaddr_change
 * Purpose: interface ip addr change callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
int
dhcprelay6_recv_ipaddr_change(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    struct nsm_msg_address *pmsg = (struct nsm_msg_address *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    struct interface_info *pInfo = NULL;
    u_int8_t bJoined = 0;

    pIntf = dhcprelay6_get_intf_by_ifindex(pmsg->ifindex);
    if (NULL == pIntf)
    {
        return RESULT_OK;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Received ipaddr change message for interface : %s\n", pIntf->szName);
    
    pInfo = dhcprelay6_locate_interface(pIntf->szName);
    if (pInfo)
    {
        if (pInfo->v6address_count > 0)
        {
            bJoined = 1;
        }
        dhcprelay6_update_primary_address(pInfo);
        
        if (bJoined && pInfo->v6address_count <= 0)
        {
            dhcprelay6_leave_multicast_group(pInfo);
        }
        else if (!bJoined && pInfo->v6address_count > 0)
        {
            dhcprelay6_join_multicast_group(pInfo);            
        }
    }
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhcprelay6_recv_interface_add
 * Purpose: interface add callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
int
dhcprelay6_recv_interface_add(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    struct nsm_msg_link *pmsg = (struct nsm_msg_link *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_TYPE))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        return RESULT_OK;
    }

    /* is it L3 interface? */
    if (NSM_IF_TYPE_L2 == pmsg->type)
    {
        return RESULT_OK;
    }
    
    if (NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pmsg->ifname);
    }
    if (NULL != (pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf)))
    {
        /* item exists */
        return RESULT_OK;
    }

    /* add item to hash */
    if (NULL == (pIntf = dhcprelay6_intf_calloc()))
    {
        return RESULT_OK;
    }
    pIntf->nIfIndex = pmsg->ifindex;
    pIntf->nGroupIdx = pmsg->v6_server_group_index;
    pIntf->up = (pmsg->flags & IFF_UP);
    snprintf(pIntf->szName, INTERFACE_NAMSIZ + 1, stIntf.szName);
    dhcprelay6_intf_hash_add(g_dhcprelay6Intfhash, pIntf);

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Received interface add message for interface : %s\n", pIntf->szName);
    
    if (DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)))
    {
        if (pIntf->up && pIntf->nGroupIdx > 0)
        {
            dhcprelay6_register_interface(pIntf);
        }
    }
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhcprelay6_recv_interface_delete
 * Purpose: interface delete callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
int
dhcprelay6_recv_interface_delete(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    struct nsm_msg_link *pmsg = (struct nsm_msg_link *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        return RESULT_OK;
    }

    snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pmsg->ifname);
    pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf);
    if (NULL != pIntf)
    {
        DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
            "Received interface delete message for interface : %s\n", pIntf->szName);
            
        dhcprelay6_intf_hash_del(g_dhcprelay6Intfhash, pIntf);
        dhcprelay6_intf_free(pIntf);
    }

    dhcprelay6_unregister_interface(pmsg->ifname);
    dhcprelay6_serveritem_del_byintf(pmsg->ifname);
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhcprelay6_recv_intftype_change
 * Purpose: interface type change notify
 * Input: 
 *   header:  message header
 *   arg: message argument
 *   message: message 
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : 
 * Note: 
 ******************************************************************************/
int dhcprelay6_recv_intftype_change(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcp_t *pmsg = (nsm_msg_dhcp_t *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    u_int32_t ifindex;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_NAME))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_TYPE))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_SERVER_GROUP_INDEX))
    {
        return RESULT_OK;
    }

        /*modified by xgu for bug 14721, 2011-04-26*/
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_SERVER_VRFID))
    {
        return RESULT_OK;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Received type change message for interface : %s, type : %s\n",
        (char *)pmsg->ifname, NSM_IF_TYPE_L2 == pmsg->type ? "L2" : "L3");

    /* locate interface item in the hash */
    snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
    pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf);
    if (pmsg->type == NSM_IF_TYPE_L2)
    {
        /* interface change to L2
         * delete hash item and unregister interface
         */
        dhcprelay6_unregister_interface((char *)pmsg->ifname);
        if (NULL != pIntf)
        {
            dhcprelay6_intf_hash_del(g_dhcprelay6Intfhash, pIntf);
            dhcprelay6_intf_free(pIntf);
        }
        return RESULT_OK;
    }
        
    ifindex = if_nametoindex((char *)pmsg->ifname);
    if (ifindex <= 0)
    {
        return RESULT_OK;
    }    

    /* interface change to L3
     * add interface to hash
     * and register interface if need
     */
    if (NULL == pIntf)
    {
        pIntf = dhcprelay6_intf_calloc();
        if (NULL == pIntf)
        {
            return RESULT_OK;
        }
        snprintf(pIntf->szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
        pIntf->nIfIndex = pmsg->ifindex;
        dhcprelay6_intf_hash_add(g_dhcprelay6Intfhash, pIntf);
    }

    pIntf->nGroupIdx = pmsg->v6_server_group_index;
        /*modified by xgu for bug 14721, 2011-04-26*/
    pIntf->vrfId = pmsg->vrfid;
    if (DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)))
    {
        if (pIntf->up && pIntf->nGroupIdx > 0)
        {
            dhcprelay6_register_interface(pIntf);
        }
        else
        {
            dhcprelay6_unregister_interface((char *)pmsg->ifname);
        }
    }
    return RESULT_OK;

}

/*******************************************************************************
 * Name:    dhcprelay6_recv_select_group
 * Purpose: dhcp service group change notify
 * Input: 
 *   header: message header
 *   arg:    message argument
 *   message: message
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : 
 * Note: 
 ******************************************************************************/
int dhcprelay6_recv_select_group(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcp_t *pmsg = (nsm_msg_dhcp_t *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_NAME))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_SERVER_GROUP_INDEX))
    {
        return RESULT_OK;
    }

    /* locate interface item in the hash */
    snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
    pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf);
    if (NULL == pIntf)
    {
        return RESULT_OK;
    }

    pIntf->nGroupIdx = pmsg->v6_server_group_index;
    if (DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)))
    {
        if (pIntf->up && pIntf->nGroupIdx > 0)
        {
            dhcprelay6_register_interface(pIntf);
        }
        else
        {
            dhcprelay6_unregister_interface((char *)pIntf->szName);
        }
    }
    return RESULT_OK;
}

int
dhcprelay6_recv_interface_state(struct nsm_msg_header *header,
        void *arg, void *message)
{
    struct nsm_msg_link *pmsg = (struct nsm_msg_link *)message;
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_FLAGS))
    {
        return RESULT_OK;
    }
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        return RESULT_OK;
    }

    snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pmsg->ifname);
    pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf);
    if (NULL == pIntf)
    {
        return RESULT_OK;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS,
            "Interface %s state to %s",
           pIntf->szName,
            (pmsg->flags & IFF_UP) ? "up" : "down");
    
    pIntf->up = (pmsg->flags & IFF_UP);
    if (!DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)))
    {
        return RESULT_OK;
    }

    if (pIntf->up && pIntf->nGroupIdx > 0)
    {
        dhcprelay6_register_interface(pIntf);
    }
    else
    {
        dhcprelay6_unregister_interface(pmsg->ifname);
    }
    
    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcprelay6_update_primary_address
 * Purpose: update primary address
 * Input: 
 *   pInfo: interface info struct 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_update_primary_address(struct interface_info *pInfo)
{
    struct in6_addr in;
    if (pInfo->v6address_count > 0)
    {
        dfree(pInfo->v6addresses, MDL);
        pInfo->v6address_count = 0;
        pInfo->v6addresses = NULL;
    }

    return setting_link_address(pInfo, (unsigned char *)&in, sizeof(struct in6_addr));
}

/*******************************************************************************
 * Name:    dhcprelay6_init_hardware_address
 * Purpose: get hardware address for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_init_hardware_address(struct interface_info *tmp)
{
    struct ifreq ifr;
    struct sockaddr sa;
    struct ifreq *tif = NULL;
    int sock = -1;

    if (NULL == tmp)
    {
        return -1;
    }

    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        log_fatal ("Can't create addrlist socket");
        return -1;
    }   

    if (!tmp -> ifp) 
    {
        /* Make up an ifreq structure. */
        tif = (struct ifreq *)dmalloc (sizeof (struct ifreq), MDL);
        if (!tif)
        {
            close(sock);
            log_fatal ("no space to remember ifp.");
            return -1;
        }
        
        memset (tif, 0, sizeof (struct ifreq));
        strcpy (tif -> ifr_name, tmp -> name);
        tmp -> ifp = tif;
    }

    /* Read the hardware address from this interface. */
    ifr = *tmp -> ifp;
    if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(sock);
        log_error("get hardware address for interface %s failed: %s",
                  tmp->name, strerror(errno));
        return -1;
    }
    
    sa = *(struct sockaddr *)&ifr.ifr_hwaddr;
    switch (sa.sa_family) 
    {
#ifdef HAVE_ARPHRD_TUNNEL
    case ARPHRD_TUNNEL:
        /* ignore tunnel interfaces. */
#endif
#ifdef HAVE_ARPHRD_ROSE
    case ARPHRD_ROSE:
#endif
#ifdef HAVE_ARPHRD_IRDA
    case ARPHRD_IRDA:
        /* ignore infrared interfaces. */
#endif
#ifdef HAVE_ARPHRD_SIT
    case ARPHRD_SIT:
        /* ignore IPv6-in-IPv4 interfaces. */
#endif
#ifdef HAVE_ARPHRD_IEEE1394
    case ARPHRD_IEEE1394:
        /* ignore IEEE1394 interfaces. */
#endif
#ifdef HAVE_ARPHRD_LOOPBACK
    case ARPHRD_LOOPBACK:
        /* ignore loopback interface */
        break;
#endif
    case ARPHRD_ETHER:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_ETHER;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;

#ifndef HAVE_ARPHRD_IEEE802
# define ARPHRD_IEEE802 HTYPE_IEEE802
#endif
#if defined (HAVE_ARPHRD_IEEE802_TR)
    case ARPHRD_IEEE802_TR:
#endif
    case ARPHRD_IEEE802:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_IEEE802;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;

#ifndef HAVE_ARPHRD_FDDI
# define ARPHRD_FDDI HTYPE_FDDI
#endif
    case ARPHRD_FDDI:
        tmp -> hw_address.hlen = 17;
        tmp -> hw_address.hbuf [0] = HTYPE_FDDI; /* XXX */
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 16);
        break;

#ifdef HAVE_ARPHRD_METRICOM
    case ARPHRD_METRICOM:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_METRICOM;
        memcpy (&tmp -> hw_address.hbuf [0], sa.sa_data, 6);
        break;
#endif

#ifdef HAVE_ARPHRD_AX25
    case ARPHRD_AX25:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_AX25;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;
#endif

#ifdef HAVE_ARPHRD_NETROM
    case ARPHRD_NETROM:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_NETROM;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;
#endif
    default:
        log_error("%s: unknown hardware address type %d",
                  ifr.ifr_name, sa.sa_family);
        break;
    }

    close(sock);
    return 0;
}

void dhcprelay6_get_enterprise_oid()
{
#if 0
    char buf[OEM_INFO_BUFSIZ];
    
    oem_info_get_snmp_enterprise_oid(buf);
    g_enterprise_id = hton32(atoi(buf));
#endif
}

/* form type 3 duid */
int
dhcprelay6_form_sys_duid()
{
    struct interface_info *ip;
    int len;
    struct data_string *duid = &g_relay_duid;
    /* use the first interface on the list and they have same mac address(route-mac) */
    ip = interfaces;
    if (ip == NULL)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "No interface in list to construct duid\n");
        return -1;
    }

    if ((ip->hw_address.hlen == 0) ||
        (ip->hw_address.hlen > sizeof(ip->hw_address.hbuf)))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "Impossible hardware address length for duid\n");
        return -1;
    }

    len = 4 + (ip->hw_address.hlen - 1);
    if (duid->buffer)
    {
        buffer_dereference(&duid->buffer, MDL);
        duid->data = NULL;
        duid->len = 0;
    }
    
    if (!buffer_allocate(&duid->buffer, len, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "no memory for default DUID!\n");
        return -1;
    }

    duid->data = duid->buffer->data;
    duid->len = len;

    /* Basic Link Local Address type of DUID. */
    putUShort(duid->buffer->data, DUID_LL);
    putUShort(duid->buffer->data + 2, ip->hw_address.hbuf[0]);
    memcpy(duid->buffer->data + 4, ip->hw_address.hbuf + 1,
           ip->hw_address.hlen - 1);
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_register_interface
 * Purpose: register interface to dhcpv6 relay interface list
 * Input: 
 *   name:  interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcprelay6_register_interface(S_Dhcpv6RelayInterface *pItem)
{
    struct interface_info *tmp = (struct interface_info *)0;
    isc_result_t status = ISC_R_SUCCESS;
    int need_register_io = (interfaces == NULL);
    char *name = pItem->szName;

    if ((tmp = dhcprelay6_locate_interface(name)) != NULL)
    {
        return tmp;
    }
    
    status = interface_allocate (&tmp, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_fatal("%s: interface_allocate: %s", 
                  name, isc_result_totext (status));
        return NULL;
    }
    
    strcpy (tmp->name, name);
    tmp->index = -1;
    tmp->ifindex = pItem->nIfIndex;

    interface_snorf(tmp, INTERFACE_REQUESTED);

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Register interface : %s\n", name);

    tmp->vrfid = pItem->vrfId;
    dhcprelay6_init_hardware_address(tmp);
    if (!dhcprelay6_duid_isset())
    {
        dhcprelay6_form_sys_duid();
    }
    dhcprelay6_update_primary_address(tmp);    
    /* Register the interface... */
    tmp->flags |= (INTERFACE_DOWNSTREAM | INTERFACE_REQUESTED);
    if_register6(tmp, 0);
    if (tmp->v6address_count > 0)
    {
        dhcprelay6_join_multicast_group(tmp);
    }

    /* interface_stash (tmp); */
    
    /* Now register the interfaces as protocols. */
    if (need_register_io && tmp->rfdesc != -1)
    {
        status = omapi_register_io_object((omapi_object_t *)tmp,
                                          if_readsocket, 0, got_one_v6, 0, 0);
        if (status != ISC_R_SUCCESS)
        {
            log_fatal("Can't register I/O handle for %s: %s",
                      tmp->name, isc_result_totext(status));
        }
    }
    
    interface_dereference(&tmp, MDL);
    return tmp;
}

struct interface_info *dhcprelay6_locate_interface_by_id(int id)
{
    struct interface_info *ip = NULL;

	for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (id == ip->ifindex)
        {
            return ip;
        }
	}
    
	return NULL;
}

/*******************************************************************************
 * Name:    dhcprelay6_locate_interface
 * Purpose: locate interface 
 * Input: 
 *   name: interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcprelay6_locate_interface(char *name)
{
    struct interface_info *ip = NULL;

	for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (!strcmp(ip->name, name))
        {
            return ip;
        }
	}
    
	return NULL;
}

/*******************************************************************************
 * Name:    dhcprelay6_unregister_interface
 * Purpose: unregister interface from dhcpv6 relay interface list
 * Input: 
 *   name: interface name
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_unregister_interface(char *name)
{
    struct interface_info *interface = NULL;
    struct interface_info *ip = NULL;
    struct interface_info *last = NULL;
    struct timeval tv;
    S_Dhcpv6PDClient *pPDClient = NULL;
    u_int8_t need_leave = 0;

    /* remove from interfaces */
    last = 0;
    for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (!strcmp(ip->name, name))
        {
            if (ip->v6address_count > 0)
            {
                dfree(ip->v6addresses, MDL);
                ip->v6addresses = NULL;
                ip->v6address_count = 0;
                need_leave = 1;
            }
            if (NULL != ip->remote_id)
            {
                free(ip->remote_id);
                ip->remote_id = NULL;
                ip->remote_id_len = 0;
                free(ip->circuit_id);
                ip->circuit_id = (u_int8_t *)ip->name;
                ip->circuit_id_len = strlen(ip->name);
            }
            interface_reference(&interface, ip, MDL);
            if (last) 
            {
                interface_dereference (&last -> next, MDL);
                if (ip -> next)
                {
                    interface_reference (&last -> next, ip -> next, MDL);
                }
            } 
            else 
            {
                interface_dereference (&interfaces, MDL);
                if (ip -> next)
                {
                    interface_reference (&interfaces, ip -> next, MDL);
                }
            }
            if (ip -> next)
            {
                interface_dereference (&ip -> next, MDL);
            }
            break;
        }

        last = ip;
    }
    
    if (!ip)
    {
        /* not found */
        return;
    }

#if 0
	/* add the interface to the dummy_interface list */
	if (dummy_interfaces) 
    {
		interface_reference (&interface -> next, dummy_interfaces, MDL);
		interface_dereference (&dummy_interfaces, MDL);
	}
	interface_reference (&dummy_interfaces, interface, MDL);

	/* do a DHCPRELEASE */
	if (dhcp_interface_shutdown_hook)
    {
        (*dhcp_interface_shutdown_hook) (interface);
    }
#endif

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Unregister interface : %s\n", name);

    /* wait for route del notify*/
    tv.tv_sec = cur_time + g_dhcprelay6_pdroute_time_interval;
    tv.tv_usec = 0;
    TAILQ_FOREACH(pPDClient, &(g_dhcprelay6_pdclient.pdHdr), entry)
    {
        if (name && strncmp(name, pPDClient->ifname, IFNAMSIZ))
        {
            continue;
        }
        
        add_timeout(&tv, dhcprelay6_pdclient_del_wait, (void*)pPDClient, NULL, NULL);   
    }
    
	/* remove the io object */
    if (NULL != ((omapi_object_t *)interface)->outer
        && NULL != interfaces)
    {
        /* We only register ONE object
         * If we remove it we need register another one
         */
        isc_result_t status = ISC_R_SUCCESS;

        omapi_unregister_io_object ((omapi_object_t *)interface);
        /* for bug 21546 modified by liuyang 2012-12-20*/
        if (need_leave)
        {
            dhcprelay6_leave_multicast_group(interface);
        }
        if_deregister6(interface);

        status = omapi_register_io_object((omapi_object_t *)interfaces,
                if_readsocket, 0, got_one_v6, 0, 0);
        if (status != ISC_R_SUCCESS)
        {
            log_fatal("Can't register I/O handle for %s: %s",
                    interfaces->name, isc_result_totext(status));
        }
    }
    else
    {
        omapi_unregister_io_object ((omapi_object_t *)interface);
        /* for bug 21546 modified by liuyang 2012-12-20*/
        if (need_leave)
        {
            dhcprelay6_leave_multicast_group(interface);
        }
        if_deregister6(interface);
    }
    interface_dereference (&interface, MDL);    
	return;
}

/*******************************************************************************
 * Name:    dhcprelay6_serveritem_calloc
 * Purpose: calloc a dhcpv6-server item
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: item pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
S_Dhcpv6ServerItem *dhcprelay6_serveritem_calloc(void)
{
    S_Dhcpv6ServerItem *pItem = NULL;

    pItem = calloc(1, sizeof(S_Dhcpv6ServerItem));
    if (NULL == pItem)
    {
        errno = ENOMEM;
    }
    return pItem;
}

/*******************************************************************************
 * Name:    dhcprelay6_serveritem_free
 * Purpose: free a dhcpv6-server item
 * Input:  
 *   pItem: server item pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_serveritem_free(S_Dhcpv6ServerItem *pItem)
{
    if (NULL == pItem)
    {
        return;
    }

    free(pItem);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_servergroup_calloc
 * Purpose: calloc a dhcpv6-server group item
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: group pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
S_Dhcpv6ServerGroup *dhcprelay6_servergroup_calloc(void)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    
    pGroup = calloc(1, sizeof(S_Dhcpv6ServerGroup));
    if (NULL != pGroup)
    {
        TAILQ_INIT(&pGroup->itemHdr);
    }
    else
    {
        errno = ENOMEM;
    }
    
    return pGroup;
}

/*******************************************************************************
 * Name:    dhcprelay6_servergroup_free
 * Purpose: free dhcpv6-server group
 * Input:  
 *   pGroup: group pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_servergroup_free(S_Dhcpv6ServerGroup *pGroup)
{
    S_Dhcpv6ServerItem *pItem = NULL;
    
    if (NULL == pGroup)
    {
        return;
    }

    while (!TAILQ_EMPTY(&pGroup->itemHdr))
    {
        pItem = TAILQ_FIRST(&pGroup->itemHdr);
        TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
        dhcprelay6_serveritem_free(pItem);
    }

    free(pGroup);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_locate_servergroup
 * Purpose: locate dhcpv6-server group item, create if not found
 * Input:  
 *   nGroup: group index
 * Output: 
 * Return: 
 *   success: group pointer
 *   failed : NULL for out of memory
 * Note: 
 ******************************************************************************/
S_Dhcpv6ServerGroup *dhcprelay6_locate_servergroup(unsigned long nGroup)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6ServerGroup *pGroupTmp = NULL;

    TAILQ_FOREACH(pGroup, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        if (nGroup == pGroup->nGroupIdx)
        {
            /* match item */
            return pGroup;
        }
    }

    /* callocate one group item */
    pGroup = dhcprelay6_servergroup_calloc();
    if (NULL == pGroup)
    {
        errno = ENOMEM;
        return NULL;
    }

    /* insert it into group queue */
    pGroup->nGroupIdx = nGroup;
    TAILQ_FOREACH(pGroupTmp, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        if (nGroup > pGroupTmp->nGroupIdx)
        {
            continue;
        }

        TAILQ_INSERT_BEFORE(pGroupTmp, pGroup, entry);
        return pGroup;
    }
    
    TAILQ_INSERT_TAIL(&g_dhcprelay6Cfg.groupHdr, pGroup, entry);
    return pGroup;
}

/*******************************************************************************
 * Name:    dhcprelay6_serveritem_add
 * Purpose: add dhcpv6-server item to group
 * Input:  
 *   pGroup: group pointer
 *   pszAddr: ipv6 address string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int dhcprelay6_serveritem_add(S_Dhcpv6ServerGroup *pGroup, char *pszAddr, char *name)
{
    S_Dhcpv6ServerItem *pItem = NULL;
    gensock_t su;
    unsigned long nItemCnt = 0;

    /* sanity check */
    if (NULL == pGroup || NULL == pszAddr) {
        return -1;
    }

    /* convert address string to sockunion */
    if (0 != str2gensock(&su, pszAddr)) {
        return -1;
    }

    /* ignore duplication item */
    TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry) {
        nItemCnt++;
        if (!memcmp(&su, &pItem->su, sizeof(gensock_t))) {
            if ((name && strcmp(name, pItem->name))
                || (!name && '\0' != pItem->name[0])) {
                continue;
            }
            /* exists, ignore it */
            return 0;
        }       
    }

    /* add new item to queue */
    if (nItemCnt >= DHCPV6_RELAY_MAX_SERVER) {
        return -1;
    }
    
    pItem = dhcprelay6_serveritem_calloc();
    if (NULL == pItem) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(&pItem->su, &su, sizeof(gensock_t));
    if (name) {
        strncpy(pItem->name, name, IFNAMSIZ);
    }
    TAILQ_INSERT_TAIL(&pGroup->itemHdr, pItem, entry);
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_serverlist_add
 * Purpose: add dhcpv6-server address list to group
 * Input:  
 *   cli:    client pointer
 *   nGroup: group number
 *   nToks:  tokens number
 *   ppTokArr: token array
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_serverlist_add(unsigned long nGroup, 
                              char *addr, char *name)
{
    int nRet = 0;
    S_Dhcpv6ServerGroup *pGroup = NULL;

    /* sanity checks */
    if (NULL == addr) {
        return -1;
    }
    
    if (0 == nGroup || nGroup > DHCPV6_RELAY_MAX_GROUP) {
        //cli_out(cli, "%% Invalid dhcpv6-server group number\n");
        return -1;
    }

    /* locate group item */
    pGroup = dhcprelay6_locate_servergroup(nGroup);
    if (NULL == pGroup) {
        //cli_out(cli, "%% WARNING: Out of memory!\n");
        return -1;
    }

    /* add ipv6 address */
    if (dhcprelay6_serveritem_add(pGroup, addr, name) != 0) {
        if (errno != ENOMEM) {
            //cli_out(cli, "%% Too many addresses for group %d\n",
            //        pGroup->nGroupIdx);
        } else {
            //cli_out(cli, "%% WARNING: Out of memory\n");
        }
        return -1;
    }

    return nRet;
}
    
/*******************************************************************************
 * Name:    dhcprelay6_serveritem_del
 * Purpose: del dhcpv6-server item from group
 * Input:  
 *   pGroup: group pointer
 *   pszAddr: ipv6 address string
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int dhcprelay6_serveritem_del(S_Dhcpv6ServerGroup *pGroup, char *pszAddr, char *name)
{
    S_Dhcpv6ServerItem *pItem = NULL;
    S_Dhcpv6ServerItem *pItem_next = NULL;
    gensock_t su;

    /* sanity check */
    if (NULL == pGroup || NULL == pszAddr)
    {
        return -1;
    }

    /* convert address string to sockunion */
    if (0 != str2gensock(&su, pszAddr)) {
        errno = ENOMEM;
        return -1;
    }

    /* find item */
    TAILQ_DEL_FOREACH(pItem, pItem_next, &pGroup->itemHdr, entry) {
        if (!memcmp(&su, &pItem->su, sizeof(gensock_t))) {
            /* compare ifname if specify*/
            if (name && strncmp(name, pItem->name, IFNAMSIZ)) {
                continue;
            }
            /* exists, del it */
            TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
            dhcprelay6_serveritem_free(pItem);
        }       
    }

    /* ignore nonexist item */
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_serverlist_del
 * Purpose: del dhcpv6-server address list from group
 * Input:  
 *   cli:    client pointer
 *   nGroup: group number
 *   nToks:  tokens number
 *   ppTokArr: token array
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_serverlist_del(unsigned long nGroup, char *addr, char *name)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;

    /* sanity checks */
    if (NULL == addr) {
        return -1;
    }
    
    if (0 == nGroup || nGroup > DHCPV6_RELAY_MAX_GROUP) {
        //cli_out(cli, "%% Invalid dhcpv6-server group number.\n");
        return -1;
    }

    /* locate group item */
    pGroup = dhcprelay6_locate_servergroup(nGroup);
    if (NULL == pGroup) {
        return 0;
    }

    /* del ipv6 address */
    dhcprelay6_serveritem_del(pGroup, addr, name);
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_server_address_check
 * Purpose: check dhcpv6-server address (ipv4 only)
 * Input:  
 *   nToks:  token numbers
 *   ppTokArr: token array
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_server_address_check(char *addr)
{
    gensock_t su;
    char *p = addr;
    int ret = 0;
    
    if (addr) {
        /* is it digit and dot ? */
        if (strspn(addr, "01234567890abcdefABCDEF:") != strlen(p)) {
            return -1;
        }

        /* is it valid ipv6 address */
        if (0 != str2gensock(&su, p)
            || IN6_IS_ADDR_LOOPBACK(&(su.sin6.sin6_addr))
            || IN6_IS_ADDR_UNSPECIFIED(&(su.sin6.sin6_addr))
            || IN6_IS_ADDR_V4MAPPED(&(su.sin6.sin6_addr))
            || IN6_IS_ADDR_V4COMPAT(&(su.sin6.sin6_addr))) {
            ret = -1;
        } else if (IN6_IS_ADDR_LINKLOCAL(&(su.sin6.sin6_addr))
                   || IN6_IS_ADDR_MULTICAST(&(su.sin6.sin6_addr))) {
            ret = -2;
        }
    }
    return ret;
}

/*******************************************************************************
 * Name:    dhcprelay6_serverlist_del_bygroup
 * Purpose: del all server addresses in group
 * Input:  
 *   cli:    client pointer
 *   nGroup: group index
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_serverlist_del_bygroup(unsigned long nGroup)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6ServerItem *pItem = NULL;

    if (0 == nGroup || nGroup > DHCPV6_RELAY_MAX_GROUP)
    {
        //cli_out(cli, "%% Invalid dhcpv6-server group number.\n");
        return -1;
    }

    /* locate group item */
    pGroup = dhcprelay6_locate_servergroup(nGroup);
    if (NULL == pGroup) {
        return 0;
    }

    /* del all ipv6 address */
    while (!TAILQ_EMPTY(&pGroup->itemHdr)) {
        pItem = TAILQ_FIRST(&pGroup->itemHdr);
        TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
        dhcprelay6_serveritem_free(pItem);
    }
    return 0;
}

int dhcprelay6_serveritem_del_byintf(char *name)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6ServerItem *pItem = NULL;
    S_Dhcpv6ServerItem *pItem_next = NULL;

    /* sanity checks */
    if (NULL == name)
    {
        return -1;
    }

    /*  group*/
    TAILQ_FOREACH(pGroup, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        /* del all ipv6 address */
        TAILQ_DEL_FOREACH(pItem, pItem_next, &pGroup->itemHdr, entry)
        {
            if (strncmp(name, pItem->name, IFNAMSIZ))
            {
                continue;
            }
            /* exists, del it */
            TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
            dhcprelay6_serveritem_free(pItem);
        }
    }
    return 0;
}

/*
 * Check if the relay_duid has been set.
 */
int
dhcprelay6_duid_isset(void)
{
    return (g_relay_duid.data != NULL);
}

/*
 * Set the relay DUID 
 */
void
dhcprelay6_duid_set(struct data_string *new_duid)
{
    if (new_duid && new_duid->data)
    {
        if (dhcprelay6_duid_isset())
        {
            data_string_forget(&g_relay_duid, MDL);
        }
        data_string_copy(&g_relay_duid, new_duid, MDL);
    }
}

int
dhcprelay6_duid_create(unsigned char *data, int len) 
{
    struct data_string generated_duid;

    if (!data || len <= 0)
    {
        return -1;
    }
    memset(&generated_duid, 0, sizeof (struct data_string));
    generated_duid.len = 4 + len;
    if (!buffer_allocate(&generated_duid.buffer,
        generated_duid.len, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "No memory for create DUID\n");
        return -1;
    }
    
    generated_duid.data = generated_duid.buffer->data;
    /* net byteorder*/
    putUShort(generated_duid.buffer->data, DUID_LL);
    putUShort(generated_duid.buffer->data + 2, 1);
    memcpy(generated_duid.buffer->data + 4, data, len);

    dhcprelay6_duid_set(&generated_duid);
    
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, 
        "Create duid success: %s", 
        dhcprelay6_print_duid(generated_duid.len, generated_duid.data));  

    /* for bug 21540 added by liuyang 2012-12-21 */
    data_string_forget(&generated_duid, MDL);        
    return 0;
}

#if 0
int
dhcprelay6_encode_pdroute_msg(nsm_msg_dhcpv6_pdroute_t *pmsg, u_int8_t **buf, u_int16_t *size,
    int (*pencode_func)(u_int8_t **, u_int16_t *, nsm_msg_dhcpv6_pdroute_t *))
{
    int nbytes;

    if (NULL == pmsg || NULL == buf || NULL == size
        || NULL == pencode_func)
    {
        return -1;
    }

    assert(NULL != pmsg->ifname);
    assert(NULL != pmsg->prefix);
    assert(NULL != pmsg->ip);

    memset(&pmsg->cindex, 0x00, sizeof(cindex_t));
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_IFNAME);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_IPADDR);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_PREFIX);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_FLAG);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_ID);
    /* NSM_SET_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_VRFID); */
    
    nbytes = pencode_func(buf, size, pmsg);
    if (nbytes < 0)
    {
        return -1;
    }

    return nbytes;
}
#endif

int
dhcprelay6_pdroute_destroy(S_Dhcpv6PDRouteItem *pPDRoute)
{
    if(pPDRoute)
    {
        free(pPDRoute);
    }

    return 0;
}

int 
dhcprelay6_pdroute_send(S_Dhcpv6PDRouteItem *pPDRoute, u_int8_t bAdd)
{
#if 0
    nsm_msg_dhcpv6_pdroute_t msg;
    struct nsm_client *nc = ZG6->nc;
    struct nsm_client_handler *nch;
    struct timeval tv;

    u_int16_t size;
    u_char *pnt;
    u_int32_t msg_id = NSM_MSG_DHCPV6_PDROUTE;

    int nbytes;

    if (NULL == pPDRoute || NULL == pPDRoute->pdclient
        || NULL == pPDRoute->addr 
        || NULL == nc)
    {
        return -1;
    }
    
    memset(&msg, 0x00, sizeof(msg));

    nch = nc->async;
    if (! nch || ! nch->up)
    {
        return -1;
    }

    msg.ifname_len = strlen(pPDRoute->pdclient->ifname);
    msg.ifname = pPDRoute->pdclient->ifname;
    
    msg.distance = g_dhcprelay6_pdroute_distance;

    msg.ip = pPDRoute->pdclient->client_addr.iabuf;
    msg.ip_len = pPDRoute->pdclient->client_addr.len;


    msg.prefix = pPDRoute->addr->address.iabuf;
    msg.prefix_len = pPDRoute->addr->address.len;
    msg.plen = pPDRoute->addr->plen;

    msg.flag = bAdd;
    msg.id = pPDRoute->id; 
    msg.time = pPDRoute->time;

    /* Set pnt and size.  */
    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    nbytes = dhcprelay6_encode_pdroute_msg(&msg, &pnt, &size, nsm_encode_dhcpv6_pdroute_msg);
    if (nbytes < 0)
    {
        return -1;
    }

    nsm_client_send_message(nch, 0, 0,
            NSM_MSG_DHCPV6_PDROUTE, nbytes, &msg_id);
    
    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Send pd route %s msg : %s %s %s\n",
        msg.flag ? "add" : "delete",
        piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
        piaddr(pPDRoute->pdclient->client_addr), 
        pPDRoute->pdclient->ifname);

    if (bAdd && 0 == g_pdroute_timeout_running)
    {
        tv.tv_sec = cur_time + g_dhcprelay6_pdroute_time_interval;
        tv.tv_usec = cur_tv.tv_usec;
        add_timeout(&tv, dhcprelay6_pdroute_timeout, NULL, 0, 0);
        g_pdroute_timeout_running = 1;
    }
#endif
    return 0; /* fake */
}

int 
dhcprelay6_pdroute_add(S_Dhcpv6PDClient *pPDClient, struct dhc6_addr *addr)
{
    static int count = 0;
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;

    if (NULL == pPDClient || NULL == addr)
    {
        return -1;
    }
    /* only for init addr*/
    if (addr->flags)
    {
        log_sys(M_MOD_DHCPRLY6, E_WARNING, 
            "Nothing to do for a prefix with flags\n");

        return -1;
    }
    
    pPDRoute = calloc(1, sizeof(S_Dhcpv6PDRouteItem));
    if (NULL == pPDRoute)
    {
        return -1;
    }
    
    pPDRoute->addr = addr;
    pPDRoute->pdclient = pPDClient;
    pPDRoute->id = ++count;
    pPDRoute->waitcnt = 0;
    pPDRoute->time = addr->starts;    

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Add pd route : %s %s %s\n",
        piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
        piaddr(pPDRoute->pdclient->client_addr), 
        pPDRoute->pdclient->ifname);

    if (dhcprelay6_pdroute_send(pPDRoute, 1))
    {
        addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
        addr->flags |= DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT;

        dhcprelay6_pdroute_destroy(pPDRoute);        
        return -1;
    }

    TAILQ_INSERT_TAIL(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
    return 0;    
}

int 
dhcprelay6_pdroute_delete(S_Dhcpv6PDClient *pPDClient, struct dhc6_addr *addr)
{
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;
    if (NULL == pPDClient || NULL == addr)
    {
        return -1;
    }

    if (!(addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS)
        && addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
    {   /* the item installed failed should not be in pdroute list*/
        return 0;
    }
    
    TAILQ_FOREACH(pPDRoute, &(g_dhcprelay6_pdroute.pdRouteHdr), entry)
    {
        if (pPDClient != pPDRoute->pdclient
            || addr != pPDRoute->addr)
        {
            continue;
        }

        /* not delete addr in init state*/
        if (addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
        {
            DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
                "Delete pd route : %s %s %s\n",
                piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
                piaddr(pPDRoute->pdclient->client_addr), 
                pPDRoute->pdclient->ifname);
        
            dhcprelay6_pdroute_send(pPDRoute, 0);
            addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
            
            TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
            dhcprelay6_pdroute_destroy(pPDRoute); 
        }
        else
        {
            if (!g_pdroute_timeout_running)
            {
                struct timeval tv;
                
                tv.tv_sec = cur_time + g_dhcprelay6_pdroute_time_interval;
                tv.tv_usec = cur_tv.tv_usec;
                add_timeout(&tv, dhcprelay6_pdroute_timeout, NULL, 0, 0);
                g_pdroute_timeout_running = 1;
            }
        }
        break;
    }
    return 0;    
}

/* check prefixes in init state*/
void
dhcprelay6_pdroute_timeout(void *arg)
{
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;
    S_Dhcpv6PDRouteItem *pPDRoute_next = NULL;
    struct timeval tv;
    u_int8_t wait = 0;
  
    TAILQ_DEL_FOREACH(pPDRoute, pPDRoute_next, &g_dhcprelay6_pdroute.pdRouteHdr, entry)
    {
        if (!(pPDRoute->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT))
        {   /* make sure wait time > time_interval*/
            if (++(pPDRoute ->waitcnt) > 2)
            {                
                DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
                    "Didn't receive any result for pd route adding: %s %s %s\n",
                    piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
                    piaddr(pPDRoute->pdclient->client_addr), 
                    pPDRoute->pdclient->ifname);
    
                pPDRoute->addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
                pPDRoute->addr->flags |= DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT;
                
                TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
                dhcprelay6_pdroute_destroy(pPDRoute);
            }
            else
            {
                wait = 1;
            }
        }
        /* install-failed route should not be here, just make sure about it*/
        else if (!(pPDRoute->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS))
        {
            TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
            dhcprelay6_pdroute_destroy(pPDRoute);
        }
    }

    if (!wait)
    {   /* no need to wait for any result*/
        g_pdroute_timeout_running = 0;
        return;
    }

    tv.tv_sec = cur_time + g_dhcprelay6_pdroute_time_interval;
    tv.tv_usec = cur_tv.tv_usec;
    add_timeout(&tv, dhcprelay6_pdroute_timeout, NULL, 0, 0);
}

int dhcprelay6_pdroute_clear(struct prefix_ipv6 *prefix, struct iaddr  *ip, char *ifname)
{
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;
    S_Dhcpv6PDRouteItem *pPDRoute_next = NULL;   
  
    TAILQ_DEL_FOREACH(pPDRoute, pPDRoute_next, &g_dhcprelay6_pdroute.pdRouteHdr, entry)
    {
        if (ip && memcmp(ip, &pPDRoute->pdclient->client_addr, sizeof (struct iaddr)))
        {
            continue;
        }

        if (prefix)
        {
            if (prefix->prefixlen != pPDRoute->addr->plen
                || memcmp(&prefix->prefix, pPDRoute->addr->address.iabuf, 16))
            {
                continue;
            }
        }

        if (ifname && strncmp(ifname, pPDRoute->pdclient->ifname, IFNAMSIZ))
        {
            continue;
        }

        if(pPDRoute->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT
            && pPDRoute->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS) 
        {
            if (!dhcprelay6_pdroute_send(pPDRoute, 0))
            {
                DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
                    "Clear pd route: %s %s %s\n",
                    piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
                    piaddr(pPDRoute->pdclient->client_addr),
                    pPDRoute->pdclient->ifname);
                
                pPDRoute->addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
                TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
                dhcprelay6_pdroute_destroy(pPDRoute);                
            }
            else
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, 
                    "Failed to clear pd route: %s %s %s\n",
                    piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
                    piaddr(pPDRoute->pdclient->client_addr),
                    pPDRoute->pdclient->ifname);
                return -1;                
            }
        }        
    }
    return 0;
}

/*
 * Traverse the addresses list, and destroy their contents, and NULL the
 * list pointer.
 */
static void
dhcprelay6_ia_destroy(struct dhc6_ia **src, S_Dhcpv6PDClient *pPDClient)
{
    struct dhc6_addr *addr, *naddr;
    struct dhc6_ia *ia;

    if (src == NULL || *src == NULL) 
    {
        log_error("Attempt to destroy null IA.");
        return;
    }
    ia = *src;

    for (addr = ia->addrs ; addr != NULL ; addr = naddr) 
    {
        naddr = addr->next;
        dfree(addr, MDL);
    }

    if (ia->options != NULL)
    {
        option_state_dereference(&ia->options, MDL);
    }

    dfree(ia, MDL);
    *src = NULL;
}

static void
dhcprelay6_ia_delete(struct dhc6_ia **src, S_Dhcpv6PDClient *pPDClient,
                                u_int8_t * destroy)
{
    struct dhc6_addr *addr, *naddr;
    struct dhc6_ia *ia;

    if (NULL == src || NULL == *src || NULL == pPDClient || NULL == destroy) 
    {
        return;
    }
    ia = *src;
    *destroy = 1;

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Delete IA : %x%x%x%x\n", 
        ia->iaid[0], ia->iaid[1], ia->iaid[2], ia->iaid[3]);  

    for (addr = ia->addrs ; addr != NULL ; addr = naddr) 
    {
        naddr = addr->next;
        addr->flags |= DHCPRELAY6_IA_PD_ROUTE_DELETE;
        
        if (addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT
            && addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS)
        {
            dhcprelay6_pdroute_delete(pPDClient, addr);
        }
        else if (!(addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT))
        {
            *destroy = 0;
        }
    }

    if (ia->options != NULL)
    {
        option_state_dereference(&ia->options, MDL);
    }
}
/*
 * Duplicate an IAADDR or IAPREFIX structure.
 */
static struct dhc6_addr *
dhcprelay6_dup_addr(struct dhc6_addr *addr)
{
    struct dhc6_addr *copy;

    copy = dmalloc(sizeof(*addr), MDL);

    if (copy == NULL)
        return NULL;

    memcpy(&copy->address, &addr->address, sizeof(copy->address));

    copy->plen = addr->plen;
    copy->flags = addr->flags;
    copy->starts = addr->starts;
    copy->preferred_life = addr->preferred_life;
    copy->max_life = addr->max_life;

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Duplicate a new prefix : %s\n",
         piaddrcidr_s(&copy->address, copy->plen));  

    return copy;
}
/*
 * Duplicate an IA structure.
 */
static struct dhc6_ia *
dhcprelay6_dup_ia(struct dhc6_ia *ia)
{
    struct dhc6_ia *copy;
    struct dhc6_addr **insert_addr, *addr;

    copy = dmalloc(sizeof(*ia), MDL);

    memcpy(copy->iaid, ia->iaid, sizeof(copy->iaid));

    copy->ia_type = ia->ia_type;
    copy->starts = ia->starts;
    copy->renew = ia->renew;
    copy->rebind = ia->rebind;

    insert_addr = &copy->addrs;
    for (addr = ia->addrs ; addr != NULL ; addr = addr->next)
    {
        *insert_addr = dhcprelay6_dup_addr(addr);

        if (*insert_addr == NULL)
        {
            dhcprelay6_ia_destroy(&copy, NULL);
            return NULL;
        }

        insert_addr = &(*insert_addr)->next;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Duplicate a new IA : %x%x%x%x\n", 
            copy->iaid[0], copy->iaid[1], copy->iaid[2],copy->iaid[3]);  
    return copy;
}

static int
dhcprelay6_prefix_update(struct dhc6_addr *old_addr, struct dhc6_addr *new_addr, 
                        S_Dhcpv6PDClient *pPDClient, int flag)
{
    if (!old_addr || !new_addr || !pPDClient)
    {
        return -1;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Update prefix : %s\n",
        piaddrcidr_s(&old_addr->address, old_addr->plen));
    
    if (flag)
    {
        old_addr->starts = new_addr->starts;
        old_addr->max_life = new_addr->max_life;
        old_addr->preferred_life = new_addr->preferred_life;
        /* reinit for failed adding */
        if ((old_addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
            && !( old_addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS))
        {
            old_addr->flags = 0;   
            dhcprelay6_pdroute_add(pPDClient, old_addr);  
        }   
        DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pPDClient, old_addr);
    }
    else
    {
        if (old_addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT
            && old_addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS)
        {
            dhcprelay6_pdroute_delete(pPDClient, old_addr);
        }
        /* not delete now, timeout func will do it */
        old_addr->flags |= DHCPRELAY6_IA_PD_ROUTE_DELETE;
    } 

    return 0;        
}

static int
dhcprelay6_ia_update(struct dhc6_ia *old_ia, struct dhc6_ia *new_ia, 
                        S_Dhcpv6PDClient *pPDClient, int flag)
{
    struct dhc6_addr *old_addr, *new_addr, *temp_addr;

    if (!old_ia || !new_ia || !pPDClient)
    {
        return -1;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Update IA : %x%x%x%x\n", 
        old_ia->iaid[0], old_ia->iaid[1], old_ia->iaid[2],old_ia->iaid[3]);  

    for (new_addr = new_ia->addrs; new_addr; new_addr = new_addr->next)
    {      
        if (new_addr->max_life == 0 && flag)
        {
            continue;
        }
        
        for (old_addr = old_ia->addrs; old_addr; old_addr = old_addr->next)
        {
            if (new_addr->plen == old_addr->plen 
                && old_addr->address.len == new_addr->address.len
                && !memcmp(&new_addr->address, &old_addr->address, old_addr->address.len))
            {
                dhcprelay6_prefix_update(old_addr, new_addr, pPDClient, flag);                     
                break;
            }                            
        }

        /* new addr */ 
        if (!old_addr && flag)
        {
            temp_addr = dhcprelay6_dup_addr(new_addr);
            if (!temp_addr)
            {
                return -1;
            }
            temp_addr->next = old_ia->addrs;
            old_ia->addrs = temp_addr;

            dhcprelay6_pdroute_add(pPDClient, temp_addr);
            DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pPDClient, temp_addr);
        }
    }
    return 0;
}

void dhcprelay6_pdclient_timeout(void *arg)
{
    S_Dhcpv6PDClient *pPDClient = (S_Dhcpv6PDClient *)arg;
    struct timeval tv;
    struct dhc6_ia **ia, *temp_ia = NULL;
    struct dhc6_addr **addr, *tempt_addr = NULL;

    if(!pPDClient)
    {
        return;
    }
    /* pPDClient update */
    if (pPDClient->expire > cur_time)
    {
        /* modified by liuyang 2012-12-21*/
        tv.tv_sec = pPDClient->expire;
        if (tv.tv_sec > cur_time + g_dhcprelay6_pdclient_time_interval)
        {
            tv.tv_sec = cur_time + g_dhcprelay6_pdclient_time_interval;
        }
        tv.tv_usec = 0;
        
        add_timeout(&tv, dhcprelay6_pdclient_timeout, 
            (void *)pPDClient, 0, 0);
        return;
    }

    for (ia = &pPDClient->ia_pd_list; ia && *ia;)
    {
        for (addr = &(*ia)->addrs; addr && *addr; )
        {
            tempt_addr = *addr;
            /* expire addr*/
            if((*addr)->max_life != 0xFFFFFFFF
                &&  ((*addr)->starts + (*addr)->max_life) <= cur_time) 
            {
                (*addr)->flags |= DHCPRELAY6_IA_PD_ROUTE_DELETE;
                
                DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
                    "Set delete flag for timeout prefix in pd client: \n",
                    piaddr(pPDClient->client_addr), pPDClient->ifname,   
                    dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));      

            }

            if ((*addr)->flags & DHCPRELAY6_IA_PD_ROUTE_DELETE
                && (*addr)->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
            {
                if((*addr)->flags &DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS)
                {
                    dhcprelay6_pdroute_delete(pPDClient, *addr);
                } 

                /* only delete addr with nsm result and DELETE flag*/
                if(addr == &(*ia)->addrs)
                {
                    *addr = (*ia)->addrs->next;
                }
                else
                {
                    *addr = (*addr)->next;                  
                }
                dfree(tempt_addr, MDL);
            }
            else if ((*addr)->flags & DHCPRELAY6_IA_PD_ROUTE_DELETE)
            {  /* wait nsm result to delete it*/
                pPDClient->expire = cur_time + g_dhcprelay6_pdroute_time_interval;
                addr = &(*addr)->next;
            }
            else
            {
                DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pPDClient, *addr);
                addr = &(*addr)->next;
            } 
        }
        /* delete empty ia*/
        if (NULL == (*ia)->addrs)
        {
            temp_ia = *ia;
            if (ia == &pPDClient->ia_pd_list)
            {
                *ia = pPDClient->ia_pd_list->next;
            }
            else
            {
                *ia = (*ia)->next;
            }
            dhcprelay6_ia_destroy(&temp_ia, pPDClient);
        }
        else
        {
            ia = &(*ia)->next;
        }
    }

    cancel_timeouts_by_arg(pPDClient);

    if (NULL == pPDClient->ia_pd_list)
    {
        TAILQ_REMOVE(&g_dhcprelay6_pdclient.pdHdr, pPDClient, entry);
        
        DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
            "Destroy empty pd client : %s %s %s\n",
            piaddr(pPDClient->client_addr), pPDClient->ifname,
            dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));  
        
        dhcprelay6_pdclient_destroy(pPDClient);
    }
    else
    {
        tv.tv_sec = pPDClient->expire;
        if (tv.tv_sec <= cur_time 
            || tv.tv_sec > cur_time + g_dhcprelay6_pdclient_time_interval)
        {
            tv.tv_sec = cur_time + g_dhcprelay6_pdclient_time_interval;
        }
        tv.tv_usec = cur_tv.tv_usec;
        
        add_timeout(&tv, dhcprelay6_pdclient_timeout, 
            (void *)pPDClient, 0, 0);
    }
}

S_Dhcpv6PDClient *
dhcprelay6_pdclient_new(int len, unsigned char *pduid, const struct iaddr *paddr,
                                            struct interface_info *ifp, struct dhc6_ia *pia)
{
    S_Dhcpv6PDClient *pPDClient = NULL;
    struct dhc6_ia *ia = pia;
    struct dhc6_addr *addr;
    struct timeval tv;

    if (!pduid || !paddr || !ifp || !pia)
    {
        return NULL;
    }
    
    pPDClient = malloc(sizeof(S_Dhcpv6PDClient));
    if (NULL == pPDClient)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Out of memory for create pdclient");
        return NULL;
    }

    pPDClient->duid_len = len;
    pPDClient->pduid = (unsigned char *) malloc(len);
    memcpy(pPDClient->pduid, pduid, len);
    strncpy(pPDClient->ifname, ifp->name, IFNAMSIZ);
    pPDClient->ia_pd_list = pia;
    pPDClient->client_addr.len = paddr->len;
    memcpy(pPDClient->client_addr.iabuf, paddr->iabuf, 16);

    while (ia)
    {
        for(addr = pia->addrs; addr; addr = addr->next)
        {
            dhcprelay6_pdroute_add(pPDClient, addr);

            DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pPDClient, addr);
            
            if (addr->options != NULL)
            {
                option_state_dereference(&addr->options, MDL);
            }
        }

        if (ia->options != NULL)
        {
            option_state_dereference(&ia->options, MDL);
        }
        ia = ia->next;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Create a new pd client : %s %s %s \n",
        piaddr(pPDClient->client_addr), pPDClient->ifname,
        dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));

    tv.tv_sec = pPDClient->expire;
    if (tv.tv_sec <= cur_time
        || tv.tv_sec > cur_time + g_dhcprelay6_pdclient_time_interval)
    {
        tv.tv_sec = cur_time + g_dhcprelay6_pdclient_time_interval;
    }
    tv.tv_usec = cur_tv.tv_usec;
    
    add_timeout(&tv, dhcprelay6_pdclient_timeout, 
        (void *)pPDClient, 0, 0);  
    return pPDClient;
}

/*******************************************************************************
 * Name:    dhcprelay6_pdclient_free
 * Purpose: free pd client
 * Input:  
 *   pPDClient:    pd client pointer
 * Output: 
 * Return: 
 * Note: There must be no prefix in pdroute list
 ******************************************************************************/
void dhcprelay6_pdclient_destroy(S_Dhcpv6PDClient *pPDClient)
{
    struct dhc6_ia *ia, *next_ia = NULL;
    if (NULL == pPDClient)
    {
        return;
    }

    cancel_timeouts_by_arg(pPDClient);
    
    if (pPDClient->pduid)
    {
        free(pPDClient->pduid);
    }

    for (ia = pPDClient->ia_pd_list; ia != NULL; ia = next_ia)
    {
        next_ia = ia->next;
        dhcprelay6_ia_destroy(&ia, pPDClient);
    }

    free(pPDClient);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_pdclient_update
 * Purpose: update prefix-delegation client info
 * Input:  
 *   pPDClient:    pd client
 *   pia:          new ia_pd
 *   flag:         0 means delete it, 1 means update lease time
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
static int 
dhcprelay6_pdclient_update(S_Dhcpv6PDClient *pPDClient, struct dhc6_ia * pia, int flag)
{
    struct dhc6_ia *temp_ia, *new_ia;
    struct dhc6_ia *old_ia;
    struct dhc6_addr *new_addr;
    TIME old_expire;
    struct timeval tv;
    
    if (!pPDClient || !pia)
    {
        return -1;
    }

    old_expire = pPDClient->expire;    
    new_ia = pia;    

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Update pd client : %s %s %s \n",
        piaddr(pPDClient->client_addr), pPDClient->ifname,
        dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));
        
    while (new_ia)
    {
        for (old_ia = pPDClient->ia_pd_list; old_ia; old_ia = old_ia->next)
        {
            if(!memcmp(old_ia->iaid, new_ia->iaid, sizeof (new_ia->iaid)))
            {
                break;
            }
        }

        if (!old_ia && flag)
        {
            temp_ia = dhcprelay6_dup_ia(new_ia);
            if(!temp_ia)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't duplicate ia %d for interface %s",
                    ntohl(*(u_int32_t*)new_ia->iaid), pPDClient->ifname);
                return -1;                
            }
            temp_ia->next = pPDClient->ia_pd_list;          
            pPDClient->ia_pd_list = temp_ia;

            for(new_addr = new_ia->addrs; new_addr; new_addr = new_addr->next)
            {
                dhcprelay6_pdroute_add(pPDClient, new_addr);
                DHCPRELAY6_PDCLIENT_EXPIRE_TIME_UPDATE(pPDClient, new_addr);
            }
        }
        else if (old_ia) 
        {
            dhcprelay6_ia_update(old_ia, new_ia, pPDClient, flag);           
        }

        new_ia = new_ia->next;
    }

    /* if the new expire time is much less than old one, may need readd a timeout*/
    if (pPDClient->expire < old_expire - 2 || !flag)
    {
        cancel_timeouts_by_arg((void *)pPDClient);

        if (flag)
        {
            tv.tv_sec = pPDClient->expire;
            if (tv.tv_sec <= cur_time
                || tv.tv_sec > cur_time + g_dhcprelay6_pdclient_time_interval)
            {
                tv.tv_sec = cur_time + g_dhcprelay6_pdclient_time_interval;
            }
        }
        else
        {
            tv.tv_sec = cur_time + 1;
            pPDClient->expire = tv.tv_sec;
        }
        tv.tv_usec = cur_tv.tv_usec;
        add_timeout(&tv, dhcprelay6_pdclient_timeout, (void *)pPDClient, 0, 0);
    }
    return 0;
}

void dhcprelay6_pdclient_delete(S_Dhcpv6PDClient *pPDClient)
{
    struct timeval tv;
    u_int8_t temp = 0;
    u_int8_t bDestroy = 1;
    
    struct dhc6_ia *ia, *next_ia;
    if (NULL == pPDClient)
    {
        return;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Delete pd client : %s %s %s\n",
        piaddr(pPDClient->client_addr), pPDClient->ifname,
        dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));  

    for (ia = pPDClient->ia_pd_list; ia != NULL; ia = next_ia)
    {
        next_ia = ia->next;
        dhcprelay6_ia_delete(&ia, pPDClient, &temp);

        if (!temp)
        {
            bDestroy = 0;
        }
    }

    cancel_timeouts_by_arg(pPDClient);
    
    if (!bDestroy)
    {       
        tv.tv_sec = cur_time + 1;
        tv.tv_usec = 0;

        add_timeout(&tv, dhcprelay6_pdclient_timeout, 
            (void *)pPDClient, 0, 0);
        return;
    }

    TAILQ_REMOVE(&g_dhcprelay6_pdclient.pdHdr, pPDClient, entry);        
    dhcprelay6_pdclient_destroy(pPDClient);
    return;
}

int dhcprelay6_pdclient_delete_all(char *name)
{
    S_Dhcpv6PDClient *pPDClient = NULL; 
    S_Dhcpv6PDClient *pPDClient_next = NULL;
    int count = 0;
 
    TAILQ_DEL_FOREACH(pPDClient, pPDClient_next, &(g_dhcprelay6_pdclient.pdHdr), entry)
    {
        if (name && strncmp(name, pPDClient->ifname, IFNAMSIZ))
        {
            continue;
        }
        
        count++;        
        dhcprelay6_pdclient_delete(pPDClient);       
    }
    
    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Total %d pd clients are cleared\n", count);
    return 0;
}

void dhcprelay6_pdclient_del_wait(void *arg)
{
    S_Dhcpv6PDClient *pPDClient = (S_Dhcpv6PDClient *)arg;

    dhcprelay6_pdclient_delete(pPDClient);
}

#if 0
/* process nsm notify*/
int dhcprelay6_recv_notify(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcpv6_relay_notify_t *pmsg = (nsm_msg_dhcpv6_relay_notify_t *)message;
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCPV6_RELAY_NOTIFY_CTYPE_TYPE))
    {
        return 0;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Received notify message with type : %d, result : %d\n", pmsg->type, pmsg->result);

    if (NSM_MSG_DHCPV6_PDROUTE == pmsg->type)
    {
        TAILQ_FOREACH(pPDRoute, &g_dhcprelay6_pdroute.pdRouteHdr, entry)
        {
            if (pPDRoute->id != pmsg->id || !pmsg->data
                || pPDRoute->time != *(long *)(pmsg->data))
            {
                continue;
            }

            pPDRoute->addr->flags |= DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT;                         
            if (pPDRoute->addr->flags & DHCPRELAY6_IA_PD_ROUTE_DELETE
                && 0 == pmsg->result )
            {
                pPDRoute->addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
                
                dhcprelay6_pdroute_send(pPDRoute, 0);
                TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
                dhcprelay6_pdroute_destroy(pPDRoute);
                return 0;
            }

            /* success */
            if(!pmsg->result)
            {
                pPDRoute->addr->flags |= DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;                
            }
            else
            {
                 pPDRoute->addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;
                TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
                dhcprelay6_pdroute_destroy(pPDRoute);
            }
            return 0;
        }
    }
    else if (NSM_MSG_DHCPV6_RELAY_REQUEST == pmsg->type)
    {
        dhcprelay6_duid_create(pmsg->data, pmsg->data_len);
    }
    else
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Unknown message type %d in notify message\n", pmsg->type);
        return -1;
    }    
  
    return 0;
}

/* process pdroute delete*/
int dhcprelay6_recv_pdroute_del(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcpv6_pdroute_t *pmsg = (nsm_msg_dhcpv6_pdroute_t *)message;
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;
    S_Dhcpv6PDRouteItem *pPDRoute_next = NULL;
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_FLAG)
        || NSM_DHCPV6_PDROUTE_FLAG_DELETE != pmsg->flag)
    {
        return 0;
    }
    
    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Received pdroute delete message\n");

    TAILQ_DEL_FOREACH(pPDRoute, pPDRoute_next, &g_dhcprelay6_pdroute.pdRouteHdr, entry)
    {
        if (NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_IFNAME))
        {
            if(strncmp(pmsg->ifname, pPDRoute->pdclient->ifname, pmsg->ifname_len))
            {
                continue;
            }
        }

        if (NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_IPADDR))
        {
            if (memcmp(pmsg->ip, pPDRoute->pdclient->client_addr.iabuf, 16))
            {
                continue;
            }
        }

        if (NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCPV6_PDROUTE_CTYPE_PREFIX))
        { 
            if (pmsg->plen != pPDRoute->addr->plen
                ||memcmp(pmsg->prefix, pPDRoute->addr->address.iabuf, 16))
            {
                continue;
            }
        }
        /* don't process init prefix waiting for notify message*/
        if (pPDRoute->addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
        {
            pPDRoute->addr->flags &= ~DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS;

            DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
                "pdroute delete by nsm : %s %s %s\n",
                piaddrcidr_s(&pPDRoute->addr->address, pPDRoute->addr->plen),
                piaddr(pPDRoute->pdclient->client_addr),
                pPDRoute->pdclient->ifname);
            
            TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
            dhcprelay6_pdroute_destroy(pPDRoute);            
        }                       
   }
    return 0;
}
#endif

/*******************************************************************************
 * Name:    dhcprelay6_serverlist_del_bygroup
 * Purpose: del all server addresses in group
 * Input:  
 *   cli:    client pointer
 *   nGroup: group index
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_add_remoteid_option(struct interface_info *ifp, 
                                            struct option_state *opts)
{
    unsigned len = 0;
    u_int16_t vid = 0;
    unsigned char buf[DHCPRELAY6_REMOTE_ID_MAX_LENGTH];
    
    if (NULL == ifp || NULL == opts)
    {
        return 0;
    }

    memcpy(buf, &g_enterprise_id, sizeof (u_int32_t));
    len += 4;
    
    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_DUID)
    {   /* length of duid should greate than length of duid type and hw type*/
        if (g_relay_duid.len > 4)
        {
            memcpy(&buf[len], g_relay_duid.data, g_relay_duid.len);
            len += g_relay_duid.len;
        }
    }
    buf[len++] = ':';

    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME)
    { 
        if (ifp)
        {
            memcpy(&buf[len], ifp->name, strlen(ifp->name));
            len += strlen(ifp->name);
        }
    }
    buf[len++] = ':';

    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN)
    { 
        if (!strncasecmp(ifp->name, "vlan", 4))
        {
            vid = htons((u_int16_t)atoi(ifp->name + 4));
        }
        memcpy(&buf[len], &vid, sizeof (u_int16_t));
        len += 2;
    }
    
    if (!save_option_buffer(&dhcpv6_universe, opts,
            NULL, buf, len, D6O_REMOTE_ID, 0))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't save remote-id.");
        option_state_dereference(&opts, MDL);
        return -1;
    }
    return 0;
}

static int
dhcprelay6_parse_prefixes(struct dhc6_addr **ppfx, struct packet *packet,
		    struct option_state *options)
{
    struct data_string ds;
    struct option_cache *oc;
    struct dhc6_addr *pfx;

    memset(&ds, 0, sizeof(ds));

    oc = lookup_option(&dhcpv6_universe, options, D6O_IAPREFIX);
    for ( ; oc != NULL ; oc = oc->next)
    {
        pfx = dmalloc(sizeof(*pfx), MDL);
        if (pfx == NULL)
        {
            log_error("Out of memory allocating "
                    "prefix structure.");
            return ISC_R_NOMEMORY;
        } 
        else if (evaluate_option_cache(&ds, packet, NULL, NULL,
                            options, NULL, &global_scope,
                            oc, MDL) && (ds.len >= 25))
        {
            pfx->preferred_life = getULong(ds.data);
            pfx->max_life = getULong(ds.data + 4);
            pfx->plen = getUChar(ds.data + 8);
            pfx->address.len = 16;
            memcpy(pfx->address.iabuf, ds.data + 9, 16);
            pfx->starts = cur_time;

            /* Sanity check over the prefix length */
            if ((pfx->plen < 4) || (pfx->plen > 128))
            {
                dfree(pfx, MDL);
                data_string_forget(&ds, MDL);
                continue;
            }
            /*
            * RFC 3633 section 10 says we must discard
            * prefixes whose pref is later than valid.
            */
            if ((pfx->preferred_life > pfx->max_life))
            {
                dfree(pfx, MDL);
                data_string_forget(&ds, MDL);
                continue;
            }

/* not use pfx->options*/
#if 0
            /*
            * Fortunately this is the last recursion in the
            * protocol.
            */
            if (ds.len > 25) {
                if (!option_state_allocate(&pfx->options,
                MDL)) {
                    log_error("Out of memory allocating "
                    "IAPREFIX option state.");
                    dfree(pfx, MDL);
                    data_string_forget(&ds, MDL);
                    return ISC_R_NOMEMORY;
                }

                if (!parse_option_buffer(pfx->options,
                        ds.data + 25,
                        ds.len - 25,
                        &dhcpv6_universe)) {
                    log_error("Corrupt IAPREFIX options.");
                    option_state_dereference(&pfx->options, MDL);
                    dfree(pfx, MDL);
                    data_string_forget(&ds, MDL);
                    return DHCP_R_BADPARSE;
                }
            }

            if (pfx->options != NULL)
                log_debug("RCV:  | | | X-- "
                "[Options]");
#endif
            data_string_forget(&ds, MDL);

            *ppfx = pfx;
            ppfx = &pfx->next;
        } 
        else 
        {
            log_error("Invalid IAPREFIX option cache.");
            dfree(pfx, MDL);
            if (ds.len != 0)
            data_string_forget(&ds, MDL);
            return ISC_R_UNEXPECTED;
        }
    }
    
    delete_option(&dhcpv6_universe, options, D6O_IAPREFIX);
    return ISC_R_SUCCESS;
}

static int 
dhcprelay6_parse_ia_pd(struct dhc6_ia **pia, struct packet *packet,
                        		 struct option_state *options)
{
    struct data_string ds;
    struct dhc6_ia *ia;
    struct option_cache *oc;
    int ret = -1;

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "parse IA_PD in packet \n");

    memset(&ds, 0, sizeof(ds));
    oc = lookup_option(&dhcpv6_universe, options, D6O_IA_PD);
    for ( ; oc != NULL ; oc = oc->next) 
    {
        ia = dmalloc(sizeof(*ia), MDL);
        if (ia == NULL)
        {
            log_error("Out of memory allocating IA_PD structure.");
            return ret;
        }
        else if (evaluate_option_cache(&ds, packet, NULL, NULL,
                        options, NULL,
                        &global_scope, oc, MDL) &&
                        ds.len >= 12)
        {
            memcpy(ia->iaid, ds.data, 4);
            ia->ia_type = D6O_IA_PD;
            ia->starts = cur_time;
            ia->renew = getULong(ds.data + 4);
            ia->rebind = getULong(ds.data + 8);

            /*
            * RFC3633 section 9, discard IA_PD's that
            * have t1 greater than t2, and both not zero.
            * Since RFC3633 defines this behaviour, it is not
            * an error - just normal operation.
            */
            if ((ia->renew > 0) && (ia->rebind > 0) &&
                    (ia->renew > ia->rebind)) 
            {
                log_debug("RCV:  | !-- INVALID renew/rebind times");
                dfree(ia, MDL);
                data_string_forget(&ds, MDL);
                continue;
            }

            if (ds.len > 12)
            {
                if (!option_state_allocate(&ia->options, MDL))
                {
                    log_error("Out of memory allocating "
                            "IA_PD option state.");
                    dfree(ia, MDL);
                    data_string_forget(&ds, MDL);
                    return ret;;
                }

                if (!parse_option_buffer(ia->options,
                        ds.data + 12,
                        ds.len - 12,
                        &dhcpv6_universe)) 
                {
                    log_error("Corrupt IA_PD options.");
                    option_state_dereference(&ia->options, MDL);
                    dfree(ia, MDL);
                    data_string_forget(&ds, MDL);
                    return ret;
                }
            }
            data_string_forget(&ds, MDL);

            if (ia->options != NULL)
            {
                ret = dhcprelay6_parse_prefixes(&ia->addrs, packet, ia->options);
                if (ret != 0) 
                {
                    option_state_dereference(&ia->options, MDL);
                    dfree(ia, MDL);
                    return ret;;
                }
            }

            while (*pia != NULL)
            {
                pia = &(*pia)->next;
            }
            
            *pia = ia;
            pia = &ia->next;
        }
        else
        {
            log_error("Invalid IA_PD option cache.");
            dfree(ia, MDL);
            if (ds.len != 0)
                data_string_forget(&ds, MDL);
            return ret;;
        }
    }
    ret = 0;

    delete_option(&dhcpv6_universe, options, D6O_IA_PD);
    return ret;
}

static int 
dhcprelay6_parse_status_code(struct option_state *options, unsigned *code)
{
    struct option_cache *oc;
    struct data_string ds;
    int ret = -1;

    if ((options == NULL) || (code == NULL))
    {
        return -1;
    }
    memset(&ds, 0, sizeof(ds));

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "parse status code in packet \n");
    
    /* Assume success if there is no option. */
    *code = STATUS_Success;

    oc = lookup_option(&dhcpv6_universe, options, D6O_STATUS_CODE);
    if ((oc != NULL) &&
        evaluate_option_cache(&ds, NULL, NULL, NULL, options,
        NULL, &global_scope, oc, MDL))
    {
        if (ds.len < 2) 
        {
            log_error("Invalid status code length %d.", ds.len);
        } 
        else
        {      
            *code = getUShort(ds.data);
            ret = 0;
        }
        data_string_forget(&ds, MDL);
    }
    return ret;
}

static int 
dhcprelay6_parse_duid(struct option_state *options, unsigned char **ppduid, 
                                            unsigned *len)
{
    struct option_cache *oc;
    struct data_string ds;
    int ret = -1;

    if (options == NULL || ppduid == NULL)
    {
        return ret;
    }
    memset(&ds, 0, sizeof(ds));

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "parse duid from client-id in packet \n");

    oc = lookup_option(&dhcpv6_universe, options, D6O_CLIENTID);
    if ((oc != NULL) &&
        evaluate_option_cache(&ds, NULL, NULL, NULL, options,
        NULL, &global_scope, oc, MDL))
    {
        if(ds.len > 0)
        {
            *len = ds.len;
            *ppduid = (unsigned char *)dmalloc(ds.len, MDL);
            memcpy(*ppduid, ds.data, ds.len);
            ret = 0;
        }    
        data_string_forget(&ds, MDL);
    }
    return ret;
}

/*******************************************************************************
 * Name:    dhcprelay6_check_ia_pd
 * Purpose: save or cancel prefix-delegation
 * Input:  
 *   parse_pkt:    parsed packet(if specified, pkt and pkt_len should be 0)
 *   pkt:          raw dhcpv6 packet
 *   pkt_len:      raw dhcpv6 packet length
 *   ifp:          interface to pd client
 *   piaddr:       ipv6 address of pd client
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_check_ia_pd(struct packet *parse_pkt, const struct dhcpv6_packet *pkt, 
                                int pkt_len, struct interface_info *ifp, const struct iaddr *piaddr )
{
    struct packet *temp_packet = NULL;
    S_Dhcpv6PDClient *pPDClient;
    unsigned char *pduid = NULL;
    unsigned len = 0;
    unsigned code = 0;
    struct dhc6_ia *pia = NULL;
    int flag = 0;
    int ret = -1;
    int msglen;

    if (!parse_pkt)
    {
        /* construct struct packet*/
        if (!packet_allocate(&temp_packet, MDL))
        {
            return ret;
        }

        if (!option_state_allocate(&temp_packet->options, MDL))
        {
            packet_dereference(&temp_packet, MDL);
            return ret;
        }

        temp_packet->raw = (struct dhcp_packet *) pkt;
        temp_packet->packet_length = (unsigned) pkt_len;
        interface_reference(&temp_packet->interface, ifp, MDL);

        msglen= (int)(offsetof(struct dhcpv6_packet, options));
        temp_packet->dhcpv6_msg_type = pkt->msg_type;
        memcpy(temp_packet->dhcpv6_transaction_id, 
            pkt->transaction_id, sizeof(temp_packet->dhcpv6_transaction_id));

        /* not check pkt_len for it should be do before this function*/
        if (!parse_option_buffer(temp_packet->options, 
                pkt->options, pkt_len - msglen, &dhcpv6_universe))
        {
            goto cleanup;
        }
    }
    else
    {
        temp_packet = parse_pkt;
    }

    if (temp_packet->dhcpv6_msg_type!= DHCPV6_REPLY
        && temp_packet->dhcpv6_msg_type != DHCPV6_DECLINE
        &&temp_packet->dhcpv6_msg_type != DHCPV6_RELEASE)
    {
        goto cleanup;
    }

    DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, 
        "Start to parse packet to get information for pd route \n");
    
    if(!dhcprelay6_parse_duid(temp_packet->options, &pduid, &len)
        && !dhcprelay6_parse_ia_pd(&pia, temp_packet, temp_packet->options))
    {   /* for reply message*/
        dhcprelay6_parse_status_code(temp_packet->options, &code);
    }
    else
    {
        goto cleanup;
    }

    if (temp_packet->dhcpv6_msg_type == DHCPV6_REPLY && !code)
    {   /* need add or renew pd client*/     
        flag = 1; 
    }

    if (pia)
    {     
        TAILQ_FOREACH(pPDClient, &(g_dhcprelay6_pdclient.pdHdr), entry)
        {
            if (len == pPDClient->duid_len && !memcmp(pduid, pPDClient->pduid, len)
                && !strncmp(ifp->name, pPDClient->ifname, IFNAMSIZ)
                && piaddr->len == pPDClient->client_addr.len
                && !memcmp(piaddr->iabuf, pPDClient->client_addr.iabuf, piaddr->len))
            {
                break;
            }       
        }
        
        if(NULL == pPDClient && flag)
        {
            /* use the pia to create when parsing*/
            pPDClient = dhcprelay6_pdclient_new(len, pduid, piaddr, ifp, pia);
            if (NULL == pPDClient)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, " Failed to create a new pdclient\n");
                goto cleanup;
            }
            TAILQ_INSERT_TAIL(&g_dhcprelay6_pdclient.pdHdr, pPDClient, entry);
            pia = NULL;
        }     
        else if (pPDClient)
        {   /* update lease time or delete pd client*/
            dhcprelay6_pdclient_update(pPDClient, pia, flag);
        }      
    }

    ret = 0;
 cleanup:   
    if (pia)
    {
        dhcprelay6_ia_destroy(&pia, NULL);
    }
    if (pduid)
    {
        dfree(pduid, MDL);
    }
    if (!parse_pkt)
    {
        option_state_dereference(&temp_packet->options, MDL);
        packet_dereference(&temp_packet, MDL);
    }
    return ret;
}

/*******************************************************************************
 * Name:    dhcprelay6_module_init
 * Purpose: initialize dhcpv6 relay module
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_module_init(void)
{
    if (g_dhcprelay6_initialized)
    {
        return -1;
    }

    g_dhcprelay6Switch = 0x00;

    dhcprelay6_intf_hash_init();
    memset(&g_dhcprelay6Cfg, 0x00, sizeof(S_Dhcpv6Relay));
    TAILQ_INIT(&g_dhcprelay6Cfg.groupHdr);
    
    memset(&g_dhcprelay6_pdclient, 0x00, sizeof(S_Dhcpv6PD));
    TAILQ_INIT(&g_dhcprelay6_pdclient.pdHdr);

    memset(&g_dhcprelay6_pdroute, 0x00, sizeof(S_Dhcpv6PDRoute));
    TAILQ_INIT(&g_dhcprelay6_pdroute.pdRouteHdr);   
    
    g_dhcprelay6_initialized = 1;
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_module_free
 * Purpose: release dhcpv6 relay module
 * Input:  N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_module_free(void)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6PDClient *pPDClient = NULL;
    S_Dhcpv6PDRouteItem *pPDRoute = NULL;

    if (0 == g_dhcprelay6_initialized)
    {
        return;
    }

    /* iterate group list */
    while (!TAILQ_EMPTY(&g_dhcprelay6Cfg.groupHdr))
    {
        pGroup = TAILQ_FIRST(&g_dhcprelay6Cfg.groupHdr);
        TAILQ_REMOVE(&g_dhcprelay6Cfg.groupHdr, pGroup, entry);
        dhcprelay6_servergroup_free(pGroup);
    }

    /* iterate pd client list */
    while (!TAILQ_EMPTY(&g_dhcprelay6_pdclient.pdHdr))
    {
        pPDClient = TAILQ_FIRST(&g_dhcprelay6_pdclient.pdHdr);
        TAILQ_REMOVE(&g_dhcprelay6_pdclient.pdHdr, pPDClient, entry);
        dhcprelay6_pdclient_destroy(pPDClient);
    }
    /* iterate pd route list */
    while (!TAILQ_EMPTY(&g_dhcprelay6_pdroute.pdRouteHdr))
    {
        pPDRoute = TAILQ_FIRST(&g_dhcprelay6_pdroute.pdRouteHdr);
        TAILQ_REMOVE(&g_dhcprelay6_pdroute.pdRouteHdr, pPDRoute, entry);
        dhcprelay6_pdroute_destroy(pPDRoute);
    }
    ctclib_hash_iterate(g_dhcprelay6Intfhash, dhcprelay6_do_unregister_interface, NULL);
    dhcprelay6_intf_hash_finish(g_dhcprelay6Intfhash);
    g_dhcprelay6_initialized = 0;
    DEBUG_UNREGISTER(dhcpv6, relay);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_start
 * Purpose: starting dhcpv6 relay module
 * Input: 
 *   conffile: configuration file
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcprelay6_start(char *conffile)
{
#if 1
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "Please implement dhcprelay6_start.");
    return -1;
#else
    dhcprelay6_send_routemac_request();
    dhcprelay6_get_enterprise_oid();
    dhcrelay_init(PM_ID_DHCRELAY6);
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "Ready to service.");

    return dhcprelay6_dispatch();
#endif
}

/*******************************************************************************
 * Name:    dhcprelay6_intfhash_make 
 * Purpose: get hash key
 * Input: 
 *   intf: interface pointer
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
u_int32_t dhcprelay6_intfhash_make (S_Dhcpv6RelayInterface *intf)
{
    u_int32_t val = 0;
    int nLen = strlen(intf->szName);
    int i;
    
    if (NULL != intf)
    {
        for (i = 0; i < nLen; i++)
        {
            val += intf->szName[i];
        }
    }
    
    return val;
}

/*******************************************************************************
 * Name:    dhcprelay6_intfhash_cmp 
 * Purpose: hash compare function
 * Input: 
 *   a: interface pointer
 *   b: interface pointer
 * Output: 
 * Return: 0, 1
 * Note: 
 ******************************************************************************/
int dhcprelay6_intfhash_cmp (void *a, void *b)
{
    S_Dhcpv6RelayInterface *intfa = (S_Dhcpv6RelayInterface *) a;
    S_Dhcpv6RelayInterface *intfb = (S_Dhcpv6RelayInterface *) b;

    if (!strcmp(intfa->szName, intfb->szName))
    {
        return 1;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_calloc
 * Purpose: calloc dhcpv6 relay interface 
 * Input: N/A
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
S_Dhcpv6RelayInterface *dhcprelay6_intf_calloc(void)
{
    S_Dhcpv6RelayInterface *pItem = NULL;
    pItem = calloc(1, sizeof(S_Dhcpv6RelayInterface));
    return pItem;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_free
 * Purpose: free dhcpv6 relay interface struct
 * Input: 
 *   pItem: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_intf_free(S_Dhcpv6RelayInterface *pItem)
{
    if (NULL == pItem)
    {
        return;
    }

    free(pItem);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_hash_init
 * Purpose: initialize dhcprelay6 interface hash
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay6_intf_hash_init(void)
{
    if (NULL != g_dhcprelay6Intfhash)
    {
        return 0;
    }

    g_dhcprelay6Intfhash = ctclib_hash_create(dhcprelay6_intfhash_make, dhcprelay6_intfhash_cmp);
    if (NULL == g_dhcprelay6Intfhash)
    {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_hash_finish 
 * Purpose: free hash items
 * Input: 
 *   pHash: interface hash
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_intf_hash_finish (ctclib_hash_t *pHash)
{
    ctclib_hash_clean (pHash, (HASH_CLEAN_FUNC)dhcprelay6_intf_free);
    ctclib_hash_free (pHash);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_hash_add
 * Purpose: add interface to hash
 * Input: 
 *   pHash: interface hash
 *   pIntf: intercace item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_intf_hash_add(ctclib_hash_t *pHash, S_Dhcpv6RelayInterface *pIntf)
{
    ctclib_hash_get(pHash, pIntf, ctclib_hash_alloc_intern);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_intf_hash_del
 * Purpose: delete interface from hash
 * Input: 
 *   pHash: interface hash
 *   pIntf: interface item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_intf_hash_del(ctclib_hash_t *pHash, S_Dhcpv6RelayInterface *pIntf)
{
    ctclib_hash_release (pHash, pIntf);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_send_bootrequest
 * Purpose: send boot request to server
 * Input: 
 *   pInfo: interface info
 *   data: packet
 *   len: packet length
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_send_bootrequest(struct interface_info *pInfo,
                                 char *data, size_t len)
{
    S_Dhcpv6RelayInterface *pIntf = NULL;
    S_Dhcpv6RelayInterface stIntf;
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6ServerItem *pItem = NULL;
    
    if (NULL == pInfo || NULL == data || len <= 0)
    {
        return;
    }
    
    /* locate server group */
    snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pInfo->name);
    if (NULL == (pIntf = ctclib_hash_lookup(g_dhcprelay6Intfhash, &stIntf)))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't locate interface control block for %s.",
                 stIntf.szName);
        return;
    }

    if (!pIntf->nGroupIdx)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "please setting dhcpv6-server group for interface %s",
                 stIntf.szName);
        return;
    }

    pGroup = dhcprelay6_locate_servergroup(pIntf->nGroupIdx);
    if (NULL == pGroup)
    {
        return;
    }

    if (TAILQ_EMPTY(&pGroup->itemHdr))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "please setting server address for dhcpv6-server group %u",
                 pGroup->nGroupIdx);
        return;
    }

    /* send boot request to all servers */
    if (!TAILQ_EMPTY(&pGroup->itemHdr))
    {
        DEBUG_OUT_HEX(dhcpv6, relay, DHCPRELAY6_E_DUMP,
                      "The raw DHCPv6 request packet information (up stream)",
                      (u_char *)data, len, 0);
    }
    TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry)
    {
        struct sockaddr_in6 to;
         
        memcpy(&to, &pItem->su, sizeof(struct sockaddr_in6));
        to.sin6_port = local_port;
#ifdef HAVE_SA_LEN
        to.sin6_len = sizeof(struct sockaddr_in6);
#endif
        if ('\0' != pItem->name[0])
        {
            pInfo->outindex = if_nametoindex(pItem->name);
        }
        else
        {
            pInfo->outindex = 0;
        }

        DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS,
                "send DHCPv6 request to %s port %d",
                in6addr2str(&to.sin6_addr), ntohs(local_port));

        if (send_packet6(pInfo, (unsigned char *)data, len, &to) < 0)
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "send DHCPv6 request packet failed: %s",
                    strerror(errno));
        }
    }
    return;
}

extern int add_ipv6_addr_to_interface(struct interface_info *, struct in6_addr *);
/* copy the first global address */
int
setting_link_address(struct interface_info *ifp, unsigned char *buf, size_t len)
{
    struct ifaddrs *ifap = NULL;
    struct ifaddrs *ifapfree = NULL;
    u_int8_t ret = -1;

    if (ifp->v6address_count <= 0)
    {
        if (getifaddrs(&ifap) != 0)
        {
            DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_ERR, "can not get interface (%s) address: %s", 
                    ifp->name, strerror(errno));
            return -1;
        }

        for (ifapfree = ifap; ifap; ifap = ifap->ifa_next)
        {
            if (strcmp(ifap->ifa_name, ifp->name)
                || NULL == ifap->ifa_addr)
            {
                continue;
            }

            if (AF_INET6 == ifap->ifa_addr->sa_family)
            {
                struct sockaddr_in6 *in = (struct sockaddr_in6 *)ifap->ifa_addr;
                struct in6_addr *addr = &in->sin6_addr;

                if (IN6_IS_ADDR_LOOPBACK(addr) 
                    || IN6_IS_ADDR_UNSPECIFIED(addr)
                    || IN6_IS_ADDR_LINKLOCAL(addr)
                    || IN6_IS_ADDR_SITELOCAL(addr)) /* XXX: we does not have DHCPv6 server */
                {
                    continue;
                }

                DEBUG_OUT(dhcpv6, relay, DHCPRELAY6_E_EVENTS, "interface %s have ipv6 address %s",
                        ifp->name, in6addr2str(addr));

                add_ipv6_addr_to_interface(ifp, addr);
                ret = 1;
            }
        }

        freeifaddrs (ifapfree);
    }

    if (ifp->v6address_count > 0)
    {
        memcpy(buf, &ifp->v6addresses[0], 16);
        return 0;
    }

    return ret;;
}

/* s*_len stuff */
static __inline u_int8_t
sysdep_sa_len (const struct sockaddr *sa)
{
#ifndef HAVE_SA_LEN
    switch (sa->sa_family)
    {
        case AF_INET:
            return sizeof (struct sockaddr_in);
        case AF_INET6:
            return sizeof (struct sockaddr_in6);
    }
    return sizeof (struct sockaddr_in);
#else
    return sa->sa_len;
#endif
}

char *
addr2str(struct sockaddr *sa)
{
    static char addrbuf[8][NI_MAXHOST];
    static int round = 0;
    char *cp;

    round = (round + 1) & 7;
    cp = addrbuf[round];

    getnameinfo(sa, sysdep_sa_len(sa), cp, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);

    return (cp);
}

char *
in6addr2str(struct in6_addr *in6)
{
    struct sockaddr_in6 sa6;

    memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
    sa6.sin6_len = sizeof(sa6);
#endif
    sa6.sin6_addr = *in6;

    return (addr2str((struct sockaddr *)&sa6));
}

char * dhcprelay6_print_duid (unsigned len, const u_int8_t *data)
{
    static char buf[DHCPRELAY6_DUID_MAX_LENGTH];
    unsigned i;
    /* for bug 21540 added by liuyang 2012-12-21*/
    strcpy(buf, "null");
    if(!data)
    {
        return buf;
    }

    for (i = 0; i < len; i++)
    {
        sprintf(&buf[i*2], "%02x", data[i]);
    }
    buf[i * 2] = 0;
    return buf;
}
/* for bug 21540 added by liuyang 2012-12-21*/
char * dhcprelay6_print_host_duid ()
{
    return dhcprelay6_print_duid(g_relay_duid.len, g_relay_duid.data);
}

int dhcprelay6_send_routemac_request()
{
#if 0
    nsm_msg_dhcpv6_relay_request_t msg;
    struct nsm_client *nc = ZG6->nc;
    struct nsm_client_handler *nch;

    u_int16_t size;
    u_char *pnt;
    u_int32_t msg_id = NSM_MSG_DHCPV6_RELAY_REQUEST;
    int nbytes;
    
    memset(&msg, 0x00, sizeof(msg));
    nch = nc->async;
    if (! nch || ! nch->up)
    {
        return -1;
    }

    msg.flag = NSM_DHCPV6_RELAY_REQUEST_ROUTEMAC;
        /* Set pnt and size.  */
    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;

    memset(&msg.cindex, 0x00, sizeof(cindex_t));
    NSM_SET_CTYPE (msg.cindex, NSM_DHCPV6_RELAY_REQUEST_CTYPE_FLAG);
    
    nbytes =  nsm_encode_dhcpv6_relay_request_msg(&pnt, &size, &msg);
    if (nbytes < 0)
    {
        return -1;
    }

    nsm_client_send_message(nch, 0, 0,
            NSM_MSG_DHCPV6_RELAY_REQUEST, nbytes, &msg_id);
#endif
    return 0;
}

/* piaddr() turns an iaddr structure into a printable address. */
static const char *
piaddr_r(const struct iaddr *addr)
{
    static char
		pbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
			 /* "255.255.255.255" */
   /* INSIST((addr.len == 0) || (addr.len == 4) || (addr.len == 16)); */
    if (addr->len == 0)
    {
        return "<null address>";
    }
    if (addr->len == 4)
    {
        return inet_ntop(AF_INET, addr->iabuf, pbuf, sizeof(pbuf));
    } 
    if (addr->len == 16) 
    {
        return inet_ntop(AF_INET6, addr->iabuf, pbuf, sizeof(pbuf));
    }

    log_fatal("piaddr():%s:%d: Invalid address length %d.", MDL,
        addr->len);
    /* quell compiler warnings */
    return NULL;
}

/* Format an address and mask-length into printable CIDR notation. */
char *
piaddrcidr_s(const struct iaddr *addr, unsigned int bits) 
{
    static char ret[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128")];
    /* "255.255.255.255/32" */

    if (bits > (addr->len * 8))
    {
        return NULL;
    }

    sprintf(ret, "%s/%d", piaddr_r(addr), bits);
    return ret;            
}

 /*
  * Map an interface index into its name.
  * Returns NULL on error.
  */
char*   dhcprelay6_if_indextoname(unsigned ifindex, char *ifname)
{
    int ctl_sock;
    struct ifreq ifr;
    char*  ret = NULL;

    if(!ifname)
    {
       return ret;  
    }
    
    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_ifindex = ifindex;
    if ((ctl_sock = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        if (ioctl(ctl_sock, SIOCGIFNAME, &ifr) >= 0)
        {
            ret = strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
        } 
        else
        {
            /* Posix requires ENXIO */
            if (errno == ENODEV)
            errno = ENXIO;
        } 
        close(ctl_sock);
    }
    return ret;
}   


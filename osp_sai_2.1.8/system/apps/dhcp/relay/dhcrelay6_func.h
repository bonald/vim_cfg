/****************************************************************************
* $Id: dhcrelay_func.h,v 1.1 2007/01/19 06:10:33 xliu Exp $
*  functions for DHCP relay
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-09 18:41
* Reason        : First Create.
****************************************************************************/

#ifndef _DHCRELAY6_FUNC_H_
#define _DHCRELAY6_FUNC_H_

#include "bsd_queue.h"
#include "param_check.h"

#ifndef prefix_ipv6
struct prefix_ipv6
{
    u_int8_t family;
    u_int8_t prefixlen;
    u_int8_t pad1;
    u_int8_t pad2;
    struct in6_addr prefix;
};
#endif

#ifndef DEBUG_DEFINE
#define DEBUG_DEFINE(mod, sub, typeenum, ...) 
#define DEBUG_IMPLEMENT(mod, sub, typestr, ...)  
#define DEBUG_REGISTER(mod, sub, pmodid, pcmdmode)
#define DEBUG_UNREGISTER(mod, sub)
#define IS_DEBUG_ON(mod, sub, typeenum)
#define DEBUG_OUT(mod, sub, typeenum, ...)
#define DEBUG_OUT_HEX(mod, sub, typeenum, msg, buf, len, offset)
#define DEBUG_SHOW(mod, sub, ...)
#endif

#define DHCPV6_RELAY_MAX_GROUP        16
#define DHCPV6_RELAY_MAX_SERVER       8

/* service switch */
#define DHCPV6_SERVICE_FLAG           0x01
#define DHCPV6_RELAY_SERVICE_FLAG     0x02

#define DHCPRELAY6_REMOTEID_OPTION_FLAG 0x04
#define DHCPRELAY6_IA_PD_ADVERTISE_FLAG 0X08

#define DHCPRELAY6_PDCLIENT_TIME_INTERVAL_DEFAULT 60
#define DHCPRELAY6_PDROUTE_TIME_INTERVAL_DEFAULT 5
/* The max length of remote-id include length of ifname, duid , vlanid*/
#define DHCPRELAY6_REMOTE_ID_MAX_LENGTH 256
#define DHCPRELAY6_DUID_MAX_LENGTH 128
#define DHCPRELAY6_PDROUTE_DISTANCE_DEFAULT 254

#define DHCP_ALL_PKT_TYPE_MAX 24
#define DHCPV6_TYPE_BASE 8
/* for bug 21663 modified by liuyang 2012-12-24*/
typedef struct dhcp_relay_pkt_stats_s
{
    u_int32_t recv[DHCP_ALL_PKT_TYPE_MAX];
    u_int32_t send[DHCP_ALL_PKT_TYPE_MAX];
    u_int32_t err[DHCP_ALL_PKT_TYPE_MAX];
    
    u_int32_t client6_relayed;	
    u_int32_t server6_relayed;	
    u_int32_t client6_errors;	
    u_int32_t server6_errors;
    u_int32_t unknown;    
} dhcp_relay_pkt_stats_t;

#define DHCPRELAY6_INTERFACE_CHECK(cli, name) \
do \
{ \
    if (!if_nametoindex(name) \
        || (pal_strncmp(name, "eth-", 4) \
        && pal_strncmp(name, "vlan", 4) \
        && pal_strncmp(name, "agg", 3))) \
    {\
        if (NULL != cli) \
        { \
            cli_out(cli, "%% Invalid interface %s\n", name); \
        }\
        return CLI_ERROR; \
    }\
}while (0)

//DEBUG_DEFINE(dhcpv6, relay,
//             DHCPRELAY6_E_PKT, DHCPRELAY6_E_EVENTS,
//             DHCPRELAY6_E_ERR, DHCPRELAY6_E_DUMP);

/* DHCP-server item */
typedef struct tagDhcpv6ServerItem
{
    TAILQ_ENTRY(tagDhcpv6ServerItem) entry;

    gensock_t su; /* server ip address */
    char name[IFNAMSIZ]; /* interface used for send pkt*/
} S_Dhcpv6ServerItem;

/* DHCP-server item header */
typedef TAILQ_HEAD(tagDhcpv6ServerItemHdr, tagDhcpv6ServerItem) S_Dhcpv6ServerItemHdr;

/* DHCP-server group */
typedef struct tagDhcpv6ServerGroup
{
    TAILQ_ENTRY(tagDhcpv6ServerGroup) entry;

    S_Dhcpv6ServerItemHdr itemHdr;
    unsigned long nGroupIdx; /* group index */
} S_Dhcpv6ServerGroup;

/* DHCP-server group header */
typedef TAILQ_HEAD(tagDhcpv6ServerGroupHdr, tagDhcpv6ServerGroup) S_Dhcpv6ServerGroupHdr;

/* DHCP relay configuration 
 */
typedef struct tagDhcpv6Relay
{
    S_Dhcpv6ServerGroupHdr groupHdr; /* dhcpv6 server groups header */
} S_Dhcpv6Relay;

/* DHCP PD client group */
typedef struct tagDhcpv6PDClient
{
    TAILQ_ENTRY(tagDhcpv6PDClient) entry;

    unsigned char * pduid;
    int duid_len;
    char ifname[IFNAMSIZ];
    TIME expire;
    struct iaddr client_addr;
    struct dhc6_ia * ia_pd_list;
} S_Dhcpv6PDClient;

typedef TAILQ_HEAD(tagDhcpv6PDClientHdr, tagDhcpv6PDClient) S_Dhcpv6PDClientHdr;

typedef struct tagDhcpv6PD
{
    S_Dhcpv6PDClientHdr pdHdr; /* dhcpv6 pd client header */
} S_Dhcpv6PD;

/* remote-id keyword */
#define DHCPRELAY6_REMOTE_ID_KEYWORD_DUID      0x0001
#define DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN      0x0002
#define DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME     0x0004
#define DHCPRELAY6_REMOTE_ID_KEYWORD_DEFAULT \
    (DHCPRELAY6_REMOTE_ID_KEYWORD_DUID | DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME)
/* DHCP relay configuration per interface */
typedef struct tagDhcpv6RelayInterface
{
    unsigned long nIfIndex;
    char szName[INTERFACE_NAMSIZ + 1];
    int up; /* is this interface up ? */

   /* u_int16_t remote_id_format;  define remote-id format*/
    unsigned long nGroupIdx; /* dhcpv6-server group Index */
    unsigned int vrfId;    /*modified by xgu for bug 14721, 2011-04-26*/
} S_Dhcpv6RelayInterface;

typedef struct tagDhcpv6PDRouteItem
{
    TAILQ_ENTRY(tagDhcpv6PDRouteItem) entry;
    
    S_Dhcpv6PDClient *pdclient;
    struct dhc6_addr *addr;

    u_int32_t id;
    TIME time;
    
    u_int8_t waitcnt;
} S_Dhcpv6PDRouteItem;

typedef TAILQ_HEAD(tagDhcpv6PDRouteHdr, tagDhcpv6PDRouteItem) S_Dhcpv6PDRouteHdr;

typedef struct tagDhcpv6PDRoute
{
    S_Dhcpv6PDRouteHdr pdRouteHdr; /* dhcpv6 pd route item header */
} S_Dhcpv6PDRoute;


extern S_Dhcpv6PD g_dhcprelay6_pdclient;
extern S_Dhcpv6PDRoute g_dhcprelay6_pdroute;
extern ctclib_hash_t *g_dhcprelay6Intfhash;
extern S_Dhcpv6Relay g_dhcprelay6Cfg;
extern S_Dhcpv6PD g_dhcprelay6pd;
extern int g_dhcprelay6_initialized;
extern int g_dhcprelay6Switch;
#define DHCPRELAY6_CHK_FLAG(flag)    ((g_dhcprelay6Switch & (flag)) == (flag))
#define DHCPRELAY6_SET_FLAG(flag)    g_dhcprelay6Switch |= (flag)
#define DHCPRELAY6_CLR_FLAG(flag)    g_dhcprelay6Switch &= (~(flag))

extern S_Dhcpv6ServerGroup *dhcprelay6_locate_servergroup(unsigned long nGroup);
extern int dhcprelay6_serverlist_del(unsigned long nGroup, 
                                    char *addr, char *name);
extern int dhcprelay6_serverlist_del_bygroup(unsigned long nGroup);
extern int dhcprelay6_serverlist_add(unsigned long nGroup, 
                                    char *addr, char *name);
extern int dhcprelay6_server_address_check(char *addr);
extern int dhcprelay6_module_init(void);
extern void dhcprelay6_module_free(void);
extern int dhcprelay6_intf_hash_init(void);
extern int dhcprelay6_add_remoteid_option(struct interface_info *ifp, 
                                        struct option_state *opts);
//extern int init_hardware_address(struct interface_info *interface);
extern struct interface_info *dhcprelay6_register_interface(S_Dhcpv6RelayInterface *pItem);
extern void dhcprelay6_unregister_interface(char *name);
extern int dhcprelay6_service_onoff(unsigned char bOn);

extern struct interface_info *dhcprelay6_locate_interface_by_id(int id);

extern void dhcprelay6_send_bootrequest(struct interface_info *pInfo,
        char *, size_t);

extern int dhcprelay6_update_primary_address(struct interface_info *pInfo);
extern void dhcprelay6_signal_init ();

extern void dhcprelay6_dump_packet (struct packet *);

extern int setting_link_address(struct interface_info *ifp, unsigned char *, size_t);
char *in6addr2str(struct in6_addr *in6);

extern int dhcprelay6_check_ia_pd(struct packet *parse_pkt, const struct dhcpv6_packet *pkt, 
                                int pkt_len, struct interface_info *ifp, const struct iaddr *piaddr );

extern void dhcprelay6_pdroute_timeout(void *arg);
extern int dhcprelay6_pdroute_clear(struct prefix_ipv6 *prefix, struct iaddr  *ip, char *ifname);
char * dhcprelay6_print_hex (unsigned len, const u_int8_t * data, unsigned limit, char *buf);
extern int dhcprelay6_pdclient_delete_all(char *name);
extern int dhcprelay6_duid_isset(void);
extern int dhcprelay6_send_routemac_request(void);
extern void dhcprelay6_pdclient_destroy(S_Dhcpv6PDClient *pPDClient);
extern char * dhcprelay6_print_duid (unsigned len, const u_int8_t *data);

char * piaddrcidr_s(const struct iaddr *addr, unsigned int bits) ;
char*   dhcprelay6_if_indextoname(unsigned ifindex, char *ifname);
/*for bug 21540 added by liuyang 2012-12-21*/
char * dhcprelay6_print_host_duid (void);
#endif /* !_DHCRELAY6_FUNC_H_ */

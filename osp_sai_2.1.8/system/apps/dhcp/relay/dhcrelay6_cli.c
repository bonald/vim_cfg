/****************************************************************************
 * $Id$
 *  dhcpv6 relay
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Alexander Liu
 * Date          : 2009-01-15 10:14
 * Reason        : First Create.
 ****************************************************************************/

#if 0

#ifdef _CENTEC_

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "dhcpd.h"

#include "pal.h"
#include "pal_log.h"
#include "lib.h"
#include "module_config.h"
#include <syslog.h>

#include "cli.h"
#include "log.h"
#include "param_check.h"
#include "dhcrelay6_func.h"

/****************************************************************************
 *  
* Defines and Macros
* 
*****************************************************************************/
#define DHCPV6_OUT_SPLIT   \
cli_out(cli, "============================================================\n")

#define DHCPV6_RELAY_INIT_CHECK() \
do \
{ \
    if (!g_dhcprelay6_initialized) \
    { \
        if (NULL != cli) \
        { \
            cli_out(cli, "%% Internal error: system initialize failed\n"); \
        } \
        return CLI_ERROR; \
    } \
} while (0)

DEBUG_IMPLEMENT(dhcpv6, relay, "packet", "events", "error", "dump");
DEBUG_CLI_IMPLEMENT(dhcpv6, relay, 
                    "debug dhcpv6 relay (packet|events|error|dump|all)",
                    "Debugging functions",
                    "DHCPv6",
                    "Relay",
                    "DHCPv6 message fields",
                    "Relay events",
                    "Error DHCPv6 message",
                    "Dump message in hex format",
                    "Turn all debug")

/****************************************************************************
 *  
* Global and Declaration
* 
*****************************************************************************/
extern int drop_agent_mismatches;
extern int add_agent_options;

/* statistics */
extern int bad_circuit_id;
extern int bogus_giaddr_drops;
extern int client_packet_errors;
extern int client_packets_relayed;
extern int corrupt_agent_options;
extern int missing_agent_option;
extern int missing_circuit_id;
extern int server_packet_errors;
extern int server_packets_relayed;

extern dhcp_relay_pkt_stats_t g_dhcp_relay_pkt_stats;
extern u_int16_t g_dhcprelay6_remoteid_format;
extern u_int8_t g_dhcprelay6_pdroute_distance;

/****************************************************************************
 *  
* Function
* 
*****************************************************************************/

/*******************************************************************************
 * Name:    dhcprelay6_dump_packet_option
 * Purpose: dump dhcpv6 packet option for debug
 * Input: 
 *   oc:
 *   packet: dhcpv6 packet
 *   lease:
 *   client:
 *   in_options:
 *   cfg_options:
 *   scope: 
 *   u: 
 *   foo: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
dhcprelay6_dump_packet_option(struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    memset (&ds, 0, sizeof ds);

    if (u != &dhcpv6_universe) 
    {
        name = u->name;
        dot = ".";
    }
    else 
    {
        name = "";
        dot = "";
    }
    if (evaluate_option_cache (&ds, packet, lease, client,
                   in_options, cfg_options, scope, oc, MDL)) 
    {
        DEBUG_SHOW(dhcpv6, relay, "  option %s%s%s %s;",
                   name, dot, oc->option->name,
                   pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
        data_string_forget (&ds, MDL);
    }
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_dump_packet 
 * Purpose: dump packet fields
 * Input: 
 *   packet: packet after decode
 *   len: packet length
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
dhcprelay6_dump_packet (struct packet *packet)
{
    int i;

    if (NULL == packet || !(IS_DEBUG_ON(dhcpv6, relay, DHCPRELAY6_E_PKT)))
    {
        return;
    }

    DEBUG_SHOW(dhcpv6, relay, "Dump DHCPv6 packet fields");
    DEBUG_SHOW(dhcpv6, relay, " packet length %d", packet->packet_length);
    DEBUG_SHOW(dhcpv6, relay, " packet type %s", 
               dhcpv6_type_names[packet->dhcpv6_msg_type]);
    DEBUG_SHOW(dhcpv6, relay, " transaction id: 0x%02x%02x%02x",
               packet->dhcpv6_transaction_id[0],
               packet->dhcpv6_transaction_id[1],
               packet->dhcpv6_transaction_id[2]);
    DEBUG_SHOW(dhcpv6, relay, " hop count: %d", packet->dhcpv6_hop_count);
    DEBUG_SHOW(dhcpv6, relay, " link address: %s", 
            in6addr2str(&packet->dhcpv6_link_address));
    DEBUG_SHOW(dhcpv6, relay, " peer address: %s", 
            in6addr2str(&packet->dhcpv6_peer_address));

    /* dump options */
    for (i = 0; i < packet->options->universe_count; i++) 
    {
        if (!packet->options->universes[i]) 
        {
            continue;
        }
        option_space_foreach(packet, (struct lease *)0,
                (struct client_state *)0,
                (struct option_state *)0,
                packet->options,
                &global_scope,
                universes [i], 0,
                dhcprelay6_dump_packet_option);
    }
    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_show_server_group
 * Purpose: show one dhcpv6 server group information
 * Input:  
 *   cli:    client pointer
 *   pGroup: group pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_show_server_group(struct cli *cli, S_Dhcpv6ServerGroup *pGroup)
{
    char szAddrBuf[SU_ADDRSTRLEN + 1];
    S_Dhcpv6ServerItem *pItem = NULL;
    unsigned long nIdx = 0;

    if (NULL == cli || NULL == pGroup)
    {
        return;
    }

    if (TAILQ_EMPTY(&pGroup->itemHdr))
    {
        return;
    }   

    cli_out(cli, "group %u ipv6 address list:\n", pGroup->nGroupIdx);
    TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry)
    {
        nIdx++;
        cli_out(cli, "    [%u] %s", 
                nIdx, sockunion2str(&pItem->su, szAddrBuf, SU_ADDRSTRLEN + 1));
        if ('\0' != pItem->name[0])
        {
           cli_out(cli, " - %s" , pItem->name);
        }
        cli_out(cli, "\n");
    }
    cli_out(cli, "\n");
    return;
}   

/*******************************************************************************
 * Name:    dhcprelay6_show_server_groups
 * Purpose: show dhcpv6 server groups
 * Input:  
 *   cli:   client pointer
 *   nGroup: group index (0 for all groups)
 * Output: 
 * Return:
 *   success: CLI_SUCCESS
 *   failed:  CLI_ERROR
 * Note: 
 ******************************************************************************/
int dhcprelay6_show_server_groups(struct cli *cli, unsigned long nGroup)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    
    if (NULL == cli)
    {
        return CLI_ERROR;
    }

    cli_out(cli, "DHCPv6 server group information:\n");
    DHCPV6_OUT_SPLIT;
    if (0 != nGroup)
    {
        /* show one group */
        pGroup = dhcprelay6_locate_servergroup(nGroup);
        if (NULL == pGroup)
        {
            cli_out(cli, "%% WARNING: out of memory\n");
            return CLI_ERROR;
        }
        
        dhcprelay6_show_server_group(cli, pGroup);
        return CLI_SUCCESS;
    }

    /* show all group */
    TAILQ_FOREACH(pGroup, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        dhcprelay6_show_server_group(cli, pGroup);
    }
    return CLI_SUCCESS;
}

void dhcprelay6_show_ia_pd(struct cli *cli, struct dhc6_ia * pia)
{
    struct dhc6_ia *ia;
    struct dhc6_addr *addr;
    struct pal_timeval now;
    char time[20];

    if (!cli || !pia)
    {
        return;
    }

    ia = pia;
    while(ia)
    {
        cli_out(cli, "    IA ID  : %x%x%x%x\n", 
            ia->iaid[0], ia->iaid[1], ia->iaid[2], ia->iaid[3]);

        for (addr = ia->addrs; addr; addr = addr->next)
        {
            if (!(addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT) 
                ||!(addr->flags & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS)
                || addr->flags & DHCPRELAY6_IA_PD_ROUTE_DELETE)
            {
                continue;
            }
            /* calculate aboslute time added by liuyang 2012-8-16*/
            gettimeofday(&now, NULL);
            format_time_str(time, 20, 
                    "%Y-%m-%d %H:%M:%S", addr->starts + addr->max_life - cur_time + now.tv_sec );

            cli_out(cli, "        IA Prefix :  %s/%d\n", piaddr(addr->address), addr->plen);
            cli_out(cli, "            prefered/max lifetime : %d/%d\n", addr->preferred_life, addr->max_life);
            cli_out(cli, "            expired time :  %s\n", time);
        }
        ia = ia->next;
    }    
}

void dhcprelay6_show_pdclient(struct cli *cli, char *ifname)
{
    S_Dhcpv6PDClient *pPDClient;
    
    if (NULL == cli)
    {
        return;
    }

    cli_out(cli, "DHCPv6 prefix-delegation client information:\n");
    DHCPV6_OUT_SPLIT;

    TAILQ_FOREACH(pPDClient, &g_dhcprelay6_pdclient.pdHdr, entry)
    {
        if (ifname && pal_strncmp(ifname, pPDClient->ifname, IFNAMSIZ))
        {
            continue;
        }
        
        cli_out(cli, "Interface  : %s\n", pPDClient->ifname);       
        cli_out(cli, "Client DUID  : %s\n", dhcprelay6_print_duid(pPDClient->duid_len, pPDClient->pduid));
        cli_out(cli, "Client IPv6 address  : %s\n", piaddr(pPDClient->client_addr));

        dhcprelay6_show_ia_pd(cli, pPDClient->ia_pd_list);
        DHCPV6_OUT_SPLIT;
    }
}

int dhcprelay6_cli_pdroute_clear(struct cli *cli, char *prefix, char *ip, char *ifname)
{
    struct prefix_ipv6 *p = NULL;
    struct iaddr *addr = NULL;
    union sockunion *psu = NULL;
    int ret = CLI_ERROR;;

    if (ip)
    {
        if (!( psu= sockunion_str2su(ip)))
        {
            cli_out(cli, "%% Out of memory when parsing ipv6 address");
            goto err_out;
        }

        if (IN6_IS_ADDR_LOOPBACK(&(psu->sin6.sin6_addr))
            || IN6_IS_ADDR_UNSPECIFIED(&(psu->sin6.sin6_addr))
            || IN6_IS_ADDR_V4MAPPED(&(psu->sin6.sin6_addr))
            || IN6_IS_ADDR_V4COMPAT(&(psu->sin6.sin6_addr)))
        {
            cli_out(cli, "%% Invalid ipv6 address\n");         
            goto err_out;
        }
        addr = XMALLOC(MTYPE_TMP, sizeof (struct iaddr));
        addr->len = 16; /* no length in sockaddr_in6*/
        pal_mem_cpy(addr->iabuf, &psu->sin6.sin6_addr, addr->len);
    }

    if (prefix)
    {
        if(NULL != (p = XMALLOC(MTYPE_TMP, sizeof (struct prefix_ipv6))))
        {
            if (str2prefix_ipv6(prefix, p) < 0)
            {
                cli_out(cli, "%% Error occur when parsing ipv6 prefix");                
                goto err_out;
            }

            if (IPV6_PREFIX_IS_LINK_LOCAL(p)
                || IPV6_PREFIX_IS_MULTICAST(p)
                || IPV6_PREFIX_IS_LOOPBACK(p)
                || IPV6_PREFIX_IS_UNSPECIFIED(p)
                || IPV6_PREFIX_IS_LOOPBACK_AND_UNSPECIFIED(p))
            {
                cli_out(cli, "%% Invalid ipv6 prefix\n");         
                goto err_out;
            }                
        }
        else
        {
            cli_out(cli, "%% Out of memory when parsing ipv6 prefix");                
                goto err_out;
        }
    }

    if(CLI_SUCCESS != (ret = dhcprelay6_pdroute_clear(p, addr, ifname)))
    {
        cli_out(cli, "%% Failed to clear prefix-delegation route");
    }

err_out:
    if (p)
    {
        XFREE(MTYPE_TMP, p);
    }
    if (psu)
    {
        XFREE(MTYPE_SOCKUNION, psu);
    }
    if (addr)
    {
        XFREE(MTYPE_TMP, addr);
    }    
    return ret;
}
/*******************************************************************************
 * Name:    dhcp6_service_switch
 * Purpose: command for control dhcpv6 services
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp6_service_switch,
     dhcp6_service_switch_cmd,
     "service dhcpv6 (enable|disable)",
     "Modify use of network based services",
     "DHCPv6 service",
     "Enable this service",
     "Disable this service"
    )
{
    if (!pal_strcasecmp(argv[0], "enable"))
    {
        if (DHCPRELAY6_CHK_FLAG(DHCPV6_SERVICE_FLAG))
        {
            return CLI_SUCCESS;
        }
        
        DHCPRELAY6_SET_FLAG(DHCPV6_SERVICE_FLAG);
    }
    else
    {
        if (!DHCPRELAY6_CHK_FLAG(DHCPV6_SERVICE_FLAG))
        {
            return CLI_SUCCESS;
        }
        
        DHCPRELAY6_CLR_FLAG(DHCPV6_SERVICE_FLAG);
    }

    return dhcprelay6_service_onoff(
            DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:    dhcprelay6_service_switch
 * Purpose: command for control dhcpv6 relay feature
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_service_switch,
     dhcprelay6_service_switch_cmd,
     "dhcpv6 relay",
     "Specify DHCPv6 parameter",
     "Enable DHCPv6 relay feature"
    )
{
    if (DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG))
    {
        return CLI_SUCCESS;
    }

    DHCPRELAY6_SET_FLAG(DHCPV6_RELAY_SERVICE_FLAG);
    return dhcprelay6_service_onoff(
            DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:    no_dhcprelay6_service_switch
 * Purpose: command for control dhcpv6 relay feature
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcprelay6_service_switch,
     no_dhcprelay6_service_switch_cmd,
     "no dhcpv6 relay",
     "Negate a command or set its defaults",
     "Specify DHCPv6 parameter",
     "Enable DHCPv6 relay feature"
    )
{
    if (!DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG))
    {
        return CLI_SUCCESS;
    }

    DHCPRELAY6_CLR_FLAG(DHCPV6_RELAY_SERVICE_FLAG);
    return dhcprelay6_service_onoff(
            DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:    dhcprelay6_server_group
 * Purpose: command for dhcpv6-server group
 * Input: 
 *   ipv6 address list, format is X:X::X:X X:X::X:X ...
 *   Can input up to 8 ipv6 address
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_server_group,
     dhcprelay6_server_group_cmd,
     "dhcpv6-server <1-16> X:X::X:X (interface (IFPHYSICAL | IFVLAN | IFAGG)|)",
     "Specify DHCPv6 server parameter",
     "Specify DHCPv6 server group number",
     "Specify DHCPv6 server address. (X:X::X:X ...)",
     "Specify interface to DHCPv6 server for link-local address",
     CLI_IFNAME_PHY_STR ,
     CLI_IFNAME_VLAN_STR, 
     CLI_IFNAME_AGG_STR
    )
{
    int nRet = CLI_SUCCESS;
    unsigned long nGroup = pal_atoi(argv[0]);
    char *name = NULL;
    u_int8_t i;
    
    DHCPV6_RELAY_INIT_CHECK();

    if (argc >= 4)
    {
        name = argv[3];
        for (i = 0; i < pal_strlen(name); i++)
        {
            name[i] = pal_char_tolower(name[i]);
        }
        DHCPRELAY6_INTERFACE_CHECK(cli, name);
    }

    nRet = dhcprelay6_server_address_check(argv[1]);
    if (nRet == CLI_ERROR)
    {
        cli_out(cli, "%% Invalid ipv6 address\n");
        return CLI_ERROR;
    }
    
    if (nRet == -2 && !name)
    {
        cli_out(cli, "%% Interface is needed for link-local/multicast address\n");
        return CLI_ERROR;
    }
    else if (nRet == CLI_SUCCESS && name)
    {
        cli_out(cli, "%% Interface is not needed for global/site-local address\n");
        return CLI_ERROR;        
    }   

    nRet = dhcprelay6_serverlist_add(cli, nGroup, argv[1], name);
    return nRet;
}

/*******************************************************************************
 * Name:    no_dhcprelay6_server_group
 * Purpose: command for no dhcpv6-server group
 * Input: 
 *   Can input up to 8 ipv6 address
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcprelay6_server_group,
     no_dhcprelay6_server_group_cmd,
     "no dhcpv6-server <1-16> (X:X::X:X (interface (IFPHYSICAL |IFVLAN | IFAGG) |) |)",
     "Negate a command or set its defaults",
     "Specify DHCPv6 server group parameter",
     "Specify DHCPv6 server group number",
     "Specify DHCPv6 server address list. (X:X::X:X ...)",
     "Specify interface to DHCPv6 server",
     CLI_IFNAME_PHY_STR ,
     CLI_IFNAME_VLAN_STR, 
     CLI_IFNAME_AGG_STR
    )
{
    int nRet = CLI_SUCCESS;
    unsigned long nGroup = pal_atoi(argv[0]);
    char *name = NULL;

    DHCPV6_RELAY_INIT_CHECK();
    
    if (1 == argc)
    {
        /* del dhcpv6-server group */
        return dhcprelay6_serverlist_del_bygroup(cli, nGroup);
    }

    /* for bug 20735 modified by liuyang 2012-10-8*/
    if (argc >= 4)
    {
        name = argv[3];
        DHCPRELAY6_INTERFACE_CHECK(cli, name);
    }

    if (CLI_ERROR == (nRet = dhcprelay6_server_address_check(argv[1])))
    {
        cli_out(cli, "%% Invalid ipv6 address\n");
        return CLI_ERROR;
    }

    if (nRet == -2 && !name)
    {
        cli_out(cli, "%% Interface is needed for link-local/multicast address\n");
        return CLI_ERROR;
    }
    else if (nRet == CLI_SUCCESS && name)
    {
        cli_out(cli, "%% Interface is not needed for global/site-local address\n");
        return CLI_ERROR;        
    }

    nRet = dhcprelay6_serverlist_del(cli, nGroup, argv[1], name);
    return nRet;
}

/*******************************************************************************
 * Name:    dhcprelay6_remoteid_option_format
 * Purpose: command for dhcpv6 remote-id option format
 * Input: 
 *      vlan    include vlan info
 *      ifname include interface name
 *      duid    include system duid
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_remoteid_option_format,
     dhcprelay6_remoteid_option_format_cmd,
     "dhcpv6 relay remote-id format {vlan | ifname | duid}",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "Specify DHCPv6 remote-id option",
     "Specify DHCPv6 remote-id fromat",
     "Set vlan information in remote-id option",
     "Set interface information in remote-id option",
     "Set system duid in remote-id option"
    )
{
    int nRet = CLI_SUCCESS;
    int i;
    
    if (0 == argc)
    {
        g_dhcprelay6_remoteid_format = DHCPRELAY6_REMOTE_ID_KEYWORD_DEFAULT;
        return nRet;
    }

    g_dhcprelay6_remoteid_format = 0x0000;   
    for (i = 0;i < argc;i++)
    {
        if (!pal_strcmp(argv[i], "vlan"))
        {
            g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN;
        }
        else if (!pal_strcmp(argv[i], "ifname"))
        {
            g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME;        
        }
        else
        {
            g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_DUID;       
        }
    }

    return nRet;
}

CLI (no_dhcprelay6_remoteid_option_format,
     no_dhcprelay6_remoteid_option_format_cmd,
     "no dhcpv6 relay remote-id format",
     "Negate a command or set its defaults",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "Specify DHCPv6 remote-id option",
     "Specify DHCPv6 remote-id fromat"
    )
{
    g_dhcprelay6_remoteid_format = DHCPRELAY6_REMOTE_ID_KEYWORD_DEFAULT;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    dhcprelay6_remoteid_option
 * Purpose: command for dhcpv6 remote-id option
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_remoteid_option,
     dhcprelay6_remoteid_option_cmd,
     "dhcpv6 relay remote-id option",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "Specify DHCPv6 remote-id ",
     "Specify DHCPv6 option"
    )
{
    int nRet = CLI_SUCCESS;

    if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG))
    {
        return CLI_SUCCESS;
    }
    
    DHCPRELAY6_SET_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG);
    return nRet;
}

/*******************************************************************************
 * Name:    no_dhcprelay6_remoteid_option
 * Purpose: command for dhcpv6 remote-id option
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcprelay6_remoteid_option,
     no_dhcprelay6_remoteid_option_cmd,
     "no dhcpv6 relay remote-id option",
     "Negate a command or set its defaults",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "Specify DHCPv6 remote-id ",
     "Specify DHCPv6 option"
    )
{
    int nRet = CLI_SUCCESS;

    if (!DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG))
    {
        return CLI_SUCCESS;
    }
    
    DHCPRELAY6_CLR_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG);
    return nRet;
}

/*******************************************************************************
 * Name:    dhcprelay6_pd_route_distance
 * Purpose: command for dhcpv6 pd route distance
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_pd_route_distance,
     dhcprelay6_pd_route_distance_cmd,
     "dhcpv6 relay pd route distance <1-255>",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "DHCPv6 Prefix-Delegation",
     "Route information",
     "Route distance value",    
     "Specify default distance for pd route"
    )
{
    unsigned long number = strtoul(argv[0], NULL, 10);
    g_dhcprelay6_pdroute_distance = (u_int8_t)(number & 0xFF);
    return CLI_SUCCESS;
}

CLI (no_dhcprelay6_pd_route_distance,
     no_dhcprelay6_pd_route_distance_cmd,
     "no dhcpv6 relay pd route distance",
     "Negate a command or set its defaults",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "DHCPv6 Prefix-Delegation",
     "Route information",
     "Specify default distance for pd route"
    )
{
    g_dhcprelay6_pdroute_distance = DHCPRELAY6_PDROUTE_DISTANCE_DEFAULT;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    dhcprelay6_pd_route_advertise
 * Purpose: command for dhcpv6 pd option advertise
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcprelay6_pd_route_advertise,
     dhcprelay6_pd_route_advertise_cmd,
     "dhcpv6 relay pd route",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "DHCPv6 Prefix-Delegation",
     "Route information"
    )
{
    int nRet = CLI_SUCCESS;

    if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG))
    {
        return CLI_SUCCESS;
    }
    
    DHCPRELAY6_SET_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG);
    return nRet;
}

/*******************************************************************************
 * Name:    no_dhcprelay6_pd_route_advertise
 * Purpose: command for dhcpv6 pd option advertise
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcprelay6_pd_route_advertise,
     no_dhcprelay6_pd_route_advertise_cmd,
     "no dhcpv6 relay pd route",
     "Negate a command or set its defaults",
     "Specify DHCPv6 parameter",
     "Specify DHCPv6 relay parameter",
     "DHCPv6 Prefix-Delegation",
     "Route information"
    )
{
    if (!DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG))
    {
        return CLI_SUCCESS;
    }
    
    dhcprelay6_pdclient_delete_all(NULL);    
    DHCPRELAY6_CLR_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG);
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    show_dhcprelay6_pd_client
 * Purpose: show prefix-delegation client
 * Input: 
 * 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (show_dhcprelay6_pd_client,
     show_dhcprelay6_pd_client_cmd,
     "show dhcpv6 relay pd client",
     "Show configuration",
     "Show DHCPv6 configuration",
     "Show DHCPv6 relay configuration",
     "Show DHCPv6 prefix-delegation configuration",
     "Show DHCPv6 Client information"
    )
{
    dhcprelay6_show_pdclient(cli, NULL);
    return CLI_SUCCESS;
}

CLI (clear_dhcprelay6_pd_route,
     clear_dhcprelay6_pd_route_cmd,
     "clear dhcpv6 relay pd route (prefix X:X::X:X/M |)  (interface (IFPHYSICAL | IFVLAN | IFAGG)|) (X:X::X:X|)",
     "Reset",
     "DHCPv6 configuration",
     "DHCPv6 relay configuration",
     "DHCPv6 prefix-delegation configuration",
     "DHCPv6 prefix-delegation route",     
     "Specify Prefix information",
     "Prefix value",
     "Specify interface information",
     CLI_IFNAME_PHY_STR ,
     CLI_IFNAME_VLAN_STR, 
     CLI_IFNAME_AGG_STR,
     "Ipv6 address of pd client"
    )
{
    char *ip = NULL;
    char *prefix = NULL;
    char *ifname = NULL;

    switch (argc)
    {
    case 5:
        ip = argv[4];
    case 4:
        prefix = argv[1];
        ifname = argv[3];
        break;
    case 3:
        ip = argv[2];
    case 2:
        if (!pal_strncmp("prefix", argv[0], 6))
        {
            prefix = argv[1];
        }
        else 
        {
            ifname = argv[1];
        }
        break;
    case 1:
        ip = argv[0];
    case 0:
        break;
    default:
        cli_out(cli, "%% Wrong parameter number ");
        return CLI_ERROR;
    }
    
    return dhcprelay6_cli_pdroute_clear(cli, prefix, ip, ifname);
}

/*******************************************************************************
 * Name:    show_dhcprelay6_server_group
 * Purpose: show dhcpv6-server groups
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (show_dhcprelay6_server_group,
     show_dhcprelay6_server_group_cmd,
     "show dhcpv6-server (<1-16>|)",
     "Show configuration",
     "Show DHCPv6 server group configuration",
     "Specify group number"
    )
{
    unsigned long nGroup = 0;
    
    if (1 == argc)
    {
        nGroup = pal_atoi(argv[0]);
    }

    /* show group(s) */
    dhcprelay6_show_server_groups(cli, nGroup);
    return CLI_SUCCESS;
}

/*for bug 21628 added by liuyang 2012-12-21*/
CLI (show_dhcprelay6_cfg,
     show_dhcprelay6_cfg_cmd,
     "show dhcpv6 relay config",
     "Show configuration",
     "Show DHCPv6 configuration",
     "Show relay configuration",
     "Show current configuration"
    )
{
    cli_out(cli, "DHCPv6 relay configuration:\n");
    DHCPV6_OUT_SPLIT;
    
    cli_out(cli, "Remote-ID : %s\n", 
        DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG) ? "Enable" : "Disable");
    cli_out(cli, "Remote-ID Format : \"");
    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_DUID)
    {
        cli_out(cli, "DUID");
    }
    cli_out (cli, ":");
    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN)
    {
        cli_out(cli, "VLANID");            
    }
    cli_out (cli, ":");
    if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME)
    {
        cli_out(cli, "IFNAME");            
    }
    cli_out(cli, "\"\n");
    cli_out(cli, "Prefix-delegation Route : %s\n", 
        DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG) ? "Enable" : "Disable");
    cli_out(cli, "Prefix-delegation Route distance : %d\n", g_dhcprelay6_pdroute_distance);
    return CLI_SUCCESS;
}

/*for bug 21540 added by liuyang 2012-12-21*/
CLI (show_dhcprelay6_duid,
     show_dhcprelay6_duid_cmd,
     "show dhcpv6 duid",
     "Show configuration",
     "Show DHCPv6 configuration",
     "Show DHCPv6 DUID")
{
    cli_out (cli, "DHCPv6 unique identifier(DUID) : %s\n", 
        dhcprelay6_print_host_duid ());
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    show_dhcprelay6_statistics
 * Purpose: statistic command
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (show_dhcprelay6_statistics,
     show_dhcprelay6_statistics_cmd,
     "show dhcpv6 relay statistics",
     "Show configuration",
     "Show DHCPv6 configuration",
     "Show relay configuration",
     "Statistics information"
    )
{
    int i;
    
    cli_out(cli, "DHCPv6 relay packet statistics:\n");
    DHCPV6_OUT_SPLIT;
    /* for bug 21663 modified by liuyang 2012-12-24*/
    cli_out(cli, "Client relayed packets :  %d\n", g_dhcp_relay_pkt_stats.client6_relayed);
    cli_out(cli, "Server relayed packets :  %d\n", g_dhcp_relay_pkt_stats.server6_relayed);

    cli_out(cli, "Client error packets   :  %d\n", g_dhcp_relay_pkt_stats.client6_errors);
    cli_out(cli, "Server error packets   :  %d\n", g_dhcp_relay_pkt_stats.server6_errors);
    cli_out(cli, "Unknown type packets   :  %d\n\n", g_dhcp_relay_pkt_stats.unknown);   

    cli_out(cli, "DHCPv6 Msg type      Receive    Send       Error\n");
    for (i = DHCPV6_SOLICIT + DHCPV6_TYPE_BASE; 
            i < DHCP_ALL_PKT_TYPE_MAX; i++)
    {
        cli_out(cli, "%-20s %-10d %-10d %-10d\n", 
            dhcpv6_type_names[i - DHCPV6_TYPE_BASE],
            g_dhcp_relay_pkt_stats.recv[i],
            g_dhcp_relay_pkt_stats.send[i],
            g_dhcp_relay_pkt_stats.err[i]);        
    }
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    clear_dhcprelay6_statistics
 * Purpose: clear dhcpv6 relay statistics
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (clear_dhcprelay6_statistics,
     clear_dhcprelay6_statistics_cmd,
     "clear dhcpv6 relay statistics",
     "Reset",
     "DHCPv6",
     "Relay",
     "Statistics information"
     )
{
    pal_mem_set(&g_dhcp_relay_pkt_stats, 0, sizeof (dhcp_relay_pkt_stats_t));
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    dhcprelay6_cli_init
 * Purpose: initialize dhcpv6 relay module commands
 * Input:  
 *   ctree: client tree
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_cli_init(struct cli_tree *ctree)
{
    if (NULL == ctree)
    {
        return;
    }

#define DHCPRELAY6_CFG_MODE   CONFIG_MODE
#define DHCPRELAY6_SHOW_MODE  EXEC_MODE

    /* for bug 28957 add control by liuyang 2014-7-2*/
    if (module_control("IPV6") > 0)
    {
        /* configuration commands */
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &dhcprelay6_server_group_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcprelay6_server_group_cmd);
        cli_install_imi(ctree, DHCPRELAY6_CFG_MODE, PM_DHCPRELAY6|PM_NSM, PRIVILEGE_VR_MAX,
                        0, &dhcprelay6_service_switch_cmd);
        cli_install_imi(ctree, DHCPRELAY6_CFG_MODE, PM_DHCPRELAY6|PM_NSM, PRIVILEGE_VR_MAX, 
                        0, &no_dhcprelay6_service_switch_cmd);
        cli_install_imi(ctree, DHCPRELAY6_CFG_MODE, PM_DHCPRELAY6|PM_NSM, PRIVILEGE_VR_MAX,
                        0, &dhcp6_service_switch_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &dhcprelay6_remoteid_option_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcprelay6_remoteid_option_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &dhcprelay6_remoteid_option_format_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcprelay6_remoteid_option_format_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &dhcprelay6_pd_route_advertise_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcprelay6_pd_route_advertise_cmd);  
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &dhcprelay6_pd_route_distance_cmd);
        cli_install_gen(ctree, DHCPRELAY6_CFG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcprelay6_pd_route_distance_cmd);  

        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &clear_dhcprelay6_statistics_cmd);
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &clear_dhcprelay6_pd_route_cmd);


        /* show commands */
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcprelay6_server_group_cmd);
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcprelay6_pd_client_cmd);
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcprelay6_statistics_cmd);
        /*for bug 21628 added by liuyang 2012-12-21*/
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcprelay6_cfg_cmd);
        /*for bug 21540 added by liuyang 2012-12-21*/
        cli_install_gen(ctree, DHCPRELAY6_SHOW_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcprelay6_duid_cmd);

        /* debug commands */  
        DEBUG_CLI_INSTALL(dhcpv6, relay, ctree, DHCPRELAY6_SHOW_MODE, PM_DHCPRELAY6, PRIVILEGE_VR_MAX, 0);
    }

    return;
}

/*******************************************************************************
 * Name:    dhcprelay6_config_write_server_group
 * Purpose: write server group configuration
 * Input: 
 *   cli:  client pointer
 *   pGroup: group pointer
 *   pnLines: writed lines
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay6_config_write_server_group(struct cli *cli, S_Dhcpv6ServerGroup *pGroup, 
                                         int *pnLines)
{
    char szAddrBuf[SU_ADDRSTRLEN + 1];
    S_Dhcpv6ServerItem *pItem = NULL;

    if (NULL == cli || NULL == pGroup)
    {
        return;
    }

    if (TAILQ_EMPTY(&pGroup->itemHdr))
    {
        return;
    }   

    TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry)
    {
        cli_out(cli, "dhcpv6-server %u %s %s%s\n", 
                pGroup->nGroupIdx,
                sockunion2str(&pItem->su, szAddrBuf, SU_ADDRSTRLEN + 1),
                '\0' == pItem->name[0] ? "" : "interface ",
                pItem->name);
        (*pnLines)++;
    }
    return;
}   

/*******************************************************************************
 * Name:    dhcprelay6_config_write
 * Purpose: write dhcpv6 relay global based configuration
 * Input:  
 *   cli: client pointer
 * Output: 
 * Return: 
 * Note: 
 ******************************************************************************/
int dhcprelay6_config_write(struct cli *cli)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    int nLines = 0;
    
    if (NULL == cli)
    {
        return 0;
    }

    /* dhcpv6-server */
    TAILQ_FOREACH(pGroup, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        dhcprelay6_config_write_server_group(cli, pGroup, &nLines);
    }

    /* service dhcpv6 relay */
    if (DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG))
    {
        cli_out(cli, "dhcpv6 relay\n");
        nLines++;
    }

    if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG))
    {
        cli_out(cli, "dhcpv6 relay remote-id option\n");
        nLines++;
    }

    if (DHCPRELAY6_REMOTE_ID_KEYWORD_DEFAULT != g_dhcprelay6_remoteid_format)
    {
        cli_out(cli, "dhcpv6 relay remote-id format");
        if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_DUID)
        {
            cli_out(cli, " duid");
        }
        if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN)
        {
            cli_out(cli, " vlan");            
        }
        if (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME)
        {
            cli_out(cli, " ifname");            
        }
        cli_out(cli, "\n");
        nLines++;
    }

    if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG))
    {
        cli_out(cli, "dhcpv6 relay pd route\n");
        nLines++;
    }

    if (DHCPRELAY6_PDROUTE_DISTANCE_DEFAULT != g_dhcprelay6_pdroute_distance)
    {
        cli_out(cli, "dhcpv6 relay pd route distance %u\n", g_dhcprelay6_pdroute_distance);
        nLines++;        
    }

    return nLines;
}

#endif /* ! _CENTEC_ */

#endif

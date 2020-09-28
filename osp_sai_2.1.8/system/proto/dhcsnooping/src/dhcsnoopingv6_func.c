/****************************************************************************
 * $Id$
 *  DHCPv6 snooping
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Alexander Liu
 * Date          : 2009-02-01 10:44
 * Reason        : First Create.
 ****************************************************************************/

/****************************************************************************
 *
 * Header Files
 * 
 ****************************************************************************/
#include "dhcpd.h"
#include <netinet/ip6.h>
#include "netinet/ip.h"
#include "netinet/udp.h"

#include "proto.h"
#include "genlog.h"
#include "dhcsnooping_func.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "lib_fs.h"
#include <assert.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_IPV6

/****************************************************************************
 *
 * Defines and Macros
 * 
 ****************************************************************************/

/****************************************************************************
 *
 * Global and Declaration
 * 
 ****************************************************************************/
DEBUG_DECLAR(dhcp_debug);
S_DhcpSnoopingStat g_stStat6;

char *in6addr2str(struct in6_addr *in6);
int dhcp_snooping_write_file(u_int8_t bV4, u_int8_t bV6);
int dhcsnooping_reload_database(FILE *pFile, u_int8_t bIPv6, 
    int bSyslog, char *pszErrMsg, size_t nErrBufsz);
int dhcsnooping_prepare_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg,
    char *pszIntfName, struct packet *pDhcpPkt);
int dhcsnooping_release_check(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg,
    char *pszIntfName, struct packet *pDhcpPkt);
int dhcsnooping_del_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    char *pszIntfName, struct packet *pDhcpPkt);
int dhcsnooping_add_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    char *pszIntfName, struct packet *pDhcpPkt);
int dhcsnooping_send_pkt(S_DhcpSnoopingInHdr *pHdr, int nBufLen, 
    DhcSnoopingCMsg_t *pcmsg, int nAddrLen);

/****************************************************************************
 *
 * Function
 * 
 ****************************************************************************/
 
int
dhcsnoopingv6_get_mac_by_duid(S_DhcpSnoopingInHdr *pHdr, struct packet *pDhcpPkt, char *pszRet)
{
    struct option_cache *oc = NULL;
    struct data_string cid;
    u_int16_t cid_type;
    unsigned char *pszMac = NULL;

    if (NULL == pDhcpPkt->options
        || NULL == (oc = lookup_option(&dhcpv6_universe, 
                                       pDhcpPkt->options, D6O_CLIENTID))) {
        return -1;
    }

    memset(&cid, 0x00, sizeof(struct data_string));
    if (!evaluate_option_cache(&cid, pDhcpPkt, NULL,
                NULL, pDhcpPkt->options, NULL, &global_scope, oc, MDL)) {
        return -1;
    }

    cid_type = sal_ntoh16(*((u_int16_t *)cid.data));
    DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "client id: type %u, len %d", cid_type, cid.len);
    if ((1 == cid_type && 14 != cid.len) 
        || (3 == cid_type && 10 != cid.len)) {
        data_string_forget(&cid, MDL);
        return -1;
    }

    if (1 == cid_type) { /* DUID-LLT */
        memcpy(pszRet, cid.data + 8, ETH_ALEN);
    } else { /* DUID-LL */
        memcpy(pszRet, cid.data + 4, ETH_ALEN);
    }
    pszMac = (unsigned char *)pszRet;
    DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "client id: hardware type '%s' address %02x:%02x:%02x:%02x:%02x:%02x", 
            hardware_types[ntohs(*(((u_int16_t *)cid.data) + 1)) & 0xFF],
            pszMac[0], pszMac[1], pszMac[2], pszMac[3], pszMac[4], pszMac[5]);
    data_string_forget(&cid, MDL);

    return 0;
}

 void
dhcsnoopingv6_ia_destroy(struct dhc6_ia **src, const char *file, int line)
{
    struct dhc6_addr *addr, *naddr;
    struct dhc6_ia *ia;

    if (src == NULL || *src == NULL) {
        return;
    }
    ia = *src;

    for(addr = ia->addrs ; addr != NULL ; addr = naddr) {
        naddr = addr->next;

    if (addr->options != NULL)
        option_state_dereference(&addr->options, file, line);

        dfree(addr, file, line);
    }

    if (ia->options != NULL)
        option_state_dereference(&ia->options, file, line);

    dfree(ia, file, line);
    *src = NULL;
    return;
}

void
dhcsnoopingv6_bindings_destroy(struct dhc6_ia **ppbindings, const char *file, int line)
{
    struct dhc6_ia *ia, *nia;

    if (NULL == ppbindings)
    {
        return;
    }

    for (ia = *ppbindings ; ia != NULL ; ia = nia) 
    {
        nia = ia->next;
        dhcsnoopingv6_ia_destroy(&ia, file, line);
    }

    return;
}

isc_result_t
dhcsnoopingv6_parse_addrs(struct dhc6_addr **paddr, struct packet *packet,
        struct option_state *options)
{
    struct data_string ds;
    struct option_cache *oc;
    struct dhc6_addr *addr;

    memset(&ds, 0, sizeof(ds));

    oc = lookup_option(&dhcpv6_universe, options, D6O_IAADDR);
    for ( ; oc != NULL ; oc = oc->next) {
        addr = dmalloc(sizeof(*addr), MDL);
        if (addr == NULL) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
            "Out of memory allocating, address structure.");
            return ISC_R_NOMEMORY;
        } else if (evaluate_option_cache(&ds, packet, NULL, NULL,
        options, NULL, &global_scope,
        oc, MDL) &&
        (ds.len >= 24)) {

            addr->address.len = 16;
            memcpy(addr->address.iabuf, ds.data, 16);
            addr->starts = cur_time;
            addr->preferred_life = getULong(ds.data + 16);
            addr->max_life = getULong(ds.data + 20);

            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "  | | X-- IAADDR %s",
            piaddr(addr->address));
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "  | | | X-- Preferred lifetime %u.",
            addr->preferred_life);
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "  | | | X-- Max lifetime %u.",
            addr->max_life);

            /*
            * RFC 3315 section 22.6 says we must discard
            * addresses whose pref is later than valid.
            */
            if ((addr->preferred_life > addr->max_life)) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                "  | | | !-- INVALID lifetimes, "
                "IAADDR discarded.  Check your "
                "server configuration.");
                dfree(addr, MDL);
                data_string_forget(&ds, MDL);
                continue;
            }

            /*
            * Fortunately this is the last recursion in the
            * protocol.
            */
            if (ds.len > 24) {
                if (!option_state_allocate(&addr->options,
                MDL)) {
                    DEBUG_OUT(dhcp_debug, flags,
                    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
                    "Out of memory allocating, IAADDR option state.");
                    dfree(addr, MDL);
                    data_string_forget(&ds, MDL);
                    return ISC_R_NOMEMORY;
                }

                if (!parse_option_buffer(addr->options,
                ds.data + 24,
                ds.len - 24,
                &dhcpv6_universe)) {
                    DEBUG_OUT(dhcp_debug, flags,
                    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                    "Corrupt IAADDR options.");
                    option_state_dereference(&addr->options,
                    MDL);
                    dfree(addr, MDL);
                    data_string_forget(&ds, MDL);
                    return ISC_R_BADPARSE;
                }
            }

            if (addr->options != NULL)
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "  | | | X-- [Options]");

            data_string_forget(&ds, MDL);

            *paddr = addr;
            paddr = &addr->next;
        } else {
            DEBUG_OUT(dhcp_debug, flags,
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
            "Invalid IAADDR option cache.");
            dfree(addr, MDL);
            if (ds.len != 0)
            data_string_forget(&ds, MDL);
            return ISC_R_UNEXPECTED;
        }
    }
    delete_option(&dhcpv6_universe, options, D6O_IAADDR);

    return ISC_R_SUCCESS;
}

isc_result_t
dhcsnoopingv6_parse_ia_na(struct dhc6_ia **pia, struct packet *packet,
        struct option_state *options)
{
    struct data_string ds;
    struct dhc6_ia *ia;
    struct option_cache *oc;
    isc_result_t result;

    memset(&ds, 0, sizeof(ds));

    oc = lookup_option(&dhcpv6_universe, options, D6O_IA_NA);
    for ( ; oc != NULL ; oc = oc->next) {
        ia = dmalloc(sizeof(*ia), MDL);
        if (ia == NULL) {
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
            "Out of memory allocating IA_NA structure.");
            return ISC_R_NOMEMORY;
        } else if (evaluate_option_cache(&ds, packet, NULL, NULL,
        options, NULL,
        &global_scope, oc, MDL) &&
        ds.len >= 12) {
            memcpy(ia->iaid, ds.data, 4);
            ia->ia_type = D6O_IA_NA;
            ia->starts = cur_time;
            ia->renew = getULong(ds.data + 4);
            ia->rebind = getULong(ds.data + 8);

            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "Parse IA_NA");
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  X-- IA_NA %s",
            print_hex_1(4, ia->iaid, 59));
            /* XXX: This should be the printed time I think. */
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  | X-- starts %u",
            (unsigned)ia->starts);
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  | X-- t1 - renew  +%u", ia->renew);
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  | X-- t2 - rebind +%u", ia->rebind);

            /*
            * RFC3315 section 22.4, discard IA_NA's that
            * have t1 greater than t2, and both not zero.
            * Since RFC3315 defines this behaviour, it is not
            * an error - just normal operation.
            *
            * Note that RFC3315 says we MUST honor these values
            * if they are not zero.  So insane values are
            * totally OK.
            */
            if ((ia->renew > 0) && (ia->rebind > 0) &&
            (ia->renew > ia->rebind)) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
                "  | !-- INVALID renew/rebind times, IA_NA discarded.");
                dfree(ia, MDL);
                data_string_forget(&ds, MDL);
                continue;
            }

            if (ds.len > 12) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
                "  | X-- [Options]");

                if (!option_state_allocate(&ia->options,
                MDL)) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
                "Out of memory allocating IA_NA option state.");
                dfree(ia, MDL);
                data_string_forget(&ds, MDL);
                return ISC_R_NOMEMORY;
                }

                if (!parse_option_buffer(ia->options,
                ds.data + 12,
                ds.len - 12,
                &dhcpv6_universe)) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
                "Corrupt IA_NA options.");
                option_state_dereference(&ia->options,
                 MDL);
                dfree(ia, MDL);
                data_string_forget(&ds, MDL);
                return ISC_R_BADPARSE;
                }
            }
            data_string_forget(&ds, MDL);

                if (ia->options != NULL) {
                result = dhcsnoopingv6_parse_addrs(&ia->addrs, packet,
                ia->options);
                if (result != ISC_R_SUCCESS) {
                option_state_dereference(&ia->options,
                 MDL);
                dfree(ia, MDL);
                return result;
                }
            }

            while (*pia != NULL)
            pia = &(*pia)->next;
            *pia = ia;
            pia = &ia->next;
        } else {
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
            "Invalid IA_NA option cache.");
            dfree(ia, MDL);
            if (ds.len != 0)
            data_string_forget(&ds, MDL);
            return ISC_R_UNEXPECTED;
        }
    }

    return ISC_R_SUCCESS;
}

isc_result_t
dhcsnoopingv6_parse_ia_ta(struct dhc6_ia **pia, struct packet *packet,
        struct option_state *options)
{
    struct data_string ds;
    struct dhc6_ia *ia;
    struct option_cache *oc;
    isc_result_t result;

    memset(&ds, 0, sizeof(ds));

    oc = lookup_option(&dhcpv6_universe, options, D6O_IA_TA);
    for ( ; oc != NULL ; oc = oc->next) {
        ia = dmalloc(sizeof(*ia), MDL);
        if (ia == NULL) {
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
            "Out of memory allocating IA_TA structure.");
            return ISC_R_NOMEMORY;
        } else if (evaluate_option_cache(&ds, packet, NULL, NULL,
        options, NULL,
        &global_scope, oc, MDL) &&
        ds.len >= 4) {
            memcpy(ia->iaid, ds.data, 4);
            ia->ia_type = D6O_IA_TA;
            ia->starts = cur_time;

            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "Parse IA_TA");
            DEBUG_OUT(dhcp_debug, flags,
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  X-- IA_TA %s", print_hex_1(4, ia->iaid, 59));
            /* XXX: This should be the printed time I think. */
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "  | X-- starts %u", (unsigned)ia->starts);

            if (ds.len > 4) {
                DEBUG_OUT(dhcp_debug, flags, 
                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, "  | X-- [Options]");

                if (!option_state_allocate(&ia->options,
                MDL)) {
                    DEBUG_OUT(dhcp_debug, flags, 
                    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                    "Out of memory allocating IA_TA option state.");
                    dfree(ia, MDL);
                    data_string_forget(&ds, MDL);
                    return ISC_R_NOMEMORY;
                }

                if (!parse_option_buffer(ia->options,
                ds.data + 4,
                ds.len - 4,
                &dhcpv6_universe)) {
                    DEBUG_OUT(dhcp_debug, flags, 
                    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, "Corrupt IA_TA options.");
                    option_state_dereference(&ia->options,
                    MDL);
                    dfree(ia, MDL);
                    data_string_forget(&ds, MDL);
                    return ISC_R_BADPARSE;
                }
            }
            data_string_forget(&ds, MDL);

            if (ia->options != NULL) {
                result = dhcsnoopingv6_parse_addrs(&ia->addrs, packet,
                ia->options);
                if (result != ISC_R_SUCCESS) {
                    option_state_dereference(&ia->options,
                    MDL);
                    dfree(ia, MDL);
                    return result;
                }
            }

            while (*pia != NULL)
            pia = &(*pia)->next;
            *pia = ia;
            pia = &ia->next;
        } else {
            DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, "Invalid IA_TA option cache.");
            dfree(ia, MDL);
            if (ds.len != 0)
            data_string_forget(&ds, MDL);
            return ISC_R_UNEXPECTED;
        }
    }

    return ISC_R_SUCCESS;
}

int 
dhcsnoopingv6_parse_status_code(struct option_state *options, unsigned *code)
{
    struct option_cache *oc;
    struct data_string ds;
    int ret = -1;

    if ((options == NULL) || (code == NULL)) {
        return -1;
    }
    memset(&ds, 0, sizeof(ds));
    
    /* Assume success if there is no option. */
    *code = STATUS_Success;

    oc = lookup_option(&dhcpv6_universe, options, D6O_STATUS_CODE);
    if ((oc != NULL) &&
        evaluate_option_cache(&ds, NULL, NULL, NULL, options,
        NULL, &global_scope, oc, MDL)) {
        if (ds.len >= 2) {
            *code = getUShort(ds.data);
            ret = 0;
        }
        data_string_forget(&ds, MDL);
    }
    return ret;
}

int
dhcsnoopingv6_is_server_side_msg(unsigned char type)
{
    switch (type)
    {
    case DHCPV6_ADVERTISE:
    case DHCPV6_REPLY:
    case DHCPV6_RECONFIGURE:
    case DHCPV6_RELAY_REPL:
    case DHCPV6_LEASEQUERY_REPLY:
        return 1;
    default:
        break;
    }
    return 0;
}

void 
dhcsnoopingv6_dump_packet_option(struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    memset (&ds, 0, sizeof ds);

    if (u != &dhcpv6_universe) {
        name = u->name;
        dot = ".";
    } else {
        name = "";
        dot = "";
    }
    if (evaluate_option_cache (&ds, packet, lease, client,
                   in_options, cfg_options, scope, oc, MDL)) {
        DEBUG_SHOW(dhcp_debug, flags, "  option %s%s%s %s;",
                   name, dot, oc->option->name,
                   pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
        data_string_forget (&ds, MDL);
    }
    return;
}

void
dhcsnoopingv6_dump_packet (struct packet *packet)
{
    int i;

    if (NULL == packet || !(IS_DEBUG_ON(dhcp_debug, flags, 
                                TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS)))
    {
        return;
    }

    DEBUG_SHOW(dhcp_debug, flags, "Dump DHCPv6 packet fields");
    DEBUG_SHOW(dhcp_debug, flags, " packet length %d", packet->packet_length);
    DEBUG_SHOW(dhcp_debug, flags, " packet type %s", 
               dhcpv6_type_names[packet->dhcpv6_msg_type]);
    DEBUG_SHOW(dhcp_debug, flags, " transaction id: 0x%02x%02x%02x",
               packet->dhcpv6_transaction_id[2],
               packet->dhcpv6_transaction_id[1],
               packet->dhcpv6_transaction_id[0]);
    DEBUG_SHOW(dhcp_debug, flags, " hop count: %d", packet->dhcpv6_hop_count);
    DEBUG_SHOW(dhcp_debug, flags, " link address: %s", 
            in6addr2str(&packet->dhcpv6_link_address));
    DEBUG_SHOW(dhcp_debug, flags, " peer address: %s", 
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
                dhcsnoopingv6_dump_packet_option);
    }
    return;
}

void
dhcsnooping_save_timer6()
{
    /* XXX */
    DEBUG_OUT(dhcp_debug, flags, 
        TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
        "save dhcpv6 snooping binding database");

    dhcp_snooping_write_file(0, 1);
    return;
}

int init_binding_table6()
{
    FILE *fp = NULL;
    int ret = 0;
    char szErrMsg[256];

    if (NULL == (fp = sal_fopen(DHCPSNOOPING_SAVE_BINDING_FILE6, "r"))) {
        if (ENOENT == errno) {
            /* no such file */
            return 0;
        }

        log_sys(M_MOD_DHCPSNOOPING6, E_ERROR, 
            "load dhcpv6 snooping binding item failed: can't open file: %s",
            strerror(errno));
        return -1;
    }

    ret = dhcsnooping_reload_database(fp, 1, 1, szErrMsg, 256);
    sal_fclose(fp);
    return ret;
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

int
dhcsnooping_decode_dhcpv6(S_DhcpSnoopingInHdr *pHdr,
        DhcSnoopingCMsg_t *pcmsg, struct packet *decoded_packet)
{
    unsigned char msg_type;
    const struct dhcpv6_packet *msg;
    const struct dhcpv6_relay_packet *relay; 
    int len;

    if (NULL == pHdr || NULL == decoded_packet)
    {
        return -1;
    }
    len = pHdr->nDhcpLen;

    if (!packet6_len_okay((char *)pHdr->dhcpv6, len)) 
    {
        DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
            "short packet (len %d), dropped", len);
        return -1;
    }

    if (!option_state_allocate(&decoded_packet->options, MDL)) 
    {
        log_sys(M_MOD_DHCPSNOOPING6, E_ALERT, "no memory for options");
        return -1;
    }

    decoded_packet->raw = (struct dhcp_packet *) pHdr->dhcpv6;
    decoded_packet->packet_length = (unsigned) len;
#if 0
    decoded_packet->client_port = from_port;
    decoded_packet->client_addr = *from;
    decoded_packet->unicast = was_unicast;
#endif

    msg_type = *((u_char *)pHdr->dhcpv6);
    if ((msg_type == DHCPV6_RELAY_FORW) || 
        (msg_type == DHCPV6_RELAY_REPL)) 
    {
        relay = (const struct dhcpv6_relay_packet *)pHdr->dhcpv6;
        decoded_packet->dhcpv6_msg_type = relay->msg_type;

        /* relay-specific data */
        decoded_packet->dhcpv6_hop_count = relay->hop_count;
        memcpy(&decoded_packet->dhcpv6_link_address,
               relay->link_address, sizeof(relay->link_address));
        memcpy(&decoded_packet->dhcpv6_peer_address,
               relay->peer_address, sizeof(relay->peer_address));
        if (!parse_option_buffer(decoded_packet->options, 
             relay->options, len-sizeof(*relay), 
             &dhcpv6_universe))
        {
            /* no logging here, as parse_option_buffer() logs all
               cases where it fails */
            return -1;
        }
    }
    else if (msg_type < dhcpv6_type_name_max)
    {
        msg = (const struct dhcpv6_packet *)pHdr->dhcpv6;
        decoded_packet->dhcpv6_msg_type = msg->msg_type;

        /* message-specific data */
        memcpy(decoded_packet->dhcpv6_transaction_id, 
            msg->transaction_id, 
            sizeof(decoded_packet->dhcpv6_transaction_id));

        if (!parse_option_buffer(decoded_packet->options, 
            msg->options, len-sizeof(*msg), 
            &dhcpv6_universe))
        {
            /* no logging here, as parse_option_buffer() logs all
            cases where it fails */
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int
dhcsnoopingv6_validate_pkt(S_DhcpSnoopingInHdr *pHdr)
{
    struct ip6_hdr *iph = NULL;
    int nUdpLen = 0;
    int nDhcpLen = 0;
    u_int32_t sum = 0, usum = 0;

    if (NULL == pHdr || pHdr->nTotalLen <= 0)
    {
        return -1;
    }

    if (pHdr->nTotalLen < sizeof(struct ip6_hdr))
    {
        ++g_stStat6.ullInvalid6;
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                 "drop too small packet");
        return -1;
    }

    iph = (struct ip6_hdr *)&pHdr->uip6h;
    if (iph->ip6_nxt != IPPROTO_UDP)
    {
        ++g_stStat6.ullInvalid6;
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                 "drop unknown next header packet %u", iph->ip6_nxt);
        return -1;
    }

    if ((pHdr->nTotalLen - sizeof(struct ip6_hdr)) != ntohs(iph->ip6_plen))
    {
        ++g_stStat6.ullInvalid6;
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                 "payload length %d disagrees with bytes received %d",
                 ntohs(iph->ip6_plen),
                 (pHdr->nTotalLen - sizeof(struct ip6_hdr)));
        return -1;
    }

    pHdr->udp = (struct udphdr *)((u_char *)iph + sizeof(struct ip6_hdr));
    nUdpLen = pHdr->nTotalLen - sizeof(struct ip6_hdr);
    if (nUdpLen < sizeof(struct udphdr)
        || nUdpLen != sal_ntohs(pHdr->udp->uh_ulen))
    {
        ++g_stStat6.ullInvalid6;
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                 "bogus UDP packet length: %d", nUdpLen);
        return -1;
    }

    /*for bug 21517 added by liuyang 2012-12-25*/
    if (pHdr->udp->uh_dport != sal_htons(546)
        && pHdr->udp->uh_dport != sal_htons(547))
    {
        ++g_stStat6.ullInvalid6;
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                 "unknown UDP dest port number: %d", ntohs(pHdr->udp->uh_dport));
        return -1;
    }
    
    nDhcpLen = nUdpLen - sizeof(struct udphdr);
    pHdr->dhcpv6 = (struct dhcpv6_packet *)((u_char *)pHdr->udp + sizeof(struct udphdr));
    pHdr->nDhcpLen = nDhcpLen;

    /*for bug 21517 added by liuyang 2012-12-25*/   
    usum =  pHdr->udp->uh_sum;
    pHdr->udp->uh_sum = 0;
    sum = wrapsum(checksum((u_char *)pHdr->udp, sizeof (struct udphdr),
                checksum((u_char *)pHdr->dhcpv6, nDhcpLen, checksum((unsigned char *)&iph->ip6_dst,
                sizeof iph->ip6_dst, checksum((unsigned char *)&iph->ip6_src, sizeof iph->ip6_src, 
                IPPROTO_UDP + (u_int32_t)nUdpLen)))));
    if (usum && usum != sum) 
    {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                  "drop bad udp checksums packet");
        ++g_stStat6.ullInvalid6;
        return -1;
    }

    pHdr->udp->uh_sum = usum;
    /* for bug 21681 modified by liuyang 2012-12-26*/
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, "validate ipv6 and udp header for DHCPv6 packet success");
    return 0;
}

int
dhcsnoopingv6_snooping_relay_forw(S_DhcpSnoopingInHdr *pHdr,
        DhcSnoopingCMsg_t *pcmsg, tbl_interface_t *pIntf, struct packet *porigpkt)
{
	const struct dhcpv6_packet *msg;
    struct option_cache *oc;
	//const struct dhcpv6_relay_packet *relay; 
    struct data_string relay_msg;
    struct packet *pnewpkt = NULL;
    int len;

    if (NULL == pHdr || NULL == pcmsg || NULL == pIntf
        || NULL == porigpkt || NULL == porigpkt->options) {
        return -1;
    }

    /* Get the relay-msg option (carrying the message to relay). */
    oc = lookup_option(&dhcpv6_universe, porigpkt->options, D6O_RELAY_MSG);
    if (NULL == oc) {
        return -1;
    }

    sal_memset(&relay_msg, 0x00, sizeof(struct data_string));
    if (!evaluate_option_cache(&relay_msg, porigpkt, NULL, NULL,
                porigpkt->options, NULL, &global_scope, oc, MDL) ||
            (relay_msg.len < sizeof(struct dhcpv6_packet))) {
        DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, "Can't evaluate relay-msg.");
        return -1;
    }

    if (!packet_allocate(&pnewpkt, MDL)) {
        log_sys(M_MOD_DHCPSNOOPING6, E_ALERT, "No memory for parse relay forw/repl options.");
        data_string_forget(&relay_msg, MDL);
        return -1;
    }

    if (!option_state_allocate(&pnewpkt->options, MDL)) {
        log_sys(M_MOD_DHCPSNOOPING6, E_ALERT, "No memory for parse relay forw/repl options.");
        packet_dereference(&pnewpkt, MDL);
        data_string_forget(&relay_msg, MDL);
        return -1;
    }

    msg = (const struct dhcpv6_packet *)relay_msg.data;
    pnewpkt->dhcpv6_msg_type = msg->msg_type;
    len = pHdr->nDhcpLen;
    pnewpkt->raw = (struct dhcp_packet *) msg;
    pnewpkt->packet_length = (unsigned) len - sizeof(*msg);

    if (DHCPV6_RELAY_FORW == pnewpkt->dhcpv6_msg_type
        || DHCPV6_RELAY_REPL == pnewpkt->dhcpv6_msg_type) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                  "multi relay forw/repl message, does not snooping it.");
        packet_dereference(&pnewpkt, MDL);
        data_string_forget(&relay_msg, MDL);
        return 0;
    }

    /* message-specific data */
    memcpy(pnewpkt->dhcpv6_transaction_id, 
            msg->transaction_id, 
            sizeof(pnewpkt->dhcpv6_transaction_id));

    if (!parse_option_buffer(pnewpkt->options, 
                msg->options, len-sizeof(*msg), 
                &dhcpv6_universe)) {
        packet_dereference(&pnewpkt, MDL);
        data_string_forget(&relay_msg, MDL);
        return -1;
    }

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
            "DHCPv6 message in the %s packet is",
            dhcpv6_type_names[porigpkt->dhcpv6_msg_type]);
    dhcsnoopingv6_dump_packet(pnewpkt);

    if (DHCPV6_REQUEST == pnewpkt->dhcpv6_msg_type
        || DHCPV6_RENEW == pnewpkt->dhcpv6_msg_type
        || DHCPV6_REBIND == pnewpkt->dhcpv6_msg_type
        || (DHCPV6_SOLICIT == pnewpkt->dhcpv6_msg_type 
            && lookup_option(&dhcpv6_universe, pnewpkt->options, D6O_RAPID_COMMIT))) {
        dhcsnooping_prepare_bindingitem(pHdr, pcmsg, pIntf->key.name, pnewpkt);
    }

    if (DHCPV6_RELEASE == pnewpkt->dhcpv6_msg_type
        || DHCPV6_DECLINE == pnewpkt->dhcpv6_msg_type) {
        if (!dhcsnooping_release_check(pHdr, pcmsg, pIntf->key.name, pnewpkt)) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                      "drop binding database mismatched %s",
                      dhcpv6_type_names[pnewpkt->dhcpv6_msg_type]);
            packet_dereference(&pnewpkt, MDL);
            data_string_forget(&relay_msg, MDL);
            return -1;
        }
    
        /* del binding table */
        dhcsnooping_del_bindingitem(pHdr, pcmsg, pIntf->key.name, pnewpkt);
    }

    if (DHCPV6_REPLY == pnewpkt->dhcpv6_msg_type) {
        if (pIntf->dhcp_snooping_trust) {
            dhcsnooping_add_bindingitem(pHdr, pcmsg, pIntf->key.name, pnewpkt);
        } else {
            packet_dereference(&pnewpkt, MDL);
            data_string_forget(&relay_msg, MDL);
            return -1;
        }
    }

    packet_dereference(&pnewpkt, MDL);
    data_string_forget(&relay_msg, MDL);
    return 0;
}

int
dhcsnoopingv6_snooping_relay_repl(S_DhcpSnoopingInHdr *pHdr,
        DhcSnoopingCMsg_t *pcmsg, tbl_interface_t *pIntf, struct packet *porigpkt)
{
    return dhcsnoopingv6_snooping_relay_forw(pHdr, pcmsg, pIntf, porigpkt);
}

int
dhcsnoopingv6_do_snooping(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    tbl_interface_t *pIntf, struct packet *pDhcpPkt)
{
    u_int bSrvMsg = 0; /* is it server side message ? */
    u_char szDestIp[128];
    u_char *pSrcMac = NULL;
    u_char *pDstMac = NULL;
    u_int32_t code = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
        
    if (NULL == pDhcpPkt || NULL == pIntf || NULL == pHdr || NULL == pcmsg)
    {
        return -1;
    }

    sal_snprintf((char*)szDestIp, 128, "%s", in6addr2str(&pHdr->uip6h->ip6_dst));

    if (IS_DEBUG_ON(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS)) {
        pSrcMac = pcmsg->src_mac;
        pDstMac = pcmsg->dest_mac;

        DEBUG_SHOW(dhcp_debug, flags,  
                  "%s on %s %02x:%02x:%02x:%02x:%02x:%02x > "
                  "%02x:%02x:%02x:%02x:%02x:%02x %s > %s",
                  dhcpv6_type_names[pDhcpPkt->dhcpv6_msg_type], IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), 
                  pSrcMac[0], pSrcMac[1], pSrcMac[2], pSrcMac[3], pSrcMac[4], pSrcMac[5],
                  pDstMac[0], pDstMac[1], pDstMac[2], pDstMac[3], pDstMac[4], pDstMac[5],
                  in6addr2str(&pHdr->uip6h->ip6_src), szDestIp);
    }
    dhcsnoopingv6_dump_packet(pDhcpPkt);

    bSrvMsg = dhcsnoopingv6_is_server_side_msg(pDhcpPkt->dhcpv6_msg_type);

    /* process for untrust port */
    if (!(pIntf->dhcp_snooping_trust)) {
        if (bSrvMsg) {
            /* invalidate server side message from untrusted port */
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                      "drop server side v6 message from untrusted port: %s message %s",
                      IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), dhcpv6_type_names[pDhcpPkt->dhcpv6_msg_type]);
            return -1;
        }
    } else {
        /* process for trust port */
        if (DHCPV6_REPLY == pDhcpPkt->dhcpv6_msg_type
            && pDhcpPkt->options) {
            dhcsnoopingv6_parse_status_code(pDhcpPkt->options, &code);
            if (!code) {
                /* add binding table */
                dhcsnooping_add_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
            } else {
                /* del binding table */
                dhcsnooping_del_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);                
            }
        }
#if 0
        else if (DHCPV6_RELAY_REPL == pDhcpPkt->dhcpv6_msg_type) {
            if (0 != dhcsnoopingv6_snooping_relay_repl(pHdr, pcmsg, ifp, pDhcpPkt)) {
                return -1;
            }
        }
#endif
    }

    /* check interface for DHCPDECLINE & DHCPRELEASE message */
    if (DHCPV6_RELEASE == pDhcpPkt->dhcpv6_msg_type
        || DHCPV6_DECLINE == pDhcpPkt->dhcpv6_msg_type) {
        /* drop mismatched interface message */
        if (!dhcsnooping_release_check(pHdr, pcmsg, pIntf->key.name, pDhcpPkt)) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                      "drop binding database mismatched %s",
                      dhcpv6_type_names[pDhcpPkt->dhcpv6_msg_type]);
            return -1;
        }
    
        /* del binding table */
        dhcsnooping_del_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
    }

    /* prepare binding item
     * 1) DHCPV6_SOLICIT + Rapid Commit option
     * 2) DHCPV6_REQUEST
     */
    if (DHCPV6_REQUEST == pDhcpPkt->dhcpv6_msg_type
        || DHCPV6_RENEW == pDhcpPkt->dhcpv6_msg_type
        || DHCPV6_REBIND == pDhcpPkt->dhcpv6_msg_type
        || (DHCPV6_SOLICIT == pDhcpPkt->dhcpv6_msg_type 
            && lookup_option(&dhcpv6_universe, pDhcpPkt->options, D6O_RAPID_COMMIT))) {
        dhcsnooping_prepare_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
    }
#if 0
    else if (DHCPV6_RELAY_FORW == pDhcpPkt->dhcpv6_msg_type) {
        if (0 != dhcsnoopingv6_snooping_relay_forw(pHdr, pcmsg, ifp, pDhcpPkt)) {
            return -1;
        }
    }
#endif
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
              "forward %s from %s to %s.",
              dhcpv6_type_names[pDhcpPkt->dhcpv6_msg_type],
              in6addr2str(&pHdr->uip6h->ip6_src), szDestIp);
    return 0;
}

int
dhcsnoopingv6_process_pkt(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg)
{
    tbl_dhcsnooping_t *pDhcSnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
    DhcSnoopingCMsg_t *pVlan = pcmsg;
    tbl_interface_t *pIntf = NULL;
    struct packet stDhcpPkt;
    int nRet = 0;
    int bSrvSideMsg = 0;
    int bSecPort = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    if (NULL == pHdr || NULL == pcmsg) {
        return -1;
    }

    /* XXX: Alexander, By pass IPv6 */
    goto out;

    /* locate interface */
    if (NULL == (pIntf = tbl_interface_get_interface_by_ifindex(pVlan->ifindex))) { 
        /* can't locate interface, resend this packet */
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, 
                  "forward DHCPv6 message for unknown interface (ifindex %u)",
                  pVlan->ifindex);
        goto out;
    }

    /* decode dhcp packets */
    sal_memset(&stDhcpPkt, 0x00, sizeof(struct packet));
    nRet = dhcsnooping_decode_dhcpv6(pHdr, pcmsg, &stDhcpPkt);
    if (nRet) {
        DEBUG_OUT(dhcp_debug, flags, 
            TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR, "decode DHCP message failed");
        ++g_stStat6.ullInvalid6;
        goto out;
    }    
    ++g_stStat6.ullDhcpPkts6;
    
    bSecPort = (pIntf->dhcp_snooping_trust);
    bSrvSideMsg = dhcsnoopingv6_is_server_side_msg(stDhcpPkt.dhcpv6_msg_type);
    if (!bSecPort && bSrvSideMsg) {
            /* invalidate server side message from untrusted port */
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                  "drop server side message from untrusted port: %s message %s",
                  IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), dhcpv6_type_names[stDhcpPkt.dhcpv6_msg_type]);
 
        /* drop this packet for snooping failed */
        ++g_stStat6.ullDropped6;
        nRet = -1;
        goto out;
    }
    
    /* check snoopinged vlan */
    if (pVlan->vlanid > GLB_VLAN_MAX
        || !GLB_BMP_ISSET(pDhcSnoopingCfg->vlans, pVlan->vlanid)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                "forward DHCPv6 message for unsnooping VLAN %d", pVlan->vlanid);
        goto out;
    }

    /* for bug 21681 modified by liuyang 2012-12-26*/        
    if (!bSecPort && DHCPV6_RELAY_FORW == stDhcpPkt.dhcpv6_msg_type)
    {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                "drop DHCPv6 RELAY_FORW message from untrusted port: %s message %s",
                 IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), dhcpv6_type_names[stDhcpPkt.dhcpv6_msg_type]);
        ++g_stStat6.ullDropped6;
        nRet = -1;
        goto out;
    }

    if (DHCPV6_RELAY_REPL == stDhcpPkt.dhcpv6_msg_type
        ||  DHCPV6_RELAY_FORW == stDhcpPkt.dhcpv6_msg_type) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                "forward DHCPv6 RELAY_REPL/RELAY_FORW message for VLAN %d", 
                pVlan->vlanid);
        goto out;
    }

    nRet = dhcsnoopingv6_do_snooping(pHdr, pcmsg, pIntf, &stDhcpPkt);
    if (0 != nRet) {
        /* drop this packet for snooping failed */
        ++g_stStat6.ullDropped6;
    }

out:
    if (!nRet )
    {
        ++g_stStat6.ullForwarded6;
        nRet = dhcsnooping_send_pkt(pHdr, pHdr->nTotalLen, 
                              pcmsg, sizeof(DhcSnoopingCMsg_t));        
        if (nRet)
        {
            log_sys(M_MOD_DHCPSNOOPING6, E_ERROR, "write DHCPv6 snooping socket %s", "failed");
        }
    }

    if (NULL != stDhcpPkt.options)
    {
        option_state_dereference(&stDhcpPkt.options, MDL);
    }
    if (NULL != pHdr->bindings)
    {
        dhcsnoopingv6_bindings_destroy(&pHdr->bindings, MDL);
        pHdr->bindings = NULL;
    }

    return nRet;
}

int
dhcsnoopingv6_get_one(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pmsg)
{
    /* do sanity check */
    if (dhcsnoopingv6_validate_pkt(pHdr) != 0)
    {
        /* for bug 21681 modified by liuyang 2012-12-26*/
        return -1;
    }

    /* process DHCPv6 packet */
    return dhcsnoopingv6_process_pkt(pHdr, pmsg);
}

#endif


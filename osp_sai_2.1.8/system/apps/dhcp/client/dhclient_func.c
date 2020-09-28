/****************************************************************************
* $Id$
*  Implement for DHCP client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-06-16 09:01
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <signal.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "proto.h"
#include "dhcpd.h"
#include "dhcp_version.h"

#include "ctclib_hash.h"
#include "ctc_msg.h"
#include "dhclient_func.h"
#ifdef HAVE_BM            
#include "bmlib.h"
#endif
#include "param_check.h"

#include "dhc_common.h"
#include "dhclient_func.h"

#ifdef HAVE_SMARTCFG /* Modified by Alexander Liu 2011-07-23 16:39 */
#include "scfgctl.h"
#endif /* !HAVE_SMARTCFG */


#include "genlog.h"
#include "glb_const.h"
#include "ctc_task.h"
#include "ctc_sock.h"
#include "glb_tempfile_define.h"

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_dhcpv6_debug_define.h"
#include "gen/tbl_dhcpv6_debug.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
/* modified by liwh for bug 50196, 2019-01-07 */
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
/* liwh end */

#include "cdb_ds.h"
#include "glb_macro.h"
#include "glb_debug_define.h"
#include "dhclient_func.h"

/* modified by liwh for bug 44686, 2018-11-16 */
#include "lib_fs.h"
#include "glb_ip_define.h"
/* liwh end */

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
int
dhclient_recv_event(char *ifname, int what);
void dhclient_add_duid(struct interface_info *ip);
void dhclient_del_duid(struct interface_info *ip);

/**************** temp ***********/

/* be careful when change this value */
#define DHC_DISP_BUF_SIZE                   1024
#define DHC_ERR_MSG_BUF_LEN                 256
#define DHC_MAX_ROUTERS_RECORDS             8
#define DHC_PREFIX_BUF_LEN                  64
#define DHC_MAX_TFTPSRV_BUF_LEN             128
#define DHC_MAX_TFTPSRV_RECORDS             8

#define DHC_MAX_OPTION_3_BYTES         32 /* support 8 records */
#define DHC_MAX_OPTION_33_BYTES        64 /* support 8 records */
#define DHC_MAX_OPTION_121_BYTES       72 /* support 8 records */
#define DHC_MAX_OPTION_150_BYTES       32 /* support 8 records */
#define DHC_ROUTERS_MAX_LEN            256 /* routers buffer length */

#define DHC_RELEASE_WAIT_INTERVAL       1500 /* msec */

#define DHC_MAX_DISTANCE                255
/**************** temp *******/

#define DHC_DEBUG 

#ifndef NSM_IF_TYPE_L2
#define NSM_IF_TYPE_L2        1
#endif

#define DHC_OUT_SPLIT(fp)   \
sal_fprintf(fp, "============================================================\n")

dhc_stats_t g_pkt_stats;

extern struct option **default_requested_options;
DEBUG_DECLAR(dhcp_debug);
DEBUG_DECLAR(dhcpv6_debug);

extern void do_init6(void *input);



/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
dhc_control_block_t g_dhc_ctl_blk = 
{
    .flags = DHCLIENT_DEFAULT_FLAGS,
    .distance = DHCLIENT_DEFAULT_ROUTER_DISTANCE,
};
int g_dhc_initialized = 0; /* dhcp client initialized flag */

struct interface_info *free_intf_list = NULL;

/* global read / write / exception fd sets */
//static struct thread_select_arg stThreadSelArg;

struct lib_globals *ZG = NULL;
char *config_file = NULL;

extern u_int16_t local_port;
extern omapi_io_object_t omapi_io_states;
extern dhc_stats_t g_pkt_stats;
extern dhc6_stats_t g_pkt_dhc6_stats;

extern int timeval_cmp (struct sal_timeval a, struct sal_timeval b);
extern void make_decline(struct client_state *, struct client_lease *);
extern void send_decline (void *);
extern void destroy_client_lease(struct client_lease *lease);
extern struct timeout *get_timeout(void (*where) (void *), void *what);
extern void state_init (void *cpp);
char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

int
dhclient_get_clientid_by_interface(char *cur, char *ifname, char *obuf, int *olen);
int dhclient_proc_req_lists(char *ifname, 
        u_int32_t flags, int badd, dhc_req_option_t *arr);

/****************************************************************************
 *
* Function
* 
****************************************************************************/
#define _COMM_TOUPPER(c)  (c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c)
#define _COMM_TOLOWER(c)  (c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c)

#define _COMM_HEX2VAL(s) \
        ((isalpha(s) ? (_COMM_TOLOWER(s)-'a'+10) : (_COMM_TOLOWER(s)-'0')) & 0xf)
#define _COMM_VAL2HEX(s)  ((s) + (((s) >= 10) ? ('a'-10) : '0'))
int
comm_bin2hex(const u_char *input, size_t len, u_char *output)
{
    const u_char *ip = input;
    u_char  *op = output;

    while (ip - input < (int) len) {
        *op++ = _COMM_VAL2HEX((*ip >> 4) & 0xf);
        *op++ = _COMM_VAL2HEX(*ip & 0xf);
        ip++;
    }
    *op = '\0';

    return 2 * len + 1;
}

int
comm_hex2bin(const u_char *input, size_t len, u_char *output)
{
    u_char *op = output;
    const u_char *ip = input;

    *op = 0;
    if (len % 2) {
        if (!isxdigit(*ip))
        {
            return -1;
        }
        *op++ = _COMM_HEX2VAL(*ip);
        ip++;
    }

    while (ip - input < (int) len) {
        if (!isxdigit(*ip))
        {
            return -1;
        }
        *op = _COMM_HEX2VAL(*ip) << 4;
        ip++;

        if (!isxdigit(*ip))
        {
            return -1;
        }
        *op++ += _COMM_HEX2VAL(*ip);
        ip++;
    }

    return (len / 2) + (len % 2);
}

union sockunion *
sockunion_str2su (char *str)
{
  int ret;
  union sockunion *su;

  su = malloc(sizeof (union sockunion));
#ifdef _CENTEC_ 
  if( !su )
  {
    return NULL;
  }
#endif

  ret = sal_inet_pton (AF_INET, str, &su->sin.sin_addr);
  if (ret > 0)			/* Valid IPv4 address format. */
    {
      su->sin.sin_family = AF_INET;
#ifdef HAVE_SIN_LEN
      su->sin.sin_len = sizeof(struct pal_sockaddr_in4);
#endif /* HAVE_SIN_LEN */
      return su;
    }
#ifdef HAVE_IPV6
  ret = sal_inet_pton (AF_INET6, str, &su->sin6.sin6_addr);
  if (ret > 0)			/* Valid IPv6 address format. */
    {
      su->sin6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
      su->sin6.sin6_len = sizeof(struct sal_sockaddr_in6);
#endif /* SIN6_LEN */
      return su;
    }
#endif /* HAVE_IPV6 */

  free (su);
  return NULL;
}

char *
sockunion_su2str (union sockunion *su)
{
  char str[INET6_ADDRSTRLEN];

  switch (su->sa.sa_family)
    {
    case AF_INET:
      sal_inet_ntop (AF_INET, &su->sin.sin_addr, str, sizeof (str));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      sal_inet_ntop (AF_INET6, &su->sin6.sin6_addr, str, sizeof (str));
      break;
#endif /* HAVE_IPV6 */
    }
  return strdup (str);
}

uint8
ip_masklen (struct sal_in4_addr netmask)
{
  uint8 len;
  uint8 *pnt;
  uint8 *end;
  uint8 val;

  len = 0;
  pnt = (u_int8_t *) &netmask;
  end = pnt + 4;

  /*modified by dingyi for bug7692, 2008-12-08*/
  #ifndef _CENTEC_
  while ((*pnt == 0xff) && pnt < end)
  #else
  while (pnt < end && (*pnt == 0xff))
  #endif
    {
      len+= 8;
      pnt++;
    }

  if (pnt < end)
    {
      val = *pnt;
      while (val)
	{
	  len++;
	  val <<= 1;
	}
    }
  return len;
}

/*******************************************************************************
 * Name:    dhclient_dump_packet_option
 * Purpose: dump dhcp packet option for debug
 * Input: 
 *   oc:
 *   packet: dhcp packet
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
dhclient_dump_packet_option(struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    memset (&ds, 0, sizeof ds);

    if (u != &dhcp_universe) 
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
        DEBUG_SHOW(dhcp_debug, flags, "  option %s%s%s %s;",
                   name, dot, oc->option->name,
                   pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
        data_string_forget (&ds, MDL);
    }
    return;
}

/*******************************************************************************
 * Name:    dhclient_dump_packet 
 * Purpose: dump packet fields
 * Input: 
 *   packet: packet after decode
 *   len: packet length
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
dhclient_dump_packet (struct packet *tp)
{
	struct dhcp_packet *tdp;

    if (NULL == tp || NULL == tp->raw)
    {
        return;
    }

    if (NULL == tp ||!(IS_DEBUG_ON(dhcp_debug, flags,TBL_DHCP_DEBUG_FLD_CLIENT_PACKET)))
    {
        return;
    }

    tdp = tp -> raw;
    DEBUG_SHOW(dhcp_debug, flags,  "Dump DHCP packet fields:");
	DEBUG_SHOW(dhcp_debug, flags,  "packet length %d", tp -> packet_length);
	DEBUG_SHOW(dhcp_debug, flags, "op = %d  htype = %d  hlen = %d  hops = %d",
	       tdp -> op, tdp -> htype, tdp -> hlen, tdp -> hops);
	DEBUG_SHOW(dhcp_debug, flags, "xid = 0x%x  secs = %ld  flags = 0x%x",
	       sal_ntoh32(tdp -> xid), (unsigned long)tdp -> secs, sal_ntoh16(tdp -> flags));
	DEBUG_SHOW(dhcp_debug, flags, "ciaddr = %s", inet_ntoa (tdp -> ciaddr));
	DEBUG_SHOW(dhcp_debug, flags, "yiaddr = %s", inet_ntoa (tdp -> yiaddr));
	DEBUG_SHOW(dhcp_debug, flags, "siaddr = %s", inet_ntoa (tdp -> siaddr));
	DEBUG_SHOW(dhcp_debug, flags, "giaddr = %s", inet_ntoa (tdp -> giaddr));
	DEBUG_SHOW(dhcp_debug, flags, "chaddr = %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
	       ((unsigned char *)(tdp -> chaddr)) [0],
	       ((unsigned char *)(tdp -> chaddr)) [1],
	       ((unsigned char *)(tdp -> chaddr)) [2],
	       ((unsigned char *)(tdp -> chaddr)) [3],
	       ((unsigned char *)(tdp -> chaddr)) [4],
	       ((unsigned char *)(tdp -> chaddr)) [5]);
	DEBUG_SHOW(dhcp_debug, flags, "filename = %s", tdp -> file);
	DEBUG_SHOW(dhcp_debug, flags, "server_name = %s", tdp -> sname);
	if (tp -> options_valid) {
		int i;

		for (i = 0; i < tp -> options -> universe_count; i++) {
			if (tp -> options -> universes [i]) {
				option_space_foreach (tp, (struct lease *)0,
						      (struct client_state *)0,
						      (struct option_state *)0,
						      tp -> options,
						      &global_scope,
						      universes [i], 0,
						      dhclient_dump_packet_option);
			}
		}
	}
    return;
}

void
dhclient_dump_dhcp_packet (struct dhcp_packet *tdp, int len)
{
    struct packet stTP;
    struct packet *tp = &stTP;
    
    if (NULL == tdp)
    {
        return;
    }
    if (!(IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_PACKET))
            && !(IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_DUMP)))
    {
        return;
    }
    
    sal_memset(&stTP, 0x00, sizeof(struct packet));
    tp->raw = tdp;
    tp->packet_length = len;

    if (!parse_options(tp))
    {
        if (tp->options)
        {
            option_state_dereference(&tp->options, MDL);
            packet_dereference(&tp, MDL);
        }
    }

    if (IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_PACKET))
    {
        dhclient_dump_packet(&stTP);
    }

    DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_DUMP,
            "DUMP dhcp message (send)",
            (u_char *)&stTP, len, 0);
    
    if (tp->options)
    {
        option_state_dereference(&tp->options, MDL);
    }
    return;
}

void
reset_dhclient_statistics()
{
    memset(&g_pkt_stats, 0, sizeof(dhc_stats_t));
}

void output_dhclient_statistics(FILE *fp)
{
    fprintf(fp, "/DHCP OFFER=%d", g_pkt_stats.recv_offer);
    fprintf(fp, "/DHCP ACK=%d", g_pkt_stats.recv_ack);
    fprintf(fp, "/DHCP NAK=%d", g_pkt_stats.recv_nak);
    fprintf(fp, "/DHCP Others=%d", g_pkt_stats.recv_other);
    fprintf(fp, "/DHCP DISCOVER=%d", g_pkt_stats.send_discover);
    fprintf(fp, "/DHCP DECLINE=%d", g_pkt_stats.send_decline);
    fprintf(fp, "/DHCP RELEASE=%d", g_pkt_stats.send_release);
    fprintf(fp, "/DHCP REQUEST=%d", g_pkt_stats.send_release);
    fprintf(fp, "/DHCP packet send failed=%d\r\n", g_pkt_stats.send_failed);
    return;
}


/*******************************************************************************
 * Name:    dhclient_stop
 * Purpose: shutdown dhclient PM
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhclient_stop(void)
{
    dhclient_mgmt_free();
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
void sighup (int sig)
{
    DHCLIENT_LOG_INFO("SIGHUP received");
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
void sigint (int sig)
{
    DHCLIENT_LOG_INFO("SIGINT received");
    
    dhclient_stop ();
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
void sigusr1 (int sig)
{
    DHCLIENT_LOG_INFO("SIGUSR1 received");
}

/*******************************************************************************
 * Name:    signal_init 
 * Purpose: Initialize signal handles
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void signal_init ()
{
    signal(SIGHUP, sighup);
    signal(SIGINT, sigint);
    signal(SIGTERM, sigint);
    signal(SIGUSR1, sigusr1);
}

char*
dhclient_get_mgmt_if_name()
{
    static char mgt_name[8] = {0};
    FILE* fp;
    char buf[128];
    
    if('\0' == mgt_name[0])
    {
        fp = sal_fopen(GLB_CPU_IF_INFO_FILE, SAL_OPEN_RO);
        if(fp==NULL)
        {
            sal_strcpy(mgt_name, "eth0");
        }
        else
        {
            while(sal_fgets (buf, 128, fp))
            {
                if (sal_sscanf(buf, "MGMT_IF %s", mgt_name))
                {                
                    break;
                }
            }
            sal_fclose(fp);
        }
    }
    return mgt_name;
}

int 
_is_mgmt_dev(char *ifname)
{
    return strcasecmp(ifname, dhclient_get_mgmt_if_name()) ? 0 : 1;
}

char * 
_get_intf_display_name(char *ifname)
{
    static char ifname_ext[IFNAME_EXT_SIZE];
    
    if (_is_mgmt_dev(ifname))
    {
        return "management";
    }

    IFNAME_ETH2FULL(ifname, ifname_ext);
    return ifname_ext;
}

/*******************************************************************************
 * Name:    dhclient_init_hardware_address
 * Purpose: get hardware address for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhclient_init_hardware_address(struct interface_info *tmp)
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
    /* bind to vrf */
    ///pal_sock_set_bindtofib (sock, tmp->vrfid);

    if (!tmp->ifp) 
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
        strcpy (tif->ifr_name, tmp->name);
        tmp->ifp = tif;
    }

    /* Read the hardware address from this interface. */
    ifr = *tmp->ifp;
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
    case ARPHRD_ETHER:
        tmp->hw_address.hlen = 7;
        tmp->hw_address.hbuf [0] = ARPHRD_ETHER;
        memcpy (&tmp->hw_address.hbuf [1], sa.sa_data, 6);

        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Initialize hardware address to ETHER %02x.%02x.%02x.%02x.%02x.%02x",
                tmp->hw_address.hbuf [1],
                tmp->hw_address.hbuf [2],
                tmp->hw_address.hbuf [3],
                tmp->hw_address.hbuf [4],
                tmp->hw_address.hbuf [5],
                tmp->hw_address.hbuf [6]);
        }
        else
        {
            DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Initialize hardware address to ETHER %02x.%02x.%02x.%02x.%02x.%02x",
                tmp->hw_address.hbuf [1],
                tmp->hw_address.hbuf [2],
                tmp->hw_address.hbuf [3],
                tmp->hw_address.hbuf [4],
                tmp->hw_address.hbuf [5],
                tmp->hw_address.hbuf [6]);    
        }
        break;

    case ARPHRD_IEEE802:
        tmp->hw_address.hlen = 7;
        tmp->hw_address.hbuf [0] = ARPHRD_IEEE802;
        memcpy (&tmp->hw_address.hbuf [1], sa.sa_data, 6);

        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Initialize hardware address to IEEE802 %02x.%02x.%02x.%02x.%02x.%02x",
                tmp->hw_address.hbuf [1],
                tmp->hw_address.hbuf [2],
                tmp->hw_address.hbuf [3],
                tmp->hw_address.hbuf [4],
                tmp->hw_address.hbuf [5],
                tmp->hw_address.hbuf [6]);
        }
        else
        {
            DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Initialize hardware address to IEEE802 %02x.%02x.%02x.%02x.%02x.%02x",
                tmp->hw_address.hbuf [1],
                tmp->hw_address.hbuf [2],
                tmp->hw_address.hbuf [3],
                tmp->hw_address.hbuf [4],
                tmp->hw_address.hbuf [5],
                tmp->hw_address.hbuf [6]);    
        }
        break;

    default:
        log_error("%s: unknown hardware address type %d",
                  ifr.ifr_name, sa.sa_family);
        break;
    }

    close(sock);
    return 0;
}

/*******************************************************************************
 * Name:    dhclient_locate_interface
 * Purpose: locate interface 
 * Input: 
 *   name: interface name
 *   list: interface list
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhclient_locate_interface(
        struct interface_info *list, char *name)
{
    struct interface_info *ip = NULL;

    for (ip = list; ip; ip = ip->next) 
    {
        if (!strcmp(ip->name, name))
        {
            return ip;
        }
    }
    
	return NULL;
}

struct interface_info *dhclient_locate_interface_by_ifindex(
        struct interface_info *list, int32 ifindex)
{
    struct interface_info *ip = NULL;

	for (ip = list; ip; ip = ip->next) 
    {
        /* modified by liuyang 2011-11-24 */
        if (ip->ifindex == ifindex)
        {
            return ip;
        }
	}
    
	return NULL;
}

void
_dhclient_add_into_intf_list(struct interface_info **list, struct interface_info *ip)
{
    if (*list) {
        interface_reference (&ip->next, *list, MDL);
        interface_dereference (list, MDL);
    }
    interface_reference (list, ip, MDL);
    return;
}
/* merge bug 19775 modified by liuyang 2012-7-30 */
void
_dhclient_remove_from_intf_list(struct interface_info **list, struct interface_info *tmp)
{
    struct interface_info *last, *ip;

    /*last = 0;
    interface_reference(&last, tmp, MDL);*/
    last = 0;
    for (ip = *list; ip; ip = ip->next) {
        if (!strcmp (ip->name, tmp->name)) {
            /* Remove from list */
            if (last) {
                ip = (struct interface_info *)0;
                interface_reference (&ip, last->next, MDL);
                interface_dereference (&last->next, MDL);
                if (ip->next) {
                    interface_reference (&last->next, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            } else {
                ip = (struct interface_info *)0;
                interface_reference (&ip, *list, MDL);
                interface_dereference (list, MDL);
                if (ip->next) {
                    interface_reference (list, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            }
            if (ip->client)
            {
                tmp->client = ip->client;
                tmp->client->interface = tmp;
            }
            break;
        }
        last = ip;
    }

    interface_dereference(&ip, MDL);
    return;
}

int
_dhclient_prepare_config(struct interface_info *ip)
{
    struct client_state *client = NULL;
    struct client_config *config = NULL;
    struct ifreq *tif = NULL;

    client = (struct client_state *)dmalloc(sizeof(struct client_state), MDL);
    if (NULL == client)
    {
        DHCLIENT_LOG_ERR("Warning: Out of memory");
        goto err_out;
    }

    config = (struct client_config *)dmalloc(sizeof(struct client_config), MDL);
    if (NULL == config)
    {
        DHCLIENT_LOG_ERR("Warning: Out of memory");
        goto err_out;
    }
    /* merge bug 19775 by liuyang 2012-7-30 */
    if (!ip->ifp)
    {
        if (NULL == (tif = (struct ifreq *)dmalloc (sizeof (struct ifreq), MDL)))
        {
            goto err_out;
        }
        ip->ifp = tif;
    }
    strcpy (ip->ifp->ifr_name, ip->name);

    ip->client = client;
    ip->client->config = config;
    client->interface = ip;
    client->state = S_STOPPED;

    config->timeout = 60;
    config->select_interval = 0;
    config->reboot_timeout = 10;
    config->retry_interval = 300;
    config->backoff_cutoff = 15;
    config->initial_interval = 3;
    config->bootp_policy = P_ACCEPT;
    config->script_name = NULL;
    config->requested_options = default_requested_options;
    config->requested_flags = M_DHC_REQ_DEFAULT;
    config->omapi_port = -1;
    config->do_forward_update = 1;

    if (AF_INET == dhclient_get_local_family())
    {
        config->requested_lease = M_DHC_DEFAULT_LEASE_TIME;
        
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
            "Prepare config for interface %s",
            _get_intf_display_name(ip->name));
    }
    /* modified by liwh for bug 50538, 2018-12-25 */
    else
    {
        config->requested_lease = 7200;
        config->iana_t1 = config->requested_lease / 2;
        config->iana_t2 = config->iana_t1 + (config->iana_t1 / 2);

        DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
            "Prepare config for interface %s",
            _get_intf_display_name(ip->name));
    }
    /* liwh end */
     
    return 1;

err_out:    
    if (NULL != config)
    {
        dfree(config, MDL);
    }

    if (NULL != client)
    {
        dfree(client, MDL);
    }

    if (NULL != tif)
    {
        dfree(tif, MDL);
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhclient_register_interface
 * Purpose: register interface to dhcp client interface list
 * Input: 
 *   name:  interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: merge bug 19775 by liuyang 2012-7-30
 ******************************************************************************/
struct interface_info *dhclient_register_interface(char *name)
{
    struct interface_info *tmp = (struct interface_info *)0;
    struct interface_info *ip = NULL;
    isc_result_t status = ISC_R_SUCCESS;

    if (NULL != (tmp = dhclient_locate_interface(interfaces, name)))
    {
        return tmp;
    }

    /* get from free_intf_list */
    if (NULL == (tmp = dhclient_locate_interface(free_intf_list, name)))
    {
#if 0
        if (GetSystemStartupFinishedFlag())
        {
            DHCLIENT_LOG_ERR("Locate interface %s failed when enable client function",
                    _get_intf_display_name(name));
            return NULL;
        }
#endif
        /* when startup we add interface to list
         * dhclient_recv_interface_add will register r/w fdesc
         * and startup state machine later
         */
        if (ISC_R_SUCCESS != (status = interface_allocate (&tmp, MDL)))
        {
            DHCLIENT_LOG_ERR("Warning: Out of Memory!");
            return NULL;
        }
        snprintf(tmp->name, IFNAMSIZ, name);
        if (!_dhclient_prepare_config(tmp))
        {
            DHCLIENT_LOG_ERR("Warning: Out of Memory!");
            interface_dereference(&tmp, MDL);
            return NULL;
        }
        tmp->index = -1;
        tmp->rfdesc = tmp->wfdesc = -1;
        tmp->vrfid = 0;
        tmp->flags &= ~INTERFACE_SHUTDOWN; /* setting shutdown flag */
        tmp->client->state = S_STOPPED;
        _dhclient_add_into_intf_list(&interfaces, tmp);
        ip = tmp;
        interface_dereference(&tmp, MDL);
        return ip;
    }

    if (!tmp->client)
    {
        if (!_dhclient_prepare_config(tmp))
        {
            DHCLIENT_LOG_ERR("Warning: out of memory!");
            return NULL;
        }
    }

    if (AF_INET == dhclient_get_local_family())
    {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
            "Enable dhcp client function for interface %s",
            _get_intf_display_name(name));
    }
    else
    {       
        /* modified by liwh for bug 49365, 2018-11-27 */
        if (tmp->client->config && tmp->client->config->requested_options)
        {
            tmp->client->config->requested_options = default_requested_options;         
        }
        /*liwh end */
    
        DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
            "Enable dhcpv6 client function for interface %s",
            _get_intf_display_name(name));   
    }

    /* hold interface for move it*/
    interface_reference(&ip, tmp, MDL);
    _dhclient_remove_from_intf_list(&free_intf_list, tmp);
    tmp->index = -1;
    tmp->client->state = S_STOPPED;
    interface_snorf(tmp, INTERFACE_REQUESTED);
    if (0 == tmp->hw_address.hlen)
    {
        dhclient_init_hardware_address(tmp);
    }
    /* Register the interface... */
    if_register_receive (tmp);
    if_register_send (tmp);
    /* interface_stash (tmp); */
    
    /* Now register the interfaces as protocols. */
    if (tmp->rfdesc != -1)
    {
        status = omapi_register_io_object((omapi_object_t *)tmp,
                if_readsocket, 0, got_one, 0, 0);
        if (ISC_R_SUCCESS != status)
        {
            DHCLIENT_LOG_ERR("Can't register I/O handle for %s: %s",
                    tmp->name, isc_result_totext(status));
            if_deregister_send (tmp);
            if_deregister_receive (tmp);
            _dhclient_remove_from_intf_list(&interfaces, tmp);
            _dhclient_add_into_intf_list(&free_intf_list, tmp);
            interface_dereference(&ip, MDL);
            return NULL;
        }
    }
    
    /* ip = tmp; */
    interface_dereference(&ip, MDL);
    return tmp;
}
void dhclient_unregister_interface(char *name)
{
    struct interface_info *interface = NULL;
    struct interface_info *ip = NULL;
    struct interface_info *last = NULL;

    /* remove from interfaces */
    last = 0;
    for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (!strcmp(ip->name, name))
        {
            if (NULL != ip->remote_id)
            {
                free(ip->remote_id);
                ip->remote_id = NULL;
                ip->remote_id_len = 0;
                free(ip->circuit_id);
                ip->circuit_id = (u_int8_t *)ip->name;
                ip->circuit_id_len = strlen(ip->name);
            }
            interface_reference (&interface, ip, MDL);
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

    /* remove the io object */
    omapi_unregister_io_object ((omapi_object_t *)interface);
    if_deregister_send (interface);
    if_deregister_receive (interface);
    interface_dereference(&interface, MDL);
    return;
}

void
_dhclient_free_option_list(struct option **ppopts)
{
    struct option *op;
    int idx;

    if (NULL == ppopts || ppopts == default_requested_options)
    {
        return;
    }

    idx = 0;
    do
    {
        op = ppopts[idx];
        option_dereference(&op, MDL);
        idx++;
    } while (ppopts[idx]);

    dfree(ppopts, MDL);
    return;
}

void
_dhclient_free_intf_vars(struct interface_info *ip)
{
    struct client_state *client;

    if (NULL == ip)
    {
        return;
    }

    if (NULL != ip->remote_id)
    {
        free(ip->remote_id);
        ip->remote_id = NULL;
        ip->remote_id_len = 0;
        free(ip->circuit_id);
        ip->circuit_id = (u_int8_t *)ip->name;
        ip->circuit_id_len = sal_strlen(ip->name);
    }

    if (NULL != (client = ip->client))
    {
        struct client_config *config;
        if (NULL != (config = client->config))
        {
            _dhclient_free_option_list(config->requested_options);
            config->requested_options = NULL;
            dfree(config, MDL);
        }
        dfree(client, MDL);
        ip->client = NULL;
    }

    if (NULL != ip->ifp)
    {
        dfree(ip->ifp, MDL);
        ip->ifp = NULL;
    }

    return;
}

/* merge bug 19775 by liuyang 2012-7-30 */
void _dhclient_disable_interface(struct interface_info *ip)
{
    struct interface_info *interface = NULL;
    if (NULL == ip)
    {
        return;
    }
    
    if (AF_INET == dhclient_get_local_family())
    {
        /* for bug 24807 modified by liuyang 2013-9-24*/
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Disable dhcp client function for interface %s",
                _get_intf_display_name(ip->name));
    }
    else
    {
        DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Disable dhcpv6 client function for interface %s",
                _get_intf_display_name(ip->name));
    }

    /* hold interface for move it to free list */
    interface_reference(&interface, ip, MDL);
    _dhclient_remove_from_intf_list(&interfaces, ip);
    omapi_unregister_io_object ((omapi_object_t *)ip);
    if_deregister_send (ip);
    if_deregister_receive (ip);
    if (AF_INET == dhclient_get_local_family())
    {
        if (ip->client && ip->client->active)
        {
            destroy_client_lease (ip->client->active);
            ip->client->active = NULL;
        }
    }
    /* modified by liwh for bug 49365, 2018-11-28 */
    else
    {
        if (ip->client && ip->client->active_lease)
        {
            dhc6_lease_destroy(&ip->client->active_lease, MDL);
	     ip->client->active_lease = NULL;
        }
    }
    /* liwh end */
    
    _dhclient_add_into_intf_list(&free_intf_list, ip);
    interface_dereference(&interface, MDL);
    return;
}

/* added by liuyang 2012-4-20*/
int dhclient_option_clientid_del(dhc_send_options_t *popts)
{
    dhc_send_options_t *p = popts;
    if (NULL == popts )
        return RESULT_ERROR;

    p->client_type[0] = '\0';
    if (NULL != p->client_id)
    {
        dfree(p->client_id, MDL);
        p->client_id = NULL;
        p->client_id_len = 0;
    }
    return RESULT_OK;
}

/* added by liuyang 2012-4-26*/
int _dhclient_clientid_del_from_intf_list(struct interface_info *list, char *ifname)
{
    dhc_send_options_t *popts = NULL;
    struct interface_info *ip = NULL;
    
    if (NULL == list || NULL == ifname)
        return RESULT_ERROR;

    for (ip = list; ip; ip = ip->next) 
    {
        if (ip->client && ip->client->config 
            && (popts = (dhc_send_options_t *)ip->client->config->sdata))
        {
            if (!strcmp(popts->client_type, ifname))
            {
                dhclient_option_clientid_del(popts);
            }
        }
    }
    return RESULT_OK;
}

/* added by liuyang 2012-4-27*/
int _dhclient_clientid_update_from_intf_list(struct interface_info *list, struct interface_info *ifp)
{
    dhc_send_options_t *popts = NULL;
    struct interface_info *ip = NULL;
    char *mem = NULL;
    
    if (NULL == list || NULL == ifp)
        return RESULT_ERROR;
    /*beacuse mac address update was more slowly in nsm than here , just clear old mac */
    for (ip = list; ip; ip = ip->next) 
    {
        if (ip->client && ip->client->config 
            && (popts = (dhc_send_options_t *)ip->client->config->sdata))
        {
            if (!strcmp(popts->client_type, ifp->name))
            {   /* client_id_len == 0 means this interface has a empty client-id, so it doesn't need to be clear,just skip it*/
                if (NULL != popts->client_id && 0 != popts->client_id_len)
                {
                    dfree(popts->client_id, MDL);
                }
                else
                {
                    continue;
                }
                mem = dmalloc(1, MDL); /* 1 more bytes for '\0' */
                if (NULL == mem)
                {
                    DHCLIENT_LOG_ERR("%% Update option clientid for interface list failed (out of memory)\n");
                    return RESULT_ERROR;
                }
                mem[0] = '\0';
                
                popts->client_id_len = 0;
                popts->client_id = mem;  
            }
        }
    }
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhclient_delete_interface
 * Purpose: destroy interface
 * Input: 
 *   name: interface name
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhclient_delete_interface(struct interface_info **list, char *name)
{
    struct interface_info *ip = NULL;
    struct interface_info *last = NULL;

    /* remove from interface list */
    last = 0;
    for (ip = *list; ip; ip = ip->next) 
    {
        if (!strcmp(ip->name, name))
        {
            if (AF_INET == dhclient_get_local_family())
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                    "Delete interface %s",
                    _get_intf_display_name(ip->name));
            }
            else
            {
                DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                    "Delete interface %s",
                    _get_intf_display_name(ip->name));    
            }

            /* free memories */
            _dhclient_free_intf_vars(ip);

            if (last) 
            {
                ip = (struct interface_info *)0;
                interface_reference(&ip, last->next, MDL);
                interface_dereference (&last->next, MDL);
                if (ip->next)
                {
                    interface_reference (&last->next, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            } 
            else 
            {
                ip = (struct interface_info *)0;
                interface_reference (&ip, *list, MDL);
                interface_dereference (list, MDL);
                if (ip->next)
                {
                    interface_reference (list, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            }
            break;
        }
        last = ip;
    }
    
    if (ip)
    {
        omapi_unregister_io_object ((omapi_object_t *)ip);
        if_deregister_send (ip);
        if_deregister_receive (ip);
        interface_dereference(&ip, MDL);
    }

    /* clear clientid for interfaces that use this interface 's mac as clientid
    ** for bug 18802 added by liuyang 2012-4-20
    */
    _dhclient_clientid_del_from_intf_list(interfaces, name);
    _dhclient_clientid_del_from_intf_list(free_intf_list, name);
    return;
}

/*for IFNAME clientid*/
int
dhclient_option_client_id_update(dhc_send_options_t *popts)
{
    char *mem = NULL;
    struct interface_info *ip = NULL;
    int len = 0;
    
    if (NULL == popts)
    {
        return RESULT_ERROR;
    }
    if (NULL == (ip = dhclient_locate_interface(free_intf_list, popts->client_type))
        && NULL == (ip = dhclient_locate_interface(interfaces, popts->client_type)))
    {
        DHCLIENT_LOG_ERR("%% Locate interface %s failed when update client id\n",
            _get_intf_display_name(popts->client_type));
        return RESULT_ERROR;
    }
    
    if (ip->hw_address.hlen <= 0)
    {
        if (0 != dhclient_init_hardware_address(ip))
        {
            DHCLIENT_LOG_ERR("%% Get hardware address for interface %s failed\n",
                    ip->name);
            return RESULT_ERROR;
        }
    }
    len = ip->hw_address.hlen - 1;
    
    mem = dmalloc(len + 1, MDL); /* 1 more bytes for '\0' */
    if (NULL == mem)
    {
        DHCLIENT_LOG_ERR("%% Setting option clientid failed (out of memory)\n");
        return RESULT_ERROR;
    }
    memcpy(mem, &ip->hw_address.hbuf[1], len);
    if (NULL != popts->client_id)
    {
        dfree(popts->client_id, MDL);
    }
    popts->client_id_len = len;
    popts->client_id = mem;
    mem[len] = '\0';
    
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhclient_recv_interface_add
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
dhclient_recv_interface_add(tbl_interface_t *p_db_if)
{
    struct interface_info *ip = NULL;
    int is_free_list = 0;
    int status;
    int find = 0;
    char mac_addr_buf[CMD_BUF_16];
    
    /*for bug 17567 not allow loopback interface and return immediately,added by liuyang 2011-12-26
    ** only allow routed-port, vlan interface, agg interface 2012-8-27
    */
    DHC_INTF_TYPE_CHECK(p_db_if->key.name, NULL);


    if (NULL != (ip = dhclient_locate_interface(free_intf_list, p_db_if->key.name)))
    {
        is_free_list = 1;
        find = 1;
    }
    else 
    {
        ip = dhclient_locate_interface(interfaces, p_db_if->key.name);
        if (NULL != ip)
        {
            find = 1;
        }
    }

    if (NULL == ip)
    {
        if (ISC_R_SUCCESS != (status = interface_allocate (&ip, MDL)))
        {
            DHCLIENT_LOG_ERR("Warning: Out of Memory!");
            return RESULT_OK;
        }
        snprintf(ip->name, IFNAMSIZ, p_db_if->key.name);
    }

    ip->ifindex = p_db_if->ifindex;
    ip->vrfid = 0;
    ip->flags |= INTERFACE_SHUTDOWN; /* setting shutdown flag */
    if (tbl_interface_is_running(p_db_if))
    {
        ip->flags &= ~INTERFACE_SHUTDOWN;
    }

    cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_if->hw_mac_addr);
    ip->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
    ip->hw_address.hbuf[0] = ARPHRD_ETHER;
    sal_memcpy(&ip->hw_address.hbuf[1], p_db_if->mac_addr, GLB_ETH_ADDR_LEN);

    if (!find)
    {
        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));
        }
        else
        {
             DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));    
        }

        /* for bug 17407 added by liuyang for pmsg not include hw address for vlan if 2011-12-15*/
        if (ip->hw_address.hlen <= 1)
        {
            dhclient_init_hardware_address(ip);
        }       
        /* add interface to list */
        _dhclient_add_into_intf_list(&free_intf_list, ip);
        interface_dereference (&ip, MDL);
    }
    else if (!is_free_list && -1 == ip->rfdesc)
    {
        if (0 == ip->hw_address.hlen)
        {
            dhclient_init_hardware_address(ip);
        }
        if_register_receive (ip);
        if_register_send (ip);
        if (ISC_R_SUCCESS != (status = omapi_register_io_object(
                        (omapi_object_t *)ip, if_readsocket, 0, got_one, 0, 0)))
        {
            DHCLIENT_LOG_ERR("Can't register I/O handle for %s: %s",
                    _get_intf_display_name(ip->name), isc_result_totext(status));
            if_deregister_send (ip);
            if_deregister_receive (ip);
            return RESULT_OK;
        }

        start_state_machine(ip);
    }
    return RESULT_OK;
}

/*******************************************************************************
 * Name:    dhclient_recv_interface_delete
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
dhclient_recv_interface_delete(tbl_interface_t *p_db_if)
{
    struct interface_info *ip = NULL;
    
    if (NULL != (ip = dhclient_locate_interface(interfaces, p_db_if->key.name)))
    {
        stop_timers(ip);
        dhclient_delete_interface(&interfaces, p_db_if->key.name);
    }
    else
    {
        dhclient_delete_interface(&free_intf_list, p_db_if->key.name);
    }
    return RESULT_OK;
}

int
dhclient_recv_interface_state(tbl_interface_t *p_sys_if)
{
    struct interface_info *ip = NULL;
    char ifname[IFNAMSIZ];
    char is_free_list = 0;

    
    snprintf(ifname, IFNAMSIZ, p_sys_if->key.name);

    if (NULL != (ip = dhclient_locate_interface(free_intf_list, ifname)))
    {
        is_free_list = 1;
    }
    else if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))
    {
        /* for add msg after state msg when create agg ,modified by liuyang 2011-11-23
        DHCLIENT_LOG_ERR("Locate interface %s failed when process state change notify",
                ifname);
        */        
        return RESULT_OK;
    }

    if ((tbl_interface_is_running(p_sys_if)&& (ip->flags & INTERFACE_SHUTDOWN))
        || ((!tbl_interface_is_running(p_sys_if))&& (!(ip->flags & INTERFACE_SHUTDOWN))))
    {
        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Interface %s state to %s",
                _get_intf_display_name(ip->name),
                (tbl_interface_is_running(p_sys_if)) ? "up" : "down");
        }
        else
        {
            DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Interface %s state to %s",
                _get_intf_display_name(ip->name),
                (tbl_interface_is_running(p_sys_if)) ? "up" : "down");    
        }    
    }

    if (tbl_interface_is_running(p_sys_if))
    {
        ip->flags &= ~INTERFACE_SHUTDOWN;
        if (!is_free_list && ip->client && S_STOPPED == ip->client->state)
        {
            start_state_machine(ip);
        }
    }
    else
    {
        ip->flags |= INTERFACE_SHUTDOWN;
        if (!is_free_list)
        {
            stop_timers(ip);
        }
    }

    return RESULT_OK;
}
/* for bug 18797 update clientid added by liuyang 2012-4-27*/
int
dhclient_recv_interface_type_change(tbl_interface_t *p_sys_if)
{
    struct interface_info *ip = NULL;

    if (NULL == (ip = dhclient_locate_interface_by_ifindex(free_intf_list, (p_sys_if->ifindex)))
        && NULL == (ip = dhclient_locate_interface_by_ifindex(interfaces, (p_sys_if->ifindex))))
    {
        return RESULT_OK;
    }
    /* we can't get mac now for it had not be updated in kernel, so just clear it*/    
    memset(&(ip->hw_address), 0, sizeof(struct hardware));

    _dhclient_clientid_update_from_intf_list(interfaces, ip);
    _dhclient_clientid_update_from_intf_list(free_intf_list, ip);

    return RESULT_OK;
}

char *
dhclient_get_tftpsrv_addresses(struct client_state *client, 
        struct client_lease *lease, char *pdst_buf, int *dst_buf_len)
{
    struct data_string data;
    struct option_cache *oc;
    int len = 0;
    const char *psz;

    if (NULL == lease || NULL == pdst_buf)
    {
        return NULL;
    }

    memset (&data, 0, sizeof data);
    oc = lookup_option (&dhcp_universe, lease->options, DHO_TFTP_SERVER_ADDRS);
    if (oc && evaluate_option_cache (&data, (struct packet *)0,
                (struct lease *)0, client, (struct option_state *)0,
                lease->options, &global_scope, oc, MDL)) {
        if (data.len >= 4) {
            if (data.len > DHC_MAX_OPTION_150_BYTES)
            {
                DHCLIENT_LOG_INFO("option tftp-server-ip-addresses too long, total %d bytes, trim to %d bytes",
                        data.len, DHC_MAX_OPTION_150_BYTES);
            }
            psz = pretty_print_option(oc->option, data.data, 
                    (data.len <= DHC_MAX_OPTION_150_BYTES) ? 
                    data.len : DHC_MAX_OPTION_150_BYTES,
                    1, 1);
            if (NULL != psz && '\0' != psz[0])
            {
                len = strlen(psz);
                if (len >= *dst_buf_len)
                {
                    len = *dst_buf_len - 1;
                }
                memcpy(pdst_buf, psz, len);
                *dst_buf_len = len;
                pdst_buf[len] = '\0';
            }
            data_string_forget (&data, MDL);
        }
        data_string_forget (&data, MDL);
    }
    return len ? pdst_buf : NULL;
}

/*******************************************************************************
 * Name:    dhclient_get_route_addresses
 * Purpose: Parse and get route options (3/33/121/249)
 *          the priority for route options is 121 > 249 > 3 > 33
 * Input: 
 *   client: 
 *   lease: 
 *   route_addr_buf: 
 *   route_addr_len: 
 *   route_type: 
 * Output: 
 *   route_addr_buf: 
 *   route_addr_len: 
 *   route_type: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int 
dhclient_get_route_addresses(struct client_state *client, 
        struct client_lease *lease, char *routers, 
        char *cfs_routers, char *cls_routers, int warn) 
{
    struct data_string data;
    struct option_cache *oc;
    const char *psz;

    if (NULL == lease || NULL == client)
    {
        return -1;
    }

    memset (&data, 0, sizeof data);
    oc = lookup_option (&dhcp_universe, lease->options, DHO_CLASSLESS_STATIC_ROUTES);
    if (NULL == oc)
    {
        oc = lookup_option (&dhcp_universe, lease->options, DHO_CLASSLESS_STATIC_ROUTES_MS);
    }
    if (oc && evaluate_option_cache (&data, (struct packet *)0,
                (struct lease *)0, client, (struct option_state *)0,
                lease->options, &global_scope, oc, MDL)) {
        if (data.len >= 4) {
            if (data.len > DHC_MAX_OPTION_121_BYTES)
            {
                if (warn)
                DHCLIENT_LOG_INFO("option classless-static-routers too long, total %d bytes, trim to %d bytes",
                        data.len, DHC_MAX_OPTION_121_BYTES);
            }
            psz = pretty_print_option(oc->option, data.data, 
                    (data.len <= DHC_MAX_OPTION_121_BYTES) ? 
                    data.len : DHC_MAX_OPTION_121_BYTES,
                    1, 1);
            if (NULL != psz && '\0' != psz[0])
            {
                snprintf(cls_routers, DHC_ROUTERS_MAX_LEN, "%s", psz);
            }
            data_string_forget (&data, MDL);
        }
        data_string_forget (&data, MDL);
    }
    else
    {
        /* no option 121 & no option 249 */
        oc = lookup_option (&dhcp_universe, lease->options, DHO_STATIC_ROUTES);
        if (oc && evaluate_option_cache (&data, (struct packet *)0,
                    (struct lease *)0, client, (struct option_state *)0,
                    lease->options, &global_scope, oc, MDL)) {
            if (data.len >= 4) {
                if (data.len > DHC_MAX_OPTION_33_BYTES)
                {
                    if (warn)
                    DHCLIENT_LOG_INFO("option static routers too long, total %d bytes, trim to %d bytes",
                            data.len, DHC_MAX_OPTION_33_BYTES);
                }
                psz = pretty_print_option(oc->option, data.data,
                        (data.len <= DHC_MAX_OPTION_33_BYTES) ? 
                        data.len : DHC_MAX_OPTION_33_BYTES,
                        1, 1);
                if (NULL != psz && '\0' != psz[0])
                {
                    snprintf(cfs_routers, DHC_ROUTERS_MAX_LEN, "%s", psz);
                }
                data_string_forget (&data, MDL);
            }
            data_string_forget (&data, MDL);
        }
    }

    oc = lookup_option (&dhcp_universe, lease->options, DHO_ROUTERS);
    if (oc && evaluate_option_cache (&data, (struct packet *)0,
                (struct lease *)0, client, (struct option_state *)0,
                lease->options, &global_scope, oc, MDL)) {
        if (data.len >= 4) {
            if (data.len > DHC_MAX_OPTION_3_BYTES)
            {
                if (warn)
                DHCLIENT_LOG_INFO("option routers too long, total %d bytes, trim to %d bytes",
                        data.len, DHC_MAX_OPTION_3_BYTES);
            }
            psz = pretty_print_option(oc->option, data.data, 
                    (data.len <= DHC_MAX_OPTION_3_BYTES) ? 
                    data.len : DHC_MAX_OPTION_3_BYTES,
                    1, 1);
            if (NULL != psz && '\0' != psz[0])
            {
                snprintf(routers, DHC_ROUTERS_MAX_LEN, "%s", psz);
            }
            data_string_forget (&data, MDL);
        }
        data_string_forget (&data, MDL);
    }
    return 0;
}

struct iaddr *
dhclient_get_iaddr_from_options(struct client_state *client, 
        struct client_lease *lease, unsigned code, struct iaddr *pia)
{
    struct data_string data;
    struct option_cache *oc;
    int find = 0;

    if (NULL == lease || NULL == pia)
    {
        return NULL;
    }

    memset (&data, 0, sizeof data);
    oc = lookup_option (&dhcp_universe, lease->options, code);
    if (oc && evaluate_option_cache (&data, (struct packet *)0,
                (struct lease *)0, client, (struct option_state *)0,
                lease->options, &global_scope, oc, MDL)) {
        if (data.len > 3) {
            memcpy (pia->iabuf, data.data, data.len);
            pia->len = data.len;
            find = 1;
            data_string_forget (&data, MDL);
        }
        data_string_forget (&data, MDL);
    }
    return find ? pia : NULL;
}

int
dhclient_get_u32_from_options(struct client_state *client, 
        struct client_lease *lease, unsigned code, u_int32_t *uret)
{
    struct data_string data;
    struct option_cache *oc;
    int find = 0;

    if (NULL == lease)
    {
        return 0;
    }

    memset (&data, 0, sizeof data);
    oc = lookup_option (&dhcp_universe, lease->options, code);
    if (oc && evaluate_option_cache (&data, (struct packet *)0,
                (struct lease *)0, client, (struct option_state *)0,
                lease->options, &global_scope, oc, MDL)) {
        if (sizeof(u_int32_t) == data.len) {
            *uret = ntohl(*(u_int32_t *)data.data);
            find = 1;
            data_string_forget (&data, MDL);
        }
        data_string_forget (&data, MDL);
    }
    return find;
}

int
dhclient_compare_ip_routers(struct client_state *client, 
        struct client_lease *old, struct client_lease *new)
{
    if (NULL == client->interface || NULL == old || NULL == new)
    {
        return 0;
    }

    /* compare ip address */
    if (memcmp(&old->address, &new->address, sizeof(struct iaddr)))
    {
        return 1;
    }

    /* compare subnet mask */
    struct iaddr ia_mask_new;
    struct iaddr ia_mask_old;
    dhclient_get_iaddr_from_options(client, old, DHO_SUBNET_MASK, &ia_mask_old);
    dhclient_get_iaddr_from_options(client, old, DHO_SUBNET_MASK, &ia_mask_old);
    if (memcmp(&ia_mask_old, &ia_mask_new, sizeof(struct iaddr)))
    {
        return 1;
    }

    /* compare routers */
    char routers_new[DHC_ROUTERS_MAX_LEN];
    char cfs_routers_new[DHC_ROUTERS_MAX_LEN];
    char cls_routers_new[DHC_ROUTERS_MAX_LEN];
    char routers_old[DHC_ROUTERS_MAX_LEN];
    char cfs_routers_old[DHC_ROUTERS_MAX_LEN];
    char cls_routers_old[DHC_ROUTERS_MAX_LEN];
    routers_old[0] = cfs_routers_old[0] = cls_routers_old[0] = '\0';
    routers_new[0] = cfs_routers_new[0] = cls_routers_new[0] = '\0';
    dhclient_get_route_addresses(client, old, 
            routers_old, cfs_routers_old, cls_routers_old, 0);
    dhclient_get_route_addresses(client, new, 
            routers_new, cfs_routers_new, cls_routers_new, 0);

    if (strcasecmp(routers_new, routers_old)
            || strcasecmp(cls_routers_new, cls_routers_old)
            || strcasecmp(cfs_routers_new, cfs_routers_old))
    {
        return 1;
    }

    return 0;
}

/*******************************************************************************
 * Name:    dhclient_send_bound_msg
 * Purpose: send bound message to NSM
 * Input: 
 *   client: 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhclient_send_bound_msg(struct client_state *client)
{
    struct client_lease *lease = client->active;
    struct interface_info *iptr;
    struct iaddr ia_mask;

    tbl_route_if_t* p_route_if = NULL;
    tbl_route_if_key_t route_if_key;

    iptr = client->interface;

    sal_memset(&route_if_key, 0, sizeof(route_if_key));
    sal_memcpy(route_if_key.name, iptr->name, IFNAMSIZ);
    p_route_if = tbl_route_if_get_route_if(&route_if_key);
    if (NULL == p_route_if)
    {
        return 0;
    }

    if (NULL == client->interface || NULL == lease)
    {
        return -1;
    }

    if (NULL == dhclient_get_iaddr_from_options(client, lease, 
                DHO_SUBNET_MASK, &ia_mask))
    {
        return -1;
    }


    sal_strcpy(p_route_if->dhcp_client_ipv4_mask, (char *)piaddr(ia_mask));
    tbl_route_if_set_route_if_field(p_route_if, TBL_ROUTE_IF_FLD_DHCP_CLIENT_IPV4_MASK);

    sal_strcpy(p_route_if->dhcp_client_ipv4, (char *)piaddr(lease->address));
    tbl_route_if_set_route_if_field(p_route_if, TBL_ROUTE_IF_FLD_DHCP_CLIENT_IPV4);

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
            "cfg interface %s bound ip %s mask %s",
            _get_intf_display_name(client->interface->name),
            (char *)piaddr(lease->address),
            (char *)piaddr(ia_mask));


    return 0; /* fake */
}

int
dhclient_route_if_enable(tbl_route_if_t* p_rt_if)
{
    struct interface_info *ip;
    if (p_rt_if->dhcp_client_enable) {
        ip = dhclient_register_interface(p_rt_if->key.name);
        if (NULL == ip)
        {
            log_error("%% Trigger DHCP client function for interface %s failed\n",
                    p_rt_if->key.name);
            return PM_E_FAIL;
        }

         /* modified by liwh for bug 50987, 2019-02-13 */
        ip->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
        ip->hw_address.hbuf[0] = ARPHRD_ETHER;
        sal_memcpy(&ip->hw_address.hbuf[1], p_rt_if->mac, GLB_ETH_ADDR_LEN);
        /* liwh end */
        
        /* start state machine */
        start_state_machine(ip);

    } else {
        dhclient_recv_event(p_rt_if->key.name, DHC_EVENT_DISABLE);
    }

    return 0;
}

/* modified by liwh for bug 49365, 2018-11-27 */
#ifdef _CENTEC_
int
dhclientv6_route_if_enable(tbl_route_if_t* p_rt_if)
{
    struct interface_info *ip;
    if (p_rt_if->dhcpv6_client_enable) {
        ip = dhclient_register_interface(p_rt_if->key.name);
        if (NULL == ip)
        {
            log_error("%% Trigger DHCP client function for interface %s failed\n",
                    p_rt_if->key.name);
            return PM_E_FAIL;
        }

        /* modified by liwh for bug 50987, 2019-02-13 */
        ip->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
        ip->hw_address.hbuf[0] = ARPHRD_ETHER;
        sal_memcpy(&ip->hw_address.hbuf[1], p_rt_if->mac, GLB_ETH_ADDR_LEN);
        /* liwh end */
        
        /* start state machine */
        start_state_machine(ip);

    } else {
        dhclient_recv_event(p_rt_if->key.name, DHC_EVENT_DISABLE);
    }

    return 0;
}
#endif
/*liwh end */

int
dhclient_route_if_flags(tbl_route_if_t* p_rt_if)
{
    struct interface_info *ip = NULL;

    ip = dhclient_locate_interface(interfaces, p_rt_if->key.name);
    if (NULL == ip)
    {
        DHCLIENT_LOG_ERR("Locate interface %s failed when process route port flags events",
                _get_intf_display_name(p_rt_if->key.name));
        return RESULT_OK;
    }

    if (DHC_NTF_UNBOUND_DONE == p_rt_if->dhcp_client_flags)
    {
        /* ip address install failed */
        DHCLIENT_LOG_ERR("Interface %s install ip failed, change to state STOP",
                _get_intf_display_name(p_rt_if->key.name));

        if (ip->client && ip->client->active)
        {
            make_decline(ip->client, ip->client->active);
            send_decline(ip->client);
            destroy_client_lease(ip->client->active);
            ip->client->active = NULL;
            dhclient_uninstall_intf_ip(ip->client);
        }
        stop_timers(ip);
    }

    return 0;
}

int32
dhclient_process_route_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_route_if_t *p_sys_if = (tbl_route_if_t*)p_tbl;
    
    CFG_DBG_FUNC();

    switch (oper) {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:    
        switch (field_id)
        {
            case TBL_ROUTE_IF_FLD_DHCP_CLIENT_ENABLE:
                if (AF_INET == dhclient_get_local_family())
                {
                    dhclient_route_if_enable(p_sys_if);
                }
                break;
                
            case TBL_ROUTE_IF_FLD_DHCP_CLIENT_FLAGS:
                if (AF_INET == dhclient_get_local_family())                   
                {
                    dhclient_route_if_flags(p_sys_if);
                }
                break;

            case TBL_ROUTE_IF_FLD_DHCPV6_CLIENT_ENABLE:
                if (AF_INET6 == dhclient_get_local_family())
                {
                    dhclientv6_route_if_enable(p_sys_if);
                }
                break;

            default:
                break;
        }

        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int dhclient_file_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}

static int
dhclient_get_mgmt_if_ipv4_address_mask(char *ipv4_addr)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    
#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig eth99 | grep 'inet' | grep 'netmask' | awk '{print $4}'>%s", GLB_CONFIG_MGMT_IPV4_ADDR);
#else        
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig eth0 | grep 'inet addr:' | awk '{print $4}'>%s", GLB_CONFIG_MGMT_IPV4_ADDR);
#endif
    system(szCmd);
    if (dhclient_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
#ifdef _GLB_UML_SYSTEM_
               sal_strncpy(ipv4_addr, buf, (sal_strlen(buf)-1));
#else
               sal_strncpy(ipv4_addr, &buf[5], (sal_strlen(buf) - 6));
#endif
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

static int
dhclient_get_mgmt_if_ipv4_address(char *ipv4_addr)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  

#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip netns exec mgmt ifconfig eth99 | grep 'inet' | grep 'netmask' | awk '{print $2}'>%s", GLB_CONFIG_MGMT_IPV4_ADDR);
#else        
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip netns exec mgmt ifconfig eth0 | grep 'inet addr:' | awk '{print $2}'>%s", GLB_CONFIG_MGMT_IPV4_ADDR);
#endif
    system(szCmd);
    if (dhclient_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
#ifdef _GLB_UML_SYSTEM_
               sal_strncpy(ipv4_addr, buf, (sal_strlen(buf) -1));
#else
               sal_strncpy(ipv4_addr, &buf[5], (sal_strlen(buf) - 6));
#endif
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

#define _IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define IPV4_MASK_TO_LEN(mask, len) \
    { \
        int32 i; \
        len = 0; \
        for (i = 31; i >= 0; i--) \
        { \
            if (_IS_BIT_SET((mask), (uint32)i)) \
            {(len)++; } \
            else \
            {; } \
        } \
    }

int
dhclient_mgmt_if_enable(tbl_manage_if_t* p_mgmt_if)
{
    char szCmd[128];
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];
    char ipv4_addr[CMD_BUF_64] ={0}; 
    char ipv4_addr_mask[CMD_BUF_64] ={0};
    uint32   ipv4_mask = 0;
    uint32   ipv4_len = 0;

    if (p_mgmt_if->dhclient_enable) {
#ifdef _GLB_UML_SYSTEM_
        sal_snprintf(szCmd, 128, "ip netns exec mgmt dhclient-mgmt eth99");
#else
        sal_snprintf(szCmd, 128, "ip netns exec mgmt dhclient-mgmt eth0 -sf /usr/bin/dhclient-script -lf /usr/bin/dhclient.leases");
#endif

        system(szCmd);
    }
    else
    {
        /* kill dhclient-mgmt process */
        sal_snprintf(szCmd, 128, "ps -u root | grep dhclient-mgmt  | awk '{print $1}' >%s", GLB_CONFIG_MGMT_IPV4_ADDR);
        system(szCmd);

        if (dhclient_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
        {
            fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
            if (NULL != fp)
            {
                while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
                {
                    sal_snprintf(szCmd, 128, "kill -9 %s", buf);
                    system(szCmd);
                }
    
                sal_fclose(fp);
            }
        } 
        
        /* clear docs */
#ifndef _GLB_UML_SYSTEM_
        sal_snprintf(szCmd, 128, ">%s", "/usr/bin/dhclient.leases");
        system(szCmd);
#endif

        sal_snprintf(szCmd, 128, ">%s", GLB_CONFIG_MGMT_IPV4_ADDR);
        system(szCmd);
        
        /* flush mgmt ip addr */
        /* get mask */
        dhclient_get_mgmt_if_ipv4_address_mask(ipv4_addr_mask);
        sal_inet_pton(AF_INET, ipv4_addr_mask, &ipv4_mask);
        IPV4_MASK_TO_LEN(ipv4_mask, ipv4_len) ;
        dhclient_get_mgmt_if_ipv4_address(ipv4_addr);
#ifdef _GLB_UML_SYSTEM_
        sal_snprintf(szCmd, 128, "ip netns exec mgmt ip addr del %s/%d dev eth99", ipv4_addr, ipv4_len);
#else
        sal_snprintf(szCmd, 128, "ip netns exec mgmt ip addr del %s/%d dev eth0", ipv4_addr, ipv4_len);
#endif
        system(szCmd);
    }
    
#if 0
    struct interface_info *ip = NULL;
    if (p_mgmt_if->dhclient_enable) {
        ip = dhclient_register_interface(dhclient_get_mgmt_if_name());
        if (NULL == ip)
        {
            log_error("Trigger DHCP client function for interface %s failed\n",
                    dhclient_get_mgmt_if_name());
            return PM_E_FAIL;
        }
        
        /* start state machine */
        ip->flags &= ~INTERFACE_SHUTDOWN;
        start_state_machine(ip);

    } else {
        dhclient_recv_event(dhclient_get_mgmt_if_name(), DHC_EVENT_DISABLE);
    }
#endif

    return 0;
}

int
dhclientv6_mgmt_if_enable(tbl_manage_if_t* p_mgmt_if)
{
    char szCmd[128];
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  

    if (p_mgmt_if->dhclientv6_enable) {
#ifdef _GLB_UML_SYSTEM_
        sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt sysctl -w net.ipv6.conf.eth99.disable_ipv6=0 >/dev/null 2>&1");
        system(szCmd);

        sleep(5);
    
        sal_snprintf(szCmd, 128, "ip netns exec mgmt dhclient6-mgmt eth99 -nw -6 -cf /etc/dhcp/dhclient-dhcpv6.conf -lf /etc/dhcp/dhclient6.leases");
#else
        //sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt sysctl -w net.ipv6.conf.eth0.disable_ipv6=0 >/dev/null 2>&1");
        //system(szCmd);

        //usleep(200000);
        sleep(5);
        sal_snprintf(szCmd, 128, "ip netns exec mgmt dhclient6-mgmt eth0 -nw -6 -cf /usr/bin/dhclient-dhcpv6.conf -lf /usr/bin/dhclient6.leases");
#endif

        system(szCmd);
    }
    else
    {
        /* kill dhclient-mgmt process */
        sal_snprintf(szCmd, 128, "ps -u root | grep dhclient6-mgmt  | awk '{print $1}' >%s", GLB_CONFIG_MGMT_IPV6_ADDR);
        system(szCmd);

        if (dhclient_file_exist(GLB_CONFIG_MGMT_IPV6_ADDR))
        {
            fp = sal_fopen(GLB_CONFIG_MGMT_IPV6_ADDR, SAL_OPEN_RO);
            if (NULL != fp)
            {
                while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
                {
                    sal_snprintf(szCmd, 128, "kill -9 %s", buf);
                    system(szCmd);
                }
    
                sal_fclose(fp);
            }
        } 
        
        /* clear docs */
#ifdef _GLB_UML_SYSTEM_
        sal_snprintf(szCmd, 128, ">%s", "/etc/dhcp/dhclient6.leases");
        system(szCmd);
#else
        sal_snprintf(szCmd, 128, ">%s", "/usr/bin/dhclient6.leases");
        system(szCmd);
#endif

        sal_snprintf(szCmd, 128, ">%s", GLB_CONFIG_MGMT_IPV6_ADDR);
        system(szCmd);
    }

    return 0;
}

int32
dhclient_process_mgmt_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_manage_if_t *p_sys_if = (tbl_manage_if_t*)p_tbl;
    
    CFG_DBG_FUNC();

    switch (oper) {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        switch (field_id)
        {
            case TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE:
                dhclient_mgmt_if_enable(p_sys_if);
                break;
                
            /* modified by liwh for bug 49365, 2018-12-04 */
            case TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE:
                dhclientv6_mgmt_if_enable(p_sys_if);
                break;
            /* liwh end */

            default:
                break;
        }

        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

/*for bug 24795 modified by liuyang 2014-2-19*/
char *_dhclient_get_state(struct client_state *client, char *buf, int len)
{
    enum dhcp_state es = client->state;
    switch (es)
    {
    case S_REBOOTING:
        sal_snprintf(buf, len, "REBOOT");
        break;
    case S_INIT:
        if (AF_INET == dhclient_get_local_family())
        {
            if ((cur_time - client->first_sending) < client->config->timeout)
            {
                sal_snprintf(buf, len, "INIT");
            }
            else
            {
                sal_snprintf(buf, len, "HALT");
            }
        }
        else
        {
            sal_snprintf(buf, len, "INIT");
        }
        break;
    case S_SELECTING:
        sal_snprintf(buf, len, "SELECTING");
        break;
    case S_REQUESTING:
        sal_snprintf(buf, len, "REQUESTING");
        break;
    case S_BOUND:
        sal_snprintf(buf, len, "BOUND");
        break;
    case S_RENEWING:
        sal_snprintf(buf, len, "RENEWING");
        break;
    case S_REBINDING:
        sal_snprintf(buf, len, "REBINDING");
        break;
    case S_STOPPED:
    case S_DECLINING:
        sal_snprintf(buf, len, "SUSPEND");
        break;
    default:
        sal_snprintf(buf, len, "unknown %d", es);
        break;
    }

    return buf;
}

/* modified by liwh for bug 44686, 2018-11-16 */
/* When string format is invalid return 0. */
int str2prefix_ipv4(const char *str, prefix_ipv4_t *p)
{
	int ret;
	int plen;
	char *pnt;
	char *cp;

	/* Find slash inside string. */
	pnt = strchr(str, '/');

	/* String doesn't contail slash. */
	if (pnt == NULL) {
		/* Convert string to prefix. */
		ret = inet_aton(str, &p->prefix);
		if (ret == 0)
			return 0;

		/* If address doesn't contain slash we assume it host address.
		 */
		p->family = AF_INET;
		p->prefixlen = IPV4_MAX_BITLEN;

		return ret;
	} else {
		//cp = XMALLOC(MTYPE_TMP, (pnt - str) + 1);
		cp = malloc((pnt - str) + 1);
		strncpy(cp, str, pnt - str);
		*(cp + (pnt - str)) = '\0';
		ret = inet_aton(cp, &p->prefix);
		//XFREE(MTYPE_TMP, cp);
		free (cp);

		/* Get prefix length. */
		plen = (u_char)atoi(++pnt);
		if (plen > IPV4_MAX_PREFIXLEN)
			return 0;

		p->family = AF_INET;
		p->prefixlen = plen;
	}

	return ret;
}

/* format: A.B.C.D,A.B.C.D */
int
dhc_comm_install_static_routers(nsm_msg_dhc_bound_t *pbound, 
        dhc_comm_rt_cb_arg_t *proc)
{
    unsigned char distance = pbound->distance;
    dhc_comm_routers_rec_t item;
    char *psz_errmsg = proc->errmsg;
    char *psz_routers = (char *)pbound->routers;
    int nRet = PM_E_FAIL;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nProcToks;
    int idx;

    if (NULL == psz_routers)
    {
        return PM_E_FAIL;
    }

    if (sal_strspn(psz_routers, "0123456789.,")
             != sal_strlen(psz_routers))
    {
        sal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid routers string %s", psz_routers);
        goto err_out;
    }

    if (split_tokens(psz_routers, sal_strlen(psz_routers), 
                DHC_MAX_OPTION_3_BYTES, ",", &nToks, &ppTokArr) != 0)
    {
        sal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                "Invalid routers string %s", psz_routers);
        goto err_out;
    }

    nProcToks = (nToks < DHC_MAX_ROUTERS_RECORDS) ? 
        nToks : DHC_MAX_ROUTERS_RECORDS;
    for (idx = 0; idx < nProcToks; idx++)
    {
        sal_memset(&item, 0x00, sizeof(dhc_comm_routers_rec_t));

        /* check router address */
        if (!str2prefix_ipv4(ppTokArr[idx], &item.router))
        {
            sal_snprintf(psz_errmsg, DHC_ERR_MSG_BUF_LEN, 
                    "Invalid routers router address %s", ppTokArr[idx]);
            goto err_out;
        }
        str2prefix_ipv4("0.0.0.0/0", &item.dest);
        if (PM_E_NONE != proc->cb_install_static_router(proc, &item, distance))
        {
            goto err_out;
        }

        if (distance < DHC_MAX_DISTANCE)
        {
            distance++;
        }
    }

    nRet = PM_E_NONE;

err_out:
    if (NULL != ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return nRet;
}

int _dhclient_get_router(struct dhc_comm_rt_cb_arg *args,
        dhc_comm_routers_rec_t *prec, unsigned char distance)
{
    char *buf = (char *)args->data;
    char szRouter[DHC_PREFIX_BUF_LEN];
    int len = sal_strlen(buf);

    sal_inet_ntop (prec->router.family, &prec->router.prefix,
            szRouter, DHC_PREFIX_BUF_LEN);
    sal_snprintf(buf + len, DHC_DISP_BUF_SIZE - len, "%s%s",
            len ? ", " : "", szRouter);
    return RESULT_OK;
}
/* liwh end */

int
dhclient_get_default_router(struct client_state *client,
    struct client_lease *active, char* route_buf)
{
    char cfs_routers[DHC_ROUTERS_MAX_LEN];
    char cls_routers[DHC_ROUTERS_MAX_LEN];
    char routers[DHC_ROUTERS_MAX_LEN];
    char errmsg[DHC_ERR_MSG_BUF_LEN];
    
    routers[0] = cfs_routers[0] = cls_routers[0] = '\0';
    if (0 == dhclient_get_route_addresses(client, active, 
                routers, cfs_routers, cls_routers, 0))
    {
        dhc_comm_rt_cb_arg_t rt_proc;
        
        nsm_msg_dhc_bound_t bound;
        bound.classless_routers = cls_routers;
        bound.classful_routers = cfs_routers;
        bound.routers = routers;

        rt_proc.errmsg = errmsg;
        rt_proc.data = route_buf;
        rt_proc.pbound = &bound;

        if ('\0' != routers[0])
        {
            //route_buf[0] = '\0';
            rt_proc.cb_install_static_router = _dhclient_get_router;
            dhc_comm_install_static_routers(&bound, &rt_proc);
        }
    }

    return RESULT_OK;
}

int
_dhclient_show_one_intf(FILE *fp, struct interface_info *ip, int verbose)
{
    char buf[DHC_ERR_MSG_BUF_LEN];
    struct client_state *client;
    struct client_lease *active;
    struct timeout *ptm;
    int is_man_dev;
    char *p;

    is_man_dev = _is_mgmt_dev(ip->name);
    if (is_man_dev)
    {
        sal_fprintf(fp, "Management interface DHCP client information:\n");
    }
    else
    {
        sal_fprintf(fp, "%s DHCP client information:\n", _get_intf_display_name(ip->name));
    }

    client = ip->client;
    if (!client)
    {
        sal_fprintf(fp, "  Current state: HALT\n"); /* fake HALT status */
        return RESULT_OK;
    }

    if (verbose && (S_INIT == client->state)
            && (ptm = get_timeout(state_init, client)))
    {
        sal_fprintf(fp, "  Current state: %s (timeout %u seconds)\n",
                _dhclient_get_state(client, buf, DHC_ERR_MSG_BUF_LEN),
                (u_int32_t)(ptm->when.tv_sec - cur_time));
    }
    else
    {
        sal_fprintf(fp, "  Current state: %s\n",
                _dhclient_get_state(client, buf, DHC_ERR_MSG_BUF_LEN));
    }

    /*modified by chenc for bug 52514, 2019-06-22*/
    if ((active = client->active) && 
        (S_BOUND == client->state
        || S_RENEWING == client->state
        || S_REBINDING == client->state))
    /*chenc end*/
    {
        struct iaddr mask;
        char szip[DHC_PREFIX_BUF_LEN];
        char szmask[DHC_PREFIX_BUF_LEN];
        u_int32_t lease;
         /* added by liuyang 2012-3-31*/
        u_int32_t time_delta = 0;
        struct sal_timeval now;

        sal_snprintf(szip, DHC_PREFIX_BUF_LEN, "%s", piaddr(active->address));
        if (NULL == dhclient_get_iaddr_from_options(client, client->active,
                    DHO_SUBNET_MASK, &mask))
        {
            sal_fprintf(fp, "%% Get subnet mask failed (out of memory)\n");
            return PM_E_FAIL;
        }
        sal_snprintf(szmask, DHC_PREFIX_BUF_LEN, "%s", piaddr(mask));

        sal_fprintf(fp, "  Allocated IP: %s %s\n", szip, szmask);

        /*modified by chenc for bug 52514, 2019-06-22*/
        if (S_BOUND == client->state)
        /*chenc end*/
        {
            sal_fprintf(fp, "  Lease/renewal/rebinding: %u/%u/%u seconds\n", 
                (u_int32_t)(active->expiry - cur_time),
                (u_int32_t)(active->renewal - cur_time),
                (u_int32_t)(active->rebind - cur_time));
        }
        
        if (verbose)
        {
            if (!dhclient_get_u32_from_options(client, active, 
                        DHO_DHCP_LEASE_TIME, &lease))
            {
                sal_fprintf(fp, "%% Get lease time failed (out of memory)\n");
                return PM_E_FAIL;
            }
            /*for bug 18764 calculate aboslute time added by liuyang 2012-3-31*/
            gettimeofday(&now, NULL);
            time_delta = now.tv_sec - cur_time;
            format_time_str(szip, DHC_PREFIX_BUF_LEN, 
                    "%Y-%m-%d %H:%M:%S", (active->expiry - lease + time_delta));
            format_time_str(szmask, DHC_PREFIX_BUF_LEN, 
                    "%Y-%m-%d %H:%M:%S", active->expiry + time_delta);
            sal_fprintf(fp, "  Lease from %s to %s\n", szip, szmask);

            /*modified by chenc for bug 52514, 2019-06-22*/
            if (S_BOUND == client->state)
            /*chenc end*/
            {
                int renew = active->renewal - cur_time;
                sal_fprintf(fp, "  Will Renewal in %d days %d hours %d minutes %d seconds\n",
                    renew / 86400, (renew % 86400) / 3600,
                    ((renew % 86400) % 3600) / 60, ((renew % 86400) % 3600) % 60);
            }
        }

        if (NULL == dhclient_get_iaddr_from_options(client, client->active,
                    DHO_DHCP_SERVER_IDENTIFIER, &mask))
        {
            sal_fprintf(fp, "%% Get DHCP server identifier failed (out of memory)\n");
            return PM_E_FAIL;
        }
        sal_fprintf(fp, "  DHCP server: %s\n", piaddr(mask));

        if (!verbose)
        {
            return RESULT_OK;
        }
  /*for bug 19457 modified endian just for display by liuyang 2012-5-30*/
        sal_fprintf(fp, "  Transaction ID: 0x%x\n", sal_ntoh32(client->xid));

        /* modified by liwh for bug 44686, 2018-11-16 */
        char route_buf[DHC_DISP_BUF_SIZE];

        route_buf[0] = '\0';
        dhclient_get_default_router(client, active, route_buf);
        if ('\0' != route_buf[0])
        {
            sal_fprintf(fp, "  Default router: %s\n", route_buf);
        }
        
        #if 0
        char cfs_routers[DHC_ROUTERS_MAX_LEN];
        char cls_routers[DHC_ROUTERS_MAX_LEN];
        char routers[DHC_ROUTERS_MAX_LEN];
        char errmsg[DHC_ERR_MSG_BUF_LEN];
        
        routers[0] = cfs_routers[0] = cls_routers[0] = '\0';
        if (0 == dhclient_get_route_addresses(client, active, 
                    routers, cfs_routers, cls_routers, 0))
        {
            dhc_comm_rt_cb_arg_t rt_proc;
            char route_buf[DHC_DISP_BUF_SIZE];
            nsm_msg_dhc_bound_t bound;
            bound.classless_routers = cls_routers;
            bound.classful_routers = cfs_routers;
            bound.routers = routers;

            rt_proc.errmsg = errmsg;
            rt_proc.data = route_buf;
            rt_proc.pbound = &bound;

            if ('\0' != routers[0])
            {
                route_buf[0] = '\0';
                rt_proc.cb_install_static_router = _dhclient_get_router;
                dhc_comm_install_static_routers(&bound, &rt_proc);
                if ('\0' != route_buf[0])
                {
                    sal_fprintf(fp, "  Default router: %s\n", route_buf);
                }
            }
        }
        #endif
        /* liwh end */

#if 0
        char cfs_routers[DHC_ROUTERS_MAX_LEN];
        char cls_routers[DHC_ROUTERS_MAX_LEN];
        char routers[DHC_ROUTERS_MAX_LEN];
        routers[0] = cfs_routers[0] = cls_routers[0] = '\0';
        if (0 == dhclient_get_route_addresses(client, active, 
                    routers, cfs_routers, cls_routers, 0))
        {
            dhc_comm_rt_cb_arg_t rt_proc;
            char route_buf[DHC_DISP_BUF_SIZE];
            nsm_msg_dhc_bound_t bound;
            bound.classless_routers = cls_routers;
            bound.classful_routers = cfs_routers;
            bound.routers = routers;

            rt_proc.errmsg = errmsg;
            rt_proc.data = route_buf;
            rt_proc.pbound = &bound;

            if ('\0' != routers[0])
            {
                route_buf[0] = '\0';
                rt_proc.cb_install_static_router = _dhclient_get_router;
                dhc_comm_install_static_routers(&bound, &rt_proc);
                if ('\0' != route_buf[0])
                {
                    sal_fprintf(fp, "  Default router: %s\n", route_buf);
                }
            }
            if ('\0' != cls_routers[0])
            {
                route_buf[0] = '\0';
                rt_proc.cb_install_static_router = _dhclient_get_cls_router;
                dhc_comm_install_classless_routers(&bound, &rt_proc);
                if ('\0' != route_buf[0])
                {
                    sal_fprintf(fp, "  Classless static route: %s\n", route_buf);
                }
            }
            else if ('\0' != cfs_routers[0])
            {
                route_buf[0] = '\0';
                rt_proc.cb_install_static_router = _dhclient_get_cfs_router;
                dhc_comm_install_classful_routers(&bound, &rt_proc);
                if ('\0' != route_buf[0])
                {
                    sal_fprintf(fp, "  Classful static route: %s\n", route_buf);
                }
            }
        }
#endif
        char tftpsrv_addr_buf[DHC_MAX_TFTPSRV_BUF_LEN + 1];
        int tftpsrv_addr_len = DHC_MAX_TFTPSRV_BUF_LEN;
        if (dhclient_get_tftpsrv_addresses(client, active, 
                    tftpsrv_addr_buf, &tftpsrv_addr_len))
        {
            tftpsrv_addr_buf[tftpsrv_addr_len] = '\0';
            p = tftpsrv_addr_buf;
            while ('\0' != *p)
            {
                if (',' == *p)
                {
                    *p = ' ';
                }
                p++;
            }
            sal_fprintf(fp, "  TFTP server addresses: %s\n", tftpsrv_addr_buf);
        }

        /* show client id */
        char clientid_buf[DHC_DISP_BUF_SIZE];
        dhc_send_options_t *popts = (dhc_send_options_t *)client->config->sdata;
        if (NULL != popts && NULL != popts->client_id)
        {
            if (!sal_strcmp(popts->client_type, "ascii"))
            {
                sal_snprintf(clientid_buf, DHC_DISP_BUF_SIZE, "%s", popts->client_id);
            }
            else if (!sal_strcmp(popts->client_type, "hex"))
            {
                comm_bin2hex((u_char *)popts->client_id, 
                        popts->client_id_len, (u_char *)clientid_buf);
            }
            else
            {
                if (0 == popts->client_id_len && dhclient_option_client_id_update(popts))
                {
                    clientid_buf[0] = '\0';
                    DHCLIENT_LOG_ERR("Update clientid failed as showing dhcp client status for interface %s",
                        ip->name);
                }
                else
                {
                    /* is interface name */
                    /* for bug 17239 ,we use mac as client id ,modified by liuyang 2011-12-6*/
                    unsigned char *q = (u_char *)popts->client_id;
                    sal_snprintf(clientid_buf, DHC_DISP_BUF_SIZE,
                            "%02x%02x.%02x%02x.%02x%02x",
                            q[0], q[1], q[2], q[3], q[4], q[5]);
                }                
            }
        }
        else
        {
            char hwaddr[M_DHC_ID_MAX_LEN];
            unsigned char *q = &client->interface->hw_address.hbuf[1];

            sal_snprintf(hwaddr, M_DHC_ID_MAX_LEN, 
                    "%02x%02x.%02x%02x.%02x%02x",
                    q[0], q[1], q[2], q[3], q[4], q[5]);
            sal_snprintf(clientid_buf, DHC_DISP_BUF_SIZE,
                    "switch-%s-%s", hwaddr, 
                    _get_intf_display_name(client->interface->name));
        }
        sal_fprintf(fp, "  Client ID: %s\n", clientid_buf);
    }
    else
    {   /*for bug 19457 modified endian just for display by liuyang 2012-5-30*/
        sal_fprintf(fp, "  Transaction ID: 0x%x\n", sal_ntoh32(client->xid));
    }

    return RESULT_OK;
}

int dhclient_show_intf_status(cfg_cmd_para_t *para, char *ifname, int verbose)
{
    struct interface_info *ip;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");    

    if (verbose)
    {
        sal_fprintf(fp, "DHCP client informations:\n");
        DHC_OUT_SPLIT(fp);
    }

    if (NULL == interfaces && verbose)
    {
        sal_fprintf(fp, "  DHCP client function disabled on all interfaces\n");
        sal_fclose(fp);
        return RESULT_OK;
    }

    if (NULL != ifname && sal_strlen(ifname))
    {
        if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))
        {
            if (verbose)
            {
                sal_fprintf(fp, "  DHCP client function disabled on interface %s\n",
                        _get_intf_display_name(ifname));
            }
            sal_fclose(fp);
            return RESULT_OK;
        } 

        _dhclient_show_one_intf(fp, ip, verbose);

        sal_fclose(fp);
        return RESULT_OK;
    }

    for (ip = interfaces; ip; ip = ip->next)
    {
        if (verbose && ip != interfaces)
        {
            DHC_OUT_SPLIT(fp);
        }

        if (RESULT_OK != _dhclient_show_one_intf(fp, ip, verbose))
        {
            sal_fclose(fp);
            return PM_E_FAIL;
        }
    }

    sal_fclose(fp);

    return RESULT_OK;
}

int dhclient_show_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");    

    sal_fprintf(fp, "DHCP client packet statistics:\n");
    DHC_OUT_SPLIT(fp);
    sal_fprintf(fp, "DHCP OFFER     received: %u\n", g_pkt_stats.recv_offer);
    sal_fprintf(fp, "DHCP ACK       received: %u\n", g_pkt_stats.recv_ack);
    sal_fprintf(fp, "DHCP NAK       received: %u\n", g_pkt_stats.recv_nak);
    sal_fprintf(fp, "DHCP Others    received: %u\n", g_pkt_stats.recv_other);
    sal_fprintf(fp, "DHCP DISCOVER      sent: %u\n", g_pkt_stats.send_discover);
    sal_fprintf(fp, "DHCP DECLINE       sent: %u\n", g_pkt_stats.send_decline);
    sal_fprintf(fp, "DHCP RELEASE       sent: %u\n", g_pkt_stats.send_release);
    sal_fprintf(fp, "DHCP REQUEST       sent: %u\n", g_pkt_stats.send_request);
    sal_fprintf(fp, "DHCP packet send failed: %u\n", g_pkt_stats.send_failed);

    sal_fclose(fp);
    return RESULT_OK;
}

int dhclient_clear_statistics()
{
    sal_memset(&g_pkt_stats, 0x00, sizeof(dhc_stats_t));
    return RESULT_OK;
}

int32
dhclient_process_dhclient_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char        ifname[IFNAME_SIZE];
    char        *p_ifname = NULL;
    int32       rc  = PM_E_NONE;
    int32       verbose = 0;

    sal_memset(ifname, 0, sizeof(ifname));
    
    switch (para->oper)
    {
    case CDB_OPER_DEL:
        if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "statistics"))
            {
                rc = dhclient_clear_statistics();
            }
        }
        break;
    case CDB_OPER_GET:
        if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "statistics"))
            {
                rc = dhclient_show_statistics(para);
            }
            if (0 == sal_strcmp(argv[0], "verbose"))
            {
                verbose = 1;
                rc = dhclient_show_intf_status(para, NULL, verbose);
            }
            if (0 == sal_strcmp(argv[0], "management"))
            {
                p_ifname = dhclient_get_mgmt_if_name();
                rc = dhclient_show_intf_status(para, p_ifname, verbose);
            }
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_strcpy(ifname, argv[1]);
                verbose = 0;
            }
            else if (0 == sal_strcmp(argv[0], "management"))
            {
                sal_strcpy(ifname, dhclient_get_mgmt_if_name());
                verbose = 1;
            }
            rc = dhclient_show_intf_status(para, ifname, verbose);
        }

        else if (3 == argc)
        {
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_strcpy(ifname, argv[1]);
                verbose = 1;
                rc = dhclient_show_intf_status(para, ifname, verbose);
            }
            else if (0 == sal_strcmp(argv[0], ""))
            {
                verbose = 1;
                rc = dhclient_show_intf_status(para, NULL, verbose);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

/* modified by liwh for bug 50219, 2018-12-19 */
int32
dhclient_cmd_pre_delete(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t              *p_db_if = NULL;
    tbl_interface_t              intf;
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;

    if (1 == argc)
    {
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        /* check exist */
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", argv[0]); 
        }

        sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
        sal_strncpy(rtif.key.name, argv[0], IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
        if (p_db_rtif)
        {
            if (p_db_rtif->dhcp_client_enable &&  (AF_INET == dhclient_get_local_family()))
            {
                p_db_rtif->dhcp_client_enable = FALSE;
                dhclient_route_if_enable(p_db_rtif);
            }
            else if ((p_db_rtif->dhcpv6_client_enable) && (AF_INET6 == dhclient_get_local_family()))
            {
                 p_db_rtif->dhcpv6_client_enable = FALSE;
                 dhclientv6_route_if_enable(p_db_rtif);   
            }
        }
   
        usleep(100000); /*sleep 100ms*/
    }
    
    return PM_E_NONE;
}
/* liwh end */

/* modified by liwh for bug 50646, 2019-01-02 */
int32
dhclient_cmd_pre_shutdown(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t              *p_db_if = NULL;
    tbl_interface_t              intf;
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;

    if (1 == argc)
    {
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        /* check exist */
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", argv[0]); 
        }

        sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
        sal_strncpy(rtif.key.name, argv[0], IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
        if (p_db_rtif)
        {
            dhclient_recv_event(p_db_rtif->key.name, DHC_EVENT_RELEASE);
        }
   
        usleep(100000); /*sleep 100ms*/
    }
    
    return PM_E_NONE;
}
/* liwh end */

void 
_dhclient_parse_requested_options(dhc_req_option_t *parr, 
        int argc, char **argv, u_int32_t *pflags)
{
    dhc_req_option_t *p;
    int i;

    for (i = 0; i < argc; i++)
    {
        p = parr;
        while (NULL != p->name)
        {
            if (!sal_strcmp(p->name, argv[i]))
            {
                *pflags |= p->flag;
                break;
            }

            p++;
        }
    }

    return;
}

int32
dhclient_intf_if_add(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    rc = dhclient_recv_interface_add(p_db_if);

    return rc;
}

int32
dhclient_intf_if_del(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    rc = dhclient_recv_interface_delete(p_db_if);

    return rc;
}

/* modified by liwh for bug 50704, 2019-01-08 */
int32
dhclient_update_client_id(tbl_interface_t *p_db_update_if)
{
    int32 rc = PM_E_NONE;
    ds_dhclient_if_t *p_db_dhclient = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    struct client_state *client = NULL;
    dhc_send_options_t *popts = NULL;
    struct interface_info *ip = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {      
        p_db_dhclient = p_db_if->dhclient_if;
        if (p_db_dhclient)
        {
            if ((GLB_DHCLIENT_FLAG_PORT == p_db_dhclient->client_flags)
                && (0 == sal_strcmp(p_db_update_if->key.name, p_db_dhclient->client_id)))
            {
                if ((ip = dhclient_locate_interface(interfaces, p_db_if->key.name)))
                {
                    if (ip->client && ip->client->config && ip->client->config->sdata)
                    {
                        popts = (dhc_send_options_t *)ip->client->config->sdata;
                        dhclient_option_client_id_update(popts);
                    }
                }
            }
        }
    }
    
    return rc;
}
/* liwh end */

/* modified by liwh for bug 50200, 2018-12-24 */
int32
dhclient_intf_update_mac_address(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    struct interface_info *ip = NULL;
    char mac_addr_buf[CMD_BUF_16];

    /* if not agg or ethernet, don't update mac address */
    if (memcmp(p_db_if->key.name, "agg", 3) && memcmp(p_db_if->key.name, "eth", 3))
    {
        return rc;
    }

    /* find dhclient interface */
    ip = dhclient_locate_interface(free_intf_list, p_db_if->key.name);
    if (!ip)
    {
        ip = dhclient_locate_interface(interfaces, p_db_if->key.name);
    }

    /* update mac address */
    if (ip)
    {
        cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_if->hw_mac_addr);
        ip->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
        ip->hw_address.hbuf[0] = ARPHRD_ETHER;
        sal_memcpy(&ip->hw_address.hbuf[1], p_db_if->mac_addr, CMD_BUF_16);

        /* modified by liwh for bug 50704, 2019-01-08 */
        dhclient_update_client_id(p_db_if);
        /* liwh end */
    }

    return rc;
}
/* liwh end */

int _dhclient_parse_hex_string(char *phex, char *output, int *olen)
{
    int len;
    int nbytes;

    if (NULL == phex || NULL == output || *olen <= 0)
    {
        sal_assert(0);
        return PM_E_FAIL; 
    }

    len = sal_strlen(phex);
    if (sal_strspn(phex, "0123456789abcdefABCDEF") != len)
    {
        DHCLIENT_LOG_ERR("Invalid hex string\n");
        return PM_E_FAIL;
    }

    if (len > (M_DHC_ID_MAX_HEXSTR_LEN - 2))
    {
        /* for bug 52212 modified by wus 2019-8-1 */
        DHCLIENT_LOG_ERR("%% The length of hex string should not exceed %d\n", M_DHC_ID_MAX_HEXSTR_LEN - 2);
        return PM_E_FAIL;
    }

    if (*olen < ((len / 2 ) + (len %2)))
    {
        sal_assert(0); /* output buffer too short */
        return PM_E_FAIL;
    }

    nbytes = comm_hex2bin((u_char *)phex, sal_strlen(phex), (u_char *)output);
    if  (-1 == nbytes)
    {
        DHCLIENT_LOG_ERR("Convert hex string to binary format failed\n");
        return PM_E_FAIL;
    }

    *olen = nbytes;
    return PM_E_NONE;
}

int32
dhclient_intf_if_cfg(tbl_interface_t *p_db_if, tbl_interface_t *p_sys_if, uint32 field_id)
{
    int32 rc = PM_E_NONE;
    int32 len = 0, lease = 0, olen = 0;
    char buf[M_DHC_ID_MAX_HEXSTR_LEN];
    char type[COMM_NAME_SIZE];
    int ret = 0;
    uint32  t1 = 0;
    uint32  t2 = 0;

    M_DHC_REQ_OPTION_ARR(arr_req);

    switch (field_id)
    {
        case DS_DHCLIENT_IF_FLD_LEASE:
            if (AF_INET != dhclient_get_local_family())
            {
                return rc;    
            }
            
            lease = p_db_if->dhclient_if->lease;
            dhclient_proc_send_option(p_db_if->key.name, M_DHC_LEASE_CODE,  (char *)&lease, sizeof(uint32), NULL);
            break;

        case DS_DHCLIENT_IF_FLD_HOSTNAME:
            if (AF_INET != dhclient_get_local_family())
            {
                return rc;    
            }
            
            len = sal_strlen(p_db_if->dhclient_if->hostname);
            if (0 != len)
            {
                dhclient_proc_send_option(p_db_if->key.name, M_DHC_HOSTNAME_CODE, p_db_if->dhclient_if->hostname, len, NULL);
            }
            else
            {
                dhclient_proc_send_option(p_db_if->key.name, M_DHC_HOSTNAME_CODE, NULL, 0, NULL);
            }
        
            break;

        case DS_DHCLIENT_IF_FLD_CLASS_ID:
            if (AF_INET != dhclient_get_local_family())
            {
                return rc;    
            }
            
            olen = sal_strlen(p_db_if->dhclient_if->class_id);
            if (0 == olen)
            {
                dhclient_proc_send_option(p_db_if->key.name, M_DHC_CLASS_ID_CODE, NULL, 0, NULL);
            }
            else
            {
                if (GLB_DHCLIENT_FLAG_HEX == p_db_if->dhclient_if->class_flags)
                {
                    ret = _dhclient_parse_hex_string(p_db_if->dhclient_if->class_id, buf, &olen);
                    if (PM_E_NONE != ret)
                    {
                        return ret;
                    }
                    sal_snprintf(type, COMM_NAME_SIZE, "hex");
                }
                else if (GLB_DHCLIENT_FLAG_ASCII == p_db_if->dhclient_if->class_flags)
                {
                    olen = sal_snprintf(buf, M_DHC_ID_MAX_LEN, "%s", p_db_if->dhclient_if->class_id);
                    sal_snprintf(type, COMM_NAME_SIZE, "ascii");
                }
                else
                {
                    DHCLIENT_LOG_ERR("Unknown dhclient class type");        
                    return 0;
                }

                dhclient_proc_send_option(p_db_if->key.name, M_DHC_CLASS_ID_CODE, buf, olen, type);
            }
            break;

        case DS_DHCLIENT_IF_FLD_CLIENT_ID:
            if (AF_INET != dhclient_get_local_family())
            {
                return rc;    
            }
            
            olen = sal_strlen(p_db_if->dhclient_if->client_id);
            if (0 == olen)
            {
                dhclient_proc_send_option(p_db_if->key.name, M_DHC_CLIENT_ID_CODE, NULL, 0, NULL);
            }
            else
            {
                if (GLB_DHCLIENT_FLAG_HEX == p_db_if->dhclient_if->client_flags)
                {
                    ret = _dhclient_parse_hex_string(p_db_if->dhclient_if->client_id, buf, &olen);
                    if (PM_E_NONE != ret)
                    {
                        return ret;
                    }
                    sal_snprintf(type, COMM_NAME_SIZE, "hex");
                }
                else if (GLB_DHCLIENT_FLAG_ASCII == p_db_if->dhclient_if->client_flags)
                {
                    olen = sal_snprintf(buf, M_DHC_ID_MAX_LEN, "%s", p_db_if->dhclient_if->client_id);
                    sal_snprintf(type, COMM_NAME_SIZE, "ascii");
                }
                else if (GLB_DHCLIENT_FLAG_PORT == p_db_if->dhclient_if->client_flags)
                {
                    ret = dhclient_get_clientid_by_interface(p_db_if->key.name, p_db_if->dhclient_if->client_id, buf, &olen);
                    if (RESULT_OK != ret)
                    {
                        return ret;
                    }
                    sal_snprintf(type, COMM_NAME_SIZE, "%s", p_db_if->dhclient_if->client_id);
                }
                else
                {
                    DHCLIENT_LOG_ERR("Unknown dhclient client type");        
                    return 0;
                }
                dhclient_proc_send_option(p_db_if->key.name, M_DHC_CLIENT_ID_CODE, buf, olen, type);
                
            }
            break;

        case DS_DHCLIENT_IF_FLD_OPTION:
            //hsrv_interface_set_speed(p_sys_if);
            break;

        case DS_DHCLIENT_IF_FLD_STATUS:
            //hsrv_interface_set_speed(p_sys_if);
            break;

        case DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS:
            if (AF_INET != dhclient_get_local_family())
            {
                return rc;    
            }
            
            return dhclient_proc_req_lists(p_db_if->key.name, p_db_if->dhclient_if->requested_flags, 0, arr_req);
            break;

        case DS_DHCLIENT_IF_FLD_IANA_T1:
            if (AF_INET6 != dhclient_get_local_family())
            {
                return rc;    
            }

            t1 = p_db_if->dhclient_if->iana_t1;
            dhclient_proc_send_option(p_db_if->key.name, M_DHC_IPV6_IANA_T1_CODE,  (char *)&t1, sizeof(uint32), NULL);
            break;

        case DS_DHCLIENT_IF_FLD_IANA_T2:
            if (AF_INET6 != dhclient_get_local_family())
            {
                return rc;    
            }

            t2 = p_db_if->dhclient_if->iana_t2;
            dhclient_proc_send_option(p_db_if->key.name, M_DHC_IPV6_IANA_T2_CODE,  (char *)&t2, sizeof(uint32), NULL);
            break;

        default:
            break;
    }

    return rc;
}

bool 
dhclient_check_dhcp_client_enable(tbl_route_if_t * p_db_rt_if)
{
    if (p_db_rt_if->dhcp_client_enable &&  (AF_INET == dhclient_get_local_family()))
    {
        return TRUE;    
    }

    if ((p_db_rt_if->dhcpv6_client_enable) && (AF_INET6 == dhclient_get_local_family()))
    {
        return TRUE;
    }
    
    return FALSE;
}

int32
dhclient_intf_change_mode(tbl_interface_t *p_db_if, tbl_route_if_t * p_db_rt_if)
{
    int32 rc = PM_E_NONE;

    /* enable dhclient in port ?*/
    if ((NULL != p_db_rt_if) && dhclient_check_dhcp_client_enable(p_db_rt_if)) 
    {
        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
            //dhclient_recv_event(p_db_if->key.name, DHC_EVENT_ENABLE);
        }
        else if (GLB_IF_MODE_L2 == p_db_if->mode)
        {
            dhclient_recv_event(p_db_if->key.name, DHC_EVENT_DISABLE);
        }
    }

    // = dhclient_recv_interface_type_change(p_db_if);
    return rc;
}

int32
dhclient_intf_change_up(tbl_interface_t *p_db_if, tbl_route_if_t * p_db_rt_if)
{
    int32 rc = PM_E_NONE;

    /* enable dhclient in port ?*/
    if (NULL != p_db_rt_if && dhclient_check_dhcp_client_enable(p_db_rt_if)) 
    {
        if (tbl_interface_is_running(p_db_if))
        {
            dhclient_recv_event(p_db_if->key.name, DHC_EVENT_ENABLE);
        }
        else
        {
            dhclient_recv_event(p_db_if->key.name, DHC_EVENT_RELEASE);
        }
    }

    dhclient_recv_interface_state(p_db_if);
    return rc;
}

/* modified by liwh for bug 50211, 2018-12-20 */
int32
dhclient_intf_change_running(tbl_interface_t *p_db_if, tbl_route_if_t * p_db_rt_if)
{
    int32 rc = PM_E_NONE;

    /* enable dhclient in port ?*/
    if (NULL != p_db_rt_if && dhclient_check_dhcp_client_enable(p_db_rt_if)) 
    {
        if (tbl_interface_is_running(p_db_if))
        {
            dhclient_recv_event(p_db_if->key.name, DHC_EVENT_ENABLE);
        }
        else
        {
            dhclient_recv_event(p_db_if->key.name, DHC_EVENT_RELEASE);
        }
    }

    dhclient_recv_interface_state(p_db_if);
    return rc;
}
/*liwh end */

int32
dhclient_process_intf_if(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;

    tbl_route_if_t * p_db_rt_if = NULL;
    tbl_route_if_key_t rt_if_key;

    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    tbl_interface_t *p_sys_if = (tbl_interface_t*)p_tbl;

    if (NULL != p_if_key) {
        p_db_if = tbl_interface_get_interface(p_if_key);

        if (NULL == p_db_if) {
            return PM_E_NONE;
        }
    }

    /* physical, agg, vlan support only*/
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
    && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
    && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type
    )
    {
        return PM_E_NONE;
    }

    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, p_db_if->key.name, sizeof(rt_if_key.name));
    p_db_rt_if = tbl_route_if_get_route_if(&rt_if_key);
    
    switch (oper) {
        case CDB_OPER_ADD:
            if (p_ds_node)
            {
                /* add sub DS */
                break;
            }
                        
            rc = dhclient_intf_if_add(p_db_if);
            break;

        case CDB_OPER_DEL:
            if (p_ds_node) {
                /* delete sub DS */
                break;
            }
            
            /* enable dhclient in port ?*/
            if (NULL != p_db_rt_if
                && dhclient_check_dhcp_client_enable(p_db_rt_if)) {
                dhclient_recv_event(p_db_if->key.name, DHC_EVENT_DISABLE);
            }

            if (p_if_key && '\0' != p_if_key->name[0]) {
                rc = dhclient_intf_if_del(p_db_if);
            }
            break;

        case CDB_OPER_SET:
            if (p_ds_node)
            {
                /* cfg sub DS */
                rc = dhclient_intf_if_cfg(p_db_if, p_sys_if, field_id);
                break;
            }
            else
            {
                switch (field_id)
                {
                    case TBL_INTERFACE_FLD_MODE:
                        rc = dhclient_intf_change_mode(p_db_if, p_db_rt_if);
                        break;

                    case TBL_INTERFACE_FLD_UP:
                        rc = dhclient_intf_change_up(p_db_if, p_db_rt_if);
                        break;

                    /* modified by liwh for bug 50211, 2018-12-20 */
                    case TBL_INTERFACE_FLD_RUNNING:
                        rc = dhclient_intf_change_running(p_db_if, p_db_rt_if);
                        break;
                    /* liwh end */
                        
                    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
                        break;

                    /* modified by liwh for bug 50200, 2018-12-24 */
                    case TBL_INTERFACE_FLD_MAC_ADDR:
                        rc = dhclient_intf_update_mac_address(p_db_if);
                        break;
                    /* liwh end */

                    default:
                        break;
                }
            }
            break;
            
        default:
            break;
        }

    return rc;
}

/*******************************************************************************
 * Name:    dhclient_mgmt_init
 * Purpose: initialize dhcp client module
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
int dhclient_mgmt_init(void)
{
    struct interface_info *ip = NULL;
        
    if (g_dhc_initialized)
    {
        return PM_E_FAIL;
    }

    if (ISC_R_SUCCESS != interface_allocate (&ip, MDL))
    {
        DHCLIENT_LOG_ERR("Warning: Out of Memory!");
        return PM_E_FAIL;
    }

    sal_snprintf(ip->name, IFNAMSIZ, "%s", dhclient_get_mgmt_if_name());
    ip->index = -1;
    ip->vrfid = DHCLIENT_MANG_VRF;
    _dhclient_add_into_intf_list(&free_intf_list, ip);
    interface_dereference (&ip, MDL);
    g_dhc_initialized = 1;
    return PM_E_NONE;
}

/*******************************************************************************
 * Name:    dhclient_mgmt_free
 * Purpose: release dhcp client module
 * Input:  N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhclient_mgmt_free(void)
{
    if (0 == g_dhc_initialized)
    {
        return;
    }

    g_dhc_initialized = 0;
    g_dhc_ctl_blk.flags = DHCLIENT_DEFAULT_FLAGS;
    g_dhc_ctl_blk.distance = DHCLIENT_DEFAULT_ROUTER_DISTANCE;

    return;
}

int dhclient_proc_req_lists(char *ifname, 
        u_int32_t flags, int badd, dhc_req_option_t *arr)
{
    struct interface_info *ip;
    int cnt;
    struct option **ppopts = NULL;
    dhc_req_option_t *p = arr;
    struct client_config *config;
    int is_free_list = 0;
    u_int32_t newflags;

    (void)is_free_list;
    if (NULL != (ip = dhclient_locate_interface(free_intf_list, ifname)))
    {
        is_free_list = 1;
    }
    else if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))     
    {
        /* for bug 18911 added by liuayng 2012-4-19 
        ** interface add msg may not be receive now , we should create it first 
        */
        if (ISC_R_SUCCESS != interface_allocate (&ip, MDL))
        {
            DHCLIENT_LOG_ERR("Warning: Out of Memory!");
            return RESULT_OK;
        }  

        snprintf(ip->name, IFNAMSIZ, ifname);
        ip->index = -1;
        ip->rfdesc = ip->wfdesc = -1;
//        ip->vrfid = VRF_ID_UNSPEC;
        ip->vrfid = 0;
        ip->flags |= INTERFACE_SHUTDOWN; /* setting shutdown flag */

        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));
        }
        else
        {
            DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));    
        }
        
        _dhclient_add_into_intf_list(&free_intf_list, ip);
    }
    
    if (!ip->client)
    {
        if (!_dhclient_prepare_config(ip))
        {
            return PM_E_INVALID_PARAM;
        }
    }

/* modified by liul for bug 44561, routed sync whole result not single bit to dhclient */
    newflags = flags;
    newflags |= M_DHC_REQ_MUST;

    cnt = 1; /* for hold the last NULL */
    while (NULL != p->name)
    {
        if (p->flag & newflags)
        {
            cnt++;
        }
        p++;
    }

    ppopts = dmalloc(cnt * sizeof(struct option *), MDL);
    if (NULL == ppopts)
    {
        return PM_E_INVALID_PARAM;
    }
    cnt = 0;
    p = arr;
    while (NULL != p->name)
    {
        if (p->flag & newflags)
        {
            unsigned code = p->code;
            option_code_hash_lookup(&ppopts[cnt], dhcp_universe.code_hash,
                    &code, 0, MDL);
            cnt++;
        }
        p++;
    }

    config = ip->client->config;
    _dhclient_free_option_list(config->requested_options);
    config->requested_options = ppopts;
    config->requested_flags = newflags;
    return RESULT_OK;
}

char * 
_get_dhclient_command_name(int what)
{
    if (DHC_EVENT_ENABLE == what)
    {
        return "enable";
    }
    else if (DHC_EVENT_DISABLE == what)
    {
        return "disable";
    }

    return "release";
}

int dhclient_recv_event (char *ifname, int what)
{
    struct interface_info *ip;

    

    if (what & (DHC_EVENT_RELEASE | DHC_EVENT_DISABLE))
    {
        
        if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))
        {
            if (AF_INET == dhclient_get_local_family())
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                    "Locate interface %s not exist when process %s message",
                    _get_intf_display_name(ifname), (what & DHC_EVENT_RELEASE) 
                    ?  "release" : "disable");
            }
            else
            {
                DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                    "Locate interface %s not exist when process %s message",
                    _get_intf_display_name(ifname), (what & DHC_EVENT_RELEASE) 
                    ?  "release" : "disable");
            }

            return RESULT_OK;
        }

        if ((NULL != ip) && (NULL != ip->client) && (S_STOPPED != ip->client->state))
        {
            if (AF_INET == dhclient_get_local_family())
            {
                 DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                    "Dhclient recv command %s for interface %s", 
                    _get_dhclient_command_name(what), _get_intf_display_name(ifname));
            }
            else
            {
                DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                    "Dhclientv6 recv command %s for interface %s", 
                    _get_dhclient_command_name(what), _get_intf_display_name(ifname));
            }       
        }

        if (AF_INET == dhclient_get_local_family())
        {
            if (ip->client && ip->client->active)
            {
                do_release(ip->client);
            }

            stop_timers(ip);
        }
        /* modified by liwh for bug 49365, 2018-11-24 */
        else
        {        
            if (ip->client && ip->client->active_lease)
            {
                start_release6(ip->client);
            }
            /* modified by liwh for bug 50556, 2018-12-26 */
            else
            {
                stop_timers(ip);
            }
            /* liwh end */

            dhclient_del_duid(ip);
        }
        /* liwh end */
 
        if (what & DHC_EVENT_DISABLE)
        {
            _dhclient_disable_interface(ip);
        }
    }
    else if (what & DHC_EVENT_ENABLE)
    {
        ip = dhclient_register_interface(ifname);
        if (NULL == ip)
        {
            DHCLIENT_LOG_ERR("Warning: out of memory!!!");
            return RESULT_OK;
        }

        if ((NULL != ip) && (NULL != ip->client) && (S_STOPPED == ip->client->state))
        {
            if (AF_INET == dhclient_get_local_family())
            {
                 DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                    "Dhclient recv command %s for interface %s", 
                    _get_dhclient_command_name(what), _get_intf_display_name(ifname));
            }
            else
            {
                DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                    "Dhclientv6 recv command %s for interface %s", 
                    _get_dhclient_command_name(what), _get_intf_display_name(ifname));
            }       
        }

        if (ip->client && S_STOPPED == ip->client->state)
        {
             start_state_machine(ip);
        }
    }
    else
    {
        DHCLIENT_LOG_ERR("Internal error: unknown command 0x%x", what);        
    }
    return RESULT_OK;
}

int
dhclient_insert_send_option(struct option_state **op,
        u_int32_t code, char *buf, int len)
{
    struct option *option = NULL;
    struct option_cache *oc = NULL;

    if (!(option_code_hash_lookup(&option, dhcp_universe.code_hash, &code, 0, MDL) 
        && make_const_option_cache(&oc, NULL, (u_char *)buf, len, option, MDL)))
    {
        DHCLIENT_LOG_ERR("Prepare send option %d failed (out of memory)", code);        
        return 0;
    }
    else 
    {
        save_option (&dhcp_universe, *op, oc);
        option_cache_dereference (&oc, MDL);
    }
    option_dereference(&option, MDL);
    return 1;
}

int
dhclient_fill_lease_option(struct client_state *client, struct option_state **op)
{
    dhc_send_options_t *popts;
    u_int32_t lease = M_DHC_DEFAULT_LEASE_TIME;

    if (NULL == client || NULL == op || NULL == client->config)
    {
        return 0;
    }

    popts = (dhc_send_options_t *)client->config->sdata;
    if (NULL != popts)
    {
        if (popts->lease == 0)
        {
            /* lease == 0, we don't send lease option */
            return 1;
        }

        lease = popts->lease;
    }
    lease = htonl(lease);
    if (!dhclient_insert_send_option(op, M_DHC_LEASE_CODE, (char *)&lease, 4)) 
    {
        return 0;
    }
    return 1;
}

int dhclient_fill_hostname_option(struct client_state *client, struct option_state **op)
{
    dhc_send_options_t *popts;
    char hostname[MAXHOSTNAMELEN];
    char *p;

    if (NULL == client || NULL == op || NULL == client->config)
    {
        return 0;
    }

    popts = (dhc_send_options_t *)client->config->sdata;
    if (NULL != popts && NULL != popts->hostname && ('\0' != popts->hostname[0]))
    {
        p = popts->hostname;
    }
    else
    {
        gethostname(hostname, MAXHOSTNAMELEN);
        p = hostname;
    }

    if (!dhclient_insert_send_option(op, M_DHC_HOSTNAME_CODE, p, strlen(p)))
    {
        return 0;
    }
    return 1;
}

int dhclient_fill_client_id_option(struct client_state *client, struct option_state **op)
{
    dhc_send_options_t *popts;
    char client_id[M_DHC_ID_MAX_LEN];
    int len;
    char *p;

    if (NULL == client || NULL == op || NULL == client->config)
    {
        return 0;
    }

    popts = (dhc_send_options_t *)client->config->sdata;
    if (NULL != popts && NULL != popts->client_id)
    {
        p = popts->client_id;
        len = popts->client_id_len;
        /*for bug 18802 added by liuyang 2012-4-24*/
        if (strcmp(popts->client_type, "ascii")
            && strcmp(popts->client_type, "hex")  
            && 0 == popts->client_id_len)
        {
            if (RESULT_OK == dhclient_option_client_id_update(popts))
            {
                p = popts->client_id;
                len = popts->client_id_len;
            }
            else
            {
                DHCLIENT_LOG_ERR("Failed to Update client id(%s) for %s \n",
                    popts->client_type, client->interface->name);
                return 0;            
            }
        }
    }
    else
    {
        char hwaddr[M_DHC_ID_MAX_LEN];
        unsigned char *q = &client->interface->hw_address.hbuf[1];

        snprintf(hwaddr, M_DHC_ID_MAX_LEN, 
                "%02x%02x.%02x%02x.%02x%02x",
                q[0], q[1], q[2], q[3], q[4], q[5]);
        len = snprintf(client_id, M_DHC_ID_MAX_LEN,
                "switch-%s-%s", hwaddr, 
                _get_intf_display_name(client->interface->name));
        p = client_id;
    }

    if (!dhclient_insert_send_option(op, M_DHC_CLIENT_ID_CODE, p, len))
    {
        return 0;
    }

    return 1;
}

int dhclient_fill_class_id_option(struct client_state *client, struct option_state **op)
{
    dhc_send_options_t *popts;

    if (NULL == client || NULL == op || NULL == client->config)
    {
        return 0;
    }

    popts = (dhc_send_options_t *)client->config->sdata;
    if (NULL != popts && NULL != popts->class_id)
    {
        if (!dhclient_insert_send_option(op, M_DHC_CLASS_ID_CODE, 
                    popts->class_id, popts->class_id_len))
        {
            return 0;
        }
    }
    return 1;
}

int dhclient_fill_send_options(struct client_state *client, struct option_state **op)
{
    /* dhcp-max-message-size */
    u_int16_t msize = htons(M_DHC_MAX_DHCP_MSG_SIZE); 

    if (!dhclient_insert_send_option(op, M_DHC_MAX_DHCP_MSG_SIZE_CODE, 
                (char *)&msize, sizeof(msize)))
    {
        return 0;
    }

    if (!dhclient_fill_lease_option(client, op))
    {
        return 0;
    }

    if (!dhclient_fill_hostname_option(client, op))
    {
        return 0;
    }

    if (!dhclient_fill_client_id_option(client, op))
    {
        return 0;
    }

    return dhclient_fill_class_id_option(client, op);
}

int dhclient_is_startup_done()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->startup_done) {
        return 1;
    }
    return 0;
}

int
dhclient_get_clientid_by_interface(char *cur, char *ifname, char *obuf, int *olen)
{
    struct interface_info *ip;
    //int is_free_list = 0;

    if (NULL != (ip = dhclient_locate_interface(free_intf_list, ifname)))
    {
        //is_free_list = 1;
    }
    else if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))     
    {  
        if (dhclient_is_startup_done() && sal_strcmp(ifname, cur))
        {
            DHCLIENT_LOG_ERR("Locate interface %s failed when extract client id\n", _get_intf_display_name(ifname));
            return PM_E_FAIL;
        } 
        *olen = 0;
        return RESULT_OK;
    }
    
    if (ip->hw_address.hlen <= 0)
    {
        if (0 != dhclient_init_hardware_address(ip))
        {
            DHCLIENT_LOG_ERR("Get hardware address for interface %s failed\n", ip->name);
            return PM_E_FAIL;
        }
    }
    sal_memcpy(obuf, &ip->hw_address.hbuf[1], ip->hw_address.hlen - 1);
    *olen = ip->hw_address.hlen - 1;
    return RESULT_OK;
}

int
dhclient_del_send_option(dhc_send_options_t **ppopts, u_int32_t code) 
{
    dhc_send_options_t *p;

    if (NULL == ppopts || NULL == *ppopts)
    {
        return 0;
    }

    p = *ppopts;
    switch (code)
    {
    case M_DHC_CLIENT_ID_CODE:
        if (NULL != p->client_id)
        {
            dfree(p->client_id, MDL);
            p->client_id = NULL;
            p->client_id_len = 0;
        }
        break;
    case M_DHC_CLASS_ID_CODE:
        if (NULL != p->class_id)
        {
            dfree(p->class_id, MDL);
            p->class_id = NULL;
            p->class_id_len = 0;
        }
        break;
    case M_DHC_HOSTNAME_CODE:
        if (NULL != p->hostname)
        {
            dfree(p->hostname, MDL);
            p->hostname = NULL;
        }
        break;
    case M_DHC_LEASE_CODE:
        if (0 != p->lease)
        {
            p->lease = M_DHC_DEFAULT_LEASE_TIME;
        }
        break;
    default:
        log_error("%% Unknown option code %d\n", code);
        return PM_E_INVALID_PARAM;
    }

    if (0 == p->lease
        && NULL == p->hostname
        && NULL == p->client_id
        && NULL == p->class_id)
    {
        /* no any options, free it */
        dfree(p, MDL);
        *ppopts = NULL;
    }

    return 0;
}

int
dhclient_add_send_option(dhc_send_options_t **ppopts,
        u_int32_t code, char *buf, int len, char *type)
{
    dhc_send_options_t *p;
    char *mem = NULL;
    int b_alloced = 0;
    
    if (NULL == ppopts)
    {
        return 0;
    }

    if (NULL == (p = *ppopts))
    {
        p = (dhc_send_options_t *)dmalloc(sizeof(dhc_send_options_t), MDL);
        if (NULL == p)
        {
            log_error("Process option %d failed (out of memory)\n", code);
            return PM_E_INVALID_PARAM;
        }
        b_alloced = 1;
        p->lease = M_DHC_DEFAULT_LEASE_TIME;

        /* modified by liwh for bug 49365, 2018-11-29 */
        p->dhcpv6_iana_t1 = DHCPV6_CLIENT_DEFAULT_IANA_T1_TIME;
       /* liwh end */
    }

    if (M_DHC_LEASE_CODE == code)
    {
        p->lease = *((u_int32_t *)buf);
    }
    /* modified by liwh for bug 49365, 2018-11-29 */
    else if (M_DHC_IPV6_IANA_T1_CODE == code)
    {
        p->dhcpv6_iana_t1 = *((u_int32_t *)buf);
    }
    /* liwh end */
    else
    {
        mem = dmalloc(len + 1, MDL); /* 1 more bytes for '\0' */
        if (NULL == mem)
        {
            log_error("%% Setting option code %d failed (out of memory)\n", code);
            if (b_alloced)
            {
                dfree(p, MDL);
            }
            return PM_E_INVALID_PARAM;
        }
        switch (code)
        {
        case M_DHC_CLIENT_ID_CODE:
            if (NULL != p->client_id)
            {
                dfree(p->client_id, MDL);
            }
            p->client_id_len = len;
            p->client_id = mem;
            snprintf(p->client_type, IFNAMSIZ, type);
            break;
        case M_DHC_CLASS_ID_CODE:
            if (NULL != p->class_id)
            {
                dfree(p->class_id, MDL);
            }
            p->class_id_len = len;
            p->class_id = mem;
            if (!strcmp(type, "ascii"))
            {
                p->class_type = 'a';
            }
            else
            {
                p->class_type = 'h';
            }
            break;
        case M_DHC_HOSTNAME_CODE:
            if (NULL != p->hostname)
            {
                dfree(p->hostname, MDL);
            }
            p->hostname = mem;
            break;
        case M_DHC_LEASE_CODE:
        default:
            log_error("%% Unknown option code %d\n", code);
            if (b_alloced)
            {
                dfree(p, MDL);
            }
            dfree(mem, MDL);
            return PM_E_INVALID_PARAM;
        }
        memcpy(mem, buf, len);
        mem[len] = '\0';
    }

    if (b_alloced)
    {
        *ppopts = p;
    }

    return 0;
}

int dhclient_proc_send_option(char *ifname, u_int32_t code,
        char *buf, int len, char *type)
{
    struct interface_info *ip;
    dhc_send_options_t **ppopts;
    int is_free_list = 0;

    (void)is_free_list;
    if (NULL != (ip = dhclient_locate_interface(free_intf_list, ifname)))
    {
        is_free_list = 1;
    }
    else if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))     
    {
        /* for bug 18911 modified by liuayng 2012-4-19 */
        if (ISC_R_SUCCESS != interface_allocate (&ip, MDL))
        {
            DHCLIENT_LOG_ERR("Warning: Out of Memory!");
            return 0;
        }

        snprintf(ip->name, IFNAMSIZ, ifname);
        ip->index = -1;
        ip->rfdesc = ip->wfdesc = -1;
        ip->vrfid = 0;
        ip->flags |= INTERFACE_SHUTDOWN; /* setting shutdown flag */

        if (AF_INET == dhclient_get_local_family())
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));
        }
        else
        {
             DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS,
                "Add interface %s into free list",
                _get_intf_display_name(ip->name));   
        }
        
        _dhclient_add_into_intf_list(&free_intf_list, ip);
    }
    
    if (!ip->client)
    {
        if (!_dhclient_prepare_config(ip))
        {
            return PM_E_INVALID_PARAM;
        }
    }

    if (M_DHC_IPV6_IANA_T1_CODE == code)
    {
        //ip->client->config->requested_lease = 2 * (*((u_int32_t *)buf));
        ip->client->config->iana_t1= *((u_int32_t *)buf);
        return 0;
    }
    else if (M_DHC_IPV6_IANA_T2_CODE == code)
    {
        ip->client->config->iana_t2= *((u_int32_t *)buf);
        return 0;
    }

    ppopts = (dhc_send_options_t **)&ip->client->config->sdata;
    if (NULL == buf)
    {
        /* delete option */
        return dhclient_del_send_option(ppopts, code);
    }

    /* add option */
    return dhclient_add_send_option(ppopts, code, buf, len, type);;
}

int _dhclient_get_cls_router(struct dhc_comm_rt_cb_arg *args,
        dhc_comm_routers_rec_t *prec, unsigned char distance)
{
    char *buf = (char *)args->data;
    char szRouter[DHC_PREFIX_BUF_LEN];
    char szDest[DHC_PREFIX_BUF_LEN];
    struct in_addr imask;
    int len = sal_strlen(buf);

    sal_inet_ntop (prec->router.family, &prec->router.prefix,
            szRouter, DHC_PREFIX_BUF_LEN);
    sal_inet_ntop (prec->dest.family, &prec->dest.prefix,
            szDest, DHC_PREFIX_BUF_LEN);
    sal_masklen2ip (prec->mask_width, &imask);
    sal_snprintf(buf + len, DHC_DISP_BUF_SIZE - len, 
            "\n    Destination: %s, mask: %s, Nexthop: %s",
            szDest, inet_ntoa(imask), szRouter);
    return RESULT_OK;
}

int _dhclient_get_cfs_router(struct dhc_comm_rt_cb_arg *args,
        dhc_comm_routers_rec_t *prec, unsigned char distance)
{
    char *buf = (char *)args->data;
    char szRouter[DHC_PREFIX_BUF_LEN];
    char szDest[DHC_PREFIX_BUF_LEN];
    int len = sal_strlen(buf);

    sal_inet_ntop (prec->router.family, &prec->router.prefix,
            szRouter, DHC_PREFIX_BUF_LEN);
    sal_inet_ntop (prec->dest.family, &prec->dest.prefix,
            szDest, DHC_PREFIX_BUF_LEN);
    sal_snprintf(buf + len, DHC_DISP_BUF_SIZE - len, 
            "\n    Destination: %s, Nexthop: %s", szDest, szRouter);
    return RESULT_OK;
}

void init_fallback_interface(void)
{
    maybe_setup_fallback();    
#if defined (HAVE_SETFD)
    if (fallback_interface) 
    {
        if (fcntl (fallback_interface -> rfdesc, F_SETFD, 1) < 0)
        {
            log_error ("Can't set close-on-exec on fallback: %m");
        }
        if (fallback_interface -> rfdesc != fallback_interface -> wfdesc) 
        {
            if (fcntl (fallback_interface -> wfdesc, F_SETFD, 1) < 0)
            {
                log_error ("Can't set close-on-exec on fallback: %m");
            }
        }
    }
#endif
    return;
}

#ifdef HAVE_SMARTCFG /* Modified by Alexander 2011-08-10 16:11 */
int dhclient_foreach_bounded_intf(scfgctl_bound_cb_t pFunc, void *pArg)
{
    struct interface_info *ip;

    for (ip = interfaces; ip; ip = ip->next)
    {
        struct client_state *client = ip->client;
        if (NULL == client || NULL == client->active)
        {
            continue;
        }

        struct client_lease *lease = client->active;
        char tftpsrv_addr_buf[DHC_MAX_TFTPSRV_BUF_LEN];
        int tftpsrv_addr_len;

        tftpsrv_addr_len = DHC_MAX_TFTPSRV_BUF_LEN;
        if (dhclient_get_tftpsrv_addresses(client, lease, 
                    tftpsrv_addr_buf, &tftpsrv_addr_len))
        {
            pFunc(_get_intf_display_name(ip->name), ip->vrfid, 
                    tftpsrv_addr_buf, tftpsrv_addr_len, pArg);
        }
    }

    return 0;
}

void dhclient_do_reboot(char *msg, size_t len)
{
    struct interface_info *ip;

    for (ip = interfaces; ip; ip = ip->next)
    {
        struct client_state *client = ip->client;
        if (NULL == client || NULL == client->active)
        {
            continue;
        }

        do_release(ip->client);
    }

    if (ZG->nc && ZG->nc->async)
    {
        dhc_comm_send_cmd(ZG->nc->async, DHC_CMD_REBOOT,
                msg, len, dhclient_get_mgmt_if_name(), CHSM_MANG_VRF);
    }
    return;
}
#endif /* !HAVE_SMARTCFG */

int
dhclient_pdu_rx(int sock, int sess_fd, ctc_msg_t *p_msg)
{
    int result;
    uint32  ifindex = 0;
    struct sockaddr_in from;
    struct hardware hfrom;
    struct iaddr ifrom;
    tbl_interface_t     *p_db_if = NULL;
    tbl_interface_t     *p_db_vlan_if = NULL;
    struct interface_info *ip = NULL;
    tbl_interface_key_t if_key;
    uint16      vlanid = 0;
    char        name[IFNAME_SIZE];
    int                      is_unicast = 0;
    /* modified by liwh for bug 49365, 2018-11-23 */
    struct sockaddr_in6 from6;
    struct in6_addr to;
    mac_header_t     *p_mac_header = NULL;
    /* liwh end */

    /* check ethertype */
    p_mac_header = (mac_header_t*)p_msg->data;

    if (AF_INET == dhclient_get_local_family())
    {
        if (p_mac_header->eth_type != ntohs(ETH_P_IPV4))
        {
            return ISC_R_UNEXPECTED;
        }
    }
    else
    {
        if (p_mac_header->eth_type != ntohs(ETH_P_IPV6))
        {
            return ISC_R_UNEXPECTED;
        }
    }

    union {
    	unsigned char packbuf [10240]; /* Packet input buffer.
    				 	 Must be as large as largest
    					 possible MTU. */
    	struct dhcp_packet packet;
    } u;

    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if) {
        return PM_E_FAIL;
    }

    memcpy(name, p_db_if->key.name, IFNAME_SIZE);

    /* receive from vlan interface ? */
    vlanid = p_msg->u_hdr.pkt.vid;
    if (0 != vlanid) {

        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_VLAN_PREFIX"%u", vlanid);
        p_db_vlan_if = tbl_interface_get_interface(&if_key);
        if (NULL  == p_db_vlan_if)
        {
            return 0;
        }

        memcpy(name, p_db_vlan_if->key.name, IFNAME_SIZE);

//#ifdef _GLB_UML_SYSTEM_
        if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            p_msg->data_len = p_msg->data_len - 4;

#ifndef _GLB_UML_SYSTEM_
            if (p_db_if->brgif)
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS, "dhclient dhclient_pdu_rx packet  is vlanid %d p_db_if->brgif->pvid %d\n", p_msg->data_len, p_db_if->brgif->pvid);

                if (p_db_if->brgif->pvid == vlanid)
                {
                    p_msg->data_len = p_msg->data_len + 4;  
                }
            }
#endif
        }
//#endif
    }

    for (ip = interfaces; ip; ip = ip->next) {
    	if (!strcmp(ip->name, name))
    		break;
    }

    /* this port is not enable dhclient now, return */
    if (ip == NULL) {
        return PM_E_NONE;
    }

    /* it should a omapi_interface type, not omapi_io or other type */
    if (ip-> type != dhcp_type_interface)
    	return DHCP_R_INVALIDARG;

   
    /* parse pakcet bootstrap header, put it to u.packbuf */
      again:
/* modified by liwh for bug 49365, 2018-11-23 */
    if (AF_INET == dhclient_get_local_family())
    {        
/*liwh end */       
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS, "dhclient dhclient_pdu_rx packet length is %d\n", p_msg->data_len);

	if ((result =
	     dhcrelay_receive_packet (ip, u.packbuf, sizeof u, &from, &hfrom, (unsigned char *)p_msg->data, p_msg->data_len)) < 0) {
		return ISC_R_UNEXPECTED;
	}
	if (result == 0)
		return ISC_R_UNEXPECTED;

	/*
	 * If we didn't at least get the fixed portion of the BOOTP
	 * packet, drop the packet.
	 * Previously we allowed packets with no sname or filename
	 * as we were aware of at least one client that did.  But
	 * a bug caused short packets to not work and nobody has
	 * complained, it seems rational to tighten up that
	 * restriction.
	 */
	if (result < DHCP_FIXED_NON_UDP)
		return ISC_R_UNEXPECTED;
//todo:check use_v4_pktinfo
#if defined(IP_PKTINFO) && defined(IP_RECVPKTINFO) && defined(USE_V4_PKTINFO)
	{
		/* We retrieve the ifindex from the unused hfrom variable */
		unsigned int ifindex;

		memcpy(&ifindex, hfrom.hbuf, sizeof (ifindex));

		/*
		 * Seek forward from the first interface to find the matching
		 * source interface by interface index.
		 */
		ip = interfaces;
		while ((ip != NULL) && (if_nametoindex(ip->name) != ifindex))
			ip = ip->next;
		if (ip == NULL)
			return ISC_R_NOTFOUND;
	}
#endif

       if (bootp_packet_handler) {
		ifrom.len = 4;
		memcpy (ifrom.iabuf, &from.sin_addr, ifrom.len);

		(*bootp_packet_handler) (ip, &u.packet, (unsigned)result,
					 from.sin_port, ifrom, &hfrom);
	}
/* modified by liwh for bug 49365, 2018-11-23 */
    }
    else
    {        
        DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS, "dhclient6 dhclient_pdu_rx packet length is %d\n", p_msg->data_len);    

        if ((result =
	     dhclient6_receive_packet (ip, u.packbuf, sizeof u, &from6, &to, (unsigned char *)p_msg->data, p_msg->data_len)) <= 0) {
		return ISC_R_UNEXPECTED;
	}

        DEBUG_OUT(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS, "dhclient6 dhclient_pdu_rx dhcpv6 length is %d\n", result);

        DEBUG_OUT_HEX(dhcpv6_debug, flags, TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP,
                  "dump the DHCPv6 pkt (receive)", (u_char *)u.packbuf, result, 0);
    
        if (IN6_IS_ADDR_MULTICAST(&to)) { 
	    is_unicast = ISC_FALSE;
	} else {
           is_unicast = ISC_TRUE;
	}
       
	ifrom.len = 16;
	memcpy(ifrom.iabuf, &from6.sin6_addr, ifrom.len);
        
        (*dhcpv6_packet_handler)(ip, (char*)&u.packbuf, 
					 result, from6.sin6_port, 
					 &ifrom, is_unicast);
    }
/*liwh end */

	/* If there is buffered data, read again.    This is for, e.g.,
	   bpf, which may return two packets at once. */
	if (ip->rbuf_offset != ip->rbuf_len)
		goto again;

    return PM_E_NONE;
}

int dhclient_uninstall_intf_ip(struct client_state *client)
{
    char szCmd[128];
    char route_buf[DHC_DISP_BUF_SIZE];
    int   distance = 0;

    route_buf[0] = '\0';
    if (client && client->active)
    {
        dhclient_get_default_router(client, client->active, route_buf);
        if ('\0' != route_buf[0])
        {
            //distance = dhclient_get_distance();
            //sal_snprintf(szCmd, 128, "cdbctl delete/cdb/l3/static_route_cfg/ip/0.0.0.0/0/ipgw/%s/distance/%u/dhcp_client/1", route_buf, distance);
            sal_snprintf(szCmd, 128, "cdbctl delete/cdb/l3/static_route_cfg/ip/0.0.0.0/0/ipgw/%s/dhcp_client/1", route_buf);
            system(szCmd);
        }
    }

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
            "Interface %s uninstall ip", _get_intf_display_name(client->interface->name));

    sal_snprintf(szCmd, 128, "cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/del/all", client->interface->name);
    system(szCmd);

    return PM_E_NONE;
}

int 
dhclient_get_distance(void)
{
    tbl_dhclient_t *pitem = NULL;

    pitem = tbl_dhclient_get_dhclient();

    if (pitem)
    {
        return pitem->distance;
    }

    return DHCLIENT_DEFAULT_ROUTER_DISTANCE;
}


int dhclient_install_intf_ip(struct client_state *client)
{
    char szCmd[128];
    struct client_lease *lease = client->active;
    struct iaddr ia_mask;
    uint32 prefixlen = 0;
    char *psz_mask = NULL;
    union sockunion *mask = NULL;
    struct sal_in4_addr in4_mask;
    char szip[DHC_PREFIX_BUF_LEN];
    int   distance = 0;
    char route_buf[DHC_DISP_BUF_SIZE];

    if (NULL == client->interface || NULL == lease)
    {
        return -1;
    }

    if (NULL == dhclient_get_iaddr_from_options(client, lease, 
                DHO_SUBNET_MASK, &ia_mask))
    {
        return PM_E_FAIL;
    }

    sal_memset(szip, 0, DHC_PREFIX_BUF_LEN);
    sal_snprintf(szip, DHC_PREFIX_BUF_LEN, "%s", piaddr(lease->address));
    mask = sockunion_str2su((char *)piaddr(ia_mask));
    if (NULL == mask)
    {
        return PM_E_FAIL;
    }
    
    psz_mask = sockunion_su2str(mask);

    inet_aton (psz_mask, &in4_mask);
    prefixlen = ip_masklen (in4_mask);
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS,
            "Interface %s install ip %s mask %u", _get_intf_display_name(client->interface->name), szip, prefixlen);

    sal_snprintf(szCmd, 128, "cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/add/%s/%u/", client->interface->name, szip, prefixlen);
    system(szCmd);

    route_buf[0] = '\0';
    if (client && client->active)
    {
        dhclient_get_default_router(client, client->active, route_buf);
        if ('\0' != route_buf[0])
        {
            distance = dhclient_get_distance();
            sal_snprintf(szCmd, 128, "cdbctl create/cdb/l3/static_route_cfg/ip/0.0.0.0/0/ipgw/%s/distance/%u/dhcp_client/1", route_buf, distance);
            system(szCmd);
        }
    }
    
    return PM_E_NONE;
}

int dhclient_install_intf_ipv6(char *ifname, struct iaddr *address)
{
    char szip[64];
    char szCmd[128];
    u_int32_t prefixlen = 64;
    int32 ret = 0;
 
    snprintf(szip, 64, "%s", piaddr(*address));
 
    snprintf(szCmd, 128, "cdbctl update/cdb/l3/route_if/%s/ifc_ipv6/add/ip_prefix/%s/%u/dhcpv6", ifname, szip, prefixlen);
    ret = system(szCmd);

    return ret;
}

int dhclient_uninstall_intf_ipv6(char *ifname, struct iaddr *address)
{
    char szip[64];
    char szCmd[128];
    u_int32_t prefixlen = 64;
 
    snprintf(szip, 64, "%s", piaddr(*address));
 
    snprintf(szCmd, 128, "cdbctl update/cdb/l3/route_if/%s/ifc_ipv6/del/ip_prefix/%s/%u/dhcpv6", ifname, szip, prefixlen);
    system(szCmd);

    return PM_E_NONE;
}

uint32
dhclient_stm_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }
    
    return FALSE;
}

char* print_duid(unsigned len,
	const u_int8_t *data,
	unsigned limit)
{
    u_int32_t    index = 0;
    static char duid_buf[52];

    memset(duid_buf, 0, 52);

    for(index = 0; index < len; index++)
    {
         sprintf(&duid_buf[index*2], "%02x", data[index]);  
    }
     
    return duid_buf;
}

int
_dhclient6_show_one_intf(FILE *fp, struct interface_info *ip, int verbose)
{
    char buf[DHC_ERR_MSG_BUF_LEN];
    struct client_state *client;
    struct dhc6_lease *active_lease;
    struct timeout *ptm;
    char szip[DHC_PREFIX_BUF_LEN];
    char szmask[DHC_PREFIX_BUF_LEN];
    struct dhc6_ia *ia;
    struct dhc6_addr *addr;
    u_int32_t  preferred = 0;
    u_int32_t  expired = 0;
    u_int32_t  passed_time = 0;
    struct data_string *client_duid = NULL;
    u_int32_t  iana_t1 = 0;
    u_int32_t  iana_t2 = 0;
    struct dhc6_addr *last_addr = NULL;

    sal_fprintf(fp, "%s DHCPv6 client information:\n", _get_intf_display_name(ip->name));
    
    client = ip->client;
    if (!client)
    {
        sal_fprintf(fp, "  Current state: HALT\n"); /* fake HALT status */
        return RESULT_OK;
    }

    if (verbose && (S_INIT == client->state)
            && (ptm = get_timeout(do_init6, client)))
    {
        sal_fprintf(fp, "  Current state: %s (timeout %u seconds)\n",
                _dhclient_get_state(client, buf, DHC_ERR_MSG_BUF_LEN),
                (u_int32_t)(ptm->when.tv_sec - cur_time));
    }
    else
    {
        sal_fprintf(fp, "  Current state: %s\n",
                _dhclient_get_state(client, buf, DHC_ERR_MSG_BUF_LEN));
    }

    active_lease = client->active_lease;
    if ((active_lease) && ((S_BOUND == client->state) || (S_RENEWING == client->state) || (S_REBINDING == client->state)))
    {
        for (ia = active_lease->bindings ; ia != NULL ; ia = ia->next) 
        {               
            for (addr = ia->addrs ; addr != NULL ; addr = addr->next) 
            {
                if (addr->flags & DHC6_ADDR_EXPIRED)
                    continue;

                /* modified by liwh for bug 50640, 2019-01-03 */
                if (addr->is_conflict)
                    continue;
                /* liwh end */

                sal_snprintf(szip, DHC_PREFIX_BUF_LEN, "%s", piaddr(addr->address));
                sal_fprintf(fp, "  Allocated IP: %s \n", szip);

                if (cur_time >= addr->starts)
                {
                    passed_time = (u_int32_t)(cur_time - addr->starts);        
                }

                if (addr->preferred_life >= passed_time)
                {
                    preferred = addr->preferred_life - passed_time;
                }
                else
                {
                    preferred = 0;    
                }

                if (addr->max_life >= passed_time)
                {
                    expired = addr->max_life - passed_time;
                }
                else
                {
                    expired = 0;
                }

                if (ia->renew > passed_time)
                {            
                    iana_t1 = ia->renew - passed_time;
                }
                else
                {
                    iana_t1 = 0;    
                }

                if (ia->rebind > passed_time)
                {            
                    iana_t2 = ia->rebind - passed_time;
                }
                else
                {
                    iana_t2 = 0;    
                }
                
                sal_fprintf(fp, "  preferred/expired/renewal/rebinding: %u/%u/%u/%u seconds\n", preferred, expired, iana_t1, iana_t2);

                if(verbose)
                {
                    format_time_str(szip, DHC_PREFIX_BUF_LEN, "%Y-%m-%d %H:%M:%S", addr->starts);
                    format_time_str(szmask, DHC_PREFIX_BUF_LEN,  "%Y-%m-%d %H:%M:%S", (addr->starts + addr->max_life));
                    sal_fprintf(fp, "  Lease from %s to %s\n", szip, szmask);  

                    last_addr = addr;
                }
            }

            if (verbose)
            {
                if (last_addr)
                {
                    format_time_str(szmask, DHC_PREFIX_BUF_LEN,  "%Y-%m-%d %H:%M:%S", (last_addr->starts + ia->renew));
                    sal_fprintf(fp, "  Renews: %s\n", szmask);
                    format_time_str(szmask, DHC_PREFIX_BUF_LEN,  "%Y-%m-%d %H:%M:%S", (last_addr->starts + ia->rebind));
                    sal_fprintf(fp, "  Rebinds: %s\n", szmask);
                }
                
                if (ia)
                {
                    sal_fprintf(fp, "  IAID: 0x%02x%02x%02x%02x\n", ia->iaid[0], ia->iaid[1], ia->iaid[2], ia->iaid[3]);
                }
            }            
        }

        if (verbose)
        {
            if (active_lease->server_id.data)
            {
                sal_fprintf(fp, "  Server DUID: %s\n", print_duid(active_lease->server_id.len,
				      active_lease->server_id.data, 52));
            }
            else
            {
                sal_fprintf(fp, "  Server DUID: N/A\n");
            }

            client_duid = &ip->default_duid;
            if (client_duid && (0 != client_duid->len))
            {
                sal_fprintf(fp, "  Client DUID: %s\n",  print_duid(client_duid->len,
				      client_duid->data, 52));
            }
            else
            {
                sal_fprintf(fp, "  Client DUID: N/A\n");
            }
        }
    }

    sal_fprintf(fp, "  Transaction ID: 0x%02x%02x%02x\n", client->dhcpv6_transaction_id[0], 
            client->dhcpv6_transaction_id[1], client->dhcpv6_transaction_id[2]);

    return RESULT_OK;
}

/* modified by liwh for bug 49365, 2018-11-30 */
int dhclient6_show_intf_status(cfg_cmd_para_t *para, char *ifname, int verbose)
{
    struct interface_info *ip;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");    

    if (verbose)
    {
        sal_fprintf(fp, "DHCPv6 client informations:\n");
        DHC_OUT_SPLIT(fp);
    }

    if (NULL == interfaces && verbose)
    {
        sal_fprintf(fp, "  DHCPv6 client function disabled on all interfaces\n");
        sal_fclose(fp);
        return RESULT_OK;
    }

    if (NULL != ifname && sal_strlen(ifname))
    {
        if (NULL == (ip = dhclient_locate_interface(interfaces, ifname)))
        {
            if (verbose)
            {
                sal_fprintf(fp, "  DHCPv6 client function disabled on interface %s\n",
                        _get_intf_display_name(ifname));
            }
            sal_fclose(fp);
            return RESULT_OK;
        } 

        _dhclient6_show_one_intf(fp, ip, verbose);

        sal_fclose(fp);
        return RESULT_OK;
    }

    for (ip = interfaces; ip; ip = ip->next)
    {
        if (verbose && ip != interfaces)
        {
            DHC_OUT_SPLIT(fp);
        }

        if (RESULT_OK != _dhclient6_show_one_intf(fp, ip, verbose))
        {
            sal_fclose(fp);
            return PM_E_FAIL;
        }
    }

    sal_fclose(fp);

    return RESULT_OK;
}

int dhclient6_show_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");    

    sal_fprintf(fp, "DHCPv6 client packet statistics:\n");
    DHC_OUT_SPLIT(fp);
    sal_fprintf(fp, "DHCPv6 ADVERTISE       received: %u\n", g_pkt_dhc6_stats.recv_advertise);
    sal_fprintf(fp, "DHCPv6 REPLY           received: %u\n", g_pkt_dhc6_stats.recv_reply);
    sal_fprintf(fp, "DHCPv6 Others          received: %u\n", (g_pkt_dhc6_stats.recv_other + g_pkt_dhc6_stats.recv_reconfigure));
    sal_fprintf(fp, "DHCPv6 SOLICIT             sent: %u\n", g_pkt_dhc6_stats.send_solicit);
    sal_fprintf(fp, "DHCPv6 REQUEST             sent: %u\n", g_pkt_dhc6_stats.send_request);
    sal_fprintf(fp, "DHCPv6 RENEW               sent: %u\n", g_pkt_dhc6_stats.send_renew);
    sal_fprintf(fp, "DHCPv6 REBIND              sent: %u\n", g_pkt_dhc6_stats.send_rebind);
    sal_fprintf(fp, "DHCPv6 DECLINE             sent: %u\n", g_pkt_dhc6_stats.send_decline);
    sal_fprintf(fp, "DHCPv6 RELEASE             sent: %u\n", g_pkt_dhc6_stats.send_release);
    sal_fprintf(fp, "DHCPv6 Others              sent: %u\n", (g_pkt_dhc6_stats.send_info_request + g_pkt_dhc6_stats.send_confirm));
    sal_fprintf(fp, "DHCPv6 packet       send failed: %u\n", g_pkt_dhc6_stats.send_failed);

    sal_fclose(fp);
    return RESULT_OK;
}

int dhclient6_clear_statistics()
{
    sal_memset(&g_pkt_dhc6_stats, 0x00, sizeof(dhc6_stats_t));
    return RESULT_OK;
}

int32
dhclient_process_dhclient6_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char        ifname[IFNAME_SIZE];
    //char        *p_ifname = NULL;
    int32       rc  = PM_E_NONE;
    int32       verbose = 0;

    sal_memset(ifname, 0, sizeof(ifname));

    if (FALSE == dhclient_stm_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }  
    
    switch (para->oper)
    {
    case CDB_OPER_DEL:
        if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "statistics"))
            {
                rc = dhclient6_clear_statistics();
            }
        }
        break;
    case CDB_OPER_GET:
        if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "statistics"))
            {
                rc = dhclient6_show_statistics(para);
            }
            
            if (0 == sal_strcmp(argv[0], "verbose"))
            {
                verbose = 1;
                rc = dhclient6_show_intf_status(para, NULL, verbose);
            }
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_strcpy(ifname, argv[1]);
                verbose = 0;
            }

            rc = dhclient6_show_intf_status(para, ifname, verbose);
        }

        else if (3 == argc)
        {
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_strcpy(ifname, argv[1]);
                verbose = 1;
                rc = dhclient6_show_intf_status(para, ifname, verbose);
            }
            else if (0 == sal_strcmp(argv[0], ""))
            {
                verbose = 1;
                rc = dhclient6_show_intf_status(para, NULL, verbose);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}
/* liwh end */

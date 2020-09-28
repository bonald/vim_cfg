/****************************************************************************
* $Id$
*  Macros and structs for DHCP client
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-06-16 08:54
* Reason        : First Create.
****************************************************************************/

#ifndef _DHCLIENT_FUNC_H_
#define _DHCLIENT_FUNC_H_

#include "bsd_queue.h"
#include "param_check.h"
#include <net/if.h>

#define HAVE_DHCLIENT 
#ifdef HAVE_DHCLIENT
#endif

extern int g_dhc_initialized;
typedef struct dhc_control_block
{
    unsigned int flags;

    unsigned char distance; /* default-router distance */
} dhc_control_block_t;

/* interface type check*/
#define DHC_INTF_TYPE_CHECK(name, pcli) \
    do\
    {\
        if (sal_strncmp(name, "eth-", 4) \
            && sal_strncmp(name, "vlan", 4) \
            && sal_strncmp(name, "agg", 3))\
        {\
            if (NULL != pcli)\
            {\
                return RESULT_ERROR;\
            }\
            return RESULT_OK;\
        }\
    }\
    while (0)

union sockunion 
{
  struct sal_sockaddr sa;
  struct sal_sockaddr_in4 sin;
#ifdef HAVE_IPV6
  struct sal_sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
};

#define DHCLIENT_MANG_VRF 64

#define M_DHC_ID_MAX_LEN            64
#define M_DHC_ID_MAX_HEXSTR_LEN     (2 * M_DHC_ID_MAX_LEN)

#define M_DHC_MIN_LEASE_TIME        30 /* 30 seconds */

#define M_DHC_CLIENT_ID_CODE        61
#define M_DHC_CLASS_ID_CODE         60
#define M_DHC_HOSTNAME_CODE         12
#define M_DHC_LEASE_CODE            51 

#define M_DHC_MAX_DHCP_MSG_SIZE_CODE    57        
#define M_DHC_MAX_DHCP_MSG_SIZE         1152
#define M_DHC_DEFAULT_LEASE_TIME        1200 /* 20 m */

/* modified by liwh for bug 49365, 2018-11-29 */
#define M_DHC_IPV6_IANA_T1_CODE           52 
#define M_DHC_IPV6_IANA_T2_CODE           53
/* liwh end */

#define DHCLIENT_LOG_EMERG(fmt, args...)    log_sys(M_MOD_DHCLIENT, E_EMERGENCY, fmt, ##args)
#define DHCLIENT_LOG_ALERT(fmt, args...)    log_sys(M_MOD_DHCLIENT, E_ALERT, fmt, ##args)
#define DHCLIENT_LOG_CRIT(fmt, args...)     log_sys(M_MOD_DHCLIENT, E_CRITICAL, fmt, ##args)
#define DHCLIENT_LOG_ERR(fmt, args...)      log_sys(M_MOD_DHCLIENT, E_ERROR, fmt, ##args)
#define DHCLIENT_LOG_WARN(fmt, args...)     log_sys(M_MOD_DHCLIENT, E_WARNING, fmt, ##args)
#define DHCLIENT_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_DHCLIENT, E_NOTICE, fmt, ##args)
#define DHCLIENT_LOG_INFO(fmt, args...)     log_sys(M_MOD_DHCLIENT, E_INFORMATIONAL, fmt, ##args)

typedef struct dhc_send_options
{
    u_int32_t lease;
    char *hostname;
    char *client_id; /* NULL means we don't use it and '\0' means need to be updated*/
    char *class_id;
    char client_type[IFNAMSIZ]; /* ascii or hex or interface name */
    char class_type;  /* 'a' for ascii, 'h' for hex */
    unsigned char client_id_len;
    unsigned char class_id_len;
    u_int32_t  dhcpv6_iana_t1;
    u_int32_t  dhcpv6_iana_t2;
} dhc_send_options_t;

#define M_REASON_EXPIRED    "lease has expired"
#define M_REASON_NAK        "get dhcpnak message"
#define M_REASON_RENEW      "lease renewed"
/*for bug 24712 modified by liuyang 2013-8-30*/
typedef struct dhc_stats
{
    unsigned int  recv_offer;
    unsigned int  recv_ack;
    unsigned int  recv_nak;
    unsigned int  recv_other;
    unsigned int  send_discover;
    unsigned int  send_decline;
    unsigned int  send_release;
    unsigned int  send_request;
    unsigned int  send_failed;
} dhc_stats_t;

extern dhc_control_block_t g_dhc_ctl_blk;

extern char *config_file;

extern int dhclient_module_init(void);
extern int dhclient_mgmt_init(void);
extern void dhclient_mgmt_free(void);
extern int32 dhclient_init(int pm_id);

extern int dhclient_start(char *conffile);
extern int dhclient_service_onoff(unsigned char bOn);

extern void signal_init ();
extern void dhclient_dump_packet (struct packet *);
extern void dhclient_dump_dhcp_packet (struct dhcp_packet *tdp, int len);

extern struct interface_info *dhclient_register_interface(char *name);
extern struct interface_info *dhclient_locate_interface(struct interface_info *, char *);
extern void dhclient_unregister_interface( char *);

extern void do_release(struct client_state *);
extern void state_reboot (void *);
extern int dhclient_send_bound_msg(struct client_state *);

void stop_timers(struct interface_info *);
int start_state_machine(struct interface_info *);

int dhclient_fill_send_options(struct client_state *, struct option_state **);

int _is_mgmt_dev(char *ifname);
char * _get_intf_display_name(char *ifname);

int dhclient_compare_ip_routers(struct client_state *client, 
        struct client_lease *old, struct client_lease *new);

void init_fallback_interface(void);

int dhclient_pdu_rx(int sock, int sess_fd, ctc_msg_t *p_msg);

int dhclient_uninstall_intf_ip(struct client_state *client);
int dhclient_install_intf_ip(struct client_state *client);
int dhclient_proc_send_option(char *ifname, u_int32_t code,
        char *buf, int len, char *type);

int dhclient_install_intf_ipv6(char *ifname, struct iaddr *address);
int dhclient_uninstall_intf_ipv6(char *ifname, struct iaddr *address);

extern int dhclient_get_local_family(void);

/* modified by liwh for bug 49365, 2018-11-24 */
extern void
do_release6(void *input);
extern void
unconfigure6(struct client_state *client, const char *reason);
extern int32 dhclient6_init(int pm_id);
/* liwh end */

#endif /* !_DHCLIENT_FUNC_H_ */

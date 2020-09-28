#ifndef _DHCPD_FUNC_H_
#define _DHCPD_FUNC_H_

#include "proto.h"
#include "bsd_queue.h"
#include "prefix.h"
//#include "sockunion.h"
//#include "debug.h"

/*DEBUG_DEFINE(dhcp, server,
             DHCPSERVER_E_PKT, DHCPSERVER_E_EVENTS,
             DHCPSERVER_E_ERR, DHCPSERVER_E_DUMP);*/

#define sal_in4_addr in_addr
#define sal_in6_addr in6_addr

/* IPv4 and IPv6 unified prefix structure. */
struct prefix
{
  u_int8_t family;
  u_int8_t prefixlen;
  u_int8_t pad1;
  u_int8_t pad2;
  union 
  {
    u_int8_t prefix;
    struct sal_in4_addr prefix4;
#ifdef HAVE_IPV6
    struct sal_in6_addr prefix6;
#endif /* HAVE_IPV6 */
    struct 
    {
      struct sal_in4_addr id;
      struct sal_in4_addr adv_router;
    } lp;
    u_int8_t val[9];
  } u;
};

/* IPv4 prefix structure. */
struct prefix_ipv4
{
  u_int8_t family;
  u_int8_t prefixlen;
  u_int8_t pad1;
  u_int8_t pad2;
  struct sal_in4_addr prefix;
};

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

#define DHCP_STR_CHECK(_VNAME, _VLEN) \
switch (check_name((_VNAME), (_VLEN)))     \
{   \
    case NAME_ERR_EXCEED_LEN:   \
      cli_out (cli, "%% The name length should not exceed %d\n", (_VLEN));  \
      return CLI_ERROR; \
    case NAME_ERR_FIRST_CHAR:   \
        cli_out(cli, "%% The first character should be a-z or A-Z\n");    \
        return CLI_ERROR;   \
    case NAME_ERR_LAST_CHAR:   \
        cli_out(cli, "%% The last character should be 0-9 or a-z or A-Z\n");    \
        return CLI_ERROR;   \
    case NAME_ERR_INVALID_CHAR:   \
        cli_out(cli, "%% The name character should be \"0-9A-Za-z.-_\"\n");    \
        return CLI_ERROR;   \
    default:    \
        break;  \
}

#define DHCP_CLI_GET_IPV4_PREFIX(NAME,V,STR)                        \
    do {                                                            \
      int retv;                                                        \
      retv = str2prefix_ipv4 ((STR), (struct prefix_ipv4 *)&(V));    \
      if (retv <= 0)                                                \
        {                                                            \
          cli_out (cli, "%% Invalid %s value\n", NAME);                \
          return CLI_ERROR;                                            \
        }                                                            \
    } while (0)
    
#define FREE_LEASES 0
#define ACTIVE_LEASES 1
#define EXPIRED_LEASES 2
#define ABANDONED_LEASES 3
#define BACKUP_LEASES 4
#define RESERVED_LEASES 5

#define LEASE_ACTION_RELEASE_ABANDON 0
#define LEASE_ACTION_RELEASE_ACTIVE 1

/* service switch */
#define DHCP_SERVICE_FLAG                   0x01
#define DHCP_SERVER_SERVICE_FLAG             0x02
#define DHCP_SERVER_MAX_POOL_CNT              256
#define MAX_DHCP_IP_ADDR_CNT                 8
#define POOL_NAME_SIZE                         31
#define DHCP_OPTION_MAX_LEN                    63
#define MAX_POOL_CNT 256
#define MAX_EXCLUDE_IP_CNT 256
#define MAX_LEASE_PER_POOL_CNT 1024

enum dhcp_pool_attr {
    unknowType,
    dynPool,
    staticPool
};

enum dhcp_option_Input_type {
    IP_TYPE,
    ASCII_TYPE,
    HEX_TYPE
};

typedef struct dhcp_option_s{
    struct dhcp_option_s *next;
    unsigned code;
    enum dhcp_option_Input_type type;
    u_int32_t data_len;
    u_int8_t *buf;
}dhcp_option_t;

#define DHCPD_CHK_FLAG(V, flags)        ((V & (flags)) == (flags))
#define DHCPD_SET_FLAG(V, flags)        V |= (flags)
#define DHCPD_CLR_FLAG(V, flags)        V &= (~(flags))

typedef struct dhcp_pool_common_param_s{    
    char *domain_name;
    char * bootfile_name;
    struct sal_in4_addr *dns_server;
    struct sal_in4_addr *gateway;
    struct sal_in4_addr *netbios_server;
    struct sal_in4_addr *tftp_server_addr;
    u_int8_t n_dns_server;
    u_int8_t n_gateway;
    u_int8_t n_netbios_server;
    u_int8_t n_tftp_server_addr;
    u_int32_t lease_time;//0 means default 1 day
    #define LEASE_ONE_MINUTE 60
    #define LEASE_ONE_HOUR (60*LEASE_ONE_MINUTE)
    #define LEASE_ONE_DAY (24*LEASE_ONE_HOUR)
    

    u_int8_t netbios_node_type;
    #define UNKNOWN_NODE    0x00
    #define B_NODE          0x01
    #define P_NODE            0x02
    #define M_NODE            0x04
    #define H_NODE            0x08
    struct dhcp_option_s *options;
}dhcp_pool_common_param_t;

typedef struct dhcp_pool_s{
    char pool_name[POOL_NAME_SIZE+1];
    struct dhcp_pool_s *next;
    enum dhcp_pool_attr pool_type;    
    struct prefix network;
    
    struct prefix fixIp;
    u_int8_t *hw_addr;
    u_int8_t hw_len;
    u_int8_t clientId_len;
    u_int8_t clientId_type;
    #define DHCP_CLENTID_NONE 0
    #define DHCP_CLENTID_ASCII 1
    #define DHCP_CLENTID_HEX 2
    u_int8_t *clientId;
    
    struct dhcp_pool_common_param_s dhcp_pool_common;
}dhcp_pool_t;

typedef struct dhcp_exclude_ip_range_s {
    struct dhcp_exclude_ip_range_s *next;
    struct sal_in4_addr startip;
    struct sal_in4_addr endip;
} dhcp_exclude_ip_range_t;

/* DHCP server configuration per interface */
typedef struct dhcp_server_interface_s{
    unsigned long nIfIndex;
    char szName[INTERFACE_NAMSIZ + 1];
    /*int up; */
    unsigned int vrfId;
} dhcp_server_interface_t;

extern unsigned int server_packet_boot_request;
extern unsigned int server_packet_boot_reply;
extern unsigned int server_packet_dhcp_discover;
extern unsigned int server_packet_dhcp_request;
extern unsigned int server_packet_dhcp_decline;
extern unsigned int server_packet_dhcp_release;
extern unsigned int server_packet_dhcp_inform;
extern unsigned int server_packet_dhcp_offer;
extern unsigned int server_packet_dhcp_ack;
extern unsigned int server_packet_dhcp_nak;

extern struct hash *g_dhcpserverIntfhash;
extern ctclib_hash_t *g_dhcpserverPoolhash;
extern dhcp_exclude_ip_range_t *g_dhcpserverExcludeIpList;

extern int g_dhcpserver_initialized;
extern int g_dhcpserverSwitch;
#define DHCPSERVER_CHK_FLAG(flag)    ((g_dhcpserverSwitch & (flag)) == (flag))
#define DHCPSERVER_SET_FLAG(flag)    g_dhcpserverSwitch |= (flag)
#define DHCPSERVER_CLR_FLAG(flag)    g_dhcpserverSwitch &= (~(flag))
extern char *config_file;

extern int init_hardware_address(struct interface_info *interface);
extern void signal_init ();
int dhcpserver_module_init(void);
void dhcpserver_module_free(void);
//void dhcpserver_cli_init(struct cli_tree *ctree);

#if 0
void dhcpserver_write_exclude_ip_address(struct cli *cli,
            dhcp_exclude_ip_range_t *pExcludeIpRangeList, 
            int *pnLines);
void dhcpserver_write_array_config_write(struct cli *cli, 
    char* str,
    struct sal_in4_addr * pIn4Addr, 
    u_int8_t cnt,
    int *pnLines);
void dhcpserver_pool_option_code_config_write(struct cli *cli,
    struct dhcp_option_s *pOptions,
    int *pnLines);
void dhcpserver_pool_option_config_write(struct cli *cli, 
    dhcp_pool_t *pPool,
    int *pnLines);
void dhcpserver_pool_config_write(struct cli *cli, int *pnLines);
int dhcpserver_config_write(struct cli *cli);
#endif

int dhcpserver_intf_hash_init(void);
int dhcpserver_start(char *conffile);
struct interface_info *dhcpserver_register_interface(char *name, uint32  ifindex);
void dhcpserver_unregister_interface(char *name);
int dhcpserver_service_onoff(unsigned char bOn);

struct interface_info *dhcpserver_locate_interface_by_id(int id);
extern int dhcpserver_update_primary_address(struct interface_info *pInfo);
int dhcpserver_setup_if_share_network(struct interface_info *tmp);
int dhcpserver_remove_if_share_network(struct interface_info *tmp);

struct interface_info *dhcpserver_locate_interface(char *name);
struct interface_info *dhcpserver_locate_interface_by_id(int id);

dhcp_server_interface_t *dhcpserver_intf_calloc(void);
void dhcpserver_intf_free(dhcp_server_interface_t *pItem);
int dhcpserver_intf_hash_init(void);
//void dhcpserver_intf_hash_finish (struct hash *pHash);
//void dhcpserver_intf_hash_add(struct hash *pHash, dhcp_server_interface_t *pIntf);
//void dhcpserver_intf_hash_del(struct hash *pHash, dhcp_server_interface_t *pIntf);
u_int32_t dhcpserver_intf_hash_make (dhcp_server_interface_t *pool);
bool dhcpserver_intf_hash_cmp (void *a, void *b);

dhcp_exclude_ip_range_t *dhcpserver_exclude_ip_item_calloc(void);
void dhcpserver_exclude_ip_item_free(dhcp_exclude_ip_range_t *pItem);
int dhcpserver_exclude_ip_item_add(struct sal_in4_addr startip, struct sal_in4_addr endip);
int dhcpserver_exclude_ip_item_delete(struct sal_in4_addr startip, struct sal_in4_addr endip);
dhcp_exclude_ip_range_t *dhcpserver_exclude_ip_item_lookup(struct sal_in4_addr startip, struct sal_in4_addr endip);
void dhcpserver_exclude_ip_free_all();
int isc_dhcpserver_ip_forbid(struct iaddr ip_addr);
dhcp_pool_t *isc_dhcpserver_ip_static(struct iaddr ip_addr);
u_int32_t dhcpserver_pool_hash_make (dhcp_pool_t *pItem);
bool dhcpserver_pool_hash_cmp (void *a, void *b);
dhcp_pool_t *dhcpserver_pool_calloc(void);
void dhcpserver_pool_free(dhcp_pool_t *pItem);
int dhcpserver_pool_hash_init(void);

void dhcpserver_pool_hash_add(ctclib_hash_t *pHash, dhcp_pool_t *pItem);
#if 0
void dhcpserver_pool_hash_finish (struct hash *pHash);
void dhcpserver_pool_hash_del(struct hash *pHash, dhcp_pool_t *pItem);
#endif

dhcp_pool_t *dhcpserver_get_static_pool(struct prefix *ip, struct hardware *h);
dhcp_pool_t *dhcpserver_get_dynamic_pool(struct prefix *network);
int dhcpserver_setifup_share_network(struct interface_info *pIntf);
int dhcpserver_remove_if_share_network(struct interface_info *pIntf);
isc_result_t dhcpserver_isc_pool_cfg(dhcp_pool_t *dhcp_pool);
int dhcpserver_cli_ipv4_network_addr_check(struct prefix *p);
int dhcpserver_cli_ipv4_network_check(struct prefix *p);
int dhcpserver_cli_ethernet_mac_addr_get(char* str, u_int8_t *mac_addr);
int dhcpserver_cli_check_hex(char *hexStr);
dhcp_pool_t *dhcpserver_static_pool_confilict_check(dhcp_pool_t *pInPool, 
                                        struct prefix *ip);
dhcp_pool_t *dhcpserver_dynamic_pool_confilict_check(struct prefix *network,
                                        dhcp_pool_t *pInPool);
int dhcpserver_static_pool_config(dhcp_pool_t *pInPool,
                                        struct prefix *ip,
                                        unsigned char *hw_addr,
                                        unsigned char *clientId,
                                        unsigned clientId_len,
                                        unsigned clientId_type);
dhcp_option_t * dhcpserver_pool_option_lookup(dhcp_pool_t *pInPool, unsigned code);
dhcp_option_t *dhcpserver_pool_option_get(unsigned code, 
                            u_int8_t* val,
                            u_int32_t len,
                            enum dhcp_option_Input_type type);
int dhcpserver_pool_option_add(dhcp_pool_t *pInPool, dhcp_option_t *option);
dhcp_option_t *dhcpserver_pool_option_remove(dhcp_pool_t *pInPool, unsigned code);
void dhcpserver_pool_option_free(dhcp_option_t **option);

void dhcpserver_dump_packet_option (struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo);
void dhcpserver_dump_packet (struct packet *tp);
void dhcpserver_dump_dhcp_packet (struct dhcp_packet *tdp, int len);
int dhcpserver_send_ack_msg_to_snooping(struct lease *lease, struct interface_info *ifp);
int dhcpserver_send_nak_msg_to_snooping(struct dhcp_packet *packet, struct iaddr *cip, struct interface_info *ifp);

void dhcpserver_timer_string_get(TIME *t, char* tbuf);
char *dhcpserver_piaddrmaskv4(struct iaddr addr, struct iaddr mask);
void isc_dhcpserver_expire_pool(struct pool *p);
int isc_dhcpserver_shared_network_name_get(char* name, u_int8_t len, struct prefix *network);
int isc_dhcpserver_ipv4_addr_range_get(struct prefix_ipv4 *network,  struct sal_in4_addr *low, struct sal_in4_addr *high);
isc_result_t isc_dhcpserver_options_declaration(dhcp_pool_t *dhcp_pool, struct group *group);
isc_result_t isc_dhcpserver_pool_declaration(dhcp_pool_t *dhcp_pool, struct group *group);
isc_result_t isc_dhcpserver_common_subnet_declaration(dhcp_pool_t *dhcp_pool, struct shared_network *share, struct subnet *subnet);
isc_result_t isc_dhcpserver_subnet_declaration(dhcp_pool_t *dhcp_pool, struct shared_network *share);
isc_result_t isc_dhcpserver_add_dynamic_pool(dhcp_pool_t *dhcp_pool, struct group *group);
isc_result_t isc_dhcpserver_add_static_pool(dhcp_pool_t *dhcp_pool, struct group *group);
isc_result_t isc_dhcpserver_fixip_declaration(dhcp_pool_t *dhcp_pool, struct host_decl *host);
int isc_dhcpserver_ip_lease_config(struct lease *lease, u_int8_t action);
int isc_dhcpserver_all_lease_config(u_int8_t action);
int isc_dhcpserver_pool_lease_config(struct pool *pool, u_int8_t action);
/*int dhcpserver_show_conflict_lease(struct cli *cli, struct lease* lease);
int dhcpserver_cli_show_conflict(struct cli *cli, struct sal_in4_addr *ip, u_int8_t all);
int dhcpserver_cli_show_binding(struct cli *cli, struct sal_in4_addr *ip, dhcp_pool_t *dhcp_pool, u_int8_t all);
int dhcpserver_cli_show_binding_host(struct cli *cli, dhcp_pool_t *dhcp_pool);
int dhcpserver_cli_show_binding_lease(struct cli *cli, struct lease* lease);
int dhcpserver_cli_show_all_static_binding(struct cli *cli);
int dhcpserver_cli_clear_conflict_lease(struct cli* cli, struct sal_in4_addr *ip, u_int8_t all);
int dhcpserver_cli_clear_binding_lease(struct cli* cli, struct sal_in4_addr *ip, dhcp_pool_t *dhcp_pool, u_int8_t all);
*/
void dhcpserver_setup_send_interface(void);
int dhcpserver_send_packet(struct interface_info *interface,
                        struct packet *packet, struct dhcp_packet *raw, size_t len,
                        struct in_addr from, struct sockaddr_in *to, struct hardware *hto);
void isc_dhcpserver_lease_into_pool(struct pool *pool);

isc_result_t isc_dhcpserver_new_address_range(struct iaddr, struct iaddr,
                   struct subnet *, 
                   struct pool *);
//void isc_dhcpserver_remove_lease_content(struct lease *lease);
void isc_dhcpserver_remove_pool_content(struct pool *pool);
void isc_dhcpserver_remove_subnet_content(struct subnet *subnet);
void isc_dhcpserver_remove_dynamic_pool (char* shared_network_name);
void isc_dhcpserver_remove_static_pool (char* host_name);
int isc_dhcpserver_valid_option_check(unsigned code);
int isc_dhcpserver_dynamic_pool_option_add(char* shared_network_name,
                        unsigned code, 
                        u_int8_t* val,
                        u_int32_t len,
                        int terminated);
int isc_dhcpserver_static_pool_option_add(char* host_name,
                        unsigned code, 
                        u_int8_t* val,
                        u_int32_t len,
                        int terminated);
int isc_dhcpserver_option_add(struct group *group, 
                        unsigned code, 
                        u_int8_t* val,
                        u_int32_t len,
                        int terminated);
int isc_dhcpserver_option_remove(struct group *group, u_int16_t code);
int isc_dhcpserver_dynamic_pool_option_remove(char* shared_network_name, unsigned code);
int isc_dhcpserver_static_pool_option_remove(char* host_name, unsigned code);
#endif /* !_DHCPD_FUNC_H_ */

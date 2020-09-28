
#if 0
#ifndef _CENTEC_
/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "dhcpd.h"

#include "sal.h"
#include "pal.h"
#include "pal_log.h"
#include "lib.h"

#include <syslog.h>

#include "cli.h"
#include "log.h"
#include "param_check.h"
#include "dhcpd_func.h"
#include "hash.h"

/****************************************************************************
 *  
* Defines and Macros
* 
*****************************************************************************/
#undef DHCP_OUT_SPLIT
#define DHCP_OUT_SPLIT   \
cli_out(cli, "============================================================\n")

#define DHCP_OUT_ERR_STR    \
cli_out(cli, "%% Can only include [0-9a-zA-Z.-_], and must starting with alphabetic, ending with alphanumeric or digit.\n")
    
#define DHCP_OUT_DHCP_BINDING                \
cli_out (cli, " %-15s %-21s %-28s %-7s\n",    \
    "IP address",                             \
    "Client-ID/",                             \
    "Lease expiration",                     \
    "Type");                                \
cli_out (cli, " %-15s %-21s\n",                \
    "",                                        \
    "Hardware address")                        
    
#define DHCP_OUT_DHCP_CONFILICT                \
cli_out (cli, " %-15s %-32s\n", "IP address", "Detection time")

#define DHCP_SERVER_INIT_CHECK() \
do \
{ \
    if (!g_dhcpserver_initialized) \
    { \
        if (NULL != cli) \
        { \
            cli_out(cli, "%% Internal error: system initialize failed.\n"); \
        } \
        return CLI_ERROR; \
    } \
} while (0)

DEBUG_IMPLEMENT(dhcp, server, "packet", "events", "error", "dump");
DEBUG_CLI_IMPLEMENT(dhcp, server, 
                    "debug dhcp server (packet|events|error|dump|all)",
                    "Debugging functions",
                    "DHCP",
                    "Server",
                    "DHCP message fields",
                    "server events",
                    "Error DHCP message",
                    "Dump message in hex format",
                    "Turn all debug")

/****************************************************************************
 *  
* Global and Declaration
* 
*****************************************************************************/

/****************************************************************************
 *  
* Function
* 
*****************************************************************************/
/*******************************************************************************
 * Name:    dhcpserver_cli_vaild_string_check
 * Purpose: checking string validity
 *     Only support [0-9A-Za-z.-_], starting with alphabetic, ending with alphanumeric or digital.
 * Input:  
 *     str: input string
 *    max_len: the max length of the input string
 * Output: N/A
 * Return: 
 *     success: 0
 *     fail: -1
 * Note: N/A
 ******************************************************************************/
static int dhcpserver_cli_vaild_string_check(const char *str, int max_len)
{
    int nLen = 0;
    char *p = NULL;
    
    if (NULL == str)
    {
        return -1;
    }

    /* check the string length */
    nLen = pal_strlen(str);
    if ((nLen < 1) || (nLen > max_len))
    {
        return -1;
    }

    /*check the first character*/     
    if(!pal_char_isalpha(*str))
    {
        return -1;
    }
    
    /* check the chars */
    p = (char *)(str + 1);
    while ('\0' != *p)
    {
        if ((pal_char_isdigit(*p)) 
            || (pal_char_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }

    /*check the last character*/
    p--;
    if(!(pal_char_isdigit(*p) || pal_char_isalpha(*p)))
    {
        return -1;
    }
    
    return 0;
}

static inline unsigned char str2hexnum(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0; /* foo */
}

static int uint8tohexstr(u_int8_t *data, int len, char *str)
{
    int i = 0;
    int j = 0;
    u_int8_t low = 0;
    u_int8_t high = 0;
    for(i = 0; i<len; i++)
    {
        j = i<<1;
        low = data[i]&0x0F;
        high = data[i]>>4;
        
        if(high <= 9)
        {
            str[j] = '0' + high;
        }
        else
        {
            str[j] = high - 10 + 'a';
        }
        
        if(low <= 9)
        {
            str[j+1] = '0' + low;
        }
        else
        {
            str[j+1] = low - 10 + 'a';
        }
    }
    
    j = (i<<1)+1;
    str[j] = '\0';
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_cli_ipv4_network_addr_check
 * Purpose: check dhcp server address (ipv4 only)
 * Input:  
 *   p:  ipv4 address
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcpserver_cli_ipv4_network_addr_check(struct prefix *p)
{
  struct pal_in4_addr addr;
  struct pal_in4_addr mask;
  u_int32_t addrval;

  if(NULL == p ||IPV4_MAX_BITLEN == p->prefixlen  || 0 == p->prefixlen)
    return -1;

  sal_masklen2ip (p->prefixlen, &mask);
  addrval = sal_ntohl(p->u.prefix4.s_addr);
  
  if (IN_BADCLASS(addrval) 
      || IN_MULTICAST(addrval) 
      || IN_LOOPBACK(addrval)
      || IPV4_NET0(addrval))
      return -1;
  
  /* host address is rejected */
  addr.s_addr = p->u.prefix4.s_addr & mask.s_addr;
  if (addr.s_addr == p->u.prefix4.s_addr)
    return -1;

  /* broadcast address is rejected */
  addr.s_addr = p->u.prefix4.s_addr | (~mask.s_addr);
  if (addr.s_addr == p->u.prefix4.s_addr || addr.s_addr == 0)
    return -1;
  
  return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_cli_ipv4_network_check
 * Purpose: check network address valid or not (ipv4 only)
 * Input:  
 *   p:  network address
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcpserver_cli_ipv4_network_check(struct prefix *p)
{
  u_int32_t addrval;
  if (NULL == p)
    return -1;
  
  addrval = sal_ntoh32(p->u.prefix4.s_addr);
  
  if (IN_BADCLASS(addrval) 
      || IN_MULTICAST(addrval) 
      || IN_LOOPBACK(addrval)
      || IPV4_NET0(addrval))
      return -1;
  
  return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_cli_ethernet_mac_addr_get
 * Purpose: get dhcp ethernet mac address
 * Input:  
 *   str:  input string
 *   mac_addr:  mac address return buffer for output
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcpserver_cli_ethernet_mac_addr_get(char* str, u_int8_t *mac_addr)
{    
    if(str == NULL || mac_addr == NULL || 0 != check_macaddr(str))
    {
        return -1;
    }
    
    if (pal_sscanf (str, "%2hhx%2hhx.%2hhx%2hhx.%2hhx%2hhx",
          (u_int8_t *)&mac_addr[1],
          (u_int8_t *)&mac_addr[2],
          (u_int8_t *)&mac_addr[3],
          (u_int8_t *)&mac_addr[4],
          (u_int8_t *)&mac_addr[5],
          (u_int8_t *)&mac_addr[6]) != 6)
    {
        return -1;
    }
    
    mac_addr[0] = 1;//hardware for ethernet type
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_cli_ethernet_mac_addr_get
 * Purpose: get dhcp ethernet mac address
 * Input:  
 *   str:  input string
 *   mac_addr:  mac address return buffer for output
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcpserver_cli_hex_get(char* str, u_int8_t *buf)
{    
    int nLen = 0;
    int i = 0; 
    int j = 0;
    
    nLen = pal_strlen(str);
    
    if (pal_strspn(str, "0123456789AaBbCcDdEeFf") != nLen)
    {
        return -1;
    }
    
    for(i = 0; i<nLen; i+=2, j++)
    {
        buf[j] = str2hexnum(str[i]);
        buf[j] = buf[j]<<4;
        buf[j] |= str2hexnum(str[i+1]);
    }
    
    return 0;
}

int dhcpserver_cli_check_hex(char *hexStr)
{
    int nLen = 0;
    
    if (NULL == hexStr)
    {
        return -1;
    }
    
    nLen = pal_strlen(hexStr);
    if (nLen != (nLen >> 1 << 1) || sal_strspn(hexStr, "0123456789AaBbCcDdEeFf") != nLen)
    {
        return -1;
    }
    
    return 0;
}

int dhcpserver_cli_show_conflict_lease(struct cli *cli, struct lease* lease)
{
    char tbuf [64];
    char addr[16];
    sprintf(addr, "%d.%d.%d.%d",
        lease->ip_addr.iabuf[0],
        lease->ip_addr.iabuf[1],
        lease->ip_addr.iabuf[2],
        lease->ip_addr.iabuf[3]);
    if(lease && lease->binding_state == FTS_ABANDONED)
    {
        sal_memset(tbuf, 0, 64);
        dhcpserver_timer_string_get(&lease->ends, tbuf);
        cli_out (cli, " %-15s %-32s\n", addr, tbuf);
    }
    return 0;
}

int dhcpserver_cli_show_conflict(struct cli *cli, struct pal_in4_addr *ip, u_int8_t all)
{
    struct iaddr addr;
    struct lease *lease = (struct lease *)0;
    struct shared_network *s;
    struct pool *p;
    
    if(NULL != ip)
    {
        sal_memset(&addr, 0, sizeof(struct iaddr));    
        addr.len = 4;
        sal_memcpy(addr.iabuf, ip, 4);
        
        if (find_lease_by_ip_addr (&lease, addr, MDL)
            && lease->binding_state == FTS_ABANDONED)
        {
            DHCP_OUT_DHCP_CONFILICT;
            dhcpserver_cli_show_conflict_lease(cli, lease);
            lease_dereference(&lease, MDL);
        }
        else
        {
            cli_out (cli, "%% The specified IP does not exist in conflict table\n");
        }
    }
    
    if(all)
    {
        DHCP_OUT_DHCP_CONFILICT;
        for (s = shared_networks; s; s = s ->next)
        {
            for (p = s ->pools; p; p = p ->next)
            {
                for(lease = p->abandoned;lease; lease = lease->next)
                {
                    if(lease ->ends != MAX_TIME)
                    {
                        dhcpserver_cli_show_conflict_lease(cli, lease);
                    }
                }
            }
        }
    }
    
    return 0;
}

int dhcpserver_cli_show_binding_lease(struct cli *cli, struct lease* lease)
{
    char *str = NULL;
    int total = 0;
    int len = 0;
    char tbuf [64];
    #define UID_BUF_LEN 21
    char buf[UID_BUF_LEN+1];
    char addr[16];

    if(lease && FTS_ACTIVE == lease->binding_state)
    {
        sal_memset(tbuf, 0, 64);
        sal_memset(buf, 0, UID_BUF_LEN+1);

        dhcpserver_timer_string_get(&lease->ends, tbuf);
                
        if (lease ->hardware_addr.hlen)
        {
            sal_sprintf (buf, "%s", print_hw_addr (lease ->hardware_addr.hbuf [0],
                            lease ->hardware_addr.hlen - 1,
                            &lease ->hardware_addr.hbuf [1]));
        }
        else if(lease ->uid_len)
        {
            total = (lease ->uid_len<<1);
            str = dmalloc (total + 1, MDL);
            if(NULL == str)
            {
                cli_out (cli, "%% No memory for dns server option\n");
                return -1;
            }
            uint8tohexstr(lease ->uid, lease ->uid_len, str);
        }

        if(total >= UID_BUF_LEN)
        {
            sal_strncpy(buf, str, UID_BUF_LEN);
            buf[UID_BUF_LEN] = '\0';
            total -= UID_BUF_LEN;
            len = UID_BUF_LEN;
        }
        else if(0 != total)
        {
            sal_strncpy(buf, str, total);
            buf[total] = '\0';
            total = 0;
            len = total;
        }
        
        sprintf(addr, "%d.%d.%d.%d",
            lease->ip_addr.iabuf[0],
            lease->ip_addr.iabuf[1],
            lease->ip_addr.iabuf[2],
            lease->ip_addr.iabuf[3]);
        
        cli_out (cli, " %-15s %-21s %-28s %-7s\n",
                addr,
                buf,
                tbuf,
                (lease ->flags & STATIC_LEASE)? "Static": "Dynamic");
        
        while(total > 0)
        {
            if(total > UID_BUF_LEN)
            {
                sal_strncpy(buf, str+len, UID_BUF_LEN);
                buf[UID_BUF_LEN] = '\0';
                total -= UID_BUF_LEN;
                len += UID_BUF_LEN;
            }
            else
            {
                sal_strncpy(buf, str+len, total);
                buf[total] = '\0';
                total = 0;
                len = total;
            }
            cli_out (cli, " %-15s %-21s\n",
                "",
                buf);
            
        }
        
        if(str)
        {
            dfree (str, MDL);
            str = NULL;
        }
        return 0;
    }
    
    return -1;
}

int dhcpserver_cli_show_binding_host(struct cli *cli, dhcp_pool_t *dhcp_pool)
{
    char *str = NULL;
    int total = 0;
    int len = 0;
    #define UID_BUF_LEN 21
    char buf[UID_BUF_LEN+1];
    char ipAddr[16];
    
    if(staticPool == dhcp_pool->pool_type)
    {
        sal_memset(buf, 0, UID_BUF_LEN+1);

        if (dhcp_pool->hw_addr)
        {
            sal_sprintf (buf, "%s", print_hw_addr (dhcp_pool->hw_addr[0],
                            dhcp_pool->hw_len - 1,
                            &dhcp_pool->hw_addr[1]));
        }
        else if(dhcp_pool ->clientId)
        {
            total = (dhcp_pool->clientId_len<<1);
            str = dmalloc (total + 1, MDL);
            if(NULL == str)
            {
                cli_out (cli, "%% No memory for allocate\n");
                return -1;
            }
            uint8tohexstr(dhcp_pool->clientId, dhcp_pool->clientId_len, str);
        }

        if(total >= UID_BUF_LEN)
        {
            sal_strncpy(buf, str, UID_BUF_LEN);
            buf[UID_BUF_LEN] = '\0';
            total -= UID_BUF_LEN;
            len = UID_BUF_LEN;
        }
        else if(0 != total)
        {
            sal_strncpy(buf, str, total);
            buf[total] = '\0';
            total = 0;
            len = total;
        }
        
        sal_memset(ipAddr, 0, 16);
        sal_inet_ntop(AF_INET, &(dhcp_pool->fixIp.u.prefix4), ipAddr, 16);
        
        cli_out (cli, " %-15s %-21s %-28s %-7s\n",
                ipAddr,
                buf,
                "Infinite",
                "Static");
        
        while(total > 0)
        {
            if(total > UID_BUF_LEN)
            {
                sal_strncpy(buf, str+len, UID_BUF_LEN);
                buf[UID_BUF_LEN] = '\0';
                total -= UID_BUF_LEN;
                len += UID_BUF_LEN;
            }
            else
            {
                sal_strncpy(buf, str+len, total);
                buf[total] = '\0';
                total = 0;
                len = total;
            }
            cli_out (cli, " %-15s %-21s\n",
                "",
                buf);
            
        }
        
        if(str)
        {
            dfree (str, MDL);
            str = NULL;
        }
        return 0;
    }
    
    return -1;
}

int dhcpserver_cli_show_all_static_binding(struct cli *cli)
{
    dhcp_pool_t *pTmp = NULL;
    struct hash *hash = g_dhcpserverPoolhash;
    struct hash_backet *hb = NULL;
    int i;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = NULL;
            pTmp = (dhcp_pool_t *)hb->data;
            if(pTmp && staticPool== pTmp->pool_type)
            {
                dhcpserver_cli_show_binding_host(cli, pTmp);
            }
        }
    }
    
    return 0;
}

int dhcpserver_cli_show_binding(struct cli *cli, struct pal_in4_addr *ip, dhcp_pool_t *dhcp_pool, u_int8_t all)
{
    struct iaddr addr;
    struct lease *lease = (struct lease *)0;
    struct shared_network *s = (struct shared_network *)0;
    struct pool *p = (struct pool *)0;
    dhcp_pool_t *pDhcpPool = (dhcp_pool_t *)0;
    char pool_name[POOL_NAME_SIZE+1];
    
    if(NULL != ip)
    {
        addr.len = 4;
        sal_memcpy(addr.iabuf, ip, 4);
        pDhcpPool = isc_dhcpserver_ip_static(addr);
        if(pDhcpPool)
        {
            DHCP_OUT_DHCP_BINDING;
            dhcpserver_cli_show_binding_host(cli, pDhcpPool);
        }
        else
        {
            find_lease_by_ip_addr (&lease, addr, MDL);
            if(lease && FTS_ACTIVE == lease->binding_state)
            { 
                DHCP_OUT_DHCP_BINDING;
                dhcpserver_cli_show_binding_lease(cli, lease);
                lease_dereference (&lease, MDL);
            }
            else
            {
                cli_out (cli, "%% The specified IP does not exist in the binding table\n");
            }
        }
        
        return 0;
    }
    
    if(all)
    {
        DHCP_OUT_DHCP_BINDING;
        for (s = shared_networks; s; s = s ->next)
        {
            for (p = s ->pools; p; p = p ->next)
            {
                for(lease = p->active;lease; lease = lease->next)
                {
                    dhcpserver_cli_show_binding_lease(cli, lease);
                }
            }
        }
        
        dhcpserver_cli_show_all_static_binding(cli);
        return 0;
    }

    if(NULL != dhcp_pool)
    {
        DHCP_OUT_DHCP_BINDING;
        if(dynPool == dhcp_pool->pool_type)
        {
            sal_memset(pool_name, 0, POOL_NAME_SIZE+1);
            isc_dhcpserver_shared_network_name_get(pool_name, POOL_NAME_SIZE+1, &dhcp_pool->network);
            
            for (s = shared_networks; s; s = s ->next) 
            {
                if (s->name && !strcmp(s->name, pool_name))
                {
                    for (p = s ->pools; p; p = p ->next) 
                    {
                        for(lease = p->active;lease; lease = lease->next)
                        {
                            dhcpserver_cli_show_binding_lease(cli, lease);
                        }
                    }
                }
            }
        }
        
        if(staticPool == dhcp_pool->pool_type)
        {
            dhcpserver_cli_show_binding_host(cli, dhcp_pool);
        }
    }
    
    return 0;
}

int dhcpserver_cli_clear_conflict_lease(struct cli *cli, struct pal_in4_addr *ip, u_int8_t all)
{
    struct iaddr addr;
    struct lease *lease = (struct lease *)0;
    struct shared_network *s = (struct shared_network *)0;
    struct pool *p = (struct pool *)0;
    
    if(NULL != ip)
    {
        addr.len = 4;
        sal_memcpy(addr.iabuf, ip, 4);
        
        find_lease_by_ip_addr (&lease, addr, MDL);
/* modified by cuixl for bug 24921, 2013-12-03 */
/* for clear conflict address, we need specify LEASE_ACTION_RELEASE_ABANDON
otherwise, binding ip would be released*/
/* modification start */        
/*         if(NULL != lease && FTS_ACTIVE == lease->binding_state)
        { 
            isc_dhcpserver_ip_lease_config(lease, LEASE_ACTION_RELEASE_ACTIVE);
*/        
        if(NULL != lease && FTS_ABANDONED == lease->binding_state)
        { 
            isc_dhcpserver_ip_lease_config(lease, LEASE_ACTION_RELEASE_ABANDON);            
/* modification end */
            
            lease_dereference (&lease, MDL);
        }
        else
        {
            cli_out (cli, "%% The specified IP does not exist in the conflict table\n");
            return -1;
        }
    }
    
    if(all)
    {
        for (s = shared_networks; s; s = s ->next)
        {
            for (p = s ->pools; p; p = p ->next)
            {
/* modified by cuixl for bug 24921, 2013-12-03 */
/* for clear conflict address, we need specify LEASE_ACTION_RELEASE_ABANDON
otherwise, binding ip would be released*/
/* modification start */           
/*                isc_dhcpserver_pool_lease_config(p, LEASE_ACTION_RELEASE_ACTIVE); */
                isc_dhcpserver_pool_lease_config(p, LEASE_ACTION_RELEASE_ABANDON);                
/* modification end */
                
            }
        }
    }
    
    return 0;
}

int dhcpserver_cli_clear_binding_lease(struct cli* cli, struct pal_in4_addr *ip, dhcp_pool_t *dhcp_pool, u_int8_t all)
{
    struct iaddr addr;
    struct lease *lease = (struct lease *)0;
    struct shared_network *s = (struct shared_network *)0;
    struct pool *p = (struct pool  *)0;
    char pool_name[POOL_NAME_SIZE+1];
    
    if(NULL != ip)
    {
        addr.len = 4;
        sal_memcpy(addr.iabuf, ip, 4);
        
        find_lease_by_ip_addr (&lease, addr, MDL);
        if(NULL != lease && FTS_ACTIVE == lease->binding_state)
        { 
            isc_dhcpserver_ip_lease_config(lease, LEASE_ACTION_RELEASE_ACTIVE);
            lease_dereference (&lease, MDL);
        }
        else
        {
            cli_out (cli, "%% The specified IP does not exist in binding table\n");
            return -1;
        }
    }
    
    if(all)
    {
        for (s = shared_networks; s; s = s ->next)
        {
            for (p = s ->pools; p; p = p ->next)
            {
                for(lease = p->active;lease; lease = lease->next)
                {
                    isc_dhcpserver_pool_lease_config(p, LEASE_ACTION_RELEASE_ACTIVE);
                }
            }
        }
    }

    if(dhcp_pool)
    {
        if(dynPool == dhcp_pool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(pool_name, POOL_NAME_SIZE+1, &dhcp_pool->network);
            
            for (s = shared_networks; s; s = s ->next) 
            {
                if (s->name && !strcmp(s->name, pool_name))
                {
                    for (p = s ->pools; p; p = p ->next) 
                    {
                        isc_dhcpserver_pool_lease_config(p, LEASE_ACTION_RELEASE_ACTIVE);
                    }
                }
            }
        }
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcp_service_switch
 * Purpose: command for control dhcp services
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_service_switch,
     dhcp_service_switch_cmd,
     "service dhcp (enable|disable)",
     "Modify use of network based services",
     "DHCP service",
     "Enable this service",
     "Disable this service"
    )
{
    if (!sal_strcasecmp(argv[0], "enable"))
    {
        if (DHCPSERVER_CHK_FLAG(DHCP_SERVICE_FLAG))
        {
            return CLI_SUCCESS;
        }
        
        DHCPSERVER_SET_FLAG(DHCP_SERVICE_FLAG);
    }
    else
    {
        if (!DHCPSERVER_CHK_FLAG(DHCP_SERVICE_FLAG))
        {
            return CLI_SUCCESS;
        }
        
        DHCPSERVER_CLR_FLAG(DHCP_SERVICE_FLAG);
    }

    return dhcpserver_service_onoff(
            DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:    dhcpserver_service_switch
 * Purpose: command for control dhcp server feature
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcpserver_service_switch,
     dhcpserver_service_switch_cmd,
     "dhcp server",
     "Specify DHCP parameter",
     "Enable DHCP server feature"
    )
{
    if (DHCPSERVER_CHK_FLAG(DHCP_SERVER_SERVICE_FLAG))
    {
        return CLI_SUCCESS;
    }

    DHCPSERVER_SET_FLAG(DHCP_SERVER_SERVICE_FLAG);
    return dhcpserver_service_onoff(
            DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:    no_dhcpserver_service_switch
 * Purpose: command for control dhcp server feature
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcpserver_service_switch,
     no_dhcpserver_service_switch_cmd,
     "no dhcp server",
     CLI_NO_STR,
     "Specify DHCP parameter",
     "Enable DHCP server feature"
    )
{
    if (!DHCPSERVER_CHK_FLAG(DHCP_SERVER_SERVICE_FLAG))
    {
        return CLI_SUCCESS;
    }

    DHCPSERVER_CLR_FLAG(DHCP_SERVER_SERVICE_FLAG);
    return dhcpserver_service_onoff(
            DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)));
}

/*******************************************************************************
 * Name:   dhcp_ping_timeout
 * Purpose: Specifies how long to wait for a ping reply in seconds
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_ping_packet,
     dhcp_ping_packet_cmd,
     "dhcp ping packets <0-10>",
     "Specify DHCP parameter",
     "Define DHCP server ping parameters",
     "Define number of ping packets",
     "Number of ping packets (0 to disable pings)"
     )
{
    int cnt = 0;
    CLI_GET_INTEGER_RANGE ("Number of ping packets (0 to disable pings)", cnt , argv[0], 0, 10);

    server_ping_packets = pal_atoi(argv[0]);
    if(0 == server_ping_timeout)
        server_ping_timeout = DEFAULT_PING_TIMEOUT;
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_ping_packet
 * Purpose: restore the default value of ping packet
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_ping_packet,
     no_dhcp_ping_packet_cmd,
     "no dhcp ping packets",
     CLI_NO_STR,
     "Specify DHCP parameter",
     "Define DHCP server ping parameters",
     "Define number of ping packets"
     )
{
    server_ping_packets = DEFAULT_PING_PACKET;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_ping_timeout
 * Purpose: Specifies how long to wait for a ping reply in seconds
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_ping_timeout,
     dhcp_ping_timeout_cmd,
     "dhcp ping timeout <1-10>",
     "Specify DHCP parameter",
     "Define DHCP server ping parameters",
     "Define ping timeout",
     "Specifies how long to wait for a ping reply in seconds"
     )
{
    int cnt = 0;
    CLI_GET_INTEGER_RANGE ("Specifies how long to wait for a ping reply in seconds", cnt , argv[0], 1, 10);
    
    server_ping_timeout = pal_atoi(argv[0]);
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_ping_timeout
 * Purpose: restore the default value of ping timeout
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_ping_timeout,
     no_dhcp_ping_timeout_cmd,
     "no dhcp ping timeout",
     CLI_NO_STR,
     "Specify DHCP parameter",
     "Define DHCP server ping parameters",
     "Define ping timeout"
     )
{

    server_ping_timeout = DEFAULT_PING_TIMEOUT;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_excluded_addr
 * Purpose: To configure excluded address for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_excluded_addr,
     dhcp_excluded_addr_cmd,
     "dhcp excluded-address A.B.C.D (A.B.C.D|)",
     "Specify DHCP parameter",
     "Define addresses DHCP server can not assign",
     "Low IP address",
     "High IP address"
     )
{
    struct pal_in4_addr startip;
    struct pal_in4_addr endip;
    u_int32_t low, high;
    
    if(server_exclude_ip_count >= MAX_EXCLUDE_IP_CNT)
    {
        cli_out (cli, "%% The number of DHCP excluded address list over %u\n", MAX_POOL_CNT);
        return CLI_ERROR;
    }
        
    sal_memset(&startip, 0, sizeof (struct pal_in4_addr));
    sal_memset(&endip, 0, sizeof (struct pal_in4_addr));
    
    CLI_GET_IPV4_ADDRESS("excluded low IP address", startip, argv[0]);
    
    if(argc == 2)
    {
        CLI_GET_IPV4_ADDRESS("excluded high IP address", endip, argv[1]);
    }
    else if (argc == 1)
    {
        sal_memcpy(&endip, &startip, sizeof (struct  pal_in4_addr));
    }
    else
    {
        cli_out (cli, "%% Wrong arguments\n");
        return CLI_ERROR;
    }
    
    low = sal_ntohl(startip.s_addr);
    high = sal_ntohl(endip.s_addr);

    /* delete "=" by liuyang*/
    if((low > high) && (argc == 2))
    {
        cli_out (cli, "%% Low IP address is greater than high IP address\n");
        return CLI_ERROR;
    }
    
    if(dhcpserver_exclude_ip_item_lookup(startip, endip))
    {
        return CLI_SUCCESS;
    }

    if(dhcpserver_exclude_ip_item_add(startip, endip))
    {
        cli_out(cli, "%% WARNING: Out of memory\n");
        return CLI_ERROR;
    }
    
    server_exclude_ip_count++;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_excluded_addr
 * Purpose: To remove the configuration of excluded address for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_excluded_addr,
     no_dhcp_excluded_addr_cmd,
     "no dhcp excluded-address A.B.C.D (A.B.C.D|)",
     CLI_NO_STR,
     "Specify DHCP parameter",
     "Define addresses DHCP server can not assign",
     "Low IP address",
     "High IP address"
     )
{
    struct pal_in4_addr startip;
    struct pal_in4_addr endip;
    u_int32_t low, high;

    sal_memset(&startip, 0, sizeof (struct pal_in4_addr));
    sal_memset(&endip, 0, sizeof (struct pal_in4_addr));

    CLI_GET_IPV4_ADDRESS("excluded low IP address", startip, argv[0]);

    if(argc == 2)
    {
        CLI_GET_IPV4_ADDRESS("excluded high IP address", endip, argv[1]);
    }
    else if (argc == 1)
    {
        sal_memcpy(&endip, &startip, sizeof (struct  pal_in4_addr));
    }
    else
    {
        cli_out (cli, "%% Wrong arguments\n");
        return CLI_ERROR;
    }
    
    low = sal_ntohl(startip.s_addr);
    high = sal_ntohl(endip.s_addr);
     /* delete "=" by liuyang*/
    if((low > high) && (argc == 2))
    {
        cli_out (cli, "%% Low IP address is greater than high IP address\n");
        return CLI_ERROR;
    }
    
    if(dhcpserver_exclude_ip_item_delete(startip, endip))
    {
        cli_out(cli, "%% The specified excluded-ip node/range does not exist\n");
        return CLI_ERROR;
    }
    else
    {
        server_exclude_ip_count--;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_pool
 * Purpose: To configure a DHCP address pool on a DHCP Server, 
 *    specify the dhcp address pool name
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_pool,
     dhcp_pool_cmd,
     "dhcp pool WORD",
     "Specify DHCP parameter",
     "Pool",
     "Pool name"
     )
{
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t stPool;
    if(server_pool_count >= MAX_POOL_CNT)
    {
        cli_out (cli, "%% The number of DHCP pool over %u\n", MAX_POOL_CNT);
        return CLI_ERROR;
    }
    
    if(sal_strlen(argv[0]) > POOL_NAME_SIZE)
    {
        cli_out (cli, "%% Configuration pool name should not be more than %d characters\n", POOL_NAME_SIZE);       
        return CLI_ERROR;
    }
    
    if(dhcpserver_cli_vaild_string_check(argv[0], POOL_NAME_SIZE))
    {
        DHCP_OUT_ERR_STR;
        return CLI_ERROR;
    }
    
    sal_memset(&stPool, 0, sizeof(dhcp_pool_t));
    sal_strcpy(stPool.pool_name, argv[0]);
    
    pPool = hash_lookup(g_dhcpserverPoolhash, &stPool);
    if (NULL == pPool)
    {
        pPool = dhcpserver_pool_calloc();
        if (NULL == pPool)
        {
            cli_out(cli, "%% WARNING: Out of memory\n");
            return CLI_ERROR;
        }
        sal_strcpy(pPool->pool_name, argv[0]);
        dhcpserver_pool_hash_add(g_dhcpserverPoolhash, pPool);
        server_pool_count++;
    }

    cli->index = pPool;
    cli->mode = DHCPS_MODE;    
            
        
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_pool
 * Purpose: To remove a DHCP address pool  
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_pool,
     no_dhcp_pool_cmd,
     "no dhcp pool WORD",
     CLI_NO_STR,
     "Specify DHCP parameter",
     "Pool",
     "Pool name"
     )
{
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t stPool;
    char name[POOL_NAME_SIZE+1];
    
    if(sal_strlen(argv [0]) > POOL_NAME_SIZE)
    {
        cli_out (cli, "%% Configuration pool name should not be more than %d characters\n", POOL_NAME_SIZE);       
        return CLI_ERROR;
    }
    
    if(dhcpserver_cli_vaild_string_check(argv[0], POOL_NAME_SIZE))
    {
        DHCP_OUT_ERR_STR;
        return CLI_ERROR;
    }

    sal_memset(name, 0, POOL_NAME_SIZE+1);
    sal_strcpy(stPool.pool_name, argv[0]);
    pPool = hash_lookup(g_dhcpserverPoolhash, &stPool);
    
    if (NULL != pPool)
    {
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_remove_static_pool(pPool->pool_name);
        }
        else if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_remove_dynamic_pool(name);
        }
        
        dhcpserver_pool_hash_del(g_dhcpserverPoolhash, pPool);
        dhcpserver_pool_free(pPool);
        pPool = NULL;
    }
    else
    {
        cli_out (cli, "%% DHCP pool does not exist\n");
        return CLI_ERROR;
    }
    
    server_pool_count--;
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_static_binding
 * Purpose: To configure an address binding mapping
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_static_binding,
     dhcp_static_binding_cmd,
     "static-bind ip-address A.B.C.D/M (mac-address HHHH.HHHH.HHHH | client-identifier (ascii WORD | hex Hex-string))",
     "Specify static binding information",
     "IP address and mask",
     "Specify an IP address prefix",
     "Specify mac address configuration information",
     "static binding MAC address in HHHH.HHHH.HHHH format",
     "Specify client-identifier configuration information",
     "Client-ID as ascii string",
     "Client-ID string",
     "Class-ID in hex",
     "Class-ID in hex"
     )
{
    struct prefix new_prefix;
    s_int32_t ret = 0;
    dhcp_pool_t *pPool = cli->index;
    dhcp_pool_t *pTmp = (dhcp_pool_t *)0;
    int len = 0;
    int change = 0;
    unsigned char hardware[ETHER_ADDR_LEN+1];
    unsigned char cid[DHCP_OPTION_MAX_LEN+1];
    u_int8_t clientId_type;
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }

    if(dynPool == pPool->pool_type)
    {
        cli_out (cli, "%% This command may not be used with dynamic pools\n");    
        return CLI_ERROR;
    }

    /* Clean prefix and mask*/
    sal_memset (&new_prefix, 0, sizeof (struct prefix));
    
        
    /* Get the ip address and mask */
    ret = str2prefix (argv[0], &new_prefix);
    
    if (0 == ret || dhcpserver_cli_ipv4_network_addr_check(&new_prefix))
    {
      cli_out (cli, "%% Bad unicast host address\n");
      return CLI_ERROR;
    }
        
    pTmp = dhcpserver_static_pool_confilict_check(pPool, &new_prefix);
    
    if(NULL != pTmp)
    {
        cli_out (cli, "%% The binding configuration conflict with pool %s\n",
                 pTmp->pool_name);
        return CLI_ERROR;
    }
    
    if(sal_memcmp(&new_prefix, &pPool->fixIp, sizeof(struct prefix)))
    {
        change = 1;
    }
        
    if(!sal_strcmp(argv[1], "mac-address"))
    {
        sal_memset (&hardware, 0, ETHER_ADDR_LEN+1);
        if (dhcpserver_cli_ethernet_mac_addr_get(argv[2], hardware))
        {
            cli_out (cli, "%% Unable to translate mac address %s\n", argv[2]);
            return CLI_ERROR;
        }
        
        clientId_type = DHCP_CLENTID_NONE;
        if(NULL == pPool->hw_addr 
            || sal_memcmp(hardware, pPool->hw_addr, ETHER_ADDR_LEN+1))
        {
            change = 1;
        }
    }
    else
    {
        if(!sal_strcmp(argv[2], "hex"))
        {
            len =sal_strlen(argv[3]);
            if(DHCP_OPTION_MAX_LEN*2 < len)
            {
                cli_out(cli, "%% Hex string should not exceed %d bytes\n",
                      DHCP_OPTION_MAX_LEN*2);
                return CLI_ERROR;
            }
            
            if(0 != dhcpserver_cli_check_hex(argv[3]))
            {
                cli_out (cli, "%% Invalid Hex string\n");
                return CLI_ERROR;
            }
            
            clientId_type = DHCP_CLENTID_HEX;    
            
            sal_memset(cid, 0, DHCP_OPTION_MAX_LEN+1);            
            dhcpserver_cli_hex_get(argv[3], cid);
            len = len>>1;
            if(NULL == pPool->clientId 
                || len != pPool->clientId_len
                || sal_memcmp(cid, pPool->clientId, len))
            {
                change = 1;
            }
        }
        else
        {
            len =sal_strlen(argv[3]);
            if (len > DHCP_OPTION_MAX_LEN)
            {
                cli_out(cli, "%% ASCII string should not exceed %d bytes\n",
                      DHCP_OPTION_MAX_LEN);
                return CLI_ERROR;
            }
            
            clientId_type = DHCP_CLENTID_ASCII;
            if(NULL == pPool->clientId 
                || len != pPool->clientId_len
                || sal_memcmp(argv[3], pPool->clientId, len))
            {
                change = 1;
            }
        }
    }
    
    if(NULL != pTmp)
    {
        cli_out (cli, "%% The binding configuration conflict with pool %s\n",
                 pTmp->pool_name);
        return CLI_ERROR;
    }
    
    if(0 == change)
        return CLI_SUCCESS;
    
    if(staticPool == pPool->pool_type)
         isc_dhcpserver_remove_static_pool(pPool->pool_name);


    switch(clientId_type)
    {
    case DHCP_CLENTID_NONE:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            hardware,
                            NULL, 0, DHCP_CLENTID_NONE))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    case DHCP_CLENTID_HEX:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            NULL,
                            cid, len, clientId_type))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    case DHCP_CLENTID_ASCII:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            NULL,
                            (u_char *)argv[3], len, clientId_type))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    default:
        cli_out (cli, "%% Invalid arguments, unknown client id type: %d\n",
                 clientId_type);
        return CLI_ERROR;
    }
    
    if(isc_dhcpserver_add_static_pool(pPool, root_group))
    {
        cli_out (cli, "%% Adding static binding fail\n");
        return CLI_ERROR;
    }
    
    /*static pool do not allow lease time, so clear it*/
    pPool->dhcp_pool_common.lease_time = 0;
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_static_binding_mask
 * Purpose: To configure an address binding mapping
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_static_binding_mask,
     dhcp_static_binding_mask_cmd,
     "static-bind ip-address A.B.C.D A.B.C.D (mac-address HHHH.HHHH.HHHH | client-identifier (ascii WORD | hex Hex-string))",
     "Specify static binding information",
     "Specify a specific IP address",
     "IP address",
     "Specify an IP address prefix",
     "Specify mac address configuration information",
     "static binding MAC address in HHHH.HHHH.HHHH format",
     "Specify client-identifier configuration information",
     "Client-ID as ascii string",
     "Client-ID string",
     "Class-ID in hex",
     "Class-ID in hex"
     )
{
    struct prefix new_prefix;
    struct pal_in4_addr mask;
    
    dhcp_pool_t *pPool = cli->index;
    dhcp_pool_t *pTmp = (dhcp_pool_t *)0;
    int len = 0;
    int change = 0;
    unsigned char hardware[ETHER_ADDR_LEN+1];
    unsigned char cid[DHCP_OPTION_MAX_LEN+1];
    u_int8_t clientId_type;
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }

    if(dynPool == pPool->pool_type)
    {
        cli_out (cli, "%% This command may not be used with dynamic pools\n");    
        return CLI_ERROR;
    }

    /* Clean prefix */
    sal_memset (&new_prefix, 0, sizeof (struct prefix));
    sal_memset (&mask, 0, sizeof (struct pal_in4_addr));
    
    /* Get mask. */
    CLI_GET_IPV4_ADDRESS("wildcard mask", mask, argv[1]);
    
    /* Get the ip address */
    DHCP_CLI_GET_IPV4_PREFIX("network address", new_prefix, argv[0]);
    new_prefix.prefixlen = ip_masklen(mask);
    
    if (dhcpserver_cli_ipv4_network_addr_check(&new_prefix))
    {
      cli_out (cli, "%% Bad unicast host address\n");
      return CLI_ERROR;
    }
        
    pTmp = dhcpserver_static_pool_confilict_check(pPool, &new_prefix);
    
    if(NULL != pTmp)
    {
        cli_out (cli, "%% The binding configuration conflict with pool %s\n",
                 pTmp->pool_name);
        return CLI_ERROR;
    }
    
    if(sal_memcmp(&new_prefix, &pPool->fixIp, sizeof(struct prefix)))
    {
        change = 1;
    }
        
    if(!sal_strcmp(argv[2], "mac-address"))
    {
        sal_memset (&hardware, 0, ETHER_ADDR_LEN+1);
        if (dhcpserver_cli_ethernet_mac_addr_get(argv[3], hardware))
        {
            cli_out (cli, "%% Unable to translate mac address %s\n", argv[3]);
            return CLI_ERROR;
        }
        
        clientId_type = DHCP_CLENTID_NONE;
        if(NULL == pPool->hw_addr 
            || sal_memcmp(hardware, pPool->hw_addr, ETHER_ADDR_LEN+1))
        {
            change = 1;
        }
    }
    else
    {
        if(!sal_strcmp(argv[3], "hex"))
        {
            len =sal_strlen(argv[4]);
            if(DHCP_OPTION_MAX_LEN*2 < len)
            {
                cli_out(cli, "%% Hex string should not exceed %d bytes\n",
                      DHCP_OPTION_MAX_LEN*2);
                return CLI_ERROR;
            }
            
            if(0 != dhcpserver_cli_check_hex(argv[4]))
            {
                cli_out (cli, "%% Invalid Hex string\n");
                return CLI_ERROR;
            }
            
            clientId_type = DHCP_CLENTID_HEX;    
            
            sal_memset(cid, 0, DHCP_OPTION_MAX_LEN+1);            
            dhcpserver_cli_hex_get(argv[4], cid);
            len = len>>1;
            if(NULL == pPool->clientId 
                || len != pPool->clientId_len
                || sal_memcmp(cid, pPool->clientId, len))
            {
                change = 1;
            }
        }
        else
        {
            len =sal_strlen(argv[4]);
            if (len > DHCP_OPTION_MAX_LEN)
            {
                cli_out(cli, "%% ASCII string should not exceed %d bytes\n",
                      DHCP_OPTION_MAX_LEN);
                return CLI_ERROR;
            }
            
            clientId_type = DHCP_CLENTID_ASCII;
            if(NULL == pPool->clientId 
                || len != pPool->clientId_len
                || sal_memcmp(argv[4], pPool->clientId, len))
            {
                change = 1;
            }
        }
    }
  
    if(0 == change)
        return CLI_SUCCESS;
    
    if(staticPool == pPool->pool_type)
         isc_dhcpserver_remove_static_pool(pPool->pool_name);


    switch(clientId_type)
    {
    case DHCP_CLENTID_NONE:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            hardware,
                            NULL, 0, DHCP_CLENTID_NONE))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    case DHCP_CLENTID_HEX:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            NULL,
                            cid, len, clientId_type))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    case DHCP_CLENTID_ASCII:
        if(dhcpserver_static_pool_config(pPool, &new_prefix, 
                            NULL,
                            (u_char *)argv[4], len, clientId_type))
        {
            cli_out (cli, "%% The static binding configure fail\n",
                     pPool->pool_name);
            return CLI_ERROR;
        }
        break;
    default:
        cli_out (cli, "%% Invalid arguments, unknown client id type: %d\n",
                 clientId_type);
        return CLI_ERROR;
    }
    
    if(isc_dhcpserver_add_static_pool(pPool, root_group))
    {
        cli_out (cli, "%% Adding static binding fail\n");
        return CLI_ERROR;
    }
    
    /*static pool do not allow lease time, so clear it*/
    pPool->dhcp_pool_common.lease_time = 0;
    return CLI_SUCCESS;
}


/*******************************************************************************
 * Name:   no_dhcp_static_binding
 * Purpose: To remove an address binding mapping
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_static_binding,
     no_dhcp_static_binding_cmd,
     "no static-bind",
     CLI_NO_STR,
     "Specify static binding information"
     )
{
    dhcp_pool_t *pPool = cli->index;
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    
    }
    if(pPool->pool_type == staticPool)
    {
        isc_dhcpserver_remove_static_pool(pPool->pool_name);
        pPool->pool_type = unknowType;
        if(pPool->clientId)
        {
            XFREE(MTYPE_TMP, pPool->clientId);
            pPool->clientId = NULL;
            pPool->clientId_len = 0;
        }
        pPool->clientId_type= DHCP_CLENTID_NONE;
        
        if(pPool->hw_addr)
        {
            XFREE(MTYPE_TMP, pPool->hw_addr);
            pPool->hw_addr = NULL;
            pPool->hw_len = 0;
        }
    }
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_dns_server
 * Purpose: To configure dns server option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_dns_server,
     dhcp_dns_server_cmd,
     "dns-server A.B.C.D",
     "DNS servers",
     "dns server IP address"
     )
{
    dhcp_pool_t *pPool = cli->index;
    struct pal_in4_addr *ip4_arry = (struct pal_in4_addr *)0;
    char name[POOL_NAME_SIZE+1];
    int cnt = 0;
    int retv = 1;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    ip4_arry = XCALLOC(MTYPE_TMP, argc*sizeof(struct pal_in4_addr));
    
    if(NULL == ip4_arry)
    {
        cli_out (cli, "%% No memory for dns server option\n");
        return CLI_ERROR;
    }
    
    for(cnt = 0; cnt<argc; cnt++)
    {
        retv = sal_inet_pton (AF_INET, argv[cnt], &ip4_arry[cnt]);
        if (!retv)
        {
            XFREE(MTYPE_TMP, ip4_arry);
            cli_out (cli, "%% Invalid dns server value: %s\n", argv[cnt]);
            return CLI_ERROR;
        }
    }
    
    if(pPool->dhcp_pool_common.dns_server 
        && pPool->dhcp_pool_common.n_dns_server == argc
        && !sal_memcmp(pPool->dhcp_pool_common.dns_server, ip4_arry, argc*sizeof(struct pal_in4_addr)))
    {
        XFREE(MTYPE_TMP, ip4_arry);
        return CLI_SUCCESS;
    }
    else
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DOMAIN_NAME_SERVERS);            
            if(isc_dhcpserver_dynamic_pool_option_add(name, 
                DHO_DOMAIN_NAME_SERVERS, 
                (u_int8_t *)ip4_arry, 
                argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding dns server option fail\n");                
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_DOMAIN_NAME_SERVERS);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, 
                    DHO_DOMAIN_NAME_SERVERS, 
                    (u_int8_t *)ip4_arry, 
                    argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding dns server option fail\n");
                return CLI_ERROR;
            }
        }
                        
        if(NULL != pPool->dhcp_pool_common.dns_server)
        {
            XFREE(MTYPE_TMP, pPool->dhcp_pool_common.dns_server);
        }
        pPool->dhcp_pool_common.n_dns_server = argc;
        pPool->dhcp_pool_common.dns_server= ip4_arry;
    }
    
    return CLI_SUCCESS;
}

ALI (dhcp_dns_server,
     dhcp_dns_server2_cmd,
     "dns-server A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server3_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server4_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
      "dns server IP address",
      "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server5_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
      "dns server IP address",
      "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server6_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
     "dns server IP address",
      "dns server IP address",
      "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server7_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
     "dns server IP address",
     "dns server IP address",
      "dns server IP address",
      "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
ALI (dhcp_dns_server,
     dhcp_dns_server8_cmd,
     "dns-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "DNS servers",
     "dns server IP address",
     "dns server IP address",
     "dns server IP address",
     "dns server IP address",
      "dns server IP address",
      "dns server IP address",
     "dns server IP address",
     "dns server IP address"
     );
/*******************************************************************************
 * Name:   no_dhcp_static_binding
 * Purpose: To remove dns server option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_dns_server,
     no_dhcp_dns_server_cmd,
     "no dns-server",
     CLI_NO_STR,
     "DNS servers"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.dns_server)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DOMAIN_NAME_SERVERS);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_DOMAIN_NAME_SERVERS);
        }
    
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.dns_server);
        pPool->dhcp_pool_common.dns_server = NULL;
        pPool->dhcp_pool_common.n_dns_server = 0;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_gateway
 * Purpose: To configure default route option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_gateway,
     dhcp_gateway_cmd,
     "gateway A.B.C.D",
     "Default routers",
     "default router IP address"
     )
{
    dhcp_pool_t *pPool = cli->index;
    struct pal_in4_addr *ip4_arry = (struct pal_in4_addr *)0;
    char name[POOL_NAME_SIZE+1];
    int cnt = 0;
    int retv = 1;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    ip4_arry = XCALLOC(MTYPE_TMP, argc*sizeof(struct pal_in4_addr));
    
    if(NULL == ip4_arry)
    {
        cli_out (cli, "%% No memory for default routers option\n");
        return CLI_ERROR;
    }
    
    for(cnt = 0; cnt<argc; cnt++)
    {
        retv = sal_inet_pton (AF_INET, argv[cnt], &ip4_arry[cnt]);
        if (!retv)
        {
            XFREE(MTYPE_TMP, ip4_arry);
            cli_out (cli, "%% Invalid default routers value: %s\n", argv[cnt]);
            return CLI_ERROR;
        }
    }
    
    if(pPool->dhcp_pool_common.gateway 
        && pPool->dhcp_pool_common.n_gateway
        && !sal_memcmp(pPool->dhcp_pool_common.gateway, ip4_arry, argc*sizeof(struct pal_in4_addr)))
    {
        XFREE(MTYPE_TMP, ip4_arry);
        return CLI_SUCCESS;
    }
    else
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_ROUTERS);
            if(isc_dhcpserver_dynamic_pool_option_add(name, 
                DHO_ROUTERS, 
                (u_int8_t *)ip4_arry, 
                argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding default routers option fail\n");                
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_ROUTERS);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, 
                    DHO_ROUTERS, 
                    (u_int8_t *)ip4_arry, 
                    argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding default routers option fail\n");
                return CLI_ERROR;
            }
        }
                
        if(NULL != pPool->dhcp_pool_common.gateway)
        {
            XFREE(MTYPE_TMP, pPool->dhcp_pool_common.gateway);
        }
        pPool->dhcp_pool_common.n_gateway = argc;
        pPool->dhcp_pool_common.gateway= ip4_arry;
    }
    
    return CLI_SUCCESS;
}

ALI (dhcp_gateway,
     dhcp_gateway2_cmd,
     "gateway A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway3_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway4_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway5_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway6_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway7_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );
ALI (dhcp_gateway,
     dhcp_gateway8_cmd,
     "gateway A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Default routers",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address",
     "default router IP address"
     );

/*******************************************************************************
 * Name:   no_dhcp_gateway
 * Purpose: To remove default route option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_gateway,
     no_dhcp_gateway_cmd,
     "no gateway",
     CLI_NO_STR,
     "Default routers"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.gateway)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_ROUTERS);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_ROUTERS);
        }
    
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.gateway);
        pPool->dhcp_pool_common.gateway = NULL;
        pPool->dhcp_pool_common.n_gateway = 0;
    }
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_netbios_name_server
 * Purpose: To configure netbios name server option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_netbios_name_server,
     dhcp_netbios_name_server_cmd,
     "netbios-name-server A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address"
     )
{
    dhcp_pool_t *pPool = cli->index;
    struct pal_in4_addr *ip4_arry = (struct pal_in4_addr *)0;
    char name[POOL_NAME_SIZE+1];
    int cnt = 0;
    int retv = 1;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    ip4_arry = XCALLOC(MTYPE_TMP, argc*sizeof(struct pal_in4_addr));
    
    if(NULL == ip4_arry)
    {
        cli_out (cli, "%% No memory for NetBIOS (WINS) name servers option\n");
        return CLI_ERROR;
    }
    
    for(cnt = 0; cnt<argc; cnt++)
    {
        retv = sal_inet_pton (AF_INET, argv[cnt], &ip4_arry[cnt]);
        if (!retv)
        {
            XFREE(MTYPE_TMP, ip4_arry);
            cli_out (cli, "%% Invalid NetBIOS (WINS) name servers value: %s\n", argv[cnt]);
            return CLI_ERROR;
        }
    }
    
    if(pPool->dhcp_pool_common.netbios_server 
        && pPool->dhcp_pool_common.n_netbios_server
        && !sal_memcmp(pPool->dhcp_pool_common.netbios_server, ip4_arry, argc*sizeof(struct pal_in4_addr)))
    {
        XFREE(MTYPE_TMP, ip4_arry);
        return CLI_SUCCESS;
    }
    else
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_NETBIOS_NAME_SERVERS);
            if(isc_dhcpserver_dynamic_pool_option_add(name, 
                DHO_NETBIOS_NAME_SERVERS, 
                (u_int8_t *)ip4_arry, 
                argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding NetBIOS (WINS) name servers option fail\n");                
                return CLI_ERROR;
            }            
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_NETBIOS_NAME_SERVERS);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, 
                    DHO_NETBIOS_NAME_SERVERS, 
                    (u_int8_t *)ip4_arry, 
                    argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding NetBIOS (WINS) name servers option fail\n");
                return CLI_ERROR;
            }
        }
        
        if(pPool->dhcp_pool_common.netbios_server)
        {
            XFREE(MTYPE_TMP, pPool->dhcp_pool_common.netbios_server);
        }
        pPool->dhcp_pool_common.n_netbios_server = argc;
        pPool->dhcp_pool_common.netbios_server= ip4_arry;
    }
    
    return CLI_SUCCESS;
}

ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server2_cmd,
     "netbios-name-server A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server3_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server4_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server5_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server6_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server7_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );
ALI (dhcp_netbios_name_server,
     dhcp_netbios_name_server8_cmd,
     "netbios-name-server A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "NetBIOS (WINS) name servers",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address",
     "NetBIOS (WINS) name servers IP address"
     );

/*******************************************************************************
 * Name:   no_dhcp_netbios_name_server
 * Purpose: To remove netbios name server option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_netbios_name_server,
     no_dhcp_netbios_name_server_cmd,
     "no netbios-name-server",
     CLI_NO_STR,
     "NetBIOS (WINS) name servers"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.netbios_server)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_NETBIOS_NAME_SERVERS);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_NETBIOS_NAME_SERVERS);
        }
    
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.netbios_server);
        pPool->dhcp_pool_common.netbios_server = NULL;
        pPool->dhcp_pool_common.n_netbios_server = 0;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_next_server
 * Purpose: To configure tftp server address list for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_tftp_server_address,
     dhcp_tftp_server_address_cmd,
     "tftp-server-address A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address"
     )
{
    dhcp_pool_t *pPool = cli->index;
    struct pal_in4_addr *ip4_arry = (struct pal_in4_addr *)0;
    char name[POOL_NAME_SIZE+1];
    int cnt = 0;
    int retv = 1;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    ip4_arry = XCALLOC(MTYPE_TMP, argc*sizeof(struct pal_in4_addr));
    
    if(NULL == ip4_arry)
    {
        cli_out (cli, "%% No memory for TFTP servers option\n");
        return CLI_ERROR;
    }
    
    for(cnt = 0; cnt<argc; cnt++)
    {
        retv = sal_inet_pton (AF_INET, argv[cnt], &ip4_arry[cnt]);
        if (!retv)
        {
            XFREE(MTYPE_TMP, ip4_arry);
            cli_out (cli, "%% Invalid TFTP servers value: %s\n", argv[cnt]);
            return CLI_ERROR;
        }
    }
    
    if(pPool->dhcp_pool_common.tftp_server_addr 
        && pPool->dhcp_pool_common.n_tftp_server_addr
        && !sal_memcmp(pPool->dhcp_pool_common.tftp_server_addr, ip4_arry, argc*sizeof(struct pal_in4_addr)))
    {
        XFREE(MTYPE_TMP, ip4_arry);
        return CLI_SUCCESS;
    }
    else
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_TFTP_SERVER_ADDRS);
            if(isc_dhcpserver_dynamic_pool_option_add(name, 
                DHO_TFTP_SERVER_ADDRS, 
                (u_int8_t *)ip4_arry, 
                argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding TFTP servers option fail\n");                
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_TFTP_SERVER_ADDRS);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, 
                    DHO_TFTP_SERVER_ADDRS, 
                    (u_int8_t *)ip4_arry, 
                    argc*sizeof(struct pal_in4_addr), 0))
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Adding TFTP servers option fail\n");
                return CLI_ERROR;
            }
        }
        
        if(NULL != pPool->dhcp_pool_common.tftp_server_addr)
        {
            XFREE(MTYPE_TMP, pPool->dhcp_pool_common.tftp_server_addr);
        }
        pPool->dhcp_pool_common.n_tftp_server_addr = argc;
        pPool->dhcp_pool_common.tftp_server_addr= ip4_arry;
    }
    
    return CLI_SUCCESS;
}


ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address2_cmd,
     "tftp-server-address A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address3_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address4_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address5_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address6_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address7_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );
ALI (dhcp_tftp_server_address,
     dhcp_tftp_server_address8_cmd,
     "tftp-server-address A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Specify TFTP servers",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address",
     "TFTP servers IP address"
     );

/*******************************************************************************
 * Name:   no_dhcp_next_server
 * Purpose: To remove tftp server address list for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_tftp_server_address,
     no_dhcp_tftp_server_address_cmd,
     "no tftp-server-address",
     CLI_NO_STR,
     "Specify TFTP servers"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.tftp_server_addr)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_TFTP_SERVER_ADDRS);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_TFTP_SERVER_ADDRS);
        }
    
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.tftp_server_addr);
        pPool->dhcp_pool_common.tftp_server_addr = NULL;
        pPool->dhcp_pool_common.n_tftp_server_addr = 0;
    }
    
    return CLI_SUCCESS;
}


/*******************************************************************************
 * Name:   dhcp_netbios_type
 * Purpose: To configure netbios type option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_netbios_type,
     dhcp_netbios_type_cmd,
     "netbios-node-type (b-node|p-node|m-node|h-node)",
     "NetBIOS node type",
     "Broadcast node",
     "Peer-to-peer node",
     "Mixed node",
     "Hybrid node"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    u_int8_t netbios_node_type = UNKNOWN_NODE;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");   
        return CLI_ERROR;
    }
    
    if(!sal_strcmp(argv[0], "b-node"))
    {
        netbios_node_type = B_NODE;
    }
    else if(!sal_strcmp(argv[0], "p-node"))
    {
        netbios_node_type = P_NODE;
    }
    else if(!sal_strcmp(argv[0], "m-node"))
    {
        netbios_node_type = M_NODE;
    }
    else
    {
        netbios_node_type = H_NODE;
    }
    
    if(netbios_node_type != pPool->dhcp_pool_common.netbios_node_type)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);    
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_NETBIOS_NODE_TYPE);
            if(isc_dhcpserver_dynamic_pool_option_add(name, DHO_NETBIOS_NODE_TYPE, 
                                        &netbios_node_type, 1, 0))
            {
                cli_out (cli, "%% Adding NetBIOS node type option fail\n");
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_NETBIOS_NODE_TYPE);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, DHO_NETBIOS_NODE_TYPE, 
                                        &netbios_node_type, 1, 0))
            {
                cli_out (cli, "%% Adding NetBIOS node type option fail\n");
                return CLI_ERROR;
            }
        }
        pPool->dhcp_pool_common.netbios_node_type = netbios_node_type;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_netbios_type
 * Purpose: To remove netbios type option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_netbios_type,
     no_dhcp_netbios_type_cmd,
     "no netbios-node-type",
     CLI_NO_STR,
     "NetBIOS node type"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
        isc_dhcpserver_dynamic_pool_option_remove(name, DHO_NETBIOS_NODE_TYPE);
    }
    
    if(staticPool == pPool->pool_type)
    {
        isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_NETBIOS_NODE_TYPE);
    }
    
    pPool->dhcp_pool_common.netbios_node_type = UNKNOWN_NODE;
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_network_mask
 * Purpose: To configure network for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_network_mask,
     dhcp_network_mask_cmd,
     "network A.B.C.D A.B.C.D",
     "Network number and mask",
     "Specify an IP address range",
     "Specify an IP address prefix"
     )
{
    struct prefix new_prefix;
    struct pal_in4_addr mask;
    dhcp_pool_t *pPool = cli->index;
    dhcp_pool_t *pTmp = NULL;
    char old_name[POOL_NAME_SIZE+1];
    char new_name[POOL_NAME_SIZE+1];
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }

    if(staticPool== pPool->pool_type)
    {
        cli_out (cli, "%% This command may not be used with static pools\n");    
        return CLI_ERROR;
    }
    
    /* Clean prefix */
    sal_memset (&new_prefix, 0, sizeof (struct prefix));
    sal_memset (&mask, 0, sizeof (struct pal_in4_addr));
    sal_memset (old_name, 0, POOL_NAME_SIZE+1);
    sal_memset (new_name, 0, POOL_NAME_SIZE+1);
    
    /* Get the ip address */
    DHCP_CLI_GET_IPV4_PREFIX("network address", new_prefix, argv[0]);
    
    /* Get mask. */
    CLI_GET_IPV4_ADDRESS("wildcard mask", mask, argv[1]);
    new_prefix.prefixlen = ip_masklen(mask);
    new_prefix.u.prefix4.s_addr = new_prefix.u.prefix4.s_addr & mask.s_addr;
    
    if (dhcpserver_cli_ipv4_network_check(&new_prefix))
    {
        cli_out (cli, "%% Invalid network address value\n");
        return CLI_ERROR;
    }
    
    if (new_prefix.prefixlen == 0 
        || new_prefix.prefixlen == 31 
        || new_prefix.prefixlen == 32)
    {
        cli_out (cli, "%% Bad network mask\n");
        return CLI_ERROR;
    }
    
    pTmp = dhcpserver_dynamic_pool_confilict_check(&new_prefix, pPool);
    if(pTmp)
    {
        cli_out (cli, "%% The network configuration conflict with pool %s \n",
                 pTmp->pool_name);
        return CLI_ERROR;
    }
    
    isc_dhcpserver_shared_network_name_get(new_name, POOL_NAME_SIZE+1, &new_prefix);
    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(old_name, POOL_NAME_SIZE+1, &pPool->network);
        isc_dhcpserver_remove_dynamic_pool(old_name);
    }
    
    pPool->pool_type = dynPool;
    sal_memcpy(&pPool->network, &new_prefix,sizeof(struct prefix));
    
    if(isc_dhcpserver_add_dynamic_pool(pPool, root_group))
    {
        pPool->pool_type = unknowType;
        sal_memset(&pPool->network,0 ,sizeof(struct prefix));
        cli_out (cli, "%% Adding network configuration fail\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}


/*******************************************************************************
 * Name:   dhcp_network
 * Purpose: To configure network for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_network,
     dhcp_network_cmd,
     "network A.B.C.D/M",
     "Network number and mask",
     "Specify an IP address prefix"
     "Subnet mask length"
     )
{
    struct prefix new_prefix;
    struct pal_in4_addr mask;
    dhcp_pool_t *pPool = cli->index;
    dhcp_pool_t *pTmp = NULL;
    s_int32_t ret = 0;
    char old_name[POOL_NAME_SIZE+1];
    char new_name[POOL_NAME_SIZE+1];
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }

    if(staticPool== pPool->pool_type)
    {
        cli_out (cli, "%% This command may not be used with static pools\n");    
        return CLI_ERROR;
    }
    
    sal_memset (&new_prefix, 0, sizeof (struct prefix));
    sal_memset (old_name, 0, POOL_NAME_SIZE+1);
    sal_memset (new_name, 0, POOL_NAME_SIZE+1);
    
    /* Get the ip address and mask */
    ret = str2prefix (argv[0], &new_prefix);
    sal_masklen2ip(new_prefix.prefixlen, &mask);
    new_prefix.u.prefix4.s_addr = new_prefix.u.prefix4.s_addr & mask.s_addr;
    
    if (dhcpserver_cli_ipv4_network_check(&new_prefix))
    {
        cli_out (cli, "%% Bad network address\n");
        return CLI_ERROR;
    }
    
    if (0 == ret 
        ||new_prefix.prefixlen == 0 
        || new_prefix.prefixlen == 31 
        || new_prefix.prefixlen == 32)
    {
        cli_out (cli, "%% Invalid network address value\n");
        return CLI_ERROR;
    }
    
    
    pTmp = dhcpserver_dynamic_pool_confilict_check(&new_prefix, pPool);
    if(pTmp)
    {
        cli_out (cli, "%% The network configuration conflict with pool %s \n",
                 pTmp->pool_name);
        return CLI_ERROR;
    }
    
    isc_dhcpserver_shared_network_name_get(new_name, POOL_NAME_SIZE+1, &new_prefix);
    
    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(old_name, POOL_NAME_SIZE+1, &pPool->network);
        isc_dhcpserver_remove_dynamic_pool(old_name);
    }
    
    pPool->pool_type = dynPool;
    sal_memcpy(&pPool->network, &new_prefix,sizeof(struct prefix));
    
    if(isc_dhcpserver_add_dynamic_pool(pPool, root_group))
    {
        pPool->pool_type = unknowType;
        sal_memset(&pPool->network,0 ,sizeof(struct prefix));
        cli_out (cli, "%% Adding network configuration fail\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}


/*******************************************************************************
 * Name:   no_dhcp_netbios_type
 * Purpose: To remove netbios type option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_network,
     no_dhcp_network_cmd,
     "no network",
     CLI_NO_STR,
     "Network number and mask"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char old_name[POOL_NAME_SIZE+1];
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }

    if(staticPool == pPool->pool_type)
    {
        return CLI_SUCCESS;        
    }
    
    sal_memset (old_name, 0, POOL_NAME_SIZE+1);
    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(old_name, POOL_NAME_SIZE+1, &pPool->network);
        isc_dhcpserver_remove_dynamic_pool(old_name);
    }
    
    pPool->pool_type = unknowType;
    sal_memset(&pPool->network, 0, sizeof(struct prefix));
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_lease
 * Purpose: To configure address lease time for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_lease,
     dhcp_lease_cmd,
     "lease <0-365> (<0-23> (<0-59>|)|)",
     "Address lease time for dynamic users",
     "Days",
     "Hours",
     "Minutes"
     )
{
    dhcp_pool_t *pPool = cli->index;
    u_int32_t lease_time;
    char name[POOL_NAME_SIZE+1];
    unsigned days = 0;
    unsigned hours = 0;
    unsigned minutes = 0;
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }
    
    if(staticPool == pPool->pool_type)
    {
        cli_out(cli, "%% DHCP static pool does not allow configure the lease time\n");
        return CLI_ERROR;
    }

    switch(argc)
    {
        case 1:
            CLI_GET_INTEGER_RANGE ("Days", days, argv[0], 0, 365);
            break;
        case 2:
            CLI_GET_INTEGER_RANGE ("Days", days, argv[0], 0, 365);
            CLI_GET_INTEGER_RANGE ("Hours", hours, argv[1], 0, 23);
            break;
        case 3:
            CLI_GET_INTEGER_RANGE ("Days", days, argv[0], 0, 365);
            CLI_GET_INTEGER_RANGE ("Hours", hours, argv[1], 0, 23);
            CLI_GET_INTEGER_RANGE ("Minutes", minutes, argv[2], 0, 59);
            break;
    }
    lease_time = days*LEASE_ONE_DAY + hours*LEASE_ONE_HOUR + minutes*LEASE_ONE_MINUTE;
    
    if(lease_time < 300)
    {
        cli_out (cli, "%% The lease time must larger than 5 minutes\n");       
        return CLI_ERROR;
    }
    
    if(lease_time != pPool->dhcp_pool_common.lease_time)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);    
            if(0 != pPool->dhcp_pool_common.lease_time)
                isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DHCP_LEASE_TIME);
            
            if(isc_dhcpserver_dynamic_pool_option_add(name, DHO_DHCP_LEASE_TIME, 
                                        (u_int8_t *)&lease_time, 4, 0))
            {
                cli_out (cli, "%% Adding lease time option fail\n");
                return CLI_ERROR;
            }
        }
        
        pPool->dhcp_pool_common.lease_time = lease_time;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_lease
 * Purpose: To restore address lease time for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_lease,
     no_dhcp_lease_cmd,
     "no lease",
     CLI_NO_STR,
     "Address lease time for dynamic users"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    u_int32_t lease_time = DEFAULT_DEFAULT_LEASE_TIME;
    
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }
    
    if(staticPool == pPool->pool_type)
    {
        cli_out(cli, "%% DHCP static pool does not allow configure the lease time\n");
        return CLI_ERROR;
    }
    
    if(0 != pPool->dhcp_pool_common.lease_time)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);    
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DHCP_LEASE_TIME);
            if(isc_dhcpserver_dynamic_pool_option_add(name, DHO_DHCP_LEASE_TIME, 
                                        (u_int8_t *)&lease_time, 4, 0))
            {
                cli_out (cli, "%% Adding lease time option fail\n");
                return CLI_ERROR;
            }
        }

        pPool->dhcp_pool_common.lease_time = 0;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_option
 * Purpose: To configure dhcp option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_option,
     dhcp_option_cmd,
     "option <2-254> (ascii WORD | hex HHHH | ip A.B.C.D)",
     "Raw DHCP options",
     "DHCP option code",
     "ASCII string",
     "ASCII string",
     "Hexadecimal string",
     "Hex string",
     "IP addresses",
     "IP address"
     )
{
    dhcp_pool_t *pPool = cli->index;    
    dhcp_option_t *new_option = (dhcp_option_t *)0;
    dhcp_option_t *old_option = (dhcp_option_t *)0;
    u_int32_t data_len;
    char name[POOL_NAME_SIZE+1];
    u_int8_t buf[DHCP_OPTION_MAX_LEN+1];
    unsigned code = 0;
    enum dhcp_option_Input_type type;
    struct pal_in4_addr *ip4_arry = (struct pal_in4_addr *)0;
    int cnt = 0;
    int retv = 1;
    int terminated = 0;
    CLI_GET_INTEGER_RANGE("DHCP option ", code, argv[0], 2, 254);
    
    if(0 == isc_dhcpserver_valid_option_check(code))
    {
        cli_out(cli, "%% DHCP does not allow raw option %d\n", code);
        return CLI_ERROR;
    }

    if(!sal_strcmp(argv[1], "ascii"))
    {
        type = ASCII_TYPE;
        terminated = 1;
    }
    else if(!sal_strcmp(argv[1], "hex"))
    {
        type = HEX_TYPE;
        terminated = 0;
    }
    else
    {
        type = IP_TYPE;
        terminated = 0;
    }

    sal_memset (buf, 0, DHCP_OPTION_MAX_LEN+1);
    
    if(IP_TYPE == type)
    {
        ip4_arry = XCALLOC(MTYPE_TMP, (argc - 2)*sizeof(struct pal_in4_addr));
        if(NULL == ip4_arry)
        {
            cli_out (cli, "%% No memory for default routers option\n");
            return CLI_ERROR;
        }
        for(cnt = 2; cnt<argc; cnt++)
        {
            retv = sal_inet_pton (AF_INET, argv[cnt], &ip4_arry[cnt - 2]);
            if (!retv)
            {
                XFREE(MTYPE_TMP, ip4_arry);
                cli_out (cli, "%% Invalid option value: %s\n", argv[cnt]);
                return CLI_ERROR;
            }
        }
        data_len = (argc-2)*sizeof(struct pal_in4_addr);
        sal_memcpy(buf, ip4_arry, data_len);
    }
    else if(ASCII_TYPE == type)
    {
        data_len = sal_strlen(argv[2]);
        if(DHCP_OPTION_MAX_LEN < data_len)
        {
            cli_out(cli, "%% ASCII string should not exceed %d bytes\n",
                  DHCP_OPTION_MAX_LEN);
            return CLI_ERROR;
        }
        sal_memcpy(buf, argv[2], data_len+1);
    }
    else
    {
        data_len = sal_strlen(argv[2]);
        if(DHCP_OPTION_MAX_LEN*2 < data_len)
        {
            cli_out(cli, "%% Hex string should not exceed %d bytes\n",
                  DHCP_OPTION_MAX_LEN*2);
            return CLI_ERROR;
        }

        if(0 != dhcpserver_cli_check_hex(argv[2]))
        {
            cli_out (cli, "%% Invalid Hex string\n");
            return CLI_ERROR;
        }

        dhcpserver_cli_hex_get(argv[2], buf);
        data_len = data_len>>1;
    }
    
    old_option= dhcpserver_pool_option_lookup(pPool, code);

    if(old_option 
        && old_option->type == type 
        && old_option->data_len == (data_len+terminated)
        && !sal_memcmp(old_option->buf, buf, data_len+terminated))
    {
        return CLI_SUCCESS;
    }
        
    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
        isc_dhcpserver_dynamic_pool_option_remove(name, code);        
        if(isc_dhcpserver_dynamic_pool_option_add(name, code, 
                                    buf, data_len, terminated))
        {
            cli_out (cli, "%% Adding option %d fail\n", code);
            return CLI_ERROR;
        }
    }
    
    if(staticPool == pPool->pool_type)
    {
        isc_dhcpserver_static_pool_option_remove(pPool->pool_name, code);
        if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, code, 
                                    buf, data_len, terminated))
        {
            cli_out (cli, "%% Adding option %d fail\n", code);
            return CLI_ERROR;
        }
    }
    
    new_option = (dhcp_option_t*)0;
    new_option = dhcpserver_pool_option_get(code, buf, data_len+terminated, type);
    
    if(NULL == new_option)
    {
        cli_out (cli, "%% No memory for default routers option\n");
        return CLI_ERROR;
    }
        
    old_option = dhcpserver_pool_option_remove(pPool, code);    
    dhcpserver_pool_option_free(&old_option);
    dhcpserver_pool_option_add(pPool, new_option);
    
    return CLI_SUCCESS;
}

ALI (dhcp_option,
     dhcp_option2_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option3_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option4_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
       "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option5_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option6_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address",
      "IP address",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option7_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address"
    );
ALI (dhcp_option,
     dhcp_option8_cmd,
     "option <2-254> ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
     "Raw DHCP options",
     "DHCP option code",
     "IP addresses",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address",
     "IP address"
    );

/*******************************************************************************
 * Name:   no_dhcp_option
 * Purpose: To remove dhcp option for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_option,
     no_dhcp_option_cmd,
     "no option <2-254>",
     CLI_NO_STR,
     "Raw DHCP options",
     "DHCP option code"
     )
{
    dhcp_pool_t *pPool = cli->index;    
    unsigned code;
    dhcp_option_t *old_option = (dhcp_option_t *)0;
    char name[POOL_NAME_SIZE+1];
    
    CLI_GET_INTEGER_RANGE("DHCP option ", code, argv[0], 2, 254);
    
    if(0 == isc_dhcpserver_valid_option_check(code))
    {
        cli_out(cli, "%% DHCP does not allow raw option %d\n", code);
        return CLI_ERROR;
    }
    
    old_option = dhcpserver_pool_option_lookup(pPool, code);

    if(NULL == old_option)
    {
        return CLI_SUCCESS;
    }

    if(dynPool == pPool->pool_type)
    {
        isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);    
        isc_dhcpserver_dynamic_pool_option_remove(name, code);
    }
    
    if(staticPool == pPool->pool_type)
    {
        isc_dhcpserver_static_pool_option_remove(pPool->pool_name, code);
    }
    
    old_option = dhcpserver_pool_option_remove(pPool, code);
    
    dhcpserver_pool_option_free(&old_option);
    return CLI_SUCCESS;
}


/*******************************************************************************
 * Name:   dhcp_domain_name
 * Purpose: To configure domain name for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_domain_name,
     dhcp_domain_name_cmd,
     "domain-name WORD",
     "Specify Domain name",
     "Domain name"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char *domain_name = NULL;
    char name[POOL_NAME_SIZE+1];
    u_int32_t len = 0;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }
    
    len = sal_strlen(argv[0]);
    if(len > DHCP_OPTION_MAX_LEN)
    {
        cli_out (cli, "%% Domain-name length should less than %d characters\n", DHCP_OPTION_MAX_LEN+1);       
        return CLI_ERROR;
    }
    
    if(dhcpserver_cli_vaild_string_check(argv[0], DHCP_OPTION_MAX_LEN))
    {
        DHCP_OUT_ERR_STR;
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.domain_name 
        && !sal_strcmp(pPool->dhcp_pool_common.domain_name, argv[0]))
    {
        return CLI_SUCCESS;
    }
    else
    {
        domain_name = XCALLOC(MTYPE_TMP, len+1);
        if(NULL == domain_name)
        {
            cli_out (cli, "%% No memory for domain-name option\n");
            return CLI_ERROR;
        }
        
        sal_strcpy(domain_name, argv[0]);
        
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DOMAIN_NAME);
            if(isc_dhcpserver_dynamic_pool_option_add(name, DHO_DOMAIN_NAME, 
                                        (u_char *)domain_name, len, 1))
            {
                cli_out (cli, "%% Adding domain-name option fail\n");
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_DOMAIN_NAME);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, DHO_DOMAIN_NAME, 
                                        (u_char *)domain_name, len, 1))
            {
                cli_out (cli, "%% Adding domain-name option fail\n");
                return CLI_ERROR;
            }
        }
        
        pPool->dhcp_pool_common.domain_name = domain_name;
    }

    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   no_dhcp_domain_name
 * Purpose: To remove domain name for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_domain_name,
     no_dhcp_domain_name_cmd,
     "no domain-name",
     CLI_NO_STR,
     "Specify Domain name"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(NULL != pPool->dhcp_pool_common.domain_name)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_DOMAIN_NAME);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_DOMAIN_NAME);
        }
        
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.domain_name);
        pPool->dhcp_pool_common.domain_name = NULL;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:   dhcp_bootfile_name
 * Purpose: To configure bootfile name for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (dhcp_bootfile_name,
     dhcp_bootfile_name_cmd,
     "bootfile-name WORD",
     "Specify boot file names",
     "Boot file names"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char *bootfile_name = NULL;
    char name[POOL_NAME_SIZE+1];
    u_int32_t len = 0;
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;

    }
    
    len = sal_strlen(argv[0]);
    if(len > DHCP_OPTION_MAX_LEN)
    {
        cli_out (cli, "%% Bootfile name length should less than %d characters\n", DHCP_OPTION_MAX_LEN+1);     
        return CLI_ERROR;
    }
    
    if(dhcpserver_cli_vaild_string_check(argv[0], DHCP_OPTION_MAX_LEN))
    {
        DHCP_OUT_ERR_STR;
        return CLI_ERROR;
    }
    
    if(pPool->dhcp_pool_common.bootfile_name 
        && !sal_strcmp(pPool->dhcp_pool_common.bootfile_name, argv[0]))
    {
        return CLI_SUCCESS;
    }
    else
    {
        bootfile_name = XCALLOC(MTYPE_TMP, len+1);
        if(NULL == bootfile_name)
        {
            cli_out (cli, "%% No memory for bootfile name option\n");
            return CLI_ERROR;
        }
        
        sal_strcpy(bootfile_name, argv[0]);
        
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_BOOTFILE_NAME);
            if(isc_dhcpserver_dynamic_pool_option_add(name, DHO_BOOTFILE_NAME, 
                                        (u_char *)bootfile_name, len, 1))
            {
                cli_out (cli, "%% Adding bootfile name option fail\n");
                return CLI_ERROR;
            }
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_BOOTFILE_NAME);
            if(isc_dhcpserver_static_pool_option_add(pPool->pool_name, DHO_BOOTFILE_NAME, 
                                        (u_char *)bootfile_name, len, 1))
            {
                cli_out (cli, "%% Adding bootfile name option fail\n");
                return CLI_ERROR;
            }
        }
        
        pPool->dhcp_pool_common.bootfile_name = bootfile_name;
    }

    return CLI_SUCCESS;
}



/*******************************************************************************
 * Name:   no_dhcp_domain_name
 * Purpose: To remove bootfile for dhcp pool
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (no_dhcp_bootfile_name,
     no_dhcp_bootfile_name_cmd,
     "no bootfile-name",
     CLI_NO_STR,
     "Specify boot file names"
     )
{
    dhcp_pool_t *pPool = cli->index;
    char name[POOL_NAME_SIZE+1];
    if(NULL == pPool)
    {
        cli_out (cli, "%% Can't find dhcp pool statement\n");       
        return CLI_ERROR;
    }
    
    if(NULL != pPool->dhcp_pool_common.bootfile_name)
    {
        if(dynPool == pPool->pool_type)
        {
            isc_dhcpserver_shared_network_name_get(name, POOL_NAME_SIZE+1, &pPool->network);            
            isc_dhcpserver_dynamic_pool_option_remove(name, DHO_BOOTFILE_NAME);
        }
        
        if(staticPool == pPool->pool_type)
        {
            isc_dhcpserver_static_pool_option_remove(pPool->pool_name, DHO_BOOTFILE_NAME);
        }
        
        XFREE(MTYPE_TMP, pPool->dhcp_pool_common.bootfile_name);
        pPool->dhcp_pool_common.bootfile_name = NULL;
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    show_dhcpserver_statistics
 * Purpose: statistic command
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (show_dhcpserver_statistics,
     show_dhcpserver_statistics_cmd,
     "show dhcp server statistics",
     "Show configuration",
     "Show DHCP configuration",
     "Show server configuration",
     "Statistics information"
    )
{/*for bug 24702 modified by liuyang 2013-8-30*/
    cli_out(cli, "DHCP server packet statistics:\n");
    DHCP_OUT_SPLIT;
    cli_out(cli, "Message Received:  \n");
    cli_out(cli, "BOOTREQUEST  : %lu\n", server_packet_boot_request);
    cli_out(cli, "DHCPDISCOVER : %lu\n", server_packet_dhcp_discover);
    cli_out(cli, "DHCPREQUEST  : %lu\n", server_packet_dhcp_request);
    cli_out(cli, "DHCPDECLINE  : %lu\n", server_packet_dhcp_decline);
    cli_out(cli, "DHCPRELEASE  : %lu\n", server_packet_dhcp_release);
    cli_out(cli, "DHCPINFORM   : %lu\n\n", server_packet_dhcp_inform);
    cli_out(cli, "Message Sent:  \n");
    cli_out(cli, "BOOTREPLY    : %lu\n", server_packet_boot_reply);
    cli_out(cli, "DHCPOFFER    : %lu\n", server_packet_dhcp_offer);
    cli_out(cli, "DHCPACK      : %lu\n", server_packet_dhcp_ack);
    cli_out(cli, "DHCPNAK      : %lu\n", server_packet_dhcp_nak);
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    clear_dhcpserver_statistics
 * Purpose: clear dhcp server statistics
 * Input: 
 * Output: 
 * Return: 
 *   success: CLI_SUCCESS
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
CLI (clear_dhcpserver_statistics,
     clear_dhcpserver_statistics_cmd,
     "clear dhcp server statistics",
     "Reset",
     "DHCP configuration",
     "Server",
     "Statistics information"
     )
{
    server_packet_boot_request = 0;
    server_packet_boot_reply = 0;
    server_packet_dhcp_discover = 0;
    server_packet_dhcp_request = 0;
    server_packet_dhcp_decline = 0;
    server_packet_dhcp_release = 0;
    server_packet_dhcp_inform = 0;
    server_packet_dhcp_offer = 0;
    server_packet_dhcp_ack = 0;
    server_packet_dhcp_nak = 0;

    return CLI_SUCCESS;
}

CLI (show_dhcpserver_conflict,
     show_dhcpserver_conflict_cmd,
     "show dhcp server conflict (ip A.B.C.D |all)",
     "Show configuration",
     "Show DHCP configuration",
     "Server",
     "DHCP server address conflicts",
     "Specify a specific IP address",
     "IP address",
     "All information"
     )
{
    struct pal_in4_addr ip;
    
    sal_memset(&ip, 0, sizeof (struct pal_in4_addr));
    
    if(!sal_strcmp(argv[0], "ip"))
    {
        
        CLI_GET_IPV4_ADDRESS("conflict ip address", ip, argv[1]);
        dhcpserver_cli_show_conflict(cli, &ip, 0);
    }
    else
    {
        dhcpserver_cli_show_conflict(cli, (struct pal_in4_addr *)0, 1);
    }
    
    return CLI_SUCCESS;
}

CLI (clear_dhcpserver_conflict,
     clear_dhcpserver_conflict_cmd,
     "clear dhcp server conflict (ip A.B.C.D | all)",
     "Reset",
     "DHCP configuration",
     "Server",
     "DHCP server address conflicts",
     "Specify a specific IP address",
     "IP address",
     "All information"
     )
{
    struct pal_in4_addr ip;
    
    sal_memset(&ip, 0, sizeof (struct pal_in4_addr));
    if(!sal_strcmp(argv[0], "ip"))
    {
        CLI_GET_IPV4_ADDRESS("conflict ip address", ip, argv[1]);
        if(dhcpserver_cli_clear_conflict_lease(cli, &ip, 0))
        {
            return CLI_ERROR;
        }
    }
    else
    {
        if(dhcpserver_cli_clear_conflict_lease(cli, (struct pal_in4_addr *)0, 1))
        {
            return CLI_ERROR;
        }
    }
    
    return CLI_SUCCESS;
}

CLI (show_dhcpserver_binding,
     show_dhcpserver_binding_cmd,
     "show dhcp server binding (ip A.B.C.D | pool WORD | all)",
     "Show configuration",
     "Show DHCP configuration",
     "Server",
     "Addresses assigned",
     "Specify a specific IP address",
     "IP address",
     "Pool",
     "Pool name",
     "All information"
     )
{
    struct pal_in4_addr ip;
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t stPool;
    
    if(!sal_strcmp(argv[0], "ip"))
    {
    
        CLI_GET_IPV4_ADDRESS("conflict ip address", ip, argv[1]);
        dhcpserver_cli_show_binding(cli, &ip, (dhcp_pool_t *)0, 0);
    }
    else if(!sal_strcmp(argv[0], "pool"))
    {
    
        if(sal_strlen(argv [1]) > POOL_NAME_SIZE)
        {
            cli_out (cli, "%% Configuration pool name should not be more than %d characters\n", POOL_NAME_SIZE);       
            return CLI_ERROR;
        }
        
        if(dhcpserver_cli_vaild_string_check(argv[1], POOL_NAME_SIZE))
        {
            DHCP_OUT_ERR_STR;
            return CLI_ERROR;
        }
        
        sal_strcpy(stPool.pool_name, argv[1]);
        pPool = hash_lookup(g_dhcpserverPoolhash, &stPool);
        
        if(NULL == pPool)
        {
            cli_out (cli, "%% %s is not in the database.\n", argv[1]);
            return CLI_ERROR;
        }
        dhcpserver_cli_show_binding(cli, (struct pal_in4_addr *)0, pPool, 0);
    }
    else
    {
    
        dhcpserver_cli_show_binding(cli, (struct pal_in4_addr *)0, (dhcp_pool_t *)0, 1);
    }
    
    return CLI_SUCCESS;
}

CLI (clear_dhcpserver_binding,
     clear_dhcpserver_binding_cmd,
     "clear dhcp server binding (ip A.B.C.D | pool WORD | all)",
     "Reset",
     "DHCP configuration",
     "Server",
     "Addresses assigned",
     "Specify a specific IP address",
     "IP address",
     "Pool",
     "Pool name",
     "All information"
     )
{
    struct pal_in4_addr ip;
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t stPool;
    
    if(!sal_strcmp(argv[0], "ip"))
    {
    
        CLI_GET_IPV4_ADDRESS("conflict ip address", ip, argv[1]);
        if(dhcpserver_cli_clear_binding_lease(cli, &ip, (dhcp_pool_t *)0, 0))
        {
            return CLI_ERROR;
        }        
    }
    else if(!sal_strcmp(argv[0], "pool"))
    {
    
        if(sal_strlen(argv [1]) > POOL_NAME_SIZE)
        {
            cli_out (cli, "%% Configuration pool name should not be more than %d characters\n", POOL_NAME_SIZE);       
            return CLI_ERROR;
        }
        
        if(dhcpserver_cli_vaild_string_check(argv[1], POOL_NAME_SIZE))
        {
            DHCP_OUT_ERR_STR;
            return CLI_ERROR;
        }

        sal_memset(&stPool, 0, sizeof(dhcp_pool_t));
        sal_strcpy(stPool.pool_name, argv[1]);
        pPool = hash_lookup(g_dhcpserverPoolhash, &stPool);
        
        if(NULL == pPool)
        {
            cli_out (cli, "%% %s is not in the database.\n", argv[1]);
            return CLI_ERROR;
        }
        
        if(dhcpserver_cli_clear_binding_lease(cli, (struct pal_in4_addr *)0, pPool, 0))
        {
            return CLI_ERROR;
        }
    }
    else
    {
        if(dhcpserver_cli_clear_binding_lease(cli, (struct pal_in4_addr *)0, (dhcp_pool_t *)0, 1))
        {
            return CLI_ERROR;
        }
    }
    
    return CLI_SUCCESS;
}

/*******************************************************************************
 * Name:    dhcpserver_cli_init
 * Purpose: initialize dhcp server module commands
 * Input:  
 *   ctree: client tree
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/

void dhcpserver_cli_init(struct cli_tree *ctree)
{
    if (NULL == ctree)
    {
        return;
    }

    cli_install_config(ctree, DHCPS_MODE, dhcpserver_config_write);
    cli_install_default (ctree, DHCPS_MODE);
    /* configuration commands */
    cli_install_imi(ctree, CONFIG_MODE, PM_DHCPD|PM_DHCPSNOOPING|PM_DHCPRELAY|PM_NSM, PRIVILEGE_VR_MAX,
                    0, &dhcp_service_switch_cmd);
    cli_install_imi(ctree, CONFIG_MODE, PM_DHCPD|PM_NSM, PRIVILEGE_VR_MAX,
                    0, &dhcpserver_service_switch_cmd);
    cli_install_imi(ctree, CONFIG_MODE, PM_DHCPD|PM_NSM, PRIVILEGE_VR_MAX, 
                    0, &no_dhcpserver_service_switch_cmd);
    
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_excluded_addr_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_excluded_addr_cmd);
    
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_ping_packet_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_ping_packet_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_ping_timeout_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_ping_timeout_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_pool_cmd);
    cli_install_gen(ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_pool_cmd);
    
    /* dhcp pool commands */
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_static_binding_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_static_binding_mask_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_static_binding_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server2_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server3_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server4_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server5_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server6_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server7_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_dns_server8_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_dns_server_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway2_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway3_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway4_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway5_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway6_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway7_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_gateway8_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_gateway_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server2_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server3_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server4_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server5_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server6_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server7_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_name_server8_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_netbios_name_server_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address2_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address3_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address4_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address5_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address6_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address7_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_tftp_server_address8_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_tftp_server_address_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_netbios_type_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_netbios_type_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_network_mask_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_network_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_network_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_domain_name_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_domain_name_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_bootfile_name_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_bootfile_name_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_lease_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_lease_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option2_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option3_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option4_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option5_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option6_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option7_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &dhcp_option8_cmd);
    cli_install_gen(ctree, DHCPS_MODE, PRIVILEGE_VR_MAX, 0, &no_dhcp_option_cmd);
    
    /* show commands */
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcpserver_statistics_cmd);
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcpserver_conflict_cmd);
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &show_dhcpserver_binding_cmd);
    
    /* clear commands */
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &clear_dhcpserver_statistics_cmd);
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &clear_dhcpserver_conflict_cmd);
    cli_install_gen(ctree, EXEC_MODE, PRIVILEGE_VR_MAX, 0, &clear_dhcpserver_binding_cmd);

    /* debug commands */  
    DEBUG_CLI_INSTALL(dhcp, server, ctree, EXEC_MODE, PM_DHCPD, PRIVILEGE_VR_MAX, 0);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_write_exclude_ip_address
 * Purpose: write dhcp server exclude ip address range
 * Input: 
 *   cli:  client pointer
 *   pExcludeIpRange: exclude ip address range
 *   pnLines: writed lines
 * Output: 
 * Return: N/A
 * Note: three type(modified by liuyang):
 *        1 start == end : it means this item was set manually for a single address;
 *        2 start != end : it means this item was set manually for a range of addresses;
 *        3 end  == 0 
 *          start!= 0: it means this item was set automatically for the server's address and should not be
 *                              writen to running-config;
 ******************************************************************************/
void dhcpserver_write_exclude_ip_address(struct cli *cli, 
        dhcp_exclude_ip_range_t *pExcludeIpRangeList, 
        int *pnLines)
{
    dhcp_exclude_ip_range_t *pItem = NULL;
    u_int32_t high = 0;
    char addr_low[16];
    char addr_high[16];
    pItem = pExcludeIpRangeList;

    if (NULL == cli || NULL == pExcludeIpRangeList || NULL == pnLines)
    {
        return;
    }
    
    while(pItem)
    {
        sal_memset(addr_low, 0, 16);
        sal_memset(addr_high, 0, 16);
        high = ntohl(pItem->endip.s_addr);
        if (high)
        {
            if (pItem->startip.s_addr != pItem->endip.s_addr)
            {
                sal_inet_ntop(AF_INET, &pItem->startip, addr_low, 16);
                sal_inet_ntop(AF_INET, &pItem->endip, addr_high, 16);
                cli_out(cli, "dhcp excluded-address %s %s\n", 
                        addr_low, addr_high);
             }
            else
            {
                sal_inet_ntop(AF_INET, &pItem->startip, addr_low, 16);
                cli_out(cli, "dhcp excluded-address %s\n", addr_low);

            }
            (*pnLines)++;
        }
        else
        {
            if (ntohl(pItem->startip.s_addr) == high)
            {
                sal_inet_ntop(AF_INET, &pItem->startip, addr_low, 16);
                cli_out(cli, "dhcp excluded-address %s\n", addr_low);
                (*pnLines)++;
            }
        }
        
        pItem = pItem->next;
    }
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_write_array_config_write
 * Purpose: write dhcp server ip address range cmd
 * Input: 
 *   cli:  client pointer
 *   str:  cmd line
 *   pIn4Addr: ip address pointer
 *   cnt: ip address count
 *   pnLines: writed lines
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_write_array_config_write(struct cli *cli, 
    char* str,
    struct pal_in4_addr *pIn4Addr, 
    u_int8_t cnt,
    int *pnLines)
{
    char ipAddr[8][16];

    if (NULL == cli || NULL == str||NULL == pIn4Addr || NULL == pnLines)
    {
        return;
    }
    
    sal_memset(ipAddr, 0, sizeof(ipAddr));
    switch(cnt)
    {
        case 1:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            cli_out(cli, " %s %s\n", str, ipAddr[0]);
            (*pnLines)++;
            break;
        case 2:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            cli_out(cli, " %s %s %s\n",
                 str, ipAddr[0], ipAddr[1]);
            (*pnLines)++;
            break;
        case 3:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            cli_out(cli, " %s %s %s %s\n",
                 str, ipAddr[0], ipAddr[1], ipAddr[2]);
            (*pnLines)++;
            break;
        case 4:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[3], ipAddr[3], 16);
            cli_out(cli, " %s %s %s %s %s\n",
                 str, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
            (*pnLines)++;
            break;
        case 5:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[3], ipAddr[3], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[4], ipAddr[4], 16);
            cli_out(cli, " %s %s %s %s %s %s\n",
                 str, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], ipAddr[4]);
            (*pnLines)++;
            break;
        case 6:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[3], ipAddr[3], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[4], ipAddr[4], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[5], ipAddr[5], 16);
            cli_out(cli, " %s %s %s %s %s %s %s",
                 str, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], ipAddr[4], ipAddr[5]);
            (*pnLines)++;
            break;
        case 7:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[3], ipAddr[3], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[4], ipAddr[4], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[5], ipAddr[5], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[6], ipAddr[6], 16);
            cli_out(cli, " %s %s %s %s %s %s %s %s\n",
                 str, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], ipAddr[4], ipAddr[5], ipAddr[6]);
            (*pnLines)++;
            break;
        case 8:
            sal_inet_ntop(AF_INET, &pIn4Addr[0], ipAddr[0], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[1], ipAddr[1], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[2], ipAddr[2], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[3], ipAddr[3], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[4], ipAddr[4], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[5], ipAddr[5], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[6], ipAddr[6], 16);
            sal_inet_ntop(AF_INET, &pIn4Addr[7], ipAddr[7], 16);
            cli_out(cli, " %s %s %s %s %s %s %s %s %s\n",
                 str, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], ipAddr[4], ipAddr[5], ipAddr[6], ipAddr[7]);
            (*pnLines)++;
            break;
        default:
            break;
    }
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_option_code_config_write
 * Purpose: write dhcp pool option code config
 * Input: 
 *   cli:  client pointer
 *   pnLines: writed lines
 * Output: 
 * Return:
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_option_code_config_write(struct cli *cli, struct dhcp_option_s *pOptions, int *pnLines)
{
    unsigned char buf[DHCP_OPTION_MAX_LEN*2+1];
    u_int8_t cnt = 0;
    
    if (NULL == cli || NULL == pOptions || NULL == pnLines)
    {
        return;
    }
    
    switch(pOptions->type)
    {
        case IP_TYPE:
            sal_memset(buf, 0, DHCP_OPTION_MAX_LEN*2+1);
            sal_sprintf((char *)buf, "option %u ip", pOptions->code);
            cnt = pOptions->data_len/sizeof(struct pal_in4_addr);
            dhcpserver_write_array_config_write(cli,
                (char *)buf, 
                (struct pal_in4_addr *)pOptions->buf,
                cnt,
                pnLines);
            break;
        case ASCII_TYPE:
            cli_out(cli, " option %u ascii %s\n", pOptions->code, pOptions->buf);
            (*pnLines)++;
            break;
        case HEX_TYPE:
            uint8tohexstr(pOptions->buf, pOptions->data_len, (char *)buf);
            cli_out(cli, " option %u hex %s\n", pOptions->code, buf);
            (*pnLines)++;
            break;
        default:
            break;
    }
    
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_option_config_write
 * Purpose: write dhcp pool option config
 * Input: 
 *   cli:  client pointer
 *   pPool: pool pointer
 *   pnLines: writed lines
 * Output: 
 * Return:
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_option_config_write(struct cli *cli, dhcp_pool_t *pPool, int *pnLines)
{
    struct dhcp_option_s *pOptions = (struct dhcp_option_s *)0;
    u_int32_t days = 0;
    u_int32_t hours = 0;
    u_int32_t minutes = 0;
    
    if (NULL == cli || NULL == pPool || NULL == pnLines)
    {
        return;
    }
    
    if(pPool->dhcp_pool_common.domain_name)
    {
        cli_out(cli, " domain-name %s\n", pPool->dhcp_pool_common.domain_name);
        (*pnLines)++;
    }
    
    if(pPool->dhcp_pool_common.bootfile_name)
    {
        cli_out(cli, " bootfile-name %s\n", pPool->dhcp_pool_common.bootfile_name);
        (*pnLines)++;
    }
    
    if(pPool->dhcp_pool_common.dns_server)
    {
        dhcpserver_write_array_config_write(cli, 
            "dns-server", 
            pPool->dhcp_pool_common.dns_server,
            pPool->dhcp_pool_common.n_dns_server,
            pnLines);
    }    
    
    if(pPool->dhcp_pool_common.gateway)
    {
        dhcpserver_write_array_config_write(cli, 
            "gateway", 
            pPool->dhcp_pool_common.gateway,
            pPool->dhcp_pool_common.n_gateway,
            pnLines);
    }
    
    if(pPool->dhcp_pool_common.netbios_server)
    {
        dhcpserver_write_array_config_write(cli, 
            "netbios-name-server", 
            pPool->dhcp_pool_common.netbios_server,
            pPool->dhcp_pool_common.n_netbios_server,
            pnLines);
    }    
    
    if(pPool->dhcp_pool_common.tftp_server_addr)
    {
        dhcpserver_write_array_config_write(cli, 
            "tftp-server-address", 
            pPool->dhcp_pool_common.tftp_server_addr,
            pPool->dhcp_pool_common.n_tftp_server_addr,
            pnLines);
    }    

    if(0 != pPool->dhcp_pool_common.lease_time)
    {
        days = pPool->dhcp_pool_common.lease_time/LEASE_ONE_DAY;
        hours = pPool->dhcp_pool_common.lease_time/LEASE_ONE_HOUR - days*24;
        minutes = pPool->dhcp_pool_common.lease_time/LEASE_ONE_MINUTE - days*24*60 - hours*60;
        cli_out(cli, " lease %lu %lu %lu\n", days, hours, minutes);
        (*pnLines)++;
    }

    if(UNKNOWN_NODE != pPool->dhcp_pool_common.netbios_node_type)
    {
        switch(pPool->dhcp_pool_common.netbios_node_type)
        {
            case B_NODE:
                cli_out(cli, " netbios-node-type b-node\n");
                (*pnLines)++;
                break;
            case P_NODE:
                cli_out(cli, " netbios-node-type p-node\n");
                (*pnLines)++;
                break;
            case M_NODE:
                cli_out(cli, " netbios-node-type m-node\n");
                (*pnLines)++;
                break;
            case H_NODE:
                cli_out(cli, " netbios-node-type h-node\n");
                (*pnLines)++;
                break;
            default:
                break;
        }
    }
    
    pOptions = pPool->dhcp_pool_common.options;
    while(pOptions)
    {
        dhcpserver_pool_option_code_config_write(cli, pOptions, pnLines);
        pOptions = pOptions->next;
    }
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_config_write
 * Purpose: write dhcp pool config
 * Input: 
 *   cli:  client pointer
 *   pnLines: writed lines
 * Output: 
 * Return:
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_config_write(struct cli *cli, int *pnLines)
{
    dhcp_pool_t *pPool = NULL;
    struct hash *hash = g_dhcpserverPoolhash;
    struct hash_backet *hb = NULL;
    int i;
    char ipAddr[16];
    char cid[DHCP_OPTION_MAX_LEN*2+1];
    
    if (NULL == cli || NULL == hash || NULL == pnLines)
    {
        return;
    }
    
    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pPool = NULL;
            pPool = (dhcp_pool_t *)hb->data;
            if (NULL != pPool)
            {
                cli_out(cli, "dhcp pool %s\n", pPool->pool_name);
                (*pnLines)++;
                
                if (staticPool == pPool->pool_type)
                {
                    sal_memset(ipAddr, 0, 16);
                    sal_memset(cid, 0, DHCP_OPTION_MAX_LEN*2+1);
                    sal_inet_ntop(AF_INET, &(pPool->fixIp.u.prefix4), ipAddr, 16);
                    if(pPool->hw_addr)
                    {
                    /* modified by liuyang for print correctly*/
                        sal_sprintf(cid, "%02x%02x.%02x%02x.%02x%02x",
                              pPool->hw_addr[1],
                              pPool->hw_addr[2],
                              pPool->hw_addr[3],
                              pPool->hw_addr[4],
                              pPool->hw_addr[5],
                              pPool->hw_addr[6]);
                        cli_out(cli, " static-bind ip-address %s/%u mac-address %s\n", 
                            ipAddr, pPool->fixIp.prefixlen, cid);
                        (*pnLines)++;
                    }
                    else if(pPool->clientId && DHCP_CLENTID_NONE != pPool->clientId_type)
                    {
                        switch(pPool->clientId_type)
                        {
                            case DHCP_CLENTID_ASCII:
                                sal_memcpy(cid, pPool->clientId, pPool->clientId_len);
                                cli_out(cli, " static-bind ip-address %s/%u client-identifier ascii %s\n",
                                    ipAddr, pPool->fixIp.prefixlen, cid);
                                (*pnLines)++;
                                break;
                            case DHCP_CLENTID_HEX:
                                uint8tohexstr(pPool->clientId, pPool->clientId_len, cid);
                                cli_out(cli, " static-bind ip-address %s/%u client-identifier hex %s\n",
                                    ipAddr, pPool->fixIp.prefixlen, cid);
                                (*pnLines)++;
                                break;
                            default:
                                break;
                        }
                    }
                }
                
                if (dynPool == pPool->pool_type)
                {
                    sal_memset(ipAddr, 0, 16);
                    sal_inet_ntop(AF_INET, &(pPool->network.u.prefix4), ipAddr, 16);
                    cli_out(cli, " network %s/%u\n",
                        ipAddr, pPool->network.prefixlen);
                    (*pnLines)++;
                }
                
                dhcpserver_pool_option_config_write(cli, pPool, pnLines);
            }
        }
    }
    
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_config_write
 * Purpose: write dhcp server global based configuration
 * Input:  
 *   cli: client pointer
 * Output: 
 * Return: 
 *     nLines: cli line count
 * Note: 
 ******************************************************************************/
int dhcpserver_config_write(struct cli *cli)
{
    int nLines = 0;
    
    if (NULL == cli)
    {
        return 0;
    }
    
    /* dhcp server ping */
    if (DEFAULT_PING_PACKET != server_ping_packets)
    {
        cli_out(cli, "dhcp ping packets %u\n", server_ping_packets);
        nLines++;
    }
    
    if (DEFAULT_PING_TIMEOUT != server_ping_timeout)
    {
        cli_out(cli, "dhcp ping timeout %u\n", server_ping_timeout);
        nLines++;
    }
    
    /* dhcp server excluded ip */
    dhcpserver_write_exclude_ip_address(cli, g_dhcpserverExcludeIpList, &nLines);
    
    /* dhcp server pool */
    dhcpserver_pool_config_write(cli, &nLines);
    return nLines;
}
#endif /* ! _CENTEC_ */
#endif

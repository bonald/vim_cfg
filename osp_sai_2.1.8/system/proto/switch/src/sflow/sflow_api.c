/****************************************************************************
*  sflow api process
*
* Modify History:
* Revision      : R0.01
* Author        : Liwh
* Date          : 2017-05-15 11:07
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netinet/ip6.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "switch.h"
#include "proto.h"
#include "param_check.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#include "gen/tbl_sflow_counter_port_define.h"
#include "gen/tbl_sflow_counter_port.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "swth_sys.h"
#include "sflow.h"
#include "glb_macro.h"
/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
extern bool
mirror_discard_sflow_packet(uint32 sessionid);

/****************************************************************************
 *
* Functions
* 
****************************************************************************/
int32
sflow_api_get_sflow_enable(void)
{
    tbl_sflow_global_t  *p_sflow_glb = NULL;

    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    return p_sflow_glb->enable;
}

bool sflow_debug_is_on(uint32 flag)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;

    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return FALSE;
    }
    
    if (GLB_FLAG_ISSET(p_sflow_glb->sflow, flag))
    {
        return TRUE;
    }
    
    return FALSE;
}

void sflow_log_timer_fn(void * p_data)
{
    tbl_sflow_collector_t   *p_db_collector = NULL;
    tbl_sflow_collector_t   *p_collector = NULL;
    
    p_collector = (tbl_sflow_collector_t *)p_data;
    if (p_collector == NULL)
    {
        return;
    }

    if (p_collector->t_log_timer == NULL)
    {
        return;
    }
    
    p_db_collector = tbl_sflow_collector_get_sflow_collector(&(p_collector->key));
    if (p_db_collector == NULL)
    {
        return;
    }

    if (p_db_collector->t_log_timer == NULL)
    {
        return;
    }
    
    CTC_TASK_STOP_TIMER(p_collector->t_log_timer);
    p_collector->t_log_timer = NULL;
    tbl_sflow_collector_set_sflow_collector_field(p_collector, TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER);

}

void sflow_transmit_packet(tbl_sflow_collector_t   *p_db_collector)
{
    struct sflow_datagram_v4_inet *p_sflowpacket4 = NULL;
    struct sflow_datagram_v5_inet *p_sflowpacket5 = NULL;
    struct sal_sockaddr_in4       in4_sock;
    struct sal_sockaddr           *p_dest_sock;
    uint32                        dest_sock_len = 0;
    tbl_sflow_global_t            *p_sflow_glb = NULL;
    uint32                        sequenceno = 0;
    uint32                        uptime = 0;
    uint32                        samplenumber = 0;
    int32                         rc = PM_E_NONE;
    char                          ipbuf[256];
    struct sal_timeval            tv;
    /* modified by liwh for bug 53920, 2019-10-12 */
    struct sal_sockaddr_in6 in6_sock;
    struct sflow_datagram_v4_inet6 *p_sflowpacket4_inet6 = NULL;
    struct sflow_datagram_v5_inet6 *p_sflowpacket5_inet6 = NULL;
    /* liwh end */

    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        SFLOW_LOG_ERR("TBL_SFLOW_GLOBAL is not initialized");
        return;
    }

    if (AF_INET == p_db_collector->key.addr.family)
    {
        /*added by yejl for sflow version 5*/
        if (SFLOW_VERSION5 == p_sflow_glb->version)
        {                
            sal_memset(&in4_sock, 0, sizeof(struct sal_sockaddr_in4));
            in4_sock.sin_family = AF_INET;
            in4_sock.sin_port = sal_hton16 (p_db_collector->key.udp_port);
            sal_memcpy(&(in4_sock.sin_addr.s_addr), &(p_db_collector->key.addr.u.prefix4), sizeof(uint32));
            
            p_dest_sock = (struct sal_sockaddr *)&in4_sock;
            dest_sock_len = sizeof(struct sal_sockaddr_in4);
    
            p_sflowpacket5 = (struct sflow_datagram_v5_inet*)p_db_collector->buf;
            p_sflowpacket5->version = sal_hton32(SFLOW_VERSION5);
            p_sflowpacket5->address_type = sal_hton32(SFLOW_ADDRESS_IP_V4);
            
            sal_memcpy(&(p_sflowpacket5->agentipaddr), &(p_sflow_glb->agent.u.prefix4), sizeof(uint32));
            p_sflowpacket5->sub_agent_id = sal_hton32(0);
            sequenceno = p_db_collector->sequence++;
            p_sflowpacket5->sequence_number = sal_hton32(sequenceno);
    
            ctc_task_update_boottime(&tv);
            uptime = tv.tv_sec * 1000 + + tv.tv_usec / 1000;
            p_sflowpacket5->uptime = sal_hton32(uptime);
            samplenumber = p_sflowpacket5->sample_num;
#ifdef USW
            p_sflowpacket5->sample_num = samplenumber;
#else
            p_sflowpacket5->sample_num = sal_hton32(samplenumber);
#endif
        }
        else
        {        
            sal_memset(&in4_sock, 0, sizeof(struct sal_sockaddr_in4));
            in4_sock.sin_family = AF_INET;
            in4_sock.sin_port = sal_hton16 (p_db_collector->key.udp_port);
            sal_memcpy(&(in4_sock.sin_addr.s_addr), &(p_db_collector->key.addr.u.prefix4), sizeof(uint32));
            
            p_dest_sock = (struct sal_sockaddr *)&in4_sock;
            dest_sock_len = sizeof(struct sal_sockaddr_in4);
    
            p_sflowpacket4 = (struct sflow_datagram_v4_inet*)p_db_collector->buf;
            p_sflowpacket4->version = sal_hton32(SFLOW_VERSION4);
            p_sflowpacket4->address_type = sal_hton32(SFLOW_ADDRESS_IP_V4);
    
            sal_memcpy(&(p_sflowpacket4->agentipaddr), &(p_sflow_glb->agent.u.prefix4), sizeof(uint32));
            sequenceno = p_db_collector->sequence++;
            p_sflowpacket4->sequence_number = sal_hton32(sequenceno);
    
            ctc_task_update_boottime(&tv);
            uptime = tv.tv_sec * 1000 + + tv.tv_usec / 1000;
            p_sflowpacket4->uptime = sal_hton32(uptime);
            samplenumber = p_sflowpacket4->sample_num;
            p_sflowpacket4->sample_num = sal_hton32(samplenumber);
        }
    }
    /* modified by liwh for bug 53920, 2019-10-12 */
    else
    {
        if (SFLOW_VERSION5 == p_sflow_glb->version)
        {                
            sal_memset(&in6_sock, 0, sizeof(struct sal_sockaddr_in6));
            in6_sock.sin6_family = AF_INET6;
            in6_sock.sin6_port = sal_hton16 (p_db_collector->key.udp_port);
            sal_memcpy(&(in6_sock.sin6_addr.s6_addr), &(p_db_collector->key.addr.u.prefix6), IPV6_MAX_BYTELEN);
    
            p_dest_sock = (struct sal_sockaddr *)&in6_sock;
            dest_sock_len = sizeof(struct sal_sockaddr_in6);
    
            p_sflowpacket5_inet6 = (struct sflow_datagram_v5_inet6*)p_db_collector->buf;
            p_sflowpacket5_inet6->version = sal_hton32(SFLOW_VERSION5);
            p_sflowpacket5_inet6->address_type = sal_hton32(SFLOW_ADDRESS_IP_V6);
            
            sal_memcpy(&(p_sflowpacket5_inet6->agentipaddr), &(p_sflow_glb->agent_ipv6.u.prefix6), IPV6_MAX_BYTELEN);
            p_sflowpacket5_inet6->sub_agent_id = sal_hton32(0);
            sequenceno = p_db_collector->sequence++;
            p_sflowpacket5_inet6->sequence_number = sal_hton32(sequenceno);
    
            ctc_task_update_boottime(&tv);
            uptime = tv.tv_sec * 1000 + + tv.tv_usec / 1000;
            p_sflowpacket5_inet6->uptime = sal_hton32(uptime);
            samplenumber = p_sflowpacket5_inet6->sample_num;
#ifdef USW
            p_sflowpacket5_inet6->sample_num = samplenumber;
#else
            p_sflowpacket5_inet6->sample_num = sal_hton32(samplenumber);
#endif
        }
        else
        {        
            sal_memset(&in6_sock, 0, sizeof(struct sal_sockaddr_in6));
            in6_sock.sin6_family = AF_INET6;
            in6_sock.sin6_port = sal_hton16 (p_db_collector->key.udp_port);
            sal_memcpy(&(in6_sock.sin6_addr.s6_addr), &(p_db_collector->key.addr.u.prefix6), IPV6_MAX_BYTELEN);
    
            p_dest_sock = (struct sal_sockaddr *)&in6_sock;
            dest_sock_len = sizeof(struct sal_sockaddr_in6);
    
            p_sflowpacket4_inet6 = (struct sflow_datagram_v4_inet6*)p_db_collector->buf;
            p_sflowpacket4_inet6->version = sal_hton32(SFLOW_VERSION4);
            p_sflowpacket4_inet6->address_type = sal_hton32(SFLOW_ADDRESS_IP_V6);
    
            sal_memcpy(&(p_sflowpacket4_inet6->agentipaddr), &(p_sflow_glb->agent_ipv6.u.prefix6), IPV6_MAX_BYTELEN);
            sequenceno = p_db_collector->sequence++;
            p_sflowpacket4_inet6->sequence_number = sal_hton32(sequenceno);
    
            ctc_task_update_boottime(&tv);
            uptime = tv.tv_sec * 1000 + + tv.tv_usec / 1000;
            p_sflowpacket4_inet6->uptime = sal_hton32(uptime);
            samplenumber = p_sflowpacket4_inet6->sample_num;
            p_sflowpacket4_inet6->sample_num = sal_hton32(samplenumber);
        }            
    }
    /* liwh end */

    rc = sendto(p_db_collector->sendfd, p_db_collector->buf, p_db_collector->pkt_len, 0, 
        p_dest_sock, dest_sock_len);

    cdb_addr_val2str(ipbuf, 256, &p_db_collector->key.addr);
    if (rc < 0)
    {
        if (p_db_collector->t_log_timer == NULL)
        {
            logid_sys(LOG_SFLOW_4_FAIL, ipbuf, p_db_collector->key.udp_port, p_db_collector->sendfd, sal_strerror (errno));
            p_db_collector->t_log_timer = CTC_TASK_ADD_TIME(sflow_log_timer_fn,p_db_collector,SFLOW_LOG_INTERVAL);
            tbl_sflow_collector_set_sflow_collector_field(p_db_collector, TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER);
        }
    }
    if (!rc)
    {
       if (p_db_collector->t_log_timer != NULL)
        { 
            CTC_TASK_STOP_TIMER(p_db_collector->t_log_timer);
            p_db_collector->t_log_timer = NULL;
            tbl_sflow_collector_set_sflow_collector_field(p_db_collector, TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER);
        }
    }
    
    SFLOW_LOG_DEBUG(SFLOW_FLAG_PACKET, "PACKET: Send to %s:%u, seq %u, %u samples\n", 
        ipbuf, p_db_collector->key.udp_port, sequenceno, samplenumber);

    /* reset packet buf and length */
    sal_memset(p_db_collector->buf, 0, p_db_collector->pkt_len);
    p_db_collector->pkt_len = 0;

    return;
}

void sflow_transdelay_timer_fn(void *p_data)
{
    tbl_sflow_collector_t          *p_db_collector = NULL;

    p_db_collector = (tbl_sflow_collector_t*)p_data;

    if(!p_db_collector)
    {
        SFLOW_LOG_ERR("sflow_transdelay_timer_fn input parameter is NULL");
        return;
    }

    p_db_collector->transdelay_timer = NULL;

    /* transmit sflow packet */
    sflow_transmit_packet(p_db_collector);

    return;
}


uint32 sflow_get_next_counter_port(uint32 ifindex)
{
    tbl_sflow_counter_port_master_t  *p_counter_master = tbl_sflow_counter_port_get_master();
    tbl_sflow_counter_port_t         *p_db_sflow_counter_port = NULL;
    ctclib_slistnode_t               *listnode = NULL;  

    /* if not any counter port, return invalid value */
    if (0 == p_counter_master->counter_port_list->count)
    {
        return GLB_INVALID_IFINDEX;
    }
    /* if only one counter port, return the head node */
    else if (1 == p_counter_master->counter_port_list->count)
    {
        p_db_sflow_counter_port = p_counter_master->counter_port_list->head->data;   
        return p_db_sflow_counter_port->key.ifindex;   
    }

    CTCLIB_SLIST_LOOP(p_counter_master->counter_port_list, p_db_sflow_counter_port, listnode)
    {
        if (p_db_sflow_counter_port->key.ifindex <= ifindex)
        {
            continue;    
        }

        return p_db_sflow_counter_port->key.ifindex;
    }

    /* if current node is the last node, return the head node */
    p_db_sflow_counter_port = p_counter_master->counter_port_list->head->data;
    return p_db_sflow_counter_port->key.ifindex;
    
}

void sflow_set_counter_port(uint32  ifindex, uint32  counter_interval)
{
    tbl_sflow_counter_port_t   *p_db_counter_port = NULL;
    tbl_sflow_counter_port_t   counter_port;

    sal_memset(&counter_port, 0, sizeof(tbl_sflow_counter_port_t));

    /* find counter port */
    counter_port.key.ifindex = ifindex;
    counter_port.left_interval = counter_interval* GLB_SFLOW_TIME_SEC2MSEC; 
    p_db_counter_port = tbl_sflow_counter_port_get_sflow_counter_port(&counter_port.key);

    /* set or create counter port */
    if (p_db_counter_port)
    {
        tbl_sflow_counter_port_set_sflow_counter_port_field(&counter_port, 
            TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL);
    }
    else
    {
        tbl_sflow_counter_port_add_sflow_counter_port(&counter_port);
    }
    
    return;
}

uint32 sflow_if_direction(tbl_interface_t  *p_db_if)
{
    uint32 ifdirection = 0;
    
    /* Derived from MAU MIB (RFC 2668)
       0 = unknown, 1 = full-duplex,
       2 = half-duplex, 3 = in, 4 = out */ 
     /*for bug 24510 modified by liuyang 2013-9-2 */
    switch(p_db_if->duplex)
    {
    case GLB_DUPLEX_AUTO:
        switch(p_db_if->oper_duplex)
        {
        case GLB_DUPLEX_HALF:
            ifdirection = 2;
            break;
        case GLB_DUPLEX_FULL:
            ifdirection = 1;
            break;
        default :
            ifdirection = 0;
            break;
        }
        break;

    case GLB_DUPLEX_HALF:
        ifdirection = 2;
        break;
    case GLB_DUPLEX_FULL:
        ifdirection = 1;
        break;
    default :
        ifdirection = 0;
        break;
    }
      
    return ifdirection;
}

uint32 sflow_if_status(tbl_interface_t  *p_db_if)
{    
    uint32 ifstatus = 0;
    
    if (tbl_interface_is_up(p_db_if))
    {
        GLB_SET_FLAG(ifstatus, SFLOW_IFSTATUS_ADMIN);
    }

    if (tbl_interface_is_running(p_db_if))
    {
        GLB_SET_FLAG(ifstatus, SFLOW_IFSTATUS_OPER);
    }
    
    return ifstatus;
}

uint32 sflow_iftype_map(tbl_interface_t  *p_db_if)
{
    if (p_db_if->flags & GLB_IFF_POINTOPOINT)
    {
        return SNMP_INTERFACE_PPP;
    }
    
    switch (p_db_if->hw_type)
    {
        case GLB_IF_TYPE_LINKAGG_IF:
            return SNMP_INTERFACE_AGG;

        case GLB_IF_TYPE_VLAN_IF:
            return SNMP_INTERFACE_VLANIF;

        case GLB_IF_TYPE_TUNNEL_IF:
            return SNMP_INTERFACE_TUNNEL;

        /*case IF_TYPE_FRELAY:
            return NSM_SNMP_INTERFACE_FRAMERELAY;*/
            
        default:
            return SNMP_INTERFACE_ETHERNETCSMACD;
    }
}

/* add by yejl for sflow version 5*/
uint64_t sflow_htonll(uint64_t x)
{
    uint64_t tmp = x;
    char *s = (char *)&tmp;
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        s[i] = (uint8_t)(x >> (56 - 8 * i));
    }
    
    return tmp;
}


void sflow_counter_output(uint32 ifindex)
{
    tbl_interface_t                *p_db_if = NULL;
    tbl_interface_t                *p_db_if_lag = NULL;
    tbl_port_stats_t               *p_db_port_stats = NULL;
    tbl_sflow_counter_port_t       *p_db_sflow_counter_port = NULL;
    uint32                         sequence = 0;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    struct sflow_datagram_v4_inet  *p_sflowpacket4 = NULL;
    struct sflow_datagram_v5_inet  *p_sflowpacket5 = NULL;
    uint32                         *size = NULL;
    uint8                          *pnt = NULL;
    tbl_sflow_global_t             *p_sflow_glb = NULL;
    uint32                         temp = 0;
    uint64                         speed = 0;
    uint32                         sample_num = 0;
    uint64                         octets_rcv = 0;
    /* modified by liwh for bug 53920, 2019-10-12 */
    struct sflow_datagram_v4_inet6 *p_sflowpacket4_inet6 = NULL;
    struct sflow_datagram_v5_inet6 *p_sflowpacket5_inet6 = NULL;
    /* liwh end */
    
    SFLOW_LOG_DEBUG(SFLOW_FLAG_COUNTER, "sflow_counter_output ifindex %u\n", ifindex);

    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        SFLOW_LOG_ERR("TBL_SFLOW_GLOBAL is not initialized");
        return;
    }

    /* get collector */
    p_glb_collector = tbl_sflow_collector_get_master();
    if (!p_glb_collector)
    {
        SFLOW_LOG_ERR("TBL_SFLOW_COLLECTOR is not initialized");
        return;
    }
    
    /* get sflow counter port */
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if)
    {
        return;
    }

    /* get port stats */
    p_db_port_stats = tbl_port_stats_get_port_stats((tbl_port_stats_key_t*)&ifindex);
    if (!p_db_port_stats)
    {
        SFLOW_LOG_ERR("Sflow can't get stats from ifindex %d", ifindex);
        return;
    }

    /* get sflow counter port */
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        char ifname[IFNAME_SIZE] = {0};
        sal_sprintf(ifname, "agg%u", p_db_if->lag->lag_id);
        p_db_if_lag = tbl_interface_get_interface_by_name(ifname);
        if (NULL == p_db_if_lag)
        {
            
            return;
        }
        p_db_sflow_counter_port = tbl_sflow_counter_port_get_sflow_counter_port((tbl_sflow_counter_port_key_t*)&(p_db_if_lag->ifindex));
        if (!p_db_sflow_counter_port)
        {
            SFLOW_LOG_ERR("Sflow can't get sflow counter port from ifindex %d", ifindex);
            return;
        }
        sequence = p_db_sflow_counter_port->sequence++;
    }
    else
    {
        p_db_sflow_counter_port = tbl_sflow_counter_port_get_sflow_counter_port((tbl_sflow_counter_port_key_t*)&ifindex);
        if (!p_db_sflow_counter_port)
        {
            SFLOW_LOG_ERR("Sflow can't get sflow counter port from ifindex %d", ifindex);
            return;
        }
        sequence = p_db_sflow_counter_port->sequence++;
    }

    /* loop all collector */
    CTCLIB_SLIST_LOOP(p_glb_collector->sflow_collector_list, p_db_collector, listnode)
    {
        struct sflow_packet_counter_sample_expanded counter_sample_exp;
        struct sflow_packet_if_couters generic_if_counters;
        
        /* if add next packet, the buf is not enough, send packet */

        if (SFLOW_VERSION5 == p_sflow_glb->version)
        {
            if (p_db_collector->pkt_len + SFLOW_COUNTER_SAMPLE_SIZE_VERSION5 > GLB_SFLOW_PKT_BUF)
            {
                sflow_transmit_packet(p_db_collector);    
            }
        }
        else
        {
            if (p_db_collector->pkt_len + SFLOW_COUNTER_SAMPLE_SIZE > GLB_SFLOW_PKT_BUF)
            {
                sflow_transmit_packet(p_db_collector);    
            }
        }
        
        if (AF_INET == p_db_collector->key.addr.family)
        {
            /* reserve sflow packet header */
            if (0 == p_db_collector->pkt_len)
            {
                if (SFLOW_VERSION5 == p_sflow_glb->version)
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v5_inet);
                }
                else
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v4_inet);
                }  
            }
#ifndef _GLB_UML_SYSTEM_  
#ifndef USW
            /* set sflow packet sample num */
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5 = (struct sflow_datagram_v5_inet*)p_db_collector->buf;
                sample_num = p_sflowpacket5->sample_num;
                sample_num++;
                p_sflowpacket5->sample_num = sal_hton32(sample_num);
            }
            else
            {
                p_sflowpacket4 = (struct sflow_datagram_v4_inet*)p_db_collector->buf;
                sample_num = p_sflowpacket4->sample_num;
                sample_num++;
                p_sflowpacket4->sample_num = sal_hton32(sample_num);
            }
#else
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5 = (struct sflow_datagram_v5_inet*)p_db_collector->buf;
                sample_num = sal_ntoh32(p_sflowpacket5->sample_num);
                sample_num++;
                p_sflowpacket5->sample_num = sal_hton32(sample_num);
            }
            else
            {
                p_sflowpacket4 = (struct sflow_datagram_v4_inet*)p_db_collector->buf;
                sample_num = sal_ntoh32(p_sflowpacket4->sample_num);
                sample_num++;
                p_sflowpacket4->sample_num = sal_hton32(sample_num);
            }
#endif
#else
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5 = (struct sflow_datagram_v5_inet*)p_db_collector->buf;
                sample_num = p_sflowpacket5->sample_num;
                sample_num++;
                p_sflowpacket5->sample_num = sample_num;
            }
            else
            {
                p_sflowpacket4 = (struct sflow_datagram_v4_inet*)p_db_collector->buf;
                sample_num = p_sflowpacket4->sample_num;
                sample_num++;
                p_sflowpacket4->sample_num = sample_num;
            }
#endif
        }
        /* modified by liwh for bug 53920, 2019-10-12 */
        else
        {
            /* reserve sflow packet header */
            if (0 == p_db_collector->pkt_len)
            {
                if (SFLOW_VERSION5 == p_sflow_glb->version)
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v5_inet6);
                }
                else
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v4_inet6);
                }  
            }
#ifndef _GLB_UML_SYSTEM_  
#ifndef USW
            /* set sflow packet sample num */
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5_inet6= (struct sflow_datagram_v5_inet6*)p_db_collector->buf;
                sample_num = p_sflowpacket5_inet6->sample_num;
                sample_num++;
                p_sflowpacket5_inet6->sample_num = sal_hton32(sample_num);
            }
            else
            {
                p_sflowpacket4_inet6= (struct sflow_datagram_v4_inet6*)p_db_collector->buf;
                sample_num = p_sflowpacket4_inet6->sample_num;
                sample_num++;
                p_sflowpacket4_inet6->sample_num = sal_hton32(sample_num);
            }
#else
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5_inet6 = (struct sflow_datagram_v5_inet6*)p_db_collector->buf;
                sample_num = sal_ntoh32(p_sflowpacket5_inet6->sample_num);
                sample_num++;
                p_sflowpacket5_inet6->sample_num = sal_hton32(sample_num);
            }
            else
            {
                p_sflowpacket4_inet6 = (struct sflow_datagram_v4_inet6*)p_db_collector->buf;
                sample_num = sal_ntoh32(p_sflowpacket4_inet6->sample_num);
                sample_num++;
                p_sflowpacket4_inet6->sample_num = sal_hton32(sample_num);
            }
#endif
#else
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {
                p_sflowpacket5_inet6 = (struct sflow_datagram_v5_inet6*)p_db_collector->buf;
                sample_num = p_sflowpacket5_inet6->sample_num;
                sample_num++;
                p_sflowpacket5_inet6->sample_num = sample_num;
            }
            else
            {
                p_sflowpacket4_inet6 = (struct sflow_datagram_v4_inet6*)p_db_collector->buf;
                sample_num = p_sflowpacket4_inet6->sample_num;
                sample_num++;
                p_sflowpacket4_inet6->sample_num = sample_num;
            }
#endif            
        }
        /* liwh end */
        
        pnt = p_db_collector->buf;
        pnt += p_db_collector->pkt_len;
        size = &(p_db_collector->pkt_len);

        if (SFLOW_VERSION5 == p_sflow_glb->version)
        {
            /* counter sample expanded 1 */
            SFLOW_ENCODE_PUTL(SFLOW_COUNTER_SAMPLE_EXPANDED);
            SFLOW_ENCODE_PUTL(sizeof(struct sflow_packet_if_couters) + 8 + sizeof(struct sflow_packet_counter_sample_expanded));
            sal_memset(&counter_sample_exp, 0, sizeof(counter_sample_exp));
            counter_sample_exp.sequence_number = sal_hton32(sequence);
            counter_sample_exp.source_id_type = 0;
            counter_sample_exp.source_id_index = sal_hton32(ifindex & 0x00FFFFFF);
            counter_sample_exp.counter_records_num = sal_hton32(1); // record num : generic if counters, ethernet if counters
            SFLOW_ENCODE_PUT(&counter_sample_exp, sizeof(struct sflow_packet_counter_sample_expanded));

            /* Generic Interface Counters */
            SFLOW_ENCODE_PUTL(SFLOW_COUNTER_GENERIC);
            SFLOW_ENCODE_PUTL(sizeof(struct sflow_packet_if_couters));
            sal_memset(&generic_if_counters, 0, sizeof(generic_if_counters));
            generic_if_counters.ifIndex = sal_hton32(ifindex);
            generic_if_counters.ifType = sal_hton32(sflow_iftype_map(p_db_if));
            generic_if_counters.ifSpeed = sflow_htonll((p_db_if->oper_bandwidth) * 1000ULL);
            generic_if_counters.ifDirection = sal_hton32(sflow_if_direction(p_db_if));
            generic_if_counters.ifStatus = sal_hton32(sflow_if_status(p_db_if));

            /*modified by yangl for bug 54119*/
            if (p_db_if->ifg_stats_en)
            {
                octets_rcv = p_db_port_stats->octets_rcv + p_db_port_stats->pkts_rcv*GLB_DEF_IPG_LEN;
            }
            else
            {
                octets_rcv = p_db_port_stats->octets_rcv;
            }
            generic_if_counters.ifInOctets = sflow_htonll(octets_rcv);
            /*yangl end*/
            generic_if_counters.ifInUcastPkts = sal_hton32(p_db_port_stats->uc_pkts_rcv);
            generic_if_counters.ifInMulticastPkts = sal_hton32(p_db_port_stats->mc_pkts_rcv);
            generic_if_counters.ifInBroadcastPkts = sal_hton32(p_db_port_stats->brdc_pkts_rcv);
            generic_if_counters.ifInDiscards = 0;    /* ifInDiscards not support */
            generic_if_counters.ifInErrors = sal_hton32(p_db_port_stats->mac_rcv_error + p_db_port_stats->bad_crc);
            generic_if_counters.ifInUnknownProtos = 0;   /* ifInUnknownProtos not support */
            generic_if_counters.ifOutOctets = sflow_htonll(p_db_port_stats->octets_send);
            generic_if_counters.ifOutUcastPkts = sal_hton32(p_db_port_stats->uc_pkts_send);
            generic_if_counters.ifOutMulticastPkts = sal_hton32(p_db_port_stats->mc_pkts_send);
            generic_if_counters.ifOutBroadcastPkts = sal_hton32(p_db_port_stats->brdc_pkts_send);
            generic_if_counters.ifOutDiscards = 0;    /* ifOutDiscards not support */
            generic_if_counters.ifOutErrors = sal_hton32(p_db_port_stats->mac_transmit_err);
            if (p_db_if->flags & GLB_IFF_PROMISC)
            {
                generic_if_counters.ifPromiscuousMode = sal_hton32(SNMP_PROMISC_TRUE);
            }
            else
            {
                generic_if_counters.ifPromiscuousMode = sal_hton32(SNMP_PROMISC_FALSE); 
            }
            SFLOW_ENCODE_PUT(&generic_if_counters, sizeof(struct sflow_packet_if_couters));
        }
        else
        {
            /* encode sflow counter packet */
            SFLOW_ENCODE_PUTL(SFLOW_COUNTER_SAMPLE);
            SFLOW_ENCODE_PUTL(sequence);
            SFLOW_ENCODE_PUTL(ifindex & 0x00FFFFFF);
            SFLOW_ENCODE_PUTL(p_sflow_glb->counter_interval);
            SFLOW_ENCODE_PUTL(SFLOW_COUNTER_GENERIC);
            SFLOW_ENCODE_PUTL(ifindex);

            temp = sflow_iftype_map(p_db_if);
            SFLOW_ENCODE_PUTL(temp);

            speed = sflow_htonll((p_db_if->oper_bandwidth) * 1000ULL);
            SFLOW_ENCODE_PUT64(speed);
            
            SFLOW_ENCODE_PUTL(sflow_if_direction(p_db_if));
            SFLOW_ENCODE_PUTL(sflow_if_status(p_db_if));

            SFLOW_ENCODE_PUT64(p_db_port_stats->octets_rcv);

            temp = p_db_port_stats->pkts_rcv - p_db_port_stats->brdc_pkts_rcv - p_db_port_stats->mc_pkts_rcv;
            SFLOW_ENCODE_PUTL(temp);

            temp = p_db_port_stats->mc_pkts_rcv;
            SFLOW_ENCODE_PUTL(temp);

            temp = p_db_port_stats->brdc_pkts_rcv;
            SFLOW_ENCODE_PUTL(temp);

            SFLOW_ENCODE_PUTL(0); /* ifInDiscards not support */

            /* modified by liwh for bug 44460, 2017-06-26 
              input_error should be same as show interface */
            //temp = p_db_port_stats->mac_rcv_error;
            temp = p_db_port_stats->mac_rcv_error + p_db_port_stats->bad_crc;
            /* liwh end */
            SFLOW_ENCODE_PUTL(temp);        

            SFLOW_ENCODE_PUTL(0); /* ifInUnknownProtos not support */

            SFLOW_ENCODE_PUT64(p_db_port_stats->octets_send);

            temp = p_db_port_stats->pkts_send - p_db_port_stats->mc_pkts_send - p_db_port_stats->brdc_pkts_send;
            SFLOW_ENCODE_PUTL(temp);

            temp = p_db_port_stats->mc_pkts_send;        
            SFLOW_ENCODE_PUTL(temp);

            temp = p_db_port_stats->brdc_pkts_send;
            SFLOW_ENCODE_PUTL(temp);

            SFLOW_ENCODE_PUTL(0);    /* ifOutDiscards not support */

            temp = p_db_port_stats->mac_transmit_err;
            SFLOW_ENCODE_PUTL(temp);

            if (p_db_if->flags & GLB_IFF_PROMISC)
            {
                temp = SNMP_PROMISC_TRUE;
            }
            else
            {
                temp = SNMP_PROMISC_FALSE;    
            }
            SFLOW_ENCODE_PUTL(temp);  
        }
        
        /* start transdelay timer */
        if (!p_db_collector->transdelay_timer)
        {
            p_db_collector->transdelay_timer = ctc_task_add_timer(TASK_PRI_NORMAL, sflow_transdelay_timer_fn, 
                p_db_collector, SFLOW_TRANSMIT_DELAY);
            if (!p_db_collector->transdelay_timer)
            {
                SFLOW_LOG_ERR("Start sflow transdelay timer fail");
                return;
            }
        }
    }
    
    return;    
}

#define SFLOW_UPDATE_STATS_TIME     200

void sflow_update_counter_timer_fn(void *p_data)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_sflow_global_t *p_sflow_glb = tbl_sflow_global_get_sflow_global();

    p_sys_glb->sflow_counter_ifindex = p_sflow_glb->counter_next_port;

    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX);

    return;
}

void sflow_counter_timer_fn(void *p_data)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;
    uint32             timer = 0;
    int32              update_stats_timer = 0;
    tbl_interface_t    *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;

    p_sflow_glb = (tbl_sflow_global_t*)p_data;
    if (!p_sflow_glb)
    {
        SFLOW_LOG_ERR("TBL_SFLOW_GLOBAL is not initialized");
        return;
    }

    p_sflow_glb->counter_timer = NULL;

    /* if counter next port is invalid, don't continue */
    if (GLB_INVALID_IFINDEX == p_sflow_glb->counter_next_port)
    {
        return;    
    }

    /* get stats from next port and send sflow packet */
    sflow_counter_output(p_sflow_glb->counter_next_port);
    p_db_if = tbl_interface_get_interface_by_ifindex(p_sflow_glb->counter_next_port);
    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
             //sflow_set_counter_port(p_if_mem->ifindex, p_sflow_glb->counter_interval);
             sflow_counter_output(p_if_mem->ifindex); 
        }
    } 
    
    /* get next interface */
    p_sflow_glb->counter_next_port = sflow_get_next_counter_port(p_sflow_glb->counter_next_port);

    /* calculate counter timer interval */
    timer = GLB_SFLOW_TIME_SEC2MSEC * p_sflow_glb->counter_interval / p_sflow_glb->counter_port_num;
    update_stats_timer = timer - SFLOW_UPDATE_STATS_TIME;
    if (update_stats_timer > 0)
    {
        ctc_task_add_timer_msec(TASK_PRI_NORMAL, sflow_update_counter_timer_fn, NULL, update_stats_timer);
    }
    else
    {
        sflow_update_counter_timer_fn(NULL);
    }
    
    /* start timer for new interval */
    p_sflow_glb->counter_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, sflow_counter_timer_fn, p_sflow_glb, timer);
    if (!p_sflow_glb->counter_timer)
    {
        SFLOW_LOG_ERR("Start sflow counter timer fail");
    }
}

int32
sflow_api_counter_sampling_enable(tbl_interface_t *p_db_if, uint32 enable)
{
    tbl_sflow_global_t         *p_sflow_glb = NULL;
    uint32                     timer = 0;
    tbl_sflow_counter_port_t   *p_db_counter_port = NULL;
    tbl_sflow_counter_port_t   counter_port;
    
    sal_memset(&counter_port, 0, sizeof(tbl_sflow_counter_port_t));
    
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    if (enable)
    {
        /* create counter port node */
        sflow_set_counter_port(p_db_if->ifindex, p_sflow_glb->counter_interval);

        /* if counter timer is running, stop timer */
        if (p_sflow_glb->counter_timer)
        {
            ctc_task_delete_timer(p_sflow_glb->counter_timer);
            p_sflow_glb->counter_timer = NULL;
        }
        
        /* counter sampling for new interface and send sflow packet */
        p_sflow_glb->counter_next_port = p_db_if->ifindex;
        sflow_counter_output(p_sflow_glb->counter_next_port);

        /* add counter port num */
        p_sflow_glb->counter_port_num++;
        
        /* get next interface */
        p_sflow_glb->counter_next_port = sflow_get_next_counter_port(p_db_if->ifindex);

        /* calculate counter timer interval */
        timer = GLB_SFLOW_TIME_SEC2MSEC * p_sflow_glb->counter_interval / p_sflow_glb->counter_port_num;
        
        /* start timer for new interval */
        p_sflow_glb->counter_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, sflow_counter_timer_fn, p_sflow_glb, timer);
        if (!p_sflow_glb->counter_timer)
        {
            SFLOW_LOG_ERR("Start sflow counter timer fail");
        }
    }
    else
    {
        counter_port.key.ifindex = p_db_if->ifindex;
        p_db_counter_port = tbl_sflow_counter_port_get_sflow_counter_port(&counter_port.key);
        if (!p_db_counter_port)
        {
             SFLOW_LOG_ERR("Counter sampling disable fail for ifintx %d", p_db_if->ifindex);
             return PM_E_INVALID_PARAM;
        }
        
        /* reduce counter port num */
        p_sflow_glb->counter_port_num--;
        
        /* remove counter port */
        tbl_sflow_counter_port_del_sflow_counter_port(&counter_port.key);

        /* check next port is not this port */
        if (p_sflow_glb->counter_next_port != p_db_if->ifindex)
        {
            return PM_E_INVALID_PARAM;    
        }
         
        /* if counter timer is running, stop timer */
        if (p_sflow_glb->counter_timer)
        {
            ctc_task_delete_timer(p_sflow_glb->counter_timer);
            p_sflow_glb->counter_timer = NULL;
        }

        /* if there is no other counter port, set next port to invalid port */
        if (0 == p_sflow_glb->counter_port_num)
        {
            p_sflow_glb->counter_next_port = GLB_INVALID_IFINDEX;
            return PM_E_INVALID_PARAM;    
        }

        /* change timer and select next port */
        if (p_sflow_glb->counter_next_port == p_db_if->ifindex)
        {
            /* get next port from head node */
            p_sflow_glb->counter_next_port = sflow_get_next_counter_port(0);

            /* calculate counter timer interval */
            timer = GLB_SFLOW_TIME_SEC2MSEC * p_sflow_glb->counter_interval / p_sflow_glb->counter_port_num;
            
            /* start timer for new interval */
            p_sflow_glb->counter_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, sflow_counter_timer_fn, p_sflow_glb, 
                timer);
            if (!p_sflow_glb->counter_timer)
            {
                SFLOW_LOG_ERR("Start sflow counter timer fail");
            }
        }
    }

    return PM_E_NONE;
}

uint32 sflow_get_pkt_vid(ctc_msg_t *p_msg)
{
    eth_hdr_t                   *eth = NULL;
    sflow_vlan_hdr_t         *vlan = NULL;
    
    if (!p_msg || !p_msg->data)
    {
        return 0;    
    }

    eth = (eth_hdr_t*)p_msg->data;
#ifndef _GLB_UML_SYSTEM_
    if (sal_htons(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        return sal_htons(vlan->tag);
    }
    #else
   if (sal_htons(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        return (vlan->tag);
    }
    
    #endif

    return 0;
}

/* added by yejl for sflow version 5*/
int32 
sflow_get_mac(ctc_msg_t *p_msg, uint8_t *dst_mac, uint8_t *src_mac)
{
    eth_hdr_t                *eth = NULL;
   
    if (!p_msg || !p_msg->data)
    {
        return 0;    
    }

    eth = (eth_hdr_t*)p_msg->data;
    
    sal_memcpy(dst_mac, eth->dest_mac, sizeof(eth->dest_mac));
    sal_memcpy(src_mac, eth->src_mac, sizeof(eth->src_mac));
    return 0;
}


int32
sflow_get_expand_route_data(ctc_msg_t *p_msg, struct sflow_flow_extended_router_data *tmp_flow_extended_router_data)//add by yangl
{
     struct sflow_flow_extended_router_data * tmp_data=NULL;
     //uint32 tmp_nexthop=0;
     if (!p_msg || !p_msg->data)
    {
        return 0;    
    }
     uint32 extended_router_data_len = p_msg->data_len - sizeof(struct sflow_flow_extended_router_data);
     tmp_data=(struct sflow_flow_extended_router_data *)(p_msg->data+extended_router_data_len);
     tmp_flow_extended_router_data->IP_type = sal_ntoh32(SFLOW_IP_V4); 
     tmp_flow_extended_router_data->nexthop = tmp_data->nexthop;
     tmp_flow_extended_router_data->src_mask = sal_ntoh32(tmp_data->src_mask);
     tmp_flow_extended_router_data->dst_mask = sal_ntoh32(tmp_data->dst_mask);
     return 0;
}

int32
sflow_get_ip_v4_data(ctc_msg_t *p_msg, struct sflow_flow_sample_ipv4_data * tmp_flow_sample_ipv4_data)
{
    ip_v4_hdr                           *ip_hdr=NULL;
    eth_hdr_t                           *eth = NULL;
    sflow_vlan_hdr_t                  *vlan = NULL;
    sflow_udp_hdr                    *udp_hdr=NULL;
    sflow_tcp_hdr                    *tcp_hdr = NULL;
    uint32                                u32 = 0;
    if (!p_msg || !p_msg->data)
    {
        return 0;    
    }
    eth = (eth_hdr_t*)p_msg->data;
    #ifndef _GLB_UML_SYSTEM_
    #ifndef USW
    if (sal_hton32(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        ip_hdr=(ip_v4_hdr  *)(vlan+1);
        if (SFLOW_PROTO_UDP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);  
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->src_port = (udp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (udp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
        else if (SFLOW_PROTO_TCP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            u32 = (tcp_hdr->hdrlen_flags)&0x3f;
            tmp_flow_sample_ipv4_data->tcp_flags = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->src_port = (tcp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (tcp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
        else
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            tmp_flow_sample_ipv4_data->src_port =0;
            tmp_flow_sample_ipv4_data->dst_port = 0; 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
    }
    else
    {
         ip_hdr=(ip_v4_hdr  *)(eth+1);
         if (SFLOW_PROTO_UDP == ip_hdr->protocol)
         {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->src_port = (udp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (udp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
         }
        else if (SFLOW_PROTO_TCP ==ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            u32 = (tcp_hdr->hdrlen_flags)&0x3f;
            tmp_flow_sample_ipv4_data->tcp_flags = GLB_CTC_SWAP32(u32);  
            tmp_flow_sample_ipv4_data->src_port = (tcp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (tcp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
        else
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            tmp_flow_sample_ipv4_data->src_port =0;
            tmp_flow_sample_ipv4_data->dst_port = 0; 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
      }
    #else
    uint16 u16_1 = 0;
    uint16 u16_2 = 0;
    uint16 u16_3 = 0;
    if (sal_hton32(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        ip_hdr=(ip_v4_hdr  *)(vlan+1);
        if (SFLOW_PROTO_UDP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);  
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->src_port = (udp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (udp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
        else if (SFLOW_PROTO_TCP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            u32 = (tcp_hdr->hdrlen_flags)&0x3f;
            tmp_flow_sample_ipv4_data->tcp_flags = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->src_port = (tcp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port = (tcp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
        else
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            tmp_flow_sample_ipv4_data->src_port =0;
            tmp_flow_sample_ipv4_data->dst_port = 0; 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32(ip_hdr->total_length);
        }
    }
    else
    {
         ip_hdr=(ip_v4_hdr  *)(eth+1);
         if (SFLOW_PROTO_UDP == ip_hdr->protocol)
         {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            u16_1 = sal_ntoh16(udp_hdr->src_Port_id);
            u16_2 = sal_ntoh16(udp_hdr->dst_Port_id);
            u16_3 = sal_ntoh16(ip_hdr->total_length);
            tmp_flow_sample_ipv4_data->src_port = GLB_CTC_SWAP32((uint32)u16_1);
            tmp_flow_sample_ipv4_data->dst_port = GLB_CTC_SWAP32((uint32)u16_2);
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32((uint32)u16_3);
         }
        else if (SFLOW_PROTO_TCP ==ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            u32 = (tcp_hdr->hdrlen_flags)&0x3f;
            tmp_flow_sample_ipv4_data->tcp_flags = GLB_CTC_SWAP32(u32); 
            u16_1 = sal_ntoh16(tcp_hdr->src_Port_id);
            u16_2 = sal_ntoh16(tcp_hdr->dst_Port_id);
            u16_3 = sal_ntoh16(ip_hdr->total_length);
            tmp_flow_sample_ipv4_data->src_port = GLB_CTC_SWAP32((uint32)u16_1);
            tmp_flow_sample_ipv4_data->dst_port = GLB_CTC_SWAP32((uint32)u16_2);
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32((uint32)u16_3);
        }
        else
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr);
            u32 = ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos = GLB_CTC_SWAP32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            tmp_flow_sample_ipv4_data->src_port =0;
            tmp_flow_sample_ipv4_data->dst_port = 0; 
            tmp_flow_sample_ipv4_data->protocol = sal_ntoh32(ip_hdr->protocol);
            u16_3 = sal_ntoh16(ip_hdr->total_length);
            tmp_flow_sample_ipv4_data->length = sal_ntoh32((uint32)u16_3);
        }
      }
    #endif
    #else
    if (sal_hton32(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        ip_hdr=(ip_v4_hdr  *)(vlan+1);
        if (SFLOW_PROTO_UDP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip= (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            tmp_flow_sample_ipv4_data->tcp_flags = 0;
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->src_port=GLB_CTC_SWAP32(udp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port=GLB_CTC_SWAP32(udp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
        }
        else if (SFLOW_PROTO_TCP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip= (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip=( ip_hdr->dst_ip_addr);
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->tcp_flags=((sal_ntoh16(tcp_hdr->hdrlen_flags))&0x3f);  
            tmp_flow_sample_ipv4_data->src_port=GLB_CTC_SWAP32(tcp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port=GLB_CTC_SWAP32(tcp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
         }
         else if (SFLOW_PROTO_ICMP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip= (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip=( ip_hdr->dst_ip_addr);
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags=0;  
            tmp_flow_sample_ipv4_data->src_port=0;
            tmp_flow_sample_ipv4_data->dst_port=0; 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
         }
    }
    else
    {
        ip_hdr=(ip_v4_hdr  *)(eth+1);
        if (SFLOW_PROTO_UDP == ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip= (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip=( ip_hdr->dst_ip_addr); 
            tmp_flow_sample_ipv4_data->tcp_flags=(0);
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            udp_hdr=(sflow_udp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->src_port=GLB_CTC_SWAP32(udp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port=GLB_CTC_SWAP32(udp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
        }
        else if (SFLOW_PROTO_TCP ==ip_hdr->protocol)
        {
            tmp_flow_sample_ipv4_data->src_ip = (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip = ( ip_hdr->dst_ip_addr); 
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            tcp_hdr=(sflow_tcp_hdr *)(ip_hdr+1);
            tmp_flow_sample_ipv4_data->tcp_flags=((sal_ntoh16(tcp_hdr->hdrlen_flags))&0x3f);
            tmp_flow_sample_ipv4_data->src_port=GLB_CTC_SWAP32(tcp_hdr->src_Port_id);
            tmp_flow_sample_ipv4_data->dst_port=GLB_CTC_SWAP32(tcp_hdr->dst_Port_id); 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
        }
        else
        {
            tmp_flow_sample_ipv4_data->src_ip= (ip_hdr->src_ip_addr);
            tmp_flow_sample_ipv4_data->dst_ip=( ip_hdr->dst_ip_addr);
            u32=ip_hdr->tp_service;
            tmp_flow_sample_ipv4_data->tos=sal_ntoh32(u32);
            tmp_flow_sample_ipv4_data->tcp_flags=0;  
            tmp_flow_sample_ipv4_data->src_port=0;
            tmp_flow_sample_ipv4_data->dst_port=0; 
            tmp_flow_sample_ipv4_data->protocol=sal_ntoh32(ip_hdr->protocol);
            tmp_flow_sample_ipv4_data->length=sal_ntoh32(ip_hdr->total_length);
         }
    }

    #endif  
    return 0;
}

int32 
sflow_if_in_vlan_l3(uint32 vid, tbl_interface_t  *p_db_if, ctc_msg_t *p_msg)
{
    tbl_vlan_key_t                  key;
    tbl_vlan_t                      *p_db_vlan = NULL;
    tbl_interface_t                 *p_db_vlanif = NULL;
    char                                vlan_ifname[IFNAME_SIZE];
    struct sflow_flow_extended_router_data extended_router_data;
    uint32                              nexthop_ip = 0;    
    
    key.vid = (vid&0x0FFF);
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        return FALSE;
    }
    if (!GLB_BMP_ISSET(p_db_vlan->member_port, p_db_if->ifindex)&&(GLB_VLAN_PORT_TYPE_ACCESS != p_db_if->vlan_type))
    {
        return FALSE;
    }
    sal_sprintf(vlan_ifname,GLB_IFNAME_VLAN_PREFIX"%d", vid);
    p_db_vlanif = tbl_interface_get_interface_by_name(vlan_ifname);
    if (NULL == p_db_vlanif)
    {
        return FALSE;
    }
    sal_memset(&extended_router_data, 0, sizeof(extended_router_data));
    sflow_get_expand_route_data(p_msg, &extended_router_data);
    nexthop_ip = (uint32)extended_router_data.nexthop.s_addr;
    return (nexthop_ip) ? TRUE : FALSE;
}

int32 
sflow_get_l3_flag(ctc_msg_t * p_msg)//add by yangl
{
    uint32                    ifindex =0;
    uint32                    pkt_vid=0;
    tbl_route_if_t         *p_db_rtif=NULL;//add by yangl
    tbl_interface_t        *p_db_if = NULL;
    tbl_sys_global_t             * p_sys_glb = NULL;
    uint32                         rc = 0;
    struct sflow_flow_expanded_frame_data flow_expanded_frame_data;

    if (!p_msg || !p_msg->data)
    {
        return FALSE;    
    }
    p_sys_glb = tbl_sys_global_get_sys_global();
    sflow_get_mac(p_msg, flow_expanded_frame_data.dst_mac, flow_expanded_frame_data.src_mac);
    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if)
    {
        return FALSE;    
    }
    if (GLB_SFLOW_INGRESS_PACKET == p_msg->u_hdr.pkt.sub_type)
    {
        rc = sal_memcmp(flow_expanded_frame_data.dst_mac,(uint8 *)p_sys_glb->route_mac,6);
        if (rc)
        {
                return FALSE; 
        }
    }
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
    if (NULL != p_db_rtif)
    {
        return TRUE;
    }
    else
    {
        pkt_vid = p_msg->u_hdr.pkt.vid;
        return sflow_if_in_vlan_l3(pkt_vid, p_db_if, p_msg);
    }

    return TRUE;
}

int32
sflow_get_ethertype(ctc_msg_t *p_msg, uint32_t *ethertype)
{
    eth_hdr_t                *eth = NULL;
    sflow_vlan_hdr_t         *vlan = NULL;
    
    if (!p_msg || !p_msg->data)
    {
        return 0;    
    }
    eth = (eth_hdr_t*)p_msg->data;
#ifndef _GLB_UML_SYSTEM_
#ifndef USW
    if (sal_hton16(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        *ethertype = vlan->proto;
        return 0;
    }
    *ethertype = eth->ether_type;
#else
    uint32                       u32=0;
    uint16                       u16=0;
    if ((ETH_P_8021Q) == sal_ntoh16(eth->ether_type))
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        u16=sal_ntoh16(vlan->proto);
        u32=(uint32)u16;
        *ethertype =(u32);
        return 0;
    }
    u16=sal_ntoh16(eth->ether_type);
    u32=(uint32)u16;
    *ethertype =GLB_CTC_SWAP32(u32);
#endif
#else
    uint32                       u32=0;
    uint16                       u16=0;
    if (sal_hton32(ETH_P_8021Q) == eth->ether_type)
    {
        vlan = (sflow_vlan_hdr_t*)(eth + 1);
        u32=vlan->proto;
        *ethertype = GLB_CTC_SWAP32(u32);
        return 0;
    }
    u16=sal_ntoh16(eth->ether_type);
    u32=(uint32)u16;
    *ethertype =sal_ntoh32(u32);
#endif
    return 0;
}

int32
sflow_is_ip_message(ctc_msg_t *p_msg)
{
    uint32          ethertype = 0;
    if (!p_msg || !p_msg->data)
    {
        return FALSE;    
    }
    sflow_get_ethertype(p_msg, &ethertype);
    #ifndef _GLB_UML_SYSTEM_
    #ifndef USW
    return (SFLOW_IP_PACKET == ethertype) ? TRUE : FALSE;
    #else
    return (SFLOW_IP_PACKET == GLB_CTC_SWAP32(ethertype)) ? TRUE : FALSE;
    #endif
    #else
    return (sal_ntoh32(ethertype) == SFLOW_IP_PACKET) ? TRUE : FALSE; 
    #endif
}

/*ended by yejl*/
void
sflow_pkt_func(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    uint32                         ifindex = 0;
    uint32                         frame_original_len = 0;
    uint32                         frame_sampled_len = 0;
    uint32                         frame_stripped_len = 0; /* added by yejl for sflow version 5*/
    uint32                         padlen = 0;
    uint32                         sampledir = 0;
    tbl_interface_t                *p_db_if = NULL;
    char                           *sampledirstr = NULL;
    uint32                         sequence = 0;
    uint32                         samplesize = 0;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_global_t             *p_sflow_glb = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    struct sflow_datagram_v4_inet  *p_sflowpacket4 = NULL;
    struct sflow_datagram_v5_inet  *p_sflowpacket5 = NULL; // added by yejl for sflow version 5sflow  datagrame
    uint32                         *size = NULL;
    uint8                          *pnt = NULL;
    uint32                         temp = 0;
    uint32                         sample_num = 0;
    uint32                         pkt_vid = 0;
    int32                           l2_l3_flag=0;
    uint32                          nexthop_ip = 0;
    uint32                          is_ip_message = 0;
    /* modified by liwh for bug 53920, 2019-10-12 */
    struct sflow_datagram_v4_inet6 *p_sflowpacket4_inet6 = NULL;
    struct sflow_datagram_v5_inet6 *p_sflowpacket5_inet6 = NULL;
    /* liwh end */
    
    struct sflow_flow_extended_router_data  flow_extended_router_data;
    ifindex = p_msg->u_hdr.pkt.ifindex;
    frame_original_len = p_msg->data_len - sizeof(struct sflow_flow_extended_router_data);
    sal_memset(&flow_extended_router_data, 0, sizeof(flow_extended_router_data));
    is_ip_message = sflow_is_ip_message(p_msg);
    sflow_get_expand_route_data(p_msg,&flow_extended_router_data);
    nexthop_ip = *(uint32*)(&(flow_extended_router_data.nexthop));
    l2_l3_flag= sflow_get_l3_flag(p_msg);
    /* get collector */
    p_glb_collector = tbl_sflow_collector_get_master();
    if ((!p_glb_collector) || (0 == p_glb_collector->sflow_collector_list->count))
    {
        SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "TBL_SFLOW_COLLECTOR is not initialized");
        return;
    }

    /* if packet length is longer than 256 bytes, only sample 256 bytes */
    if (frame_original_len > SFLOW_DEFAULT_MAX_HEADERSIZE)
    {
        frame_sampled_len = SFLOW_DEFAULT_MAX_HEADERSIZE;   
        frame_stripped_len = frame_original_len - SFLOW_DEFAULT_MAX_HEADERSIZE;
    }
    else
    {
        frame_sampled_len = frame_original_len;
        frame_stripped_len = 0;
    }
    
    /* Sampled data must be a multiple of SFLOW_HEADER_ALIGNMENT */
    padlen = frame_sampled_len % SFLOW_HEADER_ALIGNMENT;
    padlen = SFLOW_HEADER_ALIGNMENT - padlen;
    padlen %= SFLOW_HEADER_ALIGNMENT;

    /* get sample direction */
    if (GLB_SFLOW_INGRESS_PACKET == p_msg->u_hdr.pkt.sub_type)
    {
        sampledir = GLB_INGRESS;
        sampledirstr = "input";
    }
    else
    {
        sampledir = GLB_EGRESS;
        sampledirstr = "output";
    }

    /* get sample port */
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if)
    {
        return;
    }

    if (p_db_if->mirror_dest_id && mirror_discard_sflow_packet(p_db_if->mirror_dest_id))
    {
        SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "Sample packet is discard by mirror for ifindex %d\n", ifindex);
        return;
    }

    /* check sample is not enabled for port */
    if ((!p_db_if->sflow_flow_ingress) && (!p_db_if->sflow_flow_egress))
    {
        SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "Sample[Id: 0x%x]: Fail to get flow sampler\n", ifindex);
        return;
    }

    /* check sample is not enabled for packet direction */
    if (((GLB_INGRESS == sampledir) && (!p_db_if->sflow_flow_ingress))
        && ((GLB_EGRESS == sampledir) && (!p_db_if->sflow_flow_egress)))
    {
        SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "Sample[Id: 0x%x] for direction %s is not enable\n", ifindex, sampledirstr);
        return;
    }

    /* add sample pool */
    p_db_if->sflow_sample_pool = p_db_if->sflow_sample_pool + p_db_if->sflow_rate;

    /* add sflow sequence */
    sequence = p_db_if->sflow_sequence++;

    SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "Sample[%s]: Sampling packet on %s direction, seq %u, len %u, pool %u\n", 
        p_db_if->key.name, sampledirstr, sequence, frame_original_len, p_db_if->sflow_sample_pool);

    
    /*added by yejl for sflow version 5*/
    /* check exist */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (NULL == p_sflow_glb)
    {
        SFLOW_LOG_DEBUG(SFLOW_FLAG_SAMPLE, "Sample sflow init failed\n");
        return;
    }
    /*added by yejl for sflow version 5*/
    if (SFLOW_VERSION5 == p_sflow_glb->version) /* sFlow version 5 */
    {
        samplesize += sizeof(struct sflow_packet_flow_sample_expanded) + 8;
        samplesize += sizeof(struct sflow_flow_raw_packet_header) + 8;
        samplesize += frame_sampled_len;
        samplesize += padlen;
        samplesize +=sizeof(struct sflow_flow_expanded_frame_data)+ 8;
        if (is_ip_message)
        {
            samplesize += sizeof(struct sflow_flow_sample_ipv4_data) + 8;//add by yangl
            if (nexthop_ip && l2_l3_flag)
            { 
                samplesize +=sizeof(struct sflow_flow_extended_router_data)+8;
            }
        }
        
        samplesize += sizeof(struct sflow_flow_expanded_switch_data) + 8;
    }
    else if (SFLOW_VERSION4 == p_sflow_glb->version) /* sFlow version 4 */
    {
        samplesize += sizeof(struct sflow_packet_sample);
        samplesize += sizeof(struct sflow_packet_flow_sample);
        samplesize += sizeof(struct sflow_flow_sample_packet);
        samplesize += sizeof(struct sflow_flow_sample_header);
        samplesize += frame_sampled_len;
        samplesize += padlen;
        samplesize += sizeof(uint32);  /* space for extended info number */
        samplesize += sizeof(struct sflow_flow_extended_info);
        samplesize += sizeof(struct sflow_flow_extended_switch);
    }

    /* loop all collector */
    CTCLIB_SLIST_LOOP(p_glb_collector->sflow_collector_list, p_db_collector, listnode)
    {
        struct sflow_packet_flow_sample_expanded flow_sample_exp;
        struct sflow_flow_raw_packet_header flow_raw_packet_header;
        struct sflow_flow_expanded_frame_data flow_expanded_frame_data;
        struct sflow_flow_expanded_switch_data flow_expanded_switch_data;
        struct sflow_flow_sample_ipv4_data  flow_sample_ipv4_data;
        
      

        /* if add next packet, the buf is not enough, send packet */
        if (p_db_collector->pkt_len + samplesize > GLB_SFLOW_PKT_BUF)
        {
            sflow_transmit_packet(p_db_collector);//send    
        }

        /* modified by liwh for bug 53920, 2019-10-12 */
        if (AF_INET == p_db_collector->key.addr.family)
        /* liwh end */
        {
            /* reserve sflow packet header */
            if (0 == p_db_collector->pkt_len)
            {
                /*added by yejl for sflow version 5*/
                if (SFLOW_VERSION5 == p_sflow_glb->version)
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v5_inet);
                }
                else
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v4_inet); 
                }
            }
            
            /* set sflow packet sample num */
            /*added by yejl for sflow version 5*/
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {            
                p_sflowpacket5 = (struct sflow_datagram_v5_inet*)p_db_collector->buf;
#ifndef _GLB_UML_SYSTEM_
#ifndef USW
                sample_num = sal_ntoh32(p_sflowpacket5->sample_num);
#else
                sample_num = sal_ntoh32(p_sflowpacket5->sample_num);
#endif
#else
                sample_num =(p_sflowpacket5->sample_num);
#endif
                sample_num++;
#ifndef _GLB_UML_SYSTEM_
#ifndef USW
                p_sflowpacket5->sample_num = sal_hton32(sample_num);          
#else
                p_sflowpacket5->sample_num = sal_hton32(sample_num);
#endif
#else
                p_sflowpacket5->sample_num = sample_num;
#endif
            }
            else
            {
                p_sflowpacket4 = (struct sflow_datagram_v4_inet*)p_db_collector->buf;
#ifndef _GLB_UML_SYSTEM_
                sample_num = sal_ntoh32(p_sflowpacket4->sample_num);
#else
                sample_num =(p_sflowpacket4->sample_num);
#endif
                sample_num++;
#ifndef _GLB_UML_SYSTEM_
                p_sflowpacket4->sample_num = sal_hton32(sample_num);
#else
                p_sflowpacket4->sample_num = sample_num;
#endif
            }
        }
        /* modified by liwh for bug 53920, 2019-10-12 */
        else
        {
            /* reserve sflow packet header */
            if (0 == p_db_collector->pkt_len)
            {
                /*added by yejl for sflow version 5*/
                if (SFLOW_VERSION5 == p_sflow_glb->version)
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v5_inet6);
                }
                else
                {
                    p_db_collector->pkt_len = p_db_collector->pkt_len + sizeof(struct sflow_datagram_v4_inet6); 
                }
            }
            
            /* set sflow packet sample num */
            /*added by yejl for sflow version 5*/
            if (SFLOW_VERSION5 == p_sflow_glb->version)
            {            
                p_sflowpacket5_inet6 = (struct sflow_datagram_v5_inet6*)p_db_collector->buf;
#ifndef _GLB_UML_SYSTEM_
#ifndef USW
                sample_num = sal_ntoh32(p_sflowpacket5_inet6->sample_num);
#else
                sample_num = sal_ntoh32(p_sflowpacket5_inet6->sample_num);
#endif
#else
                sample_num =(p_sflowpacket5_inet6->sample_num);
#endif
                sample_num++;
#ifndef _GLB_UML_SYSTEM_
#ifndef USW
                p_sflowpacket5_inet6->sample_num = sal_hton32(sample_num);          
#else
                p_sflowpacket5_inet6->sample_num = sal_hton32(sample_num);
#endif
#else
                p_sflowpacket5_inet6->sample_num = sample_num;
#endif
            }
            else
            {
                p_sflowpacket4_inet6 = (struct sflow_datagram_v4_inet6*)p_db_collector->buf;
#ifndef _GLB_UML_SYSTEM_
                sample_num = sal_ntoh32(p_sflowpacket4_inet6->sample_num);
#else
                sample_num =(p_sflowpacket4_inet6->sample_num);
#endif
                sample_num++;
#ifndef _GLB_UML_SYSTEM_
                p_sflowpacket4_inet6->sample_num = sal_hton32(sample_num);
#else
                p_sflowpacket4_inet6->sample_num = sample_num;
#endif
            }        
        }
        /* liwh end */
        
        pnt = p_db_collector->buf;
        pnt += p_db_collector->pkt_len;
        size = &(p_db_collector->pkt_len);

        /*added by yejl for sflow version 5*/
        if (SFLOW_VERSION5 == p_sflow_glb->version)
        {
            /* expanded flow sample */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_SAMPLE_EXPANDED); // Tag
            if (is_ip_message)
            {
                if (nexthop_ip && l2_l3_flag)
                {
                    SFLOW_ENCODE_PUTL(sizeof(struct sflow_packet_flow_sample_expanded)
                                + sizeof(struct sflow_flow_raw_packet_header) + 8 + frame_sampled_len + padlen
                                + sizeof(struct sflow_flow_expanded_frame_data)+ 8
                                + sizeof(struct sflow_flow_expanded_switch_data) + 8
                                + sizeof(struct sflow_flow_sample_ipv4_data)+ 8// add by yangl
                                + sizeof(struct sflow_flow_extended_router_data)+ 8 );  // add by yangl
                }
                else
                {
                    SFLOW_ENCODE_PUTL(sizeof(struct sflow_packet_flow_sample_expanded)
                                + sizeof(struct sflow_flow_raw_packet_header) + 8 + frame_sampled_len + padlen
                                + sizeof(struct sflow_flow_expanded_frame_data)+ 8
                                + sizeof(struct sflow_flow_expanded_switch_data) + 8
                                + sizeof(struct sflow_flow_sample_ipv4_data)+ 8);// add by yangl

                }
            }
            else
            {
                    SFLOW_ENCODE_PUTL(sizeof(struct sflow_packet_flow_sample_expanded)
                                + sizeof(struct sflow_flow_raw_packet_header) + 8 + frame_sampled_len + padlen
                                + sizeof(struct sflow_flow_expanded_frame_data)+ 8
                                + sizeof(struct sflow_flow_expanded_switch_data) + 8);
            }
            sal_memset(&flow_sample_exp, 0, sizeof(flow_sample_exp)); 
            
#ifndef _GLB_UML_SYSTEM_           
            flow_sample_exp.sequence_number = sal_hton32(sequence);
            flow_sample_exp.source_id_type = 0; // type = 0, source_id_index is ifIndex
            flow_sample_exp.source_id_index = sal_hton32((ifindex & 0x00FFFFFF));
            flow_sample_exp.sampling_rate = sal_hton32(p_db_if->sflow_rate);
            flow_sample_exp.sample_pool = sal_hton32(p_db_if->sflow_sample_pool);
            flow_sample_exp.drops = sal_hton32(0);
#else//add by yangl
            flow_sample_exp.sequence_number = sal_ntoh32(sequence);
            flow_sample_exp.source_id_type = 0; // type = 0, source_id_index is ifIndex
            flow_sample_exp.source_id_index =sal_ntoh32((ifindex & 0x00FFFFFF));//((ifindex & 0xFFFFFF00));
            flow_sample_exp.sampling_rate = sal_ntoh32(p_db_if->sflow_rate);
            flow_sample_exp.sample_pool = sal_ntoh32(p_db_if->sflow_sample_pool);
            flow_sample_exp.drops = (0);
 #endif
 
#ifndef _GLB_UML_SYSTEM_   
            if (GLB_INGRESS == sampledir)
            {
                flow_sample_exp.input_format = sal_hton32(ifindex >> 30 & 0x3);       // high 2 bits 
                flow_sample_exp.input_value = sal_hton32(ifindex & 0x3FFFFFFF);       // low 30 bits
            }
            else
            {
                flow_sample_exp.output_format = sal_hton32(ifindex >> 30 & 0x3);     // high 2 bits
                flow_sample_exp.output_value = sal_hton32(ifindex & 0x3FFFFFFF);     // low 30 bits    
            }
#else
             if (GLB_INGRESS == sampledir)
            {
                flow_sample_exp.input_format = sal_ntoh32(ifindex >> 30 & 0x3);       // high 2 bits 
                flow_sample_exp.input_value = sal_ntoh32(ifindex & 0x3FFFFFFF);       // low 30 bits
            }
            else
            {
                flow_sample_exp.output_format = sal_ntoh32(ifindex >> 30 & 0x3);     // high 2 bits
                flow_sample_exp.output_value =sal_ntoh32(ifindex & 0x3FFFFFFF);     // low 30 bits    
            }
#endif
            if (is_ip_message)
            {
                if (nexthop_ip && l2_l3_flag)
                {
                   flow_sample_exp.flow_records_num = sal_hton32(5);  
                }// records numbers : raw pkt hdr, expanded frame data, expanded switch data
                else
                {
                  flow_sample_exp.flow_records_num = sal_hton32(4); 
                }
            }
            else
            {
                flow_sample_exp.flow_records_num = sal_hton32(3); 
            }
            SFLOW_ENCODE_PUT(&flow_sample_exp, sizeof(struct sflow_packet_flow_sample_expanded));

            /* record 1: raw pkt hdr */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_RAW_PACKET_HEADER);  // Tag
            SFLOW_ENCODE_PUTL(sizeof(struct sflow_flow_raw_packet_header) + frame_sampled_len + padlen);  // length
            sal_memset(&flow_raw_packet_header, 0, sizeof(flow_raw_packet_header)); 
      
            flow_raw_packet_header.header_protocol = sal_hton32(SFLOW_FLOW_HEADER_ISO8023); // only support ethernet
            flow_raw_packet_header.frame_length = sal_hton32(frame_original_len);
            flow_raw_packet_header.stripped = sal_hton32(frame_stripped_len);
            flow_raw_packet_header.header_length = sal_hton32(frame_sampled_len);
            SFLOW_ENCODE_PUT(&flow_raw_packet_header, sizeof(struct sflow_flow_raw_packet_header));
            /* Header of sampled packet */
            SFLOW_ENCODE_PUT(p_msg->data, frame_sampled_len);
            if (0 != padlen)
            {   
                temp = 0;
                SFLOW_ENCODE_PUT(&temp, padlen);
            }

            /* record 2: ethernet frame data */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_ETHERNET_FRAME_DATA);    // Tag
            SFLOW_ENCODE_PUTL(sizeof(struct sflow_flow_expanded_frame_data)); // length
            sal_memset(&flow_expanded_frame_data, 0, sizeof(flow_expanded_frame_data));            
            flow_expanded_frame_data.length_mac_packet = sal_hton32(frame_sampled_len); // equal to header_length

   
            sflow_get_mac(p_msg, flow_expanded_frame_data.dst_mac, flow_expanded_frame_data.src_mac);
            sflow_get_ethertype(p_msg, &flow_expanded_frame_data.ethernet_packet_type);
            SFLOW_ENCODE_PUT(&flow_expanded_frame_data, sizeof(struct sflow_flow_expanded_frame_data));

             /* record 3: expanded swtich data */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_EXPENDED_SWITCH_DATA);    // Tag 
            SFLOW_ENCODE_PUTL(sizeof(struct sflow_flow_expanded_switch_data));    // length
            sal_memset(&flow_expanded_switch_data, 0, sizeof(flow_expanded_switch_data)); 
            pkt_vid = sflow_get_pkt_vid(p_msg);
            
 #ifndef _GLB_UML_SYSTEM_ 
            if (GLB_INGRESS == sampledir)
            {
                if (pkt_vid)
                {
                    flow_expanded_switch_data.src_vlan = sal_hton32(pkt_vid & 0xfff); /* src_vlan */
                    flow_expanded_switch_data.src_priority = sal_hton32((pkt_vid >> 13) & 0x7); /* src_priority */
                }
                else if(p_msg->u_hdr.pkt.vid)
                {
                    flow_expanded_switch_data.src_vlan = sal_hton32(p_msg->u_hdr.pkt.vid & 0xfff); /* src_vlan */
                    flow_expanded_switch_data.src_priority = sal_hton32((p_msg->u_hdr.pkt.vid >> 13) & 0x7); /* src_priority */
                }
                else
                {
                    if (p_db_if->brgif)
                    {
                        flow_expanded_switch_data.src_vlan = sal_hton32(p_db_if->brgif->pvid & 0xfff);
                    }
                    else
                    {
                        flow_expanded_switch_data.src_vlan = 0;   
                    }    
                }
            

                /* dst_vlan */ 
                flow_expanded_switch_data.dst_vlan = 0; /* dst_vlan */ 
                flow_expanded_switch_data.dst_priority = 0; /* dst_priority */
            }
            else
            {
                /* src_vlan */
                flow_expanded_switch_data.src_vlan = 0;
                /* src_priority */
                flow_expanded_switch_data.src_priority = 0;

                /* dst_vlan */

             
                if (pkt_vid)
                {
                    flow_expanded_switch_data.dst_vlan = sal_hton32(pkt_vid & 0xfff);
                }
                else
                {    
                    if (p_db_if->brgif)
                    {
                        flow_expanded_switch_data.dst_vlan = sal_hton32(p_db_if->brgif->pvid & 0xfff);
                    }
                    else
                    {
                        flow_expanded_switch_data.dst_vlan = 0;   
                    }
                }

                /* dst_priority */
                if (pkt_vid)
                {
                    flow_expanded_switch_data.dst_priority = sal_hton32((pkt_vid >> 13) & 0x7);
                }
                else
                {
                    flow_expanded_switch_data.dst_priority = 0;
                }
                
               
            }
#else
            if (GLB_INGRESS == sampledir)
            {
                if (pkt_vid)
                {
                    flow_expanded_switch_data.src_vlan =(pkt_vid & 0xfff); /* src_vlan */
                    flow_expanded_switch_data.src_priority = ((pkt_vid >> 13) & 0x7); /* src_priority */
                }
                else if(p_msg->u_hdr.pkt.vid)
                {
                    flow_expanded_switch_data.src_vlan = (p_msg->u_hdr.pkt.vid & 0xfff); /* src_vlan */
                    flow_expanded_switch_data.src_priority =((p_msg->u_hdr.pkt.vid >> 13) & 0x7); /* src_priority */
                }
                else
                {
                    if (p_db_if->brgif)
                    {
                        flow_expanded_switch_data.src_vlan = (p_db_if->brgif->pvid & 0xfff);
                    }
                    else
                    {
                        flow_expanded_switch_data.src_vlan = 0;   
                    }    
                }
            

                /* dst_vlan */ 
                flow_expanded_switch_data.dst_vlan = 0; /* dst_vlan */ 
                flow_expanded_switch_data.dst_priority = 0; /* dst_priority */
            }
            else
            {
                /* src_vlan */
                flow_expanded_switch_data.src_vlan = 0;
                /* src_priority */
                flow_expanded_switch_data.src_priority = 0;

                /* dst_vlan */

             
                if (pkt_vid)
                {
                    flow_expanded_switch_data.dst_vlan = (pkt_vid & 0xfff);
                }
                else
                {    
                    if (p_db_if->brgif)
                    {
                        flow_expanded_switch_data.dst_vlan = (p_db_if->brgif->pvid & 0xfff);
                    }
                    else
                    {
                        flow_expanded_switch_data.dst_vlan = 0;   
                    }
                }

                /* dst_priority */
                if (pkt_vid)
                {
                    flow_expanded_switch_data.dst_priority = ((pkt_vid >> 13) & 0x7);
                }
                else
                {
                    flow_expanded_switch_data.dst_priority = 0;
                }           

            }
 #endif
            SFLOW_ENCODE_PUT(&flow_expanded_switch_data, sizeof(struct sflow_flow_expanded_switch_data));

             /*record 4 ipv4_data add by yangl*/
            if (is_ip_message)
            {
                SFLOW_ENCODE_PUTL(SFLOW_FLOW_PACKET_IP_V4_DATA );    // Tag 
                SFLOW_ENCODE_PUTL(sizeof(struct sflow_flow_sample_ipv4_data) );//length
                sal_memset(&flow_sample_ipv4_data, 0, sizeof(flow_sample_ipv4_data)); //space
                sflow_get_ip_v4_data(p_msg, &flow_sample_ipv4_data);
                SFLOW_ENCODE_PUT(&flow_sample_ipv4_data, sizeof(struct sflow_flow_sample_ipv4_data));
            /*record 5 sflow_flow_extended_router_data add by yangl*/
                if (nexthop_ip && l2_l3_flag)
                {
                    SFLOW_ENCODE_PUTL(SFLOW_FLOW_EXPENDED_ROUTER_DATA );  //tag
                    SFLOW_ENCODE_PUTL(sizeof(struct sflow_flow_extended_router_data) );//length  
                    SFLOW_ENCODE_PUT(&flow_extended_router_data, sizeof(struct sflow_flow_extended_router_data));
                }
            }

        }
        else 
        {
            /* encode sflow flow packet */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_SAMPLE);
            SFLOW_ENCODE_PUTL(sequence);
            SFLOW_ENCODE_PUTL(ifindex & 0x00FFFFFF);
            SFLOW_ENCODE_PUTL(p_db_if->sflow_rate);
            SFLOW_ENCODE_PUTL(p_db_if->sflow_sample_pool);

            /* drops */
            temp = 0;
            SFLOW_ENCODE_PUTL(temp);

            if (GLB_INGRESS == sampledir)
            {
                /* input */
                SFLOW_ENCODE_PUTL(ifindex);

                /* output */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);
            }
            else
            {
                /* input */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);
                
                /* output */
                SFLOW_ENCODE_PUTL(ifindex);
            }
            
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_HEADER);
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_HEADER_ISO8023);
            SFLOW_ENCODE_PUTL(frame_original_len);
            SFLOW_ENCODE_PUTL(frame_sampled_len);

            /* copy packet */
            SFLOW_ENCODE_PUT(p_msg->data, frame_sampled_len);

            /* set pad to 0 */
            temp = 0;
            if (0 != padlen)
            {
                SFLOW_ENCODE_PUT(&temp, padlen);
            }
            
            /* Extended info */
            SFLOW_ENCODE_PUTL(1);  /* Only one extended infomation */
            SFLOW_ENCODE_PUTL(SFLOW_FLOW_EXTENDED_SWITCH);  /* Only SFLOW_FLOW_EXTENDED_SWITCH now */

            pkt_vid = sflow_get_pkt_vid(p_msg);

            if (GLB_INGRESS == sampledir)
            {
                /* src_vlan */
                if (pkt_vid)
                {
                    temp = pkt_vid & 0xfff;
                }
                else
                {
                    temp = p_msg->u_hdr.pkt.vid & 0xfff;
                }
                SFLOW_ENCODE_PUTL(temp);

                /* src_priority */
                if (pkt_vid)
                {
                    temp = (pkt_vid >> 13) & 0x7;
                }
                else
                {
                    temp = (p_msg->u_hdr.pkt.vid >> 13) & 0x7;
                }
                SFLOW_ENCODE_PUTL(temp);

                /* dst_vlan */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);

                /* dst_priority */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);
            }
            else
            {
                /* src_vlan */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);

                /* src_priority */
                temp = 0;
                SFLOW_ENCODE_PUTL(temp);

                /* dst_vlan */
                if (pkt_vid)
                {
                    temp = pkt_vid & 0xfff;
                }
                else
                {    
                    if (p_db_if->brgif)
                    {
                        temp = p_db_if->brgif->pvid & 0xfff;
                    }
                    else
                    {
                        temp = 0;    
                    }
                }
                SFLOW_ENCODE_PUTL(temp);

                /* dst_priority */
                if (pkt_vid)
                {
                    temp = (pkt_vid >> 13) & 0x7;
                }
                else
                {
                    temp = 0;
                }          
                SFLOW_ENCODE_PUTL(temp);
            }
        }

        /* start transdelay timer */
        if (!p_db_collector->transdelay_timer)
        {
            p_db_collector->transdelay_timer = ctc_task_add_timer(TASK_PRI_NORMAL, sflow_transdelay_timer_fn, 
                p_db_collector, SFLOW_TRANSMIT_DELAY);
            if (!p_db_collector->transdelay_timer)
            {
                SFLOW_LOG_ERR("Start sflow transdelay timer fail");
                return;
            }
        }
    }
    
    return;
}

int32
sflow_api_set_sflow_counter_interval(uint32 counter_interval)
{
    tbl_sflow_global_t  sflow_glb;
    tbl_sflow_global_t  *p_sflow_glb = NULL;
    uint32              timer = 0;

    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    /* check value isn't change */
    if (counter_interval == p_sflow_glb->counter_interval)
    {
        return PM_E_NONE;    
    }

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));    
    sflow_glb.counter_interval = counter_interval;
    tbl_sflow_global_set_sflow_global_field(&sflow_glb, TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL);

    if (0 == p_sflow_glb->counter_port_num)
    {
        return PM_E_NONE; 
    }
    
    /* if counter timer is running, stop timer */
    if (p_sflow_glb->counter_timer)
    {
        ctc_task_delete_timer(p_sflow_glb->counter_timer);
        p_sflow_glb->counter_timer = NULL;
    }
        
    /* counter sampling for new interface and send sflow packet */
    if (GLB_INVALID_IFINDEX != p_sflow_glb->counter_next_port)
    {
        sflow_counter_output(p_sflow_glb->counter_next_port);
    }
        
    /* get next interface */
    p_sflow_glb->counter_next_port = sflow_get_next_counter_port(p_sflow_glb->counter_next_port);

    if (GLB_INVALID_IFINDEX == p_sflow_glb->counter_next_port)
    {
        return PM_E_NONE; 
    }

    /* calculate counter timer interval */
    timer = GLB_SFLOW_TIME_SEC2MSEC * p_sflow_glb->counter_interval / p_sflow_glb->counter_port_num;
        
    /* start timer for new interval */
    p_sflow_glb->counter_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, sflow_counter_timer_fn, p_sflow_glb, timer);
    if (!p_sflow_glb->counter_timer)
    {
        SFLOW_LOG_ERR("Start sflow counter timer fail");
    }

    return PM_E_NONE;
}


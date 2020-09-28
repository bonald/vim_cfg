
#include "proto.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"

#include "ptp_datatype.h"
#include "ptp_constant.h"
#include "ptp_error.h"
#include "ptp_desc.h"
#include "ptp_msg.h"
#include "ptp_ops.h"
#include "ptp_sock.h"
#include "ptp_adpt.h"

#define PTP_RCV_BUF 256
#define PTP_TIMESTAMP_BYTES 10

#ifndef AF_PTP
#define AF_PTP 30
#endif 

#define ptp_vlan_tag_pkt_move_pld(buf, ptp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += VLAN_TAG_LEN;       \
    ptp_p = p;              \
} while(0)

#define ptp_cos_vlan_pkt_move_pld(buf, ptp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += (VLAN_TAG_LEN - 2);       \
    ptp_p = p;              \
} while(0)

#define ptp_eth_pkt_move_pld(buf, ptp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += ETH_HEADER_LEN;       \
    ptp_p = p;              \
} while(0)

#define ptp_ip_pkt_move_pld(buf, ptp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += IP_HEADER_LEN;       \
    ptp_p = (ptp_sk_buff_t*)p;              \
} while(0)

#define ptp_udp_pkt_move_pld(buf, ptp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += UDP_HEADER_LEN;       \
    ptp_p = (ptp_sk_buff_t*)p;              \
} while(0)


#ifndef ptp_skb

uint8 *
ptp_skb_push(ptp_sk_buff_t* skb, uint32 len)
{
    skb->data -= len;
    skb->len  += len;
    return skb->data;
}

uint8 *
ptp_skb_pull(ptp_sk_buff_t* skb, uint32 len)
{
    skb->len -= len;
    return skb->data += len;
}

uint8 *
ptp_skb_put(ptp_sk_buff_t* skb, uint32 len)
{
    uint8 * tmp = skb->tail;
    skb->tail += len;
    skb->len  += len;
    return tmp;
}

uint8 *
ptp_skb_get(ptp_sk_buff_t* skb, uint32 len)
{
    skb->tail -= len;
    skb->len  -= len;
    return skb->tail;
}

ptp_sk_buff_t *
ptp_alloc_skb(uint32 size)
{
    ptp_sk_buff_t * skb;

    skb = XCALLOC(MEM_PTP_PDU, sizeof(ptp_sk_buff_t) + PTP_SKB_RESERVE + size);
    if (!skb)
        return NULL;

    skb->head = (uint8 *)(skb + 1);
    skb->end = (uint8 *)(skb + 1) + PTP_SKB_RESERVE + size;
    skb->data = skb->tail = (uint8 *)(skb + 1) + PTP_SKB_RESERVE;
    skb->len = 0;

    return skb;
}

void
ptp_init_skb(ptp_sk_buff_t* skb, uint32 size)
{
    skb->head = (uint8 *)(skb + 1);
    skb->end = (uint8 *)(skb + 1) + PTP_SKB_RESERVE + size;
    skb->data = skb->tail = (uint8 *)(skb + 1) + PTP_SKB_RESERVE;
    skb->len = 0;
    return;
}

ptp_sk_buff_t *
ptp_skb_copy(ptp_sk_buff_t* skb)
{
    ptp_sk_buff_t * skb2;
    uint32 len;

    len = skb->end - (uint8 *)skb;
    skb2 = XMALLOC(MEM_PTP_PDU, len);
    if (!skb2)
        return NULL;

    skb2->head = (uint8 *)(skb2 + 1);
    skb2->end = (uint8 *)skb2 + len;
    skb2->data = skb2->head + (skb->data - skb->head);
    skb2->tail = skb2->head + (skb->tail - skb->head);
    sal_memcpy(skb2->data, skb->data, skb->len);
    skb2->len = skb->len;

    return skb2;
}

void
ptp_free_skb(ptp_sk_buff_t* skb)
{
    if (skb)
    {
        XFREE(MEM_PTP_PDU, skb);
        skb = NULL;
    }
}

#endif

static inline uint32
_ptp_is_event_message(ptp_msg_type_t type)
{
    switch (type)
    {
    case PTP_MSG_TYPE_SYNC:
    case PTP_MSG_TYPE_DELAY_REQ:
    case PTP_MSG_TYPE_PDELAY_REQ:
    case PTP_MSG_TYPE_PDELAY_RESP:
        return TRUE;

    default:
        return FALSE;
    }
}

static inline uint32
_ptp_is_peer_message(ptp_msg_type_t type)
{
    switch (type)
    {
    case PTP_MSG_TYPE_PDELAY_REQ:
    case PTP_MSG_TYPE_PDELAY_RESP:
    case PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP:
        return TRUE;

    default:
        return FALSE;
    }
}

static void
_ptp_sock_stats(tbl_ptp_port_t *p_port, ptp_stats_type_t mode, ptp_msg_type_t type)
{
    switch (mode)
    {
    case PTP_STATS_TX:
    case PTP_STATS_RX:
    case PTP_STATS_DROP:
        break;
    default:
        sal_assert(0);
        return;
    }

    switch (type)
    {
    case PTP_MSG_TYPE_SYNC:
        p_port->packet_stats[mode].sync++;
        break;
    case PTP_MSG_TYPE_DELAY_REQ:
        p_port->packet_stats[mode].delay_req++;
        break;
    case PTP_MSG_TYPE_PDELAY_REQ:
        p_port->packet_stats[mode].pdelay_req++;
        break;
    case PTP_MSG_TYPE_PDELAY_RESP:
        p_port->packet_stats[mode].pdelay_resp++;
        break;
    case PTP_MSG_TYPE_FOLLOW_UP:
        p_port->packet_stats[mode].follow_up++;
        break;
    case PTP_MSG_TYPE_DELAY_RESP:
        p_port->packet_stats[mode].delay_resp++;
        break;
    case PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP:
        p_port->packet_stats[mode].pdelay_resp_follow_up++;
        break;
    case PTP_MSG_TYPE_ANNOUNCE:
        p_port->packet_stats[mode].announce++;
        break;
    default:
        p_port->packet_stats[mode].unknown++;
        break;
    }

    return;
}

void
_ptp_message_dump_raw(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, uint32 is_rcv)
{
#define PTP_DUMP_RAW_COL_MAX 10
#define PTP_DUMP_RAW_BUF_LEN 40
    uint32 idx, start = 0;
    uint8 offset = 0;
    char dump_buff[PTP_DUMP_RAW_BUF_LEN];
    sal_memset(dump_buff, 0x00, PTP_DUMP_RAW_BUF_LEN);
    for (idx = 0; idx < skb->len; idx++)
    {
        if (0 == offset)
        {
            start = idx;
        }

        sal_sprintf((dump_buff + (3 * offset)), "%.2hhx ", skb->data[idx]);
        offset++;

        if (0 == offset % PTP_DUMP_RAW_COL_MAX)
        {
            PTP_LOG_INFO("0x%.8x:  %s", start, dump_buff);
            if (is_rcv)
            {
                PTP_DBG_PACKET_RX("MSG_DUMP 0x%.8x:  %s", start, dump_buff);
            }
            else
            {
                PTP_DBG_PACKET_TX("MSG_DUMP 0x%.8x:  %s", start, dump_buff);
            }
            sal_memset(dump_buff, 0x00, PTP_DUMP_RAW_BUF_LEN);
            offset = 0;
        }
    }

    PTP_LOG_INFO("0x%.8x:  %s", start, dump_buff);
    if (is_rcv)
    {
        PTP_DBG_PACKET_RX("MSG_DUMP 0x%.8x:  %s", start, dump_buff);
    }
    else
    {
        PTP_DBG_PACKET_RX("MSG_DUMP 0x%.8x:  %s", start, dump_buff);
    }
}

typedef struct
{
#if _GLB_HOST_IS_LE
    uint8   ihl:4;                  /* header length */
    uint8   version:4;              /* version */
#else
    uint8   version:4;              /* version */
    uint8   ihl:4;                  /* header length */
#endif
    uint8   tos;                    /* type of service */
    uint16  total_length;           /* total length */
    
    uint16  identification;         /* identification */
    uint16  offset;                 /* fragment offset field */
    
    uint8   ttl;                    /* time to live */
    uint8   protocol;               /* protocol */
    uint16  checksum;               /* checksum */
    
    uint32  src_ip_addr;            /* src ip */    
    uint32  dest_ip_addr;           /* dest ip */
} ptp_ip_header_t;

/*
** UDP Packet header.
*/
typedef struct
{
  u_int16_t uh_sport;
  u_int16_t uh_dport;
  u_int16_t uh_ulen;
  u_int16_t uh_sum;
} ptp_udp_header_t;

/* Set length of IP packet
**
** Parameters:
**   IN/OUT    pal_in4_header   *iph
**   IN         u_int16_T        length of IP packet
** Results:
**   RESULT_OK for success, < 0 for error
*/
uint32
pal_in4_ip_packet_len_set (ptp_ip_header_t *iph,
			u_int32_t length)
{
#ifdef _CENTEC_
  iph->total_length = sal_hton16(iph->ihl * 4 + length);
#else
  iph->total_length = iph->ihl * 4 + length;
#endif
  return PM_E_NONE;
}

/* Set length of IP header given IP options length
**
** Parameters:
**   IN/OUT      pal_in4_header      *iph
**   IN          u_int16_t           length of IP options field.
** Results:
**   RESULT_OK for success, < 0 for error
*/
uint32
pal_in4_ip_hdr_len_set (ptp_ip_header_t *iph,
			u_int16_t length)
{
  iph->ihl = (sizeof(ptp_ip_header_t) + length) >> 2;
  return PM_E_NONE;
}

/* Set TOS bits in IP header
** 
** Parameters:
**   IN/OUT      pal_in4_header     *iph
**   IN          s_int32_t          TOS bitmask
*/
uint32
pal_in4_ip_hdr_tos_set (ptp_ip_header_t *iph,
                        int32_t    tos)
{
    #define IPTOS_PREC_INTERNETCONTROL 0xc0
    
  if(tos & PAL_IPTOS_PREC_INTERNETCONTROL)
    iph->tos = IPTOS_PREC_INTERNETCONTROL;

  /* Other TOS bits are currently not used by protocols */
  /* Can add others as required */

  return PM_E_NONE;
}

#define GLB_PTP_IPV4_ETYPE                  0x0800

static void
_ptp_encap_eth_hdr(ptp_sk_buff_t* skb, char *mac_da, char *mac_sa, uint32 is_l2)
{
    eth_hdr_t *p_eth_hdr = (eth_hdr_t *) ptp_skb_push(skb, ETH_HEADER_LEN);
    
    sal_memset(p_eth_hdr, 0x00, ETH_HEADER_LEN);

    sal_memcpy(p_eth_hdr->dest_mac, mac_da, MAC_ADDR_LEN);
    sal_memcpy(p_eth_hdr->src_mac, mac_sa, MAC_ADDR_LEN);
    if (TRUE == is_l2)
    {
        p_eth_hdr->ether_type = sal_hton16(GLB_PTP_1588_ETYPE);
    }
    else
    {
        p_eth_hdr->ether_type = sal_hton16(GLB_PTP_IPV4_ETYPE);
    }
    
    return;
}

uint16
ptp_in_checksum(uint16 *ptr, uint8 input)
{
    register int32       sum = 0;
    uint16               oddbyte = 0;
    register uint16      result = 0;
    uint8                *p, *q;
    uint8                nbytes = input;
    
    sum = 0;
    while (nbytes > 1)  
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) 
    {
        p = (uint8 *)&oddbyte;
        q = (uint8 *)ptr;
        *p = *q;
        sum += oddbyte;
    }

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

static void
_ptp_encap_ip_hdr(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, uint8 msg_type)
{
    uint32 pay_load_len = skb->len;

    ptp_ip_header_t *p_ip_hdr = (ptp_ip_header_t *) ptp_skb_push(skb, IP_HEADER_LEN);
    sal_memset(p_ip_hdr, 0x00, IP_HEADER_LEN);

    p_ip_hdr->version = 4; /*IP version*/
    pal_in4_ip_hdr_len_set(p_ip_hdr, 0);/* header length */

    /*FIXME: see Annex D.5*/
    if (_ptp_is_event_message(msg_type))
    {
        pal_in4_ip_hdr_tos_set(p_ip_hdr, PAL_IPTOS_PREC_INTERNETCONTROL);/* type of service */
    }

    //p_ip_hdr->src_ip_addr = sal_hton32(p_port->port_addr.field.ip_addr.s_addr);/* src ip */
    p_ip_hdr->src_ip_addr = p_port->port_addr.field.ip_addr.s_addr;/* src ip */
    
    #define IPDEFTTL 64
    
    if(PTP_NET_PROTO_UDP_IPV4 == p_port->port_addr.protocol)
    {
        if (_ptp_is_peer_message(msg_type))
        {
            p_ip_hdr->dest_ip_addr = sal_hton32(GLB_PTP_IPV4_ADDRESS_PDELAY);/* dest ip */
            p_ip_hdr->ttl = 1;/* time to live */
        }
        else
        {
            p_ip_hdr->dest_ip_addr = sal_hton32(GLB_PTP_IPV4_ADDRESS_PRIMARY);/* dest ip */
            p_ip_hdr->ttl = IPDEFTTL;/* time to live */
        }
    }
    else
    {
        //p_ip_hdr->dest_ip_addr = sal_hton32(p_port->port_addr.dest_ip_addr.s_addr);/* dest ip */
        p_ip_hdr->dest_ip_addr = p_port->port_addr.dest_ip_addr.s_addr;/* dest ip *//*save network order in system, no need to transform*/
        p_ip_hdr->ttl = IPDEFTTL;/* time to live */
    }

    pal_in4_ip_packet_len_set(p_ip_hdr, pay_load_len);/* total length */

    p_ip_hdr->protocol = IPPROTO_UDP;/* protocol */
    //p_ip_hdr->checksum = sal_hton16(ptp_in_checksum((uint16*) p_ip_hdr, IP_HEADER_LEN));/* checksum */
    //p_ip_hdr->checksum = wrapsum(checksum((uint16*) p_ip_hdr, IP_HEADER_LEN, 0));
    /*new add , ref vrrp*/
    #if 0
    #define	IP_DF 0x4000			/* dont fragment flag */
    #define	IP_MF 0x2000			/* more fragments flag */
    #define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
    #endif
    p_ip_hdr->identification = 0;/* identification */
    p_ip_hdr->offset = sal_hton16(0x4000);/* fragment offset field */
    p_ip_hdr->checksum = ptp_in_checksum((uint16*) p_ip_hdr, IP_HEADER_LEN);/* checksum */
    
    return;
}

static void
_ptp_encap_udp_hdr(ptp_sk_buff_t* skb, uint8 msg_type)
{
    ptp_udp_header_t *p_udp_hdr = (ptp_udp_header_t *) ptp_skb_push(skb, UDP_HEADER_LEN);
    
    sal_memset(p_udp_hdr, 0x00, UDP_HEADER_LEN);
    
    p_udp_hdr->uh_sum = 0; /*Refer to Annex D.1*/
    p_udp_hdr->uh_ulen = sal_hton16(skb->len); /*udp length is include header itself*/
    
    if (_ptp_is_event_message(msg_type))
    {
        p_udp_hdr->uh_dport = sal_hton16(GLB_PTP_UDP_EVENT_PORT);
        p_udp_hdr->uh_sport = sal_hton16(GLB_PTP_UDP_EVENT_PORT);
    }
    else
    {
        p_udp_hdr->uh_dport = sal_hton16(GLB_PTP_UDP_GENERAL_PORT);
        p_udp_hdr->uh_sport = sal_hton16(GLB_PTP_UDP_GENERAL_PORT);
    }
    
    return;
}

int32
_ptp_handle_frame(ptp_sockaddr_t* p_skaddr, ptp_sk_buff_t* skb)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    PTP_PTR_INVALID_RETURN(p_skaddr);
    ptp_common_message_header_t* hdr = NULL;
    uint32 match_l2_encap = 0;
    uint32 match_l4_encap = 0;
    uint32 rx_msg = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
   // uint64 current_time = 0;
    //uint64 s_to_ns = 0;
    //glb_ptp_timeinternal_t time_adjust;
    //uint32 is_negative = FALSE;
    //sal_memset(&time_adjust, 0x0, sizeof(glb_ptp_timeinternal_t));

    tbl_interface_t *p_db_if = tbl_interface_get_interface_by_ifindex(p_skaddr->ifindex);
    if (NULL == p_db_if)
    {
        PTP_DBG_PACKET_DISCARD("Cannot find the interface whose ifindex is %d.", p_skaddr->ifindex);
        return PTP_E_NOT_FOUND;
    }

    tbl_ptp_port_t* p_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&(p_db_if->key)));
    PTP_PTR_INVALID_RETURN(p_port);

    if (!p_port->enable)
    {
        /* if ptp port is not enable, do nothing */
        return PTP_E_SUCCESS;
    }

    /*timestamp is append at the tail of the message*/
    if (skb->len < PTP_TIMESTAMP_BYTES)
    {
        _ptp_sock_stats(p_port, PTP_STATS_DROP, PTP_MSG_TYPE_RESERVED);
        PTP_DBG_PACKET_DISCARD("Unknown PTP message discard in (skb->len (%d) < PTP_TIMESTAMP_BYTES)", skb->len);
        return PTP_E_INVALID_LENGTH;
    }

    glb_ptp_timestamp_t rx_ts;
    sal_memset(&rx_ts, 0x00, sizeof(rx_ts));
    //ptp_skb_pull(skb, PTP_TIMESTAMP_BYTES);
    rx_ts.sec.msb = 0;
    rx_ts.sec.lsb = p_skaddr->seconds;
    rx_ts.ns = p_skaddr->nanoseconds;
#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/dev/console", "a+"); 
    fprintf(fp_console, "111rx_ts.sec.lsb = %u, rx_ts.ns = %u\n", rx_ts.sec.lsb, rx_ts.ns);     
    fprintf(fp_console, "111g_p_master->time_adjust. %"PRId64"\n", g_p_master->time_adjust.v);
#endif
    
#if 0
    fprintf(fp_console, "222rx_ts.sec.lsb = %u, rx_ts.ns = %u\n", rx_ts.sec.lsb, rx_ts.ns);     
    fprintf(fp_console, "222g_p_master->time_adjust. %"PRId64"\n", g_p_master->time_adjust.v);
    fclose(fp_console); 
#endif

/*if use uml the timestamp should be 0*/
#if 0
    rx_ts.sec.lsb = 0;
    rx_ts.ns = 0;
#endif
    //PTP_DBG_PACKET_RX("SOCK_RX Message receipt timestamp {{0x%hx:0x%x}:0x%x}", rx_ts.sec.msb, rx_ts.sec.lsb, rx_ts.ns);

    if (GLB_PTP_IPV4_ETYPE == p_skaddr->ether_type)
    {
        if (skb->len < PTP_UDP_IP_HDR_LEN)
        {
            _ptp_sock_stats(p_port, PTP_STATS_DROP, PTP_MSG_TYPE_RESERVED);
            PTP_DBG_PACKET_DISCARD("Unknown PTP message discard in (skb->len (%d) < PTP_UDP_IP_HDR_LEN (28))", skb->len);
            return PTP_E_INVALID_LENGTH;
        }

        /*strip the ipv4 and udp header and temporary the header is useless*/
        ptp_skb_pull(skb, PTP_UDP_IP_HDR_LEN);
    }

    if (skb->len < PTP_MSG_HEADER_LENGTH)
    {
        _ptp_sock_stats(p_port, PTP_STATS_DROP, PTP_MSG_TYPE_RESERVED);
        PTP_DBG_PACKET_DISCARD("Unknown PTP message discard in (skb->len (%d) < PTP_MSG_HEADER_LENGTH (34))", skb->len);
        return PTP_E_INVALID_LENGTH;
    }

    hdr = (ptp_common_message_header_t *) skb->data; 
  /*
    if(!((PTP_MSG_TYPE_PDELAY_REQ == hdr->message_type) && (!g_p_master->default_ds.two_step_flag)))
    {
        time_adjust.v = g_p_master->time_adjust.v;
        s_to_ns = rx_ts.sec.lsb;
        if(g_p_master->time_adjust.v < 0)
        {
            is_negative = TRUE;
            time_adjust.v = -g_p_master->time_adjust.v;
        }
        if(is_negative)
        {
            current_time = (s_to_ns * GLB_PTP_NS_PER_SEC) + rx_ts.ns;
            current_time = current_time -  time_adjust.v;
        }
        else
        {
            current_time = (s_to_ns * GLB_PTP_NS_PER_SEC) + rx_ts.ns;
            current_time = current_time +time_adjust.v;
        }
        rx_ts.sec.lsb = current_time / GLB_PTP_NS_PER_SEC;
        rx_ts.ns = current_time % GLB_PTP_NS_PER_SEC;
    }
*/
    /*check the version in PTP message*/
    if (hdr->version_ptp != p_port->port_ds.version_number)
    {
        _ptp_sock_stats(p_port, PTP_STATS_DROP, PTP_MSG_TYPE_RESERVED);
        PTP_DBG_PACKET_DISCARD("Unsupported PTP message in version %hhu", hdr->version_ptp);
        return PTP_E_MSG_NOT_CORRESPONDING_VERSION;
    }

    /*check the length in PTP message*/
    if (skb->len < sal_ntoh16(hdr->message_length))
    {
        _ptp_sock_stats(p_port, PTP_STATS_DROP, hdr->message_type);
        PTP_DBG_PACKET_DISCARD("Discard %s by invalid length %d < hdr message_length %d", ptp_desc_msg_type(hdr->message_type), skb->len, sal_ntoh16(hdr->message_length));
        return PTP_E_INVALID_LENGTH;
    }

    if ((p_port->vlan_info.vid != p_skaddr->vlan_id) && 1 != p_skaddr->vlan_id)
    {
        PTP_LOG_WARN("Message vlan tagging %d mismatch with interface %s vlan tagging %d",
                    p_skaddr->vlan_id, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_port->vlan_info.vid);
        PTP_DBG_PACKET_DISCARD("Discard %s by mismatch tagging vlan %d, %d with interface %s", 
        ptp_desc_msg_type(hdr->message_type), p_skaddr->vlan_id, p_port->vlan_info.vid, ifname_ext);
        _ptp_sock_stats(p_port, PTP_STATS_DROP, hdr->message_type);
        return PTP_E_SUCCESS;
    }

    match_l2_encap = ((GLB_PTP_1588_ETYPE == p_skaddr->ether_type)
                            && (PTP_NET_PROTO_IEEE_802_3 == p_port->port_addr.protocol));
    match_l4_encap = ((GLB_PTP_IPV4_ETYPE == p_skaddr->ether_type)
                            && ((PTP_NET_PROTO_UDP_IPV4 == p_port->port_addr.protocol)
                                   ||PTP_NET_PROTO_UDP_IPV4_UNICAST == p_port->port_addr.protocol));
    if ((!match_l2_encap) && (!match_l4_encap))
    {
        PTP_LOG_WARN("Message ethertype 0x%.04hx mismatch with interface %s encap %s", p_skaddr->ether_type,
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), ptp_desc_encap(p_port->port_addr.protocol));
        PTP_DBG_PACKET_DISCARD("Discard %s by mismatch encap: packet ethertype 0x%.04hx, interface %s encap %s ", 
            ptp_desc_msg_type(hdr->message_type), p_skaddr->ether_type, IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_encap(p_port->port_addr.protocol));
        _ptp_sock_stats(p_port, PTP_STATS_DROP, hdr->message_type);
        return PTP_E_MSG_ERROR_ENCAP;
    }

    PTP_DBG_PACKET_RX("RCV %s %s from %s, %s tc, recommended_state %s, delay_mechanism %s", 
        match_l2_encap ? "L2" : "L4", ptp_desc_msg_type(hdr->message_type), IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_ops_is_tc() ? "" : "not", 
        ptp_desc_state(p_port->recommended_state) , ptp_desc_delay_mechanism(p_port->port_ds.delay_mechanism));
        
    rx_msg = FALSE;
    if (p_port->msg_proc_ops.handle_proc[hdr->message_type])
    {
        if (ptp_ops_is_tc())
        {
            if (hdr->domain_number == g_p_master->default_ds.primary_domain)
            {
                rx_msg = p_port->msg_proc_ops.handle_proc[hdr->message_type](p_port, skb, &rx_ts);
            }
        }
        else
        {
            if (hdr->domain_number == g_p_master->default_ds.domain_number)
            {
                rx_msg = p_port->msg_proc_ops.handle_proc[hdr->message_type](p_port, skb, &rx_ts);
            }
        }
    }

    uint32 fwd_msg = FALSE;
    if (p_port->msg_proc_ops.forward_proc[hdr->message_type])
    {
        fwd_msg = p_port->msg_proc_ops.forward_proc[hdr->message_type](p_port, skb, &rx_ts);
    }

    if (rx_msg || fwd_msg)
    {
        //PTP_DBG_PACKET_RX("MSG_RECV Receipt %s, count++", ptp_desc_msg_type(hdr->message_type));
        _ptp_sock_stats(p_port, PTP_STATS_RX, hdr->message_type);
    }
    else
    {
        PTP_DBG_PACKET_DISCARD("Discard %s for msg proc failed", ptp_desc_msg_type(hdr->message_type));
        _ptp_sock_stats(p_port, PTP_STATS_DROP, hdr->message_type);
    }

    return PTP_E_SUCCESS;
}

int32
_ptp_send_pkt(uint8 *p_pkt, uint32 pkt_len, ptp_sockaddr_t *p_skaddr, uint32 aux_len, uint32 ifindex)
{
    uint32 flags = 0;
    
    if (TRUE != p_skaddr->is_unicast)
    {
        //flags |= GLB_PKT_FLAGS_TX_MCAST;
    }
    
    if (ipc_pkt_tx_ptp(p_pkt, pkt_len, (uint8 *)p_skaddr, aux_len, ifindex, flags))
    {
        PTP_LOG_ERR("send msg through port %d fail", ifindex);
        
        return PM_E_SOCK;
    } 

    return PM_E_NONE;
}

static int32
_ptp_send_l2(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* p_rx_ts)
{
    int32 ret = 0;
    uint8 ptp_msg_type = 0;
    ptp_common_message_header_t *hdr;
    hdr = (ptp_common_message_header_t *) skb->data;

    ptp_msg_type = hdr->message_type;
    ptp_sockaddr_t skaddr;
    sal_memset(&skaddr, 0x00, sizeof(ptp_sockaddr_t));
    if (_ptp_is_event_message(ptp_msg_type))
    {
        /*one step sync*/
        if ((ptp_msg_type == PTP_MSG_TYPE_SYNC) && (!hdr->flag_field_0_two_step_flag))
        {
            skaddr.replace = TRUE;
        }
        #ifdef DUET2
        //add by weij for bug 45114, use uninterrupt mode
        /*two step sync*/
        else if ((ptp_msg_type == PTP_MSG_TYPE_SYNC) && (hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            //tmp for capture with sdk unsupport
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        else if (ptp_msg_type == PTP_MSG_TYPE_DELAY_REQ)
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        else if (ptp_msg_type == PTP_MSG_TYPE_PDELAY_REQ)
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        #endif
        /*one step pdelay resp*/
        else if((ptp_msg_type == PTP_MSG_TYPE_PDELAY_RESP) && (!hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.correction = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
            //PTP_LOG_ERR("send ptp msg_pdelay_resp with rx time,  second = %u, nanosecond = %u",  skaddr.seconds, skaddr.nanoseconds);
        }
        #ifdef DUET2
        //add by weij for bug 45114, use uninterrupt mode
        /*two step pdelay resp*/
        else if((ptp_msg_type == PTP_MSG_TYPE_PDELAY_RESP) && (hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            //tmp for capture with sdk unsupport
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        #endif
        else
        {
            skaddr.capture = TRUE;
            skaddr.seq_id = p_port->ptp_seq_id;
            p_port->ptp_seq_id ++;
        }
      //  skaddr.ts_offset = PTP_MSG_TS_OFFSET;
    }

    skaddr.ifindex = p_port->ifindex;
    if (_ptp_is_peer_message(ptp_msg_type))
    {
        mac_addr_t dest_addr = GLB_PTP_MAC_ADDRESS_PDELAY;
        sal_memcpy(skaddr.dest_mac, dest_addr, GLB_ETH_ADDR_LEN);
    }
    else
    {
        mac_addr_t dest_addr = GLB_PTP_MAC_ADDRESS_DEFAULT;
        sal_memcpy(skaddr.dest_mac, dest_addr, GLB_ETH_ADDR_LEN);
    }

    skaddr.vlan_id = p_port->vlan_info.vid;
    skaddr.vlan_priority = p_port->vlan_info.cos;

    skaddr.ether_type = GLB_PTP_1588_ETYPE;
    skaddr.ptp_msg_type = ptp_msg_type;
    skaddr.ptp_offset = 14;

    if(skaddr.vlan_id)
    {
         skaddr.ptp_offset = skaddr.ptp_offset + 4;
    }

    PTP_DBG_PACKET_TX("Send L2 %s", ptp_desc_msg_type(ptp_msg_type));

    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    /*encap the eth header*/
    _ptp_encap_eth_hdr(skb, (char *)skaddr.dest_mac, (char *)p_sys_glb->route_mac, TRUE);

    PTP_DESC_BUFFER_((char *)skb->data, skb->len, TRUE);

    if (_ptp_send_pkt(skb->data, skb->len, &skaddr, sizeof(skaddr), p_port->ifindex))
    {
        PTP_LOG_ERR("send msg through port %d fail", p_port->ifindex);
        
        return PM_E_SOCK;
    } 

    if (ret < 0)
    {
        PTP_LOG_ERR("PDU[SEND]: send failed (ret %d)", ret);
        return PTP_E_SOCK_SEND;
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_send_l4(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* p_rx_ts)
{
    int32 ret = 0;
    uint8 ptp_msg_type = 0;
    ptp_common_message_header_t *hdr;
    hdr = (ptp_common_message_header_t *) skb->data;

    ptp_msg_type = hdr->message_type;
    ptp_sockaddr_t skaddr;
    sal_memset(&skaddr, 0x00, sizeof(ptp_sockaddr_t));
    if (_ptp_is_event_message(ptp_msg_type))
    {
        /*transparent clock: always two step, should not replace timestamp field*/
        if ((ptp_msg_type == PTP_MSG_TYPE_SYNC) && (!hdr->flag_field_0_two_step_flag))
        {
            /*Delay_Req and Pdelay_Req can always have t1 in message*/
            skaddr.replace = TRUE;
        }
        #ifdef DUET2
        //add by weij for bug 45114, use uninterrupt mode
        else if ((ptp_msg_type == PTP_MSG_TYPE_SYNC) && (hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            //tmp for capture with sdk unsupport
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        else if (ptp_msg_type == PTP_MSG_TYPE_DELAY_REQ)
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        else if (ptp_msg_type == PTP_MSG_TYPE_PDELAY_REQ)
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        #endif
        else if((ptp_msg_type == PTP_MSG_TYPE_PDELAY_RESP) && (!hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            skaddr.correction = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        #ifdef DUET2
        //add by weij for bug 45114, use uninterrupt mode
        else if((ptp_msg_type == PTP_MSG_TYPE_PDELAY_RESP) && (hdr->flag_field_0_two_step_flag))
        {
            if (!p_rx_ts)
            {
                PTP_LOG_ERR("p_rx_ts is NULL");
                return PTP_E_SOCK_SEND;
            }
            
            //tmp for capture with sdk unsupport
            skaddr.capture = TRUE;
            skaddr.seconds = p_rx_ts->sec.lsb;
            skaddr.nanoseconds = p_rx_ts->ns;
        }
        #endif
        else
        {
            skaddr.capture = TRUE;
            skaddr.seq_id = p_port->ptp_seq_id;
            p_port->ptp_seq_id ++; 
        }
        //skaddr.ts_offset = PTP_UDP_IP_HDR_LEN + PTP_MSG_TS_OFFSET;
    }
    
    if(PTP_NET_PROTO_UDP_IPV4_UNICAST == p_port->port_addr.protocol)
    {
        skaddr.is_unicast = TRUE;
    }
    skaddr.ifindex = p_port->ifindex;
    
    if (_ptp_is_peer_message(ptp_msg_type))
    {
        mac_addr_t dest_addr = GLB_PTP_IP_MAC_ADDRESS_PDELAY;
        sal_memcpy(skaddr.dest_mac, dest_addr, GLB_ETH_ADDR_LEN);
    }
    else
    {
        mac_addr_t dest_addr = GLB_PTP_IP_MAC_ADDRESS_PRIMARY;
        sal_memcpy(skaddr.dest_mac, dest_addr, GLB_ETH_ADDR_LEN);
    }

    skaddr.vlan_id = p_port->vlan_info.vid;
    skaddr.vlan_priority = p_port->vlan_info.cos;

    skaddr.ether_type = GLB_PTP_IPV4_ETYPE;

    skaddr.ptp_offset = 14 + PTP_UDP_IP_HDR_LEN;

    skaddr.ptp_msg_type = ptp_msg_type;
    if(skaddr.vlan_id)
    {
        skaddr.ptp_offset = skaddr.ptp_offset + 4;
    }

    PTP_DBG_PACKET_TX("Send L4 %s", ptp_desc_msg_type(ptp_msg_type));
    
    /*encap the udp header*/
    _ptp_encap_udp_hdr(skb, ptp_msg_type);

    /*encap the ip header*/
    _ptp_encap_ip_hdr(p_port, skb, ptp_msg_type);

    if(PTP_NET_PROTO_UDP_IPV4_UNICAST == p_port->port_addr.protocol)
    {
        tbl_arp_t * p_arp = NULL;
        tbl_arp_key_t arp_key;

        sal_memset(&arp_key, 0x00, sizeof(arp_key));
        sal_memcpy(&(arp_key.ip), &(p_port->port_addr.dest_ip_addr), sizeof(addr_ipv4_t));
        p_arp = tbl_arp_get_arp(&arp_key);
        if (NULL != p_arp)
        {
            sal_memcpy(skaddr.dest_mac, p_arp->mac_addr, GLB_ETH_ADDR_LEN);
        }
        else
        {
            char ip_str[32] = {0};
            PTP_DBG_PACKET_TX("Get arp failed for %s", cdb_addr_ipv4_val2str(ip_str, sizeof(ip_str), &(p_port->port_addr.dest_ip_addr)));
        }
    }
    
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    /*encap the eth header*/
    _ptp_encap_eth_hdr(skb, (char *)skaddr.dest_mac, (char *)p_sys_glb->route_mac, FALSE);

    if (_ptp_send_pkt(skb->data, skb->len, &skaddr, sizeof(skaddr), p_port->ifindex))
    {
        PTP_LOG_ERR("send msg through port %d fail", p_port->ifindex);
        return PM_E_SOCK;
    } 
    
    if (ret < 0)
    {
        PTP_LOG_ERR("PDU[SEND]: send failed (ret %d)", ret);
        return PTP_E_SOCK_SEND;
    }

    return PTP_E_SUCCESS;
}

int32
ptp_sock_send(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* p_rx_ts)
{
    ptp_common_message_header_t *hdr;
    hdr = (ptp_common_message_header_t *) skb->data;
    ptp_msg_type_t message_type = hdr->message_type;

    switch (p_port->port_addr.protocol)
    {
    case PTP_NET_PROTO_IEEE_802_3:
        PTP_OP_ERROR_RETURN(_ptp_send_l2(p_port, skb, p_rx_ts));
        break;
    case PTP_NET_PROTO_UDP_IPV4:
    case   PTP_NET_PROTO_UDP_IPV4_UNICAST:
        PTP_OP_ERROR_RETURN(_ptp_send_l4(p_port, skb, p_rx_ts));
        break;
    default:
        PTP_LOG_ERR("Send PTP message in unsupported encapsulation.");
        return PTP_E_SOCK_SEND;
    }

    _ptp_sock_stats(p_port, PTP_STATS_TX, message_type);
    
    return PTP_E_SUCCESS;
}

int32
ptp_sock_send_w_ts_resp(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* tx_ts, glb_ptp_timestamp_t* p_rx_ts)
{
    int32 ret = ptp_adpt_reset_tx_ts();
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to reset TX timestamp");
        return ret;
    }

    ret = ptp_sock_send(p_port, skb, p_rx_ts);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to send PTP message to %s", ptp_desc_ifname(p_port));
        return ret;
    }

    #ifndef _GLB_UML_SYSTEM_
        ret = ptp_adpt_get_tx_ts(p_port->ifindex, tx_ts, ((p_port->ptp_seq_id - 1) % 4), p_port->key.name);
        if (PTP_E_IF_BLOCKED == ret)
        {
            PTP_LOG_ERR("Failed to get TX timestamp for blocked");
            return ret;
        }
        else if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_ERR("Failed to get TX timestamp");
            return ret;
        }
        char ifname_ext[IFNAME_EXT_SIZE] = {0};
        PTP_DBG_PACKET_TX("Get TX ts %hu%u:%u for %s",
                tx_ts->sec.msb, tx_ts->sec.lsb, tx_ts->ns, IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    #else
        sal_memset(tx_ts, 0x00, sizeof(glb_ptp_timestamp_t));
    #endif

    return PTP_E_SUCCESS;
}

int32
ptp_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    ptp_sockaddr_t *p_skaddr;
    ptp_sk_buff_t* skb;
    uint32 msg_len = 0;
    uint32 skaddr_len = 0;
    eth_hdr_t *eth_p = NULL;
    uint8 *buf = NULL;
    int32 ret = 0;
    uint8 *skb_data;
    uint16 rm_vlan_tag_length = 0;

    skb = ptp_alloc_skb(PTP_RCV_BUF);
    if (NULL == skb)
    {
        PTP_LOG_ERR("PDU[RECV]: no memory to receive PTP messages.");
        ret = PTP_E_NO_MEMORY;
        ptp_free_skb(skb);
        return ret;
    }
    
    skaddr_len = sizeof(ptp_sockaddr_t);
    if (p_msg->data_len < skaddr_len)
    {
        ptp_free_skb(skb);
        return PM_E_FAIL;
    }

    /*get sockaddr*/
    msg_len = p_msg->data_len - skaddr_len;
    p_skaddr = (ptp_sockaddr_t*)((uint8*)p_msg->data + msg_len);

    /*msg with skaddr at tail*/
    PTP_DESC_BUFFER_((char *)p_msg->data, p_msg->data_len, FALSE);
    
    //ptp_desc_ptp_sockaddr(p_skaddr);
    #if 0
    char buf_dest_mac[16] = {0};
    PTP_DBG_PACKET_RX("sockaddr:ptp_msg_type %s ifindex %d seconds %d nanoseconds %d capture %d correction %d dest_mac %s ether_type 0x%x is_unicast %d  ptp_offset %d replace %d seq_id %d ts_offset %d vlan %d vlan_priority %d\n", 
            ptp_desc_msg_type(p_skaddr->ptp_msg_type), p_skaddr->ifindex, p_skaddr->seconds, p_skaddr->nanoseconds, p_skaddr->capture, p_skaddr->correction, 
            cdb_mac_addr_val2str(buf_dest_mac, sizeof(buf_dest_mac), p_skaddr->dest_mac), p_skaddr->ether_type, p_skaddr->is_unicast,  
            p_skaddr->ptp_offset, p_skaddr->replace,p_skaddr->seq_id, p_skaddr->ts_offset, p_skaddr->vlan_id, p_skaddr->vlan_priority);
    #endif
    buf = p_msg->data;
    eth_p = (eth_hdr_t*)buf;

    /*msg strip skaddr*/
    PTP_DESC_BUFFER_((char *)buf, msg_len, FALSE);
    
    ptp_eth_pkt_move_pld(buf, skb_data);

    p_skaddr->ifindex = p_msg->u_hdr.pkt.ifindex;
    p_skaddr->ether_type = sal_ntoh16(eth_p->ether_type);

    /*vlan process*/
    if (0x8100 == p_skaddr->ether_type || 0x9100 == p_skaddr->ether_type || 0x88a8 == p_skaddr->ether_type)
    {
        uint16* p16;
        uint16 ether_type = 0;
        //uint16 vlan_cos = 0;
        //uint16 vlan = 0;
        //uint8 cos = 0;

        p16 = (uint16 *) (skb_data);
        //vlan_cos = sal_ntoh16(*p16);

        /*outer vlan*/
        //vlan = vlan_cos & 0x0fff;
        //cos = vlan_cos >> 13;
        //PTP_DBG_PACKET_RX("get vlan tag: vlan %d vlan_priority %d\n", vlan, cos);
        
        /*mv cos + vlan*/
        ptp_cos_vlan_pkt_move_pld(skb_data, skb_data);
        rm_vlan_tag_length += 2;

        p16 = (uint16 *) (skb_data);
        ether_type = sal_ntoh16(*p16);

        /*cover vlan ether type*/
        p_skaddr->ether_type = ether_type;
        
        if (0x8100 == ether_type || 0x9100 == ether_type || 0x88a8 == ether_type)
        {
            /*inner vlan :mv vlan tag*/
            ptp_vlan_tag_pkt_move_pld(skb_data, skb_data);
            rm_vlan_tag_length += 4;

            p16 = (uint16 *) (skb_data);
            ether_type = sal_ntoh16(*p16);

            /*cover vlan ether type*/
            p_skaddr->ether_type = ether_type;
        }
        
        /*mv ether_type*/
        ptp_cos_vlan_pkt_move_pld(skb_data, skb_data);
        rm_vlan_tag_length += 2;
    }
    
    PTP_DESC_BUFFER_((char *)buf, msg_len, FALSE);
    sal_memcpy(skb->data, skb_data, msg_len - ETH_HEADER_LEN - rm_vlan_tag_length);
    skb->len = msg_len - ETH_HEADER_LEN - rm_vlan_tag_length;
    skb->tail += skb->len;
    
    //skb.head = ;
    //skb.end = ;
    
    /* Handle PTP PDU. */
    _ptp_handle_frame(p_skaddr, skb);

    ptp_free_skb(skb);
    return PM_E_NONE;
}


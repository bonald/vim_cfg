#ifndef __ARP_ETHER_H__
#define __ARP_ETHER_H__

/* error code */
enum arp_eth_err_type_e
{
    ARP_ETH_SUCCESS = 0,

    ARP_ETH_ERR_PTR = -2000,
    ARP_ETH_ERR_FAIL,
    ARP_ETH_ERR_NOT_SUP,
    ARP_ETH_PKT_SEND_FAIL,
    ARP_ETH_ERR_IS_ROUTER,
    ARP_ETH_IF_NOT_MATCH,
    ARP_ETH_PKT_INVALID,
    ARP_ETH_MACFILTER,
    ARP_ETH_IP_INVALID,
    ARP_ETH_MAC_INVALID,
    ARP_ETH_VRRP_MAC_INVALID
};

#define arp_pkt_move_pld(buf, arp_p)    \
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += ETH_HEADER_LEN;       \
    arp_p = (arp_hdr_t*)p;              \
} while(0)

#define arp_eth_pkt_move_pld(buf, arp_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += ETH_HEADER_LEN;       \
    arp_p = (arp_eth_t*)p;              \
} while(0)


/* ARP packet over ethernet
 *
 *           +--------+--------+--------+--------+
 *           |  Haedware Type  |  Protocol Type  |
 *           +--------+--------+--------+--------+
 *           |  HLEN  |  PLEN  |    Operation    |
 *           +--------+--------+--------+--------+
 *           |            Sender MAC             |
 *           +--------+--------+--------+--------+
 *           |   Sender MAC    |    Sender IP    |
 *           +--------+--------+--------+--------+
 *           |   Sender IP     |  Target MAC     |
 *           +--------+--------+--------+--------+
 *           |             Target MAC            |
 *           +--------+--------+--------+--------+
 *           |             Target IP             |
 *           +--------+--------+--------+--------+
 */


/* send an arp packet to network */
int32
arp_ether_packet_tx(tbl_arp_t *ae_p, arp_pkt_op_t op, arp_pkt_type_t type, uint8 is_gra);

/* receive an arp packet over ethernet from network */
int32
arp_ether_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

/* ether link address update */
int32
arp_ether_lladdr_update(tbl_arp_t *ae_p, uint8 *mac);

#endif /* !__ARP_ETHER_H__ */


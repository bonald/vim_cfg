#ifndef __ARP_DEFINE_H__
#define __ARP_DEFINE_H__

#define ROUTE_ARP_AGING_SCAN_INTERVAL   1  /* 1 second */

#define ROUTE_ARP_RETRY_TIME            3

/* send or receive */
typedef enum
{
    ARP_ACT_SEND,
    ARP_ACT_REV,
    
    ARP_ACT_MAX
} arp_pkt_action_t;

/* add or remove */
typedef enum
{
    ARP_ENTRY_ADD,
    ARP_ENTRY_DEL,
    ARP_ENTRY_UP,
    
    ARP_ENTRY_OP_MAX
} arp_entry_action_t;

/* ARP protocol HARDWARE identifiers. */
typedef enum
{
    ARP_HRD_NETROM          = 0,        /* from KA9Q: NET/ROM pseudo    */
    ARP_HRD_ETHER           = 1,        /* Ethernet 10Mbps      */
    ARP_HRD_EETHER          = 2,        /* Experimental Ethernet    */
    ARP_HRD_AX25            = 3,        /* AX.25 Level 2        */
    ARP_HRD_PRONET          = 4,        /* PROnet token ring        */
    ARP_HRD_CHAOS           = 5,        /* Chaosnet         */
    ARP_HRD_IEEE802         = 6,        /* IEEE 802.2 Ethernet/TR/TB    */
    ARP_HRD_ARCNET          = 7,        /* ARCnet           */
    ARP_HRD_APPLETLK        = 8,        /* APPLEtalk            */
    ARP_HRD_DLCI            = 15,       /* Frame Relay DLCI     */
    ARP_HRD_ATM             = 19,       /* ATM              */
    ARP_HRD_METRICOM        = 23,       /* Metricom STRIP (new IANA id) */
    ARP_HRD_IEEE1394        = 24,       /* IEEE 1394 IPv4 - RFC 2734    */
    ARP_HRD_EUI64           = 27,       /* EUI-64                       */
    ARP_HRD_INFINIBAND      = 32,       /* InfiniBand           */

/* Dummy types for non ARP hardware */
    ARP_HRD_SLIP            = 256,
    ARP_HRD_CSLIP           = 257,
    ARP_HRD_SLIP6           = 258,
    ARP_HRD_CSLIP6          = 259,
    ARP_HRD_RSRVD           = 260,      /* Notional KISS type       */
    ARP_HRD_ADAPT           = 264,
    ARP_HRD_ROSE            = 270,
    ARP_HRD_X25             = 271,      /* CCITT X.25           */
    ARP_HRD_HWX25           = 272,      /* Boards with X.25 in firmware */
    ARP_HRD_PPP             = 512,
    ARP_HRD_CISCO           = 513,      /* Cisco HDLC           */
    ARP_HRD_HDLC            = ARP_HRD_CISCO,
    ARP_HRD_LAPB            = 516,      /* LAPB             */
    ARP_HRD_DDCMP           = 517,      /* Digital's DDCMP protocol     */
    ARP_HRD_RAWHDLC         = 518,      /* Raw HDLC         */

    ARP_HRD_TUNNEL          = 768,      /* IPIP tunnel          */
    ARP_HRD_TUNNEL6         = 769,      /* IP6IP6 tunnel            */
    ARP_HRD_FRAD            = 770,      /* Frame Relay Access Device    */
    ARP_HRD_SKIP            = 771,      /* SKIP vif         */
    ARP_HRD_LOOPBACK        = 772,      /* Loopback device      */
    ARP_HRD_LOCALTLK        = 773,      /* Localtalk device     */
    ARP_HRD_FDDI            = 774,      /* Fiber Distributed Data Interface */
    ARP_HRD_BIF             = 775,             /* AP1000 BIF                   */
    ARP_HRD_SIT             = 776,      /* sit0 device - IPv6-in-IPv4   */
    ARP_HRD_IPDDP           = 777,      /* IP over DDP tunneller    */
    ARP_HRD_IPGRE           = 778,      /* GRE over IP          */
    ARP_HRD_PIMREG          = 779,      /* PIMSM register interface */
    ARP_HRD_HIPPI           = 780,      /* High Performance Parallel Interface */
    ARP_HRD_ASH             = 781,      /* Nexus 64Mbps Ash     */
    ARP_HRD_ECONET          = 782,      /* Acorn Econet         */
    ARP_HRD_IRDA            = 783,      /* Linux-IrDA           */
    /* ARP works differently on different FC media .. so  */
    ARP_HRD_FCPP            = 784,      /* Point to point fibrechannel  */
    ARP_HRD_FCAL            = 785,      /* Fibrechannel arbitrated loop */
    ARP_HRD_FCPL            = 786,      /* Fibrechannel public loop */
    ARP_HRD_FCFABRIC        = 787,      /* Fibrechannel fabric      */
    /* 787->799 reserved for fibrechannel media types */
    ARP_HRD_IEEE802_TR      = 800,      /* Magic type ident for TR  */
    ARP_HRD_IEEE80211       = 801,      /* IEEE 802.11          */
    ARP_HRD_IEEE80211_PRISM = 802,      /* IEEE 802.11 + Prism2 header  */
} arp_l2_type_t;

typedef enum
{
    ARP_OP_REQUEST      = 1,    /* ARP request          */
    ARP_OP_REPLY        = 2,    /* ARP reply            */
    ARP_OP_RREQUEST     = 3,    /* RARP request         */
    ARP_OP_RREPLY       = 4,    /* RARP reply           */
    ARP_OP_InREQUEST    = 8,    /* InARP request        */
    ARP_OP_InREPLY      = 9,    /* InARP reply          */
    ARP_OP_NAK          = 10,   /* (ATM)ARP NAK         */
    ARP_OP_MAX,
} arp_pkt_op_t;

typedef enum
{
    ARP_UNICAST         = 1,    /* unicast */
    ARP_BROADCAST       = 2,    /* broadcast */
} arp_pkt_type_t;

/* arp synchronize operation */
typedef enum 
{
    ARP_SYN_ADD = 1,
    ARP_SYN_DEL = 2,
    ARP_SYN_MAX
} arp_syn_op_t;

/* arp event */
typedef enum
{
    AE_TX_ARP_REQ  = 0, /* sent ARP Request */
    AE_TX_GARP_REQ    , /* sent Gratuitous ARP Request to network */
    AE_RX_ARP_RSP     , /* receive ARP Reply */
    AE_RX_ARP_REQ     , /* receive ARP Request */
    AE_RETRY_EXP      , /* retry timer expires */
    AE_AGING_EXP      , /* aging timer expires */
    AE_REFRESH_EXP    , /* refresh timer expires */
    AE_DELETE_ENTRY   , /* delete ARP entry */
    AE_MAX    
} arp_event_t;

/* arp header */
typedef struct
{
    uint16      ar_hrd;     /* format of hardware address   */
    uint16      ar_pro;     /* format of protocol address   */
    uint8       ar_hln;     /* length of hardware address   */
    uint8       ar_pln;     /* length of protocol address   */
    uint16      ar_op;      /* ARP opcode (command)     */
} arp_hdr_t;

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

/* arp packet over ethernet */
typedef struct
{
    uint16  ar_hrd;     /* format of hardware address   */
    uint16  ar_pro;     /* format of protocol address   */
    uint8   ar_hln;     /* length of hardware address   */
    uint8   ar_pln;     /* length of protocol address   */
    uint16  ar_op;      /* ARP opcode (command)         */

    uint8   ar_sha[MAC_ADDR_LEN];       /* sender hardware address  */
    uint8   ar_sip[IPV4_MAX_BYTELEN];   /* sender IP address        */
    uint8   ar_tha[MAC_ADDR_LEN];       /* target hardware address  */
    uint8   ar_tip[IPV4_MAX_BYTELEN];   /* target IP address        */
} arp_eth_t;

#define ARP_PDU_LEN             28      /* sizeof(arp_eth_t) */
#define ARP_TOTAL_PDU_LEN       ETH_HEADER_LEN + ARP_PDU_LEN    /* NOT consider vlan */

#endif /* !__ARP_DEFINE_H__ */


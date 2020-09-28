

#ifndef __GLB_IF_DEFINE_H__
#define __GLB_IF_DEFINE_H__
#include "glb_chip_specific.h"
#include "glb_const.h"

#define GLB_KNAME_TO_UNAME(kname)
#define GLB_UNAME_TO_KNAME(uname)

#define GLB_MAX_L3IF_NUM                1024+256

#define GLB_MAX_VLAN_IF_NUMBER          256

#define GLB_IFNAME_ETH_PREFIX           "eth-"
#define GLB_IFNAME_AGG_PREFIX           "agg"
#define GLB_IFNAME_VLAN_PREFIX          "vlan"
#define GLB_IFNAME_TUNNEL_PREFIX        "tunnel"
#define GLB_IFNAME_LOOPBACK_PREFIX      "loopback"
#define GLB_IFNAME_L2GRE_PREFIX         "l2gre"
#define GLB_IFNAME_L3GRE_PREFIX         "l3gre"
#define GLB_IFNAME_NVGRE_PREFIX         "nvgre"
#define GLB_IFNAME_VXLAN_PREFIX         "vxlan"
#define GLB_IFNAME_CPU_PREFIX           "cpu"

#define GLB_IFNAME_ETH_PREFIX_LEN       4
#define GLB_IFNAME_AGG_PREFIX_LEN       3
#define GLB_IFNAME_VLAN_PREFIX_LEN      4
#define GLB_IFNAME_TUNNEL_PREFIX_LEN    6
#define GLB_IFNAME_LOOPBACK_PREFIX_LEN  8
#define GLB_IFNAME_L2GRE_PREFIX_LEN     5
#define GLB_IFNAME_L3GRE_PREFIX_LEN     5
#define GLB_IFNAME_NVGRE_PREFIX_LEN     5
#define GLB_IFNAME_VXLAN_PREFIX_LEN     5
#define GLB_IFNAME_CPU_PREFIX_LEN       3

#define GLB_IF_SAI_OBJECT_TYPE_PORT 1
#define GLB_IF_SAI_OBJECT_TYPE_COPP 33

#define GLB_IFINDEX_ALL             (0x0)
#define GLB_PORT_NUM_PER_SLOT       (0x40)
#define GLB_PHY_IFINDEX_MIN         (0x0)
#define GLB_PHY_IFINDEX_MAX         (0x7FF)
#define GLB_AGG_IFINDEX_MIN         (0x800)
#define GLB_AGG_IFINDEX_MAX         (0xFFF)
#define GLB_VLAN_IFINDEX_MIN        (0x1000)
#define GLB_VLAN_IFINDEX_MAX        (0x1FFF)
#define GLB_TUNNEL_IFINDEX_MIN      (0x2000)
#define GLB_TUNNEL_IFINDEX_MAX      (0x2FFF)
#define GLB_LB_IFINDEX_MIN          (0x3000)
#define GLB_LB_IFINDEX_MAX          (0x3FFF)
#define GLB_L2GRE_IFINDEX_MIN       (0x4000)
#define GLB_L2GRE_IFINDEX_MAX       (0x43FF)
#define GLB_L3GRE_IFINDEX_MIN       (0x4400)
#define GLB_L3GRE_IFINDEX_MAX       (0x47FF)
#define GLB_NVGRE_IFINDEX_MIN       (0x4800)
#define GLB_NVGRE_IFINDEX_MAX       (0x4BFF)
#define GLB_VXLAN_IFINDEX_MIN       (0x4C00)
#define GLB_VXLAN_IFINDEX_MAX       (0x4FFE)
#define GLB_CPU_IFINDEX_MIN         (0x4FFF)
#define GLB_CPU_IFINDEX_MAX         (GLB_CPU_IFINDEX_MIN)
#define GLB_OTHER_IFINDEX_BASE      (GLB_CPU_IFINDEX_MAX+1)

/* Added by weizj for inband */
#define GLB_VIF_BR0_IFINDEX         (GLB_OTHER_IFINDEX_BASE+1)
#define GLB_VIF_BR0_GPORT           0xFF

#define GLB_INVALID_IFINDEX         0
#define GLB_INVALID_GPORTID         0xFFFF
#define GLB_INVALID_VLANID          0xFFFF

#define GLB_FEC_OFF    0 /* force fec none, when auto, need  GLB_FEC_AUTO|GLB_FEC_OFF*/
#define GLB_FEC_RS      (1<<0) /* force fec rs, when auto, need  GLB_FEC_AUTO|GLB_FEC_RS*/
#define GLB_FEC_BASE_R  (1<<1) /* force fec baser, when auto, need  GLB_FEC_AUTO|GLB_FEC_BAST_R*/
#define GLB_FEC_DEFAULT    (1<<7)
#define GLB_FEC_ALL    (3<<0) /* auto both baser and rs */
#define GLB_FEC_MASK    0x0f
#define GLB_FEC_DEFAULT_MASK 0x80
#define GLB_GG_FEC_SUPPORT 2
#define GLB_D2_FEC_SUPPORT 1
#if (defined DUET2) || (defined TSINGMA)
/* define cl73 ability, same as ctc_port_cl73_ability_t, here is for show ability in nsm */
#define GLB_CL73_10GBASE_KR       (1 << 0 )      /**< [GG.D2] 10GBase-KR ability */
#define GLB_CL73_40GBASE_KR4      (1 << 1 )     /**< [GG.D2] 40GBase-KR4 ability */
#define GLB_CL73_40GBASE_CR4      (1 << 2 )     /**< [GG.D2] 40GBase-CR4 ability */
#define GLB_CL73_100GBASE_KR4     (1 << 3 )    /**< [GG.D2] 100GBase-KR4 ability */
#define GLB_CL73_100GBASE_CR4     (1 << 4 )    /**< [GG.D2] 100GBase-CR4 ability */
#define GLB_CL73_FEC_ABILITY      (1 << 5 )      /**< [GG.D2] BASE-R FEC supported*/
#define GLB_CL73_FEC_REQUESTED    (1 << 6)      /**< [GG.D2] BASE-R requested */

#define GLB_CL73_25GBASE_KRS                 (1 << 7)  /**< [D2] 25GBase-KRS ability */
#define GLB_CL73_25GBASE_CRS                 (1 << 7)  /**< [D2] 25GBase-CRS ability */
#define GLB_CL73_25GBASE_KR                  (1 << 8)  /**< [D2] 25GBase-CRS ability */
#define GLB_CL73_25GBASE_CR                  (1 << 8)  /**< [D2] 25GBase-CR ability */
#define GLB_CL73_25G_RS_FEC_REQUESTED        (1 << 9)  /**< [D2] 25G RS-FEC requested */
#define GLB_CL73_25G_BASER_FEC_REQUESTED     (1 << 10)  /**< [D2] 25G BASE-R FEC requested */
#define GLB_CSTM_25GBASE_KR1                 (1 << 11)  /**< [D2] Consortium mode 25GBase-KR1 ability */
#define GLB_CSTM_25GBASE_CR1                 (1 << 12)  /**< [D2] Consortium mode 25GBase-CR1 ability */
#define GLB_CSTM_50GBASE_KR2                 (1 << 13)  /**< [D2] Consortium mode 50GBase-KR2 ability */
#define GLB_CSTM_50GBASE_CR2                 (1 << 14)  /**< [D2] Consortium mode 50GBase-CR2 ability */
#define GLB_CSTM_RS_FEC_ABILITY              (1 << 15)  /**< [D2] Consortium mode RS-FEC ability */
#define GLB_CSTM_BASER_FEC_ABILITY           (1 << 16)  /**< [D2] Consortium mode BASE-R FEC ability */
#define GLB_CSTM_RS_FEC_REQUESTED            (1 << 17)  /**< [D2] Consortium mode RS-FEC requested */
#define GLB_CSTM_BASER_FEC_REQUESTED         (1 << 18)  /**< [D2] Consortium mode BASE-R FEC requested */
#else /* for GG on CNOS */
#define GLB_CL73_1000BASE_KX     (1 << 0 ),  /**< [GG] 1000Base-KX ability */
#define GLB_CL73_10GBASE_KX4     (1 << 1 ),  /**< [GG] 10GBase-KX4 ability */
#define GLB_CL73_10GBASE_KR      (1 << 2 ),  /**< [GG] 10GBase-KR ability */
#define GLB_CL73_40GBASE_KR4     (1 << 3 ),  /**< [GG] 40GBase-KR4 ability */
#define GLB_CL73_40GBASE_CR4     (1 << 4 ),  /**< [GG] 40GBase-CR4 ability */
#define GLB_CL73_100GBASE_CR10   (1 << 5 ),  /**< [GG] 100GBase-CR10 ability */
#define GLB_CL73_100GBASE_KP4    (1 << 6 ),  /**< [GG] 100GBase-KP4 ability */
#define GLB_CL73_100GBASE_KR4    (1 << 7 ),  /**< [GG] 100GBase-KR4 ability */
#define GLB_CL73_100GBASE_CR4    (1 << 8 ),  /**< [GG] 100GBase-CR4 ability */
#define GLB_CL73_FEC_ABILITY     (1 << 9 ),  /**< [GG] local device support FEC */
#define GLB_CL73_FEC_REQUESTED   (1 << 10),  /**< [GG] request remote device enable FEC */
#endif
#define GLB_MIN_FRAME_SIZE          64 /*added by ychen in 20130806 for bug 23987*/
#define GLB_VLANIF_MAX_NUM          256
#define GLB_LINKAGG_CHIPID          0x1F /*this should be private*/
#define GLB_LOCAL_PORT_LENGTH       8    /*this should be private*/
#define GLB_LOCAL_PORT_MASK         0xFF

#define GLB_TRANS_PORTID_TO_GPORT(chipid, lportid)  ((((chipid) & 0x1F) << GLB_LOCAL_PORT_LENGTH) | (lportid))
/* Modifed by weizj for bug 36460 */
#define GLB_GCHIP_LENGTH            7           /**< Gchip id length(unit: bit) */
#define GLB_GCHIP_MASK              0x7F        /**< Gchip id mask(unit: bit) */
#define GLB_GSLICE_MASK             7           /**< Gchip id mask(unit: bit) */

#define GLB_GET_GLOBAL_CHIPID(gportid) (((gportid) >> GLB_LOCAL_PORT_LENGTH) & GLB_GCHIP_MASK)
#define GLB_GET_LOCAL_PORTID(gport)    (((gport >> (GLB_LOCAL_PORT_LENGTH + GLB_GCHIP_LENGTH)) \
                                           << GLB_LOCAL_PORT_LENGTH) | ((gport) & 0xFF))
#define GLB_IS_EXTEND_PORT(lport)      (((lport) > 63 && (lport) < 256) || ((lport) > 319 && (lport) < 512))
#define GLB_TRANS_LOCAL_PORTID_TO_GPORT(chipid, lportid) (((((lportid) >> GLB_LOCAL_PORT_LENGTH) & GLB_GSLICE_MASK) \
                                                               << (GLB_LOCAL_PORT_LENGTH + GLB_GCHIP_LENGTH))  \
                                                               | ((chipid & GLB_GCHIP_MASK) << GLB_LOCAL_PORT_LENGTH) \
                                                               | (lportid & GLB_LOCAL_PORT_MASK))
/*modified by weij for 15044, 2011-05-31*/
#define GLB_IS_LINKAGG_PORT(gport)   ((((gport) >> GLB_LOCAL_PORT_LENGTH)& GLB_LINKAGG_CHIPID) == GLB_LINKAGG_CHIPID)
#define GLB_TRANS_TID_TO_AGGPORT(tid)   ((tid) | (GLB_LINKAGG_CHIPID << 8))
#define GLB_TRANS_AGGPORT_TO_TID(gport) ((gport) & 0xFF)

/*refter to netlink_check_unsupport_link*/
#define GLB_KERNEL_PORT_ETH "eth"
#define GLB_KERNEL_PORT_LO "lo"
#define GLB_KERNEL_PORT_LOO "loo"
#define GLB_KERNEL_PORT_GRE0 "gre0"
#define GLB_KERNEL_PORT_IP6TNL0 "ip6tnl0"
#define GLB_KERNEL_PORT_SIT0 "sit0"
#define GLB_KERNEL_PORT_TUNL0 "tunl0"
#define GLB_KERNEL_PORT_PIMREG "pimreg"
#define GLB_KERNEL_PORT_PIM6REG "pim6reg"
#define GLB_KERNEL_PORT_DUMMY "dummy"
#define GLB_KERNEL_PORT_STACKING "stacking"

/* IF_SPLITTING */
#define GLB_DEF_PHY_PORT_PREFIX         GLB_IFNAME_ETH_PREFIX
#define GLB_DEF_PHY_PORT_FULL_PREFIX    GLB_IFNAME_ETH_PREFIX
#define GLB_PHY_PORT_PREFIX_SZ          5
#define GLB_PHY_PORT_NAME_SZ            (GLB_PHY_PORT_PREFIX_SZ + 8)
#define GLB_PHY_PORT_FULL_PREFIX_SZ     25
#define GLB_PHY_PORT_FULL_NAME_SZ       (GLB_PHY_PORT_FULL_PREFIX_SZ + 8)
#define GLB_PHT_PORT_TYPE_MAX           8
#define GLB_PHT_PORT_NUM_MAX            65533

#define GLB_DFT_GG1_MAX_FRAME_SIZE_0 9600
#define GLB_DFT_GG1_MAX_FRAME_SIZE_1 9600
#ifndef TAPPRODUCT
#define GLB_DFT_MAX_FRAME_SIZE_0    1632 
#define GLB_DFT_MAX_FRAME_SIZE_1    9600
#else
/* modify for TAP BUG48079 default 12800 to maximum 16127 */
#define GLB_DFT_MAX_FRAME_SIZE_0    16127
#define GLB_DFT_MAX_FRAME_SIZE_1    16127
#endif

#define GLB_MIB_PORT_STATS_READ_NUM_PER_ROUND       20
#define GLB_MIB_FLOW_STATS_READ_NUM_PER_ROUND       40

/* added by hansf for flow-statistics include ipg */
#define GLB_DEF_IPG_LEN       20    /* include ipg(12) and preamble(8) */
#define GLB_DEF_IFG_STATS_EN      1

/* This name should be consistent with cdb_const IFNAME_SIZE. */
#define GLB_IFNAME_SZ           32

/*added by hansf for loopback num*/
#define GLB_LOOPBACK_MAX    4

#define GLB_IFF_UP              0x1        /* interface is up*/
#define GLB_IFF_BROADCAST       0x2        /* broadcast address valid*/
#define GLB_IFF_NONPRESENT      0x4        /* turn on debugging*/
#define GLB_IFF_LOOPBACK        0x8        /* is a loopback net*/
#define GLB_IFF_POINTOPOINT     0x10       /* interface is has p-p link*/
#define GLB_IFF_NOTRAILERS      0x20       /* avoid use of trailers*/
#define GLB_IFF_RUNNING         0x40       /* resources allocated*/
#define GLB_IFF_NOARP           0x80       /* no ARP protocol*/
#define GLB_IFF_PROMISC         0x100      /* receive all packets*/
#define GLB_IFF_ALLMULTI        0x200      /* receive all multicast packets*/
#define GLB_IFF_CFG_SHUTDOWN    0x400      /* physical port shutdown in startup-config */


#define GLB_OPENFLOW_TUNNEL_IFF_SRCIP              0x1
#define GLB_OPENFLOW_TUNNEL_IFF_DSTIP              0x2
#define GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT              0x4
#define GLB_OPENFLOW_TUNNEL_IFF_BIND_MAC              0x8
#define GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN              0x10

#define GLB_OPENFLOW_TUNNEL_TYPE_LEN    20

#define GLB_OPENFLOW_TUNNEL_LOCAL_IP_CNT_MAX 4

#define GLB_OPENFLOW_L2EDIT_MAX    1000

typedef enum _glb_port_oper_status_t
{
    /** Unknown */
    GLB_PORT_OPER_STATUS_UNKNOWN,

    /** Up */
    GLB_PORT_OPER_STATUS_UP,

    /** Down */
    GLB_PORT_OPER_STATUS_DOWN,

    /** Test Running */
    GLB_PORT_OPER_STATUS_TESTING,

    /** Not Present */
    GLB_PORT_OPER_STATUS_NOT_PRESENT

} glb_port_oper_status_t;

/* modified by wangjj for change default mtu from 1514(defined by SAIi) to 1500, 2017-03-30 */
#define GLB_ETHER_IF_DEFAULT_MTU    1500
#define GLB_AGG_IF_DEFAULT_MTU      1500
#define GLB_VLAN_IF_DEFAULT_MTU     1500
#define GLB_LOOPBACK_IF_DEFAULT_MTU 1500
#define GLB_TUNNEL_IF_DEFAULT_MTU   1500
#define GLB_SUB_IF_DEFAULT_MTU      1500
#define GLB_CPU_IF_DEFAULT_MTU      1500

/* Flow control. */
#define GLB_IF_FLOW_CONTROL_OFF         0
#define GLB_IF_FLOW_CONTROL_SEND        (1 << 0)
#define GLB_IF_FLOW_CONTROL_RECEIVE     (1 << 1)
#define GLB_IF_FLOW_OPER_CONTROL_SEND        (1 << 2)
#define GLB_IF_FLOW_OPER_CONTROL_RECEIVE     (1 << 3)

/* Dot1x */
#define GLB_IF_DOT1X_UNAUTHED       (1 << 0)
#define GLB_IF_DOT1X_DIR_IN         (1 << 1)
#define GLB_IF_DOT1X_DIR_OUT        (1 << 2)
#define GLB_IF_DOT1X_MODE_MAC       (1 << 3)

/* ip nat inside/outside. added by taocy for NAT, 2019/06/01*/
#define GLB_IF_NAT_DISABLE           0
#define GLB_IF_NAT_INSIDE           (1 << 0)
#define GLB_IF_NAT_OUTSIDE          (1 << 1)

#define GLB_IF_NAT_CATEGORY_ID_INVALID  0

typedef uint16_t glb_agg_mem_ports_t[GLB_LINKAGG_MAX_MEMBER];

typedef enum glb_agg_mode_e
{
    GLB_AGG_MODE_STATIC = 0,
    GLB_AGG_MODE_DYNAMIC_ACTIVE,
    GLB_AGG_MODE_DYNAMIC_PASSIVE,
    GLB_ROUND_ROBIN_LOAD_BALANCE,
    GLB_AGG_MODE_MAX
} glb_agg_mode_t;

typedef enum glb_if_mode_e
{
    GLB_IF_MODE_L3 = 0,
    GLB_IF_MODE_L2,
    GLB_IF_MODE_OTHER,
    GLB_IF_MODE_MAX
} glb_if_mode_t;

typedef enum glb_if_egress_type_e
{
    GLB_IF_EGRESS_UNTAGGED = 0,
    GLB_IF_EGRESS_TAGGED   = 1
} glb_if_egress_type_t;

enum glb_if_direction_e
{
    GLB_IF_DIR_RX,
    GLB_IF_DIR_TX
};
typedef enum glb_if_direction_e  glb_if_direction_t;

enum glb_if_route_mac_type_e
{
    GLB_IF_ROUTE_MAC_TYPE0 ,        /**<  type 0, for VRRP IPv4*/
    GLB_IF_ROUTE_MAC_TYPE1 ,        /**<  type 1, for VRRP IPv6*/
    GLB_IF_ROUTE_MAC_TYPE_RSV_RMAC, /**<  type 2, reserved for system routermac*/
    GLB_IF_ROUTE_MAC_TYPE_INVALID ,  /**< non-support type*/  
};
typedef enum glb_if_route_mac_type_e  glb_if_route_mac_type_t;

/* Port type. */
enum glb_vlan_port_type_e
{
    GLB_VLAN_PORT_TYPE_ACCESS,
    GLB_VLAN_PORT_TYPE_TRUNK,
    GLB_VLAN_PORT_TYPE_QINQ,
    GLB_VLAN_PORT_TYPE_MAX
};
typedef enum glb_vlan_port_type_e glb_vlan_port_type_t;

enum glb_vlan_qinq_type_e
{
    GLB_VLAN_QINQ_TYPE_BASIC,
    GLB_VLAN_QINQ_TYPE_SELECTIVE,
    GLB_VLAN_QINQ_TYPE_MAX
};
typedef enum glb_vlan_qinq_type_e glb_vlan_qinq_type_t;

/*define interface type*/
enum glb_if_type_e 
{
    GLB_IF_TYPE_NULL,
    GLB_IF_TYPE_PORT_IF,
    GLB_IF_TYPE_LINKAGG_IF,
    GLB_IF_TYPE_VLAN_IF, 
    GLB_IF_TYPE_SUB_IF,
    GLB_IF_TYPE_LOOPBACK_IF, 
    GLB_IF_TYPE_TUNNEL_IF,
    GLB_IF_TYPE_L2GRE_IF,
    GLB_IF_TYPE_L3GRE_IF,
    GLB_IF_TYPE_NVGRE_IF,
    GLB_IF_TYPE_VXLAN_IF,
    GLB_IF_TYPE_CPU_IF,
    GLB_IF_TYPE_MAX
};
typedef enum glb_if_type_e glb_if_type_t;

/*define interface agg type*/
enum glb_if_agg_type_e 
{
    GLB_IF_AGG_NONE,
    GLB_IF_AGG_MEMBER,
    GLB_IF_AGG_GROUP,
    GLB_IF_AGG_MAX
};
typedef enum glb_if_agg_type_e glb_if_agg_type_t;

enum glb_agg_type_e 
{
    GLB_AGG_FLAG_DISABLE_DISTRIBUTOR,
    GLB_AGG_FLAG_DISABLE_COLLECTOR,
    GLB_AGG_FLAG_ENABLE_DLB,
    GLB_AGG_FLAG_DISABLE_INVALID
};
typedef enum glb_agg_type_e glb_agg_type_t;

enum glb_if_ip_type_e
{
    GLB_IF_L3_IPV4_UC_EN = (1 << 0),
    GLB_IF_L3_IPV6_UC_EN = (1 << 1),
    GLB_IF_L3_IPV4_MC_EN = (1 << 2),
    GLB_IF_L3_IPV6_MC_EN = (1 << 3),
    GLB_IF_L3_IPV4_UCRPF_EN = (1 << 4),
    GLB_IF_L3_IPV6_UCRPF_EN = (1 << 5),
    GLB_IF_L3_IPV4_MCRPF_EN = (1 << 6),    
    GLB_IF_L3_IPV6_MCRPF_EN = (1 << 7),
    GLB_IF_L3_RT_ALL_PKTS   = (1 << 8),
    GLB_IF_L3_DHCP_RELAY_EN = (1 << 9),
    GLB_IF_L3_TYPE_MAX
};
typedef enum glb_if_ip_type_e glb_if_ip_type_t;

/* Port type. */
enum glb_agg_load_balance_e
{
    GLB_AGG_LOAD_BALANCE_SRC_MAC,
    GLB_AGG_LOAD_BALANCE_DST_MAC,
    GLB_AGG_LOAD_BALANCE_SRC_IP,
    GLB_AGG_LOAD_BALANCE_DST_IP,
    GLB_AGG_LOAD_BALANCE_IP_PROTOCOL,
    GLB_AGG_LOAD_BALANCE_SRC_PORT,
    GLB_AGG_LOAD_BALANCE_DST_PORT,
    GLB_AGG_LOAD_BALANCE_VXLAN_VNI,
    GLB_AGG_LOAD_BALANCE_INNER_SRC_MAC,
    GLB_AGG_LOAD_BALANCE_INNER_DST_MAC,
    GLB_AGG_LOAD_BALANCE_INNER_SRC_IP,
    GLB_AGG_LOAD_BALANCE_INNER_DST_IP,
    GLB_AGG_LOAD_BALANCE_INNER_IP_PROTOCOL,
    GLB_AGG_LOAD_BALANCE_INNER_SRC_PORT_L4,
    GLB_AGG_LOAD_BALANCE_INNER_DST_PORT_L4,
    GLB_AGG_LOAD_BALANCE_GRE_KEY,
    GLB_AGG_LOAD_BALANCE_NVGRE_VNI,
    GLB_AGG_LOAD_BALANCE_NVGRE_FLOW_ID,
    GLB_AGG_LOAD_BALANCE_MAX
};
typedef enum glb_agg_load_balance_e glb_agg_load_balance_t;

enum glb_port_isolate_e
{
    GLB_PORT_ISOLATE_L2,
    GLB_PORT_ISOLATE_ALL,
    GLB_PORT_ISOLATE_MAX
};
typedef enum glb_port_isolate_e glb_port_isolate_t;


enum glb_if_pvlan_type_e
{
    GLB_IF_PVLAN_TYPE_NONE = 0,
    GLB_IF_PVLAN_TYPE_PROMISCUOUS,
    GLB_IF_PVLAN_TYPE_ISOLATED,
    GLB_IF_PVLAN_TYPE_COMMUNITY,
    GLB_IF_PVLAN_TYPE_HOST,
    GLB_IF_PVLAN_TYPE_PVLAN_INIT,
    GLB_IF_PVLAN_TYPE_MAX
};
typedef enum glb_if_pvlan_type_e glb_if_pvlan_type_t;

#define GLB_INTERFACE_MAX_PRIVATE_VLAN_BMP_GROUP (0xFFFF)  /*GB:31,HB:63*/

/*for port*/
#define GLB_IF_SYNC_RUNNING                 (1 << 0)
#define GLB_IF_SYNC_AGGREGATED              (1 << 1)
#define GLB_IF_SYNC_ROUTE_EN                (1 << 2)
#define GLB_IF_SYNC_UNTAG_DEF_VID           (1 << 3)
#define GLB_IF_SYNC_UNTAG_SVLAN             (1 << 4)
#define GLB_IF_SYNC_JUMBO_FRAME_EN          (1 << 5)
#define GLB_IF_SYNC_EFM_LINK_MONITOR_ON     (1 << 7)
#define GLB_IF_SYNC_SELECTIVE_QINQ          (1 << 8)
#define GLB_IF_SYNC_VLAN_TRANSLATION        (1 << 9)

#define GLB_IF_STATS_LOAD_DEF_INTVAL        300         /* interface stats default load intval is 5min */
#define GLB_IF_STATS_LOAD_MIN_INTVAL        30          /* interface stats minimum load intval is 30s */
#define GLB_IF_STATS_STORE_DEF_INTVAL        60         /* interface stats default load intval is 5min */
#define GLB_IF_STATS_STORE_CNT               6

#define GLB_ND_DEFAULT_RA_CUR_HOP_LIMIT       0
#define GLB_ND_DUP_ADDR_DETECT_TRANSMITS    1    /* transmissions */
#define GLB_ND_TRT_ADV_INTERVAL                       600000  /* milliseconds */
#define GLB_ND_ADV_DEFAULT_LIFETIME                 1800           /* seconds */
#define GLB_ND_UNSPECIFIED                                  0
#define GLB_ND_RETRANS_TIMER    1000    /* milliseconds */
#define GLB_ND_BASE_REACHABLE_TIME    30000        /* milliseconds */
#define GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME    2592000   /* seconds (30 days) */
#define GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME    604800     /* seconds (7 days) */
#define GLB_ND_DEFAULT_IPV6_HOP_LIMIT    64

/* for openflow use */
#define GLB_IF_STATS_OF_INTVAL        5         /* openflow poll mac stats */
enum glb_if_stats_req_type_e {
    GLB_IF_STATS_REQ_IMISH,
    GLB_IF_STATS_REQ_RMON,
    GLB_IF_STATS_REQ_SFLOW,
    GLB_IF_STATS_REQ_EFM,
    GLB_IF_STATS_REQ_SNMP,
    GLB_IF_STATS_REQ_OF, /* openflow sync: 5s */
    GLB_IF_STATS_REQ_MAX
};
typedef enum glb_if_stats_req_type_e glb_if_stats_req_type_t;

#define GLB_IF_STATS_SFLOW_RESP_INTERVAL    1

typedef enum
{
    PORT_INFO_CONNECTION = 1,
    PORT_INFO_SUB_CONNECTION,
    PORT_INFO_PREFIX_NUM,
    PORT_INFO_PREFIX,
    PORT_INFO_FULL_PREFIX,
    PORT_INFO_MAX
} port_info_type_e;

typedef struct port_info_s
{
    char connection;
    char sub_connection;
    char prefix_num;
    char prefix[GLB_PHT_PORT_TYPE_MAX][GLB_PHY_PORT_PREFIX_SZ];
    char full_prefix[GLB_PHT_PORT_TYPE_MAX][GLB_PHY_PORT_FULL_PREFIX_SZ];
}port_info_t;

enum arp_inspection_trust_enable_e
{
    ARPINSP_PORT_UNTRUST = 0,
    ARPINSP_PORT_TRUST,
    ARPINSP_PORT_MAX,
};
typedef enum arp_inspection_trust_enable_e arp_inspection_trust_enable_t;

enum arp_numberlimit_enable_e
{
    ARP_PORT_NUMBERLIMIT_DISABLE = 0,
    ARP_PORT_NUMBERLIMIT_ENABLE,
    ARP_PORT_NUMBERLIMIT_MAX
};
typedef enum arp_numberlimit_enable_e arp_numberlimit_enable_t;

enum arp_numberlimit_violate_e
{
    ARP_PORT_NUMBERLIMIT_PROTECT = 0,
    ARP_PORT_NUMBERLIMIT_RESTRICT,
    ARP_PORT_NUMBERLIMIT_ERRDISABLE,
    ARP_PORT_NUMBERLIMIT_VIOLATE_MAX
};
typedef enum arp_numberlimit_violate_e arp_numberlimit_violate_t;

enum arp_rate_limit_enable_e
{
    ARP_RATE_LIMIT_DISABLE = 0, 
    ARP_RATE_LIMIT_ENABLE,
    ARP_RATE_LIMIT_STATE_MAX
};
typedef enum arp_rate_limit_enable_e arp_rate_limit_enable_t;

enum arp_rate_limit_exceed_action_e
{
    ARP_RATE_LIMIT_ACTION_RESTRICT = 0,
    ARP_RATE_LIMIT_ACTION_ERRDISABLE,
    ARP_RATE_LIMIT_ACTION_MAX
};
typedef enum arp_rate_limit_exceed_action_e arp_rate_limit_exceed_action_t;

enum arp_rate_limit_abnormal_flag_e
{
    ARP_RATE_LIMIT_ABNORMAL_FALSE = 0, /*NORMAL*/
    ARP_RATE_LIMIT_ABNORMAL_TRUE,/*ABNORMAL*/
    ARP_RATE_LIMIT_ABNORMAL_MAX
};
typedef enum arp_rate_limit_abnormal_flag_e arp_rate_limit_abnormal_flag_t;
    
#define ARP_PORT_NUMBERLIMIT_NUMBER 512
#define ARP_RATE_LIMIT_DEFAULT_NUMBER 150

#define LATENCY_EVENT_LOW_THRESHOLD_DEFAULT  2
#define LATENCY_EVENT_HIGH_THRESHOLD_DEFAULT 6 

#define GLB_VLAN_BMP_WORD_MAX        ((GLB_MAX_VLAN_ID + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_PORT_BMP_WORD_MAX        ((GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_PORT_VLAN_BMP_WORD_MAX   ((2*GLB_MAX_VLAN_ID + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_UINT8_BMP_WORD_MAX       ((GLB_MAX_UINT8_VALUE + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_VCLASS_BMP_WORD_MAX      ((GLB_VLANCLASS_RULE_ID_MAX + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_VRID_BMP_WORD_MAX        ((GLB_VRRP_MAX_VRID + GLB_UINT32_BITS) / GLB_UINT32_BITS)
#define GLB_UINT16_BMP_WORD_MAX       ((GLB_MAX_UINT16_VALUE + GLB_UINT32_BITS) / GLB_UINT32_BITS)

typedef uint32_t vlan_bmp_t[GLB_VLAN_BMP_WORD_MAX];
typedef uint32_t port_bmp_t[GLB_PORT_BMP_WORD_MAX];
typedef uint32_t port_vlan_bmp_t[GLB_PORT_VLAN_BMP_WORD_MAX];
typedef uint32_t uint8_bmp_t[GLB_UINT8_BMP_WORD_MAX];
typedef uint32_t vclass_bmp_t[GLB_VCLASS_BMP_WORD_MAX];
typedef uint32_t vrid_bmp_t[GLB_VRID_BMP_WORD_MAX];

typedef port_bmp_t vlan_memberport_t;



/*added by wanh for bug 52727, 2019-09-16*/
#define MPLS_LABEL_VALUE_MIN				16
#define MPLS_LABEL_VALUE_MAX				1048575
#define MPLS_LABEL_DEFAULT_VALUE			20

#define GLB_MPLS_LABEL_BMP_WORD_MAX        ((MPLS_LABEL_VALUE_MAX + GLB_UINT32_BITS) / GLB_UINT32_BITS)

typedef uint32_t mpls_label_bmp_t[GLB_MPLS_LABEL_BMP_WORD_MAX];
/*wanh end*/

#endif /* !__GLB_IF_DEFINE_H__ */


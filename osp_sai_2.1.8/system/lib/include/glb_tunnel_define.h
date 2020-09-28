#ifndef __GLB_TUNNEL_DEFINE_H__
#define __GLB_TUNNEL_DEFINE_H__

//#include "sal.h"

#define GLB_TUNNEL_GRE_PROTO_ETH    0x6558
#define GLB_TUNNEL_GRE_PROTO_IPV4   0x0800
#define GLB_TUNNEL_GRE_PROTO_IPV6   0x86DD

#define GLB_TUNNEL_GRE_FLAG_CHKSUM    0x1
#define GLB_TUNNEL_GRE_FLAG_KEY       0x2
#define GLB_TUNNEL_GRE_FLAG_SEQUENCE  0x4

#define GLB_TUNNEL_CONFIG_CHECKSUM     (1 << 0)
#define GLB_TUNNEL_CONFIG_PMTUD        (1 << 1)
#define GLB_TUNNEL_CONFIG_TTL_INHERIT  (1 << 2)
#define GLB_TUNNEL_CONFIG_DSCP_INHERIT (1 << 3)
#define GLB_TUNNEL_CONFIG_SRC_IFNAME   (1 << 4) /*only used in nsm*/
#define GLB_TUNNEL_LINE_PROTO_UP       (1 << 5) /*only used in nsm*/
#define GLB_TUNNEL_ENTRY_MAX            64 /*when mtu enable, ipsa uses the bit5:0 in tunnelv4edit.ipsaindex*/

enum glb_tunnel_mode_e
{
    GLB_TUNNEL_MODE_NONE = 0,
    GLB_TUNNEL_MODE_IPIP,
    GLB_TUNNEL_MODE_GREIP,
    GLB_TUNNEL_MODE_IPV6IP_MANUAL,
    GLB_TUNNEL_MODE_IPV6IP_6TO4,
    GLB_TUNNEL_MODE_IPV6IP_ISATAP,
    GLB_TUNNEL_MODE_MAX,
};
typedef enum glb_tunnel_mode_e glb_tunnel_mode_t;

#endif /*__GLB_TUNNEL_DEFINE_H__*/


/* Added by weizj for tunnel */
#ifndef __OFP_TUNNEL_H__
#define __OFP_TUNNEL_H__


#define OFP_TUNNEL_PORT_NO_NUM        900
#define OFP_TUNNEL_IFINDEX_BASE       20000
#define OFP_MAX_TUNNEL_ID             OFP_UINT24_MAX
#define OFP_MIN_TUNNEL_ID             0
 
#define OFP_TUNNEL_L2GRE_NO_BASE      200
#define OFP_TUNNEL_NVGRE_NO_BASE      1300
#define OFP_TUNNEL_VXLAN_NO_BASE      2200

#define OFP_TUNNEL_L2GRE_NO_MAX       (OFP_TUNNEL_L2GRE_NO_BASE + OFP_TUNNEL_PORT_NO_NUM + 1)
#define OFP_TUNNEL_NVGRE_NO_MAX       (OFP_TUNNEL_NVGRE_NO_BASE + OFP_TUNNEL_PORT_NO_NUM + 1)
#define OFP_TUNNEL_VXLAN_NO_MAX       (OFP_TUNNEL_VXLAN_NO_BASE + OFP_TUNNEL_PORT_NO_NUM + 1)


/**
 * @brief tunnel information
 */
struct ofp_tunnel_info_s
{
    uint32_ofp local_ip;                     /**< local ip address */
    uint32_ofp remote_ip;                    /**< remote ip address */
    uint16_ofp vlan_id;                      /**< vlan id */
    uint8_ofp nexthop_mac[OFP_ETH_ADDR_LEN]; /**< nexthop mac address */
    char bind_port_name[OFP_IFNAME_SIZE];    /**< bind physical port name, eg eth-0-1 */
    bool is_dynamic;
};
typedef struct ofp_tunnel_info_s ofp_tunnel_info_t;

#endif /* !__OFP_TUNNEL_H__ */

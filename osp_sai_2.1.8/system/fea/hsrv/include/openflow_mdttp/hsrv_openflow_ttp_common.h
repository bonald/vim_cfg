#ifndef __HSRV_OPENFLOW_TTP_COMMON_H__
#define __HSRV_OPENFLOW_TTP_COMMON_H__

#define HSRV_UINT8_MAX       0xff
#define HSRV_UINT16_MAX      0xffff
#define HSRV_UINT24_MAX      0xffffff
#define HSRV_UINT32_MAX      0xffffffff

#define TTP_GLB_COPP_GROUP  1027   /* sai acl init */
#define TTP_GLB_ACL_GROUP    1022
#define TTP_GLB_SCL_GROUP    2000

#define TTP_DROP_NEXTHOP    1

#define TTP_TUNNEL_PORT_TYPE    0x00010000
#define TTP_PHYIF_PORT_TYPE      0x00000000

#define HSRV_INVALID_STATS_ID   0

#define IS_TUNNEL_PORT(port)    (port) & TTP_TUNNEL_PORT_TYPE

#define TABLE_ID_TO_STR(table_id) \
    ( \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_PORT == (table_id) ? "ingress port flow": \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN == (table_id) ? "vlan flow": \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC == (table_id) ? "termination mac flow": \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING == (table_id) ? "unicast routing flow": \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING == (table_id) ? "bridging flow": \
    GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL == (table_id) ? "ingress flow": \
    "unkown")

#define GROUP_ID_TO_STR(group_id) \
    ( \
    GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE == (GLB_OPENFLOW_TTP_GROUP_TYPE(group_id)) ? "l2-interface": \
    GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST == (GLB_OPENFLOW_TTP_GROUP_TYPE(group_id)) ? "l3-unicast": \
    GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD == (GLB_OPENFLOW_TTP_GROUP_TYPE(group_id)) ? "l2-flood": \
    GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP == (GLB_OPENFLOW_TTP_GROUP_TYPE(group_id)) ? "l3-ecmp": \
    "unkown")

enum hsrv_openflow_ttp_stats_type_e
{
    HSRV_OPENFLOW_TTP_STATS_TYPE_SCL,
    HSRV_OPENFLOW_TTP_STATS_TYPE_ACL,
    HSRV_OPENFLOW_TTP_STATS_TYPE_NEXTHOP,
    HSRV_OPENFLOW_TTP_STATS_TYPE_ECMP,
    HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST,
    HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL   /* ip tunnel nexthop, or tunnel decap scl */
};
typedef enum hsrv_openflow_ttp_stats_type_e hsrv_openflow_ttp_stats_type_t;

struct hsrv_openflow_ttp_stats_s
{
    uint64 packet_count;
    uint64 byte_count;
};
typedef struct hsrv_openflow_ttp_stats_s hsrv_openflow_ttp_stats_t;

struct hsrv_openflow_ttp_meter_stats_s
{
    hsrv_openflow_ttp_stats_t   confirm_stats;
    hsrv_openflow_ttp_stats_t   exceed_stats;
    hsrv_openflow_ttp_stats_t   violate_stats;
};
typedef struct hsrv_openflow_ttp_meter_stats_s hsrv_openflow_ttp_meter_stats_t;

#endif 

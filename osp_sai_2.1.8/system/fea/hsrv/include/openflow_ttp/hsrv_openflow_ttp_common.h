#ifndef __HSRV_OPENFLOW_TTP_COMMON_H__
#define __HSRV_OPENFLOW_TTP_COMMON_H__

#define HSRV_UINT8_MAX       0xff
#define HSRV_UINT16_MAX      0xffff
#define HSRV_UINT24_MAX      0xffffff
#define HSRV_UINT32_MAX      0xffffffff

#define TTP_GLB_ACL_GROUP    1022
#define TTP_GLB_SCL_GROUP    2000

#define TTP_DROP_NEXTHOP    1

enum hsrv_openflow_ttp_stats_type_e
{
    HSRV_OPENFLOW_TTP_STATS_TYPE_SCL,
    HSRV_OPENFLOW_TTP_STATS_TYPE_ACL,
    HSRV_OPENFLOW_TTP_STATS_TYPE_NEXTHOP,
    HSRV_OPENFLOW_TTP_STATS_TYPE_ECMP,
    HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST,
    HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL
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

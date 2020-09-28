#ifndef __ERPS_DEFINE_H__
#define __ERPS_DEFINE_H__

typedef enum 
{
    ERPS_IDLE = 0,
    ERPS_COMPLETE,
    ERPS_FAILED,
    ERPS_LINK_UP,
    ERPS_LINK_DOWN,
    ERPS_PREFORWORDING,
    ERPS_INIT,
    ERPS_STATE_MAX
} erps_state_t;

typedef enum
{
    ERPS_HEALTH_PKT = 5,
    ERPS_RING_UP_FLUSH_FDB_PKT,
    ERPS_RING_DOWN_FLUSH_FDB_PKT,
    ERPS_LINK_DOWN_PKT,
    ERPS_EDGE_HELLO_PKT,
    ERPS_MAJOR_FAULT_PKT,
    ERPS_MAX_PKT
} erps_msg_type_t;

typedef enum
{
    ERPS_NODE_UNKNOWN = 0,
    ERPS_NODE_MASTER,
    ERPS_NODE_TRANSIT,
    ERPS_NODE_ROLE_MAX
} erps_node_role_t;

typedef enum
{
    ERPS_NODE_EDGE_UNKNOWN = 0,
    ERPS_NODE_EDGE,
    ERPS_NODE_ASSI_EDGE,
    ERPS_EDGE_NODE_ROLE_MAX
} erps_edge_node_role_t;

typedef enum
{
    ERPS_RING_PRIMARY = 0, 
    ERPS_RING_SUB,
    ERPS_RING_LEVEL_MAX
} erps_ring_level_t;

typedef enum
{
    ERPS_NODE_CHECK_MASTER = 0, 
    ERPS_NODE_CHECK_EDGE,
    ERPS_NODE_CHECK_MAX
} erps_node_check_t;

typedef struct erps_message_s
{
    uint8   dst_mac[6];
    uint8   src_mac[6];
    uint16  eth_type;
    uint16  pri_vlan_id;
    uint16  frame_len;
    uint16  dsap;
    uint8   control;
    uint8   oui[3];
    uint32  rub1;
    uint16  erps_len;
    uint8   erps_ver;
    uint8   erps_type;
    uint16  domain_id;
    uint16  ring_id;
    uint8   rub2[2];
    uint8   sys_mac[6];
    uint16  h_timer;
    uint16  f_timer;
    uint8   state;
    uint8   level;
    uint16  h_seq;
    uint16  rub4;
    uint8   res[36];
} erps_message_t;

#define ERPS_PRI_MAX 10

#define ERPS_SECS_TO_TICKS(x)  (x*60)
#define ERPS_TICKS_TO_SECS(y)  (y/60)

int32
erps_start(void);

int32
erps_stop(void);
#endif /* !__ERPS_DEFINE_H__ */

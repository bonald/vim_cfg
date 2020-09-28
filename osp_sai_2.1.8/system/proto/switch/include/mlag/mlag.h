
#ifndef __MLAG_H__
#define __MLAG_H__

#define MLAG_DBG_FN_DESC_STR                ""

#define MLAG_LOG_ERR(FMT, ...) \
do \
{ \
    log_sys(M_MOD_INTERNAL, E_ERROR, "%s%s" FMT, MLAG_DBG_FN_DESC_STR, ": " , ##__VA_ARGS__); \
} while (0)

#if 1
#define MLAG_DEBUG_INFO(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(MLAG, mlag, typeenum, fmt, ##args);\
}while(0)
#else
#define MLAG_DEBUG_INFO(mod, sub, typeenum, fmt, args...) \
do { \
    log_sys(M_MOD_MLAG, E_ERROR, fmt, ##args); \
} while (0)
#endif

#define MLAG_DEBUG_FUNC(typeenum) \
    MLAG_DEBUG_INFO(typeenum, "Enter into %s.", __FUNCTION__)

#define MLAG_DEBUG(typeenum, fmt, args...) \
do {\
    MLAG_DEBUG_INFO(typeenum, fmt, ##args);\
}while(0)

/* Utility Macros */
#define MLAG_UNREFERENCED_PARAMETER(PARAM) ((PARAM) = (PARAM))

/* MLAG version */
#define MLAG_VERSION_1                           (1)

/* MLAG Default Port number */
#define MLAG_PORT_DEFAULT                        (61000)

/* MLAG Listen Socket Listen Backlog Count */
#define MLAG_SOCK_LISTEN_BACKLOG                 (5)

#define MLAG_FDB_SYNC_TIMER                     (1)   
#define MLAG_FDB_SYNC_MAX_NUM                   GLB_MLAG_SYNC_COUNT_MAX

#define MLAG_STP_SYNC_TIMER                     (2)/*add by huangxt for mlag-stp. 2014.09.29*/

/* MLAG message header and packet size.  */
#define MLAG_HEADER_SIZE                         (4)
#define MLAG_MAX_PACKET_SIZE                     (4096)

#define MLAG_MSG_FDBSYNC_HDR_SIZE                (MLAG_HEADER_SIZE + 1)
#define MLAG_MSG_FDBSYNC_BLK_SIZE                (12)

/* MLAG minimum message size */
#define MLAG_MSG_OPEN_MIN_SIZE                   (MLAG_HEADER_SIZE + 6 + 2)
#define MLAG_MSG_KEEPALIVE_MIN_SIZE              (MLAG_HEADER_SIZE + 0)
#define MLAG_MSG_FDBSYNC_MIN_SIZE                (MLAG_MSG_FDBSYNC_HDR_SIZE + MLAG_MSG_FDBSYNC_BLK_SIZE)
#define MLAG_MSG_FAILOVER_MIN_SIZE               (MLAG_HEADER_SIZE + 4)
#define MLAG_MSG_CONF_MIN_SIZE                   (MLAG_HEADER_SIZE + 4)
/*add by huangxt for mlag-stp. 2014.09.29*/
#define MLAG_MSG_STP_MIN_SIZE                    (MLAG_HEADER_SIZE + 1) /*sub type 1*/
#define MLAG_MSG_SYSPRI_MIN_SIZE                 (MLAG_HEADER_SIZE + 2) 
#define MLAG_MSG_PEER_FDB_MIN_SIZE               (MLAG_HEADER_SIZE + 9)
#define MLAG_MSG_DISCONNECT_MIN_SIZE             (MLAG_HEADER_SIZE + 0)

/* MLAG Maximum message sizes */
#define MLAG_MSG_OPEN_MAX_SIZE                   (MLAG_MSG_OPEN_MIN_SIZE)
#define MLAG_MSG_KEEPALIVE_MAX_SIZE              (MLAG_MSG_KEEPALIVE_MIN_SIZE)
#define MLAG_MSG_FDBSYNC_MAX_SIZE                (MLAG_MSG_FDBSYNC_HDR_SIZE + MLAG_MSG_FDBSYNC_BLK_SIZE*MLAG_FDB_SYNC_MAX_NUM)
#define MLAG_MSG_FAILOVER_MAX_SIZE               (MLAG_MSG_FAILOVER_MIN_SIZE)
#define MLAG_MSG_CONF_MAX_SIZE                   (MLAG_MSG_CONF_MIN_SIZE)
#define MLAG_MSG_SYSPRI_MAX_SIZE                 (MLAG_MSG_SYSPRI_MIN_SIZE)
#define MLAG_MSG_PEER_FDB_MAX_SIZE               (MLAG_MSG_PEER_FDB_MIN_SIZE + 256*2)
#define MLAG_MSG_DISCONNECT_MAX_SIZE             (MLAG_MSG_DISCONNECT_MIN_SIZE)

/*add by huangxt for mlag-stp. 2014.09.29*/
#define MLAG_MSG_STP_PKT_BUFF_SIZE               256
#define MLAG_MSG_STP_MAX_SIZE                     1500
#define MLAG_MSG_STP_GLOBAL_SIZE                 (MLAG_MSG_STP_MIN_SIZE + 10) /*enable 1 + type 1 + prio 2 + mac 6 */
#define MLAG_MSG_STP_PORT_SIZE                   (MLAG_MSG_STP_MIN_SIZE + 3) /*enable 1 + mlag id 2*/
#define MLAG_MSG_STP_BPDU_PKT_HEADER_SIZE        (MLAG_MSG_STP_MIN_SIZE + 30)/*mlagid 2 + sockaddr_vlan 24 + lenth 4*/
#define MLAG_MSG_STP_STATE_SIZE                  (MLAG_MSG_STP_MIN_SIZE + 5) /*mlagid 2 + instatnce 2 + state 1*/
#define MLAG_MSG_STP_INSTANCE_SIZE               (MLAG_MSG_STP_MIN_SIZE + 2) /*instatnce 2*/

#define MLAG_MSG_FDBSYNC_TYPE_DYNAMIC            (0)
#define MLAG_MSG_FDBSYNC_TYPE_STATIC             (1)

#define MLAG_MSG_CONF_ACT_ADD                    (0)
#define MLAG_MSG_CONF_ACT_DEL                    (1)

#define MLAG_MSG_RESERVED                       (0)

/* MLAG uptime string length.  */
#define MLAG_UPTIME_LEN                         (25)

/*MLAG id invalid value*/
#define MLAG_ID_INVALID                         (0)

typedef enum
{
    MLAG_SOCK_CONNECTED_NONE    = 0,
    MLAG_SOCK_CONNECTED_SERVER  = 1,
    MLAG_SOCK_CONNECTED_CLIENT  = 2,
    MLAG_SOCK_CONNECTED_BOTH    = 3
} mlag_sock_connected_t;

/*
 * MLAG Peer FSM State Definitions
 */
typedef enum
{
    MPF_STATE_INVALID       = 0,
    MPF_STATE_IDLE          = 1,
    MPF_STATE_ACTIVE        = 2,
    MPF_STATE_OPEN_SENT     = 3,
    MPF_STATE_OPEN_CFM      = 4,
    MPF_STATE_ESTABLISHED   = 5,
    MPF_STATE_MAX           = 6
} mlag_state_t;

/*
 * MLAG Peer FSM Event Definitions
 */
typedef enum
{
    MPF_EVENT_INVALID               = 0,
    /* MLAG Peer FSM Administrative Events */
    MPF_EVENT_MANUAL_START          = 1,
    MPF_EVENT_MANUAL_STOP           = 2,
    /* MLAG Peer FSM Timer based Events */
    MPF_EVENT_HOLD_EXP              = 3,
    MPF_EVENT_KEEPALIVE_EXP         = 4,
    /* MLAG Peer FSM TCP Connection based Events */
    MPF_EVENT_TCP_CONN_CFM          = 5,
    MPF_EVENT_TCP_CONN_FAIL         = 6,
    /* MLAG Peer FSM Message based Events */
    MPF_EVENT_OPEN_VALID            = 7,
    MPF_EVENT_KEEPALIVE_VALID       = 8,
    MPF_EVENT_SOCK_TX_FAIL          = 9,
    MPF_EVENT_RX_DISCONNECT         = 10,
    MPF_EVENT_MAX                   = 11
} mlag_event_t;


/* MLAG Peer Finite State Machine Action Function Type */
typedef int32 (*mpf_act_func_t)(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);

/* MLAG timers default value.  */
#define MLAG_DEFAULT_AUTO_START                  (5)
#define MLAG_DEFAULT_CONNECT_RETRY               (20)
#define MLAG_DEFAULT_HOLDTIME                    (240)
#define MLAG_DEFAULT_KEEPALIVE                   (60)
#define MLAG_DEFAULT_RELOAD_DELAY                (300)
#define MLAG_MIN_RELOAD_DELAY                    (0)
#define MLAG_MAX_RELOAD_DELAY                    (86400)
#define MLAG_RELOAD_DELAY_STR                    "<0-86400>"

/* MLAG Message Types */
#define MLAG_MSG_OPEN                            (1)
#define MLAG_MSG_KEEPALIVE                       (2)
#define MLAG_MSG_FDBSYNC                         (3)
#define MLAG_MSG_FAILOVER                        (4)
#define MLAG_MSG_CONF                            (5)
//#define MLAG_MSG_NOTIFY                        (6)
/*add by huangxt for mlag-stp. 2014.09.29*/
#define MLAG_MSG_STP                             (7)
#define MLAG_MSG_SYSPRI                          (8)
#define MLAG_MSG_PEER_FDB                        (9)
#define MLAG_MSG_DISCONNECT                      (10)

#define MLAG_MSG_STP_GLOBAL                      (1)
#define MLAG_MSG_STP_PORT                        (2)
#define MLAG_MSG_STP_INST                        (3)
#define MLAG_MSG_STP_PACKET                      (4)
#define MLAG_MSG_STP_STATE                       (5)

#define MLAG_MSG_PEER_FDB_OP_ADD                 (1)
#define MLAG_MSG_PEER_FDB_OP_DEL                 (2)
#define MLAG_MSG_PEER_FDB_OP_SYNC                (3)

#define MLAG_SYNC_FDB_ACT_STR(ACT)         \
  ((ACT) == GLB_MLAG_FDB_SYNC_ADD ?   "ADD" :  \
   (ACT) == GLB_MLAG_FDB_SYNC_DEL ?   "DEL" :  \
   (ACT) == GLB_MLAG_FDB_SYNC_FLUSH_ALL ?     "FLUSH_ALL" :    \
   (ACT) == GLB_MLAG_FDB_SYNC_FLUSH_PORT ?     "FLUSH_PORT" :    \
   (ACT) == GLB_MLAG_FDB_SYNC_FLUSH_VLAN ?     "FLUSH_VLAN" :    \
   (ACT) == GLB_MLAG_FDB_SYNC_FLUSH_MAC ?     "FLUSH_MAC" :    \
                                       "N/A")
typedef struct
{
    uint8 version;
    uint8 type;     /* MLAG_MSG_xxx */
    uint32 length;
    uint8 *data;
    uint8 *pnt;
    int32  size_left;
    int32 *p_size_left;
} mlag_decode_t;

typedef struct
{
    /* Destination Mac address */
    uint8 dest_mac[MAC_ADDR_LEN];

    /* Source Mac address */
    uint8 src_mac[MAC_ADDR_LEN];

    /* Outgoing/Incoming interface index */
    uint32 port;

    /* Vlan id */
    uint16 vlanid;

    /* SVlan id */
    uint16 svlanid;

    /* pkt type */
    uint16 type;

    /* reserve */
    uint16 reserve;
} l2_socket_addr_t;

int32
mlag_start(switch_master_t *master);

int32
mlag_stop(switch_master_t *master);

#endif /* !__MLAG_H__ */


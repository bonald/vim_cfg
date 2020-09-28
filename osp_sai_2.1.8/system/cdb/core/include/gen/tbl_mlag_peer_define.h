
#ifndef __TBL_MLAG_PEER_DEFINE_H__
#define __TBL_MLAG_PEER_DEFINE_H__

/* TBL_MLAG_PEER field defines */
typedef enum
{
    TBL_MLAG_PEER_FLD_PEER_ADDR            = 0 ,  /* RW */
    TBL_MLAG_PEER_FLD_SET_TIMERS           = 1 ,  /* RW */
    TBL_MLAG_PEER_FLD_HOLDTIME             = 2 ,  /* RW */
    TBL_MLAG_PEER_FLD_KEEPALIVE            = 3 ,  /* RW */
    TBL_MLAG_PEER_FLD_SERVER_SOCK          = 4 ,  /* READ */
    TBL_MLAG_PEER_FLD_CLIENT_SOCK          = 5 ,  /* READ */
    TBL_MLAG_PEER_FLD_V_AUTO_START         = 6 ,  /* READ */
    TBL_MLAG_PEER_FLD_V_CONNECT            = 7 ,  /* READ */
    TBL_MLAG_PEER_FLD_V_HOLDTIME           = 8 ,  /* READ */
    TBL_MLAG_PEER_FLD_V_KEEPALIVE          = 9 ,  /* READ */
    TBL_MLAG_PEER_FLD_OPEN_IN              = 10,  /* READ */
    TBL_MLAG_PEER_FLD_OPEN_OUT             = 11,  /* READ */
    TBL_MLAG_PEER_FLD_KEEPALIVE_IN         = 12,  /* READ */
    TBL_MLAG_PEER_FLD_KEEPALIVE_OUT        = 13,  /* READ */
    TBL_MLAG_PEER_FLD_DISCONNECT_IN        = 14,  /* READ */
    TBL_MLAG_PEER_FLD_DISCONNECT_OUT       = 15,  /* READ */
    TBL_MLAG_PEER_FLD_FDBSYNC_IN           = 16,  /* READ */
    TBL_MLAG_PEER_FLD_FDBSYNC_OUT          = 17,  /* READ */
    TBL_MLAG_PEER_FLD_FAILOVER_IN          = 18,  /* READ */
    TBL_MLAG_PEER_FLD_FAILOVER_OUT         = 19,  /* READ */
    TBL_MLAG_PEER_FLD_CONF_IN              = 20,  /* READ */
    TBL_MLAG_PEER_FLD_CONF_OUT             = 21,  /* READ */
    TBL_MLAG_PEER_FLD_SYSPRI_IN            = 22,  /* READ */
    TBL_MLAG_PEER_FLD_SYSPRI_OUT           = 23,  /* READ */
    TBL_MLAG_PEER_FLD_PEER_FDB_IN          = 24,  /* READ */
    TBL_MLAG_PEER_FLD_PEER_FDB_OUT         = 25,  /* READ */
    TBL_MLAG_PEER_FLD_STP_IN               = 26,  /* READ */
    TBL_MLAG_PEER_FLD_STP_OUT              = 27,  /* READ */
    TBL_MLAG_PEER_FLD_STP_GLOBAL_IN        = 28,  /* READ */
    TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT       = 29,  /* READ */
    TBL_MLAG_PEER_FLD_STP_PORT_IN          = 30,  /* READ */
    TBL_MLAG_PEER_FLD_STP_PORT_OUT         = 31,  /* READ */
    TBL_MLAG_PEER_FLD_STP_PACKET_IN        = 32,  /* READ */
    TBL_MLAG_PEER_FLD_STP_PACKET_OUT       = 33,  /* READ */
    TBL_MLAG_PEER_FLD_STP_INSTANCE_IN      = 34,  /* READ */
    TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT     = 35,  /* READ */
    TBL_MLAG_PEER_FLD_STP_STATE_IN         = 36,  /* READ */
    TBL_MLAG_PEER_FLD_STP_STATE_OUT        = 37,  /* READ */
    TBL_MLAG_PEER_FLD_ESTABLISHED          = 38,  /* READ */
    TBL_MLAG_PEER_FLD_DROPPED              = 39,  /* READ */
    TBL_MLAG_PEER_FLD_TTL                  = 40,  /* READ */
    TBL_MLAG_PEER_FLD_UPTIME               = 41,  /* READ */
    TBL_MLAG_PEER_FLD_LASTREAD             = 42,  /* READ */
    TBL_MLAG_PEER_FLD_MPF_STATE            = 43,  /* READ */
    TBL_MLAG_PEER_FLD_T_AUTO_START         = 44,  /* READ */
    TBL_MLAG_PEER_FLD_T_HOLDTIME           = 45,  /* READ */
    TBL_MLAG_PEER_FLD_T_KEEPALIVE          = 46,  /* READ */
    TBL_MLAG_PEER_FLD_MAX                  = 47
} tbl_mlag_peer_field_id_t;

/* TBL_MLAG_PEER defines */
typedef struct
{
    addr_t               peer_addr;
    uint32               set_timers;
    uint32               holdtime;
    uint32               keepalive;
    int32                server_sock;         /* server socket ID */
    int32                client_sock;         /* client socket ID */
    uint32               v_auto_start;        /* Peer FSM Timer value auto_start */
    uint32               v_connect;           /* Peer FSM Timer value connect */
    uint32               v_holdtime;          /* Peer FSM Timer value holdtime */
    uint32               v_keepalive;         /* Peer FSM Timer value keepalive */
    uint32               open_in;             /* Open message input count */
    uint32               open_out;            /* Open message output count */
    uint32               keepalive_in;        /* Keepalive input count */
    uint32               keepalive_out;       /* Keepalive output count */
    uint32               disconnect_in;       /* Disconnect input count */
    uint32               disconnect_out;      /* Disconnect output count */
    uint32               fdbsync_in;          /* Fdbsync input count */
    uint32               fdbsync_out;         /* Fdbsync output count */
    uint32               failover_in;         /* Failover input count */
    uint32               failover_out;        /* Failover output count */
    uint32               conf_in;             /* Conf input count */
    uint32               conf_out;            /* Conf output count */
    uint32               syspri_in;           /* Syspri input count */
    uint32               syspri_out;          /* Syspri output count */
    uint32               peer_fdb_in;
    uint32               peer_fdb_out;
    uint32               stp_in;
    uint32               stp_out;
    uint32               stp_global_in;
    uint32               stp_global_out;
    uint32               stp_port_in;
    uint32               stp_port_out;
    uint32               stp_packet_in;
    uint32               stp_packet_out;
    uint32               stp_instance_in;
    uint32               stp_instance_out;
    uint32               stp_state_in;
    uint32               stp_state_out;
    uint32               established;
    uint32               dropped;
    uint8                ttl;
    sal_time_t           uptime;
    sal_time_t           lastread;
    uint32               mpf_state;           /* mlag_state_t */
    ctc_task_t           *t_auto_start;
    ctc_task_t           *t_holdtime;
    ctc_task_t           *t_keepalive;
} tbl_mlag_peer_t;

#endif /* !__TBL_MLAG_PEER_DEFINE_H__ */


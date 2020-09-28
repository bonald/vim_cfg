
#ifndef __TBL_VRRP_SESSION_DEFINE_H__
#define __TBL_VRRP_SESSION_DEFINE_H__

/* TBL_VRRP_SESSION field defines */
typedef enum
{
    TBL_VRRP_SESSION_FLD_KEY                  = 0 ,  /* READ */
    TBL_VRRP_SESSION_FLD_ENABLE               = 1 ,  /* RW */
    TBL_VRRP_SESSION_FLD_STATE                = 2 ,  /* RW */
    TBL_VRRP_SESSION_FLD_VIP                  = 3 ,  /* RW */
    TBL_VRRP_SESSION_FLD_OWNER                = 4 ,  /* RW */
    TBL_VRRP_SESSION_FLD_IFNAME               = 5 ,  /* RW */
    TBL_VRRP_SESSION_FLD_IFINDEX              = 6 ,  /* RW */
    TBL_VRRP_SESSION_FLD_PRIORITY             = 7 ,  /* RW */
    TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY      = 8 ,  /* RW */
    TBL_VRRP_SESSION_FLD_ADVT_INTERVAL        = 9 ,  /* RW */
    TBL_VRRP_SESSION_FLD_TIME_MODE            = 10,  /* RW */
    TBL_VRRP_SESSION_FLD_PREEMPT_DELAY        = 11,  /* RW */
    TBL_VRRP_SESSION_FLD_PREEMPT_MODE         = 12,  /* RW */
    TBL_VRRP_SESSION_FLD_VMAC                 = 13,  /* RW */
    TBL_VRRP_SESSION_FLD_IP_PROTOCOL          = 14,  /* RW */
    TBL_VRRP_SESSION_FLD_TIMER                = 15,  /* RW */
    TBL_VRRP_SESSION_FLD_OLD_MAC              = 16,  /* RW */
    TBL_VRRP_SESSION_FLD_VIP_STATUS           = 17,  /* RW */
    TBL_VRRP_SESSION_FLD_IFP_STATUS           = 18,  /* RW */
    TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG        = 19,  /* RW */
    TBL_VRRP_SESSION_FLD_SKEW_TIME            = 20,  /* RW */
    TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL = 21,  /* RW */
    TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS         = 22,  /* RW */
    TBL_VRRP_SESSION_FLD_TRACKOBJ_ID          = 23,  /* RW */
    TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS      = 24,  /* RW */
    TBL_VRRP_SESSION_FLD_PRIORITY_DELTA       = 25,  /* RW */
    TBL_VRRP_SESSION_FLD_VRRP_UPTIME          = 26,  /* RW */
    TBL_VRRP_SESSION_FLD_ADMIN_STATE          = 27,  /* RW */
    TBL_VRRP_SESSION_FLD_ROWSTATUS            = 28,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER  = 29,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD      = 30,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS = 31,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES  = 32,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS  = 33,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD = 34,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT = 35,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD = 36,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS = 37,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE = 38,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH = 39,  /* RW */
    TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS = 40,  /* RW */
    TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS     = 41,  /* RW */
    TBL_VRRP_SESSION_FLD_MASTER_IP            = 42,  /* RW */
    TBL_VRRP_SESSION_FLD_MASTER_PRIORITY      = 43,  /* RW */
    TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL = 44,  /* RW */
    TBL_VRRP_SESSION_FLD_PRIMARY_IP           = 45,  /* RW */
    TBL_VRRP_SESSION_FLD_LEARN_MASTER         = 46,  /* RW */
    TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP = 47,  /* RW */
    TBL_VRRP_SESSION_FLD_MAX                  = 48
} tbl_vrrp_session_field_id_t;

/* TBL_VRRP_SESSION defines */
typedef struct
{
    uint32               vrid;
} tbl_vrrp_session_key_t;

typedef struct
{
    tbl_vrrp_session_key_t key;
    uint8                enable;              /* enable */
    uint8                state;               /* Current State. */
    addr_t               vip;                 /* Virtual IP address. */
    uint8                owner;               /* IP owner */
    char                 ifname[IFNAME_SIZE]; /* vrrp interface name */
    uint32               ifindex;             /* vrrp interface ifindex */
    uint8                priority;            /* running priority */
    uint8                config_priority;     /* configured priority */
    uint16               advt_interval;       /* Advertisement interval */
    uint8                time_mode;           /* time unit is second or mili-second */
    uint16               preempt_delay;       /* preempt delay */
    uint8                preempt_mode;        /* preempt mode */
    mac_addr_t           vmac;                /* virual mac */
    uint8                ip_protocol;         /* ip or ipv6 */
    uint32               timer;               /* master down timer or advt timer */
    mac_addr_t           old_mac;             /* original interface mac. */
    uint8                vip_status;          /* vip is set */
    uint8                ifp_status;          /* vrrp interface is set */
    uint8                shutdown_flag;       /* vrrp interface is shutdown */
    uint8                skew_time;           /* Skew time. */
    uint32               master_down_interval; /* master down interval. */
    uint8                num_ip_addrs;        /* number of vip address */
    uint16               trackobj_id;         /* track object id */
    uint8                trackobj_status;     /* track object status */
    uint8                priority_delta;      /* track priority decrement */
    sal_time_t           vrrp_uptime;         /* vrrp update time for snmp */
    uint8                admin_state;         /* vrrp admin state */
    uint8                rowstatus;           /* row status */
    uint32               stats_become_master; /* Times state transitioned to master */
    uint32               stats_advt_rcvd;     /* Advt rcvd */
    uint32               stats_advt_interval_errors; /* Advt rcvd with wrong interval */
    uint32               stats_auth_failures; /* pkts rcvd failed auth check */
    uint32               stats_ip_ttl_errors; /* pkts rcvd with ttl != 255 */
    uint32               stats_priority_zero_pkts_rcvd; /* pkts rcvd with priority=0 */
    uint32               stats_priority_zero_pkts_sent; /* pkts sent with priority=0 */
    uint32               stats_invalid_type_pkts_rcvd; /* pkts rcvd with invalid type */
    uint32               stats_addr_list_errors; /* pkts rcvd with addr mismatch */
    uint32               stats_invalid_auth_type; /* pkts rcvd with unknown auth type */
    uint32               stats_auth_type_mismatch; /* pkts rcvd with auth type mismatch */
    uint32               stats_pkt_len_errors; /* pkts rcvd with pkt length < length of VRRP header */
    uint8                master_ip_status;    /* master ip status */
    addr_t               master_ip;           /* master ip address */
    uint8                master_priority;     /* master priority */
    uint16               master_advt_interval; /* master advertisement interval */
    addr_t               primary_ip;          /* vrrp interface primary ip */
    uint8                learn_master;        /* learn master advt for backup */
    uint8                secondaryip_same_vip; /* vrrp interface secondary ip is same as vip */
} tbl_vrrp_session_t;

typedef struct
{
    tbl_vrrp_session_t   *session_array[GLB_VRRP_MAX_VRID+1];
} tbl_vrrp_session_master_t;

#endif /* !__TBL_VRRP_SESSION_DEFINE_H__ */


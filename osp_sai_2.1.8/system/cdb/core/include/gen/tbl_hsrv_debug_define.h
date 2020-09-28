
#ifndef __TBL_HSRV_DEBUG_DEFINE_H__
#define __TBL_HSRV_DEBUG_DEFINE_H__

/* TBL_HSRV_DEBUG field defines */
typedef enum
{
    TBL_HSRV_DEBUG_FLD_VSWITCH              = 0 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF       = 1 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF       = 2 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB       = 3 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV      = 4 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM      = 5 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_RX               = 6 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_TX               = 7 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_BPDU             = 8 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO        = 9 ,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_EAPOL            = 10,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_LLDP             = 11,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_ERPS             = 12,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_MACDA            = 13,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_RIP              = 14,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_OSPF             = 15,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_BGP              = 16,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_ARP              = 17,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_DHCP             = 18,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_IPDA             = 19,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_IGMP             = 20,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT         = 21,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH      = 22,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU        = 23,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_OTHER            = 24,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_RAW              = 25,  /* RW */
    TBL_HSRV_DEBUG_FLD_CPU_PTP              = 26,  /* RW */
    TBL_HSRV_DEBUG_FLD_IP_ICMP              = 27,  /* RW */
    TBL_HSRV_DEBUG_FLD_NEXTHOP              = 28,  /* RW */
    TBL_HSRV_DEBUG_FLD_VLAN                 = 29,  /* RW */
    TBL_HSRV_DEBUG_FLD_MIRROR               = 30,  /* RW */
    TBL_HSRV_DEBUG_FLD_FDB                  = 31,  /* RW */
    TBL_HSRV_DEBUG_FLD_L2MC                 = 32,  /* RW */
    TBL_HSRV_DEBUG_FLD_AGG                  = 33,  /* RW */
    TBL_HSRV_DEBUG_FLD_IPUC                 = 34,  /* RW */
    TBL_HSRV_DEBUG_FLD_NEIGHBOR             = 35,  /* RW */
    TBL_HSRV_DEBUG_FLD_ECMP                 = 36,  /* RW */
    TBL_HSRV_DEBUG_FLD_ACL                  = 37,  /* RW */
    TBL_HSRV_DEBUG_FLD_STATS                = 38,  /* RW */
    TBL_HSRV_DEBUG_FLD_QOS_CLASS            = 39,  /* RW */
    TBL_HSRV_DEBUG_FLD_QOS_POLICER          = 40,  /* RW */
    TBL_HSRV_DEBUG_FLD_QOS_PROCESS          = 41,  /* RW */
    TBL_HSRV_DEBUG_FLD_QOS_QUEUE            = 42,  /* RW */
    TBL_HSRV_DEBUG_FLD_IPSOUR               = 43,  /* RW */
    TBL_HSRV_DEBUG_FLD_DOT1X                = 44,  /* RW */
    TBL_HSRV_DEBUG_FLD_STP                  = 45,  /* RW */
    TBL_HSRV_DEBUG_FLD_PTP_PKT_RX           = 46,  /* RW */
    TBL_HSRV_DEBUG_FLD_PTP_PKT_TX           = 47,  /* RW */
    TBL_HSRV_DEBUG_FLD_PTP_CFG              = 48,  /* RW */
    TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT        = 49,  /* RW */
    TBL_HSRV_DEBUG_FLD_IPMC                 = 50,  /* RW */
    TBL_HSRV_DEBUG_FLD_TPOAM_COMMON         = 51,  /* RW */
    TBL_HSRV_DEBUG_FLD_TPOAM_EVENT          = 52,  /* RW */
    TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE         = 53,  /* RW */
    TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX         = 54,  /* RW */
    TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX         = 55,  /* RW */
    TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX    = 56,  /* RW */
    TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX    = 57,  /* RW */
    TBL_HSRV_DEBUG_FLD_NAT_RULE             = 58,  /* RW */
    TBL_HSRV_DEBUG_FLD_NAT_SESSION          = 59,  /* RW */
    TBL_HSRV_DEBUG_FLD_NAT_TIMER            = 60,  /* RW */
    TBL_HSRV_DEBUG_FLD_G8032_EVENT          = 61,  /* RW */
    TBL_HSRV_DEBUG_FLD_MAX                  = 62
} tbl_hsrv_debug_field_id_t;

/* TBL_HSRV_DEBUG defines */
typedef struct
{
    uint32               vswitch;             /* bitmap of HSRVDBG_FLAG_VSWITCH_ */
    uint32               interface;           /* bitmap of HSRVDBG_FLAG_INTERFACE_ */
    uint32               message;             /* bitmap of HSRVDBG_FLAG_MESSAGE_ */
    uint32               cpu;                 /* bitmap of HSRVDBG_FLAG_CPU_ */
    uint32               ip;                  /* bitmap of HSRVDBG_FLAG_IP_ */
    uint32               nexthop;             /* bitmap of HSRVDBG_FLAG_NEXTHOP_ */
    uint32               vlan;                /* bitmap of HSRVDBG_FLAG_VLAN_ */
    uint32               mirror;              /* bitmap of HSRVDBG_FLAG_MIRROR_ */
    uint32               fdb;                 /* bitmap of HSRVDBG_FLAG_FDB_ */
    uint32               l2mc;                /* bitmap of HSRVDBG_FLAG_L2MC_ */
    uint32               agg;                 /* bitmap of HSRVDBG_FLAG_AGG_ */
    uint32               ipuc;                /* bitmap of HSRVDBG_FLAG_IPUC_ */
    uint32               neighbor;            /* bitmap of HSRVDBG_FLAG_NEIGHBOR_ */
    uint32               ecmp;                /* bitmap of HSRVDBG_FLAG_ECMP_ */
    uint32               acl;                 /* bitmap of HSRVDBG_FLAG_ACL_ */
    uint32               stats;               /* bitmap of HSRVDBG_FLAG_STATS_ */
    uint32               qos;                 /* bitmap of HSRVDBG_FLAG_QOS_ */
    uint32               ipsour;              /* bitmap of HSRVDBG_FLAG_QOS_ */
    uint32               dot1x;               /* bitmap of HSRVDBG_FLAG_DOT1X_ */
    uint32               stp;                 /* bitmap of HSRVDBG_FLAG_STP_ */
    uint32               ptp;                 /* bitmap of HSRVDBG_FLAG_PTP_ */
    uint32               ipmc;                /* bitmap of HSRVDBG_FLAG_IPMC_ */
    uint32               tpoam;               /* bitmap of HSRVDBG_FLAG_TPOAM_ */
    uint32               l2protocol;          /* bitmap of HSRVDBG_FLAG_L2PROTOCOL_ */
    uint32               nat;                 /* bitmap of HSRVDBG_FLAG_NAT_ */
    uint32               g8032;               /* bitmap of HSRVDBG_FLAG_NAT_ */
} tbl_hsrv_debug_t;

#endif /* !__TBL_HSRV_DEBUG_DEFINE_H__ */


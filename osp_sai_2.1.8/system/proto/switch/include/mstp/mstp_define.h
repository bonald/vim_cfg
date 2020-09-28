#ifndef __MSTP_DEFINE_H__
#define __MSTP_DEFINE_H__

#include "mstp_config.h"

#define MSTP_PATH_COST_MIN              1
#define MSTP_PATH_COST_1T_MAX           200000000
#define MSTP_PATH_COST_1D1998_MAX       65535

#define MSTP_BRIDGE_PRIORITY_GRANULARITY    4096
#define MSTP_PORT_PRIORITY_GRANULARITY      16

/* Timer intervals */
#define MSTP_PORT_TIMER_INTERVAL        1

#define MSTP_BPDU_LOOP_THRESHOLD        3

/* BPDU defines */
#define LLC_DEFAULT_ADDR        0x42
#define LLC_DEFAULT_CTRL        0x03
#define LLC_LEN                 0x03

#define BPDU_VERSION_STP        0x0
#define BPDU_VERSION_RSTP       0x2
#define BPDU_VERSION_MSTP       0x3
#define BPDU_VERSION_DEFAULT    0x4

#define MSTP_ONE_SEC_UNIT       256

#define BPDU_STP_LEN            35
#define BPDU_RSTP_LEN           36

#define BPDU_PKT_LEN            10240

#define BPDU_STP_TCN_SIZE       7
#define BPDU_STP_CONFIG_SIZE    38
#define BPDU_RSTP_SIZE          39
#define BPDU_VERSION_1_LEN      0
#define BPDU_MSTI_MAX           GLB_STP_INSTANCE_MAX 
#define BPDU_MSTI_LEN           16

#define MSTP_BRIDGE_ID_LEN      8

#define MSTP_GET_PORT_FROM_PORTID(_PID_)    ((_PID_) & 0x0FFF)

typedef enum
{
    MSTP_LINK_SHARED = 0, 
    MSTP_LINK_P2P,
    MSTP_LINK_AUTO,
    MSTP_LINK_MAX
} mstp_link_t;

typedef enum
{
    MSTP_INFO_DISABLED = 0,
    MSTP_INFO_MINE,
    MSTP_INFO_AGED,
    MSTP_INFO_RECEIVED,
    MSTP_INFO_MAX
} mstp_info_t;

/* 17.21.8 rcvInfo() */
typedef enum
{
    MSTP_SUPERIOR_DESIGNATED = 0,
    MSTP_REPEATED_DESIGNATED,
    MSTP_INFERIOR_DESIGNATED,
    MSTP_INFERIOR_ROOTALTERNATE,
    MSTP_OTHER,
    MSTP_RCVD_INFO_MAX
} mstp_rcvd_info_t;

typedef enum
{
    BPDU_TYPE_CONFIG =  0x00, 
    BPDU_TYPE_RST =     0x02,
    BPDU_TYPE_MST =     0x03, 
    BPDU_TYPE_TCN =     0x80
} bpdu_type_t;

typedef enum
{
    ROLE_DISABLED = 0,
    ROLE_ALTERNATE,
    ROLE_ROOTPORT,
    ROLE_DESIGNATED,
    ROLE_BACKUP,
    ROLE_MASTERPORT,
    MSTP_PORT_ROLE_MAX
} mstp_port_role_t;

/* 9.2.9 Encoding of Port Role values */
typedef enum
{
    BPDU_ROLE_UNKNOWN           = 0,
    BPDU_ROLE_ALTERNATE_BACKUP  = 1,
    BPDU_ROLE_ROOT              = 2,
    BPDU_ROLE_DESIGNATED        = 3,
    BPDU_ROLE_MAX
} mstp_bpdu_role_t;

typedef enum
{
    STATE_DISCARDING = 0,
    STATE_LISTENING,
    STATE_LEARNING,
    STATE_FORWARDING,
    STATE_BLOCKING,
    STATE_FIXED_FORWARDING,
    MSTP_PORT_STATE_MAX
} mstp_port_state_t;

typedef enum
{
    TC_INACTIVE,
    TC_ACTIVE,
    MSTP_TC_STATE_MAX
} mstp_tc_state_t;

typedef enum
{
    MSTP_TYPE_STP = 0,
    MSTP_TYPE_RSTP,
    MSTP_TYPE_MSTP,
    MSTP_TYPE_MAX
} mstp_type_t;

typedef enum
{
    MSTP_EDGEPORT_DISABLED = 0,
    MSTP_EDGEPORT_ENABLED,
    MSTP_EDGEPORT_DEFAULT,
    MSTP_EDGEPORT_MAX
} mstp_portfast_t;

/* PTI State (Port TImer Machine)
   802.1D.2004  P.170  17.22 Port Timers state machine
   802.1Q.2005  P.179  13.27 The Port Timers state machine
*/
typedef enum
{
    PTI_TICK = 0,
    PTI_ONE_SECOND,
    PTI_STATE_MAX
} pti_state_t;

/* PTI Event */
typedef enum
{
    PTI_EVT_BEGIN = 0,  /* BEGIN */
    PTI_EVT_TICKTRUE,   /* tick == TRUE */
    PTI_EVENT_MAX
} pti_event_t;

/* PRX State (Port Receive Machine)
   802.1D.2004  P.170  17.23 Port Receive state machine
   802.1Q.2005  P.179  13.28 Port Receive state machine
*/
typedef enum
{
    PRX_DISCARD = 0,
    PRX_RECEIVE,
    PRX_STATE_MAX
} prx_state_t;
    
/* PRX Event */
typedef enum
{
    PRX_EVT_BEGIN = 0,                          /* BEGIN*/
    PRX_EVT_RCVDBPDU_PORT_ENABLED,              /* rcvdBpdu && portEnabled */
    PRX_EVT_RCVDBPDU_PORT_ENABLED_NOT_RCVDMSG,  /* rcvdBpdu && portEnabled && !rcvdMsg */
    PRX_EVENT_MAX
} prx_event_t;

/* PPM State (Port Protocol Migration)
   802.1D.2004  P.171  17.24 Port Protocol Migration state machine
   802.1Q.2005  P.180  13.29 Port Protocol Migration state machine
*/
typedef enum
{
    PPM_CHECKING_RSTP = 0,
    PPM_SELECTING_STP,
    PPM_SENSING,
    PPM_STATE_MAX
} ppm_state_t;

/* PPM Event */
typedef enum
{         
    PPM_EVT_BEGIN = 0,                              /* BEGIN*/
    PPM_EVT_MCHECK,                                 /* mcheck */
    PPM_EVT_SENDRSTP_RCVDSTP,                       /*  sendRSTP && rcvdSTP */
    PPM_EVT_RSTPVERSION_NOTSENDRSTP_RCVDRSTP,       /* ((rstpVersion) && !sendRSTP && rcvdRSTP)) */
    PPM_EVENT_MAX        
} ppm_event_t;

/* BDM State (Bridge Detection Machine)
   802.1D.2004  P.171  17.25 Bridge Detection state machine
   802.1Q.2005  P.180  13.30 Bridge Detection state machine
*/
typedef enum
{
    BDM_NOT_EDGE = 0,
    BDM_EDGE,
    BDM_STATE_MAX
} bdm_state_t;

/* BDM Event */
typedef enum
{        
    BDM_EVT_BEGIN_ADMINEDGE = 0,                            /* BEGIN && AdminEdge  */
    BDM_EVT_BEGIN_NOTADMINEDGE,                             /* BEGIN && !AdminEdge */
    BDM_EVT_EDGEDELAY_TIMEOUT_EDGE,                         /* adminEdge && edgeDelayWhile=0 */
    BDM_EVT_RECV_BPDU,                                      /* receiveBPDU */
    BDM_EVENT_MAX
} bdm_event_t;

/* PTX State (Port Transmit)
   802.1D.2004  P.172  17.26 Port Transmit state machine
   802.1Q.2005  P.180  13.31 Port Transmit state machine
*/
typedef enum
{
    PTX_TRANSMIT_INIT = 0,
    PTX_TRANSMIT_PERIODIC,
    PTX_IDLE,
    PTX_TRANSMIT_RSTP,
    PTX_TRANSMIT_TCN,
    PTX_TRANSMIT_CONFIG,
    PTX_STATE_MAX
} ptx_state_t;

/* PTX Event */
typedef enum
{         
    PTX_EVT_BEGIN = 0,                  /* BEGIN */
    PTX_EVT_HELLOWHENZERO,              /* helloWhen == 0 */
    PTX_EVT_SENDRSTP,                   /* sendRSTP && newInfo && (txCount < TxHoldCount) && (helloWhen !=0) */
    PTX_EVT_SENDTCN,                    /* !sendRSTP && newInfo && RootPort && (txCount < TxHoldCount) && (helloWhen != 0) */
    PTX_EVT_SENDCONFIG,                 /* !sendRSTP && newInfo && DesignatedPort && (txCount < TxHoldCount) && (helloWhen != 0) */
    PTX_EVENT_MAX
} ptx_event_t;

/* PIM State (Port Information Machine)
   802.1D.2004  P.173  17.27 Port Information state machine
   802.1Q.2005  P.182  13.32 Port Information state machine
*/
typedef enum
{
    PIM_DISABLED = 0,
    PIM_AGED,
    PIM_UPDATE,
    PIM_CURRENT,
    PIM_OTHER,
    PIM_NOT_DESIGNATED,
    PIM_INFERIOR_DESIGNATED,
    PIM_REPEATED_DESIGNATED,
    PIM_SUPERIOR_DESIGNATED,
    PIM_RECEIVE,
    PIM_STATE_MAX
} pim_state_t;

/* PIM Event */
typedef enum
{
    PIM_EVT_BEGIN = 0,              /* BEGIN */
    PIM_EVT_RCVDMSG,                /* rcvdMsg */
    PIM_EVT_SELECTEDUPDTINFO,       /* (selected && updtInfo)*/
    PIM_EVT_RECVINFOWHILEZERO,      /* (infoIs == Received) && (rcvdInfoWhile == 0) && !updtInfo && !rcvdMsg*/
    PIM_EVT_PORTENABLED,            /* portEnabled*/
    PIM_EVT_RCVDMSGNOTUPDTINFO,     /* rcvdMsg && !updtInfo */
    PIM_EVENT_MAX
} pim_event_t;

/* PRS State (Port Role Selection Machine)
   802.1D.2004  P.174  17.28 Port Role Selection state machine
   802.1Q.2005  P.183  13.33 Port Role Selection state machine
*/
typedef enum
{
    PRS_INIT_BRIDGE = 0,
    PRS_ROLE_SELECTION,
    PRS_STATE_MAX
} prs_state_t;

/* PRS Event */
typedef enum
{
    PRS_EVT_BEGIN = 0,                      /* BEGIN */
    PRS_EVT_RESELECT,                       /* reselect1 || reselect2 || ... reselectN */
    PRS_EVENT_MAX
} prs_event_t;

/* PRT State (Port Role Transitions state machine)
   802.1D.2004  P.174  17.29 Port Role Transitions state machine
   802.1Q.2005  P.183  13.34 Port Role Transitions state machine
*/
typedef enum
{
    /* 802.1D.2004  P.175  Figure 17-20 Disabled Port role transitions 
       802.1Q.2005  P.184  Figure 13-15 Disabled Port role transitions 
     */
    PRT_INIT_PORT = 0,
#if 0
    /* We omit disable port state */
    PRT_DISABLE_PORT,
#endif
    PRT_DISABLED_PORT,
    /* 802.1Q.2005  P.184  Figure 13-16 Port Role Transitions state machine: MasterPort
     */
    PRT_MASTER_PROPOSED,
    PRT_MASTER_AGREED,
    PRT_MASTER_SYNCED,
    PRT_MASTER_RETIRED,
    PRT_MASTER_FORWARD,
    PRT_MASTER_LEARN,
    PRT_MASTER_DISCARD,
    PRT_MASTER_PORT,
    /* 802.1D.2004  P.176  Figure 17-21 Root Port role transitions
       802.1Q.2005  P.185  Figure 13-17 Port Role Transitions state machine: RootPort
     */
    PRT_ROOT_PROPOSED,
    PRT_ROOT_AGREED,
    PRT_REROOT,
    PRT_ROOT_PORT,
    PRT_REROOTED,
    PRT_ROOT_LEARN,
    PRT_ROOT_FORWARD,
    /* 802.1D.2004  P.176  Figure 17-22 Designated port role transitions
       802.1Q.2005  P.186  Figure 13-18 Port Role Transitions state machine: DesignatedPort
     */
    PRT_DESIGNATED_PROPOSE,
    PRT_DESIGNATED_SYNCED,
    PRT_DESIGNATED_RETIRED,
    PRT_DESIGNATED_PORT,
    PRT_DESIGNATED_DISCARD,
    PRT_DESIGNATED_LEARN,
    PRT_DESIGNATED_FORWARD,
    /* 802.1D.2004  P.177  Figure 17-23 Alternate and Backup Port role transitions
       802.1Q.2005  P.186  Figure 13-19 Port Role Transitions state machine: AlternatePort and BackupPort
     */
    PRT_ALTERNATE_PROPOSED,
    PRT_ALTERNATE_AGREED,
    PRT_ALTERNATE_PORT,
    PRT_BACKUP_PORT,
    PRT_BLOCK_PORT,
    PRT_STATE_MAX
} prt_state_t;

/* Events for Port Role Transition Machine */
typedef enum
{
    PRT_EVT_BEGIN = 0,
    
    /* All transtions, except UCT,are qualified by: "&& selected && !updtInfo".*/
    PRT_EVT_ROLE_DISABLED,
    PRT_EVT_ROLE_MASTERPORT,
    PRT_EVT_ROLE_ROOTPORT,
    PRT_EVT_ROLE_DESIGNATED,
    PRT_EVT_ROLE_ALTERNATE,
    PRT_EVT_ROLE_BACKUP,

    PRT_EVT_DESIGNATED_SYNCED,
    PRT_EVT_DESIGNATEDDISCARD,
    PRT_EVT_DESIGNATEDLEARN,
    PRT_EVT_DESIGNATEDFORWARD,
    
    PRT_EVT_MASTER_SYNCED,
    PRT_EVT_MASTERDISCARD,
    PRT_EVT_MASTERLEARN,
    PRT_EVT_MASTERFORWARD,
    
    PRT_EVT_ROOTLEARN,
    PRT_EVT_ROOTFORWARD,
    PRT_EVT_PROPOSING,
    PRT_EVT_PROPOSED_NOTAGREE,
    PRT_EVT_ALLSYNCED_NOTAGREE,

    PRT_EVENT_MAX
} prt_event_t;

/* PST State (Port State Transition)
   802.1D.2004  P.177  17.30 Port State Transition state machine
   802.1Q.2005  P.187  13.35 Port State Transition state machine
*/
typedef enum
{
    PST_DISCARDING = 0,
    PST_LISTENING,      /* only for STP */
    PST_LEARNING,
    PST_FORWARDING,
    PST_STATE_MAX
} pst_state_t;

/* PST Event */
typedef enum
{        
    PST_EVT_BEGIN = 0,              /* BEGIN */
    PST_EVT_LISTEN,                 /* listen */
    PST_EVT_LEARN,                  /* learn */
    PST_EVT_NOTLEARN,               /* !learn */
    PST_EVT_FORWARD,                /* forward */
    PST_EVT_NOTFORWARD,             /* !forward */
    PST_EVENT_MAX
} pst_event_t;

/* TCM State (Topology Change Machine)
   802.1D.2004  P.178  17.31 Topology Change state machine
   802.1Q.2005  P.187  13.36 Topology Change state machine
*/
typedef enum
{
    TCM_INACTIVE = 0,
    TCM_DETECTED,
    TCM_ACTIVE,
    TCM_ACKNOWLEDGED,
    TCM_PROPAGATING,
    TCM_NOTIFIED_TC,
    TCM_NOTIFIED_TCN,
    TCM_STATE_MAX
} tcm_state_t;

/* TCM Event */
typedef enum
{                        
    TCM_EVT_BEGIN =0 ,                                  /* BEGIN */
    TCM_EVT_RCVDTC,                                     /* rcvdTc */
    TCM_EVT_RCVDTCN,                                    /* rcvdTcn */
    TCM_EVT_RCVDTCACK,                                  /* rcvdTcAck */
    TCM_EVT_TCPROP,                                     /* tcProp */
    TCM_EVT_TCPROPNOTOPEREDGE,                          /* tcProp && !operEdge */
    TCM_EVT_FORWARDNOTOPEREDGE,                         /* (role == RootPort || role == DesignatedPort)&& forward && !operEdge */
    TCM_EVT_OPEREDGE,                                   /* operEdge */
    TCM_EVT_ROLENOTROOTDESIGNATEDPORT,                  /* (role != RootPort) && (role != DesignatedPort) */ 
    TCM_EVT_LEARNNOTFDBFLUSH,                           /* learn && !fdbFlush  */
    TCM_EVENT_MAX
} tcm_event_t;

typedef struct
{
    uint8   format_id;      /* format selector */
    uint16  revision_level;
    char    region_name[MSTP_REGION_NAME_LEN + 1]; /* SNMP admin string */
    uint8   digest[MSTP_DIGEST_LEN];
} mstp_cfg_id_t;

typedef struct
{
    uint8       bridge_priority;
    uint8       port_priority;
    uint8       hops;
    uint8       port_role;
    uint8       topology_change:1;
    uint8       proposal:1;
    uint8       empty:2;
    uint8       learning:1;
    uint8       forwarding:1;
    uint8       agreement:1;
    uint8       master:1;
    mstp_brg_id_t    reg_root_id;
    mstp_brg_id_t    bridge_id;
    uint32           internal_rpc;
    uint16      instance_id;
    uint16      port_id;
    void        *cist;  /* bpdu_t */
} msti_bpdu_t;

typedef struct
{
    uint16      proto_id;
    uint8       version;    /* BPDU_VERSION_xxx */
    uint8       type;       /* bpdu_type_t */
    
    uint8       port_role;
    uint8       topology_change:1;
    uint8       proposal:1;
    uint8       empty:2;
    uint8       learning:1;
    uint8       forwarding:1;
    uint8       agreement:1;
    uint8       topology_change_ack:1;

    mstp_brg_id_t   root_id;
    mstp_brg_id_t   reg_root_id;            /* only for MSTP */
    mstp_brg_id_t   bridge_id;
    uint32          external_rpc;
    uint32          internal_rpc;           /* only for MSTP */
    uint16          port_id;
    uint16          messageAge;
    uint16          maxAge;
    uint16          helloTime;
    uint16          fwdDelay;
    uint16          version3_len;
    uint16          cisco_bpdu;
    mstp_cfg_id_t   mstp_cfg;               /* only for MSTP */
    uint8           mstp_hops;              /* only for MSTP */
    uint8           msti_num;               /* only for MSTP */
    msti_bpdu_t     msti[BPDU_MSTI_MAX];    /* only for MSTP */
} bpdu_t;

typedef struct
{
    uint8       macda[MAC_ADDR_LEN];
    uint8       macsa[MAC_ADDR_LEN];
    uint16      len;
    uint8       dsap;
    uint8       ssap;
    uint8       ctrl;
} mstp_llc_hdr_t;

extern const char *mstp_fea_state_strs[];
extern const char *mstp_port_role_strs[];
extern const char *mstp_bpdu_role_strs[];
extern const char *mstp_info_type_strs[];
extern const char *mstp_port_state_strs[];
extern const char *mstp_tc_state_strs[];
extern const char *mstp_type_strs[];
extern const char *mstp_type_lower_strs[];
extern const char *mstp_edgeport_strs[];
extern const char *mstp_path_cost_strs[];
extern const char *mstp_link_strs[];
extern const char *mstp_info_strs[];
extern const char *mstp_rcvd_info_strs[];

extern const char *pti_state_strs[];
extern const char *prx_state_strs[];
extern const char *ppm_state_strs[];
extern const char *bdm_state_strs[];
extern const char *ptx_state_strs[];
extern const char *pim_state_strs[];
extern const char *prs_state_strs[];
extern const char *prt_state_strs[];
extern const char *pst_state_strs[];
extern const char *tcm_state_strs[];

extern const char *pti_event_strs[];
extern const char *prx_event_strs[];
extern const char *ppm_event_strs[];
extern const char *bdm_event_strs[];
extern const char *ptx_event_strs[];
extern const char *pim_event_strs[];
extern const char *prs_event_strs[];
extern const char *prt_event_strs[];
extern const char *pst_event_strs[];
extern const char *tcm_event_strs[];

#if 1
#define MSTP_DEBUG_INFO(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(MSTP, stp, typeenum, fmt, ##args);\
}while(0)
#else
#define MSTP_LOG_DEBUG(mod, sub, typeenum, fmt, args...) \
do { \
    log_sys(M_MOD_MSTP, E_ERROR, fmt, ##args); \
} while (0)
#endif

#define MSTP_DEBUG_FUNC(typeenum) \
    MSTP_DEBUG_INFO(typeenum, "Enter into %s.", __FUNCTION__)

#define MSTP_DEBUG(typeenum, fmt, args...) \
do {\
    MSTP_DEBUG_INFO(typeenum, fmt, ##args);\
}while(0)

#endif /* !__MSTP_DEFINE_H__ */

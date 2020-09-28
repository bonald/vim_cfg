
#ifndef __TBL_MSTP_GLOBAL_DEFINE_H__
#define __TBL_MSTP_GLOBAL_DEFINE_H__

#include "gen/tbl_mstp_port_define.h"
/* TBL_MSTP_GLOBAL field defines */
typedef enum
{
    TBL_MSTP_GLOBAL_FLD_TYPE                 = 0 ,  /* RW */
    TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG        = 1 ,  /* RW */
    TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST = 2 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST = 3 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_PORTID          = 4 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID          = 5 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST    = 6 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID    = 7 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST = 8 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID = 9 ,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID = 10,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID    = 11,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE      = 12,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY        = 13,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME       = 14,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE          = 15,  /* READ */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY      = 16,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME     = 17,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE        = 18,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS       = 19,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY      = 20,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BRIDGE_ID            = 21,  /* READ */
    TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME         = 22,  /* READ */
    TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES     = 23,  /* READ */
    TBL_MSTP_GLOBAL_FLD_MLAG_ROLE            = 24,  /* READ */
    TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE          = 25,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ENABLE               = 26,  /* RW */
    TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED = 27,  /* READ */
    TBL_MSTP_GLOBAL_FLD_BPDUGUARD            = 28,  /* RW */
    TBL_MSTP_GLOBAL_FLD_BPDUFILTER           = 29,  /* RW */
    TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO          = 30,  /* RW */
    TBL_MSTP_GLOBAL_FLD_OPER_CISCO           = 31,  /* READ */
    TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT          = 32,  /* RW */
    TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD    = 33,  /* RW */
    TBL_MSTP_GLOBAL_FLD_TC_PROTECTION        = 34,  /* RW */
    TBL_MSTP_GLOBAL_FLD_HOPCOUNT             = 35,  /* READ */
    TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD   = 36,  /* READ */
    TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER      = 37,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX    = 38,  /* READ */
    TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX = 39,  /* READ */
    TBL_MSTP_GLOBAL_FLD_PRS_STATE            = 40,  /* READ */
    TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID       = 41,  /* READ */
    TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL  = 42,  /* RW */
    TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME     = 43,  /* RW */
    TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST          = 44,  /* READ */
    TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER  = 45,  /* READ */
    TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN  = 46,  /* RW */
    TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP  = 47,  /* RW */
    TBL_MSTP_GLOBAL_FLD_MAX                  = 48
} tbl_mstp_global_field_id_t;

/* TBL_MSTP_GLOBAL defines */
typedef struct
{
    uint8                type;                /* mstp_type_t */
    uint32               no_mst_config;       /* remove all mst configuration */
    uint32               external_root_path_cost;
    uint32               internal_root_path_cost;
    uint32               root_portId;         /* 17.18.5 rootPortId */
    mstp_brg_id_t        root_rootId;         /* 17.18.6/13.23.6 rootPriority - rootId */
    uint32               root_rootPathCost;   /* 17.18.6/13.23.6 rootPriority - rootPathCost */
    mstp_brg_id_t        root_regionRootId;   /* 17.18.6/13.23.6 rootPriority - regionRootId */
    uint32               root_intRootPathCost; /* 17.18.6/13.23.6 rootPriority - internalRootPathCost */
    mstp_brg_id_t        root_designatedBridgeId; /* 17.18.6/13.23.6 rootPriority - designatedBridgeId */
    uint16               root_designatedPortId; /* 17.18.6/13.23.6 rootPriority - designatedPortId */
    uint16               root_bridgePortId;   /* 17.18.6/13.23.6 rootPriority - bridgePortId */
    uint32               root_messageAge;     /* 17.18.7 rootTimes - messageAge */
    uint32               root_fwdDelay;       /* 17.18.7 rootTimes - 17.20.6 fwdDelay */
    uint32               root_helloTime;      /* 17.18.7 rootTimes - 17.20.7 helloTime */
    uint32               root_maxAge;         /* 17.18.7 rootTimes - 17.20.8 maxAge */
    uint32               bridge_fwdDelay;     /* 17.20.6 FwdDelay */
    uint32               bridge_helloTime;    /* 17.20.7 HelloTime */
    uint32               bridge_maxAge;       /* 17.20.8 MaxAge */
    uint32               bridge_maxHops;
    uint32               bridge_priority;     /* 17.18.3 BridgePriority */
    mstp_brg_id_t        bridge_id;           /* 17.18.2 BridgeIdentifier */
    sal_time_t           last_tc_time;
    uint32               num_topo_changes;
    uint32               mlag_role;           /* MLAG module notified mlag_role */
    uint8                mlag_enable;         /* MLAG module notified mlag_enable */
    uint8                enable;              /* enable xSTP global */
    uint8                topology_change_detected;
    uint8                bpduguard;
    uint8                bpdufilter;
    uint8                admin_cisco;
    uint8                oper_cisco;
    uint8                txHoldCount;         /* 17.13.12 Transmit Hold Count */
    uint8                pathcost_standard;   /* mstp_path_cost_t */
    uint8                tc_protection;
    uint8                hopCount;
    uint16               recv_tcn_threshold;
    uint16               recv_tcn_number;
    uint32               root_port_ifindex;   /* ifindex of root port */
    uint32               alternate_port_ifndex; /* ifindex of alternate port */
    uint8                prs_state;           /* prs_state_t - Port Role Selection */
    uint8                mstp_format_id;
    uint16               mstp_revision_level;
    char                 mstp_region_name[MSTP_REGION_NAME_LEN+1];
    mstp_digest_t        mstp_digest;
    tbl_mstp_port_t      *root_port;
    tbl_mstp_port_t      *alternate_port;
    ctc_task_t           *tc_protection_timer;
    uint32               of_exclude_all_vlan; /* openflow exclude all VLANs */
    vlan_bmp_t           of_include_vlan_bmp; /* openflow include VLAN bitmap */
} tbl_mstp_global_t;

#endif /* !__TBL_MSTP_GLOBAL_DEFINE_H__ */


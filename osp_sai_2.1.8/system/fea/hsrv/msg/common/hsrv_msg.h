#ifndef __HSRV_MSG_H__
#define __HSRV_MSG_H__

#define HAL_NAME_LEN            33
#define FEI_NAME_LEN            33

typedef enum
{
    FEI_MSG_TYPE_MIN = 0,
    
    /* MLAG */
    FEI_MLAG_FDB_SYNC,          /* set MLAG FDB synced from peer */

    /* PTP */
    FEI_PTP_GET_TX_TS,          /* sync get PTP TX timestamp */
    FEI_PTP_RESET_TX_TS,          /* sync reset PTP TX timestamp */
    FEI_PTP_GET_HW_SECOND,          /* sync get PTP TX timestamp */
    
    FEI_OPENFLOW_POST_SYS_STARTUP,
    FEI_OPENFLOW_DO_BARRIER,
    FEI_OPENFLOW_DO_PORT_BARRIER_BY_OVS,
    FEI_OPENFLOW_DO_PORT_BARRIER_BY_SWITCH,
    FEI_OPENFLOW_PORT_ENABLE,
    FEI_OPENFLOW_PORT_OPERATION_AGG,
    FEI_OPENFLOW_PORT_SET_PROTECTED_VLAN,
    FEI_OPENFLOW_PORT_IGNORE_VLAN_FILTER,
    FEI_OPENFLOW_PORT_TUNNEL_MPLS_DISABLE,
    FEI_OPENFLOW_PORT_NATIVE_VLAN_CONFIG,
    FEI_OPENFLOW_PORT_SET_PKT_ACTION,
    FEI_OPENFLOW_PORT_NOTIFY_LINK,
    FEI_OPENFLOW_PORT_RESET,
    FEI_OPENFLOW_BOND_CREATE,
    FEI_OPENFLOW_BOND_UPDATE_STATE,
    FEI_OPENFLOW_BOND_ATTACH_SLAVE,
    FEI_OPENFLOW_BOND_SET_MAC,
    FEI_OPENFLOW_PORT_SET_CONFIG,
    FEI_OPENFLOW_PORT_ADD_TO_BRIDGE,
    FEI_OPENFLOW_FLOW_ADD,
    FEI_OPENFLOW_FLOW_MOD,
    FEI_OPENFLOW_FLOW_DEL,
    FEI_OPENFLOW_FLOW_RESET,
    FEI_OPENFLOW_FLOW_STATS,
    FEI_OPENFLOW_FLOW_UPDATE_ALL_ACTION_LIST,
    /* Added by weizj for tunnel bug 33724 */
    FEI_OPENFLOW_FLOW_DROP_INGRESS,
    /* Added by weizj for udf */
    FEI_OPENFLOW_FLOW_UDF_ENABLE,
    FEI_OPENFLOW_FLOW_E2E_ENABLE,
    FEI_OPENFLOW_FLOW_SET_STPID,
    FEI_OPENFLOW_FLOW_SET_CTPID,
    FEI_OPENFLOW_FLOW_SET_UDF_PARSER,
    FEI_OPENFLOW_GROUP_ADD,
    FEI_OPENFLOW_GROUP_MOD,
    FEI_OPENFLOW_GROUP_DEL,
    FEI_OPENFLOW_GROUP_RESET,
    FEI_OPENFLOW_GROUP_STATS,
    /* Added by weizj for tunnel */
    FEI_OPENFLOW_TUNNEL_ADD,
    FEI_OPENFLOW_TUNNEL_DEL,
    FEI_OPENFLOW_TUNNEL_PORT_DECAP_EN,
    FEI_OPENFLOW_TUNNEL_DECAP_MODE_MOD,
    FEI_OPENFLOW_TUNNEL_VTEP_IP_MOD,
    /* Added by weizj for meter */
    FEI_OPENFLOW_METER_ADD,
    FEI_OPENFLOW_METER_MOD,
    FEI_OPENFLOW_METER_DEL,
    FEI_OPENFLOW_METER_MOD_IPG,
    /* Added by wangl for qos */
    FEI_OPENFLOW_QUEUE_SHAPE_ADD,
    /* Added by weizj for inband */
    FEI_OPENFLOW_INBAND_VIF_ADD,
    FEI_OPENFLOW_INBAND_VIF_DEL,
    FEI_OPENFLOW_INBAND_SET_CONFIG,
    FEI_OPENFLOW_STATS_ADD,
    FEI_OPENFLOW_STATS_DEL,
    FEI_OPENFLOW_STATS_GET,
    FEI_OPENFLOW_STATS_CLEAR,
    FEI_OPENFLOW_PARSER_SET_ECMP_HASH_KEY,
    FEI_OPENFLOW_PARSER_SET_LINKAGG_HASH_KEY,
    FEI_OPENFLOW_PARSER_SET_MPLS_PARSER,
    FEI_OPENFLOW_STUB_CREATE,
    FEI_OPENFLOW_STUB_ENABLE,

    FEI_OPENFLOW_PACKET_OUT,

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
    FEI_OPENFLOW_FLOW_ADD_TPOAM,
    FEI_OPENFLOW_FLOW_DEL_TPOAM,
    FEI_OPENFLOW_FLOW_UPDATE_TPOAM,
    FEI_OPENFLOW_SET_PW_MODE,
    /* liwh end */

    /*added by yejl for g8131 fast aps en*/
    FEI_G8131_MPLS_FAST_APS_EN,
    FEI_G8131_MPLS_PROTECTION_EN,
    /* yejl end */
    
    FEI_MSG_TYPE_MAX
} hsrv_msg_type_t;

typedef enum
{
    SWITCH_FEI_PHY_NOTIFY = 0,      /* Notify physical port link/phy state changes */
    SWITCH_FEI_MLAG_FDB_SYNC,       /* Notify MLAG FDB sync to peer */
    SWITCH_FEI_FDB_LOOP_SYNC,       /* Notify FDB LOOP sync to switch */
    SWITCH_FEI_MSG_TYPE_MAX
} switch_fei_msg_type_t;

typedef enum
{
    ROUTED_FEI_FDB_AGE_ARP_NOTIFY = 0, 
    ROUTED_FEI_FDB_AGE_ND_NOTIFY = 1, 
    /* modified by liwh for bug 48486, 2018-08-14 */
    ROUTED_FEI_KERNEL_UPDATE_ND_NOTIFY = 2,
    /* liwh end */
    ROUTED_FEI_DYNAMIC_TUNNEL_UPDATE_NOTIFY = 3,

    ROUTED_FEI_NAT_SESSION_AGING_DEL = 4,

    ROUTED_FEI_MSG_TYPE_MAX
} route_fei_msg_type_t;

typedef enum
{
    /* switch openflow hal msg */
    OPENFLOW_HAL_MSG_TYPE_FLOW_STATS,
    OPENFLOW_HAL_MSG_TYPE_METER_STATS,
    OPENFLOW_HAL_MSG_TYPE_GROUP_STATS,
    
    OPENFLOW_HAL_MSG_TYPE_MAX
} openflow_fei_msg_type_t;

#endif /* __HSRV_MSG_H__ */

